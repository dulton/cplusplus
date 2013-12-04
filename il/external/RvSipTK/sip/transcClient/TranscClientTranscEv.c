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
 *                              <RegClientTranscEv.c>
 *
 *  Handles state changed events received from the transaction layer.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Oct 2006
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "TranscClientTranscEv.h"
#include "TranscClientObject.h"
#include "TranscClientCallbacks.h"
#include "RvSipMsg.h"
#include "RvSipAddress.h"
#include "_SipAuthenticator.h"
#include "RvSipContactHeader.h"

#ifdef RV_SIP_IMS_ON
#include "TranscClientSecAgree.h"
#endif /* RV_SIP_IMS_ON */

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static void RVCALLCONV TranscClientTransactionGenFinalResponseRcvd(
                              IN SipTranscClient                  *pTranscClient,
                              IN RvSipTranscHandle                 hTransaction);
static void RVCALLCONV TranscClientTransactionTerminated(
                              IN SipTranscClient                  *pTranscClient,
                              IN RvSipTranscHandle                 hTransaction,
                              IN RvSipTransactionStateChangeReason eReason);

static void RVCALLCONV ResetReceivedMsg(
                            IN  SipTranscClient          *pTranscClient,
                            IN  RvSipTransactionState    eTranscState);

static RvStatus RVCALLCONV UpdateReceivedMsg(
                            IN  SipTranscClient        *pTranscClient,
                            IN  RvSipTranscHandle      hTransc,
                            IN  RvSipTransactionState  eTranscState,
							IN  RvSipTransactionStateChangeReason eTranscReason);

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
static RvStatus RVCALLCONV HandleTranscStateMsgSendFailure(
                                IN  SipTranscClient		               *pTranscClient,
                                IN  RvSipTranscHandle                   hTransc,
                                IN  RvSipTransactionStateChangeReason   eStateReason);
#endif

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/



/***************************************************************************
 * TranscClientTranscEvStateChanged
 * ------------------------------------------------------------------------
 * General: Called when a state changes in the active transactions.
 *          The Transc-client will change state accordingly.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pOwner -       The owner of the transaction in which the "event
 *                         state changed" occurred.
 *            hTransaction - The transaction in which the "event state changed"
 *                         occurred.
 *            eNewState -    The new state of the transaction.
 *            eReason -      The reason for the state change.
 ***************************************************************************/
void RVCALLCONV TranscClientTranscEvStateChanged(
                            IN RvSipTranscHandle                   hTransaction,
                            IN RvSipTranscOwnerHandle              hTranscClient,
                            IN RvSipTransactionState               eNewState,
                            IN RvSipTransactionStateChangeReason   eReason)
{
    SipTranscClient   *pTranscClient;
    RvStatus           rv;

    if (NULL == hTranscClient)
    {
        return;
    }
    pTranscClient = (SipTranscClient *)hTranscClient;

	/* if this state change occurred due to a received message, update
       this message in the Transc-client */
    rv = UpdateReceivedMsg(pTranscClient, hTransaction, eNewState, eReason);
    if (RV_OK != rv)
    {
        return;
    }

	switch (eNewState)
    {
    case RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT:
    case RVSIP_TRANSC_STATE_CLIENT_GEN_PROCEEDING:
        break;
    case RVSIP_TRANSC_STATE_CLIENT_GEN_FINAL_RESPONSE_RCVD:
		TranscClientTransactionGenFinalResponseRcvd(pTranscClient, hTransaction);
        break;
    case RVSIP_TRANSC_STATE_TERMINATED:
        TranscClientTransactionTerminated(pTranscClient, hTransaction, eReason);
        break;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    case RVSIP_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE:
        rv = HandleTranscStateMsgSendFailure(pTranscClient,hTransaction,eReason);
        break;
#endif
    default:
        /* The transaction must be a general transaction thus has one of
           the states above */
        RvLogExcep(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientTranscEvStateChanged - Transc-client 0x%p: transaction changed state to un-expected state",
                  pTranscClient));
    break;
    }
    ResetReceivedMsg(pTranscClient, eNewState);
}

