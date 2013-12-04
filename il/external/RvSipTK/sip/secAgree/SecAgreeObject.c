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
 *                              <SecAgreeObject.c>
 *
 * This file defines the security-agreement object, attributes and interface functions.
 * The security-agreement object is used to negotiate a security agreement with the
 * remote party, and to maintain this agreement in further requests, as defined in RFC 3329.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Nov 2005
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "SecAgreeObject.h"
#include "SecAgreeSecurityData.h"
#include "SecAgreeTypes.h"
#include "SecAgreeState.h"
#include "SecAgreeMgrObject.h"
#include "SecAgreeCallbacks.h"
#include "SecAgreeUtils.h"
#include "SecAgreeSecurityData.h"
#include "SecAgreeClient.h"
#include "SecAgreeServer.h"
#include "_SipTransport.h"
#include "RvSipSecurityHeader.h"
#include "SecAgreeCallbacks.h"
#include "RvSipAuthenticationHeader.h"
#include "RvSipAuthorizationHeader.h"
#include "_SipCommonConversions.h"
#include "_SipSecuritySecObj.h"
#include "RvSipSecurity.h"
#include "SecAgreeStatus.h"
#include "RvSipCompartment.h"

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV SecAgreeLock(IN SecAgree          *pSecAgree,
                                        IN SecAgreeMgr       *pSecAgreeMgr,
                                        IN SipTripleLock     *tripleLock,
                                        IN RvInt32            identifier);

static void RVCALLCONV SecAgreeUnLock(IN  RvLogMgr *pLogMgr,
                                      IN  RvMutex  *hLock);

static RvStatus RVCALLCONV GetOwner(IN  SecAgree*              pSecAgree,
									OUT SecAgreeOwnerRecord**  ppOwnerRec);

static RvStatus RVCALLCONV SecAgreeDetachAllOwners(
											IN   SecAgree*    pSecAgree);

static void RVCALLCONV InitAddresses(IN  SecAgree     *pSecAgree);

static RvStatus RVCALLCONV CopyLocalAddresses(INOUT SecAgree*    pDestSecAgree,
											  IN    SecAgree*    pSourceSecAgree);

static RvStatus RVCALLCONV CopyRemoteAddresses(INOUT SecAgree*    pDestSecAgree,
											   IN    SecAgree*    pSourceSecAgree);

#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
static RvStatus RVCALLCONV CopyOwnersList(IN    SecAgree*			    pSecAgree,
										  INOUT RLIST_HANDLE            hTempList);

static RvStatus RVCALLCONV InitiateSecObj(
									IN  SecAgree*                      pSecAgree,
									IN  RvSipSecurityHeaderHandle      hIpsecLocalSecurity,
									OUT RvSipSecObjHandle*             phSecObj,
									OUT RvBool*                        pbSecObjInitiated);

static RvStatus RVCALLCONV ActivateSecObj(
									IN  SecAgree*                  pSecAgree,
									IN  RvSipSecObjHandle          hSecObj,
									IN  RvSipSecurityHeaderHandle  hRemoteSecurity);

static RvBool RVCALLCONV ShouldTerminateSecObj(
										 IN  SecAgree*            pSecAgree,
										 IN  SecAgreeProcessing   eProcessing);

static void RVCALLCONV TerminateSecObj(
									   IN   SecAgree*            pSecAgree,
									   IN   RvSipSecObjHandle    hSecObj);

static RvBool RVCALLCONV CanInitiateSecObj(
										 IN  SecAgree*            pSecAgree);

static RvBool RVCALLCONV CanActivateSecObj(
										 IN  SecAgree*            pSecAgree);

static RvBool RVCALLCONV ShouldSetMechanismToSecObj(
										 IN  SecAgree*            pSecAgree,
										 IN  SecAgreeProcessing   eProcessing);

static RvStatus RVCALLCONV UpdateSecObjMechanism(
									  IN  SecAgree*                   pSecAgree);

#ifdef RV_SIGCOMP_ON
static RvStatus RVCALLCONV SetSendOptionsToSecObj(
									IN    SecAgree           *pSecAgree,
									IN    RvSipSecObjHandle   hSecObj);
#endif /* #ifdef RV_SIGCOMP_ON */

#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */

#if (RV_IMS_IPSEC_ENABLED == RV_YES)
static RvStatus RVCALLCONV AllocAndCopyAddress(IN  HRPOOL      hPool,
											   IN  HPAGE       hPage,
											   IN  RvChar*     strAddrToCopy,
                                               OUT RvChar**    ppAddr);

static RvStatus RVCALLCONV SetLocalIpsecParamsToSecObj(
									     IN  RvSipSecObjHandle          hSecObj,
									     IN  RvSipSecurityHeaderHandle  hLocalSecurity);

static RvStatus RVCALLCONV SetLocalSecAgreeIpsecParamsToSecObj(
										 IN  SecAgree*                      pSecAgree,
										 IN  RvSipSecObjHandle              hSecObj);

static RvStatus RVCALLCONV SetRemoteIpsecParamsToSecObj(
									     IN  RvSipSecObjHandle          hSecObj,
									     IN  RvSipSecurityHeaderHandle  hRemoteSecurity);

static RvStatus RVCALLCONV SetRemoteSecAgreeIpsecParamsToSecObj(
										 IN  SecAgree*               pSecAgree,
									     IN  RvSipSecObjHandle       hSecObj);

static RvStatus RVCALLCONV GetIpsecPeerParamsFromSecurityHeader(
									IN    RvSipSecurityHeaderHandle   hIpsecSecurity,
									OUT   RvSipSecIpsecPeerParams*    pIpsecPeerParams);

static RvStatus RVCALLCONV GetIpsecParamsFromSecurityHeader(
									IN    RvSipSecurityHeaderHandle   hIpsecSecurity,
									OUT   RvSipSecIpsecParams*        pIpsecParams);

static RvStatus RVCALLCONV GetLocalIpsecParamsFromSecObj(
										 IN  SecAgree*                  pSecAgree,
									     IN  RvSipSecObjHandle          hSecObj,
									     IN  RvSipSecurityHeaderHandle  hLocalSecurity);

#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */

#if (RV_TLS_TYPE != RV_TLS_NONE)
static RvStatus RVCALLCONV SetSecAgreeTlsParamsToSecObj(
										 IN  SecAgree*            pSecAgree);
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */

static void RVCALLCONV ResetDataUponFailureToCopy(
										 IN  SecAgree*         pDestSecAgree,
										 IN  RvBool            bResetRemote);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#include "rvexternal.h"
#endif
/* SPIRENT_END */

/*-----------------------------------------------------------------------*/
/*                         SECURITY AGREEMENT FUNCTIONS                  */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeInitialize
 * ------------------------------------------------------------------------
 * General: Initialize a new security-agreement object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr - Pointer to the Security-Mgr
 *         hAppSecAgree - The application handle for the new security-agreement.
 *         pSecAgree    - Pointer to the new security-agreement
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeInitialize(
									IN    SecAgreeMgr*					pSecAgreeMgr,
									IN    RvSipAppSecAgreeHandle		hAppSecAgree,
									INOUT SecAgree*			            pSecAgree)
{
	RvStatus      rv;

	/* Allocate memory page */
	rv = RPOOL_GetPage(pSecAgreeMgr->hMemoryPool, 0, &pSecAgree->hPage);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeInitialize - Failed to allocate memory page. rv=%d", rv));
        return rv;
	}

	/* initialize security-agreement-data object */
	rv = SecAgreeSecurityDataInitialize(pSecAgreeMgr, &pSecAgree->securityData);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc ,(pSecAgreeMgr->pLogSrc  ,
					"SecAgreeInitialize - Failed to initialize security-agreement 0x%p, rv=%d",
					pSecAgree, rv));
		RPOOL_FreePage(pSecAgreeMgr->hMemoryPool, pSecAgree->hPage);
		return rv;
	}

	/* Construct owners list */
	pSecAgree->hOwnersList = RLIST_ListConstruct(pSecAgreeMgr->hSecAgreeOwnersPool);
	if (pSecAgree->hOwnersList == NULL)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeInitialize - Failed to allocate owners list"));
		RPOOL_FreePage(pSecAgreeMgr->hMemoryPool, pSecAgree->hPage);
        return RV_ERROR_OUTOFRESOURCES;
	}

	InitAddresses(pSecAgree); 
    /* Get default local address */
	rv = SecAgreeMgrGetDefaultLocalAddr(pSecAgreeMgr, pSecAgree); 
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeInitialize - Failed to get default local address. rv=%d", rv));
		RPOOL_FreePage(pSecAgreeMgr->hMemoryPool, pSecAgree->hPage);
        return rv;
	}

	pSecAgree->pSecAgreeMgr					= pSecAgreeMgr;
	pSecAgree->hAppOwner					= hAppSecAgree;
	pSecAgree->pContext                     = NULL;
	pSecAgree->tripleLock                   = &pSecAgree->secAgreeTripleLock;
	pSecAgree->numOfOwners                  = 0;
	pSecAgree->eRole                        = RVSIP_SEC_AGREE_ROLE_UNDEFINED;
	pSecAgree->eState                       = RVSIP_SEC_AGREE_STATE_UNDEFINED;
	pSecAgree->responseCode                 = 0;
	pSecAgree->eSpecialStandard             = RVSIP_SEC_AGREE_SPECIAL_STANDARD_UNDEFINED;
	pSecAgree->eSecAssocType                = RVSIP_SEC_AGREE_SEC_ASSOC_UNDEFINED;
	RvRandomGeneratorGetInRange(pSecAgree->pSecAgreeMgr->seed,RV_INT32_MAX,(RvRandom*)&pSecAgree->secAgreeUniqueIdentifier);
	memset(&(pSecAgree->terminationInfo), 0, sizeof(RvSipTransportObjEventInfo));
	
	return RV_OK;
}

/***************************************************************************
 * SecAgreeTerminate
 * ------------------------------------------------------------------------
 * General: Notifies the security-agreement owners on its termination and sends
 *          the security-agreement to the termination queue.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree - Pointer to the security-agreement to terminate.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeTerminate(
                                 IN SecAgree*      pSecAgree)
{
	RvSipSecAgreeState  eOldState = pSecAgree->eState;
	RvStatus            rv;

	/* Changing the state to terminated makes sure that this security-agreement will
	not be attached to new owners */
	pSecAgree->eState = RVSIP_SEC_AGREE_STATE_TERMINATED;

	RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc  ,
             "SecAgreeTerminate - security-agreement 0x%p: Inserting terminated state to the event queue",
			 pSecAgree));

	rv = SecAgreeStateTerminate(pSecAgree, eOldState);

	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc  ,
					"SecAgreeTerminate - security-agreement 0x%p: Failed to insert terminated state to the event queue, rv=%d",
					pSecAgree, rv));
	}

	return rv;
}

/***************************************************************************
 * SecAgreeDestruct
 * ------------------------------------------------------------------------
 * General: Free the security-agreement resources.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree - Pointer to the security-agreement to free.
 ***************************************************************************/
void RVCALLCONV SecAgreeDestruct(IN  SecAgree*     pSecAgree)
{
	SecAgreeMgr               *pSecAgreeMgr = pSecAgree->pSecAgreeMgr;
	RvStatus                   rv;

	/* Go over the list of owners and notify them to detach from this security-agreement. */
	rv = SecAgreeDetachAllOwners(pSecAgree);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc  ,
					"SecAgreeDestruct - security-agreement 0x%p: Failed to notify owners, rv=%d",
					pSecAgree, rv));
	}

	/* Reset unique identifier */
	pSecAgree->secAgreeUniqueIdentifier = 0;


	
	/* Detach from security object */
	if (NULL != pSecAgree->securityData.hSecObj)
	{
		rv = SipSecObjSetSecAgreeObject(pSecAgree->pSecAgreeMgr->hSecMgr, 
										pSecAgree->securityData.hSecObj, NULL, 0);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc  ,
						"SecAgreeDestruct - security-agreement 0x%p: Failed to detach from security object, rv=%d",
						pSecAgree, rv));
		}
		pSecAgree->securityData.hSecObj = NULL;
	}

	/* Free security-agreement data */
	SecAgreeSecurityDataDestruct(pSecAgreeMgr, &pSecAgree->securityData, pSecAgree->eRole);

	/* Free memory page */
	RPOOL_FreePage(pSecAgreeMgr->hMemoryPool, pSecAgree->hPage);

	/* remove this security-agreement object from the manager */
	SecAgreeMgrRemoveSecAgree(pSecAgreeMgr, pSecAgree);
}

