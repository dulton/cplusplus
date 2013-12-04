/***********************************************************************
Filename   : rvobjhash.h
Description: rvobjhash header file
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
	{name: ObjHash}
	{superpackage: CUtils}
	{include: rvobjhash.h}
	{description:	
		{p: This module provides functions for creating and manipulating
			hash tables. Memory for the internal data of the table will
			be allocated and freed via callback functions.}
		{p:	To make a structure usable in a hash table, an element of type
			RvObjHashElement must be declared in that structure.}
		{p: There are three types of hash tables:}
		{bulletlist:
			{item:  FIXED: This creates a hash table with a fixed number of
					buckets. The size may only be changed with the
					RvObjHashChangeSize function.}
			{item:  EXPANDING: This creates a hash table which expands to try
					and optimize the number of buckets based on the number of
					items in the hash table. When the number of items is larger
					than the number of buckets, the number of buckets will be
					increased to a prime number greater than twice the current
					size. When the size changes, it will allocate a new space
					and release the old space. The size may also be changed with
					the RvObjHashChangeSize function.}
			{item:  DYNAMIC: This creates a hash table which expands exactly
					like an EXPANDING hash table but also has the ability to
					reduce its size. When the number of items is smaller than
					half a prime number that is less than half the current
					number of buckets, the number of buckets will be reduced to
					that prime number. This new space allocated and the old space
					will be released. The size may also be changed with the
					RvObjHashChangeSize function. The size will never be reduced
					below the starting size.}
		}
	}
	{notes:
		{note:  This module does no locking at all. The locking of the
				hash table is the responsibility of the user.}
		{note:  Remember that objects should be removed from the hash
				table before key items in those objects are changed.}
	}
}
$*/
#ifndef RV_OBJHASH_H
#define RV_OBJHASH_H

#include "rvtypes.h"
#include "rvobjlist.h"

/*$
{type:
	{name: RvObjHashElement}
	{superpackage: ObjHash}
	{include: rvobjhash.h}
	{description:	
		{p: An element of this data type must be declared in the
			structure to be put into a hash table.}
		{p: The information stored in this element is only used while a
			particular object is actually in the hash table. The
			information stored in that element does not need to be
			maintained when the object is not in the hash table.}
		{p: A structure can be put into multiple hash tables by
			simply declaring multiple elements of this type in the
			structure.}
	}
}
$*/
typedef struct {
	RvObjListElement listelem; /* Must be first element in struct. */
	RvSize_t hashval;
} RvObjHashElement;

/*$
{type:
	{name: RvObjHashFuncs}
	{superpackage: ObjHash}
	{include: rvobjhash.h}
	{description:	
		{p: A structure containing callback information for a hash table. This structure
			must be completely filled out before passing it to RvObjHashConstruct.}
		{p: The memalloc and memfree callbacks will be used to allocate and release
			memory used for the hash table.}
		{p: The itemcmp callback is the user defined operation that determines if the keys
			of two objects match.}
		{p: The gethash callback is the user defined operation to find the hash value of an item.}
	}
	{attributes scope="public":
		{attribute: {t: void *()(RvSize_t size, void *data)} {n: memalloc} {d: Callback to allocate size bytes of memory. Return pointer to memory or NULL for failure.}}
		{attribute: {t: void ()(void *ptr, void *data} {n: memfree} {d: Callback to free the memory at ptr.}}
		{attribute: {t: RvInt ()(void *ptr1, void *ptr2)} {n: itemcmp} {d: Callback to compare two items' keys. Return 0 if ptr1 equals ptr2, -1 if ptr1 < ptr2, and 1 if ptr1 > ptr2.}}
		{attribute: {t: RvSize_t ()(void *item)} {n: gethash} {d: Get hash value of item.}}
		{attribute: {t: void *} {n: memallocdata} {d: User data parameter passed into memalloc.}}
		{attribute: {t: void *} {n: memfreedata} {d: User data parameter passed into memfree.}}
		{attribute: {t: void *} {n: itemcmpdata} {d: User data parameter passed into itemcmp.}}
		{attribute: {t: void *} {n: gethashdata} {d: User data parameter passed into gethash.}}
	}
}
$*/
typedef struct {
	void    *(*memalloc)(RvSize_t size, void *data); /* Allocate memory for heap. Return pointer to memory, NULL = failed. */
	void     (*memfree)(void *ptr, void *data);      /* Free memory allocated by memalloc. */
	RvInt    (*itemcmp)(void *ptr1, void *ptr2, void *data);     /* Compare 2 items' keys, return 0 if ptr1 equals ptr2, -1 if ptr1 < ptr2, and 1 if ptr1 > ptr2. */
	RvSize_t (*gethash)(void *item, void *data);                 /* Get hash value of item. */
	void *memallocdata;    /* User data parameter passed into memalloc */
	void *memfreedata;     /* User data parameter passed into memfree */
	void *itemcmpdata;     /* User data parameter passed into itemcmp */
	void *gethashdata;     /* User data parameter passed into gethash */
} RvObjHashFuncs;

