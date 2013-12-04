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
 *                              <RegClientCallbacks.c>
 *
 *
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RegClientCallbacks.h"


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * RegClientCallbackStateChangeEv
 * ------------------------------------------------------------------------
 * General: Change the register-client state to the state given. Call the
 *          EvStateChanged callback with the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The new state of the register-client.
 *        eReason - The reason for the state change.
 *        pRegClient - The register-client in which the state has
 *                       changed.
 * Output:(-)
 ***************************************************************************/
void RVCALLCONV RegClientCallbackStateChangeEv(
                            IN  RvSipRegClientState             eNewState,
                            IN  RvSipRegClientStateChangeReason eReason,
                            IN  RegClient                       *pRegClient)
{
    RvInt32                     nestedLock = 0;
    RvSipRegClientEvHandlers    *regClientEvHandlers = pRegClient->regClientEvHandlers;

    /* Call the event state changed that was set to the register-clients
       manager */
    if (regClientEvHandlers != NULL && NULL != (*(regClientEvHandlers)).pfnStateChangedEvHandler)
    {
        RvSipAppRegClientHandle hAppRegClient = pRegClient->hAppRegClient;
        SipTripleLock          *pTripleLock;
        RvThreadId              currThreadId; RvInt32 threadIdLockCount;

        pTripleLock = pRegClient->tripleLock;

        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCallbackStateChangeEv: Register-client 0x%p, Before callback",pRegClient));

        SipCommonUnlockBeforeCallback(pRegClient->pMgr->pLogMgr,pRegClient->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*(regClientEvHandlers)).pfnStateChangedEvHandler(
                                      (RvSipRegClientHandle)pRegClient,
                                      hAppRegClient,
                                      eNewState,
                                      eReason);

        SipCommonLockAfterCallback(pRegClient->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCallbackStateChangeEv: Register-client 0x%p,After callback",pRegClient));

    }
    else
    {
        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCallbackStateChangeEv: Register-client 0x%p,Application did not registered to callback",pRegClient));

    }
}

/***************************************************************************
 * RegClientCallbackMsgToSendEv
 * ------------------------------------------------------------------------
 * General: Calls to pfnMsgToSendEvHandler.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient -  Handle to the reg-client object.
 *            hMsgToSend -   The message handle.
 ***************************************************************************/
RvStatus RVCALLCONV RegClientCallbackMsgToSendEv(
                                       IN RvSipRegClientHandle   hRegClient,
                                       IN RvSipMsgHandle         hMsgToSend)
{
    RegClient                *pRegClient = (RegClient *)hRegClient;
    RvStatus                 retStatus  = RV_OK;
    RvInt32                  nestedLock = 0;
    RvSipRegClientEvHandlers *regClientEvHandlers = pRegClient->regClientEvHandlers;

    /* Call event message to send of the application */
    if (regClientEvHandlers != NULL && NULL != (*(regClientEvHandlers)).pfnMsgToSendEvHandler)
    {
        RvSipAppRegClientHandle hAppRegClient = pRegClient->hAppRegClient;
        SipTripleLock          *pTripleLock;
        RvThreadId              currThreadId; RvInt32 threadIdLockCount;

        pTripleLock = pRegClient->tripleLock;

        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCallbackMsgToSendEv: Register-client 0x%p, Before callback",pRegClient));

        SipCommonUnlockBeforeCallback(pRegClient->pMgr->pLogMgr,pRegClient->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        retStatus = (*(regClientEvHandlers)).pfnMsgToSendEvHandler(
                                            hRegClient,
                                            hAppRegClient,
                                            hMsgToSend);

        SipCommonLockAfterCallback(pRegClient->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCallbackMsgToSendEv: Register-client 0x%p,After callback",pRegClient));
    }
    else
    {
        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCallbackMsgToSendEv: Register-client 0x%p,Application did not registered to callback",pRegClient));

    }

    if(retStatus != RV_OK)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RegClientCallbackMsgToSendEv: RegClient 0x%p - returned from callback with status %d",
            pRegClient, retStatus));
    }
    return retStatus;
}

