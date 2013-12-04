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
 *                              <RvSipMid.c>
 *
 * This file contains all API functions relevant to intrer acting with core lever 
 * functionality, including select poll and timers.
 *
 * select/poll 
 * ------------------------
 * Enables an application to register event for select loop or poll queries.
 * Enables an application to perform a select loop or a poll loop
 *
 * timers
 * -----------------
 * Enables an application to set and release timers.
 * Timer can be set on threads that ran one of RADVISION toolkits only, as only those 
 * threads use the RADVISION select() loop.
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                   March 2004
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonConversions.h"
#include "rvccore.h"
#include "rvbase64.h"
#include <string.h>

#include "RvSipMidTypes.h"
#include "MidMgrObject.h"
#include "MidSelect.h"
#include "RvSipMid.h"
#include "_SipCommonCore.h"
#include "MidTimer.h"
/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                         MID MANAGER API FUNCTIONS                     */
/*-----------------------------------------------------------------------*/

/************************************************************************************
 * RvSipMidInit
 * ----------------------------------------------------------------------------------
 * General: Starts the Mid layer services. before calling to any of the function 
 *          of this module, the application must call this function
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   -
 * Output:  -
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidInit(void)
{
    return CRV2RV(RvCBaseInit());
}

/************************************************************************************
 * RvSipMidEnd
 * ----------------------------------------------------------------------------------
 * General: Ends the Mid layer services. 
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   -
 * Output:  -
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidEnd(void)
{
    return CRV2RV(RvCBaseEnd());
}

/************************************************************************************
 * RvSipMidConstruct
 * ----------------------------------------------------------------------------------
 * General: Constructs and initializes the middle layer. This function allocates the
 *          required memory and constructs middle layer objects acording to the given 
 *          configuration. The function returns a handle to the middle layer manager. 
 *          You need this handle in order to use the middle layer APIs.
 *          when finishig using the middle layer call RvSipMidPrepareDestruct() if 
 *          needed and RvSipMidDesstruct() to free all resources.
 *          allocated resources
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidCfg     - Structure containing middle layer configuration parameters.
 *          sizeOfCfg   - The size of the configuration structure.
 * Output:  phMidMgr     - Handle to the middle layer manager.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidConstruct(
                                         IN    RvInt32               sizeOfCfg,
                                         IN    RvSipMidCfg*          pMidCfg,
                                         OUT   RvSipMidMgrHandle*    phMidMgr)
{
    RvStatus    rv      = RV_OK;
    MidMgr*     pMidMgr = NULL;
    RvUint32   minStructSize = RvMin((RvUint32)sizeOfCfg,sizeof(RvSipMidCfg));
    RvSipMidCfg internalCfg;


    if (NULL == pMidCfg || NULL == phMidMgr)
    {
        return RV_ERROR_NULLPTR;
    }

    memset(&internalCfg,0,sizeof(internalCfg));
    memcpy(&internalCfg,pMidCfg,minStructSize);
    
    rv = RvMemoryAlloc(NULL,sizeof(MidMgr),(RvLogMgr*)internalCfg.hLog,(void**)&pMidMgr);
    if (RV_OK != rv)
    {
        return rv;
    }
    memset(pMidMgr,0,sizeof(MidMgr));
    pMidMgr->pLogMgr = (RvLogMgr*)internalCfg.hLog;

    rv = RvMemoryAlloc(NULL,sizeof(RvMutex),pMidMgr->pLogMgr,(void**)&pMidMgr->pSelectMutex);
    if (RV_OK != rv)
    {
        pMidMgr->pSelectMutex = NULL;
        RvSipMidDestruct((RvSipMidMgrHandle)pMidMgr);
        return rv;
    }
    rv = RvMutexConstruct(pMidMgr->pLogMgr,pMidMgr->pSelectMutex);
    if (RV_OK != rv)
    {
        RvSipMidDestruct((RvSipMidMgrHandle)pMidMgr);
        return rv;
    }

    rv = RvMemoryAlloc(NULL,sizeof(RvMutex),pMidMgr->pLogMgr,(void**)&pMidMgr->pTimerMutex);
    if (RV_OK != rv)
    {
        pMidMgr->pTimerMutex = NULL;
        RvSipMidDestruct((RvSipMidMgrHandle)pMidMgr);
        return rv;
    }
    rv = RvMutexConstruct(pMidMgr->pLogMgr,pMidMgr->pTimerMutex);
    if (RV_OK != rv)
    {
        RvSipMidDestruct((RvSipMidMgrHandle)pMidMgr);
        return rv;
    }
    rv = MidMgrAllocateResources(pMidMgr,&internalCfg);
    if (RV_OK != rv)
    {
        RvSipMidDestruct((RvSipMidMgrHandle)pMidMgr);
        return rv;
    }
    *phMidMgr = (RvSipMidMgrHandle)pMidMgr;
    return rv;
}

/************************************************************************************
 * RvSipMidPrepareDestruct
 * ----------------------------------------------------------------------------------
 * General: This function will free all application resources. after calling this 
 *          function, allow enough time for other threads (if there are any) to finish 
 *          any calls to middle functions and call RvSipMidDestruct().
 *          If the application is sure that no other threads wait tp preform actions 
 *          on the mid layer, it can call RvSipMidDestruct() directly.
 *          After calling this function the application is not allowed to set timers 
 *          or register on select events
 *          All Application timers will be released.
 *          All select registrations will be removed.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hMidMgr     - Handle to the middle layer manager
 * Output:
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidPrepareDestruct(IN RvSipMidMgrHandle    hMidMgr)
{
    MidMgr*     pMidMgr = (MidMgr*)hMidMgr;
    
    if (NULL == pMidMgr)
    {
        return RV_ERROR_NULLPTR;
    }
    
    pMidMgr->bIsDestructing = RV_TRUE;
    
    /* make sure no events are registered on select engine or timer */
    MidMgrRemovePendingRegistrations(pMidMgr);

    return RV_OK;
}

