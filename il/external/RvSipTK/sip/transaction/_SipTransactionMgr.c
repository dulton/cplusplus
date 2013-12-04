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
 *                              <SipTransactionMgr.c>
 *
 * This file defines the transactions manager object, attributes and interface
 * functions. A transaction object is implemented by the name
 * SipTransactionMgr. The interface functions defined here implement methods
 * to construct and destruct SipTransactionMgr object; and functions to
 * control the elements of the data-structure: methods to insert, delete and
 * find an element within the data-structure.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include <string.h>
#include "RvSipTransmitter.h"
#include "RvSipMsg.h"
#include "RvSipCSeqHeader.h"
#include "RvSipViaHeader.h"
#include "RvSipEventHeader.h"
#include "RvSipTransport.h"
#include "RvSipPartyHeader.h"
#include "_SipCommonConversions.h"
#include "_SipViaHeader.h"
#include "_SipCSeqHeader.h"
#include "_SipMsg.h"
#include "_SipTransport.h"
#include "_SipTransportTypes.h"
#include "_SipPartyHeader.h"
#include "_SipCommonCore.h"
#include "_SipCommonUtils.h"
#include "_SipTransactionMgr.h"
#include "_SipTransmitter.h"
#include "TransactionObject.h"
#include "TransactionMgrObject.h"
#include "TransactionMsgReceived.h"
#include "TransactionAuth.h"
#include "TransactionControl.h"
#include "TransactionObject.h"
#include "TransactionTransport.h"
#include "TransactionCallBacks.h"
#include "AdsRlist.h"
#ifdef RV_SIP_IMS_ON
#include "_SipSecAgreeTypes.h"
#include "_SipSecAgreeMgr.h"
#include "TransactionSecAgree.h"
#endif /* #ifdef RV_SIP_IMS_ON */
#ifdef RV_SIGCOMP_ON
#include "_SipCompartment.h"
#endif

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV HandleIncomingRequestMsg(
                            IN    TransactionMgr                   *pTranscMgr,
                            IN    RvSipMsgHandle                    hReceivedMsg,
                            IN    RvSipTransportConnectionHandle    hConn,
                            IN    RvSipTransportLocalAddrHandle     hLocalAddr,
                            IN    SipTransportAddress              *pRecvFrom,
                            INOUT SipTransportSigCompMsgInfo       *pSigCompMsgInfo,
                            OUT   RvInt16                          *pFailureResp);

static RvStatus RVCALLCONV HandleIncomingResponseMsg(
                            IN    TransactionMgr                 *pTranscMgr,
                            IN    RvSipMsgHandle                  hReceivedMsg,
                            IN    RvSipTransportConnectionHandle  hConn,
                            IN    RvSipTransportLocalAddrHandle   hLocalAddr,
                            IN    SipTransportAddress            *pRecvFrom,
                            INOUT SipTransportSigCompMsgInfo     *pSigCompInfo);

static RvBool RVCALLCONV IsNewTranscRequired(
                            IN TransactionMgr*                pTranscMgr,
                            IN RvSipMsgHandle                 hReceivedMsg,
                            IN SipTransactionKey*             pTranscKey,
                            IN RvSipTransportLocalAddrHandle  hLocalAddr,
                            IN SipTransportAddress*           pRcvdFromAddr,
                            IN RvSipTransportAddrOptions*     pOptions,
                            IN RvSipTransportConnectionHandle hConn);

#ifndef RV_SIP_PRIMITIVES
static RvStatus RVCALLCONV IsCallLegNotificationRequired(IN Transaction* pTransc,
                                                          OUT RvBool   *bNotifyMgr);
#endif /*#ifndef RV_SIP_PRIMITIVES*/

static void RVCALLCONV AttachServerCancelToServerInvite(
                                                       IN Transaction* pTransc);

static RvStatus RVCALLCONV CreateNewServerTransaction(
                                               IN TransactionMgr        *pTranscMgr,
                                               IN RvSipMsgHandle         hReceivedMsg,
                                               IN SipTransactionKey     *pKey,
                                               OUT Transaction          **pTransc);

static RvStatus RVCALLCONV UpdateServerTransactionOwner(
                                               IN TransactionMgr        *pTranscMgr,
                                               IN Transaction           *pTransc,
                                               IN RvSipMsgHandle         hReceivedMsg,
                                               IN SipTransactionKey     *pKey,
                                               IN RvBool                bIsApplicationMsg,
                                               IN RvSipTranscOwnerHandle hTranscOwner);

static void RVCALLCONV HandleViaHeaderParameters(
                               IN TransactionMgr             *pMgr,
                               IN RvSipMsgHandle              hMsg,
                               IN SipTransportAddress        *pRecvFrom);

static RvStatus RVCALLCONV AddReceivedParameter(
                                             IN TransactionMgr*       pMgr,
                                             IN RvSipMsgHandle        hMsg,
                                             IN SipTransportAddress*  pRecvFrom);
static RvStatus RVCALLCONV AddRportParameter(
                                             IN RvSipMsgHandle       hMsg,
                                             IN SipTransportAddress* pRecvFrom);

static RvStatus RVCALLCONV CheckCSeqMethod( IN RvSipMsgHandle        hMsg);

#ifndef RV_SIP_PRIMITIVES
static RvBool IsInitialRequest(IN Transaction* pTransc);
#endif /*#ifndef RV_SIP_PRIMITIVES*/

static RvStatus UpdateRequestTransaction(IN Transaction   *pTransc,
                                          IN RvSipMsgHandle hMsg);
static RvStatus UpdateTranscConnection(
                       IN Transaction                       *pTransc,
                       IN  RvSipTransportConnectionHandle   hConn,
                       IN  RvBool                          bIsRequest);



static RvStatus RVCALLCONV TranscMgrRejectMsgNoTransc(
                            IN    TransactionMgr                *pTranscMgr,
                            IN    RvSipTransportLocalAddrHandle  hLocalAddr,
                            IN    RvSipMsgHandle                 hMsg,
                            IN    RvInt16                        responseCode,
                            IN    RvBool                         bAddToTag);

static RvStatus RVCALLCONV RequestMessageReceived(
                         IN    TransactionMgr                 *pTranscMgr,
                         IN    RvSipMsgHandle                  hReceivedMsg,
                         IN    RvSipTransportConnectionHandle  hConn,
                         IN    RvSipTransportLocalAddrHandle   hLocalAddr,
                         IN    SipTransportAddress*            pRecvFromAddr,
                         IN    RvSipTransportBsAction          eBSAction,
                         INOUT SipTransportSigCompMsgInfo     *pSigCompMsgInfo);

static RvStatus RVCALLCONV ResponseMessageReceived(
                         IN    TransactionMgr                 *pTranscMgr,
                         IN    RvSipMsgHandle                 hReceivedMsg,
                         IN    RvSipTransportConnectionHandle hConn,
                         IN    RvSipTransportLocalAddrHandle  hLocalAddr,
                         IN    SipTransportAddress           *pRecvFromAddr,
                         IN    RvSipTransportBsAction         eBSAction,
                         INOUT SipTransportSigCompMsgInfo    *pSigCompMsgInfo);

#ifndef RV_SIP_PRIMITIVES
static RvStatus RVCALLCONV UpdateCancelTransactionOwner(
                                       IN  Transaction           *pTransc,
                                       IN  SipTransactionKey     *pKey,
                                       OUT void                  **ppOwner);
#endif /*#ifndef RV_SIP_PRIMITIVES*/
static RvStatus UpdateTranscSavedMsgToAndFrom(
                       IN  Transaction     *pTransc,
                       IN  RvSipMsgHandle   hReceivedMsg);
static RvStatus RVCALLCONV MergeRequestIfNeeded(
                                       IN  TransactionMgr       *pTranscMgr,
                                       IN  RvSipMsgHandle        hMsg,
                                       IN  SipTransactionKey    *pKey,
                                       OUT RvInt16              *pFailureResp,
                                       OUT RvBool               *bMerged,
                                       OUT Transaction          **pMergedTransc);

static void RVCALLCONV CheckFoundTranscValidity(
                                         IN  Transaction*   pTransc,
                                         IN  RvSipMsgHandle hMsg,
                                         IN  RvBool         bIsRespOnInvite,
                                         OUT RvBool*        pbTranscValid);


static RvBool VerifyResponseAndTranscTags(Transaction* pTransc,
                                         RvSipMsgHandle hReceivedReponseMsg);

static RvBool Is1xx2xxRespWithToTagOnInviteMsg(RvSipMsgHandle  hReceivedMsg);

static void RVCALLCONV NotifyAppOfResponseWithNoTransc(
                            IN TransactionMgr*                 pTranscMgr,
                            IN RvBool                          bIsRespOnInvite,
                            IN RvSipMsgHandle                  hMsg,
                            IN RvSipTransportLocalAddrHandle   hLocalAddr,
                            IN SipTransportAddress*            pRcvdFromAddr,
                            IN RvSipTransportAddrOptions*      pOptions,
                            IN RvSipTransportConnectionHandle  hConn);

#ifdef RV_SIP_IMS_ON
static void RVCALLCONV SetSecAgreeEvHandlers(
                               IN RvSipSecAgreeMgrHandle   hSecAgreeMgr);
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIGCOMP_ON
static RvStatus RVCALLCONV HandleIncomingSigCompAckIfNeeded(
                             IN    TransactionMgr                  *pMgr,
                             IN    RvSipTransportConnectionHandle   hConn,
                             IN    SipTransportAddress             *pRecvFrom,
                             IN    RvSipTransportLocalAddrHandle    hLocalAddr,
                             IN    RvSipMsgHandle                   hMsg, 
                             INOUT SipTransportSigCompMsgInfo      *pSigCompMsgInfo);
#endif /* RV_SIGCOMP_ON */

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * SipTransactionMgrConstruct
 * ------------------------------------------------------------------------
 * General: Called in the Stack initialization process.
 * Initiates data-structures. Sets global memory-pools, timer-pool,
 * log Id number to the manager. Signs transaction default callbacks to the
 * manager.
 * Return Value: RV_OK - The out parameter pTranscMgr now points
 *                            to a valid transaction manager object.
 *                 RV_ERROR_OUTOFRESOURCES - Couldn't allocate memory for the
 *                                     transaction manager object.
 *                 RV_ERROR_NULLPTR - pConfiguration or phTranscMgr are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pConfiguration - Managers parameters as were defined in the
 *                             configuration.
 * Output:    phTranscMgr          - The new transaction manager object created.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrConstruct(
                              INOUT  SipTranscMgrConfigParams  *pConfiguration,
                              OUT    RvSipTranscMgrHandle      *phTranscMgr)
{
    RvStatus          rv;
    RvStatus           crv;
    RvLogSource       *pLogSrc;
    TransactionMgr    *pTranscMgr = NULL;


    *phTranscMgr = NULL;

    pLogSrc = pConfiguration->pLogSrc;

    /*allocate the transaction manager*/
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(TransactionMgr), pConfiguration->pLogMgr, (void*)&pTranscMgr))
    {
        RvLogError(pLogSrc ,(pLogSrc ,
                 "SipTransactionMgrConstruct - Error in constructing the transaction manager"));
        return RV_ERROR_OUTOFRESOURCES;
    }
    memset(pTranscMgr,0,sizeof(TransactionMgr));

    /*allocate the manager Mutex*/
    crv = RvMutexConstruct(NULL, &pTranscMgr->hLock);
    if(crv != RV_OK)
    {
        RvLogError(pLogSrc ,(pLogSrc ,
                 "SipTransactionMgrConstruct - Failed to construct mutex for manager (rv=%d)",CRV2RV(crv)));

        RvMemoryFree(pTranscMgr, pTranscMgr->pLogMgr);
        return CRV2RV(crv);
    }

	/*allocating mutex for switching locks between transaction and its manager.*/
    crv = RvMutexConstruct(NULL, &pTranscMgr->hSwitchSafeLock);
    if(crv != RV_OK)
    {
        RvLogError(pLogSrc ,(pLogSrc ,
                 "SipTransactionMgrConstruct - Failed to construct switching locks mutex for manager (rv=%d)",CRV2RV(crv)));

        RvMemoryFree(pTranscMgr, pTranscMgr->pLogMgr);
        return CRV2RV(crv);
    }

    rv = TransactionMgrInitialize(pTranscMgr,pConfiguration);

    if(rv != RV_OK)
    {
        RvLogError(pLogSrc ,(pLogSrc ,
                 "SipTransactionMgrConstruct - Failed to construct the transaction manager - freeing resources"));
        SipTransactionMgrDestruct((RvSipTranscMgrHandle)pTranscMgr);
        return rv;
    }

    *phTranscMgr = (RvSipTranscMgrHandle)pTranscMgr;

    RvLogInfo(pLogSrc ,(pLogSrc ,
             "SipTransactionMgrConstruct - Transaction manager was constructed successfully"));
    return RV_OK;
}

/***************************************************************************
 * SipTransacMgrSetEnableNestedInitialRequest
 * ------------------------------------------------------------------------
 * General: Sets the bEnableNestedInitialRequestHandling parameter to RV_TRUE.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr - A handle to the transaction manager
 ***************************************************************************/
void RVCALLCONV SipTransacMgrSetEnableNestedInitialRequest(
                                   IN RvSipTranscMgrHandle hTranscMgr)
{
    TransactionMgr      *pTranscMgr;

    pTranscMgr = (TransactionMgr *)hTranscMgr;
    if (NULL == pTranscMgr)
    {
        return;
    }
    RvMutexLock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);

    pTranscMgr->bEnableNestedInitialRequestHandling = RV_TRUE;

    RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
}
/***************************************************************************
 * SipTransactionMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the transactions data - structure.
 * Return Value: RV_OK - The transaction manager has been successfully
 *                            destructed.
 *               RV_ERROR_NULLPTR - The pointer to the transaction manager to be
 *                               deleted (received as a parameter) is a NULL
 *                                 pointer.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscMgr - A handle to the transaction manager that will be
 *                       destructed.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrDestruct(
                           IN RvSipTranscMgrHandle hTranscMgr)
{
    TransactionMgr      *pTranscMgr;

    pTranscMgr = (TransactionMgr *)hTranscMgr;

    if (NULL == pTranscMgr)
    {
        return RV_ERROR_NULLPTR;
    }

    TransactionMgrDestruct(pTranscMgr);

    return RV_OK;
}


/***************************************************************************
 * SipTransactionMgrSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets a set of event handlers identified with the entity received
 * in the eOwnerType parameter.
 * Each of the entities which can own a transaction must attach a
 * set of implementations to the transaction's callbacks.
 * Return Value: RV_ERROR_NULLPTR - The pointer to the handlers struct is NULL.
 *                 RV_OK - The event handlers were successfully set.
 *                 RV_ERROR_BADPARAM - Size of struct is negative,
 *                                       or owner enumeration is UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscMgr - A handle to the transaction manager to set the
 *                       handlers to.
 *            eOwnerType - The type of the owner to which the event handlers
 *                       belong.
 *          pHandlers  - The struct of the event handlers.
 *          evHandlerStructSize - The size of the pHandlers struct.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrSetEvHandlers(
                     IN RvSipTranscMgrHandle               hTranscMgr,
                     IN SipTransactionOwner                eOwnerType,
                     IN SipTransactionMgrEvHandlers       *pMgrDetails,
                     IN RvUint32                          evMgrHandlerStructSize,
                     IN RvSipTransactionEvHandlers        *pHandlers,
                     IN RvUint32                          evHandlerStructSize)
{
    TransactionMgr *pTranscMgr;
    RvLogSource*   logId;
 
	RV_UNUSED_ARG(evMgrHandlerStructSize)

    pTranscMgr = (TransactionMgr *)hTranscMgr;

    if ((NULL == hTranscMgr) ||
        (NULL == pHandlers)  ||
        (NULL == pMgrDetails))
    {
        return RV_ERROR_NULLPTR;
    }

    RvMutexLock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    logId = pTranscMgr->pLogSrc;
    if (evHandlerStructSize == 0)
    {
        RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
        return RV_ERROR_BADPARAM;
    }
    switch (eOwnerType)
    {
    case (SIP_TRANSACTION_OWNER_NON):
            /* Set default callbacks */
            memset(&(pTranscMgr->pDefaultEvHandlers), 0, evHandlerStructSize);
            memcpy(&(pTranscMgr->pDefaultEvHandlers), pHandlers,
                   evHandlerStructSize);
            RvLogDebug(logId ,(logId ,
                      "SipTransactionMgrSetEvHandlers - Default event handlers were successfully set to the transaction manager"));
            RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
            return RV_OK;
#ifndef RV_SIP_PRIMITIVES
    case (SIP_TRANSACTION_OWNER_CALL):
            /* The manager signs in is the Call manager:
               Set call's callback*/
            memset(&(pTranscMgr->pCallLegEvHandlers), 0, evHandlerStructSize);
            memcpy(&(pTranscMgr->pCallLegEvHandlers), pHandlers,
                   evHandlerStructSize);

            /* Set call manager callback */
            memset(&(pTranscMgr->pCallMgrDetail), 0, evMgrHandlerStructSize);
            memcpy(&(pTranscMgr->pCallMgrDetail), pMgrDetails,
                    evMgrHandlerStructSize);
            
            /* Set the call manager to the transaction manager */
            (pTranscMgr->pCallMgrDetail).pMgr = pMgrDetails->pMgr;
            RvLogDebug(logId ,(logId ,
                      "SipTransactionMgrSetEvHandlers - Transaction event handlers were successfully set to the transaction manager"));
            RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
            return RV_OK;
    case (SIP_TRANSACTION_OWNER_TRANSC_CLIENT):
            /* The manager signs in is the Register-client manager:
               Set call's callback*/
            memset(&(pTranscMgr->pTranscClientEvHandlers), 0, evHandlerStructSize);
            memcpy(&(pTranscMgr->pTranscClientEvHandlers), pHandlers,
                   evHandlerStructSize);
            RvLogDebug(logId ,(logId ,
                      "SipTransactionMgrSetEvHandlers - Register transc-client event handlers was successfully set to the transaction manager"));
            RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
            return RV_OK;