/***************************************************************************
 * RegClientCallbackMsgReceivedEv
 * ------------------------------------------------------------------------
 * General: call pfnMsgReceivedEvHandler callback
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hRegClient   - The reg client object.
 *          pMsgReceived - The new message the transaction has received.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV RegClientCallbackMsgReceivedEv (
                                       IN RvSipRegClientHandle  hRegClient,
                                  IN RvSipMsgHandle        hMsgReceived)
{
    RvStatus                  retStatus  = RV_OK;
    RvInt32                   nestedLock = 0;
    RegClient                *pRegClient = (RegClient *)hRegClient;
    RvSipAppRegClientHandle   hAppRegClient = pRegClient->hAppRegClient;
    SipTripleLock            *pTripleLock;
    RvThreadId                currThreadId; RvInt32 threadIdLockCount;
    RvSipRegClientEvHandlers *regClientEvHandlers = pRegClient->regClientEvHandlers;

    pTripleLock = pRegClient->tripleLock;

    /* Call event message received of the application */
    if (regClientEvHandlers != NULL && NULL != (*(regClientEvHandlers)).pfnMsgReceivedEvHandler)
    {
        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                          "RegClientCallbackMsgReceivedEv: Register-client 0x%p, Before callback",pRegClient));

        SipCommonUnlockBeforeCallback(pRegClient->pMgr->pLogMgr,pRegClient->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        retStatus = (*(regClientEvHandlers)).pfnMsgReceivedEvHandler(
                                        (RvSipRegClientHandle)pRegClient,
                                        hAppRegClient,
                                        hMsgReceived);

        SipCommonLockAfterCallback(pRegClient->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCallbackMsgReceivedEv: Register-client 0x%p,After callback",pRegClient));
    }
    else
    {
        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCallbackMsgReceivedEv: Register-client 0x%p,Application did not registered to callback",pRegClient));

    }

    if(retStatus != RV_OK)
    {
        RvLogError(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RegClientCallbackMsgReceivedEv: RegClient 0x%p - returned from callback with status %d",
            pRegClient, retStatus));
    }

    return retStatus;
}


/***************************************************************************
 * RegClientTranscEvOtherURLAddressFound
 * ------------------------------------------------------------------------
 * General: Notifies the application that other URL address (URL that is
 *            currently not supported by the RvSip stack) was found and has
 *            to be converted to known SIP URL address.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hRegClient     - The RegClient handle for this transaction.
 *            hMsg           - The message that includes the other URL address.
 *            hAddress       - Handle to unsupport address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV RegClientCallbackOtherURLAddressFoundEv(
                     IN  RvSipTranscHandle      hTransc,
                     IN  SipTranscClientOwnerHandle hRegClient,
                     IN  RvSipMsgHandle         hMsg,
                     IN  RvSipAddressHandle        hAddress,
                     OUT RvSipAddressHandle        hSipURLAddress,
                     OUT RvBool               *bAddressResolved)
{
    RegClient                *pRegClient = (RegClient*)hRegClient;
    RvInt32                   nestedLock = 0;
    SipTripleLock            *pTripleLock;
    RvThreadId                currThreadId; RvInt32 threadIdLockCount;
    RvSipAppRegClientHandle   hAppRegClient = pRegClient->hAppRegClient;
    RvSipRegClientEvHandlers *evHandlers  = pRegClient->regClientEvHandlers;
    RvStatus rv;

    RV_UNUSED_ARG(hTransc);
    pTripleLock = pRegClient->tripleLock;
      if(evHandlers != NULL && (*evHandlers).pfnOtherURLAddressFoundEvHandler != NULL)
    {
        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                     "RegClientCallbackOtherURLAddressFoundEv: Register-client 0x%p, Before callback",pRegClient));
        SipCommonUnlockBeforeCallback(pRegClient->pMgr->pLogMgr,pRegClient->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*evHandlers).pfnOtherURLAddressFoundEvHandler(
                                           (RvSipRegClientHandle)pRegClient,
                                           hAppRegClient,
                                           hMsg,
                                           hAddress,
                                           hSipURLAddress,
                                           bAddressResolved);

        SipCommonLockAfterCallback(pRegClient->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCallbackOtherURLAddressFoundEv: Register-client 0x%p,After callback(rv=%d)",
                  pRegClient, rv));

    }
    else
    {
        *bAddressResolved = RV_FALSE;
        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCallbackOtherURLAddressFoundEv: Register-client 0x%p,Application did not registered to callback",pRegClient));

    }

    return RV_OK;
}



/***************************************************************************
 * RegClientCallbackFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnFinalDestResolvedEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hRegClient     - The RegClient handle for this transaction.
 *            hMsg           - The message to send
 ***************************************************************************/
