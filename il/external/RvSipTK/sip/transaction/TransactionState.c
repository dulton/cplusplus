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
 *                              <TransactionState.c>
 *
 *    This file defines one function to each of the transactions states. This
 *    function is called when the state changes. The function will change the
 *    transaction's state, call EvStateChanged callback, and set the timer according
 *  to the new state.
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

#include <string.h>

#include "TransactionState.h"
#include "RV_SIP_DEF.h"
#include "RvSipPartyHeader.h"
#include "RvSipMsg.h"
#include "TransactionTimer.h"
#include "TransactionMgrObject.h"
#include "TransactionControl.h"
#include "TransactionCallBacks.h"
#include "_SipCommonCore.h"
#include "_SipCommonUtils.h"


/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * TransactionStateSrvGeneralReqRecvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateSrvGeneralReqRecvd(
                            IN  RvSipTransactionStateChangeReason eReason,
                            IN  Transaction                       *pTransc)

{

    /* Release timer */
    TransactionTimerMainTimerRelease(pTransc);
    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD,
                             eReason);

    return RV_OK;
}


/***************************************************************************
 * TransactionStateSrvGeneralFinalRespSent
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateSrvGeneralFinalRespSent(
                        IN  RvSipTransactionStateChangeReason eReason,
                        IN  Transaction                       *pTransc)
{
    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT,
                             eReason);

    /* If the message was sent in UDP and the linger is 0 the transaction
       should terminate.*/
    if ((pTransc->pTimers->genLingerTimeout == 0) &&
        (pTransc->eTranspType==RVSIP_TRANSPORT_UDP))
    {
        TransactionTerminate(RVSIP_TRANSC_REASON_NORMAL_TERMINATION,
            pTransc);
    }

    return RV_OK;
}


/***************************************************************************
 * TransactionStateSrvInviteReqRecvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateSrvInviteReqRecvd(
                 IN  RvSipTransactionStateChangeReason eReason,
                 IN  Transaction                       *pTransc)
{

    /* Release timer */
    TransactionTimerMainTimerRelease(pTransc);
    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD,
                             eReason);

    return RV_OK;
}


/***************************************************************************
 * TransactionStateSrvInviteFinalRespSent
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateSrvInviteFinalRespSent(
                 IN  RvSipTransactionStateChangeReason eReason,
                 IN  Transaction                       *pTransc)
{
    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT,
                             eReason);

    return RV_OK;
}
/***************************************************************************
 * TransactionStateProxyInvite2xxRespSent
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer
 *            according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateProxyInvite2xxRespSent(
                            IN  RvSipTransactionStateChangeReason eReason,
                            IN  Transaction                       *pTransc)
{
    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_SERVER_INVITE_PROXY_2XX_RESPONSE_SENT,
                             eReason);


    if (pTransc->pMgr->proxy2xxSentTimer == 0 &&
        pTransc->eTranspType==RVSIP_TRANSPORT_UDP)
    {
        TransactionTerminate(RVSIP_TRANSC_REASON_NORMAL_TERMINATION, pTransc);
    }

    return RV_OK;
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * TransactionStateSrvInviteReliableProvRespSent
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateSrvInviteReliableProvRespSent(
                            IN  RvSipTransactionStateChangeReason eReason,
                            IN  Transaction                       *pTransc)
{
    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT,
                             eReason);

    return RV_OK;
}
#endif /* RV_SIP_PRIMITIVES */


/***************************************************************************
 * TransactionStateUacInviteCancelling
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacInviteCancelling(
                            IN  RvSipTransactionStateChangeReason eReason,
                            IN  Transaction                       *pTransc)
{
    RvUint32 timerInterval;
    RvStatus rv;


    /*release transaction timers*/
    TransactionTimerReleaseAllTimers(pTransc);


    timerInterval = (pTransc->pTimers->cancelInviteNoResponseTimeout);

    rv = TransactionTimerMainTimerStart(pTransc, timerInterval, "TransactionStateUacInviteCancelling");
    if (rv != RV_OK)
    {
        RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "TransactionStateUacInviteCancelling - Transaction 0x%p: No timer is available",
                  pTransc));
        TransactionTerminate(RVSIP_TRANSC_REASON_RAN_OUT_OF_TIMERS,
                             pTransc);
        return RV_ERROR_OUTOFRESOURCES;
    }
    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_CLIENT_INVITE_CANCELLING,
                             eReason);


    return RV_OK;
}

