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
 *                              <SecAgreeServer.c>
 *
 * This file implement the server security-agreement behavior, as defined in RFC 3329.
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

#include "RvSipSecAgreeTypes.h"
#include "SecAgreeObject.h"
#include "SecAgreeState.h"
#include "SecAgreeStatus.h"
#include "SecAgreeServer.h"
#include "SecAgreeMgrObject.h"
#include "SecAgreeMsgHandling.h"
#include "SecAgreeCallbacks.h"
#include "SecAgreeUtils.h"
#include "_SipSecuritySecObj.h"
#include "RvSipCSeqHeader.h"

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#include "SecurityObject.h"
#endif
/* SPIRENT_END */

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV SecAgreeServerMsgRcvdReplaceObjectIfNeeded(
							 IN     SecAgreeMgr*               pSecAgreeMgr,
							 IN     RvSipMsgHandle             hMsg,
							 IN     RvSipSecAgreeOwner*        pOwner,
							 IN     SipTripleLock*             pOwnerLock,
							 INOUT  SecAgree**                 ppSecAgreeServer,
							 OUT    RvSipHeaderListElemHandle* phClientMsgListElem,
							 OUT    RvSipSecurityHeaderHandle* phClientHeader,
							 OUT    RvSipHeaderListElemHandle* phVerMsgListElem,
							 OUT    RvSipSecurityHeaderHandle* phVerHeader);

static RvStatus RVCALLCONV SecAgreeServerMsgRcvdReplaceObject(
							 IN     SecAgreeMgr*               pSecAgreeMgr,
							 IN     RvSipMsgHandle             hMsg,
							 IN     RvSipSecAgreeOwner*        pOwner,
							 IN     SipTripleLock*             pOwnerLock,
							 IN     RvSipSecAgreeReason        eReason,
							 INOUT  SecAgree**                 ppSecAgreeServer);

static RvBool RVCALLCONV ServerShouldProcessRequestMsg(IN     RvSipMsgHandle      hMsg);

static RvStatus RVCALLCONV SecAgreeServerMsgRcvdUpdateRemoteAddr(
							 IN    SecAgree*                      pSecAgreeServer,
							 IN    SipTransportAddress*           pRcvdFromAddr);

static RvStatus RVCALLCONV SecAgreeServerMsgRcvdUpdateSecurityMechanism(
							 IN    SecAgree*                      pSecAgreeServer,
							 IN    RvSipTransportLocalAddrHandle  hLocalAddr,
							 IN    SipTransportAddress*           pRcvdFromAddr,
							 IN	   RvSipTransportConnectionHandle hConn);

static RvBool RVCALLCONV CanResponseBePartOfNegotiation(IN     RvSipMsgHandle      hMsg);

static RvStatus RVCALLCONV SecAgreeServerMsgRcvdHandleSecurityHeaders(
							 IN     RvSipMsgHandle             hMsg,
							 IN     RvSipSecAgreeOwner*        pOwner,
							 IN     SipTripleLock*             pOwnerLock,
							 IN     RvSipHeaderListElemHandle  hCltMsgListElem,
							 IN     RvSipSecurityHeaderHandle  hCltHeader,
							 IN     RvSipHeaderListElemHandle  hVerMsgListElem,
							 IN     RvSipSecurityHeaderHandle  hVerHeader,
							 INOUT  SecAgree*                 *ppSecAgreeServer,
							 OUT    RvBool                    *pbSecAgreeClientLoaded,
							 OUT    RvBool                    *pbHasSecVerify);

/*-----------------------------------------------------------------------*/
/*                         SECURITY Server FUNCTIONS                     */
/*-----------------------------------------------------------------------*/

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#include "rvexternal.h"
#endif
/* SPIRENT_END */

/***************************************************************************
 * SecAgreeServerInit
 * ------------------------------------------------------------------------
 * General: Used in states IDLE and SERVER_REMOTE_LIST_READY,
 *          to indicate that the application has completed to initialize the local 
 *          security list; and local digest list for server security-agreement. Once 
 *          called, the application can no longer modify the local security information. 
 *          Calling RvSipSecAgreeInit() modifies the state of the security-agreement 
 *          object to allow proceeding in the security-agreement process. It will invoke 
 *          the creation and initialization of a security object according to the supplied 
 *          parameters.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeServer   - Pointer to the server security-agreement object
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeServerInit(IN   SecAgree*    pSecAgreeServer)
{	
	RvStatus  rv;

	/* Initiate the security object */
	rv = SecAgreeObjectUpdateSecObj(pSecAgreeServer, SEC_AGREE_PROCESSING_INIT);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
				   "SecAgreeServerInit - Server security-agreement=0x%p: Failed to init security object, rv=%d",
				   pSecAgreeServer, rv));
		return rv;
	}

	/* Modify state due to init */
	rv = SecAgreeStateProgress(pSecAgreeServer, 
						  SEC_AGREE_PROCESSING_INIT, 
						  NULL, /* irrelevant */
						  RVSIP_TRANSPORT_UNDEFINED, /* irrelevant */
						  RV_FALSE, /* irrelevant */
						  RV_FALSE, /* irrelevant */
						  RV_FALSE /* irrelevant */);

	return rv;
}

