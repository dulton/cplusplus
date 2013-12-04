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
 *                              <_SipResolver.c>
 *
 * This file defines the Resolver internal API functions
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                    Dec 2004
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipResolverTypes.h"
#include "ResolverObject.h"
#include "ResolverDns.h"
#include "_SipTransport.h"

#if defined(UPDATED_BY_SPIRENT)
#include "TransportMgrObject.h"
#endif //SPIRENT

#include "RvSipAddress.h"
#include "_SipCommonConversions.h"
#include "rvstdio.h"
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * SipResolverDestruct
 * ------------------------------------------------------------------------
 * General: destruct the resolver.
 *          deletes the Ares Query
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hRslv -- The resolver handle
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV SipResolverDestruct(
                         IN RvSipResolverHandle       hRslv)
{
    Resolver*   pRslv = (Resolver*)hRslv;
    
    RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
        "SipResolverDestruct - pRslv=0x%p. destructing... (eState=%d)", pRslv, pRslv->eState));
       
    if(pRslv->eState != RESOLVER_STATE_UNDEFINED) /* did not detached from ARES yes */
    {
        ResolverDnsResetQueryRes(pRslv);
        pRslv->eState = RESOLVER_STATE_UNDEFINED;
    }

    if (NULL != pRslv->hQueryPage)
    {
        RPOOL_FreePage(pRslv->pRslvMgr->hGeneralPool,pRslv->hQueryPage);
        pRslv->hQueryPage= NULL_PAGE;
    }
    
    /*the transmitter should be removed from the manager list*/
    ResolverMgrRemoveResolver(pRslv->pRslvMgr,hRslv);
    return;
}

/***************************************************************************
 * SipResolverDetachFromAres
 * ------------------------------------------------------------------------
 * General: deletes the Ares Query
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hRslv -- The resolver handle
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV SipResolverDetachFromAres(
                         IN RvSipResolverHandle       hRslv)
{
    Resolver*   pRslv = (Resolver*)hRslv;
    
   RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
        "SipResolverDetachFromAres - pRslv=0x%p. destructing", pRslv));
   ResolverDnsResetQueryRes(pRslv);
   pRslv->eState = RESOLVER_STATE_UNDEFINED;
   
    return;
}

/***************************************************************************
 * SipResolverResolve
 * ------------------------------------------------------------------------
 * General: resolve a name according to a scheme
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hRslv          -- The resolver handle
 *           eMode          -- The resolving modus operandi
 *           strQueryString -- The string to resolve (e.g. "host.com")
 *                             For enum queries, it is the application's 
 *                             responsibility to keep the query string until 
 *                             the query has ended.
 *           eScheme        -- enumeration representing "im", "sip", etc...
 *           bIsSecure      -- is the resolution restricted to TLS only
 *           knownPort      -- the port for queries without port
 *           knownTransport -- the transport for queries without transport
 *           hDns           -- dns list for the answers
 *           pfnResolveCB   -- function to use for reporting results
 * Output:  (-)
 * Return:  RV_ERROR_TRY_AGAIN - the resolver has send a query and is waiting for response 
 *                              (this is a good error code)
 *          RV_ERROR_ILLEGAL_ACTION - another resolution is now in progress.
 *          note: this function should not return RV_OK
 ***************************************************************************/
