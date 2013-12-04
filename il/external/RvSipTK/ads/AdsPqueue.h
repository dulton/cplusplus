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
 *                              PQUEUE_API.h
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

#ifndef _TRANSPORT_QUEUE_TYPES_H
#define _TRANSPORT_QUEUE_TYPES_H


#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_ADS_DEF.h"
#include "rvselect.h"
#include "rvmutex.h"
#include "rvlog.h"

/*-----------------------------------------------------------------------*/
/*                                MACROS                                 */
/*-----------------------------------------------------------------------*/
#define PQUEUE_MESSAGE_WAITING      1
#define PQUEUE_CELL_AVAILABLE       2

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/* Declaration of the processing queue handler */
RV_DECLARE_HANDLE (RV_PROCESSING_QUEUE);

/* Declaration of the processing queue event handler */
RV_DECLARE_HANDLE (HQEVENT);


/************************************************************************************
 * PQUEUE_Construct
 * ----------------------------------------------------------------------------------
 * General: Construct processing queue.
 * Return Value: RvStatus -	RV_ERROR_OUTOFRESOURCES
 *								RV_OK
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   queueSize	- maximum lenght of the queue
 *			pLogMgr  	- log manager
 *			hLock		- queue lock
 *			maxEventLen - maximum event lenght
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
										   OUT RV_PROCESSING_QUEUE	*hQueue);

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
    IN RvUint8              preemptionCellAvailable);

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
RVAPI void RVCALLCONV PQUEUE_Destruct(IN RV_PROCESSING_QUEUE	hQueue);

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
RVAPI RvStatus RVCALLCONV PQUEUE_KillThread(IN RV_PROCESSING_QUEUE		hQueue);

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
											IN HQEVENT					ev);

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
										   OUT HQEVENT					*ev);

/************************************************************************************
 * PQUEUE_PopEventForced
 * ----------------------------------------------------------------------------------
 * General: Retrive & delete event from the head of the queue.
 *          Doesn't return error, if the queue is being shutdowned
 * Return Value: RvStatus -	RV_ERROR_UNKNOWN
 *								RV_OK
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hQueue		- queue handler
 * Output:  ev			- retrived event
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV PQUEUE_PopEventForced(IN  RV_PROCESSING_QUEUE	hQueue,
										        OUT HQEVENT*            ev);

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
												OUT HQEVENT					*ev);

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
									   IN  HQEVENT					ev);


/************************************************************************************************************************
 * PQUEUE_GetResourcesStatus
 * purpose : Return information about the resource allocation of this RLIST pool.
 * input   : hQueue   - handle to the pqueue.
 * out     : queueRresources - The queue resources
 *           eventResources  - The event pool resoueces.
 ************************************************************************************************************************/
void  RVAPI RVCALLCONV PQUEUE_GetResourcesStatus(IN   RV_PROCESSING_QUEUE  hQueue ,
                                                 OUT RV_Resource*  queueRresources,
                                                 OUT RV_Resource*  eventResources);


#ifdef __cplusplus
}
#endif

#endif  /*END OF: define _TRANSPORT_QUEUE_TYPES_H*/
