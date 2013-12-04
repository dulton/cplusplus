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
 *                              <ResolverMgrObject.c>
 *   This file includes the functionality of the Resolver manager object.
 *   The Resolver manager is responsible for creating new Resolver.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                  January 2004
 *********************************************************************************/
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "ResolverMgrObject.h"
#include "ResolverObject.h"
#include "ResolverDns.h"
#include "rvmemory.h"
#include "_SipCommonCore.h"
#include "_SipCommonUtils.h"
/*#include "ResolverDns.h"*/
#include "_SipTransport.h"
#include "ResolverObject.h"
#include "_SipCommonConversions.h"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV ResolversListConstruct(
                                     IN ResolverMgr*   pRslvMgr);
static void RVCALLCONV ResolversListDestruct(
                                     IN ResolverMgr*   pRslvMgr);

static void RVCALLCONV DestructAllResolvers(IN ResolverMgr*   pRslvMgr);

/*-----------------------------------------------------------------------*/
/*                     RESOLVER MGR FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * ResolverMgrConstruct
 * ------------------------------------------------------------------------
 * General: Constructs the Resolver manager.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCfg        - The module configuration.
 *          pStack      - A handle to the stack manager.
 *          bTryServers - Try and get the DNS servers from the os.
 *          bTryDomains - Try and get the DNS domains from the os.
 * Output:  *ppRslvMgr   - Pointer to the newly created Resolver manager.
 ***************************************************************************/
RvStatus RVCALLCONV ResolverMgrConstruct(
                            IN  SipResolverMgrCfg*            pCfg,
                            IN  void*                         pStack,
                            IN  RvBool                        bTryServers,
                            IN  RvBool                        bTryDomains,
                            OUT ResolverMgr**                 ppRslvMgr)
{
    RvStatus           rv               = RV_OK;
    RvStatus           crv              = RV_OK;
    ResolverMgr*       pRslvMgr         = NULL;
    RvSelectEngine*    pSelect          = NULL;
    RvDnsConfigType    dnscfg           = (RvDnsConfigType)0;
    RvChar*            strDialString    = NULL;

    *ppRslvMgr = NULL;
    
    /*allocate the Resolver manager*/
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(ResolverMgr), pCfg->pLogMgr, (void*)&pRslvMgr))
    {
         RvLogError(pCfg->pLogSrc,(pCfg->pLogSrc,
                   "ResolverMgrConstruct - Failed to allocate memory for the Resolver manager"));
         return RV_ERROR_UNKNOWN;
    }
    memset(pRslvMgr,0,sizeof(ResolverMgr));

    /*set information from the configuration structure*/
    pRslvMgr->pLogMgr             = pCfg->pLogMgr;
    pRslvMgr->pLogSrc             = pCfg->pLogSrc;
    pRslvMgr->hTransportMgr       = pCfg->hTransportMgr;
    pRslvMgr->seed           = pCfg->seed;
    pRslvMgr->maxNumOfRslv    = pCfg->maxNumOfRslv;
    pRslvMgr->hGeneralPool   = pCfg->hGeneralPool;
    pRslvMgr->hMessagePool   = pCfg->hMessagePool;
    pRslvMgr->hElementPool   = pCfg->hElementPool;
    pRslvMgr->pStack         = pStack;
#ifdef UPDATED_BY_SPIRENT
    pRslvMgr->hDnsServersPool = RPOOL_CoreConstruct(pCfg->maxDnsServers*sizeof(RvAddress),1,pRslvMgr->pLogMgr,RV_TRUE,"DNSSrvsPool");
    if(pRslvMgr->hDnsServersPool == NULL){
         RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
                   "ResolverMgrConstruct - DnsServersPool couldn't be constructed."));
         return RV_ERROR_OUTOFRESOURCES;
    }
