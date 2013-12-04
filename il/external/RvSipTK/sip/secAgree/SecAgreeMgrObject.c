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
 *                              <SecAgreeMgrObject.c>
 *
 * This file defines the security-agrement manager object, attributes and interface functions.
 * The security-agreement manager manager the activity of all security-agreement clients and
 * servers of the SIP stack.
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

#include "SecAgreeMgrObject.h"
#include "SecAgreeObject.h"
#include "SecAgreeTypes.h"
#include "SecAgreeServer.h"
#include "SecAgreeClient.h"
#include "SecAgreeUtils.h"
#include "_SipTransport.h"

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvBool OwnersHashCompare(IN void *newHashElement,
                                IN void *oldHashElement);

static RvStatus RVCALLCONV OwnersHashInsert(
						IN SecAgreeMgr                  *pSecAgreeMgr,
                        IN SecAgree                     *pSecAgree,
                        IN SecAgreeOwnerRecord          *pOwner);

static RvStatus RVCALLCONV OwnersHashRemove(
						IN SecAgreeMgr           *pSecAgreeMgr,
                        IN void                  *pHashElement);

static void RVCALLCONV OwnersHashFind(
						 IN  SecAgreeMgr					*pSecAgreeMgr,
                         IN  SecAgree				        *pSecAgree,
						 IN  void							*pOwner,
                         OUT SecAgreeOwnerRecord		   **ppOwnerInfo);

#if (RV_IMS_IPSEC_ENABLED == RV_YES)
static void RVCALLCONV InitAddressDetails(IN  RvSipTransportAddr   *pAddr);
#endif /* (RV_IMS_IPSEC_ENABLED == RV_YES) */

/*-----------------------------------------------------------------------*/
/*                         SECURITY CLIENT FUNCTIONS                     */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeMgrCreateSecAgree
 * ------------------------------------------------------------------------
 * General: Creates a new Security-Agreement object on the Security-Mgr's pool.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr       - Pointer to the security-mgr.
 *         hAppSecAgree       - The application handle for the new security-agreement.
 * Output: phSecAgree         - Pointer to the new security-agreement.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMgrCreateSecAgree(
										IN  SecAgreeMgr*                    pSecAgreeMgr,
										IN  RvSipAppSecAgreeHandle          hAppSecAgree,
										OUT RvSipSecAgreeHandle*            phSecAgree)
{
	RvStatus            rv;
    RLIST_ITEM_HANDLE   listItem;
	SecAgree*           pSecAgree;

    *phSecAgree = NULL;

	if(pSecAgreeMgr->hSecAgreePool == NULL)
    {
        RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
                   "SecAgreeMgrCreateSecAgree - Failed. The security-manager was initialized with 0 security-agreement"));
        return RV_ERROR_UNKNOWN;
    }
    
	/* lock manager */
	RvMutexLock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);

    rv = RLIST_InsertTail(pSecAgreeMgr->hSecAgreePool,pSecAgreeMgr->hSecAgreeList,&listItem);
	if(rv != RV_OK)
    {
		/*there are no more available items*/
        RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
                  "SecAgreeMgrCreateSecAgree - Failed to insert new security-agreement to list (rv=%d)",
                  rv));
        RvMutexUnlock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);
        return rv;
    }

	pSecAgree    = (SecAgree*)listItem;

	RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
             "SecAgreeMgrCreateSecAgree - security-agreement 0x%p was created.", pSecAgree));

	rv = SecAgreeInitialize(pSecAgreeMgr, hAppSecAgree, pSecAgree);

	if(rv != RV_OK)
	{
		/*initialization failed*/
        *phSecAgree = NULL;
		RLIST_Remove(pSecAgreeMgr->hSecAgreePool, pSecAgreeMgr->hSecAgreeList, listItem);
        RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
                  "SecAgreeMgrCreateSecAgree - Failed to initialize the new security-agreement 0x%p (rv=%d)",
                  pSecAgree, rv));
        RvMutexUnlock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);
        return rv;
	}

	*phSecAgree = (RvSipSecAgreeHandle)pSecAgree;

	/* unlock manager */
    RvMutexUnlock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);
    
	return RV_OK;
}

