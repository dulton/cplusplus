/************************************************************************
 File Name	   : rvsrtpdb.c
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
#include "rvsrtpdb.h"
#include "rvsrtplog.h"
#include "rv64ascii.h"

static void *rvSrtpKeyHashMalloc (RvSize_t size, void *data);
static void rvSrtpKeyHashFree (void *ptr, void *data);
static RvInt rvSrtpKeyHashItemCmp (void *ptr1, void *ptr2, void *data);
static RvSize_t rvSrtpKeyHashGetHash (void *item, void *data);
static void *rvSrtpSrcHashMalloc (RvSize_t size, void *data);
static void rvSrtpSrcHashFree (void *ptr, void *data);
static RvInt rvSrtpSrcHashItemCmp (void *ptr1, void *ptr2, void *data);
static RvSize_t rvSrtpSrcHashGetHash (void *item, void *data);
static void *rvSrtpDstHashMalloc (RvSize_t size, void *data);
static void rvSrtpDstHashFree (void *ptr, void *data);
static RvInt rvSrtpDstHashItemCmp (void *ptr1, void *ptr2, void *data);
static RvSize_t rvSrtpDstHashGetHash (void *item, void *data);

RvSrtpDb *rvSrtpDbConstruct (RvSrtpDb *thisPtr, RvSize_t masterKeySize, RvSize_t masterSaltSize,
                             RvSize_t mkiSize, RvSize_t rtpEncryptKeySize, RvSize_t rtpAuthKeySize,
                             RvSize_t rtpSaltSize, RvSize_t rtcpEncryptKeySize, RvSize_t rtcpAuthKeySize,
                             RvSize_t rtcpSaltSize, RvUint64 rtpReplayListSize, RvUint64 rtcpReplayListSize,
                             RvUint64 srtpHistorySize, RvUint64 srtcpHistorySize, RvSrtpDbPoolConfig *poolConfigPtr)
{
    RvObjHashFuncs     keyHashCB;
    RvObjHashFuncs     dstHashCB;
    RvObjHashFuncs     srcHashCB;
    RvSrtpStream       tmpStream;
    RvSrtpKey          tmpKey;
    RvSrtpContextPool *tmpContextPool;
    RvSrtpStreamPool  *tmpStreamPool;
    RvSrtpKeyPool     *tmpKeyPool;
    RvObjHash         *tmpKeyHash;
    RvObjHash         *tmpSrcHash;
    RvObjHash         *tmpDstHash;


    if ((NULL == thisPtr) || (NULL == poolConfigPtr))
        return (NULL);

    /* init parameters */
    thisPtr->masterKeySize  = masterKeySize;
    thisPtr->masterSaltSize = masterSaltSize;
    thisPtr->mkiSize        = mkiSize;
    thisPtr->srtpEncryptKeySize  = rtpEncryptKeySize;
    thisPtr->srtpAuthKeySize     = rtpAuthKeySize;
    thisPtr->srtpSaltSize        = rtpSaltSize;
    thisPtr->srtcpEncryptKeySize = rtcpEncryptKeySize;
    thisPtr->srtcpAuthKeySize    = rtcpAuthKeySize;
    thisPtr->srtcpSaltSize       = rtcpSaltSize;
    thisPtr->srtpReplayListSize  = rtpReplayListSize;
    thisPtr->srtcpReplayListSize = rtcpReplayListSize;
	thisPtr->srtpHistorySize     = srtpHistorySize;
	thisPtr->srtcpHistorySize    = srtcpHistorySize;

    /* ---- construct pools ---- */
    /* key pool */
    tmpKeyPool = rvSrtpKeyPoolConstruct (&(thisPtr->keyPool), masterKeySize, masterSaltSize, mkiSize,
                                         poolConfigPtr->keyRegion, poolConfigPtr->keyPageItems,
                                         poolConfigPtr->keyPageSize, poolConfigPtr->keyPoolType,
                                         poolConfigPtr->keyMaxItems, poolConfigPtr->keyMinItems,
                                         poolConfigPtr->keyFreeLevel);
    if (NULL == tmpKeyPool)
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbConstruct: Failed to construct key pool"));
        return (NULL);
    }
    else {
        rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbConstruct: Key pool created"));
    }

	/* context pool */
	tmpContextPool = rvSrtpContextPoolConstruct (&(thisPtr->contextPool),
		RvMax(rtpEncryptKeySize, rtcpEncryptKeySize),
		RvMax(rtpAuthKeySize, rtcpAuthKeySize),
		RvMax(rtpSaltSize, rtcpSaltSize),
		poolConfigPtr->contextRegion,
		poolConfigPtr->contextPageItems,
		poolConfigPtr->contextPageSize,
		poolConfigPtr->contextPoolType,
		poolConfigPtr->contextMaxItems,
		poolConfigPtr->contextMinItems,
		poolConfigPtr->contextFreeLevel);
	if (NULL == tmpContextPool)
	{
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbConstruct: Failed to construct context pool"));
		rvSrtpStreamPoolDestruct (&thisPtr->streamPool);
		rvSrtpKeyPoolDestruct (&thisPtr->keyPool);
		return (NULL);
	}
   else {
		rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbConstruct: Context pool created"));
   }

    /* stream pool */
    tmpStreamPool = rvSrtpStreamPoolConstruct (&(thisPtr->streamPool),
                                               RvMax(rtpReplayListSize, rtcpReplayListSize), &thisPtr->contextPool,
                                               poolConfigPtr->streamRegion, poolConfigPtr->streamPageItems,
                                               poolConfigPtr->streamPageSize, poolConfigPtr->streamPoolType,
                                               poolConfigPtr->streamMaxItems, poolConfigPtr->streamMinItems,
                                               poolConfigPtr->streamFreeLevel);
    if (NULL == tmpStreamPool)
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr,"rvSrtpDbConstruct: Failed to construct stream pool"));
        rvSrtpKeyPoolDestruct (&thisPtr->keyPool);
        return (NULL);
    }
    else {
        rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbConstruct: Stream pool created"));
    }


    /* ---- construct hashs ---- */
    /* key hash */
    keyHashCB.memalloc = rvSrtpKeyHashMalloc;
    keyHashCB.memfree  = rvSrtpKeyHashFree;
    keyHashCB.itemcmp  = rvSrtpKeyHashItemCmp;
    keyHashCB.gethash  = rvSrtpKeyHashGetHash;
    keyHashCB.memallocdata = poolConfigPtr->keyHashRegion;
    keyHashCB.memfreedata  = NULL;
    keyHashCB.itemcmpdata  = thisPtr;
    keyHashCB.gethashdata  = thisPtr;
    tmpKeyHash = RvObjHashConstruct (&(thisPtr->keyHash), &tmpKey, rvSrtpKeyGetHashElem (&tmpKey),
                                     poolConfigPtr->keyHashType, poolConfigPtr->keyHashStartSize, &keyHashCB);
    if (NULL == tmpKeyHash)
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbConstruct: Failed to construct key hash"));
        rvSrtpContextPoolDestruct (&(thisPtr->contextPool));
        rvSrtpStreamPoolDestruct (&(thisPtr->streamPool));
        rvSrtpKeyPoolDestruct (&(thisPtr->keyPool));
        return (NULL);
    }
    else {
        rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbConstruct: Key hash created"));
    }


    /* source hash */
    srcHashCB.memalloc = rvSrtpSrcHashMalloc;
    srcHashCB.memfree  = rvSrtpSrcHashFree;
    srcHashCB.itemcmp  = rvSrtpSrcHashItemCmp;
    srcHashCB.gethash  = rvSrtpSrcHashGetHash;
    srcHashCB.memallocdata = poolConfigPtr->sourceHashRegion;
    srcHashCB.memfreedata  = NULL;
    srcHashCB.itemcmpdata  = NULL;
    srcHashCB.gethashdata  = NULL;
    tmpSrcHash = RvObjHashConstruct (&(thisPtr->sourceHash), &tmpStream, rvSrtpStreamGetHashElem (&tmpStream),
                                     poolConfigPtr->sourceHashType, poolConfigPtr->sourceHashStartSize,
                                     &srcHashCB);
    if (NULL == tmpSrcHash)
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbConstruct: Failed to construct source hash"));
        RvObjHashDestruct (&(thisPtr->keyHash));
        rvSrtpContextPoolDestruct (&(thisPtr->contextPool));
        rvSrtpStreamPoolDestruct (&(thisPtr->streamPool));
        rvSrtpKeyPoolDestruct (&(thisPtr->keyPool));
        return (NULL);
    }
    else {
        rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbConstruct: Source hash created"));
    }

    /* destination hash */
    dstHashCB.memalloc = rvSrtpDstHashMalloc;
    dstHashCB.memfree  = rvSrtpDstHashFree;
    dstHashCB.itemcmp  = rvSrtpDstHashItemCmp;
    dstHashCB.gethash  = rvSrtpDstHashGetHash;
    dstHashCB.memallocdata = poolConfigPtr->destHashRegion;
    dstHashCB.memfreedata  = NULL;
    dstHashCB.itemcmpdata  = NULL;
    dstHashCB.gethashdata  = NULL;
    tmpDstHash = RvObjHashConstruct (&(thisPtr->destHash), &tmpStream, rvSrtpStreamGetHashElem (&tmpStream),
                                     poolConfigPtr->destHashType, poolConfigPtr->destHashStartSize,
                                     &dstHashCB);
    if (NULL == tmpDstHash)
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbConstruct: Failed to construct destination hash"));
        RvObjHashDestruct (&(thisPtr->sourceHash));
        RvObjHashDestruct (&(thisPtr->keyHash));
        rvSrtpContextPoolDestruct (&(thisPtr->contextPool));
        rvSrtpStreamPoolDestruct (&(thisPtr->streamPool));
        rvSrtpKeyPoolDestruct (&(thisPtr->keyPool));
        return (NULL);
    }
    else {
        rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbConstruct: Dest hash created"));
    }


    return (thisPtr);
} /* rvSrtpDbConstruct */