#endif

    /* allocate mutex*/
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(RvMutex), pRslvMgr->pLogMgr, (void*)&pRslvMgr->pMutex))
    {
         RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
                   "ResolverMgrConstruct - Failed to allocate mutex for the Resolver manager"));
         return RV_ERROR_UNKNOWN;
    }
    strDialString = (NULL == pCfg->strDialPlanSuffix) ? "":pCfg->strDialPlanSuffix;

    if (RV_OK != RvMemoryAlloc(NULL, strlen(strDialString)+1, pRslvMgr->pLogMgr, (void*)&pRslvMgr->strDialPlanSuffix))
    {
         RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
                   "ResolverMgrConstruct - Failed to Dial Plan Sufix for the Resolver manager"));
         return RV_ERROR_UNKNOWN;
    }
    strcpy(pRslvMgr->strDialPlanSuffix,strDialString);
    pRslvMgr->dialPlanLen = (RvInt)strlen(strDialString)+1;
    
    crv = RvMutexConstruct(pRslvMgr->pLogMgr, pRslvMgr->pMutex);
    if(crv != RV_OK)
    {
         RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
                   "ResolverMgrConstruct - Failed to construct mutex for the Resolver manager"));
         RvMemoryFree((void*)pRslvMgr->pMutex,pRslvMgr->pLogMgr);
         pRslvMgr->pMutex = NULL;
         return CRV2RV(crv);
    }

    rv = RvSelectGetThreadEngine(pRslvMgr->pLogMgr,&pSelect);
    if (rv != RV_OK)
    {
        RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
            "ResolverMgrConstruct - Failed to obtain select engine"));
        return CRV2RV(rv);
    }


#if defined(UPDATED_BY_SPIRENT)
 {
   int i=0;
   for(i=0;i<RVSIP_MAX_NUM_VDEVBLOCK;i++) {

     if (RV_OK != RvMemoryAlloc(NULL, sizeof(RvDnsEngine), pRslvMgr->pLogMgr, (void*)&(pRslvMgr->pDnsEngine[i]))) {
       RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
				     "ResolverMgrConstruct - Failed to allocate DNS engine in the Resolver manager"));
       return RV_ERROR_UNKNOWN;
     }
     
     crv = RvAresConstruct(pSelect,
			   ResolverDNSNewRecordCB,
			   pCfg->maxDnsServers,
			   pCfg->maxDnsDomains,
			   pCfg->maxDnsBuffLen,
			   pRslvMgr->pLogMgr,
               i,
			   pRslvMgr->pDnsEngine[i]);
     
     if (crv != RV_OK) {
       RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
				     "ResolverMgrConstruct - Failed to construct DNS Engine"));
       RvMemoryFree((void*)pRslvMgr->pDnsEngine[i],pRslvMgr->pLogMgr);
       pRslvMgr->pDnsEngine[i] = NULL;
       return CRV2RV(crv);
     }
     
     /* if we need to get DNS domains or DNS servers from OS, do it now */
     dnscfg |= (RV_TRUE == bTryServers) ? RV_DNS_SERVERS  : (RvDnsConfigType)0;
     dnscfg |= (RV_TRUE == bTryDomains) ? RV_DNS_SUFFIXES : (RvDnsConfigType)0;
     RvLogDebug(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
				   "ResolverMgrConstruct - bTryServers=%d, bTryDomains=%d",bTryServers,bTryDomains));
     
     if (0 != (RvInt)dnscfg) {
       crv = RvAresConfigure(pRslvMgr->pDnsEngine[i],dnscfg);
       if (crv != RV_OK) {
	 RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
				       "ResolverMgrConstruct - Failed to configure DNS Engine from OS (dnscfg=%d)",(RvInt)dnscfg));
	 return CRV2RV(crv);
       }
     }
   }
 }
