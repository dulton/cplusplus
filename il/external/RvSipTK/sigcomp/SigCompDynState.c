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
 *                              <RvSigCompDynState.c>
 *
 * The SigComp functions of the RADVISION SIP stack enable you to
 * create and manage SigComp Compartment objects in dynamic compression manner, 
 * attach/detach to/from compartments and control the compartment parameters.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Sasha G					    July 2006
*********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RvSigComp.h"
#include "SigCompMgrObject.h"
#include "RvSigCompDyn.h"
#include "SigCompDynZTime.h"
#include "rvmemory.h"
#include "rvassert.h"
#include "rvlog.h"
#include <stdio.h>

#define RV_SIGCOMP_DYN_DEBUG 1

/*-----------------------------------------------------------------------*/
/*                          COMMON DEFINITIONS                           */
/*-----------------------------------------------------------------------*/
#define MAX_STATE_PRIO 65534


#define rvZAlgoName(self) ((self)->vt.algoName)

#define rvZStreamAlloc(self, pps) \
    (self)->vt.pfnZsalloc((self)->vt.ctx, pps)

#define rvZStreamFree(self, ps) \
    (self)->vt.pfnZsfree((self)->vt.ctx, ps)

#define rvZStreamEnd(self, ps) \
    (self)->vt.pfnZsend((self)->vt.ctx, ps)



#define rvZStreamCompress(self, newStream, baseStream, plain, plainSize, compressed, pCompressedSize, sid) \
    (self)->vt.pfnZscompress((self)->vt.ctx, newStream, baseStream, plain, plainSize, compressed, pCompressedSize, sid)

#define rvZStreamGetBytecode(self) ((self)->vt.bytecode)
#define rvZStreamGetBytecodeSize(self) ((self)->vt.bytecodeSize)
#define rvZStreamGetBytecodeStart(self) ((self)->vt.codeStart)
#define rvZStreamGetStateSize(self) ((self)->vt.stateSize)
#define rvZStreamGetMinAccessLen(self) ((self)->vt.minAccessLen)
/* Returns returned parameters location. 
 *  It's assumed that this is the last field used by our decompressors,
 */
#define rvZStreamGetReturnedParamsLocation(self) ((self)->vt.returnedParamsLocation)

#define rvZStreamGetLocalStateSize(self, ps) \
    ((self)->vt.localStateSize)

#define LOGSRC ((SigCompMgr*)(self->hSigCompMgr))->pLogSource
#define CURTIME self->curTimeSec
#define COMPARTMENT self
#define FID(fid) (fid)->prio, (fid)->zid
#define FIDZ(z) FID(&(z)->fid)

#define DYN_LOG_DEBUG0(format) RvLogDebug(LOGSRC, (LOGSRC, "[T:%d C:%p S:%s] "   format, CURTIME, COMPARTMENT, STAGE))
#define DYN_LOG_DEBUG1(format, arg) RvLogDebug(LOGSRC, (LOGSRC, "[T:%d C:%p S:%s] " format, CURTIME, COMPARTMENT, STAGE,  arg))
#define DYN_LOG_DEBUG2(format, arg1, arg2) RvLogDebug(LOGSRC, (LOGSRC, "[T:%d C:%p S:%s] " format, CURTIME, COMPARTMENT, STAGE,  arg1, arg2))
#define DYN_LOG_DEBUG3(format, arg1, arg2, arg3) RvLogDebug(LOGSRC, (LOGSRC, "[T:%d C:%p S:%s] " format, CURTIME, COMPARTMENT, STAGE,  arg1, arg2, arg3))
#define DYN_LOG_DEBUG4(format, arg1, arg2, arg3, arg4) RvLogDebug(LOGSRC, (LOGSRC, "[T:%d C:%p S:%s] " format, CURTIME, COMPARTMENT, STAGE,  arg1, arg2, arg3, arg4))
#define DYN_LOG_DEBUG5(format, arg1, arg2, arg3, arg4, arg5) RvLogDebug(LOGSRC, (LOGSRC, "[T:%d C:%p S:%s] " format, CURTIME, COMPARTMENT, STAGE,  arg1, arg2, arg3, arg4, arg5))
#define DYN_LOG_DEBUG6(format, arg1, arg2, arg3, arg4, arg5, arg6) RvLogDebug(LOGSRC, (LOGSRC, "[T:%d C:%p S:%s] " format, CURTIME, COMPARTMENT, STAGE,  arg1, arg2, arg3, arg4, arg5, arg6))

#define DYN_LOG_ERROR0(format) RvLogError(LOGSRC, (LOGSRC, "[T:%d C:%p S:%s] "   format, CURTIME, COMPARTMENT, STAGE))
#define DYN_LOG_ERROR1(format, arg) RvLogError(LOGSRC, (LOGSRC, "[T:%d C:%p S:%s] " format, CURTIME, COMPARTMENT, STAGE,  arg))
#define DYN_LOG_ERROR2(format, arg1, arg2) RvLogError(LOGSRC, (LOGSRC, "[T:%d C:%p S:%s] " format, CURTIME, COMPARTMENT, STAGE,  arg1, arg2))
#define DYN_LOG_ERROR3(format, arg1, arg2, arg3) RvLogError(LOGSRC, (LOGSRC, "[T:%d C:%p S:%s] " format, CURTIME, COMPARTMENT, STAGE,  arg1, arg2, arg3))
#define DYN_LOG_ERROR4(format, arg1, arg2, arg3, arg4) RvLogError(LOGSRC, (LOGSRC, "[T:%d C:%p S:%s] " format, CURTIME, COMPARTMENT, STAGE,  arg1, arg2, arg3, arg4))
#define DYN_LOG_ERROR5(format, arg1, arg2, arg3, arg4, arg5) RvLogError(LOGSRC, (LOGSRC, "[T:%d C:%p S:%s] " format, CURTIME, COMPARTMENT, STAGE,  arg1, arg2, arg3, arg4, arg5))
#define DYN_LOG_ERROR6(format, arg1, arg2, arg3, arg4, arg5, arg6) RvLogError(LOGSRC, (LOGSRC, "[T:%d C:%p S:%s] " format, CURTIME, COMPARTMENT, STAGE,  arg1, arg2, arg3, arg4, arg5, arg6))


#define MAX_CACHED_Z 5


/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

#if RV_SIGCOMP_DYN_DEBUG
static 
void SigCompDynStateDump(RvSigCompDynState *self);
#else
#define SigCompDynStateDump(self)
#endif


static
void  SigCompDynStateDeleteZStream(RvSigCompDynState *self, RvSigCompDynZStream *zStream) {
    rvZStreamEnd(self, zStream); 
    rvZStreamFree(self, zStream);
    self->nCachedZ--;
}

static RvSigCompDynZ* SigCompDynStateFindStateFid(IN RvSigCompDynState *self, 
												  IN RvSigCompDynFid   *fid)  {
    RvSigCompDynZ   *cur  = self->states;
    RvSigCompDynZid  zid  = fid->zid;
    RvSigCompDynPrio prio = fid->prio;

    while(cur != 0) {
        if(ZID(cur) == zid && PRIO(cur) == prio) {
            return cur;
        }

        cur = cur->pZnext;
    }

    return 0;
}

static RvStatus SigCompDynStateZAlloc(IN  RvSigCompDynState *self,
									  OUT RvSigCompDynZ    **ppz) {
    RvSigCompDynZ *z = 0;
    RvStatus       s;

    s = RvMemoryAlloc(0, sizeof(**ppz), 0, (void **)ppz);
    if(s != RV_OK) {
        return s;
    }

    z = *ppz;
    memset(z, 0, sizeof(*z));

    self->nStates++;		
    return RV_OK;
}

