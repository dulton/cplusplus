/***********************************************************************
Filename   : objslist.h
Description: objslist header file
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
/*$
{package:
	{name: ObjSList}
	{superpackage: CUtils}
	{include: rvobjslist.h}
	{description:	
		{p: This module provides functions for creating and manipulating
			sorted object lists. This uses a simple insert sort and
			assumes that the comparison operation is not an intensive
			operation.}
		{p:	To make a structure usable in a list, an element of type
			RvObjSListElement must be declared in that structure.}
	}
}
$*/
#ifndef RV_OBJSLIST_H
#define RV_OBJSLIST_H

#include "rvtypes.h"
#include "rvobjlist.h" 

/*$
{type:
	{name: RvObjSListElement}
	{superpackage: ObjSList}
	{include: rvobjslist.h}
	{description:	
		{p: An element of this data type must be declared in the
			structure to be put into a sorted list.}
		{p: The information stored in this element is only used while a
			particular object is actually in the list. The information
			stored in that element does not need to be maintained when
			the object is not in the list, even if it is to be returned
			to a list..}
		{p: A structure can be put into multiple lists by simply declaring
			multiple elements of this type in the structure.}
	}
	{notes:
		{note:  This module does no locking at all. The locking of the
				object list is the responsibility of the user.}
	}
}
$*/
typedef RvObjListElement RvObjSListElement;

/*$
{callback:
	{name: RvObjSListFunc}	
	{superpackage: RvObjSList}
	{include: rvobjslist.h}
	{description:
		{p: This function is called to compare the keys of two items.}
	}
	{proto: RvInt RvObjSListFunc(void *ptr1, void *ptr2);}
	{params:
		{param: {n: ptr1} {d: Object 1.}}
		{param: {n: ptr2} {d: Object 2.}}
		{param: {n: data} {d: User data.}}
	}
	{returns: 0 if ptr1 equals ptr2, -1 if ptr1 < ptr2, and 1 if ptr1 > ptr2.}
}
$*/
typedef RvInt (*RvObjSListFunc)(void *ptr1, void *ptr2, void *data);

/*$
{type:
	{name: RvObjSList}
	{superpackage: ObjSList}
	{include: rvobjslist.h}
	{description:	
		{p: A sorted object list object.}
	}
}
$*/
typedef struct {
	RvObjList itemlist;
	RvObjSListFunc itemcmp;
	void *data;
	void *itemtemp;
	RvObjListElement *elementptr;
} RvObjSList;

/* Constants for indicating if items should be removed from the list. */
#define RV_OBJSLIST_REMOVE RV_OBJLIST_REMOVE
#define RV_OBJSLIST_LEAVE RV_OBJLIST_LEAVE

