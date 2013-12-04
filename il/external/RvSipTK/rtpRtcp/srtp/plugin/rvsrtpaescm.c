/***********************************************************************
Filename   : rvsrtpaescm.c
Description: rvsrtpaescm
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
#include "rvsrtpaescm.h"
#include "rvbuffer.h"
#include <math.h>
#include "rvstdio.h" /* memset on linux */
#include "rvnet2host.h"
#include "rvnet2host-e.h"

/*-----------------------------------------------------------------------*/
/*                   Definitions & Constants                             */
/*-----------------------------------------------------------------------*/
#define RV_SRTP_PACKET            RvUint8Const(0)
#define RV_SRTCP_PACKET           RvUint8Const(1)

#define RV_AES_CM_IV_SIZE         RvUint32Const(16) /*in bytes*/

/*key definitions*/
#define RV_KEY_DER_BLOCK_SIZE     RvUint32Const(16) /*in bytes*/
#define RV_KEY_DER_MAX_VAL        (EXP_2(24))
#define RV_KEY_MAX_SIZE           RvIntConst(256) /*bits*/

#define RV_NUM_OF_SESSION_KEYS    RvIntConst(3)
#define IS_SRTP_KEY(key)          ((key) / RV_NUM_OF_SESSION_KEYS)

#define RV_SRTP_SSRC_SIZE         RvUint8Const(4)
#define RV_SRTP_SRTP_INDEX_SIZE   RvUint8Const(6) /*48 bit*/
#define RV_SRTP_SRTCP_INDEX_SIZE  RvUint8Const(4) /*31 bit*/
#define RV_SRTP_SRTCP_INDEX_MASK  RvUint32Const(0x7FFFFFFF)

#define BIT_2_BYTE(bitNum)        ((bitNum) >> 3)
#define BYTE_2_BIT(bitNum)        ((bitNum) << 3)
#define EXP_2(exp)                ((RvUint64)(1 << (exp)))

/*-----------------------------------------------------------------------*/
/*                   internal functions                                  */
/*-----------------------------------------------------------------------*/

