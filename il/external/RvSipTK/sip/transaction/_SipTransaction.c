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
 *                              <SipTransaction.c>
 *
 * This file defines the transaction object, interface
 * functions. A transaction object handle is called RvSipTranscHandle.
 * The interface functions defined here implement methods to
 * construct and destruct SipTransaction object; to attach an owner to the
 * transaction object, and a set of callbacks belonging to this owner; to attach
 * a lock to the inner lock handle of the object; and to get all the key fields
 * of a transaction. A few functions are defined to manipulate transactions.
 * These functions are also defined here and they are: Terminate a transaction
 * object, Request, Respond and Acknowledge.
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

#include "RvSipVersion.h"

#include "_SipTransaction.h"
#include "_SipTransactionMgr.h"
#include "TransactionState.h"
#include "TransactionMgrObject.h"
#include "TransactionObject.h"
#include "TransactionTransport.h"
#include "TransactionAuth.h"
#include "AdsRpool.h"
#include "_SipCommonUtils.h"
#include "RvSipPartyHeader.h"
#include "RvSipMsg.h"
#include "TransactionTimer.h"
#include "_SipPartyHeader.h"
#include "_SipViaHeader.h"
#include "_SipAddress.h"
#include "RvSipReferredByHeader.h"
#include "TransactionControl.h"
#include "RvSipTransport.h"
#include "_SipTransport.h"
#include "_SipCommonCore.h"

#include "_SipTransmitter.h"
#include "TransactionMsgReceived.h"
#include "rvansi.h"

/*-----------------------------------------------------------------------*/
/*                             STATIC FUNCTIONS                          */
/*-----------------------------------------------------------------------*/
#if(RV_NET_TYPE & RV_NET_IPV6)
static RvUint32 RVCALLCONV IdIpv6HashFunction(IN const RvUint8* ip6Addr);
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * SipTransactionCreate
 * ------------------------------------------------------------------------
 * General: Called when a new Uac transaction is created.
 * The transaction key values are received as parameters
 * and set to the transaction's inner parameters.
 * The transaction's state will be set to "Initial" state.
 * The default response-code will be set to 0.
 * The Transaction's callbacks will be set with a set of callbacks that
 * implement a default behavior.
 * The transaction will sign itself to the hash manager, in other words the
 * transaction will call the ManageTransaction function of the transactions
 * manager with a handle to itself.
 * Return Value: RV_OK - The out parameter pTransc now points
 *                            to a valid transaction, initialized by the
 *                              received key.
 *                 RV_ERROR_BADPARAM - At least one of the transaction key
 *                                     values received as a parameter is not
 *                                     legal.
 *                                     All key fields must have legal values.
 *                 RV_ERROR_NULLPTR - At least one of the pointers to the
 *                                 transaction key values is a NULL pointer,
 *                                 or the pointer to the transaction pointer is a
 *                                 NULL pointer.
 *               RV_ERROR_OUTOFRESOURCES - Couldn't allocate memory for the
 *                                     transaction object.
 *                                     Another posibility is that the transaction
 *                                     manager is full. In that case the
 *                                     transaction will not be created and this
 *                                     value is returned.
 *               RV_ERROR_UNKNOWN - A transaction with the given key already exists.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr    - The transaction manager to which this transaction will
 *                          belong.
 *            pKey        -  The transaction's KEY.
 *          pOwner        -  The transaction's owner.
 *          eOwnerType    -  The transaction's owner type.
 *          tripleLock    -  The transaction's lock, processing lock, API lock
 *                            and API counter.
 * Output:  phTransaction - The new trasnaction created.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionCreate(
                                    IN  RvSipTranscMgrHandle    hTranscMgr,
                                    IN  SipTransactionKey       *pKey,
                                    IN  void                    *pOwner,
                                    IN  SipTransactionOwner      eOwnerType,
                                    IN  SipTripleLock           *tripleLock,
                                    OUT RvSipTranscHandle       *phTransaction)
{
    Transaction *pTransc = NULL;
    RvStatus rv = RV_OK;

    if ((NULL == phTransaction)        || (NULL == pKey) ||
        (NULL == pKey->hFromHeader)    || (NULL == pKey->hToHeader) ||
        (NULL == pKey->strCallId.hPool)|| (NULL_PAGE == pKey->strCallId.hPage) ||
        (UNDEFINED == pKey->strCallId.offset) )
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransactionCreate(hTranscMgr,pKey,pOwner,eOwnerType,tripleLock,
                           RV_TRUE,phTransaction);
    if (RV_OK != rv)
    {
        return rv;
    }
    pTransc = (Transaction *)*phTransaction;
    pTransc->bIsProxy = RV_FALSE; /* not a proxy transaction, created by the calleg layer*/

    return RV_OK;

}

/***************************************************************************
 * SipTransactionTerminate
 * ------------------------------------------------------------------------
 *  General: Causes immediate shut-down of the transaction:
 *  Changes the transaction's state to "terminated" state.
 *  Calls Destruct on the transaction being
 *  terminated. The termintated state will not be notified to the owner.
 *  This function is used only by the call-leg.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTransc - The transaction to terminate.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionTerminate(
                                            IN RvSipTranscHandle hTransc)
{
    Transaction        *pTransc;
    RvStatus           rv;
    pTransc = (Transaction *)hTransc;
    /*release transaction timers*/
       if(pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
    {
        return RV_OK;
    }

    TransactionTimerReleaseAllTimers(pTransc);
    RvLogInfo((pTransc->pMgr)->pLogSrc,((pTransc->pMgr)->pLogSrc,
              "SipTransactionTerminate - Transaction 0x%p: Terminated", pTransc));
    pTransc->eState = RVSIP_TRANSC_STATE_TERMINATED;
    rv = TransactionDestruct((Transaction*)hTransc);
    return rv;
}

/***************************************************************************
 * SipTransactionRequest
 * ------------------------------------------------------------------------
 * General: Creates and sends a request message according to the given method,
 *          with the given address as a Request-Uri.
 * Return Value: RV_OK - The message was created and sent successfully.
 *                 RV_ERROR_NULLPTR - The Request-Uri pointer is a NULL pointer,
 *                                 or the transaction pointer is a NULL pointer.
 *                 RV_ERROR_ILLEGAL_ACTION - The Request was called in a state where
 *                                    request can not be executed.
 *                 RV_ERROR_UNKNOWN - An error occured while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -   The transaction this request refers to.
 *            eRequestMethod - The enumeration value of the request method.
 *            pRequestUri -    The Request-Uri to be used in the request
 *                           message.
 *                           The message's Request-Uri will be the address
 *                           given here, and the message will be sent
 *                           accordingly.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionRequest(
                           IN  RvSipTranscHandle           hTransc,
                           IN  SipTransactionMethod        eRequestMethod,
                           IN  RvSipAddressHandle          hRequestUri)
{
    Transaction         *pTransc;
    RvStatus           rv;

    pTransc = (Transaction *)hTransc;

    if ((NULL == pTransc) ||
        (NULL == hRequestUri))
    {
        return RV_ERROR_NULLPTR;
    }

    /*request can be sent only in initial state*/
    if (RVSIP_TRANSC_STATE_IDLE != pTransc->eState)
    {
        /* A request is allowed only in an Initial state */
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "SipTransactionRequest - Transaction 0x%p: Error - Called Request in a state that is not defined for it",
                  pTransc));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    pTransc->eMethod = eRequestMethod;

    switch (eRequestMethod)
    {
    case SIP_TRANSACTION_METHOD_INVITE:
    case SIP_TRANSACTION_METHOD_BYE:
    case SIP_TRANSACTION_METHOD_REGISTER:
#ifdef RV_SIP_SUBS_ON
    case SIP_TRANSACTION_METHOD_REFER:
    case SIP_TRANSACTION_METHOD_NOTIFY:
    case SIP_TRANSACTION_METHOD_SUBSCRIBE:
#endif /* RV_SIP_SUBS_ON */
        break;
#ifndef RV_SIP_PRIMITIVES
    case SIP_TRANSACTION_METHOD_PRACK:
        break;
#endif /* #ifndef RV_SIP_PRIMITIVES */
    default:
        /* Request method is not supported*/
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "SipTransactionRequest - Transaction 0x%p: Request was called with an invalid method (enum %d)",
                  pTransc,eRequestMethod));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    rv = TransactionControlSendRequest(pTransc, hRequestUri,RV_TRUE);
    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "SipTransactionRequest - Transaction 0x%p: Failed",
                  pTransc));
        return rv;
    }

    /* Update the boolean indicating weather the To header includes tag at this point*/
    if (UNDEFINED != SipPartyHeaderGetTag(pTransc->hToHeader))
    {
        pTransc->bTagInTo = RV_TRUE;
    }
    else
    {
        pTransc->bTagInTo = RV_FALSE;
    }
    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "SipTransactionRequest - Transaction 0x%p: Request was sent successfully",
                  pTransc));

    return RV_OK;
}