/***************************************************************************
 * SecAgreeRemoveOwner
 * ------------------------------------------------------------------------
 * General: Detaches an existing owner from the security-agreement object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree   - Pointer to the security-agreement
 *         pOwnerInfo  - Owner information
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeRemoveOwner(
							 IN  SecAgree*               pSecAgree,
							 IN  SecAgreeOwnerRecord*    pOwnerInfo)
{
	RvStatus rv;

	/* Decrement owners counter (if we reach that point, this owner is definitely in the
	   hash and list of the security-agreement object */
	pSecAgree->numOfOwners -= 1;

	/* remove from hash */
	rv = SecAgreeMgrRemoveOwner(pSecAgree->pSecAgreeMgr, pOwnerInfo->pHashElement);
	/* remove from list */
	RLIST_Remove(pSecAgree->pSecAgreeMgr->hSecAgreeOwnersPool, pSecAgree->hOwnersList, pOwnerInfo->hListItem);
	
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
                  "SecAgreeRemoveOwner - Failed to remove security-agreement object from hash (security-agreement=0x%p, owner=0x%p, rv=%d)",
                  pSecAgree, pOwnerInfo->owner.pOwner, rv));
	}

	return rv;
}

/***************************************************************************
 * SecAgreeCreateHeader
 * ------------------------------------------------------------------------
 * General: Create header on the security-agreement object page
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree    - Pointer to the security-agreement object
 *         eType        - The type of the header to create
 * Output: phHeader     - The created header
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeCreateHeader(
									IN   SecAgree* 	      pSecAgree,
									IN   RvSipHeaderType  eType,
									OUT  void**           phHeader)
{
	RvStatus    rv;
	RvChar*     strHeaderName;

	switch (eType) {
	case RVSIP_HEADERTYPE_SECURITY:
		{
			strHeaderName = "Security";
			rv =  RvSipSecurityHeaderConstruct(pSecAgree->pSecAgreeMgr->hMsgMgr,
												pSecAgree->pSecAgreeMgr->hMemoryPool,
												pSecAgree->hPage,
												(RvSipSecurityHeaderHandle*)phHeader);
			break;
		}
	case RVSIP_HEADERTYPE_AUTHENTICATION:
		{
			strHeaderName = "Authentication";
			rv =  RvSipAuthenticationHeaderConstruct(pSecAgree->pSecAgreeMgr->hMsgMgr,
												      pSecAgree->pSecAgreeMgr->hMemoryPool,
													  pSecAgree->hPage,
													  (RvSipAuthenticationHeaderHandle*)phHeader);
			break;
		}
	default:
		return RV_ERROR_ILLEGAL_ACTION;
	}

	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"SecAgreeCreateHeader - Security-agreement 0x%p: Failed to construct %s Header, rv=%d",
					pSecAgree, strHeaderName, rv));
	}

	return rv;
}

/***************************************************************************
 * SecAgreeCopy
 * ------------------------------------------------------------------------
 * General: Copies local information from source security-agreement to
 *          destination security-agreement. The fields being copied here are:
 *          1. Local security list.
 *          2. Auth header for server security-agreement only.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pDestSecAgree   - Pointer to the destination security-agreement
 *         pSourceSecAgree - Pointer to the source security-agreement
 ***************************************************************************/

RvStatus RVCALLCONV SecAgreeCopy(
							 IN   SecAgree*      pDestSecAgree,
							 IN   SecAgree*      pSourceSecAgree)
{
	RvBool   bResetRemote;
	RvStatus rv;

	/* Copy role if it was not set in advance */
	if (pDestSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_UNDEFINED)
	{
		SecAgreeSetRole(pDestSecAgree, pSourceSecAgree->eRole);
	}

	/* Copy local security list and server authentication info */
	rv = SecAgreeSecurityDataCopy(pDestSecAgree->pSecAgreeMgr, pSourceSecAgree,
								  &pSourceSecAgree->securityData, &pDestSecAgree->securityData,
								  pDestSecAgree->eRole, pDestSecAgree->hPage, &bResetRemote);
	if (RV_OK == rv)
	{
		/* Copy local addresses */
		rv = CopyLocalAddresses(pDestSecAgree, pSourceSecAgree);
		if (RV_OK == rv)
		{
			/* Copy remote addresses */
			rv = CopyRemoteAddresses(pDestSecAgree, pSourceSecAgree);
		}
	}

	if (RV_OK != rv)
	{
		RvLogError(pDestSecAgree->pSecAgreeMgr->pLogSrc,(pDestSecAgree->pSecAgreeMgr->pLogSrc,
					"SecAgreeCopy - Security-agreement 0x%p: Failed to copy security data, rv=%d",
					pDestSecAgree, rv));
		/* Reset the data that was partially copied */
		ResetDataUponFailureToCopy(pDestSecAgree, bResetRemote);
		return rv;
	}

	/* Copy special standard support */
	pDestSecAgree->eSpecialStandard = pSourceSecAgree->eSpecialStandard;

	/* Copy application context */
	pDestSecAgree->pContext = pSourceSecAgree->pContext;
	
	/* Copy transaction indication */
	pDestSecAgree->securityData.cseqStep = pSourceSecAgree->securityData.cseqStep;

	return RV_OK;

}

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
RvStatus RVCALLCONV SecAgreeSetPortC(
							 IN   SecAgree*      pSecAgree,
							 IN   int portC)
{
	RvStatus rv=0;

	/* Set local security list */
	rv = SecAgreeSecurityDataSetPortC(&pSecAgree->securityData, portC);

	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"%s - Security-agreement 0x%p: Failed to set portC %d, rv=%d",
					__FUNCTION__,pSecAgree, portC, rv));
	}

	return rv;
}

#endif
/* SPIRENT_END */

/***************************************************************************
 * SecAgreeSetRole
 * ------------------------------------------------------------------------
 * General: Sets the given role to the security-agreement
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree - The security-agreement object
 ***************************************************************************/
void RVCALLCONV SecAgreeSetRole(IN   SecAgree*          pSecAgree,
								IN   RvSipSecAgreeRole  eRole)
{
	pSecAgree->eRole  = eRole;
	pSecAgree->eState = RVSIP_SEC_AGREE_STATE_IDLE;
}


/*------------ S E C U R I T Y  O B J E C T  F U N C T I O N S ------------*/

/***************************************************************************
 * SecAgreeObjectUpdateSecObj
 * ------------------------------------------------------------------------
 * General: Decides according to the inner state and the event being processed
 *          what actions to apply to the security object: initiate, activate
 *          or terminate.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecAgree   - Pointer to the security-agreement object
 *          eProcessing - Indicates the event being processed (message to send,
 *                        message received, etc)
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeObjectUpdateSecObj(
									  IN  SecAgree*                   pSecAgree,
									  IN  SecAgreeProcessing          eProcessing)
{
#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
	RvSipSecurityHeaderHandle       hIpsecHeader     = NULL;
	RvBool                          bIsTls           = RV_FALSE;
	RvBool                          bIsRemoteTls     = RV_FALSE;
	RvInt32                         openMechanisms   = 0;
	RvBool                          bSecObjInitiated = RV_FALSE;
	RvStatus                        rv               = RV_OK;

	/* Check whether to initiate the security object */
	if (SecAgreeStateShouldInitiateSecObj(pSecAgree, eProcessing) == RV_TRUE &&
		CanInitiateSecObj(pSecAgree) == RV_TRUE)
	{
		/* Check if ipsec should be initiated */
		rv = SecAgreeUtilsIsMechanismInList(pSecAgree->securityData.hLocalSecurityList, 
											RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP, NULL, &hIpsecHeader, NULL);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "SecAgreeObjectUpdateSecObj - security-agreement=0x%p: Failed to check local list for security object, rv=%d",
					   pSecAgree, rv));
			return rv;
		}

		/* Check if tls should be initiated */
		/* Check local list */
		rv = SecAgreeUtilsIsMechanismInList(pSecAgree->securityData.hLocalSecurityList, 
											RVSIP_SECURITY_MECHANISM_TYPE_TLS, &bIsTls, NULL, NULL);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "SecAgreeObjectUpdateSecObj - security-agreement=0x%p: Failed to check local list for security object, rv=%d",
					   pSecAgree, rv));
			return rv;
		}
		/* Check remote list */
		rv = SecAgreeUtilsIsMechanismInList(pSecAgree->securityData.hRemoteSecurityList, 
											RVSIP_SECURITY_MECHANISM_TYPE_TLS, &bIsRemoteTls, NULL, NULL);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "SecAgreeObjectUpdateSecObj - security-agreement=0x%p: Failed to check local list for security object, rv=%d",
					   pSecAgree, rv));
			return rv;
		}

		if (hIpsecHeader != NULL && SecAgreeUtilsIsIpsecNegOnly() == RV_FALSE)
		{
			/* There is ipsec in the local security list. Initiate security object for ipsec */
			openMechanisms = openMechanisms | RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP;
		}
		if (bIsTls == RV_TRUE ||
			(SecAgreeStateClientIsSpecialChoosing(pSecAgree, eProcessing) && bIsRemoteTls == RV_TRUE))
		{
			/* TLS is initiated if there is a header indicating it in the local list, or
			   if it is a client that has only remote list and is choosing to use TLS */
			openMechanisms = openMechanisms | RVSIP_SECURITY_MECHANISM_TYPE_TLS;
		}
		pSecAgree->securityData.secMechsToInitiate = openMechanisms;

		/* Ipsec was suggested by the client. Initiate security object for ipsec */
		rv = InitiateSecObj(pSecAgree, hIpsecHeader, &pSecAgree->securityData.hSecObj, &bSecObjInitiated);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "SecAgreeObjectUpdateSecObj - security-agreement=0x%p: Failed to initiate security object, rv=%d",
					   pSecAgree, rv));
			pSecAgree->securityData.hSecObj = NULL;
			return rv;
		}

		RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				  "SecAgreeObjectUpdateSecObj - security-agreement=0x%p: Security object 0x%p was initiated",
				  pSecAgree, pSecAgree->securityData.hSecObj));
	}

	/* Check whether to set security mechanism to the security object.
	   The client security-agreement first sets the mechanism and only then activates the
	   security object */
	if (pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_CLIENT &&
		ShouldSetMechanismToSecObj(pSecAgree, eProcessing))
	{
		/* Set security mechanism to the security object */
		rv = SipSecObjSetMechanism(pSecAgree->pSecAgreeMgr->hSecMgr, 
		  							pSecAgree->securityData.hSecObj, 
									pSecAgree->securityData.selectedSecurity.eType);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "SecAgreeObjectUpdateSecObj - security-agreement=0x%p: Failed to set mechanism to security object=0x%p, rv=%d",
					   pSecAgree, pSecAgree->securityData.hSecObj, rv));
			return rv;
		}
	}

	/* check whether to activate the security object */
	if (pSecAgree->securityData.hSecObj != NULL &&
		SecAgreeStateShouldActivateSecObj(pSecAgree, eProcessing) == RV_TRUE &&
		CanActivateSecObj(pSecAgree) == RV_TRUE)
	{
		RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				  "SecAgreeObjectUpdateSecObj - security-agreement=0x%p: Activating security object 0x%p",
				  pSecAgree, pSecAgree->securityData.hSecObj));

		/* get ipsec remote information from the security list */
		rv = SecAgreeUtilsIsMechanismInList(pSecAgree->securityData.hRemoteSecurityList, 
												RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP, NULL, &hIpsecHeader, NULL);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "SecAgreeObjectUpdateSecObj - security-agreement=0x%p: Failed to check local list for security object, rv=%d",
					   pSecAgree, rv));
			return rv;
		}
		
		/* Activate security object with ipsec */
		rv = ActivateSecObj(pSecAgree, pSecAgree->securityData.hSecObj, hIpsecHeader);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

    if(0 && pSecAgree->securityData.hSecObj && bSecObjInitiated)
    {
      RvSipSecObjHandle          hSecObj = pSecAgree->securityData.hSecObj;
      RvSipSecIpsecParams        ipsecGeneralParams;
      RvSipSecIpsecPeerParams    ipsecLocalParams;
      RvSipSecIpsecPeerParams    ipsecRemoteParams;
      
      RvSipSecObjGetIpsecParams(hSecObj, &ipsecGeneralParams, &ipsecLocalParams, &ipsecRemoteParams);
      // show general params:
      dprintf("%s:%d: pSecAgree=%p hSecObj=%p, General-Params: eEAlg=%d, eIAlg=%d, eTransportType=%d, lifetime=%d, IKlen=%d, CKlen=%d\n", 
        __FUNCTION__,__LINE__, pSecAgree, hSecObj, ipsecGeneralParams.eEAlg, ipsecGeneralParams.eIAlg, ipsecGeneralParams.eTransportType, 
        ipsecGeneralParams.lifetime, ipsecGeneralParams.IKlen, ipsecGeneralParams.CKlen);
      
      // show local params:
      dprintf("%s:%d: pSecAgree=%p hSecObj=%p, Local-Params: portC=%d, portS=%d, spiC=%d, spiS=%d\n", 
        __FUNCTION__,__LINE__, pSecAgree, hSecObj, ipsecLocalParams.portC, ipsecLocalParams.portS, ipsecLocalParams.spiC, ipsecLocalParams.spiS);
      
      // show remote params:
      dprintf("%s:%d: pSecAgree=%p hSecObj=%p, Remote-Params: portC=%d, portS=%d, spiC=%d, spiS=%d\n", 
        __FUNCTION__,__LINE__, pSecAgree, hSecObj, ipsecRemoteParams.portC, ipsecRemoteParams.portS, ipsecRemoteParams.spiC, ipsecRemoteParams.spiS);
    }

