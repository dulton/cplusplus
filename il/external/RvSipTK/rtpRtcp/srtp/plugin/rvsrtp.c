/************************************************************************
 File Name	   : rvsrtp.c
 Description   :
*************************************************************************
 Copyright (c)	2005 RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVision Inc.
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVision Inc.

 RADVision Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
*************************************************************************/
#include "rvsrtp.h"
#include "rv_srtp.h"
#include "rvsrtplog.h"
#include "RtcpTypes.h"

extern RvStatus rvRtpSessionRegisterEncryptionPlugIn(
        IN RvRtpSession session,
        IN RvSrtpEncryptionPlugIn *plugInPtr);


/* Encryption plugin version: SRTP/SRTCP for RTP/RTCP  */
static const RvUint8 RvSrtpMajorVersion       = 1;  /* The plugin's major version number. */
static const RvUint8 RvSrtpMinorVersion       = 0;  /* The plugin's minor version number. */
static const RvUint8 RvSrtpEngineeringRelease = 4;  /* The plugin's engineering release number. */
static const RvUint8 RvSrtpPatchLevel         = 0;  /* The plugin's patch level. */

/* Key Derivation defaults */
#define RV_SRTP_DEFAULT_KEYDERIVATIONRATE 0
#define RV_SRTP_DEFAULT_KEYDERIVATIONALG  RV_SRTP_KEYDERIVATIONALG_AESCM

/* Processing defaults */
#define RV_SRTP_DEFAULT_PREFIXLEN 0
#define RV_SRTP_DEFAULT_RTPENCALG RV_SRTP_ENCYRPTIONALG_AESCM
#define RV_SRTP_DEFAULT_RTPUSEMKI RV_TRUE
#define RV_SRTP_DEFAULT_RTPAUTHALG RV_SRTP_AUTHENTICATIONALG_HMACSHA1
#define RV_SRTP_DEFAULT_RTPAUTHTAGSIZE 10
#define RV_SRTP_DEFAULT_RTCPENCALG RV_SRTP_ENCYRPTIONALG_AESCM
#define RV_SRTP_DEFAULT_RTCPUSEMKI RV_TRUE
#define RV_SRTP_DEFAULT_RTCPAUTHALG RV_SRTP_AUTHENTICATIONALG_HMACSHA1
#define RV_SRTP_DEFAULT_RTCPAUTHTAGSIZE 10

/* DB defaults */
#define RV_SRTP_DEFAULT_RTPENCRYPTKEYSIZE  16
#define RV_SRTP_DEFAULT_RTPAUTHKEYSIZE     20
#define RV_SRTP_DEFAULT_RTPSALTSIZE        14
#define RV_SRTP_DEFAULT_RTCPENCRYPTKEYSIZE 16
#define RV_SRTP_DEFAULT_RTCPAUTHKEYSIZE    20
#define RV_SRTP_DEFAULT_RTCPSALTSIZE       14
#define RV_SRTP_DEFAULT_RTPREPLAYLISTSIZE  64
#define RV_SRTP_DEFAULT_RTCPREPLAYLISTSIZE 64
#define RV_SRTP_DEFAULT_RTPHISTORYSIZE     65536
#define RV_SRTP_DEFAULT_RTCPHISTORYSIZE    65536

/* DB Pool configuration defaults */
#define RV_SRTP_DBDEFAULT_KEYREGION           NULL
#define RV_SRTP_DBDEFAULT_KEYPAGEITEMS        10
#define RV_SRTP_DBDEFAULT_KEYPAGESIZE         0
#define RV_SRTP_DBDEFAULT_KEYPOOLTYPE         RV_OBJPOOL_TYPE_EXPANDING
#define RV_SRTP_DBDEFAULT_KEYMAXITEMS         0
#define RV_SRTP_DBDEFAULT_KEYMINITEMS         0
#define RV_SRTP_DBDEFAULT_KEYFREELEVEL        0
#define RV_SRTP_DBDEFAULT_STREAMREGION        NULL
#define RV_SRTP_DBDEFAULT_STREAMPAGEITEMS     20
#define RV_SRTP_DBDEFAULT_STREAMPAGESIZE      0
#define RV_SRTP_DBDEFAULT_STREAMPOOLTYPE      RV_OBJPOOL_TYPE_EXPANDING
#define RV_SRTP_DBDEFAULT_STREAMMAXITEMS      0
#define RV_SRTP_DBDEFAULT_STREAMMINITEMS      0
#define RV_SRTP_DBDEFAULT_STREAMFREELEVEL     0
#define RV_SRTP_DBDEFAULT_CONTEXTREGION       NULL
#define RV_SRTP_DBDEFAULT_CONTEXTPAGEITEMS    40
#define RV_SRTP_DBDEFAULT_CONTEXTPAGESIZE     0
#define RV_SRTP_DBDEFAULT_CONTEXTPOOLTYPE     RV_OBJPOOL_TYPE_EXPANDING
#define RV_SRTP_DBDEFAULT_CONTEXTMAXITEMS     0
#define RV_SRTP_DBDEFAULT_CONTEXTMINITEMS     0
#define RV_SRTP_DBDEFAULT_CONTEXTFREELEVEL    0
#define RV_SRTP_DBDEFAULT_KEYHASHREGION       NULL
#define RV_SRTP_DBDEFAULT_KEYHASHSTARTSIZE    17
#define RV_SRTP_DBDEFAULT_KEYHASHTYPE         RV_OBJPOOL_TYPE_EXPANDING
#define RV_SRTP_DBDEFAULT_SOURCEHASHREGION    NULL
#define RV_SRTP_DBDEFAULT_SOURCEHASHSTARTSIZE 17
#define RV_SRTP_DBDEFAULT_SOURCEHASHTYPE      RV_OBJPOOL_TYPE_EXPANDING
#define RV_SRTP_DBDEFAULT_DESTHASHREGION      NULL
#define RV_SRTP_DBDEFAULT_DESTHASHSTARTSIZE   17
#define RV_SRTP_DBDEFAULT_DESTHASHTYPE        RV_OBJPOOL_TYPE_EXPANDING

/* Convert RTP sequence number and ROC to index */
#define rvSrtpRtpSeqNumToIndex(seqNum, roc) (((RvUint64)(seqNum)) | ((RvUint64)(roc) << 16));

/* Convert RTCP index to our 31 bit index (RTCP stack sends 32 bits) */
#define rvSrtpRtcpIndexConvert(index) ((index) & RvUint32Const(0x7FFFFFFF))

static RvRtpStatus rvSrtpRegistered(RvRtpSession *thisPtr, void *encryptionPluginData, RvBool enableRtp, RvBool enableRtcp, RvUint32 ssrc, RvUint16 rtpSeqNum, RvUint32 rtpRoc, RvUint32 rtcpIndex);
static RvRtpStatus rvSrtpUnregistered(RvRtpSession *thisPtr, void *encryptionPluginData);
static RvRtpStatus rvSrtpOpen(RvRtpSession *thisPtr, void *encryptionPluginData);
static RvRtpStatus rvSrtpClose(RvRtpSession *thisPtr, void *encryptionPluginData);
static RvRtpStatus rvSrtpEncryptRtp(RvRtpSession thisPtr, void *encryptionPluginData, RvDataBuffer *srcBuffer, RvDataBuffer *dstBuffer, RvAddress *srcAddress, RvAddress *dstAddress, RvUint32 roc);
static RvRtpStatus rvSrtpEncryptRtcp(RvRtpSession thisPtr, void *encryptionPluginData, RvDataBuffer *srcBuffer, RvDataBuffer *dstBuffer, RvAddress *srcAddress, RvAddress *dstAddress, RvUint32 index, RvBool encrypt);
static RvRtpStatus rvSrtpDecryptRtp(RvRtpSession thisPtr, void *encryptionPluginData, RvDataBuffer *srcBuffer, RvDataBuffer *dstBuffer, RvAddress *srcAddress, RvAddress *dstAddress);
static RvRtpStatus rvSrtpDecryptRtcp(RvRtpSession thisPtr, void *encryptionPluginData, RvDataBuffer *srcBuffer, RvDataBuffer *dstBuffer, RvAddress *srcAddress, RvAddress *dstAddress);
static RvBool rvSrtpValidateSsrc(RvRtpSession *thisPtr, void *encryptionPluginData, RvUint32 ssrc);
static RvRtpStatus rvSrtpSsrcChanged(RvRtpSession *thisPtr, void *encryptionPluginData, RvUint32 oldSsrc, RvUint32 newSsrc);
static RvUint32 rvSrtpGetPaddingSize(RvRtpSession *thisPtr, void *encryptionPluginData, RvBool forRtp);
static RvUint32 rvSrtpGetHeaderSize(RvRtpSession *thisPtr, void *encryptionPluginData, RvBool forRtp);
static RvUint32 rvSrtpGetFooterSize(RvRtpSession *thisPtr, void *encryptionPluginData, RvBool forRtp);
static RvUint32 rvSrtpGetTransmitSize(RvRtpSession *thisPtr, void *encryptionPluginData, RvUint32 packetSize, RvUint32 rtpHeaderSize, RvBool encrypt, RvBool forRtp);
static void rvSrtpRtpSeqNumChanged(RvRtpSession* thisPtr, void *encryptionPluginData, RvUint16 sequenceNum, RvUint32 roc);
static void rvSrtpRtcpIndexChanged(RvRtcpSession* thisPtr, void *encryptionPluginData, RvUint32 index);
static RvAddress *rvSrtpAddressConstructByName(RvAddress *addressPtr, RvChar *addressString, RvUint16 port);


/* Basic operations */
RvRtpStatus rvSrtpConstruct(RvSrtp *thisPtr
#ifdef UPDATED_BY_SPIRENT
        ,RvSrtpMasterKeyEventCB masterKeyEventCB
#endif // UPDATED_BY_SPIRENT
)
{
	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpConstruct"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpConstruct: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpConstruct"));
		return RV_RTPSTATUS_Failed;
	}

	if(RvMutexConstruct(NULL, &thisPtr->srtpLock) != RV_OK) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpConstruct: Lock construct failed"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpConstruct"));
		return RV_RTPSTATUS_Failed;
	}

	RvMutexLock(&thisPtr->srtpLock, NULL);

	/* Set things that need to be initialized */
	thisPtr->rtpSession = NULL;
	thisPtr->rtcpSession = NULL;
	thisPtr->registering = RV_FALSE;
	thisPtr->ourSsrc = RvUint32Const(0);
   //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
   thisPtr->ourRtpIndex = 0;
#else
	thisPtr->ourRtpIndex = RvUint64Const2(0);
#endif
   //SPIRENT_END
	thisPtr->ourRtcpIndex = RvUint32Const(0);

	/* Initialize configuration defaults */
#ifndef UPDATED_BY_SPIRENT
	thisPtr->keyDerivationRate = RV_SRTP_DEFAULT_KEYDERIVATIONRATE;
#else
	thisPtr->keyDerivationRateLocal = RV_SRTP_DEFAULT_KEYDERIVATIONRATE;
	thisPtr->keyDerivationRateRemote = RV_SRTP_DEFAULT_KEYDERIVATIONRATE;
