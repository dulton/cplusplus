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
 *                              <SecAgreeMsgHandling.c>
 *
 * This file defines methods for handling messages: loading security infomration
 * from message, loading security information onto messages, etc.
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

#include "SecAgreeMsgHandling.h"
#include "SecAgreeTypes.h"
#include "SecAgreeSecurityData.h"
#include "_SipAuthenticator.h"
#include "RvSipSecurityHeader.h"
#include "_SipSecurityHeader.h"
#include "RvSipOtherHeader.h"
#include "_SipOtherHeader.h"
#include "SecAgreeUtils.h"
#include "SecAgreeObject.h"
#include "_SipCommonConversions.h"

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV AddSecurityListToMsg(
									IN    RvSipCommonListHandle   hList,
									IN    RvSipMsgHandle          hMsg);

static RvBool RVCALLCONV IsThereSecurityListInMsg(
									IN    RvSipMsgHandle             hMsg,
									IN    RvSipSecurityHeaderType    eSecurityHeaderType,
									OUT   RvSipHeaderListElemHandle* phMsgListElem,
									OUT   RvSipSecurityHeaderHandle* phHeader);

static RvStatus RVCALLCONV LoadRemoteSecurityListFromMsg(
									IN    SecAgreeMgr*               pSecAgreeMgr,
									IN    HPAGE                      hPage,
									IN    RvSipMsgHandle             hMsg,
									IN    RvSipSecAgreeRole          eRole,
									IN    RvSipHeaderListElemHandle  hPrevMsgListElem,
									IN    RvSipSecurityHeaderHandle  hPrevHeader,
									INOUT SecAgreeSecurityData*              pSecAgreeSecurityData,
									OUT   RvBool*                    pbIsDigest);

static RvStatus RVCALLCONV SecAgreeCheckList(
									IN    RvSipCommonListHandle      hSecurityList,
									IN    RvSipMsgHandle             hMsg,
									IN    RvSipHeaderListElemHandle  hMsgListElem,
									IN    RvSipSecurityHeaderHandle  hMsgSecurityHeader,
									IN    RvSipSecurityHeaderType    eHeaderType,
									OUT   RvBool*                    pbHasMsgList,
									OUT   RvBool*                    pbAreEqual);
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
static RvStatus RVCALLCONV SecAgreeCheckLists(
									IN    RvSipCommonListHandle      hSecurityList1,
									IN    RvSipCommonListHandle      hSecurityList2,
									IN    RvSipSecurityHeaderType    eHeaderType,
									OUT   RvBool*                    pbAreEqual);
#endif
/* SPIRENT_END */

static RvBool RVCALLCONV IsSecAgreeInMsg(IN  RvSipMsgHandle  hMsg,
										 IN  RvChar*         strHeaderName);

static RvBool RVCALLCONV IsSecAgreeInHeader(
										IN    RvSipOtherHeaderHandle  hHeader);

static RvBool RVCALLCONV IsRequireHeader(IN    RvSipOtherHeaderHandle  hHeader);

static void RVCALLCONV FindFirstProxyAuthenticate(
									IN    RvSipMsgHandle                   hMsg,
									OUT   RvSipAuthenticationHeaderHandle* phProxyAuth);

static RvSipSecurityHeaderHandle RVCALLCONV GetSecurityHeader(
									IN    RvSipMsgHandle			   hMsg,
									IN    RvSipSecurityHeaderType      eHeaderType,
									IN    RvSipHeadersLocation         location,
									INOUT RvSipHeaderListElemHandle*   phListElem);

/*-----------------------------------------------------------------------*/
/*                         SECURITY AGREEMENT FUNCTIONS                  */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeMsgHandlingSetRequireHeadersToMsg
 * ------------------------------------------------------------------------
 * General: Adds Require sec-agree and Proxy-Require sec-agree to message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecurity    - Pointer to the security-agreement
 *         hMsg         - Handle to the message to write on
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingSetRequireHeadersToMsg(
									IN    SecAgree*            pSecAgree,
									IN    RvSipMsgHandle       hMsg)
{
	RvSipOtherHeaderHandle   hHeader;
	RvStatus                 rv;

	rv = RvSipOtherHeaderConstructInMsg(hMsg, RV_FALSE, &hHeader);
	if (RV_OK == rv)
	{
		rv = RvSipOtherHeaderSetName(hHeader, REQUIRE_STRING);
		if (RV_OK == rv)
		{
			rv = RvSipOtherHeaderSetValue(hHeader, SEC_AGREE_STRING);
		}
	}

	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				   "SecAgreeMsgHandlingSetRequireHeadersToMsg - Security-agreement=0x%p: Failed to set Require header to message 0x%p, rv=%d",
				   pSecAgree, hMsg, rv));
		return rv;
	}

	rv = RvSipOtherHeaderConstructInMsg(hMsg, RV_FALSE, &hHeader);
	if (RV_OK == rv)
	{
		rv = RvSipOtherHeaderSetName(hHeader, PROXY_REQUIRE_STRING);
		if (RV_OK == rv)
		{
			rv = RvSipOtherHeaderSetValue(hHeader, SEC_AGREE_STRING);
		}
	}

	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				   "SecAgreeMsgHandlingSetRequireHeadersToMsg - Security-agreement=0x%p: Failed to set Proxy-Require header to message 0x%p, rv=%d",
				   pSecAgree, hMsg, rv));
		return rv;
	}

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecAgree);
#endif

	return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                         SECURITY CLIENT FUNCTIONS                     */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeMsgHandlingClientSetSecurityClientListToMsg
 * ------------------------------------------------------------------------
 * General: Adds the list of local security mechanisms as Security-Client
 *          headers to the message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecurityClient    - Pointer to the client security-agreement
 *         hMsg               - Handle to the message to write on
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingClientSetSecurityClientListToMsg(
									IN    SecAgree*            pSecAgreeClient,
									IN    RvSipMsgHandle       hMsg)
{
	RvStatus               rv    = RV_OK;
	RvSipCommonListHandle  hList = pSecAgreeClient->securityData.hLocalSecurityList;

	if (hList == NULL)
	{
		return RV_OK;
	}

	RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
			   "SecAgreeMsgHandlingClientSetSecurityClientListToMsg - Client security-agreement=0x%p: inserting Security-Client list to message=0x%p",
			   pSecAgreeClient, hMsg));

	/* The local security list is the list of security-client headers. Add it to the message */
	rv = AddSecurityListToMsg(hList, hMsg);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
				   "SecAgreeMsgHandlingClientSetSecurityClientListToMsg - Failed to add Security-Client list to message. client security-agreement=0x%p, rv=%d",
				   pSecAgreeClient, rv));
	}

	return rv;
}

/***************************************************************************
 * SecAgreeMsgHandlingClientCleanMsg
 * ------------------------------------------------------------------------
 * General: Remove Require, Proxy-Require and Security-Client headers that
 *          were previously added to the message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecurityClient    - Pointer to the client security-agreement
 *         hMsg               - Handle to the message to write on
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingClientCleanMsg(
									IN    SecAgree*            pSecAgreeClient,
									IN    RvSipMsgHandle       hMsg)
{
	RvSipHeaderListElemHandle  hListElem;
	RvSipHeaderListElemHandle  hNextListElem;
	void*                      hHeader;
	void*                      hNextHeader;
	RvSipHeaderType            eHeaderType;
	RvSipHeaderType            eNextHeaderType;
	RvInt                      safeCounter = 0;
	RvStatus                   rv;

	hHeader = RvSipMsgGetHeader(hMsg, RVSIP_FIRST_HEADER, &hListElem, &eHeaderType);
	while (hHeader != NULL && safeCounter < SEC_AGREE_MAX_LOOP(pSecAgreeClient))
	{
		/* We would not want to start from the beginning each time, therefore we first find the next
		   header and only then delete the previous one */
		hNextListElem = hListElem;
		hNextHeader = RvSipMsgGetHeader(hMsg, RVSIP_NEXT_HEADER, &hNextListElem, &eNextHeaderType);

		if ( /* Remove if this is Security-Client */
			(RVSIP_HEADERTYPE_SECURITY == eHeaderType &&
			 RVSIP_SECURITY_CLIENT_HEADER == RvSipSecurityHeaderGetSecurityHeaderType((RvSipSecurityHeaderHandle)hHeader)) ||
			 /* Remove if this is Require or Proxy-Require */
			 (RVSIP_HEADERTYPE_OTHER == eHeaderType &&
			 RV_TRUE == IsRequireHeader((RvSipOtherHeaderHandle)hHeader)))
		{
			/* We remove only if this is a Security-Client header */
			rv = RvSipMsgRemoveHeaderAt(hMsg, hListElem);
			if (RV_OK != rv)
			{
				RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
							"SecAgreeMsgHandlingClientCleanMsg - Security-Client=0x%p: Failed to remove security-client header from message=0x%p, rv=%d",
							pSecAgreeClient, hMsg, rv));
				return rv;
			}
		}

		hListElem   = hNextListElem;
		hHeader		= hNextHeader;
		eHeaderType = eNextHeaderType;

		safeCounter += 1;
	}

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecAgreeClient);
#endif
	return RV_OK;
}

