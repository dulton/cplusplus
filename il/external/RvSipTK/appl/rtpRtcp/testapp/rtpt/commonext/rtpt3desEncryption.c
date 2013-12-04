/************************************************************************
 File Name     : rv3desencryption.c
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


#include "rtpt3desEncryption.h"
#include "rvassert.h"

/***********************************************************************
 * Public declarations
 ***********************************************************************/
/*$
{function publish="no":
    {name: rv3DesEncryptionConstruct }
    {class: Rv3DesEncryption }
    {include: rv3desencryption.h}
    {description:
        {p: This method constructs a Rv3DesEncryption object.}
    }
    {proto: Rv3DesEncryption* rv3DesEncryptionConstruct(Rv3DesEncryption* thisPtr);}
    {params:
        {param: {n: thisPtr} {d:The Rv3DesEncryption object to construct.}}
    }
    {returns: If successful a pointer to the object is returned, otherwise NULL.}
}
$*/
RVCOREAPI
Rv3DesEncryption* RVCALLCONV rv3DesEncryptionConstruct(Rv3DesEncryption *thisPtr)
{
    memset(thisPtr->iv,0,sizeof(thisPtr->iv));
    memset(thisPtr->prevBlock,0,sizeof(thisPtr->prevBlock));
    thisPtr->encrypt = RV_TRUE;
    rvDesEncryptionConstruct(&thisPtr->pass1);
    rvDesEncryptionConstruct(&thisPtr->pass2);
    rvDesEncryptionConstruct(&thisPtr->pass3);
    RvKeyConstruct(&thisPtr->currentKey);
    return thisPtr;
}

/*$
{function publish="no":
    {name: rv3DesEncryptionDestruct }
    {class: Rv3DesEncryption }
    {include: rv3desencryption.h}
    {description:
        {p: The rv3DesEncryptionDestruct destructor. This method destroys the
            object and releases any resource used by the object. }
    }
    {proto: void rv3DesEncryptionDestruct(Rv3DesEncryption* thisPtr);}
    {params:
        {param: {n: thisPtr} {d:The Rv3DesEncryption object to destruct.}}
    }
}
$*/
RVCOREAPI
void RVCALLCONV   rv3DesEncryptionDestruct(Rv3DesEncryption *thisPtr)
{

    rvDesEncryptionDestruct(&thisPtr->pass1);
    rvDesEncryptionDestruct(&thisPtr->pass2);
    rvDesEncryptionDestruct(&thisPtr->pass3);
    RvKeyDestruct(&thisPtr->currentKey);
}

/*$
{function publish="no":
    {name: rv3DesEncryptionInitialize }
    {class: Rv3DesEncryption }
    {include: rv3desencryption.h}
    {description:
        {p: This method initializes the algorithm. }
    }
    {proto: void rv3DesEncryptionInitialize(Rv3DesEncryption* thisPtr, RvUint8 direction);}
    {params:
        {param: {n: thisPtr}   {d:The Rv3DesEncryption object to be copied to.}}
        {param: {n: direction} {d:The direction to run the algorithm in. Use RV_3DESENCRYPTION_DIRECTION_ENCRYPT
                                  to encrypt, RV_3DESENCRYPTION_DIRECTION_DECRYPT to decrypt.}}
    }
}
$*/
RVCOREAPI
void RVCALLCONV rv3DesEncryptionInitialize(Rv3DesEncryption *thisPtr, RvEncryptionMode mode, RvEncryptionDirection direction)
{
    thisPtr->encrypt = (direction == RV_ENCRYPTIONDIRECTION_ENCRYPT);

    if(thisPtr->encrypt)
    {
        rvDesEncryptionInitialize(&thisPtr->pass1, RV_ENCRYPTIONMODE_ECB, RV_ENCRYPTIONDIRECTION_ENCRYPT);
        rvDesEncryptionInitialize(&thisPtr->pass2, RV_ENCRYPTIONMODE_ECB, RV_ENCRYPTIONDIRECTION_DECRYPT);
        rvDesEncryptionInitialize(&thisPtr->pass3, RV_ENCRYPTIONMODE_ECB, RV_ENCRYPTIONDIRECTION_ENCRYPT);
    }
    else
    {
        rvDesEncryptionInitialize(&thisPtr->pass1, RV_ENCRYPTIONMODE_ECB, RV_ENCRYPTIONDIRECTION_DECRYPT);
        rvDesEncryptionInitialize(&thisPtr->pass2, RV_ENCRYPTIONMODE_ECB, RV_ENCRYPTIONDIRECTION_ENCRYPT);
        rvDesEncryptionInitialize(&thisPtr->pass3, RV_ENCRYPTIONMODE_ECB, RV_ENCRYPTIONDIRECTION_DECRYPT);
    }
    RV_UNUSED_ARG(mode);
}

RVCOREAPI
void RVCALLCONV  rv3DesEncryptionSetIV(Rv3DesEncryption *thisPtr, const RvUint8 iv[8])
{
    memcpy(thisPtr->iv, iv, 8);
}