#endif /* RV_SIP_PRIMITIVES */
    default:
        /* Owner type is not supported */
        RvLogError(logId ,(logId ,
            "SipTransactionMgrSetEvHandlers - Error, function was called for undefined owner type"));
        RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
        return RV_ERROR_BADPARAM;
    }
}


/***************************************************************************
* SipTransactionMgrMessageReceived
* ------------------------------------------------------------------------
* General: Searches the data - structure for a transaction
* uniquely defined by the key values that were received in the message.
* If the transaction exist it will receive the
* new message. If it is doesn't exist, and a new transaction should be
* created (when the message is a request message, except for an ACK request),
* a new transaction is created. A new transaction created will also
* be given the new message received but before that the event that indicates a
* new transaction will be called.
* Return Value: RV_OK - The message has been received by a compatible
*                              transaction.
*                 RV_ERROR_NULLPTR - The transaction manager pointer or the message
*                               pointer are NULL pointers
*                 RV_OutOfResource - Couldn't allocate a new transaction when
*                                    required.
*               RV_ERROR_BADPARAM - The message is invalid.
* ------------------------------------------------------------------------
* Arguments:
* Input:  pReceivedMsg -   A message belonging to the transaction this
*                          function is searching for. There can be only
*                          one transaction this message belongs to.
*          hTranscMgr  -   The transactions manager.
*          pLocalAddr  -   The local address to which the request message was
*                          received.
*          pRecvFromAddr - The address from which the message was received.
*          eBSAction    -  Bad-syntax action, that was given by application in
*                          bad-syntax callbacks. continue processing, reject, discard.
*          pSigCompMsgInfo - SigComp info related to the received message.
*                            The information contains indication if the UDVM
*                            waits for a compartment ID for the given unique
*                            ID that was related to message by the UDVM.
***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrMessageReceivedEv(
                         IN    SipTransportCallbackContextHandle msgContext,
                         IN    RvSipMsgHandle                    hReceivedMsg,
                         IN    RvSipTransportConnectionHandle    hConn,
                         IN    RvSipTransportLocalAddrHandle     hLocalAddr,
                         IN    SipTransportAddress*              pRecvFromAddr,
                         IN    RvSipTransportBsAction            eBSAction,
                         IN    SipTransportSigCompMsgInfo       *pSigCompMsgInfo)
{
    TransactionMgr        *pTranscMgr;
    RvSipMsgType           eType;
    RvStatus              rv;

    pTranscMgr = (TransactionMgr*)msgContext;

    if ((NULL == pTranscMgr) ||
        (NULL == hReceivedMsg))
    {
        return RV_ERROR_NULLPTR;
    }

    if(eBSAction == RVSIP_TRANSPORT_BS_ACTION_DISCARD_MSG)
    {
        /* application chose to discard this incoming request, because of
           syntax-errors in it */
        RvLogInfo(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "SipTransactionMgrMessageReceivedEv - Discard message 0x%p. eBSAction=%d",
            hReceivedMsg, eBSAction));
#ifdef RV_SIGCOMP_ON
        TransactionMgrFreeUnusedSigCompResources(pTranscMgr,pSigCompMsgInfo);
#endif /* RV_SIGCOMP_ON */

        return RV_OK;
    }

    eType = RvSipMsgGetMsgType(hReceivedMsg);

    if (RVSIP_MSG_REQUEST == eType)
    {
        rv = RequestMessageReceived(pTranscMgr,
                                    hReceivedMsg,
                                    hConn,
                                    hLocalAddr,
                                    pRecvFromAddr,
                                    eBSAction,
                                    pSigCompMsgInfo);
        if (rv != RV_OK)
        {
            RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                "SipTransactionMgrMessageReceivedEv - Failed to handle incoming request message 0x%p.",hReceivedMsg));
        }
        /* Don't add "return rv;" here since TransactionMgrFreeUnusedSigCompResources() has to be called below */ 
    }
    else if (RVSIP_MSG_RESPONSE == eType)
    {
        rv = ResponseMessageReceived(pTranscMgr,
                                     hReceivedMsg,
                                     hConn,
                                     hLocalAddr,
                                     pRecvFromAddr,
                                     eBSAction,
                                     pSigCompMsgInfo);
        if(rv != RV_OK)
        {
            if (RV_ERROR_DESTRUCTED==rv)
            {
                RvLogWarning(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                    "SipTransactionMgrMessageReceivedEv - Failed to handle incoming response message 0x%p",hReceivedMsg));
            }
            else
            {
                RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                "SipTransactionMgrMessageReceivedEv - Failed to handle incoming response message 0x%p",hReceivedMsg));
            }
        }
        /* Don't add "return rv;" here since TransactionMgrFreeUnusedSigCompResources() has to be called below */ 
    }
    else
    {
        /* The message is neither a request nor a response */
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                  "SipTransactionMgrMessageReceivedEv - Failed. The message 0x%p type is not defined",
                  hReceivedMsg));

        rv = RV_ERROR_BADPARAM;
        /* Don't add "return rv;" here since TransactionMgrFreeUnusedSigCompResources() has to be called below */ 
    }

#ifdef RV_SIGCOMP_ON
    TransactionMgrFreeUnusedSigCompResources(pTranscMgr,pSigCompMsgInfo);
#endif /* RV_SIGCOMP_ON */

    return rv;
}

#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT
/***************************************************************************
 * SipTransactionMgrReprocessTransactionEv
 * ------------------------------------------------------------------------
 * General: This function is called from the event queue due to its insertion
 *          by RvSipTransactionMgrReprocessTransaction().
 *          Reprocess a transaction: The transaction manager will terminate 
 *          the given transaction, and will reprocess the request that was 
 *          received by it via a new transaction. 
 *          Notice: The transaction you supply here must be in state 
 *          SERVER_GEN_REQUEST_RCVD or in state SERVER_INVITE_REQUEST_RCVD, 
 *          and it must have a valid hReceivedMsg.
 *          Notice: The TransactionTerminated() callback will notify the 
 *          termination of the old transaction. The CallLegTransactionCreated() 
 *          callback will notify the creation of the new transaction. The new 
 *          transaction will notify the processing of the request via callbacks 
 *          MsgRcvd(), StateChanged(), etc.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc     - The transaction to be reprocessed.
 *          
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrReprocessTransactionEv(IN  void        *hTransc,
                                                            IN  RvInt32      notUsed)
{
    RvStatus                          rv;
    Transaction                      *pTransc = (Transaction*)hTransc;
	TransactionMgr                   *pTranscMgr = pTransc->pMgr;
    RvSipMsgHandle                    hReceivedMsg;
	RvSipTransportConnectionHandle    hConn;
	RvSipTransportLocalAddrHandle    *phLocalAddr;
	RvSipTransportLocalAddrHandle     hLocalAddr;
	SipTransportAddress              *pRecvFrom;
	RvSipTransportBsAction            eBSAction;
	
	RvLogInfo(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
		"SipTransactionMgrReprocessTransactionEv - Reprocessing transaction 0x%p", hTransc));

    RvMutexLock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
	/* remove the old transaction from the hash */
	TransactionMgrHashRemove(hTransc);
    RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    
	/* Lock the old transaction in order to load its data */
	if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
	if (pTransc->eState != RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD &&
		pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD)
	{
		RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
				   "SipTransactionMgrReprocessTransactionEv - Reprocessing cannot occur in state other than REQUEST_RCVD. Transaction=0x%p, State=%s", hTransc, TransactionGetStateName(pTransc->eState)));
		TransactionUnLockAPI(pTransc);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	/* The received message was stores in hInitialRequestReceivedMsg specifically for reprocessing
	   It was never terminated due to incrementing its owners index by 1 */
	hReceivedMsg    = pTransc->hInitialRequestForReprocessing;
	/* The received from connection was stored in the transaction on the receipt of the message.
	   It is important that we initialize the new transaction with this connection, before we
	   detach the old transaction from it - otherwise the connection might be closed by the
	   old transaction. If the connection was closed in other circumstances, we will set NULL
	   to the new transaction, and the new transaction will be responsible to open a new connection
	   when needed */
	hConn           = pTransc->hActiveConn;
	/* We copy the address in use from the transmitter. This address was initialized with the local
	   address that the request message was received on. We assume that no changes to the address 
	   in use were made since the message was received. If changes were made, they will be reflected 
	   in the new transaction as well */
	phLocalAddr = SipTransmitterGetAddrInUse(pTransc->hTrx);
	if (phLocalAddr == NULL)
	{
		hLocalAddr = NULL;
	}
	else
	{
		hLocalAddr = *phLocalAddr;
	}
	/* The received from address was stored in the transaction on the receipt of the message */
	pRecvFrom       = &pTransc->receivedFrom;
	/* We can assume that if we got to this stage, either there was no bad syntax, or
	   the BS action was CONTINUE */
	eBSAction       = RVSIP_TRANSPORT_BS_ACTION_CONTINUE_PROCESS;
	

	/* Notice: the original transaction is still locked while calling the transaction manager
	   with the request message. This way the resources we supply here will remain valid.
	   Since locking a transaction before the transaction manager is allowed, no deadlock is 
	   expected */
	rv = RequestMessageReceived(pTranscMgr, hReceivedMsg, hConn, hLocalAddr, pRecvFrom, eBSAction, NULL);
	if (RV_OK != rv)
	{
		RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
					"SipTransactionMgrReprocessTransactionEv - Reprocessing failed in RequestMessageReceived(). Transaction=0x%p, rv=%d", hTransc, rv));
	}

	/* We terminate the original transaction even if the returned status of RequestMessageReceived()
	   indicates failure. We trust the transaction manager to automatically respond on the
	   new transaction in case that error occurs, therefore responding on the old
	   transaction is not needed. There is still a possibility that the transaction manager
	   did not manage to respond on the new transaction, which will indicate an extreme failure, 
	   but we will not answer on the old transaction in that case as well */
	TransactionTerminate(RVSIP_TRANSC_REASON_REPROCESSED_BY_ANOTHER_TRANSC, pTransc);
	TransactionUnLockAPI(pTransc);

	RV_UNUSED_ARG(notUsed);
    return rv;
}
#endif /* #ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */

/***************************************************************************
* SipTransactionMgrCreateServerTransactionFromMsg
* ------------------------------------------------------------------------
* General: Called to create a new Server transaction from a request message that
 *         the application supplies. This function creates a new transaction in the
 *         Request-received state.
* Return Value: RV_OK - The message has been received by a compatible
*                              transaction.
*                 RV_ERROR_NULLPTR - The transaction manager pointer or the message
*                               pointer are NULL pointers
*                 RV_OutOfResource - Couldn't allocate a new transaction when
*                                    required.
*               RV_ERROR_BADPARAM - The message is invalid.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hTranscMgr   -   The transactions manager.
*          hTranscOwner -   The application owner of the transaction.
*          hMsg         -   The handle to the request message that the application
*                           supplies to create the transaction
* Output:  pTransc      -   A pointer to the address of the new transaction created.
***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrCreateServerTransactionFromMsg(
                                              IN  TransactionMgr          *pTranscMgr,
                                              IN  RvSipTranscOwnerHandle   hTranscOwner,
                                              IN  RvSipMsgHandle           hMsg,
                                              OUT Transaction             **pTransc)
{
    RvStatus                         rv;
    SipTransactionKey                 key;

    if ((NULL == hMsg))
    {
        return RV_ERROR_NULLPTR;
    }

    memset(&key,0,sizeof(key));
    RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                "SipTransactionMgrCreateServerTransactionFromMsg - Creating a transaction to message 0x%p",
                hMsg));

    *pTransc = NULL;

    /*---------------------------------------------------------
    Correct To header before processing the request
    -----------------------------------------------------------*/
    rv = TransactionCorrectToHeaderUrl(hMsg);
    if (RV_OK != rv)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "SipTransactionMgrCreateServerTransactionFromMsg - Failed to correct To header in msg 0x%p- header may not exist",
            hMsg));
        return rv;
    }
    /*------------------------------------
      Get the key from the message
    --------------------------------------*/
    rv = SipTransactionMgrGetKeyFromMessage((RvSipTranscMgrHandle)pTranscMgr, hMsg, &key);
    if (RV_OK != rv)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "SipTransactionMgrCreateServerTransactionFromMsg - Failed. An illegal message was received by the transaction manager"));
        return RV_ERROR_BADPARAM;
    }

    /*------------------------------------------------
       Look for a transaction this request belongs to
      -------------------------------------------------*/
    RvMutexLock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    rv = SipTransactionMgrFindTransaction(pTranscMgr, hMsg,&key, RV_FALSE,pTransc);

    if (RV_OK != rv)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "SipTransactionMgrCreateServerTransactionFromMsg - Failed when looking for transaction in hash"));
        RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
        return rv;
    }
    if(*pTransc == NULL)
    {
        /*---------------------------------------------------
              create transaction
        ----------------------------------------------------*/
        rv =  CreateNewServerTransaction(pTranscMgr, hMsg, &key,
                                         pTransc);

        if(rv != RV_OK)
        {
            RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                       "SipTransactionMgrCreateServerTransactionFromMsg - Failed to create new transaction for message"));
            RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
            return rv;
        }

        rv = UpdateRequestTransaction(*pTransc, hMsg);
        if(rv != RV_OK)
        {
            RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                "SipTransactionMgrCreateServerTransactionFromMsg - UpdateRequestTransaction() Failed"));
            TransactionDestruct(*pTransc);
            RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
            return rv;
        }

		/*First we need to Unlock the mgr and then lock the transaction in order to keep the locking
		  hierarchy*/
        RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
		RvMutexLock(&(*pTransc)->transactionTripleLock.hProcessLock,(*pTransc)->pMgr->pLogMgr);

        rv = UpdateServerTransactionOwner(pTranscMgr, *pTransc, hMsg, &key,
                                          RV_TRUE, hTranscOwner);
        if(rv != RV_OK)
        {
            RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                       "SipTransactionMgrCreateServerTransactionFromMsg - Failed to update server transaction owner"));
            RvMutexUnlock(&(*pTransc)->transactionTripleLock.hProcessLock,(*pTransc)->pMgr->pLogMgr);
            *pTransc = NULL;
            return rv;
        }
        RvMutexUnlock(&(*pTransc)->transactionTripleLock.hProcessLock,(*pTransc)->pMgr->pLogMgr);
    }
    else
    {
        /*transaction was found*/
        RvLogInfo(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                  "SipTransactionMgrCreateServerTransactionFromMsg - Found existing transaction 0x%p for the message",
                  *pTransc));
        RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
        RvMutexLock(&(*pTransc)->transactionTripleLock.hProcessLock,(*pTransc)->pMgr->pLogMgr);
        RvMutexUnlock(&(*pTransc)->transactionTripleLock.hProcessLock,(*pTransc)->pMgr->pLogMgr);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /*do not add code here - only before the UpdateServerTransactionOwner
      function where the transaction is not known to the application*/

    return RV_OK;
}





#ifndef RV_SIP_PRIMITIVES

/***************************************************************************
 * SipTransactionMgrGet100relStatus
 * ------------------------------------------------------------------------
 * General: returns the 100 rel status kept in the transaction manager
 * Return Value: 100 rel status of the transaction manager
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr - The transaction manager.
 *
 ***************************************************************************/
RvSipTransaction100RelStatus RVCALLCONV SipTransactionMgrGet100relStatus(
                                    IN RvSipTranscMgrHandle hTranscMgr)
{
    TransactionMgr        *pTranscMgr;
    pTranscMgr = (TransactionMgr*)hTranscMgr;
    return pTranscMgr->status100rel;
}

/***************************************************************************
 * SipTransactionMgrGetT1
 * ------------------------------------------------------------------------
 * General: returns the T1 timeout value kept in the transaction manager
 * Return Value: 100 rel status of the transaction manager
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr - The transaction manager.
 ***************************************************************************/
RvInt32 RVCALLCONV SipTransactionMgrGetT1(
                                    IN RvSipTranscMgrHandle hTranscMgr)
{
    return ((TransactionMgr*)hTranscMgr)->pTimers->T1Timeout;
}

/***************************************************************************
 * SipTransactionMgrGetT2
 * ------------------------------------------------------------------------
 * General: returns the T2 timeout value kept in the transaction manager
 * Return Value: 100 rel status of the transaction manager
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr - The transaction manager.
 ***************************************************************************/
