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
 *                              <PubClientCallbacks.c>
 *
 *    Gilad Govrin                 Sep 2006
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "PubClientCallbacks.h"


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * PubClientCallbackStateChangeEv
 * ------------------------------------------------------------------------
 * General: Change the publish-client state to the state given. Call the
 *          EvStateChanged callback with the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The new state of the publish-client.
 *        eReason - The reason for the state change.
 *        pPubClient - The publish-client in which the state has
 *                       changed.
 * Output:(-)
 ***************************************************************************/
void RVCALLCONV PubClientCallbackStateChangeEv(
                            IN  RvSipPubClientState             eNewState,
                            IN  RvSipPubClientStateChangeReason eReason,
                            IN  PubClient                       *pPubClient)
{
    RvInt32                     nestedLock = 0;
    RvSipPubClientEvHandlers    *pubClientEvHandlers = pPubClient->pubClientEvHandlers;

    /* Call the event state changed that was set to the publish-clients
       manager */
    if (pubClientEvHandlers != NULL && NULL != (*(pubClientEvHandlers)).pfnStateChangedEvHandler)
    {
        RvSipAppPubClientHandle hAppPubClient = pPubClient->hAppPubClient;
        SipTripleLock          *pTripleLock;
        RvThreadId              currThreadId; RvInt32 threadIdLockCount;

        pTripleLock = pPubClient->tripleLock;

        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                  "PubClientCallbackStateChangeEv: Publish-client 0x%p, Before callback",pPubClient));

        SipCommonUnlockBeforeCallback(pPubClient->pMgr->pLogMgr,pPubClient->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*(pubClientEvHandlers)).pfnStateChangedEvHandler(
                                      (RvSipPubClientHandle)pPubClient,
                                      hAppPubClient,
                                      eNewState,
                                      eReason);

        SipCommonLockAfterCallback(pPubClient->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                  "PubClientCallbackStateChangeEv: Publish-client 0x%p,After callback",pPubClient));

    }
    else
    {
        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                  "PubClientCallbackStateChangeEv: Publish-client 0x%p,Application did not registered to callback",pPubClient));

    }
}

/***************************************************************************
 * PubClientCallbackMsgToSendEv
 * ------------------------------------------------------------------------
 * General: Calls to pfnMsgToSendEvHandler.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient -  Handle to the pub-client object.
 *            hMsgToSend -   The message handle.
 ***************************************************************************/
RvStatus RVCALLCONV PubClientCallbackMsgToSendEv(
                                       IN RvSipPubClientHandle   hPubClient,
                                       IN RvSipMsgHandle         hMsgToSend)
{
    PubClient                *pPubClient = (PubClient *)hPubClient;
    RvStatus                 retStatus  = RV_OK;
    RvInt32                  nestedLock = 0;
    RvSipPubClientEvHandlers *pubClientEvHandlers = pPubClient->pubClientEvHandlers;

    /* Call event message to send of the application */
    if (pubClientEvHandlers != NULL && NULL != (*(pubClientEvHandlers)).pfnMsgToSendEvHandler)
    {
        RvSipAppPubClientHandle hAppPubClient = pPubClient->hAppPubClient;
        SipTripleLock          *pTripleLock;
        RvThreadId              currThreadId; RvInt32 threadIdLockCount;

        pTripleLock = pPubClient->tripleLock;

        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                  "PubClientCallbackMsgToSendEv: Publish-client 0x%p, Before callback",pPubClient));

        SipCommonUnlockBeforeCallback(pPubClient->pMgr->pLogMgr,pPubClient->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        retStatus = (*(pubClientEvHandlers)).pfnMsgToSendEvHandler(
                                            hPubClient,
                                            hAppPubClient,
                                            hMsgToSend);

        SipCommonLockAfterCallback(pPubClient->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                  "PubClientCallbackMsgToSendEv: Publish-client 0x%p,After callback",pPubClient));
    }
    else
    {
        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                  "PubClientCallbackMsgToSendEv: Publish-client 0x%p,Application did not registered to callback",pPubClient));

    }

    if(retStatus != RV_OK)
    {
        RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "PubClientCallbackMsgToSendEv: PubClient 0x%p - returned from callback with status %d",
            pPubClient, retStatus));
    }
    return retStatus;
}

