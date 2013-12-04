/*
*********************************************************************************
*                                                                               *
* NOTICE:                                                                       *
* This document contains information that is confidential and proprietary to    *
* RADVision LTD.. No part of this publication may be reproduced in any form     *
* whatsoever without written prior approval by RADVision LTD..                  *
*                                                                               *
* RADVision LTD. reserves the right to revise this publication and f changes *
* without obligation to notify any person of such revisions or changes.         *
* Copyright RADVision 1996.                                                     *
* Last Revision: Jan. 2000                                                      *
*********************************************************************************
*/


/*********************************************************************************
 *                              <RvSipTransaction.c>
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

#include "TransactionControl.h"
#include "_SipTransaction.h"
#include "_SipTransactionMgr.h"
#include "_SipTransmitter.h"
#include "TransactionState.h"
#include "TransactionMgrObject.h"
#include "TransactionObject.h"
#include "TransactionTransport.h"
#include "TransactionAuth.h"
#include "AdsRpool.h"
#include "_SipCommonUtils.h"
#include "RvSipPartyHeader.h"
#include "RvSipViaHeader.h"
#include "RvSipMsg.h"
#include "TransactionTimer.h"
#include "_SipPartyHeader.h"
#include "RvSipAddress.h"
#include "TransactionControl.h"
#include "RvSipCSeqHeader.h"
#include "_SipMsg.h"
#include "_SipCSeqHeader.h"
#include "_SipAddress.h"
#include "_SipTransport.h"
#include "_SipCommonCSeq.h"
#include "RvSipCompartmentTypes.h"
#include "RvSipTransmitter.h"
#include "_SipHeader.h"
#include "RvSipCompartment.h"
#include "_SipCommonConversions.h"
#ifdef RV_SIP_IMS_ON
#include "TransactionSecAgree.h"
#endif /*#ifdef RV_SIP_IMS_ON*/

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/
#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT
#define TRANSC_REPROCESSING_STR      "Reprocessing"
#define TRANSC_OBJECT_NAME_STR       "Transaction"
#endif /*RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT*/


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

#ifdef RV_SIP_AUTH_ON
static RvStatus RVCALLCONV RespondUnauthenticated(
                                                   IN  RvSipTranscHandle    hTransc,
                                                   IN  RvUint16            responseCode,
                                                   IN  RvChar              *strReasonPhrase,
                                                   IN  RvSipHeaderType      headerType,
                                                   IN  void*                hHeader,
                                                   IN  RvChar              *realm,
                                                   IN  RvChar              *domain,
                                                   IN  RvChar              *nonce,
                                                   IN  RvChar              *opaque,
                                                   IN  RvBool              stale,
                                                   IN  RvSipAuthAlgorithm   eAlgorithm,
                                                   IN  RvChar              *strAlgorithm,
                                                   IN  RvSipAuthQopOption   eQop,
                                                   IN  RvChar              *strQop);
#endif /* #ifdef RV_SIP_AUTH_ON */

static RvBool RVCALLCONV IsStateToSendMsg(Transaction *pTransc);

static RvStatus RVCALLCONV SendSpecificMethod( IN RvSipTranscHandle              hTransc,
                                               IN RvSipTransactionStrParamsCfg*  pTranscParams,
                                               IN RvInt32                        sizeofCfg,
                                               IN RvChar*                        strMethod,
                                               IN RvSipAddressHandle             hRequestUri);

static RvBool TransactionBlockTermination(IN Transaction        *pTransc);
/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/



/***************************************************************************
 * RvSipTransactionMgrSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets a set of event handlers identified with the application.
 * Return Value: RV_ERROR_NULLPTR - The pointer to the handlers struct is NULL.
 *                 RV_OK - The event handlers were successfully set.
 *                 RV_ERROR_BADPARAM - Size of struct is negative,
 *                                       or owner enumeration is UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscMgr - A handle to the transaction manager to set the
 *                       handlers to.
 *            pAppContext - An application context. This context will be supplied
 *                        with the callback informing of the creation of a
 *                        new transaction.
 *          pHandlers  - The struct of the event handlers.
 *          evHandlerStructSize - The size of the pHandlers struct.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionMgrSetEvHandlers(
                        IN RvSipTranscMgrHandle            hTranscMgr,
                        IN void                            *pAppContext,
                        IN RvSipTransactionEvHandlers      *pHandlers,
                        IN RvUint32                        evHandlerStructSize)
{
    TransactionMgr      *pTranscMgr;
    RvLogSource*   logId;

    RvInt32 minStructSize = (evHandlerStructSize < sizeof(RvSipTransactionEvHandlers)) ?
                              evHandlerStructSize : sizeof(RvSipTransactionEvHandlers);

    pTranscMgr = (TransactionMgr *)hTranscMgr;

    if ((NULL == hTranscMgr) ||
        (NULL == pHandlers) )
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

    /* The manager signs in is the application manager:
       Set application's calbacks*/
    memset(&(pTranscMgr->pAppEvHandlers), 0, sizeof(RvSipTransactionEvHandlers));
    memcpy(&(pTranscMgr->pAppEvHandlers), pHandlers, minStructSize);

    /* Set the application manager to the transaction manager */
    pTranscMgr->pAppMgr = pAppContext;
    RvLogDebug(logId ,(logId ,
              "RvSipTransactionMgrSetEvHandlers - application event handlers were successfully set to the transaction manager"));
    RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionMgrSetMgrEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets a set of event handlers the transaction manager will use.
 *          Transaction manager events do not belong to a specific transaction
 *          but to the manager itself.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscMgr - A handle to the transaction manager.
 *            pAppContext - An application context. This context will be supplied
 *                        when the manager callbacks will be called.
 *            pMgrHandlers  - A pointer to a structure of event handlers.
 *            evHandlerStructSize - The size of the pMgrHandlers structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionMgrSetMgrEvHandlers(
                        IN RvSipTranscMgrHandle            hTranscMgr,
                        IN void                            *pAppContext,
                        IN RvSipTransactionMgrEvHandlers   *pMgrHandlers,
                        IN RvUint32                        evHandlerStructSize)
{
    TransactionMgr*   pTranscMgr;
    RvLogSource*      logId;

    RvInt32 minStructSize = (evHandlerStructSize < sizeof(RvSipTransactionMgrEvHandlers)) ?
                             evHandlerStructSize : sizeof(RvSipTransactionMgrEvHandlers);

    pTranscMgr = (TransactionMgr *)hTranscMgr;

    if ((NULL == hTranscMgr) ||
        (NULL == pMgrHandlers) )
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

    /* The manager signs in is the application manager:
       Set application's calbacks*/
    memset(&(pTranscMgr->pMgrEvHandlers), 0, sizeof(RvSipTransactionMgrEvHandlers));
    memcpy(&(pTranscMgr->pMgrEvHandlers), pMgrHandlers, minStructSize);

    /* Set the application manager to the transaction manager */
    pTranscMgr->pAppMgr = pAppContext;
    RvLogDebug(logId ,(logId ,
              "RvSipTransactionMgrSetMgrEvHandlers - Transc manager event handlers were successfully set to the transaction manager"));
    RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    return RV_OK;
}


/***************************************************************************
 * RvSipTranscMgrCreateTransaction
 * ------------------------------------------------------------------------
 * General: Called to create a new Uac transaction.
 * The transaction's state will be set to "Initial" state.
 * The default response-code will be set to 0.
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
 *                                     Another possibility is that the transaction
 *                                     manager is full. In that case the
 *                                     transaction will not be created and this
 *                                     value is returned.
 *               RV_ERROR_UNKNOWN - A transaction with the given key already exists.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr - The transaction manager to which this transaction will
 *                         belong.
 *          pOwner - The application owner of the transaction.
 * Output:  phTransaction - The new transaction created.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscMgrCreateTransaction(
                                  IN  RvSipTranscMgrHandle     hTranscMgr,
                                  IN  RvSipTranscOwnerHandle   hTranscOwner,
                                  OUT RvSipTranscHandle        *hTransc)
{
    Transaction        *pTransc;
    TransactionMgr    *pTranscMgr;
    RvStatus          rv;
    RvLogSource*      logId;

    if (hTranscMgr == NULL || NULL == hTransc)
    {
        return RV_ERROR_NULLPTR;
    }
    *hTransc = NULL;
    pTranscMgr = (TransactionMgr*)hTranscMgr;

    RvMutexLock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);

    logId = ((TransactionMgr*)hTranscMgr)->pLogSrc;
    /*---------------------------------------
       Create a new transaction
      ---------------------------------------*/
    rv = TransactionMgrCreateTransaction(pTranscMgr,RV_TRUE,
                                        (RvSipTranscHandle *)&pTransc);
    if (rv != RV_OK)
    {
        RvLogError(logId ,(logId ,
                  "RvSipTranscMgrCreateTransaction - Failed to create a new transaction (rv=%d)",rv));
        RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
        return rv;
    }

    /* Initiate the transaction's owner relevant parameters */
    rv = TransactionInitiateOwner(pTransc, (void*)hTranscOwner,
                                  SIP_TRANSACTION_OWNER_APP, NULL);
    if (RV_OK != rv)
    {
        TransactionDestruct(pTransc);
        RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
        return rv;
    }

    *hTransc = (RvSipTranscHandle)pTransc;

    /* The determine whether is a proxy transaction or not by the configuration
       of the transaction manager */
    pTransc->bIsProxy = pTranscMgr->isProxy;

    RvLogInfo(logId ,(logId ,
              "RvSipTranscMgrCreateTransaction - New transaction 0x%p was created",
              pTransc));

    RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    return RV_OK;
}


/***************************************************************************
 * RvSipTranscMgrCreateServerTransactionFromMsg
 * ------------------------------------------------------------------------
 * General: Called to create a new Server transaction from a request message that
 *          the application supplies. This function creates a new transaction in the
 *          Request-Receaved state.
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
 *                                     Another possibility is that the transaction
 *                                     manager is full. In that case the
 *                                     transaction will not be created and this
 *                                     value is returned.
 *               RV_ERROR_UNKNOWN - A transaction with the given key already exists.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr - The transaction manager to which this transaction will
 *                         belong.
 *          hTranscOwner - The application owner of the transaction.
 *          hMsg - The handle to the request message that the application
 *                 supplies to create the transaction
 * Output:  hTransc - The new transaction created. If the function failed hTransc = NULL
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscMgrCreateServerTransactionFromMsg(
                                              IN  RvSipTranscMgrHandle     hTranscMgr,
                                              IN  RvSipTranscOwnerHandle   hTranscOwner,
                                              IN  RvSipMsgHandle           hMsg,
                                              OUT RvSipTranscHandle        *hTransc)
{
    Transaction       *pTransc = NULL;
    TransactionMgr    *pTranscMgr;
    RvStatus          rv;
    RvLogSource*        logId;
    RvSipMsgType       eType;
    RvSipMethodType    eMethod;

    if (hTranscMgr == NULL || NULL == hTransc || hMsg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    pTranscMgr = (TransactionMgr*)hTranscMgr;
    *hTransc = NULL;

    logId = ((TransactionMgr*)hTranscMgr)->pLogSrc;

    /* Check that the message is a request that is not ACK */
    eType = RvSipMsgGetMsgType(hMsg);

    if(eType != RVSIP_MSG_REQUEST)
    {
        RvLogError(logId ,(logId ,
                  "RvSipTranscMgrCreateServerTransactionFromMsg - message is not a request"));
        *hTransc = NULL;
        return RV_ERROR_ILLEGAL_ACTION;
    }

    eMethod = RvSipMsgGetRequestMethod(hMsg);

    if(eMethod == RVSIP_METHOD_ACK)
    {
        RvLogError(logId ,(logId ,
                  "RvSipTranscMgrCreateServerTransactionFromMsg - message is an ACK request"));
        *hTransc = NULL;
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = SipTransactionMgrCreateServerTransactionFromMsg(pTranscMgr,
                                                         hTranscOwner,
                                                         hMsg,
                                                         &pTransc);

    if(rv != RV_OK)
    {
        RvLogError(logId ,(logId ,
                  "RvSipTranscMgrCreateServerTransactionFromMsg - Failed to create a new transaction (rv=%d)",rv));
        *hTransc = NULL;
        return rv;
    }

    *hTransc = (RvSipTranscHandle)pTransc;

    RvLogInfo(logId ,(logId ,
              "RvSipTranscMgrCreateServerTransactionFromMsg - New transaction 0x%p was created",
              pTransc));

    return RV_OK;
}




/***************************************************************************
 * RvSipTranscMgrSetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: Sets a handle to the application transc manager in the stack
 *          transaction manager. The application can use this handle to
 *          save a context in the transaction manager. This handle is given in
 *          transaction created callback.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr - The SIP stack transaction manager handle
 *            hAppMgr - The application manager handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscMgrSetAppMgrHandle(
                                  IN  RvSipTranscMgrHandle     hTranscMgr,
                                  IN  void*                    hAppMgr)
{

    TransactionMgr    *pTranscMgr;

    if (hTranscMgr == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    pTranscMgr = (TransactionMgr*)hTranscMgr;
    RvMutexLock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    pTranscMgr->pAppMgr = hAppMgr;

    RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    return RV_OK;

}
/***************************************************************************
 * RvSipTranscMgrGetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: Gets the handle to the application transaction manager from the stack
 *          transaction manager.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr - The SIP stack transaction manager handle
 * Output:  hAppMgr - The application manager handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscMgrGetAppMgrHandle(
                                  IN   RvSipTranscMgrHandle     hTranscMgr,
                                  OUT  void**                   hAppMgr)
{
    TransactionMgr    *pTranscMgr;

    if (hTranscMgr == NULL || hAppMgr == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    pTranscMgr = (TransactionMgr*)hTranscMgr;
    RvMutexLock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    *hAppMgr = pTranscMgr->pAppMgr;

    RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    return RV_OK;

}

/***************************************************************************
 * RvSipTranscMgrSendOutOfContextMsg
 * ------------------------------------------------------------------------
 * General: This function is deprecated. For sending messages that are not
 *          related to a specific transaction, please use the Transmitter API.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTranscMgr              - The transaction manager handle.
 *          hMsgToSend              - Handle to the message to send.
 *          handleTopVia            - Indicate whether the transaction manager should
 *                                    add a top Via header to request messages and
 *                                    remove the top via from response messages.
 *          strViaBranch            - The Via branch to add to the top via header.
 *                                    This parameter is ignored for response messages or when
 *                                    the handleTopVia parameter is RV_FALSE.
 *          localAddress            - The local address to use when sending the message. If
 *                                    NULL is given the default local address will be used.
 *          localPort               - The local port to use when sending the message.
 *          bIgnoreOutboundProxy    -
 *          bSendToFirstRouteHeader       - Determines weather to send a request to to a loose router
 *                                    (to the first in the route list) or to a strict router
 *                                    (to the request URI). When the message sent is a response
 *                                    this parameter is ignored.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscMgrSendOutOfContextMsg(
                                    IN RvSipTranscMgrHandle            hTranscMgr,
                                    IN RvSipMsgHandle                  hMsgToSend,
                                    IN RvBool                         handleTopVia,
                                    IN RvChar                        *strViaBranch,
                                    IN RvChar                        *strLocalAddress,
                                    IN RvUint16                       localPort,
#if defined(UPDATED_BY_SPIRENT)
                                    IN RvChar                         *if_name,
				    IN RvUint16                       vdevblock,
#endif
                                    IN RvBool                         bIgnoreOutboundProxy,
                                    IN RvBool                         bSendToFirstRouteHeader)

{

    RvStatus rv = RV_OK;
    TransactionMgr      *pTranscMgr;
    pTranscMgr = (TransactionMgr *)hTranscMgr;

    if (NULL == hTranscMgr || hMsgToSend == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    RvLogInfo(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
              "RvSipTranscMgrSendOutOfContextMsg - TranscMgr= 0x%p sending message=0x%p HandleTopVia= %d ViaBranch=%s local addr= %s port= %d",
              pTranscMgr, hMsgToSend, handleTopVia,
              (strViaBranch==NULL)?"NULL":strViaBranch,
              (strLocalAddress==NULL)?"NULL":strLocalAddress, localPort));


    rv = TransactionTransportSendOutofContextMsg(pTranscMgr,hMsgToSend,handleTopVia,
                                                 strViaBranch,NULL,NULL,
                                                 strLocalAddress,localPort,
#if defined(UPDATED_BY_SPIRENT)
						 if_name,
						 vdevblock,
#endif
                                                 bIgnoreOutboundProxy,
                                                 bSendToFirstRouteHeader,RV_TRUE);
    if(RV_OK != rv)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "RvSipTranscMgrSendOutOfContextMsg - TransactionTransportSendOutofContextMsg Failed (rv=%d)",rv));
        return rv;
    }

    return RV_OK;

}

/***************************************************************************
 * RvSipTranscMgrSendOutOfContextMsgExt
 * ------------------------------------------------------------------------
 * General: This function is deprecated. For sending messages that are not
 *          related to a specific transaction, please use the Transmitter API.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTranscMgr           - The transaction manager handle.
 *          hMsgToSend           - Handle to the message to send.
 *          handleTopVia         - Indicate whether the transaction manager should
 *                                 add a top Via header to request messages and
 *                                 remove the top via from response messages.
 *          strViaBranch         - The Via branch to add to the top via header.
 *                                 This parameter is ignored for response messages or when
 *                                 the handleTopVia parameter is RV_FALSE.
 *          strLocalAddress      - The local address to use when sending the message. If
 *                                 NULL is given the default local address will be used.
 *          localPort            - The local port to use when sending the message.
 *          bIgnoreOutboundProxy - Indicate the proxy whether to ignore an
 *                                 outbound proxy.Set this parameter to RV_TRUE only
 *                                 If a Route Header was used as a request uri.
 *          bSendToFirstRouteHeader    - Determines weather to send a request to to a loose router
 *                                 (to the first in the route list) or to a strict router
 *                                 (to the request URI). When the message sent is a response
 *                                 this parameter is ignored.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscMgrSendOutOfContextMsgExt(
                                    IN RvSipTranscMgrHandle           hTranscMgr,
                                    IN RvSipMsgHandle                 hMsgToSend,
                                    IN RvBool                         handleTopVia,
                                    IN RPOOL_Ptr                     *pViaBranch,
                                    IN RvChar                        *strLocalAddress,
                                    IN RvUint16                       localPort,
#if defined(UPDATED_BY_SPIRENT)
				    IN RvChar                         *if_name,
				    IN RvUint16                       vdevblock,
#endif
                                    IN RvBool                         bIgnoreOutboundProxy,
                                    IN RvBool                         bSendToFirstRouteHeader)
{


    RvStatus                      rv         = RV_OK;
    TransactionMgr*               pTranscMgr = (TransactionMgr *)hTranscMgr;

    if (NULL == hTranscMgr || hMsgToSend == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    RvLogInfo(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
              "RvSipTranscMgrSendOutOfContextMsgExt - TranscMgr= 0x%p sending message=0x%p HandleTopVia= %d local addr= %s port= %d",
              pTranscMgr, hMsgToSend, handleTopVia,
              (strLocalAddress==NULL)?"NULL":strLocalAddress, localPort));

    rv = TransactionTransportSendOutofContextMsg(pTranscMgr,hMsgToSend,handleTopVia,
                                                 NULL,pViaBranch,NULL,
                                                 strLocalAddress,localPort,
#if defined(UPDATED_BY_SPIRENT)
						 if_name,
						 vdevblock,
#endif
                                                 bIgnoreOutboundProxy,
                                                 bSendToFirstRouteHeader,RV_TRUE);
    if(RV_OK != rv)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
            "RvSipTranscMgrSendOutOfContextMsgExt - TransactionTransportSendOutofContextMsg Failed (rv=%d)",rv));
        return rv;
    }

    return RV_OK;

}





/***************************************************************************
 * RvSipTranscMgrGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this transaction
 *          manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscMgr      - Handle to the transaction manager.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscMgrGetStackInstance(
                                   IN   RvSipTranscMgrHandle   hTranscMgr,
                                   OUT  void**                 phStackInstance)
{
    TransactionMgr    *pTranscMgr;

    if (NULL == phStackInstance)
    {
        return RV_ERROR_NULLPTR;
    }
    *phStackInstance = (void*)NULL;

    if (hTranscMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pTranscMgr = (TransactionMgr*)hTranscMgr;
    RvMutexLock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);

    *phStackInstance = (void*)pTranscMgr->hStack;

    RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    return RV_OK;

}

#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT
/***************************************************************************
 * RvSipTranscMgrReprocessTransaction
 * ------------------------------------------------------------------------
 * General: Reprocess a transaction: The transaction manager will terminate
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
 * Input:   hTranscMgr - Handle to the transaction manager.
 *          hTransc    - Handle to the transaction to be reprocessed.
 *
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscMgrReprocessTransaction(
											IN   RvSipTranscMgrHandle   hTranscMgr,
											IN   RvSipTranscHandle      hTransc)
{
    TransactionMgr                   *pTranscMgr = (TransactionMgr*)hTranscMgr;
    Transaction                      *pTransc = (Transaction*)hTransc;
	static RvInt32                    unusedInt = 0;

	if (hTranscMgr == NULL || hTransc == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* Lock the old transaction in order to load its data */
	if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (pTransc->eState != RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD &&
		pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD)
	{
		RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
				   "RvSipTranscMgrReprocessTransaction - Reprocessing cannot occur in state other than REQUEST_RCVD. Transaction=0x%p, State=%s", hTransc, TransactionGetStateName(pTransc->eState)));
		TransactionUnLockAPI(pTransc);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	/* unlock the old transaction */
	TransactionUnLockAPI(pTransc);

	RvLogInfo(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
			  "RvSipTranscMgrReprocessTransaction - Inserting reprocess transaction request to the event queue. Transaction=0x%p", hTransc));

	return  SipTransportSendTerminationObjectEvent(pTranscMgr->hTransport,
									    (void *)hTransc,
										&(pTransc->reprocessingInfo),
                                        (RvInt32)unusedInt,
                                        SipTransactionMgrReprocessTransactionEv,
										RV_TRUE,
										TRANSC_REPROCESSING_STR,
                                        TRANSC_OBJECT_NAME_STR);
}
#endif /* #ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */

