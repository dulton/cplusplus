/************************************************************************
 File Name	   : rvsrtpaesplugin.h
 Description   : This plugin must be implemented for the low interface of AES encryption.
                 It does not cover building of initialization vector  functions,            
                 but only the encrypting functions.
                 For more information see the implementation of RvSrtpAesPlugIn in
                 files rtptSrtpaesencryption.h and rtptAesencryption.h in directory
                 appl\rtpRtcp\testapp\rtpt\commonext. This implementation uses 
                 Open SSL library.
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

#if !defined(RV_SRTP_AES_PLUGIN_H)
#define RV_SRTP_AES_PLUGIN_H

#include "rvtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define     RVSRTP_AESPLUGIN_DIRECTION_ENCRYPT ((RvUint8)0) /* Encrypt buffer */
#define     RVSRTP_AESPLUGIN_DIRECTION_DECRYPT ((RvUint8)1) /* Decrypt buffer */

/*************************************************************************
 * RvSrtpEncryptionPurpose
 *   This enumeration type allows to identify encrypting AesEncryption data context
 *   according to the applied purposes in SRTP session. 
 *   It needed in order to avoid memory collisions
 *   in different AES encryption contexts for the same session.
 *************************************************************************/
typedef enum _RvSrtpAesEncryptionPurpose
{
   RvSrtpEncryptionPurpose_ENCRYPT_RTP      = 0,        /* RTP encryption in mode AESCM */
   RvSrtpEncryptionPurpose_ENCRYPT_RTCP,                /* RTCP encryption in mode AESCM */
   RvSrtpEncryptionPurpose_DECRYPT_RTP,                 /* RTP encryption in mode AESCM */
   RvSrtpEncryptionPurpose_DECRYPT_RTCP,                /* RTCP encryption in mode AESCM */
   RvSrtpEncryptionPurpose_IV_BUILDING_ENCRYPT_RTP,     /* AES is used for building IV for encrypting RTP */
   RvSrtpEncryptionPurpose_IV_BUILDING_ENCRYPT_RTCP,    /* AES is used for building IV for encrypting RTCP */
   RvSrtpEncryptionPurpose_IV_BUILDING_DECRYPT_RTP,     /* AES is used for building IV for decrypting RTP */
   RvSrtpEncryptionPurpose_IV_BUILDING_DECRYPT_RTCP,    /* AES is used for building IV for decrypting RTCP */
   RvSrtpEncryptionPurpose_KeyDerivation_ENCRYPT_RTP,  /* KeyDerivation in mode AESCM when encryptiong RTP */
   RvSrtpEncryptionPurpose_KeyDerivation_ENCRYPT_RTCP, /* KeyDerivation in mode AESCM when encryptiong RTCP */
   RvSrtpEncryptionPurpose_KeyDerivation_DECRYPT_RTP,  /* KeyDerivation in mode AESCM when decryptiong RTP  */
   RvSrtpEncryptionPurpose_KeyDerivation_DECRYPT_RTCP, /* KeyDerivation in mode AESCM when decryptiong RTCP */
   RvSrtpEncryptionPurposes,

} RvSrtpEncryptionPurpose;

/********************************************************************
 *	RvSrtpAesPlugIn
 *   This plugin must implement the low interface for AES encryption.
 *   It does not cover building of initialization vector  functions,
 *   but only the encrypting functions.
 *   For more information see the implementation of RvSrtpAesPlugIn in
 *   files rtptSrtpaesencryption.h and rtptAesencryption.h in directory
 *   appl\rtpRtcp\testapp\rtpt\commonext. This implementation uses 
 *   Open SSL library.
 ********************************************************************/
 /* forward declaration */
struct RvSrtpAesPlugIn_;

/**************************************************************************
 * RvSrtpAesPlugInEncryptDataConstructCB
 * description: This callback is called when the user is needs to
 *              construct data (encryption configuration) related to the encryption.
 * parameters:
 *   thisPtr - A pointer to the RvSrtpAesPlugIn_ object.
 *   purpose - purpose of the data
 ***************************************************************************/
typedef void (*RvSrtpAesPlugInEncryptDataConstructCB)(
    struct RvSrtpAesPlugIn_ *thisPtr,
    RvSrtpEncryptionPurpose purpose);

/**************************************************************************
 * RvSrtpAesPlugInEncryptDataDestructCB
 * description: This callback is called when the user is needs to
 *              destruct data (encryption configuration) related to the encryption.
 * parameters:
 *   thisPtr - A pointer to the RvSrtpAesPlugIn_ object.
 *   purpose - purpose of the data
 ***************************************************************************/