#define WRITE16(addr, val) \
{\
    *(pMem + (addr)) = (RvUint8)((val) >> 8);\
    *(pMem + (addr) + 1) = (RvUint8)((val) & 0x00ff);\
}


static void SigCompDynStateComputeInitStateID(RvSigCompDynState *self,  RvSigCompStateID sid) {
    RvSigCompSHA1Context	ctx;      
    RvUint8					pMem[8];  
    RvUint8  *pBytecode = rvZStreamGetBytecode(self);
    RvUint16  bytecodeStart = rvZStreamGetBytecodeStart(self);
    RvSize_t  bytecodeSize = rvZStreamGetBytecodeSize(self);
    RvSize_t  minAccessLen = rvZStreamGetMinAccessLen(self);


    WRITE16(0, bytecodeSize);
    WRITE16(2, bytecodeStart);
    WRITE16(4, bytecodeStart);
    WRITE16(6, minAccessLen);

    RvSigCompSHA1Reset(&ctx);
    RvSigCompSHA1Input(&ctx, pMem, 8);
    RvSigCompSHA1Input(&ctx, pBytecode, (RvUint32)bytecodeSize);
    RvSigCompSHA1Result(&ctx, sid);
}

static RvStatus SigCompDynStateZInit(IN    RvSigCompDynState   *self, 
									 IN    RvSigCompDynZStream *initState, 
									 INOUT RvSigCompDynZ       *rvz) {
    RvStatus s = RV_OK;
    
    (void)self;
    
    memset(rvz, 0, sizeof(*rvz));
    rvz->includesBytecode   = RV_FALSE;
    rvz->persistent			= RV_TRUE;
    rvz->fid.prio			= 0;
    rvz->fid.wrapCnt        = 0;
    rvz->fid.zid			= 0;
    rvz->nRequests			= 0;
    rvz->createsState		= RV_TRUE;
    rvz->sendTime			= 0;
    rvz->ackedTime			= 0;
    rvz->sendTimeSec		= 0;
    rvz->pZnext				= 0;
    rvz->pZfirstDependent	= 0;
    rvz->pZnextDependent	= 0;
    rvz->acked				= RV_TRUE;
    rvz->pZ					= initState;    
    rvz->stateSize          = rvZStreamGetBytecodeSize(self) + 64;
    SigCompDynStateComputeInitStateID(self, rvz->sid);
    return s;
}


static
void SigCompDynZAddDependent(RvSigCompDynZ *self, RvSigCompDynZ *dependent) {
    dependent->dfid = self->fid;
    dependent->pZdad = self;
    dependent->pZnextDependent = self->pZfirstDependent;
    self->pZfirstDependent = dependent;
}


/* Removes dependent state */
static void SigCompDynZRemoveDependent(IN RvSigCompDynZ *dependent) {
    RvSigCompDynZ *dad  = dependent->pZdad;
    RvSigCompDynZ *prev = 0;
    RvSigCompDynZ *cur;

    if(dad == 0) {
        return;
    }

    for(cur = dad->pZfirstDependent;
		cur && cur != dependent; 
		prev = cur, cur = cur->pZnextDependent)
	;

    if(cur == 0) {
        return;
    }

    if(prev == 0) {
        dad->pZfirstDependent = cur->pZnextDependent;
    } else {
        prev->pZnextDependent = cur->pZnextDependent;
    }

    dependent->pZdad = 0;
}

static void SigCompDynZRemoveDependency(IN RvSigCompDynZ *dependency) {
    RvSigCompDynZ *cur;

    for(cur = dependency->pZfirstDependent; cur; cur = cur->pZnextDependent) {
        cur->pZdad = 0;
    }
}

static 
void SigCompDynFreePlain(RvSigCompDynState *self, RvUint8 *plain, RvSize_t plainSize) {
    (void)self;
    RvMemoryFree(plain, 0);
    self->totalSize -= plainSize;
}


static void SigCompDynStateZDelete(IN RvSigCompDynState *self, OUT RvSigCompDynZ *z) {
#define STAGE "DEL"
    DYN_LOG_DEBUG1("   Deleting local state <%d,%d>", FID(&z->fid));
    SigCompDynZRemoveDependent(z);
    SigCompDynZRemoveDependency(z);

    if(z->pZ) {
        SigCompDynStateDeleteZStream(self, z->pZ);
    }

    if(z->plain) {
        SigCompDynFreePlain(self, z->plain, z->plainSize);
    }

    RvMemoryFree(z, 0);
    self->nStates--;
#undef STAGE
}


/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/*************************************************************************
* SigCompDynStateConstruct
* ------------------------------------------------------------------------
* General: This function construct a dynamic compression state
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr - SigComp manager handle 
*		 pSelf       - Pointer to the dynamic state
*        initStream  - Pointer to the stream to be related to the dynamic
*					   state.
*************************************************************************/
RvStatus RVCALLCONV SigCompDynStateConstruct(
										IN RvSigCompDynState   *self, 
										IN RvSigCompDynZStream *initStream,
										IN RvSigCompMgrHandle   hSigCompMgr)
{ 
#define STAGE "CONSTR"
	SigCompMgr	  *pSigCompMgr = (SigCompMgr *)hSigCompMgr;
    RvSigCompDynZ *initState; 
    RvStatus       s;    

    self->hSigCompMgr     = hSigCompMgr;

    
    DYN_LOG_DEBUG1("Constructing dynamic compressor using %s algorithm", rvZAlgoName(self));

    s = RvMemoryAlloc(0, sizeof(*initState), 0, (void **)&initState);
    if(s != RV_OK) {
        DYN_LOG_ERROR0("Failed to allocate initState object");
        return s;
    }

    s = SigCompDynStateZInit(self, initStream, initState);

    if(s != RV_OK) {
        DYN_LOG_ERROR0("Failed to initialize initState object");
        RvMemoryFree(initState, 0);
        return s;
    }

    self->reliableTransport = RV_FALSE;
    self->maxTripTime = pSigCompMgr->dynMaxTripTime;
    self->activeState = self->initState = initState;
    self->peerInfoKnown   = RV_FALSE; 
    self->curTime         = 0;
    self->curTimeSec      = 0;
    self->lastPeerMessageTime = 0;
    self->lastPeerMessageNum = 0;
    self->states          = 0;
    self->peerCompSize    = RVSIGCOMP_DEFAULT_SMS - initState->stateSize;
    self->curZid          = 1;
    self->curPlain        = 0;
    self->curCompressed   = 0;
#ifdef RV_SIGCOMP_STATISTICS
    self->curRatio        = 0;
#endif /*#ifdef RV_SIGCOMP_STATISTICS*/
    self->nStates         = 0;
    self->peekStates      = 0;
    self->maxStates       = pSigCompMgr->dynMaxStatesNo;

    self->totalSize       = 0; 
    self->peekTotalSize   = 0;
    /* Half of maxTotalStatesSize is for saving plain text and 
     * another half is for saving whole zstreams 
     */
    self->maxTotalSize    = pSigCompMgr->dynMaxTotalStatesSize / 2;
    self->maxCachedZ      = 1 + (RvInt)(self->maxTotalSize / rvZStreamGetLocalStateSize(self, 0));
    self->nCachedZ        = 1;
    
    return RV_OK;
#undef STAGE
}

/*************************************************************************
* SigCompDynStateDestruct
* ------------------------------------------------------------------------
* General: This function destruct a dynamic compression state
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: pSelf       - Pointer to the destructed dynamic state
*************************************************************************/
RvStatus SigCompDynStateDestruct(IN RvSigCompDynState *self) 
{
    RvSigCompDynZ *next = 0;
    RvSigCompDynZ *cur = 0;

    SigCompDynStateDump(self);
    if(self->initState->pZ) {
        SigCompDynStateDeleteZStream(self, self->initState->pZ);
    }
    RvMemoryFree(self->initState, 0);

    for(cur = self->states; cur; cur = next) {
        next = cur->pZnext;
        if(cur->pZ) {
            SigCompDynStateDeleteZStream(self, cur->pZ);
            cur->pZ = 0;
        }
        if(cur->plain) {
            SigCompDynFreePlain(self, cur->plain, cur->plainSize);
            cur->plain = 0;
        }

        RvMemoryFree(cur, 0);

    }

    return RV_OK;
}

