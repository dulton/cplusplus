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
 *                              <_SipSecAgreeMgr.c>
 *
 * This file defines the internal API of the security-agreement manager object.
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

#include "SecAgreeTypes.h"
#include "_SipSecAgreeMgr.h"
#include "SecAgreeMsgHandling.h"
#include "SecAgreeObject.h"
#include "SecAgreeClient.h"
#include "SecAgreeServer.h"
#include "SecAgreeCallbacks.h"
#include "AdsRlist.h"
#include "AdsHash.h"
#include "rvansi.h"
#include "rvmemory.h"
#include "_SipCommonUtils.h"
#include "_SipCommonConversions.h"
#include "_SipSecurityMgr.h"

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV SecAgreeMgrInitialize(
							IN     SipSecAgreeMgrCfg*    pConfiguration,
							INOUT  SecAgreeMgr*          pSecAgreeMgr);

static RvStatus RVCALLCONV SecAgreeMgrSetSecEvHandlers(
								IN   SecAgreeMgr*          pSecAgreeMgr);

static RvStatus RVCALLCONV SecAgreePoolListAllocation(
							IN   SecAgreeMgr*               pSecAgreeMgr,
							IN   RvInt32                    numOfElements);

static void RVCALLCONV SecAgreePoolListDestruction(SecAgreeMgr  *pSecAgreeMgr);

static RvUint OwnersHashFunction(IN void *key);

static RvStatus RVCALLCONV AddNewArrayEntry(
								  IN     SecAgreeMgr*                          pSecAgreeMgr,
								  IN     SipSecAgreeEvHandlers*				   pEntry,
								  IN     RvInt32                               index,
								  INOUT  SipSecAgreeEvHandlers**			   pArray,
								  INOUT  RvInt32*							   pArraySize);

static RvStatus RVCALLCONV SecAgreeInitiateTripleLock(IN SecAgree  *pSecAgree);

static void RVCALLCONV SecAgreeDestructTripleLock(IN SecAgree  *pSecAgree);


/*-----------------------------------------------------------------------*/
/*                         MANAGER FUNCTIONS                             */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SipSecAgreeMgrConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes the security-manager object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pConfiguration   - A structure off initialized configuration parameters.
 * Output:  phSecAgreeMgr    - The newly created Security-Mgr
 ***************************************************************************/
RvStatus RVCALLCONV SipSecAgreeMgrConstruct(
							IN   SipSecAgreeMgrCfg*           pConfiguration,
							OUT  RvSipSecAgreeMgrHandle*      phSecAgreeMgr)
{
	RvStatus            rv;
    RvLogSource        *pLogSrc;
    SecAgreeMgr        *pSecAgreeMgr = NULL;
	
    *phSecAgreeMgr = NULL;

    pLogSrc = pConfiguration->pLogSrc;

    /*allocate the security-manager*/
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(SecAgreeMgr), pConfiguration->pLogMgr, (void*)&pSecAgreeMgr))
    {
        RvLogError(pLogSrc ,(pLogSrc ,
                   "SipSecAgreeMgrConstruct - Error in constructing the security manager"));
        return RV_ERROR_OUTOFRESOURCES;
    }
    
	/* Initialize manager */
	rv = SecAgreeMgrInitialize(pConfiguration, pSecAgreeMgr);
    if(rv != RV_OK)
    {
        RvLogError(pLogSrc ,(pLogSrc ,
                   "SipSecAgreeMgrConstruct - Failed to construct the security manager - freeing resources"));
        RvMemoryFree((void*)pSecAgreeMgr, pSecAgreeMgr->pLogMgr);
        return rv;
    }
	
	/* Set event handlers to the security manager */
    if (NULL != pSecAgreeMgr->hSecMgr)
    {
        rv = SecAgreeMgrSetSecEvHandlers(pSecAgreeMgr);
        if(rv != RV_OK)
        {
            RvLogError(pLogSrc ,(pLogSrc ,
                "SipSecAgreeMgrConstruct - Failed to set event handlers to the security manager - destructing security-agreement manager"));
            SipSecAgreeMgrDestruct((RvSipSecAgreeMgrHandle)pSecAgreeMgr);
            return rv;
        }
    }

    *phSecAgreeMgr = (RvSipSecAgreeMgrHandle)pSecAgreeMgr;

    RvLogInfo(pLogSrc ,(pLogSrc ,
             "SipSecAgreeMgrConstruct - Security manager was constructed successfully"));
    return RV_OK;
}

