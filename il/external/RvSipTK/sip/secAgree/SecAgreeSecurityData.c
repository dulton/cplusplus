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
 *                              <SecAgreeSecurityData.c>
 *
 * This file defines the security-agreement-data object, attributes and interface functions.
 * The security-agreement-data object is used to encapsulate all security-agreement data,
 * negotiated as defined in RFC 3329.
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

#include "RvSipSecAgreeTypes.h"
#include "SecAgreeSecurityData.h"
#include "SecAgreeUtils.h"
#include "_SipSecurityHeader.h"
#include "_SipAuthenticationHeader.h"
#include "RvSipSecurity.h"
#include "_SipSecuritySecObj.h"
#include "_SipTransport.h"
#include "RvSipSecurityHeader.h"

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static void RVCALLCONV SecAgreeSecurityHeaderRunOver(
									 IN    RvSipSecurityHeaderHandle   hSecurityHeader,
									 INOUT RvUint32*                   pSpiC,
									 INOUT RvBool*                     pbIsInitSpiC,
									 INOUT RvUint32*                   pSpiS,
									 INOUT RvBool*                     pbIsInitSpiS,
									 INOUT RvInt32*                    pPortC,
                                     INOUT RvInt32*                    pPortS);

static RvStatus RVCALLCONV SecAgreeSecurityDataCopySecurityList(
									IN    SecAgreeMgr*					pSecAgreeMgr,
                                    IN    SecAgree*						pSourceSecAgree,
									IN    SecAgreeSecurityData*			pSourceSecAgreeSecurityData,
                                    IN    SecAgreeSecurityData*			pDestSecAgreeSecurityData,
									IN    SecAgreeSecurityDataListType  eListType,
									IN    HPAGE							hPage,
									OUT   RvBool*						pbResetRemote);

/*-----------------------------------------------------------------------*/
/*                         SECURITY CLIENT FUNCTIONS                     */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeSecurityDataInitialize
 * ------------------------------------------------------------------------
 * General: Initialize a new Security-Agreement-Data object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr  - Pointer to the Security-Mgr
 *         pSecAgreeSecurityData - Pointer to the new Security-Agreement-Data
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeSecurityDataInitialize(
									IN    SecAgreeMgr*    pSecAgreeMgr,
									INOUT SecAgreeSecurityData*   pSecAgreeSecurityData)
{
	memset(pSecAgreeSecurityData, 0, sizeof(SecAgreeSecurityData));
	pSecAgreeSecurityData->eInsertSecClient        = SEC_AGREE_SET_CLIENT_LIST_UNDEFINED;
	pSecAgreeSecurityData->selectedSecurity.eType  = RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED;
	
	RV_UNUSED_ARG(pSecAgreeMgr);
	return RV_OK;
}

/***************************************************************************
 * SecAgreeSecurityDataDestruct
 * ------------------------------------------------------------------------
 * General: Destructs a Security-Agreement-Data object.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgreeMgr  - Pointer to the security-agreement manager
 *            pSecAgreeSecurityData - Pointer to the Security-Agreement-Data to terminate.
 *            eRole         - Indication whether this is client or server security
 *                            agreement
 ***************************************************************************/
void RVCALLCONV SecAgreeSecurityDataDestruct(
                                 IN SecAgreeMgr*       pSecAgreeMgr,
								 IN SecAgreeSecurityData*      pSecAgreeSecurityData,
								 IN RvSipSecAgreeRole  eRole)
{
	RvStatus   rv;

	SecAgreeSecurityDataDestructAuthData(pSecAgreeMgr, pSecAgreeSecurityData, eRole);

	rv = SecAgreeSecurityDataDestructSecurityList(pSecAgreeSecurityData, SEC_AGREE_SEC_DATA_LIST_LOCAL);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				  "SecAgreeSecurityDataDestruct - Failed to destruct local security list. rv=%d", rv));
	}

	rv = SecAgreeSecurityDataDestructSecurityList(pSecAgreeSecurityData, SEC_AGREE_SEC_DATA_LIST_REMOTE);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				  "SecAgreeSecurityDataDestruct - Failed to destruct remote security list. rv=%d", rv));
	}
}

