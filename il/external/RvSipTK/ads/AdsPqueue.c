#ifdef __cplusplus
extern "C" {
#endif


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
* Copyright RADVision 1996.                                                     *
* Last Revision:																*
**********************************************************************************/


/*********************************************************************************
 *                              PQUEUE_API.c
 *
 * The file contains support for processing queue (PQUEUE). Processing queue is used
 * for sending stack internal events between main & processing stack threads in case
 * of multi-threaded stack configuration or for future processing by the same thread
 * in case of single threaded stack configuration.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *  Boris Perlov                 September 2002
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_ADS_DEF.h"
#include "AdsPqueue.h"
#include "rvsemaphore.h"
#include "AdsRpool.h"
#include "AdsRlist.h"
#include <string.h>
/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/
/* PQUEUE
 * ---------------------------------------------------------------------------
 * the PQUEUE defines processing queue object.
 *
 * logQueueId		-- LOG registration ID.
 * hLock            -- A lock used to lock the queue object to avoid threads
 *                     collisions.
 * evSize			-- size of single event
 * hEventsPool      -- pool of 'free' processing queue events.
 * hEventQueuePool  -- pool of queues
 * hEventQueuePool  -- the event queue.
 * noEnoughEvents   -- Mark that specifies if event allocation failure occured
 *					   due to no enough resources
 * startShutdown    -- used to shutdown processing threads
 * newMsgSync       -- Semaphore, used to notify waiting for event
 *					   threads that there is new event in the queue
 * hLi				-- LI handler, used for resource available notifications
 * hMainThread		-- main thread handler, used for sending notifications
 * isMultiThreaded	-- specifies if processing queue is used in multi or single
 *					   threaded environment.
 * name             -- The name of the QUEUE instance.
 * pSelect          -- pointer to queue select. this is the place to post messages  to
 * tryAgainCellAvailable -- if RV_TRUE, will try to post CELL_AVAILABLE message
 *                     on each call to PQUEUE_AllocateEvent & PQUEUE_FreeEvent. 
 */
typedef struct {
	RvLogSource        *pLogSource;
	RvMutex             *hLock;
	RvUint32			evSize;
	HRA					hEventsPool;
    RLIST_POOL_HANDLE   hEventQueuePool;
	RLIST_HANDLE        hEventQueue;

	RvBool				noEnoughEvents;
	RvBool				startShutdown;
	RvSemaphore			*newMsgSync;
	RvBool				isMultiThreaded;
    RvChar             name[32];

    /* cc*/
    RvSelectEngine     *pSelect;
    RvLogMgr           *pLogMgr;
    RvUint8            preemptionDispatchEvents;
    RvUint8            preemptionReadBuffersReady;
    RvUint8            preemptionCellAvailable;
    RvBool             tryAgainCellAvailable;
} PQUEUE;




/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS					             */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/************************************************************************************
 * PQUEUE_Construct
 * ----------------------------------------------------------------------------------
 * General: Construct processing queue.
 * Return Value: RvStatus -	RV_ERROR_OUTOFRESOURCES
 *								RV_OK
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   queueSize	- maximum length of the queue
 *          pLogMgr     - pointer to a log manager.
 *			hLock		- queue lock
 *			evSize - maximum event length
 *			hLi			- LI handler
 *			isMultiThreaded - boolean that specifies multi threaded configuration
 *          pSelect     - pointer to select engine
 *          name        - Name of the QUEUE instance (used for log messages)
 * Output:  hQueue		- queue handler
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV PQUEUE_Construct(IN  RvUint32				 queueSize,
										   IN  RvLogMgr			    *pLogMgr,
										   IN  RvMutex              *hLock,
										   IN  RvUint32				 evSize,
										   IN  RvBool				 isMultiThreaded,
                                           IN  RvSelectEngine       *pSelect,
                                           IN  const char           *name,
										   OUT RV_PROCESSING_QUEUE	*hQueue)
{
	RvStatus			status = RV_OK;
	RvStatus            crv;
    PQUEUE	*processing_queue;

    crv = RV_OK;
	
    if (RV_OK != RvMemoryAlloc(NULL, sizeof (PQUEUE), pLogMgr, (void*)&processing_queue))
	{
        return RV_ERROR_OUTOFRESOURCES;
	}

	processing_queue->hEventQueuePool = RLIST_PoolListConstruct(queueSize,1,
		                                sizeof(RA_Element),pLogMgr, name);

	if(processing_queue->hEventQueuePool == NULL)
	{
		RvMemoryFree(processing_queue,pLogMgr);
		return RV_ERROR_OUTOFRESOURCES;
	}

	/*allocate a list from the pool*/
	processing_queue->hEventQueue = RLIST_ListConstruct(processing_queue->hEventQueuePool);
	if(processing_queue->hEventQueue == NULL)
	{
		RvMemoryFree(processing_queue,pLogMgr);
		return RV_ERROR_OUTOFRESOURCES;
	}

	processing_queue->evSize = evSize;

    processing_queue->hEventsPool = RA_Construct(evSize,queueSize,
                                                 NULL,pLogMgr,name);
	if (processing_queue->hEventsPool == NULL)
	{
        RLIST_PoolListDestruct(processing_queue->hEventQueuePool);
        RvMemoryFree(processing_queue,pLogMgr);
        return RV_ERROR_OUTOFRESOURCES;
	}

	processing_queue->newMsgSync = NULL;
	if (isMultiThreaded)
	{
		crv = RvMemoryAlloc(NULL, sizeof (RvSemaphore), pLogMgr,
			                (void*)&processing_queue->newMsgSync);

		if (crv != RV_OK)
		{
            RLIST_PoolListDestruct(processing_queue->hEventQueuePool);
            RA_Destruct(processing_queue->hEventsPool);
			RvMemoryFree(processing_queue,pLogMgr);
			return RV_ERROR_OUTOFRESOURCES;
		}
		status = RvSemaphoreConstruct(0,pLogMgr,processing_queue->newMsgSync);
		if (status != RV_OK)
		{
            RLIST_PoolListDestruct(processing_queue->hEventQueuePool);
            RA_Destruct(processing_queue->hEventsPool);
			RvMemoryFree(processing_queue->newMsgSync,pLogMgr);
			RvMemoryFree(processing_queue,pLogMgr);
			return status;
		}
	}
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    if (pLogMgr != NULL)
    {
		crv = RvMemoryAlloc(NULL,sizeof(RvLogSource),pLogMgr,(void*)&processing_queue->pLogSource);
        if(crv != RV_OK)
		{
            RLIST_PoolListDestruct(processing_queue->hEventQueuePool);
            RA_Destruct(processing_queue->hEventsPool);
			RvSemaphoreDestruct(processing_queue->newMsgSync,pLogMgr);
			RvMemoryFree(processing_queue->newMsgSync,pLogMgr);
			RvMemoryFree(processing_queue,pLogMgr);
			return RV_ERROR_OUTOFRESOURCES;
		}
		crv = RvLogSourceConstruct(pLogMgr,processing_queue->pLogSource,ADS_PQUEUE, "Processing Queue");
		if(crv != RV_OK)
		{
            RLIST_PoolListDestruct(processing_queue->hEventQueuePool);
            RA_Destruct(processing_queue->hEventsPool);
			RvSemaphoreDestruct(processing_queue->newMsgSync,pLogMgr);
			RvMemoryFree(processing_queue->newMsgSync,pLogMgr);
		    RvMemoryFree(processing_queue->pLogSource,pLogMgr);
			RvMemoryFree(processing_queue,pLogMgr);
			return RV_ERROR_OUTOFRESOURCES;
		}
        RvLogInfo(processing_queue->pLogSource, (processing_queue->pLogSource,
            "PQUEUE_Construct - (queueSize=%d,pLogMgr=0x%p,hLock=0x%p,evSize=%u,isMultiThreaded=%d,pSelect=0x%p,name=%s,hQueue=0x%p)",
            queueSize,pLogMgr,hLock,evSize,isMultiThreaded,pSelect,NAMEWRAP(name),hQueue));
    }
    else
    {
        processing_queue->pLogSource = NULL;
    }
#else
    processing_queue->pLogSource = NULL;
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
	processing_queue->hLock = hLock;
    if (NULL != name)
    {
        memcpy((RvChar *)processing_queue->name, name, sizeof(processing_queue->name) - 1);
    }
    else
    {
        processing_queue->name[0] = '\0';
    }
	processing_queue->startShutdown         = RV_FALSE;
	processing_queue->noEnoughEvents        = RV_FALSE;
    processing_queue->pSelect               = pSelect;
	processing_queue->isMultiThreaded       = isMultiThreaded;
    processing_queue->pLogMgr               = pLogMgr;
    processing_queue->tryAgainCellAvailable = RV_FALSE;

	*hQueue = (RV_PROCESSING_QUEUE)processing_queue;

    RvLogLeave(processing_queue->pLogSource, (processing_queue->pLogSource,
        "PQUEUE_Construct - (queueSize=%d,pLogMgr=0x%p,hLock=0x%p,evSize=%u,isMultiThreaded=%d,pSelect=0x%p,name=%s,*hQueue=0x%p)=%d",
        queueSize,pLogMgr,hLock,evSize,isMultiThreaded,pSelect,NAMEWRAP(name),*hQueue,RV_OK));
    
	return RV_OK;
}