/***************************************************************************
 * SipTransactionAck
 * ------------------------------------------------------------------------
 * General:  Creates and sends an acknowledgment message with the given
 *           address as the Request-Uri.
 * Return Value: RV_OK - The message was created and sent successfully.
 *                 RV_ERROR_NULLPTR - The Request-Uri pointer is a NULL pointer,
 *                                 or the transaction pointer is a NULL pointer.
 *                 RV_ERROR_ILLEGAL_ACTION - The Ack was called in a state where
 *                                    acknowledgment can not be executed.
 *                 RV_ERROR_UNKNOWN - An error occured while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTransc - The transaction this request refers to.
 *            pRequestUri -  The Request-Uri to be used in the ACK message. The
 *                           message's Request-Uri will be the address given
 *                         here, and the message will be sent accordingly.
 *                         If the request URI is NULL then the request URI of the
 *                         transaction is used as the request URI. The request URI
 *                         can be NULL when the Ack is to a non 2xx response, and then
 *                         the request URI must be the same as the invite request URI
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionAck(
                                       IN  RvSipTranscHandle  hTransc,
                                       IN  RvSipAddressHandle hRequestUri)
{
    Transaction        *pTransc;
    RvStatus           rv;
    RvLogSource*  logId;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    logId = (pTransc->pMgr)->pLogSrc;

    if ( (RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD != pTransc->eState)                             &&
          (RVSIP_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE != pTransc->eState &&
           503 != pTransc->responseCode) )
    {
        /* Ack is allowed only in Uac INVITE final response received state */
        RvLogError(logId ,(logId ,
                  "SipTransactionAck - Transaction 0x%p: Error - Called Ack in a state that is not defined for it",
                  pTransc));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    RvLogInfo(logId ,(logId ,
              "SipTransactionAck - Transaction 0x%p: about to send ACK ...",
              pTransc));

    if(hRequestUri == NULL)
    {
        if(pTransc->hRequestUri != NULL)
        {
            hRequestUri = pTransc->hRequestUri;
        }
        else
        {
            RvLogError(logId ,(logId ,
                      "SipTransactionAck - Transaction 0x%p: Request URI is NULL",
                      pTransc));
            return RV_ERROR_INVALID_HANDLE;
        }
    }
    rv = TransactionControlSendAck(pTransc,hRequestUri);
    if(rv != RV_OK)
    {
        RvLogError(logId ,(logId ,
                  "SipTransactionAck - Transaction 0x%p: Failed to send ACK message",
                  pTransc));
        return rv;
    }
    RvLogInfo(logId ,(logId ,
              "SipTransactionAck - Transaction 0x%p: ACK request was sent",
              pTransc));

    return RV_OK;
}

#ifndef RV_SIP_PRIMITIVES


/***************************************************************************
 * SipTransactionSendPrackResponse
 * ------------------------------------------------------------------------
 * General:  Creates and sends a response to a PRACK message.
 * Return Value: RV_OK - The message was created and sent successfully.
 *                 RV_ERROR_NULLPTR - The Request-Uri pointer is a NULL pointer,
 *                                 or the transaction pointer is a NULL pointer.
 *                 RV_ERROR_ILLEGAL_ACTION - The Ack was called in a state where
 *                                    acknowledgment can not be executed.
 *                 RV_ERROR_UNKNOWN - An error occured while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTransc - The INVITE transaction handle.
 *            responseCode - The response code of the response.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionSendPrackResponse(
                                       IN  RvSipTranscHandle  hTransc,
                                       IN  RvUint16          responseCode)
{
    Transaction        *pTransc;
    RvStatus           rv;
    RvLogSource*  logId;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    logId = (pTransc->pMgr)->pLogSrc;

    RvLogInfo(logId ,(logId ,
              "SipTransactionSendPrackResponse - Transaction 0x%p: about to send %d response on PRACK ...",
              pTransc, responseCode));

    rv = TransactionControlSendResponse(pTransc,responseCode,NULL,
        RVSIP_TRANSC_REASON_TRANSACTION_COMMAND,
        RV_FALSE);
    if(rv != RV_OK)
    {
        RvLogError(logId ,(logId ,
                  "SipTransactionSendPrackResponse - Transaction 0x%p: Failed to send response message",
                  pTransc));
        return rv;
    }
    RvLogInfo(logId ,(logId ,
              "SipTransactionSendPrackResponse - Transaction 0x%p: %d response was sent",
              pTransc, responseCode));

    return RV_OK;
}

/***************************************************************************
 * SipTransactionGetPrackParent
 * ------------------------------------------------------------------------
 * General:  Gets the handle to the PRACK parent transaction.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hPrackTransc - The PRACK transaction handle.
 * Output:  hTransc      - Handle to the PRACK parent transaction.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGetPrackParent(
                                       IN  RvSipTranscHandle   hPrackTransc,
                                       OUT RvSipTranscHandle  *hTransc)
{
    Transaction        *pPrackTransc;

    if(hPrackTransc == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pPrackTransc = (Transaction *)hPrackTransc;

    *hTransc = pPrackTransc->hPrackParent;

    return RV_OK;
}

#endif /* #ifndef RV_SIP_PRIMITIVES   */

/***************************************************************************
 * SipTransactionGetCallId
 * ------------------------------------------------------------------------
 * General: Returns the transaction's Call-Id value. This value is indicated
 *          by the offset within a page of a memory pool, all received
 *          in the returned rpool pointer.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The handle to the transaction is invalid.
 *               RV_ERROR_NULLPTR - The pointer to the rpool pointer is invalid.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction from which to get Call-Id value.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGetCallId(
                                            IN  RvSipTranscHandle hTransc,
                                            OUT RPOOL_Ptr      *pstrCallId)
{
    Transaction        *pTransc;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pstrCallId)
    {
        return RV_ERROR_NULLPTR;
    }

    pstrCallId->hPage = pTransc->strCallId.hPage;
    pstrCallId->hPool = pTransc->strCallId.hPool;
    pstrCallId->offset = pTransc->strCallId.offset;
    return RV_OK;
}

/***************************************************************************
 * SipTransactionGetMethod
 * ------------------------------------------------------------------------
 * General: Returns the transaction's method type.If the transaction method
 *               is kept as a string SIP_TRANSACTION_METHOD_OTHER will be
 *               returned. In this case use RvSipTransactionGetMethodStr to
 *               get the method in a string format.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to get method.
 * Output:  peMethod - The transaction's method type.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGetMethod(
                                          IN RvSipTranscHandle        hTransc,
                                          OUT SipTransactionMethod *peMethod)
{
    Transaction           *pTransc;

    pTransc = (Transaction *)hTransc;
    *peMethod = SIP_TRANSACTION_METHOD_UNDEFINED;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    *peMethod = pTransc->eMethod;
    return RV_OK;
}

/***************************************************************************
 * SipTransactionSetMethodStr
 * ------------------------------------------------------------------------
 * General: Copy the method sting on to the transacion page.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc - The transaction handle.
 *            strMethod - The transaction string method.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionSetMethodStr(
                                    IN RvSipTranscHandle       hTransc,
                                    IN RvSipMethodType         eMethod,
                                    IN  RvChar               *strMethod)
{
    RvStatus rv = RV_OK;
    if(strMethod == NULL)
    {
        TransactionSetMethod(hTransc, eMethod);
    }
    else
    {
        TransactionSetMethod(hTransc, RVSIP_METHOD_OTHER);
        rv = TransactionSetMethodStr((Transaction*)hTransc,strMethod);
    }
    return rv;

}

/***************************************************************************
 * SipTransactionGetTransport
 * ------------------------------------------------------------------------
 * General: Returns the transport type the transaction is using.
 * Return Value: the transport type
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to get method.
 * Output:
 ***************************************************************************/
