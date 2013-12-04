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
 *                              <ResolverDns.c>
 *
 *  Contains functions for DNS operations
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                    JAN 2005
 *********************************************************************************/
/*-----------------------------------------------------------------------*/
/*                        MACRO DEFINITIONS                              */
/*-----------------------------------------------------------------------*/
#define RV_SIP_MODULE_RESOLVER_DNS 1025
#define RvSipResolverDnsErrorCode(_e) RvErrorCode(RV_ERROR_LIBCODE_SIP, RV_SIP_MODULE_RESOLVER_DNS, (_e))
/* SRV Support */
#define SRV_UDP_PREFIX                  "_udp."
#define SRV_TCP_PREFIX                  "_tcp."
#define SRV_TLS_PREFIX                  "_tcp."
#define SRV_SCTP_PREFIX                 "_sctp."

#define SRV_SIPS_SCHEME                 "_sips."
#define SRV_SIP_SCHEME                  "_sip."
#define SRV_PRES_SCHEME                 "_pres."
#define SRV_IM_SCHEME                   "_im."


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "ResolverObject.h"
#include "ResolverDns.h"
#include "_SipTransport.h"
#include "ResolverCallbacks.h"
#include "RvSipMsgTypes.h"
#include "RvSipTransportDNS.h"
#include "rvansi.h"
#include "rvstdio.h"
#include "_SipCommonUtils.h"
/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static RvChar* RVCALLCONV GetAresDataTypeName(IN RvDnsQueryType eDataType);
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */
static RvStatus RVCALLCONV AllocateAndSendQuery(
                            IN  Resolver* pRslv,
                            IN RvDnsQueryType dnsQuery,
                            IN RvChar* dnsName,
                            IN RvBool bAsIs);

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
static RvStatus RVCALLCONV DnsGetNextSrvTransportAndState(
                            IN  Resolver* pRslv,
                            IN  ResolverState eState,
                            OUT ResolverState*  peNewState,
                            OUT RvSipTransport* peTransport);

static void RVCALLCONV DnsCreateSrvName(
             IN    RvSipResolverScheme eScheme,
             IN    RvChar*           strHostName,
             IN    RvSipTransport    eTransport,
             IN    RvBool            bIsSecure,
			 IN    RvSize_t          strSrvNameLen,
             OUT   RvChar*           strSrvName);
#endif /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/

static RvStatus DnsLockByQueryId(IN Resolver* pRslv,
                                 IN RvUint32     queryId);


static RvStatus ResolverHandleEndOfQuery(IN Resolver* pRslv,
                                         IN RvDnsQueryType eStatus);

static RvStatus ResolverHandleFailure(IN Resolver* pRslv);

static RvStatus ResolverHandleNewNaptr(IN Resolver* pRslv,
                                       IN RvDnsData* pDnsData);

static RvStatus RVCALLCONV DnsCreateENUMQueryFromAus(
             IN    ResolverMgr* pRslvMgr,
             IN    RvChar*      strAus,
             IN    RvInt        bufLen,    
             OUT   RvChar*      strENUM);

static RvStatus RVCALLCONV DnsValidateAndExtractAddr(
             IN Resolver* pRslv ,
             IN RvDnsData* pDnsData);


/*-----------------------------------------------------------------------*/
/*                           FILE FUNCTIONS                              */
/*-----------------------------------------------------------------------*/