/***************************************************************************
 * TranscClientTranscEvMsgToSend
 * ------------------------------------------------------------------------
 * General: Called right before the transaction attempts to send a message
 * (not retransmission). 
 * Return Value: RV_OK - success.
 *               RV_ERROR_OUTOFRESOURCES - No resources to add personal information
 *                                   to the message.
 *               RV_ERROR_UNKNOWN - failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pOwner -       The owner of the transaction in which the "event
 *                         message to send" occurred.
 *            hTransaction - The transaction in which the "event message to
 *                         send" occurred.
 *            pMsgToSend -   The message the transaction is going to send. After
 *                         the callback returns the message object can not be
 *                         used anymore (by the "owner" of this callback).
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientTranscEvMsgToSend(
                                       IN RvSipTranscHandle        hTransaction,
                                       IN RvSipTranscOwnerHandle   hTranscClient,
                                       IN RvSipMsgHandle           hMsgToSend)
{
    SipTranscClient                *pTranscClient = (SipTranscClient *)hTranscClient;
    RvSipHeaderListElemHandle hListElem;
    void                     *hMsgListElem;
    RvStatus                 retStatus = RV_OK;

    RV_UNUSED_ARG(hTransaction);

	/* Add the Expires header associate with the transaction-client to the
       message before sending */
    if (NULL != pTranscClient->hExpireHeader)
    {
        hListElem = NULL;
        retStatus = RvSipMsgPushHeader(
                           hMsgToSend, RVSIP_LAST_HEADER,
                           (void *)(pTranscClient->hExpireHeader),
                           RVSIP_HEADERTYPE_EXPIRES, &hListElem,
                           &hMsgListElem);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
    }

#ifdef RV_SIP_AUTH_ON
    /* Add authorization headers to the message if exist */
    if (RVSIP_MSG_REQUEST==RvSipMsgGetMsgType(hMsgToSend))
    {
        /* Build the Authorization headers in the Msg */
        retStatus = SipAuthenticatorBuildAuthorizationListInMsg(
                                pTranscClient->pMgr->hAuthModule, 
								hMsgToSend, 
								(void*)pTranscClient->hOwner,
                                pTranscClient->eOwnerType,
                                pTranscClient->tripleLock, 
                                pTranscClient->pAuthListInfo,
                                pTranscClient->hListAuthObj);
        if(retStatus != RV_OK)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "TranscClientTranscEvMsgToSend - TranscClient 0x%p - Failed to build authorization headers (rv=%d:%s)",
                pTranscClient, retStatus, SipCommonStatus2Str(retStatus)));
            return retStatus;
        }
    }
#endif /* #ifdef RV_SIP_AUTH_ON */
#ifdef RV_SIP_IMS_ON
    if(NULL!=pTranscClient->hInitialAuthorization && RVSIP_MSG_REQUEST==RvSipMsgGetMsgType(hMsgToSend))
    {
        /* insert the empty authorization header to a request message, 
           only if no other authorization header was set */
        if(NULL == pTranscClient->hListAuthObj)
        {
            hListElem = NULL;
            retStatus = RvSipMsgPushHeader( hMsgToSend, RVSIP_LAST_HEADER,
                                            (void *)(pTranscClient->hInitialAuthorization),
                                            RVSIP_HEADERTYPE_AUTHORIZATION, &hListElem,
                                            &hMsgListElem);
            if (RV_OK != retStatus)
            {
                RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                    "TranscClientTranscEvMsgToSend - TranscClient 0x%p - Failed to insert the empty authorization header (rv=%d:%s)",
                    pTranscClient, retStatus, SipCommonStatus2Str(retStatus)));
                return retStatus;
            }
        }
    }

	/* Add security-agreement information to outgoing messages */
	retStatus = TranscClientSecAgreeMsgToSend(pTranscClient, hMsgToSend);
	if (RV_OK != retStatus)
	{
		RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
					"TranscClientTranscEvMsgToSend - Transc-client 0x%p - Failed to add security information to message - message will not be sent",
					pTranscClient));
        return retStatus;
	}
