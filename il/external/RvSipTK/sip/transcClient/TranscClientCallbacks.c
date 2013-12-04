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
 *                              <TranscClientCallbacks.c>
 *
 *
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "TranscClientCallbacks.h"


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * TranscClientCallbackStateChangeEv
 * ------------------------------------------------------------------------
 * General: Change the transc-client state to the state given. Call the
 *          EvStateChanged callback with the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The new state of the transc-client.
 *        eReason - The reason for the state change.
 *        pTranscClient - The Transc-client in which the state has
 *                       changed.
 * Output:(-)
 ***************************************************************************/
void RVCALLCONV TranscClientCallbackStateChangeEv(
                            IN  SipTranscClientState             eNewState,
                            IN  SipTranscClientStateChangeReason eReason,
                            IN  SipTranscClient                  *pTranscClient,
							IN  RvInt16							 responseCode)
{
    SipTranscClientEvHandlers    *transcClientEvHandlers = &pTranscClient->transcClientEvHandlers;

    /* Call the event state changed that was set to the register-clients
       manager */
    if (transcClientEvHandlers != NULL && NULL != (*(transcClientEvHandlers)).pfnStateChangedEvHandler)
    {
        SipTranscClientOwnerHandle hTranscClientOwner = pTranscClient->hOwner;
        SipTripleLock          *pTripleLock;

        pTripleLock = pTranscClient->tripleLock;

        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCallbackStateChangeEv: Transc-client 0x%p, Before callback",pTranscClient));

        (*(transcClientEvHandlers)).pfnStateChangedEvHandler(
                                      hTranscClientOwner,
                                      eNewState,
                                      eReason,
									  responseCode);

        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCallbackStateChangeEv: Transc-client 0x%p,After callback",pTranscClient));

    }
    else
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCallbackStateChangeEv: Transc-client 0x%p,Client did not registered to callback",pTranscClient));

    }
}

/***************************************************************************
 * TranscClientCallbackMsgToSendEv
 * ------------------------------------------------------------------------
 * General: Calls to pfnMsgToSendEvHandler.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient -  Handle to the transc-client object.
 *            hMsgToSend -   The message handle.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientCallbackMsgToSendEv(
                                       IN SipTranscClientHandle   hTranscClient,
                                       IN RvSipMsgHandle         hMsgToSend)
{
    SipTranscClient                *pTranscClient = (SipTranscClient *)hTranscClient;
    RvStatus                 retStatus  = RV_OK;
    SipTranscClientEvHandlers *transcClientEvHandlers = &pTranscClient->transcClientEvHandlers;

    /* Call event message to send of the application */
    if (transcClientEvHandlers != NULL && NULL != (*(transcClientEvHandlers)).pfnMsgToSendEvHandler)
    {
        SipTranscClientOwnerHandle hTranscClientOwner = pTranscClient->hOwner;
        SipTripleLock          *pTripleLock;

        pTripleLock = pTranscClient->tripleLock;

        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCallbackMsgToSendEv: Transc-client 0x%p, Before callback",pTranscClient));

        retStatus = (*(transcClientEvHandlers)).pfnMsgToSendEvHandler(
                                            hTranscClientOwner,
                                            hMsgToSend);

        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCallbackMsgToSendEv: Transc-client 0x%p,After callback",pTranscClient));
    }
    else
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCallbackMsgToSendEv: Transc-client 0x%p,Client did not registered to callback",pTranscClient));

    }

    if(retStatus != RV_OK)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "TranscClientCallbackMsgToSendEv: TranscClient 0x%p - returned from callback with status %d",
            pTranscClient, retStatus));
    }
    return retStatus;
}