RvInt32 RVCALLCONV SipTransactionMgrGetT2(
                                    IN RvSipTranscMgrHandle hTranscMgr)
{
    return ((TransactionMgr*)hTranscMgr)->pTimers->T2Timeout;
}
#endif /*#ifndef RV_SIP_PRIMITIVES */

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipTransactionMgrSetSecAgreeMgrHandle
 * ------------------------------------------------------------------------
 * General: Sets the security-agreement manager handle in transaction mgr.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr         - Handle to the transaction manager.
 *            hSecAgreeMgr - Handle to the security-agreement manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrSetSecAgreeMgrHandle(
                                        IN RvSipTranscMgrHandle     hMgr,
                                        IN RvSipSecAgreeMgrHandle   hSecAgreeMgr)
{
    TransactionMgr* pMgr = (TransactionMgr*)hMgr;

	/* Update handle */
    pMgr->hSecAgreeMgr = hSecAgreeMgr;

	/* Set event handlers */
	SetSecAgreeEvHandlers(hSecAgreeMgr);

    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_ON */

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * HandleIncomingRequestMsg
 * ------------------------------------------------------------------------
 * General: Handles an incoming request message. look for a transaction or
 *          create new one.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr   - The transaction manager.
 *          hReceivedMsg - the received message.
 *          pLocalAddr   - The local address to which the message was
 *                         received.
 *          pRecvFrom    - The address from which the message was received.
 *          SipTransportSigCompMsgInfo -
 *                         Message information concerning SigComp.
 * Output:  pFailureResp - in case of failure, indicated whether to send
 *                          '400 bad-syntax' or '500 server internal error' response.
 ***************************************************************************/
static RvStatus RVCALLCONV HandleIncomingRequestMsg(
                            IN    TransactionMgr                   *pTranscMgr,
                            IN    RvSipMsgHandle                    hReceivedMsg,
                            IN    RvSipTransportConnectionHandle    hConn,
                            IN    RvSipTransportLocalAddrHandle     hLocalAddr,
                            IN    SipTransportAddress              *pRecvFrom,
                            INOUT SipTransportSigCompMsgInfo       *pSigCompMsgInfo,
                            OUT   RvInt16                          *pFailureResp)
{
    RvStatus           rv;
    SipTransactionKey  key;
    Transaction*       pTransc;
    RvBool             bOpenTransaction = RV_FALSE;
    RvInt32            curTranscIdentifier;
#ifdef RV_SIGCOMP_ON
	RvSipMethodType	   eMethodType;
#endif

    RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                "HandleIncomingRequestMsg - A request msg 0x%p,Connection 0x%p - starting to process...",
                hReceivedMsg,hConn));

    HandleViaHeaderParameters(pTranscMgr,hReceivedMsg,pRecvFrom);

    *pFailureResp = 400; /* If this function fails, send 400 by default */

    /*---------------------------------------------------------
    Correct To header before processing the request
    -----------------------------------------------------------*/
    rv = TransactionCorrectToHeaderUrl(hReceivedMsg);
    if (RV_OK != rv)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "HandleIncomingRequestMsg - Failed to correct To header in msg 0x%p- header may not exist",
            hReceivedMsg));
        return rv;
    }

    /*---------------------------------------------------------
    Check if the methods in the start-line and in the cseq header
    are the same.
    -----------------------------------------------------------*/
    rv = CheckCSeqMethod(hReceivedMsg);
    if (RV_OK != rv)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "HandleIncomingRequestMsg - methods are not the same in cseq and in request line of msg 0x%p",
            hReceivedMsg));
        return rv;
    }

    /*------------------------------------
      Get the key from the message
    --------------------------------------*/
    memset(&key,0,sizeof(key));
    rv = SipTransactionMgrGetKeyFromMessage((RvSipTranscMgrHandle)pTranscMgr,
                                            hReceivedMsg, &key);
    if (RV_OK != rv)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "HandleIncomingRequestMsg - Failed. An illegal message was received by the transaction manager"));
        return RV_ERROR_BADPARAM;
    }

    /*------------------------------------------------
       Look for a transaction this request belongs to
      -------------------------------------------------*/
    RvMutexLock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    rv = SipTransactionMgrFindTransaction(pTranscMgr, hReceivedMsg,&key, RV_FALSE,&pTransc);
    if (RV_OK != rv)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "HandleIncomingRequestMsg - Failed when looking for transaction in hash"));
        RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
        return rv;
    }


    /*-------------------------------------------------
        Transaction was not found
      -------------------------------------------------*/
    if(pTransc == NULL)
    {
        RvSipTransportAddrOptions addrOptions;

        memset(&addrOptions,0,sizeof(addrOptions));
        addrOptions.eCompression = RVSIP_COMP_UNDEFINED;
#ifdef RV_SIGCOMP_ON        
        if(pSigCompMsgInfo != NULL && pSigCompMsgInfo->bExpectComparmentID == RV_TRUE)
        {
            addrOptions.eCompression = RVSIP_COMP_SIGCOMP;
        }
#endif /* #ifdef RV_SIGCOMP_ON */
        RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
        /*determine whether to open a new transaction*/
        bOpenTransaction = IsNewTranscRequired(pTranscMgr, hReceivedMsg,&key, hLocalAddr,
                                               pRecvFrom, &addrOptions, hConn);
        if(bOpenTransaction == RV_FALSE)
        {
			rv = RV_OK;
#ifdef RV_SIGCOMP_ON
			/* checking if ACK */
			eMethodType = RvSipMsgGetRequestMethod(hReceivedMsg);			
			if (eMethodType == RVSIP_METHOD_ACK)
			{
				rv = HandleIncomingSigCompAckIfNeeded(pTranscMgr,hConn,pRecvFrom,hLocalAddr,hReceivedMsg,pSigCompMsgInfo);
			}
#endif
            return rv;
        }
        /*---------------------------------------------------
              Looking for a Transaction for the second time -
              after callbacks, called from IsNewTranscRequired()
        ----------------------------------------------------*/
        RvMutexLock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);

        rv = SipTransactionMgrFindTransaction(pTranscMgr, hReceivedMsg,&key, RV_FALSE,&pTransc);
        if (RV_OK != rv)
        {
            RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                "HandleIncomingRequestMsg - Failed when looking for transaction in hash"));
            RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
            return rv;
        }
    }

    /*-------------------------------------------------
        Transaction was not found for the second time
      -------------------------------------------------*/
    if(pTransc == NULL)
    {
        if(pTranscMgr->bDisableMerging == RV_FALSE)
        {
            RvBool bMerged = RV_FALSE;
            Transaction* pMergedTransc = NULL;
            /* check if this request was already received in a different transaction
            (because of request forking), and if so - reject it with 482 */
            rv = MergeRequestIfNeeded(pTranscMgr, hReceivedMsg, &key, pFailureResp, &bMerged, &pMergedTransc);
            if(rv != RV_OK)
            {
                RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                    "HandleIncomingRequestMsg - Failed to check merging case for message (rv=%d)", rv));
                RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
                *pFailureResp = 486;
                return rv;
            }
            if(bMerged == RV_TRUE)
            {
                RvLogInfo(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                    "HandleIncomingRequestMsg - merged! msg 0x%p is the same request as transc 0x%p. reject with %d",
                    hReceivedMsg, pMergedTransc, *pFailureResp));
                RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
                return RV_ERROR_OBJECT_ALREADY_EXISTS;
            }
        }
        rv =  CreateNewServerTransaction(pTranscMgr, hReceivedMsg,&key, &pTransc);
        if(rv != RV_OK)
        {
            RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                "HandleIncomingRequestMsg - Failed to create new transaction for message"));
            RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
            *pFailureResp = 486;
            return rv;
        }
        bOpenTransaction = RV_TRUE; /*mark that a transaction was created*/
        curTranscIdentifier = pTransc->transactionUniqueIdentifier;

        /* save the received from address */
        TransactionSaveReceivedFromAddr(pTransc,pRecvFrom);

        /*set local address in the transmitter so that the response will use it*/
        SipTransmitterSetLocalAddressHandle(pTransc->hTrx,hLocalAddr,RV_TRUE);
		
		/*In order to avoid other thread to find the newly created transaction after the 
		  manager is unlocked, we need to lock the switching section of the locks between the
		  transaction and its manager.*/
		RvMutexLock(&pTranscMgr->hSwitchSafeLock, pTranscMgr->pLogMgr);


		/*First we need to unlock the manager before locking the transaction, in order
		  to keep the locking hierarchy*/
        RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
		RvMutexLock(&pTransc->transactionTripleLock.hProcessLock,pTransc->pMgr->pLogMgr);

		/*We finished the switch in of the locks between the transaction and its manager
		  we can safely unlock the switching lock because the transaction is already locked*/
		RvMutexUnlock(&pTranscMgr->hSwitchSafeLock, pTranscMgr->pLogMgr);

        key.bIgnoreLocalAddr = RV_TRUE;

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
        {           
           TransportMgrLocalAddr *laddr = (TransportMgrLocalAddr*)hLocalAddr;
           
           TransportMgrSipAddrGetDetails(&(laddr->addr),RvSipTransactionGetLocalAddr((RvSipTranscHandle)pTransc));           
        }
#endif
//SPIRENT_END

        rv = UpdateServerTransactionOwner(pTranscMgr, pTransc, hReceivedMsg, &key,
            RV_FALSE, NULL);
        if(rv != RV_OK)
        {
            RvMutexUnlock(&pTransc->transactionTripleLock.hProcessLock,pTranscMgr->pLogMgr);
            RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                "HandleIncomingRequestMsg - Failed to update server transaction 0x%p owner",
                pTransc));
            pTransc = NULL;
            *pFailureResp = 486;
            return rv;
        }

#ifdef RV_SIP_IMS_ON 
        /* Attach to Security Object, if the request was received on secure socket */
        {
            void* pSecOwner;
            rv = RvSipTransportMgrLocalAddressGetSecOwner(hLocalAddr,&pSecOwner);
            if (RV_OK != rv)
            {
				/*The 486 response will be sent out of the transaction scope so we need to terminate the transc explicitly*/
                TransactionTerminate(RVSIP_TRANSC_REASON_ERROR, pTransc);
				RvMutexUnlock(&pTransc->transactionTripleLock.hProcessLock,pTranscMgr->pLogMgr);
                RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                    "HandleIncomingRequestMsg - Failed to get Security Owner for Local Address termination transc object 0x%p",
                    pTransc));				
				*pFailureResp = 486;
                return rv;
            }
            if (NULL != pSecOwner)
            {
                rv = TransactionSetSecObj(pTransc, (RvSipSecObjHandle )pSecOwner);
                if (RV_OK != rv)
                {
                    RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                        "HandleIncomingRequestMsg - Transaction 0x%p: Failed to set Security Object %p",
                        pTransc, pTransc->hSecObj));
                }
            }
        }
#endif /* #ifdef RV_SIP_IMS_ON */
        RvMutexUnlock(&pTransc->transactionTripleLock.hProcessLock,pTranscMgr->pLogMgr);
    }
    else /*if(pTransc == NULL)*/

    /*-------------------------------------------------
        Transaction was found
      -------------------------------------------------*/
    {
        RvLogInfo(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "HandleIncomingRequestMsg - Found existing transaction 0x%p for the request message",
            pTransc));
        curTranscIdentifier = pTransc->transactionUniqueIdentifier;
        RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
		
		/*In order to avoid other thread to find the newly created transaction and to start processing it
		  after the manager is unlocked, we need to lock the switching section of the locks between the
		  transaction and its manager .*/
		RvMutexLock(&pTranscMgr->hSwitchSafeLock, pTranscMgr->pLogMgr);
        RvMutexUnlock(&pTranscMgr->hSwitchSafeLock, pTranscMgr->pLogMgr);

        /*the following lock prevent handling a retransmission before
        the initial message was handled. In initial message handling we
        lock the processing lock before we have the actual locks of the
        transaction.*/
        RvMutexLock(&pTransc->transactionTripleLock.hProcessLock,pTranscMgr->pLogMgr);
        RvMutexUnlock(&pTransc->transactionTripleLock.hProcessLock,pTranscMgr->pLogMgr);
    }

    rv = TransactionLockMsg(pTransc);
    if (rv != RV_OK)
    {
        *pFailureResp = 486;
        return rv;
    }

    /* The manager has to be locked since one of the transaction hash */
    /* keys points to 'bAllowCancellation' field */
    RvMutexLock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    pTransc->bAllowCancellation = RV_TRUE;
    RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);

    if(pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED ||
       curTranscIdentifier != pTransc->transactionUniqueIdentifier)
    /* The call leg killed the transaction and it is in the queue */
    {
        TransactionUnLockMsg(pTransc);
        return RV_OK;
    }

    /*we now have a transaction (new or old), We need to update the connection*/
    rv = UpdateTranscConnection(pTransc,hConn,RV_TRUE);
    if(rv != RV_OK)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "HandleIncomingRequestMsg - Can't update transc 0x%p connection",pTransc));
        TransactionTerminate(RVSIP_TRANSC_REASON_ERROR, pTransc);
        TransactionUnLockMsg(pTransc);
        *pFailureResp = 486;
        return rv;
    }
    /*update information that was already updated in newly created transactions*/
    if(bOpenTransaction == RV_FALSE)
    {
        TransactionSaveReceivedFromAddr(pTransc,pRecvFrom);
    }


#ifdef RV_SIGCOMP_ON
    rv = TransactionHandleIncomingSigCompMsgIfNeeded(
                pTransc,hConn,pRecvFrom,hLocalAddr,hReceivedMsg,pSigCompMsgInfo);
    if (RV_OK != rv)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "HandleIncomingRequestMsg - Transaction 0x%p - Failed to handle SigComp transc",
            pTransc));
        *pFailureResp = 486;
        TransactionTerminate(RVSIP_TRANSC_REASON_ERROR, pTransc);
		TransactionUnLockMsg(pTransc);
		return rv;
    } 
#else
    RV_UNUSED_ARG(pSigCompMsgInfo);
#endif /* RV_SIGCOMP_ON */

    /*-----------------------------------------------------
    give the transaction the received message
    -----------------------------------------------------*/
    /* save received message handle*/
    TransactionSetReceivedMsg(pTransc, hReceivedMsg);
    rv = TransactionMsgReceived(pTransc, hReceivedMsg);
    if(rv != RV_OK)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "HandleIncomingRequestMsg - Transaction 0x%p - Failed to process the received message (rv=%d)",
			pTransc,rv));
        /* clear the received message handle */
        TransactionSetReceivedMsg(pTransc, NULL);
        TransactionTerminate(RVSIP_TRANSC_REASON_ERROR, pTransc);
        TransactionUnLockMsg(pTransc);
        *pFailureResp = 486;
        return rv;
    }
    /* clear the received message handle */
    TransactionSetReceivedMsg(pTransc, NULL);

     /* Unlock the transaction */
    TransactionUnLockMsg(pTransc);
    return rv;
}


/***************************************************************************
 * IsNewTranscRequired
 * ------------------------------------------------------------------------
 * General: Determines wether to open a new transaction when a transaction
 *          was not found for an incoming request
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr - the transaction mgr handle
 *          hReceivedMsg - the received message.
 *          pTranscKey - the transaction key
 *          hLocalAddr - indicates the local address on which the message
 *                       was received.
 *          pRcvdFromAddr - The address from where the message
 *                         was received.
 *          pOptions - Other identifiers of the message such as compression type.
 *          hConn      - If the message was received on a specific connection,
 *                       the connection handle is supplied. Otherwise this parameter
 *                       is set to NULL.
 ***************************************************************************/
static RvBool RVCALLCONV IsNewTranscRequired(
                            IN TransactionMgr*                pTranscMgr,
                            IN RvSipMsgHandle                 hReceivedMsg,
                            IN SipTransactionKey*             pTranscKey,
                            IN RvSipTransportLocalAddrHandle  hLocalAddr,
                            IN SipTransportAddress*           pRcvdFromAddr,
                            IN RvSipTransportAddrOptions*     pOptions,
                            IN RvSipTransportConnectionHandle hConn)
{

    RvSipMethodType eMethodType = RvSipMsgGetRequestMethod(hReceivedMsg);

    /*if a transaction was not found and this is ACK ignore the message*/
    if (eMethodType == RVSIP_METHOD_ACK)
    {
        RvBool bWasHandled = RV_FALSE;
        /*give the ack message to the call-leg layer*/
        TransactionCallbackMgrAckNoTranscEv(pTranscMgr,hReceivedMsg,pRcvdFromAddr,&bWasHandled);
        if(bWasHandled == RV_TRUE)
        {
            RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                      "IsNewTranscRequired - Received an ACK, no match INVITE - handled by the call-leg layer"));
            return RV_FALSE;  /*no need to open a new transaction*/

        }
        /*ACK was not handled by the call-leg layer. Give it to the application as followes:
          1. If the application uses the bDynamicInviteHandling then it is possible
             that the application handled the invite above the transaction layer and
             therefore it should receive the ACK
          2. If this is a proxy it should get all out of context ACK messages.*/
        if((pTranscMgr->bDynamicInviteHandling == RV_TRUE &&
            NULL != pTranscMgr->pMgrEvHandlers.pfnEvOutOfContextMsg) ||
            (pTranscMgr->isProxy &&
            (NULL != pTranscMgr->proxyEvHandlers.pfnEvOutOfContextMsg ||
             NULL != pTranscMgr->pMgrEvHandlers.pfnEvOutOfContextMsg)))
        {
            TransactionCallbackMgrOutOfContextMsgEv(pTranscMgr,
                                                    hReceivedMsg,
                                                    hLocalAddr,pRcvdFromAddr,
                                                    pOptions,hConn);
        }
        else
        {
            RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                      "IsNewTranscRequired - Received an ACK to non existing INVITE - message ignored"));
        }
        return RV_FALSE; /*no need to open a new transaction*/
    }

    /*in a proxy mode if we did not find a transaction to cancel we need to forward
      the cancel in an out of context manner without creating a cancel transaction*/
    if(pTranscMgr->isProxy == RV_TRUE && eMethodType == RVSIP_METHOD_CANCEL)
    {
        Transaction* pCancelledTransc;

        TransactionMgrHashFindCancelledTranscByMsg(pTranscMgr,
                                                   hReceivedMsg,
                                                   pTranscKey,
                                                   (RvSipTranscHandle*)&pCancelledTransc);
        if(pCancelledTransc == NULL)
        {
            if(NULL != pTranscMgr->pMgrEvHandlers.pfnEvOutOfContextMsg)
            {
                RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                          "IsNewTranscRequired - Received CANCEL and the cancelled transaction was not found - handle as out of context"));
                TransactionCallbackMgrOutOfContextMsgEv(pTranscMgr,
                                                       hReceivedMsg,
                                                       hLocalAddr,pRcvdFromAddr,
                                                       pOptions,hConn);
            }
              return RV_FALSE; /*no need to open a new transaction*/
        }
    }

    /*---------------------------------------------------------
       Ask the app if we should create a new transaction
    -----------------------------------------------------------*/
    if(pTranscMgr->proxyEvHandlers.pfnEvNewRequestRcvd != NULL ||
		pTranscMgr->pMgrEvHandlers.pfnEvNewRequestRcvd != NULL)
    {
        RvBool bCreateTransc = RV_TRUE;
        RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                  "IsNewTranscRequired - New request received - Asking application if a new transc should be created"));
       TransactionCallbackMgrNewRequestRcvdEv(pTranscMgr,
                                             hReceivedMsg,
                                             &bCreateTransc);
        if(bCreateTransc == RV_FALSE)
        {
            RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                      "IsNewTranscRequired - Stack will not open a new transaction.bCreateTransc=%d",
                      bCreateTransc));
            /*handle the message in an out of context way*/
            TransactionCallbackMgrOutOfContextMsgEv(pTranscMgr,
                                                   hReceivedMsg,
                                                   hLocalAddr,pRcvdFromAddr,
                                                   pOptions,hConn);
            return RV_FALSE;
        }
    }
    return RV_TRUE;
}

