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
 *                              <PubClientTranscClientEv.c>
 *
 *  Handles state changed events received from the transaction layer.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Sep 2006
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "PubClientTranscClientEv.h"
#include "PubClientObject.h"
#include "PubClientCallbacks.h"
#include "RvSipContactHeader.h"
#include "RvSipMsg.h"
#include "RvSipAddress.h"
#include "_SipAuthenticator.h"

#include "RvSipRetryAfterHeader.h"

#include "_SipTranscClient.h"

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvSipPubClientState PubClientConvertTranscClientState(SipTranscClientState eNewState);
static RvSipPubClientStateChangeReason PubClientConvertTranscClientReason(SipTranscClientStateChangeReason eReason, RvInt16 responseCode);

static void PubClientUpdateRetryAfterIntervalIfNeeded(PubClient *pPubClient);


/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/



/***************************************************************************
 * PubClientTranscClientEvStateChanged
 * ------------------------------------------------------------------------
 * General: Called when a state changes in the active transactions.
 *          The publish-client will change state accordingly.
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
void RVCALLCONV PubClientTranscClientEvStateChanged(
								  IN  SipTranscClientOwnerHandle       hTranscClientOwner,
								  IN  SipTranscClientState			 eNewState,
								  IN  SipTranscClientStateChangeReason eReason,
								  IN  RvInt16							 responseCode)
{
    PubClient						*pPubClient;
	RvSipPubClientState				ePubClientConvertedState;
	RvSipPubClientStateChangeReason ePubClientConvertedReason;
	RvSipMsgHandle					hReceivedMsg;
	RvStatus						rv;
	
	
	if (NULL == hTranscClientOwner)
    {
        return;
    }
    pPubClient = (PubClient *)hTranscClientOwner;
	
	ePubClientConvertedState = PubClientConvertTranscClientState(eNewState);
	ePubClientConvertedReason = PubClientConvertTranscClientReason(eReason, responseCode);

	if (pPubClient->bRemoveInProgress == RV_TRUE)
	{
		if (ePubClientConvertedState == RVSIP_PUB_CLIENT_STATE_PUBLISHING)
		{
			ePubClientConvertedState = RVSIP_PUB_CLIENT_STATE_REMOVING;
		}
		
		if (pPubClient->eState == RVSIP_PUB_CLIENT_STATE_REMOVING)
		{
			if (ePubClientConvertedReason == RVSIP_PUB_CLIENT_REASON_RESPONSE_UNAUTHENTICATED_RECVD
				|| ePubClientConvertedReason == RVSIP_PUB_CLIENT_REASON_RESPONSE_REDIRECTION_RECVD)
			{
				/*We are reseting the remove progress since the application can use the publish() api
				  if it wishes to contiue trying to remove the remove api should be used*/
				pPubClient->bRemoveInProgress = RV_FALSE;
				SipTranscClientSetExpiresHeader(&pPubClient->transcClient, NULL);
				ePubClientConvertedState = RVSIP_PUB_CLIENT_STATE_PUBLISHED;			
			}
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
			/*We are moving to MSG_SEND_FAILURE only if the DNS feature is availeble otherwise we are
			  handling the 503 response as any other reject response.*/
			else if (ePubClientConvertedReason == RVSIP_PUB_CLIENT_REASON_503_RECEIVED || 
				ePubClientConvertedReason == RVSIP_PUB_CLIENT_REASON_NETWORK_ERROR ||
				ePubClientConvertedReason == RVSIP_PUB_CLIENT_REASON_TRANSACTION_TIMEOUT)
			{

				ePubClientConvertedState = RVSIP_PUB_CLIENT_STATE_MSG_SEND_FAILURE;
			}
