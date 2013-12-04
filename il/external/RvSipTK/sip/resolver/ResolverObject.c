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
 * This file contains the functionality of the Resolver object
 * The transmitter object is used to send and to retransmit sip messages.
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                  January 2005
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipTransport.h"
#include "RvSipAddress.h"
#include "_SipCommonConversions.h"
#include "_SipCommonCore.h"
#include "_SipCommonTypes.h"
#include "_SipResolver.h"
#include "_SipTransport.h"
#include "ResolverObject.h"
#include "_SipCommonUtils.h"
#include "AdsRlist.h"
#include "rvansi.h"

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                         TRANSMITTER FUNCTIONS                         */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/*-----------------LOCKING FUNCTIONS------------------------*/
static RvStatus RVCALLCONV ResolverLock(
                         IN Resolver          *pRslv,
                         IN ResolverMgr       *pRslvMgr,
                         IN SipTripleLock        *pTripleLock,
                         IN RvInt32               identifier);

static void RVCALLCONV ResolverUnLock(
                                IN RvMutex *pLock,
                                IN RvLogMgr* logMgr);
#else
#define ResolverLock(a,b,c,d) (RV_OK)
#define ResolverUnLock(a)
#define TransmitterTerminateUnlock(a,b,c,d)
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

/***************************************************************************
 * ResolverDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the Resolver 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv   - pointer to the Resolver
 **************************************************************************/
void RVCALLCONV ResolverDestruct(IN Resolver  *pRslv)
{
    pRslv->rslvUniqueId = UNDEFINED;
}

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
                             IN ResolverMgr*           pRslvMgr)
#else
RvStatus RVCALLCONV ResolverInitialize(
                             IN Resolver*              pRslv,
                             IN RvBool                 bIsAppRslv,
                             IN RvSipAppResolverHandle hAppRslv,
							 IN SipTripleLock*         pTripleLock)
#endif
/* SPIRENT_END */
{
    /*initialize the transmitters fields*/

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    pRslv->pRslvMgr                   = pRslvMgr;
    pRslv->vdevblock=0;
    pRslv->if_name[0] = '\0';
    memset(&pRslv->localAddr,0,sizeof(pRslv->localAddr));
#endif
/* SPIRENT_END */

    pRslv->pfnReport                  = NULL;
    pRslv->hOwner                     = hAppRslv;
    pRslv->hDns                       = NULL;
    pRslv->eState                     = RESOLVER_STATE_UNDEFINED;
    pRslv->pTripleLock                = (pTripleLock == NULL) ? &pRslv->rslvTripleLock : pTripleLock;
    RvRandomGeneratorGetInRange(pRslv->pRslvMgr->seed,RV_INT32_MAX,(RvRandom*)&pRslv->rslvUniqueId);
    pRslv->bIsAppRslv                 = bIsAppRslv;
    pRslv->hQueryPage                 = NULL_PAGE;
    pRslv->eScheme                    = RVSIP_RESOLVER_SCHEME_UNDEFINED;
    pRslv->queryId                    = 0;

    RvLogInfo(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
              "ResolverInitialize - Resolver 0x%p was initialized,owner=0x%p",
               pRslv,pRslv->hOwner));

    return RV_OK;
}

