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

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RTPT_general.h"
#include "RTPT_threads.h"

#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)
#include <process.h>
#elif (RV_OS_TYPE == RV_OS_TYPE_SOLARIS) || (RV_OS_TYPE == RV_OS_TYPE_LINUX) || (RV_OS_TYPE == RV_OS_TYPE_UNIXWARE)
#include <sys/time.h>
#include <pthread.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

typedef struct
{
    rtptObj         **pTerm;
    ThreadMainFunc  func;
    void*           context;
} ThreadInfo;

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)
static unsigned __stdcall ThreadStartFunc(IN void *context);
#else
static void* ThreadStartFunc(IN void *context);
#endif



/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
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
    IN rtptObj              *term)
{
#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)
    RV_UNUSED_ARG(term);

    return (RvUint32)GetCurrentTime();

#elif (RV_OS_TYPE == RV_OS_TYPE_SOLARIS) || (RV_OS_TYPE == RV_OS_TYPE_LINUX) || (RV_OS_TYPE == RV_OS_TYPE_UNIXWARE)
    RvUint32 result;
    static RvUint32 sSeconds = 0;
    struct timeval tv;

    RV_UNUSED_ARG(term);

    gettimeofday(&tv, NULL);
    if (sSeconds == 0)
        sSeconds = (RvUint32)tv.tv_sec;

    result = (RvUint32)( (tv.tv_sec - sSeconds) * 1000 + (tv.tv_usec / 1000) );

    return (result == 0xffffffff) ? 0 : result;
#else
#error Implement for OS!
#endif
}


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
    IN rtptObj              *term)
{
    RV_UNUSED_ARG(term);

#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)
    return (int)GetCurrentThreadId();

#elif (RV_OS_TYPE == RV_OS_TYPE_SOLARIS) || (RV_OS_TYPE == RV_OS_TYPE_LINUX) || (RV_OS_TYPE == RV_OS_TYPE_UNIXWARE)
    return (int)pthread_self();

#else
#error Implement for OS!
#endif
}


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
    IN void             *context)
{
    ThreadInfo *tInfo;
    RvBool anyError = RV_FALSE;
    rtptObj     *term = *pTerm;

    tInfo = (ThreadInfo *)termMalloc(term, sizeof(ThreadInfo));
    if (tInfo == NULL)
        return RV_ERROR_OUTOFRESOURCES;

    tInfo->pTerm = pTerm;
    tInfo->func = func;
    tInfo->context = context;

#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)
    {
        unsigned tid;

        if (_beginthreadex(NULL, 0, ThreadStartFunc, tInfo, 0, &tid) == 0)
            anyError = RV_TRUE;
    }
#else
    {
        pthread_t tid;

        if (pthread_create(&tid, NULL, ThreadStartFunc, (void*)tInfo) != 0)
            anyError = RV_TRUE;
    }
#endif


    if (anyError)
    {
        termFree(term, tInfo);
        return RV_ERROR_UNKNOWN;
    }

    return RV_OK;
}


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
    IN RvUint32             ms)
{
    RV_UNUSED_ARG(term);

    Tcl_Sleep(ms);

    return RV_OK;
}


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
    IN void             *context)
{
    RV_UNUSED_ARG(pTerm);

    TclExecute("after %d {thread:doContext %p %p}", ms, func, context);

    return RV_OK;
}

#ifdef LOCK_CODE_ENABLE

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
    IN rtptObj              *term)
{
    LockHANDLE lock;

#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)
    RvUint32 *lockCount;
    lock = (LockHANDLE)termMalloc(term, sizeof(*lockCount)+sizeof(CRITICAL_SECTION));
    lockCount = (RvUint32 *)lock;
    if (lock != NULL)
    {
        *lockCount = 0;
        InitializeCriticalSection((LPCRITICAL_SECTION)(lockCount+1));
    }

#if (RV_MUTEX_TYPE == RV_MUTEX_SOLARIS)
        RvMutexLogError((&logMgr->mutexSource, "RvMutexConstruct(mutex=%p) pthread_mutexattr_settype failure", mu));
        return RvMutexErrorCode(RV_ERROR_UNKNOWN);
    }
#endif
    
#elif (RV_OS_TYPE == RV_OS_TYPE_SOLARIS) || (RV_OS_TYPE == RV_OS_TYPE_LINUX) || (RV_OS_TYPE == RV_OS_TYPE_UNIXWARE)
    {
        pthread_mutexattr_t ma;
        pthread_mutexattr_init(&ma);
#if (RV_OS_TYPE == RV_OS_TYPE_SOLARIS)
        pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);