/************************************************************************************
 * PQUEUE_SetPreemptionLocation
 * ----------------------------------------------------------------------------------
 * General: sets preemption location for the processing queue.
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hQueue		- queue handler
 *          preemptionDispatchEvents - the message type for dispatch events message
 *          preemptionReadBuffersReady - the message type for buffers ready message
 *          preemptionCellAvailable - the message type for cell avaliable message
 * Output:  none
 ***********************************************************************************/
RVAPI void RVCALLCONV PQUEUE_SetPreemptionLocation(
    IN RV_PROCESSING_QUEUE  hQueue,
    IN RvUint8              preemptionDispatchEvents,
    IN RvUint8              preemptionReadBuffersReady,
    IN RvUint8              preemptionCellAvailable)
{
	PQUEUE	*processing_queue = (PQUEUE *)hQueue;
    processing_queue->preemptionDispatchEvents = preemptionDispatchEvents;
    processing_queue->preemptionReadBuffersReady = preemptionReadBuffersReady;
    processing_queue->preemptionCellAvailable = preemptionCellAvailable;
}

/************************************************************************************
 * PQUEUE_Destruct
 * ----------------------------------------------------------------------------------
 * General: Destruct processing queue.
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hQueue		- queue handler
 * Output:  none
 ***********************************************************************************/