/***************************************************************************
 * SipSecAgreeMgrDestruct
 * ------------------------------------------------------------------------
 * General: Deallocates all resources held by the security-manager
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgreeMgr - Handle to the Security-Mgr to terminate.
 ***************************************************************************/
void RVCALLCONV SipSecAgreeMgrDestruct(
                                 IN RvSipSecAgreeMgrHandle      hSecAgreeMgr)
{
	SecAgreeMgr  *pSecAgreeMgr = (SecAgreeMgr*)hSecAgreeMgr;
	
	/* lock mutex */
	RvMutexLock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);
	
	if (NULL != pSecAgreeMgr->pEvHandlers)
	{
		/* Free event handlers array */
		RvMemoryFree((void*)pSecAgreeMgr->pEvHandlers, pSecAgreeMgr->pLogMgr);
	}
	if (NULL != pSecAgreeMgr->hSecAgreePool)
	{
		/* Free security-agreement pool */
		SecAgreePoolListDestruction(pSecAgreeMgr);
	}
	if (NULL != pSecAgreeMgr->hOwnersHash)
	{
		/* Free owners hash */
		HASH_Destruct(pSecAgreeMgr->hOwnersHash);
	}

	/* unlock mutex */
	RvMutexUnlock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);
	/* destruct mutex */
	RvMutexDestruct(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);
		
	RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
			  "SipSecAgreeMgrDestruct - Security-agreement manager was destructed successfully"));

	/* Free manager object */
	RvMemoryFree((void*)pSecAgreeMgr, pSecAgreeMgr->pLogMgr);
}

/***************************************************************************
 * SipSecAgreeMgrHandleMsgRcvd
 * ------------------------------------------------------------------------
 * General: Processes a received message : modifies the state of the current 
 *          security-agreement according to the received message, calls
 *          SecAgreeRequiredEv if necessary, and loads any additional security
 *          agreement information.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgreeMgr    - Handle to the Security-Mgr
 *         hSecAgree       - Handle to the current security-agreement
 *         pOwner          - Pointer to the owner that received the message
 *         eOwnerType      - The type of the supplied owner
 *         pOwnerLock      - The triple lock of the owner, to open before callback
 *         hMsg            - Handle to the message
 *         hLocalAddr      - The address that the message was received on. Required
 *                           only for requests.
 *         pRcvdFromAddr   - The address the message was received from. Required
 *                           only for requests.
 *         hRcvdOnConn     - The connection the message was received on. Required
 *                           only for requests.
 ***************************************************************************/
RvStatus RVCALLCONV SipSecAgreeMgrHandleMsgRcvd(
							 IN  RvSipSecAgreeMgrHandle          hSecAgreeMgr,
							 IN  RvSipSecAgreeHandle		     hSecAgree,
							 IN  void                           *pOwner,
							 IN  RvSipCommonStackObjectType      eOwnerType,
							 IN  SipTripleLock                  *pOwnerLock,
							 IN  RvSipMsgHandle                  hMsg,
							 IN  RvSipTransportLocalAddrHandle   hLocalAddr,
							 IN  SipTransportAddress            *pRcvdFromAddr,
							 IN	 RvSipTransportConnectionHandle  hRcvdOnConn)
{
	SecAgreeMgr        *pSecAgreeMgr    = (SecAgreeMgr*)hSecAgreeMgr;
	SecAgree           *pSecAgree       = (SecAgree*)hSecAgree;
	RvSipSecAgreeOwner *pOwnerToUse;
	RvSipSecAgreeOwner  ownerToUse;
	RvStatus            rv              = RV_OK;

	if (NULL == hSecAgreeMgr && NULL == hSecAgree)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* We lock only if the object isn't null. 
	   If it is null, a new object may be locked within HandleMsgRcvd() */ 
	if (pSecAgree != NULL)
	{
		/* lock the security-agreement */
		rv = SecAgreeLockAPI(pSecAgree);
		if(rv != RV_OK)
		{
			return RV_ERROR_INVALID_HANDLE;
		}
	}

	/* update manager pointer from security-agreement if needed */
	if (hSecAgreeMgr == NULL)
	{
		pSecAgreeMgr = pSecAgree->pSecAgreeMgr;
	}

	/* check owner information */
	if (pOwner != NULL)
	{
		/* Insert owner information to owner structure */
		ownerToUse.pOwner     = pOwner;
		ownerToUse.eOwnerType = eOwnerType;
		pOwnerToUse           = &ownerToUse;
	}
	else
	{
		/* No owner supplied */
		ownerToUse.pOwner     = NULL;
		ownerToUse.eOwnerType = RVSIP_COMMON_STACK_OBJECT_TYPE_UNDEFINED;
		pOwnerToUse           = &ownerToUse;
	}

	if (RVSIP_MSG_RESPONSE == RvSipMsgGetMsgType(hMsg))
	{
		if (pSecAgree == NULL ||
			pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_CLIENT)
		{
			rv = SecAgreeClientHandleMsgRcvd(pSecAgreeMgr, pOwnerToUse, pOwnerLock, hMsg, &pSecAgree);
		}
	}
	else /* response message */
	{
		if (pSecAgree == NULL ||
			pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_SERVER)
		{
			rv = SecAgreeServerHandleMsgRcvd(pSecAgreeMgr, pOwnerToUse, pOwnerLock, hMsg, hLocalAddr, pRcvdFromAddr, hRcvdOnConn, &pSecAgree);
		}
	}

	/* We lock only if the object isn't null. 
	   If it is NULL, the old security-agreement was freed within HandleMsgRcvd() */
	if (NULL != pSecAgree)
	{
		SecAgreeUnLockAPI(pSecAgree);
	}

	return rv;
}

