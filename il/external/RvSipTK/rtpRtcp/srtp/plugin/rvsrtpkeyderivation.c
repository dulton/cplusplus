/***********************************************************************
Filename   : rvsrtpkeyderivation.c
Description:
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

#include "rvsrtpkeyderivation.h"
#include "rvsrtpaescm.h"
#include "rv64ascii.h"
#include "rvsrtplog.h"

/*-----------------------------------------------------------------------*/
/*                   internal function                                   */
/*-----------------------------------------------------------------------*/

/*************************************************************************
* rvSrtpKeyDerivationSrtpKeysDerive
* ------------------------------------------------------------------------
* General: this function creates the SRTP session keys, from the master
*          key and master salt
*
* Return Value: RvRtpStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    thisPtr - pointer to the RvSrtpKeyDerivation object
*           contextPtr - pointer to the current context
*           thisPtr - pointer to the current index
*
* Output    None
*************************************************************************/
static RvRtpStatus rvSrtpKeyDerivationKeysDerive
(
    RvSrtpKeyDerivation *thisPtr,
    RvSrtpAesPlugIn* plugin,
    RvSrtpEncryptionPurpose purpose,
    RvSrtpContext       *contextPtr,
    RvUint64            index
)
{
    RvRtpStatus rc;
    RvSrtpKey *keyInfo;
    RvUint8   *masterKey;
    RvUint8   *masterSalt;
    RvSize_t   masterKeySize;
    RvSize_t   masterSaltSize;
    RvUint8   *key;
	RvSize_t   keySize;
	RvSize_t   keyDerivationRate;
	RvInt      encryptType;
	RvInt      authType;
	RvInt      saltType;

	/* Set proper type constants for RTP or RTCP */
	if(rvSrtpDbContextGetIsRtp(contextPtr) == RV_TRUE) {
		encryptType = RV_SRTP_AESCMKEYTYPE_RTPENCRYPT;
		authType = RV_SRTP_AESCMKEYTYPE_RTPAUTH;
		saltType = RV_SRTP_AESCMKEYTYPE_RTPSALT;
	} else {
		encryptType = RV_SRTP_AESCMKEYTYPE_RTCPENCRYPT;
		authType = RV_SRTP_AESCMKEYTYPE_RTCPAUTH;
		saltType = RV_SRTP_AESCMKEYTYPE_RTCPSALT;
	}

    /* get master key and master salt information */
    keyInfo = rvSrtpDbContextGetMasterKey(contextPtr);
    masterKey = rvSrtpDbKeyGetMasterKey(keyInfo);
    masterKeySize = rvSrtpDbGetMasterKeySize(thisPtr->dbPtr);
    masterSalt = rvSrtpDbKeyGetMasterSalt(keyInfo);
    masterSaltSize = rvSrtpDbGetMasterSaltSize(thisPtr->dbPtr);
	keyDerivationRate = rvSrtpDbContextGetKeyDerivationRate(contextPtr);

	keySize = rvSrtpDbGetSrtpEncryptKeySize(thisPtr->dbPtr);
    key = rvSrtpDbContextGetCurrentEncryptKey(contextPtr);
	rc = rvSrtpAesCmCreateKey(plugin, purpose, encryptType, key, keySize, masterKey,
                              masterKeySize, masterSalt, masterSaltSize,
                              keyDerivationRate, index);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
        return RV_RTPSTATUS_NotFound;
    }

    keySize = rvSrtpDbGetSrtpAuthKeySize(thisPtr->dbPtr);
    key = rvSrtpDbContextGetCurrentAuthKey(contextPtr);
    if (NULL == key)
    {
        return RV_RTPSTATUS_NotFound;
    }
    rc = rvSrtpAesCmCreateKey(plugin, purpose, authType, key, keySize, masterKey,
                              masterKeySize, masterSalt, masterSaltSize,
                              keyDerivationRate, index);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
        return RV_RTPSTATUS_NotFound;
    }

    keySize = rvSrtpDbGetSrtpSaltSize(thisPtr->dbPtr);
    key = rvSrtpDbContextGetCurrentSalt(contextPtr);
    if (NULL == key)
    {
        return RV_RTPSTATUS_NotFound;
    }
    rc = rvSrtpAesCmCreateKey(plugin, purpose, saltType, key, keySize, masterKey,
                              masterKeySize, masterSalt, masterSaltSize,
                              keyDerivationRate, index);
    if (RV_RTPSTATUS_Succeeded != rc)
    {
        return rc;
    }

    return RV_RTPSTATUS_Succeeded;
}

/*-----------------------------------------------------------------------*/
/*                   external function                                   */
/*-----------------------------------------------------------------------*/