#endif
/* SPIRENT_END */

		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "SecAgreeObjectUpdateSecObj - security-agreement=0x%p: Failed to activate security object 0x%p, rv=%d",
					   pSecAgree, pSecAgree->securityData.hSecObj, rv));
			if (bSecObjInitiated == RV_TRUE)
			{
				/* The security object was created in this scope but could not activate. 
				   Terminate the security object */
				TerminateSecObj(pSecAgree, pSecAgree->securityData.hSecObj);
			}
			return rv;
		}

		RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				  "SecAgreeObjectUpdateSecObj - security-agreement=0x%p: Security object 0x%p was activated",
				  pSecAgree, pSecAgree->securityData.hSecObj));

	}
	
	/* check whether to set security mechanism to the security object */
	if (ShouldSetMechanismToSecObj(pSecAgree, eProcessing))
	{
		/* Set security mechanism to the security object if needed (server), and notify
		   the security-agreement owners that the mechanism is chosen */
		rv = UpdateSecObjMechanism(pSecAgree);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "SecAgreeObjectUpdateSecObj - security-agreement=0x%p: Failed to set mechanism to security object 0x%p, rv=%d",
					   pSecAgree, pSecAgree->securityData.hSecObj, rv));
			return rv;
		}
	}
	
	/* Check if there is no more need in the security object and terminate it accordingly */
	if (ShouldTerminateSecObj(pSecAgree, eProcessing) == RV_TRUE)
	{
		TerminateSecObj(pSecAgree, pSecAgree->securityData.hSecObj);
	}

	return rv;
#else /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */
	RV_UNUSED_ARG(pSecAgree);
	RV_UNUSED_ARG(eProcessing);
	return RV_OK;
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */
}

/***************************************************************************
 * SecAgreeObjectSupplySecObjToOwners
 * ------------------------------------------------------------------------
 * General: Go over the owners of this security-agreement and supply them
 *          the security object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree    - Handle to the security-agreement
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeObjectSupplySecObjToOwners(
											IN   SecAgree*    pSecAgree)
{
#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
	SecAgreeMgr*              pSecAgreeMgr = pSecAgree->pSecAgreeMgr;
	RLIST_HANDLE              hTempList;
	RLIST_ITEM_HANDLE         hListElem;
	SecAgreeOwnerRecord*      pSecAgreeOwner;
	RvInt                     safeCounter = 0;
	RvStatus                  rv;

	/* We copy the list of owners because when we call an owner's callbacks, we unlock the 
	   security-agreement and then the list of owners might change */
	hTempList = RLIST_ListConstruct(pSecAgree->pSecAgreeMgr->hSecAgreeOwnersPool);
	if (hTempList == NULL)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"SecAgreeObjectSupplySecObjToOwners - Security-agreement 0x%p: Failed to construct temporary list",
					pSecAgree));
		return RV_ERROR_OUTOFRESOURCES;
	}
	
	rv = CopyOwnersList(pSecAgree, hTempList);
	if (RV_OK != rv)
	{
		RLIST_ListDestruct(pSecAgree->pSecAgreeMgr->hSecAgreeOwnersPool, hTempList);
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"SecAgreeObjectSupplySecObjToOwners - Security-agreement 0x%p: Failed while preparing to notify owners on security-object activation, rv=%d",
					pSecAgree, rv));
		return rv;
	}	

	/* Go over the list of owners and notify them to attach to the security object */
	RLIST_GetHead(pSecAgreeMgr->hSecAgreeOwnersPool, pSecAgree->hOwnersList, &hListElem);
	while (hListElem != NULL && safeCounter < SEC_AGREE_MAX_LOOP(pSecAgree))
	{
		pSecAgreeOwner = (SecAgreeOwnerRecord*)hListElem;
		rv = SecAgreeCallbacksAttachSecObjEv(pSecAgree, pSecAgreeOwner);
		if (RV_OK != rv)
		{
			RvLogWarning(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					     "SecAgreeObjectSupplySecObjToOwners - Security-agreement 0x%p: Failed to notify owner=0x%p to attach to security-object=0x%p, rv=%d",
					     pSecAgree, pSecAgreeOwner->owner.pOwner, pSecAgree->securityData.hSecObj, rv));
			/* we do not quite even if there is a failure with one owner */
			rv = RV_OK;
		}

		/* We recheck the state of the security-agreement, because the security-agreement object was unlocked
		   during the callback call */
		if (SecAgreeStateIsValid(pSecAgree) == RV_FALSE)
		{
			/* The security-agreement became invalid. Cease notifications */
			RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					  "SecAgreeObjectSupplySecObjToOwners - Security-agreement 0x%p: turned invalid, cease notifying owner on the security-object",
					  pSecAgree));
			RLIST_ListDestruct(pSecAgree->pSecAgreeMgr->hSecAgreeOwnersPool, hTempList);
			return RV_OK;
		}

		RLIST_GetNext(pSecAgreeMgr->hSecAgreeOwnersPool, pSecAgree->hOwnersList, hListElem, &hListElem);

		safeCounter += 1;
	}

	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"SecAgreeObjectSupplySecObjToOwners - Security-agreement 0x%p: Failed to notify owners to attach to security-object, rv=%d",
					pSecAgree, rv));
	}

	RLIST_ListDestruct(pSecAgree->pSecAgreeMgr->hSecAgreeOwnersPool, hTempList);
	
	return rv;
#else /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */
	RV_UNUSED_ARG(pSecAgree);
	return RV_OK;
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */
}

/*------------------- N E T W O R K   F U N C T I O N S  -----------------*/

#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
/***************************************************************************
 * SecAgreeObjectSetLocalAddr
 * ------------------------------------------------------------------------
 * General: Sets the given local address to the security-agreement
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecAgree     - Pointer to the security-agreement object
 *          pLocalAddress - The local address to set
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeObjectSetLocalAddr(
									  IN   SecAgree*            pSecAgree,
									  IN   RvSipTransportAddr*  pLocalAddress)
{
	RvStatus   rv = RV_OK;
	
	RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
              "SecAgreeObjectSetLocalAddr - Security-agreement 0x%p - Setting %s %s local address %s:%d",
               pSecAgree,
               SipCommonConvertEnumToStrTransportType(pLocalAddress->eTransportType),
               SipCommonConvertEnumToStrTransportAddressType(pLocalAddress->eAddrType),
               pLocalAddress->strIP,pLocalAddress->port));

  //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
  pSecAgree->addresses.eTransportType = pLocalAddress->eTransportType;
#endif
  //SPIRENT_END

#if (RV_TLS_TYPE != RV_TLS_NONE)
	if (pLocalAddress->eTransportType == RVSIP_TRANSPORT_TLS)
	{
		RvSipTransportLocalAddrHandle  hLocalAddr;

		rv = SipTransportLocalAddressGetHandleByAddress(pSecAgree->pSecAgreeMgr->hTransport,
														RVSIP_TRANSPORT_TLS,
														pLocalAddress->eAddrType,
														pLocalAddress->strIP,
														pLocalAddress->port,
														&hLocalAddr);

		if(hLocalAddr == NULL)
		{
			rv = RV_ERROR_NOT_FOUND;
		}
		if(rv != RV_OK)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
						"SecAgreeObjectSetLocalAddr - Security-agreement 0x%p - Failed to find the requested TLS local address",
						pSecAgree));
			return rv;
		}
		
		if (pLocalAddress->eAddrType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
		{
			pSecAgree->addresses.hLocalTlsIp4Addr = hLocalAddr;
		}
		else if (pLocalAddress->eAddrType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP6)
		{
			pSecAgree->addresses.hLocalTlsIp6Addr = hLocalAddr;
		}
		return RV_OK;
	} 
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/
		
#if (RV_IMS_IPSEC_ENABLED==RV_YES)		
	if (pLocalAddress->eTransportType == RVSIP_TRANSPORT_UDP ||
		pLocalAddress->eTransportType == RVSIP_TRANSPORT_TCP)
	{
        /* Take an IP in string format */
		rv = AllocAndCopyAddress(pSecAgree->pSecAgreeMgr->hMemoryPool, pSecAgree->hPage, 
								 pLocalAddress->strIP, &pSecAgree->addresses.pLocalIpsecIp);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
						"SecAgreeObjectSetLocalAddr - Security-agreement 0x%p - Failed to copy the given local address",
						pSecAgree));
		}
        /* Take type of address */
        pSecAgree->addresses.localIpsecAddrType = pLocalAddress->eAddrType;
        /* Take Scope ID for IPv6 address */
#if (RV_NET_TYPE & RV_NET_IPV6)
        pSecAgree->addresses.localIpsecIpv6Scope = pLocalAddress->Ipv6Scope;
#endif /*#if (RV_NET_TYPE & RV_NET_IPV6)*/
        return rv;
	}
#endif /* #if (RV_IMS_IPSEC_ENABLED==RV_YES) */

	RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
			   "SecAgreeObjectSetLocalAddr - Security-agreement 0x%p - Bad local address supplied as parameter",
			   pSecAgree));
	return RV_ERROR_BADPARAM;
}

/***************************************************************************
 * SecAgreeObjectSetRemoteAddr
 * ------------------------------------------------------------------------
 * General: Sets the given remote address to the security-agreement
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecAgree      - Pointer to the security-agreement object
 *          pRemoteAddress - The remote address to set
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeObjectSetRemoteAddr(
									  IN   SecAgree*            pSecAgree,
									  IN   RvSipTransportAddr*  pRemoteAddress)
{
	SipTransportAddress   convertedAddr;
	RvAddress            *pSecAgreeAddress = NULL;
	RvAddress            *pRes             = NULL;
	RvBool                bIsIp4;
	RvStatus              rv;

    if (pRemoteAddress->eTransportType != RVSIP_TRANSPORT_TLS &&
        pRemoteAddress->eTransportType != RVSIP_TRANSPORT_UDP &&
		pRemoteAddress->eTransportType != RVSIP_TRANSPORT_TCP)
	{
		/* We set remote addresses only for TLS or ipsec */
		return RV_OK;
	}

	RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
              "SecAgreeObjectSetRemoteAddr - Security-agreement 0x%p - Setting %s remote address %s:%d",
               pSecAgree,
               SipCommonConvertEnumToStrTransportType(pRemoteAddress->eTransportType),
               pRemoteAddress->strIP,pRemoteAddress->port));

	/* Identify the address to update (TLS or ipsec) */
#if (RV_TLS_TYPE != RV_TLS_NONE)
	if (pRemoteAddress->eTransportType == RVSIP_TRANSPORT_TLS)
	{
		pSecAgreeAddress = &pSecAgree->addresses.remoteTlsAddr;
	}
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_IMS_IPSEC_ENABLED==RV_YES)	
	if (pRemoteAddress->eTransportType == RVSIP_TRANSPORT_UDP ||
		pRemoteAddress->eTransportType == RVSIP_TRANSPORT_TCP)
	{
		pSecAgreeAddress = &pSecAgree->addresses.remoteIpsecAddr;
		/* Set transport */
		pSecAgree->addresses.eRemoteIpsecTransportType = pRemoteAddress->eTransportType;
	}
#endif /* #if (RV_IMS_IPSEC_ENABLED==RV_YES) */

	if (NULL == pSecAgreeAddress)
	{
		return RV_OK;
	}

	/* Construct core address */
	if (pRemoteAddress->eAddrType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP6)
	{
		bIsIp4 = RV_FALSE;
	}
	else
	{
		bIsIp4 = RV_TRUE;
	}

	rv = SipTransportString2Address(pRemoteAddress->strIP, &convertedAddr, bIsIp4);
	if (RV_OK == rv)
	{
		pRes = RvAddressCopy(&convertedAddr.addr, pSecAgreeAddress);
		if (pRes != NULL)
		{
			pRes = RvAddressSetIpPort(pSecAgreeAddress, pRemoteAddress->port);
		}
	}
	
	if (NULL == pRes || RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"SecAgreeObjectSetRemoteAddr - Security-agreement 0x%p - Failed to set the given remote address",
					pSecAgree));
		return RV_ERROR_UNKNOWN;
	}

	return RV_OK;
}
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */


