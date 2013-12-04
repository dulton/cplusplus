/************************************************************************
 File Name	   : RtcpTypes.h
 Description   : scope: Private
     RTCP related type definitions
*************************************************************************
************************************************************************
        Copyright (c) 2005 RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Ltd.. No part of this document may be reproduced in any
form whatsoever without written prior approval by RADVISION Ltd..

RADVISION Ltd. reserve the right to revise this publication and make
changes without obligation to notify any person of such revisions or
changes.
***********************************************************************/

#ifndef __RTCP_TYPES_H__
#define __RTCP_TYPES_H__

#include "rvtypes.h"
#include "rvrtpconfig.h"
#include "rtcp.h"
#include "rtpProfilePlugin.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAXRTCPPACKET             (1470)
#define RTCP_WORKER_BUFFER_SIZE   (1600)

typedef enum
{
   RTCP_SR   = 200,               /* sender report            */
   RTCP_RR   = 201,               /* receiver report          */
   RTCP_SDES = 202,               /* source description items */
   RTCP_BYE  = 203,               /* end of participation     */
   RTCP_APP  = 204,               /* application specific     */
   RTCP_LAST_LEGAL  = RTCP_APP,   /* application specific     */   
   RTCP_EMPTY_RR = 1 /* empty RR */
} rtcpType;

typedef struct
{
   RvUint64     tNNTP;
   RvUint32     tRTP;

   RvUint32     nPackets;
   RvUint32     nBytes;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    RvUint32    nPackets_prior; //transferred packets since last report; local info
#endif
/* SPIRENT END */
} rtcpSR;

typedef struct
{
   RvUint32 ssrc;
   RvUint32 bfLost;      /* 8Bit fraction lost and 24 bit cumulative lost */
   RvUint32 nExtMaxSeq;
   RvUint32 nJitter;
   RvUint32 tLSR;
   RvUint32 tDLSR;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
   RvUint32 rxPacketsSLR;
#endif
/* SPIRENT END */
} rtcpRR;

typedef struct
{
   RvUint16 max_seq;               /* highest seq. number seen */
   RvUint32 cycles;                /* shifted count of seq. number cycles */
   RvUint32 base_seq;              /* base seq number */
   RvUint32 bad_seq;               /* last 'bad' seq number + 1 */
   RvUint32 probation;             /* sequ. packets till source is valid */
   RvUint32 received;              /* packets received */
   RvUint32 expected_prior;        /* packet expected at last interval */
   RvUint32 received_prior;        /* packet received at last interval */
   RvUint32 transit;               /* relative trans time for prev packet */
   RvUint32 jitter;                /* estimated jitter */
   /* ... */
} rtpSource;

typedef struct
{
   RvUint8  type;
   RvUint8  length;
   char     value[RTCP_MAXSDES_LENGTH + 1];     /* leave a place for an asciiz */
} rtcpSDES;


typedef struct
{
   RvInt32        invalid;
   RvBool     active;        /* received packet from SSRC (field ssrc) */
   rtpSource  src;

   RvUint32   ssrc;
   RvUint32   tLSRmyTime;
   rtcpSR     eSR;
   rtcpRR     eToRR;
   rtcpRR     eFromRR;
   rtcpSDES   eSDESCname;
} rtcpInfo;

typedef struct
{
   RvUint32 bits;
   RvUint32 ssrc;
} rtcpHeader;


typedef struct
{
   RvInt32   active;
   RvInt32   collision;
   RvUint32  ssrc;
   RvUint32  timestamp;
   RvUint32  rtpClockRate;    /* clock rate of RTP media */
   RvUint8   rtpPayloadType;  /* payloadtype of RTP media */
   RvUint64  lastPackNTP;     /* last sent packet NTP time */
   RvUint32  lastPackRTPts;   /* last sent packet RTP timestamp */
   rtcpSR    eSR;
   rtcpSDES  SDESbank[RV_RTCP_SDES_PRIV+1];
} rtcpMyInfo;

