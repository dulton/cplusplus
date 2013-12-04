/************************************************************************
 File Name	   : rvsrtpdbsource.c
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
#include "rvstdio.h" /* memset on linux */
#include "rvsrtpstream.h"
#include "rvsrtpkey.h"

/* Max index values for RTP and RTCP */
#define RV_SRTPSTREAM_RTPMAXINDEX RvUint64Const2(0xFFFFFFFFFFFF)
#define RV_SRTPSTREAM_RTCPMAXINDEX RvUint64Const2(0x7FFFFFFF)

static void *rvSrtpStreamDbObjConstruct (void *objptr, void *data);
static void rvSrtpStreamDbObjDestruct (void *objptr, void *data);
static void *rvSrtpStreamDbAlloc (RvSize_t size, void *data);
static void rvSrtpStreamDbFree (void *ptr, void *data);

RvSrtpStream *rvSrtpStreamConstruct (RvSrtpStream *thisPtr, RvUint64 replayListSize, RvSrtpContextPool *contextPool)
{
    if (NULL == thisPtr)
        return (NULL);

    thisPtr->ssrc = RvUint32Const(0);
	thisPtr->isRTP = RV_FALSE;
    thisPtr->isRemote = RV_FALSE;
    thisPtr->replayListSize = replayListSize;
	RvAddressConstruct(RV_ADDRESS_TYPE_NONE, &thisPtr->address);
	rvSrtpHistoryConstruct(&thisPtr->history, RvUint64Const2(0), RvUint64Const2(0), RV_SRTPSTREAM_RTPMAXINDEX, contextPool);

    return thisPtr;
} /* rvSrtpStreamConstruct */

void rvSrtpStreamDestruct (RvSrtpStream *thisPtr)
{
    if (NULL != thisPtr)
    {
		rvSrtpHistoryDestruct(&thisPtr->history);
		RvAddressDestruct(&thisPtr->address);
    }
    
} /* rvSrtpStreamDestruct */

RvSize_t rvSrtpStreamCalcSize (RvUint64 replayListSize)
{
    RvSrtpStream tmp;
    RvSize_t     size = (((RvUint8 *)&tmp.replayList[0] - (RvUint8 *)&tmp) + (RvSize_t)(replayListSize >> 3));
    
    return size;
} /* rvSrtpStreamCalcSize */

RvBool rvSrtpStreamSetUp(RvSrtpStream *thisPtr, RvUint64 initIndex, RvUint64 historySize, RvBool isRtp)
{
	RvUint64 wrapIndex;

	wrapIndex = (thisPtr->isRTP == RV_TRUE) ? RV_SRTPSTREAM_RTPMAXINDEX : RV_SRTPSTREAM_RTCPMAXINDEX;
	rvSrtpHistoryReset(&thisPtr->history, initIndex, historySize, wrapIndex);
    RV_UNUSED_ARG(isRtp);
	return RV_TRUE;
}

RvBool rvSrtpStreamTestReplayList(RvSrtpStream *thisPtr, RvUint64 index, RvBool update)
{
	RvUint64 distance;
	RvUint8 bitMask;
	RvSize_t byteLoc;
	RvBool result;

    RV_UNUSED_ARG(update);
    
	if(thisPtr->replayListSize == 0)
		return RV_TRUE; /* replay list is disabled, allow everything */

	/* Calculate how old index is */
	distance = (rvSrtpHistoryGetMaxIndex(&thisPtr->history) - index) & rvSrtpHistoryGetWrapIndex(&thisPtr->history);
	if(distance > rvSrtpHistoryGetSize(&thisPtr->history))
		return RV_TRUE; /* Index is newer, so index is OK and should be processed. */
	if(distance > thisPtr->replayListSize)
		return RV_FALSE; /* Index is older than replay list size, so it is considered a repeat */

#if OLD_JAYS_CODE
	/* Find location in replay list and check it */
	result = RV_FALSE;
	bitMask = (RvUint8) (RvUint8Const(1) << (RvUint8)(index & RvUint64Const2(0x7)));
	byteLoc = (RvSize_t)(index >> 3);
	if((thisPtr->replayList[byteLoc] & bitMask) == RvUint8Const(0)) {
		result = RV_TRUE;

		/* Update the replay list if requested */
		thisPtr->replayList[byteLoc] |= bitMask;
	}
#else
	/* Find location in replay list and check it */
	result = RV_FALSE;
	bitMask = (RvUint8) (RvUint8Const(1) << (RvUint8)(distance & RvUint64Const2(0x7)));
	byteLoc = (RvSize_t)(distance >> 3);
	if((thisPtr->replayList[byteLoc] & bitMask) == RvUint8Const(0)) {
		result = RV_TRUE;

		/* Update the replay list if requested */
		thisPtr->replayList[byteLoc] |= bitMask;
	}
#endif
	return result;
}

