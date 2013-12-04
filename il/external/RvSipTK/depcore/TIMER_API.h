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
 *                              <TIMER_API.h>
 *
 *  In this file there are old core wrappers. these wrappers are deprecated and 
 *  are not recommended for use.
 *  The wrappers in this file are of the TIMER module
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                  April 2004
 *********************************************************************************/
#ifndef _TIMER_API
#define _TIMER_API

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_DEF.h"
#include "RV_ADS_DEF.h"
    
#if defined(RV_DEPRECATED_CORE)
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
/* RV_TIMER_Handle - Definition of a handle to a specific timer*/
RV_DECLARE_HANDLE(RV_TIMER_Handle);

/********************************************************************************************
 * RV_TIMER_EVENTHANDLER
 * Definition of the Callback routine provides for each timer
 * timerHandle    - Handle to the timer object
 * context        - The context user supplied on TIMER_Set()
 ********************************************************************************************/
typedef void(RVCALLCONV *RV_TIMER_EVENTHANDLER)(IN RV_TIMER_Handle  timerHandle,
                                                IN void*            context);

/********************************************************************************************
 * Definition of RV_TIMER_Resource:
 * timers                   - Timer resources status
 * totalAllocatedTimers     - Total number of allocated timers on the system
 * totalUsedTimers          - Total number of set timers on the system
 ********************************************************************************************/
typedef struct
{
    RV_Resource timers;
    RvUint32   totalAllocatedTimers;
    RvUint32   totalUsedTimers;
} RV_TIMER_Resource;

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
RVAPI RV_TIMER_PoolHandle RVCALLCONV TIMER_Allocate(IN RV_LOG_Handle logHandle);

/********************************************************************************************
 * TIMER_End
 * purpose : De-initialize a TIMER module instance
 * input   : hTimerPool - Handle to the pool of timers to free
 * output  : none
 * return  : RV_Success on success
 *           RV_Failure otherwise
 ********************************************************************************************/
RVAPI RV_Status RVCALLCONV TIMER_End(IN RV_TIMER_PoolHandle  hTimerPool);

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
RVAPI RV_Status RVCALLCONV TIMER_SetConfig(IN RV_TIMER_PoolHandle hTimerPool,
				                           IN RvUint32           maxTimers,
				                           IN RvUint32           maxThreads);

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
			                                IN RV_SELI_Handle      seliHandle);

/********************************************************************************************
 * TIMER_Init
 * purpose : Initialize a TIMER module instance
 * input   : hTimerPool - Handle to the pool of timers to initialize
 * output  : none
 * return  : RV_Success on success
 *           RV_OutOfResources on allocation failure
 *           RV_Failure otherwise
 ********************************************************************************************/
RVAPI RvStatus RVCALLCONV TIMER_Init(IN RV_TIMER_PoolHandle hTimerPool);

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
                                             IN RV_THREAD_Handle                hThreadId);

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
                                         IN RvUint32                timeOut);

/********************************************************************************************
 * TIMER_Release
 * purpose : Releases a timer. Can be called inside the callback routine or before.
 * input   : timerHandle     : The handle to the timer
 * output  : none
 * return  : RV_Success on success
 *           Other on failure
 ********************************************************************************************/
RVAPI RvStatus RVCALLCONV TIMER_Release(IN RV_TIMER_Handle   timerHandle);

/********************************************************************************************
 * TIMER_RemainingTime
 * purpose : Returns the time that is left until the timer will call its event handler.
 *           This function can be called only in the thread that initiated the timer
 * input   : N/A
 * output  : msec - number of seconds left until the timer will call its event handler
 * return  : RV_Success or RV_Failure
 ********************************************************************************************/
RVAPI RvStatus RVCALLCONV TIMER_RemainingTime(OUT RvUint32 *msec);

/********************************************************************************************
 * TIMER_NodeRemainingTime
 * purpose : Returns the time that is left until the timer will call its event handler
 * input   : timerHandle     : The handle to the timer
 * output  : none
 * return  : The time that is left
 ********************************************************************************************/
RVAPI RvUint32 RVCALLCONV TIMER_NodeRemainingTime(IN RV_TIMER_Handle    timerHandle);

/********************************************************************************************
 * TIMER_Handle
 * purpose : Execute any event handlers whose time has already passed in the current
 *           running thread for all the allocated timer pools.
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
RVAPI void RVCALLCONV TIMER_Handle(void);

/********************************************************************************************
 * TIMER_HandleEvent
 * purpose : Execute any event handlers whose time has already passed in the current
 *           running thread for all the allocated timer pools.
 * input   : threadId - The thread id whos events are going to be process.
 * output  : none
 * return  : none
 ********************************************************************************************/
RVAPI void RVCALLCONV TIMER_HandleEvent(IN RV_THREAD_Handle        threadId);


/********************************************************************************************
 * TIMER_GetTimeInMilliseconds
 * purpose : Get the current time in milliseconds since platform startup
 * input   : none
 * output  : none
 * return  : The current time in milliseconds
 ********************************************************************************************/
RVAPI RvUint32 RVCALLCONV TIMER_GetTimeInMilliseconds(void);

/********************************************************************************************
 * TIMER_GetTimeInSeconds
 * purpose : Get the current time in seconds since 1970
 * input   : none
 * output  : none
 * return  : The current time in seconds
 ********************************************************************************************/
RVAPI RV_UINT32 RVCALLCONV TIMER_GetTimeInSeconds(void);

/********************************************************************************************
 * TIMER_GetResourcesStatus
 * purpose : Get the resources status of the TIMER module instance
 *           This function is used for debugging purposes
 *           Note the "timers" field of the "resources" structure is not in use
 *           and should be ignored by the appliation.
 * input   : hTimerPool - Handle to the pool of timers
 * output  : resources  - Resources status
 * return  : RV_Success          - Success
 *           RV_InvalidHandle    - Timer handle is invalid
 ********************************************************************************************/
RVAPI RV_Status RVCALLCONV TIMER_GetResourcesStatus(IN  RV_TIMER_PoolHandle   hTimerPool,
			                                        OUT RV_TIMER_Resource*    resources);


#endif /*#if defined(RV_DEPRECATED_CORE) */
#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef _TIMER_API */


