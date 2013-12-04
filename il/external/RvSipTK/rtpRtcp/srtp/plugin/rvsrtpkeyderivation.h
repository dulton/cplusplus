/***********************************************************************
Filename   : rvsrtpkeyderivation.h
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
#ifndef RV_SRTP_KEY_DERIVATION_H
#define RV_SRTP_KEY_DERIVATION_H

#include "rvtypes.h"
#include "rvrtpstatus.h"
#include "rvsrtpaesplugin.h"
#include "rvsrtpdb.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*$
{type scope="private":
	{name: RvSrtpKeyDerivation}
	{superpackage: RvSrtpEncryptionPlugin}
	{include: rvsrtpkeyderivation.h}
	{description:
		{p: This class represents a key derivation configuration.}
	}
}
$*/
typedef struct {
	RvSrtpDb *dbPtr;
	RvInt    keyDerivationAlg;  /* Only AES-CM currently supported */
} RvSrtpKeyDerivation;

/* Derivation algorithms */
#define RV_SRTP_DERIVATION_AESCM RvIntConst(1)

RvSrtpKeyDerivation *rvSrtpKeyDerivationConstruct(RvSrtpKeyDerivation *thisPtr, RvSrtpDb *dbPtr, RvInt keyDerivationAlg);
#define  rvSrtpKeyDerivationDestruct(thisPtr)
RvRtpStatus rvSrtpKeyDerivationUpdateContext(RvSrtpKeyDerivation *thisPtr, RvSrtpAesPlugIn* plugin,  RvSrtpEncryptionPurpose purpose, RvSrtpContext *contextPtr, RvUint64 index, RvBool *currentKeys);

#if defined(RV_TEST_CODE)
RvRtpStatus RvSrtpKeyDerivationTest(RvSrtpDb *dbPtr, RvSrtpContext *contextPtr);
#endif /* RV_TEST_CODE */

/* Function Docs */
/*$
{function scope="private":
	{name: rvSrtpKeyDerivationConstruct}
	{class: rvSrtpKeyDerivation}
	{include: rvsrtpkeyderivation.h}
	{description:
        {p: Constructs a key derivation object.}
        {p: The only key derivation currently supported is AES-CM (RV_SRTP_DERIVATION_AESCM).}
	}
	{proto: RvSrtpKeyDerivation *rvSrtpKeyDerivationConstruct(RvSrtpKeyDerivation *thisPtr, RvSrtpDb *dbPtr, RvInt keyDerivationAlg);}
	{params:
		{param: {t: RvSrtpKeyDerivation *} {n: thisPtr} {d: The key derivation object.}}
		{param: {t: RvSrtpDb *} {n: dbPtr} {d: The database to get information from.}}
		{param: {t: RvInt} {n: keyDerivationAlg} {d: Key derivation algorithm.}}
	}
	{returns: A pointer to the key derivation object or NULL if there was an error.}
}
$*/
/*$
{function scope="private":
	{name: rvSrtpSsrcValidationDestruct}
	{class: rvSrtpKeyDerivation}
	{include: rvsrtpkeyderivation.h}
	{description:
		{p: Destructs a key derivation object.}
	}
	{proto: void rvSrtpKeyDerivationDestruct(RvSrtpKeyDerivation *thisPtr);}
	{params:
		{param: {t: RvSrtpKeyDerivation *} {n: thisPtr} {d: The key derivation object.}}
	}
}
$*/
/*$
{function scope="private":
	{name: rvSrtpKeyDerivationUpdateContext}
	{class: rvSrtpKeyDerivation}
	{include: rvsrtpkeyderivation.h}
	{description:
        {p: This function updates a context, generating session keys if
			necessary. If the update is successful, a value will be
			stored in the currentKeys parameter indicating if the the
			specified index should use the current session keys
			(RV_TRUE) or the old session keys (RV_FALSE).}
		{p: The following actions will be done:}
		{bulletlist:
			{item: {bold:Key Check:} Will check the master key being
				used by the context to insure that it has not hit its usage
				limit. A master kit that has hit the limit no longer can be
				used.}
			{item: {bold:Key Check:} Update the session key buffers and
				generate session session keys, if necessary. Keys may
				also need to be regenerated based on the key derivation
				rate.}
			{item: {bold:Key Trigger:} If this context had never before
				contained session keys, then it indicates that a master key
				switch has taken place. If a master key switch has taken
				place, and the context is flagged as being a trigger, then
				every other context which uses the same master key as the
				previous context will have to be checked to see if a key
				switch is required. For each of these contexts, check to
				see of the context is current. If so, a new context should
				be created that uses the same master keys as the triggering context.}
		}
	}
	{proto: RvRtpStatus rvSrtpKeyDerivationUpdateContext(RvSrtpKeyDerivation *thisPtr, RvSrtpContext *contextPtr, RvUint64 index, RvBool *currentKeys);}
	{params:
		{param: {t: RvSrtpKeyDerivation *} {n: thisPtr} {d: The database to get information from.}}
		{param: {t: RvSrtpContext *} {n: contextPtr} {d: The context to update.}}
		{param: {t: RvUint64}      {n: index} {d: The current SRTP or SRTCP index.}}
		{param: {t: RvBool *}      {n: currentKeys} {d: result: RV_TRUE = index uses current keys, RV_FALSE = old keys.}}
	}
{returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/

#if defined(__cplusplus)
}
#endif

#endif
