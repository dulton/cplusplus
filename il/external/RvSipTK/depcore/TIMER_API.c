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
 *                              <TIMER_API.c>
 *
 *  In this file there are old core wrappers. these wrappers are deprecated and 
 *  are not recommended for use.
 *  The wrappers in this file are of the TIMER module
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                  April 2004
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_DEF.h"
#include "SELI_API.h"
#include "TIMER_API.h"
#include "depcoreInternals.h"
#include "rvmemory.h"
#include "rvselect.h"
#include "rvtimestamp.h"
#include "rvstdio.h"

#if defined(RV_DEPRECATED_CORE)

/*-----------------------------------------------------------------------*/
/*                        MACRO DEFINITIONS                              */
/*-----------------------------------------------------------------------*/
#define TIMER_LOCK_MGR   (NULL == g_pTimerMgr || NULL == g_pTimerMgr->plock  ) ? RV_OK : RvMutexLock(g_pTimerMgr->plock,g_pLogMgr)
#define TIMER_UNLOCK_MGR (NULL == g_pTimerMgr || NULL == g_pTimerMgr->plock  ) ? RV_OK : RvMutexUnlock(g_pTimerMgr->plock,g_pLogMgr)
/*-----------------------------------------------------------------------*/
/*                   STATIC AND GLOBAL PARAMS                            */
/*-----------------------------------------------------------------------*/
/* the log mgr enables the use to set a log mgr to the Wrapper before it was initiated
*/
extern RvLogMgr* g_pLogMgr;


/* the g_pSeliMgr hold data that is relevent to all instances that 
   make use of the select engine.*/
static WTimerMgr* g_pTimerMgr   = NULL;

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RV_Status RVCALLCONV WrapperConstructInstance(IN WTimerInstance*  pTIns);

static RvBool WrapperTimerFunc(void *);

static void WSetRes(IN RvInt change);

/*-----------------------------------------------------------------------*/
/*                            TIMER FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/********************************************************************************************
 * TIMER_Allocate
 * purpose : Allocate a TIMER module instance
 * input   : logHandle       - Handle to a LOG instance
 * output  : none
 * return  : A handle to the TIMER instance on success.
 *           NULL on failure
 ********************************************************************************************/
RVAPI RV_TIMER_PoolHandle RVCALLCONV TIMER_Allocate(IN RV_LOG_Handle logHandle)
{
    WTimerInstance* pTIns = NULL;

    g_pLogMgr = (RvLogMgr*)logHandle;
    /* Initiating the global timer moudule */
    if (NULL == g_pTimerMgr)
    {
        if (RV_OK != RvMemoryAlloc(NULL,sizeof(WTimerMgr),g_pLogMgr,(void**)&g_pTimerMgr))
            return NULL;
        memset(g_pTimerMgr,0,sizeof(WTimerMgr));

        if (RV_OK != RvMemoryAlloc(NULL,sizeof(RvMutex),g_pLogMgr,(void**)&g_pTimerMgr->plock))
        {
            RvMemoryFree((void*)g_pTimerMgr,g_pLogMgr);
            return NULL;
        }
        if (RV_OK != RvMutexConstruct(g_pLogMgr,g_pTimerMgr->plock))
        {
            RvMemoryFree((void*)g_pTimerMgr->plock,g_pLogMgr);
            RvMemoryFree((void*)g_pTimerMgr,g_pLogMgr);
            return NULL;
        }
        if (RV_OK != RvLogSourceConstruct(g_pLogMgr,&g_pTimerMgr->logSrc,"WTIMER","Timer Wrappers"))
        {
            RvMutexDestruct(g_pTimerMgr->plock,g_pLogMgr);
            RvMemoryFree((void*)g_pTimerMgr->plock,g_pLogMgr);
            RvMemoryFree((void*)g_pTimerMgr,g_pLogMgr);
            return NULL;
        }
    }
    g_pTimerMgr->initCalls++;

    /* allocating a spesific instance */
    if (RV_OK != RvMemoryAlloc(NULL,sizeof(WTimerInstance),g_pLogMgr,(void**)&pTIns))
    {
        TIMER_End(NULL);
        return NULL;
    }
    memset(pTIns,0,sizeof(WTimerInstance));
    if (RV_OK != RvSelectGetThreadEngine(g_pLogMgr,&pTIns->pSelect))
    {
        RvMemoryFree((void*)pTIns,g_pLogMgr);
        TIMER_End(NULL);
        return NULL;
    }
    return (RV_TIMER_PoolHandle)pTIns;    
}

