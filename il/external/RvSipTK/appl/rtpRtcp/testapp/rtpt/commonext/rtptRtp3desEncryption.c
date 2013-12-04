/************************************************************************
 File Name     : rvrtp3desencryption.c
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
#include "rtptRtp3desEncryption.h"
#include "rvlog.h"
#include "rvrtpencryptiondata.h"

static RvLogSource* rvLogPtr = NULL;
/* RTP Triple DES Initialization vector */
static const RvUint8 iv[8] = {0,0,0,0,0,0,0,0};

/**********************************************************************************
* Private method declarations
**********************************************************************************/

static void rvRtp3DesEncryptionEncryptCallback(RvRtpEncryptionPlugIn *thisPtr,
								const RvUint8 *srcBuffer,
								RvUint8 *dstBuffer,
								RvUint32 byteLength,
                                RvRtpEncryptionData* dataPtr,
                                const  RvKey* keyPtr);
static void rvRtp3DesEncryptionDecryptCallback(RvRtpEncryptionPlugIn *thisPtr,
								const RvUint8 *srcBuffer,
								RvUint8 *dstBuffer,
								RvUint32 byteLength,
                                RvRtpEncryptionData* dataPtr,
                                const  RvKey* keyPtr);
static RvUint32 rvRtp3DesEncryptionGetBlockSizeCallback(RvRtpEncryptionPlugIn *thisPtr);
static RvUint32 rvRtp3DesEncryptionGetHeaderSizeCallback(RvRtpEncryptionPlugIn *thisPtr);

/*$
{function:
    {name: rvRtp3DesEncryptionConstruct }
    {class: RvRtp3DesEncryption }
    {include: rvrtp3desencryption.h}
    {description:
        {p: This method constructs a RvRtp3DesEncryption object.}
    }
    {proto: RvRtp3DesEncryption* rvRtp3DesEncryptionConstruct(RvRtp3DesEncryption* thisPtr);}
    {params:
        {param: {n:thisPtr} {d: The RvRtp3DesEncryption object being constructed.}}
    }
    {returns: A pointer to the object, if successful. NULL, otherwise.}
}
$*/
RVAPI
RvRtp3DesEncryption* RVCALLCONV rvRtp3DesEncryptionConstruct(RvRtp3DesEncryption *thisPtr)
{
    RvLogEnter(rvLogPtr, (rvLogPtr, "rvRtp3DesEncryptionConstruct"));

    rv3DesEncryptionConstruct(&thisPtr->encryptor);
    rv3DesEncryptionConstruct(&thisPtr->decryptor);

    RvRtpEncryptionPlugInConstruct(&thisPtr->plugIn, (void*)thisPtr,
                                   rvRtp3DesEncryptionEncryptCallback,
                                   rvRtp3DesEncryptionDecryptCallback,
                                   rvRtp3DesEncryptionGetBlockSizeCallback,
                                   rvRtp3DesEncryptionGetHeaderSizeCallback);

    rv3DesEncryptionInitialize(&thisPtr->encryptor, RV_ENCRYPTIONMODE_CBC, RV_ENCRYPTIONDIRECTION_ENCRYPT);
    rv3DesEncryptionInitialize(&thisPtr->decryptor, RV_ENCRYPTIONMODE_CBC, RV_ENCRYPTIONDIRECTION_DECRYPT);

    RvLogLeave(rvLogPtr, (rvLogPtr,"rvRtp3DesEncryptionConstruct"));
    return thisPtr;
}

/*$
{function:
    {name: rvRtp3DesEncryptionDestruct }
    {class: RvRtp3DesEncryption }
    {include: rvrtp3desencryption.h}
    {description:
        {p: This method destroys the object, freeing any recources
            it held.  }
    }
    {proto: void rvRtp3DesEncryptionDestruct(RvRtp3DesEncryption* thisPtr);}
    {params:
        {param: {n:thisPtr} {d: The RvRtp3DesEncryption object being destroyed.}}
    }
}
$*/
RVAPI
void RVCALLCONV rvRtp3DesEncryptionDestruct(RvRtp3DesEncryption *thisPtr)
{
    RvLogEnter(rvLogPtr, (rvLogPtr, "rvRtp3DesEncryptionDestruct"));

    rv3DesEncryptionDestruct(&thisPtr->encryptor);
    rv3DesEncryptionDestruct(&thisPtr->decryptor);

    RvRtpEncryptionPlugInDestruct(&thisPtr->plugIn);

    RvLogLeave(rvLogPtr, (rvLogPtr, "rvRtp3DesEncryptionDestruct"));
}

