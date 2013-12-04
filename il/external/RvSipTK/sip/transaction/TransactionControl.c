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
*********************************************************************************
*/


/*********************************************************************************
 *                              <TransactionControl.c>
 *  Handles all tranaction activities initiated by the user such as
 *  request/response/ack/cancel.
 *  call-leg and more.
 *
 *    Author                         Date
 *    ------                        ------
 *   Sarit Mekler                   30/7/01
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "TransactionControl.h"
#include "TransactionState.h"
#include "TransactionTransport.h"
#include "TransactionTimer.h"
#include "_SipPartyHeader.h"
#include "_SipViaHeader.h"
#include "RvSipViaHeader.h"
#include "_SipMsg.h"
#include "TransactionCallBacks.h"
#include "_SipCommonUtils.h"
#include "_SipCommonCore.h"
#include "_SipCommonCSeq.h"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/



static RvStatus RVCALLCONV GenerateAndSendRequest(
                                    IN  Transaction         *pTransc,
                                    IN  RvSipAddressHandle   hRequestUri,
                                    IN  RvSipMsgHandle       hMsgToSend,
                                    IN  RvBool               bResolveAddress);

static RvSipTransactionState RVCALLCONV TransactionGetNextSrvState(IN Transaction* pTransc,
                                    IN RvUint16    uRespCode,
                                    IN RvBool      bIsReliable);

static RvSipTransactionState RVCALLCONV TransactionGetNextUacState(
                                    IN Transaction* pTransc);

static RvStatus RVCALLCONV TransactionChangeToNextUacState(
                                    IN  Transaction                          *pTransc,
                                    IN  RvSipTransactionStateChangeReason eReason);

static RvStatus RVCALLCONV SynchronizeTranscAndMsgToTags(
                                    IN Transaction* pTransc,
                                    IN RvSipMsgHandle hMsg);
static RvStatus RVCALLCONV VerifyMsgToFromHeaders(IN Transaction* pTransc,
                                          IN RvSipMsgHandle hMsg);

static RvStatus RVCALLCONV VerifyAckMsgToHeader(IN Transaction* pTransc,
                                             IN RvSipMsgHandle hMsg);

#ifndef RV_SIP_PRIMITIVES
static RvStatus RVCALLCONV AddReferredByHeaderIfExists(IN Transaction   *pTransc,
                                                       IN RvSipMsgHandle hMsgToSend);
#endif /* #ifndef RV_SIP_PRIMITIVES */

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TransactionControlSendRequest
 * ------------------------------------------------------------------------
 * General: Creates and sends a request message, according to the
 *          given method with the given address as a Request-Uri.
 * Return Value: RV_OK - The message was created and sent successfully.
 *                 RV_ERROR_UNKNOWN - An error occured while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -   The transaction this request refers to.
 *            pRequestUri -    The Request-Uri to be used in the request
 *                           message.
 *                           The message's Request-Uri will be the address
 *                           given here, and the message will be sent
 *                           accordingly.
*        bResolveAddr - RV_TRUE if we want the transmitter to resolve the dest addr
*                       from the message, RV_FALSE if the transmitter should use the
*                       address it currently hold.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionControlSendRequest(
                                    IN  Transaction        *pTransc,
                                    IN  RvSipAddressHandle  hRequestUri,
                                    IN  RvBool              bResolveAddr)
{
    RvStatus    rv;

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "TransactionControlSendRequest - Transaction 0x%p: about to send a request", pTransc));


    /*generate a new empty message if the user did not set one */
    if (pTransc->hOutboundMsg == NULL)
    {
        rv = RvSipMsgConstruct(pTransc->pMgr->hMsgMgr, pTransc->pMgr->hMessagePool,&pTransc->hOutboundMsg);
        if(rv != RV_OK)
        {
            RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionControlSendRequest - Transaction 0x%p: Failed to create new message", pTransc));
            return rv;
        }
    }
#ifndef RV_SIP_PRIMITIVES
    /* add headers list to the request message (if exist) */
    if(pTransc->hHeadersListToSetInInitialRequest != NULL)
    {
        rv = SipMsgPushHeadersFromList(pTransc->hOutboundMsg, pTransc->hHeadersListToSetInInitialRequest);
        if(RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "TransactionControlSendRequest - Transaction 0x%p - Failed to set headers list to msg",
                pTransc));
            return rv;
        }
    }
#endif /*#ifndef RV_SIP_PRIMITIVES*/

    rv = GenerateAndSendRequest(pTransc,hRequestUri,pTransc->hOutboundMsg,bResolveAddr);

    if(pTransc->hOutboundMsg != NULL)
    {
        RvSipMsgDestruct(pTransc->hOutboundMsg);
        pTransc->hOutboundMsg = NULL;

    }

    if(rv != RV_OK)
    {
        return rv;
    }


    if(pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlSendRequest - Transaction 0x%p: Failed to send Request. Transaction was terminated",
                  pTransc));
        return RV_ERROR_UNKNOWN;
    }



    /*notify the application about the new state*/
    rv = TransactionChangeToNextUacState(pTransc,
                                         RVSIP_TRANSC_REASON_USER_COMMAND);
    return rv;
}

/***************************************************************************
 * TransactionControlSendResponse
 * ------------------------------------------------------------------------
 * General: Creates and sends a response message
 * Return Value: RV_OK - The message was created and sent successfully.
 *                 RV_ERROR_UNKNOWN - An error occured while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc -   The transaction handle
 *          responseCode - The response code to use in the response
 *          strReasonPhrase - The reason fraze to use in the response.
 *          RvSipTransactionStateChangeReason - The reason to indicate when
 *                                              changing the state.
 *          bIsReliable - RV_TRUE if the response needs to be sent reliably
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionControlSendResponse(
                    IN  Transaction*       pTransc,
                    IN  RvUint16           responseCode,
                    IN  RvChar*            strReasonPhrase,
                    IN  RvSipTransactionStateChangeReason eReason,
                    IN  RvBool             bIsReliable)
{
    RvStatus    rv;
    RvSipTransactionState eInitialState = pTransc->eState;

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "TransactionControlSendResponse - Transaction 0x%p: about to send %d response",
              pTransc,responseCode));


    /*generate a new empty message if the user did not send one */
    if (pTransc->hOutboundMsg == NULL)
    {
        rv = RvSipMsgConstruct(pTransc->pMgr->hMsgMgr, pTransc->pMgr->hMessagePool,&pTransc->hOutboundMsg);
        if(rv != RV_OK)
        {
            RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionControlSendResponse - Transaction 0x%p: Failed to create new message", pTransc));
            return rv;
        }
    }


    pTransc->retransmissionsCount=0;
    rv = TransactionControlGenerateAndSendResponse(pTransc,responseCode,
                                                   strReasonPhrase,bIsReliable);
    if(pTransc->hOutboundMsg != NULL)
    {
        RvSipMsgDestruct(pTransc->hOutboundMsg);
        pTransc->hOutboundMsg = NULL;

    }

    if(rv != RV_OK)
    {
         return rv;
    }

    if(pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlSendResponse - Transaction 0x%p: Failed to send response. Transaction was terminated",
                  pTransc));
        return RV_Failure;
    }

    rv = TransactionControlChangeToNextSrvState(pTransc,eInitialState, responseCode,
                                                bIsReliable, eReason);


    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
          "TransactionControlSendResponse - Transaction 0x%p: changing to next server state (rv=%d).",
          pTransc,rv));
        return rv;
    }
    return RV_OK;
}