#endif // UPDATED_BY_SPIRENT

	/* Initialize Master Key configuration defaults */
	thisPtr->mkiSize = RV_SRTP_DEFAULT_MKISIZE;
	thisPtr->keySize = RV_SRTP_DEFAULT_MASTERKEYSIZE;
	thisPtr->saltSize = RV_SRTP_DEFAULT_SALTSIZE;

	/* Initialize Key Derivation configuration defaults */
	thisPtr->keyDerivationAlg = RV_SRTP_DEFAULT_KEYDERIVATIONALG;

	/* Initialize Processing configuration defaults */
	thisPtr->prefixLen =       RV_SRTP_DEFAULT_PREFIXLEN;
	thisPtr->rtpEncAlg =       RV_SRTP_DEFAULT_RTPENCALG;
	thisPtr->rtpUseMki =       RV_SRTP_DEFAULT_RTPUSEMKI;
	thisPtr->rtpAuthAlg =      RV_SRTP_DEFAULT_RTPAUTHALG;
	thisPtr->rtpAuthTagSize =  RV_SRTP_DEFAULT_RTPAUTHTAGSIZE;
	thisPtr->rtcpEncAlg =      RV_SRTP_DEFAULT_RTCPENCALG;
	thisPtr->rtcpUseMki =      RV_SRTP_DEFAULT_RTCPUSEMKI;
	thisPtr->rtcpAuthAlg =     RV_SRTP_DEFAULT_RTCPAUTHALG;
	thisPtr->rtcpAuthTagSize = RV_SRTP_DEFAULT_RTCPAUTHTAGSIZE;

	/* Initialize DB configuration defaults */
	thisPtr->rtpEncryptKeySize =  RV_SRTP_DEFAULT_RTPENCRYPTKEYSIZE;
	thisPtr->rtpAuthKeySize =     RV_SRTP_DEFAULT_RTPAUTHKEYSIZE;
	thisPtr->rtpSaltSize =        RV_SRTP_DEFAULT_RTPSALTSIZE;
	thisPtr->rtcpEncryptKeySize = RV_SRTP_DEFAULT_RTCPENCRYPTKEYSIZE;
	thisPtr->rtcpAuthKeySize =    RV_SRTP_DEFAULT_RTCPAUTHKEYSIZE;
	thisPtr->rtcpSaltSize =       RV_SRTP_DEFAULT_RTCPSALTSIZE;
	thisPtr->rtpReplayListSize =  RV_SRTP_DEFAULT_RTPREPLAYLISTSIZE;
	thisPtr->rtcpReplayListSize = RV_SRTP_DEFAULT_RTCPREPLAYLISTSIZE;
	thisPtr->rtpHistorySize =     RV_SRTP_DEFAULT_RTPHISTORYSIZE;
	thisPtr->rtcpHistorySize =    RV_SRTP_DEFAULT_RTCPHISTORYSIZE;

	/* Initialize DB pool configuration defaults */
	thisPtr->poolConfig.keyRegion =           RV_SRTP_DBDEFAULT_KEYREGION;
	thisPtr->poolConfig.keyPageItems =        RV_SRTP_DBDEFAULT_KEYPAGEITEMS;
	thisPtr->poolConfig.keyPageSize =         RV_SRTP_DBDEFAULT_KEYPAGESIZE;
	thisPtr->poolConfig.keyPoolType =         RV_SRTP_DBDEFAULT_KEYPOOLTYPE;
	thisPtr->poolConfig.keyMaxItems =         RV_SRTP_DBDEFAULT_KEYMAXITEMS;
	thisPtr->poolConfig.keyMinItems =         RV_SRTP_DBDEFAULT_KEYMINITEMS;
	thisPtr->poolConfig.keyFreeLevel =        RV_SRTP_DBDEFAULT_KEYFREELEVEL;
	thisPtr->poolConfig.streamRegion =        RV_SRTP_DBDEFAULT_STREAMREGION;
	thisPtr->poolConfig.streamPageItems =     RV_SRTP_DBDEFAULT_STREAMPAGEITEMS;
	thisPtr->poolConfig.streamPageSize =      RV_SRTP_DBDEFAULT_STREAMPAGESIZE;
	thisPtr->poolConfig.streamPoolType =      RV_SRTP_DBDEFAULT_STREAMPOOLTYPE;
	thisPtr->poolConfig.streamMaxItems =      RV_SRTP_DBDEFAULT_STREAMMAXITEMS;
	thisPtr->poolConfig.streamMinItems =      RV_SRTP_DBDEFAULT_STREAMMINITEMS;
	thisPtr->poolConfig.streamFreeLevel =     RV_SRTP_DBDEFAULT_STREAMFREELEVEL;
	thisPtr->poolConfig.contextRegion =       RV_SRTP_DBDEFAULT_CONTEXTREGION;
	thisPtr->poolConfig.contextPageItems =    RV_SRTP_DBDEFAULT_CONTEXTPAGEITEMS;
	thisPtr->poolConfig.contextPageSize =     RV_SRTP_DBDEFAULT_CONTEXTPAGESIZE;
	thisPtr->poolConfig.contextPoolType =     RV_SRTP_DBDEFAULT_CONTEXTPOOLTYPE;
	thisPtr->poolConfig.contextMaxItems =     RV_SRTP_DBDEFAULT_CONTEXTMAXITEMS;
	thisPtr->poolConfig.contextMinItems =     RV_SRTP_DBDEFAULT_CONTEXTMINITEMS;
	thisPtr->poolConfig.contextFreeLevel =    RV_SRTP_DBDEFAULT_CONTEXTFREELEVEL;
	thisPtr->poolConfig.keyHashRegion =       RV_SRTP_DBDEFAULT_KEYHASHREGION;
	thisPtr->poolConfig.keyHashStartSize =    RV_SRTP_DBDEFAULT_KEYHASHSTARTSIZE;
	thisPtr->poolConfig.keyHashType =         RV_SRTP_DBDEFAULT_KEYHASHTYPE;
	thisPtr->poolConfig.sourceHashRegion =    RV_SRTP_DBDEFAULT_SOURCEHASHREGION;
	thisPtr->poolConfig.sourceHashStartSize = RV_SRTP_DBDEFAULT_SOURCEHASHSTARTSIZE;
	thisPtr->poolConfig.sourceHashType =      RV_SRTP_DBDEFAULT_SOURCEHASHTYPE;
	thisPtr->poolConfig.destHashRegion =      RV_SRTP_DBDEFAULT_DESTHASHREGION;
	thisPtr->poolConfig.destHashStartSize =   RV_SRTP_DBDEFAULT_DESTHASHSTARTSIZE;
	thisPtr->poolConfig.destHashType =        RV_SRTP_DBDEFAULT_DESTHASHTYPE;

	
	/* Initialize callbacks*/
#ifdef UPDATED_BY_SPIRENT
    /* Callbacks */
	thisPtr->masterKeyEventCB = masterKeyEventCB; /* Call back when master the key deletion is */
#endif // UPDATED_BY_SPIRENT

	
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpConstruct"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpDestruct(RvSrtp *thisPtr)
{
	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpDestruct"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestruct: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestruct"));
		return RV_RTPSTATUS_Failed;
	}

	/* Make sure we're unregistered so everything's cleaned up */
	rvSrtpUnregister(thisPtr);

	RvMutexDestruct(&thisPtr->srtpLock, NULL);
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestruct"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpRegister(RvSrtp *thisPtr, RvRtpSession rtpSession, RvSrtpAesPlugIn* AESPlugin)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpRegister"));

	if((thisPtr == NULL) || (rtpSession == NULL)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRegister: NULL SRTP or session object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRegister"));
		return RV_RTPSTATUS_Failed;
	}

	RvMutexLock(&thisPtr->srtpLock, NULL);
	if((thisPtr->rtpSession != NULL) || (thisPtr->registering == RV_TRUE)) {
		RvMutexUnlock(&thisPtr->srtpLock, NULL);
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRegister: SRTP Plugin already registered"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRegister"));
		return RV_RTPSTATUS_Failed;
	}

	/* Construct the rest of the system */
	if(rvSrtpDbConstruct(&thisPtr->db, thisPtr->keySize, thisPtr->saltSize, thisPtr->mkiSize, thisPtr->rtpEncryptKeySize, thisPtr->rtpAuthKeySize, thisPtr->rtpSaltSize, thisPtr->rtcpEncryptKeySize, thisPtr->rtcpAuthKeySize, thisPtr->rtcpSaltSize, thisPtr->rtpReplayListSize, thisPtr->rtcpReplayListSize, thisPtr->rtpHistorySize, thisPtr->rtcpHistorySize, &thisPtr->poolConfig) == NULL) {
		RvMutexUnlock(&thisPtr->srtpLock, NULL);
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRegister: Database construction failure"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRegister"));
		return RV_RTPSTATUS_Failed;
	}
	if(rvSrtpKeyDerivationConstruct(&thisPtr->keyDerivation, &thisPtr->db, thisPtr->keyDerivationAlg) == NULL) {
		rvSrtpDbDestruct(&thisPtr->db);
		RvMutexUnlock(&thisPtr->srtpLock, NULL);
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRegister: Key derivation construction failure"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRegister"));
		return RV_RTPSTATUS_Failed;
	}
	if(rvSrtpProcessConstruct(&thisPtr->packetProcess, &thisPtr->db, &thisPtr->keyDerivation, AESPlugin) != RV_RTPSTATUS_Succeeded) {
		rvSrtpKeyDerivationDestruct(&thisPtr->keyDerivation);
		rvSrtpDbDestruct(&thisPtr->db);
		RvMutexUnlock(&thisPtr->srtpLock, NULL);
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRegister: Process construction failure"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRegister"));
		return RV_RTPSTATUS_Failed;
	}
	if(rvSrtpSsrcValidationConstruct(&thisPtr->ssrcValidation, &thisPtr->db, RV_TRUE, RV_TRUE) == NULL) {
		rvSrtpProcessDestruct(&thisPtr->packetProcess);
		rvSrtpKeyDerivationDestruct(&thisPtr->keyDerivation);
		rvSrtpDbDestruct(&thisPtr->db);
		RvMutexUnlock(&thisPtr->srtpLock, NULL);
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRegister: SSRC validation construction failure"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRegister"));
		return RV_RTPSTATUS_Failed;
	}

	/* Fill in plugIn structure */
	thisPtr->plugIn.registerPlugin = rvSrtpRegistered;
	thisPtr->plugIn.unregisterPlugin = rvSrtpUnregistered;
	thisPtr->plugIn.sessionOpen = rvSrtpOpen;
	thisPtr->plugIn.sessionClose = rvSrtpClose;
	thisPtr->plugIn.encryptRtp = rvSrtpEncryptRtp;
	thisPtr->plugIn.encryptRtcp = rvSrtpEncryptRtcp;
	thisPtr->plugIn.decryptRtp = rvSrtpDecryptRtp;
	thisPtr->plugIn.decryptRtcp = rvSrtpDecryptRtcp;
	thisPtr->plugIn.validateSsrc = rvSrtpValidateSsrc;
	thisPtr->plugIn.ssrcChanged = rvSrtpSsrcChanged;
	thisPtr->plugIn.getPaddingSize = rvSrtpGetPaddingSize;
	thisPtr->plugIn.getHeaderSize = rvSrtpGetHeaderSize;
	thisPtr->plugIn.getFooterSize = rvSrtpGetFooterSize;
	thisPtr->plugIn.getTransmitSize = rvSrtpGetTransmitSize;
	thisPtr->plugIn.rtpSeqNumChanged = rvSrtpRtpSeqNumChanged;
	thisPtr->plugIn.rtcpIndexChanged = rvSrtpRtcpIndexChanged;
    thisPtr->plugIn.userData = (void*)thisPtr;

	/* We can't be locked when calling RTP functions in order to prevent deadlocks. */
	thisPtr->registering = RV_TRUE;
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
	status = rvRtpSessionRegisterEncryptionPlugIn(rtpSession, &thisPtr->plugIn);
	RvMutexLock(&thisPtr->srtpLock, NULL);
	thisPtr->registering = RV_FALSE;

	/* Additional Configuration */
	if(status == RV_RTPSTATUS_Succeeded)
		status = rvSrtpProcessSetEncryption(&thisPtr->packetProcess, thisPtr->rtpEncAlg, thisPtr->rtcpEncAlg);
	if(status == RV_RTPSTATUS_Succeeded)
		status = rvSrtpProcessSetAuthentication(&thisPtr->packetProcess, thisPtr->rtpAuthAlg, thisPtr->rtpAuthTagSize, thisPtr->rtcpAuthAlg, thisPtr->rtcpAuthTagSize);
	if(status == RV_RTPSTATUS_Succeeded)
		status = rvSrtpProcessUseMki(&thisPtr->packetProcess, thisPtr->rtpUseMki, thisPtr->rtcpUseMki);
	if(status == RV_RTPSTATUS_Succeeded)
		status = rvSrtpProcessSetPrefixLength(&thisPtr->packetProcess, thisPtr->prefixLen);

	/* Make sure everything worked. */
	if(status != RV_RTPSTATUS_Succeeded) {
		rvSrtpSsrcValidationDestruct(&thisPtr->ssrcValidation);
		rvSrtpProcessDestruct(&thisPtr->packetProcess);
		rvSrtpKeyDerivationDestruct(&thisPtr->keyDerivation);
		rvSrtpDbDestruct(&thisPtr->db);
		RvMutexUnlock(&thisPtr->srtpLock, NULL);
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRegister: Could not register or configure plugin"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRegister"));
		return status;
	}

	thisPtr->rtpSession = rtpSession;
    thisPtr->rtcpSession = RvRtpGetRTCPSession(rtpSession);
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRegister"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpUnregister(RvSrtp *thisPtr)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpUnregister"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpUnregister: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpUnregister"));
		return RV_RTPSTATUS_Failed;
	}

	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession == NULL) {
		RvMutexUnlock(&thisPtr->srtpLock, NULL);
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpUnregister: SRTP not registered"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpUnregister"));
		return RV_RTPSTATUS_Succeeded;
	}

	/* We can't be locked when calling RTP functions in order to prevent deadlocks. */
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
	status = rvRtpSessionRegisterEncryptionPlugIn(thisPtr->rtpSession, NULL);
	RvMutexLock(&thisPtr->srtpLock, NULL);

	/* Make sure unregister worked. */
	if(status != RV_RTPSTATUS_Succeeded) {
		RvMutexUnlock(&thisPtr->srtpLock, NULL);
		return status;
	}

	/* Just in case unregister was called multiple times simultaneously */
	if(thisPtr->rtpSession == NULL) {
		RvMutexUnlock(&thisPtr->srtpLock, NULL);
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpUnregister"));
		return RV_RTPSTATUS_Succeeded;
	}

	rvSrtpSsrcValidationDestruct(&thisPtr->ssrcValidation);
	rvSrtpProcessDestruct(&thisPtr->packetProcess);
	rvSrtpKeyDerivationDestruct(&thisPtr->keyDerivation);
	rvSrtpDbDestruct(&thisPtr->db);
	thisPtr->rtpSession = NULL;
	thisPtr->rtcpSession = NULL;

	RvMutexUnlock(&thisPtr->srtpLock, NULL);
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpUnregister"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpSession rvSrtpGetSession(RvSrtp *thisPtr)
{
	RvRtpSession session;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpGetSession"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpGetSession: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpGetSession"));
		return NULL;
	}

	RvMutexLock(&thisPtr->srtpLock, NULL);
	session = thisPtr->rtpSession;
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpGetSession"));
	return session;
}

