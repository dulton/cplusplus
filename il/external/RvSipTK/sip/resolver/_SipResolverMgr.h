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
 *                              <_SipResolverMgr.h>
 *
 * This file defines the Resolver internal API functions
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                    Dec 2004
 *********************************************************************************/

#ifndef _SIP_RESOLVER_MGR_H
#define _SIP_RESOLVER_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipResolverTypes.h"
#include "RV_SIP_DEF.h"
#include "rvlog.h"
#include "rvrandomgenerator.h"
#include "RvSipResourcesTypes.h"
#include "RvSipTransportTypes.h"
#include "RvSipResolverTypes.h"
#include "_SipCommonTypes.h"
#include "rvares.h"
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
/* SipResolverMgrCfg
   -----------------------------------------------------------------------
   contains data needed to initialize the resolver module
   Data Members:
   pLogMgr          -- The stack log manager.
   pLogSrc          -- The module log-source.
   maxNumOfRslv     -- Max number of resolvers in the resolver list.
   hTransportMgr    -- Handle to the transport module manager.
   seed             -- Used to generate unique identifier to the Resolvers
   maxDnsServers    -- The maximal number of DNS servers that the 
                       application will use concurrently.
   maxDnsDomains    -- The maximal number of DNS domains that the 
                       application will use concurrently.
   maxDnsBuffLen    -- The length of ARES TCP buffer 
   hGeneralPool     -- general pool
   hMessagePool     -- message pool
   hElementPool     -- element pool
   strDialPlanSuffix -- suffix for dial plan (like "e164.arpa")

*/
typedef struct
{
    RvLogMgr*                 pLogMgr;
    RvLogSource*              pLogSrc;
    RvInt32                   maxNumOfRslv;
    RvSipTransportMgrHandle   hTransportMgr;
    RvRandomGenerator*        seed;
    RvInt32                   maxDnsServers;
    RvInt32                   maxDnsDomains;
    RvInt32                   maxDnsBuffLen;
    HRPOOL                    hGeneralPool;
    HRPOOL                    hMessagePool;
    HRPOOL                    hElementPool;
    RvChar*                   strDialPlanSuffix;
}SipResolverMgrCfg;

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
 * Output:  *phTrxMgr   - Handle to the newly created Resolver manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipResolverMgrConstruct(
                            IN  SipResolverMgrCfg*            pCfg,
                            IN  void*                         pStack,
                            IN  RvBool                        bTryServers,
                            IN  RvBool                        bTryDomains,
                            OUT RvSipResolverMgrHandle*         phTrxMgr);


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
                            IN RvSipResolverMgrHandle hRslvMgr);

/***************************************************************************
 * SipResolverMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the Resolver manager freeing all resources.
 * Return Value:  RV_OK -  Resolver manager was destructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hRslvMgr - Handle to the manager to destruct.
 ***************************************************************************/
RvStatus RVCALLCONV SipResolverMgrDestruct(
                            IN RvSipResolverMgrHandle hRslvMgr);

/***************************************************************************
 * SipResolverMgrGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: Returns a structure with the status of all resources used by
 *          the Resolver module.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hRslvMgr   - Handle to the Resolver manager.
 * Output:     pResources - Pointer to a Resolver resource structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipResolverMgrGetResourcesStatus (
                                 IN  RvSipResolverMgrHandle  hRslvMgr,
                                 OUT RvSipResolverResources  *pResources);


/***************************************************************************
 * SipResolverMgrCreateResolver
 * ------------------------------------------------------------------------
 * General: Creates a new Resolver and exchange handles with the
 *          application.  The new Resolver assumes the IDLE state.
 *          Using this function the transaction can supply its memory
 *          page and triple lock to the transmitter.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRslvMgr - Handle to the Resolver manager
 *            hAppRslv - Application handle to the newly created Resolver.
 *            hPool     - A pool for this transmitter. DEPRECATED
 *            hPage     - A memory page to be used by this transmitter. DEPRECATED
 *            pTripleLock - A triple lock to use by the transmitter. If NULL
 *                        is supplied the transmitter will use its own lock.
 * Output:    phRslv   -   SIP stack handle to the Resolver
 ***************************************************************************/
RvStatus RVCALLCONV SipResolverMgrCreateResolver(
                   IN  RvSipResolverMgrHandle   hRslvMgr,
                   IN  RvSipAppResolverHandle   hAppRslv,
                   IN  SipTripleLock*           pTripleLock,
                   OUT RvSipResolverHandle*     phRslv);

#if defined(UPDATED_BY_SPIRENT)
  RVAPI RvStatus RVCALLCONV SipResolverMgrSetTimeout(
						     IN  RvSipResolverMgrHandle   hRslvMgr,
						     IN  RvInt timeout,
						     IN  RvInt tries,
						     IN  RvUint16 vdevblock
						     );
#endif

/************************************************************************************
 * SipResolverMgrSetDnsServers
 * ----------------------------------------------------------------------------------
 * General: Sets a new list of dns servers to the stack
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hRslvMgr          - Resolver mgr
 *          pDnsServers      - a list of DNS servers to set to the stack
 *          numOfDnsServers  - The number of Dns servers in the list
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV SipResolverMgrSetDnsServers(
      IN  RvSipResolverMgrHandle   hRslvMgr,
      IN  RvSipTransportAddr*    pDnsServers,
      IN  RvInt32                numOfDnsServers
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
);

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
      IN  RvSipTransportAddr*    pDnsServers,
      INOUT  RvInt32*            pNumOfDnsServers
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
);

/************************************************************************************
 * SipResolverMgrSetDnsDomains
 * ----------------------------------------------------------------------------------
 * General: Sets a new list of dns domains to the stack
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hRslvMgr         - Resolver mgr
 *          pDomainList      - a list of DNS domain. (an array of NULL terminated strings
 *                             to set to the stack
 *          numOfDomains     - The number of domains in the list
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV SipResolverMgrSetDnsDomains(
      IN  RvSipResolverMgrHandle   hRslvMgr,
      IN  RvChar**               pDomainList,
      IN  RvInt32                numOfDomains
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
);

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
                      INOUT  RvChar**            pDomainList,
                      INOUT  RvInt32*            pNumOfDomains
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
);

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
      IN  RvSipResolverMgrHandle   hTrxMgr);
#else /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */
#define SipResolverMgrPrintDnsParams(_m)
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */


#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef _SIP_RESOLVER_MGR_H*/