/***************************************************************************
 * ResolverSetTripleLock
 * ------------------------------------------------------------------------
 * General: Sets the transmitter triple lock.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pRslv - The transmitter handle
 *           pTripleLock - The new triple lock
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV ResolverSetTripleLock(
                         IN Resolver*       pRslv,
                         IN SipTripleLock*     pTripleLock)
{
    if(pTripleLock == NULL)
    {
        return;
    }
    /* Set the new triple lock */
    RvMutexLock(&pRslv->rslvTripleLock.hLock,pRslv->pRslvMgr->pLogMgr);
    pRslv->pTripleLock = pTripleLock;
    RvMutexUnlock(&pRslv->rslvTripleLock.hLock,pRslv->pRslvMgr->pLogMgr);
}



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
                                   IN ResolverState eNewState)
{
    RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
        "ResolverChangeState - pRslv = 0x%p, change state %s -> %s",
        pRslv, ResolverGetStateName(pRslv->eState),ResolverGetStateName(eNewState)));
    pRslv->eState = eNewState;
}

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
                         IN ResolverState eState)
{
    switch(eState)
    {
    case RESOLVER_STATE_UNDEFINED:
        return "UNDEFINED";
    case RESOLVER_STATE_TRY_A:
        return "TRY_A";
    case RESOLVER_STATE_TRY_AAAA:
        return "TRY_AAAA";
    case RESOLVER_STATE_TRY_NAPTR:
        return "TRY_NAPTR";
    case RESOLVER_STATE_TRY_SRV_UDP:
        return "TRY_SRV_UDP";
    case RESOLVER_STATE_TRY_SRV_SCTP:
        return "TRY_SRV_SCTP";
    case RESOLVER_STATE_TRY_SRV_TCP:
        return "TRY_SRV_TCP";
    case RESOLVER_STATE_TRY_SRV_TLS:
        return "TRY_SRV_TLS";
    case RESOLVER_STATE_TRY_SRV_SINGLE:
        return "TRY_SRV_SINGLE";
    default:
        return "UNDEFINED";
    }
}
/***************************************************************************
 * ResolverGetModeName
 * ------------------------------------------------------------------------
 * General: converts mode name to string.
 *
 * Return Value: mode as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    eMode - mode as enumeration
 * Output:  (-)
 ***************************************************************************/
RvChar* RVCALLCONV ResolverGetModeName(
                         IN RvSipResolverMode eMode)
{
    switch(eMode)
    {
    case RVSIP_RESOLVER_MODE_UNDEFINED:
        return "UNDEFINED";
    case RVSIP_RESOLVER_MODE_FIND_TRANSPORT_BY_NAPTR:
        return "TRANSPORT BY NAPTR";
    case RVSIP_RESOLVER_MODE_FIND_TRANSPORT_BY_3WAY_SRV:
        return "TRANSPORT BY 3WAY SRV";
    case RVSIP_RESOLVER_MODE_FIND_IP_BY_HOST:
        return "IP BY HOST";
    case RVSIP_RESOLVER_MODE_FIND_URI_BY_NAPTR:
        return "URI BY NAPTR";
    case RVSIP_RESOLVER_MODE_FIND_HOSTPORT_BY_SRV_STRING:
        return "HOSTPORT BY SRV";
    case RVSIP_RESOLVER_MODE_FIND_HOSTPORT_BY_TRANSPORT:
        return "HOSTPORT BY TRANSPORT";
    default:
        return "UNDEFINED";
    }
}

/***************************************************************************
 * ResolverGetSchemeName
 * ------------------------------------------------------------------------
 * General: converts scheme name to string.
 *
 * Return Value: mode as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    eMode - mode as enumeration
 * Output:  (-)
 ***************************************************************************/
RvChar* RVCALLCONV ResolverGetSchemeName(
                         IN RvSipResolverScheme eScheme)
{
    switch(eScheme)
    {
    case RVSIP_RESOLVER_SCHEME_UNDEFINED:
        return "UNDEFINED";
    case RVSIP_RESOLVER_SCHEME_SIP:
        return "sip";
    case RVSIP_RESOLVER_SCHEME_PRES:
        return "pres";
    case RVSIP_RESOLVER_SCHEME_IM:
        return "im";
    default:
        return "UNDEFINED";
    }
}
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
                                     IN  Resolver *pRslv)
{
    RvStatus            rv;
    SipTripleLock      *pTripleLock;
    ResolverMgr     *pMgr;
    RvInt32             identifier;

    if (pRslv == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    for(;;)
    {

        RvMutexLock(&pRslv->rslvTripleLock.hLock,NULL);
        pMgr        = pRslv->pRslvMgr;
        pTripleLock = pRslv->pTripleLock;
        identifier  = pRslv->rslvUniqueId;
        RvMutexUnlock(&pRslv->rslvTripleLock.hLock,NULL);

        if ((pMgr == NULL) || (pTripleLock == NULL))
        {
            return RV_ERROR_NULLPTR;
        }

        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "ResolverLockAPI - Locking Resolver 0x%p: Triple Lock 0x%p", pRslv,
            pTripleLock));

        rv = ResolverLock(pRslv,pMgr,pTripleLock,identifier);
        if (rv != RV_OK)
        {
            return rv;
        }

        /*make sure the triple lock was not changed after just before the locking*/
        if (pRslv->pTripleLock == pTripleLock)
        {
            break;
        }
        RvLogWarning(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "ResolverLockAPI - Locking Resolver 0x%p: Tripple lock 0x%p was changed, reentering lockAPI",
            pRslv, pTripleLock));
        ResolverUnLock(&pTripleLock->hLock,pMgr->pLogMgr);

    } 

    rv = CRV2RV(RvSemaphoreTryWait(&pTripleLock->hApiLock,pMgr->pLogMgr));
    if ((rv != RV_ERROR_TRY_AGAIN) && (rv != RV_OK))
    {
        ResolverUnLock(&pTripleLock->hLock,pMgr->pLogMgr);
        return rv;
    }
    pTripleLock->apiCnt++;
    /* This thread actually locks the API lock */
    if(pTripleLock->objLockThreadId == 0)
    {
        pTripleLock->objLockThreadId = RvThreadCurrentId();
        RvMutexGetLockCounter(&pTripleLock->hLock,pMgr->pLogMgr,
                              &pTripleLock->threadIdLockCount);
    }


    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "ResolverLockAPI - Locking Resolver 0x%p: Triple Lock 0x%p apiCnt=%d exiting function",
             pRslv, pTripleLock, pTripleLock->apiCnt));

    return RV_OK;
}


