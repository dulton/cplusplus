/************************************************************************
 File Name	   : rvsrtpprocess.h
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
#if !defined(RVSRTP_PROCESS_H)
#define RVSRTP_PROCESS_H

#include "rvtypes.h"
#include "rvdatabuffer.h"
#include "rvrtpstatus.h"
#include "rvsrtpdb.h"
#include "rvsrtpauthentication.h"
#include "rvsrtpkeyderivation.h"
#include "rvsrtpaesplugin.h"

#ifdef __cplusplus
extern "C" {
#endif

/*$
{type scope="private":
	{name: RvSrtpProcess}
	{superpackage: RvSrtpEncryptionPlugin}
	{include: rvsrtprocess.h}
	{description:
        {p: This package provides processing function for sending and
			received SRTP and SRTCP packets.}
	}
}
$*/
typedef struct {
	RvSrtpDb *dbPtr;            /* Database to use */
	RvSrtpKeyDerivation *keyDerivationPtr; /* Key derivation to use */
	RvSize_t prefixLen;         /* Size of prefix in packet (default 0) */
	RvUint8 *authTagBuf;        /* Buffer for temporarily storing authentication tags */
    RvSrtpAesPlugIn* AESPlugin;  /* pointer to the AES plug in */
	/* RTP encyrption info */
	RvInt    srtpEncAlg;
	RvBool   srtpMki;            /* RV_TRUE = inlcude mki in packet */

	/* RTP authentication info */
	RvInt    srtpAuthAlg;
	RvSize_t srtpAuthTagSize;

	/* RTCP encyrption info */
	RvInt    srtcpEncAlg;
	RvBool   srtcpMki;            /* RV_TRUE = inlcude mki in packet */

	/* RTCP authentication info */
	RvInt    srtcpAuthAlg;
	RvSize_t srtcpAuthTagSize;
} RvSrtpProcess;

/* Encyrption types */
#define RV_SRTPPROCESS_ENCYRPTIONTYPE_NULL  RvIntConst(0)
#define RV_SRTPPROCESS_ENCYRPTIONTYPE_AESCM RvIntConst(1)
#define RV_SRTPPROCESS_ENCYRPTIONTYPE_AESF8 RvIntConst(2)

/* Authentication types */
#define RV_SRTPPROCESS_AUTHENTICATIONTYPE_NONE     RV_SRTP_AUTHENTICATION_NONE
#define RV_SRTPPROCESS_AUTHENTICATIONTYPE_HMACSHA1 RV_SRTP_AUTHENTICATION_HMACSHA1

/* base functions */
RvRtpStatus rvSrtpProcessConstruct(RvSrtpProcess *thisPtr, RvSrtpDb *dbPtr, RvSrtpKeyDerivation *keyDerivationPtr, RvSrtpAesPlugIn* plugin);
RvRtpStatus rvSrtpProcessDestruct(RvSrtpProcess *thisPtr);
RvSize_t rvSrtpProcessGetHeaderSize(RvSrtpProcess *thisPtr, RvBool forRtp);
RvSize_t rvSrtpProcessGetFooterSize(RvSrtpProcess *thisPtr, RvBool forRtp);
RvUint32 rvSrtpProcessGetPaddingSize(RvSrtpProcess *thisPtr, RvBool forRtp);
RvSize_t rvSrtpProcessGetTransmitSize(RvSrtpProcess *thisPtr, RvUint32 packetSize, RvUint32 rtpHeaderSize, RvBool encrypt, RvBool forRtp);

/* Configuration */
RvRtpStatus rvSrtpProcessSetEncryption(RvSrtpProcess *thisPtr, RvInt rtpType, RvInt rtcpType);
RvRtpStatus rvSrtpProcessSetAuthentication(RvSrtpProcess *thisPtr, RvInt rtpType, RvSize_t rtpTagSize, RvInt rtcpType, RvSize_t rtcpTagSize);
RvRtpStatus rvSrtpProcessUseMki(RvSrtpProcess *thisPtr, RvBool rtpMki, RvBool rtcpMki);
RvRtpStatus rvSrtpProcessSetPrefixLength(RvSrtpProcess *thisPtr, RvSize_t prefixLength);

