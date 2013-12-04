/***********************************************************************
Filename   : rvobjhas.c
Description: utility for building a hash table of objects (structures)
************************************************************************
        Copyright (c) 2005 RADVISION Inc. and RADVISION Ltd.
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
#include "rvobjhash.h"
#include <string.h>
#define RV_OBJHASH_PRIMETABLE_SIZE 29 /* Number of non-zero values in RvObjHashPrimeSizes */
static const RvSize_t RvObjHashPrimeSizes[] = { /* First and last MUST be 0 */
	RvUintConst(0),
	RvUintConst(17),		RvUintConst(53),		RvUintConst(97),		RvUintConst(193),
	RvUintConst(389),		RvUintConst(769),		RvUintConst(1543),		RvUintConst(3079),
	RvUintConst(6151),      RvUintConst(12289),		RvUintConst(24593),		RvUintConst(49157),
	RvUintConst(98317),		RvUintConst(196613),    RvUintConst(393241),	RvUintConst(786433),
	RvUintConst(1572869),	RvUintConst(3145739),   RvUintConst(6291469),   RvUintConst(12582917),
	RvUintConst(25165843),	RvUintConst(50331653),  RvUintConst(100663319), RvUintConst(201326611),
	RvUintConst(402653189),	RvUintConst(805306457),	RvUintConst(1610612741),RvUintConst(3221225473),
	RvUintConst(4294967291),
	RvUintConst(0)
};

static RvSize_t RvObjHashNewSize(RvObjHash *objhash, RvSize_t newsize);

/* The callbacks structure must be completely filled in. A duplicate */
/* of the callbacks structure will be made so that the user doesn't have */
/* to keep their own copy of that structure. Note that the memalloc callback */
/* will be called to allocate the starting array. See header file for htype */
/* options. */
RvObjHash *RvObjHashConstruct(RvObjHash *objhash, void *itemtmp, RvObjHashElement *elementptr, RvInt htype, RvSize_t startsize, RvObjHashFuncs *callbacks)
{
	RvSize_t i;

#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if((objhash == NULL) || (itemtmp == NULL) || (elementptr == NULL) || (callbacks == NULL) ||
	   (callbacks->memalloc == NULL) || (callbacks->memfree == NULL) ||
	   (callbacks->itemcmp == NULL) || (callbacks->gethash == NULL))
		return NULL;
#endif
#if (RV_CHECK_MASK & RV_CHECK_RANGE)
	if((htype != RV_OBJHASH_TYPE_FIXED) && (htype != RV_OBJHASH_TYPE_EXPANDING) &&
	   (htype != RV_OBJHASH_TYPE_DYNAMIC))
		return NULL;
#endif

	if(startsize < 2)
		startsize = 2;
	objhash->tabletype = htype;
	objhash->startsize = startsize;
	objhash->cursize = startsize;
	objhash->numitems = 0;
	objhash->shrinksize = 0; /* We can't shrink until we expand so just clear it for now */
	objhash->offset = (RvInt8 *)elementptr - (RvInt8 *)itemtmp;
	objhash->itemtmp = itemtmp;
	objhash->eptr = (RvObjListElement *)elementptr;

	/* Copy callbacks */
	memcpy(&objhash->callbacks, callbacks, sizeof(*callbacks));

	/* Allocate starting array */
	objhash->buckets = NULL;
	objhash->buckets = (RvObjList *)callbacks->memalloc((RvSize_t)&objhash->buckets[startsize], callbacks->memallocdata);
	if(objhash->buckets == NULL)
		return NULL;
	for(i = 0; i < startsize; i++)
		RvObjListConstruct(itemtmp, (RvObjListElement *)elementptr, &objhash->buckets[i]);

	return objhash;
}

/* Any items still in the table are assumed to be removed. */
void RvObjHashDestruct(RvObjHash *objhash)
{
	RvSize_t i;

#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if(objhash == NULL)
		return;
#endif

	for(i = 0; i < objhash->cursize; i++)
		RvObjListDestruct(&objhash->buckets[i]);
	objhash->callbacks.memfree(objhash->buckets, objhash->callbacks.memfreedata);
}

