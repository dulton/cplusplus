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

#ifndef _RV_RTPTSESSION_H_
#define _RV_RTPTSESSION_H_

#include "rvaddress.h"
#include "rvrandomgenerator.h"
#include "rvrtpencryptionplugin.h"
#include "rtp.h"
#include "rtcp.h"
#include "rvrtplogger.h"
#include "rtptSender.h"
#include "rtptSessionCallbacks.h"
#include "rtptRtpDesEncryption.h"
#include "rtptRtp3desEncryption.h"
#include "rtptSrtpaesencryption.h"


#ifdef __cplusplus
extern "C" {
#endif
    
#define RTPT_MAX_SHUTDOWN_REASON_LEN               (20)
#define RTPT_MAX_REMOTE_ADDRS                      (20) 
#define RTPT_MAX_NUMBER_OF_SESSIONS_IN_TEST_CLIENT (50)
#define RTPT_MAX_NUMBER_OF_CLIENTS                 (5)

//#define __RTP_PRINT_PACK

typedef struct rtptObj_Tag rtptObj;

typedef struct rtptInstanceGlobalData__
{
	RvRtpDesEncryption     desEncryptor;       /* DES encryption implementation */
    RvRtp3DesEncryption    tripleDesEncryptor; /* 3DES encryption implementation */	

} rtptEncryptors;

typedef struct srtpparams__
{
    RvNetAddress        address;            /* client address */
    RvUint32            ssrc;               /* ssrc */
    RvUint32            roc;                /* rollover counter */
    RvUint32            seqnum;             /* RTP sequence number */
    RvUint32            index;              /* RTCP index */

    RvUint64            indexRTP;           /* RTP  index for changing key*/
    RvUint64            indexRTCP;          /* RTCP index for changing key*/
    
    RvUint32            clientId;           /* client number */
    RvBool              valid;              /* valid data */
} rtptSrtpParams;


typedef struct RtptSessionObj__
{
    rtptObj             *rtpt;              /* pointer to the RTP Test object */
    RvBool              open;
    RvBool              terminiatedSender; 
    RvBool              terminiatedReceiver; 
    RvUint32            id;                 /* Id given to the call */
    rtptEncryptors*     encryptors;         /* pointer to the Global per test appl data */
    RvRtpSession        hRTP;               /* RTP session handle */
    RvRtcpSession       hRTCP;              /* RTCP session handle */
    RvSendThread        sender;             /* RTP sender thread */
    RvSendThread        receiver;           /* only in blocking case */
    rtpRtcpCallbacks    cb;                 /* All RTP/RTCP sessions callbacks */
    RvChar              reason[RTPT_MAX_SHUTDOWN_REASON_LEN]; /* reason for shutdown */
    RvRandomGenerator   randomGenerator;    /* Random number generator */
    RvLock              timeLock;           /* Lock to guard timestamp info */
    RvInt64             rtpTimestampTime;   /* Time last RTP packet was sent */
    RvUint32            rtpTimestamp;       /* RTP timestamp last time RTP packet was sent */
    RvUint32            rtpRate;            /* RTP timestamp increase between RTP packet being sent */
    RvInt64             rtpPeriod;          /* Time between RTP packets being sent */
    RvUint32            readStat;           /* RvRtpRead statistics */
    RvUint16            seqNum;             /* sequence number to use */

    /* addresses - this will hold a local copy of the addresses passed to the RTP stack, since
    there is no way to extract these addresses from the stack later */
    RvAddress           localAddr;
    RvAddress           remoteAddrs[RTPT_MAX_REMOTE_ADDRS];
    int                 numRemoteAddrs;
    RvAddress           mcastAddr;
    /* SRTP local params */
#if defined(SRTP_ADD_ON)    
    RvSrtpAesEncryption    aesEncryption;       /* AES encryption plugin implementation for SRTP only */
#endif
    rtptSrtpParams      localsrtp;
    rtptSrtpParams      remotesrtp[RTPT_MAX_NUMBER_OF_CLIENTS][RTPT_MAX_NUMBER_OF_SESSIONS_IN_TEST_CLIENT];


} rtptSessionObj;



/********************************************************************************************
 * rtptSessionObjInit
 * purpose : initializes the rtptSessionObj 
 * input   : rtpt - pointer to the RTP Test object (rtptObj)
 *           s - pointer to the session object (rtptSessionObj)
 *           pencryptors - pointers to the test encryptors
 * output  : none
 * return  : RV_OK
 ********************************************************************************************/
RvStatus rtptSessionObjInit(rtptObj * rtpt, rtptSessionObj* s, rtptEncryptors* pencryptors);

/********************************************************************************************
 * function rtptSessionOpen
 * purpose : opens RTP and RTCP sessions
 * input   : text    - address string 
 *           port    - address port
 *           IPscope - IPV6 scope
 *           seqNum  - Sequence number to use (zero for none)
 *           bUseSsrc- True iff we want to 
 *           seqNum  - Sequence number to use
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionOpen(
    rtptSessionObj*     s,
    RvUint16            Port,
    const RvChar *      IPstring,
    RvUint32            IPscope,
    RvUint16            seqNum,
    RvBool              bUseSsrc,
    RvUint32            ssrc);

/********************************************************************************************
 * function rtptSessionClose
 * purpose : closes session
 * input   : s - pointer to session object (rtptSessionObj) 
 * output  : nono
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionClose     (rtptSessionObj* s);

/********************************************************************************************
 * function rtptSessionAddRemoteAddr
 * purpose : adds session sending address (or multicast address)
 * input   : s - pointer to session object (rtptSessionObj) 
 *           Port - port
 *           IPstring - network IP address string
 *           IPscope 
 * output  : nono
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionAddRemoteAddr  (rtptSessionObj* s, RvUint16 Port, const RvChar *IPstring, RvUint32 IPscope);
/********************************************************************************************
 * function rtptSessionSetPayload
 * purpose : sets payload related sending/receiving functions
 * input   : s - pointer to session object (rtptSessionObj) 
 *           payload - payload type
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSetPayload(rtptSessionObj* s, RvUint32 Payload);
/********************************************************************************************
 * function rtptSessionStartRead
 * purpose : sets RTP event handler in non-blocking mode
 *           or starts RTP receiver task, which waits blocked for each RTP session packet
 *           which will be received
 * input   : s - pointer to session object (rtptSessionObj) 
 *           bBlocked - is session blocked boolean
 * output  : addressPtr - pointer to RvNetAddress
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionStartRead (rtptSessionObj* s, RvBool bBlocked);
/********************************************************************************************
 * function rtptSessionSet3DesEncryption
 * purpose : sets 3DES encryption for the session
 * input   : s - pointer to session object (rtptSessionObj) 
 *           ekeyData - encrypting key
 *           dkeyData - dencrypting key
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSet3DesEncryption(
	IN rtptSessionObj* s, IN const RvChar *ekeyData, IN const RvChar *dkeyData);
/********************************************************************************************
 * function rtptSessionSetDesEncryption
 * purpose : sets DES encryption for the session
 * input   : s - pointer to session object (rtptSessionObj) 
 *           ekeyData - encrypting key
 *           dkeyData - dencrypting key
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSetDesEncryption(
	IN rtptSessionObj* s, IN const RvChar *ekeyData, IN const RvChar *dkeyData);

/********************************************************************************************
 * function rtptSessionSendManualRTCPSR
 * purpose : sends Manual RTCP SR
 * input   : s - pointer to session object (rtptSessionObj) 
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSendManualRTCPSR(IN rtptSessionObj* s);

/********************************************************************************************
 * function rtptSessionSendManualRTCPRR
 * purpose : sends Manual RTCP RR
 * input   : s - pointer to session object (rtptSessionObj) 
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSendManualRTCPRR(IN rtptSessionObj* s);

/********************************************************************************************
 * function rtptSessionSendManualBYE
 * purpose : sends Manual RTCP immediate BYE
 * input   : s - pointer to session object (rtptSessionObj) 
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSendManualBYE   (IN rtptSessionObj* s, IN const RvChar *reason);

/********************************************************************************************
 * function rtptSessionSendShutdown
 * purpose : sends Manual RTCP BYE after apropriate delay defined in RFC 3550
 * input   : s - pointer to session object (rtptSessionObj) 
 *           reason - RTCP BYE reason
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSendShutdown    (IN rtptSessionObj* s, IN const RvChar *reason);

/********************************************************************************************
 * function rtptSessionSendRTCPAPP
 * purpose : sends RTCP APP message
 * input   : s - pointer to session object (rtptSessionObj) 
 * Note: with dummy user data  
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSendRTCPAPP     (IN rtptSessionObj* s);

/********************************************************************************************
 * function rtptSessionListenMulticast
 * purpose : joins the RTP and RTCP session to listen to multicast address
 * input   : s - pointer to session object (rtptSessionObj) 
 *           Port - multicast port
 *           IPstring - multicast IP
 *           IPscope  - scope for IPV6
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionListenMulticast (
	IN rtptSessionObj* s, IN RvUint16 Port, IN const RvChar *IPstring, IN RvUint32 IPscope);

/********************************************************************************************
 * function rtptSessionSetRtpMulticastTTL
 * purpose : sets multicast TTL for RTP
 * input   : s - pointer to session object (rtptSessionObj) 
 *           RTPttl - RTP  time to live
 *           RTCPttl - RTCP time to live
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSetRtpMulticastTTL(rtptSessionObj* s, RvInt RTPttl);

/********************************************************************************************
 * function rtptSessionSetRtcpMulticastTTL
 * purpose : sets multicast TTL for  RTCP
 * input   : s - pointer to session object (rtptSessionObj) 
 *           RTPttl - RTP  time to live
 *           RTCPttl - RTCP time to live
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSetRtcpMulticastTTL(rtptSessionObj* s,  RvInt RTCPttl);

/********************************************************************************************
 * function rtptSessionSetRtpTOS
 * purpose : sets TOS for RTP
 * input   : s - pointer to session object (rtptSessionObj) 
 *           RTPtos - RTP  ToS
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSetRtpTOS       (IN rtptSessionObj* s, IN RvInt RTPtos);

/********************************************************************************************
 * function rtptSessionSetRtcpTOS
 * purpose : sets TOS for RTP
 * input   : s - pointer to session object (rtptSessionObj) 
 *           RTCPtos - RTCP  ToS
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSetRtcpTOS      (IN rtptSessionObj* s, IN RvInt RTCPtos);

/********************************************************************************************
 * function rtptSessionSetRtcpBandwith
 * purpose : sets RTCP bandwith (usially 5% of RTP bandwith)
 * input   : s - pointer to session object (rtptSessionObj) 
 *           RTCPband - RTCP bandwith in bytes per second
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSetRtcpBandwith (IN rtptSessionObj* s, IN RvInt RTCPband);

/********************************************************************************************
 * function rtptSessionRemoveRemoteAddr
 * purpose : removes session sending remote address
 * input   : s - pointer to session object (rtptSessionObj) 
 *           Port - port
 *           IP address to be removed
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionRemoveRemoteAddr(
	IN rtptSessionObj* s, IN RvUint16 Port, IN const RvChar *IPstring);

/********************************************************************************************
 * function rtptSessionRemoveAllRemoteAddresses
 * purpose : removes all session remote addresses
 * input   : s - pointer to session object (rtptSessionObj) 
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionRemoveAllRemoteAddresses(IN rtptSessionObj* s);
/********************************************************************************************
 * function rtpAddressConstructByName
 * purpose : constructs RvNetAddress from network string (IPV4 & IPV6) and port and scope for IPV^
 *           
 * input   : text    - address string 
 *           port    - address port
 *           IPscope - IPV6 scope
 * output  : addressPtr - pointer to RvNetAddress
 * return  : RvNetAddress*
 ********************************************************************************************/
RvNetAddress* testAddressConstruct(RvNetAddress* addressPtr, const RvChar* text, RvUint16 port, RvUint32 IPscope);

/********************************************************************************************
 * Rv64UtoA
 * Converts a decimal string to an unsigned 64 bit integer (RvUint64)  .
 * INPUT   : buf                - The buffer where resulting string will be placed
 * OUTPUT  :  num64              - The unsigned 64 bit number to be converted.
 * RETURN  : A pointer to num64 successful otherwise NULL.
 * NOTE:  The buffer (buf) should be at least RV_64TOASCII_BUFSIZE bytes long
 *        or the end of the buffer may be overwritten.
 */
RVCOREAPI RvUint64* RVCALLCONV Rv64AstoRv64(
    OUT RvUint64* num64,
    IN const RvChar *buf);

#ifdef __cplusplus
}
#endif

#endif /* _RV_RTPTSESSION_H_ */