/***************************************************************************
 * SecAgreeMgrRemoveSecAgree
 * ------------------------------------------------------------------------
 * General: Removes a security-agreement object from the Security-Mgr's pool.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr    - Pointer to the security-mgr.
 *         pSecAgree       - Pointer to the security-agreement object to remove.
 ***************************************************************************/
void RVCALLCONV SecAgreeMgrRemoveSecAgree(
										IN  SecAgreeMgr*				pSecAgreeMgr,
										IN  SecAgree*			        pSecAgree)
{
	/* lock manager */
	RvMutexLock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);

	/* Remove security-agreement from list */
	RLIST_Remove(pSecAgreeMgr->hSecAgreePool, pSecAgreeMgr->hSecAgreeList, (RLIST_ITEM_HANDLE)pSecAgree);

	/* unlock manager */
	RvMutexUnlock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);
}

/***************************************************************************
 * SecAgreeMgrGetEvHandlers
 * ------------------------------------------------------------------------
 * General: Gets the event handlers structure of the given owner.
 * Return Value: The event handlers
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecAgreeMgr    - Pointer to the Security-Mgr
 *          eOwnerType      - The type of owner to get its event handlers.
 ***************************************************************************/
SipSecAgreeEvHandlers* RVCALLCONV SecAgreeMgrGetEvHandlers(
							IN   SecAgreeMgr                      *pSecAgreeMgr,
							IN   RvSipCommonStackObjectType        eOwnerType)
{
	if ((RvInt32)eOwnerType >= pSecAgreeMgr->evHandlersSize)
	{
		return NULL;
	}

	return &(pSecAgreeMgr->pEvHandlers[eOwnerType]);
}

/***************************************************************************
 * SecAgreeMgrGetDefaultLocalAddr
 * ------------------------------------------------------------------------
 * General: Initializes the local address structure of the security-agreement
 *          with default values
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr    - Pointer to the security-agreement mgr.
 * Inout:  pSecAgree       - Pointer to the security-agreement object
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMgrGetDefaultLocalAddr(
								IN    SecAgreeMgr*				 pSecAgreeMgr,
								INOUT SecAgree*                  pSecAgree)
{
#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
	SipTransportObjLocalAddresses   localAddresses;
    RvStatus                        rv;
#if (RV_IMS_IPSEC_ENABLED == RV_YES)
    RvSipTransportLocalAddrHandle   hLocalAddrDefault;
	RvSipTransportAddr              defaultAddrDetails;
	RvUint32                        sizeOfAddr = sizeof(RvSipTransportAddr);
#endif /*#if (RV_IMS_IPSEC_ENABLED == RV_YES)*/
	
    /* Retrieve local addresses, opened in the Stack */
	rv = SipTransportLocalAddressInitDefaults(
                pSecAgreeMgr->hTransport, &localAddresses);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				   "SecAgreeMgrGetDefaultLocalAddr - Security-agreement 0x%p: Failed to get structure of local addresses (rv=%d)", 
				   pSecAgree, rv));
		return rv;
	}

