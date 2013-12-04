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
 *                              <TransactionMsgReceived.c>
 *
 *    This file defines the transaction behavior on the receipt of a message.
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

#include "TransactionMsgReceived.h"
#include "TransactionObject.h"
#include "TransactionTimer.h"
#include "TransactionState.h"
#include "_SipTransaction.h"
#include "_SipTransactionTypes.h"
#include "TransactionTransport.h"
#include "TransactionAuth.h"
#include "_SipPartyHeader.h"
#include "_SipOtherHeader.h"
#include "RvSipRSeqHeader.h"
#include "TransactionControl.h"
#include "RvSipOtherHeader.h"
#include "TransactionCallBacks.h"
#include "_SipCommonUtils.h"
#include "_SipTransmitter.h"
#include "_SipMsg.h"
#ifdef RV_SIP_IMS_ON
#include "TransactionSecAgree.h"
#endif /* RV_SIP_IMS_ON */

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/


/*------------------------------------------------------------------------
                  H A N D L E   R E Q U E S T S
 -------------------------------------------------------------------------*/

static RvStatus RVCALLCONV HandleRequestMsg(IN  Transaction    *pTransc,
                                             IN  RvSipMsgHandle hMsg);


static RvStatus RVCALLCONV HandleRequestInInitialState(
                                                        IN  Transaction    *pTransc,
                                                        IN  RvSipMsgHandle hMsg);

static RvStatus RVCALLCONV HandleInviteInInitialState(IN  Transaction    *pTransc,
                                                       IN  RvSipMsgHandle hMsg);

static RvStatus RVCALLCONV HandleGeneralInInitialState(
                                      IN  Transaction       *pTransc,
                                      IN  RvSipMsgHandle    hMsg);


static RvStatus RVCALLCONV HandleCancelInInitialState(
                                      IN  Transaction           *pTransc,
                                      IN  RvSipMsgHandle        hMsg,
                                      IN  SipTransactionMethod  eMethod);

#ifndef RV_SIP_PRIMITIVES
static RvStatus RVCALLCONV HandlePrackInInitialState(
                                      IN  Transaction           *pTransc,
                                      IN  RvSipMsgHandle         hMsg);
#endif

static RvStatus SaveNon2xxToHeaderForAckSending(
                       IN  Transaction     *pTransc,
                       IN  RvSipMsgHandle   hMsg);
/*------------------------------------------------------------------------
         H A N D L E    P R O V I S I O N A L    R E S P O N S E S
 -------------------------------------------------------------------------*/

static RvStatus RVCALLCONV HandleProvisionalResponse(
                                                      IN  Transaction   *pTransc,
                                                      IN  RvSipMsgHandle hMsg);


#ifndef RV_SIP_PRIMITIVES


static RvBool RVCALLCONV IsReliableProvisionalResponse(
                                       IN RvSipMsgHandle     hMsg);

static void RVCALLCONV CheckRel1xxRseqHeader(IN  RvSipMsgHandle hMsg,
                                             OUT RvUint32      *pRseqStep,
                                             OUT RvBool        *pbLegal1xx);

#endif /*#ifndef RV_SIP_PRIMITIVES */

static RvBool RVCALLCONV IgnoreProvsionalResponse(
										  IN  Transaction    *pTransc,
										  IN  RvUint16		  respCode,
										  IN  RvSipMsgHandle  hMsg,
										  OUT RvBool         *bReliable1xx,
										  OUT RvUint32       *prseqStep);

/*------------------------------------------------------------------------
         H A N D L E    F I N A L    R E S P O N S E S
 -------------------------------------------------------------------------*/

static RvStatus RVCALLCONV HandleFinalResponse(
                                       IN  Transaction   *pTransc,
                                       IN  RvSipMsgHandle hMsg);

static RvStatus RVCALLCONV HandleInviteFinalResponseMsg(
                                       IN  Transaction   *pTransc,
                                       IN  RvSipMsgHandle hMsg,
                                       IN  RvUint16      responseCode);

static RvStatus RVCALLCONV HandleGeneralFinalResponse(
                                       IN  Transaction    *pTransc,
                                       IN  RvSipMsgHandle hMsg,
                                       IN  RvUint16      responseCode);

static RvStatus RVCALLCONV HandleCancelFinalResponse(
                                       IN  Transaction    *pTransc,
                                       IN  RvSipMsgHandle  hMsg,
                                       IN  RvUint16       responseCode);


static RvStatus RVCALLCONV HandleProxy2xxResponse(
                                       IN  Transaction    *pTransc,
                                       IN  RvSipMsgHandle hMsg);


/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TransactionMsgReceived
 * ------------------------------------------------------------------------
 * General: Called when a transaction has received a message. The transaction
 *            will behave according to the state it is in, and to the message
 *            received (request, response...).
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *               RV_ERROR_BADPARAM - The message is not a legal request or
 *                                       response.
 *                 RV_OK - The message has been received and handled
 *                              successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction which received the message.
 *          pMsg - The message received.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionMsgReceived(IN  Transaction   *pTransc,
                                            IN  RvSipMsgHandle hMsg)
{
    RvSipMsgType        bMsgType;
    RvStatus           retStatus;

#ifdef SIP_DEBUG
    if ((NULL == pTransc) || (NULL == hMsg))
    {
        return RV_ERROR_NULLPTR;
    }
#endif

    bMsgType = RvSipMsgGetMsgType(hMsg);

    if (RVSIP_MSG_REQUEST == bMsgType)
    {
        /*The message is a request message*/
        retStatus = HandleRequestMsg(pTransc, hMsg);
        if (RV_OK != retStatus)
        {
			RvSipTransactionState stateBeforeTerm = pTransc->eState; 

            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                       "TransactionMsgReceived - Failed for transaction 0x%p (request) - terminating with error.",pTransc));

            TransactionTerminate(RVSIP_TRANSC_REASON_ERROR, pTransc);

			/* Check the transaction's state before terminating and returning an erroneous */
			/* value. If the given transaction was responded successfully by the stack but */
			/* further related actions failed, a RV_OK should be returned - this way the   */
			/* received message WON'T be responded out of context.						   */
			/* For example: when a CANCEL message is received, the stack:				   */
			/* (a) Responds the CANCEL with 200 OK (b) Sends a 487 response for the		   */
			/* cancelled INVITE. In this example action (a) might be successful but not    */
			/* action (b). In this case a RV_OK value should be returned although action   */
			/* (b) failed - otherwise the CANCEL request will be responded twice		   */ 
			/* mistakenly: (1) Firstly by a 200 OK (2) Secondly out of context by 486.     */
			switch (stateBeforeTerm) 
			{
			case RVSIP_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT: 
			case RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT:
			case RVSIP_TRANSC_STATE_SERVER_CANCEL_FINAL_RESPONSE_SENT:
				return RV_OK;
			default:
				break;
				
			}
        }
        return retStatus;
    }
    else if (RVSIP_MSG_RESPONSE == bMsgType)
    {
        /*The message is a response message*/
        RvUint16 responseCode;
        responseCode = RvSipMsgGetStatusCode(hMsg);
        if ((100 <= responseCode) && (responseCode < 200))
        {
            /* Provisional response */
            retStatus = HandleProvisionalResponse(pTransc, hMsg);
        }
        else if ((200 <= responseCode) && (700 > responseCode))
        {
            /* Final response */
            retStatus = HandleFinalResponse(pTransc, hMsg);
        }
        else
        {
            /* Response code is out of range */
            return RV_ERROR_BADPARAM;
        }
        if(retStatus != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                       "TransactionMsgReceived -Failed for transaction 0x%p (response)- terminating with error.",pTransc));

            TransactionTerminate(RVSIP_TRANSC_REASON_ERROR, pTransc);
        }
        return retStatus;
    }
    /*The message is not a request or a response message*/
    return RV_ERROR_BADPARAM;
}

