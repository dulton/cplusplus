/*
*******************************************************************************
*                                                                             *
* NOTICE:                                                                     *
* This document contains information that is confidential and proprietary to  *
* RADVision LTD.. No part of this publication may be reproduced in any form   *
* whatsoever without written prior approval by RADVision LTD..                *
*                                                                             *
* RADVision LTD. reserves the right to revise this publication and make       *
* changes without obligation to notify any person of such revisions or changes*
*******************************************************************************
*/

/******************************************************************************
 *                              <_SipSecurityMgr.c>
 *
 * The _SipSecurityMgr.h file contains Internal API functions of the Security
 * Module, including the module construct and destruct and resource status API.
 *
 *
 *    Author                        Date
 *    ------------                  ------------
 *    Igor                          Februar 2006
 ** **************************************************************************/
/*---------------------------------------------------------------------------*/
/*                            INCLUDE HEADER FILES                           */
/*---------------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#include "_SipCommonCore.h"
#include "_SipCommonTypes.h"
#include "_SipCommonConversions.h"
#include "_SipTransport.h"

#ifdef RV_SIP_IMS_ON

#include "_SipSecurityMgr.h"
#include "_SipSecuritySecObj.h"
#include "SecurityMgrObject.h"
#include "SecurityObject.h"

#include "RvSipResourcesTypes.h"

/*---------------------------------------------------------------------------*/
/*                             TYPE DEFINITIONS                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                             MODULE VARIABLES                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                          STATIC FUNCTIONS PROTOTYPES                      */
/*---------------------------------------------------------------------------*/
static RvStatus ConstructLocksInSecObjPool(SecMgr* pSecMgr);

static void DestructLocksInSecObjPool(SecMgr* pSecMgr);

/*---------------------------------------------------------------------------*/
/*                             MODULE FUNCTIONS                              */
/*---------------------------------------------------------------------------*/
/******************************************************************************
 * SipSecurityMgrConstruct
 * ----------------------------------------------------------------------------
 * General: Construct a new Security manager.
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pCfg          - Handle to the configuration
 * Output:  phSecurityMgr - Handle to the created Security Manager.
 *****************************************************************************/