/***************************************************************************
 * TransactionStateUacGeneralCancelling
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacGeneralCancelling(
                            IN  RvSipTransactionStateChangeReason eReason,
                            IN  Transaction                       *pTransc)
{
    RvUint32 timerInterval;
    RvStatus rv;

    /*release transaction timers*/
    TransactionTimerReleaseAllTimers(pTransc);

    timerInterval = (pTransc->pMgr->cancelGeneralNoResponseTimeout);

    rv = TransactionTimerMainTimerStart(pTransc, timerInterval, "TransactionStateUacGeneralCancelling");
    if (rv != RV_OK)
    {
        RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "TransactionStateUacGeneralCancelling - Transaction 0x%p: No timer is available",
                  pTransc));
        TransactionTerminate(RVSIP_TRANSC_REASON_RAN_OUT_OF_TIMERS,
                             pTransc);
        return RV_ERROR_OUTOFRESOURCES;
    }
    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_CLIENT_GEN_CANCELLING,
                             eReason);

    return RV_OK;
}

/***************************************************************************
 * TransactionStateUacGeneralReqMsgSent
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 *Output: (-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacGeneralReqMsgSent(
                 IN  RvSipTransactionStateChangeReason   eReason,
                 IN  Transaction                         *pTransc)
{
    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT,
                             eReason);
    return RV_OK;
}
/***************************************************************************
 * TransactionStateUacCancelReqMsgSent
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 *Output: (-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacCancelReqMsgSent(
                           IN  RvSipTransactionStateChangeReason eReason,
                           IN  Transaction                       *pTransc)
{
    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_CLIENT_CANCEL_SENT,
                             eReason);

    return RV_OK;
}




/***************************************************************************
 * TransactionStateUacGeneralProceeding
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 *Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacGeneralProceeding(
                 IN  RvSipTransactionStateChangeReason   eReason,
                 IN  Transaction                         *pTransc)
{
    RvUint32 timerInterval;
    RvStatus rv;

    /* Release timer */
    TransactionTimerMainTimerRelease(pTransc);



    /* This state requires a timer to be set, according to the time defined
       in the state machine. Set the transaction's timer accordingly */
    switch (pTransc->eTranspType)
    {
    case RVSIP_TRANSPORT_TCP:
    case RVSIP_TRANSPORT_TLS:
    case RVSIP_TRANSPORT_SCTP:
        timerInterval = 0;
        break;
    case RVSIP_TRANSPORT_UDP:
    case RVSIP_TRANSPORT_UNDEFINED:
        timerInterval = (pTransc->pTimers->T2Timeout);
        break;
    default:
        RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "TransactionStateUacGeneralProceeding - Transaction 0x%p: Undefined transport.",
                  pTransc));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if(timerInterval != 0)
    {
        
        rv = TransactionTimerMainTimerStart(pTransc, timerInterval, "TransactionStateUacGeneralProceeding");
        if (rv != RV_OK)
        {
            RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "TransactionStateUacGeneralProceeding - Transaction 0x%p: No timer is available",
                pTransc));
            TransactionTerminate(RVSIP_TRANSC_REASON_RAN_OUT_OF_TIMERS,
                pTransc);
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_CLIENT_GEN_PROCEEDING,
                             eReason);
    return RV_OK;
}