/***************************************************************************
 * PubClientCallbackMsgReceivedEv
 * ------------------------------------------------------------------------
 * General: call pfnMsgReceivedEvHandler callback
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPubClient   - The pub client object.
 *          pMsgReceived - The new message the transaction has received.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV PubClientCallbackMsgReceivedEv (
                                       IN RvSipPubClientHandle  hPubClient,
                                  IN RvSipMsgHandle        hMsgReceived)
{
    RvStatus                  retStatus  = RV_OK;
    RvInt32                   nestedLock = 0;
    PubClient                *pPubClient = (PubClient *)hPubClient;
    RvSipAppPubClientHandle   hAppPubClient = pPubClient->hAppPubClient;
    SipTripleLock            *pTripleLock;
    RvThreadId                currThreadId; RvInt32 threadIdLockCount;
    RvSipPubClientEvHandlers *pubClientEvHandlers = pPubClient->pubClientEvHandlers;

    pTripleLock = pPubClient->tripleLock;

    /* Call event message received of the application */
    if (pubClientEvHandlers != NULL && NULL != (*(pubClientEvHandlers)).pfnMsgReceivedEvHandler)
    {
        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                          "PubClientCallbackMsgReceivedEv: Publish-client 0x%p, Before callback",pPubClient));

        SipCommonUnlockBeforeCallback(pPubClient->pMgr->pLogMgr,pPubClient->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        retStatus = (*(pubClientEvHandlers)).pfnMsgReceivedEvHandler(
                                        (RvSipPubClientHandle)pPubClient,
                                        hAppPubClient,
                                        hMsgReceived);

        SipCommonLockAfterCallback(pPubClient->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                  "PubClientCallbackMsgReceivedEv: Publish-client 0x%p,After callback",pPubClient));
    }
    else
    {
        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                  "PubClientCallbackMsgReceivedEv: Publish-client 0x%p,Application did not registered to callback",pPubClient));

    }

    if(retStatus != RV_OK)
    {
        RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "PubClientCallbackMsgReceivedEv: PubClient 0x%p - returned from callback with status %d",
            pPubClient, retStatus));
    }

    return retStatus;
}


/***************************************************************************
 * PubClientTranscEvOtherURLAddressFound
 * ------------------------------------------------------------------------
 * General: Notifies the application that other URL address (URL that is
 *            currently not supported by the RvSip stack) was found and has
 *            to be converted to known SIP URL address.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hPubClient     - The PubClient handle for this transaction.
 *            hMsg           - The message that includes the other URL address.
 *            hAddress       - Handle to unsupport address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV PubClientCallbackOtherURLAddressFoundEv(
                     IN  RvSipTranscHandle			hTransc,
                     IN  SipTranscClientOwnerHandle hPubClient,
                     IN  RvSipMsgHandle				hMsg,
                     IN  RvSipAddressHandle			hAddress,
                     OUT RvSipAddressHandle			hSipURLAddress,
                     OUT RvBool						*bAddressResolved)
{
    PubClient                *pPubClient = (PubClient*)hPubClient;
    RvInt32                   nestedLock = 0;
    SipTripleLock            *pTripleLock;
    RvThreadId                currThreadId; RvInt32 threadIdLockCount;
    RvSipAppPubClientHandle   hAppPubClient = pPubClient->hAppPubClient;
    RvSipPubClientEvHandlers *evHandlers  = pPubClient->pubClientEvHandlers;

    RV_UNUSED_ARG(hTransc);
    pTripleLock = pPubClient->tripleLock;
      if(evHandlers != NULL && (*evHandlers).pfnOtherURLAddressFoundEvHandler != NULL)
    {
        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                     "PubClientCallbackOtherURLAddressFoundEv: Publish-client 0x%p, Before callback",pPubClient));
        SipCommonUnlockBeforeCallback(pPubClient->pMgr->pLogMgr,pPubClient->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*evHandlers).pfnOtherURLAddressFoundEvHandler(
                                           (RvSipPubClientHandle)pPubClient,
                                           hAppPubClient,
                                           hMsg,
                                           hAddress,
                                           hSipURLAddress,
                                           bAddressResolved);

        SipCommonLockAfterCallback(pPubClient->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                  "PubClientCallbackOtherURLAddressFoundEv: Publish-client 0x%p,After callback",pPubClient));

    }
    else
    {
        *bAddressResolved = RV_FALSE;
        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                  "PubClientCallbackOtherURLAddressFoundEv: Publish-client 0x%p,Application did not registered to callback",pPubClient));

    }

    return RV_OK;
}



/***************************************************************************
 * PubClientCallbackFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnFinalDestResolvedEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hPubClient     - The PubClient handle for this transaction.
 *            hMsg           - The message to send
 ***************************************************************************/
