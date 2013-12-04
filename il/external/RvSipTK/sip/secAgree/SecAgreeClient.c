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
 *                              <SecAgreeClientObject.c>
 *
 * This file implement the client security-agreement behavior, as defined in RFC 3329.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2006
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "_SipSecAgree.h"
#include "SecAgreeTypes.h"
#include "SecAgreeState.h"
#include "SecAgreeStatus.h"
#include "SecAgreeObject.h"
#include "SecAgreeClient.h"
#include "SecAgreeCallbacks.h"
#include "SecAgreeUtils.h"
#include "SecAgreeMgrObject.h"
#include "SecAgreeMsgHandling.h"
#include "RvSipCommonList.h"
#include "RvSipTransmitter.h"
#include "_SipTransaction.h"
#include "_SipTransmitter.h"
#include "_SipTransport.h"
#include "RvSipSecurityHeader.h"
#include "_SipSecuritySecObj.h"

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV SecAgreeClientMsgRcvdReplaceObjectIfNeeded(
							 IN     SecAgreeMgr*               pSecAgreeMgr,
							 IN     RvSipMsgHandle             hMsg,
							 IN     RvSipSecAgreeOwner*        pOwner,
							 IN     SipTripleLock*			   pOwnerLock,
							 INOUT  SecAgree**                 ppSecAgreeClient,
							 OUT    RvSipHeaderListElemHandle* phMsgListElem,
							 OUT    RvSipSecurityHeaderHandle* phHeader);

static RvBool RVCALLCONV SecAgreeClientShouldInsertClientListToMsg(
									IN     SecAgree*               pSecAgree);

static void RVCALLCONV SecAgreeClientListInsertedToMsg(
										IN  SecAgree*      pSecAgree);

static RvBool RVCALLCONV CanMsgBePartOfNegotiation(IN     RvSipMsgHandle      hMsg);

static void RVCALLCONV GetIpsecParamsIndex(IN    RvSipSecurityHeaderHandle   hSecurityHeader,
										   OUT   RvInt32                    *pSpecificParameter,
										   OUT   RvBool                     *pbIsSupported);

static RvBool RVCALLCONV IsProvisionalResponse(IN     RvSipMsgHandle      hMsg);

static void RVCALLCONV SetChosenSecurity(
							 IN  SecAgree*                       pSecAgreeClient,
							 IN  RvSipSecurityMechanismType      eSecurityType,
							 IN  RvSipSecurityHeaderHandle       hLocalSecurity,
							 IN  RvSipSecurityHeaderHandle       hRemoteSecurity);

static RvStatus RVCALLCONV UseDefaultSecurityMechanism(
                                      IN   SecAgree*                   pSecAgree,
									  OUT  RvSipSecurityMechanismType* peSecurityType,
									  OUT  RvSipSecurityHeaderHandle*  phLocalSecurity,
									  OUT  RvSipSecurityHeaderHandle*  phRemoteSecurity,
									  OUT  RvBool*                     pbIsValid);

static RvStatus RVCALLCONV SetApplicationSecurityMechanism(
                                      IN   SecAgree*                  pSecAgree,
									  IN   RvSipSecurityMechanismType eSecurityType,
									  OUT  RvSipSecurityHeaderHandle* phLocalSecurity,
									  OUT  RvSipSecurityHeaderHandle* phRemoteSecurity,
									  OUT  RvBool*                    pbIsValid);

static RvInt RVCALLCONV SecurityMechanismEnumToIndex(
							 IN  RvSipSecurityMechanismType  eMechanism);

static RvBool RVCALLCONV CanActivateSecObj(
							 IN  SecAgree*                   pSecAgree,
							 IN  RvSipSecurityMechanismType  eMechanism);

/*-----------------------------------------------------------------------*/
/*                         SECURITY CLIENT FUNCTIONS                     */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeClientInit
 * ------------------------------------------------------------------------
 * General: Used in states IDLE and CLIENT_REMOTE_LIST_READY,
 *          to indicate that the application has completed to initialize the local
 *          security list; and local digest list for server security-agreement. Once
 *          called, the application can no longer modify the local security information.
 *          Calling RvSipSecAgreeInit() modifies the state of the security-agreement
 *          object to allow proceeding in the security-agreement process. It will also
 *          cause a client security-agreement object to choose the security-mechanism
 *          to use. Calling this function will invoke the creation and initialization of
 *          a security object according to the supplied parameters.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient   - Pointer to the client security-agreement object
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeClientInit(IN   SecAgree*    pSecAgreeClient)
{
	RvStatus						rv;

	/* Initiate the security object */
	rv = SecAgreeObjectUpdateSecObj(pSecAgreeClient, SEC_AGREE_PROCESSING_INIT);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
				   "SecAgreeClientInit - Client security-agreement=0x%p: Failed to init security object, rv=%d",
				   pSecAgreeClient, rv));
		return rv;
	}

	/* Modify state due to init */
	rv = SecAgreeStateProgress(pSecAgreeClient,
						  SEC_AGREE_PROCESSING_INIT,
						  NULL, /* irrelevant */
						  RVSIP_TRANSPORT_UNDEFINED, /* irrelevant */
						  RV_FALSE, /* irrelevant */
						  RV_FALSE, /* irrelevant */
						  RV_FALSE /* irrelevant */);

	return rv;
}

/***************************************************************************
 * SecAgreeClientHandleMsgToSend
 * ------------------------------------------------------------------------
 * General: Processes a message about to be sent: adds security information
 *          and updates the client security-agreement object accordingly.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient - Pointer to the client security-agreement
 *         hSecObj         - Handle to the security object of the SIP object
 *                           sending the message
 *         hMsg            - Handle to the message
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeClientHandleMsgToSend(
							 IN  SecAgree                      *pSecAgreeClient,
							 IN  RvSipSecObjHandle              hSecObj,
							 IN  RvSipMsgHandle                 hMsg)
{
    RvStatus                rv = RV_OK;
	RvBool                  bSecAgreeInfoInserted = RV_FALSE;

	if (CanMsgBePartOfNegotiation(hMsg) == RV_FALSE)
	{
		/* This message does not require security-agreement processing */
		return RV_OK;
	}

	RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
			  "SecAgreeClientHandleMsgToSend - Client security-agreement 0x%p handling message to send (message=0x%p)",
			  pSecAgreeClient, hMsg));
	
	/* print warning to the log if the security-agreement is invalid. */
	if (SecAgreeStateIsValid(pSecAgreeClient) == RV_FALSE)
	{
		RvLogWarning(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
					 "SecAgreeClientHandleMsgToSend - Client security-agreement=0x%p sending message=0x%p in state %s",
					 pSecAgreeClient, hMsg, SecAgreeUtilsStateEnum2String(pSecAgreeClient->eState)));
		return RV_OK;
	}

	/* check if the state requires processing message to send */
	if (SecAgreeStateShouldProcessMsgToSend(pSecAgreeClient) == RV_FALSE)
	{
		RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
					"SecAgreeClientHandleMsgToSend - Client security-agreement=0x%p nothing to do in state %s (message=0x%p)",
					pSecAgreeClient, SecAgreeUtilsStateEnum2String(pSecAgreeClient->eState), hMsg));
		return RV_OK;
	}

	/* Check whether Security-Client list should be inserted to the message */
	if (SecAgreeClientShouldInsertClientListToMsg(pSecAgreeClient) == RV_TRUE)
	{
		/* The outgoing message will contain a Security-Client list  */
		rv = SecAgreeMsgHandlingClientSetSecurityClientListToMsg(pSecAgreeClient, hMsg);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
					   "SecAgreeClientHandleMsgToSend - client security-agreement=0x%p: Failed to set Security-Client list to message=0x%p, rv=%d",
						pSecAgreeClient, hMsg, rv));
			return rv;
		}
		bSecAgreeInfoInserted = RV_TRUE;
		/* The transaction on which the security-client list was set, is the transaction from which
	       the security-server list should be loaded */
		pSecAgreeClient->securityData.cseqStep = SecAgreeUtilsGetCSeqStep(hMsg);
	}

	/* Check whether security information should be inserted to the message */
	if (SecAgreeStateIsValid(pSecAgreeClient) == RV_TRUE)
	{
		/* Add Security-Verify list if exists */
		rv = SecAgreeMsgHandlingClientSetSecurityVerifyListToMsg(pSecAgreeClient, hMsg);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
						"SecAgreeClientHandleMsgToSend - client security-agreement=0x%p: Failed to set security-verify list to message=0x%p, rv=%d",
						pSecAgreeClient, hMsg, rv));
			return rv;
		}

		/* Add Authorization information if exists */
		rv = SecAgreeMsgHandlingClientSetAuthorizationDataToMsg(pSecAgreeClient, hMsg);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
						"SecAgreeClientHandleMsgToSend - client security-agreement=0x%p: Failed to set authorization data to message=0x%p, rv=%d",
						pSecAgreeClient, hMsg, rv));
			return rv;
		}

		bSecAgreeInfoInserted = RV_TRUE;
	}

	if (bSecAgreeInfoInserted == RV_TRUE)
	{
		/* Security lists were inserted to the message. We insert Require and Proxy-Require with sec-agree */
		rv = SecAgreeMsgHandlingSetRequireHeadersToMsg(pSecAgreeClient, hMsg);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
						"SecAgreeClientHandleMsgToSend - client security-agreement=0x%p: Failed to set Require and Proxy-Require headers to message=0x%p, rv=%d",
						pSecAgreeClient, hMsg, rv));
			return rv;
		}
	}

	/* Update security-agreement object due to sending of a message */
	SecAgreeClientListInsertedToMsg(pSecAgreeClient);

	/* Move to the next state if needed */
	rv = SecAgreeStateProgress(pSecAgreeClient,
						  SEC_AGREE_PROCESSING_MSG_TO_SEND,
						  hSecObj,
						  RVSIP_TRANSPORT_UNDEFINED /*irrelevant*/ ,
						  RV_FALSE, /* irrelevant */
						  RV_FALSE, /* irrelevant */
						  RV_FALSE /* irrelevant */);

	if (rv != RV_OK)
	{
		RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
						"SecAgreeClientHandleMsgToSend - client security-agreement=0x%p: Failed to change state, rv=%d",
						pSecAgreeClient, rv));
		return rv;
	}

	/* Check the chosen security in state ACTIVE */
	if (SecAgreeStateIsActive(pSecAgreeClient) == RV_TRUE)
	{
		if (pSecAgreeClient->securityData.selectedSecurity.eType != RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED)
		{
			RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
					  "SecAgreeClientHandleMsgToSend - Active client security-agreement=0x%p: chosen mechanism %s",
					  pSecAgreeClient, SecAgreeUtilsMechanismEnum2String(pSecAgreeClient->securityData.selectedSecurity.eType)));
		}
		else /* RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED */
		{
			RvLogExcep(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
					   "SecAgreeClientHandleMsgToSend - Active client security-agreement=0x%p (hMsg=0x%p) does not have chosen security mechanism",
					   pSecAgreeClient, hMsg));
		}
	}

	return rv;
}