RvBool rvSrtpDbDestruct (RvSrtpDb *thisPtr)
{
    RvBool     srcRes;
    RvBool     dstRes;
    RvBool     keyRes;
    RvObjHash *keyHash;
    RvObjHash *srcHash;
    RvObjHash *dstHash;
    void      *tmpEntry;

    if (NULL == thisPtr)
        return (RV_FALSE);

    srcRes = dstRes = keyRes = RV_TRUE;

    keyHash = &(thisPtr->keyHash);
    srcHash = &(thisPtr->sourceHash);
    dstHash = &(thisPtr->destHash);

    /* go over source hash, dest hash, and key hash, and remove all entries */
    /* 1. go over source hash */
    while (NULL != (tmpEntry = RvObjHashGetNext (srcHash, NULL)))
    {
        if (RV_FALSE == rvSrtpDbSourceRemove (thisPtr, (RvSrtpStream *)tmpEntry))
        {
            rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbDestruct: failed to remove source entry %x",
                             (RvInt)tmpEntry));
            srcRes = RV_FALSE;
        }
    }
	/* destruct source hash */
	RvObjHashDestruct (srcHash);

    /* 2. go over dest hash */
    while (NULL != (tmpEntry = RvObjHashGetNext (dstHash, NULL)))
    {
        if (RV_FALSE == rvSrtpDbDestRemove (thisPtr, (RvSrtpStream *)tmpEntry))
        {
            rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbDestruct: Failed to remove destination entry %x",
                             (RvInt)tmpEntry));
            dstRes = RV_FALSE;
        }
    }
	/* destruct dest hash */
	RvObjHashDestruct (dstHash);

	/* 3. go over key hash */
	while (NULL != (tmpEntry = RvObjHashGetNext (keyHash, NULL)))
	{
		if (RV_FALSE == rvSrtpDbKeyRemove (thisPtr, (RvSrtpKey *)tmpEntry))
		{
			rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbDestruct: Failed to remove key entry %x",
							 (RvInt)tmpEntry));
			keyRes = RV_FALSE;
		}
	}
	/* destruct key hash */
	RvObjHashDestruct (keyHash);

    /* destruct stream pool */
    if ((RV_TRUE == srcRes) && (RV_TRUE == dstRes))
    {
        if (RV_FALSE == rvSrtpStreamPoolDestruct (&(thisPtr->streamPool)))
        {
            rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbDestruct: Failed to destruct stream pool"));
        }
    }
    else
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbDestruct: Stream pool not empty, not destructing"));
    }

	/* all contexts should be freed so we can destruct the context pool */
	if ((RV_TRUE == srcRes) && (RV_TRUE == dstRes) && (RV_TRUE == keyRes))
	{
		if (RV_FALSE == rvSrtpContextPoolDestruct (&(thisPtr->contextPool)))
		{
			rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbDestruct: Failed to destruct context pool"));
		}
	}
	else
	{
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbDestruct: Context pool not empty, not destructing"));
	}

    /* destruct key pool */
    if (RV_TRUE == keyRes)
    {
        if (RV_FALSE == rvSrtpKeyPoolDestruct (&(thisPtr->keyPool)))
        {
            rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbDestruct: Failed to destruct key pool"));
        }
    }
    else
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbDestruct: Key pool not empty, not destructing"));
    }

    if ((RV_TRUE == srcRes) && (RV_TRUE == dstRes) && (RV_TRUE == keyRes))
    {
        rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbDestruct: DB destructed successfully"));
        return (RV_TRUE);
    }
    else
    {
        rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbDestruct: DB not destructed"));
        return (RV_FALSE);
    }

} /* rvSrtpDbDestruct */

