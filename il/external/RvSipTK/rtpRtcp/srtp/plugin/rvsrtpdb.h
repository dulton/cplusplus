/************************************************************************
 File Name	   : rvsrtpdb.h
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
#if !defined(RV_SRTP_DB_H)
#define RV_SRTP_DB_H

#include "rvtypes.h"
#include "rvobjpool.h"
#include "rvobjhash.h"
#include "rvobjlist.h"
#include "rvobjslist.h"
#include "rvmemory.h"
#include "rvsrtpkey.h"
#include "rvsrtpstream.h"
#include "rvsrtpcontext.h"

#if defined(__cplusplus)
extern "C" {
#endif 


/*$
{type scope="private":
	{name: RvSrtpDbPoolConfig}
	{superpackage: RvSrtpEncryptionPlugin}
	{include: rvsrtpdb.h}
	{description:
		{p: This is the pool configuration structure for SRTP/SRTCP plugin database.}
		{p: There are three pools: master key, stream (includes both
			sources and destinations), and context.}
	}
	{attributes:
		{attribute: {t: RvMemory *} {n: keyRegion}
					{d: Memory region to allocate keys from (NULL = use default).}}
		{attribute: {t: RvSize_t} {n: keyPageItems} 
					{d: Number of key objects per memory page (0 = calculate from keyPageSize).}}
		{attribute: {t: RvSize_t} {n: keyPageSize} 
					{d: Size of memory page (0 = calculate from keyPageItems).}}
		{attribute: {t: RvInt} {n: keyPoolType} 
					{d: RV_OBJPOOL_TYPE_FIXED, RV_OBJPOOL_TYPE_EXPANDING, or RV_OBJPOOL_TYPE_DYNAMIC.}}
		{attribute: {t: RvSize_t} {n: keyMaxItems} 
					{d: Number of key objects will never exceed this value (0 = no max).}}
		{attribute: {t: RvSize_t} {n: keyFreeLevel} 
					{d: Minimum number of key objects per 100 (0 to 100) - DYNAMIC pools only.}}
		{attribute: {t: RvMemory *} {n: streamRegion}
					{d: Memory region to allocate streams from (NULL = use default).}}
		{attribute: {t: RvSize_t} {n: streamPageItems} 
					{d: Number of stream objects per memory page (0 = calculate from streamPageSize).}}
		{attribute: {t: RvSize_t} {n: streamPageSize} 
					{d: Size of memory page (0 = calculate from streamPageItems).}}
		{attribute: {t: RvInt} {n: streamPoolType} 
					{d: RV_OBJPOOL_TYPE_FIXED, RV_OBJPOOL_TYPE_EXPANDING, or RV_OBJPOOL_TYPE_DYNAMIC.}}
		{attribute: {t: RvSize_t} {n: streamMaxItems} 
					{d: Number of stream objects will never exceed this value (0 = no max).}}
		{attribute: {t: RvSize_t} {n: streamFreeLevel} 
					{d: Minimum number of stream objects per 100 (0 to 100) - DYNAMIC pools only.}}
		{attribute: {t: RvMemory *} {n: contextRegion}
					{d: Memory region to allocate contexts from (NULL = use default).}}
		{attribute: {t: RvSize_t} {n: contextPageItems} 
					{d: Number of context objects per memory page (0 = calculate from contextPageSize).}}
		{attribute: {t: RvSize_t} {n: contextPageSize} 
					{d: Size of memory page (0 = calculate from contextPageItems).}}
		{attribute: {t: RvInt} {n: contextPoolType} 
					{d: RV_OBJPOOL_TYPE_FIXED, RV_OBJPOOL_TYPE_EXPANDING, or RV_OBJPOOL_TYPE_DYNAMIC.}}
		{attribute: {t: RvSize_t} {n: contextMaxItems} 
					{d: Number of context objects will never exceed this value (0 = no max).}}
		{attribute: {t: RvSize_t} {n: contextFreeLevel} 
					{d: Minimum number of context objects per 100 (0 to 100) - DYNAMIC pools only.}}
		{attribute: {t: RvMemory *} {n: keyHashRegion}
					{d: Memory region to allocate key hash buckets from (NULL = use default).}}
		{attribute: {t: RvSize_t} {n: keyHashStartSize}
					{d: Starting number of buckets in hash table (and minimum size).}}
		{attribute: {t: RvInt} {n: keyHashType} 
					{d: RV_OBJHASH_TYPE_FIXED, RV_OBJHASH_TYPE_EXPANDING, or RV_OBJHASH_TYPE_DYNAMIC.}}
		{attribute: {t: RvMemory *} {n: sourceHashRegion}
					{d: Memory region to allocate source hash buckets from (NULL = use default).}}
		{attribute: {t: RvSize_t} {n: sourceHashStartSize}
					{d: Starting number of buckets in hash table (and minimum size).}}
		{attribute: {t: RvInt} {n: sourceHashType} 
					{d: RV_OBJHASH_TYPE_FIXED, RV_OBJHASH_TYPE_EXPANDING, or RV_OBJHASH_TYPE_DYNAMIC.}}
		{attribute: {t: RvMemory *} {n: destHashRegion}
					{d: Memory region to allocate destination hash buckets from (NULL = use default).}}
		{attribute: {t: RvSize_t} {n: destHashStartSize}
					{d: Starting number of buckets in hash table (and minimum size).}}
		{attribute: {t: RvInt} {n: destHashType} 
					{d: RV_OBJHASH_TYPE_FIXED, RV_OBJHASH_TYPE_EXPANDING, or RV_OBJHASH_TYPE_DYNAMIC.}}
	}
}
$*/
typedef struct {

	/* Master key pool */
	RvMemory *keyRegion;    /* Memory region to allocate keys from (NULL = use default) */
	RvSize_t  keyPageItems; /* Number of key objects per memory page (0 = calculate from keyPageSize). */
	RvSize_t  keyPageSize;  /* Size of memory page (0 = calculate from keyPageItems). */
	RvInt     keyPoolType;  /* RV_OBJPOOL_TYPE_FIXED, RV_OBJPOOL_TYPE_EXPANDING, or RV_OBJPOOL_TYPE_DYNAMIC. */
	RvSize_t  keyMaxItems;  /* Number of key objects will never exceed this value (0 = no max). */
	RvSize_t  keyMinItems;  /* Number of key objects will never go below this value. */
	RvSize_t  keyFreeLevel; /* Minimum number of key objects per 100 (0 to 100) - DYNAMIC pools only. */

	/* Stream pool */
	RvMemory *streamRegion;    /* Memory region to allocate streams from (NULL = use default) */
	RvSize_t  streamPageItems; /* Number of stream objects per memory page (0 = calculate from streamPageSize). */
	RvSize_t  streamPageSize;  /* Size of memory page (0 = calculate from streamPageItems). */
	RvInt     streamPoolType;  /* RV_OBJPOOL_TYPE_FIXED, RV_OBJPOOL_TYPE_EXPANDING, or RV_OBJPOOL_TYPE_DYNAMIC. */
	RvSize_t  streamMaxItems;  /* Number of stream objects will never exceed this value (0 = no max). */
	RvSize_t  streamMinItems;  /* Number of stream objects will never go below this value. */
	RvSize_t  streamFreeLevel; /* Minimum number of stream objects per 100 (0 to 100) - DYNAMIC pools only. */
			
	/* Context pool */
	RvMemory *contextRegion;    /* Memory region to allocate contexts from (NULL = use default) */
	RvSize_t  contextPageItems; /* Number of context objects per memory page (0 = calculate from contextPageSize). */
	RvSize_t  contextPageSize;  /* Size of memory page (0 = calculate from contextPageItems). */
	RvInt     contextPoolType;  /* RV_OBJPOOL_TYPE_FIXED, RV_OBJPOOL_TYPE_EXPANDING, or RV_OBJPOOL_TYPE_DYNAMIC. */
	RvSize_t  contextMaxItems;  /* Number of context objects will never exceed this value (0 = no max). */
	RvSize_t  contextMinItems;  /* Number of context objects will never go below this value. */
	RvSize_t  contextFreeLevel; /* Minimum number of context objects per 100 (0 to 100) - DYNAMIC pools only. */

	/* Master Key hash */
	RvMemory *keyHashRegion;    /* Memory region to allocate key hash buckets from (NULL = default)*/
	RvSize_t  keyHashStartSize; /* Starting number of buckets in hash table (and minimum size) */
	RvInt     keyHashType;      /* RV_OBJHASH_TYPE_FIXED, RV_OBJHASH_TYPE_EXPANDING, or RV_OBJHASH_TYPE_DYNAMIC. */

	/* Source Hash */
	RvMemory *sourceHashRegion;    /* Memory region to allocate source hash buckets from (NULL = default)*/
	RvSize_t  sourceHashStartSize; /* Starting number of buckets in hash table (and minimum size) */
	RvInt     sourceHashType;      /* RV_OBJHASH_TYPE_FIXED, RV_OBJHASH_TYPE_EXPANDING, or RV_OBJHASH_TYPE_DYNAMIC. */

	/* Destination Hash*/
	RvMemory *destHashRegion;    /* Memory region to allocate destination hash buckets from (NULL = default)*/
	RvSize_t  destHashStartSize; /* Starting number of buckets in hash table (and minimum size) */
	RvInt     destHashType;      /* RV_OBJHASH_TYPE_FIXED, RV_OBJHASH_TYPE_EXPANDING, or RV_OBJHASH_TYPE_DYNAMIC. */
} RvSrtpDbPoolConfig;

