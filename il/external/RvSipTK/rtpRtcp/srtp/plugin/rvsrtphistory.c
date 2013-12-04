/************************************************************************
 File Name	   : rvsrtphistory.c
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
#include "rvsrtphistory.h"
//#include "rvconfigext.h"

static void rvSrtpHistoryDelete(RvSrtpHistory *thisPtr, RvSrtpContext *contextPtr);
static RvInt rvSrtpHistorySListCmp (void *ptr1, void *ptr2, void *data);

/* Context History functions */
RvSrtpHistory *rvSrtpHistoryConstruct(RvSrtpHistory *thisPtr, RvUint64 initIndex, RvUint64 historySize, RvUint64 wrapIndex, RvSrtpContextPool *pool)
{
	RvSrtpContext tmpContext;

	if(pool == NULL)
		return NULL;
	if(RvObjSListConstruct(&thisPtr->contextSList, &tmpContext, rvSrtpContextGetSListElem(&tmpContext), rvSrtpHistorySListCmp, thisPtr) == NULL)
		return NULL;
	thisPtr->contextPool = pool;
	thisPtr->historySize = historySize;
	thisPtr->initIndex = initIndex;
	thisPtr->wrapIndex = wrapIndex;
	thisPtr->maxIndex = (initIndex - RvUint64Const2(1)) & wrapIndex; /* Last packet received (reverse wrap) */

	return thisPtr;
}

void rvSrtpHistoryDestruct(RvSrtpHistory *thisPtr)
{
	rvSrtpHistoryClear(thisPtr);
	RvObjSListDestruct(&thisPtr->contextSList);
}

void rvSrtpHistoryReset(RvSrtpHistory *thisPtr, RvUint64 initIndex, RvUint64 historySize, RvUint64 wrapIndex)
{
	RvObjSListClear(&thisPtr->contextSList);
	thisPtr->historySize = historySize;
	thisPtr->initIndex = initIndex;
	thisPtr->wrapIndex = wrapIndex;
	thisPtr->maxIndex = (initIndex - RvUint64Const2(1)) & wrapIndex; /* Last packet received (reverse wrap) */
}

void rvSrtpHistoryClear(RvSrtpHistory *thisPtr)
{
	RvSrtpContext *context;
	
	/* Delete all contexts in the history list */
	for(;;) {
		context = (RvSrtpContext *)RvObjSListGetNext(&thisPtr->contextSList, NULL, RV_OBJSLIST_LEAVE);
		if(context == NULL)
			break;
		rvSrtpHistoryDelete(thisPtr, context);
	}
}

RvBool rvSrtpHistoryUpdateMaxIndex(RvSrtpHistory *thisPtr, RvUint64 newMaxIndex)
{
	RvUint64 distance;
	RvUint64 startHistory;
	RvSrtpContext *context;
	RvSrtpContext *tmpContext;

	/* Check for new maxIndex in range */
	if(newMaxIndex > thisPtr->wrapIndex)
		return RV_FALSE;

	/* Insure new maxIndex is not in history window */
	distance = (thisPtr->maxIndex - newMaxIndex) & thisPtr->wrapIndex;
	if(distance <= thisPtr->historySize)
		return RV_FALSE; /* In history window, thus an old packet, don't update */

	/* Find the context at the new history starting point */
	startHistory = (newMaxIndex - thisPtr->historySize) & thisPtr->wrapIndex;
	context = rvSrtpHistoryFind(thisPtr, startHistory);

	/* Update maxIndex */
	thisPtr->maxIndex = newMaxIndex;

	/* Adjust contexts for new history starting point */
	if(context == NULL)
		return RV_TRUE; /* No contexts to clean up, we are done */

	/* Make the context start here if it doesn't already */
	rvSrtpContextSetFromIndex(context, startHistory);

	/* List is chronological, delete all contexts before this one to make it first */
	for(;;) {
		tmpContext = (RvSrtpContext *)RvObjSListGetPrevious(&thisPtr->contextSList, context, RV_OBJSLIST_LEAVE);
		if(tmpContext == NULL)
			break;
		rvSrtpHistoryDelete(thisPtr, tmpContext);
	}

	/* Now clean up any NULL contexts at the beginning of the list */
	while(rvSrtpContextGetMasterKey(context) == NULL) {
		rvSrtpHistoryDelete(thisPtr, context);
		context = (RvSrtpContext *)RvObjSListGetNext(&thisPtr->contextSList, NULL, RV_OBJSLIST_LEAVE);
		if(context == NULL)
			break;
	}

	return RV_TRUE;
}