/************************************************************************************
 * ResolverUnLockAPI
 * ----------------------------------------------------------------------------------
 * General: Relese Resolver according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv - pointer to the Resolver.
 ***********************************************************************************/
void RVCALLCONV ResolverUnLockAPI(
    IN  Resolver *pRslv)
{
    SipTripleLock        *pTripleLock;
    ResolverMgr       *pMgr;
    RvInt32               lockCnt = 0;

    if (pRslv == NULL)
    {
        return;
    }

    pMgr        = pRslv->pRslvMgr;
    pTripleLock = pRslv->pTripleLock;

    if ((pMgr == NULL) || (pTripleLock == NULL))
    {
        return;
    }
    RvMutexGetLockCounter(&pTripleLock->hLock,NULL,&lockCnt);

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "ResolverUnLockAPI - UnLocking Resolver 0x%p: Triple Lock 0x%p apiCnt=%d lockCnt=%d",
             pRslv, pTripleLock, pTripleLock->apiCnt,lockCnt));

    if (pTripleLock->apiCnt == 0)
    {
        RvLogExcep(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "ResolverUnLockAPI - UnLocking Resolver 0x%p: Triple Lock 0x%p apiCnt=%d",
             pRslv, pTripleLock, pTripleLock->apiCnt));
        return;
    }

    if (pTripleLock->apiCnt == 1)
    {
        RvSemaphorePost(&pTripleLock->hApiLock,pMgr->pLogMgr);
    }

    pTripleLock->apiCnt--;
    if(lockCnt == pTripleLock->threadIdLockCount)
    {
        pTripleLock->objLockThreadId = 0;
        pTripleLock->threadIdLockCount = UNDEFINED;
    }
    ResolverUnLock(&pTripleLock->hLock,pMgr->pLogMgr);
}


/************************************************************************************
 * ResolverLockEvent
 * ----------------------------------------------------------------------------------
 * General: Locks Resolver according to MSG processing schema
 *          at the end of this function processingLock is locked. and hLock is locked.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv - pointer to the Resolver.
 ***********************************************************************************/