/***************************************************************************
 * TransactionControlSendAck
 * ------------------------------------------------------------------------
 * General:  Creates and sends an acknowledgment message with the given
 *           address as the Request-Uri.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      pTransc - The transaction handle.
 *            pRequestUri -  The Request-Uri to be used in the ACK message. The
 *                           message's Request-Uri will be the address given
 *                         here, and the message will be sent accordingly.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionControlSendAck(
                                       IN Transaction         *pTransc,
                                       IN RvSipAddressHandle hRequestUri)
{
    RvStatus                    rv;
    RvSipTransactionState       eInitialState = pTransc->eState;

    /*generate a new empty message*/
    if (pTransc->hOutboundMsg == NULL)
    {
        rv = RvSipMsgConstruct(pTransc->pMgr->hMsgMgr,
                               pTransc->pMgr->hMessagePool,
                               &pTransc->hOutboundMsg);
        if(rv != RV_OK)
        {
            RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionControlSendAck - Transaction 0x%p: Failed to create new message", pTransc));
            return rv;
        }
    }


    /* Create a request message with the To, From, Call-Id, CSeq of the
       transaction, the given Request-Uri, and the ACK Request-Method */
    rv = TransactionCreateRequestMessage(pTransc,hRequestUri, pTransc->hOutboundMsg);
    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlSendAck - Transaction 0x%p: Error in trying to create a request message.(rv=%d)",
                   pTransc,rv));
        RvSipMsgDestruct(pTransc->hOutboundMsg);
        pTransc->hOutboundMsg = NULL;
        return rv;
    }

    
    /* 17.1.1.3 Construction of the ACK Request
    ... The To header field in the ACK MUST equal the To header field in the
        response being acknowledged, and therefore will usually differ from
        the To header field in the original request by the addition of the
        tag parameter.*/
    /* --> in case of ack on non-2xx or on 2xx in old invite behavior, we are making
       sure that the to-tag in the ACK message, is the same as was received in the final response */
    {
        VerifyAckMsgToHeader(pTransc, pTransc->hOutboundMsg);
    }

    /* Call "message to send" callback */
    rv= TransactionCallbackMsgToSendEv(pTransc, pTransc->hOutboundMsg);

    if (RV_OK != rv)
    {
        /* Destruct the message object */
        RvSipMsgDestruct(pTransc->hOutboundMsg);
        pTransc->hOutboundMsg = NULL;
        return rv;
    }

    /* updating the last state, in order of determining the reason */
    pTransc->eLastState = pTransc->eState;
    pTransc->eState = RVSIP_TRANSC_STATE_CLIENT_INVITE_ACK_SENT;

    /* Send message */
    pTransc->msgType = SIP_TRANSPORT_MSG_ACK_SENT;
    pTransc->retransmissionsCount=0;

    if((pTransc->responseCode >= 200) && (pTransc->responseCode < 300))
    {
        rv = TransactionTransportSendMsg(pTransc, RV_TRUE);
    }
    else
    {
        rv = TransactionTransportSendMsg(pTransc, RV_FALSE);
    }

    if(pTransc->hOutboundMsg != NULL)
    {
        RvSipMsgDestruct(pTransc->hOutboundMsg);
        pTransc->hOutboundMsg = NULL;

    }

    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlSendAck - Transaction 0x%p: Error in trying to send a request message.(rv=%d)",
                  pTransc,rv));
        /*since the message was not sent, move back to the initial state*/
        pTransc->eState = eInitialState;
        TransactionTerminate(RVSIP_TRANSC_REASON_NETWORK_ERROR, pTransc);
        return rv;
    }
    /*the transaction was terminated when ACK failed to be sent*/
    if(pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlSendAck - Transaction 0x%p: Failed to send ACK. Transaction was terminated",
                  pTransc));
        return RV_Failure;
    }

    /*set the transaction tag*/
    if ((300 <= pTransc->responseCode) &&
        (RV_FALSE == pTransc->bTagInTo))
    {
        /* The tag should be updated to NULL for the following request
           since the response was not success, and the request did not include a tag */
        TransactionSetTag(pTransc, NULL, RV_TRUE);
    }


#ifdef SIP_DEBUG
    /* Update statistics */
    pTransc->pMgr->msgStatistics.sentNonInviteReq += 1;
#endif

     rv = TransactionStateUacInviteAckSent(
                                 RVSIP_TRANSC_REASON_USER_COMMAND,
                                 pTransc);

     return rv;
}






/***************************************************************************
* TransactionControlSendCancel
* ------------------------------------------------------------------------
* General: Cancels the transaction given as a parameter.
*          calling TransactionControlCancel() will cause A client CANCEL
*          transaction to be created with transaction key
*          taken from the transaction that is about to be canceled.
*          The CANCEL request will then be sent to the remote party.
*          If the transaction to cancel is an Invite transaction it will
*          assume the Invite Canceled state. A general transaction will assume
*          the general canceled state.
*          The newly created CANCEL transaction will assume the Cancel sent
*          state.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pTransc - Pointer to the transaction to cancel.
* Output:(-)
***************************************************************************/
RvStatus RVCALLCONV TransactionControlSendCancel (IN  Transaction  *pTransc)
{


    RvStatus           rv;
    Transaction         *pCancelTransc;

    /*---------------------------------------
       Create a new transaction
     ---------------------------------------*/
    rv = TransactionMgrCreateTransaction(pTransc->pMgr,RV_TRUE,
                                        (RvSipTranscHandle *)&pCancelTransc);
    if (rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlSendCancel - Failed to create a new cancel transaction (rv=%d)",rv));
        return rv;
    }

    rv = TransactionInitializeCancelTransc(pCancelTransc,pTransc);
    if (rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlSendCancel - Failed to set cancel transaction key (rv=%d)",rv));
        TransactionDestruct(pCancelTransc);
        return rv;
    }
    /*since the canceled transaction is locked, At this point the canceled transaction
     is also locked since their locks are shared*/
    if(pTransc->hOutboundMsg != NULL)
    {
        pCancelTransc->hOutboundMsg = pTransc->hOutboundMsg;
        pTransc->hOutboundMsg = NULL;
    }

    pTransc->hCancelTranscPair = (RvSipTranscHandle)pCancelTransc;
    pCancelTransc->hCancelTranscPair = (RvSipTranscHandle)pTransc;

    if(pCancelTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP &&
        pCancelTransc->pMgr->pAppEvHandlers.pfnEvInternalTranscCreated != NULL)
    {
        TransactionCallbackAppInternalClientCreatedEv(pCancelTransc);
    }

    /* cancel is sent to the same address as the initial request, so bResolve=false*/
    rv = TransactionControlSendRequest(pCancelTransc,pTransc->hRequestUri,RV_FALSE);

    if(rv != RV_OK  &&  pCancelTransc->eState != RVSIP_TRANSC_STATE_TERMINATED)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlSendCancel - Failed to send cancel request (rv=%d)",rv));
        if(pCancelTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP &&
            pCancelTransc->pMgr->pAppEvHandlers.pfnEvInternalTranscCreated != NULL)
        {
            TransactionTerminate(RVSIP_TRANSC_REASON_ERROR, pCancelTransc);
        }
        else
        {
            TransactionDestruct(pCancelTransc);
        }
        return rv;
    }

    if(pTransc->eState == RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING   ||
        pTransc->eState == RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING ||
        pTransc->eState == RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING_TIMEOUT)
    {

        rv = TransactionStateUacInviteCancelling(RVSIP_TRANSC_REASON_USER_COMMAND,
                                                 pTransc);
    }
    else
    {
        rv = TransactionStateUacGeneralCancelling(RVSIP_TRANSC_REASON_USER_COMMAND,
                                                  pTransc);
    }
    return rv;
}



