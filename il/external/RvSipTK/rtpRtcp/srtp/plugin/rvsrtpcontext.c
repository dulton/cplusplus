/************************************************************************
 File Name	   : rvsrtpcontext.c
 Description   :
*************************************************************************
 Copyright (c)	2000-2005, RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVision Inc. 
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVision Inc. 
 
 RADVision Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
*************************************************************************/
#include "rvsrtpcontext.h"
//#include "rvconfigext.h"

static void *rvSrtpContextDbObjConstruct (void *objptr, void *data);
static void rvSrtpContextDbObjDestruct (void *objptr, void *data);
static void *rvSrtpContextDbAlloc (RvSize_t size, void *data);
static void rvSrtpContextDbFree (void *ptr, void *data);

RvSrtpContext *rvSrtpContextConstruct(RvSrtpContext *thisPtr, RvSize_t encryptKeySize, RvSize_t authKeySize, RvSize_t saltSize)
{
	RvSize_t accum;

    if(NULL == thisPtr)
        return (NULL);

    thisPtr->masterKey   = NULL;
    thisPtr->data        = NULL;
    thisPtr->fromIndex   = RvUint64Const2(0);
	thisPtr->switchIndex = RvUint64Const2(0);
	thisPtr->nextSwitchIndex = RvUint64Const2(0);
	thisPtr->neverUsed   = RV_TRUE;
	thisPtr->trigger     = RV_FALSE;
    thisPtr->count       = RvInt64Const2(0);
    thisPtr->currentKey  = RvUint8Const(0);
    thisPtr->keyValid[0] = RV_FALSE;
    thisPtr->keyValid[1] = RV_FALSE;

	/* Set up pointers into variable length buffer and set total size */
	accum = 0;
	thisPtr->encryptKey[0] = (&thisPtr->keys[accum]);
	accum += encryptKeySize;
	thisPtr->encryptKey[1] = (&thisPtr->keys[accum]);
	accum += encryptKeySize;
	thisPtr->authKey[0] = (&thisPtr->keys[accum]);
	accum += authKeySize;
	thisPtr->authKey[1] = (&thisPtr->keys[accum]);
	accum += authKeySize;
	thisPtr->salt[0] = (&thisPtr->keys[accum]);
	accum += saltSize;
	thisPtr->salt[1] = (&thisPtr->keys[accum]);
	accum += saltSize;
	thisPtr->keyBufSize = accum;

    return (thisPtr);
} /* rvSrtpContextConstruct */

RvSize_t rvSrtpContextCalcSize (RvSize_t encryptKeySize, RvSize_t authKeySize, RvSize_t saltSize)
{
    RvSrtpContext tmp;

    return ((RvSize_t)(((encryptKeySize + authKeySize + saltSize) * 2) + (RvUint8 *)&tmp.keys[0] - (RvUint8 *)&tmp));
} /* rvSrtpContextCalcSize */

/* Standard context functions */
RvSize_t rvSrtpContextGetSize(RvSrtpContext *thisPtr)
{
    RvSrtpContext tmp;
    RvSize_t      size;

    size = ((RvUint8 *)&tmp.keys[0] - (RvUint8 *)&tmp) + thisPtr->keyBufSize;

    return (size);
} /* rvSrtpContextGetSize */

