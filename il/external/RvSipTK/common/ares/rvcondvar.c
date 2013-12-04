#include "rvcondvar.h"

RVCOREAPI
RvStatus RVCALLCONV RvCondConstruct(RvCond *self, RvLogMgr *logMgr) {
    RvStatus s = RV_OK;

    s = RvLockConstruct(logMgr, &self->lock);
    if(s != RV_OK) {
        return s;
    }

    s = RvSemaphoreConstruct(0, logMgr, &self->sema);
    if(s != RV_OK) {
        RvLockDestruct(&self->lock, logMgr);
        return s;
    }

    self->callCount = 0;
    return s;
}

RVCOREAPI
RvStatus RVCALLCONV RvCondWaitL(RvCond *self, RvLock *lock, RvLogMgr *logMgr) {
    RvStatus s = RV_OK;

    RvLockGet(&self->lock, logMgr);
    self->callCount++;
    RvLockRelease(&self->lock, logMgr);

    RvLockRelease(lock, logMgr);
    RvSemaphoreWait(&self->sema, logMgr);
    RvLockGet(lock, logMgr);
    return s;
}

#if (RV_MUTEX_TYPE != RV_MUTEX_NONE)
RVCOREAPI
RvStatus RVCALLCONV RvCondWaitM(RvCond *self, RvMutex *mutex, RvLogMgr *logMgr) {
    RvStatus s = RV_OK;
    RvInt32 lockCnt = 0;

    RvLockGet(&self->lock, logMgr);
    self->callCount++;
    RvLockRelease(&self->lock, logMgr);

    RvMutexGetLockCounter(mutex, logMgr, &lockCnt);
    RvMutexRelease(mutex, logMgr, &lockCnt);
    RvSemaphoreWait(&self->sema, logMgr);
    RvMutexRestore(mutex, logMgr, lockCnt);
    return s;
}
#endif /*#if (RV_MUTEX_TYPE != RV_MUTEX_NONE)*/

RVCOREAPI 
RvStatus RVCALLCONV RvCondBroadcast(RvCond *self, RvLogMgr *logMgr) {
    RvStatus s = RV_OK;
    

    RvLockGet(&self->lock, logMgr);

    while(self->callCount) {
        RvSemaphorePost(&self->sema, logMgr);
        self->callCount--;
    }

    RvLockRelease(&self->lock, logMgr);
    return s;
}

RVCOREAPI
RvStatus RVCALLCONV RvCondDestruct(RvCond *self, RvLogMgr *logMgr) {
    RvLockDestruct(&self->lock, logMgr);
    RvSemaphoreDestruct(&self->sema, logMgr);
    return RV_OK;
}