/*------------------ L O C K   F U N C T I O N S -----------------*/

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/************************************************************************************
 * SecAgreeLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks the security-agreement according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree - pointer to the security-agreement object
***********************************************************************************/
RvStatus RVCALLCONV SecAgreeLockAPI(IN  SecAgree*   pSecAgree)
{
    RvStatus           retCode;
    SipTripleLock     *tripleLock;
    SecAgreeMgr       *pSecAgreeMgr;
    RvInt32            identifier;

    if (pSecAgree == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    for(;;)
    {
        RvMutexLock(&pSecAgree->secAgreeTripleLock.hLock,pSecAgree->pSecAgreeMgr->pLogMgr);
        pSecAgreeMgr = pSecAgree->pSecAgreeMgr;
        tripleLock   = pSecAgree->tripleLock;
        identifier   = pSecAgree->secAgreeUniqueIdentifier;
        RvMutexUnlock(&pSecAgree->secAgreeTripleLock.hLock,pSecAgree->pSecAgreeMgr->pLogMgr);

        if (tripleLock == NULL)
        {
            return RV_ERROR_NULLPTR;
        }

        RvLogSync(pSecAgreeMgr->pLogSrc, (pSecAgreeMgr->pLogSrc,
				  "SecAgreeLockAPI - Locking security-agreement 0x%p: Triple Lock 0x%p",
				  pSecAgree, tripleLock));

        retCode = SecAgreeLock(pSecAgree, pSecAgreeMgr, tripleLock, identifier);
        if (retCode != RV_OK)
        {
            return retCode;
        }

        /*make sure the triple lock was not changed after just before the locking*/
        if (pSecAgree->tripleLock == tripleLock)
        {
            break;
        }

        RvLogWarning(pSecAgreeMgr->pLogSrc ,(pSecAgreeMgr->pLogSrc ,
					"SecAgreeLockAPI - Locking security-agreement 0x%p: Tripple lock 0x%p was changed, reentering lockAPI",
					pSecAgree,tripleLock));

        SecAgreeUnLock(pSecAgreeMgr->pLogMgr, &tripleLock->hLock);
    }

    retCode = CRV2RV(RvSemaphoreTryWait(&tripleLock->hApiLock,NULL));
    if ((retCode != RV_ERROR_TRY_AGAIN) && (retCode != RV_OK))
    {
        SecAgreeUnLock(pSecAgree->pSecAgreeMgr->pLogMgr, &tripleLock->hLock);
        return retCode;
    }

    tripleLock->apiCnt++;

    /* This thread actually locks the API lock */
    if(tripleLock->objLockThreadId == 0)
    {
        tripleLock->objLockThreadId = RvThreadCurrentId();
        RvMutexGetLockCounter(&tripleLock->hLock, pSecAgree->pSecAgreeMgr->pLogMgr,
                              &tripleLock->threadIdLockCount);
    }

    RvLogSync(pSecAgreeMgr->pLogSrc ,(pSecAgreeMgr->pLogSrc ,
             "SecAgreeLockAPI - Locking security-agreement 0x%p: Triple Lock 0x%p apiCnt=%d exiting function",
             pSecAgree, tripleLock, tripleLock->apiCnt));

    return RV_OK;
}


/************************************************************************************
 * SecAgreeUnlockAPI
 * ----------------------------------------------------------------------------------
 * General: Unlocks security-agreement according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree       - pointer to the security-agreement
 ***********************************************************************************/
void RVCALLCONV SecAgreeUnLockAPI(IN  SecAgree*   pSecAgree)
{
    SipTripleLock         *tripleLock;
    SecAgreeMgr           *pSecAgreeMgr;
    RvInt32                lockCnt = 0;

    if (pSecAgree == NULL)
    {
        return;
    }

    pSecAgreeMgr = pSecAgree->pSecAgreeMgr;
    tripleLock = pSecAgree->tripleLock;

    if ((pSecAgreeMgr == NULL) || (tripleLock == NULL))
    {
        return;
    }

    RvMutexGetLockCounter(&tripleLock->hLock, pSecAgreeMgr->pLogMgr, &lockCnt);

    RvLogSync(pSecAgreeMgr->pLogSrc ,(pSecAgreeMgr->pLogSrc ,
             "SecAgreeUnLockAPI - UnLocking security-agreement 0x%p: Triple Lock 0x%p apiCnt=%d lockCnt=%d",
             pSecAgree, tripleLock, tripleLock->apiCnt, lockCnt));


    if (tripleLock->apiCnt == 0)
    {
        RvLogExcep(pSecAgreeMgr->pLogSrc ,(pSecAgreeMgr->pLogSrc ,
                   "SecAgreeUnLockAPI - UnLocking call 0x%p: Triple Lock 0x%p, apiCnt=%d",
                   pSecAgree, tripleLock, tripleLock->apiCnt));
        return;
    }

    if (tripleLock->apiCnt == 1)
    {
        RvSemaphorePost(&tripleLock->hApiLock,NULL);
    }

    tripleLock->apiCnt--;
    /*reset the thread id in the counter that set it previously*/
    if(lockCnt == tripleLock->threadIdLockCount)
    {
        tripleLock->objLockThreadId = 0;
        tripleLock->threadIdLockCount = UNDEFINED;
    }

    SecAgreeUnLock(pSecAgreeMgr->pLogMgr, &tripleLock->hLock);
}


/************************************************************************************
 * SecAgreeLockMsg
 * ----------------------------------------------------------------------------------
 * General: Locks security-agreement according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree - pointer to the security-agreement.
***********************************************************************************/
RvStatus RVCALLCONV SecAgreeLockMsg(IN  SecAgree*   pSecAgree)
{
    RvStatus          retCode      = RV_OK;
    RvBool            didILockAPI  = RV_FALSE;
    RvThreadId        currThreadId = RvThreadCurrentId();
    SipTripleLock    *tripleLock;
    SecAgreeMgr*      pSecAgreeMgr;
    RvInt32           identifier;

    if (pSecAgree == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    RvMutexLock(&pSecAgree->secAgreeTripleLock.hLock, pSecAgree->pSecAgreeMgr->pLogMgr);
    tripleLock   = pSecAgree->tripleLock;
    pSecAgreeMgr = pSecAgree->pSecAgreeMgr;
    identifier   = pSecAgree->secAgreeUniqueIdentifier;
    RvMutexUnlock(&pSecAgree->secAgreeTripleLock.hLock,pSecAgree->pSecAgreeMgr->pLogMgr);

    if (tripleLock == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    /* In case that the current thread has already gained the API lock of */
    /* the object, locking again will be useless and will block the stack */
    if (tripleLock->objLockThreadId == currThreadId)
    {
        RvLogSync(pSecAgreeMgr->pLogSrc ,(pSecAgreeMgr->pLogSrc ,
                  "SecAgreeLockMsg - Exiting without locking security-agreement 0x%p: Triple Lock 0x%p, apiCnt=%d already locked",
                   pSecAgree, tripleLock, tripleLock->apiCnt));

        return RV_OK;
    }

    RvMutexLock(&tripleLock->hProcessLock, pSecAgree->pSecAgreeMgr->pLogMgr);

    for(;;)
    {
        retCode = SecAgreeLock(pSecAgree, pSecAgreeMgr, tripleLock, identifier);
        if (retCode != RV_OK)
        {
          RvMutexUnlock(&tripleLock->hProcessLock, pSecAgree->pSecAgreeMgr->pLogMgr);
          if (didILockAPI)
          {
            RvSemaphorePost(&tripleLock->hApiLock,NULL);
          }

          return retCode;
        }

        if (tripleLock->apiCnt == 0)
        {
            if (didILockAPI)
            {
              RvSemaphorePost(&tripleLock->hApiLock,NULL);
            }
            break;
        }

        SecAgreeUnLock(pSecAgree->pSecAgreeMgr->pLogMgr, &tripleLock->hLock);

        retCode = RvSemaphoreWait(&tripleLock->hApiLock,NULL);
        if (retCode != RV_OK)
        {
          RvMutexUnlock(&tripleLock->hProcessLock,pSecAgree->pSecAgreeMgr->pLogMgr);
          return retCode;
        }

        didILockAPI = RV_TRUE;
    }

    RvLogSync(pSecAgreeMgr->pLogSrc ,(pSecAgreeMgr->pLogSrc ,
             "SecAgreeLockMsg - Locking security-agreement 0x%p: Triple Lock 0x%p exiting function",
             pSecAgree, tripleLock));

    return retCode;
}


/************************************************************************************
 * SecAgreeUnLockMsg
 * ----------------------------------------------------------------------------------
 * General: UnLocks security-agreement according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree - pointer to the security-agreement.
***********************************************************************************/
void RVCALLCONV SecAgreeUnLockMsg(IN  SecAgree*   pSecAgree)
{
    SipTripleLock    *tripleLock;
    SecAgreeMgr      *pSecAgreeMgr;
    RvThreadId        currThreadId = RvThreadCurrentId();

    if (pSecAgree == NULL)
    {
        return;
    }

    pSecAgreeMgr       = pSecAgree->pSecAgreeMgr;
    tripleLock = pSecAgree->tripleLock;

    if ((pSecAgreeMgr == NULL) || (tripleLock == NULL))
    {
        return;
    }

    /* In case that the current thread has already gained the API lock of */
    /* the object wasn't lock internally, thus it shouldn't be unlocked   */
    if (tripleLock->objLockThreadId == currThreadId)
    {
        RvLogSync(pSecAgreeMgr->pLogSrc ,(pSecAgreeMgr->pLogSrc ,
                  "SecAgreeUnLockMsg - Exiting without unlocking security-agreement 0x%p: Triple Lock 0x%p already locked",
                   pSecAgree, tripleLock));

        return;
    }

    RvLogSync(pSecAgreeMgr->pLogSrc ,(pSecAgreeMgr->pLogSrc ,
              "SecAgreeUnLockMsg - UnLocking call 0x%p: Triple Lock 0x%p",
			  pSecAgree, tripleLock));

    SecAgreeUnLock(pSecAgree->pSecAgreeMgr->pLogMgr, &tripleLock->hLock);
    RvMutexUnlock(&tripleLock->hProcessLock, pSecAgree->pSecAgreeMgr->pLogMgr);
}
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * SecAgreeObjectSetSendOptions
 * ------------------------------------------------------------------------
 * General: Sets the sending options of the security agreement. The sending
 *          options affect the message sending of the security object, for
 *          example may be used to instruct the security-object to send 
 *          messages compressed with sigcomp. The send options shall be set
 *          in parallel to setting the remote address via 
 *          RvSipSecAgreeSetRemoteAddr (or deciding that the default remote
 *          address shall be used.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree    - The security-agreement object
 *            pOptions	   - Pointer to the configured options structure.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeObjectSetSendOptions(
                                     IN  SecAgree                   *pSecAgree,
									 IN  RvSipSecAgreeSendOptions   *pOptions)
{
	RvStatus rv = RV_OK;

#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
	RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				"RvSipSecAgreeSetSendOptions - Security-agreement 0x%p: setting send options (comp=%d,sigcomp-id=%s)",
				pSecAgree,pOptions->eMsgCompType,pOptions->strSigcompId));
	
	pSecAgree->addresses.eMsgCompType = pOptions->eMsgCompType;
	if (pOptions->strSigcompId != NULL)
	{
		RPOOL_ITEM_HANDLE   temp;
		RvInt32             len = strlen(pOptions->strSigcompId);
    
		if (len > RVSIP_COMPARTMENT_MAX_URN_LENGTH-1)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "RvSipSecAgreeSetSendOptions - Security-agreement 0x%p: given sigcomp-id is too long (len=%d, maximal-len=%d)",
					   pSecAgree, len, RVSIP_COMPARTMENT_MAX_URN_LENGTH-1));
			return RV_ERROR_BADPARAM;
		}
		rv = RPOOL_AppendFromExternal(pSecAgree->pSecAgreeMgr->hMemoryPool, pSecAgree->hPage, 
									  pOptions->strSigcompId, len+1, 
									  RV_FALSE, &pSecAgree->addresses.strSigcompId, &temp);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "RvSipSecAgreeSetSendOptions - Security-agreement 0x%p: Failed in calling RPOOL_AppendFromExternal",
					   pSecAgree));
			return rv;
		}
	}

	if (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_ACTIVE)
	{
		/* In case the security-agreement is in the active state, we set the send options
		   directly to the security-object */
		rv = SetSendOptionsToSecObj(pSecAgree, pSecAgree->securityData.hSecObj);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "RvSipSecAgreeSetSendOptions - Security-agreement 0x%p: Failed to set send options to security-object 0x%p",
					   pSecAgree, pSecAgree->securityData.hSecObj));
			return rv;
		}
	}