/*$
{type scope="private":
	{name: RvSrtpDb}
	{superpackage: RvSrtpEncryptionPlugin}
	{include: rvsrtpdb.h}
	{description:
		{p: This is the SRTP/SRTCP plugin database. It maintains and
		provides access to all master key, stream, and context objects
		within the system.}
	}
}
$*/
typedef struct {
	RvSrtpKeyPool keyPool;         /* pool of master key objects */
	RvSrtpStreamPool streamPool;   /* pool of stream objects */
	RvSrtpContextPool contextPool; /* pool of context objects */
    RvObjHash keyHash;             /* hash of masker keys, by MKI */
    RvObjHash sourceHash;          /* hash of remote sources, by ssrc and RTP/RTCP flag */
    RvObjHash destHash;            /* hash of destinations, by ssrc and address */

	/* Configuration parameters */
	RvSize_t  masterKeySize;       /* master key size */
	RvSize_t  masterSaltSize;      /* master salt size */
	RvSize_t  mkiSize;             /* MKI size */
	RvSize_t  srtpEncryptKeySize;  /* RTP encrytpion key size */
	RvSize_t  srtpAuthKeySize;     /* RTP authentication key size */
	RvSize_t  srtpSaltSize;        /* RTP salt size */
	RvSize_t  srtcpEncryptKeySize; /* RTCP encrytpion key size */
	RvSize_t  srtcpAuthKeySize;    /* RTCP authentication key size */
	RvSize_t  srtcpSaltSize;       /* RTCP salt size */
	RvUint64  srtpReplayListSize;  /* RTP replay list size (in packets) */
	RvUint64  srtcpReplayListSize; /* RTCP replay list size (in packets) */
	RvUint64  srtpHistorySize;     /* RTP history size (index value) */
	RvUint64  srtcpHistorySize;    /* RTCP history size (index value) */
} RvSrtpDb;

