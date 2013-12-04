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
 *                              <_SipResolver.h>
 *
 * This file defines the Resolver internal API functions
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                    Dec 2004
 *********************************************************************************/

#ifndef _SIP_RESOLVER_H
#define _SIP_RESOLVER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipResolverTypes.h"
    
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
                         IN RvSipResolverHandle       hRslv);

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
                         IN RvSipResolverHandle       hRslv);

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
             ,IN RvSipTransportAddr             *localAddr
#endif
);

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
             OUT   RvChar*          strENUM);
#endif /*#ifdef RV_SIP_TEL_URI_SUPPORT*/

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
                         IN SipTripleLock*      pTripleLock); 

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef _SIP_RESOLVER_H*/