/***************************************************************************
 * TransactionControlSendRequestMsg
 * ------------------------------------------------------------------------
 * General: Sends a request with a prepared message that was set to the
 *          transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc - Pointer to the transaction.
 *          bAddTopVia - indicate wether to add top via header before
 *                       sending the message.
 *    Output:    (-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionControlSendRequestMsg(IN  Transaction    *pTransc,
                                                     IN  RvBool         bAddTopVia)
{
    RvStatus              rv = RV_OK;
    SipTransactionKey     key;
    RvSipMethodType       eMethodType;
    RvSipTransactionState eInitialState = pTransc->eState;
    RvSipTransactionState     eOriginalLastState = pTransc->eLastState;

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "TransactionControlSendRequestMsg - Transaction 0x%p, a request is about to be sent",
              pTransc));


    /* Set transaction method*/
    eMethodType = RvSipMsgGetRequestMethod(pTransc->hOutboundMsg);
    TransactionSetMethod((RvSipTranscHandle)pTransc, eMethodType);
    if(eMethodType == RVSIP_METHOD_OTHER)
    {
        rv = TransactionSetMethodStrFromMsg((RvSipTranscHandle)pTransc, pTransc->hOutboundMsg);
        if(RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlSendRequestMsg - Transaction 0x%p: Error - Setting other method (rv=%d)",
                  pTransc,rv));
            return(rv);
        }
    }


    /* Add top via to the message if indicated*/
    if(RV_TRUE == bAddTopVia)
    {
        rv = TransactionAddViaToMsg(pTransc, pTransc->hOutboundMsg);
        if (RV_OK != rv)
        {
            return rv;
        }
    }

    /*Update transaction top via header*/
    rv = TransactionUpdateTopViaFromMsg(pTransc, pTransc->hOutboundMsg);
    if(RV_OK != rv)
    {
        return rv;
    }


    /* Check if a transaction with this key already exists*/
    rv = TransactionGetKeyFromTransc(pTransc, &key);
    if (rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionControlSendRequestMsg - Transaction 0x%p, failed to get key",
            pTransc));
        return rv;
    }

    if (RV_TRUE == TransactionMgrCheckIfExists((TransactionMgr *)(pTransc->pMgr),
                                                &key, pTransc->eMethod,
                                                pTransc->strMethod,    RV_TRUE,
                                                pTransc->hOutboundMsg))
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlSendRequestMsg - Error trying to request with a duplicated transaction 0x%p.",
                  pTransc));
        return RV_ERROR_OBJECT_ALREADY_EXISTS;
    }

    /* Update transaction request uri */
    rv = TransactionSetRequestUri(pTransc, RvSipMsgGetRequestUri(pTransc->hOutboundMsg));
    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlSendRequestMsg - Transaction 0x%p: Error setting transaction request uri. (rv=%d)",
                  pTransc,rv));
        return rv;
    }

    /* SPIRENT_BEGIN */
#if !defined(UPDATED_BY_SPIRENT)
    /* Call "message to send" callback */
    rv= TransactionCallbackMsgToSendEv(pTransc, pTransc->hOutboundMsg);
    if (RV_OK != rv)
    {
      return rv;
    }
#endif
    /* SPIRENT_END */

    /*Update transaction top via header*/
    rv = TransactionUpdateTopViaFromMsg(pTransc, pTransc->hOutboundMsg);
    if(RV_OK != rv)
    {
        return rv;
    }
    /* Insert the transaction handle to the hash table */
    rv = TransactionMgrHashInsert((RvSipTranscHandle)pTransc);
    if(RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlSendRequestMsg - Transaction 0x%p: Error trying to insert the transaction to the hash table. (rv=%d)",
                  pTransc,rv));
        return (rv);
    }

    /*-----------------
      Send the message
     -----------------*/
    /* updating the last state, in order of determining the reason */
    pTransc->eLastState = pTransc->eState;
    pTransc->eState = TransactionGetNextUacState(pTransc);
    pTransc->msgType = SIP_TRANSPORT_MSG_REQUEST_SENT;
    pTransc->retransmissionsCount=0;

    /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    /* Call "message to send" callback */
    rv= TransactionCallbackMsgToSendEv(pTransc, pTransc->hOutboundMsg);
    if (RV_OK != rv)
    {
      return rv;
    }
#endif
    /* SPIRENT_END */

    rv = TransactionTransportSendMsg(pTransc, RV_TRUE);


    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlSendRequestMsg - Transaction 0x%p: Error in trying to send a request message.(rv=%d)",
                  pTransc,rv));
        /*Remove transaction from manager hash table*/
        TransactionMgrHashRemove((RvSipTranscHandle)pTransc);
        pTransc->eState = eInitialState;
        pTransc->eLastState = eOriginalLastState;
        return (rv);
    }

#ifdef SIP_DEBUG
    /* Update statistics */
    if (RVSIP_METHOD_INVITE == eMethodType)
    {
        pTransc->pMgr->msgStatistics.sentINVITE += 1;
    }
    else
    {
        pTransc->pMgr->msgStatistics.sentNonInviteReq += 1;
    }
#endif

    rv = TransactionChangeToNextUacState(pTransc,
                                         RVSIP_TRANSC_REASON_USER_COMMAND);


    return rv;
}