/***************************************************************************
 * HandleIncomingResponseMsg
 * ------------------------------------------------------------------------
 * General: Handles an incoming response message. look for a transaction and
 *          give it the message.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr - The transaction manager.
 *          hReceivedMsg - the received message.
 *          port - the port that the message was receive from
 *          pRecvFrom - address from which the message was received
 *          pSigCompInfo        - SigComp information related to the incoming
 *                                response.
 ***************************************************************************/
static RvStatus RVCALLCONV HandleIncomingResponseMsg(
                            IN    TransactionMgr                 *pTranscMgr,
                            IN    RvSipMsgHandle                  hReceivedMsg,
                            IN    RvSipTransportConnectionHandle  hConn,
                            IN    RvSipTransportLocalAddrHandle   hLocalAddr,
                            IN    SipTransportAddress            *pRecvFrom,
                            INOUT SipTransportSigCompMsgInfo     *pSigCompInfo)
{

    RvStatus          rv;
    SipTransactionKey key;
    Transaction       *pTransc;
    RvBool            bIsRespOnInvite;

    RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                "HandleIncomingResponseMsg - A response msg 0x%p- starting to process...",
                hReceivedMsg));

   /*------------------------------------
      Get the key from the massage
    --------------------------------------*/
    rv = SipTransactionMgrGetKeyFromMessage((RvSipTranscMgrHandle)pTranscMgr, hReceivedMsg, &key);
    if (RV_OK != rv)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "HandleIncomingResponseMsg - Failed. An illegal message was received by the transaction manager"));
        return RV_ERROR_BADPARAM;
    }

    /*------------------------------------------------
       Look for a transaction this request belongs to
      -------------------------------------------------*/
    rv = SipTransactionMgrFindTransaction(pTranscMgr, hReceivedMsg, &key, RV_TRUE,&pTransc);
    if (RV_OK != rv)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "HandleIncomingResponseMsg - Failed when looking for transaction in hash"));
        return rv;
    }

    bIsRespOnInvite = Is1xx2xxRespWithToTagOnInviteMsg(hReceivedMsg);
    if(pTransc != NULL)
    {
        /*we found a transaction but it might be a mistake in some cases*/
        RvBool bTranscValid = RV_TRUE;
        CheckFoundTranscValidity(pTransc,hReceivedMsg, bIsRespOnInvite,&bTranscValid);
        if(bTranscValid == RV_FALSE)
        {
            pTransc = NULL;
        }
    }

    if(pTransc == NULL)
    {
        RvSipTransportAddrOptions addrOptions;
        addrOptions.eCompression = RVSIP_COMP_UNDEFINED;
#ifdef RV_SIGCOMP_ON        
        if(pSigCompInfo != NULL && pSigCompInfo->bExpectComparmentID == RV_TRUE)
        {
            addrOptions.eCompression = RVSIP_COMP_SIGCOMP;
        }
#endif /* #ifdef RV_SIGCOMP_ON */ 
        NotifyAppOfResponseWithNoTransc(pTranscMgr, bIsRespOnInvite,hReceivedMsg,
                                        hLocalAddr, pRecvFrom, &addrOptions, hConn);

        return RV_OK;
    }
    /*here we have a transaction. lock it before processing*/
    rv = TransactionLockMsg(pTransc);
    if (RV_OK != rv)
    {
        if (RV_ERROR_DESTRUCTED == rv)
        {
            RvLogWarning(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                "HandleIncomingResponseMsg - Found Transaction 0x%p was destructed",pTransc));
        }
        else
        {
            RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                "HandleIncomingResponseMsg - Failed to lock found Transaction 0x%p",pTransc));
        }
        return rv;
    }
    
    TransactionSaveReceivedFromAddr(pTransc,pRecvFrom);

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    {       
       TransportMgrLocalAddr *laddr = (TransportMgrLocalAddr*)hLocalAddr;
       
       TransportMgrSipAddrGetDetails(&(laddr->addr),RvSipTransactionGetLocalAddr((RvSipTranscHandle)pTransc));       
    }
#endif
//SPIRENT_END
    
    rv = UpdateTranscConnection(pTransc, hConn,RV_FALSE);
    if(rv != RV_OK)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "HandleIncomingResponseMsg - Transaction=0x%p: Failed to update connection on incoming response, hConn=0x%p",pTransc,hConn));
        TransactionUnLockMsg(pTransc);
        return rv;
    }

#ifdef RV_SIGCOMP_ON
    rv = TransactionHandleIncomingSigCompMsgIfNeeded(
                pTransc,hConn,pRecvFrom,hLocalAddr,hReceivedMsg,pSigCompInfo);
    if (RV_OK != rv)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "HandleIncomingResponseMsg - Transaction 0x%p - Failed to handle SigComp transc (rv=%d)",
            pTransc,rv));
        TransactionUnLockMsg(pTransc);
        return rv;
    }
#else
    RV_UNUSED_ARG(pSigCompInfo);
#endif /* RV_SIGCOMP_ON */

    /*-----------------------------------------------------
        give the transaction the received message
     -----------------------------------------------------*/
    /* save received message handle*/
    TransactionSetReceivedMsg(pTransc, hReceivedMsg);
    rv = TransactionMsgReceived(pTransc, hReceivedMsg);
    if(rv != RV_OK)
    {
        /* Clear the received message handle*/
        TransactionSetReceivedMsg(pTransc, NULL);
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                       "HandleIncomingResponseMsg - Transaction 0x%p - Failed to process the received message",pTransc));
        TransactionUnLockMsg(pTransc);
        return rv;

    }
    /* Clear the received message handle*/
    TransactionSetReceivedMsg(pTransc, NULL);

    TransactionUnLockMsg(pTransc);


    return RV_OK;
}


/***************************************************************************
 * SipTransactionMgrGetKeyFromMessage
 * ------------------------------------------------------------------------
 * General: Gets the To, From, Call-Id and CSeq-Step values from the received
 *          message
 * Return Value: RV_ERROR_BADPARAM - The message is missing one of these
 *                                     values.
 *                 RV_OK - All the values were updated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr - The transaction manager.
 *          hMsg - the received message.
 * Output:    key - a structure with the key taken from the message.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrGetKeyFromMessage(
                                    IN  RvSipTranscMgrHandle    hTranscMgr,
                                    IN  RvSipMsgHandle          hMsg,
                                    OUT SipTransactionKey      *pKey)
{
    RvSipCSeqHeaderHandle hCSeqHeader;
    TransactionMgr         *pTranscMgr = (TransactionMgr*)hTranscMgr;
	RvStatus				rv;

    /*To prevent compilation warning on Nucleus, while compiling without logs*/
    RV_UNUSED_ARG(pTranscMgr);

    pKey->hFromHeader = RvSipMsgGetFromHeader(hMsg);
    pKey->hToHeader = RvSipMsgGetToHeader(hMsg);
    pKey->strCallId.offset = SipMsgGetCallIdHeader(hMsg);
    pKey->strCallId.hPage = SipMsgGetPage(hMsg);
    pKey->strCallId.hPool = SipMsgGetPool(hMsg);

    hCSeqHeader = RvSipMsgGetCSeqHeader(hMsg);

    /* Error if At least one of the message values is invalid */
    if ((NULL == pKey->hFromHeader)           ||
        (NULL == pKey->hToHeader)             ||
        (UNDEFINED == pKey->strCallId.offset) ||
        (NULL_PAGE == pKey->strCallId.hPage)  ||
        (NULL == pKey->strCallId.hPool)       ||
        (NULL == hCSeqHeader))
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                 "SipTransactionMgrGetKeyFromMessage - An illegal message was received - A Key field is missing"));
        return RV_ERROR_BADPARAM;
    }

    if((SipPartyHeaderGetStrBadSyntax(pKey->hFromHeader) > UNDEFINED)||
       (SipPartyHeaderGetStrBadSyntax(pKey->hToHeader) > UNDEFINED) ||
       (SipCSeqHeaderGetStrBadSyntax(hCSeqHeader) > UNDEFINED))
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                 "SipTransactionMgrGetKeyFromMessage - An illegal message was received - A Key field with syntax error"));
        return RV_ERROR_BADPARAM;
    }

	/* Check that the CSeq-Step is valid */
	rv = SipCSeqHeaderGetInitializedCSeq(hCSeqHeader,&pKey->cseqStep); 
	if (RV_OK != rv)
	{
		RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
			"SipTransactionMgrGetKeyFromMessage: TranscMgr 0x%p - SipCSeqHeaderGetInitializedCSeq() failed (CSeq might be uninitialized)",
			pTranscMgr));
		return rv;
	}

    pKey->bIgnoreLocalAddr = RV_TRUE; /*the key does no include local addr*/
    return RV_OK;
}
/***************************************************************************
 * SipTransactionMgrFindTransaction
 * ------------------------------------------------------------------------
 * General: Finds a transaction according to the received key. If the transaction
 *          existes give the message to the transaction.
 * Return Value: RV_OK - The transaction was found and the message was
 *                            given successfully.
 *                 RV_ERROR_OUTOFRESOURCES - Not enough resources.
 *                 RV_ERROR_NULLPTR - Bad parameters.
 *               RV_ERROR_NOT_FOUND - The transaction was not found.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr - The transaction manager.
 *          hReceivedMsg - the received message.
 *          pKey - The transaction key taken from the message
 *          bIsUac - is the transaction should be client or server.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrFindTransaction(
                                    IN TransactionMgr        *pTranscMgr,
                                    IN RvSipMsgHandle         hReceivedMsg,
                                    IN SipTransactionKey     *pKey,
                                    IN RvBool                bIsUac,
                                    OUT Transaction           **pTransc)
{
    RvStatus             retStatus;
    TransactionMgrHashKey hashKey;
    RvSipCSeqHeaderHandle hCSeqHeader;
    RvChar               strMethod[33];
    RvUint               size;
    RvSipMethodType       eMethod;
    RvSipTranscHandle     hTransc;
    RvSipHeaderListElemHandle hPos;
    RvSipViaHeaderHandle    hTopVia;

    if ((NULL == pTranscMgr) ||
        (NULL == hReceivedMsg))
    {
        return RV_ERROR_NULLPTR;
    }
    /* Build the transaction hash key */
    hashKey.bIsUac             = bIsUac;
    if(bIsUac == RV_TRUE)
    {   /* If we are here and the transaction is client transaction, we received a response */
        hashKey.bSendRequest       = RV_FALSE;
    }
    hashKey.cseqStep           = pKey->cseqStep;
    hCSeqHeader                = RvSipMsgGetCSeqHeader(hReceivedMsg);
    eMethod                    = RvSipCSeqHeaderGetMethodType(hCSeqHeader);
    hashKey.strMethod          = NULL;
    switch (eMethod)
    {
    case RVSIP_METHOD_INVITE:
    case RVSIP_METHOD_ACK:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_INVITE;
        break;
    case RVSIP_METHOD_BYE:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_BYE;
        break;
    case RVSIP_METHOD_REGISTER:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_REGISTER;
        break;
#ifdef RV_SIP_SUBS_ON
    case RVSIP_METHOD_REFER:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_REFER;
        break;
    case RVSIP_METHOD_SUBSCRIBE:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_SUBSCRIBE;
        break;
    case RVSIP_METHOD_NOTIFY:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_NOTIFY;
        break;
#endif /* #ifdef RV_SIP_SUBS_ON */
#ifndef RV_SIP_PRIMITIVES
    case RVSIP_METHOD_PRACK:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_PRACK;
        break;
#endif
    case RVSIP_METHOD_CANCEL:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_CANCEL;
        break;

    case RVSIP_METHOD_OTHER:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_OTHER;
        retStatus = RvSipCSeqHeaderGetStrMethodType(
                                        hCSeqHeader, strMethod, 32, &size);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        hashKey.strMethod = strMethod;
        break;
    default:
        hashKey.eMethod = SIP_TRANSACTION_METHOD_UNDEFINED;
    }
    hashKey.hFrom           = &(pKey->hFromHeader);
    hashKey.hTo             = &(pKey->hToHeader);
    hashKey.pCallId         = &(pKey->strCallId);
    hashKey.hRequestUri     = RvSipMsgGetRequestUri(hReceivedMsg);
    hashKey.pMgr            = pTranscMgr;
    hTopVia                 = (RvSipViaHeaderHandle)RvSipMsgGetHeaderByTypeExt(
                                                hReceivedMsg,
                                                RVSIP_HEADERTYPE_VIA,
                                                RVSIP_FIRST_HEADER,
                                                RVSIP_MSG_HEADERS_OPTION_ALL,
                                                &hPos);
    hashKey.hTopVia         = &hTopVia;

    if(hTopVia == NULL)
    {
        RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
            "SipTransactionMgrFindTransaction: top via is missing. essential key parameter"));
        return RV_ERROR_BADPARAM;
    }
    else if(SipViaHeaderGetStrBadSyntax(hTopVia) > UNDEFINED)
    {
        RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
            "SipTransactionMgrFindTransaction: top via has syntax error. illegal key parameter"));
        return RV_ERROR_BADPARAM;
    }

    /* Search the transactions hash for a transaction that match the received key. */
    TransactionMgrHashFind(pTranscMgr, &hashKey, &hTransc, eMethod);
    *pTransc = (Transaction*)hTransc;
    return RV_OK;
}


/***************************************************************************
* CreateNewServerTransaction
* ------------------------------------------------------------------------
* General: If the received message is a request other than Ack message,
*          create a new transaction and give this transaction the message.
* Return Value: RV_OK - No new transaction needs to be created, or a
*                            new transaction was successfully created.
*                 RV_ERROR_OUTOFRESOURCES - Not enough memory available.
*                 RV_ERROR_BADPARAM - illegal message received.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTranscMgr - The transactions manager.
*          hReceivedMsg - The message received.
*          pKey - The transaction key taken from the message
* Output:  (-)
***************************************************************************/
static RvStatus RVCALLCONV CreateNewServerTransaction(
                                                       IN TransactionMgr        *pTranscMgr,
                                                       IN RvSipMsgHandle         hReceivedMsg,
                                                       IN SipTransactionKey     *pKey,
                                                       OUT Transaction          **ppTransc)
{
    RvSipMethodType       eMethodType;
    RvStatus             retStatus;
    RvSipTranscHandle     hTransc;
    Transaction           *pTransc;
    RvSipAddressHandle    hRequestUri;


    *ppTransc = NULL;

    eMethodType = RvSipMsgGetRequestMethod(hReceivedMsg);

    /*---------------------------------------------------
    create new transaction
    ----------------------------------------------------*/
    /* Create a new transaction */
    retStatus = TransactionCreate((RvSipTranscMgrHandle)pTranscMgr, pKey,
                                   NULL, SIP_TRANSACTION_OWNER_NON, NULL, RV_FALSE,
                                   &hTransc);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }

    pTransc = (Transaction *)hTransc;



    /*set the request uri in the new transaction*/
    hRequestUri = RvSipMsgGetRequestUri(hReceivedMsg);
    if(hRequestUri == NULL)
    {
        TransactionDestruct(pTransc);
        return RV_ERROR_UNKNOWN;
    }

    retStatus = TransactionSetRequestUri(pTransc,hRequestUri);
    if(retStatus != RV_OK)
    {
        TransactionDestruct(pTransc);
        return retStatus;
    }

    /* Set the transaction's method to the received method */
    TransactionSetMethod(hTransc, eMethodType);
    if (RVSIP_METHOD_OTHER == eMethodType)
    {
        TransactionSetMethodStrFromMsg(hTransc, hReceivedMsg);
    }

    /* Set the transaction manager to the transaction */
    pTransc->pMgr = pTranscMgr;

    /* Copy the list of Via headers from the message to the transaction */
    retStatus = TransactionUpDateViaList(pTransc, hReceivedMsg);
    if (RV_OK != retStatus)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "CreateNewServerTransaction - Transaction 0x%p: Error in trying to create a new Via list.",
                  pTransc));
        TransactionDestruct(pTransc);
        return retStatus;
    }

    /* Insert the transaction handle to the hash table */
    retStatus = TransactionMgrHashInsert(hTransc);
    if(RV_OK != retStatus)
    {
        TransactionDestruct(pTransc);
        return retStatus;
    }

    /* set the msg tags in transaction now, so in any default response, it will be set to
       the response message */
    retStatus = UpdateTranscSavedMsgToAndFrom(pTransc, hReceivedMsg);
    if(RV_OK != retStatus)
    {
        TransactionDestruct(pTransc);
        return retStatus;
    }
    *ppTransc = pTransc;
    return RV_OK;
}


