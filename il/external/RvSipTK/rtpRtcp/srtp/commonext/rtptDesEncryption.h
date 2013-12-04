/************************************************************************
 File Name	   : rvdesencryption.h
 Description   : This file contains the RvDesEncryption class for
				 performing DES encryption and decryption on a buffer.
				 The code for this class was based on the code found in
				 "Applied Cryptology" by Bruce Schneier ISBN 0-471-11709-9.
				 Chapter 12 covers the DES algorithm is detail.
*************************************************************************
 Copyright (c)	2001 , RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVision Inc.
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVision Inc.

 RADVision Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
*************************************************************************
 $Revision: #1 $
 $Date: 2011/08/05 $
 $Author: songkamongkol $
************************************************************************/

#if !defined(RVDESENCRYPTION_H)
#define RVDESENCRYPTION_H

#if defined(NO_SSL) /* for embedded OS */
#define const_DES_cblock RvUint32
#define DES_key_schedule RvUint32
#define DES_set_key_unchecked(key, ks)
#define DES_cbc_encrypt(input, output, length, ks, iv, e_d) memcpy(output, input, length)
#define DES_ecb_encrypt(input, output, ks, e_d)             memcpy(output, input, 8)
typedef unsigned char DES_cblock[8];
#else
#include <openssl/des.h>
#endif

#include "rvtypes.h"
#include "rvmemory.h"
#include "rtptEncryption.h"
#include "rvkey.h"

#ifdef __cplusplus
extern "C" {
#endif

/*$
{type publish="no":
	{name: RvDesEncryption}
	{superpackage: Security}
	{include: rvdesencryption.h}
	{description:
		{p: This class is used for encrypting and decrypting buffers
			using the DES algorithm. It supports ECB and CBC modes of
			encryption. Only buffers that are multiples of the block
			size (64-bits) are allowed.}
	}
}
$*/
typedef struct
{
    RvKey            currentKey;    /* The current key being used */
	RvUint32         iv[2]; 		/* Initialization Vector	  */
	RvUint32         prevBlock[2];	/* Previously processed block */
	RvEncryptionMode mode;			/* Encryption mode			  */
	RvUint8          key[8];		/* The expanded key						  */
	RvBool	         encrypt;		/* RV_TRUE, if encrypting data, otherwise decrypting data */

    DES_key_schedule ks;            /* open ssl DES key schedule */
    
} RvDesEncryption;

/* Construct/Destruct Methods */
RVCOREAPI
RvDesEncryption* RVCALLCONV rvDesEncryptionConstruct(RvDesEncryption *thisPtr);

RVCOREAPI
void RVCALLCONV rvDesEncryptionDestruct(RvDesEncryption *thisPtr);


/* Encryption/Decryption Methods */
RVCOREAPI
void RVCALLCONV rvDesEncryptionInitialize(RvDesEncryption *thisPtr, RvEncryptionMode mode, RvEncryptionDirection direction);

RVCOREAPI
void RVCALLCONV rvDesEncryptionSetIV(RvDesEncryption *thisPtr, const RvUint8 iv[8]);

RVCOREAPI
void RVCALLCONV rvDesEncryptionSetKey(RvDesEncryption *thisPtr, const RvKey* keyPtr);

RVCOREAPI
void RVCALLCONV rvDesEncryptionProcess(RvDesEncryption *thisPtr, const RvUint8 *input,
							RvUint8 *output, RvUint32 length);
RVCOREAPI
void RVCALLCONV rvDesEncryptionReset(RvDesEncryption *thisPtr);


#ifdef __cplusplus
}
#endif

#endif	/* Include guard */