RVAPI void RVCALLCONV PQUEUE_Destruct(IN RV_PROCESSING_QUEUE	hQueue)
{
	PQUEUE	*processing_queue;
	RvLogMgr            *pLogMgr = NULL;
	processing_queue = (PQUEUE *)hQueue;
	if(processing_queue == NULL)
    {
        return;
    }
	RvMutexLock(processing_queue->hLock,processing_queue->pLogMgr);
	pLogMgr = processing_queue->pLogMgr;

    RvLogInfo(processing_queue->pLogSource, (processing_queue->pLogSource,
        "PQUEUE_Destruct - (hQueue=0x%p(%s))",processing_queue,processing_queue->name));

	if (processing_queue->hEventQueuePool != NULL)
    {
        RLIST_PoolListDestruct(processing_queue->hEventQueuePool);
    }
    if(processing_queue->hEventsPool != NULL)
    {
        RA_Destruct(processing_queue->hEventsPool);
    }

	if (processing_queue->newMsgSync != NULL)
	{
		RvSemaphoreDestruct(processing_queue->newMsgSync,pLogMgr);
		RvMemoryFree(processing_queue->newMsgSync,pLogMgr);
	}

	if(processing_queue->pLogSource != NULL)
	{
		RvLogSourceDestruct(processing_queue->pLogSource);
		RvMemoryFree(processing_queue->pLogSource,pLogMgr);
	}
	RvMutexUnlock(processing_queue->hLock,pLogMgr);
	RvMemoryFree(processing_queue,pLogMgr);
}