RvSrtpContext *rvSrtpHistoryAdd(RvSrtpHistory *thisPtr, RvUint64 startIndex, RvSrtpKey *masterKey, RvBool *contextAdded)
{
	RvUint64 distance;
	RvSrtpContext *context;
	RvSrtpContext *newContext;

	*contextAdded = RV_FALSE;

	/* Check for startIndex in range */
	if(startIndex > thisPtr->wrapIndex)
		return NULL;

	/* First, insure new startIndex is not in history window */
	distance = (thisPtr->maxIndex - startIndex) & thisPtr->wrapIndex;
	if(distance <= thisPtr->historySize)
		return NULL; /* In history window, inserting new contexts not allowed */

	/* Get a new context object and initialize items we need */
	newContext = rvSrtpContextPoolNew(thisPtr->contextPool);
	if(newContext == NULL)
		return NULL; /* Ran out of contexts */
	rvSrtpContextSetFromIndex(newContext, startIndex);
	rvSrtpContextSetMasterKey(newContext, NULL);

	/* Add the new context into the history list */
	if(RvObjSListAdd(&thisPtr->contextSList, newContext) == NULL) {
		rvSrtpContextPoolRelease(thisPtr->contextPool, newContext);
		return NULL; /* list is failing - should never happen */
	}

	/* Merge: Check previous context */
	context = (RvSrtpContext *)RvObjSListGetPrevious(&thisPtr->contextSList, newContext, RV_OBJSLIST_LEAVE);
	if(context != NULL) {
		/* If the previous context has the same master key, we don't need the new one */
		if(rvSrtpContextGetMasterKey(context) == masterKey) {
			rvSrtpHistoryDelete(thisPtr, newContext);
			return context; /* Return the existing one */
		}

		/* If the previous context has the same fromIndex, we are replacing it */
		if(rvSrtpContextGetFromIndex(context) == startIndex)
			rvSrtpHistoryDelete(thisPtr, context);
	} else {
		/* Start of list, if new context is a NULL context, we don't need it */
		if(masterKey == NULL) {
			rvSrtpHistoryDelete(thisPtr, newContext);
			return NULL; /* Nothing was added */
		}
	}

	/* Merge: Check next context */
	context = (RvSrtpContext *)RvObjSListGetNext(&thisPtr->contextSList, newContext, RV_OBJSLIST_LEAVE);
	if(context != NULL) {
		/* If the next context has the same fromIndex or master key, we don't need it anymore */
		if((rvSrtpContextGetFromIndex(context) == startIndex) || (rvSrtpContextGetMasterKey(context) == masterKey))
			rvSrtpHistoryDelete(thisPtr, context);
	}

	/* This new context is being added so create the master key link */
	if(masterKey != NULL) {
		if(RvObjListInsertAfter(rvSrtpKeyGetContextList(masterKey), NULL, newContext) == NULL)
			return NULL; /* We can't undo, just leave the NULL context and return */
		rvSrtpContextSetMasterKey(newContext, masterKey);
	}

	*contextAdded = RV_TRUE;
	return newContext;
}

RvBool rvSrtpHistoryRemove(RvSrtpHistory *thisPtr, RvSrtpContext *contextPtr)
{
	RvSrtpContext *tmpContext;

	/* First, remove the context from it's master key to make it a NULL context */
	RvObjListRemoveItem(rvSrtpKeyGetContextList(rvSrtpContextGetMasterKey(contextPtr)), contextPtr);
	rvSrtpContextSetMasterKey(contextPtr, NULL);

	/* Merge: remove any NULL contexts that follow this context */
	for(;;) {
		tmpContext = (RvSrtpContext *)RvObjSListGetNext(&thisPtr->contextSList, contextPtr, RV_OBJSLIST_LEAVE);
		if((tmpContext == NULL) || (rvSrtpContextGetMasterKey(tmpContext) != NULL))
			break;
		rvSrtpHistoryDelete(thisPtr, tmpContext);
	}

	/* Merge: if the previous context is NULL (or there is none), we can delete this one */
	tmpContext = (RvSrtpContext *)RvObjSListGetPrevious(&thisPtr->contextSList, contextPtr, RV_OBJSLIST_LEAVE);
	if((tmpContext != NULL) && (rvSrtpContextGetMasterKey(tmpContext) == NULL))
		rvSrtpHistoryDelete(thisPtr, contextPtr);

	return RV_TRUE;
}