/*----------------------------------------------------------------------------
        TRANSACTION CONTROL
  ----------------------------------------------------------------------------*/

#ifdef SUPPORT_EV_HANDLER_PER_OBJ
/***************************************************************************
 * RvSipTransactionSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets a set of event handlers identified with the application for a certain transaction.
 * Return Value:   RV_OK - The event handlers were successfully set otherwise error occurred.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:		hTransc    - A handle to the transaction to set the handlers to.
 *				pHandlers  - The struct of the event handlers.
 *				evHandlerStructSize - The size of the pHandlers struct.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetEvHandlers(
                        IN RvSipTranscHandle	            hTransc,
                        IN RvSipTransactionEvHandlers      *pHandlers,
                        IN RvUint32                        evHandlerStructSize)
{
    Transaction     *pTransc;
    RvLogSource*	logId;
	RvStatus		rv;

    RvInt32 minStructSize = (evHandlerStructSize < sizeof(RvSipTransactionEvHandlers)) ?
                              evHandlerStructSize : sizeof(RvSipTransactionEvHandlers);

    pTransc = (Transaction*)hTransc;

    if ((NULL == hTransc) ||
        (NULL == pHandlers) )
    {
        return RV_ERROR_NULLPTR;
    }

	rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	logId = pTransc->pMgr->pLogSrc;
	
	/*We can set proprietary event handlers to transaction that has no owner or the owner is the application.*/
	if (pTransc->eOwnerType != SIP_TRANSACTION_OWNER_APP && 
		pTransc->eOwnerType != SIP_TRANSACTION_OWNER_NON)
	{
		RvLogError(logId ,(logId ,
              "RvSipTransactionSetEvHandlers - Transc (0x%p) is not owned by application, unable to set object EvHandler.", pTransc));
		TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;
	}

    

    if (evHandlerStructSize == 0)
    {
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_BADPARAM;
    }

    /* The manager signs in is the application manager:
       Set application's calbacks*/
    memset(&(pTransc->objEvHandlers), 0, sizeof(RvSipTransactionEvHandlers));
    memcpy(&(pTransc->objEvHandlers), pHandlers, minStructSize);

	/*Since the application set a proprietary event handler for this transaction we need to 
	  point to this new event handler.*/
	pTransc->pEvHandlers = &pTransc->objEvHandlers;

    
    RvLogDebug(logId ,(logId ,
              "RvSipTransactionSetEvHandlers - application event handlers were successfully set to the transaction 0x%p", pTransc));
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}

#endif /*SUPPORT_EV_HANDLER_PER_OBJ*/

/***************************************************************************
 * RvSipTransactionMake
 * ------------------------------------------------------------------------
 * General: Initialize a transaction and sends a request. before calling
 *          this function you should call RvSipTranscMgrCreateTransaction
 *          to create a new transaction object. A call-id will
 *          be created automatically by the transaction layer. You can
 *          use RvSipTransactionSetCallId before calling this function
 *          to specify a different Call-ID.
 * Return Value: RV_OK - The out parameter pTransc now points
 *                            to a valid transaction, initialized by the
 *                              received key.
 *                 RV_ERROR_NULLPTR - At least one of the pointers to the
 *                                 transaction key values is a NULL pointer,
 *                                 or the pointer to the transaction pointer is a
 *                                 NULL pointer.
 *               RV_ERROR_OUTOFRESOURCES - Couldn't allocate memory for the
 *                                     transaction object.
 *                                     Another possibility is that the transaction
 *                                     manager is full. In that case the
 *                                     transaction will not be created and this
 *                                     value is returned.
 *               RV_ERROR_UNKNOWN - A transaction with the given key already exists.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc   - The transaction handle.
 *          strFrom   - A string with the from party header for example:
 *                     "From:sip:172.20.1.1:5060"
 *          strTo     - A string with the to party header for example:
 *                     "To:sip:172.20.5.5:5060"
 *          strRequestURI - A string with the request uri address. for example:
 *                     "sip:sara@172.20.1.1:5060"
 *          cseqStep  - The CSeq step to use in this transaction.
 *          strMethod - The request method as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionMake(
                                  IN  RvSipTranscHandle       hTransc,
                                  IN  RvChar*                strFrom,
                                  IN  RvChar*                strTo,
                                  IN  RvChar*                strRequestURI,
                                  IN  RvInt32                cseqStep,
                                  IN  RvChar*                strMethod)
{

    RvStatus  rv;

    RvSipPartyHeaderHandle  hTo;
    RvSipPartyHeaderHandle  hFrom;
    RvSipAddressHandle      hRequestUri;
    RvSipAddressType        eStrAddr = RVSIP_ADDRTYPE_URL;

    Transaction    *pTransc = (Transaction*)hTransc;

    if (NULL == pTransc || strRequestURI == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
    {
        TransactionUnLockAPI(pTransc);
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "RvSipTransactionMake - Failed, Transaction 0x%p: in state Terminated",pTransc));
        return RV_ERROR_INVALID_HANDLE;
    }

	if (cseqStep != UNDEFINED) 
	{
		rv = RvSipTransactionSetCSeqStep(hTransc,cseqStep);
		if (RV_OK != rv)
		{
			TransactionUnLockAPI(pTransc);
			RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
				"RvSipTransactionMake - Failed, Transaction 0x%p: in RvSipTransactionSetCSeqStep",pTransc));
			return rv;
		}
	}
    	
    if(strFrom != NULL)
    {
        rv = RvSipTransactionGetNewPartyHeaderHandle(hTransc,&hFrom);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "RvSipTransactionMake - Transaction 0x%p: Failed to allocate from party header (rv=%d)",
                pTransc, rv));
            TransactionUnLockAPI(pTransc);
            return rv;

        }
        rv = RvSipPartyHeaderParse(hFrom,RV_FALSE,strFrom);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "RvSipTransactionMake - Transaction 0x%p: Failed to parse from header - bad syntax (rv=%d)",
                pTransc, rv));
            TransactionUnLockAPI(pTransc);
            return rv;
        }
        rv = RvSipTransactionSetFromHeader(hTransc,hFrom);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "RvSipTransactionMake - Transaction 0x%p: Failed to set from header (rv=%d)",
                pTransc,rv));
            TransactionUnLockAPI(pTransc);
            return rv;
        }
    }

    if(strTo != NULL)
    {
        rv = RvSipTransactionGetNewPartyHeaderHandle(hTransc,&hTo);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "RvSipTransactionMake - Transaction 0x%p: Failed to allocate to party header (rv=%d)",
                pTransc, rv));
            TransactionUnLockAPI(pTransc);
            return rv;

        }
        rv = RvSipPartyHeaderParse(hTo,RV_TRUE,strTo);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "RvSipTransactionMake - Transaction 0x%p: Failed to parse to header - bad syntax (rv=%d)",
                pTransc,rv));
            TransactionUnLockAPI(pTransc);
            return rv;
        }
        rv = RvSipTransactionSetToHeader(hTransc,hTo);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "RvSipTransactionMake - Transaction 0x%p: Failed to set To header (rv=%d)",
                pTransc,rv));
            TransactionUnLockAPI(pTransc);
            return rv;
        }
    }

    /* Set Request-URI */
    /* get address type from the request uri string */
    eStrAddr = SipAddrGetAddressTypeFromString(pTransc->pMgr->hMsgMgr, strRequestURI);
    rv = RvSipTransactionGetNewAddressHandle(hTransc,eStrAddr, &hRequestUri);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "RvSipTransactionMake - Transaction 0x%p: Failed to allocate request uri address (rv=%d)",
            pTransc,rv));
        TransactionUnLockAPI(pTransc);
        return rv;
        
    }
    rv = RvSipAddrParse(hRequestUri,strRequestURI);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "RvSipTransactionMake - Transaction 0x%p: Failed to parse request uri - bad syntax (rv=%d)",
            pTransc,rv));
        TransactionUnLockAPI(pTransc);
        return rv;
    }
    rv = RvSipTransactionRequest(hTransc,strMethod,hRequestUri);

    TransactionUnLockAPI(pTransc);
    return rv;
}




/***************************************************************************
 * RvSipTransactionRequest
 * ------------------------------------------------------------------------
 * General: Creates and sends a request message according to the given method,
 *          with the given address as a Request-Uri.
 * Return Value: RV_OK - The message was created and sent successfully.
 *                 RV_ERROR_NULLPTR - The Request-Uri pointer is a NULL pointer,
 *                                 or the transaction pointer is a NULL pointer.
 *                 RV_ERROR_ILLEGAL_ACTION - The Request was called in a state where
 *                                    request can not be executed, or with
 *                                  invalid string method.
 *                 RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -   The transaction this request refers to.
 *            strRequestMethod - The request method string.
 *            hRequestUri -  The Request-Uri to be used in the request
 *                           message.
 *                           The message's Request-Uri will be the address
 *                           given here, and the message will be sent
 *                           accordingly.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionRequest(
                           IN RvSipTranscHandle           hTransc,
                           IN RvChar                     *strRequestMethod,
                           IN RvSipAddressHandle          hRequestUri)
{
    Transaction               *pTransc = (Transaction *)hTransc;
    RvStatus                 retStatus;

    SipTransactionMethod      eMethod;

    if (pTransc == NULL  || strRequestMethod == NULL || hRequestUri==NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "RvSipTransactionRequest - Transc 0x%p: about to send a request with method %s",
              pTransc,strRequestMethod));


    /* check the legality of the request */

    if (pTransc->eState != RVSIP_TRANSC_STATE_IDLE)
    {
        /* A request is allowed only in an Idle state */
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionRequest - Transc 0x%p: cannot Request in non Idle state. state=%s",
                  pTransc,TransactionGetStateName(pTransc->eState)));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    eMethod = TransactionGetEnumMethodFromString(strRequestMethod);
    if(eMethod == SIP_TRANSACTION_METHOD_UNDEFINED)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                    "RvSipTransactionRequest - Transaction 0x%p: Method is invalid", pTransc));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_BADPARAM;
    }

    /*generate call-id if needed*/
    if(pTransc->strCallId.offset == UNDEFINED)
    {
        retStatus = TransactionGenerateCallId(pTransc);
        if (RV_OK != retStatus)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                        "RvSipTransactionRequest - Transaction 0x%p: Failed to generate call-id (rv=%d)",
                       pTransc, retStatus));
            TransactionUnLockAPI(pTransc);
            return retStatus;
        }
    }

    if (SIP_TRANSACTION_METHOD_OTHER != eMethod)
    {
        /* The method is an enumeration*/
        retStatus = SipTransactionRequest(hTransc, eMethod, hRequestUri);
        TransactionUnLockAPI(pTransc);
        return retStatus;
    }


    /*the method is a string - set it in the transction*/
    pTransc->eMethod = SIP_TRANSACTION_METHOD_OTHER;
    retStatus = TransactionSetMethodStr(pTransc,strRequestMethod);
    if (RV_OK != retStatus)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionRequest - Transaction 0x%p: Failed to set method in transaction (rv=%d)",
                  pTransc, retStatus));
        TransactionUnLockAPI(pTransc);
        return retStatus;
    }

    retStatus = TransactionControlSendRequest(pTransc, hRequestUri,RV_TRUE);

    if (RV_OK != retStatus)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionRequest - Transaction 0x%p: Error trying to request (rv=%d)",
                  pTransc, retStatus));
        TransactionUnLockAPI(pTransc);
        return retStatus;
    }
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionSendPUBLISH
 * ------------------------------------------------------------------------
 * General: Sends a PUBLISH request over a given transaction.
 *          The function may use given parameters in a string format, or application 
 *          may supply NULL as the parameters struct, so the function will use 
 *          parameters that were set to the transaction in advance.
 * Return Value: RV_OK - The message was created and sent successfully.
 *               RV_ERROR_NULLPTR - The Request-Uri pointer is a NULL pointer,
 *                                  or the transaction pointer is a NULL pointer.
 *               RV_ERROR_ILLEGAL_ACTION - The Request was called in a state where
 *                                  request can not be executed, or with
 *                                  invalid string method.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                                  message (Couldn't send a message to the given
 *                                  Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc       - The transaction this request refers to.
 *            pTranscParams - Structure containing the transaction parameters
 *                        (Request-Uri, To, From, Cseq val) in a string format,
 *                        to be used in the request message.
 *                        if NULL - the function will use parameters that were 
 *                        set to the transaction in advance.
 *            sizeofCfg     - The size of the given parameters structure.
 *            hRequestUri   - In case the given parameters structure is NULL,
 *                        the application need to supply the request-uri handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSendPUBLISH(
                           IN RvSipTranscHandle              hTransc,
                           IN RvSipTransactionStrParamsCfg*  pTranscParams,
                           IN RvInt32                        sizeofCfg,
                           IN RvSipAddressHandle             hRequestUri)
{
    return SendSpecificMethod(hTransc, pTranscParams, sizeofCfg, "PUBLISH", hRequestUri);
}
/***************************************************************************
 * RvSipTransactionSendMESSAGE
 * ------------------------------------------------------------------------
 * General: Sends a MESSAGE request over a given transaction.
 *          The function may use given parameters in a string format, or application 
 *          may supply NULL as the parameters struct, so the function will use 
 *          parameters that were set to the transaction in advance.
 * Return Value: RV_OK - The message was created and sent successfully.
 *               RV_ERROR_NULLPTR - The Request-Uri pointer is a NULL pointer,
 *                                  or the transaction pointer is a NULL pointer.
 *               RV_ERROR_ILLEGAL_ACTION - The Request was called in a state where
 *                                  request can not be executed, or with
 *                                  invalid string method.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                                  message (Couldn't send a message to the given
 *                                  Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc       - The transaction this request refers to.
 *            pTranscParams - Structure containing the transaction parameters
 *                        (Request-Uri, To, From, Cseq val) in a string format,
 *                        to be used in the request message.
 *                        if NULL - the function will use parameters that were 
 *                        set to the transaction in advance.
 *            sizeofCfg     - The size of the given parameters structure.
 *            hRequestUri   - In case the given parameters structure is NULL,
 *                        the application need to supply the request-uri handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSendMESSAGE(
                           IN RvSipTranscHandle              hTransc,
                           IN RvSipTransactionStrParamsCfg*  pTranscParams,
                           IN RvInt32                        sizeofCfg,
                           IN RvSipAddressHandle             hRequestUri)
{
    return SendSpecificMethod(hTransc, pTranscParams, sizeofCfg, "MESSAGE", hRequestUri);
}


/***************************************************************************
 * RvSipTransactionSendOPTIONS
 * ------------------------------------------------------------------------
 * General: Sends a OPTIONS request over a given transaction.
 *          The function may use given parameters in a string format, or application 
 *          may supply NULL as the parameters struct, so the function will use 
 *          parameters that were set to the transaction in advance.
 * Return Value: RV_OK - The message was created and sent successfully.
 *               RV_ERROR_NULLPTR - The Request-Uri pointer is a NULL pointer,
 *                                  or the transaction pointer is a NULL pointer.
 *               RV_ERROR_ILLEGAL_ACTION - The Request was called in a state where
 *                                  request can not be executed, or with
 *                                  invalid string method.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                                  message (Couldn't send a message to the given
 *                                  Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc       - The transaction this request refers to.
 *            pTranscParams - Structure containing the transaction parameters
 *                        (Request-Uri, To, From, Cseq val) in a string format,
 *                        to be used in the request message.
 *                        if NULL - the function will use parameters that were 
 *                        set to the transaction in advance.
 *            sizeofCfg     - The size of the given parameters structure.
 *            hRequestUri   - In case the given parameters structure is NULL,
 *                        the application need to supply the request-uri handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSendOPTIONS(
                           IN RvSipTranscHandle              hTransc,
                           IN RvSipTransactionStrParamsCfg*  pTranscParams,
                           IN RvInt32                        sizeofCfg,
                           IN RvSipAddressHandle             hRequestUri)
{
    return SendSpecificMethod(hTransc, pTranscParams, sizeofCfg, "OPTIONS", hRequestUri);
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * RvSipTransactionRespondReliable
 * ------------------------------------------------------------------------
 * General: Creates and sends a reliable provisional response message with the given
 * response code and reason phrase. This function can be called only for
 * response codes between 101 and 199. The provisional response will contain the
 * Require=100rel and the RSeq headers.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -    The transaction handle.
 *            responseCode -    The provisional response-code (between 101 and 199).
 *            strReasonPhrase - The reason phrase that describes the response code.
 *                            If NULL is supplied a default reason phrase will be
 *                            used.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionRespondReliable(
                                        IN RvSipTranscHandle   hTransc,
                                        IN RvUint16         responseCode,
                                        IN RvChar          *strReasonPhrase)
{
    Transaction        *pTransc = (Transaction *)hTransc;
    RvStatus           retStatus;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "RvSipTransactionRespondReliable - Transaction 0x%p: about to send response %u reliably",
               pTransc, responseCode));

    if ((responseCode < 101) || (responseCode >= 200))
    {
        /* The response code is out of limit */
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionRespondReliable - Transaction 0x%p: Failed, Response code must be between 101 to 199",
                   pTransc));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_BADPARAM;
    }

    if(pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD &&
       pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_PRACK_COMPLETED)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionRespondReliable - Transaction 0x%p: Failed - can't send reliable response in state %s",
                  pTransc,TransactionGetStateName(pTransc->eState)));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if(pTransc->bSending == RV_TRUE)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionRespondReliable - Transaction 0x%p: Failed - The transaction is in the middle of sending a different response",
                  pTransc));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;

    }


    pTransc->bSending = RV_TRUE;
    retStatus = TransactionControlSendResponse(pTransc,responseCode,strReasonPhrase,
                                               RVSIP_TRANSC_REASON_USER_COMMAND,
                                               RV_TRUE);
    pTransc->bSending = RV_FALSE;
    if(retStatus != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "RvSipTransactionRespondReliable - Transaction 0x%p: Failed to send response (rv=%d)",
            pTransc, retStatus));
        TransactionUnLockAPI(pTransc);
        return retStatus;
    }

    TransactionUnLockAPI(pTransc);
    return RV_OK;
}


/***************************************************************************
 * RvSipTransactionGet100RelStatus
 * ------------------------------------------------------------------------
 * General: Return the 100rel option tag status of a received Invite request.
 *          A transaction can get a request message with a Supported=100rel
 *          or Require=10rel headers. These headers indicate whether a
 *          a reliable provisional response should be sent.
 *          The function can be used only for server invite transactions.
 *          For any other transaction RVSIP_TRANSC_100_REL_UNDEFINED will be
 *          returned.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -    The transaction handle.
 * Output:  relStatus - The reliable status received in the INVITE request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGet100RelStatus(
                                    IN RvSipTranscHandle              hTransc,
                                    OUT RvSipTransaction100RelStatus  *relStatus)
{
    RvStatus  rv;

    Transaction    *pTransc = (Transaction*)hTransc;
    if(pTransc == NULL || relStatus == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    *relStatus = pTransc->relStatus;


    TransactionUnLockAPI(pTransc);
    return RV_OK;

}

/***************************************************************************
 * RvSipTransactionIsUnsupportedExtRequired
 * ------------------------------------------------------------------------
 * General: Returns whether the transaction require an extensions that is
 *          not supported. You can call this function in states that indicates
 *          that a request was receives.
 *          You can use this function only if the rejectUnsupportedExtensions
 *          configuration parameter is set to RV_TRUE.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle.
 * Output:  bUnsupportedReq - RV_TRUE if the request requires unsupported
 *                            extension. RV_FALSE otherwise.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionIsUnsupportedExtRequired(
                                 IN  RvSipTranscHandle hTransc,
                                 OUT RvBool           *bUnsupportedReq)
{
    RvStatus  rv;

    Transaction    *pTransc = (Transaction*)hTransc;
    if(pTransc == NULL || bUnsupportedReq == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *bUnsupportedReq = RV_FALSE;
    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    if(pTransc->unsupportedList != UNDEFINED)
    {
        *bUnsupportedReq = RV_TRUE;
    }
    TransactionUnLockAPI(pTransc);
    return RV_OK;

}

#endif /*#ifndef RV_SIP_PRIMITIVES*/
/***************************************************************************
 * RvSipTransactionIgnoreOutboundProxy
 * ------------------------------------------------------------------------
 * General:  Instructs the transaction to ignore its outbound proxy when sending
 *           requests.
 *           In some cases you will want the transaction to ignore its outbound
 *           proxy even if it is configured to use one.
 *           An example is when the request uri was calculated from a Route
 *           header that was found in the message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle.
 *          bIgnoreOutboundProxy - RV_TRUE if you wish the transaction to ignored
 *          its configured outbound proxy, RV_FALSE otherwise.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionIgnoreOutboundProxy(
                                 IN  RvSipTranscHandle hTransc,
                                 IN  RvBool           bIgnoreOutboundProxy)
{
    RvStatus  rv;

    Transaction    *pTransc = (Transaction*)hTransc;
    if(pTransc == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    rv = RvSipTransmitterSetIgnoreOutboundProxyFlag(pTransc->hTrx,bIgnoreOutboundProxy);

    TransactionUnLockAPI(pTransc);
    return rv;
}

/***************************************************************************
 * RvSipTransactionRespond
 * ------------------------------------------------------------------------
 * General: Creates and sends a response message with the given response code
 * and reason phrase.
 * Return Value: RV_OK - The response was successfully created and sent.
 *                 RV_ERROR_NULLPTR - The pointer to the transaction is a NULL
 *                                 pointer.
 *                 RV_ERROR_ILLEGAL_ACTION - The Respond was called in a state where
 *                                    respond can not be executed.
 *                 RV_ERROR_UNKNOWN - an error occurred while trying to send the
 *                              response message.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -    The transaction which the response belongs to.
 *            responseCode -    The response-code to respond with.
 *            strReasonPhrase - The reason phrase to describe the response code.
 *                            The string given here will be used in the
 *                            message as the reason-phrase. This parameter is
 *                            optional and has a default value of NULL.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionRespond(
                                        IN RvSipTranscHandle   hTransc,
                                        IN RvUint16         responseCode,
                                        IN RvChar          *strReasonPhrase)
{
    Transaction        *pTransc = (Transaction *)hTransc;
    RvStatus           retStatus;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }
    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "RvSipTransactionRespond - Transaction 0x%p: going to send response %u",
               pTransc,responseCode));

    if ((responseCode < 100) || (responseCode >= 700))
    {
        /* The response code is out of limit */
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionRespond - Transaction 0x%p: Response was executed with a response-code out of the limit",
                   pTransc));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_BADPARAM;
    }

