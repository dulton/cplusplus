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
 *                              <_SipResolverMgr.c>
 *
 * This file defines the Resolver internal API functions
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                    JAn 2005
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipResolverTypes.h"
#include "_SipResolverMgr.h"
#include "ResolverMgrObject.h"
#include "ResolverObject.h"

#include "_SipTransport.h"
#include "_SipCommonConversions.h"
#include "_SipCommonUtils.h"

/*-----------------------------------------------------------------------*/
/*                        MACRO DEFINITIONS                              */
/*-----------------------------------------------------------------------*/
#define KNOWN_DNS_SERVER_PORT 53

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * SipResolverMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct a new Resolver manager. The Resolver manager is
 *          responsible for all the Resolvers. The manager holds a list of
 *          Resolvers and is used to create new Resolvers.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCfg        - The module configuration.
 *          pStack      - A handle to the stack manager.
 *          bTryServers - Try and get the DNS servers from the os.
 *          bTryDomains - Try and get the DNS domains from the os.
 * Output:  *phRslvMgr   - Handle to the newly created Resolver manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipResolverMgrConstruct(
                            IN  SipResolverMgrCfg*            pCfg,
                            IN  void*                         pStack,
                            IN  RvBool                        bTryServers,
                            IN  RvBool                        bTryDomains,
                            OUT RvSipResolverMgrHandle*         phRslvMgr)
{
    RvStatus rv = RV_OK;
    rv = ResolverMgrConstruct(pCfg,pStack,bTryServers,bTryDomains,
        (ResolverMgr**)phRslvMgr);
    if (RV_OK != rv)
    {
        RvLogError(pCfg->pLogSrc,(pCfg->pLogSrc,
              "SipResolverMgrConstruct - failed to construct the resolver Mgr (rv = %d)",rv));
        return rv;

    }
    return rv;
}

/***************************************************************************
 * SipResolverMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the Resolver manager freeing all resources.
 * Return Value:  RV_OK -  Resolver manager was destructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pRslvMgr - Handle to the manager to destruct.
 ***************************************************************************/
RvStatus RVCALLCONV SipResolverMgrDestruct(
                            IN RvSipResolverMgrHandle hRslvMgr)
{
    ResolverMgr*    pRslvMgr   = NULL;
    RvStatus           rv        = RV_OK;
    if(hRslvMgr == NULL)
    {
        return RV_OK;
    }
    pRslvMgr = (ResolverMgr*)hRslvMgr;

    RvLogInfo(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
              "SipResolverMgrDestruct - About to construct the Resolver manager 0x%p",pRslvMgr))

    rv = ResolverMgrDestruct(pRslvMgr);
    if(rv != RV_OK)
    {
         RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
                   "SipResolverMgrDestruct - Failed, destructing the module (rv = %d)",rv));
         return rv;
    }
    return RV_OK;
}

/***************************************************************************
 * SipResolverMgrStopAres
 * ------------------------------------------------------------------------
 * General: Stops DNS functionality in Resolver module
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hRslvMgr     - The Resolver manager.
 ***************************************************************************/
void RVCALLCONV SipResolverMgrStopAres(
                            IN RvSipResolverMgrHandle hRslvMgr)
{
    ResolverMgrStopAres((ResolverMgr*)hRslvMgr);
}


/***************************************************************************
 * SipResolverMgrGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: Returns a structure with the status of all resources used by
 *          the Resolver module.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hRslvMgr       - Handle to the Resolver manager.
 * Output:     pResources - Pointer to a Resolver resource structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipResolverMgrGetResourcesStatus (
                                 IN  RvSipResolverMgrHandle     hRslvMgr,
                                 OUT RvSipResolverResources*  pResources)
{
    ResolverMgr*     pRslvMgr                = (ResolverMgr*)hRslvMgr;
    RvUint32         allocNumOfLists         = 0;
    RvUint32         allocMaxNumOfUserElement= 0;
    RvUint32         currNumOfUsedLists      = 0;
    RvUint32         currNumOfUsedUserElement= 0;
    RvUint32         maxUsageInLists         = 0;
    RvUint32         maxUsageInUserElement   = 0;

    /* getting Resolvers */
    RLIST_GetResourcesStatus(pRslvMgr->hRslvListPool,
                             &allocNumOfLists,
                             &allocMaxNumOfUserElement,
                             &currNumOfUsedLists,
                             &currNumOfUsedUserElement,
                             &maxUsageInLists,
                             &maxUsageInUserElement);

    pResources->resolvers.currNumOfUsedElements  = currNumOfUsedUserElement;
    pResources->resolvers.numOfAllocatedElements = allocMaxNumOfUserElement;
    pResources->resolvers.maxUsageOfElements     = maxUsageInUserElement;
    return RV_OK;
}