/********************************************************************************************
 * TIMER_End
 * purpose : De-initialize a TIMER module instance
 * input   : hTimerPool - Handle to the pool of timers to free
 * output  : none
 * return  : RV_Success on success
 *           RV_Failure otherwise
 ********************************************************************************************/
RVAPI RvStatus RVCALLCONV TIMER_End(IN RV_TIMER_PoolHandle  hTimerPool)
{
    WTimerInstance* pTIns   = (WTimerInstance*)hTimerPool;
    WTimerCell*     pCell   = NULL;
    RvStatus        rv      = RV_OK;

    if (NULL != pTIns)
    {
        RvInt32 loc;
        /* killing all the timers */
        rv = RA_GetFirstUsedElement(pTIns->timerNodes,0,(RA_Element* )&pCell,&loc);
        while (RV_OK == rv)
        {
            TIMER_Release((RV_TIMER_Handle)pCell);
            rv = RA_GetFirstUsedElement(pTIns->timerNodes,loc,(RA_Element* )&pCell,&loc);
        }
        TIMER_LOCK_MGR;
        g_pTimerMgr->wtmgrres.totalAllocatedTimers -= pTIns->wtRes.maxNumOfElements;
        TIMER_UNLOCK_MGR;

        RvMemoryFree((void*)pTIns,g_pLogMgr);
    }
    g_pTimerMgr->initCalls--;
    if (0 == g_pTimerMgr->initCalls)
    {
        RvMutexDestruct(g_pTimerMgr->plock,g_pLogMgr);
        RvMemoryFree((void*)g_pTimerMgr->plock,g_pLogMgr);
        RvMemoryFree((void*)g_pTimerMgr,g_pLogMgr);        
    }
    return RV_OK;
}

/********************************************************************************************
 * TIMER_SetConfig
 * purpose : Set the TIMER module configuration parameters
 * input   : hTimerPool     - TIMER pool handle
 *           maxTimers      - Maximum amount of timers
 *           maxThreads     - Maximum number of threads which will have to callback on timers
 *                            expiration
 * output  : none
 * return  : RV_Success on success
 *           RV_Failure otherwise
 ********************************************************************************************/
RVAPI RvStatus RVCALLCONV TIMER_SetConfig(IN RV_TIMER_PoolHandle hTimerPool,
				                           IN RvUint32           maxTimers,
				                           IN RvUint32           maxThreads)
{
    WTimerInstance*  pTIns = (WTimerInstance*)hTimerPool;
    
    if ((g_pTimerMgr   == NULL) || (pTIns == NULL))
        return RV_ERROR_INVALID_HANDLE;

    if ((maxTimers == 0) || (maxThreads < 1))
    {
        RvLogError(&g_pTimerMgr->logSrc,(&g_pTimerMgr->logSrc,
                  "TIMER_SetConfig - pool=0x%x: Called with bad init params", pTIns));
        return RV_ERROR_BADPARAM;
    }

    pTIns->wtRes.maxNumOfElements = maxTimers;
    TIMER_LOCK_MGR;
    g_pTimerMgr->wtmgrres.totalAllocatedTimers += pTIns->wtRes.maxNumOfElements;
    TIMER_UNLOCK_MGR;
    return RV_Success;
}