#ifndef RV_SIP_PRIMITIVES
    /*a provisional response cannot be sent in state reliable prov sent*/
    if((responseCode >= 100) && (responseCode < 200) &&
        pTransc->eState == RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionRespond - Transaction 0x%p: Failed - can't send provisional response at state %s",
                  pTransc,TransactionGetStateName(pTransc->eState)));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;
    }
#endif

    if(pTransc->eState != RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD &&
       pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD
       &&
       pTransc->eState != RVSIP_TRANSC_STATE_SERVER_CANCEL_REQUEST_RCVD

#ifndef RV_SIP_PRIMITIVES
       &&
       pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT &&
       pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_PRACK_COMPLETED
#endif /*RV_SIP_PRIMITIVES*/
       )
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionRespond - Transaction 0x%p: Failed - can't send response in state %s",
                  pTransc,TransactionGetStateName(pTransc->eState)));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pTransc->bSending == RV_TRUE)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionRespond - Transaction 0x%p: Failed - The transaction is in the middle of sending a different response",
                  pTransc));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;

    }


    pTransc->bSending = RV_TRUE;
    retStatus = TransactionControlSendResponse(pTransc,responseCode,strReasonPhrase,
                                               RVSIP_TRANSC_REASON_USER_COMMAND,
                                               RV_FALSE);
    pTransc->bSending = RV_FALSE;
    if(retStatus != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "RvSipTransactionRespond - Transaction 0x%p: Failed to send response (rv=%d)",
            pTransc, retStatus));
        TransactionUnLockAPI(pTransc);
        return retStatus;
    }

    TransactionUnLockAPI(pTransc);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionCancel
 * ------------------------------------------------------------------------
 * General: Cancels the transaction given as a parameter.
 *          calling RvSipTransactionCancel() will cause A client CANCEL
 *          transaction to be created with transaction key
 *          taken from the transaction that is about to be canceled.
 *          The CANCEL request will then be sent to the remote party.
 *          If the transaction to cancel is an Invite transaction it will
 *          assume the Invite Canceled state. A general transaction will assume
 *          the general canceled state.
 *          The newly created CANCEL transaction will assume the Cancel sent
 *          state.
 *          You can cancel only transactions that have received a provisional
 *          response and are in the PROCEEDING or PROCEEDING_TIMEOUT state.
 *          canceling a transaction that has already received a 
 *          final response has no affect.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTransc - The transaction to cancel.
 ***************************************************************************/
RVAPI RvStatus  RVCALLCONV RvSipTransactionCancel(
                                         IN  RvSipTranscHandle   hTransc)
{
    Transaction        *pTransc;
    RvStatus           rv;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "RvSipTransactionCancel - Transaction 0x%p: Canceling",
              pTransc));

    /*check that the transaction state is valid for cancelling*/
    if(pTransc->eState != RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING    &&
       pTransc->eState != RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING &&
       pTransc->eState != RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT  &&
       pTransc->eState != RVSIP_TRANSC_STATE_CLIENT_GEN_PROCEEDING    &&
       pTransc->eState != RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING_TIMEOUT)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionCancel - Transaction 0x%p: Failed - cannot cancel at state %s",
                  pTransc,TransactionGetStateName(pTransc->eState)));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;
    }
	
	if(pTransc->eState != RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING &&
	   pTransc->eState != RVSIP_TRANSC_STATE_CLIENT_GEN_PROCEEDING &&
	   pTransc->eState != RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING_TIMEOUT)
	{
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
			"RvSipTransactionCancel - Transaction 0x%p: Failed - cancel should be sent only after 1xx is received",
			pTransc));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;
	}

#ifndef RV_SIP_PRIMITIVES
    if(pTransc->eMethod == SIP_TRANSACTION_METHOD_PRACK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionCancel - Transaction 0x%p: Failed - PRACK transaction must not be canceled",
                  pTransc));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;

    }
#endif /*#ifndef RV_SIP_PRIMITIVES */


    rv = TransactionControlSendCancel(pTransc);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionCancel - Transaction 0x%p: Failed to cancel transaction (rv=%d:%s)",
                  pTransc, rv, SipCommonStatus2Str(rv)));
        TransactionUnLockAPI(pTransc);
        return rv;
    }

    TransactionUnLockAPI(pTransc);
    return RV_OK;
}


/***************************************************************************
 * RvSipTransactionTerminate
 * ------------------------------------------------------------------------
 * General: Causes immediate shut-down of the transaction:
 *          Changes the transaction's state to "terminated" state.
 *          Calls Destruct on the transaction being terminated.
 *  
 *          You may not use this function when the stack is in the
 *          middle of processing one of the following callbacks:
 *          pfnEvTransactionCreated, pfnEvInternalTranscCreated,
 *          pfnEvTranscCancelled, pfnEvSupplyParams, pfnEvOpenCallLeg
 *          pfnEvSigCompMsgNotResponded, pfnEvNewConnInUse,
 *          In this case the function returns RV_ERROR_TRY_AGAIN.
 *
 * Return Value: RV_OK - The transaction was terminated successfully.
 *               RV_ERROR_NULLPTR - The pointer to the transaction is a NULL
 *                        pointer.
 *               RV_ERROR_INVALID_HANDLE - The stack object failed to be locked.
 *               RV_ERROR_TRY_AGAIN - The object can not be terminated at this
 *                        stage, because it is in the middle of a stack callback.
 *               RV_ERROR_UNKNOWN - The transaction was not found in the transaction
 *                        manager, in other words the UnManageTransaction
 *                        call has returned the RV_ERROR_NOT_FOUND return value.
 *                        In practice the Destruct function call returns
 *                        RV_ERROR_UNKNOWN (that way it is detected here).
 *                        Note: The transaction was destructed and can
 *                        not be used any more. The RV_ERROR_UNKNOWN return is
 *                        for indication only.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTransc - The transaction to terminate.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionTerminate(
                                            IN RvSipTranscHandle hTransc)
{
    Transaction        *pTransc;
    RvStatus           retStatus;


    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(RV_TRUE == TransactionBlockTermination(pTransc))
    {    
        TransactionUnLockAPI(pTransc); /*unlock the call leg*/
        return RV_ERROR_TRY_AGAIN;

    }

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "RvSipTransactionTerminate - Transaction 0x%p: terminating transaction", pTransc));

    /*release transaction timers*/
    TransactionTimerReleaseAllTimers(pTransc);

    retStatus = TransactionTerminate(RVSIP_TRANSC_REASON_USER_COMMAND,
                                    (Transaction*)hTransc);
    TransactionUnLockAPI(pTransc);
    return retStatus;
}
/***************************************************************************
 * RvSipTransactionAck
 * ------------------------------------------------------------------------
 * General: Sends Ack request on a given Invite transaction.
 *          This function can only be called on Invite client transaction in state
 *          RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 *          pRequestUri -  The Request-Uri to use in the ACK message.
 *                         If the request URI is NULL then the request URI of the
 *                         transaction is used as the request URI. The request URI
 *                         can be NULL when the Ack is to a non 2xx response, and then
 *                         the request URI must be the same as the invite request URI
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionAck(
                                              IN  RvSipTranscHandle     hTransc,
                                              IN RvSipAddressHandle hRequestUri)
{
    Transaction        *pTransc;
    RvStatus           retStatus;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }
    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "RvSipTransactionAck - Transaction=0x%p, ReqUri=0x%p",
               hTransc, hRequestUri));

    /* validity checks */
    switch (pTransc->eState)
    {
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD:
        /*Ack can be sent only for reject response or if Invite is handled
          according to Bis 2543*/
        if(pTransc->responseCode >= 300 || 
           pTransc->pMgr->bOldInviteHandling == RV_TRUE)
        {
            break;
        }
    case RVSIP_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE:
        {
            /* if we have reached msg send failure, we can ack only on 503 */
            if (503 == pTransc->responseCode && 
				pTransc->eMethod == SIP_TRANSACTION_METHOD_INVITE)
            {
                break;
            }
        }
    default:
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                       "RvSipTransactionAck - Failed for transaction 0x%p. cannot ACK in state %s or for non INVITE method",
                       hTransc,TransactionGetStateName(pTransc->eState)));
               TransactionUnLockAPI(pTransc);
            return RV_ERROR_ILLEGAL_ACTION;
        }
    }/* switch (pTransc->eState) */

    retStatus = SipTransactionAck(hTransc, hRequestUri);
    TransactionUnLockAPI(pTransc);
    return(retStatus);
}

/***************************************************************************
 * RvSipTransactionRequestMsg
 * ------------------------------------------------------------------------
 * General: Sends a request message to the remote party using a given
 *          prepared message object. This function will normally be used by
 *          proxy implementations in order to proxy a received request to the remote
 *          party. The request will be sent according to the Request URI
 *          found in the  message object.
 *          The user is responsible for setting the correct Request-URI in the
 *          given message object and for applying the Record-Route
 *          rules if necessary.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 *            hMsg -           Handle of the request message object. This message
 *                           will be sent to the remote party.
 *          bAddTopVia -      Indicates whether the stack should add a top Via  header
 *                           to the request message before the message is sent.
 *                           You should set this parameter to RV_TRUE if you wish
 *                           the stack to handle the Via header for you. If you
 *                           have added the Via header by your self, set this
 *                           parameter to RV_FALSE.
 *                           Note: If you wish the stack to add a branch
 *                           parameter to the top Via header you should first use the
 *                           RvSipTransactionSetViaBranch() API function and set
 *                           the branch in the transaction.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionRequestMsg(
                                                   IN  RvSipTranscHandle     hTransc,
                                                   IN  RvSipMsgHandle         hMsg,
                                                   IN  RvBool               bAddTopVia)
{
    Transaction*       pTransc;
    RvStatus           retStatus;
    RvSipMsgHandle     hClonedMsg;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc || hMsg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "RvSipTransactionRequestMsg - Transaction=0x%p is about to send a request, Msg=0x%p, bAddTopVia=%d",
               pTransc, hMsg, bAddTopVia));

    /*destruct the transaction outbound message if it is different then the received msg*/
    if(pTransc->hOutboundMsg != NULL && pTransc->hOutboundMsg != hMsg)
    {
        RvSipMsgDestruct(pTransc->hOutboundMsg);
        pTransc->hOutboundMsg = NULL;
    }


    if(RVSIP_METHOD_ACK    == RvSipMsgGetRequestMethod(hMsg))
    {
        RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionRequestMsg(pTransc=0x%p) - Illegal to send ACK using RvSipTransactionRequestMsg function", pTransc));
        TransactionUnLockAPI(pTransc);
        return(RV_ERROR_BADPARAM);
    }
    /* Check if the message is request messgae */
    if(RV_TRUE != IsValidMessage(pTransc,hMsg,RV_TRUE))
    {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "RvSipTransactionRequestMsg - Transaction=0x%p, Msg=0x%p is not a valid message",
               pTransc, hMsg));
        TransactionUnLockAPI(pTransc);
        return(RV_ERROR_ILLEGAL_ACTION);
    }

    /*request can be sent only in initial state*/
    if (RVSIP_TRANSC_STATE_IDLE != pTransc->eState)
    {
        /* A request is allowed only in an Initial state */
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionRequestMsg - Transaction 0x%p: Error - The function can be called only on Idle state",
                  pTransc));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* Clone & copy the specified message */
    retStatus = RvSipMsgConstruct((pTransc->pMgr)->hMsgMgr, (pTransc->pMgr)->hMessagePool, &hClonedMsg);
    if (RV_OK != retStatus)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "RvSipTransactionRequestMsg - Transaction=0x%p, Error Cloning of the specified message (rv=%d)",
               pTransc, retStatus));
        TransactionUnLockAPI(pTransc);
        return retStatus;
    }
    retStatus = RvSipMsgCopy(hClonedMsg, hMsg);
    if (RV_OK != retStatus)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "RvSipTransactionRequestMsg - Transaction=0x%p,  Error Copying of the specified message (rv=%d)",
               pTransc, retStatus));
        RvSipMsgDestruct(hClonedMsg);
        TransactionUnLockAPI(pTransc);
        return retStatus;
    }
    pTransc->hOutboundMsg = hClonedMsg;
    retStatus = TransactionControlSendRequestMsg((Transaction*)hTransc, bAddTopVia);
    /*destruct the message if it was not reset*/
    if(pTransc->hOutboundMsg != NULL)
    {
        RvSipMsgDestruct(hClonedMsg);
        pTransc->hOutboundMsg = NULL;
    }
    TransactionUnLockAPI(pTransc);
    return(retStatus);
}

/***************************************************************************
 * RvSipTransactionRespondMsg
 * ------------------------------------------------------------------------
 * General: Sends a response message to the remote party using a given
 *          prepared message object. This function will normally be used by
 *          proxy implementations in order to proxy a received response.
 *          The response will be sent according to the top Via header
 *          found in the  message object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 *            hMsg -           Handle of response message object.
 *          bRemoveTopVia -   Indicate whether the stack should remove the
 *                           top Via header from the message object before the
 *                           destination is resolved and the message is sent.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionRespondMsg(
                                                   IN  RvSipTranscHandle     hTransc,
                                                   IN  RvSipMsgHandle         hMsg,
                                                   IN  RvBool               bRemoveTopVia)
{
    Transaction        *pTransc;
    RvStatus           retStatus;
    RvSipMsgHandle        hClonedMsg;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
    {
        TransactionUnLockAPI(pTransc);
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "RvSipTransactionRespondMsg - Failed, Transaction 0x%p: in state Terminated",pTransc));
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pTransc->eState != RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD &&
       pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD &&
       pTransc->eState != RVSIP_TRANSC_STATE_SERVER_CANCEL_REQUEST_RCVD &&
       pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_PROXY_2XX_RESPONSE_SENT
#ifndef RV_SIP_PRIMITIVES
       &&
       pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT &&
       pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_PRACK_COMPLETED
#endif /*RV_SIP_PRIMITIVES*/
       )
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionRespondMsg - Transaction 0x%p: Failed - can't send response in state %s",
                  pTransc,TransactionGetStateName(pTransc->eState)));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pTransc->bSending == RV_TRUE)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionRespondMsg - Transaction 0x%p: Failed - The transaction is in the middle of sending a different response",
                  pTransc));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;

    }

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "RvSipTransactionRespondMsg - Transaction=0x%p Is about to send a response, Msg=0x%p, bRemoveTopVia=%d",
               pTransc, hMsg, bRemoveTopVia));


    /*destruct the transaction outbound message if it is different then the received msg*/
    if(pTransc->hOutboundMsg != NULL && pTransc->hOutboundMsg != hMsg)
    {
        RvSipMsgDestruct(pTransc->hOutboundMsg);
        pTransc->hOutboundMsg = NULL;
    }

    /* Check if the message is response messgae */
    if(RV_TRUE != IsValidMessage(pTransc,hMsg, RV_FALSE))
    {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "RvSipTransactionRespondMsg - Transaction=0x%p, Msg=0x%p is not a valid message",
               pTransc, hMsg));
        TransactionUnLockAPI(pTransc);
        return(RV_ERROR_ILLEGAL_ACTION);
    }
    /* Clone & copy the specified message */
    retStatus = RvSipMsgConstruct((pTransc->pMgr)->hMsgMgr, (pTransc->pMgr)->hMessagePool, &hClonedMsg);
    if (RV_OK != retStatus)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "RvSipTransactionRespondMsg - Transaction=0x%p, Error Cloning of the specified message (rv=%d)",
               pTransc, retStatus));
           TransactionUnLockAPI(pTransc);
        return retStatus;
    }
    retStatus = RvSipMsgCopy(hClonedMsg, hMsg);
    if (RV_OK != retStatus)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "RvSipTransactionRespondMsg - Transaction=0x%p,  Error Copying of the specified message (rv=%d)",
               pTransc, retStatus));
        RvSipMsgDestruct(hClonedMsg);
           TransactionUnLockAPI(pTransc);
        return retStatus;
    }
    pTransc->hOutboundMsg = hClonedMsg;

    retStatus = TransactionControlSendRespondMsg((Transaction*)hTransc,bRemoveTopVia);
    /*destruct the message if it was not reset in the process of sending*/
    if(pTransc->hOutboundMsg != NULL)
    {
        RvSipMsgDestruct(hClonedMsg);
        pTransc->hOutboundMsg = NULL;
    }
    TransactionUnLockAPI(pTransc);
    return(retStatus);
}

/***************************************************************************
 * RvSipTransactionSetKeyFromMsg
 * ------------------------------------------------------------------------
 * General: Initialize the transaction key from a given message.
 *          The following fields will be set in the transaction:
 *          To and From (including tags), Call-Id and CSeq step.
 *          before calling this function you should call
 *            RvSipTranscMgrCreateTransaction to create a new transaction object.
 *          This function will normally be used by proxy implementations. Before a
 *          proxy server can proxy a request it should:
 *          1. Create a new client transaction object with RvSipTranscMgrCreateTransaction().
 *          2. Get the received message from the server
 *             transaction using the RvSipTransactionGetReceivedMsg() function.
 *          3. Set the transaction key using the RvSipTransactionSetKeyFromMsg() function.
 *          Then the proxy can use the initialized client transaction to proxy the
 *          request.
 *
 * Return Value:RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc - The transaction handle.
 *          hMsg    - Handle to the message from which the transaction key will be set.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionSetKeyFromMsg(
                                                      IN  RvSipTranscHandle     hTransc,
                                                      IN  RvSipMsgHandle         hMsg)
{
    RvStatus  rv = RV_OK;

    RvSipPartyHeaderHandle  hTo;
    RvSipPartyHeaderHandle  hFrom;
    RvSipCSeqHeaderHandle   hCSeq;
    RvSipAddressHandle      hMsgReqUri;
    RvInt32                 cseqStep = UNDEFINED;


    Transaction    *pTransc = (Transaction*)hTransc;

    if (NULL == hTransc || hMsg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "RvSipTransactionSetKeyFromMsg - Transaction=0x%p, Msg=0x%p",
               pTransc, hMsg));


    /* Set CSeq data */
    hCSeq = RvSipMsgGetCSeqHeader(hMsg);
    if(NULL == hCSeq)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "RvSipTransactionSetKeyFromMsg - Transaction=0x%p, Msg=0x%p Unable to retrieve CSeq",
                   pTransc, hMsg));
        TransactionUnLockAPI(pTransc);
        return(RV_ERROR_UNKNOWN);
    }
    else
    {
		rv = SipCSeqHeaderGetInitializedCSeq(hCSeq,&cseqStep); 
		if (RV_OK != rv)
		{
			RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
				"RvSipTransactionSetKeyFromMsg: pTransc 0x%p - SipCSeqHeaderGetInitializedCSeq() failed (CSeq might be uninitialized)", pTransc));
			TransactionUnLockAPI(pTransc);
			return rv;
		}

        RvSipTransactionSetCSeqStep(hTransc,cseqStep);
    }

    /* Set From data */
    hFrom = RvSipMsgGetFromHeader(hMsg);
    if(NULL == hFrom)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "RvSipTransactionSetKeyFromMsg - Transaction=0x%p, Msg=0x%p Unable to retrieve From header",
                   pTransc, hMsg));
        TransactionUnLockAPI(pTransc);
        return(RV_ERROR_UNKNOWN);
    }
    else
    {
        rv = RvSipTransactionSetFromHeader(hTransc,hFrom);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "RvSipTransactionSetKeyFromMsg - Transaction 0x%p: Failed to set from header (rv=%d)",
                pTransc,rv));
            TransactionUnLockAPI(pTransc);
            return rv;
        }
    }

    /* Set To data */
    hTo = RvSipMsgGetToHeader(hMsg);
    if(NULL == hTo)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "RvSipTransactionSetKeyFromMsg - Transaction=0x%p, Msg=0x%p Unable to retrieve To header (rv=%d)",
                   pTransc, hMsg,rv));
        TransactionUnLockAPI(pTransc);
        return(RV_ERROR_UNKNOWN);
    }
    else
    {
        rv = RvSipTransactionSetToHeader(hTransc,hTo);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "RvSipTransactionSetKeyFromMsg - Transaction 0x%p: Failed to set from header (rv=%d)",
                pTransc,rv));
            TransactionUnLockAPI(pTransc);
            return rv;
        }
        /* bug-fix: update the transaction bTagInTo parameter */
        pTransc->bTagInTo = (SipPartyHeaderGetTag(hTo)==UNDEFINED) ? RV_FALSE : RV_TRUE;
    }

    /* Set Call-Id data */
    TransactionSetCallID(pTransc, SipMsgGetPool(hMsg),
                    SipMsgGetPage(hMsg), SipMsgGetCallIdHeader(hMsg));

    rv = TransactionCopyCallId(pTransc);
    if(RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "RvSipTransactionSetKeyFromMsg - Transaction=0x%p, Msg=0x%p Unable to Copy call-id header (rv=%d)",
                   pTransc, hMsg,rv));
        TransactionUnLockAPI(pTransc);
        return(rv);
    }

    hMsgReqUri = RvSipMsgGetRequestUri(hMsg);
    if(NULL == hMsgReqUri )
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
            "RvSipTransactionSetKeyFromMsg - Transaction=0x%p, Msg=0x%p Unable to Get Request-Uri",
            pTransc, hMsg));
        TransactionUnLockAPI(pTransc);
        return(RV_ERROR_UNKNOWN);
    }
    rv = TransactionSetRequestUri(pTransc,hMsgReqUri);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "RvSipTransactionSetKeyFromMsg - Transaction 0x%p: Failed to set request uri address (rv=%d)",
            pTransc,rv));
        TransactionUnLockAPI(pTransc);
        return rv;

    }
    TransactionUnLockAPI(pTransc);
    return(rv);
}

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * RvSipTransactionAuthBegin
 * ------------------------------------------------------------------------
 * General: Begin the server authentication process.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionAuthBegin(IN RvSipTranscHandle hTransc)
{
    RvStatus           rv = RV_OK;

    Transaction         *pTransc = (Transaction*)hTransc;

    if (NULL == hTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "RvSipTransactionAuthBegin - Transaction=0x%p",
               pTransc));

    if(pTransc->eState != RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD &&
       pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD &&
       pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT &&
       pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_PRACK_COMPLETED &&
       pTransc->eState != RVSIP_TRANSC_STATE_SERVER_PRACK_FINAL_RESPONSE_SENT)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RvSipTransactionAuthBegin - Transaction 0x%p: Failed - can't do authentication in state %s",
                  pTransc,TransactionGetStateName(pTransc->eState)));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = TransactionAuthBegin(hTransc);

    TransactionUnLockAPI(pTransc);
    return(rv);
}

