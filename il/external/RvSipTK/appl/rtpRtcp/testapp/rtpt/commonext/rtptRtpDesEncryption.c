/************************************************************************
 File Name     : rvrtpdesencryption.c
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
#include "rtptRtpDesEncryption.h"
#include "rvlog.h"

static RvLogSource* rvLogPtr = NULL; 
/* RTP DES Initialization vector (RFC 1889, section 9.1) */
static const RvUint8 iv[8] = {0,0,0,0,0,0,0,0};

/**********************************************************************************
* Private method declarations
**********************************************************************************/
static void rvRtpDesEncryptionEncryptCallback(RvRtpEncryptionPlugIn *thisPtr,
								const RvUint8 *srcBuffer,
								RvUint8 *dstBuffer,
								RvUint32 byteLength,
                                RvRtpEncryptionData* dataPtr,
                                const  RvKey* keyPtr);
static void rvRtpDesEncryptionDecryptCallback(RvRtpEncryptionPlugIn *thisPtr,
								const RvUint8 *srcBuffer,
								RvUint8 *dstBuffer,
								RvUint32 byteLength,
                                RvRtpEncryptionData* dataPtr,
                                const  RvKey* keyPtr);
static RvUint32 rvRtpDesEncryptionGetBlockSizeCallback(RvRtpEncryptionPlugIn *thisPtr);
static RvUint32 rvRtpDesEncryptionGetHeaderSizeCallback(RvRtpEncryptionPlugIn *thisPtr);

/*$
{function:
    {name: rvRtpDesEncryptionConstruct }
    {class: RvRtpDesEncryption }
    {include: rvrtpdesencryption.h}
    {description:
        {p: This method constructs a RvRtpDesEncryption object.}
    }
    {proto: RvRtpDesEncryption* rvRtpDesEncryptionConstruct(RvRtpDesEncryption* thisPtr);}
    {params:
        {param: {n:thisPtr} {d: The RvRtpDesEncryption object being constructed.}}
    }
    {returns: A pointer to the object, if successful. NULL, otherwise.}
}
$*/
RVAPI
RvRtpDesEncryption* RVCALLCONV rvRtpDesEncryptionConstruct(RvRtpDesEncryption *thisPtr)
{
    RvLogEnter(rvLogPtr, (rvLogPtr, "rvRtpDesEncryptionConstruct"));

    rvDesEncryptionConstruct(&thisPtr->encryptor);
    rvDesEncryptionConstruct(&thisPtr->decryptor);

    rvDesEncryptionInitialize(&thisPtr->encryptor, RV_ENCRYPTIONMODE_CBC, RV_ENCRYPTIONDIRECTION_ENCRYPT);
    rvDesEncryptionInitialize(&thisPtr->decryptor, RV_ENCRYPTIONMODE_CBC, RV_ENCRYPTIONDIRECTION_DECRYPT);


    RvRtpEncryptionPlugInConstruct(&thisPtr->plugIn, (void*)thisPtr,
                                   rvRtpDesEncryptionEncryptCallback,
                                   rvRtpDesEncryptionDecryptCallback,
                                   rvRtpDesEncryptionGetBlockSizeCallback,
                                   rvRtpDesEncryptionGetHeaderSizeCallback);


    RvLogLeave(rvLogPtr, (rvLogPtr, "rvRtpDesEncryptionConstruct"));
    return thisPtr;
}

/*$
{function:
    {name: rvRtpDesEncryptionDestruct }
    {class: RvRtpDesEncryption }
    {include: rvrtpdesencryption.h}
    {description:
        {p: This method destroys the object, freeing any recources
            it held.  }
    }
    {proto: void rvRtpDesEncryptionDestruct(RvRtpDesEncryption* thisPtr);}
    {params:
        {param: {n:thisPtr} {d: The RvRtpDesEncryption object being destroyed.}}
    }
}
$*/
RVAPI
void RVCALLCONV rvRtpDesEncryptionDestruct(RvRtpDesEncryption *thisPtr)
{
    RvLogEnter(rvLogPtr, (rvLogPtr, "rvRtpDesEncryptionDestruct"));

    rvDesEncryptionDestruct(&thisPtr->encryptor);
    rvDesEncryptionDestruct(&thisPtr->decryptor);

    RvRtpEncryptionPlugInDestruct(&thisPtr->plugIn);

    RvLogLeave(rvLogPtr, (rvLogPtr, "rvRtpDesEncryptionDestruct"));
}