/********************************************************************************************
 * TIMER_SetHandles
 * purpose : Set the handles to external modules used by TIMER
 * input   : hTimer         - Handle to TIMER instance
 *           seliHandle     - SELI object to use
 * output  : none
 * return  : RV_Success - In case the routine has succeded.
 *           RV_Failure - In case the routine has failed.
 ********************************************************************************************/
RVAPI RvStatus  RVCALLCONV TIMER_SetHandles(IN RV_TIMER_PoolHandle hTimer,
			                                IN RV_SELI_Handle      seliHandle)
{
    WTimerInstance*  pTIns = (WTimerInstance*)hTimer;
    
    if ((g_pTimerMgr   == NULL) || (pTIns == NULL))
        return RV_ERROR_INVALID_HANDLE;


    pTIns->pSelect = (RvSelectEngine*)seliHandle;

    return RV_Success;
}

/********************************************************************************************
 * TIMER_Init
 * purpose : Initialize a TIMER module instance
 * input   : hTimerPool - Handle to the pool of timers to initialize
 * output  : none
 * return  : RV_Success on success
 *           RV_OutOfResources on allocation failure
 *           RV_Failure otherwise
 ********************************************************************************************/
RVAPI RvStatus RVCALLCONV TIMER_Init(IN RV_TIMER_PoolHandle hTimerPool)
{
    WTimerInstance*  pTIns  = (WTimerInstance*)hTimerPool;
    RvStatus         rv     = RV_OK;
    
    if ((g_pTimerMgr   == NULL) || (pTIns == NULL))
        return RV_ERROR_INVALID_HANDLE;

    /* Make sure we can initialize */
    if (pTIns->wtRes.maxNumOfElements == 0)
    {
        RvLogError(&g_pTimerMgr->logSrc,(&g_pTimerMgr->logSrc,
                  "TIMER_Init - pool=0x%x: num of timers not set", pTIns));
        return RV_ERROR_BADPARAM;
    }

    if (pTIns->pSelect == NULL)
    {
        RvLogError(&g_pTimerMgr->logSrc,(&g_pTimerMgr->logSrc,
                  "TIMER_Init - pool=0x%x: select interface not set", pTIns));
        return RV_ERROR_UNKNOWN;
    }

    /* Make sure we've got the nodes for it */
    rv = WrapperConstructInstance(pTIns);
    if (rv != RV_OK) 
    {
        RvLogError(&g_pTimerMgr->logSrc,(&g_pTimerMgr->logSrc,
                  "TIMER_Init - pool=0x%x: failed to construct nodes", pTIns));
        return rv;
    }
    return rv;
}

/********************************************************************************************
 * TIMER_SetOnThread
 * purpose : 
 *           1. Allocate a new timer from that pool that its handle is given as input parameter
 *           2. Set the timer according to the given time
 *           3. Set the internal data structure to keep the callback routine, and the paramreters
 *           4. call core to set the timer
 * input   : hTimerPool     - Handle to the pool of timers
 *           eventHandler   - Callback routine to call when the timer's time elapses
 *           context        - Context to return in callback function
 *           timeout        - The time measured by the timer, given in milliseconds
 *           threadId       - The ID of the thread to pop the timer event
 * output  : none
 * return  : Handle to the allocated timer
 *           NULL on failure (no more timer resources available)
 ********************************************************************************************/