/* Basic functions */
RvSrtpDb *rvSrtpDbConstruct(RvSrtpDb *thisPtr, RvSize_t masterKeySize, RvSize_t masterSaltSize, RvSize_t mkiSize, RvSize_t rtpEncryptKeySize, RvSize_t rtpAuthKeySize, RvSize_t rtpSaltSize, RvSize_t rtcpEncryptKeySize, RvSize_t rtcpAuthKeySize, RvSize_t rtcpSaltSize, RvUint64 rtpReplayListSize, RvUint64 rtcpReplayListSize, RvUint64 srtpHistorySize, RvUint64 srtcpHistorySize, RvSrtpDbPoolConfig *poolConfigPtr);
RvBool rvSrtpDbDestruct(RvSrtpDb *thisPtr);

/* Get confguration information */
RvSize_t rvSrtpDbGetMasterKeySize(RvSrtpDb *thisPtr);
RvSize_t rvSrtpDbGetMasterSaltSize(RvSrtpDb *thisPtr);
RvSize_t rvSrtpDbGetMkiSize(RvSrtpDb *thisPtr);
RvSize_t rvSrtpDbGetSrtpEncryptKeySize(RvSrtpDb *thisPtr);
RvSize_t rvSrtpDbGetSrtpAuthKeySize(RvSrtpDb *thisPtr);
RvSize_t rvSrtpDbGetSrtpSaltSize(RvSrtpDb *thisPtr);
RvSize_t rvSrtpDbGetSrtcpEncryptKeySize(RvSrtpDb *thisPtr);
RvSize_t rvSrtpDbGetSrtcpAuthKeySize(RvSrtpDb *thisPtr);
RvSize_t rvSrtpDbGetSrtcpSaltSize(RvSrtpDb *thisPtr);
RvUint64 rvSrtpDbGetRtpReplayListSize(RvSrtpDb *thisPtr);
RvUint64 rvSrtpDbGetRtcpReplayListSize(RvSrtpDb *thisPtr);
RvUint64 rvSrtpDbGetRtpHistorySize(RvSrtpDb *thisPtr);
RvUint64 rvSrtpDbGetRtcpHistorySize(RvSrtpDb *thisPtr);

/* Get DB information */
RvObjHash *rvSrtpDbGetKeyHash(RvSrtpDb *thisPtr);
RvObjHash *rvSrtpDbGetSrcHash(RvSrtpDb *thisPtr);
RvObjHash *rvSrtpDbGetDstHash(RvSrtpDb *thisPtr);

/* Master key functions */
#ifdef UPDATED_BY_SPIRENT
RvSrtpKey *rvSrtpDbKeyAdd(RvSrtpDb *thisPtr, RvUint8 *mki, RvUint8 *masterKey, RvUint8 *masterSalt,
        RvSize_t keyDerivationRate, RvUint64 lifetime, RvUint64 threshold, RvUint8 direction);
RvSrtpKey *rvSrtpDbKeyFind(RvSrtpDb *thisPtr, RvUint8 *mki, RvUint8 direction);
#else
RvSrtpKey *rvSrtpDbKeyAdd(RvSrtpDb *thisPtr, RvUint8 *mki, RvUint8 *masterKey, RvUint8 *masterSalt, RvSize_t keyDerivationRate);
RvSrtpKey *rvSrtpDbKeyFind(RvSrtpDb *thisPtr, RvUint8 *mki);
#endif // UPDATED_BY_SPIRENT
RvBool rvSrtpDbKeyRemove(RvSrtpDb *thisPtr, RvSrtpKey *key);
#define rvSrtpDbKeyGetMasterKey(keyPtr)       (rvSrtpKeyGetMasterKey((keyPtr)))
#define rvSrtpDbKeyGetMasterSalt(keyPtr)      (rvSrtpKeyGetMasterSalt((keyPtr)))
#define rvSrtpDbKeyGetMki(keyPtr)             (rvSrtpKeyGetMki((keyPtr)))
#define rvSrtpDbKeyGetContextListSize(keyPtr) (rvSrtpKeyGetContextListSize((keyPtr)))

/* Remote source functions */
RvSrtpStream *rvSrtpDbSourceAdd(RvSrtpDb *thisPtr, RvUint32 ssrc, RvBool isRtp, RvUint64 initIndex);
RvBool rvSrtpDbSourceRemove(RvSrtpDb *thisPtr, RvSrtpStream *stream);
RvSrtpStream *rvSrtpDbSourceFind(RvSrtpDb *thisPtr, RvUint32 ssrc, RvBool isRtp);

/* Destination functions */
RvSrtpStream *rvSrtpDbDestAdd(RvSrtpDb *thisPtr, RvUint32 ssrc, RvBool isRtp, RvUint64 initIndex, RvAddress *address);
RvBool rvSrtpDbDestRemove(RvSrtpDb *thisPtr, RvSrtpStream *stream);
RvSrtpStream *rvSrtpDbDestFind(RvSrtpDb *thisPtr, RvUint32 ssrc, RvAddress *address);

/* Stream functions (source or destination) */
#define rvSrtpDbStreamUpdateMaxIndex(streamPtr, i)    (rvSrtpStreamUpdateMaxIndex((streamPtr), (i)))
#define rvSrtpDbStreamGetIsRtp(streamPtr)             (rvSrtpStreamGetIsRtp((streamPtr)))
#define rvSrtpDbStreamGetAddress(streamPtr)           (rvSrtpStreamGetAddress((streamPtr)))
#define rvSrtpDbStreamGetMaxIndex(streamPtr)          (rvSrtpStreamGetMaxIndex((streamPtr)))
#define rvSrtpDbStreamGetSsrc(streamPtr)              (rvSrtpStreamGetSsrc((streamPtr)))
#define rvSrtpDbStreamGetWrapIndex(streamPtr)         (rvSrtpStreamGetWrapIndex((streamPtr)))
#define rvSrtpDbStreamClearContext(streamPtr)         (rvSrtpStreamClearContext((streamPtr)))
#define rvSrtpDbStreamTestReplayList(streamPtr, i, u) (rvSrtpStreamTestReplayList((streamPtr), (i), (u)))