/***************************************************************************
 * SipSecAgreeMgrGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: Returns a structure with the status of all resources used by
 *          the securit-agreement module, i.e., the security-agreement list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgreeMgr - Handle to the security-agreement manager.
 * Output:    pResources   - Pointer to a security-agreement resource structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipSecAgreeMgrGetResourcesStatus(
                                 IN  RvSipSecAgreeMgrHandle   hSecAgreeMgr,
                                 OUT RvSipSecAgreeResources  *pResources) 
{

    SecAgreeMgr		   *pSecAgreeMgr = (SecAgreeMgr*)hSecAgreeMgr;

    RvUint32			allocNumOfLists;
    RvUint32			allocMaxNumOfUserElement;
    RvUint32			currNumOfUsedLists;
    RvUint32			currNumOfUsedUserElement;
    RvUint32			maxUsageInLists;
    RvUint32			maxUsageInUserElement;
	RV_HASH_Resource	ownersHash;

	RvStatus			rv = RV_OK;

	if (hSecAgreeMgr == NULL)
	{
		return RV_OK;
	}
	
	if(pResources == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    memset(pResources,0,sizeof(RvSipSecAgreeResources));
    
	/* lock mutex */
	RvMutexLock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);
	
	/* Security-agreements list*/
    if(pSecAgreeMgr->hSecAgreePool != NULL)
    {
        RLIST_GetResourcesStatus(pSecAgreeMgr->hSecAgreePool,
								&allocNumOfLists, /* always 1*/
								&allocMaxNumOfUserElement,
								&currNumOfUsedLists,/* always 1*/
								&currNumOfUsedUserElement,
								&maxUsageInLists,
								&maxUsageInUserElement);

        pResources->secAgrees.currNumOfUsedElements  = currNumOfUsedUserElement;
        pResources->secAgrees.numOfAllocatedElements = allocMaxNumOfUserElement;
        pResources->secAgrees.maxUsageOfElements     = maxUsageInUserElement;
    }
	
	/* Owners hash */
	if (pSecAgreeMgr->hOwnersHash != NULL)
	{
		rv = HASH_GetResourcesStatus(pSecAgreeMgr->hOwnersHash, &ownersHash);
        pResources->ownersHash.currNumOfUsedElements  = ownersHash.elements.numOfUsed;
        pResources->ownersHash.numOfAllocatedElements = ownersHash.elements.maxNumOfElements;
        pResources->ownersHash.maxUsageOfElements     = ownersHash.elements.maxUsage;
	}

	/* Owners list*/
    if(pSecAgreeMgr->hSecAgreeOwnersPool != NULL)
    {
        RLIST_GetResourcesStatus(pSecAgreeMgr->hSecAgreeOwnersPool,
								&allocNumOfLists, 
								&allocMaxNumOfUserElement,
								&currNumOfUsedLists,
								&currNumOfUsedUserElement,
								&maxUsageInLists,
								&maxUsageInUserElement);

		pResources->ownersLists.numOfAllocatedElements = allocNumOfLists;
        pResources->ownersLists.currNumOfUsedElements = currNumOfUsedLists;
        pResources->ownersLists.maxUsageOfElements = maxUsageInLists;
        pResources->ownersHandles.numOfAllocatedElements = allocMaxNumOfUserElement;
        pResources->ownersHandles.currNumOfUsedElements = currNumOfUsedUserElement;
        pResources->ownersHandles.maxUsageOfElements = maxUsageInUserElement;
    }

    /* unlock mutex */
	RvMutexUnlock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);

    return rv;
}