/************************************************************************************
 * RvSipMidDestruct
 * ----------------------------------------------------------------------------------
 * General: This function will free all manager resources. after calling this 
 *          function the application is not allowed to call on file descriptors 
 *          or set timers.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hMidMgr     - Handle to the middle layer manager
 * Output:
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidDestruct(IN RvSipMidMgrHandle    hMidMgr)
{
    MidMgr*     pMidMgr = (MidMgr*)hMidMgr;
    RvLogMgr*   pLogMgr = NULL;

    if (NULL == pMidMgr)
    {
        return RV_ERROR_NULLPTR;
    }

    pLogMgr = pMidMgr->pLogMgr;
    /* remove registrations and stop accepting new registrations */
    if (RV_FALSE == pMidMgr->bIsDestructing)
    {
        RvSipMidPrepareDestruct(hMidMgr);
    }

    /* destruct and free mutexes */
    if (NULL != pMidMgr->pSelectMutex)
    {
        RvMutexDestruct(pMidMgr->pSelectMutex,pMidMgr->pLogMgr);
        RvMemoryFree((void*)pMidMgr->pSelectMutex,pMidMgr->pLogMgr);
        pMidMgr->pSelectMutex = NULL;
    }
    if (NULL != pMidMgr->pTimerMutex)
    {
        RvMutexDestruct(pMidMgr->pTimerMutex,pMidMgr->pLogMgr);
        RvMemoryFree((void*)pMidMgr->pTimerMutex,pMidMgr->pLogMgr);
        pMidMgr->pTimerMutex = NULL;
    }

    /*free allocated data structures */
    MidMgrFreeResources(pMidMgr);

    /* free mgr memory */
    RvMemoryFree((void*)pMidMgr,pLogMgr);
    
    return RV_OK;
}

/************************************************************************************
 * RvSipMidSetLog
 * ----------------------------------------------------------------------------------
 * General: Sets a log handle to the mid layer. Use this function if mid layed was 
 *          initiated before the stack.
 *          Use RvSipStackGetLogHandle() to get the log handle from the SIP Toolkit.
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hMidMgr   -- handle to the middle layer manager.
 *          hLog      -- log handle
 * Output:  -
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidSetLog(IN RvSipMidMgrHandle     hMidMgr,
                                         IN RV_LOG_Handle         hLog)
{
    MidMgr*     pMidMgr = (MidMgr*)hMidMgr;
    if (NULL == pMidMgr)
    {
        return RV_ERROR_NULLPTR;
    }
    pMidMgr->pLogMgr = (RvLogMgr*)hLog;
    return RV_OK;
}

/************************************************************************************
 * RvSipMidSelectCallOn
 * ----------------------------------------------------------------------------------
 * General: registers a file descriptor to the select loop. You can register to 
 *          listen on read or write events, and provide a callback that will be called 
 *          when the select exits due to activity on that file descriptor
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hMidMgr   -- handle to the middle layer manager.
 *          fd        -- OS file descriptor
 *          events    -- read/write
 *          pCallBack -- user callback
 *          ctx       -- usr context
 * Output:  
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidSelectCallOn(IN RvSipMidMgrHandle    hMidMgr,
                                               IN RvInt32              fd,
                                               IN RvSipMidSelectEvent  events,
                                               IN RvSipMidSelectEv     pCallBack,
                                               IN void*                ctx)
{
    RvStatus    rv      = RV_OK;
    MidMgr*     pMidMgr = (MidMgr*)hMidMgr;
    if (NULL == pMidMgr)
    {
        return RV_ERROR_NULLPTR;
    }
    if (RV_TRUE == pMidMgr->bIsDestructing)
    {
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (NULL == pMidMgr->userFDPool)
    {
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = MidSelectCallOn(pMidMgr,fd,events,pCallBack,ctx);
    return rv;
}

/******************************************************************************
 * RvSipMidSelectSetMaxDescs
 * ------------------------------------------------------------------------
 * General: Set the amount of file descriptors that the Select module can handle in a single
 *          select engine. This is also the value of the highest file descriptor possible.
 *          This function MUST be called before stack/middle layer initialization
 * Return Value: RV_OK on success, error code otherwise
 *               (see RV_SIP_DEF.h, rverror.h)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  maxDescs - Maximum value of file descriptor possible
 * Output: - 
 *****************************************************************************/
RVAPI RvStatus RvSipMidSelectSetMaxDescs(IN RvUint32 maxDescs)
{
    RvStatus crv;
    crv = RvSelectSetMaxFileDescriptors(maxDescs);
    return CRV2RV(crv);
}