/* Context functions */
RvSrtpContext *rvSrtpDbContextAdd(RvSrtpDb *thisPtr, RvSrtpStream *stream, RvUint64 startIndex, RvSrtpKey *key, RvBool trigger);
RvBool rvSrtpDbContextRemove(RvSrtpDb *thisPtr, RvSrtpContext *context);
RvSrtpContext *rvSrtpDbContextFind(RvSrtpDb *thisPtr, RvSrtpStream *stream, RvUint64 index);
#define rvSrtpDbContextTrigger(contextPtr)              (rvSrtpStreamTrigger(contextPtr))
#define rvSrtpDbContextGetMasterKey(contextPtr)         (rvSrtpContextGetMasterKey((contextPtr)))
#define rvSrtpDbContextHasCurrentKeys(contextPtr)       (rvSrtpContextHasCurrentKeys((contextPtr)))
#define rvSrtpDbContextHasOldKeys(contextPtr)           (rvSrtpContextHasOldKeys((contextPtr)))
#define rvSrtpDbContextGetCurrentEncryptKey(contextPtr) (rvSrtpContextGetCurrentEncryptKey((contextPtr)))
#define rvSrtpDbContextGetOldEncryptKey(contextPtr)     (rvSrtpContextGetOldEncryptKey((contextPtr)))
#define rvSrtpDbContextGetCurrentAuthKey(contextPtr)    (rvSrtpContextGetCurrentAuthKey((contextPtr)))
#define rvSrtpDbContextGetOldAuthKey(contextPtr)        (rvSrtpContextGetOldAuthKey((contextPtr)))
#define rvSrtpDbContextGetCurrentSalt(contextPtr)       (rvSrtpContextGetCurrentSalt((contextPtr)))
#define rvSrtpDbContextGetOldSalt(contextPtr)           (rvSrtpContextGetOldSalt((contextPtr)))
#define rvSrtpDbContextGetIsRtp(contextPtr)             (rvSrtpStreamGetIsRtp(rvSrtpStreamGetFromContext((contextPtr))))
#define rvSrtpDbContextGetMaxLimit(contextPtr)          (rvSrtpKeyGetMaxLimit(rvSrtpContextGetMasterKey((contextPtr))))
#define rvSrtpDbContextGetKeyDerivationRate(contextPtr) (rvSrtpKeyGetKeyDerivationRate(rvSrtpContextGetMasterKey((contextPtr))))
#define rvSrtpDbContextGetNeverUsed(contextPtr)         (rvSrtpContextGetNeverUsed((contextPtr)))
#define rvSrtpDbContextGetWrapIndex(contextPtr)         (rvSrtpStreamGetWrapIndex(rvSrtpStreamGetFromContext((contextPtr))))
#define rvSrtpDbContextUpdateKeys(contextPtr, i, c)     (rvSrtpContextUpdateKeys((contextPtr), (i), rvSrtpDbContextGetWrapIndex((contextPtr)), (c)))
#define rvSrtpDbContextEnableCurrentKeys(contextPtr, i) (rvSrtpContextEnableCurrentKeys((contextPtr), (i), rvSrtpDbContextGetWrapIndex((contextPtr))))
#define rvSrtpDbContextIncEncryptCount(contextPtr)      (rvSrtpStreamIncEncryptCount((contextPtr)))

#if defined(RV_TEST_CODE)
#include "rvrtpstatus.h"
RvRtpStatus RvSrtpDbTest();
#endif /* RV_TEST_CODE */