#if (RV_IMS_IPSEC_ENABLED == RV_YES)		
	/* Set UDP address into the SecAgree */
    hLocalAddrDefault = NULL;
    if (NULL != localAddresses.hLocalUdpIpv4)
    {
        hLocalAddrDefault = localAddresses.hLocalUdpIpv4;
        RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
            "SecAgreeMgrGetDefaultLocalAddr - UDP IPv4 address %p was set by default",
            hLocalAddrDefault));
    }
    if (NULL == hLocalAddrDefault && NULL != localAddresses.hLocalUdpIpv6)
    {
        hLocalAddrDefault = localAddresses.hLocalUdpIpv6;
        RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
            "SecAgreeMgrGetDefaultLocalAddr - UDP IPv6 address %p was set by default",
            hLocalAddrDefault));
    }
    if (NULL == hLocalAddrDefault)
    {
        RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
            "SecAgreeMgrGetDefaultLocalAddr - no UDP address to be used by default was found"));
    }
    else
	{
		InitAddressDetails(&defaultAddrDetails);
        rv = RvSipTransportMgrLocalAddressGetDetails(
                hLocalAddrDefault, sizeOfAddr, &defaultAddrDetails);
		if (RV_OK == rv)
		{
			rv = SecAgreeObjectSetLocalAddr(pSecAgree, &defaultAddrDetails);
            if (RV_OK != rv)
            {
                RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
                    "SecAgreeMgrGetDefaultLocalAddr - Security-agreement 0x%p: Failed to use local UDP address %p as a default (rv=%d:%s)", 
                    pSecAgree, hLocalAddrDefault, rv, SipCommonStatus2Str(rv)));
            }
		}
	}

    /* Set TCP address into the SecAgree */
    hLocalAddrDefault = NULL;
    if (NULL != localAddresses.hLocalTcpIpv4)
    {
        hLocalAddrDefault = localAddresses.hLocalTcpIpv4;
        RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
            "SecAgreeMgrGetDefaultLocalAddr - TCP IPv4 address %p was set by default",
            hLocalAddrDefault));
    }
    if (NULL == hLocalAddrDefault && NULL != localAddresses.hLocalTcpIpv6)
    {
        hLocalAddrDefault = localAddresses.hLocalTcpIpv6;
        RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
            "SecAgreeMgrGetDefaultLocalAddr - TCP IPv6 address %p was set by default",
            hLocalAddrDefault));
    }
    if (NULL == hLocalAddrDefault)
    {
        RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
            "SecAgreeMgrGetDefaultLocalAddr - no TCP address to be used by default was found"));
    }
    else
    {
        InitAddressDetails(&defaultAddrDetails);
        rv = RvSipTransportMgrLocalAddressGetDetails(
                hLocalAddrDefault, sizeOfAddr, &defaultAddrDetails);
        if (RV_OK == rv)
        {
            rv = SecAgreeObjectSetLocalAddr(pSecAgree, &defaultAddrDetails);
            if (RV_OK != rv)
            {
                RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
                    "SecAgreeMgrGetDefaultLocalAddr - Security-agreement 0x%p: Failed to use local TCP address %p as a default (rv=%d:%s)", 
                    pSecAgree, hLocalAddrDefault, rv, SipCommonStatus2Str(rv)));
            }
        }
    }

#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */

#if (RV_TLS_TYPE != RV_TLS_NONE)
    /* Set TLS IPv4 address into the SecAgree */
    if (NULL != localAddresses.hLocalTlsIpv4)
    {
        pSecAgree->addresses.hLocalTlsIp4Addr = localAddresses.hLocalTlsIpv4;
    }
    else
    {
        RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
            "SecAgreeMgrGetDefaultLocalAddr - no default TLS IPv4 address was found"));
    }

    /* Set TLS IPv6 address into the SecAgree */
    if (NULL != localAddresses.hLocalTlsIpv4)
    {
        pSecAgree->addresses.hLocalTlsIp6Addr = localAddresses.hLocalTlsIpv6;
    }
    else
    {
        RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
            "SecAgreeMgrGetDefaultLocalAddr - no default TLS IPv6 address was found"));
    }
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */ 

#if (RV_IMS_IPSEC_ENABLED != RV_YES) && (RV_TLS_TYPE == RV_TLS_NONE)
	RV_UNUSED_ARG(pSecAgreeMgr);
	RV_UNUSED_ARG(pSecAgree);
#endif

	return RV_OK;	
}