#else
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(RvDnsEngine), pRslvMgr->pLogMgr, (void*)&pRslvMgr->pDnsEngine))
    {
         RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
                   "ResolverMgrConstruct - Failed to allocate DNS engine in the Resolver manager"));
         return RV_ERROR_UNKNOWN;
    }

    crv = RvAresConstruct(pSelect,
                         ResolverDNSNewRecordCB,
                         pCfg->maxDnsServers,
                         pCfg->maxDnsDomains,
                         pCfg->maxDnsBuffLen,
                         pRslvMgr->pLogMgr,
                         pRslvMgr->pDnsEngine);

    if (crv != RV_OK)
    {
        RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
                   "ResolverMgrConstruct - Failed to construct DNS Engine"));
        RvMemoryFree((void*)pRslvMgr->pDnsEngine,pRslvMgr->pLogMgr);
        pRslvMgr->pDnsEngine = NULL;
        return CRV2RV(crv);
    }
    
    /* if we need to get DNS domains or DNS servers from OS, do it now */
    dnscfg |= (RV_TRUE == bTryServers) ? RV_DNS_SERVERS  : (RvDnsConfigType)0;
    dnscfg |= (RV_TRUE == bTryDomains) ? RV_DNS_SUFFIXES : (RvDnsConfigType)0;
    RvLogDebug(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
               "ResolverMgrConstruct - bTryServers=%d, bTryDomains=%d",bTryServers,bTryDomains));

    if (0 != (RvInt)dnscfg)
    {
        crv = RvAresConfigure(pRslvMgr->pDnsEngine,dnscfg);
        if (crv != RV_OK)
        {
            RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
                       "ResolverMgrConstruct - Failed to configure DNS Engine from OS (dnscfg=%d)",(RvInt)dnscfg));
            return CRV2RV(crv);
        }
    }
#endif

    /* construct Resolvers resources */
    rv = ResolversListConstruct(pRslvMgr);
    if(rv != RV_OK)
    {
        RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
                   "ResolverMgrConstruct - Failed to construct Resolvers list"));
        return rv;
    }

    *ppRslvMgr = pRslvMgr;

    return RV_OK;
}


/***************************************************************************
 * ResolverMgrCreateResolver
 * ------------------------------------------------------------------------
 * General: Creates a new Resolver
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRslvMgr   - Handle to the Resolver manager
 *            bIsAppRslv - indicates whether the Resolver was created
 *                        by the application or by the stack.
 *            hAppRslv   - Application handle to the Resolver.
 *            pTripleLock - A triple lock to use by the Resolver. If NULL
 *                        is supplied the Resolver will use its own lock.
 * Output:     phRslv    - sip stack handle to the new Resolver
 ***************************************************************************/
RvStatus RVCALLCONV ResolverMgrCreateResolver(
                   IN     ResolverMgr*              pRslvMgr,
                   IN     RvBool                    bIsAppRslv,
                   IN     RvSipAppResolverHandle    hAppRslv,
                   IN     SipTripleLock*            pTripleLock,
                   OUT    RvSipResolverHandle*      phRslv)
{
    Resolver*            pRslv        = NULL;
    RLIST_ITEM_HANDLE       hListItem   = NULL;
    RvStatus                rv          = RV_OK;

    *phRslv  = NULL;
    RvMutexLock(pRslvMgr->pMutex,pRslvMgr->pLogMgr); /*lock the Resolver mgr*/

    RvLogDebug(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
               "ResolverMgrCreateResolver - Creating a new Resolver..."));

    /*allocate a new resolver object*/
    rv = RLIST_InsertTail(pRslvMgr->hRslvListPool,
                          pRslvMgr->hRslvList,
                          &hListItem);

    if(rv != RV_OK)
    {
        RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
                   "ResolverMgrCreateResolver - Failed to create a new Resolver (rv=%d)",rv));
        RvMutexUnlock(pRslvMgr->pMutex,pRslvMgr->pLogMgr);
        return rv;
    }

    pRslv = (Resolver*)hListItem;

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    rv = ResolverInitialize(pRslv,bIsAppRslv,hAppRslv,pTripleLock, pRslvMgr);
#else
    rv = ResolverInitialize(pRslv,bIsAppRslv,hAppRslv,pTripleLock);
#endif
/* SPIRENT_END */

    if(rv != RV_OK)
    {
        RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
                   "ResolverMgrCreateResolver - Failed to initialize the Resolver (rv=%d)",rv));
        RLIST_Remove(pRslvMgr->hRslvListPool,pRslvMgr->hRslvList,
                     (RLIST_ITEM_HANDLE)pRslv);
        RvMutexUnlock(pRslvMgr->pMutex,pRslvMgr->pLogMgr);
        return rv;
    }

    RvMutexUnlock(pRslvMgr->pMutex,pRslvMgr->pLogMgr); /*unlock the manager*/

    *phRslv = (RvSipResolverHandle)pRslv;
    RvLogDebug(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
               "ResolverMgrCreateResolver - Resolver 0x%p was created successfully, pTripleLock=0x%p",
               pRslv,pRslv->pTripleLock));

    return RV_OK;
}