/************************************************************************************
 * PQUEUE_KillThread
 * ----------------------------------------------------------------------------------
 * General: Send 'kill' command to listening threads
 * Return Value: RvStatus -	RV_ERROR_OUTOFRESOURCES
 *								RV_OK
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hQueue		- queue handler
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV PQUEUE_KillThread(IN RV_PROCESSING_QUEUE		hQueue)
{
	PQUEUE	*processing_queue;
	RvStatus			retCode = RV_OK;

	processing_queue = (PQUEUE *)hQueue;
	if(processing_queue == NULL)
	{
		return RV_ERROR_UNKNOWN;
	}

    RvLogInfo(processing_queue->pLogSource, (processing_queue->pLogSource,
        "PQUEUE_KillThread - (hQueue=0x%p(%s))",processing_queue,processing_queue->name));


	processing_queue->startShutdown = RV_TRUE; /* there is no reason to protect this variable
												with lock/semaphore */
	if (processing_queue->isMultiThreaded)
    {
		retCode = RvSemaphorePost(processing_queue->newMsgSync,processing_queue->pLogMgr);
    }
    if (RV_OK == retCode)
    {
        RvLogLeave(processing_queue->pLogSource, (processing_queue->pLogSource,
            "PQUEUE_KillThread - (hQueue=0x%p(%s))=%d",
            processing_queue,processing_queue->name,retCode));
    }
    else
    {
        RvLogError(processing_queue->pLogSource, (processing_queue->pLogSource,
            "PQUEUE_KillThread - (hQueue=0x%p(%s))=%d",
            processing_queue,processing_queue->name,RvErrorGetCode(retCode)));
    }

	return retCode;
}