/***************************************************************************
 * RvSipTransactionAuthProceed
 * ------------------------------------------------------------------------
 * General: The function order the stack to proceed in authentication procedure.
 *          actions options are:
 *          RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD
 *          - Check the given authorization header, with the given password.
 *
 *          RVSIP_TRANSC_AUTH_ACTION_SUCCESS
 *          - user had checked the authorization header by himself, and it is
 *            correct. (will cause AuthCompletedEv to be called, with status success)..
 *
 *          RVSIP_TRANSC_AUTH_ACTION_FAILURE
 *          - user wants to stop the loop that searches for authorization headers.
 *            (will cause AuthCompletedEv to be called, with status failure).
 *
 *          RVSIP_TRANSC_AUTH_ACTION_SKIP
 *          - order to skip the given header, and continue the authentication procedure
 *            with next header (if exists).
 *            (will cause AuthCredentialsFoundEv to be called, or
 *            AuthCompletedEv(failure) if there are no more authorization headers.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc -        The transaction handle.
 *          action -         In which action to proceed. (see above)
 *          hAuthorization - Handle of the authorization header that the function
 *                           will check authentication for. (needed if action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD, else NULL.)
 *          password -       The password for the realm+userName in the header.
 *                           (needed if action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD, else NULL.)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionAuthProceed(
                            IN  RvSipTranscHandle              hTransc,
                            IN  RvSipTransactionAuthAction     action,
                            IN  RvSipAuthorizationHeaderHandle hAuthorization,
                            IN  RvChar                        *password)
{
#ifdef RV_SIP_AUTH_ON
    RvStatus           rv = RV_OK;

    Transaction         *pTransc = (Transaction*)hTransc;

    if (NULL == hTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionAuthProceed - Transaction=0x%p, action %d, hAuthorization %d"
              ,pTransc, action, hAuthorization));

    rv = SipTransactionAuthProceed(hTransc, action, hAuthorization, password,
            (void*)pTransc, RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSACTION,
            pTransc->tripleLock);
    if (RV_OK != rv)
    {
        TransactionUnLockAPI(pTransc);
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc,
            "RvSipTransactionAuthProceed - Failed to proceed. Transaction=0x%p, action %d, hAuthorization %d"
            ,pTransc, action, hAuthorization));
        return rv;
    }

    TransactionUnLockAPI(pTransc);
    return RV_OK;
#else
    RV_UNUSED_ARG(hTransc);
    RV_UNUSED_ARG(action);
    RV_UNUSED_ARG(hAuthorization);
    RV_UNUSED_ARG(password);
    return RV_ERROR_NOTSUPPORTED;
#endif /* #ifdef RV_SIP_AUTH_ON */
}

/***************************************************************************
 * RvSipTransactionRespondUnauthenticated
 * ------------------------------------------------------------------------
 * General: Sends 401/407 response.
 *          Add a copy of the given authentication header to the response message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 *          responseCode -   401 or 407
 *          headerType -     The type of the given header
 *          hHeader -        Pointer to the header to be set in the msg.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionRespondUnauthenticated(
                                   IN  RvSipTranscHandle    hTransc,
                                   IN  RvUint16            responseCode,
                                   IN  RvChar              *strReasonPhrase,
                                   IN  RvSipHeaderType      headerType,
                                   IN  void*                hHeader)
{
    return RespondUnauthenticated(hTransc, responseCode, strReasonPhrase,
                                   headerType, hHeader,
                                   NULL, NULL, NULL, NULL, RV_TRUE,
                                   RVSIP_AUTH_ALGORITHM_UNDEFINED, NULL,
                                   RVSIP_AUTH_QOP_UNDEFINED, NULL);
}

/***************************************************************************
 * RvSipTransactionRespondUnauthenticatedDigest
 * ------------------------------------------------------------------------
 * General: Sends 401/407 response.
 *          Add the authentication header to the response message, with the
 *          given arguments in it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 *          responseCode -   401 or 407
 *          strReasonPhrase - The reason phrase for this response code.
 *          strRealm -       mandatory.
 *          strDomain -      Optional string. may be NULL.
 *          strNonce -       Optional string. may be NULL.
 *          strOpaque -      Optional string. may be NULL.
 *          bStale -         TRUE or FALSE
 *          eAlgorithm -     Enumeration of algorithm. if RVSIP_AUTH_ALGORITHM_OTHER
 *                           the algorithm value is taken from the the next argument.
 *          strAlgorithm -   String of algorithm. this parameters will be set only if
 *                           eAlgorithm parameter is set to be RVSIP_AUTH_ALGORITHM_OTHER.
 *          eQop -           Enumeration of qop. if RVSIP_AUTH_QOP_OTHER, the qop value
 *                           will be taken from the next argument.
 *          strQop -         String of qop. this parameter will be set only if eQop
 *                           argument is set to be RVSIP_AUTH_QOP_OTHER.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionRespondUnauthenticatedDigest(
                                   IN  RvSipTranscHandle hTransc,
                                   IN  RvUint16         responseCode,
                                   IN  RvChar           *strReasonPhrase,
                                   IN  RvChar           *realm,
                                   IN  RvChar           *domain,
                                   IN  RvChar           *nonce,
                                   IN  RvChar           *opaque,
                                   IN  RvBool           stale,
                                   IN  RvSipAuthAlgorithm eAlgorithm,
                                   IN  RvChar            *strAlgorithm,
                                   IN  RvSipAuthQopOption eQop,
                                   IN  RvChar            *strQop)
{
    return RespondUnauthenticated(hTransc, responseCode, strReasonPhrase,
                                   RVSIP_HEADERTYPE_UNDEFINED, NULL,
                                   realm, domain, nonce, opaque, stale,
                                   eAlgorithm, strAlgorithm, eQop, strQop);
}
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT
/***************************************************************************
 * RvSipTransactionPrepareForReprocessing
 * ------------------------------------------------------------------------
 * General: Prepares the transaction for reprocessing. 
 *          Notice: The latest possible point to call this function is at 
 *          the StateChanged() that reports a receipt of a request. 
 *          Notice: You must call this function before you call 
 *          RvSipTranscMgrReprocessTransactionEv() for this transaction. 
 *          However, you may not call RvSipTranscMgrReprocessTransactionEv() 
 *          at all even if this function was called earlier. In this case you
 *          can respond or terminate this transaction normally.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc -        The transaction handle.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionPrepareForReprocessing(
                                   IN  RvSipTranscHandle    hTransc)
{
	Transaction    *pTransc = (Transaction*)hTransc;
	RvStatus        rv;

	if (pTransc == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc,
				"RvSipTransactionPrepareForReprocessing - Transaction=0x%p is preparing for reprocessing",pTransc));

	/* Make sure that the request that was received by the transaction will not terminate.
	1) Store its handle in a different field. 2) Increment its counter so it will require 
	two calls to RvSipMsgDestruct() */
	pTransc->hInitialRequestForReprocessing = pTransc->hReceivedMsg;
	SipMsgIncCounter(pTransc->hInitialRequestForReprocessing);
	
	TransactionUnLockAPI(pTransc);
    return(rv);
}
#endif /* #ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */


/*-----------------------------------------------------------------------
       T R A N S A C T I O N:  G E T   A N D   S E T    A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipTransactionGetNewMsgElementHandle
 * ------------------------------------------------------------------------
 * General:Allocates a new message element on the transaction page, and returns the new
 *         element handle.
 *         The application may use this function to allocate a message header or a message
 *         address. It should then fill the element information,
 *         and set it back to the transaction using the relevant 'set' function.
 *
 *         The function supports the following elements:
 *         To, From - you should set these headers back with 
 *                    RvSipTransactionSetToHeader() or RvSipTransactionSetFromHeader().
 *         Address of any type - you should supply the address to the 
 *                               RvSipTransactionRequest() function
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction.
 *            eHeaderType - The type of header to allocate. RVSIP_HEADERTYPE_UNDEFINED
 *                        should be supplied when allocating an address.
 *            eAddrType - The type of the address to allocate. RVSIP_ADDRTYPE_UNDEFINED
 *                        should be supplied when allocating a header.
 * Output:    phMsgElement - Handle to the newly created header or address.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetNewMsgElementHandle (
									  IN   RvSipTranscHandle    hTransc,
                                      IN   RvSipHeaderType      eHeaderType,
                                      IN   RvSipAddressType     eAddrType,
                                      OUT  void*                *phMsgElement)

{
	RvStatus     rv;
    Transaction *pTransc = (Transaction*)hTransc;

    if(pTransc == NULL || phMsgElement == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phMsgElement = NULL;

    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (eHeaderType != RVSIP_HEADERTYPE_TO &&
        eHeaderType != RVSIP_HEADERTYPE_FROM &&
        eHeaderType == RVSIP_HEADERTYPE_UNDEFINED &&
		eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
    {
        /* None of the supported headers, and not an address */
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "RvSipTransactionGetNewMsgElementHandle - Transc 0x%p - unsupported action for header %s address %s",
            pTransc, SipHeaderName2Str(eHeaderType), SipAddressType2Str(eAddrType)));
        TransactionUnLockAPI(pTransc); /*unlock the transaction*/
        return RV_ERROR_ILLEGAL_ACTION;
		
    }
	
    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
		"RvSipTransactionGetNewMsgElementHandle - getting new header-%s || addr-%s handle from transc 0x%p",
		SipHeaderName2Str(eHeaderType),SipAddressType2Str(eAddrType),pTransc));
	
    if(eHeaderType != RVSIP_HEADERTYPE_UNDEFINED)
    {
        rv = SipHeaderConstruct(pTransc->pMgr->hMsgMgr, eHeaderType,pTransc->pMgr->hGeneralPool,
			pTransc->memoryPage, phMsgElement);
    }
    else /*address*/
    {
        rv = RvSipAddrConstruct(pTransc->pMgr->hMsgMgr, pTransc->pMgr->hGeneralPool,
			pTransc->memoryPage, eAddrType, (RvSipAddressHandle*)phMsgElement);
    }
	
    if (rv != RV_OK || *phMsgElement == NULL)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
			"RvSipTransactionGetNewMsgElementHandle - Transc 0x%p - Failed to create new header (rv=%d)",
			pTransc, rv));
        TransactionUnLockAPI(pTransc); /*unlock the transaction*/
        return RV_ERROR_OUTOFRESOURCES;
    }
	TransactionUnLockAPI(pTransc); /*unlock the transaction*/
    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionGetNewPartyHeaderHandle
 * ------------------------------------------------------------------------
 * General:Allocates a new Party header and returns the new Party header
 *         handle.
 *         In order to set the To or From headers of a transaction ,the application
 *         should:
 *         1. Get a new party header handle using this function.
 *         2. Fill the party header information and set it back using
 *            RvSipTransactionSetToHeader() or RvSipTransactionSetFromHeader().
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction.
 * Output:     phParty - Handle to the newly created party header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetNewPartyHeaderHandle (
                                      IN   RvSipTranscHandle      hTransc,
                                      OUT  RvSipPartyHeaderHandle  *phParty)
{
    return RvSipTransactionGetNewMsgElementHandle(hTransc, RVSIP_HEADERTYPE_FROM,
		                                          RVSIP_ADDRTYPE_UNDEFINED, 
												  (void**)phParty);
    
}


/***************************************************************************
 * RvSipTransactionGetNewAddressHandle
 * ------------------------------------------------------------------------
 * General:DEPRECATED!!! use RvSipTransactionGetNewMsgElementHandle() instead.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc  - Handle to the call-leg.
 *            eAddrType - Type of address the application wishes to create.
 * Output:     phAddr    - Handle to the newly created address header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetNewAddressHandle (
                                      IN   RvSipTranscHandle      hTransc,
                                      IN   RvSipAddressType        eAddrType,
                                      OUT  RvSipAddressHandle      *phAddr)
{
    return RvSipTransactionGetNewMsgElementHandle(hTransc, RVSIP_HEADERTYPE_UNDEFINED,
		                                          eAddrType, 
		                                          (void**)phAddr);
}



/***************************************************************************
 * RvSipTransactionSetLocalAddress
 * ------------------------------------------------------------------------
 * General: Sets the local address from which the transaction will send outgoing requests.
 *          The SIP Stack can be configured to listen to many local addresses. Each local
 *          address has a transport type (UDP/TCP/TLS) and an address type (IPv4/IPv6).
 *          When the SIP Stack sends an outgoing request, the local address (from where
 *          the request is sent) is chosen according to the characteristics of the remote
 *          address. Both the local and remote addresses must have the same characteristics,
 *          such as the same transport type and address type. If several configured local
 *          addresses match the remote address characteristics, the first configured address
 *          is taken.
 *          You can use RvSipTransactionSetLocalAddress() to force the transaction to
 *          choose a specific local address for a specific transport and address type. For
 *          example, you can force the transaction to use the second configured UDP/
 *          IPv4 local address. If the transaction sends a request to a UDP/IPv4 remote
 *          address, it will use the local address that you set instead of the default first local
 *          address.
 *          Note: The localAddress string you provide for this function must match exactly
 *          with the local address that was inserted in the configuration structure in the
 *          initialization of the SIP Stack. If you configured the SIP Stack to listen to a 0.0.0.0
 *          local address, you must use the same notation here.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction
 *          eTransportType - The transport type(UDP, TCP, SCTP, TLS).
 *          eAddressType - The address type(ip or ip6).
 *            localAddress - The local address to be set to this transaction.
 *          localPort - The local port to be set to this transaction.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetLocalAddress(
                            IN  RvSipTranscHandle         hTransc,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            IN  RvChar                   *localAddress,
                            IN  RvUint16                 localPort
#if defined(UPDATED_BY_SPIRENT)
                            , IN  RvChar                 *if_name
			    , IN  RvUint16               vdevblock
#endif
			    )
{
    RvStatus                      rv         = RV_OK;
    Transaction*                  pTransc    = (Transaction *)hTransc;

    if(pTransc == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "RvSipTransactionSetLocalAddress - Trans 0x%p, Failed to lock the transaction object (rv=%d:%s)",
            pTransc,rv,SipCommonStatus2Str(rv)));
        return rv;
    }

    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionSetLocalAddress - Transc 0x%p - Setting local address to transmitter 0x%p",
               pTransc,pTransc->hTrx));


    rv = SipTransmitterSetLocalAddress(pTransc->hTrx,
                                       eTransportType,
                                       eAddressType,
                                       localAddress,
                                       localPort
#if defined(UPDATED_BY_SPIRENT)
				       ,if_name
				       ,vdevblock
#endif
				       );

    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionSetLocalAddress - Transaction 0x%p, Failed to sel local address to transmitter 0x%p (rv=%d)",
              pTransc,pTransc->hTrx,rv));
        TransactionUnLockAPI(pTransc);
        return rv;
    }

    TransactionUnLockAPI(pTransc);
    return RV_OK;
}
/***************************************************************************
 * RvSipTransactionGetLocalAddress
 * ------------------------------------------------------------------------
 * General: Get the local address the Transaction will use to send outgoing
 *          requests. This is the address the user set using the
 *          RvSipCallLegSetLocalAddress function. If no address was set the
 *          function will return the default UDP address.
 *          The user can use the transport type and the address type to indicate
 *          which kind of local address he wishes to get.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction.
 *          eTransportType - The transport type(UDP, TCP, SCTP, TLS).
 *          eAddressType - The address type(ip or ip6).
 * Output:    localAddress - The local address this transaction is using.
 *          localPort - The local port this transaction is using.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetLocalAddress(
                            IN  RvSipTranscHandle         hTransc,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            OUT  RvChar                  *localAddress,
                            OUT  RvUint16                *localPort
#if defined(UPDATED_BY_SPIRENT)
                            , OUT  RvChar                *if_name
			    , OUT  RvUint16              *vdevblock
#endif
			    )
{
    Transaction*   pTransc = (Transaction*)hTransc;
    RvStatus       rv      = RV_OK;


    if (NULL == pTransc || localAddress==NULL || localPort==NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionGetLocalAddress - Transc 0x%p - Getting local From transmitter 0x%p",
               pTransc,pTransc->hTrx));


    rv = SipTransmitterGetLocalAddress(pTransc->hTrx,eTransportType,eAddressType,localAddress,localPort
#if defined(UPDATED_BY_SPIRENT)
				       ,if_name
				       ,vdevblock
#endif
				       );
    if(RV_OK != rv)
    {
       RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "RvSipTransactionGetLocalAddress - Transc 0x%p - Transmitter 0x%p, Failed(rv=%d:%s)",
                   pTransc,pTransc->hTrx, rv, SipCommonStatus2Str(rv)));
       TransactionUnLockAPI(pTransc);
       return rv;
    }

    TransactionUnLockAPI(pTransc);
    return RV_OK;

}

/***************************************************************************
 * RvSipTransactionGetCurrentLocalAddress
 * ------------------------------------------------------------------------
 * General: Get the local address the Transaction will use to send outgoing
 *          requests. The transaction holds 4 types of local address:
 *          UDP address on IP4, UDP address on IP6, TCP address on IP4 and TCP address
 *          on IP6. This function returns the actual address from the 4 addresses
 *          that was used or going to be used.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction.
 * Output:    eTransporType - The transport type(UDP, TCP, SCTP, TLS).
 *          eAddressType  - The address type(IP4 or IP6).
 *          localAddress - The local address this transaction is using(must be longer than 48).
 *          localPort - The local port this transaction is using.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetCurrentLocalAddress(
                            IN  RvSipTranscHandle         hTransc,
                            OUT RvSipTransport            *eTransportType,
                            OUT RvSipTransportAddressType *eAddressType,
                            OUT RvChar                   *localAddress,
                            OUT RvUint16                 *localPort
#if defined(UPDATED_BY_SPIRENT)
			    ,IN  RvChar                  *if_name
			    ,IN  RvUint16                *vdevblock
#endif
			    )
{

    Transaction        *pTransc = (Transaction *)hTransc;
    RvStatus          rv;
    if (NULL == pTransc || localAddress==NULL || localPort==NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
               "RvSipTransactionGetCurrentLocalAddress - Transc 0x%p - getting current local address from Transmitter 0x%p",
                pTransc,pTransc->hTrx));

    rv = SipTransmitterGetCurrentLocalAddress(pTransc->hTrx,eTransportType,eAddressType,localAddress,localPort
#if defined(UPDATED_BY_SPIRENT)
					      ,if_name
					      ,vdevblock
#endif
					      );

    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "RvSipTransactionGetCurrentLocalAddress - Failed, Transc 0x%p - Transmitter 0x%p (rv=%d)",
                  pTransc,pTransc->hTrx,rv));
        TransactionUnLockAPI(pTransc);
        return rv;
    }

    TransactionUnLockAPI(pTransc);
    return RV_OK;
}

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
RvSipTransportLocalAddrHandle  RvSipTransactionGetCurrentLocalAddressHandle(IN  RvSipTranscHandle         hTransc)
{
  RvSipTransportLocalAddrHandle handle = NULL;
  
  Transaction        *pTransc = (Transaction *)hTransc;
  RvStatus          rv;
  if (NULL == pTransc) {
    return NULL;
  }
  
  if (RV_OK != TransactionLockAPI(pTransc)) {
    return NULL;
  }
  
  handle = SipTransmitterGetCurrentLocalAddressHandle(pTransc->hTrx);
  
  TransactionUnLockAPI(pTransc);

  return handle;
}

#endif
//SPIRENT_END

/***************************************************************************
 * RvSipTransactionGetCurrentDestAddress
 * ------------------------------------------------------------------------
 * General: Get the destination address the Transaction will use to send the
 *          next outgoing message. This address is available only in the
 *          context of the RvSipTransactionFinalDestResolvedEv callback.
 * Return Value: RvStatus. If the destination address was not yet resolved
 *                          the function returns RV_ERROR_NOT_FOUND.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction.
 * Output:    peTransportType - The transport type(UDP, TCP, SCTP, TLS).
 *          peAddressType  - The address type(IP4 or IP6).
 *          strDestAddress - The destination ip address. (must be of RVSIP_TRANSPORT_LEN_STRING_IP size)
 *          pDestPort - The destination port.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetCurrentDestAddress(
                            IN  RvSipTranscHandle         hTransc,
                            OUT RvSipTransport            *peTransportType,
                            OUT RvSipTransportAddressType *peAddressType,
                            OUT RvChar                    *strDestAddress,
                            OUT RvUint16                  *pDestPort)
{
    Transaction        *pTransc = (Transaction *)hTransc;
    SipTransportAddress trxDestAddr;
    RvStatus            rv;
    if (NULL == pTransc || strDestAddress==NULL || pDestPort==NULL ||
        NULL == peTransportType || NULL == peAddressType)
    {
        return RV_ERROR_BADPARAM;
    }

    strDestAddress[0] = '\0'; /*reset the local address*/
    *pDestPort = 0;
    *peTransportType = RVSIP_TRANSPORT_UNDEFINED;
    *peAddressType = RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED;

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*get the destination address from the transmitter*/
    rv = SipTransmitterGetDestAddr(pTransc->hTrx,&trxDestAddr);
    if(rv != RV_OK)
    {
        TransactionUnLockAPI(pTransc);
        return rv;
    }

    if(0 == RvAddressGetIpPort(&trxDestAddr.addr))
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                   "RvSipTransactionGetCurrentDestAddress - Transc 0x%p, Destination address was not yet resolved.",
                   pTransc));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_NOT_FOUND;
    }
    *peTransportType = trxDestAddr.eTransport;
    *peAddressType = SipTransportConvertCoreAddrTypeToSipAddrType(RvAddressGetType(&trxDestAddr.addr));
    *pDestPort = RvAddressGetIpPort(&trxDestAddr.addr);


    if(RVSIP_TRANSPORT_ADDRESS_TYPE_IP == *peAddressType )
    {
        RvAddressGetString(&trxDestAddr.addr,RVSIP_TRANSPORT_LEN_STRING_IP,strDestAddress);
    }
