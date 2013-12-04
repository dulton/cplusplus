/************************************************************************
 File Name	   : rvsrtpsha1.c
 Description   :
*************************************************************************
 Copyright (c)	2005 , RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVision Inc. 
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVision Inc. 
 
 RADVision Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
*************************************************************************/
#include "rvsrtpsha1.h"

#ifdef __cplusplus
extern "C" {
#endif

RvRtpHmacSha1CB srtpSha1Func = NULL;
/***********************************************************************************
 * RvRtpSetSha1EncryptionCallback
 * description: 
 *   This function sets user's implementation of RvRtpHmacSha1CB function
 * Parameters:
 *   sha1Callback - user's implemented HMAC SHA1 callback
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvRtpSetSha1EncryptionCallback(IN RvRtpHmacSha1CB sha1Callback)
{
    if (NULL == sha1Callback)
        return RV_ERROR_NULLPTR;
    srtpSha1Func = sha1Callback;
    return RV_OK;
}

#ifdef __cplusplus
}
#endif
