/* rvadthread.c - POSIX adapter thread management functions */
/************************************************************************
        Copyright (c) 2001 RADVISION Inc. and RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Inc. and RADVISION Ltd.. No part of this document may be
reproduced in any form whatsoever without written prior approval by
RADVISION Inc. or RADVISION Ltd..

RADVISION Inc. and RADVISION Ltd. reserve the right to revise this
publication and make changes without obligation to notify any person of
such revisions or changes.
***********************************************************************/
#include "rvadthread.h"

#if (RV_THREAD_TYPE != RV_THREAD_NONE)
#include <errno.h>
#if (RV_OS_TYPE == RV_OS_TYPE_INTEGRITY)
#include <sys/socket.h>
#endif


/*
 * Some pthread implementations don't reuse TLS variables, e.g. once created 
 *  pthread_key value isn't reused anymore, even if pthread_key_delete is 
 *  called. This means that after some number of 
 *  <pthread_key_create, pthread_key_delete> iterations any future 
 *  pthread_key_create will fail. The problem is aggravated in implementations
 *  where maximal number of a keys is limited by some small number.
 *  So, in order to solve this problem we choose never delete allocated 
 *  pthread_key
 */

static RvBool threadKeyAllocated = RV_FALSE;
static pthread_key_t RvThreadCurrentKey; /* Used to track current thread ptr */


static void* RvAdThreadWrapper(void* arg1)
{
#if (RV_OS_TYPE == RV_OS_TYPE_INTEGRITY)
    InitLibSocket();
#endif

    RvThreadWrapper(arg1, RV_FALSE);

#if (RV_OS_TYPE == RV_OS_TYPE_INTEGRITY)
    /* ShutdownLibSocket();  !!! causes problems to other threads ??? */
#endif

    return 0;
}


RvStatus RvAdThreadInit(void)
{
    /* create a thread specific variable for holding copies of thread pointers */
    if(!threadKeyAllocated) {
        if(pthread_key_create(&RvThreadCurrentKey, RvThreadExitted) != 0) {
            return RV_ERROR_UNKNOWN;
        }
        threadKeyAllocated = RV_TRUE;
    }
    
    return RV_OK;
}


void RvAdThreadEnd(void)
{
    /* pthread_key_delete(RvThreadCurrentKey); */
}


RvStatus RvAdThreadConstruct(
    IN  RvThreadBlock*  tcb)
{
    if(pthread_attr_init(tcb) != 0)
        return RV_ERROR_UNKNOWN;

    return RV_OK;
}


void RvAdThreadDestruct(
    IN  RvThreadBlock*  tcb)
{
    pthread_attr_destroy(tcb);
}

/* Allowed values for priority are:
 * 0 - for SCHED_OTHER policy
 * 0..99 - for SCHED_RR and SCHED_FIFO
 */
static RvInt RvAdNormalizePriority(RvInt policy, RvInt priority) {
    if(policy == SCHED_OTHER) {
        return 0;
    } 
 
    if(priority < 1) {
        return 1;
    }

    if(priority > 99) {
        return 99;
    }
    

    return priority;
}

RvStatus RvAdThreadCreate(
    IN  RvThreadBlock*  tcb,
    IN  RvChar*         name,
    IN  RvInt32         priority,
    IN  RvThreadAttr*   attr,
    IN  void*           stackaddr,
    IN  RvInt32         realstacksize,
    IN  void*           arg1,
    OUT RvThreadId*     id)
{
    int result;
    struct sched_param params;

    RV_UNUSED_ARG(name);
    RV_UNUSED_ARG(stackaddr);
    RV_UNUSED_ARG(arg1);
    RV_UNUSED_ARG(id);

    /* We can't actually create the thread, but we can set up all the attributes. */
    /* We'll have to set a copy of the thread pointer in the ThreadWrapper. */
    result = pthread_attr_getschedparam(tcb, &params);
    
    if(result == 0) {
        /* Set correct values for priority depending on scheduling policy
         * For scheduling policy == SCHED_OTHER the only legal value is 0
         * otherwise priority should be in the range 0..99
         */
        params.sched_priority = RvAdNormalizePriority(attr->policy, priority);
        /* FIX THIS result |= */ pthread_attr_setschedparam(tcb, &params);
    }
    
    result |= pthread_attr_setdetachstate(tcb, PTHREAD_CREATE_DETACHED);
    if (realstacksize != 0)
        result |= pthread_attr_setstacksize(tcb, realstacksize);

    /* Now set attributes we allow the user to set */
    result |= pthread_attr_setschedpolicy(tcb, attr->policy);
#if (RV_OS_TYPE == RV_OS_TYPE_INTEGRITY)
    result |= pthread_attr_setthreadname(tcb, name);
#else
    result |= pthread_attr_setscope(tcb, attr->contentionscope);
    result |= pthread_attr_setinheritsched(tcb, attr->inheritsched);
#endif
#if (RV_OS_TYPE == RV_OS_TYPE_SOLARIS) && (RV_OS_VERSION > RV_OS_SOLARIS_2_6)
    /* Solaris extension */
    if(attr->guardsize != (size_t)(-1))
        result |= pthread_attr_setguardsize(tcb, attr->guardsize);
#endif

    return result;
}