static void rvSrtpStreamInitReplayList(RvSrtpStream *thisPtr)
{
	/* At the start, old packets are not accepted. Only packets that */
	/* have been skipped are accepted, so start will the entire list set.*/
	if(thisPtr->replayListSize != 0)
		memset(thisPtr->replayList, 0xFF, ((RvSize_t)(thisPtr->replayListSize + RvUint64Const2(7)) >> 3));
}

/* Shift the replay list and set the newest one at being seen */
static void rvSrtpStreamShiftReplayList(RvSrtpStream *thisPtr, RvUint64 distance)
{
	RvSize_t bufSize;
	RvSize_t byteShift;
	RvSize_t bitShift;
	RvUint8 *source;
	RvUint8 *dest;
	RvSize_t revShift;
	RvUint8 carry;

	if(thisPtr->replayListSize == 0)
		return; /* Replay list disabled */

	bufSize = (RvSize_t)((thisPtr->replayListSize + RvUint64Const2(7)) >> 3);

	if(distance < thisPtr->replayListSize) {
		/* We need to shift the entire buffer */
		bitShift = (RvSize_t)(distance & RvUint64Const2(0x7));
		byteShift = (RvSize_t)(distance >> 3);

		/* Do the byte shift */
		if(byteShift > 0) {
			dest = thisPtr->replayList + bufSize - 1;
			source = dest - byteShift;
			for(;;) {
				*dest = *source;
				if(source == thisPtr->replayList)
					break;
				dest--;
				source--;
			}
		}

		/* Do the bit shift */
		if(bitShift > 0) {
			source = thisPtr->replayList;
			dest = thisPtr->replayList + bufSize - 1; /* last byte */
			revShift = (8 - bitShift);
			carry = RvUint8Const(0);
			for(;;) {
				carry = (RvUint8) (*source >> revShift);
				*source = (RvUint8)((*source << bitShift) | carry);
				if(source == dest)
					break;
				source++;
			}
		}
	} else {
		/* Jumped past everything, just clear the list */
		memset(thisPtr->replayList, 0x0, bufSize);
	}

	/* Set the newest */
	thisPtr->replayList[0] |= RvUint8Const(1);
}

/* Standard stream functions */
RvSize_t rvSrtpStreamGetSize (RvSrtpStream *thisPtr)
{
    RvSrtpStream tmp;
    RvSize_t     size;

    if (NULL == thisPtr)
        return (0);
    size = (((RvUint8 *)&(tmp.replayList[0]) - (RvUint8 *)&tmp) + (RvSize_t)(thisPtr->replayListSize >> 3));
    
    return size;
} /* rvSrtpStreamGetSize */

/* Context related functions */
RvSrtpContext *rvSrtpStreamAddContext(RvSrtpStream *thisPtr, RvUint64 startIndex, RvSrtpKey *masterKey, RvBool trigger)
{
	RvSrtpContext *context;
	RvBool contextAdded;

	context = rvSrtpHistoryAdd(&thisPtr->history, startIndex, masterKey, &contextAdded);
	if((context != NULL) && (contextAdded == RV_TRUE)) {
		/* New context object, initialize our context information */
		rvSrtpContextSetData(context, thisPtr);
		rvSrtpContextSetTrigger(context, trigger);
	}
	return context;
}

RvBool rvSrtpStreamUpdateMaxIndex(RvSrtpStream *thisPtr, RvUint64 newMaxIndex)
{
	RvUint64 oldMaxIndex;
	RvUint64 distance;
	RvBool result;

	oldMaxIndex = rvSrtpHistoryGetMaxIndex(&thisPtr->history);

	/* Update maxIndex and history list */
	result = rvSrtpHistoryUpdateMaxIndex(&thisPtr->history, newMaxIndex);

	/* If maxIndex got updated, then we also need to update the replay list */
	if(result == RV_TRUE) {
		distance = (newMaxIndex - oldMaxIndex) & rvSrtpHistoryGetWrapIndex(&thisPtr->history);
		rvSrtpStreamShiftReplayList(thisPtr, distance);
	}

	return result;
}