typedef void (*RvSrtpAesPlugInEncryptDataDestructCB)(
    struct RvSrtpAesPlugIn_ *thisPtr,
    RvSrtpEncryptionPurpose purpose);

/**************************************************************************
 * RvSrtpAesPlugInEncryptCB
 * description: This callback is called when the user is needs to
 *              encrypt an outgoing data packet.}
 * parameters:
 *   thisPtr  - A pointer to the RvSrtpAesPlugIn_ object.
 *   purpose  - An encrypting purpose 
 * srcBuffer  - A pointer to the plain text buffer.
 * dstBuffer  - A pointer to the cipher text buffer.
 * byteLength - Length of the plain text buffer.
 * dataPtr    - Data related to AES encryption.
 * key        - The key to use for encryption.
 ***************************************************************************/
typedef void (*RvSrtpAesPlugInEncryptCB)(
    struct RvSrtpAesPlugIn_ *thisPtr,
    RvSrtpEncryptionPurpose purpose,
    const RvUint8 *         srcBuffer,
    RvUint8 *               dstBuffer,
    RvUint32                byteLength);

/**************************************************************************
 * RvSrtpAesPlugInDecryptCB
 * description: This callback is called when the user is needs to
 *              decrypt an outgoing data packet.}
 * parameters:
 *   thisPtr  - A pointer to the RvSrtpAesPlugIn_ object.
 *   purpose  - An encrypting purpose 
 * srcBuffer  - A pointer to the plain text buffer.
 * dstBuffer  - A pointer to the cipher text buffer.
 * byteLength - Length of the plain text buffer.
 * key        - The key to use for encryption.
 * Returns none
 ***************************************************************************/
typedef void (*RvSrtpAesPlugInDecryptCB)(
    struct RvSrtpAesPlugIn_ *thisPtr,
    RvSrtpEncryptionPurpose  purpose,
    const RvUint8 *          srcBuffer,
    RvUint8 *                dstBuffer,
    RvUint32                 byteLength);

/**************************************************************************
 * RvSrtpAesPlugInInitializeECBModeCB
 * description: This callback is called when the user is needs to
 *              initialize AES encryption in ECB mode
 * parameters:
 *   thisPtr    - A pointer to the RvSrtpAesPlugIn_ object.
 *   purpose    - An encrypting purpose 
 * direction    - encryption(RVSRTP_AESPLUGIN_DIRECTION_ENCRYPT) or decryption(RVSRTP_AESPLUGIN_DIRECTION_DECRYPT).
 *       key    - The key to use for encryption.
 * keyBitSize   - The key size to use for encryption (in bits).
 * blockBitSize - The block size to use for encryption (in bits). 
 * Returns TRUE, if initialization successes, otherwise FALSE.
 ***************************************************************************/
typedef  RvBool (*RvSrtpAesPlugInInitializeECBModeCB)(
     struct RvSrtpAesPlugIn_ *thisPtr,
     RvSrtpEncryptionPurpose purpose,
     RvUint8                 direction,
     const RvUint8*          key,
     RvUint16                keyBitSize,
     RvUint16                blockBitSize);

/**************************************************************************
 * RvSrtpAesPlugInInitializeCBCModeCB
 * description: This callback is called when the user is needs to
 *              initialize AES encryption in CBC mode
 * parameters:
 *   thisPtr    - A pointer to the RvSrtpAesPlugIn_ object.
 *   purpose    - An encrypting purpose 
 * direction    - encryption(RVSRTP_AESPLUGIN_DIRECTION_ENCRYPT) or decryption(RVSRTP_AESPLUGIN_DIRECTION_DECRYPT).
 *       key    - The key to use for encryption.
 * keyBitSize   - The key size to use for encryption (in bits).
 * blockBitSize - The block size to use for encryption (in bits). 
 *           iv - The initialization vector to use for encryption
 * Returns TRUE, if initialization successes, otherwise FALSE.
 ***************************************************************************/
typedef  RvBool (*RvSrtpAesPlugInInitializeCBCModeCB)(
     struct RvSrtpAesPlugIn_ *thisPtr,
     RvSrtpEncryptionPurpose purpose,
     RvUint8                 direction,
     const RvUint8*          key,
     RvUint16                keyBitSize,
     RvUint16                blockBitSize,
     const RvUint8*          iv);