RvStatus SipSecurityMgrConstruct(
                                IN  SipSecurityMgrCfg* pCfg,
                                OUT RvSipSecMgrHandle* phSecMgr)
{
    RvStatus rv, crv;
    SecMgr*  pSecMgr;

    *phSecMgr = NULL;

    /* Allocate manager */
    if (RV_OK != RvMemoryAlloc(NULL,sizeof(SecMgr),pCfg->pLogMgr,(void*)&pSecMgr))
    {
        RvLogError(pCfg->pLogSrc,(pCfg->pLogSrc,
            "SipSecurityMgrConstruct - Failed to allocate %d bytes",sizeof(SecMgr)));
        return RV_ERROR_UNKNOWN;
    }
    memset(pSecMgr, 0, sizeof(SecMgr));

    /* Initialize manager */
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    pSecMgr->getAlternativeSecObjFunc = pCfg->getAlternativeSecObjFunc;
#endif
//SPIRENT_END
    pSecMgr->pLogMgr = pCfg->pLogMgr;
    pSecMgr->pLogSrc = pCfg->pLogSrc;
    pSecMgr->hStack  = pCfg->hStack;
    pSecMgr->hMsgMgr = pCfg->hMsgMgr;
    pSecMgr->hTransportMgr    = pCfg->hTransportMgr;
    pSecMgr->bTcpEnabled      = pCfg->bTcpEnabled;
    pSecMgr->pSeed            = pCfg->pSeed;
    pSecMgr->maxSecObjects    = pCfg->maxNumOfSecObj;
    pSecMgr->maxIpsecSessions = pCfg->maxNumOfIpsecSessions;
    pSecMgr->spiRangeStart    = pCfg->spiRangeStart;
    pSecMgr->spiRangeEnd      = pCfg->spiRangeEnd;

    /* Get Select Engine. The Select Engine is used for timers. */
    crv = RvSelectGetThreadEngine(pSecMgr->pLogMgr,&pSecMgr->pSelectEngine);
    if (RV_OK != crv)
    {
        RvMemoryFree((void*)pSecMgr, pSecMgr->pLogMgr);
        RvLogError(pCfg->pLogSrc,(pCfg->pLogSrc,
            "SipSecurityMgrConstruct - Failed to get Select Enginge (crv=%d)",
            RvErrorGetCode(crv)));
        return CRV2RV(crv);
    }

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    if (pSecMgr->maxIpsecSessions > 0)
    {
        /* Construct structures stored in Manager for usage by Common Core */
        crv = rvImsConstructCfg(&pSecMgr->ccImsCfg,
            pSecMgr->spiRangeStart, pSecMgr->spiRangeEnd,
            /*Each session uses 4 SPIs*/ pSecMgr->maxIpsecSessions*4,
            pSecMgr->pLogMgr);
        if (RV_OK != crv)
        {
            RvMemoryFree((void*)pSecMgr, pSecMgr->pLogMgr);
            RvLogError(pCfg->pLogSrc,(pCfg->pLogSrc,
                "SipSecurityMgrConstruct - Failed to construct Common Core IPsec configuration"));
            return CRV2RV(crv);
        }
    }
	
	/* Construct IPsec Engine */
	crv = RvIpsecConstruct(pSecMgr->pLogMgr);
	if (crv != RV_OK)
	{
        RvMemoryFree((void*)pSecMgr, pSecMgr->pLogMgr);
        RvLogError(pCfg->pLogSrc,(pCfg->pLogSrc,
                "SipSecurityMgrConstruct - Failed to construct Common Core IPsec module"));
        return CRV2RV(crv);
	}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
    /*set a mutex for the manager*/
    crv = RvMutexConstruct(pSecMgr->pLogMgr, &pSecMgr->hMutex);
    if(crv != RV_OK)
    {
        RvMemoryFree((void*)pSecMgr, pSecMgr->pLogMgr);
        RvLogError(pCfg->pLogSrc,(pCfg->pLogSrc,
            "SipSecurityMgrConstruct - Failed to construct mutex"));
        return CRV2RV(crv);
    }
#endif

    /* Construct pool of Security Objects */
    pSecMgr->hSecObjPool = RLIST_PoolListConstruct(
        pSecMgr->maxSecObjects, 1, sizeof(SecObj), pSecMgr->pLogMgr, "Security Objects");
    if(NULL == pSecMgr->hSecObjPool)
    {
        RvLogError(pCfg->pLogSrc,(pCfg->pLogSrc,
            "SipSecurityMgrConstruct - Failed to construct pool of %d Security Objects",
            pSecMgr->maxSecObjects));
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
        RvMutexDestruct(&pSecMgr->hMutex,pSecMgr->pLogMgr);
#endif
        RvMemoryFree((void*)pSecMgr, pSecMgr->pLogMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }
    pSecMgr->hSecObjList = RLIST_ListConstruct(pSecMgr->hSecObjPool);
    if(NULL == pSecMgr->hSecObjList)
    {
        RvLogError(pCfg->pLogSrc,(pCfg->pLogSrc,
            "SipSecurityMgrConstruct - Failed to construct list of Security Objects from pool %p",
            pSecMgr->hSecObjPool));
        RLIST_PoolListDestruct(pSecMgr->hSecObjPool);
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
        RvMutexDestruct(&pSecMgr->hMutex,pSecMgr->pLogMgr);
#endif
        RvMemoryFree((void*)pSecMgr, pSecMgr->pLogMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* Construct pool of IPsec sessions */
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    if (pSecMgr->maxIpsecSessions > 0)
    {
        pSecMgr->hIpsecSessionPool = RLIST_PoolListConstruct(
            pSecMgr->maxIpsecSessions,1,sizeof(SecObjIpsec),pSecMgr->pLogMgr,
            "IPsec Sessions");
        if(NULL == pSecMgr->hIpsecSessionPool)
        {
            RvLogError(pCfg->pLogSrc,(pCfg->pLogSrc,
                "SipSecurityMgrConstruct - Failed to construct pool of %d IPsec Sessions",
                pSecMgr->maxIpsecSessions));
            RLIST_PoolListDestruct(pSecMgr->hSecObjPool);
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
            RvMutexDestruct(&pSecMgr->hMutex,pSecMgr->pLogMgr);
#endif
            RvMemoryFree((void*)pSecMgr, pSecMgr->pLogMgr);
            return RV_ERROR_OUTOFRESOURCES;
        }
        pSecMgr->hIpsecSessionList = RLIST_ListConstruct(pSecMgr->hIpsecSessionPool);
        if(NULL == pSecMgr->hIpsecSessionList)
        {
            RvLogError(pCfg->pLogSrc,(pCfg->pLogSrc,
                "SipSecurityMgrConstruct - Failed to construct list of IPsec Sessions from pool %p",
                pSecMgr->hIpsecSessionPool));
            RLIST_PoolListDestruct(pSecMgr->hIpsecSessionPool);
            RLIST_PoolListDestruct(pSecMgr->hSecObjPool);
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
            RvMutexDestruct(&pSecMgr->hMutex,pSecMgr->pLogMgr);
#endif
            RvMemoryFree((void*)pSecMgr, pSecMgr->pLogMgr);
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

    /* Set transport event handlers into the Transport Manager */
    if (NULL != pSecMgr->hTransportMgr)
    {
        SipTransportSecEvHandlers evHandlers;
        evHandlers.pfnMsgRcvdHandler          = SipSecObjMsgReceivedEv;
        evHandlers.pfnMsgSendFailureHandler   = SipSecObjMsgSendFailureEv;
        evHandlers.pfnConnStateChangedHandler = SipSecObjConnStateChangedEv;
        rv = SipTransportSetSecEventHandlers(
                                        pSecMgr->hTransportMgr, &evHandlers);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "SipSecurityMgrConstruct - failed to set transport event handlers(rv=%d:%s)",
                rv, SipCommonStatus2Str(rv)));
            if (NULL != pSecMgr->hIpsecSessionPool)
            {
                RLIST_PoolListDestruct(pSecMgr->hIpsecSessionPool);
            }
            RLIST_PoolListDestruct(pSecMgr->hSecObjPool);
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
            RvMutexDestruct(&pSecMgr->hMutex,pSecMgr->pLogMgr);
#endif
            RvMemoryFree((void*)pSecMgr, pSecMgr->pLogMgr);
            return rv;
        }

    }

    /* Create Locks for all Security Objects in advance.
       These locks live as long as the Security Manager lives.
       This code block should be last before return.
       Otherwise - leak of Locks might occur */
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
    rv = ConstructLocksInSecObjPool(pSecMgr);
    if(RV_OK != rv)
    {
        RvLogError(pCfg->pLogSrc,(pCfg->pLogSrc,
            "SipSecurityMgrConstruct - Failed to construct locks in the elements of the Security Object Pool %p",
            pSecMgr->hSecObjPool));
        if (NULL != pSecMgr->hIpsecSessionPool)
        {
            RLIST_PoolListDestruct(pSecMgr->hIpsecSessionPool);
        }
        RLIST_PoolListDestruct(pSecMgr->hSecObjPool);
        RvMutexDestruct(&pSecMgr->hMutex,pSecMgr->pLogMgr);
        RvMemoryFree((void*)pSecMgr, pSecMgr->pLogMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }
#endif

    *phSecMgr = (RvSipSecMgrHandle)pSecMgr;

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SipSecurityMgrConstruct - Security Manager %p was constructed successfully. %d Security Objects. %d IPsec Sessions",
        pSecMgr, pSecMgr->maxSecObjects, pSecMgr->maxIpsecSessions));
#else
    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SipSecurityMgrConstruct - Security Manager %p was constructed successfully. %d Security Objects. 0 IPsec Sessions(IPSEC is not supported)",
        pSecMgr, pSecMgr->maxSecObjects));