void *RvObjHashAdd(RvObjHash *objhash, void *newitem)
{
	RvSize_t newsize, bucket;
	RvObjHashElement *elem;
	void *result;
	RvInt i;

#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if((objhash == NULL) || (newitem == NULL))
		return NULL;
#endif

	/* Deal with running out of space in the table array. */
	if(objhash->numitems >= objhash->cursize) {
		if(objhash->tabletype == RV_OBJHASH_TYPE_FIXED)
			return NULL; /* Fixed size, just return error */

		/* Calculate new size from prime table and change the size. */
		newsize = objhash->cursize * 2;
		i = 1;
		while(RvObjHashPrimeSizes[i] != 0) {
			if(newsize <= RvObjHashPrimeSizes[i]) {
				newsize = RvObjHashPrimeSizes[i];
				break;
			}
			i++;
		}
		if(RvObjHashNewSize(objhash, newsize) != newsize)
			return NULL; /* No more space available */
	}

	elem = (RvObjHashElement *)((RvInt8 *)newitem + objhash->offset);
	elem->hashval = objhash->callbacks.gethash(newitem, objhash->callbacks.gethashdata);
	bucket = elem->hashval % objhash->cursize;

	result = RvObjListInsertBefore(&objhash->buckets[bucket], NULL, newitem);
	if(result != NULL)
		objhash->numitems += 1;
	
	return result;
}

void *RvObjHashAddUnique(RvObjHash *objhash, void *newitem)
{
	void *result;

	/* Just call find then add since it doesn't incur much overhead */
	result = RvObjHashFind(objhash, newitem);
	if (result != NULL)
		return result; /* Object with same key already exists, return it */

	return RvObjHashAdd(objhash, newitem);
}

void *RvObjHashFind(RvObjHash *objhash, void *keyitem)
{
	void *result;
	RvObjHashElement *elem;
	RvSize_t hashval, bucket;

#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if((objhash == NULL) || (keyitem == NULL))
		return NULL;
#endif

	hashval = objhash->callbacks.gethash(keyitem, objhash->callbacks.gethashdata);
	bucket = hashval % objhash->cursize;
	result = NULL;
	do {
		result = RvObjListGetNext(&objhash->buckets[bucket], result, RV_OBJLIST_LEAVE);
		if(result == NULL)
			return NULL; /* End of bucket. */
		elem = (RvObjHashElement *)((RvInt8 *)result + objhash->offset);
	} while ((elem->hashval != hashval) || (objhash->callbacks.itemcmp(keyitem, result, objhash->callbacks.itemcmpdata) != 0));

	return result;
}

void *RvObjHashFindNext(RvObjHash *objhash, void *item)
{
	void *result;
	RvObjHashElement *elem, *keyelem;
	RvSize_t bucket;

#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if((objhash == NULL) || (item == NULL))
		return NULL;
#endif

	keyelem = (RvObjHashElement *)((RvInt8 *)item + objhash->offset);
	bucket = keyelem->hashval % objhash->cursize;
	result = item;
	do {
		result = RvObjListGetNext(&objhash->buckets[bucket], result, RV_OBJLIST_LEAVE);
		if(result == NULL)
			return NULL; /* End of bucket. */
		elem = (RvObjHashElement *)((RvInt8 *)result + objhash->offset);
	} while((elem->hashval != keyelem->hashval) || (objhash->callbacks.itemcmp(item, result, objhash->callbacks.itemcmpdata) != 0));

	return result;
}

void *RvObjHashGetNext(RvObjHash *objhash, void *item)
{
	void *result;
	RvSize_t bucket;
	RvObjHashElement *elem;

#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if(objhash == NULL)
		return NULL;
#endif

	if(item != NULL) {
		elem = (RvObjHashElement *)((RvInt8 *)item + objhash->offset);
		bucket = elem->hashval % objhash->cursize;
	} else bucket = 0;
	result = RvObjListGetNext(&objhash->buckets[bucket], item, RV_OBJLIST_LEAVE);

	while(result == NULL) {
		/* Search through buckets until we find something. */
		bucket += 1;
		if(bucket >= objhash->cursize)
			break; /* We ran out of buckets. */
		result = RvObjListGetNext(&objhash->buckets[bucket], NULL, RV_OBJLIST_LEAVE);
	}

	return result;
}

