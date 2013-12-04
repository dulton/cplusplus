/* rvadsema4.c - POSIX adapter semaphore functions */
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
#include "rvadsema4.h"
#include "rvlog.h"
#include "rvccoreglobals.h"
#include <errno.h>


#if (RV_SEMAPHORE_TYPE == RV_SEMAPHORE_POSIX)

/* define the default attributes since they may be a structure */
/* static RvSemaphoreAttr RvDefaultSemaphoreAttr = RV_SEMAPHORE_ATTRIBUTE_DEFAULT; */


RvStatus RvAdSema4Construct(
    OUT RvSemaphore*    sema,
    IN  RvUint32        startcount,
    IN  RvLogMgr*       logMgr)
{
    RV_USE_CCORE_GLOBALS;
    RV_UNUSED_ARG(logMgr);

    if(sem_init((sem_t*)sema, RvDefaultSemaphoreAttr, startcount) < 0)
        return RV_ERROR_UNKNOWN;

    return RV_OK;
}


RvStatus RvAdSema4Destruct(
    IN  RvSemaphore*    sema,
    IN  RvLogMgr*       logMgr)
{
    RV_UNUSED_ARG(logMgr);

    if(sem_destroy((sem_t*)sema) < 0)
        return RV_ERROR_UNKNOWN;

    return RV_OK;
}


RvStatus RvAdSema4Post(
    IN  RvSemaphore*    sema,
    IN  RvLogMgr*       logMgr)
{
    RV_UNUSED_ARG(logMgr);

    if(sem_post((sem_t*)sema) < 0)
        return RV_ERROR_UNKNOWN;

    return RV_OK;
}


RvStatus RvAdSema4Wait(
    IN  RvSemaphore*    sema,
    IN  RvLogMgr*       logMgr)
{
    RV_UNUSED_ARG(logMgr);

    /* We must loop if we were awoken by a signal. */
    while(sem_wait((sem_t*)sema) < 0) {
        if(errno != EINTR)
            return RV_ERROR_UNKNOWN;
    }

    return RV_OK;
}


RvStatus RvAdSema4TryWait(
    IN  RvSemaphore*    sema,
    IN  RvLogMgr*       logMgr)
{
    RV_UNUSED_ARG(logMgr);

    /* We must loop if we were awoken by a signal. */
    while(sem_trywait((sem_t*)sema) < 0) {
        if ((errno == EAGAIN) || (errno == 0))
            return RV_ERROR_TRY_AGAIN;

        if(errno != EINTR)
            return RV_ERROR_UNKNOWN;
    }

    return RV_OK;
}


RvStatus RvAdSema4SetAttr(
    IN  RvSemaphoreAttr*attr,
    IN  RvLogMgr*       logMgr)
{
    RV_USE_CCORE_GLOBALS;
    RV_UNUSED_ARG(logMgr);

    RvDefaultSemaphoreAttr = *attr;

    return RV_OK;
}


#elif (RV_SEMAPHORE_TYPE == RV_SEMAPHORE_CONDVAR)

#if (RV_LOGMASK & RV_LOGLEVEL_ERROR)
#define RvSema4LogError(p) {if (logMgr != NULL) RvLogError(&logMgr->sema4Source, p);}
#else
#define RvSema4LogError(p) {RV_UNUSED_ARG(logMgr);}
#endif

#if (RV_LOGMASK & RV_LOGLEVEL_DEBUG)
#define RvSema4LogDebug(p) {if (logMgr != NULL) RvLogDebug(&logMgr->sema4Source, p);}
#else
#define RvSema4LogDebug(p) {RV_UNUSED_ARG(logMgr);}
#endif



RvStatus RvAdSema4Construct(
    OUT RvSemaphore*    sema,
    IN  RvUint32        startcount,
    IN  RvLogMgr*       logMgr)
{
    if (pthread_mutex_init(&sema->mutex, NULL) != 0)
    {
        RvSema4LogError((&logMgr->sema4Source, "RvAdSema4Construct: pthread_mutex_init failed with %d", errno));
        return RV_ERROR_UNKNOWN;
    }

    if (pthread_cond_init(&sema->cond_var, NULL) != 0)
    {
        RvSema4LogError((&logMgr->sema4Source, "RvAdSema4Construct: pthread_cond_init failed with %d", errno));
        pthread_mutex_destroy(&sema->mutex);
        return RV_ERROR_UNKNOWN;
    }

    sema->sem_cnt = startcount;
    sema->waiters = 0;

    return RV_OK;
}