/***************************************************************************
 * SipSecAgreeMgrResetMaxUsageResourcesStatus
 * ------------------------------------------------------------------------
 * General: Resets the counts of max number of security-agreements used 
 *          simultaneously so far.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hSecAgreeMgr - The security-agreement manager.
 ***************************************************************************/
void RVCALLCONV SipSecAgreeMgrResetMaxUsageResourcesStatus (
                                 IN  RvSipSecAgreeMgrHandle  hSecAgreeMgr)
{
    SecAgreeMgr *pSecAgreeMgr = (SecAgreeMgr*)hSecAgreeMgr;

	/* There is no need to lock: called once by the stack manager */

    RLIST_ResetMaxUsageResourcesStatus(pSecAgreeMgr->hSecAgreePool);
}


/***************************************************************************
 * SipSecAgreeMgrSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets an event handlers structure to the security-manager object.
 *          The security-manager stores them according to the type of owner.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgreeMgr    - Handle to the Security-Mgr
 *          pEvHandlers     - The structure of even handlers
 *          eOwnerType      - The type of owner setting the event handlers
 *                            (there is a different set of handlers for each
 *                             type of owner).
 ***************************************************************************/
RvStatus RVCALLCONV SipSecAgreeMgrSetEvHandlers(
							IN  RvSipSecAgreeMgrHandle           hSecAgreeMgr,
							IN  SipSecAgreeEvHandlers			*pEvHandlers,
							IN  RvSipCommonStackObjectType       eOwnerType)
{
	SecAgreeMgr *pSecAgreeMgr = (SecAgreeMgr*)hSecAgreeMgr;
	RvStatus     rv;

	if (NULL == hSecAgreeMgr)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (NULL == pEvHandlers)
	{
		return RV_ERROR_BADPARAM;
	}

	/* There is no need to lock: called once by each module in stack initialization */

	RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
			  "SipSecAgreeMgrSetEvHandlers - %s setting event handlers to security-agreement manager",
			  SipCommonObjectType2Str(eOwnerType)));

	rv = AddNewArrayEntry(pSecAgreeMgr,
						  pEvHandlers, (RvInt32)eOwnerType,
						  &pSecAgreeMgr->pEvHandlers, &pSecAgreeMgr->evHandlersSize);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc, (pSecAgreeMgr->pLogSrc,
					"SipSecAgreeMgrSetEvHandlers - Failed to set event handlers"));
	}

	return rv;
}

/***************************************************************************
 * SipSecAgreeMgrIsInvalidRequest
 * ------------------------------------------------------------------------
 * General: Checks whether the received request contains Require or Proxy-Require
 *          with sec-agree, but more than one Via. This message must be rejected
 *          with 502 (Bad Gateway).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgreeMgr	 - Handle to the security-agreement manager
 *         hMsg          - Handle to the message
 * Output: pResponseCode - 502 if this is invalid response. 0 otherwise.
 ***************************************************************************/
RvStatus RVCALLCONV SipSecAgreeMgrIsInvalidRequest(
							 IN  RvSipSecAgreeMgrHandle   hSecAgreeMgr,
							 IN  RvSipMsgHandle           hMsg,
							 OUT RvUint16*                pResponseCode)
{
	SecAgreeMgr* pSecAgreeMgr = (SecAgreeMgr*)hSecAgreeMgr;
	RvBool       bHasRequired;
	RvBool       bIsInvalid;
	RvStatus     rv;
	
	*pResponseCode = 0;

	/* There is no need to lock : we do not use shared resources of the manager
      except the log */

	rv = SecAgreeMsgHandlingServerIsInvalidRequest(pSecAgreeMgr, hMsg, &bIsInvalid, &bHasRequired);
    if (RV_OK != rv)
    {
        return rv;
    }

	if (bIsInvalid == RV_TRUE)
	{
		*pResponseCode = RESPONSE_502;
	}

	return RV_OK;
}

/*------------------------------------------------------------------------
           S T A T I C   F U N C T I O N S
 -------------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeMgrInitialize
 * ------------------------------------------------------------------------
 * General: Initializes the security-manager object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pConfiguration  - A structure of initialized configuration parameters.
 * Inout:   pSecAgreeMgr    - The newly created Security-Mgr
 ***************************************************************************/