/***************************************************************************
 * TransactionMsgReceivedNotifyOthers
 * ------------------------------------------------------------------------
 * General: Called when a transaction has received a message to notify other
 *          entities who might be interested:
 *			1. We call MsgReceivedEv to notify the owner of the transaction 
 *             on the receipt of a message
 *          2. We notify the security-agreement layer on the receipt of a message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction which received the message.
 *        pMsg    - The message received.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionMsgReceivedNotifyOthers(
											IN  Transaction   *pTransc,
                                            IN  RvSipMsgHandle hMsg)
{
	RvStatus  retStatus; 
	
	/* Call "message received" callback */
    retStatus = TransactionCallBackMsgReceivedEv(pTransc, hMsg);
    if (retStatus != RV_OK)
    {
		return retStatus;
    }

#ifdef RV_SIP_IMS_ON
	/* Notify security-agreement layer */
	retStatus = TransactionSecAgreeMsgRcvd(pTransc, hMsg);
	if (retStatus != RV_OK)
    {
		return retStatus;
    }
#endif /* RV_SIP_IMS_ON */
	
	return RV_OK;
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * TransactionMsgIs1xxLegalRel
 * ------------------------------------------------------------------------
 * General: Checks if a given 1xx message is a reliable provisional response,
 *          and if has all mandatory parameters.
 * Return Value: RV_TRUE - reliable and legal.
 *               RV_FALSE - not.
 * ------------------------------------------------------------------------
 * Input:
 *      hMsg - The reliable 1xx message.
 * Output:
 *      pRseqStep - the step in the rseq header.
 *      pbLegal1xx - is the rel 1xx message legal (contains legal rseq header)
 *                   or not.
 ***************************************************************************/
void RVCALLCONV TransactionMsgIs1xxLegalRel(IN  RvSipMsgHandle hMsg,
                                            OUT RvUint32      *pRseqStep,
                                            OUT RvBool        *pbLegal1xx)
{

    if(IsReliableProvisionalResponse(hMsg) == RV_FALSE)
    {
        *pbLegal1xx = RV_FALSE;
        return;
    }
    CheckRel1xxRseqHeader(hMsg, pRseqStep, pbLegal1xx);
}
#endif /*RV_SIP_PRIMITIVES*/

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * HandleRequestMsg
 * ------------------------------------------------------------------------
 * General: Called when a transaction has received a request message. The
 *          transaction will behave according to the state it is in.
 * Return Value: RV_ERROR_NULLPTR - pTransc or hMsg are NULL pointers.
 *                 RV_OK - The message has been received and handled
 *                              successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction which received the request messgae.
 *          hMsg - The request message received.
 * Output:(-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleRequestMsg(
                                             IN  Transaction    *pTransc,
                                             IN  RvSipMsgHandle hMsg)
{
    RvStatus            retStatus;
    RvSipMethodType        eMethod;

    eMethod = RvSipMsgGetRequestMethod(hMsg);

#ifdef SIP_DEBUG
    /* Update statistics */
    if (RVSIP_METHOD_INVITE == eMethod)
    {
        pTransc->pMgr->msgStatistics.rcvdINVITE += 1;
        if (RVSIP_TRANSC_STATE_IDLE != pTransc->eState)
        {
            pTransc->pMgr->msgStatistics.rcvdINVITERetrans += 1;
        }
    }
    else
    {
        pTransc->pMgr->msgStatistics.rcvdNonInviteReq += 1;
        if ((RVSIP_TRANSC_STATE_IDLE != pTransc->eState &&
             RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT != pTransc->eState) ||
            (RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT == pTransc->eState &&
             RVSIP_METHOD_ACK != eMethod))
        {
            pTransc->pMgr->msgStatistics.rcvdNonInviteReqRetrans += 1;
        }
    }
#endif

#if defined(UPDATED_BY_SPIRENT)
#if !defined(UPDATED_BY_SPIRENT_ABACUS)
    if (RVSIP_METHOD_INVITE == eMethod)
    {
       if(RVSIP_TRANSC_STATE_IDLE != pTransc->eState)
       {
          if(pTransc->pMgr->pfnRetransmitRcvdEvHandler)
             pTransc->pMgr->pfnRetransmitRcvdEvHandler(hMsg, pTransc->pOwner);
       }
    }else if(RVSIP_METHOD_BYE == eMethod)
    {
       if(RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD == pTransc->eState ||
          RVSIP_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT == pTransc->eState)
       {
          if(pTransc->pMgr->pfnRetransmitRcvdEvHandler)
             pTransc->pMgr->pfnRetransmitRcvdEvHandler(hMsg, pTransc->pOwner);
       }
    }    
#endif    
#endif    


    switch (pTransc->eState)
    {
    case (RVSIP_TRANSC_STATE_IDLE):
        retStatus = HandleRequestInInitialState(pTransc, hMsg);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        break;
    case (RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT):
        eMethod = RvSipMsgGetRequestMethod(hMsg);
        if (RVSIP_METHOD_ACK == eMethod)
        {
            /* Ack method received in a server Invite in the "Final
               Response Sent state */
            RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                      "HandleRequestMsg - Transaction 0x%p: has received Ack request", pTransc));

            /* Notify the receipt of a message */
            retStatus = TransactionMsgReceivedNotifyOthers(pTransc,hMsg);
            if (retStatus != RV_OK)
            {
                return retStatus;
            }
            retStatus = TransactionStateServerInviteAckRcvd(
                                                RVSIP_TRANSC_REASON_ACK_RECEIVED,
                                                pTransc);
            if (retStatus != RV_OK)
            {
                return retStatus;
            }
        }
        break;
    case RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD:
    case RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD:
    case RVSIP_TRANSC_STATE_SERVER_CANCEL_REQUEST_RCVD:
        /* Retransmit the last provisional response message,
           if exists */
        if(eMethod == RVSIP_METHOD_ACK)
        {
            RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "HandleRequestMsg - Transaction 0x%p: has received ACK in state %s, Ignoring",
                pTransc,TransactionGetStateName(pTransc->eState)));
        }
        else
        {
            RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "HandleRequestMsg - Transaction 0x%p: has received request retransmission",
                pTransc));
            retStatus = TransactionTransportRetransmitMessage(pTransc);
            if (RV_OK != retStatus)
            {
                return retStatus;
            }
        }
        break;
    case (RVSIP_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT):
    case (RVSIP_TRANSC_STATE_SERVER_CANCEL_FINAL_RESPONSE_SENT):
        /* Retransmit the final response message */
        RvLogInfo((pTransc->pMgr)->pLogSrc,((pTransc->pMgr)->pLogSrc,
                  "HandleRequestMsg - Transaction 0x%p: has received request retransmission",
                  pTransc));
        retStatus = TransactionTransportRetransmitMessage(pTransc);
        if (RV_OK != retStatus)
        {
           return retStatus;
        }
        break;
    /* Proxy received ACK after sending 2xx response or an
       ACK retransamission*/
    case (RVSIP_TRANSC_STATE_SERVER_INVITE_PROXY_2XX_RESPONSE_SENT):
        {
            eMethod = RvSipMsgGetRequestMethod(hMsg);
            if(RVSIP_METHOD_INVITE == eMethod)
            {
                RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                    "HandleRequestMsg - A proxy Transc=0x%p: received request retransmission msg=0x%p",
                    pTransc,hMsg));
                return(RV_OK);
            }
            else
            {
                RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                    "HandleRequestMsg - A proxy Transc=0x%p: received request=%d on State:Proxy2XXSent msg=0x%p",
                    pTransc,eMethod, hMsg));
                return(RV_ERROR_ILLEGAL_ACTION);
            }
        }
    case RVSIP_TRANSC_STATE_TERMINATED:
        {
            if(pTransc->eLastState == RVSIP_TRANSC_STATE_SERVER_INVITE_PROXY_2XX_RESPONSE_SENT)
            {
                eMethod = RvSipMsgGetRequestMethod(hMsg);
                if(RVSIP_METHOD_INVITE == eMethod)
                {
                    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                        "HandleRequestMsg - A proxy Transc=0x%p: received request retransmission msg=0x%p",
                        pTransc,hMsg));
                    return(RV_OK);
                }
                else
                {
                    RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                              "HandleRequestMsg - A proxy Transc=0x%p: received request=%d on State:Proxy2XXSent msg=0x%p",
                              pTransc,eMethod, hMsg));
                        return(RV_ERROR_ILLEGAL_ACTION);
                }
            }
        }
    default:
        RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "HandleRequestMsg - Transaction 0x%p: received a request and ignored it",
                  pTransc));
        break;
    }
    return RV_OK;
}