/* Get configuration information */
RvSize_t rvSrtpDbGetMasterKeySize (RvSrtpDb *thisPtr)
{
    if (NULL == thisPtr)
        return (0);
    return (thisPtr->masterKeySize);
} /* rvSrtpDbGetMasterKeySize */

RvSize_t rvSrtpDbGetMasterSaltSize (RvSrtpDb *thisPtr)
{
    if (NULL == thisPtr)
        return (0);
    return (thisPtr->masterSaltSize);
} /* rvSrtpDbGetMasterSaltSize */

RvSize_t rvSrtpDbGetMkiSize (RvSrtpDb *thisPtr)
{
    if (NULL == thisPtr)
        return (0);
    return (thisPtr->mkiSize);
} /* rvSrtpDbGetMkiSize */

RvSize_t rvSrtpDbGetSrtpEncryptKeySize (RvSrtpDb *thisPtr)
{
    if (NULL == thisPtr)
        return (0);
    return (thisPtr->srtpEncryptKeySize);
} /* rvSrtpDbGetSrtpEncryptKeySize */

RvSize_t rvSrtpDbGetSrtpAuthKeySize (RvSrtpDb *thisPtr)
{
    if (NULL == thisPtr)
        return (0);
    return (thisPtr->srtpAuthKeySize);
} /* rvSrtpDbGetSrtpAuthKeySize */

RvSize_t rvSrtpDbGetSrtpSaltSize (RvSrtpDb *thisPtr)
{
    if (NULL == thisPtr)
        return (0);
    return (thisPtr->srtpSaltSize);
} /* rvSrtpDbGetSrtpSaltSize */

RvSize_t rvSrtpDbGetSrtcpEncryptKeySize(RvSrtpDb *thisPtr)
{
    if (NULL == thisPtr)
        return (0);
    return (thisPtr->srtcpEncryptKeySize);
} /* rvSrtpDbGetSrtcpEncryptKeySize */

RvSize_t rvSrtpDbGetSrtcpAuthKeySize(RvSrtpDb *thisPtr)
{
    if (NULL == thisPtr)
        return (0);
    return (thisPtr->srtcpAuthKeySize);
} /* rvSrtpDbGetSrtcpAuthKeySize */

RvSize_t rvSrtpDbGetSrtcpSaltSize(RvSrtpDb *thisPtr)
{
    if (NULL == thisPtr)
        return (0);
    return (thisPtr->srtcpSaltSize);
} /* rvSrtpDbGetSrtcpSaltSize */

RvUint64 rvSrtpDbGetRtpReplayListSize (RvSrtpDb *thisPtr)
{
    if (NULL == thisPtr)
        return (0);
    return (thisPtr->srtpReplayListSize);
} /* rvSrtpDbGetRtpReplayListSize */

RvUint64 rvSrtpDbGetRtcpReplayListSize (RvSrtpDb *thisPtr)
{
    if (NULL == thisPtr)
        return (0);
    return (thisPtr->srtcpReplayListSize);
} /* rvSrtpDbGetRtcpReplayListSize */

/* DB functions */
RvObjHash *rvSrtpDbGetKeyHash (RvSrtpDb *thisPtr)
{
    if (NULL == thisPtr)
        return (NULL);

    return (&(thisPtr->keyHash));
} /* rvSrtpDbGetKeyHash */

RvObjHash *rvSrtpDbGetSrcHash (RvSrtpDb *thisPtr)
{
	if (NULL == thisPtr)
		return (NULL);

	return (&(thisPtr->sourceHash));
} /* rvSrtpDbGetSrcHash */

RvObjHash *rvSrtpDbGetDstHash (RvSrtpDb *thisPtr)
{
    if (NULL == thisPtr)
        return (NULL);

    return (&(thisPtr->destHash));
} /* rvSrtpDbGetDstHash */

/* Master key functions */
RvSrtpKey *rvSrtpDbKeyAdd (RvSrtpDb *thisPtr, RvUint8 *mki, RvUint8 *masterKey, RvUint8 *masterSalt, RvSize_t keyDerivationRate
#ifdef UPDATED_BY_SPIRENT
       ,IN RvUint64   lifetime
       ,IN RvUint64   threshold
       ,IN RvUint8    direction
#endif // UPDATED_BY_SPIRENT
       )
{
    RvSrtpKey *allocKey;
    void      *hashRes;

    if (NULL == thisPtr)
        return (NULL);

    /* allocate new key entry */
    allocKey = rvSrtpKeyPoolNew (&(thisPtr->keyPool));
    if (NULL == allocKey)
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbKeyAdd: Failed to allocate key element"));
        return (NULL);
    }

    /* copies keys to entry and set derivation rate */
	memcpy (rvSrtpKeyGetMasterKey(allocKey), masterKey, thisPtr->masterKeySize);
	memcpy (rvSrtpKeyGetMasterSalt(allocKey), masterSalt, thisPtr->masterSaltSize);
	memcpy (rvSrtpKeyGetMki(allocKey), mki, thisPtr->mkiSize);
#ifdef UPDATED_BY_SPIRENT
	allocKey->use_lifetime = lifetime > 0 ? RV_TRUE : RV_FALSE;
	allocKey->rtp_lifetime = lifetime;
	allocKey->rtcp_lifetime = lifetime;
	allocKey->threshold = threshold;
	allocKey->direction = direction;
#endif // UPDATED_BY_SPIRENT
	rvSrtpKeySetKeyDerivationRate(allocKey, keyDerivationRate);
	rvSrtpKeySetMaxLimit(allocKey, RV_FALSE);

    /* add entry to hash */
    hashRes = RvObjHashAddUnique (&(thisPtr->keyHash), allocKey);
    if (NULL == hashRes)
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbKeyAdd: Failed to add key entry %x to hash",
                         (RvInt)allocKey));
        rvSrtpKeyPoolRelease (&(thisPtr->keyPool), allocKey);
        return (NULL);
    }
    else
    {
        /* check if addUnique returned an existing entry, or a new one */
        if (hashRes != allocKey)
        {
            /* entry is already there (hashRes != allocKey, so no actual ADD was made) */
            rvSrtpKeyPoolRelease (&(thisPtr->keyPool), allocKey);
            rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbKeyAdd: Master key %x already exist",
                             (RvInt)hashRes));
            return (hashRes);
        }
    }

    rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbKeyAdd: Master key added - %x", (RvInt)masterKey));
    return (allocKey);
} /* rvSrtpDbKeyAdd */