#elif (RV_OS_TYPE == RV_OS_TYPE_LINUX)
        pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE_NP);
#endif
        lock = (LockHANDLE)termMalloc(term, sizeof(pthread_mutex_t));
        pthread_mutex_init((pthread_mutex_t*)lock, &ma);
    }
#else

#error Implement for OS!
#endif

    return lock;
}

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
    IN LockHANDLE           lock)
{
    if (lock == NULL)
        return RV_ERROR_NULLPTR;

#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)
    {
        RvUint32 *lockCount = (RvUint32 *)lock;
        if (*lockCount > 0)
            *lockCount = 1 / (term == NULL);
        DeleteCriticalSection((LPCRITICAL_SECTION)(lockCount+1));
    }

#elif (RV_OS_TYPE == RV_OS_TYPE_SOLARIS) || (RV_OS_TYPE == RV_OS_TYPE_LINUX) || (RV_OS_TYPE == RV_OS_TYPE_UNIXWARE)
        pthread_mutex_destroy((pthread_mutex_t*)lock);
#else

#error Implement for OS!
#endif

    termFree(term, (void*)lock);
    return RV_OK;
}


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
    IN LockHANDLE           lock)
{
    RvStatus status = RV_OK;

    RV_UNUSED_ARG(term);

    if (lock == NULL)
        return RV_ERROR_NULLPTR;

#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)
    {
        RvUint32 *lockCount = (RvUint32 *)lock;
        EnterCriticalSection((LPCRITICAL_SECTION)(lockCount + 1));
        (*lockCount)++;
    }

#elif (RV_OS_TYPE == RV_OS_TYPE_SOLARIS) || (RV_OS_TYPE == RV_OS_TYPE_LINUX) || (RV_OS_TYPE == RV_OS_TYPE_UNIXWARE)
    {
        status = (RvStatus)pthread_mutex_lock((pthread_mutex_t*)lock);
        if (status != RV_OK)
            printf("ERROR: termLockLock - %d\n", status);
    }
#else

#error Implement for OS!
#endif

    return status;
}


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
    IN LockHANDLE           lock)
{
    RvStatus status = RV_OK;

    RV_UNUSED_ARG(term);

    if (lock == NULL)
        return RV_ERROR_NULLPTR;

#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)
    {
        RvUint32 *lockCount = (RvUint32 *)lock;
        if (*lockCount == 0)
            *lockCount = 1 / (term == NULL);
        (*lockCount)--;
        LeaveCriticalSection((LPCRITICAL_SECTION)(lockCount + 1));
    }

#elif (RV_OS_TYPE == RV_OS_TYPE_SOLARIS) || (RV_OS_TYPE == RV_OS_TYPE_LINUX) || (RV_OS_TYPE == RV_OS_TYPE_UNIXWARE)
    {
        status = (RvStatus)pthread_mutex_unlock((pthread_mutex_t*)lock);
        if (status != RV_OK)
            printf("ERROR: termLockUnlock - %d\n", status);
    }
#else

#error Implement for OS!
#endif

    return status;
}


#endif



/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)

/******************************************************************************
 * ThreadStartFunc
 * ----------------------------------------------------------------------------
 * General: Thread starting function. This one is implemented here so the
 *          user of this module implements a single function for all OS's.
 *
 * Return Value: Non-negative if successful, negative on failure
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  context  - Context given to AppThreadCreate().
 * Output: None
 *****************************************************************************/
static unsigned __stdcall ThreadStartFunc(IN void *context)
{
    ThreadInfo tInfo = *((ThreadInfo *)context);

    termFree(*(tInfo.pTerm), context);
    return (unsigned)tInfo.func(tInfo.pTerm, tInfo.context);
}


#else

/******************************************************************************
 * ThreadStartFunc
 * ----------------------------------------------------------------------------
 * General: Thread starting function. This one is implemented here so the
 *          user of this module implements a single function for all OS's.
 *
 * Return Value: Non-negative if successful, negative on failure
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  context  - Context given to AppThreadCreate().
 * Output: None
 *****************************************************************************/
static void* ThreadStartFunc(IN void *context)
{
    ThreadInfo tInfo = *((ThreadInfo *)context);

    termFree(*tInfo.pTerm, context);
    return (void*)(tInfo.func(tInfo.pTerm, tInfo.context));
}

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
int thread_doContext(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    rtptObj **pTerm = (rtptObj **)clientData;
    ThreadMainFunc func;
    void *context;

    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);

    sscanf(argv[1], "%p", &func);
    sscanf(argv[2], "%p", &context);

    func(pTerm, context);

    return TCL_OK;
}




#ifdef __cplusplus
}
#endif