/***************************************************************************
* UpdateServerTransactionOwner
* ------------------------------------------------------------------------
* General: Update the transaction owner and call event transaction created on
*          the owner.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTranscMgr - The transactions manager.
*          hReceivedMsg - The message received.
*          bIsApplicationMsg - RV_TRUE if the transaction is a transaction that the
*                             application creates from an out of context message.
*          hTranscOwner - The application handle of the transaction. This parameter
*                         is read only if isApplicationMsg is RV_TRUE
*          pKey - The transaction key taken from the message
* Output:  (-)
***************************************************************************/
static RvStatus RVCALLCONV UpdateServerTransactionOwner(
                                                       IN TransactionMgr        *pTranscMgr,
                                                       IN Transaction           *pTransc,
                                                       IN RvSipMsgHandle         hReceivedMsg,
                                                       IN SipTransactionKey     *pKey,
                                                       IN RvBool                bIsApplicationMsg,
                                                       IN RvSipTranscOwnerHandle hTranscOwner)
{
    void                  *pOwner      = NULL;
    SipTripleLock          *tripleLock  = NULL;
    RvSipMethodType       eMethodType;
    RvStatus             retStatus;
    RvBool               b_handleTransc = RV_FALSE;
#ifndef RV_SIP_PRIMITIVES
    RvBool               bNotifyCallLeg = RV_TRUE;
#endif /*#ifndef RV_SIP_PRIMITIVES*/
    RvSipTranscHandle     hTransc = (RvSipTranscHandle)pTransc;
    RvUint16             statusCode = 0;
    RvInt32              curTranscIdentifier = pTransc->transactionUniqueIdentifier;

    eMethodType = RvSipMsgGetRequestMethod(hReceivedMsg);

    if(bIsApplicationMsg == RV_FALSE) /* message arrived from the network */
    {

        /* if the transaction manager configure as a proxy and the request is an initial
           request ask the application if the stack need to open a call-leg for it. */
        TransactionSetReceivedMsg(pTransc,hReceivedMsg);

#ifndef RV_SIP_PRIMITIVES
        /* if this is a CANCEL transaction, first we will find the cancelled transaction,
        and use it's owner as the owner of this cancel transaction too */

        if(RV_TRUE == pTranscMgr->bEnableNestedInitialRequestHandling &&
           RVSIP_METHOD_CANCEL == eMethodType)
        {
            UpdateCancelTransactionOwner(pTransc, pKey, &pOwner);
        }
        IsCallLegNotificationRequired(pTransc,&bNotifyCallLeg);

        if(RV_TRUE == bNotifyCallLeg)
        {

            TransactionCallbackCallTranscCreatedEv(
                                 pTranscMgr,
                                 hTransc, pKey,
                                 &pOwner, &tripleLock, &statusCode);

        }
#endif /* RV_SIP_PRIMITIVES */

        TransactionSetReceivedMsg(pTransc,NULL);
        if(statusCode > 0)
        {
            /* call-leg manager decided to reject this response, without opening
            an object */
            RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                "UpdateServerTransactionOwner: transaction 0x%p - set %d response, Call-leg manager order",
                pTransc, statusCode));
            pTransc->responseCode = statusCode;
        }

        /* Attach the transaction owner (and the lock and key associated with this owner).
        to the transaction. If the transaction has no call owner, NULL is returned
        by this callback, and no attachment is made */
        if(pOwner != NULL && tripleLock != NULL)
        {
            TransactionSetTripleLock(tripleLock,pTransc);

            retStatus = TransactionLockMsg(pTransc);
            if (RV_OK != retStatus)
            {
                RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                          "UpdateServerTransactionOwner: transaction 0x%p couldn't be locked (rv=%d)",
                           pTransc, retStatus));
                TransactionTerminate(RVSIP_TRANSC_REASON_ERROR, pTransc);
                return retStatus;
            }

            if (pTransc->transactionUniqueIdentifier != curTranscIdentifier)
            {
                RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                    "UpdateServerTransactionOwner: transaction 0x%p identifier discrepancy: 0x%X!=0x%X",
                    pTransc, pTransc->transactionUniqueIdentifier,curTranscIdentifier));
                TransactionUnLockMsg(pTransc);
                return RV_ERROR_DESTRUCTED;
            }
            if (pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
            {
                RvLogWarning(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                    "UpdateServerTransactionOwner: transaction 0x%p is terminated (transc state = %s)",
                    pTransc, TransactionGetStateName(pTransc->eState)));
                TransactionUnLockMsg(pTransc);
                return RV_ERROR_DESTRUCTED;
            }

            retStatus = TransactionAttachOwner(hTransc, pOwner, pKey,
                tripleLock,
                SIP_TRANSACTION_OWNER_CALL,
                RV_FALSE);

            /*not a proxy transaction*/
            pTransc->bIsProxy = RV_FALSE;
            if (RV_OK != retStatus)
            {
                RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                    "UpdateServerTransactionOwner: transaction 0x%p - Failed to attach owner, terminate transaction",
                    pTransc));
                TransactionTerminate(RVSIP_TRANSC_REASON_ERROR, pTransc);
                TransactionUnLockMsg(pTransc);
                return retStatus;
            }
            AttachServerCancelToServerInvite(pTransc);
            TransactionUnLockMsg(pTransc);
        }
        else
        {
            AttachServerCancelToServerInvite(pTransc);

            /*------------------------------------------------------
            give the application a chance to handle the transaction if
            this is not BYE/INVITE. proxy will get all the transactions.
            if statusCode != 0, the call-leg manager had already decided
            to reject the request,
            ---------------------------------------------------------*/
            if ((NULL != pTranscMgr->pAppEvHandlers.pfnEvTransactionCreated &&
                eMethodType != RVSIP_METHOD_BYE &&
                eMethodType != RVSIP_METHOD_INVITE &&
                0 == statusCode)
                || (NULL != pTranscMgr->pAppEvHandlers.pfnEvTransactionCreated &&
                    pTranscMgr->isProxy == RV_TRUE &&
                    0 == statusCode)
                   || (pTranscMgr->bDynamicInviteHandling == RV_TRUE &&
                    0 == statusCode)
                )

            {
               /* Update the transaction key values before calling the created event so
                that the application can explore the transaction key*/
                TransactionAttachKey(pTransc,pKey);
                TransactionSetReceivedMsg(pTransc,hReceivedMsg);
                pTransc->pMgr = pTranscMgr;

                TransactionCallbackAppTranscCreatedEv(pTransc,
                                                      (RvSipTranscOwnerHandle*)&pOwner,
                                                       &b_handleTransc);

                TransactionSetReceivedMsg(pTransc,NULL);

                /*If the application wishes to handle the transaction Attach the application
                as the transaction owner*/
                if(b_handleTransc == RV_TRUE)
                {
                    pTransc->bIsProxy = pTranscMgr->isProxy;
                    retStatus = TransactionAttachApplicationOwner(hTransc, pKey,pOwner);
                    if (RV_OK != retStatus)
                    {
                        RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                            "UpdateServerTransactionOwner: transaction 0x%p - Failed to attach app owner, terminate transaction",
                            pTransc));
                        TransactionTerminate(RVSIP_TRANSC_REASON_ERROR, pTransc);
                        return retStatus;
                    }
                }
                else /* There is no owner for the transaction */
                {
                    retStatus = TransactionNoOwner((Transaction *)hTransc);
                    if (RV_OK != retStatus)
                    {
                        TransactionTerminate(RVSIP_TRANSC_REASON_ERROR, pTransc);
                        return retStatus;
                    }
                }
            }
            else
            {
                /*------------------------------------
                 There is no owner for the transaction
                -------------------------------------*/
                retStatus = TransactionNoOwner((Transaction *)hTransc);
                if (RV_OK != retStatus)
                {
                    RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                        "UpdateServerTransactionOwner: transaction 0x%p - Failed to attach no owner, terminate transaction",
                        pTransc));
                    TransactionTerminate(RVSIP_TRANSC_REASON_ERROR, pTransc);
                    return retStatus;
                }
            }
        }
    }
    else /* message arrived from the application */
    {
            retStatus = TransactionAttachApplicationOwner(hTransc, pKey,(void *)hTranscOwner);
            if (RV_OK != retStatus)
            {
                RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                    "UpdateServerTransactionOwner: transaction 0x%p - Failed to attach app owner, terminate transaction",
                    pTransc));

                TransactionDestruct(pTransc);
                return retStatus;
            }
    }

    return RV_OK;
}

/***************************************************************************
 * SipTransactionMgrGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: Get the status of the transaction manager's resources. The
 *          transaction manager's resources are the transaction's pool list
 *          and the Via headers' pool list.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTranscMgr - The transaction manager.
 * Output:  pResources - The transaction's resources structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrGetResourcesStatus (
                                 IN  RvSipTranscMgrHandle  hTranscMgr,
                                 OUT RvSipTranscResources *pResources)
{

    TransactionMgr *pTranscMgr = (TransactionMgr *)hTranscMgr;

    RvUint32 allocNumOfLists;
    RvUint32 allocMaxNumOfUserElement;
    RvUint32 currNumOfUsedLists;
    RvUint32 currNumOfUsedUserElement;
    RvUint32 maxUsageInLists;
    RvUint32 maxUsageInUserElement;

    if(pTranscMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pResources == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    memset(pResources,0,sizeof(RvSipTranscResources));
    /* transaction list */
    RLIST_GetResourcesStatus(pTranscMgr->hTranasactionsListPool,
                             &allocNumOfLists, /*allways 1*/
                             &allocMaxNumOfUserElement,
                             &currNumOfUsedLists,/*allways 1*/
                             &currNumOfUsedUserElement,
                             &maxUsageInLists,
                             &maxUsageInUserElement);

    pResources->transactions.currNumOfUsedElements = currNumOfUsedUserElement;
    pResources->transactions.numOfAllocatedElements = allocMaxNumOfUserElement;
    pResources->transactions.maxUsageOfElements = maxUsageInUserElement;

    return RV_OK;
}

/***************************************************************************
* SipTransactionMgrResetMaxUsageResourcesStatus
* ------------------------------------------------------------------------
* General: Reset the counter that counts the max number of transactions that
*          were used ( at one time ) until the call to this routine.
* ------------------------------------------------------------------------
* Arguments:
* Input:    hTranscMgr - The transaction manager.
***************************************************************************/
void RVCALLCONV SipTransactionMgrResetMaxUsageResourcesStatus (
                                                               IN  RvSipTranscMgrHandle  hTranscMgr)
{
    TransactionMgr *pTranscMgr = (TransactionMgr *)hTranscMgr;

    RLIST_ResetMaxUsageResourcesStatus(pTranscMgr->hTranasactionsListPool);
}
#ifdef SIP_DEBUG
/***************************************************************************
 * SipTransactionMgrGetStatistics
 * ------------------------------------------------------------------------
 * General: Get the statistics of received and sent messages. This statistic
 *          is managed by the transaction manager.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTranscMgr - The transaction manager.
 * Output:  pStatistics - The transaction's manager statistics structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrGetStatistics(
                                 IN  RvSipTranscMgrHandle         hTranscMgr,
                                 OUT SipTransactionMgrStatistics *pStatistics)
{

    TransactionMgr *pTranscMgr = (TransactionMgr *)hTranscMgr;

    if(pTranscMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pStatistics == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *pStatistics = pTranscMgr->msgStatistics;

    return RV_OK;
}
#endif

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


#ifndef RV_SIP_PRIMITIVES

/***************************************************************************
 * IsInitialRequest
 * ------------------------------------------------------------------------
 * General: Returning an Indication whether the transaction is an initial
 *          request transaction.
 *          Initial request = The transaction has no to tag and the request
 *                              method is INVITE or REFFER.
 * Return Value: RV_TRUE if the transaction is an initial request transaction
 *               and RV_FALSE if not.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction.
 ***************************************************************************/
static RvBool IsInitialRequest(IN Transaction* pTransc)
{
    if (UNDEFINED != SipPartyHeaderGetTag(pTransc->hToHeader))
    {
        return RV_FALSE;
    }
    switch (pTransc->eMethod)
    {
    case SIP_TRANSACTION_METHOD_INVITE:
#ifdef RV_SIP_SUBS_ON
    case SIP_TRANSACTION_METHOD_REFER:
    case SIP_TRANSACTION_METHOD_SUBSCRIBE:
#endif /* #ifdef RV_SIP_SUBS_ON */
        return RV_TRUE;
    default:
        return RV_FALSE;
    }
}

/***************************************************************************
 * IsCallLegNotificationRequired
 * ------------------------------------------------------------------------
 * General: determine whether to notify the call leg manager of the new
 *          server transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction.
 * Output: bNotifyMgr - RV_TRUE if we should notify the call-leg manager
 *                      of the new transaction. RV_FALSE otherwise.
 * Output:(-)
 ***************************************************************************/
static RvStatus RVCALLCONV IsCallLegNotificationRequired(IN Transaction* pTransc,
                                                          OUT RvBool   *bNotifyMgr)
{
/*in extra lean we always work with call-legs*/
    RvBool bIsInitialRequest = RV_FALSE;

    bIsInitialRequest = IsInitialRequest(pTransc);
    /*for proxy application the default is NOT to open a call-leg, for other apps
      the default is to use the call-leg manager*/

    /*the application did not register to this callback*/
    if(NULL == pTransc->pMgr->pAppEvHandlers.pfnEvOpenCallLeg)
    {
        if(pTransc->pMgr->isProxy == RV_TRUE)
        {
            *bNotifyMgr = RV_FALSE;
        }
#ifdef RV_SIP_SUBS_ON
        else if(bIsInitialRequest == RV_TRUE &&
                (pTransc->eMethod == SIP_TRANSACTION_METHOD_SUBSCRIBE ||
                 pTransc->eMethod == SIP_TRANSACTION_METHOD_REFER) &&
                pTransc->pMgr->maxSubscriptions == 0)
        {
            *bNotifyMgr = RV_FALSE;
        }
#endif /* #ifdef RV_SIP_SUBS_ON */
    }
    else /*the application registered to the callback*/
    {
        if(pTransc->pMgr->isProxy == RV_TRUE)
        {
            if(bIsInitialRequest)
            {
                TransactionCallbackOpenCallLegEv(pTransc,bNotifyMgr);

            }
        }
        else /*not a proxy - registered to the callback*/
        {
            if(bIsInitialRequest == RV_TRUE)
            {
                switch (pTransc->eMethod)
                {
#ifdef RV_SIP_SUBS_ON
                case SIP_TRANSACTION_METHOD_SUBSCRIBE:
                case SIP_TRANSACTION_METHOD_REFER:
                    if (pTransc->pMgr->maxSubscriptions > 0)
                    {
                        TransactionCallbackOpenCallLegEv(pTransc,bNotifyMgr);
                    }
                    else
                    {
                        *bNotifyMgr = RV_FALSE;
                    }
                    break;
#endif /* #ifdef RV_SIP_SUBS_ON */
                case SIP_TRANSACTION_METHOD_INVITE:
                    if (pTransc->pMgr->bDynamicInviteHandling == RV_TRUE)
                    {
                        TransactionCallbackOpenCallLegEv(pTransc,bNotifyMgr);
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }



    return RV_OK;
}
#endif /* RV_SIP_PRIMITIVES */
/***************************************************************************
 * AttachServerCancelToServerInvite
 * ------------------------------------------------------------------------
 * General: Checks whether the supplied transaction is cancel and if so
 *          locate its invite and and apply the invite looks to the cancel.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction.
 ***************************************************************************/
static void RVCALLCONV AttachServerCancelToServerInvite
                                         (IN Transaction* pTransc)
{
    Transaction* pCancelledTransc = NULL;
    RvStatus rv = RV_OK;
    /*don't handle non invite transactions*/
    if(pTransc->eMethod != SIP_TRANSACTION_METHOD_CANCEL)
    {
        return;
    }
    /*The cancel should be IDLe - this will always be the case*/
    if(pTransc->eState != RVSIP_TRANSC_STATE_IDLE)
    {
        return;
    }
    /*If we reached this point then we have an initial cancel request*/
    /*find the transaction this transaction is cancelling*/
    TransactionMgrHashFindCancelledTransc(pTransc->pMgr,
                                       (RvSipTranscHandle)pTransc,
                                       (RvSipTranscHandle*)&pCancelledTransc);

    /*lock the transaction*/
    if(pCancelledTransc != NULL)
    {
        rv = TransactionLockMsg(pCancelledTransc);
        if (rv != RV_OK)
        {
            pCancelledTransc = NULL;
        }
        /*see if the transaction is still in the hash - the transaction can be
          out of the hash if other thread terminated it before we got the chance to
          lock it.*/
        if(pCancelledTransc != NULL)
        {
            Transaction*    pSecondCheckTransc = NULL;
            TransactionMgrHashFindCancelledTransc(pTransc->pMgr,
                                                  (RvSipTranscHandle)pTransc,
                                                 (RvSipTranscHandle*)&pSecondCheckTransc);

            if(pSecondCheckTransc == NULL)
            {
                TransactionUnLockMsg(pCancelledTransc);
                pCancelledTransc = NULL;
            }
        }
    }

    if(pCancelledTransc != NULL) /*then we have a locked cancelled transaction*/
    {
        /*if one of the transactions belong to a call-leg and the other is not
          then there is no match*/
        if((pTransc->eOwnerType == SIP_TRANSACTION_OWNER_CALL &&
            pCancelledTransc->eOwnerType != SIP_TRANSACTION_OWNER_CALL) ||
           (pTransc->eOwnerType != SIP_TRANSACTION_OWNER_CALL &&
           pCancelledTransc->eOwnerType == SIP_TRANSACTION_OWNER_CALL) )
        {
            TransactionUnLockMsg(pCancelledTransc);
            return;
        }
        /*share the locks of the transactions and update the
          relationship between the transactions*/
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "AttachServerCancelToServerInvite - Cancel Transaction 0x%p: Found cancelled transaction 0x%p",
                   pTransc,pCancelledTransc));

        pCancelledTransc->hCancelTranscPair = (RvSipTranscHandle)pTransc;
        pTransc->hCancelTranscPair          = (RvSipTranscHandle)pCancelledTransc;
        TransactionSetTripleLock(pCancelledTransc->tripleLock,pTransc);
        TransactionUnLockMsg(pCancelledTransc);

    }

}

/***************************************************************************
 * HandleViaHeaderParameters
 * ------------------------------------------------------------------------
 * General: Handle the top Via header parameters according to the transport
 *          information received about the incoming message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pMgr            - The transaction manager.
 *         hMsg            - received request message.
 *         pRecvFrom       - The received from address.
 ***************************************************************************/
static void RVCALLCONV HandleViaHeaderParameters(
                               IN TransactionMgr             *pMgr,
                               IN RvSipMsgHandle              hMsg,
                               IN SipTransportAddress        *pRecvFrom)
{
    RvStatus rv;

    /* Add received parameter to the Via header before processing the request */
    rv = AddReceivedParameter(pMgr, hMsg, pRecvFrom);
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "HandleViaHeaderParameters - Failed to add received parameter in msg 0x%p - continue",
            hMsg));
    }
    /* Add rport parameter to the Via header before rejecting the request */
    rv = AddRportParameter(hMsg, pRecvFrom);
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "HandleViaHeaderParameters - Failed to add rport parameter in msg 0x%p - continue",
            hMsg));
    }
}