/*****************************************************************************
* ResolverDNSNewRecordCB
* ---------------------------------------------------------------------------
* General: Inserts a new DNS record data to the appropriate list.
*          lock on first record and unlocks after the last record.
*          continues the DNS resolution process
* Return Value: RvStatus
*                           RV_ERROR_INVALID_HANDLE - the trx was invalid
*                           RV_ERROR_DESTRUCTED     - the trx was allredy destracted
*                           RV_ERROR_OUTOFRANGE     - To many DNS records
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pRslvVoid    - the resolver as a void pointer
*          queryId     - the Id the query was sent with
*          pDnsData    - structure of type RvDnsData with a new record data to add.
*               the dataType member indicates the type of data (IPv4/6, SRV or NAPTR)
*               in the structure
* Output:  -
*****************************************************************************/
RvStatus ResolverDNSNewRecordCB (
    IN  void*               pRslvVoid,
    IN  RvUint32            queryId,
    IN  RvDnsData*          pDnsData)
{
    Resolver*            pRslv           = (Resolver*)pRslvVoid;
    RvStatus             rv              = RV_OK;
    ResolverMgr*         pMgr            = NULL;


    if (pRslv == NULL)
    {
        return  RvSipResolverDnsErrorCode(RV_ERROR_INVALID_HANDLE);
    }

    /* locking                                  unlocking
       (a) failure notification                      +
       (b) first record of any query                 when reaching end of list
     */
    if ( /*a*/(RV_DNS_STATUS_TYPE == pDnsData->dataType) /*a*/||
         /*b*/( (RV_DNS_HOST_IPV4_TYPE==pDnsData->dataType || RV_DNS_HOST_IPV6_TYPE==pDnsData->dataType ||
                RV_DNS_NAPTR_TYPE==pDnsData->dataType || RV_DNS_SRV_TYPE==pDnsData->dataType) &&
                1 == pDnsData->recordNumber) /*b*/)
    {
        /* we are starting to process an answer, lock the Resolver */
        rv = DnsLockByQueryId(pRslv,queryId);
        if (RV_OK != rv)
        {
            return RvSipResolverDnsErrorCode(RV_ERROR_DESTRUCTED);
        }

        /* the ARES engine no longer need send buff, we can release it */
        ResolverDnsResetQueryRes(pRslv);
    }

    pMgr = pRslv->pRslvMgr;

    /* If we need to process more records than we are "allowed" to,
       inform Ares to stop processing.
    */
    if (RV_DNS_HOST_IPV4_TYPE==pDnsData->dataType || 
        RV_DNS_HOST_IPV6_TYPE==pDnsData->dataType ||
        RV_DNS_SRV_TYPE==pDnsData->dataType)
    {
        if (pDnsData->recordNumber > (RvInt)SipTransportGetMaxDnsElements(pRslv->pRslvMgr->hTransportMgr))
        {
            RvLogWarning(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                "ResolverDNSNewRecordCB - pRslv=0x%p: there are more records, but processing is restricted to %u records",
                pRslv, SipTransportGetMaxDnsElements(pRslv->pRslvMgr->hTransportMgr)));

			/* Once RvSipResolverDnsErrorCode(RV_ERROR_OUTOFRANGE) was returned */
			/* However, the ares module does NOT finish with END_OF_LIST record */
			/* when a non-RV_OK value is returned from this callback. Thus, a   */
			/* RV_OK value is returned here until an END_OF_LIST is accepted    */
			/* from ares.														*/
		    /* NOTE: The ares module keeps on calling this callback as long as  */
			/*       there are available query records, replied by the DNS      */ 
            return RV_OK;
        }
    }

    RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
              "ResolverDNSNewRecordCB - pRslv=0x%p: Got type type %s", pRslv, GetAresDataTypeName(pDnsData->dataType)));

    switch (pDnsData->dataType)
    {
    case RV_DNS_HOST_IPV4_TYPE:
    case RV_DNS_HOST_IPV6_TYPE:
        rv = SipTransportDnsAddIPRecord(pMgr->hTransportMgr,pDnsData,pRslv->knownTransport,
            pRslv->knownPort,pRslv->hDns);
        if (RV_OK != rv)
        {
            /*DnsPortIPStateChange(pRslv,Resolver_DNS_IP_PORT_STATE_GENERAL_FAILURE);*/
        }
        break;
    case RV_DNS_NAPTR_TYPE:
        rv = ResolverHandleNewNaptr(pRslv,pDnsData);
        break;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    case RV_DNS_SRV_TYPE:
        rv = SipTransportDnsAddSRVRecord(pMgr->hTransportMgr,pDnsData,pRslv->knownTransport,pRslv->hDns);
        break;
#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/

    case RV_DNS_ENDOFLIST_TYPE:
    case RV_DNS_STATUS_TYPE:
	case RV_DNS_NO_DATA_TYPE:
        /* The DNS query failed */
        ResolverHandleEndOfQuery(pRslv,pDnsData->dataType);
        ResolverUnLockEvent(pRslv); /*c*/
        break;
    default:
        break;
    }

    if (rv != RV_OK)
    {
        return RvSipResolverDnsErrorCode(RV_ERROR_UNKNOWN);
    }

    return RV_OK;
}

/*****************************************************************************
* ResolverDnsResolveIpByHost
* ---------------------------------------------------------------------------
* General: makes decisions as to where to go for resolving in the 
*          RVSIP_RESOLVER_MODE_FIND_IP_BY_HOST mode:
*          first try IPv4 (A) -> try IPv6 (AAAA) -> Fail
* Return Value: RvStatus - RvStatus
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pRslv        - Transport Resolver
* Output:  none
*****************************************************************************/
RvStatus ResolverDnsResolveIpByHost(IN Resolver* pRslv)
{
    RvDnsQueryType  eNextQuery   = RV_DNS_UNDEFINED;
    ResolverState   eNextState   = RESOLVER_STATE_UNDEFINED;
    RvStatus        rv           = RV_OK;
    RvUint32        numIpv4      = 0;
	RvUint32		numIPv6		 = 0;
	RvBool          bLookForIpv4 = RV_FALSE;
	RvBool          bLookForIpv6 = RV_FALSE;
	RvInt32			transportType;
    
    /* checking what transports we can use*/
	for (transportType = 0; 
		 transportType < (int) RVSIP_TRANSPORT_OTHER && 
			 (bLookForIpv4 == RV_FALSE || bLookForIpv6 == RV_FALSE);
		 transportType++) { 
		SipTransportMgrCounterLocalAddrsGet(pRslv->pRslvMgr->hTransportMgr,(RvSipTransport)transportType,&numIpv4,&numIPv6);
		if (RV_FALSE == bLookForIpv4 && numIpv4 > 0) { bLookForIpv4 = RV_TRUE; }
		if (RV_FALSE == bLookForIpv6 && numIPv6 > 0) { bLookForIpv6 = RV_TRUE; }
	}

    
    switch (pRslv->eState)
    {
    case RESOLVER_STATE_UNDEFINED:
	/* Start by looking for an ipv6 address according to sipping-v6-transition-02.txt */
	/* which directs to RFC 3484 paragraph 10.2: "default destination ipv6 addresses  */ 
	/* should always be preferred over ipv4 addresses. */ 
#if(RV_NET_TYPE & RV_NET_IPV6)
        if (RV_TRUE == bLookForIpv6)
        {
            eNextQuery = RV_DNS_HOST_IPV6_TYPE;
            RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
                "ResolverDnsResolveIpByHost - pRslv=0x%p. in state %s will try %s query for %s ",
                pRslv, ResolverGetStateName(pRslv->eState), 
                GetAresDataTypeName(eNextQuery), pRslv->strQueryString));
            eNextState = RESOLVER_STATE_TRY_AAAA;
        }
		else /* Move forward to ipv4 query */ 
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/
        if (RV_TRUE == bLookForIpv4)
        {
            eNextQuery = RV_DNS_HOST_IPV4_TYPE;
            RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
                "ResolverDnsResolveIpByHost - pRslv=0x%p. in state %s will try %s query for %s ",
                pRslv, ResolverGetStateName(pRslv->eState), 
                GetAresDataTypeName(eNextQuery), pRslv->strQueryString));
            eNextState = RESOLVER_STATE_TRY_A;
        }
        break;