RvBool rvSrtpDbKeyRemove (RvSrtpDb *thisPtr, RvSrtpKey *key)
{
    RvSrtpContext *context;
    RvObjList     *list;

    /* before the key can be removed, all associated contexts should be removed */
    if ((NULL == thisPtr) || (NULL == key))
        return (RV_FALSE);

    /* get contexts list */
    list = rvSrtpKeyGetContextList (key);
    while ((context = (RvSrtpContext *)RvObjListGetNext (list, NULL, RV_OBJLIST_LEAVE)) != NULL)
    {
		/* Remove context from stream (also removes it from this list) */
       if(rvSrtpStreamRemoveContext(context) == RV_FALSE) {
			rvSrtpLogError ((rvSrtpLogSourcePtr,
							 "rvSrtpDbKeyRemove: Unable to remove context %x from stream",
							 (RvInt)context));
       }
    }

    /* remove key from hash */
    if (NULL == RvObjHashRemove (&(thisPtr->keyHash), key))
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbKeyRemove: Failed to remove key %x from hash",
                         (RvInt)key));
    }

    /* destruct and free the key's memory */
    rvSrtpKeyPoolRelease (&(thisPtr->keyPool), key);
    rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbKeyRemove: Master key removed - %x", (RvInt)key));
    return (RV_TRUE);
} /* rvSrtpDbKeyRemove */

RvSrtpKey *rvSrtpDbKeyFind (RvSrtpDb *thisPtr, RvUint8 *mki
#ifdef UPDATED_BY_SPIRENT
        ,RvUint8 direction
#endif // UPDATED_BY_SPIRENT
)
{
    RvSrtpKey *tmpKey;
    RvUint8   *mkiPtr;
    void      *tmp;

    if ((NULL == thisPtr) || (NULL == mki))
        return (NULL);

    tmpKey = rvSrtpKeyPoolNew (&(thisPtr->keyPool));
    if (NULL == tmpKey)
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbKeyFind: Unable to allocate key element"));
        return (NULL);
    }
    mkiPtr = rvSrtpKeyGetMki (tmpKey);
    memcpy (mkiPtr, mki, thisPtr->mkiSize);
#ifdef UPDATED_BY_SPIRENT
    tmpKey->direction = direction;
#endif // UPDATED_BY_SPIRENT
    tmp = RvObjHashFind (&(thisPtr->keyHash), (void *)tmpKey);

    rvSrtpKeyPoolRelease (&(thisPtr->keyPool), tmpKey);

    return ((RvSrtpKey *)tmp);
} /* rvSrtpDbKeyFind */

/* Remote source functions */
RvSrtpStream *rvSrtpDbSourceAdd (RvSrtpDb *thisPtr, RvUint32 ssrc, RvBool isRtp, RvUint64 initIndex)
{
    RvSrtpStream *allocSrc;
    void         *hashRes;
	RvUint64      historySize;

    if (NULL == thisPtr)
        return (NULL);

	historySize = (isRtp == RV_TRUE) ? thisPtr->srtpHistorySize : thisPtr->srtcpHistorySize;
    allocSrc = rvSrtpStreamPoolNew (&thisPtr->streamPool, initIndex, historySize, isRtp);
    if (NULL == allocSrc)
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbSourceAdd: Failed to allocate source element"));
        return (NULL);
    }

    /* set initial parameters we control */
    rvSrtpStreamSetSsrc (allocSrc, ssrc);
    rvSrtpStreamSetIsRemote (allocSrc, RV_TRUE);

    /* add to src hash */
    hashRes = RvObjHashAddUnique (&(thisPtr->sourceHash), allocSrc);
    if (NULL == hashRes)
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbSourceAdd: Failed to add source entry %x to hash",
                         (RvInt)allocSrc));
        rvSrtpStreamPoolRelease (&(thisPtr->streamPool), allocSrc);
        return (NULL);
    }
    else
    {
        /* if addUnique returned an existing entry, or a new one */
        if (hashRes != allocSrc)
        {
            /* entry is already there (hashRes != allocSrc, so no actual ADD was made) */
            rvSrtpStreamPoolRelease (&(thisPtr->streamPool), allocSrc);
            rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbSourceAdd: source %x already exist",
                             (RvInt)hashRes));
            return (hashRes);
        }
    }

    rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbSourceAdd: source added - %d", ssrc));
    return (allocSrc);
} /* rvSrtpDbSourceAdd */

RvBool rvSrtpDbSourceRemove (RvSrtpDb *thisPtr, RvSrtpStream *stream)
{
    void          *item;
    RvUint32       ssrc;

    if ((NULL == thisPtr) || (NULL == stream))
        return (RV_FALSE);

    /* remove source from the hash */
    item = RvObjHashRemove (&(thisPtr->sourceHash), stream);
    if (NULL == item)
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbSourceRemove: Failed to remove source %x from hash",
                         (RvInt)stream));
    }

    ssrc = rvSrtpStreamGetSsrc (stream);

	/* destruct and free source memory (deletes any remaining contexts) */
    rvSrtpStreamPoolRelease (&(thisPtr->streamPool), stream);
    rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbSourceRemove: source removed - %d", ssrc));

	return (RV_TRUE);
} /* rvSrtpDbSourceRemove */

RvSrtpStream *rvSrtpDbSourceFind (RvSrtpDb *thisPtr, RvUint32 ssrc, RvBool isRtp)
{
    RvSrtpStream tmpStream;
    void        *tmp;

    if (NULL == thisPtr)
        return (NULL);

	/* Dummy stream, just set fields required for search */
    rvSrtpStreamSetSsrc (&tmpStream, ssrc);
    rvSrtpStreamSetIsRtp (&tmpStream, isRtp);

    tmp = RvObjHashFind (&(thisPtr->sourceHash), &tmpStream);

    return ((RvSrtpStream *)tmp);
} /* rvSrtpDbSourceFind */


/* Destination functions */
RvSrtpStream *rvSrtpDbDestAdd (RvSrtpDb *thisPtr, RvUint32 ssrc, RvBool isRtp, RvUint64 initIndex, RvAddress *address)
{
    RvSrtpStream *allocDst;
    RvAddress    *addrPtr;
    void         *hashRes;
	RvUint64      historySize;

    if ((NULL == thisPtr) || (NULL == address))
        return (NULL);

	historySize = (isRtp == RV_TRUE) ? thisPtr->srtpHistorySize : thisPtr->srtcpHistorySize;
	allocDst = rvSrtpStreamPoolNew (&thisPtr->streamPool, initIndex, historySize, isRtp);
    if (NULL == allocDst)
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbDestAdd: Failed to allocate destination element"));
        return (NULL);
    }

    /* set initial parameters we control */
    rvSrtpStreamSetSsrc (allocDst, ssrc);
    addrPtr = rvSrtpStreamGetAddress (allocDst);
    memcpy (addrPtr, address, sizeof (RvAddress));
    rvSrtpStreamSetIsRemote (allocDst, RV_FALSE);

    /* add to dst hash */
    hashRes = RvObjHashAddUnique (&(thisPtr->destHash), allocDst);
    if (NULL == hashRes)
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbDestAdd: Failed to add destination entry %x to hash",
                         (RvInt)allocDst));
        rvSrtpStreamPoolRelease (&(thisPtr->streamPool), allocDst);
        return (NULL);
    }
    else
    {
        /* check if uniqeAdd returned an existing entry or a new one */
        if (hashRes != allocDst)
        {
            /* entry is already there (hashRes != allocDst, so no actual ADD was made) */
            rvSrtpStreamPoolRelease (&(thisPtr->streamPool), allocDst);
            rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbDestAdd: Destination %x already exist",
                             (RvInt)hashRes));
            return (hashRes);


        }
    }

    rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbDestAdd: dest added - %d", ssrc));
    return (allocDst);
} /* rvSrtpDbDestAdd */