#endif /*#ifdef RV_SIP_IMS_ON*/

    /* Call event message to send of the client */
    retStatus = TranscClientCallbackMsgToSendEv((SipTranscClientHandle)pTranscClient, hMsgToSend);

    return retStatus;
}


/***************************************************************************
 * TranscClientTranscEvMsgReceived
 * ------------------------------------------------------------------------
 * General: Called when the transaction has received a new message (not
 * retransmission). Calls the event message received of the client.
 * Return Value: -.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pOwner -       The owner of the transaction in which the "event
 *                         message received" occurred.
 *            pTransaction - The transaction in which the "event message
 *                         received" occurred.
 *            pMsgReceived - The new message the transaction has received. After
 *                         the callback returns the message object can not be
 *                         used anymore (by the "owner" of this callback).
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientTranscEvMsgReceived (
                                  IN RvSipTranscHandle       hTransaction,
                                  IN RvSipTranscOwnerHandle  hTranscClient,
                                  IN RvSipMsgHandle          hMsgReceived)
{
    SipTranscClient                *pTranscClient;
    RvStatus                       retStatus = RV_OK;
    RvInt16                        responseCode = RvSipMsgGetStatusCode(hMsgReceived);
	RvSipContactHeaderHandle		hContactToRedirect;
	RvSipAddressHandle              hAddress;
	RvSipHeaderListElemHandle       hListElem = NULL;
    

    RV_UNUSED_ARG(hTransaction);
    pTranscClient = (SipTranscClient *)hTranscClient;
    /* Call event message received of the application */
    retStatus = TranscClientCallbackMsgReceivedEv ((SipTranscClientHandle)pTranscClient,
                                                hMsgReceived);
    if (retStatus !=RV_OK)
    {
        return retStatus;
    }

	responseCode = RvSipMsgGetStatusCode(hMsgReceived);
#ifdef RV_SIP_AUTH_ON
    if(401 == responseCode || 407 == responseCode)
    {

		retStatus = SipAuthenticatorUpdateAuthObjListFromMsg(
			pTranscClient->pMgr->hAuthModule, hMsgReceived,
			pTranscClient->hPage, (void*)pTranscClient->hOwner,
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
			pTranscClient->ownerLockUtils.pfnOwnerLock, pTranscClient->ownerLockUtils.pfnOwnerUnlock,
#else
			NULL, NULL,
#endif
			&pTranscClient->pAuthListInfo, &pTranscClient->hListAuthObj);
		if (RV_OK != retStatus)
		{
			RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
				"TranscClientTranscEvMsgReceived - TranscClient 0x%p (hMsg=0x%p) - failed to load Authentication data, rv=%d (%s)",
				pTranscClient, hMsgReceived, retStatus, SipCommonStatus2Str(retStatus)));
		}
    }