#if(RV_NET_TYPE & RV_NET_IPV6)
    case RESOLVER_STATE_TRY_AAAA:
        if (RV_TRUE == bLookForIpv4)
        {
            eNextQuery = RV_DNS_HOST_IPV4_TYPE;
            RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
                "ResolverDnsResolveIpByHost - pRslv=0x%p. in state %s will try %s query for %s ",
                pRslv, ResolverGetStateName(pRslv->eState), 
                GetAresDataTypeName(eNextQuery), pRslv->strQueryString));
            eNextState= RESOLVER_STATE_TRY_A;
        }
        break;
#endif /*#if(RV_NET_TYPE & RV_NET_IPV6)*/
    default:
        /* nothing to do, just print*/
        break;
    }
    if (RV_DNS_UNDEFINED != eNextQuery)
    {
        rv = AllocateAndSendQuery(pRslv,eNextQuery,pRslv->strQueryString, RV_FALSE);
        if (RV_OK != rv)
        {
            RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
                "ResolverDnsResolveIpByHost - pRslv=0x%p. failed to send %s query for %s ",
                pRslv, GetAresDataTypeName(eNextQuery), pRslv->strQueryString));
            return rv;            
        }
        ResolverChangeState(pRslv,eNextState);
        return RV_ERROR_TRY_AGAIN;
    }
    else
    {
        RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
            "ResolverDnsResolveIpByHost - pRslv=0x%p. failed to resolve host %s ",
            pRslv,pRslv->strQueryString));
    }
    return rv;
}

/************************************************************************************
 * ResolverDnsResolveURIByNAPTR
 * ----------------------------------------------------------------------------------
 * General: resolves a string to URI by applying an NAPTR query:
 *          send NAPTR -> fail
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv - pointer to the Resolver.
 ***********************************************************************************/
RvStatus RVCALLCONV ResolverDnsResolveURIByNAPTR(IN  Resolver *pRslv)
{
    RvStatus rv;
    RvChar strQuery[MAX_HOST_NAME_LEN+1];
    
    switch (pRslv->eState)
    {
    case RESOLVER_STATE_UNDEFINED:
        rv = DnsCreateENUMQueryFromAus(pRslv->pRslvMgr,pRslv->strQueryString,MAX_HOST_NAME_LEN+1,strQuery);
        if (RV_OK != rv)
        {
            RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
                "ResolverDnsResolveURIByNAPTR - pRslv=0x%p. can not create ENUM query form AUS",
                pRslv));
            return rv;
        }
        rv = AllocateAndSendQuery(pRslv,RV_DNS_NAPTR_TYPE,strQuery,RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
                "ResolverDnsResolveURIByNAPTR - pRslv=0x%p. failed to send %s query for %s ",
                pRslv, GetAresDataTypeName(RV_DNS_NAPTR_TYPE), strQuery));
            return rv;            
        }
        ResolverChangeState(pRslv,RESOLVER_STATE_TRY_NAPTR);
        break;
    default:
        break;
    }
    return RV_ERROR_TRY_AGAIN;
}


/************************************************************************************
 * ResolverDnsResolveProtocolByNAPTR
 * ----------------------------------------------------------------------------------
 * General: resolves a string to transport protocol by applying an NAPTR query
 *          send NAPTR -> fail
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv - pointer to the Resolver.
 ***********************************************************************************/
RvStatus RVCALLCONV ResolverDnsResolveProtocolByNAPTR(IN  Resolver *pRslv)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus        rv          = RV_OK;
    RvDnsQueryType  eNextQuery  = RV_DNS_UNDEFINED;
    switch (pRslv->eState)
    {
    case RESOLVER_STATE_UNDEFINED:
        eNextQuery = RV_DNS_NAPTR_TYPE;
        break;
    default:
        break;
    }
    if (RV_DNS_UNDEFINED != eNextQuery)
    {
        rv = AllocateAndSendQuery(pRslv,eNextQuery,pRslv->strQueryString,RV_FALSE);
        if (RV_OK != rv)
        {
            RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
                "ResolverDnsResolveProtocolByNAPTR - pRslv=0x%p. failed to send %s query for %s ",
                pRslv, GetAresDataTypeName(eNextQuery), pRslv->strQueryString));
            return rv;            
        }
        ResolverChangeState(pRslv,RESOLVER_STATE_TRY_NAPTR);
        return RV_ERROR_TRY_AGAIN;
    }
    else
    {
        RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
            "ResolverDnsResolveProtocolByNAPTR - pRslv=0x%p. failed to resolve host %s ",
            pRslv,pRslv->strQueryString));
    }
    return rv;
#else /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/
    RV_UNUSED_ARG(pRslv);
    return RV_ERROR_NOTSUPPORTED;
#endif /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/
}

/************************************************************************************
 * ResolverDnsResolveProtocolBy3WaySrv
 * ----------------------------------------------------------------------------------
 * General: resolves a string to transport protocol by applying SRV queries
 *          non secure: Try UDP -> Try TCP -> Try SCTP -> Try TLS -> fail
 *          secure    : Try TLS -> fail
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv - pointer to the Resolver.
 ***********************************************************************************/