/***************************************************************************
 * TransactionControlSendRespondMsg
 * ------------------------------------------------------------------------
 * General: Sends a response with a prepared message that was set to the
 *          transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - Pointer to the transaction.
 *            bRemoveTopVia - indicate wether to remove top via header before
 *                          sending the message.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionControlSendRespondMsg(IN  Transaction  *pTransc,
                                                     IN  RvBool        bRemoveTopVia)
{
    RvStatus              rv = RV_OK;
    RvUint16              responseCode = 200;
    RvSipTransactionState eInitialState = pTransc->eState;
    RvSipTransactionState eOriginalLastState = pTransc->eLastState;

    responseCode = RvSipMsgGetStatusCode(pTransc->hOutboundMsg);

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "TransactionControlSendRespondMsg - Transaction 0x%p: about to send %d response",
              pTransc,responseCode));

    rv = SynchronizeTranscAndMsgToTags(pTransc,pTransc->hOutboundMsg);
    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlSendRespondMsg - Transaction 0x%p: Error trying to sync tags.(rv=%d)",
                  pTransc,rv));
        return rv;
    }

    if(bRemoveTopVia == RV_TRUE)
    {
        rv = TransactionRemoveTopViaFromMsg(pTransc->pMgr, pTransc->hOutboundMsg);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                     "TransactionControlSendRespondMsg - Transaction 0x%p:Failed to remove top Via from message 0x%p (rv=%d)",
                      pTransc,pTransc->hOutboundMsg,rv));
            return rv;
        }
    }

    pTransc->responseCode = responseCode;

    /* Call "message to send" callback */
    rv= TransactionCallbackMsgToSendEv(pTransc, pTransc->hOutboundMsg);
    if (RV_OK != rv)
    {
         return rv;
    }
    /*-------------------
       Send the message
     --------------------*/
    /* updating the last state, in order of determining the reason */
    pTransc->eLastState = pTransc->eState;
    pTransc->eState = TransactionGetNextSrvState(pTransc, responseCode, RV_FALSE);
    pTransc->retransmissionsCount=0;
    if((responseCode >= 200 && responseCode<300) &&
       (RV_TRUE == pTransc->bIsProxy) &&
       (SIP_TRANSACTION_METHOD_INVITE == pTransc->eMethod))
    {
            pTransc->msgType = SIP_TRANSPORT_MSG_PROXY_2XX_SENT;
    }
    else
    {
        if(responseCode < 200)
        {
            pTransc->msgType = SIP_TRANSPORT_MSG_PROV_RESP_SENT;
        }
        else
        {
            pTransc->msgType = SIP_TRANSPORT_MSG_FINAL_RESP_SENT;
        }
    }        


    rv = TransactionTransportSendMsg(pTransc,RV_TRUE);
    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlSendRespondMsg - Transaction 0x%p: Error in trying to send a reponse message.(rv=%d)",
                  pTransc,rv));
        pTransc->eState = eInitialState;
        pTransc->eLastState = eOriginalLastState;
        return rv;
    }

#ifdef SIP_DEBUG
    /* Update statistics */
    pTransc->pMgr->msgStatistics.sentResponse += 1;
#endif

    /* Change to next server transaction state*/
    rv = TransactionControlChangeToNextSrvState(pTransc, eInitialState, responseCode,
                                         RV_FALSE, RVSIP_TRANSC_REASON_USER_COMMAND);
    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlSendRespondMsg - Transaction 0x%p: changing to next server state (rv=%d).",
                  pTransc,rv));
        return rv;
    }
    return RV_OK;
}


/***************************************************************************
 * TransactionControlChangeToNextSrvState
 * ------------------------------------------------------------------------
 * General: Call the next server transaction state according to the
 *          trasnaction current state, method response code and
 *          the proxy indication
 * Return Value: next transaction state.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction.
 *        uRespCode - server response code.
 *        bIsReliable - indication whether the response is a reliable response
 *        eReason    - reason of changing the state.
 * output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionControlChangeToNextSrvState(
                            IN  Transaction*                      pTransc,
                            IN  RvSipTransactionState             ePrevState,
                            IN  RvUint16                         uRespCode,
                            IN  RvBool                           bIsReliable,
                            IN  RvSipTransactionStateChangeReason eReason)
{
    RvStatus rv = RV_OK;


    if(ePrevState == pTransc->eState
        && pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_PROXY_2XX_RESPONSE_SENT)
    {
        return RV_OK;
    }

    if(uRespCode >= 200)
    {
        switch(pTransc->eMethod)
        {
        case SIP_TRANSACTION_METHOD_INVITE:
            {
                /* If the transaction is a proxy transaction and the response code
                   is 2xx, move to proxy state machine */
                if(uRespCode >= 200 && uRespCode < 300 && pTransc->bIsProxy)
                {
                    rv = TransactionStateProxyInvite2xxRespSent(eReason,pTransc);

                }
                else
                {
                    rv = TransactionStateSrvInviteFinalRespSent(eReason,pTransc);
                }
            }
            break;
        case SIP_TRANSACTION_METHOD_CANCEL:
                    rv = TransactionStateServerCancelFinalRespSent(eReason,pTransc);
            break;
#ifndef RV_SIP_PRIMITIVES
        case SIP_TRANSACTION_METHOD_PRACK:
             if(pTransc->hPrackParent != NULL)
             {
                 if(((Transaction*)pTransc->hPrackParent)->eState ==
                     RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT)
                 {
                     TransactionStateSrvInvitePrackCompeleted(
                         RVSIP_TRANSC_REASON_TRANSACTION_COMMAND,
                         ((Transaction*)pTransc->hPrackParent));
                 }
                 rv = TransactionStateSrvGeneralFinalRespSent(eReason,
                                                               pTransc);
             }
             else
             {
                 rv = TransactionStateSrvGeneralFinalRespSent(eReason,
                                                              pTransc);

             }
            break;
