/************************************************************************
 File Name	   : rvsrtpcontext.h
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
#if !defined(RV_SRTP_CONTEXT_H)
#define RV_SRTP_CONTEXT_H

#include "rvtypes.h"
#include "rvobjlist.h"
#include "rvobjslist.h"
#include "rvobjpool.h"
#include "rvmemory.h"
#include "rvsrtpkey.h"

#if defined(__cplusplus)
extern "C" {
#endif 

/*$
{type scope="private":
	{name: RvSrtpContext}
	{superpackage: RvSrtpEncryptionPlugin}
	{include: rvsrtpcontext.h}
	{description:
		{p: This type holds per-key context information for a given
			range of indexes for a given master key. Each source will have
			a list of contexts containing previous contexts and contexts
			that will be used in the future.}
	}
}
$*/
typedef struct {
	RvObjListElement   listElem;    /* Master Key list of contexts */
	RvObjSListElement  slistElem;   /* source/destination fromIndex sorted list */
	RvObjPoolElement   poolElem;    /* Context allocation pool */
	RvSrtpKey     *masterKey;       /* master key this context is derived from */
	void          *data;            /* data associated with this context (usually a stream) */
	RvUint64       fromIndex;       /* index indicating when context should start being used */
	RvUint64       switchIndex;     /* index indicating the point where the current keys should be used instead of the old keys */
	RvUint64       nextSwitchIndex; /* index indicating the point where new current keys should be derived */
	RvBool         neverUsed;       /* RV_TRUE if no session keys have ever been generated for this context (otherwise RV_FALSE) */
	RvBool         trigger;         /* RV_TRUE is switching to this key should trigger others */
	RvInt64        count;           /* How many packets have been encrypted/decrytped (max depends on RTP or RTCP) */
	RvUint8        currentKey;      /* indicates which key is current (thus other is old) */
	RvBool         keyValid[2];     /* RV_TRUE if key has been created */
	RvUint8       *encryptKey[2];   /* current and old encryption key */
	RvUint8       *authKey[2];      /* current and old authentication key */
	RvUint8       *salt[2];         /* current and old salt */
	RvSize_t       keyBufSize;      /* Number of bytes required to store all the keys */
	RvUint8        keys[1];         /* variable length structure for storing all keys */
} RvSrtpContext;

/*$
{type scope="private":
	{name: RvSrtpContextPool}
	{superpackage: RvSrtpEncryptionPlugin}
	{include: rvsrtpcontext.h}
	{description:
		{p: This is a pool of context objects. Since context objects are
			variable size, the actual object size will depend on the
			sizes of the keys (there are two of each).}
	}
}
$*/
typedef struct {
	RvObjPool pool;
	RvMemory *region;
	RvSize_t encryptKeySize;
	RvSize_t authKeySize;
	RvSize_t saltSize;
} RvSrtpContextPool;

/* Use only for stand-alone context objects (not pooled) */
RvSrtpContext *rvSrtpContextConstruct(RvSrtpContext *thisPtr, RvSize_t encryptKeySize, RvSize_t authKeySize, RvSize_t saltSize);
#define rvSrtpContextDestruct(thisPtr)
RvSize_t rvSrtpContextCalcSize(RvSize_t encryptKeySize, RvSize_t authKeySize, RvSize_t saltSize);

/* Standard context functions */
RvSize_t rvSrtpContextGetSize(RvSrtpContext *thisPtr);
RvBool rvSrtpContextUpdateKeys(RvSrtpContext *thisPtr, RvUint64 index, RvUint64 WrapIndex, RvBool *isCurrent);
void rvSrtpContextEnableCurrentKeys(RvSrtpContext *thisPtr, RvUint64 index, RvUint64 wrapIndex);
#define rvSrtpContextGetListElem(thisPtr)           (&((thisPtr)->listElem))
#define rvSrtpContextGetSListElem(thisPtr)          (&((thisPtr)->slistElem))
#define rvSrtpContextGetMasterKey(thisPtr)          ((thisPtr)->masterKey)
#define rvSrtpContextSetMasterKey(thisPtr, v)       ((thisPtr)->masterKey = (v))
#define rvSrtpContextGetData(thisPtr)               ((thisPtr)->data)
#define rvSrtpContextSetData(thisPtr, v)            ((thisPtr)->data = (v))
#define rvSrtpContextGetFromIndex(thisPtr)          ((thisPtr)->fromIndex)
#define rvSrtpContextSetFromIndex(thisPtr, v)       ((thisPtr)->fromIndex = (v))
#define rvSrtpContextGetNeverUsed(thisPtr)          ((thisPtr)->neverUsed)
#define rvSrtpContextGetTrigger(thisPtr)            ((thisPtr)->trigger)
#define rvSrtpContextSetTrigger(thisPtr, v)         ((thisPtr)->trigger = (v))
#define rvSrtpContextIncCount(thisPtr)              (++((thisPtr)->count))
#define rvSrtpContextHasCurrentKeys(thisPtr)        ((thisPtr)->keyValid[(thisPtr)->currentKey])
#define rvSrtpContextHasOldKeys(thisPtr)            ((thisPtr)->keyValid[(thisPtr)->currentKey ^ RvUint8Const(1)])
#define rvSrtpContextGetCurrentEncryptKey(thisPtr)  ((thisPtr)->encryptKey[(thisPtr)->currentKey])
#define rvSrtpContextGetOldEncryptKey(thisPtr)      ((thisPtr)->encryptKey[(thisPtr)->currentKey ^ RvUint8Const(1)])
#define rvSrtpContextGetCurrentAuthKey(thisPtr)     ((thisPtr)->authKey[(thisPtr)->currentKey])
#define rvSrtpContextGetOldAuthKey(thisPtr)         ((thisPtr)->authKey[(thisPtr)->currentKey ^ RvUint8Const(1)])
#define rvSrtpContextGetCurrentSalt(thisPtr)        ((thisPtr)->salt[(thisPtr)->currentKey])
#define rvSrtpContextGetOldSalt(thisPtr)            ((thisPtr)->salt[(thisPtr)->currentKey ^ RvUint8Const(1)])