RvSipTransport RVCALLCONV SipTransactionGetTransport(
                                          IN RvSipTranscHandle        hTransc)
{
    Transaction           *pTransc;

    pTransc = (Transaction *)hTransc;
    return pTransc->eTranspType;
}


/***************************************************************************
 * SipTransactionWasAckSent
 * ------------------------------------------------------------------------
 * General: Ack was actually sent over the network.
 * Return Value: RV_TRUE - if ack was actually sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle
 ***************************************************************************/
RvBool RVCALLCONV SipTransactionWasAckSent(IN RvSipTranscHandle        hTransc)
{
    Transaction           *pTransc;
    pTransc = (Transaction *)hTransc;
    return pTransc->bAckSent;

}

/***************************************************************************
 * SipTransactionIsEqualBranchToMsgBranch
 * ------------------------------------------------------------------------
 * General: Compare the transaction branch to a branch with in a given message
 * Return Value: RV_TRUE if the brances are equal or if there was no branch in
 *               the transaction and the message. RV_FALSE otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to get method.
 *              hMsg - A message to take the branch from
 ***************************************************************************/
RvBool RVCALLCONV SipTransactionIsEqualBranchToMsgBranch(
                                          IN RvSipTranscHandle        hTransc,
                                          IN RvSipMsgHandle           hMsg)

{
    Transaction                *pTransc;
    RvSipHeaderListElemHandle   hListElem;
    RPOOL_Ptr                   msgViaBranch;
    RvSipViaHeaderHandle        hTopVia;
    RvInt32                    transcBranchOffset;

    pTransc = (Transaction *)hTransc;
    /*get branch offset from transaction top via*/
    transcBranchOffset = SipViaHeaderGetBranchParam(pTransc->hTopViaHeader);
    /*get top via branch from the message*/
    hTopVia = RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_VIA,
                                      RVSIP_FIRST_HEADER, &hListElem);
    if (NULL == hTopVia)
    {
        msgViaBranch.hPool  = NULL;
        msgViaBranch.hPage  = NULL_PAGE;
        msgViaBranch.offset = UNDEFINED;
    }
    msgViaBranch.hPool  = SipViaHeaderGetPool(hTopVia);
    msgViaBranch.hPage  = SipViaHeaderGetPage(hTopVia);
    msgViaBranch.offset = SipViaHeaderGetBranchParam(hTopVia);

    /*compare branches*/
    if(msgViaBranch.offset == UNDEFINED && transcBranchOffset == UNDEFINED)
    {
        return RV_TRUE;
    }
    if(msgViaBranch.offset != UNDEFINED && transcBranchOffset != UNDEFINED)
    {
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
        if(0 == SPIRENT_RPOOL_Stricmp(msgViaBranch.hPool,msgViaBranch.hPage,msgViaBranch.offset,
                                     pTransc->pMgr->hGeneralPool,pTransc->memoryPage,transcBranchOffset))
#else
        if(RV_TRUE == RPOOL_Stricmp(msgViaBranch.hPool,msgViaBranch.hPage,msgViaBranch.offset,
                                     pTransc->pMgr->hGeneralPool,pTransc->memoryPage,transcBranchOffset))
#endif
/* SPIRENT_END */
        {
            return RV_TRUE;
        }
    }
    return RV_FALSE;
}




/***************************************************************************
 * SipTransactionSetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Sets a handle to the outbound message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hTransc - handle to transaction to which the mesasge should be set
 *        hReceivedMsg - Handle to the received message.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionSetOutboundMsg(
                                              IN RvSipTranscHandle hTransc,
                                               IN RvSipMsgHandle    hOutboundMsg)
{
    Transaction           *pTransc;

    pTransc = (Transaction *)hTransc;
    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    pTransc = (Transaction *)hTransc;
    /*if the transaction already have an outbound message print
      warning and release the outbound message*/
    if(pTransc->hOutboundMsg != NULL)
    {
        RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "SipTransactionSetOutboundMsg - Transaction 0x%p: There is already an outbound message 0x%p. destucting...",
              pTransc,pTransc->hOutboundMsg));

        RvSipMsgDestruct(pTransc->hOutboundMsg);
        pTransc->hOutboundMsg = NULL;
    }

    pTransc->hOutboundMsg = hOutboundMsg;

    return RV_OK;
}