/***************************************************************************
 * TranscClientCallbackMsgReceivedEv
 * ------------------------------------------------------------------------
 * General: call pfnMsgReceivedEvHandler callback
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hTranscClient   - The transc client object.
 *          pMsgReceived - The new message the transaction has received.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientCallbackMsgReceivedEv (
                                       IN SipTranscClientHandle  hTranscClient,
                                  IN RvSipMsgHandle        hMsgReceived)
{
    RvStatus                  retStatus  = RV_OK;
    SipTranscClient           *pTranscClient = (SipTranscClient *)hTranscClient;
    SipTranscClientOwnerHandle   hTranscClientOwner = pTranscClient->hOwner;
    SipTripleLock            *pTripleLock;
    SipTranscClientEvHandlers *transcClientEvHandlers = &pTranscClient->transcClientEvHandlers;

    pTripleLock = pTranscClient->tripleLock;

    /* Call event message received of the client */
    if (transcClientEvHandlers != NULL && NULL != (*(transcClientEvHandlers)).pfnMsgReceivedEvHandler)
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                          "TranscClientCallbackMsgReceivedEv: Transc-client 0x%p, Before callback",pTranscClient));

        retStatus = (*(transcClientEvHandlers)).pfnMsgReceivedEvHandler(
                                        hTranscClientOwner,
                                        hMsgReceived);

        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCallbackMsgReceivedEv: Transc-client 0x%p,After callback",pTranscClient));
    }
    else
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCallbackMsgReceivedEv: Transc-client 0x%p,Client did not registered to callback",pTranscClient));

    }

    if(retStatus != RV_OK)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "TranscClientCallbackMsgReceivedEv: TranscClient 0x%p - returned from callback with status %d",
            pTranscClient, retStatus));
    }

    return retStatus;
}