#endif /* #ifdef RV_SIP_AUTH_ON */

    if ((300 <= responseCode) &&
        (400 > responseCode))
    {
		/* The register is redirected. The first Contact header will be
		copied to the Request-Uri object of the register-client */
		hContactToRedirect = RvSipMsgGetHeaderByType(
								   hMsgReceived, RVSIP_HEADERTYPE_CONTACT,
								   RVSIP_FIRST_HEADER, &hListElem);
		if (NULL != hContactToRedirect)
		{
		   hAddress = RvSipContactHeaderGetAddrSpec(hContactToRedirect);
		   if (NULL != hAddress)
		   {
			   retStatus = RvSipAddrCopy(pTranscClient->hRequestUri, hAddress);
			   if (RV_OK != retStatus)
			   {
				  RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
						"TranscClientTranscEvMsgReceived - TranscClient 0x%p (hMsg=0x%p) - failed to copy contact (0x%p) to request request-uri (rv=%d).",
						pTranscClient, hMsgReceived, hContactToRedirect, retStatus));
			   }
		   }
		   else
		   {
			   RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
						"TranscClientTranscEvMsgReceived - TranscClient 0x%p (hMsg=0x%p) - failed to copy contact (0x%p) to request request-uri (rv=%d).",
						pTranscClient, hMsgReceived, hContactToRedirect, retStatus));
		   }
		}
		else
		{
		   RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
						"TranscClientTranscEvMsgReceived - TranscClient 0x%p (hMsg=0x%p) - no Contact header on Redirected response.",
						pTranscClient, hMsgReceived));
		}
    }

#ifdef RV_SIP_IMS_ON 
	TranscClientSecAgreeMsgRcvd(pTranscClient, hMsgReceived);
#endif /* #ifdef RV_SIP_IMS_ON */

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
 *            hAddress       - Handle to unsupported address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientTranscEvOtherURLAddressFound(
                     IN  RvSipTranscHandle      hTransc,
                     IN  RvSipTranscOwnerHandle hTranscClient,
                     IN  RvSipMsgHandle         hMsg,
                     IN  RvSipAddressHandle     hAddress,
                     OUT RvSipAddressHandle     hSipURLAddress,
                     OUT RvBool               *bAddressResolved)
{
    if (NULL == hTranscClient || NULL == hAddress || NULL == hSipURLAddress)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    TranscClientCallbackOtherURLAddressFoundEv(hTransc, hTranscClient, hMsg, hAddress,
                                             hSipURLAddress, bAddressResolved);

    return RV_OK;
}

/***************************************************************************
 * TranscClientTranscFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: This callback indicates that the transaction is about to send
 *          a message after the destination address was resolved.
 *          The callback supplies the final message.
 *          Changes in the message at this stage will
 *          not effect the destination address.
 *
 * Return Value: RvStatus. If the application returns a value other then
 *               RV_OK the message will not be sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc      - The transaction handle.
 *            hTranscClient   - The transc-client handle.
 *            hMsgToSend   - The handle to the outgoing message.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientTranscFinalDestResolvedEv(
                            IN RvSipTranscHandle       hTransc,
                            IN RvSipTranscOwnerHandle  hTranscClient,
                            IN RvSipMsgHandle          hMsgToSend)
{
    RvStatus                rv;
    SipTranscClient			   *pTranscClient = (SipTranscClient*)hTranscClient;

    if (NULL == hTranscClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
#ifdef RV_SIP_IMS_ON 
	TranscClientSecAgreeDestResolved(pTranscClient, hMsgToSend, hTransc);
#endif /* RV_SIP_IMS_ON */

      rv = TranscClientCallbackFinalDestResolvedEv(hTransc,
                                                (RvSipTranscOwnerHandle)pTranscClient,
                                                hMsgToSend);
    if (RV_OK != rv)
    {
        return rv;
    }


	RV_UNUSED_ARG(hTransc || pTranscClient)

    return rv;
}