RvStatus RVCALLCONV PubClientCallbackFinalDestResolvedEv(
                     IN  RvSipTranscHandle      hTransc,
                     IN  RvSipTranscOwnerHandle hPubClient,
                     IN  RvSipMsgHandle         hMsg)
{
    PubClient                *pPubClient = (PubClient*)hPubClient;
    RvInt32                   nestedLock = 0;
    SipTripleLock            *pTripleLock;
    RvThreadId                currThreadId; RvInt32 threadIdLockCount;
    RvSipAppPubClientHandle   hAppPubClient = pPubClient->hAppPubClient;
    RvSipPubClientEvHandlers *evHandlers    = pPubClient->pubClientEvHandlers;
    RvStatus                  rv = RV_OK;

    pTripleLock = pPubClient->tripleLock;
      if(evHandlers != NULL && (*evHandlers).pfnFinalDestResolvedEvHandler != NULL)
    {
        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                     "PubClientCallbackFinalDestResolvedEv: hPubClient=0x%p, hAppPubClient=0x%p, hTransc=0x%p, hMsg=0x%p Before callback",
                     pPubClient,hAppPubClient,hTransc,hMsg));
        SipCommonUnlockBeforeCallback(pPubClient->pMgr->pLogMgr,pPubClient->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*evHandlers).pfnFinalDestResolvedEvHandler(
                                           (RvSipPubClientHandle)pPubClient,
                                           hAppPubClient,
                                           hTransc,
                                           hMsg);

        SipCommonLockAfterCallback(pPubClient->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                  "PubClientCallbackFinalDestResolvedEv: Publish-client 0x%p,After callback, (rv=%d)",pPubClient,rv));

    }
    else
    {
        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                  "PubClientCallbackFinalDestResolvedEv: hPubClient=0x%p, hAppPubClient=0x%p, hTransc=0x%p, hMsg=0x%p,Application did not registered to callback",
                  pPubClient,hAppPubClient,hTransc,hMsg));

    }

    return rv;
}

/***************************************************************************
 * PubClientCallbackNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: calls to pfnNewConnInUseEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClient     - The PubClient pointer
 *            hConn          - The new connection in use.
 *            bNewConnCreated - The connection is also a newly created connection
 *                               (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV PubClientCallbackNewConnInUseEv(
                     IN  PubClient*                     pPubClient,
                     IN  RvSipTransportConnectionHandle hConn,
                     IN  RvBool                         bNewConnCreated)
{
    RvInt32                   nestedLock    = 0;
    SipTripleLock            *pTripleLock;
    RvThreadId                currThreadId; RvInt32 threadIdLockCount;
    RvSipAppPubClientHandle   hAppPubClient = pPubClient->hAppPubClient;
    RvSipPubClientEvHandlers *evHandlers    = pPubClient->pubClientEvHandlers;
    RvStatus                  rv            = RV_OK;

    pTripleLock = pPubClient->tripleLock;
    if(evHandlers != NULL && (*evHandlers).pfnNewConnInUseEvHandler != NULL)
    {
        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                     "PubClientCallbackNewConnInUseEv: hPubClient=0x%p, hAppPubClient=0x%p, hConn=0x%p Before callback",
                     pPubClient,hAppPubClient,hConn));
        SipCommonUnlockBeforeCallback(pPubClient->pMgr->pLogMgr,pPubClient->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*evHandlers).pfnNewConnInUseEvHandler(
                                           (RvSipPubClientHandle)pPubClient,
                                           hAppPubClient,
                                           hConn,
                                           bNewConnCreated);

        SipCommonLockAfterCallback(pPubClient->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                  "PubClientCallbackNewConnInUseEv: Publish-client 0x%p,After callback, (rv=%d)",pPubClient,rv));

    }
    else
    {
        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                  "PubClientCallbackNewConnInUseEv: hPubClient=0x%p, hAppPubClient=0x%p, hConn=0x%p,Application did not registered to callback",
                  pPubClient,hAppPubClient,hConn));

    }

    return rv;
}

/***************************************************************************
 * PubClientCallbackObjExpired
 * ------------------------------------------------------------------------
 * General: calls to pfnObjExpireEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClient     - The PubClient pointer
 ***************************************************************************/
