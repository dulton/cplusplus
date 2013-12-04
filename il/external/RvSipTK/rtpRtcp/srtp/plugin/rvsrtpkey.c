/************************************************************************
 File Name	   : rvsrtpkey.c
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
#include "rvsrtpkey.h"
#include "rvsrtpcontext.h"

static void *rvSrtpKeyDbObjConstruct (void *objptr, void *data);
static void rvSrtpKeyDbObjDestruct (void *objptr, void *data);
static void *rvSrtpKeyDbAlloc (RvSize_t size, void *data);
static void rvSrtpKeyDbFree (void *ptr, void *data);

/* Use only for stand-alone key objects (not pooled) */
RvSrtpKey *rvSrtpKeyConstruct(RvSrtpKey *thisPtr, RvSize_t masterKeySize, RvSize_t masterSaltSize, RvSize_t mkiSize)
{
    RvSrtpContext tmpContext;
    RvObjList    *tmpList;

    if (NULL == thisPtr)
        return (NULL);

    thisPtr->keyBufSize = masterKeySize + masterSaltSize + mkiSize;
	thisPtr->masterKey = &(thisPtr->keys[0]);
	thisPtr->masterSalt = &(thisPtr->keys[masterKeySize]);
	thisPtr->mki = &(thisPtr->keys[masterKeySize + masterSaltSize]);
	thisPtr->maxLimit = RV_FALSE;
	thisPtr->keyDerivationRate = 0;

    /* construct the context list */
    tmpList = RvObjListConstruct (&tmpContext, rvSrtpContextGetListElem (&tmpContext), &(thisPtr->contextList));
    if (NULL == tmpList)
    {
        return (NULL);
    }

    return (thisPtr);
} /* *rvSrtpKeyConstruct */

void rvSrtpKeyDestruct (RvSrtpKey *thisPtr)
{
    /* destruct the list */
	RvObjListDestruct (&(thisPtr->contextList));
    RV_UNUSED_ARG(thisPtr);
} /* rvSrtpKeyDestruct */

RvSize_t rvSrtpKeyCalcSize (RvSize_t masterKeySize, RvSize_t masterSaltSize, RvSize_t mkiSize)
{
    RvSrtpKey tmp;
    RvSize_t  size = (((RvUint8 *)&tmp.keys[0] - (RvUint8 *)&tmp) + masterKeySize + masterSaltSize + mkiSize);
    
    return size;
} /* rvSrtpKeyCalcSize */

/* Standard key functions */
RvSize_t rvSrtpKeyGetSize (RvSrtpKey *thisPtr)
{
    RvSrtpKey tmp;
    RvSize_t  size;
    
    if (NULL == thisPtr)
        return 0;
    size = ((&tmp.keys[0] - (RvUint8 *)&tmp) + thisPtr->keyBufSize);
    
    return size;
} /* rvSrtpKeyGetSize */

/* Pool functions */
RvSrtpKeyPool *rvSrtpKeyPoolConstruct (RvSrtpKeyPool *pool, 
                                       RvSize_t masterKeySize, 
                                       RvSize_t masterSaltSize,
                                       RvSize_t mkiSize, 
                                       RvMemory *region, 
                                       RvSize_t pageitems, 
                                       RvSize_t pagesize,
                                       RvInt pooltype, 
                                       RvSize_t maxitems, 
                                       RvSize_t minitems, 
                                       RvSize_t freelevel)
{
    RvObjPoolFuncs keyCB;
    RvSrtpKey      tempKey;
    RvSize_t       itemSize;
    RvObjPool     *tmpPool;

    if (NULL == pool)
        return (NULL);

    pool->region = region;
    pool->masterKeySize = masterKeySize;
    pool->masterSaltSize = masterSaltSize;
    pool->mkiSize = mkiSize;

    keyCB.objconstruct = rvSrtpKeyDbObjConstruct;
    keyCB.objdestruct = rvSrtpKeyDbObjDestruct;
    keyCB.pagealloc = rvSrtpKeyDbAlloc;
    keyCB.pagefree = rvSrtpKeyDbFree;
    keyCB.objconstructdata = pool;
    keyCB.objdestructdata = NULL;
    keyCB.pageallocdata = pool;
    keyCB.pagefreedata = NULL;

    itemSize = rvSrtpKeyCalcSize (masterKeySize, masterSaltSize, mkiSize);
    tmpPool = RvObjPoolConstruct (&tempKey, &(tempKey.poolElem), &keyCB, itemSize, pageitems, pagesize, pooltype,
                                  ((RV_OBJPOOL_TYPE_DYNAMIC == pooltype) ? RV_TRUE : RV_FALSE), maxitems,
                                  minitems, freelevel, &(pool->pool));

    if (NULL == tmpPool)
    {
        return (NULL);
    }

    return (pool);
} /* *rvSrtpKeyPoolConstruct */