#else /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */
	RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
			   "RvSipSecAgreeSetSendOptions - Security-agreement 0x%p: calling this function is illegal when compiled with no ipsec or tls",
			   pSecAgree));
	RV_UNUSED_ARG(pOptions);
	rv = RV_ERROR_ILLEGAL_ACTION;
#endif
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecAgree)
#endif

	return rv;
}

/***************************************************************************
 * SecAgreeObjectGetSendOptions
 * ------------------------------------------------------------------------
 * General: Gets the sending options of the security agreement. 
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree    - The security-agreement object
 *            pOptions	   - Pointer to the configured options structure.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeObjectGetSendOptions(
                                    IN  SecAgree                   *pSecAgree,
									IN  RvSipSecAgreeSendOptions   *pOptions)
{
	RvStatus		rv = RV_OK;
	
#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
	pOptions->eMsgCompType = pSecAgree->addresses.eMsgCompType;
	if (pOptions->strSigcompId != NULL)
	{
		pOptions->strSigcompId[0] = '\0';
		if (pSecAgree->addresses.strSigcompId != UNDEFINED)
		{
			RvInt32   len;

			len = RPOOL_Strlen(pSecAgree->pSecAgreeMgr->hMemoryPool, pSecAgree->hPage, 
							   pSecAgree->addresses.strSigcompId);
			rv = RPOOL_CopyToExternal(pSecAgree->pSecAgreeMgr->hMemoryPool, pSecAgree->hPage, 
									  pSecAgree->addresses.strSigcompId, pOptions->strSigcompId, len+1);
			if (RV_OK != rv)
			{
				RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
						   "RvSipSecAgreeGetSendOptions - Security-agreement 0x%p: Failed in RPOOL_CopyToExternal. rv=%d",
						   pSecAgree, rv));
				return rv;
			}
		}
	}
#else /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */
	RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
			   "RvSipSecAgreeGetSendOptions - Security-agreement 0x%p: calling this function is illegal when compiled with no ipsec or tls",
			   pSecAgree));
	rv = RV_ERROR_ILLEGAL_ACTION;
	RV_UNUSED_ARG(pOptions);
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecAgree)
#endif
	
	return rv;
}

#endif /* #ifdef RV_SIGCOMP_ON */

/*------------------------------------------------------------------------
           S T A T I C   F U N C T I O N S
 -------------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeDetachAllOwners
 * ------------------------------------------------------------------------
 * General: Go over the owners of this security-agreement and detach them.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree    - Handle to the security-agreement
 ***************************************************************************/
static RvStatus RVCALLCONV SecAgreeDetachAllOwners(
											IN   SecAgree*    pSecAgree)
{
	SecAgreeOwnerRecord *pSecAgreeOwner;
	RvInt                safeCounter = 0;
	RvStatus             rv;

	/* Go over the list of owners and notify them to detach from the old security-agreement */
	rv = GetOwner(pSecAgree, &pSecAgreeOwner);
	while (RV_OK == rv && pSecAgreeOwner != NULL && safeCounter < SEC_AGREE_MAX_LOOP(pSecAgree))
	{
		SecAgreeRemoveOwner(pSecAgree, pSecAgreeOwner);

		SecAgreeCallbacksDetachSecAgreeEv(pSecAgree, pSecAgreeOwner);

		rv = GetOwner(pSecAgree, &pSecAgreeOwner);

		safeCounter += 1;
	}
	if (pSecAgree->hOwnersList != NULL)
	{
		RLIST_ListDestruct(pSecAgree->pSecAgreeMgr->hSecAgreeOwnersPool, pSecAgree->hOwnersList);
	}

	return rv;
}

/***************************************************************************
 * GetOwner
 * ------------------------------------------------------------------------
 * General: Returns the first owner of this security-agreement
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement
 * Output: ppOwnerRec - Pointer to the owner record.
 ***************************************************************************/
static RvStatus RVCALLCONV GetOwner(
							 IN  SecAgree*              pSecAgree,
							 OUT SecAgreeOwnerRecord**  ppOwnerRec)
{
	RLIST_ITEM_HANDLE		  hListElem;
	
	if (pSecAgree->hOwnersList == NULL)
	{
		*ppOwnerRec = NULL;
		return RV_OK;
	}

	RLIST_GetHead(pSecAgree->pSecAgreeMgr->hSecAgreeOwnersPool, pSecAgree->hOwnersList, &hListElem);
	*ppOwnerRec = (SecAgreeOwnerRecord*)hListElem;
	
	return RV_OK;
}

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/***************************************************************************
 * SecAgreeLock
 * ------------------------------------------------------------------------
 * General: Locks security-agreement.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree  - pointer to security-agreement to be locked
 *            pMgr       - pointer to security-agreement manager
 *            hLock      - security-agreement lock
 *            identifier - the security-agreement unique identifier
 ***************************************************************************/
static RvStatus RVCALLCONV SecAgreeLock(IN SecAgree          *pSecAgree,
                                        IN SecAgreeMgr       *pSecAgreeMgr,
                                        IN SipTripleLock     *tripleLock,
                                        IN RvInt32            identifier)
{


    if ((pSecAgree == NULL) || (tripleLock == NULL) || (pSecAgreeMgr == NULL))
    {
        return RV_ERROR_BADPARAM;
    }

	/*lock the security-agreement object*/
    RvMutexLock(&tripleLock->hLock, pSecAgreeMgr->pLogMgr);

    if (pSecAgree->secAgreeUniqueIdentifier != identifier ||
        pSecAgree->secAgreeUniqueIdentifier == 0)
    {
        /*The security-agreement is invalid*/
        RvLogWarning(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
                      "SecAgreeLock - Security-agreement 0x%p: RLIST_IsElementVacant returned RV_TRUE",
                      pSecAgree));
        RvMutexUnlock(&tripleLock->hLock, pSecAgreeMgr->pLogMgr);
        return RV_ERROR_DESTRUCTED;
    }

    return RV_OK;
}

/***************************************************************************
 * SecAgreeUnLock
 * ------------------------------------------------------------------------
 * General: Unlocks a given lock.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pLogMgr  - The log mgr pointer
 *            hLock    - A lock to unlock.
 ***************************************************************************/
static void RVCALLCONV SecAgreeUnLock(IN  RvLogMgr *pLogMgr,
                                      IN  RvMutex  *hLock)
{
	/*unlock the security-agreement */
    RvMutexUnlock(hLock, pLogMgr);
}

#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

/***************************************************************************
 * InitAddresses
 * ------------------------------------------------------------------------
 * General: Initializes the addresses structure of a new security agreement
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree - The security-agreement to initialize
 ***************************************************************************/
static void RVCALLCONV InitAddresses(IN  SecAgree     *pSecAgree)
{
#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
	memset(&pSecAgree->addresses, 0, sizeof(SecAgreeAddresses));
#ifdef RV_SIGCOMP_ON
	pSecAgree->addresses.eMsgCompType = RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED;
	pSecAgree->addresses.strSigcompId = UNDEFINED;
#endif /* #ifdef RV_SIGCOMP_ON */
#endif /* (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */

#if (RV_IMS_IPSEC_ENABLED == RV_YES) 
	pSecAgree->addresses.eRemoteIpsecTransportType = RVSIP_TRANSPORT_UNDEFINED;
    pSecAgree->addresses.localIpsecAddrType        = RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED;
	pSecAgree->addresses.localIpsecIpv6Scope       = UNDEFINED;
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES)  */

#if (RV_IMS_IPSEC_ENABLED != RV_YES) && (RV_TLS_TYPE == RV_TLS_NONE)
	RV_UNUSED_ARG(pSecAgree);
#endif	
}

/***************************************************************************
 * CopyLocalAddresses
 * ------------------------------------------------------------------------
 * General: Copies the local address information from one security-agreement
 *          to another
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Inout:    pDestSecAgree   - The destination security-agreement
 * Input:    pSourceSecAgree - The source security-agreement
 ***************************************************************************/
static RvStatus RVCALLCONV CopyLocalAddresses(INOUT SecAgree*    pDestSecAgree,
											  IN    SecAgree*    pSourceSecAgree)
{
#if (RV_IMS_IPSEC_ENABLED == RV_YES)
	RvStatus    rv;

	rv = AllocAndCopyAddress(pDestSecAgree->pSecAgreeMgr->hMemoryPool, pDestSecAgree->hPage, 
							 pSourceSecAgree->addresses.pLocalIpsecIp, &pDestSecAgree->addresses.pLocalIpsecIp);
	if (RV_OK != rv)
	{
		return rv;
	}
    /* Copy type of address */
    pDestSecAgree->addresses.localIpsecAddrType = pSourceSecAgree->addresses.localIpsecAddrType;
    /* Copy Scope ID for IPv6 address */
#if (RV_NET_TYPE & RV_NET_IPV6)
    pDestSecAgree->addresses.localIpsecIpv6Scope = pSourceSecAgree->addresses.localIpsecIpv6Scope;
#endif /*#if (RV_NET_TYPE & RV_NET_IPV6)*/

	pDestSecAgree->addresses.hLocalIpsecUdpAddr = pSourceSecAgree->addresses.hLocalIpsecUdpAddr;
	pDestSecAgree->addresses.hLocalIpsecTcpAddr = pSourceSecAgree->addresses.hLocalIpsecTcpAddr;
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */

#if (RV_TLS_TYPE != RV_TLS_NONE)
	pDestSecAgree->addresses.hLocalTlsIp4Addr   = pSourceSecAgree->addresses.hLocalTlsIp4Addr;
	pDestSecAgree->addresses.hLocalTlsIp6Addr   = pSourceSecAgree->addresses.hLocalTlsIp6Addr;
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */

#if (RV_IMS_IPSEC_ENABLED != RV_YES) && (RV_TLS_TYPE == RV_TLS_NONE)
	RV_UNUSED_ARG(pDestSecAgree);
	RV_UNUSED_ARG(pSourceSecAgree);
#endif
	return RV_OK;
}

/***************************************************************************
 * CopyRemoteAddresses
 * ------------------------------------------------------------------------
 * General: Copies the remote address information from one security-agreement
 *          to another
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Inout:    pDestSecAgree   - The destination security-agreement
 * Input:    pSourceSecAgree - The source security-agreement
 ***************************************************************************/
static RvStatus RVCALLCONV CopyRemoteAddresses(INOUT SecAgree*    pDestSecAgree,
											   IN    SecAgree*    pSourceSecAgree)
{
#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
	RvAddress  *pRes = NULL;
#endif

#if (RV_IMS_IPSEC_ENABLED == RV_YES)
	pRes = RvAddressCopy(&pSourceSecAgree->addresses.remoteIpsecAddr,
						 &pDestSecAgree->addresses.remoteIpsecAddr);
	if (NULL == pRes)
	{
		return RV_ERROR_UNKNOWN;
	}

	pDestSecAgree->addresses.eRemoteIpsecTransportType = pSourceSecAgree->addresses.eRemoteIpsecTransportType;
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */
	
#if (RV_TLS_TYPE != RV_TLS_NONE)
	pRes = RvAddressCopy(&pSourceSecAgree->addresses.remoteTlsAddr,
						 &pDestSecAgree->addresses.remoteTlsAddr);
	if (NULL == pRes)
	{
		return RV_ERROR_UNKNOWN;
	}
#endif 
	
#if (RV_IMS_IPSEC_ENABLED != RV_YES) && (RV_TLS_TYPE == RV_TLS_NONE)
	RV_UNUSED_ARG(pDestSecAgree);
	RV_UNUSED_ARG(pSourceSecAgree);
#endif

	return RV_OK;
}

#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
/***************************************************************************
 * CopyOwnersList
 * ------------------------------------------------------------------------
 * General: Copies the inner owners list of the security-agreement onto the 
 *          given list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree    - Handle to the security-agreement
 * Inout:  hTempList    - Handle to the destination list
 ***************************************************************************/