/***************************************************************************
 * SecAgreeServerHandleMsgToSend
 * ------------------------------------------------------------------------
 * General: Processes a message about to be sent: adds security information
 *          and updates the server security-agreement object accordingly.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeServer - Pointer to the server security-agreement
 *         hMsg            - Handle to the message
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeServerHandleMsgToSend(
							 IN  SecAgree                *pSecAgreeServer,
							 IN  RvSipMsgHandle           hMsg)
{
	RvUint32  cseqStep;
	RvStatus  rv;

	if (CanResponseBePartOfNegotiation(hMsg) == RV_FALSE)
	{
		/* The status of this security-agreement might change due to this received message.
		   This is the only processing that should be taken for a non negotiation response. */
		rv = SecAgreeStatusConsiderNewStatus(pSecAgreeServer, 
											 SEC_AGREE_PROCESSING_MSG_TO_SEND, 
											 RvSipMsgGetStatusCode(hMsg));
		return rv;
	}

	RvLogInfo(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
			  "SecAgreeServerHandleMsgToSend - Server security-agreement 0x%p handling message to send (message=0x%p)",
			  pSecAgreeServer, hMsg));

	/* print warning to the log if the security-agreement is invalid. */
	if (SecAgreeStateIsValid(pSecAgreeServer) == RV_FALSE)
	{
		RvLogWarning(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
					 "SecAgreeServerHandleMsgToSend - Server security-agreement=0x%p sending message=0x%p in state %s",
					 pSecAgreeServer, hMsg, SecAgreeUtilsStateEnum2String(pSecAgreeServer->eState)));
		return RV_OK;
	}

	/* check if the state requires processing message to send */
	if (SecAgreeStateShouldProcessMsgToSend(pSecAgreeServer) == RV_FALSE)
	{
		RvLogInfo(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
					"SecAgreeServerHandleMsgToSend - Server security-agreement=0x%p nothing to do in state %s (message=0x%p)",
					pSecAgreeServer, SecAgreeUtilsStateEnum2String(pSecAgreeServer->eState), hMsg));
		return RV_OK;
	}

	/* retrieve the cseq step from the message to send */
	cseqStep = SecAgreeUtilsGetCSeqStep(hMsg);
	
	/* If the CSeq of the message does not match the CSeq stored within the server security-agreement,
	   we do not add security information to the response (this is a response on inner transaction, such
	   as PRACK */
	if (pSecAgreeServer->securityData.cseqStep != 0 &&
		pSecAgreeServer->securityData.cseqStep != cseqStep)
	{
		return RV_OK;
	}

	/* Add security-agreement information to message */
	
	/* Add Security-Server list to message */
	rv = SecAgreeMsgHandlingServerSetSecurityServerListToMsg(pSecAgreeServer, hMsg);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
					"SecAgreeServerHandleMsgToSend - server security-agreement=0x%p: Failed to set security-server list to message=0x%p, rv=%d",
					pSecAgreeServer, hMsg, rv));
		return rv;
	}

	/* Add digest information to message */
	rv = SecAgreeMsgHandlingServerSetAuthenticationDataToMsg(pSecAgreeServer, hMsg);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
					"SecAgreeServerHandleMsgToSend - server security-agreement=0x%p: Failed to set authentication data to message=0x%p, rv=%d",
					pSecAgreeServer, hMsg, rv));
		return rv;
	}

	/* Security-Server list wes inserted to the message. We insert Require and Proxy-Require with sec-agree */
	rv = SecAgreeMsgHandlingSetRequireHeadersToMsg(pSecAgreeServer, hMsg);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
					"SecAgreeServerHandleMsgToSend - server security-agreement=0x%p: Failed to set Require and Proxy-Require headers to message=0x%p, rv=%d",
					pSecAgreeServer, hMsg, rv));
		return rv;
	}

	/* Move to the next state if needed */
	rv = SecAgreeStateProgress(pSecAgreeServer,
						  SEC_AGREE_PROCESSING_MSG_TO_SEND,
						  NULL, /* irrelevant */
						  RVSIP_TRANSPORT_UNDEFINED /*irrelevant*/ ,
						  RV_FALSE, /* irrelevant */
						  RV_FALSE, /* irrelevant */
						  RV_FALSE /* irrelevant */);

	return rv;
}

/***************************************************************************
 * SecAgreeServerHandleMsgSendFailure
 * ------------------------------------------------------------------------
 * General: Invalidates a security-agreement whenever a message send failure
 *          occurs.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeServer - Pointer to the server security-agreement
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeServerHandleMsgSendFailure(
							 IN  SecAgree                      *pSecAgreeServer)
{
	if (RV_FALSE == SecAgreeStateIsValid(pSecAgreeServer))
	{	
		/* Nothing to do in invalid state */
		RvLogInfo(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
					"SecAgreeServerHandleMsgSendFailure - Server security-agreement=0x%p: Message send failure is ignored in state %s", 
					pSecAgreeServer, SecAgreeUtilsStateEnum2String(pSecAgreeServer->eState)));
		return RV_OK;
	}

	RvLogInfo(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
			  "SecAgreeServerHandleMsgSendFailure - Server security-agreement 0x%p handling message send failure",
			  pSecAgreeServer));
	
	SecAgreeStateInvalidate(pSecAgreeServer, RVSIP_SEC_AGREE_REASON_MSG_SEND_FAILURE);

	return RV_OK;
}