/* Processing functions */
#ifdef UPDATED_BY_SPIRENT
RvRtpStatus rvSrtpProcessDecryptRtp(RvSrtpProcess *thisPtr, RvSrtpStream *streamPtr, RvDataBuffer *inputBufPtr, RvDataBuffer *outputBufPtr, RvUint32 *rocPtr, RvSrtpMasterKeyEventCB masterKeyEvent, void* context);
RvRtpStatus rvSrtpProcessDecryptRtcp(RvSrtpProcess *thisPtr, RvSrtpStream *streamPtr, RvDataBuffer *inputBufPtr, RvDataBuffer *outputBufPtr, RvUint32 *indexPtr, RvBool *encryptedPtr, RvSrtpMasterKeyEventCB masterKeyEvent, void* context);
RvRtpStatus rvSrtpProcessEncryptRtp(RvSrtpProcess *thisPtr, RvSrtpStream *streamPtr, RvDataBuffer *inputBufPtr, RvDataBuffer *outputBufPtr, RvUint32 roc, RvSrtpMasterKeyEventCB masterKeyEvent, void* context);
RvRtpStatus rvSrtpProcessEncryptRtcp(RvSrtpProcess *thisPtr, RvSrtpStream *streamPtr, RvDataBuffer *inputBufPtr, RvDataBuffer *outputBufPtr, RvUint32 index, RvBool encrypt, RvSrtpMasterKeyEventCB masterKeyEvent, void* context);
#else
RvRtpStatus rvSrtpProcessDecryptRtp(RvSrtpProcess *thisPtr, RvSrtpStream *streamPtr, RvDataBuffer *inputBufPtr, RvDataBuffer *outputBufPtr, RvUint32 *rocPtr);
RvRtpStatus rvSrtpProcessDecryptRtcp(RvSrtpProcess *thisPtr, RvSrtpStream *streamPtr, RvDataBuffer *inputBufPtr, RvDataBuffer *outputBufPtr, RvUint32 *indexPtr, RvBool *encryptedPtr);
RvRtpStatus rvSrtpProcessEncryptRtp(RvSrtpProcess *thisPtr, RvSrtpStream *streamPtr, RvDataBuffer *inputBufPtr, RvDataBuffer *outputBufPtr, RvUint32 roc);
RvRtpStatus rvSrtpProcessEncryptRtcp(RvSrtpProcess *thisPtr, RvSrtpStream *streamPtr, RvDataBuffer *inputBufPtr, RvDataBuffer *outputBufPtr, RvUint32 index, RvBool encrypt);
#endif // UPDATED_BY_SPIRENT

#if defined(RV_TEST_CODE)
RvRtpStatus rvSrtpProcessTest();
#endif /* RV_TEST_CODE */