RVAPI RV_TIMER_Handle RVCALLCONV TIMER_SetOnThread(IN RV_TIMER_PoolHandle       hTimerPool,
                                             IN RV_TIMER_EVENTHANDLER           eventHandler,
                                             IN void*                           context,
                                             IN RvUint32                        timeOut,
                                             IN RV_THREAD_Handle                hThreadId)
{
    RvStatus        rv      = RV_OK;
    WTimerInstance* pTIns   = (WTimerInstance*)hTimerPool;
    WTimerCell*     pCell   = NULL;
    RvSelectEngine* pSelect = (RvSelectEngine*)hThreadId;
    
    if ((g_pTimerMgr   == NULL) || (pTIns == NULL) || (pSelect == NULL) || (eventHandler == NULL))
        return NULL;

    /*1. Allocate a timer cell from RA */
    if (RV_OK != RA_Alloc(pTIns->timerNodes,(RA_Element*)&pCell))
    {
        return NULL;
    }
    
    /*2. Set timer params */
    memset(pCell,0,sizeof(WTimerCell));
    pCell->bIsOn    = RV_FALSE;
    pCell->ctx      = context;
    pCell->handler  = eventHandler;
    pCell->pWt      = pTIns;
    /*3. set the timer on proper select engine*/
    if (RV_OK != RvTimerStart(&pCell->timer, &pSelect->tqueue, RV_TIMER_TYPE_ONESHOT,
                       timeOut*RV_TIME64_NSECPERMSEC, WrapperTimerFunc, (void*)pCell))
    {
        RA_DeAlloc(pTIns->timerNodes,(RA_Element)pCell);
        return NULL;
    }

    /*4. give timer to application */
    if (rv == RV_OK)
    {
        pCell->bIsOn = RV_TRUE;
        WSetRes(1);
        return (RV_TIMER_Handle)pCell;
    }

    /* If we got here, somthing went wrong, dealloc the timer */
    RA_DeAlloc(pTIns->timerNodes,(RA_Element)pCell);
    return NULL;
}

/********************************************************************************************
 * TIMER_Set
 * purpose : sets a timer that will expire on the current thread
 * input   : hTimerPool     - Handle to the pool of timers
 *           eventHandler   - Callback routine to call when the timer's time elapses
 *           context        - Context to return in callback function
 *           timeout        - The time measured by the timer, given in milliseconds
 * output  : none
 * return  : Handle to the allocated timer
 *           NULL on failure (no more timer resources available)
 ********************************************************************************************/
RVAPI RV_TIMER_Handle RVCALLCONV TIMER_Set(IN RV_TIMER_PoolHandle     hTimerPool,
                                         IN RV_TIMER_EVENTHANDLER   eventHandler,
                                         IN void*                   context,
                                         IN RvUint32               timeOut)
{
    RvSelectEngine* pSelect = NULL;

    if (g_pTimerMgr  == NULL)
        return NULL;

    if (RV_OK != RvSelectGetThreadEngine(g_pLogMgr,&pSelect))
    {
        return NULL;
    }
    return TIMER_SetOnThread(hTimerPool,eventHandler,context,timeOut,(RV_THREAD_Handle)pSelect);
}

/********************************************************************************************
 * TIMER_Release
 * purpose : Releases a timer. Can be called inside the callback routine or before.
 * input   : timerHandle     : The handle to the timer
 * output  : none
 * return  : RV_Success on success
 *           Other on failure
 ********************************************************************************************/