/***************************************************************************
 * SecAgreeServerHandleMsgRcvd
 * ------------------------------------------------------------------------
 * General: Processes a received message : loads security information, verifies
 *          the security agreement and updates the server security-agreement object 
 *          accordingly.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr     - Pointer to the security-agreement manager
 *         pOwner           - The owner of the server security-agreement
 *         pOwnerLock       - The triple lock of the owner, to open before callback
 *         hMsg             - Handle to the message
 *         hLocalAddr       - The address that the message was received on
 *         pRcvdFromAddr    - The address the message was received from
 *         hRcvdOnConn      - The connection the message was received on
 * Inout:  ppSecAgreeServer - Pointer to the security-agreement object. If the
 *                            security-agreement object is replaced, the new 
 *                            security-agreement is updated here
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeServerHandleMsgRcvd(
							 IN    SecAgreeMgr* 				  pSecAgreeMgr,
							 IN    RvSipSecAgreeOwner*            pOwner,
							 IN    SipTripleLock*                 pOwnerLock,
							 IN    RvSipMsgHandle                 hMsg,
							 IN    RvSipTransportLocalAddrHandle  hLocalAddr,
							 IN    SipTransportAddress*           pRcvdFromAddr,
							 IN	   RvSipTransportConnectionHandle hRcvdOnConn,
							 INOUT SecAgree** 		              ppSecAgreeServer)
{
	SecAgree*  	              pSecAgreeServer = (SecAgree*)*ppSecAgreeServer;
	RvBool					  bSecurityClientLoaded = RV_FALSE;
	RvBool					  bHasSecAgree = RV_FALSE;
	RvBool					  bRequireSecAgree = RV_FALSE;
	RvBool                    bHasSecVerify = RV_FALSE;
	RvBool                    bIsInvalid = RV_FALSE;
	RvSipHeaderListElemHandle hClientMsgListElem = NULL;
    RvSipSecurityHeaderHandle hClientHeader = NULL;
	RvSipHeaderListElemHandle hVerMsgListElem = NULL;
    RvSipSecurityHeaderHandle hVerHeader = NULL;
	RvStatus                  rv;

	if (ServerShouldProcessRequestMsg(hMsg) == RV_FALSE)
	{
		/* This message does not require security-agreement processing */
		return RV_OK;
	}
	
	/* check if the state requires processing message received */
	if (SecAgreeStateShouldProcessRcvdMsg(pSecAgreeServer) == RV_FALSE)
	{
		if (NULL != pSecAgreeServer)
		{
			RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeServerHandleMsgRcvd - Server security-agreement=0x%p nothing to do in state %s (message=0x%p)",
					pSecAgreeServer, SecAgreeUtilsStateEnum2String(pSecAgreeServer->eState), hMsg));
		}
		return RV_OK;
	}

	if (pSecAgreeServer != NULL && pSecAgreeServer->numOfOwners == 0)
	{
		/* We do not allow processing a message without having any owners */
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeServerHandleMsgRcvd - Security-agreement 0x%p cannot process message 0x%p: there are no owners to the security-agreement",
					pSecAgreeServer, hMsg));
		return RV_ERROR_ILLEGAL_ACTION;
	}
	
	/* Check that the received message is valid for processing. If it has more than one Via, 
	   and Require with sec-agree, it is invalid and should be rejected with 502. If the owner
	   is a call-leg or a subs, the reject will occur automatically. If this is transaction, it
	   would be application responsibility. We help the application by setting the recommended 
	   response code to 502, if there is a security-agreement object at that point */
	rv = SecAgreeMsgHandlingServerIsInvalidRequest(pSecAgreeMgr, hMsg, &bIsInvalid, &bHasSecAgree);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeServerHandleMsgRcvd - Failed to check the validity of request 0x%p",
					hMsg));
		SecAgreeStateInvalidate(pSecAgreeServer, RVSIP_SEC_AGREE_REASON_UNDEFINED);
		return rv;
	}
	if (bIsInvalid == RV_TRUE)
	{
		RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeServerHandleMsgRcvd - Request 0x%p contains multiple Via headers with sec-agree required",
					hMsg));
		if (pSecAgreeServer != NULL)
		{
			pSecAgreeServer->responseCode = RESPONSE_502;
		}
		return RV_OK;
	}

	/* Until now we looked for Require sec-agree. Following we look for Supported sec-agree */
	bRequireSecAgree = bHasSecAgree;
	
	if (bHasSecAgree == RV_FALSE)
	{
		/* There are no Require headers in the message with sec-agree.
		   Check if there Supported header with sec-agree */
		rv = SecAgreeMsgHandlingServerIsSupportedSecAgreeInMsg(hMsg, &bHasSecAgree);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					   "SecAgreeServerHandleMsgRcvd - Failed to check request 0x%p for Supported header",
					   hMsg));
			SecAgreeStateInvalidate(pSecAgreeServer, RVSIP_SEC_AGREE_REASON_UNDEFINED);
			return rv;
		}
	}

	/* If this is initial state and the message was received on TLS, the server should not 
	   perform a security agreement */
	if (SecAgreeStateServerConsiderNoAgreementTls(pSecAgreeServer) == RV_TRUE)
	{
		if (pRcvdFromAddr->eTransport == RVSIP_TRANSPORT_TLS &&
			bRequireSecAgree == RV_FALSE)
		{
			if (pSecAgreeServer == NULL)
			{
				RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
						  "SecAgreeServerHandleMsgRcvd - Request 0x%p received on TLS. Do not require a security-agreement",
						  hMsg));
			}
			else
			{
				RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
						  "SecAgreeServerHandleMsgRcvd - Security-agreement 0x%p: Request 0x%p received on TLS in state %s.",
						  pSecAgreeServer, hMsg, SecAgreeUtilsStateEnum2String(pSecAgreeServer->eState)));
				
				/* Update the state of the server security-agreement */
				rv = SecAgreeStateProgress(pSecAgreeServer, 
									  SEC_AGREE_PROCESSING_MSG_RCVD, 
									  NULL, /* irrelevant */
									  RVSIP_TRANSPORT_TLS, 
									  RV_FALSE, /* irrelevant */
									  RV_FALSE, /* irrelevant */
									  RV_FALSE /* irrelevant */);
				if (RV_OK != rv)
				{
					RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
								"SecAgreeServerHandleMsgRcvd - server security-agreement=0x%p: Failed to change state, rv=%d",
								pSecAgreeServer, rv));
					return rv;
				}

				/* Notify the security-object that there is no need for an agreement */
				rv = SecAgreeObjectUpdateSecObj(pSecAgreeServer, SEC_AGREE_PROCESSING_MSG_RCVD);
				if (RV_OK != rv)
				{
					RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
								"SecAgreeServerHandleMsgRcvd - server security-agreement=0x%p: Failed to notify security-object 0x%p that there is no need for an agreement, rv=%d",
								pSecAgreeServer, pSecAgreeServer->securityData.hSecObj, rv));
					return rv;
				}
			}		
			
			return RV_OK;
		}
	}
	

	/* Before we load any information, we check that we have a valid server security-agreement object
	   to do so. If not, we request the application to supply one */
	rv = SecAgreeServerMsgRcvdReplaceObjectIfNeeded(pSecAgreeMgr, hMsg, pOwner, pOwnerLock,
													ppSecAgreeServer, 
													&hClientMsgListElem, &hClientHeader,
													&hVerMsgListElem, &hVerHeader);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				   "SecAgreeServerHandleMsgRcvd - server security-agreement=0x%p: Failed to assert object validity, rv=%d",
				   pSecAgreeServer, rv));
		return rv;
	}

	pSecAgreeServer = *ppSecAgreeServer;

	/* The application does not wish to initiate or participate in a security-agreement */
	if (NULL == pSecAgreeServer)
	{
		return RV_OK;
	}

	RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
			  "SecAgreeServerHandleMsgRcvd - Server security-agreement 0x%p handling message received (owner=0x%p, message=0x%p)",
			  pSecAgreeServer, pOwner, hMsg));

	/* Process Security headers. */
	rv = SecAgreeServerMsgRcvdHandleSecurityHeaders(hMsg, pOwner, pOwnerLock, hClientMsgListElem, hClientHeader, hVerMsgListElem, hVerHeader, 
													ppSecAgreeServer, &bSecurityClientLoaded, &bHasSecVerify);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeServerHandleMsgRcvd - server security-agreement=0x%p: Failed to handle security-verify list, rv=%d",
					pSecAgreeServer, rv));
		return rv;
	}
	
	pSecAgreeServer = *ppSecAgreeServer;

	/* The application does not wish to initiate or participate in a security-agreement */
	if (NULL == pSecAgreeServer)
	{
		return RV_OK;
	}

	/* Store the remote address in the server security-agreement and notify the security object */
	rv = SecAgreeServerMsgRcvdUpdateRemoteAddr(pSecAgreeServer, pRcvdFromAddr);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				   "SecAgreeServerHandleMsgRcvd - Server security-agreement 0x%p: Failed to store remote address",
				   pSecAgreeServer));
		SecAgreeStateInvalidate(pSecAgreeServer, RVSIP_SEC_AGREE_REASON_UNDEFINED);
		return rv;
	}

	/* Store the chosen mechanism in the server security-agreement and notify the security object */
	rv = SecAgreeServerMsgRcvdUpdateSecurityMechanism(pSecAgreeServer, hLocalAddr, pRcvdFromAddr, hRcvdOnConn);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				   "SecAgreeServerHandleMsgRcvd - Server security-agreement 0x%p: Failed to store chosen mechanism",
				   pSecAgreeServer));
		SecAgreeStateInvalidate(pSecAgreeServer, RVSIP_SEC_AGREE_REASON_UNDEFINED);
		return rv;
	}

	/* Update the state of the server security-agreement, if needed */
	rv = SecAgreeStateProgress(pSecAgreeServer, 
						  SEC_AGREE_PROCESSING_MSG_RCVD, 
						  NULL, /* irrelevant */
						  pRcvdFromAddr->eTransport, 
						  bSecurityClientLoaded,
						  bHasSecVerify,
						  bHasSecAgree);

	return rv;
}