/***************************************************************************
 * HandleProvisionalResponse
 * ------------------------------------------------------------------------
 * General: Called when a transaction has received a provisional response
 *          message. The transaction will behave according to the state it
 *            is in.
 * Return Value: RV_ERROR_NULLPTR - pTransc or hMsg are NULL pointers.
 *                 RV_OK - The message has been received and handled
 *                              successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction which received the response messgae.
 *          hMsg - The provisional response message received.
 * Output:(-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleProvisionalResponse(
                                            IN  Transaction   *pTransc,
                                            IN  RvSipMsgHandle hMsg)
{
    RvStatus  retStatus;
    RvInt16   responseCode;
    RvUint32  rseqStep = 0;
    RvBool    bReliable1xx = RV_FALSE;

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "HandleProvisionalResponse - Transaction 0x%p: has received a provisional response",
              pTransc));
    responseCode = RvSipMsgGetStatusCode(hMsg);
 
    if(IgnoreProvsionalResponse(pTransc,responseCode,hMsg,&bReliable1xx, &rseqStep) == RV_TRUE)
    {
        RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "HandleProvisionalResponse - Transaction 0x%p: ignoring the provisional response",
                  pTransc));
        return RV_OK;
    }
	TransactionSetResponseCode(pTransc, responseCode);

#ifdef SIP_DEBUG
    /* Update statistics */
    pTransc->pMgr->msgStatistics.rcvdResponse += 1;
#endif
#ifndef RV_SIP_PRIMITIVES

    /* save the to-tag and the time-stamp of the provisional response
       message in the transaction. */
    if(responseCode > 100)
    {
        /* update to-tag only from 1xx>100 (chapter 12.1 in RFC 3261)*/
        RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "HandleProvisionalResponse - Transaction 0x%p: copying tag to transaction",
                  pTransc));
        /* patch - before updating the to header, save the original to header aside,
           to use it in a cancel request. the standard say that the cancel request must be sent
           with the exact to header as in the INVITE request */
        if(pTransc->hOrigToForCancel == NULL)
        {
            /* Copy the given To header to the transaction's To header */
            retStatus = SipPartyHeaderConstructAndCopy(pTransc->pMgr->hMsgMgr,
                                                        pTransc->pMgr->hGeneralPool,
                                                        pTransc->memoryPage,
                                                        pTransc->hToHeader,
                                                        RVSIP_HEADERTYPE_TO,
                                                        &(pTransc->hOrigToForCancel));
            if (RV_OK != retStatus)
            {
                RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                    "HandleProvisionalResponse - Transaction 0x%p: Failed to copy To header for cancel",
                    pTransc));
                return retStatus;
            }
        }
        retStatus = TransactionUpdatePartyHeaders(pTransc,hMsg,responseCode);
        if (RV_OK != retStatus)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "HandleProvisionalResponse - Transaction 0x%p: Failed to update party headers",
                pTransc));
            return retStatus;
        }
    }
    retStatus = TransactionCopyTimestampFromMsg(pTransc, hMsg);

    if(retStatus != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "HandleProvisionalResponse - Failed for transc 0x%p, failed to copy Timestamp header",
            pTransc));
        return retStatus;
    }
#endif

    switch (pTransc->eState)
    {
    case (RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING):
        /* Notify the receipt of a message */
        retStatus = TransactionMsgReceivedNotifyOthers(pTransc,hMsg);
        if (retStatus != RV_OK)
        {
            return retStatus;
        }
        /* The transaction assumes Invite Proceeding state */
        retStatus = TransactionStateUacInviteProceeding(
                            RVSIP_TRANSC_REASON_PROVISIONAL_RESPONSE_RECEIVED,
                            pTransc);
        if (retStatus != RV_OK)
        {
            return retStatus;
        }
        break;
    case (RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT):

        retStatus = TransactionMsgReceivedNotifyOthers(pTransc,hMsg);
        if (retStatus != RV_OK)
        {
            return retStatus;
        }
        /* The transaction assumes General Proceeding state */
        retStatus = TransactionStateUacGeneralProceeding(
                            RVSIP_TRANSC_REASON_PROVISIONAL_RESPONSE_RECEIVED,
                            pTransc);

        if (retStatus != RV_OK)
        {
            return retStatus;
        }
        break;

    case (RVSIP_TRANSC_STATE_CLIENT_CANCEL_SENT):

        retStatus = TransactionMsgReceivedNotifyOthers(pTransc,hMsg);
        if (retStatus != RV_OK)
        {
            return retStatus;
        }

        /* The transaction assumes Cancel Proceeding state */
        retStatus = TransactionStateUacCancelProceeding(
                            RVSIP_TRANSC_REASON_PROVISIONAL_RESPONSE_RECEIVED,
                            pTransc);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        break;
    case (RVSIP_TRANSC_STATE_CLIENT_GEN_PROCEEDING):
    case (RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING):
    case (RVSIP_TRANSC_STATE_CLIENT_CANCEL_PROCEEDING):
    case (RVSIP_TRANSC_STATE_CLIENT_INVITE_CANCELLING):
    case (RVSIP_TRANSC_STATE_CLIENT_GEN_CANCELLING):
    case (RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING_TIMEOUT):
        /* Notify the receipt of a message */
        retStatus = TransactionMsgReceivedNotifyOthers(pTransc,hMsg);
        if (retStatus != RV_OK)
        {
            return retStatus;
        }
        break;

    default:
        break;
    }