/***************************************************************************
 * TransactionStateUacGeneralFinalResponseRcvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacGeneralFinalResponseRcvd(
                        IN  RvSipTransactionStateChangeReason eReason,
                        IN  Transaction                       *pTransc)
{
    RvUint32 timerInterval = 0;
    RvStatus rv            = RV_OK;

    /*release transaction timers*/
    TransactionTimerReleaseAllTimers(pTransc);
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    /* In case of 503 responce, send failure callback
    should be applied  - we save the value for future use*/
    /*if the state is cancelling handle 503 as a regular reject and done move
    to msg send failure*/
    if ((pTransc->responseCode == 503)   &&
        (pTransc->eMethod != SIP_TRANSACTION_METHOD_CANCEL) &&
        (pTransc->eState != RVSIP_TRANSC_STATE_CLIENT_GEN_CANCELLING))
    {
        rv = TransactionHandleMsgSentFailureEvent(pTransc,RVSIP_TRANSC_REASON_503_RECEIVED);
        return rv; /*in msg sent failure we do not set the response timer*/
    }
    else
    {
#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */

    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_CLIENT_GEN_FINAL_RESPONSE_RCVD,
                             eReason);

    if(pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
    {
       RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionStateUacGeneralFinalResponseRcvd - Transaction 0x%p: was terminated from callback - timers are not set",
                  pTransc));

        return RV_OK;
    }


#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    }
#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */

    switch (pTransc->eTranspType)
    {
    case RVSIP_TRANSPORT_TCP:
    case RVSIP_TRANSPORT_TLS:
    case RVSIP_TRANSPORT_SCTP:
        timerInterval = 0;
        break;
    case RVSIP_TRANSPORT_UDP:
    case RVSIP_TRANSPORT_UNDEFINED:
        timerInterval = (pTransc->pTimers->T4Timeout);
        break;
    default:
        RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "TransactionStateUacGeneralFinalResponseRcvd - Transaction 0x%p: Undefined transport.",
                  pTransc));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if((timerInterval == 0))
    {
        TransactionTerminate(RVSIP_TRANSC_REASON_NORMAL_TERMINATION, pTransc);
        return RV_OK;
     }

    rv = TransactionTimerMainTimerStart(pTransc, timerInterval, "TransactionStateUacFinalResponseRcvd");
    if (rv != RV_OK)
    {
        RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionStateUacGeneralFinalResponseRcvd - Transaction 0x%p: No timer is available",
                  pTransc));
        TransactionTerminate(RVSIP_TRANSC_REASON_RAN_OUT_OF_TIMERS,
                             pTransc);
        return RV_ERROR_OUTOFRESOURCES;
    }

    return RV_OK;
}
/***************************************************************************
 * TransactionStateUacCancelFinalResponseRcvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacCancelFinalResponseRcvd(
                 IN  RvSipTransactionStateChangeReason eReason,
                 IN  Transaction                       *pTransc)
{
    RvUint32 timerInterval;
    RvStatus rv;

    /*release transaction timers*/
    TransactionTimerReleaseAllTimers(pTransc);

    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_CLIENT_CANCEL_FINAL_RESPONSE_RCVD,
                             eReason);

    if(pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
    {
       RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionStateUacCancelFinalResponseRcvd - Transaction 0x%p: was terminated from callback - timers are not set",
            pTransc));

        return RV_OK;
    }

    switch (pTransc->eTranspType)
    {
    case RVSIP_TRANSPORT_TCP:
    case RVSIP_TRANSPORT_TLS:
    case RVSIP_TRANSPORT_SCTP:
         timerInterval = 0;
        break;
    case RVSIP_TRANSPORT_UDP:
    case RVSIP_TRANSPORT_UNDEFINED:
        timerInterval = (pTransc->pTimers->T4Timeout);
        break;
    default:
        RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "TransactionStateUacCancelFinalResponseRcvd - Transaction 0x%p: Undefined transport.",
                  pTransc));
        return RV_ERROR_ILLEGAL_ACTION;
    }


    if(timerInterval == 0)
    {
        TransactionTerminate(RVSIP_TRANSC_REASON_NORMAL_TERMINATION, pTransc);
        return RV_OK;
    }
    rv = TransactionTimerMainTimerStart(pTransc, timerInterval, "TransactionStateUacCancelFinalResponseRcvd");
    if (rv != RV_OK)
    {
        RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionStateUacCancelFinalResponseRcvd - Transaction 0x%p: No timer is available",
                  pTransc));
        TransactionTerminate(RVSIP_TRANSC_REASON_RAN_OUT_OF_TIMERS,
                             pTransc);
        return RV_ERROR_OUTOFRESOURCES;
    }

    return RV_OK;
}