/*************************************************************************
* rvSrtpAesCmCreateRtpIv
* ------------------------------------------------------------------------
* General: builds AES-CM's initialization vector for SRTP packets
*          the output is a 128 bit vector
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* description: the IV is generated as follow:
*
* IV = (k_s * 2^16) XOR (SSRC * 2^64) XOR (i * 2^16)
* construction of the iv is done according to network order
*
* Arguments:
* Input:    blockCipherSize - iv size in bytes,
*           saltKey - session salt key
*           saltKeyLen - salt key size in bytes
*           SSRC - the ssrc of this packet
*           index - index of this packet
*
* Output    iv - the initialization vector
*************************************************************************/
static RvRtpStatus rvSrtpAesCmCreateRtpIv
(
    RvUint8  *iv,
    RvUint8  *saltKey,
    RvSize_t saltKeyLen,
    RvUint32 ssrc,
    RvUint64 index
)
{
    RvUint8 tempBuff[RV_AES_CM_IV_SIZE];
    RvUint8 keyOffset; /*if salt key is larger than 14 only least significant bits
                         will be coppied*/
    RvUint8 bufOffset; /*in case salt key is smaller than 14 */

    if ((NULL == iv) || (NULL == saltKey))
    {
        return RV_RTPSTATUS_NullPointer;
    }

    memset(iv, 0, RV_AES_CM_IV_SIZE);

    /*salt key is coppied such that the least significant bit
      leave the 2 lower bytes for the counter*/
    keyOffset = (RvUint8) ((saltKeyLen > 14) ? (saltKeyLen - 14) : 0);
    bufOffset = (RvUint8) ((saltKeyLen < 14) ? (14 - saltKeyLen) : 0);
    saltKeyLen = ((saltKeyLen < 14) ? saltKeyLen : 14);
    memcpy((iv + bufOffset), (saltKey + keyOffset), saltKeyLen);

    memset(tempBuff, 0, RV_AES_CM_IV_SIZE);
    ssrc = RvConvertHostToNetwork32(ssrc);
    memcpy(tempBuff + 4, &ssrc, RV_SRTP_SSRC_SIZE);

    /*XOR them*/
    RvBufferXor(iv, tempBuff, iv, RV_AES_CM_IV_SIZE);

    memset(tempBuff, 0, RV_AES_CM_IV_SIZE);
    {
        RvUint16 msb = RvUint64ToRvUint16(RvUint64ShiftRight(index, 32));
        RvUint32 lsb = RvUint64ToRvUint32(index);
        
        msb = RvConvertHostToNetwork16(msb);
        lsb = RvConvertHostToNetwork32(lsb);
        
        memcpy(tempBuff + 8, &msb, 2);
        memcpy(tempBuff + 10, &lsb, 4);
    }
    /*XOR them*/
    RvBufferXor(iv, tempBuff, iv, RV_AES_CM_IV_SIZE);

    return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
* rvSrtpAesCmCreateRtcpIv
* ------------------------------------------------------------------------
* General: the same as rvSrtpAesCmCreateRtpIv for SRTCP
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* description: the IV is generated as follow:
*
* IV = (k_s * 2^16) XOR (SSRC * 2^64) XOR (i * 2^16)
*
* Arguments:
* Input:    blockCipherSize - iv size in bytes,
*           saltKey - session salt key
*           saltKeyLen - salt key size in bytes
*           SSRC - the ssrc of this packet
*           index - index of this packet
*
* Output    iv - the initialization vector
*************************************************************************/
static RvRtpStatus rvSrtpAesCmCreateRtcpIv
(
    RvUint8  *iv,
    RvUint8  *saltKey,
    RvSize_t saltKeyLen,
    RvUint32 ssrc,
    RvUint32 index
)
{
    RvUint8 tempBuff[RV_AES_CM_IV_SIZE];
    RvUint8 keyOffset; /*if salt key is larger than 14 only least significant bits
                         will be coppied*/
    RvUint8 bufOffset; /*in case salt key is smaller than 14 */

    if ((NULL == iv) || (NULL == saltKey))
    {
        return RV_RTPSTATUS_NullPointer;
    }

    memset(iv, 0, RV_AES_CM_IV_SIZE);

    /*salt key is coppied such that the least significant bit
      leave the 2 lower bytes for the counter*/
    keyOffset = (RvUint8) ((saltKeyLen > 14) ? (saltKeyLen - 14) : 0);
    bufOffset = (RvUint8) ((saltKeyLen < 14) ? (14 - saltKeyLen) : 0);
    saltKeyLen = ((saltKeyLen < 14) ? saltKeyLen : 14);
    memcpy((iv + bufOffset), (saltKey + keyOffset), saltKeyLen);

    memset(tempBuff, 0, RV_AES_CM_IV_SIZE);
    ssrc = RvConvertHostToNetwork32(ssrc);
    memcpy(tempBuff + 4, &ssrc, RV_SRTP_SSRC_SIZE);

    /*XOR them*/
    RvBufferXor(iv, tempBuff, iv, RV_AES_CM_IV_SIZE);

    memset(tempBuff, 0, RV_AES_CM_IV_SIZE);
    index = RvConvertHostToNetwork32(index);
    memcpy(tempBuff + 10, &index, RV_SRTP_SRTCP_INDEX_SIZE);

    /*XOR them*/
    RvBufferXor(iv, tempBuff, iv, RV_AES_CM_IV_SIZE);

    return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
* rvSrtpAesCmDerivationKeyIVBuild
* ------------------------------------------------------------------------
* General: builds the initialization vector for generating a session key
*          all session keys are generated using this function, depending
*          on the input parameters
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* description: the key is generated as follow:
*
* r = index DIV key_derivation_rate
* key_id = <label> || r.
* x = key_id XOR master_salt,
* IV = x * 2^16
*
* example :
* index DIV kdr:                 000000000000
* label:                       00
* master salt:   0EC675AD498AFEEBB6960B3AABE6
* -----------------------------------------------
* xor:           0EC675AD498AFEEBB6960B3AABE6     (x, PRF input)
*
* x*2^16:        0EC675AD498AFEEBB6960B3AABE60000 (AES-CM input)
*
* Arguments:
* Input:    keyDerivationRate - key derivation rate
*           index - SRTP or SRTCP packet index
*           *masterSalt - the structure holding master salt information
*           keyType - which key to generate
*
* Output    keyMat - the initialization vector for generating the specific key
*************************************************************************/
static RvRtpStatus rvSrtpAesCmDerivationKeyIVBuild
(
    RvSize_t keyDerivationRate,
    RvUint64 index,
    RvUint8  *masterSalt,
    RvSize_t masterSaltSize,
    RvUint   keyType,
    RvUint8  *keyMat /*output*/
)
{
    RvUint8  indexLen;
    RvUint8  keyId[RV_KEY_DER_BLOCK_SIZE];
    RvUint8 keyOffset; /*if salt key is larger than 14 only least significant bits
                         will be coppied*/
    RvUint8 bufOffset; /*in case salt key is smaller than 14 */

    if ((NULL == masterSalt) || (NULL == keyMat))
    {
        return RV_RTPSTATUS_NullPointer;
    }

    if (RV_SRTCP_PACKET == IS_SRTP_KEY(keyType))
    {
        indexLen = RV_SRTP_SRTCP_INDEX_SIZE;
    }
    else /*RV_SRTP_PACKET*/
    {
        indexLen = RV_SRTP_SRTP_INDEX_SIZE;
    }

    /*1. r = index DIV key_derivation_rate - DIV by key_derivation_rate is done by
      shifting index by log2 of key_derivation_rate
      the shifted index is already put in place*/
    /*check if keyDerivationRate  is with in limits*/
    if (RV_KEY_DER_MAX_VAL < keyDerivationRate)
    {
        return RV_RTPSTATUS_BadParameter;
    }
    if (0 != keyDerivationRate)
    {
		index = RvUint64Div(index, RvUint64FromRvSize_t(keyDerivationRate));
    }
    else
    {
        index = RvUint64Const2(0);
    }

    /*2. key_id = <label> || r*/
    /*input the label*/
    memset(keyId, 0, RV_KEY_DER_BLOCK_SIZE);
    {
        RvUint16 msb = RvUint64ToRvUint16(RvUint64ShiftRight(index, 32));
        RvUint32 lsb = RvUint64ToRvUint32(index);
        
        msb = RvConvertHostToNetwork16(msb);
        lsb = RvConvertHostToNetwork32(lsb);        
        
        /*index = RvConvertHostToNetwork64(index);*/
        
        /*subtracting 2 for the shift 16 at the end*/
        
        //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
        if(indexLen == RV_SRTP_SRTP_INDEX_SIZE) {
           memcpy(&keyId[RV_KEY_DER_BLOCK_SIZE -2 - indexLen],     &msb, 2);
           memcpy(&keyId[RV_KEY_DER_BLOCK_SIZE -2 - indexLen + 2],     &lsb, 4);
        } else if(indexLen == RV_SRTP_SRTCP_INDEX_SIZE) {
           //msb is 0 anyway:
           memcpy(&keyId[RV_KEY_DER_BLOCK_SIZE -2 - indexLen],     &lsb, 4);
        } else {
           return RV_RTPSTATUS_BadParameter;
        }
#else
        memcpy(&keyId[RV_KEY_DER_BLOCK_SIZE -2 - indexLen],     &msb, 2);
        memcpy(&keyId[RV_KEY_DER_BLOCK_SIZE -2 - indexLen + 4], &lsb, 4);
#endif
        //SPIRENT_END
    }
    keyId[RV_KEY_DER_BLOCK_SIZE -2 - indexLen - 1] = (RvUint8)keyType;
   
    /* -2 is because RFC:
      1) ... Let x = key_id XOR master_salt, where key_id and master_salt are
      aligned so that their least significant bits agree (right-
      alignment).
      2)     The currently defined PRF, keyed by 128, 192, or 256 bit master key,
         has input block size m = 128 and can produce n-bit outputs for n up
         to 2^23.  PRF_n(k_master,x) SHALL be AES in Counter Mode as described
         in Section 4.1.1, applied to key k_master, and IV equal to (x*2^16)!!!!!!,
         and with the output key stream truncated to the n first (left-most)
         bits.  (Requiring n/128, rounded up, applications of AES.) */

    /*check if keyDerivationRate  is with in limits*/
    if (RV_KEY_MAX_SIZE < BYTE_2_BIT(masterSaltSize))
    {
        return RV_RTPSTATUS_BadParameter;
    }
    memset(keyMat, 0, RV_KEY_DER_BLOCK_SIZE);
    /*master salt is coppied to the beginning of the array
      master salt size is 14 shifted by 16 bits -> beginning of teh array*/
    keyOffset = (RvUint8) ((masterSaltSize > 14) ? (masterSaltSize - 14) : 0);
    bufOffset = (RvUint8) ((masterSaltSize < 14) ? (14 - masterSaltSize) : 0);
    masterSaltSize = ((masterSaltSize < 14) ? masterSaltSize : 14);
    memcpy((keyMat + bufOffset), (masterSalt + keyOffset), masterSaltSize);

    RvBufferXor(keyId, keyMat, keyMat, RV_KEY_DER_BLOCK_SIZE);

    return RV_RTPSTATUS_Succeeded;
}

/*-----------------------------------------------------------------------*/
/*                   external functions                                  */
/*-----------------------------------------------------------------------*/

/*************************************************************************
* rvSrtpAesCmProcessRtp
* ------------------------------------------------------------------------
* General: this function does the actual encryption / decryption for srtp
*          packets
*          it implements the AES algorithm in counter mode
*          and generates a 128 bit keystream, then it immediately XORs it
*          with the input buffer
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    aesInfo - all aes algorithm information
*           iv - initialization vector created above
*           blockCipherSize - block cipher size for aes algorithm in bytes
*           key - the key for the the aes algorithm
*           key - the key for the the aes algorithm
*           keySize - key length in bytes
*           inputBuf - pointer to the encrypted portion
*           bufLen - the result buffer length in bytes
*
* Output    outputBuf - the encrypted portion
*************************************************************************/
RvRtpStatus rvSrtpAesCmProcessRtp
(
    RvSrtpAesPlugIn* plugin,
    RvSrtpEncryptionPurpose purpose,
    RvUint8  *inputBuf,
    RvUint8  *outputBuf,
    RvSize_t bufSize,
    RvUint8  *key,
    RvSize_t keySize,
    RvUint8  *salt,
    RvSize_t saltSize,
    RvUint32 ssrc,
    RvUint64 index
)
{
    RvRtpStatus rc;

    RvUint32        rounds;
    RvUint32        remainder;
    RvUint16        counter;
    RvUint8         *keyStreamPtr;
    RvUint8         iv[RV_AES_CM_IV_SIZE];
    RvUint8         keyStreamBuf[RV_AES_CM_IV_SIZE];

    if ((NULL == inputBuf) || (NULL == outputBuf) || (NULL == key) || (NULL == salt))
    {
        return RV_RTPSTATUS_NullPointer;
    }

    /*build the input for creating the key*/
    rc = rvSrtpAesCmCreateRtpIv(iv, salt, saltSize, ssrc, index);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
        return rc;
    }

    /*now call the aes-cm*/
    plugin->construct(plugin, purpose);
    rc = (plugin->initECBMode(plugin, purpose, RVSRTP_AESPLUGIN_DIRECTION_ENCRYPT,
                                           key, (RvUint16)BYTE_2_BIT(keySize),
                                           BYTE_2_BIT(RV_AES_CM_IV_SIZE)) ==
                                           RV_TRUE ? RV_RTPSTATUS_Succeeded : RV_RTPSTATUS_BadParameter);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
        return rc;
    }

    rounds = bufSize / RV_AES_CM_IV_SIZE;
    remainder = bufSize % RV_AES_CM_IV_SIZE;

    keyStreamPtr = outputBuf;
    /*write the output straight to the bitstream buffer
      in case the length is not a multiple of block cipher size
      another round will be done at the end and only the amount of remainder*/
    for (counter = 0; counter < rounds; counter++)
    {
        counter = RvConvertHostToNetwork16(counter);
        memcpy(iv + 14, &counter, 2);
        counter = RvConvertNetworkToHost16(counter);
        plugin->encrypt(plugin, purpose, iv, keyStreamPtr, RV_AES_CM_IV_SIZE);
        /*xor it with the input during building the keystream*/
        RvBufferXor(keyStreamPtr, inputBuf, keyStreamPtr, RV_AES_CM_IV_SIZE);

        keyStreamPtr += RV_AES_CM_IV_SIZE;
        inputBuf += RV_AES_CM_IV_SIZE;
    }

    /*do another iteration in case there is a remainder*/
    if (0 < remainder)
    {
        counter = RvConvertHostToNetwork16(counter);
        memcpy(iv + 14, &counter, 2);
        plugin->encrypt(plugin, purpose, iv, keyStreamBuf, RV_AES_CM_IV_SIZE);

        /*append the remainder to the output buf*/
        RvBufferXor(keyStreamBuf, inputBuf, keyStreamPtr, remainder);
    }

    plugin->destruct(plugin, purpose);
    return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
* rvSrtpAesCmProcessRtcp
* ------------------------------------------------------------------------
* General: this function does the actual encryption / decryption for srtcp
*          packets
*          it implements the AES algorithm in counter mode
*          and generates a 128 bit keystream, then it immediately XORs it
*          with the input buffer
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    aesInfo - all aes algorithm information
*           iv - initialization vector created above
*           blockCipherSize - block cipher size for aes algorithm in bytes
*           key - the key for the the aes algorithm
*           key - the key for the the aes algorithm
*           keySize - key length in bytes
*           inputBuf - pointer to the encrypted portion
*           bufLen - the result buffer length in bytes
*
* Output    outputBuf - the encrypted portion
*************************************************************************/
RvRtpStatus rvSrtpAesCmProcessRtcp
(
    RvSrtpAesPlugIn* plugin,
    RvSrtpEncryptionPurpose purpose,
    RvUint8  *inputBuf,
    RvUint8  *outputBuf,
    RvSize_t bufSize,
    RvUint8  *key,
    RvSize_t keySize,
    RvUint8  *salt,
    RvSize_t saltSize,
    RvUint32 ssrc,
    RvUint32 index
)
{
    RvRtpStatus rc;

    RvUint32        rounds;
    RvUint32        remainder;
    RvUint16        counter;
    RvUint8         *keyStreamPtr;
    RvUint8         iv[RV_AES_CM_IV_SIZE];
    RvUint8         keyStreamBuf[RV_AES_CM_IV_SIZE];

    if ((NULL == inputBuf) || (NULL == outputBuf) || (NULL == key) || (NULL == salt))
    {
        return RV_RTPSTATUS_NullPointer;
    }

    /*build the input for creating the key*/
    rc = rvSrtpAesCmCreateRtcpIv(iv, salt, saltSize, ssrc, index);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
        return rc;
    }

    /*now call the aes-cm*/
    plugin->construct(plugin, purpose);
    rc = (
        plugin->initECBMode(plugin, purpose, RVSRTP_AESPLUGIN_DIRECTION_ENCRYPT,
                                           key, (RvUint16)BYTE_2_BIT(keySize), BYTE_2_BIT(RV_AES_CM_IV_SIZE)) ==
                                           RV_TRUE ? RV_RTPSTATUS_Succeeded : RV_RTPSTATUS_BadParameter);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
        return rc;
    }

    rounds = bufSize / RV_AES_CM_IV_SIZE;
    remainder = bufSize % RV_AES_CM_IV_SIZE;

    keyStreamPtr = outputBuf;
    /*write the output straight to the bitstream buffer
      in case the length is not a multiple of block cipher size
      another round will be done at the end and only the amount of remainder*/
    for (counter = 0; counter < rounds; counter++)
    {
        counter = RvConvertHostToNetwork16(counter);
        memcpy(iv + 14, &counter, 2);
        counter = RvConvertNetworkToHost16(counter);
        plugin->encrypt(plugin, purpose, iv, keyStreamPtr, RV_AES_CM_IV_SIZE);
        /*xor it with the input during building the keystream*/
        RvBufferXor(keyStreamPtr, inputBuf, keyStreamPtr, RV_AES_CM_IV_SIZE);

        keyStreamPtr += RV_AES_CM_IV_SIZE;
        inputBuf += RV_AES_CM_IV_SIZE;
    }

    /*do another iteration in case there is a remainder*/
    if (0 < remainder)
    {
        counter = RvConvertHostToNetwork16(counter);
        memcpy(iv + 14, &counter, 2);
        plugin->encrypt(plugin, purpose, iv, keyStreamBuf, RV_AES_CM_IV_SIZE);

        /*append the remainder to the output buf*/
        RvBufferXor(keyStreamBuf, inputBuf, keyStreamPtr, remainder);
    }

    plugin->destruct(plugin, purpose);
    return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