/***************************************************************************
 * SecAgreeSecurityDataConstructSecurityList
 * ------------------------------------------------------------------------
 * General: Constructs a security list of this Security-Agreement-Data object
 *          according to the list type given in eListType
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr  - Pointer to the Security-Mgr
 *         hPage         - Handle to the memory page to construct the list on
 *         pSecAgreeSecurityData - Pointer to the Security-Agreement-Data
 *         eListType     - The type of the list to construct: local, remote
 *                         or local for TISPAN support
 * Output: phList        - Pointer to the newly constructed list within the
 *                         security-data structure
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeSecurityDataConstructSecurityList(
							 IN    SecAgreeMgr*					 pSecAgreeMgr,
							 IN    HPAGE						 hPage,
                             IN    SecAgreeSecurityData*		 pSecAgreeSecurityData,
							 IN    SecAgreeSecurityDataListType  eListType,
							 OUT   RvSipCommonListHandle        *phList)
{
	RvSipCommonListHandle  hList;
	RvStatus               rv;

	/* Construct local security list */
	rv = RvSipCommonListConstructOnPage(pSecAgreeMgr->hMemoryPool,
										hPage,
										(RV_LOG_Handle)pSecAgreeMgr->pLogMgr,
										&hList);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				  "SecAgreeSecurityDataConstructLocalList - Failed to allocate local security list. rv=%d", rv));
        return rv;
	}

	switch (eListType) 
	{
	case SEC_AGREE_SEC_DATA_LIST_LOCAL:
		pSecAgreeSecurityData->hLocalSecurityList  = hList;
		break;
	case SEC_AGREE_SEC_DATA_LIST_REMOTE:
		pSecAgreeSecurityData->hRemoteSecurityList = hList;
		break;
	case SEC_AGREE_SEC_DATA_LIST_TISPAN_LOCAL:
		pSecAgreeSecurityData->hTISPANLocalList    = hList;
		break;
	default:
		return RV_ERROR_UNKNOWN;
	}

	if (phList != NULL)
	{
		*phList = hList;
	}

	return RV_OK;
}

/***************************************************************************
 * SecAgreeSecurityDataDestructSecurityList
 * ------------------------------------------------------------------------
 * General: Destructs a security list of this Security-Agreement-Data object
 *          according to the list type given in eListType
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeSecurityData - Pointer to the Security-Agreement-Data
 *         eListType			 - The type of the list to construct: local, remote
 *								   or local for TISPAN support
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeSecurityDataDestructSecurityList(
										IN SecAgreeSecurityData*		pSecAgreeSecurityData,
										IN SecAgreeSecurityDataListType eListType)
{
	RvSipCommonListHandle  *phList;
	RvStatus                rv;
	
	switch (eListType) 
	{
	case SEC_AGREE_SEC_DATA_LIST_LOCAL:
		phList = &pSecAgreeSecurityData->hLocalSecurityList;
		break;
	case SEC_AGREE_SEC_DATA_LIST_REMOTE:
		phList = &pSecAgreeSecurityData->hRemoteSecurityList;
		break;
	case SEC_AGREE_SEC_DATA_LIST_TISPAN_LOCAL:
		phList = &pSecAgreeSecurityData->hTISPANLocalList;
		break;
	default:
		return RV_ERROR_UNKNOWN;
	}

	if (*phList == NULL)
	{
		return RV_OK;
	}

	rv = RvSipCommonListDestruct(*phList);
	*phList = NULL;

	return rv;
}

/***************************************************************************
 * SecAgreeSecurityDataDestructAuthData
 * ------------------------------------------------------------------------
 * General: Destructs the local authentication data
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr  - Pointer to the security-agreement manager
 *         pSecAgreeSecurityData - Pointer to the Security-Agreement-Data
 *         eRole         - The role of the security-agreement
 ***************************************************************************/
