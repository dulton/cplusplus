/***********************************************************************
Filename   : rvsrtpaesf8.h
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
#ifndef RV_SRTP_AES_F8_H
#define RV_SRTP_AES_F8_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "rvrtpstatus.h"
#include "rvsrtpaesplugin.h"
/*$
{package scope="private":
	{name: RvSrtpAesF8}
	{superpackage: RvSrtpEncryptionPlugin}
	{include: rvsrtpaescm.h}
	{description:
		{p: This module implements the F8 mode of operating of AES (AES-F8).}
	}
}
$*/

RvRtpStatus rvSrtpAesF8ProcessRtp(
    RvSrtpAesPlugIn* plugin,
    RvSrtpEncryptionPurpose purpose,
    RvUint8 *inputBuf,
    RvUint8 *outputBuf,
    RvSize_t bufSize,
    RvUint8 *key,
    RvSize_t keySize,
    RvUint8 *salt,
    RvSize_t saltSize,
    RvBool mBit,
    RvUint8 packetType,
    RvUint16 seqNum,
    RvUint32 timestamp,
    RvUint32 ssrc,
    RvUint32 roc);

RvRtpStatus rvSrtpAesF8ProcessRtcp(
    RvSrtpAesPlugIn* plugin,
    RvSrtpEncryptionPurpose purpose,
    RvUint8 *inputBuf,
    RvUint8 *outputBuf,
    RvSize_t bufSize,
    RvUint8 *key,
    RvSize_t keySize,
    RvUint8 *salt,
    RvSize_t saltSize,
    RvBool eBit,
    RvUint32 index,
    RvUint8 version,
    RvBool padBit,
    RvUint8 count,
    RvUint8 packetType,
    RvUint16 length,
    RvUint32 ssrc);

RvSize_t rvSrtpAesF8GetBlockSize(void);

#if defined(RV_TEST_CODE)
void RvSrtpAesF8Test(void);
#endif /* RV_TEST_CODE */

/* Function docs */
/*$
	{function scope="private":
	{name: rvSrtpAesF8ProcessRtp}
	{superpackage: RvSrtpAesF8}
	{include: rvsrtpaescm.h}
	{description:
		{p: This function implements the AES algorithm in F8 mode.
			It processes (encrypts or decrypts) the provided buffer
			using the required RTP parameters.}
		{p: The input and output buffer size must be a multiple of the
			block size reported by the rvSrtpAesF8GetBlockSize
			function.}
		{p: If the input buffer and the output buffer are the same, the
			result will overwrite the original.}
	}
	{proto: RvRtpStatus RvSrtpAesF8ProcessRtp(RvUint8 *inputBuf, RvUint8 *outputBuf, RvSize_t bufSize, RvUint8 *key, RvSize_t keySize, RvUint8 *salt, RvSize_t saltSize, RvBool mBit, RvUint8 packetType, RvUint16 seqNum, RvUint32 timestamp, RvUint32 ssrc, RvUint32 roc);}
	{params:
		{param: {t: RvUint8 *} {n: inputBuf} {d: The buffer to be operated on.}}
		{param: {t: RvUint8 *} {n: outputBuf} {d: The location to store the result.}}
		{param: {t: RvSize_t}  {n: bufSize} {d: The data size of the input and output buffers.}}
		{param: {t: RvUint8 *} {n: key} {d: The key.}}
		{param: {t: RvSize_t}  {n: keySize} {d: The size of the key (in bytes).}}
		{param: {t: RvUint8 *} {n: salt} {d: The salt.}}
		{param: {t: RvSize_t}  {n: saltSize} {d: The size of the salt (in bytes).}}
		{param: {t: RvBool}    {n: mBit} {d: The marker bit.}}
		{param: {t: RvUint8}   {n: packetType} {d: The packet type (7 bits).}}
		{param: {t: RvUint16}  {n: seqNum} {d: The sequence number (16 bits).}}
		{param: {t: RvUint32}  {n: timestamp} {d: The timestamp (32 bits).}}
		{param: {t: RvUint32}  {n: ssrc} {d: The ssrc (32 bits).}}
		{param: {t: RvUint32}  {n: roc} {d: The roll over counter (ROC, 32 bits).}}
	}
	{returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
	}
$*/
/*$
	{function scope="private":
	{name: rvSrtpAesF8ProcessRtcp}
	{superpackage: RvSrtpAesF8}
	{include: rvsrtpaescm.h}
	{description:
		{p: This function implements the AES algorithm in F8 mode.
			It processes (encrypts or decrypts) the provided buffer
			using the required RTCP parameters.}
		{p: The input and output buffer size must be a multiple of the
			block size reported by the rvSrtpAesF8GetBlockSize
			function.}
		{p: If the input buffer and the output buffer are the same, the
			result will overwrite the original.}
	}
	{proto: RvRtpStatus RvSrtpAesF8ProcessRtcp(RvUint8 *inputBuf, RvUint8 *outputBuf, RvSize_t bufSize, RvUint8 *key, RvSize_t keySize, RvUint8 *salt, RvSize_t saltSize, RvBool eBit, RvUint32 index, RvUint8 version, RvBool padBit, RvUint8 count, RvUint8 packetType, RvUint16 length, RvUint32 ssrc);}
	{params:
		{param: {t: RvUint8 *} {n: inputBuf} {d: The buffer to be operated on.}}
		{param: {t: RvUint8 *} {n: outputBuf} {d: The location to store the result.}}
		{param: {t: RvSize_t}  {n: bufSize} {d: The data size of the input and output buffers.}}
		{param: {t: RvUint8 *} {n: key} {d: The key.}}
		{param: {t: RvSize_t}  {n: keySize} {d: The size of the key (in bytes).}}
		{param: {t: RvUint8 *} {n: salt} {d: The salt.}}
		{param: {t: RvSize_t}  {n: saltSize} {d: The size of the salt (in bytes).}}
		{param: {t: RvBool}    {n: eBit} {d: The encryption bit.}}
		{param: {t: RvUint32}  {n: index} {d: The index (31 bits).}}
		{param: {t: RvUint8}   {n: version} {d: The version number (2 bits).}}
		{param: {t: RvBool}    {n: padBit} {d: The padding bit.}}
		{param: {t: RvUint8}   {n: count} {d: The reception report count (5 bits).}}
		{param: {t: RvUint8}   {n: packetType} {d: The packet type (8 bits).}}
		{param: {t: RvUint16}  {n: length} {d: The packet length (16 bits).}}
		{param: {t: RvUint32}  {n: ssrc} {d: The ssrc (32 bits).}}
	}
	{returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
	}
$*/
/*$
{function scope="private":
	{name: rvSrtpAesCmGetBlockSize}
	{superpackage: RvSrtpAesF8}
	{include: rvsrtpaesf8.h}
	{description:
        {p: This function gets the block size that the AES-F8 algorithm
			requires the data to be in. The size of all input data must
			be a multiple of this value.}
	}
	{proto: RvSize_t rvSrtpAesF8GetBlockSize(void);}
	{returns: The AES-F8 required data block size.}
}
$*/

#if defined(__cplusplus)
}
#endif

#endif