/*************************************************************************
* SigCompDynStateCompareFid
* ------------------------------------------------------------------------
* General: Compares between 2 fids.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: fid1  - Pointer to the first fid
*		 fid2  - Pointer to the second fid
*************************************************************************/
static RvInt SigCompDynStateCompareFid(IN RvSigCompDynFid *fid1, 
									   IN RvSigCompDynFid *fid2) {
    
    RvInt res = fid1->wrapCnt - fid2->wrapCnt;

    
    if(res) {
        return res;
    }

    res = fid1->prio - fid2->prio;
    if(res) {
        return res;
    }

    res = fid1->zid - fid2->zid;
    return res;
	
}

static 
RvBool SigCompDynZBetter(RvSigCompDynZ *z1,
                                    RvSigCompDynZ *z2) {

    RvSigCompDynFid *fid1 = &z1->fid;
    RvSigCompDynFid *fid2 = &z2->fid;

    return (fid1->wrapCnt > fid2->wrapCnt) || 
        (fid1->wrapCnt == fid2->wrapCnt && fid1->prio > fid2->prio);
} 

/*
 * Delete states given by array of FIDs (rfids)
 */
static
void SigCompDynStateDeleteRemoved(RvSigCompDynState *self, RvSigCompDynFid *rfids, RvSize_t nfids) {
#define STAGE "DELREM"
    RvSigCompDynZ *cur;
    RvSigCompDynZ *prev = 0;
    RvSize_t i;
    RvSigCompDynFid *curfid;


    DYN_LOG_DEBUG1("Start deleting %d local states", nfids);

    for(i = 0, curfid = rfids; i < nfids; i++, curfid++) {
        prev = 0;
        for(cur = self->states; cur; cur = cur->pZnext) {
            if(SigCompDynStateCompareFid(&cur->fid, curfid) == 0) {
                break;
            }

            prev = cur;
        }

        if(cur == 0) {
            continue;
        }

        if(prev == 0) {
            self->states = cur->pZnext;
        } else {
            prev->pZnext = cur->pZnext;
        }

        SigCompDynStateZDelete(self, cur);
    }

    DYN_LOG_DEBUG0("End deleting local states");

#undef STAGE
}


static 
void SigCompDynStateDeleteByPtr(RvSigCompDynState *self, RvSigCompDynZ *z) {
    RvSigCompDynZ *cur;
    RvSigCompDynZ *prev = 0;

    for(cur = self->states; cur && cur != z; prev = cur, cur = cur->pZnext)
    ;
    
    if(cur == 0) {
        return;
    }

    if(prev == 0) {
        self->states = z->pZnext;
    } else {
        prev->pZnext = z->pZnext;
    }

    SigCompDynStateZDelete(self, z);
}

/*************************************************************************
* SigCompDynStateAckState
* ------------------------------------------------------------------------
* General: Acknowledge state
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:  self      - points to deflate instance
*		  fid	    - full state id
* Output:
*         pIsUsable - whether acknowledged state may be used as a basis 
*				      of compression
*************************************************************************/
static RvSigCompDynZ* SigCompDynStateAckState(IN RvSigCompDynState *self, 
									IN RvSigCompDynFid   *fid,
									IN RvBool			 *pIsUsable) 
{ 
#define STAGE "ACK"

    RvSigCompDynZ *cur;
    RvSigCompZTime    sendTime;
    RvSigCompZTimeSec sendTimeSec;
    RvSigCompZTimeSec minArriveTime;
    RvSigCompDynPrio  prio = fid->prio;
    RvSigCompDynZ    *ackedState = 0;
    RvSigCompDynFid  *removedFids = 0;
    RvSize_t  nRemoved = 0;
    RvSize_t  highPrioDemands = 0;
    RvSize_t  lowPrioDemands = 0;
    RvSize_t  compAlloced = 0;
    RvBool    bConsider = RV_FALSE;
    RvSigCompZTimeSec  tripTime;

    DYN_LOG_DEBUG1("State <%d,%d> acked", FID(fid));

    *pIsUsable = RV_FALSE;
    ackedState = SigCompDynStateFindStateFid(self, fid);


    /* Acked state wasn't found, probably already removed, or already acknowledged - nothing to do,
     * just return.
     */
    if(ackedState == 0) {
        *pIsUsable = 0;
        DYN_LOG_DEBUG1("State <%d,%d> wasn't found, probably removed", FID(fid));
        return 0;
    }

    minArriveTime = sendTimeSec = ackedState->sendTimeSec;
    tripTime = ((self->curTimeSec - sendTimeSec + 1) >> 1);

    if(tripTime > self->maxTripTime) {
        DYN_LOG_DEBUG2("Estimation on trip time is too optimistic (%d), changing to %d", 
            self->maxTripTime, tripTime);
        self->maxTripTime = tripTime;
    }

    if(ackedState->nDeleteCount > 0) {
        /* We already asked our peer to delete this state. Probably, trip time isn't set properly
         * 
         */
        DYN_LOG_DEBUG1("Already deleted state was acked <%d,%d>, probably due to maxTripTime too low", FID(fid));
        *pIsUsable = 0;
        return 0;
    }

    /* Newly acknowledged state, do some bookkeeping */
    if(!ackedState->acked) {
        ackedState->acked = RV_TRUE;

        ackedState->ackedTime = self->curTime;
        ackedState->ackedTimeSec = self->curTimeSec;


        removedFids = &ackedState->removeRequests[0];
        nRemoved = ackedState->nRequests;

        SigCompDynStateDeleteRemoved(self, removedFids, nRemoved);

        if(!ackedState->createsState){
            /* Doesn't create state, we can safely delete it - it will not take part in 
             * any further processing
             */
            DYN_LOG_DEBUG1("Delete <%d,%d> as acked, not creating state", FID(fid));
            SigCompDynStateDeleteByPtr(self, ackedState);
            return 0;
        } 

        /* Remove ackedState from the list of unacknowledged
        * dependent of it's dad
        */
        SigCompDynZRemoveDependent(ackedState);
    }

    bConsider = SigCompDynZBetter(ackedState, self->activeState);
    if(!bConsider) {
        DYN_LOG_DEBUG1("Currently active state <%d,%d> is better, nothing to analyze", FID(fid));
        *pIsUsable = RV_FALSE;
        return ackedState;
    }

    sendTime = ackedState->sendTime;

    DYN_LOG_DEBUG2("Analyzing <%d,%d> sent on %d", FID(fid), sendTimeSec);


    /* 
     * For acknowledged state 'a'
     *
     * 1. We may safely to take out of consideration any ascendant of state 'a'.
     *    The reason is that ascendant is necessary for successful decompression of 'a'.
     *    'a' was acked, it means that any ascendant of 'a' arrived to parent compartment before 'a'.
     *    Our algorithm always assigns to descendants higher priority, e.g. any ascendant will be pushed
     *    out of compartment before 'a'.
     *    
     * 2. We may safely to take out of consideration each state 'e' such that PRIO(e) <= PRIO(a)
     *    and 'e' was decompressed before 'a'. In such case 'e' will be pushed out of compartment before 'a'
     *
     *
     * 3. We may safely to take out of consideration every state 'e' that was acknowledged 
     *    before state 'a' was sent to the peer and such that prio(e) <= prio(a)
     *    
     *    The reason:
     *    If state 'e' was acknowledged before state 'a' was sent then state 'a' arrived to
     *    the peer later than 'e'. In this case if prio(e) <= prio(a), then state 'e' will be 
     *    pushed out of compartment before state 'a'.
     *
     * 4. We ask to remove state at the peer in 2 cases:
     *     1. State was acknowledged.
     *     2. State wasn't acknowledged, and it's old state. 
     *        State 'o' is considered as old if we receive message from peer 2*TT seconds after
     *        message 'o' was sent. In this case we positively sure that state already got to peer
     *        or it was lost. 
     *    In both cases we may take this state out of consideration
     *
     *    So, now, we can safely to take out of consideration every state 'b' that 'a' asked to remove 
     *    from peer compartment. 
     *
     * 5. If there is some state 'd' that depends on some deleted state 'b' and prio(d) <= prio(a) it may
     *    be safely took out of consideration also. 
     *    Let's see why:
     *      There are 3 cases:
     *       1. State 'd' was already saved at peer when 'a' arrived. In this case, because 
     *          prio(d) <= prio(a) - it will be pushed out of compartment before 'a'
     *
     *       2. State 'd' is somehow delayed and will arrive to the peer only after 'a' arrived,
     *          but, because 'a' removed the state that 'd' depends on, decompression of 'd' will 
     *          fail.
     *
     *       3. 'd' was lost. 
     *
     */


    /* Traverse all states and mark each state 's' that doesn't affect availability
     * of 'a' at the peer
     */ 

    for(cur = self->states; cur; cur = cur->pZnext) {
        int i;
        RvSigCompDynFid *rfid;
        RvBool affects = RV_FALSE;

        /* Rule 1 - mark ascendant */
        if(!affects && cur->servedAsActive && PRIO(cur) < prio) {
            DYN_LOG_DEBUG1("  Rule1: Marking <%d,%d> as ascendant", FIDZ(cur));
            affects = RV_TRUE;
        }

        /* Rule 2 */
        if(!affects && (PRIO(cur) <= prio) && (minArriveTime > cur->sendTimeSec + self->maxTripTime)) {
            /* Acked message arrived to peer not before minArriveTime,
             * 'cur' message arrived to peer not after cur->sendTimeSec + RV_SIGCOMP_MAX_TRIP_TIME
             */
            DYN_LOG_DEBUG2("  Rule2: Marking <%d,%d> as old (send time: %d) with not greater prio", FIDZ(cur), cur->sendTimeSec);
            affects = RV_TRUE;
        } 

        /* Rule 3 */
        if(!affects && cur->acked && (cur->ackedTime <= sendTime) && PRIO(cur) <= prio) {
            DYN_LOG_DEBUG2(" Rule 3: Marking <%d,%d> acked on %d as acked-old", FIDZ(cur), cur->ackedTimeSec); 
            affects = RV_TRUE;
        } 

        for(i = (int)nRemoved, rfid = removedFids; i-- && rfid != NULL; rfid++) {
            RvInt cres;

            cres = SigCompDynStateCompareFid(&cur->dfid, rfid);
            if(!affects && !cres && PRIO(cur) <= prio) {
                /* cur is dependent of some deleted state */
                DYN_LOG_DEBUG2(" Rule 5: Marking <%d,%d> as dependent on deleted state <%d,%d>", FIDZ(cur), FID(rfid));
                affects = RV_TRUE;
                break;
            }
        }

        if(!affects && cur->createsState) {
            if(PRIO(cur) >= fid->prio) {
                DYN_LOG_DEBUG1(" <%d,%d> affects high prio demands", FIDZ(cur));
                highPrioDemands += cur->stateSize;
            } else {
                DYN_LOG_DEBUG1(" <%d,%d> may affects low prio demands", FIDZ(cur));
                lowPrioDemands = RvMax(lowPrioDemands, cur->stateSize);
            }
        }
    }


    compAlloced = lowPrioDemands + highPrioDemands;
    *pIsUsable = ackedState->createsState && compAlloced <= self->peerCompSize;

    DYN_LOG_DEBUG6("<%d,%d> is %s (lowPrioDemands: %d, highPrioDemands: %d, totalDemands: %d, compsize: %d)", 
        FID(fid), (*pIsUsable ? "usable" : "not usable"), lowPrioDemands, highPrioDemands, compAlloced, self->peerCompSize);
    return ackedState;

#undef STAGE
}