RvBool rvSrtpDbDestRemove (RvSrtpDb *thisPtr, RvSrtpStream *stream)
{
    void          *item;
    RvUint32       ssrc;

    if ((NULL == thisPtr) || (NULL == stream))
        return (RV_FALSE);

    /* remove dest from the hash */
    item = RvObjHashRemove (&(thisPtr->destHash), stream);
    if (NULL == item)
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbDestRemove: Failed to remove destination %x from hash",
                        (RvInt)stream));
    }

    ssrc = rvSrtpStreamGetSsrc (stream);

	/* destruct and free dest memory (deletes any remaining contexts) */
    rvSrtpStreamPoolRelease (&(thisPtr->streamPool), stream);
    rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbDestRemove: dest removed - %d", ssrc));

    return (RV_TRUE);
} /* rvSrtpDbDestRemove */

RvSrtpStream *rvSrtpDbDestFind (RvSrtpDb *thisPtr, RvUint32 ssrc, RvAddress *address)
{
    RvSrtpStream tmpStream;
    RvAddress    *adrsPtr;
    void         *tmp;

    if ((NULL == thisPtr) || (NULL == address))
        return (NULL);

	/* Dummy stream, just set fields required for search */
    rvSrtpStreamSetSsrc (&tmpStream, ssrc);
    adrsPtr = rvSrtpStreamGetAddress (&tmpStream);
    memcpy (adrsPtr, address, sizeof (RvAddress));

    tmp = RvObjHashFind (&(thisPtr->destHash), &tmpStream);

    return ((RvSrtpStream *)tmp);
} /* rvSrtpDbDestFind */


/* Context functions */
RvSrtpContext *rvSrtpDbContextAdd (RvSrtpDb *thisPtr, RvSrtpStream *stream, RvUint64 startIndex, RvSrtpKey *key, RvBool trigger)
{
	RvSrtpContext *newContext;
	RvChar         tmpbuf[RV_64TOASCII_BUFSIZE];

    if ((NULL == thisPtr) || (NULL == stream))
        return (NULL);

	newContext = rvSrtpStreamAddContext(stream, startIndex, key, trigger);
	if(newContext == NULL) {
		rvSrtpLogWarning ((rvSrtpLogSourcePtr, "rvSrtpDbContextAdd: Requested context was not added"));
		return NULL;
	}
    RV_UNUSED_ARG(tmpbuf); /* warning elimination, when LOG_MASK is NONE */ 
    rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbContextAdd: context added, with initIndex - %s", Rv64UtoA(startIndex, tmpbuf)));
    return newContext;
} /* rvSrtpDbContextAdd */

RvBool rvSrtpDbContextRemove (RvSrtpDb *thisPtr, RvSrtpContext *context)
{
    if ((NULL == thisPtr) || (NULL == context))
        return (RV_FALSE);

	if(rvSrtpStreamRemoveContext(context) == RV_FALSE) {
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpDbContextRemove: Failed to remove context %x from stream",
						 (RvInt)context));
		return RV_FALSE;
	}

	rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpDbContextRemove: context removed"));
    return RV_TRUE;
} /* rvSrtpDbContextRemove */

RvSrtpContext *rvSrtpDbContextFind (RvSrtpDb *thisPtr, RvSrtpStream *stream, RvUint64 index)
{
    RvSrtpContext *context;

    if ((NULL == thisPtr) || (NULL == stream))
        return (NULL);

	context = rvSrtpStreamFindContext(stream, index);
	if((context != NULL) && (rvSrtpContextGetMasterKey(context) == NULL))
		context = NULL; /* the context is a "black-hole", hence invalid */

    return context;
} /* rvSrtpDbContextFind */

static void *rvSrtpKeyHashMalloc (RvSize_t size, void *data)
{
    void *res;
    RvStatus rc;

    rc = RvMemoryAlloc ((RvMemory *)data, size, NULL, &res);
    if (RV_OK != rc)
        return (NULL);
    return (res);
} /* rvSrtpKeyHashMalloc */

static void rvSrtpKeyHashFree (void *ptr, void *data)
{
    RvMemoryFree (ptr, NULL);
    RV_UNUSED_ARG(data);
} /* rvSrtpKeyHashFree */


static RvInt rvSrtpKeyHashItemCmp (void *ptr1, void *ptr2, void *data)
{
    RvUint8   *mki1;
    RvUint8   *mki2;
    RvInt res;
    RvSize_t  size;

    size = ((RvSrtpDb *)data)->mkiSize;

    mki1 = rvSrtpKeyGetMki ((RvSrtpKey *)ptr1);
    mki2 = rvSrtpKeyGetMki ((RvSrtpKey *)ptr2);

    res = memcmp (mki1, mki2, size);
    if (res < 0)
        return (-1);
    else if (res > 0)
        return (1);
    else
#ifdef UPDATED_BY_SPIRENT
       if(((RvSrtpKey *)ptr1)->direction != ((RvSrtpKey *)ptr2)->direction)
           return (1);
       else
#endif // UPDATED_BY_SPIRENT
        return (0);
} /* rvSrtpKeyHashItemCmp */

static RvSize_t rvSrtpKeyHashGetHash (void *item, void *data)
{
    RvSrtpKey *pEntry;
    RvUint8    *temp;
    RvSize_t   retHash;
    RvUint32   hashSize;

    pEntry = (RvSrtpKey *)item;

    retHash = 0;
    hashSize = ((RvSrtpDb *)data)->mkiSize;
    temp = (RvUint8 *)rvSrtpKeyGetMki (pEntry);

    while ((hashSize-- >0) && (temp != NULL))
    {
        retHash = (retHash << 1) ^ *temp;
        temp++;
    }
#ifdef UPDATED_BY_SPIRENT
    retHash = (retHash << 1) ^ pEntry->direction;
#endif // UPDATED_BY_SPIRENT

    return ((RvSize_t)retHash);
} /* rvSrtpKeyHashGetHash */