/************************************************************************************
 * PQUEUE_TailEvent
 * ----------------------------------------------------------------------------------
 * General: Adds event to the end of the queue.
 * Return Value: RvStatus -	RV_ERROR_OUTOFRESOURCES
 *								RV_OK
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hQueue		- queue handler
 *			ev			- event to be sent
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV PQUEUE_TailEvent(IN RV_PROCESSING_QUEUE		hQueue,
											IN HQEVENT					ev)
{
	RvStatus			retCode = RV_OK;
	PQUEUE	*processing_queue;
    RLIST_ITEM_HANDLE   listItem;
    HQEVENT				*pEv;
	processing_queue = (PQUEUE *)hQueue;

	if ((ev == NULL) || (processing_queue == NULL))
	{
		return RV_ERROR_UNKNOWN;
	}

	RvMutexLock(processing_queue->hLock,processing_queue->pLogMgr);
    RvLogEnter(processing_queue->pLogSource, (processing_queue->pLogSource,
        "PQUEUE_TailEvent - (hQueue=0x%p(%s) ev=0x%p)",
        processing_queue,processing_queue->name,ev));

	retCode = RLIST_InsertTail(processing_queue->hEventQueuePool,processing_queue->hEventQueue,&listItem);
    if(retCode != RV_OK)
    {
    	RvMutexUnlock(processing_queue->hLock,processing_queue->pLogMgr);
        RvLogError(processing_queue->pLogSource, (processing_queue->pLogSource,
            "PQUEUE_TailEvent - (hQueue=0x%p(%s) ev=0x%p) failed to tail event",
            processing_queue,processing_queue->name,ev));
		return RV_ERROR_OUTOFRESOURCES;
    }
    pEv = (HQEVENT*)listItem;
    *pEv = ev;

	RvMutexUnlock(processing_queue->hLock,processing_queue->pLogMgr);


    RvLogInfo(processing_queue->pLogSource, (processing_queue->pLogSource,
        "PQUEUE_TailEvent - (hQueue=0x%p(%s) ev=0x%p) notifying threads",
        processing_queue,processing_queue->name,ev));
    
	if (processing_queue->isMultiThreaded)
    {
		retCode = RvSemaphorePost(processing_queue->newMsgSync,processing_queue->pLogMgr);
    }
	else
    {
        RvSelectStopWaiting(processing_queue->pSelect,
            processing_queue->preemptionDispatchEvents,
            processing_queue->pLogMgr);
    }
    if (RV_OK == retCode)
    {
        RvLogLeave(processing_queue->pLogSource, (processing_queue->pLogSource,
            "PQUEUE_TailEvent - (hQueue=0x%p(%s) ev=0x%p)=%d",
            processing_queue,processing_queue->name,ev,retCode));
    }
    else
    {
        RvLogError(processing_queue->pLogSource, (processing_queue->pLogSource,
            "PQUEUE_TailEvent - (hQueue=0x%p(%s) ev=0x%p)=%d",
            processing_queue,processing_queue->name,ev,RvErrorGetCode(retCode)));
    }
	return RvErrorGetCode(retCode);
}


/************************************************************************************
 * PQUEUE_PopEvent
 * ----------------------------------------------------------------------------------
 * General: Retrive & delete event from the head of the queue
 * Return Value: RvStatus -	RV_ERROR_UNKNOWN
 *								RV_OK
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hQueue		- queue handler
 * Output:  ev			- retrived event
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV PQUEUE_PopEvent(IN  RV_PROCESSING_QUEUE		hQueue,
										   OUT HQEVENT					*ev)
{
	RvStatus			retCode;
	PQUEUE*             processing_queue = (PQUEUE*)hQueue;
    RLIST_ITEM_HANDLE   listItem  = NULL;

    if(processing_queue == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }
    
    RvLogEnter(processing_queue->pLogSource, (processing_queue->pLogSource,
        "PQUEUE_PopEvent - (hQueue=0x%p(%s) ev=0x%p)",
        processing_queue,processing_queue->name,ev));

	if ((ev == NULL))
	{
        RvLogError(processing_queue->pLogSource, (processing_queue->pLogSource,
            "PQUEUE_PopEvent - (hQueue=0x%p(%s) ev=0x%p)",
            processing_queue,processing_queue->name,ev));
        return RV_ERROR_UNKNOWN;
	}

	if (processing_queue->isMultiThreaded)
    {
		retCode = RvSemaphoreWait(processing_queue->newMsgSync,processing_queue->pLogMgr);
		if (retCode != RV_OK)
		{
			RvLogError(processing_queue->pLogSource, (processing_queue->pLogSource,
                "PQUEUE_PopEvent - (hQueue=0x%p(%s))=%d,failed wating for semaphore",
                processing_queue,processing_queue->name,RvErrorGetCode(retCode)));
			return retCode;
		}
	}

	if(processing_queue->startShutdown == RV_TRUE)
	{
        RvLogDebug(processing_queue->pLogSource, (processing_queue->pLogSource,
            "PQUEUE_PopEvent(hQueue=0x%p(%s)): shutdown started",
            processing_queue,processing_queue->name));
		return RV_ERROR_DESTRUCTED;
	}

	RvMutexLock(processing_queue->hLock,processing_queue->pLogMgr);
    RLIST_GetHead(processing_queue->hEventQueuePool, processing_queue->hEventQueue, &listItem);
	if (listItem == NULL)
	{
		if (processing_queue->isMultiThreaded)
        {
            RvLogError(processing_queue->pLogSource, (processing_queue->pLogSource,
                "PQUEUE_PopEvent - (hQueue=0x%p(%s))=%d, failed to get head of list",
                processing_queue,processing_queue->name, RV_ERROR_NOT_FOUND));
		}
    	RvMutexUnlock(processing_queue->hLock,processing_queue->pLogMgr);
		return RV_ERROR_NOT_FOUND;
	}
    *ev = *((HQEVENT*)listItem);
    RLIST_Remove(processing_queue->hEventQueuePool, processing_queue->hEventQueue,listItem);

    RvMutexUnlock(processing_queue->hLock,processing_queue->pLogMgr);
    RvLogInfo(processing_queue->pLogSource, (processing_queue->pLogSource,
        "PQUEUE_PopEvent - (hQueue=0x%p(%s), *ev=0x%p)=%d,",
        processing_queue,processing_queue->name,*ev,RV_OK));


    RvLogLeave(processing_queue->pLogSource, (processing_queue->pLogSource,
        "PQUEUE_PopEvent - (hQueue=0x%p(%s), *ev=0x%p)=%d,",
        processing_queue,processing_queue->name,*ev,RV_OK));

	return RV_OK;
}

/******************************************************************************
 * PQUEUE_PopEventForced
 * ----------------------------------------------------------------------------
 * General: Retrive & delete event from the head of the queue, even the queue
 *          is being shutdowned.
 *          Application should not destruct the PQueue, if it migh have events.
 * Return Value: RV_OK - on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hQueue - queue handler
 * Output:  ev     - retrived event.
 *                   NULL, if no events left in the queue due to queue shutdown
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV PQUEUE_PopEventForced(IN  RV_PROCESSING_QUEUE hQueue,
                                                OUT HQEVENT*            pEv)
{
    RvStatus            retCode;
    PQUEUE*             pQueue = (PQUEUE*)hQueue;
    RLIST_ITEM_HANDLE   listItem = NULL;

    if(NULL == pQueue)
    {
        return RV_ERROR_UNKNOWN;
    }

    RvLogEnter(pQueue->pLogSource, (pQueue->pLogSource,
        "PQUEUE_PopEventForced - (hQueue=0x%p(%s) pEv=0x%p)",
        pQueue,pQueue->name,pEv));

    if ((pEv == NULL))
    {
        RvLogError(pQueue->pLogSource, (pQueue->pLogSource,
            "PQUEUE_PopEventForced - (hQueue=0x%p(%s) pEv=0x%p)",
            pQueue,pQueue->name,pEv));
        return RV_ERROR_UNKNOWN;
    }

    /* For multithreading - wait for event notification before pop.
    If shutdown was started, don't wait - enable application to pop all events
    in loop */
    if (pQueue->isMultiThreaded &&
        pQueue->startShutdown   == RV_FALSE)
    {
        retCode = RvSemaphoreWait(pQueue->newMsgSync,pQueue->pLogMgr);
        if (retCode != RV_OK)
        {
            RvLogError(pQueue->pLogSource, (pQueue->pLogSource,
                "PQUEUE_PopEventForced - (hQueue=0x%p(%s))=%d,failed wating for semaphore",
                pQueue,pQueue->name,RvErrorGetCode(retCode)));
            return retCode;
        }
    }

    RvMutexLock(pQueue->hLock,pQueue->pLogMgr);
    RLIST_GetHead(pQueue->hEventQueuePool, pQueue->hEventQueue, &listItem);
    if (listItem == NULL)
    {
        if(pQueue->startShutdown == RV_TRUE)
        {
            *pEv = NULL;
            RvMutexUnlock(pQueue->hLock,pQueue->pLogMgr);
            return RV_OK;
        }

        if (pQueue->isMultiThreaded)
        {
            RvLogError(pQueue->pLogSource, (pQueue->pLogSource,
                "PQUEUE_PopEventForced - (hQueue=0x%p(%s))=%d, failed to get head of list",
                pQueue,pQueue->name,RV_ERROR_UNKNOWN));
        }
        RvMutexUnlock(pQueue->hLock,pQueue->pLogMgr);
        return RV_ERROR_UNKNOWN;
    }
    *pEv = *((HQEVENT*)listItem);
    RLIST_Remove(pQueue->hEventQueuePool, pQueue->hEventQueue,listItem);

    RvMutexUnlock(pQueue->hLock,pQueue->pLogMgr);

    RvLogInfo(pQueue->pLogSource, (pQueue->pLogSource,
        "PQUEUE_PopEventForced - (hQueue=0x%p(%s), *pEv=0x%p)=%d,",
        pQueue,pQueue->name,*pEv,RV_OK));

    RvLogLeave(pQueue->pLogSource, (pQueue->pLogSource,
        "PQUEUE_PopEventForced - (hQueue=0x%p(%s), *pEv=0x%p)=%d,",
        pQueue,pQueue->name,*pEv,RV_OK));

    return RV_OK;
}

