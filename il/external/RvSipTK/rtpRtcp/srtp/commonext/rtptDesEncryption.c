/************************************************************************
 File Name     : rvdesencryption.c
 Description   :
*************************************************************************
 Copyright (c)  2001 , RADVision, Inc. All rights reserved.
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

#include "rvstdio.h"
#include "rvccore.h"

#if (RV_SECURITY == RV_YES)

#if !defined(NO_SSL) /* for embedded OS to remove warnings from compilations */
#include <openssl/des.h>
#endif

#include "rtptDesEncryption.h"

#include "rvassert.h"

/***********************************************************************
 * Private method declarations
 ***********************************************************************/
static void rvDesEncryptionScrunch(const RvUint8 outof[8], RvUint32 into[2]);
static void rvDesEncryptionExpandKey(const RvUint8 key[7], RvUint8 expandedKey[8]);

/***********************************************************************
 * Public declarations
 ***********************************************************************/
/*$
{function publish="no":
    {name: rvDesEncryptionConstruct }
    {class: RvDesEncryption }
    {include: rvdesencryption.h}
    {description:
        {p: Default constructor. }
    }
    {proto: RvDesEncryption* rvDesEncryptionConstruct(RvDesEncryption* thisPtr);}
    {params:
        {param: {n: thisPtr} {d:The RvDesEncryption object to construct.}}
    }
    {returns: If successful a pointer to the object is returned, otherwise NULL.}
}
$*/
RVCOREAPI
RvDesEncryption* RVCALLCONV rvDesEncryptionConstruct(RvDesEncryption *thisPtr)
{
    memset(thisPtr->iv,0,sizeof(thisPtr->iv));
    memset(thisPtr->prevBlock,0,sizeof(thisPtr->prevBlock));
    thisPtr->mode = RV_ENCRYPTIONMODE_ECB;
    memset(thisPtr->key,0,sizeof(thisPtr->key));
    thisPtr->encrypt = RV_TRUE;
    memset(&thisPtr->ks, 0, sizeof(DES_key_schedule));
    RvKeyConstruct(&thisPtr->currentKey);
    return thisPtr;
}

/*$
{function publish="no":
    {name: rvDesEncryptionDestruct }
    {class: RvDesEncryption }
    {include: rvdesencryption.h}
    {description:
        {p: This method destroys the object. }
    }
    {proto: void rvDesEncryptionDestruct(RvDesEncryption* thisPtr);}
    {params:
        {param: {n: thisPtr} {d:The RvDesEncryption object to destruct.}}
    }
}
$*/
RVCOREAPI
void RVCALLCONV rvDesEncryptionDestruct(RvDesEncryption *thisPtr)
{
    RvKeyDestruct(&thisPtr->currentKey);
}

/*$
{function publish="no":
    {name: rvDesEncryptionInitialize }
    {class: RvDesEncryption }
    {include: rvdesencryption.h}
    {description:
        {p: Initializes to RvDesEncryption object's mode  and whether to encrypt or
            decrypt. }
    }
    {proto: rvDesEncryptionInitialize(RvDesEncryption *thisPtr, RvEncryptioMode mode, RvEncryptionDirection direction);}
    {params:
        {param: {n: thisPtr}      {d:The RvDesEncryption object to initialize.}}
        {param: {n: mode}         {d:A flag to set the RvDesEncryption object to encrypt or decrypt. Valid
                                     modes are: RV_ENCRYPTIONMODE_ECB and RV_ENCRYPTIONMODE_CBC.}}
        {param: {n: direction}    {d:A flag to set the RvDesEncryption object to encrypt or decrypt. Use
                                     RV_ENCRYPTIONDIRECTION_ENCRYPT to encrypt,
                                     RV_ENCRYPTIONDIRECTION_DECRYPT to decrypt.}}
    }
}
$*/
RVCOREAPI
void RVCALLCONV rvDesEncryptionInitialize(RvDesEncryption *thisPtr, RvEncryptionMode mode, RvEncryptionDirection direction)
{
    thisPtr->mode = mode;
    thisPtr->encrypt = (direction == RV_ENCRYPTIONDIRECTION_ENCRYPT);
}

RVCOREAPI
void RVCALLCONV rvDesEncryptionSetIV(RvDesEncryption *thisPtr, const RvUint8 iv[8])
{
    rvDesEncryptionScrunch(iv, thisPtr->iv);
}

RVCOREAPI
void RVCALLCONV rvDesEncryptionSetKey(RvDesEncryption *thisPtr, const RvKey* keyPtr)
{
    RvAssert(RvKeyGetLengthInBits(keyPtr) >= 56);

    if(!RvKeyEqual(&thisPtr->currentKey, keyPtr))
    {
        RvKeyCopy(&thisPtr->currentKey, keyPtr);

        rvDesEncryptionExpandKey(RvKeyGetMaterial(keyPtr), thisPtr->key);
        DES_set_key_unchecked((const_DES_cblock*) thisPtr->key, &thisPtr->ks);
        /*rvDesEncryptionGenerateKeySet(thisPtr->key, thisPtr->keySet, thisPtr->encrypt);*/
    }
}

