/************************************************************************
 File Name	   : rvsrtplog.h
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
#if !defined(RVSRTPLOG_H)
#define RVSRTPLOG_H

#include "rvtypes.h"
#include "rtputil.h"

#ifdef __cplusplus
extern "C" {
#endif

/* SRTP/SRTP just uses the log source in RTP/RTCP, so just provide macros */
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/* Get the log source (RvLogSource *) */
#define rvSrtpLogSourcePtr rtpGetSource(RVRTP_SRTP_MODULE)

/* Logging functions for the various levels. Remember that they still */
/* require the odd function format. An example of which is: */
/* rvSrtpLogError((rvSrtpLogGetSource(), "rvSrtp: User did bad thing #%d", errcode)); */
#define rvSrtpLogExcep(logParams)   RvLogExcep(rvSrtpLogSourcePtr, logParams)
#define rvSrtpLogError(logParams)   RvLogError(rvSrtpLogSourcePtr, logParams)
#define rvSrtpLogWarning(logParams) RvLogWarning(rvSrtpLogSourcePtr, logParams)
#define rvSrtpLogInfo(logParams)    RvLogInfo(rvSrtpLogSourcePtr, logParams)
#define rvSrtpLogDebug(logParams)   RvLogDebug(rvSrtpLogSourcePtr, logParams)
#define rvSrtpLogEnter(logParams)   RvLogEnter(rvSrtpLogSourcePtr, logParams)
#define rvSrtpLogLeave(logParams)   RvLogLeave(rvSrtpLogSourcePtr, logParams)

#else
    /* Get the log source (RvLogSource *) */
#define rvSrtpLogSourcePtr (NULL)
    
    /* Logging functions for the various levels. Remember that they still */
    /* require the odd function format. An example of which is: */
    /* rvSrtpLogError((rvSrtpLogGetSource(), "rvSrtp: User did bad thing #%d", errcode)); */
#define rvSrtpLogExcep(logParams)   
#define rvSrtpLogError(logParams)   
#define rvSrtpLogWarning(logParams) 
#define rvSrtpLogInfo(logParams)    
#define rvSrtpLogDebug(logParams)   
#define rvSrtpLogEnter(logParams)  
#define rvSrtpLogLeave(logParams)  
    
#endif

#ifdef __cplusplus
}
#endif

#endif