/************************************************************************************
 * PQUEUE_AllocateEvent
 * ----------------------------------------------------------------------------------
 * General: Allocates event structure from events pool
 * Return Value: RvStatus -	RV_ERROR_UNKNOWN
 *								RV_ERROR_OUTOFRESOURCES
 *								RV_OK
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hQueue		- queue handler
 * Output:  ev			- allocated event
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV PQUEUE_AllocateEvent(IN  RV_PROCESSING_QUEUE		hQueue,
												OUT HQEVENT					*ev)
{
	RvStatus	retCode = RV_OK;
	PQUEUE	*processing_queue;
	RA_Element	elementPtr;

	processing_queue = (PQUEUE *)hQueue;

	if ((ev == NULL) || (processing_queue == NULL))
	{
		return RV_ERROR_UNKNOWN;
	}

	RvMutexLock(processing_queue->hLock,processing_queue->pLogMgr);
    
    RvLogEnter(processing_queue->pLogSource, (processing_queue->pLogSource,
        "PQUEUE_AllocateEvent - (hQueue=0x%p(%s) ev=0x%p)",
        processing_queue,processing_queue->name,ev));
    
    /* If failed to post CELL_AVAILABLE message in past, try it again */
	if (processing_queue->tryAgainCellAvailable == RV_TRUE)
	{
        RvStatus crv = RV_OK;
        crv = RvSelectStopWaiting(processing_queue->pSelect,
                processing_queue->preemptionCellAvailable,
                processing_queue->pLogMgr);
        if (RV_OK != crv)
        {
            processing_queue->tryAgainCellAvailable = RV_TRUE;
            RvLogWarning(processing_queue->pLogSource, (processing_queue->pLogSource,
                "PQUEUE_AllocateEvent - (hQueue=0x%p(%s)), error while stop waiting",
                processing_queue,processing_queue->name));
        }
        else
        {
            processing_queue->tryAgainCellAvailable = RV_FALSE;
            RvLogDebug(processing_queue->pLogSource, (processing_queue->pLogSource,
                "PQUEUE_AllocateEvent - (hQueue=0x%p(%s)), CELL_AVAILABLE message was posted",
                processing_queue,processing_queue->name));
        }
 	}

    /* Allocate event */
	retCode = RA_Alloc(processing_queue->hEventsPool,&elementPtr);
	if (retCode != RV_OK)
	{
        RvLogWarning(processing_queue->pLogSource, (processing_queue->pLogSource,
            "PQUEUE_AllocateEvent - (hQueue=0x%p(%s) ev=0x%p)=%d, out of event resources",
            processing_queue,processing_queue->name,ev,RV_ERROR_OUTOFRESOURCES));
		processing_queue->noEnoughEvents = RV_TRUE;
		RvMutexUnlock(processing_queue->hLock,processing_queue->pLogMgr);
		return RV_ERROR_OUTOFRESOURCES;
	}
	RvMutexUnlock(processing_queue->hLock,processing_queue->pLogMgr);

	*ev = (HQEVENT) elementPtr;

    RvLogLeave(processing_queue->pLogSource, (processing_queue->pLogSource,
            "PQUEUE_AllocateEvent - (hQueue=0x%p(%s) *ev=0x%p)=%d",
            processing_queue,processing_queue->name,*ev,RV_OK));

	return RV_OK;
}