/*$
{type:
	{name: RvObjHash}
	{superpackage: ObjHash}
	{include: rvobjhash.h}
	{description:	
		{p: An hash table object.}
	}
}
$*/
typedef struct {
	RvSize_t startsize;      /* Starting number of buckets in hash table, also is minimum size */
	RvSize_t cursize;        /* Current number of buckets in hash table */
	RvInt tabletype;         /* type of hash table: see RV_OBJHASH_TYPE_ */
	RvObjHashFuncs callbacks; /* User defined function callbacks */
	RvSize_t numitems;       /* Number or items currently in the hash table */
	RvSize_t shrinksize;     /* Next lower prime number which DYNAMIC table will shrink to. */
	RvPtrdiff_t offset;      /* Offset into objects of RvObjHashElement */
	void *itemtmp;           /* Temp object pointer for reconstructing lists. */
	RvObjListElement *eptr;  /* Temp element pointer for reconstructing lists. */
	RvObjList *buckets;      /* Actual array of buckets (each being a list), which will be allocated */
} RvObjHash;

/* Object Hash Types */
/*   FIXED: number of buckets stays fixed at starting size */
/*   EXPANDING: number of buckets is increased (and reallocated) to maintain optimum size. */
/*   DYNAMIC: number of buckets is reduced (and reallocated) to maintain optimum size (min of startsize) */
#define RV_OBJHASH_TYPE_FIXED RvIntConst(0)
#define RV_OBJHASH_TYPE_EXPANDING RvIntConst(1)
#define RV_OBJHASH_TYPE_DYNAMIC RvIntConst(2)

#if defined(__cplusplus)
extern "C" {
#endif 

/* Prototypes: See documentation blocks below for details. */
RvObjHash *RvObjHashConstruct(RvObjHash *objhash, void *itemtmp, RvObjHashElement *elementptr, RvInt htype, RvSize_t startsize, RvObjHashFuncs *callbacks);
void RvObjHashDestruct(RvObjHash *objhash);
void *RvObjHashAdd(RvObjHash *objhash, void *newitem);
void *RvObjHashAddUnique(RvObjHash *objhash, void *newitem);
void *RvObjHashFind(RvObjHash *objhash, void *keyitem);
void *RvObjHashFindNext(RvObjHash *objhash, void *item);
void *RvObjHashGetNext(RvObjHash *objhash, void *item);
void *RvObjHashGetPrevious(RvObjHash *objhash, void *item);
RvSize_t RvObjHashNumItems(RvObjHash *objhash);
RvSize_t RvObjHashSize(RvObjHash *objhash);
RvSize_t RvObjHashChangeSize(RvObjHash *objhash, RvSize_t newsize);
void RvObjHashClear(RvObjHash *objhash);
void *RvObjHashRemove(RvObjHash *objhash, void *item);

#if defined(RV_TEST_CODE)
void RvObjHashTest(void);
#endif /* RV_TEST_CODE */

#if defined(__cplusplus)
}
#endif