#if (RV_IMS_IPSEC_ENABLED == RV_YES) 
/***************************************************************************
 * SecAgreeServerForceIpsecSecurity
 * ------------------------------------------------------------------------
 * General: Makes the previously obtained ipsec-3gpp channels serviceable at the server 
 *          security-agreement. Calling this function will cause any outgoing request 
 *          from this server to be sent on those ipsec-3gpp channels.
 *          Normally, the server security-agreement will automatically make
 *          the ipsec-3gpp channels serviceable, once the first secured request
 *          is receives from the client. However, in some circumstances, the server
 *          might wish to make the ipsec-3gpp channels serviceable with no further delay,
 *          which can be obtained by calling RvSipSecAgreeServerForceIpsecSecurity().
 *          Note: Calling RvSipSecAgreeServerForceIpsecSecurity() is enabled only in the 
 *          ACTIVE state of the security-object. 
 *          Note: Calling RvSipSecAgreeServerForceIpsecSecurity() is enabled only when
 *          the Security-Server list contains a single header with ipsec-3gpp 
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeServer - Pointer to the server security-agreement
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeServerForceIpsecSecurity(
                                   IN  SecAgree              *pSecAgreeServer)
{
	RvStatus   rv;
	RvBool     bIsSingle;

	/* Forcing ipsec is allowed only in state active */
	if (SecAgreeStateIsActive(pSecAgreeServer) == RV_FALSE)
	{
		RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
				   "SecAgreeServerForceIpsecSecurity - Server security-agreement 0x%p: Forcing ipsec is illegal in state %s",
				   pSecAgreeServer, SecAgreeUtilsStateEnum2String(pSecAgreeServer->eState)));
		return RV_ERROR_ILLEGAL_ACTION;
	}

	if (pSecAgreeServer->securityData.selectedSecurity.eType != RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED)
	{
		/* Nothing to do if a mechanism is already set to the security-object */
		return RV_OK;
	}

	/* Forcing ipsec is allowed only if ipsec-3gpp is the only mechanism that was offered to the remote party */
	rv = SecAgreeUtilsIsMechanismInList(pSecAgreeServer->securityData.hLocalSecurityList, 
										RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP, NULL, NULL, &bIsSingle);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
				   "SecAgreeServerForceIpsecSecurity - Server security-agreement 0x%p: Failed to find ipsec-3gpp mechanism in list",
				   pSecAgreeServer));
		return rv;
	}
	if (RV_FALSE == bIsSingle)
	{
		RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
				   "SecAgreeServerForceIpsecSecurity - Server security-agreement 0x%p: Forcing ipsec-3gpp is enabled only when it is the only local security mechanism",
				   pSecAgreeServer));
		return RV_ERROR_ILLEGAL_ACTION;
	}
		
	/* Set the ipsec mechanism to the server security-agreement */
	pSecAgreeServer->securityData.selectedSecurity.eType = RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP;

	/* Update the security object on choice of mechanism. */
	rv = SecAgreeObjectUpdateSecObj(pSecAgreeServer, SEC_AGREE_PROCESSING_SET_SECURITY);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
				   "SecAgreeServerForceIpsecSecurity - Server security-agreement=0x%p: Failed to set mechanism to security object, rv=%d",
				   pSecAgreeServer, rv));
		pSecAgreeServer->securityData.selectedSecurity.eType = RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED;
	}

	return rv;
}
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */


/*------------------------------------------------------------------------
           S T A T I C   F U N C T I O N S
 -------------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeServerMsgRcvdReplaceObjectIfNeeded
 * ------------------------------------------------------------------------
 * General: Check if there are Security-Client headers in the message. If there
 *          are, check that there is a server security-agreement object with initial state
 *          ready to handle them. If there is a client security-agreement with illegal state,
 *          make it invalid. Also, ask the application to supply a new security-
 *          client object to handle the new Security-Server list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr     - Pointer to the security manager
 *         hMsg             - Handle to the received message
 *         pOwner           - Owner details to supply the application.
 *         pOwnerLock       - The triple lock of the owner, to open before callback
 * Inout:  ppSecAgreeServer - Pointer to the server security-agreement. If a new server security-agreement
 *                            is obtained, it will replace the current one when function
 *                            returns.
 * Output: phClientMsgListElem - Handle to the list element of the first security-client header,
 *                            if exists. This way when building the list we can avoid
 *                            repeating the search for the first header
 *         phClientHeader      - Pointer to the first security-client header, if exists.
 *         phVerMsgListElem - Handle to the list element of the first security-client header,
 *							  if exists. This way when building the list we can avoid
 *                            repeating the search for the first header
 *         phVerHeader      - Pointer to the first security-client header, if exists.
 ***************************************************************************/
static RvStatus RVCALLCONV SecAgreeServerMsgRcvdReplaceObjectIfNeeded(
							 IN     SecAgreeMgr*               pSecAgreeMgr,
							 IN     RvSipMsgHandle             hMsg,
							 IN     RvSipSecAgreeOwner*        pOwner,
							 IN     SipTripleLock*             pOwnerLock,
							 INOUT  SecAgree**                 ppSecAgreeServer,
							 OUT    RvSipHeaderListElemHandle* phClientMsgListElem,
							 OUT    RvSipSecurityHeaderHandle* phClientHeader,
							 OUT    RvSipHeaderListElemHandle* phVerMsgListElem,
							 OUT    RvSipSecurityHeaderHandle* phVerHeader)
{
	SecAgree*  pSecAgreeServer    = (SecAgree*)*ppSecAgreeServer;
	RvBool     bHasSecurityClient = RV_FALSE;
	RvBool     bHasSecurityVerify = RV_FALSE;
	RvBool     bShouldReplace     = RV_FALSE; 
	RvBool     bIsLegal           = RV_TRUE;
	RvStatus   rv;

	/* check if there are Security-Client headers in the message */
	if (RV_TRUE == SecAgreeMsgHandlingServerIsThereSecurityClientListInMsg(hMsg, phClientMsgListElem, phClientHeader))
		
	{
		/* there are Security-Client headers in the message */
		/* Check if the server security-agreement is ready to handle incoming Security-Client list */
		if (RV_FALSE == SecAgreeStateCanLoadRemoteSecList(pSecAgreeServer) &&
			RV_FALSE == SecAgreeStateShouldCompareRemoteSecList(pSecAgreeServer))
		{
			/* the current state of the server will not allow it to handle incoming Security-Client list
		   (load it or compare it to previously loaded one) */
			bShouldReplace     = RV_TRUE;
			bHasSecurityClient = RV_TRUE;
		}
	}

	/* check if there are Security-Verify headers in the message */
	if (RV_TRUE == SecAgreeMsgHandlingServerIsThereSecurityVerifyListInMsg(hMsg, phVerMsgListElem, phVerHeader))
	{
		/* there are Security-Verify headers in the message */
		/* Check if the server security-agreement is ready to handle incoming Security-Verify list */
		if (RV_FALSE == SecAgreeStateShouldCompareLocalSecList(pSecAgreeServer, NULL))
		{
			/* the current state of the server will not allow it to compare incoming Security-Verify list */
			bShouldReplace     = RV_TRUE;
			bHasSecurityVerify = RV_TRUE;
		}
	}
	
	if (bShouldReplace == RV_FALSE)
	{
		/* No need to replace security-agreement */
		return RV_OK;
	}

	/* replace the current security-agreement object with a new one */
	rv = SecAgreeServerMsgRcvdReplaceObject(pSecAgreeMgr, hMsg, pOwner, pOwnerLock,
											RVSIP_SEC_AGREE_REASON_SECURITY_CLIENT_RCVD, 
											ppSecAgreeServer);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeServerMsgRcvdReplaceObjectIfNeeded - Failed to replace object, rv=%d",
					rv));
		return rv;
	}

	pSecAgreeServer = *ppSecAgreeServer;

	if (NULL != pSecAgreeServer &&
		RVSIP_SEC_AGREE_ROLE_SERVER != pSecAgreeServer->eRole)
	{
		/* we demand that the returned security-agreement would be server */
		bIsLegal = RV_FALSE;
	}
	else if (NULL != pSecAgreeServer &&
			 RV_TRUE  == bHasSecurityClient &&
			 RV_FALSE == SecAgreeStateCanLoadRemoteSecList(pSecAgreeServer) &&
		     RV_FALSE == SecAgreeStateShouldCompareRemoteSecList(pSecAgreeServer))
	{
		/* The message has security-client list but the security-agreement cannot handle it */
		bIsLegal = RV_FALSE;
	}
	else if (NULL != pSecAgreeServer &&
			 RV_TRUE == bHasSecurityVerify &&
			 RV_FALSE == SecAgreeStateShouldCompareLocalSecList(pSecAgreeServer, NULL))
	{
		/* The message has security-verify list but the security-agreement cannot handle it */
		bIsLegal = RV_FALSE;
	}

	/* check the state of the new server security-agreement that was supplied by the application */
	if (bIsLegal == RV_FALSE)
	{
		/* Invalidate the security-agreement */
		SecAgreeStateInvalidate(pSecAgreeServer, RVSIP_SEC_AGREE_REASON_UNDEFINED);
		/* The application supplied invalid client security-agreement object */
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeServerMsgRcvdReplaceObjectIfNeeded - The application supplied new server security-agreement=0x%p in un initial state=%s for owner=0x%p",
					pSecAgreeServer, SecAgreeUtilsStateEnum2String(pSecAgreeServer->eState), pOwner->pOwner));
		return RV_ERROR_UNKNOWN;
	}

	return RV_OK;
}