/***************************************************************************
 * SecAgreeMsgHandlingClientSetSecurityVerifyListToMsg
 * ------------------------------------------------------------------------
 * General: Adds the list of remote security mechanisms as Security-Verify
 *          headers to the message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient    - Pointer to the client security-agreement
 *         hMsg               - Handle to the message to write on
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingClientSetSecurityVerifyListToMsg(
									IN    SecAgree*            pSecAgreeClient,
									IN    RvSipMsgHandle       hMsg)
{
	RvStatus               rv    = RV_OK;
	RvSipCommonListHandle  hList = pSecAgreeClient->securityData.hRemoteSecurityList;

	if (hList == NULL)
	{
		return RV_OK;
	}

	RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
			   "SecAgreeMsgHandlingClientSetSecurityVerifyListToMsg - Client security-agreement=0x%p: inserting Security-Verify list to message=0x%p",
			   pSecAgreeClient, hMsg));

	/* The remote security list is the list of security-verify headers. Add it to the message */
	rv = AddSecurityListToMsg(hList, hMsg);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
				   "SecAgreeMsgHandlingClientSetSecurityVerifyListToMsg - Failed to add Security-Client list to message. client security-agreement=0x%p, rv=%d",
				   pSecAgreeClient, rv));
	}

	return rv;
}

/***************************************************************************
 * SecAgreeMsgHandlingClientIsThereSecurityServerListInMsg
 * ------------------------------------------------------------------------
 * General: Checks whether there are Security-Server headers in the message
 * Return Value: RV_TURE if there is Security-Server header in the message
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg          - Handle to the message
 * Output   phMsgListElem - Handle to the list element of the first security header,
 *                          if exists. This way when building the list we can avoid
 *                          repeating the search for a new
 *          phHeader      - Pointer to the first security header, if exists.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeMsgHandlingClientIsThereSecurityServerListInMsg(
									IN    RvSipMsgHandle             hMsg,
									OUT   RvSipHeaderListElemHandle* phMsgListElem,
									OUT   RvSipSecurityHeaderHandle* phHeader)
{
	return IsThereSecurityListInMsg(hMsg, RVSIP_SECURITY_SERVER_HEADER, phMsgListElem, phHeader);
}

/***************************************************************************
 * SecAgreeMsgHandlingClientLoadSecurityServerListFromMsg
 * ------------------------------------------------------------------------
 * General: Load the list of Security-Server headers from the message
 *          into the client security-agreement object. Notice: each Security-Server
 *          header is stored in the client security-agreement object as Security-Verify
 *          header for insertions to further requests.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient    - Pointer to the client security-agreement
 *         hMsg               - Handle to the message to write on
 *         hMsgListElem       - The first Security-Server header in the message, if
 *                              previously found.
 *         hHeader            - The first Security-Server header in the message, if
 *                              previously found.
 * Output: pbIsSecurityServer - RV_TRUE if there are security-server headers in
 *                              the message, RV_FALSE if there aren't.
 *		   pbIsDigest         - RV_TRUE if there was digest mechanism in one of
 *                              the Security-Server headers. If yes, Proxy-Authenticate
 *                              headers will be copied from the message.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingClientLoadSecurityServerListFromMsg(
									IN    SecAgree*                   pSecAgreeClient,
									IN    RvSipMsgHandle              hMsg,
									IN    RvSipHeaderListElemHandle   hMsgListElem,
									IN    RvSipSecurityHeaderHandle   hHeader,
									OUT   RvBool*                     pbIsSecurityServer,
									OUT   RvBool*                     pbIsDigest)
{
	RvStatus					rv;

	if (hMsgListElem == NULL)
	{
		/* check if there are security-server headers in the message */
		*pbIsSecurityServer = IsThereSecurityListInMsg(hMsg, RVSIP_SECURITY_SERVER_HEADER, &hMsgListElem, &hHeader);

		if (*pbIsSecurityServer == RV_FALSE)
		{
			return RV_OK;
		}
	}
	else
	{
		/* A pointer to the first security-server was received from outside, therefore there must
		   be security-server headers in the message */
		*pbIsSecurityServer = RV_TRUE;
	}

	if (pSecAgreeClient->securityData.hRemoteSecurityList != NULL)
	{
		/* There is already a security-server list in the object. Usually the state would indicate that
	       there was already a remote list set to this object, and we won't reach that point (the state
		   would change to invalid and a new object would be supplied for the received list). This can
		   only happen if the application set a remote list to the object, and no state change took place */
		RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
				  "SecAgreeMsgHandlingClientLoadSecurityServerListFromMsg - client security-agreement=0x%p has a previously set remote security list. This list will be ran over by the list that arrived on the message",
				  pSecAgreeClient));
		SecAgreeSecurityDataDestructSecurityList(&pSecAgreeClient->securityData, SEC_AGREE_SEC_DATA_LIST_REMOTE);
	}

	RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
			  "SecAgreeMsgHandlingClientLoadSecurityServerListFromMsg - Client security-agreement=0x%p loading Security-Server list from message=0x%p",
			  pSecAgreeClient, hMsg));

	/* Construct the remote security list */
	rv = SecAgreeSecurityDataConstructSecurityList(pSecAgreeClient->pSecAgreeMgr, pSecAgreeClient->hPage, &pSecAgreeClient->securityData, SEC_AGREE_SEC_DATA_LIST_REMOTE, NULL);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
					"SecAgreeMsgHandlingClientLoadSecurityServerListFromMsg - Failed to construct remote security list for client security-agreement=0x%p, rv=%d",
					pSecAgreeClient, rv));
		return rv;
	}

	/* Load the list of security-server from the message */
	rv = LoadRemoteSecurityListFromMsg(pSecAgreeClient->pSecAgreeMgr,
									   pSecAgreeClient->hPage,
									   hMsg,
									   RVSIP_SEC_AGREE_ROLE_CLIENT,
									   hMsgListElem,
									   hHeader,
									   &pSecAgreeClient->securityData,
									   pbIsDigest);
	if (RV_OK != rv)
	{
		SecAgreeSecurityDataDestructSecurityList(&pSecAgreeClient->securityData, SEC_AGREE_SEC_DATA_LIST_REMOTE);
		RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
				   "SecAgreeMsgHandlingClientLoadSecurityServerListFromMsg - Failed to copy Security-Server list from message. client security-agreement=0x%p, rv=%d",
				   pSecAgreeClient, rv));
		return rv;
	}

	return RV_OK;
}