/***************************************************************************
 * SecAgreeClientHandleDestResolved
 * ------------------------------------------------------------------------
 * General: Performs last state changes and message changes based on the
 *          transport type that is obtained via destination resolution.
 *          If this is TLS on initial state, we abort negotiation attempt by
 *          removing all negotiation info from the message, and changing state
 *          to NO_AGREEMENT_TLS. If this is not TLS we change state to
 *          CLIENT_LOCAL_LIST_SENT.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient - Pointer to the client security-agreement
 *         hMsg            - Handle to the message
 *         hTrx            - The transmitter that notified destination resolution
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeClientHandleDestResolved(
							 IN  SecAgree*                     pSecAgreeClient,
							 IN  RvSipMsgHandle                hMsg,
							 IN  RvSipTransmitterHandle        hTrx)
{
	RvSipTransportAddr         remoteAddr;
	RvSipTransmitterExtOptions extOptions;
	RvInt                      tlsAddrType   = 1;
	RvInt                      ipsecAddrType = 1;
	RvStatus                   rv            = RV_OK;

	if (CanMsgBePartOfNegotiation(hMsg) == RV_FALSE)
	{
		/* This message does not require security-agreement processing */
		return RV_OK;
	}

	RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
			  "SecAgreeClientHandleDestResolved - Client security-agreement 0x%p handling dest resolved (message=0x%p)",
			  pSecAgreeClient, hMsg));

	/* Check whether processing is required in dest resolved */
	if (SecAgreeStateShouldProcessDestResolved(pSecAgreeClient) == RV_FALSE)
	{
		RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
					"SecAgreeClientHandleDestResolved - Client security-agreement=0x%p nothing to do in state %s (message=0x%p)",
					pSecAgreeClient, SecAgreeUtilsStateEnum2String(pSecAgreeClient->eState), hMsg));
		return RV_OK;
	}

	RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
              "SecAgreeClientHandleDestResolved - Client security-agreement=0x%p processing dest resolved of message=0x%p",
              pSecAgreeClient, hMsg));

	/* Get the destination transport type from the transmitter */
	rv = RvSipTransmitterGetDestAddress(hTrx,
										sizeof(RvSipTransportAddr),
										sizeof(RvSipTransmitterExtOptions),
										&remoteAddr,
										&extOptions);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
					"SecAgreeClientHandleDestResolved - client security-agreement=0x%p: Failed to get transport type from transmitter=0x%p, rv=%d",
					pSecAgreeClient, hTrx, rv));
		return rv;
	}

#if (RV_IMS_IPSEC_ENABLED == RV_YES)
	ipsecAddrType = pSecAgreeClient->addresses.remoteIpsecAddr.addrtype;
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */
#if (RV_TLS_TYPE != RV_TLS_NONE)
	tlsAddrType   = pSecAgreeClient->addresses.remoteTlsAddr.addrtype;
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */

#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
	/* Check if there is already remote address set to the security-agreement */
	if ((remoteAddr.eTransportType == RVSIP_TRANSPORT_TLS && 
		 tlsAddrType == 0) ||
		((remoteAddr.eTransportType == RVSIP_TRANSPORT_UDP || remoteAddr.eTransportType == RVSIP_TRANSPORT_TCP) && 
		 ipsecAddrType == 0))
	{
		/* There is no remote address set to the security-agreement. Set the address that was 
		   resolved here to be its remote address */
		rv = SecAgreeObjectSetRemoteAddr(pSecAgreeClient, &remoteAddr);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
						"SecAgreeClientHandleDestResolved - client security-agreement=0x%p: Failed to set remote address(rv=%d:%s)",
						pSecAgreeClient, rv, SipCommonStatus2Str(rv)));
			return rv;
		}
	}
#else /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */
	RV_UNUSED_ARG(tlsAddrType);
	RV_UNUSED_ARG(ipsecAddrType);
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */
	
	/* Check if the transport is not TLS */
	if (remoteAddr.eTransportType == RVSIP_TRANSPORT_TLS)
	{
		RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
				  "SecAgreeClientHandleDestResolved - Client security-agreement=0x%p dest resolved to TLS, clear message=0x%p from security-agreement info",
				  pSecAgreeClient, hMsg));

		/* Since tls is the chosen security, we remove all security information we inserted the
		message before the transport resolution. */
		rv = SecAgreeMsgHandlingClientCleanMsg(pSecAgreeClient, hMsg);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
						"SecAgreeClientHandleDestResolved - client security-agreement=0x%p: Failed to clean messsage 0x%p, rv=%d",
						pSecAgreeClient, hMsg, rv));
			return rv;
		}
	}

	/* Move to the next state if needed */
	rv = SecAgreeStateProgress(pSecAgreeClient,
						  SEC_AGREE_PROCESSING_DEST_RESOLVED,
						  NULL, /* irrelevant */
						  remoteAddr.eTransportType,
						  RV_FALSE, /* irrelevant */
						  RV_FALSE, /* irrelevant */
						  RV_FALSE /* irrelevant */);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
					"SecAgreeClientHandleDestResolved - client security-agreement=0x%p: Failed to change state., rv=%d",
					pSecAgreeClient, rv));
		return rv;
	}

	/* Notify the security-object on the destination resolution */
	rv = SecAgreeObjectUpdateSecObj(pSecAgreeClient, SEC_AGREE_PROCESSING_DEST_RESOLVED);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
					"SecAgreeClientHandleDestResolved - client security-agreement=0x%p: Failed to notify security-object 0x%p on dest resolved, rv=%d",
					pSecAgreeClient, pSecAgreeClient->securityData.hSecObj, rv));
		return rv;
	}

	return RV_OK;
}