/***************************************************************************
 * SipTransactionGetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Return the transaction outbound message handle
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - handle to transaction to which the mesasge should be set
 * Output: phMsg - The outbound message of the transaction.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGetOutboundMsg(
                                                IN RvSipTranscHandle hTransc,
                                                OUT RvSipMsgHandle   *phMsg)
{
    Transaction           *pTransc;

    pTransc = (Transaction *)hTransc;
    *phMsg = NULL;
    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }
    pTransc = (Transaction *)hTransc;

    *phMsg = pTransc->hOutboundMsg;

    return RV_OK;
}

/***************************************************************************
 * SipTransactionDetachOwner
 * ------------------------------------------------------------------------
 * General: Called when the transaction's old owner is not it's owner any
 *          more. The transaction will begin acting as a stand alone transaction.
 *          note that in case of failure, you must terminate the transaction
 *          afterwards, since the transaction is no longer in the hash table.
 * Return Value: RV_ERROR_NULLPTR - The hTransc is a NULL pointer.
 *               RV_ERROR_OUTOFRESOURCES - There wasn't enough memory for the
 *                                   transaction's To, From and Call-Id headers.
 *                                   (at this point the transaction allocates its
 *                                   own place for these).
 *               RV_OK - The detaching was successfully finished.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to detach owner.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionDetachOwner(
                                             IN RvSipTranscHandle hTransc)
{
    Transaction        *pTransc;
    RvStatus           rv;


    if (NULL == hTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    pTransc = (Transaction *)hTransc;
       RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "SipTransactionDetachOwner - transc 0x%p, calling  TransactionNoOwner()",hTransc));


    rv = TransactionNoOwner(pTransc);
    if(rv != RV_OK)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "SipTransactionDetachOwner - Transaction 0x%p: Failed to detach owner",
                  pTransc));

    }
    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "SipTransactionDetachOwner - Transaction 0x%p: Detached successfully",
              pTransc));
    return rv;
}

/***************************************************************************
 * SipTransactionDetachFromOwnerKey
 * ------------------------------------------------------------------------
 * General: Called when the transaction's owner wishes to detach from the transaction
 *          key, i.e., to have its own independent key. After calling this function,
 *          changes made by the transaction on its key will not be reflected at the
 *          owner. For example, if the transaction will copy the To tag from a 
 *          received message, the owner will not be updated with this To tag.
 *          NOTE: If the function fails during the copying of the key, the old
 *                key (that is the owners key) will be restored in the transaction.
 * Return Value: RV_ERROR_NULLPTR - The hTransc is a NULL pointer.
 *               RV_ERROR_OUTOFRESOURCES - There wasn't enough memory for the
 *                                   transaction's To, From and Call-Id headers.
 *                                   (at this point the transaction allocates its
 *                                   own place for these).
 *               RV_OK - The detaching was successfully finished.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to detach owner key.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionDetachFromOwnerKey(
                                             IN RvSipTranscHandle hTransc)
{
    Transaction            *pTransc;
	RvSipPartyHeaderHandle  hOldTo;
	RvSipPartyHeaderHandle  hOldFrom;
	RPOOL_Ptr               oldCallId;
    RvStatus				rv;


    if (NULL == hTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    pTransc = (Transaction *)hTransc;
    
	RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "SipTransactionDetachFromOwnerKey - transc 0x%p, calling  TransactionCopyKeyToInternalPage()",hTransc));

	TransactionLockTranscMgr(pTransc);

	hOldFrom		 = pTransc->hFromHeader;
	hOldTo			 = pTransc->hToHeader;
	oldCallId.hPage  = pTransc->strCallId.hPage;
    oldCallId.hPool  = pTransc->strCallId.hPool;
    oldCallId.offset = pTransc->strCallId.offset;
        
	rv = TransactionCopyFromHeader(pTransc);
	if (RV_OK == rv)
	{
		rv = TransactionCopyToHeader(pTransc);
		if (RV_OK != rv)
		{
			rv = TransactionCopyCallId(pTransc);
		}
	}

    if (rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "SipTransactionDetachFromOwnerKey - Transaction 0x%p - Failed to copy key to transaction page. Will not detach from key",
                  pTransc));
		pTransc->hFromHeader      = hOldFrom;
		pTransc->hToHeader        = hOldTo;
		pTransc->strCallId.hPage  = oldCallId.hPage;
        pTransc->strCallId.hPool  = oldCallId.hPool;
		pTransc->strCallId.offset = oldCallId.offset;
    }
	else
	{
		RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "SipTransactionDetachFromOwnerKey - Transaction 0x%p: Detached from owner key successfully",
              pTransc));
	}

	TransactionUnlockTranscMgr(pTransc);

    return rv;
}

/***************************************************************************
 * SipTransactionSetAppInitiator
 * ------------------------------------------------------------------------
 * General: Set the application as the initiator of the transaction.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle.
 ***************************************************************************/
void RVCALLCONV SipTransactionSetAppInitiator(IN RvSipTranscHandle hTransc)
{

    Transaction        *pTransc;
    if (NULL == hTransc)
    {
        return;
    }

    pTransc = (Transaction *)hTransc;
     pTransc->bAppInitiate = RV_TRUE;
}

/***************************************************************************
 * SipTransactionSetCancelTranscPair
 * ------------------------------------------------------------------------
 * General: Sets the handle to the cancel pair transaction.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 *          hCancelPair -  The cancel pair transaction handle.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionSetCancelTranscPair(
                                        IN RvSipTranscHandle    hTransc,
                                        IN RvSipTranscHandle    hCancelPair)
{

    Transaction           *pTransc;

    pTransc = (Transaction*)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }


    pTransc->hCancelTranscPair = hCancelPair;
    return RV_OK;
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)

/***************************************************************************
 * SipTransactionGetStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV SipTransactionGetStateName (
                                      IN  RvSipTransactionState  eState)
{
    return TransactionGetStateName(eState);
}

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * SipTransactionGetMsgCompTypeName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given transaction compression message type
 * Return Value: The string with the message type.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eMsgCompType - The compression type as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV SipTransactionGetMsgCompTypeName (
                             IN  RvSipTransmitterMsgCompType eTranscMsgType)
{
    return TransactionGetMsgCompTypeName(eTranscMsgType);
}
#endif /* RV_SIGCOMP_ON */

#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */


/***************************************************************************
 * SipTransactionGetResponseCode
 * ------------------------------------------------------------------------
 * General: Get the transaction response code.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle.
 *          responseCode - The response code  of the transaction.
 ***************************************************************************/
void RVCALLCONV SipTransactionGetResponseCode(IN  RvSipTranscHandle hTransc,
                                              OUT RvUint16         *responseCode)
{

    Transaction        *pTransc;

    if (NULL == hTransc)
    {
        return;
    }

    pTransc = (Transaction *)hTransc;

     *responseCode = pTransc->responseCode;
}

/***************************************************************************
 * SipTransactionSetResponseCode
 * ------------------------------------------------------------------------
 * General: Set the transaction response code.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle.
 *          responseCode - The response code  of the transaction.
 ***************************************************************************/
void RVCALLCONV SipTransactionSetResponseCode(IN  RvSipTranscHandle hTransc,
                                              IN  RvUint16          responseCode)
{

    Transaction        *pTransc;

    if (NULL == hTransc)
    {
        return;
    }

    pTransc = (Transaction *)hTransc;

     pTransc->responseCode = responseCode;
}


/***************************************************************************
 * SipTransactionGetLastState
 * ------------------------------------------------------------------------
 * General: returns the lastState enumuration
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle.
 ***************************************************************************/
RvSipTransactionState RVCALLCONV SipTransactionGetLastState(
                                                    IN  RvSipTranscHandle hTransc)
{

    Transaction *pTransc = (Transaction *)hTransc;

     return pTransc->eLastState;
}




/***************************************************************************
 * SipTransactionSetOutboundProxy
 * ------------------------------------------------------------------------
 * General: Set the outbound address of the transaction. Any requests sent
 *          by this transaction will be sent according to the transaction
 *          outbound proxy address, unless the transaction is using a
 *          Record-Route path.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction
 *            outboundAddr  - The outbound proxy to set.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionSetOutboundAddress(
                            IN  RvSipTranscHandle           hTransc,
                            IN  SipTransportOutboundAddress *outboundAddr)
{
    Transaction        *pTransc;

    pTransc = (Transaction *)hTransc;

    return SipTransmitterSetOutboundAddsStructure(pTransc->hTrx,outboundAddr);
}


/***************************************************************************
 * SipTransactionGetContext
 * ------------------------------------------------------------------------
 * General: Get pointer to the context memory saved by the transaction
 *          for the call-leg.
 * Return Value: The context memory.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction.
 ***************************************************************************/
void *RVCALLCONV SipTransactionGetContext(
                                IN  RvSipTranscHandle           hTransc)
{
    Transaction        *pTransc;
    if (NULL == hTransc)
    {
        return NULL;
    }
    pTransc = (Transaction *)hTransc;

    return pTransc->pContext;
}

/***************************************************************************
 * SipTransactionAllocateOwnerMemory
 * ------------------------------------------------------------------------
 * General: Append a consecutive memory to the transaction memory to be filled
 *          with owner information. The transaction holds the information
 *          for its owner. The pointer to this memory will be saved by
 *          the transaction and may be retrieved using SipTransactionGetContext().
 *          If a context was already allocated for this owner its pointer will be
 *          returned.
 * Return Value: A pointer to the consecutive memory.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction.
 *          size           - Size of memory to append.
 ***************************************************************************/
void *RVCALLCONV SipTransactionAllocateOwnerMemory(
                                IN  RvSipTranscHandle      hTransc,
                                IN  RvInt32               size)
{
    Transaction        *pTransc;
    RvInt32            offset;

    if (NULL == hTransc)
    {
        return NULL;
    }
    pTransc = (Transaction *)hTransc;

    if(pTransc->pContext != NULL)
    {
        return pTransc->pContext;
    }
    pTransc->pContext = RPOOL_AlignAppend(pTransc->pMgr->hGeneralPool,
                      pTransc->memoryPage,
                      size, &offset);
    return pTransc->pContext;
}