/***************************************************************************
 * SecAgreeServerMsgRcvdReplaceObject
 * ------------------------------------------------------------------------
 * General: Change the state of the current server security-agreement to INVALID
 *          and request a new server security-agreement from the application
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr     - Pointer to the security manager
 *         hMsg             - Handle to the received message
 *         pOwner           - Owner details to supply the application.
 *         pOwnerLock       - The triple lock of the owner, to open before callback
 *         eReason          - The reason for requesting a new security-agreement
 * Output: ppSecAgreeServer - Pointer to the server security-agreement. If a new server security-agreement
 *                            is obtained, it will replace the current one when function
 *                            returns.
 ***************************************************************************/
static RvStatus RVCALLCONV SecAgreeServerMsgRcvdReplaceObject(
							 IN     SecAgreeMgr*               pSecAgreeMgr,
							 IN     RvSipMsgHandle             hMsg,
							 IN     RvSipSecAgreeOwner*        pOwner,
							 IN     SipTripleLock*             pOwnerLock,
							 IN     RvSipSecAgreeReason        eReason,
							 INOUT  SecAgree**                 ppSecAgreeServer)
{
	SecAgree*  pSecAgreeServer    = (SecAgree*)*ppSecAgreeServer;
	SecAgree*  pNewSecAgreeServer = NULL;
	RvStatus   rv;

	if (NULL != pSecAgreeServer)
	{
		/* There is a current server security-agreement object to this owner and either
		   (1) it is invalid already or (2) its state indicates it will become invalid due
		   to the receipt of a security-client list.
		   First we request a new server security-agreement object from the application, and then we change
		   the state of the current one to INVALID (if needed) */
		if (RV_TRUE == SecAgreeStateIsValid(pSecAgreeServer))
		{
			/* The old server security-agreement object becomes INVALID */
			RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeServerMsgRcvdReplaceObject - server security-agreement=0x%p: become INVALID in state %s, reason=%s, message=0x%p",
					pSecAgreeServer, SecAgreeUtilsStateEnum2String(pSecAgreeServer->eState), SecAgreeUtilsReasonEnum2String(eReason), hMsg));
			SecAgreeStateInvalidate(pSecAgreeServer, eReason);
		}
		/* We unlock the server security-agreement at this point. We do not refer to it after this point */
		SecAgreeUnLockAPI(pSecAgreeServer);	
		/* reset the new server security-agreement until a new one will be supplied */
		*ppSecAgreeServer = NULL;
	}


	RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
			 "SecAgreeServerMsgRcvdReplaceObject - current server security-agreement=0x%p; requesting the application to supply new server security-agreement for Owner=0x%p",
			 pSecAgreeServer, pOwner->pOwner));

	/* The application is asked to supply a server security-agreement object, since the client
		requested the beginning of a negotiation */
	rv = SecAgreeCallbacksRequiredEv(pSecAgreeMgr,
									 pOwner, 
									 pOwnerLock,
									 hMsg,
									 pSecAgreeServer,
									 RVSIP_SEC_AGREE_ROLE_SERVER, 
									 &pNewSecAgreeServer);
	if (RV_OK != rv)
	{
		/* the application is responsible to attach the new security object to the old owners */
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeServerMsgRcvdReplaceObject - The application returned rv=%d from callback EvSecurityAgreementRequired, owner=0x%p",
					rv, pOwner->pOwner));
		return rv;
	}

	RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				"SecAgreeServerMsgRcvdReplaceObject - Application supplied new server security-agreement=0x%p for owner=0x%p",
				pNewSecAgreeServer, pOwner->pOwner));

	/* lock the new server security-agreement */
	if (NULL != pNewSecAgreeServer)
	{
		rv = SecAgreeLockAPI(pNewSecAgreeServer);
		if(rv != RV_OK)
		{
			return RV_ERROR_INVALID_HANDLE;
		}
	}

	*ppSecAgreeServer = pNewSecAgreeServer;

	return RV_OK;
}

/***************************************************************************
 * SecAgreeServerMsgRcvdHandleSecurityHeaders
 * ------------------------------------------------------------------------
 * General: Handles the received Security-Client and Security-Verify lists
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecAgreeServer  - Pointer to the server security-agreement
 *          hMsg             - Handle to the received message
 * Output:  pbHasSecVerify   - Indication to whether there are security-verify headers in
 *                                the message.
 *          pbIsVerified     - Pointer to the server security-agreement. If a new client security-agreement
 *                             is obtained, it will replace the current one when function
 *                             returns.
 ***************************************************************************/