/***************************************************************************
 * SecAgreeClientHandleMsgSendFailure
 * ------------------------------------------------------------------------
 * General: Invalidates a security-agreement whenever a message send failure
 *          occurs.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient - Pointer to the client security-agreement
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeClientHandleMsgSendFailure(
							 IN  SecAgree                      *pSecAgreeClient)
{
	if (RV_FALSE == SecAgreeStateIsValid(pSecAgreeClient))
	{
		/* Nothing to do in invalid state */
		RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
					"SecAgreeClientHandleMsgSendFailure - Client security-agreement=0x%p: Message send failure is ignored in state %s",
					pSecAgreeClient, SecAgreeUtilsStateEnum2String(pSecAgreeClient->eState)));
		return RV_OK;
	}

	RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
			  "SecAgreeClientHandleMsgSendFailure - Client security-agreement 0x%p handling message send failure",
			  pSecAgreeClient));

	/* change state to INVALID */
	SecAgreeStateInvalidate(pSecAgreeClient, RVSIP_SEC_AGREE_REASON_MSG_SEND_FAILURE);

	return RV_OK;
}

/***************************************************************************
 * SecAgreeClientHandleMsgRcvd
 * ------------------------------------------------------------------------
 * General: Processes a received message : loads and stores security information,
 *          and change client security-agreement state accordingly
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr     - Pointer to the Security-Mgr
 *         pOwner           - Owner details to supply to the application
 *         pOwnerLock       - The triple lock of the owner, to open before callback
 *         hMsg             - Handle to the message
 * Inout:  ppSecAgreeClient - Pointer to the security-agreement object. If the
 *                            security-agreement object is replaced, the new
 *                            security-agreement is updated here
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeClientHandleMsgRcvd(
							 IN    SecAgreeMgr*                  pSecAgreeMgr,
							 IN    RvSipSecAgreeOwner*           pOwner,
							 IN    SipTripleLock*				 pOwnerLock,
							 IN    RvSipMsgHandle                hMsg,
							 INOUT SecAgree**                    ppSecAgreeClient)
{
	SecAgree*                 pSecAgreeClient    = (SecAgree*)*ppSecAgreeClient;
	RvBool                    bHasSecurityServer = RV_FALSE;
	RvSipHeaderListElemHandle hMsgListElem       = NULL;
    RvSipSecurityHeaderHandle hHeader            = NULL;
	RvBool                    bIsDigest          = RV_FALSE;
	RvStatus                  rv                 = RV_OK;
	RvUint32                  cseqStep           = SecAgreeUtilsGetCSeqStep(hMsg);

	if (CanMsgBePartOfNegotiation(hMsg) == RV_FALSE)
	{
		/* This message does not require security-agreement processing */
		if (pSecAgreeClient != NULL && 
			(cseqStep == pSecAgreeClient->securityData.cseqStep || pSecAgreeClient->securityData.cseqStep == 0) &&
			SecAgreeStateClientShouldLoadRemoteSecList(pSecAgreeClient) == RV_TRUE &&
			IsProvisionalResponse(hMsg) == RV_FALSE)
		{
			/* The security-agreement is expecting a Security-Server list on a 4xx response.
			   It received final non 4xx response hence changes state to INVALID */
			RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
						"SecAgreeClientHandleMsgRcvd - client security-agreement=0x%p: Received response other than 4xx when expecting Security-Server list. State changes to INVALID",
						pSecAgreeClient));
			SecAgreeStateInvalidate(pSecAgreeClient, RVSIP_SEC_AGREE_REASON_RESPONSE_MISSING);
		}
		else if (pSecAgreeClient != NULL)
		{
			/* The status of this security-agreement might change due to this received message */
			rv = SecAgreeStatusConsiderNewStatus(pSecAgreeClient, 
												 SEC_AGREE_PROCESSING_MSG_RCVD, 
												 RvSipMsgGetStatusCode(hMsg));
		}

		return rv;
	}

	/* check if the state requires processing message received */
	if (SecAgreeStateShouldProcessRcvdMsg(pSecAgreeClient) == RV_FALSE)
	{
		if (NULL != pSecAgreeClient)
		{
			RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeClientHandleMsgRcvd - Client security-agreement=0x%p nothing to do in state %s (message=0x%p)",
					pSecAgreeClient, SecAgreeUtilsStateEnum2String(pSecAgreeClient->eState), hMsg));
		}
		return RV_OK;
	}

	/* Before we load any information, we check that we have a valid client security-agreement object
	   to do so. If not, we request the application to supply one */
	rv = SecAgreeClientMsgRcvdReplaceObjectIfNeeded(pSecAgreeMgr, hMsg, pOwner, pOwnerLock, ppSecAgreeClient, &hMsgListElem, &hHeader);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				   "SecAgreeClientHandleMsgRcvd - client security-agreement=0x%p: Failed to assert object validity, rv=%d",
				   pSecAgreeClient, rv));
		return rv;
	}

	pSecAgreeClient = *ppSecAgreeClient;

	/* The application does not wish to initiate or participate in a security-agreement */
	if (NULL == pSecAgreeClient)
	{
		return RV_OK;
	}

	RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
			  "SecAgreeClientHandleMsgRcvd - Client security-agreement 0x%p handling message received (owner=0x%p, message=0x%p)",
			  pSecAgreeClient, pOwner, hMsg));

	/* Check if there are Security-Server headers in the message and copy them into tempRemoteList */
	rv = SecAgreeMsgHandlingClientLoadSecurityServerListFromMsg(pSecAgreeClient, hMsg, hMsgListElem, hHeader, &bHasSecurityServer, &bIsDigest);
	if (RV_OK != rv)
	{
		/* Invalidate the security-agreement due to failure */
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				   "SecAgreeClientHandleMsgRcvd - client security-agreement=0x%p: Failed to get security-server header from message=0x%p, rv=%d",
				   pSecAgreeClient, hMsg, rv));
		SecAgreeStateInvalidate(pSecAgreeClient, RVSIP_SEC_AGREE_REASON_OUT_OF_RESOURCES);
		return rv;
	}

	if (SecAgreeStateClientShouldLoadRemoteSecList(pSecAgreeClient) == RV_TRUE &&
		(cseqStep == pSecAgreeClient->securityData.cseqStep || pSecAgreeClient->securityData.cseqStep == 0) &&
		bHasSecurityServer == RV_FALSE)
	{
		/* There is no security-server list in the message. The client security-agreement is invalidated */
		RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
					"SecAgreeClientHandleMsgRcvd - client security-agreement=0x%p: Received response with no Security-Server list. State changes to INVALID",
					pSecAgreeClient));
		SecAgreeStateInvalidate(pSecAgreeClient, RVSIP_SEC_AGREE_REASON_RESPONSE_MISSING);
		return RV_OK;
	}

	if (bHasSecurityServer == RV_TRUE)
	{
		/* There were Security-Server headers in the message - the server requests for an agreement */

		/* Load additional security keys from the message */
		if (bIsDigest == RV_TRUE)
		{
			rv = SecAgreeMsgHandlingClientLoadAuthenticationDataFromMsg(pSecAgreeClient, hMsg);
			if (RV_OK != rv)
			{
				/* Invalidate the security-agreement due to failure */
				RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
							"SecAgreeClientHandleMsgRcvd - client security-agreement=0x%p: Failed to get authentication info from message=0x%p, rv=%d",
							pSecAgreeClient, hMsg, rv));
				SecAgreeStateInvalidate(pSecAgreeClient, RVSIP_SEC_AGREE_REASON_OUT_OF_RESOURCES);
				return rv;
			}
		}
	}

	/* Move to the next state of this client security-agreement object */
	rv = SecAgreeStateProgress(pSecAgreeClient,
						  SEC_AGREE_PROCESSING_MSG_RCVD,
	  					  NULL, /* irrelevant */
						  RVSIP_TRANSPORT_UNDEFINED /*irrelevant*/,
						  bHasSecurityServer,
						  RV_FALSE, /* irrelevant */
						  RV_FALSE /* irrelevant */);

	return rv;
}

