/***********************************************************************
Filename   : rvsrtpaesf8.c
Description: rvsrtpaesf8
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

#include "rverror.h"
#include "rvstdio.h" /* memset on linux */
#include "rvsrtpaesf8.h"
#include "rvbuffer.h"
#include "rvnet2host.h"
#include "rvnet2host-e.h"

/*-----------------------------------------------------------------------*/
/*                   Definitions & Constants                             */
/*-----------------------------------------------------------------------*/
#define AES_F8_IV_SIZE             RvUint32Const(16) /*in bytes*/

#define RV_SRTP_SSRC_SIZE          RvUint8Const(4)
#define RV_SRTP_ROLL_OVER_CNT_SIZE RvUint8Const(4)
#define RV_SRTP_SRTP_INDEX_SIZE    RvUint8Const(6) /*48 bit*/
#define RV_SRTP_SRTCP_INDEX_SIZE   RvUint8Const(4) /*31 bit*/
#define RV_SRTP_SRTCP_INDEX_MASK   RvUint32Const(0x7FFFFFFF)

#define SET_BITS(val,shift,msk)    (((val) << (shift)) & (msk))
#define RTP_HDR_MBIT_BITS          RvUint8Const(0x80)
#define RTP_HDR_MBIT_SHIFT         RvUint8Const(7)
#define RTP_HDR_PT_BITS            RvUint8Const(0x7F)
#define RTP_HDR_PT_SHIFT           RvUint8Const(0)

#define BIT_2_BYTE(bitNum)        ((bitNum) >> 3)
#define BYTE_2_BIT(bitNum)        ((bitNum) << 3)

/*-----------------------------------------------------------------------*/
/*                   type definitions and structures                     */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                   internal functions                                  */
/*-----------------------------------------------------------------------*/