typedef struct
{
   RvBool    initialized;      /* first time initialization */
   RvInt64   previousTime;     /* time of last scheduled transmission */
   RvInt64   nextTime;         /* time of next scheduled transmission */
   RvUint32  pmembers;         /* number of members at time nextTransmitTime was computed */
   RvUint32  members;          /* current number of members */
   RvUint32  senders;          /* current number of senders */
   RvUint32  rtcpBandwidth;    /* bandwidth for RTCP session */
   RvInt32   lastRTCPPacketSize;
   RvInt32   aveRTCPPacketSize;
} rtcpTxInterval;

typedef struct
{
   RvBool                       isAllocated;
   RvBool                       isShutdown;
   RvSocket                     socket;
   RvSelectEngine               *selectEngine;     /* Select engine used by this fd */
   RvSelectFd                   selectFd;
   rtcpMyInfo                   myInfo;
   RvRtpAddressList             addressList; /* address list to store the remote addresses for unicast/multicast */
   RvBool                       remoteAddressSet;
   RvTimer                      timer;             /* Timer of this RTCP session */
   RvBool                       isTimerSet;        /* RV_TRUE if we started a timer for this session */
   rtcpInfo*                    participantsArray; /* session members information array */
   RvInt32                      sessionMembers;
   RvInt32                      maxSessionMembers;
   RvRtcpEventHandler_CB        rtcpRecvCallback;
   void*                        haRtcp;
   //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
   RvRtcpEventHandler_CB        rtcpSendCallback;
   void*                        haRtcpSend; //send context
   RvUint32 rtcpReportInterval;
#endif
   //SPIRENT_END
#if defined(UPDATED_BY_SPIRENT)
   RvRtcpAppTIPEventHandler_CB  rtcpAppTIPRecvCallback; /* Callback for received RTCP APP messages */
   void*                        rtcpAppTIPContext;
#endif
   RvRtcpAppEventHandler_CB     rtcpAppRecvCallback; /* Callback for received RTCP APP messages */
   void*                        rtcpAppContext;
   RvRtcpByeEventHandler_CB     rtcpByeRecvCallback; /* Callback for received RTCP BYE messages */
   void*                        rtcpByeContext;
   RvChar*                      byeReason;
   RvUint32                     byeReasonLength;
   RvRtpShutdownCompleted_CB    rtpShutdownCompletedCallback; /* Callback defines the function for RTP session closing */
   void*                        rtcpShutdownCompletedContext;
   RvRtcpSDESMessageReceived_CB rtcpSDESMessageArrivedCallback;
   RvRtpSession                 rtpSession;
   rtcpTxInterval               txInterval;
   RvRtpEncryptionPlugIn        *encryptionPlugInPtr;   /* Registered encryption plug in */
   RvRtpEncryptionKeyPlugIn     *encryptionKeyPlugInPtr;/* Registered encryption key plug in */
   RvRtpEncryptionMode          encryptionMode;
   RvUint32                     index; /* RTCP sequence number (required by some encryption plugins) */
   RvLock                       lock; /* Lock of this session. Used to protect the session members */
   RtpProfilePlugin             *profilePlugin;

} rtcpSession;


//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
#define RV_RTCP_MAXIPS RV_RTP_MAXIPS
#else
#define RV_RTCP_MAXIPS RvUint32Const(20)
#endif
//SPIRENT_END

/* Internal RTCP instance used. There a single such object */
typedef struct
{
    RvSelectEngine*     selectEngine; /* select engine used for RTP/RTCP messages */
    RvAddress           localAddress;
    RvAddress           hostIPs[RV_RTCP_MAXIPS]; /* Local host IPs */
    RvUint32            addresesNum;      /* number of addresses in host list */
    RvUint32            timesInitialized; /* Times the RTP was initialized */

    RvTimerQueue*       timersQueue; /* Timers queue to use */

    RvRandomGenerator   randomGenerator; /* Random numbers generator to use */
} RvRtcpInstance;

extern RvRtcpInstance rvRtcpInstance;

#ifdef __cplusplus
}
#endif

#endif  /* __RTCP_TYPES_H__ */