RvSrtpContext *rvSrtpHistoryFind(RvSrtpHistory *thisPtr, RvUint64 index)
{
	RvSrtpContext *context;
	RvSrtpContext tmpContext;
	RvUint64 distance;

	rvSrtpContextSetFromIndex(&tmpContext, index); /* Only fromIndex is needed for find operation */

	/* Find matching context or context with next higher index */
	context = (RvSrtpContext *)RvObjSListFindPosition(&thisPtr->contextSList, &tmpContext);

	/* NULL indicates end of list (or empty) so the last context applies */
	if(context == NULL)
		return (RvSrtpContext *)RvObjSListGetPrevious(&thisPtr->contextSList, context, RV_OBJSLIST_LEAVE);

	/* Exact match, just return it */
	if(rvSrtpContextGetFromIndex(context) == index)
		return context;

	/* We want the previous context because that what applies to the requested index */
	context = (RvSrtpContext *)RvObjSListGetPrevious(&thisPtr->contextSList, context, RV_OBJSLIST_LEAVE);

	if(context != NULL)
		return context; /* Previous context applies */

	/* We are at the beginning of the list, we have to check the history window */
	distance = (thisPtr->maxIndex - index) & thisPtr->wrapIndex;
	if(distance <= thisPtr->historySize)
			return NULL; /* In history window */

	/* Not in history window, so we are really at the end and the last context applies */
	return (RvSrtpContext *)RvObjSListGetPrevious(&thisPtr->contextSList, NULL, RV_OBJSLIST_LEAVE);
}

RvBool rvSrtpHistoryCheckIndex(RvSrtpHistory *thisPtr, RvUint64 index, RvSrtpContext *contextPtr)
{
	RvSrtpContext *nextContext;
	RvUint64 historyStart;
	RvUint64 testIndex;
	RvUint64 contextStart;
	RvUint64 contextEnd;

	/* Get next context to find where this one stops */
	nextContext = (RvSrtpContext *)RvObjSListGetNext(&thisPtr->contextSList, contextPtr, RV_OBJSLIST_LEAVE);
	
	/* Calucate indexes relative to the history starting point */
	historyStart = (thisPtr->maxIndex - thisPtr->historySize) & thisPtr->wrapIndex;
	testIndex = (index - historyStart) & thisPtr->wrapIndex;
	contextStart = (rvSrtpContextGetFromIndex(contextPtr) - historyStart) & thisPtr->wrapIndex;
	if(nextContext != NULL) {
		contextEnd = (rvSrtpContextGetFromIndex(nextContext) - historyStart) & thisPtr->wrapIndex;
	} else contextEnd = thisPtr->wrapIndex; /* Last context goes to the end */

	/* Now check the range */
	if((testIndex >= contextStart) && (testIndex < contextEnd))
		return RV_TRUE;

	return RV_FALSE;
}

/* Remove a context from a history and delete it */
static void rvSrtpHistoryDelete(RvSrtpHistory *thisPtr, RvSrtpContext *contextPtr)
{
	RvObjSListRemoveItem(&thisPtr->contextSList, contextPtr);
	if(rvSrtpContextGetMasterKey(contextPtr) != NULL)
		RvObjListRemoveItem(rvSrtpKeyGetContextList(rvSrtpContextGetMasterKey(contextPtr)), contextPtr);
	rvSrtpContextPoolRelease(thisPtr->contextPool, contextPtr);
}

/* SList sorting function - cronological, based on context's index and stream's maxIndex. */
static RvInt rvSrtpHistorySListCmp (void *ptr1, void *ptr2, void *data)
{
	RvSrtpHistory *thisPtr;
	RvSrtpContext *data1;
	RvSrtpContext *data2;
	RvUint64 historyStart;
	RvUint64 data1Index;
	RvUint64 data2Index;

	thisPtr = (RvSrtpHistory *)data;
	data1 = (RvSrtpContext *)ptr1;
	data2 = (RvSrtpContext *)ptr2;

	/* Calucate indexes relative to the history starting point */
	historyStart = (thisPtr->maxIndex - thisPtr->historySize) & thisPtr->wrapIndex;
	data1Index = (rvSrtpContextGetFromIndex(data1) - historyStart) & thisPtr->wrapIndex;
	data2Index = (rvSrtpContextGetFromIndex(data2) - historyStart) & thisPtr->wrapIndex;

	if(data1Index < data2Index)
		return -1;
	if(data1Index > data2Index)
		return 1;
	return 0;
}

#if defined(RV_TEST_CODE)
RvRtpStatus RvSrtpContextTest ()
{
}

#endif /* RV_TEST_CODE */