/*************************************************************************
* rvSrtpAesF8CreateRtpIv
* ------------------------------------------------------------------------
* General: builds AES-F8's initial initialization vector for SRTP packets
*          this vector is used afterwards to generate IV' (described bellow)
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* description: the IV is generated as follow:
*
* IV = 0x00 || M || PT || SEQ || TS || SSRC || ROC
* M, PT, SEQ, TS, SSRC will be taken from the RTP header.
*
* Arguments:
* Input:    mBit - The marker bit
*           packetType - The packet type
*           seqNum - The sequence number
*           timestamp - The timestamp
*           ssrc - The ssrc
*           roc - The roll over counter (ROC)
* Output    iv - Buffer to store the initialization vector
*************************************************************************/
static RvRtpStatus rvSrtpAesF8CreateRtpIv
(
    RvUint8  *iv,
    RvBool   mBit,
    RvUint8  packetType,
    RvUint16 seqNum,
    RvUint32 timestamp,
    RvUint32 ssrc,
    RvUint32 roc
)
{
    RvUint8 ivPosition = 1;
    RvUint8 tempByte = 0;

    if (NULL == iv)
    {
        return RV_RTPSTATUS_NullPointer;
    }

    memset(iv, 0, AES_F8_IV_SIZE);

    /*put the M bit and the payload type in one byte*/
    if (RV_TRUE == mBit)
    {
        tempByte = 0x1;
    }
    else
    {
        tempByte = 0x0;
    }
    tempByte = (RvUint8)(SET_BITS(packetType, RTP_HDR_PT_SHIFT ,RTP_HDR_PT_BITS) |
                         SET_BITS(tempByte, RTP_HDR_MBIT_SHIFT, RTP_HDR_MBIT_BITS));
    memcpy(iv + ivPosition, &tempByte, 1);
    ivPosition += 1;

    seqNum = RvConvertHostToNetwork16(seqNum);
    memcpy(iv + ivPosition, &seqNum, 2);
    ivPosition += 2;
    timestamp = RvConvertHostToNetwork32(timestamp);
    memcpy(iv + ivPosition, &timestamp, 4);
    ivPosition += 4;
    ssrc = RvConvertHostToNetwork32(ssrc);
    memcpy(iv + ivPosition, &ssrc, RV_SRTP_SSRC_SIZE);
    ivPosition += RV_SRTP_SSRC_SIZE;
    roc = RvConvertHostToNetwork32(roc);
    memcpy(iv + ivPosition, &roc, RV_SRTP_ROLL_OVER_CNT_SIZE);
    ivPosition += RV_SRTP_ROLL_OVER_CNT_SIZE;

    return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
* rvSrtpAesF8CreateRtcpIv
* ------------------------------------------------------------------------
* General: builds AES-F8's initial initialization vector for SRTCP packets
*          this vector is used afterwards to generate IV' (described bellow)
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* description: the IV is generated as follow:
*
* IV= 0..0 || E || SRTCP index || V || P || RC || PT || length || SSRC
* V, P, RC, PT, length, SSRC SHALL be taken from the first header
* in the RTCP compound packet.
*
* Arguments:
* Input:    eBit - The encryption bit
*           index - The index
*           version - The version number
*           padBit - The padding bit
*           count - The reception report count
*           packetType - The packet type
*           length - The packet length
*           ssrc - The ssrc
* Output    iv - Buffer to store the initialization vector
*************************************************************************/
static RvRtpStatus rvSrtpAesF8CreateRtcpIv
(
    RvUint8  *iv,
    RvBool   eBit,
    RvUint32 index,
    RvUint8  version,
    RvBool   padBit,
    RvUint8  count,
    RvUint8  packetType,
    RvUint16 length,
    RvUint32 ssrc
)
{
    RvUint8  ivPosition = 4, tempByte = 0;
    RvUint32 tempLong = 0;

    if (NULL == iv)
    {
        return RV_RTPSTATUS_NullPointer;
    }

    memset(iv, 0, AES_F8_IV_SIZE);

    /*put the SRTCPIndex and the encryptionBit in one byte*/
    if (RV_TRUE == eBit)
    {
        tempLong = index|0x80000000;
    }
    else
    {
        tempLong = index&0x7FFFFFFF;
    }
    tempLong = RvConvertHostToNetwork32(tempLong);
    memcpy((iv + ivPosition), &tempLong, RV_SRTP_SRTCP_INDEX_SIZE);
    ivPosition += RV_SRTP_SRTCP_INDEX_SIZE;

    /*put the version, padding bit, and the reception report count in one byte*/
    if (RV_TRUE == padBit)
    {
        tempByte = 0x1;
    }
    else
    {
        tempByte = 0x0;
    }
    tempByte = (RvUint8)(SET_BITS(count, 0 ,0x1F) | SET_BITS(tempByte, 5, 0x20) | SET_BITS(version, 6, 0xC0));
    memcpy(iv + ivPosition, &tempByte, 1);
    ivPosition += 1;

    memcpy(iv + ivPosition, &packetType, 1);
    ivPosition += 1;
    length = RvConvertHostToNetwork16(length);
    memcpy(iv + ivPosition, &length, 2);
    ivPosition += 2;
    ssrc = RvConvertHostToNetwork32(ssrc);
    memcpy(iv + ivPosition, &ssrc, RV_SRTP_SSRC_SIZE);

    return RV_RTPSTATUS_Succeeded;
}


/*************************************************************************
* rvSrtpAesCmCreateIvTag
* ------------------------------------------------------------------------
* General: once given the IV, F8 builds an IV' which with it, the AES-F8
*          will be called afterwards. IV' is generated using the AES
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* description: the IV' is generated as follow:
*
* IV' = E(k_e XOR m, IV)
* and m = k_s || 0x555..5
*
* Arguments:
* Input:    aesInfo - AES parameters
*           iv - Buffer containing the initialization vector
*           blockCipherSize - the AES block cipher size in bytes
*           key - the key to encrypt with
*           keySize - key size
*           salt - the salt to encrypt with
*           saltSize - salt size
*
* Output    ivTag - The resulting initialization vector (IV')
*************************************************************************/
static RvRtpStatus rvSrtpAesCmCreateIvTag
(
 RvSrtpAesPlugIn*           plugin,
 RvSrtpEncryptionPurpose    purpose,
 RvUint8         *          iv,
 RvUint8         *          key,
 RvSize_t                   keySize,
 RvUint8         *          salt,
 RvSize_t                   saltSize,
 RvUint8         *          ivTag
 )
{
    RvRtpStatus rc;
    RvSize_t        cnt;
    RvUint8         m[AES_F8_IV_SIZE];

    if ((NULL == iv) || (NULL == key) || (NULL == salt) || (NULL == ivTag))
    {
        return RV_RTPSTATUS_NullPointer;
    }

    /*generate m*/
    saltSize = ((saltSize < keySize) ? saltSize : keySize);
    memcpy(m, salt, saltSize);
    for (cnt = saltSize; cnt < keySize; cnt++)
    {
        m[cnt] = 0x55;
    }

    /*XOR key with m*/
    RvBufferXor(m, key, m, keySize);

    plugin->construct(plugin, purpose);
    rc = (plugin->initECBMode(plugin, purpose, RVSRTP_AESPLUGIN_DIRECTION_ENCRYPT,
                                           m, (RvUint16)BYTE_2_BIT(keySize),
                                           BYTE_2_BIT(AES_F8_IV_SIZE)) ==
                                           RV_TRUE ? RV_RTPSTATUS_Succeeded : RV_RTPSTATUS_Failed);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
        return rc;
    }

    plugin->encrypt(plugin, purpose, iv, ivTag, AES_F8_IV_SIZE);
    plugin->destruct(plugin, purpose);

    return RV_RTPSTATUS_Succeeded;
}

