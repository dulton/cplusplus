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
 *                              <RegClientTranscClientEv.c>
 *
 *  Handles state changed events received from the transaction layer.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2001
 *	  Gilad Govrin					Nov 2006
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "RegClientTranscClientEv.h"
#include "RegClientObject.h"
#include "RegClientCallbacks.h"
#include "RvSipContactHeader.h"
#include "RvSipMsg.h"
#include "RvSipAddress.h"
#include "_SipAuthenticator.h"

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

static RvSipRegClientState RegClientConvertTranscClientState(SipTranscClientState eNewState);
static RvSipRegClientStateChangeReason RegClientConvertTranscClientReason(SipTranscClientStateChangeReason eReason, RvInt16 responseCode);


/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/



/***************************************************************************
 * RegClientTranscClientEvStateChanged
 * ------------------------------------------------------------------------
 * General: Called when a state changes in the active transactions.
 *          The register-client will change state accordingly.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pOwner -       The owner of the transaction in which the "event
 *                         state changed" occurred.
 *            eNewState -    The new state of the transaction.
 *            eReason -      The reason for the state change.
 ***************************************************************************/
void RVCALLCONV RegClientTranscClientEvStateChanged(
                            IN SipTranscClientOwnerHandle          hRegClient,
                            IN SipTranscClientState                eNewState,
                            IN SipTranscClientStateChangeReason    eReason,
							IN RvInt16							   responseCode)
{
    RegClient						*pRegClient;
	RvSipRegClientState				eRegClientConvertedState;
	RvSipRegClientStateChangeReason eRegClientConvertedReason;

    if (NULL == hRegClient)
    {
        return;
    }
    pRegClient = (RegClient *)hRegClient;

	eRegClientConvertedState = RegClientConvertTranscClientState(eNewState);
	eRegClientConvertedReason = RegClientConvertTranscClientReason(eReason, responseCode);

	if (eRegClientConvertedState != RVSIP_REG_CLIENT_STATE_UNDEFINED)
	{
		RegClientChangeState(eRegClientConvertedState, eRegClientConvertedReason, pRegClient);
	}
	
}

/***************************************************************************
 * RegClientTranscClientEvMsgToSend
 * ------------------------------------------------------------------------
 * General: Called right before the transaction attempts to send a message
 * (not retransmission). Adds the list of Contact headers and the Expires
 * header that were set to the register-client to the message
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
RvStatus RVCALLCONV RegClientTranscClientEvMsgToSend(
                                       IN SipTranscClientOwnerHandle	hRegClient,
                                       IN RvSipMsgHandle				hMsgToSend)
{
    RegClient                *pRegClient = (RegClient *)hRegClient;
    RvStatus                 retStatus;

    /* Add the Contact headers associated with the register-client to the
       message before sending, Since the Register client has special handling of contact header
	   it is done here and not at the transcClient object.*/
    retStatus = RegClientAddContactsToMsg(pRegClient, hMsgToSend);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    
    /* Call event message to send of the application */
    retStatus = RegClientCallbackMsgToSendEv((RvSipRegClientHandle)pRegClient, hMsgToSend);

    return retStatus;
}


/***************************************************************************
 * RegClientTranscClientEvMsgReceived
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
RvStatus RVCALLCONV RegClientTranscClientEvMsgReceived (
                                  IN SipTranscClientOwnerHandle hRegClient,
                                  IN RvSipMsgHandle				hMsgReceived)
{
    RegClient                      *pRegClient;
    RvStatus                       retStatus = RV_OK;
    

    pRegClient = (RegClient *)hRegClient;

	RegClientSetAlertTimerIfNeeded(pRegClient, hMsgReceived);

    /* Call event message received of the application */
    retStatus = RegClientCallbackMsgReceivedEv ((RvSipRegClientHandle)pRegClient,
	                                              hMsgReceived);
    return retStatus;
}

/***************************************************************************
 * RegClientTranscClientEvOtherURLAddressFound
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
 *            hAddress       - Handle to unsupported address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV RegClientTranscClientEvOtherURLAddressFound(
                     IN  RvSipTranscHandle			hTransc,
                     IN  SipTranscClientOwnerHandle hRegClient,
                     IN  RvSipMsgHandle				hMsg,
                     IN  RvSipAddressHandle			hAddress,
                     OUT RvSipAddressHandle			hSipURLAddress,
                     OUT RvBool						*bAddressResolved)
{
    if (NULL == hRegClient || NULL == hAddress || NULL == hSipURLAddress)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RegClientCallbackOtherURLAddressFoundEv(hTransc, hRegClient, hMsg, hAddress,
                                             hSipURLAddress, bAddressResolved);

    return RV_OK;
}

/***************************************************************************
 * RegClientTranscClientFinalDestResolvedEv
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
 *            hRegClient   - The reg-client handle.
 *            hMsgToSend   - The handle to the outgoing message.
 ***************************************************************************/