void rvSrtpGetSoftwareVersion(RvUint8 *majorVersionPtr, RvUint8 *minorVersionPtr, RvUint8 *engineeringReleasePtr, RvUint8 *patchLevelPtr)
{
	*majorVersionPtr       = RvSrtpMajorVersion;
	*minorVersionPtr       = RvSrtpMinorVersion;
	*engineeringReleasePtr = RvSrtpEngineeringRelease;
	*patchLevelPtr         = RvSrtpPatchLevel;
}


/* Master key management */
RvRtpStatus rvSrtpMasterKeyAdd(RvSrtp *thisPtr, RvUint8 *mki, RvUint8 *key, RvUint8 *salt
#ifdef UPDATED_BY_SPIRENT
       ,IN RvUint64      lifetime
       ,IN RvUint64      threshold
       ,IN RvUint8       direction
#endif // UPDATED_BY_SPIRENT
)
{
	RvSrtpKey *newKey;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpMasterKeyAdd"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpMasterKeyAdd: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpMasterKeyAdd"));
		return RV_RTPSTATUS_Failed;
	}
	if((mki == NULL) || (key == NULL) || (salt == NULL)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpMasterKeyAdd: NULL key parameter"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpMasterKeyAdd"));
		return RV_RTPSTATUS_Failed;
	}

	newKey = NULL;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL)
#ifndef UPDATED_BY_SPIRENT
		newKey = rvSrtpDbKeyAdd(&thisPtr->db, (RvUint8 *)mki, (RvUint8 *)key, (RvUint8 *)salt, thisPtr->keyDerivationRate);
#else
	newKey = rvSrtpDbKeyAdd(&thisPtr->db, (RvUint8 *)mki, (RvUint8 *)key, (RvUint8 *)salt,
	        (direction == RV_SRTP_KEY_LOCAL)?thisPtr->keyDerivationRateLocal:thisPtr->keyDerivationRateRemote, lifetime, threshold, direction);
#endif // UPDATED_BY_SPIRENT
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	if(newKey == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpMasterKeyAdd: Could not add key"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpMasterKeyAdd"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpMasterKeyAdd"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpMasterKeyRemove(RvSrtp *thisPtr, RvUint8 *mki
#ifdef UPDATED_BY_SPIRENT
       ,IN RvUint8       direction
#endif // UPDATED_BY_SPIRENT
)
{
	RvBool result;
	RvSrtpKey *key;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpMasterKeyRemove"));
	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpMasterKeyRemove: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpMasterKeyRemove"));
		return RV_RTPSTATUS_Failed;
	}
	if(mki == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpMasterKeyRemove: NULL mki"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpMasterKeyRemove"));
		return RV_RTPSTATUS_Failed;
	}

	result = RV_FALSE;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		key = rvSrtpDbKeyFind(&thisPtr->db, (RvUint8 *)mki
#ifdef UPDATED_BY_SPIRENT
		        , direction
#endif // UPDATED_BY_SPIRENT
		);
		if(key != NULL)
			result = rvSrtpDbKeyRemove(&thisPtr->db, key);
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	if(result == RV_FALSE) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpMasterKeyRemove: Could not find key"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpMasterKeyRemove"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpMasterKeyRemove"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpMasterKeyRemoveAll(RvSrtp *thisPtr)
{
	RvBool result;
	RvSrtpKey *key;
	RvObjHash *keyHash;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpMasterKeyRemoveAll"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpMasterKeyRemoveAll: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpMasterKeyRemoveAll"));
		return RV_RTPSTATUS_Failed;
	}

	result = RV_TRUE;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		keyHash = rvSrtpDbGetKeyHash(&thisPtr->db);
		for(;;) {
			key = (RvSrtpKey *)RvObjHashGetNext(keyHash, NULL);
			if(key == NULL)
				break;
			result = rvSrtpDbKeyRemove(&thisPtr->db, key);
			if(result == RV_FALSE)
				break; /* Something bad has happened, exit to prevent infinite loop */
			}
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	if(result == RV_FALSE) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpMasterKeyRemoveAll: Can not remove all keys"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpMasterKeyRemoveAll"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpMasterKeyRemoveAll"));
	return RV_RTPSTATUS_Succeeded;
}

RvSize_t rvSrtpMasterKeyGetContextCount(RvSrtp *thisPtr, RvUint8 *mki
#ifdef UPDATED_BY_SPIRENT
       ,IN RvUint8       direction
#endif // UPDATED_BY_SPIRENT
        )
{
	RvSize_t result;
	RvSrtpKey *key;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpMasterKeyGetContextCount"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpMasterKeyGetContextCount: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpMasterKeyGetContextCount"));
		return 0;
	}
	if(mki == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpMasterKeyGetContextCount: NULL mki"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpMasterKeyGetContextCount"));
		return 0;
	}

	result = 0;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		key = rvSrtpDbKeyFind(&thisPtr->db, (RvUint8 *)mki
#ifdef UPDATED_BY_SPIRENT
		        , direction
#endif // UPDATED_BY_SPIRENT
		);
		if(key != NULL)
			result = rvSrtpDbKeyGetContextListSize(key);
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpMasterKeyGetContextCount"));
	return result;
}


/* Destination Management */
RvRtpStatus rvSrtpDestinationAddRtp(RvSrtp *thisPtr, RvChar *address, RvUint16 port)
{
	RvSrtpStream *newStream;
	RvAddress addr;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpDestinationAddRtp"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationAddRtp: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationAddRtp"));
		return RV_RTPSTATUS_Failed;
	}

	if(rvSrtpAddressConstructByName(&addr, address, port) == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationAddRtp: Address not found"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationAddRtp"));
		return RV_RTPSTATUS_Failed;
	}

	newStream = NULL;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL)
		newStream = rvSrtpDbDestAdd(&thisPtr->db, thisPtr->ourSsrc, RV_TRUE, thisPtr->ourRtpIndex, &addr);
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	RvAddressDestruct(&addr);

	if(newStream == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationAddRtp: Could not add destination"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationAddRtp"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationAddRtp"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpDestinationAddRtcp(RvSrtp *thisPtr, RvChar *address, RvUint16 port)
{
	RvSrtpStream *newStream;
	RvAddress addr;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpDestinationAddRtcp"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationAddRtcp: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationAddRtcp"));
		return RV_RTPSTATUS_Failed;
	}

	if(rvSrtpAddressConstructByName(&addr, address, port) == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationAddRtcp: Address not found"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationAddRtcp"));
		return RV_RTPSTATUS_Failed;
	}

	newStream = NULL;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL)
		newStream = rvSrtpDbDestAdd(&thisPtr->db, thisPtr->ourSsrc, RV_FALSE, thisPtr->ourRtcpIndex, &addr);
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	RvAddressDestruct(&addr);

	if(newStream == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationAddRtcp: Could not add destination"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationAddRtcp"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationAddRtcp"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpDestinationRemove(RvSrtp *thisPtr, RvChar *address, RvUint16 port)
{
	RvBool result;
	RvSrtpStream *stream;
	RvAddress addr;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpDestinationRemove"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationRemove: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationRemove"));
		return RV_RTPSTATUS_Failed;
	}

	if(rvSrtpAddressConstructByName(&addr, address, port) == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationRemove: Address not found"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationRemove"));
		return RV_RTPSTATUS_Failed;
	}

	result = RV_FALSE;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		stream = rvSrtpDbDestFind(&thisPtr->db, thisPtr->ourSsrc, &addr);
		if(stream != NULL)
			result = rvSrtpDbDestRemove(&thisPtr->db, stream);
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	RvAddressDestruct(&addr);

	if(result == RV_FALSE) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationRemove: Could not remove destination"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationRemove"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationRemove"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpDestinationRemoveAll(RvSrtp *thisPtr)
{
	RvBool result;
	RvSrtpStream *stream;
	RvObjHash *destHash;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpDestinationRemoveAll"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationRemoveAll: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationRemoveAll"));
		return RV_RTPSTATUS_Failed;
	}

	result = RV_TRUE;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		destHash = rvSrtpDbGetDstHash(&thisPtr->db);
		for(;;) {
			stream = (RvSrtpStream *)RvObjHashGetNext(destHash, NULL);
			if(stream == NULL)
				break;
			result = rvSrtpDbDestRemove(&thisPtr->db, stream);
			if(result == RV_FALSE)
				break; /* Something bad has happened, exit to prevent infinite loop */
		}
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	if(result == RV_FALSE) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationRemoveAll: Could not remove all destinations"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationRemoveAll"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationRemoveAll"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpDestinationChangeKey(RvSrtp *thisPtr, RvChar *address, RvUint16 port, RvUint8 *mki, RvBool shareTrigger)
{
	RvAddress addr;
	RvSrtpStream *stream;
	RvSrtpKey *key;
	RvSrtpContext *context;
	RvUint64 index;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpDestinationChangeKey"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationChangeKey: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationChangeKey"));
		return RV_RTPSTATUS_Failed;
	}

	if(rvSrtpAddressConstructByName(&addr, address, port) == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationChangeKey: Address not found"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationChangeKey"));
		return RV_RTPSTATUS_Failed;
	}

	context = NULL;
	key = NULL;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		if(mki != NULL)
			key = rvSrtpDbKeyFind(&thisPtr->db, (RvUint8 *)mki
#ifdef UPDATED_BY_SPIRENT
			, RV_SRTP_KEY_LOCAL
#endif // UPDATED_BY_SPIRENT
			);
		if((mki == NULL) || (key != NULL)) {
			stream = rvSrtpDbDestFind(&thisPtr->db, thisPtr->ourSsrc, &addr);
			if(stream != NULL) {
				if(rvSrtpDbStreamGetIsRtp(stream) == RV_TRUE) {
					index = thisPtr->ourRtpIndex;
				} else index = (RvUint64)thisPtr->ourRtcpIndex;
				context = rvSrtpDbContextAdd(&thisPtr->db, stream, index, key, shareTrigger);
			}
		}
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	RvAddressDestruct(&addr);

	if(context == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationChangeKey: Could not change key"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationChangeKey"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationChangeKey"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpDestinationChangeKeyAt(RvSrtp *thisPtr, RvChar *address, RvUint16 port, RvUint8 *mki, RvUint64 index, RvBool shareTrigger)
{
	RvAddress addr;
	RvSrtpStream *stream;
	RvSrtpKey *key;
	RvSrtpContext *context;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpDestinationChangeKeyAt"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationChangeKeyAt: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationChangeKeyAt"));
		return RV_RTPSTATUS_Failed;
	}

	if(rvSrtpAddressConstructByName(&addr, address, port) == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationChangeKeyAt: Address not found"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationChangeKeyAt"));
		return RV_RTPSTATUS_Failed;
	}

	context = NULL;
	key = NULL;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		if(mki != NULL)
			key = rvSrtpDbKeyFind(&thisPtr->db, (RvUint8 *)mki
#ifdef UPDATED_BY_SPIRENT
			, RV_SRTP_KEY_LOCAL
#endif // UPDATED_BY_SPIRENT
			);
		if((mki == NULL) || (key != NULL)) {
			stream = rvSrtpDbDestFind(&thisPtr->db, thisPtr->ourSsrc, &addr);
			if(stream != NULL)
				context = rvSrtpDbContextAdd(&thisPtr->db, stream, index, key, shareTrigger);
		}
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	RvAddressDestruct(&addr);

	if(context == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationChangeKeyAt: Could not schedule key change"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationChangeKeyAt"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationChangeKeyAt"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpDestinationClearAllKeys(RvSrtp *thisPtr, RvChar *address, RvUint16 port)
{
	RvAddress addr;
	RvSrtpStream *stream;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpDestinationClearAllKeys"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationClearAllKeys: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationClearAllKeys"));
		return RV_RTPSTATUS_Failed;
	}

	if(rvSrtpAddressConstructByName(&addr, address, port) == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationClearAllKeys: Address not found"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationClearAllKeys"));
		return RV_RTPSTATUS_Failed;
	}

	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		stream = rvSrtpDbDestFind(&thisPtr->db, thisPtr->ourSsrc, &addr);
		if(stream != NULL)
			rvSrtpDbStreamClearContext(stream);
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	RvAddressDestruct(&addr);
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationClearAllKeys"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpDestinationGetIndex(RvSrtp *thisPtr, RvChar *address, RvUint16 port, RvUint64 *indexPtr)
{
	RvSrtpStream *stream;
	RvAddress addr;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpDestinationGetIndex"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationGetIndex: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationGetIndex"));
		return RV_RTPSTATUS_Failed;
	}

	if(rvSrtpAddressConstructByName(&addr, address, port) == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationGetIndex: Address not found"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationGetIndex"));
		return RV_RTPSTATUS_Failed;
	}

	stream = NULL;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		stream = rvSrtpDbDestFind(&thisPtr->db, thisPtr->ourSsrc, &addr);
		if(stream != NULL)
			*indexPtr = rvSrtpDbStreamGetMaxIndex(stream);
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	RvAddressDestruct(&addr);

	if(stream == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpDestinationGetIndex: Could not find destination"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationGetIndex"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpDestinationGetIndex"));
	return RV_RTPSTATUS_Succeeded;
}