#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/
			else
			{
				/*We are changing the state from removing to removed no matter 
				what was the response code that triggered this state change*/
				ePubClientConvertedState = RVSIP_PUB_CLIENT_STATE_REMOVED;

				/*Since we success with the remove request we need to stop the timer that is currently running
				  from previous request.*/
				SipTranscClientSetAlertTimer(&pPubClient->transcClient, 0);
			}
			
		}
		
	}	
	
	if (ePubClientConvertedState == RVSIP_PUB_CLIENT_STATE_FAILED ||
		ePubClientConvertedReason == RVSIP_PUB_CLIENT_REASON_503_RECEIVED ||
		ePubClientConvertedReason == RVSIP_PUB_CLIENT_REASON_RESPONSE_REMOTE_REJECT_RECVD)
	{
		/*For certain responseCode we need to check the retry after header.*/
		PubClientUpdateRetryAfterIntervalIfNeeded(pPubClient);
	}
	
	/*If we are changing the state to publish due to succesful response we need to do two thing updating the sip-if-match and set timer for refreshing
	These two things are taken from the message caused the state change.*/
	if (ePubClientConvertedState == RVSIP_PUB_CLIENT_STATE_PUBLISHED && 
		ePubClientConvertedReason == RVSIP_PUB_CLIENT_REASON_RESPONSE_SUCCESSFUL_RECVD)
	{
		rv = SipTranscClientGetReceivedMsg(&pPubClient->transcClient, &hReceivedMsg);
		if (rv != RV_OK)
		{
			RvLogExcep(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
				"PubClientTranscClientEvStateChanged - Publish client 0x%p: Unable to get received message.",
				pPubClient));
		}
		else 
		{
			/*First we are trying to update the sip-if-match header from the incoming response.*/
			rv = PubClientUpdateSipIfMatchHeader(pPubClient, hReceivedMsg);
			if (rv != RV_OK)
			{
				/*If the sip-etag is not present on the incoming message it is not according to the standart\
				  we are changing the reason of the state change to indicate this problem*/
				ePubClientConvertedReason = RVSIP_PUB_CLIENT_REASON_MISSING_SIP_ETAG;
			}
			else 
			{
				/*If the sip-etag is present and was changed successfully we are trying to set a timer accoring
				  to the incoming expires header*/
				rv = PubClientSetAlertTimerIfNeeded(pPubClient, hReceivedMsg);
				if (rv == RV_ERROR_NOT_FOUND)
				{
				/*If the expires header is not present on the incoming message we are changing the reason to indicate
				  this problem to the application*/
					ePubClientConvertedReason = RVSIP_PUB_CLIENT_REASON_MISSING_EXPIRES;
				}
			}
		}
		
	}

	if (ePubClientConvertedState != RVSIP_PUB_CLIENT_STATE_UNDEFINED)
	{
		PubClientChangeState(ePubClientConvertedState, ePubClientConvertedReason, pPubClient);
	}
	
}

/***************************************************************************
 * PubClientTranscClientEvMsgToSend
 * ------------------------------------------------------------------------
 * General: Called right before the transaction attempts to send a message
 * (not retransmission). Adds the list of Contact headers and the Expires
 * header that were set to the publish-client to the message
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
RvStatus RVCALLCONV PubClientTranscClientEvMsgToSend(
								   IN  SipTranscClientOwnerHandle	hTranscClientOwner,
								   IN  RvSipMsgHandle				hMsg)
{
    PubClient                *pPubClient = (PubClient *)hTranscClientOwner;
    RvStatus                 retStatus;
	
	/*Add the sip if match header to the outgoing message*/
	retStatus = SipPubClientAddSipIfMatchHeaderIfNeeded(pPubClient, hMsg);
	if (retStatus != RV_OK)
	{
		RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
			"PubClientTranscClientEvMsgToSend - Publish client 0x%p: Unable to set sip-if-match header to message.",
			pPubClient));
		return retStatus;
	}
	
	retStatus = SipPubClientAddEventHeaderIfNeeded(pPubClient, hMsg);
	if (retStatus != RV_OK)
	{
		RvLogError(pPubClient->pMgr->pLogSrc,(pPubClient->pMgr->pLogSrc,
			"PubClientTranscClientEvMsgToSend - Publish client 0x%p: Unable to set Event header to message.",
			pPubClient));
		return retStatus;
	}

    /* Call event message to send of the application */
    retStatus = PubClientCallbackMsgToSendEv((RvSipPubClientHandle)pPubClient, hMsg);

    return retStatus;
}