/***************************************************************************
 * AddReceivedParameter
 * ------------------------------------------------------------------------
 * General: Add received parameter to Via header of incoming request.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pMgr      - The transaction manager.
 *         hMsg      - received request message.
 *         pRecvFrom - The received from address.
 ***************************************************************************/
static RvStatus RVCALLCONV AddReceivedParameter(
                                             IN TransactionMgr*       pMgr,
                                             IN RvSipMsgHandle        hMsg,
                                             IN SipTransportAddress*  pRecvFrom)
{
    RvChar                     strViaAddr[128];
    RvSipHeaderListElemHandle  hListElem;
    RvSipViaHeaderHandle       hViaHeader;
    RvSipMethodType            eMethodType;
    RvUint                     viaAddrLen = 0;
    RvStatus                   rv;
    RvChar                     strReceivedAddr[RVSIP_TRANSPORT_LEN_STRING_IP];
    RvInt                      receivedAddrType;
    RvChar*                    pStrViaAddr;
    RvChar*                    pStrReceivedAddr;
#ifdef TRANSPORT_SCTP_MIXEDIPV4IPV6_ON
    RvChar*                    strIPv6Prefix = "::ffff:";
    RvSize_t                   lenIPv6Prefix = strlen("::ffff:");
#endif /*#ifdef TRANSPORT_SCTP_MIXEDIPV4IPV6_ON*/

    eMethodType = RvSipMsgGetRequestMethod(hMsg);
    if (RVSIP_METHOD_ACK == eMethodType)
    {
        return RV_OK;
    }
    receivedAddrType = RvAddressGetType(&pRecvFrom->addr);
    if(RV_ADDRESS_TYPE_NONE == receivedAddrType)
    {
        return RV_OK;
    }
    if (RV_ADDRESS_TYPE_IPV4 == RvAddressGetType(&pRecvFrom->addr) &&
        0 == RvAddressIpv4GetIp(RvAddressGetIpv4(&pRecvFrom->addr)) )
    {
        return RV_OK;
    }

    /*we need to add a receive only if the via host is a name or if it is an IP
     that is different from the actual received address*/

    hViaHeader = RvSipMsgGetHeaderByTypeExt(hMsg, RVSIP_HEADERTYPE_VIA,
                                         RVSIP_FIRST_HEADER, RVSIP_MSG_HEADERS_OPTION_ALL,
                                         &hListElem);
    if (NULL == hViaHeader)
    {
        return RV_ERROR_UNKNOWN;
    }

    RvAddressGetString(&pRecvFrom->addr,RVSIP_TRANSPORT_LEN_STRING_IP,strReceivedAddr);

    /* Extract host name from the VIA header */
    viaAddrLen = RvSipViaHeaderGetStringLength(hViaHeader, RVSIP_VIA_HOST);
    /* If host name can't be extracted - add RECEIVED parameter */
    if (0 == viaAddrLen || viaAddrLen >= 128)
    {
        rv = RvSipViaHeaderSetReceivedParam(hViaHeader, strReceivedAddr);
        return rv;
    }
    rv = RvSipViaHeaderGetHost(hViaHeader, strViaAddr, viaAddrLen, &viaAddrLen);
    /* If host name can't be extracted - add RECEIVED parameter */
    if (RV_OK != rv)
    {
        rv = RvSipViaHeaderSetReceivedParam(hViaHeader, strReceivedAddr);
        return rv;
    }

    /* Prepare RECEIVED FROM address and VIA address strings for comparison */
    pStrViaAddr = strViaAddr;
    pStrReceivedAddr = strReceivedAddr;
#if(RV_NET_TYPE & RV_NET_IPV6)
    if (strViaAddr[0] == '[')
    {
        RvChar *pStrEndOfAddr;
        pStrViaAddr++;
        pStrEndOfAddr = strchr(strViaAddr,']');
        if (NULL != pStrEndOfAddr)
        {
            *pStrEndOfAddr = '\0';
        }
    }
#ifdef TRANSPORT_SCTP_MIXEDIPV4IPV6_ON
    if (0==strncmp(pStrViaAddr, strIPv6Prefix, lenIPv6Prefix))
    {
        pStrViaAddr += lenIPv6Prefix;
    }
    if (0==strncmp(pStrReceivedAddr, strIPv6Prefix, lenIPv6Prefix))
    {
        pStrReceivedAddr += lenIPv6Prefix;
    }
#endif /* #ifdef (RV_NET_TYPE & RV_NET_IPV6) */
#endif /* #if(RV_NET_TYPE & RV_NET_IPV6) */
    
    /* If RECEIVED FROM address and VIA address are not equal, add RECEIVED*/
    if (0 != strcmp(pStrViaAddr, pStrReceivedAddr))
    {
        rv = RvSipViaHeaderSetReceivedParam(hViaHeader, strReceivedAddr);
        return rv;
    }
    
    RV_UNUSED_ARG(pMgr);
    return RV_OK;
}

/***************************************************************************
 * AddRportParameter
 * ------------------------------------------------------------------------
 * General: Add rport parameter to Via header of incoming request, if the
            client asks for it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMsg      - received request message.
 *         pRecvFrom - The received from address.
 ***************************************************************************/
static RvStatus RVCALLCONV AddRportParameter(
                                             IN RvSipMsgHandle       hMsg,
                                             IN SipTransportAddress* pRecvFrom)
{
    RvInt32                    rport;
    RvBool                     bUseRport;
    RvSipHeaderListElemHandle   hListElem;
    RvSipViaHeaderHandle        hViaHeader;
    RvSipMethodType             eMethodType;
    RvStatus                   rv;

    eMethodType = RvSipMsgGetRequestMethod(hMsg);
    if (RVSIP_METHOD_ACK == eMethodType)
    {
        return RV_OK;
    }

    hViaHeader = RvSipMsgGetHeaderByTypeExt(hMsg, RVSIP_HEADERTYPE_VIA,
                                         RVSIP_FIRST_HEADER,RVSIP_MSG_HEADERS_OPTION_ALL,
                                         &hListElem);
    if (NULL == hViaHeader)
    {
        return RV_ERROR_UNKNOWN;
    }

    /* even if RPORT is present, we only fill the parameter for UDP */
    if (RVSIP_TRANSPORT_UDP != RvSipViaHeaderGetTransport(hViaHeader))
    {
        return RV_OK;
    }

    /*-----------------------------------------------------------------
      Check if the client supports and requests the rport parameter
      -----------------------------------------------------------------*/
    rv = RvSipViaHeaderGetRportParam(hViaHeader,&rport,&bUseRport);
    if (RV_OK != rv || RV_FALSE == bUseRport)
    {
        /* if the header is bad-syntax, we want to set the rport parameter.
        so here we check if this is a legal header, and only if it is, we exit
        the function */
        if(RvSipViaHeaderGetStringLength(hViaHeader, RVSIP_VIA_BAD_SYNTAX) == 0)
        {
            return RV_OK;
        }

    }

    /*----------------------------------------------------------------
      If the client sends non-empty rport parameter , it's value
      is ignored
      ----------------------------------------------------------------*/
    rport = RvAddressGetIpPort(&pRecvFrom->addr);
    rv = RvSipViaHeaderSetRportParam(hViaHeader,rport,RV_TRUE);
    return rv;
}

/***************************************************************************
 * CheckCSeqMethod
 * ------------------------------------------------------------------------
 * General: Check if the methods in the request line and in the cseq header are
 *          the same.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMsg      - received request message.
 ***************************************************************************/
static RvStatus RVCALLCONV CheckCSeqMethod( IN RvSipMsgHandle        hMsg)
{
    RvSipMethodType requestMethod, cseqMethod;
    RvSipCSeqHeaderHandle   hCSeq;

    hCSeq = RvSipMsgGetCSeqHeader(hMsg);
    if(NULL == hCSeq)
    {
        return RV_ERROR_UNKNOWN;
    }
    requestMethod = RvSipMsgGetRequestMethod(hMsg);
    cseqMethod = RvSipCSeqHeaderGetMethodType(hCSeq);

    if(requestMethod != cseqMethod)
    {
        return RV_ERROR_UNKNOWN;
    }
    else if(requestMethod == RVSIP_METHOD_OTHER)
    {
        RvInt32 requestMethodOffset, cseqMethodOffset;

        requestMethodOffset = SipMsgGetStrRequestMethod(hMsg);
        cseqMethodOffset = SipCSeqHeaderGetStrMethodType(hCSeq);

/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
        if (SPIRENT_RPOOL_Stricmp(SipMsgGetPool(hMsg), SipMsgGetPage(hMsg), requestMethodOffset,
                          SipCSeqHeaderGetPool(hCSeq), SipCSeqHeaderGetPage(hCSeq),
                          cseqMethodOffset) != 0)
#else
        if (RPOOL_Stricmp(SipMsgGetPool(hMsg), SipMsgGetPage(hMsg), requestMethodOffset,
                          SipCSeqHeaderGetPool(hCSeq), SipCSeqHeaderGetPage(hCSeq),
                          cseqMethodOffset) == RV_FALSE)
#endif
/* SPIRENT_END */
        {
            return RV_ERROR_UNKNOWN;
        }
    }
    return RV_OK;
}


/***************************************************************************
 * UpdateRequestTransaction
 * ------------------------------------------------------------------------
 * General: Update a few components in the transaction that created from a request
 *          message by the application.
 * Return Value: RV_ERROR_NULLPTR - pTransc or hMsg are NULL pointers.
 *                 RV_OutOfResource - No memory for the allocations required.
 *               RV_ERROR_UNKNOWN - An error occured.
 *                 RV_OK - The Via header was successfully created and
 *                            inserted.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction to which the Via header list belongs
 *                         to.
 *          hMsg - The handle to the message from which the application created the
 *               transaction
 ***************************************************************************/
static RvStatus UpdateRequestTransaction(IN Transaction   *pTransc,
                                          IN RvSipMsgHandle hMsg)
{
    RvSipMethodType eMethod;
    RvStatus       rv;

    pTransc->bIsProxy = pTransc->pMgr->isProxy;
    eMethod = RvSipMsgGetRequestMethod(hMsg);
#ifndef RV_SIP_PRIMITIVES
    if(eMethod != RVSIP_METHOD_ACK &&
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
                return rv;
            }
        }
    }
#endif /* #ifdef RV_SIP_AUTH_ON */

    if(eMethod == RVSIP_METHOD_INVITE)
    {
#ifndef RV_SIP_PRIMITIVES
           rv = TransactionUpdateReliableStatus(pTransc, hMsg);
           if (RV_OK != rv)
           {
                RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                          "UpdateRequestTransaction - Transaction 0x%p: Error in trying to construct a new Via list.",
                          pTransc));
                return rv;
           }
#endif /*#ifndef RV_SIP_PRIMITIVES*/
           pTransc->eState = RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD;
    }
    else
    {
        pTransc->eState = RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD;
    }

   /* Copy the list of Via headers from the message to the transaction */
   rv = TransactionUpDateViaList(pTransc, hMsg);
   if (RV_OK != rv)
   {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "UpdateRequestTransaction - Transaction 0x%p: Error in trying to construct a new Via list.",
                  pTransc));
        return rv;
   }
   return RV_OK;
}


/***************************************************************************
 * UpdateTranscConnection
 * ------------------------------------------------------------------------
 * General: Update the connection of the transaction.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - pointer to the transaction.
 *          hConn   - new connection the message was received on.
 *          bIsRequest - indicates whether the transaction is a request
 ***************************************************************************/
static RvStatus UpdateTranscConnection(
                       IN Transaction                       *pTransc,
                       IN  RvSipTransportConnectionHandle   hConn,
                       IN  RvBool                           bIsRequest)
{

    RvStatus rv = RV_OK;
    RvSipTransportConnectionState eConState;

    if(hConn == NULL) /*the response was received on UDP*/
    {
        return RV_OK; 
    }

    /*-------------------------------
      this is a request message
    --------------------------------*/
    if(bIsRequest == RV_TRUE)
    {
        /*The incoming connection is different then the one we just got - detach from it*/
        if (pTransc->hActiveConn != hConn)
        {
            TransactionDetachFromConnection(pTransc,pTransc->hActiveConn,RV_TRUE);

            /*attach to the new connection*/
            rv = TransactionAttachToConnection(pTransc,hConn,RV_TRUE);
            if(rv != RV_OK)
            {
                RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                           "UpdateTranscConnection - Failed to  Attach Connection 0x%p transaction 0x%p - returning success",hConn,pTransc));
                return RV_OK;
            }
            
            /*update the connection in the transmitter*/
            rv = SipTransmitterSetConnection(pTransc->hTrx,pTransc->hActiveConn);
            if(rv != RV_OK)
            {
                RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                    "UpdateTranscConnection - Can't Attach Connection 0x%p to tranmitter 0x%p(rv=%d:%s)",
                    hConn, pTransc->hTrx, rv, SipCommonStatus2Str(rv)));
                return RV_OK;

            }
        }
        return RV_OK;
    }
    /*-------------------------------
      this is a response message
    --------------------------------*/
    /*if the response was received on one of the transaction connections
      there is no need for update*/
    if(hConn == pTransc->hActiveConn ||
       hConn == pTransc->hClientResponseConnBackUp)
    {
        return RV_OK;
    }

    /*the response was received on a totally different connection
      save it in the hClientResponseConnBackUp*/
    TransactionDetachFromConnection(pTransc,pTransc->hClientResponseConnBackUp,RV_FALSE);
    pTransc->hClientResponseConnBackUp = NULL;

    RvSipTransportConnectionGetCurrentState(hConn,&eConState);
    if (RVSIP_TRANSPORT_CONN_STATE_CLOSING != eConState &&
        RVSIP_TRANSPORT_CONN_STATE_CLOSED != eConState)
    {
        rv = TransactionAttachToConnection(pTransc,hConn,RV_FALSE);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                       "UpdateTranscConnection - Transc 0x%p, Connection 0x%p , Failed to attach to hClientResponseConnBackUp, returning success",pTransc,hConn));

        }
        else
        {
            pTransc->hClientResponseConnBackUp = hConn;
        }
    }

    return RV_OK;
}
/***************************************************************************
 * UpdateTranscSavedMsgToAndFrom
 * ------------------------------------------------------------------------
 * General: Update the mag tags in transaction, in order to check it in
 *          outgoing response
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc        - pointer to the transaction.
 *          hReceivedMsg   - received message.
 ***************************************************************************/
static RvStatus UpdateTranscSavedMsgToAndFrom(
                       IN  Transaction     *pTransc,
                       IN  RvSipMsgHandle   hReceivedMsg)
{
    RvSipPartyHeaderHandle hPartyHeader;
   RvStatus              rv;

    /* to-header */
    hPartyHeader = RvSipMsgGetToHeader(hReceivedMsg);
    rv = SipPartyHeaderConstructAndCopy(pTransc->pMgr->hMsgMgr,
                                        pTransc->pMgr->hGeneralPool,
                                        pTransc->memoryPage,
                                        hPartyHeader,
                                        RVSIP_HEADERTYPE_TO,
                                        &(pTransc->reqMsgToHeader));

    if(RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "UpdateTranscSavedMsgToAndFrom: transc 0x%p - failed to copy To header. destruct transc", pTransc));
        return rv;
    }
    
    /* from-header */
    hPartyHeader = RvSipMsgGetFromHeader(hReceivedMsg);
    rv = SipPartyHeaderConstructAndCopy(pTransc->pMgr->hMsgMgr,
                                        pTransc->pMgr->hGeneralPool,
                                        pTransc->memoryPage,
                                        hPartyHeader,
                                        RVSIP_HEADERTYPE_FROM,
                                        &(pTransc->reqMsgFromHeader));
    if(RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "UpdateTranscSavedMsgToAndFrom: transc 0x%p - failed to copy From header. destruct transc", pTransc));
        return rv;
    }

    return RV_OK;
}







/***************************************************************************
 * TranscMgrRejectMsgNoTransc
 * ------------------------------------------------------------------------
 * General: Sends 400 response for a bad-syntax request, without a related
 *          transaction.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pTranscMgr     - The transaction manager object.
 *          hLocalAddr      - The local address to send the response from.
 *          hMsg            - Handle to the received bad-syntax message.
 *          responseCode    - 400 for bad-syntax or 500 for server internal error.
 ***************************************************************************/
