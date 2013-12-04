/************************************************************************
 File Name	   : rtptSession.c
 Description   : RTP/RTCP sessions related functions
*************************************************************************
 Copyright (c)	2004 RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVision Inc. 
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVision Inc. 
 
 RADVision Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
*************************************************************************/
#include <string.h>
#include "rvstdio.h"
#include "rv64ascii.h"
#include "rvmemory.h"
#include "rtptSession.h"
#include "rtptPayloads.h"
#include "rtptUtils.h"

#ifdef __cplusplus
extern "C" {
#endif
    
static RvLogMgr*    pAppMngLog = NULL;
#if defined(RTPT_TA)
#include "RTPT_init.h" /* for rtptGetRtptObj() */
#else
#include "RTPPlugin.h" /* for rtptGetRtptObj() */
#endif
#define pAppRvLog rtptGetLogSource(rtptGetRtptObj())

static RvBool RVCALLCONV rtcpAppEventHandler (IN  RvRtcpSession  hRTCP,
                                       IN  void * rtcpAppContext,
                                       IN  RvUint8        subtype,
                                       IN  RvUint32       ssrc,
                                       IN  RvUint8*       name,
                                       IN  RvUint8*       userData,
                                       IN  RvUint32       userDataLen);

/********************************************************************************************
 * rtptSessionObjInit
 * purpose : initializes the rtptSessionObj 
 * input   : rtpt - pointer to the RTP Test object (rtptObj)
 *           s - pointer to the session object (rtptSessionObj)
 *           pencryptors - pointers to the test encryptors
 * output  : none
 * return  : RV_OK
 ********************************************************************************************/
RvStatus rtptSessionObjInit(rtptObj * rtpt, rtptSessionObj* s, rtptEncryptors* pencryptors)
{
    RvUint32 id =0;
	if (NULL==s)
		return RV_ERROR_NULLPTR;
    id = s->id;
	memset(s,0, sizeof(rtptSessionObj));
	s->encryptors = pencryptors;
    s->rtpt       = rtpt;
    s->id         = id;
	return RV_OK;
}
/********************************************************************************************
 * rtpEventHandler
 * purpose : RTP event handler, when received RTP message this function is called. 
 * input   : hRTP - RTP session handle
 *           context - calling context
 * output  : none
 * return  : RV_OK
 ********************************************************************************************/
void rtpEventHandler(IN  RvRtpSession  hRTP, IN  void *       context)
{
    RvUint8          rtpPacket[RV_SAMPLE_RTP_PACK_MAX_SIZE];
    RvRtpParam       rtpParam;
    RvStatus         status = RV_OK;
    RvUint32         timestamp = 0;
	rtptSessionObj     *thisPtr = (rtptSessionObj *)context;
    
	RvLogEnter(pAppRvLog,(pAppRvLog, "RvRtpSampleEventHandler"));
    if (!thisPtr->open)
        return;
	
    RvRtpParamConstruct(&rtpParam);

    status = RvRtpRead(hRTP, rtpPacket, sizeof(rtpPacket), &rtpParam);
    if (status>=0)
    {
        RvRtcpSession hRTCP = RvRtpGetRTCPSession(hRTP);
#if (RVRTP_SAMPLE_PAYLOAD_TYPE == RVRTP_SAMPLE_PCMU)||(RVRTP_SAMPLE_PAYLOAD_TYPE == RVRTP_SAMPLE_AMR)
        RvInt64     currentTime = RvInt64Const(1,0,0);
        timestamp = rtptTimestamp(thisPtr, &currentTime);
        RvLogDebug(pAppRvLog,(pAppRvLog, "Receiving Timestamp:%lu", timestamp));
#else
        timestamp = 0; /* need a specific calculation */
#endif
#ifdef      __RTP_PRINT_PACK    
        {
            RvInt32 count;
            printf("\nreceived packet:\n");
            for (count = rtpParam.sByte; count < rtpParam.len + rtpParam.sByte; count++)
                printf("%x%x ", rtpPacket[count]>>4, rtpPacket[count]&0x0F);
            printf("\n");
        }
#endif
        /* Timestamp in terms of senders, when packet is received */
        if (hRTCP)
            RvRtcpRTPPacketRecv(hRTCP, rtpParam.sSrc, timestamp,  rtpParam.timestamp, rtpParam.sequenceNumber);
		
		thisPtr->cb.rtpreadcb(hRTP, rtpPacket, rtpParam.len, &rtpParam);
        thisPtr->readStat++;
        if (thisPtr->readStat%200 == 0)
            RvLogInfo(pAppRvLog,(pAppRvLog, "RTP Session (%#x): received %u RTP packets", (RvUint32)thisPtr->hRTP, thisPtr->readStat));
    }
    RvLogLeave(pAppRvLog,(pAppRvLog, "RvRtpSampleEventHandler"));
    RV_UNUSED_ARG(thisPtr);
}

/********************************************************************************************
 * rtcpAppEventHandler
 * purpose : RTCP APP event handler, when received RTCP APP message this function is called. 
 * input   : hRTCP - RTCP session handle
 *           rtcpAppContext - calling context
 *           subtype - subtype of APP message
 *           ssrc    - SSRC of sending session
 *           name    - name of RTCP APP message
 *           userData -  pointer to user data
 *           userDataLen  -  user data length     
 * output  : none
 * return  : RV_TRUE
 ********************************************************************************************/

RvBool RVCALLCONV rtcpAppEventHandler (IN  RvRtcpSession  hRTCP,
                                       IN  void * rtcpAppContext,
                                       IN  RvUint8        subtype,
                                       IN  RvUint32       ssrc,
                                       IN  RvUint8*       name,
                                       IN  RvUint8*       userData,
                                       IN  RvUint32       userDataLen)
{
    RvChar buffer[1500];
    RvInt32 Count = 0;
    RvUint8 namez[5];
    RvUint8 userDataz[256];
    memcpy(namez,name,4);
    namez[4]='\0';
    memcpy(userDataz, userData, userDataLen);
    userDataz[userDataLen]='\0';
    Count += sprintf (buffer, "Received APP RTCP report:subtype=%u\n", subtype);
    Count += sprintf ((buffer + Count), "ssrc=%u\n", ssrc);
    Count += sprintf ((buffer + Count), "Name = %s\n", namez);
    Count += sprintf ((buffer + Count), "userDataName = %s, userDataLength = %d,\n", userDataz, userDataLen);
    RvLogInfo(pAppRvLog, (pAppRvLog, buffer));
    RV_UNUSED_ARG(rtcpAppContext);
    RV_UNUSED_ARG(hRTCP);
    return RV_TRUE;
}
/********************************************************************************************
 * rtpShutdownCompletedEvent
 * purpose : RTP shutdown completed event handler, 
 *           when after calling to RvRtpSessionShutdown() this function is called. 
 * input   : RTP - RTP session handle
 *           reason - BYE reason
 *           rtpContext - calling context     
 * output  : none
 * return  : none
 ********************************************************************************************/
RVAPI
void RVCALLCONV rtpShutdownCompletedEvent (IN  RvRtpSession  hRTP,
                                           IN RvChar* reason,
                                           IN  void * rtpContext)
{
    rtptSessionObj* s = (rtptSessionObj*) rtpContext;


    RvLogEnter(pAppRvLog,(pAppRvLog, "rtpShutdownCompletedEvent()"));
    RvLogInfo(pAppRvLog, (pAppRvLog, "Shutdown is completed, calling to close the session"));
    rtptUtilsEvent(s->rtpt, "RtpShutdown", s, "");
    rtptSessionClose(s);
    RvLogLeave(pAppRvLog,(pAppRvLog, "rtpShutdownCompletedEvent()"));
    RV_UNUSED_ARG(reason);
    RV_UNUSED_ARG(hRTP);
}
/********************************************************************************************
 * rtcpByeEventHandler
 * purpose : RTCP BYE event handler
 *           when RTCP stack receive RTCP BYE message - this function is called. 
 * input   : hRTCP - RTCP session handle
 *           userReason - RTCP BYE reason
 *           userReasonLen - RTCP BYE reason length   
 * output  : none
 * return  : RV_TRUE
 ********************************************************************************************/
RvBool RVCALLCONV rtcpByeEventHandler(IN  RvRtcpSession  hRTCP,
									  IN  void * rtcpByeContext,
									  IN  RvUint32       ssrc,
									  IN  RvUint8*       userReason,
									  IN  RvUint32       userReasonLen)
{
    RvInt8 reasonZ[256];
    RvLogEnter(pAppRvLog,(pAppRvLog, "rtcpByeEventHandler()"));
    memcpy(reasonZ, userReason, userReasonLen);
    reasonZ[userReasonLen] = '\0';
    RvLogInfo(pAppRvLog, (pAppRvLog, "Received RTCP BYE report ssrc=%#x, reason:%s", ssrc, reasonZ));
    RV_UNUSED_ARG(rtcpByeContext);
    RV_UNUSED_ARG(hRTCP);
    RV_UNUSED_ARG(ssrc); /* warning elimination, when LOG_MASK is NONE */ 
    RvLogLeave(pAppRvLog,(pAppRvLog, "rtcpByeEventHandler()"));
    return RV_TRUE;
}

#include "rvntptime.h"
#include "rvclock.h"
#define reduceNNTP(a) (RvUint64ToRvUint32(RvUint64And(RvUint64ShiftRight(a, 16), RvUint64Const(0, 0xffffffff))))
static RvUint64 getNNTPTime(void)
{
    RvTime t;
    RvNtpTime ntptime;
    RvUint64 result;
    RvLogEnter(pAppRvLog, (pAppRvLog, "getNNTPTime"));
    RvTimeConstruct(0, 0, &t); /* create and clear time structure */
    RvClockGet(NULL, &t); /* Get current wall-clock time */
    RvNtpTimeConstructFromTime(&t, RV_NTPTIME_ABSOLUTE, &ntptime); /* convert to NTP time */
    result = RvNtpTimeTo64(&ntptime, 32, 32); /* Convert to format getNNTPTime returns */
    RvNtpTimeDestruct(&ntptime); /* clean up */
    RvTimeDestruct(&t); /* clean up */
    RvLogLeave(pAppRvLog, (pAppRvLog, "getNNTPTime"));
    return result;
}
/********************************************************************************************
 * rtcpEventHandler
 * purpose : RTCP  event handler
 *           when RTCP stack receive any RTCP packet - this function is called. 
 * input   : hRTCP - RTCP session handle
 *           context - user context
 *           ssrc - packet SSRC
 * output  : none
 * return  : none
 ********************************************************************************************/
void RVCALLCONV rtcpEventHandler(IN RvRtcpSession hRTCP,
								 IN void *context,
								 IN RvUint32 ssrc)
{
    RvRtcpINFO info, info1;
    rtptSessionObj* s          = (rtptSessionObj*)context;
    RvUint32        localSSRC  = RvRtpGetSSRC(s->hRTP);
    RvUint32        timeReciv  =  reduceNNTP(getNNTPTime()); /* not accurate must be calculated at time of receiving */
    float           rtd = 0.0;
    
    RvRtcpGetSourceInfo(hRTCP, localSSRC, &info1);
    RvRtcpGetSourceInfo(hRTCP, ssrc, &info);
	RvLogInfo(pAppRvLog, (pAppRvLog, "========================= RECEIVED RTCP REPORT ======================"));
	RvLogInfo(pAppRvLog, (pAppRvLog, "Local RTP:%#x, RTCP:%#x. Received RTCP report from ssrc=%#x CNAME=\"%s\"", 
        (RvUint32) s->hRTP, (RvUint32) s->hRTCP, ssrc, info.cname));
    if (info.sr.valid)
    {
        RvLogInfo(pAppRvLog, (pAppRvLog, "SR mNTPtimestamp=%#x", info.sr.mNTPtimestamp));
        RvLogInfo(pAppRvLog, (pAppRvLog, "SR lNTPtimestamp=%#x", info.sr.lNTPtimestamp));
        RvLogInfo(pAppRvLog, (pAppRvLog, "SR RTP timestamp=%#x", info.sr.timestamp));
        RvLogInfo(pAppRvLog, (pAppRvLog, "SR packets=%u", info.sr.packets));
        RvLogInfo(pAppRvLog, (pAppRvLog, "SR octets=%u",  info.sr.octets));
    }
    if (info.rrFrom.valid)
    {
        RvLogInfo(pAppRvLog, (pAppRvLog, "RR from: lost since previous SR/RR=%u",  info.rrFrom.fractionLost));
        RvLogInfo(pAppRvLog, (pAppRvLog, "RR from: lost packets=%u",               info.rrFrom.cumulativeLost));
        RvLogInfo(pAppRvLog, (pAppRvLog, "RR from: sequence number=%u",            info.rrFrom.sequenceNumber));
        RvLogInfo(pAppRvLog, (pAppRvLog, "RR from: jitter=%u",                     info.rrFrom.jitter));
        RvLogInfo(pAppRvLog, (pAppRvLog, "RR from: lSR=%#x",                       info.rrFrom.lSR));
        RvLogInfo(pAppRvLog, (pAppRvLog, "RR from: dlSR (delay since last SR)=%#x", info.rrFrom.dlSR));
    }
    if (info.rrTo.valid)
    {
        RvLogInfo(pAppRvLog, (pAppRvLog, "RR to: lost since previous SR/RR=%u",  info.rrTo.fractionLost));
        RvLogInfo(pAppRvLog, (pAppRvLog, "RR to: lost packets=%u",               info.rrTo.cumulativeLost));
        RvLogInfo(pAppRvLog, (pAppRvLog, "RR to: sequence number=%u",            info.rrTo.sequenceNumber));
        RvLogInfo(pAppRvLog, (pAppRvLog, "RR to: jitter=%u",                     info.rrTo.jitter));
        RvLogInfo(pAppRvLog, (pAppRvLog, "RR to: lSR=%#x",                       info.rrTo.lSR));
        RvLogInfo(pAppRvLog, (pAppRvLog, "RR to: dlSR (delay since last SR)=%#x", info.rrTo.dlSR));
    }
    if (info.rrFrom.valid)
    {
        rtd = (float) (((int)(timeReciv -  info.rrFrom.lSR - info.rrFrom.dlSR)))/(float)65535.0;
       /* if (rtd < 0.0)
            rtd=0.0;  not accurate calculations */
        RvLogInfo(pAppRvLog, (pAppRvLog, "Round Trip delay = %#x(A) - %#x(LSR) -%#x(dlSR) = %f sec", timeReciv, info.rrFrom.lSR,  info.rrFrom.dlSR, rtd));
    }
	RvLogInfo(pAppRvLog, (pAppRvLog, "---- LOCAL RTCP REPORT(latest information, that has not been sent yet)--------"));
    
    if (info1.sr.valid)
    {
        RvLogInfo(pAppRvLog, (pAppRvLog, "SR mNTPtimestamp=%#x", info1.sr.mNTPtimestamp));
        RvLogInfo(pAppRvLog, (pAppRvLog, "SR lNTPtimestamp=%#x", info1.sr.lNTPtimestamp));
        RvLogInfo(pAppRvLog, (pAppRvLog, "SR RTP timestamp=%#x", info1.sr.timestamp));
        RvLogInfo(pAppRvLog, (pAppRvLog, "SR packets=%u", info1.sr.packets));
        RvLogInfo(pAppRvLog, (pAppRvLog, "SR octets=%u",  info1.sr.octets));
    }
	RV_UNUSED_ARG(context);
}
/********************************************************************************************
 * Local function rtpAddressConstructByName
 * purpose : constructs RvAddress from network string (IPV4 & IPV6) and port
 *           
 * input   : addressString - address string 
 *           port - address port
 * output  : addressPtr - pointer to RvAddress
 * return  : RvAddress*
 ********************************************************************************************/
static RvAddress *rtpAddressConstructByName(
											 RvAddress *addressPtr,
											 RvChar *addressString,
											 RvUint16 port)
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
RvNetAddress* testAddressConstruct(RvNetAddress* addressPtr, const RvChar* text, RvUint16 port, RvUint32 IPscope)
{
    RvAddress* addressPtrTemp = NULL;
    RvLogEnter(pAppRvLog,(pAppRvLog, "sampleRtpAddressConstruct"));
	
    if (NULL!=addressPtr)
    {
        addressPtrTemp = rtpAddressConstructByName((RvAddress*)addressPtr->address,
            (RvChar*)text, port);
        if (NULL == addressPtrTemp)
        {
            RvLogError(pAppRvLog,(pAppRvLog, "sampleRtpAddressConstruct: wrong address"));
            RvLogLeave(pAppRvLog,(pAppRvLog, "sampleRtpAddressConstruct"));
            return NULL; /* ERROR */
        }
        if (RvNetGetAddressType(addressPtr) == RVNET_ADDRESS_IPV6)
        {
#if (RV_NET_TYPE & RV_NET_IPV6)
            RvAddressSetIpv6Scope((RvAddress*)addressPtr->address, IPscope);
#endif
            RV_UNUSED_ARG(IPscope);
        }
        RvLogLeave(pAppRvLog,(pAppRvLog, "sampleRtpAddressConstruct"));
    }
	RvLogLeave(pAppRvLog,(pAppRvLog, "sampleRtpAddressConstruct"));
    return addressPtr;
}
/* for IOT only - not documented function */
RvStatus RVCALLCONV RvRtpSetRtpSequenceNumber(IN  RvRtpSession  hRTP, RvUint16 sn);
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
    RvUint32            ssrc)
{
	RvNetAddress rtpSessionAddress;
	RvNetAddress* rtpSessionAddressPtr = &rtpSessionAddress;	
	RvChar SDESname[20];
    RvUint32 ssrcPattern = 1, ssrcMask = 0xff;
	
	if (NULL==s)
		return RV_ERROR_NULLPTR;

    if (bUseSsrc)
    {
        ssrcPattern = ssrc;
        ssrcMask = 0xFFFFFFFF;
        rtptUtilsEvent(s->rtpt, "Rec", s, "opensession entry= %d port= %d address= \"%s\" scope= %d seqNum= %d ssrc= %u ",
            s->id, Port, IPstring, IPscope, seqNum, ssrc);
    }
    else
    {
        rtptUtilsEvent(s->rtpt, "Rec", s, "opensession entry= %d port= %d address= \"%s\" scope= %d ",
            s->id, Port, IPstring, IPscope);
    }

    rtpSessionAddressPtr = testAddressConstruct(&rtpSessionAddress, IPstring, Port, IPscope);
	if (NULL == rtpSessionAddressPtr)
		return RV_ERROR_NULLPTR;

    sprintf(SDESname, "RTPT:%#x", (RvUint32)s);
    
	if ((s->hRTP = RvRtpOpenEx(rtpSessionAddressPtr, ssrcPattern, ssrcMask, SDESname)) == NULL)
        return RV_ERROR_UNKNOWN;

	s->hRTCP = RvRtpGetRTCPSession(s->hRTP);
    RvLockConstruct(pAppMngLog, &s->timeLock); 

	s->cb.appEvent = (RvRtcpAppEventHandler_CB)rtcpAppEventHandler;
	RvRtcpSetAppEventHandler(s->hRTCP, s->cb.appEvent, s);
	s->cb.byeEvent = (RvRtcpByeEventHandler_CB)rtcpByeEventHandler;
	RvRtcpSetByeEventHandler(s->hRTCP, s->cb.byeEvent, s);
	s->cb.rtcpEvent = (RvRtcpEventHandler_CB) rtcpEventHandler;
	RvRtcpSetRTCPRecvEventHandler(s->hRTCP, s->cb.rtcpEvent, s);
	s->cb.shutdownEvent = (RvRtpShutdownCompleted_CB) rtpShutdownCompletedEvent;
    RvRtpSetShutdownCompletedEventHandler(s->hRTP, s->hRTCP, s->cb.shutdownEvent, s);
    /* Default payload type set to g.711 PCMU */
	rtptTimestampInit(s, 0);
	s->cb.rtpsendercb  = rtpSenderPCMU;
	s->cb.rtpreadcb    = rtpReceivePCMUPacket;
	s->open = RV_TRUE;
    s->readStat = 0;

    if (seqNum != 0)
    {
        s->seqNum = seqNum;
        RvRtpUseSequenceNumber(s->hRTP);
#if defined(SRTP_ADD_ON)  
        RvRtpSetRtpSequenceNumber(s->hRTP, seqNum); /* for working with SRTP */
#endif
    }
#if defined(SRTP_ADD_ON)       
    memcpy(&(s->localsrtp.address), &rtpSessionAddress, sizeof(rtpSessionAddress));
#endif
	return RV_OK;
}

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
RvStatus rtptSessionStartRead(rtptSessionObj* s, RvBool bBlocked)
{
	RvStatus status = RV_OK;
	
	if (NULL==s||NULL == s->hRTP)
		return RV_ERROR_NULLPTR;

    rtptUtilsEvent(s->rtpt, "Rec", s, "startread entry= %d blocked= %d", s->id, bBlocked);

	if (bBlocked)
    {
		status = rtptStartRecvThread((struct RvRtpSessionObj__*)s);
        RvLogInfo(pAppRvLog, (pAppRvLog, "Started Receiving Thread for blocking reading for session %#x", (RvUint32)s->hRTP));
    }
	else
	{
		s->cb.rtpReadEvent = rtpEventHandler;
		RvRtpSetEventHandler(s->hRTP, rtpEventHandler, s);
	}
	return status;
}

