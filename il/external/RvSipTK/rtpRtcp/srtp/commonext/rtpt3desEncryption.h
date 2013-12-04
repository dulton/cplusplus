/************************************************************************
 File Name	   : rv3desencryption.h
 Description   :
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

#if !defined(RV3DESENCRYPTION_H)
#define RV3DESENCRYPTION_H

#include "rvtypes.h"
#include "rtptEncryption.h"
#include "rtptDesEncryption.h"
#include "rvkey.h"

#ifdef __cplusplus
extern "C" {
#endif

/*$
{type publish="no":
	{name: Rv3DesEncryption}
	{superpackage: Security}
	{include: rv3desencryption.h}
	{description:
		{p: This class implements the Triple DES encryption algorithm in
			outer CBC mode. Only buffers that are multiples of the block
			size (64-bits) are allowed.}
	}
	{attributes scope="private":
		{attribute: {t: RvDesEncryption } {n: pass1 } {d: This encryptor performs the first pass.}}
		{attribute: {t: RvDesEncryption } {n: pass2 } {d: This encryptor performs the second pass.}}
		{attribute: {t: RvDesEncryption } {n: pass3 } {d: This encryptor performs the third pass.}}
	}
}
$*/
typedef struct
{
    RvKey           currentKey;    /* The current key being used */
	RvBool			encrypt;	   /* RV_TRUE, if encrypting data, otherwise decrypting data */
	RvUint8 		iv[8];		   /* Initialization Vector 	 */
	RvUint8 		prevBlock[8];  /* Previously processed block */
	RvDesEncryption pass1;
	RvDesEncryption pass2;
	RvDesEncryption pass3;
} Rv3DesEncryption;

/* Methods */
RVCOREAPI
Rv3DesEncryption* RVCALLCONV rv3DesEncryptionConstruct(Rv3DesEncryption *thisPtr);

RVCOREAPI
void RVCALLCONV  rv3DesEncryptionDestruct(Rv3DesEncryption *thisPtr);

RVCOREAPI
void RVCALLCONV rv3DesEncryptionInitialize(Rv3DesEncryption *thisPtr, RvEncryptionMode mode, RvEncryptionDirection direction);

RVCOREAPI
void RVCALLCONV  rv3DesEncryptionSetIV(Rv3DesEncryption *thisPtr, const RvUint8 iv[8]);
RVCOREAPI
void RVCALLCONV  rv3DesEncryptionSetKey(Rv3DesEncryption *thisPtr, const RvKey* keyPtr);
RVCOREAPI
void RVCALLCONV rv3DesEncryptionProcess(Rv3DesEncryption *thisPtr, const RvUint8 *input, RvUint8 *output, RvUint32 length);
RVCOREAPI
void RVCALLCONV rv3DesEncryptionReset(Rv3DesEncryption *thisPtr);


#ifdef __cplusplus
}
#endif

#endif	/* Include guard */