static RvStatus RVCALLCONV SecAgreeServerMsgRcvdHandleSecurityHeaders(
							 IN     RvSipMsgHandle             hMsg,
							 IN     RvSipSecAgreeOwner*        pOwner,
							 IN     SipTripleLock*             pOwnerLock,
							 IN     RvSipHeaderListElemHandle  hCltMsgListElem,
							 IN     RvSipSecurityHeaderHandle  hCltHeader,
							 IN     RvSipHeaderListElemHandle  hVerMsgListElem,
							 IN     RvSipSecurityHeaderHandle  hVerHeader,
							 INOUT  SecAgree*                 *ppSecAgreeServer,
							 OUT    RvBool                    *pbSecAgreeClientLoaded,
							 OUT    RvBool                    *pbHasSecVerify)
{
	RvBool       bHasVerify       = RV_FALSE;
	RvBool       bHasClient       = RV_FALSE;
	RvBool       bIsEqualVerify   = RV_FALSE;
	RvBool       bIsEqualClient   = RV_FALSE;
	RvBool       bIsTispan        = RV_FALSE;
	RvBool       bCompareVerify   = RV_FALSE;
	RvBool       bCompareClient   = RV_FALSE;
	SecAgree    *pSecAgreeServer  = *ppSecAgreeServer;
	SecAgreeMgr *pSecAgreeMgr     = pSecAgreeServer->pSecAgreeMgr;
	RvStatus	 rv;

	*pbSecAgreeClientLoaded = RV_FALSE;
	
	/* Check the need of comparing Security-Client list and Security-Verify list based on Sec-Agree state and
	   other parameters */
	bCompareVerify = SecAgreeStateShouldCompareLocalSecList(pSecAgreeServer, &bIsTispan);
	bCompareClient = SecAgreeStateShouldCompareRemoteSecList(pSecAgreeServer);

	/* Compare the remote security list with the Security-Client list received */
	rv = SecAgreeMsgHandlingServerCheckSecurityClientList(pSecAgreeServer, hMsg, hCltMsgListElem, hCltHeader, &bIsEqualClient, &bHasClient);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeServerMsgRcvdHandleSecurityHeaders - server security-agreement=0x%p: Failed to compare security-client headers from message=0x%p, rv=%d",
					pSecAgreeServer, hMsg, rv));
		SecAgreeStateInvalidate(pSecAgreeServer, RVSIP_SEC_AGREE_REASON_UNDEFINED);
		return rv;
	}

	/* Compare the local security list with the Security-Verify list received */
	rv = SecAgreeMsgHandlingServerCheckSecurityVerifyList(pSecAgreeServer, hMsg, hVerMsgListElem, hVerHeader, bIsTispan, &bIsEqualVerify, &bHasVerify);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeServerMsgRcvdHandleSecurityHeaders - server security-agreement=0x%p: Failed to compare security-verify headers from message=0x%p, rv=%d",
					pSecAgreeServer, hMsg, rv));
		SecAgreeStateInvalidate(pSecAgreeServer, RVSIP_SEC_AGREE_REASON_UNDEFINED);
		return rv;
	}

	*pbHasSecVerify         = bHasVerify;

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
/* If the given hMsg contains a new security-agreement which is different than the existing security-agreement used 
to protect the message, then we let the application to decide whether it wants to replace the existing 
security-agreement with the new offerred one.
  */

  /*
  dprintf("%s:%d: 111.111: pSecAgreeServer=%p, bHasClient=%d, bCompareClient=%d, bIsEqualClient=%d, bHasVerify=%d, bCompareVerify =%d, bIsEqualVerify=%d\n",__FUNCTION__,__LINE__,
    (int)pSecAgreeServer, (int)bHasClient, (int)bCompareClient, (int)bIsEqualClient, 
    (int)bHasVerify, (int)bCompareVerify, (int)bIsEqualVerify);
    */

  if(pSecAgreeServer && 
    (bHasVerify || SecAgreeUtilsIsGmInterfaceSupport(pSecAgreeServer)) && 
    bCompareVerify) 
  {
    RvBool      bReplace       = RV_TRUE;
    SecAgree*   pOldSecAgree   = NULL;
    
    RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
      "SecAgreeServerMsgRcvdHandleSecurityHeaders - detected new security-agreements in message=0x%p. existing sec-agree=%p",
					 hMsg, pSecAgreeServer));
    
    rv = SecAgreeCallbacksReplaceSecAgreeEv(pSecAgreeMgr, pOwner, pOwnerLock, hMsg, 
      pSecAgreeServer, pSecAgreeServer->eRole, &bReplace, &pOldSecAgree);

    if(rv<0) return rv;

    if(!bReplace)
    {     
      RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					   "SecAgreeServerMsgRcvdHandleSecurityHeaders - no replacement is required for security-agreement=%p",
             pSecAgreeServer));
      return RV_OK;
    }
    else
    {
      if(bCompareVerify && !bIsEqualVerify && pOldSecAgree) {
        
        if(pOldSecAgree == pSecAgreeServer) {
          
          if(pSecAgreeServer->securityData.hSecObj) {
            dprintf("%s:%d: old sec agree legal usage1: remIP=%s\n",__FUNCTION__,__LINE__,((SecObj*)pSecAgreeServer->securityData.hSecObj)->strRemoteIp);
          } else {
            dprintf("%s:%d: old sec agree legal usage1: remIP=NULL\n",__FUNCTION__,__LINE__);
          }
          
          return RV_OK;
          
        } else {

          RvStatus                  rv        = RV_OK;
          RvBool                    bAreEqual = RV_TRUE;
          RvSipSecurityHeaderHandle hSecurityHeader;
          RvInt                     elementType;
          RvSipCommonListElemHandle hSecurityListElem;
          
          rv = SecAgreeServerCheckSecurityVerifyList(pOldSecAgree, pSecAgreeServer, &bIsEqualVerify);
          
          if(rv<0) return rv;

          if(bIsEqualVerify) {
            if(pSecAgreeServer->securityData.hSecObj) {
              dprintf("%s:%d: old sec agree legal usage2: remIP=%s (old %p, new %p)\n",__FUNCTION__,__LINE__,((SecObj*)pSecAgreeServer->securityData.hSecObj)->strRemoteIp,pOldSecAgree, pSecAgreeServer);
            } else {
              dprintf("%s:%d: old sec agree legal usage2: remIP=NULL (old %p, new %p)\n",__FUNCTION__,__LINE__,pOldSecAgree, pSecAgreeServer);
            }
            *ppSecAgreeServer=pOldSecAgree;
            return RV_OK;
          }
        }
        
        RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
          "SecAgreeServerMsgRcvdHandleSecurityHeaders - application wanted to replace the existing security-agreement=%p, old security-agreement=%p",
          pSecAgreeServer, pOldSecAgree));
      }
    }
  }