/* Source Management */
RvRtpStatus rvSrtpRemoteSourceAddRtp(RvSrtp *thisPtr, RvUint32 ssrc, RvUint32 roc, RvUint32 sequenceNum)
{
	RvSrtpStream *newStream;
	RvUint64 initIndex;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceAddRtp"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceAddRtp: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceAddRtp"));
		return RV_RTPSTATUS_Failed;
	}

	initIndex = rvSrtpRtpSeqNumToIndex(sequenceNum, roc);
	newStream = NULL;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL)
		newStream = rvSrtpDbSourceAdd(&thisPtr->db, ssrc, RV_TRUE, initIndex);
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	if(newStream == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceAddRtp: Could not add source"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceAddRtp"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceAddRtp"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpRemoteSourceAddRtcp(RvSrtp *thisPtr, RvUint32 ssrc, RvUint32 index)
{
	RvSrtpStream *newStream;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceAddRtcp"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceAddRtcp: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceAddRtcp"));
		return RV_RTPSTATUS_Failed;
	}

	newStream = NULL;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL)
		newStream = rvSrtpDbSourceAdd(&thisPtr->db, ssrc, RV_FALSE, (RvUint64)rvSrtpRtcpIndexConvert(index));
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	if(newStream == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceAddRtcp: Could not add source"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceAddRtcp"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceAddRtcp"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpRemoteSourceRemove(RvSrtp *thisPtr, RvUint32 ssrc, RvBool sourceType)
{
	RvBool result;
	RvSrtpStream *stream;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceRemove"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceRemove: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceRemove"));
		return RV_RTPSTATUS_Failed;
	}

	result = RV_FALSE;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		stream = rvSrtpDbSourceFind(&thisPtr->db, ssrc, sourceType);
		if(stream != NULL)
			result = rvSrtpDbSourceRemove(&thisPtr->db, stream);
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	if(result == RV_FALSE) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceRemove: Could not remove source"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceRemove"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceRemove"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpRemoteSourceRemoveAll(RvSrtp *thisPtr)
{
	RvBool result;
	RvSrtpStream *stream;
	RvObjHash *sourceHash;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceRemoveAll"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceRemoveAll: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceRemoveAll"));
		return RV_RTPSTATUS_Failed;
	}

	result = RV_TRUE;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		sourceHash = rvSrtpDbGetSrcHash(&thisPtr->db);
		for(;;) {
			stream = (RvSrtpStream *)RvObjHashGetNext(sourceHash, NULL);
			if(stream == NULL)
				break;
			result = rvSrtpDbSourceRemove(&thisPtr->db, stream);
			if(result == RV_FALSE)
				break; /* Something bad has happened, exit to prevent infinite loop */
		}
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	if(result == RV_FALSE) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceRemoveAll: Could not remove all sources"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceRemoveAll"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceRemoveAll"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpRemoteSourceChangeKey(RvSrtp *thisPtr, RvUint32 ssrc, RvBool sourceType, RvUint8* mki, RvBool shareTrigger)
{
	RvSrtpStream *stream;
	RvSrtpKey *key;
	RvSrtpContext *context;
	RvUint64 startIndex;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceChangeKey"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceChangeKey: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceChangeKey"));
		return RV_RTPSTATUS_Failed;
	}

	context = NULL;
	key = NULL;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		if(mki != NULL)
			key = rvSrtpDbKeyFind(&thisPtr->db, (RvUint8 *)mki
#ifdef UPDATED_BY_SPIRENT
			, RV_SRTP_KEY_LOCAL
#endif // UPDATED_BY_SPIRENT
			);
		if((mki == NULL) || (key != NULL)) {
			stream = rvSrtpDbSourceFind(&thisPtr->db, ssrc, sourceType);
			if(stream != NULL) {
				/* Add context at next expected packet */
            //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
            startIndex = (rvSrtpDbStreamGetMaxIndex(stream) + 1) & rvSrtpDbStreamGetWrapIndex(stream);
#else
				startIndex = (rvSrtpDbStreamGetMaxIndex(stream) + RvUint64Const2(1)) & rvSrtpDbStreamGetWrapIndex(stream);
#endif
            //SPIRENT_END
				context = rvSrtpDbContextAdd(&thisPtr->db, stream, startIndex, key, shareTrigger);
			}
		}
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	if(context == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceChangeKey: Could not change key"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceChangeKey"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceChangeKey"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpRemoteSourceChangeKeyAt(RvSrtp *thisPtr, RvUint32 ssrc, RvBool sourceType, RvUint8 *mki, RvUint64 index, RvBool shareTrigger)
{
	RvSrtpStream *stream;
	RvSrtpKey *key;
	RvSrtpContext *context;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceChangeKeyAt"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceChangeKeyAt: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceChangeKeyAt"));
		return RV_RTPSTATUS_Failed;
	}

	context = NULL;
	key = NULL;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		if(mki != NULL)
			key = rvSrtpDbKeyFind(&thisPtr->db, (RvUint8 *)mki
#ifdef UPDATED_BY_SPIRENT
			, RV_SRTP_KEY_LOCAL
#endif // UPDATED_BY_SPIRENT
			);
		if((mki == NULL) || (key != NULL)) {
			stream = rvSrtpDbSourceFind(&thisPtr->db, ssrc, sourceType);
			if(stream != NULL)
				context = rvSrtpDbContextAdd(&thisPtr->db, stream, index, key, shareTrigger);
		}
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	if(context == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceChangeKeyAt: Could not schedule key change"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceChangeKeyAt"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceChangeKeyAt"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpRemoteSourceClearAllKeys(RvSrtp *thisPtr, RvUint32 ssrc, RvBool sourceType)
{
	RvSrtpStream *stream;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceClearAllKeys"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceClearAllKeys: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceClearAllKeys"));
		return RV_RTPSTATUS_Failed;
	}

	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		stream = rvSrtpDbSourceFind(&thisPtr->db, ssrc, sourceType);
		if(stream != NULL)
			rvSrtpDbStreamClearContext(stream);
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceClearAllKeys"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpRemoteSourceGetIndex(RvSrtp *thisPtr, RvUint32 ssrc, RvBool sourceType, RvUint64 *indexPtr)
{
	RvSrtpStream *stream;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceGetIndex"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceGetIndex: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceGetIndex"));
		return RV_RTPSTATUS_Failed;
	}

	stream = NULL;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		stream = rvSrtpDbSourceFind(&thisPtr->db, ssrc, sourceType);
		if(stream != NULL)
			*indexPtr = rvSrtpDbStreamGetMaxIndex(stream);
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	if(stream == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceGetIndex: Could not find source"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceGetIndex"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpRemoteSourceGetIndex"));
	return RV_RTPSTATUS_Succeeded;
}