#endif /*#ifndef RV_SIP_PRIMITIVES */
        default:
                    rv = TransactionStateSrvGeneralFinalRespSent(eReason,
                                                                 pTransc);
            break;
        }
    }

    /*if the respone is provisional - change state only for reliable
      provisional response*/
    if(uRespCode < 200)
    {
        rv = RV_OK;
#ifndef RV_SIP_PRIMITIVES

        if(bIsReliable && uRespCode>100)
        {
            rv =  TransactionStateSrvInviteReliableProvRespSent(eReason,
                                                                pTransc);
        }
#else
        RV_UNUSED_ARG(bIsReliable);
#endif
    }

    return(rv);
}
/***************************************************************************
 * TransactionControlGenerateAndSendResponse
 * ------------------------------------------------------------------------
 * General: Creates and sends a response message.
 * Return Value: RV_OK - The message was created and sent successfully.
 *                 RV_ERROR_UNKNOWN - An error occured while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc -   The transaction this request refers to.
 *            responseCode - The response code for the response message
 *            strReasonPhrase - The reason phrase for the response
 *            bIsReliable - RV_TRUE if the response should be sent reliably.
 * output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionControlGenerateAndSendResponse(
                                    IN  Transaction    *pTransc,
                                    IN  RvUint16        responseCode,
                                    IN  RvChar         *strReasonPhrase,
                                    IN  RvBool          bIsReliable)
{
    RvStatus          rv;
    RvSipTransactionState eInitialState = pTransc->eState;

    RvSipTransactionState eOriginalLastState = pTransc->eLastState;

    pTransc->responseCode = responseCode;

    /* Add tag to the To header, if missing */
    rv = TransactionAddTag(pTransc, RV_TRUE);
    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionControlGenerateAndSendResponse - Transaction 0x%p: Error trying to generate To tag.",
            pTransc));
        return rv;
    }


    /* Create a response message with the To, From, Call-Id, CSeq of the
    transaction, the given Response-Code and Reason-Phrase,
    and the INVITE Method in the CSeq header*/
    rv = TransactionCreateResponseMessage(pTransc,
        responseCode, strReasonPhrase,
        bIsReliable,
        pTransc->hOutboundMsg);
    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionControlGenerateAndSendResponse - Transaction 0x%p: Error in trying to create a reponse message.(rv=%d)",
            pTransc,rv));
        return rv;
    }

    /* If the transaction is a Proxy transaction, remove the tag from the to header
    proxies don't add tags to messages*/
    if((RV_TRUE == pTransc->bIsProxy) &&
        (RV_FALSE == pTransc->bTagInTo))
    {
        TransactionSetTag(pTransc, NULL, RV_TRUE);
    }
    /* verify that the message has the correct tags, and if not - set it to the message */
    rv = VerifyMsgToFromHeaders(pTransc, pTransc->hOutboundMsg);
    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlGenerateAndSendResponse - Transaction 0x%p: Error in setting tags in message.(rv=%d)",
                  pTransc,rv));
        return rv;
    }

    /* Call "message to send" callback */
    rv=TransactionCallbackMsgToSendEv(pTransc, pTransc->hOutboundMsg);
    if (RV_OK != rv)
    {
            return rv;
    }

    /*-------------------
       Send the message
      --------------------*/
    /* updating the last state, in order to determine the reason */
    pTransc->eLastState = pTransc->eState;

    pTransc->eState = TransactionGetNextSrvState(pTransc, responseCode, bIsReliable);
    if ((100 <= responseCode) && (200 > responseCode))
    {
        if (bIsReliable== RV_TRUE)
        {
            pTransc->msgType = SIP_TRANSPORT_MSG_PROV_RESP_REL_SENT;
        }
        else
        {
            pTransc->msgType = SIP_TRANSPORT_MSG_PROV_RESP_SENT;
        }
    }
    else
    {
        if((responseCode >= 200 && responseCode<300) &&
            (RV_TRUE == pTransc->bIsProxy) &&
            (SIP_TRANSACTION_METHOD_INVITE == pTransc->eMethod))
        {
            pTransc->msgType = SIP_TRANSPORT_MSG_PROXY_2XX_SENT;
        }
        else
        {

            pTransc->msgType = SIP_TRANSPORT_MSG_FINAL_RESP_SENT;
        }
    }
    rv = TransactionTransportSendMsg(pTransc, RV_TRUE);

    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlGenerateAndSendResponse - Transaction 0x%p: Error in trying to send a reponse message.(rv=%d)",
                  pTransc,rv));
        pTransc->eState = eInitialState;
        pTransc->eLastState = eOriginalLastState;
        return rv;
    }

#ifdef SIP_DEBUG
    /* Update statistics */
    pTransc->pMgr->msgStatistics.sentResponse += 1;
#endif

    if ( (200 <= responseCode && 700 > responseCode) ||
         (101 <= responseCode && 200 > responseCode && bIsReliable == RV_TRUE))
    {
        /* The response is final */
        RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlGenerateAndSendResponse - Transaction 0x%p: Final Response was sent (rv=%d)",
                  pTransc,rv));
    }
    else if ((100 <= responseCode) && (200 > responseCode))
    {
        /* The response is provisional */
        RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionControlGenerateAndSendResponse - Transaction 0x%p: Provisional Response was sent",
                  pTransc));
    }
    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * GenerateAndSendRequest
 * ------------------------------------------------------------------------
 * General: Creates and sends a general request message, according to the
 *          given method with the given address as a Request-Uri.
 *          the method is given as a string.
 * Return Value: RV_OK - The message was created and sent successfully.
 *                 RV_ERROR_UNKNOWN - An error occured while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc -   The transaction this request refers to.
 *            hRequestUri -    The Request-Uri to be used in the request
 *                           message.
 *                           The message's Request-Uri will be the address
 *                           given here, and the message will be sent
 *                           accordingly.
 *        bResolveAddr - RV_TRUE if we want the transmitter to resolve the dest addr
 *                       from the message, RV_FALSE if the transmitter should use the
 *                       address it currently hold.
 ***************************************************************************/
static RvStatus RVCALLCONV GenerateAndSendRequest(
                                    IN  Transaction         *pTransc,
                                    IN  RvSipAddressHandle   hRequestUri,
                                    IN  RvSipMsgHandle       hMsgToSend,
                                    IN  RvBool               bResolveAddress)
{

    RvStatus           retStatus;
    SipTransactionKey  key;
	RvStatus		   rv;
	RvInt32			   cseqStep;
#ifdef SIP_DEBUG
    RvSipMethodType    eMethod;
#endif /* #ifdef SIP_DEBUG */

    RvSipTransactionState     eOriginalLastState = pTransc->eLastState;

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "GenerateAndSendRequest - a request is about to be generated and send on transaction 0x%p",pTransc));

    /* Randomly generate Cseq-Step in case it is not defined */
	rv = SipCommonCSeqGetStep(&pTransc->cseq,&cseqStep); 
    if (rv != RV_OK)
    {
		cseqStep = TransactionRand(pTransc);
		SipCommonCSeqSetStep(cseqStep,&pTransc->cseq);
    }
    /* Check if a transaction with this key already exists*/
    TransactionGetKeyFromTransc(pTransc, &key);

    /* Add tag to the From header, if missing */
    retStatus = TransactionAddTag(pTransc, RV_FALSE);
    if (RV_OK != retStatus)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "GenerateAndSendRequest - Transaction 0x%p: Error trying to generate From tag.(rv=%d)",
                  pTransc,retStatus));
        return retStatus;
    }