RvStatus RVCALLCONV ResolverLockEvent(
    IN  Resolver *pRslv)
{
    RvStatus          rv           = RV_OK;
    RvBool            didILockAPI  = RV_FALSE;
    RvThreadId        currThreadId = RvThreadCurrentId();
    SipTripleLock    *pTripleLock;
    ResolverMgr      *pMgr;
    RvInt32           identifier;

    if (pRslv == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    for(;;)
    {
        RvMutexLock(&pRslv->rslvTripleLock.hLock,pRslv->pRslvMgr->pLogMgr);
        pMgr        = pRslv->pRslvMgr;
        pTripleLock = pRslv->pTripleLock;
        identifier  = pRslv->rslvUniqueId;
        RvMutexUnlock(&pRslv->rslvTripleLock.hLock,pRslv->pRslvMgr->pLogMgr);
        /*note: at this point the object locks can be replaced so after locking we need
          to check if we used the correct locks*/

        if (pTripleLock == NULL)
        {
            return RV_ERROR_BADPARAM;
        }

        /* In case that the current thread has already gained the API lock of */
        /* the object, locking again will be useless and will block the stack */
        if (pTripleLock->objLockThreadId == currThreadId)
        {
            RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "ResolverLockEvent - Exiting without locking Resolver 0x%p: Triple Lock 0x%p already locked",
                   pRslv, pTripleLock));

            return RV_OK;
        }
        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "ResolverLockEvent - Locking Resolver 0x%p: Triple Lock 0x%p apiCnt=%d",
                  pRslv, pTripleLock, pTripleLock->apiCnt));

        RvMutexLock(&pTripleLock->hProcessLock,pMgr->pLogMgr);

        for(;;)
        {
            rv = ResolverLock(pRslv,pMgr,pTripleLock,identifier);
            if (rv != RV_OK)
            {
                RvMutexUnlock(&pTripleLock->hProcessLock,pMgr->pLogMgr);
                if (didILockAPI)
                {
                    RvSemaphorePost(&pTripleLock->hApiLock,pMgr->pLogMgr);
                }
                return rv;
            }
            if (pTripleLock->apiCnt == 0)
            {
                if (didILockAPI)
                {
                    RvSemaphorePost(&pTripleLock->hApiLock,pMgr->pLogMgr);
                }
                break;
            }

            ResolverUnLock(&pTripleLock->hLock,pMgr->pLogMgr);

            rv = CRV2RV(RvSemaphoreWait(&pTripleLock->hApiLock,pMgr->pLogMgr));
            if (rv != RV_OK)
            {
                RvMutexUnlock(&pTripleLock->hProcessLock,pMgr->pLogMgr);
                return rv;
            }

            didILockAPI = RV_TRUE;
        } 

        /*check if the transaction lock pointer was changed. If so release the locks and lock
          the transaction again with the new locks*/
        if (pRslv->pTripleLock == pTripleLock)
        {
            break;
        }
        RvLogWarning(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                    "ResolverLockEvent - Locking Resolver 0x%p: Tripple lock %p was changed, reentering lockEvent",
                    pRslv, pTripleLock));
        ResolverUnLock(&pTripleLock->hLock,pMgr->pLogMgr);
        RvMutexUnlock(&pTripleLock->hProcessLock,pMgr->pLogMgr);
    }

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
              "ResolverLockEvent - Locking Resolver 0x%p: Triple Lock 0x%p apiCnt=%d exiting function",
              pRslv, pTripleLock, pTripleLock->apiCnt));

    return rv;
}


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
    IN  Resolver *pRslv)
{
    SipTripleLock        *pTripleLock;
    ResolverMgr       *pMgr;
    RvThreadId            currThreadId = RvThreadCurrentId();

    if (pRslv == NULL)
    {
        return;
    }

    pMgr        = pRslv->pRslvMgr;
    pTripleLock = pRslv->pTripleLock;

    if ((pMgr == NULL) || (pTripleLock == NULL))
    {
        return;
    }

    /* In case that the current thread has already gained the API lock of */
    /* the object wasn't lock internally, thus it shouldn't be unlocked   */
    if (pTripleLock->objLockThreadId == currThreadId)
    {
        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "ResolverUnLockEvent - Exiting without unlocking Resolver 0x%p: Triple Lock 0x%p apiCnt=%d wasn't locked",
                  pRslv, pTripleLock, pTripleLock->apiCnt));

        return;
    }

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "ResolverUnLockEvent - UnLocking Resolver 0x%p: Triple Lock 0x%p apiCnt=%d",
             pRslv, pTripleLock, pTripleLock->apiCnt));

    ResolverUnLock(&pTripleLock->hLock,pMgr->pLogMgr);
    RvMutexUnlock(&pTripleLock->hProcessLock,pMgr->pLogMgr);
}

#ifdef CompileResolverTerminateUnlock
/***************************************************************************
 * ResolverTerminateUnlock
 * ------------------------------------------------------------------------
 * General: Unlocks processing and transmitter locks. This function is called to unlock
 *          the locks that were locked by the LockEvent() function. The UnlockEvent() function
 *          is not called because in this stage the transmitter object was already destructed.
 *          and could have been reallocated
 * Return Value: - RV_Succees
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  pLogMgr            - The log mgr pointer
 *            pLogSrc        - log handler
 *            hObjLock        - The transaction lock.
 *            hProcessingLock - The transaction processing lock
 ***************************************************************************/