/* Finds all saved states that will never serve as 'active' and free
 * state memory
 */
static
void SigCompDynClearUnneededStates(RvSigCompDynState *self) {
    RvSigCompDynZ *cur;

    RvSigCompDynFid *fid;

    fid = &self->activeState->fid;

    for(cur = self->states; cur; cur = cur->pZnext) {
        RvSigCompDynFid *curFid = &cur->fid;

        if(curFid->wrapCnt > fid->wrapCnt ||
            (curFid->wrapCnt == fid->wrapCnt && curFid->prio > fid->prio)) {
                break;
        }


        if(cur == self->activeState) {
            continue;
        }

        if(cur->plain) {
            SigCompDynFreePlain(self, cur->plain, cur->plainSize);
            cur->plain = 0;
        }

        if(cur->pZ) {
            SigCompDynStateDeleteZStream(self, cur->pZ);
            cur->pZ = 0;
        }
    }

}

static
RvStatus SigCompDynStateChangeActive(RvSigCompDynState *self, RvSigCompDynZ *newActive) {
    RvSigCompDynZStream *activeStream;
    RvStatus s = RV_OK;
    RvSigCompStateID sid;

    if(newActive->pZ) {
        RvSigCompDynZ *oldActive = self->activeState;
        SigCompDynStateDeleteZStream(self, oldActive->pZ);
        oldActive->pZ = 0;
        self->activeState = newActive;
        return RV_OK;
    }

    activeStream = self->activeState->pZ;
    self->activeState->pZ = 0;
    newActive->pZ = activeStream;
    self->activeState = newActive;
    s = rvZStreamCompress(self, activeStream, activeStream, newActive->plain, newActive->plainSize, 
        0, 0, sid);
    
    SigCompDynFreePlain(self, newActive->plain, newActive->plainSize);
    newActive->plain = 0;
    return s;
}

/*************************************************************************
* SigCompDynStateAnalyzeFeedback
* ------------------------------------------------------------------------
* General: Strategy before we get first acknowledge (e.g. before we know 
*		   how much space is available for saving states at peer: each 
*		   message is compressed relative to sip dictionary and sent with 
*		   priority 0 and sequential zid. 
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: pSelf       - Pointer to the dynamic state
*************************************************************************/
RvStatus RVCALLCONV SigCompDynStateAnalyzeFeedback(
						IN RvSigCompDynState        *self, 
						IN RvSigCompCompressionInfo *info)
{
    RvSigCompDynZ   *ackedState = 0;
    RvSigCompDynFid  fid;
    RvUint8			*pfeedback;
    RvBool			 isUsable;

#define STAGE "ACK"

    if(info->returnedFeedbackItemSize != 4) {
        DYN_LOG_DEBUG0("No valid feedback accepted");
        return RV_OK;
    }

    DYN_LOG_DEBUG0("Start feedback analysis");
    
    pfeedback = info->pReturnedFeedbackItem;
    fid.prio = (RvUint16)((pfeedback[0] << 8) + pfeedback[1]);
    fid.zid = (RvUint16)((pfeedback[2] << 8) + pfeedback[3]);
    
    if(!self->peerInfoKnown) {
        self->peerInfoKnown = RV_TRUE;
        self->peerCompSize = info->remoteSMS - self->initState->stateSize;
        self->peerInfoKnown = RV_TRUE;
        self->initState->includesBytecode = RV_TRUE;
        DYN_LOG_DEBUG1("Peer compartment size is %d", self->peerCompSize);
    }
    
    ackedState = SigCompDynStateAckState(self, &fid, &isUsable);

    if(isUsable && SigCompDynZBetter(ackedState, self->activeState)) {
        DYN_LOG_DEBUG2("Changing active state from <%d,%d> to <%d,%d>",
                       FIDZ(self->activeState), FIDZ(ackedState));
        SigCompDynStateChangeActive(self, ackedState);
        self->curZid	  = 1;
    } else {
        DYN_LOG_DEBUG1("Active state wasn't changed, still <%d,%d>", FIDZ(self->activeState));
    }

    SigCompDynClearUnneededStates(self);

    DYN_LOG_DEBUG0("End feedback analisys");

    return RV_OK;
    
#undef STAGE
}