/***************************************************************************
 * TransactionStateUacCancelProceeding
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacCancelProceeding(
                 IN  RvSipTransactionStateChangeReason   eReason,
                 IN  Transaction                         *pTransc)
{
    RvUint32 timerInterval;
    RvStatus rv;

    /* Release timer */
    TransactionTimerMainTimerRelease(pTransc);


    /* This state requires a timer to be set, according to the time defined
       in the state machine. Set the transaction's timer accordingly */
    switch (pTransc->eTranspType)
    {
    case RVSIP_TRANSPORT_TCP:
    case RVSIP_TRANSPORT_TLS:
    case RVSIP_TRANSPORT_SCTP:
        timerInterval = 0;
        break;
    case RVSIP_TRANSPORT_UDP:
    case RVSIP_TRANSPORT_UNDEFINED:
        timerInterval = (pTransc->pTimers->T2Timeout);
        break;
    default:
           RvLogExcep((pTransc->pMgr)->pLogSrc,((pTransc->pMgr)->pLogSrc,
                  "TransactionStateUacCancelProceeding - Transaction 0x%p: Undefined transport.",
                  pTransc));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if(timerInterval != 0)
    {
        
        rv = TransactionTimerMainTimerStart(pTransc, timerInterval, "TransactionStateUacCancelProceeding");
        if (rv != RV_OK)
        {
            RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionStateUacCancelProceeding - Transaction 0x%p: No timer is available",
                pTransc));
            TransactionTerminate(RVSIP_TRANSC_REASON_RAN_OUT_OF_TIMERS,
                pTransc);
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_CLIENT_CANCEL_PROCEEDING,
                             eReason);

    return RV_OK;
}

/***************************************************************************
 * TransactionStateUacInviteCalling
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 *Output: (-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacInviteCalling(
                 IN  RvSipTransactionStateChangeReason eReason,
                 IN  Transaction                       *pTransc)
{
    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING,
                             eReason);

    return RV_OK;
}


/***************************************************************************
 * TransactionStateUacInviteProceeding
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacInviteProceeding(
                 IN  RvSipTransactionStateChangeReason eReason,
                 IN  Transaction                       *pTransc)
{
    RvUint32 timerInterval;
    RvStatus rv;

    /*release transaction timers*/
    TransactionTimerReleaseAllTimers(pTransc);
    if ((0 != pTransc->pTimers->provisionalTimeout))
    {
        /* This state requires a timer to be set, according to the time defined
           in the state machine. Set the transaction's timer accordingly.
           If provisional = 0, it means that this state is not bounded in time. */
        timerInterval = (pTransc->pTimers->provisionalTimeout);
        rv = TransactionTimerMainTimerStart(pTransc, timerInterval, "TransactionStateUacInviteProceeding");
        if (rv != RV_OK)
        {
               RvLogExcep((pTransc->pMgr)->pLogSrc,((pTransc->pMgr)->pLogSrc,
                    "TransactionStateUacInviteProceeding - Transaction 0x%p: No timer is available",
                    pTransc));
            TransactionTerminate(RVSIP_TRANSC_REASON_RAN_OUT_OF_TIMERS,
                                pTransc);
            return RV_ERROR_OUTOFRESOURCES;
        }
    }

    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING,
                             eReason);

    if(pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                   "TransactionStateUacInviteProceeding - Transaction 0x%p: was terminated from callback - timers are not set",
                   pTransc));
        return RV_OK;
    }

    return RV_OK;
}