static void *rvSrtpSrcHashMalloc (RvSize_t size, void *data)
{
    void *res;
    RvStatus rc;

    rc = RvMemoryAlloc ((RvMemory *)data, size, NULL, &res);
    if (RV_OK != rc)
        return (NULL);
    return (res);
} /* rvSrtpSrcHashMalloc */

static void rvSrtpSrcHashFree (void *ptr, void *data)
{
    RvMemoryFree (ptr, NULL);
    RV_UNUSED_ARG(data);
} /* rvSrtpSrcHashFree */

static RvInt rvSrtpSrcHashItemCmp (void *ptr1, void *ptr2, void *data)
{
    RvUint32   ssrc1;
    RvUint32   ssrc2;

    RV_UNUSED_ARG(data);

    ssrc1 = rvSrtpStreamGetSsrc ((RvSrtpStream *)ptr1);
    ssrc2 = rvSrtpStreamGetSsrc ((RvSrtpStream *)ptr2);

    if (ssrc1 < ssrc2)
        return (-1);
    else if (ssrc1 > ssrc2)
        return (1);
    else
    {
        RvBool isRtp1;
        RvBool isRtp2;

        isRtp1 = rvSrtpStreamGetIsRtp ((RvSrtpStream *)ptr1);
        isRtp2 = rvSrtpStreamGetIsRtp ((RvSrtpStream *)ptr2);

        if (isRtp1 < isRtp2)
            return (1);
        else if (isRtp2 > isRtp2)
            return (-1);
        else
            return (0);
    }
} /* rvSrtpKeyHashItemCmp */

static RvSize_t rvSrtpSrcHashGetHash (void *item, void *data)
{
    RvSrtpStream *pEntry;
    RvUint8      *temp;
    RvSize_t      retHash;
    RvUint32      hashSize;
    RvUint32      ssrc;
    RvBool        isRTP;

    RV_UNUSED_ARG(data);

    pEntry = (RvSrtpStream *)item;

    retHash = 0;
    hashSize = sizeof (ssrc);
    ssrc = rvSrtpStreamGetSsrc (pEntry);
    temp = (RvUint8 *)&ssrc;

    while ((hashSize-- >0) && (temp != NULL))
    {
        retHash = (retHash << 1) ^ *temp;
        temp++;
    }

    isRTP = rvSrtpStreamGetIsRtp (pEntry);
    retHash = (retHash << 1) ^ (RvUint8)isRTP;

    return (retHash);
} /* rvSrtpSrcHashGetHash */

static void *rvSrtpDstHashMalloc (RvSize_t size, void *data)
{
    void *res;
    RvStatus rc;

    rc = RvMemoryAlloc ((RvMemory *)data, size, NULL, &res);
    if (RV_OK != rc)
        return (NULL);
    return (res);
} /* rvSrtpDstHashMalloc */

static void rvSrtpDstHashFree (void *ptr, void *data)
{
    RvMemoryFree (ptr, NULL);
    RV_UNUSED_ARG(data);
} /* rvSrtpDstHashFree */

static RvInt rvSrtpDstHashItemCmp (void *ptr1, void *ptr2, void *data)
{
    RvInt       addrType1, addrType2;
    RvAddress  *adrs1;
    RvAddress  *adrs2;

    RV_UNUSED_ARG(data);

    adrs1 = rvSrtpStreamGetAddress ((RvSrtpStream *)ptr1);
    adrs2 = rvSrtpStreamGetAddress ((RvSrtpStream *)ptr2);

    addrType1 = RvAddressGetType (adrs1);
    addrType2 = RvAddressGetType (adrs2);

    if (addrType1 < addrType2)
        return (-1);
    else if (addrType1 > addrType2)
        return (1);
    else
    {
        if (RV_FALSE == RvAddressCompare (adrs1, adrs2, RV_ADDRESS_BASEADDRESS))
            return (-1);
        else
        {
            RvUint16 port1, port2;

            port1 = RvAddressGetIpPort (adrs1);
            port2 = RvAddressGetIpPort (adrs2);

            if (port1 < port2)
                return (-1);
            else if (port1 > port2)
                return (1);
            else
            {
                RvUint32 ssrc1, ssrc2;

                ssrc1 = rvSrtpStreamGetSsrc ((RvSrtpStream *)ptr1);
                ssrc2 = rvSrtpStreamGetSsrc ((RvSrtpStream *)ptr2);

                if (ssrc1 < ssrc2)
                    return (-1);
                else if (ssrc1 > ssrc2)
                    return (1);
                else
                    return (0);
            }
        }
    }
} /* rvSrtpDstHashItemCmp */

static RvSize_t rvSrtpDstHashGetHash (void *item, void *data)
{
    RvSrtpStream *pEntry;
    RvAddress    *addr;
    RvUint8      *temp;
    RvSize_t      retHash;
    RvUint32      hashSize;
    RvInt         addrType;
    RvUint32      ssrc;
    RvUint16      port;

    RV_UNUSED_ARG(data);

    pEntry = (RvSrtpStream *)item;

    retHash = 0;
    addr = rvSrtpStreamGetAddress (pEntry);
    addrType = RvAddressGetType (addr);

    if ((RV_ADDRESS_TYPE_IPV4 != addrType) && (RV_ADDRESS_TYPE_IPV6 != addrType))
        return (0);

    if (RV_ADDRESS_TYPE_IPV4 == addrType)
    {
        RvChar buf[RV_ADDRESS_IPV4_STRINGSIZE];

        memset (buf, 0, RV_ADDRESS_IPV4_STRINGSIZE);
        RvAddressGetString (addr, RV_ADDRESS_IPV4_STRINGSIZE, buf);
        hashSize = RV_ADDRESS_IPV4_STRINGSIZE;
        temp = (RvUint8 *)buf;
        while ((hashSize-- >0) && (temp != NULL))
        {
            retHash = (retHash << 1) ^ *temp;
            temp++;
        }

    }
#if (RV_NET_TYPE & RV_NET_IPV6)
    else
    {
        RvChar buf[RV_ADDRESS_IPV6_STRINGSIZE];

        memset (buf, 0, RV_ADDRESS_IPV6_STRINGSIZE);
        RvAddressGetString (addr, RV_ADDRESS_IPV6_STRINGSIZE, buf);
        hashSize = RV_ADDRESS_IPV6_STRINGSIZE;
        temp = (RvUint8 *)buf;
        while ((hashSize-- >0) && (temp != NULL))
        {
            retHash = (retHash << 1) ^ *temp;
            temp++;
        }
    }
#endif

    port = RvAddressGetIpPort (addr);
    hashSize = sizeof(port);
    temp = (RvUint8 *)&port;
    while ((hashSize-- >0) && (temp != NULL))
    {
        retHash = (retHash << 1) ^ *temp;
        temp++;
    }

    ssrc = rvSrtpStreamGetSsrc (pEntry);
    hashSize = sizeof(ssrc);
    temp = (RvUint8 *)&ssrc;
    while ((hashSize-- >0) && (temp != NULL))
    {
        retHash = (retHash << 1) ^ *temp;
        temp++;
    }

    return (retHash);
} /* rvSrtpKeyHashGetHash */