void RVCALLCONV SecAgreeSecurityDataDestructAuthData(
										IN SecAgreeMgr*      pSecAgreeMgr,
									    IN SecAgreeSecurityData*     pSecAgreeSecurityData,
										IN RvSipSecAgreeRole eRole)
{
	if (eRole == RVSIP_SEC_AGREE_ROLE_CLIENT &&
		pSecAgreeSecurityData->additionalKeys.authInfo.hClientAuth.hAuthList != NULL)
    {
        SipAuthenticatorAuthListDestruct(pSecAgreeMgr->hAuthenticator,
										 pSecAgreeSecurityData->additionalKeys.authInfo.hClientAuth.pAuthListInfo,
										 pSecAgreeSecurityData->additionalKeys.authInfo.hClientAuth.hAuthList);
        pSecAgreeSecurityData->additionalKeys.authInfo.hClientAuth.hAuthList     = NULL;
		pSecAgreeSecurityData->additionalKeys.authInfo.hClientAuth.pAuthListInfo = NULL;
    }
}

/***************************************************************************
 * SecAgreeSecurityDataPushSecurityHeader
 * ------------------------------------------------------------------------
 * General: Push Security header to the list of security headers
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr        - Pointer to the Security-Mgr
 *         hPage               - The memory page to construct the header on (if needed)
 *         pSecAgreeSecurityData       - Pointer to the Security-Agreement-Data
 *         hSecurityHeader     - The security header to push
 *         eLocation		   - Inserted element location (first, last, etc).
 *         hPos				   - Current list position, relevant in case that location is next or prev.
 *         eListType           - Indicates the type of list to push the security mechanism
 *                               to: local, remote or local that is specific for TISPAN comparison
 * Inout:  pNewPos             - The location of the object that was pushed.
 * Output: phNewSecurityHeader - The new security header that was actually pushed to the
 *                               list (if relevant).
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeSecurityDataPushSecurityHeader(
									IN     SecAgreeMgr*					pSecAgreeMgr,
									IN     HPAGE						hPage,
									IN     SecAgreeSecurityData*		pSecAgreeSecurityData,
									IN     RvSipSecurityHeaderHandle	hSecurityHeader,
									IN     RvSipListLocation			eLocation,
									IN     RvSipCommonListElemHandle	hPos,
									IN     SecAgreeSecurityDataListType	eListType,
									INOUT  RvSipCommonListElemHandle*	phNewPos,
									OUT    RvSipSecurityHeaderHandle*	phNewSecurityHeader)
{
	RvSipCommonListHandle      *phList;
	RvSipCommonListElemHandle   hListElem;
	RvSipSecurityHeaderHandle   hSecHeaderToPush;
	RvStatus                    rv;

	switch (eListType) 
	{
	case SEC_AGREE_SEC_DATA_LIST_LOCAL:
		phList = &pSecAgreeSecurityData->hLocalSecurityList;
		break;
	case SEC_AGREE_SEC_DATA_LIST_REMOTE:
		phList = &pSecAgreeSecurityData->hRemoteSecurityList;
		break;
	case SEC_AGREE_SEC_DATA_LIST_TISPAN_LOCAL:
		phList = &pSecAgreeSecurityData->hTISPANLocalList;
		break;
	default:
		return RV_ERROR_UNKNOWN;
	}

	if (*phList == NULL)
	{
		rv = SecAgreeSecurityDataConstructSecurityList(pSecAgreeMgr, hPage, pSecAgreeSecurityData, eListType, phList);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeSecurityDataPushSecurityHeader - Failed to construct security list. rv=%d", rv));
			return rv;
		}
	}

	if (pSecAgreeMgr->hMemoryPool != SipSecurityHeaderGetPool(hSecurityHeader) ||
	    hPage != SipSecurityHeaderGetPage(hSecurityHeader))
	{
		/* The given header does not lie on the same page as the data, therefore it needs to be
		   copied there */

		/* Construct a new header on the given memory page */
		rv = RvSipSecurityHeaderConstruct(pSecAgreeMgr->hMsgMgr, pSecAgreeMgr->hMemoryPool, hPage, &hSecHeaderToPush);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
						"SecAgreeSecurityDataPushSecurityHeader - Failed to construct new security header. rv=%d", rv));
			return rv;
		}

		/* Copy the given header to the new one */
		rv = RvSipSecurityHeaderCopy(hSecHeaderToPush, hSecurityHeader);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
						"SecAgreeSecurityDataPushSecurityHeader - Failed to copy security header. rv=%d", rv));
			return rv;
		}
	}
	else
	{
		/* Same memory page: no need to copy */
		hSecHeaderToPush = hSecurityHeader;
	}

	if (NULL == phNewPos)
	{
		phNewPos = &hListElem;
	}

	/* Push the header to the local security list */
	rv = RvSipCommonListPushElem(*phList,
								 RVSIP_HEADERTYPE_SECURITY,
								 (void*)hSecHeaderToPush,
								 eLocation,
								 hPos,
								 phNewPos);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"SecAgreeSecurityDataPushSecurityHeader - Failed to push security header 0x%p to local security list, rv=%d",
					hSecurityHeader, rv));
	}

	if (phNewSecurityHeader != NULL)
	{
		/* Output the new header handle */
		*phNewSecurityHeader = hSecHeaderToPush;
	}

	return rv;
}