static int SigCompDynLog2(RvUint n)
{
    RvInt  i  = 0;
    RvUint n1 = n;
	
    while(n1 >>= 1)
    {
        i++;
    }
	
    return i;
}


static RvInt SigCompDynGenerateReturnedParameters(RvSigCompDynState *self,
                                                  RvSigCompCompressionInfo *pCompressionInfo,
                                                  RvUint8 *pBuffer,
                                                  RvUint32 bufferSize) {
#define STAGE "GENRET"

	RvUint32 reqSize = 0;
	RvUint8  *pCur   = pBuffer;
	RvUint   msb;
	RvUint   lsb;

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARGS(self);
#endif
    if(!pCompressionInfo->bSendLocalCapabilities && !pCompressionInfo->bSendLocalStatesIdList)  {
        
        DYN_LOG_DEBUG0("No returned parameters were requested");
        if(bufferSize < 2)	{
            return -1;
        }
        
        *pCur++ = 0;
        *pCur = 0;
        return 2;
    }

	/* Compute requested size */
	
	reqSize = 2; /* capabilities byte + version bytes */
	
	if(pCompressionInfo->bSendLocalStatesIdList) 
	{
		reqSize += pCompressionInfo->localStatesSize; /* local states ids length */
	}

	/* report error if there is not enough space in the buffer   
	 * 2 additional bytes: first one for flags + msb of returned parameters
	 *   second - lsb of returned parameters
	 */

	if((reqSize + 2) > bufferSize)
	{
		return -1;
    } 

	msb = reqSize >> 8; /* msb */
	lsb = reqSize & 0xff; /* lsb */

	*pCur++ = (RvUint8)msb;
	*pCur++ = (RvUint8)lsb;

	if(!pCompressionInfo->bSendLocalCapabilities)
	{
        DYN_LOG_DEBUG0("Local capabilities weren't requested");
		*pCur++ = 0;
		*pCur++ = 0;
	} 
    else 
	{
		RvUint8 cpb = (RvUint8)(SigCompDynLog2(pCompressionInfo->localCPB) - 4); /* CPB should be at least 16   */
		RvUint8 dms = (RvUint8)(SigCompDynLog2(pCompressionInfo->localDMS) - 10);/* DMS should be at least 8192 */
		RvUint8 sms = (RvUint8)(SigCompDynLog2(pCompressionInfo->localSMS) - 10);


		if(dms <= 2)
		{
			return -1;
		}

		
		cpb = (RvUint8)RvMin(cpb, 3);
		dms = (RvUint8)RvMin(dms, 7);
		sms = (RvUint8)RvMin(sms, 7);

		*pCur++ = (RvUint8)((cpb << 6) | (dms << 3) | sms);

		*pCur++ = 1; /* SigComp Ver. 1 */
        DYN_LOG_DEBUG4("Generating local caps: cpb=%d, dms=%d, sms=%d, coded=%x", cpb, dms, sms, pCur[-2]);
	}

	if(pCompressionInfo->bSendLocalStatesIdList)
	{
        DYN_LOG_DEBUG0("Generating local state id list");
		memcpy(pCur, pCompressionInfo->pLocalStateIds, pCompressionInfo->localStatesSize);
	}

	return reqSize + 2;
    
#undef STAGE
}

/* Layout of Udvm memory:
* [0..algo_start) - UDVM registers, various variables, tables, etc
* [algo_start..algo_end) - algorithm bytecode
* [algo_end..algo_end+2) - decompressed_pointer
* [circular_buffer..circular_buffer_end) - circular buffer
* [circular_buffer_end..circular_buffer_end + returnedParamsSize) - returned parameters
*
*/
static
RvStatus SigCompDynStateCheckMessageSize(IN RvSigCompDynState *self, 
                                         RvSigCompCompressionInfo *pCompressionInfo,
                                         RvSigCompMessageInfo *msg, 
                                         RvInt retParamsSize) {
    RvInt udvmSize;
    RvInt requiredSize;

    if(msg->transportType & RVSIGCOMP_STREAM_TRANSPORT) {
        udvmSize = pCompressionInfo->remoteDMS / 2;
    } else {
        udvmSize = pCompressionInfo->remoteDMS - msg->compressedMessageBuffSize;
    }


    requiredSize = rvZStreamGetReturnedParamsLocation(self) + retParamsSize;
    if(requiredSize > udvmSize) {
        return RV_ERROR_OUTOFRESOURCES;
    }
    return RV_OK;
}

static RvBool SigCompDynStateSafeToDelete(IN RvSigCompDynState *self,  IN RvSigCompDynZ *z) 
{
#define STAGE "REM"

    RvSigCompDynZ     *curDep;
    RvSigCompZTimeSec  latestUnack = 0;

    /* State 's' is safe for delete if:
     *  1. State wasn't acknowledged and is too old, e.g. when last message from peer was sent, our message 
     *     corresponding to this state, was already accepted by it or our message was lost
     *     In any case, we may ask to delete this state
     *
     *  2. State was acknowledged and latest non-acknowledged dependent of this state is already decompressed 
     *     or lost - we may ask to delete this state
     */

    DYN_LOG_DEBUG1("Analyzing whether <%d,%d> is safe to delete", FIDZ(z));
    if(!z->acked && z->sendTimeSec + 2 * self->maxTripTime < self->lastPeerMessageTime) {
        DYN_LOG_DEBUG2("<%d,%d> sent on %d is safe to delete as old unacked", FIDZ(z), z->sendTimeSec);
        return RV_TRUE;
    }

    if(!z->acked) {
        DYN_LOG_DEBUG1("<%d,%d> isn't safe to delete as young unacked", FIDZ(z));
        return RV_FALSE;
    } else if (z == self->activeState) {
        DYN_LOG_DEBUG1("<%d,%d> isn't safe to delete as current active", FIDZ(z));
        return RV_FALSE;
    }
    
    DYN_LOG_DEBUG1("<%d,%d> was acked, looking for it's latest unacked dependent", FIDZ(z));

    /* Find latest unacknowledged state 
     * 'lastDependentTime' is set only in cases of memory shortage.
     */
    latestUnack = z->lastDependentTime;
    if((curDep = z->pZfirstDependent) != 0) {
        latestUnack = RvMax(latestUnack, curDep->sendTimeSec);

    }
    
    if(latestUnack == 0 || latestUnack + self->maxTripTime < self->curTimeSec) {
        DYN_LOG_DEBUG2("<%d,%d> is safe to delete, latest unacked was sent on %d", FIDZ(z), latestUnack);
        return RV_TRUE;
    }

    DYN_LOG_DEBUG1("<%d,%d> isn't safe to delete", FIDZ(z));
    return RV_FALSE;
#undef STAGE
}