#if(RV_NET_TYPE & RV_NET_IPV6)
    else
    {
        RvInt32 destLen;
        strDestAddress[0] = '[';
        RvAddressGetString(&trxDestAddr.addr,RVSIP_TRANSPORT_LEN_STRING_IP-3,strDestAddress+1);
        destLen = (RvInt32)strlen(strDestAddress);
        strDestAddress[destLen] = ']';
        strDestAddress[destLen+1] = '\0';
    }
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/

    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionGetCurrentDestAddress - Transc 0x%p - Destination address is: ip=%s, port=%d, transport=%s, address type=%s",
               pTransc,strDestAddress,*pDestPort,
               SipCommonConvertEnumToStrTransportType(*peTransportType),
               SipCommonConvertEnumToStrTransportAddressType(*peAddressType)));

    TransactionUnLockAPI(pTransc);
    return RV_OK;

}

/***************************************************************************
 * RvSipTransactionSetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Sets all outbound proxy details to the transmitter object.
 *          All details are supplied in the RvSipTransportOutboundProxyCfg structure that
 *           includes parameters such as the IP address or host name, transport, port and
 *           compression type. Requests sent by this object will use the outbound detail
 *           specifications as a remote address. The Request-URI will be ignored. However,
 *           the outbound proxy will be ignored if the message contains a Route header or if
 *           the RvSipTransactionIgnoreOutboundProxy() function was called.
 *
 *          Note: If you specify both the IP address and a host name in the configuration
 *          structure, either of them will be set BUT the IP address will be used.
 *          If you do not specify port or transport, both will be determined according
 *          to the DNS procedures specified in RFC 3263.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - Handle to the transaction
 *            pOutboundCfg   - Pointer to the outbound proxy configuration
 *                             structure with all relevant details.
 *            sizeOfCfg      - The size of the outbound proxy configuration structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetOutboundDetails(
                            IN  RvSipTranscHandle               hTransc,
                            IN  RvSipTransportOutboundProxyCfg *pOutboundCfg,
                            IN  RvInt32                         sizeOfCfg)
{

    RvStatus           rv = RV_OK;
    Transaction            *pTransc;

    RV_UNUSED_ARG(sizeOfCfg);
    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc || NULL == pOutboundCfg)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "RvSipTransactionSetOutboundDetails - Transaction 0x%p - setting outbound details to transmitter 0x%p",
            pTransc, pTransc->hTrx));
    rv = RvSipTransmitterSetOutboundDetails(pTransc->hTrx,pOutboundCfg,
                                            sizeof(RvSipTransportOutboundProxyCfg));
    if(rv != RV_OK)
    {
         RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                   "RvSipTransactionSetOutboundDetails - Transaction 0x%p - Failed to set outbound details to the transaction trx 0x%p",
                    pTransc, pTransc->hTrx));
         TransactionUnLockAPI(pTransc); /*unlock the transaction*/
         return rv;
    }

    TransactionUnLockAPI(pTransc); /*unlock the transaction*/

    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionGetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Gets all the outbound proxy details that the transaction object uses.
 *          The details are placed in the RvSipTransportOutboundProxyCfg structure that
 *          includes parameters such as the IP address or host name, transport, port and
 *          compression type.
 *          If the outbound details were not set to the specific transaction object but the
 *          outbound proxy was defined to the SIP Stack on initialization, the SIP Stack
 *          parameters will be returned. If the transaction is not using an outbound address,
 *          NULL/UNDEFINED values are returned.
 *          Note You must supply a valid consecutive buffer in the
 *          RvSipTransportOutboundProxyCfg structure to get the outbound strings (host
 *          name and IP address).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc      - Handle to the transaction
 *            sizeOfCfg      - The size of the configuration structure.
 * Output:  pOutboundCfg   - Pointer to outbound proxy configuration
 *                           structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetOutboundDetails(
                            IN  RvSipTranscHandle               hTransc,
                            IN  RvInt32                         sizeOfCfg,
                            OUT RvSipTransportOutboundProxyCfg *pOutboundCfg)
{
    RvStatus           rv = RV_OK;
    Transaction            *pTransc;

    RV_UNUSED_ARG(sizeOfCfg);
    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc || NULL == pOutboundCfg)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionGetOutboundDetails - Transaction 0x%p - getting outbound details from transmitter 0x%p",
               pTransc, pTransc->hTrx));

    rv = RvSipTransmitterGetOutboundDetails(pTransc->hTrx,sizeof(RvSipTransportOutboundProxyCfg),pOutboundCfg);

    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "RvSipTransactionGetOutboundDetails - Transaction 0x%p - Failed to get outbound details from transmitter 0x%p (rv=%d)",
                   pTransc,pTransc->hTrx,rv));
        TransactionUnLockAPI(pTransc); /*unlock the Transaction*/
        return rv;
    }

    RvLogInfo(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
             "RvSipTransactionGetOutboundDetails - outbound details of Transaction 0x%p: address=%s, host=%s, port=%d, trans=%s",
              pTransc, pOutboundCfg->strIpAddress,pOutboundCfg->strHostName, pOutboundCfg->port,
              SipCommonConvertEnumToStrTransportType(pOutboundCfg->eTransport)));

    TransactionUnLockAPI(pTransc); /*unlock the Transaction*/

    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionSetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Sets the outbound address the transaction will use as the request
 *          destination address. A requests sent
 *          by this transaction will be sent according to the transaction
 *          outbound address and not according to the Request-URI.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc         - Handle to the transaction
 *            strOutboundAddrIp    - The outbound ip to be set to this
 *                                 transaction.
 *          outboundPort         - The outbound port to be set to this
 *                                 transaction.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetOutboundAddress(
                            IN  RvSipTranscHandle           hTransc,
                            IN  RvChar                     *strOutboundAddrIp,
                            IN  RvUint16                   outboundPort)

{
    RvStatus           rv=RV_OK;
    Transaction            *pTransc;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionSetOutboundAddress - Transaction 0x%p - Setting an outbound address to transmitter 0x%p",
              pTransc, pTransc->hTrx));

    rv = SipTransmitterSetOutboundAddress(pTransc->hTrx,strOutboundAddrIp,outboundPort);
    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                    "RvSipTransactionSetOutboundAddress - Failed, Transaction 0x%p, Transmitter 0x%p (rv=%d)",
                   pTransc,pTransc->hTrx,rv));
        TransactionUnLockAPI(pTransc); /*unlock the transaction*/
        return rv;
    }
    TransactionUnLockAPI(pTransc); /*unlock the transaction*/
    return RV_OK;
}



/***************************************************************************
 * RvSipTransactionGetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Get the outbound address the transaction is using. If an
 *          outbound address was set to the transaction it is returned. If
 *          no outbound address was set to the transaction, but an outbound
 *          proxy was configured for the stack the configured address is
 *          returned. '\0' is returned no address was defined for
 *          the transaction or the stack.
 *          NOTE: you must supply a valid consecutive buffer of size 20 for the
 *          strOutboundIp parameter.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTransc      - Handle to the transaction
 * Output:
 *            strOutboundIp   - A buffer with the outbound IP address the transaction.
 *                               is using
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetOutboundAddress(
                            IN   RvSipTranscHandle    hTransc,
                            OUT  RvChar             *strOutboundIp,
                            OUT  RvUint16           *pOutboundPort)
{
    Transaction             *pTransc;
    RvStatus           rv;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc || strOutboundIp == NULL || pOutboundPort== NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTransmitterGetOutboundAddress(pTransc->hTrx,strOutboundIp,pOutboundPort);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "RvSipTransactionGetOutboundAddress - Failed, Transaction 0x%p Transmitter 0x%p,(rv=%d)",
            pTransc, pTransc->hTrx,rv));
        TransactionUnLockAPI(pTransc); /*unlock the transaction*/
        return rv;
    }

    TransactionUnLockAPI(pTransc); /*unlock the transaction*/
    return RV_OK;
}


/***************************************************************************
 * RvSipTransactionSetOutboundHostName
 * ------------------------------------------------------------------------
 * General: Sets the outbound proxy host name of the transaction object.
 * The outbound host name will be resolved each time a request is sent to this host.
 * the request will be sent to the resolved IP address.
 * Note: To set a specific IP address, use RvSipTransactionSetOutboundAddress().
 * If you configure a transaction with both an outbound IP address and an
 * outbound host name, the transaction will ignore the outbound host name and
 * will use the outbound IP address.
 * When using an outbound host all outgoing requests will be sent directly to the transaction
 * outbound proxy host unless the application specifically ordered to ignore the outbound host.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc           - Handle to the transaction
 *            strOutboundHostName    - The outbound proxy host to be set to this
 *                               transaction.
 *            outboundPort  - The outbound proxy port to be set to this
 *                               transaction. If you set the port to zero it
 *                               will be determined using the DNS procedure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetOutboundHostName(
                            IN  RvSipTranscHandle     hTransc,
                            IN  RvChar                *strOutboundHost,
                            IN  RvUint16              outboundPort)
{
    RvStatus       rv          =RV_OK;
    Transaction   *pTransc;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

     RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionSetOutboundHostName - Transaction 0x%p - Setting an outbound host name to transmitter 0x%p",
              pTransc, pTransc->hTrx));

    /*we set all info on the size an it will be copied to the Transaction on success*/
    rv = SipTransmitterSetOutboundHostName(pTransc->hTrx,strOutboundHost,outboundPort);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "RvSipTransactionSetOutboundHostName - Failed, Transaction 0x%p Transmitter 0x%p (rv=%d)",
                  pTransc,pTransc->hTrx,rv));
        TransactionUnLockAPI(pTransc);
        return rv;
    }
    TransactionUnLockAPI(pTransc); /*unlock the Transaction*/
    return RV_OK;
}


/***************************************************************************
 * RvSipTransactionGetOutboundHostName
 * ------------------------------------------------------------------------
 * General: Gets the host name of the outbound proxy that the transaction is using.
 * If an outbound host was set to the transaction this host will be returned. If no
 * outbound host was set to the transaction but an outbound host was configured
 * for the SIP Stack, the configured host is returned. If the transaction is not
 * using an outbound host, '\0' is returned.
 * Note: You must supply a valid consecutive buffer to get the strOutboundHost host
 *       name.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTransc      - Handle to the transaction
 * Output:
 *            strOutboundHost   -  A buffer with the outbound host name
 *          pOutboundPort - The outbound host port.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetOutboundHostName(
                            IN   RvSipTranscHandle    hTransc,
                            OUT  RvChar              *strOutboundHostName,
                            OUT  RvUint16            *pOutboundPort)
{
    Transaction         *pTransc;
    RvStatus           rv = RV_OK;
    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTransmitterGetOutboundHostName(pTransc->hTrx,strOutboundHostName,pOutboundPort);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                 "RvSipTransactionGetOutboundHostName - Failed, Transaction 0x%p, Transmitter 0x%p (rv=%d)",
                 pTransc,pTransc->hTrx,rv));
        TransactionUnLockAPI(pTransc); /*unlock the Transaction*/
        return rv;
    }

    TransactionUnLockAPI(pTransc); /*unlock the Transaction*/
    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionSetOutboundTransport
 * ------------------------------------------------------------------------
 * General: Sets the outbound transport of the transaction outbound proxy.
 * This transport will be used for the outbound proxy that you set using the
 * RvSipTransactionSetOutboundAddress() function or the
 * RvSipTransactionSetOutboundHostName() function. If you do not set an
 * outbound transport, the transport will be determined using the DNS procedures.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc           - Handle to the transaction
 *          eOutboundTransport - The outbound transport to be set
 *                               to this transaction.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetOutboundTransport(
                            IN  RvSipTranscHandle           hTransc,
                            IN  RvSipTransport              eOutboundTransport)
{
    Transaction        *pTransc;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionSetOutboundTransport - Transaction 0x%p - Setting outbound transport to transmitter 0x%p",
               pTransc, pTransc->hTrx));
    SipTransmitterSetOutboundTransport(pTransc->hTrx,eOutboundTransport);
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}



/***************************************************************************
 * RvSipTransactionGetOutboundTransport
 * ------------------------------------------------------------------------
 * General: Gets the transport of the outbound proxy that the transaction is using.
 * If an outbound transport was set to the transaction, this transport will be
 * returned. If no outbound transport was set to the transaction but an outbound
 * proxy was configured for the SIP Stack, the configured transport is returned.
 * RVSIP_TRANSPORT_UNDEFINED is returned if the transaction is not
 * using an outbound proxy.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTransc           - Handle to the transaction
 * Output: eOutboundTransport - The outbound proxy transport the transaction is using.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetOutboundTransport(
                            IN   RvSipTranscHandle    hTransc,
                            OUT  RvSipTransport      *eOutboundProxyTransport)
{
    Transaction   *pTransc = (Transaction *)hTransc;
    RvStatus       rv      = RV_OK;
    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = SipTransmitterGetOutboundTransport(pTransc->hTrx,eOutboundProxyTransport);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "RvSipTransactionGetOutboundTransport - Failed, Transaction 0x%p, Transmitter 0x%p(rv=%d)",
                  pTransc,pTransc->hTrx,rv));
        TransactionUnLockAPI(pTransc);
        return rv;
    }
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionSetPersistency
 * ------------------------------------------------------------------------
 * General: instructs the transaction on whether to try and use persistent
 *          connection or not. You can change the transaction persistency
 *          only at the Idle state.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle
 *          bIsPersistent - Determines whether the transaction will try to use
 *                          a persistent connection or not.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetPersistency(
                           IN RvSipTranscHandle       hTransc,
                           IN RvBool                 bIsPersistent)
{
    RvStatus  rv;

    Transaction             *pTransc = (Transaction*)hTransc;
    if(pTransc == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pTransc->eState != RVSIP_TRANSC_STATE_IDLE)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "RvSipTransactionSetPersistency - Failed. Persistency level can be changes only at the Idle state. Transc 0x%p state is %s",
                   pTransc,TransactionGetStateName(pTransc->eState)));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionSetPersistency - Setting transc 0x%p persistency to %d ",
              pTransc,bIsPersistent));


    rv = RvSipTransmitterSetPersistency(pTransc->hTrx,bIsPersistent);
    TransactionUnLockAPI(pTransc);
    return rv;
}
/***************************************************************************
 * RvSipTransactionGetPersistency
 * ------------------------------------------------------------------------
 * General: Returns whether the transaction is configured to try and use a
 *          persistent connection or not.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle
 * Output:  pbIsPersistent - Determines whether the call-leg uses
 *                          a persistent connection or not.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetPersistency(
        IN  RvSipTranscHandle                   hTransc,
        OUT RvBool                            *pbIsPersistent)
{

    Transaction        *pTransc;

    pTransc = (Transaction *)hTransc;
    *pbIsPersistent = RV_FALSE;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvSipTransmitterGetPersistency(pTransc->hTrx,pbIsPersistent);
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}


/***************************************************************************
 * RvSipTransactionSetConnection
 * ------------------------------------------------------------------------
 * General: Set a connection to be used by the transaction. The transaction will
 *          try to use the given connection for outgoing requests only. Responses are
 *          sent on the same connection as the request. If the connection
 *          will not fit the transaction local and remote addresses it will be
 *          replaced. You can set the transaction connection only on the IDLE
 *          state.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction
 *          hConn - Handle to the connection.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetConnection(
                   IN  RvSipTranscHandle                hTransc,
                   IN  RvSipTransportConnectionHandle   hConn)
{
    RvStatus                           rv;
    Transaction                        *pTransc= (Transaction *)hTransc;
    RvSipTransportConnectionEvHandlers evHandler;
    memset(&evHandler,0,sizeof(RvSipTransportConnectionEvHandlers));
    evHandler.pfnConnStateChangedEvHandler = TransactionTransportConnStateChangedEv;

    if (NULL == pTransc || hConn==NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionSetConnection - Transc 0x%p - Setting connection 0x%p", pTransc,hConn));

    /*setting is allowed only on IDLE state*/
    if(pTransc->eState != RVSIP_TRANSC_STATE_IDLE)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionSetConnection - Failed, transaction 0x%p Cannot set connection 0x%p on the %s state",
              pTransc,hConn,TransactionGetStateName(pTransc->eState)));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /*Attach the transmitter to the connection*/
    rv = SipTransmitterSetConnection(pTransc->hTrx,hConn);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionSetConnection - Failed, transaction 0x%p Cannot set connection 0x%p to transmitter 0x%p",
              pTransc,hConn,pTransc->hTrx));
        TransactionUnLockAPI(pTransc);
        return rv;
    }

    /*first try to attach the transaction to the connection*/
    rv = SipTransportConnectionAttachOwner(hConn,
                                  (RvSipTransportConnectionOwnerHandle)hTransc,
                                  &evHandler,sizeof(evHandler));

    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionSetConnection - Failed to attach transaction 0x%p to connection 0x%p",pTransc,hConn));
        TransactionUnLockAPI(pTransc);
        return rv;
    }
    /*detach from the transaction current owner if there is one and set the new connection
      handle*/
    if(pTransc->hActiveConn != NULL)
    {
        rv = RvSipTransportConnectionDetachOwner(pTransc->hActiveConn,
                                            (RvSipTransportConnectionOwnerHandle)hTransc);
           if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                      "RvSipTransactionSetConnection - Failed to detach transaction 0x%p from prev connection 0x%p",pTransc,pTransc->hActiveConn));
        }
    }
    pTransc->hActiveConn = hConn;

    TransactionUnLockAPI(pTransc);
    return RV_OK;
}
/***************************************************************************
 * RvSipTransactionGetConnection
 * ------------------------------------------------------------------------
 * General: Returns the connection that is currently being used by the
 *          transaction.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction.
 * Output:    phConn - Handle to the currently used connection
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetConnection(
                            IN  RvSipTranscHandle               hTransc,
                           OUT  RvSipTransportConnectionHandle *phConn)
{
    Transaction   *pTransc   = (Transaction*)hTransc;
    RvStatus       rv        = RV_OK;

    if (NULL == pTransc || phConn==NULL)
    {
        return RV_ERROR_NULLPTR;
    }

     *phConn = NULL;
    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionGetConnection - Transc 0x%p - Getting connection 0x%p",pTransc,pTransc->hActiveConn));

    if(pTransc->hActiveConn != NULL)
    {
        RvSipTransportConnectionType eConnectionType;
        rv = RvSipTransportConnectionGetType(pTransc->hActiveConn,&eConnectionType);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "RvSipTransactionGetConnection - Transc 0x%p - Failed in IsTcpClient for connection 0x%p",
                pTransc,pTransc->hActiveConn));
            TransactionUnLockAPI(pTransc);
            return rv;
        }

        if(RVSIP_TRANSPORT_CONN_TYPE_CLIENT == eConnectionType)
        {
            *phConn = pTransc->hActiveConn;
        }
    }
    TransactionUnLockAPI(pTransc);
    return rv;

}



/***************************************************************************
 * RvSipTransactionSetCallId
 * ------------------------------------------------------------------------
 * General: Sets the transaction's Call-Id value.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction from which to get Call-Id value.
 *          strCallId    - The call id string. Must be NULL terminated
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetCallId(
                                     IN  RvSipTranscHandle hTransc,
                                     IN  RvChar           *strCallId)
{
    Transaction        *pTransc;

    RvStatus           rv;
    RvInt32                offset;
    RPOOL_ITEM_HANDLE       ptr;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == strCallId)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    /*copy the call-id to the transaction page*/
    rv = RPOOL_AppendFromExternal(pTransc->pMgr->hGeneralPool,
                                  pTransc->memoryPage,
                                  strCallId, (RvInt)strlen(strCallId)+1, RV_FALSE,
                                  &offset, &ptr);

    if(rv != RV_OK)
    {
        TransactionUnLockAPI(pTransc);
        return rv;
    }

    /*set the call-id pool information*/
    TransactionSetCallID(pTransc, pTransc->pMgr->hGeneralPool,
        pTransc->memoryPage, offset);