RvStatus RVCALLCONV RegClientTranscClientFinalDestResolvedEv(
                            IN RvSipTranscHandle			hTransc,
                            IN SipTranscClientOwnerHandle	hRegClient,
                            IN RvSipMsgHandle				hMsgToSend)
{
    RvStatus                rv;
    RegClient			   *pRegClient = (RegClient*)hRegClient;

    if (NULL == hRegClient)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
    rv = RegClientCallbackFinalDestResolvedEv(hTransc,
                                                (SipTranscClientOwnerHandle)pRegClient,
                                                hMsgToSend);
    return rv;
}

/***************************************************************************
 * RegClientTranscClientNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that the reg-client is now using a new
 *          connection. The connection can be a totally new connection or
 *          a suitable connection that was found in the hash.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                         otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hRegClient     - RegClient handle
 *            hConn          - The new connection handle.
 *            bNewConnCreated - The connection is also a newly created connection
 *                               (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV RegClientTranscClientNewConnInUseEv(
                     IN  SipTranscClientOwnerHandle         hRegClient,
                     IN  RvSipTransportConnectionHandle		hConn,
                     IN  RvBool								bNewConnCreated)
{
    RegClient              *pRegClient   =  (RegClient*)hRegClient;
	
	return RegClientCallbackNewConnInUseEv(pRegClient,
			hConn,
			bNewConnCreated);
}

/***************************************************************************
 * RegClientTranscClientObjExpiredEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that the reg-client is about to expire
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient     - RegClient handle
 ***************************************************************************/
void RVCALLCONV RegClientTranscClientObjExpiredEv(
                     IN  SipTranscClientOwnerHandle         hRegClient)
{
    RegClient              *pRegClient   =  (RegClient*)hRegClient;
	
	RegClientCallbackObjExpired(pRegClient);
}

/***************************************************************************
 * RegClientTranscClientObjTerminated
 * ------------------------------------------------------------------------
 * General: Handle transcClient Termianted.
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient     - RegClient handle
 ***************************************************************************/
