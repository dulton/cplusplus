/*
*********************************************************************************
*                                                                               *
* NOTICE:                                                                       *
* This document contains information that is confidential and proprietary to    *
* RADVision LTD.. No part of this publication may be reproduced in any form     *
* whatsoever without written prior approval by RADVision LTD..                  *
*                                                                               *
* RADVision LTD. reserves the right to revise this publication and make changes *
* without obligation to notify any person of such revisions or changes.         *
* Copyright RADVision 1996.                                                     *
* Last Revision: Jan. 2000                                                      *
*********************************************************************************
*/


/*********************************************************************************
 *                              <TransactionTimer.c>
 *
 *  This file defines the behavior of a transaction object when it's timer expires.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Nov 2000
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "TransactionTimer.h"
#include "TransactionObject.h"
#include "TransactionControl.h"
#include <math.h>
#include "_SipTransactionTypes.h"
#include "TransactionTransport.h"
#include "TransactionState.h"
#include "TransactionCallBacks.h"
#include "TransactionSecAgree.h"
#include "_SipCommonCore.h"
#include "_SipCommonUtils.h"
#include "_SipTransmitter.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        MACRO DEFINITIONS                              */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus HandleMainTimerNonInviteRetransmission(
                      IN  Transaction          *pTransc);

static RvStatus HandleMainTimerInviteRetransmission(
                      IN  Transaction          *pTransc);

#ifdef RV_SIGCOMP_ON
static RvStatus SetNextSigCompRetransmittedMsgType(
                        IN  Transaction  *pTransc,
                        OUT RvBool       *pbContinueRetrans);

static void TransactionTimerSetSigCompTCPTimer(
                      IN   Transaction         *pTransc,
                      IN   SipTransportMsgType  msgType,
                      OUT  RvInt32             *pMainTimerInterval);
#endif /* RV_SIGCOMP_ON */

static void TransactionTimerSetFirstTCPTimer(
                      IN   Transaction          *pTransc,
                      IN   SipTransportMsgType   msgType,
                      OUT  RvInt32              *pMainTimerInterval,
                      OUT  RvInt32              *pLongTimerInterval);

static void TransactionTimerSetRetransTCPTimer(
                      IN   Transaction          *pTransc,
                      IN   SipTransportMsgType   msgType,
                      OUT  RvInt32              *pMainTimerInterval,
                      OUT  RvInt32              *pLongTimerInterval);

static void TcpTimersDecide(IN  Transaction*           pTransc,
                         IN  SipTransportMsgType    msgType,
                         OUT  RvBool*               bSetMainTimer,
                         OUT  RvInt32*              mainTimerInterval,
                         OUT  RvBool*               bSetLongReqestTimer,
                         OUT  RvInt32*              longTimerInterval);

static void UDPTimersDecide(IN  Transaction*           pTransc,
                         IN  SipTransportMsgType    msgType,
                         OUT  RvBool*               bSetMainTimer,
                         OUT  RvInt32*              mainTimerInterval,
                         OUT  RvBool*               bSetLongReqestTimer,
                         OUT  RvInt32*              longTimerInterval);

static void RVCALLCONV HandleTimeoutAtInviteProceedingState(
                                         IN Transaction *pTransc);


/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TransactionTimerMainTimerStart
 * ------------------------------------------------------------------------
 * General: Starts the transaction's main timer.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction to start the timer from.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionTimerMainTimerStart(IN Transaction *pTransc,
                                                   IN RvUint32     timerInterval,
                                                   IN RvChar*     functionName)
{
    RvStatus rv;

    rv = SipCommonCoreRvTimerStartEx(&pTransc->hMainTimer,
                                    pTransc->pMgr->pSelect,
                                    timerInterval,
                                    TransactionTimerHandleMainTimerTimeout,
                                    pTransc);
    
    if (rv == RV_OK)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
            "%s - Transaction 0x%p: timer was set to %u milliseconds",
            functionName, pTransc, timerInterval));
    }
	
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(functionName);
#endif

    return rv;
}
/***************************************************************************
 * TransactionTimerMainTimerRelease
 * ------------------------------------------------------------------------
 * General: Release the transaction's main timer, and set it to NULL.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction to release the timer from.
 ***************************************************************************/
void RVCALLCONV TransactionTimerMainTimerRelease(IN Transaction *pTransc)
{
    if (NULL != pTransc &&
        SipCommonCoreRvTimerStarted(&pTransc->hMainTimer))
    {
        /* Release the timer and set it to NULL */
        SipCommonCoreRvTimerCancel(&pTransc->hMainTimer);
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "TransactionTimerMainTimerRelease - Transaction 0x%p: Timer 0x%p was released",
                  pTransc, &pTransc->hMainTimer));
    }
}

/***************************************************************************
 * TransactionTimerReleaseAllTimers
 * ------------------------------------------------------------------------
 * General: Release both timers of the transactions. (generalRequestTimeoutTimer
 *          and the interval timer).
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction to release the timer from.
 ***************************************************************************/
void RVCALLCONV TransactionTimerReleaseAllTimers(IN Transaction *pTransc)
{
    if (NULL != pTransc)
    {
        if(SipCommonCoreRvTimerStarted(&pTransc->hRequestLongTimer))
        {
            /* Release the timer */
            SipCommonCoreRvTimerCancel(&pTransc->hRequestLongTimer);
            RvLogDebug((pTransc->pMgr)->pLogSrc,((pTransc->pMgr)->pLogSrc,
                  "TransactionTimerReleaseAllTimers - Transaction 0x%p: Timeout timer was released",
                  pTransc));
        }
        TransactionTimerMainTimerRelease(pTransc);
    }
}

/***************************************************************************
 * TransactionTimerRetransmit
 * ------------------------------------------------------------------------
 * General: Called when a retransmission timer expires.
 *          If the number of retransmissions sent is smaller than the number
 *          of retransmissions that are supposed to be sent, the last message
 *          is retransmited, and the retransmission timer is set according to
 *          the next retransmission round.
 *          If the number of retransmission sent matches the number of
 *          retransmissions that were supposed to be sent the transaction will
 *          be terminated.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction to retransmit from.
 *        maxRetransmissions - The number of retransmissions supposed to be
 *                             sent by this transaction according to state.
 *                             If maxRetransmissions is equal to -1 then there is no
 *                             limitation on the retransmissions number.
 *        enableTerminateTransaction - enables/disables transaction termination
 ***************************************************************************/