static RvStatus RVCALLCONV TranscMgrRejectMsgNoTransc(
                            IN    TransactionMgr                *pTranscMgr,
                            IN    RvSipTransportLocalAddrHandle  hLocalAddr,
                            IN    RvSipMsgHandle                 hMsg,
                            IN    RvInt16                        responseCode,
                            IN    RvBool                         bAddToTag)
{
    RvSipMsgHandle            hResponseMsg;
    RvStatus                 rv = RV_OK;

    /* build the response message*/
    rv = RvSipMsgConstruct(pTranscMgr->hMsgMgr, pTranscMgr->hMessagePool, &hResponseMsg);
    if(RV_OK != rv)
    {
        RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "TranscMgrRejectMsgNoTransc - TranscMgr= 0x%p, Failed to construct a response message (rv=%d)",
            pTranscMgr,rv));
        return rv;
    }

    /* copy the headers from received message, and it to response */
    rv = RvSipMsgSetToHeader(hResponseMsg, RvSipMsgGetToHeader(hMsg));
    if(rv == RV_OK)
    {
        rv = RvSipMsgSetFromHeader(hResponseMsg, RvSipMsgGetFromHeader(hMsg));
    }
    if(rv == RV_OK)
    {
        rv = RvSipMsgSetCSeqHeader(hResponseMsg, RvSipMsgGetCSeqHeader(hMsg));
    }
    if(rv == RV_OK)
    {
        RvChar callId[200];
        RvUint len;
        rv = RvSipMsgGetCallIdHeader(hMsg, callId, 200, &len);
        if(rv == RV_OK)
        {
            rv = RvSipMsgSetCallIdHeader(hResponseMsg, callId);
        }
    }
    if(rv == RV_OK)
    {
        RvSipHeaderListElemHandle hReqElem, hRespElem;
        void*                     via;
        RvSipHeadersLocation eLocation = RVSIP_FIRST_HEADER;
        /* copy all via headers from the request to the response */
        via = RvSipMsgGetHeaderByTypeExt(hMsg, RVSIP_HEADERTYPE_VIA, eLocation,
                                         RVSIP_MSG_HEADERS_OPTION_ALL, &hReqElem);
        if(via == NULL)
        {
            RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                "TranscMgrRejectMsgNoTransc - TranscMgr= 0x%p, Failed to get via header from message (rv=%d)",
                pTranscMgr,rv));
            rv = RV_ERROR_NOT_FOUND;
        }
        while (rv == RV_OK && via != NULL)
        {
            rv = RvSipMsgPushHeader(hResponseMsg, eLocation, via, RVSIP_HEADERTYPE_VIA, &hRespElem, &via);
            if(rv != RV_OK)
            {
                RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                    "TranscMgrRejectMsgNoTransc - TranscMgr= 0x%p, Failed to set via header in message (rv=%d)",
                    pTranscMgr,rv));
            }

            eLocation = RVSIP_NEXT_HEADER;

            via = RvSipMsgGetHeaderByTypeExt(hMsg, RVSIP_HEADERTYPE_VIA, eLocation,
                                           RVSIP_MSG_HEADERS_OPTION_ALL, &hReqElem);
        }
    }
    if(rv != RV_OK)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "TranscMgrRejectMsgNoTransc - TranscMgr= 0x%p, Failed to fill the response message (rv=%d) destruct msg 0x%p",
            pTranscMgr,rv, hResponseMsg));
        RvSipMsgDestruct(hResponseMsg);
        return rv;
    }


    /* add to-tag if needed */
    if(RV_TRUE == bAddToTag)
    {
        /* Add tag to the To header, if missing */
        RvSipPartyHeaderHandle hTo;
        RvChar   strTag[SIP_COMMON_ID_SIZE];

        hTo = RvSipMsgGetToHeader(hResponseMsg);
        if(UNDEFINED == SipPartyHeaderGetTag(hTo))
        {
            rv = TransactionGenerateTag(pTranscMgr, NULL, (RvChar*)&strTag);
            if (RV_OK != rv)
            {
                RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                    "TranscMgrRejectMsgNoTransc - TranscMgr= 0x%p, Failed to generate To tag.",
                    pTranscMgr));
				RvSipMsgDestruct(hResponseMsg);
                return rv;
            }
            rv = RvSipPartyHeaderSetTag(hTo, strTag);
            if (RV_OK != rv)
            {
                RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                    "TranscMgrRejectMsgNoTransc - TranscMgr= 0x%p, Failed to set To tag.",
                    pTranscMgr));
				RvSipMsgDestruct(hResponseMsg);
                return rv;
            }
        }
    }
    if(responseCode == 400 && SipMsgGetBadSyntaxReasonPhrase(hMsg) != UNDEFINED)
    {
        RvSipMsgSetStatusCode(hResponseMsg, 400, RV_FALSE);
        rv = SipMsgSetReasonPhrase(hResponseMsg,
                          NULL,
                          SipMsgGetPool(hMsg),
                          SipMsgGetPage(hMsg),
                          SipMsgGetBadSyntaxReasonPhrase(hMsg));
        if(RV_OK != rv)
        {
            RvSipMsgDestruct(hResponseMsg);
            return rv;
        }
    }
    else
    {
        RvSipMsgSetStatusCode(hResponseMsg, responseCode, RV_TRUE);
    }

    /* send message - the message will be destructed in this function */
    rv = TransactionTransportSendOutofContextMsg(pTranscMgr,hResponseMsg,
                                                 RV_FALSE,NULL,NULL,
                                                 hLocalAddr,NULL,0,
#if defined(UPDATED_BY_SPIRENT)
						 NULL,0,
#endif
                                                 RV_FALSE,RV_FALSE,
                                                 RV_FALSE /*don't copy message*/);
    if(RV_OK != rv)
    {
        RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "TranscMgrRejectMsgNoTransc - TranscMgr= 0x%p, Failed to send the response message (rv=%d)",
            pTranscMgr,rv));
        return rv;
    }
    return RV_OK;
}

/***************************************************************************
* RequestMessageReceived
* ------------------------------------------------------------------------
* General: Handles a received SIP request message (including errneous msgs)
* Return Value: RV_OK             - The message has been received by a
*                                   compatible transaction.
*               RV_ERROR_NULLPTR  - The transaction manager pointer or the
*                                   message pointer are NULL pointers
*               RV_OutOfResource  - Couldn't allocate a new transaction when
*                                    required.
*               RV_ERROR_BADPARAM - The message is invalid.
* ------------------------------------------------------------------------
* Arguments:
* Input:  pReceivedMsg -   A message belonging to the transaction this
*                          function is searching for. There can be only
*                          one transaction this message belongs to.
*          hTranscMgr  -   The transactions manager.
*          pLocalAddr  -   The local address to which the request message was
*                          received.
*          pRecvFromAddr - The address from which the message was received.
*          eBSAction    -  Bad-syntax action, that was given by application in
*                          bad-syntax callbacks. continue processing, reject, discard.
*          pSigCompMsgInfo - SigComp info related to the received message.
*                            The information contains indication if the UDVM
*                            waits for a compartment ID for the given unique
*                            ID that was related to message by the UDVM.
***************************************************************************/
static RvStatus RVCALLCONV RequestMessageReceived(
                         IN    TransactionMgr                 *pTranscMgr,
                         IN    RvSipMsgHandle                  hReceivedMsg,
                         IN    RvSipTransportConnectionHandle  hConn,
                         IN    RvSipTransportLocalAddrHandle   hLocalAddr,
                         IN    SipTransportAddress*            pRecvFromAddr,
                         IN    RvSipTransportBsAction          eBSAction,
                         INOUT SipTransportSigCompMsgInfo     *pSigCompMsgInfo)
{
    RvSipMethodType    eMethod;
    RvStatus           rv;

    if ((NULL == hReceivedMsg))
    {
        return RV_ERROR_NULLPTR;
    }

    eMethod = RvSipMsgGetRequestMethod(hReceivedMsg);

    if(eBSAction == RVSIP_TRANSPORT_BS_ACTION_REJECT_MSG &&
       eMethod   != RVSIP_METHOD_ACK)
    {
    /* application chose to reject this incoming request, because of
        syntax-errors in it */
        RvLogInfo(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "RequestMessageReceived - Reject message 0x%p. eBSAction=%d",
            hReceivedMsg, eBSAction));
        HandleViaHeaderParameters(pTranscMgr,hReceivedMsg,pRecvFromAddr);

        rv = TranscMgrRejectMsgNoTransc(pTranscMgr,hLocalAddr,hReceivedMsg, 400, RV_TRUE);

        return rv;
    }
    else if(eBSAction == RVSIP_TRANSPORT_BS_ACTION_REJECT_MSG &&
            eMethod   == RVSIP_METHOD_ACK)
    {
        /* cannot reject an ACK request. we shall treat it as reject on response
           --> discard the ACK message */
        RvLogInfo(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "RequestMessageReceived - Discard Ack message 0x%p. (syntax errors + BSAction=Reject)",
            hReceivedMsg));

        return RV_OK;
    }
    else
    {
        RvInt16 respInFailureCase = 0;
        rv = HandleIncomingRequestMsg(pTranscMgr, hReceivedMsg, hConn, hLocalAddr,
            pRecvFromAddr, pSigCompMsgInfo, &respInFailureCase);
        if(rv != RV_OK && eMethod != RVSIP_METHOD_ACK && respInFailureCase >= 400)
        {
/* SPIRENT_BEGIN */
/*
* COMMENTS:
* Modified by Armyakov
* TransportError handlers 
*/
#if defined(UPDATED_BY_SPIRENT)
#define EVENT_TEXT_MSG_MAX_LEN 127
            RvChar pEventTextMsg[EVENT_TEXT_MSG_MAX_LEN + 1];
            RvChar pTextIPAddr[EVENT_TEXT_MSG_MAX_LEN + 1];
            RvAddressGetString(&(pRecvFromAddr->addr), EVENT_TEXT_MSG_MAX_LEN, pTextIPAddr);
            pTextIPAddr[EVENT_TEXT_MSG_MAX_LEN] = 0;
            if(respInFailureCase == 482)
            {
                /* not error, but loop detected - print warning and not error */
                snprintf(pEventTextMsg, EVENT_TEXT_MSG_MAX_LEN,
                         "Merging incoming request from %s. Reject new request with %d",
                         pTextIPAddr, respInFailureCase);
                RvLogInfo(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                    "RequestMessageReceived - merging incoming request... reject new request with %d",
                    respInFailureCase));
            }
            else
            {
                snprintf(pEventTextMsg, EVENT_TEXT_MSG_MAX_LEN,
                         "Failed to handle incoming request message (rv=%d) from %s. Rejecting message with %d",
                         rv, pTextIPAddr, respInFailureCase);
                RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                    "RequestMessageReceived - Failed to handle incoming request message (rv=%d). rejecting message with %d",
                    rv,respInFailureCase));
            }
            pEventTextMsg[EVENT_TEXT_MSG_MAX_LEN] = 0;
            RvSipTransportErrorRejectMsgEvHandler(pEventTextMsg);
#else /* not defined(UPDATED_BY_SPIRENT) */
            if(respInFailureCase == 482)
            {
                /* not error, but loop detected - print warning and not error */
                RvLogInfo(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                    "RequestMessageReceived - merging incoming request 0x%p... reject new request with %d",
                    hReceivedMsg,respInFailureCase));
            }
            else
            {
                RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                    "RequestMessageReceived - Failed to handle incoming request message (rv=%d). rejecting message 0x%p with %d",
                    rv,hReceivedMsg,respInFailureCase));
            }
#endif /* defined(UPDATED_BY_SPIRENT) */
/* SPIRENT_END */
            RvMutexLock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
            rv = TranscMgrRejectMsgNoTransc(pTranscMgr,
                                            hLocalAddr,
                                            hReceivedMsg,
                                            respInFailureCase,
                                            RV_TRUE);

            RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
            if(rv != RV_OK)
            {
                RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                    "RequestMessageReceived - Failed to send out of context reject message (rv=%d)",rv));
            }

            return rv;
        }
    }

    return rv;
}

/***************************************************************************
* ResponseMessageReceived
* ------------------------------------------------------------------------
* General: Handles a received SIP request message (including errneous msgs)
* Return Value: RV_OK             - The message has been received by a
*                                   compatible transaction.
*               RV_ERROR_NULLPTR  - The transaction manager pointer or the
*                                   message pointer are NULL pointers
*               RV_OutOfResource  - Couldn't allocate a new transaction when
*                                    required.
*               RV_ERROR_BADPARAM - The message is invalid.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pReceivedMsg -   A message belonging to the transaction this
*                          function is searching for. There can be only
*                          one transaction this message belongs to.
*          hTranscMgr  -   The transactions manager.
*          pLocalAddr  -   The local address to which the request message was
*                          received.
*          pRecvFromAddr - The address from which the message was received.
*          eBSAction    -  Bad-syntax action, that was given by application in
*                          bad-syntax callbacks. continue processing, reject, discard.
*          pSigCompMsgInfo - SigComp info related to the received message.
*                            The information contains indication if the UDVM
*                            waits for a compartment ID for the given unique
*                            ID that was related to message by the UDVM.
***************************************************************************/
static RvStatus RVCALLCONV ResponseMessageReceived(
                         IN    TransactionMgr                *pTranscMgr,
                         IN    RvSipMsgHandle                 hReceivedMsg,
                         IN    RvSipTransportConnectionHandle hConn,
                         IN    RvSipTransportLocalAddrHandle  hLocalAddr,
                         IN    SipTransportAddress           *pRecvFromAddr,
                         IN    RvSipTransportBsAction         eBSAction,
                         INOUT SipTransportSigCompMsgInfo    *pSigCompMsgInfo)
{
    RvStatus       rv = RV_OK;

    rv = HandleIncomingResponseMsg(pTranscMgr,
                                   hReceivedMsg,
                                   hConn,
                                   hLocalAddr,
                                   pRecvFromAddr,
                                   pSigCompMsgInfo);
    if(rv != RV_OK)
    {
        RvLogWarning(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "ResponseMessageReceived - Failed to handle incoming response message 0x%p",hReceivedMsg));
    }

    RV_UNUSED_ARG(eBSAction);

    return rv;
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
* UpdateCancelTransactionOwner
* ------------------------------------------------------------------------
* General: Update the cancel transaction owner according to the canceled
*          transaction owner.
*          This function is used only when bEnableNestedInitialRequestHandling is true,
*          because in this case - of two invites that created two call-legs
*          with same identifiers, the canceled call-leg is not in the hash yet.
*          in this case we can reached the canceled call-leg, by using the owner
*          of the cancelled transaction.
* Return Value: RV_Status
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTransc - The CANCEL transaction.
*          pKey - The transaction key.
* Output:  ppOwner - The owner that we set to the canceled transaction.
***************************************************************************/
static RvStatus RVCALLCONV UpdateCancelTransactionOwner(
                                       IN  Transaction           *pTransc,
                                       IN  SipTransactionKey     *pKey,
                                       OUT void                  **ppOwner)
{
    RvStatus rv;
    if(RV_TRUE == pTransc->pMgr->bEnableNestedInitialRequestHandling)
    {
        Transaction* pCancelledTransc;

        /*find the transaction this transaction is cancelling*/
        TransactionMgrHashFindCancelledTransc(pTransc->pMgr,
                                   (RvSipTranscHandle)pTransc,
                                   (RvSipTranscHandle*)&pCancelledTransc);
        if(NULL != pCancelledTransc)
        {
            RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "UpdateCancelTransactionOwner: transaction 0x%p - setting owner according to the cancelled transc 0x%p",
                pTransc, pCancelledTransc));
            /*lock the transaction*/
            rv = TransactionLockMsg(pCancelledTransc);
            if (rv != RV_OK)
            {
                pCancelledTransc = NULL;
            }

            /* see if the transaction is still in the hash - the transaction can be
               out of the hash if other thread terminated it before we got the chance to
               lock it.*/
            if(pCancelledTransc != NULL)
            {
                Transaction*    pSecondCheckTransc = NULL;
                TransactionMgrHashFindCancelledTransc(pTransc->pMgr,
                                                     (RvSipTranscHandle)pTransc,
                                                     (RvSipTranscHandle*)&pSecondCheckTransc);

                if(pSecondCheckTransc == NULL)
                {
                    TransactionUnLockMsg(pCancelledTransc);
                    pCancelledTransc = NULL;
                }
            }

            if(pCancelledTransc != NULL)
            {
                /* now we have a locked canceled transaction. we can attach the
                   canceled transaction owner, also to the cancel transaction owner */
                TransactionAttachOwner((RvSipTranscHandle)pTransc,
                                        pCancelledTransc->pOwner,
                                        pKey,
                                        pCancelledTransc->tripleLock,
                                        pCancelledTransc->eOwnerType,
                                        RV_TRUE);
                if(pCancelledTransc->eOwnerType == SIP_TRANSACTION_OWNER_CALL)
                {
                     *ppOwner = pCancelledTransc->pOwner;
                }
                TransactionUnLockMsg(pCancelledTransc);
            }
        }
    }
    return RV_OK;
}
#endif /*#ifndef RV_SIP_PRIMITIVES*/
/***************************************************************************
* MergeRequestIfNeeded
* ------------------------------------------------------------------------
* General: For each incoming initial request, the function checks if there is
*          already a transaction that handles this request.
*          (this is what the standard calls 'merging' - a proxy forked the
*           request and it reach the final UAS in 2 different transactions).
*          the function search for a transaction with the same request identifiers:
*          from-tag, call-id, and cseq, and the transaction is on ongoing state.
*          if such transaction is found - 482 'Loop Detected' should be returned
*          in response, bu the transaction manager.
* Return Value: RV_Status
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTranscMgr - The transaction manager.
*          hMsg - The received request message.
*          pKey - The transaction key.
* Output:  pFailureResp - The responseCode to retrieve if merging is needed (482).
*          bMerged       - is merging needed or not.
*          pMergedTransc - The first transaction that handles this request.
***************************************************************************/
static RvStatus RVCALLCONV MergeRequestIfNeeded(
                                       IN  TransactionMgr       *pTranscMgr,
                                       IN  RvSipMsgHandle        hMsg,
                                       IN  SipTransactionKey    *pKey,
                                       OUT RvInt16              *pFailureResp,
                                       OUT RvBool               *bMerged,
                                       OUT Transaction          **pMergedTransc)
{
    RvSipPartyHeaderHandle hTo;

    /* for request with to-tag, no need to check merging */
    hTo = RvSipMsgGetToHeader(hMsg);
    if(SipPartyHeaderGetTag(hTo) > UNDEFINED)
    {
        /* to-tag exists - no need to check merging */
        *bMerged = RV_FALSE;
        return RV_OK;
    }

    /* for cancel - no need to check??? */

    TransactionMgrHashFindMergedTransc(pTranscMgr, hMsg, pKey,
                                       (RvSipTranscHandle*)pMergedTransc);
    if(*pMergedTransc == NULL)
    {
        *bMerged = RV_FALSE;
        return RV_OK;
    }

    /* for merging case - first transaction must be in ongoing state,
       and the transaction is not a proxy transaction....
       (proxy will never support merging transactions)*/
    if((*pMergedTransc)->eState != RVSIP_TRANSC_STATE_TERMINATED &&
        ((*pMergedTransc)->bIsProxy == RV_FALSE))
    {
        /* we found an ongoing transaction, handles the same request */
        *bMerged = RV_TRUE;
        *pFailureResp = 482;
        return RV_OK;
    }

    /* the transaction we found is already terminated */
    *bMerged = RV_FALSE;
    return RV_OK;

}
/***************************************************************************
* CheckFoundTranscValidity
* ------------------------------------------------------------------------
* General: check that a transaction that we found for an incoming message
*          indeed belong to the message. The message will not belong to the
*          transaction in the following cases:
*          1. We fail to lock the transaction
*          2. We locked the transaction but it is no longer in the hash
*          3. The tags of the message don't fit the tags of the transaction.
*          Note: currently the function is called for response messages only.
* Return Value: (-)
* ------------------------------------------------------------------------
* Arguments:
* Input:  pTransc      - pointer to the transaction.
*         hReceivedMsg - The message.
***************************************************************************/
static void RVCALLCONV CheckFoundTranscValidity(
                                         IN  Transaction*   pTransc,
                                         IN  RvSipMsgHandle hMsg,
                                         IN  RvBool         bIsRespOnInvite,
                                         OUT RvBool*        pbTranscValid)
{

    *pbTranscValid = RV_TRUE;
    if (RV_OK != TransactionLockMsg(pTransc))
    {
        *pbTranscValid = RV_FALSE;
        return;
    }

    /*check if the transaction is still in the hash*/
    if(pTransc->hashElementPtr == NULL)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "CheckFoundTranscValidity - transaction 0x%p, msg 0x%p - transc is not valid - not in hash",
            pTransc, hMsg));

        *pbTranscValid = RV_FALSE;
        TransactionUnLockMsg(pTransc);
        return;

    }

    /*check if the tags are matching in case forking is enabled*/
    if(RV_TRUE == bIsRespOnInvite &&
       RV_TRUE == pTransc->pMgr->bEnableForking)
    {
        RvBool correctTags;
        /* we need to verify that the transaction we found, has same tags as in the message
           (transaction compartion is done with the via branch. but if this response has
            different tags, then it was probably received from a different UAS after request
            had forked )*/
        correctTags = VerifyResponseAndTranscTags(pTransc, hMsg);
        if(correctTags == RV_FALSE)
        {
            RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "CheckFoundTranscValidity - transaction 0x%p tags and received response 0x%p - tags not match",pTransc,hMsg));
            *pbTranscValid = RV_FALSE;
            TransactionUnLockMsg(pTransc);
            return;
        }
    }
    TransactionUnLockMsg(pTransc);
}