void *RvObjHashGetPrevious(RvObjHash *objhash, void *item)
{
	void *result;
	RvSize_t bucket;
	RvObjHashElement *elem;

#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if(objhash == NULL)
		return NULL;
#endif

	if(item != NULL) {
		elem = (RvObjHashElement *)((RvInt8 *)item + objhash->offset);
		bucket = elem->hashval % objhash->cursize;
	} else bucket = objhash->cursize - 1;
	result = RvObjListGetPrevious(&objhash->buckets[bucket], item, RV_OBJLIST_LEAVE);

	while(result == NULL) {
		/* Search through buckets until we find something. */
		if(bucket == 0)
			break; /* We ran out of buckets. */
		bucket -= 1;
		result = RvObjListGetPrevious(&objhash->buckets[bucket], NULL, RV_OBJLIST_LEAVE);
	}

	return result;
}

RvSize_t RvObjHashNumItems(RvObjHash *objhash)
{
#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if(objhash == NULL)
		return 0;
#endif

	return objhash->numitems;
}

RvSize_t RvObjHashSize(RvObjHash *objhash)
{
#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if(objhash == NULL)
		return 0;
#endif

	return objhash->cursize;
}

/* Force a change of the hash table size. Can not change */
/* the size smaller than the number of items in it. DYNAMIC */
/* tables will not reduce below this value (resets startsize) */
/* Returns new size (or old size if it couldn't be changed. */
RvSize_t RvObjHashChangeSize(RvObjHash *objhash, RvSize_t newsize)
{
	RvSize_t result;

#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if(objhash == NULL)
		return 0;
#endif

	if((newsize < 2) || (newsize < objhash->numitems))
		return objhash->cursize;

	result = RvObjHashNewSize(objhash, newsize);
	if(result == newsize) {
		objhash->startsize = newsize; /* DYNAMIC hard floor */
		objhash->shrinksize = 0; /* No shrinking until we expand again. */
	}

	return result;
}

void RvObjHashClear(RvObjHash *objhash)
{
	RvSize_t bucket;

#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if(objhash == NULL)
		return;
#endif

	/* Remove existing items. */
	for(bucket = 0; bucket < objhash->cursize; bucket++) {
		while(RvObjListGetNext(&objhash->buckets[bucket], NULL, RV_OBJLIST_REMOVE) != NULL);
	}
	objhash->numitems = 0;

	/* If shrinking is allowed, shrink back to starting size. */
	if((objhash->tabletype == RV_OBJHASH_TYPE_DYNAMIC) && (objhash->cursize != objhash->startsize))
		RvObjHashNewSize(objhash, objhash->startsize);
}

/* Returns the item removed. */
void *RvObjHashRemove(RvObjHash *objhash, void *item)
{
	void *result;
	RvObjHashElement *elem;
	RvSize_t bucket;

#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if((objhash == NULL) || (item == NULL))
		return NULL;
#endif

	elem = (RvObjHashElement *)((RvInt8 *)item + objhash->offset);
	bucket = elem->hashval % objhash->cursize;
	result = RvObjListRemoveItem(&objhash->buckets[bucket], item);
	if(result == NULL)
		return result;

	objhash->numitems -= 1;

	/* If shrinking is allowed, do it when required. */
	if((objhash->tabletype == RV_OBJHASH_TYPE_DYNAMIC) && (objhash->numitems < (objhash->shrinksize / 2)))
		RvObjHashNewSize(objhash, objhash->shrinksize);

	return result;
}