RvBool rvSrtpContextUpdateKeys(RvSrtpContext *thisPtr, RvUint64 index, RvUint64 wrapIndex, RvBool *isCurrent)
{

	RvSize_t keyDerivationRate;
	RvUint64 switchIndex;
	RvUint64 nextSwitchIndex;
	RvUint64 testIndex;
	RvUint64 testDelta;

	/* Assume we should use current keys unless we find otherwise */
	*isCurrent = RV_TRUE;

	/* Check to see if we have any current keys */
	if(thisPtr->keyValid[thisPtr->currentKey] == RV_FALSE) {
        /* Need first set of keys, if index is at context start, we can create them */
        /* the index must be the exact index that the context starts with, since the 
           index is part of the IV - any other index (even a larger one) will result
           with a wrong IV == wrong encryption - this is check complies the RFC though
           it might happen that the exact index will not arrive due to packet loss or 
           unordered packets - this can be considered a "bug" in the RFC
        */
				if(index >= thisPtr->fromIndex)
      			return RV_TRUE; /* Valid index, create current keys and use them */
/******************************************************************************************
 * There is one condition that may cause the authentication to fail.
 * This condition may occur when only the initial ROC is given and the initial SEQ is not.
 *
 * Here is the sequence summary:
 * 1 - The sender and the receiver are given initial ROC = 0, the initial
 * SEQ is not given
 * 2 - The sender sends first packet with SEQ = 0xFFFF (SRTP computation is
 * made with ROC = 0, and SEQ = 0xFFFF)
 * 3 - This packet is lost
 * 4 - The sender sends a second packet with SEQ = 0 since it wrapped. So
 * does the ROC. SRTP computation is made with ROC = 1, and SEQ = 0.
 * 5 - The receiver receives this packet. Since an initial SEQ number
 * hasn't been provided, it uses the SEQ received in the packet (highest
 * received RTP SEQ) = 0. When the receiver determines the index with the
 * SRTP algorithm in appendix A, it uses ROC = 0 and SEQ = 0. Since the sender
 * has used ROC = 1 and SEQ = 0, it won't synchronize and will cause the
 * authentication to fail.
 *
 * Note that when initial SEQ is given to the receiver, this problem does 
 * not exist unless more than 32K packets are lost.
 *
 * The solution that is proposed on message board of IETF is
 * " ...this is a problem for the signaling. We have discussed it in the 
 * context of SDP Security Descriptions signaling for SRTP and offer the
 * following guideline: Select the initial sequence number between 0..2^15-1 
 * rather than 0..2^16-1. The solution that you mention is fine, 
 * but I have heard from many that signaling the RTP ROC or SEQ is an  
 * unnatural act - it places constraints on an SRTP implementation to 
 * be  tightly coupled with signaling and/or key management during a session."
 ********************************************************************************************/


		/* We can't derive keys from this index, indicate invalid index */
		return RV_FALSE;
	}

	/* If periodic key generation is disabled, everything is simple */
	keyDerivationRate = rvSrtpKeyGetKeyDerivationRate(thisPtr->masterKey);
	if(keyDerivationRate == 0)
		return RV_TRUE; /* Valid index, use current keys */

	/* If we're at the next key derivation index, it's also simple */
	if(index == thisPtr->nextSwitchIndex) {
		/* Make current keys the old keys (previous old keys disappear) */
		thisPtr->currentKey ^= RvUint8Const(1);
		thisPtr->keyValid[thisPtr->currentKey] = RV_FALSE;
		return RV_TRUE; /* valid index, create current keys and use them */
	}

	/* Normalize values to start of context so the math works */
	switchIndex = (thisPtr->switchIndex - thisPtr->fromIndex) % wrapIndex;
	testIndex = (index - thisPtr->fromIndex) % wrapIndex;

	/* See if we have an index older than the current keys */
	if(testIndex < switchIndex) {
		/* Old index, use old keys */
		*isCurrent = RV_FALSE;

		/* If there's no old keys, we're done */
		if(thisPtr->keyValid[thisPtr->currentKey ^ RvUint8Const(1)] == RV_FALSE)
			return RV_FALSE; /* We can't generate old keys, invalid index */

		/* We do have old keys, see if index is older than old keys */
		testDelta = switchIndex - testIndex;
		if(testDelta > (RvUint64)keyDerivationRate)
			return RV_FALSE; /* Index is too old, can't use it */

		return RV_TRUE; /* valid index, use old keys */
	}

	/* If index is before next switch index, we can use the current keys */
	nextSwitchIndex = (thisPtr->nextSwitchIndex - thisPtr->fromIndex) % wrapIndex;
	if(testIndex < nextSwitchIndex)
		return RV_TRUE; /* Valid index, use use current keys */

	/* Past current keys, see if it happens to be a new derivation point */
	if((index % (RvUint64)keyDerivationRate) == RvUint64Const2(0)) {
		/* Reset old and current keys */
		thisPtr->keyValid[(thisPtr)->currentKey] = RV_FALSE;
		thisPtr->keyValid[(thisPtr)->currentKey ^ RvUint8Const(1)] = RV_FALSE;
		return RV_TRUE; /* Create new current keys and use them */
	}

	/* Invalid index, its too new and we can't create keys */
	return RV_FALSE;
}