static RvStatus RVCALLCONV CopyOwnersList(IN    SecAgree*			    pSecAgree,
										  INOUT RLIST_HANDLE            hTempList)
{
	RLIST_ITEM_HANDLE         hListElem     = NULL;
	RLIST_ITEM_HANDLE         hTempListElem = NULL;
	SecAgreeOwnerRecord*      pOwnerRec     = NULL;
	SecAgreeOwnerRecord*      pTempOwnerRec = NULL;
	RvInt                     safeCounter   = 0;
	RvStatus                  rv            = RV_OK;

	/* Get first owner from the security-agreement list */
	RLIST_GetHead(pSecAgree->pSecAgreeMgr->hSecAgreeOwnersPool, pSecAgree->hOwnersList, &hListElem);
	if (NULL == hListElem)
	{
		return RV_OK;
	}
	pOwnerRec = (SecAgreeOwnerRecord*)hListElem;

	/* Allocate first owner in the temporary list */
	rv = RLIST_InsertHead(pSecAgree->pSecAgreeMgr->hSecAgreeOwnersPool, hTempList, &hTempListElem);
	if (RV_OK != rv)
	{
		return rv;
	}
	pTempOwnerRec = (SecAgreeOwnerRecord*)hTempListElem;

	/* Copy owner record to the temporary record */
	memcpy(pTempOwnerRec, pOwnerRec, sizeof(SecAgreeOwnerRecord));

	/* Get next owner from the security-agreement list */
	RLIST_GetNext(pSecAgree->pSecAgreeMgr->hSecAgreeOwnersPool, pSecAgree->hOwnersList, hListElem, &hListElem);
	while (hListElem != NULL && safeCounter < SEC_AGREE_MAX_LOOP(pSecAgree))
	{
		/* Allocate owner record in the temp list */
		rv = RLIST_InsertAfter(pSecAgree->pSecAgreeMgr->hSecAgreeOwnersPool, hTempList, hTempListElem, &hTempListElem);
		if (RV_OK != rv)
		{
			break;
		}

		/* Copy owner record to the temporary record */
		pOwnerRec = (SecAgreeOwnerRecord*)hListElem;
		pTempOwnerRec = (SecAgreeOwnerRecord*)hTempListElem;
		memcpy(pTempOwnerRec, pOwnerRec, sizeof(SecAgreeOwnerRecord));

		/* Get next owner from the security-agreement list */
		RLIST_GetNext(pSecAgree->pSecAgreeMgr->hSecAgreeOwnersPool, pSecAgree->hOwnersList, hListElem, &hListElem);

		safeCounter += 1;
	}

	return rv;
}

/***************************************************************************
 * InitiateSecObj
 * ------------------------------------------------------------------------
 * General: Creates a new security object, sets its parameters and initiates it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree            - The security-agreement object
 *         hIpsecLocalSecurity  - Local security header with ipsec mechanism
 * Output: phSecObj             - The newly created security object
 *         pbSecObjInitiated    - Indication that a new security object was initiated.
 ***************************************************************************/
static RvStatus RVCALLCONV InitiateSecObj(
									IN  SecAgree*                      pSecAgree,
									IN  RvSipSecurityHeaderHandle      hIpsecLocalSecurity,
									OUT RvSipSecObjHandle*             phSecObj,
									OUT RvBool*                        pbSecObjInitiated)
{
	SecAgreeMgr*                pSecAgreeMgr = pSecAgree->pSecAgreeMgr;
	RvSipSecObjHandle           hSecObj;
	RvStatus                    rv;

	if (pSecAgree->securityData.secMechsToInitiate == 0) 
	{
		return RV_OK;	
	}
	
	/* Create a new security object */
	rv = RvSipSecMgrCreateSecObj(pSecAgreeMgr->hSecMgr, NULL, &hSecObj);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				 	"InitiateSecObj - Security-agreement=0x%p: Failed to create new security object. rv=%d", 
					pSecAgree, rv));
		return rv;
	}

	/* Set the security-agreement handle to the security object */
	rv = SipSecObjSetSecAgreeObject(pSecAgreeMgr->hSecMgr, hSecObj, 
									(void*)pSecAgree, pSecAgree->securityData.secMechsToInitiate);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				 	"InitiateSecObj - Security-agreement=0x%p: Failed to set security-agreement handle to security object 0x%p. rv=%d", 
					pSecAgree, hSecObj, rv));
		/* Terminate the security object */
		RvSipSecObjTerminate(hSecObj);
		return rv;
	}

	/* Set the application context to the security object */
	if (pSecAgree->pContext != NULL)
	{
		rv = SipSecObjSetImpi(pSecAgree->pSecAgreeMgr->hSecMgr, hSecObj, pSecAgree->pContext);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
						"InitiateSecObj - Security-agreement 0x%p: Failed to set context (IMPI) to security-object 0x%p. rv=%d",
						pSecAgree, hSecObj, rv));
			/* Terminating the security object */
			TerminateSecObj(pSecAgree, hSecObj);
			return rv;
		}
	}

	/* On initialization we only set ipsec parameters to the security object, because we need
	   to open ipsec channels upon suggesting them. Tls connection on the other hand needs to be
	   established only after the agreement is obtained, hence on activation */
	if (NULL != hIpsecLocalSecurity)
	{
#if (RV_IMS_IPSEC_ENABLED == RV_YES)
		/* Retrieve ipsec parameters from the local security header and set them to the security object */
		rv = SetLocalIpsecParamsToSecObj(hSecObj, hIpsecLocalSecurity);
		if (RV_OK == rv)
		{
			rv = SetLocalSecAgreeIpsecParamsToSecObj(pSecAgree, hSecObj);
		}		
		if (RV_OK != rv)
		{
			RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				 		"InitiateSecObj - Security-agreement=0x%p: Failed to set ipsec parameters to security object=0x%p. rv=%d", 
						pSecAgree, hSecObj, rv));
			/* Terminating the security object */
			TerminateSecObj(pSecAgree, hSecObj);
			return rv;
		}
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */
	}
	
	/* Initiate the security object  */
	rv = RvSipSecObjInitiate(hSecObj);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				 	"InitiateSecObj - Security-agreement=0x%p: Failed to construct new security object. rv=%d", 
					pSecAgree, rv));
		/* Terminating the security object */
		TerminateSecObj(pSecAgree, hSecObj);
		return rv;
	}

	if (NULL != hIpsecLocalSecurity)
	{
#if (RV_IMS_IPSEC_ENABLED == RV_YES)
		/* Get the actual ipsec parameters used by the security object and set them to the security header */
		rv = GetLocalIpsecParamsFromSecObj(pSecAgree, hSecObj, hIpsecLocalSecurity);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				 		"InitiateSecObj - Security-agreement=0x%p: Failed to get ipsec parameters from security object=0x%p into security header. rv=%d", 
						pSecAgree, hSecObj, rv));
			/* Terminating the security object */
			TerminateSecObj(pSecAgree, hSecObj);
			return rv;
		}
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */
	}

	*phSecObj          = hSecObj;
	*pbSecObjInitiated = RV_TRUE;

	return RV_OK;
}

/***************************************************************************
 * ActivateSecObj
 * ------------------------------------------------------------------------
 * General: Sets additional parameters to the security object and activates it
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree        - The security-agreement object
 *         hSecObj          - The security object
 *         eMechanism       - The mechanism selected
 *         hRemoteSecurity  - Remote security header with ipsec mechanism
 ***************************************************************************/
static RvStatus RVCALLCONV ActivateSecObj(
									IN  SecAgree*                  pSecAgree,
									IN  RvSipSecObjHandle          hSecObj,
									IN  RvSipSecurityHeaderHandle  hRemoteSecurity)
{
	SecAgreeMgr*                pSecAgreeMgr = pSecAgree->pSecAgreeMgr;
	RvBool                      bIsIpsec = RV_FALSE;
	RvStatus                    rv;

#if (RV_IMS_IPSEC_ENABLED == RV_YES)
	if ((NULL != hRemoteSecurity) && (SecAgreeUtilsIsIpsecNegOnly() == RV_FALSE) &&
		(pSecAgree->securityData.selectedSecurity.eType == RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP ||
		 pSecAgree->securityData.selectedSecurity.eType == RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED) &&
		(0 != (pSecAgree->securityData.secMechsToInitiate & RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP)))
	{
		/* Retrieve ipsec parameters from the remote security header and set them to the security object */
		rv = SetRemoteIpsecParamsToSecObj(hSecObj, hRemoteSecurity);
		if (RV_OK == rv)
		{
			/* Set IK and CK to the security object */
			rv = SetRemoteSecAgreeIpsecParamsToSecObj(pSecAgree, hSecObj);
		}
		if (RV_OK != rv)
		{
			RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				 		"ActivateSecObj - Security-agreement=0x%p: Failed to set ipsec parameters to security object=0x%p. rv=%d", 
						pSecAgree, hSecObj, rv));
			return rv;
		}

		bIsIpsec = RV_TRUE;
	}
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */

#if (RV_TLS_TYPE != RV_TLS_NONE)
	if (pSecAgree->securityData.selectedSecurity.eType == RVSIP_SECURITY_MECHANISM_TYPE_TLS &&
		pSecAgree->addresses.remoteTlsAddr.addrtype != 0)
	{
		/* We set Tls parameters only if Tls was chosen */
		rv = SetSecAgreeTlsParamsToSecObj(pSecAgree);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "ActivateSecObj - security-agreement=0x%p: Failed to set TLS parameters to security object=0x%p, rv=%d",
					   pSecAgree, pSecAgree->securityData.hSecObj, rv));
			return rv;
		}
	}
	RV_UNUSED_ARG(hRemoteSecurity);
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */

	/* Activate the security object  */
	rv = RvSipSecObjActivate(hSecObj);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				 	"ActivateSecObj - Security-agreement=0x%p: Failed to activate security object=0x%p. rv=%d", 
					pSecAgree, hSecObj, rv));
		return rv;
	}

	if (bIsIpsec == RV_TRUE)
	{
		SecAgreeStatusConsiderNewStatus(pSecAgree, SEC_AGREE_PROCESSING_IPSEC_ACTIVATION, 
										UNDEFINED /* response code is irrelevant */);
	}

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecAgreeMgr);
#endif

	return RV_OK;
}

/***************************************************************************
 * ShouldTerminateSecObj
 * ------------------------------------------------------------------------
 * General: Determines whether the security object should be terminated
 * Return Value: Boolean to indicate whether to terminate the security object
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree    - The security-agreement object
 *         eProcessing  - Indicates the event being processed (message to send,
 *                        message received, etc)
 ***************************************************************************/
static RvBool RVCALLCONV ShouldTerminateSecObj(
										 IN  SecAgree*            pSecAgree,
										 IN  SecAgreeProcessing   eProcessing)
{
	if (pSecAgree->securityData.hSecObj == NULL)
	{
		return RV_FALSE;
	}

	if (SEC_AGREE_PROCESSING_SET_SECURITY == eProcessing &&
		RV_FALSE == SecAgreeUtilsIsMechanismHandledBySecObj(pSecAgree->securityData.selectedSecurity.eType))
	{
		/* The security mechanism that was chosen does not require a security object */ 
		return RV_TRUE;
	}
	
	if ((SEC_AGREE_PROCESSING_MSG_RCVD == eProcessing || SEC_AGREE_PROCESSING_DEST_RESOLVED == eProcessing) &&
		 RVSIP_SEC_AGREE_STATE_NO_AGREEMENT_REQUIRED == pSecAgree->eState)
	{
		/* The security-agreement was aborted due to TLS resolution. The security object
		   will not be used. */
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * TerminateSecObj
 * ------------------------------------------------------------------------
 * General: Terminate the security-object of this security-agreement
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecAgree     - Pointer to the security-agreement object
 *          hSecObj       - Handle to the security-object
 ***************************************************************************/
static void RVCALLCONV TerminateSecObj(
									  IN   SecAgree*            pSecAgree,
									  IN   RvSipSecObjHandle    hSecObj)
{
	RvStatus   rv;

	RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				"TerminateSecObj - security-agreement=0x%p: detaching from security object 0x%p",
				pSecAgree, hSecObj));

	/* Detach from the security-object in order not to receive its events from now on */
	rv = SipSecObjSetSecAgreeObject(pSecAgree->pSecAgreeMgr->hSecMgr, hSecObj, NULL, 0);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				   "TerminateSecObj - security-agreement=0x%p: Failed to detach from security object 0x%p, rv=%d",
				   pSecAgree, hSecObj, rv));
	}

	/* Reset the security-object pointer in the security-agreement */
	pSecAgree->securityData.hSecObj = NULL;	
	
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
	/* Reset the local address details that were received from the security object */    
	pSecAgree->addresses.hLocalIpsecTcpAddr = NULL;
	pSecAgree->addresses.hLocalIpsecUdpAddr = NULL;
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
}

/***************************************************************************
 * CanInitiateSecObj
 * ------------------------------------------------------------------------
 * General: Determines whether the security object can be initiated
 * Return Value: Boolean to indicate whether to initiate the security object
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree    - The security-agreement object
 ***************************************************************************/
static RvBool RVCALLCONV CanInitiateSecObj(
										 IN  SecAgree*            pSecAgree)
{
	if (pSecAgree->securityData.hSecObj != NULL)
	{
		/* The security object was already initiated */
		return RV_FALSE;
	}
	
	if (pSecAgree->securityData.selectedSecurity.eType == RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED ||
		SecAgreeUtilsIsMechanismHandledBySecObj(pSecAgree->securityData.selectedSecurity.eType) == RV_TRUE) 
	{
		return RV_TRUE;
	}

	/* The security mechanism that was chosen does not require a security object */ 
		
	return RV_FALSE;
}