/***************************************************************************
 * SipResolverMgrCreateResolver
 * ------------------------------------------------------------------------
 * General: Creates a new Resolver and exchange handles with the
 *          application.  The new Resolver assumes the IDLE state.
 *          Using this function the transaction can supply its memory
 *          page and triple lock to the Resolver.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRslvMgr - Handle to the Resolver manager
 *            hAppRslv - Application handle to the newly created Resolver.
 *            pTripleLock - A triple lock to use by the Resolver. If NULL
 *                        is supplied the Resolver will use its own lock.
 * Output:    phRslv   -   SIP stack handle to the Resolver
 ***************************************************************************/
RvStatus RVCALLCONV SipResolverMgrCreateResolver(
                   IN  RvSipResolverMgrHandle   hRslvMgr,
                   IN  RvSipAppResolverHandle   hAppRslv,
                   IN  SipTripleLock*           pTripleLock,
                   OUT RvSipResolverHandle*     phRslv)
{
    RvStatus       rv      = RV_OK;
    ResolverMgr*   pRslvMgr = (ResolverMgr*)hRslvMgr;

    *phRslv = NULL;

    RvLogInfo(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
              "SipResolverMgrCreateResolver - Creating a new Resolver"));

    /*create a new Resolver. This will lock the Resolver manager*/
    rv = ResolverMgrCreateResolver(pRslvMgr,RV_FALSE,hAppRslv,pTripleLock,phRslv);
    if(rv != RV_OK)
    {
        RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
              "SipResolverMgrCreateResolver - Failed, ResolverMgr failed to create a new Resolver (rv=%d:%s)",
              rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    RvLogInfo(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
              "SipResolverMgrCreateResolver - New Resolver 0x%p was created successfully",*phRslv));
    return RV_OK;

}


/************************************************************************************
 * SipResolverMgrSetDnsServers
 * ----------------------------------------------------------------------------------
 * General: Sets a new list of dns servers to the stack
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hRslvMgr          - resolver mgr
 *          pDnsServers      - a list of DNS servers to set to the stack
 *          numOfDnsServers  - The number of Dns servers in the list
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV SipResolverMgrSetDnsServers(
      IN  RvSipResolverMgrHandle   hRslvMgr,
      IN  RvSipTransportAddr*       pDnsServers,
      IN  RvInt32                   numOfDnsServers
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
)
{
    RvStatus        crv         = RV_OK;
    RvStatus        rv          = RV_OK;
    HPAGE           hServerPage = NULL_PAGE;
    ResolverMgr* pMgr        = (ResolverMgr*)hRslvMgr;
    RvAddress*      pCoreDNSs   = NULL;
    RvInt32         offset      = 0;
    RvInt32         i           = 0;

    RvMutexLock(pMgr->pMutex,pMgr->pLogMgr); /*lock the manager*/
    /* we need consecutive memory to convert from API address to Core address. we use a page */
#ifdef UPDATED_BY_SPIRENT
    rv = RPOOL_GetPage(pMgr->hDnsServersPool,0,&hServerPage);
#else
    rv = RPOOL_GetPage(pMgr->hGeneralPool,0,&hServerPage);
#endif
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "SipResolverMgrSetDnsServers - failed to get memory for setting the servers (rv=%d)",rv));
        RvMutexUnlock(pMgr->pMutex,pMgr->pLogMgr); /*unlock the manager*/
        return rv;
    }
#ifdef UPDATED_BY_SPIRENT
    pCoreDNSs = (RvAddress*)RPOOL_AlignAppend(pMgr->hDnsServersPool,hServerPage,sizeof(RvAddress)*numOfDnsServers,&offset);
#else
    pCoreDNSs = (RvAddress*)RPOOL_AlignAppend(pMgr->hGeneralPool,hServerPage,sizeof(RvAddress)*numOfDnsServers,&offset);