RvBool rvSrtpKeyPoolDestruct (RvSrtpKeyPool *pool)
{
    if (NULL == pool)
        return (RV_FALSE);

    return (RvObjPoolDestruct (&(pool->pool)));
} /* rvSrtpKeyPoolDestruct */

RvSrtpKey *rvSrtpKeyPoolNew (RvSrtpKeyPool *pool)
{
    RvSrtpKey    *item;

    if (NULL == pool)
        return (NULL);

    item = (RvSrtpKey *)RvObjPoolGetItem (&pool->pool);
	return item;
} /* rvSrtpKeyPoolNew */

void rvSrtpKeyPoolRelease (RvSrtpKeyPool *pool, RvSrtpKey *key)
{
	if((key != NULL) && (pool != NULL))
		RvObjPoolReleaseItem (&pool->pool, key);
} /* rvSrtpKeyPoolRelease */

static void *rvSrtpKeyDbObjConstruct (void *objptr, void *data)
{
    RvSrtpKey *entry;
    RvSrtpKeyPool *pool;

    pool  = (RvSrtpKeyPool *)data;
	entry = rvSrtpKeyConstruct((RvSrtpKey *)objptr, pool->masterKeySize, pool->masterSaltSize, pool->mkiSize);
	
    return objptr;
} /* rvSrtpKeyDbObjConstruct */

static void rvSrtpKeyDbObjDestruct (void *objptr, void *data)
{
    RvSrtpKey *pEntry;

    RV_UNUSED_ARG(data);
    
    pEntry = objptr;
} /* rvSrtpKeyDbObjDestruct */

static void *rvSrtpKeyDbAlloc (RvSize_t size, void *data)
{
    void *res;
    RvSrtpKeyPool *keyPool = (RvSrtpKeyPool *)data;   
    RvStatus rc;

    rc = RvMemoryAlloc (keyPool->region, size, NULL, &res);
    if (RV_OK != rc)
        return (NULL);

    return (res);
} /* rvSrtpKeyDbAlloc */

static void rvSrtpKeyDbFree (void *ptr, void *data)
{
    RV_UNUSED_ARG(data);
    RvMemoryFree (ptr, NULL);
} /* rvSrtpKeyDbFree */


#if defined(RV_TEST_CODE)
#include "rvstdio.h"
#include "rvrtpstatus.h"
#include "rvsrtpdb.h"