RvStatus RvAdSema4Destruct(
    IN  RvSemaphore*    sema,
    IN  RvLogMgr*       logMgr)
{
    RV_UNUSED_ARG(logMgr);
    pthread_mutex_destroy(&sema->mutex);
    pthread_cond_destroy(&sema->cond_var);
    return RV_OK;
}


RvStatus RvAdSema4Post(
    IN  RvSemaphore*    sema,
    IN  RvLogMgr*       logMgr)
{
    RvStatus rv = RV_OK;
    if (pthread_mutex_lock(&sema->mutex) != 0)
    {
        RvSema4LogError((&logMgr->sema4Source, "RvAdSema4Post: pthread_mutex_lock failed with %d", errno));
        return RV_ERROR_UNKNOWN;
    }

    if (sema->waiters)
    {
        RvSema4LogDebug((&logMgr->sema4Source, "RvAdSema4Post: there are %d waiters, signalling", sema->waiters));
        if (pthread_cond_signal(&sema->cond_var) != 0)
        {
            RvSema4LogError((&logMgr->sema4Source, "RvAdSema4Post: pthread_cond_signal failed with %d", errno));
            rv = RV_ERROR_UNKNOWN;
        }
    }

    sema->sem_cnt ++;

    if (pthread_mutex_unlock(&sema->mutex) != 0)
    {
        RvSema4LogError((&logMgr->sema4Source, "RvAdSema4Post: pthread_mutex_lock failed with %d", errno));
        return RV_ERROR_UNKNOWN;
    }

    return rv;
}


RvStatus RvAdSema4Wait2(
    IN  RvSemaphore*    sema,
    IN  RvBool          bIsTry,
    IN  RvLogMgr*       logMgr)
{
    int ret;


    if (pthread_mutex_lock(&sema->mutex) != 0)
    {
        RvSema4LogError((&logMgr->sema4Source, "RvAdSema4Post: RvAdSema4Wait2 failed with %d", errno));
        return RV_ERROR_UNKNOWN;
    }

    while (sema->sem_cnt == 0)
    {
        if (bIsTry)
        {
            if (pthread_mutex_unlock(&sema->mutex) != 0)
            {
                RvSema4LogError((&logMgr->sema4Source, "RvAdSema4Wait2: pthread_mutex_unlock failed with %d", errno));
                return RV_ERROR_UNKNOWN;
            }
            return RV_ERROR_TRY_AGAIN;
        }
        else {
            sema->waiters ++;
            ret = pthread_cond_wait(&sema->cond_var,&sema->mutex);
            sema->waiters --;
            if (ret != 0)
            {
                RvSema4LogError((&logMgr->sema4Source, "RvAdSema4Wait2: pthread_cond_wait failed with %d", errno));
                return  RV_ERROR_UNKNOWN;
            }
        }
    }

    sema->sem_cnt --;

    if (pthread_mutex_unlock(&sema->mutex) != 0)
    {
        RvSema4LogError((&logMgr->sema4Source, "RvAdSema4Post: RvAdSema4Wait2 failed with %d", errno));
        return RV_ERROR_UNKNOWN;
    }

    return RV_OK;
}

RvStatus RvAdSema4Wait(
    IN  RvSemaphore*    sema,
    IN  RvLogMgr*       logMgr)
{
    return RvAdSema4Wait2(sema,RV_FALSE,logMgr);
}


RvStatus RvAdSema4TryWait(
    IN  RvSemaphore*    sema,
    IN  RvLogMgr*       logMgr)
{
    return RvAdSema4Wait2(sema,RV_TRUE,logMgr);
}


RvStatus RvAdSema4SetAttr(
    IN  RvSemaphoreAttr*attr,
    IN  RvLogMgr*       logMgr)
{
    RV_USE_CCORE_GLOBALS;
    RV_UNUSED_ARG(logMgr);

    RvDefaultSemaphoreAttr = *attr;

    return RV_OK;
}

#endif /* RV_SEMAPHORE_TYPE == RV_SEMAPHORE_CONDVAR */