/***************************************************************************
 * TransactionStateUacInviteFinalRespRecvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacInviteFinalRespRecvd(
                 IN  RvSipTransactionStateChangeReason eReason,
                 IN  Transaction                       *pTransc)
{
    /*release transaction timers*/
    TransactionTimerReleaseAllTimers(pTransc);
    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "TransactionStateUacInviteFinalRespRecvd - Transaction 0x%p:  state changed to \"Uac Invite final response received\"",
              pTransc));

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    /* In case of 503 response, send failure callback
    should be applied  - we save the value for future use*/
    /*if the state is cancelling handle 503 as a regular reject and done move
    to msg send failure*/
    if (pTransc->responseCode == 503 &&
        pTransc->eState != RVSIP_TRANSC_STATE_CLIENT_INVITE_CANCELLING)
    {

        return TransactionHandleMsgSentFailureEvent(pTransc,RVSIP_TRANSC_REASON_503_RECEIVED);

    }
    /* the stuff that was here is done inside TransactionChangeStateEv() */
    else
    {
#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */

        TransactionChangeState(pTransc,
                                 RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD,
                                 eReason);

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    }
#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */

    if (200 <= pTransc->responseCode && 300 > pTransc->responseCode &&
        pTransc->pMgr->bOldInviteHandling == RV_FALSE)
    {
        TransactionTerminate(RVSIP_TRANSC_REASON_NORMAL_TERMINATION,pTransc);
    }
    
    return RV_OK;
}

/***************************************************************************
 * TransactionStateProxyInvite2xxRespRcvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer according
 *            to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateProxyInvite2xxRespRcvd(
                        IN  RvSipTransactionStateChangeReason eReason,
                        IN  Transaction                       *pTransc)

{
    RvStatus rv;

    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_CLIENT_INVITE_PROXY_2XX_RESPONSE_RCVD,
                             eReason);

    if(pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
    {
       RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionStateProxyInvite2xxRespRcvd - Transaction 0x%p: was terminated from callback - timers are not set",
            pTransc));

        return RV_OK;
    }


    /* if the proxy didn't received 2xx message before, set the transaction termination timer*/
    if(RV_TRUE != pTransc->bIsProxySend2xxTimer)
    {   
		/*A condition for UDP was removed since the transaction has to terminate anyway*/
        if (pTransc->pMgr->proxy2xxRcvdTimer == 0/* && pTransc->eTranspType==RVSIP_TRANSPORT_UDP*/)
        {
            TransactionTerminate(RVSIP_TRANSC_REASON_NORMAL_TERMINATION, pTransc);
            return RV_OK;
        }

        /* Release timer */
        TransactionTimerMainTimerRelease(pTransc);

        /* Set proxy timer to 2xx received timer*/
        pTransc->retransmissionsCount = 1;
        rv = TransactionTimerMainTimerStart(pTransc, (pTransc->pMgr)->proxy2xxRcvdTimer,
                                            "TransactionStateProxyInvite2xxRespRcvd");
        if (rv != RV_OK)
        {
            RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "TransactionStateProxyInvite2xxRespRcvd - Transaction 0x%p: No timer is available",
                pTransc));
            return RV_ERROR_OUTOFRESOURCES;
        }
        pTransc->bIsProxySend2xxTimer = RV_TRUE;
    }

    return RV_OK;
}
/***************************************************************************
 * TransactionStateUacInviteAckSent
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateUacInviteAckSent(
                 IN  RvSipTransactionStateChangeReason eReason,
                 IN  Transaction                       *pTransc)
{
    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_CLIENT_INVITE_ACK_SENT,
                             eReason);


    if (pTransc->pTimers->inviteLingerTimeout == 0 &&
        pTransc->eTranspType==RVSIP_TRANSPORT_UDP)
    {
        TransactionTerminate(RVSIP_TRANSC_REASON_NORMAL_TERMINATION,
            pTransc);
    }

    return RV_OK;
}
/***************************************************************************
 * TransactionStateServerInviteAckRcvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateServerInviteAckRcvd(
                           IN  RvSipTransactionStateChangeReason eReason,
                           IN  Transaction                       *pTransc)
{
    RvUint32 timerInterval;
    RvStatus rv;

    /*release transaction timers*/
    TransactionTimerReleaseAllTimers(pTransc);

    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_SERVER_INVITE_ACK_RCVD,
                             eReason);

    if(pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
    {
       RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionStateServerInviteAckRcvd - Transaction 0x%p: was terminated from callback - timers are not set",pTransc));

        return RV_OK;
    }

    switch (pTransc->eTranspType)
    {
    case RVSIP_TRANSPORT_TCP:
    case RVSIP_TRANSPORT_TLS:
    case RVSIP_TRANSPORT_SCTP:
        timerInterval = 0;
        break;
    case RVSIP_TRANSPORT_UDP:
    case RVSIP_TRANSPORT_UNDEFINED:
        timerInterval = (pTransc->pTimers->T4Timeout);
        break;
    default:
           RvLogExcep((pTransc->pMgr)->pLogSrc,((pTransc->pMgr)->pLogSrc,
                  "TransactionStateServerInviteAckRcvd - Transaction 0x%p: Undefined transport.",
                  pTransc));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if(timerInterval == 0)
    {
        TransactionTerminate(RVSIP_TRANSC_REASON_NORMAL_TERMINATION, pTransc);
        return RV_OK;
    }

    rv = TransactionTimerMainTimerStart(pTransc, timerInterval, "TransactionStateServerInviteAckRcvd");   
    if (rv != RV_OK)
    {
        RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionStateServerInviteAckRcvd - Transaction 0x%p: No timer is available",
                  pTransc));
        TransactionTerminate(RVSIP_TRANSC_REASON_RAN_OUT_OF_TIMERS,
                             pTransc);
        return RV_ERROR_OUTOFRESOURCES;
    }
    return RV_OK;
}