/******************************************************************************
 * RvSipMidSelectGetMaxDesc
 * ------------------------------------------------------------------------
 * General: Get the current value used as the maximum value for a file descriptor 
 *          by the select procedures.
 * Return Value: RV_OK on success, error code otherwise
 *               (see RV_SIP_DEF.h, rverror.h)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  -
 * Output: pMaxFds - place to store maximun number of file descriptors
 *****************************************************************************/
RVAPI RvStatus RvSipMidSelectGetMaxDesc(OUT RvUint32 *pMaxFds)
{
    RvUint32 maxFds;

    maxFds = RvSelectGetMaxFileDescriptors();

    if (maxFds == 0)
    {
        return RV_ERROR_UNKNOWN;
    }
    if (NULL != pMaxFds)
    {
        *pMaxFds = maxFds;
    }
    else
    {
        return RV_ERROR_NULLPTR;
    }
    return RV_OK;
}
/* The following functions are only relevant for systems supporting the select() interface */
#if defined(FD_SETSIZE)
#if (RV_SELECT_TYPE == RV_SELECT_SELECT)
/******************************************************************************
 * RvSipMidSelectGetEventsRegistration
 * ------------------------------------------------------------------------
 * General: Get Select file descriptors for select operation
 *          NOTE: 1. This function cann be called only in the thread that 
 *                   initiated the stack.
 *                2. This function resets the fd sets that was given to it.
 *                   if you intent to add file descriptors to the fd sets do 
 *                   so AFTER calling this function
 *
 * Return Value: RV_OK on success, error code otherwise
 *               (see RV_SIP_DEF.h, rverror.h)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMidMgr         - handle to the middle layer manager
 *         fdSetLen        - the length of the file descriptors set
 * Output: pMaxfd          - maximum number of file descriptors
 *         rdSet           - pointer to read file descriptor set
 *         wrSet           - pointer to write file descriptor set
 *         exSet           - pointer to exeption file descriptor set (reserved for future use)
 *         pTimeOut        - time out the stack want to give to select. (-1 means ingfinite)      
 *****************************************************************************/
RVAPI RvStatus RvSipMidSelectGetEventsRegistration(IN  RvSipMidMgrHandle               hMidMgr,
                                                    IN  RvInt                           fdSetLen,
                                                    OUT RvInt*                          pMaxfd,
                                                    OUT fd_set*                         rdSet,
                                                    OUT fd_set*                         wrSet,
                                                    OUT fd_set*                         exSet,
                                                    OUT RvUint32*                       pTimeOut)
{
    RvSelectEngine* selectEngine;
    RvTimerQueue* timerQueue;
    RvStatus rv;
    MidMgr*     pMidMgr = (MidMgr*)hMidMgr;
    RV_UNUSED_ARG(exSet);
    RV_UNUSED_ARG(fdSetLen);
    
    if (NULL == pMidMgr)
    {
        return RV_ERROR_NULLPTR;
    }

    /* Find the select engine for this thread at first */
    rv = RvSelectGetThreadEngine(pMidMgr->pLogMgr, &selectEngine);
    if ((rv != RV_OK) || (selectEngine == NULL))
    {
        return rv;
    }
    /* Get fd_set bits from the select engine */
    rv = RvSelectGetSelectFds(selectEngine, pMaxfd, rdSet, wrSet);
    if (rv != RV_OK)
    {
        return rv;
    }
    rv = RvSelectGetTimeoutInfo(selectEngine, NULL, &timerQueue);
    if (rv != RV_OK)
    {
        timerQueue = NULL;
    }
    if (timerQueue != NULL)
    {
        RvInt64 nextEvent = RvInt64Const(0, 0, 0);

        rv = RvTimerQueueNextEvent(timerQueue, &nextEvent);
        if (rv == RV_OK)
        {
            if (RvInt64IsGreaterThan(nextEvent, RvInt64Const(0, 0, 0)))
            {
                /* Convert it to milliseconds, rounding up */
                *pTimeOut = RvInt64ToRvUint32(RvInt64Div(
                    RvInt64Sub(RvInt64Add(nextEvent, RV_TIME64_NSECPERMSEC), RvInt64Const(1, 0, 1)),
                        RV_TIME64_NSECPERMSEC));
            }
            else
            {
                /* Seems like we have timers to handle - enter select() without blocking there */
                *pTimeOut = 0;
            }
        }
        else if (RvErrorGetCode(rv) == RV_TIMER_WARNING_QUEUEEMPTY)
        {
            /* Seems like we have no timers to handle - we should give some kind of an "infinite" timeout value */
            *pTimeOut = (RvUint32)-1;
        }

        rv = RV_OK; /* Make sure we select() */
    }

    return rv;
}

/******************************************************************************
 * RvSipMidSelectEventsHandling
 * ------------------------------------------------------------------------
 * General: Handle events for sockets opened by Stack
 *          NOTE: this function can be called only in the thread that 
 *          initiated the stack
 * Return Value: RV_OK on success, error code otherwise
 *               (see RV_SIP_DEF.h, rverror.h)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMidMgr   - handle to the middle layer manager
 *         rdSet     - not in use since SIP TK 4.5 GA3
 *         wrSet     - not in use since SIP TK 4.5 GA3
 *         exSet     - not in use since SIP TK 4.5 GA3
 *         numFds    - not in use since SIP TK 4.5 GA3
 *         numEvents - not in use since SIP TK 4.5 GA3
 *****************************************************************************/