static RvStatus RVCALLCONV SecAgreeMgrInitialize(
							IN     SipSecAgreeMgrCfg*    pConfiguration,
							INOUT  SecAgreeMgr*          pSecAgreeMgr)
{
	RvStatus    crv;
    RvStatus    rv;

	/* Initializing the struct members to NULL (0) */
    memset(pSecAgreeMgr, 0, sizeof(SecAgreeMgr));

	pSecAgreeMgr->hSipStack      = pConfiguration->hSipStack;
	pSecAgreeMgr->pLogMgr        = pConfiguration->pLogMgr;
	pSecAgreeMgr->pLogSrc        = pConfiguration->pLogSrc;
	pSecAgreeMgr->hMemoryPool    = pConfiguration->hMemoryPool;
	pSecAgreeMgr->hTransport     = pConfiguration->hTransport;
	pSecAgreeMgr->hSecMgr        = pConfiguration->hSecMgr;
	pSecAgreeMgr->hAuthenticator = pConfiguration->hAuthenticator;
	pSecAgreeMgr->hMsgMgr        = pConfiguration->hMsgMgr;
	pSecAgreeMgr->seed           = pConfiguration->seed;
	pSecAgreeMgr->pEvHandlers    = NULL;
	pSecAgreeMgr->maxNumOfOwners = pConfiguration->maxSecAgreeOwners;
	pSecAgreeMgr->ownersPerSecAgree         = pConfiguration->maxOwnersPerSecAgree;

	/* Construct mutex */
    crv = RvMutexConstruct(pSecAgreeMgr->pLogMgr, &pSecAgreeMgr->hMutex);
    if(crv != RV_OK)
    {
		return CRV2RV(crv);
    }

	/* Allocate security-agreement list */
	rv = SecAgreePoolListAllocation(pSecAgreeMgr,
									pConfiguration->maxSecAgrees);
	if (RV_OK != rv)
	{	
		/* destruct mutex */
		RvMutexDestruct(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);
		return rv;
	}

	/* Allocate owners pool */
    pSecAgreeMgr->hSecAgreeOwnersPool = RLIST_PoolListConstruct(
											pSecAgreeMgr->ownersPerSecAgree * pConfiguration->maxSecAgrees, 
											2 * pConfiguration->maxSecAgrees, /* We multiply by 2 because of the temporary list we need in order to set owners to sec-obj */
											sizeof(SecAgreeOwnerRecord),
											pSecAgreeMgr->pLogMgr, 
											"Security-Agreements Owners Pool");

    if (NULL == pSecAgreeMgr->hSecAgreePool)
    {
        RvLogError(pSecAgreeMgr->pLogSrc ,(pSecAgreeMgr->pLogSrc ,
                   "SecAgreePoolListAllocation - Failed to construct security-agreement owners pool"));
        return RV_ERROR_OUTOFRESOURCES;
    }

	/* Allocate owners hash */
	pSecAgreeMgr->ownersHashSize = HASH_DEFAULT_TABLE_SIZE((pSecAgreeMgr->maxNumOfOwners*2));
    pSecAgreeMgr->hOwnersHash = HASH_Construct(
							pSecAgreeMgr->ownersHashSize,
							pSecAgreeMgr->maxNumOfOwners,
							OwnersHashFunction,
							sizeof(SecAgreeOwnerRecord*),
							sizeof(SecAgreeOwnerHashKey),
							pSecAgreeMgr->pLogMgr,
							"Security-Agreement Owners Hash");
    if (NULL == pSecAgreeMgr->hOwnersHash)
    {
		/* destruct mutex */
		RvMutexDestruct(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);
		/* Free security-agreement pool */
		SecAgreePoolListDestruction(pSecAgreeMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }

	return RV_OK;
}

/***************************************************************************
 * SecAgreeMgrSetSecEvHandlers
 * ------------------------------------------------------------------------
 * General: Set event handlers to the security module
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecAgreeMgr - Pointer to the security-agreement manager
 ***************************************************************************/
static RvStatus RVCALLCONV SecAgreeMgrSetSecEvHandlers(
								IN   SecAgreeMgr*          pSecAgreeMgr)
{
	RvSipSecEvHandlers  secEvHandlers;

	memset(&secEvHandlers, 0, sizeof(RvSipSecEvHandlers));
	secEvHandlers.pfnSecObjStateChangedEvHandler = SecAgreeCallbacksSecObjStateChangedEvHandler;
	secEvHandlers.pfnSecObjStatusEvHandler = SecAgreeCallbacksSecObjStatusEvHandler;
	return SipSecMgrSetEvHandlers(pSecAgreeMgr->hSecMgr, &secEvHandlers);
}

/***************************************************************************
 * SecAgreePoolListAllocation
 * ------------------------------------------------------------------------
 * General: Allocates a the list of security-agreement objects
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pSecAgreeMgr  - Handle to the security manager.
 *           numOfElements - The number of security-agreement objects to allocate.
 ***************************************************************************/
static RvStatus RVCALLCONV SecAgreePoolListAllocation(
							IN   SecAgreeMgr*               pSecAgreeMgr,
							IN   RvInt32                    numOfElements)
{
	SecAgree  *pSecAgree;
	RvInt32    i;
	RvStatus   rv;
    
	/* Allocate pool */
    pSecAgreeMgr->hSecAgreePool = RLIST_PoolListConstruct(numOfElements, 1, sizeof(SecAgree),
                                      pSecAgreeMgr->pLogMgr, "Security-Agreements List");

    if (NULL == pSecAgreeMgr->hSecAgreePool)
    {
        RvLogError(pSecAgreeMgr->pLogSrc ,(pSecAgreeMgr->pLogSrc ,
                   "SecAgreePoolListAllocation - Failed to construct security-agreement pool"));
        return RV_ERROR_OUTOFRESOURCES;
    }

	/* Allocate a single list within pool */
    pSecAgreeMgr->hSecAgreeList = RLIST_ListConstruct(pSecAgreeMgr->hSecAgreePool);
    if (NULL == pSecAgreeMgr->hSecAgreeList)
    {
        RvLogError(pSecAgreeMgr->pLogSrc ,(pSecAgreeMgr->pLogSrc ,
                   "SecAgreePoolListAllocation - Failed to construct security-agreement list"));
        RLIST_PoolListDestruct(pSecAgreeMgr->hSecAgreePool);
        pSecAgreeMgr->hSecAgreePool = NULL;
        return RV_ERROR_OUTOFRESOURCES;
    }

	
    /* initiate locks of all security-agreements */
    for (i = 0; i < numOfElements; i++)
    {
        RLIST_InsertTail(pSecAgreeMgr->hSecAgreePool, pSecAgreeMgr->hSecAgreeList, (RLIST_ITEM_HANDLE *)&pSecAgree);
        pSecAgree->pSecAgreeMgr = pSecAgreeMgr;
        rv = SecAgreeInitiateTripleLock(pSecAgree);
        if(rv != RV_OK)
        {
            RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
                      "SecAgreePoolListAllocation - Failed to initiate triple locks in SecAgreeInitiateTripleLock (rv=%d)", rv));
            SecAgreePoolListDestruction(pSecAgreeMgr);
			pSecAgreeMgr->hSecAgreePool = NULL;
			pSecAgreeMgr->hSecAgreeList = NULL;
            return rv;
        }
    }

    /* release all security-agreements that were temporarily allocated for lock construction */
    for (i = 0; i < numOfElements; i++)
    {
        RLIST_GetHead(pSecAgreeMgr->hSecAgreePool, pSecAgreeMgr->hSecAgreeList,(RLIST_ITEM_HANDLE *)&pSecAgree);
        RLIST_Remove(pSecAgreeMgr->hSecAgreePool, pSecAgreeMgr->hSecAgreeList,(RLIST_ITEM_HANDLE)pSecAgree);
    }

	return RV_OK;
}