RVCOREAPI
void RVCALLCONV  rv3DesEncryptionSetKey(Rv3DesEncryption *thisPtr, const RvKey* keyPtr)
{

    RvAssert(RvKeyGetLengthInBits(keyPtr) >= 168);

    if(!RvKeyEqual(&thisPtr->currentKey, keyPtr))
    {
        RvKey key;

        RvKeyConstruct(&key);
        RvKeyCopy(&thisPtr->currentKey, keyPtr);

        if(thisPtr->encrypt)
        {
            RvKeySetValue(&key,&RvKeyGetMaterial(keyPtr)[0],56);
            rvDesEncryptionSetKey(&thisPtr->pass1, &key);
            RvKeySetValue(&key,&RvKeyGetMaterial(keyPtr)[7],56);
            rvDesEncryptionSetKey(&thisPtr->pass2, &key);
            RvKeySetValue(&key,&RvKeyGetMaterial(keyPtr)[14],56);
            rvDesEncryptionSetKey(&thisPtr->pass3, &key);
        }
        else
        {
            RvKeySetValue(&key,&RvKeyGetMaterial(keyPtr)[14],56);
            rvDesEncryptionSetKey(&thisPtr->pass1, &key);
            RvKeySetValue(&key,&RvKeyGetMaterial(keyPtr)[7],56);
            rvDesEncryptionSetKey(&thisPtr->pass2, &key);
            RvKeySetValue(&key,&RvKeyGetMaterial(keyPtr)[0],56);
            rvDesEncryptionSetKey(&thisPtr->pass3, &key);
        }

        RvKeyDestruct(&key);
    }
}


/*$
{function publish="no":
    {name: rv3DesEncryptionProcess }
    {class: Rv3DesEncryption }
    {include: rv3desencryption.h}
    {description:
        {p: This method process a buffer using the triple DES algorithm based on the
            how the object was initialized in rv3DesEncryptionInitialize. }
    }
    {proto: void rv3DesEncryptionProcess(Rv3DesEncryption* thisPtr, const RvUint8* input, RvUint8* output, RvUint32 length);}
    {params:
        {param: {n: thisPtr} {d:The Rv3DesEncryption object to be copied to.}}
        {param: {n: input}   {d:The buffer to process.}}
        {param: {n: output}  {d:The results of the processing.}}
        {param: {n: length}  {d:The length of the input buffer in bytes.}}
    }
    {see_also:
        {n:void rv3DesEncryptionInitialize(Rv3DesEncryption* thisPtr, RvUint8 direction, const RvUint8 key[21], const RvUint8 iv[8]);}
    }
}
$*/
RVCOREAPI
void RVCALLCONV rv3DesEncryptionProcess(Rv3DesEncryption* thisPtr, const RvUint8* input, RvUint8* output, RvUint32 length)
{
    RvUint32 i;
    RvUint8  pass1Result[8];
    RvUint8  pass2Result[8];
    RvUint8  inputBlock[8];

    if(thisPtr->encrypt)
    {
        for(i = 0;i < length; i += 8)
        {
            inputBlock[0] = (RvUint8)(thisPtr->prevBlock[0] ^ input[i+0]);
            inputBlock[1] = (RvUint8)(thisPtr->prevBlock[1] ^ input[i+1]);
            inputBlock[2] = (RvUint8)(thisPtr->prevBlock[2] ^ input[i+2]);
            inputBlock[3] = (RvUint8)(thisPtr->prevBlock[3] ^ input[i+3]);
            inputBlock[4] = (RvUint8)(thisPtr->prevBlock[4] ^ input[i+4]);
            inputBlock[5] = (RvUint8)(thisPtr->prevBlock[5] ^ input[i+5]);
            inputBlock[6] = (RvUint8)(thisPtr->prevBlock[6] ^ input[i+6]);
            inputBlock[7] = (RvUint8)(thisPtr->prevBlock[7] ^ input[i+7]);

            rvDesEncryptionProcess(&thisPtr->pass1, inputBlock, pass1Result, 8);
            rvDesEncryptionProcess(&thisPtr->pass2, pass1Result, pass2Result, 8);
            rvDesEncryptionProcess(&thisPtr->pass3, pass2Result, &output[i], 8);

            memcpy(thisPtr->prevBlock, &output[i], 8);
        }

    }
    else
    {
        for(i = 0;i < length; i += 8)
        {
            memcpy(inputBlock, &input[i], 8);

            rvDesEncryptionProcess(&thisPtr->pass1, inputBlock, pass1Result, 8);
            rvDesEncryptionProcess(&thisPtr->pass2, pass1Result, pass2Result, 8);
            rvDesEncryptionProcess(&thisPtr->pass3, pass2Result, &output[i], 8);

            output[i+0] ^= thisPtr->prevBlock[0];
            output[i+1] ^= thisPtr->prevBlock[1];
            output[i+2] ^= thisPtr->prevBlock[2];
            output[i+3] ^= thisPtr->prevBlock[3];
            output[i+4] ^= thisPtr->prevBlock[4];
            output[i+5] ^= thisPtr->prevBlock[5];
            output[i+6] ^= thisPtr->prevBlock[6];
            output[i+7] ^= thisPtr->prevBlock[7];

            memcpy(thisPtr->prevBlock, inputBlock, 8);
        }
    }
}

/*$
{function publish="no":
    {name: rv3DesEncryptionReset }
    {class: Rv3DesEncryption }
    {include: rv3desencryption.h}
    {description:
        {p: This method resets the processing state. This causes the algorithm to reset the
            initialization vector to the one supplied in the rv3DesEncryptionInitialize
            method.}
    }
    {proto: void rv3DesEncryptionReset(Rv3DesEncryption* thisPtr);}
    {params:
        {param: {n: thisPtr} {d:The Rv3DesEncryption object to be copied to.}}
    }
    {see_also:
        {n:void rv3DesEncryptionInitialize(Rv3DesEncryption* thisPtr, RvUint8 direction, const RvUint8 key[21], const RvUint8 iv[8]);}
    }
}
$*/
RVCOREAPI
void RVCALLCONV  rv3DesEncryptionReset(Rv3DesEncryption* thisPtr)
{
    memcpy(thisPtr->prevBlock, thisPtr->iv, 8);

    rvDesEncryptionReset(&thisPtr->pass1);
    rvDesEncryptionReset(&thisPtr->pass2);
    rvDesEncryptionReset(&thisPtr->pass3);
}

#endif /* RV_SECURITY */