#ifndef RV_SIP_PRIMITIVES
    /* Check if the current Transaction was not terminated */
    /* meanwhile by one of the executed callbacks          */
    if(pTransc->eState != RVSIP_TRANSC_STATE_TERMINATED && bReliable1xx == RV_TRUE)
    {
        retStatus = TransactionCallbackCallRelProvRespRcvdEv(pTransc, rseqStep);
        if(retStatus != RV_OK)
        {
            return retStatus;
        }
    }
#endif


    return RV_OK;
}


/***************************************************************************
 * HandleFinalResponse
 * ------------------------------------------------------------------------
 * General: Called when a transaction has received a final response
 *          message. The transaction will behave according to the state it
 *            is in.
 * Return Value: RV_ERROR_NULLPTR - pTransc or hMsg are NULL pointers.
 *                 RV_OK - The message has been received and handled
 *                              successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction which received the response messgae.
 *          hMsg - The final response message received.
 * Output:(-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleFinalResponse(
                                            IN  Transaction   *pTransc,
                                            IN  RvSipMsgHandle hMsg)
{
    RvUint16 responseCode;
    RvStatus retStatus;

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    if(RV_TRUE == pTransc->bMsgSentFailureInQueue)
    {
        RvLogInfo(pTransc->pMgr->pLogSrc, (pTransc->pMgr->pLogSrc, 
            "HandleFinalResponse: Transaction 0x%p - already has msg-sent-failure event in the queue. ignore.", pTransc));
        return RV_OK;
    }
#endif /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/
    responseCode = RvSipMsgGetStatusCode(hMsg);
    TransactionSetResponseCode(pTransc, responseCode);
    switch (pTransc->eState)
    {
    case (RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING):
    case (RVSIP_TRANSC_STATE_CLIENT_INVITE_CANCELLING):
    case (RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING):
        {
#ifdef SIP_DEBUG
            /* Update statistics */
            pTransc->pMgr->msgStatistics.rcvdResponse += 1;
#endif
            /* Proxy transaction receiving 2xx response*/
            if((pTransc->bIsProxy) &&
                (responseCode >= 200 && responseCode < 300))
            {
                retStatus = HandleProxy2xxResponse(pTransc, hMsg);
            }
            else
            {
                /* The transaction is a Uac INVITE */
                retStatus = HandleInviteFinalResponseMsg(
                                pTransc, hMsg, responseCode);
            }
            if (RV_OK != retStatus)
            {
                return retStatus;
            }
        }
        break;
    case (RVSIP_TRANSC_STATE_CLIENT_INVITE_ACK_SENT):
        /* The transaction is a Uac INVITE */
#ifdef SIP_DEBUG
        /* Update statistics */
        pTransc->pMgr->msgStatistics.rcvdResponse += 1;
        pTransc->pMgr->msgStatistics.rcvdResponseRetrans += 1;
#endif
        /* if Ack was not yet transmitted, we will not retranmit it*/
        if (RV_TRUE == pTransc->bAckSent)
        {
            /* Retransmit the ACK message */
            retStatus = TransactionTransportRetransmitMessage(pTransc);
            if (RV_OK != retStatus)
            {
                return retStatus;
            }
        }
        break;
    case (RVSIP_TRANSC_STATE_CLIENT_CANCEL_SENT):
#ifdef SIP_DEBUG
        /* Update statistics */
        pTransc->pMgr->msgStatistics.rcvdResponse += 1;
#endif
        /* The transaction is a Uac General */
        retStatus = HandleCancelFinalResponse(pTransc, hMsg, responseCode);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        break;
    case (RVSIP_TRANSC_STATE_CLIENT_GEN_PROCEEDING):
    case (RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT):
    case (RVSIP_TRANSC_STATE_CLIENT_GEN_CANCELLING):
        /* The transaction is a Uac General */
#ifdef SIP_DEBUG
        /* Update statistics */
        pTransc->pMgr->msgStatistics.rcvdResponse += 1;
#endif
        retStatus = HandleGeneralFinalResponse(
                                   pTransc, hMsg, responseCode);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        break;
    case (RVSIP_TRANSC_STATE_CLIENT_CANCEL_PROCEEDING):
        /* The transaction is a Uac General */
#ifdef SIP_DEBUG
        /* Update statistics */
        pTransc->pMgr->msgStatistics.rcvdResponse += 1;
#endif
        retStatus = HandleCancelFinalResponse(
                                  pTransc, hMsg, responseCode);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        break;
        /* The proxy received 2xx after sending ack, returned back to 2xx received
           or the proxy received a 2xx response retransmission*/
    case (RVSIP_TRANSC_STATE_CLIENT_INVITE_PROXY_2XX_RESPONSE_RCVD):
        {
#ifdef SIP_DEBUG
            /*ignore any non 2xx response at this state*/
            if(responseCode < 200 || responseCode >= 300)
            {
                return RV_OK;
            }
            /* Update statistics */
            pTransc->pMgr->msgStatistics.rcvdResponse += 1;
            pTransc->pMgr->msgStatistics.rcvdResponseRetrans += 1;
#endif
            retStatus = HandleProxy2xxResponse(pTransc, hMsg);
            return(retStatus);
        }

    default:
    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "HandleFinalResponse - Transaction 0x%p: received a response on state %s - ignored",
              pTransc,TransactionGetStateName(pTransc->eState)));

#ifdef SIP_DEBUG

        pTransc->pMgr->msgStatistics.rcvdResponse += 1;
        pTransc->pMgr->msgStatistics.rcvdResponseRetrans += 1;
#endif
        break;
    }
    return RV_OK;
}


/***************************************************************************
 * HandleRequestInInitialState
 * ------------------------------------------------------------------------
 * General: Called when a transaction has received a request message in the
 *          initial state.
 * Return Value: RV_ERROR_NULLPTR - pTransc or hMsg are NULL pointers.
 *                 RV_OK - The message has been received and handled
 *                              successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction which received the request messgae.
 *          hMsg - The request message received.
 ***************************************************************************/