/***************************************************************************
 * SecAgreeSecurityDataSetProxyAuth
 * ------------------------------------------------------------------------
 * General: Set Proxy-Authenticate header to the server authentication info.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr        - Pointer to the Security-Mgr
 *         hPage               - The memory page to construct the header on (if needed)
 *         pSecAgreeSecurityData       - Pointer to the Security-Agreement-Data
 *         hProxyAuthHeader    - The Proxy-Authenticate header to push
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeSecurityDataSetProxyAuth(
									IN    SecAgreeMgr*				      pSecAgreeMgr,
									IN    HPAGE                           hPage,
									IN    SecAgreeSecurityData*			  pSecAgreeSecurityData,
									IN    RvSipAuthenticationHeaderHandle hProxyAuthHeader)
{
	RvSipAuthenticationHeaderHandle hProxyAuthHeaderToSet;
	RvStatus                        rv = RV_OK;

	if (pSecAgreeMgr->hMemoryPool != SipAuthenticationHeaderGetPool(hProxyAuthHeader) ||
	    hPage != SipAuthenticationHeaderGetPage(hProxyAuthHeader))
	{
		/* The given header does not lie on the same page as the data, therefore it needs to be
		   copied there */

		/* Construct a new header on the given memory page */
		rv = RvSipAuthenticationHeaderConstruct(pSecAgreeMgr->hMsgMgr, pSecAgreeMgr->hMemoryPool, hPage, &hProxyAuthHeaderToSet);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
						"SecAgreeSecurityDataSetProxyAuth - Failed to construct new Proxy-Authenticate header. rv=%d", rv));
			return rv;
		}

		/* Copy the given header to the new one */
		rv = RvSipAuthenticationHeaderCopy(hProxyAuthHeaderToSet, hProxyAuthHeader);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
						"SecAgreeSecurityDataSetProxyAuth - Failed to copy Proxy-Authenticate header. rv=%d", rv));
			return rv;
		}
	}
	else
	{
		/* Same memory page: no need to copy */
		hProxyAuthHeaderToSet = hProxyAuthHeader;
	}

	pSecAgreeSecurityData->additionalKeys.authInfo.hServerAuth.hProxyAuth = hProxyAuthHeaderToSet;

	return rv;
}