/* Function Docs */
/*$
{function scope="private":
    {name:    rvSrtpProcessConstruct}
    {class:   RvSrtpProcess}
    {include: rvsrtpprocess.h}
    {description:
        {p: This function constructs a processing object.}
    }
    {proto: RvRtpStatus rvSrtpProcessConstruct(RvSrtpProcess *thisPtr, RvSrtpDb *dbPtr, RvSrtpKeyDerivation *keyDerivationPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpProcess object to construct.}}
        {param: {n:dbPtr} {d:The database to use.}}
        {param: {n:keyDerivationPtr} {d:The key derivation to use.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if constructed sucessfully. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpProcessDestruct}
    {class:   RvSrtpProcess}
    {include: rvsrtpprocess.h}
    {description:
        {p: This function destructs a processing object.}
    }
    {proto: RvRtpStatus rvSrtpProcessDestruct(RvSrtpProcess *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpProcess object to destruct.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if destructed sucessfully. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpProcessGetHeaderSize}
    {class:   RvSrtpProcess}
    {include: rvsrtpprocess.h}
    {description:
        {p: This function returns the size of header that is added to
			the front of the RTP or RTCP packet.}
    }
    {proto: RvUint32 rvSrtpProcessGetHeaderSize(RvSrtpProcess *thisPtr, RvBool forRtp);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpProcess object.}}
		{param: {n:forRtp}  {d:RV_TRUE = Return header size for RTP, RV_FALSE = for RTCP.}}
    }
    {returns: Header size.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpProcessGetFooterSize}
    {class:   RvSrtpProcess}
    {include: rvsrtpprocess.h}
    {description:
        {p: This function returns the size of footer that is added to
			the end of the RTP or RTCP packet.}
    }
    {proto: RvUint32 rvSrtpProcessGetFooterSize(RvSrtpProcess *thisPtr, RvBool forRtp);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpProcess object.}}
		{param: {n:forRtp}  {d:RV_TRUE = Return footer size for RTP, RV_FALSE = for RTCP.}}
    }
    {returns: Footer size.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpProcessGetPaddingSize}
    {class:   RvSrtpProcess}
    {include: rvsrtpprocess.h}
    {description:
        {p: This function returns worst case padding size that the
			combined encyrption and authentication algorithms will add to a
			packet.}
    }
    {proto: RvUint32 rvSrtpProcessGetPaddingSize(RvSrtpProcess *thisPtr, RvBool forRtp);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpProcess object.}}
		{param: {n:forRtp}  {d:RV_TRUE = Return padding size for RTP, RV_FALSE = for RTCP.}}
    }
    {returns: Maximum padding size.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpProcessGetTransmitSize}
    {class:   RvSrtpProcess}
    {include: rvsrtpprocess.h}
    {description:
        {p: This function returns the actual packet size that will be
			transmitted given an unencrypted packet of a specified size. This
			is used when the size of the final packet size needs to be known
			before the packet is encrypted (or perhaps even before it is
			created).}
    }
    {proto: RvSize_t rvSrtpProcessGetTransmitSize(RvSrtpProcess *thisPtr, RvUint32 packetSize, RvUint32 rtpHeaderSize, RvBool encrypt, RvBool forRtp);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpProcess object.}}
        {param: {n:packetSize} {d:The packet size of the entire packet that would be sent.}}
        {param: {n:headerSize} {d:The size of the header portion of the packet (RTP only).}}
		{param: {n:encrypt} {d:RV_TRUE = packet to be encrypted, RV_FALSE = NULL encryption (RTCP only).}}
		{param: {n:forRtp}  {d:RV_TRUE = Return footer size for RTP, RV_FALSE = for RTCP.}}
    }
    {returns: Size of the packet that would be transmitted.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpProcessSetEncryption}
    {class:   RvSrtpProcess}
    {include: rvsrtpprocess.h}
    {description:
        {p: This function sets the type of encrytion to use for RTP
			and RTCP. Currently supported encryption types are
			AES-CM (RV_SRTPPROCESS_ENCYRPTIONTYPE_AESCM),
			AES-F8 (RV_SRTPPROCESS_ENCYRPTIONTYPE_AESF8),
			no encyrption (RV_SRTPPROCESS_ENCYRPTIONTYPE_NULL).}
		{p: This must be set before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpProcessSetEncryption(RvSrtpProcess *thisPtr, RvInt rtpType, RvInt rtcpType);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpProcess object.}}
        {param: {n:rtpType} {d:Type of encryption to use for RTP.}}
        {param: {n:rtcpType} {d:Type of encryption to use for RTCP.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpProcessSetAuthentication}
    {class:   RvSrtpProcess}
    {include: rvsrtpprocess.h}
    {description:
        {p: This function sets the type of authentication to use for RTP
			and RTCP along with the size of the authentication tag that
			will be included in each packet. Currently supported
			authentication types are
			HMAC-SHA1 (RV_SRTPPROCESS_AUTHENTICATIONTYPE_HMACSHA1),
			no authentication (RV_SRTPPROCESS_AUTHENTICATIONTYPE_NONE).}
		{p: The authentication tags sizes must have a value greater
			than zero, unless no authentication is to be used (then the
			size is ignored).}
    }
    {proto: RvRtpStatus rvSrtpProcessSetAuthentication(RvSrtpProcess *thisPtr, RvInt rtpType, RvSize_t rtpSize, RvInt rtcpType, RvSize_t rtcpSize);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpProcess object.}}
        {param: {n:rtpType} {d:Type of authentication to use for RTP.}}
        {param: {n:rtpSize} {d:The size of the authentication tag to use for RTP.}}
        {param: {n:rtcpType} {d:Type of authentication to use for RTCP.}}
        {param: {n:rtcpSize} {d:The size of the authentication tag to use for RTCP.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpProcessUseMki}
    {class:   RvSrtpProcess}
    {include: rvsrtpprocess.h}
    {description:
        {p: This function specifies if the MKI tags should be included
			in the SRTP and SRTCP packets.}
    }
    {proto: RvRtpStatus rvSrtpProcessUseMki(RvSrtpProcess *thisPtr, RvBool rtpMki, RvBool rtcpMki);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpProcess object.}}
        {param: {n:rtpMki}  {d:RV_TRUE to use MKI in RTP packet, RV_FALSE to not include it.}}
        {param: {n:rtcpMki}  {d:RV_TRUE to use MKI in RTCP packet, RV_FALSE to not include it.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpProcessSetPrefixLength}
    {class:   RvSrtpProcess}
    {include: rvsrtpprocess.h}
    {description:
        {p: This function sets keystream prefix length (as per the
			RFC). This is normally set to 0 for standard SRTP/SRTCP.}
    }
    {proto: RvRtpStatus rvSrtpProcessSetPrefixLength(RvSrtp *thisPtr, RvSize_t prefixLength);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpProcess object.}}
        {param: {n:perfixLength} {d:The length of the keystream prefix.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpProcessDecryptRtp}
    {class:   RvSrtpProcess}
    {include: rvsrtpprocess.h}
    {description:
        {p: This decrypts an RTP packet for the specificc remote source
			stream. The resulting roc value is calculated and returned
			if the decryption is successful.}
    }
    {proto: RvRtpStatus rvSrtpProcessDecryptRtp(RvSrtpProcess *thisPtr, RvSrtpStream *streamPtr, RvDataBuffer *inputBufPtr, RvDataBuffer *outputBufPtr, RvUint32 *rocPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpProcess object.}}
        {param: {n:streamPtr} {d:The stream representing the remote source.}}
        {param: {n:inputBuffer} {d:The data buffer to decrypt.}}
        {param: {n:outputBuffer} {d:The data buffer to store the decrypted data in.}}
        {param: {n:rocPtr} {d:Pointer to location to store calculated ROC value.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpProcessDecryptRtcp}
    {class:   RvSrtpProcess}
    {include: rvsrtpprocess.h}
    {description:
        {p: This decrypts an RTCP packet for the specific remote source
			stream. The RTCP index value is returned along with an
			indication if the packet was actually encrypted.}
    }
    {proto: RvRtpStatus rvSrtpProcessDecryptRtcp(RvSrtpProcess *thisPtr, RvSrtpStream *streamPtr, RvDataBuffer *inputBufPtr, RvDataBuffer *outputBufPtr, RvUint32 *indexPtr, RvBool *encryptedPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpProcess object.}}
        {param: {n:streamPtr} {d:The stream representing the remote source.}}
        {param: {n:inputBuffer} {d:The data buffer to decrypt.}}
        {param: {n:outputBuffer} {d:The data buffer to store the decrypted data in.}}
        {param: {n:indexPtr} {d:Location to store RTCP index.}}
        {param: {n:encryptedPtr} {d:Location to store encryped flag (RV_TRUE = encrypted, RV_FALSE = no encrypted).}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpProcessEncryptRtp}
    {class:   RvSrtpProcess}
    {include: rvsrtpprocess.h}
    {description:
        {p: This encrypts an RTP packet for the specific detsination
			stream. The ROC value to use must be provided.}
    }
    {proto: RvRtpStatus rvSrtpProcessEncryptRtp(RvSrtpProcess *thisPtr, RvSrtpStream *streamPtr, RvDataBuffer *inputBufPtr, RvDataBuffer *outputBufPtr, RvUint32 roc);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpProcess object.}}
        {param: {n:streamPtr} {d:The stream representing the destination.}}
        {param: {n:inputBuffer} {d:The data buffer to encrypt.}}
        {param: {n:outputBuffer} {d:The data buffer to store the encrypted data in.}}
        {param: {n:roc} {d:ROC value.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpProcessEncryptRtcp}
    {class:   RvSrtpProcess}
    {include: rvsrtpprocess.h}
    {description:
        {p: This encrypts an RTCP packet for the specific destination
			stream. The RTCP index value and a flag indicating if the
			packet should actually be encrypted must be provided.}
    }
    {proto: RvRtpStatus rvSrtpProcessEncryptRtcp(RvSrtpProcess *thisPtr, RvSrtpStream *streamPtr, RvDataBuffer *inputBufPtr, RvDataBuffer *outputBufPtr, RvUint32 index, RvBool encrypted);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpProcess object.}}
        {param: {n:streamPtr} {d:The stream representing the destination.}}
        {param: {n:inputBuffer} {d:The data buffer to encrypt.}}
        {param: {n:outputBuffer} {d:The data buffer to store the encrypted data in.}}
        {param: {n:index} {d:RTCP index.}}
        {param: {n:encrypted} {d:RV_TRUE = encrypt packet, RV_FALSE = do no encrypt.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/

#ifdef __cplusplus
}
#endif

#endif