static void SigCompDynStateFindRemoveStates(IN  RvSigCompDynState *self, 
											OUT RvSigCompDynZ    **ppz, 
											OUT RvSize_t		  *pnz) {
#define STAGE "REM"

    RvSize_t	   nz = 0;
    RvSigCompDynZ *cur;
    RvSigCompDynZ **sppz = ppz;
    RvSize_t i;
    RvSigCompDynZ *minKeep = 0;
    RvSigCompDynZ *prev, *next;
    RvInt minKeepPrio;


    DYN_LOG_DEBUG0("Start looking for states to remove");

    for(cur = self->states; cur; cur = cur->pZnext) {
        RvBool safeToDelete = cur->mayBeDeleted = SigCompDynStateSafeToDelete(self, cur);

        if(safeToDelete) {
            continue;

        }
        if(minKeep == 0 || PRIO(minKeep) > PRIO(cur)) {
            minKeep = cur;
        }
    }

    minKeepPrio = minKeep ? PRIO(minKeep) : 65535;

    for(prev = 0, cur = self->states; cur; cur = next) {
        next = cur->pZnext;

        if(!cur->mayBeDeleted) {
            continue;
        }

        if(PRIO(cur) < minKeepPrio) {
            if(prev == 0) {
                self->states = next;
            } else {
                prev->pZnext = next;
            }

            SigCompDynStateZDelete(self, cur);
            continue;
        }

        if(nz < MAX_STATES_TO_REMOVE) {
            *sppz++ = cur;
            nz++;
            continue;
        }

        for(i = 0, sppz = ppz; i < MAX_STATES_TO_REMOVE; sppz++, i++) {
            RvSigCompDynZ *curRem = *sppz;

            if(curRem->nDeleteCount > cur->nDeleteCount) {
                *sppz = cur;
            }
            
        }

    }


    
    *pnz = nz;

    for(i = 0, sppz = ppz; i < nz; sppz++, i++) {
        (*sppz)->nDeleteCount++;
    }

    DYN_LOG_DEBUG1("End looking for states to remove, found %d", *pnz);

#undef STAGE
}


/* State 'a' may affect presence of state 's' at peer compartment */
static RvBool SigCompDynStateZMayBeAffected(IN RvSigCompDynState *self, 
											IN RvSigCompDynZ     *s,
											IN RvSigCompDynZ	 *a)
{
#define STAGE "SAV"

    /* If message doesn't create state at peer - it couldn't affect nothing */

    DYN_LOG_DEBUG2("Decide whether <%d,%d> affects <%d,%d>", FIDZ(a), FIDZ(s));

    if(!a->createsState) {
        DYN_LOG_DEBUG2(" <%d,%d> doesn't create state - couldn't affect <%d,%d>", FIDZ(a), FIDZ(s));
        return RV_FALSE;
    }

    /* If 'a' was already acked, then if PRIO(a) < PRIO(s) 'a' couldn't affect 's' */
    if(a->acked) {
        if(PRIO(a) >= PRIO(s)) {
            DYN_LOG_DEBUG2(" <%d,%d> could affect <%d,%d> as acked with GE prio", FIDZ(a), FIDZ(s));
            return RV_TRUE;
        }

        DYN_LOG_DEBUG2(" <%d,%d> couldn't affect <%d,%d> as acked with LT prio", FIDZ(a), FIDZ(s));
        return RV_FALSE;
    }

    /* If 'a' was already acked, PRIO(a) == PRIO(s) and 'a' was decompressed before 's'
     * 'a' couldn't affect 's'
     */

    if(a->acked && PRIO(a) == PRIO(s)) {
        /* 'a' was decompressed after s was sent,e.g,
         * DecompressTime(s) >= SentTime(s)
         * 'a' was decompressed before SentTime(a) + RV_SIGCOMP_MAX_TRIP_TIME 
         * DecompressTime(a) <= SentTime(a) + RV_SIGCOMP_MAX_TRIP_TIME 
         */
        if(a->sendTimeSec + self->maxTripTime <= s->sendTimeSec) {
            DYN_LOG_DEBUG2(" <%d,%d> could affect <%d,%d> as acked with EQ prio that was decompressed first", FIDZ(a), FIDZ(s));
            return RV_TRUE;
        }


        return RV_FALSE;
    }

    /* In which circumstances non-acknowledged state 'a' doesn't affect presence of 's'?
     *
     * If PRIO(a) <= PRIO(s) and when 's' was decompressed 'a' was already present at peer
     * 
     * 'a' arrives to peer not after a->sendTimeSec + RV_SIGCOMP_MAX_TRIP_TIME
     * 's' arrives to peer not before s->sendTimeSec so if 
     * s->sendTimeSec > a->sendTimeSec + RV_SIGCOMP_MAX_TRIP_TIME 
     * 'a' couldn't affect 's'
     */

    return (PRIO(a) > PRIO(s)) || (s->sendTimeSec <= a->sendTimeSec + self->maxTripTime);
#undef STAGE
}

/* RvDynStateMaySaveState
 *  Returns RV_TRUE if state if size 'stateSize' may be safely saved at the peer, assuming that 
 *  states given by array **ppz may be deleted at the same time
 */

#define DONT_SAVE_ANY 0  /* No info can be saved, even not Z entry */
#define DONT_SAVE   1    /* Don't save state information */
#define SAVE_PLAIN  2    /* May save state information as plain text only */
#define SAVE_STREAM 3    /* May save state information as full compressor state */