/***************************************************************************
 * SecAgreeClientChooseMechanism
 * ------------------------------------------------------------------------
 * General: Compares the local and remote lists and chooses the best security
 *          mechanism
 *          Notice: we choose from the list of supported mechanisms only.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgreeClient        - Pointer to the client security-agreement object
 * Output:    peSelectedMechanism    - Enumeration of the selected mechanism
 *            phSelectedLocalHeader  - Pointer to the selected local security header
 *            phSelectedRemoteHeader - Pointer to the selected local security header
 *            pbIsValid              - RV_TRUE if the chosen security is valid
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeClientChooseMechanism(
											IN  SecAgree*                      pSecAgreeClient,
											OUT RvSipSecurityMechanismType*    peSelectedMechanism,
											OUT RvSipSecurityHeaderHandle*     phSelectedLocalHeader,
											OUT RvSipSecurityHeaderHandle*     phSelectedRemoteHeader,
											OUT RvBool*                        pbIsValid)
{
#define MAX_NUM_OF_SUPPORTED_MECHANISMS      16 /* see RvSipSecurityMechanismType. Safety margin taken */
#define MAX_NUM_OF_IPSEC_PARAMETERS          32 /* see RvSipSecurityAlgorithmType*RvSipSecurityEncryptAlgorithmType. Safety margin taken */
	RvSipCommonListHandle      hOptionsList;
	RvSipCommonListHandle      hPreferencedList;
	RvSipCommonListElemHandle  hListElem;
	RvInt                      elementType;
	RvSipSecurityHeaderHandle  hBestLocalHeader  = NULL;
	RvSipSecurityHeaderHandle  hBestRemoteHeader = NULL;
	RvInt32                    bestQIntPart      = 0;
	RvInt32                    bestQDecPart      = 0;
	RvSipSecurityHeaderHandle  hHeader;
	RvSipSecurityHeaderHandle  pPreferedHeaders[MAX_NUM_OF_SUPPORTED_MECHANISMS];
	RvInt32                    pBestQIntPart[MAX_NUM_OF_SUPPORTED_MECHANISMS];
	RvInt32                    pBestQDecPart[MAX_NUM_OF_SUPPORTED_MECHANISMS];
	RvSipSecurityHeaderHandle  pPreferedIpsecHeaders[MAX_NUM_OF_IPSEC_PARAMETERS];
	RvInt32                    pBestIpsecQIntPart[MAX_NUM_OF_IPSEC_PARAMETERS];
	RvInt32                    pBestIpsecQDecPart[MAX_NUM_OF_IPSEC_PARAMETERS];
	RvSipSecurityHeaderHandle* pCurrHeader;
	RvInt32*                   pCurrQIntPart;
	RvInt32*                   pCurrQDecPart;
	RvSipSecurityMechanismType eSecurityType     = RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED;
	RvSipSecurityMechanismType eBestMechanism    = RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED;
	RvInt                      index;
	RvInt                      safeCounter       = 0;
	RvStatus                   rv;

	RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
			  "SecAgreeClientChooseMechanism - Client security-agreement=0x%p choosing best security mechanism",
			  pSecAgreeClient));

	/* The remote list contains the server's preferences. We compare it to the local list to
	   find the best match */
	hOptionsList     = pSecAgreeClient->securityData.hLocalSecurityList;
	hPreferencedList = pSecAgreeClient->securityData.hRemoteSecurityList;

	*pbIsValid = RV_TRUE;

	if (NULL == hOptionsList || NULL == hPreferencedList)
	{
		return RV_ERROR_NOT_FOUND;
	}

	for (index = 0; index < MAX_NUM_OF_SUPPORTED_MECHANISMS; index++)
	{
		pPreferedHeaders[index] = NULL;
		pBestQIntPart[index]    = UNDEFINED;
		pBestQDecPart[index]    = UNDEFINED;
	}
	for (index = 0; index < MAX_NUM_OF_IPSEC_PARAMETERS; index++)
	{
		pPreferedIpsecHeaders[index] = NULL;
		pBestIpsecQIntPart[index]    = UNDEFINED;
		pBestIpsecQDecPart[index]    = UNDEFINED;
	}

	/* Go over the remote list. Choose the most preferable header for each type and place it
	in pPreferedHeaders (using its type enumeration as its index) */
	rv = RvSipCommonListGetElem(hPreferencedList, RVSIP_FIRST_ELEMENT, NULL, &elementType, (void**)&hHeader, &hListElem);
	while (RV_OK == rv && hListElem != NULL && safeCounter < SEC_AGREE_MAX_LOOP(pSecAgree))
	{
		eSecurityType = RvSipSecurityHeaderGetMechanismType(hHeader);
		index = SecurityMechanismEnumToIndex(eSecurityType);
		if (RV_TRUE == SecAgreeUtilsIsSupportedMechanism(eSecurityType) && index != -1)
		{
			/* If the next header has a higher preference than the header in index eSecurityType, replace them */

			if (eSecurityType != RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP)
			{
				/* Use the main headers array */
				pCurrHeader   = &pPreferedHeaders[index];
				pCurrQIntPart = &pBestQIntPart[index];
				pCurrQDecPart = &pBestQDecPart[index];
			}
			else
			{
				RvInt32  ipsecIndex;
				RvBool   bIsSupported;

				GetIpsecParamsIndex(hHeader, &ipsecIndex, &bIsSupported);

				if (bIsSupported == RV_FALSE)
				{
					/* The algorithm of this ipsec header is not supported by the sip stack. continue to next header */
					rv = RvSipCommonListGetElem(hPreferencedList, RVSIP_NEXT_ELEMENT, hListElem, &elementType, (void**)&hHeader, &hListElem);
					safeCounter += 1;
					continue;
				}

				/* This is ipsec - use the specific ipsec array */
				pCurrHeader   = &pPreferedIpsecHeaders[ipsecIndex];
				pCurrQIntPart = &pBestIpsecQIntPart[ipsecIndex];
				pCurrQDecPart = &pBestIpsecQDecPart[ipsecIndex];
			}

			if (*pCurrHeader == NULL)
			{
				*pCurrHeader = hHeader;
				rv = SecAgreeUtilsGetQVal(hHeader, pCurrQIntPart, pCurrQDecPart);
				if (RV_OK != rv)
				{
					break;
				}

			}
			else
			{
				rv = SecAgreeUtilsGetBetterHeader(*pCurrHeader, hHeader,
												  pCurrQIntPart, pCurrQDecPart, NULL, NULL,
												  pCurrHeader, pCurrQIntPart, pCurrQDecPart);
				if (RV_OK != rv)
				{
					break;
				}
			}
		}
		else if (RV_TRUE == SecAgreeUtilsIsSupportedMechanism(eSecurityType))
		{
			RvLogExcep(pSecAgreeClient->pSecAgreeMgr->pLogSrc, (pSecAgreeClient->pSecAgreeMgr->pLogSrc,
				       "SecAgreeClientChooseMechanism - client security-agreement 0x%p: Array too small",
				       pSecAgreeClient));
		}

		rv = RvSipCommonListGetElem(hPreferencedList, RVSIP_NEXT_ELEMENT, hListElem, &elementType, (void**)&hHeader, &hListElem);

		safeCounter += 1;
	}
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc, (pSecAgreeClient->pSecAgreeMgr->pLogSrc,
				   "SecAgreeClientChooseMechanism - client security-agreement 0x%p: Failed to go over remote security list, rv=%d",
				   pSecAgreeClient, rv));
		return rv;
	}

	/* Go over the options list and match it to the preferable list. Find the best header possible */
	rv = RvSipCommonListGetElem(hOptionsList, RVSIP_FIRST_ELEMENT, NULL, &elementType, (void**)&hHeader, &hListElem);
	while (RV_OK == rv && hListElem != NULL && safeCounter < 2*SEC_AGREE_MAX_LOOP(pSecAgree))
	{
		/* Compare this header to the best of its type from the remote list in pPreferedHeaders */
		eSecurityType = RvSipSecurityHeaderGetMechanismType(hHeader);
		index = SecurityMechanismEnumToIndex(eSecurityType);
		
		if (eSecurityType != RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP && index != -1)
		{
			/* Use the main headers array */
			pCurrHeader   = &pPreferedHeaders[index];
			pCurrQIntPart = &pBestQIntPart[index];
			pCurrQDecPart = &pBestQDecPart[index];
		}
		else
		{
			RvInt32  ipsecIndex;
			RvBool   bIsSupported;

			GetIpsecParamsIndex(hHeader, &ipsecIndex, &bIsSupported);

			if (bIsSupported == RV_FALSE)
			{
				/* The algorithm of this ipsec header is not supported by the sip stack. continue to next header */
				rv = RvSipCommonListGetElem(hPreferencedList, RVSIP_NEXT_ELEMENT, hListElem, &elementType, (void**)&hHeader, &hListElem);
				safeCounter += 1;
				continue;
			}

			/* This is ipsec - use the specific ipsec array */
			pCurrHeader   = &pPreferedIpsecHeaders[ipsecIndex];
			pCurrQIntPart = &pBestIpsecQIntPart[ipsecIndex];
			pCurrQDecPart = &pBestIpsecQDecPart[ipsecIndex];
		}

		if (*pCurrHeader != NULL)
		{
			if (NULL == hBestLocalHeader)
			{
				/* This is the first match found. It is considered to be the best for now */
				hBestLocalHeader  = hHeader;
				eBestMechanism    = eSecurityType;
				hBestRemoteHeader = *pCurrHeader;
				bestQIntPart      = *pCurrQIntPart;
				bestQDecPart      = *pCurrQDecPart;
			}
			else if (RV_TRUE == SecAgreeUtilsIsLargerQ(bestQIntPart, bestQDecPart, *pCurrQIntPart, *pCurrQDecPart))
			{
				/* We found a header better than the current best. Update the best header. */
				hBestLocalHeader  = hHeader;
				eBestMechanism    = eSecurityType;
				hBestRemoteHeader = *pCurrHeader;
				bestQIntPart      = *pCurrQIntPart;
				bestQDecPart      = *pCurrQDecPart;
			}
		}

		rv = RvSipCommonListGetElem(hOptionsList, RVSIP_NEXT_ELEMENT, hListElem, &elementType, (void**)&hHeader, &hListElem);

		safeCounter += 1;
	}
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc, (pSecAgreeClient->pSecAgreeMgr->pLogSrc,
				   "SecAgreeClientChooseMechanism - client security-agreement 0x%p: Failed to go over local security list, rv=%d",
				   pSecAgreeClient, rv));
		return rv;
	}

	*peSelectedMechanism     = eBestMechanism;
	*phSelectedLocalHeader   = hBestLocalHeader;
	*phSelectedRemoteHeader  = hBestRemoteHeader;

	rv = SecAgreeClientCheckSecurity(pSecAgreeClient, eBestMechanism, pbIsValid);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc, (pSecAgreeClient->pSecAgreeMgr->pLogSrc,
				   "SecAgreeClientChooseMechanism - client security-agreement 0x%p: Failed to verify the mechanism chosen, rv=%d",
				   pSecAgreeClient, rv));
		return rv;
	}

	RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
			  "SecAgreeClientChooseMechanism - Client security-agreement=0x%p best security mechanism chosen = %s",
			  pSecAgreeClient, SecAgreeUtilsMechanismEnum2String(eBestMechanism)));

	return RV_OK;
}

