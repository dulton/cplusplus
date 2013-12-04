/***********************************************************************
Filename   : rtptSha1.h
Description: sha1 functions header file based on open ssl library
************************************************************************
      Copyright (c) 2001,2002 RADVISION Inc. and RADVISION Ltd.
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

#ifndef RTPT_SHA1_H
#define RTPT_SHA1_H

#if defined(__cplusplus)
extern "C" {
#endif 
  
#include "rvtypes.h"
    
/**************************************************************************************************
 * type enumeration: ShaStatus
 * description:
 *           This type represents sha1 returned status codes.
 * enumeration values:
 *            shaSuccess      -  on success.
 *            shaNull         -  Null pointer parameter.
 *            shaInputTooLong -  input data too long.
 *            shaStateError   -  called Input after Result.
 *************************************************************************************************/

#ifndef __SHA_STATUS_TYPE__   
#define __SHA_STATUS_TYPE__  
typedef enum 
{   shaSuccess = 0x00,
    shaNull,            
    shaInputTooLong,    
    shaStateError       
} ShaStatus;
#endif    /* __SHA_STATUS_TYPE__ */

ShaStatus RvHmacSha1(RvUint8* secretKey, RvUint32 keyLength, RvUint8* inputBuf1,     
                     RvUint32 inputBuf1Len, RvUint8* inputBuf2, RvUint32 inputBuf2Len,  
                     RvUint8* outputBuf, RvUint32 outputBufLen);


/**************************************************************************************************
 * RvHmacSha1()
 * Descryption:
 *    This method creates a Hashed Message Authentication Code (HMAC) using the
 *    SHA-1 algorithm.
 *    See the IETF Informational RFC 2104 "HMAC: Keyed-Hashing for message
 *     authentication" for information on HMAC.
 *    this function can accept two input buffers and internally concatenate them
 *    to one input.
 *    Params:
 *           secretKey    - secret key.
 *           keyLength    - secret key length.
 *           inputBuf1    - first input buffer.
 *           inputBuf1Len - first input buffer length.
 *           inputBuf2    - second input buffer.
 *           inputBuf2Len - second input buffer length.
 *           outputBuf    - output buffer. 
 *           outputBufLen} {d:output buffer length.
 *  Returns: ShaStatus
 **************************************************************************************************/

#if defined(__cplusplus)
}
#endif

#endif /* RTPT_SHA1_H */