RVAPI RvStatus RvSipMidSelectEventsHandling(IN RvSipMidMgrHandle  hMidMgr,
                                            IN fd_set*            rdSet,
                                            IN fd_set*            wrSet,
                                            IN fd_set*            exSet,
                                            IN RvInt              numFds,
                                            IN RvInt              numEvents)
{
    RvStatus        rv;
    MidMgr*         pMidMgr = (MidMgr*)hMidMgr;
    RvSelectEngine* selectEngine = NULL;

    RV_UNUSED_ARG(rdSet);
    RV_UNUSED_ARG(wrSet);
    RV_UNUSED_ARG(exSet);
    RV_UNUSED_ARG(numFds);
    RV_UNUSED_ARG(numEvents);
    
    if (NULL == pMidMgr)
    {
        return RV_ERROR_NULLPTR;
    }

    /* Find the select engine for this thread at first */
    rv = RvSelectGetThreadEngine(pMidMgr->pLogMgr, &selectEngine);
    if ((rv != RV_OK) || (selectEngine == NULL))
    {
        return rv;
    }
    /* Handle the events registered currently in SelectEngine:
       call to RvSelectWaitAndBlock with zero timeout will cause
       handling of events. */
    rv = RvSelectWaitAndBlock(selectEngine, RvUint64Const(0,0));
    return rv;
}
#endif /*#if (RV_SELECT_TYPE == RV_SELECT_SELECT)*/
#endif /*defined(FD_SETSIZE)*/

#if (RV_SELECT_TYPE == RV_SELECT_POLL)
/******************************************************************************
 * RvSipMidPollGetEventsRegistration
 * ------------------------------------------------------------------------
 * General: Get Select file descriptors for select operation
 *          NOTE: 1. This function cann be called only in the thread that 
 *                   initiated the stack.
 *                2. This function resets the fd sets that was given to it.
 *                   if you intent to add file descriptors to the fd sets do 
 *                   so AFTER calling this function
 *
 * Return Value: RV_OK on success, error code otherwise
 *               (see RV_SIP_DEF.h, rverror.h)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMidMgr         - handle to the middle layer manager
 *         len             - the length of the poll fd set
 * Output: pollFdSet       - pointer poll fd set to poll 
 *         pNum            - the number of file descriptors on the poll set      
 *         pTimeOut        - time out the stack want to give to poll. (-1 means ingfinite)      
 *****************************************************************************/
RVAPI RvStatus RvSipMidPollGetEventsRegistration(IN RvSipMidMgrHandle               hMidMgr,
                                                IN  RvInt           len,
                                                OUT struct pollfd*  pollFdSet,
                                                OUT RvInt*          pNum,
                                                OUT RvUint32*       pTimeOut)
{
    RvSelectEngine* selectEngine;
    RvTimerQueue* timerQueue;
    RvUint32 numFds;
    RvStatus rv;
    MidMgr*     pMidMgr = (MidMgr*)hMidMgr;
    
    if (NULL == pMidMgr)
    {
        return RV_ERROR_NULLPTR;
    }

    /* Find the select engine for this thread at first */
    rv = RvSelectGetThreadEngine(pMidMgr->pLogMgr, &selectEngine);
    if ((rv != RV_OK) || (selectEngine == NULL))
    {
        return rv;
    }
    /* Get pollfd array from the select engine */
    numFds = (RvUint32)len;
    rv = RvSelectGetPollFds(selectEngine, &numFds, pollFdSet);
    if (rv != RV_OK)
    {
        return rv;
    }
    *pNum = (RvInt)numFds;

    rv = RvSelectGetTimeoutInfo(selectEngine, NULL, &timerQueue);
    if (rv != RV_OK)
    {
        timerQueue = NULL;
    }
    if (timerQueue != NULL)
    {
        RvInt64 nextEvent = RvInt64Const(0, 0, 0);

        rv = RvTimerQueueNextEvent(timerQueue, &nextEvent);
        if (rv == RV_OK)
        {
            if (RvInt64IsGreaterThan(nextEvent, RvInt64Const(0, 0, 0)))
            {
                /* Convert it to milliseconds, rounding up */
                *pTimeOut = RvInt64ToRvUint32(RvInt64Div(
                    RvInt64Sub(RvInt64Add(nextEvent, RV_TIME64_NSECPERMSEC), RvInt64Const(1, 0, 1)),
                        RV_TIME64_NSECPERMSEC));
            }
            else
            {
                /* Seems like we have timers to handle - enter poll() without blocking there */
                *pTimeOut = 0;
            }
        }
        else if (RvErrorGetCode(rv) == RV_TIMER_WARNING_QUEUEEMPTY)
        {
            /* Seems like we have no timers to handle - we should give some kind of an "infinite" timeout value */
            *pTimeOut = (RvUint32)-1;
        }

        rv = RV_OK; /* Make sure we poll() */
    }

    return rv;
}