* rvSrtpAesCmCreateKey
* ------------------------------------------------------------------------
* General: this function generates one session key, to generate each key
*          basically it creates the initialization vector and calls the
*          AES-CM
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    keyType - which key to generate
*           keySize - the length of the key to create
*           index - SRTP or SRTCP packet index
*           *masterKey - for creating the key ,a parameter to the AES-CM
*           masterKeySize - the length of the master key
*           *masterSalt - for creating the IV
*           masterSaltSize - the length of the salt key
*           keyDerivationRate - key derivation rate ,for the iv
*           index - the srtp /srtcp index ,for the iv
*
*
* Output    *key - the key itsef
*************************************************************************/
RvRtpStatus rvSrtpAesCmCreateKey
(
    RvSrtpAesPlugIn* plugin,
    RvSrtpEncryptionPurpose purpose,
    RvUint   keyType,
    RvUint8  *key,
    RvSize_t keySize,
    RvUint8  *masterKey,
    RvSize_t masterKeySize,
    RvUint8  *masterSalt,
    RvSize_t masterSaltSize,
    RvSize_t keyDerivationRate,
    RvUint64 index
)
{
    RvRtpStatus     rc;
    RvUint32        rounds;
    RvUint32        remainder;
    RvUint16        counter = 0;
    RvUint8         *keyStreamPtr;
    RvUint8         remainderBuf[RV_KEY_DER_BLOCK_SIZE];
    RvUint8         keyMat[RV_KEY_DER_BLOCK_SIZE];

    if ((NULL == key) || (NULL == masterKey) || (NULL == masterSalt))
    {
        return RV_RTPSTATUS_NullPointer;
    }

    /*build the input for creating the key*/
    rc = rvSrtpAesCmDerivationKeyIVBuild(keyDerivationRate, index, masterSalt,
                                       masterSaltSize, keyType, keyMat);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
        return rc;
    }

    /*now create the key using the aes-cm*/
    plugin->construct(plugin, purpose);
    rc = (plugin->initECBMode(plugin, purpose, RVSRTP_AESPLUGIN_DIRECTION_ENCRYPT,
                                           masterKey, (RvUint16)BYTE_2_BIT(masterKeySize),
                                           BYTE_2_BIT(RV_KEY_DER_BLOCK_SIZE)) ==
                                           RV_TRUE ? RV_RTPSTATUS_Succeeded : RV_RTPSTATUS_BadParameter);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
        return rc;
    }

    rounds = keySize / RV_KEY_DER_BLOCK_SIZE;
    remainder = keySize % RV_KEY_DER_BLOCK_SIZE;

    keyStreamPtr = key;

    /*write the output straight to the bitstream buffer
      in case the length is not a multiple of block cipher size
      another round will be done at the end and only the amount of remainder*/
    for (; counter < rounds; counter++)
    {
        counter = RvConvertHostToNetwork16(counter);
        memcpy(keyMat + 14, &counter, 2);
        counter = RvConvertNetworkToHost16(counter);
        plugin->encrypt(plugin, purpose, keyMat, keyStreamPtr, RV_KEY_DER_BLOCK_SIZE);
        keyStreamPtr += RV_KEY_DER_BLOCK_SIZE;
    }

    /*do another iteration in case there is a remainder*/
    if (0 < remainder)
    {
        counter = RvConvertHostToNetwork16(counter);
        memcpy(keyMat + 14, &counter, 2);
        plugin->encrypt(plugin, purpose, keyMat, remainderBuf, RV_KEY_DER_BLOCK_SIZE);

        /*append the remainder to the output buf*/
        memcpy(keyStreamPtr, remainderBuf, remainder);
    }

    plugin->destruct(plugin, purpose);
    return RV_RTPSTATUS_Succeeded;
}

