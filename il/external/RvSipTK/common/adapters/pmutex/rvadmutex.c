/* rvadmutex.c - POSIX adapter recursive mutex functions */
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
#include "rvadmutex.h"
#include "rvlog.h"
#include "rvccoreglobals.h"
#if (RV_MUTEX_TYPE == RV_MUTEX_SOLARIS)
#include <errno.h>
#endif


#if (RV_MUTEX_TYPE != RV_MUTEX_NONE)

#if (RV_MUTEX_TYPE == RV_MUTEX_SOLARIS)
/* define the default attributes since they may be a structure */
/* static RvMutexAttr RvDefaultMutexAttr = RV_MUTEX_ATTRIBUTE_DEFAULT; */

/* See if we have a Solaris that can set a mutex's protocol or not */
static RvBool RvMutexTrySettingProtocol = RV_TRUE;
#endif


RvStatus RvAdMutexConstruct(
    OUT RvMutex*        mu,
    IN  RvLogMgr*       logMgr)
{
    RV_USE_CCORE_GLOBALS;
    pthread_mutexattr_t ma;

    RV_UNUSED_ARG(logMgr);

    if(pthread_mutexattr_init(&ma) != 0)
        return RV_ERROR_UNKNOWN;

#if (RV_MUTEX_TYPE == RV_MUTEX_SOLARIS)
    /* Should allow these along with _setprioceiling and */
    /* _setrobust_np to be set from config file. */
    if (RvMutexTrySettingProtocol)
    {
        int result;

        result = pthread_mutexattr_setprotocol(&ma, RvDefaultMutexAttr.protocol);
        if(result != 0)
        {
            if (result == ENOSYS)
                RvMutexTrySettingProtocol = RV_FALSE; /* Not supported - don't try anymore */
            else
                return RV_ERROR_UNKNOWN;
        }
    }
    if(pthread_mutexattr_setpshared(&ma, RvDefaultMutexAttr.pshared) != 0)
        return RV_ERROR_UNKNOWN;
#else
    RV_UNUSE_GLOBALS;
#endif

    if(pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE) != 0)
        return RV_ERROR_UNKNOWN;

    if(pthread_mutex_init(&(mu->mtx), &ma) != 0)
        return RV_ERROR_UNKNOWN;
    if(pthread_mutexattr_destroy(&ma) != 0)
        return RV_ERROR_UNKNOWN;

    mu->count = 0;

    return RV_OK;
}


RvStatus RvAdMutexDestruct(
    IN  RvMutex*        mu,
    IN  RvLogMgr*       logMgr)
{
    RV_UNUSED_ARG(logMgr);

    if(pthread_mutex_destroy(&mu->mtx) != 0)
        return RV_ERROR_UNKNOWN;

    return RV_OK;
}


RvStatus RvAdMutexLock(
    IN  RvMutex*        mu,
    IN  RvLogMgr*       logMgr)
{
    RV_UNUSED_ARG(logMgr);

    if(pthread_mutex_lock(&mu->mtx) != 0)
        return RV_ERROR_UNKNOWN;

    mu->count++;

    return RV_OK;
}


RvStatus RvAdMutexUnlock(
    IN  RvMutex*        mu,
    IN  RvLogMgr*       logMgr)
{
    RV_UNUSED_ARG(logMgr);

    mu->count--;

    if(pthread_mutex_unlock(&mu->mtx) != 0)
        return RV_ERROR_UNKNOWN;

    return RV_OK;
}


RvStatus RvAdMutexSetAttr(
    IN  RvMutexAttr*    attr,
    IN  RvLogMgr*       logMgr)
{
    RV_USE_CCORE_GLOBALS;
    RV_UNUSED_ARG(logMgr);

#if (RV_MUTEX_TYPE == RV_MUTEX_SOLARIS)
    RvDefaultMutexAttr = *attr;
#else
    RV_UNUSE_GLOBALS;
    RV_UNUSED_ARG(attr);
#endif
    
    return RV_OK;
}

#endif /* RV_MUTEX_TYPE != RV_MUTEX_NONE */
