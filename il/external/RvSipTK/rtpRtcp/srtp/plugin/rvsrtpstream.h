/************************************************************************
 File Name	   : rvsrtpstream.h
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
#if !defined(RV_SRTP_STREAM_H)
#define RV_SRTP_STREAM_H

#include "rvtypes.h"
#include "rvobjhash.h"
#include "rvobjpool.h"
#include "rvobjslist.h"
#include "rvaddress.h"
#include "rvmemory.h"
#include "rvsrtpkey.h"
#include "rvsrtphistory.h"
#include "rvsrtpcontext.h"

#if defined(__cplusplus)
extern "C" {
#endif 

/*$
{type scope="private":
	{name: RvSrtpStream}
	{superpackage: RvSrtpEncryptionPlugin}
	{include: rvsrtpstream.h}
	{description:
		{p: This type represents an RTP or RTCP stream and contains all information
			regarding that stream. It can represent a destination of a
			stream we are transmitting or the remote source of a stream
			we are receiving.}
	}
}
$*/
typedef struct {
	RvObjHashElement hashElem; /* hash lookup: ssrc/isRtp for source, ssrc/address for destination */
	RvObjPoolElement poolElem; /* source allocation pool */
	RvSrtpHistory    history;  /* History timeline of stream (also tracks maxIndex) */
	RvUint32   ssrc;           /* ssrc of stream */
	RvAddress  address;        /* address of the destination (destination only) */
	RvBool     isRTP;          /* RV_TRUE if RTP source, RV_FALSE if RTCP source */
	RvBool     isRemote;       /* RV_TRUE if this is a remote source, RV_FALSE if this is a destination */
	RvUint64   replayListSize; /* Size of the replay list */
    RvUint8    replayList[1];  /* Variable length replay list (remote source only) */
} RvSrtpStream;

/*$
{type scope="private":
	{name: RvSrtpStreamPool}
	{superpackage: RvSrtpEncryptionPlugin}
	{include: rvsrtpstream.h}
	{description:
		{p: This is a pool of stream objects. Since stream objects are
			variable size, the actual object size will depend on the size
			of the replay list.}
	}
}
$*/
typedef struct {
	RvObjPool pool;
	RvMemory *region;
	RvUint64 replayListSize;
	RvSrtpContextPool *contextPool;
} RvSrtpStreamPool;

/* Use only for stand-alone stream objects (not pooled) */
RvSrtpStream *rvSrtpStreamConstruct (RvSrtpStream *thisPtr, RvUint64 replayListSize, RvSrtpContextPool *contextPool);
void rvSrtpStreamDestruct(RvSrtpStream *thisPtr);
RvSize_t rvSrtpStreamCalcSize(RvUint64 replayListSize);
RvBool rvSrtpStreamSetUp(RvSrtpStream *thisPtr, RvUint64 initIndex, RvUint64 historySize, RvBool isRtp);

/* Use only for stream objects which do not maintain a history */
#define rvSrtpStreamSetIsRtp(thisPtr, v) ((thisPtr)->isRTP = (v))