//NGA: the following codes cause problem for all subscription w/ transport=tcp; comment them out for now
#if defined(UPDATED_BY_SPIRENT)
    /*set the request uri in the transation*/
    retStatus = TransactionSetRequestUri(pTransc,hRequestUri);
    {
        if (RV_OK != retStatus)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "GenerateAndSendRequest - Transaction 0x%p: Error in trying to set the request uri.(rv=%d)",
                pTransc,retStatus));
            return retStatus;
        }
    }

    /* Correct Request URI */
    /* Delete the MAddr parameter of the Sip-Url*/
    RvSipAddrUrlSetMaddrParam(hRequestUri, NULL);
    /* Delete the Transport parameter of the Sip-Url if the configuration doesn't include it*/
    if (!(pTransc->URI_Cfg_Flag & URI_CFG_TRANSPORT))
       RvSipAddrUrlSetTransport(hRequestUri, RVSIP_TRANSPORT_UNDEFINED, NULL);

    /* Delete port of the Sip-Url if the configuration doesn't include it*/
    if (!(pTransc->URI_Cfg_Flag & URI_CFG_PORT))
       RvSipAddrUrlSetPortNum(hRequestUri, UNDEFINED);

#endif

    retStatus = TransactionCreateRequestMessage(pTransc,hRequestUri, hMsgToSend);
    if (RV_OK != retStatus)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "GenerateAndSendRequest - Transaction 0x%p: Error in trying to create a request message.(rv=%d)",
            pTransc,retStatus));
        return retStatus;
    }

    /* Update transaction top via header */
    retStatus = TransactionUpdateTopViaFromMsg(pTransc, hMsgToSend);
    if(RV_OK != retStatus)
    {
        return retStatus;
    }
    
    /* update it the transaction is initial request or not */
    {
        if(0 < RvSipPartyHeaderGetStringLength(key.hToHeader, RVSIP_PARTY_TAG))
        {
            pTransc->bInitialRequest = RV_FALSE; /* has a to-tag */
        }
        else
        {
            pTransc->bInitialRequest = RV_TRUE;
        }
    }

    /* Need to check the if such transaction exist on the transaction manager hash table*/
    if (RV_TRUE == TransactionMgrCheckIfExists((TransactionMgr *)(pTransc->pMgr),
                                                &key, pTransc->eMethod,
                                                pTransc->strMethod,    RV_TRUE, hMsgToSend))
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "GenerateAndSendRequest(pTransc=0x%p) - Error trying to request with a duplicated transaction",
                  pTransc));
        return RV_ERROR_OBJECT_ALREADY_EXISTS;
    }

//NGA: the above codes at line 1099 has been commented out because it cause problem for all subscription w/ transport=tcp; so, bring back the following codes
//#if !defined(UPDATED_BY_SPIRENT)
#if 1
    /*set the request uri in the transation*/
    retStatus = TransactionSetRequestUri(pTransc,hRequestUri);
    {
        if (RV_OK != retStatus)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "GenerateAndSendRequest - Transaction 0x%p: Error in trying to set the request uri.(rv=%d)",
                pTransc,retStatus));
            return retStatus;
        }
    }
#endif

#ifndef RV_SIP_PRIMITIVES
    retStatus = AddReferredByHeaderIfExists(pTransc,hMsgToSend);
#endif /*#ifndef RV_SIP_PRIMITIVES */

/* SPIRENT_BEGIN */
#if !defined(UPDATED_BY_SPIRENT)
    /* Call "message to send" callback */
    retStatus= TransactionCallbackMsgToSendEv(pTransc, hMsgToSend);
    if (RV_OK != retStatus)
    {
         /* Destruct the message object */
         return retStatus;
    }
#endif
/* SPIRENT_END */

#ifdef SIP_DEBUG
    eMethod = RvSipMsgGetRequestMethod(hMsgToSend);
#endif /* #ifdef SIP_DEBUG */

    /*Update transaction top via header*/
    retStatus = TransactionUpdateTopViaFromMsg(pTransc, hMsgToSend);
    if(RV_OK != retStatus)
    {
        return retStatus;
    }

    retStatus = TransactionMgrHashInsert((RvSipTranscHandle)pTransc);
    if(RV_OK != retStatus)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "GenerateAndSendRequest - Transaction 0x%p: Error trying to insert the transaction to the hash table.(rv=%d)",
                  pTransc,retStatus));
        return retStatus;
    }

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    /* Call "message to send" callback */
    retStatus= TransactionCallbackMsgToSendEv(pTransc, hMsgToSend);
    if (RV_OK != retStatus)
    {
         /* Destruct the message object */
         return retStatus;
    }
#endif
/* SPIRENT_END */

    /*-----------------
      Send the message
     -----------------*/
    /* updating the last state, in order of determining the reason */
    pTransc->eLastState = pTransc->eState;
    /*move internally to the new state*/
    pTransc->eState = TransactionGetNextUacState(pTransc);

    pTransc->msgType = SIP_TRANSPORT_MSG_REQUEST_SENT;

    retStatus = TransactionTransportSendMsg(pTransc, bResolveAddress);

    if (RV_OK != retStatus)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "GenerateAndSendRequest - Transaction 0x%p: Error in trying to send a request message.(rv=%d)",
                  pTransc,retStatus));
        pTransc->eState = pTransc->eLastState;
        pTransc->eLastState = eOriginalLastState;



#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
        if(retStatus != RV_ERROR_TRY_AGAIN ||
           pTransc->eState != RVSIP_TRANSC_STATE_TERMINATED)
        { /* If we work with DNS and there was a network problem the stack might have
             already killed the transaction after the user received a msg_send_failure event
             and called the DNScontinue() function */
#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/

            /*Remove transaction from manager hash table*/
            TransactionMgrHashRemove((RvSipTranscHandle)pTransc);


#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
        }
#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/
        return retStatus;
    }

#ifdef SIP_DEBUG
    if (RVSIP_METHOD_INVITE == eMethod)
    {
        pTransc->pMgr->msgStatistics.sentINVITE += 1;
    }
    else
    {
        pTransc->pMgr->msgStatistics.sentNonInviteReq += 1;
    }
#endif /* #ifdef SIP_DEBUG */
    return RV_OK;
}





/***************************************************************************
 * TransactionGetNextSrvState
 * ------------------------------------------------------------------------
 * General: Return the next server transaction state (enum) according to the
 *          trasnaction current state, method response code and
 *          the proxy indication
 * Return Value: next transaction state.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction.
 *        uRespCode - server response code.
 *        bIsReliable - indication whether the response is reliable response
 ***************************************************************************/