/***************************************************************************
 * SecAgreeSecurityDataCopy
 * ------------------------------------------------------------------------
 * General: Copies local information from source security-agreement data to
 *          destination security-agreement data. The fields being copied here are:
 *          1. Local security list.
 *          2. Auth header for server security-agreement only.
 *          Notice: Not all fields are being copied here. For example, the
 *          remote security list is not copied.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr        - Pointer to the security-agreement manager
 *         pSourceSecAgreeSecurityData - Pointer to the source security-agreement-data
 *         pDestSecAgreeSecurityData   - Pointer to the destination security-agreement-data
 *         eRole               - The role of this security-agreement
 *         hPage               - The memory page to be used by the destination
 *                               security-agreement-data
 *         pbResetRemote       - Indicates in case of failure whether the remote list should be 
 *                               reset
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeSecurityDataCopy(
									 IN    SecAgreeMgr*              pSecAgreeMgr,
                                     IN    SecAgree*                 pSourceSecAgree,
									 IN    SecAgreeSecurityData*	 pSourceSecAgreeSecurityData,
									 IN    SecAgreeSecurityData*	 pDestSecAgreeSecurityData,
									 IN    RvSipSecAgreeRole         eRole,
									 IN    HPAGE                     hPage,
									 OUT   RvBool                   *pbResetRemote)
{
	RvStatus                   rv;

	*pbResetRemote = RV_FALSE;

	/* Copy local security list */
	rv = SecAgreeSecurityDataCopySecurityList(pSecAgreeMgr, pSourceSecAgree, 
											  pSourceSecAgreeSecurityData, pDestSecAgreeSecurityData, 
											  SEC_AGREE_SEC_DATA_LIST_LOCAL, hPage, pbResetRemote);
	if (RV_OK != rv)
	{
		return rv;
	}

	/* Copy the remote security list in case of TISPAN support. This was the client security-agreement
	   will add Security-Verify to outgoing REGISTER that starts renegotiation process, as specified
	   in TISPAN. */
	if (eRole == RVSIP_SEC_AGREE_ROLE_CLIENT &&
		pDestSecAgreeSecurityData->hRemoteSecurityList == NULL &&
		SecAgreeUtilsIsGmInterfaceSupport(pSourceSecAgree) == RV_TRUE)
	{
		/* Copy local security list */
		rv = SecAgreeSecurityDataCopySecurityList(pSecAgreeMgr, pSourceSecAgree, 
												  pSourceSecAgreeSecurityData, pDestSecAgreeSecurityData, 
												  SEC_AGREE_SEC_DATA_LIST_REMOTE, hPage, pbResetRemote);
		if (RV_OK != rv)
		{
			return rv;
		}
	}
	/* Copy the local security list in case of TISPAN support. This will be compared to the initially received 
	   Security-Verify list from the client, as specified in TISPAN. After this initial comparison, the
	   server will insert a new Security-Server list to the response, and the next time the client would
	   send a Security-Verify list it would reflect the new Security-Server list. */
	if (eRole == RVSIP_SEC_AGREE_ROLE_SERVER &&
		SecAgreeUtilsIsGmInterfaceSupport(pSourceSecAgree) == RV_TRUE)
	{
		/* Copy TISPAN local security list */
		rv = SecAgreeSecurityDataCopySecurityList(pSecAgreeMgr, pSourceSecAgree, 
												  pSourceSecAgreeSecurityData, pDestSecAgreeSecurityData, 
												  SEC_AGREE_SEC_DATA_LIST_TISPAN_LOCAL, hPage, pbResetRemote);
		if (RV_OK != rv)
		{
			return rv;
		}
	}

	/* If this is a server security-agreement data, we copy the Proxy-Authenticate header as part
	   of the local data */
	if (eRole == RVSIP_SEC_AGREE_ROLE_SERVER)
	{
		if (pSourceSecAgreeSecurityData->additionalKeys.authInfo.hServerAuth.hProxyAuth != NULL)
		{
			rv = SecAgreeSecurityDataSetProxyAuth(pSecAgreeMgr, hPage, pDestSecAgreeSecurityData,
												  pSourceSecAgreeSecurityData->additionalKeys.authInfo.hServerAuth.hProxyAuth);
			if (RV_OK != rv)
			{
				return rv;
			}
		}
	}

	return RV_OK;
}

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

RvStatus RVCALLCONV SecAgreeSecurityDataSetPortC(
									 IN    SecAgreeSecurityData*			pSecAgreeSecurityData,
                   IN    int portC) {

  RvSipCommonListHandle      hList;
	RvSipCommonListElemHandle  hCurrElem;
	RvSipCommonListElemHandle  hNextElem;
	RvSipSecurityHeaderHandle  hSecurityHeader;
	RvInt                      elementType;
	RvInt                      safeCounter  = 0;
	RvStatus                   rv = 0;
    
	hList   = pSecAgreeSecurityData->hLocalSecurityList;

	if (hList == NULL) {
		return RV_ERROR_UNKNOWN;
	}
	
	/* Go over the elements */
	rv = RvSipCommonListGetElem(hList, RVSIP_FIRST_ELEMENT, NULL, &elementType, (void**)&hSecurityHeader, &hCurrElem);
	while (rv == RV_OK && hCurrElem != NULL && safeCounter < SEC_AGREE_MAX_LOOP(pSecAgreeSecurityData))
	{	
		/* Copy and push the security header to the remote list of this security-agreement data */
		RvSipSecurityHeaderSetPortC(hSecurityHeader, portC);

		rv = RvSipCommonListGetElem(hList, RVSIP_NEXT_ELEMENT, hCurrElem, &elementType, (void**)&hSecurityHeader, &hNextElem);
		hCurrElem = hNextElem;
		safeCounter += 1;
	}
	
	return RV_OK;
}