/* Destination management for translators only */
RvRtpStatus rvSrtpForwardDestinationAddRtp(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvUint32 roc, RvUint32 sequenceNum)
{
	RvSrtpStream *newStream;
	RvAddress addr;
	RvUint64 initIndex;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationAddRtp"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationAddRtp: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationAddRtp"));
		return RV_RTPSTATUS_Failed;
	}

	if(rvSrtpAddressConstructByName(&addr, address, port) == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationAddRtp: Address not found"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationAddRtp"));
		return RV_RTPSTATUS_Failed;
	}

	initIndex = rvSrtpRtpSeqNumToIndex(sequenceNum, roc);
	newStream = NULL;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL)
		newStream = rvSrtpDbDestAdd(&thisPtr->db, ssrc, RV_TRUE, initIndex, &addr);
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	RvAddressDestruct(&addr);

	if(newStream == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationAddRtp: Could not add destination"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationAddRtp"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationAddRtp"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpForwardDestinationAddRtcp(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvUint32 index)
{
	RvSrtpStream *newStream;
	RvAddress addr;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationAddRtcp"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationAddRtcp: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationAddRtcp"));
		return RV_RTPSTATUS_Failed;
	}

	if(rvSrtpAddressConstructByName(&addr, address, port) == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationAddRtcp: Address not found"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationAddRtcp"));
		return RV_RTPSTATUS_Failed;
	}

	newStream = NULL;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtcpSession != NULL) /* @@@ Michael */
		newStream = rvSrtpDbDestAdd(&thisPtr->db, ssrc, RV_FALSE, (RvUint64)rvSrtpRtcpIndexConvert(index), &addr);
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	RvAddressDestruct(&addr);

	if(newStream == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationAddRtcp: Could not add destination"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationAddRtcp"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationAddRtcp"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpForwardDestinationRemove(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port)
{
	RvBool result;
	RvSrtpStream *stream;
	RvAddress addr;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationRemove"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationRemove: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationRemove"));
		return RV_RTPSTATUS_Failed;
	}

	if(rvSrtpAddressConstructByName(&addr, address, port) == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationRemove: Address not found"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationRemove"));
		return RV_RTPSTATUS_Failed;
	}

	result = RV_FALSE;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		stream = rvSrtpDbDestFind(&thisPtr->db, ssrc, &addr);
		if(stream != NULL)
			result = rvSrtpDbDestRemove(&thisPtr->db, stream);
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	RvAddressDestruct(&addr);

	if(result == RV_FALSE) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationRemove: Could not remove destination"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationRemove"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationRemove"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpForwardDestinationChangeKey(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvChar *mki, RvBool shareTrigger)
{
	RvAddress addr;
	RvSrtpStream *stream;
	RvSrtpKey *key;
	RvSrtpContext *context;
	RvUint64 startIndex;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationChangeKey"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationChangeKey: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationChangeKey"));
		return RV_RTPSTATUS_Failed;
	}

	if(rvSrtpAddressConstructByName(&addr, address, port) == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationChangeKey: Address not found"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationChangeKey"));
		return RV_RTPSTATUS_Failed;
	}

	context = NULL;
	key = NULL;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		if(mki != NULL)
			key = rvSrtpDbKeyFind(&thisPtr->db, (RvUint8 *)mki
#ifdef UPDATED_BY_SPIRENT
			, RV_SRTP_KEY_LOCAL
#endif // UPDATED_BY_SPIRENT
			);
		if((mki == NULL) || (key != NULL)) {
			stream = rvSrtpDbDestFind(&thisPtr->db, ssrc, &addr);
			if(stream != NULL) {
				/* Add context at next expected packet */
            //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
            startIndex = (rvSrtpDbStreamGetMaxIndex(stream) + 1) & rvSrtpDbStreamGetWrapIndex(stream);
#else
				startIndex = (rvSrtpDbStreamGetMaxIndex(stream) + RvUint64Const2(1)) & rvSrtpDbStreamGetWrapIndex(stream);
#endif
            //SPIRENT_END
				context = rvSrtpDbContextAdd(&thisPtr->db, stream, startIndex, key, shareTrigger);
			}
		}
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	RvAddressDestruct(&addr);

	if(context == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationChangeKey: Could not change key"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationChangeKey"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationChangeKey"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpForwardDestinationChangeKeyAt(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvChar *mki, RvUint64 index, RvBool shareTrigger)
{
	RvAddress addr;
	RvSrtpStream *stream;
	RvSrtpKey *key;
	RvSrtpContext *context;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationChangeKeyAt"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationChangeKeyAt: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationChangeKeyAt"));
		return RV_RTPSTATUS_Failed;
	}

	if(rvSrtpAddressConstructByName(&addr, address, port) == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationChangeKeyAt: Address not found"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationChangeKeyAt"));
		return RV_RTPSTATUS_Failed;
	}

	context = NULL;
	key = NULL;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		if(mki != NULL)
			key = rvSrtpDbKeyFind(&thisPtr->db, (RvUint8 *)mki
#ifdef UPDATED_BY_SPIRENT
			, RV_SRTP_KEY_LOCAL
#endif // UPDATED_BY_SPIRENT
			);
		if((mki == NULL) || (key != NULL)) {
			stream = rvSrtpDbDestFind(&thisPtr->db, ssrc, &addr);
			if(stream != NULL)
				context = rvSrtpDbContextAdd(&thisPtr->db, stream, index, key, shareTrigger);
		}
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	RvAddressDestruct(&addr);

	if(context == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationChangeKeyAt: Could not schedule key change"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationChangeKeyAt"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationChangeKeyAt"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpForwardDestinationClearAllKeys(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port)
{
	RvAddress addr;
	RvSrtpStream *stream;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationClearAllKeys"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationClearAllKeys: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationClearAllKeys"));
		return RV_RTPSTATUS_Failed;
	}

	if(rvSrtpAddressConstructByName(&addr, address, port) == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationClearAllKeys: Address not found"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationClearAllKeys"));
		return RV_RTPSTATUS_Failed;
	}

	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		stream = rvSrtpDbDestFind(&thisPtr->db, ssrc, &addr);
		if(stream != NULL)
			rvSrtpDbStreamClearContext(stream);
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	RvAddressDestruct(&addr);
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationClearAllKeys"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpForwardDestinationGetIndex(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvUint64 *indexPtr)
{
	RvSrtpStream *stream;
	RvAddress addr;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationGetIndex"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationGetIndex: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationGetIndex"));
		return RV_RTPSTATUS_Failed;
	}

	if(rvSrtpAddressConstructByName(&addr, address, port) == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationGetIndex: Address not found"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationGetIndex"));
		return RV_RTPSTATUS_Failed;
	}

	stream = NULL;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession != NULL) {
		stream = rvSrtpDbDestFind(&thisPtr->db, ssrc, &addr);
		if(stream != NULL)
			*indexPtr = rvSrtpDbStreamGetMaxIndex(stream);
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	RvAddressDestruct(&addr);

	if(stream == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationGetIndex: Could not find destination"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationGetIndex"));
		return RV_RTPSTATUS_Failed;
	}
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationGetIndex"));
	return RV_RTPSTATUS_Succeeded;
}

RvRtpStatus rvSrtpForwardDestinationSsrcChanged(RvSrtp *thisPtr, RvUint32 oldSsrc, RvUint32 newSsrc)
{
	RvObjHash *destHash;
	RvSrtpStream *stream;
	RvBool isRtp;
	RvAddress *addrPtr;
	RvAddress address;
	RvUint64 index;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationSsrcChanged"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationSsrcChanged: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationSsrcChanged"));
		return RV_RTPSTATUS_Failed;
	}

	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession == NULL) {
		RvMutexUnlock(&thisPtr->srtpLock, NULL);
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationSsrcChanged: SRTP Plugin not registered"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationSsrcChanged"));
		return RV_RTPSTATUS_Failed;
	}
	/* Clear all destinations and reset to use new SSRC */
	destHash = rvSrtpDbGetDstHash(&thisPtr->db);

	/* The performance of this algorithm is lousy (exponential) because */
	/* you can't continue to iterate through the hash list after something has */
	/* been added or removed. If this becomes an issue, some other */
	/* mechanism will have to be created. */
	for(;;) {
		stream = NULL;
		/* Find a destination we need to change */
		for(;;) {
			stream = (RvSrtpStream *)RvObjHashGetNext(destHash, stream);
			if(stream == NULL)
				break; /* End of list */
			if(rvSrtpDbStreamGetSsrc(stream) == oldSsrc) {
				/* Found one, remember the info we need */
				isRtp = rvSrtpDbStreamGetIsRtp(stream);
				addrPtr = rvSrtpDbStreamGetAddress(stream);
				memcpy(&address, addrPtr, sizeof(RvAddress));
            //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
            index = (rvSrtpDbStreamGetMaxIndex(stream) + 1) & rvSrtpDbStreamGetWrapIndex(stream);
#else
				index = (rvSrtpDbStreamGetMaxIndex(stream) + RvUint64Const2(1)) & rvSrtpDbStreamGetWrapIndex(stream);
#endif
            //SPIRENT_END
				/* Remove the old destination */
            if(rvSrtpDbDestRemove(&thisPtr->db, stream) != RV_RTPSTATUS_Succeeded) {
					rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationSsrcChanged: Failed to remove a destination"));
            }

				/* Add the new one */
            if(rvSrtpDbDestAdd(&thisPtr->db, newSsrc, isRtp, index, &address) == NULL) {
					rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationSsrcChanged: Failed to add a destination"));
            }

				/* Hash table changed, we have to start from the beginning */
				break;
			}
		}
		if(stream == NULL)
			break; /* We reached the end of the list */
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);

	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpForwardDestinationSsrcChanged"));
	return RV_RTPSTATUS_Succeeded;
}


/* Translator forwarding functions (use instead of RTP/RTCP forwarding functions) */
RvRtpStatus rvSrtpForwardRtp(RvSrtp *thisPtr,
                             RvChar *bufStartPtr,
                             RvUint32 bufSize,
                             RvChar *dataStartPtr,
                             RvUint32 dataSize,
                             RvRtpParam *headerPtr,
                             RvUint32 roc)
{
    RV_UNUSED_ARG(thisPtr);
    RV_UNUSED_ARG(bufStartPtr);
    RV_UNUSED_ARG(bufSize);
    RV_UNUSED_ARG(dataStartPtr);
    RV_UNUSED_ARG(dataSize);
    RV_UNUSED_ARG(headerPtr);
    RV_UNUSED_ARG(roc);
	return RV_RTPSTATUS_NotSupported;
}

RvRtpStatus rvSrtpForwardRtcp(RvSrtp *thisPtr,
                              RvChar *bufStartPtr,
                              RvUint32 bufSize,
                              RvChar *dataStartPtr,
                              RvUint32 dataSize,
                              RvUint32 index)
{
    RV_UNUSED_ARG(thisPtr);
    RV_UNUSED_ARG(bufStartPtr);
    RV_UNUSED_ARG(bufSize);
    RV_UNUSED_ARG(dataStartPtr);
    RV_UNUSED_ARG(dataSize);
    RV_UNUSED_ARG(index);
	return RV_RTPSTATUS_NotSupported;
}


/* Configuration */
RvRtpStatus rvSrtpSetMasterKeySizes(RvSrtp *thisPtr, RvSize_t mkiSize, RvSize_t keySize, RvSize_t saltSize)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeySizes"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeySizes: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeySizes"));
		return RV_RTPSTATUS_Failed;
	}

	if((mkiSize < 1) || (keySize < 1)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeySizes: MKI and Key size must be > 0"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeySizes"));
		return RV_RTPSTATUS_Failed;
	}

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession == NULL) {
		thisPtr->mkiSize = mkiSize;
		thisPtr->keySize = keySize;
		thisPtr->saltSize = saltSize;
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeySizes: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
    rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetMasterKeySizes: MKI size set to %u, key size set to %u, salt size set to %u", mkiSize, keySize, saltSize));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeySizes"));
	return status;
}

#ifndef UPDATED_BY_SPIRENT
RvRtpStatus rvSrtpSetKeyDerivation(RvSrtp *thisPtr, RvInt keyDerivationAlg, RvUint32 keyDerivationRate)
#else
RvRtpStatus rvSrtpSetKeyDerivation(RvSrtp *thisPtr, RvInt keyDerivationAlg, RvUint32 keyDerivationRateLocal,
        RvUint32 keyDerivationRateRemote)
#endif // UPDATED_BY_SPIRENT
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetKeyDerivation"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetKeyDerivation: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetKeyDerivation"));
		return RV_RTPSTATUS_Failed;
	}

	if(keyDerivationAlg != RV_SRTP_KEYDERIVATIONALG_AESCM) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetKeyDerivation: Invalid algorithm"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetKeyDerivation"));
		return RV_RTPSTATUS_Failed;
	}

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession == NULL) {
		thisPtr->keyDerivationAlg = keyDerivationAlg;
#ifndef UPDATED_BY_SPIRENT
		thisPtr->keyDerivationRate = keyDerivationRate;
#else
		thisPtr->keyDerivationRateLocal = keyDerivationRateLocal;
		thisPtr->keyDerivationRateRemote = keyDerivationRateRemote;
#endif // UPDATED_BY_SPIRENT
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetKeyDerivation: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
    rvSrtpLogInfo((rvSrtpLogSourcePtr,
                   "rvSrtpSetKeyDerivation: Key derivation algorithm set to %s, local rate set to %u, remote rate set to %u ",
                  (keyDerivationAlg == RV_SRTP_KEYDERIVATIONALG_AESCM?"AESCM":"NONE")
#ifdef UPDATED_BY_SPIRENT
                  , keyDerivationRateLocal, keyDerivationRateRemote
#endif // UPDATED_BY_SPIRENT
                 ));

	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetKeyDerivation"));
	return status;
}

RvRtpStatus rvSrtpSetPrefixLength(RvSrtp *thisPtr, RvSize_t prefixLength)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetPrefixLength"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetPrefixLength: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetPrefixLength"));
		return RV_RTPSTATUS_Failed;
	}

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession == NULL) {
		thisPtr->prefixLen = prefixLength;
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetPrefixLength: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
    rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetPrefixLength: Prefix length set to %u", prefixLength));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetPrefixLength"));
	return status;
}