/***************************************************************************
 * SecAgreeMgrInsertSecAgreeOwner
 * ------------------------------------------------------------------------
 * General: Inserts a new owner of this security-agreement to the owner's hash
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr     - Pointer to the Security-Mgr.
 *         pSecAgree        - Pointer to the security-agreement
 *         pOwnerInfo       - Pointer to the owner information record.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMgrInsertSecAgreeOwner(
										IN  SecAgreeMgr*         pSecAgreeMgr,
										IN  SecAgree*            pSecAgree,
										IN  SecAgreeOwnerRecord* pOwnerInfo)
{
	return OwnersHashInsert(pSecAgreeMgr, pSecAgree, pOwnerInfo);
}

/***************************************************************************
 * SecAgreeMgrFindSecAgreeOwner
 * ------------------------------------------------------------------------
 * General: Searches for the hash entry of the given owner for this security-
 *          agreement.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr     - Pointer to the Security-Mgr.
 *         pSecAgree        - Pointer to the security-agreement
 *         pOwner           - Pointer to the owner object.
 * Output: ppOwnerInfo      - Pointer to the owner record info.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMgrFindSecAgreeOwner(
										IN  SecAgreeMgr*          pSecAgreeMgr,
										IN  SecAgree*             pSecAgree,
										IN  void*                 pOwner,
										OUT SecAgreeOwnerRecord** ppOwnerInfo)
{
	SecAgreeOwnerRecord* pOwnerInfo;

	OwnersHashFind(pSecAgreeMgr, pSecAgree, pOwner, &pOwnerInfo);

	if (NULL == pOwnerInfo)
	{
		*ppOwnerInfo = NULL;
		return RV_ERROR_NOT_FOUND;
	}

	*ppOwnerInfo = pOwnerInfo;

	return RV_OK;
}

/***************************************************************************
 * SecAgreeMgrRemoveOwner
 * ------------------------------------------------------------------------
 * General: Removes an owner given its hash entry
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr     - Pointer to the Security-Mgr
 *         pHashElement     - Pointer to the hash element
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMgrRemoveOwner(
										IN  SecAgreeMgr*         pSecAgreeMgr,
										IN  void*                pHashElement)
{
	return OwnersHashRemove(pSecAgreeMgr, pHashElement);
}

/*------------------------------------------------------------------------
           S T A T I C   F U N C T I O N S
 -------------------------------------------------------------------------*/

/*-------------------- OWNERS HASH FUNCTIONS ----------------------*/

/***************************************************************************
* OwnersHashCompare
* ------------------------------------------------------------------------
* General: Used to compare to keys that where mapped to the same hash
*          value in order to decide whether they are the same
* Return Value: TRUE if the keys are equal, FALSE otherwise.
* ------------------------------------------------------------------------
* Arguments:
* Input:     newHashElement  - a new hash element
*            oldHashElement  - an element from the hash
***************************************************************************/
static RvBool OwnersHashCompare(IN void *newHashElement,
                                IN void *oldHashElement)
{
    SecAgreeOwnerHashKey   *newHashElem = (SecAgreeOwnerHashKey *)newHashElement;
    SecAgreeOwnerHashKey   *oldHashElem = (SecAgreeOwnerHashKey *)oldHashElement;

    if(newHashElem->hOwner != oldHashElem->hOwner ||
       newHashElem->pSecAgree != oldHashElem->pSecAgree)
    {
        return RV_FALSE;
    }
    return RV_TRUE;
}


/***************************************************************************
 * OwnersHashInsert
 * ------------------------------------------------------------------------
 * General: Insert a security-object owner into the hash table.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr     - Pointer to the security manager
 *         pSecurityObject  - Pointer of the security-object to insert to the hash
 *         eRole            - Indicate whether this is client or server
 *         pOwner           - The owner information.
 ***************************************************************************/
