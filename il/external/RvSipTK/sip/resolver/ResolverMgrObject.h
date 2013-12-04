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
 *                              <ResolverMgrObject.h>
 *
 * This file defines the Resolver object, attributes and interface functions.
 * The resolver is responsible for all DNS operations in the SIP stack.
 * Assamptions:
 * 1. A resolver is always created by a transmitter and is held by it.
 * 2. 
 * 
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                    Dec 2004
 *********************************************************************************/

#ifndef RESOLVER_MGR_OBJECT_H
#define RESOLVER_MGR_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "rvmutex.h"
#include "rvlog.h"
#include "rvrandomgenerator.h"
#include "rvares.h"
#include "rvarescache.h"

#include "_SipResolverTypes.h"
#include "RvSipTransportTypes.h"
#include "_SipResolverMgr.h"
#include "_SipCommonTypes.h"
#include "AdsRlist.h"
#include "AdsRpool.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
/* ResolverMgr
   -----------------------------------------------------------------------
   The Resolver Mgr holds general resolving data such as pools, lists, 
   core objects reference
   Data Members:
   pLogMgr          -- The stack log manager.
   pLogSrc          -- The module log-source.
   pDnsEngine       -- a pointer to DNS engine
   hRslvListPool    -- Handle to a pool of Transmitter lists. only one 
                       list is allocated from this pool.
   hRslvList        -- Handle to the Transmitters list.
   hMutex           -- A lock used to lock the manager object to avoid 
                       threads collisions.
   maxNumOfSlrv     -- Max number of resolvers in the resolvers list.
   hTransportMgr    -- Handle to the transport module manager.
   seed             -- Used to generate unique identifier to the resolvers
   hGeneralPool     -- general pool
   hMessagePool     -- message pool
   hElementPool     -- element pool
   pStack           -- pointer to the stack mgr
   hAppMgr          -- application handle for callbacks
   appEvHandlers    -- struct for application callbacks
   strDialPlanSuffix -- suffix for dial plan (like "e164.arpa")

 */

typedef struct {
    RvLogMgr*                 pLogMgr;
    RvLogSource*              pLogSrc;
#if defined(UPDATED_BY_SPIRENT)
    RvDnsEngine*              pDnsEngine[RVSIP_MAX_NUM_VDEVBLOCK];
#else
    RvDnsEngine*              pDnsEngine;
#endif
    RvInt32                   maxNumOfRslv;
    RLIST_POOL_HANDLE         hRslvListPool;
    RLIST_HANDLE              hRslvList;
    RvMutex*                  pMutex;
    RvSipTransportMgrHandle   hTransportMgr;
    RvRandomGenerator*        seed;
    HRPOOL                    hGeneralPool;
    HRPOOL                    hMessagePool;
    HRPOOL                    hElementPool;
#ifdef UPDATED_BY_SPIRENT
    HRPOOL                    hDnsServersPool;
#endif
    void*                     pStack;
    RvChar*                   strDialPlanSuffix;
    RvInt                     dialPlanLen;
} ResolverMgr;

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
                            IN  SipResolverMgrCfg*         pCfg,
                            IN  void*                         pStack,
                            IN  RvBool                        bTryServers,
                            IN  RvBool                        bTryDomains,
                            OUT ResolverMgr**              ppRslvMgr);

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
                            IN ResolverMgr*   pRslvMgr);

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
                   OUT    RvSipResolverHandle*      phRslv);

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
                            IN ResolverMgr*   pRslvMgr);

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
                                IN RvSipResolverHandle hRslv);

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
RvStatus RVCALLCONV ResolverMgrClearDnsCache(IN ResolverMgr *pRslvMgr); 
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
RvStatus RVCALLCONV ResolverMgrClearDnsEngineCache(IN ResolverMgr *pRslvMgr,RvUint16 vdevblock);
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
								 IN RvChar      *strDumpFile); 

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef RESOLVER_MGR_OBJECT_H*/