/***************************************************************************
 * TranscClientTranscNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: Notifies the client that the transc-client is now using a new
 *          connection. The connection can be a totally new connection or
 *          a suitable connection that was found in the hash.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                         otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hTranscClient     - SipTranscClient handle
 *            hConn          - The new connection handle.
 *            bNewConnCreated - The connection is also a newly created connection
 *                               (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientTranscNewConnInUseEv(
                     IN  RvSipTranscHandle              hTransc,
                     IN  RvSipTranscOwnerHandle         hTranscClient,
                     IN  RvSipTransportConnectionHandle hConn,
                     IN  RvBool                         bNewConnCreated)
{
    SipTranscClient              *pTranscClient   =  (SipTranscClient*)hTranscClient;
    RvStatus              rv            = RV_OK;
	RvBool					bCallback = RV_TRUE;
	
	if (NULL == hTranscClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	/*if this is a connection the call-leg already knows*/
    if(hConn == pTranscClient->hTcpConn)
    {
		bCallback = RV_FALSE;
        return RV_OK;
    }
	
    RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
		"TranscClientTranscNewConnInUseEv - pTranscClient 0x%p - transc 0x%p - new conn 0x%p in use ",
		pTranscClient,hTransc,hConn));
	
	/*attach to the new connection and notify the application*/
    if(pTranscClient->bIsPersistent == RV_TRUE)
    {
        if(pTranscClient->hTcpConn != NULL)
        {
            TranscClientDetachFromConnection(pTranscClient);
        }
		
		rv = TranscClientAttachOnConnection(pTranscClient,hConn);
        if(rv != RV_OK)
        {
            RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                "TranscClientTranscNewConnInUseEv - Call-leg 0x%p - transc 0x%p - failed to attach to conn 0x%p",
                pTranscClient,hTransc,hConn));
			bCallback = RV_FALSE;
            return rv;
        }
    }
    
	if (bCallback == RV_TRUE)
	{
		rv = TranscClientCallbackNewConnInUseEv(pTranscClient,
			hConn,
			bNewConnCreated);
	}

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(hTransc);
#endif

    return rv;
}





