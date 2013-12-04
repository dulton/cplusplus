/***********************************************************************
Filename   : rvsrtpauthentication.c
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
#include "rverror.h"
#include "rvsrtpauthentication.h"

extern RvRtpHmacSha1CB srtpSha1Func;
/*************************************************************************
 * rvSrtpAuthenticationCalcRtp
 *
 * this function receives the encrypted packet, the input for this 
 * function is the output of the encryption function and actually is the 
 * SRTP output packet.
 * the authenticationTag pointer points to the place where the authentication 
 * tag will be on the same output packet
 * 
 *************************************************************************/
RvRtpStatus rvSrtpAuthenticationCalcRtp
(
    RvInt    authType, 
    RvUint8  *buf, 
    RvSize_t bufSize, 
    RvUint8  *key, 
    RvSize_t keySize, 
    RvUint8  *tag, 
    RvSize_t tagSize, 
    RvUint32 roc
)
{
    RvRtpStatus rc = RV_OK;
    RvUint8 rocStr[4];
    rocStr[0]= (RvUint8)((roc&0xFF000000)>>24);
    rocStr[1]= (RvUint8)((roc&0x00FF0000)>>16);
    rocStr[2]= (RvUint8)((roc&0x0000FF00)>>8);
    rocStr[3]= (RvUint8)((roc&0x000000FF));
    if ((NULL == buf) || (NULL == key) || (NULL == tag)) 
    {
        return RV_RTPSTATUS_NullPointer;
    }

    switch(authType)
    {
    case RV_SRTP_AUTHENTICATION_HMACSHA1:
        if (srtpSha1Func!=NULL)
        rc = srtpSha1Func(key, keySize, buf, bufSize, (RvUint8 *)rocStr, 4, tag, tagSize);
        if (RV_RTPSTATUS_Succeeded != rc)
        {
            return rc;
        }
        break;
    case RV_SRTP_AUTHENTICATION_NONE:
        return RV_RTPSTATUS_Succeeded;
        break;
    default:
            return RV_RTPSTATUS_BadParameter;
    }
    
    return RV_RTPSTATUS_Succeeded;
    
}

/*************************************************************************
 * rvSrtpAuthenticationCalcRtcp
 *
 * this function receives the encrypted packet, the input for this 
 * function is the output of the encryption function and actually is the 
 * SRTCP output packet.
 * the authenticationTag pointer points to the place where the authentication 
 * tag will be on the same output packet
 * 
 *************************************************************************/
RvRtpStatus rvSrtpAuthenticationCalcRtcp
(
    RvInt    authType, 
    RvUint8  *buf, 
    RvSize_t bufSize, 
    RvUint8  *key, 
    RvSize_t keySize, 
    RvUint8  *tag, 
    RvSize_t tagSize
)
{
    RvRtpStatus rc = RV_OK;
    
    if ((NULL == buf) || (NULL == key) || (NULL == tag)) 
    {
        return RV_RTPSTATUS_NullPointer;
    }

    switch(authType)
    {
    case RV_SRTP_AUTHENTICATION_HMACSHA1:
        /*for SRTCP you dont need buffer2 ,input the same buffer with length 0
          in order to make the RvSrtpHmacSha1 ignore it*/
        if (srtpSha1Func!=NULL)
            rc = srtpSha1Func(key, keySize, buf, bufSize, buf, 0, tag, tagSize);
        if (RV_RTPSTATUS_Succeeded != rc)
        {
            return rc;
        }
        break;        
    default:
            return RV_RTPSTATUS_BadParameter;
    }
    
    return RV_RTPSTATUS_Succeeded;
    
}

/*************************************************************************
* RvSrtpAuthenticationGetBlockSize
* ------------------------------------------------------------------------
* General: This function get the block size of the sha1 algorithm. 
*          Some algorithms require the input and output buffer to be a 
*          multiple of some blocksize. in sha1 case the answer will be 1
*          i.e. buffer sizes must be a multiple of one byte.
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    None
*
* Output    None
*************************************************************************/
RvSize_t rvSrtpAuthenticationGetBlockSize
(
    RvInt authType
)
{
    RV_UNUSED_ARG(authType);
        return 1;
}
#if defined(RV_TEST_CODE)

RvUint8 authPortion[] = "The SRTCP index is a 31-bit counter for the SRTCP packet. \
                         The index is explicitly included in each packet, in contrast \
                         to the \"implicit\" index approach used for SRTP.  The SRTCP \
                         index MUST be set to zero before the first SRTCP packet is \
                         sent, and MUST be incremented by one, modulo 2^31, after \
                         each SRTCP packet is sent.  In particular, after a re-key, \
                         the SRTCP index MUST NOT be reset to zero again";

RvUint8 authKey[20] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
                       0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                       0x10, 0x11, 0x12, 0x13};
RvUint16 authKey_len = 160;
RvUint16 authTag_len = 80;

RvUint8 authTag[80];

RvRtpStatus RvSrtpAuthenticationTest()
{
    RvUint32 ROC = 0xa5a5a5a5;
    RvRtpStatus rc;
    
    rc = rvSrtpAuthenticationCalcRtp(RV_SRTP_AUTHENTICATION_HMACSHA1, authPortion, 
                                     sizeof(authPortion), authKey, authKey_len, 
                                     authTag, authTag_len, ROC);
    rc = rvSrtpAuthenticationCalcRtcp(RV_SRTP_AUTHENTICATION_HMACSHA1, authPortion, 
                                      sizeof(authPortion), authKey, authKey_len, 
                                      authTag, authTag_len);
    return RV_RTPSTATUS_Succeeded;
}

#endif /* RV_TEST_CODE */

