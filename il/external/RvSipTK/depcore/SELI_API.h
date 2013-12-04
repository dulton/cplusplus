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
 *                              <SELI_API.h>
 *
 *  In this file there are old core wrappers. these wrappers are deprecated and 
 *  are not recommended for use.
 *  The wrappers in this file are of the SELI module
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                  April 2004
 *********************************************************************************/
#ifndef _SELI_API_H
#define _SELI_API_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_DEF.h"

#if defined(RV_DEPRECATED_CORE)
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
#define WSELI_MAX_USER_FD 50

/* user data handle */
DECLARE_VOID_POINTER(RV_SELI_USER_DATA);

/* definition of user file descriptor */
typedef RvInt32 RV_SELI_FdHandle;


/********************************************************************************************
 * Description of RV_SELI_Events
 * This enumeration is used to distinguish between the different events that occur on a call
 * to select() on file descriptors.
 * EvNone       - No event. Used for unregistering events
 * EvRead       - Read event
 * EvWrite      - Write event
 * EvException  - Exception event
 ********************************************************************************************/
typedef enum {
    RV_SELI_EvNone          = 0x00,
    RV_SELI_EvRead          = 0x01,
    RV_SELI_EvWrite         = 0x02,
    RV_SELI_EvException     = 0x04
} RV_SELI_Events;

/********************************************************************************************
 * RV_SELI_EVENT_HANDLER callback
 * This event handler routine is called from the SELI package when one of the events above
 * (RV_SELI_Events) occured on a file descriptor.
 * fdHandle     - File descriptor handle
 * event        - The type of event that occured on the file descriptor
 * indicators   - may contain processedEvents - list of events, already processed 
 *                in this select iteration.
 *                contains indication for bytes on the socket after exiting the select
 * userData     - User data related with the specific file descriptor given
 * error        - Boolean indication of error occurances
 ********************************************************************************************/
typedef RV_Status (RVCALLCONV *RV_SELI_EVENT_HANDLER)(
                              IN RV_SELI_FdHandle    fdHandle,
						      IN RV_SELI_Events      event,
						      IN RV_SELI_Events      indicators,
						      IN RV_SELI_USER_DATA   userData,
						      IN RV_BOOL             error);


/*-----------------------------------------------------------------------*/
/*                            SELI FUNCTIONS                             */
/*-----------------------------------------------------------------------*/
/********************************************************************************************
 * SELI_CallOn
 * purpose : Register an event on a file descriptor.
 *           To unregister events, this function should be called with event=RV_SELI_EvNone
 *           Registered sockets will be re-registered by this function.
 * input   : hSeli          - SELI application handle to use
 *           fdHandle       - File descriptor handle to register on
 *           event          - Event to register for file descriptor
 *           eventHandler   - Callback function to call on event
 * output  : userData       - Pointer to the user data for this file descriptor
 *                            If set to NULL, it is ignored
 * return  : RV_Success - In case the routine has succeded.
 *           RV_Failure - In case the routine has failed.
 ********************************************************************************************/
RVAPI RV_Status RVCALLCONV SELI_CallOn (IN  RV_SELI_Handle         hSeli,
			                            IN  RV_SELI_FdHandle       fdHandle,
				                        IN  RV_SELI_Events         event,
						                IN  RV_SELI_EVENT_HANDLER  eventHandler,
							            OUT RV_SELI_USER_DATA*     userData);

/********************************************************************************************
 * SELI_CallOnThread
 * purpose : Register an event on a file descriptor in a specific thread.
 *           To unregister events, this function should be called with event=RV_SELI_EvNone
 *           Registered sockets will be re-registered by this function.
 * input   : hSeli          - SELI application handle to use
 *           fdHandle       - File descriptor handle to register on
 *           event          - Event to register for file descriptor
 *           eventHandler   - Callback function to call on event
 *           threadId       - Thread where event will be generated
 * output  : userData       - Pointer to the user data for this file descriptor
 *                            If set to NULL, it is ignored
 * return  : RV_Success - In case the routine has succeded.
 *           RV_Failure - In case the routine has failed.
 ********************************************************************************************/
RVAPI RV_Status RVCALLCONV SELI_CallOnThread(IN  RV_SELI_Handle         hSeli,
			                                 IN  RV_SELI_FdHandle       fdHandle,
				                             IN  RV_SELI_Events         event,
						                     IN  RV_SELI_EVENT_HANDLER  eventHandler,
							                 IN  RV_THREAD_Handle       threadId,
									         OUT RV_SELI_USER_DATA*     userData);

/********************************************************************************************
 * SELI_SetUserDataSize
 * purpose : Set the size of user data to associate with each file descriptor.
 *           
 * input   : hSeli      - Handle to SELI instance
 *           size       - Size of user data per file descriptor in bytes
 * output  : none
 * return  : RV_Success          - In case the routine has succeded.
 *           RV_OutOfResources   - Allocation problem
 *           RV_Failure          - In case the routine has failed.
 ********************************************************************************************/
RVAPI RV_Status RVCALLCONV SELI_SetUserDataSize (IN RV_SELI_Handle hSeli,
                                                 IN RvUint32      size);

/********************************************************************************************
 * SELI_GetUserData
 * purpose : Returns the user's data pointer for the specific file descriptor
 * input   : fdHandle       - File descriptor handle to use
 * output  : none
 * return  : User data pointer
 *           NULL on failure or when the file descriptor is not used
 ********************************************************************************************/
RVAPI RV_SELI_USER_DATA RVCALLCONV SELI_GetUserData (IN RV_SELI_FdHandle      fdHandle);

/********************************************************************************************
 * SELI_Select
 * purpose : Checks for events on all registered file descriptors and execute any callback
 *           functions necessary.
 *           This function also calls any timeout callbacks pending in the TIMER.
 *           SELI_Select can be called only from the thread that is the thread that initiated 
 *           the select engine
 * input   : none
 * output  : none
 * return  : RV_Success - In case the routine has succeded.
 *           RV_Failure - In case the routine has failed.
 ********************************************************************************************/
RVAPI RV_Status RVCALLCONV SELI_Select(void);

/********************************************************************************************
 * SELI_End
 * purpose : Ends the seli module
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
RVAPI RV_Status RVCALLCONV SELI_End(void);

#endif /*#if defined(RV_DEPRECATED_CORE) */
#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef _SELI_API_H */