RvBool rvSrtpStreamTrigger(RvSrtpContext *contextPtr)
{
	RvSrtpStream *streamPtr;
	RvSrtpContext *oldContext;
	RvSrtpKey *oldKey;
	RvObjList *oldKeyContextList;
	RvSrtpContext *testContext;
	RvSrtpStream *testStream;
	RvSrtpContext *nextContext;
	RvUint64 newIndex;

	if(rvSrtpContextGetTrigger(contextPtr) == RV_FALSE)
		return RV_FALSE; /* Context does not cause a trigger */

	/* Get old master key context list */
	streamPtr = (RvSrtpStream *)rvSrtpContextGetData(contextPtr);
	oldContext = rvSrtpHistoryGetPrevious(&streamPtr->history, contextPtr);
	if(oldContext == NULL)
		return RV_FALSE; /* Nothing to trigger from */
	oldKey = rvSrtpContextGetMasterKey(oldContext);
	if(oldKey == NULL)
		return RV_FALSE; /* Can't trigger from a NULL context */
	oldKeyContextList = rvSrtpKeyGetContextList(oldKey);

	/* Find streams that need to be triggered in the key's list and schedule the change */
	testContext = NULL;
	for(;;) {
		/* Get next context in key's list and its stream */
		testContext = RvObjListGetNext(oldKeyContextList, testContext, RV_OBJLIST_LEAVE);
		if(testContext == NULL)
			break;
		testStream = (RvSrtpStream *)rvSrtpContextGetData(testContext);
		if(testStream == streamPtr)
			continue; /* Ignore contexts in triggering stream */

		/* See if the stream is currently using the context */
		newIndex = rvSrtpHistoryGetMaxIndex(&testStream->history);
		if(rvSrtpHistoryCheckIndex(&testStream->history, newIndex, testContext) == RV_FALSE)
			continue; /* context not being used, skip to next one */

		/* Insure that the stream isn't already scheduled for a key switch */
		newIndex = (newIndex + RvUint64Const2(1)) & rvSrtpHistoryGetWrapIndex(&testStream->history);
		nextContext = rvSrtpHistoryGetNext(&testStream->history, testContext);
		if((nextContext != NULL) && (rvSrtpContextGetFromIndex(nextContext) == newIndex))
			continue; /* key switch already scheduled */

		/* Schedule a switch to the new key for the next packet */
		rvSrtpStreamAddContext(testStream, newIndex, rvSrtpContextGetMasterKey(contextPtr), RV_FALSE);
	}

	return RV_TRUE;
}

void rvSrtpStreamIncEncryptCount(RvSrtpContext *contextPtr)
{
	RvSrtpStream *streamPtr;
	RvUint64 newCount;

	streamPtr = (RvSrtpStream *)rvSrtpContextGetData(contextPtr);

	/* Increment context counter since it refers to a particular master key */
	newCount = rvSrtpContextIncCount(contextPtr);

	/* Master key maximums happen to be the same as the wrap indexes, so use it */
	if(newCount >= rvSrtpHistoryGetWrapIndex(&streamPtr->history))
		rvSrtpKeySetMaxLimit(rvSrtpContextGetMasterKey(contextPtr), RV_TRUE); /* Limit reached */
}

/* Pool functions */
RvSrtpStreamPool *rvSrtpStreamPoolConstruct (RvSrtpStreamPool *pool, RvUint64 replayListSize, RvSrtpContextPool *contextPool, RvMemory *region,
                                             RvSize_t pageitems, RvSize_t pagesize, RvInt pooltype,
                                             RvSize_t maxitems, RvSize_t minitems, RvSize_t freelevel)
{
    RvObjPoolFuncs streamCB;
    RvSrtpStream   tmpStream;
    RvSize_t       itemSize;
    RvObjPool     *tmpPool;
    
    if((pool == NULL) || (contextPool == NULL))
        return NULL;

    pool->region = region;
	pool->replayListSize = replayListSize;
	pool->contextPool = contextPool;
    
    streamCB.objconstruct = rvSrtpStreamDbObjConstruct;
    streamCB.objdestruct = rvSrtpStreamDbObjDestruct;
    streamCB.pagealloc = rvSrtpStreamDbAlloc;
    streamCB.pagefree = rvSrtpStreamDbFree;
    streamCB.objconstructdata = pool;
    streamCB.objdestructdata = pool;
    streamCB.pageallocdata = pool;
    streamCB.pagefreedata = pool;
    
    itemSize = rvSrtpStreamCalcSize (replayListSize);
    tmpPool = RvObjPoolConstruct (&tmpStream, &(tmpStream.poolElem), &streamCB, itemSize, pageitems, pagesize, 
                                 pooltype, ((RV_OBJPOOL_TYPE_DYNAMIC == pooltype) ? RV_TRUE : RV_FALSE), maxitems,
                                  minitems, freelevel, &(pool->pool));

    if (NULL == tmpPool)
    {
        return (NULL);
    }

    return (pool);
} /* rvSrtpStreamPoolConstruct */