RvRtpStatus RvSrtpKeyTest ()
{
#define STAM (4000)
    RvSrtpKeyPool      pool;
    RvSrtpKeyPool     *tmpPool;
    RvSrtpDbPoolConfig cfg;
    RvSrtpKey     *keyArr[STAM];
    RvSrtpKey     *key;
    RvInt kkk, iii;

    cfg.keyRegion = NULL;
    cfg.keyPageItems = 3;
    cfg.keyPageSize  = 2000;
    cfg.keyPoolType  = RV_OBJPOOL_TYPE_FIXED;
    cfg.keyMaxItems  = 5;
    cfg.keyMinItems  = 1;
    cfg.keyFreeLevel = 2;

    tmpPool = rvSrtpKeyPoolConstruct (&pool, 50, 40, 60, cfg.keyRegion, cfg.keyPageItems,
                                      cfg.keyPageSize,cfg.keyPoolType, cfg.keyMaxItems,
                                      cfg.keyMinItems, cfg.keyFreeLevel);

    if (NULL == tmpPool)
    {
        RvPrintf ("Failed to construct key pool\n");
        return (RV_RTPSTATUS_Failed);
    }

    key = rvSrtpKeyPoolNew (&pool);
    if (NULL == key)
    {
        RvPrintf ("Failed allocating new key\n");
        return (RV_RTPSTATUS_Failed);
    }

    RvPrintf ("MaxLimit = %s\n", (RV_TRUE == rvSrtpKeyGetMaxLimit (key)) ? "True" : "False");
    memcpy (rvSrtpKeyGetMasterKey (key), "3333333\0", 7);
    memcpy (rvSrtpKeyGetMasterSalt (key), "1111111\0", 7);
    memcpy (rvSrtpKeyGetMki (key), "2323232\0", 10);
    RvPrintf ("MasterKey = %s\n", rvSrtpKeyGetMasterKey (key));
    RvPrintf ("SaltKey = %s\n", rvSrtpKeyGetMasterSalt (key));
    RvPrintf ("MKI = %s\n", rvSrtpKeyGetMki (key));

    rvSrtpKeyPoolRelease (&pool, key);

    /* check "re-initialization" by re-allocation */
    key = rvSrtpKeyPoolNew (&pool);
    if (NULL == key)
    {
        RvPrintf ("Failed to allocate new key\n");
        return (RV_RTPSTATUS_Failed);
    }
    rvSrtpKeyPoolRelease (&pool, key);
    
    /* check stand alone objects */
    key = (RvSrtpKey *)RvObjPoolGetItem (&(pool.pool));
    if (NULL == key)
    {
        RvPrintf ("Failed to allocate new key\n");
        return (RV_RTPSTATUS_Failed);
    }

    rvSrtpKeyConstruct (key, 10, 20, 10);
    rvSrtpKeyDestruct (key);
    RvObjPoolReleaseItem (&(pool.pool), key);

    kkk = 10;
    for (iii = 0; iii < 10; iii++)
    {
        keyArr[iii] = rvSrtpKeyPoolNew (&pool);
        if (NULL == keyArr[iii])
        {
            kkk = iii;
            RvPrintf ("Unable to add new key %d\n", iii);
            break;
        }
    }

    for (iii = 0; iii < kkk; iii++)
    {
        rvSrtpKeyPoolRelease (&pool, keyArr[iii]);
    }

    rvSrtpKeyPoolDestruct (&pool);

    cfg.keyRegion = NULL;
    cfg.keyPageItems = 3;
    cfg.keyPageSize  = 2000;
    cfg.keyPoolType  = RV_OBJPOOL_TYPE_EXPANDING;
    cfg.keyMaxItems  = 0;
    cfg.keyMinItems  = 1;
    cfg.keyFreeLevel = 2;

    tmpPool = rvSrtpKeyPoolConstruct (&pool, 10, 30, 70, cfg.keyRegion, cfg.keyPageItems,
                                      cfg.keyPageSize,cfg.keyPoolType, cfg.keyMaxItems,
                                      cfg.keyMinItems, cfg.keyFreeLevel);

    if (NULL == tmpPool)
    {
        RvPrintf ("Failed to construct key pool\n");
        return (RV_RTPSTATUS_Failed);
    }

    kkk = STAM;
    for (iii = 0; iii < STAM; iii++)
    {
        RvSrtpKey *tmpKey;

        tmpKey = rvSrtpKeyPoolNew (&pool);
        keyArr[iii] = tmpKey;
        if (NULL == keyArr[iii])
        {
            kkk = iii;
            RvPrintf ("Unable to add new key %d\n", iii);
            break;
        }
    }
    if (STAM == iii)
        RvPrintf ("Allocated all keys\n");
        

    for (iii = 0; iii < kkk; iii++)
    {
        rvSrtpKeyPoolRelease (&pool, keyArr[iii]);
    }

    rvSrtpKeyPoolDestruct (&pool);

    return (RV_RTPSTATUS_Succeeded);
} /* RvSrtpKeyTest */

#endif /* RV_TEST_CODE */
