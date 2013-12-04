/***********************************************************************
Filename   : rvsrtpprocess.c
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

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "rvsrtpprocess.h"
#include "rvmemory.h"
#include "rvsrtpaescm.h"
#include "rvsrtpaesf8.h"
#include "rvsrtplog.h"

/*-----------------------------------------------------------------------*/
/*                   Definitions & Constants                             */
/*-----------------------------------------------------------------------*/

/*SRTP header definitions*/
#define RV_SRTP_HEADER_EXT_BIT          RvUint8Const(0x10)
#define RV_SRTP_HEADER_FIXED_LEN        12 /*12 bytes*/
#define RV_SRTP_HEADER_EXT_FIXED_LEN    4  /*4 bytes*/
#define RV_SRTP_HEADER_CSRC_LEN         4  /*4 bytes*/
#define RV_SRTP_HEADER_CSRC_COUNT_BITS  RvUint8Const(0x0F)
#define RV_SRTP_HEADER_MBIT_BITS        RvUint8Const(0x80)
#define RV_SRTP_HEADER_PT_BITS          RvUint8Const(0x7F)

/*SRTCP header definitions*/
#define RV_SRTCP_HEADER_FIXED_LEN       8 /*8 bytes*/
#define RV_SRTCP_HEADER_VERSION_BITS    RvUint8Const(0xC0)
#define RV_SRTCP_HEADER_PADBIT_BITS     RvUint8Const(0x20)
#define RV_SRTCP_HEADER_COUNT_BITS      RvUint8Const(0x1F)

/*index definitions*/
#define RV_SRTP_SRTP_INDEX_SIZE   6 /*48 bit*/
#define RV_SRTP_SRTP_INDEX_MASK   RvUint64Const2(0xFFFFFFFFFFFF)
#define RV_SRTP_SRTCP_INDEX_SIZE  4 /*31 bit*/
#define RV_SRTP_SRTCP_INDEX_MASK  RvUint32Const(0x7FFFFFFF)
#define RV_SRTP_SRTCP_E_BIT       RvUint32Const(0x80000000)

#ifdef UPDATED_BY_SPIRENT
/* Master Key destination types*/
#ifndef RV_SRTP_KEY_LOCAL
#define RV_SRTP_KEY_LOCAL 1
#endif
#ifndef RV_SRTP_KEY_REMOTE
#define RV_SRTP_KEY_REMOTE 0
#endif
#endif // UPDATED_BY_SPIRENT

/*-----------------------------------------------------------------------*/
/*                   internal function                                   */
/*-----------------------------------------------------------------------*/
#undef _SRTP_DEBUG_
#ifdef _SRTP_DEBUG_
static RvChar printBuffer[2000];

static RvChar* dbgPrint(RvUint8* buffer, RvUint32 size)
{
    RvUint32 count=0;
    memset(printBuffer, 0, 2000);
    for (count=0; count<size; count++)
    {
        RvInt32 oneByte = (RvInt32) buffer[count];
        {
            if ((oneByte>>4) <= 9)
                printBuffer[count*2]   =  (RvChar) ((oneByte>>4)   + (RvInt32)'0');
            else
                printBuffer[count*2]   =  (RvChar) ((oneByte>>4) - 10  + (RvInt32)'a');
            if ((oneByte&0x0000000F) <= 9)
                printBuffer[count*2+1] =  (RvChar) ((oneByte&0x0000000F) + (RvInt32)'0');
            else
                printBuffer[count*2+1] =  (RvChar) ((oneByte&0x0000000F) - 10 + (RvInt32)'a');
        }
    }
    return printBuffer;
}
#endif
/*************************************************************************
 * rvSrtpProcessGetRtpParams
 *
 * this function gets the srtp parameters from packet header
 * for the aes-f8 algorithm
 *************************************************************************/