#endif
/* SPIRENT_END */

/***************************************************************************
 * SecAgreeSecurityDataResetData
 * ------------------------------------------------------------------------
 * General: Resets the security data structure in case of failure in the 
 *          middle of copy
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecurityData - The security-data to reset
 *         eRole         - The role of this security-agreement
 *         bIsLocal      - Indication whether to reset local data or remote
 ***************************************************************************/
void RVCALLCONV SecAgreeSecurityDataResetData(
									 IN    SecAgreeSecurityData*		pSecurityData,
									 IN    RvSipSecAgreeRole			eRole,
									 IN    SecAgreeSecurityDataListType eListType)
{
	SecAgreeSecurityDataDestructSecurityList(pSecurityData, eListType);

	if (eListType == SEC_AGREE_SEC_DATA_LIST_LOCAL &&
		eRole == RVSIP_SEC_AGREE_ROLE_SERVER)
	{
		pSecurityData->additionalKeys.authInfo.hServerAuth.hProxyAuth = NULL;
	}
}

/***************************************************************************
 * SecAgreeSecurityDataIsLocalSecObj
 * ------------------------------------------------------------------------
 * General: Indicates whether the local list requires the creation of a 
 *          security object, i.e. contains ipsec or tls.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr    - Pointer to the Security-Mgr
 * Output: pbIsLocalSecObj - Boolean indication to whether there should be a 
 *                           security object
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeSecurityDataIsLocalSecObj(
							 IN    SecAgreeSecurityData*    pSecAgreeSecurityData,
							 OUT   RvBool*                  pbIsLocalSecObj)
{
#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) 
	RvInt                      safeCounter  = 0;
    RvSipCommonListElemHandle  hListElem;
	RvSipSecurityHeaderHandle  hSecurityHeader;
	RvInt                      elementType;
	RvSipSecurityMechanismType eMechanism;
	RvStatus                   rv = RV_OK;

	*pbIsLocalSecObj = RV_FALSE;

	if (pSecAgreeSecurityData->hLocalSecurityList == NULL)
	{
		return RV_OK;
	}
	
	/* Go over the elements and copy them into the destination security-agreement */
	rv = RvSipCommonListGetElem(pSecAgreeSecurityData->hLocalSecurityList, RVSIP_FIRST_ELEMENT, 
								NULL, &elementType, (void**)&hSecurityHeader, &hListElem);
	while (rv == RV_OK && hListElem != NULL && safeCounter < SEC_AGREE_MAX_LOOP(pSecAgreeSecurityData))
	{
	    eMechanism = RvSipSecurityHeaderGetMechanismType(hSecurityHeader);
		if (SecAgreeUtilsIsMechanismHandledBySecObj(eMechanism) == RV_TRUE)
		{
			*pbIsLocalSecObj = RV_TRUE;
			return RV_OK;
		}
		
		rv = RvSipCommonListGetElem(pSecAgreeSecurityData->hLocalSecurityList, RVSIP_NEXT_ELEMENT, 
									hListElem, &elementType, (void**)&hSecurityHeader, &hListElem);
		safeCounter += 1;
	}
	return rv;

#else /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)  */

	/* There is no security-object when compiled without ipsec and TLS */
	*pbIsLocalSecObj = RV_FALSE;
	RV_UNUSED_ARG(pSecAgreeSecurityData)
	return RV_OK;
	
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)  */	
}

