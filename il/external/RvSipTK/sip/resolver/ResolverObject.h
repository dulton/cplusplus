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
 *                              <ResolverObject.c>
 *
 * This file defines the Resolver object, attributes and interface functions.
 * The resolver is responsible for all DNS operations in the SIP stack.
 * Assamptions:
 * 1. A resolver is always created by a Resolver and is held by it.
 * 2. 
 * 
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                    Dec 2004
 *********************************************************************************/

#ifndef RESOLVER_OBJECT_H
#define RESOLVER_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipResolverTypes.h"
#include "RvSipTransportDNSTypes.h"
#include "ResolverMgrObject.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
typedef enum{
    RESOLVER_STATE_UNDEFINED = -1,
    RESOLVER_STATE_TRY_A,
    RESOLVER_STATE_TRY_AAAA,
    RESOLVER_STATE_TRY_NAPTR,
    RESOLVER_STATE_TRY_SRV_UDP,
    RESOLVER_STATE_TRY_SRV_TCP,
    RESOLVER_STATE_TRY_SRV_TLS,
    RESOLVER_STATE_TRY_SRV_SCTP,
    RESOLVER_STATE_TRY_SRV_SINGLE,
    RESOLVER_STATE_RESOLVED,
    RESOLVER_STATE_UNRESOLVED
} ResolverState;

/* Resolver
   -----------------------------------------------------------------------
   The Resolver is in charge for interacting with DNS servers.
   Data Members:
   pRslvMgr       -- Resolver manager
   hOwner         -- The owner of the resolver. the owner makes a resolving 
                     request and is notified with the results.
   bIsAppRslv     -- Is this stack resolver or applicative resolver (relevant 
                     for locking)
   rslvUniqueId   -- A unique identifier for this resolver (randomly generated
                     when the resolver is created.
   pTripleLock    -- The triple lock of the owning object
   rslvTripleLock -- A triple lock for standalone resolvers. (UNUSED)
  
   eMode          -- The modus operandi of the resolving request
   strQueryString -- The string to resolve (e.g. "host.com")
   eScheme        -- Scheme for adding to SRV queries ("sip", "im" etc...)
   eState         -- The state of the resolving process. all mode are under 
                     one state enumeration.
   queryId        -- The id of the query given by Ares.
   hQueryPage     -- The buffer for the query. this buffer is required by 
                     Ares.
   hDns           -- DNS list to save data to. If the resolve is standalone
                     the list sill be constructed on the resolver page. if
                     is not standalone, the list will be held by the owner
   pfnReport      -- Results reporting callback.
 */

typedef struct {
    /* "constant" members */
    ResolverMgr*                pRslvMgr;
    RvSipAppResolverHandle      hOwner;
    RvBool                      bIsAppRslv;
    RvInt32                     rslvUniqueId;
    SipTripleLock*              pTripleLock;
    SipTripleLock               rslvTripleLock;
    
    /* Dns resolutions members*/
    RvSipResolverMode             eMode;
    RvChar*                     strQueryString;
    RvSipResolverScheme           eScheme;
    ResolverState               eState;
    RvUint32                    queryId;
    HPAGE                       hQueryPage;
    RvSipResolverReportDataEv   pfnReport;
    RvSipTransportDNSListHandle hDns;
    RvUint16                    knownPort;
    RvSipTransport              knownTransport;
    RvBool                      bIsSecure;
#if defined(UPDATED_BY_SPIRENT)
    RvUint16                    vdevblock; //vdevblock used in query
    RvChar                      if_name[RVSIP_TRANSPORT_IFNAME_SIZE];
    RvAddress                   localAddr;

#endif
} Resolver;

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * ResolverDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the Resolver 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv   - pointer to the Resolver
 **************************************************************************/
void RVCALLCONV ResolverDestruct(IN Resolver  *pRslv);