void RVCALLCONV PubClientCallbackObjExpired(
                     IN  PubClient*                     pPubClient)
{
    RvInt32                   nestedLock    = 0;
    SipTripleLock            *pTripleLock;
    RvThreadId                currThreadId; RvInt32 threadIdLockCount;
    RvSipAppPubClientHandle   hAppPubClient = pPubClient->hAppPubClient;
    RvSipPubClientEvHandlers *evHandlers    = pPubClient->pubClientEvHandlers;
    
    pTripleLock = pPubClient->tripleLock;
    if(evHandlers != NULL && (*evHandlers).pfnObjExpiredEvHandler != NULL)
    {
        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                     "PubClientCallbackObjExpired: hPubClient=0x%p, hAppPubClient=0x%p, Before callback",
                     pPubClient,hAppPubClient));
        SipCommonUnlockBeforeCallback(pPubClient->pMgr->pLogMgr,pPubClient->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*evHandlers).pfnObjExpiredEvHandler(
                                           (RvSipPubClientHandle)pPubClient,
                                           hAppPubClient);

        SipCommonLockAfterCallback(pPubClient->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                  "PubClientCallbackObjExpired: Publish-client 0x%p,After callback",pPubClient));

    }
    else
    {
        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
                  "PubClientCallbackObjExpired: hPubClient=0x%p, hAppPubClient=0x%p,Application did not registered to callback",
                  pPubClient,hAppPubClient));

    }

}


#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * PubClientCallbackSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnSigCompMsgNotRespondedEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hPubClient     - The PubClient handle for this transaction.
 *          retransNo     - The number of retransmissions of the request
 *                          until now.
 *          ePrevMsgComp  - The type of the previous not responded request
 * Output:  peNextMsgComp - The type of the next request to retransmit (
 *                          RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                          application wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV PubClientCallbackSigCompMsgNotRespondedEv(
                     IN  RvSipTranscHandle            hTransc,
                     IN  RvSipTranscOwnerHandle       hPubClient,
                     IN  RvInt32                      retransNo,
                     IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                     OUT RvSipTransmitterMsgCompType *peNextMsgComp)
{
    SipTripleLock            *pTripleLock;
    RvThreadId                currThreadId; RvInt32 threadIdLockCount;
    PubClient                *pPubClient    = (PubClient*)hPubClient;
    RvInt32                   nestedLock    = 0;
    RvSipAppPubClientHandle   hAppPubClient = pPubClient->hAppPubClient;
    RvSipPubClientEvHandlers *evHandlers    = pPubClient->pubClientEvHandlers;
    RvStatus                  rv            = RV_OK;

    pTripleLock = pPubClient->tripleLock;

      if(evHandlers != NULL && (*evHandlers).pfnSigCompMsgNotRespondedEvHandler != NULL)
    {
        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "PubClientCallbackSigCompMsgNotRespondedEv: hPubClient=0x%p, hTransc=0x%p, retrans=%d, prev=%s Before callback",
            pPubClient,hTransc,retransNo,SipTransactionGetMsgCompTypeName(ePrevMsgComp)));
        SipCommonUnlockBeforeCallback(pPubClient->pMgr->pLogMgr,pPubClient->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*evHandlers).pfnSigCompMsgNotRespondedEvHandler(
                                           (RvSipPubClientHandle)pPubClient,
                                           hAppPubClient,
                                           hTransc,
                                           retransNo,
                                           ePrevMsgComp,
                                           peNextMsgComp);

        SipCommonLockAfterCallback(pPubClient->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
          "PubClientCallbackSigCompMsgNotRespondedEv: hPubClient=0x%p,After callback, (rv=%d)",pPubClient,rv));

    }
    else
    {
        RvLogDebug(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
            "PubClientCallbackSigCompMsgNotRespondedEv: hPubClient=0x%p, hTransc=0x%p, retrans=%d, prev=%s,Application did not registered to callback",
            pPubClient,hTransc,retransNo,SipTransactionGetMsgCompTypeName(ePrevMsgComp)));
    }

    return rv;
}
#endif /* RV_SIGCOMP_ON */

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

#endif /* RV_SIP_PRIMITIVES */