RvStatus RVCALLCONV ResolverDnsResolveProtocolBy3WaySrv(IN  Resolver *pRslv)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus        rv          = RV_OK;
    RvSipTransport  eTransport  = RVSIP_TRANSPORT_UNDEFINED;
    RvChar          srvHostName[MAX_HOST_NAME_LEN + 50]  = {'\0'};
#if (RV_TLS_TYPE==RV_TLS_OPENSSL)
    RvUint32        numIpv4, numIPv6;
#endif /*RV_TLS_TYPE==RV_TLS_OPENSSL*/
    ResolverState   eNewState   = RESOLVER_STATE_UNDEFINED;

    switch (pRslv->eState)
    {
    case RESOLVER_STATE_UNDEFINED:
        if (RV_TRUE == pRslv->bIsSecure)
        {
#if (RV_TLS_TYPE==RV_TLS_OPENSSL)
            SipTransportMgrCounterLocalAddrsGet(pRslv->pRslvMgr->hTransportMgr,RVSIP_TRANSPORT_TLS,&numIpv4,&numIPv6);
            if (0 < numIpv4 + numIPv6)
            {
                eTransport = RVSIP_TRANSPORT_TLS;
                eNewState = RESOLVER_STATE_TRY_SRV_TLS;
                break;
            }
            else
#endif /*RV_TLS_TYPE==RV_TLS_OPENSSL*/
            {
                RvLogError(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
                    "ResolverDnsResolveProtocolBy3WaySrv - pRslv=0x%p (%s). no TLS addresses",
                    pRslv,pRslv->strQueryString));
                return RV_ERROR_UNKNOWN;
            }
        }
        else
        {   
            rv =DnsGetNextSrvTransportAndState(pRslv,pRslv->eState,&eNewState,&eTransport);
        }
        break;
    case RESOLVER_STATE_TRY_SRV_UDP:
    case RESOLVER_STATE_TRY_SRV_TCP:
    case RESOLVER_STATE_TRY_SRV_SCTP:
            rv =DnsGetNextSrvTransportAndState(pRslv,pRslv->eState,&eNewState,&eTransport);
        break;
    default:
        break;
    }
    if (RVSIP_TRANSPORT_UNDEFINED != eTransport)
    {
        DnsCreateSrvName(pRslv->eScheme,pRslv->strQueryString,eTransport,pRslv->bIsSecure,
			sizeof(srvHostName),srvHostName);
        rv = AllocateAndSendQuery(pRslv,RV_DNS_SRV_TYPE,srvHostName,RV_FALSE);
        if (RV_OK != rv)
        {
            RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
                "ResolverDnsResolveProtocolBy3WaySrv - pRslv=0x%p. failed to send %s query for %s ",
                pRslv, GetAresDataTypeName(RV_DNS_SRV_TYPE), srvHostName));
            return rv;            
        }
        pRslv->knownTransport = eTransport;
        ResolverChangeState(pRslv,eNewState);
        return RV_ERROR_TRY_AGAIN;
    }
    else
    {
        RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
            "ResolverDnsResolveProtocolBy3WaySrv - pRslv=0x%p. failed to resolve host %s ",
            pRslv,pRslv->strQueryString));
    }

    return rv;
#else /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/
    RV_UNUSED_ARG(pRslv);
    return RV_ERROR_NOTSUPPORTED;
#endif /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/
	
}
    
/************************************************************************************
 * ResolverDnsResolveHostPortBySrvString
 * ----------------------------------------------------------------------------------
 * General: resolves a string to host and port by applying an SRV query
 *          Send SRV -> fail
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv - pointer to the Resolver.
 ***********************************************************************************/
RvStatus RVCALLCONV ResolverDnsResolveHostPortBySrvString(IN  Resolver *pRslv)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;

    switch (pRslv->eState)
    {
    case RESOLVER_STATE_UNDEFINED:
        rv = AllocateAndSendQuery(pRslv,RV_DNS_SRV_TYPE,pRslv->strQueryString,RV_FALSE);
        if (RV_OK != rv)
        {
            RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
                "ResolverDnsResolveHostPortBySrvString - pRslv=0x%p. failed to send %s query for %s ",
                pRslv, GetAresDataTypeName(RV_DNS_SRV_TYPE), pRslv->strQueryString));
            return rv;            
        }
        ResolverChangeState(pRslv,RESOLVER_STATE_TRY_SRV_SINGLE);
        return RV_ERROR_TRY_AGAIN;
    case RESOLVER_STATE_TRY_SRV_SINGLE:
        RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
            "ResolverDnsResolveHostPortBySrvString - pRslv=0x%p. failed to resolve host %s ",
            pRslv,pRslv->strQueryString));
        break;
    default:
        break;
    }
    return rv;
#else /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/
    RV_UNUSED_ARG(pRslv);
    return RV_ERROR_NOTSUPPORTED;
#endif /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/
}

/************************************************************************************
 * ResolverDnsResolveHostPortByTransport
 * ----------------------------------------------------------------------------------
 * General: resolves a string to host and port by applying an SRV query
 *          Send SRV -> fail
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv - pointer to the Resolver.
 ***********************************************************************************/