/*************************************************************************
* rvSrtpAesCmGetBlockSize
* ------------------------------------------------------------------------
* General: This function get the block size of the AES-CM algorithm.
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
RvSize_t rvSrtpAesCmGetBlockSize()
{
    return 1;
}

#if defined(RV_TEST_CODE)
RvUint8 masterKey[16] = {0xE1, 0xF9, 0x7A, 0x0D, 0x3E, 0x01, 0x8B, 0xE0,
                         0xD6, 0x4F, 0xA3, 0x2C, 0x06, 0xDE, 0x41, 0x39};
RvUint16 masterKey_len = 128;


RvUint8 masterSalt[16] = {0x0E, 0xC6, 0x75, 0xAD, 0x49, 0x8A, 0xFE, 0xEB,
                          0xB6, 0x96, 0x0B, 0x3A, 0xAB, 0xE6};
RvUint16 saltKey_len = 112;

RvUint8 encKey[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
RvUint16 encKey_len = 128;

RvUint8 inputBuf[51]  = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                         0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
                         0x98, 0x16, 0xee, 0x74, 0x00, 0xf8, 0x7f, 0x55,
                         0x6b, 0x2c, 0x04, 0x9c, 0x8e, 0x5a, 0xd0, 0x36,
                         0x47, 0x43, 0x87, 0x35, 0xa4, 0x1c, 0x65, 0xb9,
                         0xe0, 0x16, 0xba, 0xf4, 0xae, 0xbf, 0x7a, 0xd2,
                         0x69, 0xc4, 0xe0};
RvUint16 inputBuf_len = 51;

RvUint16 cmIv_len = 128;


RvUint8 output[51];
RvUint8 input[51];

void rvSrtpAesCmTest()
{
    RvRtpStatus     rc;
    RvUint32        ssrc = 0x98765432;

    /*test SRTP*/
    RvUint64        index = 0x123456789abc;
    RvUint8         indexLen = 6;

    rvSrtpAesCmCreateKey(1, output, 20, masterKey, 16, masterSalt, 14, 0, 0);