#ifdef SIP_DEBUG
    pTransc->pCallId = RPOOL_GetPtr(pTransc->strCallId.hPool,
                                         pTransc->strCallId.hPage,
                                         pTransc->strCallId.offset);
#endif

    TransactionUnLockAPI(pTransc);
    return RV_OK;

}

/***************************************************************************
 * RvSipTransactionSendToFirstRoute
 * ------------------------------------------------------------------------
 * General: Sets the loose route parameter for sending a request. If the transaction
 *          sends the message to a loose route proxy (the next hop proxy) the bSendToFirstRouteHeader
 *          will set to RV_TRUE. If the transaction sends the message to a strict route
 *          proxy (the next hop proxy) the bSendToFirstRouteHeader will set to RV_FALSE.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The handle to the transaction is invalid.
 *               RV_ERROR_NULLPTR - The pointer to the strCallId buffer is
 *               invalid.
 *               RV_InsuffitientBuffer - The given buffer is not large enough.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc      - The transaction to set the bSendToFirstRouteHeader in.
 *          bSendToFirstRouteHeader - The bSendToFirstRouteHeader value to set in the transaction
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSendToFirstRoute(IN RvSipTranscHandle hTransc,
                                                         IN RvBool           bSendToFirstRouteHeader)
{
    Transaction  *pTransc;
    RvStatus      rv = RV_OK;


    if(hTransc == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTransc = (Transaction *)hTransc;

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = RvSipTransmitterSetUseFirstRouteFlag(pTransc->hTrx,bSendToFirstRouteHeader);

    TransactionUnLockAPI(pTransc);

    return rv;
}


/***************************************************************************
 * RvSipTransactionGetCallId
 * ------------------------------------------------------------------------
 * General: Returns the transaction's Call-Id value. The Call-Id is copied
 *          to the given buffer.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The handle to the transaction is invalid.
 *               RV_ERROR_NULLPTR - The pointer to the strCallId buffer is
 *               invalid.
 *               RV_InsuffitientBuffer - The given buffer is not large enough.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction from which to get Call-Id value.
 * Output     strCallId - the call id.
 *            pActualSize - if present, will be filled with the actual size 
 *                          of the buffer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetCallId(
                                            IN  RvSipTranscHandle hTransc,
                                            IN  RvInt32          bufSize,
                                            OUT  RvChar           *strCallId,
                                            OUT RvInt32          *pActualSize)

{
    Transaction        *pTransc;
    RvStatus           retStatus;
    RvInt32            actualSize;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == strCallId)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if ((NULL == pTransc->strCallId.hPool) ||
        (NULL_PAGE == pTransc->strCallId.hPage) ||
        (UNDEFINED == pTransc->strCallId.offset))
    {
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_INVALID_HANDLE;
    }
    actualSize = RPOOL_Strlen(pTransc->strCallId.hPool,
                              pTransc->strCallId.hPage,
                              pTransc->strCallId.offset) + 1;
    if (NULL != pActualSize)
        *pActualSize = actualSize;
    
    if (bufSize < actualSize)
    {
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }

    retStatus = RPOOL_CopyToExternal(pTransc->strCallId.hPool,
                                     pTransc->strCallId.hPage,
                                     pTransc->strCallId.offset,
                                     strCallId, actualSize);
    TransactionUnLockAPI(pTransc);
    return retStatus;
}

/***************************************************************************
 * RvSipTransactionSetFromHeader
 * ------------------------------------------------------------------------
 * General: Returns the transaction's From header object.
 * Return Value: The transaction's From header object.
 *                 If there is no From header for the transaction (NULL
 *                 pointer), NULL is returned. If the transaction handle
 *                 parameter is a NULL pointer, NULL is returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to get From header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetFromHeader(
                                   IN RvSipTranscHandle       hTransc,
                                   IN RvSipPartyHeaderHandle  hFrom)
{
    RvStatus  rv;

    Transaction             *pTransc = (Transaction*)hTransc;
    if(pTransc == NULL || hFrom == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionSetFromHeader - Setting transc 0x%p From header",pTransc));

    /*set the from in the transaction and then copy it onto the transaction page*/
    pTransc->hFromHeader = hFrom;
    SipPartyHeaderSetType(hFrom, RVSIP_HEADERTYPE_FROM);

    if(SipPartyHeaderGetPool(hFrom) == pTransc->pMgr->hGeneralPool &&
       SipPartyHeaderGetPage(hFrom) == pTransc->memoryPage)
    {
        /* the header is already on the transaction page. no need to copy it */
        TransactionUnLockAPI(pTransc);
        return RV_OK;
    }

    rv = TransactionCopyFromHeader(pTransc);

    TransactionUnLockAPI(pTransc);
    return rv;
}

/***************************************************************************
 * RvSipTransactionGetFromHeader
 * ------------------------------------------------------------------------
 * General: Returns the transaction's From header object.
 * Return Value: The transaction's From header object.
 *                 If there is no From header for the transaction (NULL
 *                 pointer), NULL is returned. If the transaction handle
 *                 parameter is a NULL pointer, NULL is returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to get From header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetFromHeader(
                                          IN RvSipTranscHandle        hTransc,
                                          OUT RvSipPartyHeaderHandle *phFrom)
{
    Transaction           *pTransc;


    pTransc = (Transaction *)hTransc;
    if (NULL == pTransc || NULL == phFrom)
    {
        return RV_ERROR_NULLPTR;
    }
    *phFrom = NULL;

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phFrom = pTransc->hFromHeader;
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionGetTransmitter
 * ------------------------------------------------------------------------
 * General: Returns the transmitter object that the transaction is using.
 *          The application can use the transmitter object to hold and
 *          resume message sending.
 *          After address resolution is completed, the transaction calls
 *          the RvSipTransactionFinalDestResolvedEv() callback.
 *          At this stage the application can get the transmitter object from
 *          the transaction and call the RvSipTransmitterHoldSending()
 *          function. As a result, the message will not be sent to the
 *          remote party. The application can then manipulate the DNS list
 *          and the destination address. It is the application responsibility
 *          to call the RvSipTransmitterResumeSending() function so that
 *          the message will be sent to the updated destination.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle
 * Output:   phTrx - The transmitter handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetTransmitter(
                                          IN RvSipTranscHandle        hTransc,
                                          OUT RvSipTransmitterHandle *phTrx)
{
    Transaction           *pTransc;

    pTransc = (Transaction *)hTransc;
    if (NULL == pTransc || NULL == phTrx)
    {
        return RV_ERROR_NULLPTR;
    }
    *phTrx = NULL;

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phTrx = pTransc->hTrx;
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}


/***************************************************************************
 * RvSipTransactionSetToHeader
 * ------------------------------------------------------------------------
 * General: Returns the transaction's From header object.
 * Return Value: The transaction's From header object.
 *                 If there is no From header for the transaction (NULL
 *                 pointer), NULL is returned. If the transaction handle
 *                 parameter is a NULL pointer, NULL is returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to get From header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetToHeader(
                                   IN RvSipTranscHandle       hTransc,
                                   IN RvSipPartyHeaderHandle  hTo)
{

    RvStatus  rv;

    Transaction             *pTransc = (Transaction*)hTransc;
    if(pTransc == NULL || hTo == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionSetToHeader - Setting transc 0x%p To header",pTransc));

    /*set the to in the transaction and then copy it onto the transaction page*/
    pTransc->hToHeader = hTo;
    SipPartyHeaderSetType(hTo, RVSIP_HEADERTYPE_TO);

    if(SipPartyHeaderGetPool(hTo) == pTransc->pMgr->hGeneralPool &&
       SipPartyHeaderGetPage(hTo) == pTransc->memoryPage)
    {
        /* the header is already on the transaction page. no need to copy it */
        TransactionUnLockAPI(pTransc);
        return RV_OK;
    }

    rv = TransactionCopyToHeader(pTransc);

    TransactionUnLockAPI(pTransc);
    return rv;

}

/***************************************************************************
 * RvSipTransactionSetViaBranch
 * ------------------------------------------------------------------------
 * General: Sets the top Via branch of the transaction. The transaction will add
 *          the branch to the top Via header of outgoing requests.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc    - The transaction handle.
 *          strBranch    - The Via branch as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetViaBranch(
                                   IN RvSipTranscHandle       hTransc,
                                   IN RvChar                 *strBranch)
{
    RvStatus  rv = RV_OK;

    Transaction             *pTransc = (Transaction*)hTransc;
    if(pTransc == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = RvSipTransmitterSetViaBranch(pTransc->hTrx,strBranch,NULL);


    if(RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "RvSipTransactionSetViaBranch - Transaction 0x%p Failed to  Set branch to transmitter 0x%p (rv=%d)",
                   pTransc, pTransc->hTrx,rv));
    }

    TransactionUnLockAPI(pTransc);
    return rv;
}
/***************************************************************************
 * RvSipTransactionGetToHeader
 * ------------------------------------------------------------------------
 * General: Returns the To address associated with the transaction.
 *          Attempting to alter the To address after
 *          transaction left the Initial state might cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - To header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the call-leg.
 * Output:     phTo     - Pointer to the call-leg To header handle.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetToHeader(
                                          IN  RvSipTranscHandle       hTransc,
                                          OUT RvSipPartyHeaderHandle  *phTo)
{
    Transaction           *pTransc;


    pTransc = (Transaction *)hTransc;
    *phTo = NULL;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phTo = pTransc->hToHeader;
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionSetCSeqStep
 * ------------------------------------------------------------------------
 * General: Returns the transaction's CSeq-Step number.
 * Return Value: The transaction's CSeq-Step number.
 *                 If there is no CSeq-Step value for the transaction, -1
 *                 is returned. If the transaction handle
 *                 parameter is a NULL pointer, UNDEFINED is returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to get CSeq-Step value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetCSeqStep(
                                            IN RvSipTranscHandle hTransc,
                                            IN RvInt32          cseqStep)
{
    RvStatus  rv;

    Transaction             *pTransc = (Transaction*)hTransc;
    if(pTransc == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionSetCSeqStep - Setting transc 0x%p CSeq step to %d",
              pTransc,cseqStep));

	rv = SipCommonCSeqSetStep(cseqStep,&pTransc->cseq); 
	if (rv != RV_OK)
	{
		TransactionUnLockAPI(pTransc);
		RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
			"RvSipTransactionSetCSeqStep - Transc 0x%p failed to set CSeq step %d",
			pTransc,cseqStep));
		return rv;
	}
   
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}
/***************************************************************************
 * RvSipTransactionGetCSeqStep
 * ------------------------------------------------------------------------
 * General: Gets the transaction's CSeq-Step number.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - To header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to get CSeq-Step value.
 * Output:  pCSeqStep - The transaction's CSeq step number.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetCSeqStep(
                                            IN  RvSipTranscHandle hTransc,
                                            OUT RvInt32          *pCSeqStep)
{
	RvStatus      rv; 
    Transaction  *pTransc;


    pTransc = (Transaction *)hTransc;
    *pCSeqStep = UNDEFINED;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipCommonCSeqGetStep(&pTransc->cseq,pCSeqStep); 
	if (rv != RV_OK)
	{
		RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
			"RvSipTransactionGetCSeqStep - Transc 0x%p failed to retrieve CSeq step",
			pTransc));
		TransactionUnLockAPI(pTransc);
		return rv;
	}

    TransactionUnLockAPI(pTransc);
    return RV_OK;
}


/***************************************************************************
 * RvSipTransactionGetMethodStr
 * ------------------------------------------------------------------------
 * General: Returns the transaction method.
 * Return Value: RV_OK - the method was successfully copied to the buffer.
 *               RV_InsuffitientBuffer - The given buffer is not long enough.
 *               RV_ERROR_INVALID_HANDLE - The transaction handle is invalid.
 *               RV_ERROR_NULLPTR - The pointer to the buffer is invalid.
 *               RV_ERROR_NOT_FOUND - The method is enumerated thus no string can be
 *                             found.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to get method.
 *          bufferSize - The buffer's size.
 * Output:  strMethodBuffer - The buffer to be filled with the method.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetMethodStr(
                                          IN RvSipTranscHandle  hTransc,
                                          IN RvUint32          bufferSize,
                                          OUT RvChar           *strMethodBuffer)
{
    Transaction        *pTransc;

    RvUint32           length;
    RvChar             *strMethod;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == strMethodBuffer)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    if(pTransc->eMethod != SIP_TRANSACTION_METHOD_OTHER)
    {
        strMethod = TransactionGetStringMethodFromEnum(pTransc->eMethod);
    }
    else
    {
        strMethod = pTransc->strMethod;
    }

    if (strMethod == NULL)
    {
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_NOT_FOUND;
    }
    length = (RvUint32)strlen(strMethod)+1;
    if (length > bufferSize)
    {
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }
    strcpy(strMethodBuffer, strMethod);
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}


/***************************************************************************
 * RvSipTransactionGetResponseCode
 * ------------------------------------------------------------------------
 * General: Gets the transaction's response code. 0 if there is no relevant
 *          response code (If a response hasn't yet been sent/received).
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - To header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to get response code.
 * Output:  pResponseCode - The transaction response code.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetResponseCode(
                                         IN  RvSipTranscHandle hTransc,
                                           OUT RvUint16          *pResponseCode)
{
    Transaction        *pTransc;


    pTransc = (Transaction *)hTransc;
    *pResponseCode = 0;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *pResponseCode = pTransc->responseCode;
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}


/***************************************************************************
 * RvSipTransactionGetCurrentState
 * ------------------------------------------------------------------------
 * General: Gets the transaction current state
 * Return Value: RV_ERROR_INVALID_HANDLE - if the given transaction leg handle is invalid
 *               RV_ERROR_NULLPTR    - Bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 * Output:     peState -  The transaction current state.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetCurrentState(
                                        IN  RvSipTranscHandle     hTransc,
                                        OUT RvSipTransactionState *peState)
{
    Transaction           *pTransc;


    *peState = RVSIP_TRANSC_STATE_UNDEFINED;
    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *peState = pTransc->eState;
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}
/***************************************************************************
 * RvSipTransactionGetCancelTransc
 * ------------------------------------------------------------------------
 * General: Gets the handle to the transaction that cancels the transaction
 *          given as a parameter. When calling the RvSipTransactionCancel()
 *          function a new cancel transaction is created. The
 *          RvSipTransactionGetCancelTransc() returns the handle to this
 *          transaction.
 *          If the transaction given as a parameter was never cancelled NULL
 *          is returned.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 * Output:     phCancelTransc -  The cancel transaction handle.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetCancelTransc(
                                        IN  RvSipTranscHandle     hTransc,
                                        OUT RvSipTranscHandle    *phCancelTransc)
{

    Transaction           *pTransc;

    *phCancelTransc = NULL;
    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phCancelTransc = pTransc->hCancelTranscPair;
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}


/***************************************************************************
 * RvSipTransactionGetReceivedMsg
 * ------------------------------------------------------------------------
 * General: Gets the message that was received by the transaction. You can
 *          call this function from the state changed call back function
 *          when the new state indicates that a message was received.
 *          If there is no valid received message, NULL will be returned.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 * Output:     phMsg -  pointer to the received message.
 ***************************************************************************/
RVAPI  RvStatus  RVCALLCONV RvSipTransactionGetReceivedMsg(
                                                IN  RvSipTranscHandle     hTransc,
                                                 OUT RvSipMsgHandle       *phMsg)
{
    Transaction           *pTransc;

    *phMsg = NULL;
    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionGetReceivedMsg - transc= 0x%p returned handle= 0x%p",
              pTransc,pTransc->hReceivedMsg));
    *phMsg = TransactionGetReceivedMsg(pTransc);
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionGetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Gets the message that is going to be sent by the transaction.
 *          It is only valid in states where the transaction is about to
 *          send a message. These states are:
 *          IDLE, SERVER_GEN_REQUEST_RCVD, CLIENT_INVITE_FINAL_RESPONSE_RCVD,
 *          SERVER_INVITE_REQUEST_RCVD, SERVER_INVITE_REL_PROV_RESPONSE_SENT,
 *          SERVER_CANCEL_REQUEST_RCVD.
 *          You can call this function before you call API functions such as
 *          Request(), Respond() and Ack(), or from
 *          the state changed call back function when the new state indicates
 *          that a message is going to be sent.
 *          Note: The message you receive from this function is not complete.
 *          In some cases it might even be empty.
 *          You should use this function to add headers to the message before
 *          it is sent. To view the complete message use event message to
 *          send.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 * Output:     phMsg   -  pointer to the message.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetOutboundMsg(
                                     IN  RvSipTranscHandle     hTransc,
                                     OUT RvSipMsgHandle       *phMsg)
{
    Transaction    *pTransc;
    RvSipMsgHandle  hOutboundMsg = NULL;
    RvStatus        rv;

    if (NULL == hTransc || phMsg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phMsg = NULL;
    pTransc = (Transaction *)hTransc;
    
    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (NULL != pTransc->hOutboundMsg)
    {
        hOutboundMsg = pTransc->hOutboundMsg;
    }
    else if (RV_TRUE == IsStateToSendMsg(pTransc))
    {
        rv = RvSipMsgConstruct(pTransc->pMgr->hMsgMgr,
                               pTransc->pMgr->hMessagePool,
                               &hOutboundMsg);
        if (RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                       "RvSipTransactionGetOutboundMsg - transc= 0x%p error in constructing message",
                      pTransc));
            TransactionUnLockAPI(pTransc);
            return rv;
        }
        pTransc->hOutboundMsg = hOutboundMsg;
    }
    else
    {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                       "RvSipTransactionGetOutboundMsg - transc= 0x%p cannot get outbound message at state %s",
                      pTransc,TransactionGetStateName(pTransc->eState)));
            TransactionUnLockAPI(pTransc);
            return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionGetOutboundMsg - transc= 0x%p returned handle= 0x%p",
              pTransc,hOutboundMsg));

    *phMsg = hOutboundMsg;

    TransactionUnLockAPI(pTransc);
    return RV_OK;
}




