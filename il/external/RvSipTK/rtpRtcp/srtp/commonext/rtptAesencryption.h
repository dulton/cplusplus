/************************************************************************
 File Name	   : rtptAesencryption.h
 Description   : implementation of AES using openssl library
*************************************************************************
 Copyright (c)	2001 , RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVISION Inc. 
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVISION Inc. 
 
 RADVISION Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
*************************************************************************
 $Revision: #1 $
 $Date: 2011/08/05 $
 $Author: songkamongkol $
************************************************************************/

#if !defined(RTPT_AESENCRYPTION_H)
#define RTPT_AESENCRYPTION_H

#if defined(NO_SSL) /* for embedded OS */
#define AES_BLOCK_SIZE 16
#define AES_KEY RvUint32
#define AES_set_decrypt_key(a, b, c)   
#define AES_set_encrypt_key(a, b, c)   
#define AES_ecb_encrypt(input, output, key, e_d)             memcpy(output, input, AES_BLOCK_SIZE)
#define AES_cbc_encrypt(input, output, length, key, iv, e_d) memcpy(output, input, length)
#else

#include <openssl/aes.h>

#endif

#include "rvtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Encryption direction */
#define RV_AESENCRYPTION_DIRECTION_ENCRYPT ((RvUint8)0) /* Encrypt buffer */
#define RV_AESENCRYPTION_DIRECTION_DECRYPT ((RvUint8)1) /* Decrypt buffer */


#define RV_AESENCRYPTION_MAX_BC 			(256/32)
#define RV_AESENCRYPTION_MAX_KC 			(256/32)
#define RV_AESENCRYPTION_MAX_ROUNDS 		14

/********************************************************************************
 * type: RvAesEncryption
 *  description:	
 *          This class is used for encrypting and decrypting buffers
 *          using the AES (Rijndael) algorithm that is implemented in openssl library.
 *          It supports ECB and CBC modes of encryption. 
 *********************************************************************************/
typedef struct
{
	RvUint8  blockWidth;	/* Width of a data block in bytes */
	RvUint8  mode;			/* Encryption mode */
	RvBool	 encrypt;		/* RV_TRUE, if encrypting data, otherwise decrypting data */
    
    AES_KEY  aesKey;              /* open ssl structure */
	RvUint8  iv[AES_BLOCK_SIZE];  /* open ssl initialization vector */

} RvAesEncryption;

/* Construct/Destruct Methods */
RvAesEncryption* rvAesEncryptionConstruct(RvAesEncryption* thisPtr);

void rvAesEncryptionDestruct(RvAesEncryption* thisPtr);

/* Encryption/Decryption Methods */
RvBool rvAesEncryptionInitializeECBMode(RvAesEncryption* thisPtr, 
										RvUint8 direction,
										const RvUint8* key,
										RvUint16 keyBitSize,
										RvUint16 blockBitSize);
RvBool rvAesEncryptionInitializeCBCMode(RvAesEncryption* thisPtr, 
										RvUint8 direction,
										const RvUint8* key,
										RvUint16 keyBitSize,
										RvUint16 blockBitSize,
										const RvUint8* iv);
void rvAesEncryptionProcess(RvAesEncryption* thisPtr, const RvUint8* input, 
							RvUint8* output, RvUint32 length);


#ifdef __cplusplus
}
#endif

#endif	/* RTPT_AESENCRYPTION_H Include guard */

