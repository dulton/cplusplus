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
 *                              <RvSipResolver.c>
 *
 * The reoslver functions enable the user to query a DNS server with several DNS
 * Queries: NAPTR, SRV, A and AAAA.
 * The data retrieved from the DNS answer will be stored in a DNS list the user 
 * supplies
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra W.                       May 2005
 *    Udi Tirosh                    May 2005
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipResolverTypes.h"
#include "RvSipTransport.h"
#include "_SipResolverMgr.h"
#include "ResolverMgrObject.h"
#include "ResolverObject.h"
#include "_SipResolver.h"
#include "_SipCommonUtils.h"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*                        RESOLVER MANAGER API FUNCTIONS                 */
/*-----------------------------------------------------------------------*/
/************************************************************************************
 * RvSipResolverMgrSetDnsServers
 * ----------------------------------------------------------------------------------
 * General: Sets a new list of DNS servers to the SIP Stack.
 *          In the process of address resolution, host names are resolved 
 *          to IP addresses by sending DNS queries to the SIP Stack DNS servers. 
 *          If one server fails, the next DNS server on the list is queried. 
 *          When the SIP Stack is constructed, a DNS list is set to the SIP 
 *          Stack using the computer configuration. The application can use
 *          this function to provide a new set of DNS servers to the SIP Stack
 * Note: This function replaces the current list of servers that the SIP Stack 
 *       is using.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hRslvMgr          - resolver mgr
 *          pDnsServers      - A list of addresses of DNS servers to set to the SIP Stack.
 *          numOfDnsServers  - The number of DNS servers in the list.
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipResolverMgrSetDnsServers(
      IN  RvSipResolverMgrHandle   hRslvMgr,
      IN  RvSipTransportAddr*       pDnsServers,
      IN  RvInt32                   numOfDnsServers
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
)
{
    RvStatus        rv          = RV_OK;
    ResolverMgr*    pMgr        = (ResolverMgr*)hRslvMgr;

    if (NULL == pMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pDnsServers || 0 == numOfDnsServers)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipResolverMgrSetDnsServers - bad params to function. pDnsServers=0x%p, numOfDnsServers=%d (rv=%d)",
             pDnsServers,numOfDnsServers, RV_ERROR_BADPARAM));
        return RV_ERROR_BADPARAM;
    }
    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
        "RvSipResolverMgrSetDnsServers - pDnsServers=0x%p, numOfDnsServers=%d",
        pDnsServers,numOfDnsServers));
#if defined(UPDATED_BY_SPIRENT)
    rv = SipResolverMgrSetDnsServers(hRslvMgr,pDnsServers,numOfDnsServers,vdevblock);
#else
    rv = SipResolverMgrSetDnsServers(hRslvMgr,pDnsServers,numOfDnsServers);
#endif

    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipResolverMgrSetDnsServers - failed to set DNS servers"));
    }
    else
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipResolverMgrSetDnsServers - DNS servers set successfully"));
    }
    return rv;
}

#if defined(UPDATED_BY_SPIRENT)
RVAPI RvStatus RVCALLCONV RvSipResolverMgrSetTimeout(
      IN  RvSipResolverMgrHandle   hRslvMgr,
      IN  RvInt timeout,
      IN  RvInt tries,
      IN  RvUint16 vdevblock
)
{
    RvStatus        rv          = RV_OK;
    ResolverMgr*    pMgr        = (ResolverMgr*)hRslvMgr;

    if (NULL == pMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipResolverMgrSetTimeout(hRslvMgr,timeout,tries,vdevblock);

    return rv;
}
#endif

/************************************************************************************
 * RvSipResolverMgrGetDnsServers
 * ----------------------------------------------------------------------------------
 * General: Gets the list of the DNS servers that the SIP Stack is using. 
 *          You can use this function to retrieve the DNS servers with which the SIP 
 *          Stack was initialized.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hRslvMgr          - Resolver mgr
 * Output:  pDnsServers      - An empty list of DNS servers that will be filled 
 *                             with the DNS servers that the SIP Stack is currently using.
 *          pNumOfDnsServers - The size of pDnsServers. 
 *                             This value will be updated with the actual size of the
 *                             DNS servers list.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipResolverMgrGetDnsServers(
      IN  RvSipResolverMgrHandle   hRslvMgr,
      IN  RvSipTransportAddr*         pDnsServers,
      INOUT  RvInt32*                 pNumOfDnsServers
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
)
{
    RvStatus        rv          = RV_OK;
    ResolverMgr*    pMgr        = (ResolverMgr*)hRslvMgr;

    if (NULL == pMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pDnsServers || NULL == pNumOfDnsServers)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipResolverMgrGetDnsServers - bad params to function. pDnsServers=0x%p, pNumOfDnsServers=0x%p (rv=%d)",
             pDnsServers,pNumOfDnsServers, RV_ERROR_BADPARAM));
        return RV_ERROR_BADPARAM;
    }
#if defined(UPDATED_BY_SPIRENT)
    rv = SipResolverMgrGetDnsServers(hRslvMgr,pDnsServers,pNumOfDnsServers,vdevblock);
#else
    rv = SipResolverMgrGetDnsServers(hRslvMgr,pDnsServers,pNumOfDnsServers);
#endif
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipResolverMgrGetDnsServers - failed to get DNS servers"));
    }
    else
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipResolverMgrGetDnsServers - DNS servers got seccesfully"));
    }
    return rv;
}

/************************************************************************************
 * RvSipResolverMgrRefreshDnsServers
 * ----------------------------------------------------------------------------------
 * General: Forces a refreshing of the list of DNS servers in the SIP Stack.
 *          This function will be called generally by the application when a new
 *          list of DNS servers has been configured in the system outside of the stack.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hRslvMgr          - Resolver mgr
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipResolverMgrRefreshDnsServers(
							IN  RvSipResolverMgrHandle   hRslvMgr)
{
    RvStatus        rv          = RV_OK;
    ResolverMgr*    pMgr        = (ResolverMgr*)hRslvMgr;
    
    if (NULL == pMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

#if defined(UPDATED_BY_SPIRENT)
 {
   int i=0;
   for(i=0;i<RVSIP_MAX_NUM_VDEVBLOCK;i++) {
     rv |= RvAresConfigure(pMgr->pDnsEngine[i],RV_DNS_SERVERS);
   }
 }
#else
    rv = RvAresConfigure(pMgr->pDnsEngine,RV_DNS_SERVERS);
#endif

    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipResolverMgrRefreshDnsServers - failed to refresh the list of DNS servers"));
    }
    else
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipResolverMgrRefreshDnsServers - list of DNS servers refreshed successfully"));
    }
    return rv;
}

/************************************************************************************
 * RvSipResolverMgrSetDnsDomains
 * ----------------------------------------------------------------------------------
 * General: Sets a new list of DNS domains to the SIP Stack. 
 * The SIP Stack provides Domain Suffix Search Order capability.
 * The Domain Suffix Search Order specifies the DNS domain suffixes to be
 * appended to the host names during name resolution. When attempting to resolve
 * a fully qualified domain name (FQDN) from a host that includes a name only,
 * the system will first append the local domain name to the host name and will
 * query DNS servers. If this is not successful, the system will use the Domain
 * Suffix list to create additional FQDNs in the order listed, and will query DNS
 * servers for each. When the SIP Stack initializes, the DNS domain list is set
 * according to the computer configuration. The application can use this function
 * to provide a new set of DNS domains to the SIP Stack.
 * Note: This function replaces the current Domain list that the SIP Stack is using.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hRslvMgr         - The Resolver mgr
 *          pDomainList      - A list of DNS domains (an array of NULL terminated 
 *                             strings to set to the SIP Stack).
 *          numOfDomains     - The number of domains in the list
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipResolverMgrSetDnsDomains(
      IN  RvSipResolverMgrHandle   hRslvMgr,
      IN  RvChar**                    pDomainList,
      IN  RvInt32                     numOfDomains
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
)
{
    RvStatus        rv          = RV_OK;
    ResolverMgr*    pMgr        = (ResolverMgr*)hRslvMgr;

    if (NULL == pMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pDomainList || 0 == numOfDomains)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipResolverMgrSetDnsDomains - bad params to function. pDnsServers=0x%p, pNumOfDnsServers=0x%p (rv=%d)",
             pDomainList,numOfDomains, RV_ERROR_BADPARAM));
        return RV_ERROR_BADPARAM;
    }
#if defined(UPDATED_BY_SPIRENT)
    rv = SipResolverMgrSetDnsDomains(hRslvMgr,pDomainList,numOfDomains,vdevblock);
#else
    rv = SipResolverMgrSetDnsDomains(hRslvMgr,pDomainList,numOfDomains);
#endif

    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipResolverMgrSetDnsDomains - failed to set DNS domains"));
    }
    else
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipResolverMgrSetDnsDomains - DNS domain set seccesfully"));
    }
    return rv;
}

/************************************************************************************
 * RvSipResolverMgrGetDnsDomains
 * ----------------------------------------------------------------------------------
 * General: Gets the list of DNS domains from the SIP Stack.
 * This function is useful for determining the list of DNS domains with which the
 * SIP Stack was initialized. To learn more about DNS domains, refer to
 * RvSipResolverMgrSetDnsDomains().
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hRslvMgr         - The Resolver mgr
 * Output:  pDomainList      - A list of DNS domains (an array of "char" pointers). This array will be filled
 *                             with pointers to the DNS domains. 
 *                             The list of DNS domains is part of the SIP Stack 
 *                             memory. If the application wishes to manipulate 
 *                             this list, it must copy the strings to a different 
 *                             memory space. The size of this array must not be
 *                             smaller than numOfDomains.
 *          numOfDomains     - The size of the pDomainList array. 
 *                             The Resolver Manager will update this
 *                             parameter with the actual number of domains set in the list.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipResolverMgrGetDnsDomains(
                      IN  RvSipResolverMgrHandle   hRslvMgr,
                      INOUT  RvChar**                    pDomainList,
                      INOUT  RvInt32*                    pNumOfDomains
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
)
{
    RvStatus        rv          = RV_OK;
    ResolverMgr*    pMgr        = (ResolverMgr*)hRslvMgr;

    if (NULL == pMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pDomainList || NULL == pNumOfDomains)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipResolverMgrGetDnsDomains - bad params to function. pDomainList=0x%p, pNumOfDomains=0x%p (rv=%d)",
             pDomainList,pNumOfDomains, RV_ERROR_BADPARAM));
        return RV_ERROR_BADPARAM;
    }
#if defined(UPDATED_BY_SPIRENT)
    rv = SipResolverMgrGetDnsDomains(hRslvMgr,pDomainList,pNumOfDomains,vdevblock);
#else
    rv = SipResolverMgrGetDnsDomains(hRslvMgr,pDomainList,pNumOfDomains);
#endif

    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipResolverMgrGetDnsDomains - failed to get DNS domains"));
    }
    else
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipResolverMgrGetDnsDomains - DNS domain got seccesfully"));
    }
    return rv;
}

/***************************************************************************
 * RvSipResolverMgrCreateResolver
 * ------------------------------------------------------------------------
 * General: Creates a new Resolver and exchange handles with the
 *          application.  The newly created Resolver is ready to send
 *          DNS queries when RvSipResolverResolve() is called.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRslvMgr - Handle to the Resolver manager
 *            hAppRslv - Application handle to the newly created Resolver.
 * Output:    phRslv   -   SIP stack handle to the Resolver
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipResolverMgrCreateResolver(
                   IN  RvSipResolverMgrHandle   hRslvMgr,
                   IN  RvSipAppResolverHandle   hAppRslv,
                   OUT RvSipResolverHandle*     phRslv)
{
    RvStatus       rv      = RV_OK;
    ResolverMgr*   pRslvMgr = (ResolverMgr*)hRslvMgr;

    *phRslv = NULL;

    RvMutexLock(pRslvMgr->pMutex,pRslvMgr->pLogMgr);

    RvLogInfo(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
              "RvSipResolverMgrCreateResolver - Creating a new Resolver"));

    /*create a new Resolver. This will lock the Resolver manager*/
    rv = ResolverMgrCreateResolver(pRslvMgr,RV_TRUE,hAppRslv,NULL,phRslv);
    if(rv != RV_OK)
    {
        RvLogError(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
              "RvSipResolverMgrCreateResolver - Failed, ResolverMgr failed to create a new Resolver (rv=%d:%s)",
              rv, SipCommonStatus2Str(rv)));
        RvMutexUnlock(pRslvMgr->pMutex,pRslvMgr->pLogMgr);
        return rv;
    }

    RvLogInfo(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
              "RvSipResolverMgrCreateResolver - New Resolver 0x%p was created successfully",*phRslv));
    RvMutexUnlock(pRslvMgr->pMutex,pRslvMgr->pLogMgr);
    return RV_OK;

}