void rvSrtpContextEnableCurrentKeys(RvSrtpContext *thisPtr, RvUint64 index, RvUint64 wrapIndex)
{
	    
	thisPtr->keyValid[thisPtr->currentKey] = RV_TRUE;
	thisPtr->switchIndex = index;
#if 0
	thisPtr->nextSwitchIndex = (index + (RvUint64)rvSrtpKeyGetKeyDerivationRate(thisPtr->masterKey)) % wrapIndex;
#else
    {
       RvUint64 rate = RvUint64FromRvSize_t(rvSrtpKeyGetKeyDerivationRate(thisPtr->masterKey));
       if (rate > RvUint64Const(0,0))
       {
           thisPtr->nextSwitchIndex = RvUint64Mod(RvUint64Mul(RvUint64Div(RvUint64Add(index, rate), rate), rate), wrapIndex);
       }
       else
           thisPtr->nextSwitchIndex = RvUint64Mod(index, wrapIndex);
    }
#endif
	thisPtr->neverUsed = RV_FALSE;
}

/* Pool functions */
RvSrtpContextPool *rvSrtpContextPoolConstruct (RvSrtpContextPool *pool, RvSize_t encryptKeySize, RvSize_t authKeySize,
											   RvSize_t saltSize, RvMemory *region, RvSize_t pageitems,
											   RvSize_t pagesize, RvInt pooltype, RvSize_t maxitems,
											   RvSize_t minitems, RvSize_t freelevel)
{
	RvObjPoolFuncs contextCB;
	RvSrtpContext  tempContext;
	RvSize_t       itemSize;
	RvObjPool     *tmpPool;

	if (NULL == pool)
		return (NULL);

	pool->encryptKeySize = encryptKeySize;
	pool->authKeySize = authKeySize;
	pool->saltSize = saltSize;
	pool->region = region;

	contextCB.objconstruct = rvSrtpContextDbObjConstruct;
	contextCB.objdestruct = rvSrtpContextDbObjDestruct;
	contextCB.pagealloc = rvSrtpContextDbAlloc;
	contextCB.pagefree = rvSrtpContextDbFree;
	contextCB.objconstructdata = pool;
	contextCB.objdestructdata = pool;
	contextCB.pageallocdata = pool;
	contextCB.pagefreedata = pool;

	itemSize = rvSrtpContextCalcSize (encryptKeySize, authKeySize, saltSize);
	tmpPool = RvObjPoolConstruct (&tempContext, &(tempContext.poolElem), &contextCB, itemSize, pageitems,
								  pagesize, pooltype, ((RV_OBJPOOL_TYPE_DYNAMIC == pooltype) ? RV_TRUE : RV_FALSE),
								  maxitems, minitems, freelevel, &(pool->pool));

	if (NULL == tmpPool)
		return (NULL);

	return (pool);
} /* RvSrtpContextPoolConstruct */

RvBool rvSrtpContextPoolDestruct(RvSrtpContextPool *pool)
{
	if (NULL == pool)
		return (RV_FALSE);

	return (RvObjPoolDestruct (&(pool->pool)));
} /* RvSrtpContextPoolDestruct */

RvSrtpContext *rvSrtpContextPoolNew(RvSrtpContextPool *pool)
{
	RvSrtpContext *item;

	if (NULL == pool)
		return (NULL);

	item = (RvSrtpContext *)RvObjPoolGetItem(&pool->pool);
	if(NULL == item)
		return NULL;

	item->masterKey   = NULL;
	item->data        = NULL;
	item->fromIndex   = RvUint64Const2(0);
	item->switchIndex = RvUint64Const2(0);
	item->trigger     = RV_FALSE;
	item->count       = RvInt64Const2(0);
	item->currentKey  = RvUint8Const(0);
	item->keyValid[0] = RV_FALSE;
	item->keyValid[1] = RV_FALSE;

	return item;
} /* *RvSrtpContextPoolNew */