#if 0
    rc = rvSrtpAesCmProcessRtp(inputBuf, output, inputBuf_len, encKey, BIT_2_BYTE(encKey_len),
                               saltKey, BIT_2_BYTE(saltKey_len), ssrc, index);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
    }

    rc = rvSrtpAesCmProcessRtp(output, input, inputBuf_len, encKey, BIT_2_BYTE(encKey_len),
                               saltKey, BIT_2_BYTE(saltKey_len), ssrc, index);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
    }

    rc = memcmp(input, inputBuf, inputBuf_len);

    /*test SRTCP*/
    index = 0x123456789 & RV_SRTP_SRTCP_INDEX_MASK;

    rc = rvSrtpAesCmProcessRtcp(inputBuf, output, inputBuf_len, encKey, BIT_2_BYTE(encKey_len),
                                saltKey, BIT_2_BYTE(saltKey_len), ssrc, (RvUint32)index);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
    }

    rc = rvSrtpAesCmProcessRtcp(output, input, inputBuf_len, encKey, BIT_2_BYTE(encKey_len),
                                saltKey, BIT_2_BYTE(saltKey_len), ssrc, (RvUint32)index);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
    }

    rc = memcmp(input, inputBuf, inputBuf_len);
#endif
}

#endif /* RV_TEST_CODE */






