/***************************************************************************
 * ResolverMgrRemoveResolver
 * ------------------------------------------------------------------------
 * General: removes a Resolver object from the Resolvers list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRslvMgr   - Handle to the Resolver manager
 *            hRslv      - Handle to the Resolver
 ***************************************************************************/
void RVCALLCONV ResolverMgrRemoveResolver(
                                IN ResolverMgr*        pRslvMgr,
                                IN RvSipResolverHandle hRslv)
{
    Resolver*           pRslv = (Resolver*)hRslv;
    
    RvMutexLock(pRslvMgr->pMutex,pRslvMgr->pLogMgr); /*lock the Resolver manager*/

    RvLogInfo(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
               "ResolverMgrRemoveResolver - removing Resolver 0x%p from list",pRslv));

    RLIST_Remove(pRslvMgr->hRslvListPool,pRslvMgr->hRslvList,
                     (RLIST_ITEM_HANDLE)pRslv);
    RvMutexUnlock(pRslvMgr->pMutex,pRslvMgr->pLogMgr); /*unlock the manager*/
}

/************************************************************************************
 * ResolverMgrClearDnsCache
 * ----------------------------------------------------------------------------------
 * General: Asks the ARES module to reset its' DNS cache.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pRslv      - Resolver
 ***********************************************************************************/
RvStatus RVCALLCONV ResolverMgrClearDnsCache(IN ResolverMgr *pRslvMgr)
{
	RvStatus rv; 

	RvLogDebug(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
		"ResolverMgrClearDnsCache - pRslvMgr=0x%p. about to clear the DNS cache",pRslvMgr));

#if defined(UPDATED_BY_SPIRENT)
 {
   int i=0;
   rv = RV_OK;
   for(i=0;i<RVSIP_MAX_NUM_VDEVBLOCK;i++) {
     rv |= RvAresClearCache(pRslvMgr->pDnsEngine[i]);
   }
 }
#else
	rv = RvAresClearCache(pRslvMgr->pDnsEngine);
#endif
	if (RV_OK != rv)
	{
		RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
			"ResolverMgrClearDnsCache - pRslvMgr=0x%p. failed to clear cache",pRslvMgr));
		return rv;  
	}

	return RV_OK;
}
#if defined(UPDATED_BY_SPIRENT)
/************************************************************************************
 * ResolverMgrClearDnsEngineCache
 * ----------------------------------------------------------------------------------
 * General: Asks the ARES module to reset its' DNS cache.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pRslv      - Resolver
 *          vdevblock  - virtual device block to which dns engine blongs
 ***********************************************************************************/
RvStatus RVCALLCONV ResolverMgrClearDnsEngineCache(IN ResolverMgr *pRslvMgr,RvUint16 vdevblock)
{
	RvStatus rv; 

	RvLogDebug(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
		"ResolverMgrClearDnsEngineCache - pRslvMgr=0x%p vdevblock:%d. about to clear the DNS Engine cache",pRslvMgr,vdevblock));

    RVSIP_ADJUST_VDEVBLOCK(vdevblock);

	rv = RvAresClearCache(pRslvMgr->pDnsEngine[vdevblock]);
	if (RV_OK != rv)
	{
		RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
			"ResolverMgrClearDnsEngineCache - pRslvMgr=0x%p vdevblock:%d. failed to clear cache",pRslvMgr,vdevblock));
		return rv;  
	}

	return RV_OK;
}
#endif

/************************************************************************************
 * ResolverMgrDumpDnsCache
 * ----------------------------------------------------------------------------------
 * General: Asks the ARES module to dump its' DNS cache into a file.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input: hRslvMgr    - Resolver manager
 *        strDumpFile - The name of the file to dump the cache into
 ***********************************************************************************/