#endif
    if (NULL == pCoreDNSs)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "SipResolverMgrSetDnsServers - failed to get memory for core server list (rv=%d)",RV_ERROR_OUTOFRESOURCES));
        RvMutexUnlock(pMgr->pMutex,pMgr->pLogMgr); /*unlock the manager*/
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* convert each of the addresses, when we fail, stop */
    for (i=0;i<numOfDnsServers;i++)
    {
#if !(RV_NET_TYPE & RV_NET_IPV6)
        if (pDnsServers[i].eAddrType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP6)
        {
            rv = RV_ERROR_BADPARAM;
            break;
        }
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/
        rv = SipTransportConvertApiTransportAddrToCoreAddress(
                pMgr->hTransportMgr, &pDnsServers[i],&pCoreDNSs[i]);
        if (RV_OK != rv)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                      "SipResolverMgrSetDnsServers - address %s is not valid (rv=%d)",pDnsServers[i].strIP,rv))
            break;
        }
        if (0 == RvAddressGetIpPort(&pCoreDNSs[i]))
        {
            RvAddressSetIpPort(&pCoreDNSs[i],KNOWN_DNS_SERVER_PORT);
        }
    }

    /* we try to set parameters only if conversion worked */
    if (RV_OK == rv)
#if defined(UPDATED_BY_SPIRENT)
      RVSIP_ADJUST_VDEVBLOCK(vdevblock);
      crv = RvAresSetParams(pMgr->pDnsEngine[vdevblock],-1,-1,pCoreDNSs,numOfDnsServers,NULL,0);
#else
      crv = RvAresSetParams(pMgr->pDnsEngine,-1,-1,pCoreDNSs,numOfDnsServers,NULL,0);
#endif

    /* releasing resources */
    for(;i>0;i--)
    {
        RvAddressDestruct(&pCoreDNSs[i]);
    }
#ifdef UPDATED_BY_SPIRENT
    RPOOL_FreePage(pMgr->hDnsServersPool,hServerPage);
#else
    RPOOL_FreePage(pMgr->hGeneralPool,hServerPage);
#endif

    if (RV_OK != rv)
    {
        RvMutexUnlock(pMgr->pMutex,pMgr->pLogMgr); /*unlock the manager*/
        return rv;
    }
    if (RV_OK != crv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipResolverMgrSetDnsServers - failed to set Dns Servers (rv=%d)",CRV2RV(crv)))
        RvMutexUnlock(pMgr->pMutex,pMgr->pLogMgr); /*unlock the manager*/
        return CRV2RV(crv);
    }
    RvMutexUnlock(pMgr->pMutex,pMgr->pLogMgr); /*unlock the manager*/
    return rv;
}

#if defined(UPDATED_BY_SPIRENT)
RVAPI RvStatus RVCALLCONV SipResolverMgrSetTimeout(
      IN  RvSipResolverMgrHandle   hRslvMgr,
      IN  RvInt timeout,
      IN  RvInt tries,
      IN  RvUint16 vdevblock
)
{
  RvStatus        crv         = RV_OK;
  RvStatus        rv          = RV_OK;
  ResolverMgr* pMgr        = (ResolverMgr*)hRslvMgr;
  
  RvMutexLock(pMgr->pMutex,pMgr->pLogMgr); /*lock the manager*/
  
  RVSIP_ADJUST_VDEVBLOCK(vdevblock);

  crv = RvAresSetParams(pMgr->pDnsEngine[vdevblock],timeout,tries,NULL,0,NULL,0);
  
  if (RV_OK != crv) {
    RvMutexUnlock(pMgr->pMutex,pMgr->pLogMgr); /*unlock the manager*/
    return CRV2RV(crv);
  }
  RvMutexUnlock(pMgr->pMutex,pMgr->pLogMgr); /*unlock the manager*/
  return rv;
}
#endif

/************************************************************************************
 * SipResolverMgrGetDnsServers
 * ----------------------------------------------------------------------------------
 * General: Sets a new list of dns servers to the stack
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hRslvMgr          - Resolver mgr
 * Output:  pDnsServers      - a list of DNS servers that will be filled
 *          pNumOfDnsServers - the size of pDnsServers. will be filled with the
 *                             acctual size of the DNS server list
 ***********************************************************************************/
RvStatus RVCALLCONV SipResolverMgrGetDnsServers(
      IN  RvSipResolverMgrHandle   hRslvMgr,
      IN  RvSipTransportAddr*         pDnsServers,
      INOUT  RvInt32*                 pNumOfDnsServers
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
)
{
    RvStatus        crv         = RV_OK;
    RvStatus        rv          = RV_OK;
    HPAGE           hServerPage = NULL_PAGE;
    ResolverMgr* pMgr        = (ResolverMgr*)hRslvMgr;
    RvAddress*      pCoreDNSs   = NULL;
    RvInt32         offset      = 0;
    RvInt32         i           = 0;
    RvInt32         suggestedSize = *pNumOfDnsServers;


    RvMutexLock(pMgr->pMutex,pMgr->pLogMgr); /*lock the manager*/
    /* we need consecutive memory to convert from API address to Core address. we use a page */
#ifdef UPDATED_BY_SPIRENT
    rv = RPOOL_GetPage(pMgr->hDnsServersPool,0,&hServerPage);
#else
    rv = RPOOL_GetPage(pMgr->hGeneralPool,0,&hServerPage);
#endif
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "SipResolverMgrGetDnsServers - failed to get memory for setting the servers (rv=%d)",rv));
        RvMutexUnlock(pMgr->pMutex,pMgr->pLogMgr); /*unlock the manager*/
        return rv;
    }