#endif
  /* SPIRENT_END */

	/* check if the current state requires comparing the security-client 
	   list to the remote security list */
	if (bCompareClient == RV_TRUE)
	{
		/* Check if the remote security list is equivalent to the security-client list */
		if (bIsEqualClient == RV_FALSE)
		{
			/* Since the security-client list does not match, we will invalidate the security-agreement.
			   However, we must first check whether the security-verify headers matched or not. This is
			   important since a failure in security-verify matching indicates a security failure and 
			   must be handled as such. Note that is there are security-client headers there is no obligation
			   for security-verify headers, unless it is TISPAN */
			if (bCompareVerify == RV_TRUE && bIsEqualVerify == RV_FALSE &&
				(bHasVerify == RV_TRUE || SecAgreeUtilsIsGmInterfaceSupport(pSecAgreeServer)))
			{
				/* The Security-Verify lists do not match */
				if (bHasVerify == RV_TRUE)
				{
					RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
							  "SecAgreeServerMsgRcvdHandleSecurityHeaders - Server security-agreement=0x%p: Security-Verify list in message=0x%p does not match local security list",
							  pSecAgreeServer, hMsg));
				}
				else
				{
					RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
							  "SecAgreeServerMsgRcvdHandleSecurityHeaders - Server security-agreement=0x%p: Security-Verify list missing from message=0x%d",
							  pSecAgreeServer, hMsg));
				}
				SecAgreeStateInvalidate(pSecAgreeServer, RVSIP_SEC_AGREE_REASON_SECURITY_VERIFY_MISMATCH);
				return RV_OK;
			}
			else
			{
				/* The Security-Client lists do not match */
				RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
						  "SecAgreeServerMsgRcvdHandleSecurityHeaders - Server security-agreement=0x%p: Security-Client list in message=0x%p does not match remote security list",
						  pSecAgreeServer, hMsg));
				/* There is no more need to compare Security-Verify headers */
				bCompareVerify = RV_FALSE;
				SecAgreeStateInvalidate(pSecAgreeServer, RVSIP_SEC_AGREE_REASON_SECURITY_CLIENT_MISMATCH);
			}
		}
		else
		{
			if (bHasClient == RV_TRUE)
			{
				RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
							"SecAgreeServerMsgRcvdHandleSecurityHeaders - Server security-agreement=0x%p: Security-Client lists match",
							pSecAgreeServer));
			}
		}
	}
	
	if (RV_TRUE == bHasClient && 
	    ((RV_TRUE == bCompareClient && RV_FALSE == bIsEqualClient) || 
		 (RV_FALSE == bCompareClient && RV_FALSE == SecAgreeStateCanLoadRemoteSecList(pSecAgreeServer))))
	{
		/* replace the current security-agreement object with a new one */
		rv = SecAgreeServerMsgRcvdReplaceObject(pSecAgreeServer->pSecAgreeMgr, hMsg, pOwner, pOwnerLock, 
												RVSIP_SEC_AGREE_REASON_SECURITY_CLIENT_RCVD, 
												ppSecAgreeServer);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
						"SecAgreeServerMsgRcvdHandleSecurityHeaders - Failed to replace object, rv=%d",
						rv));
			return rv;
		}

		pSecAgreeServer = *ppSecAgreeServer;
		
		/* check the state of the new server security-agreement that was supplied by the application */
		if (NULL != pSecAgreeServer &&
			(RVSIP_SEC_AGREE_ROLE_SERVER != pSecAgreeServer->eRole ||
			 RV_FALSE == SecAgreeStateCanLoadRemoteSecList(pSecAgreeServer)))
		{
			/* Invalidate the security-agreement */
			SecAgreeStateInvalidate(pSecAgreeServer, RVSIP_SEC_AGREE_REASON_UNDEFINED);
			/* The application supplied invalid server security-agreement object */
			RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
						"SecAgreeServerMsgRcvdHandleSecurityHeaders - The application supplied new server security-agreement=0x%p in un initial state for owner=0x%p",
						pSecAgreeServer, pOwner->pOwner));
			return RV_ERROR_UNKNOWN;
		}

		if (NULL == pSecAgreeServer)
		{
			return RV_OK;
		}
	}

	if (RV_TRUE == bHasClient && RV_TRUE == SecAgreeStateCanLoadRemoteSecList(pSecAgreeServer))
	{
		/* The sever security-agreement does not have a security-client list.
		   Load the security-client list from the message */
		rv = SecAgreeMsgHandlingServerLoadSecurityClientListFromMsg(pSecAgreeServer, hMsg, hCltMsgListElem, hCltHeader, pbSecAgreeClientLoaded);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
						"SecAgreeServerMsgRcvdHandleSecurityHeaders - server security-agreement=0x%p: Failed to set security-client list to message=0x%p, rv=%d",
						pSecAgreeServer, hMsg, rv));
			SecAgreeStateInvalidate(pSecAgreeServer, RVSIP_SEC_AGREE_REASON_OUT_OF_RESOURCES);
			return rv;
		}
		
		/* The transaction from which the security-client list was loaded, is the transaction on which
		   the security-server list should be returned */
		pSecAgreeServer->securityData.cseqStep = SecAgreeUtilsGetCSeqStep(hMsg);
	}

	/* Check if the state requires comparison of local security list */
	if (RV_FALSE == bCompareVerify)
	{	
		if (RV_TRUE == bHasVerify)
		{
			/* An unexpected Security-Verify list was received */
			RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
						"SecAgreeServerMsgRcvdHandleSecurityHeaders - server security-agreement=0x%p: received Security-Verify list in state %s (message=0x%p). Security-Verify list is ignored",
						pSecAgreeServer, SecAgreeUtilsStateEnum2String(pSecAgreeServer->eState), hMsg));
		}
		return RV_OK;
	}

	if (bIsEqualVerify == RV_TRUE)
	{
		RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				  "SecAgreeServerMsgRcvdHandleSecurityHeaders - Server security-agreement=0x%p: Security-Verify list matches local security list",
				  pSecAgreeServer));
	}
	else
	{
		RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				  "SecAgreeServerMsgRcvdHandleSecurityHeaders - Server security-agreement=0x%p: Security-Verify list does not match local security list",
			       pSecAgreeServer));
		SecAgreeStateInvalidate(pSecAgreeServer, RVSIP_SEC_AGREE_REASON_SECURITY_VERIFY_MISMATCH);
	}

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecAgreeMgr);
#endif

	return RV_OK;

}

/***************************************************************************
 * SecAgreeServerMsgRcvdUpdateRemoteAddr
 * ------------------------------------------------------------------------
 * General: Updates the remote address (for ipsec) in the security-agreement,
 *          and notifies the security object about it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecAgreeServer  - Pointer to the server security-agreement
 *          pRcvdFromAddr    - The address the message was received from
 ***************************************************************************/