RvStatus ResolverMgrDumpDnsCache(IN ResolverMgr *pRslvMgr,
								 IN RvChar      *strDumpFile)
{
#if RV_DNS_CACHE_DEBUG
	RvLogDebug(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
		"ResolverMgrDumpDnsCache - pRslvMgr=0x%p. about to dump the DNS cache into %s",
		pRslvMgr,strDumpFile));
#else
	RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
		"ResolverMgrDumpDnsCache - pRslvMgr=0x%p. Unable to dump DNS cache when RV_DNS_CACH_DEBUG is false.",
		pRslvMgr));
#endif
	
	
	RvAresCacheDumpGlobal(strDumpFile);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pRslvMgr);
#endif

	return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * ResolversListConstruct
 * ------------------------------------------------------------------------
 * General: Allocated the list of Resolvers and initialize their locks.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslvMgr   - Pointer to the Resolver manager
 ***************************************************************************/
static RvStatus RVCALLCONV ResolversListConstruct(
                                          IN ResolverMgr*   pRslvMgr)
{
    Resolver*    pRslv = NULL;
    RvStatus        rv   = RV_OK;

    /*allocate a pool with 1 list*/
    pRslvMgr->hRslvListPool = RLIST_PoolListConstruct(pRslvMgr->maxNumOfRslv,
                                                    1,
                                                    sizeof(Resolver),
                                                    pRslvMgr->pLogMgr ,
                                                    "Resolvers List");

    if(pRslvMgr->hRslvListPool == NULL)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }

    /*allocate a list from the pool*/
    pRslvMgr->hRslvList = RLIST_ListConstruct(pRslvMgr->hRslvListPool);

    if(pRslvMgr->hRslvList == NULL)
    {
        ResolversListDestruct(pRslvMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }


    RLIST_PoolGetFirstElem(pRslvMgr->hRslvListPool,
                           (RLIST_ITEM_HANDLE*)&pRslv);
    while (NULL != pRslv)
    {
        RvStatus rvInit = RV_OK;
        pRslv->pRslvMgr = pRslvMgr;
        rvInit = SipCommonConstructTripleLock(
                                    &pRslv->rslvTripleLock, pRslvMgr->pLogMgr);
        pRslv->pTripleLock = NULL;
        RLIST_PoolGetNextElem(pRslvMgr->hRslvListPool,
                              (RLIST_ITEM_HANDLE)pRslv,
                              (RLIST_ITEM_HANDLE*)&pRslv);
        if(rvInit != RV_OK)
        {
            rv = rvInit;
        }
    }

    if(rv != RV_OK)
    {
        ResolversListDestruct(pRslvMgr);
    }
    return RV_OK;
}

/***************************************************************************
 * DestructAllResolvers
 * ------------------------------------------------------------------------
 * General: Destructs the list of Resolvers and free their locks.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslvMgr   - Pointer to the Resolver manager
 ***************************************************************************/
static void RVCALLCONV DestructAllResolvers(IN ResolverMgr*   pRslvMgr)
{
    Resolver    *pNextToKill = NULL;
    Resolver    *pRslv2Kill = NULL;

    if(pRslvMgr->hRslvListPool == NULL)
    {
        return;
    }


    RLIST_GetHead(pRslvMgr->hRslvListPool,pRslvMgr->hRslvList,(RLIST_ITEM_HANDLE *)&pRslv2Kill);

    while (NULL != pRslv2Kill)
    {
        RLIST_GetNext(pRslvMgr->hRslvListPool,pRslvMgr->hRslvList,(RLIST_ITEM_HANDLE)pRslv2Kill,(RLIST_ITEM_HANDLE *)&pNextToKill);
        ResolverDestruct(pRslv2Kill);
        pRslv2Kill = pNextToKill;
    }
}

/***************************************************************************
 * ResolversListDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the list of Resolvers and free their locks.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslvMgr   - Pointer to the Resolver manager
 ***************************************************************************/