/*$
{function:
    {name: rvRtpDesEncryptionGetPlugIn }
    {class: RvRtpDesEncryption }
    {include: rvrtpdesencryption.h}
    {description:
        {p: This method retreives the RvRtpEncryptionPlugIn interface to be registered
            with the RvRtpSesison.}
    }
    {proto: RvRtpEncryptionPlugIn* rvRtpDesEncryptionGetPlugIn(RvRtpDesEncryption* thisPtr);}
    {params:
        {param: {n: thisPtr} {d: The RvRtpDesEncryption object.}}
    }
    {returns: The RvRtpEncryptionPlugIn interface.}
    {see_also:
        {n: RvRtpStatus rvRtpSessionRegisterEncryptionPlugIn(RvRtpSession* thisPtr, RvRtpEncryptionPlugIn* pluginPtr);}
    }
}
$*/

static void rvRtpDesEncryptionProcess(RvRtpDesEncryption* thisPtr,
								const RvUint8* srcBuffer,
								RvUint8* dstBuffer,
								RvUint32 byteLength,
                                RvRtpEncryptionData* dataPtr,
                                const RvKey* key,
                                RvDesEncryption* encryptorPtr)
{
    if(RvRtpEncryptionDataGetMode(dataPtr) == RV_RTPENCRYPTIONMODE_H235_PADDING)
    {
        RvUint8 h235Iv[8];
        RvUint16 sequenceNumber = (RvRtpEncryptionDataGetRtpHeader(dataPtr))->sequenceNumber;
        RvUint32 timeStamp      = (RvRtpEncryptionDataGetRtpHeader(dataPtr))->timestamp;

        /* Create an IV based on H.235E v2 section B.3 requirement
           of using Sequence Number + Timestamp. */
        h235Iv[0] = (RvUint8) ((sequenceNumber >> 8 )&0xFF);
        h235Iv[1] = (RvUint8) (sequenceNumber&0xFF);
        h235Iv[2] = (RvUint8) ((timeStamp >> 24)&0xFF);
        h235Iv[3] = (RvUint8) ((timeStamp >> 16)&0xFF);
        h235Iv[4] = (RvUint8) ((timeStamp >> 8 )&0xFF);
        h235Iv[5] = (RvUint8) (timeStamp&0xFF);
        h235Iv[6] = h235Iv[0];
        h235Iv[7] = h235Iv[1];

        rvDesEncryptionSetIV(encryptorPtr, h235Iv);
    }
    else if(RvRtpEncryptionDataGetMode(dataPtr) == RV_RTPENCRYPTIONMODE_RFC1889)
    {
        rvDesEncryptionSetIV(encryptorPtr,iv);
    }

    rvDesEncryptionSetKey(encryptorPtr,key);
    rvDesEncryptionReset(encryptorPtr);
    rvDesEncryptionProcess(encryptorPtr, srcBuffer, dstBuffer, byteLength);
	RV_UNUSED_ARG(thisPtr);
}


static void rvRtpDesEncryptionEncryptCallback(RvRtpEncryptionPlugIn *thisPtr,
								const RvUint8 *srcBuffer,
								RvUint8 *dstBuffer,
								RvUint32 byteLength,
                                RvRtpEncryptionData* dataPtr,
                                const  RvKey* keyPtr)
{
    RvRtpDesEncryption *plugInPtr = (RvRtpDesEncryption *)RvRtpEncryptionPlugInGetUserData(thisPtr);

    rvRtpDesEncryptionProcess(plugInPtr, srcBuffer, dstBuffer, byteLength,
                              dataPtr, keyPtr, &plugInPtr->encryptor);
}

static void rvRtpDesEncryptionDecryptCallback(RvRtpEncryptionPlugIn *thisPtr,
								const RvUint8 *srcBuffer,
								RvUint8 *dstBuffer,
								RvUint32 byteLength,
                                RvRtpEncryptionData* dataPtr,
                                const  RvKey* keyPtr)
{
    RvRtpDesEncryption *plugInPtr = (RvRtpDesEncryption *)RvRtpEncryptionPlugInGetUserData(thisPtr);

    rvRtpDesEncryptionProcess(plugInPtr, srcBuffer, dstBuffer, byteLength,
                              dataPtr, keyPtr, &plugInPtr->decryptor);
}

static RvUint32 rvRtpDesEncryptionGetBlockSizeCallback(RvRtpEncryptionPlugIn *thisPtr)
{
	RV_UNUSED_ARG(thisPtr);
    return 8;
}

static RvUint32 rvRtpDesEncryptionGetHeaderSizeCallback(RvRtpEncryptionPlugIn *thisPtr)
{
	RV_UNUSED_ARG(thisPtr);
    return 0;
}

void rvRtpDesEncryptionSetKey(RvRtpDesEncryption *thisPtr, const RvKey* keyPtr)
{
	rvDesEncryptionSetKey(&thisPtr->encryptor, keyPtr);
	rvDesEncryptionSetKey(&thisPtr->decryptor, keyPtr);

}