/***************************************************************************
 * PubClientTranscClientEvMsgReceived
 * ------------------------------------------------------------------------
 * General: Called when the transaction has received a new message (not
 * retransmission). Calls the event message received of the application.
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
RvStatus RVCALLCONV PubClientTranscClientEvMsgReceived (
                                  IN  SipTranscClientOwnerHandle	hTranscClientOwner,
                                  IN  RvSipMsgHandle				hMsg)
{
    PubClient                      *pPubClient;
    RvStatus                       retStatus = RV_OK;
    

    pPubClient = (PubClient *)hTranscClientOwner;

	/* Call event message received of the application */
    retStatus = PubClientCallbackMsgReceivedEv ((RvSipPubClientHandle)pPubClient,
                                                hMsg);
    return retStatus;
}

/***************************************************************************
 * PubClientTranscClientEvOtherURLAddressFound
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
 *            hAddress       - Handle to unsupported address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV PubClientTranscClientEvOtherURLAddressFound(
														  IN  RvSipTranscHandle			hTransc,
														  IN  SipTranscClientOwnerHandle hTranscClientOwner,
														  IN  RvSipMsgHandle				hMsg,
														  IN  RvSipAddressHandle         hAddress,
														  OUT RvSipAddressHandle         hSipURLAddress,
														  OUT RvBool                *bAddressResolved)
{
    if (NULL == hTranscClientOwner || NULL == hAddress || NULL == hSipURLAddress)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    PubClientCallbackOtherURLAddressFoundEv(hTransc, hTranscClientOwner, hMsg, hAddress,
                                             hSipURLAddress, bAddressResolved);

    return RV_OK;
}

/***************************************************************************
 * PubClientTranscClientFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: This callback indicates that the transaction is about to send
 *          a message after the destination address was resolved.
 *          The callback supplies the final message.
 *          Changes in the message at this stage will
 *          not effect the destination address.
 *          When this callback is called the application can query the
 *          transaction about the destination address using the
 *          RvSipTransactionGetCurrentDestAddr() API function.
 *          If the application wishes it can update the sent-by part of
 *          the top most via. The application must not update the branch parameter.
 *
 * Return Value: RvStatus. If the application returns a value other then
 *               RV_OK the message will not be sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc      - The transaction handle.
 *            hPubClient   - The pub-client handle.
 *            hMsgToSend   - The handle to the outgoing message.
 ***************************************************************************/