static void RVCALLCONV ResolversListDestruct(IN ResolverMgr*   pRslvMgr)
{
    Resolver       *pRslv;

    if(pRslvMgr->hRslvListPool == NULL)
    {
        return;
    }

    if(pRslvMgr->hRslvList == NULL)
    {
        RLIST_PoolListDestruct(pRslvMgr->hRslvListPool);
        pRslvMgr->hRslvListPool = NULL;
        return;
    }

    /*free triple locks.*/
    RLIST_PoolGetFirstElem(pRslvMgr->hRslvListPool,
                           (RLIST_ITEM_HANDLE*)&pRslv);
    while (NULL != pRslv)
    {
        pRslv->pRslvMgr = pRslvMgr;
        SipCommonDestructTripleLock(&pRslv->rslvTripleLock, pRslvMgr->pLogMgr);
        RLIST_PoolGetNextElem(pRslvMgr->hRslvListPool,
                              (RLIST_ITEM_HANDLE)pRslv,
                              (RLIST_ITEM_HANDLE*)&pRslv);
    }
    /*free the list pool*/
    RLIST_PoolListDestruct(pRslvMgr->hRslvListPool);
    pRslvMgr->hRslvListPool = NULL;
    pRslvMgr->hRslvList     = NULL;
}

/***************************************************************************
 * ResolverMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destruct the Resolver manager and free all its resources.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pRslvMgr     - The Resolver manager.
 ***************************************************************************/
RvStatus RVCALLCONV ResolverMgrDestruct(
                            IN ResolverMgr*   pRslvMgr)
{
    RvLogMgr*   pLogMgr = NULL;
    /*noting to free*/
    if(pRslvMgr == NULL)
    {
        return RV_OK;
    }

    pLogMgr = pRslvMgr->pLogMgr;
    if(pRslvMgr->pMutex != NULL)
    {
        RvMutexLock(pRslvMgr->pMutex,pRslvMgr->pLogMgr);
    }

    ResolverMgrStopAres(pRslvMgr);

    DestructAllResolvers(pRslvMgr);

    ResolversListDestruct(pRslvMgr);
#ifdef UPDATED_BY_SPIRENT
    if(pRslvMgr->hDnsServersPool)
        RPOOL_Destruct(pRslvMgr->hDnsServersPool);
#endif

    RvMemoryFree(pRslvMgr->strDialPlanSuffix,pRslvMgr->pLogMgr);
    pRslvMgr->strDialPlanSuffix = NULL;
    
    if(pRslvMgr->pMutex != NULL)
    {
        RvMutexUnlock(pRslvMgr->pMutex,pRslvMgr->pLogMgr);
        RvMutexDestruct(pRslvMgr->pMutex,pRslvMgr->pLogMgr);
        RvMemoryFree((void*)pRslvMgr->pMutex,pRslvMgr->pLogMgr);
    }

    RvMemoryFree((void*)pRslvMgr,pLogMgr);

    return RV_OK;
}

/***************************************************************************
 * ResolverMgrStopAres
 * ------------------------------------------------------------------------
 * General: Stops DNS functionality in Resolver module
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pRslvMgr     - The Resolver manager.
 ***************************************************************************/
void RVCALLCONV ResolverMgrStopAres(
                            IN ResolverMgr*   pRslvMgr)
{
    RvMutexLock(pRslvMgr->pMutex,pRslvMgr->pLogMgr);
#if defined(UPDATED_BY_SPIRENT)
 {
   int i=0;
   for(i=0;i<RVSIP_MAX_NUM_VDEVBLOCK;i++) {
     if(pRslvMgr->pDnsEngine[i] != NULL) {
       RvAresDestruct(pRslvMgr->pDnsEngine[i]);
       RvMemoryFree((void*)pRslvMgr->pDnsEngine[i],pRslvMgr->pLogMgr);
     }
     pRslvMgr->pDnsEngine[i] = NULL;
   }
 }
#else
    if(pRslvMgr->pDnsEngine != NULL)
    {
        RvAresDestruct(pRslvMgr->pDnsEngine);
        RvMemoryFree((void*)pRslvMgr->pDnsEngine,pRslvMgr->pLogMgr);
    }
    pRslvMgr->pDnsEngine = NULL;
#endif
    RvMutexUnlock(pRslvMgr->pMutex,pRslvMgr->pLogMgr);
}