/*$
{function publish="no":
    {name: rvDesEncryptionProcess }
    {class: RvDesEncryption }
    {include: rvdesencryption.h}
    {description:
        {p: This method process the input buffer based on how the RvDesEncryption
            object was initialized.}
    }
    {proto:void rvDesEncryptionProcess(RvDesEncryption* thisPtr, const RvUint8* input, RvUint8* output, RvUint32 length);}
    {params:
        {param: {n: thisPtr}   {d:The RvDesEncryption object to perform the processing.}}
        {param: {n: input}     {d:The input buffer.}}
        {param: {n: output}    {d:The output buffer. This can be the same buffer as the input buffer.}}
        {param: {n: length}    {d:The size of the buffers in bytes.}}
    }
}
$*/
RVCOREAPI
void RVCALLCONV rvDesEncryptionProcess(RvDesEncryption* thisPtr, const RvUint8* input,
                            RvUint8* output, RvUint32 length)
{
    if (thisPtr->mode == RV_ENCRYPTIONMODE_CBC)
    {
        if (thisPtr->encrypt == RV_ENCRYPTIONDIRECTION_ENCRYPT)
        {
            DES_cbc_encrypt((const unsigned char*)input, output, length, &thisPtr->ks, (DES_cblock*)thisPtr->iv, DES_ENCRYPT);
        }
        else
        {
            DES_cbc_encrypt((const unsigned char*)input, output, length, &thisPtr->ks, (DES_cblock*)thisPtr->iv, DES_DECRYPT);        
        }
    }
    else
    {
        if (thisPtr->encrypt == RV_ENCRYPTIONDIRECTION_ENCRYPT)
        {
            DES_ecb_encrypt((const_DES_cblock*)input, (DES_cblock*)output, &thisPtr->ks, DES_ENCRYPT);
        }
        else
        {
            DES_ecb_encrypt((const_DES_cblock*)input, (DES_cblock*)output, &thisPtr->ks, DES_DECRYPT);
        }
    }
}

/*$
{function publish="no":
    {name: rvDesEncryptionReset }
    {class: RvDesEncryption }
    {include: rvdesencryption.h}
    {description:
        {p: This method reinitalizes the algorithm based on the previous
            initialization call.}
    }
    {proto:void rvDesEncryptionReset(RvDesEncryption* thisPtr);}
    {params:
        {param: {n: thisPtr}   {d:The RvDesEncryption object to reset.}}
    }
}
$*/
RVCOREAPI
void RVCALLCONV rvDesEncryptionReset(RvDesEncryption *thisPtr)
{
    thisPtr->prevBlock[0] = thisPtr->iv[0];
    thisPtr->prevBlock[1] = thisPtr->iv[1];
}

/***********************************************************************
 * Private method definitions
 ***********************************************************************/

static void rvDesEncryptionScrunch(const RvUint8 outof[8], RvUint32 into[2])
{
    *into    = (*outof++ & 0xFFL) << 24;
    *into   |= (*outof++ & 0xFFL) << 16;
    *into   |= (*outof++ & 0xFFL) <<  8;
    *into++ |= (*outof++ & 0xFFL);
    *into    = (*outof++ & 0xFFL) << 24;
    *into   |= (*outof++ & 0xFFL) << 16;
    *into   |= (*outof++ & 0xFFL) <<  8;
    *into   |= (*outof   & 0xFFL);
}

/*$
{function publish="no":
    {name: rvDesEncryptionExpandKey }
    {class: RvDesEncryption }
    {include: rvdesencryption.h}
    {description:
        {p: Expands a 56 bit key into a 64 bit key by esxpanding it into
            8 octets with an odd parity bit in the most significant position. }
    }
    {proto: void rvDesEncryptionExpandKey(const RvUint8 key[7], RvUint8 expandedKey[8]);}
    {params:
        {param: {n:key}         {d:The 56 bit key.}}
        {param: {n:expandedKey} {d:The expanded 64 bit key.}}
    }
}
$*/
void rvDesEncryptionExpandKey(const RvUint8 key[7], RvUint8 expandedKey[8])
{
    static const RvUint8 byte1[8]  = {   0,   1,   2,   3,   4,   5,   6,   6};
    static const RvUint8 shift1[8] = {   0,   1,   2,   3,   4,   5,   6,   1};
    static const RvUint8 byte2[8]  = {   0,   0,   1,   2,   3,   4,   5,   6};
    static const RvUint8 shift2[8] = {   0,   7,   6,   5,   4,   3,   2,   1};
    static const RvUint8 mask2[8]  = {0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F};

    int     i;
    int     j;
    int     parity;

    /* Expand key to set msb in the byte to be an odd parity bit. */
    for(i = 0; i < 8; i++)
    {
        if(byte1[i] == byte2[i])
            expandedKey[i] = (RvUint8)((key[byte2[i]] >> shift2[i]) & 0x7F);
        else
            expandedKey[i] = (RvUint8)((((key[byte2[i]] >> shift2[i])  & mask2[i])+
                               (key[byte1[i]] << shift1[i])) & 0x7F);

        /* Add odd parity bit to msb */
        parity = 0;
        for(j = 0; j < 8; j++)
           parity += ((expandedKey[i] >>  j) & 0x01);

        if((parity%2) == 0)
            expandedKey[i] += 0x80;

    }
}

#endif /* RV_SECURITY */