/***************************************************************************
 * SecAgreeClientCheckSecurity
 * ------------------------------------------------------------------------
 * General: Checks that the chosen security is valid, and that the client has
 *          all the information it needs in order to apply this mechanism
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient    - Pointer to the client security-agreement
 *         eSecurityType      - The type of the chosen security mechanism
 * Output: pbIsValid          - RV_TRUE if the chosen security is valid,
 *                              RV_FALSE otherwise
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeClientCheckSecurity(
							 IN  SecAgree*                  pSecAgreeClient,
							 IN  RvSipSecurityMechanismType eSecurityType,
							 OUT RvBool*					pbIsValid)
{
	RvStatus rv;

	*pbIsValid = RV_TRUE;

	if (eSecurityType == RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED)
	{
		/* no security mechanism chosen */
		RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
				  "SecAgreeClientCheckSecurity - client security-agreement=0x%p: There is no matching security mechanism",
				  pSecAgreeClient));
		*pbIsValid = RV_FALSE;
	}
	else if (eSecurityType == RVSIP_SECURITY_MECHANISM_TYPE_DIGEST)
	{
		/* there was no authentication data in the message even though the security chosen is digest */
		if (pSecAgreeClient->securityData.additionalKeys.authInfo.hClientAuth.hAuthList == NULL)
		{
			RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
					   "SecAgreeClientCheckSecurity - client security-agreement=0x%p: There is no valid Proxy-Authenticate header in the response",
					   pSecAgreeClient));
			*pbIsValid = RV_FALSE;
		}
		else
		{
			rv = SipAuthenticatorCheckValidityOfAuthList(
												   pSecAgreeClient->pSecAgreeMgr->hAuthenticator,
												   pSecAgreeClient->securityData.additionalKeys.authInfo.hClientAuth.pAuthListInfo,
												   pSecAgreeClient->securityData.additionalKeys.authInfo.hClientAuth.hAuthList,
												   pbIsValid);
			if (RV_OK != rv)
			{
				RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
						   "SecAgreeClientCheckSecurity - client security-agreement=0x%p: Failed to check the validity of Authentication data",
						   pSecAgreeClient));
				return rv;
			}
			if (*pbIsValid == RV_FALSE)
			{
				RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
						 "SecAgreeClientCheckSecurity - client security-agreement=0x%p: Proxy-Authenticate header is invalid",
						 pSecAgreeClient));
			}
		}
	}

	return RV_OK;
}

/***************************************************************************
 * SecAgreeClientSetChosenSecurity
 * ------------------------------------------------------------------------
 * General: The security-agreement process results in choosing a security
 *          mechanism to use in further requests. Once the client security-
 *          agreement becomes active, it automatically makes the choice of a
 *          security mechanism (to view its choice use RvSipSecAgreeGetChosenSecurity).
 *          If an uninitialized client security-agreement received a Security-Server list,
 *          it assumes the CLIENT_REMOTE_LIST_READY state.
 *          In this state you can either initialize a local security list and
 *          call RvSipSecAgreeInit(), or you can avoid initialization and manually
 *          determine the chosen security by calling this function.
 *          You can also call RvSipSecAgreeSetChosenSecurity to run the automatic
 *          choice of the stack with your own choice.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree        - Handle to the client security-agreement object
 * Inout:  peSecurityType   - The chosen security mechanism. Use this parameter
 *                            to specify the chosen security mechanism. If you set
 *                            it to RVSIP_SECURITY_MECHANISM_UNDEFINED, the best
 *                            security mechanism will be computed and returned here.
 * Output: phLocalSecurity  - The local security header specifying the chosen security
 *         phRemoteSecurity - The remote security header specifying the chosen security
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeClientSetChosenSecurity(
							 IN     SecAgree                       *pSecAgree,
							 INOUT  RvSipSecurityMechanismType     *peSecurityType,
							 OUT    RvSipSecurityHeaderHandle      *phLocalSecurity,
							 OUT    RvSipSecurityHeaderHandle      *phRemoteSecurity)
{
	RvSipSecurityMechanismType  eChosenMechanism;
	RvSipSecurityHeaderHandle   hChosenLocalSecurityHeader;
	RvSipSecurityHeaderHandle   hChosenRemoteSecurityHeader;
	RvBool                      bIsValid;
	RvStatus                    rv;

	/* check if the application supplied security mechanism to choose */
	if (*peSecurityType == RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED)
	{
		/* The application did not supply security mechanism. We use the default one */
		rv = UseDefaultSecurityMechanism(pSecAgree, &eChosenMechanism, &hChosenLocalSecurityHeader, 
										 &hChosenRemoteSecurityHeader, &bIsValid);
		if (RV_OK != rv)
		{
			/* Failed to compute chosen mechanism */
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
						"SecAgreeClientSetChosenSecurity - Security-agreement 0x%p: Failed to compute best mechanism",
						pSecAgree));
			return rv;
		}
	}
	else
	{
		/* The application did supply security mechanism */
		eChosenMechanism = *peSecurityType;
		rv = SetApplicationSecurityMechanism(pSecAgree, eChosenMechanism, &hChosenLocalSecurityHeader, 
											 &hChosenRemoteSecurityHeader, &bIsValid);
		if (RV_OK != rv)
		{
			/* Failed to set application mechanism */
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
						"SecAgreeClientSetChosenSecurity - Security-agreement 0x%p: Failed to set mechanism %s",
						pSecAgree, SecAgreeUtilsMechanismEnum2String(eChosenMechanism)));
			return rv;
		}
	}		
	
	if (bIsValid == RV_FALSE)
	{
		/* Choice is invalid */
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc, (pSecAgree->pSecAgreeMgr->pLogSrc,
				   "SecAgreeClientSetChosenSecurity - Security-agreement 0x%p: Failed to verify the mechanism chosen",
				   pSecAgree));
		return RV_ERROR_ILLEGAL_ACTION;
	}

	if(RV_FALSE == CanActivateSecObj(pSecAgree, eChosenMechanism))
	{
		/* Address information is not sufficient */
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc, (pSecAgree->pSecAgreeMgr->pLogSrc,
				   "SecAgreeClientSetChosenSecurity - Security-agreement 0x%p: The address information (local or remote) is insufficient",
				   pSecAgree));
		return RV_ERROR_ILLEGAL_ACTION;
	}
	
	SetChosenSecurity(pSecAgree, eChosenMechanism, hChosenLocalSecurityHeader, hChosenRemoteSecurityHeader);
	
	/* Init security object if needed */
	rv = SecAgreeObjectUpdateSecObj(pSecAgree, SEC_AGREE_PROCESSING_SET_SECURITY);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"SecAgreeClientSetChosenSecurity - Client security-agreement=0x%p failed to init security object, rv=%d",
					pSecAgree, rv));
		/* Reset the chosen mechanism */
		SetChosenSecurity(pSecAgree, RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED, NULL, NULL);
		return rv;
	}	

	*peSecurityType	  = eChosenMechanism;
	*phLocalSecurity  = hChosenLocalSecurityHeader;
	*phRemoteSecurity = hChosenRemoteSecurityHeader;
	
	rv = SecAgreeStateProgress(pSecAgree,
						  SEC_AGREE_PROCESSING_SET_SECURITY,
						  NULL, /* irrelevant */
						  RVSIP_TRANSPORT_UNDEFINED, /* irrelevant */
						  RV_FALSE, /* irrelevant */
						  RV_FALSE, /* irrelevant */
						  RV_FALSE /* irrelevant */);

	return rv;
}