void RVCALLCONV TransactionTimerRetransmit(
                                 IN Transaction *pTransc,
                                 IN RvInt8       maxRetransmissions,
                                 IN RvBool       enableTerminateTransaction)
{
    if (NULL == pTransc)
    {
        return;
    }

    if ((pTransc->retransmissionsCount < maxRetransmissions) ||
        (maxRetransmissions == -1))
    {
        /* The number of retransmissions sent is smaller than the number
           of retransmissions that are suppose to be sent */
        /* Retransmit the last message transmited by the transaction */
        if (RV_OK != TransactionTransportRetransmitMessage(pTransc))
        {
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
            TransactionTerminate(RVSIP_TRANSC_REASON_TIME_OUT, pTransc);
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

            return;
        }
    }
    else
    {
        /* The transaction finished all of the retransmissions without
           receiving an appropriate response. The transaction is terminated */
        if (enableTerminateTransaction)
        {
            TransactionTerminate(RVSIP_TRANSC_REASON_TIME_OUT, pTransc);
        }
    }

}


/***************************************************************************
 * TransactionTimerHandleMainTimerTimeout
 * ------------------------------------------------------------------------
 * General: Called when ever the main transaction timer expires.
 *          If this is a retransmission    timer call the sipTransactionRetransmit function.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: timerHandle - The timer that has expired.
 *          pContext - The transaction this timer was called for.
 ***************************************************************************/
RvBool TransactionTimerHandleMainTimerTimeout(IN void  *pContext,
                                              IN RvTimer *timerInfo)
{
    Transaction        *pTransc;

    pTransc = (Transaction *)pContext;

    if (NULL == pTransc)
    {
        return RV_FALSE;
    }

    if(pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
    {
        return RV_FALSE;
    }

    if (TransactionLockMsg(pTransc) != RV_OK)
    {
        return RV_FALSE;
    }

    if(SipCommonCoreRvTimerIgnoreExpiration(&(pTransc->hMainTimer), timerInfo) == RV_TRUE)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
            "TransactionTimerHandleMainTimerTimeout - Transaction 0x%p: timer expired but was already released. do nothing...",
            pTransc));

        TransactionUnLockMsg(pTransc);
        return RV_FALSE;
    }

    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "TransactionTimerHandleMainTimerTimeout - Transaction 0x%p: timer expired (transc state=%s)",
              pTransc,SipTransactionGetStateName(pTransc->eState)));

    /* Release the timer */
    SipCommonCoreRvTimerExpired(&pTransc->hMainTimer);
    TransactionTimerMainTimerRelease(pTransc);

    switch (pTransc->eState)
    {
    case (RVSIP_TRANSC_STATE_CLIENT_INVITE_CANCELLING):
    case (RVSIP_TRANSC_STATE_CLIENT_GEN_CANCELLING):
        /* Terminate timer*/
        TransactionTerminate(RVSIP_TRANSC_REASON_TIME_OUT,pTransc);
        break;
    case (RVSIP_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT):
    case (RVSIP_TRANSC_STATE_CLIENT_INVITE_ACK_SENT):
    case (RVSIP_TRANSC_STATE_SERVER_CANCEL_FINAL_RESPONSE_SENT):
        TransactionTerminate(RVSIP_TRANSC_REASON_NORMAL_TERMINATION,
                             pTransc);
        break;
    case RVSIP_TRANSC_STATE_CLIENT_GEN_FINAL_RESPONSE_RCVD:
    case RVSIP_TRANSC_STATE_CLIENT_CANCEL_FINAL_RESPONSE_RCVD:
    case RVSIP_TRANSC_STATE_SERVER_INVITE_ACK_RCVD:
        TransactionTerminate(FinalResponseReason(pTransc->responseCode),
                             pTransc);
        break;
    case (RVSIP_TRANSC_STATE_CLIENT_CANCEL_SENT):
    case (RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT):
    case (RVSIP_TRANSC_STATE_CLIENT_GEN_PROCEEDING):
    case (RVSIP_TRANSC_STATE_CLIENT_CANCEL_PROCEEDING):
#ifndef RV_SIP_PRIMITIVES
    case (RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT):
#endif
        /* Retransmission timer. */
        HandleMainTimerNonInviteRetransmission(pTransc);
        break;
    case (RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT):
        if(pTransc->pMgr->bOldInviteHandling == RV_FALSE &&
           pTransc->responseCode >= 200 && pTransc->responseCode < 300)
        {
            TransactionTerminate(RVSIP_TRANSC_REASON_NORMAL_TERMINATION,
                pTransc);
        }
        else
        {
            HandleMainTimerNonInviteRetransmission(pTransc);
            
        }
        break;
    case (RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING):
        /* Retransmission timer. Terminate after 7 retransmissions*/
        HandleMainTimerInviteRetransmission(pTransc);
        break;
    case (RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING):
        HandleTimeoutAtInviteProceedingState(pTransc);
        break;
    case (RVSIP_TRANSC_STATE_CLIENT_INVITE_PROXY_2XX_RESPONSE_RCVD):
    case (RVSIP_TRANSC_STATE_SERVER_INVITE_PROXY_2XX_RESPONSE_SENT):
        /* terminate the proxy transaction if the timer has expires
           if the proxy send or received 2xx response and if the proxy
           send or received ACK request when he sends earlier 2xx response*/
   		TransactionTerminate(RVSIP_TRANSC_REASON_NORMAL_TERMINATION,pTransc);
        break;
    default:
        break;
    }

    TransactionUnLockMsg(pTransc);

    return RV_FALSE;
}

/***************************************************************************
 * TransactionTimerHandleRequestLongTimerTimeout
 * ------------------------------------------------------------------------
 * General: Called when ever the request long timer expires.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: timerHandle - The handle to the timer that has expired.
 *          pContext - The transaction this timer was called for.
 ***************************************************************************/