#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * TranscClientTranscSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: This callback indicates that a SigComp compressed request
 *          was sent over the transaction and a SigComp timer expired
 *          before receiving any response. This callback is part of a
 *          recovery mechanism and it supplies information about the previous
 *          sent request (that wasn't responded yet). 
 *          NOTE: A RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED value of the out
 *          parameter is referred as a request from the client to block
 *          the sending of additional requests in the current transaction
 *          session.
 *
 * Return Value: RvStatus. If the client returns a value other then
 *               RV_OK no further message will be sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc       - The transaction handle.
 *            hTranscClient    - The Transc-client handle.
 *            retransNo     - The number of retransmissions of the request
 *                            until now.
 *            ePrevMsgComp  - The type of the previous not responded request
 * Output:    peNextMsgComp - The type of the next request to retransmit (
 *                            RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                            client wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientTranscSigCompMsgNotRespondedEv(
                     IN  RvSipTranscHandle            hTransc,
                     IN  RvSipTranscOwnerHandle       hTranscClient,
                     IN  RvInt32                      retransNo,
                     IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                     OUT RvSipTransmitterMsgCompType *peNextMsgComp)
{
    if (NULL == hTranscClient || NULL == hTransc || NULL == peNextMsgComp)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    return TranscClientCallbackSigCompMsgNotRespondedEv(hTransc,
                                                     hTranscClient,
                                                     retransNo,
                                                     ePrevMsgComp,
                                                     peNextMsgComp);
}
#endif /* RV_SIGCOMP_ON */

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TranscClientTransactionGenFinalResponseRcvd
 * ------------------------------------------------------------------------
 * General: Called when the active transaction received a final response to
 *          the request. Changes the transc-client's state
 *          accordingly and informs the client.
 * Return Value: -.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - The transc-client handle.
 *          hTransaction - The handle to the transaction that received the final
 *          response.
 ***************************************************************************/
static void RVCALLCONV TranscClientTransactionGenFinalResponseRcvd(
                              IN SipTranscClient                      *pTranscClient,
                              IN RvSipTranscHandle              hTransaction)
{
    RvUint16 responseCode;


    if (SIP_TRANSC_CLIENT_STATE_ACTIVATING != pTranscClient->eState)
    {
        TranscClientTerminate(pTranscClient,SIP_TRANSC_CLIENT_REASON_LOCAL_FAILURE);
        RvLogExcep(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientTransactionGenFinalResponseRcvd - Transc client 0x%p: response in un-expected state - terminating",
                  pTranscClient));
        return;
    }

    RvSipTransactionGetResponseCode(hTransaction,&responseCode);
    /* 2xx response was received */
    if((responseCode >= 200) && (responseCode < 300))
    {
        TranscClientChangeState(
                 SIP_TRANSC_CLIENT_STATE_ACTIVATED,
                 SIP_TRANSC_CLIENT_REASON_RESPONSE_SUCCESSFUL_RECVD,
                 pTranscClient, responseCode);
    }
    /* 3xx response was received */
    else if((responseCode >= 300) && (responseCode < 400))
    {
		if (pTranscClient->bWasActivated == RV_FALSE)
		{
			TranscClientChangeState(
				SIP_TRANSC_CLIENT_STATE_REDIRECTED,
				SIP_TRANSC_CLIENT_REASON_REDIRECT_RESPONSE_RECVD,
				pTranscClient, responseCode);
		}
		else
		{
			TranscClientChangeState(
				SIP_TRANSC_CLIENT_STATE_ACTIVATED,
				SIP_TRANSC_CLIENT_REASON_REDIRECT_RESPONSE_RECVD,
				pTranscClient, responseCode);
		}
        
    }
    /* 4xx response was received */
    else if((responseCode >= 400) && (responseCode < 500))
    {
#ifdef RV_SIP_AUTH_ON
        if ((401==responseCode || 407==responseCode) && 
			pTranscClient->hListAuthObj != NULL)
        {
            RvBool bValid;
            RvStatus rv;
            
			rv = SipAuthenticatorCheckValidityOfAuthList(
										pTranscClient->pMgr->hAuthModule,
										pTranscClient->pAuthListInfo, 
										pTranscClient->hListAuthObj, &bValid);
			if (RV_OK != rv)
            {
                RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                    "TranscClientTransactionGenFinalResponseRcvd - Transc client 0x%p: failed to check Authentication objects validity (rv=%d:%s)",
                    pTranscClient, rv, SipCommonStatus2Str(rv)));
                TranscClientTerminate(pTranscClient,SIP_TRANSC_CLIENT_REASON_LOCAL_FAILURE);
                return;
            }
            
			if(bValid == RV_TRUE)
            {
				if (pTranscClient->bWasActivated == RV_FALSE)
				{
					TranscClientChangeState(SIP_TRANSC_CLIENT_STATE_UNAUTHENTICATED,
						SIP_TRANSC_CLIENT_REASON_UNAUTHENTICATE,
						pTranscClient, responseCode);      
				}
				else 
				{
					TranscClientChangeState(SIP_TRANSC_CLIENT_STATE_ACTIVATED,
						SIP_TRANSC_CLIENT_REASON_UNAUTHENTICATE,
						pTranscClient, responseCode);      
				}				          
            }
            else
            {
                RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                    "TranscClientTransactionGenFinalResponseRcvd - Transc client 0x%p: invalid Authentication challenge",
                    pTranscClient));
                TranscClientChangeState(SIP_TRANSC_CLIENT_STATE_FAILED,
                                    SIP_TRANSC_CLIENT_REASON_RESPONSE_REQUEST_FAILURE_RECVD,
                                     pTranscClient, responseCode);
            }
        }
        else
