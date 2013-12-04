/***********************************************************************
Filename   : rvsrtpssrcvalidation.h
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
#ifndef RV_SRTP_SSRC_VALIDATION_H
#define RV_SRTP_SSRC_VALIDATION_H

#if defined(__cplusplus)
extern "C" {
#endif 

#include "rvrtpstatus.h"
#include "rvsrtpdb.h"


/*$
{type scope="private":
	{name: rvSrtpSsrcValidation}
	{superpackage: RvSrtpEncryptionPlugin}
	{include: rvsrtpssrcvalidation.h}
	{description:
		{p: This class represents an ssrc validation configuration.}
	}
}
$*/
typedef struct {
	RvSrtpDb *dbPtr;
	RvBool rtpCheck;
	RvBool rtcpCheck;
} RvSrtpSsrcValidation;

RvSrtpSsrcValidation *rvSrtpSsrcValidationConstruct(RvSrtpSsrcValidation *thisPtr, RvSrtpDb *dbPtr, RvBool rtpCheck, RvBool rtcpCheck);
void rvSrtpSsrcValidationDestruct(RvSrtpSsrcValidation *thisPtr);
RvRtpStatus rvSrtpSsrcValidationCheck(RvSrtpSsrcValidation *thisPtr, RvUint32 ssrc, RvUint64 rtpIndex, RvUint64 rtcpIndex);
#define rvSrtpSsrcValidationSetRtpCheck(thisPtr, value) ((thisPtr)->rtpCheck = (value))
#define rvSrtpSsrcValidationSetRtcpCheck(thisPtr, value) ((thisPtr)->rtpCheck = (value))

#if defined(RV_TEST_CODE)
RvRtpStatus RvSrtpSsrcValidationTest();
#endif /* RV_TEST_CODE */

/* Function Docs */
/*$
{function scope="private":
	{name: rvSrtpSsrcValidationConstruct}
	{class: RvSrtpSsrcValidation}
	{include: rvsrtpssrcvalidation.h}
	{description:
        {p: Constructs an ssrc validation object.}
	}
    {proto: RvSrtpSsrcValidation *rvSrtpSsrcValidationConstruct(RvSrtpSsrcValidation *thisPtr, RvSrtpDb *dbPtr, RvBool rtpCheck, RvBool rtcpCheck);}
	{params:
		{param: {t: RvSrtpSsrcValidation *} {n: thisPtr} {d: The ssrc validation object.}}
		{param: {t: RvSrtpDb *} {n: dbPtr} {d: The database to get information from.}}
		{param: {t: RvBool} {n: rtpCheck} {d: RV_TRUE if SRTP initialization vectors should be checked.}}
		{param: {t: RvBool} {n: rtcpCheck} {d: RV_TRUE if SRTCP initialization vectors should be checked.}}
	}
	{returns: A pointer to the ssrc validation object or NULL if there was an error.}
}
$*/
/*$
{function scope="private":
	{name: rvSrtpSsrcValidationDestruct}
	{class: RvSrtpSsrcValidation}
	{include: rvsrtpssrcvalidation.h}
	{description:
        {p: Destructs an ssrc validation object.}
	}
	{proto: void rvSrtpSsrcValidationDestruct(RvSrtpSsrcValidation *thisPtr);}
	{params:
		{param: {t: RvSrtpSsrcValidation *} {n: thisPtr} {d: The ssrc validation object.}}
	}
}
$*/
/*$
{function scope="private":
	{name: rvSrtpSsrcValidationCheck}
	{class: RvSrtpSsrcValidation}
	{include: rvsrtpssrcvalidation.h}
	{description:
        {p: This function validates an ssrc in insure that it would not
			cause initialization vectors, to be indentical to others
			already in the session.}
		{p: Note that while the requirement is that no two
			initialization vectors be indentical for the same key, since we
			don't know every key we may ever use, we must insure that other
			elements that make up the initialization vectors are unique for
			the entire session.}
	}
	{proto: RvRtpStatus rvSrtpSsrcValidationCheck(RvSrtpSsrcValidation *thisPtr, RvUint32 ssrc, RvUint64 rtpIndex, RvUint64 rtcpIndex);}
	{params:
		{param: {t: RvSrtpSsrcValidation *} {n: thisPtr} {d: The ssrc validation object.}}
		{param: {t: RvUint32} {n: ssrc} {d: The ssrc to validate.}}
		{param: {t: RvUint64} {n: rtpIndex} {d: The current rtp index for the ssrc.}}
		{param: {t: RvUint64} {n: rtcpIndex} {d: The current rtcp index for the ssrc.}}
	}
	{returns: RV_OK if the ssrc is OK to use, otherwise returns an error.}
}
$*/
/*$
{function scope="private":
	{name: rvSrtpSsrcValidationSetRtpCheck}
	{class: RvSrtpSsrcValidation}
	{include: rvsrtpssrcvalidation.h}
	{description:
        {p: Enables/disables the validation of SSRCs for RTP.}
	}
    {proto: void rvSrtpSsrcValidationSetRtpCheck(RvSrtpSsrcValidation *thisPtr, RvBool rtpCheck);}
	{params:
		{param: {t: RvSrtpSsrcValidation *} {n: thisPtr} {d: The ssrc validation object.}}
		{param: {t: RvBool} {n: rtpCheck} {d: RV_TRUE if SRTP initialization vectors should be checked.}}
	}
}
$*/
/*$
{function scope="private":
	{name: rvSrtpSsrcValidationSetRtcpCheck}
	{class: RvSrtpSsrcValidation}
	{include: rvsrtpssrcvalidation.h}
	{description:
        {p: Enables/disables the validation of SSRCs for RTCP.}
	}
    {proto: void rvSrtpSsrcValidationSetRtpCheck(RvSrtpSsrcValidation *thisPtr, RvBool rtcpCheck);}
	{params:
		{param: {t: RvSrtpSsrcValidation *} {n: thisPtr} {d: The ssrc validation object.}}
		{param: {t: RvBool} {n: rtcpCheck} {d: RV_TRUE if SRTCP initialization vectors should be checked.}}
	}
}
$*/

#if defined(__cplusplus)
}
#endif

#endif