RvBool TransactionTimerHandleRequestLongTimerTimeout(
                                 IN void    *pContext,
                                 IN RvTimer *timerInfo)
{
    Transaction        *pTransc;
    RvStatus            rv = RV_OK;
    RvBool                enableTransactionTermination = RV_TRUE;

    pTransc = (Transaction *)pContext;


    if (TransactionLockMsg(pTransc) != RV_OK)
    {
        return RV_FALSE;
    }
    
    if(SipCommonCoreRvTimerIgnoreExpiration(&(pTransc->hRequestLongTimer), timerInfo) == RV_TRUE)
    {
        RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
            "TransactionTimerHandleRequestLongTimerTimeout - Transaction 0x%p: timer expired but was already released. do nothing", pTransc));
        TransactionUnLockMsg(pTransc);
        
        return RV_FALSE;
    }

    /*release transaction timers*/
    SipCommonCoreRvTimerExpired(&pTransc->hRequestLongTimer);
    TransactionTimerReleaseAllTimers(pTransc);
    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "TransactionTimerHandleRequestLongTimerTimeout - Transaction 0x%p: timer expired", pTransc));

    /* Call Send Failure callback */
    switch (pTransc->eState)
    {
    case RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT:
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING:
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
        if ((pTransc->pEvHandlers)->pfnEvStateChanged != NULL &&
            pTransc->eMethod != SIP_TRANSACTION_METHOD_CANCEL)
        {
            enableTransactionTermination = RV_FALSE; /*do not terminate transaction when we use dns*/
            rv = TransactionHandleMsgSentFailureEvent(pTransc,RVSIP_TRANSC_REASON_TIME_OUT);

            if (rv != RV_OK)
            {
                enableTransactionTermination = RV_TRUE;
                break;
            }
        }
#elif defined RV_SIP_IMS_ON
		/* Since we are compiled without DNS, the transaction will not move to the message
		   send failure state. Therefore we need to notify the security-agreement that 
		   a message send failure occurred. */
		TransactionSecAgreeMsgSendFailure(pTransc);
#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */
        break;

    case RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT:
        rv = TransactionControlSendResponse(pTransc,
                                            500,
                                            NULL,
                                            RVSIP_TRANSC_REASON_REL_PROV_RESP_TIME_OUT,
                                            RV_FALSE);
        if(rv != RV_OK)
        {
            enableTransactionTermination = RV_TRUE;
            break;
        }
        enableTransactionTermination = RV_FALSE;
        break;
    default:
        break;
    }

    if (enableTransactionTermination == RV_TRUE)
    {
        /* Terminates the transaction */
        TransactionTerminate(RVSIP_TRANSC_REASON_TIME_OUT, pTransc);
    }
    TransactionUnLockMsg(pTransc);

    return RV_FALSE;
}

/***************************************************************************
 * TransactionTimerSetTimers
 * ------------------------------------------------------------------------
 * General:
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc   - the transaction handle
 *            msgType   - the type of message that was sent
 ***************************************************************************/
RvStatus RVCALLCONV TransactionTimerSetTimers(
                                   IN  Transaction*           pTransc,
                                   IN  SipTransportMsgType    msgType)
{
    RvInt32  mainTimerInterval = 0;
    RvInt32  longTimerInterval = 0;
    RvBool   bSetMainTimer = RV_FALSE;
    RvBool   bSetLongReqestTimer = RV_FALSE;
    RvStatus rv;

    /*do not release a linger timer if the message is proxy 2xx response*/
    if(msgType == SIP_TRANSPORT_MSG_PROXY_2XX_SENT &&
       pTransc->bIsProxySend2xxTimer == RV_TRUE)
    {
        return RV_OK;
    }

 
    /*if the state is cancelling then no other timer should be set.
    This code was added if the state has changed to cancelling but the invite
    message was not sent yet. We do not want to set the invite timers after
    it will be sent*/
    if(pTransc->eState == RVSIP_TRANSC_STATE_CLIENT_GEN_CANCELLING ||
       pTransc->eState == RVSIP_TRANSC_STATE_CLIENT_INVITE_CANCELLING)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionTimerSetTimers - Transaction 0x%p: is in the %s state. Timer was already set",
                pTransc,TransactionGetStateName(pTransc->eState)));
        return RV_OK;
    }
    /* If the transaction message is a retransmission then the timer will release in the
       TcpTimersDecide / UDPTimersDecide function, if needed */
    if(pTransc->retransmissionsCount == 0)
    {
        /*release transaction timers*/
        TransactionTimerReleaseAllTimers(pTransc);
    }
    else
    {
        if(msgType == SIP_TRANSPORT_MSG_ACK_SENT)
        {
            RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionTimerSetTimers - Transaction 0x%p: state %s, ACK retransmission - timer was already set",
                pTransc,TransactionGetStateName(pTransc->eState)));
            return RV_OK;
        }
    }

    /*we never retransmit a non reliable provisional response*/
    if(msgType == SIP_TRANSPORT_MSG_PROV_RESP_SENT)
    {
        return RV_OK;
    }

    /*determine timers interval according to transport*/
    switch (pTransc->eTranspType)
    {
    case RVSIP_TRANSPORT_TCP:
    case RVSIP_TRANSPORT_TLS:
    case RVSIP_TRANSPORT_SCTP:
        TcpTimersDecide(pTransc, msgType, &bSetMainTimer, &mainTimerInterval, 
                        &bSetLongReqestTimer, &longTimerInterval);
        break;
    case RVSIP_TRANSPORT_UDP:
    case RVSIP_TRANSPORT_UNDEFINED:
        UDPTimersDecide(pTransc, msgType, &bSetMainTimer, &mainTimerInterval, 
                        &bSetLongReqestTimer, &longTimerInterval);
        break;
    default:
        UDPTimersDecide(pTransc, msgType, &bSetMainTimer, &mainTimerInterval, 
                        &bSetLongReqestTimer, &longTimerInterval);
        break;
    }

    if(bSetMainTimer == RV_TRUE)
    {
        rv = TransactionTimerMainTimerStart(pTransc, mainTimerInterval, "TransactionTimerSetTimers");   
        if (rv != RV_OK)
        {
            RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "TransactionTimerSetTimers - Transaction 0x%p: No timer is available",
                pTransc));
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
    if(bSetLongReqestTimer == RV_TRUE)
    {
        rv = SipCommonCoreRvTimerStartEx(&pTransc->hRequestLongTimer,
                    pTransc->pMgr->pSelect,
                    longTimerInterval,
                    TransactionTimerHandleRequestLongTimerTimeout,
                    pTransc);
        if (rv != RV_OK)
        {
            RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "TransactionTimerSetTimers - Transaction 0x%p: No timer is available for long timeout timer",
                pTransc));
            return RV_ERROR_OUTOFRESOURCES;
        }
        else
        {
            RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc   ,
                "TransactionTimerSetTimers - Transaction 0x%p: long timeout timer was set to %u milliseconds",
                pTransc, longTimerInterval));
        }
    }
    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * HandleMainTimerNonInviteRetransmission
 * ------------------------------------------------------------------------
 * General: Handles a retransmission of general request when the
 *          transaction main timer expires.
 * Return Value: RV_OK on successful handling.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc         - The handle to the transaction
 ***************************************************************************/
static RvStatus HandleMainTimerNonInviteRetransmission(
                      IN  Transaction          *pTransc)
{
    RvStatus rv               = RV_OK;
    RvBool   bContinueRetrans = RV_TRUE;

#ifdef RV_SIGCOMP_ON
    if (TransactionGetOutboundMsgCompType(pTransc) == RVSIP_COMP_SIGCOMP)
    {
        rv = SetNextSigCompRetransmittedMsgType(pTransc,&bContinueRetrans);
        if (RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "HandleMainTimerNonInviteRetransmission - Transaction 0x%p: failed to determine what is the next msg type- terminated",
                pTransc));
            TransactionTerminate(RVSIP_TRANSC_REASON_TIME_OUT,pTransc);

            return rv;
        }
    }