/***************************************************************************
 * RvSipTransactionGetCancelledTransc
 * ------------------------------------------------------------------------
 * General: Gets the handle to the transaction that a cancel transaction
 *          cancelled. You can call this function when cancel is received
 *          to get the handle to the cancelled transaction.
 *          If there is not cancelled transaction NULL is returned.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The cancel transaction handle.
 * Output:     phCancelTransc -  The transaction cancelled by the cancel transaction.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetCancelledTransc(
                                        IN  RvSipTranscHandle     hTransc,
                                        OUT RvSipTranscHandle    *phCancelledTransc)
{

    Transaction           *pTransc;

    *phCancelledTransc = NULL;
    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phCancelledTransc = pTransc->hCancelTranscPair;
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}



/***************************************************************************
 * RvSipTransactionMgrSetProxyEvHandlers
 * ------------------------------------------------------------------------
 * General: This function is deprecated. All needed events can be found
 *          in the RvSipTransactionMgrEvHandlers structure.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr - A handle to the transaction manager.
 *          pHandlers  - Pointer to structure containing application event
 *                        handler pointers.
 *          evHandlerStructSize - The size of the event handler structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionMgrSetProxyEvHandlers(
                        IN RvSipTranscMgrHandle            hTranscMgr,
                        IN RvSipTransactionProxyEvHandlers *pHandlers,
                        IN RvUint32                        evHandlerStructSize)
{
    TransactionMgr      *pTranscMgr;
    RvLogSource*        logId;

    RvInt32 minStructSize = (evHandlerStructSize < sizeof(RvSipTransactionProxyEvHandlers)) ?
                              evHandlerStructSize : sizeof(RvSipTransactionProxyEvHandlers);

    pTranscMgr = (TransactionMgr *)hTranscMgr;

    if ((NULL == hTranscMgr) ||
        (NULL == pHandlers) )
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

    /* The manager signs in is the application manager:
       Set application's calbacks*/
    memset(&(pTranscMgr->proxyEvHandlers), 0, sizeof(RvSipTransactionProxyEvHandlers));
    memcpy(&(pTranscMgr->proxyEvHandlers), pHandlers, minStructSize);

    RvLogDebug(logId ,(logId ,
              "RvSipTransactionMgrSetProxyEvHandlers - Proxy event handlers were successfully set to the transaction manager"));
    RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionSetOwner
 * ------------------------------------------------------------------------
 * General: Sets the owner of a given transaction.
 *          You can call this function for application transactions only.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc      - The transaction handle.
 *          hOwnerHandle - The new transaction owner handle
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionSetOwner(
                                        IN  RvSipTranscHandle      hTransc,
                                        IN  RvSipTranscOwnerHandle hOwnerHandle)
{

    Transaction           *pTransc;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(SIP_TRANSACTION_OWNER_APP != pTransc->eOwnerType)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "RvSipTransactionSetOwner: Transc 0x%p - illegal type %d",
            pTransc,pTransc->eOwnerType));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    TransactionInitiateOwner(pTransc, (void*)hOwnerHandle, SIP_TRANSACTION_OWNER_APP, NULL);
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionGetOwner
 * ------------------------------------------------------------------------
 * General: returns the handle to the owner of a given transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 * Output:     phOwnerHandle -  The transaction owner handle
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetOwner(
                                        IN  RvSipTranscHandle      hTransc,
                                        OUT RvSipTranscOwnerHandle *phOwnerHandle)
{

    Transaction           *pTransc;

    *phOwnerHandle = NULL;
    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phOwnerHandle = (RvSipTranscOwnerHandle)pTransc->pOwner;
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionIsStackOwner
 * ------------------------------------------------------------------------
 * General: Indicates if the owner of the transaction is one of the SIP stack
 *          objects or not.
 *          This is an internal function, written for the SipServer.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 * Output:    pbStackOwner -  RV_TRUE if it is a stack owner.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionIsStackOwner(
                                        IN  RvSipTranscHandle      hTransc,
                                        OUT RvBool*                pbStackOwner)
{

    Transaction    *pTransc;
    
    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc || pbStackOwner == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pTransc->eOwnerType == SIP_TRANSACTION_OWNER_CALL ||
        pTransc->eOwnerType == SIP_TRANSACTION_OWNER_TRANSC_CLIENT)
    {
        *pbStackOwner = RV_TRUE;
    }
    else
    {
        *pbStackOwner = RV_FALSE;
    }
    
    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "RvSipTransactionIsStackOwner - Transaction 0x%p - owner type=%d, returning %d.", pTransc, pTransc->eOwnerType, *pbStackOwner));

    TransactionUnLockAPI(pTransc);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionSetAppHandle
 * ------------------------------------------------------------------------
 * General: Sets the application handle to the given transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 *          hAppHandle -  The application handle
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionSetAppHandle(
                                        IN  RvSipTranscHandle      hTransc,
                                        IN RvSipAppTranscHandle    hAppHandle)
{

    Transaction *pTransc;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "RvSipTransactionSetAppHandle - Transaction 0x%p - set appHandle to 0x%p",pTransc,hAppHandle));

    pTransc->hAppHandle = hAppHandle;
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionGetAppHandle
 * ------------------------------------------------------------------------
 * General: returns the application handle to the given transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 * Output:     phAppHandle -  The application handle
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetAppHandle(
                                        IN  RvSipTranscHandle      hTransc,
                                        OUT RvSipAppTranscHandle *phAppHandle)
{

    Transaction           *pTransc;

    *phAppHandle = NULL;
    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phAppHandle = pTransc->hAppHandle;
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}
/***************************************************************************
 * RvSipTransactionGetRequestUri
 * ------------------------------------------------------------------------
 * General: Return the Request URI the transaction is using.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc    - The transaction handle.
 * Output:  hReqUri    - The Request URI.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetRequestUri(
                                   IN  RvSipTranscHandle     hTransc,
                                   OUT RvSipAddressHandle    *hReqUri)
{
    Transaction           *pTransc;

    *hReqUri = NULL;
    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionGetRequestUri - transc= 0x%p returned request uri handle= 0x%p",
              pTransc,pTransc->hRequestUri ));
    *hReqUri = pTransc->hRequestUri;
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionDNSContinue
 * ------------------------------------------------------------------------
 * General: Creates new transaction and copies all parameters from the
 *          original transaction to the new transaction.
 *          This functions also terminates the original transaction unless
 *          the transaction received 503 response on INVITE.
 *          If the function failed the original transaction will not be
 *          terminated.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hOrigTransaction    - The source transaction handle.
 *          owner               - pointer to the new transaction owner.
 * Output:  hNewTransaction     - Pointer to the new transaction handler.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionDNSContinue(
                               IN  RvSipTranscHandle        hOrigTransaction,
                               IN  RvSipTranscOwnerHandle   hOwner,
                               OUT RvSipTranscHandle        *hCloneTrans)
{
    RvStatus               retCode     = RV_OK;
    Transaction*            pOrigTrans  = NULL;

    if(hOrigTransaction == NULL)
    {
        return (RV_ERROR_UNKNOWN);
    }

    pOrigTrans = (Transaction *)hOrigTransaction;

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT

    if (RV_OK != TransactionLockAPI(pOrigTrans))
    {
        return (RV_ERROR_INVALID_HANDLE);
    }
    RvLogDebug(pOrigTrans->pMgr->pLogSrc,(pOrigTrans->pMgr->pLogSrc,
              "RvSipTransactionDNSContinue - transaction 0x%p is cloned", pOrigTrans));

    retCode = SipTransactionDNSContinue(hOrigTransaction, hOwner, RV_TRUE, hCloneTrans);
    TransactionUnLockAPI(pOrigTrans);
    return (retCode);

#else /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */
    RvLogError(pOrigTrans->pMgr->pLogSrc ,(pOrigTrans->pMgr->pLogSrc ,
        "RvSipTransactionDNSContinue - DNS Enhanced Features is not supported"));
    *hCloneTrans = NULL;
    retCode = RV_ERROR_UNKNOWN;
    RV_UNUSED_ARG(hOwner);

    return (retCode);
#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */
}

/***************************************************************************
 * RvSipTransactionDNSGetList
 * ------------------------------------------------------------------------
 * General: retrieves the DNS list object from the transaction object.
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc    - Transaction Handle
 * Output   phDnsList        - DNS list handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionDNSGetList(IN RvSipTranscHandle               hTransc,
                                                      OUT RvSipTransportDNSListHandle *phDnsList)

{
    Transaction                *pTrans;
    RvStatus                retCode;


    if((hTransc == NULL) || (phDnsList == NULL))
    {
        return RV_ERROR_BADPARAM;
    }

    pTrans = (Transaction *)hTransc;

    if (RV_OK != TransactionLockAPI(pTrans))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    retCode = RvSipTransmitterDNSGetList(pTrans->hTrx,phDnsList);

    TransactionUnLockAPI(pTrans);

    return retCode;
}

/***************************************************************************
 * RvSipTransactionIsUAC
 * ------------------------------------------------------------------------
 * General: Return RV_TRUE if the transaction is a client transaction, and RV_FALSE
 *          if the transaction is a server transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc    - The transaction handle.
 * Output:  bIsUAC     - The type of the transaction (RV_TRUE if the transaction is
 *                       a client transaction, and RV_FALSE if the transaction is a
 *                       server transaction
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionIsUAC(IN  RvSipTranscHandle hTransc,
                                                 OUT RvBool          *bIsUAC)
{
    Transaction           *pTransc;


    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    *bIsUAC = pTransc->bIsUac;

    TransactionUnLockAPI(pTransc);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionDetachOwner
 * ------------------------------------------------------------------------
 * General: Called when the transaction's old owner is not it's owner any
 *          more. The transaction will begin acting as a stand alone transaction.
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
RVAPI RvStatus RVCALLCONV RvSipTransactionDetachOwner(
                                             IN RvSipTranscHandle hTransc)
{
    Transaction           *pTransc;

    RvStatus             rv = RV_OK;
    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTransactionDetachOwner(hTransc);

    TransactionUnLockAPI(pTransc);
    return rv;
}


/***************************************************************************
 * RvSipTransactionSetIsProxyFlag
 * ------------------------------------------------------------------------
 * General: Sets the isProxy flag of the transaction.
 *          This flag defines if the transaction should behave as a proxy
 *          transaction.
 *          The transaction isProxy flag is set when the transaction is created
 *          according to the stack configuration. (If the stack is configured
 *          as a proxy, the isProxy flag will be set to RV_TRUE automatically.
 *          For server transaction you must not call this function from the
 *          transcCreated callback. (You can use the stateChanged callback).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc  - The transaction handle.
 *          bIsProxy - The value to set in the is proxy flag.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionSetIsProxyFlag(
                                        IN  RvSipTranscHandle   hTransc,
                                        IN  RvBool              bIsProxy)
{
    Transaction           *pTransc;

    RvStatus             rv = RV_OK;
    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    RvMutexLock(&pTransc->pMgr->hLock,pTransc->pMgr->pLogMgr);

    pTransc->bIsProxy = bIsProxy;
    RvMutexUnlock(&pTransc->pMgr->hLock,pTransc->pMgr->pLogMgr);

    TransactionUnLockAPI(pTransc);
    return rv;
}


/***************************************************************************
 * RvSipTransactionSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the transaction
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hTransc - Handle to the transaction object.
 *           eStringName   - The string the user wish to set
 *         pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetRpoolString(
                              IN  RvSipTranscHandle     hTransc,
                              IN  RvSipTransactionStringName  eStringName,
                              IN  RPOOL_Ptr                 *pRpoolPtr)
{
    Transaction           *pTransc;

    RvStatus             rv = RV_OK;
    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    switch(eStringName)
    {
    case RVSIP_TRANSC_VIA_BRANCH:
        rv = RvSipTransmitterSetViaBranch(pTransc->hTrx,NULL,pRpoolPtr);
        break;
    default:
          RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "RvSipTransactionSetRpoolString - transc 0x%p, string type is not supported by this function",hTransc));
        rv = RV_ERROR_BADPARAM;
    }

    TransactionUnLockAPI(pTransc);
    return rv;

}

/***************************************************************************
 * RvSipTranscGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this transaction
 *          belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc         - Handle to the transaction.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscGetStackInstance(
                                   IN   RvSipTranscHandle      hTransc,
                                   OUT  void*      *phStackInstance)
{
    Transaction           *pTransc;

    pTransc = (Transaction *)hTransc;

    if (NULL == phStackInstance)
    {
        return RV_ERROR_NULLPTR;
    }
    *phStackInstance = NULL;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phStackInstance = pTransc->pMgr->hStack;

    TransactionUnLockAPI(pTransc);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransactionSetCompartment
 * ------------------------------------------------------------------------
 * General: Associates the transaction to a SigComp compartment. The transaction will use
 *          this compartment in the message compression process.
 *          Note: The message will be compressed only if the remote URI includes the
 *          comp=sigcomp parameter.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc      - The transaction handle.
 *            hCompartment - The handle to the SigComp compartment.
 *
 * NOTE: Function deprecated
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetCompartment(
                            IN RvSipTranscHandle      hTransc,
                            IN RvSipCompartmentHandle hCompartment)
{
    RV_UNUSED_ARG(hTransc);
    RV_UNUSED_ARG(hCompartment);

    return RV_ERROR_ILLEGAL_ACTION;
}

/***************************************************************************
 * RvSipTransactionGetCompartment
 * ------------------------------------------------------------------------
 * General: Retrieves the SigComp compartment associated with the transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTransc       - The transaction handle.
 *         phCompartment - The handle to the SigComp compartment.
 *
 * NOTE: Function deprecated
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetCompartment(
                            IN  RvSipTranscHandle       hTransc,
                            OUT RvSipCompartmentHandle *phCompartment)
{
    RV_UNUSED_ARG(hTransc);
    *phCompartment = NULL;

    return RV_ERROR_ILLEGAL_ACTION;
}

/***************************************************************************
 * RvSipTransactionDisableCompression
 * ------------------------------------------------------------------------
 * General: Disables the compression mechanism in a transaction. This means
 *          that even if the message indicates compression, it will not be
 *          compressed.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTransc       - The transaction handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionDisableCompression(
                                IN  RvSipTranscHandle hTransc)
{
#ifdef RV_SIGCOMP_ON
    RvStatus   rv         = RV_OK;
    Transaction *pTransc = (Transaction*)hTransc;

    if(pTransc == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionDisableCompression- disabling the compression in Transaction 0x%p",
              pTransc));

    pTransc->bSigCompEnabled = RV_FALSE; /* Mark this flag locally */
    if (pTransc->hTrx != NULL)
    {
        rv = SipTransmitterDisableCompression(pTransc->hTrx);
    }

    TransactionUnLockAPI(pTransc); /*unlock the transaction*/

    return rv;
#else /* RV_SIGCOMP_ON */
    RV_UNUSED_ARG(hTransc);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* RV_SIGCOMP_ON */
}

/***************************************************************************
 * RvSipTransactionGetOutboundMsgCompType
 * ------------------------------------------------------------------------
 * General: Retrieves the compression type of the Transaction outbound
 *          message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - The transaction handle.
 * Output:    peCompType - The compression type.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetOutboundMsgCompType(
                                         IN  RvSipTranscHandle  hTransc,
                                         OUT RvSipCompType     *peCompType)
{
#ifdef RV_SIGCOMP_ON
    RvStatus       rv        = RV_OK;
    Transaction   *pTransc   = (Transaction*)hTransc;
    *peCompType = RVSIP_COMP_UNDEFINED;

    if(pTransc == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *peCompType = TransactionGetOutboundMsgCompType(pTransc);

    TransactionUnLockAPI(pTransc); /*unlock the transaction*/

    return RV_OK;

#else /* RV_SIGCOMP_ON */
    RV_UNUSED_ARG(hTransc);
    RV_UNUSED_ARG(peCompType);
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* RV_SIGCOMP_ON */
}

/***************************************************************************
 * RvSipTransactionIsCompartmentRequired
 * ------------------------------------------------------------------------
 * General: Checks if the transaction has to be related to a compartment.
 *          The function should be used by applications that would
 *          like to manage the stack compartments manually. For instance,
 *          if you wish to relate an incoming transaction object, which handles
 *          a sigComp message, to a specific compartment, you can call this
 *          function in the RvSipTransactionCreatedEv() callback and attach
 *          your chosen compartment to the transaction is needed.
 *          When the callback returns the stack will automatically attach
 *          the transaction object to compartment if it does not have one yet.
 * Return Value: RvStatus
 *
 * NOTE: Function deprecated
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc     - The transaction handle.
 * Output:    pbRequired  - Indication if a compartment is required.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionIsCompartmentRequired(
                                         IN  RvSipTranscHandle  hTransc,
                                         OUT RvBool            *pbRequired)
{
    Transaction   *pTransc   = (Transaction*)hTransc;

    if(pTransc == NULL || pbRequired == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *pbRequired = RV_FALSE;

    return RV_ERROR_ILLEGAL_ACTION;
}

/***************************************************************************
* RvSipTransactionGetReceivedFromAddress
* ------------------------------------------------------------------------
* General: Gets the address from which the transaction received it's last message.
* Return Value: RvStatus.
* ------------------------------------------------------------------------
* Arguments:
* Input:
* hTransaction - Handle to the transaction
* Output:
* pAddr     - basic details about the received from address
* pOptions  - Options about the the address details. If NULL, then only basic
*             data will be retrieved
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetReceivedFromAddress(
                            IN  RvSipTranscHandle          hTransaction,
                            OUT RvSipTransportAddr        *pAddr,
                            OUT RvSipTransportAddrOptions *pOptions)
{
    RvStatus        rv      = RV_OK;
    Transaction*   pTransc  = (Transaction*)hTransaction;

    if(pTransc == NULL || pAddr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    SipTransportConvertCoreAddressToApiTransportAddr(&pTransc->receivedFrom.addr,pAddr);
    pAddr->eTransportType = pTransc->receivedFrom.eTransport;
    if (NULL != pOptions)
    {
#ifdef RV_SIGCOMP_ON
        pOptions->eCompression = pTransc->eIncomingMsgComp;
#else
        pOptions->eCompression = RVSIP_COMP_UNDEFINED;
#endif /*RV_SIGCOMP_ON*/
    }
    TransactionUnLockAPI(pTransc); /*unlock the transaction*/
    return rv;
}