/***************************************************************************
 * SecAgreePoolListDestruction
 * ------------------------------------------------------------------------
 * General: Destructs the list of security-agreement objects.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgreeMgr  - Handle to the security manager.
 ***************************************************************************/
static void RVCALLCONV SecAgreePoolListDestruction(SecAgreeMgr  *pSecAgreeMgr)
{
    RvUint32   i;
    SecAgree  *pSecAgree;
    RvUint32   allocNumOfLists = 0;
    RvUint32   allocMaxNumOfUserElement = 0;
    RvUint32   currNumOfUsedLists = 0;
    RvUint32   currNumOfUsedUsreElement = 0;
    RvUint32   maxUsageInLists = 0;
    RvUint32   maxUsageInUserElement = 0;

    RLIST_GetResourcesStatus(pSecAgreeMgr->hSecAgreePool,
                             &allocNumOfLists,    &allocMaxNumOfUserElement,
                             &currNumOfUsedLists, &currNumOfUsedUsreElement,
                             &maxUsageInLists,    &maxUsageInUserElement);

    for (i = 0; i < allocMaxNumOfUserElement; i++)
    {
        RLIST_GetHead(pSecAgreeMgr->hSecAgreePool, pSecAgreeMgr->hSecAgreeList,(RLIST_ITEM_HANDLE *)&pSecAgree);
        if (pSecAgree == NULL)
            break;
        RLIST_Remove(pSecAgreeMgr->hSecAgreePool, pSecAgreeMgr->hSecAgreeList, (RLIST_ITEM_HANDLE)pSecAgree);
    }

    for (i = 0; i < allocMaxNumOfUserElement; i++)
    {
        RLIST_InsertTail(pSecAgreeMgr->hSecAgreePool, pSecAgreeMgr->hSecAgreeList,(RLIST_ITEM_HANDLE *)&pSecAgree);
    }

    for (i = 0; i < allocMaxNumOfUserElement; i++)
    {
        RLIST_GetHead(pSecAgreeMgr->hSecAgreePool, pSecAgreeMgr->hSecAgreeList,(RLIST_ITEM_HANDLE *)&pSecAgree);
        SecAgreeDestructTripleLock(pSecAgree);
        RLIST_Remove(pSecAgreeMgr->hSecAgreePool, pSecAgreeMgr->hSecAgreeList, (RLIST_ITEM_HANDLE)pSecAgree);
    }

    RLIST_PoolListDestruct(pSecAgreeMgr->hSecAgreePool);
    pSecAgreeMgr->hSecAgreePool = NULL;
}