/***************************************************************************
 * TranscClientTranscEvOtherURLAddressFound
 * ------------------------------------------------------------------------
 * General: Notifies the client that other URL address (URL that is
 *            currently not supported by the RvSip stack) was found and has
 *            to be converted to known SIP URL address.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hTranscClient     - The TranscClient handle for this transaction.
 *            hMsg           - The message that includes the other URL address.
 *            hAddress       - Handle to unsupport address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientCallbackOtherURLAddressFoundEv(
                     IN  RvSipTranscHandle      hTransc,
                     IN  RvSipTranscOwnerHandle hTranscClient,
                     IN  RvSipMsgHandle         hMsg,
                     IN  RvSipAddressHandle        hAddress,
                     OUT RvSipAddressHandle        hSipURLAddress,
                     OUT RvBool               *bAddressResolved)
{
    SipTranscClient                *pTranscClient = (SipTranscClient*)hTranscClient;
    SipTripleLock            *pTripleLock;
    SipTranscClientOwnerHandle   hTranscClientOwner = pTranscClient->hOwner;
    SipTranscClientEvHandlers *evHandlers  = &pTranscClient->transcClientEvHandlers;

    RV_UNUSED_ARG(hTransc);
    pTripleLock = pTranscClient->tripleLock;
      if(evHandlers != NULL && (*evHandlers).pfnOtherURLAddressFoundEvHandler != NULL)
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                     "TranscClientCallbackOtherURLAddressFoundEv: Transc-client 0x%p, Before callback",pTranscClient));

        (*evHandlers).pfnOtherURLAddressFoundEvHandler(
										   hTransc,
                                           hTranscClientOwner,
                                           hMsg,
                                           hAddress,
                                           hSipURLAddress,
                                           bAddressResolved);

        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCallbackOtherURLAddressFoundEv: Transc-client 0x%p,After callback",pTranscClient));

    }
    else
    {
        *bAddressResolved = RV_FALSE;
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCallbackOtherURLAddressFoundEv: Transc-client 0x%p,Client did not registered to callback",pTranscClient));

    }

    return RV_OK;
}



/***************************************************************************
 * TranscClientCallbackFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnFinalDestResolvedEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hTranscClient     - The TranscClient handle for this transaction.
 *            hMsg           - The message to send
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientCallbackFinalDestResolvedEv(
                     IN  RvSipTranscHandle      hTransc,
                     IN  RvSipTranscOwnerHandle hTranscClient,
                     IN  RvSipMsgHandle         hMsg)
{
    SipTranscClient                *pTranscClient = (SipTranscClient*)hTranscClient;
    SipTripleLock            *pTripleLock;
    SipTranscClientOwnerHandle   hTranscClientOwner = pTranscClient->hOwner;
    SipTranscClientEvHandlers *evHandlers    = &pTranscClient->transcClientEvHandlers;
    RvStatus                  rv = RV_OK;

    pTripleLock = pTranscClient->tripleLock;
      if(evHandlers != NULL && (*evHandlers).pfnFinalDestResolvedEvHandler != NULL)
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                     "TranscClientCallbackFinalDestResolvedEv: hTranscClient=0x%p, hTranscClientOwner=0x%p, hTransc=0x%p, hMsg=0x%p Before callback",
                     pTranscClient,hTranscClientOwner,hTransc,hMsg));

        rv = (*evHandlers).pfnFinalDestResolvedEvHandler(
                                            hTransc,
											hTranscClientOwner,
											hMsg);

        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCallbackFinalDestResolvedEv: Transc-client 0x%p,After callback, (rv=%d)",pTranscClient,rv));

    }
    else
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCallbackFinalDestResolvedEv: hTranscClient=0x%p, hTranscClientOwner=0x%p, hTransc=0x%p, hMsg=0x%p,Client did not registered to callback",
                  pTranscClient,hTranscClientOwner,hTransc,hMsg));

    }

    return rv;
}

/***************************************************************************
 * TranscClientCallbackNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: calls to pfnNewConnInUseEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient     - The SipTranscClient pointer
 *            hConn          - The new connection in use.
 *            bNewConnCreated - The connection is also a newly created connection
 *                               (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientCallbackNewConnInUseEv(
                     IN  SipTranscClient*                     pTranscClient,
                     IN  RvSipTransportConnectionHandle hConn,
                     IN  RvBool                         bNewConnCreated)
{
    SipTripleLock            *pTripleLock;
    SipTranscClientOwnerHandle   hTranscClientOwner = pTranscClient->hOwner;
    SipTranscClientEvHandlers *evHandlers    = &pTranscClient->transcClientEvHandlers;
    RvStatus                  rv            = RV_OK;

    pTripleLock = pTranscClient->tripleLock;
    if(evHandlers != NULL && (*evHandlers).pfnNewConnInUseEvHandler != NULL)
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                     "TranscClientCallbackNewConnInUseEv: hTranscClient=0x%p, hTranscClientOwner=0x%p, hConn=0x%p Before callback",
                     pTranscClient,hTranscClientOwner,hConn));

        rv = (*evHandlers).pfnNewConnInUseEvHandler(
                                           hTranscClientOwner,
                                           hConn,
                                           bNewConnCreated);

        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCallbackNewConnInUseEv: Transc-client 0x%p,After callback, (rv=%d)",pTranscClient,rv));

    }
    else
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientCallbackNewConnInUseEv: hTranscClient=0x%p, hTranscClientOwner=0x%p, hConn=0x%p,Client did not registered to callback",
                  pTranscClient,hTranscClientOwner,hConn));

    }

    return rv;
}




#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * TranscClientCallbackSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnSigCompMsgNotRespondedEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hTranscClient     - The TranscClient handle for this transaction.
 *          retransNo     - The number of retransmissions of the request
 *                          until now.
 *          ePrevMsgComp  - The type of the previous not responded request
 * Output:  peNextMsgComp - The type of the next request to retransmit (
 *                          RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                          client wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientCallbackSigCompMsgNotRespondedEv(
                     IN  RvSipTranscHandle            hTransc,
                     IN  RvSipTranscOwnerHandle       hTranscClient,
                     IN  RvInt32                      retransNo,
                     IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                     OUT RvSipTransmitterMsgCompType *peNextMsgComp)
{
    SipTripleLock            *pTripleLock;
    SipTranscClient          *pTranscClient    = (SipTranscClient*)hTranscClient;
    SipTranscClientOwnerHandle   hTranscClientOwner = pTranscClient->hOwner;
    SipTranscClientEvHandlers *evHandlers    = &pTranscClient->transcClientEvHandlers;
    RvStatus                  rv            = RV_OK;
 
    pTripleLock = pTranscClient->tripleLock;

      if(evHandlers != NULL && (*evHandlers).pfnSigCompMsgNotRespondedEvHandler != NULL)
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "TranscClientCallbackSigCompMsgNotRespondedEv: hTranscClient=0x%p, hTransc=0x%p, retrans=%d, prev=%s Before callback",
            pTranscClient,hTransc,retransNo,SipTransactionGetMsgCompTypeName(ePrevMsgComp)));

        rv = (*evHandlers).pfnSigCompMsgNotRespondedEvHandler(
                                           hTransc,
                                           hTranscClientOwner,
                                           retransNo,
                                           ePrevMsgComp,
                                           peNextMsgComp);

        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
          "TranscClientCallbackSigCompMsgNotRespondedEv: hTranscClient=0x%p,After callback, (rv=%d)",pTranscClient,rv));
    }
    else
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "TranscClientCallbackSigCompMsgNotRespondedEv: hTranscClient=0x%p, hTransc=0x%p, retrans=%d, prev=%s,Client did not registered to callback",
            pTranscClient,hTransc,retransNo,SipTransactionGetMsgCompTypeName(ePrevMsgComp)));
    }

    return rv;
}
#endif /* RV_SIGCOMP_ON */