#endif /* RV_SIGCOMP_ON */
    
    if (bContinueRetrans == RV_TRUE)
    {
        RvInt8 maxRetrans;
        if(RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT == pTransc->eState)
        {
            maxRetrans = pTransc->pTimers->maxInviteResponseRetransmissions;
        }
        else
        {
            maxRetrans = pTransc->pTimers->maxGeneralRequestRetransmissions;
        }
        TransactionTimerRetransmit(pTransc, 
                                   maxRetrans, 
                                   RV_TRUE);
    }
    else
    {
        /* If the the retransmission is stopped the transaction is terminated */
        rv = TransactionTerminate(RVSIP_TRANSC_REASON_TIME_OUT,pTransc);
    }

    return rv;
}

/***************************************************************************
 * HandleMainTimerInviteRetransmission
 * ------------------------------------------------------------------------
 * General: Handles a retransmission of INVITE request when the
 *          transaction main timer expires.
 * Return Value: RV_OK on successful handling.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc         - The handle to the transaction
 ***************************************************************************/
static RvStatus HandleMainTimerInviteRetransmission(
                                                    IN  Transaction          *pTransc)
{
  RvBool   enableTransactionTermination = RV_TRUE;
  RvStatus rv                           = RV_OK;
#ifdef RV_SIGCOMP_ON
  RvBool   bContinueRetrans;
  
  if (TransactionGetOutboundMsgCompType(pTransc) == RVSIP_COMP_SIGCOMP)
  {
    rv = SetNextSigCompRetransmittedMsgType(pTransc,&bContinueRetrans);
    if (RV_OK != rv)
    {
      RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
        "HandleMainTimerInviteRetransmission - Transaction 0x%p: failed to determine what is the next msg type- terminated",
        pTransc));
      TransactionTerminate(RVSIP_TRANSC_REASON_TIME_OUT,pTransc);
      
      return rv;
    }
    if (bContinueRetrans == RV_FALSE)
    {
      /* If the the retransmission is stopped the transaction is terminated */
      rv = TransactionTerminate(RVSIP_TRANSC_REASON_TIME_OUT,pTransc);
      
      return rv;
    }
    /* Beyond 6 INVITE retransmissions the application can    */
    /* only choose to "Return to Decompressed" otherwise the  */
    /* transaction will be handled in a general manner.       */
    /* NOTE: The application MUST choose to continue retrans. */
    /* SPIRENT_BEGIN */
    /*
    * COMMENTS: 
    */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    /* Note: the pTransc->retransmissionsCount is the total number of sent "INVITE" messages... it's not just 
    * the count of retransmitting "INVITE" messages alone.  So, we need to add one to the retransmit
    * count before passing it to this function.
    */
    if (pTransc->retransmissionsCount >= (pTransc->pMgr->maxRetransmitINVITENumber + 1)  &&
      SipTransmitterGetNextMsgCompType(pTransc->hTrx) == 
      RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNCOMPRESSED)
#else /* !defined(UPDATED_BY_SPIRENT) */
      if (pTransc->retransmissionsCount >= pTransc->pTimers->maxInviteRequestRetransmissions  &&
        SipTransmitterGetNextMsgCompType(pTransc->hTrx) == 
        RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNCOMPRESSED)
#endif /* defined(UPDATED_BY_SPIRENT) */ 
        /* SPIRENT_END */
      {
        /* Enable retransmission */ 
        TransactionTimerRetransmit(pTransc,pTransc->pTimers->maxInviteRequestRetransmissions,enableTransactionTermination);
        
        return RV_OK;
      }
  }
#endif /* RV_SIGCOMP_ON */
  
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
  /* Retransmission timer. Terminate after 7 retransmissions*/
  /* SPIRENT_BEGIN */
  /*
  * COMMENTS: 
  */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
  /* Note: the pTransc->retransmissionsCount is the total number of sent "INVITE" messages... it's not just 
  * the count of retransmitting "INVITE" messages alone.  So, we need to add one to the retransmit
  * count before passing it to this function.
  */
  if ((pTransc->retransmissionsCount >= (pTransc->pMgr->maxRetransmitINVITENumber + 1)) &&
    ((pTransc->pEvHandlers)->pfnEvStateChanged != NULL))
#else /* !defined(UPDATED_BY_SPIRENT) */
    if ((pTransc->retransmissionsCount >= pTransc->pTimers->maxInviteRequestRetransmissions) &&
      ((pTransc->pEvHandlers)->pfnEvStateChanged != NULL))
#endif /* defined(UPDATED_BY_SPIRENT) */ 
      /* SPIRENT_END */
    {
      RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
        "HandleMainTimerInviteRetransmission - Transaction 0x%p: will move to MsgSentFailure state",
        pTransc));
      
      enableTransactionTermination = RV_FALSE;
      rv = TransactionHandleMsgSentFailureEvent(pTransc,RVSIP_TRANSC_REASON_TIME_OUT);
      
      if (rv != RV_OK)
      {
        enableTransactionTermination = RV_TRUE;
      }
    }
#elif defined RV_SIP_IMS_ON
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    if(0) {
#endif
/* SPIRENT_END */
    /* Since we are compiled without DNS, the transaction will not move to the message
	   send failure state. Therefore we need to notify the security-agreement that 
	   a message send failure occurred. */
      TransactionSecAgreeMsgSendFailure(pTransc);
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    }
#endif
/* SPIRENT_END */

#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */
    /* SPIRENT_BEGIN */
    /*
    * COMMENTS: 
    */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    TransactionTimerRetransmit(pTransc,
      (pTransc->pMgr->maxRetransmitINVITENumber + 1),
      enableTransactionTermination);
#else /* !defined(UPDATED_BY_SPIRENT) */
    TransactionTimerRetransmit(pTransc,
      pTransc->pTimers->maxInviteRequestRetransmissions,
      enableTransactionTermination);
#endif /* defined(UPDATED_BY_SPIRENT) */ 
    /* SPIRENT_END */
    
    return rv;
}

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * SetNextSigCompRetransmittedMsgType
 * ------------------------------------------------------------------------
 * General: Set the next retransmitted message type that might be changed
 *          in case SigComp message.
 * Return Value: RV_OK on successful handling.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc         - The handle to the transaction
 * Output: bContinueRetrans     - Indicates if to continue the retransmission
 ***************************************************************************/
