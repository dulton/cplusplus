/************************************************************************
 File Name	   : rvsrtphistory.h
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
#if !defined(RV_SRTP_HISTORY_H)
#define RV_SRTP_HISTORY_H

#include "rvtypes.h"
#include "rvobjslist.h"
#include "rvsrtpkey.h"
#include "rvsrtpcontext.h"

#if defined(__cplusplus)
extern "C" {
#endif 

/*$
{type scope="private":
	{name: RvSrtpHistory}
	{superpackage: RvSrtpEncryptionPlugin}
	{include: rvsrtphistory.h}
	{description:
		{p: This manages all of the contexts for a given stream by
			maintaining them in the history timeline (which is a sorted
			chronological list of the contexts). Both historical and
			scheduled contexts are maintained in the history timeline.}
		{p: All contexts for a given stream should be managed through
			this object. It will use the specified pool to create and
			remove contexts as necessary and will also update the master
			key context list appropriately.}
		{p: It also maintains all the information that this list
			depends on (maxIndex, startIndex, historySize, and
			wrapIndex).}
		{p: Important: Do not modify the masterKey, contextPool, or
			fromIndex values of the contexts being managed, they will
			be automatically updated.}
	}
}
$*/
typedef struct {
	RvObjSList contextSList;   /* list of contexts to use, sorted cronologically */
	RvUint64   maxIndex;       /* The "current" index (highest index encrypted or decrypted by the stream) */
	RvUint64   initIndex;      /* The first index value to be used (starting index of the stream) */
	RvUint64   historySize;    /* Size of the history that should be held in the context list */
	RvUint64   wrapIndex;      /* maximum index value */
	RvSrtpContextPool *contextPool; /* Pointer to pool to create contexts from */
} RvSrtpHistory;

/* Context History functions */
RvSrtpHistory *rvSrtpHistoryConstruct(RvSrtpHistory *thisPtr, RvUint64 initIndex, RvUint64 historySize, RvUint64 wrapIndex, RvSrtpContextPool *pool);
void rvSrtpHistoryDestruct(RvSrtpHistory *thisPtr);
void rvSrtpHistoryReset(RvSrtpHistory *thisPtr, RvUint64 initIndex, RvUint64 historySize, RvUint64 wrapIndex);
void rvSrtpHistoryClear(RvSrtpHistory *thisPtr);
RvSrtpContext *rvSrtpHistoryAdd(RvSrtpHistory *thisPtr, RvUint64 startIndex, RvSrtpKey *masterKey, RvBool *contextAdded);
RvBool rvSrtpHistoryRemove(RvSrtpHistory *thisPtr, RvSrtpContext *contextPtr);
RvSrtpContext *rvSrtpHistoryFind(RvSrtpHistory *thisPtr, RvUint64 index);
RvBool rvSrtpHistoryCheckIndex(RvSrtpHistory *thisPtr, RvUint64 index, RvSrtpContext *contextPtr);
RvBool rvSrtpHistoryUpdateMaxIndex(RvSrtpHistory *thisPtr, RvUint64 newMaxIndex);
#define rvSrtpHistoryGetMaxIndex(thisPtr) ((thisPtr)->maxIndex)
#define rvSrtpHistoryGetInitIndex(thisPtr) ((thisPtr)->initIndex)
#define rvSrtpHistoryGetSize(thisPtr) ((thisPtr)->historySize)
#define rvSrtpHistoryGetWrapIndex(thisPtr) ((thisPtr)->wrapIndex)
#define rvSrtpHistoryGetNext(thisPtr, c) ((RvSrtpContext *)RvObjSListGetNext(&((thisPtr)->contextSList), (c), RV_OBJSLIST_LEAVE))
#define rvSrtpHistoryGetPrevious(thisPtr, c) ((RvSrtpContext *)RvObjSListGetPrevious(&((thisPtr)->contextSList), (c), RV_OBJSLIST_LEAVE))

#if defined(RV_TEST_CODE)
RvRtpStatus rvSrtpHistoryTest();
#endif /* RV_TEST_CODE */