/*------------------------------------------------------------------------
           S T A T I C   F U N C T I O N S
 -------------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeSecurityHeaderRunOver
 * ------------------------------------------------------------------------
 * General: Retrieve old parameters from the security header and sets the
 *          given ones.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecurityHeader - Handle to the security header
 * Inout:  pSpiC           - Pointer to spic
 *         pbIsInitSpiC    - Pointer to boolean indication to whether spic was initialized
 *         pSpiC           - Pointer to spis
 *         pbIsInitSpiS    - Pointer to boolean indication to whether spis was initialized
 *         pPortC          - Pointer to portc
 *         pPortS          - Pointer to ports. Can be NULL;
 ***************************************************************************/
static void RVCALLCONV SecAgreeSecurityHeaderRunOver(
									 IN    RvSipSecurityHeaderHandle   hSecurityHeader,
									 INOUT RvUint32*                   pSpiC,
									 INOUT RvBool*                     pbIsInitSpiC,
									 INOUT RvUint32*                   pSpiS,
									 INOUT RvBool*                     pbIsInitSpiS,
									 INOUT RvInt32*                    pPortC,
                                     INOUT RvInt32*                    pPortS)
{
	RvUint32  oldSpiC;
	RvBool    bIsInitSpiC;
	RvUint32  oldSpiS;
	RvBool    bIsInitSpiS;
    RvInt32   oldPortC;
    RvInt32   oldPortS;
	
	/* Store old values */
    oldPortC = RvSipSecurityHeaderGetPortC(hSecurityHeader);
    oldPortS = RvSipSecurityHeaderGetPortS(hSecurityHeader);
	RvSipSecurityHeaderGetSpiC(hSecurityHeader, &bIsInitSpiC, &oldSpiC);
	RvSipSecurityHeaderGetSpiC(hSecurityHeader, &bIsInitSpiS, &oldSpiS);

	/* Run with new values */
	RvSipSecurityHeaderSetPortC(hSecurityHeader, *pPortC);
	RvSipSecurityHeaderSetSpiC(hSecurityHeader, *pbIsInitSpiC, *pSpiC);
	RvSipSecurityHeaderSetSpiS(hSecurityHeader, *pbIsInitSpiS, *pSpiS);
    if (NULL != pPortS)
    {
        RvSipSecurityHeaderSetPortS(hSecurityHeader, *pPortS);
    }

	/* return old values */
	*pPortC       = oldPortC;
	*pSpiC        = oldSpiC;
	*pbIsInitSpiC = bIsInitSpiC;
	*pSpiS        = oldSpiS;
	*pbIsInitSpiS = bIsInitSpiS;
    if (NULL != pPortS)
    {
        *pPortS = oldPortS;
    }
}

/***************************************************************************
 * SecAgreeSecurityDataCopySecurityList
 * ------------------------------------------------------------------------
 * General: Copy the source security list onto the destination security list
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr                - Pointer to the security-agreement manager
 *         pSourceSecAgree             - The source security-agreement
 *         pSourceSecAgreeSecurityData - Pointer to the source security-agreement-data
 *         pDestSecAgreeSecurityData   - Pointer to the destination security-agreement-data
 *         eListType                   - Indicates the type of list to copy: local, remote
 *                                       ot local for TISPAN comparison
 *         hPage					   - The memory page to be used by the destination
 *									     security-agreement-data
 *         pbResetRemote               - Indicates in case of failure whether the remote list 
 *                                       should be reset
 ***************************************************************************/