#if defined (RV_TEST_CODE)
#include "rvstdio.h"

static void rvSrtpStreamSListPrint (RvSrtpStream *stream);

RvRtpStatus RvSrtpDbTest ()
{
    RvSrtpDb           db;
    RvSrtpDbPoolConfig cfg;
    RvAddress      destAddr;
    RvSrtpStream  *src;
    RvSrtpStream  *dst;
    RvSrtpContext *context;
    RvSrtpContext *context1;
    RvSrtpKey     *key;
    RvSrtpKey     *key2;

    cfg.keyRegion = NULL;
    cfg.keyPageItems = 10;
    cfg.keyPageSize  = 2000;
    cfg.keyPoolType  = RV_OBJPOOL_TYPE_DYNAMIC;
    cfg.keyMaxItems  = 0;
    cfg.keyMinItems  = 10;
    cfg.keyFreeLevel = 2;

    cfg.streamRegion = NULL;
    cfg.streamPageItems = 10;
    cfg.streamPageSize  = 3000;
    cfg.streamPoolType  = RV_OBJPOOL_TYPE_EXPANDING;
    cfg.streamMaxItems  = 400;
    cfg.streamMinItems  = 10;
    cfg.streamFreeLevel = 0;

    cfg.contextRegion = NULL;
    cfg.contextPageItems = 10;
    cfg.contextPageSize  = 3000;
    cfg.contextPoolType  = RV_OBJPOOL_TYPE_EXPANDING;
    cfg.contextMaxItems  = 0;
    cfg.contextMinItems  = 10;
    cfg.contextFreeLevel = 0;

    cfg.keyHashRegion = NULL;
    cfg.keyHashStartSize = 10;
    cfg.keyHashType = RV_OBJHASH_TYPE_DYNAMIC;

    cfg.sourceHashRegion = NULL;
    cfg.sourceHashStartSize = 12;
    cfg.sourceHashType = RV_OBJHASH_TYPE_DYNAMIC;

    cfg.destHashRegion = NULL;
    cfg.destHashStartSize = 5;
    cfg.destHashType = RV_OBJHASH_TYPE_EXPANDING;

    if (NULL == rvSrtpDbConstruct (&db, 10, 2, 3, 10, 12, 14, 12, 14, 8, 6, 5, &cfg))
        return (RV_RTPSTATUS_Failed);

    key = rvSrtpDbKeyAdd (&db, (RvUint8 *)"11111\0", (RvUint8 *)"2222222\0", (RvUint8 *)"3333333333\0");
    if (NULL == key)
        rvSrtpDbDestruct (&db);

    key2 = rvSrtpDbKeyAdd (&db, (RvUint8 *)"2323232\0", (RvUint8 *)"11123\0", (RvUint8 *)"3434\0");
    if (NULL == key2)
        rvSrtpDbDestruct (&db);

    if (NULL == RvAddressConstruct (RV_ADDRESS_TYPE_IPV4, &destAddr))
        RvPrintf ("Fail constructing address\n");
    RvAddressSetIpPort (&destAddr, 1000);
    RvAddressSetString ("11.22.33.44", &destAddr);
    dst = rvSrtpDbDestAdd (&db, 0x98765432, RV_TRUE, 0x1, &destAddr);
    src = rvSrtpDbSourceAdd (&db, 0x1357675, RV_FALSE, 4);

    /* check insert after */
    context = rvSrtpDbContextAdd (&db, src, 1000, key, RV_TRUE, RV_TRUE);
    rvSrtpStreamSListPrint (src);
    context = rvSrtpDbContextFind (&db, src, 1200);
    context = rvSrtpDbContextAdd (&db, src, 2000, key, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);

    /* check replacing */
    context = rvSrtpDbContextAdd (&db, src, 3000, NULL, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);
    context = rvSrtpDbContextAdd (&db, src, 4000, key, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);
    context = rvSrtpDbContextAdd (&db, src, 3000, key, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);

    /* check insert before */
    context = rvSrtpDbContextAdd (&db, src, 500, key, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);

    context = rvSrtpDbContextAdd (&db, src, 4000, key2, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);
    context = rvSrtpDbContextAdd (&db, src, 4000, NULL, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);
    context = rvSrtpDbContextAdd (&db, src, 2000, NULL, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);
    context = rvSrtpDbContextAdd (&db, src, 2300, NULL, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);
    context = rvSrtpDbContextAdd (&db, src, 2100, NULL, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);
    context = rvSrtpDbContextAdd (&db, src, 1900, NULL, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);

    /* test removal */
    context = rvSrtpDbContextAdd (&db, src, 2000, key, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);
    context = rvSrtpDbContextAdd (&db, src, 3000, NULL, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);
    context1 = rvSrtpDbContextAdd (&db, src, 4000, key, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);
    context = rvSrtpDbContextAdd (&db, src, 5000, NULL, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);
    if (RV_FALSE == rvSrtpDbContextRemove (&db, context1))
        RvPrintf ("Failed to remove context 0x%x\n", (RvInt)context1);

    /* check history removal */
    context = rvSrtpDbContextAdd (&db, src, 10, key, RV_FALSE, RV_TRUE);
    context = rvSrtpDbContextAdd (&db, src, 20, key2, RV_FALSE, RV_TRUE);
    context = rvSrtpDbContextAdd (&db, src, 30, key, RV_FALSE, RV_TRUE);
    context = rvSrtpDbContextAdd (&db, src, 40, key2, RV_FALSE, RV_TRUE);
    context = rvSrtpDbContextAdd (&db, src, 50, key, RV_FALSE, RV_TRUE);
    context = rvSrtpDbContextAdd (&db, src, 60, key2, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);
    if (RV_FALSE == rvSrtpDbContextHistoryClean (&db, src, 45))
        RvPrintf ("Source history failed\n");
    rvSrtpStreamSListPrint (src);

    context = rvSrtpDbContextAdd (&db, dst, 1000, key, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (dst);
    context = rvSrtpDbContextAdd (&db, dst, 2000, key, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (dst);
    context = rvSrtpDbContextAdd (&db, dst, 3000, key, RV_TRUE, RV_TRUE);
    rvSrtpStreamSListPrint (dst);
    context = rvSrtpDbContextAdd (&db, dst, 1, key, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (dst);

    /* check history removal */
    context = rvSrtpDbContextAdd (&db, dst, 10, key2, RV_FALSE, RV_TRUE);
    context = rvSrtpDbContextAdd (&db, dst, 20, key, RV_FALSE, RV_TRUE);
    context = rvSrtpDbContextAdd (&db, dst, 30, key2, RV_FALSE, RV_TRUE);
    context = rvSrtpDbContextAdd (&db, dst, 40, key, RV_FALSE, RV_TRUE);
    context = rvSrtpDbContextAdd (&db, dst, 50, key2, RV_FALSE, RV_TRUE);
    context = rvSrtpDbContextAdd (&db, dst, 60, key, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (dst);
    if (RV_FALSE == rvSrtpDbContextHistoryClean (&db, dst, 100))
        RvPrintf ("Dest history failed\n");
    rvSrtpStreamSListPrint (dst);

    context = rvSrtpDbContextAdd (&db, dst, 10, key2, RV_FALSE, RV_TRUE);
    context = rvSrtpDbContextAdd (&db, dst, 20, key, RV_FALSE, RV_TRUE);
    context = rvSrtpDbContextAdd (&db, dst, 30, key2, RV_FALSE, RV_TRUE);
    context = rvSrtpDbContextAdd (&db, dst, 40, key, RV_FALSE, RV_TRUE);
    context = rvSrtpDbContextAdd (&db, dst, 50, key2, RV_FALSE, RV_TRUE);
    context = rvSrtpDbContextAdd (&db, dst, 60, key, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (dst);
    if (RV_FALSE == rvSrtpDbContextHistoryClean (&db, dst, 40))
        RvPrintf ("Dest history failed\n");
    rvSrtpStreamSListPrint (dst);

    if (key != rvSrtpDbKeyFind (&db, (RvUint8 *)"11111\0"))
        RvPrintf ("Bad key found\n");

    context = rvSrtpDbContextFind (&db, src, 3123);
    printf ("context init index is %d, searched index is 3123\n", rvSrtpContextGetFromIndex (context));

    context = rvSrtpDbContextFind (&db, src, 1001);
    printf ("context init index is %d, searched index is 1001\n", rvSrtpContextGetFromIndex (context));

    context = rvSrtpDbContextFind (&db, dst, 1200);
    printf ("context init index is %d, searched index is 1200\n", rvSrtpContextGetFromIndex (context));

    /* check context finding after removing its stream */
    rvSrtpDbDestRemove (&db, dst);
    context = rvSrtpDbContextFind (&db, dst, 1200);

    /* check non-replace operation */
    context = rvSrtpDbContextAdd (&db, src, 3000, NULL, RV_FALSE, RV_FALSE);
    rvSrtpStreamSListPrint (src);

    /* test removal key */
    rvSrtpDbKeyRemove (&db, key2);

    {
        RvSrtpStream *tmp1;
        RvSrtpStream *tmp2;

        tmp1 = rvSrtpDbSourceAdd (&db, 0x135543, RV_TRUE, 231);
        tmp2 = rvSrtpDbSourceAdd (&db, 0x135543, RV_TRUE, 221);

    }
    rvSrtpDbDestruct (&db);

    if (NULL == rvSrtpDbConstruct (&db, 10, 2, 3, 10, 12, 14, 12, 14, 8, 6, 5, &cfg))
        return (RV_RTPSTATUS_Failed);

    key = rvSrtpDbKeyAdd (&db, (RvUint8 *)"113\0", (RvUint8 *)"2223\0", (RvUint8 *)"454545\0");
    if (NULL == key)
        rvSrtpDbDestruct (&db);
    src = rvSrtpDbSourceAdd (&db, 0x123, RV_FALSE, 4);

    context = rvSrtpDbContextAdd (&db, src, 1000, key, RV_TRUE, RV_TRUE);
    rvSrtpStreamSListPrint (src);
    context = rvSrtpDbContextAdd (&db, src, 2000, key, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);

    /* check replacing */
    context = rvSrtpDbContextAdd (&db, src, 3000, NULL, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);
    context = rvSrtpDbContextAdd (&db, src, 4000, key, RV_FALSE, RV_TRUE);
    rvSrtpStreamSListPrint (src);

    context1 = rvSrtpDbContextGetNextContextFromKey (&db, key, NULL);
    context1 = rvSrtpDbContextGetNextContextFromKey (&db, key, context1);

    context = rvSrtpDbContextGetPrevContextFromStream (&db, context);

    rvSrtpDbDestruct (&db);

    /* test multi constructions */
    {
#define DBSIZEDB 100
        RvSrtpDb *dbArr[DBSIZEDB];
        RvSrtpDb  db[DBSIZEDB];
        RvInt iii, kkk;
        RvBool suc = RV_TRUE;

        kkk = DBSIZEDB;
        for (iii = 0; iii < DBSIZEDB; iii++)
        {
            dbArr[iii] = rvSrtpDbConstruct (&db[iii], 20,20,20,20,20,20,20,20,20, 72, 96, &cfg);
            if (NULL == dbArr[iii])
            {
                kkk = iii;
                break;
            }
        }

        if (DBSIZEDB == iii)
            RvPrintf ("All DBs were allocated\n");

        for (iii = 0; iii < kkk; iii++)
            if (RV_FALSE == rvSrtpDbDestruct (dbArr[iii]))
            {
                RvPrintf ("Failed destructing DB %d\n", iii);
                suc = RV_FALSE;
            }

        if (RV_TRUE == suc)
            RvPrintf ("All DBs were destructed\n");
    }
    return (RV_RTPSTATUS_Succeeded);
} /* RvSrtpDbTest */

static void rvSrtpStreamSListPrint (RvSrtpStream *stream)
{
    RvObjSList    *slist;
    RvSrtpContext *context;
    RvInt cnt;

    if (NULL == stream)
        return;

    slist = rvSrtpStreamGetContextSList (stream);
    if (NULL == slist)
    {
        RvPrintf ("Stream 0x%x has no SList\n", (RvInt)stream);
        return;
    }

    context = NULL;
    RvPrintf ("\n");
    RvPrintf ("Stream = 0x%x\n", (RvInt)stream);
    cnt = 1;
    while (NULL != (context = RvObjSListGetNext (slist, context, RV_OBJLIST_LEAVE)))
    {
        RvPrintf ("Context %d = 0x%x\n", cnt, (RvInt)context);
        RvPrintf ("From Index = %d\n", rvSrtpContextGetFromIndex (context));
        RvPrintf ("MasterKey  = 0x%x\n", rvSrtpContextGetMasterKey (context));
        RvPrintf ("\n");
        cnt++;
    }
} /* rvSrtpStreamSListPrint */
#endif /* RV_TEST_CODE */