/************************************************************************************
 * PQUEUE_FreeEvent
 * ----------------------------------------------------------------------------------
 * General: Returns event structure to the events pool
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hQueue		- queue handler
 *			ev			- event to be freed
***********************************************************************************/
RVAPI void RVCALLCONV PQUEUE_FreeEvent(IN  RV_PROCESSING_QUEUE		hQueue,
									   IN  HQEVENT					ev)
{
	PQUEUE	*processing_queue;
	RvStatus			stat;

	processing_queue = (PQUEUE *)hQueue;

	if ((ev == NULL) || (processing_queue == NULL))
	{
		return;
	}

	/*	Lock used to protect from following situation:
		thread 1 - main thread that allocates message received event
		thread 2 - processing thread that frees object termination event
		----------------------------------------------------------------
		thread 1 -	applied RA_Alloc that returned out of resources error
		CONTEXT SWITCH
		thread 2 -	applied RA_DeAllocByPtr and reset processing_queue->noEnoughEvents
					to RV_FALSE (now there is one event available)
		CONTEXT SWITCH
		thread 1 -	continues execution and sets processing_queue->noEnoughEvents
					back to RV_TRUE. That will cause sending 'Resource available'
					notification whithout need when calling PQUEUE_FreeEvent function
					next time.
	*/
	RvMutexLock(processing_queue->hLock,processing_queue->pLogMgr);

    RvLogEnter(processing_queue->pLogSource, (processing_queue->pLogSource,
        "PQUEUE_FreeEvent - (hQueue=0x%p(%s) ev=0x%p)",
        processing_queue,processing_queue->name,ev));

	stat = RA_DeAllocByPtr(processing_queue->hEventsPool,(RA_Element)ev);
	if (((stat == RV_OK) && (processing_queue->noEnoughEvents)) ||
        processing_queue->tryAgainCellAvailable == RV_TRUE)
	{
        RvStatus crv = RV_OK;
		processing_queue->noEnoughEvents = RV_FALSE;
        
        RvLogInfo(processing_queue->pLogSource, (processing_queue->pLogSource,
            "PQUEUE_FreeEvent - (hQueue=0x%p(%s)), recover from out of resources (TRY_AGAIN=%d)",
            processing_queue,processing_queue->name, processing_queue->tryAgainCellAvailable));

        crv = RvSelectStopWaiting(processing_queue->pSelect,
                processing_queue->preemptionCellAvailable,
                processing_queue->pLogMgr);
        if (RV_OK != crv)
        {
            processing_queue->tryAgainCellAvailable = RV_TRUE;
            RvLogWarning(processing_queue->pLogSource, (processing_queue->pLogSource,
                "PQUEUE_FreeEvent - (hQueue=0x%p(%s)), error while stop waiting",
                processing_queue,processing_queue->name));
        }
        else
        {
            processing_queue->tryAgainCellAvailable = RV_FALSE;
        }
 	}
    RvLogLeave(processing_queue->pLogSource, (processing_queue->pLogSource,
        "PQUEUE_FreeEvent - (hQueue=0x%p(%s) ev=0x%p)",
        processing_queue,processing_queue->name,ev));

	RvMutexUnlock(processing_queue->hLock,processing_queue->pLogMgr);
}


