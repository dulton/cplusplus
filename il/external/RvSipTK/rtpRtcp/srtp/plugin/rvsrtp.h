/************************************************************************
 File Name	   : rvsrtp.h
 Description   :
*************************************************************************
 Copyright (c)	2005 , RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVision Inc.
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVision Inc.

 RADVision Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
*************************************************************************/
#if !defined(RVSRTP_H)
#define RVSRTP_H

#include "rvtypes.h"
//#include "rvconfigext.h"
#include "rvmutex.h"
#include "rvrtpstatus.h"
#include "rtp.h"
#include "rtcp.h"
#include "rvsrtpaesplugin.h"
#include "rvsrtpdb.h"
#include "rvsrtpprocess.h"
#include "rvsrtpkeyderivation.h"
#include "rvsrtpssrcvalidation.h"
#include "rvsrtpencryptionplugin.h"

#ifdef __cplusplus
extern "C" {
#endif


/*$
{package:
	{name: RvSrtpEncryptionPlugin}
	{superpackage: RvRtp}
	{include: rvsrtp.h}
	{description:
        {p: This is an RTP/RTCP encryption plugin that implements
			the SRTP/SRTCP protocol.}
		{p: The user application interface to this plugin is through an
			RvSrtp object. The RvSrtp functions are used in addition to
			the standard RTP/RTCP interface functions and do not change
			the operation of the standard RTP/RTCP interface.}
        {p: {bold:Plugin Usage}}
		{p: In order to provide all of the features of SRTP/SRTCP and
			to allow an application to properly interface it to a key
			management application, this plugin is reasonable complex.
			However, once the basic concept of how to use the plugin to
			create keys, destinations, and sources, is understood along
			with how to map keys to destinations and sources, it is not too
			difficult to create an application. The following outline shows
			the basic steps an application needs to follow in order to
			create and use the SRTP/SRTCP plugin with an RTP/RTCP session.
			Remember that all sessions are completely independent of each
			other, and an instance of the plugin needs to be created for
			each session that requires the use of SRTP/SRTCP.}
        {bulletlist:
            {item: {bold:Construct and configure RTP/RTCP:} Construct
					and configure the RTP/RTCP session normally.}
            {item: {bold:SRTP/SRTCP plugin construct:} An SRTP/SRTCP
					plugin is constructed using the rvSrtpConstruct function.}
			{item: {bold:SRTP/SRTCP configuration:} Configure the
					SRTP/SRTCP plugin using the configuration functions of the
					RvSrtp interface. There are no required configuration
					settings and the plugin will function using default values.
					Note that there are also a set of advanced configuration
					functions which allow more precise control over the internal
					memory and hash tables used by the plugin. The default
					values for these advanced parameters will be sufficient for
					most applications although users in very small footprint
					environments and very large scale environments may wish to
					adjust them.}
            {item: {bold:SRTP/SRTCP plugin registration:} Use the
					rvSrtpRegister function to register the plugin with an
					RTP/RTCP session. Once registered, the plugin is fully
					functional and will respond to anything done by the
					RTP/RTCP session. The plugin must be registered
					before the RTP/RTCP session is opened.}
            {item: {bold:Normal operation:} During normal operation of
					a session using the SRTP/SRTCP plugin, the RTP/RTCP
					stack is used just as it normally is. All that is
					required in addition is that the application notify
					the SRTP/SRTCP plugin of keys, sources,
					destinations, and which sources and destinations
					should use which keys (and when). Functions are
					provided for managing all of these things.}
            {item: {bold:SRTP/SRTCP plugin unregistration:} Once the
					RTP/RTCP session is closed, use the rvSrtpUnregister
					function to unregister the plugin with its RTP/RTCP
					session.}
            {item: {bold:SRTP/SRTCP plugin destruct:} Destructing the
					plugin will clean up all resources associated with that
					instance of the plugin. To do this, simply call the
					rvSrtpDestruct function.}
            {item: {bold:Destruct the RTP/RTCP session:} The normal
					destruction of the RTP/RTCP session may now be done.}
		}
		{p: During a session, a number of SRTP/SRTCP related items need
			to be managed by the user's application. The basic items that
			need to be managed are Master Keys, Remote Sources, and
			Destinations. Each of these has a set of functions for managing
			each of these items. The user's application is COMPLETELY
			responsible for these items, meaning that the user's
			application must not only add items as needed, but must also
			remove them when they are no longer needed. The SRTP/SRTCP
			plugin will not automatically clean up any of these items. Here
			is an overview of these items:}
        {bullet list:
            {item: {bold:Master Keys:} Master keys are the primary set
					of information that need to be exchanged between endpoints
					in a session. How these are exchanged is up to the
					application and is outside the scope of SRTP/SRTCP. Master
					keys are added using the rvSrtpMasterKeyAdd and removed
					with the rvSrtpMasterKeyRemove function. The entire master
					key database can be cleared using the
					rvSrtpMasterKeyRemoveAll function. It's important to
					remember that the sizes of the keys must be exactly the
					same size that was set when configuring the plugin. It's
					also important to remember that removing a key will also
					remove any contexts (see below) that may be generated from
					that key, even if that context is currently being used.}
            {item: {bold:Remote Sources:} Remote sources are entities
					that the local session expects to receives SRTP or SRTCP
					packets from. While the standard RTP/RTCP stack will
					automatically detetect remote sources, SRTP/SRTCP will not
					because it needs to be given information about that remote
					source (such as what key to use) in order to process even
					the first packet that will be receieved from that remote
					source. It is important to note that SRTP and SRTCP remote
					sources are completely independent of each other and must
					be added individually, using the rvSrtpRemoteSourceAddRtp
					and rvSrtpRemoteSourceAddRtcp functions. All operations on
					remote sources identify the remote source by their SSRC
					(and whether it is an SRTP or SRTCP source). Sources are
					removed using the rvSrtpRemoteSourceRemove function and all
					sources in the session can be removed with the
					rvSrtpRemoteSourceRemoveAll function.}
            {item: {bold:Destinations:} Destinations are entities
					that the local session will be sending SRTP or SRTCP
					packets to. These are separate from the destinations that
					are set for standard RTP/RTCP, although under most
					circumstances a destination will be set in SRTP/SRTCP for
					each destination set for RTP/RTCP. Note that SRTP and SRTCP
					destinations are set separately, by using the rvSrtpDestinationAddRtp
					and rvSrtpDestinationAddRtcp functions. All operations on
					destinations identify the destination by its IP address string
					and port number. Destinations are removed using the
					rvSrtpDestinationRemove function and all destinations in
					the session can be removed with the rvSrtpDestinationRemoveAll
					function.}
		}
		{p: Once the system knows about Master Keys, Remote Sources,
			and destinations, then the application must program each remote
			source and destination to tell it which Master Key to use and
			when to use it. This is done by creating a "timeline" for each
			remote source and destination indicating at what point in time
			that source or destination should switch to a given Master
			Key. Each block in the timeline where a given key is used is
			called a context.}
		{p: The "time" in the timeline is measured by the index value,
			which is the 48 bit index for RTP (ROC || Seq#), and the 31 bit
			index for RTCP. Since both of these indexes can wrap, it can be
			confusing as to what values are considered "future" and what
			values are "past". The current index value is considered the
			"current" time and any earlier index outside the value
			configured by the rvSrtpSetRtpHistory and rvSrtpSetRtcpHistory
			functions are considered "future".}
		{p: The functions rvSrtpDestinationChangeKey and
			rvSrtpRemoteSourceChangeKey change the Masker Key to be used
			for the next packet to be processed. To schedule a change at
			some point in the future, use the rvSrtpDestinationChangeKeyAt
			and rvSrtpRemoteSourceChangeKeyAt functions. When scheduling a
			future change, make sure that the index specified is really in
			the future since changing keys in the past is not allowed.}
		{p: It is allowed to change a key to NULL by specifying NULL as
			the master key. Thus the expiration of a key could be programed
			by simply switching to a NULL key at the point in the timeline
			where the Master Key has expired. A NULL key will indicate that
			no packets are to be processed in the given range, it does NOT
			indicate that NULL encryption should be used.}
		{p: There is no "remove key change" function. However, a key
			change can be overwritten by simply requesting a different key
			change (including to a NULL key) at the same point in the
			timeline. So if the timeline is programed to switch from key A
			to key B at a time X, programming a key change to A at time X
			will remove the key switch and key A will continue to be used
			at and beyond point X (until the next programmed change).}
		{p: If a master key is removed, any scheduled key changes that
			are supposed to use that key will be changed to NULL keys.
			Also, if, at any time, you need to completely clear the
			timeline of a remote source or destination, it can be done with
			the rvSrtpDestinationClearAllKeys and rvSrtpRemoteSourceClearAllKeys
			functions.}
		{p: An advanced function of the timeline programming functions
			is the "shareTrigger" flag. What this does is allow things that
			share the same master key to all switch to a new master key at
			the same time. When a key change is programmed, if
			"shareTrigger" is set to RV_TRUE, then any other source or
			destination currently using the same existing Master Key, will
			switch to this new Master Key. So, for example, if you have an
			RTP destination and an RTCP destination both set to use Master
			Key A and you program the RTP destination to switch to Masker
			Key B at a given point in time, when the RTP destination
			switches to Masker Key B, the RTCP destination will also switch
			to Master Key B (with no need to schedule a key change for the
			RTCP destination).}
		{p: Something to remember when programming key changes is that
			SRTP/SRTCP depends on the first packet being processed to
			generate session keys, thus it is critical that key switches be
			coordinated between the sender and receiver. If the key
			switches are not perfectly synchronized, the effect will be
			that the receiver will not be able to decrypt the packets and
			the stream will effectively be dropped.}
	}
}
$*/
/*$
{type:
	{name: RvSrtp}
	{superpackage: RvSrtpEncryptionPlugin}
	{include: rvsrtp.h}
	{description:
        {p: This is the object that represents an instance of
			SRTP/SRTCP. There must be one SRTP/SRTCP instance for each
			RTP/RTCP session that will be using SRTP/SRTCP.}
	}
}
$*/
typedef struct {
	RvRtpSession            rtpSession;             /* Session plugin is registered with */
	RvRtcpSession           rtcpSession;            /* Session plugin is registered with */
	RvSrtpEncryptionPlugIn  plugIn;                 /* RTP/RTCP encryption plugin */
	RvSrtpDb                db;                     /* session database */
	RvSrtpKeyDerivation     keyDerivation;          /* session key derivation */
	RvSrtpProcess           packetProcess;          /* packet processing */
	RvSrtpSsrcValidation    ssrcValidation;         /* SSRC validation */
	RvMutex                 srtpLock;               /* plugin lock (required for simultaneous user and RTP/RTCP access) */
	RvBool                  registering;            /* RV_TRUE when register is in progress */
	RvUint32                ourSsrc;                /* SSRC of master local source */
	RvUint64                ourRtpIndex;            /* Index (48 bits) of next RTP packet that master local source will send */
	RvUint32                ourRtcpIndex;           /* Index (31 bits) of next RTCP packet that master local source will send */

	/* Configuration parameters */
#ifndef UPDATED_BY_SPIRENT
	RvSize_t                keyDerivationRate;      /* derivation rate of session keys (same for all masters) */
#else
	RvSize_t                keyDerivationRateLocal; /* derivation rate of session keys for local master keys */
	RvSize_t                keyDerivationRateRemote;/* derivation rate of session keys for remote master keys */
#endif // UPDATED_BY_SPIRENT

	/* Database Configuration Parameters */
	RvSrtpDbPoolConfig      poolConfig;             /* DB pool configuration parameters */
	RvSize_t                mkiSize;                /* Master key mki size */
	RvSize_t                keySize;                /* Master key size */
	RvSize_t                saltSize;               /* Master salt size */
	RvSize_t                rtpEncryptKeySize;      /* RTP encryption key size */
	RvSize_t                rtpAuthKeySize;         /* RTP authentication key size */
	RvSize_t                rtpSaltSize;            /* RTP salt size */
	RvSize_t                rtcpEncryptKeySize;     /* RTCP encryption key size */
	RvSize_t                rtcpAuthKeySize;        /* RTCP authentication key size */
	RvSize_t                rtcpSaltSize;           /* RTCP salt size */
	RvUint64                rtpReplayListSize;      /* Size of RTP replay list (in packets, minimum of 64, default of 64) */
	RvUint64                rtcpReplayListSize;     /* Size of RTCP replay list (in packets, minimum of 64, default of 64) */
	RvUint64                rtpHistorySize;         /* RTP context history size (in packets, minimum of replay list size) */
	RvUint64                rtcpHistorySize;        /* RTCP context history size (in packets, minimum of replay list size) */

	/* Key Derivation Configuration Parameters */
	RvInt                   keyDerivationAlg;       /* derivation algorithm for session keys */

	/* Processing Configuration Parameters */
	RvSize_t                prefixLen;              /* size of the packet prefix */
	RvInt                   rtpEncAlg;              /* RTP encryption algorithm */
	RvBool                  rtpUseMki;              /* RV_TRUE = include mki in packet */
	RvInt                   rtpAuthAlg;             /* RTP authentication algorithm */
	RvSize_t                rtpAuthTagSize;         /* RTP authentication tag size*/
	RvInt                   rtcpEncAlg;             /* RTCP encryption algorithm */
	RvBool                  rtcpUseMki;             /* RV_TRUE = include mki in packet */
	RvInt                   rtcpAuthAlg;            /* RTCP authentication algorithm */
	RvSize_t                rtcpAuthTagSize;        /* RTCP authentication tag size*/
	
#ifdef UPDATED_BY_SPIRENT
	/* Callbacks */
	RvSrtpMasterKeyEventCB masterKeyEventCB;        /* Call back when master the key deletion is or when threshold is reached */
#endif // UPDATED_BY_SPIRENT


} RvSrtp;

/* Source/Destination types */
#define RV_SRTP_STREAMTYPE_SRTP RV_TRUE
#define RV_SRTP_STREAMTYPE_SRTCP RV_FALSE

/* Key Derivation Algorithms */
#define RV_SRTP_KEYDERIVATIONALG_AESCM RV_SRTP_DERIVATION_AESCM

/* Encryption Algorithms */
#define RV_SRTP_ENCYRPTIONALG_NULL  RV_SRTPPROCESS_ENCYRPTIONTYPE_NULL
#define RV_SRTP_ENCYRPTIONALG_AESCM RV_SRTPPROCESS_ENCYRPTIONTYPE_AESCM
#define RV_SRTP_ENCYRPTIONALG_AESF8 RV_SRTPPROCESS_ENCYRPTIONTYPE_AESF8

/* Authentication Algorithms */
#define RV_SRTP_AUTHENTICATIONALG_NONE RV_SRTPPROCESS_AUTHENTICATIONTYPE_NONE
#define RV_SRTP_AUTHENTICATIONALG_HMACSHA1 RV_SRTPPROCESS_AUTHENTICATIONTYPE_HMACSHA1

/* Pool types */
#define RV_SRTP_POOLTYPE_FIXED RV_OBJPOOL_TYPE_FIXED
#define RV_SRTP_POOLTYPE_EXPANDING RV_OBJPOOL_TYPE_EXPANDING
#define RV_SRTP_POOLTYPE_DYNAMIC RV_OBJPOOL_TYPE_DYNAMIC

/* Hash types */
#define RV_SRTP_HASHTYPE_FIXED RV_OBJHASH_TYPE_FIXED
#define RV_SRTP_HASHTYPE_EXPANDING RV_OBJHASH_TYPE_EXPANDING
#define RV_SRTP_HASHTYPE_DYNAMIC RV_OBJHASH_TYPE_DYNAMIC

/* Basic operations */
RvRtpStatus rvSrtpConstruct(RvSrtp *thisPtr
#ifdef UPDATED_BY_SPIRENT
        ,RvSrtpMasterKeyEventCB masterKeyEventCB
#endif // UPDATED_BY_SPIRENT
);
RvRtpStatus rvSrtpDestruct(RvSrtp *thisPtr);
RvRtpStatus rvSrtpRegister(RvSrtp *thisPtr, RvRtpSession rtpSession, RvSrtpAesPlugIn* AESPlugin);
RvRtpStatus rvSrtpUnregister(RvSrtp *thisPtr);
RvRtpSession rvSrtpGetSession(RvSrtp *thisPtr);
void rvSrtpGetSoftwareVersion(RvUint8 *majorVersionPtr, RvUint8 *minorVersionPtr, RvUint8 *engineeringReleasePtr, RvUint8 *patchLevelPtr);

/* Master key management */
#ifdef UPDATED_BY_SPIRENT
RvRtpStatus rvSrtpMasterKeyAdd(RvSrtp *thisPtr, RvUint8 *mki, RvUint8 *key, RvUint8 *salt,
        RvUint64 lifetime, RvUint64 threshold, RvUint8 direction);
RvRtpStatus rvSrtpMasterKeyRemove(RvSrtp *thisPtr, RvUint8 *mki, RvUint8 direction);
RvSize_t rvSrtpMasterKeyGetContextCount(RvSrtp *thisPtr, RvUint8 *mki, RvUint8 direction);
#else
RvRtpStatus rvSrtpMasterKeyAdd(RvSrtp *thisPtr, RvUint8 *mki, RvUint8 *key, RvUint8 *salt);
RvRtpStatus rvSrtpMasterKeyRemove(RvSrtp *thisPtr, RvUint8 *mki);
RvSize_t rvSrtpMasterKeyGetContextCount(RvSrtp *thisPtr, RvUint8 *mki);
#endif // UPDATED_BY_SPIRENT

RvRtpStatus rvSrtpMasterKeyRemoveAll(RvSrtp *thisPtr);

/* Destination Management */
RvRtpStatus rvSrtpDestinationAddRtp(RvSrtp *thisPtr, RvChar *address, RvUint16 port);
RvRtpStatus rvSrtpDestinationAddRtcp(RvSrtp *thisPtr, RvChar *address, RvUint16 port);
RvRtpStatus rvSrtpDestinationRemove(RvSrtp *thisPtr, RvChar *address, RvUint16 port);
RvRtpStatus rvSrtpDestinationRemoveAll(RvSrtp *thisPtr);
RvRtpStatus rvSrtpDestinationChangeKey(RvSrtp *thisPtr, RvChar *address, RvUint16 port, RvUint8 *mki, RvBool shareTrigger);
RvRtpStatus rvSrtpDestinationChangeKeyAt(RvSrtp *thisPtr, RvChar *address, RvUint16 port, RvUint8 *mki, RvUint64 index, RvBool shareTrigger);
RvRtpStatus rvSrtpDestinationClearAllKeys(RvSrtp *thisPtr, RvChar *address, RvUint16 port);
RvRtpStatus rvSrtpDestinationGetIndex(RvSrtp *thisPtr, RvChar *address, RvUint16 port, RvUint64 *indexPtr);

/* Source Management */
RvRtpStatus rvSrtpRemoteSourceAddRtp(RvSrtp *thisPtr, RvUint32 ssrc, RvUint32 roc, RvUint32 sequenceNum);
RvRtpStatus rvSrtpRemoteSourceAddRtcp(RvSrtp *thisPtr, RvUint32 ssrc, RvUint32 index);
RvRtpStatus rvSrtpRemoteSourceRemove(RvSrtp *thisPtr, RvUint32 ssrc, RvBool sourceType);
RvRtpStatus rvSrtpRemoteSourceRemoveAll(RvSrtp *thisPtr);
RvRtpStatus rvSrtpRemoteSourceChangeKey(RvSrtp *thisPtr, RvUint32 ssrc, RvBool sourceType, RvUint8 *mki, RvBool shareTrigger);
RvRtpStatus rvSrtpRemoteSourceChangeKeyAt(RvSrtp *thisPtr, RvUint32 ssrc, RvBool sourceType, RvUint8 *mki, RvUint64 index, RvBool shareTrigger);
RvRtpStatus rvSrtpRemoteSourceClearAllKeys(RvSrtp *thisPtr, RvUint32 ssrc, RvBool sourceType);
RvRtpStatus rvSrtpRemoteSourceGetIndex(RvSrtp *thisPtr, RvUint32 ssrc, RvBool sourceType, RvUint64 *indexPtr);

/* Destination management for translators only */
RvRtpStatus rvSrtpForwardDestinationAddRtp(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvUint32 roc, RvUint32 sequenceNum);
RvRtpStatus rvSrtpForwardDestinationAddRtcp(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvUint32 index);
RvRtpStatus rvSrtpForwardDestinationRemove(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port);
RvRtpStatus rvSrtpForwardDestinationChangeKey(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvChar *mki, RvBool shareTrigger);
RvRtpStatus rvSrtpForwardDestinationChangeKeyAt(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvChar *mki, RvUint64 index, RvBool shareTrigger);
RvRtpStatus rvSrtpForwardDestinationClearAllKeys(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port);
RvRtpStatus rvSrtpForwardDestinationGetIndex(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvUint64 *indexPtr);
RvRtpStatus rvSrtpForwardDestinationSsrcChanged(RvSrtp *thisPtr, RvUint32 oldSsrc, RvUint32 newSsrc);

/* Translator forwarding functions (use instead of RTP/RTCP forwarding functions) */
RvRtpStatus rvSrtpForwardRtp(RvSrtp *thisPtr, RvChar *bufStartPtr, RvUint32 bufSize, RvChar *dataStartPtr, RvUint32 dataSize, RvRtpParam *headerPtr, RvUint32 roc);
RvRtpStatus rvSrtpForwardRtcp(RvSrtp *thisPtr, RvChar *bufStartPtr, RvUint32 bufSize, RvChar *dataStartPtr, RvUint32 dataSize, RvUint32 index);

/* Configuration */
RvRtpStatus rvSrtpSetMasterKeySizes(RvSrtp *thisPtr, RvSize_t mkiSize, RvSize_t keySize, RvSize_t saltSize);
#ifndef UPDATED_BY_SPIRENT
RvRtpStatus rvSrtpSetKeyDerivation(RvSrtp *thisPtr, RvInt keyDerivationAlg, RvUint32 keyDerivationRate);
#else
RvRtpStatus rvSrtpSetKeyDerivation(RvSrtp *thisPtr, RvInt keyDerivationAlg, RvUint32 keyDerivationRateLocal,
        RvUint32 keyDerivationRateRemote);
#endif // UPDATED_BY_SPIRENT
RvRtpStatus rvSrtpSetPrefixLength(RvSrtp *thisPtr, RvSize_t prefixLength);
RvRtpStatus rvSrtpSetRtpEncryption(RvSrtp *thisPtr, RvInt encryptType, RvBool useMki);
RvRtpStatus rvSrtpSetRtcpEncryption(RvSrtp *thisPtr, RvInt encryptType, RvBool useMki);
RvRtpStatus rvSrtpSetRtpAuthentication(RvSrtp *thisPtr, RvInt authType, RvSize_t tagSize);
RvRtpStatus rvSrtpSetRtcpAuthentication(RvSrtp *thisPtr, RvInt authType, RvSize_t tagSize);
RvRtpStatus rvSrtpSetRtpKeySizes(RvSrtp *thisPtr, RvSize_t encryptKeySize, RvSize_t authKeySize, RvSize_t saltSize);
RvRtpStatus rvSrtpSetRtcpKeySizes(RvSrtp *thisPtr, RvSize_t encryptKeySize, RvSize_t authKeySize, RvSize_t saltSize);
RvRtpStatus rvSrtpSetRtpReplayListSize(RvSrtp *thisPtr, RvUint64 replayListSize);
RvRtpStatus rvSrtpSetRtcpReplayListSize(RvSrtp *thisPtr, RvUint64 replayListSize);
RvRtpStatus rvSrtpSetRtpHistory(RvSrtp *thisPtr, RvUint64 historySize);
RvRtpStatus rvSrtpSetRtcpHistory(RvSrtp *thisPtr, RvUint64 historySize);

/* Advanced Configuration */
RvRtpStatus rvSrtpSetMasterKeyPool(RvSrtp *thisPtr, RvInt poolType, RvSize_t pageItems, RvSize_t pageSize, RvSize_t maxItems, RvSize_t minItems, RvSize_t freeLevel, RvMemory *region);
RvRtpStatus rvSrtpSetStreamPool(RvSrtp *thisPtr, RvInt poolType, RvSize_t pageItems, RvSize_t pageSize, RvSize_t maxItems, RvSize_t minItems, RvSize_t freeLevel, RvMemory *region);
RvRtpStatus rvSrtpSetContextPool(RvSrtp *thisPtr, RvInt poolType, RvSize_t pageItems, RvSize_t pageSize, RvSize_t maxItems, RvSize_t minItems, RvSize_t freeLevel, RvMemory *region);
RvRtpStatus rvSrtpSetMasterKeyHash(RvSrtp *thisPtr, RvInt hashType, RvSize_t startSize, RvMemory *region);
RvRtpStatus rvSrtpSetSourceHash(RvSrtp *thisPtr, RvInt hashType, RvSize_t startSize, RvMemory *region);
RvRtpStatus rvSrtpSetDestHash(RvSrtp *thisPtr, RvInt hashType, RvSize_t startSize, RvMemory *region);

/* Function Docs */
/*$
{function:
    {name:    rvSrtpConstruct}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function constructs the SRTP/SRTCP plugin.}
        {p: Once constructed, configure the plugin, then register it
			with the RTP/RTCP session using the rvSrtpRegister function.}
    }
    {proto: RvRtpStatus rvSrtpConstruct(RvSrtp *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object to construct.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the plugin is constructed sucessfully. }
}
$*/
/*$
{function:
    {name:    rvSrtpDestruct}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function destructs the SRTP/SRTCP plugin.}
        {p: The plugin must be unregistered in order to destruct it.}
    }
    {proto: RvRtpStatus rvSrtpDestruct(RvSrtp *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object to destruct.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the plugin is destructed sucessfully. }
}
$*/
/*$
{function:
    {name:    rvSrtpRegister}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function registers the SRTP/SRTCP plugin with an RTP/RTCP session.}
        {p: The plugin must be registered before the session is opened.}
    }
    {proto: RvRtpStatus rvSrtpRegister(RvSrtp *thisPtr, RvRtpSession *sessionPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object to register.}}
        {param: {n:sessionPtr} {d:The session to register the plugin with.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the plugin is registered sucessfully. }
}
$*/
/*$
{function:
    {name:    rvSrtpUnregister}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function unregisters the SRTP/SRTCP plugin.}
    }
    {proto: RvRtpStatus rvSrtpUnregister(RvSrtp *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object to unregister.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the plugin is unregistered sucessfully. }
}
$*/
/*$
{function:
    {name:    rvSrtpGetSession}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function gets the RTP/RTCP session that the plugin is
			registered with.}
    }
    {proto: RvRtpSession *rvSrtpGetSession(RvSrtp *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
    }
    {returns: The session the plugin is registered with (NULL if not registered). }
}
$*/
/*$
{function:
    {name:    rvSrtpGetSoftwareVersion}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This method gets the version information for the SRTP/SRTCP encryption plugin module.}
    }
    {proto: void rvSrtpGetSoftwareVersion(RvUint8 *majorVersionPtr, RvUint8 *minorVersionPtr, RvUint8 *engineeringReleasePtr, RvUint8 *patchLevelPtr);}
    {params:
        {param: {n: majorVersionPtr}       {d: The RvUint8 pointed to by this parameter will
                                               be filled in with the module's major version number.}}
        {param: {n: minorVersionPtr}       {d: The RvUint8 pointed to by this parameter will
                                               be filled in with the module's minor version number.}}
        {param: {n: engineeringReleasePtr} {d: The RvUint8 pointed to by this parameter will
                                               be filled in with the module's engineering release number.}}
        {param: {n: patchLevelPtr}         {d: The RvUint8 pointed to by this parameter will
                                               be filled in with the module's patch level.}}
    }
}
$*/
/*$
{function:
    {name:    rvSrtpMasterKeyAdd}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function adds a new master key (and salt) to the session.}
        {p: The length of the mki, key, and salt must be the lengths
			configured with the rvSrtpSetMasterKeySizes function.}
		{p: Even if the MKI is not to be included in the SRTP/SRTCP
			packets, a unique value must be specified since this value
			will be used to identify this key when it is to be
			removed.}
		{p: The mki, key, and salt will be copied, so the user does not
			need to maintain the original copies of this data.}
    }
    {proto: RvRtpStatus rvSrtpMasterKeyAdd(RvSrtp *thisPtr, RvChar *mki, RvChar *key, RvChar *salt);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:mki} {d:Pointer to the MKI indentifier for the key.}}
        {param: {n:key} {d:Pointer to the master key.}}
        {param: {n:salt} {d:Pointer to the master salt.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the key has been added.}
}
$*/
/*$
{function:
    {name:    rvSrtpMasterKeyRemove}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function removes a master key (and salt) from the session.}
    }
    {proto: RvRtpStatus rvSrtpMasterKeyRemove(RvSrtp *thisPtr, RvChar *mki);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:mki} {d:Pointer to the MKI indentifier for the key.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the key has been removed.}
}
$*/
/*$
{function:
    {name:    rvSrtpMasterKeyRemoveAll}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function removes all master keys (and salts) from the session.}
    }
    {proto: RvRtpStatus rvSrtpMasterKeyRemoveAll(RvSrtp *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if all the keys have been removed.}
}
$*/
/*$
{function:
    {name:    rvSrtpMasterKeyGetContextCount}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function returns the number of contexts that currently
			refer to the specified master key. This includes contexts
			that are scheduled to be created in the future along with
			those remaining in the history.}
    }
    {proto: RvRtpStatus rvSrtpMasterKeyGetContextCount(RvSrtp *thisPtr, RvChar *mki);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:mki} {d:Pointer to the MKI indentifier for the key.}}
    }
    {returns: The number of contexts referring to this key.}
}
$*/
/*$
{function:
    {name:    rvSrtpDestinationAddRtp}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function adds a new RTP destination for the master local
			source to the session.}
		{p: A destination needs to be created for each destination
			address that the session will be sending to. If SRTP and
			SRTCP are being used, a separate destination needs to be
			added for both (since, at a minumum, the port number
			will be different).}
		{p: If the ssrc of the master local source changes, all key
			mappings for the destination will be cleared and the destination
			will be reinitialized to use the new ssrc.}
        {p: Translators that need to create destinations for other
			sources should use the rvSrtpForwardDestinationAddRtp function.}
    }
    {proto: RvRtpStatus rvSrtpDestinationAddRtp(RvSrtp *thisPtr, RvChar *address, RvUint16 port);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:address} {d:Pointer to the address string containing the destination IP address.}}
        {param: {n:port}    {d:IP port number for the destination IP address.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the RTP destination has been added.}
}
$*/
/*$
{function:
    {name:    rvSrtpDestinationAddRtcp}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function adds a new RTCP destination for the master local
			source to the session.}
		{p: A destination needs to be created for each destination
			address that the session will be sending to. If SRTP and
			SRTCP are being used, a separate destination needs to be
			added for both (since, at a minumum, the port number
			will be different).}
		{p: If the ssrc of the master local source changes, all key
			mappings for the destination will be cleared and the destination
			will be reinitialized to use the new ssrc.}
        {p: Translators that need to create destinations for other
			sources should use the rvSrtpForwardDestinationAddRtcp function.}
    }
    {proto: RvRtpStatus rvSrtpDestinationAddRtcp(RvSrtp *thisPtr, RvChar *address, RvUint16 port);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:address} {d:Pointer to the address string containing the destination IP address.}}
        {param: {n:port}    {d:IP port number for the destination IP address.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the RTCP destination has been added.}
}
$*/
/*$
{function:
    {name:    rvSrtpDestinationRemove}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function removes a destination for the master local
			source from the session.}
        {p: Translators that need to remove destinations for other
			sources should use the rvSrtpForwardDestinationRemove function.}
    }
    {proto: RvRtpStatus rvSrtpDestinationRemove(RvSrtp *thisPtr, RvChar *address, RvUint16 port);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:address} {d:Pointer to the address string containing the destination IP address.}}
        {param: {n:port} {d:IP port number for the destination IP address.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the destination has been removed.}
}
$*/
/*$
{function:
    {name:    rvSrtpDestinationRemoveAll}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function remove all destinations from the session.}
        {p: Destinations added by translators for other sources are also removed.}
    }
    {proto: RvRtpStatus rvSrtpDestinationRemoveAll(RvSrtp *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if all the destinations have been removed.}
}
$*/
/*$
{function:
    {name:    rvSrtpDestinationChangeKey}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function changes the master key that should be used to
			send to the specified destination. The new key will be used
			until changed, either by another call to this function or at a
			key change point specified by the rvSrtpDestinationChangeKeyAt
			function.}
		{p: If shareTrigger is set to RV_TRUE, every source and
			destination in the session that shares the destinations old
			master key will switch to this same new master key when
			this destination switches to this new key.}
		{p: If mki is set to NULL, that indicates that the destination
			has no key and any packets to be sent to that destination can
			not be sent.}
		{p: A destination for the specified address must have been
			created using the rvSrtpDestinationAddRtp or
			rvSrtpDestinationAddRtcp functions before keys can
			be set for that destination.}
		{p: If the ssrc of the master local source changes, all key
			mappings for the destination will be cleared and the destination
			will be reinitialized to use the new ssrc.}
        {p: Translators that need to create destination key mappings for other
			sources should use the rvSrtpForwardDestinationChangeKey function.}
    }
    {proto: RvRtpStatus rvSrtpDestinationChangeKey(RvSrtp *thisPtr, RvChar *address, RvUint16 port, RvChar *mki, RvBool shareTrigger);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:address} {d:Pointer to the address string containing the destination IP address.}}
        {param: {n:port} {d:IP port number for the destination IP address.}}
        {param: {n:mki} {d:Pointer to the MKI indentifier for the master key to use.}}
        {param: {n:shareTrigger} {d:RV_TRUE if anything sharing keys should switch keys when this destination switches, RV_FALSE otherwise.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the destination has been set to use the specified key.}
}
$*/
/*$
{function:
    {name:    rvSrtpDestinationChangeKeyAt}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets a point in time (based on the index) at
			which the specified master key should be used to send to
			the specified destination. The new key will be used until
			changed, either by another change specified by a call to
			this function or by a call rvSrtpDestinationChangeKey once
			the key is in use.}
		{p: If shareTrigger is set to RV_TRUE, every source and
			destination in the session that shares the destinations old
			master key will switch to this same new master key when
			this destination switches to this new key.}
		{p: Remember that the index is different for an SRTP
			destination as opposed to an SRTCP destination. Use the
			approprate range and values for the type of destination
			specified.}
		{p: IMPORTANT: This function is designed to set new keys for
			future use. However, since the current index value may be
			constantly changing, it is possible that an index value for
			a future key change may not be received until after the
			point of the switchover. Thus, a historic index value is
			allowed, however, because index values are allowed to wrap,
			the user must insure that the index value specified is
			never older than the maximum history length configured for
			the session, otherwise the "historic" key change will
			actually be set for a time in the distant future. That is
			why it is important to set the history length (using
			rvSrtpSetHistory) to a period of time long enough to
			safely allow for any delay in key exchanges.}
		{p: If mki is set to NULL, that indicates that the destination
			has no key and any packets to be sent to that destination can
			not be sent.}
		{p: A destination for the specified address must have been
			created using the rvSrtpDestinationAddRtp or
			rvSrtpDestinationAddRtcp functions before keys can
			be set for that destination.}
		{p: If the ssrc of the master local source changes, all key
			mappings for the destination will be cleared and the destination
			will be reinitialized to use the new ssrc.}
        {p: Translators that need to create destination key mappings for other
			sources should use the rvSrtpForwardDestinationChangeKeyAt function.}
    }
    {proto: RvRtpStatus rvSrtpDestinationChangeKeyAt(RvSrtp *thisPtr, RvChar *address, RvUint16 port, RvChar *mki, RvUint64 index, RvBool shareTrigger);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:address} {d:Pointer to the address string containing the destination IP address.}}
        {param: {n:port} {d:IP port number for the destination IP address.}}
        {param: {n:mki} {d:Pointer to the MKI indentifier for the master key to use.}}
        {param: {n:index} {d:The index at which the key should change.}}
        {param: {n:shareTrigger} {d:RV_TRUE if anything sharing keys should switch keys when this destination switches, RV_FALSE otherwise.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the destination has been set to use the specified key.}
}
$*/
/*$
{function:
    {name:    rvSrtpDestinationClearAllKeys}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function removes all key mappings for a destination.}
        {p: Translators that need to clear destination key mappings for other
			sources should use the rvSrtpForwardDestinationClearAllKeys function.}
    }
    {proto: RvRtpStatus rvSrtpDestinationClearAllKeys(RvSrtp *thisPtr, RvChar *address, RvUint16 port);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:address} {d:Pointer to the address string containing the destination IP address.}}
        {param: {n:port} {d:IP port number for the destination IP address.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpDestinationGetIndex}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function gets the highest index value that has been
			encrypted for a message to be sent to the specified
			destination. Note that this value may be constantly
			changing and should only be used for status or for
			estimating future index values for key changes.}
		{p: Remember that the index is different for an SRTP
			destination as opposed to an SRTCP destination.}
        {p: Translators that need to get this value for other
			sources should use the rvSrtpForwardDestinationGetIndex function.}
    }
    {proto: RvRtpStatus rvSrtpDestinationGetIndex(RvSrtp *thisPtr, RvChar *address, RvUint16 port, RvUint64 *indexPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:address} {d:Pointer to the address string containing the destination IP address.}}
        {param: {n:port} {d:IP port number for the destination IP address.}}
        {param: {n:indexPtr} {d:Pointer to location where index should be stored.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpRemoteSourceAddRtp}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function adds a new remote RTP source to the session.}
		{p: A source needs to be created for each ssrc that the session
			will be receiving RTP from. If SRTCP is also going to be
			received for a given ssrc, a separate remote source needs
			to be added for it using rvSrtpRemoteSourceAddRtcp.}
		{p: If the starting rollover counter value (roc) and/or
			seqeunce number is not available, set them to zero, but be
			aware of the possible implications as explained in the
			SRTP/SRTCP RFC.}
    }
    {proto: RvRtpStatus rvSrtpRemoteSourceAddRtp(RvSrtp *thisPtr, RvUint32 ssrc, RvUint32 roc, RvUint32 sequenceNum);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:ssrc} {d:The ssrc of the remote source.}}
        {param: {n:roc} {d:The initial rollover counter value that will be received.}}
        {param: {n:sequenceNum} {d:The initial sequence number value that will be received.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the remote source has been added.}
}
$*/
/*$
{function:
    {name:    rvSrtpRemoteSourceAddRtcp}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function adds a new remote RTCP source to the session.}
		{p: A source needs to be created for each ssrc that the session
			will be receiving RTP from. If SRTP is also going to be
			received for a given ssrc, a separate remote source needs
			to be added for it using rvSrtpRemoteSourceAddRtp.}
		{p: If the starting index number is not available, set it to
			zero, but be aware of the implications as explained in the
			SRTP/SRTCP RFC.}
    }
    {proto: RvRtpStatus rvSrtpRemoteSourceAddRtcp(RvSrtp *thisPtr, RvUint32 ssrc, RvUint32 index);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:ssrc} {d:The ssrc of the remote source.}}
        {param: {n:index} {d:The initial index value (31 bits) that will be received.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the remote source has been added.}
}
$*/
/*$
{function:
    {name:    rvSrtpRemoteSourceRemove}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function removes a remote source from the session.}
    }
    {proto: RvRtpStatus rvSrtpRemoteSourceRemove(RvSrtp *thisPtr, RvUint32 ssrc, RvBool sourceType);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:ssrc} {d:The ssrc of the remote source.}}
        {param: {n:sourceType} {d:RV_SRTP_STREAMTYPE_SRTP or RV_SRTP_STREAMTYPE_SRTCP.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the remote source has been removed.}
}
$*/
/*$
{function:
    {name:    rvSrtpRemoteSourceRemoveAll}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function remove all remote sources from the session.}
    }
    {proto: RvRtpStatus rvSrtpRemoteSourceRemoveAll(RvSrtp *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if all the remote sources have been removed.}
}
$*/
/*$
{function:
    {name:    rvSrtpRemoteSourceChangeKey}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function changes the master key that should be used to
			receive from the specified remote source. The new key will be used
			until changed, either by another call to this function or at a
			key change point specified by the rvSrtpRemoteSourceChangeKeyAt
			function.}
		{p: If shareTrigger is set to RV_TRUE, every source and
			destination in the session that shares the remote source's old
			master key will switch to this same new master key when
			this remote source switches to this new key.}
		{p: If mki is set to NULL, that indicates that the remote source
			has no key and any packets to be received from that
			source can not be decrypted.}
		{p: A remote source of the appropriate type (RTP or RTCP) for
			the specified ssrc must have been created using the
			rvSrtpRemoteSourceAddRtp or rvSrtpRemoteSourceAddRtcp function
			before keys can be set for that remote source.}
    }
    {proto: RvRtpStatus rvSrtpRemoteSourceChangeKey(RvSrtp *thisPtr, RvUint32 ssrc, RvBool sourceType, RvChar *mki, RvBool shareTrigger);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:ssrc} {d:The ssrc of the remote source.}}
        {param: {n:sourceType} {d:RV_SRTP_STREAMTYPE_SRTP or RV_SRTP_STREAMTYPE_SRTCP.}}
        {param: {n:mki} {d:Pointer to the MKI indentifier for the master key to use.}}
        {param: {n:shareTrigger} {d:RV_TRUE if anything sharing keys should switch keys when this source switches, RV_FALSE otherwise.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the remote source has been set to use the specified key.}
}
$*/
/*$
{function:
    {name:    rvSrtpRemoteSourceChangeKeyAt}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets a point in time (based on the index) at
			which the specified master key should be used to receive
			from the specified source. The new key will be used until
			changed, either by another change specified by a call to
			this function or by a call rvSrtpRemoteSourceChangeKey once
			the key is in use.}
		{p: If shareTrigger is set to RV_TRUE, every source and
			destination in the session that shares the remote source's old
			master key will switch to this same new master key when
			this remote source switches to this new key.}
		{p: Remember that the index is different for an SRTP
			destination as opposed to an SRTCP destination. Use the
			approprate range and values for the type of destination
			specified.}
		{p: IMPORTANT: This function is designed to set new keys for
			future use. However, since the current index value may be
			constantly changing, it is possible that an index value for
			a future key change may not be received until after the
			point of the switchover. Thus, a historic index value is
			allowed, however, because index values are allowed to wrap,
			the user must insure that the index value specified is
			never older than the maximum history length configured for
			the session, otherwise the "historic" key change will
			actually be set for a time in the distant future. That is
			why it is important to set the history length (using
			rvSrtpSetHistory) to a period of time long enough to
			safely allow for any delay in key exchanges.}
		{p: If mki is set to NULL, that indicates that the remote source
			has no key and any packets to be received from that source can
			not be decrypted.}
		{p: A remote source of the appropriate type (RTP or RTCP) for
			the specified ssrc must have been created using the
			rvSrtpRemoteSourceAddRtp or rvSrtpRemoteSourceAddRtcp function
			before keys can be set for that remote source.}
    }
    {proto: RvRtpStatus rvSrtpRemoteSourceChangeKeyAt(RvSrtp *thisPtr, RvUint32 ssrc, RvBool sourceType, RvChar *mki, RvUint64 index, RvBool shareTrigger);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:ssrc} {d:The ssrc of the remote source.}}
        {param: {n:sourceType} {d:RV_SRTP_STREAMTYPE_SRTP or RV_SRTP_STREAMTYPE_SRTCP.}}
        {param: {n:mki} {d:Pointer to the MKI indentifier for the master key to use.}}
        {param: {n:index} {d:The index at which the key should change.}}
        {param: {n:shareTrigger} {d:RV_TRUE if anything sharing keys should switch keys when this source switches, RV_FALSE otherwise.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the remote source has been set to use the specified key.}
}
$*/
/*$
{function:
    {name:    rvSrtpRemoteSourceClearAllKeys}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function removes all key mappings for a remote source.}
    }
    {proto: RvRtpStatus rvSrtpRemoteSourceClearAllKeys(RvSrtp *thisPtr, RvUint32 ssrc, RvBool sourceType);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:ssrc} {d:The ssrc of the remote source.}}
        {param: {n:sourceType} {d:RV_SRTP_STREAMTYPE_SRTP or RV_SRTP_STREAMTYPE_SRTCP.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpRemoteSourceGetIndex}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function gets the highest index value that has been
			decrypted for a message received from the specified
			remote source. Note that this value may be constantly
			changing and should only be used for status or for
			estimating future index values for key changes.}
		{p: Remember that the index is different for an SRTP
			source as opposed to an SRTCP source.}
    }
    {proto: RvRtpStatus rvSrtpRemoteSourceGetIndex(RvSrtp *thisPtr, RvUint32 ssrc, RvBool sourceType, RvUint64 *indexPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:ssrc} {d:The ssrc of the remote source.}}
        {param: {n:sourceType} {d:RV_SRTP_STREAMTYPE_SRTP or RV_SRTP_STREAMTYPE_SRTCP.}}
        {param: {n:indexPtr} {d:Pointer to location where index should be stored.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpForwardDestinationAddRtp}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function adds a new RTP destination for a forwarded source
			to the session.}
		{p: A destination needs to be created for each source, for each
			destination address that the session will be sending to. If
			SRTP and SRTCP are being used, a separate destination needs
			to be added for both (since, at a minumum, the port number
			will be different).}
        {p: This function is identical to the rvSrtpDestinationAdd
			function except it is specifically for translators that will be
			forwarding packets for multiple sources, thus allows the
			ssrc to be specified.}
    }
    {proto: RvRtpStatus rvSrtpForwardDestinationAddRtp(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvUint32 roc, RvUint32 sequenceNum);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:ssrc}    {d:The ssrc of the source being forwarded.}}
        {param: {n:address} {d:Pointer to the address string containing the destination IP address.}}
        {param: {n:port} {d:IP port number for the destination IP address.}}
        {param: {n:roc} {d:The initial rollover counter value that will be forwarded.}}
        {param: {n:sequenceNum} {d:The initial sequence number value that will be forwarded.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the RTP destination has been added.}
}
$*/
/*$
{function:
    {name:    rvSrtpForwardDestinationAddRtcp}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function adds a new RTCP destination for a forwarded source
			to the session.}
		{p: A destination needs to be created for each source, for each
			destination address that the session will be sending to. If
			SRTP and SRTCP are being used, a separate destination needs
			to be added for both (since, at a minumum, the port number
			will be different).}
        {p: This function is identical to the rvSrtpDestinationAdd
			function except it is specifically for translators that will be
			forwarding packets for multiple sources, thus allows the
			ssrc to be specified.}
    }
    {proto: RvRtpStatus rvSrtpForwardDestinationAddRtcp(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvUint32 index);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:ssrc}    {d:The ssrc of the source being forwarded.}}
        {param: {n:address} {d:Pointer to the address string containing the destination IP address.}}
        {param: {n:port} {d:IP port number for the destination IP address.}}
        {param: {n:index} {d:The initial index value that will be forwarded.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the RTCP destination has been added.}
}
$*/
/*$
{function:
    {name:    rvSrtpForwardDestinationRemove}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function removes a destination for a forwarded
			source from the session.}
        {p: This function is identical to the rvSrtpDestinationRemove
			function except it is specifically for translators that will be
			forwarding packets for multiple sources, thus allows the
			ssrc to be specified.}
    }
    {proto: RvRtpStatus rvSrtpForwardDestinationRemove(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:ssrc}    {d:The ssrc of the source being forwarded.}}
        {param: {n:address} {d:Pointer to the address string containing the destination IP address.}}
        {param: {n:port} {d:IP port number for the destination IP address.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the destination has been removed.}
}
$*/
/*$
{function:
    {name:    rvSrtpForwardDestinationChangeKey}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function changes the master key that should be used to
			forward to the specified destination. The new key will be used
			until changed, either by another call to this function or at a
			key change point specified by the rvSrtpForwardDestinationChangeKeyAt
			function.}
		{p: If shareTrigger is set to RV_TRUE, every source and
			destination in the session that shares the destinations old
			master key will switch to this same new master key when
			this destination switches to this new key.}
		{p: If mki is set to NULL, that indicates that the destination
			has no key and any packets to be sent to that destination can
			not be sent.}
		{p: A destination for the specified address must have been
			created using the rvSrtpForwardDestinationAddRtp or
			rvSrtpForwardDestinationAddRtcp functions before keys can
			be set for that destination.}
        {p: This function is identical to the rvSrtpDestinationChangeKey
			function except it is specifically for translators that will be
			forwarding packets for multiple sources, thus allows the
			ssrc to be specified.}
    }
    {proto: RvRtpStatus rvSrtpForwardDestinationChangeKey(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvChar *mki, RvBool shareTrigger);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:ssrc}    {d:The ssrc of the source being forwarded.}}
        {param: {n:address} {d:Pointer to the address string containing the destination IP address.}}
        {param: {n:port} {d:IP port number for the destination IP address.}}
        {param: {n:mki} {d:Pointer to the MKI indentifier for the master key to use.}}
        {param: {n:shareTrigger} {d:RV_TRUE if anything sharing keys should switch keys when this destination switches, RV_FALSE otherwise.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the destination has been set to use the specified key.}
}
$*/
/*$
{function:
    {name:    rvSrtpForwardDestinationChangeKeyAt}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets a point in time (based on the index) at
			which the specified master key should be used to forward to
			the specified destination. The new key will be used until
			changed, either by another change specified by a call to
			this function or by a call rvSrtpForwardDestinationChangeKey once
			the key is in use.}
		{p: If shareTrigger is set to RV_TRUE, every source and
			destination in the session that shares the destinations old
			master key will switch to this same new master key when
			this destination switches to this new key.}
		{p: Remember that the index is different for an SRTP
			destination as opposed to an SRTCP destination. Use the
			approprate range and values for the type of destination
			specified.}
		{p: IMPORTANT: This function is designed to set new keys for
			future use. However, since the current index value may be
			constantly changing, it is possible that an index value for
			a future key change may not be received until after the
			point of the switchover. Thus, a historic index value is
			allowed, however, because index values are allowed to wrap,
			the user must insure that the index value specified is
			never older than the maximum history length configured for
			the session, otherwise the "historic" key change will
			actually be set for a time in the distant future. That is
			why it is important to set the history length (using
			rvSrtpSetHistory) to a period of time long enough to
			safely allow for any delay in key exchanges.}
		{p: If mki is set to NULL, that indicates that the destination
			has no key and any packets to be sent to that destination can
			not be sent.}
		{p: A destination for the specified address must have been
			created using the rvSrtpForwardDestinationAddRtp or
			rvSrtpForwardDestinationAddRtcp function before keys can
			be set for that destination.}
        {p: This function is identical to the rvSrtpDestinationChangeKeyAt
			function except it is specifically for translators that will be
			forwarding packets for multiple sources, thus allows the
			ssrc to be specified.}
    }
    {proto: RvRtpStatus rvSrtpForwardDestinationChangeKeyAt(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvChar *mki, RvUint64 index, RvBool shareTrigger);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:ssrc}    {d:The ssrc of the source being forwarded.}}
        {param: {n:address} {d:Pointer to the address string containing the destination IP address.}}
        {param: {n:port} {d:IP port number for the destination IP address.}}
        {param: {n:mki} {d:Pointer to the MKI indentifier for the master key to use.}}
        {param: {n:index} {d:The index at which the key should change.}}
        {param: {n:shareTrigger} {d:RV_TRUE if anything sharing keys should switch keys when this destination switches, RV_FALSE otherwise.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the destination has been set to use the specified key.}
}
$*/
/*$
{function:
    {name:    rvSrtpForwardDestinationClearAllKeys}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function removes all key mappings for a forwarded destination.}
        {p: This function is identical to the rvSrtpDestinationClearAllKeys
			function except it is specifically for translators that will be
			forwarding packets for multiple sources, thus allows the
			ssrc to be specified.}
    }
    {proto: RvRtpStatus rvSrtpForwardDestinationClearAllKeys(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:ssrc}    {d:The ssrc of the source being forwarded.}}
        {param: {n:address} {d:Pointer to the address string containing the destination IP address.}}
        {param: {n:port} {d:IP port number for the destination IP address.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpForwardDestinationGetIndex}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function gets the highest index value that has been
			encrypted for a message to be forwarded to the specified
			destination. Note that this value may be constantly
			changing and should only be used for status or for
			estimating future index values for key changes.}
		{p: Remember that the index is different for an SRTP
			destination as opposed to an SRTCP destination.}
        {p: This function is identical to the rvSrtpDestinationgetIndex
			function except it is specifically for translators that will be
			forwarding packets for multiple sources, thus allows the
			ssrc to be specified.}
    }
    {proto: RvRtpStatus rvSrtpForwardDestinationGetIndex(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvUint64 *indexPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:ssrc}    {d:The ssrc of the source being forwarded.}}
        {param: {n:address} {d:Pointer to the address string containing the destination IP address.}}
        {param: {n:port} {d:IP port number for the destination IP address.}}
        {param: {n:indexPtr} {d:Pointer to location where index should be stored.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpForwardDestinationSsrcChanged}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function changes the SSRC of a forwarded source
			in the session. This function provides an easy (but
			exactly equivalent) way to deal with the SSRC of a remote
			source changing instead of having to remove all the old
			destinations and add new ones with the new SSRC.}
		{p: Any key settings for the old SSRC will be cleared and the
			new SSRC will have no keys set (now or in the future).}
		{p: The old SSRC is only allowed to be an SSRC of a
			forwarded source which had destinations added with the
			rvSrtpForwardDestinationAddRtp or
			rvSrtpForwardDestinationAddRtcp functions.}
    }
    {proto: RvRtpStatus rvSrtpForwardDestinationSsrcChanged(RvSrtp *thisPtr, RvUint32 oldSsrc, RvUint32 newSsrc);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:oldSsrc} {d:The old ssrc of the source being forwarded.}}
        {param: {n:newSsrc} {d:The new ssrc of the source being forwarded.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpForwardRtp}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: Forwards a raw RTP packet from a session using SRTP
			encryption. The packet will be created only using the
			information specified in the header information and will
			use the rollover counter (roc) value provided.}
        {p: This function is used by translators to relay a packet received
			in one cloud to other clouds and should be used instead of
			the standard rvRtpSessionForwardRtp function when using SRTP.}
        {p: Remember that the buffer requires the appropriate amount of
			padding before and after the actual RTP data. The amount of
			padding can be determined using the rvRtpSessionGetRtpHeaderSize
			and rvRtpSessionGetRtpFooterSize functions.}
    }
    {proto: RvRtpStatus rvSrtpForwardRtp(RvRtpSession *thisPtr, RvChar *bufStartPtr, RvUint32 bufSize, RvChar *dataStartPtr, RvUint32 dataSize, RvRtpSessionRtpHeader *headerPtr, RvUint32 roc);}
    {params:
        {param: {n:thisPtr}      {d:The RvRtpSession to send from.}}
        {param: {n:bufStartPtr}  {d:The buffer conataining the RTP data with padding.}}
        {param: {n:bufSize}      {d:The size of the buffer.}}
        {param: {n:dataStartPtr} {d:The start of the actual RTP data within the buffer.}}
        {param: {n:dataSize}     {d:The size of RTP data.}}
        {param: {n:headerPtr}    {d:The header information to create the packet with.}}
        {param: {n:roc}          {d:The rollover counter value to use.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the packet was sent successfully. }
}
$*/
/*$
{function:
    {name:    rvSrtpForwardRtcp}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: Forwards a raw RTCP packet from a session using SRTCP
			encyrption. The packet must be fully formed unencrypted
			RTCP packet and ready to be sent. The index value provided
			will be used for the SRTCP index value.}
        {p: This function is used by translators to relay a packet received
			in one cloud to other clouds and should be used instead of
			the standard rvRtpSessionForwardRtcp function when using SRTCP.}
        {p: Remember that the buffer requires the appropriate amount of
			padding before and after the actual RTCP packet. The amount of
			padding can be determined using the rvRtpSessionGetRtcpHeaderSize
			and rvRtpSessionGetRtcpFooterSize functions.}
    }
    {proto: RvRtpStatus rvSrtpForwardRtcp(RvRtpSession *thisPtr, RvChar *bufStartPtr, RvUint32 bufSize, RvChar *dataStartPtr, RvUint32 dataSize, RvUint32 index);}
    {params:
        {param: {n:thisPtr}      {d:The RvRtpSession to send from.}}
        {param: {n:bufStartPtr}  {d:The buffer conatining the RTCP packet with padding.}}
        {param: {n:bufSize}      {d:The size of the buffer.}}
        {param: {n:dataStartPtr} {d:The start of the actual RTCP packet within the buffer.}}
        {param: {n:dataSize}     {d:The size of RTCP packet.}}
        {param: {n:index}        {d:The SRTCP index to use.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if the packet was sent successfully. }
}
$*/
/*$
{function:
    {name:    rvSrtpSetMasterKeySizes}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the key sizes to be used for master
			keys, master salts, and the mki indentifier.}
		{p: The master key and MKI size must have values greater than 0.
			Even if MKI values are not to be included in the packet, a
			size is requred because the MKI value is used to identify master keys.}
		{p: The master key will default to 16 bytes (128 bits), the
			master salt will default to 14 bytes (112 bits), and the MKI
			will default to 4 bytes, if not specifically set.}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetMasterKeySizes(RvSrtp *thisPtr, RvSize_t mkiSize, RvSize_t keySize, RvSize_t saltSize);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:mkiSize} {d:The size of the MKI.}}
        {param: {n:keySize} {d:The size of the master key.}}
        {param: {n:saltSize} {d:The size of the master salt.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetKeyDerivation}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the algorithm which should be used to
			gereate new session keys and the rate at which new keys
			should be generated (0 meaning that keys should only be
			generated once). The only algorithm currently
			supported is AES-CM (RV_SRTP_KEYDERIVATIONALG_AESCM).}
		{p: The algorithm defaults to AES-CM and the derivation rate
			defaults to 0, if not specifically set.}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetKeyDerivation(RvSrtp *thisPtr, RvUint32 keyDerivationAlg, RvUint32 keyDerivationRate);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:keyDerivationAlt} {d:The key derivation algorithm.}}
        {param: {n:keyDerivationRate} {d:The rate at which to regenerate session keys.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetPrefixLength}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets keystream prefix length (as per the
			RFC). This is normally set to 0 for standard SRTP/SRTCP and
			that is what the value defaults to if not specifically set.
			Currently, only a value of 0 is supported.}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetPrefixLength(RvSrtp *thisPtr, RvSize_t prefixLength);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:perfixLength} {d:The length of the keystream prefix.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetRtpEncryption}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the type of encrytion to use for RTP
			along with the encyrption block size and whether or not the MKI
			should be included in the packets. Currently supported
			encryption types are
			AES-CM (RV_SRTP_ENCYRPTIONALG_AESCM),
			AES-F8 (RV_SRTP_ENCYRPTIONALG_AESF8),
			no encyrption (RV_SRTP_ENCYRPTIONALG_NULL).}
		{p: The algorithm defaults to AES-CM and with MKI enabled, if
			they are not specifically set.}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetRtpEncryption(RvSrtp *thisPtr, RvInt encryptType, RvBool useMki);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:encrptType} {d:Type of encryption to use for RTP.}}
        {param: {n:useMki}  {d:RV_TRUE to use MKI in RTP packet, RV_FALSE to not include it.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetRtcpEncryption}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the type of encrytion to use for RTCP
			along with the encyrption block size and whether or not the MKI
			should be included in the packets. Currently supported
			encryption types are
			AES-CM (RV_SRTP_ENCYRPTIONALG_AESCM),
			AES-F8 (RV_SRTP_ENCYRPTIONALG_AESF8),
			no encyrption (RV_SRTP_ENCYRPTIONALG_NULL).}
		{p: The algorithm defaults to AES-CM and with MKI enabled, if
			they are not specifically set.}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetRtcpEncryption(RvSrtp *thisPtr, RvInt encryptType, RvBool useMki);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:encrptType} {d:Type of encryption to use for RTCP.}}
        {param: {n:useMki}  {d:RV_TRUE to use MKI in RTP packet, RV_FALSE to not include it.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetRtpAuthentication}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the type of authentication to use for RTP
			along with the size of the authentication tag that
			will be included in each packet. Currently supported
			authentication types are
			HMAC-SHA1 (RV_SRTP_AUTHENTICATIONALG_HMACSHA1),
			no authentication (RV_SRTP_AUTHENTICATIONALG_NONE).}
		{p: If the authentication tags size is 0 then an authentication
			algorithm of RV_SRTP_AUTHENTICATIONALG_NONE is assumed.}
		{p: The algorithm defaults to HMAC-SHA1 with a tag size of 10
			bytes (80 bits), if they are not specifically set.}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetRtpAuthentication(RvSrtp *thisPtr, RvInt authType, RvSize_t tagSize);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:authType} {d:Type of authentication to use for RTP.}}
        {param: {n:tagSize} {d:The size of the authentication tag to use for RTP.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetRtcpAuthentication}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the type of authentication to use for RTCP
			along with the size of the authentication tag that
			will be included in each packet. Currently supported
			authentication types are
			HMAC-SHA1 (RV_SRTP_AUTHENTICATIONALG_HMACSHA1),
			no authentication (RV_SRTP_AUTHENTICATIONALG_NONE).}
		{p: If the authentication tags size is 0 then an authentication
			algorithm of RV_SRTP_AUTHENTICATIONALG_NONE is assumed.}
		{p: The algorithm defaults to HMAC-SHA1 with a tag size of 10
			bytes (80 bits), if they are not specifically set.}
		{p: Note that the SRTP/SRTCP RFC requires that authentication
			be used with RTCP.}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetRtcpAuthentication(RvSrtp *thisPtr, RvInt authType, RvSize_t tagSize);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:authType} {d:Type of authentication to use for RTCP.}}
        {param: {n:tagSize} {d:The size of the authentication tag to use for RTCP.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetRtpKeySizes}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the RTP session key sizes to be used for
			the encyrption keys, authentication keys, and salts.}
		{p: The session key size must have a value greater than 0.
			If authentication is enabled, then the authentication key size
			must also be greater than 0.}
		{p: The session key will default to 16 bytes (128 bits), the
			session salt will default to 14 bytes (112 bits), and the
			authentication key will default to 20 bytes (160 bits), if
			not specifically set.}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetRtpKeySizes(RvSrtp *thisPtr, RvSize_t encryptKeySize, RvSize_t authKeySize, RvSize_t saltSize);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:encryptKeySize} {d:The size of the RTP encryption key.}}
        {param: {n:authKeySize} {d:The size of the RTP authentication key.}}
        {param: {n:saltSize} {d:The size of the RTP salt.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetRtcpKeySizes}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the RTCP session key sizes to be used for
			the encyrption keys, authentication keys, and salts.}
		{p: The session key size must have a value greater than 0.
			If authentication is enabled, then the authentication key size
			must also be greater than 0.}
		{p: The session key will default to 16 bytes (128 bits), the
			session salt will default to 14 bytes (112 bits), and the
			authentication key will default to 20 bytes (160 bits), if
			not specifically set.}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetRtcpKeySizes(RvSrtp *thisPtr, RvSize_t encryptKeySize, RvSize_t authKeySize, RvSize_t saltSize);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:encryptKeySize} {d:The size of the RTCP encryption key.}}
        {param: {n:authKeySize} {d:The size of the RTCP authentication key.}}
        {param: {n:saltSize} {d:The size of the RTCP salt.}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetRtpReplayListSize}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the size of the replay list to be used
			for insuring that RTP packets received from remote sources are not
			replicated (intentionally or accidentally). Refer to the
			SRTP/SRTCP RFC for further information.}
		{p: Setting the size to 0 disables the use of replay lists.}
		{p: The minumum (and default) size of the replay list is 64 (as per the RFC).}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetRtpReplayListSize(RvSrtp *thisPtr, RvUint64 replayListSize);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:replayListSize} {d:The size of the RTP replay lists (in packets).}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetRtcpReplayListSize}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the size of the replay list to be used
			for insuring that RTCP packets received from remote sources are not
			replicated (intentionally or accidentally). Refer to the
			SRTP/SRTCP RFC for further information.}
		{p: Setting the size to 0 disables the use of replay lists.}
		{p: The minumum (and default) size of the replay list is 64 (as per the RFC).}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetRtcpReplayListSize(RvSrtp *thisPtr, RvUint64 replayListSize);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:replayListSize} {d:The size of the RTCP replay lists (in packets).}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetRtpHistory}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the size (according to index count) of
			the history list that should be kept for each RTP source and
			destination. Since the indexes are unsigned values that can
			wrap, this creates the "window" of indexes that are considered
			to be old indexes in the history, with the rest being future
			index values not yet received.}
		{p: This value defaults to 65536. Refer to the rvSrtpDestinationChangeKeyAt,
			rvSrtpRemoteSourceChangeKeyAt, or rvSrtpForwardDestinationChangeKeyAt
			functions for further information on the effect of this setting.}
		{p: If the value is lower than the replay list size (set with
			rvSrtpSetRtpReplayListSize), then the value will
			be set to the same as the replay list size.}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetRtpHistory(RvSrtp *thisPtr, RvSize_t historySize);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:historySize} {d:The SRTP key map history size (in packets).}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetRtcpHistory}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the size (according to index count) of
			the history list that should be kept for each RTCP source and
			destination. Since the indexes are unsigned values that can
			wrap, this creates the "window" of indexes that are considered
			to be old indexes in the history, with the rest being future
			index values not yet received.}
		{p: This value defaults to 65536. Refer to the rvSrtpDestinationChangeKeyAt,
			rvSrtpRemoteSourceChangeKeyAt, or rvSrtpForwardDestinationChangeKeyAt
			functions for further information on the effect of this setting.}
		{p: If the value is lower than the replay list size (set with
			rvSrtpSetRtcpReplayListSize), then the value will
			be set to the same as the replay list size.}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetRtcpHistory(RvSrtp *thisPtr, RvSize_t historySize);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:historySize} {d:The SRTCP key map history size (in packets).}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetMasterKeyPool}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the behavior of the master key pool. A
			master key object is required for every master key that
			SRTP/SRTCP will maintain.}
		{p: Fixed pools are just that, fixed, and will only allow the
			specified number of objects to exist. Expanding pools will
			expand as necessary, but will never shrink (until the
			plugin is unregistered). Dynamic pools will, in addition to
			expanding, shrink based on the freeLevel parameter.}
		{p: By default, this pool will be configured as an expanding
			pool with 10 master key objects per memory page and the memory
			being allocated from the default region.}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetMasterKeyPool(RvSrtp *thisPtr, RvInt poolType, RvSize_t pageItems, RvSize_t pageSize, RvSize_t maxItems, RvSize_t minItems, RvSize_t freeLevel, RvMemory *region);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:poolType} {d:The type of pool: RV_SRTP_POOLTYPE_FIXED, RV_SRTP_POOLTYPE_EXPANDING, or RV_SRTP_POOLTYPE_DYNAMIC.}}
        {param: {n:pageItems} {d:The number of objects per page (0 = calculate from pageSize).}}
        {param: {n:pageSize} {d:The size of a memory page (0 = calculate from pageItems).}}
        {param: {n:maxItems} {d:The number of objects will never exceed this value (0 = no max).}}
        {param: {n:minItems} {d:The number of objects will never go below this value.}}
        {param: {n:freeLevel} {d:The minimum number of objects per 100 (0 to 100) - DYNAMIC pools only.}}
        {param: {n:region} {d:The memory region to allocate from (NULL = use default).}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetStreamPool}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the behavior of the stream pool. A
			stream object is required for every remote source and every
			destination that SRTP/SRTCP will maintain.}
		{p: Fixed pools are just that, fixed, and will only allow the
			specified number of objects to exist. Expanding pools will
			expand as necessary, but will never shrink (until the
			plugin is unregistered). Dynamic pools will, in addition to
			expanding, shrink based on the freeLevel parameter.}
		{p: By default, this pool will be configured as an expanding
			pool with 20 stream objects per memory page and the memory
			being allocated from the default region.}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetStreamPool(RvSrtp *thisPtr, RvInt poolType, RvSize_t pageItems, RvSize_t pageSize, RvSize_t maxItems, RvSize_t minItems, RvSize_t freeLevel, RvMemory *region);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:poolType} {d:The type of pool: RV_SRTP_POOLTYPE_FIXED, RV_SRTP_POOLTYPE_EXPANDING, or RV_SRTP_POOLTYPE_DYNAMIC.}}
        {param: {n:pageItems} {d:The number of objects per page (0 = calculate from pageSize).}}
        {param: {n:pageSize} {d:The size of a memory page (0 = calculate from pageItems).}}
        {param: {n:maxItems} {d:The number of objects will never exceed this value (0 = no max).}}
        {param: {n:minItems} {d:The number of objects will never go below this value.}}
        {param: {n:freeLevel} {d:The minimum number of objects per 100 (0 to 100) - DYNAMIC pools only.}}
        {param: {n:region} {d:The memory region to allocate from (NULL = use default).}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetContextPool}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the behavior of the Context pool. A
			context object is required for every combination of master
			key and stream that will be used and maintained in the
			history.}
		{p: Fixed pools are just that, fixed, and will only allow the
			specified number of objects to exist. Expanding pools will
			expand as necessary, but will never shrink (until the
			plugin is unregistered). Dynamic pools will, in addition to
			expanding, shrink based on the freeLevel parameter.}
		{p: By default, this pool will be configured as an expanding
			pool with 40 context objects per memory page and the memory
			being allocated from the default region.}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetContextPool(RvSrtp *thisPtr, RvInt poolType, RvSize_t pageItems, RvSize_t pageSize, RvSize_t maxItems, RvSize_t minItems, RvSize_t freeLevel, RvMemory *region);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:poolType} {d:The type of pool: RV_SRTP_POOLTYPE_FIXED, RV_SRTP_POOLTYPE_EXPANDING, or RV_SRTP_POOLTYPE_DYNAMIC.}}
        {param: {n:pageItems} {d:The number of objects per page (0 = calculate from pageSize).}}
        {param: {n:pageSize} {d:The size of a memory page (0 = calculate from pageItems).}}
        {param: {n:maxItems} {d:The number of objects will never exceed this value (0 = no max).}}
        {param: {n:minItems} {d:The number of objects will never go below this value.}}
        {param: {n:freeLevel} {d:The minimum number of objects per 100 (0 to 100) - DYNAMIC pools only.}}
        {param: {n:region} {d:The memory region to allocate from (NULL = use default).}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetMasterKeyHash}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the behavior of the master key hash. A
			master key object is required for every master key that
			SRTP/SRTCP will maintain.}
		{p: Fixed hashes are just that, fixed, and will only allow the
			specified number of objects to be put into the hash table.
			Expanding hashes will expand as necessary, but will never
			shrink (until the plugin is unregistered). Dynamic hashes
			will, in addition to expanding, shrink to maintain optimum
			size, but will never shrink below the startSize parameter.}
		{p: By default, this hash will be configured as an expanding
			hash of size 17 with the memory	being allocated from the
			default region.}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetMasterKeyHash(RvSrtp *thisPtr, RvInt hashType, RvSize_t startSize, RvMemory *region);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:hashType} {d:The type of pool: RV_SRTP_HASHTYPE_FIXED, RV_SRTP_HASHTYPE_EXPANDING, or RV_SRTP_HASHTYPE_DYNAMIC.}}
        {param: {n:startSize} {d:The starting number of buckets in the hash table (and minumum size).}}
        {param: {n:region} {d:The memory region to allocate from (NULL = use default).}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetSourceHash}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the behavior of the source hash. A
			source exists for every remote source that
			SRTP/SRTCP will maintain.}
		{p: Fixed hashes are just that, fixed, and will only allow the
			specified number of objects to be put into the hash table.
			Expanding hashes will expand as necessary, but will never
			shrink (until the plugin is unregistered). Dynamic hashes
			will, in addition to expanding, shrink to maintain optimum
			size, but will never shrink below the startSize parameter.}
		{p: By default, this hash will be configured as an expanding
			hash of size 17 with the memory	being allocated from the
			default region.}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetSourceHash(RvSrtp *thisPtr, RvInt hashType, RvSize_t startSize, RvMemory *region);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:hashType} {d:The type of pool: RV_SRTP_HASHTYPE_FIXED, RV_SRTP_HASHTYPE_EXPANDING, or RV_SRTP_HASHTYPE_DYNAMIC.}}
        {param: {n:startSize} {d:The starting number of buckets in the hash table (and minumum size).}}
        {param: {n:region} {d:The memory region to allocate from (NULL = use default).}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
/*$
{function:
    {name:    rvSrtpSetDestHash}
    {class:   RvSrtp}
    {include: rvsrtp.h}
    {description:
        {p: This function sets the behavior of the destination hash. A
			destination exists for every destination that
			SRTP/SRTCP will maintain.}
		{p: Fixed hashes are just that, fixed, and will only allow the
			specified number of objects to be put into the hash table.
			Expanding hashes will expand as necessary, but will never
			shrink (until the plugin is unregistered). Dynamic hashes
			will, in addition to expanding, shrink to maintain optimum
			size, but will never shrink below the startSize parameter.}
		{p: By default, this hash will be configured as an expanding
			hash of size 17 with the memory	being allocated from the
			default region.}
		{p: This function may only be called before the plugin is registered.}
    }
    {proto: RvRtpStatus rvSrtpSetDestHash(RvSrtp *thisPtr, RvInt hashType, RvSize_t startSize, RvMemory *region);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtp plugin object.}}
        {param: {n:hashType} {d:The type of pool: RV_SRTP_HASHTYPE_FIXED, RV_SRTP_HASHTYPE_EXPANDING, or RV_SRTP_HASHTYPE_DYNAMIC.}}
        {param: {n:startSize} {d:The starting number of buckets in the hash table (and minumum size).}}
        {param: {n:region} {d:The memory region to allocate from (NULL = use default).}}
    }
    {returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/

#ifdef __cplusplus
}
#endif

#endif