RvStatus RVCALLCONV ResolverDnsResolveHostPortByTransport(IN  Resolver *pRslv)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;
    RvChar srvHostName [MAX_HOST_NAME_LEN + 50] = {'\0'};

    switch (pRslv->eState)
    {
    case RESOLVER_STATE_UNDEFINED:
        DnsCreateSrvName(pRslv->eScheme,pRslv->strQueryString,pRslv->knownTransport,pRslv->bIsSecure,
			sizeof(srvHostName),srvHostName);
        rv = AllocateAndSendQuery(pRslv,RV_DNS_SRV_TYPE,srvHostName,RV_FALSE);
        if (RV_OK != rv)
        {
            RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
                "ResolverDnsResolveHostPortByTransport - pRslv=0x%p. failed to send %s query for %s ",
                pRslv, GetAresDataTypeName(RV_DNS_SRV_TYPE), pRslv->strQueryString));
            return rv;            
        }
        ResolverChangeState(pRslv,RESOLVER_STATE_TRY_SRV_SINGLE);
        return RV_ERROR_TRY_AGAIN;
    case RESOLVER_STATE_TRY_SRV_SINGLE:
        RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
            "ResolverDnsResolveHostPortByTransport - pRslv=0x%p. failed to resolve host %s ",
            pRslv,pRslv->strQueryString));
        break;
    default:
        break;
    }
    return rv;
#else /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/
    RV_UNUSED_ARG(pRslv);
    return RV_ERROR_NOTSUPPORTED;
#endif /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/
}
/************************************************************************************
 * ResolverDnsResetQueryRes
 * ----------------------------------------------------------------------------------
 * General: resets Q id and free Q page. after that function is called we do 
 *          not expect anything from the Ares module
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pRslv      - Resolver
 ***********************************************************************************/
void RVCALLCONV ResolverDnsResetQueryRes(
                     IN  Resolver*    pRslv)
{
    RvLogDebug(pRslv->pRslvMgr->pLogSrc, (pRslv->pRslvMgr->pLogSrc,
        "ResolverDnsResetQueryRes: pResolver 0x%p - cancel query id %d", pRslv, pRslv->queryId));
    if (0 != pRslv->queryId)
    {
#if defined(UPDATED_BY_SPIRENT)
        RVSIP_ADJUST_VDEVBLOCK(pRslv->vdevblock);
        RvAresCancelQuery(pRslv->pRslvMgr->pDnsEngine[pRslv->vdevblock],pRslv->queryId);
#else
	RvAresCancelQuery(pRslv->pRslvMgr->pDnsEngine,pRslv->queryId);
#endif
        pRslv->queryId = 0;
    }    
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * GetAresDataTypeName
 * ------------------------------------------------------------------------
 * General: converts Ares datatype to string
 * Return Value: - datatype name
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:    eDataType        - eDataType enum
 ***************************************************************************/
static RvChar* RVCALLCONV GetAresDataTypeName(IN RvDnsQueryType eDataType)
{
    switch(eDataType)
    {
    /* these are symmetric codes: application <---> resolver */
    case RV_DNS_HOST_IPV4_TYPE: return "IPv4";
    case RV_DNS_HOST_IPV6_TYPE: return "IPv6";
    case RV_DNS_SRV_TYPE:       return "Srv";
    case RV_DNS_NAPTR_TYPE:     return "NAPTR";
    case RV_DNS_STATUS_TYPE:    return "Status";
    case RV_DNS_ENDOFLIST_TYPE: return "EndOfList";
	case RV_DNS_NO_DATA_TYPE:   return "NoData";
    default:                    return "n/a";
    }/*eReason*/
}
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */
/*****************************************************************************
* AllocateAndSendQuery
* ---------------------------------------------------------------------------
* General: 1. allocate the query page and the buffer inside the page. 
*          2. sends the dns query by calling the Ares functions
* Return Value: RvStatus - RvStatus
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pRslv - pointer to the  Resolver
*          dnsQuery - query type
*          dnsName - name to perform dns query on
* Output:  None
*****************************************************************************/
static RvStatus RVCALLCONV AllocateAndSendQuery(
                            IN  Resolver* pRslv,
                            IN RvDnsQueryType dnsQuery,
                            IN RvChar* dnsName,
                            IN RvBool bAsIs)
{
    RvStatus    crv     = RV_ERROR_UNKNOWN;
    RvStatus    rv      = RV_OK;
    RvInt32     elementSize = 0;
    RvInt32     offsetDummy;
    
    RvChar*     pQbuff  = NULL;
    RvInt       bufLen  = 0;
    
    while (RV_OK != crv)
    {
#if defined(UPDATED_BY_SPIRENT)
        RVSIP_ADJUST_VDEVBLOCK(pRslv->vdevblock);
        strcpy(pRslv->pRslvMgr->pDnsEngine[pRslv->vdevblock]->if_name,pRslv->if_name);
        memcpy(&pRslv->pRslvMgr->pDnsEngine[pRslv->vdevblock]->localAddr,&pRslv->localAddr,sizeof(pRslv->pRslvMgr->pDnsEngine[pRslv->vdevblock]->localAddr));
        crv = RvAresSendQuery(pRslv->pRslvMgr->pDnsEngine[pRslv->vdevblock],
#else
        crv = RvAresSendQuery(pRslv->pRslvMgr->pDnsEngine,
#endif
            dnsQuery,
            dnsName,
            bAsIs,
            pQbuff,
            &bufLen,
            pRslv,
            &pRslv->queryId);
        if (RV_ERROR_INSUFFICIENT_BUFFER == RvErrorGetCode(crv))
        {
            if (NULL_PAGE == pRslv->hQueryPage)
            {
                RPOOL_GetPoolParams(pRslv->pRslvMgr->hGeneralPool,&elementSize,NULL);
                if (elementSize < bufLen)
                {
                    return RV_ERROR_INSUFFICIENT_BUFFER;
                }
                rv = RPOOL_GetPage(pRslv->pRslvMgr->hGeneralPool,0,&pRslv->hQueryPage);
                if (RV_OK != rv)
                {
                    RvLogError(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
                        "AllocateAndSendQuery - pRslv=0x%p. can not allocate page",pRslv));
                    return rv;
                }
            }
            pQbuff = RPOOL_AlignAppend(pRslv->pRslvMgr->hGeneralPool,
                pRslv->hQueryPage,
                bufLen,
                &offsetDummy);
            if (NULL == pQbuff)
            {
                RvLogError(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
                    "AllocateAndSendQuery - pRslv=0x%p. can not allign-append",pRslv));
                return RV_ERROR_OUTOFRESOURCES;
            }
        }
		else if (RV_DNS_ERROR_NOTFOUND == crv)
		{
			RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
                "AllocateAndSendQuery - pRslv=0x%p. got DNS negative response, query is unresolved (rv=%d)",pRslv,RvErrorGetCode(crv)));
            return crv;
		}
        else if (RV_OK != RvErrorGetCode(crv))
        {
            RvLogError(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
                "AllocateAndSendQuery - pRslv=0x%p. error (rv=%d)",pRslv,RvErrorGetCode(crv)));
            return RvErrorGetCode(crv);
        }
    }
    RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
        "AllocateAndSendQuery - pRslv=0x%p. name=%s bAsIs=%d was sent succesfully",
        pRslv,dnsName,bAsIs));
    return RV_OK;

}