/************************************************************************************************************************
 * PQUEUE_GetResourcesStatus
 * purpose : Return information about the resource allocation of this RLIST pool.
 * input   : hQueue   - handle to the queue.
 * out     : queueRresources - The queue resources
 *           eventResources  - The event pool resources.
 ************************************************************************************************************************/
void  RVAPI RVCALLCONV PQUEUE_GetResourcesStatus(IN   RV_PROCESSING_QUEUE  hQueue ,
                                                 OUT RV_Resource*  queueRresources,
                                                 OUT RV_Resource*  eventResources)

{
    RV_Resource  queuePoolRresources;
	PQUEUE	*processing_queue;
	processing_queue = (PQUEUE *)hQueue;
    if(processing_queue == NULL)
    {
        memset(queueRresources,0,sizeof(RV_Resource));
        memset(eventResources,0,sizeof(RV_Resource));
        return;
    }
    RA_GetResourcesStatus(processing_queue->hEventsPool,eventResources);
    RLIST_GetResourcesStatus(processing_queue->hEventQueuePool,
                             &queuePoolRresources.maxNumOfElements,
                             &queueRresources->maxNumOfElements,
                             &queuePoolRresources.numOfUsed,
                             &queueRresources->numOfUsed,
                             &queuePoolRresources.maxUsage,
                             &queueRresources->maxUsage);
}


#ifdef __cplusplus
}
#endif