/***************************************************************************
 * RvSipTransactionSetTimers
 * ------------------------------------------------------------------------
 * General: Sets new timer values to the transaction.
 *          According to RFC3261, the transaction has to set different timers 
 *          during its life cycle and perform different actions when the timers 
 *          expire. For example, after sending a final response, the transaction 
 *          has to set a timer to the value of 32,000 MSec. 
 *          When this timer expires the transaction must terminate.
 *          The values of the transaction timers are taken from
 *			the stack configuration and are defined upon initialization.
 *			The application can use the RvSipTransactionSetTimers() API function 
 *          to change the different timer values of the transaction.
 *			The RvSipTimers structure received by this function contains all the 
 *          transaction configurable timers. 
 *          You can set values to the timers you want to configure. You can set
 *          UNDEFINED to timers you wish the stack to calculate.
 *          If you set a timer value to UNDEFINED, and this timer cannot be
 *          calculated from other timers in the structure, the timer value will
 *          be taken from the stack configuration.
 *          The RvSipTimers structure also enables you to change the number
 *          of retransmissions performed by the transaction. The default number
 *          of retransmissions is calculated using the different transaction
 *          timers. However, the application can set a different number and change
 *          the retransmission count.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hTransc - Handle to the transaction object.
 *           pTimers - Pointer to the struct contains all the timeout values.
 *           sizeOfTimersStruct - Size of the RvSipTimers structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetTimers(
                              IN  RvSipTranscHandle     hTransc,
                              IN  RvSipTimers           *pTimers,
                              IN  RvInt32               sizeOfTimersStruct)
{
    Transaction           *pTransc;
    RvStatus             rv = RV_OK;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    rv = TransactionSetTimers(pTransc,pTimers);


    TransactionUnLockAPI(pTransc);
    RV_UNUSED_ARG(sizeOfTimersStruct);
    return rv;

}

/***************************************************************************
 * RvSipTransactionSetRejectStatusCodeOnCreation
 * ------------------------------------------------------------------------
 * General: This function can be used synchronously from the 
 *          RvSipTransactionCreatedEv when the application decides not to handle
 *          the transaction. A transaction that is not handled by the application
 *          is handled automatically by the stack and the stack rejects the request
 *          that created the transaction with a default response code.   
 *          The application can change the default response code by calling the
 *          function RvSipTransactionSetRejectStatusCodeOnCreation() and suppling
 *          a new response code.
 *          Note that after the application decides not to handle the transaction,
 *          it will not get any further callbacks
 *          that relate to this transaction. The application will not get the
 *          RvSipTransactionMsgToSendEv for the reject response message or the 
 *          Terminated state for the transaction object.
 *          Note: When this function is used to reject a request, the application
 *          cannot use the outbound message mechanism to add information
 *          to the outgoing response message. If you wish to change the response
 *          message you must use the regular respond mechanism.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hTransc - Handle to the transaction object.
 *           rejectStatusCode - The reject status code for rejecting the request
 *                              that created this object. The value must be
 *                              between 300 to 699.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetRejectStatusCodeOnCreation(
                              IN  RvSipTranscHandle      hTransc,
                              IN  RvUint16               rejectStatusCode)
{
    Transaction          *pTransc;
    RvStatus             rv = RV_OK;

    pTransc = (Transaction *)hTransc;

    if (NULL == pTransc)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "RvSipTransactionSetRejectStatusCodeOnCreation - transaction 0x%p, setting reject status to %d",
              pTransc, rejectStatusCode));

    /*invalid status code*/
    if (rejectStatusCode < 300 || rejectStatusCode >= 700)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "RvSipTransactionSetRejectStatusCodeOnCreation - Failed. transaction 0x%p. %d is not a valid reject response code",
                  pTransc,rejectStatusCode));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_BADPARAM;
    }

    if(pTransc->bIsUac == RV_TRUE ||
       pTransc->eState != RVSIP_TRANSC_STATE_IDLE)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "RvSipTransactionSetRejectStatusCodeOnCreation(pTransc=0x%p) - Failed. Cannot be called in state %s or if isUac=%d",
                  pTransc,TransactionGetStateName(pTransc->eState),
                  pTransc->bIsUac));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_BADPARAM;

    }

    pTransc->responseCode = rejectStatusCode;

    TransactionUnLockAPI(pTransc);
    return rv;
}
/***************************************************************************
 * RvSipTransactionSetForceOutboundAddrFlag 
 * ------------------------------------------------------------------------
 * General: Sets the force outbound addr flag. This flag forces the transaction
 *          to send request messages to the outbound address regardless of the
 *          message content.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hTransc           - The transaction handle.
 *     	    bForceOutboundAddr - The flag value to set.
 **************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionSetForceOutboundAddrFlag(
                                IN  RvSipTranscHandle   hTransc,
			                    IN  RvBool              bForceOutboundAddr)
{
	Transaction           *pTransc;
    RvStatus               rv = RV_OK;
    
	pTransc = (Transaction *)hTransc;
    
	if (NULL == pTransc)
	{
		return RV_ERROR_NULLPTR;
	}

    if (RV_OK != TransactionLockAPI(pTransc))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
		      "RvSipTransactionSetForceOutboundAddrFlag - transaction= %x bForceOutboundAddr was set to %d",
			  pTransc,bForceOutboundAddr));
    
    rv = RvSipTransmitterSetForceOutboundAddrFlag(pTransc->hTrx,bForceOutboundAddr); 
    TransactionUnLockAPI(pTransc);
    return rv;    
}

#if (RV_NET_TYPE & RV_NET_SCTP)
/******************************************************************************
 * RvSipTransactionSetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Sets SCTP transport parameters to the transaction object.
 *          All possible SCTP parameters are located in the 
 *          RvSipTransportSctpMsgSendingParams structure and will be used 
 *          only if the transaction will use SCTP as the destination transport.
 *          Note: This function is optional. If specific SCTP parameters are
 *          not set, the default SCTP parameters will be used.
 *          Note: Calling this function does not affect the chosen transport.
 *          The transport will be chosen in the address resolution process.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hTransc    - Handle to the Transaction object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams    - Pointer to the structure that contains all SCTP parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetSctpMsgSendingParams(
                    IN  RvSipTranscHandle                  hTransc,
                    IN  RvInt32                            sizeOfParamsStruct,
                    IN  RvSipTransportSctpMsgSendingParams *pParams)
{
    RvStatus rv;
    Transaction  *pTransc = (Transaction*)hTransc;
    RvSipTransportSctpMsgSendingParams params;
    RvUint32 minStructSize = RvMin(((RvUint)sizeOfParamsStruct),sizeof(RvSipTransportSctpMsgSendingParams));
    
    if(pTransc == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    memset(&params,0,sizeof(RvSipTransportSctpMsgSendingParams));
    memcpy(&params,pParams,minStructSize);

    rv = TransactionLockAPI(pTransc);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "RvSipTransactionSetSctpMsgSendingParams - transaction 0x%p - failed to lock the Transaction",
            pTransc));
        return RV_ERROR_INVALID_HANDLE;
    }
    
    SipTransactionSetSctpMsgSendingParams(hTransc, &params);
    
    RvLogInfo(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
        "RvSipTransactionSetSctpMsgSendingParams - transaction 0x%p - SCTP params(stream=%d, flag=0x%x) were set",
        pTransc, pParams->sctpStream, pParams->sctpMsgSendingFlags));
    
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}

/******************************************************************************
 * RvSipTransactionGetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Gets SCTP parameters that are used by the transaction
 *          while sending a message over SCTP.
 *          Note: Only SCTP parameters that were set with the 
 *          RvSipTransactionSetSctpMsgSendingParams() will be returned when
 *          calling this function.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hTransc    - Handle to the Transaction object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams      - Pointer to the struct contains all the parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetSctpMsgSendingParams(
                    IN  RvSipTranscHandle                  hTransc,
                    IN  RvInt32                            sizeOfParamsStruct,
                    OUT RvSipTransportSctpMsgSendingParams *pParams)
{
    RvStatus rv;
    Transaction  *pTransc = (Transaction*)hTransc;
    RvSipTransportSctpMsgSendingParams params;
    RvUint32 minStructSize = RvMin(((RvUint)sizeOfParamsStruct),sizeof(RvSipTransportSctpMsgSendingParams));

    if(pTransc == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = TransactionLockAPI(pTransc);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "RvSipTransactionGetSctpMsgSendingParams - transaction 0x%p - failed to lock the Transaction",
            pTransc));
        return RV_ERROR_INVALID_HANDLE;
    }

    memset(&params,0,sizeof(RvSipTransportSctpMsgSendingParams));
    SipTransactionGetSctpMsgSendingParams(hTransc, &params);
    memcpy(pParams,&params,minStructSize);
    
    RvLogInfo(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
        "RvSipTransactionGetSctpMsgSendingParams - transaction 0x%p - SCTP params(stream=%d, flag=0x%x) were get",
        pTransc, pParams->sctpStream, pParams->sctpMsgSendingFlags));

    TransactionUnLockAPI(pTransc);
    return RV_OK;
}
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipTransactionSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Transaction.
 *          As a result of this operation, all messages, sent by this
 *          Transaction, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hTransc - Handle to the Transaction object.
 *          hSecObj - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetSecObj(
                                        IN  RvSipTranscHandle    hTransc,
                                        IN  RvSipSecObjHandle    hSecObj)
{
    RvStatus     rv;
    Transaction* pTransc = (Transaction*)hTransc;

    if(NULL == pTransc)
    {
        return RV_ERROR_BADPARAM;
    }
    
    RvLogInfo(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
        "RvSipTransactionSetSecObj - Transaction 0x%p - Set Security Object %p",
        pTransc, hSecObj));
    
    /*try to lock the Transaction*/
    rv = TransactionLockAPI(pTransc);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "RvSipTransactionSetSecObj - Transaction 0x%p - failed to lock the Transc",
            pTransc));
        return RV_ERROR_INVALID_HANDLE;
    }
	
	if (pTransc->hSecAgree != NULL)
	{
		/* We do not allow setting security-object when there is a security-agreement attached to this transaction */
		RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
					"RvSipTransactionSetSecObj - Transaction 0x%p - Setting security-object (0x%p) is illegal when the transaction has a security-agreement (security-agreement=0x%p)",
					pTransc, hSecObj, pTransc->hSecAgree));
		TransactionUnLockAPI(pTransc);
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = TransactionSetSecObj(pTransc, hSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "RvSipTransactionSetSecObj - Transaction 0x%p - failed to set Security Object %p(rv=%d:%s)",
            pTransc, hSecObj, rv, SipCommonStatus2Str(rv)));
        TransactionUnLockAPI(pTransc);
        return rv;
    }

    TransactionUnLockAPI(pTransc);
    return RV_OK;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipTransactionGetSecObj
 * ----------------------------------------------------------------------------
 * General: Gets Security Object that was set into the Transaction.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hTransc  - Handle to the Transaction object.
 *  Output: phSecObj - Handle to the Security Object.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetSecObj(
                                        IN  RvSipTranscHandle    hTransc,
                                        OUT RvSipSecObjHandle*   phSecObj)
{
    RvStatus     rv;
    Transaction* pTransc = (Transaction*)hTransc;
    
    if(NULL==pTransc || NULL==phSecObj)
    {
        return RV_ERROR_BADPARAM;
    }
    
    RvLogInfo(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
        "RvSipTransactionGetSecObj - Transaction 0x%p - Get Security Object",
        pTransc));
    
    /*try to lock the Transaction*/
    rv = TransactionLockAPI(pTransc);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "RvSipTransactionGetSecObj - Transaction 0x%p - failed to lock the Transc",
            pTransc));
        return RV_ERROR_INVALID_HANDLE;
    }
    
    *phSecObj = pTransc->hSecObj;
    
    TransactionUnLockAPI(pTransc);
    return RV_OK;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * RvSipTransactionSetSecAgree
 * ------------------------------------------------------------------------
 * General: Sets a security-agreement object to this transaction. If this 
 *          security-agreement object maintains an existing agreement with the 
 *          remote party, the transaction will take upon itself this agreement 
 *          and the data it brings. If not, the transaction will use this security-agreement 
 *          object to negotiate an agreement with the remote party.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransaction   - Handle to the transaction.
 *          hSecAgree      - Handle to the security-agreement.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetSecAgree(
                    IN  RvSipTranscHandle           hTransaction,
                    IN  RvSipSecAgreeHandle         hSecAgree)
{
	Transaction      *pTransaction;
	RvStatus          rv;

    if (hTransaction == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	pTransaction = (Transaction *)hTransaction;
    
	if (RV_OK != TransactionLockAPI(pTransaction))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	/* if the transaction was terminated we do not allow the attachment */
	if (RVSIP_TRANSC_STATE_TERMINATED == pTransaction->eState)
	{
		RvLogError(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
					"RvSipTransactionSetSecAgree - transaction 0x%p - Attaching security-agreement 0x%p is forbidden in state Terminated",
					pTransaction, hSecAgree));
		TransactionUnLockAPI(pTransaction);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	rv = TransactionSecAgreeAttachSecAgree(pTransaction, hSecAgree);
	if (RV_OK != rv)
	{
		RvLogError(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
					"RvSipTransactionSetSecAgree - transaction 0x%p - Failed to attach security-agreement=0x%p (retStatus=%d)",
					pTransaction, hSecAgree, rv));
	}

	TransactionUnLockAPI(pTransaction);

	return rv;    
}

/***************************************************************************
 * RvSipTransactionGetSecAgree
 * ------------------------------------------------------------------------
 * General: Gets the security-agreement object associated with the transaction. 
 *          The security-agreement object captures the security-agreement with 
 *          the remote party as defined in RFC3329.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransaction  - Handle to the transaction.
 *          hSecAgree     - Handle to the security-agreement.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetSecAgree(
									IN   RvSipTranscHandle       hTransaction,
									OUT  RvSipSecAgreeHandle    *phSecAgree)
{
	Transaction          *pTransaction = (Transaction *)hTransaction;
    
    if (NULL == hTransaction)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	if (NULL == phSecAgree)
	{
		return RV_ERROR_BADPARAM;
	}

    if (RV_OK != TransactionLockAPI(pTransaction))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	*phSecAgree = pTransaction->hSecAgree;

	TransactionUnLockAPI(pTransaction);

	return RV_OK;	
}
#endif /* #ifdef RV_SIP_IMS_ON */

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * IsStateToSendMsg
 * ------------------------------------------------------------------------
 * General: returns RV_TRUE is the transaction is in states where it is about
 *          to send a message. These states are:
 *          IDLE, SERVER_GEN_REQUEST_RCVD, CLIENT_INVITE_FINAL_RESPONSE_RCVD,
 *          SERVER_INVITE_REQUEST_RCVD, SERVER_INVITE_REL_PROV_RESPONSE_SENT,
 *          SERVER_CANCEL_REQUEST_RCVD.
 *          Otherwise returns RV_FALSE.
 * Return Value: RvBool
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc - A pointer to the transaction.
 ***************************************************************************/
static RvBool RVCALLCONV IsStateToSendMsg(Transaction *pTransc)
{
    switch (pTransc->eState)
    {
    case RVSIP_TRANSC_STATE_IDLE:
    case RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD:
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD:
    case RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD:
#ifndef RV_SIP_PRIMITIVES
    case RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT:
    case RVSIP_TRANSC_STATE_SERVER_INVITE_PRACK_COMPLETED:    
#endif /* RV_SIP_PRIMITIVES */
    case RVSIP_TRANSC_STATE_SERVER_CANCEL_REQUEST_RCVD:
	case RVSIP_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE:	
        return RV_TRUE;
    default:
        return RV_FALSE;
    }
}

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * RespondUnauthenticated
 * ------------------------------------------------------------------------
 * General: The function unified the implementation of 2 APIs:
 *          RvSipTransactionRespondUnauthenticatedDigest() and
 *          RvSipTransactionRespondUnauthenticated().
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 *          responseCode -   401 or 407.
 *          strReasonPhrase - Reason phrase for this response code.
 *          headerType -     The type of the given header
 *          hHeader -        Pointer to the header to be set in the msg.
 *          realm -          mandatory.
 *          domain -         optional string. may be NULL.
 *          nonce -          optional string. may be NULL.
 *          opaque -         optional string. may be NULL.
 *          stale -          TRUE or FALSE
 *          eAlgorithm -     enumeration of algorithm. if OTHER we will take the
 *                           algorithm value from next argument.
 *          strAlgorithm -   string of algorithm. will be set only if eAlgorithm
 *                           argument is set to be RVSIP_AUTH_ALGORITHM_OTHER.
 *          eQop -           enumeration of qop. if OTHER we will take the
 *                           qop value from next argument.
 *          strQop -         string of qop. will be set only if eQop
 *                           argument is set to be RVSIP_AUTH_QOP_OTHER.
 * output:   (-)
 ***************************************************************************/
static RvStatus RVCALLCONV RespondUnauthenticated(
                                   IN  RvSipTranscHandle    hTransc,
                                   IN  RvUint16            responseCode,
                                   IN  RvChar              *strReasonPhrase,
                                   IN  RvSipHeaderType      headerType,
                                   IN  void*                hHeader,
                                   IN  RvChar              *realm,
                                   IN  RvChar              *domain,
                                   IN  RvChar              *nonce,
                                   IN  RvChar              *opaque,
                                   IN  RvBool              stale,
                                   IN  RvSipAuthAlgorithm   eAlgorithm,
                                   IN  RvChar              *strAlgorithm,
                                   IN  RvSipAuthQopOption   eQop,
                                   IN  RvChar              *strQop)
{
    RvStatus           rv = RV_OK;

    Transaction         *pTransc = (Transaction*)hTransc;

    if (NULL == hTransc)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransactionLockAPI(pTransc); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "RespondUnauthenticated - Transaction=0x%p", pTransc));

    if ((responseCode != 401) && (responseCode != 407))
    {
        /* The response code is out of limit */
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RespondUnauthenticated - Transaction 0x%p: Response is not 401/407",
                   pTransc));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_BADPARAM;
    }

    if(pTransc->eState != RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD &&
       pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD &&
       pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT &&
       pTransc->eState != RVSIP_TRANSC_STATE_SERVER_INVITE_PRACK_COMPLETED)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "RespondUnauthenticated - Transaction 0x%p: Failed - can't send response in state %s",
                  pTransc,TransactionGetStateName(pTransc->eState)));
        TransactionUnLockAPI(pTransc);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = TransactionAuthRespondUnauthenticated(pTransc, responseCode, strReasonPhrase,
                                           headerType, hHeader,
                                           realm, domain, nonce, opaque, stale,
                                           eAlgorithm, strAlgorithm, eQop, strQop);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "RespondUnauthenticated - Transaction 0x%p: Failed to send response (rv=%d)",
            pTransc, rv));
    }

    TransactionUnLockAPI(pTransc);
    return(rv);
}
#endif /* #ifdef RV_SIP_AUTH_ON */

/***************************************************************************
 * SendSpecificMethod
 * ------------------------------------------------------------------------
 * General: Sends a MESSAGE/OPTIONS/PUBLISH request over a given transaction.
 *          The function may use given parameters in a string format, or application 
 *          may supply NULL as the parameters struct, so the function will use 
 *          parameters that were set to the transaction in advance.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc       - The transaction this request refers to.
 *            pTranscParams - Structure containing the transaction parameters
 *                        (Request-Uri, To, From, Cseq val) in a string format,
 *                        to be used in the request message.
 *                        if NULL - the function will use parameters that were 
 *                        set to the transaction in advance.
 *            sizeofCfg     - The size of the given parameters structure.
  *           hRequestUri   - In case the given parameters structure is NULL,
  *                        the application need to supply the request-uri handle.
 ***************************************************************************/
static RvStatus RVCALLCONV SendSpecificMethod( IN RvSipTranscHandle              hTransc,
                                               IN RvSipTransactionStrParamsCfg*  pTranscParams,
                                               IN RvInt32                        sizeofCfg,
                                               IN RvChar*                        strMethod,
                                               IN RvSipAddressHandle             hRequestUri)
{
	RV_UNUSED_ARG(sizeofCfg)
		
    if(pTranscParams == NULL && hRequestUri == NULL)
    {
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    if(pTranscParams == NULL)
    {
        return RvSipTransactionRequest(hTransc, strMethod, hRequestUri);
    }
    else
    {
        return RvSipTransactionMake(hTransc, 
                                    pTranscParams->strFrom, 
                                    pTranscParams->strTo, 
                                    pTranscParams->strRequestURI, 
                                    pTranscParams->cseqStep, 
                                    strMethod);
    }
}

/******************************************************************************
 * TransactionBlockTermination
 * ----------------------------------------------------------------------------
 * General: The function checks if we need to block the terminate API because
 *          the transaction is in the middle of a callback that does not allow termination.
 * Return Value: RV_TRUE - should block the Terminate function.
 *               RV_FALSE - should not block it.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pTransc      - Pointer to the transaction object.
 ******************************************************************************/
static RvBool TransactionBlockTermination(IN Transaction        *pTransc)
{
    RvInt32 res = 0;
    res = (pTransc->cbBitmap & pTransc->pMgr->terminateMask); 
    if (res > 0)
    {
        /* at least one of the bits marked in the mask, are also turned on
           in the cb bitmap */
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionBlockTermination - Transaction 0x%p - can not terminate while inside callback %s",
                  pTransc, TranscGetCbName(res)));
        return RV_TRUE;
    }
    return RV_FALSE;
}

#endif /*#ifndef RV_SIP_LIGHT*/

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

int RvSipTransactionGetCct(IN  RvSipTranscHandle hTransc) {
   if(hTransc) {
      return ((Transaction *)hTransc)->cctContext;
   }
   return -1;
}

void RvSipTransactionSetCct(IN  RvSipTranscHandle hTransc,int cct) {
   if(hTransc) {
      Transaction *t = (Transaction*)hTransc;
      t->cctContext = cct;
   }
}

int RvSipTransactionGetURIFlag(IN  RvSipTranscHandle hTransc) {
   if(hTransc) {
      return ((Transaction *)hTransc)->URI_Cfg_Flag;
   } else {
     return 0xFF;
   }
}

void RvSipTransactionSetURIFlag(IN  RvSipTranscHandle hTransc,int uri_flag) {
   if(hTransc) {
      Transaction *t = (Transaction*)hTransc;
      t->URI_Cfg_Flag = uri_flag;
   }
}

RvSipTransportAddr* RvSipTransactionGetLocalAddr(IN RvSipTranscHandle hTransc) {

   if(!hTransc) {

      return NULL;

   } else {

      Transaction *t = (Transaction*)hTransc;

      return &(t->localAddr);
   }
}

const RvChar* RvSipTransactionGetLocalIpStr(IN RvSipTranscHandle hTransc) {

   if(!hTransc) {

      return NULL;

   } else {

      Transaction *t = (Transaction*)hTransc;
      
      if(t->localAddr.eAddrType < 0) return NULL;
      if(t->localAddr.eTransportType < 0) return NULL;

      return t->localAddr.strIP;
   }
}

RvInt32 RvSipTransactionGetLocalPort(IN RvSipTranscHandle hTransc) {
   
   if(!hTransc) {

      return -1;

   } else {

      Transaction *t = (Transaction*)hTransc;
      
      if(t->localAddr.eAddrType < 0) return -1;
      if(t->localAddr.eTransportType < 0) return -1;

      return t->localAddr.port;
   }
}

RvStatus RvSipTransactionSetLocalAddr(IN RvSipTranscHandle hTransc,RvSipTransportAddr  *localAddr) {

   if(!hTransc || !localAddr) {
      return RV_ERROR_INVALID_HANDLE;
   } else {
      Transaction *t = (Transaction*)hTransc;
      memcpy(&(t->localAddr),localAddr,sizeof(t->localAddr));
      return RV_OK;
   }
}

RvStatus RVCALLCONV RvSipTransactionAddViaToMsg(RvSipTranscHandle   hTransc,
                                                RvSipMsgHandle hMsg) {
  return TransactionAddViaToMsg((Transaction*)hTransc,hMsg);
}


RvStatus RVCALLCONV RvTransactionIsIncomingSigComp(IN    RvSipTranscHandle   hTransc, int* EComp)
{
   Transaction*  pTransc = (Transaction*)hTransc;
   if(pTransc)
	  *EComp = pTransc->eIncomingMsgComp; 
   else
      *EComp = -2;
   return RV_OK;
}
RvStatus RVCALLCONV RvTransactionGetIncomingSigCompUrn(IN    RvSipTranscHandle   hTransc, char* purn)
{
	Transaction*  pTransc = (Transaction*)hTransc;
    if(pTransc && pTransc->eIncomingMsgComp == RVSIP_COMP_SIGCOMP) 
	{
       if(pTransc->hSigCompCompartment)
	   {
		   RvSipCompartmentGetUrn(pTransc->hSigCompCompartment, purn);
           return RV_OK;
	   }
	   else return -2;
	}
	return -1;
}

RVAPI RvStatus RVCALLCONV RvSipTransactionMgrSetSpirentEvHandlers(
                        IN RvSipTranscMgrHandle            hTranscMgr,          
                        IN RvSipTransmitterEvHandlers      *pHandlers,
                        IN RvUint32                        evHandlerStructSize)
{
    TransactionMgr      *pTranscMgr;
    RvLogSource*   logId;
    RvInt32 minStructSize = (evHandlerStructSize < sizeof(RvSipTransmitterEvHandlers)) ?
                              evHandlerStructSize : sizeof(RvSipTransmitterEvHandlers);
    pTranscMgr = (TransactionMgr *)hTranscMgr;
    if ((NULL == hTranscMgr) ||
        (NULL == pHandlers) )
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

	pTranscMgr->transcTrxEvHandlers.pfnLastChangeBefSendEvHandler = pHandlers->pfnLastChangeBefSendEvHandler;    
    RvMutexUnlock(&pTranscMgr->hLock,pTranscMgr->pLogMgr);
    return RV_OK;
}

RvStatus RVCALLCONV RvSipTransactionGetFromAddr(RvSipTranscHandle   hTransc, 
                                                RvAddress           *addr,
                                                RvSipTransport      *eTransport) {

  RvStatus rv=RV_OK;

  if(hTransc) {
    Transaction *t = (Transaction*)hTransc;
    if(addr) memcpy(addr,&(t->receivedFrom.addr),sizeof(*addr));
    if(eTransport) *eTransport=t->receivedFrom.eTransport;
  }

  return rv;
}

#endif
//SPIRENT_END