/***************************************************************************
 * TransactionStateSrvGeneralReqRecvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:TransactionStateSrvCancelReqRecvd
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateSrvCancelReqRecvd(
                           IN  RvSipTransactionStateChangeReason eReason,
                           IN  Transaction                       *pTransc,
                           IN  Transaction                       *pCancelledTransc)

{

    RvStatus            rv;
    RvUint16            responseCode;

    RvBool                bIsProxy = pTransc->bIsProxy;


    /*respond to the cancel transcation*/
    if(pCancelledTransc == NULL)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionStateSrvCancelReqRecvd - the cancelled transaction was not found"));
        responseCode = 481;  /*the cancelled transaction was not found*/
    }
    else
    {
#ifndef RV_SIP_PRIMITIVES
        if(pCancelledTransc->eMethod == SIP_TRANSACTION_METHOD_PRACK)
        {
            responseCode = 405;
        }
        else
#endif
        {
            pTransc->hCancelTranscPair = (RvSipTranscHandle)pCancelledTransc;
            responseCode = 200;  /*the cancelled transaction was found*/
        }
    }

    /* if the cancel transaction is a proxy transaction and mo match non cancel transaction
       inform the application */
    if(RV_TRUE == pTransc->bIsProxy)
    {
        TransactionChangeState(pTransc,
                                 RVSIP_TRANSC_STATE_SERVER_CANCEL_REQUEST_RCVD,
                                 eReason);
        return RV_OK;

    }

    rv = TransactionControlSendResponse(pTransc,responseCode,NULL,
        RVSIP_TRANSC_REASON_TRANSACTION_CANCELED,
        RV_FALSE);

    if(rv != RV_OK)
    {
        return rv;
    }

    if(pCancelledTransc != NULL && responseCode == 200 &&
       (pCancelledTransc->eState == RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD ||
       pCancelledTransc->eState == RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD
#ifndef RV_SIP_PRIMITIVES
       ||
       pCancelledTransc->eState == RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT
       ||
       pCancelledTransc->eState == RVSIP_TRANSC_STATE_SERVER_INVITE_PRACK_COMPLETED
#endif /*#ifndef RV_SIP_PRIMITIVES*/
       ))
     {
        /*notify the owner that the transaction is cancelled*/
       if (NULL != pCancelledTransc->pEvHandlers->pfnEvTranscCancelled)
       {
           rv = TransactionCallbackCancelledEv(pCancelledTransc);
           if (rv != RV_OK)
           {
               RvLogError(pCancelledTransc->pMgr->pLogSrc ,(pCancelledTransc->pMgr->pLogSrc ,
                        "TransactionStateSrvCancelReqRecvd - pCancelledTransc 0x%p - failed in TransactionCallbackCancelledEv. rv=%d",
                         pCancelledTransc, rv));
               return rv;
           }
           /* in the callback, application may sent a final response on the invite transaction,
              in this case we do not want to continue and send 487... */
           if(pCancelledTransc->eState == RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT ||
              pCancelledTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
           {
               RvLogDebug(pCancelledTransc->pMgr->pLogSrc ,(pCancelledTransc->pMgr->pLogSrc ,
                   "TransactionStateSrvCancelReqRecvd - pCancelledTransc 0x%p - transc state is %s. do not send 487",
                   pCancelledTransc, TransactionGetStateName(pCancelledTransc->eState)));
               return RV_OK;
           }

           if(bIsProxy == RV_TRUE)
           {
               return RV_OK;
           }
       }

       /*respond to the cancelled transaction with 487, or let the application send
         the 487, if manual response flag is on*/
#ifndef RV_SIP_PRIMITIVES
       if (RV_TRUE == pCancelledTransc->pMgr->manualBehavior)
       {
           /* if manual response is on the application will call reject */
           return RV_OK;
       }
#endif /*#ifndef RV_SIP_PRIMITIVES */
       /* Destruct the outbound message if the application has already created it
       but did not send it yet */
       if (NULL != pCancelledTransc->hOutboundMsg)
       {
            RvSipMsgDestruct(pCancelledTransc->hOutboundMsg);
            pCancelledTransc->hOutboundMsg = NULL;
       }

       rv = TransactionControlSendResponse(pCancelledTransc,487,NULL,
                                            RVSIP_TRANSC_REASON_TRANSACTION_CANCELED,
                                            RV_FALSE);
       return rv;
    }
    return RV_OK;

}