static 
RvInt SigCompDynStateMaySaveState(
								   IN RvSigCompDynState *self, 
								   IN RvSize_t			 stateSize, 
                                   IN RvSize_t           localSize,
								   IN RvSigCompDynZ		**ppz,
								   IN RvSize_t			  *pnz) 
{
#define STAGE "SAV"

    RvSigCompDynZ *cur;
    RvSigCompDynZ *s = 0;
    RvInt		   spaceDemands = (RvInt)stateSize;
    RvInt		   lowPrioDemands;
    RvSize_t	   i;
    RvUint16	   minPrio = 65535;
    RvSize_t       newTotal;
    RvSize_t       nz = *pnz;
    RvInt          retVal;

    /* State may be saved at the peer if it doesn't push out of compartment
     *  any state that may be still in use.
     * E.g if there exists a state that was used once as active state and it still has
     *  unacknowledged dependents - such a state shouldn't be pushed out of peer compartment
     */

    DYN_LOG_DEBUG1("Deciding whether state of size %d should be saved at peer", stateSize);

    if(spaceDemands > (RvInt)self->peerCompSize) {
        DYN_LOG_DEBUG2( "  State may not be saved: exceeds peer compartment size (stateSize=%d, compartmentSize=%d)",
            spaceDemands, self->peerCompSize);
        return DONT_SAVE_ANY;
    }

    if(self->nStates == self->maxStates) {
        DYN_LOG_DEBUG1("  State may not be saved, exceeding maxStates: %d", self->maxStates);
        *pnz = 0;
        return DONT_SAVE_ANY;
    }

    if(self->nCachedZ + 1 <= self->maxCachedZ) {
        retVal = SAVE_STREAM;
    } else {
        newTotal = self->totalSize + localSize;
        if(newTotal > self->maxTotalSize) {
            DYN_LOG_DEBUG1("  State may not be saved, exceeding maxTotalSize %d", newTotal);
            return DONT_SAVE;
        }
        retVal = SAVE_PLAIN;
    }

    if(self->reliableTransport) {
        return retVal;
    }

    for(cur = self->states; cur; cur = cur->pZnext) {
        /* Find the first acknowledged state that should not be pushed out of
         * peer compartment
         */
        if(cur->acked && !cur->mayBeDeleted && PRIO(cur) < minPrio) {
            s = cur;
            minPrio = PRIO(cur);
        }
    }


    /* There is no state we have to keep */
    if(s == 0 || s->persistent) {
        DYN_LOG_DEBUG0("No states should be kept at peer, may ask to save state");
        return retVal;
    }

    /* Mark all states that MAY be deleted by this message 
    * They couldn't affect ability to save this message, because they will be deleted by
    * this message prior to attempt to save it.
    */
    for(i = 0; i < nz; i++) {
        ppz[i]->marked = RV_TRUE;
    }

    
    DYN_LOG_DEBUG1("<%d,%d> is minimal priority state that should be kept", FIDZ(s));

    /* Now we found state with minimal priority that still has unacknowledged dependents */

    DYN_LOG_DEBUG0("Computing of current space demands at peer");

    for(lowPrioDemands = 0, cur = self->states; 
        cur; 
        cur = cur->pZnext) {

        if(SigCompDynStateZMayBeAffected(self, s, cur) && !cur->marked) {
            /* 'cur' may affect s and couldn't be removed */
            if(PRIO(cur) >= PRIO(s)) {
                DYN_LOG_DEBUG1("  Adding demands of <%d,%d> to highprio demands", FIDZ(cur));
                spaceDemands += (RvInt)cur->stateSize;
            } else {
                /* Space demands of all low priority states are not cumulative
                 * sum of space demands of each of them, but rather maximum
                 * on these demands
                 */
                DYN_LOG_DEBUG1("  Adding demands of <%d,%d> to lowprio demands", FIDZ(cur));
                lowPrioDemands = RvMax(lowPrioDemands, (RvInt)cur->stateSize);
            }
        }
    }

    DYN_LOG_DEBUG3("Space demands: lowprio: %d, highprio: %d, total: %d", lowPrioDemands, spaceDemands, spaceDemands + lowPrioDemands);
    spaceDemands += lowPrioDemands;

    for(i = 0; i < nz; i++) {
        ppz[i]->marked = RV_FALSE;
    }

    if(spaceDemands <= (RvInt)self->peerCompSize) {
        DYN_LOG_DEBUG1("State of size %d may be saved at peer", stateSize);
        return retVal;
    }

    DYN_LOG_DEBUG1("State of size %d may not be saved at peer", stateSize);
    return DONT_SAVE;
#undef STAGE
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static
RvChar *SSID(RvSigCompStateID sid, RvChar *buf) {
    if(sid == 0) {
        strcpy(buf, "00 00 00 00 00 00");
    } else {
        sprintf(buf, "%2.2x %2.2x %2.2x %2.2x %2.2x %2.2x", sid[0], sid[1], sid[2], sid[3], sid[4], sid[5]);
    }
    return buf;
}
#endif

static 
RvUint8* SigCompDynDupPlain(RvSigCompDynState *self, RvUint8 *plain, RvSize_t plainSize) {
    RvUint8 *newPlain;
    RvStatus s;

    (void)self;
    s = RvMemoryAlloc(0, plainSize, 0, (void **)&newPlain);
    if(s != RV_OK) {
        return 0;                
    }

    self->totalSize += plainSize;
    self->peekTotalSize = RvMax(self->peekTotalSize, self->totalSize);

    memcpy(newPlain, plain, plainSize);
    return newPlain;
}


RvStatus RVCALLCONV SigCompDynStateCompress(
    IN  RvSigCompDynState		  *self,
    IN  RvSigCompCompartmentHandle hCompartment,
    IN  RvSigCompCompressionInfo   *pCompressionInfo,
    OUT RvSigCompMessageInfo       *pMsgInfo) 
{
#define STAGE "CMP"

    RvStatus      s                  = RV_OK;
    RvUint8       *pByteCode;        
    RvUint32      bytecodeSize;      
    RvUint32      bytecodeStart;     
    RvSize_t      compressedSize;    
    RvUint8       *pCompressedStart;
    RvSigCompDynZ *newState = 0;       
    RvSigCompDynZ *activeState;      
    RvBool        peerHasBytecode;   
    RvSize_t      sigCompHeaderSize;
    RvSize_t      dynCompHeaderSize;
    RvInt         maySaveState;
	RvSigCompDynZStream  *z;                 
    RvSigCompDynZ        *removeStates[MAX_STATES_TO_REMOVE];
    RvSize_t      nRemoveStates = 0;
    RvSize_t      i;
    RvSize_t      minAccessLen;
    RvUint8       *plain;
    RvSize_t      plainSize;
    RvUint8       *newPlain = 0;
    RvSize_t      headerOverhead;
    RvSize_t      stateSize;
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    RvChar        sidbuf[30];
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */ 
    RvInt         retParamsSize = 0;
    RvSigCompDynFid newFid;
    RvBool createsState = RV_FALSE;
    RvBool reliableTransport;
    
    self->curTimeSec = SigCompDynZTimeSecGet();
    self->reliableTransport = reliableTransport = pMsgInfo->transportType & RVSIGCOMP_RELIABLE_TRANSPORT;
    if(reliableTransport) {
        self->maxTripTime = 0;
    }

    if(pCompressionInfo->nIncomingMessages > self->lastPeerMessageNum) {
        self->lastPeerMessageNum = pCompressionInfo->nIncomingMessages;
        self->lastPeerMessageTime = pCompressionInfo->lastIncomingMessageTime;
    }

    DYN_LOG_DEBUG0("Starting compression");

    SigCompDynStateFindRemoveStates(self, removeStates, &nRemoveStates);
    stateSize = rvZStreamGetStateSize(self) + 64;

    plain = pMsgInfo->pPlainMessageBuffer;
    plainSize = pMsgInfo->plainMessageBuffSize;

    maySaveState = SigCompDynStateMaySaveState(self, stateSize, plainSize, removeStates, &nRemoveStates);
    activeState = self->activeState;

    if(maySaveState == SAVE_PLAIN) {
        /* If we should save state info as plain text - try
         * duplicate 'plain'. If duplication failed - don't save state 
         * info
         */
        newPlain = SigCompDynDupPlain(self, plain, plainSize);
        if(newPlain == 0) {
            maySaveState = DONT_SAVE;
        }
    }

    if(maySaveState == DONT_SAVE_ANY || (s = SigCompDynStateZAlloc(self, &newState) != RV_OK)) {
        nRemoveStates = 0;
        maySaveState = DONT_SAVE_ANY;
        activeState->lastDependentTime = self->curTimeSec;
    } 

    createsState = maySaveState > DONT_SAVE;
    newFid.prio = (RvUint16)(activeState->fid.prio + 1);
    newFid.wrapCnt = activeState->fid.wrapCnt;

    if(newFid.prio == MAX_STATE_PRIO) {
        newFid.prio = 1;
        newFid.wrapCnt++;
    }
    newFid.zid = self->curZid++;


    peerHasBytecode = activeState->includesBytecode;
    minAccessLen = rvZStreamGetMinAccessLen(self);
    if(peerHasBytecode) {
      pByteCode = activeState->sid;
      bytecodeSize = (RvUint32)minAccessLen;
      bytecodeStart = 0;
    } else {
      pByteCode = rvZStreamGetBytecode(self);
      bytecodeSize = (RvUint32)rvZStreamGetBytecodeSize(self);
      bytecodeStart = rvZStreamGetBytecodeStart(self);
    }

    pCompressedStart = pMsgInfo->pCompressedMessageBuffer;
    compressedSize   = sigCompHeaderSize = pMsgInfo->compressedMessageBuffSize;

    s = RvSigCompGenerateHeader(hCompartment, 
                                pByteCode, 
                                bytecodeSize, 
                                bytecodeStart,
                                pCompressedStart, 
                                (RvUint32*)&sigCompHeaderSize);



    if(s != RV_OK) {
      goto failure;
    }

  /*
   * Format of sent message:
   *
   * Each message is preceded by dyncomp header:
   *
   *  Bytes 0..3  - message id that consists of 2 two-bytes values
   *  state_retention_priority 
   *  zid
   * 
   * Bytes 4 - 4-bits flag
   *   bit 3 - if 1 - state should be saved
   *   bit 0..2 - if n > 0 gives the number of states to remove (up to 4)
   *              next 6 * n bytes gives state ids of states to be removed
   * Next min_access_len * n bytes state ids of states that should be removed
   *
   */

    dynCompHeaderSize = DYNCOMP_RHEADER_SIZE + minAccessLen * nRemoveStates;
    if(sigCompHeaderSize + dynCompHeaderSize > compressedSize) {
        s = RV_ERROR_INSUFFICIENT_BUFFER;
        goto failure;
    }

    headerOverhead = sigCompHeaderSize + dynCompHeaderSize;
    compressedSize -= headerOverhead;
    pCompressedStart += sigCompHeaderSize;

    /* Start generation of dyncomp header */

    /* Write prio */
    *pCompressedStart++ = (RvUint8)(newFid.prio >> 8);
    *pCompressedStart++ = (RvUint8)(newFid.prio & 0xff);

    /* Write zid */
    *pCompressedStart++ = (RvUint8)(newFid.zid >> 8);
    *pCompressedStart++ = (RvUint8)(newFid.zid & 0xff);

    /* Write flags */
    *pCompressedStart++ = (RvUint8)((nRemoveStates << 1) | ((maySaveState > DONT_SAVE) ? 1 : 0));

    /* Write state ids of states to remove */
    for(i = 0; i < nRemoveStates; i++) {
        memcpy(pCompressedStart, removeStates[i]->sid, minAccessLen);
        pCompressedStart += minAccessLen;
    }

    /* End generation of dyncomp header */

    retParamsSize = SigCompDynGenerateReturnedParameters(self, 
        pCompressionInfo, 
        pCompressedStart, 
        (RvUint32)compressedSize);

    if(retParamsSize < 0) {
        s = RV_ERROR_UNKNOWN;
        goto failure;
    }

    pCompressedStart += retParamsSize;
    compressedSize -= retParamsSize;
    headerOverhead += retParamsSize;


    s = rvZStreamAlloc(self, &z);

    if(s != RV_OK) {
        DYN_LOG_ERROR1("Deflate stream allocation error in compartment %p", self);
        goto failure;
    }

    s = rvZStreamCompress(
        self, z, activeState->pZ, plain, plainSize, pCompressedStart, &compressedSize, (newState ? newState->sid : 0));
    
    if(s != RV_OK) {
        DYN_LOG_ERROR1("Deflate compression error in compartment %p", self);
        rvZStreamFree(self, z);
        goto failure;
    }

    compressedSize += headerOverhead;
    pMsgInfo->compressedMessageBuffSize = (RvUint32)compressedSize;

    s = SigCompDynStateCheckMessageSize(self, pCompressionInfo, pMsgInfo, retParamsSize);
    if(s != RV_OK) {
        DYN_LOG_ERROR3("Compressed message is too long to decompress successfully in compartment %p"
            "(plainSize: %d, compressedSize: %d", self, pMsgInfo->plainMessageBuffSize, pMsgInfo->compressedMessageBuffSize);
        rvZStreamEnd(self, z);
        rvZStreamFree(self, z);
        goto failure;
    }

    activeState->servedAsActive = RV_TRUE;

    if(maySaveState != SAVE_STREAM) {
        /* Free stream */
        rvZStreamEnd(self, z);
        rvZStreamFree(self, z);
        z = 0;
    }

    
    self->curCompressed += compressedSize;
    self->curPlain += plainSize;
#ifdef RV_SIGCOMP_STATISTICS
    self->curRatio = (double)self->curPlain / (double)self->curCompressed;
#endif /*#ifdef RV_SIGCOMP_STATISTICS*/


#ifdef RV_SIGCOMP_STATISTICS
    DYN_LOG_DEBUG6("Compression successful, ratio: %d / %d = %g, total ratio: %d / %d = %g", plainSize, compressedSize, 
        (double)plainSize / (double)compressedSize, self->curPlain, self->curCompressed, self->curRatio);
#endif /*#ifdef RV_SIGCOMP_STATISTICS*/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    DYN_LOG_DEBUG4("<%d,%d SID: %s> created compressed relative to <%d,%d>, %s", 
        FID(&newFid),  SSID((newState ? newState->sid : 0), sidbuf), FIDZ(activeState), 
        createsState ? "creates state" : "doesn't create state");
#endif

    if(maySaveState == DONT_SAVE_ANY || newState == 0) {
        /* Actually, newState shouldn't be '0' here, e.g (newState == 0) => maySaveState == DONT_SAVE_ANY,
         * but lint still complains about it
         */
        return RV_OK;
    }

    newState->includesBytecode = RV_TRUE;

    /* Initialize remove requests field in newState */
    for(i = 0; i < nRemoveStates; i++) {
        newState->removeRequests[i] = removeStates[i]->fid;
    }

    newState->nRequests = nRemoveStates;
    newState->createsState = createsState;
    newState->stateSize = createsState ? stateSize : 0;
    newState->fid = newFid;
    newState->sendTime = self->curTime;
    newState->sendTimeSec = self->curTimeSec;
    newState->acked = RV_FALSE;
    newState->mayBeDeleted = RV_FALSE;

     /* Do some bookkeeping in case new state should be kept */
     
     /* Update dependent chain */
    SigCompDynZAddDependent(activeState, newState);
    
     /* Update state chain */
     if(self->states == 0) {
         self->states = newState;
     } else {
         RvSigCompDynZ *cur;
         RvSigCompDynZ *prev = 0;
         
         for(cur = self->states; cur; prev = cur, cur = cur->pZnext) {/*empty*/}
         /*lint -e{794} 'prev' can't be '0' in this context ('for' loop is taken at least once)*/
         prev->pZnext = newState;
     }
     
     self->peekStates = RvMax(self->nStates, self->peekStates);
     if(maySaveState == SAVE_STREAM) {
         RvAssert(z);
         self->nCachedZ++;
         newState->pZ = z;
     } else if(maySaveState == SAVE_PLAIN) {
         RvAssert(newPlain);
         newState->plain = newPlain;
         newState->plainSize = plainSize;
     }


     if(maySaveState > DONT_SAVE && reliableTransport) {
         newState->acked = RV_TRUE;
         SigCompDynStateChangeActive(self, newState);
     }
    
     return s;

failure:
    if(newPlain) {
        SigCompDynFreePlain(self, newPlain, plainSize);
    }
    
    if(newState) {
        SigCompDynStateZDelete(self, newState);
    }

    return s;
#undef STAGE
}