RVAPI RvStatus RVCALLCONV TIMER_Release(IN RV_TIMER_Handle   timerHandle)
{
    WTimerInstance* pTIns   = NULL;
    WTimerCell*     pCell   = (WTimerCell*)timerHandle;
    
    if (g_pTimerMgr   == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if (NULL == pCell)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pTIns = pCell->pWt;
    if (RV_TRUE == pCell->bIsOn)
    {
        RvTimerCancel(&pCell->timer,RV_TIMER_CANCEL_DONT_WAIT_FOR_CB);
        pCell->bIsOn = RV_FALSE;
        WSetRes(-1);
    }
    RA_DeAlloc(pTIns->timerNodes,(RA_Element)pCell);
    return RV_OK;
}

/********************************************************************************************
 * TIMER_RemainingTime
 * purpose : Returns the time that is left until the timer will call its event handler.
 *           This function can be called only in the thread that initiated the timer
 * input   : N/A
 * output  : msec - number of seconds left until the timer will call its event handler
 * return  : RV_Success or RV_Failure
 ********************************************************************************************/
RVAPI RvStatus RVCALLCONV TIMER_RemainingTime(OUT RvUint32 *msec)
{
    RvSelectEngine* pSelect = NULL;
    RvInt64         nextevent = 0;

    *msec = (RvUint32)-1;
    
    if (g_pTimerMgr   == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if (RV_OK != RvSelectGetThreadEngine(g_pLogMgr,&pSelect))
    {
        return RV_ERROR_UNKNOWN;
    }
    
    if (RV_OK != RvTimerQueueNextEvent(&pSelect->tqueue,&nextevent))
    {
        return RV_ERROR_UNKNOWN;
    }
    *msec = RvInt64ToRvUint32(RvInt64Div(nextevent,RV_TIME64_NSECPERMSEC));
    return RV_OK;
}

/********************************************************************************************
 * TIMER_NodeRemainingTime
 * purpose : Returns the time that is left until the timer will call its event handler
 * input   : timerHandle     : The handle to the timer
 * output  : none
 * return  : The time that is left
 ********************************************************************************************/
RVAPI RvUint32 RVCALLCONV TIMER_NodeRemainingTime(IN RV_TIMER_Handle    timerHandle)
{
    WTimerInstance* pTIns   = NULL;
    WTimerCell*     pCell   = (WTimerCell*)timerHandle;
    RvInt64         poptime = 0;
    
    if (g_pTimerMgr   == NULL)
        return (RvUint32)-1;

    if (NULL == pCell || pCell->bIsOn == RV_FALSE)
    {
        return (RvUint32)-1;
    }
    pTIns = pCell->pWt;
    if (RV_OK != RvTimerGetRemainingTime(&pCell->timer,&poptime))
    {
        return (RvUint32)-1;
    }
    return RvInt64ToRvUint32(RvInt64Div(poptime,RV_TIME64_NSECPERMSEC));
}

/********************************************************************************************
 * TIMER_Handle
 * purpose : Execute any event handlers whose time has already passed in the current
 *           running thread for all the allocated timer pools.
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
RVAPI void RVCALLCONV TIMER_Handle(void)
{
    RvSelectEngine* pSelect = NULL;
    if (g_pTimerMgr   == NULL)
        return;

    if (RV_OK != RvSelectGetThreadEngine(g_pLogMgr,&pSelect))
    {
        return;
    }
    TIMER_HandleEvent((RV_THREAD_Handle)pSelect);
}

/********************************************************************************************
 * TIMER_HandleEvent
 * purpose : Execute any event handlers whose time has already passed in the current
 *           running thread for all the allocated timer pools.
 * input   : threadId - The thread id whos events are going to be process.
 * output  : none
 * return  : none
 ********************************************************************************************/
RVAPI void RVCALLCONV TIMER_HandleEvent(IN RV_THREAD_Handle        threadId)
{
    RvSelectEngine* pSelect = (RvSelectEngine*)threadId;
    RvSize_t dontcare = 0;
    if (NULL == pSelect)
        return;
    RvTimerQueueService(&pSelect->tqueue,0,&dontcare,NULL,NULL);
}

/********************************************************************************************
 * TIMER_GetTimeInMilliseconds
 * purpose : Get the current time in milliseconds since platform startup
 * input   : none
 * output  : none
 * return  : The current time in milliseconds
 ********************************************************************************************/
RVAPI RvUint32 RVCALLCONV TIMER_GetTimeInMilliseconds(void)
{
    return RvInt64ToRvUint32((RvInt64Div(RvTimestampGet(NULL),RV_TIME64_NSECPERMSEC)));
}

/********************************************************************************************
 * TIMER_GetTimeInSeconds
 * purpose : Get the current time in seconds since 1970
 * input   : none
 * output  : none
 * return  : The current time in seconds
 ********************************************************************************************/
RVAPI RV_UINT32 RVCALLCONV TIMER_GetTimeInSeconds(void)
{
    return (RvUint32)(RvInt64Div(RvTimestampGet(NULL),RV_TIME64_NSECPERSEC));    
}

/********************************************************************************************
 * TIMER_GetResourcesStatus
 * purpose : Get the resources status of the TIMER module instance
 *           This function is used for debugging purposes.
 *           Note the "timers" field of the "resources" structure is not in use
 *           and should be ignored by the appliation.
 * input   : hTimerPool - Handle to the pool of timers
 * output  : resources  - Resources status
 * return  : RV_Success          - Success
 *           RV_InvalidHandle    - Timer handle is invalid
 ********************************************************************************************/
RVAPI RV_Status RVCALLCONV TIMER_GetResourcesStatus(IN  RV_TIMER_PoolHandle   hTimerPool,
			                                        OUT RV_TIMER_Resource*    resources)
{
    WTimerInstance*  pTIns  = (WTimerInstance*)hTimerPool;
    
    if ((g_pTimerMgr   == NULL) || (pTIns == NULL) || (resources == NULL))
        return RV_ERROR_INVALID_HANDLE;
    TIMER_LOCK_MGR;
    memcpy(resources,&g_pTimerMgr->wtmgrres,sizeof(g_pTimerMgr->wtmgrres));
    TIMER_UNLOCK_MGR;
    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                            STATIC FUNCTIONS                           */
/*-----------------------------------------------------------------------*/

/********************************************************************************************
 * WrapperConstructInstance
 * purpose : Construct the nodes and binary heaps on initializations
 * input   : tApp   - Application information
 * output  : none
 * return  : RV_Success on success
 *           RV_OutOfResources on failure
 ********************************************************************************************/
static RV_Status RVCALLCONV WrapperConstructInstance(IN WTimerInstance*  pTIns)
{
    RvChar    appName[30];

    sprintf(appName, "WTIMER_INST_0x%p", pTIns);

    if (RV_OK != RvMemoryAlloc(NULL,sizeof(RvMutex),g_pLogMgr,(void**)&pTIns->plock))
        return RV_ERROR_UNKNOWN ;
    
    /* Set the locks first */
    RvMutexConstruct(g_pLogMgr,pTIns->plock);

    /* Create file descriptor nodes */
    pTIns->timerNodes = RA_Construct(sizeof(WTimerCell), pTIns->wtRes.maxNumOfElements,
                               NULL, g_pLogMgr, appName);
    if (pTIns->timerNodes == NULL)
    {
        RvLogError(&g_pTimerMgr->logSrc,(&g_pTimerMgr->logSrc,
                  "WrapperConstructInstance - pool=0x%x: faild to construct timer nodes", pTIns));
        RvMutexDestruct(pTIns->plock,g_pLogMgr);
        RvMemoryFree((void*)pTIns->plock,g_pLogMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }
    return RV_OK;
}

/********************************************************************************************
 * WrapperTimerFunc
 * purpose : Construct the nodes and binary heaps on initializations
 * input   : tApp   - Application information
 * output  : none
 * return  : RV_Success on success
 *           RV_OutOfResources on failure
 ********************************************************************************************/
static RvBool WrapperTimerFunc(IN void *ctx)
{
    WTimerCell*     pCell = ctx;
    pCell->bIsOn = RV_FALSE;
    pCell->handler((RV_TIMER_Handle)pCell,pCell->ctx);
    WSetRes(-1);
    return RV_TRUE;
}

/********************************************************************************************
 * WSetRes
 * purpose : Construct the nodes and binary heaps on initializations
 * input   : tApp   - Application information
 * output  : none
 * return  : RV_Success on success
 *           RV_OutOfResources on failure
 ********************************************************************************************/
static void WSetRes(IN RvInt change)
{
    TIMER_LOCK_MGR;
    g_pTimerMgr->wtmgrres.totalUsedTimers += change;
    TIMER_UNLOCK_MGR;
}

#endif /*defined(RV_DEPRECATED_CORE)*/

    