/*****************************************************************************
* DnsLockByQueryId
* ---------------------------------------------------------------------------
* General: locks the Resolver by Q id. this is necessary to make sure 
*          that the resolver did not switch under our feet
* Return Value: RvStatus - RvStatus
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pRslv        - Transport Resolver
*          queryId     - the Qid to lock with
* Output:  none
*****************************************************************************/
static RvStatus DnsLockByQueryId(IN Resolver* pRslv,
                                 IN RvUint32     queryId)
{
    RvStatus rv = RV_OK;

    /* we are starting to process an answer, lock the Resolver */
    rv = ResolverLockEvent(pRslv);
    if (RV_OK != rv)
    {
        return rv;
    }
    /* Sending and receiving DNS messages is a-synchronic. it is possible for a DNS
       response to arrive after a resolver was destructed, we try and lock the Resolver.
       We try and match the qid of the resolver to the qid of the responce.
       if we fail we assume that the response is no longer needed
    */
    if (queryId != pRslv->queryId)
    {
        ResolverUnLockEvent(pRslv);
        return RV_ERROR_INVALID_HANDLE;
    }
    return rv;
}

/*****************************************************************************
* ResolverHandleEndOfQuery
* ---------------------------------------------------------------------------
* General: goes to the next stage of resolution by the resolution mode.
*          or report the failure to the application.
*          for NAPTR replies the list is changed to contain only one order
* Return Value: RvStatus - RvStatus
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pRslv        - Transport Resolver
*          eStatus      - was the query good
* Output:  none
*****************************************************************************/
static RvStatus ResolverHandleEndOfQuery(IN Resolver* pRslv,
                                         IN RvDnsQueryType eStatus)
{
    RvStatus rv = RV_OK;
    switch (eStatus)
    {
    case RV_DNS_ENDOFLIST_TYPE: /* the query was completed succesfully */
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
        
        if (pRslv->hDns != NULL && pRslv->eMode == RVSIP_RESOLVER_MODE_FIND_TRANSPORT_BY_NAPTR)
        {
            SipTransportDnsListPrepare(pRslv->pRslvMgr->hTransportMgr,
                pRslv->strQueryString,
                pRslv->hDns);
        }
#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/

        ResolverCallbacksReportData(pRslv,RV_FALSE);
        break;
	case RV_DNS_NO_DATA_TYPE:
    case RV_DNS_STATUS_TYPE:
        rv = ResolverHandleFailure(pRslv);
        break;
    default:
        return RV_ERROR_UNKNOWN;
    }
    return rv;
}

/*****************************************************************************
* ResolverHandleFailure
* ---------------------------------------------------------------------------
* General: Try and continue by mode
* Return Value: RvStatus - RvStatus
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pRslv        - Transport Resolver
*          eStatus      - was the query good
* Output:  none
*****************************************************************************/
static RvStatus ResolverHandleFailure(IN Resolver* pRslv)
{
    RvStatus    rv = RV_OK;
    switch (pRslv->eMode)
    {
    case RVSIP_RESOLVER_MODE_FIND_TRANSPORT_BY_NAPTR:
        rv = ResolverDnsResolveProtocolByNAPTR(pRslv);
        break;
    case RVSIP_RESOLVER_MODE_FIND_TRANSPORT_BY_3WAY_SRV:
        rv = ResolverDnsResolveProtocolBy3WaySrv(pRslv);
        break;
    case RVSIP_RESOLVER_MODE_FIND_IP_BY_HOST:
        rv = ResolverDnsResolveIpByHost(pRslv);
        break;
    case RVSIP_RESOLVER_MODE_FIND_HOSTPORT_BY_SRV_STRING:
        rv = ResolverDnsResolveHostPortBySrvString(pRslv);
        break;
    default:
        break;
    
    }
    if (RV_ERROR_TRY_AGAIN == rv)
    {
        return RV_OK;
    }
    else
    {
        ResolverCallbacksReportData(pRslv,RV_TRUE);
    }
    return rv;
}