static void RVCALLCONV ResolverTerminateUnlock(IN RvLogMgr              *pLogMgr,
                                                       IN RvLogSource          *pLogSrc,
                                                       IN RvMutex              *hObjLock,
                                                       IN RvMutex              *hProcessingLock)
{
    RvLogSync(pLogSrc ,(pLogSrc ,
        "ResolverTerminateUnlock - Unlocking Object lock 0x%p: Processing Lock 0x%p",
        hObjLock, hProcessingLock));

    if (hObjLock != NULL)
    {
        RvMutexUnlock(hObjLock,pLogMgr);
    }

    if (hProcessingLock != NULL)
    {
        RvMutexUnlock(hProcessingLock,pLogMgr);
    }

    return;
}
#endif /*CompileResolverTerminateUnlock*/

#else /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) - single thread*/
/************************************************************************************
 * ResolverLockEvent
 * ----------------------------------------------------------------------------------
 * General: Locks Resolver according to MSG processing schema
 *          at the end of this function processingLock is locked. and hLock is locked.
 *          NOTE: If the stack is compiled in a non multithreaded way, the transmitter
 *          can still be destructed while it is wating for a DNS response.
 *          this is why in non multithreaded mode we still check that the transmitter
 *          location is not vacant
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv - pointer to the Resolver.
 ***********************************************************************************/
RvStatus RVCALLCONV ResolverLockEvent(
    IN  Resolver *pRslv)
{
    if (RV_FALSE == RLIST_IsElementVacant(pRslv->pRslvMgr->hRslvListPool,
                                          pRslv->pRslvMgr->hRslvList,
                                          (RLIST_ITEM_HANDLE)(pRslv)))
    {
        /*The transmitter is valid*/
        return RV_OK;
    }
    RvLogWarning(pRslv->pRslvMgr->pLogSrc ,(pRslv->pRslvMgr->pLogSrc ,
        "ResolverLockEvent - Resolver 0x%p: RLIST_IsElementVacant returned TRUE!!!",
        pRslv));
    return RV_ERROR_INVALID_HANDLE;
}

#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/




/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/***************************************************************************
 * ResolverLock
 * ------------------------------------------------------------------------
 * General: Lock the transmitter
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  pRslv           - The transmitter to lock.
 *            pRslvMgr        - pointer to the transmitter manager
 *            pTripleLock     - triple lock of the transmitter
 ***************************************************************************/
static RvStatus RVCALLCONV ResolverLock(
                         IN Resolver          *pRslv,
                         IN ResolverMgr       *pRslvMgr,
                         IN SipTripleLock        *pTripleLock,
                         IN RvInt32               identifier)
{

    if ((pRslv == NULL) || (pRslvMgr == NULL) || (pTripleLock == NULL))
    {
        return RV_ERROR_NULLPTR;
    }
    RvMutexLock(&pTripleLock->hLock,pRslvMgr->pLogMgr);
    if (RV_FALSE == RLIST_IsElementVacant(pRslvMgr->hRslvListPool,
                                          pRslvMgr->hRslvList,
                                          (RLIST_ITEM_HANDLE)(pRslv))
        && (identifier == pRslv->rslvUniqueId))
    {
        /*The transmitter is valid*/
        return RV_OK;
    }
    else
    {
        RvMutexUnlock(&pTripleLock->hLock,pRslvMgr->pLogMgr);
        RvLogWarning(pRslvMgr->pLogSrc ,(pRslvMgr->pLogSrc ,
            "ResolverLock - Resolver 0x%p: RLIST_IsElementVacant returned TRUE!!!",
            pRslv));
    }
    return RV_ERROR_DESTRUCTED;
}


/***************************************************************************
 * ResolverUnLock
 * ------------------------------------------------------------------------
 * General: Unlocks the transmitter's lock.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hLock - The lock to un-lock.
 ***************************************************************************/
static void RVCALLCONV ResolverUnLock(
                                IN RvMutex *pLock,
                                IN RvLogMgr* logMgr)
{
    if (NULL != pLock)
    {
        RvMutexUnlock(pLock,logMgr);
    }
}

#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/