static RvStatus SetNextSigCompRetransmittedMsgType(
                        IN  Transaction  *pTransc,
                        OUT RvBool       *pbContinueRetrans)
{
    RvStatus                    rv = RV_OK;
    RvSipTransmitterMsgCompType eOutboundMsgCompType;
    RvSipTransmitterMsgCompType eNextMsgCompType;

    /* Retrieve the current outbound msg compression type */
    eOutboundMsgCompType = SipTransmitterGetOutboundMsgCompType(pTransc->hTrx);

    /* Set default Output values for assuring a default value */
    /* in an application that didn't register to the          */
    /* SigCompMsgNotRespondedEv callback */
    eNextMsgCompType   = RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNCOMPRESSED;
    *pbContinueRetrans = RV_TRUE;

    /* If the last sent message was SigComp then the stack   */
    /* consult the application about the next retransmission */
    rv = TransactionCallbackSigCompMsgNotRespondedEv(pTransc,
                                                     pTransc->retransmissionsCount,
                                                     eOutboundMsgCompType,
                                                     &eNextMsgCompType);
    if (rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "SetNextSigCompRetransmittedMsgType - Transaction 0x%p: Failed to consult the application",
                pTransc));

        return rv;
    }

    SipTransmitterSetNextMsgCompType(pTransc->hTrx,eNextMsgCompType);
    /* Check the validity of the application determination */
    switch (eNextMsgCompType)
    {
    /* Check if the application chose before to disable the sigComp message */
    case RVSIP_TRANSMITTER_MSG_COMP_TYPE_SIGCOMP_COMPRESSED:
    case RVSIP_TRANSMITTER_MSG_COMP_TYPE_SIGCOMP_COMPRESSED_INCLUDE_BYTECODE: 
        /* The 'pTransc->bIsUac == RV_TRUE' shouldn't be checked since the */
        /* callback SigCompMsgNotResponded is triggered only for outgoing  */ 
        /* requests. */
        if (pTransc->bSigCompEnabled == RV_FALSE)
        {
            RvLogWarning(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "SetNextSigCompRetransmittedMsgType - Transaction 0x%p: The application chose to retransmit SigComp in SigComp disabled transc. Stop retrans.",
                pTransc));
            *pbContinueRetrans = RV_FALSE;
            return rv;
        }
        break;
    /* The application asked to stop the retransmission process */
    case RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED:
        *pbContinueRetrans = RV_FALSE;
        return rv;
    default:
        rv = RV_OK; /* Do Nothing */
    }

    return rv;
}

/***************************************************************************
 * TransactionTimerSetSigCompTCPTimer
 * ------------------------------------------------------------------------
 * General: Returns the timers of a TCP SigComp transaction.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc         - The handle to the transaction
 *         msgType              - The type of message that is sent
 * Output: pMainTimerInterval   - The value to be set in the timer interval,
 *                                if bSetMainTimer = TRUE
 ***************************************************************************/
static void TransactionTimerSetSigCompTCPTimer(
                      IN   Transaction          *pTransc,
                      IN   SipTransportMsgType   msgType,
                      OUT  RvInt32              *pMainTimerInterval)
{
    if (SIP_TRANSPORT_MSG_REQUEST_SENT == msgType ||
        /* retransmit the message only if it is a response to INVITE */
        RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT == pTransc->eState)
    {
        *pMainTimerInterval = pTransc->pMgr->sigCompTcpTimer;
    }
}
#endif /* RV_SIGCOMP_ON */

/***************************************************************************
 * TransactionTimerSetFirstTCPTimer
 * ------------------------------------------------------------------------
 * General: Returns the timers of a TCP transaction that is sent for the
 *          first time (with no preceding retransmissions)
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc         - The handle to the transaction
 *         msgType              - The type of message that is sent
 * Output: pMainTimerInterval   - The value to be set in the timer interval,
 *                                if bSetMainTimer = TRUE
 *         pLongTimerInterval   - the value to be set in the timeout timer interval,
 *                                if bSetLongReqestTimer = TRUE
 ***************************************************************************/