/********************************************************************************************
 * function rtptSessionClose
 * purpose : closes session
 * input   : s - pointer to session object (rtptSessionObj) 
 * output  : nono
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionClose(rtptSessionObj* s)
{
	RvStatus            status = RV_OK;

	if (NULL==s||NULL==s->hRTP)
		return RV_ERROR_NULLPTR;

    rtptUtilsEvent(s->rtpt, "Rec", s, "closesession entry= %d", s->id);
    s->open = RV_FALSE;	
	/* wait until sender thread exits */
	if (s->sender.launched)
    {
        /*RvBool bResult = RV_FALSE;*/
        s->sender.launched = RV_FALSE;
        /*status = RvThreadJoin(&s->sender.thread, &bResult);*/
	    status = RvThreadDestruct(&s->sender.thread);
    }
	if ((status = RvRtpClose(s->hRTP))<0)
	{	
		return status;
	}
	s->hRTP = NULL;
	s->hRTCP = NULL;
	if (s->receiver.launched)
    {
        s->receiver.launched = RV_FALSE;
		status = RvThreadDestruct(&s->receiver.thread);
    }
	rtptSessionObjInit(s->rtpt, s, s->encryptors); /* sets zero to the s-pointed memory */
	return status;
}

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
RvStatus rtptSessionAddRemoteAddr(rtptSessionObj* s, RvUint16 Port, const RvChar *IPstring, RvUint32 IPscope)
{
	RvStatus status = RV_OK;
	RvNetAddress rtpSessionAddress;
	RvNetAddress* rtpSessionAddressPtr = &rtpSessionAddress;
	RvNetAddress rtcpSessionAddress;
	RvNetAddress* rtcpSessionAddressPtr = &rtcpSessionAddress;
	
	if (NULL==s||NULL == s->hRTP|| NULL == s->hRTCP)
		return RV_ERROR_NULLPTR;

    rtptUtilsEvent(s->rtpt, "Rec", s, "addraddr entry= %d port= %d address= \"%s\" scope= %d", s->id, Port, IPstring, (int)IPscope);

    rtpSessionAddressPtr =  testAddressConstruct(&rtpSessionAddress, IPstring, Port, IPscope);
    rtcpSessionAddressPtr = testAddressConstruct(&rtcpSessionAddress, IPstring, (RvUint16)(Port+1), IPscope);
	if (NULL == rtpSessionAddressPtr||NULL == rtcpSessionAddressPtr)
		return RV_ERROR_NULLPTR;
	
    RvRtpAddRemoteAddress(s->hRTP, rtpSessionAddressPtr);
    if (s->hRTCP!=NULL)
        RvRtcpAddRemoteAddress(s->hRTCP, rtcpSessionAddressPtr);

	status = rtptStartSendThread((struct RvRtpSessionObj__*)s);
    
	return status;
}
/********************************************************************************************
 * function rtptSessionSetPayload
 * purpose : sets payload related sending/receiving functions
 * input   : s - pointer to session object (rtptSessionObj) 
 *           payload - payload type
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSetPayload(rtptSessionObj* s, RvUint32 Payload)
{
	RvStatus status = RV_OK;
	
	if (NULL==s||NULL == s->hRTP|| NULL == s->hRTCP)
		return RV_ERROR_NULLPTR;

    rtptUtilsEvent(s->rtpt, "Rec", s, "setpayload entry= %d payload= %d", s->id, Payload);

    if (!s->open)
		return RV_ERROR_UNKNOWN;
	
    switch(Payload)
	{
	default:
	case 0:
		s->cb.rtpsendercb  = rtpSenderPCMU;
		s->cb.rtpreadcb    = rtpReceivePCMUPacket;
		break;
	}
	return status;
}
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
		IN rtptSessionObj* s, 
		IN const RvChar *ekeyData, 
		IN const RvChar *dkeyData)
{
    RvKey ekey, dkey;
	RvRtpEncryptionPlugIn* pluginPtr = rvRtpDesEncryptionGetPlugIn(&s->encryptors->desEncryptor);
	RvStatus status = RV_OK;

    rtptUtilsEvent(s->rtpt, "Rec", s, "encrypt entry= %d alg= DES ekey= %s dkey= %s", s->id, ekeyData, dkeyData);

	RvKeyConstruct(&ekey);
	RvKeyConstruct(&dkey);
	RvKeySetValue(&ekey, ekeyData, 56);
	RvKeySetValue(&dkey, dkeyData, 56);
	/* default value */
	status = RvRtpSetDoubleKeyEncryption(s->hRTP, RV_RTPENCRYPTIONMODE_RFC3550, &ekey, &dkey, pluginPtr);
	RvKeyDestruct(&ekey);
	RvKeyDestruct(&dkey);
	return status;
}
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
	IN rtptSessionObj* s, 
	IN const RvChar *ekeyData, 
	IN const RvChar *dkeyData)
{
    RvKey ekey, dkey;
	RvRtpEncryptionPlugIn* pluginPtr = rvRtp3DesEncryptionGetPlugIn(&s->encryptors->tripleDesEncryptor);
	RvStatus status = RV_OK;
	RvChar eKeyData[21];
	RvChar dKeyData[21];               

    rtptUtilsEvent(s->rtpt, "Rec", s, "encrypt entry= %d alg= 3DES ekey= %s dkey= %s", s->id, ekeyData, dkeyData);

	memset(eKeyData,0,21);
	memset(dKeyData,0,21);
    if (NULL!=ekeyData)
	    strncpy(eKeyData, ekeyData, 21);
    if (NULL!=dkeyData)
		strncpy(dKeyData, dkeyData, 21);
	RvKeyConstruct(&ekey);
	RvKeyConstruct(&dkey);
	RvKeySetValue(&ekey, eKeyData, 168);
	RvKeySetValue(&dkey, dKeyData, 168);
	/* default value */
	status = RvRtpSetDoubleKeyEncryption(s->hRTP, RV_RTPENCRYPTIONMODE_RFC3550, &ekey, &dkey, pluginPtr);
	RvKeyDestruct(&ekey);
	RvKeyDestruct(&dkey);
	return status;
}
/********************************************************************************************
 * function rtptSessionSendManualRTCPSR
 * purpose : sends Manual RTCP SR
 * input   : s - pointer to session object (rtptSessionObj) 
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSendManualRTCPSR(IN rtptSessionObj* s)
{
	if (NULL==s||NULL == s->hRTP|| NULL == s->hRTCP)
		return RV_ERROR_NULLPTR;

    rtptUtilsEvent(s->rtpt, "Rec", s, "sendsr entry= %d", s->id);

    if (!s->open)
		return RV_ERROR_UNKNOWN;
	
	return RvRtcpSessionSendSR(s->hRTCP, RV_TRUE, NULL, 0);
}
/********************************************************************************************
 * function rtptSessionSendManualRTCPRR
 * purpose : sends Manual RTCP RR
 * input   : s - pointer to session object (rtptSessionObj) 
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSendManualRTCPRR(IN rtptSessionObj* s)
{
	if (NULL==s||NULL == s->hRTP|| NULL == s->hRTCP)
		return RV_ERROR_NULLPTR;

    rtptUtilsEvent(s->rtpt, "Rec", s, "sendrr entry= %d", s->id);

    if (!s->open)
		return RV_ERROR_UNKNOWN;
	
	return RvRtcpSessionSendRR(s->hRTCP, RV_TRUE, NULL, 0);
}
/********************************************************************************************
 * function rtptSessionSendManualBYE
 * purpose : sends Manual RTCP immediate BYE
 * input   : s - pointer to session object (rtptSessionObj) 
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSendManualBYE(IN rtptSessionObj* s, const RvChar *reason)
{
	if (NULL==s||NULL == s->hRTP|| NULL == s->hRTCP)
		return RV_ERROR_NULLPTR;

    rtptUtilsEvent(s->rtpt, "Rec", s, "sendbye entry= %d reason= %s", s->id, reason);

    if (!s->open)
		return RV_ERROR_UNKNOWN;
	
	return  RvRtcpSessionSendBye(s->hRTCP, reason, strlen(reason)+1);
}
/********************************************************************************************
 * function rtptSessionSendShutdown
 * purpose : sends Manual RTCP BYE after apropriate delay defined in RFC 3550
 * input   : s - pointer to session object (rtptSessionObj) 
 *           reason - RTCP BYE reason
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSendShutdown(IN rtptSessionObj* s, const RvChar *reason)
{
	if (NULL==s||NULL == s->hRTP|| NULL == s->hRTCP)
		return RV_ERROR_NULLPTR;

    rtptUtilsEvent(s->rtpt, "Rec", s, "shutdown entry= %d reason= %s", s->id, reason);

    if (!s->open)
		return RV_ERROR_UNKNOWN;
	strncpy(s->reason, reason, RTPT_MAX_SHUTDOWN_REASON_LEN-1);
    s->reason[RTPT_MAX_SHUTDOWN_REASON_LEN-1] = '\0';
	return RvRtpSessionShutdown(s->hRTP, s->reason, (RvUint8)(strlen(s->reason)+1));
}

/********************************************************************************************
 * function rtptSessionSendRTCPAPP
 * purpose : sends RTCP APP message
 * input   : s - pointer to session object (rtptSessionObj) 
 * Note: with dummy user data  
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSendRTCPAPP(IN rtptSessionObj* s)
{
	RvUint32 DummyValue = 10000, Index = 0;
	static RvChar *FirstMesssage  = "RTCP APP First Message ";
	static RvChar *SecondMesssage = "RTCP APP Second Message";
	RvRtcpAppMessage appTable[3];

	if (NULL==s||NULL == s->hRTP|| NULL == s->hRTCP)
		return RV_ERROR_NULLPTR;

    rtptUtilsEvent(s->rtpt, "Rec", s, "sendapp entry= %d", s->id);
    
    if (!s->open)
		return RV_ERROR_UNKNOWN;
	
	appTable[Index].subtype = 0x0F;
	memcpy(appTable[Index].name, (RvInt8*)"1111", 4);
	appTable[Index].userData = (RvUint8*)FirstMesssage;
	appTable[Index].userDataLength = strlen(FirstMesssage)+1; /* with eol */
	Index++;
	appTable[Index].subtype = 0x0F;
	memcpy(appTable[Index].name, (RvInt8*)"2222", 4);
	appTable[Index].userData = (RvUint8*)SecondMesssage;
	appTable[Index].userDataLength = strlen(SecondMesssage)+1; /* with eol */
	Index++;
	
	appTable[Index].subtype = 0x0F;
	memcpy(appTable[Index].name, (RvInt8*)"3333", 4);
	appTable[Index].userData = (RvUint8*)&DummyValue;
	appTable[Index].userDataLength = 4;
	RvLogInfo(pAppRvLog, (pAppRvLog,"Sending 3 Manual RTCP APP messages."));		
	return RvRtcpSessionSendApps(s->hRTCP, appTable,   Index+1, RV_TRUE);
}
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
RvStatus rtptSessionListenMulticast(rtptSessionObj* s, 
								  RvUint16 Port, 
								  const RvChar *IPstring, 
								  RvUint32 IPscope)
{
	RvStatus status = RV_OK;
	RvNetAddress multicastRtpAddress;
	RvNetAddress* multicastRtpAddressPtr = &multicastRtpAddress;	
	RvNetAddress multicastRtcpAddress;
	RvNetAddress* multicastRtcpAddressPtr = &multicastRtcpAddress;

	if (NULL==s||NULL==s->hRTP)
		return RV_ERROR_NULLPTR;

    rtptUtilsEvent(s->rtpt, "Rec", s, "listenmulticast entry= %d port= %d address= %s scope= %d ",
        s->id, Port, IPstring, IPscope);

    multicastRtpAddressPtr = testAddressConstruct(&multicastRtpAddress, IPstring, Port, IPscope);
	if (NULL == multicastRtpAddressPtr)
		return RV_ERROR_NULLPTR;
	status = RvRtpSetGroupAddress(s->hRTP, multicastRtpAddressPtr);
	
	multicastRtcpAddressPtr = testAddressConstruct(&multicastRtcpAddress, IPstring, (RvUint16)(Port+1), IPscope);
	if (NULL!=s->hRTCP&&multicastRtcpAddressPtr!=NULL)
	    status = RvRtcpSetGroupAddress(s->hRTCP, multicastRtpAddressPtr);
	return status;
}
/********************************************************************************************
 * function rtptSessionSetRtpMulticastTTL
 * purpose : sets multicast TTL for RTP
 * input   : s - pointer to session object (rtptSessionObj) 
 *           RTPttl - RTP  time to live
 *           RTCPttl - RTCP time to live
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSetRtpMulticastTTL(rtptSessionObj* s, RvInt RTPttl)
{
	RvStatus status = RV_OK;
	RvUint8 rtpttl = (RvUint8) RTPttl;
	if (NULL==s||NULL==s->hRTP)
		return RV_ERROR_NULLPTR;

    rtptUtilsEvent(s->rtpt, "Rec", s, "multicastttl entry= %d rtpttl= %d", s->id, RTPttl);

	status = RvRtpSetMulticastTTL(s->hRTP, rtpttl);
	return status;
}
/********************************************************************************************
 * function rtptSessionSetRtcpMulticastTTL
 * purpose : jsets multicast TTL for  RTCP
 * input   : s - pointer to session object (rtptSessionObj) 
 *           RTPttl - RTP  time to live
 *           RTCPttl - RTCP time to live
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSetRtcpMulticastTTL(rtptSessionObj* s,  RvInt RTCPttl)
{
	RvStatus status = RV_OK;
	RvUint8 rtcpttl = (RvUint8) RTCPttl;
	if (NULL==s||NULL==s->hRTCP)
		return RV_ERROR_NULLPTR;

    rtptUtilsEvent(s->rtpt, "Rec", s, "multicastttl entry= %d rtcpttl= %d", s->id, RTCPttl);

    status = RvRtcpSetMulticastTTL(s->hRTCP, rtcpttl);
	return status;
}
/********************************************************************************************
 * function rtptSessionSetRtpTOS
 * purpose : sets TOS for RTP
 * input   : s - pointer to session object (rtptSessionObj) 
 *           RTPtos - RTP  ToS
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSetRtpTOS(rtptSessionObj* s, RvInt RTPtos)
{
	RvStatus status = RV_OK;
	if (NULL==s||NULL==s->hRTP)
		return RV_ERROR_NULLPTR;

    rtptUtilsEvent(s->rtpt, "Rec", s, "settos entry= %d rtptos= %d", s->id, RTPtos);

	status = RvRtpSetTypeOfService(s->hRTP, RTPtos);
	return status;
}
/********************************************************************************************
 * function rtptSessionSetRtcpTOS
 * purpose : sets TOS for RTP
 * input   : s - pointer to session object (rtptSessionObj) 
 *           RTCPtos - RTCP  ToS
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSetRtcpTOS(rtptSessionObj* s, RvInt RTCPtos)
{
	RvStatus status = RV_OK;
	if (NULL==s||NULL==s->hRTCP)
		return RV_ERROR_NULLPTR;

    rtptUtilsEvent(s->rtpt, "Rec", s, "settos entry= %d rtcptos= %d", s->id, RTCPtos);

	status = RvRtcpSetTypeOfService(s->hRTCP, RTCPtos);
	return status;
}
/********************************************************************************************
 * function rtptSessionSetRtcpBandwith
 * purpose : sets RTCP bandwith (usially 5% of RTP bandwith)
 * input   : s - pointer to session object (rtptSessionObj) 
 *           RTCPband - RTCP bandwith in bytes per second
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSetRtcpBandwith(rtptSessionObj* s, RvInt RTCPband)
{
	RvStatus status = RV_OK;
	if (NULL==s||NULL==s->hRTCP)
		return RV_ERROR_NULLPTR;

    rtptUtilsEvent(s->rtpt, "Rec", s, "rtcpband entry= %d band= %d", s->id, RTCPband);

    RvRtcpSetBandwidth(s->hRTCP, RTCPband);
	return status;
}
/********************************************************************************************
 * function rtptSessionRemoveRemoteAddr
 * purpose : removes session sending remote address
 * input   : s - pointer to session object (rtptSessionObj) 
 *           Port - port
 *           IP address to be removed
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionRemoveRemoteAddr(rtptSessionObj* s, RvUint16 Port, const RvChar *IPstring)
{
	RvStatus status = RV_OK;
	RvNetAddress rtpSessionAddress;
	RvNetAddress* rtpSessionAddressPtr = &rtpSessionAddress;
	RvNetAddress rtcpSessionAddress;
	RvNetAddress* rtcpSessionAddressPtr = &rtcpSessionAddress;
	
	if (NULL==s||NULL == s->hRTP|| NULL == s->hRTCP)
		return RV_ERROR_NULLPTR;

    rtptUtilsEvent(s->rtpt, "Rec", s, "removeraddr entry= %d port= %d address= %s", s->id, Port, IPstring);

    rtpSessionAddressPtr =  testAddressConstruct(&rtpSessionAddress, IPstring, Port, 0);
    rtcpSessionAddressPtr = testAddressConstruct(&rtcpSessionAddress, IPstring, (RvUint16)(Port+1), 0);
	if (NULL == rtpSessionAddressPtr||NULL == rtcpSessionAddressPtr)
		return RV_ERROR_NULLPTR;
	
    RvRtpRemoveRemoteAddress(s->hRTP, rtpSessionAddressPtr);
    if (s->hRTCP!=NULL)
        RvRtcpRemoveRemoteAddress(s->hRTCP, rtcpSessionAddressPtr);
	/* it does not close the sending thread */
	return status;
}