/******************************************************************************
 * RvSipMidPollEventsHandling
 * ------------------------------------------------------------------------
 * General: Handle events gotten from poll procedure
 *          NOTE: this function can be called only in the thread that 
 *          initiated the stack
 *
 * Return Value: RV_OK on success, error code otherwise
 *               (see RV_SIP_DEF.h, rverror.h)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMidMgr         - handle to the middle layer manager
 *         num             - number of file descriptors
 *         pollFdSet       - pointer poll fd set from poll 
 * Output: 
 *           
 *****************************************************************************/
RVAPI RvStatus RvSipMidPollEventsHandling(IN RvSipMidMgrHandle  hMidMgr,
                                            IN struct pollfd*     pollFdSet,
                                            IN RvInt              num,
                                            IN RvInt              numEvents)
{
    RvSelectEngine*     selectEngine    = NULL;
    RvTimerQueue*       timerQueue      = NULL;
    RvStatus            rv              = RV_OK;
    MidMgr*             pMidMgr         = (MidMgr*)hMidMgr;
    RvSelectEngine*     checkEngine     = NULL;
    
    if (NULL == pMidMgr)
    {
        return RV_ERROR_NULLPTR;
    }

    /* Find the select engine for this thread at first */
    rv = RvSelectGetThreadEngine(pMidMgr->pLogMgr, &selectEngine);
    if ((rv != RV_OK) || (selectEngine == NULL))
    {
        return rv;
    }

    /* Handle the events registered currently in SelectEngine:
       call to RvSelectWaitAndBlock with zero timeout will cause
       handling of events. */
    rv = RvSelectWaitAndBlock(selectEngine, RvUint64Const(0,0));
    return rv;
}
#endif  /* (RV_SELECT_TYPE == RV_SELECT_POLL) */


/***************************************************************************
 * RvSipMidMemAlloc
 * ------------------------------------------------------------------------
 * General: Allocates memory for application use.
 *          This function can be called only after core services were initialized.
 * Return Value: Pointer to newly allocated memory if successful.
 *               NULL o/w.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     size   - Allocation size in bytes.
 ***************************************************************************/
RVAPI void* RVCALLCONV RvSipMidMemAlloc(IN RvInt32 size)
{
    void*      allocation  = NULL;

    if (RV_OK == RvMemoryAlloc(NULL,size,NULL,&allocation))
    {
        return allocation;
    }
    return NULL;
}


/***************************************************************************
 * RvSipMidMemFree
 * ------------------------------------------------------------------------
 * General: Frees memory allocated by RvSipMemAlloc().
 *          Only memory allocation that were done using RvSipMemAlloc() should
 *          be freed with this function.
 * Return Value: (-).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     memptr     - memory to free.
 ***************************************************************************/
RVAPI void RVCALLCONV RvSipMidMemFree(IN void* memptr)
{
    RvMemoryFree(memptr,NULL);
}

/***************************************************************************
 * RvSipMidTimeInMilliGet
 * ------------------------------------------------------------------------
 * General: Gets a timestamp value in milliseconds.
 * Return Value: The timestamp value in milliseconds.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     
 ***************************************************************************/
RVAPI RvUint32 RVCALLCONV RvSipMidTimeInMilliGet(void)
{
    return SipCommonCoreRvTimestampGet(IN_MSEC);
}

/***************************************************************************
 * RvSipMidTimeInSecondsGet
 * ------------------------------------------------------------------------
 * General: Gets a timestamp value in seconds.
 * Return Value: The timestamp value in seconds.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     
 ***************************************************************************/
RVAPI RvUint32 RVCALLCONV RvSipMidTimeInSecondsGet(void)
{
    return SipCommonCoreRvTimestampGet(IN_SEC);
}

