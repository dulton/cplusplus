/***********************************************************************
Filename   : rvsha1.c
Description: 
************************************************************************
        Copyright (c) 2003 RADVISION Inc. and RADVISION Ltd.
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


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#if !defined(NO_SSL) /* for embedded OS to remove warnings from compilations */
#include <openssl/sha.h>
#endif

#include "rtptSha1.h"
#include "rverror.h"

/*-----------------------------------------------------------------------*/
/*                   Definitions & Constants                             */
/*-----------------------------------------------------------------------*/
#define SHA_DIGESTSIZE  SHA_DIGEST_LENGTH
#define SHA_BLOCKSIZE   SHA_CBLOCK

/***********************/
/* SHA-1 related stuff */
/***********************/
ShaStatus truncate
(
 RvChar*   inputBuf,  /* data to be truncated */
 RvChar*   outputBuf, /* truncated data */
 RvUint32  len        /* length in bytes to keep */
 )
{
    RvUint32 cnt;
    
    if ((NULL == inputBuf) || (NULL == outputBuf)) 
    {
        return shaNull;
    }
    
    for (cnt = 0 ; cnt < len ; cnt++)
    {
        outputBuf[cnt] = inputBuf[cnt];
    }
    return shaSuccess;
}
/* Constants defined in SHA-1 */
/*************************************************************************
* RvHmacSha1
* ------------------------------------------------------------------------
* General:
*   According to the standard, the message must be padded to an even
*   512 bits. The first padding bit must be a ’1’. The last 64
*   bits represent the length of the original message. All bits in
*   between should be 0. This function will pad the message
*   according to those rules by filling the messageBlock array
*   accordingly. It will also call the ProcessMessageBlock function
*   provided appropriately. When it returns, it can be assumed that
*   the message digest has been computed.
*
* Return Value: void
* ------------------------------------------------------------------------
* Arguments:
* Input:    *context - The SHA context to update
*
*************************************************************************/
ShaStatus RvHmacSha1
(
    RvUint8*  secretKey,     /* secret key */
    RvUint32  keyLength,     /* length of the key in bytes */
    RvUint8*  inputBuf1,     /* data */
    RvUint32  inputBuf1Len,  /* length of data in bytes */
    RvUint8*  inputBuf2,     /* data */
    RvUint32  inputBuf2Len,  /* length of data in bytes */
    RvUint8*  outputBuf,     /* output buffer, at least "outputBufLen" bytes */
    RvUint32  outputBufLen   /* in bytes*/
)
{
#if defined(NO_SSL)
    RV_UNUSED_ARG(secretKey);
    RV_UNUSED_ARG(keyLength);
    RV_UNUSED_ARG(inputBuf1);
    RV_UNUSED_ARG(inputBuf1Len);
    RV_UNUSED_ARG(inputBuf2);
    RV_UNUSED_ARG(inputBuf2Len);   
    RV_UNUSED_ARG(outputBuf);
    RV_UNUSED_ARG(outputBufLen);  
    if (outputBufLen <= inputBuf1Len)
    {
       memcpy(outputBuf, inputBuf1, outputBufLen);
    }
    else if (outputBufLen <= (inputBuf1Len+inputBuf2Len))
    {
       memcpy(outputBuf, inputBuf1, inputBuf1Len);
       memcpy(outputBuf + inputBuf1Len, inputBuf2, outputBufLen - inputBuf1Len);
    }
    else
    {
       memcpy(outputBuf, inputBuf1, inputBuf1Len);
       memcpy(outputBuf + inputBuf1Len, inputBuf2, inputBuf2Len);
       memset(outputBuf + (inputBuf1Len+ inputBuf2Len), 0, outputBufLen - (inputBuf1Len+ inputBuf2Len));
    }
#else /* NO_SSL */
    ShaStatus   rc;
    SHA_CTX innerContext;
    SHA_CTX outerContext;
    RvUint8     isha[SHA_DIGESTSIZE];
    RvUint8     osha[SHA_DIGESTSIZE];
    RvUint8     key[SHA_DIGESTSIZE];
    RvUint8     buf[SHA_BLOCKSIZE];
    RvUint32    cnt;
    
    if (keyLength > SHA_BLOCKSIZE)
    {
        SHA_CTX tempContext;
        
        rc = SHA1_Init(&tempContext);

        rc = SHA1_Update(&tempContext, secretKey, keyLength) ;

        rc = SHA1_Final(key, &tempContext);

        secretKey = key ;
        keyLength = SHA_DIGESTSIZE;
    }
    
    /**** Inner Digest ****/
    
    rc = SHA1_Init(&innerContext);
   
    /* Pad the key for inner digest */
    for (cnt = 0 ; cnt < keyLength ; ++cnt) buf[cnt] = (RvUint8)(secretKey[cnt] ^ 0x36);
    for (cnt = keyLength ; cnt < SHA_BLOCKSIZE ; ++cnt) buf[cnt] = 0x36;
    
    rc = SHA1_Update(&innerContext, buf, SHA_BLOCKSIZE);

    rc = SHA1_Update(&innerContext, inputBuf1, inputBuf1Len);
  
    rc = SHA1_Update(&innerContext, inputBuf2, inputBuf2Len);
  
    rc = SHA1_Final(isha, &innerContext);
  
    /**** Outter Digest ****/
    
    rc = SHA1_Init(&outerContext);
  
    /* Pad the key for outter digest */
    for (cnt = 0 ; cnt < keyLength ; ++cnt) buf[cnt] = (RvUint8) (secretKey[cnt] ^ 0x5C);
    for (cnt = keyLength ; cnt < SHA_BLOCKSIZE ; ++cnt) buf[cnt] = 0x5C;
    
    rc = SHA1_Update(&outerContext, buf, SHA_BLOCKSIZE);

    rc = SHA1_Update(&outerContext, isha, SHA_DIGESTSIZE);
  
    rc = SHA1_Final(osha, &outerContext);

    
    /* truncate and prRvUint32 the results */
    outputBufLen = ((outputBufLen > SHA_DIGESTSIZE) ? SHA_DIGESTSIZE : outputBufLen);
    rc = truncate((RvChar *)osha, (RvChar *)outputBuf, outputBufLen);
    
    if (shaSuccess != rc)
    {
        return rc;
    }
#endif     /* NO_SSL */
    
    return shaSuccess;
}