/*-----------------------------------------------------------------------*/
/*                   external functions                                  */
/*-----------------------------------------------------------------------*/

/*************************************************************************
* rvSrtpAesF8ProcessRtp
* ------------------------------------------------------------------------
* General: AES in F8 mode for srtp packets. this function builds a 128
*          bit keystream and immediately xors it with the input buffer
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* description: the keystream is generated as follow:
*
* keystream = S(0) ||... || S(L-1)
* S(j) = E(k_e, IV' XOR j XOR S(j-1))
* S(-1) = 00..0.
* j = 0,1,..,L-1 where L = N/n_b
*
* AES in the CBC mode automatically xors S(j-1) with the input
* so I have to xor only j with the input
*
* Arguments:
* Input:    plugin   - pointer to the AES plugin
*           purpose  - purpose of the AES plugin
*           inputBuf - pointer to the data to encrypt
*           bufSize - data length in bytes
*           key - the key to encrypt with
*           keySize - key size
*           salt - the salt to encrypt with
*           saltSize - salt size
*           mBit - The marker bit
*           packetType - The packet type
*           seqNum - The sequence number
*           timestamp - The timestamp
*           ssrc - The ssrc
*           roc - The roll over counter (ROC)
*
* Output    *outputBuf - encrypted portion
*************************************************************************/
RvRtpStatus rvSrtpAesF8ProcessRtp
(
 RvSrtpAesPlugIn*            plugin,
 RvSrtpEncryptionPurpose    purpose,
 RvUint8  *                 inputBuf,
 RvUint8  *                 outputBuf,
 RvSize_t                   bufSize,
 RvUint8  *                 key,
 RvSize_t                   keySize,
 RvUint8  *                 salt,
 RvSize_t                   saltSize,
 RvBool                     mBit,
 RvUint8                    packetType,
 RvUint16                   seqNum,
 RvUint32                   timestamp,
 RvUint32                   ssrc,
 RvUint32                   roc
)
{
    RvRtpStatus rc;
    RvUint32        remainder;
    RvUint32        rounds;
    RvUint16        counter;
    RvUint8         *keyStreamPtr;
    RvUint8         prevBuf[AES_F8_IV_SIZE];
    RvUint8         tempBuf[AES_F8_IV_SIZE];
    RvUint8         IVTag[AES_F8_IV_SIZE];
    RvUint8         keyStreamBuf[AES_F8_IV_SIZE];
    RvSrtpEncryptionPurpose    IVbuildPurpose;

 

    if ((NULL == inputBuf) || (NULL == outputBuf) || (NULL == key) || (NULL == salt))
    {
        return RV_RTPSTATUS_NullPointer;
    }

 

    /*create the initialization vector for the aes-f8*/
    rc = rvSrtpAesF8CreateRtpIv(tempBuf, mBit, packetType, seqNum, timestamp, ssrc, roc);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
        return rc;
    }

 

    /*build iv'*/
    switch (purpose)
    {
    default:    
    case RvSrtpEncryptionPurpose_ENCRYPT_RTP:
        IVbuildPurpose = RvSrtpEncryptionPurpose_IV_BUILDING_ENCRYPT_RTP;
        break;
    case RvSrtpEncryptionPurpose_DECRYPT_RTP:
        IVbuildPurpose = RvSrtpEncryptionPurpose_IV_BUILDING_DECRYPT_RTP;
        break;
    }
    rc = rvSrtpAesCmCreateIvTag(plugin, IVbuildPurpose, tempBuf, key, keySize, salt, saltSize, IVTag);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
        return rc;
    }

 

    /*process*/
    plugin->construct(plugin, purpose);
    memset(tempBuf, 0, AES_F8_IV_SIZE);
    rc = (
        plugin->initECBMode(plugin, purpose, RVSRTP_AESPLUGIN_DIRECTION_ENCRYPT,
                                           key, (RvUint16)BYTE_2_BIT(keySize),
                                           BYTE_2_BIT(AES_F8_IV_SIZE)) ==
                                           RV_TRUE ? RV_RTPSTATUS_Succeeded : RV_RTPSTATUS_Failed);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
        return rc;
    }

 

    rounds = bufSize / AES_F8_IV_SIZE; /*rounded down*/
    remainder = bufSize % AES_F8_IV_SIZE;

 

    keyStreamPtr = outputBuf;
    /*write the output straight to the bitstream buffer
      in case the length is not a multiple of block cipher size
      another round will be done at the end and only the amount of remainder*/

 

    memset(prevBuf, 0, AES_F8_IV_SIZE);  /* this function is fixed */
    for (counter = 0; counter < rounds; counter++)
    {
        /*S(j) = E(k_e, IV' XOR j XOR S(j-1))*/
        RvBufferXor(prevBuf, IVTag, tempBuf, AES_F8_IV_SIZE);
        counter = RvConvertHostToNetwork16(counter);
        *(RvUint16 *)(tempBuf + 14) ^= counter;
        counter = RvConvertNetworkToHost16(counter);
        plugin->encrypt(plugin, purpose, tempBuf, keyStreamPtr, AES_F8_IV_SIZE);
        /*store the previous output*/
        memcpy(prevBuf, keyStreamPtr, AES_F8_IV_SIZE);

 

        /*xor it with the input during building the keystream*/
        RvBufferXor(keyStreamPtr, inputBuf, keyStreamPtr, AES_F8_IV_SIZE);

 

        keyStreamPtr += AES_F8_IV_SIZE;
        inputBuf += AES_F8_IV_SIZE;
    }

 

    /*do another iteration in case there is a remainder*/
    if (0 < remainder)
    {
        RvBufferXor(prevBuf, IVTag, tempBuf, AES_F8_IV_SIZE);
        counter = RvConvertHostToNetwork16(counter);
        *(RvUint16 *)(tempBuf + 14) ^= counter;
        plugin->encrypt(plugin, purpose, tempBuf, keyStreamBuf, AES_F8_IV_SIZE);
        /*append the remainder to the output buf*/
        RvBufferXor(keyStreamBuf, inputBuf, keyStreamPtr, remainder);
    }
    plugin->destruct(plugin, purpose);
    return RV_RTPSTATUS_Succeeded;
}


