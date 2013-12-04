/* rvadlock.c - POSIX adapter lock functions */
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
#include "rvadlock.h"
#include "rvlog.h"
#include "rvccoreglobals.h"
#if (RV_LOCK_TYPE == RV_LOCK_SOLARIS)
#include <errno.h>
#endif


#if (RV_LOCK_TYPE != RV_LOCK_NONE)

/* define the default attributes since they may be a structure */
/* static RvLockAttr RvDefaultLockAttr = RV_LOCK_ATTRIBUTE_DEFAULT; */

#if (RV_LOCK_TYPE == RV_LOCK_SOLARIS)
/* See if we have a Solaris that can set a mutex's protocol or not */
static RvBool RvLockTrySettingProtocol = RV_TRUE;
#endif


RvStatus RvAdLockConstruct(
    OUT RvLock*         lock,
    IN  RvLogMgr*       logMgr)
{
    RV_USE_CCORE_GLOBALS;
    pthread_mutexattr_t ma;

    RV_UNUSED_ARG(logMgr);
    
    if(pthread_mutexattr_init(&ma) != 0)
        return RV_ERROR_UNKNOWN;

#if (RV_LOCK_TYPE == RV_LOCK_SOLARIS)
    /* Should allow these along with _setprioceiling and */
    /* _setrobust_np to be set from config file. */
    if (RvLockTrySettingProtocol)
    {
        int result;
        
        result = pthread_mutexattr_setprotocol(&ma, RvDefaultLockAttr.protocol);
        if(result != 0)
        {
            if (result == ENOSYS)
                RvLockTrySettingProtocol = RV_FALSE; /* Not supported - don't try anymore */
            else
                return RV_ERROR_UNKNOWN;
        }
    }
    if(pthread_mutexattr_setpshared(&ma, RvDefaultLockAttr.pshared) != 0)
        return RV_ERROR_UNKNOWN;
#endif

    if(pthread_mutexattr_settype(&ma, RvDefaultLockAttr.kind) != 0)
        return RV_ERROR_UNKNOWN;
    
    if(pthread_mutex_init(lock, &ma) != 0)
        return RV_ERROR_UNKNOWN;
    if(pthread_mutexattr_destroy(&ma) != 0)
        return RV_ERROR_UNKNOWN;

    return RV_OK;
}


RvStatus RvAdLockDestruct(
    IN  RvLock*         lock,
    IN  RvLogMgr*       logMgr)
{
    RV_UNUSED_ARG(logMgr);

    if(pthread_mutex_destroy(lock) != 0)
        return RV_ERROR_UNKNOWN;

    return RV_OK;
}


RvStatus RvAdLockGet(
    IN  RvLock*         lock,
    IN  RvLogMgr*       logMgr)
{
    RV_UNUSED_ARG(logMgr);

    if(pthread_mutex_lock(lock) != 0)
        return RV_ERROR_UNKNOWN;

    return RV_OK;
}


RvStatus RvAdLockRelease(
    IN  RvLock*         lock,
    IN  RvLogMgr*       logMgr)
{
    RV_UNUSED_ARG(logMgr);

    if(pthread_mutex_unlock(lock) != 0)
        return RV_ERROR_UNKNOWN;

    return RV_OK;
}


RvStatus RvAdLockSetAttr(
    IN  RvLockAttr*     attr,
    IN  RvLogMgr*       logMgr)
{
    RV_USE_CCORE_GLOBALS;
    RV_UNUSED_ARG(logMgr);

    RvDefaultLockAttr = *attr;

    return RV_OK;
}

#endif /* RV_LOCK_TYPE != RV_LOCK_NONE */
