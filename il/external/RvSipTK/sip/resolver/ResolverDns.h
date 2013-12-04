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

#ifndef RESOLVER_DNS_H
#define RESOLVER_DNS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************
 *                              <ResolverDns.h>
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
#define RESOLVER_DNS_REG_MAX_MATCHES 10
    

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipResolverTypes.h"
#include "RvSipTransportTypes.h"
#include "_SipResolverMgr.h"

/*-----------------------------------------------------------------------*/
/*                           FILE FUNCTIONS                              */
/*-----------------------------------------------------------------------*/

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
RvStatus RVCALLCONV ResolverDnsResolveURIByNAPTR(IN  Resolver *pRslv);

/*****************************************************************************
* ResolverDnsResolveIpByHost
* ---------------------------------------------------------------------------
* General: makes decisions as to where to go for resolving in the 
*          SIP_RESOLVER_MODE_FIND_IP_BY_HOST mode:
*          first try IPv4 (A) -> try IPv6 (AAAA) -> Fail
* Return Value: RvStatus - RvStatus
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pRslv        - Transport Resolver
* Output:  none
*****************************************************************************/
RvStatus ResolverDnsResolveIpByHost(IN Resolver* pRslv);

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
RvStatus RVCALLCONV ResolverDnsResolveProtocolByNAPTR(IN  Resolver *pRslv);

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
RvStatus RVCALLCONV ResolverDnsResolveProtocolBy3WaySrv(IN  Resolver *pRslv);
    
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
RvStatus RVCALLCONV ResolverDnsResolveHostPortBySrvString(IN  Resolver *pRslv);

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
RvStatus RVCALLCONV ResolverDnsResolveHostPortByTransport(IN  Resolver *pRslv);

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
* Input:   pRslvVoid    - the transmiter as a void pointer
*          queryId     - the Id the query was sent with
*          pDnsData    - structure of type RvDnsData with a new record data to add.
*               the dataType member indicates the type of data (IPv4/6, SRV or NAPTR)
*               in the structure
* Output:  -
*****************************************************************************/
RvStatus ResolverDNSNewRecordCB (
    IN  void*               pRslvVoid,
    IN  RvUint32            queryId,
    IN  RvDnsData*          pDnsData);

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
                     IN  Resolver*    pRslv);

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef RESOLVER_DNS_H*/