/*************************************************************************
* rvSrtpAesF8ProcessRtcp
* ------------------------------------------------------------------------
* General: AES in F8 mode for srtcp packets. this function builds a 128
*          bit keystream and immediately xors it with the input buffer
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* description: the keystream is generated as follow:
*
* keystream = S(0) ||... || S(L-1)
* S(j) = E(k_e, IV' XOR j XOR S(j-1))
* S(-1) = 00..0.
* j = 0,1,..,L-1 where L = N/n_b
*
* AES in the CBC mode automatically xors S(j-1) with the input
* so I have to xor only j with the input
*
* Arguments:
* Input:    inputBuf - pointer to the data to encrypt
*           bufSize - data length in bytes
*           key - the key to encrypt with
*           keySize - key size
*           salt - the salt to encrypt with
*           saltSize - salt size
*           eBit - The encryption bit
*           index - The index
*           version - The version number
*           padBit - The padding bit
*           count - The reception report count
*           packetType - The packet type
*           length - The packet length
*           ssrc - The ssrc
*
* Output    *outputBuf - encrypted portion
*************************************************************************/
RvRtpStatus rvSrtpAesF8ProcessRtcp
(
    RvSrtpAesPlugIn*            plugin,
    RvSrtpEncryptionPurpose     purpose,
    RvUint8  *                  inputBuf,
    RvUint8  *                  outputBuf,
    RvSize_t                    bufSize,
    RvUint8  *                  key,
    RvSize_t                    keySize,
    RvUint8  *                  salt,
    RvSize_t                    saltSize,
    RvBool                      eBit,
    RvUint32                    index,
    RvUint8                     version,
    RvBool                      padBit,
    RvUint8                     count,
    RvUint8                     packetType,
    RvUint16                    length,
    RvUint32                    ssrc
)
{
    RvRtpStatus                 rc;
    RvUint32                    remainder;
    RvUint32                    rounds;
    RvUint16                    counter;
    RvUint8                     *keyStreamPtr;
    RvUint8                     prevBuf[AES_F8_IV_SIZE];
    RvUint8                     tempBuf[AES_F8_IV_SIZE];
    RvUint8                     IVTag[AES_F8_IV_SIZE];
    RvUint8                     keyStreamBuf[AES_F8_IV_SIZE];
    RvSrtpEncryptionPurpose     IVbuildPurpose;

    if ((NULL == inputBuf) || (NULL == outputBuf) || (NULL == key) || (NULL == salt))
    {
        return RV_RTPSTATUS_NullPointer;
    }

    /*create the initialization vector for the aes-f8*/
    rc = rvSrtpAesF8CreateRtcpIv(tempBuf, eBit, index, version, padBit, count,
                                 packetType, length, ssrc);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
        return rc;
    }

    /*build iv'*/
    switch (purpose)
    {
    default:
    case RvSrtpEncryptionPurpose_ENCRYPT_RTCP:
        IVbuildPurpose = RvSrtpEncryptionPurpose_IV_BUILDING_ENCRYPT_RTCP;
        break;
    case RvSrtpEncryptionPurpose_DECRYPT_RTCP:
        IVbuildPurpose = RvSrtpEncryptionPurpose_IV_BUILDING_DECRYPT_RTCP;
        break;
    }
    rc = rvSrtpAesCmCreateIvTag(plugin, IVbuildPurpose, tempBuf, key, keySize, salt, saltSize, IVTag);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
        return rc;
    }

    /*process*/
    plugin->construct(plugin, purpose);
    memset(tempBuf, 0, AES_F8_IV_SIZE);
    rc = (plugin->initECBMode(plugin, purpose, RVSRTP_AESPLUGIN_DIRECTION_ENCRYPT,
                                           key, (RvUint16)BYTE_2_BIT(keySize),
                                           BYTE_2_BIT(AES_F8_IV_SIZE)) ==
                                           RV_TRUE ? RV_RTPSTATUS_Succeeded : RV_RTPSTATUS_Failed);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
        return rc;
    }

    rounds = bufSize / AES_F8_IV_SIZE; /*rounded down*/
    remainder = bufSize % AES_F8_IV_SIZE;

    keyStreamPtr = outputBuf;
    /*write the output straight to the bitstream buffer
      in case the length is not a multiple of block cipher size
      another round will be done at the end and only the amount of remainder*/

    memset(prevBuf, 0, AES_F8_IV_SIZE);
    for (counter = 0; counter < rounds; counter++)
    {
        /*S(j) = E(k_e, IV' XOR j XOR S(j-1))*/
        RvBufferXor(prevBuf, IVTag, tempBuf, AES_F8_IV_SIZE);
        counter = RvConvertHostToNetwork16(counter);
        *(RvUint16 *)(tempBuf + 14) ^= counter;
        counter = RvConvertNetworkToHost16(counter);
        plugin->encrypt(plugin, purpose, tempBuf, keyStreamPtr, AES_F8_IV_SIZE);
        /*store the previous output*/
        memcpy(prevBuf, keyStreamPtr, AES_F8_IV_SIZE);

        /*xor it with the input during building the keystream*/
        RvBufferXor(keyStreamPtr, inputBuf, keyStreamPtr, AES_F8_IV_SIZE);

        keyStreamPtr += AES_F8_IV_SIZE;
        inputBuf += AES_F8_IV_SIZE;
    }

    /*do another iteration in case there is a remainder*/
    if (0 < remainder)
    {
        RvBufferXor(prevBuf, IVTag, tempBuf, AES_F8_IV_SIZE);
        counter = RvConvertHostToNetwork16(counter);
        *(RvUint16 *)(tempBuf + 14) ^= counter;
        plugin->encrypt(plugin, purpose, tempBuf, keyStreamBuf, AES_F8_IV_SIZE);

        /*append the remainder to the output buf*/
        RvBufferXor(keyStreamBuf, inputBuf, keyStreamPtr, remainder);
    }
    plugin->destruct(plugin, purpose);
    return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