/*****************************************************************************
* ResolverHandleNewNaptr
* ---------------------------------------------------------------------------
* General: handles an NAPTR reply according to resolver mode
* Return Value: RvStatus - RvStatus
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pRslv        - Transport Resolver
*          pDnsData      - data as recived from Ares
* Output:  none
*****************************************************************************/
static RvStatus ResolverHandleNewNaptr(IN Resolver* pRslv,
                                       IN RvDnsData* pDnsData)
{
    RvStatus rv = RV_OK;
    switch (pRslv->eMode)
    {
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    case RVSIP_RESOLVER_MODE_FIND_TRANSPORT_BY_NAPTR:
        rv = SipTransportDnsAddNAPTRRecord(pRslv->pRslvMgr->hTransportMgr,pDnsData,pRslv->bIsSecure,pRslv->hDns);
        if (RV_ERROR_BADPARAM == rv) /* the protocol was unfamiliar, ignore and keep processing */
        {
            rv = RV_OK;
        }
        
        break;
#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/
    case RVSIP_RESOLVER_MODE_FIND_URI_BY_NAPTR:
        {
            rv = DnsValidateAndExtractAddr(pRslv, pDnsData);
        }
        break;
    default:
        break;
    }
    return rv;
}

/************************************************************************************
 * DnsValidateAndExtractAddr
 * ---------------------------------------------------------------------------
 * General: make sure that the NAPTR response matches the ruls of ENUM. if it does, 
 *          insert it to the list
 * Return Value: RvStatus - RvStatus
 * Arguments:
 * Input:   pRslv        - Resolver
 *          pDnsData     - the DNS data from the callback
 * output:  (-)
 ***********************************************************************************/
static RvStatus RVCALLCONV DnsValidateAndExtractAddr(
             IN Resolver* pRslv ,
             IN RvDnsData* pDnsData)
{
    RvStatus    rv          = RV_OK;
    RvChar*     pDummy      = NULL;
    
    /* make sure that flag is "u" */
#define ENUM_FLAG_U "u"
#define ENUM_SERIVE_E2U_SIP "e2u+sip"
#define ENUM_SERIVE_SIP_E2U "sip+e2u"

    /* testing the validity of the received NAPTR record */
    
    /* RFC3761 - [2.4.1]*/
    if (RV_TRUE != SipCommonStricmp(ENUM_FLAG_U,pDnsData->data.dnsNaptrData.flags))
    {
        RvLogError(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
            "DnsValidateAndExtractAddr - pRslv=0x%p. flag must be '%s'. returning rv=%d",
            pRslv,ENUM_FLAG_U, RV_ERROR_UNKNOWN));
        return RV_ERROR_UNKNOWN;
    }

    /* RFC3761 [8] + RFC3824 [5.1] */
    if (RV_TRUE != SipCommonStricmp(ENUM_SERIVE_E2U_SIP,pDnsData->data.dnsNaptrData.service) &&
        RV_TRUE != SipCommonStricmp(ENUM_SERIVE_SIP_E2U,pDnsData->data.dnsNaptrData.service)  )
    {
        RvLogError(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
            "DnsValidateAndExtractAddr - pRslv=0x%p. service (%s) is not E2U and sip. returning rv=%d",
            pRslv,pDnsData->data.dnsNaptrData.service,RV_ERROR_UNKNOWN));
        return RV_ERROR_UNKNOWN;
    }

    /* checking if we already have an NAPTR result in the list we only allow one record */
    /* The only one record is derived from RFC3824 [4] two last paragraphs */
    if (RV_ERROR_NOT_FOUND != SipTransportDnsGetEnumResult(pRslv->pRslvMgr->hTransportMgr,pRslv->hDns,&pDummy))
    {
        RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
            "DnsValidateAndExtractAddr - pRslv=0x%p. only one enum allowed",
            pRslv));   
        return RV_ERROR_OBJECT_ALREADY_EXISTS;
    }
    rv = SipTransportDnsSetEnumResult(pRslv->pRslvMgr->hTransportMgr,pDnsData,pRslv->hDns);
    if (RV_OK != rv)
    {
        RvLogError(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
            "DnsValidateAndExtractAddr - pRslv=0x%p. can not set enum result",
            pRslv));   
        return rv;
    }
    return rv;
}



/************************************************************************************
 * DnsCreateENUMQueryFromAus
 * ---------------------------------------------------------------------------
 * General: Creates an ENUM string from a TEL URL according to RFC3761 (2.4)
 * Return Value: RvStatus - RvStatus
 * Arguments:
 * Input:   hRslv        - Transport Resolver
 * output:  strENUM  - formatted string for NAPTR queries
 ***********************************************************************************/
static RvStatus RVCALLCONV DnsCreateENUMQueryFromAus(
             IN    ResolverMgr* pRslvMgr,
             IN    RvChar*      strAus,
             IN    RvInt        bufLen,    
             OUT   RvChar*      strENUM)
{
    RvInt   i           = 0;
    RvInt   strlocation = 0;
    RvInt   ausLen      = 0;

    strENUM[0]  = '\0';
    ausLen      = (RvInt)strlen(strAus);

    if (ausLen*2+pRslvMgr->dialPlanLen > bufLen)
    {
        RvLogDebug(pRslvMgr->pLogSrc,(pRslvMgr->pLogSrc,
            "DnsCreateENUMQueryFromAus - not enough space"));
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }
    for (i=ausLen;i>=0;i--)
    {
        if (isdigit((RvInt)strAus[i]))
        {
            strENUM[strlocation++]=strAus[i];
            strENUM[strlocation++]='.';
        }
    }
    sprintf(&strENUM[strlocation],pRslvMgr->strDialPlanSuffix);
    return RV_OK;
}

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/************************************************************************************
 * DnsGetNextSrvTransportAndState
 * ---------------------------------------------------------------------------
 * General: get the nex srv transport to be queried (+state) from the previously
 *          asked SRV transport.
 *          the order is: UDP->TCP->SCPT->TLS
 * Return Value: RvStatus - RvStatus
 * Arguments:
 * Input:   hRslv        - Transport Resolver
 * output:  strENUM  - formatted string for NAPTR queries
 ***********************************************************************************/