RvBool rvSrtpStreamPoolDestruct (RvSrtpStreamPool *pool)
{
    if (NULL == pool)
        return (RV_FALSE);

    return (RvObjPoolDestruct (&(pool->pool)));
} /* rvSrtpStreamPoolDestruct */

RvSrtpStream *rvSrtpStreamPoolNew (RvSrtpStreamPool *pool, RvUint64 initIndex, RvUint64 historySize, RvBool isRtp)
{
    RvSrtpStream *item;
    
    if (NULL == pool)
        return (NULL);

	item = (RvSrtpStream *)RvObjPoolGetItem(&pool->pool);
    if (NULL == item)
        return (NULL);

    item->ssrc = RvUint32Const(0);
    item->isRTP = isRtp;
    item->isRemote = RV_FALSE;
	rvSrtpStreamInitReplayList(item);
	if(rvSrtpStreamSetUp(item, initIndex, historySize, isRtp) == RV_FALSE) {
		RvObjPoolReleaseItem (&pool->pool, item);
		return NULL;
	}

    return item;
} /* rvSrtpStreamPoolNew */

void rvSrtpStreamPoolRelease (RvSrtpStreamPool *pool, RvSrtpStream *stream)
{
	if((stream != NULL) && (pool != NULL)) {
		rvSrtpHistoryClear(&stream->history); /* Make sure all contexts are gone */
		RvObjPoolReleaseItem (&pool->pool, stream);
	}
} /* rvSrtpStreamPoolRelease */

static void *rvSrtpStreamDbObjConstruct (void *objptr, void *data)
{
    RvSrtpStream *entry;
    RvSrtpStreamPool *pool;

	pool = (RvSrtpStreamPool *)data;
	entry = rvSrtpStreamConstruct ((RvSrtpStream *)objptr, pool->replayListSize, pool->contextPool);

    return (void *)entry;
} /* rvSrtpStreamDbObjConstruct */

static void rvSrtpStreamDbObjDestruct (void *objptr, void *data)
{
	rvSrtpStreamDestruct((RvSrtpStream *)objptr);
    RV_UNUSED_ARG(data);
} /* rvSrtpStreamDbObjDestruct */

static void *rvSrtpStreamDbAlloc (RvSize_t size, void *data)
{
    void *res;
    RvSrtpStreamPool *pool = (RvSrtpStreamPool *)data;   
    RvStatus rc;

    rc = RvMemoryAlloc (pool->region, size, NULL, &res);
    if (RV_OK != rc)
        return (NULL);

    return (res);
} /* rvSrtpStreamDbAlloc */

static void rvSrtpStreamDbFree (void *ptr, void *data)
{
    RV_UNUSED_ARG(data);
    RvMemoryFree (ptr, NULL);
} /* rvSrtpStreamDbFree */

#if defined(RV_TEST_CODE)
#include "rvstdio.h"
#include "rvrtpstatus.h"
#include "rvsrtpdb.h"