#endif

    return RV_OK;
}

/******************************************************************************
 * SipSecurityMgrDestruct
 * ----------------------------------------------------------------------------
 * General: Destructs the Security manager freeing all resources.
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr - Handle to the manager to be destructed
 *****************************************************************************/

RvStatus SipSecurityMgrDestruct(
                                IN RvSipSecMgrHandle hSecMgr)
{
    SecMgr*      pSecMgr = (SecMgr*)hSecMgr;
    RvLogSource* pLogSrc = pSecMgr->pLogSrc;

    /*To prevent compilation warning on Nucleus, while compiling without logs*/
    RV_UNUSED_ARG(pLogSrc);

    if (RV_OK != SecMgrLock(&pSecMgr->hMutex,pSecMgr->pLogMgr))
	{
		return RV_ERROR_INVALID_HANDLE;
	}

    /* Free all existing Security Objects */
    if (NULL != pSecMgr->hSecObjList)
    {
        SecObj* pSecObj;

        RLIST_GetHead(pSecMgr->hSecObjPool,pSecMgr->hSecObjList,
                      (RLIST_ITEM_HANDLE*)&pSecObj);
        while (NULL != pSecObj)
        {
            SecObjFree(pSecObj);
            RLIST_GetHead(pSecMgr->hSecObjPool,pSecMgr->hSecObjList,
                          (RLIST_ITEM_HANDLE*)&pSecObj);
        }
    }

    /* Destruct IPsec Session Pool */
    if (NULL != pSecMgr->hIpsecSessionPool)
    {
        RLIST_PoolListDestruct(pSecMgr->hIpsecSessionPool);
        pSecMgr->hIpsecSessionPool = NULL;
    }

    /* Destruct locks in the elements of the Security Object Pool */
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
    DestructLocksInSecObjPool(pSecMgr);
#endif

    /* Destruct Security Object Pool */
    if (NULL != pSecMgr->hSecObjPool)
    {
        RLIST_PoolListDestruct(pSecMgr->hSecObjPool);
        pSecMgr->hSecObjPool = NULL;
    }

    SecMgrUnlock(&pSecMgr->hMutex,pSecMgr->pLogMgr);

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
    RvMutexDestruct(&pSecMgr->hMutex,pSecMgr->pLogMgr);
#endif

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    rvImsDestructCfg(&pSecMgr->ccImsCfg,pSecMgr->pLogMgr);
    RvIpsecDestruct(pSecMgr->pLogMgr);
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

    RvLogInfo(pLogSrc,(pLogSrc,
        "SipSecurityMgrDestruct - Security Manager %p was destructed successfully",
        pSecMgr));

    RvMemoryFree((void*)pSecMgr, pSecMgr->pLogMgr);

    return RV_OK;
}