static RvStatus RVCALLCONV SecAgreeSecurityDataCopySecurityList(
									IN    SecAgreeMgr*					pSecAgreeMgr,
                                    IN    SecAgree*						pSourceSecAgree,
									IN    SecAgreeSecurityData*			pSourceSecAgreeSecurityData,
                                    IN    SecAgreeSecurityData*			pDestSecAgreeSecurityData,
									IN    SecAgreeSecurityDataListType  eListType,
									IN    HPAGE							hPage,
									OUT   RvBool*						pbResetRemote)
{
	RvSipCommonListHandle      hSrcList;
	RvSipCommonListElemHandle  hCurrElem;
	RvSipCommonListElemHandle  hNextElem;
	RvSipSecurityHeaderHandle  hSecurityHeader;
	RvInt                      elementType;
	RvInt                      safeCounter  = 0;
	RvUint32                   spiC         = 0;
	RvBool                     bIsInitPortC = RV_FALSE;
	RvUint32                   spiS         = 0;
	RvBool                     bIsInitPortS = RV_FALSE;
	RvInt32                    portC        = UNDEFINED;
    RvInt32                    portS        = UNDEFINED;
    RvInt32*                   pPortS       = NULL;
	RvStatus                   rv;
    
	/* If the underlying Security Object doesn't listens on the Port-S,
    the handle of the Local Addresses, bound to the Port-S, should not
    be stored in the SegAgree object. In this case Port-S should not be copied
    into the destination security list. */
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    if (pSourceSecAgree->addresses.hLocalIpsecUdpAddr == NULL &&
        pSourceSecAgree->addresses.hLocalIpsecTcpAddr == NULL)
    {
        pPortS = &portS;  /* Prevent copying of Port-S */
        /*pPortS = NULL;*/ /* Cause copying of Port-S */
		RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				  "SecAgreeSecurityDataCopySecurityList - Do not copy Port-S from sec-agree=0x%p since the underlying Security Object doesn't listens on the Port-S", pSourceSecAgree));
    }
#else
    RV_UNUSED_ARG(pSourceSecAgree || portS)
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
		
	/* Identify the list to copy */
	switch (eListType) 
	{
	case SEC_AGREE_SEC_DATA_LIST_LOCAL:
		hSrcList   = pSourceSecAgreeSecurityData->hLocalSecurityList;
		break;
	case SEC_AGREE_SEC_DATA_LIST_REMOTE:
		hSrcList   = pSourceSecAgreeSecurityData->hRemoteSecurityList;
		break;
	case SEC_AGREE_SEC_DATA_LIST_TISPAN_LOCAL:
		hSrcList   = pSourceSecAgreeSecurityData->hTISPANLocalList;
		break;
	default:
		return RV_ERROR_UNKNOWN;
	}

	if (hSrcList == NULL)
	{
		return RV_OK;
	}
	
	/* Construct local list at destination security-agreement */
	rv = SecAgreeSecurityDataConstructSecurityList(pSecAgreeMgr, hPage, pDestSecAgreeSecurityData, eListType, NULL);
	if (RV_OK != rv)
	{
		return rv;
	}
	if (eListType == SEC_AGREE_SEC_DATA_LIST_REMOTE)
	{
		*pbResetRemote = RV_TRUE;
	}
	
	/* Go over the elements and copy them into the destination security-agreement */
	rv = RvSipCommonListGetElem(hSrcList, RVSIP_FIRST_ELEMENT, NULL, &elementType, (void**)&hSecurityHeader, &hCurrElem);
	while (rv == RV_OK && hCurrElem != NULL && safeCounter < SEC_AGREE_MAX_LOOP(pDestSecAgreeSecurityData))
	{
		
		/* Copy and push the security header to the remote list of this security-agreement data */
		if (eListType != SEC_AGREE_SEC_DATA_LIST_REMOTE)
		{
			/* We do not copy spic, spis, portc and, sometimes, ports. Temporarily reset them in the source header. */
			SecAgreeSecurityHeaderRunOver(hSecurityHeader, &spiC, &bIsInitPortC, &spiS, &bIsInitPortS, &portC, pPortS);
		}

		rv = SecAgreeSecurityDataPushSecurityHeader(pSecAgreeMgr, hPage, pDestSecAgreeSecurityData, hSecurityHeader, RVSIP_LAST_ELEMENT, NULL, eListType, NULL, NULL);
	
		if (eListType != SEC_AGREE_SEC_DATA_LIST_REMOTE)
		{
			/* Restore original spic, spis and portc */
			SecAgreeSecurityHeaderRunOver(hSecurityHeader, &spiC, &bIsInitPortC, &spiS, &bIsInitPortS, &portC, pPortS);
		}		
		
		if (RV_OK != rv)
		{
			break;
		}

		rv = RvSipCommonListGetElem(hSrcList, RVSIP_NEXT_ELEMENT, hCurrElem, &elementType, (void**)&hSecurityHeader, &hNextElem);
		hCurrElem = hNextElem;
		safeCounter += 1;
	}
	
	return rv;
}


#endif /* #ifdef RV_SIP_IMS_ON */