static RvStatus RVCALLCONV HandleRequestInInitialState(
                                              IN  Transaction    *pTransc,
                                              IN  RvSipMsgHandle hMsg)
{
    RvSipMethodType eMethod;
    RvStatus        rv      = RV_OK;

    eMethod = RvSipMsgGetRequestMethod(hMsg);

#ifndef RV_SIP_PRIMITIVES
    if(eMethod != RVSIP_METHOD_ACK && eMethod != RVSIP_METHOD_CANCEL &&
       pTransc->pMgr->rejectUnsupportedExtensions == RV_TRUE)
    {
        rv = TransactionSetUnsupportedList(pTransc,hMsg);
        if(rv != RV_OK)
        {
            return rv;
        }
    }
#endif /*#ifndef RV_SIP_PRIMITIVES*/

#ifdef RV_SIP_AUTH_ON
    /* set authorization headers list from msg in the transaction -
       not for cancel and ack, because you can't challenge this kind of requests (rfc, bis6) */
    if(RV_TRUE == pTransc->pMgr->enableServerAuth)
    {
        if((RVSIP_METHOD_CANCEL != eMethod) && (RVSIP_METHOD_ACK != eMethod))
        {
            rv = TransactionAuthSetAuthorizationList(pTransc, hMsg);
            if(rv != RV_OK)
            {
                RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                      "HandleRequestInInitialState - Transaction 0x%p: Error - fail to set authorization list",
                      pTransc));
                return rv;
            }
        }
    }
#endif /* #ifdef RV_SIP_AUTH_ON */

    /*copy the time stamp to the transaction page*/
    rv = TransactionCopyTimestampFromMsg(pTransc,hMsg);
    if(rv != RV_OK)
    {
        return rv;
    }

    switch (eMethod)
    {
    case (RVSIP_METHOD_INVITE):
        /* The request received is an Invite request */
        return HandleInviteInInitialState(pTransc, hMsg);
    case (RVSIP_METHOD_ACK):
        /* Ack Request has been received in the Initial state */
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "HandleRequestInInitialState - Transaction 0x%p: Error - A new server transaction received Ack request",
                  pTransc));
        return RV_ERROR_BADPARAM;
    case (RVSIP_METHOD_BYE):
        /* The request received is a BYE request */
    case (RVSIP_METHOD_REGISTER):
        /* The request received is a REGISTER request */
#ifdef RV_SIP_SUBS_ON
    case (RVSIP_METHOD_REFER):
        /* The request received is a REFER request */
    case (RVSIP_METHOD_SUBSCRIBE):
        /* The request received is a SUBSCRIBE request */
    case (RVSIP_METHOD_NOTIFY):
        /* The request received is a NOTIFY request */
#endif /* #ifdef RV_SIP_SUBS_ON */
    case (RVSIP_METHOD_OTHER):
        return HandleGeneralInInitialState(pTransc, hMsg);
    case (RVSIP_METHOD_CANCEL):
        /* The request received is a NOTIFY request */
        return HandleCancelInInitialState(
                     pTransc, hMsg, SIP_TRANSACTION_METHOD_CANCEL);

#ifndef RV_SIP_PRIMITIVES
    case (RVSIP_METHOD_PRACK):
        {

            Transaction* pPrackPair= NULL;

            if(pTransc->pMgr->status100rel == RVSIP_TRANSC_100_REL_UNDEFINED)
            {
                return HandleGeneralInInitialState(pTransc, hMsg);
            }

            /*find the Invite transaction*/
            TransactionMgrHashFindMatchPrackTransc(pTransc->pMgr,
                                                    (RvSipTranscHandle)pTransc,
                                                    hMsg,
                                                    (RvSipTranscHandle*)&pPrackPair);

                /* The request received is a PRACK request, if the transaction is
                a proxy transaction call general recvd insted only if the pooxy didnt
            send the 1xx reliable.*/
            if(RV_TRUE == pTransc->bIsProxy)
            {
                if((NULL != pPrackPair) &&
                    (RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT == pPrackPair->eState))
                {
                    pTransc->hPrackParent = (RvSipTranscHandle)pPrackPair;
                    return HandlePrackInInitialState(pTransc, hMsg);
                }
                else
                {
                    return HandleGeneralInInitialState(pTransc, hMsg);
                }
            }
            else
            {
                pTransc->hPrackParent = (RvSipTranscHandle)pPrackPair;
                return HandlePrackInInitialState(pTransc, hMsg);
            }
        }
#endif /*#ifndef RV_SIP_PRIMITIVES */
    default:
        RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "HandleRequestInInitialState - Transaction 0x%p: Error - A new server transaction received request with undefined method",
                  pTransc));
        return RV_ERROR_BADPARAM;
    }
}






/***************************************************************************
 * HandleInviteInInitialState
 * ------------------------------------------------------------------------
 * General: Called when a transaction has received a INVITE request message in
 *          the initial state.
 * Return Value: RV_ERROR_OUTOFRESOURCES - No memory to update the transaction's
 *                                   Via list.
 *                 RV_OK - The message has been received and handled
 *                              successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction which received the request messgae.
 *          hMsg - The request message received.
 ***************************************************************************/
static RvStatus RVCALLCONV HandleInviteInInitialState(
                                            IN  Transaction    *pTransc,
                                            IN  RvSipMsgHandle hMsg)
{
    RvStatus            retStatus;

    /* Notify the receipt of a message */
    retStatus = TransactionMsgReceivedNotifyOthers(pTransc,hMsg);
    if (retStatus != RV_OK)
    {
        return retStatus;
    }
   /* Set the transaction's method to INVITE */
   pTransc->eMethod = SIP_TRANSACTION_METHOD_INVITE;


#ifndef RV_SIP_PRIMITIVES

   retStatus = TransactionUpdateReliableStatus(pTransc, hMsg);
   if (RV_OK != retStatus)
   {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "HandleInviteInInitialState - Transaction 0x%p: Error in trying to construct a new Via list.",
                  pTransc));
        return retStatus;
   }
#endif  /*v #ifndef RV_SIP_PRIMITIVES */

   /* The transaction assumes "Invite Request Received" state */
   retStatus = TransactionStateSrvInviteReqRecvd(
                                     RVSIP_TRANSC_REASON_REQUEST_RECEIVED,
                                     pTransc);
   if (RV_OK != retStatus)
   {
        return retStatus;
   }
   return RV_OK;
}


/***************************************************************************
 * HandleGeneralInInitialState
 * ------------------------------------------------------------------------
 * General: Called when a transaction has received a general request message
 *          in the initial state.
 * Return Value: RV_ERROR_OUTOFRESOURCES - No memory to update the transaction's
 *                                   Via list.
 *                 RV_OK - The message has been received and handled
 *                              successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction which received the request messgae.
 *          hMsg - The request message received.
 ***************************************************************************/