/************************************************************************************
 * RvSipMidTimerSet
 * ----------------------------------------------------------------------------------
 * General: Sets a new timer.
 *          when a timer expires, the resources it consumes will be released automaticlly.
 *          it is forbidden to call RvSipMidTimerReset() in the timer callback.
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hMidMgr     -- handle to the middle layer manager.
 *          miliTimeOut -- experation specified in miliseconds
 *          cb          -- application callback
 *          ctx         -- context to be called when timer expires
 * Output:  phTimer     -- a newly allocated timer
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidTimerSet(IN RvSipMidMgrHandle       hMidMgr,
                                    IN RvUint32                 miliTimeOut,
                                    IN RvSipMidTimerExpEv       cb,
                                    IN void*                    ctx,
                                    OUT RvSipMidTimerHandle*    phTimer)
{
    MidMgr*     pMidMgr = (MidMgr*)hMidMgr;
    
    if (NULL == hMidMgr || NULL == cb || NULL == phTimer)
    {
        return RV_ERROR_NULLPTR;
    }
    if (RV_TRUE == pMidMgr->bIsDestructing)
    {
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if (NULL == pMidMgr->userTimerPool)
    {
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    return MidTimerSet((MidMgr*)hMidMgr,miliTimeOut,cb,ctx,(MidTimer**)phTimer);

}

/************************************************************************************
 * RvSipMidTimerReset
 * ----------------------------------------------------------------------------------
 * General: releases a timer
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidMgr   -- pointer to the middle layer manager.
 *          hTimer    -- handle of timer to delete
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidTimerReset(IN RvSipMidMgrHandle       hMidMgr,
                                             IN RvSipMidTimerHandle    hTimer)
{
    MidMgr*     pMidMgr = (MidMgr*)hMidMgr;

    if (NULL == hMidMgr || NULL == hTimer)
    {
        return RV_ERROR_NULLPTR;
    }
    if (RV_TRUE == pMidMgr->bIsDestructing)
    {
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if (NULL == pMidMgr->userTimerPool)
    {
        return RV_ERROR_ILLEGAL_ACTION;
    }

    return MidTimerReset((MidMgr*)hMidMgr,(MidTimer*)hTimer);
}

/************************************************************************************
 * RvSipMidTimerGetNextExpiration
 * ----------------------------------------------------------------------------------
 * General: get the time left untill the next timer expiration 
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidMgr         -- pointer to the middle layer manager.
 * Output:  pExpirationTime -- time left untill the next timer expiration
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidTimerGetNextExpiration(IN RvSipMidMgrHandle       hMidMgr,
                                                         OUT RvUint32*              pExpirationTime)
{
    MidMgr*     pMidMgr     = (MidMgr*)hMidMgr;
    RvInt64     nextevent   = RvInt64Const(0,0,0);
    RvStatus    crv         = RV_OK;

    if (NULL == pMidMgr || NULL == pExpirationTime)
    {
        return RV_ERROR_NULLPTR;
    }
    crv = RvTimerQueueNextEvent(&pMidMgr->pSelect->tqueue,&nextevent);
    if (RV_OK != crv)
    {
        return CRV2RV(crv);
    }
    *pExpirationTime = RvInt64ToRvUint32(RvInt64Div(nextevent,RV_TIME64_NSECPERMSEC));
    return RV_OK;
}

/************************************************************************************
 * RvSipMidTimerGetTimerExpiration
 * ----------------------------------------------------------------------------------
 * General: get the time left untill the timer expiration 
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidMgr         -- pointer to the middle layer manager.
 *          hTimer          -- handle of timer to get info on
 * Output:  pExpirationTime -- time left untill the timer expiration
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidTimerGetTimerExpiration(IN RvSipMidMgrHandle       hMidMgr,
                                                          IN RvSipMidTimerHandle    hTimer,
                                                          OUT RvUint32*              pExpirationTime)
{
    MidTimer*   pTimer  = (MidTimer*)hTimer;
    RvInt64     poptime = RvInt64Const(0,0,0);
    RvStatus    crv     = RV_OK;

    if (NULL == hMidMgr || NULL == hTimer || NULL == pExpirationTime)
    {
        return RV_ERROR_NULLPTR;
    }
    crv = RvTimerGetRemainingTime(&pTimer->sipTimer.hTimer,&poptime);
    if (RV_OK != crv)
    {
        return CRV2RV(crv);
    }
    *pExpirationTime = RvInt64ToRvUint32(RvInt64Div(poptime,RV_TIME64_NSECPERMSEC));
    return RV_OK;
}

/************************************************************************************
 * RvSipMidMainThreadClean
 * ----------------------------------------------------------------------------------
 * General: releases all resources taken by the main thread.
 *          if an application wishes to clean all the resources taken by SIP Stack 
 *          main thread, it should call this function. This function should be called 
 *          only after ALL RADVISION's toolkits have been destructed
 *          You should call this function before RvSipMidEnd()
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   -
 * Output:  -
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidMainThreadClean(void)
{
 /*   RvStatus    crv = RV_OK;
    RvThread*   pCurrent = NULL;*/
    
	return RV_OK;
/*    pCurrent = RvThreadCurrent();
    if (NULL == pCurrent)
    {
        return RV_OK;
    }
    crv = RvThreadDestruct(pCurrent);
    if (RV_OK != crv)
    {
        return CRV2RV(crv);
    }
    crv = RvMemoryFree(pCurrent,NULL);
    return CRV2RV(crv);*/
}