static RvStatus RVCALLCONV SecAgreeServerMsgRcvdUpdateRemoteAddr(
							 IN    SecAgree*                      pSecAgreeServer,
							 IN    SipTransportAddress*           pRcvdFromAddr)
{
	RvStatus     rv;

#if (RV_IMS_IPSEC_ENABLED == RV_YES)
	if (RVSIP_TRANSPORT_UDP == pRcvdFromAddr->eTransport ||
		RVSIP_TRANSPORT_TCP == pRcvdFromAddr->eTransport)
	{
		/* Check if we already have a remote address in the security-agreement object */
		if (pSecAgreeServer->addresses.remoteIpsecAddr.addrtype == 0 ||
			pSecAgreeServer->addresses.eRemoteIpsecTransportType == RVSIP_TRANSPORT_UNDEFINED)
		{
			RvAddress   *pRes;
			/* We do not have a remote address set to the security-agreement. 
			   Store the received from address as the remote address for ipsec usage */
			pRes = RvAddressCopy(&pRcvdFromAddr->addr, &pSecAgreeServer->addresses.remoteIpsecAddr);		
			if (pRes == NULL)
			{
				RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
							"SecAgreeServerMsgRcvdUpdateRemoteAddr - server security-agreement=0x%p: Failed to copy received from address",
							pSecAgreeServer));
				return RV_ERROR_UNKNOWN;
			}
			pSecAgreeServer->addresses.eRemoteIpsecTransportType = pRcvdFromAddr->eTransport;
		}
	}
#else /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */
	RV_UNUSED_ARG(pRcvdFromAddr);
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */
    
	/* Update the security object on the receipt message. The security object will initiate
	   and activate if needed. */
	rv = SecAgreeObjectUpdateSecObj(pSecAgreeServer, SEC_AGREE_PROCESSING_MSG_RCVD);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
				   "SecAgreeServerMsgRcvdUpdateRemoteAddr - Server security-agreement=0x%p: Failed to init security object, rv=%d",
				   pSecAgreeServer, rv));
		return rv;
	}

	return RV_OK;
}

/***************************************************************************
 * SecAgreeServerMsgRcvdUpdateSecurityMechanism
 * ------------------------------------------------------------------------
 * General: Updates the chosen security mechanism in the security-agreement,
 *          and notifies the security object about it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecAgreeServer  - Pointer to the server security-agreement
 *          hLocalAddr       - The address the message was received on
 *          pRcvdFromAddr    - The address the message was received from
 *          hConn            - The connection the message was received on
 ***************************************************************************/
static RvStatus RVCALLCONV SecAgreeServerMsgRcvdUpdateSecurityMechanism(
							 IN    SecAgree*                      pSecAgreeServer,
							 IN    RvSipTransportLocalAddrHandle  hLocalAddr,
							 IN    SipTransportAddress*           pRcvdFromAddr,
							 IN	   RvSipTransportConnectionHandle hConn)
{
	void						*pSecObj;
	RvStatus					 rv;

	if (pSecAgreeServer->eState != RVSIP_SEC_AGREE_STATE_ACTIVE)
	{
		/* Nothing to do if the security-agreement is not in the ACTIVE state */
		return RV_OK;
	}

	if (pSecAgreeServer->securityData.selectedSecurity.eType != RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED)
	{
		/* Nothing to do if a security mechanism was already set */
		return RV_OK;
	}
	
	if (pSecAgreeServer->securityData.hSecObj != NULL)
	{
		if (RVSIP_TRANSPORT_UDP == pRcvdFromAddr->eTransport ||
			RVSIP_TRANSPORT_TCP == pRcvdFromAddr->eTransport)
		{
			/* Possible Ipsec. We try getting the security object from the local address to figure if ipsec
			   was used */
			rv = RvSipTransportMgrLocalAddressGetSecOwner(hLocalAddr, &pSecObj);
			if (RV_OK != rv)
			{
				RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
							"SecAgreeServerMsgRcvdUpdateSecurityMechanism - Server security-agreement=0x%p: Failed to check local address, rv=%d",
							pSecAgreeServer, rv));
				return rv;
			}
			if (NULL != pSecObj)
			{
				/* Mark that the client chose ipsec */
				pSecAgreeServer->securityData.selectedSecurity.eType = RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP;
			}
			else
			{
				/* Mark that the client chose digest */
				pSecAgreeServer->securityData.selectedSecurity.eType = RVSIP_SECURITY_MECHANISM_TYPE_DIGEST;
			}
		}
		else if (RVSIP_TRANSPORT_TLS == pRcvdFromAddr->eTransport)
		{
#if (RV_TLS_TYPE != RV_TLS_NONE)
			/* Set the TLS connection to the security object */
			rv = SipSecObjSetTlsConnection(pSecAgreeServer->pSecAgreeMgr->hSecMgr,
											pSecAgreeServer->securityData.hSecObj, hConn);
			if (RV_OK != rv)
			{
				RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
							"SecAgreeServerMsgRcvdUpdateSecurityMechanism - Server security-agreement=0x%p: Failed to set TLS connection to security object 0x%p, rv=%d",
							pSecAgreeServer, pSecAgreeServer->securityData.hSecObj, rv));
				return rv;
			}
			/* Mark that the client chose tls */
			pSecAgreeServer->securityData.selectedSecurity.eType = RVSIP_SECURITY_MECHANISM_TYPE_TLS;
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/
		}
	}
	else
	{
		/* Mark that the client chose digest */
		pSecAgreeServer->securityData.selectedSecurity.eType = RVSIP_SECURITY_MECHANISM_TYPE_DIGEST;
	}

	/* Update the security object on choice of mechanism. */
  /*  SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
	rv = SecAgreeObjectUpdateSecObj(pSecAgreeServer, SEC_AGREE_PROCESSING_MSG_RCVD);
#else
  	rv = SecAgreeObjectUpdateSecObj(pSecAgreeServer, SEC_AGREE_PROCESSING_SET_SECURITY);
#endif
  /* SPIRENT_END */
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
				   "SecAgreeServerMsgRcvdUpdateSecurityMechanism - Server security-agreement=0x%p: Failed to init security object, rv=%d",
				   pSecAgreeServer, rv));
		return rv;
	}

	RV_UNUSED_ARG(hConn);

	return RV_OK;
}

/***************************************************************************
 * ServerShouldProcessRequestMsg
 * ------------------------------------------------------------------------
 * General: Checks the method and the response code of the message to determine
 *          if it should be processed by the server
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg             - Handle to the message
 ***************************************************************************/
static RvBool RVCALLCONV ServerShouldProcessRequestMsg(IN     RvSipMsgHandle      hMsg)
{
	return SecAgreeUtilsIsMethodForSecAgree(hMsg);
}

/***************************************************************************
 * ServerShouldProcessRequestMsg
 * ------------------------------------------------------------------------
 * General: Checks the method and the response code of the message to determine
 *          if it should be processed by the server
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg             - Handle to the message
 ***************************************************************************/
static RvBool RVCALLCONV CanResponseBePartOfNegotiation(IN     RvSipMsgHandle      hMsg)
{
	RvInt16               statusCode;
	
	/* Check status code */
	statusCode = RvSipMsgGetStatusCode(hMsg);
	if (statusCode < 400 || statusCode > 499 || statusCode == 423)
	{
		/* We do not insert security information to those responses */
		return RV_FALSE;
	}

	return RV_TRUE;
}

#endif /* #ifdef RV_SIP_IMS_ON */