/* Standard stream functions */
RvSize_t rvSrtpStreamGetSize(RvSrtpStream *thisPtr);
RvBool rvSrtpStreamTestReplayList(RvSrtpStream *thisPtr, RvUint64 index, RvBool update);
RvBool rvSrtpStreamUpdateMaxIndex(RvSrtpStream *thisPtr, RvUint64 newMaxIndex);
RvBool rvSrtpStreamTrigger(RvSrtpContext *contextPtr);
void rvSrtpStreamIncEncryptCount(RvSrtpContext *contextPtr);
#define rvSrtpStreamSetSsrc(thisPtr, v)        ((thisPtr)->ssrc = (v))
#define rvSrtpStreamSetIsRemote(thisPtr, v)    ((thisPtr)->isRemote = (v))
#define rvSrtpStreamGetHashElem(thisPtr)       (&((thisPtr)->hashElem))
#define rvSrtpStreamGetHistorySize(thisPtr)    (rvSrtpHistoryGetHistorySize(&(thisPtr)->history))
#define rvSrtpStreamGetSsrc(thisPtr)           ((thisPtr)->ssrc)
#define rvSrtpStreamGetAddress(thisPtr)        (&((thisPtr)->address))
#define rvSrtpStreamGetMaxIndex(thisPtr)       (rvSrtpHistoryGetMaxIndex(&(thisPtr)->history))
#define rvSrtpStreamGetInitIndex(thisPtr)      (rvSrtpHistoryGetInitIndex(&(thisPtr)->history))
#define rvSrtpStreamGetIsRtp(thisPtr)          ((thisPtr)->isRTP)
#define rvSrtpStreamGetIsRemote(thisPtr)       ((thisPtr)->isRemote)
#define rvSrtpStreamGetReplayListSize(thisPtr) ((thisPtr)->replayListSize)
#define rvSrtpStreamGetWrapIndex(thisPtr)      (rvSrtpHistoryGetWrapIndex(&(thisPtr)->history))

/* Context management functions */
RvSrtpContext *rvSrtpStreamAddContext(RvSrtpStream *thisPtr, RvUint64 startIndex, RvSrtpKey *masterKey, RvBool trigger);
#define rvSrtpStreamRemoveContext(contextPtr)  (rvSrtpHistoryRemove(&(((RvSrtpStream *)(rvSrtpContextGetData(contextPtr)))->history), (contextPtr)))
#define rvSrtpStreamFindContext(thisPtr, i)    (rvSrtpHistoryFind((&(thisPtr)->history), (i)))
#define rvSrtpStreamClearContext(thisPtr)      (rvSrtpHistoryClear(&((thisPtr)->history)))
#define rvSrtpStreamGetFromContext(contextPtr) ((RvSrtpStream *)rvSrtpContextGetData(contextPtr))

/* Pool functions */
RvSrtpStreamPool *rvSrtpStreamPoolConstruct(RvSrtpStreamPool *pool, RvUint64 replayListSize, RvSrtpContextPool *contextPool, RvMemory *region,
                                            RvSize_t pageitems, RvSize_t pagesize, RvInt pooltype, RvSize_t maxitems,
                                            RvSize_t minitems, RvSize_t freelevel);
RvBool rvSrtpStreamPoolDestruct(RvSrtpStreamPool *pool);
RvSrtpStream *rvSrtpStreamPoolNew(RvSrtpStreamPool *pool, RvUint64 initIndex, RvUint64 historySize, RvBool isRtp);
void rvSrtpStreamPoolRelease(RvSrtpStreamPool *pool, RvSrtpStream *stream);

#if defined(RV_TEST_CODE)
#include "rvrtpstatus.h"
RvRtpStatus RvSrtpStreamTest();
#endif /* RV_TEST_CODE */