static RvStatus RVCALLCONV HandleGeneralInInitialState(
                                      IN  Transaction       *pTransc,
                                      IN  RvSipMsgHandle    hMsg)
{
    RvStatus         retStatus;

    /* Notify the receipt of a message */
    retStatus = TransactionMsgReceivedNotifyOthers(pTransc,hMsg);
    if (retStatus != RV_OK)
    {
        return retStatus;
    }
     /* Add To Tag if required */
    /* The transaction assumes "General Request Received" state */
    retStatus = TransactionStateSrvGeneralReqRecvd(RVSIP_TRANSC_REASON_REQUEST_RECEIVED,
                                                   pTransc);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    return RV_OK;
}

/***************************************************************************
 * HandleCancelInInitialState
 * ------------------------------------------------------------------------
 * General: Called when a transaction has received a cancel request message
 *          in the initial state.
 * Return Value: RV_ERROR_OUTOFRESOURCES - No memory to update the transaction's
 *                                   Via list.
 *                 RV_OK - The message has been received and handled
 *                              successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction which received the request messgae.
 *          hMsg - The request message received.
 * Output:(-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleCancelInInitialState(
                                      IN  Transaction           *pTransc,
                                      IN  RvSipMsgHandle        hMsg,
                                      IN  SipTransactionMethod  eMethod)
{
    RvStatus         retStatus;

    RV_UNUSED_ARG(eMethod);
    /* Notify the receipt of a message */
    retStatus = TransactionMsgReceivedNotifyOthers(pTransc,hMsg);
    if (retStatus != RV_OK)
    {
        return retStatus;
    }
    /*release transaction timers*/
    TransactionTimerReleaseAllTimers(pTransc);

    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "HandleCancelInInitialState - transc 0x%p - hCancelTranscPair = 0x%p",
              pTransc, pTransc->hCancelTranscPair));

       retStatus = TransactionStateSrvCancelReqRecvd(RVSIP_TRANSC_REASON_REQUEST_RECEIVED,pTransc,
                                                  (Transaction*)pTransc->hCancelTranscPair);



    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    return RV_OK;
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * HandlePrackInInitialState
 * ------------------------------------------------------------------------
 * General: Called when a transaction has received a prack request message
 *          in the initial state.
 * Return Value: RV_ERROR_OUTOFRESOURCES - No memory to update the transaction's
 *                                   Via list.
 *                 RV_OK - The message has been received and handled
 *                              successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction which received the request messgae.
 *          hMsg - The request message received.
 ***************************************************************************/
static RvStatus RVCALLCONV HandlePrackInInitialState(
                                      IN  Transaction           *pTransc,
                                      IN  RvSipMsgHandle         hMsg)
{
    RvStatus retStatus;
    Transaction* pRelRespTransc = (Transaction*)pTransc->hPrackParent;

    /* If no matching 18X transaction was found, don't handle PRACK.
       The matching 18X transaction may be closed on timeout before PRACK
       will be received. */
    if (NULL == pTransc->hPrackParent)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "HandlePrackInInitialState - Transaction 0x%p: no matching 18X transaction was found",
                  pTransc));
    }

    /* Notify the receipt of a message */
    retStatus = TransactionMsgReceivedNotifyOthers(pTransc,hMsg);
    if (retStatus != RV_OK)
    {
        return retStatus;
    }

    /* Release the timers of the reliable provisional response transaction*/
    if (pRelRespTransc != NULL && TransactionLockMsg(pRelRespTransc) == RV_OK)
    {
        if (RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT ==
            pRelRespTransc->eState)
        {
            TransactionTimerReleaseAllTimers(pRelRespTransc);
        }
        TransactionUnLockMsg(pRelRespTransc);
    }

    /* The transaction assumes "Prack Request Received" state */
    retStatus = TransactionStateSrvPrackReqRecvd(RVSIP_TRANSC_REASON_REQUEST_RECEIVED,
                                                 pTransc,
                                                 hMsg);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    return RV_OK;
}
#endif    /* #ifndef RV_SIP_PRIMITIVES */

/***************************************************************************
 * HandleInviteFinalResponseMsg
 * ------------------------------------------------------------------------
 * General: Called when an INVITE transaction has received a final response
 *          message. (accept after an Ack was already sent). The transaction
 *          will behave according to the state it is in.
 * Return Value: RV_ERROR_NULLPTR - pTransc or hMsg are NULL pointers.
 *                 RV_OK - The message has been received and handled
 *                              successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction which received the response messgae.
 *          hMsg - The final response message received.
 *        responseCode - The response code received in the message.
 * Output:(-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleInviteFinalResponseMsg(
                                              IN  Transaction   *pTransc,
                                              IN  RvSipMsgHandle hMsg,
                                              IN  RvUint16      responseCode)
{
    RvStatus          rv;
    RvLogSource*       logId;

    if ((NULL == pTransc) || (NULL == hMsg))
    {
        return RV_ERROR_NULLPTR;
    }

    logId = (pTransc->pMgr)->pLogSrc;
    RvLogInfo(logId ,(logId ,
              "HandleInviteFinalResponseMsg - Transaction 0x%p: has received a final response",
              pTransc));

    /*if the response is 2xx - remove the transaction from the hash so that no
      other response will be matched to this transaction*/
    if (200 <= responseCode && 300 > responseCode &&
        pTransc->pMgr->bOldInviteHandling == RV_FALSE)
    {
        TransactionMgrHashRemove((RvSipTranscHandle)pTransc);
    }


    /* Notify the receipt of a message */
    rv = TransactionMsgReceivedNotifyOthers(pTransc,hMsg);
    if (rv != RV_OK)
    {
        return rv;
    }

    if ((200 <= responseCode) && (300 > responseCode)) /*2xx */
    {
        rv = TransactionUpdatePartyHeaders(pTransc,hMsg, responseCode);
        if (RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "HandleInviteFinalResponseMsg - Transaction 0x%p: Failed to update party headers",
                pTransc));
            return rv;
        }
        /* Ack for 2xx is considered a new transaction there for we need to re query the DNS
           for the address of the URI. to accomplish that we clear the DNS list of the transaction */
        rv = SipTransmitterDNSListReset(pTransc->hTrx);
        if (RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "HandleInviteFinalResponseMsg - Transaction 0x%p: Failed to update party headers",
                pTransc));
            return rv;
        }
    }
    else
    {
        /* Copy the To tag received in the To header */
        rv = TransactionCopyToTag(pTransc, hMsg, RV_FALSE);
        if (RV_OK != rv)
        {
            RvLogError(logId ,(logId ,
                      "HandleInviteFinalResponseMsg - Transaction 0x%p: Error - Couldn't update To header",
                      pTransc));
            return rv;
        }
        /* save also the to-tag for ACK sending, (TransactionCopyToTag() might find an
        exist to-tag from 1xx, and then to-tag won't be updated */
        rv = SaveNon2xxToHeaderForAckSending(pTransc, hMsg);
        if (RV_OK != rv)
        {
            RvLogError(logId ,(logId ,
                      "HandleInviteFinalResponseMsg - Transaction 0x%p: Error - Couldn't save to-tag for future ACK",
                      pTransc));
            return rv;
        }
    }
    /* The transaction assumes Invite Final Response Received state */
    rv = TransactionStateUacInviteFinalRespRecvd(
                    FinalResponseReason(responseCode),
                    pTransc);
    if (RV_OK != rv)
    {
        return rv;
    }
    return RV_OK;
}


