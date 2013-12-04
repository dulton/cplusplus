/***********************************************************************
        Copyright (c) 2002 RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Ltd.. No part of this document may be reproduced in any
form whatsoever without written prior approval by RADVISION Ltd..

RADVISION Ltd. reserve the right to revise this publication and make
changes without obligation to notify any person of such revisions or
changes.
***********************************************************************/

#ifndef _RTPT_THREADS_H_
#define _RTPT_THREADS_H_

/* General purpose thread safety functions used by the test application
   for creation and synchronization of threads - nothing fancy. */


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "rvtypes.h"
#include "RTPT_general.h"


#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/
typedef int (* ThreadMainFunc) (IN rtptObj **pTerm, IN void *context);

/*-----------------------------------------------------------------------*/
/*                           FUNCTIONS HEADERS                           */
/*-----------------------------------------------------------------------*/


/******************************************************************************
 * termCurrentTime
 * ----------------------------------------------------------------------------
 * General: Gives the current relative time in milliseconds.
 *          This function isn't really a thread-related function, but it is
 *          OS dependent, so we put it here.
 *
 * Return Value: Current time in ms
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  term         - Terminal object to use.
 * Output: None
 *****************************************************************************/
RvUint32 termCurrentTime(
    IN rtptObj              *term);


/******************************************************************************
 * termThreadCurrent
 * ----------------------------------------------------------------------------
 * General: Give an OS specific thread handle. This can be used to check the
 *          thread's id.
 *
 * Return Value: Current thread's unique id
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  term         - Terminal object to use.
 * Output: None
 *****************************************************************************/
int termThreadCurrent(
    IN rtptObj              *term);


/******************************************************************************
 * termThreadCreate
 * ----------------------------------------------------------------------------
 * General: Create and start the execution of a thread
 *
 * Return Value: RV_OK on success, other on failure.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pTerm    - Terminal object to use.
 *         func     - Function to start running in the new thread
 *         context  - Context passed to the thread function
 * Output: None
 *****************************************************************************/
RvStatus termThreadCreate(
    IN rtptObj          **pTerm,
    IN ThreadMainFunc   func,
    IN void             *context);


/******************************************************************************
 * termThreadSleep
 * ----------------------------------------------------------------------------
 * General: Sleep for a given amount of time.
 *
 * Return Value: RV_OK on success, other on failure.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  term         - Terminal object to use.
 *         ms           - Time to sleep in milliseconds.
 * Output: None
 *****************************************************************************/
RvStatus termThreadSleep(
    IN rtptObj              *term,
    IN RvUint32             ms);


/******************************************************************************
 * termThreadTimer
 * ----------------------------------------------------------------------------
 * General: Wait for a given amount of time for function invocation.
 *          The function might be called from any active thread.
 *
 * Return Value: RV_OK on success, other on failure.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pTerm    - Terminal object to use.
 *         ms       - Milliseconds to wait until function is called.
 *         func     - Function to run after the interval passes.
 *         context  - Context passed to the thread function.
 * Output: None
 *****************************************************************************/
RvStatus termThreadTimer(
    IN rtptObj          **pTerm,
    IN RvUint32         ms,
    IN ThreadMainFunc   func,
    IN void             *context);

#if 0
/******************************************************************************
 * termLockInit
 * ----------------------------------------------------------------------------
 * General: Initialize a lock. This is a non-recursive lock.
 *
 * Return Value: Lock object on success.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  term         - Terminal object to use.
 * Output: None.
 *****************************************************************************/
LockHANDLE termLockInit(
    IN rtptObj              *term);


/******************************************************************************
 * termLockEnd
 * ----------------------------------------------------------------------------
 * General: Destruct a lock.
 *
 * Return Value: RV_OK on success, other on failure.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  term         - Terminal object to use.
 *         lock         - Lock object to use.
 * Output: None.
 *****************************************************************************/
RvStatus termLockEnd(
    IN rtptObj              *term,
    IN LockHANDLE           lock);


/******************************************************************************
 * termLockLock
 * ----------------------------------------------------------------------------
 * General: Lock a lock.
 *
 * Return Value: RV_OK on success, other on failure.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  term         - Terminal object to use.
 *         lock         - Lock object to use.
 * Output: None.
 *****************************************************************************/
RvStatus termLockLock(
    IN rtptObj              *term,
    IN LockHANDLE           lock);


/******************************************************************************
 * termLockUnlock
 * ----------------------------------------------------------------------------
 * General: Unlock a lock.
 *
 * Return Value: RV_OK on success, other on failure.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  term         - Terminal object to use.
 *         lock         - Lock object to use.
 * Output: None.
 *****************************************************************************/
RvStatus termLockUnlock(
    IN rtptObj              *term,
    IN LockHANDLE           lock);

#endif
/******************************************************************************
 * thread_doContext
 * ----------------------------------------------------------------------------
 * General: Timer invocation function used from TCL.
 * Syntax: thread:doContext <func> <context>
 *
 * Return Value: TCL_OK
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  Regular TCL
 * Output: None
 *****************************************************************************/
int thread_doContext(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);








#ifdef __cplusplus
}
#endif

#endif /* _RTPT_THREADS_H_ */