/************************************************************************************
 * RvSipResolverMgrClearDnsCache
 * ----------------------------------------------------------------------------------
 * General: This function resets the SIP Stack DNS cache.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pRslv      - Resolver
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipResolverMgrClearDnsCache(IN RvSipResolverMgrHandle hRslvMgr)
{
	return ResolverMgrClearDnsCache((ResolverMgr*) hRslvMgr);
}
#if defined(UPDATED_BY_SPIRENT)
/************************************************************************************
 * RvSipResolverMgrClearDnsEngineCache
 * ----------------------------------------------------------------------------------
 * General: This function resets the SIP Stack DNS Engine cache.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pRslv      - Resolver
 *      :   vdevblock  - virtual device block to which dns engine belongs
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipResolverMgrClearDnsEngineCache(IN RvSipResolverMgrHandle hRslvMgr,IN RvUint16 vdevblock)
{
	return ResolverMgrClearDnsEngineCache((ResolverMgr*) hRslvMgr,vdevblock);
}
#endif

/************************************************************************************
 * RvSipResolverMgrDumpDnsCache
 * ----------------------------------------------------------------------------------
 * General: This function dumps the SIP Stack DNS cache data into a file.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input: hRslvMgr    - Resolver manager
 *        strDumpFile - The name of the file to dump the cache data into
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipResolverMgrDumpDnsCache(
										IN RvSipResolverMgrHandle  hRslvMgr,
										IN RvChar                 *strDumpFile)
{
	return ResolverMgrDumpDnsCache((ResolverMgr*) hRslvMgr,strDumpFile);
}

/***************************************************************************
 * RvSipResolverTerminate
 * ------------------------------------------------------------------------
 * General: Terminate a Resolver object. If the resolver has a pending query, 
 *          the query is canceled and a result will not be reported.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hRslv - The Resolver handle
 * Output:  (-)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipResolverTerminate(
                         IN RvSipResolverHandle       hRslv)
{
    Resolver*   pRslv = (Resolver*)hRslv;
    RvStatus rv = RV_OK;

    if(NULL == hRslv)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    rv = ResolverLockAPI(pRslv);
    if (RV_OK != rv)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
        "RvSipResolverTerminate - pRslv=0x%p. terminating", pRslv));
    SipResolverDestruct(hRslv);

    ResolverUnLockAPI(pRslv);
 
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pRslv);
#endif

	return RV_OK;
}

/***************************************************************************
 * RvSipResolverGetDnsList
 * ------------------------------------------------------------------------
 * General: Gets the DNS list from the Resolver object. The list contains
 *          the results for the DNS queries.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hRslv -- The resolver handle
 * Output:  phdns -- The list handle of the DNS.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipResolverGetDnsList(
                         IN RvSipResolverHandle       hRslv,
                         OUT RvSipTransportDNSListHandle *phdns)
{
    Resolver*   pRslv = (Resolver*)hRslv;
    RvStatus rv = RV_OK;

    if(NULL == hRslv || NULL == phdns)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    rv = ResolverLockAPI(pRslv);
    if (RV_OK != rv)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
        "RvSipResolverGetDnsList - pRslv=0x%p. getting hdns list",pRslv));
    *phdns = pRslv->hDns;
    ResolverUnLockAPI(pRslv);
    return RV_OK;
}

/***************************************************************************
 * RvSipResolverResolve
 * ------------------------------------------------------------------------
 * General: Resolves a name according to a scheme. When calling this function
 * the resolver object starts a DNS algorithm. The algorithm will try to obtain 
 * information from the configured DNS server using one or more DNS queries.
 * The different algorithms are defined in the eMode parameter that is supplied
 * to this function. For example, if the supplied mode is 
 * RVSIP_RESOLVER_MODE_FIND_IP_BY_HOST the resolver will try to find the IP 
 * of a specific host. The resolver will try both IPv4 and IPv6 
 * using A and AAAA queries respectively.
 *
 * The data retrieved from the DNS server is stored in the Dns list supplied
 * to this function as a parameter.
 * Since DNS activities are a-syncrounous, the resolver will notify the DNS
 * query results using a callback function. The callback function must also be
 * supplied to this function.
 * Note: A Single resolver object can perform only a single Algorithm at any
 * given time.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hRslv          -- The resolver handle
 *           eMode          -- The resolving mode that specifies the DNS
 *                             algorithm that will be activated.
 *           strQueryString -- The string to resolve (e.g. "host.com")
 *                             It is the application's 
 *                             responsibility to keep the query string until 
 *                             the query has ended.
 *           eScheme        -- The required scheme such as "im", "sip", etc...
 *           bIsSecure      -- Is the resolution restricted to TLS only. You can 
 *                             use this parameter to indicate that the any 
 *                             record that does not represent a secure 
 *                             transport will be discarded.
 *           knownPort      -- Port for queries that do not retrieve the port.
 *                             This port will be placed in the DNS list along with
 *                             the DNS results. for example, A records do not contain port.
 *           knownTransport -- Transport for queries that do not retrieve the transport
 *                             This transport will be placed in the DNS list along with
 *                             the DNS results.For example, SRV records do
 *                             not contain transport
 *           hDns           -- A DNS list. The resolver will place all DNS
 *                             results in this list along with additional information such
 *                             as knownPort and knownTransport received as parameters to this
 *                             function.
 *           pfnResolveCB   -- Function pointer that will be used by the 
 *                             resolver for reporting results.
 * Output:  (-)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipResolverResolve(
                         IN RvSipResolverHandle     hRslv,
                         IN RvSipResolverMode         eMode,
                         IN RvChar*                 strQueryString,
                         IN RvSipResolverScheme       eScheme,
                         IN RvBool                  bIsSecure,
                         IN RvUint16                knownPort,
                         IN RvSipTransport          knownTransport,
                         IN RvSipTransportDNSListHandle hDns,
                         IN RvSipResolverReportDataEv   pfnResolveCB
#if defined(UPDATED_BY_SPIRENT)
		    , IN RvSipTransportAddr *localAddr
#endif
)
{
    RvStatus    rv = RV_OK;
    Resolver*   pRslv = (Resolver*)hRslv;
    
    if(NULL == hRslv)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
	if (RVSIP_RESOLVER_MODE_UNDEFINED == eMode) {
		RvLogError(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
			"RvSipResolverResolve - pRslv=0x%p, cannot resolved in UNDEFINED query mode",pRslv));
		return RV_ERROR_BADPARAM;
	}

    rv = ResolverLockAPI(pRslv);
    if (RV_OK != rv)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
        "RvSipResolverResolve - pRslv=0x%p. resolving...",
        pRslv));

    rv = SipResolverResolve(hRslv,eMode,strQueryString,eScheme,bIsSecure,knownPort,knownTransport,hDns,pfnResolveCB
#if defined(UPDATED_BY_SPIRENT)
		    ,localAddr 
#endif
);
    
    if(rv == RV_ERROR_TRY_AGAIN)
	{
		rv = RV_OK;
	}
	ResolverUnLockAPI(pRslv);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pRslv);
#endif

    return rv;
}

/************************************************************************************
 * RvSipResolverMgrGetDnsEngine
 * ----------------------------------------------------------------------------------
 * General: Gets a pointer to the SIP Stack DNS engine.
 *          This function should be used only in order to supply the stack DNS engine
 *          to the STUN add-on.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hRslvMgr         - The Resolver mgr
 * Output:  pDnsEngine       - Pointer to the DNS engine.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipResolverMgrGetDnsEngine(
                      IN     RvSipResolverMgrHandle   hRslvMgr,
                      OUT    void**                    pDnsEngine
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
)
{
    ResolverMgr*    pMgr        = (ResolverMgr*)hRslvMgr;

    if (NULL == pMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

#if defined(UPDATED_BY_SPIRENT)
    RVSIP_ADJUST_VDEVBLOCK(vdevblock);
    *pDnsEngine = (void*)(pMgr->pDnsEngine[vdevblock]);
#else
    *pDnsEngine = (void*)(pMgr->pDnsEngine);
#endif

    return RV_OK;
}