/***************************************************************************
 * RvSipMidEncodeB64 
 * ------------------------------------------------------------------------
 * General: Performs a 64 bit encoding operation of a given number of bytes 
 *          in a given buffer.
 * Return Value: The number of used bytes in the 'outTxt' buffer or -1 if 
 *          the function fails.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    inTxt - the buffer to be encoded.
 *           inLen - the length of buffer to be encoded
 *           outTxt - an encoding destination buffer
 *           outLen - the size of 'outTxt' buffer
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipMidEncodeB64 (IN RvUint8* inTxt,   
                                             IN RvInt    inLen,      
                                             INOUT RvUint8* outTxt, 
                                             IN RvInt outLen)
{
    return rvEncodeB64(inTxt,inLen, outTxt, outLen);
}

/***************************************************************************
 * RvSipMidDecodeB64 
 * ------------------------------------------------------------------------
 * General: Performs a 64 bit decoding.
 * Return Value: number of used bytes in the 'outTxt' or -1 if function fails
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    inTxt - the buffer to be decoded.
 *           inLen - the length of buffer to be decoded
 *           outTxt - this buffer is used as decoding destination
 *           outLen - size of 'outTxt' buffer
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipMidDecodeB64(IN RvUint8* inTxt,  
                                            IN RvInt inLen,     
                                            INOUT RvUint8* outTxt, 
                                            IN RvInt outLen)
{
    return rvDecodeB64(inTxt,inLen, outTxt, outLen);
}

/******************************************************************************
 * RvSipMidTlsSetLockingCallback
 * ----------------------------------------------------------------------------
 * General: The RADVISION SIP Stack uses OpenSSL library in order to provide
 *          the application with the TLS ability. OpenSSL library forces
 *          the modules run above it to manage locks. That means, the OpenSSL
 *          asks the modules to lock or unlock locks on behalf of the OpenSSL.
 *          This request is performed using special callback, which should be
 *          implemented in the modules, and which is called by the OpenSSL.
 *          If the callback is not set into the OpenSSL library, OpenSSL
 *          doesn't lock shared objects. This means, no multithread safety is
 *          provided.
 *              By default the RADVISION SIP Stack sets the callback into
 *          the OpenSSL library on construction and removes it on destruction.
 *          See RV_TLS_AUTO_SET_OS_CALLBACKS macro in rvusrconfig.h.
 *          It is defined as RV_YES by default.
 *              But if the Application has another modules in addition to
 *          the RADVISION SIP Stack that accesses the OpenSSL library,
 *          it should define the RV_TLS_AUTO_SET_OS_CALLBACKS macro as RV_NO.
 *          Now the Application can implements it's own callback, or it can use
 *          the RADVISION Stacks's implementation.
 *          Pay attention, the RADVISION implementation will not be available
 *          after the RADVISION Stack is destructed.
 *              To activate or to stop the RADVISION SIP Stack implementation
 *          of the locking callback, the RvSipMidTlsSetLockingCallback()
 *          function should be called. It just sets the callback into
 *          the OpenSSL library or remove it.
 * Return Value: RV_OK - on success
 *               RV_ERROR_NOTSUPPORTED   - if TLS is not supported
 *               RV_ERROR_ILLEGAL_ACTION - if callback is set automatically
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:    bSet - If RV_TRUE the callback will be set.
 *                  Otherwise the callback will be removed.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidTlsSetLockingCallback(RvBool bSet)
{
#if (RV_TLS_TYPE != RV_TLS_OPENSSL)
    RV_UNUSED_ARG(bSet);
    return RV_ERROR_NOTSUPPORTED;
#else
#if (RV_TLS_AUTO_SET_OS_CALLBACKS == RV_YES)
    RV_UNUSED_ARG(bSet);
    return RV_ERROR_ILLEGAL_ACTION;
#else
    RvTLSSetLockingCallback(bSet);
    return RV_OK;
#endif
#endif
}

/******************************************************************************
 * RvSipMidTlsSetThreadIdCallback
 * ----------------------------------------------------------------------------
 * General: The RADVISION SIP Stack uses OpenSSL library in order to provide
 *          the application with the TLS ability. To ensure multithread safety,
 *          every time the OpenSSL needs to know ID of the current thread,
 *          it asks modules run above for this ID.
 *          This request is performed using special callback, which should be
 *          implemented in the modules, and which is called by the OpenSSL.
 *          If the callback is not set into the OpenSSL library, OpenSSL
 *          doesn't know the thread ID. As a result, no multithread safety can
 *          be ensured.
 *              By default the RADVISION SIP Stack sets the callback into
 *          the OpenSSL library on construction and removes it on destruction.
 *          See RV_TLS_AUTO_SET_OS_CALLBACKS macro in rvusrconfig.h.
 *          It is defined as RV_YES by default.
 *              But if the Application has another modules in addition to
 *          the RADVISION SIP Stack that accesses the OpenSSL library,
 *          it should define the RV_TLS_AUTO_SET_OS_CALLBACKS macro as RV_NO.
 *          Now the Application can implements it's own callback, or it can use
 *          the RADVISION Stacks's implementation.
 *          Pay attention, the RADVISION implementation will not be available
 *          after the RADVISION Stack is destructed.
 *              To activate or to stop the RADVISION SIP Stack implementation
 *          of the Thread ID callback, the RvSipMidTlsSetThreadIdCallback()
 *          function should be called. It just sets the callback into
 *          the OpenSSL library or remove it.
 * Return Value: RV_OK - on success
 *               RV_ERROR_NOTSUPPORTED   - if TLS is not supported
 *               RV_ERROR_ILLEGAL_ACTION - if callback is set automatically
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:    bSet - If RV_TRUE the callback will be set.
 *                  Otherwise the callback will be removed.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidTlsSetThreadIdCallback(RvBool bSet)
{
#if (RV_TLS_TYPE != RV_TLS_OPENSSL)
    RV_UNUSED_ARG(bSet);
    return RV_ERROR_NOTSUPPORTED;
#else
#if (RV_TLS_AUTO_SET_OS_CALLBACKS == RV_YES)
    RV_UNUSED_ARG(bSet);
    return RV_ERROR_ILLEGAL_ACTION;
#else
    RvTLSSetThreadIdCallback(bSet);
    return RV_OK;
#endif
#endif
}

/******************************************************************************
 * RvSipMidAttachThread
 * ----------------------------------------------------------------------------
 * General: Indicate that a thread created by the application is about to use
 *          the SIP toolkit's API functions. 
 *          The function should be called within the attached thread.
 *			Without a call to this function, calls to API functions might cause a crash.
 *          On thread destruction, application must call RvSipMidDetachThread()
 *          in order to remove any memory allocated by the toolkit for this thread.
 *
 * Return Value: Returns RvStatus
 *               RV_ERROR_ILLEGAL_ACTION - if the thread was already attached.
 *               RV_OK  - if successful
 * ----------------------------------------------------------------------------
 * Arguments: 
 * Input:   hMidMgr    - handle to the middle layer manager.
 *			threadName - Name to give this thread in the logs
 * Output:  
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidAttachThread(IN RvSipMidMgrHandle    hMidMgr,
											   IN const RvChar         *threadName)
{
    RvStatus status;
    RvThread *curThread;    
    MidMgr*       pMidMgr = (MidMgr*)hMidMgr;

    if(hMidMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* See if this thread is already known to us */
    curThread = RvThreadCurrent();
    if (curThread == NULL)
    {
        /* New thread - let's allocate some space for it and make it a known one */
        status = RvMemoryAlloc(NULL, sizeof(RvThread), pMidMgr->pLogMgr, (void**)&curThread);
        if (status == RV_OK)
        {
            status = RvThreadConstructFromUserThread(pMidMgr->pLogMgr, curThread);
            if (status != RV_OK)
            {
                RvMemoryFree(curThread, pMidMgr->pLogMgr);
                return status;
            }
            else
            {
                RvThreadSetName(curThread, threadName);
            }
        }
    }
	else
	{
		/* the thread was already attached */
		status = RV_ERROR_ILLEGAL_ACTION;
	}

    return status;
}