static RvStatus RVCALLCONV DnsGetNextSrvTransportAndState(
                            IN  Resolver* pRslv,
                            IN  ResolverState eState,
                            OUT ResolverState*  peNewState,
                            OUT RvSipTransport* peTransport)
{
    RvUint32        numIpv4, numIPv6;
    
    switch (eState)
    {
    case RESOLVER_STATE_UNDEFINED:
        SipTransportMgrCounterLocalAddrsGet(pRslv->pRslvMgr->hTransportMgr,RVSIP_TRANSPORT_UDP,&numIpv4,&numIPv6);
        if (0 < numIpv4 + numIPv6)
        {
            *peTransport = RVSIP_TRANSPORT_UDP;
            *peNewState = RESOLVER_STATE_TRY_SRV_UDP;        
            return RV_OK;
        }
        /* no break continue to try TCP!!!*/
    case RESOLVER_STATE_TRY_SRV_UDP:
        SipTransportMgrCounterLocalAddrsGet(pRslv->pRslvMgr->hTransportMgr,RVSIP_TRANSPORT_TCP,&numIpv4,&numIPv6);
        if (0 < numIpv4 + numIPv6)
        {
            *peTransport = RVSIP_TRANSPORT_TCP;
            *peNewState = RESOLVER_STATE_TRY_SRV_TCP;
            return RV_OK;
        }
        /* no break continue to try SCTP!!!*/
    case RESOLVER_STATE_TRY_SRV_TCP:

#if (RV_NET_TYPE & RV_NET_SCTP)
        SipTransportMgrCounterLocalAddrsGet(pRslv->pRslvMgr->hTransportMgr,RVSIP_TRANSPORT_SCTP,&numIpv4,&numIPv6);
        if (0 < numIpv4 + numIPv6)
        {
            *peTransport = RVSIP_TRANSPORT_SCTP;
            *peNewState = RESOLVER_STATE_TRY_SRV_SCTP;
            return RV_OK;
        }
        /* no break continue to try TLS!!!*/
    case RESOLVER_STATE_TRY_SRV_SCTP:
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

        SipTransportMgrCounterLocalAddrsGet(pRslv->pRslvMgr->hTransportMgr,RVSIP_TRANSPORT_TLS,&numIpv4,&numIPv6);
        if (0 < numIpv4 + numIPv6)
        {
            *peTransport = RVSIP_TRANSPORT_TLS;
            *peNewState = RESOLVER_STATE_TRY_SRV_TLS;
            return RV_OK;
        }
        /* no break - not address was found!!!*/
    default:
        return RV_ERROR_NOT_FOUND;
    } /* switch */
}

/************************************************************************************
 * DnsCreateSrvName
 * ---------------------------------------------------------------------------
 * General: created an SRV name for s specific transport/scheme 
 *          something like: _.scheme._transport.name
 *          if there is no scheme or transport ther will not be added
 * Return Value: RvStatus - RvStatus
 * Arguments:
 * input   : eScheme        - im/sip/pres
 *           srvHostName    - the domain name
 *           eTransport     - transport protocol
 *           bIsSecure      - used to decide sip/sips for the sip scheme
 *           strSrvNameLen  - the size of the strSrvName buffer
 * output  : strSrvName     - default for the protocol SIP SRV name
 ***********************************************************************************/
static void RVCALLCONV DnsCreateSrvName(
             IN    RvSipResolverScheme eScheme,
             IN    RvChar*           strHostName,
             IN    RvSipTransport    eTransport,
             IN    RvBool            bIsSecure,
			 IN    RvSize_t          strSrvNameLen,
             OUT   RvChar*           strSrvName)
{
    RvChar* prefix = "";
    RvChar* scheme = "";
    switch (eTransport)
    {
    case RVSIP_TRANSPORT_TCP:
        prefix = SRV_TCP_PREFIX;
        break;
    case RVSIP_TRANSPORT_UDP:
        prefix = SRV_UDP_PREFIX;
    break;
    case RVSIP_TRANSPORT_TLS:
        prefix = SRV_TLS_PREFIX;
            break;
    case RVSIP_TRANSPORT_SCTP:
        prefix = SRV_SCTP_PREFIX;
            break;
    default:
        break;
    }
    switch (eScheme)
    {
    case RVSIP_RESOLVER_SCHEME_SIP:
        scheme = (RV_TRUE == bIsSecure || RVSIP_TRANSPORT_TLS==eTransport) ? SRV_SIPS_SCHEME :SRV_SIP_SCHEME;
        break;
    case RVSIP_RESOLVER_SCHEME_PRES:
        scheme = SRV_PRES_SCHEME;
        break;
    case RVSIP_RESOLVER_SCHEME_IM:
        scheme = SRV_IM_SCHEME;
        break;
    default:
        break;
    }
    RvSnprintf(strSrvName,strSrvNameLen,"%s%s%s",scheme,prefix,strHostName);
}
#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/