/***************************************************************************
 * SecAgreeMsgHandlingClientLoadAuthenticationDataFromMsg
 * ------------------------------------------------------------------------
 * General: Stores the list of Proxy-Authenticate headers that was received
 *          on the response in the client security-agreement object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient    - Pointer to the Security-Manager
 *         hMsg               - Handle to the message to write on
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingClientLoadAuthenticationDataFromMsg(
									IN    SecAgree*            pSecAgreeClient,
									IN    RvSipMsgHandle       hMsg)
{
	RvSipAuthenticationHeaderHandle hProxyAuth;
	RvSipSecurityHeaderHandle       hSecurityHeader;
	RvInt16							status;
	RvSipAuthQopOption              eQop;
	RPOOL_Ptr                       pQop;
	RvSipAuthAlgorithm              eAlgorithm;
	RPOOL_Ptr                       pAlgorithm;
	RvInt32                         akaVersion;
	RvStatus						rv = RV_OK;

	status = RvSipMsgGetStatusCode(hMsg);

	if (RESPONSE_494 != status)
	{
		RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
				  "SecAgreeMsgHandlingClientLoadAuthenticationDataFromMsg - Response %d contains Security-Server with digest. Client security-agreement=0x%p ignores Proxy-Authenticate information",
				  status, pSecAgreeClient));
		return RV_OK;
	}

	RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
			  "SecAgreeMsgHandlingClientLoadAuthenticationDataFromMsg - Client security-agreement=0x%p loading Proxy-Authenticate from message=0x%p",
			  pSecAgreeClient, hMsg));

	/* Find the first Proxy-Authenticate header within the message */
	FindFirstProxyAuthenticate(hMsg, &hProxyAuth);

	if (hProxyAuth == NULL)
	{
		RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
				  "SecAgreeMsgHandlingClientLoadAuthenticationDataFromMsg - Security-agreement 0x%p: Response 0x%p contains Security-Server with digest but no Proxy-Authenticate information",
				  pSecAgreeClient, hMsg));
		return RV_OK;
	}

	/* Find Security-Server header with digest */
	rv = SecAgreeUtilsIsMechanismInList(pSecAgreeClient->securityData.hRemoteSecurityList,
										RVSIP_SECURITY_MECHANISM_TYPE_DIGEST, NULL, &hSecurityHeader, NULL);
	if (NULL == hSecurityHeader)
	{
		rv = RV_ERROR_UNKNOWN;
	}
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
					"SecAgreeMsgHandlingClientLoadAuthenticationDataFromMsg - Security-agreement 0x%p: Failed to find Security-Server header with digest, rv=%d",
                    pSecAgreeClient, rv));
        return rv;
	}

	/* Construct Auth-List */
	rv = SipAuthenticatorConstructAuthObjList(pSecAgreeClient->pSecAgreeMgr->hAuthenticator,
											  pSecAgreeClient->hPage,
											  pSecAgreeClient,
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
											  (ObjLockFunc)SecAgreeLockAPI,
											  (ObjUnLockFunc)SecAgreeUnLockAPI,
#else
                                              NULL, NULL,
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/
											  &pSecAgreeClient->securityData.additionalKeys.authInfo.hClientAuth.pAuthListInfo,
											  &pSecAgreeClient->securityData.additionalKeys.authInfo.hClientAuth.hAuthList);
    if (rv != RV_OK)
    {
		RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
					"SecAgreeMsgHandlingClientLoadAuthenticationDataFromMsg - Security-agreement 0x%p: Failed to construct authentication list, rv=%d",
                    pSecAgreeClient, rv));
        return rv;
    }

	/* Get qop and algorithm from the Security-Server header, to run over the ones in the Proxy-Authenticate header */
	eQop              = RvSipSecurityHeaderGetDigestQopOption(hSecurityHeader);
	pQop.hPool        = SipSecurityHeaderGetPool(hSecurityHeader);
	pQop.hPage        = SipSecurityHeaderGetPage(hSecurityHeader);
	pQop.offset       = SipSecurityHeaderGetStrDigestQop(hSecurityHeader);
	eAlgorithm        = RvSipSecurityHeaderGetDigestAlgorithm(hSecurityHeader);
	pAlgorithm.hPool  = pQop.hPool;
	pAlgorithm.hPage  = pQop.hPage;
	pAlgorithm.offset = SipSecurityHeaderGetStrDigestAlgorithm(hSecurityHeader);
	akaVersion        = RvSipSecurityHeaderGetDigestAlgorithmAKAVersion(hSecurityHeader);

	RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
			  "SecAgreeMsgHandlingClientLoadAuthenticationDataFromMsg - Client security-agreement=0x%p: digest qop=%s and algorithm=%s retrieved from Security-Server header",
			  pSecAgreeClient, SipCommonConvertEnumToStrAuthQopOption(eQop), SipCommonConvertEnumToStrAuthAlgorithm(eAlgorithm)));

	/* Load Authentication data from header */
	rv = SipAuthenticatorUpdateAuthObjListFromHeader(
											pSecAgreeClient->pSecAgreeMgr->hAuthenticator,
											pSecAgreeClient->securityData.additionalKeys.authInfo.hClientAuth.pAuthListInfo,
											pSecAgreeClient->securityData.additionalKeys.authInfo.hClientAuth.hAuthList,
											hProxyAuth,
											eQop, &pQop,
											eAlgorithm, &pAlgorithm, akaVersion);
	if (RV_OK != rv)
	{
		SecAgreeSecurityDataDestructAuthData(pSecAgreeClient->pSecAgreeMgr, &pSecAgreeClient->securityData, RVSIP_SEC_AGREE_ROLE_CLIENT);
		RvLogError(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
					"SecAgreeMsgHandlingClientLoadAuthenticationDataFromMsg - Security-agreement 0x%p: Failed to update authentication list, rv=%d",
                    pSecAgreeClient, rv));
	}

	return rv;
}

/***************************************************************************
 * SecAgreeMsgHandlingClientSetAuthorizationDataToMsg
 * ------------------------------------------------------------------------
 * General: Computes a list of Authorization headers from the Proxy-Authenticate
 *          list in the client security-agreement object, and adds it to the message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient    - Pointer to the client security-agreement
 *         hMsg               - Handle to the message to write on
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingClientSetAuthorizationDataToMsg(
									IN    SecAgree*            pSecAgreeClient,
									IN    RvSipMsgHandle       hMsg)
{
	RvStatus               rv = RV_OK;
	AuthObjListInfo*       hAuthInfo;
	RLIST_HANDLE           hAuthList;

	/* we add authorization data only if the selected mechanism is 'digest' */
	if (pSecAgreeClient->securityData.selectedSecurity.eType != RVSIP_SECURITY_MECHANISM_TYPE_DIGEST)
	{
		return RV_OK;
	}

	hAuthList = pSecAgreeClient->securityData.additionalKeys.authInfo.hClientAuth.hAuthList;
	hAuthInfo = pSecAgreeClient->securityData.additionalKeys.authInfo.hClientAuth.pAuthListInfo;

	if (NULL == hAuthList)
	{
		/* No Authorization data to add */
		return RV_OK;
	}

	RvLogInfo(pSecAgreeClient->pSecAgreeMgr->pLogSrc,(pSecAgreeClient->pSecAgreeMgr->pLogSrc,
			  "SecAgreeMsgHandlingClientSetAuthorizationDataToMsg - Client security-agreement=0x%p: inserting authorization info to message=0x%p",
			  pSecAgreeClient, hMsg));

	/* Use the Authenticator to build an authorization list in the message */
	rv = SipAuthenticatorBuildAuthorizationListInMsg(pSecAgreeClient->pSecAgreeMgr->hAuthenticator,
													 hMsg,
													 pSecAgreeClient,
													 RVSIP_COMMON_STACK_OBJECT_TYPE_SECURITY_AGREEMENT,
													 pSecAgreeClient->tripleLock,
													 hAuthInfo,
													 hAuthList);


	return rv;
}

