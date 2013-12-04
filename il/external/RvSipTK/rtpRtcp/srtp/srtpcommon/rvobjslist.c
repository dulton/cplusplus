/***********************************************************************
Filename   : rvobjslist.c
Description: utility for building sorted lists of objects (structures)
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
#include "stdlib.h"
#include "rvobjslist.h"

/* Basic list utility for structures. User is responsible for locking */
/* if list is to be shared. */

/* A sorted list is simply a regular list with a sort function. */
RvObjSList *RvObjSListConstruct(RvObjSList *objslist, void *itemtemp, RvObjSListElement *elementptr, RvObjSListFunc itemcmp, void *data)
{
#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if((objslist == NULL) || (itemtemp == NULL) || (elementptr == NULL) || itemcmp == NULL)
		return NULL;
#endif

	objslist->itemcmp = itemcmp;
	objslist->data = data;
	objslist->itemtemp = itemtemp;                         /* Only used by RvObjSlistClear. */
	objslist->elementptr = (RvObjListElement *)elementptr; /* Only used by RvObjSlistClear. */
	if(RvObjListConstruct(itemtemp, elementptr, &objslist->itemlist) == NULL)
		return NULL;
	return objslist;
}

void RvObjSListDestruct(RvObjSList *objslist)
{
#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if(objslist == NULL)
		return;
#endif
	RvObjListDestruct(&objslist->itemlist);
}

RvSize_t RvObjSListSize(RvObjSList *objslist)
{
#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if(objslist == NULL)
		return 0;
#endif
	return RvObjListSize(&objslist->itemlist);
}

void *RvObjSListAdd(RvObjSList *objslist, void *newitem)
{
	void *curitem;

#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if((objslist == NULL) || (newitem == NULL))
		return NULL;
#endif

	/* Just compare down the list until we find the spot to insert it. */
	curitem = NULL;
	do {
		curitem = RvObjListGetNext(&objslist->itemlist, curitem, RV_OBJLIST_LEAVE);
	} while((curitem != NULL) && (objslist->itemcmp(newitem, curitem, objslist->data) > 0));
	return RvObjListInsertBefore(&objslist->itemlist, curitem, newitem);
}

void *RvObjSListGetNext(RvObjSList *objslist, void *curitem, RvBool removeitem)
{
#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if(objslist == NULL)
		return NULL;
#endif
	
	return RvObjListGetNext(&objslist->itemlist, curitem, removeitem);
}

void *RvObjSListGetPrevious(RvObjSList *objslist, void *curitem, RvBool removeitem)
{
#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if(objslist == NULL)
		return NULL;
#endif
	return RvObjListGetPrevious(&objslist->itemlist, curitem, removeitem);
}

void *RvObjSListRemoveItem(RvObjSList *objslist, void *item)
{
#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if((objslist == NULL) || (item == NULL))
		return NULL;
#endif
	return RvObjListRemoveItem(&objslist->itemlist, item);
}

void *RvObjSListFind(RvObjSList *objslist, void *keyitem)
{
	void *curitem;
	RvInt result;

#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if((objslist == NULL) || (keyitem == NULL))
		return NULL;
#endif

	/* Just compare down the list until we find it or where it would have been. */
	curitem = NULL;
	do {
		curitem = RvObjListGetNext(&objslist->itemlist, curitem, RV_OBJLIST_LEAVE);
		if(curitem == NULL)
			return NULL; /* End of list */
		result = objslist->itemcmp(keyitem, curitem, objslist->data);
	} while(result > 0);
	if(result != 0)
		return NULL;
	return curitem;
}

void *RvObjSListFindPosition(RvObjSList *objslist, void *keyitem)
{
	void *curitem;

#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if((objslist == NULL) || (keyitem == NULL))
		return NULL;
#endif

	/* Just compare down the list until we find where it would be put. */
	curitem = NULL;
	do {
		curitem = RvObjListGetNext(&objslist->itemlist, curitem, RV_OBJLIST_LEAVE);
	} while((curitem != NULL) && (objslist->itemcmp(keyitem, curitem, objslist->data) > 0));
	return curitem;
}

void RvObjSListClear(RvObjSList *objslist)
{
#if (RV_CHECK_MASK & RV_CHECK_NULL)
	if(objslist == NULL)
		return;
#endif

	/* RvObjList should have a clear function, until then... */
	RvObjListDestruct(&objslist->itemlist);
	RvObjListConstruct(objslist->itemtemp, objslist->elementptr, &objslist->itemlist);
}

#if defined(RV_TEST_CODE)
#include "rvstdio.h"
typedef struct {
	RvInt32 dummy1;
	RvInt64 dummy2;
	RvObjSListElement listElem;
	RvChar dummy3[80];
	RvInt index;
	RvInt32 value;
	RvBool intable;
} RvObjSListTestStruct;