/*------------------------------------------------------------------------
           S T A T I C   F U N C T I O N S
 -------------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeClientMsgRcvdReplaceObjectIfNeeded
 * ------------------------------------------------------------------------
 * General: Check if there are Security-Server headers in the message. If there
 *          are, check that there is a client security-agreement object with
 *          initial state ready to handle them. If the current client
 *          security-agreement has illegal state, make this client invalid,
 *          and ask the application to supply a new client object to handle the
 *          new Security-Server list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr     - Pointer to the security manager
 *         hMsg             - Handle to the received message
 *         pOwner           - Owner details to supply the application.
 *         pOwnerLock       - The triple lock of the owner, to open before callback
 * Inout:  ppSecAgreeClient - Pointer to the client security-agreement. If a new client security-agreement
 *                            is obtained, it will replace the current one when function
 *                            returns.
 * Output: phMsgListElem    - Handle to the list element of the first security header,
 *                            if exists. This way when building the list we can avoid
 *                            repeating the search for the first header
 *         phHeader         - Pointer to the first security header, if exists.
 ***************************************************************************/
static RvStatus RVCALLCONV SecAgreeClientMsgRcvdReplaceObjectIfNeeded(
							 IN     SecAgreeMgr*               pSecAgreeMgr,
							 IN     RvSipMsgHandle             hMsg,
							 IN     RvSipSecAgreeOwner*        pOwner,
							 IN     SipTripleLock*			   pOwnerLock,
							 INOUT  SecAgree**                 ppSecAgreeClient,
							 OUT    RvSipHeaderListElemHandle* phMsgListElem,
							 OUT    RvSipSecurityHeaderHandle* phHeader)
{
	SecAgree* pSecAgreeClient       = *ppSecAgreeClient;
	SecAgree* pNewSecAgreeClient    = NULL;
	RvStatus  rv;

	/* check if the client security-agreement is ready to receive and load a security-server list */
	if (RV_TRUE == SecAgreeStateCanLoadRemoteSecList(pSecAgreeClient))
	{
		/* there client is able to load Security-Server list if needed */
		return RV_OK;
	}

	/* check if there are Security-Server headers in the message */
	if (RV_FALSE == SecAgreeMsgHandlingClientIsThereSecurityServerListInMsg(hMsg, phMsgListElem, phHeader))
	{
		/* there are no Security-Server headers in the message */
		return RV_OK;
	}

	/* There are security-server headers in the message */

	if (NULL != pSecAgreeClient)
	{
		/* There is a current client security-agreement object to this owner and either
		   (1) it is invalid already or (2) its state indicates it will become invalid due
		   to the receipt of a security-server list.
		   First we request a new client security-agreement object from the application, and then we change
		   the state of the current one to INVALID (if needed) */
		if (RV_TRUE == SecAgreeStateIsValid(pSecAgreeClient))
		{
			/* The old client security-agreement object becomes INVALID */
			RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					  "SecAgreeClientMsgRcvdReplaceObjectIfNeeded - client security-agreement=0x%p: received Security-Server list in state %s, message=0x%p",
					  pSecAgreeClient, SecAgreeUtilsStateEnum2String(pSecAgreeClient->eState), hMsg));
			SecAgreeStateInvalidate(pSecAgreeClient, RVSIP_SEC_AGREE_REASON_SECURITY_SERVER_RCVD);
		}
		/* We unlock the client security-agreement at this point. We do not refer to it after this point */
		SecAgreeUnLockAPI(pSecAgreeClient);
		/* reset the new client security-agreement until a new one will be supplied */
		*ppSecAgreeClient = NULL;
	}

	RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
			  "SecAgreeClientMsgRcvdReplaceObjectIfNeeded - current client security-agreement=0x%p; requesting the application to supply new client security-agreement for Owner=0x%p",
		      pSecAgreeClient, pOwner->pOwner));

	/* The application is asked to supply a client security-agreement object, since the server
	   requested the beginning of a negotiation */
	rv = SecAgreeCallbacksRequiredEv(pSecAgreeMgr,
									 pOwner,
									 pOwnerLock,
									 hMsg,
									 pSecAgreeClient,
									 RVSIP_SEC_AGREE_ROLE_CLIENT,
									 &pNewSecAgreeClient);
	if (RV_OK != rv)
	{
		/* the application is responsible to attach the new security object to the old owners */
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeClientMsgRcvdReplaceObjectIfNeeded - The application returned rv=%d from callback EvSecurityAgreementRequired, owner=0x%p",
					rv, pOwner->pOwner));
		return rv;
	}

	RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
			  "SecAgreeClientMsgRcvdReplaceObjectIfNeeded - Application supplied new client security-agreement=0x%p for owner=0x%p",
		      pNewSecAgreeClient, pOwner->pOwner));

	/* lock the new client security-agreement */
	if (NULL != pNewSecAgreeClient)
	{
		rv = SecAgreeLockAPI(pNewSecAgreeClient);
		if(rv != RV_OK)
		{
			return RV_ERROR_INVALID_HANDLE;
		}
		/* Note: There is no need to unlock the security-agreement before leaving this function! 
		         The caller of this function is expecting a locked security-agreement to be returned
				 to it.
				 The model of locking is the following: the caller of this function sends a locked
				 security-agreement to this function. Then we unlock the security-agreement and call 
				 the RequiredEv. Then if a new security-agreement is returned (this is a different 
				 security-agreement) it is locked here. This way the caller of this function is not 
				 aware that the security-agreement was replaced, and it continues to use the 
				 security-agreement that was returned. */
				 
	}

	*ppSecAgreeClient = pNewSecAgreeClient;

	/* check the state of the new client-security agreement that was supplied by the application */
	if (NULL != pNewSecAgreeClient &&
		(RVSIP_SEC_AGREE_ROLE_CLIENT != pNewSecAgreeClient->eRole ||
		 RV_FALSE == SecAgreeStateCanLoadRemoteSecList(pNewSecAgreeClient)))
	{
		/* Invalidate the security-agreement */
		SecAgreeStateInvalidate(pNewSecAgreeClient, RVSIP_SEC_AGREE_REASON_UNDEFINED);
		/* The application supplied invalid client security-agreement object */
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeClientMsgRcvdReplaceObjectIfNeeded - The application supplied new client security-agreement=0x%p in un initial state=%s for owner=0x%p",
					pNewSecAgreeClient, SecAgreeUtilsStateEnum2String(pNewSecAgreeClient->eState), pOwner->pOwner));
		return RV_ERROR_UNKNOWN;
	}

	return RV_OK;
}

/***************************************************************************
 * SecAgreeClientShouldInsertClientListToMsg
 * ------------------------------------------------------------------------
 * General: Checks whether Security-Client list should be inserted to the
 *          outgoing message.
 *          Security-Client list should be inserted to the initial request,
 *          as described in RFC3329, and to the first protected request,
 *          as described in TS33.202-770. Therefore, we insert Security-Client
 *          list in state CLIENT_LOCAL_LIST_READY (initial request) and in state
 *          ACTIVE, when the eInsertSecClient is on and ipsec is chosen.
 *          We automatically insert Security-Client list if the application
 *          forces us to do so.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree - pointer to the security-agreement object
 ***************************************************************************/