/***************************************************************************
 * TransactionStateServerCancelFinalRespSent
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateServerCancelFinalRespSent(
                        IN  RvSipTransactionStateChangeReason eReason,
                        IN  Transaction                       *pTransc)
{

    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_SERVER_CANCEL_FINAL_RESPONSE_SENT,
                             eReason);

    if (pTransc->pTimers->genLingerTimeout == 0 &&
        pTransc->eTranspType==RVSIP_TRANSPORT_UDP)
    {
        TransactionTerminate(RVSIP_TRANSC_REASON_NORMAL_TERMINATION, pTransc);
    }
    return RV_OK;
}

/***************************************************************************
 * TransactionStateUACInviteProceedingTimeOut
 * ------------------------------------------------------------------------
 * General: This function is called when the UAC Invite transaction reached timeout
 *          in the proceeding state.
 *          This function is called only if the flag enableInviteProceedingTimeoutState is TRUE.
 * Return Value: RV_OK - The transaction was terminated successfully.
 *                 RV_ERROR_NULLPTR - The pointer to the transaction is a NULL
 *                                 pointer.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTransc - the transaction that reached request timeout
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateUACInviteProceedingTimeOut(IN Transaction *pTransc)
{
    /*release transaction timers*/
    TransactionTimerReleaseAllTimers(pTransc);

    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING_TIMEOUT,
                             RVSIP_TRANSC_REASON_TIME_OUT);

    return RV_OK;
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * TransactionStateSrvPrackReqRecvd
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 *        hPrackMsg - Handle to the PRACK message.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateSrvPrackReqRecvd(
                           IN  RvSipTransactionStateChangeReason  eReason,
                           IN  Transaction                       *pTransc,
                           IN  RvSipMsgHandle                     hPrackMsg)