/********************************************************************************************
 * function rtptSessionRemoveAllRemoteAddresses
 * purpose : removes all session remote addresses
 * input   : s - pointer to session object (rtptSessionObj) 
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/

RvStatus rtptSessionRemoveAllRemoteAddresses(rtptSessionObj* s)
{
	RvStatus status = RV_OK;
	
	if (NULL==s)
		return RV_ERROR_NULLPTR;

    rtptUtilsEvent(s->rtpt, "Rec", s, "removeallraddr entry= %d", s->id);

	if (NULL != s->hRTP)
	{
        RvRtpRemoveAllRemoteAddresses(s->hRTP);
		s->open = RV_FALSE;	
		if (s->sender.launched)
		{/*  closes the sending thread */
            s->sender.launched = RV_FALSE;
            status = RvThreadDestruct(&s->sender.thread);
		}
		s->open = RV_TRUE;
	}
    if (s->hRTCP!=NULL)
        RvRtcpRemoveAllRemoteAddresses(s->hRTCP);
	
	return status;
}

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
    IN const RvChar *buf)
{
#if (RV_64TOASCII_TYPE == RV_64TOASCII_MANUAL)
    RvChar *cptr, tmpbuf[RV_64TOASCII_BUFSIZE];
    RvUint64 temp64 = RvUint64Const(0, 0);
#endif

#if (RV_CHECK_MASK & RV_CHECK_NULL)
    if(buf == NULL||num64 == NULL) return NULL;
#endif

#if (RV_64TOASCII_TYPE == RV_64TOASCII_STANDARD)
    RvSscanf(buf, "%llu", num64);
#endif
#if (RV_64TOASCII_TYPE == RV_64TOASCII_WIN32)
    RvSscanf(buf, "%I64u", num64);
#endif
#if (RV_64TOASCII_TYPE == RV_64TOASCII_MANUAL)
    /* sprintf is broken, have to do it manually */
    cptr = buf;
    while(*cptr!='\0')
    {
        temp64 = RvUint64Mul(temp64, RvUint64Const(0,10));
        if (*cptr<='9' &&*cptr>='0')
            temp64 = RvUint64Add(temp64, RvUint64Const(0, *cptr - '0'));
        else
            return NULL;
        cptr++;
    } 
    *num64 = temp64;
#endif
    return num64;
}


#ifdef __cplusplus
}
#endif