/***************************************************************************
 * CanActivateSecObj
 * ------------------------------------------------------------------------
 * General: Determines whether the security object can be activated
 * Return Value: Boolean to indicate whether to activate the security object
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree    - The security-agreement object
 ***************************************************************************/
static RvBool RVCALLCONV CanActivateSecObj(
										 IN  SecAgree*            pSecAgree)
{
	if (pSecAgree->securityData.selectedSecurity.eType == RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED ||
		SecAgreeUtilsIsMechanismHandledBySecObj(pSecAgree->securityData.selectedSecurity.eType) == RV_TRUE) 
	{
		return RV_TRUE;
	}

	/* The security mechanism that was chosen does not require a security object */ 
		
	return RV_FALSE;
}

/***************************************************************************
 * ShouldSetMechanismToSecObj
 * ------------------------------------------------------------------------
 * General: Determines whether the security object should set the mechanism
 *          to the security object
 * Return Value: Boolean to indicate whether to set security object mechanism
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree    - The security-agreement object
 *         eProcessing  - Indicates the event being processed (message to send,
 *                        message received, etc)
 ***************************************************************************/
static RvBool RVCALLCONV ShouldSetMechanismToSecObj(
										 IN  SecAgree*            pSecAgree,
										 IN  SecAgreeProcessing   eProcessing)
{
	if (pSecAgree->securityData.hSecObj == NULL)
	{
		return RV_FALSE;
	}

	if (eProcessing == SEC_AGREE_PROCESSING_SET_SECURITY &&
		(SecAgreeUtilsIsMechanismHandledBySecObj(pSecAgree->securityData.selectedSecurity.eType) == RV_TRUE))
	{
		/* The chosen security if ipsec-3gpp or TLS */ 
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * UpdateSecObjMechanism
 * ------------------------------------------------------------------------
 * General: Sets the given mechanism to the security object
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecAgree   - Pointer to the security-agreement object
 ***************************************************************************/
static RvStatus RVCALLCONV UpdateSecObjMechanism(
									  IN  SecAgree*                   pSecAgree)
{
	RvStatus   rv;

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
	if (SecAgreeUtilsIsMechanismHandledBySecObj(pSecAgree->securityData.selectedSecurity.eType) == RV_FALSE)
	{
		return RV_OK;
	}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
	if (pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_SERVER)
	{
		/* Set the selected mechanism to the security object */
		rv = SipSecObjSetMechanism(pSecAgree->pSecAgreeMgr->hSecMgr, 
		  						   pSecAgree->securityData.hSecObj, 
								   pSecAgree->securityData.selectedSecurity.eType);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "UpdateSecObjMechanism - security-agreement=0x%p: Failed to set mechanism to security object=0x%p, rv=%d",
					   pSecAgree, pSecAgree->securityData.hSecObj, rv));
			return rv;
		}
	}

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
	if (pSecAgree->securityData.selectedSecurity.eType != RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP)
	{
		/* Reset the local address details that were received from the security object */
		pSecAgree->addresses.hLocalIpsecTcpAddr = NULL;
		pSecAgree->addresses.hLocalIpsecUdpAddr = NULL;
	}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
	
	/* supply the security-object to the SIP owners */
	rv = SecAgreeObjectSupplySecObjToOwners(pSecAgree);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"UpdateSecObjMechanism - Security-agreement=0x%p: Failed to notify owners to attach to security-object=0x%p, rv=%d", 
					pSecAgree, pSecAgree->securityData.hSecObj, rv));
	}

	return rv;
}

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * SetSendOptionsToSecObj
 * ------------------------------------------------------------------------
 * General: Sets the send options stored in the security-agreement into the 
 *          security-object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecAgree - The security-agreement object
 *          hSecObj   - Handle to the security-object
 ***************************************************************************/
static RvStatus RVCALLCONV SetSendOptionsToSecObj(
									IN    SecAgree           *pSecAgree,
									IN    RvSipSecObjHandle   hSecObj)
{
	RvStatus   rv;

	if (hSecObj == NULL)
	{
		return RV_OK;
	}

	if (pSecAgree->addresses.strSigcompId != UNDEFINED)
	{
		RPOOL_Ptr  strSigcompId;
		strSigcompId.hPool  = pSecAgree->pSecAgreeMgr->hMemoryPool;
		strSigcompId.hPage  = pSecAgree->hPage;
		strSigcompId.offset = pSecAgree->addresses.strSigcompId;
		rv = SipSecObjSetSigcompParams(hSecObj, pSecAgree->addresses.eMsgCompType, &strSigcompId, NULL);	
	}
	else
	{
		rv = SipSecObjSetSigcompParams(hSecObj, pSecAgree->addresses.eMsgCompType, NULL, NULL);	
	}
	
	return rv;
}
#endif /* #ifdef RV_SIGCOMP_ON */

#endif /* (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */

#if (RV_IMS_IPSEC_ENABLED == RV_YES)
/***************************************************************************
 * AllocAndCopyAddress
 * ------------------------------------------------------------------------
 * General: Allocates and copies the given string address.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPool         - The memory pool to copy to.
 *            hPage         - The memory page to copy to.
 *            strAddrToCopy - The string address to copy
 * Output:    ppAddr        - The new address pointer.
 ***************************************************************************/
static RvStatus RVCALLCONV AllocAndCopyAddress(IN  HRPOOL      hPool,
											   IN  HPAGE       hPage,
											   IN  RvChar*     strAddrToCopy,
                                               OUT RvChar**    ppAddr)
{
	RvStatus           rv;
	RvInt32            offset;
	RPOOL_ITEM_HANDLE  hItem;

	rv = RPOOL_AppendFromExternal(hPool, hPage, 
								  strAddrToCopy, strlen(strAddrToCopy)+1, RV_TRUE, 
								  &offset, &hItem);

	if (RV_OK == rv)
	{
		*ppAddr = RPOOL_GetPtr(hPool, hPage, offset);
	}

	return rv;
}

/***************************************************************************
 * SetLocalIpsecParamsToSecObj
 * ------------------------------------------------------------------------
 * General: Sets ipsec parameters from the given local security header to 
 *          the security object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecObj           - The security object
 *         hLocalSecurity    - The local security header
 ***************************************************************************/
static RvStatus RVCALLCONV SetLocalIpsecParamsToSecObj(
									     IN  RvSipSecObjHandle          hSecObj,
									     IN  RvSipSecurityHeaderHandle  hLocalSecurity)
{
	RvSipSecIpsecPeerParams     localPeerParams;
	RvSipSecIpsecPeerParams     remotePeerParams;
	RvSipSecIpsecParams         ipsecParams;
	RvStatus                    rv;

	/* init parameters structures */
	RvSipSecUtilInitIpsecParams(&ipsecParams, &localPeerParams, &remotePeerParams);

	/* Retrieve parameters from the local security header */
	rv = GetIpsecPeerParamsFromSecurityHeader(hLocalSecurity, &localPeerParams);
	if (RV_OK != rv)
	{
		return rv;
	}

	/* Retrieve general parameters from the local security header */
	rv = GetIpsecParamsFromSecurityHeader(hLocalSecurity, &ipsecParams);
	if (RV_OK != rv)
	{
		return rv;
	}

	/* Set local parameters to the security object. Notice that other parameters are set to NULL, i.e. remain the same */
	rv = RvSipSecObjSetIpsecParams(hSecObj, &ipsecParams, &localPeerParams, NULL);
	if (RV_OK != rv)
	{
		return rv;
	}

	return RV_OK;
}

/***************************************************************************
 * SetLocalSecAgreeIpsecParamsToSecObj
 * ------------------------------------------------------------------------
 * General: Sets additional local information from the security-agreement to
 *          the security object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree      - The security-agreement object
 *         hSecObj        - The security object
 ***************************************************************************/
static RvStatus RVCALLCONV SetLocalSecAgreeIpsecParamsToSecObj(
										 IN  SecAgree*                      pSecAgree,
										 IN  RvSipSecObjHandle              hSecObj)
{
	RvSipSecLinkCfg  linkCfg;
	RvStatus         rv;
#if (RV_NET_TYPE & RV_NET_IPV6)
    RvChar           strLocalIpv6Addr[RVSIP_TRANSPORT_LEN_STRING_IP];
#endif /* #if (RV_NET_TYPE & RV_NET_IPV6) */
    RvSipTransportAddr  addrDetails;
	
#ifdef RV_SIGCOMP_ON
	/* reset sigcomp information */
	linkCfg.eMsgCompType = RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED;
	linkCfg.strSigcompId = NULL;
#endif /* #ifdef RV_SIGCOMP_ON */

  //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
  SipSecObjIpsecSetLocalTransport(hSecObj,pSecAgree->addresses.eTransportType);
#endif
  //SPIRENT_END
	
	/* Set current security agreement link information: local address */
	
	linkCfg.strRemoteIp = NULL;
	
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
	if (pSecAgree->addresses.hLocalIpsecUdpAddr != NULL ||
#else
  if (pSecAgree->addresses.hLocalIpsecUdpAddr != NULL &&
#endif
//SPIRENT_END
		pSecAgree->addresses.hLocalIpsecTcpAddr != NULL)
	{
		/* The local address for ipsec use was already open (it was peobably copied while calling 
		   RvSipSecAgreeCopy() for renegotiation. We instruct the security object to use
		   the open local address instead of opening a new one. */
		rv = SipSecObjIpsecSetPortSLocalAddresses(pSecAgree->pSecAgreeMgr->hSecMgr, hSecObj, 
												  pSecAgree->addresses.hLocalIpsecUdpAddr,
												  pSecAgree->addresses.hLocalIpsecTcpAddr);
		if (RV_OK != rv)
		{
			return rv;
		}

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(pSecAgree->addresses.hLocalIpsecUdpAddr) {
#endif
//SPIRENT_END
		/* We still need to set local IP for port-c initiation, so we retrieve it from the local
		   address object and set it to the security object (we arbitrarily use the UDP local address) */
        rv = RvSipTransportMgrLocalAddressGetDetails(
                pSecAgree->addresses.hLocalIpsecUdpAddr,
                sizeof(RvSipTransportAddr), &addrDetails);
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    } else {
      rv = RvSipTransportMgrLocalAddressGetDetails(
                pSecAgree->addresses.hLocalIpsecTcpAddr,
                sizeof(RvSipTransportAddr), &addrDetails);
    }
#endif
//SPIRENT_END

		if (RV_OK != rv)
		{
			return rv;
		}

		linkCfg.strLocalIp  = addrDetails.strIP;
	}
	else
	{
		/* We set the local IP that was supplied by the application. */
        linkCfg.strLocalIp  = pSecAgree->addresses.pLocalIpsecIp;

        /* In case of IPv6 address add Scope ID. */
#if (RV_NET_TYPE & RV_NET_IPV6)
        if (RVSIP_TRANSPORT_ADDRESS_TYPE_IP6 == 
            pSecAgree->addresses.localIpsecAddrType)
        {
            linkCfg.strLocalIp  = strLocalIpv6Addr;
            if (NULL != strchr(pSecAgree->addresses.pLocalIpsecIp, '%'))
            {
                strcpy(linkCfg.strLocalIp, pSecAgree->addresses.pLocalIpsecIp);
            }
            else
            {
                sprintf(linkCfg.strLocalIp, "%s%%%d",
                    pSecAgree->addresses.pLocalIpsecIp,
                    pSecAgree->addresses.localIpsecIpv6Scope);
            }
        }
#endif /*#if (RV_NET_TYPE & RV_NET_IPV6)*/
	}

	return RvSipSecObjSetLinkCfg(hSecObj, &linkCfg);
}

/***************************************************************************
 * SetRemoteIpsecParamsToSecObj
 * ------------------------------------------------------------------------
 * General: Sets ipsec parameters from the given remote security header to 
 *          the security object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecObj           - The security object
 *         hRemoteSecurity   - The remote security header
 ***************************************************************************/
static RvStatus RVCALLCONV SetRemoteIpsecParamsToSecObj(
									     IN  RvSipSecObjHandle          hSecObj,
									     IN  RvSipSecurityHeaderHandle  hRemoteSecurity)
{
	RvSipSecIpsecPeerParams     localPeerParams;
	RvSipSecIpsecPeerParams     remotePeerParams;
	RvSipSecIpsecParams         ipsecParams;
	RvStatus                    rv;

	/* init parameters structures */
	RvSipSecUtilInitIpsecParams(&ipsecParams, &localPeerParams, &remotePeerParams);

	/* Retrieve parameters from the local security header */
	rv = GetIpsecPeerParamsFromSecurityHeader(hRemoteSecurity, &remotePeerParams);
	if (RV_OK != rv)
	{
		return rv;
	}

	/* Set remote parameters to the security object. Notice that other parameters are set to NULL, i.e. remain the same */
	rv = RvSipSecObjSetIpsecParams(hSecObj, NULL, NULL, &remotePeerParams);
	if (RV_OK != rv)
	{
		return rv;
	}

	return RV_OK;
}

/***************************************************************************
 * SetRemoteSecAgreeIpsecParamsToSecObj
 * ------------------------------------------------------------------------
 * General: Sets additional ipsec parameters held by the security-agreement
 *          (IK and CK, remote address) to the security object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree      - The security-agreement object
 *         hSecObj        - The security object
 ***************************************************************************/
static RvStatus RVCALLCONV SetRemoteSecAgreeIpsecParamsToSecObj(
										 IN  SecAgree*               pSecAgree,
									     IN  RvSipSecObjHandle       hSecObj)
{
	RvSipSecAgreeIpsecInfo*     pIpsecInfo = &pSecAgree->securityData.additionalKeys.ipsecInfo;
	RvSipSecIpsecPeerParams     localPeerParams;
	RvSipSecIpsecPeerParams     remotePeerParams;
	RvSipSecIpsecParams         ipsecParams;
	RvSipSecLinkCfg             linkCfg;
	RvChar                      pRemoteIp[RVSIP_TRANSPORT_LEN_STRING_IP];
	RvChar*                     pRes;
	RvStatus                    rv;

	if (pSecAgree->addresses.remoteIpsecAddr.addrtype == 0 ||
		pSecAgree->addresses.eRemoteIpsecTransportType == RVSIP_TRANSPORT_UNDEFINED)
	{
		/* Remote address was not set yet */
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				 	"SetRemoteSecAgreeIpsecParamsToSecObj - Security-agreement=0x%p: TCP/UDP remote address was not set, hence cannot initialize security object=0x%p.", 
					pSecAgree, hSecObj));
		return RV_ERROR_UNKNOWN;
	}

	/* init parameters structures */
	RvSipSecUtilInitIpsecParams(&ipsecParams, &localPeerParams, &remotePeerParams);

	/* set security-agreement ipsec information */
	SecAgreeUtilsCopyKey(ipsecParams.IK, ipsecParams.CK,
					     pIpsecInfo->IK, pIpsecInfo->CK);
	ipsecParams.IKlen    = pIpsecInfo->IKlen;
	ipsecParams.CKlen    = pIpsecInfo->CKlen;
	ipsecParams.lifetime = pIpsecInfo->lifetime;

	/* set forced transport */
	ipsecParams.eTransportType = pSecAgree->addresses.eRemoteIpsecTransportType;