RvStatus RVCALLCONV SipResolverResolve(
                         IN RvSipResolverHandle       hRslv,
                         IN RvSipResolverMode         eMode,
                         IN RvChar*                 strQueryString,
                         IN RvSipResolverScheme       eScheme,
                         IN RvBool                  bIsSecure,
                         IN RvUint16                knownPort,
                         IN RvSipTransport          knownTransport,
                         IN RvSipTransportDNSListHandle hDns,
                         IN RvSipResolverReportDataEv   pfnResolveCB
#if defined(UPDATED_BY_SPIRENT)
			 , IN RvSipTransportAddr             *localAddr
#endif
)
{
    RvStatus    rv = RV_OK;
    Resolver*   pRslv = (Resolver*)hRslv;
    
    RvLogDebug(pRslv->pRslvMgr->pLogSrc ,(pRslv->pRslvMgr->pLogSrc,
        "SipResolverResolve - Resolver 0x%p: (mode=%s,qstr=%s,scheme=%s,bIsSecure=%d,port=%u, transport=%s)",
        hRslv,ResolverGetModeName(eMode),strQueryString,
        ResolverGetSchemeName(eScheme), bIsSecure,knownPort,
        SipCommonConvertEnumToStrTransportType(knownTransport)));

    if (pRslv->eState != RESOLVER_STATE_UNDEFINED)
    {
        RvLogError(pRslv->pRslvMgr->pLogSrc ,(pRslv->pRslvMgr->pLogSrc,
            "SipResolverResolve - Resolver 0x%p: is busy",
            hRslv));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    pRslv->pfnReport        = pfnResolveCB;
    pRslv->strQueryString   = strQueryString;
    pRslv->eScheme          = eScheme;
    pRslv->eState           = RESOLVER_STATE_UNDEFINED;
    pRslv->knownPort        = knownPort;
    pRslv->knownTransport   = knownTransport;
    pRslv->hDns             = hDns;
    pRslv->eMode            = eMode;
    pRslv->bIsSecure		= bIsSecure;

#if defined(UPDATED_BY_SPIRENT)
    RVSIP_ADJUST_VDEVBLOCK(localAddr->vdevblock);
    pRslv->vdevblock = localAddr->vdevblock;
    strncpy(pRslv->if_name,localAddr->if_name,sizeof(pRslv->if_name));
    pRslv->if_name[sizeof(pRslv->if_name)-1] = '\0';
    TransportMgrConvertTransportAddr2RvAddress(NULL, localAddr, &pRslv->localAddr);
#endif
    
    switch(eMode)
    {
    case RVSIP_RESOLVER_MODE_FIND_TRANSPORT_BY_NAPTR:
        rv = ResolverDnsResolveProtocolByNAPTR(pRslv);
        break;
    case RVSIP_RESOLVER_MODE_FIND_TRANSPORT_BY_3WAY_SRV:
        rv = ResolverDnsResolveProtocolBy3WaySrv(pRslv);
        break;
    case RVSIP_RESOLVER_MODE_FIND_HOSTPORT_BY_SRV_STRING:
        rv = ResolverDnsResolveHostPortBySrvString(pRslv);
        break;
    case RVSIP_RESOLVER_MODE_FIND_IP_BY_HOST:
        rv = ResolverDnsResolveIpByHost(pRslv);
        break;
    case RVSIP_RESOLVER_MODE_FIND_URI_BY_NAPTR:
        rv = ResolverDnsResolveURIByNAPTR(pRslv);
        break;
    case RVSIP_RESOLVER_MODE_FIND_HOSTPORT_BY_TRANSPORT:
        rv = ResolverDnsResolveHostPortByTransport(pRslv);
        break;
    default:
        RvLogExcep(pRslv->pRslvMgr->pLogSrc ,(pRslv->pRslvMgr->pLogSrc,
            "SipResolverResolve - Resolver 0x%p: mode unknown=%d",
            hRslv,eMode));
        pRslv->pfnReport        = NULL;
        pRslv->strQueryString   = NULL;
        pRslv->eScheme          = RVSIP_RESOLVER_SCHEME_UNDEFINED;
        return RV_ERROR_BADPARAM;
    }
	
	if (RV_ERROR_TRY_AGAIN == rv)
	{
		RvLogDebug(pRslv->pRslvMgr->pLogSrc ,(pRslv->pRslvMgr->pLogSrc,
            "SipResolverResolve - Resolver 0x%p: now resolving host %s",
            hRslv,strQueryString));
	}
	else if (RV_DNS_ERROR_NOTFOUND == rv)
	{
		RvLogDebug(pRslv->pRslvMgr->pLogSrc ,(pRslv->pRslvMgr->pLogSrc,
            "SipResolverResolve - Resolver 0x%p: got negative DNS caching response for host %s",
            hRslv,strQueryString));
	}
    else
    {
        RvLogError(pRslv->pRslvMgr->pLogSrc ,(pRslv->pRslvMgr->pLogSrc,
            "SipResolverResolve - Resolver 0x%p: can not resolve host %s",
            hRslv,strQueryString));
    }
    
    return rv;
}

#ifdef RV_SIP_TEL_URI_SUPPORT
/************************************************************************************
 * SipResolverCreateAUSFromTel
 * ---------------------------------------------------------------------------
 * General: Creates an Application unique string from a TEL URL according to RFC3761 (2.4)
 * Return Value: RvStatus - RvStatus
 * Arguments:
 * Input:   hTel     - address to use for URL resolution
 * output:  strENUM  - formatted string for NAPTR queries
 * Return:  RvStatus
 ***********************************************************************************/
RvStatus RVCALLCONV SipResolverCreateAUSFromTel(
             IN    RvSipAddressHandle hTel,
             OUT   RvChar*          strENUM)
{
    RvChar      strPhone[MAX_HOST_NAME_LEN+1];
    RvChar*     pstrPhone   = strPhone;
    RvUint      len         = MAX_HOST_NAME_LEN+1;
    RvUint      i           = 0;
    RvUint      strlocation = 0;
    RvStatus    rv          = RV_OK;

    pstrPhone[0] = '\0';
    if (RV_TRUE == RvSipAddrTelUriGetIsGlobalPhoneNumber(hTel))
    {
        pstrPhone[0] ='+';
        pstrPhone[1] ='\0';
        pstrPhone++;
        len -= 1;
    }
    rv = RvSipAddrTelUriGetPhoneNumber(hTel,pstrPhone,len,&len);
    if (RV_OK != rv)
    {
        strENUM[0] = '\0';
        return rv;
    }
    for (i=0;i<len;i++)
    {
        if (isdigit((RvInt)strPhone[i]) || '+' == strPhone[i])
        {
            strENUM[strlocation++]=strPhone[i];
        }
    }
    strENUM[strlocation++]='\0';

    return RV_OK;
}
#endif /*#ifdef RV_SIP_OTHER_URI_SUPPORT*/

/***************************************************************************
 * SipResolverSetTripleLock
 * ------------------------------------------------------------------------
 * General: Sets the resolver triple lock.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hRslv - The resolver handle
 *           pTripleLock - The new triple lock
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV SipResolverSetTripleLock(
                         IN RvSipResolverHandle hRslv,
                         IN SipTripleLock*      pTripleLock)
{
	ResolverSetTripleLock((Resolver*)hRslv,pTripleLock);
}