#ifdef UPDATED_BY_SPIRENT
    pCoreDNSs = (RvAddress*)RPOOL_AlignAppend(pMgr->hDnsServersPool,hServerPage,sizeof(RvAddress)*suggestedSize,&offset);
#else
    pCoreDNSs = (RvAddress*)RPOOL_AlignAppend(pMgr->hGeneralPool,hServerPage,sizeof(RvAddress)*suggestedSize,&offset);
#endif
    if (NULL == pCoreDNSs)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "SipResolverMgrGetDnsServers - failed to get memory for core server list (rv=%d)",RV_ERROR_OUTOFRESOURCES));
        RPOOL_FreePage(pMgr->hGeneralPool,hServerPage);
        RvMutexUnlock(pMgr->pMutex,pMgr->pLogMgr); /*unlock the manager*/
        return RV_ERROR_OUTOFRESOURCES;
    }
#if defined(UPDATED_BY_SPIRENT)
    RVSIP_ADJUST_VDEVBLOCK(vdevblock);
    crv = RvAresGetParams(pMgr->pDnsEngine[vdevblock],NULL,NULL,pCoreDNSs,pNumOfDnsServers,NULL,NULL);
#else
    crv = RvAresGetParams(pMgr->pDnsEngine,NULL,NULL,pCoreDNSs,pNumOfDnsServers,NULL,NULL);
#endif
    if (RV_OK != crv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "SipResolverMgrGetDnsServers - failed to get Ares params (crv=%d)",CRV2RV(crv)));
        RPOOL_FreePage(pMgr->hGeneralPool,hServerPage);
        RvMutexUnlock(pMgr->pMutex,pMgr->pLogMgr); /*unlock the manager*/
        return CRV2RV(crv);
    }
    for (i=0;i<suggestedSize; i++)
    {
        SipTransportConvertCoreAddressToApiTransportAddr(&pCoreDNSs[i],&pDnsServers[i]);
    }
#ifdef UPDATED_BY_SPIRENT
    RPOOL_FreePage(pMgr->hDnsServersPool,hServerPage);
#else
    RPOOL_FreePage(pMgr->hGeneralPool,hServerPage);
#endif
    RvMutexUnlock(pMgr->pMutex,pMgr->pLogMgr); /*unlock the manager*/
    return rv;
}

/************************************************************************************
 * SipResolverMgrSetDnsDomains
 * ----------------------------------------------------------------------------------
 * General: Sets a new list of dns domains to the stack
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hRslvMgr          - Resolver mgr
 *          pDomainList      - a list of DNS domain. (an array of NULL terminated strings
 *                             to set to the stack
 *          numOfDomains     - The number of domains in the list
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV SipResolverMgrSetDnsDomains(
      IN  RvSipResolverMgrHandle   hRslvMgr,
      IN  RvChar**                    pDomainList,
      IN  RvInt32                     numOfDomains
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
)
{
    RvStatus        crv         = RV_OK;
    ResolverMgr* pMgr        = (ResolverMgr*)hRslvMgr;

    RvMutexLock(pMgr->pMutex,pMgr->pLogMgr); /*lock the manager*/
#if defined(UPDATED_BY_SPIRENT)
    RVSIP_ADJUST_VDEVBLOCK(vdevblock);
    crv = RvAresSetParams(pMgr->pDnsEngine[vdevblock],-1,-1,NULL,0,pDomainList,(RvInt)numOfDomains);
#else
    crv = RvAresSetParams(pMgr->pDnsEngine,-1,-1,NULL,0,pDomainList,(RvInt)numOfDomains);
#endif
    if (RV_OK != crv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipResolverMgrSetDnsDomains - failed to set Dns Domains (rv=%d)",CRV2RV(crv)))
    }
    RvMutexUnlock(pMgr->pMutex,pMgr->pLogMgr); /*unlock the manager*/
    return CRV2RV(crv);
}