/*-----------------------------------------------------------------------*/
/*                         SECURITY SERVER FUNCTIONS                     */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeMsgHandlingServerIsInvalidRequest
 * ------------------------------------------------------------------------
 * General: Checks whether the received request contains Require or Proxy-Require
 *          with sec-agree, but more than one Via. This message must be rejected
 *          with 502 (Bad Gateway).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr	 - Pointer to the security-agreement manager
 *         hMsg          - Handle to the message
 * Output: pbIsInvalid   - RV_TRUE if there are multiple Vias and sec-agree required
 *         pbHasRequired - RV_TRUE if there were Require or Proxy-Require with
 *                         sec-agree
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerIsInvalidRequest(
							 IN  SecAgreeMgr*             pSecAgreeMgr,
							 IN  RvSipMsgHandle           hMsg,
							 OUT RvBool*                  pbIsInvalid,
							 OUT RvBool*                  pbHasRequired)
{
	RvStatus  rv;
	RvBool    bHasSingleVia;

	*pbIsInvalid = RV_FALSE;

	rv = SecAgreeMsgHandlingServerIsRequireSecAgreeInMsg(hMsg, pbHasRequired);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc, (pSecAgreeMgr->pLogSrc,
					"SecAgreeMsgHandlingServerIsInvalidRequest - Failed to check Require header in message 0x%p, rv=%d",
					hMsg, rv));
		return rv;
	}

	rv = SecAgreeMsgHandlingServerIsSingleVia(hMsg, &bHasSingleVia);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc, (pSecAgreeMgr->pLogSrc,
					"SecAgreeMsgHandlingServerIsInvalidRequest - Failed to check Via header in message 0x%p, rv=%d",
					hMsg, rv));
		return rv;
	}

	if (*pbHasRequired == RV_TRUE && bHasSingleVia == RV_FALSE)
	{
		RvLogInfo(pSecAgreeMgr->pLogSrc, (pSecAgreeMgr->pLogSrc,
				  "SecAgreeMsgHandlingServerIsInvalidRequest - Message 0x%p has Require or Proxy-Require sec-agree but more than one Via -> rejecting with 502",
				  hMsg));
		*pbIsInvalid = RV_TRUE;
	}

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecAgreeMgr);
#endif
	return RV_OK;
}

/***************************************************************************
 * SecAgreeMsgHandlingServerIsSupportedSecAgreeInMsg
 * ------------------------------------------------------------------------
 * General: Checks if there is Supported header with sec-agree in the message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMsg               - Handle to the message to write on
 * Output: pbIsSecAgree       - Boolean indication that there are Require
 *                              and Proxy-Require headers in the message
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerIsSupportedSecAgreeInMsg(
									IN    RvSipMsgHandle         hMsg,
									OUT   RvBool*                pbIsSecAgree)
{
	/* Look for Supported: sec-agree */
	*pbIsSecAgree = IsSecAgreeInMsg(hMsg, SUPPORTED_STRING);

	return RV_OK;
}

/***************************************************************************
 * SecAgreeMsgHandlingServerIsRequireSecAgreeInMsg
 * ------------------------------------------------------------------------
 * General: Checks if there are Require or Proxy-Require headers with sec-agree
 *          in the message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMsg               - Handle to the message to write on
 * Output: pbIsSecAgree       - Boolean indication that there are Require
 *                              and Proxy-Require headers in the message
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerIsRequireSecAgreeInMsg(
									IN    RvSipMsgHandle         hMsg,
									OUT   RvBool*                pbIsSecAgree)
{
	/* Look for Require: sec-agree */
	*pbIsSecAgree = IsSecAgreeInMsg(hMsg, REQUIRE_STRING);
	if (*pbIsSecAgree == RV_TRUE)
	{
		return RV_OK;
	}

	/* Look for Proxy-Require: sec-agree */
	*pbIsSecAgree = IsSecAgreeInMsg(hMsg, PROXY_REQUIRE_STRING);

	return RV_OK;
}

/***************************************************************************
 * SecAgreeMsgHandlingServerIsSingleVia
 * ------------------------------------------------------------------------
 * General: Checks if there is a single Via header in the message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMsg               - Handle to the message to write on
 * Output: pbIsSingleVia      - Boolean indication that there is a single Via
 *                              in the message
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerIsSingleVia(
									IN    RvSipMsgHandle         hMsg,
									OUT   RvBool*                pbIsSingleVia)
{
	RvSipViaHeaderHandle       hViaHeader;
	RvSipHeaderListElemHandle  hListElem;

	*pbIsSingleVia = RV_FALSE;

	/* get the first Via */
	hViaHeader = RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_VIA, RVSIP_FIRST_HEADER, &hListElem);
	if (hViaHeader == NULL)
	{
		return RV_ERROR_NOT_FOUND;
	}

	/* get the next Via */
	hViaHeader = RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_VIA, RVSIP_NEXT_HEADER, &hListElem);
	if (hViaHeader == NULL)
	{
		*pbIsSingleVia = RV_TRUE;
	}

	return RV_OK;
}

/***************************************************************************
 * SecAgreeMsgHandlingServerSetSecurityServerListToMsg
 * ------------------------------------------------------------------------
 * General: Adds the list of local security mechanisms as Security-Server
 *          headers to the message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeServer    - Pointer to the server security-agreement
 *         hMsg               - Handle to the message to write on
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerSetSecurityServerListToMsg(
									IN    SecAgree*              pSecAgreeServer,
									IN    RvSipMsgHandle         hMsg)
{
	RvStatus               rv    = RV_OK;
	RvSipCommonListHandle  hList = pSecAgreeServer->securityData.hLocalSecurityList;

	if (hList == NULL)
	{
		return RV_OK;
	}

	RvLogInfo(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
			  "SecAgreeMsgHandlingServerSetSecurityServerListToMsg - Server security-agreement=0x%p: setting Security-Server list to message=0x%p",
			  pSecAgreeServer, hMsg));

	/* The local security list is the list of security-server headers. Add it to the message */
	rv = AddSecurityListToMsg(hList, hMsg);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
				   "SecAgreeMsgHandlingServerSetSecurityServerListToMsg - Failed to add Security-Server list to message. server security-agreement=0x%p, rv=%d",
				   pSecAgreeServer, rv));
	}

	return rv;
}

/***************************************************************************
 * SecAgreeMsgHandlingServerIsThereSecurityClientListInMsg
 * ------------------------------------------------------------------------
 * General: Checks whether there are Security-Client headers in the message
 * Return Value: RV_TURE if there is Security-Client header in the message
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMsg          - Handle to the message
 * Inout   phMsgListElem - Handle to the list element of the first security header,
 *                         if exists. This way when building the list we can avoid
 *                         repeating the search for a new
 *         ppHeader      - Pointer to the first security header, if exists.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeMsgHandlingServerIsThereSecurityClientListInMsg(
										IN    RvSipMsgHandle			 hMsg,
										OUT   RvSipHeaderListElemHandle* phMsgListElem,
										OUT   RvSipSecurityHeaderHandle* phHeader)
{
	return IsThereSecurityListInMsg(hMsg, RVSIP_SECURITY_CLIENT_HEADER, phMsgListElem, phHeader);
}

/***************************************************************************
 * SecAgreeMsgHandlingServerLoadSecurityClientListFromMsg
 * ------------------------------------------------------------------------
 * General: Load the list of Security-Client headers into the remote list of
 *          the server security-agreement.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeServer    - Pointer to the server security-agreement
 *         hMsg               - Handle to the message to write on
 *         hMsgListElem       - The first Security-Client header in the message, if
 *                              previously found.
 *         pHeader            - The first Security-Client header in the message, if
 *                              previously found.
 *         pbIsSecurityClient - RV_TRUE if there are security-client headers in
 *                              the message, RV_FALSE if there aren't.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerLoadSecurityClientListFromMsg(
									IN    SecAgree*                   pSecAgreeServer,
									IN    RvSipMsgHandle              hMsg,
									IN    RvSipHeaderListElemHandle   hMsgListElem,
									IN    RvSipSecurityHeaderHandle   hHeader,
									OUT   RvBool*                     pbIsSecurityClient)
{
	RvStatus		rv;
	RvBool          bIsDigest;

	if (hMsgListElem == NULL)
	{
		/* check if there are security-client headers in the message */
		*pbIsSecurityClient = IsThereSecurityListInMsg(hMsg, RVSIP_SECURITY_CLIENT_HEADER, &hMsgListElem, &hHeader);

		if (*pbIsSecurityClient == RV_FALSE)
		{
			return RV_OK;
		}
	}
	else
	{
		/* A pointer to the first security-client was received from outside, therefore there must
		   be security-client headers in the message */
		*pbIsSecurityClient = RV_TRUE;
	}

	RvLogInfo(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
			  "SecAgreeMsgHandlingServerLoadSecurityClientListFromMsg - Server security-agreement=0x%p: Loading Security-Client list from message=0x%p",
			  pSecAgreeServer, hMsg));

	if (pSecAgreeServer->securityData.hRemoteSecurityList != NULL)
	{
		/* There is already a security-client list in the object. Usually the state would indicate that
	       there was already a remote list set to this object, and we won't reach that point (the state
		   would change to invalid and a new object would be supplied for the received list). This can
		   only happen if the application set a remote list to the object, and no state change took place */
		RvLogWarning(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
				     "SecAgreeMsgHandlingServerLoadSecurityClientListFromMsg - server security-agreement=0x%p already has a remote security list. This list will be ran over by the list that arrived on the message",
					 pSecAgreeServer));
		SecAgreeSecurityDataDestructSecurityList(&pSecAgreeServer->securityData, SEC_AGREE_SEC_DATA_LIST_REMOTE);
	}

	/* Construct the remote security list */
	rv = SecAgreeSecurityDataConstructSecurityList(pSecAgreeServer->pSecAgreeMgr, pSecAgreeServer->hPage, &pSecAgreeServer->securityData, SEC_AGREE_SEC_DATA_LIST_REMOTE, NULL);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
					"SecAgreeMsgHandlingServerLoadSecurityClientListFromMsg - Failed to construct remote security list for server security-agreement=0x%p, rv=%d",
					pSecAgreeServer, rv));
		return rv;
	}

	/* Load the list of security-server from the message */
	rv = LoadRemoteSecurityListFromMsg(pSecAgreeServer->pSecAgreeMgr,
									   pSecAgreeServer->hPage,
									   hMsg,
									   RVSIP_SEC_AGREE_ROLE_SERVER,
									   hMsgListElem,
									   hHeader,
									   &pSecAgreeServer->securityData,
									   &bIsDigest);
	if (RV_OK != rv)
	{
		SecAgreeSecurityDataDestructSecurityList(&pSecAgreeServer->securityData, SEC_AGREE_SEC_DATA_LIST_REMOTE);
		RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
				   "SecAgreeMsgHandlingServerLoadSecurityClientListFromMsg - Failed to copy Security-Client list from message. server security-agreement=0x%p, rv=%d",
				   pSecAgreeServer, rv));
		return rv;
	}

	return RV_OK;
}

