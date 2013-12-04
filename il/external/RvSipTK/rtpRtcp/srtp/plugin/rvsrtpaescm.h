/***********************************************************************
Filename   : rvsrtpaescm.h
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
#ifndef RV_SRTP_AES_CM_H
#define RV_SRTP_AES_CM_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "rvsrtpaesplugin.h"
#include "rvrtpstatus.h"

/*$
{package scope="private":
	{name: RvSrtpAesCm}
	{superpackage: RvSrtpEncryptionPlugin}
	{include: rvsrtpaescm.h}
	{description:
		{p: This module implements the segmented integer counter mode
			of AES (AES-CM) using 128 bit blocks.}
	}
}
$*/

/* Key types - use required values, as per RFC */
#define RV_SRTP_AESCMKEYTYPE_RTPENCRYPT  RvUint32Const(0x0)
#define RV_SRTP_AESCMKEYTYPE_RTPAUTH     RvUint32Const(0x1)
#define RV_SRTP_AESCMKEYTYPE_RTPSALT     RvUint32Const(0x2)
#define RV_SRTP_AESCMKEYTYPE_RTCPENCRYPT RvUint32Const(0x3)
#define RV_SRTP_AESCMKEYTYPE_RTCPAUTH    RvUint32Const(0x4)
#define RV_SRTP_AESCMKEYTYPE_RTCPSALT    RvUint32Const(0x5)

RvRtpStatus rvSrtpAesCmCreateKey(
        RvSrtpAesPlugIn* plugin,
        RvSrtpEncryptionPurpose purpose,
        RvUint keyType,
        RvUint8 *key,
        RvSize_t keySize,
        RvUint8 *masterKey,
        RvSize_t masterKeySize,
        RvUint8 *masterSalt,
        RvSize_t masterSaltSize,
        RvSize_t keyDerivationRate,
        RvUint64 index);

RvRtpStatus rvSrtpAesCmProcessRtp(RvSrtpAesPlugIn* plugin, RvSrtpEncryptionPurpose purpose, RvUint8 *inputBuf, RvUint8 *outputBuf, RvSize_t bufSize, RvUint8 *key, RvSize_t keySize, RvUint8 *salt, RvSize_t saltSize, RvUint32 ssrc, RvUint64 index);
RvRtpStatus rvSrtpAesCmProcessRtcp(RvSrtpAesPlugIn* plugin, RvSrtpEncryptionPurpose purpose, RvUint8 *inputBuf, RvUint8 *outputBuf, RvSize_t bufSize, RvUint8 *key, RvSize_t keySize, RvUint8 *salt, RvSize_t saltSize, RvUint32 ssrc, RvUint32 index);
RvSize_t rvSrtpAesCmGetBlockSize(void);

#if defined(RV_TEST_CODE)
void rvSrtpAesCmTest(void);
#endif /* RV_TEST_CODE */