RvStatus RVCALLCONV RegClientCallbackFinalDestResolvedEv(
                     IN  RvSipTranscHandle			hTransc,
                     IN  SipTranscClientOwnerHandle hRegClient,
                     IN  RvSipMsgHandle				hMsg)
{
    RegClient                *pRegClient = (RegClient*)hRegClient;
    RvInt32                   nestedLock = 0;
    SipTripleLock            *pTripleLock;
    RvThreadId                currThreadId; RvInt32 threadIdLockCount;
    RvSipAppRegClientHandle   hAppRegClient = pRegClient->hAppRegClient;
    RvSipRegClientEvHandlers *evHandlers    = pRegClient->regClientEvHandlers;
    RvStatus                  rv = RV_OK;

    pTripleLock = pRegClient->tripleLock;
      if(evHandlers != NULL && (*evHandlers).pfnFinalDestResolvedEvHandler != NULL)
    {
        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                     "RegClientCallbackFinalDestResolvedEv: hRegClient=0x%p, hAppRegClient=0x%p, hTransc=0x%p, hMsg=0x%p Before callback",
                     pRegClient,hAppRegClient,hTransc,hMsg));
        SipCommonUnlockBeforeCallback(pRegClient->pMgr->pLogMgr,pRegClient->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*evHandlers).pfnFinalDestResolvedEvHandler(
                                           (RvSipRegClientHandle)pRegClient,
                                           hAppRegClient,
                                           hTransc,
                                           hMsg);

        SipCommonLockAfterCallback(pRegClient->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCallbackFinalDestResolvedEv: Register-client 0x%p,After callback, (rv=%d)",pRegClient,rv));

    }
    else
    {
        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCallbackFinalDestResolvedEv: hRegClient=0x%p, hAppRegClient=0x%p, hTransc=0x%p, hMsg=0x%p,Application did not registered to callback",
                  pRegClient,hAppRegClient,hTransc,hMsg));

    }

    return rv;
}

/***************************************************************************
 * RegClientCallbackNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: calls to pfnNewConnInUseEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient     - The RegClient pointer
 *            hConn          - The new connection in use.
 *            bNewConnCreated - The connection is also a newly created connection
 *                               (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV RegClientCallbackNewConnInUseEv(
                     IN  RegClient*                     pRegClient,
                     IN  RvSipTransportConnectionHandle hConn,
                     IN  RvBool                         bNewConnCreated)
{
    RvInt32                   nestedLock    = 0;
    SipTripleLock            *pTripleLock;
    RvThreadId                currThreadId; RvInt32 threadIdLockCount;
    RvSipAppRegClientHandle   hAppRegClient = pRegClient->hAppRegClient;
    RvSipRegClientEvHandlers *evHandlers    = pRegClient->regClientEvHandlers;
    RvStatus                  rv            = RV_OK;

    pTripleLock = pRegClient->tripleLock;
    if(evHandlers != NULL && (*evHandlers).pfnNewConnInUseEvHandler != NULL)
    {
        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                     "RegClientCallbackNewConnInUseEv: hRegClient=0x%p, hAppRegClient=0x%p, hConn=0x%p Before callback",
                     pRegClient,hAppRegClient,hConn));
        SipCommonUnlockBeforeCallback(pRegClient->pMgr->pLogMgr,pRegClient->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*evHandlers).pfnNewConnInUseEvHandler(
                                           (RvSipRegClientHandle)pRegClient,
                                           hAppRegClient,
                                           hConn,
                                           bNewConnCreated);

        SipCommonLockAfterCallback(pRegClient->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCallbackNewConnInUseEv: Register-client 0x%p,After callback, (rv=%d)",pRegClient,rv));

    }
    else
    {
        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCallbackNewConnInUseEv: hRegClient=0x%p, hAppRegClient=0x%p, hConn=0x%p,Application did not registered to callback",
                  pRegClient,hAppRegClient,hConn));

    }

    return rv;
}

/***************************************************************************
 * RegClientCallbackObjExpired
 * ------------------------------------------------------------------------
 * General: calls to pfnObjExpireEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient     - The RegClient pointer
 ***************************************************************************/