/***************************************************************************
 * ResolverInitialize
 * ------------------------------------------------------------------------
 * General: Initialize a new resolver in the Idle state.
 *          If a page is supplied the resolver will use it. Otherwise
 *          a new page will be allocated.
 *          Note: When calling this function the resolver manager is locked.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv      -- pointer to the new Resolver
 *            bIsAppRslv -- indicates whether the transmitter was created
 *                          by the application or by the stack.
 *            hAppRslv   -- Application handle to the transmitter.
 *            pTripleLock - A triple lock to use by the resolver. If NULL
 *                        is supplied the resolver will use its own lock.
 ***************************************************************************/
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
RvStatus RVCALLCONV ResolverInitialize(
                             IN Resolver*              pRslv,
                             IN RvBool                 bIsAppRslv,
                             IN RvSipAppResolverHandle hAppRslv,
							        IN SipTripleLock*         pTripleLock,
                             IN ResolverMgr*           pRslvMgr);
#else
RvStatus RVCALLCONV ResolverInitialize(
                             IN Resolver*              pRslv,
                             IN RvBool                 bIsAppRslv,
                             IN RvSipAppResolverHandle hAppRslv,
							 IN SipTripleLock*         pTripleLock);
#endif
/* SPIRENT_END */

/***************************************************************************
 * ResolverSetTripleLock
 * ------------------------------------------------------------------------
 * General: Sets the Resolver triple lock.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pRslv - The Resolver handle
 *           pTripleLock - The new triple lock
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV ResolverSetTripleLock(
                         IN Resolver*       pRslv,
                         IN SipTripleLock*     pTripleLock);

/***************************************************************************
 * ResolverChangeState
 * ------------------------------------------------------------------------
 * General: change resolver state 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv     - pointer to the Resolver
 *            eState  - new resolver state
 **************************************************************************/
void RVCALLCONV ResolverChangeState(IN Resolver*  pRslv,
                                   IN ResolverState eNewState);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * ResolverGetStateName
 * ------------------------------------------------------------------------
 * General: converts state name to string.
 *
 * Return Value: state as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    eState - state as enumeration
 * Output:  (-)
 ***************************************************************************/
RvChar* RVCALLCONV ResolverGetStateName(
                         IN ResolverState eState);
RvChar* RVCALLCONV ResolverGetModeName(
                         IN RvSipResolverMode eMode);
RvChar* RVCALLCONV ResolverGetSchemeName(
                         IN RvSipResolverScheme eScheme);

#else
#define ResolverGetStateName(_s) "-";
#define ResolverGetModeName(_m) "-";
#define ResolverGetSchemeName(_s) "-";
#endif /*(RV_LOGMASK != RV_LOGLEVEL_NONE)*/

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

/************************************************************************************
 * ResolverLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks Resolver according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv - pointer to the Resolver.
 ***********************************************************************************/
RvStatus RVCALLCONV ResolverLockAPI(
                                     IN  Resolver *pRslv);


/************************************************************************************
 * ResolverUnLockAPI
 * ----------------------------------------------------------------------------------
 * General: Release Resolver according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv - pointer to the Resolver.
 ***********************************************************************************/
void RVCALLCONV ResolverUnLockAPI(
                                 IN  Resolver *pRslv);



/************************************************************************************
 * ResolverUnLockEvent
 * ----------------------------------------------------------------------------------
 * General: UnLocks Resolver according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv - pointer to the Resolver.
 ***********************************************************************************/
void RVCALLCONV ResolverUnLockEvent(
                                   IN  Resolver *pRslv);



#else
#define ResolverLockAPI(a) (RV_OK)
#define ResolverUnLockAPI(a)
#define ResolverUnLockEvent(a)
#endif

/************************************************************************************
 * ResolverLockEvent
 * ----------------------------------------------------------------------------------
 * General: Locks Resolver according to MSG processing schema
 *          at the end of this function processingLock is locked. and hLock is locked.
 *          NOTE: If the stack is compiled in a non multithreaded way, the transmitter 
 *          can still be destructed while it is wating for a DNS response.
 *          this is why in non multithreaded mode we still check that the Resolver 
 *          location is not vacant
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv - pointer to the Resolver.
 ***********************************************************************************/
RvStatus RVCALLCONV ResolverLockEvent(IN  Resolver *pRslv);

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef RESOLVER_OBJECT_H*/