static void TransactionTimerSetFirstTCPTimer(
                      IN   Transaction          *pTransc,
                      IN   SipTransportMsgType   msgType,
                      OUT  RvInt32              *pMainTimerInterval,
                      OUT  RvInt32              *pLongTimerInterval)
{
    RvBool bSetLongTimer = RV_TRUE;

    /* Initializing the basic intervals , at the end the booleans are */
    /* set according to these intervals */
    *pMainTimerInterval = 0;
    *pLongTimerInterval = 0;

    switch (pTransc->eState)
    {
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING:
        *pLongTimerInterval   = (pTransc->pTimers->T1Timeout)*64;
        break;
    case RVSIP_TRANSC_STATE_CLIENT_CANCEL_SENT:
    case RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT:
        *pLongTimerInterval   = (pTransc->pTimers->generalRequestTimeoutTimeout);
        break;
#ifndef RV_SIP_PRIMITIVES
    case RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT:
        *pLongTimerInterval   = (pTransc->pTimers->T1Timeout)*64;
        *pMainTimerInterval   = (pTransc->pTimers->T1Timeout);
        break;
#endif
    case RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT:
    
         if(pTransc->responseCode >= 200 && pTransc->responseCode < 300)
         {
             /*the transaction should terminate after the first 2xx*/
             if(pTransc->pMgr->bOldInviteHandling == RV_FALSE)
             {
                *pMainTimerInterval = (RVSIP_TRANSPORT_TCP==pTransc->eTranspType ||
                                       RVSIP_TRANSPORT_SCTP==pTransc->eTranspType) ? 
                                       OBJ_TERMINATION_DEFAULT_TCP_TIMEOUT :
                                       OBJ_TERMINATION_DEFAULT_TLS_TIMEOUT;
                *pLongTimerInterval = 0;
                bSetLongTimer = RV_FALSE;
                TransactionMgrHashRemove((RvSipTranscHandle)pTransc);
             }
             else /*the transaction handles transaction retransmissions*/
             {
                *pMainTimerInterval = (pTransc->pTimers->T1Timeout);
                *pLongTimerInterval = (pTransc->pTimers->T1Timeout)*64;
             }
         }
         else
         {
            *pLongTimerInterval   = (pTransc->pTimers->T1Timeout)*64;
         }
        break;
    case RVSIP_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT:
    case RVSIP_TRANSC_STATE_SERVER_CANCEL_FINAL_RESPONSE_SENT:
        if(msgType == SIP_TRANSPORT_MSG_FINAL_RESP_SENT)
        {
            *pMainTimerInterval = (RVSIP_TRANSPORT_TCP==pTransc->eTranspType ||
                                   RVSIP_TRANSPORT_SCTP==pTransc->eTranspType) ? 
                                  OBJ_TERMINATION_DEFAULT_TCP_TIMEOUT : OBJ_TERMINATION_DEFAULT_TLS_TIMEOUT;
            bSetLongTimer       = RV_FALSE;
        }
        break;
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_ACK_SENT:
        /* This state requires a timer to be set, according to the time defined
        in the state machine. Set the transaction's timer accordingly */
        if(msgType ==  SIP_TRANSPORT_MSG_ACK_SENT)
        {
            if(pTransc->responseCode < 300 && pTransc->responseCode >= 200)
            {
                *pMainTimerInterval = (0 == pTransc->pTimers->inviteLingerTimeout) ?
                    ((RVSIP_TRANSPORT_TCP==pTransc->eTranspType ||
                      RVSIP_TRANSPORT_SCTP==pTransc->eTranspType) ? 
                     OBJ_TERMINATION_DEFAULT_TCP_TIMEOUT : OBJ_TERMINATION_DEFAULT_TLS_TIMEOUT
                    ) :
                    pTransc->pTimers->inviteLingerTimeout;

            }
            else
            {
                *pMainTimerInterval = OBJ_TERMINATION_DEFAULT_TCP_TIMEOUT;
            }
            bSetLongTimer       = RV_FALSE;
        }
        break;
    case RVSIP_TRANSC_STATE_SERVER_INVITE_PROXY_2XX_RESPONSE_SENT:
        if(RV_FALSE == pTransc->bIsProxySend2xxTimer)
        {
            *pMainTimerInterval           = pTransc->pMgr->proxy2xxSentTimer;
			if(*pMainTimerInterval == 0)
			{
				*pMainTimerInterval = (RVSIP_TRANSPORT_TCP == pTransc->eTranspType ||
                                           RVSIP_TRANSPORT_SCTP==pTransc->eTranspType) ?
                                       OBJ_TERMINATION_DEFAULT_TCP_TIMEOUT : OBJ_TERMINATION_DEFAULT_TLS_TIMEOUT;
			}
            pTransc->bIsProxySend2xxTimer = RV_TRUE;
            bSetLongTimer                 = RV_FALSE;
        }
        break;
    default:
        break;
    }

    /* If the timer is 0 then the message will not be retransmitted */
    if(*pLongTimerInterval == 0 && bSetLongTimer == RV_TRUE)
    {
        *pLongTimerInterval = (RVSIP_TRANSPORT_TCP==pTransc->eTranspType ||
                               RVSIP_TRANSPORT_SCTP==pTransc->eTranspType) ?
                              OBJ_TERMINATION_DEFAULT_TCP_TIMEOUT : OBJ_TERMINATION_DEFAULT_TLS_TIMEOUT;
    }

    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
        "TransactionTimerSetFirstTCPTimer - Transaction 0x%p: state=%s, Decisions: (pMainTimerInterval=%d, pLongTimerInterval=%d)",
        pTransc, TransactionGetStateName(pTransc->eState), *pMainTimerInterval, *pLongTimerInterval));

}

/***************************************************************************
 * TransactionTimerSetRetransTCPTimer
 * ------------------------------------------------------------------------
 * General: Returns the timers of a TCP transaction that is sent as
 *          retransmission.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc         - The handle to the transaction
 *         msgType              - The type of message that is sent
 * Output: pMainTimerInterval   - The value to be set in the timer interval,
 *                                if bSetMainTimer = TRUE
 *         pLongTimerInterval   - the value to be set in the timeout timer interval,
 *                                if bSetLongReqestTimer = TRUE
 ***************************************************************************/
static void TransactionTimerSetRetransTCPTimer(
                      IN   Transaction          *pTransc,
                      IN   SipTransportMsgType   msgType,
                      OUT  RvInt32              *pMainTimerInterval,
                      OUT  RvInt32              *pLongTimerInterval)
{
    TransactionTimerMainTimerRelease(pTransc);

    RV_UNUSED_ARG(msgType);
    /* Initializing the basic intervals , at the end the booleans are */
    /* set according to these intervals */
    *pMainTimerInterval = 0;
    *pLongTimerInterval = 0;

    switch(pTransc->eState)
    {
    case RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT:
#ifndef RV_SIP_PRIMITIVES
    case RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT:
#endif
        if(pTransc->responseCode < 300) /* 2xx final response OR provisional response */
        {
        /* Retransmission timer is T1*2^n for n=retransmissions-1, or
            T2 if T1*2^n is larger than T2. */
            *pMainTimerInterval = (pTransc->pTimers->T1Timeout)*
                                  SipCommonPower(pTransc->retransmissionsCount);
#ifndef RV_SIP_PRIMITIVES
            if ((RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT != pTransc->eState) &&
                (*pMainTimerInterval > (pTransc->pTimers->T2Timeout)))
            {
                *pMainTimerInterval = (pTransc->pTimers->T2Timeout);
            }
#endif
        }
        break;
    default:
        break;
    }
}

/***************************************************************************
 * TcpTimersDecide
 * ------------------------------------------------------------------------
 * General: Returns the timers of transactions that are working with TCP.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc        - the handle to the transaction
 *         msgType             - the type of message that was sent
 * Output: bSetMainTimer       - TRUE if we need to set the timer, FALSE - otherwise.
 *         mainTimerInterval   - the value to be set in the timer interval,
 *                               if bSetMainTimer = TRUE
 *         bSetLongReqestTimer - TRUE if we need to set the timeout timer, FALSE - otherwise.
 *         longTimerInterval   - the value to be set in the timeout timer interval,
 *                                if bSetLongReqestTimer = TRUE
 ***************************************************************************/
static void TcpTimersDecide(IN  Transaction*           pTransc,
                            IN   SipTransportMsgType   msgType,
                            OUT  RvBool*               bSetMainTimer,
                            OUT  RvInt32*              mainTimerInterval,
                            OUT  RvBool*               bSetLongReqestTimer,
                            OUT  RvInt32*              longTimerInterval)
{

    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "TcpTimersDecide - Transaction 0x%p: Setting timers (transaction state=%s, msgType=%d)",
              pTransc,TransactionGetStateName(pTransc->eState),msgType));

    /*---------------------------------------------------
          sending the mesage for the first time
     ----------------------------------------------------*/
    if (pTransc->retransmissionsCount == 0)
    {
        TransactionTimerSetFirstTCPTimer(pTransc,
                                         msgType,
                                         mainTimerInterval,
                                         longTimerInterval);
    }
    /*----------------------------------------------
        the message is a retransmission
    ------------------------------------------------*/
    else
    {
        /*if this is a retransmission of a general response then we already have
        a timer set and we should not change it - this is a fix to the following
        scenario:
        general request received, 200 response was sent and a timer was set to 10.
        Then a retransmission for sigcomp was received. We don't need to set the
        timer again. This can happen only on sigcomp scenarios.*/
        if(pTransc->eMethod != SIP_TRANSACTION_METHOD_INVITE &&
            msgType == SIP_TRANSPORT_MSG_FINAL_RESP_SENT)
        {
            *mainTimerInterval = 0;
            *longTimerInterval = 0;
            return;
        }
        
        TransactionTimerSetRetransTCPTimer(pTransc,
                                          msgType,
                                          mainTimerInterval,
                                          longTimerInterval);
    }