static RvSipTransactionState RVCALLCONV TransactionGetNextSrvState(IN Transaction* pTransc,
                                                            IN RvUint16    uRespCode,
                                                            IN RvBool      bIsReliable)
{
    RvSipTransactionState eNextState = pTransc->eState;
    if(uRespCode >= 200)
    {
        switch(pTransc->eMethod)
        {
        case SIP_TRANSACTION_METHOD_INVITE:
            {
                /* If the transaction is a proxy transaction and the response code
                   is 2xx, move to proxy state machine */
                if(uRespCode >= 200 && uRespCode < 300 && pTransc->bIsProxy)
                {
                    eNextState = RVSIP_TRANSC_STATE_SERVER_INVITE_PROXY_2XX_RESPONSE_SENT;
                }
                else
                {
                    eNextState = RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT;
                }
            }
            break;
        case SIP_TRANSACTION_METHOD_CANCEL:
            eNextState = RVSIP_TRANSC_STATE_SERVER_CANCEL_FINAL_RESPONSE_SENT;
            break;
#ifndef RV_SIP_PRIMITIVES
        case SIP_TRANSACTION_METHOD_PRACK:
            eNextState = RVSIP_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT;
            break;
#endif /*#ifndef RV_SIP_PRIMITIVES */
        default:
            eNextState = RVSIP_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT;
            break;
        }
    }
#ifndef RV_SIP_PRIMITIVES
    else
    {
        if(bIsReliable == RV_TRUE)
        {
            eNextState = RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT;
        }
    }
#endif /* #ifndef RV_SIP_PRIMITIVES  */

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "TransactionGetNextSrvState - Transaction 0x%p: method= %d response= %d IsReliable= %d next state= %s",
              pTransc, pTransc->eMethod, uRespCode, bIsReliable,TransactionGetStateName(eNextState)));
    return(eNextState);
}


/***************************************************************************
 * TransactionGetNextUacState
 * ------------------------------------------------------------------------
 * General: Return the next client transaction state (enum) according to the
 *          trasnaction current state, method and the proxy indication
 * Return Value: next transaction state.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction.
 ***************************************************************************/
static RvSipTransactionState RVCALLCONV TransactionGetNextUacState(IN Transaction* pTransc)
{
    RvSipTransactionState eNewState;

    switch(pTransc->eMethod)
    {
    case SIP_TRANSACTION_METHOD_INVITE:
        /* The transaction is in final resopnse received, The request must be ACK*/
        if(RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD == pTransc->eState)
        {
            eNewState = RVSIP_TRANSC_STATE_CLIENT_INVITE_ACK_SENT;
        }
        /* The transaction is an invite request*/
        else
        {
            eNewState = RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING;
        }
        break;
    case SIP_TRANSACTION_METHOD_CANCEL:
        eNewState = RVSIP_TRANSC_STATE_CLIENT_CANCEL_SENT;
        break;
    default:
        eNewState = RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT;
        break;
    }
    return(eNewState);
}

/***************************************************************************
 * TransactionChangeToNextUacState
 * ------------------------------------------------------------------------
 * General: Call the next client transaction state according to the
 *          trasnaction current state, method and the proxy indication
 * Return Value: next transaction state.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction.
 *        eReason    - reason of changing the state.
 *Output: (-)
 ***************************************************************************/
static RvStatus RVCALLCONV TransactionChangeToNextUacState(
                                    IN  Transaction                          *pTransc,
                                    IN  RvSipTransactionStateChangeReason eReason)
{
    RvStatus rv = RV_OK;
    /*if the state is already terminated don't move to any other state*/
    if(pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
    {
        return RV_OK;
    }


    switch(pTransc->eMethod)
    {
    case SIP_TRANSACTION_METHOD_INVITE:
        if(RVSIP_TRANSC_STATE_CLIENT_INVITE_ACK_SENT == pTransc->eState)
        {
            rv = TransactionStateUacInviteAckSent(eReason, pTransc);
        }
        else
        {
            rv = TransactionStateUacInviteCalling(eReason, pTransc);
        }
        break;

    case SIP_TRANSACTION_METHOD_CANCEL:
        rv = TransactionStateUacCancelReqMsgSent(eReason, pTransc);
        break;

    default:
        rv = TransactionStateUacGeneralReqMsgSent(eReason, pTransc);
        break;
    }
    return(rv);
}

/***************************************************************************
 * SynchronizeTranscAndMsgToTags
 * ------------------------------------------------------------------------
 * General: Synchronizes the tags between the response message and the
 *          transaction. If the tag is found in one of them it is copied to
 *          the other. If none of them has tag is is generated and set it
 *          both.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction.
 *        hMsg    - The response message.
 ***************************************************************************/
static RvStatus RVCALLCONV SynchronizeTranscAndMsgToTags(IN Transaction* pTransc,
                                                          IN RvSipMsgHandle hMsg)
{
    RvStatus rv;
    /*if the message does not have a tag add the transaction tag*/
    RvSipPartyHeaderHandle hTo = RvSipMsgGetToHeader(hMsg);
    RvInt32 msgToTagOffset = SipPartyHeaderGetTag(hTo);
    RvInt32 transcToTagOffset = SipPartyHeaderGetTag(pTransc->hToHeader);

    if(msgToTagOffset == UNDEFINED && transcToTagOffset != UNDEFINED)
    {
        /*copy the tag from the transaction object to the message*/
        transcToTagOffset = SipPartyHeaderGetTag(pTransc->hToHeader);
        rv = SipPartyHeaderSetTag(hTo,NULL,pTransc->pMgr->hGeneralPool,
            pTransc->memoryPage,transcToTagOffset);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "SynchronizeTranscAndMsgToTags - Transaction 0x%p: Failed to copy tag to the response message 0x%p (rv=%d)",
                 pTransc,hMsg,rv));
            return rv;
        }
        return RV_OK;
    }

    if(msgToTagOffset    != UNDEFINED &&
       transcToTagOffset == UNDEFINED &&
       pTransc->bIsProxy != RV_TRUE)
    {
        rv = TransactionCopyToTag(pTransc,hMsg,RV_FALSE);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "SynchronizeTranscAndMsgToTags - Transaction 0x%p: Failed to copy to tag from msg 0x%p to transaction (rv=%d)",
                 pTransc,hMsg,rv));
            return rv;
        }
        return RV_OK;
    }

    if(msgToTagOffset != UNDEFINED && transcToTagOffset != UNDEFINED)
    {
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
        if(SPIRENT_RPOOL_Stricmp(pTransc->pMgr->hGeneralPool,pTransc->memoryPage,
            transcToTagOffset, SipPartyHeaderGetPool(hTo),
            SipPartyHeaderGetPage(hTo),msgToTagOffset) != 0)
#else /* !defined(UPDATED_BY_SPIRENT) */
        if(RPOOL_Stricmp(pTransc->pMgr->hGeneralPool,pTransc->memoryPage,
            transcToTagOffset, SipPartyHeaderGetPool(hTo),
            SipPartyHeaderGetPage(hTo),msgToTagOffset) == RV_FALSE)
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "SynchronizeTranscAndMsgToTags - Transaction 0x%p: and msg 0x%p have different to tags",pTransc,hMsg));
            return RV_ERROR_UNKNOWN;

        }
    }
    return RV_OK;

}

/***************************************************************************
 * CopyPartyHeaderNoTag
 * ------------------------------------------------------------------------
 * General: Compares the party header in the outgoing response message,
 *          to the party header received in the request message, that was 
 *          saved in the transaction.
 *          if the tag or address is different - set the values from the 
 *          request message to the response message.
 * Return Value: RV_Status.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction.
 *        hMsg    - The response message.
 ***************************************************************************/