#if defined(__cplusplus)
extern "C" {
#endif 

/* Prototypes and macros: See documentation blocks below for details. */
RvObjSList *RvObjSListConstruct(RvObjSList *objslist, void *itemtemp, RvObjSListElement *elementptr, RvObjSListFunc itemcmp, void *data);
void RvObjSListDestruct(RvObjSList *objslist);
RvSize_t RvObjSListSize(RvObjSList *objslist);
void *RvObjSListAdd(RvObjSList *objslist, void *newitem);
void *RvObjSListGetNext(RvObjSList *objslist, void *curitem, RvBool removeitem);
void *RvObjSListGetPrevious(RvObjSList *objslist, void *curitem, RvBool removeitem);
void *RvObjSListRemoveItem(RvObjSList *objslist, void *item);
void *RvObjSListFind(RvObjSList *objslist, void *keyitem);
void *RvObjSListFindPosition(RvObjSList *objslist, void *keyitem);
void RvObjSListClear(RvObjSList *objslist);

#if defined(RV_TEST_CODE)
void RvObjSListTest(void);
#endif /* RV_TEST_CODE */

#if defined(__cplusplus)
}
#endif 
/*$
{function:
	{name: RvObjSListConstruct}
	{superpackage: ObjSList}
	{include: rvobjslist.h}
	{description:
		{p: Constructs a sorted object list.}
	}
	{proto: RvObjSList RvObjSListConstruct(RvObjSList *objslist, void *itemtmp, RvObjSListElement *elementptr, RvObjSListFunc itemcmp, void *data);}
	{params:
		{param: {n: objslist} {d: Pointer to object list object to construct.}}
		{param: {n: itemtmp} {d: Pointer to an object of the type to be stored in the list.}}
		{param: {n: elementptr} {d: Pointer to the element within itemtmp to use for this list.}}
		{param: {n: itemcmp} {d: Pointer to function used to compare item keys.}}
		{param: {n: data} {d: User data to be passed to the itemcmp function.}}
	}
	{returns: A pointer to the sorted object list object or, if there is an error, NULL.}
	{notes:
		{note:  The itemtmp and elementptr pointers are simply used to find the offset
				within the structure where the RvObjSListElement element is located.
				Anything the itemtmp pointer may point is is never touched.}
	}
}
$*/
/*$
{function:
	{name: RvObjSListDestruct}
	{superpackage: ObjSList}
	{include: rvobjslist.h}
	{description:
		{p: Destructs a sorted object list.}
	}
	{proto: void RvObjSListDestruct(RvObjSList *objslist);}
	{params:
		{param: {n: _objslist} {d: Pointer to sorted object list object to be destructed.}}
	}
	{notes:
		{note:  Any items still in the list are left untouched.}
	}
}
$*/
/*$
{function:
	{name: RvObjSListSize}
	{superpackage: ObjSList}
	{include: rvobjslist.h}
	{description:
		{p: Get the current size of a sorted object list.}
	}
	{proto: RvSize_t RvObjSListSize(RvObjSList *objslist);}
	{params:
		{param: {n: objslist} {d: Pointer to sorted object list to get size of.}}
	}
	{returns: The size of the sorted object list.}
}
$*/
/*$
{function:
	{name: RvObjSListAdd}
	{superpackage: ObjSList}
	{include: rvobjslist.h}
	{description:
		{p: Adds a new item into a sorted list.}
	}
	{proto: void *RvObjSListAdd(RvObjSList *objslist, void *newitem);}
	{params:
		{param: {n: objslist} {d: Pointer to sorted object list to insert new item into.}}
		{param: {n: newitem} {d: Pointer to new item to be inserted into the list.}}
	}
	{returns: A pointer to newitem or NULL if the item could not be inserted.}
}
$*/
/*$
{function:
	{name: RvObjSListGetNext}
	{superpackage: ObjSList}
	{include: rvobjslist.h}
	{description:
		{p: Gets the next item after a particular item in a sorted list.}
	}
	{proto: void *RvObjSListGetNext(RvObjSList *objslist, void *curitem, RvBool removeitem);}
	{params:
		{param: {n: objslist} {d: Pointer to sorted object list to get item from.}}
		{param: {n: curitem} {d: Pointer to item to get next item after (NULL gets the first item in the list.}}
		{param: {n: removeitem} {d: Set to RV_OBJSLIST_REMOVE to remove the item from the list or set it
									to RV_OBJLIST_LEAVE to leave the item in the list.}}
	}
	{returns: A pointer to the next item or NULL if curitem was the last item in the list.}
	{notes:
		{note:  The curitem item must be in the list because that
				condition will not be detected and will cause major problems.}
	}
}
$*/
/*$
{function:
	{name: RvObjSListGetPrevious}
	{superpackage: ObjSList}
	{include: rvobjslist.h}
	{description:
		{p: Gets the previous item before a particular item in a sorted list.}
	}
	{proto: void *RvObjSListGetPrevious(RvObjSList *objslist, void *curitem, RvBool removeitem);}
	{params:
		{param: {n: objslist} {d: Pointer to sorted object list to previous item from.}}
		{param: {n: curitem} {d: Pointer to item to get previous item before (NULL gets the last item in the list.}}
		{param: {n: removeitem} {d: Set to RV_OBJSLIST_REMOVE to remove the item from the list or set it
									to RV_OBJ_LIST_LEAVE to leave the item in the list.}}
	}
	{returns: A pointer to the previous item or NULL if curitem was the first item in the list.}
	{notes:
		{note:  The curitem item must be in the specified list because that
				condition will not be detected and will cause major problems.}
	}
}
$*/
/*$
{function:
	{name: RvObjSListRemoveItem}
	{superpackage: ObjSList}
	{include: rvobjslist.h}
	{description:
		{p: Removes an item from a sorted list.}
	}
	{proto: void *RvObjSListRemoveItem(RvObjSList *objslist, void *item);}
	{params:
		{param: {n: objslist} {d: Pointer to sorted object list to remove item from.}}
		{param: {n: item} {d: Pointer to item to remove from the list.}}
	}
	{returns: A pointer to the item that was removed or NULL if it could not be removed.}
	{notes:
		{note:  Do not remove an item from a list that it is not in because that
				condition will not be detected and will cause major problems.}
	}
}
$*/
/*$
{function:
	{name: RvObjSListFind}
	{superpackage: ObjSList}
	{include: rvobjslist.h}
	{description:
		{p: Finds an item in a sorted list that has keys which match the keys in
			the keyitem object. The keyitem object must be of the same type as the
			other objects in the table but the only information that needs to be
			filled in is that needed by the comparison function. The item is not
			removed from the sorted list.}
	}
	{proto: void *RvObjSListFind(RvObjSList *objslist, void *keyitem);}
	{params:
		{param: {n: objslist} {d: Pointer to sorted list to find the item in.}}
		{param: {n: keyitem} {d: Key object to search list for.}}
	}
	{returns: A pointer to first matching item or NULL if there are no matching items.}
}
$*/
/*$
{function:
	{name: RvObjSListFindPosition}
	{superpackage: ObjSList}
	{include: rvobjslist.h}
	{description:
		{p: This does exactly the same as RvObjSListFind except that if no exact
			match is found, the object returned will be the object in the list
			which would be immediately after keyitem in the table, if it did exist.
			Thus, an object with this key would be inserted immediately before the
			returned item in the table. If NULL is returned, then the keyitem
			belongs at the end of the sorted list. The item is not removed from
			the sorted list.}
	}
	{proto: void *RvObjSListFindPosition(RvObjSList *objslist, void *keyitem);}
	{params:
		{param: {n: objslist} {d: Pointer to sorted list to find the item in.}}
		{param: {n: keyitem} {d: Key object to search sorted list for.}}
	}
	{returns: A pointer to a matching item, or, if one is not found, a pointer to the
				item that would immediately follow the keyitem (NULL = end of list).}
}
$*/
/*$
{function:
	{name: RvObjSListClear}
	{superpackage: ObjSList}
	{include: rvobjslist.h}
	{description:
		{p: Clears all items from a sorted list.}
	}
	{proto: void RvObjSListClear(RvObjSList *objslist);}
	{params:
		{param: {n: objslist} {d: Pointer to sorted list to clear.}}
	}
}
$*/

#endif /* RV_OBJSLIST_H */
