/************************************************************************
 File Name     : rvaesencryption.c
 Description   :
*************************************************************************
 Copyright (c)  2001 , RADVision, Inc. All rights reserved.
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
************************************************************************
 $Revision: #1 $ Updating to work with Open SSL. Works only with blocksize = 16 bytes
 $Date: 2011/08/05 $
 $Author: songkamongkol $
************************************************************************/
#include "rvstdio.h" /* for memset and memcpy on some OS */
#include "rtptAesencryption.h"


/* Encryption mode */
#define RV_AESENCRYPTION_MODE_ECB 0  /* Electronic Codebook Mode   */
#define RV_AESENCRYPTION_MODE_CBC 1  /* Cipher Block Chaining Mode */

/***********************************************************************
 * Private declarations   
 ***********************************************************************/
static int rvAesEncryptionIntitialize(
    IN RvAesEncryption *thisPtr, 
    IN const RvUint8* key, 
    IN RvUint16 keySize, 
    IN RvUint16 blockSize, 
    IN RvUint8 direction);

/***********************************************************************
 * Public definitions   
 ***********************************************************************/
/*$
{function publish="no":
	{name: rvAesEncryptionConstruct }
	{class: RvAesEncryption }
	{include: rvaesencryption.h}
	{description:
		{p: Default constructor. }
	}
	{proto: RvAesEncryption* rvAesEncryptionConstruct(RvAesEncryption* thisPtr);}
	{params:
		{param: {n: thisPtr} {d:The RvAesEncryption object to construct.}}
	}
	{returns: If successful a pointer to the object is returned, otherwise NULL.}
}
$*/
RvAesEncryption *rvAesEncryptionConstruct(RvAesEncryption *thisPtr)
{
    thisPtr->mode = RV_AESENCRYPTION_MODE_ECB;          
    thisPtr->encrypt = RV_TRUE;
    thisPtr->blockWidth = 0;

    memset(&thisPtr->aesKey, 0, sizeof(AES_KEY));
    return thisPtr;
}

/*$
{function  publish="no":
	{name: rvAesEncryptionDestruct }
	{class: RvAesEncryption }
	{include: rvaesencryption.h}
	{description:
		{p: This method destroys the object. }
	}
	{proto: void rvAesEncryptionDestruct(RvAesEncryption* thisPtr);}
	{params:
		{param: {n: thisPtr} {d:The RvAesEncryption object to destruct.}}
	}
}
$*/
void rvAesEncryptionDestruct(RvAesEncryption *thisPtr)
{
    /* Nothing to do */
    RV_UNUSED_ARG(thisPtr);
}

/*$
{function  publish="no":
	{name: rvAesEncryptionInitializeECBMode }
	{class: RvAesEncryption }
	{include: rvaesencryption.h}
	{description:
		{p: Initializes to RvAesEncryption object for ECB mode encryption or
            decryption. }
	}
	{proto: RvBool rvAesEncryptionInitializeECBMode(RvAesEncryption* thisPtr, RvUint8 direction, const RvUint8* key, RvUint16 keyBitSize, RvUint16 blockBitSize);}
	{params:
		{param: {n: thisPtr}      {d:The RvAesEncryption object to initialize.}}
		{param: {n: direction}    {d:A flag to set the RvAesEncryption object to encrypt or decrypt. Use 
                                     RV_AESENCRYPTION_DIRECTION_ENCRYPT to encrypt, 
                                     RV_AESENCRYPTION_DIRECTION_DECRYPT to decrypt.}}
		{param: {n: key}          {d:The key to use.}}
		{param: {n: keyBitSize}   {d:The size of the key in bits.}}
	}
	{returns: RV_TRUE is successful, otherwise rvFalse.}
}
$*/
RvBool rvAesEncryptionInitializeECBMode(RvAesEncryption* thisPtr, 
                                      RvUint8 direction,
                                      const RvUint8* key,
                                      RvUint16 keyBitSize,
                                      RvUint16 blockBitSize)
{
    RvBool succeeded = RV_TRUE;
    
    thisPtr->mode    = RV_AESENCRYPTION_MODE_ECB;
    thisPtr->encrypt = (direction == RV_AESENCRYPTION_DIRECTION_ENCRYPT);

    succeeded = (rvAesEncryptionIntitialize(thisPtr, key, keyBitSize, blockBitSize, direction) == 0);

    return succeeded;
}

/*$
{function  publish="no":
	{name: rvAesEncryptionInitializeCBCMode }
	{class: RvAesEncryption }
	{include: rvaesencryption.h}
	{description:
		{p: Initializes to RvAesEncryption object for CBC mode encryption or
            decryption. }
	}
	{proto: RvBool rvAesEncryptionInitializeCBCMode(RvAesEncryption* thisPtr, RvUint8 direction, const RvUint8* key, RvUint16 keyBitSize, RvUint16 blockBitSize, const RvUint8* iv);}
	{params:
		{param: {n: thisPtr}      {d:The RvAesEncryption object to initialize.}}
		{param: {n: direction}    {d:A flag to set the RvAesEncryption object to encrypt or decrypt. Use 
                                     RV_AESENCRYPTION_DIRECTION_ENCRYPT to encrypt, 
                                     RV_AESENCRYPTION_DIRECTION_DECRYPT to decrypt.}}
		{param: {n: key}          {d:The key to use.}}
		{param: {n: keyBitSize}   {d:The size of the key in bits.}}
	}
	{returns: RV_TRUE is successful, otherwise rvFalse.}
}
$*/
RvBool rvAesEncryptionInitializeCBCMode(RvAesEncryption *thisPtr, 
                                        RvUint8 direction,
                                        const RvUint8* key,
                                        RvUint16 keyBitSize,
                                        RvUint16 blockBitSize,
                                        const RvUint8* iv)
{
    RvBool succeeded = RV_TRUE;

    thisPtr->mode    = RV_AESENCRYPTION_MODE_CBC;
    thisPtr->encrypt = (direction == RV_AESENCRYPTION_DIRECTION_ENCRYPT);

    succeeded = (rvAesEncryptionIntitialize(thisPtr, key, keyBitSize, blockBitSize, direction) == 0);
    /* Store the initialization vector */ 
    memcpy(thisPtr->iv, iv, AES_BLOCK_SIZE); 

    return succeeded;
}

