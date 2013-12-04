/***********************************************************************
        Copyright (c) 2002 RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Ltd.. No part of this document may be reproduced in any
form whatsoever without written prior approval by RADVISION Ltd..

RADVISION Ltd. reserve the right to revise this publication and make
changes without obligation to notify any person of such revisions or
changes.
***********************************************************************/

#ifndef __RTPUTIL_H
#define __RTPUTIL_H


#include "rvselect.h"
#include "rvnet2host.h"
#include "rvtimer.h"
#include "rvlog.h"
#include "rvloglistener.h"
#include "rvrtplogger.h"
#include "rtp.h"
#include "rvrtpencryptionplugin.h"
#include "rvrtpencryptionkeyplugin.h"
#include "rvrtpaddresslist.h"
#include "rvrandomgenerator.h"
#include "rtpProfilePlugin.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This struct is here because some applications like to include it in their own code and access
   it directly (bad, but what can we do?) */
typedef struct
{
    RvBool                   isAllocated;            /* RV_TRUE if the RTP package allocated this struct internally */
    RvSocket                 socket;                 /* UDP socket used for this session */
    RvSelectEngine           *selectEngine;          /* Select engine used by this fd */
    RvSelectFd               selectFd;               /* fd object to use for non-blocking mode */
    RvUint32                 sSrc;                   /* Synchronization source (SSRC) */
    RvUint32                 sSrcMask;
    RvUint32                 sSrcPattern;
    RvRtpEventHandler_CB     eventHandler;           /* RTP received message indication event handler */
    void *                   context;                /* RTP received message indication event handler context */
    RvUint16                 sequenceNumber;         /* RTP session's sequence number */
	RvRtpAddressList         addressList;            /* address list to store the remote addresses for unicast/multicast */
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
   RvAddress                remoteAddress; /* remote address for peer-to-peer session */
   char                     sockdata[32]; /* remote socket data for peer-to-peer session */
   int                      socklen; /* remote socket data length for peer-to-peer session */
#else
  /*  RvAddress              remoteAddress;              Remote address of this session */
#endif
//SPIRENT_END
    RvBool                   remoteAddressSet;       /* RV_TRUE if we already set the remote address */
    RvBool                   useSequenceNumber;      /* TRUE, if it is used users sequence numbers, FALSE - automatic */
    RvRtcpSession            hRTCP;                  /* Associate RTCP session if available */
/* Security hooks*/
	RvRtpEncryptionPlugIn    *encryptionPlugInPtr;   /* Registered encryption plug in */
	RvRtpEncryptionKeyPlugIn *encryptionKeyPlugInPtr; /* Registered encryption key plug in */
	RvRtpEncryptionMode      encryptionMode;          /* RFC1889, H235_PADDING, H235_CIPHERTEXTSTEALING */

    RtpProfilePlugin*        profilePlugin;           /* profile plugin for RTP/SRTP */
/* SRTP plugin information */
    RvUint32                 roc;                     /* srtp rollover counter   */
    RvLock                   lock;     
    
} RvRtpSessionInfo;                                   /* RvRtpSession */

 
typedef struct _rtpLogManager
{
	RvBool                bExternal;        /* TRUE if External Log Manager is executed */
	RvLogMgr              logMngr;          /* used when internal logmanager executed */

    RvLogSource           logSource[RVRTP_MODULES_NUMBER]; /* all log sources for RTP lib */
    RvBool                bLogSourceInited[RVRTP_MODULES_NUMBER]; /* TRUE, if logsource for specific module is initialized */
    
	RvLogListener         listener;         /* log listener */
    RvRtpPrintLogEntry_CB printCB;          /* logger entry print external function */
	void*                 logContext;       /* context for printCB */

} RvRtpLogManager;


//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
#define RV_RTP_MAXIPS RvUint32Const(8192)
#else
#define RV_RTP_MAXIPS RvUint32Const(20)
#endif
//SPIRENT_END

/* Internal RTP instance used. There a single such object */
typedef struct
{
    RvSelectEngine*       selectEngine;           /* Select engine used */
    
    RvAddress             rvLocalAddress;         /* local address */
    RvAddress             hostIPs[RV_RTP_MAXIPS]; /* Local host IPs */
    RvUint32              addressesNum;           /* maximal number of addresses stored */
    RvRandomGenerator     randomGenerator;        /* Random numbers generator to use */
    RvRtpLogManager       logManager;             /*  Logger mannager */
    
} RvRtpInstance;

/* Private functions, which are not accessible to user */ 
RVAPI
void   RVCALLCONV ConvertToNetwork(void *data, RvInt32 pos, RvInt32 n);
RVAPI
void   RVCALLCONV ConvertFromNetwork(void *data, RvInt32 pos, RvInt32 n);

RVAPI
void   RVCALLCONV ConvertToNetwork2(void *data, RvInt32 pos, RvInt32 n);
RVAPI
void   RVCALLCONV ConvertFromNetwork2(void *data, RvInt32 pos, RvInt32 n);

RVAPI
RvBool   RVCALLCONV  isMyIP(RvAddress* address);


RvStatus      rtpGetLogManager(OUT RvRtpLogManager** lmngrPtr);
/* Private function only to allow access to the SRTP module */
RVAPI
RvLogSource* RVCALLCONV   rtpGetSource(RvRtpModule module);

RvBool   rtcpSessionIsSessionInShutdown(
	 IN RvRtcpSession hRTCP);
/* Private function only to allow access to the SRTP module */
RVAPI
RvUint32  RVCALLCONV rtcpSessionGetIndex(
     IN RvRtcpSession hRTCP);

RvStatus rtcpSetRtpSession(
     IN RvRtcpSession hRTCP, 
     IN RvRtpSession hRTP);

/* Private function only to allow access to the SRTP module */
RVAPI
RvStatus  RVCALLCONV RvRtcpSetProfilePlugin(
    IN RvRtcpSession hRTCP, 
    IN RtpProfilePlugin* profilePlugin);

#ifdef __cplusplus
}
#endif

#endif  /* __RTPUTIL_H */