/***************************************************************************
 * SecAgreeMsgHandlingServerSetAuthenticationDataToMsg
 * ------------------------------------------------------------------------
 * General: Adds the list of Proxy-Authenticate headers held by the
 *          server security-agreement object to the message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeServer    - Pointer to the server security-agreement
 *         hMsg               - Handle to the message to write on
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerSetAuthenticationDataToMsg(
									IN    SecAgree*            pSecAgreeServer,
									IN    RvSipMsgHandle       hMsg)
{
	RvStatus                        rv = RV_OK;
	RvSipAuthenticationHeaderHandle hAuthHeader;
	RvSipAuthenticationHeaderHandle hMsgAuthHeader;
	RvSipHeaderListElemHandle       hMsgElem;

	hAuthHeader = pSecAgreeServer->securityData.additionalKeys.authInfo.hServerAuth.hProxyAuth;
	if (hAuthHeader == NULL)
	{
		return RV_OK;
	}

	RvLogInfo(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
			  "SecAgreeMsgHandlingServerSetAuthenticationDataToMsg - Server security-agreement=0x%p: setting Proxy-Authenticate header to message=0x%p",
			  pSecAgreeServer, hMsg));

	rv = RvSipMsgPushHeader(hMsg, RVSIP_LAST_HEADER,
							(void*)hAuthHeader, RVSIP_HEADERTYPE_AUTHENTICATION,
							&hMsgElem, (void**)&hMsgAuthHeader);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
				   "SecAgreeMsgHandlingServerSetAuthenticationDataToMsg - Server security-agreement=0x%p failed to set Proxy-Authenticate header to message 0x%p, rv=%d",
				   pSecAgreeServer, hMsg, rv));
	}

	return rv;
}

/***************************************************************************
 * SecAgreeMsgHandlingServerIsThereSecurityVerifyListInMsg
 * ------------------------------------------------------------------------
 * General: Checks whether there are Security-Verify headers in the message
 * Return Value: RV_TURE if there is Security-Verify header in the message
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMsg          - Handle to the message
 * Inout   phMsgListElem - Handle to the list element of the first security header,
 *                         if exists. This way when building the list we can avoid
 *                         repeating the search for a new
 *         ppHeader      - Pointer to the first security header, if exists.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeMsgHandlingServerIsThereSecurityVerifyListInMsg(
										IN    RvSipMsgHandle			 hMsg,
										OUT   RvSipHeaderListElemHandle* phMsgListElem,
										OUT   RvSipSecurityHeaderHandle* phHeader)
{
	return IsThereSecurityListInMsg(hMsg, RVSIP_SECURITY_VERIFY_HEADER, phMsgListElem, phHeader);
}

/***************************************************************************
 * SecAgreeMsgHandlingServerCheckSecurityVerifyList
 * ------------------------------------------------------------------------
 * General: Compare the received list of Security-Verify headers to the local
 *          list of security headers in the server security-agreement object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeServer    - Pointer to the server security-agreement
 *         hMsg               - Handle to the message to compare to
 *         hMsgListElem       - Handle to the list element of the first security header,
 *                              if exists.
 *         hHeader            - Handle to the first security header, if exists.
 *         pbIsTispan		  - Indicates whether this is a TISPAN comparison. If it is,
 *							    compares the old list of local security to the received
 *							    Security-Verify list. If it is not, compares the local
 *							    security list that was set to this security agreement 
 *							    to the received Security-Verify list.
 * Output: pbAreEqual         - RV_TRUE if the two lists are equal, RV_FALSE
 *                              if not equal.
 *         pbHasMsgList       - RV_TRUE if there are Security-Verify headers in the message
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerCheckSecurityVerifyList(
									IN    SecAgree*                  pSecAgreeServer,
									IN    RvSipMsgHandle             hMsg,
									IN    RvSipHeaderListElemHandle  hMsgListElem,
									IN    RvSipSecurityHeaderHandle  hHeader,
									IN    RvBool                     bIsTispan,
									OUT   RvBool*                    pbAreEqual,
									OUT   RvBool*                    pbHasMsgList)
{
	RvStatus				rv        = RV_OK;
	RvSipCommonListHandle   hLocalListToCompare;
	
	if (bIsTispan == RV_FALSE)
	{
		/* Compare the local list that was sent to the client to the Security-Verify list that was
		   received from the client */
		hLocalListToCompare = pSecAgreeServer->securityData.hLocalSecurityList;
	}
	else
	{
		/* Compare the list of Security-Verify that was received from the client to the local list
		   that was copied from a previous security-agreement via RvSipSecAgreeCopy() */
		hLocalListToCompare = pSecAgreeServer->securityData.hTISPANLocalList;
	}

	/* compare local security-list to the list of security-verify headers that arrived in the message */
	rv = SecAgreeCheckList(hLocalListToCompare, hMsg,
						   hMsgListElem, hHeader,
						   RVSIP_SECURITY_VERIFY_HEADER, pbHasMsgList, pbAreEqual);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
				   "SecAgreeMsgHandlingServerCheckSecurityVerifyList - Failed to compare local security list with the Security-Verify list from message. server security-agreement=0x%p, Message=0x%p, rv=%d",
				   pSecAgreeServer, hMsg, rv));
		return rv;
	}

	return RV_OK;
}

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
RvStatus RVCALLCONV SecAgreeServerCheckSecurityVerifyList(
									IN    SecAgree*                  pSecAgree1,
									IN    SecAgree*                  pSecAgree2,
                  OUT   RvBool*                    pbAreEqual) {

  RvStatus				rv        = RV_OK;
	RvSipCommonListHandle   hLocalListToCompare1;
	RvSipCommonListHandle   hLocalListToCompare2;
	
   if(pSecAgree1 == NULL || pSecAgree2 == NULL)
      return RV_ERROR_NULLPTR;

	hLocalListToCompare1 = pSecAgree1->securityData.hLocalSecurityList;
  hLocalListToCompare2 = pSecAgree2->securityData.hLocalSecurityList;

	/* compare local security-list to the list of security-verify headers that arrived in the message */
  rv = SecAgreeCheckLists(hLocalListToCompare1, hLocalListToCompare1,
						   RVSIP_SECURITY_VERIFY_HEADER, pbAreEqual);
	if (RV_OK != rv)
	{
#if 0
      RvLogError(pSecAgree1->pSecAgreeMgr->pLogSrc,(pSecAgree1->pSecAgreeMgr->pLogSrc,
				   "SecAgreeMsgHandlingServerCheckSecurityVerifyList - Failed to compare local security list with the Security-Verify list from message. sa1=0x%p, sa2=0x%p, rv=%d",
				   pSecAgree1, pSecAgree2, rv));
#endif
		return rv;
	}

	return RV_OK;
}