* rvSrtpAesF8GetBlockSize
* ------------------------------------------------------------------------
* General: This function get the block size of the AES-F8 algorithm.
*          Some algorithms require the input and output buffer to be a
*          multiple of some blocksize. in AES case the answer will be 1
*          i.e. buffer sizes must be a multiple of one byte.
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    None
*
* Output    None
*************************************************************************/
RvSize_t rvSrtpAesF8GetBlockSize()
{
    return 1;
}


#if defined(RV_TEST_CODE)
RvUint8 saltKeyF8[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                       0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
RvUint16 saltKeyF8_len = 112;

RvUint8 encKeyF8[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
RvUint16 encKeyF8_len = 128;

RvUint8 inputBufF8[51]  = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                           0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
                           0x98, 0x16, 0xee, 0x74, 0x00, 0xf8, 0x7f, 0x55,
                           0x6b, 0x2c, 0x04, 0x9c, 0x8e, 0x5a, 0xd0, 0x36,
                           0x47, 0x43, 0x87, 0x35, 0xa4, 0x1c, 0x65, 0xb9,
                           0xe0, 0x16, 0xba, 0xf4, 0xae, 0xbf, 0x7a, 0xd2,
                           0x69, 0xc4, 0xe0};
RvUint16 inputBufF8_len = 51;

RvUint8 outputF8[51];
RvUint8 inputF8[51];

void RvSrtpAesF8Test()
{
    RvRtpStatus     rc;
    RvUint32        ssrc = 0x98765432;
    RvUint32        roc =  0x12345678;
    RvUint32        index =  0x43216789;

    /*test SRTP*/
    RvBool   mBit = RV_TRUE;
    RvUint8  packetType = 0x78;
    RvUint16 seqNum = 0x8765;
    RvUint32 timestamp = 0x56781234;

    /*SRTCP parameters*/
    RvBool   eBit = RV_TRUE;
    RvUint8  version = 0x1;
    RvBool   padBit = RV_FALSE;
    RvUint8  count = 6;
    RvUint16 length = 45;

    rc = rvSrtpAesF8ProcessRtp(inputBufF8, outputF8, inputBufF8_len, encKeyF8, BIT_2_BYTE(encKeyF8_len),
                               saltKeyF8, BIT_2_BYTE(saltKeyF8_len), mBit, packetType, seqNum,
                               timestamp, ssrc, roc);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
    }

    rc = rvSrtpAesF8ProcessRtp(outputF8, inputF8, inputBufF8_len, encKeyF8, BIT_2_BYTE(encKeyF8_len),
                               saltKeyF8, BIT_2_BYTE(saltKeyF8_len), mBit, packetType, seqNum,
                               timestamp, ssrc, roc);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
    }

    rc = memcmp(inputF8, inputBufF8, inputBufF8_len);

    /*test SRTCP*/

    rc = rvSrtpAesF8ProcessRtcp(inputBufF8, outputF8, inputBufF8_len, encKeyF8, BIT_2_BYTE(encKeyF8_len),
                                saltKeyF8, BIT_2_BYTE(saltKeyF8_len), eBit, index, version,
                                padBit, count, packetType, length, ssrc);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
    }

    rc = rvSrtpAesF8ProcessRtcp(outputF8, inputF8, inputBufF8_len, encKeyF8, BIT_2_BYTE(encKeyF8_len),
                                saltKeyF8, BIT_2_BYTE(saltKeyF8_len), eBit, index, version,
                                padBit, count, packetType, length, ssrc);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
    }

    rc = memcmp(inputF8, inputBufF8, inputBufF8_len);


}
#endif /* RV_TEST_CODE */