#ifdef RV_SIGCOMP_ON
    /* Setting the configured non-standard SigComp timer for message   */
    /* ONLY in case that no other standard MAIN timeout was set before */
    if (0                  == *mainTimerInterval &&
        RVSIP_COMP_SIGCOMP == TransactionGetOutboundMsgCompType(pTransc))
    {
        TransactionTimerSetSigCompTCPTimer(pTransc,msgType,mainTimerInterval);
    }
#endif /* RV_SIGCOMP_ON */

    *bSetMainTimer       = (*mainTimerInterval == 0) ? RV_FALSE : RV_TRUE;
    *bSetLongReqestTimer = (*longTimerInterval == 0) ? RV_FALSE : RV_TRUE;

    /* Increment the retransmissions count */
    pTransc->retransmissionsCount += 1;

    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "TcpTimersDecide - Transaction 0x%p: Decisions: SetMainTimer=%d, mainTimerInterval=%d, SetLongReqestTimer=%d, longTimerInterval=%d",
               pTransc, *bSetMainTimer, *mainTimerInterval, *bSetLongReqestTimer, *longTimerInterval));
}

/***************************************************************************
 * UDPTimersDecide
 * ------------------------------------------------------------------------
 * General: Returns the timers of transactions that are working with UDP.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc           - the handle to the transaction
 *         msgType           - the type of message that was sent
 * Output: bSetMainTimer     - TRUE if we need to set the timer, FALSE - otherwise.
 *         mainTimerInterval - the value to be set in the timer interval,
 *                             if bSetMainTimer = TRUE
 *         bSetLongReqestTimer - TRUE if we need to set the timeout timer, FALSE - otherwise.
 *         longTimerInterval - the value to be set in the timeout timer interval,
 *                                if bSetLongReqestTimer = TRUE
 ***************************************************************************/