RvRtpStatus rvSrtpSetRtpEncryption(RvSrtp *thisPtr, RvInt encryptType, RvBool useMki)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetRtpEncryption"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtpEncryption: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtpEncryption"));
		return RV_RTPSTATUS_Failed;
	}

	if((encryptType != RV_SRTP_ENCYRPTIONALG_NULL) && (encryptType != RV_SRTP_ENCYRPTIONALG_AESCM) && (encryptType != RV_SRTP_ENCYRPTIONALG_AESF8)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtpEncryption: Invalid encryption algorithm"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtpEncryption"));
		return RV_RTPSTATUS_Failed;
	}
	if (encryptType == RV_SRTP_ENCYRPTIONALG_NULL && useMki)
    {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtpEncryption: No encryption with MKI usage is prohibited."));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtpEncryption"));
		return RV_RTPSTATUS_Failed;
    }
	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession == NULL) {
		thisPtr->rtpEncAlg = encryptType;
		thisPtr->rtpUseMki = useMki;
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtpEncryption: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
    rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetRtpEncryption: encryption set to %u(0-NULL,1-AESCM,2-AESF8), useMki=%d", encryptType, useMki));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtpEncryption"));
	return status;
}

RvRtpStatus rvSrtpSetRtcpEncryption(RvSrtp *thisPtr, RvInt encryptType, RvBool useMki)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetRtcpEncryption"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtcpEncryption: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtcpEncryption"));
		return RV_RTPSTATUS_Failed;
	}

	if (encryptType == RV_SRTP_ENCYRPTIONALG_NULL && useMki)
    {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtcpEncryption: No encryption with MKI usage is prohibited."));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtcpEncryption"));
		return RV_RTPSTATUS_Failed;
    }

	if((encryptType != RV_SRTP_ENCYRPTIONALG_NULL) && (encryptType != RV_SRTP_ENCYRPTIONALG_AESCM) && (encryptType != RV_SRTP_ENCYRPTIONALG_AESF8)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtcpEncryption: Invalid encryption algorithm"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtcpEncryption"));
		return RV_RTPSTATUS_Failed;
	}

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
    if(thisPtr->rtcpSession == NULL)
    {
		thisPtr->rtcpEncAlg = encryptType;
		thisPtr->rtcpUseMki = useMki;
		status = RV_RTPSTATUS_Succeeded;
	}
    else
    {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtcpEncryption: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
    rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetRtcpEncryption: encryption set to %u(0-NULL,1-AESCM,2-AESF8), useMki=%d", encryptType, useMki));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtcpEncryption"));
	return status;
}

RvRtpStatus rvSrtpSetRtpAuthentication(RvSrtp *thisPtr, RvInt authType, RvSize_t tagSize)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetRtpAuthentication"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtpAuthentication: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtpAuthentication"));
		return RV_RTPSTATUS_Failed;
	}

	if((authType != RV_SRTP_AUTHENTICATIONALG_NONE) && (authType != RV_SRTP_AUTHENTICATIONALG_HMACSHA1)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtpAuthentication: Invalid authentication algorithm"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtpAuthentication"));
		return RV_RTPSTATUS_Failed;
	}
	if((authType != RV_SRTP_AUTHENTICATIONALG_NONE) && (tagSize < 1)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtpAuthentication: Tag size must be > 0"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtpAuthentication"));
		return RV_RTPSTATUS_Failed;
	}
	if((authType == RV_SRTP_AUTHENTICATIONALG_NONE) && (tagSize > 0)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtpAuthentication: Tag size must be 0 for NULL authentication"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtpAuthentication"));
		return RV_RTPSTATUS_Failed;
	}
	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession == NULL) {
		thisPtr->rtpAuthAlg = authType;
		thisPtr->rtpAuthTagSize = tagSize;
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtpAuthentication: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
    rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetRtpAuthentication: authentication set to %u(0-NULL,1-HMACSHA1), tagSize to %d", authType, tagSize));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtpAuthentication"));
	return status;
}

RvRtpStatus rvSrtpSetRtcpAuthentication(RvSrtp *thisPtr, RvInt authType, RvSize_t tagSize)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetRtcpAuthentication"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtcpAuthentication: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtcpAuthentication"));
		return RV_RTPSTATUS_Failed;
	}

	if((authType != RV_SRTP_AUTHENTICATIONALG_NONE) && (authType != RV_SRTP_AUTHENTICATIONALG_HMACSHA1)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtcpAuthentication: Invalid authentication algorithm"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtcpAuthentication"));
		return RV_RTPSTATUS_Failed;
	}
	if((authType != RV_SRTP_AUTHENTICATIONALG_NONE) && (tagSize < 1)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtcpAuthentication: Tag size must be > 0"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtcpAuthentication"));
		return RV_RTPSTATUS_Failed;
	}
	if((authType == RV_SRTP_AUTHENTICATIONALG_NONE) && (tagSize > 0)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtcpAuthentication: Tag size must be 0 for NULL authentication"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtcpAuthentication"));
		return RV_RTPSTATUS_Failed;
	}
	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
    if(thisPtr->rtcpSession == NULL) { /* @@@ Michael */
		thisPtr->rtcpAuthAlg = authType;
		thisPtr->rtcpAuthTagSize = tagSize;
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtcpAuthentication: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
    rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetRtcpAuthentication: authentication set to %u(0-NULL,1-HMACSHA1), tagSize to %d", authType, tagSize));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtcpAuthentication"));
	return status;
}

RvRtpStatus rvSrtpSetRtpKeySizes(RvSrtp *thisPtr, RvSize_t encryptKeySize, RvSize_t authKeySize, RvSize_t saltSize)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetRtpKeySizes"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtpKeySizes: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtpKeySizes"));
		return RV_RTPSTATUS_Failed;
	}

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession == NULL) {
		thisPtr->rtpEncryptKeySize = encryptKeySize;
		thisPtr->rtpAuthKeySize = authKeySize;
		thisPtr->rtpSaltSize = saltSize;
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtpKeySizes: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
    rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetRtpKeySizes: encryptKeySize set to %u, authKeySize to %u, saltSize to %u", encryptKeySize, authKeySize, saltSize));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtpKeySizes"));
	return status;
}

RvRtpStatus rvSrtpSetRtcpKeySizes(RvSrtp *thisPtr, RvSize_t encryptKeySize, RvSize_t authKeySize, RvSize_t saltSize)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetRtcpKeySizes"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtcpKeySizes: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtcpKeySizes"));
		return RV_RTPSTATUS_Failed;
	}

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtcpSession == NULL) {
		thisPtr->rtcpEncryptKeySize = encryptKeySize;
		thisPtr->rtcpAuthKeySize = authKeySize;
		thisPtr->rtcpSaltSize = saltSize;
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtcpKeySizes: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
    rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetRtcpKeySizes: encryptKeySize set to %u, authKeySize to %u, saltSize to %u", encryptKeySize, authKeySize, saltSize));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtcpKeySizes"));
	return status;
}

RvRtpStatus rvSrtpSetRtpReplayListSize(RvSrtp *thisPtr, RvUint64 replayListSize)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetRtpReplayListSize"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtpReplayListSize: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtpReplayListSize"));
		return RV_RTPSTATUS_Failed;
	}

   //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
   if((replayListSize > 0) && (replayListSize < 64)) {
#else
	if((replayListSize > RvUint64Const2(0)) && (replayListSize < RvUint64Const2(64))) {
#endif
      //SPIRENT_END
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtpReplayListSize: Replay list size must be 0 or > 64"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtpReplayListSize"));
		return RV_RTPSTATUS_Failed;
	}

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession == NULL) {
		thisPtr->rtpReplayListSize = replayListSize;
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtpReplayListSize: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
    rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetRtpReplayListSize: replayListSize set to %#x:%#x",
        RvUint64ToRvUint32(RvUint64ShiftRight(replayListSize, 32)),
        RvUint64ToRvUint32(RvUint64And(RvUint64Const(0,0xFFFFFFFF), replayListSize))));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtpReplayListSize"));
	return status;
}

RvRtpStatus rvSrtpSetRtcpReplayListSize(RvSrtp *thisPtr, RvUint64 replayListSize)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetRtcpReplayListSize"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtcpReplayListSize: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtcpReplayListSize"));
		return RV_RTPSTATUS_Failed;
	}

   //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
   if((replayListSize > 0) && (replayListSize < 64)) {
#else
	if((replayListSize > RvUint64Const2(0)) && (replayListSize < RvUint64Const2(64))) {
#endif
      //SPIRENT_END
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtcpReplayListSize: Replay list size must be 0 or > 64"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtcpReplayListSize"));
		return RV_RTPSTATUS_Failed;
	}

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
    if(thisPtr->rtcpSession == NULL) { /* @@@ Michael */
		thisPtr->rtcpReplayListSize = replayListSize;
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtcpReplayListSize: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
    rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetRtcpReplayListSize: replayListSize set to %#x:%#x",
        RvUint64ToRvUint32(RvUint64ShiftRight(replayListSize, 32)),
        RvUint64ToRvUint32(RvUint64And(RvUint64Const(0,0xFFFFFFFF), replayListSize))));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtcpReplayListSize"));
	return status;
}

RvRtpStatus rvSrtpSetRtpHistory(RvSrtp *thisPtr, RvUint64 historySize)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetRtpHistory"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtpHistory: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtpHistory"));
		return RV_RTPSTATUS_Failed;
	}

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession == NULL) {
		thisPtr->rtpHistorySize = historySize;
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtpHistory: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
    rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetRtpHistory: historySize set to %#x:%#x",
        RvUint64ToRvUint32(RvUint64ShiftRight(historySize, 32)),
        RvUint64ToRvUint32(RvUint64And(RvUint64Const(0,0xFFFFFFFF), historySize))));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtpHistory"));
	return status;
}

RvRtpStatus rvSrtpSetRtcpHistory(RvSrtp *thisPtr, RvUint64 historySize)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetRtcpHistory"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtcpHistory: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtcpHistory"));
		return RV_RTPSTATUS_Failed;
	}

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
    if(thisPtr->rtcpSession == NULL) { /* @@@ Michael? */
		thisPtr->rtcpHistorySize = historySize;
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetRtcpHistory: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
    rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetRtcpHistory: historySize set to %#x:%#x",
        RvUint64ToRvUint32(RvUint64ShiftRight(historySize, 32)),
        RvUint64ToRvUint32(RvUint64And(RvUint64Const(0,0xFFFFFFFF), historySize))));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetRtcpHistory"));
	return status;
}


/* Advanced Configuration */
RvRtpStatus rvSrtpSetMasterKeyPool(RvSrtp *thisPtr, RvInt poolType, RvSize_t pageItems, RvSize_t pageSize, RvSize_t maxItems, RvSize_t minItems, RvSize_t freeLevel, RvMemory *region)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyPool"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyPool: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyPool"));
		return RV_RTPSTATUS_Failed;
	}

	if((poolType != RV_OBJPOOL_TYPE_FIXED) && (poolType != RV_OBJPOOL_TYPE_EXPANDING) && (poolType != RV_OBJPOOL_TYPE_DYNAMIC)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyPool: Invalid pool type"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyPool"));
		return RV_RTPSTATUS_Failed;
	}
	if(freeLevel > 100) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyPool: Free level must be <= 100"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyPool"));
		return RV_RTPSTATUS_Failed;
	}
	if((maxItems < minItems) && (maxItems != 0)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyPool: Max items must be > min items"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyPool"));
		return RV_RTPSTATUS_Failed;
	}

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession == NULL) {
		thisPtr->poolConfig.keyRegion = region;
		thisPtr->poolConfig.keyPageItems = pageItems;
		thisPtr->poolConfig.keyPageSize = pageSize;;
		thisPtr->poolConfig.keyPoolType = poolType;
		thisPtr->poolConfig.keyMaxItems = maxItems;
		thisPtr->poolConfig.keyMinItems = minItems;
		thisPtr->poolConfig.keyFreeLevel = freeLevel;
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyPool: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
	rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetMasterKeyPool: poolType set to %u (0-Fixed,1-expanding,2-dynamic),  pageItems to %u, pageSize to %u, maxItems to %u, minItems to %u, freeLevel to %u", poolType, pageItems, pageSize, maxItems, minItems, freeLevel));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyPool"));
	return status;
}