/***************************************************************************
 * TranscClientCallbackExpirationAlertEv
 * ------------------------------------------------------------------------
 * General: Notify the client about the object expiration in the server.
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments: pTranscClient - The pointer of the expired object.
 * Input:     
 ***************************************************************************/
void RVCALLCONV TranscClientCallbackExpirationAlertEv(SipTranscClient *pTranscClient)
{
	SipTripleLock            *pTripleLock;
    SipTranscClientOwnerHandle   hTranscClientOwner = pTranscClient->hOwner;
    SipTranscClientEvHandlers *evHandlers    = &pTranscClient->transcClientEvHandlers;
	
    pTripleLock = pTranscClient->tripleLock;
    if(evHandlers != NULL && (*evHandlers).pfnObjExpiredEvHandler != NULL)
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"TranscClientCallbackExpirationAlertEv: hTranscClient=0x%p, hTranscClientOwner=0x%p, Before callback",
			pTranscClient,hTranscClientOwner));
		
        (*evHandlers).pfnObjExpiredEvHandler(
			hTranscClientOwner);
		
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"TranscClientCallbackExpirationAlertEv: Transc-client 0x%p,After callback.",pTranscClient));
		
    }
    else
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"TranscClientCallbackExpirationAlertEv: hTranscClient=0x%p, hTranscClientOwner=0x%p, Client did not registered to callback",
			pTranscClient,hTranscClientOwner));
		
    }
	
}

/***************************************************************************
 * TranscClientCallbackExpirationAlertEv
 * ------------------------------------------------------------------------
 * General: Notify the client about the object expiration in the server.
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments: pTranscClient - The pointer of the expired object.
 * Input:     
 ***************************************************************************/
void RVCALLCONV TranscClientCallbackTerminatedEv(SipTranscClient *pTranscClient)
{
	SipTripleLock            *pTripleLock;
    SipTranscClientOwnerHandle   hTranscClientOwner = pTranscClient->hOwner;
    SipTranscClientEvHandlers *evHandlers    = &pTranscClient->transcClientEvHandlers;
	
    pTripleLock = pTranscClient->tripleLock;
    if(evHandlers != NULL && (*evHandlers).pfnObjTerminatedEvHandler != NULL)
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"TranscClientCallbackTerminatedEv: hTranscClient=0x%p, hTranscClientOwner=0x%p, Before callback",
			pTranscClient,pTranscClient->hOwner));
		
        (*evHandlers).pfnObjTerminatedEvHandler(hTranscClientOwner);
		
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"TranscClientCallbackTerminatedEv: Transc-client 0x%p,After callback.",pTranscClient));
		
    }
    else
    {
        RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"TranscClientCallbackTerminatedEv: hTranscClient=0x%p, hTranscClientOwner=0x%p, Client did not registered to callback",
			pTranscClient,hTranscClientOwner));
		
    }
	
}


/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

#endif /* RV_SIP_PRIMITIVES */