/*-------------------- OWNERS HASH FUNCTIONS ----------------------*/

/***************************************************************************
 * OwnersHashFunction
 * ------------------------------------------------------------------------
 * General: call back function for calculating the hash value from the
 *          hash key.
 * Return Value: The value generated from the key.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     key  - The hash key (of type SecAgreeOwnerHashKey)
 ***************************************************************************/
static RvUint OwnersHashFunction(IN void *key)
{
    SecAgreeOwnerHashKey *hashElem = (SecAgreeOwnerHashKey *)key;
    RvInt32               i;
    RvUint                s = 0;
    RvInt32               mod = hashElem->pSecAgree->pSecAgreeMgr->ownersHashSize;
    static int            base = 1 << (sizeof(char)*8);
    RvChar                buffer[64];
    RvInt32               buffSize;

	RvSprintf(buffer,"%p%p",hashElem->hOwner,hashElem->pSecAgree);
    buffSize = (RvInt32)strlen(buffer);
    for (i=0; i<buffSize; i++)
    {
        s = (s*base+buffer[i])%mod;
    }
    return s;
}

/***************************************************************************
 * AddNewArrayEntry
 * ------------------------------------------------------------------------
 * General: Sets a new entry to the array of event handlers. If the index
 *          exceeds the size of the current array, the array is doubled and all
 *          previous data is copied to the new array.
 *          Notice: we do this so that new modules that are added to the stack
 *          will not have to add code to the security layer. The dynamic
 *          array will double itself automatically.
 * Return Value: String owner type.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr - Pointer to the security manager
 *         pEntry       - The entry to set
 *         index	    - The index within the array
 * Inout:  pArray	    - The array. If doubled, a new pointer is returned
 *         pArraySize   - The size of the array. If doubled, the size is recalculated
 ***************************************************************************/