#endif /* #ifdef RV_SIP_AUTH_ON */
        {
            /* 4xx other than 401,407 was received or 401/407 not supported*/
            TranscClientChangeState(SIP_TRANSC_CLIENT_STATE_FAILED,
                               SIP_TRANSC_CLIENT_REASON_RESPONSE_REQUEST_FAILURE_RECVD,
                               pTranscClient, responseCode);
        }
    }
    /* 5xx response was received */
    else if((responseCode >= 500) && (responseCode < 600))
    {
        TranscClientChangeState(
                            SIP_TRANSC_CLIENT_STATE_FAILED,
                            SIP_TRANSC_CLIENT_REASON_RESPONSE_SERVER_FAILURE_RECVD,
                            pTranscClient, responseCode);
    }
    /* 6xx response was received */
    else if((responseCode >= 600) && (responseCode < 700))
    {
            TranscClientChangeState(
                            SIP_TRANSC_CLIENT_STATE_FAILED,
                            SIP_TRANSC_CLIENT_REASON_RESPONSE_GLOBAL_FAILURE_RECVD,
                            pTranscClient, responseCode);
    }
    else
    {
        /* Exception. The status code of the final response is not legal. */
        RvLogExcep(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                  "TranscClientTransactionGenFinalResponseRcvd - Transc client 0x%p: final response received with illegal status code %d",
                  pTranscClient, responseCode));
        TranscClientTerminate(pTranscClient,SIP_TRANSC_CLIENT_REASON_LOCAL_FAILURE);

    }
}


/***************************************************************************
 * TranscClientTransactionTerminated
 * ------------------------------------------------------------------------
 * General: Called when the active transaction has terminated. Changes the
 *          Transc-client's state accordingly and informs the application.
 * Return Value: -.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - The Transc-client handle
 *          hTransaction - The handle to the terminated transaction
 *          eReason - The reason for the transaction's termination.
 ***************************************************************************/
static void RVCALLCONV TranscClientTransactionTerminated(
                              IN SipTranscClient                      *pTranscClient,
                              IN RvSipTranscHandle                 hTransaction,
                              IN RvSipTransactionStateChangeReason eReason)
{
    RV_UNUSED_ARG(hTransaction);
    
	if (pTranscClient->hActiveTransc != hTransaction)
	{
		/*In case we are getting notification about transaction terminated which is not
		current active transaction, we have nothing to do.*/
		return;
	}

	/*Nullify the active transaction of the transc-client object*/
	pTranscClient->hActiveTransc = NULL;
	
#ifdef RV_SIP_IMS_ON
	if (eReason == RVSIP_TRANSC_REASON_TIME_OUT ||
		eReason == RVSIP_TRANSC_REASON_NETWORK_ERROR ||
		eReason == RVSIP_TRANSC_REASON_ERROR ||
		eReason == RVSIP_TRANSC_REASON_OUT_OF_RESOURCES)
	{
		/* Notify the transaction-client object on the failure in trancation */
		TranscClientSecAgreeMsgSendFailure(pTranscClient);		
	}
#endif /* #ifdef RV_SIP_IMS_ON */
	
    switch (eReason)
    {
    case RVSIP_TRANSC_REASON_TIME_OUT:
        /* The register request failed since no response was received */
        TranscClientChangeState(SIP_TRANSC_CLIENT_STATE_FAILED,
                             SIP_TRANSC_CLIENT_REASON_TRANSACTION_TIMEOUT,
                             pTranscClient, 0);
        break;
    case RVSIP_TRANSC_REASON_NETWORK_ERROR:
    case RVSIP_TRANSC_REASON_ERROR:
	case RVSIP_TRANSC_REASON_OUT_OF_RESOURCES:
        TranscClientChangeState(SIP_TRANSC_CLIENT_STATE_FAILED,
                             SIP_TRANSC_CLIENT_REASON_LOCAL_FAILURE,
                             pTranscClient, 0);
        break;

        /* the following cases are taken care of in the final response received transaction state */
    default:
        /* Exception. The transaction has terminated with unexpected reason */
        break;
    }

}



/***************************************************************************
 * UpdateReceivedMsg
 * ------------------------------------------------------------------------
 * General: Update the received message in the register-client.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient - The register handle
 *          hTransc - The transaction handle.
 *          eTranscState  - The transaction state.
 ***************************************************************************/