/* Function Docs */
/*$
{function scope="private":
    {name:    rvSrtpHistoryConstruct}
    {class:   RvSrtpHistory}
    {include: rvsrtphistory.h}
    {description:
        {p: This function constructs a history timeline with the
			specified initial parameters.}
    }
    {proto: RvSrtpHistory *rvSrtpHistoryConstruct(RvSrtpHistory *thisPtr, RvUint64 initIndex, RvUint64 historySize, RvUint64 wrapIndex, RvSrtpContextPool *pool);}
    {params:
        {param: {n:thisPtr}   {d:The RvSrtpHistory object to construct.}}
        {param: {n:initIndex} {d:The index value of the first expected packet in the timeline.}}
        {param: {n:historySize} {d:How long (in index values) to keep contexts.}}
        {param: {n:wrapIndex} {d:The maximum value of the index (after which it wraps).}}
        {param: {n:pool}      {d:The context pool to allocate contexts from.}}
    }
    {returns: The context history timeline constructed, or NULL if there was an error.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpHistoryDestruct}
    {class:   RvSrtpHistory}
    {include: rvsrtphistory.h}
    {description:
        {p: This function Destructs a history timeline. Any contexts
			still in the timeline will be deleted.}
    }
    {proto: void rvSrtpHistoryDestruct(RvSrtpHistory *thisPtr);}
    {params:
        {param: {n:thisPtr}   {d:The RvSrtpHistory object to destruct.}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpHistoryReset}
    {class:   RvSrtpHistory}
    {include: rvsrtphistory.h}
    {description:
        {p: This function resets a history timeline and restarts it
			using the specified initial parameters.}
		{p: The history timeline must be empty before calling this
			function because any contexts in the history will be lost (not
			returned to the pool).}
    }
    {proto: void rvSrtpHistoryReset(RvSrtpHistory *thisPtr, RvUint64 initIndex, RvUint64 historySize, RvUint64 wrapIndex);}
    {params:
        {param: {n:thisPtr}   {d:The RvSrtpHistory object to reset.}}
        {param: {n:initIndex} {d:The index value of the first expected packet in the timeline.}}
        {param: {n:historySize} {d:How long (in index values) to keep contexts.}}
        {param: {n:wrapIndex} {d:The maximum value of the index (after which it wraps).}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpHistoryClear}
    {class:   RvSrtpHistory}
    {include: rvsrtphistory.h}
    {description:
        {p: This function deletes all contexts in a history timeline.}
    }
    {proto: void rvSrtpHistoryClear(RvSrtpHistory *thisPtr);}
    {params:
        {param: {n:thisPtr}   {d:The RvSrtpHistory object to clear.}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpHistoryAdd}
    {class:   RvSrtpHistory}
    {include: rvsrtphistory.h}
    {description:
        {p: Adds a new context to the history timeline at the specified
			index and create a link to its master key. The masterKey
			parameter indicates which master key should be linked to
			the context (NULL is valid and will simply not create the
			link).}
        {p: The startIndex value may not be historical. A historical
			index value is one that is older than the maxIndex value and is
			within a range of historySize (set when timeline was
			constructed) of the current maxIndex value.}
        {p: If the startIndex has the same value as an existing context
			in the timeline, it will replace that context will this new
			one.}
		{p: If adding the context causes it to be adjacent in the
			timeline to a context which uses the same master key, it will
			be merged with that context (since switching to the same key is
			not actually a switch). If it is merged with a context which
			appears after the new context in the timeline, the existing
			context will be deleted. If it is merged with a context which
			appears before the new context in the timeline, no new context
			will be added and the exiting context will be returned.}
		{p: If a context is returned by the function, then the
			contextAdded flag should be checked. This will be set to
			RV_TRUE if a new context has been added to the timeline (which
			means the caller will probably need to initialize fields of the
			context to the needed values). If contextAdded is RV_FALSE,
			then no new context was added and the returned context is an
			existing context that uses the same master key and which is in
			effect at the specified startIndex.}
		{p: Note that adding a NULL context (context with a NULL master
			key) at the beginning of the timeline (or to an empty timeline)
			will return NULL because it doesn't change anything, so a new
			context is not added.}
		{p: The startIndex value must be smaller than the wrapIndex value
			specificed when the history timeline was constructed.}
    }
    {proto: RvSrtpContext *rvSrtpHistoryAdd(RvSrtpHistory *thisPtr, RvUint64 startIndex, RvSrtpKey *masterKey, RvBool *contextAdded);}
    {params:
        {param: {n:thisPtr}   {d:The RvSrtpHistory object to add the context to.}}
        {param: {n:startIndex} {d:The index value where the new context should start.}}
        {param: {n:masterKey} {d:The master key used by the context to be added.}}
        {param: {n:contextAdded} {d:Will be set to RV_TRUE if a new context is added, RV_FALSE otherwise.}}
    }
    {returns: A pointer to the added context (or an existing context if
				a new context was not needed). The pointer will be NULL if
				startIndex was out of range or a NULL context add was requested at
				the beginning of the timeline.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpHistoryRemove}
    {class:   RvSrtpHistory}
    {include: rvsrtphistory.h}
    {description:
        {p: This function removes a context from a history timeline.}
        {p: It is important to remember that the context is not
			physically removed from the timeline but it simply removes the
			reference to its master key and marks it as a NULL context. The
			context will physically be removed when it becomes older than
			the history being kept (historySize).}
		{p: Remember that a context can be physically replaced by
			adding a new context with the same starting index as an
			existing context. Technically, adding a NULL context with the
			same starting index as an existing context will do the same
			exact thing as this function, but using this function is a lot
			more efficient.}
		{p: If changing the context to a NULL context results in
			adjacent NULL contexts, the contexts will be merged into one
			and the contexts that are no longer needed will be physically
			deleted.}
    }
    {proto: RvBool rvSrtpHistoryRemove(RvSrtpHistory *thisPtr, RvSrtpContext *contextPtr);}
    {params:
        {param: {n:thisPtr}   {d:The RvSrtpHistory object remove the context from.}}
        {param: {n:contextPtr} {d:The context to be removed.}}
    }
    {returns: RV_TRUE if the context was removed, RV_FALSE otherwise.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpHistoryFind}
    {class:   RvSrtpHistory}
    {include: rvsrtphistory.h}
    {description:
        {p: Finds the context that applies to the specified index.}
		{p: The index value must be smaller than the wrapIndex value
			specificed when the history timeline was constructed,
			otherwise a NULL always will be returned.}
		{p: A value of NULL indicates that no context applies, which,
			in effect, means eactly the same as if a NULL context was found
			(a master key of NULL).}
    }
    {proto: RvSrtpContext *rvSrtpHistoryFind(RvSrtpHistory *thisPtr, RvUint64 index);}
    {params:
        {param: {n:thisPtr}   {d:The RvSrtpHistory object to find in.}}
        {param: {n:index} {d:The index value to look up.}}
    }
    {returns: A pointer to the context that applies or NULL if there is no context for the spcified index.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpHistoryCheckIndex}
    {class:   RvSrtpHistory}
    {include: rvsrtphistory.h}
    {description:
        {p: Checks to see if the specified index is within the index
			range of the specified context.}
    }
    {proto: RvBool rvSrtpHistoryCheckIndex(RvSrtpHistory *thisPtr, RvUint64 index, RvSrtpContext *contextPtr);}
    {params:
        {param: {n:thisPtr}   {d:The RvSrtpHistory object to find in.}}
        {param: {n:index}     {d:The index value to check.}}
        {param: {n:contextPtr} {d:The context object to check the index against.}}
    }
    {returns: RV_TRUE if the context contains the specified index, otherwise RV_FALSE.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpHistoryUpdateMaxIndex}
    {class:   RvSrtpHistory}
    {include: rvsrtphistory.h}
    {description:
        {p: This function updates the maxIndex value. The maxIndex
			value indicate the last index number received in the history
			timeline, which mean that it indicates what the "current time"
			is.}
        {p: The maxIndex value may only be moved "forward" in time. An
			index value is considered "forward" if is not considerd an
			historical index value. A historical index value is one that
			is older than the maxIndex value and is within a range of
			historySize (set when timeline was constructed) of the current
			maxIndex value.}
        {p: When the maxIndex value is updated, any contexts in the
			timeline that are older than the historySize are deleted. The
			start of the timeline will always be historySize earlier than
			the maxIndex value.}
		{p: The maxIndex value must be smaller than the wrapIndex value
			specificed when the history timeline was constructed.}
    }
    {proto: RvBool rvSrtpHistoryUpdateMaxIndex(RvSrtpHistory *thisPtr, RvUint64 newMaxIndex);}
    {params:
        {param: {n:thisPtr}   {d:The RvSrtpHistory object to update maxIndex for.}}
        {param: {n:newMaxIndex} {d:The new value of maxIndex.}}
    }
    {returns: RV_TRUE if maxIndex was updated, RV_FALSE if not (because
				maxIndex was considered historical or out of range).}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpHistoryGetMaxIndex}
    {class:   RvSrtpHistory}
    {include: rvsrtphistory.h}
    {description:
        {p: Gets the current value of maxIndex.}
    }
    {proto: RvUint64 rvSrtpHistoryGetMaxIndex(RvSrtpHistory *thisPtr);}
    {params:
        {param: {n:thisPtr}   {d:The RvSrtpHistory object.}}
    }
    {returns: The value of maxIndex (index of newest packet).}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpHistoryGetInitIndex}
    {class:   RvSrtpHistory}
    {include: rvsrtphistory.h}
    {description:
        {p: Gets the current value of initIndex. This is set by the
			construct or reset function and will never change.}
    }
    {proto: RvUint64 rvSrtpContextGetInitIndex(RvSrtpHistory *thisPtr);}
    {params:
        {param: {n:thisPtr}   {d:The RvSrtpHistory object.}}
    }
    {returns: The value of initIndex (index of first packet for this timeline).}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpHistoryGetSize}
    {class:   RvSrtpHistory}
    {include: rvsrtphistory.h}
    {description:
        {p: Gets the current value of historySize. This is set by the
			construct or reset function and will never change.}
    }
    {proto: RvUint64 rvSrtpHistoryGetSize(RvSrtpHistory *thisPtr);}
    {params:
        {param: {n:thisPtr}   {d:The RvSrtpHistory object.}}
    }
    {returns: The value of historySize (index value of how long to keep old contexts).}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpHistoryGetWrapIndex}
    {class:   RvSrtpHistory}
    {include: rvsrtphistory.h}
    {description:
        {p: Gets the current value of wrapIndex. This is set by the
			construct or reset function and will never change.}
    }
    {proto: RvUint64 rvSrtpContextGetWrapIndex(RvSrtpHistory *thisPtr);}
    {params:
        {param: {n:thisPtr}   {d:The RvSrtpHistory object.}}
    }
    {returns: The value of wrapIndex (maximum index value before wrap occurs).}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpHistoryGetNext}
    {class:   RvSrtpHistory}
    {include: rvsrtphistory.h}
    {description:
        {p: This function gets the context that follows the specified
			context in the history timeline.}
    }
    {proto: RvSrtpContext *rvSrtpHistoryGetNext(RvSrtpHistory *thisPtr, RvSrtpContext *contextPtr);}
    {params:
        {param: {n:thisPtr}   {d:The RvSrtpHistory object.}}
        {param: {n:contextPtr} {d:The context to look after.}}
    }
    {returns: The context that follows the given context (NULL = NONE).}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpHistoryGetPrevious}
    {class:   RvSrtpHistory}
    {include: rvsrtphistory.h}
    {description:
        {p: This function gets the context that preceeds the specified
			context in the history timeline.}
    }
    {proto: RvSrtpContext *rvSrtpHistoryGetPrevious(RvSrtpHistory *thisPtr, RvSrtpContext *contextPtr);}
    {params:
        {param: {n:thisPtr}   {d:The RvSrtpHistory object.}}
        {param: {n:contextPtr} {d:The context to look before.}}
    }
    {returns: The context that preceeds the given context (NULL = NONE).}
}
$*/

#if defined(__cplusplus)


}
#endif

#endif