#ifdef RV_SIGCOMP_ON
	/* reset sigcomp information */
	linkCfg.eMsgCompType = RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED;
	linkCfg.strSigcompId = NULL;
#endif /* #ifdef RV_SIGCOMP_ON */
	
	/* Set local parameters to the security object. Notice that other parameters are set to NULL, i.e. remain the same */
	rv = RvSipSecObjSetIpsecParams(hSecObj, &ipsecParams, NULL, NULL);
	if (RV_OK != rv)
	{
		return rv;
	}

	/* Set current security agreement link information: remote IP */
	pRes = RvAddressGetString(&pSecAgree->addresses.remoteIpsecAddr, RVSIP_TRANSPORT_LEN_STRING_IP, pRemoteIp);
	if (pRes == NULL)
	{
		return RV_ERROR_UNKNOWN;
	}

	linkCfg.strLocalIp   = NULL;
	linkCfg.strRemoteIp  = (RvChar*)pRemoteIp;
#ifdef RV_SIGCOMP_ON
	rv = SetSendOptionsToSecObj(pSecAgree, hSecObj);
	if (RV_OK != rv)
	{
		return rv;
	}
#endif /* #ifdef RV_SIGCOMP_ON */ 

	rv = RvSipSecObjSetLinkCfg(hSecObj, &linkCfg);
	if (RV_OK != rv)
	{
		return rv;
	}

	return RV_OK;
}

/***************************************************************************
 * GetIpsecPeerParamsFromSecurityHeader
 * ------------------------------------------------------------------------
 * General: Retrieve ipsec peer parameters from a security header: spis and ports
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hIpsecSecurity   - Handle to the security header
 * Output:  pIpsecPeerParams - The structure of ipsec peer parameters 
 ***************************************************************************/
static RvStatus RVCALLCONV GetIpsecPeerParamsFromSecurityHeader(
									IN    RvSipSecurityHeaderHandle   hIpsecSecurity,
									OUT   RvSipSecIpsecPeerParams*    pIpsecPeerParams)
{
	RvBool    bIsSet;
	RvInt32   portC;
	RvInt32   portS;
	RvUint32  spiC;
	RvUint32  spiS;
	RvStatus  rv;

	/* ports */
	portC = RvSipSecurityHeaderGetPortC(hIpsecSecurity);
	portS = RvSipSecurityHeaderGetPortS(hIpsecSecurity);

	if (portC > UNDEFINED)
	{
		pIpsecPeerParams->portC = (RvUint16)portC;
	}

	if(portS > UNDEFINED)
	{
		pIpsecPeerParams->portS = (RvUint16)portS;
	}

	/* spis */
	rv = RvSipSecurityHeaderGetSpiC(hIpsecSecurity, &bIsSet, &spiC);
	if (RV_OK != rv)
	{
		return rv;
	}

	if (bIsSet == RV_TRUE)
	{
		pIpsecPeerParams->spiC = spiC;
	}

	rv = RvSipSecurityHeaderGetSpiS(hIpsecSecurity, &bIsSet, &spiS);
	if (RV_OK != rv)
	{
		return rv;
	}

	if (bIsSet == RV_TRUE)
	{
		pIpsecPeerParams->spiS = spiS;
	}

	return RV_OK;
}

/***************************************************************************
 * GetIpsecParamsFromSecurityHeader
 * ------------------------------------------------------------------------
 * General: Retrieve ipsec parameters from a security header: algorithms.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hIpsecSecurity   - Handle to the security header
 * Output:  pIpsecParams     - The structure of ipsec parameters
 ***************************************************************************/
static RvStatus RVCALLCONV GetIpsecParamsFromSecurityHeader(
									IN    RvSipSecurityHeaderHandle   hIpsecSecurity,
									OUT   RvSipSecIpsecParams*        pIpsecParams)
{
	/* algorithms */
	pIpsecParams->eIAlg = SecAgreeUtilsSecurityHeaderGetAlgo(hIpsecSecurity);
	pIpsecParams->eEAlg = SecAgreeUtilsSecurityHeaderGetEAlgo(hIpsecSecurity);

	return RV_OK;
}

/***************************************************************************
 * GetLocalIpsecParamsFromSecObj
 * ------------------------------------------------------------------------
 * General: Gets the actual local ipsec parameters used by the security object 
 *          and updates them in the security header and in the security-agreement
 *          object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree         - The security-agreement object
 *         hSecObj           - The security object
 *         hLocalSecurity    - The local security header
 ***************************************************************************/
static RvStatus RVCALLCONV GetLocalIpsecParamsFromSecObj(
										 IN  SecAgree*                  pSecAgree,
									     IN  RvSipSecObjHandle          hSecObj,
									     IN  RvSipSecurityHeaderHandle  hLocalSecurity)
{
	RvSipSecIpsecPeerParams     localPeerParams;
	RvSipSecIpsecParams         ipsecParams;
	RvStatus                    rv;

	/* Get local address from the security object into the security agreement */
	rv = SipSecObjIpsecGetPortSLocalAddresses(pSecAgree->pSecAgreeMgr->hSecMgr, hSecObj, 
											  &pSecAgree->addresses.hLocalIpsecUdpAddr,
											  &pSecAgree->addresses.hLocalIpsecTcpAddr);
	if (RV_OK != rv)
	{
		return rv;
	}
	
	/* get ipsec parameters from the security object */
	rv = RvSipSecObjGetIpsecParams(hSecObj, &ipsecParams, &localPeerParams, NULL);
	if (RV_OK != rv)
	{
		return rv;
	}

	/* set local parameters into the security header */
	rv = RvSipSecurityHeaderSetPortC(hLocalSecurity, localPeerParams.portC);
	if (RV_OK != rv)
	{
		return rv;
	}

	rv = RvSipSecurityHeaderSetPortS(hLocalSecurity, localPeerParams.portS);
	if (RV_OK != rv)
	{
		return rv;
	}
	
	rv = RvSipSecurityHeaderSetSpiC(hLocalSecurity, RV_TRUE, localPeerParams.spiC);
	if (RV_OK != rv)
	{
		return rv;
	}
	
	rv = RvSipSecurityHeaderSetSpiS(hLocalSecurity, RV_TRUE, localPeerParams.spiS);
	if (RV_OK != rv)
	{
		return rv;
	}

	return RV_OK;
}
#endif /* (RV_IMS_IPSEC_ENABLED == RV_YES) */

#if (RV_TLS_TYPE != RV_TLS_NONE)
/***************************************************************************
 * SetSecAgreeTlsParamsToSecObj
 * ------------------------------------------------------------------------
 * General: Sets TLS information from the security-agreement to
 *          the security object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree      - The security-agreement object
 *         hSecObj        - The security object
 ***************************************************************************/
static RvStatus RVCALLCONV SetSecAgreeTlsParamsToSecObj(
										 IN  SecAgree*            pSecAgree)
{
	SipSecTlsParams               tlsParams;
	RvSipTransportAddressType     eAddrType;
	RvStatus                      rv;

	if (pSecAgree->addresses.remoteTlsAddr.addrtype == 0)
	{
		/* Remote address was not set yet */
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				 	"SetSecAgreeTlsParamsToSecObj - Security-agreement=0x%p: TLS remote address was not set", 
					pSecAgree));
		return RV_ERROR_UNKNOWN;
	}

	/* Choose the local address that matches the remote address type (Ip4 or Ip6) */
	eAddrType = SipTransportConvertCoreAddrTypeToSipAddrType(RvAddressGetType(&pSecAgree->addresses.remoteTlsAddr));
	if (eAddrType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
	{
		tlsParams.hLocalAddr = pSecAgree->addresses.hLocalTlsIp4Addr;
	}
	else 
	{
		tlsParams.hLocalAddr = pSecAgree->addresses.hLocalTlsIp6Addr;
	}

	/* Get the remote address */
	tlsParams.pRemoteAddr = &pSecAgree->addresses.remoteTlsAddr;
#ifdef RV_SIGCOMP_ON
	/* Get the send options */
	tlsParams.eMsgCompType = pSecAgree->addresses.eMsgCompType;
	tlsParams.strSigcompId.hPool  = pSecAgree->pSecAgreeMgr->hMemoryPool;
	tlsParams.strSigcompId.hPage  = pSecAgree->hPage;
	tlsParams.strSigcompId.offset = pSecAgree->addresses.strSigcompId;
#endif /* #ifdef RV_SIGCOMP_ON */

	/* Set local and remote address to the security object */
	rv = SipSecObjSetTlsParams(pSecAgree->pSecAgreeMgr->hSecMgr, pSecAgree->securityData.hSecObj, &tlsParams);

	return rv;
}

#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/

/***************************************************************************
 * ResetDataUponFailureToCopy
 * ------------------------------------------------------------------------
 * General: Sets TLS information from the security-agreement to
 *          the security object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pDestSecAgree - The security-agreement object that failed to copy
 *         bResetRemote  - Indicates whether to reset the remote data
 ***************************************************************************/
static void RVCALLCONV ResetDataUponFailureToCopy(
										 IN  SecAgree*         pDestSecAgree,
										 IN  RvBool            bResetRemote)
{
	/* Reset local security list */
	SecAgreeSecurityDataResetData(&pDestSecAgree->securityData, pDestSecAgree->eRole, SEC_AGREE_SEC_DATA_LIST_LOCAL);
	
	/* Reset local security list */
	SecAgreeSecurityDataResetData(&pDestSecAgree->securityData, pDestSecAgree->eRole, SEC_AGREE_SEC_DATA_LIST_TISPAN_LOCAL);
	
	/* Reset addresses */
	InitAddresses(pDestSecAgree);
	SecAgreeMgrGetDefaultLocalAddr(pDestSecAgree->pSecAgreeMgr, pDestSecAgree); 

	if (bResetRemote == RV_TRUE)
	{
		SecAgreeSecurityDataResetData(&pDestSecAgree->securityData, pDestSecAgree->eRole, SEC_AGREE_SEC_DATA_LIST_REMOTE);
	}
}


#endif /* #ifdef RV_SIP_IMS_ON */