#ifdef UPDATED_BY_SPIRENT
/**************************************************************************
 * RvSrtpMasterKeyEventCB
 * description: This callback is called when master key's lifetime threshold 
 *              is reached or master key is deleting by any cause.
 *              It is user callback
 * parameters:
 *   context - user's context (cct in our case)
 *       mki - master key's MKI  
 * direction - master key's direction (1 for local key or 0 for remote key)
 *  lifetime - remained lifetime
 * Returns none
 ***************************************************************************/
typedef  void (*RvSrtpMasterKeyEventCB)(
        void*                context,
        const RvUint8*       mki,
        RvUint8              direction,
        RvUint64             lifetime);
#endif // UPDATED_BY_SPIRENT

/***************************************************************************************
 * RvSrtpAesPlugIn :
 * description:
 *   This interface is used to allow the RTP Session to encrypt/decrypt
 *   data. To use this interface the user MUST implement all of the
 *   following callbacks:
 *              RvSrtpAesPlugInEncryptDataConstructCB
 *              RvSrtpAesPlugInEncryptDataDestructCB
 *              RvSrtpAesPlugInInitializeECBModeCB
 *              RvSrtpAesPlugInInitializeCBCModeCB
 *              RvSrtpAesPlugInEncryptCB
 *              RvSrtpAesPlugInDecryptCB
 * see also examples of usage in sample:
 *   RvSrtpAesEncryption
 ****************************************************************************************/
typedef struct RvSrtpAesPlugIn_
{
    RvSrtpAesPlugInEncryptDataConstructCB    construct;
    RvSrtpAesPlugInEncryptDataDestructCB     destruct;
    RvSrtpAesPlugInInitializeECBModeCB       initECBMode;
    RvSrtpAesPlugInInitializeCBCModeCB       initCBCMode;
	RvSrtpAesPlugInEncryptCB		         encrypt;
	RvSrtpAesPlugInDecryptCB		         decrypt;
	void*								     userData;     /* the user data associated with the object */

} RvSrtpAesPlugIn;

#if defined(SRTP_ADD_ON)
/***************************************************************************************
 * RvSrtpAesPlugInConstruct
 * description:  This method constructs a RvRtpEncryptionPlugIn. All of
 *               the callbacks must be suppled for this plugin to work.
 * parameters:
 *    plugin - the RvRtpEncryptionPlugIn object.
 *   userData - the user data associated with the object.
 *    encrypt - the RvRtpEncryptionPlugInEncryptCB callback to use with the object.
 *    decrypt - the RvRtpEncryptionPlugInGetBlockSizeCB callback to use with the object.
 * getBlockSize - the RvRtpEncryptionPlugInGetBlockSizeCB callback to use with the object.
 * getHeaderSize - the RvRtpEncryptionPlugInGetHeaderSizeCB callback to use with the object.
 * returns: A pointer to the object, if successful. NULL, otherwise.
 ***************************************************************************************/
RVAPI
RvSrtpAesPlugIn* RVCALLCONV RvSrtpAesPlugInConstruct(
     RvSrtpAesPlugIn*                         plugin,
     void*								      userData,
     RvSrtpAesPlugInEncryptDataConstructCB    construct,
     RvSrtpAesPlugInEncryptDataDestructCB     destruct,
     RvSrtpAesPlugInInitializeECBModeCB       initECBMode,
     RvSrtpAesPlugInInitializeCBCModeCB       initCBCMode,
     RvSrtpAesPlugInEncryptCB		          encrypt,
     RvSrtpAesPlugInDecryptCB		          decrypt);

/***************************************************************************************
 * RvSrtpAesPlugInDestruct
 * description:  This method destructs a RvSrtpAesPlugIn.
 * parameters:
 *    plugin - the RvSrtpAesPlugIn object.
 * returns: A pointer to the object, if successful. NULL, otherwise.
 ***************************************************************************************/
RVAPI
RvSrtpAesPlugIn* RVCALLCONV RvSrtpAesPlugInDestruct(
     RvSrtpAesPlugIn*                         plugin);

#else /* #if defined(SRTP_ADD_ON) */

#define RvSrtpAesPlugInConstruct(plugin, userData, construct, destruct, initECBMode, initCBCMode, encrypt, decrypt)
#define RvSrtpAesPlugInDestruct(plugin)

#endif /* #if defined(SRTP_ADD_ON) */

#ifdef __cplusplus
}
#endif

#endif /* RV_SRTP_AES_PLUGIN_H */