/* Debug functions */

#if RV_SIGCOMP_DYN_DEBUG 

static 
void SigCompDynStateDumpZ(RvSigCompDynState *self, RvSigCompDynZ *state) {
#define STAGE "DUMP"
    RvSigCompDynZ *cur;

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARGS(self);
#endif
    DYN_LOG_DEBUG6("<%d,%d> DAD: <%d,%d> ACT: %d SND: %d ACK: %d SAV: %d", 
        FID(&state->fid), 
        FID(&state->dfid),
        state->servedAsActive,
        state->sendTimeSec,
        state->ackedTimeSec,
        state->createsState);


    for(cur = state->pZfirstDependent; cur; cur = cur->pZnextDependent) {
        DYN_LOG_DEBUG1("\tDEP: <%d,%d>", FID(&cur->fid));
    }

#undef STAGE
}


static 
void SigCompDynStateDump(RvSigCompDynState *self) {
#define STAGE "DUMP"
    RvSigCompDynZ *cur;

    
    DYN_LOG_DEBUG2("States: %d, Peek States: %d", self->nStates, self->peekStates);
    DYN_LOG_DEBUG2("Total: %d, Peek Total: %d", self->totalSize, self->peekTotalSize);
    for(cur = self->states; cur; cur = cur->pZnext) {
        SigCompDynStateDumpZ(self, cur);
    }

#undef STAGE
}

#endif