void RVCALLCONV RegClientCallbackObjExpired(
                     IN  RegClient*                     pRegClient)
{
    RvInt32                   nestedLock    = 0;
    SipTripleLock            *pTripleLock;
    RvThreadId                currThreadId; RvInt32 threadIdLockCount;
    RvSipAppRegClientHandle   hAppRegClient = pRegClient->hAppRegClient;
    RvSipRegClientEvHandlers *evHandlers    = pRegClient->regClientEvHandlers;
    
    pTripleLock = pRegClient->tripleLock;
    if(evHandlers != NULL && (*evHandlers).pfnObjExpiredEvHandler != NULL)
    {
        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                     "RegClientCallbackObjExpired: hRegClient=0x%p, hAppRegClient=0x%p, Before callback",
                     pRegClient,hAppRegClient));
        SipCommonUnlockBeforeCallback(pRegClient->pMgr->pLogMgr,pRegClient->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*evHandlers).pfnObjExpiredEvHandler(
                                           (RvSipRegClientHandle)pRegClient,
                                           hAppRegClient);

        SipCommonLockAfterCallback(pRegClient->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCallbackObjExpired: Register-client 0x%p,After callback",pRegClient));

    }
    else
    {
        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
                  "RegClientCallbackObjExpired: hRegClient=0x%p, hAppRegClient=0x%p,Application did not registered to callback",
                  pRegClient,hAppRegClient));

    }

}



#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * RegClientCallbackSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnSigCompMsgNotRespondedEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hRegClient     - The RegClient handle for this transaction.
 *          retransNo     - The number of retransmissions of the request
 *                          until now.
 *          ePrevMsgComp  - The type of the previous not responded request
 * Output:  peNextMsgComp - The type of the next request to retransmit (
 *                          RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                          application wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV RegClientCallbackSigCompMsgNotRespondedEv(
                     IN  RvSipTranscHandle            hTransc,
                     IN  RvSipTranscOwnerHandle       hRegClient,
                     IN  RvInt32                      retransNo,
                     IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                     OUT RvSipTransmitterMsgCompType *peNextMsgComp)
{
    SipTripleLock            *pTripleLock;
    RvThreadId                currThreadId; RvInt32 threadIdLockCount;
    RegClient                *pRegClient    = (RegClient*)hRegClient;
    RvInt32                   nestedLock    = 0;
    RvSipAppRegClientHandle   hAppRegClient = pRegClient->hAppRegClient;
    RvSipRegClientEvHandlers *evHandlers    = pRegClient->regClientEvHandlers;
    RvStatus                  rv            = RV_OK;

    pTripleLock = pRegClient->tripleLock;

      if(evHandlers != NULL && (*evHandlers).pfnSigCompMsgNotRespondedEvHandler != NULL)
    {
        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RegClientCallbackSigCompMsgNotRespondedEv: hRegClient=0x%p, hTransc=0x%p, retrans=%d, prev=%s Before callback",
            pRegClient,hTransc,retransNo,SipTransactionGetMsgCompTypeName(ePrevMsgComp)));
        SipCommonUnlockBeforeCallback(pRegClient->pMgr->pLogMgr,pRegClient->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*evHandlers).pfnSigCompMsgNotRespondedEvHandler(
                                           (RvSipRegClientHandle)pRegClient,
                                           hAppRegClient,
                                           hTransc,
                                           retransNo,
                                           ePrevMsgComp,
                                           peNextMsgComp);

        SipCommonLockAfterCallback(pRegClient->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
          "RegClientCallbackSigCompMsgNotRespondedEv: hRegClient=0x%p,After callback, (rv=%d)",pRegClient,rv));

    }
    else
    {
        RvLogDebug(pRegClient->pMgr->pLogSrc,(pRegClient->pMgr->pLogSrc,
            "RegClientCallbackSigCompMsgNotRespondedEv: hRegClient=0x%p, hTransc=0x%p, retrans=%d, prev=%s,Application did not registered to callback",
            pRegClient,hTransc,retransNo,SipTransactionGetMsgCompTypeName(ePrevMsgComp)));
    }

    return rv;
}
#endif /* RV_SIGCOMP_ON */

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

#endif /* RV_SIP_PRIMITIVES */