/******************************************************************************
 * RvSipMidDetachThread
 * ----------------------------------------------------------------------------
 * General: Indicate that a thread created by the user is being shut down.
 *          This function should be called to remove any memory allocated by
 *          the toolkit for this thread (memory which was allocated when calling
 *          to RvSipMidAttachThread()).
 *          The function should be called within the thread to detach.
 *
 * Return Value: RV_OK  - if successful.
 *               Other on failure
 * ----------------------------------------------------------------------------
 * Arguments: 
 * Input:   hMidMgr    - handle to the middle layer manager.
 * Output:  
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidDetachThread(IN RvSipMidMgrHandle    hMidMgr)
{
    RvThread *curThread;
    MidMgr   *pMidMgr = (MidMgr*)hMidMgr;

    if(hMidMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    curThread = RvThreadCurrent();
    if (curThread == NULL)
    {
        /* Can't find a thread here... */
        return RV_ERROR_UNINITIALIZED;
    }

	RvThreadDestruct(curThread);
    RvMemoryFree(curThread, pMidMgr->pLogMgr);

    RV_UNUSED_ARG(pMidMgr)
    return RV_OK;
}

/************************************************************************************
 * RvSipMidGetResources
 * ----------------------------------------------------------------------------------
 * General: Gets the status of resources used by the Mid layer. 
 *
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hMidMgr    - handle to the middle layer manager.
 *          pResources - The resources in use by the Mid layer.
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidGetResources(
                                              IN    RvSipMidMgrHandle   hMidMgr,
                                              INOUT RvSipMidResources  *pResources)
{
	MidMgr      *pMidMgr = (MidMgr*)hMidMgr;
	RvUint32     allocNumOfLists;
    RvUint32     allocMaxNumOfUserElement;
    RvUint32     currNumOfUsedLists;
    RvUint32     currNumOfUsedUserElement;
    RvUint32     maxUsageInLists;
    RvUint32     maxUsageInUserElement;

    if(hMidMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pResources == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
	
    /* timers list */
    if (NULL == pMidMgr->hUserTimers)
    {
        pResources->userTimers.currNumOfUsedElements  = 0;
        pResources->userTimers.maxUsageOfElements     = 0;
        pResources->userTimers.numOfAllocatedElements = 0;
    }
    else
    {
        RLIST_GetResourcesStatus(pMidMgr->userTimerPool,
								 &allocNumOfLists, /*always 1*/
								 &allocMaxNumOfUserElement,
								 &currNumOfUsedLists,/*always 1*/
								 &currNumOfUsedUserElement,
								 &maxUsageInLists,
								 &maxUsageInUserElement);
        pResources->userTimers.currNumOfUsedElements  = currNumOfUsedUserElement;
        pResources->userTimers.maxUsageOfElements     = maxUsageInUserElement;
        pResources->userTimers.numOfAllocatedElements = allocMaxNumOfUserElement;
    }

    /* fd list */
    if (NULL == pMidMgr->hUserFds)
    {
        pResources->userFds.currNumOfUsedElements  = 0;
        pResources->userFds.maxUsageOfElements     = 0;
        pResources->userFds.numOfAllocatedElements = 0;
    }
    else
    {
        RLIST_GetResourcesStatus(pMidMgr->userFDPool,
								 &allocNumOfLists, /*always 1*/
								 &allocMaxNumOfUserElement,
								 &currNumOfUsedLists,/*always 1*/
								 &currNumOfUsedUserElement,
								 &maxUsageInLists,
								 &maxUsageInUserElement);
        pResources->userFds.currNumOfUsedElements  = currNumOfUsedUserElement;
        pResources->userFds.maxUsageOfElements     = maxUsageInUserElement;
        pResources->userFds.numOfAllocatedElements = allocMaxNumOfUserElement;
    }

    return RV_OK;
}