RvRtpStatus rvSrtpSetStreamPool(RvSrtp *thisPtr, RvInt poolType, RvSize_t pageItems, RvSize_t pageSize, RvSize_t maxItems, RvSize_t minItems, RvSize_t freeLevel, RvMemory *region)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetStreamPool"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetStreamPool: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetStreamPool"));
		return RV_RTPSTATUS_Failed;
	}

	if((poolType != RV_OBJPOOL_TYPE_FIXED) && (poolType != RV_OBJPOOL_TYPE_EXPANDING) && (poolType != RV_OBJPOOL_TYPE_DYNAMIC)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetStreamPool: Invalid pool type"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetStreamPool"));
		return RV_RTPSTATUS_Failed;
	}
	if(freeLevel > 100) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetStreamPool: Free level must be <= 100"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetStreamPool"));
		return RV_RTPSTATUS_Failed;
	}
	if((maxItems < minItems) && (maxItems != 0)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetStreamPool: Max items must be > min items"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetStreamPool"));
		return RV_RTPSTATUS_Failed;
	}

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession == NULL) {
		thisPtr->poolConfig.streamRegion = region;
		thisPtr->poolConfig.streamPageItems = pageItems;
		thisPtr->poolConfig.streamPageSize = pageSize;;
		thisPtr->poolConfig.streamPoolType = poolType;
		thisPtr->poolConfig.streamMaxItems = maxItems;
		thisPtr->poolConfig.streamMinItems = minItems;
		thisPtr->poolConfig.streamFreeLevel = freeLevel;
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetStreamPool: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
	rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetStreamPool: poolType set to %u (0-Fixed,1-expanding,2-dynamic),  pageItems to %u, pageSize to %u, maxItems to %u, minItems to %u, freeLevel to %u", poolType, pageItems, pageSize, maxItems, minItems, freeLevel));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetStreamPool"));
	return status;
}

RvRtpStatus rvSrtpSetContextPool(RvSrtp *thisPtr, RvInt poolType, RvSize_t pageItems, RvSize_t pageSize, RvSize_t maxItems, RvSize_t minItems, RvSize_t freeLevel, RvMemory *region)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetContextPool"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetContextPool: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetContextPool"));
		return RV_RTPSTATUS_Failed;
	}

	if((poolType != RV_OBJPOOL_TYPE_FIXED) && (poolType != RV_OBJPOOL_TYPE_EXPANDING) && (poolType != RV_OBJPOOL_TYPE_DYNAMIC)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetContextPool: Invalid pool type"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetContextPool"));
		return RV_RTPSTATUS_Failed;
	}
	if(freeLevel > 100) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetContextPool: Free level must be <= 100"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetContextPool"));
		return RV_RTPSTATUS_Failed;
	}
	if((maxItems < minItems) && (maxItems != 0)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetContextPool: Max items must be > min items"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetContextPool"));
		return RV_RTPSTATUS_Failed;
	}

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession == NULL) {
		thisPtr->poolConfig.contextRegion = region;
		thisPtr->poolConfig.contextPageItems = pageItems;
		thisPtr->poolConfig.contextPageSize = pageSize;;
		thisPtr->poolConfig.contextPoolType = poolType;
		thisPtr->poolConfig.contextMaxItems = maxItems;
		thisPtr->poolConfig.contextMinItems = minItems;
		thisPtr->poolConfig.contextFreeLevel = freeLevel;
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetContextPool: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
	rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetContextPool: poolType set to %u (0-Fixed,1-expanding,2-dynamic),  pageItems to %u, pageSize to %u, maxItems to %u, minItems to %u, freeLevel to %u", poolType, pageItems, pageSize, maxItems, minItems, freeLevel));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetContextPool"));
	return status;
}

RvRtpStatus rvSrtpSetMasterKeyHash(RvSrtp *thisPtr, RvInt hashType, RvSize_t startSize, RvMemory *region)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyHash"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyHash: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyHash"));
		return RV_RTPSTATUS_Failed;
	}

	if((hashType != RV_OBJHASH_TYPE_FIXED) && (hashType != RV_OBJHASH_TYPE_EXPANDING) && (hashType != RV_OBJHASH_TYPE_DYNAMIC)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyHash: Invalid hash type"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyHash"));
		return RV_RTPSTATUS_Failed;
	}

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession == NULL) {
		thisPtr->poolConfig.keyHashRegion = region;
		thisPtr->poolConfig.keyHashStartSize = startSize;
		thisPtr->poolConfig.keyHashType = hashType;
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyHash: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
	rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetMasterKeyHash: hashType set to %u (0-Fixed,1-expanding,2-dynamic), startSize to %u", hashType, startSize));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetMasterKeyHash"));
	return status;
}

RvRtpStatus rvSrtpSetSourceHash(RvSrtp *thisPtr, RvInt hashType, RvSize_t startSize, RvMemory *region)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetSourceHash"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetSourceHash: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetSourceHash"));
		return RV_RTPSTATUS_Failed;
	}

	if((hashType != RV_OBJHASH_TYPE_FIXED) && (hashType != RV_OBJHASH_TYPE_EXPANDING) && (hashType != RV_OBJHASH_TYPE_DYNAMIC)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetSourceHash: Invalid hash type"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetSourceHash"));
		return RV_RTPSTATUS_Failed;
	}

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession == NULL) {
		thisPtr->poolConfig.sourceHashRegion = region;
		thisPtr->poolConfig.sourceHashStartSize = startSize;
		thisPtr->poolConfig.sourceHashType = hashType;
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetSourceHash: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
	rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetSourceHash: hashType set to %u (0-Fixed,1-expanding,2-dynamic), startSize to %u", hashType, startSize));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetSourceHash"));
	return status;
}

RvRtpStatus rvSrtpSetDestHash(RvSrtp *thisPtr, RvInt hashType, RvSize_t startSize, RvMemory *region)
{
	RvRtpStatus status;

	rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvSrtpSetDestHash"));

	if(thisPtr == NULL) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetDestHash: NULL SRTP object"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetDestHash"));
		return RV_RTPSTATUS_Failed;
	}

	if((hashType != RV_OBJHASH_TYPE_FIXED) && (hashType != RV_OBJHASH_TYPE_EXPANDING) && (hashType != RV_OBJHASH_TYPE_DYNAMIC)) {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetDestHash: Invalid hash type"));
		rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetDestHash"));
		return RV_RTPSTATUS_Failed;
	}

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&thisPtr->srtpLock, NULL);
	if(thisPtr->rtpSession == NULL) {
		thisPtr->poolConfig.destHashRegion = region;
		thisPtr->poolConfig.destHashStartSize = startSize;
		thisPtr->poolConfig.destHashType = hashType;
		status = RV_RTPSTATUS_Succeeded;
	} else {
		rvSrtpLogError((rvSrtpLogSourcePtr, "rvSrtpSetDestHash: Session is open"));
	}
	RvMutexUnlock(&thisPtr->srtpLock, NULL);
	rvSrtpLogInfo((rvSrtpLogSourcePtr,  "rvSrtpSetDestHash: hashType set to %u (0-Fixed,1-expanding,2-dynamic), startSize to %u", hashType, startSize));
	rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvSrtpSetDestHash"));
	return status;
}

/***** Plugin Functions ******/

static RvRtpStatus rvSrtpRegistered(RvRtpSession *thisPtr, void *encryptionPluginData, RvBool enableRtp, RvBool enableRtcp, RvUint32 ssrc, RvUint16 rtpSeqNum, RvUint32 rtpRoc, RvUint32 rtcpIndex)
{
	RvSrtp *srtpPtr;

	srtpPtr = (RvSrtp *)encryptionPluginData;

	RvMutexLock(&srtpPtr->srtpLock, NULL);
	srtpPtr->ourSsrc = ssrc;
	srtpPtr->ourRtpIndex = rvSrtpRtpSeqNumToIndex(rtpSeqNum, rtpRoc);
	srtpPtr->ourRtcpIndex = rvSrtpRtcpIndexConvert(rtcpIndex);
	rvSrtpSsrcValidationSetRtpCheck(&srtpPtr->ssrcValidation, enableRtp);
	rvSrtpSsrcValidationSetRtcpCheck(&srtpPtr->ssrcValidation, enableRtcp);
	RvMutexUnlock(&srtpPtr->srtpLock, NULL);

    RV_UNUSED_ARG(thisPtr);

	return RV_RTPSTATUS_Succeeded;
}

static RvRtpStatus rvSrtpUnregistered(RvRtpSession *thisPtr, void *encryptionPluginData)
{
    RV_UNUSED_ARG(thisPtr);
    RV_UNUSED_ARG(encryptionPluginData);

	return RV_RTPSTATUS_Succeeded;
}

static RvRtpStatus rvSrtpOpen(RvRtpSession *thisPtr, void *encryptionPluginData)
{
    RV_UNUSED_ARG(thisPtr);
    RV_UNUSED_ARG(encryptionPluginData);

	return RV_RTPSTATUS_Succeeded;
}

static RvRtpStatus rvSrtpClose(RvRtpSession *thisPtr, void *encryptionPluginData)
{
    RV_UNUSED_ARG(thisPtr);
    RV_UNUSED_ARG(encryptionPluginData);

	return RV_RTPSTATUS_Succeeded;
}

#ifdef UPDATED_BY_SPIRENT
inline static void* rvSrtpGetHandlerEncrypt(RvRtcpSession rtcp)
{
    if (!rtcp) {
        return NULL;
    }
    return ((rtcpSession*)rtcp)->haRtcpSend;
}

inline static void* rvSrtpGetHandlerDecrypt(RvRtcpSession rtcp)
{
    if (!rtcp) {
        return NULL;
    }
    return ((rtcpSession*)rtcp)->haRtcpSend;
}
#endif // UPDATED_BY_SPIRENT

static RvRtpStatus rvSrtpEncryptRtp(RvRtpSession thisPtr, void *encryptionPluginData, RvDataBuffer *srcBuffer, RvDataBuffer *dstBuffer, RvAddress *srcAddress, RvAddress *dstAddress, RvUint32 roc)
{
	RvSrtp *srtpPtr;
	RvSrtpStream *streamPtr;
	RvRtpStatus status;
	RvUint32 ssrc;

	srtpPtr = (RvSrtp *)encryptionPluginData;

	/* Get SSRC from RTP packet (3rd 32 bit word) */
	rvDataBufferSkip(srcBuffer, 8);
	rvDataBufferReadUint32(srcBuffer, &ssrc);
	rvDataBufferRewind(srcBuffer, 12);

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&srtpPtr->srtpLock, NULL);
	streamPtr = rvSrtpDbDestFind(&srtpPtr->db, ssrc, dstAddress);
	if(streamPtr != NULL) {
	    status = rvSrtpProcessEncryptRtp(&srtpPtr->packetProcess, streamPtr, srcBuffer, dstBuffer, roc
#ifdef UPDATED_BY_SPIRENT
	            ,srtpPtr->masterKeyEventCB, rvSrtpGetHandlerEncrypt(srtpPtr->rtcpSession)
#endif // UPDATED_BY_SPIRENT
	    );
	}
	RvMutexUnlock(&srtpPtr->srtpLock, NULL);
    RV_UNUSED_ARG(thisPtr);
    RV_UNUSED_ARG(srcAddress);

	return status;
}

static RvRtpStatus rvSrtpEncryptRtcp(RvRtpSession thisPtr, void *encryptionPluginData, RvDataBuffer *srcBuffer, RvDataBuffer *dstBuffer, RvAddress *srcAddress, RvAddress *dstAddress, RvUint32 index, RvBool encrypt)
{
	RvSrtp *srtpPtr;
	RvSrtpStream *streamPtr;
	RvRtpStatus status;
	RvUint32 ssrc;

	srtpPtr = (RvSrtp *)encryptionPluginData;

	/* Get SSRC from RTCP packet (2nd 32 bit word) */
	rvDataBufferSkip(srcBuffer, 4);
	rvDataBufferReadUint32(srcBuffer, &ssrc);
	rvDataBufferRewind(srcBuffer, 8);

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&srtpPtr->srtpLock, NULL);
	streamPtr = rvSrtpDbDestFind(&srtpPtr->db, ssrc, dstAddress);
	if(streamPtr != NULL) {
	    status = rvSrtpProcessEncryptRtcp(&srtpPtr->packetProcess, streamPtr, srcBuffer, dstBuffer, rvSrtpRtcpIndexConvert(index), encrypt
#ifdef UPDATED_BY_SPIRENT
	            ,srtpPtr->masterKeyEventCB, rvSrtpGetHandlerEncrypt(srtpPtr->rtcpSession)
#endif // UPDATED_BY_SPIRENT
	    );
	}
	RvMutexUnlock(&srtpPtr->srtpLock, NULL);
    RV_UNUSED_ARG(thisPtr);
    RV_UNUSED_ARG(srcAddress);
	return status;
}