/***************************************************************************
 * SipTransactionIsAppInitiator
 * ------------------------------------------------------------------------
 * General: Returns weather the initiator of the transaction is the
 *          application.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle.
 ***************************************************************************/
RvBool RVCALLCONV SipTransactionIsAppInitiator(IN RvSipTranscHandle hTransc)
{
    Transaction        *pTransc;
    if (NULL == hTransc)
    {
        return RV_FALSE;
    }
    pTransc = (Transaction *)hTransc;
    return pTransc->bAppInitiate;
}


#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
* SipTransactionSetSubsInfo
* ------------------------------------------------------------------------
* General: Set the subscription info in the transaction.
*          (Subscription info is the related object handle - notification or
*          subscription).
* ------------------------------------------------------------------------
* Arguments:
* Input:    hTransc - The transaction handle.
*          pSubsInfo - the info to set.
***************************************************************************/
RvStatus RVCALLCONV SipTransactionSetSubsInfo(IN RvSipTranscHandle hTransc,
                                              IN void             *pSubsInfo)
{
#ifdef RV_SIP_SUBS_ON
    Transaction* pTransc = (Transaction *)hTransc;

    if (NULL == hTransc)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pTransc->pSubsInfo = pSubsInfo;

    return RV_OK;
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hTransc);
    RV_UNUSED_ARG(pSubsInfo);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
* SipTransactionGetSubsInfo
* ------------------------------------------------------------------------
* General: Get the subscription info from the transaction.
*          (Subscription info is the related object handle - notification or
*          subscription).
* Return Value: pointer to the subsInfo.
* ------------------------------------------------------------------------
* Arguments:
* Input:    hTransc - The transaction handle.
*          pSubsInfo - the info to set.
***************************************************************************/
void* RVCALLCONV SipTransactionGetSubsInfo(IN RvSipTranscHandle hTransc)
{
#ifdef RV_SIP_SUBS_ON
    Transaction* pTransc = (Transaction *)hTransc;

    if (NULL == hTransc)
    {
        return NULL;
    }
    return pTransc->pSubsInfo;
#else /* #ifdef RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(hTransc);

    return NULL;
#endif /* #ifdef RV_SIP_SUBS_ON */
}


/***************************************************************************
 * SipTransactionAllocateSessionTimerMemory
 * ------------------------------------------------------------------------
 * General: Append a consecutive memory to the transaction memory to be filled
 *          with a call-leg information. The transaction holds the session
 *          timer information for the call-leg.
 * Return Value: A pointer to the session timer pointer.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction.
 *          size - Size of memory to append.
 ***************************************************************************/
void *RVCALLCONV SipTransactionAllocateSessionTimerMemory(
                                IN  RvSipTranscHandle      hTransc,
                                IN  RvInt32               size)
{
    Transaction        *pTransc;
    RvInt32            offset;

    if (NULL == hTransc)
    {
        return NULL;
    }
    pTransc = (Transaction *)hTransc;
    pTransc->pSessionTimer = RPOOL_AlignAppend(pTransc->pMgr->hGeneralPool,
                      pTransc->memoryPage,
                      size, &offset);
    return pTransc->pSessionTimer;
}

/***************************************************************************
 * SipTransactionGetSessionTimerMemory
 * ------------------------------------------------------------------------
 * General: Get pointer to the context memory saved by the transaction
 *          for the call-leg.
 * Return Value: The context memory.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction.
 ***************************************************************************/
void *RVCALLCONV SipTransactionGetSessionTimerMemory(
                                IN  RvSipTranscHandle           hTransc)
{
    Transaction        *pTransc;
    if (NULL == hTransc)
    {
        return NULL;
    }
    pTransc = (Transaction *)hTransc;

    return pTransc->pSessionTimer;
}
#endif /*#ifndef RV_SIP_PRIMITIVES */

/***************************************************************************
 * SipTransactionGetMsgMethodFromTranscMethod
 * ------------------------------------------------------------------------
 * General: Returns the aquivalent message method from the transaction method.
 * Return Value: the aquivalent message method from the transaction method.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction handle
 ***************************************************************************/
RvSipMethodType SipTransactionGetMsgMethodFromTranscMethod(
                            IN  RvSipTranscHandle    hTransc)

{
    Transaction        *pTransc;
    if (NULL == hTransc)
    {
        return RVSIP_METHOD_UNDEFINED;
    }

    pTransc = (Transaction *)hTransc;
    return TransactionGetMsgMethodFromTranscMethod(pTransc);
}

/***************************************************************************
 * SipTransactionGetNewAddressHandle
 * ------------------------------------------------------------------------
 * General: Allocates a new Address on the transaction page and returns its handle.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction.
 *            eAddrType - Type of address the application wishes to create.
 * Output:     phAddr    - Handle to the newly created address header.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGetNewAddressHandle (
                                      IN   RvSipTranscHandle      hTransc,
                                      IN   RvSipAddressType       eAddrType,
                                      OUT  RvSipAddressHandle    *phAddr)
{
    RvStatus      rv;
    Transaction    *pTransc = (Transaction*)hTransc;

    if(pTransc== NULL || phAddr == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phAddr = NULL;

    rv = RvSipAddrConstruct(pTransc->pMgr->hMsgMgr, pTransc->pMgr->hGeneralPool,
                            pTransc->memoryPage, eAddrType, phAddr);
    if (rv != RV_OK || *phAddr == NULL)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "SipTransactionGetNewAddressHandle - transc 0x%p - Failed to create new address (rv=%d)",
              hTransc,rv));
        return rv;
    }
    return RV_OK;

}

/***************************************************************************
 * SipTransactionGetNewPartyHeaderHandle
 * ------------------------------------------------------------------------
 * General: Allocates a new party header on the transaction page and
 *          returns its handle.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction.
 * Output:     phHeader    - Handle to the newly created header.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGetNewPartyHeaderHandle (
                                      IN   RvSipTranscHandle          hTransc,
                                      OUT  RvSipPartyHeaderHandle    *phHeader)
{
    RvStatus      rv;
    Transaction    *pTransc = (Transaction*)hTransc;

    if(pTransc== NULL || phHeader == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phHeader = NULL;
    
    rv = RvSipPartyHeaderConstruct(pTransc->pMgr->hMsgMgr, pTransc->pMgr->hGeneralPool,
                            pTransc->memoryPage, phHeader);
    if (rv != RV_OK || *phHeader == NULL)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "SipTransactionGetNewPartyHeaderHandle - transc 0x%p - Failed to create new party header (rv=%d)",
              hTransc,rv));
        return rv;
    }
    return RV_OK;

}
/***************************************************************************
 * SipTransactionSetReqURI
 * ------------------------------------------------------------------------
 * General: Sets the given request uri address in the transaction object.
 *          Before setting check if the given address was allocated on the
 *          transaction page, and if so do not copy it
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction.
 *            eAddrType - Type of address.
 *          phAddr    - Handle to the req-uri address.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionSetReqURI (
                                      IN  RvSipTranscHandle     hTransc,
                                      IN  RvSipAddressType      eAddrType,
                                      IN  RvSipAddressHandle    hAddr)
{
    RvStatus rv;
    Transaction    *pTransc = (Transaction*)hTransc;

    /*first check if the given header was constructed on the transaction page
      and if so attach it*/

    if( SipAddrGetPool(hAddr) == pTransc->pMgr->hGeneralPool &&
        SipAddrGetPage(hAddr) == pTransc->memoryPage)
    {
        pTransc->hRequestUri = hAddr;
        return RV_OK;
    }

    /*the given to header is not on the transaction page - construct and copy*/
    if(pTransc->hRequestUri == NULL)
    {
        rv = RvSipAddrConstruct(pTransc->pMgr->hMsgMgr,
                               pTransc->pMgr->hGeneralPool,
                               pTransc->memoryPage, eAddrType,
                               &(pTransc->hRequestUri));
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                   "SipTransactionSetReqURI - transc 0x%p - Failed to construct request-uri address (rv=%d)",
                   pTransc,rv));
            return RV_ERROR_OUTOFRESOURCES;
        }
    }

    /*copy information from the given hFrom the the call-leg from header*/
    rv = RvSipAddrCopy(pTransc->hRequestUri, hAddr);
    if(rv != RV_OK)
    {
         RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                   "SipTransactionSetReqURI - transc 0x%p - Failed to copy request-uri address (rv=%d)",
                   pTransc,rv));
         return RV_ERROR_OUTOFRESOURCES;

    }
    return RV_OK;
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * SipTransactionConstructAndSetReferredByHeader
 * ------------------------------------------------------------------------
 * General: Allocate and Sets the given referred-by header in the transaction object.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionConstructAndSetReferredByHeader (
                                      IN  RvSipTranscHandle              hTransc,
                                      IN  RvSipReferredByHeaderHandle    hReferredBy)
{
#ifdef RV_SIP_SUBS_ON
    return TransactionConstructAndSetReferredByHeader((Transaction*)hTransc, hReferredBy);
#else /* #ifdef RV_SIP_SUBS_ON */
    RV_UNUSED_ARG(hTransc);
    RV_UNUSED_ARG(hReferredBy);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
}
#endif /*#ifndef RV_SIP_PRIMITIVES */