static RvStatus RVCALLCONV CopyPartyHeaderNoTag(
                            IN Transaction*           pTransc,
                            IN RvSipPartyHeaderHandle hMsgParty,
                            IN RvSipPartyHeaderHandle hTranscParty)
{
    RvStatus    rv     = RV_OK;
    RvSipAddressHandle hTranscAddr;
    RvInt32     msgTagOffset = UNDEFINED, transcTagOffset = UNDEFINED;
    RvInt32     transcOffset = UNDEFINED;
    HRPOOL      transcPool = SipPartyHeaderGetPool(hTranscParty);
    HPAGE       transcPage = SipPartyHeaderGetPage(hTranscParty);
    
    hTranscAddr = RvSipPartyHeaderGetAddrSpec(hTranscParty);

    /* address */
    /* copy the transc address to the msg address */
    rv = RvSipPartyHeaderSetAddrSpec(hMsgParty, hTranscAddr);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "CopyPartyHeaderNoTag: transc 0x%p - Failed to set address in message party header",pTransc));
        return rv;
    }
    
    /*display name */
#ifndef RV_SIP_JSR32_SUPPORT
    transcOffset = SipPartyHeaderGetDisplayName(hTranscParty);
    rv = SipPartyHeaderSetDisplayName(hMsgParty, NULL, transcPool, transcPage, transcOffset);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "CopyPartyHeaderNoTag: transc 0x%p - Failed to set display-name in message party header",pTransc));
        return rv;
    }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

    /* other params */
    transcOffset = SipPartyHeaderGetOtherParams(hTranscParty);
    rv = SipPartyHeaderSetOtherParams(hMsgParty, NULL, transcPool, transcPage, transcOffset);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "CopyPartyHeaderNoTag: transc 0x%p - Failed to set other params in message party header",pTransc));
        return rv;
    }

    /* tags */
    msgTagOffset = SipPartyHeaderGetTag(hMsgParty);
    transcTagOffset = SipPartyHeaderGetTag(hTranscParty);
    
    if((UNDEFINED == msgTagOffset && UNDEFINED != transcTagOffset))
    {
        /* set the tag kept in the transaction to the message */
        rv = SipPartyHeaderSetTag(  hMsgParty, NULL, 
                                    SipPartyHeaderGetPool(hTranscParty),
                                    SipPartyHeaderGetPage(hTranscParty),
                                    transcTagOffset);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "CopyPartyHeaderNoTag: transc 0x%p - Failed to set tag in message party header",pTransc));
            return rv;
        }
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransc);
#endif

    return RV_OK;
}
                                                
/***************************************************************************
 * VerifyMsgToFromHeaders
 * ------------------------------------------------------------------------
 * General: verify that the tags in the response message, are the same as
 *          in the incoming request.
 *          if the tags are different - set the request tags to the response.
 * Return Value: RV_Status.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction.
 *        hMsg    - The response message.
 ***************************************************************************/
static RvStatus RVCALLCONV VerifyMsgToFromHeaders(IN Transaction* pTransc,
                                                  IN RvSipMsgHandle hMsg)
{
    RvStatus              rv = RV_OK;
    RvSipPartyHeaderHandle hPartyHeader;
   
    hPartyHeader = RvSipMsgGetToHeader(hMsg);
     /* if the reqMsgToHeader has no to-tag, and the message party header has a to-tag
       we must not overwrite it */
    rv = CopyPartyHeaderNoTag(pTransc, hPartyHeader, pTransc->reqMsgToHeader);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "VerifyMsgToFromHeaders: transc 0x%p - Failed to copy To header from request to response",pTransc));
        return rv;
        
    }
   
    hPartyHeader = RvSipMsgGetFromHeader(hMsg);
    rv = CopyPartyHeaderNoTag(pTransc, hPartyHeader, pTransc->reqMsgFromHeader);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "VerifyMsgToFromHeaders: transc 0x%p - Failed to copy From header from request to response",pTransc));
        return rv;   
    }
    return rv;
}
/***************************************************************************
 * VerifyAckMsgToHeader
 * ------------------------------------------------------------------------
 * General: verify that the to-tag in the response message, is the same as
 *          in the incoming response.
 *          if the tags are different - set the response tags to the response.
 * Return Value: RV_Status.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction.
 *        hMsg    - The response message.
 ***************************************************************************/
static RvStatus RVCALLCONV VerifyAckMsgToHeader(IN Transaction* pTransc,
                                             IN RvSipMsgHandle hMsg)
{
    RvStatus              rv = RV_OK;
    RvSipPartyHeaderHandle hPartyHeader;

    if(pTransc->non2xxRespMsgToHeader == NULL)
    {
        return RV_OK;
    }
    hPartyHeader = RvSipMsgGetToHeader(hMsg);

    /* 17.1.1.3 Construction of the ACK Request
    ... The To header field in the ACK MUST equal the To header field in the
    response being acknowledged, and therefore will usually differ from
    the To header field in the original request by the addition of the
    tag parameter.*/
    rv = RvSipPartyHeaderCopy(hPartyHeader, pTransc->non2xxRespMsgToHeader);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "VerifyAckMsgToHeader: transc 0x%p - failed to update to-header in ACK msg",pTransc));
    
    }
    return RV_OK;
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * AddReferredByHeaderIfExists
 * ------------------------------------------------------------------------
 * General: Adds a ReferredBy header to message if it's configured in the
 *          Transaction.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction.
 *        hMsg    - The handled message.
 ***************************************************************************/
static RvStatus RVCALLCONV AddReferredByHeaderIfExists(IN Transaction   *pTransc,
                                                       IN RvSipMsgHandle hMsgToSend)
{
    RvStatus retStatus = RV_OK;

#ifdef RV_SIP_SUBS_ON
    if(pTransc->hReferredBy != NULL)
    {
        RvSipHeaderListElemHandle hElem;
        void* pTempPointer;

        retStatus = RvSipMsgPushHeader(hMsgToSend, RVSIP_FIRST_HEADER, (void*)pTransc->hReferredBy,
            RVSIP_HEADERTYPE_REFERRED_BY, &hElem, &pTempPointer);
        if(retStatus != RV_OK)
        {
            RvLogWarning(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "AddReferredByHeaderIfExists - Transaction 0x%p: Failed to set Referred-By header in msg 0x%p.(rv=%d)",
                pTransc,hMsgToSend,retStatus));
        }
    }
#else /* #ifdef RV_SIP_SUBS_ON */

	RV_UNUSED_ARG(pTransc);
	RV_UNUSED_ARG(hMsgToSend);

#endif /* #ifdef RV_SIP_SUBS_ON */

    return retStatus;
}

#endif /* #ifndef RV_SIP_PRIMITIVES */
#endif /*#ifndef RV_SIP_LIGHT*/

