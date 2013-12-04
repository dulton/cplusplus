/************************************************************************
 File Name	   : rvsrtpsha1.h
 Description   : Hashed Message Authentication Code (HMAC) using the
                 SHA-1 algorithm callback type definition.
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
#if !defined(RV_SRTP_SHA1_H)
#define RV_SRTP_SHA1_H

#include "rvtypes.h"
#include "rverror.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __SHA_STATUS_TYPE__   
#define __SHA_STATUS_TYPE__    
/***********************************************************************************
 * ShaStatus
 * description: this type represents sha1 returned status codes.
 * enumeration:
 * shaSuccess  - on success.
 * shaNull     - Null pointer parameter.
 * shaInputTooLong  - input data too long.
 * shaStateError  - called Input after Result.
 ***********************************************************************************/

    
typedef enum 
{
    shaSuccess = 0x00,
    shaNull,            
    shaInputTooLong,    
    shaStateError,

} ShaStatus;
#endif /* __SHA_STATUS_TYPE__ */

/***********************************************************************************
 * type definition: RvRtpHmacSha1CB
 * description: 
 *   This type represents the callback function of
 *   method, that creates a Hashed Message Authentication Code (HMAC) using the
 *   SHA-1 algorithm.
 *   See the IETF Informational RFC 2104 "HMAC: Keyed-Hashing for message
 *   authentication" for information on HMAC.
 *   this function can accept two input buffers and internally concatenate them
 *   to one input.
 * Input parameters:
 *   secretKey    - secret key.
 *   keyLength    - secret key length.
 *   inputBuf1    - first input buffer.
 *   inputBuf1Len - first input buffer length.
 *   inputBuf2    - second input buffer.
 *   inputBuf2Len - second input buffer length.
 * Output parameters:
 *   outputBuf    - output buffer.
 *   outputBufLen - output buffer length.
 * Returns : ShaStatus
 ***********************************************************************************/
typedef ShaStatus (*RvRtpHmacSha1CB)(
           RvUint8* secretKey, 
           RvUint32 keyLength, 
           RvUint8* inputBuf1,     
           RvUint32 inputBuf1Len, 
           RvUint8* inputBuf2, 
           RvUint32 inputBuf2Len,  
           RvUint8* outputBuf, 
           RvUint32 outputBufLen);

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
RvStatus RVCALLCONV RvRtpSetSha1EncryptionCallback(IN RvRtpHmacSha1CB sha1Callback);

#ifdef __cplusplus
}
#endif

#endif