/***************************************************************************
 * HandleGeneralFinalResponse
 * ------------------------------------------------------------------------
 * General: Called when a General transaction has received a final response
 *          message. The transaction will behave according to the state it
 *          is in.
 * Return Value: RV_ERROR_NULLPTR - pTransc or hMsg are NULL pointers.
 *                 RV_OK - The message has been received and handled
 *                              successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction which received the response messgae.
 *          hMsg - The final response message received.
 *        responseCode - The response code received in the message.
 * Output:(-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleGeneralFinalResponse(
                                            IN  Transaction    *pTransc,
                                            IN  RvSipMsgHandle hMsg,
                                            IN  RvUint16      responseCode)
{
    RvStatus            retStatus = RV_OK;

    RvLogInfo((pTransc->pMgr)->pLogSrc,((pTransc->pMgr)->pLogSrc,
              "HandleGeneralFinalResponse - Transaction 0x%p: has received a final response=%d",
              pTransc, responseCode));
    /* Notify the receipt of a message */
    retStatus = TransactionMsgReceivedNotifyOthers(pTransc,hMsg);
    if (retStatus != RV_OK)
    {
        return retStatus;
    }
    switch (pTransc->eMethod)
    {
#ifdef RV_SIP_SUBS_ON
    case SIP_TRANSACTION_METHOD_SUBSCRIBE:
    case SIP_TRANSACTION_METHOD_REFER:
        if ((200 <= responseCode) && (300 > responseCode))
        {
            retStatus = TransactionUpdatePartyHeaders(pTransc,hMsg,responseCode);
            if (RV_OK != retStatus)
            {
                RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                    "HandleGeneralFinalResponse - Transaction 0x%p: Failed to update party headers",
                    pTransc));
                return retStatus;
            }
        }
        break;
#endif /* #ifdef RV_SIP_SUBS_ON */
#ifndef RV_SIP_PRIMITIVES
    case SIP_TRANSACTION_METHOD_PRACK:
        pTransc->hReceivedMsg = hMsg;
        break;
#endif /* #ifndef RV_SIP_PRIMITIVES */
    default:
        break;
    }

    retStatus = TransactionStateUacGeneralFinalResponseRcvd(
                                    FinalResponseReason(responseCode),
                                    pTransc);

    return retStatus;
}

/***************************************************************************
 * HandleCancelFinalResponse
 * ------------------------------------------------------------------------
 * General: Called when a Cancel transaction has received a final response
 *          message. The transaction will behave according to the state it
 *          is in.
 * Return Value: RV_ERROR_NULLPTR - pTransc or hMsg are NULL pointers.
 *                 RV_OK - The message has been received and handled
 *                              successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction which received the response messgae.
 *          hMsg - The final response message received.
 *        responseCode - The response code received in the message.
 * Output:(-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleCancelFinalResponse(
                                            IN  Transaction    *pTransc,
                                            IN  RvSipMsgHandle  hMsg,
                                            IN  RvUint16       responseCode)
{
    RvStatus            retStatus = RV_OK;

    RvLogInfo((pTransc->pMgr)->pLogSrc,((pTransc->pMgr)->pLogSrc,
              "HandleCancelFinalResponse - Transaction 0x%p: has received a final response",
              pTransc));

    /* Notify the receipt of a message */
    retStatus = TransactionMsgReceivedNotifyOthers(pTransc,hMsg);
    if (retStatus != RV_OK)
    {
        return retStatus;
    }

    retStatus = TransactionStateUacCancelFinalResponseRcvd(
                                FinalResponseReason(responseCode),
                                pTransc);
    return retStatus;

}


#ifndef RV_SIP_PRIMITIVES

/***************************************************************************
 * IsReliableProvisionalResponse
 * ------------------------------------------------------------------------
 * General: Check if the provisional resoponse contains the "Require=100rel"
 * Return Value: RV_TRUE is this is a reliable provisional response. RV_FALSE
 *               otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: pTransc - The transaction in which to update To header.
 *           pMsg - The message from which the To tag is copied.
 ***************************************************************************/
static RvBool RVCALLCONV IsReliableProvisionalResponse(
                                             IN RvSipMsgHandle     hMsg)

{
    return SipMsgDoesOtherHeaderExist(hMsg,"Require","100rel",NULL);
}

/***************************************************************************
 * CheckRel1xxRseqHeader
 * ------------------------------------------------------------------------
 * General: Check if the reliable 1xx contains all 'must' parameters.
 *          (RSeq header, and step in it).
 * Return Value: RV_TRUE is this is a reliable provisional response. RV_FALSE
 *               otherwise.
 * ------------------------------------------------------------------------
 * Input:
 *      hMsg - The reliable 1xx message.
 * Output:
 *      pRseqStep - the step in the rseq header.
 *      pbLegal1xx - is the rel 1xx message legal (contains legal rseq header)
 *                   or not.
 ***************************************************************************/
static void RVCALLCONV CheckRel1xxRseqHeader(IN RvSipMsgHandle hMsg,
                                             OUT RvUint32     *pRseqStep,
                                             OUT RvBool       *pbLegal1xx)
{
    RvSipRSeqHeaderHandle      hRSeq;
    RvSipHeaderListElemHandle  hElement;
	RvBool					   bInitResp;
	RvStatus				   rv;

 
    /* if the 1xx contain an RSEQ header, then prack is needed */
    hRSeq = (RvSipRSeqHeaderHandle)RvSipMsgGetHeaderByType(
				hMsg,RVSIP_HEADERTYPE_RSEQ,RVSIP_FIRST_HEADER,&hElement);
    if(hRSeq == NULL)
    {
        *pbLegal1xx = RV_FALSE;
        return;
    }

	rv = RvSipRSeqHeaderIsResponseNumInitialized(hRSeq,&bInitResp); 
	if (RV_OK != rv || RV_FALSE == bInitResp)
	{
		*pbLegal1xx = RV_FALSE;
        return;
    }
	*pbLegal1xx = RV_TRUE;
    *pRseqStep   = RvSipRSeqHeaderGetResponseNum(hRSeq);
    return;
}

#endif /*#ifndef RV_SIP_PRIMITIVES */

/***************************************************************************
 * IgnoreProvsionalResponse
 * ------------------------------------------------------------------------
 * General: Check if we should ignore the provisional response. this will
 *          be the case if the provisional response is a retrunsmission or
 *          if it has an invalid RSeq for reliable provisional response.
 *          we never ignore 100 response or provisionals on a non Invite
 *          requset.
 *          This function also determines if we should send a prack. This
 *          is the case if the provsional response is on invite and is
 *          reliable.
 * Return Value: RV_TRUE if we should ignore the provisional response.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc	- The transaction handle.
 * 		   respCode	- The received provisional response code
 *         pMsg		- The message handle.
 * Output: bReliable1xx - Indication if the received provisional response
 *						  is reliable.
 *		   prseqStep    - The provisional response RSeq step
 ***************************************************************************/