{

    RvStatus rv;
    Transaction* pPrackPair;
    RvUint16 responseCode;

    RV_UNUSED_ARG(hPrackMsg);
    /* Release timer */
    TransactionTimerMainTimerRelease(pTransc);

    pPrackPair = (Transaction*)pTransc->hPrackParent;
    /*respond to the prack transcation*/
    if(pTransc->unsupportedList != UNDEFINED)
    {
        responseCode = 420;
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionStateSrvPrackReqRecvd - Transaction 0x%p: unsupported extension requires - responding with 420",
                  pTransc));

    }
    else if(pTransc->pMgr->status100rel != RVSIP_TRANSC_100_REL_SUPPORTED)
    {
        responseCode = 405; /*prack method is not allowed*/
    }
    else if(pPrackPair == NULL)
    {
        responseCode = 481;  /*the prack pair transaction was not found*/
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionStateSrvPrackReqRecvd - Transaction 0x%p: no prack matching transaction - responding with 481",
                  pTransc));

    }
    else
    {
        responseCode = 200;  /*the prack pair transaction was found*/
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                     "TransactionStateSrvPrackReqRecvd - Transaction 0x%p: found a prack matching transc 0x%p",
                      pTransc, pPrackPair));

    }

    rv = TransactionStateSrvGeneralReqRecvd(eReason, pTransc);
    if(rv != RV_OK)
    {
        return rv;
    }

    /* the PRACK might have been already rejected in the changeState callback.
       must also check the function state here... */
    if((pTransc->pMgr->manualPrack == RV_FALSE || responseCode >= 300) &&
        pTransc->eState == RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
             "TransactionStateSrvPrackReqRecvd - Transaction 0x%p: answer with %d response",
              pTransc, responseCode));
        rv = TransactionControlSendResponse(pTransc,responseCode,NULL,
            RVSIP_TRANSC_REASON_TRANSACTION_COMMAND,
            RV_FALSE);

        if(rv != RV_OK)
        {
            return rv;
        }
    }
    return RV_OK;

}
/***************************************************************************
 * TransactionStateSrvInvitePrackCompeleted
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionStateSrvInvitePrackCompeleted(
                 IN  RvSipTransactionStateChangeReason eReason,
                 IN  Transaction                       *pTransc)
{

    /*release transaction timers*/
    TransactionTimerReleaseAllTimers(pTransc);
    pTransc->retransmissionsCount = 0;

    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_SERVER_INVITE_PRACK_COMPLETED,
                             eReason);

    return RV_OK;
}

/***************************************************************************
 * TransactionStateServerPrackFinalRespSent
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state. Set the transaction timer or
 *            retransmission timer according to the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eReason - The reason for the state change.
 *        pTransc - The transaction in which the state has
 *                                     changed.
 * output:(-)
 ***************************************************************************/
/*RvStatus RVCALLCONV TransactionStateServerPrackFinalRespSent(
                        IN  RvSipTransactionStateChangeReason eReason,
                        IN  Transaction                       *pTransc)
{

    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT,
                             eReason);

    if (pTransc->pMgr->genLinger == 0 &&
        pTransc->eTranspType==RVSIP_TRANSPORT_UDP)
    {
        TransactionTerminate(RVSIP_TRANSC_REASON_NORMAL_TERMINATION,
                             pTransc);
    }
    return RV_OK;
}*/

#endif /* #ifndef RV_SIP_PRIMITIVES  */
#endif /*#ifndef RV_SIP_LIGHT*/