#endif
/*SPIRENT_END */

/***************************************************************************
 * SecAgreeMsgHandlingServerCheckSecurityClientList
 * ------------------------------------------------------------------------
 * General: Compare the received list of Security-Client headers to the remote
 *          list of security headers in the server security-agreement object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeServer    - Pointer to the server security-agreement
 *         hMsg               - Handle to the message to compare to
 *         hMsgListElem       - Handle to the list element of the first security header,
 *                              if exists.
 *         hHeader            - Handle to the first security header, if exists.
 * Output: pbAreEqual         - RV_TRUE if the two lists are equal, RV_FALSE
 *                              if not equal.
 *         pbHasMsgList       - RV_TRUE if there are Security-Verify headers in the message
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerCheckSecurityClientList(
									IN    SecAgree*					 pSecAgreeServer,
									IN    RvSipMsgHandle			 hMsg,
									IN    RvSipHeaderListElemHandle  hMsgListElem,
									IN    RvSipSecurityHeaderHandle  hHeader,
									OUT   RvBool*					 pbAreEqual,
									OUT   RvBool*                    pbHasMsgList)
{
	RvStatus             rv        = RV_OK;
	
	RvLogInfo(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
			  "SecAgreeMsgHandlingServerCheckSecurityClientList - Server security-agreement=0x%p: compare Security-Client list received in message=0x%p to previously received list",
			  pSecAgreeServer, hMsg));

	/* compare local security-list to the list of security-client headers that arrived in the message,
	   as defined in TS.33.203 */
	rv = SecAgreeCheckList(pSecAgreeServer->securityData.hRemoteSecurityList, hMsg,
						   hMsgListElem, hHeader,
						   RVSIP_SECURITY_CLIENT_HEADER, pbHasMsgList, pbAreEqual);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
				   "SecAgreeMsgHandlingServerCheckSecurityClientList - Failed to compare local security list with the Security-Client list from message. server security-agreement=0x%p, Message=0x%p, rv=%d",
				   pSecAgreeServer, hMsg, rv));
		return rv;
	}

	if (*pbHasMsgList == RV_FALSE)
	{
		/* if there are no security-client headers in the message we accept it
		   to be compliant with RFC3329 */
		RvLogInfo(pSecAgreeServer->pSecAgreeMgr->pLogSrc,(pSecAgreeServer->pSecAgreeMgr->pLogSrc,
					  "SecAgreeServerMsgRcvdHandleSecurityHeaders - Server security-agreement=0x%p: No Security-Client list in message=0x%p, no need to compare lists",
					  pSecAgreeServer, hMsg));
		*pbAreEqual = RV_TRUE;
	}
	
	return RV_OK;
}


/*------------------------------------------------------------------------
           S T A T I C   F U N C T I O N S
 -------------------------------------------------------------------------*/

/***************************************************************************
 * AddSecurityListToMsg
 * ------------------------------------------------------------------------
 * General: Adds the given list of security headers to the message. The type
 *          of headers (Client, Server or Verify) should already be updated
 *          within them.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hList     - Handle to the list of security headers
 *         hMsg      - Handle to the message to write on
 ***************************************************************************/
static RvStatus RVCALLCONV AddSecurityListToMsg(
									IN    RvSipCommonListHandle   hList,
									IN    RvSipMsgHandle          hMsg)
{
	RvStatus                  rv = RV_OK;
	RvInt                     elementType;
	RvSipSecurityHeaderHandle hSecurityHeader;
	RvSipSecurityHeaderHandle hMsgHeader;
	RvSipCommonListElemHandle hCurrElem;
	RvSipCommonListElemHandle hNextElem;
	RvSipHeaderListElemHandle hMsgElem;
	RvInt                     safeCounter = 0;

	rv = RvSipCommonListGetElem(hList, RVSIP_FIRST_ELEMENT, NULL,
						        &elementType, (void**)&hSecurityHeader, &hCurrElem);
	while (rv == RV_OK && hCurrElem != NULL && safeCounter < SEC_AGREE_MAX_LOOP(hList))
	{
		rv = RvSipMsgPushHeader(hMsg, RVSIP_LAST_HEADER,
							    (void*)hSecurityHeader, (RvSipHeaderType)elementType,
								&hMsgElem, (void**)&hMsgHeader);
		if (RV_OK != rv)
		{
			break;
		}

		rv = RvSipCommonListGetElem(hList, RVSIP_NEXT_ELEMENT, hCurrElem,
									&elementType, (void**)&hSecurityHeader, &hNextElem);
		hCurrElem = hNextElem;

		safeCounter += 1;
	}

	return rv;
}

/***************************************************************************
 * IsThereSecurityListInMsg
 * ------------------------------------------------------------------------
 * General: Checks whether there is a remote security list in the message.
 *          If there is, LoadRemoteSecurityListFromMsg() should be called to
 *          load it.
 * Return Value: RV_TRUE if there is a remote list in the message, RV_FALSE
 *               otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMsg                - Handle to the message to write on
 *         eSecurityHeaderType - Enumeration of the type of list to look for
 * Inout   phMsgListElem       - Handle to the list element of the first security header,
 *								 if exists. This way when building the list we can avoid
 *		                         repeating the search for a new
 *         ppHeader			   - Pointer to the first security header, if exists.
 ***************************************************************************/
static RvBool RVCALLCONV IsThereSecurityListInMsg(
									IN    RvSipMsgHandle             hMsg,
									IN    RvSipSecurityHeaderType    eSecurityHeaderType,
									OUT   RvSipHeaderListElemHandle* phMsgListElem,
									OUT   RvSipSecurityHeaderHandle* phHeader)
{
	RvSipSecurityHeaderHandle  hSecurityHeader;
	RvSipHeaderListElemHandle  hSecurityListElem;
	RvInt                      safeCounter = 0;

	hSecurityHeader = RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_SECURITY, RVSIP_FIRST_HEADER, &hSecurityListElem);
	while (hSecurityHeader != NULL && safeCounter < SEC_AGREE_MAX_LOOP(pSecAgreeClient))
	{
		if (eSecurityHeaderType == RvSipSecurityHeaderGetSecurityHeaderType(hSecurityHeader))
		{
			*phHeader      = hSecurityHeader;
			*phMsgListElem = hSecurityListElem;
			return RV_TRUE;
		}

		hSecurityHeader = RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_SECURITY, RVSIP_NEXT_HEADER, &hSecurityListElem);

		safeCounter++;
	}

	return RV_FALSE;
}

/***************************************************************************
 * LoadRemoteSecurityListFromMsg
 * ------------------------------------------------------------------------
 * General: Loads the remote security list from message. For client: this is
 *          the list of Security-Server headers that was received in the response.
 *          This list should be stored as Security-Verify in the Security-Client's
 *          remote list. For server: this is the list of Security-Client headers
 *          that was received in the request, which should be stored in the remote
 *          list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr - Pointer to the Security-Mgr
 *         hPage        - The memory page of the security object to construct the
 *                        headers on.
 *         hMsg         - Handle to the message to write on
 *         eRole         - Indicate whether this is client or server
 *         hPrevMsgListElem - If supplied, there is no need to search for the first
 *                            header in the list, but rather use this one
 *         pPrevHeader  - If supplied, there is no need to search for the first
 *                        header in the list, but rather use this one
 * Inout:  pSecAgreeSecurityData - Pointer to the data structure with the remote list handle
 * Ouput:  pbIsDigest   - RV_TRUE if there was digest mechanism in one of
 *                        the Security-Server headers. If yes, Proxy-Authenticate
 *                        headers will be copied from the message.
 ***************************************************************************/