void rvSrtpContextPoolRelease(RvSrtpContextPool *pool, RvSrtpContext *context)
{
	if((context != NULL) && (pool != NULL))
		RvObjPoolReleaseItem (&pool->pool, context);
} /* RvSrtpContextPoolRelease */


static void *rvSrtpContextDbObjConstruct (void *objptr, void *data)
{
	RvSrtpContextPool *pool;
	RvSrtpContext *entry;

	pool  = (RvSrtpContextPool *)data;
	entry = rvSrtpContextConstruct((RvSrtpContext *)objptr, pool->encryptKeySize, pool->authKeySize, pool->saltSize);

	return (void *)entry;
} /* rvSrtpContextDbObjConstruct */

static void rvSrtpContextDbObjDestruct (void *objptr, void *data)
{
	rvSrtpContextDestruct((RvSrtpContext *)objptr);
    RV_UNUSED_ARG(data);
    RV_UNUSED_ARG(objptr);
} /* rvSrtpContextDbObjDestruct */

static void *rvSrtpContextDbAlloc (RvSize_t size, void *data)
{
	void *res;
	RvSrtpContextPool *pool = (RvSrtpContextPool *)data;   
	RvStatus rc;

	rc = RvMemoryAlloc (pool->region, size, NULL, &res);
	if (RV_OK != rc)
		return (NULL);

	return (res);
} /* rvSrtpContextDbAlloc */

static void rvSrtpContextDbFree (void *ptr, void *data)
{
	RV_UNUSED_ARG(data);
	RvMemoryFree (ptr, NULL);
} /* rvSrtpContextDbFree */

#if defined(RV_TEST_CODE)
#include "rvstdio.h"
#include "rvsrtpdb.h"