static RvStatus RVCALLCONV AddNewArrayEntry(
								  IN     SecAgreeMgr*                          pSecAgreeMgr,
								  IN     SipSecAgreeEvHandlers*			       pEntry,
								  IN     RvInt32                               index,
								  INOUT  SipSecAgreeEvHandlers**			   pArray,
								  INOUT  RvInt32*							   pArraySize)
{
	SipSecAgreeEvHandlers*			 pNewArray;
	RvInt32                          newLen;
	RvInt32                          i;
	RvInt32                          initialLen = 6;

	if (NULL == *pArray)
	{
		/*allocate the new array*/
		if (RV_OK != RvMemoryAlloc(NULL, initialLen*sizeof(SipSecAgreeEvHandlers), pSecAgreeMgr->pLogMgr, (void*)pArray))
		{
			RvLogError(pSecAgreeMgr->pLogSrc, (pSecAgreeMgr->pLogSrc,
						"AddNewArrayEntry - Error in allocating event handlers array"));
			return RV_ERROR_OUTOFRESOURCES;
		}
		memset(*pArray,0,initialLen*sizeof(SipSecAgreeEvHandlers));

		*pArraySize = initialLen;
	}
	else if (index >= *pArraySize)
	{
		/* the array needs doubling */
		newLen = RvMax(*pArraySize,index)*2;

		/*allocate the new array*/
		if (RV_OK != RvMemoryAlloc(NULL, newLen*sizeof(SipSecAgreeEvHandlers), pSecAgreeMgr->pLogMgr, (void*)&pNewArray))
		{
			RvLogError(pSecAgreeMgr->pLogSrc, (pSecAgreeMgr->pLogSrc,
						"AddNewArrayEntry - Error in allocating event handlers array"));
			return RV_ERROR_OUTOFRESOURCES;
		}
		memset(pNewArray,0,newLen*sizeof(SipSecAgreeEvHandlers));

		/* copy old array into the new one */
		for (i = 0; i < *pArraySize; i++)
		{
			pNewArray[i] = (*pArray)[i];
		}

		/* free old array */
		RvMemoryFree((void*)*pArray, pSecAgreeMgr->pLogMgr);

		/* use new array */
		*pArray     = pNewArray;
		*pArraySize = newLen;
	}

	/* Set the new entry to the array */
	(*pArray)[index] = *pEntry;

	return RV_OK;
}

/*----------------------- TRIPLE LOCK FUNCTIONS --------------------------*/

/***************************************************************************
 * SecAgreeInitiateTripleLock
 * ------------------------------------------------------------------------
 * General: Allocates & initiates security-agreement triple lock
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree   - Pointer to security-agreement object
 **************************************************************************/
static RvStatus RVCALLCONV SecAgreeInitiateTripleLock(IN SecAgree  *pSecAgree)
{
    RvStatus        crv = RV_OK;

    pSecAgree->secAgreeTripleLock.bIsInitialized = RV_FALSE;
    crv = RvMutexConstruct(pSecAgree->pSecAgreeMgr->pLogMgr, &pSecAgree->secAgreeTripleLock.hLock);
    if(crv != RV_OK)
    {
        return CRV2RV(crv);
    }

    crv = RvMutexConstruct(pSecAgree->pSecAgreeMgr->pLogMgr, &pSecAgree->secAgreeTripleLock.hProcessLock);
    if(crv != RV_OK)
    {
        RvMutexDestruct(&pSecAgree->secAgreeTripleLock.hLock,pSecAgree->pSecAgreeMgr->pLogMgr);
        return CRV2RV(crv);
    }

    crv = RvSemaphoreConstruct(1, pSecAgree->pSecAgreeMgr->pLogMgr, &pSecAgree->secAgreeTripleLock.hApiLock);
    if(crv != RV_OK)
    {
        RvMutexDestruct(&pSecAgree->secAgreeTripleLock.hLock,pSecAgree->pSecAgreeMgr->pLogMgr);
        RvMutexDestruct(&pSecAgree->secAgreeTripleLock.hProcessLock,pSecAgree->pSecAgreeMgr->pLogMgr);
        return CRV2RV(crv);
    }
    pSecAgree->secAgreeTripleLock.apiCnt            = 0;
    pSecAgree->secAgreeTripleLock.objLockThreadId   = 0;
    pSecAgree->secAgreeTripleLock.threadIdLockCount = UNDEFINED;
    pSecAgree->secAgreeTripleLock.bIsInitialized    = RV_TRUE;
    pSecAgree->tripleLock                           = NULL;

    return RV_OK;
}

/***************************************************************************
 * SecAgreeDestructTripleLock
 * ------------------------------------------------------------------------
 * General: Destructs pre-allocated security-agreement triple lock
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree   - pointer to security-agreement
 **************************************************************************/
static void RVCALLCONV SecAgreeDestructTripleLock(IN SecAgree  *pSecAgree)
{
    if(pSecAgree->secAgreeTripleLock.bIsInitialized == RV_TRUE)
    {
        RvMutexDestruct(&pSecAgree->secAgreeTripleLock.hLock,pSecAgree->pSecAgreeMgr->pLogMgr);
        RvMutexDestruct(&pSecAgree->secAgreeTripleLock.hProcessLock, pSecAgree->pSecAgreeMgr->pLogMgr);
        RvSemaphoreDestruct(&pSecAgree->secAgreeTripleLock.hApiLock, pSecAgree->pSecAgreeMgr->pLogMgr);
        pSecAgree->secAgreeTripleLock.bIsInitialized = RV_FALSE;
    }
}

#endif /* #ifdef RV_SIP_IMS_ON */