static RvRtpStatus rvSrtpDecryptRtp(
    RvRtpSession thisPtr,
    void *encryptionPluginData,
    RvDataBuffer *srcBuffer,
    RvDataBuffer *dstBuffer,
    RvAddress *srcAddress,
    RvAddress *dstAddress)
{
	RvSrtp *srtpPtr;
	RvSrtpStream *streamPtr;
	RvRtpStatus status;
	RvUint32 ssrc;
	RvUint32 roc;

	srtpPtr = (RvSrtp *)encryptionPluginData;

	/* Get SSRC from RTP packet (3rd 32 bit word) */
	rvDataBufferSkip(srcBuffer, 8);
	rvDataBufferReadUint32(srcBuffer, &ssrc);
	rvDataBufferRewind(srcBuffer, 12);

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&srtpPtr->srtpLock, NULL);
	streamPtr = rvSrtpDbSourceFind(&srtpPtr->db, ssrc, RV_TRUE);
	if(streamPtr != NULL) {
	    status = rvSrtpProcessDecryptRtp(&srtpPtr->packetProcess, streamPtr, srcBuffer, dstBuffer, &roc
#ifdef UPDATED_BY_SPIRENT
	            ,srtpPtr->masterKeyEventCB, rvSrtpGetHandlerDecrypt(srtpPtr->rtcpSession)
#endif // UPDATED_BY_SPIRENT
	    );
	}
	RvMutexUnlock(&srtpPtr->srtpLock, NULL);
    RV_UNUSED_ARG(thisPtr);
    RV_UNUSED_ARG(srcAddress);
    RV_UNUSED_ARG(dstAddress);

	return status;
}

static RvRtpStatus rvSrtpDecryptRtcp(
    RvRtpSession thisPtr,
    void *encryptionPluginData,
    RvDataBuffer *srcBuffer,
    RvDataBuffer *dstBuffer,
    RvAddress *srcAddress,
    RvAddress *dstAddress)
{
	RvSrtp *srtpPtr;
	RvSrtpStream *streamPtr;
	RvRtpStatus status;
	RvUint32 ssrc;
	RvUint32 index;
	RvBool encrypt;

	srtpPtr = (RvSrtp *)encryptionPluginData;

	/* Get SSRC from RTCP packet (2nd 32 bit word) */
	rvDataBufferSkip(srcBuffer, 4);
	rvDataBufferReadUint32(srcBuffer, &ssrc);
	rvDataBufferRewind(srcBuffer, 8);

	status = RV_RTPSTATUS_Failed;
	RvMutexLock(&srtpPtr->srtpLock, NULL);
	streamPtr = rvSrtpDbSourceFind(&srtpPtr->db, ssrc, RV_FALSE);
	if(streamPtr != NULL) {
	    status = rvSrtpProcessDecryptRtcp(&srtpPtr->packetProcess, streamPtr, srcBuffer, dstBuffer, &index, &encrypt
#ifdef UPDATED_BY_SPIRENT
	            ,srtpPtr->masterKeyEventCB, rvSrtpGetHandlerDecrypt(srtpPtr->rtcpSession)
#endif // UPDATED_BY_SPIRENT
	    );
	}
	RvMutexUnlock(&srtpPtr->srtpLock, NULL);

    RV_UNUSED_ARG(srcAddress);
    RV_UNUSED_ARG(thisPtr);
    RV_UNUSED_ARG(dstAddress);

    return status;
}

static RvBool rvSrtpValidateSsrc(RvRtpSession *thisPtr, void *encryptionPluginData, RvUint32 ssrc)
{
	RvSrtp *srtpPtr;
	RvRtpStatus status;

	srtpPtr = (RvSrtp *)encryptionPluginData;

	RvMutexLock(&srtpPtr->srtpLock, NULL);
	status = rvSrtpSsrcValidationCheck(&srtpPtr->ssrcValidation, ssrc, srtpPtr->ourRtpIndex, srtpPtr->ourRtcpIndex);
	RvMutexUnlock(&srtpPtr->srtpLock, NULL);
    RV_UNUSED_ARG(thisPtr);

	return RV_RTPSTATUS_Succeeded;
}

static RvRtpStatus rvSrtpSsrcChanged(RvRtpSession *thisPtr, void *encryptionPluginData, RvUint32 oldSsrc, RvUint32 newSsrc)
{
	RvSrtp *srtpPtr;
	RvObjHash *destHash;
	RvSrtpStream *stream;
	RvBool isRtp;
	RvAddress *addrPtr;
	RvAddress address;
	RvUint64 index;

	srtpPtr = (RvSrtp *)encryptionPluginData;

	RvMutexLock(&srtpPtr->srtpLock, NULL);
	/* Clear all destinations and reset to use new SSRC */
	destHash = rvSrtpDbGetDstHash(&srtpPtr->db);

	/* The performance of this algorithm is lousy (exponential) because */
	/* you can't continue to iterate through the hash list after something has */
	/* been added or removed. If this becomes an issue, some other */
	/* mechanism will have to be created. */
	for(;;) {
		stream = NULL;
		/* Find a destination we need to change */
		for(;;) {
			stream = (RvSrtpStream *)RvObjHashGetNext(destHash, stream);
			if(stream == NULL)
				break; /* End of list */
			if(rvSrtpDbStreamGetSsrc(stream) == oldSsrc) {
				/* Found one, remember the info we need */
				isRtp = rvSrtpDbStreamGetIsRtp(stream);
				addrPtr = rvSrtpDbStreamGetAddress(stream);
				memcpy(&address, addrPtr, sizeof(RvAddress));
				if(isRtp == RV_TRUE) {
					index = srtpPtr->ourRtpIndex;
				} else index = srtpPtr->ourRtcpIndex;

				/* Remove the old destination */
            if(rvSrtpDbDestRemove(&srtpPtr->db, stream) != RV_RTPSTATUS_Succeeded) {
					rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpSsrcChanged: Failed to remove a destination"));
            }

				/* Add the new one */
            if(rvSrtpDbDestAdd(&srtpPtr->db, newSsrc, isRtp, index, &address) == NULL) {
					rvSrtpLogError ((rvSrtpLogSourcePtr, "rvSrtpSsrcChanged: Failed to add a destination"));
            }

				/* Hash table changed, we have to start from the beginning */
				break;
			}
		}
		if(stream == NULL)
			break; /* We reached the end of the list */
	}

	srtpPtr->ourSsrc = newSsrc;
	RvMutexUnlock(&srtpPtr->srtpLock, NULL);
    RV_UNUSED_ARG(thisPtr);

	return RV_RTPSTATUS_Succeeded;
}

static RvUint32 rvSrtpGetPaddingSize(RvRtpSession *thisPtr, void *encryptionPluginData, RvBool forRtp)
{
	RvSrtp *srtpPtr;
	RvSize_t result;

	srtpPtr = (RvSrtp *)encryptionPluginData;

	RvMutexLock(&srtpPtr->srtpLock, NULL);
	result = rvSrtpProcessGetPaddingSize(&srtpPtr->packetProcess, forRtp);
	RvMutexUnlock(&srtpPtr->srtpLock, NULL);
    RV_UNUSED_ARG(thisPtr);

	return (RvUint32)result;
}

static RvUint32 rvSrtpGetHeaderSize(RvRtpSession *thisPtr, void *encryptionPluginData, RvBool forRtp)
{
	RvSrtp *srtpPtr;
	RvSize_t result;

	srtpPtr = (RvSrtp *)encryptionPluginData;

	RvMutexLock(&srtpPtr->srtpLock, NULL);
	result = rvSrtpProcessGetHeaderSize(&srtpPtr->packetProcess, forRtp);
	RvMutexUnlock(&srtpPtr->srtpLock, NULL);
    RV_UNUSED_ARG(thisPtr);

	return (RvUint32)result;
}

static RvUint32 rvSrtpGetFooterSize(RvRtpSession *thisPtr, void *encryptionPluginData, RvBool forRtp)
{
	RvSrtp *srtpPtr;
	RvSize_t result;

	srtpPtr = (RvSrtp *)encryptionPluginData;

	RvMutexLock(&srtpPtr->srtpLock, NULL);
	result = rvSrtpProcessGetFooterSize(&srtpPtr->packetProcess, forRtp);
	RvMutexUnlock(&srtpPtr->srtpLock, NULL);
    RV_UNUSED_ARG(thisPtr);

	return (RvUint32)result;
}

static RvUint32 rvSrtpGetTransmitSize(RvRtpSession *thisPtr, void *encryptionPluginData, RvUint32 packetSize, RvUint32 rtpHeaderSize, RvBool encrypt, RvBool forRtp)
{
	RvSrtp *srtpPtr;
	RvSize_t result;

	srtpPtr = (RvSrtp *)encryptionPluginData;

	RvMutexLock(&srtpPtr->srtpLock, NULL);
	result = rvSrtpProcessGetTransmitSize(&srtpPtr->packetProcess, packetSize, rtpHeaderSize, encrypt, forRtp);
	RvMutexUnlock(&srtpPtr->srtpLock, NULL);
    RV_UNUSED_ARG(thisPtr);

	return (RvUint32)result;
}

static void rvSrtpRtpSeqNumChanged(RvRtpSession* thisPtr, void *encryptionPluginData, RvUint16 sequenceNum, RvUint32 roc)
{
	RvSrtp *srtpPtr;

	srtpPtr = (RvSrtp *)encryptionPluginData;

	RvMutexLock(&srtpPtr->srtpLock, NULL);
	srtpPtr->ourRtpIndex = rvSrtpRtpSeqNumToIndex(sequenceNum, roc);
	RvMutexUnlock(&srtpPtr->srtpLock, NULL);
    RV_UNUSED_ARG(thisPtr);
}

static void rvSrtpRtcpIndexChanged(RvRtcpSession* thisPtr, void *encryptionPluginData, RvUint32 index)
{
	RvSrtp *srtpPtr;

	srtpPtr = (RvSrtp *)encryptionPluginData;

	RvMutexLock(&srtpPtr->srtpLock, NULL);
	srtpPtr->ourRtcpIndex = rvSrtpRtcpIndexConvert(index);
	RvMutexUnlock(&srtpPtr->srtpLock, NULL);
    RV_UNUSED_ARG(thisPtr);
}

/****** Utility Function *********/

/* Convert string to address since this core is missing these functions. */
/* We don't even have all the functions we need, but we'll do our best. */
static RvAddress *rvSrtpAddressConstructByName(RvAddress *addressPtr, RvChar *addressString, RvUint16 port)
{
	RvAddress *result;

	/* We can't tell, so try IPV4 first */
	RvAddressConstruct(RV_ADDRESS_TYPE_IPV4, addressPtr);
	result = RvAddressSetIpPort(addressPtr, port);

	/* If addressString is NULL, we want ANYADDRESS, we'll assume IPV4 but */
	/* core really needs to decide on IPV4/IPV6. */
	if(addressString == NULL)
		return result;

	/* Try the string as an IPV4 address */
	result = RvAddressSetString(addressString, addressPtr);
	if(result != NULL)
		return result; /* IPV4 works */

	/* Try IPV6 */
	RvAddressChangeType(RV_ADDRESS_TYPE_IPV6, addressPtr);
	RvAddressSetIpPort(addressPtr, port); /* ChangeType loses port info */
	result = RvAddressSetString(addressString, addressPtr);
	if(result != NULL)
		return result; /* IPV6 works */

	/* It must be a name, but we don't have os independent gethostbyname, so we lose.*/
	RvAddressDestruct(addressPtr);
	return NULL;
}