#define RV_OBJSLIST_NUMTESTOBJ 1000
static RvObjSListTestStruct RvObjTestData[RV_OBJSLIST_NUMTESTOBJ];

static RvInt RvObjSListTestItemCmp(void *ptr1, void *ptr2, void *data)
{
	RvObjSListTestStruct *item1, *item2;

	item1 = (RvObjSListTestStruct *)ptr1;
	item2 = (RvObjSListTestStruct *)ptr2;
	if(item1->value == item2->value)
		return 0;
	if(item1->value < item2->value)
		return -1;
	return 1;
}

void RvObjSListTest(void)
{
	RvInt i, index;
	RvObjSList slist, *cresult;
	RvObjSListTestStruct keyitem, *item, *item2;
	RvInt32 lastvalue;

	RvPrintf("Starting test of rvobjslist.\n");
	
	for(i = 0; i < RV_OBJSLIST_NUMTESTOBJ; i++) {
		RvObjTestData[i].index = i;
		RvObjTestData[i].value = (RvInt32)rand() * (RvInt32)rand();
		RvObjTestData[i].intable = RV_FALSE;
	}

	RvPrintf("RvObjSListConstruct: ");
	cresult = RvObjSListConstruct(&slist, &RvObjTestData[0], &RvObjTestData[0].listElem, RvObjSListTestItemCmp, NULL);
	if(cresult != NULL) {
		RvPrintf("OK\n");
	} else RvPrintf("ERROR\n");

	RvPrintf("Adding %d items to sorted list...\n", RV_OBJSLIST_NUMTESTOBJ);
	for(i = 0; i < RV_OBJSLIST_NUMTESTOBJ; i++) {
		item = (RvObjSListTestStruct *)RvObjSListAdd(&slist, &RvObjTestData[i]);
		if(item == &RvObjTestData[i]) {
			RvObjTestData[i].intable = RV_TRUE;
		} else RvPrintf("ERROR adding #%d.\n", i);
	}
	RvPrintf("RvObjSListSize = %u\n", RvObjSListSize(&slist));

	for(i = 0; i < (RV_OBJSLIST_NUMTESTOBJ/10); i++) {
		index = (RvInt)rand() % RV_OBJSLIST_NUMTESTOBJ;
		RvPrintf("Finding object #%d in the list: ", index);
		item = (RvObjSListTestStruct *)RvObjSListFind(&slist, &RvObjTestData[index]);
		if(item == &RvObjTestData[index]) {
			RvPrintf("Found.\n");
			RvPrintf("Removing object #%d from the list: ", index);
			item2 = (RvObjSListTestStruct *)RvObjSListRemoveItem(&slist, item);
			if(item2 == item) {
				item2->intable = RV_FALSE;
				RvPrintf("OK\n");
			} else RvPrintf("ERROR.\n");
		} else {
			if(RvObjTestData[index].intable == RV_TRUE) {
				RvPrintf("ERROR, not found.\n");
			} else RvPrintf("Not Found: OK.\n");
		}
	}
	RvPrintf("RvObjListSize = %u\n", RvObjSListSize(&slist));
		
	for(i = 0; i < (RV_OBJSLIST_NUMTESTOBJ/10); i++) {
		keyitem.value = (RvInt32)rand() * (RvInt32)rand();
		RvPrintf("Finding position for value %d in the list:\n", keyitem.value);
		item = RvObjSListFindPosition(&slist, &keyitem);
		item2 = RvObjSListGetPrevious(&slist, item, RV_OBJSLIST_LEAVE);
		RvPrintf("Between ");
		if(item2 != NULL) {
			RvPrintf("(#%d, %d)", item2->index, item2->value);
		} else RvPrintf("Beginning");
		RvPrintf(" and ");
		if(item != NULL) {
			RvPrintf("(#%d, %d).\n", item->index, item->value);
		} else RvPrintf("End.\n");
	}
	RvPrintf("RvObjSListSize = %u\n", RvObjSListSize(&slist));

	RvPrintf("Dumping sorted list:\n");
	item = NULL;
	lastvalue = 0;
	index = 0;
	for(;;) {
		item = RvObjSListGetNext(&slist, item, RV_OBJSLIST_LEAVE);
		if(item == NULL)
			break;
		RvPrintf(" Got: %d.\n", item->value);
		if(lastvalue > item->value) {
			RvPrintf("ERROR. Sort sequence wrong.\n");
			break;
		}
		lastvalue = item->value;
		index++;
	}
	RvPrintf("Retrieved %d items from sorted table.\n", index);

	RvPrintf("RvObjSListClear...\n");
	RvObjSListClear(&slist);
	RvPrintf("RvObjSListSize = %u\n", RvObjSListSize(&slist));
	
	RvPrintf("Destructing sorted list.\n");
	RvObjSListDestruct(&slist);
}

#endif /* RV_TEST_CODE */
