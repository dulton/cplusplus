/***********************************************************************
Filename   : rvsrtpauthentication.h
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
#ifndef RV_SRTP_AUTHENTICATION_H
#define RV_SRTP_AUTHENTICATION_H

#include "rvsrtpsha1.h"
#include "rvrtpstatus.h"

#if defined(__cplusplus)
extern "C" {
#endif 
    
/*$
{package scope="private":
	{name: RvSrtpAuthentication}
	{superpackage: RvSrtpEncryptionPlugin}
	{include: rvsrtpauthentication.h}
	{description:	
		{p: This module provides calculates an authentication tag.}
	}
}
$*/

/* Authentication types */
#define RV_SRTP_AUTHENTICATION_NONE RvIntConst(0)
#define RV_SRTP_AUTHENTICATION_HMACSHA1 RvIntConst(1)

RvRtpStatus rvSrtpAuthenticationCalcRtp(RvInt authType, RvUint8 *buf, RvSize_t bufSize, RvUint8 *key, RvSize_t keySize, RvUint8 *tag, RvSize_t tagSize, RvUint32 roc);
RvRtpStatus rvSrtpAuthenticationCalcRtcp(RvInt authType, RvUint8 *buf, RvSize_t bufSize, RvUint8 *key, RvSize_t keySize, RvUint8 *tag, RvSize_t tagSize);
RvSize_t rvSrtpAuthenticationGetBlockSize(RvInt authType);

#if defined(RV_TEST_CODE)
RvRtpStatus RvSrtpAuthenticationTest(void);
#endif /* RV_TEST_CODE */

/* Function Docs */
/*$
{function scope="private":
	{name: rvSrtpAuthenticationCalcRtp}
	{superpackage: RvSrtpAuthentication}
	{include: rvsrtpauthentication.h}
	{description:
		{p: This calculates the authentication tag for an RTP packet.}
		{p: Currently, the only authentication algorithm supported is
			HMAC-SHA1 (RV_SRTP_AUTHENTICATION_HMACSHA1).}
	}
	{proto: RvRtpStatus rvSrtpAuthenticationCalcRtp(RvInt authType, RvUint8 *buf, RvSize_t bufSize, RvUint8 *key, RvSize_t keySize, RvUint8 *tag, RvSize_t tagSize, RvUint32 roc);}
	{params:
		{param: {t: RvInt}     {n: authType} {d: The authentication algorithm type.}}
		{param: {t: RvUint8 *} {n: buf}      {d: Data to calulate tag for.}}
		{param: {t: RvSize_t}  {n: bufSize}  {d: Data length (in bytes).}}
		{param: {t: RvUint8 *} {n: key}      {d: The authentication key.}}
		{param: {t: RvSize_t}  {n: keySize}  {d: The length of the authentication key (in bytes).}}
		{param: {t: RvUint8 *} {n: tag}      {d: Buffer to store authentication tag.}}
		{param: {t: RvSize_t}  {n: tagSize}  {d: The length of the authentication tag (in bytes).}}
		{param: {t: RvUint32}  {n: roc}      {d: The current roll over counter value (ROC).}}
	}
	{returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function scope="private":
	{name: rvSrtpAuthenticationCalcRtcp}
	{superpackage: RvSrtpAuthentication}
	{include: rvsrtpauthentication.h}
	{description:
		{p: This calculates the authentication tag for an RTCP packet.}
		{p: Currently, the only authentication algorithm supported is
			HMAC-SHA1 (RV_SRTP_AUTHENTICATION_HMACSHA1).}
		{p: The input buffer size must be a multiple of the
			block size reported by the rvSrtpAuthenticationGetBlockSize
			function.}
	}
	{proto: RvRtpStatus rvSrtpAuthenticationCalcRtp(RvInt authType, RvUint8 *buf, RvSize_t bufSize, RvUint8 *key, RvSize_t keySize, RvUint8 *tag, RvSize_t tagSize);}
	{params:
		{param: {t: RvInt}     {n: authType} {d: The authentication algorithm type.}}
		{param: {t: RvUint8 *} {n: buf}      {d: Data to calulate tag for.}}
		{param: {t: RvSize_t}  {n: bufSize}  {d: Data length (in bytes).}}
		{param: {t: RvUint8 *} {n: key}      {d: The authentication key.}}
		{param: {t: RvSize_t}  {n: keySize}  {d: The length of the authentication key (in bytes).}}
		{param: {t: RvUint8 *} {n: tag}      {d: Buffer to store authentication tag.}}
		{param: {t: RvSize_t}  {n: tagSize}  {d: The length of the authentication tag (in bytes).}}
	}
	{returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function scope="private":
	{name: rvSrtpAuthenticationGetBlockSize}
	{superpackage: RvSrtpAuthentication}
	{include: rvsrtpauthentication.h}
	{description:
        {p: This function gets the block size that the authentication algorithm
			requires the data to be in. The size of all input data must
			be a multiple of this value.}
		{p: Currently, the only authentication algorithm supported is
			HMAC-SHA1 (RV_SRTP_AUTHENTICATION_HMACSHA1).}
	}
	{proto: RvSize_t rvSrtpAuthenticationGetBlockSize(RvInt authType);}
	{params:
		{param: {t: RvInt}     {n: autType} {d: The authentication algorithm type.}}
	}
	{returns: The required data block size of the authentication algorithm.}
}
$*/

#if defined(__cplusplus)
}
#endif

#endif
