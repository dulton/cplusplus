#ifndef _RvCond_h
#define _RvCond_h

#include "rvsemaphore.h"
#include "rvlock.h"
#include "rvmutex.h"
#include "rvlog.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RvCond {
    RvLock lock;
    RvSemaphore sema;
    RvInt callCount;
} RvCond;

RVCOREAPI
RvStatus RVCALLCONV RvCondConstruct(RvCond *self, RvLogMgr *log);

RVCOREAPI
RvStatus RVCALLCONV RvCondDestruct(RvCond *self, RvLogMgr *logMgr);

RVCOREAPI
RvStatus RVCALLCONV RvCondWaitL(RvCond *self, RvLock *lock, RvLogMgr *log);

RVCOREAPI 
RvStatus RVCALLCONV RvCondWaitM(RvCond *self, RvMutex *mutex, RvLogMgr *log);

RVCOREAPI 
RvStatus RVCALLCONV RvCondBroadcast(RvCond *self, RvLogMgr *log);

#define RV_COND_WAITL(cond, condvar, lock, logMgr) \
    while(!(cond)) { \
        RvCondWaitL(condvar, lock, logMgr); \
    }; 

#if (RV_MUTEX_TYPE != RV_MUTEX_NONE)

#define RV_COND_WAITM(cond, condvar, lock, logMgr) \
    while(!(cond)) { \
        RvCondWaitM(condvar, lock, logMgr); \
    }; 

#endif /*#if (RV_MUTEX_TYPE != RV_MUTEX_NONE)*/


#ifdef __cplusplus
}
#endif
#endif