/*************************************************************************
* rvSrtpKeyDerivationConstruct
* ------------------------------------------------------------------------
* General: construction - initializing all the RvSrtpKeyDerivation parameters
*
* Return Value: RvSrtpKeyDerivation*
* ------------------------------------------------------------------------
* Arguments:
* Input:    thisPtr - pointer to the RvSrtpKeyDerivation object
*           dbPtr - pointer to the RvSrtpDb object
*           keyDerivationAlg - the key derivation PRF
*
* Output    thisPtr - pointer to the RvSrtpKeyDerivation object
*************************************************************************/
RvSrtpKeyDerivation *rvSrtpKeyDerivationConstruct
(
    RvSrtpKeyDerivation *thisPtr,
    RvSrtpDb            *dbPtr,
    RvInt               keyDerivationAlg
)
{
    thisPtr->dbPtr = dbPtr;
    thisPtr->keyDerivationAlg = keyDerivationAlg;

    return thisPtr;
}

RvRtpStatus rvSrtpKeyDerivationUpdateContext
(
    RvSrtpKeyDerivation *thisPtr,
    RvSrtpAesPlugIn* plugin,
    RvSrtpEncryptionPurpose purpose,
    RvSrtpContext       *contextPtr,
    RvUint64             index,
    RvBool              *currentKeys
)
{
    RvRtpStatus rc;
	RvBool doTriggers;
	RvChar tmpBuf[RV_64TOASCII_BUFSIZE];

    if (NULL == contextPtr)
    {
        return RV_RTPSTATUS_NullPointer;
	}

	/* Check to see if master key is still usable */
	if(rvSrtpDbContextGetMaxLimit(contextPtr) == RV_TRUE)
	{
		rvSrtpLogWarning((rvSrtpLogSourcePtr, "rvSrtpKeyDerivationUpdateContext: master key has hit maximum usage limit"));
		return RV_RTPSTATUS_Failed;
	}

	/* Update the key buffers and find out if keys can be aquired for this index */
	if(rvSrtpDbContextUpdateKeys(contextPtr, index, currentKeys) == RV_FALSE) {
		/* No keys are available or can be generated for this index */
		rvSrtpLogWarning((rvSrtpLogSourcePtr, "rvSrtpKeyDerivationUpdateContext: no session keys available"));
		return RV_RTPSTATUS_Failed;
	}

	/* If we using the old keys or we have current keys, we are done */
	if((*currentKeys == RV_FALSE) || (rvSrtpDbContextHasCurrentKeys(contextPtr) == RV_TRUE))
		return RV_RTPSTATUS_Succeeded;

	/* Remember if we have to check for triggers */
	doTriggers = rvSrtpDbContextGetNeverUsed(contextPtr);

	/* Generate and enable the current keys */
	rc = rvSrtpKeyDerivationKeysDerive( thisPtr, plugin, purpose, contextPtr, index);
	if(rc != RV_RTPSTATUS_Succeeded) {
		/* The stream will get distrupted, but there's nothing we can do */
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpKeyDerivationUpdateContext: session key derivation failed"));
		return rc;
	}
	rvSrtpDbContextEnableCurrentKeys(contextPtr, index);
	rvSrtpLogInfo((rvSrtpLogSourcePtr, "rvSrtpKeyDerivationUpdateContext: new session keys generated at index %s", Rv64UtoA(index, tmpBuf)));

	/* If this is the first use of the context, do triggers if necessary. */
   if((doTriggers == RV_TRUE) && (rvSrtpDbContextTrigger(contextPtr) == RV_TRUE)) {
		rvSrtpLogDebug((rvSrtpLogSourcePtr, "rvSrtpKeyDerivationUpdateContext: trigger encountered"));
   }

    RV_UNUSED_ARG(tmpBuf);
	return RV_RTPSTATUS_Succeeded;
}

#if RV_TEST_CODE
RvUint8 masterSaltKey[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                       0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
RvUint16 masterSaltKey_len = 112;

RvUint8 masterEncKey[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
RvUint16 masterEncKey_len = 128;

RvRtpStatus RvSrtpKeyDerivationTest(RvSrtpDb *dbPtr, RvSrtpContext *contextPtr)
{
    RvSrtpKeyDerivation keyDer;
    RvUint64            index = 0x1;

    rvSrtpKeyDerivationConstruct(&keyDer, dbPtr, RV_SRTP_DERIVATION_AESCM);
    rvSrtpKeyDerivationUpdateContext(&keyDer, contextPtr, index);
    rvSrtpKeyDerivationDestruct(&keyDer);

    return RV_RTPSTATUS_Succeeded;
}

#endif /* SRTP_TEST_ON */
