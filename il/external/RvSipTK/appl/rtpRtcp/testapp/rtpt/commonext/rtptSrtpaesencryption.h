/************************************************************************
 File Name	   : rvsrtpaesencryption.h
 Description   : RvSrtpAesPlugIn implementation sample
*************************************************************************
 Copyright (c)	2004 , RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVision Inc.
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVision Inc.

 RADVision Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
*************************************************************************/
#if !defined(RV_SRTP_AES_ENCRYPTION_H)
#define RV_SRTP_AES_ENCRYPTION_H

#include "rtptAesencryption.h"

#if defined(SRTP_ADD_ON)
#include "rvsrtpaesplugin.h"
#endif 

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SRTP_ADD_ON)

typedef struct RvSrtpAesEncryption_
{
    RvAesEncryption aesEncryption[RvSrtpEncryptionPurposes];
    RvSrtpAesPlugIn  plugin;

} RvSrtpAesEncryption;

/* Construct/Destruct Methods */
RVAPI
RvSrtpAesEncryption* RVCALLCONV RvSrtpAesEncryptionConstruct(RvSrtpAesEncryption *thisPtr);

RVAPI
void RVCALLCONV	RvSrtpAesEncryptionDestruct(RvSrtpAesEncryption *thisPtr);

/* Accessors */
#define RvSrtpAesEncryptionGetPlugIn(t) &(t)->plugin

#else /* #if defined(SRTP_ADD_ON)  */

#define RvSrtpAesEncryptionConstruct(thisPtr)
#define RvSrtpAesEncryptionDestruct(thisPtr)
#define RvSrtpAesEncryptionGetPlugIn            NULL

#endif /* #if defined(SRTP_ADD_ON) */

#ifdef __cplusplus
}
#endif

#endif /* RV_SRTP_AES_ENCRYPTION_H */