#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * SipTransactionIs1xxLegalRel
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
void RVCALLCONV SipTransactionIs1xxLegalRel(IN  RvSipMsgHandle hMsg,
                                            OUT RvUint32      *pRseqStep,
                                            OUT RvBool        *pbLegal1xx)
{
    TransactionMsgIs1xxLegalRel(hMsg, pRseqStep, pbLegal1xx);
}
#endif /*#ifndef RV_SIP_PRIMITIVES*/


/***************************************************************************
 * SipTransactionGetStringMethodFromEnum
 * ------------------------------------------------------------------------
 * General: return a string with the method of the given enumeration.
 * Return Value: The method as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eMethod - the method as enum.
 ***************************************************************************/
RvChar*  RVCALLCONV SipTransactionGetStringMethodFromEnum(
                                            IN SipTransactionMethod eMethod)
{
    return TransactionGetStringMethodFromEnum(eMethod);
}

/***************************************************************************
 * SipTransactionGenerateIDStr
 * ------------------------------------------------------------------------
 * General: Generate ID to be used for callID or tag.
 * Return Value: RV_OK - Call-Id was generated successfully
 *               o/w -      - failed to get local address
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransport - transportMgr handle.
 *          pseed      - pointer to random seed
 *          hObj       - pointer to the object that the ID is generated for.
 *          logId      - the logger to use for errors
 * Output:  strID      - a generated string that contains the random ID.
 *                       the string must be of size SIP_COMMON_ID_SIZE
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGenerateIDStr(
                             IN RvSipTransportMgrHandle     hTransport,
                             IN RvRandomGenerator*          pseed,
                             IN void*                       hObj,
                             IN RvLogSource*                logId,
                             OUT RvChar*                    strID)
{
    SipTransportAddress addr;
    RvStatus            rv = RV_OK;
    RvUint32            curTime = SipCommonCoreRvTimestampGet(IN_SEC);
    RvInt32             r;

    RvRandomGeneratorGetInRange(pseed,RV_INT32_MAX,(RvRandom*)&r);
    memset(&addr,0,sizeof(SipTransportAddress));
    rv = SipTransportLocalAddressGetFirstAvailAddress(hTransport, &addr);
    if(rv != RV_OK)
    {
		RvInt32             r1;
		RvInt32             r2;

		RvRandomGeneratorGetInRange(pseed,RV_INT32_MAX,(RvRandom*)&r1);
		RvRandomGeneratorGetInRange(pseed,RV_INT32_MAX,(RvRandom*)&r2);
        RvSprintf(strID, "%lx-%x-%x-%s-%x-%x-%x",
            (RvUlong)RvPtrToUlong(hObj),
            r1,
            r2,
            SIP_STACK_VERSION_FOR_CALLEG_ID,
            curTime,
            r,
            curTime);

        RvLogDebug(logId,(logId,
             "SipTransactionGenerateIDStr - No local address found, only randoms are used."));
        return RV_OK;
    }
    if (RV_ADDRESS_TYPE_IPV4 == RvAddressGetType(&addr.addr))
    {
        RvSprintf(strID, "%lx-%x-%x-%s-%x-%x-%x",
            (RvUlong)RvPtrToUlong(hObj),
            RvAddressIpv4GetIp(RvAddressGetIpv4(&addr.addr)),
            RvAddressGetIpPort(&addr.addr),
            SIP_STACK_VERSION_FOR_CALLEG_ID,
            curTime,
            r,
            curTime);
    }
#if(RV_NET_TYPE & RV_NET_IPV6)
    else if (RV_ADDRESS_TYPE_IPV6 == RvAddressGetType(&addr.addr))
    {
        RvSprintf(strID, "%lx-%x-%x-%s-%x-%x-%x",
            (RvUlong)RvPtrToUlong(hObj),
            IdIpv6HashFunction(RvAddressIpv6GetIp(RvAddressGetIpv6(&addr.addr))),
            RvAddressGetIpPort(&addr.addr),
            SIP_STACK_VERSION_FOR_CALLEG_ID,
            curTime,
            r,
            curTime);
    }
#endif /* (RV_NET_TYPE & RV_NET_IPV6) */
	
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(logId);
#endif

    return rv;
}

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/***************************************************************************
 * SipTransactionDNSContinue
 * ------------------------------------------------------------------------
 * General: Creates new transaction and copies there major transaction
 *          parameters and terminates the original transaction.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hOrigTransaction   - The source transaction handle.
 *          owner              - pointer to the new transaction owner info.
 *          bRemoveToTag       - Remove the to tag from transaction or not.
 *                             (This parameter should be TRUE for a first
 *                              INVITE transaction, or for a non-callLeg
 *                              transaction.
 * Output:  hNewTransaction    - Pointer to the new transaction handler.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionDNSContinue(
                                               IN  RvSipTranscHandle        hOrigTrans,
                                               IN  RvSipTranscOwnerHandle   hOwner,
                                               IN  RvBool                  bRemoveToTag,
                                               OUT RvSipTranscHandle        *hCloneTrans)
{
    RvStatus       retCode     = RV_OK;
    Transaction*    pOrigTrans = (Transaction*)hOrigTrans;

    if(pOrigTrans == NULL)
    {
        return (RV_ERROR_UNKNOWN);
    }

    RvLogDebug(pOrigTrans->pMgr->pLogSrc,(pOrigTrans->pMgr->pLogSrc,
              "SipTransactionDNSContinue - transaction 0x%p is cloned", pOrigTrans));

    if(pOrigTrans->eState != RVSIP_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE)
    {
        RvLogError(pOrigTrans->pMgr->pLogSrc,(pOrigTrans->pMgr->pLogSrc,
            "SipTransactionDNSContinue - transaction 0x%p Continue dns is only allowed in state MSG_SEND_FAILURE (rv=%d)",
            pOrigTrans, RV_ERROR_ILLEGAL_ACTION));
        return (RV_ERROR_ILLEGAL_ACTION);
    }

    if (!SipTransmitterCanContinue(pOrigTrans->hTrx))
    {
        RvLogError(pOrigTrans->pMgr->pLogSrc,(pOrigTrans->pMgr->pLogSrc,
            "SipTransactionDNSContinue - transaction 0x%p Continue dns failed - transmitter has no additional addresses",
            pOrigTrans));
        return RV_ERROR_NOT_FOUND;
    }

    retCode = TransactionClone(pOrigTrans,hOwner,bRemoveToTag, hCloneTrans);

    if(retCode != RV_OK)
    {
        RvLogError(pOrigTrans->pMgr->pLogSrc,(pOrigTrans->pMgr->pLogSrc,
                  "SipTransactionDNSContinue - Failed to clone transaction 0x%p (rv=%d)",
                  pOrigTrans, retCode));
        return retCode;

    }
    /* in case of invite+503 we need to ACK, general transaction do not need to send ack,
       and can be terminated imediatly. we determine general by the state */
    if ((pOrigTrans->responseCode != 503)   || /* response code not 503 */
        (RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT == pOrigTrans->eLastState || /* 503 on general, no need to ACK */
        RVSIP_TRANSC_STATE_CLIENT_GEN_PROCEEDING == pOrigTrans->eLastState) )
    {
        TransactionTerminate(RVSIP_TRANSC_REASON_CONTINUE_DNS,pOrigTrans);
    }
    return RV_OK;
}
#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * SipCallLegSetHeadersListToSetInInitialRequest
 * ------------------------------------------------------------------------
 * General: Sets the HeadersListToSetInInitialRequest to the call-leg,
 *          the list headers will be set to the outgoing INVITE request.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - Referred-By header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - Handle to the call-leg.
 *           hHeadersList - Handle to the headers list.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscSetHeadersListToSetInInitialRequest (
                            IN  RvSipTranscHandle           hTransc,
                            IN  RvSipCommonListHandle       hHeadersList)
{
    ((Transaction*)hTransc)->hHeadersListToSetInInitialRequest = hHeadersList;
    return RV_OK;
}