RvStatus RvAdThreadStart(
    IN  RvThreadBlock*  tcb,
    IN  RvThreadId*     id,
    IN  RvThreadAttr*   attr,
    IN  void*           arg1)
{
    RV_UNUSED_ARG(attr);

    if(pthread_create(id, tcb, RvAdThreadWrapper, arg1) != 0)
        return RV_ERROR_UNKNOWN;

    return RV_OK;
}


RvStatus RvAdThreadDelete(
    IN  RvThreadBlock*  tcb,
    IN  RvThreadId      id)
{
    RV_UNUSED_ARG(tcb);
    RV_UNUSED_ARG(id);

    return RV_OK;
}


RvStatus RvAdThreadWaitOnExit(
    IN  RvThreadBlock*  tcb,
    IN  RvThreadId      id)
{
    RV_UNUSED_ARG(tcb);
    RV_UNUSED_ARG(id);

    return RV_OK;
}


RvStatus RvAdThreadSetTls(
    IN  RvThreadId      id,
    IN  RvInt32         state,
    IN  void*           tlsData)
{
    RV_UNUSED_ARG(id);
    RV_UNUSED_ARG(state);

    /* Use a thread specific variable to store a copy of the thread pointer. */
    /* RvThreadCurrentKey must be set up in RvThreadInit. */
    /**** Pointer is set for current task NOT the task pointer to by th. ****/
    if(pthread_setspecific(RvThreadCurrentKey, tlsData) != 0)
        return RV_ERROR_UNKNOWN;

    return RV_OK;
}


void* RvAdThreadGetTls(void)
{
    return pthread_getspecific(RvThreadCurrentKey);
}


RvThreadId RvAdThreadCurrentId(void)
{
    return pthread_self();
}


RvBool RvAdThreadIdEqual(
    IN  RvThreadId      id1,
    IN  RvThreadId      id2)
{
    return (pthread_equal(id1, id2) != 0);
}


RvStatus RvAdThreadSleep(
    IN  const RvTime*   t)
{
    struct timespec delaytime, timeleft;

    /* To delay properly, we have to resume after being interrupted by a signal */
    delaytime.tv_sec = RvTimeGetSecs(t);
    delaytime.tv_nsec = RvTimeGetNsecs(t);
    for(;;) {
        if(nanosleep(&delaytime, &timeleft) == 0)
            return RV_OK;
        if(errno != EINTR)
            return RV_ERROR_UNKNOWN;
        delaytime.tv_sec = timeleft.tv_sec;
        delaytime.tv_nsec = timeleft.tv_nsec;
    }
}


void RvAdThreadNanosleep(
    IN  RvInt64         nsecs)
{
    /* For those who put the sleep code in RvThreadSleep, call it */
    RvTime t;

    RvTimeConstructFrom64(nsecs, &t);
    RvAdThreadSleep(&t);
}


RvStatus RvAdThreadGetPriority(
    IN  RvThreadId      id,
    OUT RvInt32*        priority)
{
    int policy;
    struct sched_param params;

    if(pthread_getschedparam(id, &policy, &params) != 0)
        return RV_ERROR_UNKNOWN;

    *priority = (RvInt32)params.sched_priority;

    return RV_OK;
}


RvStatus RvAdThreadSetPriority(
    IN  RvThreadBlock*  tcb,
    IN  RvThreadId      id,
    IN  RvInt32         priority,
    IN  RvInt32         state)
{
    struct sched_param params;
    int policy;

    /* Thread isn't started yet, so save priority settings
     *  for later use in RvAdThreadStart.
     * We arrive to this point only in the case that the thread state is one
     * of the CREATED, STARTING, or RUNNING.
     * In the STARTING or RUNNING states OS thread already exists, so
     * we don't need to save priority information locally
     */
    if(state == RV_THREAD_STATE_CREATED) {
      if(pthread_attr_getschedparam(tcb, &params) != 0 || 
         pthread_attr_getschedpolicy(tcb, &policy) != 0) {
        return RV_ERROR_UNKNOWN;
      }


      priority = RvAdNormalizePriority(policy, priority);

      params.sched_priority = priority;
      if(pthread_attr_setschedparam(tcb, &params) != 0) {
        return RV_ERROR_UNKNOWN;
      }

      return RV_OK;
    }

    /* If thread has been started so tell the scheduler about it */
    if(pthread_getschedparam(id, &policy, &params) != 0) {
      return RV_ERROR_UNKNOWN;
    }

    priority = RvAdNormalizePriority(policy, priority);

    params.sched_priority = priority;
    if(pthread_setschedparam(id, policy, &params) != 0) {
      return RV_ERROR_UNKNOWN;
    }

    return RV_OK;
}


RvStatus RvAdThreadGetName(
    IN  RvThreadId      id,
    IN  RvInt32         size,
    OUT RvChar*         buf)
{
    RV_UNUSED_ARG(id);
    RV_UNUSED_ARG(size);

    /* POSIX (and variations) don't keep names of threads */
    /* so we'll just return an empty string. */
    buf[0] = '\0';

    return RV_OK;
}

#endif /* RV_THREAD_TYPE != RV_THREAD_NONE */