/***************************************************************************
* VerifyResponseAndTranscTags
* ------------------------------------------------------------------------
* General: check that a response message tags matches the transaction tags.
*          when this function is called the transaction is locked.
* Return Value: RV_FALSE/RV_TRUE.
* ------------------------------------------------------------------------
* Arguments:
* Input:  pTransc      - pointer to the transaction.
*         hReceivedMsg - The response message.
***************************************************************************/
static RvBool VerifyResponseAndTranscTags(Transaction* pTransc,
                                         RvSipMsgHandle hReceivedReponseMsg)
{
    RvSipPartyHeaderHandle hMsgFromHeader, hMsgToHeader;
    RPOOL_Ptr             strMsgToTag;
    RPOOL_Ptr             strMsgFromTag;
    RPOOL_Ptr             strTranscToTag;
    RPOOL_Ptr             strTranscFromTag;
    RvBool               fromTagsEqual;
    RvBool               toTagsEqual;


    hMsgFromHeader = RvSipMsgGetFromHeader(hReceivedReponseMsg);
    hMsgToHeader   = RvSipMsgGetToHeader(hReceivedReponseMsg);

    strMsgFromTag.offset = SipPartyHeaderGetTag(hMsgFromHeader);
    strMsgFromTag.hPage  = SipPartyHeaderGetPage(hMsgFromHeader);
    strMsgFromTag.hPool  = SipPartyHeaderGetPool(hMsgFromHeader);

    strMsgToTag.offset   = SipPartyHeaderGetTag(hMsgToHeader);
    strMsgToTag.hPage    = SipPartyHeaderGetPage(hMsgToHeader);
    strMsgToTag.hPool    = SipPartyHeaderGetPool(hMsgToHeader);

    strTranscFromTag.offset = SipPartyHeaderGetTag(pTransc->hFromHeader);
    strTranscFromTag.hPage  = SipPartyHeaderGetPage(pTransc->hFromHeader);
    strTranscFromTag.hPool  = SipPartyHeaderGetPool(pTransc->hFromHeader);

    strTranscToTag.offset   = SipPartyHeaderGetTag(pTransc->hToHeader);
    strTranscToTag.hPage    = SipPartyHeaderGetPage(pTransc->hToHeader);
    strTranscToTag.hPool    = SipPartyHeaderGetPool(pTransc->hToHeader);

    /* if transaction has no to-tag yet, it matchs every to-tag */
    if(strTranscToTag.offset == UNDEFINED)
    {
        strMsgToTag.offset = UNDEFINED;
    }

/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
    fromTagsEqual = SPIRENT_RPOOL_Strcmp(   strMsgFromTag.hPool,
                                    strMsgFromTag.hPage,
                                    strMsgFromTag.offset,
                                    strTranscFromTag.hPool,
                                    strTranscFromTag.hPage,
                                    strTranscFromTag.offset) == 0;

    toTagsEqual = SPIRENT_RPOOL_Strcmp( strMsgToTag.hPool,
                                strMsgToTag.hPage,
                                strMsgToTag.offset,
                                strTranscToTag.hPool,
                                strTranscToTag.hPage,
                                strTranscToTag.offset) == 0;
#else
    fromTagsEqual = RPOOL_Strcmp(   strMsgFromTag.hPool,
                                    strMsgFromTag.hPage,
                                    strMsgFromTag.offset,
                                    strTranscFromTag.hPool,
                                    strTranscFromTag.hPage,
                                    strTranscFromTag.offset);

    toTagsEqual = RPOOL_Strcmp( strMsgToTag.hPool,
                                strMsgToTag.hPage,
                                strMsgToTag.offset,
                                strTranscToTag.hPool,
                                strTranscToTag.hPage,
                                strTranscToTag.offset);
#endif
/* SPIRENT_END */
    if(RV_FALSE == fromTagsEqual || RV_FALSE == toTagsEqual)
    {
        return RV_FALSE;
    }

    return RV_TRUE;

}

/***************************************************************************
* Is1xx2xxRespWithToTagOnInviteMsg
* ------------------------------------------------------------------------
* General: check that a response message is 2xx or 1xx (not 100) for an
*          INVITE request, and that the message contains to-tag.
* Return Value: RV_FALSE/RV_TRUE.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hReceivedMsg - The response message.
***************************************************************************/
static RvBool Is1xx2xxRespWithToTagOnInviteMsg(RvSipMsgHandle  hReceivedMsg)
{
    RvSipCSeqHeaderHandle hCSeq;
    RvSipMethodType cseqMethod;
    RvSipPartyHeaderHandle hTo;
    RvUint          toTagLength = 0;
    RvUint16        statusCode;

    hCSeq = RvSipMsgGetCSeqHeader(hReceivedMsg);
    cseqMethod = RvSipCSeqHeaderGetMethodType(hCSeq);

    if(cseqMethod != RVSIP_METHOD_INVITE)
    {
        /* not INVITE */
        return RV_FALSE;
    }

    hTo = RvSipMsgGetToHeader(hReceivedMsg);
    toTagLength = RvSipPartyHeaderGetStringLength(hTo,RVSIP_PARTY_TAG);
    if(toTagLength == 0)
    {
        /* response with no to-tag, ignore it */
        return RV_FALSE;
    }
    statusCode = RvSipMsgGetStatusCode(hReceivedMsg);
    if(statusCode >= 300)
    {
        /* 1xx or 2xx */
        return RV_FALSE;
    }

    return RV_TRUE;
}

/***************************************************************************
 * NotifyAppOfResponseWithNoTransc
 * ------------------------------------------------------------------------
 * General: Notifies upper layer (call-leg or application) of a new response
 *          that was received, but was not mapped to any transaction.
 *
 *  Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *          pTranscMgr - The transaction manager which received the message
 *          bIsRespOnInvite - indicates if this is a response for Invite.
 *          hMsg       - The received Out of Context message.
 *          hLocalAddr - indicates the local address on which the message
 *                       was received.
 *          pRcvdFromAddr - The address from where the message
 *                         was received.
 *          pOptions - Other identifiers of the message such as compression type.
 *          hConn      - If the message was received on a specific connection,
 *                       the connection handle is supplied. Otherwise this parameter
 *                       is set to NULL.
 ***************************************************************************/
static void RVCALLCONV NotifyAppOfResponseWithNoTransc(
                            IN TransactionMgr*                 pTranscMgr,
                            IN RvBool                          bIsRespOnInvite,
                            IN RvSipMsgHandle                  hMsg,
                            IN RvSipTransportLocalAddrHandle   hLocalAddr,
                            IN SipTransportAddress*            pRcvdFromAddr,
                            IN RvSipTransportAddrOptions*      pOptions,
                            IN RvSipTransportConnectionHandle  hConn)

{
#ifndef RV_SIP_PRIMITIVES
        RvBool bWasResponseHandled = RV_FALSE;

       /* 1. if this is 1xx/2xx on invite we will first notify the call-leg.
          The response can be a result of forking or a retransmission of 2xx
          when the first 2xx terminated the transaction.*/
        if(RV_TRUE == bIsRespOnInvite)
        {
            if(pTranscMgr->bOldInviteHandling == RV_FALSE ||
               pTranscMgr->bEnableForking == RV_TRUE)
            {

                /* inform the call-leg layer about this 'extra' 1xx/2xx invite response */
                TransactionCallbackMgrInviteResponseNoTranscEv(pTranscMgr,hMsg, pRcvdFromAddr, &bWasResponseHandled);
                if(RV_TRUE == bWasResponseHandled)
                {
                    return;
                }
                if(pTranscMgr->isProxy == RV_FALSE)
                {
                    RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                               "NotifyAppOfResponseWithNoTransc - Received an Invite 2xx/1xx response with no existing transaction or dialog, let the application handle the message"));

                    TransactionCallbackMgrOutOfContextMsgEv(pTranscMgr, hMsg, hLocalAddr,
                                                            pRcvdFromAddr,pOptions,hConn);


                }
            }
            if(pTranscMgr->isProxy == RV_TRUE)
            {
                RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                    "NotifyAppOfResponseWithNoTransc - Received an Invite 2xx/1xx response with no existing transaction or dialog, let the proxy handle the message"));

                TransactionCallbackMgrOutOfContextMsgEv(pTranscMgr, hMsg, hLocalAddr,
                                                        pRcvdFromAddr,pOptions,hConn);
                return;
            }
        }
        else
        {

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
            /*notity any application (not just proxy app) which is interested in out-of-context messages */
            if(RV_TRUE)
#else
            /*notify the proxy on any out of context response*/
            if(RV_TRUE == pTranscMgr->isProxy)
#endif
/* SPIRENT_END */
            {
                RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                    "NotifyAppOfResponseWithNoTransc - Received a response with no existing transaction, let the application handle the message"));
                TransactionCallbackMgrOutOfContextMsgEv(pTranscMgr, hMsg, hLocalAddr,
                    pRcvdFromAddr,pOptions,hConn);

            }
        }

#else /*RV_SIP_PRIMITIVES compilation*/
        if(RV_TRUE == pTranscMgr->isProxy)
        {
            RvLogDebug(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                "NotifyAppOfResponseWithNoTransc - Received a response with no existing transaction, let the application handle the message"));
            TransactionCallbackMgrOutOfContextMsgEv(pTranscMgr, hMsg, hLocalAddr,
                pRcvdFromAddr,pOptions,hConn);
        }
        else /*not proxy*/
        {
            /* 2. give 1xx/2xx response on INVITE to application for handling */
            if(RV_TRUE == bIsRespOnInvite)
            {
                
                if(pTranscMgr->bOldInviteHandling == RV_FALSE || 
                   pTranscMgr->bEnableForking == RV_TRUE)
                {
                    TransactionCallbackMgrOutOfContextMsgEv(pTranscMgr, hMsg, hLocalAddr,
                                                            pRcvdFromAddr,pOptions,hConn);
                }
                else if(NULL != pTranscMgr->pAppEvHandlers.pfnEvForkedInviteRespRcvd && 
                        pTranscMgr->bEnableForking == RV_TRUE)
                {
                    /* inform the application about this 'extra' response */
                    TransactionCallbackMgrAppForkedInviteRespEv(pTranscMgr,hMsg);
                } 
            }
        }
#endif /*#ifndef RV_SIP_PRIMITIVES*/
}

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SetSecAgreeEvHandlers
 * ------------------------------------------------------------------------
 * General: Set the security-agreement events handlers to the security-agreement 
 *          module.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgreeMgr - Handle to the security-agreement manager.
 ***************************************************************************/
static void RVCALLCONV SetSecAgreeEvHandlers(
                               IN RvSipSecAgreeMgrHandle   hSecAgreeMgr)
{
	SipSecAgreeEvHandlers   secAgreeEvHandlers;

    memset(&secAgreeEvHandlers,0,sizeof(SipSecAgreeEvHandlers));
	secAgreeEvHandlers.pfnSecAgreeAttachSecObjEvHanlder = TransactionSecAgreeAttachSecObjEv;
    secAgreeEvHandlers.pfnSecAgreeDetachSecAgreeEvHanlder = TransactionSecAgreeDetachSecAgreeEv;
	
    SipSecAgreeMgrSetEvHandlers(
                             hSecAgreeMgr,
                             &secAgreeEvHandlers,
							 RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSACTION);
}
#endif /* RV_SIP_IMS_ON */

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * HandleIncomingSigCompAckIfNeeded
 * ------------------------------------------------------------------------
 * General: The function should be called when a SigComp-ACK SIP message is
 *          received. According to the SigComp model each SigComp-SIP
 *          message MUST be related to any SigComp compartment. Thus, the
 *          function associates the ACK with a SigComp Compartment. 
 *          In case that the ACK is not associated, a new compartment is 
 *          created.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr       - The transaction manager
 *          hConn      - The connection on which the message was received.
 *          pRecvFrom  - The address from which the message was received.
 *          hLocalAddr - The address on which the message was received
 *          hMsg       - Handle to the incoming message, to be related to a 
 *                       Compartment object 
 *          pSigCompMsgInfo - 
 *                      SigComp info related to the received message.
 *                      The information contains indication if the UDVM
 *                      waits for a compartment ID for the given unique
 *                      ID that was related to message by the UDVM.
 ***************************************************************************/
static RvStatus RVCALLCONV HandleIncomingSigCompAckIfNeeded(
                             IN    TransactionMgr                  *pMgr,
                             IN    RvSipTransportConnectionHandle   hConn,
                             IN    SipTransportAddress             *pRecvFrom,
                             IN    RvSipTransportLocalAddrHandle    hLocalAddr,
                             IN    RvSipMsgHandle                   hMsg, 
                             INOUT SipTransportSigCompMsgInfo      *pSigCompMsgInfo)
{
    RPOOL_Ptr				urnRpoolPtr;
    RvSipTransportAddr		rvLocalAddr;
	SipTransportAddress		localAddr;
	RvStatus				rv					= RV_OK;
	RvBool					bUrnFound			= RV_FALSE;
    RPOOL_Ptr				*pUrnRpoolPtr		= NULL;
	RvSipCompartmentHandle	hSigCompCompartment = NULL;

    
    /* Check if the received message is a SigComp-compressed message */ 
    if (pSigCompMsgInfo == NULL || pSigCompMsgInfo->bExpectComparmentID == RV_FALSE)
    {
        return RV_OK;
    }

    /* Check if the given Transaction is already related to a Compartment.   */
    /* According to RFC5049, mapped to this Transaction, */ 
    /* would have been sent from a single endpoint,both request and response    */
    /* should have the same unique ID (and the same Compartment) */
	
	/* Look for the SIP/SigComp identifier within the incoming message */    		
	RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
		"HandleIncomingSigCompAckIfNeeded - ACK is looking for URN in hMsg 0x%p", hMsg));
	
	rv = SipCommonLookForSigCompUrnInTopViaHeader(
		hMsg,pMgr->pLogSrc,&bUrnFound,&urnRpoolPtr);
	if (rv != RV_OK)
	{
		RvLogError(pMgr->pLogSrc ,(pMgr->pLogSrc ,
			"HandleIncomingSigCompAckIfNeeded - ACK couldn't retrieve URN from hMsg 0x%p", hMsg));
		return rv; 
	}
	
	pUrnRpoolPtr = (bUrnFound == RV_TRUE) ? &urnRpoolPtr : NULL;
	
    /* Turn off the expectation indication since even if SipCompartmentRelateCompHandleToInMsg */
    /* fails it internally it automatically frees the temporal compartment resources */ 
    pSigCompMsgInfo->bExpectComparmentID = RV_FALSE;
    
    /* Relate the found or NULL (not found) Compartment to the given decompression unique ID */ 
	rv = RvSipTransportMgrLocalAddressGetDetails (hLocalAddr, sizeof(RvSipTransportAddr), &rvLocalAddr);
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "HandleIncomingSigCompAckIfNeeded - Failed to get address 0x%p details",
            hLocalAddr)); 
        return rv;
    } 

	localAddr.eTransport = rvLocalAddr.eTransportType;
	RvSipTransportConvertTransportAddr2RvAddress(&rvLocalAddr, &localAddr.addr);

    rv = SipCompartmentRelateCompHandleToInMsg(
                        pMgr->hCompartmentMgr,
                        pUrnRpoolPtr,
                        pRecvFrom,
						&localAddr,
						hMsg,
                        hConn,
                        pSigCompMsgInfo->uniqueID,
                        &hSigCompCompartment);
    if (rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "HandleIncomingSigCompAckIfNeeded - Failed to relate a Compartment (0x%p) to uniqueID %d",
            hSigCompCompartment,pSigCompMsgInfo->uniqueID)); 
        return rv;
    }

    return RV_OK;
}
#endif /* #ifdef RV_SIGCOMP_ON */ 

#endif /*#ifndef RV_SIP_LIGHT*/