static RvBool RVCALLCONV SecAgreeClientShouldInsertClientListToMsg(
									IN     SecAgree*               pSecAgree)
{
	if (pSecAgree->securityData.hLocalSecurityList == NULL)
	{
		/* No Security-Client list */
		return RV_FALSE;
	}

	if (RV_FALSE == SecAgreeStateIsActive(pSecAgree))
	{
		/* The state indicates that this is an initial request hence
		   Security-Client list should be added */
		return RV_TRUE;
	}

	if (pSecAgree->securityData.eInsertSecClient == SEC_AGREE_SET_CLIENT_LIST_BY_APP)
	{
		/* The application forced setting Security-Client list to outgoing request */
		return RV_TRUE;
	}

	if (pSecAgree->securityData.eInsertSecClient == SEC_AGREE_SET_CLIENT_LIST_BY_IPSEC &&
		pSecAgree->securityData.selectedSecurity.eType == RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP)
	{
		/* This is the first outgoing request sent in ACTIVE state and ipsec-3gpp is
		   the chosen mechanism */
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeClientListInsertedToMsg
 * ------------------------------------------------------------------------
 * General: If the client list was inserted to the message in state ACTIVE,
 *          we reset the eInsertSecClient indication to stop inserting it to
 *          requests (only if it is not forced by the application)
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree    - pointer to the security-agreement object
 *         bWasInserted - Indicates whether Security-Client list was actually
 *                        inserted to the message
 ***************************************************************************/
static void RVCALLCONV SecAgreeClientListInsertedToMsg(
										IN  SecAgree*      pSecAgree)
{
	if (RV_TRUE == SecAgreeStateIsValid(pSecAgree) &&
		pSecAgree->securityData.eInsertSecClient == SEC_AGREE_SET_CLIENT_LIST_BY_IPSEC)
	{
		/* Turn off the automatic indication to insert Security-Client list (if
		   not forced by the application) */
		pSecAgree->securityData.eInsertSecClient = SEC_AGREE_SET_CLIENT_LIST_UNDEFINED;
	}
}

/***************************************************************************
 * CanMsgBePartOfNegotiation
 * ------------------------------------------------------------------------
 * General: Checks the method and the response code of the message to determine
 *          if it should be processed by the client
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg             - Handle to the message
 ***************************************************************************/
static RvBool RVCALLCONV CanMsgBePartOfNegotiation(IN     RvSipMsgHandle      hMsg)
{
	RvInt16               statusCode;

	if (RvSipMsgGetMsgType(hMsg) == RVSIP_MSG_RESPONSE)
	{
		/* Check status code */
		statusCode = RvSipMsgGetStatusCode(hMsg);
		if (statusCode < 400 || statusCode > 499)
		{
			/* We process only 4xx responses */
			return RV_FALSE;
		}
	}

	return SecAgreeUtilsIsMethodForSecAgree(hMsg);
}

/***************************************************************************
 * IsProvisionalResponse
 * ------------------------------------------------------------------------
 * General: Checks the response code of the message is provisional
 *          
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg             - Handle to the message
 ***************************************************************************/
static RvBool RVCALLCONV IsProvisionalResponse(IN     RvSipMsgHandle      hMsg)
{
	RvInt16               statusCode;

	if (RvSipMsgGetMsgType(hMsg) == RVSIP_MSG_RESPONSE)
	{
		/* Check status code */
		statusCode = RvSipMsgGetStatusCode(hMsg);
		if (statusCode < 200)
		{
			return RV_TRUE;
		}
	}

	return RV_FALSE;
}

/***************************************************************************
 * GetIpsecParamsIndex
 * ------------------------------------------------------------------------
 * General: Returns an index that is produced from the algorithm and encryption
 *          algorithm of the ipsec security header
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecurityHeader    - Handle to the security header
 * Output:  pSpecificParameter - The specific parameter index
 *          pbIsSupported      - Indicates whether the parameters are supported
 ***************************************************************************/
static void RVCALLCONV GetIpsecParamsIndex(IN    RvSipSecurityHeaderHandle   hSecurityHeader,
										   OUT   RvInt32                    *pSpecificParameter,
										   OUT   RvBool                     *pbIsSupported)
{
#define MAX_NUM_OF_ALGORITHMS    6 /* see RvSipSecurityAlgorithmType. Safety margin taken */
	RvSipSecurityAlgorithmType        eAlgorithm;
	RvSipSecurityEncryptAlgorithmType eEAlgorithm;

	eAlgorithm  = SecAgreeUtilsSecurityHeaderGetAlgo(hSecurityHeader);
	eEAlgorithm = SecAgreeUtilsSecurityHeaderGetEAlgo(hSecurityHeader);
	
	if (eAlgorithm == RVSIP_SECURITY_ALGORITHM_TYPE_UNDEFINED)
	{
		*pbIsSupported = RV_FALSE;
		return;
	}

	*pbIsSupported = RV_TRUE;
	*pSpecificParameter = eAlgorithm + MAX_NUM_OF_ALGORITHMS*eEAlgorithm;
}

/***************************************************************************
 * UseDefaultSecurityMechanism
 * ------------------------------------------------------------------------
 * General: Computes the default security mechanism or uses the previously
 *          compared one 
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree        - Pointer to the security agreement
 * Output:    peSecurityType   - The computed mechanism
 *            phLocalSecurity  - The computed local header
 *            phRemoteSecurity - The computed remote header
 *            pbIsValid        - Boolean indication to whether the chosen 
 *                               mechanism is valid
 ***************************************************************************/
static RvStatus RVCALLCONV UseDefaultSecurityMechanism(
                                      IN   SecAgree*                   pSecAgree,
									  OUT  RvSipSecurityMechanismType* peSecurityType,
									  OUT  RvSipSecurityHeaderHandle*  phLocalSecurity,
									  OUT  RvSipSecurityHeaderHandle*  phRemoteSecurity,
									  OUT  RvBool*                     pbIsValid)
{
	RvStatus   rv;

	*peSecurityType   = RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED;
	*phLocalSecurity  = NULL;
	*phRemoteSecurity = NULL;
	*pbIsValid        = RV_TRUE;

	/* Only if the mechanism was not previously computed by the security-agreement, we compute it here */
	if (pSecAgree->securityData.selectedSecurity.eType == RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED)
	{
		RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				  "UseDefaultSecurityMechanism - Client security-agreement 0x%p computing best security mechanism",
				  pSecAgree));
	
		rv = SecAgreeClientChooseMechanism(pSecAgree, peSecurityType, phLocalSecurity, phRemoteSecurity, pbIsValid);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
						"UseDefaultSecurityMechanism - Security-agreement 0x%p: failed to compute selected mechanism",
						pSecAgree));
			return rv;
		}
	}
	else if (pSecAgree->securityData.selectedSecurity.hLocalHeader  == NULL || 
			 pSecAgree->securityData.selectedSecurity.hRemoteHeader == NULL)
	{
		/* There are no remote or local headers, we cannot compute the selected mechanism */
		*pbIsValid = RV_FALSE;
	}
	else
	{
		/* A mechanism was previously computed, and we use it here */
		*peSecurityType   = pSecAgree->securityData.selectedSecurity.eType;
		*phLocalSecurity  = pSecAgree->securityData.selectedSecurity.hLocalHeader;
		*phRemoteSecurity = pSecAgree->securityData.selectedSecurity.hRemoteHeader;
	}

	return RV_OK;
}

/***************************************************************************
 * SetApplicationSecurityMechanism
 * ------------------------------------------------------------------------
 * General: Sets the choice of the application for a security mechanism
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree        - Pointer to the security agreement
 *            eSecurityType    - The mechanism chosen by the application
 * Output:    phLocalSecurity  - The computed local header
 *            phRemoteSecurity - The computed remote header
 *            pbIsValid        - Boolean indication to whether the chosen 
 *                               mechanism is valid
 ***************************************************************************/