static void UDPTimersDecide( IN   Transaction*         pTransc,
                             IN   SipTransportMsgType  msgType,
                             OUT  RvBool*              bSetMainTimer,
                             OUT  RvInt32*             mainTimerInterval,
                             OUT  RvBool*              bSetLongReqestTimer,
                             OUT  RvInt32*             longTimerInterval)
{
    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "UDPTimersDecide - Transaction 0x%p: Setting timers",
              pTransc));

    switch(pTransc->eState)
    {
    case RVSIP_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT:
    case RVSIP_TRANSC_STATE_SERVER_CANCEL_FINAL_RESPONSE_SENT:
        if(msgType == SIP_TRANSPORT_MSG_FINAL_RESP_SENT &&
           pTransc->retransmissionsCount == 0)
        {
            *mainTimerInterval = (pTransc->pTimers->genLingerTimeout);
            *bSetMainTimer     = RV_TRUE;
        }
        else
        {
            *bSetMainTimer       = RV_FALSE;
            *bSetLongReqestTimer = RV_FALSE;
            return;
        }
        break;
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_ACK_SENT:
        {
        /* This state requires a timer to be set, according to the time defined
            in the state machine. Set the transaction's timer accordingly */
            if(msgType ==  SIP_TRANSPORT_MSG_ACK_SENT &&  pTransc->retransmissionsCount == 0)
            {
                *mainTimerInterval = pTransc->pTimers->inviteLingerTimeout;
                *bSetMainTimer     = RV_TRUE;
            }
            else
            {
                *bSetMainTimer       = RV_FALSE;
                *bSetLongReqestTimer = RV_FALSE;
                return;
            }
        }
    break;
    case RVSIP_TRANSC_STATE_SERVER_INVITE_PROXY_2XX_RESPONSE_SENT:
        if(RV_FALSE == pTransc->bIsProxySend2xxTimer &&
           pTransc->retransmissionsCount == 0)
        {
            *mainTimerInterval            = (pTransc->pMgr)->proxy2xxSentTimer;
            pTransc->bIsProxySend2xxTimer = RV_TRUE;
            *bSetMainTimer                = RV_TRUE;
        }
        else
        {
            *bSetMainTimer       = RV_FALSE;
            *bSetLongReqestTimer = RV_FALSE;
            return;
        }
        break;
    default:
        *bSetMainTimer = RV_FALSE;
        break;
    }
    if(*bSetMainTimer == RV_TRUE)
    {
        if(*mainTimerInterval == 0)
        {
            *bSetMainTimer = RV_FALSE;
        }
        *bSetLongReqestTimer = RV_FALSE;
        ++pTransc->retransmissionsCount;

        return;
    }

    /*----------------------------------------------
        the message is a retransmission
    ------------------------------------------------*/
    if (pTransc->retransmissionsCount != 0)
    {
        TransactionTimerMainTimerRelease(pTransc);
        /* Define the time interval till next retransmission */
        if (RVSIP_TRANSC_STATE_CLIENT_GEN_PROCEEDING == pTransc->eState ||
            RVSIP_TRANSC_STATE_CLIENT_CANCEL_PROCEEDING == pTransc->eState)
        {
            /* Uac General Proceeding state. Retransmission interval is T2. */
            *mainTimerInterval = pTransc->pTimers->T2Timeout;
        }
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
        else if(RVSIP_TRANSC_STATE_CLIENT_CANCEL_SENT == pTransc->eState ||
                RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT == pTransc->eState)
        {
            *mainTimerInterval = ((pTransc->pMgr)->initialGeneralRequestTimeout)*
                SipCommonPower(pTransc->retransmissionsCount);
            if(*mainTimerInterval > pTransc->pTimers->T2Timeout)
            {
                *mainTimerInterval = pTransc->pTimers->T2Timeout;
            }
        }
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

        else 
        {
            /* Retransmission timer is T1*2^n for n=retransmissions-1, or
               T2 if T1*2^n is larger than T2. */
            *mainTimerInterval = (pTransc->pTimers->T1Timeout)*
                                   SipCommonPower(pTransc->retransmissionsCount);
            if(RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING == pTransc->eState &&
               pTransc->retransmissionsCount == pTransc->pTimers->maxInviteRequestRetransmissions-1)
            {
                /* "the client transaction MUST start timer B with a value of 64*T1 seconds 
                    (Timer B controls transaction timeouts)."
                    -- we use a single timer (main timer) for both timer A and B.
                    A is set to T1*2^n, so after 7 times (the standard number of retransmissions)
                    we need to set the timer to one more T1, to reach the 64*T1 value:
                    main timer values: T1,2T1,4T1,8T1,16T1,32T1,T1 ==> 1+2+4+8+16+32+1 = 64. */
                *mainTimerInterval = (pTransc->pTimers->T1Timeout);
            }
            if ((RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING != pTransc->eState) &&
                (*mainTimerInterval > (pTransc->pTimers->T2Timeout)))
            {
#ifndef RV_SIP_PRIMITIVES
                if (RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT != pTransc->eState)
                  {
#endif
                       *mainTimerInterval = (pTransc->pTimers->T2Timeout);
#ifndef RV_SIP_PRIMITIVES
                  }
#endif

            }
        }
        *bSetMainTimer = RV_TRUE;
        /* Increment the retransmissions count */
        pTransc->retransmissionsCount =
                                       pTransc->retransmissionsCount + 1;
        *bSetLongReqestTimer = RV_FALSE;
        return;
    }

    /*---------------------------------------------------
          sending the message for the first time
     ----------------------------------------------------*/
    switch (pTransc->eState)
    {
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING:
        *bSetLongReqestTimer = RV_FALSE;
        *mainTimerInterval   = (pTransc->pTimers->T1Timeout);
        *bSetMainTimer       = RV_TRUE;
        if(*mainTimerInterval == 0)
        {
            *bSetMainTimer       = RV_FALSE;
            *longTimerInterval   = OBJ_TERMINATION_DEFAULT_TCP_TIMEOUT;
            *bSetLongReqestTimer = RV_TRUE;
        }
        break;
    case RVSIP_TRANSC_STATE_CLIENT_CANCEL_SENT:
    case RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT:
        *longTimerInterval   = (pTransc->pTimers->generalRequestTimeoutTimeout);
        *bSetLongReqestTimer = RV_TRUE;

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
        *mainTimerInterval = ((pTransc->pMgr)->initialGeneralRequestTimeout);
#else /* !defined(UPDATED_BY_SPIRENT) */
        *mainTimerInterval   = (pTransc->pTimers->T1Timeout);
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
        *bSetMainTimer       = RV_TRUE;
        break;
    case RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT:
        if(pTransc->responseCode >= 200 && pTransc->responseCode < 300)
        {
            if(pTransc->pMgr->bOldInviteHandling == RV_FALSE) /*new invite*/
            {
				/*The transaction has to remain in the hash to absorb 
				retransmission. However, it should not handle the incoming ack*/
				RvMutexLock(&pTransc->pMgr->hLock,pTransc->pMgr->pLogMgr);
				pTransc->bAllowAckHandling = RV_FALSE;
				RvMutexUnlock(&pTransc->pMgr->hLock,pTransc->pMgr->pLogMgr);

                *bSetLongReqestTimer = RV_FALSE;
                *bSetMainTimer       = RV_TRUE;
                if(pTransc->eOwnerType != SIP_TRANSACTION_OWNER_CALL)
                {
					*mainTimerInterval   = pTransc->pTimers->inviteLingerTimeout;

					/*
                    *mainTimerInterval   = OBJ_TERMINATION_DEFAULT_UDP_TIMEOUT;
                    TransactionMgrHashRemove((RvSipTranscHandle)pTransc);
					*/
                }
                else
                {
                    /* The transaction remains in the hash until call-leg terminates it.
                       Call-leg will terminate it on receiving the ACK request, or when
                       it's 2xx timer expires.
                       this way the transaction can catch the invite request retransmissions,
                       but will not handle the ACK on the 2xx. */
                    *bSetMainTimer           = RV_FALSE;
                }
            }
            else
            {
                *mainTimerInterval   = (pTransc->pTimers->T1Timeout);
                *longTimerInterval   = (pTransc->pTimers->T1Timeout)*64;
                *bSetLongReqestTimer = RV_TRUE;
                *bSetMainTimer       = RV_TRUE;
            }
        }
        else
        {
            *mainTimerInterval   = (pTransc->pTimers->T1Timeout);
            *longTimerInterval   = (pTransc->pTimers->T1Timeout)*64;
            *bSetLongReqestTimer = RV_TRUE;
            *bSetMainTimer       = RV_TRUE;
        }
        break;
#ifndef RV_SIP_PRIMITIVES
    case RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT:
        *longTimerInterval   = (pTransc->pTimers->T1Timeout)*64;
        *bSetLongReqestTimer = RV_TRUE;
        *mainTimerInterval   = (pTransc->pTimers->T1Timeout);
        *bSetMainTimer       = RV_TRUE;
        break;
#endif
    default:
        *bSetLongReqestTimer = RV_FALSE;
        *bSetMainTimer       = RV_FALSE;
        return;
    }

    pTransc->retransmissionsCount = 1;
    if((*bSetMainTimer == RV_TRUE) && (*mainTimerInterval == 0)) /* If the timer is 0 then the message will not be retransmitted */
    {
        *bSetMainTimer = RV_FALSE;
    }
    if((*bSetLongReqestTimer == RV_TRUE) && (*longTimerInterval == 0))
    {
        *longTimerInterval = OBJ_TERMINATION_DEFAULT_TCP_TIMEOUT;
    }
    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "UDPTimersDecide - Transaction 0x%p: state=%s, Decisions: (*bSetMainTimer=%d, *mainTimerInterval=%d, *bSetLongReqestTimer=%d, *longTimerInterval=%d",
               pTransc, TransactionGetStateName(pTransc->eState), *bSetMainTimer, *mainTimerInterval, *bSetLongReqestTimer, *longTimerInterval));

}


/***************************************************************************
 * HandleTimeoutAtInviteProceedingState
 * ------------------------------------------------------------------------
 * General: Called when a transaction reached invite proceeding timeout.
 *          If enableInviteProceedingTimeoutState flag is checked then the transaction state
 *          will be changed to state "Invite-Proceeding-TimeOut", otherwise the transaction
 *          will terminate.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction that reached proceeding timeout
 * Output:(-)
 ***************************************************************************/
static void RVCALLCONV HandleTimeoutAtInviteProceedingState(IN Transaction *pTransc)
{
    if(pTransc->pMgr->enableInviteProceedingTimeoutState == RV_TRUE)
    {
        /* Request Timeout Timer */
        TransactionStateUACInviteProceedingTimeOut(pTransc);
    }
    else
    {
        /* Terminate timer*/
        TransactionTerminate(RVSIP_TRANSC_REASON_TIME_OUT,pTransc);
    }
}


#endif /*#ifndef RV_SIP_LIGHT*/