/* Change the hash table to newsize. Returns the new size */
/* (or old one if it couldn't do it). Doesn't do any error */
/* checking so make sure newsize is valid. */
static RvSize_t RvObjHashNewSize(RvObjHash *objhash, RvSize_t newsize)
{
	RvObjList *newbuckets;
	RvInt i;
	RvSize_t bucket;
	void *item;
	RvObjHashElement *elem;

	/* Create new hash buckets. */
	newbuckets = NULL;
	newbuckets = objhash->callbacks.memalloc((RvSize_t)&newbuckets[newsize], objhash->callbacks.memallocdata);
	if(newbuckets == NULL)
		return objhash->cursize; /* No memory available so don't bother. */
	for(bucket = 0; bucket < newsize; bucket++)
		RvObjListConstruct(objhash->itemtmp, objhash->eptr, &newbuckets[bucket]);

	/* Move existing items to new buckets. */
	for(bucket = 0; bucket < objhash->cursize; bucket++) {
		for(;;) {
			item = RvObjListGetNext(&objhash->buckets[bucket], NULL, RV_OBJLIST_REMOVE);
			if(item == NULL)
				break; /* End of bucket */
			elem = (RvObjHashElement *)((RvInt8 *)item + objhash->offset);
			RvObjListInsertBefore(&newbuckets[elem->hashval % newsize], NULL, item);
		}
	}

	/* Get rid of old memory and set table. */
	for(bucket = 0; bucket < objhash->cursize; bucket++)
		RvObjListDestruct(&objhash->buckets[bucket]);
	objhash->callbacks.memfree(objhash->buckets, objhash->callbacks.memfreedata);
	objhash->cursize = newsize;
	objhash->buckets = newbuckets;

	/* For DYNAMIC tables, calculate shrinksize */
	if(objhash->tabletype == RV_OBJHASH_TYPE_DYNAMIC) {
		objhash->shrinksize = newsize / 2;
		i = RV_OBJHASH_PRIMETABLE_SIZE;
		while(RvObjHashPrimeSizes[i] != 0) {
			if(objhash->shrinksize >= RvObjHashPrimeSizes[i])
				break;
			i--;
		}
		objhash->shrinksize = RvObjHashPrimeSizes[i];
		if(objhash->shrinksize <= objhash->startsize)
			objhash->shrinksize = 0; /* Don't shrink below startsize. */
	}

	return objhash->cursize;
}


#if defined(RV_TEST_CODE)
#include "rvstdio.h"
#include <stdlib.h>
typedef struct {
	RvInt32 dummy1;
	RvInt64 dummy2;
	RvObjHashElement elem;
	RvChar dummy3[80];
	RvInt index;
	RvBool intable;
	RvSize_t value;
} RvObjHashTestStruct;

static volatile int callbackprint = 0;
static volatile int memcallbackprint = 0;

/* Set GETMEMORY & FREEMEMORY to function that will allocate memory */
#if (RV_OS_TYPE == RV_OS_TYPE_NUCLEUS) /* Shouldn't use this here but its a test */
#define FREEMEMORY(_x) NU_Deallocate_Memory((_x))
#define GETMEMORY(_x) NU_getmemory((_x))
#include <nucleus.h>
extern NU_MEMORY_POOL System_Memory;
void *NU_getmemory(RvSize_t n)
{
	int status;
	void *ptr;

	status = NU_Allocate_Memory(&System_Memory, &ptr, n, NU_NO_SUSPEND);
	if(status != NU_SUCCESS) return NULL;
	return ptr;
}
#elif (RV_OS_TYPE == RV_OS_TYPE_OSE) /* Shouldn't use this here but its a test */
#define GETMEMORY(_x) heap_alloc_shared((_x), (__FILE__), (__LINE__))
#define FREEMEMORY(_x) heap_free_shared((_x))
#include "ose.h"
#include "heapapi.h"
#else
#define GETMEMORY(_x) malloc((_x))
#define FREEMEMORY(_x) free((_x))
#endif

static void *RvObjHashTestMemAlloc(RvSize_t size, void *data)
{
	void *result;

	result = (void *)GETMEMORY(size);
	if(memcallbackprint != 0)
		RvPrintf("Memory Allocation (Size = %u, data = %p) = %p\n", size, data, result);
	return result;
}

static void RvObjHashTestMemFree(void *ptr, void *data)
{
	if(memcallbackprint != 0)
		RvPrintf("Memory Free (Ptr = %p, data = %p)\n", ptr, data);
	FREEMEMORY(ptr);
}