/* Function Docs */
/*$
{function:
	{name: RvObjHashConstruct}
	{superpackage: ObjHash}
	{include: rvobjhash.h}
	{description:
		{p: Constructs a hash table.}
	}
	{proto: RvObjHash *RvObjHashConstruct(RvObjHash *objhash, void *itemtmp, RvObjHashElement *elementptr, RvInt htype, RvSize_t startsize, RvObjHashFuncs *callbacks);}
	{params:
		{param: {n: objhash} {d: Pointer to hash table object to construct.}}
		{param: {n: itemtmp} {d: Pointer to an object of the type to be stored in the hash table.}}
		{param: {n: elementptr} {d: Pointer to the element within itemtmp to use for this hash table.}}
		{param: {n: htype}   {d: The type of hash table: RV_OBJHASH_TYPE_FIXED, RV_OBJHASH_TYPE_EXPANDING, or RV_OBJHASH_TYPE_DYNAMIC.}}
		{param: {n: startsize} {d: Starting size (number of buckets) of hash table.}}
		{param: {n: callbacks} {d: Pointer to structure with callback information.}}
	}
	{returns: A pointer to the hash table object or, if there is an error, NULL.}
	{notes:
		{note:  The itemtmp and elementptr pointers are simply used to find the offset
				within the structure where the RvObjHashElement element is located.
				Anything the itemtmp pointer may point is is never touched.}
		{note:  The callbacks structure will be copied so there is no need to maintain this
				structure after construction.}
		{note:  The minimum table size is 2. If startsize < 2 it will be set to 2.}
		{note:  DYNAMIC hash tables will never shrink below startsize.}
	}
}
$*/
/*$
{function:
	{name: RvObjHashDestruct}
	{superpackage: ObjHash}
	{include: rvobjhash.h}
	{description:
		{p: Destructs a hash table.}
	}
	{proto: void RvObjHashDestruct(RvObjHash *objhash);}
	{params:
		{param: {n: objhash} {d: Pointer to hash table object to destruct.}}
	}
	{notes:
		{note:  Any objects in the hash table when it is destructed are considered removed.}
	}
}
$*/
/*$
{function:
	{name: RvObjHashAdd}
	{superpackage: ObjHash}
	{include: rvobjhash.h}
	{description:
		{p: Adds a new item into a hash table.}
	}
	{proto: void *RvObjHashAdd(RvObjHash *objhash, void *newitem);}
	{params:
		{param: {n: objhash} {d: Pointer to hash table to add newitem into.}}
		{param: {n: newitem} {d: Pointer to the item to put into the hash table.}}
	}
	{returns: A pointer to newitem or, if the item can not be added to the hash table, NULL.}
}
$*/
/*$
{function:
	{name: RvObjHashAddUnique}
	{superpackage: ObjHash}
	{include: rvobjhash.h}
	{description:
		{p: Adds a new item into a hash table if there is no item with the same
			key already in the hash table.}
		{p: If an item is already in the hash table with the same key
			as the item being added, a pointer to that already existing
			item is returned.}
	}
	{proto: void *RvObjHashAdd(RvObjHash *objhash, void *newitem);}
	{params:
		{param: {n: objhash} {d: Pointer to hash table to add newitem into.}}
		{param: {n: newitem} {d: Pointer to the item to put into the hash table.}}
	}
	{returns: A pointer to newitem if it was added to the hash table.
		If it was not added, a pointer to an existing item in the table
		having the same key is returned, otherwise, if the item can not
		be added to the hash table for another reason, then NULL is returned.}
}
$*/
/*$
{function:
	{name: RvObjHashFind}
	{superpackage: ObjHash}
	{include: rvobjhash.h}
	{description:
		{p: Finds an item in the hash table that has keys which match the keys in
			the keyitem object. The keyitem object must be of the same type as the other
			objects in the table but the only information that needs to be filled in is that
			needed by the itemcmp and gethash callback functions. The item is not removed
			from the hash table.}
	}
	{proto: void *RvObjHashFind(RvObjHash *objhash, void *keyitem);}
	{params:
		{param: {n: objhash} {d: Pointer to hash table to find the item in.}}
		{param: {n: keyitem} {d: Key object to search table for.}}
	}
	{returns: A pointer to first matching item or NULL if there are no matching items.}
}
$*/
/*$
{function:
	{name: RvObjHashFindNext}
	{superpackage: ObjHash}
	{include: rvobjhash.h}
	{description:
		{p: Finds the next item in the hash table that has keys which match the keys
			in the item object. Only the same bucket will be checked, thus the item object
			must be in the hash table. Neither item is removed from the hash table.}
	}
	{proto: void *RvObjHashFindNext(RvObjHash *objhash, void *item);}
	{params:
		{param: {n: objhash} {d: Pointer to hash table to find next item in.}}
		{param: {n: item} {d: Item to start search at and compare to.}}
	}
	{returns: A pointer to next matching item or NULL if there are no more matching items.}
}
$*/
/*$
{function:
	{name: RvObjHashGetNext}
	{superpackage: ObjHash}
	{include: rvobjhash.h}
	{description:
		{p: Gets the next item in the hash table after the specified item. There are
			no comparisons made, the hash table is simply walked. The item object must
			be in the hash table or be set to NULL, which will return the first item
			in the hash table. Neither item is removed from the hash table.}
	}
	{proto: void *RvObjHashGetNext(RvObjHash *objhash, void *item);}
	{params:
		{param: {n: objhash} {d: Pointer to hash table to get next item from.}}
		{param: {n: item} {d: Current item (NULL gets first item).}}
	}
	{returns: A pointer to next item or NULL if there are no more items.}
}
$*/
/*$
{function:
	{name: RvObjHashGetPrevious}
	{superpackage: ObjHash}
	{include: rvobjhash.h}
	{description:
		{p: Gets the previous item in the hash table after the specified item. There are
			no comparisons made, the hash table is simply walked. The item object must
			be in the hash table or be set to NULL, which will return the last item
			in the hash table. Neither item is removed from the hash table.}
	}
	{proto: void *RvObjHashGetPrevious(RvObjHash *objhash, void *item);}
	{params:
		{param: {n: objhash} {d: Pointer to hash table to get previous item from.}}
		{param: {n: item} {d: Current item (NULL gets last item).}}
	}
	{returns: A pointer to previous item or NULL if there are no more items.}
}
$*/
/*$
{function:
	{name: RvObjHashNumItems}
	{superpackage: ObjHash}
	{include: rvobjhash.h}
	{description:
		{p: Gets number of items currently in a hash table.}
	}
	{proto: RvSize_t RvObjHashNumItems(RvObjHash *objhash);}
	{params:
		{param: {n: objhash} {d: Pointer to hash table to get number of items of.}}
	}
	{returns: The number of items in the hash table.}
}
$*/
/*$
{function:
	{name: RvObjHashSize}
	{superpackage: ObjHash}
	{include: rvobjhash.h}
	{description:
		{p: Gets the size (number of buckets) of a hash table.}
	}
	{proto: RvSize_t RvObjHashSize(RvObjHash *objhash);}
	{params:
		{param: {n: objhash} {d: Pointer to hash table to get size of.}}
	}
	{returns: The size (number of buckets) of the hash table.}
}
$*/
/*$
{function:
	{name: RvObjHashChangeSize}
	{superpackage: ObjHash}
	{include: rvobjhash.h}
	{description:
		{p: Changes the size (number of buckets) of a hash table. Existing items
		in the hash table will be reorganized based on the new number of buckets.}
	}
	{proto: RvSize_t RvObjHashChangeSize(RvObjHash *objhash, RvSize_t newsize);}
	{params:
		{param: {n: objhash} {d: Pointer to hash table to change the size of.}}
		{param: {n: newsize} {d: New size (number of buckets) of hash table.}}
	}
	{returns:   The size (number of buckets) of the hash table. If the size
				could not be changed than the old size of the hash table is returned.}
	{notes:
		{note:  The size can not be set smaller than 2.}
		{note:  For DYNAMIC hash tables this value replaces the startsize as
				the minimum size of the hash table.}
	}
}
$*/
/*$
{function:
	{name: RvObjHashClear}
	{superpackage: ObjHash}
	{include: rvobjhash.h}
	{description:
		{p: Clears all items from a hash table.}
	}
	{proto: void RvObjHashClear(RvObjHash *objhash);}
	{params:
		{param: {n: objhash} {d: Pointer to hash table to clear.}}
	}
	{notes:
		{note:  For DYNAMIC hash tables, their size will shrink back to
				their minimum size, which is their starting size unless it was
				changed with a call to RvObjHashChangeSize.}
	}
}
$*/
/*$
{function:
	{name: RvObjHashRemove}
	{superpackage: ObjHash}
	{include: rvobjhash.h}
	{description:
		{p: Removes an item from a hash table.}
	}
	{proto: void *RvObjHashRemove(RvObjHash *objhash, void *item);}
	{params:
		{param: {n: objhash} {d: Pointer to hash table to remove item from.}}
		{param: {n: item} {d: The item to be removed.}}
	}
	{returns: A pointer to item that has been removed from the hash table or NULL there is an error.}
}
$*/

#endif /* RV_OBJHASH_H */