/*$
{function:
    {name: rvRtp3DesEncryptionGetPlugIn }
    {class: RvRtp3DesEncryption }
    {include: rvrtp3desencryption.h}
    {description:
        {p: This method retreives the RvRtpEncryptionPlugIn interface to be registered
            with the RvRtpSesison.}
    }
    {proto: RvRtpEncryptionPlugIn* rvRtp3DesEncryptionGetPlugIn(RvRtp3DesEncryption* thisPtr);}
    {params:
        {param: {n: thisPtr} {d: The RvRtp3DesEncryption object.}}
    }
    {returns: The RvRtpEncryptionPlugIn interface.}
    {see_also:
        {n: RvRtpStatus rvRtpSessionRegisterEncryptionPlugIn(RvRtpSession* thisPtr, RvRtpEncryptionPlugIn* pluginPtr);}
    }
}
$*/

static void rvRtp3DesEncryptionProcess(RvRtp3DesEncryption* thisPtr,
								const RvUint8* srcBuffer,
								RvUint8* dstBuffer,
								RvUint32 byteLength,
                                RvRtpEncryptionData* dataPtr,
                                const RvKey* key,
                                Rv3DesEncryption* encryptorPtr)
{
    if(RvRtpEncryptionDataGetMode(dataPtr) == RV_RTPENCRYPTIONMODE_H235_PADDING)
    {
        RvUint8 h235Iv[8];
        RvUint16 sequenceNumber = (RvRtpEncryptionDataGetRtpHeader(dataPtr))->sequenceNumber;
        RvUint32 timeStamp      = (RvRtpEncryptionDataGetRtpHeader(dataPtr))->timestamp;

        /* Create an IV based on H.235E v2 section B.3 requirement
           of using Sequence Number + Timestamp. */
        h235Iv[0] = (RvUint8)((sequenceNumber >> 8 )&0xFF);
        h235Iv[1] = (RvUint8)(sequenceNumber&0xFF);
        h235Iv[2] = (RvUint8)((timeStamp >> 24)&0xFF);
        h235Iv[3] = (RvUint8)((timeStamp >> 16)&0xFF);
        h235Iv[4] = (RvUint8)((timeStamp >> 8 )&0xFF);
        h235Iv[5] = (RvUint8)(timeStamp&0xFF);
        h235Iv[6] = h235Iv[0];
        h235Iv[7] = h235Iv[1];

        rv3DesEncryptionSetIV(encryptorPtr, h235Iv);
    }
    else if(RvRtpEncryptionDataGetMode(dataPtr) == RV_RTPENCRYPTIONMODE_RFC1889)
    {
        rv3DesEncryptionSetIV(encryptorPtr,iv);
    }

    rv3DesEncryptionSetKey(encryptorPtr,key);
    rv3DesEncryptionReset(encryptorPtr);
    rv3DesEncryptionProcess(encryptorPtr, srcBuffer, dstBuffer, byteLength);

	RV_UNUSED_ARG(thisPtr);
}

static void rvRtp3DesEncryptionEncryptCallback(RvRtpEncryptionPlugIn *thisPtr,
								const RvUint8 *srcBuffer,
								RvUint8 *dstBuffer,
								RvUint32 byteLength,
                                RvRtpEncryptionData* dataPtr,
                                const  RvKey* keyPtr)
{
    RvRtp3DesEncryption *plugInPtr = (RvRtp3DesEncryption *)RvRtpEncryptionPlugInGetUserData(thisPtr);

    rvRtp3DesEncryptionProcess(plugInPtr,srcBuffer,dstBuffer,byteLength,
                               dataPtr, keyPtr, &plugInPtr->encryptor);
}

static void rvRtp3DesEncryptionDecryptCallback(RvRtpEncryptionPlugIn *thisPtr,
								const RvUint8 *srcBuffer,
								RvUint8 *dstBuffer,
								RvUint32 byteLength,
                                RvRtpEncryptionData* dataPtr,
                                const  RvKey* keyPtr)
{
    RvRtp3DesEncryption *plugInPtr = (RvRtp3DesEncryption *)RvRtpEncryptionPlugInGetUserData(thisPtr);

    rvRtp3DesEncryptionProcess(plugInPtr,srcBuffer,dstBuffer,byteLength,
                               dataPtr, keyPtr, &plugInPtr->decryptor);
	RV_UNUSED_ARG(thisPtr);
}

static RvUint32 rvRtp3DesEncryptionGetBlockSizeCallback(RvRtpEncryptionPlugIn *thisPtr)
{
	RV_UNUSED_ARG(thisPtr);
    return 8;
}

static RvUint32 rvRtp3DesEncryptionGetHeaderSizeCallback(RvRtpEncryptionPlugIn *thisPtr)
{
	RV_UNUSED_ARG(thisPtr);
    return 0;
}