/************************************************************************************
 * SipResolverMgrGetDnsDomains
 * ----------------------------------------------------------------------------------
 * General: Sets a new list of dns domains to the stack
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hRslvMgr          - Resolver mgr
 * Output:  pDomainList      - A list of DNS domains. (an array of char pointers).
 *                             that array will be filled with pointers to DNS domains.
 *                             The list of DNS domain is part of the stack memory, if
 *                             the application wishes to manipulate that list, It must copy
 *                             the strings to a different memory space.
 *                             The size of this array must be no smaller than numOfDomains
 *          numOfDomains     - The size of the pDomainList array, will reteurn the number
 *                             of domains set to the stack
 ***********************************************************************************/
RvStatus RVCALLCONV SipResolverMgrGetDnsDomains(
                      IN  RvSipResolverMgrHandle   hRslvMgr,
                      INOUT  RvChar**                    pDomainList,
                      INOUT  RvInt32*                    pNumOfDomains
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
)
{
    RvStatus        crv         = RV_OK;
    ResolverMgr* pMgr        = (ResolverMgr*)hRslvMgr;

    RvMutexLock(pMgr->pMutex,pMgr->pLogMgr); /*lock the manager*/
#if defined(UPDATED_BY_SPIRENT)
    RVSIP_ADJUST_VDEVBLOCK(vdevblock);
    crv = RvAresGetParams(pMgr->pDnsEngine[vdevblock],NULL,NULL,NULL,0,pDomainList,pNumOfDomains);
#else
    crv = RvAresGetParams(pMgr->pDnsEngine,NULL,NULL,NULL,0,pDomainList,pNumOfDomains);
#endif
    if (RV_OK != crv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipResolverMgrGetDnsDomains - failed to set Dns Domains (rv=%d)",CRV2RV(crv)))
    }
    RvMutexUnlock(pMgr->pMutex,pMgr->pLogMgr); /*unlock the manager*/
    return CRV2RV(crv);
}

#define MAX_DNS_SERVERS_TO_PRINT 5
#define MAX_DNS_DOMAINS_TO_PRINT 5
/************************************************************************************
 * SipResolverMgrPrintDnsParams
 * ----------------------------------------------------------------------------------
 * General: prints the ares DNS configuration
 * Return Value: -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr          - Resolver mgr
 * Output:  none
 ***********************************************************************************/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
void RVCALLCONV SipResolverMgrPrintDnsParams(
      IN  RvSipResolverMgrHandle   hRslvMgr)
{
    ResolverMgr* pMgr        = (ResolverMgr*)hRslvMgr;
    RvAddress   dnsServers[MAX_DNS_SERVERS_TO_PRINT];
    RvInt       nDnsServers     = MAX_DNS_SERVERS_TO_PRINT;
    RvChar*     dnsDomains[MAX_DNS_DOMAINS_TO_PRINT];
    RvInt       nDnsDomains     = MAX_DNS_DOMAINS_TO_PRINT;
    RvStatus    crv             = RV_OK;
    RvInt       i               = 0;
    RvChar      dnsBuff[RVSIP_TRANSPORT_LEN_STRING_IP];
    
#if defined(UPDATED_BY_SPIRENT)
    crv = RvAresGetParams(pMgr->pDnsEngine[0],NULL,NULL,dnsServers,&nDnsServers,dnsDomains,&nDnsDomains);
#else
    crv = RvAresGetParams(pMgr->pDnsEngine,NULL,NULL,dnsServers,&nDnsServers,dnsDomains,&nDnsDomains);
#endif
    if (RV_OK != crv)
    {
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipResolverMgrPrintDnsParams - Can't Print DNS data (rv=%d)",CRV2RV(crv)))
    }
    if (0 == nDnsServers)
    {
        RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipResolverMgrPrintDnsParams - Dns servers are not configured yet"))
    }
    for (i=0;i<RvMin(nDnsServers,MAX_DNS_SERVERS_TO_PRINT); i++)
    {
        RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipResolverMgrPrintDnsParams - Dns Server %d is %s:%u",i+1,
            RvAddressGetString(&dnsServers[i],RVSIP_TRANSPORT_LEN_STRING_IP,dnsBuff),
            RvAddressGetIpPort(&dnsServers[i])))
    }
    if (0 == nDnsDomains)
    {
        RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipResolverMgrPrintDnsParams - Dns domain are not configured yet"))
    }
    for (i=0;i<RvMin(nDnsDomains,MAX_DNS_SERVERS_TO_PRINT);i++)
    {
        RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SipResolverMgrPrintDnsParams - Dns Domain %d is %s",i+1,dnsDomains[i]))
    }
}
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */
    
