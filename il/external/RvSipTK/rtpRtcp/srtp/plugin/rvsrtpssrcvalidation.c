/***********************************************************************
Filename   : rvsrtpssrcvalidation.c
Description:
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
#include "rvsrtpssrcvalidation.h"
//#include "rvconfigext.h"

RvSrtpSsrcValidation *rvSrtpSsrcValidationConstruct (RvSrtpSsrcValidation *thisPtr, RvSrtpDb *dbPtr,
                                                     RvBool rtpCheck, RvBool rtcpCheck)
{
    if ((NULL == thisPtr) || (NULL == dbPtr))
        return (NULL);

    thisPtr->dbPtr = dbPtr;
    thisPtr->rtpCheck = rtpCheck;
    thisPtr->rtcpCheck = rtcpCheck;

    return (thisPtr);
} /* rvSrtpSsrcValidationConstruct */

void rvSrtpSsrcValidationDestruct (RvSrtpSsrcValidation *thisPtr)
{
    RV_UNUSED_ARG(thisPtr);
    return;
} /* rvSrtpSsrcValidationDestruct  */

RvRtpStatus rvSrtpSsrcValidationCheck (RvSrtpSsrcValidation *thisPtr, RvUint32 ssrc, RvUint64 rtpIndex,
                                       RvUint64 rtcpIndex)
{
    RvSrtpDb *db;
    RvObjHash *srcHash;
    RvObjHash *dstHash;
    RvSrtpStream *stream;
    RvUint32      foundSsrc;

    RV_UNUSED_ARG(rtpIndex);
    RV_UNUSED_ARG(rtcpIndex);    
    
    if (NULL == thisPtr)
        return (RV_RTPSTATUS_Failed);

    db = thisPtr->dbPtr;
    if (NULL == db)
        return (RV_RTPSTATUS_Failed);

    /* go over the DB, down all existing streams, and check their ssrc. */
    srcHash = rvSrtpDbGetSrcHash (db);
    if (NULL != srcHash)
    {
        stream = NULL;
        while (NULL != (stream = RvObjHashGetNext (srcHash, stream)))
        {
            foundSsrc = rvSrtpDbStreamGetSsrc (stream);
            if (foundSsrc == ssrc)
                return (RV_RTPSTATUS_Failed);
        }
    }

    dstHash = rvSrtpDbGetDstHash (db);
    if (NULL != dstHash)
    {
        stream = NULL;
        while (NULL != (stream = RvObjHashGetNext (dstHash, stream)))
        {
            foundSsrc = rvSrtpDbStreamGetSsrc (stream);
            if (foundSsrc == ssrc)
                return (RV_RTPSTATUS_Failed);
        }
    }

    return (RV_RTPSTATUS_Succeeded);
} /* rvSrtpSsrcValidationCheck */

#if defined(RV_TEST_CODE)
#include "rvstdio.h"

RvRtpStatus RvSrtpSsrcValidationTest ()
{
    RvSrtpDb           db;
    RvSrtpDbPoolConfig cfg;
    RvAddress      destAddr;
    RvSrtpStream  *src;
    RvSrtpStream  *dst;
    RvSrtpKey     *key;
    RvSrtpKey     *key2;
    RvSrtpSsrcValidation srtpValid;
    RvUint32      ssrc;

    
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
    
    if (NULL == rvSrtpSsrcValidationConstruct (&srtpValid, &db, RV_TRUE, RV_TRUE))
        return (RV_RTPSTATUS_Failed);

    key = rvSrtpDbKeyAdd (&db, (RvUint8 *)"11111\0", (RvUint8 *)"2222222\0", (RvUint8 *)"3333333333\0", 0);
    if (NULL == key)
        rvSrtpDbDestruct (&db);
    
    key2 = rvSrtpDbKeyAdd (&db, (RvUint8 *)"2323232\0", (RvUint8 *)"11123\0", (RvUint8 *)"3434\0", 0);
    if (NULL == key2)
        rvSrtpDbDestruct (&db);
    
    if (NULL == RvAddressConstruct (RV_ADDRESS_TYPE_IPV4, &destAddr))
        RvPrintf ("Fail constructing address\n");
    RvAddressSetIpPort (&destAddr, 1000);
    RvAddressSetString ("11.22.33.44", &destAddr);
    dst = rvSrtpDbDestAdd (&db, 0x98765432, RV_TRUE, 0x1, &destAddr);
    src = rvSrtpDbSourceAdd (&db, 0x1357675, RV_FALSE, 4);

    rvSrtpDbDestAdd (&db, 0x112233, RV_TRUE, 0x5, &destAddr);
    rvSrtpDbDestAdd (&db, 0x114564, RV_TRUE, 0x500, &destAddr);
    rvSrtpDbDestAdd (&db, 0x12211565, RV_TRUE, 0x51, &destAddr);
    rvSrtpDbDestAdd (&db, 0x1211, RV_TRUE, 0x5343, &destAddr);
    rvSrtpDbDestAdd (&db, 0x99965332, RV_TRUE, 0x57, &destAddr);

    rvSrtpDbSourceAdd (&db, 0x7675, RV_FALSE, 4);
    rvSrtpDbSourceAdd (&db, 0x75, RV_FALSE, 40);
    rvSrtpDbSourceAdd (&db, 0x357675, RV_FALSE, 400);
    rvSrtpDbSourceAdd (&db, 0x15775, RV_FALSE, 444);
    rvSrtpDbSourceAdd (&db, 0x3575, RV_FALSE, 678);
    rvSrtpDbSourceAdd (&db, 0x1765, RV_FALSE, 2345);
    rvSrtpDbSourceAdd (&db, 0x13576757, RV_FALSE, 7654345);
    rvSrtpDbSourceAdd (&db, 0x2357675, RV_FALSE, 99876654);
    
    ssrc = 0x12345;
    if (RV_RTPSTATUS_Failed == rvSrtpSsrcValidationCheck (&srtpValid, ssrc, 0, 0))
        RvPrintf ("Failed validating SSRC 0x%x\n", ssrc);
    ssrc = 0x112233;
    if (RV_RTPSTATUS_Failed == rvSrtpSsrcValidationCheck (&srtpValid, ssrc, 0, 0))
        RvPrintf ("Failed validating SSRC 0x%x\n", ssrc);
    ssrc = 0x1212;
    if (RV_RTPSTATUS_Failed == rvSrtpSsrcValidationCheck (&srtpValid, ssrc, 0, 0))
        RvPrintf ("Failed validating SSRC 0x%x\n", ssrc);
    ssrc = 0x3575;
    if (RV_RTPSTATUS_Failed == rvSrtpSsrcValidationCheck (&srtpValid, ssrc, 0, 0))
        RvPrintf ("Failed validating SSRC 0x%x\n", ssrc);
    ssrc = 0x2357675;
    if (RV_RTPSTATUS_Failed == rvSrtpSsrcValidationCheck (&srtpValid, ssrc, 0, 0))
        RvPrintf ("Failed validating SSRC 0x%x\n", ssrc);
    ssrc = 0x1010101;
    if (RV_RTPSTATUS_Failed == rvSrtpSsrcValidationCheck (&srtpValid, ssrc, 0, 0))
        RvPrintf ("Failed validating SSRC 0x%x\n", ssrc);
    
    rvSrtpSsrcValidationDestruct (&srtpValid);
    rvSrtpDbDestruct (&db);
    return (RV_RTPSTATUS_Succeeded);
} /* RvSrtpSsrcValidationTest */
#endif /* RV_TEST_CODE */