static RvInt RvObjHashTestItemCmp(void *ptr1, void *ptr2, void *data)
{
	RvObjHashTestStruct *item1, *item2;

	item1 = (RvObjHashTestStruct *)ptr1;
	item2 = (RvObjHashTestStruct *)ptr2;
	if(callbackprint != 0)
		RvPrintf("ItemCmp: Ptr1(%p) = %u, Ptr2(%p) = %u, data = %p\n", ptr1, item1->value, ptr2, item2->value, data);
	if(item1->value == item2->value)
		return 0;
	if(item1->value < item2->value)
		return -1;
	return 1;
}

static RvSize_t RvObjHashTestGetHash(void *item, void *data)
{
	RvObjHashTestStruct *ptr;

	ptr = (RvObjHashTestStruct *)item;
	if(callbackprint != 0)
		RvPrintf("GetHash: Item(%p) = %u, data = %p\n", item, ptr->value, data);
	return ptr->value;
}

static void RvObjHashTestHash(RvInt tabletype, RvSize_t startsize)
{
	RvObjHashFuncs callbacks;
	RvObjHash objhash, *objhashresult;
	RvObjHashTestStruct *item, *item2, newitem, newitem2, keyitem;
	RvSize_t arraysize, i, newsize, index;
	RvObjHashTestStruct *itemarray;

	RvPrintf("++++Testing Hash Table ++++\n");
	
	callbacks.memalloc = RvObjHashTestMemAlloc;
	callbacks.memfree = RvObjHashTestMemFree;
	callbacks.itemcmp = RvObjHashTestItemCmp;
	callbacks.gethash = RvObjHashTestGetHash;
	callbacks.memallocdata = NULL;
	callbacks.memfreedata = NULL;
	callbacks.itemcmpdata = NULL;
	callbacks.gethashdata = NULL;
	RvPrintf("Constructing with startsize * 2 (%u)...\n", (startsize * 2));
	objhashresult = RvObjHashConstruct(&objhash, &keyitem, &keyitem.elem, tabletype, (startsize * 2), &callbacks);
	if(objhashresult == NULL) {
		RvPrintf("RvObjHashConstruct: ERROR\n");
		RvPrintf("++++Object Hash Table Test Aborted++++\n");
		return;
	} else RvPrintf("RvObjHashConstruct: OK\n");
	RvPrintf("RvObjHashNumItems = %u, RvObjHashSize = %u\n", RvObjHashNumItems(&objhash), RvObjHashSize(&objhash));

	RvPrintf("RvObjHashGetNext: ");
	item = (RvObjHashTestStruct *)RvObjHashGetNext(&objhash, NULL);
	if(item == NULL) {
		RvPrintf("OK, nothing in table.\n");
	} else RvPrintf("ERROR!\n");

	RvPrintf("RvObjHashGetPrevious: ");
	item = (RvObjHashTestStruct *)RvObjHashGetPrevious(&objhash, NULL);
	if(item == NULL) {
		RvPrintf("OK, nothing in table.\n");
	} else RvPrintf("ERROR!\n");

	RvPrintf("RvObjHashAddUnique...\n");
	newitem.value = 1;
	item = (RvObjHashTestStruct *)RvObjHashAddUnique(&objhash, &newitem);
	if(item == &newitem) {
		RvPrintf("  RvObjHashAddUnique OK.\n");
	} else RvPrintf("  RvObjHashAddUnique ERROR!\n");
	RvPrintf("RvObjHashNumItems = %u, RvObjHashSize = %u\n", RvObjHashNumItems(&objhash), RvObjHashSize(&objhash));

	RvPrintf("RvObjHashAddUnique...\n");
	newitem2.value = 1;
	item = (RvObjHashTestStruct *)RvObjHashAddUnique(&objhash, &newitem2);
	if(item == &newitem) {
		RvPrintf("  RvObjHashAddUnique OK (rejected same key).\n");
	} else RvPrintf("  RvObjHashAddUnique ERROR!\n");
	RvPrintf("RvObjHashNumItems = %u, RvObjHashSize = %u\n", RvObjHashNumItems(&objhash), RvObjHashSize(&objhash));

	/* Get 2 of the same value in the same bucket. */
	RvPrintf("RvObjHashAdd...\n");
	newitem2.value = 1;
	item = (RvObjHashTestStruct *)RvObjHashAdd(&objhash, &newitem2);
	if(item == &newitem2) {
		RvPrintf("  RvObjHashAdd OK.\n");
	} else RvPrintf("  RvObjHashAdd ERROR!\n");
	RvPrintf("RvObjHashNumItems = %u, RvObjHashSize = %u\n", RvObjHashNumItems(&objhash), RvObjHashSize(&objhash));

	RvPrintf("RvObjHashChangeSize(%u)...\n", (startsize / 4));
	newsize = RvObjHashChangeSize(&objhash, (startsize / 4));
	if(newsize != (startsize / 4)) {
		RvPrintf("  RvObjHashChangeSize ERROR, size is %u.\n", newsize);
	} else RvPrintf("  RvObjHashChangeSize OK.\n");
	RvPrintf("RvObjHashNumItems = %u, RvObjHashSize = %u\n", RvObjHashNumItems(&objhash), RvObjHashSize(&objhash));
	
	RvPrintf("RvObjHashFind...\n");
	keyitem.value = 1;
	item = (RvObjHashTestStruct *)RvObjHashFind(&objhash, &keyitem);
	if(item == &newitem) {
		RvPrintf("  RvObjHashFind OK.\n");
	} else RvPrintf("  RvObjHashFind ERROR!\n");

	RvPrintf("RvObjHashFindNext...\n");
	item = (RvObjHashTestStruct *)RvObjHashFindNext(&objhash, item);
	if(item == &newitem2) {
		RvPrintf("  RvObjHashFindNext OK.\n");
	} else RvPrintf("  RvObjHashFindNext ERROR!\n");
	RvPrintf("RvObjHashNumItems = %u, RvObjHashSize = %u\n", RvObjHashNumItems(&objhash), RvObjHashSize(&objhash));

	RvPrintf("RvObjHashFindNext: ");
	item = (RvObjHashTestStruct *)RvObjHashFindNext(&objhash, item);
	if(item == NULL) {
		RvPrintf("OK.\n");
	} else RvPrintf("ERROR!\n");
	RvPrintf("RvObjHashNumItems = %u, RvObjHashSize = %u\n", RvObjHashNumItems(&objhash), RvObjHashSize(&objhash));

	RvPrintf("RvObjHashGetPrevious: ");
	item = (RvObjHashTestStruct *)RvObjHashGetPrevious(&objhash, &newitem2);
	if(item == &newitem) {
		RvPrintf("OK.\n");
	} else RvPrintf("ERROR!\n");

	RvPrintf("RvObjHashGetNext: ");
	item = (RvObjHashTestStruct *)RvObjHashGetNext(&objhash, item);
	if(item == &newitem2) {
		RvPrintf("OK.\n");
	} else RvPrintf("ERROR!\n");

	RvPrintf("RvObjHashGetPrevious (NULL): ");
	item = (RvObjHashTestStruct *)RvObjHashGetPrevious(&objhash, NULL);
	if(item == &newitem2) {
		RvPrintf("OK.\n");
	} else RvPrintf("ERROR!\n");

	RvPrintf("RvObjHashChangeSize(%u)...\n", startsize);
	newsize = RvObjHashChangeSize(&objhash, startsize);
	if(newsize != startsize) {
		RvPrintf(" RvObjHashChangeSize ERROR, size is %u.\n", newsize);
	} else RvPrintf("  RvObjHashChangeSize OK.\n");
	RvPrintf("RvObjHashNumItems = %u, RvObjHashSize = %u\n", RvObjHashNumItems(&objhash), RvObjHashSize(&objhash));

	RvPrintf("RvObjhashClear.\n");
	RvObjHashClear(&objhash);
	RvPrintf("RvObjHashNumItems = %u, RvObjHashSize = %u\n", RvObjHashNumItems(&objhash), RvObjHashSize(&objhash));

	arraysize = startsize * 3;
	itemarray = NULL;
	itemarray = (RvObjHashTestStruct *)GETMEMORY((RvSize_t)((RvInt8 *)&itemarray[arraysize] - (RvInt8 *)&itemarray[0]));
	for(i = 0; i < arraysize; i++) {
		itemarray[i].value = (RvSize_t)rand() * (RvSize_t)rand();
		itemarray[i].index = i;
		itemarray[i].intable = RV_FALSE;
	}

	RvPrintf("Adding %d items to Hash Table...\n", arraysize);
	for(i = 0; i < 	arraysize; i++) {
		if(RvObjHashAdd(&objhash, &itemarray[i]) != NULL) {
			itemarray[i].intable = RV_TRUE;
		} else RvPrintf("ERROR! Bad Add: #%d\n", i);
	}
	RvPrintf("RvObjHashNumItems = %u, RvObjHashSize = %u\n", RvObjHashNumItems(&objhash), RvObjHashSize(&objhash));

	for(i = 0; i < 	(arraysize / 4); i++) {
		index = (RvSize_t)rand() % arraysize;
		keyitem.value = itemarray[index].value;
		RvPrintf("Finding object #%d in the table...\n", index);
		item = (RvObjHashTestStruct *)RvObjHashFind(&objhash, &keyitem);
		if(item == &itemarray[index]) {
			RvPrintf("  Found.\n");
			RvPrintf("Removing object #%d from the table: ", index);
			item2 = (RvObjHashTestStruct *)RvObjHashRemove(&objhash, item);
			if(item2 != NULL) {
				item2->intable = RV_FALSE;
				RvPrintf("OK\n");
			} else RvPrintf("ERROR.\n");
		} else {
			if(itemarray[index].intable == RV_TRUE) {
				RvPrintf("  ERROR, not found.\n");
			} else RvPrintf("  Not Found: OK.\n");
		}
	}

	RvPrintf("Dumping hash table...\n");
	index = 0;
	item = NULL;
	for(;;) {
		item = (RvObjHashTestStruct *)RvObjHashGetNext(&objhash, item);
		if(item == NULL)
			break;
		RvPrintf("  #%d: Item %d, value = %u, bucket = %u)\n", index, item->index, item->value, item->value % RvObjHashSize(&objhash));
		index += 1;
	}
	RvPrintf("RvObjHashNumItems = %u, RvObjHashSize = %u\n", RvObjHashNumItems(&objhash), RvObjHashSize(&objhash));

	if(tabletype == RV_OBJHASH_TYPE_DYNAMIC) {
		RvPrintf("Checking DYNAMIC reduction by removing items...\n");
		for(i = 0; i < 	arraysize; i++) {
			if(itemarray[i].intable == RV_TRUE) {
				item = (RvObjHashTestStruct *)RvObjHashRemove(&objhash, &itemarray[i]);
				if(item != NULL)
					RvPrintf("  Removed item #%d, RvObjHashNumItems = %u, RvObjHashSize = %u\n", item->index, RvObjHashNumItems(&objhash), RvObjHashSize(&objhash));
			}
		}
	}
	
	RvPrintf("RvObjHashDestruct...\n");
	RvObjHashDestruct(&objhash);

	FREEMEMORY(itemarray);
	RvPrintf("++++Object Hash Table test completed++++\n");
}

void RvObjHashTest(void)
{
	RvPrintf("Starting test of rvobjhash.\n");

	callbackprint = 1;
	memcallbackprint = 1;

	RvPrintf("Testing ObjHash Type: FIXED (15)\n");
	RvObjHashTestHash(RV_OBJHASH_TYPE_FIXED, 15);

	callbackprint = 0;

	RvPrintf("Testing ObjHash Type: EXPANDING (100)\n");
	RvObjHashTestHash(RV_OBJHASH_TYPE_EXPANDING, 100);

	RvPrintf("Testing ObjHash Type: DYNAMIC (100)\n");
	RvObjHashTestHash(RV_OBJHASH_TYPE_DYNAMIC, 100);

	RvPrintf("Testing ObjHash Type: DYNAMIC (3) ... size reduction should fail.\n");
	RvObjHashTestHash(RV_OBJHASH_TYPE_DYNAMIC, 3);

	memcallbackprint = 0;
}

#endif /* RV_TEST_CODE */