void RVCALLCONV RegClientTranscClientObjTerminated(
                     IN  SipTranscClientOwnerHandle         hRegClient)
{
    RegClient              *pRegClient   =  (RegClient*)hRegClient;
	
	RegClientDestruct(pRegClient);
}

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * RegClientTranscClientSigCompMsgNotRespondedEv
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
 *            hRegClient    - The reg-client handle.
 *            retransNo     - The number of retransmissions of the request
 *                            until now.
 *            ePrevMsgComp  - The type of the previous not responded request
 * Output:    peNextMsgComp - The type of the next request to retransmit (
 *                            RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                            application wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV RegClientTranscClientSigCompMsgNotRespondedEv(
                     IN  RvSipTranscHandle            hTransc,
                     IN  SipTranscClientOwnerHandle   hRegClient,
                     IN  RvInt32                      retransNo,
                     IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                     OUT RvSipTransmitterMsgCompType *peNextMsgComp)
{
    if (NULL == hRegClient || NULL == hTransc || NULL == peNextMsgComp)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    return RegClientCallbackSigCompMsgNotRespondedEv(hTransc,
                                                     (RvSipTranscOwnerHandle)hRegClient,
                                                     retransNo,
                                                     ePrevMsgComp,
                                                     peNextMsgComp);
}
#endif /* RV_SIGCOMP_ON */

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
* RegClientConvertTranscClientState
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
static RvSipRegClientState RegClientConvertTranscClientState(SipTranscClientState eNewState)
{
	switch(eNewState) {
	case SIP_TRANSC_CLIENT_STATE_ACTIVATING:
		return RVSIP_REG_CLIENT_STATE_REGISTERING;
	case SIP_TRANSC_CLIENT_STATE_ACTIVATED:
		return RVSIP_REG_CLIENT_STATE_REGISTERED;
	case SIP_TRANSC_CLIENT_STATE_FAILED:
		return RVSIP_REG_CLIENT_STATE_FAILED;
	case SIP_TRANSC_CLIENT_STATE_IDLE:
		return RVSIP_REG_CLIENT_STATE_IDLE;
	case SIP_TRANSC_CLIENT_STATE_MSG_SEND_FAILURE:
		return RVSIP_REG_CLIENT_STATE_MSG_SEND_FAILURE;
	case SIP_TRANSC_CLIENT_STATE_REDIRECTED:
		return RVSIP_REG_CLIENT_STATE_REDIRECTED;
	case SIP_TRANSC_CLIENT_STATE_TERMINATED:
		return RVSIP_REG_CLIENT_STATE_TERMINATED;
	case SIP_TRANSC_CLIENT_STATE_UNAUTHENTICATED:
		return RVSIP_REG_CLIENT_STATE_UNAUTHENTICATED;
	default:
		return RVSIP_REG_CLIENT_STATE_UNDEFINED;
	}
}

/***************************************************************************
* RegClientConvertTranscClientReason
* ------------------------------------------------------------------------
* General: This function response to return the correct reason according to the incoming
*		   new reason of the transaction-client object.
* Return Value:
* ------------------------------------------------------------------------
* Arguments:
* Input:   eReason - The new reason of the transc-client
* Output:  (-)
* Return:  The chosen reason of the regClient
***************************************************************************/
static RvSipRegClientStateChangeReason RegClientConvertTranscClientReason(SipTranscClientStateChangeReason eReason, RvInt16 responseCode)
{
	switch(eReason) {
	case SIP_TRANSC_CLIENT_REASON_RESPONSE_SUCCESSFUL_RECVD:
		return RVSIP_REG_CLIENT_REASON_RESPONSE_SUCCESSFUL_RECVD;

	case SIP_TRANSC_CLIENT_REASON_USER_REQUEST:
		return RVSIP_REG_CLIENT_REASON_USER_REQUEST;

	case SIP_TRANSC_CLIENT_REASON_RESPONSE_REQUEST_FAILURE_RECVD:
		return RVSIP_REG_CLIENT_REASON_RESPONSE_REQUEST_FAILURE_RECVD;

	case SIP_TRANSC_CLIENT_REASON_RESPONSE_SERVER_FAILURE_RECVD:
		return RVSIP_REG_CLIENT_REASON_RESPONSE_SERVER_FAILURE_RECVD;

	case SIP_TRANSC_CLIENT_REASON_RESPONSE_GLOBAL_FAILURE_RECVD:
		return RVSIP_REG_CLIENT_REASON_RESPONSE_GLOBAL_FAILURE_RECVD;

	case SIP_TRANSC_CLIENT_REASON_TRANSACTION_TIMEOUT:
		return RVSIP_REG_CLIENT_REASON_TRANSACTION_TIMEOUT;

	case SIP_TRANSC_CLIENT_REASON_LOCAL_FAILURE:
		return RVSIP_REG_CLIENT_REASON_LOCAL_FAILURE;
		
	case SIP_TRANSC_CLIENT_REASON_UNAUTHENTICATE:
		return RVSIP_REG_CLIENT_REASON_RESPONSE_UNAUTHENTICATED_RECVD;

	case SIP_TRANSC_CLIENT_REASON_NETWORK_ERROR:
		return RVSIP_REG_CLIENT_REASON_NETWORK_ERROR;

	case SIP_TRANSC_CLIENT_REASON_503_RECEIVED:
		return RVSIP_REG_CLIENT_REASON_503_RECEIVED;

	case SIP_TRANSC_CLIENT_REASON_UNDEFINED:
		/*If the transcClient notified about undefined reason we need to conclude
		  the reason from the response code since the reasons of the reg client is
		  wider then the transcClient layer.*/
		if((responseCode >= 300) && (responseCode < 400))
		{
			/*for 3xx response the reason from the transcClient is undefine since it
			  is changing the state to REDIRECTED and there is only one way to move
			  to this state, for regClient there is specific reason for this state*/
			return RVSIP_REG_CLIENT_REASON_RESPONSE_REDIRECTION_RECVD;
		}
		else if(responseCode == 407 || responseCode == 401)
		{
			/*if the response code is other then 401 or 407 the reason raises from
			  the transcClient layer is similar to one of the regClient reasons*/
			return RVSIP_REG_CLIENT_REASON_RESPONSE_UNAUTHENTICATED_RECVD;
		}
		return RVSIP_REG_CLIENT_REASON_UNDEFINED;
	
	default:
		/*If we didn't managed to conclude the reason from the transcClient reason
		  and from the response code, the reason is UNDEFNIED for regClient objects*/
		return RVSIP_REG_CLIENT_REASON_UNDEFINED;
	}
}

#endif /* RV_SIP_PRIMITIVES */










