/* Function Docs */
/*$
{function scope="private":
    {name:    rvSrtpDbConstruct}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function constructs a database with the given key and replay list sizes.}
    }
    {proto: RvSrtpDb *rvSrtpDbConstruct(RvSrtpDb *thisPtr, RvSize_t masterKeySize, RvSize_t masterSaltSize, RvSize_t mkiSize, RvSize_t rtpEncryptKeySize, RvSize_t rtpAuthKeySize, RvSize_t rtpSaltSize, RvSize_t rtcpEncryptKeySize, RvSize_t rtcpAuthKeySize, RvSize_t rtcpSaltSize, RvUint64 rtpReplayListSize, RvUint64 rtcpReplayListSize, RvUint64 srtpHistorySize, RvUint64 srtcpHistorySize, RvSrtpDbPoolConfig *poolConfigPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object to construct.}}
        {param: {n:masterKeySize} {d:The size of a master key.}}
        {param: {n:masterSaltSize} {d:The size of a master salt.}}
        {param: {n:mkiSize} {d:The size of an MKI.}}
        {param: {n:rtpEncryptKeySize} {d:The size of an RTP encryption key.}}
        {param: {n:rtpAuthKeySize} {d:The size of an RTP authentication key.}}
        {param: {n:rtpSaltSize} {d:The size of an RTP salt.}}
        {param: {n:rtcpEncryptKeySize} {d:The size of an RTCP encryption key.}}
        {param: {n:rtcpAuthKeySize} {d:The size of an RTCP authentication key.}}
        {param: {n:rtcpSaltSize} {d:The size of an RTCP salt.}}
        {param: {n:rtpReplayListSize} {d:The size of a replay list (in packets).}}
        {param: {n:rtcpReplayListSize} {d:The size of a replay list (in packets).}}
        {param: {n:rtpHistorySize} {d:The SRTP context history size (in packets).}}
        {param: {n:rtcpHistorySize} {d:The SRTCP context history size (in packets).}}
        {param: {n:poolConfigPtr} {d:Pool configuration info.}}
    }
    {returns: The master key constructed or NULL if there was an error. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbDestruct}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function destructs a database.}
    }
    {proto: RvBool rvSrtpDbDestruct (RvSrtpDb *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object to destruct.}}
    }
    {returns: The RV_TRUE if it was destructed, RV_FALSE if it could not be destructed. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbGetMasterKeySize}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function get the size of a master key in the master key object.}
    }
    {proto: RvSize_t rvSrtpDbGetMasterKeySize(RvSrtpDb *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
    }
    {returns: The master key size. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbGetMasterSaltSize}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function get the size of a master salt in the master key object.}
    }
    {proto: RvSize_t rvSrtpDbGetMasterSaltSize(RvSrtpDb *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
    }
    {returns: The master salt size. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbGetMkiSize}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function get the size of an MKI in the master key object.}
    }
    {proto: RvSize_t rvSrtpDbGetMkiSize(RvSrtpDb *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
    }
    {returns: The MKI size. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbGetSrtpEncryptKeySize}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function get the size of an encryption key in the context object.}
    }
    {proto: RvSize_t rvSrtpDbGetSrtpEncryptKeySize(RvSrtpDb *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
    }
    {returns: The encryption key size. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbGetSrtpAuthKeySize}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function get the size of an authentication key in the context object.}
    }
    {proto: RvSize_t rvSrtpDbGetSrtpAuthKeySize(RvSrtpDb *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
    }
    {returns: The authentication key size. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbGetSrtpSaltSize}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function get the size of a salt in the context object.}
    }
    {proto: RvSize_t rvSrtpDbGetSrtpSaltSize(RvSrtpDb *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
    }
    {returns: The salt size. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbGetSrtcpEncryptKeySize}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function get the size of an encryption key in the context object.}
    }
    {proto: RvSize_t rvSrtpDbGetSrtcpEncryptKeySize(RvSrtpDb *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
    }
    {returns: The encryption key size. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbGetSrtcpAuthKeySize}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function get the size of an authentication key in the context object.}
    }
    {proto: RvSize_t rvSrtpDbGetSrtcpAuthKeySize(RvSrtpDb *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
    }
    {returns: The authentication key size. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbGetSrtcpSaltSize}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function get the size of a salt in the context object.}
    }
    {proto: RvSize_t rvSrtpDbGetSrtcpSaltSize(RvSrtpDb *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
    }
    {returns: The salt size. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbGetRtpReplayListSize}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function get the size of an RTP replay list in the stream
			object (for remote sources only).}
    }
    {proto: RvUint64 rvSrtpDbGetRtpReplayListSize(RvSrtpDb *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
    }
    {returns: The RTP replay list size. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbGetRtcpReplayListSize}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function get the size of an RTCP replay list in the stream
			object (for remote sources only).}
    }
    {proto: RvUint64 rvSrtpDbGetRtcpReplayListSize(RvSrtpDb *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
    }
    {returns: The RTP replay list size. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbGetRtpHistorySize}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function get the size of a the RTP stream history window.}
    }
    {proto: RvSize_t rvSrtpDbGetRtpHistorySize(RvSrtpDb *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
    }
    {returns: The SRTP history window size. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbGetRtcpHistorySize}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function get the size of a the RTCP stream history window.}
    }
    {proto: RvSize_t rvSrtpDbGetRtcpHistorySize(RvSrtpDb *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
    }
    {returns: The SRTCP history window size. }
}
$*/
/*$
{function scope="private":
	{name: rvSrtpDbGetKeyHash}
	{class: RvSrtpDb}
	{include: rvsrtpdb.h}
	{description:
		{p: This fucntion gets the pointer to the master key hash.}
	}
	{proto: RvObjHash *rvSrtpDbGetKeyHash (RvSrtpDb *thisPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvSrtpDb object.}}
	}
	{returns: The pointer to the hash, or NULL}
}
$*/
/*$
{function scope="private":
	{name: rvSrtpDbGetSrcHash}
	{class: RvSrtpDb}
	{include: rvsrtpdb.h}
	{description:
		{p: This fucntion gets the pointer to the sources hash.}
	}
	{proto: RvObjHash *rvSrtpDbGetSrcHash (RvSrtpDb *thisPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvSrtpDb object.}}
	}
	{returns: The pointer to the hash, or NULL}
}
$*/
/*$
{function scope="private":
	{name: rvSrtpDbGetDstHash}
	{class: RvSrtpDb}
	{include: rvsrtpdb.h}
	{description:
		{p: This fucntion gets the pointer to the destinations hash.}
	}
	{proto: RvObjHash *rvSrtpDbGetDstHash (RvSrtpDb *thisPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvSrtpDb object.}}
	}
	{returns: The pointer to the hash, or NULL}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbKeyAdd}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function adds a new master key to the database. The
			MKI value must be unique or the key will be rejected.}
        {p: The key derivation rate is how often the session keys will
			be regenerated from this master key (based on index mod
			rate). A value of 0 indicates that session keys should only
			be derived once.}
		{p: The key values will by copied into the master key object.}
		{p: The values of a master key object MUST never be modified.}
    }
    {proto: RvSrtpKey *rvSrtpDbKeyAdd(RvSrtpDb *thisPtr, RvUint8 *mki, RvUint8 *masterKey, RvUint8 *masterSalt, RvSize_t keyDerivationRate);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
        {param: {n:mki} {d:The MKI.}}
        {param: {n:masterKey} {d:The master key.}}
        {param: {n:masketSalt} {d:The master salt.}}
        {param: {n:keyDerivationRate} {d:The key derivation rate for this key (0 = none).}}
    }
    {returns: The new master key object, or NULL if it could not be added. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbKeyRemove}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function removes a master key from the database. Any
			contexts dervied from this master key will also be removed.}
    }
    {proto: RvBool rvSrtpDbKeyRemove(RvSrtpDb *thisPtr, RvSrtpKey *key);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
        {param: {n:key} {d:The master key object to be removed.}}
    }
    {returns: Rv_TRUE if the key has been removed, otherwise RV_FALSE.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbKeyFind}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function finds a master key in the database.}
		{p: The MKI of a master key object MUST never be modified.}
    }
    {proto: RvSrtpKey *rvSrtpDbKeyFind(RvSrtpDb *thisPtr, RvUint8 *mki);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
        {param: {n:mki} {d:The MKI.}}
    }
    {returns: The master key object, or NULL if it could not be found. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbKeyGetMasterKey}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function gets the master key string for the specified
			master key object.}
    }
    {proto: RvUint8 *rvSrtpDbKeyGetMasterKey(RvSrtpKey *keyPtr);}
    {params:
        {param: {n:keyPtr} {d:The master key object.}}
    }
    {returns: The master key string.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbKeyGetMasterSalt}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function gets the master salt string for the specified
			master key object.}
    }
    {proto: RvUint8 *rvSrtpDbKeyGetMasterSalt(RvSrtpKey *keyPtr);}
    {params:
        {param: {n:keyPtr} {d:The master key object.}}
    }
    {returns: The master salt string.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbKeyGetMki}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function gets the MKI string for the specified
			master key object.}
    }
    {proto: RvUint8 *rvSrtpDbKeyGetMki(RvSrtpKey *keyPtr);}
    {params:
        {param: {n:keyPtr} {d:The master key object.}}
    }
    {returns: The MKI string.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbKeyGetContextListSize}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function gets the number of current contexts that are derived from this master key object.}
    }
    {proto: RvSize_t rvSrtpDbKeyGetContextListSize(RvSrtpKey *keyPtr);}
    {params:
        {param: {n:keyPtr} {d:The master key object.}}
    }
    {returns: The context list size.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbSourceAdd}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function adds a new remote source to the database. The
			combination of ssrc and isRtp flag must be unique or the
			source will be rejected.}
		{p: The ssrc, isRtp flag, and isRemote flag of a source object MUST never be modified.}
    }
    {proto: RvSrtpStream *rvSrtpDbSourceAdd(RvSrtpDb *thisPtr, RvUint32 ssrc, RvBool isRtp, RvUint64 initIndex);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
        {param: {n:ssrc} {d:The SSRC.}}
        {param: {n:isRtp} {d:RV_TRUE if the source stream is RTP, RV_FALSE for RTCP.}}
        {param: {n:initIndex} {d:The initial index that will be received from the remote source.}}
    }
    {returns: The new remote source stream object, or NULL if it could not be added. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbSourceRemove}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function removes a remote source from the database.}
    }
    {proto: RvBool rvSrtpDbSourceRemove(RvSrtpDb *thisPtr, RvSrtpStream *stream);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
        {param: {n:stream} {d:The remote source stream object to be removed.}}
    }
    {returns: Rv_TRUE if the stream has been removed, otherwise RV_FALSE.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbSourceFind}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function finds a remote source in the database.}
		{p: The ssrc, isRtp flag, and isRemote flag of a source object MUST never be modified.}
    }
    {proto: RvSrtpStream *rvSrtpDbSourceFind(RvSrtpDb *thisPtr, RvUint32 ssrc, RvBool isRtp);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
        {param: {n:ssrc} {d:The SSRC.}}
        {param: {n:isRtp} {d:RV_TRUE if the source stream is RTP, RV_FALSE for RTCP.}}
    }
    {returns: The remote source stream object, or NULL if it could not be found. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbDestAdd}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function adds a new destination to the database. The
			combination of ssrc and address must be unique or the
			destination will be rejected.}
		{p: The ssrc, address, and isRemote flag of a destination object MUST never be modified.}
    }
    {proto: RvSrtpStream *rvSrtpDbDestAdd(RvSrtpDb *thisPtr, RvUint32 ssrc, RvBool isRtp, RvUint64 initIndex, RvAddress *address);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
        {param: {n:ssrc} {d:The SSRC.}}
        {param: {n:isRtp} {d:RV_TRUE if the source stream is RTP, RV_FALSE for RTCP.}}
        {param: {n:initIndex} {d:The initial index that will be received from the remote source.}}
        {param: {n:address} {d:The destination address.}}
    }
    {returns: The new destination stream object, or NULL if it could not be added. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbDestRemove}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function removes a destination from the database.}
    }
    {proto: RvBool rvSrtpDbDestRemove(RvSrtpDb *thisPtr, RvSrtpStream *stream);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
        {param: {n:stream} {d:The destination stream object to be removed.}}
    }
    {returns: Rv_TRUE if the stream has been removed, otherwise RV_FALSE.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbDestFind}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function finds a destination in the database.}
		{p: The ssrc, address, and isRemote flag of a destination object MUST never be modified.}
    }
    {proto: RvSrtpStream *rvSrtpDbDestFind(RvSrtpDb *thisPtr, RvUint32 ssrc, RvAddress *address);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
        {param: {n:ssrc} {d:The SSRC.}}
        {param: {n:address} {d:The destination address.}}
    }
    {returns: The destination stream object, or NULL if it could not be found. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbStreamUpdateMaxIndex}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
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
    {proto: RvBool rvSrtpDbStreamUpdateMaxIndex(RvSrtpStream *stream, RvUint64 newIndex);}
    {params:
        {param: {n:stream} {d:The stream object to update.}}
        {param: {n:index} {d:The new maxIndex value.}}
    }
    {returns: RV_TRUE if the index was updated, RV_FALSE if not (index out of range or old).}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbStreamGetIsRtp}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function finds out if the specified stream is an RTP
			stream or an RTCP stream.}
    }
    {proto: RvBool rvSrtpDbStreamGetIsRtp(RvSrtpContext *streamPtr);}
    {params:
        {param: {n:streamPtr} {d:The stream to check for RTP or RTCP.}}
    }
    {returns: RV_TRUE if the context is for RTP, RV_FALSE for RTCP.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbStreamGetAddress}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function returns a pointer to the address that has
			been set for the stream. Note that only destination streams
			will have addresses set.}
		{p: Never directly modify the address of a stream.}
    }
    {proto: RvAddress *rvSrtpDbStreamGetAddress(RvSrtpContext *streamPtr);}
    {params:
        {param: {n:streamPtr} {d:The stream to get the address for.}}
    }
    {returns: Pointer to the address data of the stream.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbStreamGetMaxIndex}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function gets the current maxIndex value of a stream.
			This value is the highest index value processed for the
			stream.}
    }
    {proto: RvUint64 rvSrtpDbStreamGetMaxIndex(RvSrtpContext *streamPtr);}
    {params:
        {param: {n:streamPtr} {d:The context to get the maxIndex for.}}
    }
    {returns: The maxIndex value.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbStreamGetSsrc}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function gets the current SSRC of a stream.}
    }
    {proto: RvUint32 rvSrtpDbStreamGetSsrc(RvSrtpContext *streamPtr);}
    {params:
        {param: {n:streamPtr} {d:The context to get the SSRC for.}}
    }
    {returns: The SSRC value.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbStreamGetWrapIndex}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function gets the maximum index value of the stream,
			after which this stream will wrap back to 0.}
    }
    {proto: RvUint64 rvSrtpDbStreamGetWrapIndex(RvSrtpStream *streamPtr);}
    {params:
        {param: {n:streamPtr} {d:The RvSrtpStream object.}}
    }
    {returns: The index wrap value.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbStreamClearContext}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function clears all contexts in the history timeline
			for the specified stream.}
    }
    {proto: RvBool rvSrtpDbStreamClearContext(RvSrtpContext *streamPtr);}
    {params:
        {param: {n:streamPtr} {d:The stream to clear.}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbStreamTestReplayList}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function tests the specified index against the stream's
			replay list. If an index is older (within the history size)
			than the size of the replay list, it will be considered to
			be a duplicate and indicate that the packet should not be
			processed.}
		{p: If the index has not been seen yet and should be processed,
			RV_TRUE is returned. If not, then RV_FALSE is returned.}
		{p: If update is set to RV_TRUE and the specified index is in
			the replay list, then the replay list will be updated to
			indicate that this index has been preocessed.}
    }
    {proto: RvBool rvSrtpDbStreamTestReplayList(RvSrtpStream streamPtr, RvUint64 index, RvBool update);}
    {params:
        {param: {n:streamPtr} {d:The RvSrtpStream object to check against.}}
        {param: {n:index} {d:The index to test.}}
        {param: {n:update} {d:RV_TRUE is the specified index should be marked in the replay list, otherwise RV_FALSE.}}
    }
    {returns: RV_TRUE if the packet should be accepted, otherwise RV_FALSE}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbContextAdd}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function adds a new context to a stream based on the
			index. The index indicates the point at which the context
			should start being used.}
		{p: The startIndex and stream of a context object MUST never be modified.}
    }
    {proto: RvSrtpContext *rvSrtpDbContextAdd(RvSrtpDb *thisPtr, RvSrtpStream *stream, RvUint64 startIndex, RvSrtpKey *key, RvBool trigger);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
        {param: {n:stream} {d:The stream object to add the context to.}}
        {param: {n:startIndex} {d:The index at which this context should start being used.}}
        {param: {n:key} {d:The master key object the context is derived from.}}
        {param: {n:trigger} {d:RV_TRUE if context should trigger other contexts to switch keys at the same
                 time, RV_FALSE otherwise.}}
    }
    {returns: The new context object, or NULL if it could not be added. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbContextRemove}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function removes a context from the stream is part of.
			Removing a context creates a NULL context where there is no key
			for the range.}
    }
    {proto: RvBool rvSrtpDbContextRemove(RvSrtpDb *thisPtr, RvSrtpContext *context);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
        {param: {n:context} {d:The context object to be removed.}}
    }
    {returns: RV_TRUE if the context has been removed, otherwise RV_FALSE.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbContextFind}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function finds the context of stream that is active
			for the index specified.}
		{p: A NULL result indicates that there is no valid context
			active for the specified index. This can happen if the
			relavent master key was deleted, the index is older than
			the saved history or derivation rate, or the user has
			specified that there is no key for that index.}
		{p: The startIndex and stream of a context object MUST never be modified.}
    }
    {proto: RvSrtpContext *rvSrtpDbContextFind(RvSrtpDb *thisPtr, RvSrtpStream *stream, RvUint64 index);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpDb object.}}
        {param: {n:stream} {d:The stream object to check.}}
        {param: {n:index} {d:The index to find the context for.}}
    }
    {returns: The context object, or NULL if there is no valid context for the specified index.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbContextTrigger}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
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
    {proto: RvBool rvSrtpDbContextTrigger(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:context} {d:The context to trigger from, if necessary.}}
    }
    {returns: RV_TRUE if a trigger occurred, RV_FALSE if there was no trigger.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbContextGetMasterKey}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function gets the master key a context is derived from
			(or will be derived from).}
    }
    {proto: RvSrtpKey *rvSrtpDbContextGetMasterKey(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The context to get the master key from.}}
    }
    {returns: The master key for the context.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbContextHasCurrentKeys}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function finds out if the context has a set of current
			derived keys.}
    }
    {proto: RvBool rvSrtpDbContextHasCurrentKeys(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The RvSrtpContext object.}}
    }
    {returns: RV_TRUE if the context has current keys, RV_FALSE if it does not.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbContextHasOldKeys}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function finds out if the context has a set of old
			derived keys.}
    }
    {proto: RvBool rvSrtpDbContextHasOldKeys(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The RvSrtpContext object.}}
    }
    {returns: RV_TRUE if the context has old keys, RV_FALSE if it does not.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbContextGetCurrentEncryptKey}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function gets the current encyrption key for a context.}
    }
    {proto: RvUint8 *rvSrtpDbContextGetCurrentEncryptKey(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The current encyrption key.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbContextGetOldEncryptKey}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function gets the old encyrption key for a context.}
    }
    {proto: RvUint8 *rvSrtpDbContextGetOldEncryptKey(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The old encyrption key.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbContextGetCurrentAuthKey}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function gets the current authentication key for the context.}
    }
    {proto: RvUint8 *rvSrtpDbContextGetCurrentAuthKey(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The current authentication key.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextGetOldAuthKey}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function gets the old authentication key for a context.}
    }
    {proto: RvUint8 *rvSrtpDbContextGetOldAuthKey(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The old authentication key.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextGetCurrentSalt}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function gets the current salt for a context.}
    }
    {proto: RvUint8 *rvSrtpDbContextGetCurrentSalt(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The current salt.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpContextGetOldSalt}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function gets the old salt for a context.}
    }
    {proto: RvUint8 *rvSrtpDbContextGetOldSalt(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The old salt.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbContextGetIsRtp}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function finds out if the context is part of an RTP
			stream or an RTCP stream.}
    }
    {proto: RvBool rvSrtpDbContextGetIsRtp(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The RvSrtpContext object.}}
    }
    {returns: RV_TRUE if it is part of an RTP stream, RV_FALSE if part of an RTCP stream.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbContextGetMaxLimit}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function finds out if the master key that the context
			is derived from has hit its limit of the number of times the
			key may be used.}
    }
    {proto: RvBool rvSrtpDbContextGetMaxLimit(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The RvSrtpContext object.}}
    }
    {returns: RV_TRUE if the key's maximum has been reached, RV_FALSE if the key may still be used.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbContextGetKeyDerivationRate}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function get the key derivation rate of the master key
			used by the specified context.}
    }
    {proto: RvSize_t rvSrtpDbContextGetKeyDerivationRate(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The key derivation rate.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbContextGetNeverUsed}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function return RV_TRUE if the context has never had a
			set of keys generated for it.}
    }
    {proto: RvBool rvSrtpDbContextGetNeverUsed(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The RvSrtpContext object.}}
    }
    {returns: RV_TRUE if no keys have ever been generated for this context, other RV_FALSE.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpDbContextGetWrapIndex}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
    {description:
        {p: This function gets the maximum index value of the stream
			that the context is part of, after which this stream will
			wrap back to 0.}
    }
    {proto: RvUint64 rvSrtpDbContextGetWrapIndex(RvSrtpContext *contextPtr);}
    {params:
        {param: {n:contextPtr} {d:The RvSrtpContext object.}}
    }
    {returns: The index wrap value.}
}
$*/
/*$
{function scope="private":
	{name:    rvSrtpDbContextUpdateKeys}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
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
			rvSrtpDbContextHasCurrentKeys function to determine if current
			keys need to be generated (a value of RV_FALSE indicates keys
			need to be generated). You can use the
			rvSrtpDbContextGetNeverUsed function to determine if these are
			the first keys for the context. Be sure to use the
			rvSrtpDbConextEnableCurrentKeys after generating new keys.}
	}
    {proto: RvBool rvSrtpDbContextUpdateKeys(RvSrtpContext *contextPtr, RvUint64 index, RvBool *isCurrent);}
    {params:
		{param: {n:thisPtr} {d:The RvSrtpContext object which corresponds to the index.}}
		{param: {n:index} {d:The index of the packet being processed.}}
		{param: {n:*isCurrent} {d:Output: will be set to RV_TRUE if the current keys should be used, RV_FALSE if the old keys should be used.}}
	}
    {returns: The RV_TRUE if the specified index can be processed, RV_FALSE if it must be discarded.}
}
$*/
/*$
{function scope="private":
	{name:    rvSrtpDbContextEnableCurrentKeys}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
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
    {proto: void rvSrtpDbConextEnableCurrentKeys(RvSrtpContext *contextPtr, RvUint64 index);}
    {params:
		{param: {n:contextPtr} {d:The RvSrtpContext object.}}
		{param: {n:index} {d:The index used to generate the current keys.}}
	}
}
$*/
/*$
{function scope="private":
	{name:    rvSrtpDbContextIncEncryptCount}
    {class:   RvSrtpdb}
    {include: rvsrtpdb.h}
	{description:
		{p: This increments the count of the number of packets a given
			stream has encrypted with the same master key. If the maximum
			allowed packets it reached, the master key will be marked as
			reaching its limit and will not be allowed to be used to
			encrypt any further packets.}
	}
    {proto: void rvSrtpDbContextIncEncryptCount(RvSrtpContext *contextPtr);}
    {params:
		{param: {n:contextPtr} {d:The RvSrtpContext object.}}
	}
}
$*/

#if defined(__cplusplus)
}
#endif

#endif
