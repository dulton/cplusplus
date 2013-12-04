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
 *                              <RvSipResolverTypes.h>
 *
 * This file defines the Resolver types
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tirosh                    Dec 2004
 *********************************************************************************/

#ifndef RVSIP_RESOLVER_TYPES_H
#define RVSIP_RESOLVER_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*
 * RvSipResolverMgrHandle
 * ---------------------------------------------------------------------------
 * Declaration of handle to the resolver manager object.
 */
RV_DECLARE_HANDLE(RvSipResolverMgrHandle);

/*
 * RvSipResolverHandle
 * ---------------------------------------------------------------------------
 * Declaration of handle to the resolver object.
 */
RV_DECLARE_HANDLE(RvSipResolverHandle);

/*
 * RvSipAppResolverHandle
 * ---------------------------------------------------------------------------
 * Declaration of handle to the application resolver object.
 */
RV_DECLARE_HANDLE(RvSipAppResolverHandle);

/*-----------------------------------------------------------------------*/
/*                    RESOLVER TYPE DEFINITIONS                           */
/*-----------------------------------------------------------------------*/
/*
 * RvSipResolverMode
 * ---------------------------------------------------------------------------
 * enumeration that denotes the moce by which the resolver will act.
 * RVSIP_RESOLVER_MODE_UNDEFINED  - no resolution mode.
 * RVSIP_RESOLVER_MODE_FIND_TRANSPORT_BY_NAPTR - 
 *		This mode instructs the resolver to try and obtain NAPTR records for a 
 *      domain. The resolver will use the protocol field of the NAPTR record to 
 *      determine the transport. 
 *      (e.g. getting the available transports for "radvision.com". the result 
 *       will be a list of SRV record pointers). 
 * RVSIP_RESOLVER_MODE_FIND_TRANSPORT_BY_3WAY_SRV - 
 *      This mode instructs the resolver to try and find a transport for a domain 
 *      by applying SRV queries of _service._protocol.domain, until the resolver 
 *      receives a successful answer.
 *      (e.g. Trying radvision.com will send queries like _scheme._protocol.radvision.com 
 *      trying UDP, TCL and finally TLS. If the resolver gets a positive answer, 
 *      it wil stop querying, and as a result provide a host pointer and a port number).
 * RVSIP_RESOLVER_MODE_FIND_HOSTPORT_BY_SRV_STRING - 
 *      this mode instructs the resolver to send one SRV query with the string 
 *      supplied and to save the host and port retrieved in the answer 
 *      in the DNS list. 
 *      (e.g. Trying to get the host and port for _sip._udp.radvision.com)
 * RVSIP_RESOLVER_MODE_FIND_HOSTPORT_BY_TRANSPORT - 
 *      This mode will instruct the resolver to create one service._protocol.domain 
 *      string and to try and send an SRV query for it. The host and port retrieved 
 *      in the answer will be stored in the DNS list. 
 *      (e.g., trying to get the TCP/pres SRV record for "radvision.com" will send 
 *       a _pres._udp.radvision.com SRV query and will result in a host pointer and 
 *       a port number).
 * RVSIP_RESOLVER_MODE_FIND_IP_BY_HOST - 
 *       This mode will instruct the resolver to try and find the IP of a specific 
 *       host. The resolver will try both IPv4 and IPv6 (using A and AAAA queries 
 *       respectively). 
 *      (e.g., trying to get the ip for host1.radvision.com, the result will be an
 *       ip address).
 * RVSIP_RESOLVER_MODE_FIND_URI_BY_NAPTR - 
 *       will get the NAPTR record of for an ENUM record in a DNS server. 
 *      (e.g.. Trying to resolve a phone number like +97237679623, will result in 
 *       an NAPTR query, for 3.2.6.9.7.6.7.3.2.7.9.e164.arpa. 
 *       the result will be a regular expression).
 */

typedef enum {
    RVSIP_RESOLVER_MODE_UNDEFINED = -1,
    RVSIP_RESOLVER_MODE_FIND_TRANSPORT_BY_NAPTR,
    RVSIP_RESOLVER_MODE_FIND_TRANSPORT_BY_3WAY_SRV,
    RVSIP_RESOLVER_MODE_FIND_HOSTPORT_BY_SRV_STRING,
    RVSIP_RESOLVER_MODE_FIND_HOSTPORT_BY_TRANSPORT,
    RVSIP_RESOLVER_MODE_FIND_IP_BY_HOST,
    RVSIP_RESOLVER_MODE_FIND_URI_BY_NAPTR
}RvSipResolverMode;

/*
 * RvSipResolverScheme
 * ---------------------------------------------------------------------------
 * Enumarate the different schemes that the resolver can use for SRV queries.
 * The eScheme parameter specifies the scheme to concatenate to the query string
 * parameter when SRV query is applied to retrieve the transport.
 *  RVSIP_RESOLVER_SCHEME_UNDEFINED       -- No Scheme 
 *  RVSIP_RESOLVER_SCHEME_SIP   -- "sip"/"sips" 
 *  RVSIP_RESOLVER_SCHEME_PRES  -- "pres" 
 */

typedef enum {
    RVSIP_RESOLVER_SCHEME_UNDEFINED = -1,
    RVSIP_RESOLVER_SCHEME_SIP,
    RVSIP_RESOLVER_SCHEME_PRES,
    RVSIP_RESOLVER_SCHEME_IM
}RvSipResolverScheme;

/*-----------------------------------------------------------------------*/
/*                       RESOLVER CALLBACKS                              */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipResolverReportData
 * ------------------------------------------------------------------------
 * General: Using this callback, the resolver reports the results of a DNS algorithm
 * requested by the application. The application has to supply the callback 
 * function pointer when calling the RvSipResolverResolve() API function.
 * This callback will be called whenever the algorithm block has ended, and 
 * will indicate if the algorithm block was successful or not.
 * When this callback is called, and indicates that there is no error, the DNS 
 * list will contain the required answers. You can obtain the list by calling
 * the RvSipResolverGetDnsList() API function.
 * Return Value: Ignored.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hResolver -- the resolver handle
 *           hAppRslv - Application handle to the Resolver. 
 *           bError    -- was resolution successful
 *           eMode     -- resolver mode that was used
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *RvSipResolverReportDataEv) (
                              IN RvSipResolverHandle       hResolver,
                              IN RvSipAppResolverHandle    hAppRslv,
                              IN RvBool                  bError,
                              IN RvSipResolverMode         eMode);

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef RVSIP_RESOLVER_TYPES_H*/