static RvBool RVCALLCONV IgnoreProvsionalResponse(
                                    IN  Transaction    *pTransc,
									IN  RvUint16		respCode,
                                    IN  RvSipMsgHandle  hMsg,
                                    OUT RvBool         *bReliable1xx,
                                    OUT RvUint32       *prseqStep)

{
#ifndef RV_SIP_PRIMITIVES
    RvBool  isReliable, isLegal;
#endif /* RV_SIP_PRIMITIVES */

    *bReliable1xx = RV_FALSE;

	/* Check if the provisional response is received/handled after */
	/* a final reponse, or after the transaction become terminated */
	/* If it does, then the response message should be ignored	   */
	switch (pTransc->eState) {
	case RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD:
	case RVSIP_TRANSC_STATE_CLIENT_INVITE_ACK_SENT:
	case RVSIP_TRANSC_STATE_TERMINATED:
	case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROXY_2XX_RESPONSE_RCVD:
	case RVSIP_TRANSC_STATE_CLIENT_GEN_FINAL_RESPONSE_RCVD:
	case RVSIP_TRANSC_STATE_CLIENT_CANCEL_FINAL_RESPONSE_RCVD:
		RvLogInfo((pTransc->pMgr)->pLogSrc,((pTransc->pMgr)->pLogSrc,
			"IgnoreProvsionalResponse - Transaction 0x%p: cannot handle provisional in state %s, ignoring it",
			pTransc,TransactionGetStateName(pTransc->eState)));
		return RV_TRUE; 
	default:
		/* For the rest of the states we move on */
		break;
	}
	
#ifndef RV_SIP_PRIMITIVES
    /* Proxy never ignores provisional response*/
    if(RV_TRUE == pTransc->bIsProxy)
    {
        return RV_FALSE;
    }

    /*never ignore 100 responses*/
    if(respCode == 100)
    {
        return RV_FALSE;
    }

    /*never ignore provisional for general transactions*/
    if(pTransc->eState != RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING &&
       pTransc->eState != RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING)
    {
        return RV_FALSE;
    }
    /*if we do no support the  100rel extension we handle the provisionl
      as a regular provisional */
    if(pTransc->pMgr->status100rel == RVSIP_TRANSC_100_REL_UNDEFINED)
    {
        return RV_FALSE;
    }

    /*if the provisional is not reliable do not ignore it*/
    isReliable = IsReliableProvisionalResponse(hMsg);
    if(isReliable == RV_FALSE)
    {
        return RV_FALSE;
    }

    /*the provisional is reliable - check if it is valid*/
    CheckRel1xxRseqHeader(hMsg, prseqStep, &isLegal);
    if(isLegal == RV_FALSE)
    {
        /* non legal rel 1xx. ignore it */
        return RV_TRUE;
    }

    /* consult the call-leg about this rseq step.
       (if it is not bigger than the last rseq step that was received in rel1xx,
       then this message should be ignored) */
    {
        RvBool bIgnore = RV_FALSE;
        TransactionCallbackCallIgnoreRelProvRespEv(pTransc, *prseqStep, &bIgnore);
        if(RV_TRUE == bIgnore)
        {
            return RV_TRUE;
        }
    }


    *bReliable1xx = RV_TRUE;
#else
    RV_UNUSED_ARG(prseqStep)
    RV_UNUSED_ARG(hMsg)
    RV_UNUSED_ARG(respCode)
#endif /* #ifndef RV_SIP_PRIMITIVES */ 

    return RV_FALSE;
}


/***************************************************************************
 * HandleProxy2xxResponse
 * ------------------------------------------------------------------------
 * General: Called when a Proxy server transaction has received a 2xx response
 *          message. The transaction will behave according to the state it
 *          is in.
 * Return Value: RV_ERROR_NULLPTR - pTransc or hMsg are NULL pointers.
 *                 RV_OK - The message has been received and handled
 *                              successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction which received the response messgae.
 *          hMsg - The final response message received.
 * Output:(-)
 ***************************************************************************/
static RvStatus RVCALLCONV HandleProxy2xxResponse(
                                       IN  Transaction    *pTransc,
                                       IN  RvSipMsgHandle hMsg)
{
    RvStatus            rv = RV_OK;

    RvLogInfo((pTransc->pMgr)->pLogSrc,((pTransc->pMgr)->pLogSrc,
              "HandleProxy2xxResponse - Transaction 0x%p: has received a 2xx response",
              pTransc));

    /* Notify the receipt of a message */
    rv    = TransactionMsgReceivedNotifyOthers(pTransc,hMsg);
    if (rv != RV_OK)
    {
        return rv;
    }
    /* Copy the To tag received in the To header only if this is the first time*/
    if(pTransc->eState != RVSIP_TRANSC_STATE_CLIENT_INVITE_PROXY_2XX_RESPONSE_RCVD)
    {
        if(SipPartyHeaderGetTag(pTransc->hToHeader) == UNDEFINED)
        {

            rv = TransactionCopyToTag(pTransc, hMsg, RV_FALSE);
            if (RV_OK != rv)
            {
                RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                    "HandleProxy2xxResponse - Transaction 0x%p: Error - Couldn't update To header",
                    pTransc));
                return rv;
            }
        }
    }
    rv = TransactionStateProxyInvite2xxRespRcvd(RVSIP_TRANSC_REASON_RESPONSE_SUCCESSFUL_RECVD,
                                                pTransc);
    return(rv);

}


/***************************************************************************
 * SaveNon2xxToHeaderForAckSending
 * ------------------------------------------------------------------------
 * General: Saves the non-2xx final response to tags in temp place in transaction,
 *          in order to set it in outgoing ACK.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc        - pointer to the transaction.
 *          hReceivedMsg   - received message.
 ***************************************************************************/
static RvStatus SaveNon2xxToHeaderForAckSending(
                       IN  Transaction     *pTransc,
                       IN  RvSipMsgHandle   hMsg)
{
    RvSipPartyHeaderHandle hTo;
    RvStatus              rv;

    /* to-tag */
    hTo = RvSipMsgGetToHeader(hMsg);
    
    rv = SipPartyHeaderConstructAndCopy(pTransc->pMgr->hMsgMgr,
                                        pTransc->pMgr->hGeneralPool,
                                        pTransc->memoryPage,
                                        hTo,
                                        RVSIP_HEADERTYPE_TO,
                                        &(pTransc->non2xxRespMsgToHeader));
    if(RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "SaveNon2xxToHeaderForAckSending: transc 0x%p - failed to copy incoming non-2xx to-header. destruct transc", pTransc));
        return rv;
    }
    
    return RV_OK;
}
#endif /*#ifndef RV_SIP_LIGHT*/