/******************************************************************************
 * SipSecurityMgrGetResourcesStatus
 * ----------------------------------------------------------------------------
 * General: Fills the provided resources structure with the status of
 *          resources used by the Security module.
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr    - Handle to the Security manager.
 * Output:  pResources - Security resources.
 *****************************************************************************/
RvStatus SipSecurityMgrGetResourcesStatus (
                                IN  RvSipSecMgrHandle       hSecMgr,
                                OUT RvSipSecurityResources* pResources)
{
    SecMgr*  pSecMgr = (SecMgr*)hSecMgr;
    RvUint32 allocNumOfLists;
    RvUint32 allocMaxNumOfUserElement;
    RvUint32 currNumOfUsedLists;
    RvUint32 currNumOfUsedUserElement;
    RvUint32 maxUsageInLists;
    RvUint32 maxUsageInUserElement;

    if(NULL==pSecMgr || NULL==pResources)
    {
        return RV_ERROR_BADPARAM;
    }

    memset(pResources,0,sizeof(RvSipSecurityResources));

    /* List of Security Objects */
    if(NULL != pSecMgr->hSecObjPool)
    {
        RLIST_GetResourcesStatus(pSecMgr->hSecObjPool,
            &allocNumOfLists, /*allways 1*/
            &allocMaxNumOfUserElement,
            &currNumOfUsedLists,/*allways 1*/
            &currNumOfUsedUserElement,
            &maxUsageInLists,
            &maxUsageInUserElement);

        pResources->secobjPoolElements.currNumOfUsedElements  = currNumOfUsedUserElement;
        pResources->secobjPoolElements.numOfAllocatedElements = allocMaxNumOfUserElement;
        pResources->secobjPoolElements.maxUsageOfElements     = maxUsageInUserElement;
    }
    /* List of IPsec Sessions */
    if(NULL != pSecMgr->hIpsecSessionPool)
    {
        RLIST_GetResourcesStatus(pSecMgr->hIpsecSessionPool,
            &allocNumOfLists, /*allways 1*/
            &allocMaxNumOfUserElement,
            &currNumOfUsedLists,/*allways 1*/
            &currNumOfUsedUserElement,
            &maxUsageInLists,
            &maxUsageInUserElement);

        pResources->ipsecsessionPoolElements.currNumOfUsedElements  = currNumOfUsedUserElement;
        pResources->ipsecsessionPoolElements.numOfAllocatedElements = allocMaxNumOfUserElement;
        pResources->ipsecsessionPoolElements.maxUsageOfElements     = maxUsageInUserElement;
    }
    return RV_OK;
}

/******************************************************************************
 * SipSecurityMgrResetMaxUsageResourcesStatus
 * ----------------------------------------------------------------------------
 * General: Reset the counter that counts the max number of call-legs that
 *          were used ( at one time ) until the call to this routine.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:    hSecurityMgr - The Security manager.
 *****************************************************************************/