/*$
{function  publish="no":
	{name: rvAesEncryptionProcess }
	{class: RvAesEncryption }
	{include: rvaesencryption.h}
	{description:
		{p: This method process the input buffer based on how the RvAesEncryption
            object was initialized.}
	}
	{proto:void rvAesEncryptionProcess(RvAesEncryption* thisPtr, const RvUint8* input, RvUint8* output, RvUint32 length);}
	{params:
		{param: {n: thisPtr}   {d:The RvAesEncryption object to perform the processing.}}
		{param: {n: input}     {d:The input buffer.}}
		{param: {n: output}    {d:The output buffer. This can be the same buffer as the input buffer.}}
		{param: {n: length}    {d:The size of the buffers in bytes.}}
	}
}
$*/
void rvAesEncryptionProcess(RvAesEncryption *thisPtr, const RvUint8 *input, 
                            RvUint8 *output, RvUint32 length)
{
	int i;
   	int numBlocks  = length/(thisPtr->blockWidth*4);

    RvUint8 inputBlock[AES_BLOCK_SIZE];
    RvUint8 outputBlock[AES_BLOCK_SIZE];


   if(thisPtr->encrypt)
    {
 	    switch (thisPtr->mode) 
        {
	        case RV_AESENCRYPTION_MODE_ECB: 

		        for (i = 0; i < numBlocks; i++) 
                {
                    /* Copy the input to the input block*/
			        
                    memcpy(inputBlock, input+i*AES_BLOCK_SIZE, AES_BLOCK_SIZE);
                    /* Encrypt the block */

                    /* NOTE: Open SSL AES_ecb_encrypt works with block 128bit only */
                    AES_ecb_encrypt(inputBlock, outputBlock, &thisPtr->aesKey, (const int)AES_ENCRYPT);
                    /*rvAesEncryptionEncryptBlock(thisPtr, inputBlock, outputBlock);*/
		        
                    /* Copy the output block to the output */

                    memcpy(output+i*AES_BLOCK_SIZE, outputBlock, AES_BLOCK_SIZE);
		        }
                
		        break;
		    
            case RV_AESENCRYPTION_MODE_CBC:
                {
                    /* NOTE: Open SSL AES_cbc_encrypt works with block 128bit only */
                    AES_cbc_encrypt(input, output, length, &thisPtr->aesKey, thisPtr->iv, (const int)AES_ENCRYPT);
                }
                break;

	    }
    }        
    else
    {
        /* Decrypt */
	    switch (thisPtr->mode) 
        {
	        case RV_AESENCRYPTION_MODE_ECB: 
		        for (i = 0; i < numBlocks; i++) 
                {
		                           
                    memcpy(inputBlock, input+i*AES_BLOCK_SIZE, AES_BLOCK_SIZE);
                    /* Decrypt the block */
                    /*rvAesEncryptionDecryptBlock(thisPtr, inputBlock, outputBlock);*/
                    /* NOTE: Open SSL AES_ecb_encrypt works with block 128bit only */
                    AES_ecb_encrypt(inputBlock, outputBlock, &thisPtr->aesKey, (const int)AES_DECRYPT);			        

                    memcpy(output+i*AES_BLOCK_SIZE, outputBlock, AES_BLOCK_SIZE);
		        }
		        break;
		    
	        case RV_AESENCRYPTION_MODE_CBC:
                {                    
                    /* NOTE: Open SSL AES_cbc_encrypt works with block 128bit only */
                    AES_cbc_encrypt(input, output, length, &thisPtr->aesKey, thisPtr->iv, (const int)AES_DECRYPT);
                }
		       break;
        }
    
    }
}

static int rvAesEncryptionIntitialize(
    IN RvAesEncryption *thisPtr, 
    IN const RvUint8* key, 
    IN RvUint16 keySize, 
    IN RvUint16 blockSize, 
    IN RvUint8 direction)
{  
	switch (blockSize) 
    {
	    case 128: 
            thisPtr->blockWidth = 4; 
            break;
	    case 192: 
            thisPtr->blockWidth = 6; 
            break;
	    case 256: 
            thisPtr->blockWidth = 8; 
            break;
	    default : return (-2);
	}

    if (direction == RV_AESENCRYPTION_DIRECTION_ENCRYPT)
    {
        AES_set_encrypt_key(key, (const int) keySize, &thisPtr->aesKey);
    }
    else
    {
       AES_set_decrypt_key(key, (const int) keySize, &thisPtr->aesKey);
    }
    RV_UNUSED_ARG(key);     /* for NO_SSL code */
    RV_UNUSED_ARG(keySize); /* for NO_SSL code */
    return 0;
}