static RvStatus RVCALLCONV SetApplicationSecurityMechanism(
                                      IN   SecAgree*                  pSecAgree,
									  IN   RvSipSecurityMechanismType eSecurityType,
									  OUT  RvSipSecurityHeaderHandle* phLocalSecurity,
									  OUT  RvSipSecurityHeaderHandle* phRemoteSecurity,
									  OUT  RvBool*                    pbIsValid)
{
	RvSipSecurityAlgorithmType          eIpsecAlg      = RVSIP_SECURITY_ALGORITHM_TYPE_UNDEFINED;
	RvSipSecurityEncryptAlgorithmType   eIpsecEAlg     = RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_UNDEFINED;
	RvSipSecurityModeType               eIpsecMode     = RVSIP_SECURITY_MODE_TYPE_UNDEFINED;
	RvSipSecurityProtocolType           eIpsecProtocol = RVSIP_SECURITY_PROTOCOL_TYPE_UNDEFINED;
	RvStatus							rv = RV_OK;

	*phLocalSecurity  = NULL;
	*phRemoteSecurity = NULL;
	*pbIsValid        = RV_TRUE;

	/* Check if the given mechanism is supported */
	if (RV_FALSE == SecAgreeUtilsIsSupportedMechanism(eSecurityType))
	{
		/* The application is trying to set unsupported mechanism */
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"SetApplicationSecurityMechanism - Security-agreement 0x%p does not set unsupported mechanism %s",
					pSecAgree, SecAgreeUtilsMechanismEnum2String(eSecurityType)));
		return RV_ERROR_ILLEGAL_ACTION;
	}

	RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
              "SetApplicationSecurityMechanism - Client security-agreement 0x%p setting mechanism %s",
			  pSecAgree, SecAgreeUtilsMechanismEnum2String(eSecurityType)));
	
	/* Check if there is a client list, and find the matching security header within it */
	if (RV_TRUE == SecAgreeStateIsClientChooseSecurity(pSecAgree) &&
		pSecAgree->securityData.hLocalSecurityList != NULL)
	{
		if (NULL != pSecAgree->securityData.hLocalSecurityList)
		{
			rv = SecAgreeUtilsFindBestHeader(pSecAgree->securityData.hLocalSecurityList, 
											 eSecurityType, eIpsecAlg, eIpsecEAlg, eIpsecMode, eIpsecProtocol,
											 phLocalSecurity);
		}
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
						"SetApplicationSecurityMechanism - Security-agreement 0x%p: failed to search for local security header matching mechanism %s",
						pSecAgree, SecAgreeUtilsMechanismEnum2String(eSecurityType)));
			return rv;
		}
	}
	
	if (eSecurityType == RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP)
	{
		if (*phLocalSecurity == NULL)
		{
			/* The application is trying to set unsupported mechanism */
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
						"SetApplicationSecurityMechanism - Security-agreement 0x%p Cannot choose ipsec without having local ipsec initiation",
						pSecAgree));
			return RV_ERROR_ILLEGAL_ACTION;
		}

		eIpsecAlg      = SecAgreeUtilsSecurityHeaderGetAlgo(*phLocalSecurity);
		eIpsecEAlg     = SecAgreeUtilsSecurityHeaderGetEAlgo(*phLocalSecurity);
		eIpsecMode     = SecAgreeUtilsSecurityHeaderGetMode(*phLocalSecurity);
		eIpsecProtocol = SecAgreeUtilsSecurityHeaderGetProtocol(*phLocalSecurity);
	}

	/* Find remote security header that matches the given mechanism. The chosen security must match
	   a header from the remote list. */
	if (pSecAgree->securityData.hRemoteSecurityList != NULL)
	{
		rv = SecAgreeUtilsFindBestHeader(pSecAgree->securityData.hRemoteSecurityList, 
										 eSecurityType, eIpsecAlg, eIpsecEAlg, eIpsecMode, eIpsecProtocol,
										 phRemoteSecurity);
	}
	if (NULL == *phRemoteSecurity)
	{
		rv = RV_ERROR_ILLEGAL_ACTION;
	}
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"SetApplicationSecurityMechanism - Security-agreement 0x%p: failed to find remote security header matching mechanism %s",
					pSecAgree, SecAgreeUtilsMechanismEnum2String(eSecurityType)));
		return rv;
	}

	/* Check that the chosen security is valid, e.g. check that there is Proxy-Auth
	   if the chosen security is digest */
	rv = SecAgreeClientCheckSecurity(pSecAgree, eSecurityType, pbIsValid);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc, (pSecAgree->pSecAgreeMgr->pLogSrc,
				   "SetApplicationSecurityMechanism - Security-agreement 0x%p: Failed to verify the mechanism chosen, rv=%d",
				   pSecAgree, rv));
		return rv;
	}

	return RV_OK;
}

/***************************************************************************
 * SetChosenSecurity
 * ------------------------------------------------------------------------
 * General: Sets the given security details to the security-agreement
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient    - Pointer to the client security-agreement
 *         eSecurityType      - The type of the chosen security mechanism
 *         hLocalSecurity     - The chosen local security header
 *         hRemoteSecurity    - The chosen remote security header
 ***************************************************************************/
static void RVCALLCONV SetChosenSecurity(
							 IN  SecAgree*                       pSecAgreeClient,
							 IN  RvSipSecurityMechanismType      eSecurityType,
							 IN  RvSipSecurityHeaderHandle       hLocalSecurity,
							 IN  RvSipSecurityHeaderHandle       hRemoteSecurity)
{
	pSecAgreeClient->securityData.selectedSecurity.eType         = eSecurityType;
	pSecAgreeClient->securityData.selectedSecurity.hLocalHeader  = hLocalSecurity;
	pSecAgreeClient->securityData.selectedSecurity.hRemoteHeader = hRemoteSecurity;
}

/***************************************************************************
 * SecurityMechanismEnumToIndex
 * ------------------------------------------------------------------------
 * General: Convert enumeration value of security mechanism to array index
 * Return Value: The array index
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  eMechanism - The mechanism to convert
 ***************************************************************************/
static RvInt RVCALLCONV SecurityMechanismEnumToIndex(
							 IN  RvSipSecurityMechanismType  eMechanism)
{
	RvInt  res;
	
	switch (eMechanism) 
	{
	case RVSIP_SECURITY_MECHANISM_TYPE_DIGEST:
		res = 0;
		break;
	case RVSIP_SECURITY_MECHANISM_TYPE_TLS:
		res = 1;
		break;
	case RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_IKE:
		res = 2;
		break;
	case RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_MAN:
		res = 3;
		break;
	case RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP:
		res = 4;
		break;
	default:
		return -1;
	}

	if (res >= MAX_NUM_OF_SUPPORTED_MECHANISMS)
	{
		return -1;
	}

	return res;
}

/***************************************************************************
 * CanActivateSecObj
 * ------------------------------------------------------------------------
 * General: Checks that we have all information needed to activate the security
 *          object
 * Return Value: RvBool
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - The security-agreement object
 *         eMechanism - The mechanism to check
 ***************************************************************************/
static RvBool RVCALLCONV CanActivateSecObj(
							 IN  SecAgree*                   pSecAgree,
							 IN  RvSipSecurityMechanismType  eMechanism)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
	RvInt  addrType;

	if (eMechanism == RVSIP_SECURITY_MECHANISM_TYPE_TLS)
	{
		if (pSecAgree->addresses.remoteTlsAddr.addrtype == 0)
		{
			return RV_FALSE;
		}
		addrType = RvAddressGetType(&pSecAgree->addresses.remoteTlsAddr);
		if (SipTransportConvertCoreAddrTypeToSipAddrType(addrType) == RVSIP_TRANSPORT_ADDRESS_TYPE_IP6 &&
			pSecAgree->addresses.hLocalTlsIp6Addr == NULL)
		{
			return RV_FALSE;
		}
        if (SipTransportConvertCoreAddrTypeToSipAddrType(addrType) == RVSIP_TRANSPORT_ADDRESS_TYPE_IP &&
            pSecAgree->addresses.hLocalTlsIp4Addr == NULL)
		{
			return RV_FALSE;
		}
	}
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_IMS_IPSEC_ENABLED==RV_YES)		
	if (eMechanism == RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP)
	{
		if (pSecAgree->addresses.remoteIpsecAddr.addrtype == 0 ||
		    pSecAgree->addresses.eRemoteIpsecTransportType == RVSIP_TRANSPORT_UNDEFINED)
		{
			/* No remote address set */
			return RV_FALSE;
		}
		if (pSecAgree->addresses.eRemoteIpsecTransportType == RVSIP_TRANSPORT_TCP &&
			pSecAgree->addresses.hLocalIpsecTcpAddr == NULL &&
			pSecAgree->addresses.pLocalIpsecIp == NULL)
		{
			/* No local TCP address */
			return RV_FALSE;
		}
		if (pSecAgree->addresses.eRemoteIpsecTransportType == RVSIP_TRANSPORT_UDP &&
			pSecAgree->addresses.hLocalIpsecUdpAddr == NULL &&
			pSecAgree->addresses.pLocalIpsecIp == NULL)
		{
			/* No local UDP address */
			return RV_FALSE;
		}
	}
#endif /* #if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

	RV_UNUSED_ARG(pSecAgree);
	RV_UNUSED_ARG(eMechanism);

	return RV_TRUE;
}

#endif /* #ifdef RV_SIP_IMS_ON */