/***************************************************************************
 * SipTransactionSetTimers
 * ------------------------------------------------------------------------
 * General: Sets timeout values for the transaction timers.
 *          If some of the fields in pTimers are not set (UNDEFINED), this
 *          function will calculate it, or take the values from configuration.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hTransc - Handle to the transaction object.
 *           pTimers - Pointer to the struct contains all the timeout values.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionSetTimers(
                            IN  RvSipTranscHandle    hTransc,
                            IN  RvSipTimers*         pNewTimers)
{
    Transaction        *pTransc;
    pTransc = (Transaction *)hTransc;

    if (NULL == hTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    return TransactionSetTimers(pTransc, pNewTimers);
}


/***************************************************************************
 * SipTransactionGetTrxControl
 * ------------------------------------------------------------------------
 * General: Gives the transmitter, and delete it from transaction object.
 *          Note that the trx, still has the transaction as an owner.
 *          Whoever took control on the transmitter must change it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc    - pointer to Transaction
 *            hAppTrx     - new application handle to the transmitter.
 *            pEvHanders  - new event handlers structure for this transmitter.
 *            sizeofEvHandlers - size of the event handler structure.
 ***************************************************************************/
RvStatus SipTransactionGetTrxControl(
                  IN  RvSipTranscHandle           hTransc,
                  IN  RvSipAppTransmitterHandle   hAppTrx,
                  IN  RvSipTransmitterEvHandlers* pEvHandlers,
                  IN  RvInt32                     sizeofEvHandlers,
                  OUT RvSipTransmitterHandle     *phTrx)
{
    Transaction  *pTransc = (Transaction*)hTransc;
    RvStatus rv;
    if(phTrx == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = SipTransmitterChangeOwner(pTransc->hTrx, hAppTrx, pEvHandlers, sizeofEvHandlers);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc, (pTransc->pMgr->pLogSrc, 
            "SipTransactionGetTrxControl: transc 0x%p - failed for trx 0x%p",
            pTransc, pTransc->hTrx));
        return rv;
    }

    *phTrx = pTransc->hTrx;

    pTransc->hTrx = NULL;
    
    /* The transmitter uses the same page as the transaction.
       However, the page 'knows' that it has 2 owners (transaction and transmitter),
       so it will be destructed only once */
    return rv;
}

/***************************************************************************
 * SipTransactionGetClientResponseConnBackUp
 * ------------------------------------------------------------------------
 * General: Returns hClientResponseConnBackUp. call-leg invite object will hold
 *          this connection, after transaction termination, because 2xx
 *          retransmissions might be sent on this connection.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction.
 * Output:    phConn - Handle to the hClientResponseConnBackUp connection
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGetClientResponseConnBackUp(
                            IN  RvSipTranscHandle               hTransc,
                           OUT  RvSipTransportConnectionHandle *phConn)
{
    Transaction   *pTransc   = (Transaction*)hTransc;
    RvStatus       rv        = RV_OK;

    if (NULL == pTransc || phConn==NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phConn = pTransc->hClientResponseConnBackUp;
    
    return rv;

}

/***************************************************************************
 * SipTransactionFillBasicRequestMessage
 * ------------------------------------------------------------------------
 * General: Fill a request message with basic parameters.
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *                 RV_OutOfResource - Couldn't allocate memory for the new
 *                                    message, and the associated headers.
 *                 RV_OK - The Request message was successfully created.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hRequestUri - The request Uri
 *         hFrom       - The From header
 *         hTo         - The To header.
 *         pStrCallId  - Pointer to the call-Id rpool string
 *         eMethod     - The request method.
 *         strMethod   - An other method string.
 *         pCseq       - Pointer to the cseq value.
 *         bAddMaxForwards - Add Max-Forward header to the message or not.
 * Output: phMsg      - The filled message.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionFillBasicRequestMessage(
                                        IN  RvSipAddressHandle hRequestUri,
                                        IN  RvSipPartyHeaderHandle hFrom,
                                        IN  RvSipPartyHeaderHandle hTo,
                                        IN  RPOOL_Ptr         *pStrCallId,
                                        IN  RvSipMethodType    eMethod,
                                        IN  RvChar            *strMethod,
                                        IN  SipCSeq            *pCseq,
                                        IN  RvBool             bAddMaxForwards,
                                        INOUT RvSipMsgHandle   hMsg)
{
    return TransactionFillBasicRequestMessage(hRequestUri, hFrom, hTo, pStrCallId,
                                        eMethod,strMethod,pCseq,bAddMaxForwards,hMsg);
}
#if (RV_NET_TYPE & RV_NET_SCTP)
/***************************************************************************
 * SipTransactionSetSctpMsgSendingParams
 * ------------------------------------------------------------------------
 * General: sets SCTP message sending parameters, such as stream number,
 *          into the Transaction object.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc - Handle to the Transaction object.
 *          pParams - Parameters to be get.
 * Output:  none.
 ***************************************************************************/
void RVCALLCONV SipTransactionSetSctpMsgSendingParams(
                     IN  RvSipTranscHandle                  hTransc,
                     IN  RvSipTransportSctpMsgSendingParams *pParams)
{
    Transaction *pTransc = (Transaction*)hTransc;
    /* The parameters are stored in the underlying transmitter */
    if (NULL != pTransc->hTrx)
    {
        SipTransmitterSetSctpMsgSendingParams(pTransc->hTrx,pParams);
    }
}

/***************************************************************************
 * SipTransactionGetSctpMsgSendingParams
 * ------------------------------------------------------------------------
 * General: gets SCTP message sending parameters, such as stream number,
 *          set for the Transaction object.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc - Handle to the Transaction object.
 * Output:  pParams - Parameters to be get.
 ***************************************************************************/