static RvStatus RVCALLCONV LoadRemoteSecurityListFromMsg(
									IN    SecAgreeMgr*               pSecAgreeMgr,
									IN    HPAGE                      hPage,
									IN    RvSipMsgHandle             hMsg,
									IN    RvSipSecAgreeRole          eRole,
									IN    RvSipHeaderListElemHandle  hPrevMsgListElem,
									IN    RvSipSecurityHeaderHandle  hPrevHeader,
									INOUT SecAgreeSecurityData*              pSecAgreeSecurityData,
									OUT   RvBool*                    pbIsDigest)
{
	RvStatus                  rv = RV_OK;
	RvSipSecurityHeaderType   eSecurityHeaderType;
	RvSipSecurityHeaderHandle hMsgSecurityHeader;
	RvSipSecurityHeaderHandle hNewSecurityHeader;
	RvSipHeaderListElemHandle hMsgListElem;
	RvInt                     safeCounter = 0;

	*pbIsDigest = RV_FALSE;

	if (eRole == RVSIP_SEC_AGREE_ROLE_CLIENT)
	{
		/* a client loads security-sever headers from message */
		eSecurityHeaderType = RVSIP_SECURITY_SERVER_HEADER;
	}
	else /* eRole == RVSIP_SECURITY_SERVER */
	{
		/* a server loads security-client headers from message */
		eSecurityHeaderType = RVSIP_SECURITY_CLIENT_HEADER;
	}

	if (hPrevMsgListElem == NULL)
	{
		/* find the first security-client/server header in the message */
		hMsgSecurityHeader = RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_SECURITY, RVSIP_FIRST_HEADER, &hMsgListElem);
	}
	else
	{
		/* the first header was found previous to this function, and was supplied as parameter here */
		hMsgListElem       = hPrevMsgListElem;
		hMsgSecurityHeader = hPrevHeader;
	}
	while (hMsgSecurityHeader != NULL && safeCounter < SEC_AGREE_MAX_LOOP(pSecAgreeMgr))
	{
		if (eSecurityHeaderType == RvSipSecurityHeaderGetSecurityHeaderType(hMsgSecurityHeader))
		{
			/* Copy and push the security header to the remote list of this security-agreement data */
			rv = SecAgreeSecurityDataPushSecurityHeader(pSecAgreeMgr, hPage, pSecAgreeSecurityData, hMsgSecurityHeader, RVSIP_LAST_ELEMENT, NULL, SEC_AGREE_SEC_DATA_LIST_REMOTE, NULL, &hNewSecurityHeader);
			if (RV_OK != rv)
			{
				break;
			}

			if (eRole == RVSIP_SEC_AGREE_ROLE_CLIENT)
			{
				/* We keep a list of security-verify headers in the client, instead of security-server headers as
				   were received on the message */
				rv = RvSipSecurityHeaderSetSecurityHeaderType(hNewSecurityHeader, RVSIP_SECURITY_VERIFY_HEADER);
				if (RV_OK != rv)
				{
					return rv;
				}
			}

			/* update pbIsDigest */
			if (RvSipSecurityHeaderGetMechanismType(hNewSecurityHeader) == RVSIP_SECURITY_MECHANISM_TYPE_DIGEST)
			{
				*pbIsDigest = RV_TRUE;
			}
		}

		hMsgSecurityHeader = RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_SECURITY, RVSIP_NEXT_HEADER, &hMsgListElem);

		safeCounter += 1;
	}

	return rv;
}

/***************************************************************************
 * SecAgreeCheckList
 * ------------------------------------------------------------------------
 * General: Compare the received list with the list of headers in the message
 *          Used by the server for two comparisons: comparing local list with
 *          Security-Verify list in message, and comparing remote list with
 *          Security-Client list in message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecurityList      - The list to compare to the message
 *         hMsg               - Handle to the message to compare to.
 *         hMsgListElem       - Handle to the list element of the first security header,
 *                              if exists.
 *         hMsgSecurityHeader - Handle to the first security header, if exists.
 *         strHeaderName      - The type of headers to look for in the message.
 * Output: pbHasLocalList     - RV_TRUE if there is local list to the object
 *         pbHasMsgList       - RV_TRUE if there is Security-Client in the message
 *         pbAreEqual         - RV_TRUE if the two lists are equal, RV_FALSE
 *                              if not equal.
 ***************************************************************************/
static RvStatus RVCALLCONV SecAgreeCheckList(
									IN    RvSipCommonListHandle      hSecurityList,
									IN    RvSipMsgHandle             hMsg,
									IN    RvSipHeaderListElemHandle  hMsgListElem,
									IN    RvSipSecurityHeaderHandle  hMsgSecurityHeader,
									IN    RvSipSecurityHeaderType    eHeaderType,
									OUT   RvBool*                    pbHasMsgList,
									OUT   RvBool*                    pbAreEqual)
{
	RvStatus                  rv        = RV_OK;
	RvBool                    bAreEqual = RV_TRUE;
	RvSipSecurityHeaderHandle hSecurityHeader;
	RvInt                     elementType;
	RvSipCommonListElemHandle hSecurityListElem;
	RvInt                     safeCounter = 0;

	*pbHasMsgList     = RV_FALSE;
	*pbAreEqual       = RV_FALSE;

	if (NULL == hSecurityList)
	{
		/* there is no security list in the security object */
		hSecurityListElem = NULL;
		hSecurityHeader   = NULL;
	}
	else
	{
		/* get the first element from the security-agreement local list */
		rv = RvSipCommonListGetElem(hSecurityList, RVSIP_FIRST_ELEMENT, NULL, &elementType, (void**)&hSecurityHeader, &hSecurityListElem);
	}

	if (hMsgListElem == NULL)
	{
		/* Get the next security header from hMsg according to eHeaderType */
		hMsgSecurityHeader = GetSecurityHeader(hMsg, eHeaderType, RVSIP_FIRST_HEADER, &hMsgListElem);
	}
	if (hMsgListElem != NULL)
	{
		/* Set indication that there is a list of security headers in the message */
		*pbHasMsgList = RV_TRUE;
	}
	while (hMsgSecurityHeader != NULL && hSecurityHeader != NULL && rv == RV_OK && safeCounter < SEC_AGREE_MAX_LOOP(hSecurityList))
	{
		/* Compare the two headers to find if they are equal. We do not compare header type and digest-verify parameter */
		bAreEqual = SipSecurityHeaderIsEqual(hSecurityHeader, hMsgSecurityHeader, RV_FALSE, RV_FALSE);
		if (bAreEqual == RV_FALSE)
		{
			break;
		}

		/* get the next element from the security-agreement local list */
		rv = RvSipCommonListGetElem(hSecurityList, RVSIP_NEXT_ELEMENT, hSecurityListElem, &elementType, (void**)&hSecurityHeader, &hSecurityListElem);
		
		/* Get the next security header from hMsg according to eHeaderType */
		hMsgSecurityHeader = GetSecurityHeader(hMsg, eHeaderType, RVSIP_NEXT_HEADER, &hMsgListElem);

		safeCounter += 1;
	}

	if ((hMsgSecurityHeader != NULL && hSecurityHeader == NULL) ||
		(hMsgSecurityHeader == NULL && hSecurityHeader != NULL))
	{
		/* the lists are of different sizes */
		bAreEqual = RV_FALSE;
	}
	if (RV_OK != rv)
	{
		return rv;
	}

	*pbAreEqual = bAreEqual;
	return RV_OK;
}

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
static RvStatus RVCALLCONV SecAgreeCheckLists(
									IN    RvSipCommonListHandle      hSecurityList1,
									IN    RvSipCommonListHandle      hSecurityList2,
									IN    RvSipSecurityHeaderType    eHeaderType,
                  OUT   RvBool*                    pbAreEqual) {

  RvStatus                  rv        = RV_OK;
	RvBool                    bAreEqual = RV_TRUE;
	RvSipSecurityHeaderHandle hSecurityHeader1;
	RvSipSecurityHeaderHandle hSecurityHeader2;
  RvSipCommonListElemHandle hSecurityListElem1;
  RvSipCommonListElemHandle hSecurityListElem2;
	RvInt                     elementType1;
	RvInt                     elementType2;
	RvInt                     safeCounter = 0;

  *pbAreEqual       = RV_FALSE;
  
  /* get the first element from the security-agreement local list */
  rv = RvSipCommonListGetElem(hSecurityList1, RVSIP_FIRST_ELEMENT, NULL, &elementType1, (void**)&hSecurityHeader1, &hSecurityListElem1);
  
  /* get the first element from the security-agreement local list */
  rv = RvSipCommonListGetElem(hSecurityList2, RVSIP_FIRST_ELEMENT, NULL, &elementType2, (void**)&hSecurityHeader2, &hSecurityListElem2);
  
  while (hSecurityHeader1 != NULL && hSecurityHeader2 != NULL && rv == RV_OK && safeCounter < SEC_AGREE_MAX_LOOP(hSecurityList))
	{
		/* Compare the two headers to find if they are equal. We do not compare header type and digest-verify parameter */
		bAreEqual = SipSecurityHeaderIsEqual(hSecurityHeader1, hSecurityHeader2, RV_FALSE, RV_FALSE);


		if (bAreEqual == RV_FALSE)
		{
			break;
		}

		/* get the next element from the security-agreement local list */
		rv = RvSipCommonListGetElem(hSecurityList1, RVSIP_NEXT_ELEMENT, hSecurityListElem1, &elementType1, (void**)&hSecurityHeader1, &hSecurityListElem1);
		
		/* get the next element from the security-agreement local list */
		rv = RvSipCommonListGetElem(hSecurityList2, RVSIP_NEXT_ELEMENT, hSecurityListElem2, &elementType2, (void**)&hSecurityHeader2, &hSecurityListElem2);

		safeCounter += 1;
	}

	if ((hSecurityHeader1 != NULL && hSecurityHeader2 == NULL) ||
		(hSecurityHeader1 == NULL && hSecurityHeader2 != NULL))
	{
		/* the lists are of different sizes */
		bAreEqual = RV_FALSE;
	}
	if (RV_OK != rv)
	{
		return rv;
	}

	*pbAreEqual = bAreEqual;

	return RV_OK;

}