static RvRtpStatus rvSrtpProcessPacketGetRtpParams
(
    RvDataBuffer *inputBufPtr,
    RvBool       *mBit,
    RvUint8      *packetType,
    RvUint16     *seqNum,
    RvUint32     *timestamp,
    RvUint32     *ssrc
)
{
	/* mbit and packet type */
    rvDataBufferReadAtUint8(inputBufPtr, 1, packetType);
    if((*packetType & RV_SRTP_HEADER_MBIT_BITS) != 0)
    {
        *mBit = RV_TRUE;
    }
    else
    {
        *mBit = RV_FALSE;
    }
    *packetType &= RV_SRTP_HEADER_PT_BITS;

    /*sequence number*/
    rvDataBufferReadAtUint16(inputBufPtr, 2, seqNum);

    /*timestamp*/
    rvDataBufferReadAtUint32(inputBufPtr, 4, timestamp);

    /*ssrc*/
    rvDataBufferReadAtUint32(inputBufPtr, 8, ssrc);

    return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
 * rvSrtpProcessPacketGetRtcpParams
 *
 * this function gets the srtcp parameters from packet header
 * for the aes-f8 algorithm
 *************************************************************************/
static RvRtpStatus rvSrtpProcessPacketGetRtcpParams
(
    RvDataBuffer *inputBufPtr,
    RvUint8      *version,
    RvBool       *padBit,
    RvUint8      *count,
    RvUint8      *packetType,
    RvUint16     *length,
    RvUint32     *ssrc
)
{
	/* version, padding bit, and count */
    rvDataBufferReadAtUint8(inputBufPtr, 0, count);
    *version = (RvUint8) (*count >>6);
    if ((*count & RV_SRTCP_HEADER_PADBIT_BITS) != 0)
    {
        *padBit = RV_TRUE;
    }
    else
    {
        *padBit = RV_FALSE;
    }
    *count &= RV_SRTCP_HEADER_COUNT_BITS;

    /*packetType*/
    rvDataBufferReadAtUint8(inputBufPtr, 1, packetType);

    /*length*/
    rvDataBufferReadAtUint16(inputBufPtr, 2, length);

    /*ssrc*/
    rvDataBufferReadAtUint32(inputBufPtr, 4, ssrc);

    return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
 * rvSrtpFooterSpaceCheck
 *
 * this function is used to check wether the upper layer provided enough
 * space in the given buffer for adding the SRTP / SRTCP fields
 * the added field are
 * 1. MKI if MKI is supported - default 2 bytes
 * 2. authentication tag - default 10 bytes
 * 3. SRTCP index - for SRTCP - default 4 bytes
 *************************************************************************/
static RvRtpStatus rvSrtpProcessFooterSpaceCheck
(
    RvSrtpProcess *thisPtr,
    RvDataBuffer  *inputBufPtr,
    RvBool         isRtp
)
{
    RvSize_t availableFooterSpace;
    RvSize_t neededFooterSpace;

    /*calculate the available space*/
    /*
     buffer 		    data				    dataEnd 		 bufferEnd
       |				 |						   |				 |
       V				 V						   V				 V
       +-----------------+-------------------------+-----------------+
       |   Free Space	 |			curData		   |	Free Space	 |
       +-----------------+-------------------------+-----------------+

       |<------------------------ Capacity ------------------------->|

       |<-- Available -->|<------- Length -------->|<-- Available -->|
		      Front 									  Back
		    Capacity									Capacity
    */
    availableFooterSpace = rvDataBufferGetAvailableBackCapacity(inputBufPtr);

    /*calculate the amount needed*/
    if (isRtp == RV_TRUE)
    {
		neededFooterSpace = thisPtr->srtpAuthTagSize;
    }
    else
    {
		neededFooterSpace = RV_SRTP_SRTCP_INDEX_SIZE + thisPtr->srtcpAuthTagSize;
    }
	if (thisPtr->srtcpMki)
		neededFooterSpace += rvSrtpDbGetMkiSize(thisPtr->dbPtr);

    /*compare*/
    if (availableFooterSpace < neededFooterSpace)
        return RV_RTPSTATUS_NotEnoughRoomInDataBuffer;

    return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
 * rvSrtpGetMkiFromPacket
 *
 * this function returns a pointer to the MKI inside a SRTP / SRTCP packet
 * SRTP / SRTCP messages hold the RTP / RTCP message + additional feilds
 * the 2 last feilds are MKI and authentication tag.
 * it will be used on the receivers side in order to determine the master
 * key to use.
 *************************************************************************/
static RvUint8 *rvSrtpProcessGetMkiFromPacket
(
    RvSrtpProcess  *thisPtr,
    RvDataBuffer   *inputBufPtr,
    RvBool          isRtp
)
{
	RvUint8 *mki;

	/* MKI at end of packet minus the authentication tag and mki sizes */
	mki = rvDataBufferGetData(inputBufPtr) + rvDataBufferGetLength(inputBufPtr) - rvSrtpDbGetMkiSize(thisPtr->dbPtr);
	if (isRtp == RV_TRUE)
	{
		mki -= thisPtr->srtpAuthTagSize;
	}
	else
	{
		mki -= thisPtr->srtcpAuthTagSize;
	}
	return mki;
}

/* Get Rtcp index from packet (and the ecryption bit) */
static RvUint32 rvSrtpProcessGetRtcpIndexFromPacket
(
	  RvSrtpProcess  *thisPtr,
	  RvDataBuffer   *inputBufPtr,
	  RvBool         *ebit
)
{
	RvUint32 index;
	RvUint32 indexLoc;

	*ebit = RV_FALSE;

	/* Find where in the buffer the index is */
	indexLoc = rvDataBufferGetLength(inputBufPtr) - thisPtr->srtcpAuthTagSize - RV_SRTP_SRTCP_INDEX_SIZE;
	if(thisPtr->srtcpMki == RV_TRUE)
		indexLoc -= rvSrtpDbGetMkiSize(thisPtr->dbPtr);

	/* Read the index and extract the encryption bit */
	rvDataBufferReadAtUint32(inputBufPtr, indexLoc, &index);
	if((index & RV_SRTP_SRTCP_E_BIT) != 0)
		*ebit = RV_TRUE;
	index &= RV_SRTP_SRTCP_INDEX_MASK;

	return index;
}

/*************************************************************************
 * rvSrtpProcessRtpPayloadFigure
 *
 * this function calculates the length of the RTP packet header, in order
 * to figure out where the payload begins
 * SRTP header can have an extension - depending on the X bit
 * and can have a various number of CSRC - depending on cc - CSRC count
 * payload length is packet length - header length
 *************************************************************************/
static RvUint32 rvSrtpProcessRtpPayloadFigure
(
    RvDataBuffer *inputBufPtr,
    RvUint32     *payloadLen
)
{
	RvUint32 headerSize;
    RvUint8  dataByte;
    RvUint16 extLen;

    /*
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |V=2|P|X|  CC   |M|     PT      |       sequence number         |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                           timestamp                           |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |           synchronization source (SSRC) identifier            |
    +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
    |            contributing source (CSRC) identifiers             |
    |                             ....                              |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */

    headerSize = RV_SRTP_HEADER_FIXED_LEN;
    /*add the CSRC items*/
    rvDataBufferReadAtUint8(inputBufPtr, 0, &dataByte);
	headerSize += (dataByte & RV_SRTP_HEADER_CSRC_COUNT_BITS) * RV_SRTP_HEADER_CSRC_LEN;

    if ((dataByte & RV_SRTP_HEADER_EXT_BIT) != 0)/*header extenstion exists*/
    {
        /*
        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |      defined by profile       |           length              |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                        header extension                       |
        |                             ....                              |
        */
        /*figure extensions length*/
        rvDataBufferReadAtUint16(inputBufPtr, (headerSize + 2), &extLen);
        /*extension length is given in 32 bit words in the extension*/
        headerSize += RV_SRTP_HEADER_EXT_FIXED_LEN + (extLen * 4);
    }
    /*now calculate the length and set the pointer to the payload*/
    *payloadLen = rvDataBufferGetLength(inputBufPtr) - headerSize;

	return headerSize;
}

/*************************************************************************
 * rvSrtpProcessRtcpPayloadFigure
 *
 * this function calculates the length of the RTCP packet header, in order
 * to figure out where the payload begins.
 * the SRTCP header is easy - its fixed.
 * then RTCP packet - header length is the payload
 *************************************************************************/
static RvUint32 rvSrtpProcessRtcpPayloadFigure
(
    RvDataBuffer *inputBufPtr,
    RvUint32     *payloadLen
)
{
	RvUint32 headerSize;

    /*
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |V=2|P|    RC   |   PT=SR or RR   |             length          |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                         SSRC of sender                        |
    +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
    */
    headerSize = RV_SRTCP_HEADER_FIXED_LEN;
    *payloadLen = rvDataBufferGetLength(inputBufPtr) - RV_SRTCP_HEADER_FIXED_LEN;

	return headerSize;
}

/*************************************************************************
 * rvSrtpProcessRtpCipher
 *
 * this function encrypts/decrypts the RTP payload
 * according to the appropriate cipher ,it builds up the initialization
 * vector and calls the AES algorithm.
 *************************************************************************/
static RvRtpStatus rvSrtpProcessRtpCipher
(
	RvSrtpProcess  *thisPtr,
    RvSrtpEncryptionPurpose purpose,
	RvDataBuffer   *inputBufPtr,
	RvDataBuffer   *outputBufPtr,
	RvUint32        headerSize,
	RvUint32        payloadLen,
    RvUint8        *key,
    RvSize_t        keySize,
    RvUint8        *salt,
    RvSize_t        saltSize,
    RvUint32        ssrc,
    RvUint64        index,
    RvUint32        roc
)
{
    RvRtpStatus rc;
	RvBool   mBit;
	RvUint8  packetType;
	RvUint16 seqNum;
	RvUint32 timestamp;

	switch(thisPtr->srtpEncAlg) {
		case RV_SRTPPROCESS_ENCYRPTIONTYPE_NULL:
			rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessRtpCipher: encryption type is NONE - do nothing"));
			break;
		case RV_SRTPPROCESS_ENCYRPTIONTYPE_AESCM:
            rc = rvSrtpAesCmProcessRtp(
                thisPtr->AESPlugin,
                purpose,
                (rvDataBufferGetData(inputBufPtr) + headerSize),
                (rvDataBufferGetData(outputBufPtr) + headerSize),
                payloadLen, key, keySize, salt, saltSize, ssrc, index);
			if (RV_RTPSTATUS_Succeeded != rc)
			{
				rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessRtpCipher: AESCM failed to process"));
				return rc;
			}
            break;
        case RV_SRTPPROCESS_ENCYRPTIONTYPE_AESF8:
			/* Get the parameters from the packet header */
			rc = rvSrtpProcessPacketGetRtpParams(inputBufPtr, &mBit, &packetType,
				                                 &seqNum, &timestamp, &ssrc);
			if (RV_RTPSTATUS_Succeeded != rc)
			{
				return rc;
			}
			rc = rvSrtpAesF8ProcessRtp(
                thisPtr->AESPlugin,
                purpose,
                (rvDataBufferGetData(inputBufPtr) + headerSize),
				(rvDataBufferGetData(outputBufPtr) + headerSize),
				payloadLen, key, keySize, salt, saltSize,
				mBit, packetType, seqNum, timestamp,
				ssrc, roc);
			if (RV_RTPSTATUS_Succeeded != rc)
			{
				rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessRtpCipher: AESF8 failed to process"));
				return rc;
			}
            break;
        default:
            rvSrtpLogWarning ((rvSrtpLogSourcePtr, "rvSrtpProcessRtpCipher: undefined encryption type"));
	}
    return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
 * rvSrtpProcessRtcpCipher
 *
 * this function encrypts/decripts the RTCP payload
 * according to the appropriate cipher ,it builds up the initialization
 * vector and calls the AES algorithm.
 *************************************************************************/
static RvRtpStatus rvSrtpProcessRtcpCipher
(
    RvSrtpProcess *thisPtr,
    RvSrtpEncryptionPurpose purpose,
    RvDataBuffer  *inputBufPtr,
    RvDataBuffer  *outputBufPtr,
	RvUint32       headerSize,
	RvUint32       payloadLen,
    RvUint8       *key,
    RvSize_t       keySize,
    RvUint8       *salt,
    RvSize_t       saltSize,
    RvUint32       ssrc,
    RvUint32       index,
    RvBool         encrypt
)
{
    RvRtpStatus rc;
	RvUint8  version;
	RvBool   padBit;
	RvUint8  count;
	RvUint8  packetType;
	RvUint16 length;
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
   switch(thisPtr->srtcpEncAlg) {
#else
	switch(thisPtr->srtpEncAlg) {
#endif
//SPIRENT_END
		case RV_SRTPPROCESS_ENCYRPTIONTYPE_NULL:
			rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessRtcpCipher: encryption type is NONE - do nothing"));
			break;
		case RV_SRTPPROCESS_ENCYRPTIONTYPE_AESCM:
			rc = rvSrtpAesCmProcessRtcp(
                thisPtr->AESPlugin,
                purpose,
                (rvDataBufferGetData(inputBufPtr) + headerSize),
				(rvDataBufferGetData(outputBufPtr) + headerSize),
				payloadLen, key, keySize, salt, saltSize,
				ssrc, index);
			if (RV_RTPSTATUS_Succeeded != rc)
			{
				rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessRtcpCipher: AESCM failed to process"));
				return rc;
			}
			break;
        case RV_SRTPPROCESS_ENCYRPTIONTYPE_AESF8:
			/* Get the parameters from the packet header */
			rc = rvSrtpProcessPacketGetRtcpParams(outputBufPtr, &version, &padBit, &count,
												  &packetType, &length, &ssrc);
			if (RV_RTPSTATUS_Succeeded != rc)
			{
				return rc;
			}
			rc = rvSrtpAesF8ProcessRtcp(
                thisPtr->AESPlugin,
                purpose,
                (rvDataBufferGetData(inputBufPtr) + headerSize),
										(rvDataBufferGetData(outputBufPtr) + headerSize),
										payloadLen, key, keySize, salt, saltSize,
										encrypt, index, version, padBit, count,
										packetType, length, ssrc);
			if (RV_RTPSTATUS_Succeeded != rc)
			{
				rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessRtcpCipher: AESF8 failed to process"));
				return rc;
			}
            break;
        default:
            rvSrtpLogWarning ((rvSrtpLogSourcePtr, "rvSrtpProcessRtcpCipher: undefined encryption type"));
	}
    return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
 * rvSrtpReceiveSrtpIndexCalc
 *
 * this function calculates the SRTP index on the receivers side according
 * to the RFC and calculates the new maxIndex (if it changed).
 *************************************************************************/
static void rvSrtpProcessSrtpIndexCalc
(
    RvSrtpStream  *streamPtr,
    RvUint16      seq,
	RvUint64      *srtpIndex,
	RvUint64      *maxIndex,
    RvUint32      *rocPtr
)
{
	RvUint16 s_l;
	RvBool newMaxIndex;

	*maxIndex = rvSrtpDbStreamGetMaxIndex(streamPtr);
	*rocPtr = (RvUint32)(*maxIndex >> 16);
	s_l = (RvUint16)(*maxIndex & RvUint64Const2(0xFFFF));
	newMaxIndex = RV_FALSE;

	/* Determine ROC per RFC method (but modified) */
    if (seq > s_l) /*ROC or ROC - 1*/
    {
        if ((RvUint16)(seq - s_l) > RvUint16Const(0x7FFF))/*greater than half the counter*/
			*rocPtr -= 1;
		else
			newMaxIndex = RV_TRUE; /* same ROC, larger Seq # */
    }
    else /*ROC or ROC + 1*/
    {
        if ((RvUint16)(s_l - seq) > RvUint16Const(0x7FFF))/*greater than half the counter*/
		{
			*rocPtr += 1;
			newMaxIndex = RV_TRUE; /* Incremented ROC */
		}
    }

    *srtpIndex = ((RvUint64)(*rocPtr) << 16) | (RvUint64)seq;
	if(newMaxIndex == RV_TRUE)
		*maxIndex = *srtpIndex;
}

/*************************************************************************
* copyInputToOutput
* ------------------------------------------------------------------------
* General: this function sets the pointers inside the output RvDataBuffer
*          structure to point to the appropriate positions as in the input
*          structure - in addition it copies the content of the buffer
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    inputBufPtr - pointer to the input RvDataBuffer structure
*
* Output    outputBufPtr - pointer to the output RvDataBuffer structure
*************************************************************************/
static void rvSrtpProcessCopyInputToOutput
(
    RvDataBuffer *inputBufPtr,
    RvDataBuffer *outputBufPtr
)
{
    RvUint32 dataStart;
    RvUint32 length;

    /*first set the pointers to point to the right place*/
    dataStart = rvDataBufferGetAvailableFrontCapacity(inputBufPtr);
    length = rvDataBufferGetLength(inputBufPtr);
    rvDataBufferSetDataPosition(outputBufPtr, dataStart, length);

    /*now copy the content*/
    memcpy(rvDataBufferGetData(outputBufPtr), rvDataBufferGetData(inputBufPtr), length);
}

/* Return authentication and encryption block sizes */
static void rvSrtpProcessGetBlockSizes(RvSrtpProcess *thisPtr, RvBool forRtp, RvSize_t *authSize, RvSize_t *encSize)
{
	RvInt  authAlg;
	RvInt  encAlg;

	if(forRtp == RV_TRUE) {
		authAlg = thisPtr->srtpAuthAlg;
		encAlg = thisPtr->srtpEncAlg;
	} else {
		authAlg = thisPtr->srtcpAuthAlg;
		encAlg = thisPtr->srtcpEncAlg;
	}
	switch(authAlg) {
		case RV_SRTPPROCESS_AUTHENTICATIONTYPE_HMACSHA1: *authSize = rvSrtpAuthenticationGetBlockSize(authAlg);
														 break;
		default: *authSize = 1;
	}
	switch(encAlg) {
		case RV_SRTPPROCESS_ENCYRPTIONTYPE_AESCM: *encSize = rvSrtpAesCmGetBlockSize();
												  break;
		case RV_SRTPPROCESS_ENCYRPTIONTYPE_AESF8: *encSize = rvSrtpAesF8GetBlockSize();
												  break;
		default: *encSize = 1;
	}
}

/*-----------------------------------------------------------------------*/
/*                   external function                                   */
/*-----------------------------------------------------------------------*/

/*************************************************************************
* rvSrtpProcessConstruct
* ------------------------------------------------------------------------
* General: construction - initializing all the RvSrtpProcess parameters
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    thisPtr - pointer to the RvSrtpProcess object
*           dbPtr - pointer to the RvSrtpDb object
*           keyDerivationPtr - pointer to the RvSrtpKeyDerivation object
*
* Output    None
*************************************************************************/
RvRtpStatus rvSrtpProcessConstruct
(
    RvSrtpProcess       *thisPtr,
    RvSrtpDb            *dbPtr,
    RvSrtpKeyDerivation *keyDerivationPtr,
    RvSrtpAesPlugIn* plugin
)
{
    /*fill in the things you get from the DB*/
    thisPtr->dbPtr = dbPtr;

    thisPtr->keyDerivationPtr = keyDerivationPtr;
    thisPtr->prefixLen = 0;
	thisPtr->authTagBuf = NULL;

    thisPtr->srtpAuthAlg = RV_SRTPPROCESS_AUTHENTICATIONTYPE_NONE;
    thisPtr->srtpAuthTagSize = 0;

    thisPtr->srtcpAuthAlg = RV_SRTPPROCESS_AUTHENTICATIONTYPE_NONE;
    thisPtr->srtcpAuthTagSize = 0;

    thisPtr->srtpEncAlg = RV_SRTPPROCESS_ENCYRPTIONTYPE_NULL;
    thisPtr->srtpMki = RV_TRUE;

    thisPtr->srtcpEncAlg = RV_SRTPPROCESS_ENCYRPTIONTYPE_NULL;
    thisPtr->srtcpMki = RV_TRUE;

    thisPtr->AESPlugin = plugin;

    rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessConstruct: construction succeeded"));
    return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
* rvSrtpProcessDestruct
* ------------------------------------------------------------------------
* General: destruction - at this point do nothing
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    thisPtr - pointer to the RvSrtpProcess object
*
* Output    None
*************************************************************************/
RvRtpStatus rvSrtpProcessDestruct
(
    RvSrtpProcess *thisPtr
)
{
	if(thisPtr->authTagBuf != NULL)
		RvMemoryFree(thisPtr->authTagBuf, NULL);
    rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessConstruct: desstruction succeeded"));
    return RV_RTPSTATUS_Succeeded;
}

RvSize_t rvSrtpProcessGetHeaderSize(RvSrtpProcess *thisPtr, RvBool forRtp)
{
    RV_UNUSED_ARG(thisPtr);
    RV_UNUSED_ARG(forRtp);

	return 0; /* No header needed */
}

RvSize_t rvSrtpProcessGetFooterSize(RvSrtpProcess *thisPtr, RvBool forRtp)
{
	RvSize_t result;

	result = 0;
	if(forRtp == RV_TRUE) {
		if(thisPtr->srtpAuthAlg != RV_SRTPPROCESS_AUTHENTICATIONTYPE_NONE)
			result += thisPtr->srtpAuthTagSize;
		if(thisPtr->srtpMki == RV_TRUE)
			result += rvSrtpDbGetMkiSize(thisPtr->dbPtr);
	} else {
		if(thisPtr->srtcpAuthAlg != RV_SRTPPROCESS_AUTHENTICATIONTYPE_NONE)
			result += thisPtr->srtcpAuthTagSize;
		if(thisPtr->srtcpMki == RV_TRUE)
			result += rvSrtpDbGetMkiSize(thisPtr->dbPtr);
		result += RV_SRTP_SRTCP_INDEX_SIZE;
	}

	return result;
}

RvSize_t rvSrtpProcessGetPaddingSize(RvSrtpProcess *thisPtr, RvBool forRtp)
{
	RvSize_t authBlockSize;
	RvSize_t encBlockSize;
	RvSize_t authPadding;
	RvSize_t encPadding;

	/* Get the individual block sizes */
	rvSrtpProcessGetBlockSizes(thisPtr, forRtp, &authBlockSize, &encBlockSize);

	/* Find worst case padding for both combined */
	authPadding = authBlockSize - 1;
	encPadding = encBlockSize - 1;
	while(authPadding != encPadding) {
		if(authPadding < encPadding) {
			authPadding += authBlockSize;
		} else encPadding += encBlockSize;
	}

	return authPadding; /* authPadding = encPadding */
}

RvSize_t rvSrtpProcessGetTransmitSize(RvSrtpProcess *thisPtr, RvUint32 packetSize, RvUint32 rtpHeaderSize, RvBool encrypt, RvBool forRtp)
{
	RvSize_t headerSize;
	RvSize_t footerSize;
	RvSize_t authBlockSize;
	RvSize_t encBlockSize;
	RvSize_t authPadding;
	RvSize_t encPadding;

    RV_UNUSED_ARG(encrypt);
	/* Get base information */
	headerSize = rvSrtpProcessGetHeaderSize(thisPtr, forRtp);
	footerSize = rvSrtpProcessGetFooterSize(thisPtr, forRtp);
	rvSrtpProcessGetBlockSizes(thisPtr, forRtp, &authBlockSize, &encBlockSize);

	if(forRtp == RV_TRUE) {
		/* Determine any padding needed for authentication */
		authPadding = (packetSize + headerSize) % authBlockSize;

		/* Determine any padding needed for encryption */
		/* Note: When the RTP/RTCP stack supports RTP extensions, rtpHeaderSize must include the header extension size too. */
		encPadding = (packetSize - rtpHeaderSize) % encBlockSize;
	} else {
		/* Determine any padding needed for authentication */
		authPadding = (packetSize + headerSize + RV_SRTP_SRTCP_INDEX_SIZE) % authBlockSize; /* include SRTCP index */

		/* Determine any padding needed for encryption */
		encPadding = (packetSize - RV_SRTCP_HEADER_FIXED_LEN) % encBlockSize; /* First header not encrypted */
	}

	/* Get actual padding and packet size */
	if(authPadding != 0)
		authPadding = authBlockSize - authPadding;
	if(encPadding != 0)
		encPadding = encBlockSize - encPadding;

	/* Now figure out padding size that works for both */
	while(authPadding != encPadding) {
		if(authPadding < encPadding) {
			authPadding += authBlockSize;
		} else encPadding += encBlockSize;
	}

	return packetSize + headerSize + footerSize + authPadding; /* authPadding = encpadding */
}

RvRtpStatus rvSrtpProcessSetEncryption
(
    RvSrtpProcess *thisPtr,
    RvInt         rtpType,
    RvInt         rtcpType
)
{
    thisPtr->srtpEncAlg = rtpType;
    thisPtr->srtcpEncAlg = rtcpType;

    rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessSetEncryption: set encryption type rtp : %d, rtcp : %d ",
                    rtpType, rtcpType));
    return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
* rvSrtpProcessSetAuthentication
* ------------------------------------------------------------------------
* General: set the srtp and srtcp authentication information
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    thisPtr - pointer to the RvSrtpProcess object
*           rtpType - srtp authentication algorithm
*           rtpTagSize - srtp authentication tag length
*           rtcpType - srtcp authentication algorithm
*           rtcpTagSize - srtcp authentication tag length
*
* Output    None
*************************************************************************/
RvRtpStatus rvSrtpProcessSetAuthentication
(
    RvSrtpProcess *thisPtr,
    RvInt         rtpType,
    RvSize_t      rtpTagSize,
    RvInt         rtcpType,
    RvSize_t      rtcpTagSize
)
{
	RvSize_t tagBufSize;

    thisPtr->srtpAuthAlg = rtpType;
	thisPtr->srtpAuthTagSize = rtpTagSize;
    thisPtr->srtcpAuthAlg = rtcpType;
	thisPtr->srtcpAuthTagSize = rtcpTagSize;

	/* Insure tag sizes are zero if no authentication being used */
	if(thisPtr->srtpAuthAlg == RV_SRTP_AUTHENTICATION_NONE)
		thisPtr->srtpAuthTagSize = 0;
	if(thisPtr->srtcpAuthAlg == RV_SRTP_AUTHENTICATION_NONE)
		thisPtr->srtcpAuthTagSize = 0;

	/* Create buffer to use for authentication tags (use default memory region for now) */
	tagBufSize = RvMax(thisPtr->srtpAuthTagSize, thisPtr->srtcpAuthTagSize);
	if(thisPtr->authTagBuf != NULL)
		RvMemoryFree(thisPtr->authTagBuf, NULL);
	if(RvMemoryAlloc(NULL, tagBufSize, NULL, (void **)&thisPtr->authTagBuf) != RV_OK) {
		thisPtr->authTagBuf = NULL;
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessSetAuthentication: Can't allocate authentication tag buffer"));
		return RV_RTPSTATUS_OutOfMemory;
	}

    rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessSetAuthentication: set authentication type \
                    rtp : %d, tag size %d, rtcp : %d, tag size %d",
                    rtpType, rtpTagSize, rtcpType, rtcpTagSize));
	return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
* rvSrtpProcessUseMki
* ------------------------------------------------------------------------
* General: sets whether the srtp and the srtcp should include mki in
*          the packet and check for it in the received packet
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    thisPtr - pointer to the RvSrtpProcess object
*           rtpMki - boolean value stating whether srtp should use MKI or not
*           rtcpMki - boolean value stating whether srtcp should use MKI or not
*
* Output    None
*************************************************************************/
RvRtpStatus rvSrtpProcessUseMki
(
    RvSrtpProcess *thisPtr,
    RvBool rtpMki,
    RvBool rtcpMki
)
{
    thisPtr->srtpMki = rtpMki;
    thisPtr->srtcpMki = rtcpMki;

    rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessUseMki: set mki use \
                    rtp : %d, rtcp : %d", rtpMki, rtcpMki));
    return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
* rvSrtpProcessSetPrefixLength
* ------------------------------------------------------------------------
* General: sets the prefix length, by default prefixLength = 0
*
*  +----+   +------------------+---------------------------------+
*  | KG |-->| Keystream Prefix |          Keystream Suffix       |
*  +----+   +------------------+---------------------------------+
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    thisPtr - pointer to the RvSrtpProcess object
*           prefixLength - length of the keystream prefix
*
* Output    None
*************************************************************************/
RvRtpStatus rvSrtpProcessSetPrefixLength
(
    RvSrtpProcess *thisPtr,
    RvSize_t      prefixLength
)
{
    thisPtr->prefixLen = prefixLength;

    rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessSetPrefixLength: set prefix length %d",
                    prefixLength));
    return RV_RTPSTATUS_Succeeded;
}

#ifdef UPDATED_BY_SPIRENT
/*************************************************************************
 * rvSrtpProcessDecrementMasterkeyLifetime
 * ------------------------------------------------------------------------
 * General: this function checks is masterkey's lifetime exhausted or
 * threshold reached
 *
 * Return Value: none
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    dbPtr - pointer to the RvSrtpDb object
 *           masterKey - pointer to the master key
 *           callback - pointer to the callback function
 *           isRTP - RV_TRUE when RTP lifetime decrements, RV_FALSE
 * Output    None
 *************************************************************************/
static inline void rvSrtpProcessDecrementMasterkeyLifetime(RvSrtpKey *masterKey, RvSrtpMasterKeyEventCB masterKeyEvent, void* context, RvBool isRTP)
{
    if (masterKey && masterKey->use_lifetime) {
        RvUint64* lifetime_ptr = isRTP == RV_TRUE ? &masterKey->rtp_lifetime : &masterKey->rtcp_lifetime;
        (*lifetime_ptr)--;
        if (masterKeyEvent && ((*lifetime_ptr == 0) || (*lifetime_ptr == masterKey->threshold))) {
            masterKeyEvent(context, masterKey->mki, masterKey->direction, *lifetime_ptr);
        }
    }
}

#define rvSrtpProcessDecrementMasterkeyLifetimeRTP(masterKey, masterKeyEvent, context) \
    rvSrtpProcessDecrementMasterkeyLifetime(masterKey, masterKeyEvent, context, RV_TRUE)
#define rvSrtpProcessDecrementMasterkeyLifetimeRTCP(masterKey, masterKeyEvent, context) \
    rvSrtpProcessDecrementMasterkeyLifetime(masterKey, masterKeyEvent, context, RV_FALSE)

#endif // UPDATED_BY_SPIRENT

/*************************************************************************
 * rvSrtpProcessEncyrptRtp
 * ------------------------------------------------------------------------
 * General: this function builds the SRTP packet from a given RTP packet
 *
 * Return Value: RvRtpStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    thisPtr - pointer to the RvSrtpProcess object
 *           streamPtr - pointer to the current stream
 *           inputBufPtr - pointer th input packet structure
 *           outputBufPtr - pointer th output packet structure
 *           roc - the roll over count - to calculate from it the srtp index
 *
 * Output    None
 *
 *
 *************************************************************************/
RvRtpStatus rvSrtpProcessEncryptRtp
(
    RvSrtpProcess *thisPtr,
    RvSrtpStream  *streamPtr,
    RvDataBuffer  *inputBufPtr,
    RvDataBuffer  *outputBufPtr,
    RvUint32      roc
#ifdef UPDATED_BY_SPIRENT
    ,RvSrtpMasterKeyEventCB masterKeyEvent
    ,void* context
#endif // UPDATED_BY_SPIRENT
)
{
    RvRtpStatus   rc;
	RvUint64      SRTPIndex;
	RvBool        useCurrentKeys;
	RvUint16      seq;
	RvSrtpContext *destContext;
    RvUint32      ssrc;
    RvUint8       *encKey;
    RvSize_t      encKeySize;
    RvUint8       *authKey;
    RvSize_t      authKeySize;
    RvUint8       *saltKey;
    RvSize_t      saltKeySize;
	RvUint8       *authTag;
    RvUint8       *mki;
    RvSize_t      mkiLen;
    RvSize_t      authPortionSize;
	RvUint32      headerSize;
	RvUint32      payloadLen;

    if ((NULL == thisPtr) || (NULL == streamPtr))
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtp: process or stream are null"));
        return RV_RTPSTATUS_NullPointer;
    }
    if ((NULL == inputBufPtr) || (NULL == rvDataBufferGetData(inputBufPtr)))
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtp: inputBufPtr or data are null"));
        return RV_RTPSTATUS_NullPointer;
    }
	if (NULL == outputBufPtr)
	{
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtp: outputBufPtr is null"));
		return RV_RTPSTATUS_NullPointer;
	}

	/*check whether there is enough space for MKI + authentication tag*/
	rc = rvSrtpProcessFooterSpaceCheck(thisPtr, inputBufPtr, RV_TRUE);
	if (RV_RTPSTATUS_Succeeded != rc)
	{
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtp: not enough space in buffer"));
		return rc;
	}

	/*I can calculate the index now - I got the ROC from the layer above*/
	/*get the RTP sequence number from the packet*/
    rvDataBufferReadAtUint16(inputBufPtr, 2, &seq);
    SRTPIndex = ((RvUint64)(roc) << 16) | (RvUint64)seq;
    rvSrtpLogDebug ((rvSrtpLogSourcePtr, "index is : %d", seq));

	/* Replay list check and set (items older than the replay list show as duplicates) */
	if(rvSrtpDbStreamTestReplayList(streamPtr, SRTPIndex, RV_TRUE) == RV_FALSE)
	{
		rvSrtpLogWarning ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtp: duplicate destination packet"));
		return RV_RTPSTATUS_DuplicatePacket;
	}

    /* Determine which cryptographic context to use */
    destContext = rvSrtpDbContextFind(thisPtr->dbPtr, streamPtr, SRTPIndex);
    if (NULL == destContext)
    {
        rvSrtpLogWarning ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtp: destination has no key set"));
        return RV_RTPSTATUS_NotFound;
    }

    /* Generate the session keys if necessary (including triggers) and determine which keys to use */
    rc = rvSrtpKeyDerivationUpdateContext(
        thisPtr->keyDerivationPtr,
        thisPtr->AESPlugin,
        RvSrtpEncryptionPurpose_KeyDerivation_ENCRYPT_RTP,
        destContext,
        SRTPIndex,
        &useCurrentKeys);

    if (RV_RTPSTATUS_Succeeded != rc)
    {
		rvSrtpLogWarning ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtp: could not get session keys"));
        return rc;
    }

	/* Update the stream's maxIndex (also updates history and replay list to match) */
	if(rvSrtpDbStreamUpdateMaxIndex(streamPtr, SRTPIndex) == RV_FALSE) {
		/* This should never happen because the replay list test should catch everything */
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtp: could not update stream information"));
		return RV_RTPSTATUS_Failed;
	}

	/* Set up data needed for encrytion and authentication */
	if(useCurrentKeys == RV_TRUE) {
		encKey = rvSrtpDbContextGetCurrentEncryptKey(destContext);
		saltKey = rvSrtpDbContextGetCurrentSalt(destContext);
		authKey = rvSrtpDbContextGetCurrentAuthKey(destContext);
	} else {
		encKey = rvSrtpDbContextGetOldEncryptKey(destContext);
		saltKey = rvSrtpDbContextGetOldSalt(destContext);
		authKey = rvSrtpDbContextGetOldAuthKey(destContext);
	}
	encKeySize = rvSrtpDbGetSrtpEncryptKeySize(thisPtr->dbPtr);
	saltKeySize = rvSrtpDbGetSrtpSaltSize(thisPtr->dbPtr);
	authKeySize = rvSrtpDbGetSrtpAuthKeySize(thisPtr->dbPtr);
#ifdef _SRTP_DEBUG_
	rvSrtpLogDebug ((rvSrtpLogSourcePtr, "EncryptRtp: session key:%s", dbgPrint(encKey, encKeySize)));
	rvSrtpLogDebug ((rvSrtpLogSourcePtr, "EncryptRtp: salt key   :%s", dbgPrint(saltKey, saltKeySize)));
	rvSrtpLogDebug ((rvSrtpLogSourcePtr, "EncryptRtp: auth key   :%s", dbgPrint(authKey, authKeySize)));
#endif
	rvDataBufferReadAtUint32(inputBufPtr, 8, &ssrc); /* get SSRC from packet */

	/* Preserve the input buffer by copying it and working only on the output buffer */
	rvSrtpProcessCopyInputToOutput(inputBufPtr, outputBufPtr);

	/* Figure out the start of the payload to be decrypted and its length */
	headerSize = rvSrtpProcessRtpPayloadFigure(inputBufPtr, &payloadLen);

	/* Encrypt the packet */
	rc = rvSrtpProcessRtpCipher(thisPtr, RvSrtpEncryptionPurpose_ENCRYPT_RTP, inputBufPtr, outputBufPtr, headerSize, payloadLen,
								encKey, encKeySize, saltKey, saltKeySize, ssrc, SRTPIndex, roc);
	if (RV_RTPSTATUS_Succeeded != rc)
	{
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtp: encryption failed"));
		return rc;
	}
	rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtp: encryption succeeded"));

	/* Authentication portion of packet doesn't include the MKI */
	/* Calculate authentication portion size before adding MKI */
	authPortionSize = rvDataBufferGetLength(outputBufPtr);

	/* append the MKI to the packet - if MKI is used*/
	if (RV_TRUE == thisPtr->srtpMki)
	{
		mkiLen = rvSrtpDbGetMkiSize(thisPtr->dbPtr);
		mki = rvSrtpDbKeyGetMki(rvSrtpDbContextGetMasterKey(destContext));
		rvDataBufferWriteBackUint8Array(outputBufPtr, mki, mkiLen);
		rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtp: added mki to packet"));
	}

	/* Calculate authentication tag and append it to the packet */
	if (0 != thisPtr->srtpAuthTagSize)
	{
		authTag = rvDataBufferGetData(outputBufPtr) + rvDataBufferGetLength(outputBufPtr);
		rc = rvSrtpAuthenticationCalcRtp(thisPtr->srtpAuthAlg, rvDataBufferGetData(outputBufPtr),
										 authPortionSize, authKey, authKeySize,
										 authTag, thisPtr->srtpAuthTagSize, roc);

		if (RV_RTPSTATUS_Succeeded != rc)
		{
			rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtp: authentication failed"));
			return rc;
		}
		rvDataBufferRewindBack(outputBufPtr, thisPtr->srtpAuthTagSize);
		rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtp: authentication succeeded: tag added"));
	}

    /* Track number of packets encrypted with same master key */
	rvSrtpDbContextIncEncryptCount(destContext);

#ifdef UPDATED_BY_SPIRENT
    rvSrtpProcessDecrementMasterkeyLifetimeRTP(rvSrtpDbContextGetMasterKey(destContext), masterKeyEvent, context);
#endif // UPDATED_BY_SPIRENT

	return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
 * rvSrtpProcessEncyrptRtcp
 * ------------------------------------------------------------------------
 * General: this function builds the SRTP packet from a given RTP packet
 *
 * Return Value: RvRtpStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    thisPtr - pointer to the RvSrtpProcess object
 *           streamPtr - pointer to the current stream
 *           inputBufPtr - pointer th input packet structure
 *           outputBufPtr - pointer th output packet structure
 *           roc - the roll over count - to calculate from it the srtp index
 *
 * Output    None
 *
 *
 *************************************************************************/
RvRtpStatus rvSrtpProcessEncryptRtcp
(
    RvSrtpProcess *thisPtr,
    RvSrtpStream  *streamPtr,
    RvDataBuffer  *inputBufPtr,
    RvDataBuffer  *outputBufPtr,
    RvUint32      index,
    RvBool        encrypt
#ifdef UPDATED_BY_SPIRENT
    ,RvSrtpMasterKeyEventCB masterKeyEvent
    ,void* context
#endif // UPDATED_BY_SPIRENT
)
{
    RvRtpStatus   rc;
	RvBool        useCurrentKeys;
    RvSrtpContext *destContext = NULL;
	RvUint32      ssrc;
    RvUint8       *encKey;
    RvSize_t      encKeySize;
    RvUint8       *authKey;
    RvSize_t      authKeySize;
    RvUint8       *saltKey;
    RvSize_t      saltKeySize;
	RvUint8       *authTag;
    RvUint8       *mki;
    RvSize_t      mkiLen;
    RvSize_t      authPortionSize;
	RvUint32      headerSize;
	RvUint32      payloadLen;

    rvSrtpLogDebug ((rvSrtpLogSourcePtr, "index is : %d", index));

    if ((NULL == thisPtr) || (NULL == streamPtr))
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtcp: process or stream are null"));
        return RV_RTPSTATUS_NullPointer;
    }
	if ((NULL == inputBufPtr) || (NULL == rvDataBufferGetData(inputBufPtr)))
	{
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtcp: inputBufPtr or data are null"));
		return RV_RTPSTATUS_NullPointer;
	}
	if (NULL == outputBufPtr)
	{
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtcp: outputBufPtr is null"));
		return RV_RTPSTATUS_NullPointer;
	}

	/*check whether there is enough space for MKI + authentication tag*/
	rc = rvSrtpProcessFooterSpaceCheck(thisPtr, inputBufPtr, RV_FALSE);
	if (RV_RTPSTATUS_Succeeded != rc)
	{
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtcp: not enough space in buffer"));
		return rc;
	}

	/* Replay list check and set (items older than the replay list show as duplicates) */
	if(rvSrtpDbStreamTestReplayList(streamPtr, index, RV_TRUE) == RV_FALSE)
	{
		rvSrtpLogWarning ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtcp: duplicate destination packet"));
		return RV_RTPSTATUS_DuplicatePacket;
	}

	/* Determine which cryptographic context to use */
	destContext = rvSrtpDbContextFind(thisPtr->dbPtr, streamPtr, index);
	if (NULL == destContext)
	{
		rvSrtpLogWarning ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtcp: destination has no key set"));
		return RV_RTPSTATUS_NotFound;
	}

	/* Generate the session keys if necessary (including triggers) and determine which keys to use */
	rc = rvSrtpKeyDerivationUpdateContext(
        thisPtr->keyDerivationPtr,
        thisPtr->AESPlugin,
        RvSrtpEncryptionPurpose_KeyDerivation_ENCRYPT_RTCP,
        destContext,
        index,
        &useCurrentKeys);
	if (RV_RTPSTATUS_Succeeded != rc)
	{
		rvSrtpLogWarning ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtcp: could not get session keys"));
		return rc;
	}

	/* Update the stream's maxIndex (also updates history and replay list to match) */
	if(rvSrtpDbStreamUpdateMaxIndex(streamPtr, index) == RV_FALSE) {
		/* This should never happen because the replay list test should catch everything */
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtcp: could not update stream information"));
		return RV_RTPSTATUS_Failed;
	}

	/* Set up data needed for encrytion and authentication */
	if(useCurrentKeys == RV_TRUE) {
		encKey = rvSrtpDbContextGetCurrentEncryptKey(destContext);
		saltKey = rvSrtpDbContextGetCurrentSalt(destContext);
		authKey = rvSrtpDbContextGetCurrentAuthKey(destContext);
	} else {
		encKey = rvSrtpDbContextGetOldEncryptKey(destContext);
		saltKey = rvSrtpDbContextGetOldSalt(destContext);
		authKey = rvSrtpDbContextGetOldAuthKey(destContext);
	}
	encKeySize = rvSrtpDbGetSrtpEncryptKeySize(thisPtr->dbPtr);
	saltKeySize = rvSrtpDbGetSrtpSaltSize(thisPtr->dbPtr);
	authKeySize = rvSrtpDbGetSrtpAuthKeySize(thisPtr->dbPtr);
#ifdef _SRTP_DEBUG_
	rvSrtpLogDebug ((rvSrtpLogSourcePtr, "EncryptRtcp: session key:%s", dbgPrint(encKey, encKeySize)));
	rvSrtpLogDebug ((rvSrtpLogSourcePtr, "EncryptRtcp: salt key   :%s", dbgPrint(saltKey, saltKeySize)));
	rvSrtpLogDebug ((rvSrtpLogSourcePtr, "EncryptRtcp: auth key   :%s", dbgPrint(authKey, authKeySize)));
#endif
	rvDataBufferReadAtUint32(inputBufPtr, 4, &ssrc); /* get SSRC from packet */

	/* Preserve the input buffer by copying it and working only on the output buffer */
	rvSrtpProcessCopyInputToOutput(inputBufPtr, outputBufPtr);

	/* Figure out the start of the payload to be decrypted and its length */
	headerSize = rvSrtpProcessRtcpPayloadFigure(inputBufPtr, &payloadLen);

	/* Encrypt the packet */
	if(encrypt == RV_TRUE) {
		rc = rvSrtpProcessRtcpCipher(
            thisPtr,
            RvSrtpEncryptionPurpose_ENCRYPT_RTCP,
            inputBufPtr, outputBufPtr, headerSize, payloadLen,
            encKey, encKeySize, saltKey, saltKeySize, ssrc, (RvUint32)index, encrypt);
		if (RV_RTPSTATUS_Succeeded != rc) {
			rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtcp: encryption failed"));
			return rc;
		}
		rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtcp: encryption succeeded"));
	}

    /* Add the SRTCP index (and the E bit) */
    if(encrypt == RV_TRUE)
        index |= RV_SRTP_SRTCP_E_BIT;
    rvDataBufferWriteBackUint32(outputBufPtr, index);
    rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtcp: added SRTCP index to packet"));

	/* Authentication portion of packet doesn't include the MKI */
	/* Calculate authentication portion size before adding MKI */
	authPortionSize = rvDataBufferGetLength(outputBufPtr);

	/* append the MKI to the packet - if MKI is used*/
	if((encrypt == RV_TRUE) && (RV_TRUE == thisPtr->srtcpMki))
	{
		mkiLen = rvSrtpDbGetMkiSize(thisPtr->dbPtr);
		mki = rvSrtpDbKeyGetMki(rvSrtpDbContextGetMasterKey(destContext));
		rvDataBufferWriteBackUint8Array(outputBufPtr, mki, mkiLen);
		rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtcp: added mki to packet"));
	}

	/* Calculate authentication tag and append it to the packet */
	if (0 != thisPtr->srtcpAuthTagSize && thisPtr->srtcpAuthAlg!= RV_SRTP_AUTHENTICATION_NONE)
	{
		authTag = rvDataBufferGetData(outputBufPtr) + rvDataBufferGetLength(outputBufPtr);
		rc = rvSrtpAuthenticationCalcRtcp(thisPtr->srtcpAuthAlg, rvDataBufferGetData(outputBufPtr),
										  authPortionSize, authKey, authKeySize,
										  authTag, thisPtr->srtcpAuthTagSize);
		if (RV_RTPSTATUS_Succeeded != rc)
		{
			rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtcp: authentication failed"));
			return rc;
		}
		rvDataBufferRewindBack(outputBufPtr, thisPtr->srtcpAuthTagSize);
		rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessEncyrptRtcp: authentication succeeded: tag added"));
	}
	/* Track number of packets encrypted with same master key */
	rvSrtpDbContextIncEncryptCount(destContext);

#ifdef UPDATED_BY_SPIRENT
	rvSrtpProcessDecrementMasterkeyLifetimeRTCP(rvSrtpDbContextGetMasterKey(destContext), masterKeyEvent, context);
#endif // UPDATED_BY_SPIRENT

    return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
 * rvSrtpProcessDecryptRtp
 * ------------------------------------------------------------------------
 * General: this function decrypts the SRTP
 *
 * Return Value: RvRtpStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    thisPtr - pointer to the RvSrtpProcess object
 *           streamPtr - pointer to the current stream
 *           inputBufPtr - pointer th input packet structure
 *           outputBufPtr - pointer th output packet structure
 *           rocPtr - the roll over count -
 *
 * Output    None
 *
 *
 *************************************************************************/
RvRtpStatus rvSrtpProcessDecryptRtp
(
    RvSrtpProcess *thisPtr,
    RvSrtpStream  *streamPtr,
    RvDataBuffer  *inputBufPtr,
    RvDataBuffer  *outputBufPtr,
    RvUint32      *rocPtr
#ifdef UPDATED_BY_SPIRENT
    ,RvSrtpMasterKeyEventCB masterKeyEvent
    ,void* context
#endif // UPDATED_BY_SPIRENT
)
{
    RvRtpStatus   rc;
    RvUint64      SRTPIndex;
	RvBool        useCurrentKeys;
    RvUint16      seq;
    RvSrtpContext *srcContext;
    RvUint32      ssrc;
    RvUint8       *encKey;
    RvSize_t      encKeySize;
    RvUint8       *authKey;
    RvSize_t      authKeySize;
    RvUint8       *saltKey;
    RvSize_t      saltKeySize;
    RvUint8       *packetMki;
    RvUint8       *contextMki;
    RvSize_t      authPortionSize;
    RvUint64      maxIndex;
	RvSrtpKey    *masterKey;
	RvUint8      *packetTag;
	RvUint32      headerSize;
	RvUint32      payloadLen;

	if ((NULL == thisPtr) || (NULL == streamPtr))
    {
        rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecryptRtp: process or stream are null"));
        return RV_RTPSTATUS_NullPointer;
    }
	if ((NULL == inputBufPtr) || (NULL == rvDataBufferGetData(inputBufPtr)))
	{
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecryptRtp: inputBufPtr or data are null"));
		return RV_RTPSTATUS_NullPointer;
	}
	if (NULL == outputBufPtr)
	{
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecryptRtp: outputBufPtr is null"));
		return RV_RTPSTATUS_NullPointer;
	}

    /* get the RTP sequence number from the packet*/
    rvDataBufferReadAtUint16(inputBufPtr, 2, &seq);

    /* Calculate the index of the packet, also get ROC value and new maxIndex */
	rvSrtpProcessSrtpIndexCalc(streamPtr, seq, &SRTPIndex, &maxIndex, rocPtr);

	/* Check replay list (items older than the replay list show as duplicates) */
	if(rvSrtpDbStreamTestReplayList(streamPtr, SRTPIndex, RV_FALSE) == RV_FALSE)
	{
		rvSrtpLogWarning ((rvSrtpLogSourcePtr, "rvSrtpProcessDecyrptRtp: duplicate packet received"));
		return RV_RTPSTATUS_DuplicatePacket;
	}

	/* Determine which cryptographic context to use */
	srcContext = rvSrtpDbContextFind(thisPtr->dbPtr, streamPtr, SRTPIndex);
	if (NULL == srcContext)
	{
		rvSrtpLogWarning ((rvSrtpLogSourcePtr, "rvSrtpProcessDecyrptRtp: source has no key set"));
		return RV_RTPSTATUS_NotFound;
	}

	/* If packet contains an MKI, see if we need to switch keys */
	if (thisPtr->srtpMki == RV_TRUE)
	{
		packetMki = rvSrtpProcessGetMkiFromPacket(thisPtr, inputBufPtr, RV_TRUE);
		contextMki = rvSrtpDbKeyGetMki(rvSrtpDbContextGetMasterKey(srcContext));
		if (0 != memcmp(contextMki, packetMki, rvSrtpDbGetMkiSize(thisPtr->dbPtr)))
		{
			/* different mki in packet - we have add and use a new context */
			masterKey = rvSrtpDbKeyFind(thisPtr->dbPtr, packetMki
#ifdef UPDATED_BY_SPIRENT
										, RV_SRTP_KEY_REMOTE
#endif // UPDATED_BY_SPIRENT
									   );
			if(masterKey == NULL) {
				rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecyrptRtp: packet specified unknown MKI"));
				return RV_RTPSTATUS_NotFound;
			}
			srcContext = rvSrtpDbContextAdd(thisPtr->dbPtr, streamPtr, SRTPIndex, masterKey, RV_FALSE);
			if (srcContext == NULL)
			{
				rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecryptRtp: could not add new context"));
				return RV_RTPSTATUS_Failed;
			}
		}
	}

	/* Generate the session keys if necessary (including triggers) and determine which keys to use */
    rc = rvSrtpKeyDerivationUpdateContext(
        thisPtr->keyDerivationPtr,
        thisPtr->AESPlugin,
        RvSrtpEncryptionPurpose_KeyDerivation_DECRYPT_RTP,
        srcContext,
        SRTPIndex,
        &useCurrentKeys);

	if (RV_RTPSTATUS_Succeeded != rc)
	{
		rvSrtpLogWarning ((rvSrtpLogSourcePtr, "rvSrtpProcessDecyrptRtp: could not get session keys"));
		return rc;
	}

	/* Set up data needed for encryption and authentication */
	if(useCurrentKeys == RV_TRUE) {
		encKey = rvSrtpDbContextGetCurrentEncryptKey(srcContext);
		saltKey = rvSrtpDbContextGetCurrentSalt(srcContext);
		authKey = rvSrtpDbContextGetCurrentAuthKey(srcContext);
	} else {
		encKey = rvSrtpDbContextGetOldEncryptKey(srcContext);
		saltKey = rvSrtpDbContextGetOldSalt(srcContext);
		authKey = rvSrtpDbContextGetOldAuthKey(srcContext);
	}
	encKeySize = rvSrtpDbGetSrtpEncryptKeySize(thisPtr->dbPtr);
	saltKeySize = rvSrtpDbGetSrtpSaltSize(thisPtr->dbPtr);
	authKeySize = rvSrtpDbGetSrtpAuthKeySize(thisPtr->dbPtr);
#ifdef _SRTP_DEBUG_
	rvSrtpLogDebug ((rvSrtpLogSourcePtr, "DecryptRtp: session key:%s", dbgPrint(encKey, encKeySize)));
	rvSrtpLogDebug ((rvSrtpLogSourcePtr, "DecryptRtp: salt key   :%s", dbgPrint(saltKey, saltKeySize)));
	rvSrtpLogDebug ((rvSrtpLogSourcePtr, "DecryptRtp: auth key   :%s", dbgPrint(authKey, authKeySize)));
#endif
	rvDataBufferReadAtUint32(inputBufPtr, 8, &ssrc); /* get SSRC from packet */

	/* Calculate authentication tag and compare with packet's tag */
	if (0 != thisPtr->srtpAuthTagSize)
    {
		authPortionSize = rvDataBufferGetLength(inputBufPtr) - thisPtr->srtpAuthTagSize;
		packetTag = rvDataBufferGetData(inputBufPtr) + authPortionSize;
		if(thisPtr->srtpMki == RV_TRUE)
			authPortionSize -= rvSrtpDbGetMkiSize(thisPtr->dbPtr); /* MKI not included */

		rc = rvSrtpAuthenticationCalcRtp(thisPtr->srtpAuthAlg, rvDataBufferGetData(inputBufPtr),
										 authPortionSize, authKey, authKeySize,
										 thisPtr->authTagBuf, thisPtr->srtpAuthTagSize, *rocPtr);
        if (RV_RTPSTATUS_Succeeded != rc)
        {
			rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecryptRtp: Could not calculate authentication tag"));
            return rc;
        }
		if(memcmp(thisPtr->authTagBuf, packetTag, thisPtr->srtpAuthTagSize) != 0) {
			rvSrtpLogWarning ((rvSrtpLogSourcePtr, "rvSrtpProcessDecryptRtp: packet authentication failed"));
			return RV_RTPSTATUS_Failed;
		}
        rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessDecryptRtp: packet authenticated"));
	}

	/* Now update the replay list */
	if(rvSrtpDbStreamTestReplayList(streamPtr, SRTPIndex, RV_TRUE) == RV_FALSE)
	{
		/* This should never happen since the index hasn't changes since the last call */
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecyrptRtp: replay list error"));
		return RV_RTPSTATUS_Failed;
	}

	/* Update the stream's maxIndex (also updates history and replay list to match) */
	if(rvSrtpDbStreamUpdateMaxIndex(streamPtr, maxIndex) == RV_FALSE) {
		/* This should never happen because the replay list test should catch everything */
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecyrptRtp: could not update stream information"));
		return RV_RTPSTATUS_Failed;
	}

	/* Preserve the input buffer by copying it and working only on the output buffer */
	rvSrtpProcessCopyInputToOutput(inputBufPtr, outputBufPtr);

	/* Remove the MKI and authentication fields from the output buffer (if present) */
	if(thisPtr->srtpAuthTagSize != 0)
		rvDataBufferSkipBack(outputBufPtr, thisPtr->srtpAuthTagSize);
	if (thisPtr->srtpMki == RV_TRUE)
		rvDataBufferSkipBack(outputBufPtr, rvSrtpDbGetMkiSize(thisPtr->dbPtr));

	/* Figure out the start of the payload to be decrypted and its length */
	headerSize = rvSrtpProcessRtpPayloadFigure(outputBufPtr, &payloadLen);

	/* Decrypt the packet */
	rc = rvSrtpProcessRtpCipher(thisPtr, RvSrtpEncryptionPurpose_DECRYPT_RTP, inputBufPtr, outputBufPtr, headerSize, payloadLen,
								encKey, encKeySize, saltKey, saltKeySize, ssrc, SRTPIndex, *rocPtr);
	if (RV_RTPSTATUS_Succeeded != rc)
	{
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecyrptRtp: decryption failed"));
		return rc;
	}
	rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessDecyrptRtp: decryption succeeded"));

#ifdef UPDATED_BY_SPIRENT
	rvSrtpProcessDecrementMasterkeyLifetimeRTP(rvSrtpDbContextGetMasterKey(srcContext), masterKeyEvent, context);
#endif // UPDATED_BY_SPIRENT

    return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpProcessDecryptRtcp
(
    RvSrtpProcess *thisPtr,
    RvSrtpStream  *streamPtr,
    RvDataBuffer  *inputBufPtr,
    RvDataBuffer  *outputBufPtr,
    RvUint32      *indexPtr,
    RvBool        *encryptedPtr
#ifdef UPDATED_BY_SPIRENT
    ,RvSrtpMasterKeyEventCB masterKeyEvent
    ,void* context
#endif // UPDATED_BY_SPIRENT
)
{
    RvRtpStatus   rc;
    RvUint64      index;
	RvBool        useCurrentKeys;
    RvSrtpContext *srcContext;
    RvUint32      ssrc;
    RvUint8       *encKey;
    RvSize_t      encKeySize;
    RvUint8       *authKey;
    RvSize_t      authKeySize;
    RvUint8       *saltKey;
    RvSize_t      saltKeySize;
    RvUint8       *packetMki;
    RvUint8       *contextMki;
    RvSize_t      authPortionSize;
	RvSrtpKey    *masterKey;
	RvUint8      *packetTag;
	RvUint32      headerSize;
	RvUint32      payloadLen;

	if ((NULL == thisPtr) || (NULL == streamPtr))
	{
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecryptRtcp: process or stream are null"));
		return RV_RTPSTATUS_NullPointer;
	}
	if ((NULL == inputBufPtr) || (NULL == rvDataBufferGetData(inputBufPtr)))
	{
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecryptRtcp: inputBufPtr or data are null"));
		return RV_RTPSTATUS_NullPointer;
	}
	if (NULL == outputBufPtr)
	{
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecryptRtcp: outputBufPtr is null"));
		return RV_RTPSTATUS_NullPointer;
	}

	/* get the RTCP index number and encrypt flag from the packet*/
	*indexPtr = rvSrtpProcessGetRtcpIndexFromPacket(thisPtr, inputBufPtr, encryptedPtr);
	index = (RvUint64)*indexPtr;

	/* Check replay list (items older than the replay list show as duplicates) */
	if(rvSrtpDbStreamTestReplayList(streamPtr, index, RV_FALSE) == RV_FALSE)
	{
		rvSrtpLogWarning ((rvSrtpLogSourcePtr, "rvSrtpProcessDecyrptRtcp: duplicate packet received"));
		return RV_RTPSTATUS_DuplicatePacket;
	}

	/* Determine which cryptographic context to use */
	srcContext = rvSrtpDbContextFind(thisPtr->dbPtr, streamPtr, index);
	if (NULL == srcContext)
	{
		rvSrtpLogWarning ((rvSrtpLogSourcePtr, "rvSrtpProcessDecyrptRtcp: source has no key set"));
		return RV_RTPSTATUS_NotFound;
	}

	/* If packet contains an MKI, see if we need to switch keys */
	if (thisPtr->srtcpMki == RV_TRUE)
	{
		packetMki = rvSrtpProcessGetMkiFromPacket(thisPtr, inputBufPtr, RV_FALSE);
		contextMki = rvSrtpDbKeyGetMki(rvSrtpDbContextGetMasterKey(srcContext));
		if (0 != memcmp(contextMki, packetMki, rvSrtpDbGetMkiSize(thisPtr->dbPtr)))
		{
			/* different mki in packet - we have add and use a new context */
			masterKey = rvSrtpDbKeyFind(thisPtr->dbPtr, packetMki
#ifdef UPDATED_BY_SPIRENT
										, RV_SRTP_KEY_REMOTE
#endif // UPDATED_BY_SPIRENT
									   );
			if(masterKey == NULL) {
				rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecyrptRtcp: packet specified unknown MKI"));
				return RV_RTPSTATUS_NotFound;
			}
			srcContext = rvSrtpDbContextAdd(thisPtr->dbPtr, streamPtr, index, masterKey, RV_FALSE);
			if (srcContext == NULL)
			{
				rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecryptRtcp: could not add new context"));
				return RV_RTPSTATUS_Failed;
			}
		}
	}

	/* Generate the session keys if necessary (including triggers) and determine which keys to use */
    rc = rvSrtpKeyDerivationUpdateContext(
        thisPtr->keyDerivationPtr,
        thisPtr->AESPlugin,
        RvSrtpEncryptionPurpose_KeyDerivation_DECRYPT_RTCP,
        srcContext,
        index,
        &useCurrentKeys);

	if (RV_RTPSTATUS_Succeeded != rc)
	{
		rvSrtpLogWarning ((rvSrtpLogSourcePtr, "rvSrtpProcessDecyrptRtcp: could not get session keys"));
		return rc;
	}

	/* Set up data needed for encrytion and authentication */
	if(useCurrentKeys == RV_TRUE) {
		encKey = rvSrtpDbContextGetCurrentEncryptKey(srcContext);
		saltKey = rvSrtpDbContextGetCurrentSalt(srcContext);
		authKey = rvSrtpDbContextGetCurrentAuthKey(srcContext);
	} else {
		encKey = rvSrtpDbContextGetOldEncryptKey(srcContext);
		saltKey = rvSrtpDbContextGetOldSalt(srcContext);
		authKey = rvSrtpDbContextGetOldAuthKey(srcContext);
	}
	encKeySize = rvSrtpDbGetSrtpEncryptKeySize(thisPtr->dbPtr);
	saltKeySize = rvSrtpDbGetSrtpSaltSize(thisPtr->dbPtr);
	authKeySize = rvSrtpDbGetSrtpAuthKeySize(thisPtr->dbPtr);
#ifdef _SRTP_DEBUG_
	rvSrtpLogDebug ((rvSrtpLogSourcePtr, "DecryptRtcp: session key:%s", dbgPrint(encKey, encKeySize)));
	rvSrtpLogDebug ((rvSrtpLogSourcePtr, "DecryptRtcp: salt key   :%s", dbgPrint(saltKey, saltKeySize)));
	rvSrtpLogDebug ((rvSrtpLogSourcePtr, "DecryptRtcp: auth key   :%s", dbgPrint(authKey, authKeySize)));
#endif
	rvDataBufferReadAtUint32(inputBufPtr, 4, &ssrc); /* get SSRC from packet */

	/* Calculate authentication tag and compare with packet's tag */
	if (0 != thisPtr->srtcpAuthTagSize && thisPtr->srtcpAuthAlg!=RV_SRTP_AUTHENTICATION_NONE)
	{
		authPortionSize = rvDataBufferGetLength(inputBufPtr) - thisPtr->srtcpAuthTagSize;
		packetTag = rvDataBufferGetData(inputBufPtr) + authPortionSize;
		if(thisPtr->srtcpMki == RV_TRUE)
			authPortionSize -= rvSrtpDbGetMkiSize(thisPtr->dbPtr); /* MKI not included */

		rc = rvSrtpAuthenticationCalcRtcp(thisPtr->srtcpAuthAlg, rvDataBufferGetData(inputBufPtr),
										 authPortionSize, authKey, authKeySize,
										 thisPtr->authTagBuf, thisPtr->srtcpAuthTagSize);
		if (RV_RTPSTATUS_Succeeded != rc)
		{
			rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecryptRtcp: Could not calculate authentication tag"));
			return rc;
		}
		if(memcmp(thisPtr->authTagBuf, packetTag, thisPtr->srtcpAuthTagSize) != 0) {
			rvSrtpLogWarning ((rvSrtpLogSourcePtr, "rvSrtpProcessDecryptRtcp: packet authentication failed"));
			return RV_RTPSTATUS_Failed;
		}
		rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessDecryptRtcp: packet authenticated"));
	}

	/* Now update the replay list */
	if(rvSrtpDbStreamTestReplayList(streamPtr, index, RV_TRUE) == RV_FALSE)
	{
		/* This should never happen since the index hasn't changes since the last call */
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecyrptRtcp: replay list error"));
		return RV_RTPSTATUS_Failed;
	}

	/* Update the stream's maxIndex (also updates history and replay list to match) */
	if(rvSrtpDbStreamUpdateMaxIndex(streamPtr, index) == RV_FALSE) {
		/* This should never happen because the replay list test should catch everything */
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecyrptRtcp: could not update stream information"));
		return RV_RTPSTATUS_Failed;
	}

	/* Preserve the input buffer by copying it and working only on the output buffer */
	rvSrtpProcessCopyInputToOutput(inputBufPtr, outputBufPtr);

	/* Remove the MKI, authentication tag, and RTCP index from the output buffer (if present) */
	if(thisPtr->srtcpAuthTagSize != 0)
		rvDataBufferSkipBack(outputBufPtr, thisPtr->srtcpAuthTagSize);
	if (thisPtr->srtcpMki == RV_TRUE)
		rvDataBufferSkipBack(outputBufPtr, rvSrtpDbGetMkiSize(thisPtr->dbPtr));
	rvDataBufferSkipBack(outputBufPtr, 4); /* RTCP index */

	/* Figure out the start of the payload to be decrypted and its length */
	headerSize = rvSrtpProcessRtcpPayloadFigure(outputBufPtr, &payloadLen);

	/* Decrypt the packet */
	rc = rvSrtpProcessRtcpCipher(
        thisPtr,
        RvSrtpEncryptionPurpose_DECRYPT_RTCP, inputBufPtr, outputBufPtr, headerSize, payloadLen,
		encKey, encKeySize, saltKey, saltKeySize, ssrc, *indexPtr, *encryptedPtr);
	if (RV_RTPSTATUS_Succeeded != rc)
	{
		rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpProcessDecyrptRtcp: decryption failed"));
		return rc;
	}
	rvSrtpLogDebug ((rvSrtpLogSourcePtr, "rvSrtpProcessDecyrptRtcp: decryption succeeded"));

#ifdef UPDATED_BY_SPIRENT
	rvSrtpProcessDecrementMasterkeyLifetimeRTCP(rvSrtpDbContextGetMasterKey(srcContext), masterKeyEvent, context);
#endif // UPDATED_BY_SPIRENT

    return RV_RTPSTATUS_Succeeded;
}