RvStatus RVCALLCONV PubClientTranscClientFinalDestResolvedEv(
                            IN RvSipTranscHandle			hTransc,
                            IN SipTranscClientOwnerHandle	hPubClient,
                            IN RvSipMsgHandle				hMsgToSend)
{
    RvStatus                rv;
    PubClient			   *pPubClient = (PubClient*)hPubClient;

    if (NULL == hPubClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
   rv = PubClientCallbackFinalDestResolvedEv(hTransc,
                                            (RvSipTranscOwnerHandle)pPubClient,
	                                         hMsgToSend);
   return rv;
}

/***************************************************************************
 * PubClientTranscClientNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that the pub-client is now using a new
 *          connection. The connection can be a totally new connection or
 *          a suitable connection that was found in the hash.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                         otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hPubClient     - PubClient handle
 *            hConn          - The new connection handle.
 *            bNewConnCreated - The connection is also a newly created connection
 *                               (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV PubClientTranscClientNewConnInUseEv(
                     IN  SipTranscClientOwnerHandle         hPubClient,
                     IN  RvSipTransportConnectionHandle		hConn,
                     IN  RvBool								bNewConnCreated)
{
    PubClient              *pPubClient   =  (PubClient*)hPubClient;
    RvStatus              rv            = RV_OK;

	rv = PubClientCallbackNewConnInUseEv(pPubClient,
			hConn,
			bNewConnCreated);
	    
    return rv;
}

/***************************************************************************
 * PubClientTranscClientObjExpiredEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that the pub-client is about to expire
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient     - PubClient handle
 ***************************************************************************/
void RVCALLCONV PubClientTranscClientObjExpiredEv(
                     IN  SipTranscClientOwnerHandle         hPubClient)
{
    PubClient              *pPubClient   =  (PubClient*)hPubClient;
	
	PubClientCallbackObjExpired(pPubClient);
}

/***************************************************************************
 * PubClientTranscClientObjTerminated
 * ------------------------------------------------------------------------
 * General: Handle transcClient Termianted.
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient     - PubClient handle
 ***************************************************************************/
void RVCALLCONV PubClientTranscClientObjTerminated(
                     IN  SipTranscClientOwnerHandle         hPubClient)
{
    PubClient              *pPubClient   =  (PubClient*)hPubClient;
	
	PubClientDestruct(pPubClient);
}

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * PubClientTranscClientSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: This callback indicates that a SigComp compressed request
 *          was sent over the transaction and a SigComp timer expired
 *          before receiving any response. This callback is part of a
 *          recovery mechanism and it supplies information about the previous
 *          sent request (that wasn't responded yet). The callback also
 *          gives the application the ability to determine if there will be
 *          additional sent request and what is the type of the next sent
 *          request.
 *          NOTE: A RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED value of the out
 *          parameter is referred as a request from the application to block
 *          the sending of additional requests in the current transaction
 *          session.
 *
 * Return Value: RvStatus. If the application returns a value other then
 *               RV_OK no further message will be sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc       - The transaction handle.
 *            hPubClient    - The pub-client handle.
 *            retransNo     - The number of retransmissions of the request
 *                            until now.
 *            ePrevMsgComp  - The type of the previous not responded request
 * Output:    peNextMsgComp - The type of the next request to retransmit (
 *                            RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                            application wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV PubClientTranscClientSigCompMsgNotRespondedEv(
                     IN  RvSipTranscHandle            hTransc,
                     IN  SipTranscClientOwnerHandle   hPubClient,
                     IN  RvInt32                      retransNo,
                     IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                     OUT RvSipTransmitterMsgCompType *peNextMsgComp)
{
    if (NULL == hPubClient || NULL == hTransc || NULL == peNextMsgComp)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    return PubClientCallbackSigCompMsgNotRespondedEv(hTransc,
                                                     (RvSipTranscOwnerHandle)hPubClient,
                                                     retransNo,
                                                     ePrevMsgComp,
                                                     peNextMsgComp);
}
#endif /* RV_SIGCOMP_ON */

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
* PubClientConvertTranscClientState
* ------------------------------------------------------------------------
* General: This function response to return the correct state according to the incoming
*		   new state of the transaction-client object.
* Return Value:
* ------------------------------------------------------------------------
* Arguments:
* Input:   eNewState - The new state of the transc-client
* Output:  (-)
* Return:  The new state of the regClient
***************************************************************************/
static RvSipPubClientState PubClientConvertTranscClientState(SipTranscClientState eNewState)
{
	switch(eNewState) {
	case SIP_TRANSC_CLIENT_STATE_ACTIVATING:
		return RVSIP_PUB_CLIENT_STATE_PUBLISHING;
	case SIP_TRANSC_CLIENT_STATE_ACTIVATED:
		return RVSIP_PUB_CLIENT_STATE_PUBLISHED;
	case SIP_TRANSC_CLIENT_STATE_FAILED:
		return RVSIP_PUB_CLIENT_STATE_FAILED;
	case SIP_TRANSC_CLIENT_STATE_IDLE:
		return RVSIP_PUB_CLIENT_STATE_IDLE;
	case SIP_TRANSC_CLIENT_STATE_MSG_SEND_FAILURE:
		return RVSIP_PUB_CLIENT_STATE_MSG_SEND_FAILURE;
	case SIP_TRANSC_CLIENT_STATE_REDIRECTED:
		return RVSIP_PUB_CLIENT_STATE_REDIRECTED;
	case SIP_TRANSC_CLIENT_STATE_TERMINATED:
		return RVSIP_PUB_CLIENT_STATE_TERMINATED;
	case SIP_TRANSC_CLIENT_STATE_UNAUTHENTICATED:
		return RVSIP_PUB_CLIENT_STATE_UNAUTHENTICATED;
	default:
		return RVSIP_PUB_CLIENT_STATE_UNDEFINED;
	}
}

/***************************************************************************
* PubClientConvertTranscClientReason
* ------------------------------------------------------------------------
* General: This function response to return the correct reason according to the incoming
*		   new reason of the transaction-client object.
* Return Value:
* ------------------------------------------------------------------------
* Arguments:
* Input:   eReason - The new reason of the transc-client
* Output:  (-)
* Return:  The chosen reason of the pubClient
***************************************************************************/
static RvSipPubClientStateChangeReason PubClientConvertTranscClientReason(SipTranscClientStateChangeReason eReason, RvInt16 responseCode)
{
	RV_UNUSED_ARG(responseCode);
	switch(eReason) {
	case SIP_TRANSC_CLIENT_REASON_RESPONSE_SUCCESSFUL_RECVD:
		return RVSIP_PUB_CLIENT_REASON_RESPONSE_SUCCESSFUL_RECVD;

	case SIP_TRANSC_CLIENT_REASON_USER_REQUEST:
		return RVSIP_PUB_CLIENT_REASON_USER_REQUEST;

	case SIP_TRANSC_CLIENT_REASON_RESPONSE_REQUEST_FAILURE_RECVD:
	case SIP_TRANSC_CLIENT_REASON_RESPONSE_SERVER_FAILURE_RECVD:
	case SIP_TRANSC_CLIENT_REASON_RESPONSE_GLOBAL_FAILURE_RECVD:
		if (responseCode == 412)
		{
			return RVSIP_PUB_CLIENT_REASON_RESPONSE_COND_REQ_FAILED;
		}
		return RVSIP_PUB_CLIENT_REASON_RESPONSE_REMOTE_REJECT_RECVD;
	case SIP_TRANSC_CLIENT_REASON_UNAUTHENTICATE:
		return RVSIP_PUB_CLIENT_REASON_RESPONSE_UNAUTHENTICATED_RECVD;
		
	case SIP_TRANSC_CLIENT_REASON_REDIRECT_RESPONSE_RECVD:
		return RVSIP_PUB_CLIENT_REASON_RESPONSE_REDIRECTION_RECVD;

	case SIP_TRANSC_CLIENT_REASON_TRANSACTION_TIMEOUT:
		return RVSIP_PUB_CLIENT_REASON_TRANSACTION_TIMEOUT;

	case SIP_TRANSC_CLIENT_REASON_NETWORK_ERROR:
		return RVSIP_PUB_CLIENT_REASON_NETWORK_ERROR;

	case SIP_TRANSC_CLIENT_REASON_503_RECEIVED:
		return RVSIP_PUB_CLIENT_REASON_503_RECEIVED;

	case SIP_TRANSC_CLIENT_REASON_UNDEFINED:
		return RVSIP_PUB_CLIENT_REASON_UNDEFINED;
	
	default:
		/*If we didn't managed to conclude the reason from the transcClient reason
		  and from the response code, the reason is UNDEFNIED for regClient objects*/
		return RVSIP_PUB_CLIENT_REASON_UNDEFINED;
	}

}

/***************************************************************************
* PubClientUpdateRetryAfterIntervalIfNeeded
* ------------------------------------------------------------------------
* General: This function update the retryAfter interval of the pubClient object.
* Return Value:
* ------------------------------------------------------------------------
* Arguments:
* Input:   
*		pPubClient - Pointer to the pub-client object
*		responseCode - The response code the caused the state change.
* Output:  (-)
* Return:  The chosen reason of the pubClient
***************************************************************************/
static void PubClientUpdateRetryAfterIntervalIfNeeded(PubClient *pPubClient)
{
	RvSipMsgHandle		hRecievedMsg;
	RvStatus			rv;

	pPubClient->retryAfter = 0;
	rv = SipTranscClientGetReceivedMsg(&pPubClient->transcClient, &hRecievedMsg);
	if (rv != RV_OK)
	{
		return;
	}
	SipCommonGetRetryAfterFromMsg(hRecievedMsg, &pPubClient->retryAfter);
}

#endif /* RV_SIP_PRIMITIVES */