#endif
/* SPIRENT_END */

/***************************************************************************
 * GetSecurityHeader
 * ------------------------------------------------------------------------
 * General: Get the specified security header from the message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMsg           - Handle of the message object.
 *         hHeaderType    - The header type to be retrieved (client, server or verify).
 *         location       - The location on list next, previous, first or last.
 * Inout:  phListElem     - Handle to the current position in the list. 
 ***************************************************************************/
static RvSipSecurityHeaderHandle RVCALLCONV GetSecurityHeader(
									IN    RvSipMsgHandle			   hMsg,
									IN    RvSipSecurityHeaderType      eHeaderType,
									IN    RvSipHeadersLocation         location,
									INOUT RvSipHeaderListElemHandle*   phListElem)
{
	RvSipSecurityHeaderHandle hSecurityHeader;
	RvInt                     safeCounter = 0;

	hSecurityHeader = RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_SECURITY, location, phListElem);
	while (hSecurityHeader != NULL && safeCounter < SEC_AGREE_MAX_LOOP(hSecurityList))
	{
		if (RvSipSecurityHeaderGetSecurityHeaderType(hSecurityHeader) == eHeaderType)
		{
			return hSecurityHeader;
		}
		hSecurityHeader = RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_SECURITY, RVSIP_NEXT_HEADER, phListElem);
		safeCounter += 1;
	}

	*phListElem = NULL;
	return NULL;
}

/***************************************************************************
 * IsSecAgreeInMsg
 * ------------------------------------------------------------------------
 * General: Checks if there is a header in the message of type strHeaderName
 *          that contains the sec-agree option tag
 * Return Value: RvBool
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hMsg           - Handle to the message
 *        strHeaderName  - The name of the header (Require, Proxy-Require or Supported)
 ***************************************************************************/
static RvBool RVCALLCONV IsSecAgreeInMsg(IN  RvSipMsgHandle  hMsg,
										 IN  RvChar*         strHeaderName)
{
	RvSipOtherHeaderHandle     hHeader;
	RvSipHeaderListElemHandle  hListElem;
	RvInt                      safeCounter = 0;

	hHeader = RvSipMsgGetHeaderByName(hMsg, strHeaderName, RVSIP_FIRST_HEADER, &hListElem);
	while (hHeader != NULL && safeCounter < SEC_AGREE_MAX_LOOP(hMsg))
	{
		if (RV_TRUE == IsSecAgreeInHeader(hHeader))
		{
			return RV_TRUE;
		}

		hHeader = RvSipMsgGetHeaderByName(hMsg, strHeaderName, RVSIP_NEXT_HEADER, &hListElem);

		safeCounter += 1;
	}

	return RV_FALSE;
}

/***************************************************************************
 * IsRequireHeader
 * ------------------------------------------------------------------------
 * General: Checks if the header name is either Require or Proxy-Require
 * Return Value: RvBool
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hHeader       - Handle to header
 ***************************************************************************/
static RvBool RVCALLCONV IsRequireHeader(IN    RvSipOtherHeaderHandle  hHeader)
{
	HRPOOL    hPool;
	HPAGE     hPage;
	RvInt32   offset;
	RvBool    bIsRequire;

	hPool  = SipOtherHeaderGetPool(hHeader);
	hPage  = SipOtherHeaderGetPage(hHeader);
	offset = SipOtherHeaderGetName(hHeader);

	bIsRequire = RPOOL_CmpiPrefixToExternal(hPool, hPage, offset, REQUIRE_STRING);
	if (bIsRequire == RV_FALSE)
	{
		return RPOOL_CmpiPrefixToExternal(hPool, hPage, offset, PROXY_REQUIRE_STRING);
	}

	return RV_TRUE;
}

/***************************************************************************
 * IsSecAgreeInHeader
 * ------------------------------------------------------------------------
 * General: Checks if the header value contains sec-agree
 * Return Value: RvBool
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hHeader        - Handle to header (Require, Proxy-Require or Supported)
 ***************************************************************************/
static RvBool RVCALLCONV IsSecAgreeInHeader(
										IN    RvSipOtherHeaderHandle  hHeader)
{
	HRPOOL    hPool;
	HPAGE     hPage;
	RvInt32   offset;

	hPool  = SipOtherHeaderGetPool(hHeader);
	hPage  = SipOtherHeaderGetPage(hHeader);
	offset = SipOtherHeaderGetValue(hHeader);

	return RPOOL_CmpiPrefixToExternal(hPool, hPage, offset, SEC_AGREE_STRING);
}

/***************************************************************************
 * FindFirstProxyAuthenticate
 * ------------------------------------------------------------------------
 * General: Finds the first Proxy-Authenticate header in the message
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMsg         - Handle to the message to write on
 * Ouput:  phProxyAuth  - The first Proxy-Authenticate header in the message
 ***************************************************************************/
static void RVCALLCONV FindFirstProxyAuthenticate(
									IN    RvSipMsgHandle                   hMsg,
									OUT   RvSipAuthenticationHeaderHandle* phProxyAuth)
{
	RvSipAuthenticationHeaderHandle hAuthHeader;
	RvSipHeaderListElemHandle       hListElem;

	*phProxyAuth = NULL;

	hAuthHeader = (RvSipAuthenticationHeaderHandle)RvSipMsgGetHeaderByType(
													hMsg, RVSIP_HEADERTYPE_AUTHENTICATION,
													RVSIP_FIRST_HEADER, &hListElem);
    while (NULL != hAuthHeader)
    {
        if (RvSipAuthenticationHeaderGetHeaderType(hAuthHeader) == RVSIP_AUTH_PROXY_AUTHENTICATION_HEADER)
		{
			*phProxyAuth = hAuthHeader;
			break;
		}

        /* get the next authentication header from message */
        hAuthHeader = (RvSipAuthenticationHeaderHandle)RvSipMsgGetHeaderByType(
													hMsg, RVSIP_HEADERTYPE_AUTHENTICATION,
													RVSIP_NEXT_HEADER, &hListElem);
    }
}

#endif /* #ifdef RV_SIP_IMS_ON */