void RVCALLCONV SipTransactionGetSctpMsgSendingParams(
                     IN  RvSipTranscHandle                  hTransc,
                     OUT RvSipTransportSctpMsgSendingParams *pParams)
{
    Transaction *pTransc = (Transaction*)hTransc;
    /* The parameters are stored in the underlying transmitter */
    if (NULL != pTransc->hTrx)
    {
        SipTransmitterGetSctpMsgSendingParams(pTransc->hTrx,pParams);
    }
}
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/


#endif /*#ifndef RV_SIP_PRIMITIVES*/

/***************************************************************************
 * SipTransactionAuthProceed
 * ------------------------------------------------------------------------
 * General: The function order the stack to proceed authentication procedure.
 *          Actions options are:
 *          RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD
 *          - Check the given authorization header, with the given password
 *
 *          RVSIP_TRANSC_AUTH_ACTION_SUCCESS
 *          - user had checked the authorization header by himself, and it is
 *            correct (will cause AuthCompletedEv to be called, with status
 *            success)
 *
 *          RVSIP_TRANSC_AUTH_ACTION_FAILURE
 *          - user wants to stop the loop on authorization headers.
 *            (will cause AuthCompletedEv to be called, with status failure)
 *
 *          RVSIP_TRANSC_AUTH_ACTION_SKIP
 *          - order to skip the given header, and continue the loop with next
 *            header (if exists).
 *            (will cause AuthCredentialsFoundEv to be called, or
 *            AuthCompletedEv(failure) if there are no more authorization
 *            headers)
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc        - The transaction handle.
 *          action         - Which action to take (see above)
 *          hAuthorization - Handle of the header to be authenticated.
 *                           Can be NULL.
 *                           Should be provided if action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD.
 *          password       - The password for the realm+userName in the header.
 *                           Can be NULL.
 *                           Should be provided if action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD.
 *          hObject        - handle to the object to be authenticated.
 *                           Can be Subscription, CallLeg, RegClient or
 *                           Transaction itself, if it is outstanding.
 *          eObjectType    - type of the object to be authenticated.
 *          pObjTripleLock - lock of the object. Can be NULL. If it is
 *                           supplied, it will be unlocked before callbacks
 *                           to application and locked again after them.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionAuthProceed(
                    IN  RvSipTranscHandle              hTransc,
                    IN  RvSipTransactionAuthAction     action,
                    IN  RvSipAuthorizationHeaderHandle hAuthorization,
                    IN  RvChar*                        password,
                    IN  void*                          hObject,
                    IN  RvSipCommonStackObjectType     eObjectType,
                    IN  SipTripleLock*                 pObjTripleLock)
{
#ifdef RV_SIP_AUTH_ON
    Transaction* pTransc = (Transaction*)hTransc;

    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
        "SipTransactionAuthProceed - Transaction=0x%p, action %d, hAuthorization 0x%p, hObject 0x%p, eObjectType %d",
        pTransc, action, hAuthorization, hObject, eObjectType));

    if (pTransc->eState != RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD &&
        pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD &&
        pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT &&
        pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_PRACK_COMPLETED &&
        pTransc->eState != RVSIP_TRANSC_STATE_SERVER_PRACK_FINAL_RESPONSE_SENT)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "SipTransactionAuthProceed: transc 0x%p - can't do authentication in state %s",
            pTransc,TransactionGetStateName(pTransc->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    return TransactionAuthProceed(hTransc, action, hAuthorization, password,
                                  hObject, eObjectType, pObjTripleLock);
#else /* #ifdef RV_SIP_AUTH_ON */
	RV_UNUSED_ARG(hTransc)
	RV_UNUSED_ARG(action)
	RV_UNUSED_ARG(hAuthorization)
	RV_UNUSED_ARG(password)
	RV_UNUSED_ARG(hObject)
	RV_UNUSED_ARG(eObjectType)
	RV_UNUSED_ARG(pObjTripleLock)
	
	return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_AUTH_ON */
}

/***************************************************************************
* SipTransactionGetReceivedFromAddress
* ------------------------------------------------------------------------
* General: Gets the address from which the transaction received it's last message.
* Return Value: RV_Status.
* ------------------------------------------------------------------------
* Arguments:
* Input:
* hTransaction - Handle to the transaction
* Output:
* pAddr     - basic details about the received from address
* pOptions  - Options about the the address details. If NULL, then only basic
*             data will be retrieved
***************************************************************************/
SipTransportAddress* RVCALLCONV SipTransactionGetReceivedFromAddress(
										IN  RvSipTranscHandle     hTransaction)
{
	Transaction  *pTransaction = (Transaction*)hTransaction;

    return &pTransaction->receivedFrom;
}

#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT 
/***************************************************************************
 * SipTransactionGetKeyFromTransc
 * ------------------------------------------------------------------------
 * General: Fill the Key structure with transaction/s parameters.
 * Return Value: RV_ERROR_BADPARAM - no cseq in transc.
 *                 RV_OK - All the values were updated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc - The transaction.
 * Output:  key     - A structure of the key to fill.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGetKeyFromTransc(
                                    IN  RvSipTranscHandle  hTransc,
                                    OUT SipTransactionKey *pKey)
{
    return TransactionGetKeyFromTransc((Transaction*)hTransc, pKey);
}
#endif /*#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * SipTransactionIsInActiveState
 * ------------------------------------------------------------------------
 * General: The function checks if a transaction is in active state.
 * Return Value: TRUE - if the transaction is active.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 *        hTransc  - Handle to the transaction found in the list.
 ***************************************************************************/
RvBool RVCALLCONV SipTransactionIsInActiveState(IN RvSipTranscHandle hTransc)
{
    Transaction  *pTransaction = (Transaction*)hTransc;

    /* check the transaction state */
    switch(pTransaction->eState)
    {
        case RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD :
        case RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT :
        case RVSIP_TRANSC_STATE_CLIENT_GEN_PROCEEDING:
        case RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING :
        case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING :
        case RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD :
        case RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT :
        case RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT :
        case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING_TIMEOUT :
        case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROXY_2XX_RESPONSE_RCVD :
        case RVSIP_TRANSC_STATE_SERVER_INVITE_PROXY_2XX_RESPONSE_SENT :
        case RVSIP_TRANSC_STATE_SERVER_CANCEL_REQUEST_RCVD :
        case RVSIP_TRANSC_STATE_CLIENT_INVITE_CANCELLING :
        case RVSIP_TRANSC_STATE_CLIENT_GEN_CANCELLING :
        case RVSIP_TRANSC_STATE_CLIENT_CANCEL_SENT :
        case RVSIP_TRANSC_STATE_CLIENT_CANCEL_PROCEEDING :
            /* found an active transaction */
            return RV_TRUE;
        default:
            return RV_FALSE;
    }
}
#endif /*#ifndef RV_SIP_PRIMITIVES*/

/*-----------------------------------------------------------------------*/
/*                             STATIC FUNCTIONS                          */
/*-----------------------------------------------------------------------*/

#if(RV_NET_TYPE & RV_NET_IPV6)
/***************************************************************************
 * CallLegIpv6HashFunction
 * ------------------------------------------------------------------------
 * General: Returns number out of 16 bytes array.
 * Return Value: An integer, the result of the hash function.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     ip6Addr - the ipv6 address in array of 16 bytes format.
 ***************************************************************************/
static RvUint32 RVCALLCONV IdIpv6HashFunction(IN const RvUint8 * ip6Addr)
{
    RvUint8  addrPart = 0;
    RvUint32 sum = 0;
    RvUint32 i;

    for (i = 0; i < 16 ; i++)
    {
        memcpy(&addrPart, &(ip6Addr[i]), sizeof(RvUint8));
        sum = sum + (RvUint32)addrPart;
        addrPart = 0;
    }
    return sum;
}
#endif /* (RV_NET_TYPE & RV_NET_IPV6) */

#endif /*#ifndef RV_SIP_LIGHT*/