void SipSecurityMgrResetMaxUsageResourcesStatus (
                                IN  RvSipSecMgrHandle  hSecMgr)
{
    SecMgr*  pSecMgr = (SecMgr*)hSecMgr;
    RLIST_ResetMaxUsageResourcesStatus(pSecMgr->hSecObjPool);
    if (NULL != pSecMgr->hIpsecSessionPool)
    {
        RLIST_ResetMaxUsageResourcesStatus(pSecMgr->hIpsecSessionPool);
    }
}

/******************************************************************************
 * SipSecMgrSetEvHandlers
 * ----------------------------------------------------------------------------
 * General: Set owner event handlers for all Security events.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecMgr     - Handle to the Security Manager
 *            pEvHandlers - Structure with owner event handlers
 *****************************************************************************/
RvStatus SipSecMgrSetEvHandlers(
                                   IN  RvSipSecMgrHandle   hSecMgr,
                                   IN  RvSipSecEvHandlers* pEvHandlers)
{
    SecMgr* pSecMgr = (SecMgr*)hSecMgr;

    if(NULL==pSecMgr  || NULL==pEvHandlers)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SipSecMgrSetEvHandlers(hSecMgr=%p) - Set event handlers", hSecMgr));

    if (RV_OK != SecMgrLock(&pSecMgr->hMutex,pSecMgr->pLogMgr))
	{
		return RV_ERROR_INVALID_HANDLE;
	}
    memcpy(&(pSecMgr->ownerEvHandlers), pEvHandlers, sizeof(RvSipSecEvHandlers));
    SecMgrUnlock(&pSecMgr->hMutex,pSecMgr->pLogMgr);

    return RV_OK;
}

/*---------------------------------------------------------------------------*/
/*                             STATIC FUNCTIONS                              */
/*---------------------------------------------------------------------------*/

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/******************************************************************************
 * ConstructLocksInSecObjPool
 * ----------------------------------------------------------------------------
 * General: 1. Constructs a mutex for each element in the pool of Security Objects
 *          The Security Object mutexes are never destructed.
 *          They live as long as a Security Manager lives.
 *          2. Set pointer to the Security Manager into the elements.
 *          The pointers are never changed.
 *          They live as long as a Security Manager lives.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecMgr - Pointer to the Security Manager
 *****************************************************************************/
static RvStatus ConstructLocksInSecObjPool(SecMgr* pSecMgr)
{
    RvStatus rv;
    SecObj*  pSecObj;

    RLIST_PoolGetFirstElem(pSecMgr->hSecObjPool,(RLIST_ITEM_HANDLE*)&pSecObj);
    while (NULL != pSecObj)
    {
        /* Set pointer to Security Manager */
        pSecObj->pMgr = pSecMgr;

        /* Construct triple lock */
        rv = SipCommonConstructTripleLock(
                                    &pSecObj->tripleLock, pSecMgr->pLogMgr);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "ConstructLocksInSecObjPool(pSecMgr=%p) - Failed to construct Triple Lock(rv=%d:%s)",
                pSecMgr, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        pSecObj->pTripleLock = NULL;

        RLIST_PoolGetNextElem (pSecMgr->hSecObjPool,
            (RLIST_ITEM_HANDLE)pSecObj, (RLIST_ITEM_HANDLE*)&pSecObj);
    }

    return RV_OK;
}
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/******************************************************************************
 * DestructLocksInSecObjPool
 * ----------------------------------------------------------------------------
 * General: denstructs a mutex for each element in the pool of Security Objects
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecMgr - Pointer to the Security Manager
 *****************************************************************************/
static void DestructLocksInSecObjPool(SecMgr* pSecMgr)
{
    SecObj*  pSecObj;

    RLIST_PoolGetFirstElem(pSecMgr->hSecObjPool,(RLIST_ITEM_HANDLE*)&pSecObj);
    while (NULL != pSecObj)
    {
        pSecObj->pMgr = NULL;
        SipCommonDestructTripleLock(
            &pSecObj->tripleLock, pSecMgr->pLogMgr);
        RLIST_PoolGetNextElem (pSecMgr->hSecObjPool,
            (RLIST_ITEM_HANDLE)pSecObj, (RLIST_ITEM_HANDLE*)&pSecObj);
    }
}
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

#endif /*#ifdef RV_SIP_IMS_ON*/