/* Pool functions */
RvSrtpContextPool *rvSrtpContextPoolConstruct(RvSrtpContextPool *pool, RvSize_t encryptKeySize, RvSize_t authKeySize, RvSize_t saltSize, RvMemory *region, RvSize_t pageitems, RvSize_t pagesize, RvInt pooltype, RvSize_t maxitems, RvSize_t minitems, RvSize_t freelevel);
RvBool rvSrtpContextPoolDestruct(RvSrtpContextPool *pool);
RvSrtpContext *rvSrtpContextPoolNew(RvSrtpContextPool *pool);
void rvSrtpContextPoolRelease(RvSrtpContextPool *pool, RvSrtpContext *context);

#if defined(RV_TEST_CODE)
#include "rvrtpstatus.h"
RvRtpStatus RvSrtpContextTest();
#endif /* RV_TEST_CODE */

/* Function Docs */
/*$
{function scope="private":
    {name:    rvSrtpContextConstruct}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function constructs a context object with the given
			key sizes. It assumes there is enough memory at the end of the
			object to store all of the keys.}
    }
    {proto: RvSrtpContext *rvSrtpContextConstruct(RvSrtpContext *thisPtr, RvSize_t encryptKeySize, RvSize_t authKeySize, RvSize_t saltSize);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object to construct.}}
        {param: {n:encryptKeySize} {d:The size of the encryption key.}}
        {param: {n:authKeySize} {d:The size of the authentication key.}}
        {param: {n:saltSize} {d:The size of the salt.}}
    }
    {returns: The context constructed or NULL if there was an error. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextDestruct}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function destructs a context object.}
    }
    {proto: void rvSrtpContextDestruct(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object to destruct.}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextCalcSize}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function calculates the memory required to construct a
			key object with the key sizes specified.}
    }
    {proto: RvSize_t rvSrtpContextCalcSize(RvSize_t encryptKeySize, RvSize_t authKeySize, RvSize_t saltSize);}
    {params:
        {param: {n:encryptKeySize} {d:The size of the encryption key.}}
        {param: {n:authKeySize} {d:The size of the authentication key.}}
        {param: {n:saltSize} {d:The size of the salt.}}
    }
    {returns: The memory size required for the object (in bytes).}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextGetSize}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function gets the size of the object.}
    }
    {proto: RvSize_t rvSrtpContextGetSize(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The size of the object (in bytes).}
}
$*/
/*$
{function scope="private":
	{name:    rvSrtpContextUpdateKeys}
	{class:   RvSrtpContext}
	{include: rvsrtpcontext.h}
	{description:
		{p: This function updates the context's key buffers based on the
			index provided and returns the information needed to determine
			how to process the index's packet.}
		{p: The index must be in the range covered by the specified context.}
		{p: The return value of the function indicates if the index can
			be processed. If the return value is RV_FALSE, then there are
			no keys for this index and no keys can be created for that
			index. The processing of that packet should be aborted.}
		{p: If the return value is RV_TRUE, than the packet can be
			processed and a couple of additional steps should be taken to
			determine exactly what to do. Check the returned value of the
			isCurrent parameter. If it is set to RV_FALSE, then the old
			contexts keys should be used to process the packet (they
			will always exist when indicated). If it is set to RV_TRUE,
			then the current keys should be used to process the packet.
			BEFORE using the current keys, use the
			rvSrtpContextHasCurrentKeys function to determine if current
			keys need to be generated (a value of RV_FALSE indicates keys
			need to be generated). You can use the
			rvSrtpContextGetNeverUsed function to determine if these are
			the first keys for the context. Be sure to use the
			rvSrtpConextEnableCurrentKeys after generating new keys.}
	}
    {proto: RvBool rvSrtpContextUpdateKeys(RvSrtpContext *thisPtr, RvUint64 index, RvUint64 WrapIndex, RvBool *isCurrent);}
    {params:
		{param: {n:thisPtr} {d:The RvSrtpContext object which corresponds to the index.}}
		{param: {n:index} {d:The index of the packet being processed.}}
		{param: {n:wrapIndex} {d:The maximum index value of the stream the context is part of.}}
		{param: {n:*isCurrent} {d:Output: will be set to RV_TRUE if the current keys should be used, RV_FALSE if the old keys should be used.}}
	}
    {returns: The RV_TRUE if the specified index can be processed, RV_FALSE if it must be discarded.}
}
$*/
/*$
{function scope="private":
	{name:    rvSrtpContextEnableCurrentKeys}
	{class:   RvSrtpContext}
	{include: rvsrtpcontext.h}
	{description:
		{p: This function enables the current keys, indicating that the
			content of the current encryption key, authentication key, and
			salt are valid and usable. This functionshould be called after
			generating these keys into the context's current key buffers.}
		{p: This function also marks the context as having keys
			generated for it (which can be checked with the
			rvSrtpContextGetNeverUsed function).}
		{p: In addition, for use when a key derivation rate is being
			used, the index at which this key derivation occured and
			tht index at which the next key derivation should occur is stored.}
		{p: The index must be in the range covered by the specified context.}
	}
    {proto: void rvSrtpContextEnableCurrentKeys(RvSrtpContext *thisPtr, RvUint64 index, RvUint64 wrapIndex);}
    {params:
		{param: {n:thisPtr} {d:The RvSrtpContext object.}}
		{param: {n:index} {d:The index used to generate the current keys.}}
		{param: {n:wrapIndex} {d:The maximum index value of the stream the context is part of.}}
	}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextGetListElem}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function gets the list element pointer of the object.}
    }
    {proto: RvObjListElement *rvSrtpContextGetListElem(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The list element pointer.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextGetSListElem}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function gets the sorted list element pointer of the object.}
    }
    {proto: RvObjSListElement *rvSrtpContextGetSListElem(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The sorted list element pointer.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextGetMasterKey}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function gets a pointer to the master key object that
			this context is derviced from.}
    }
    {proto: RvSrtpKey *rvSrtpContextGetMasterKey(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The master key object pointer.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextSetMasterKey}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function sets the pointer to the master key object that
			this context is derviced from.}
    }
    {proto: void rvSrtpContextSetMasterKey(RvSrtpContext *thisPtr, RvSrtpKey *masterKey);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
        {param: {n:masterKey} {d:The master key object.}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextGetData}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function gets the data pointer associated with this context.}
    }
    {proto: void *rvSrtpContextGetData(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The data pointer.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextSetData}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function sets the data pointer associated with this context.}
    }
    {proto: void rvSrtpContextSetData(RvSrtpContext *thisPtr, void *data);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
        {param: {n:data} {d:The data pointer.}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextGetFromIndex}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function gets the index that this context should start
			being used from.}
    }
    {proto: RvUint64 rvSrtpContextGetFromIndex(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The index.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextSetFromIndex}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function sets the index that this context should start
			being used from.}
    }
    {proto: void rvSrtpContextSetFromIndex(RvSrtpContext *thisPtr, RvUint64 fromIndex);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
        {param: {n:fromIndex} {d:The from index.}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextGetNeverUsed}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function finds out if the context has ever had session keys
			generated for it.}
    }
    {proto: RvBool rvSrtpContextGetNeverUsed(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
    }
    {returns: RV_TRUE if no keys have ever been generated for this context, other RV_FALSE.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextGetTrigger}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function gets the trigger flag for the context. A
			value of RV_TRUE indicates that this context will trigger other
			contexts to use the same master keys.}
    }
    {proto: RvBool rvSrtpContextGetTrigger(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The trigger flag.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextSetTrigger}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function sets the trigger flag for the context. A
			value of RV_TRUE indicates that this context will trigger other
			contexts to use the same master keys.}
    }
    {proto: void rvSrtpContextSetTrigger(RvSrtpContext *thisPtr, RvBool trigger);}
    {params:
        {param: {n:thisPtr} {d: The RvSrtpContext object.}}
        {param: {n:trigger} {d: RV_TRUE = trigger, RV_FALSE = do not trigger.}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextIncCount}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function increments count of the number of packets encrypted
			or decritped with this context by one.}
    }
    {proto: RvUint64 rvSrtpContextIncCount(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The new packet count value.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextHasCurrentKeys}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function finds out if the context has a set of current
			derived keys.}
    }
    {proto: RvBool rvSrtpContextHasCurrentKeys(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
    }
    {returns: RV_TRUE if the context has current keys, RV_FALSE if it does not.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextHasOldKeys}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function finds out if the context has a set of old
			derived keys.}
    }
    {proto: RvBool rvSrtpContextHasOldKeys(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
    }
    {returns: RV_TRUE if the context has old keys, RV_FALSE if it does not.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextGetCurrentEncryptKey}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function gets the current encyrption key.}
    }
    {proto: RvUint8 *rvSrtpContextGetCurrentEncryptKey(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The current encyrption key.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextGetOldEncryptKey}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function gets the old encyrption key.}
    }
    {proto: RvUint8 *rvSrtpContextGetOldEncryptKey(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The old encyrption key.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextGetCurrentAuthKey}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function gets the current authentication key.}
    }
    {proto: RvUint8 *rvSrtpContextGetCurrentAuthKey(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The current authentication key.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextGetOldAuthKey}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function gets the old authentication key.}
    }
    {proto: RvUint8 *rvSrtpContextGetOldAuthKey(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The old authentication key.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextGetCurrentSalt}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function gets the current salt.}
    }
    {proto: RvUint8 *rvSrtpContextGetCurrentSalt(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The current salt.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextGetOldSalt}
    {class:   RvSrtpContext}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function gets the old salt.}
    }
    {proto: RvUint8 *rvSrtpContextGetOldSalt(RvSrtpContext *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The old salt.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextPoolConstruct}
    {class:   RvSrtpContextPool}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function constructs a pool of context objects with the given
			key sizes and pool parameters.}
		{p: Refer to the RvObjPool documentation for more details on
			the pool parameters.}
    }
    {proto: RvSrtpContextPool *RvSrtpContextPoolConstruct(RvSrtpContextPool *pool, RvSize_t encryptKeySize, RvSize_t authKeySize, RvSize_t saltSize, RvMemory *region, RvSize_t pageitems, RvSize_t pagesize, RvInt pooltype, RvSize_t maxitems, RvSize_t minitems, RvSize_t freelevel);}
    {params:
        {param: {n:pool} {d:The RvSrtpContextPool object to construct.}}
        {param: {n:encryptKeySize} {d:The size of the encryption key.}}
        {param: {n:authKeySize} {d:The size of the authentication key.}}
        {param: {n:saltSize} {d:The size of the salt.}}
        {param: {n:region} {d:The memory region to allocate the pool from (NULL = default region).}}
        {param: {n:pageitems} {d:Number of items per page (0 = calculate from pagesize).}}
        {param: {n:pagesize} {d:Size of each page (0 = calculate from pageitems).}}
        {param: {n:pooltype} {d:The type of pool (FIXED, EXPANDING, DYNAMIC).}}
        {param: {n:maxitems} {d: Number of items never to exceed this value (0 = no max).}}
        {param: {n:minitems} {d:Number of items never to go below this value.}}
        {param: {n:freelevel} {d:Minimum number of items free per 100 (0 to 100). Used for DYNAMIC pools only.}}
    }
    {returns: The context pool constructed or NULL if there was an error. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextPoolDestruct}
    {class:   RvSrtpContextPool}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function destructs a pool of context objects. All
			contexts must have been released back to the pool in order for
			the destuct to be successful.}
    }
    {proto: RvBool RvSrtpContextPoolDestruct(RvSrtpContextPool *pool);}
    {params:
        {param: {n:pool} {d:The RvSrtpContextPool object to destruct.}}
    }
    {returns: RV_TRUE if the pool was destructed, otherwise RV_FALSE. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextPoolNew}
    {class:   RvSrtpContextPool}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function allocates and constructs a context object from
			the pool.}
    }
    {proto: RvSrtpContext *RvSrtpContextPoolNew(RvSrtpContextPool *pool);}
    {params:
        {param: {n:pool} {d:The RvSrtpContextPool object to allocate from.}}
    }
    {returns: The constructed context object. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextPoolRelease}
    {class:   RvSrtpContextPool}
    {include: rvsrtpcontext.h}
    {description:
        {p: This function destructs and releases a context object to
			the pool.}
    }
    {proto: void RvSrtpContextPoolRelease(RvSrtpContextPool *pool, RvSrtpContext *context);}
    {params:
        {param: {n:pool} {d:The RvSrtpContextPool object to allocate from.}}
        {param: {n:context} {d:The RvSrtpContext object release.}}
    }
}
$*/

#if defined(__cplusplus)


}
#endif

#endif
