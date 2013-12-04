/************************************************************************
 File Name     : rvrtpaesencryption.c
 Description   :
*************************************************************************
 Copyright (c)  2005 , RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVision Inc.
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVision Inc.

 RADVision Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
*************************************************************************/

#include "rtptSrtpaesencryption.h"

#if defined(SRTP_ADD_ON)

#ifdef __cplusplus
extern "C" {
#endif
/**********************************************************************************
* Private method declarations
**********************************************************************************/
static void RvSrtpAesEncryptionDataConstructCallback(
        struct RvSrtpAesPlugIn_ *thisPtr,
        RvSrtpEncryptionPurpose purpose)
{
   RvSrtpAesEncryption* aesEncryptionPtr = thisPtr->userData;
   rvAesEncryptionConstruct(&aesEncryptionPtr->aesEncryption[purpose]);
}

static void RvSrtpAesEncryptionDataDestructCallback(
                                            struct RvSrtpAesPlugIn_ *thisPtr,
                                            RvSrtpEncryptionPurpose purpose)
{
    RvSrtpAesEncryption* aesEncryptionPtr = thisPtr->userData;
    rvAesEncryptionDestruct(&aesEncryptionPtr->aesEncryption[purpose]);
}

static RvBool RvSrtpAesEncryptionInitializeECBMode(
     struct RvSrtpAesPlugIn_ *thisPtr,
     RvSrtpEncryptionPurpose purpose,
     RvUint8 direction,
     const RvUint8* key,
     RvUint16 keyBitSize,
     RvUint16 blockBitSize)
{
    RvSrtpAesEncryption* aesEncryptionPtr = thisPtr->userData;
   return rvAesEncryptionInitializeECBMode(&aesEncryptionPtr->aesEncryption[purpose], direction, key, keyBitSize, blockBitSize);
}

static  RvBool RvSrtpAesEncryptionInitializeCBCMode(
     struct RvSrtpAesPlugIn_ *thisPtr,
     RvSrtpEncryptionPurpose purpose,
     RvUint8 direction,
     const RvUint8* key,
     RvUint16 keyBitSize,
     RvUint16 blockBitSize,
     const RvUint8* iv)
{
    RvSrtpAesEncryption* aesEncryptionPtr = thisPtr->userData;
    return rvAesEncryptionInitializeCBCMode(&aesEncryptionPtr->aesEncryption[purpose], direction, key, keyBitSize, blockBitSize, iv);
}

static void RvSrtpAesEncryptionEncryptCallback(
    struct RvSrtpAesPlugIn_ *thisPtr,
     RvSrtpEncryptionPurpose purpose,
    const RvUint8 *srcBuffer,
    RvUint8 *dstBuffer,
    RvUint32 byteLength)
{
    RvSrtpAesEncryption* aesEncryptionPtr = thisPtr->userData;
    rvAesEncryptionProcess(&aesEncryptionPtr->aesEncryption[purpose], srcBuffer, dstBuffer, byteLength);
}

static void RvSrtpAesEncryptionDecryptCallback(
    struct RvSrtpAesPlugIn_ *thisPtr,
    RvSrtpEncryptionPurpose purpose,
    const RvUint8 *srcBuffer,
    RvUint8 *dstBuffer,
    RvUint32 byteLength)
{
    RvSrtpAesEncryption* aesEncryptionPtr = thisPtr->userData;
    rvAesEncryptionProcess(&aesEncryptionPtr->aesEncryption[purpose], srcBuffer, dstBuffer, byteLength);
}


RVAPI
RvSrtpAesEncryption* RVCALLCONV RvSrtpAesEncryptionConstruct(RvSrtpAesEncryption *thisPtr)
{
    RvUint32 count = 0;

    for (count = 0; count < RvSrtpEncryptionPurposes; count++)
        rvAesEncryptionConstruct(&thisPtr->aesEncryption[count]);

    RvSrtpAesPlugInConstruct(
        &thisPtr->plugin,
        thisPtr,
        RvSrtpAesEncryptionDataConstructCallback,
        RvSrtpAesEncryptionDataDestructCallback,
        RvSrtpAesEncryptionInitializeECBMode,
        RvSrtpAesEncryptionInitializeCBCMode,
        RvSrtpAesEncryptionEncryptCallback,
        RvSrtpAesEncryptionDecryptCallback);

    return thisPtr;
}

RVAPI
void RVCALLCONV RvSrtpAesEncryptionDestruct(RvSrtpAesEncryption *thisPtr)
{
    RvUint32 count = 0;

    for (count = 0; count < RvSrtpEncryptionPurposes; count++)
        rvAesEncryptionDestruct(&thisPtr->aesEncryption[count]);

    RvSrtpAesPlugInDestruct(&thisPtr->plugin);
}

#ifdef __cplusplus
}
#endif

#endif /* #if defined(SRTP_ADD_ON) */