static RvStatus RVCALLCONV OwnersHashInsert(
						IN SecAgreeMgr                  *pSecAgreeMgr,
                        IN SecAgree                     *pSecAgree,
                        IN SecAgreeOwnerRecord          *pOwner)
{
    RvStatus                  rv;
    SecAgreeOwnerHashKey      hashKey;
    RvBool                    wasInsert;

    /*prepare hash key*/
    hashKey.pSecAgree       = pSecAgree;
	hashKey.hOwner          = pOwner->owner.pOwner;

	/* lock manager */
    RvMutexLock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);

	/* Insert owner to hash */
	rv = HASH_InsertElement(pSecAgreeMgr->hOwnersHash,
                            &hashKey,
                            &pOwner,
                            RV_FALSE,
                            &wasInsert,
                            &(pOwner->pHashElement),
                            OwnersHashCompare);
	/* unlock manager */
	RvMutexUnlock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);

    if(rv != RV_OK || wasInsert == RV_FALSE)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }

    RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
              "OwnersHashInsert- owner 0x%p - was inserted to hash (security-agreement = 0x%p) ",
			  pOwner->owner.pOwner, pSecAgree));

    return RV_OK;
}

/***************************************************************************
 * OwnersHashRemove
 * ------------------------------------------------------------------------
 * General: Removes a security owner from the hash table.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr     - Pointer to the security manager
 *         pHashElement     - Pointer to the hash element
 ***************************************************************************/
static RvStatus RVCALLCONV OwnersHashRemove(
						IN SecAgreeMgr           *pSecAgreeMgr,
                        IN void                  *pHashElement)
{
	RvStatus  rv;

	/* lock manager */
	RvMutexLock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);

	/* Remove owner from hash */
    rv = HASH_RemoveElement(pSecAgreeMgr->hOwnersHash, pHashElement);

	/* unlock manager */
	RvMutexUnlock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);

	return rv;
}

/***************************************************************************
 * OwnersHashFind
 * ------------------------------------------------------------------------
 * General: find an owner in the owners hash table according to a given handle.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecAgreeMgr     - Pointer to the security manager
 *          pSecAgree        - Pointer to the security-agreement
 *          eRole            - Indicate whether this is client or server
 *          pOwner           - Pointer to the owner
 * Output:  ppOwnerInfo      - The complete owner info that was found in the hash
 ***************************************************************************/
static void RVCALLCONV OwnersHashFind(
						 IN  SecAgreeMgr					*pSecAgreeMgr,
                         IN  SecAgree				        *pSecAgree,
						 IN  void							*pOwner,
                         OUT SecAgreeOwnerRecord		   **ppOwnerInfo)
{
    RvStatus                    rv;
    RvBool                      ownerFound;
    void                       *elementPtr;
    SecAgreeOwnerHashKey        hashKey;

    /*prepare hash key*/
    hashKey.pSecAgree       = pSecAgree;
    hashKey.hOwner          = pOwner;

	/* lock manager */
    RvMutexLock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);
    
	/* Find owner in hash */
	ownerFound = HASH_FindElement(pSecAgreeMgr->hOwnersHash, &hashKey,
                                  OwnersHashCompare, &elementPtr);

    /*return if record not found*/
    if(ownerFound == RV_FALSE || elementPtr == NULL)
    {
        *ppOwnerInfo = NULL;
        RvMutexUnlock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);
        return;
    }
    rv = HASH_GetUserElement(pSecAgreeMgr->hOwnersHash,
                             elementPtr,(void*)ppOwnerInfo);
    if(rv != RV_OK)
    {
        *ppOwnerInfo = NULL;
        RvMutexUnlock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);
        return;
    }

	/* unlock manager */
	RvMutexUnlock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);
}

#if (RV_IMS_IPSEC_ENABLED == RV_YES)
/***************************************************************************
 * InitAddressDetails
 * ------------------------------------------------------------------------
 * General: Initializes an address structure
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pAddr - The address to initialize
 ***************************************************************************/
static void RVCALLCONV InitAddressDetails(IN  RvSipTransportAddr   *pAddr)
{

	memset(pAddr, 0, sizeof(RvSipTransportAddr));
	pAddr->eTransportType = RVSIP_TRANSPORT_UNDEFINED;
	pAddr->eAddrType      = RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED;
	pAddr->Ipv6Scope      = UNDEFINED;
}
#endif /* (RV_IMS_IPSEC_ENABLED == RV_YES) */

#endif /* #ifdef RV_SIP_IMS_ON */