RvRtpStatus RvSrtpStreamTest ()
{
    RvSrtpDbPoolConfig cfg;
    RvSrtpStreamPool sPool;
    RvSrtpStream  *stream;
    RvSrtpStream  *srcArr[1000];
    RvAddress      destAddr;
    RvAddress     *addr;
    RvInt          kkk, iii;

    cfg.streamRegion = NULL;
    cfg.streamPageItems = 10;
    cfg.streamPageSize  = 3000;
    cfg.streamPoolType  = RV_OBJPOOL_TYPE_EXPANDING;
    cfg.streamMaxItems  = 0;
    cfg.streamMinItems  = 10;
    cfg.streamFreeLevel = 0;

    if (NULL == rvSrtpStreamPoolConstruct (&sPool, 50, cfg.streamRegion, cfg.streamPageItems,
                                            cfg.streamPageSize,cfg.streamPoolType, cfg.streamMaxItems,
                                            cfg.streamMinItems, cfg.streamFreeLevel))
    {
        RvPrintf ("Failed to construct stream pool\n");
        return (RV_RTPSTATUS_Failed);
    }

    if (NULL == RvAddressConstruct (RV_ADDRESS_TYPE_IPV4, &destAddr))
        RvPrintf ("Fail constructing address\n");
    RvAddressSetIpPort (&destAddr, 1000);
    RvAddressSetString ("11.22.33.44", &destAddr);
    stream = rvSrtpStreamPoolNew (&sPool);
    if (NULL == stream)
    {
        rvSrtpStreamPoolDestruct (&sPool);
        RvPrintf ("Failed to allocate new stream\n");
        return (RV_RTPSTATUS_Failed);
    }

    rvSrtpStreamSetSsrc (stream, 0x98765432);
    rvSrtpStreamSetIsRtp (stream, RV_TRUE);
    rvSrtpStreamSetIsRemote (stream, RV_FALSE);
    memcpy (rvSrtpStreamGetAddress (stream), &destAddr, sizeof (RvAddress));
    rvSrtpStreamSetInitIndex (stream, 0x1);
    
    RvPrintf ("rvSrtpStreamGetSize:  size is %d\n", rvSrtpStreamGetSize (stream));
    RvPrintf ("rvSrtpStreamCalcSize: size is %d\n",
              rvSrtpStreamCalcSize (RvSize_t)(rvSrtpStreamGetReplayListSize (stream)));
    RvPrintf ("OLD SSRC is %x\n", rvSrtpStreamGetSsrc (stream));
    rvSrtpStreamSetSsrc (stream, 0x11112222);
    RvPrintf ("NEW SSRC is %x\n", rvSrtpStreamGetSsrc (stream));
    addr = rvSrtpStreamGetAddress (stream);
    RvPrintf ("address type is %s\n", (RV_ADDRESS_TYPE_IPV4 == RvAddressGetType (addr)) ? "V4" : "Other");
    RvPrintf ("address is %x\n", addr->data.ipv4.ip);
    RvPrintf ("address port is %d\n", RvAddressGetIpPort (addr));
    rvSrtpStreamSetMaxIndex (stream, 1000);
    RvPrintf ("maxIndex = %d\n", rvSrtpStreamGetMaxIndex (stream));
    rvSrtpStreamSetInitIndex (stream, 34);
    RvPrintf ("initIndex = %d\n", rvSrtpStreamGetInitIndex (stream));
    RvPrintf ("isRtp = %s\n", (RV_TRUE == rvSrtpStreamGetIsRtp (stream)) ? "SRTP" : "SRTCP");
    RvPrintf ("isRemote = %s\n", (RV_TRUE == rvSrtpStreamGetIsRemote (stream)) ? "Source" : "Dest");
    RvPrintf ("Replay List Size is %d\n", (RvSize_t)rvSrtpStreamGetReplayListSize (stream));
    RvPrintf ("Replay List Context: %s\n", rvSrtpStreamGetReplayList (stream));

    rvSrtpStreamPoolRelease (&sPool, stream);

    stream = rvSrtpStreamPoolNew (&sPool);
    if (NULL == stream)
    {
        rvSrtpStreamPoolDestruct (&sPool);
        RvPrintf ("Failed to allocate new stream\n");
        return (RV_RTPSTATUS_Failed);
    }

    rvSrtpStreamSetSsrc (stream, 0x98765432);
    rvSrtpStreamSetIsRtp (stream, RV_FALSE);
    rvSrtpStreamSetIsRemote (stream, RV_TRUE);
    rvSrtpStreamSetInitIndex (stream, 0x100);

    RvPrintf ("rvSrtpStreamGetSize:  size is %d\n", rvSrtpStreamGetSize (stream));
    RvPrintf ("rvSrtpStreamCalcSize: size is %d\n",
              rvSrtpStreamCalcSize (RvSize_t)(rvSrtpStreamGetReplayListSize (stream)));
    RvPrintf ("OLD SSRC is %x\n", rvSrtpStreamGetSsrc (stream));
    rvSrtpStreamSetSsrc (stream, 0x34454545);
    RvPrintf ("NEW SSRC is %x\n", rvSrtpStreamGetSsrc (stream));
    addr = rvSrtpStreamGetAddress (stream);
    RvPrintf ("address type is %s\n", (RV_ADDRESS_TYPE_IPV4 == RvAddressGetType (addr)) ? "V4" : "Other");
    RvPrintf ("address is %d\n", addr->data.ipv4.ip);
    RvPrintf ("address port is %d\n", RvAddressGetIpPort (addr));
    rvSrtpStreamSetMaxIndex (stream, 1000);
    RvPrintf ("maxIndex = %d\n", rvSrtpStreamGetMaxIndex (stream));
    rvSrtpStreamSetInitIndex (stream, 34);
    RvPrintf ("initIndex = %d\n", rvSrtpStreamGetInitIndex (stream));
    rvSrtpStreamSetIsRtp (stream, RV_FALSE);
    RvPrintf ("isRtp = %s\n", (RV_TRUE == rvSrtpStreamGetIsRtp (stream)) ? "SRTP" : "SRTCP");
    RvPrintf ("isRemote = %s\n", (RV_TRUE == rvSrtpStreamGetIsRemote (stream)) ? "Source" : "Dest");
    RvPrintf ("Replay List Size is %d\n", (RvSize_t)rvSrtpStreamGetReplayListSize (stream));
    RvPrintf ("Replay List Context: %s\n", rvSrtpStreamGetReplayList (stream));

    rvSrtpStreamPoolRelease (&sPool, stream);

    /* check reinitialization */
    stream = rvSrtpStreamPoolNew (&sPool);
    if (NULL == stream)
    {
        RvPrintf ("Failed to allocate new stream\n");
        return (RV_RTPSTATUS_Failed);
    }
    rvSrtpStreamPoolRelease (&sPool, stream);

    /* check stand alone construction */
    stream = (RvSrtpStream *)RvObjPoolGetItem (&(sPool.pool));
    if (NULL == stream)
    {
        RvPrintf ("Failed to allocate new stream\n");
        return (RV_RTPSTATUS_Failed);
    }

    rvSrtpStreamConstruct (stream, 40);
    rvSrtpStreamDestruct (stream);
    RvObjPoolReleaseItem (&(sPool.pool), stream);

    /* test multiple objects */
    kkk = 1000;
    for (iii = 0; iii < 1000; iii++)
    {
        srcArr[iii] = rvSrtpStreamPoolNew (&sPool);
        if (NULL == srcArr[iii])
        {
            RvPrintf ("Unable to add source %d\n", iii);
            kkk = iii;
            break;
        }
    }
    if (1000 == kkk)
    {
        RvPrintf ("success allocating all streams\n");
    }

    for (iii = 0; iii < kkk; iii++)
    {
        rvSrtpStreamPoolRelease (&sPool, srcArr[iii]);
    }

    rvSrtpStreamPoolDestruct (&sPool);


    cfg.streamRegion = NULL;
    cfg.streamPageItems = 10;
    cfg.streamPageSize  = 3000;
    cfg.streamPoolType  = RV_OBJPOOL_TYPE_FIXED;
    cfg.streamMaxItems  = 20;
    cfg.streamMinItems  = 10;
    cfg.streamFreeLevel = 0;
    
    if (NULL == rvSrtpStreamPoolConstruct (&sPool, 50, cfg.streamRegion, cfg.streamPageItems,
        cfg.streamPageSize,cfg.streamPoolType, cfg.streamMaxItems,
        cfg.streamMinItems, cfg.streamFreeLevel))
    {
        RvPrintf ("Failed to construct stream pool\n");
        return (RV_RTPSTATUS_Failed);
    }
    
    kkk = 1000;
    for (iii = 0; iii < 1000; iii++)
    {
        srcArr[iii] = rvSrtpStreamPoolNew (&sPool);
        if (NULL == srcArr[iii])
        {
            RvPrintf ("Unable to add source %d\n", iii);
            kkk = iii;
            break;
        }
    }
    if (1000 == kkk)
    {
        RvPrintf ("success allocating all streams\n");
    }
    
    for (iii = 0; iii < kkk; iii++)
    {
        rvSrtpStreamPoolRelease (&sPool, srcArr[iii]);
    }
    
    return (RV_RTPSTATUS_Succeeded);
} /* RvSrtpStreamTest */

#endif /* RV_TEST_CODE */