/* Function Docs */
/*$
{function scope="private":
    {name:    rvSrtpStreamConstruct}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function constructs a stream object with the given
			replay list size. It assumes there is enough memory at the
			end of the object for the replay list (remember to clear
			the replay list before using it).}
		{p: Contexts add to this stream will be allocated from the
			specified context pool.}
		{p: A stream is not fully function until a call is made to
			rvSrtpStreamSetUp, to set up the stream.}
    }
    {proto: RvSrtpstream *rvSrtpStreamConstruct(RvSrtpStream *thisPtr, RvUint64 replayListSize, RvSrtpContextPool *contextPool);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object to construct.}}
        {param: {n:replayListSize} {d:The size of the replay list (0 = disable).}}
        {param: {n:contextPool} {d:Pool to allocate contexts from.}}
    }
    {returns: The stream object constructed or NULL if there was an error. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamDestruct}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function destructs a stream object.}
    }
    {proto: void rvSrtpStreamDestruct(RvSrtpStream *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object to destruct.}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamCalcSize}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function calculates the memory required to construct a
			stream object with a replay list of the specified size.}
    }
    {proto: RvSize_t rvSrtpStreamCalcSize(RvUint64 replayListSize);}
    {params:
        {param: {n:replayListSize} {d:The size of the replay list.}}
    }
    {returns: The memory size required for the object (in bytes).}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamSetUp}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function sets up the initial parameters required by
			the stream history. This must only be called on a stream that
			has no contexts in its history timeline.}
		{p: A the history of a stream object will not function until
			this function called.}
    }
    {proto: RvBool rvSrtpStreamSetUp(RvSrtpStream *thisPtr, RvUint64 initIndex, RvUint64 historySize, RvBool isRtp);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object.}}
        {param: {n:initIndex} {d:The index value of the first expected packet.}}
        {param: {n:historySize} {d:How long (in index values) to keep contexts.}}
        {param: {n:isRtp} {d:RV_TRUE for RTP, RV_FALSE to RTCP.}}
    }
    {returns: RV_TRUE if the stream was successfully set up, otherwise RV_FALSE.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamSetIsRtp}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function sets the isRtp flag of the stream. A value of
			RV_TRUE means the stream is an RTP stream, a value of RV_FALSE
			means it is an RTCP stream.}
    }
    {proto: void rvSrtpStreamSetIsRtp(RvSrtpStream *thisPtr, RvBool isRtp);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object.}}
        {param: {n:isRtp} {d:RV_TRUE for RTP, RV_FALSE for RTCP.}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamGetSize}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function gets the size of the object.}
    }
    {proto: RvSize_t rvSrtpStreamGetSize(RvSrtpStream *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object.}}
    }
    {returns: The size of the object (in bytes).}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamTestReplayList}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function tests the specified index against the stream's
			replay list. If the index should be processed, RV_TRUE is
			returned. If not, then RV_FALSE is returned.}
		{p: If update is set to RV_TRUE and the specified index is in
			the replay list, then the replay list will be updated to
			indicate that this index has been preocessed.}
		{p: An index that is not historical will return RV_TRUE. An
			index that is historical but is older than the size of the
			replay list will return RV_FALSE.}
    }
    {proto: RvBool rvSrtpStreamTestReplayList(RvSrtpStream *thisPtr, RvUint64 index, RvBool update);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object to check against.}}
        {param: {n:index} {d:The index to test.}}
        {param: {n:update} {d:RV_TRUE is the specified index should be marked in the replay list, otherwise RV_FALSE.}}
    }
    {returns: RV_TRUE if the packet should be accepted, otherwise RV_FALSE}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamUpdateMaxIndex}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function updates the maxIndex value of the stream. The
			maxIndex value indicates the highest packet index value
			processed by the stream.}
		{p: The stream's context history timeline will be updated and
			old contexts will be cleaned up.}
		{p: The stream's replay list will also be aligned to the new
			index and that index will be marked as being processed.}
		{p: A return value of RV_FALSE indicates that the index value
			is not a valid value to change the maxIndex to and is out of
			range or within the history window (indicating that it is an
			old index value) Thus the function can be called with any
			index value and maxIndex will only be updated if it is
			necessary.}
    }
    {proto: RvBool rvSrtpStreamUpdateMaxIndex(RvSrtpStream *thisPtr, RvUint64 newMaxIndex);}
    {params:
        {param: {n:thisPtr} {d:The stream object to update.}}
        {param: {n:newMaxIndex} {d:The new maxIndex value.}}
    }
    {returns: RV_TRUE if the index was updated, RV_FALSE if not (index out of range or old).}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamTrigger}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function checks a context to see if it should cause a
			trigger, and if so, execute the trigger.}
		{p: If the context is a trigger, then any other streams using
			the same master key as the context's stream just switched FROM, will
			have a key change to this context's key scheduled for the next
			packet that stream will process. If a key change is already
			scheduled for the next packet, the trigger will NOT
			override that key change.}
    }
    {proto: RvBool rvSrtpStreamTrigger(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The context to trigger from, if necessary.}}
    }
    {returns: RV_TRUE if a trigger occurred, RV_FALSE if there was no trigger.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamIncEncryptCount}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
		{p: This increments the count of the number of packets a given
			stream has encrypted with the same master key (as indicated
			by the context specified). If the maximum allowed packets
			it reached, the master key will be marked as reaching its
			limit and will not be allowed to be used to encrypt any
			further packets.}
    }
    {proto: void rvSrtpStreamIncEncryptCount(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:A RvSrtpContext object.}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamSetSsrc}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function sets the ssrc of the stream.}
    }
    {proto: void rvSrtpStreamSetSsrc(RvSrtpStream *thisPtr, RvUint32 ssrc);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object.}}
        {param: {n:ssrc} {d:The ssrc.}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamSetIsRemote}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function sets the isRemote flag of the stream. A value of
			RV_TRUE means the stream is a stream from a remote source,
			a value of RV_FALSE means the stream is a destination of a
			stream being sent.}
    }
    {proto: rvSrtpStreamSetIsRemote(RvSrtpStream *thisPtr, RvBool isRemote);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object.}}
        {param: {n:isRemote} {d:RV_TRUE for remote source, RV_FALSE for destination.}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamGetHashElem}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function gets the hash element pointer of the object.}
    }
    {proto: RvObjHashElement *rvSrtpStreamGetHashElem(RvSrtpStream *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object.}}
    }
    {returns: The hash element pointer.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamGetHistorySize}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function gets the size of the history window for this stream.}
    }
    {proto: RvUint64 rvSrtpStreamGetHistorySize(RvSrtpStream *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object.}}
    }
    {returns: The history window size.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamGetSsrc}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function gets the ssrc of the stream.}
    }
    {proto: RvUint32 rvSrtpStreamGetSsrc(RvSrtpStream *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object.}}
    }
    {returns: The SSRC.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamGetAddress}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function gets the destination address of the stream.}
        {p: This is only used for destinations, not for remote sources.}
    }
    {proto: RvAddress *rvSrtpStreamGetAddress(RvSrtpStream *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object.}}
    }
    {returns: A pointer to the address.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamGetMaxIndex}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function gets the highest index that has been encrypted
			or decrypted for the stream.}
    }
    {proto: RvUint64 rvSrtpStreamGetMaxIndex(RvSrtpStream *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object.}}
    }
    {returns: The index.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamGetInitIndex}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function gets the initial index that will be encrypted
			or decrypted for the stream.}
    }
    {proto: RvUint64 rvSrtpStreamGetInitIndex(RvSrtpStream *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object.}}
    }
    {returns: The index.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamGetIsRtp}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function gets the isRtp flag of the stream. A value of
			RV_TRUE means the stream is an RTP stream, a value of RV_FALSE
			means it is an RTCP stream.}
    }
    {proto: RvBool rvSrtpStreamGetIsRtp(RvSrtpStream *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object.}}
    }
    {returns: RV_TRUE for RTP, RV_FALSE for RTCP.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamGetIsRemote}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function gets the isRemote flag of the stream. A value of
			RV_TRUE means the stream is a stream from a remote source,
			a value of RV_FALSE means the stream is a destination of a
			stream being sent.}
    }
    {proto: RvBool rvSrtpStreamGetIsRemote(RvSrtpStream *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object.}}
    }
    {returns: RV_TRUE for remote source, RV_FALSE for destination.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamGetReplayListSize}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function gets the size of the replay list (in number of packets).}
    }
    {proto: RvUint64 rvSrtpStreamGetReplayListSize(RvSrtpStream *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object.}}
    }
    {returns: The size of the replay list (in number of packets).}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamGetWrapIndex}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function gets the maximum index value, after which this stream will wrap back to 0.}
    }
    {proto: RvUint64 rvSrtpStreamGetWrapIndex(RvSrtpStream *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpStream object.}}
    }
    {returns: The index wrap value.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamAddContext}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function adds a new context to a stream based on the
			index. The index indicates the point at which the context
			should start being used.}
		{p: The startIndex and stream of a context object MUST never be modified.}
    }
    {proto: RvSrtpContext *rvSrtpStreamAddContext(RvSrtpStream *thisPtr, RvUint64 startIndex, RvSrtpKey *masterKey, RvBool trigger);}
    {params:
        {param: {n:thisPtr} {d:The stream object to add the context to.}}
        {param: {n:startIndex} {d:The index at which this context should start being used.}}
        {param: {n:masterKey} {d:The master key object the context is derived from.}}
        {param: {n:trigger} {d:RV_TRUE if context should trigger other contexts to switch keys at the same
                 time, RV_FALSE otherwise.}}
    }
    {returns: The new context object, or NULL if it could not be added. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamRemoveContext}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function removes a context from the stream is part of.
			Removing a context creates a NULL context where there is no key
			for the range.}
    }
    {proto: RvBool rvSrtpStreamRemoveContext(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The context to be removed.}}
    }
    {returns: RV_TRUE if the context has been removed, otherwise RV_FALSE.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamFindContext}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: Finds the context that applies to the specified index.}
		{p: The index value must be smaller than the wrapIndex value
			specificed when the history timeline was constructed,
			otherwise a NULL always will be returned.}
		{p: A value of NULL indicates that no context applies, which,
			in effect, means eactly the same as if a NULL context was found
			(a master key of NULL).}
    }
    {proto: RvSrtpContext *rvSrtpStreamFindContext(RvSrtpStream *thisPtr, RvUint64 index);}
    {params:
        {param: {n:thisPtr} {d:The stream object to find in.}}
        {param: {n:index} {d:The index value to look up.}}
    }
    {returns: A pointer to the context that applies or NULL if there is no context for the spcified index.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamUpdateMaxIndex}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function updates the maxIndex value of the stream. The
			maxIndex value indicates the highest packet index value
			processed by the stream.}
		{p: The stream's context history timeline will be updated and
			old contexts will be cleaned up.}
		{p: The stream's replay list will also be aligned to the new
			index and that index will be marked as being processed.}
		{p: A return value of RV_FALSE indicates that the index value
			is not a valid value to change the maxIndex to and is out of
			range or within the history window (indicating that it is an
			old index value) Thus the function can be called with any
			index value and maxIndex will only be updated if it is
			necessary.}
    }
    {proto: RvBool rvSrtpStreamUpdateMaxIndex(RvSrtpStream *thisPtr, RvUint64 newMaxIndex);}
    {params:
        {param: {n:thisPtr} {d:The stream object to update.}}
        {param: {n:newMaxIndex} {d:The new maxIndex value.}}
    }
    {returns: RV_TRUE if the index was updated, RV_FALSE if not (index out of range or old).}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamTrigger}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function checks a context to see if it should cause a
			trigger, and if so, execute the trigger.}
		{p: If the context is a trigger, then any other streams using
			the same master key as the context's stream just switched FROM, will
			have a key change to this context's key scheduled for the next
			packet that stream will process. If a key change is already
			scheduled for the next packet, the trigger will NOT
			override that key change.}
    }
    {proto: RvBool rvSrtpStreamTrigger(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The context to trigger from, if necessary.}}
    }
    {returns: RV_TRUE if a trigger occurred, RV_FALSE if there was no trigger.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbStreamClearContext}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: This function clears all contexts in the history timeline
			for the specified stream.}
    }
    {proto: RvBool rvSrtpStreamClearContext(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The stream to clear.}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamGetFromContext}
    {class:   RvSrtpStream}
    {include: rvsrtpstream.h}
    {description:
        {p: Gets the stream that the specified context is part of.}
    }
    {proto: RvSrtpStream *rvSrtpStreamGetFromContext(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The context to find the stream for.}}
    }
    {returns: The stream the context is part of.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamPoolConstruct}
    {class:   RvSrtpStreamPool}
    {include: rvsrtpstream.h}
    {description:
        {p: This function constructs a pool of stream objects with the given
			replay list size and pool parameters.}
		{p: Refer to the RvObjPool documentation for more details on
			the pool parameters.}
    }
    {proto: rvSrtpStreamPool *RvSrtpStreamPoolConstruct(RvSrtpStreamPool *pool, RvSize_t replaySize, RvSrtpContextPool *contextPool, RvMemory *region, RvSize_t pageitems, RvSize_t pagesize, RvInt pooltype, RvSize_t maxitems, RvSize_t minitems, RvSize_t freelevel);}
    {params:
        {param: {n:pool} {d:The RvSrtpStreamPool object to construct.}}
        {param: {n:replayListSize} {d:The size of the replay list.}}
        {param: {n:contextPool} {d:The contextPool that should be used for contexts in this stream.}}
        {param: {n:region} {d:The memory region to allocate the pool from (NULL = default region).}}
        {param: {n:pageitems} {d:Number of items per page (0 = calculate from pagesize).}}
        {param: {n:pagesize} {d:Size of each page (0 = calculate from pageitems).}}
        {param: {n:pooltype} {d:The type of pool (FIXED, EXPANDING, DYNAMIC).}}
        {param: {n:maxitems} {d: Number of items never to exceed this value (0 = no max).}}
        {param: {n:minitems} {d:Number of items never to go below this value.}}
        {param: {n:freelevel} {d:Minimum number of items free per 100 (0 to 100). Used for DYNAMIC pools only.}}
    }
    {returns: The stream pool constructed or NULL if there was an error. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamPoolDestruct}
    {class:   RvSrtpStreamPool}
    {include: rvsrtpstream.h}
    {description:
        {p: This function destructs a pool of stream objects. All
			streams must have been released back to the pool in order for
			the destuct to be successful.}
    }
    {proto: RvBool rvSrtpStreamPoolDestruct(RvSrtpStreamPool *pool);}
    {params:
        {param: {n:pool} {d:The RvSrtpStreamPool object to destruct.}}
    }
    {returns: RV_TRUE if the pool was destructed, otherwise RV_FALSE. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamPoolNew}
    {class:   RvSrtpStreamPool}
    {include: rvsrtpstream.h}
    {description:
        {p: This function allocates and initializes a stream object from
			the pool.}
    }
    {proto: RvSrtpStream *rvSrtpStreamPoolNew(RvSrtpStreamPool *pool, RvUint64 initIndex, RvUint64 historySize, RvBool isRtp);}
    {params:
        {param: {n:pool} {d:The RvSrtpStreamPool object to allocate from.}}
        {param: {n:initIndex} {d:The index value of the first expected packet.}}
        {param: {n:historySize} {d:How long (in index values) to keep contexts.}}
        {param: {n:isRtp} {d:RV_TRUE for RTP, RV_FALSE to RTCP.}}
    }
    {returns: The new stream object. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpStreamPoolRelease}
    {class:   RvSrtpStreamPool}
    {include: rvsrtpstream.h}
    {description:
        {p: This function destructs and releases a stream object to
			the pool. Any contexts that are still in the streams
			history timeline are deleted.}
    }
    {proto: void rvSrtpStreamPoolRelease(RvSrtpStreamPool *pool, RvSrtpStream *stream);}
    {params:
        {param: {n:pool} {d:The RvSrtpStreamPool object to allocate from.}}
        {param: {n:stream} {d:The RvSrtpStream object to release.}}
    }
}
$*/

#if defined(__cplusplus)
}
#endif

#endif