RvRtpStatus RvSrtpContextTest ()
{
    RvSrtpDbPoolConfig cfg;
    RvSrtpContextPool pool;
    RvSrtpContextPool *tmpPool;
    RvSrtpContext *context;
    RvSrtpContext *contextArry[10];
    void *data = (void *)0x1234;
    RvInt iii;
    RvInt kkk;

    cfg.contextRegion = NULL;
    cfg.contextPageItems = 2;
    cfg.contextPageSize  = 3000;
    cfg.contextPoolType  = RV_OBJPOOL_TYPE_FIXED;
    cfg.contextMaxItems  = 0;
    cfg.contextMinItems  = 2;
    cfg.contextFreeLevel = 0;
    tmpPool = rvSrtpContextPoolConstruct (&pool, 50, 40, 60, cfg.contextRegion, cfg.contextPageItems,
                                          cfg.contextPageSize,cfg.contextPoolType, cfg.contextMaxItems,
                                          cfg.contextMinItems, cfg.contextFreeLevel);

    if (NULL == tmpPool)
    {
        RvPrintf ("Failed to construct context pool\n");
        return (RV_RTPSTATUS_Failed);
    }

    context = rvSrtpContextPoolNew (&pool);
    if (NULL == context)
    {
        RvPrintf ("Failed allocating new context\n");
        return (RV_RTPSTATUS_Failed);
    }

    rvSrtpContextSetData (context, data);
    if (data != rvSrtpContextGetData (context))
        RvPrintf ("wrong data\n");

    rvSrtpContextSetFromIndex (context, 10);
    if (10 != rvSrtpContextGetFromIndex (context))
        RvPrintf ("wrong from index\n");

    rvSrtpContextSetSwitchIndex (context, 30);
    if (30 != rvSrtpContextGetSwitchIndex (context))
        RvPrintf ("wrong switch index\n");

    rvSrtpContextSetTrigger (context, RV_TRUE);
    if (RV_TRUE != rvSrtpContextGetTrigger (context))
        RvPrintf ("wrong trigger\n");

    rvSrtpContextSetCount (context, 10);
    if (10 != rvSrtpContextGetCount (context))
        RvPrintf ("wrong count\n");

    {
        RvUint8 *enc;
        RvUint8 *ath;
        RvUint8 *slt;

        enc = rvSrtpContextGetOldEncryptKey (context);
        memcpy (enc, "12345", 5);
        ath = rvSrtpContextGetOldAuthKey (context);
        memcpy (ath, "67890", 5);
        slt = rvSrtpContextGetOldSalt (context);
        memcpy (slt, "2468013579", 7);
        rvSrtpContextEnableCurrentKeys (context);
        rvSrtpContextSwapKeys (context);
        RvPrintf ("CurEncKey  = %s\n", rvSrtpContextGetCurrentEncryptKey (context));
        RvPrintf ("CurAuthKey = %s\n", rvSrtpContextGetCurrentAuthKey (context));
        RvPrintf ("CurSaltKey = %s\n", rvSrtpContextGetCurrentSalt (context));
        RvPrintf ("Current keys are %s\n",
                  (RV_TRUE == rvSrtpContextHasCurrentKeys(context))? "Enabled":"Disabled");
        RvPrintf ("Old keys are %s\n", (RV_TRUE == rvSrtpContextHasOldKeys(context))? "Enabled":"Disabled");
    }

    rvSrtpContextPoolRelease (&pool, context);

    /* check "re-initialization" by re-allocation */
    context = rvSrtpContextPoolNew (&pool);
    if (NULL == context)
    {
        RvPrintf ("Failed to allocate new context\n");
        return (RV_RTPSTATUS_Failed);
    }
    rvSrtpContextPoolRelease (&pool, context);

    /* check stand alone objects */
    context = (RvSrtpContext *)RvObjPoolGetItem (&(pool.pool));
    if (NULL == context)
    {
        RvPrintf ("Failed to allocate new context\n");
        return (RV_RTPSTATUS_Failed);
    }

    rvSrtpContextConstruct (context, 10, 20, 10);
    rvSrtpContextDestruct (context);
    RvObjPoolReleaseItem (&(pool.pool), context);

    /* test fixed pool */
    kkk = 10;
    for (iii = 0; iii < 10; iii++)
    {
        contextArry[iii] = rvSrtpContextPoolNew (&pool);
        if (NULL == contextArry[iii])
        {
            kkk = iii;
            RvPrintf ("Failed to add new context, iii = %d\n", iii);
            break;
        }
    }

    for (iii = 0; iii < kkk; iii++)
    {
        rvSrtpContextPoolRelease (&pool, contextArry[iii]);
    }

    rvSrtpContextPoolDestruct (&pool);

    cfg.contextRegion = NULL;
    cfg.contextPageItems = 5;
    cfg.contextPageSize  = 3000;
    cfg.contextPoolType  = RV_OBJPOOL_TYPE_EXPANDING;
    cfg.contextMaxItems  = 0;
    cfg.contextMinItems  = 2;
    cfg.contextFreeLevel = 0;
    tmpPool = rvSrtpContextPoolConstruct (&pool, 50, 40, 60, cfg.contextRegion, cfg.contextPageItems,
                                          cfg.contextPageSize,cfg.contextPoolType, cfg.contextMaxItems,
                                          cfg.contextMinItems, cfg.contextFreeLevel);

    kkk = 10;
    for (iii = 0; iii < 10; iii++)
    {
        contextArry[iii] = rvSrtpContextPoolNew (&pool);
        if (NULL == contextArry[iii])
        {
            kkk = iii;
            RvPrintf ("Failed to add new context, iii = %d\n", iii);
            break;
        }
    }

    for (iii = 0; iii < kkk; iii++)
    {
        rvSrtpContextPoolRelease (&pool, contextArry[iii]);
    }

    rvSrtpContextPoolDestruct (&pool);

    return (RV_RTPSTATUS_Succeeded);
} /* RvSrtpContextTest */

#endif /* RV_TEST_CODE */