/* Function Docs */
/*$
{function scope="private":
	{name: rvSrtpAesCmCreateKey}
	{superpackage: RvSrtpAesCm}
	{include: rvsrtpaescm.h}
	{description:
		{p: This function implements the AES algorithm in counter mode
			to generate a session key. There are 6 different keys that
			can be generated:
			RTP encryption key (RV_SRTP_AESCMKEYTYPE_RTPENCRYPT),
			RTP authentication key (RV_SRTP_AESCMKEYTYPE_RTPAUTH),
			RTP salt (RV_SRTP_AESCMKEYTYPE_RTPSALT),
			RTCP encryption key (RV_SRTP_AESCMKEYTYPE_RTCPENCRYPT),
			RTCP authentication key (RV_SRTP_AESCMKEYTYPE_RTCPAUTH),
			RTCP salt (RV_SRTP_AESCMKEYTYPE_RTCPSALT).}
		}
	{proto: RvRtpStatus rvSrtpAesCmCreateKey(RvUint32 keyType, RvUint8 *key, RvSize_t keySize, RvUint8 *masterKey, RvSize_t masterKeySize, *RvUint8 *masterSalt, RvSize_t masterSaltSize, RvSize_t keyDerivationRate, RvUint64 index);}
	{params:
		{param: {t: RvInt} {n: key} {d: The type of key to generate.}}
		{param: {t: RvUint8 *} {n: key} {d: The location to store the key.}}
		{param: {t: RvSize_t}  {n: keySize} {d: The size of the key to generate (in bytes).}}
		{param: {t: RvUint8 *} {n: masterKey} {d: The master key.}}
		{param: {t: RvSize_t}  {n: masterKeySize} {d: The size of the master key (in bytes).}}
		{param: {t: RvUint8 *} {n: masterSalt} {d: The master salt.}}
		{param: {t: RvSize_t}  {n: masterSaltSize} {d: The size of the master salt (in bytes).}}
		{param: {t: RvSize_t}  {n: keyDerivationRate} {d: The key derivation rate.}}
		{param: {t: RvUint64}  {n: index} {d: The current index (RTP or RTCP, as appropriate).}}
	}
	{returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
	{function scope="private":
	{name: rvSrtpAesCmProcessRtp}
	{superpackage: RvSrtpAesCm}
	{include: rvsrtpaescm.h}
	{description:
		{p: This function implements the AES algorithm in counter mode.
			It processes (encrypts or decrypts) the provided buffer
			using the required RTP parameters.}
		{p: The input and output buffer size must be a multiple of the
			block size reported by the rvSrtpAesCmGetBlockSize
			function.}
		{p: If the input buffer and the output buffer are the same, the
			result will overwrite the original.}
	}
	{proto: RvRtpStatus rvSrtpAesCmProcessRtp(RvUint8 *inputBuf, RvUint8 *outputBuf, RvSize_t bufSize, RvUint8 *key, RvSize_t keySize, RvUint8 *salt, RvSize_t saltSize, RvUint32 ssrc, RvUint64 index);}
	{params:
		{param: {t: RvUint8 *} {n: inputBuf} {d: The buffer to be operated on.}}
		{param: {t: RvUint8 *} {n: outputBuf} {d: The location to store the result.}}
		{param: {t: RvSize_t}  {n: bufSize} {d: The data size of the input and output buffers.}}
		{param: {t: RvUint8 *} {n: key} {d: The key.}}
		{param: {t: RvSize_t}  {n: keySize} {d: The size of the key (in bytes).}}
		{param: {t: RvUint8 *} {n: salt} {d: The salt.}}
		{param: {t: RvSize_t}  {n: saltSize} {d: The size of the salt (in bytes).}}
		{param: {t: RvUint32} {n: ssrc} {d: The ssrc.}}
		{param: {t: RvUint64} {n: index} {d: The RTP index (48 bits).}}
	}
	{returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
	}
$*/
/*$
	{function scope="private":
	{name: rvSrtpAesCmProcessRtcp}
	{superpackage: RvSrtpAesCm}
	{include: rvsrtpaescm.h}
	{description:
		{p: This function implements the AES algorithm in counter mode.
			It processes (encrypts or decrypts) the provided buffer
			using the required RTCP parameters.}
		{p: The input and output buffer size must be a multiple of the
			block size reported by the rvSrtpAesCmGetBlockSize
			function.}
		{p: If the input buffer and the output buffer are the same, the
			result will overwrite the original.}
	}
	{proto: RvRtpStatus rvSrtpAesCmProcessRtcp(RvUint8 *inputBuf, RvUint8 *outputBuf, RvSize_t bufSize, RvUint8 *key, RvSize_t keySize, RvUint8 *salt, RvSize_t saltSize, RvUint32 ssrc, RvUint32 index);}
	{params:
		{param: {t: RvUint8 *} {n: inputBuf} {d: The buffer to be operated on.}}
		{param: {t: RvUint8 *} {n: outputBuf} {d: The location to store the result.}}
		{param: {t: RvSize_t}  {n: bufSize} {d: The data size of the input and output buffers.}}
		{param: {t: RvUint8 *} {n: key} {d: The key.}}
		{param: {t: RvSize_t}  {n: keySize} {d: The size of the key (in bytes).}}
		{param: {t: RvUint8 *} {n: salt} {d: The salt.}}
		{param: {t: RvSize_t}  {n: saltSize} {d: The size of the salt (in bytes).}}
		{param: {t: RvUint32} {n: ssrc} {d: The ssrc.}}
		{param: {t: RvUint64} {n: index} {d: The RTCP index (31 bits).}}
	}
	{returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
	}
$*/
/*$
{function scope="private":
	{name: rvSrtpAesCmGetBlockSize}
	{superpackage: RvSrtpAesCm}
	{include: rvsrtpaescm.h}
	{description:
        {p: This function gets the block size that the AES-CM algorithm
			requires the data to be in. The size of all input data must
			be a multiple of this value.}
	}
	{proto: RvSize_t rvSrtpAesCmGetBlockSize(void);}
	{returns: The AES-CM required data block size.}
}
$*/

#if defined(__cplusplus)
}
#endif

#endif
