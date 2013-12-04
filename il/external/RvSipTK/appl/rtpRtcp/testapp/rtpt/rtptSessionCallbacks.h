/***********************************************************************
Copyright (c) 2003 RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Ltd.. No part of this document may be reproduced in any
form whatsoever without written prior approval by RADVISION Ltd..
RADVISION Ltd. reserve the right to revise this publication and make
changes without obligation to notify any person of such revisions or
changes.
***********************************************************************/

#ifndef _RV_SESSIONCALLBACKS_H_
#define _RV_SESSIONCALLBACKS_H_

#include "rtp.h"
#include "rtcp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef RvStatus (*rtpread_CB) (
							   IN RvRtpSession rtpSession,
							   IN RvUint8* rtpPacketPtr,
							   IN RvInt32 dataSize,
							   IN RvRtpParam* rtpParamPtr
							   );
typedef void (*rtpsender_CB)(IN RvThread *threadPtr, 
						 IN void *dataPtr);

typedef struct sessioncalbacks__
{
	RvRtpEventHandler_CB       rtpReadEvent;
	RvRtcpEventHandler_CB      rtcpEvent;
	RvRtcpAppEventHandler_CB   appEvent;
	RvRtcpByeEventHandler_CB   byeEvent;
	RvRtpShutdownCompleted_CB  shutdownEvent;
    rtpread_CB                 rtpreadcb;
	rtpsender_CB               rtpsendercb;
} rtpRtcpCallbacks;

#ifdef __cplusplus
}
#endif

#endif /* _RV_SESSIONCALLBACKS_H_ */