static RvStatus RVCALLCONV UpdateReceivedMsg(
                            IN  SipTranscClient             *pTranscClient,
                            IN  RvSipTranscHandle      hTransc,
                            IN  RvSipTransactionState  eTranscState,
							IN  RvSipTransactionStateChangeReason eTranscReason)
{
    RvStatus rv = RV_OK;

	switch (eTranscState)
    {
    case RVSIP_TRANSC_STATE_CLIENT_GEN_PROCEEDING:
    case RVSIP_TRANSC_STATE_CLIENT_GEN_FINAL_RESPONSE_RCVD:
		rv = RvSipTransactionGetReceivedMsg(hTransc, &(pTranscClient->hReceivedMsg));
		if (RV_OK != rv)
		{
			pTranscClient->hReceivedMsg = NULL;
		}
        break;
	case RVSIP_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE:
		if (eTranscReason == RVSIP_TRANSC_REASON_503_RECEIVED)
		{
			rv = RvSipTransactionGetReceivedMsg(hTransc, &(pTranscClient->hReceivedMsg));
			if (RV_OK != rv)
			{
				pTranscClient->hReceivedMsg = NULL;
			}
		}
		break;
    default:
        break;
    }
    return rv;
}

/***************************************************************************
 * ResetReceivedMsg
 * ------------------------------------------------------------------------
 * General: Reset the received message in the register-client.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient - The register handle
 *          eTranscState  - The transaction state.
 ***************************************************************************/
static void RVCALLCONV ResetReceivedMsg(
                            IN  SipTranscClient               *pTranscClient,
                            IN  RvSipTransactionState    eTranscState)
{
    switch (eTranscState)
    {
    case RVSIP_TRANSC_STATE_CLIENT_GEN_PROCEEDING:
    case RVSIP_TRANSC_STATE_CLIENT_GEN_FINAL_RESPONSE_RCVD:
		pTranscClient->hReceivedMsg = NULL;
    default:
        break;
    }
}


#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/***************************************************************************
* HandleTranscStateMsgSendFailure
* ------------------------------------------------------------------------
* General: This function handles the client_msg_send_failure transaction state.
* Return Value:
* ------------------------------------------------------------------------
* Arguments:
* Input:     pTranscClient - the TranscClient related to this transaction.
*          hTransc - the transaction handle
*          eMethod - method of the transaction.
*          eStateReason - The reason for this state.
* Output:  (-)
***************************************************************************/
static RvStatus RVCALLCONV HandleTranscStateMsgSendFailure(
                            IN  SipTranscClient                         *pTranscClient,
                            IN  RvSipTranscHandle						hTransc,
                            IN  RvSipTransactionStateChangeReason		eStateReason)

{

    SipTranscClientStateChangeReason eReason;
	RvStatus						rv = RV_OK;

    RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
              "HandleTranscStateMsgSendFailure - SipTranscClient 0x%p - Transaction 0x%p moved to the msg send failure state",
              pTranscClient,hTransc));

    switch(eStateReason)
    {
    case RVSIP_TRANSC_REASON_NETWORK_ERROR:
        eReason = SIP_TRANSC_CLIENT_REASON_NETWORK_ERROR;
        break;
    case RVSIP_TRANSC_REASON_503_RECEIVED:
        eReason = SIP_TRANSC_CLIENT_REASON_503_RECEIVED;
        break;
    case RVSIP_TRANSC_REASON_TIME_OUT:
        eReason = SIP_TRANSC_CLIENT_REASON_TRANSACTION_TIMEOUT;
        break;
    default:
        eReason = SIP_TRANSC_CLIENT_REASON_UNDEFINED;
    }
    TranscClientChangeState(SIP_TRANSC_CLIENT_STATE_MSG_SEND_FAILURE,
                                  eReason,pTranscClient, 0);

	
	
#ifdef RV_SIP_IMS_ON
	rv = TranscClientSecAgreeMsgSendFailure(pTranscClient);
#endif
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(hTransc);
#endif

    return rv;
}
#endif /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/

#endif /* RV_SIP_PRIMITIVES */










































