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
#include "rvcbase.h"
#include "rvstdio.h"
#include "rvlock.h"
#include "rvselect.h"
#include "rvsocket.h"
#include "rvmemory.h"
#include "rvclock.h"
#include "rvntptime.h"
#include "rvtime.h"
#include "rvtimer.h"
#include "rvtimestamp.h"
#include "rvrandomgenerator.h"
#include "rvhost.h"
#include <string.h>
#include "rvrtpbuffer.h"
#include "bitfield.h"
#include "rtputil.h"
#include "rtcp.h"
#include "RtcpTypes.h"
#include "rvrtpencryptionplugin.h"
#include "rvrtpencryptionkeyplugin.h"
#include "rvrtpheader.h"
#include "payload.h"
//#include "srtpaux.h"

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)

#include <net/if.h>
#include <arpa/inet.h>

#include "rvexternal.h"

#include "rtp_spirent.h"

#endif
//SPIRENT_END

#if(RV_LOGMASK != RV_LOGLEVEL_NONE)   
static  RvLogSource*     localLogSource = NULL;
#define RTP_SOURCE      (localLogSource!=NULL?localLogSource:(localLogSource=rtpGetSource(RVRTP_RTCP_MODULE)))
#define rvLogPtr        (localLogSource!=NULL?localLogSource:(localLogSource=rtpGetSource(RVRTP_RTCP_MODULE)))
static  RvRtpLogger      rtpLogManager = NULL;
#define logMgr          (RvRtpGetLogManager(&rtpLogManager),((RvLogMgr*)rtpLogManager))
#else
#define logMgr          (NULL)
#define rvLogPtr        (NULL)
#endif
#include "rtpLogFuncs.h"
#undef FUNC_NAME
#define FUNC_NAME(name) "RvRtcp" #name

#ifdef __cplusplus
extern "C" {
#endif




#define RTCP_BYE_REASON_LENGTH    (255)

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
#define MAXIPS                    RV_RTCP_MAXIPS
#else
#define MAXIPS                    20
#endif
//SPIRENT_END

#define MAX_DROPOUT               3000
#define MAX_MISORDER              100
#define MIN_SEQUENTIAL            2
#define RTP_SEQ_MOD               0x10000

#define ALIGNMENT                 0x10

/* RTCP header bit locations - see the standard */
#define HEADER_V                  30      /* version                       */
#define HEADER_P                  29      /* padding                       */
#define HEADER_RC                 24      /* reception report count        */
#define HEADER_PT                 16      /* packet type                   */
#define HEADER_len                0       /* packet length in 32-bit words */

/* RTCP header bit field lengths - see the standard */
#define HDR_LEN_V                 2       /* version                       */
#define HDR_LEN_P                 1       /* padding                       */
#define HDR_LEN_RC                5       /* reception report count        */
#define HDR_LEN_PT                8       /* packet type                   */
#define HDR_LEN_len               16      /* packet length in 32-bit words */


/* used to overcome byte-alignment issues */
#define SIZEOF_RTCPHEADER         (sizeof(RvUint32) * 2)
#define RTCPHEADER_SSRCMAXCOUNT   RvInt32Const(31)
#define RTCP_BYEBACKOFFMINIMUM    RvInt32Const(50)
#define RTCP_PACKETOVERHEAD       RvUint32Const(28)   /* UDP */
#define RTCP_DEFAULTPACKETSIZE    RvUint32Const(100)  /* Default RTCP packet size for TIMER initialization */

#define SIZEOF_SR                 (sizeof(RvUint32) * 5)
#define SIZEOF_RR                 (sizeof(RvUint32) * 6)

#define SIZEOF_SDES(sdes)         (((sdes).length + 6) & 0xfc)

/* initial bit field value for RTCP headers: V=2,P=0,RC=0,PT=0,len=0 */
#define RTCP_HEADER_INIT          0x80000000
#define reduceNNTP(a) (RvUint64ToRvUint32(RvUint64And(RvUint64ShiftRight(a, 16), RvUint64Const(0, 0xffffffff))))
    
#define W32Len(l)  ((l + 3) / 4)  /* length in 32-bit words */
    
/* RTCP instance to use */
RvRtcpInstance rvRtcpInstance;

/* local functions */
static RvUint32 rtcpGetEstimatedRTPTime(
        IN RvRtcpSession  hRTCP,
        const RvUint64 *ntpNewTimestampPtr);

//SPIRENT_BEGIN
#if !defined(UPDATED_BY_SPIRENT)
static RvUint64 getNNTPTime(void);
#endif
//SPIRENT_END

static RvBool rtcpTimerCallback(IN void* key);
static void   setSDES(RvRtcpSDesType type, rtcpSDES* sdes, RvUint8 *data,
                      RvInt32 length);
static void   init_seq  (rtpSource *s, RvUint16 seq);
static RvInt32    update_seq(rtpSource *s, RvUint16 seq, RvUint32 ts, RvUint32 arrival);

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
static RvUint32 getReceivedPackets(rtpSource *s);
#endif
//SPIRENT_END
static RvUint32 getLost    (rtpSource *s);
static RvUint32 getJitter  (rtpSource *s);
static RvUint32 getSequence(rtpSource *s);
/*h.e 30.05.01*/
static RvUint32 getSSRCfrom(RvUint8 *);
/*===*/

static rtcpHeader makeHeader(RvUint32 ssrc, RvUint8 count, rtcpType type,
                             RvUint16 dataLen);

static rtcpInfo * findSSrc(rtcpSession *,RvUint32);

static rtcpInfo *insertNewSSRC(rtcpSession *s, RvUint32 ssrc);

static void rtcpEventCallback(
        IN RvSelectEngine*  selectEngine,
        IN RvSelectFd*      fd,
        IN RvSelectEvents   selectEvent,
        IN RvBool           error);

static RvStatus rtcpSetTimer(
        IN rtcpSession* s,
        IN RvInt64      delay);


static RvStatus rtcpAddRTCPPacketType(IN RvRtcpSession  hRTCP,
                                      IN rtcpType type,
                                      IN RvUint8 subtype,
                                      IN RvUint8* name,
                                      IN void *userData, /* application-dependent data  */
                                      IN RvUint32 userDataLength,
                                      IN OUT  RvRtpBuffer*  buf,
                                      IN OUT RvUint32* pCurrentIndex);

/* this function send encrypted raw data for RTCP session */
static RvStatus rtcpSessionRawSend(
    IN    RvRtcpSession hRTCP,
    IN    RvUint8*      bufPtr,
    IN    RvInt32       DataLen,
    IN    RvInt32       DataCapacity,
    INOUT RvUint32*     sentLenPtr);

static RvUint32 rtcpGetTimeInterval(IN rtcpSession* s);

RVAPI
RvInt32 RVCALLCONV rtcpProcessCompoundRTCPPacket(
        IN      RvRtcpSession  hRTCP,
        IN OUT  RvRtpBuffer*  buf,
        IN      RvUint64      myTime);


RVAPI
RvInt32 RVCALLCONV rtcpProcessRTCPPacket(
        IN  rtcpSession *  s,
        IN  RvUint8 *      data,
        IN  RvInt32        dataLen,
        IN  rtcpType       type,
        IN  RvInt32        reportCount,
        IN  RvUint64       myTime);

/* == Basic RTCP Functions == */

/*=========================================================================**
**  == RvRtcpInit() ==                                                       **
**                                                                         **
**  Initializes the RTCP module.                                           **
**                                                                         **
**  RETURNS:                                                               **
**      A non-negative value upon success, or a negative integer error     **
**      code.                                                              **
**                                                                         **
**=========================================================================*/

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)

RVAPI RvStatus RVCALLCONV RvRtcpSeliSelectUntilPrivate(IN RvUint32 delay)
{
    RvSelectEngine* selectEngine = rvRtcpInstance.selectEngine;
    RvStatus status = 0;
    RvUint64 timeout = RvUint64Mul(RvUint64FromRvUint32(delay), RV_TIME64_NSECPERMSEC);

    if(selectEngine) {
      status = RvSelectWaitAndBlock(selectEngine, timeout);
    }

    return status;
}

#endif
//SPIRENT_END

RVAPI
RvInt32 RVCALLCONV RvRtcpInit(void)
    {
    RvStatus status;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpInit"));
    status = RvCBaseInit();
    if (status != RV_OK)
        return status;

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    status = RvSelectConstruct(SPIRENT_RTP_ENGINE_SIZE*2, SPIRENT_RTP_ENGINE_SIZE, logMgr, &rvRtcpInstance.selectEngine);
#else
    status = RvSelectConstruct(2048, 1024, logMgr, &rvRtcpInstance.selectEngine);
#endif
//SPIRENT_END
    if (status != RV_OK)
    {
        RvCBaseEnd();
        return ERR_RTCP_GENERALERROR;
    }

    /* Find the pool of timers to use */
    status = RvSelectGetTimeoutInfo(rvRtcpInstance.selectEngine, NULL, &rvRtcpInstance.timersQueue);
    if (status != RV_OK)
    {
      //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
      RvSelectDestruct(rvRtcpInstance.selectEngine, SPIRENT_RTP_ENGINE_SIZE);
      rvRtcpInstance.selectEngine=NULL;
#else
      RvSelectDestruct(rvRtcpInstance.selectEngine, 1024);
#endif
      //SPIRENT_END
      RvCBaseEnd();
      return ERR_RTCP_GENERALERROR;
    }

    if (rvRtcpInstance.timesInitialized == 0)
    {
       //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
           unsigned int i=0;
           RvNetAddress **ips=RTP_getHostAddresses();

           RvUint32 numAddrs=0;
           
           for (i=0; ((i<RV_RTP_MAXIPS) && ips[i]); i++) {
              rvRtcpInstance.hostIPs[numAddrs++] = *((RvAddress*)(ips[i]->address));
           }

           if(numAddrs>0) status=RV_OK;
           else status=-1;
        
#else
        RvUint32 numAddrs = RV_RTCP_MAXIPS;
        /* Find the list of host addresses we have */
        status = RvHostLocalGetAddress(logMgr, &numAddrs, rvRtcpInstance.hostIPs);
#endif
//SPIRENT_END

        if (status != RV_OK)
        {
          //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
          RvSelectDestruct(rvRtcpInstance.selectEngine, SPIRENT_RTP_ENGINE_SIZE);
          rvRtcpInstance.selectEngine=NULL;
#else
          RvSelectDestruct(rvRtcpInstance.selectEngine, 1024);
#endif
          //SPIRENT_END
          RvCBaseEnd();
          return ERR_RTCP_GENERALERROR;
        }
        rvRtcpInstance.addresesNum = numAddrs;
        if (numAddrs>0)
            RvAddressCopy(&rvRtcpInstance.hostIPs[0], &rvRtcpInstance.localAddress);
        else
#if (RV_NET_TYPE & RV_NET_IPV6)
            RvAddressConstruct(RV_ADDRESS_TYPE_IPV6, &rvRtcpInstance.localAddress);
#else
            RvAddressConstruct(RV_ADDRESS_TYPE_IPV4, &rvRtcpInstance.localAddress);
#endif
        /* Create a random generator */
        RvRandomGeneratorConstruct(&rvRtcpInstance.randomGenerator,
            (RvRandom)(RvTimestampGet(logMgr)>>8));
    }

    rvRtcpInstance.timesInitialized++;
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpInit"));
    return RV_OK;
}

/************************************************************************************
 * RvRtcpInitEx
 * description: Initializes the RTCP Stack and specifies the local IP address to which all
 *              RTCP sessions will be bound.
 * input: pRtpAddress - pointer to RvNetAddress which contains
 *         the local IPV4/6 address to which all RTCP sessions will be bound.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 * Remarks
 * - This function can be used instead of RvRtcpInit().
 * - RvRtcpInit() binds to the ?any? IP address.
 ***********************************************************************************/
#ifdef RVRTP_OLD_CONVENTION_API
RVAPI
RvInt32 RVCALLCONV rtcpInitEx(RvUint32 ip)
{
    RvInt32 rc;
    RvLogEnter(rvLogPtr, (rvLogPtr, "rtcpInitEx"));
    if ((rc=RvRtcpInit()) >= 0)
    {
        RvAddressConstructIpv4(&rvRtcpInstance.localAddress, ip, 0);
    }
    RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpInitEx"));
    return rc;
}
#endif
RVAPI
RvInt32 RVCALLCONV RvRtcpInitEx(IN RvNetAddress* pRtpAddress)
{
    RvInt32 rc;
    RvAddress* pRvAddress = NULL;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpInitEx"));
    if ((rc=RvRtcpInit()) >= 0)
    {
        if (NULL!=pRtpAddress&&RvNetGetAddressType(pRtpAddress)!=RVNET_ADDRESS_NONE)
        {
            pRvAddress = (RvAddress*) pRtpAddress->address;
            RvAddressCopy(pRvAddress, &rvRtcpInstance.localAddress);
        }
        else
            RvAddressConstruct(RV_ADDRESS_TYPE_IPV4, &rvRtcpInstance.localAddress);
    }
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpInitEx"));
    return rc;
}

/*=========================================================================**
**  == RvRtcpEnd() ==                                                        **
**                                                                         **
**  Shuts down the RTCP module.                                            **
**                                                                         **
**  RETURNS:                                                               **
**      A non-negative value upon success, or a negative integer error     **
**      code.                                                              **
**                                                                         **
**=========================================================================*/

RVAPI
RvInt32 RVCALLCONV RvRtcpEnd(void)
{
    RvStatus res = RV_OK;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpEnd"));
	if (rvRtcpInstance.timesInitialized>0)
	{
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    res = RvSelectDestruct(rvRtcpInstance.selectEngine, SPIRENT_RTP_ENGINE_SIZE);
    rvRtcpInstance.selectEngine=NULL;
#else
    res = RvSelectDestruct(rvRtcpInstance.selectEngine, 1024);
#endif
//SPIRENT_END

		RvRandomGeneratorDestruct(&rvRtcpInstance.randomGenerator);
		rvRtcpInstance.timesInitialized--;
	}
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpEnd"));
    return res;
}


/*******************************************************************************************
 * RvRtcpSetRTCPRecvEventHandler
 * description: Sets an event handler for the RTCP session. 
 * input: 
 *   hRTCP          - Handle of the RTCP session. 
 *   rtcpCallback   - Pointer to the callback function that is called each time 
 *                    a new RCTP packet arrives at the RCTP session. 
 *                    If eventHandler is set to NULL, the event handler is removed. 
 *                    The prototype of the callback is as follows: 
 *  
 *	                  void RVCALLCONV RvRtcpEventHandler_CB(
 *		                   IN RvRtcpSession,
 *		                   IN void * context,
 *		                   IN RvUint32 ssrc);
 *
 *	                  where: 		   
 *			                hRTCP is the handle of the RTCP session.
 *			                context is the same context passed by the application when calling 
 *                           RvRtcpSetRTCPRecvEventHandler().
 *			                ssrc is the synchronization source from which the packet was received.
 *	context - An application handle that identifies the particular RTCP session. 
 *            The application passes the handle to the event handler. The RTCP Stack 
 *            does not modify the parameter but simply passes it back to the application. 
 *			 			   
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 * Remarks 
 *  The application can set an event handler for each RTCP session. 
 *  The event handler will be called whenever an RTCP packet is received for this session. 
 ********************************************************************************************/

RVAPI
RvInt32 RVCALLCONV RvRtcpSetRTCPRecvEventHandler(
    IN RvRtcpSession         hRTCP,
    IN RvRtcpEventHandler_CB   rtcpCallback,
    IN void *               context)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT) 
    if(!s) return RV_Failure;
#endif
//SPIRENT_END
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSetRTCPRecvEventHandler"));
    s->rtcpRecvCallback=rtcpCallback;
    s->haRtcp=context; /*context is Event to inform Ring3 about RTCP arrival*/
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetRTCPRecvEventHandler"));
    return RV_OK;
}


//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT) 
RVAPI
RvInt32 RVCALLCONV RvRtcpSetRTCPSendEventHandler(
    IN RvRtcpSession         hRTCP,
    IN RvRtcpEventHandler_CB   rtcpCallback,
    IN void *               context)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    if(!s) return RV_Failure;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSetRTCPSendEventHandler"));
    s->rtcpSendCallback=rtcpCallback;
    s->haRtcpSend=context;
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetRTCPSendEventHandler"));
    return RV_OK;
}
#endif
//SPIRENT_END

/************************************************************************************
 * RvRtcpGetAllocationSize
 * description: Returns the number of bytes required for internal RTCP session data structure.
 *              The application may allocate the requested amount of memory and use it with
 *              the RvRtcpOpenFrom() function.
 * input: sessionMembers        - Maximum number of members in RTP conference.
 *
 * output: none.
 * Return Values - The function returns the number of bytes required for
 *                 internal RTCP session data structure.
 ***********************************************************************************/

RVAPI
RvInt32 RVCALLCONV RvRtcpGetAllocationSize(
    IN  RvInt32 sessionMembers)
{
    return sizeof(rtcpSession) + ALIGNMENT + sizeof(rtcpInfo) * sessionMembers;
}


/************************************************************************************
 * rtcpSetLocalAddress
 * description: Set the local address to use for calls to rtcpOpenXXX functions.
 *              This parameter overrides the value given in RvRtcpInitEx() for all
 *              subsequent calls.
 * input: ip    - Local IP address to use
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/

#ifdef RVRTP_OLD_CONVENTION_API
RVAPI
RvInt32 RVCALLCONV rtcpSetLocalAddress(IN RvUint32 ip)
{
    RvAddressConstructIpv4(&rvRtcpInstance.localAddress, ip, 0);
    return RV_OK;
}
#endif

RVAPI
RvInt32 RVCALLCONV RvRtcpSetLocalAddress(IN RvNetAddress* pRtpAddress)
{
    RvAddress*  pRvAddress = NULL;
    if  (NULL!=pRtpAddress)
    {
        pRvAddress = (RvAddress*) pRtpAddress->address;
        RvAddressCopy(pRvAddress, &rvRtcpInstance.localAddress);
    }
    return RV_OK;
}


/************************************************************************************
 * RvRtcpOpenFrom
 * description: Opens an RTCP session in the memory that the application allocated.
 * input: ssrc        - Synchronization source value for the RTCP session.
 *        pRtcpAddress contains the IP address (UDP port number) to be used for the RTCP session.
 *        cname       - Unique name representing the source of the RTP data.
 *                      Text that identifies the session. Must not be NULL.
 *        maxSessionMembers - Maximum number of different SSRC that can be handled
 *        buffer      - Application allocated buffer with a value no less than the
 *                      value returned by the function RvRtpGetAllocationSize().
 *        bufferSize  - size of the buffer.
 * output: none.
 * return value: If no error occurs, the function returns the handle for the opened RTP
 *               session. Otherwise, it returns NULL.
 * Remarks
 *  - Before calling RvRtpOpenFrom() the application must call RvRtpGetAllocationSize()
 *    in order to get the size of the memory allocated by the application.
 *  - The RTP port(in pRtcpAddress) should be an even number. The port for an RTCP session
 *    is always RTP port + 1.
 *  - If the port parameter is equal to zero in the RvRtpOpenFrom() call, then an arbitrary
 *    port number will be used for the new RTP session. Using a zero port number may cause
 *    problems with some codecs that anticipate the RTP and RTCP ports to be near each other.
 *    We recommend defining a port number and not using zero.
 *  - RvRtpOpenFrom() generates the synchronization source value as follows:
 *  - ssrc = ((Random 32 bit number) AND NOT ssrcMask)) OR ssrcPattern
 ***********************************************************************************/
#ifdef RVRTP_OLD_CONVENTION_API
RVAPI
RvRtcpSession RVCALLCONV rtcpOpenFrom(
        IN  RvUint32    ssrc,
        IN  RvUint16    port,
        IN  char *      cname,
        IN  RvInt32         maxSessionMembers,
        IN  void *      buffer,
        IN  RvInt32         bufferSize)
{
  RvNetAddress rtcpAddress;
  RvNetIpv4 Ipv4;
  Ipv4.port=port;
  Ipv4.ip=0;
  RvNetCreateIpv4(&rtcpAddress, &Ipv4);
  return RvRtcpOpenFrom(ssrc, &rtcpAddress, cname, maxSessionMembers, buffer, bufferSize);
}
#endif

RvUint32 rtcpInitTransmittionIntervalMechanism(rtcpSession* s)
{
    RvInt64 currentTime;
    RvUint32 delay;
    RvLogEnter(rvLogPtr, (rvLogPtr, "rtcpInitTransmittionIntervalMechanism"));

    s->txInterval.initialized   = RV_TRUE;
    s->txInterval.lastRTCPPacketSize = s->txInterval.aveRTCPPacketSize
        = RTCP_DEFAULTPACKETSIZE + RTCP_PACKETOVERHEAD;
    s->txInterval.pmembers = 1;
    s->txInterval.members  = 1;
    s->txInterval.senders  = 0;
    currentTime = RvTimestampGet(logMgr);
    delay = rtcpGetTimeInterval(s); /* in ms */
    s->txInterval.previousTime = currentTime;
    s->txInterval.nextTime = RvInt64Add(s->txInterval.previousTime,
                 RvInt64Mul(RvInt64FromRvUint32(delay), RV_TIME64_NSECPERMSEC));

    RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpInitTransmittionIntervalMechanism"));
    return delay;
}

RVAPI
RvRtcpSession RVCALLCONV RvRtcpOpenFrom(
                                      IN  RvUint32    ssrc,
                                      IN  RvNetAddress* pRtcpAddress,
                                      IN  char *      cname,
                                      IN  RvInt32         maxSessionMembers,
                                      IN  void *      buffer,
                                      IN  RvInt32         bufferSize)
{
    rtcpSession* s=(rtcpSession*)buffer;
    RvSize_t allocSize=RvRtcpGetAllocationSize(maxSessionMembers);
    RvInt32 participantsArrayOfs;
    RvAddress localAddress;
    RvStatus status;


    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpOpenFrom"));

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(!s) return NULL;
#endif
//SPIRENT_END

    if ((cname  == NULL) || (strlen(cname) > RTCP_MAXSDES_LENGTH) || ((RvInt32)allocSize > bufferSize) ||
        (NULL==pRtcpAddress)||RvNetGetAddressType(pRtcpAddress)==RVNET_ADDRESS_NONE)
    {
        RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpOpenFrom:wrong parameter"));
        RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpOpenFrom"));
        return NULL;
    }
    memset(buffer, 0, allocSize);
    if (RvNetGetAddressType(pRtcpAddress)==RVNET_ADDRESS_IPV6)
    {
#if (RV_NET_TYPE & RV_NET_IPV6)
        RvNetIpv6 Ipv6;
        RvNetGetIpv6(&Ipv6, pRtcpAddress);
        /*    RvAddressCopy(&rvRtcpInstance.localAddress , &localAddress);*/
        //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
        RvAddressConstructIpv6(&localAddress, Ipv6.ip, Ipv6.port, Ipv6.scopeId, Ipv6.flowinfo);
#else
        RvAddressConstructIpv6(&localAddress, Ipv6.ip, Ipv6.port, Ipv6.scopeId);
#endif
        //SPIRENT_END
        /* Open and bind the socket */
        status = RvSocketConstruct(RV_ADDRESS_TYPE_IPV6, RvSocketProtocolUdp, logMgr, &s->socket);
#else
        RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpOpenFrom: IPV6 is not supported in current configuration"));
        RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpOpenFrom"));
        return NULL;
#endif
    }
    else
    {
        RvNetIpv4 Ipv4;
        RvNetGetIpv4(&Ipv4, pRtcpAddress);
        RvAddressConstructIpv4(&localAddress, Ipv4.ip, Ipv4.port);
        /* Open and bind the socket */
        status = RvSocketConstruct(RV_ADDRESS_TYPE_IPV4, RvSocketProtocolUdp, logMgr, &s->socket);
    }
	RvLogDebug(rvLogPtr, (rvLogPtr, "RvRtcpOpenFrom: RTCP session opening"));
	
    s->sessionMembers = 1; /* my SSRC added to participant array */
    s->maxSessionMembers = maxSessionMembers;
    s->rtcpRecvCallback = NULL;
    s->isAllocated = RV_FALSE;
    s->isShutdown  = RV_FALSE;

    participantsArrayOfs = RvRoundToSize(sizeof(rtcpSession), ALIGNMENT);
    s->participantsArray = (rtcpInfo *) ((char *)s + participantsArrayOfs);
    s->participantsArray[0].ssrc = s->myInfo.ssrc = ssrc;
/*  s->rtpShutdownCompletedCallback  = NULL;
    s->rtcpByeRecvCallback           = NULL;
    s->rtcpAppRecvCallback           = NULL;
    s->encryptionPlugInPtr           = NULL;
    s->encryptionKeyPlugInPtr        = NULL;*/ /* all callbacks are inited to NULL by memset command */
    s->txInterval.rtcpBandwidth = RV_RTCP_BANDWIDTH_DEFAULT;  /* an arbitrary default value */

    if (status == RV_OK)
    {
/* SPIRENT_BEGIN */
/*
 * COMMENTS: Socket HW binding to Ethernet Interface
 */
#if defined(UPDATED_BY_SPIRENT)
        if (status == RV_OK)
#if defined(UPDATED_BY_SPIRENT_ABACUS)
            status = RvSocketSetBuffers(&s->socket, 16384, 16384, logMgr);
#else
            status = RvSocketSetBuffers(&s->socket, 4096, 4096, logMgr);
#endif
        if (status == RV_OK)
           status = RvSocketSetBroadcast(&s->socket, RV_TRUE, logMgr);
        if (status == RV_OK)
            status = RvSocketSetBlocking(&s->socket, RV_TRUE, logMgr);
        if (status == RV_OK)
            Spirent_RvSocketBindDevice(&s->socket, pRtcpAddress->if_name, logMgr);

#else
        RvSocketSetBuffers(&s->socket, 8192, 8192, logMgr);

            status = RvSocketSetBroadcast(&s->socket, RV_TRUE, logMgr);
        if (status == RV_OK)
            status = RvSocketSetBlocking(&s->socket, RV_TRUE, logMgr);
#endif  //#if defined(UPDATED_BY_SPIRENT)
/* SPIRENT_END */
        if (status == RV_OK)
            status = RvSocketBind(&s->socket, &localAddress, NULL, logMgr);
        if (status != RV_OK)
            RvSocketDestruct(&s->socket, RV_FALSE, NULL, logMgr);
    }

    RvAddressDestruct(&localAddress);

    if (status != RV_OK)
	{
        RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpOpenFrom: failed to open RTCP session"));
        RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpOpenFrom"));
        return NULL;
	}
    /* Initialize lock in supplied buffer */
    if (RvLockConstruct(logMgr, &s->lock) != RV_OK)
    {
        RvSocketDestruct(&s->socket, RV_FALSE, NULL, logMgr);
        RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpOpenFrom: failed to open RTCP session: RvLockConstruct() failed"));
        RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpOpenFrom"));
        return NULL;
    }

    /* Make sure we wait for RTCP packets */
    RvFdConstruct(&s->selectFd, &s->socket, logMgr);
    /* error result is not irregular behaviour, that is why NULL used for
       eluminating unneeded error printings */
    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(use_smp_media()) {
      s->selectEngine = NULL;
      status=RV_OK;
    } else
#endif
//SPIRENT_END
    status = RvSelectGetThreadEngine(/*logMgr*/NULL, &s->selectEngine);
    if ((status != RV_OK) || (s->selectEngine == NULL))
        s->selectEngine = rvRtcpInstance.selectEngine;
    RvSelectAdd(s->selectEngine, &s->selectFd, RvSelectRead, rtcpEventCallback);

    memset(&(s->myInfo.SDESbank), 0, sizeof(s->myInfo.SDESbank)); /* initialization */
    setSDES(RV_RTCP_SDES_CNAME, &(s->myInfo.SDESbank[RV_RTCP_SDES_CNAME]), (RvUint8*)cname, (RvInt32)strlen(cname));

    s->myInfo.lastPackNTP = RvUint64Const(0,0);
    s->myInfo.rtpClockRate = 0;
    RvRtpAddressListConstruct(&s->addressList);
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpOpenFrom"));
    return (RvRtcpSession)s;
}
/************************************************************************************
 * RvRtcpSetSDESItem
 * description: Defines and sets the SDES Items to the sessions
 * input: hRCTP        - The handle of the RTCP session (which must be opened).
 *        SDEStype     - type of SDES item
 *        item -         string of SDES item
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 * Remark: CNAME - is set through session opening
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvRtcpSetSDESItem(
                                      IN  RvRtcpSession  hRTCP,
                                      IN  RvRtcpSDesType SDEStype,
                                      IN  char* item)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSetSDESItem"));
    if (hRTCP == NULL)
	{
        RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpSetSDESItem: NULL RTCP session handle"));
        RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetSDESItem"));
        return RV_ERROR_BADPARAM;
	}
    switch(SDEStype)
    {
    case RV_RTCP_SDES_CNAME:
    case RV_RTCP_SDES_NAME:
    case RV_RTCP_SDES_EMAIL:
    case RV_RTCP_SDES_PHONE:
    case RV_RTCP_SDES_LOC:
    case RV_RTCP_SDES_TOOL:
    case RV_RTCP_SDES_NOTE:
    case RV_RTCP_SDES_PRIV:
        setSDES(SDEStype, &(s->myInfo.SDESbank[SDEStype]), (RvUint8*)item, (RvInt32)strlen(item));
        break;
    default:
		{
			RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpSetSDESItem: wrong SDES type"));
			RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetSDESItem"));
			return RV_ERROR_BADPARAM;
		}
    }
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetSDESItem"));
    return RV_OK;
}
/************************************************************************************
 * RvRtcpOpen
 * description: Opens a new RTCP session.
 * input: ssrc        - Synchronization source value for the RTCP session.
 *        pRtcpAddress contains the IP address (UDP port number) to be used for the RTCP session.
 *        cname       - Unique name representing the source of the RTP data.
 *                      Text that identifies the session. Must not be NULL.
 * output: none.
 * return value: If no error occurs, the function returns a handle for the
 *                new RTCP session. Otherwise it returns NULL.
 * Remarks
 *  - RvRtcpOpen() allocates a default number of 50 RTCP session members.
 *    This may result in a large allocation irrespective of whether there are only a few members.
 *    To prevent the large allocation, use RvRtcpOpenFrom() and indicate the actual number of
 *    session members.
 *  - If the port parameter is equal to zero in the RvRtcpOpen() call, then an arbitrary
 *    port number will be used for the new RTCP session. Using a zero port number may
 *    cause problems with some codecs that anticipate the RTP and RTCP ports
 *    to be near each other. We recommend defining a port number and not using zero.
 ***********************************************************************************/
#ifdef RVRTP_OLD_CONVENTION_API
RVAPI
RvRtcpSession RVCALLCONV rtcpOpen(
        IN  RvUint32    ssrc,
        IN  RvUint16    port,
        IN  char *      cname)
{
    RvNetAddress rtcpAddress;
    RvNetIpv4 Ipv4;
    Ipv4.port = port;
    Ipv4.ip = 0;
    RvNetCreateIpv4(&rtcpAddress, &Ipv4);
    return RvRtcpOpen(ssrc, &rtcpAddress, cname);
}
#endif
RVAPI
RvRtcpSession RVCALLCONV RvRtcpOpen(
                                  IN  RvUint32    ssrc,
                                  IN  RvNetAddress* pRtcpAddress,
                                  IN  char *      cname)
{
    rtcpSession* s;
    RvInt32 allocSize = RvRtcpGetAllocationSize(RTCP_MAXRTPSESSIONMEMBERS);
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpOpen"));
    if (NULL == pRtcpAddress ||
        RvMemoryAlloc(NULL, (RvSize_t)allocSize, logMgr, (void**)&s) != RV_OK)
        return NULL;

    if((rtcpSession*)RvRtcpOpenFrom(ssrc, pRtcpAddress, cname, RTCP_MAXRTPSESSIONMEMBERS, (void*)s, allocSize)==NULL)
    {
        RvMemoryFree(s, logMgr);
        return NULL;
    }

    s->isAllocated = RV_TRUE;
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpOpen"));
    return (RvRtcpSession)s;
}

/************************************************************************************
 * RvRtcpClose
 * description: Closes an RTCP session.
 * input: hRCTP        - The handle of the RTCP session.
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/

RVAPI
RvInt32 RVCALLCONV RvRtcpClose(
                IN  RvRtcpSession  hRTCP)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpClose"));
    if (hRTCP == NULL)
        return RV_ERROR_UNKNOWN;

    /* Cancel the timer is one is active */
    if (s->isTimerSet)
        RvTimerCancel(&s->timer, RV_TIMER_CANCEL_WAIT_FOR_CB);

#if defined(UPDATED_BY_SPIRENT)
    //we close RTP/RTCP session only from select() thread()
    {
      const char *reason="EndOfCall";
      RvInt32 length=strlen(reason);

      RvRtcpSessionSendBye(hRTCP,reason,length);

      RvRtpAddressListDestruct(&s->addressList);
      RvSelectRemove(s->selectEngine, &s->selectFd);
      RvFdDestruct(&s->selectFd);
      /* Close the socket */
      RvSocketDestruct(&s->socket, RV_FALSE, NULL, logMgr);
      /* Destroy the lock */
      RvLockDestruct(&s->lock, logMgr);
      /* free memory allocated for rtcpSession */
      if (s->isAllocated)
	RvMemoryFree(s, logMgr);
    }
#else
    
    /* Don't free session here.
       Instead of this register to WRITE event and free it on the WRITE event
       handling. The event handling is performed from select() thread.
       In this way we ensure that the session memory is not freed
       from the current thread, while any event is being handled
       from the select() thread. */
    RvSelectUpdate(s->selectEngine, &s->selectFd, RvSelectWrite, rtcpEventCallback);
#endif

    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpClose"));
    return RV_OK;
}


/*=========================================================================**
**  == rtcpSetRemoteAddress() ==                                           **
**                                                                         **
**  Defines the address of the remote peer or of the multicast group.      **
**                                                                         **
**  PARAMETERS:                                                            **
**      hRTCP    The handle of the RTCP session.                           **
**                                                                         **
**      ip       The IP address to which the RTCP packets will be sent.    **
**                                                                         **
**      port     The UDP port to which the RTCP packets should be sent.    **
**                                                                         **
**  RETURNS:                                                               **
**      A non-negative value upon success, or a negative integer error     **
**      code.                                                              **
**                                                                         **
**=========================================================================*/
#ifdef RVRTP_OLD_CONVENTION_API
RVAPI
void RVCALLCONV rtcpSetRemoteAddress(
                IN  RvRtcpSession  hRTCP,     /* RTCP Session Opaque Handle */
                IN  RvUint32      ip,        /* target ip address */
                IN  RvUint16      port)      /* target UDP port */
{
    RvNetAddress rtcpAddress;
    RvNetIpv4 Ipv4;

    Ipv4.port = port;
    Ipv4.ip = ip;
    RvNetCreateIpv4(&rtcpAddress, &Ipv4);
    RvRtcpSetRemoteAddress(hRTCP, &rtcpAddress);
}
#endif
/************************************************************************************
 * RvRtcpSetRemoteAddress
 * description: Defines the address of the remote peer or of the multicast group.
 * input: hRCTP        - The handle of the RTCP session.
 *        pRtcpAddress - pointer to RvNetAddress(to which packets to be sent)
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/
RVAPI
void RVCALLCONV RvRtcpSetRemoteAddress(
                                     IN  RvRtcpSession  hRTCP,     /* RTCP Session Opaque Handle */
                                     IN  RvNetAddress* pRtcpAddress)    /* target ip address and target UDP port */
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvAddress* pRvAddress = NULL;
    RvAddress  destAddress;
    RvNetIpv6 Ipv6 = {{0,0,0,0,0,0,0,0,
                       0,0,0,0,0,0,0,0}, RvUint16Const(0), RvUint32Const(0)
                       //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
                       ,RvUint32Const(0)
#endif
                       //SPIRENT_END
    }; /* initialization for warning elimination */
    RvUint32  delay;

    RTPLOG_ENTER(SetRemoteAddress);
    if (s == NULL)
    {
        RTPLOG_ERROR_LEAVE(SetRemoteAddress,"NULL session handle");
        return;
    }   
    RvRtcpRemoveAllRemoteAddresses(hRTCP);
    if (pRtcpAddress == NULL || RvNetGetAddressType(pRtcpAddress)==RVNET_ADDRESS_NONE||
        (RvNetGetAddressType(pRtcpAddress)==RVNET_ADDRESS_IPV6 &&
         RvNetGetIpv6(&Ipv6, pRtcpAddress)!=RV_OK))
    {
        RvRtcpStop(hRTCP);
        RTPLOG_ERROR_LEAVE(SetRemoteAddress,"bad address parameter or IPV6 is not supported in current configuration");
        return;
    }
    else
    {
        if (RvNetGetAddressType(pRtcpAddress)==RVNET_ADDRESS_IPV6)
        {
           //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
           pRvAddress = RvAddressConstructIpv6(&destAddress, Ipv6.ip, Ipv6.port, Ipv6.scopeId, Ipv6.flowinfo);
#else
           pRvAddress = RvAddressConstructIpv6(&destAddress, Ipv6.ip, Ipv6.port, Ipv6.scopeId);
#endif
           //SPIRENT_END
        }
        else
        {
           RvNetIpv4 Ipv4;
           RvNetGetIpv4(&Ipv4, pRtcpAddress);
           pRvAddress = RvAddressConstructIpv4(&destAddress, Ipv4.ip, Ipv4.port);
        }
        if (pRvAddress != NULL)
        {
            if (s->rtpSession!=NULL && s->profilePlugin != NULL && 
                s->profilePlugin->funcs != NULL &&
                s->profilePlugin->funcs->addRemAddress != NULL)
            {
                s->profilePlugin->funcs->addRemAddress(s->profilePlugin, s->rtpSession, RV_FALSE, pRtcpAddress);
            }
            
            RvRtpAddressListAddAddress(&s->addressList, pRvAddress);
            s->remoteAddressSet = RV_TRUE;

            if (s->isTimerSet == RV_FALSE)
            {
                RvTimerQueue *timerQ = NULL;
                RvStatus status;

                delay =  rtcpInitTransmittionIntervalMechanism(s);
                /* Find the timer queue we are working with */

                status = RvSelectGetTimeoutInfo(s->selectEngine, NULL, &timerQ);
                if (status != RV_OK)
                    timerQ = NULL;

                if (timerQ == NULL)
                    timerQ = rvRtcpInstance.timersQueue;

                /* Set a timer */
                  status = rtcpSetTimer(s, RvInt64Mul(RV_TIME64_NSECPERMSEC, RvInt64Const(1,0,delay)));
                if (status == RV_OK)
                    s->isTimerSet = RV_TRUE;
            }
        }
    }
    RTPLOG_LEAVE(SetRemoteAddress);
}
/************************************************************************************
 * RvRtcpAddRemoteAddress
 * description: Adds the address of the remote peer or of
 *             the multicast group or for multiunicast.
 * input: hRCTP        - The handle of the RTCP session.
 *        pRtcpAddress - pointer to RvNetAddress(to which packets to be sent)
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/
RVAPI
void RVCALLCONV RvRtcpAddRemoteAddress(
    IN  RvRtcpSession  hRTCP,     /* RTCP Session Opaque Handle */
    IN  RvNetAddress* pRtcpAddress) /* target ip address and target UDP port */
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvAddress* pRvAddress = NULL;
    RvAddress  destAddress;
    RvNetIpv6 Ipv6 = {{0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0}, RvUint16Const(0), RvUint32Const(0)
       //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
       ,RvUint32Const(0)
#endif
       //SPIRENT_END
    }; /* initialization for warning elimination */
    RvUint32  delay;
    
    RTPLOG_ENTER(AddRemoteAddress);
    if (NULL==pRtcpAddress||RvNetGetAddressType(pRtcpAddress)==RVNET_ADDRESS_NONE||
        (RvNetGetAddressType(pRtcpAddress)==RVNET_ADDRESS_IPV6 &&
        RvNetGetIpv6(&Ipv6, pRtcpAddress)!=RV_OK))
    {
        RTPLOG_ERROR_LEAVE(AddRemoteAddress, "bad address parameter or IPV6 is not supported in current configuration");
        return;
    }

    if (RvNetGetAddressType(pRtcpAddress) == RVNET_ADDRESS_IPV6)
    {
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
        pRvAddress = RvAddressConstructIpv6(&destAddress, Ipv6.ip, Ipv6.port, Ipv6.scopeId, Ipv6.flowinfo);
#else
        pRvAddress = RvAddressConstructIpv6(&destAddress, Ipv6.ip, Ipv6.port, Ipv6.scopeId);
#endif
//SPIRENT_END
    }
    else
    {
        RvNetIpv4 Ipv4;
        RvNetGetIpv4(&Ipv4, pRtcpAddress);
        pRvAddress = RvAddressConstructIpv4(&destAddress, Ipv4.ip, Ipv4.port);
    }
    if (pRvAddress != NULL)
    {
        if (s->rtpSession!=NULL && s->profilePlugin != NULL && 
            s->profilePlugin->funcs != NULL &&
            s->profilePlugin->funcs->addRemAddress != NULL)
        {
            s->profilePlugin->funcs->addRemAddress(s->profilePlugin, s->rtpSession, RV_FALSE, pRtcpAddress);
        }
        RvRtpAddressListAddAddress(&s->addressList, pRvAddress);
        s->remoteAddressSet = RV_TRUE;

        if (s->isTimerSet == RV_FALSE)
        {
            RvTimerQueue *timerQ = NULL;
            RvStatus status;

            delay =  rtcpInitTransmittionIntervalMechanism(s);
            /* Find the timer queue we are working with */

            status = RvSelectGetTimeoutInfo(s->selectEngine, NULL, &timerQ);
            if (status != RV_OK)
                timerQ = NULL;

            if (timerQ == NULL)
                timerQ = rvRtcpInstance.timersQueue;

            /* Set a timer */
            status = rtcpSetTimer(s, RvInt64Mul(RV_TIME64_NSECPERMSEC, RvInt64Const(1, 0, delay)));
            if (status == RV_OK)
                s->isTimerSet = RV_TRUE;
        }
    }
    RTPLOG_LEAVE(AddRemoteAddress);
}
/************************************************************************************
 * RvRtcpRemoveRemoteAddress
 * description: removes the specified RTCP address of the remote peer or of the multicast group
 *              or of the multiunicast list with elimination of address duplication.
 * input: hRCTP        - The handle of the RTCP session.
 *        pRtcpAddress - pointer to RvNetAddress to remove.
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/
RVAPI
void RVCALLCONV RvRtcpRemoveRemoteAddress(
    IN  RvRtcpSession  hRTCP,
    IN  RvNetAddress* pRtcpAddress)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvAddress* pRvAddress = NULL;

    RTPLOG_ENTER(RemoveRemoteAddress);
    if (NULL==s||NULL==pRtcpAddress||RvNetGetAddressType(pRtcpAddress)==RVNET_ADDRESS_NONE)
    {
        RTPLOG_ERROR_LEAVE(RemoveRemoteAddress, "NULL session pointer or wrong parameter");
        return;
    }
    pRvAddress = (RvAddress*) pRtcpAddress->address;

    if (s!=NULL && s->rtpSession!=NULL &&
        s->profilePlugin != NULL && 
        s->profilePlugin->funcs!=NULL &&
        s->profilePlugin->funcs->removeRemAddress != NULL)
        s->profilePlugin->funcs->removeRemAddress(s->profilePlugin, s->rtpSession, pRtcpAddress);

    RvRtpAddressListRemoveAddress(&s->addressList, pRvAddress);    

    pRvAddress = RvRtpAddressListGetNext(&s->addressList, NULL);
    if (NULL == pRvAddress)
        s->remoteAddressSet = RV_FALSE;
    RTPLOG_LEAVE(RemoveRemoteAddress);
}
/************************************************************************************
 * RvRtcpRemoveAllRemoteAddresses
 * description: removes all RTCP addresses of the remote peer or of the multicast group
 *              or of the multiunicast list with elimination of address duplication.
 * input: hRCTP        - The handle of the RTCP session.
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/
RVAPI
void RVCALLCONV RvRtcpRemoveAllRemoteAddresses(
    IN  RvRtcpSession  hRTCP)
{
    rtcpSession*       s          = (rtcpSession *)hRTCP;
    RvAddress*         pRvAddress = NULL;   
    
    RTPLOG_ENTER(RemoveAllRemoteAddresses);
    if (NULL == s)
    {
        RTPLOG_ERROR_LEAVE(RemoveAllRemoteAddresses,"NULL session pointer");
        return;
    }
    while((pRvAddress = RvRtpAddressListGetNext(&s->addressList, NULL))!= NULL)
    {   
        if ( s->rtpSession!=NULL && s->profilePlugin != NULL && 
            s->profilePlugin->funcs != NULL &&
            s->profilePlugin->funcs->removeRemAddress != NULL)
        {
            s->profilePlugin->funcs->removeRemAddress(s->profilePlugin, s->rtpSession, (RvNetAddress *) pRvAddress);
        }
        RvRtpAddressListRemoveAddress(&s->addressList, pRvAddress);
    }
    s->remoteAddressSet = RV_FALSE;
    RTPLOG_LEAVE(RemoveAllRemoteAddresses);
}
/************************************************************************************
 * RvRtcpStop
 * description: Stops RTCP session without closing it. This allows multiple subsequent
 *              RTCP sessions to share the same hRTCP without need to close/open UDP socket.
 * input: hRCTP        - The handle of the RTCP session.
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 * Remarks
 * - This function is used when it would be wasteful to call RvRtcpOpen() every time
 *   there is a call, for example in a gateway.
 *   When the call ends, rtcpStop() clears the call and enables the resumption of a new call,
 * as follows:
 *
 *   RvRtpOpen
 *   RvRtcpOpen
 *   ....RvRtcpSetRemoteAddress
 *   :
 *   ...RvRtcpStop
 *   ...RvRtcpSetRemoteAddress
 *   :
 *   RvRtpClose
 *   RvRtcpClose
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtcpStop(
                IN  RvRtcpSession  hRTCP)     /* RTCP Session Opaque Handle */
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    
    RTPLOG_ENTER(Stop);

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(!s) {
       RTPLOG_LEAVE(Stop);
       return RV_Failure;
    }
#endif
    //SPIRENT_END

    /* Cancel the timer if one is active */
    if (s->isTimerSet)
    {
        RvTimerCancel(&s->timer, RV_TIMER_CANCEL_WAIT_FOR_CB);
        s->isTimerSet = RV_FALSE;
    }
    RvRtcpRemoveAllRemoteAddresses(hRTCP);

    /* Clear the list*/
    memset(s->participantsArray,0, (RvSize_t)(sizeof(rtcpInfo))*(s->sessionMembers));
    s->myInfo.collision = 0;
    s->myInfo.active = 0;
    s->myInfo.timestamp = 0;
    memset(&(s->myInfo.eSR),0,sizeof(s->myInfo.eSR));
    
    RTPLOG_LEAVE(Stop);
    return RV_OK;
}
/********************************************************************************************
 * RvRtcpSetTypeOfService
 * Set the type of service (DiffServ Code Point) of the socket (IP_TOS)
 * This function is supported by few operating systems.
 * IPV6 does not support type of service.
 * This function is thread-safe.
 * INPUT   : hRTCP           - RTCP session to set TOS byte
 *           typeOfService  - type of service to set
 * OUTPUT  : None
 * RETURN  : RV_OK on success, other on failure
 * Remark: In Windows environment for setting the TOS byte RvRtpOpen or RvRtpOpenEx
 *         must be called with local IP address and not with "any" address. 
********************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvRtcpSetTypeOfService(
        IN RvRtcpSession hRTCP,
        IN RvInt32        typeOfService)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvStatus result = RV_OK;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSetTypeOfService"));
    if (NULL == s || !s->isAllocated)
    {
        RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpSetTypeOfService: NULL pointer or socket is not allocated"));
        return RV_ERROR_NULLPTR;
    }
    result = RvSocketSetTypeOfService(&s->socket, typeOfService, logMgr);
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetTypeOfService"));
    return result;
}
/********************************************************************************************
 * RvRtcpGetTypeOfService
 * Get the type of service (DiffServ Code Point) of the socket (IP_TOS)
 * This function is supported by few operating systems.
 * IPV6 does not support type of service.
 * This function is thread-safe.
 * INPUT   : hRTCP           - RTCP session handle
 * OUTPUT  : typeOfServicePtr  - pointer to type of service to set
 * RETURN  : RV_OK on success, other on failure
 *********************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvRtcpGetTypeOfService(
        IN RvRtpSession hRTCP,
        OUT RvInt32*     typeOfServicePtr)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvStatus result = RV_OK;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpGetTypeOfService"));
    if (NULL == s || !s->isAllocated)
    {
        RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpGetTypeOfService: NULL pointer or socket is not allocated"));
        return RV_ERROR_NULLPTR;
    }
    result = RvSocketGetTypeOfService(&s->socket, logMgr, typeOfServicePtr);
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpGetTypeOfService"));
    return result;
}
/************************************************************************************
 * RvRtcpRTPPacketRecv
 * description: Informs the RTCP session about a packet that was received
 *              in the corresponding RTP session. Call this function after RvRtpRead().
 * input: hRCTP - The handle of the RTCP session.
 *        ssrc  - The synchronization source value of the participant that sent the packet.
 *        localTimestamp - The local RTP timestamp when the received packet arrived.
 *        myTimestamp    - The RTP timestamp from the received packet.
 *        sequence       - The packet sequence number
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/

RVAPI
RvInt32 RVCALLCONV RvRtcpRTPPacketRecv(
                IN  RvRtcpSession  hRTCP,
                IN  RvUint32      ssrc,
                IN  RvUint32      localTimestamp,
                IN  RvUint32      myTimestamp,
                IN  RvUint16      sequence)

{
    rtcpSession *s = (rtcpSession *)hRTCP;
    rtcpInfo *fInfo;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpRTPPacketRecv"));

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(!s) {
       RTPLOG_LEAVE(Stop);
       return RV_Failure;
    }
    RvLockGet(&s->lock, logMgr);
#endif
    //SPIRENT_END

    if (ssrc == s->myInfo.ssrc)
    {
        s->myInfo.collision = 1;
        RvLockRelease(&s->lock, logMgr);
        return ERR_RTCP_SSRCCOLLISION;
    }

    /* See if we can find this source or not */
    fInfo = findSSrc(s,ssrc);

    /* If we didn't find this SSRC, we lock the RTCP database and search for the
       SSRC again. If we don't find it again - we insert it to the list, and
       finally we unlock... */
    if (!fInfo)  /* New source */
    {
        /* this section is working with threads.*/
        /* Lock the rtcp session.*/
#ifndef UPDATED_BY_SPIRENT
        RvLockGet(&s->lock, logMgr);
#endif

        /* check if the ssrc is exist*/
        fInfo = findSSrc(s,ssrc);

        if (!fInfo)
        {
            /* Still no SSRC - we should add it to the list ourselves */
            fInfo = insertNewSSRC(s, ssrc);
            if (fInfo ==  NULL)
            {
                RvLockRelease(&s->lock, logMgr);
                RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpRTPPacketRecv :no room to add new session member"));
                RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpRTPPacketRecv"));
                return RV_ERROR_OUTOFRESOURCES;
                /* error  cannot add the SSRC to SSRC members list */
            }
            init_seq(&(fInfo->src),sequence);
            fInfo->src.max_seq = (RvUint16)(sequence - 1);
        }
#ifndef UPDATED_BY_SPIRENT
        /* unlock the rtcp session.*/
        RvLockRelease(&s->lock, logMgr);
#endif
    }


    if (fInfo != NULL)
    {
        if (!fInfo->invalid)
        {
            fInfo->active = RV_TRUE;
            update_seq(&(fInfo->src), sequence, localTimestamp, myTimestamp);
        }
    }

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    RvLockRelease(&s->lock, logMgr);
#endif
    //SPIRENT_END

    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpRTPPacketRecv"));
    return RV_OK;
}

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)

static inline rtcpInfo *findSSrcLight(rtcpSession *s, RvUint32 ssrc)
{
    RvInt32     index = 0;
    RvBool  doAgain = RV_TRUE;
    rtcpInfo *pInfo;

    /* Look for the given SSRC */
    while ((doAgain) && (index < s->sessionMembers))
    {
       if (s->participantsArray[index].ssrc == ssrc)
            doAgain = RV_FALSE;
       else
           index ++;

    }
    if (index < s->sessionMembers )
        pInfo = &s->participantsArray[index];
    else
        pInfo = NULL;

    return pInfo;
}

/* insert new member to a participants table */
static inline rtcpInfo *insertNewSSRCLight(rtcpSession *s, RvUint32 ssrc)
{
    rtcpInfo* pInfo = NULL;
    RvInt32 index;

    if (s->sessionMembers >= s->maxSessionMembers)
    {
        /* We've got too many - see if we can remove some old ones */
        for(index=0; index < s->sessionMembers; index++)
            if (s->participantsArray[index].invalid &&
                s->participantsArray[index].ssrc == 0)
                break;
            /* BYE message can transform participant to invalid */
        /* if index < s->sessionMembers found unusable index */
    }
    else
    {
        /* Add it as a new one to the list */
        index = s->sessionMembers++;
    }

    if (index < s->sessionMembers)
    {
        /* Got a place for it ! */
        pInfo = &s->participantsArray[index];
        memset(pInfo, 0, sizeof(rtcpInfo));
        pInfo->ssrc             = ssrc;
        pInfo->eToRR.ssrc       = ssrc;
        pInfo->active           = RV_FALSE;
        pInfo->src.probation    = MIN_SEQUENTIAL;
    }

    return pInfo;
}

RVAPI
RvStatus RVCALLCONV RvRtcpRTPPacketRecvLight(
        IN  RvRtcpSession  hRTCP,
        IN  RvUint32      ssrc,
        IN  RvUint32      localTimestamp,
        IN  RvUint32      myTimestamp,
        IN  RvUint16      sequence) {

    rtcpSession *s = (rtcpSession *)hRTCP;
    rtcpInfo *fInfo;

    if(!s) {
       return RV_Failure;
    }

    if (ssrc == s->myInfo.ssrc)
    {
        s->myInfo.collision = 1;
        return ERR_RTCP_SSRCCOLLISION;
    }

    /* See if we can find this source or not */
    fInfo = findSSrcLight(s,ssrc);

    /* If we didn't find this SSRC, we lock the RTCP database and search for the
       SSRC again. If we don't find it again - we insert it to the list, and
       finally we unlock... */
    if (!fInfo)  /* New source */
    {
       /* No SSRC - we should add it to the list ourselves */
       fInfo = insertNewSSRCLight(s, ssrc);
       if (fInfo ==  NULL)
       {
          return RV_ERROR_OUTOFRESOURCES;
          /* error  cannot add the SSRC to SSRC members list */
       }
       init_seq(&(fInfo->src),sequence);
       fInfo->src.max_seq = (RvUint16)(sequence - 1);
    }


    if (fInfo && !fInfo->invalid) {
       fInfo->active = RV_TRUE;
       update_seq(&(fInfo->src), sequence, localTimestamp, myTimestamp);
    }

    return RV_OK;
}
#endif
//SPIRENT_END

/************************************************************************************
 * RvRtcpRTPPacketSent
 * description: Informs the RTCP session about a packet that was sent
 *              in the corresponding RTP session. Call this function after RvRtpWrite()
 * input: hRCTP - The handle of the RTCP session.
 *        bytes - The number of bytes in the sent packet.
 *    timestamp - The RTP timestamp from the sent packet
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtcpRTPPacketSent(
                IN  RvRtcpSession  hRTCP,
                IN  RvInt32       bytes,
                IN  RvUint32      timestamp)
{
    rtcpSession *s = (rtcpSession *)hRTCP;

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(!s) {
       return RV_Failure;
    }
#endif
    //SPIRENT_END
    
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpRTPPacketSent"));
    
    RvLockGet(&s->lock, logMgr);
    if (s->isShutdown)
    {
        RvLockRelease(&s->lock, logMgr);
        RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpRTPPacketSent: sent RTP packet after shutting down"));
        return ERR_RTCP_SHUTDOWN;
    }
    s->myInfo.active = 2; /* to store active state for 2 previous transmission
    interval reports, as per RFC 3550 6.3 (we_sent parameter) */
    s->myInfo.eSR.nPackets++;
    s->myInfo.eSR.nBytes += bytes;
    s->myInfo.lastPackNTP   = getNNTPTime();
    s->myInfo.lastPackRTPts = timestamp;
    
    if (s->myInfo.collision)
    {
        RvLockRelease(&s->lock, logMgr);
        return ERR_RTCP_SSRCCOLLISION;
    }   
    RvLockRelease(&s->lock, logMgr);
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpRTPPacketSent"));
    return RV_OK;
}

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)

RVAPI
RvInt32 RVCALLCONV RvRtcpRTPPacketSentLight(
                IN  RvRtcpSession  hRTCP,
                IN  RvInt32       bytes,
                IN  RvUint32      timestamp)
{
    rtcpSession *s = (rtcpSession *)hRTCP;

    if(!s) {
       return RV_Failure;
    }

    if (s->isShutdown)
    {
        return ERR_RTCP_SHUTDOWN;
    }
    s->myInfo.active = 2; /* to store active state for 2 previous transmission
    interval reports, as per RFC 3550 6.3 (we_sent parameter) */
    s->myInfo.eSR.nPackets++;
    s->myInfo.eSR.nBytes += bytes;
    s->myInfo.lastPackNTP   = getNNTPTime();
    s->myInfo.lastPackRTPts = timestamp;
    
    if (s->myInfo.collision)
    {
        return ERR_RTCP_SSRCCOLLISION;
    }
    return RV_OK;
}

#endif
    //SPIRENT_END

/*=========================================================================**
**  == rtcpGetPort() ==                                                    **
**                                                                         **
**  Gets the UDP port of an RTCP session.                                  **
**                                                                         **
**  PARAMETERS:                                                            **
**      hRTCP      The handle of the RTCP session.                         **
**                                                                         **
**  RETURNS:                                                               **
**      A non-negative value upon success, or a negative integer error     **
**      code.                                                              **
**                                                                         **
**=========================================================================*/

RVAPI
RvUint16 RVCALLCONV RvRtcpGetPort(
                IN  RvRtcpSession  hRTCP)
{
    rtcpSession* s = (rtcpSession *)hRTCP;
    RvAddress localAddress;
    RvUint16 sockPort = 0;
    RvStatus res;

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(!s) {
       return RV_Failure;
    }
#endif
    //SPIRENT_END

    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpGetPort"));
    res = RvSocketGetLocalAddress(&s->socket, logMgr, &localAddress);

    if (res == RV_OK)
    {
        sockPort = RvAddressGetIpPort(&localAddress);

        RvAddressDestruct(&localAddress);
    }
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpGetPort"));
    return sockPort;
}


/************************************************************************************
 * RvRtcpSetBandwidth
 * description: Sets the maximum bandwidth for a RTCP session.
 * input: hRCTP - The handle of the RTCP session.
 *    bandwidth - The bandwidth for RTCP packets.
 * output: none.
 * return none.
 * Note: according to standard RTCP bandwith must be about 5% of RTP bandwith
 ***********************************************************************************/

RVAPI
void RVCALLCONV RvRtcpSetBandwidth(
        IN  RvRtcpSession  hRTCP,
        IN  RvUint32       bandwidth)
{
    rtcpSession* s = (rtcpSession *)hRTCP;

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(s) {
       RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSetBandwidth"));
       s->txInterval.rtcpBandwidth = bandwidth;
       RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetBandwidth"));
    }
#else

    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSetBandwidth"));
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetBandwidth"));
    s->txInterval.rtcpBandwidth = bandwidth;
    
#endif
    //SPIRENT_END
}

                   /* == ENDS: Basic RTCP Functions == */



                    /* == Accessory RTCP Functions == */


/************************************************************************************
 * RvRtcpCheckSSRCCollision
 * description: Checks for SSRC collisions in the RTCP session and the corresponding
 *              RTP session.
 * input: hRCTP - The handle of the RTCP session.
 * output: none.
 * return value:
 *  If no collision was detected, the function returns RV_FALSE. Otherwise, it returns RV_TRUE.
 *
 * Remarks
 *  You can check if an SSRC collision has occurred, by calling this function
 *  rtcpRTPPacketSent() or rtcpRTPPacketRecv() returns an error.
 ***********************************************************************************/

RVAPI
RvBool RVCALLCONV RvRtcpCheckSSRCCollision(
                IN  RvRtcpSession  hRTCP)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpCheckSSRCCollision"));
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpCheckSSRCCollision"));

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(!s) {
       return RV_FALSE;
    }
#endif
    //SPIRENT_END

    return (s->myInfo.collision != 0);
}


/*=========================================================================**
**  == RvRtcpEnumParticipants() ==                                           **
**                                                                         **
**  Provides information about in the RTCP session and the corresponding   **
**  RTP session.                                                           **
**                                                                         **
**  PARAMETERS:                                                            **
**      hRTCP      The handle of the RTCP session.                         **
**                                                                         **
**      enumerator A pointer to the function that will be called once per  **
**                 SSRC in the session.                                    **
**                                                                         **
**  RETURNS:                                                               **
**      If the enumeration process was stopped by the enumerator, the      **
**      function returns RV_FALSE, otherwise RV_TRUE.                      **
**                                                                         **
**  The prototype of the SSRC enumerator is as follows:                    **
**                                                                         **
**      RvBool                                                             **
**      SSRCENUM(                                                          **
**        IN  RvRtpSession  hTRCP,                                          **
**        IN  RvUint32     ssrc                                            **
**      );                                                                 **
**                                                                         **
**  The parameters passed to the enumerator are as follows:                **
**      hRTCP      The handle of the RTCP session.                         **
**                                                                         **
**      ssrc       A synchronization source that participates in the       **
**                 session.                                                **
**                                                                         **
**  The enumerator should return RV_FALSE if it wants the enumeration      **
**  process to continue.  Returning RV_TRUE will cause                     **
**  RvRtcpEnumParticipant() to return immediately.                         **
**                                                                         **
**=========================================================================*/

RVAPI
RvBool RVCALLCONV RvRtcpEnumParticipants(
                IN RvRtcpSession hRTCP,
                IN RvRtcpSSRCENUM_CB   enumerator)
{
    RvInt32 elem, ssrc=0;

    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpEnumParticipants"));
    elem = RvRtcpGetEnumFirst(hRTCP, &ssrc);

    while (elem >= 0)
    {
        if (enumerator(hRTCP, ssrc))
        {
            return RV_FALSE;
        }

        elem = RvRtcpGetEnumNext(hRTCP, elem, &ssrc);
    }
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpEnumParticipants"));
    return RV_TRUE;
}


/************************************************************************************
 * RvRtcpGetSourceInfo
 * description: Provides information about a particular synchronization source.
 * input: hRTCP - The handle of the RTCP session.
 *        ssrc  - The source for which information is required. Get the SSRC from the function.
 * output: info  - Information about the synchronization source.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 * Remark: To get Self information use the own SSRC of the session.
 ***********************************************************************************/

RVAPI
RvInt32 RVCALLCONV RvRtcpGetSourceInfo(
                IN   RvRtcpSession  hRTCP,
                IN   RvUint32      ssrc,
                OUT  RvRtcpINFO *    info)


{
    rtcpSession *s = (rtcpSession *)hRTCP;
    rtcpInfo *fInfo;

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(!s) {
       return RV_Failure;
    }
#endif
    //SPIRENT_END

    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpGetSourceInfo"));
    if (ssrc == s->myInfo.ssrc)
    {
        info->selfNode          = RV_TRUE;

        RvLockGet(&s->lock, logMgr);
        info->sr.valid          = (s->myInfo.active>1);
#if ((RV_OS_TYPE == RV_OS_TYPE_PSOS) && (RV_OS_VERSION == RV_OS_PSOS_2_0)) || \
    (RV_OS_TYPE == RV_OS_TYPE_VXWORKS)
        /* just to get rid of annoying warnings */
        info->sr.mNTPtimestamp  = /*(RvUint32)((s->myInfo.eSR.tNNTP >> 16) >> 16);*/
            RvUint64ToRvUint32(RvUint64ShiftRight(RvUint64ShiftRight(s->myInfo.eSR.tNNTP, 16),16));
#else
        info->sr.mNTPtimestamp  = /*(RvUint32)(s->myInfo.eSR.tNNTP >> 32)*/ 
            RvUint64ToRvUint32(RvUint64ShiftRight(s->myInfo.eSR.tNNTP, 32));
#endif
        info->sr.lNTPtimestamp  = /*(RvUint32)(s->myInfo.eSR.tNNTP & 0xffffffff);*/
            RvUint64ToRvUint32(RvUint64And(s->myInfo.eSR.tNNTP, RvUint64Const(0, 0xffffffff)));
        info->sr.timestamp      = s->myInfo.eSR.tRTP;
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
        info->sr.txPacketsSLR = s->myInfo.eSR.nPackets - s->myInfo.eSR.nPackets_prior;
        s->myInfo.eSR.nPackets_prior = s->myInfo.eSR.nPackets;
#endif
//SPIRENT_END
        info->sr.packets        = s->myInfo.eSR.nPackets;
        info->sr.octets         = s->myInfo.eSR.nBytes;

        /* It's our node - receiver reports are not valid */
        info->rrFrom.valid      = RV_FALSE;
        info->rrTo.valid        = RV_FALSE;

        strncpy(info->cname, s->myInfo.SDESbank[RV_RTCP_SDES_CNAME].value, sizeof(info->cname));
        RvLockRelease(&s->lock, logMgr);
        RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpGetSourceInfo"));
        return RV_OK;
    }

    fInfo = findSSrc(s,ssrc);

    if (fInfo)
    {
       /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
       if ( fInfo->src.probation )
       {
          return ERR_RTCP_ILLEGALSSRC;
       }
#endif
       /* SPIRENT_END */

        info->selfNode              = RV_FALSE;
        
        RvLockGet(&s->lock, logMgr);
        info->sr.valid              = !fInfo->invalid;
#if ((RV_OS_TYPE == RV_OS_TYPE_PSOS) && (RV_OS_VERSION == RV_OS_PSOS_2_0)) || \
    (RV_OS_TYPE == RV_OS_TYPE_VXWORKS)
        /* just to get rid of annoying warnings */
        info->sr.mNTPtimestamp      = /*(RvUint32)((fInfo->eSR.tNNTP >> 16) >> 16);*/
                RvUint64ToRvUint32(RvUint64ShiftRight(RvUint64ShiftRight(fInfo->eSR.tNNTP, 16),16));
#else
        info->sr.mNTPtimestamp      = /*(RvUint32)(fInfo->eSR.tNNTP >> 32);*/
                RvUint64ToRvUint32(RvUint64ShiftRight(fInfo->eSR.tNNTP, 32));
#endif
        info->sr.lNTPtimestamp      = /*(RvUint32)(fInfo->eSR.tNNTP & 0xffffffff);*/
                RvUint64ToRvUint32(RvUint64And(fInfo->eSR.tNNTP, RvUint64Const(0, 0xffffffff)));
        info->sr.timestamp          = fInfo->eSR.tRTP;
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
        info->sr.txPacketsSLR = 0; //untransferrable value
#endif
//SPIRENT_END
        info->sr.packets            = fInfo->eSR.nPackets;
        info->sr.octets             = fInfo->eSR.nBytes;

        info->rrFrom.valid          = RV_TRUE;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
        info->rrFrom.rxPacketsSLR       = 0; //untransferrable value
#endif
/* SPIRENT END */
        info->rrFrom.fractionLost   = (fInfo->eFromRR.bfLost >> 24);
        info->rrFrom.cumulativeLost = (fInfo->eFromRR.bfLost & 0xffffff);
        info->rrFrom.sequenceNumber = fInfo->eFromRR.nExtMaxSeq;
        info->rrFrom.jitter         = fInfo->eFromRR.nJitter;
        info->rrFrom.lSR            = fInfo->eFromRR.tLSR;
        info->rrFrom.dlSR           = fInfo->eFromRR.tDLSR;

        info->rrTo.valid            = RV_TRUE;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
        info->rrTo.rxPacketsSLR         = fInfo->eToRR.rxPacketsSLR;
#endif
/* SPIRENT END */
        info->rrTo.fractionLost     = (fInfo->eToRR.bfLost >> 24);
        info->rrTo.cumulativeLost   = (fInfo->eToRR.bfLost & 0xffffff);
        info->rrTo.sequenceNumber   = fInfo->eToRR.nExtMaxSeq;
        info->rrTo.jitter           = fInfo->eToRR.nJitter;
        info->rrTo.lSR              = fInfo->eToRR.tLSR;
        info->rrTo.dlSR             = fInfo->eToRR.tDLSR;

        strncpy(info->cname, fInfo->eSDESCname.value, sizeof(info->cname));
        RvLockRelease(&s->lock, logMgr);
    }
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpGetSourceInfo"));
    return (!fInfo) ? ERR_RTCP_ILLEGALSSRC : 0;
}

/************************************************************************************
 * RvRtcpSetGroupAddress
 * description: Specifies a multicast address for an RTCP session.
 * input: hRTCP      - The handle of the RTCP session.
 *        pRtcpAddress - pointer to multicast address
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/
#ifdef RVRTP_OLD_CONVENTION_API
RVAPI
RvInt32 RVCALLCONV rtcpSetGroupAddress(
                IN  RvRtcpSession  hRTCP,
                IN  RvUint32      ip)
{
    RvNetAddress rtcpAddress;
    RvNetIpv4   Ipv4;
    Ipv4.ip = ip;
    Ipv4.port = 0;
    RvNetCreateIpv4(&rtcpAddress, &Ipv4);
    return RvRtcpSetGroupAddress(hRTCP, &rtcpAddress);
}
#endif
RVAPI
RvInt32 RVCALLCONV RvRtcpSetGroupAddress(
        IN  RvRtcpSession  hRTCP,
        IN  RvNetAddress* pRtcpAddress)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvAddress addresses[2], sockAddress;
    RvStatus res, res1, res2;
    RvNetIpv4 Ipv4;
    RvInt32 addressType = RV_ADDRESS_TYPE_NONE;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSetGroupAddress"));

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(!s) {
       return RV_Failure;
    }
#endif
    //SPIRENT_END

    if (NULL==pRtcpAddress||
        RV_ADDRESS_TYPE_NONE == (addressType = RvAddressGetType(&rvRtcpInstance.localAddress)))
    {
        RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpSetGroupAddress() pRtpAddress==NULL or local address is undefined"));
        RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetGroupAddress"))
        return RV_ERROR_BADPARAM;
    }
    if (RvNetGetAddressType(pRtcpAddress) == RVNET_ADDRESS_IPV6)
    {
#if (RV_NET_TYPE & RV_NET_IPV6)
       //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
        RvAddressConstructIpv6(&addresses[0],
            RvAddressIpv6GetIp(RvAddressGetIpv6((RvAddress*)pRtcpAddress->address)),
            RV_ADDRESS_IPV6_ANYPORT,
            RvAddressGetIpv6Scope((RvAddress*)pRtcpAddress->address),
            RvAddressIpv6GetFlowinfo(RvAddressGetIpv6((RvAddress*)pRtcpAddress->address)));
#else
        RvAddressConstructIpv6(&addresses[0],
            RvAddressIpv6GetIp(RvAddressGetIpv6((RvAddress*)pRtcpAddress->address)),
            RV_ADDRESS_IPV6_ANYPORT,
            RvAddressGetIpv6Scope((RvAddress*)pRtcpAddress->address));
#endif
        //SPIRENT_END
#else
        RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpSetGroupAddress: IPV6 is not supported in current configuration"));
        RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetGroupAddress"));
        return RV_ERROR_BADPARAM;
#endif
    }
    else
    {
        RvNetGetIpv4(&Ipv4, pRtcpAddress);
        RvAddressConstructIpv4(&addresses[0], Ipv4.ip, RV_ADDRESS_IPV4_ANYPORT);
    }
    RvAddressCopy(&rvRtcpInstance.localAddress, &addresses[1]);
    RvSocketGetLocalAddress(&s->socket, logMgr, &sockAddress);
    RvAddressSetIpPort(&addresses[1], RvAddressGetIpPort(&sockAddress)); /* any port for IPV6 and IPV4 */
        
    /* Allow sending to multicast addresses */
    res2 = RvSocketSetBroadcast(&s->socket, RV_TRUE, logMgr);
    
    res1 = RvSocketSetMulticastInterface(&s->socket, &addresses[1], logMgr);
    /* This function adds the specified address (in network format) to the specified
       Multicast interface for the specified socket.*/
    res = RvSocketJoinMulticastGroup(&s->socket, addresses, addresses+1, logMgr);
    if (res==RV_OK && res1==RV_OK && res2==RV_OK)
    {        
        RvChar addressStr[64];
        RTPLOG_INFO((RTP_SOURCE, "RvRtcpSetGroupAddress() - Successed to set multicast group address to %s for session (%#x)", 
            RvAddressGetString(&addresses[0], sizeof(addressStr), addressStr), s));
    }
    else
    {
		RTPLOG_ERROR((RTP_SOURCE, "RvRtcpSetGroupAddress - Failed to set multicast group address for session (%#x)", s));
    }
    RvAddressDestruct(&addresses[0]);
    RvAddressDestruct(&addresses[1]);
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetGroupAddress, result = %d", res));
	RV_UNUSED_ARG(addressType);
    return res;

}
/************************************************************************************
 * RvRtcpSetMulticastTTL
 * description:  Defines a multicast Time To Live (TTL) for the RTCP session 
 *               (multicast sending).
 * input:  hRTCP  - Handle of RTCP session.
 *         ttl   -  ttl threshold for multicast
 * output: none.
 * return value:  If no error occurs, the function returns the non-negative value.
 *                Otherwise, it returns a negative value.
 * Note: the function is supported with IP stack that has full multicast support
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtcpSetMulticastTTL(
	IN  RvRtcpSession  hRTCP,
	IN  RvUint8       ttl)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
	RvStatus res = RV_OK;
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSetMulticastTTL"));
    if (NULL==s)
	{
		RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetMulticastTTL"));
		RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpSetMulticastTTL() NULL session hendle"));
		return RV_ERROR_BADPARAM;
	}
    res = RvSocketSetMulticastTtl(&s->socket, (RvInt32) ttl, logMgr);	
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetMulticastTTL"));
	return res;
}
/************************************************************************************
 * RvRtcpGetSSRC
 * description: Returns the synchronization source value for an RTCP session.
 * input: hRTCP      - The handle of the RTCP session.
 * output: none.
 * return value: The synchronization source value for the specified RTCP session.
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtcpGetSSRC(
                IN  RvRtcpSession  hRTCP)
{
    rtcpSession *s = (rtcpSession *)hRTCP;

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(!s) {
       return RV_Failure;
    }
#endif
    //SPIRENT_END

    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpGetSSRC"));
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpGetSSRC"));
    return s->myInfo.ssrc;
}


/************************************************************************************
 * RvRtcpSetSSRC
 * description: Changes the synchronization source value for an RTCP session.
 *              If a new SSRC was regenerated after collision,
 *              call this function to notify RTCP about the new SSRC.
 * input: hRTCP  - The handle of the RTCP session.
 *        ssrc   - A synchronization source value for the RTCP session.
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/

RVAPI
RvInt32 RVCALLCONV RvRtcpSetSSRC(
                IN  RvRtcpSession  hRTCP,
                IN  RvUint32      ssrc)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSetSSRC"));

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(!s) {
       return RV_Failure;
    }
#endif
    //SPIRENT_END

    s->myInfo.ssrc      = ssrc;
    s->myInfo.collision = 0;
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetSSRC"));
    return RV_OK;
}

                 /* == ENDS: Accessory RTCP Functions == */



                     /* == Internal RTCP Functions == */

/************************************************************************************
 * RvRtcpGetEnumFirst
 * description: obtains the SSRC of the first valid session participant
 * input: hRTCP  - The handle of the RTCP session.
 * output: ssrc   - pointer to a synchronization source value of the first session participant
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value
 *               (first session participant index)
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtcpGetEnumFirst(
                IN  RvRtcpSession  hRTCP,
                IN  RvInt32 *     ssrc)
{
    return RvRtcpGetEnumNext(hRTCP, -1, ssrc);
}
/************************************************************************************
 * RvRtcpGetEnumNext
 * description: obtains the SSRC of the next session participant
 * input: hRTCP  - The handle of the RTCP session.
 *        prev   - index of the current participant
 * output: ssrc   - pointer to a synchronization source value of the next session participant
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value
 *               (next session participant index)
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtcpGetEnumNext(
                IN  RvRtcpSession  hRTCP,
                IN  RvInt32       prev,
                IN  RvInt32 *     ssrc)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    rtcpInfo* info;
    RvInt32 index, doAgain = 1;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpGetEnumNext"));

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(!s) {
       return RV_Failure;
    }
#endif
    //SPIRENT_END

    if (prev < 0)
        index = 0;
    else
        index = prev+1;

    while ((doAgain == 1)&&(index < s->sessionMembers))
    {
        info = &s->participantsArray[index];
        if (!info->invalid)
        {
            doAgain = 0;
            *ssrc = info->ssrc;
        }
        else
        {
            index++;
        }
    }
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpGetEnumNext"));
    if (index < s->sessionMembers)
        return index;
    else
        return -1;
}

/* todo: check reference guide about this function */
RVAPI
RvInt32 RVCALLCONV rtcpCreateRTCPPacket(
                IN      RvRtcpSession  hRTCP,
                IN OUT  RvRtpBuffer*  buf)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    rtcpHeader head;
    RvUint32 allocated = 0;
    RvRtpBuffer bufC;
    RvStatus res = RV_OK, senderResult = RV_ERROR_BADPARAM;

    RvLogEnter(rvLogPtr, (rvLogPtr, "rtcpCreateRTCPPacket"));

    if (s == NULL)
    {
        RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendSR: NULL RTCP session pointer"));				
        RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendReport"));
        return RV_ERROR_NULLPTR;   
    }

    RvLockGet(&s->lock, logMgr);
    senderResult = rtcpAddRTCPPacketType(hRTCP, RTCP_SR, 0, 0, NULL, 0, buf, &allocated);
    if (senderResult == RV_ERROR_BADPARAM)  /* sender report inside contains receiver report data too */

        res = rtcpAddRTCPPacketType(hRTCP, RTCP_RR, 0, 0, NULL, 0, buf, &allocated);
    RvLockRelease(&s->lock, logMgr);

    rtcpAddRTCPPacketType(hRTCP, RTCP_SDES, 1/* set inside*/, 0, NULL, 0, buf, &allocated);

    if (s->myInfo.collision == 1  &&
        buffValid(buf, allocated + SIZEOF_RTCPHEADER))
    {
        head = makeHeader(s->myInfo.ssrc, 1, RTCP_BYE, SIZEOF_RTCPHEADER);

        bufC = buffCreate(&head, SIZEOF_RTCPHEADER);
        buffAddToBuffer(buf, &bufC, allocated);
        s->myInfo.collision = 2;
        allocated += SIZEOF_RTCPHEADER;
    }
    buf->length = allocated;
    RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpCreateRTCPPacket"));
	RV_UNUSED_ARG(res);
    return RV_OK;
}

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)

/* todo: check reference guide about this function */
RVAPI
RvInt32 RVCALLCONV rtcpProcessCompoundRTCPPacket(
        IN      RvRtcpSession  hRTCP,
        IN OUT  RvRtpBuffer*  buf,
        IN      RvUint64      myTime)
{
   if(buf && buf->buffer && buf->length && hRTCP) {
      
      rtcpSession *s = (rtcpSession *)hRTCP;
      rtcpHeader *head;
      RvUint8 *currPtr = buf->buffer, *dataPtr, *compoundEnd;
      RvInt32 hdr_count, hdr_len;
      rtcpType hdr_type;
      RvBool   hdr_paddingbit = RV_FALSE;
      RvUint32 Padding = 0;
      
      RTPLOG_ENTER(ProcessCompoundRTCPPacket);
      compoundEnd = buf->buffer + buf->length;
      
      while (
         sizeof(RvUint32) < buf->length &&
         currPtr &&
         buf->buffer <= currPtr &&
         currPtr <= buf->buffer + buf->length - sizeof(RvUint32) &&
         compoundEnd && 
         buf->buffer < compoundEnd &&
         compoundEnd <= buf->buffer + buf->length &&
         currPtr < compoundEnd - sizeof(RvUint32)
         )
      {
         head = (rtcpHeader*)(currPtr);

         ConvertFromNetwork(currPtr, 0, 1);
         
         hdr_count = bitfieldGet(head->bits, HEADER_RC, HDR_LEN_RC);
         hdr_type  = (rtcpType)bitfieldGet(head->bits, HEADER_PT, HDR_LEN_PT);
         hdr_paddingbit = bitfieldGet(head->bits, HEADER_P, 1);
         if (hdr_paddingbit)
         {
            Padding = *(compoundEnd-1);
            compoundEnd = buf->buffer + buf->length - Padding;
         }
         hdr_len   = sizeof(RvUint32) *
            (bitfieldGet(head->bits, HEADER_len, HDR_LEN_len)) - Padding;

         if ((compoundEnd - currPtr) < hdr_len)
         {
            RTPLOG_ERROR_LEAVE(ProcessCompoundRTCPPacket, "Illegal RTCP packet length");
            //dprintf_T("%s:%d:ERR_RTCP_ILLEGALPACKET:%d:%d\n",__FUNCTION__,__LINE__,(compoundEnd - currPtr),hdr_len);
            return ERR_RTCP_ILLEGALPACKET;
         }
         if ((hdr_type > RTCP_LAST_LEGAL && hdr_type < RTCP_SR) || hdr_len>1400 || hdr_len<0)
         {
            RTPLOG_ERROR_LEAVE(ProcessCompoundRTCPPacket, "Illegal RTCP packet parameter header type or data length");
            //dprintf_T("%s:%d:ERR_RTCP_ILLEGALPACKET:%d:%d\n",__FUNCTION__,__LINE__,hdr_type,hdr_len);
            return ERR_RTCP_ILLEGALPACKET;
         }
         dataPtr = (RvUint8 *)head + sizeof(RvUint32);
         
         rtcpProcessRTCPPacket(s, dataPtr, hdr_len, hdr_type, hdr_count, myTime);
         
         currPtr += (hdr_len + sizeof(RvUint32));
      }
      
   }
   RTPLOG_LEAVE(ProcessCompoundRTCPPacket);
   return RV_OK;
}

#else

/* todo: check reference guide about this function */
RVAPI
RvInt32 RVCALLCONV rtcpProcessCompoundRTCPPacket(
        IN      RvRtcpSession  hRTCP,
        IN OUT  RvRtpBuffer*  buf,
        IN      RvUint64      myTime)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    rtcpHeader *head;
    RvUint8 *currPtr = buf->buffer, *dataPtr, *compoundEnd;
    RvInt32 hdr_count, hdr_len;
    rtcpType hdr_type;
    RvBool   hdr_paddingbit = RV_FALSE;
    RvUint32 Padding = 0;

    RTPLOG_ENTER(ProcessCompoundRTCPPacket);
    compoundEnd = buf->buffer + buf->length;

    while (currPtr < compoundEnd)
    {
        head = (rtcpHeader*)(currPtr);
        ConvertFromNetwork(currPtr, 0, 1);

        hdr_count = bitfieldGet(head->bits, HEADER_RC, HDR_LEN_RC);
        hdr_type  = (rtcpType)bitfieldGet(head->bits, HEADER_PT, HDR_LEN_PT);
        hdr_paddingbit = bitfieldGet(head->bits, HEADER_P, 1);
        if (hdr_paddingbit)
        {
           Padding = *(compoundEnd-1);
           compoundEnd = buf->buffer + buf->length - Padding;
        }
        hdr_len   = sizeof(RvUint32) *
                    (bitfieldGet(head->bits, HEADER_len, HDR_LEN_len)) - Padding;
        if ((compoundEnd - currPtr) < hdr_len)
        {
            RTPLOG_ERROR_LEAVE(ProcessCompoundRTCPPacket, "Illegal RTCP packet length");
            return ERR_RTCP_ILLEGALPACKET;
        }
        if ((hdr_type > RTCP_LAST_LEGAL && hdr_type < RTCP_SR) || (hdr_len>1400 && hdr_len<0))
        {
            RTPLOG_ERROR_LEAVE(ProcessCompoundRTCPPacket, "Illegal RTCP packet parameter header type or data length");
            return ERR_RTCP_ILLEGALPACKET;
        }
        dataPtr = (RvUint8 *)head + sizeof(RvUint32);

        rtcpProcessRTCPPacket(s, dataPtr, hdr_len, hdr_type, hdr_count, myTime);

        currPtr += (hdr_len + sizeof(RvUint32));
    }

    RTPLOG_LEAVE(ProcessCompoundRTCPPacket);
    return RV_OK;
}

#endif
//SPIRENT_END

RVAPI
RvInt32 RVCALLCONV rtcpProcessRTCPPacket(
        IN  rtcpSession *  s,
        IN  RvUint8 *         data,
        IN  RvInt32        dataLen,
        IN  rtcpType       type,
        IN  RvInt32        reportCount,
        IN  RvUint64       myTime)
{
    unsigned scanned = 0;
    rtcpInfo info, *fInfo=NULL;
    info.ssrc = 0;

    RvLogEnter(rvLogPtr, (rvLogPtr, "rtcpProcessRTCPPacket"));
    if (dataLen == 0)
        return RV_OK;

    switch(type)
    {
        case RTCP_SR:
        case RTCP_RR:
        case RTCP_APP:
        {
            ConvertFromNetwork(data, 0, 1);
            info.ssrc = *(RvUint32 *)(data);
            scanned = sizeof(RvUint32);

            RvLockGet(&s->lock, logMgr);
            if (info.ssrc == s->myInfo.ssrc)
            {
                s->myInfo.collision = 1;
                RvLockRelease(&s->lock, logMgr);
                return ERR_RTCP_SSRCCOLLISION;
            }
                
            fInfo = findSSrc(s,info.ssrc);
            if (!fInfo) /* New source */
            {
                /* insert the new source */
                fInfo = insertNewSSRC(s, *(RvUint32 *)data);
            }
            RvLockRelease(&s->lock, logMgr);
            break;
        }

        default:
            break;
    }

    /* process the information */
    switch(type)
    {
        case RTCP_SR:
        {
            ConvertFromNetwork(data + scanned, 0, W32Len(SIZEOF_SR));

            if (fInfo)
            {
                RvLockGet(&s->lock, logMgr);
/*                fInfo->eSR        = *(rtcpSR *)(data + scanned);*/
                memcpy(&(fInfo->eSR.tNNTP), (data + scanned), 8);
                memcpy(&(fInfo->eSR.tRTP),  (data + scanned + 8), 4);
                memcpy(&(fInfo->eSR.nPackets),  (data + scanned + 12), 4);
                memcpy(&(fInfo->eSR.nBytes),  (data + scanned + 16), 4);

                fInfo->eToRR.tLSR = reduceNNTP(fInfo->eSR.tNNTP);
                fInfo->tLSRmyTime = reduceNNTP(myTime);
                RvLockRelease(&s->lock, logMgr);
            }

            scanned += SIZEOF_SR;
        }

        /* fall into RR */

        case RTCP_RR:
        {
            if ((fInfo != NULL) && ((RvInt32)scanned < dataLen))
            {
#if 0                
                RvInt32 i;
                rtcpRR* rr = (rtcpRR*)(data + scanned);

                ConvertFromNetwork(data + scanned, 0,
                    reportCount * W32Len(sizeof(rtcpRR)));

                RvLockGet(&s->lock, logMgr);
                for (i=0; i < reportCount; i++)
                {
                    if (rr[i].ssrc == s->myInfo.ssrc)
                    {
                        fInfo->eFromRR = rr[i];
                        break;
                    }
                }
                RvLockRelease(&s->lock, logMgr);
#else
                RvInt32 i;
                rtcpRR  rr;

                RvLockGet(&s->lock, logMgr);
                for (i=0; i < reportCount; i++)
                {
                    memcpy(&rr , data + scanned, sizeof(int));
                    ConvertFromNetwork(&rr, 0, W32Len(sizeof(int)));
                    if (rr.ssrc == s->myInfo.ssrc)
                    {
                        fInfo->eFromRR.ssrc = rr.ssrc;
                        memcpy(&fInfo->eFromRR.bfLost, data + scanned + 4, sizeof(rtcpRR)-4);
                        ConvertFromNetwork(&fInfo->eFromRR.bfLost, 0, W32Len(sizeof(rtcpRR))-1);
                        break;
                    }
                    scanned += sizeof(rtcpRR);
                }
                RvLockRelease(&s->lock, logMgr);
#endif
            }

            break;
        }

        case RTCP_SDES:
            {
                RvInt32 i;
                rtcpSDES *sdes;

                for (i = 0; (i < reportCount) && ((RvInt32)scanned < dataLen); i++)
                {
                    ConvertFromNetwork(data + scanned, 0, 1);
                    info.ssrc = *(RvUint32 *)(data + scanned);

                    fInfo = findSSrc(s,info.ssrc);

                    sdes = (rtcpSDES *)(data + scanned + sizeof(info.ssrc));

                    if (fInfo != NULL)
                    {
                        scanned += sizeof(RvUint32);
                        do
                        {
                            switch(sdes->type)
                            {
                            default:
                                /* error */
                                break;
                            case RV_RTCP_SDES_CNAME:
                                memcpy(&(fInfo->eSDESCname), sdes, (RvSize_t)(sdes->length+2));
                                fInfo->eSDESCname.value[sdes->length] = 0;
                            break;
                            case RV_RTCP_SDES_NAME:
                            case RV_RTCP_SDES_EMAIL:
                            case RV_RTCP_SDES_PHONE:
                            case RV_RTCP_SDES_LOC:
                            case RV_RTCP_SDES_TOOL:
                            case RV_RTCP_SDES_NOTE:
                            case RV_RTCP_SDES_PRIV:
								if (NULL!=s->rtcpSDESMessageArrivedCallback)
								{
								   RvUint8 SDESstring[256];
								   memcpy(SDESstring, sdes->value, sdes->length);
                                   SDESstring[sdes->length] = 0;
								   RvLogDebug(rvLogPtr, (rvLogPtr, "rtcpProcessRTCPPacket: Optional SDES packet received: SSRC"
									                               ":%#x, Type =%d, string:'%s'", info.ssrc, sdes->type, sdes->value));
								   s->rtcpSDESMessageArrivedCallback((RvRtcpSession)s,	sdes->type,	info.ssrc, (RvUint8*)sdes->value, (RvUint32)sdes->length);
								}
                                break;
                            }
                            scanned += (sdes->length+2);
                            sdes = (rtcpSDES *)(data + scanned);
                        }
                        while (*(RvUint8 *)(data + scanned) != 0 && ((RvInt32)scanned < dataLen));
                    }
                }
                break;
            }
        case RTCP_BYE:
        {
            RvInt32 i;
            ConvertFromNetwork(data, 0, 1);
            info.ssrc = *(RvUint32 *)(data);
            scanned = sizeof(RvUint32);
            for (i = 0; i < reportCount; i++)
            {
                ConvertFromNetwork(data + scanned, 0, 1);
                info.ssrc = *(RvUint32 *)(data + scanned);
                scanned += sizeof(info.ssrc);

                fInfo = findSSrc(s,info.ssrc);

                if ((fInfo != NULL) )
                {
                    /* We don't really delete this SSRC, we just mark it as invalid */
                    /* Inform user about all SSRC/CSRC in BYE message
                    reason will be transfered to user only with sender SSRC (last callback)
                    s->rtcpByeRecvCallback((RvRtcpSession)s, s->rtcpByeContext, info.ssrc, NULL, 0);
                    */
                    RvLockGet(&s->lock, logMgr);
                    fInfo->invalid = RV_TRUE;
                    fInfo->ssrc    = 0;
                    RvLockRelease(&s->lock, logMgr);
                    /* number of session members is still remained unchanged */
                }
            }
            if ((RvInt32)scanned < dataLen)
            {
                RvUint8 Length = *(RvUint8 *) (data + scanned);
                scanned++;
                if ((RvInt32)scanned < dataLen)
                {
                    if (s->rtcpByeRecvCallback!=NULL)
                         s->rtcpByeRecvCallback((RvRtcpSession)s, s->rtcpByeContext, info.ssrc, (RvUint8 *)(data + scanned), Length);
                    scanned +=Length;
                }
            }
            else {/* BYE without reason */
                if (s->rtcpByeRecvCallback!=NULL)
                    s->rtcpByeRecvCallback((RvRtcpSession)s, s->rtcpByeContext, info.ssrc, NULL, 0);
            }
            break;
        }

        case RTCP_APP:
        {
            if ((fInfo != NULL) && ((RvInt32)scanned < dataLen))
            {
#if defined(RTP_CFG_APP_ALL_USER_DATA_UINT32)
                ConvertFromNetwork(data + scanned, 0, W32Len(dataLen-scanned));
#elif  defined(RTP_CFG_APP_NAME_IS_UINT32)
                ConvertFromNetwork(data + scanned, 0, 1);
#endif
                if (NULL!= s->rtcpAppRecvCallback)
                   s->rtcpAppRecvCallback((RvRtcpSession)s, s->rtcpAppContext, (RvUint8) reportCount, info.ssrc,
                    (RvUint8 *)(data + scanned),(RvUint8*)(data + scanned + 4), (dataLen-scanned-4));
                scanned += (dataLen-scanned);
            }
        }
            break;
        case RTCP_EMPTY_RR: /* no such report, produced in RTCP_RR */
        break;
    }

    RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpProcessRTCPPacket"));
    return RV_OK;
}

static RvUint32 rtcpGetEstimatedRTPTime(IN RvRtcpSession  hRTCP,
                                        IN const RvUint64 *ntpNewTimestampPtr)
{

    rtcpSession *s = (rtcpSession *)hRTCP;
    RvUint32   estimatedTime = s->myInfo.lastPackRTPts;
    RvLogEnter(rvLogPtr,(rvLogPtr, "rtcpDatabaseGetEstimatedRTPTime"));

    /* Generate estimate, if we have sent at least one RTP packet */
/*    if(RvNtpTimeGetSecs(&thisPtr->ntpTimestamp) != 0)*/
    if ( s->myInfo.lastPackNTP != RvUint64Const(0,0))
    {
        RvUint64  ntpDelta;
        RvUint32  ntpDeltaReduced;
        if (s->myInfo.rtpClockRate == 0)
            s->myInfo.rtpClockRate = RvRtpGetStandardPayloadClockRate(s->myInfo.rtpPayloadType);

        /* Compute the NTP Delta */
        ntpDelta = RvInt64Sub(*ntpNewTimestampPtr, s->myInfo.lastPackNTP);

        ntpDeltaReduced = RvUint64ToRvUint32(
            RvInt64ShiftRight(RvInt64ShiftRight(RvInt64ShiftLeft(ntpDelta, 16), 16), 16));
        /* 2 ShiftsRight 16 removes warning in VxWorks
         was RvInt64ShiftRight(RvInt64ShiftLeft(ntpDelta, 16), 32));*/
        /* Compute the estimate with rounding (ntpDeltaReduced is int fixed point 16.16) */
        estimatedTime += (((s->myInfo.rtpClockRate * ntpDeltaReduced)  + 0x8000) >> 16);
    }

    RvLogLeave(rvLogPtr,(rvLogPtr, "rtcpDatabaseGetEstimatedRTPTime"));

    return estimatedTime;
}

/********************************************************************************
 * RFC 3550
 ********************************************************************************/
/*
  6.7 APP: Application-Defined RTCP Packet

      0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |V=2|P| subtype |   PT=APP=204  |             length            |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                           SSRC/CSRC                           |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                          name (ASCII)                         |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                   application-dependent data                ...
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

      The APP packet is intended for experimental use as new applications
      and new features are developed, without requiring packet type value
      registration.  APP packets with unrecognized names SHOULD be ignored.
      After testing and if wider use is justified, it is RECOMMENDED that
      each APP packet be redefined without the subtype and name fields and
      registered with IANA using an RTCP packet type.

      version (V), padding (P), length:
      As described for the SR packet (see Section 6.4.1).

      subtype: 5 bits
             May be used as a subtype to allow a set of APP packets to be
             defined under one unique name, or for any application-dependent
             data.

             packet type (PT): 8 bits
             Contains the constant 204 to identify this as an RTCP APP packet.

      name: 4 octets
            A name chosen by the person defining the set of APP packets to be
            unique with respect to other APP packets this application might
            receive.  The application creator might choose to use the
            application name, and then coordinate the allocation of subtype
            values to others who want to define new packet types for the
            application.  Alternatively, it is RECOMMENDED that others choose
            a name based on the entity they represent, then coordinate the use
            of the name within that entity.  The name is interpreted as a
            sequence of four ASCII characters, with uppercase and lowercase
            characters treated as distinct.

          application-dependent data: variable length
            Application-dependent data may or may not appear in an APP packet.
            It is interpreted by the application and not RTP itself.  It MUST
            be a multiple of 32 bits long.
          */
static RvStatus rtcpAddRTCPPacketType(IN RvRtcpSession  hRTCP,
                            IN rtcpType type,
                            IN RvUint8 subtype,
                            IN RvUint8* name,
                            IN void *userData, /* application-dependent data  */
                            IN RvUint32 userDataLength,
                            IN OUT  RvRtpBuffer*  buf,
                            IN OUT RvUint32* pCurrentIndex)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    rtcpHeader head;
    RvUint32 allocated = *pCurrentIndex;
    RvRtpBuffer bufC;

    RvLogEnter(rvLogPtr, (rvLogPtr, "rtcpAddRTCPPacketType"));
    switch (type)
    {
    case RTCP_APP:
        {
            if (buffValid(buf,
                allocated + SIZEOF_RTCPHEADER + 4 + userDataLength))
            {
                /* Header code */
                if (NULL!=userData &&userDataLength>0)
                    head = makeHeader(s->myInfo.ssrc, subtype, type, (RvUint16) (SIZEOF_RTCPHEADER + 4 + userDataLength)); /* sizeof(Packet) in 4 octet words - 1 */
                else
                    head = makeHeader(s->myInfo.ssrc, subtype, type, (RvUint16) (SIZEOF_RTCPHEADER + 4)); /* sizeof(Packet) in 4 octet words - 1 */

                bufC = buffCreate(&head, SIZEOF_RTCPHEADER);
                buffAddToBuffer(buf, &bufC, allocated);
                allocated += bufC.length;
                bufC = buffCreate(name, 4);
                buffAddToBuffer(buf, &bufC, allocated);
#if defined(RTP_CFG_APP_NAME_IS_UINT32)||defined(RTP_CFG_APP_ALL_USER_DATA_UINT32)
                ConvertToNetwork(buf->buffer + allocated, 0, W32Len(bufC.length));
#endif
                allocated += bufC.length;
                if (NULL!=userData &&userDataLength>0)
                {
                    bufC = buffCreate(userData, userDataLength);
                    buffAddToBuffer(buf, &bufC, allocated);
#if defined(RTP_CFG_APP_ALL_USER_DATA_UINT32)
                    ConvertToNetwork(buf->buffer + allocated, 0, W32Len(bufC.length));
#endif
                    allocated += bufC.length;
                }
            }
            else
			{
                RvLogError(rvLogPtr, (rvLogPtr, "rtcpAddRTCPPacketType:RTCP_APP out of resources error"));				
                RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpAddRTCPPacketType"));
                return RV_ERROR_OUTOFRESOURCES;
			}
            break;
        }
    case RTCP_SDES:
        {
            RvInt32 Count = 1, Len = 0;
            for (Count = 1; Count <= RV_RTCP_SDES_PRIV;Count++)
                if (s->myInfo.SDESbank[Count].length)
                    Len += (s->myInfo.SDESbank[Count].length + 2);
            /* +1 for for SDES type  */
            /* +1 for for SDES length  */

            Len = (Len+4) & 0xFFFFFFFC;
            /* +1 for "NULL" termination of data items chunk */
            /* and Padding  at the end of section */

            if (buffValid(buf,
                allocated + SIZEOF_RTCPHEADER + Len))
            {
                RvInt32 Count = 0;
                RvRtpBuffer sdes_buf;
                /* 'sdes_buf' is inside the compound buffer 'buf' */
                sdes_buf = buffCreate(buf->buffer + allocated, (SIZEOF_RTCPHEADER + Len));

                head = makeHeader(s->myInfo.ssrc, 1, RTCP_SDES, (RvUint16)sdes_buf.length);
                bufC = buffCreate(&head, SIZEOF_RTCPHEADER);
                buffAddToBuffer(buf, &bufC, allocated);
                allocated += SIZEOF_RTCPHEADER;

                for (Count = 1; Count <= RV_RTCP_SDES_PRIV;Count++)
                {
                    if (s->myInfo.SDESbank[Count].length)
                    {
                        bufC = buffCreate(&(s->myInfo.SDESbank[Count]), s->myInfo.SDESbank[Count].length+2);
                        buffAddToBuffer(buf, &bufC, allocated);
                        allocated += bufC.length;
                    }
                }
                *(RvUint8*)(buf->buffer + allocated) = '\0';
                allocated++;
                allocated += (*pCurrentIndex + sdes_buf.length - allocated); /* padding */
            }
            else
			{
                RvLogError(rvLogPtr, (rvLogPtr, "rtcpAddRTCPPacketType:RTCP_SDES out of resources error"));				
                RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpAddRTCPPacketType"));
                return RV_ERROR_OUTOFRESOURCES;
			}
            break;
        }
    case RTCP_SR:
        {
            RvInt32            index;
            s->myInfo.eSR.tNNTP = getNNTPTime();
            s->myInfo.eSR.tRTP =  rtcpGetEstimatedRTPTime(hRTCP, &s->myInfo.eSR.tNNTP);

            /* time of sending the report and not time of sending the last RTP packet   */
            if (buffValid(buf, SIZEOF_RTCPHEADER + SIZEOF_SR))
            {
                RvUint64 myTime = s->myInfo.eSR.tNNTP;
                RvUint8 cc = 0;
                rtcpInfo *info;

                allocated += SIZEOF_RTCPHEADER;
                /* RTCP Header setting later */
                if (s->myInfo.active==2)
                {
#ifdef UPDATED_BY_SPIRENT
                    ///dfb: byte order is considered in RvConvertHostToNetwork64.

                    RvUint64 tNNTP_net = RvConvertHostToNetwork64(s->myInfo.eSR.tNNTP);
                    s->myInfo.active --;
                    bufC = buffCreate(&tNNTP_net, sizeof(RvUint64));
                    buffAddToBuffer(buf, &bufC, allocated);
                    allocated += sizeof(RvUint64);
#else
                    s->myInfo.active --;
                    bufC = buffCreate(&(s->myInfo.eSR.tNNTP), sizeof(RvInt64));
                    buffAddToBuffer(buf, &bufC, allocated);
                    ConvertToNetwork(buf->buffer + allocated, 0, W32Len(bufC.length));
                    allocated += sizeof(RvInt64);
#endif
                    bufC = buffCreate(&(s->myInfo.eSR.tRTP), sizeof(RvUint32)*3);
                    buffAddToBuffer(buf, &bufC, allocated);
                    ConvertToNetwork(buf->buffer + allocated, 0, W32Len(bufC.length));
                    allocated += sizeof(RvUint32)*3;
                }
                else
                {
                    s->myInfo.active--;
                    allocated -= SIZEOF_RTCPHEADER;
					RvLogWarning(rvLogPtr, (rvLogPtr, "rtcpAddRTCPPacketType: SR is not issued, because no RTP data were sent since previous SR."));				
					RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpAddRTCPPacketType"));
                    return RV_ERROR_BADPARAM; /* cannot send sender report no new sent packets */
                }
                index = 0;

                while( index < s->sessionMembers && cc < 31)
                {
                    info = &s->participantsArray[index];
                    if ((!info->invalid)&&info->active)
                    {
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
                        info->eToRR.rxPacketsSLR   = getReceivedPackets (&(info->src));
/* SPIRENT END */
#endif
                        info->eToRR.bfLost     = getLost    (&(info->src));
                        info->eToRR.nJitter    = getJitter  (&(info->src));
                        info->eToRR.nExtMaxSeq = getSequence(&(info->src));
                        info->eToRR.tDLSR      = /* time of sending - time of receiving previous report*/
                            (info->tLSRmyTime) ?
                            (reduceNNTP(myTime)-info->tLSRmyTime) : 0;

                        bufC = buffCreate(&(info->eToRR), SIZEOF_RR);

                        if (buffAddToBuffer(buf, &bufC, allocated))
                        {
                            cc++;
                            ConvertToNetwork(buf->buffer + allocated, 0, W32Len(bufC.length));
                            allocated += SIZEOF_RR;
                        }
                        info->active = RV_FALSE;
                    }
                    index++;
                }
                head = makeHeader(s->myInfo.ssrc, cc, type, (RvUint16)(allocated - *pCurrentIndex));
                bufC = buffCreate(&head, SIZEOF_RTCPHEADER);
                buffAddToBuffer(buf, &bufC, *pCurrentIndex);
            }
			else
			{
                RvLogError(rvLogPtr, (rvLogPtr, "rtcpAddRTCPPacketType:RTCP_SR out of resources error"));				
                RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpAddRTCPPacketType"));
                return RV_ERROR_OUTOFRESOURCES;
			}
        }
        break;
    case RTCP_RR:
        {
            RvInt32            index;

            if (buffValid(buf, SIZEOF_RTCPHEADER + SIZEOF_RR))
            {
                RvUint64 myTime = getNNTPTime();
                RvUint8 cc = 0;
                rtcpInfo *info;

                allocated += SIZEOF_RTCPHEADER;
                /* RTCP Header setting later */
                index = 0;

                while( index < s->sessionMembers && cc < 31)
                {
                    info = &s->participantsArray[index];
                    if ((!info->invalid)&&info->active)
                    {
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
                        info->eToRR.rxPacketsSLR   = getReceivedPackets (&(info->src));
/* SPIRENT END */
#endif
                        info->eToRR.bfLost     = getLost    (&(info->src));
                        info->eToRR.nJitter    = getJitter  (&(info->src));
                        info->eToRR.nExtMaxSeq = getSequence(&(info->src));
                        info->eToRR.tDLSR      =
                            (info->tLSRmyTime) ?
                            (reduceNNTP(myTime)-info->tLSRmyTime) :
                        0;

                        bufC = buffCreate(&(info->eToRR), SIZEOF_RR);

                        if (buffAddToBuffer(buf, &bufC, allocated))
                        {
                            cc++;
                            ConvertToNetwork(buf->buffer + allocated, 0, W32Len(bufC.length));
                            allocated += SIZEOF_RR;
                        }
                        info->active = RV_FALSE;
                    }
                    index++;
                }
                head = makeHeader(s->myInfo.ssrc, cc, type, (RvUint16)(allocated - *pCurrentIndex));
                bufC = buffCreate(&head, SIZEOF_RTCPHEADER);
                buffAddToBuffer(buf, &bufC, *pCurrentIndex);
            }
			else
			{
                RvLogError(rvLogPtr, (rvLogPtr, "rtcpAddRTCPPacketType:RTCP_RR out of resources error"));				
                RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpAddRTCPPacketType"));
                return RV_ERROR_OUTOFRESOURCES;
			}
        }
        break;
    case RTCP_BYE:
        if (buffValid(buf, allocated + SIZEOF_RTCPHEADER + userDataLength+(RTCPHEADER_SSRCMAXCOUNT-1)*4))
        {
            RvUint8 Padding[3] = {0,0,0};
            RvUint8 reasonLen =  (RvUint8) userDataLength;
            RvInt32       index = 0; /* 0 is s->myInfo.ssrc */
            RvInt32      ssrcCount = 0;
            RvInt32      ssrcSize = 0;
            rtcpInfo* info = NULL;
            /* search for all active members */
            while( index < s->sessionMembers)
            {
                info = &s->participantsArray[index];
                if (info->active) /* or validity check ? */
                     ssrcCount++;
                index++;
            }

	    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
	    if(ssrcCount<1) return RV_OK;
#endif
	    //SPIRENT_END

            if (ssrcCount> RTCPHEADER_SSRCMAXCOUNT)
                ssrcCount = RTCPHEADER_SSRCMAXCOUNT;

            ssrcSize = (ssrcCount-1)*sizeof(RvUint32); /* not included own SSRC */

            if (NULL!=userData && userDataLength>0)
                head = makeHeader(s->myInfo.ssrc, (RvUint8)ssrcCount, RTCP_BYE, (RvUint16) (SIZEOF_RTCPHEADER + ssrcSize + userDataLength + 1));
            else
                head = makeHeader(s->myInfo.ssrc, (RvUint8)ssrcCount, RTCP_BYE, (RvUint16)(SIZEOF_RTCPHEADER + ssrcSize));

            bufC = buffCreate(&head, SIZEOF_RTCPHEADER);
            buffAddToBuffer(buf, &bufC, allocated);
/*            s->myInfo.collision = 2;*/
            allocated += bufC.length;
            index = 1;
            while( index < s->sessionMembers && index < ssrcCount)
            {
                info = &s->participantsArray[index];
                if (info->active) /* or validity check ? */
                {
                    RvUint32 SSRC = info->ssrc;
                    ConvertToNetwork(&SSRC, 0, 1);
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
                    bufC = buffCreate(&SSRC, sizeof(SSRC));
#else
                    bufC = buffCreate(&info->ssrc, 4);
#endif
//SPIRENT_END   
                    buffAddToBuffer(buf, &bufC, allocated);
                    allocated += bufC.length;
                }
                index++;
            }
            /* adding the reason length */
            if (reasonLen>0)
            {
                bufC = buffCreate(&reasonLen, 1);
                buffAddToBuffer(buf, &bufC, allocated);
                allocated += bufC.length;
                /* adding the reason */
                bufC = buffCreate(userData, userDataLength);
                buffAddToBuffer(buf, &bufC, allocated);
                allocated += bufC.length;
                if ((userDataLength+1)&0x00000003) /* must be filled by NULL pudding */
                {
                    RvInt32 PaddingSize = 4 - ((userDataLength+1)&0x00000003);
                    bufC = buffCreate(Padding, PaddingSize);
                    buffAddToBuffer(buf, &bufC, allocated);
                    allocated += bufC.length;
                }
            }
        }
		else
		{
			RvLogError(rvLogPtr, (rvLogPtr, "rtcpAddRTCPPacketType:RTCP_BYE out of resources error"));				
			RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpAddRTCPPacketType"));
			return RV_ERROR_OUTOFRESOURCES;
		}	
        break;
    case RTCP_EMPTY_RR:
        {
            if (buffValid(buf, SIZEOF_RTCPHEADER + SIZEOF_RR))
            {
                /* no receiver reports incide */
                head = makeHeader(s->myInfo.ssrc, 0, RTCP_RR, (RvUint16)(SIZEOF_RTCPHEADER));
                bufC = buffCreate(&head, SIZEOF_RTCPHEADER);
                buffAddToBuffer(buf, &bufC, allocated);
                allocated += SIZEOF_RTCPHEADER;
            }
			else
			{
				RvLogError(rvLogPtr, (rvLogPtr, "rtcpAddRTCPPacketType:RTCP_EMPTY_RR out of resources error"));				
				RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpAddRTCPPacketType"));
				return RV_ERROR_OUTOFRESOURCES;
			}
        }
        break;
    default:
        break;
    }
    *pCurrentIndex = allocated;
    RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpAddRTCPPacketType"));
    return RV_OK;
}
/*********************************************************************************************
 *  RvRtcpSessionSendApps
 * description: sends manual compound APP RTCP report.
 * input: hRTCP
 *           - The handle of the RTCP session.
 *        appMessageTable
 *           - pointer to APP messages table (to be sent)
 *        appMessageTableEntriesNum - number of messages in appMessageTable
 *        bCompound -to send compound report (ALL SDES + empty RR + APP)
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value
 *  Remark:
 * This function will not affect the normal periodic RTCP messages and is not
 *  included in any statistical calculations, including bandwidth limitations.
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvRtcpSessionSendApps(IN RvRtcpSession hRTCP,
                                          IN RvRtcpAppMessage* appMessageTable,
                                          IN RvInt32 appMessageTableEntriesNum,
                                          IN RvBool bCompound)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvUint32 allocated = 0;
    RvUint32 data[MAXRTCPPACKET/sizeof(RvUint32)+1];
    RvRtpBuffer buf;
    RvStatus res = RV_OK;

    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendApps"));

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    RvLockGet(&s->lock, logMgr);
#endif
    //SPIRENT_END

    buf = buffCreate(data+1, MAXRTCPPACKET-4); /* resulting packet */

    if (bCompound)
    {
        res = rtcpAddRTCPPacketType(hRTCP, RTCP_EMPTY_RR, 0, 0, NULL, 0, &buf, &allocated);
        res = rtcpAddRTCPPacketType(hRTCP, RTCP_SDES, 1/* set inside*/, 0, NULL, 0, &buf, &allocated);
    }
    if ((appMessageTableEntriesNum>0) &&(appMessageTable!=NULL))
    {
        RvInt32 Count = 0;
        for (Count = 0;Count < appMessageTableEntriesNum;Count++)
            if (appMessageTable[Count].userData!=NULL)
                rtcpAddRTCPPacketType(hRTCP, RTCP_APP, appMessageTable[Count].subtype,
                    appMessageTable[Count].name, appMessageTable[Count].userData,
                    appMessageTable[Count].userDataLength, &buf, &allocated);
    }
/*  return RvSocketSendBuffer(&s->socket, buf.buffer, (RvSize_t)allocated, &s->remoteAddress, logMgr, NULL);*/
    res = rtcpSessionRawSend(hRTCP, buf.buffer, allocated, MAXRTCPPACKET, NULL);
    if (res >= 0)
    {
       RvLogInfo(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendApps - Manual RTCP APP message was sent"));
    }
    else
    {
       RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendApps - failed to send RTCP APP message, error=%d", res));
    }

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    RvLockRelease(&s->lock, logMgr);
#endif
    //SPIRENT_END

    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendApps"));
    return res;
}

/*********************************************************************************************
 *	RvRtcpSessionSendSR 
 * description: sends manual compound  RTCP sender report (receiver report part of it included).
 * input: hRTCP
 *           - The handle of the RTCP session. 
 *        bCompound - if set to RV_TRUE to send compound report (RR + ALL SDES + APP)
 *                    if set to RV_FALSE to send RR only
 *        appMessageTable 
 *           - pointer to APP messages table (to be sent)
 *        appMessageTableEntriesNum - number of messages in appMessageTable
 *        
 * output: none. 
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value 
 *  Remark:
 * 1) This function will not affect the normal periodic RTCP messages and is not
 *	included in any statistical calculations, including bandwidth limitations. 
 * 2) appMessageTable and appMessageTableEntriesNum are used only if bCompound is set to RV_TRUE
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvRtcpSessionSendSR(
        IN RvRtcpSession hRTCP,
        IN RvBool bCompound,
        IN RvRtcpAppMessage* appMessageTable,
        IN RvInt32 appMessageTableEntriesNum)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvUint32 allocated = 0;
    RvUint32 data[MAXRTCPPACKET/sizeof(RvUint32)+1];
    RvRtpBuffer buf;
    RvStatus senderResult = RV_ERROR_UNKNOWN;
    RvStatus res = RV_OK;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendSR"));

    if (s == NULL)
    {
        RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendSR: NULL RTCP session pointer"));				
        RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendSR"));
        return RV_ERROR_NULLPTR;   
    }

    buf = buffCreate(data+1, MAXRTCPPACKET-4); /* resulting packet */
    
    RvLockGet(&s->lock, logMgr);
    senderResult = rtcpAddRTCPPacketType(hRTCP, RTCP_SR, 0, 0, NULL, 0, &buf, &allocated);
    if (senderResult == RV_ERROR_BADPARAM)
        /* sender report inside contains recever report data too */
        res = rtcpAddRTCPPacketType(hRTCP, RTCP_RR, 0, 0, NULL, 0, &buf, &allocated);

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
        if(s->rtcpSendCallback) {
           (*(s->rtcpSendCallback))((RvRtcpSession)s, s->haRtcpSend, 0 /*unused*/);
        }
#endif
    //SPIRENT_END

    RvLockRelease(&s->lock, logMgr);

    if (bCompound)
    {
        res = rtcpAddRTCPPacketType(hRTCP, RTCP_SDES, 1/* set inside*/, 0, NULL, 0, &buf, &allocated);
        if ((appMessageTableEntriesNum>0) &&(appMessageTable!=NULL))
        {
            RvInt32 Count = 0;
            for (Count = 0;Count < appMessageTableEntriesNum;Count++)
                if (appMessageTable[Count].userData!=NULL)
                    rtcpAddRTCPPacketType(hRTCP, RTCP_APP, appMessageTable[Count].subtype,
                    appMessageTable[Count].name, appMessageTable[Count].userData,
                    appMessageTable[Count].userDataLength, &buf, &allocated);
        }
    }
    /*  return RvSocketSendBuffer(&s->socket, buf.buffer, (RvSize_t)allocated, &s->remoteAddress, logMgr, NULL);*/
    res = rtcpSessionRawSend(hRTCP, buf.buffer, allocated, MAXRTCPPACKET, NULL);
    if (res>=0)
    {
       RvLogInfo(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendSR - Manual RTCP Sender Report was sent"));
    }
    else
    {
       RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendSR - failed to send Manual RTCP Sender Report, error=%d", res));
    }

    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendSR"));
    return res;
}

/*********************************************************************************************
 *	RvRtcpSessionSendRR 
 * description: sends manual compound  RTCP Receiver report.
 * input: hRTCP
 *           - The handle of the RTCP session. 
 *        bCompound - if set to RV_TRUE to send compound report (RR + ALL SDES + APP)
 *                    if set to RV_FALSE to send RR only
 *        appMessageTable 
 *           - pointer to APP messages table (to be sent)
 *        appMessageTableEntriesNum - number of messages in appMessageTable
 *        
 * output: none. 
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value 
 *  Remark:
 * 1) This function will not affect the normal periodic RTCP messages and is not
 *	included in any statistical calculations, including bandwidth limitations. 
 * 2) appMessageTable and appMessageTableEntriesNum are used only if bCompound is set to RV_TRUE
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvRtcpSessionSendRR(
        IN RvRtcpSession hRTCP,
        IN RvBool bCompound,
        IN RvRtcpAppMessage* appMessageTable,
        IN RvInt32 appMessageTableEntriesNum)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvUint32 allocated = 0;
    RvUint32 data[MAXRTCPPACKET/sizeof(RvUint32)+1];
    RvRtpBuffer buf;
    RvStatus res = RV_OK;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendRR"));

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    RvLockGet(&s->lock, logMgr);
#endif
    //SPIRENT_END

    buf = buffCreate(data+1, MAXRTCPPACKET-4); /* resulting packet */

    res = rtcpAddRTCPPacketType(hRTCP, RTCP_RR, 0, 0, NULL, 0, &buf, &allocated);
    
    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(s->rtcpSendCallback) {
       (*(s->rtcpSendCallback))((RvRtcpSession)s, s->haRtcpSend, 0 /*unused*/);
    }
#endif
    //SPIRENT_END

    if (bCompound)
    {
        res = rtcpAddRTCPPacketType(hRTCP, RTCP_SDES, 1/* set inside*/, 0, NULL, 0, &buf, &allocated);
        if ((appMessageTableEntriesNum>0) &&(appMessageTable!=NULL))
        {
            RvInt32 Count = 0;
            for (Count = 0;Count < appMessageTableEntriesNum;Count++)
                if (appMessageTable[Count].userData!=NULL)
                    rtcpAddRTCPPacketType(hRTCP, RTCP_APP, appMessageTable[Count].subtype,
                    appMessageTable[Count].name, appMessageTable[Count].userData,
                    appMessageTable[Count].userDataLength, &buf, &allocated);
        }
    }
    /*  return RvSocketSendBuffer(&s->socket, buf.buffer, (RvSize_t)allocated, &s->remoteAddress, logMgr, NULL);*/
    res = rtcpSessionRawSend(hRTCP, buf.buffer, allocated, MAXRTCPPACKET, NULL);
    if (res >= 0)
    {
       RvLogInfo(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendRR - Manual RTCP Receiver Report was sent"));
    }
    else
    {
       RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendRR - failed to send Manual RTCP Receiver Report, error=%d", res));
    }

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    RvLockRelease(&s->lock, logMgr);
#endif
    //SPIRENT_END

    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendRR"));
    return res;
}

/************************************************************************************
 *	RvRtcpSessionSendBye  Sends immediate compound BYE without closing session. 
 *  input:  hRTCP - Handle of RTP session.	
 *          reason - reason for BYE
 *          length - length of the reason
 *  return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value 
 * Note: 1) after this function RvRtpClose have to close the session
 *       2) this message is not included in RTCP bandwidth
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvRtcpSessionSendBye(
        IN RvRtcpSession hRTCP,
        IN const char *reason,
        IN  RvInt32 length)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvUint32 allocated = 0;
    RvUint32 data[MAXRTCPPACKET/sizeof(RvUint32)+1];
    RvRtpBuffer buf;
    RvStatus res = RV_OK;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendBye"));

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    RvLockGet(&s->lock, logMgr);
#endif
    //SPIRENT_END

    if (!s->isShutdown)
        s->isShutdown = RV_TRUE;
    buf = buffCreate(data+1, MAXRTCPPACKET-4); /* resulting packet */

    /* send Bye message immediately */

    res = rtcpAddRTCPPacketType(hRTCP, RTCP_EMPTY_RR, 0, 0, NULL, 0, &buf, &allocated);
    res = rtcpAddRTCPPacketType(hRTCP, RTCP_SDES, 1/* set inside*/, 0, NULL, 0, &buf, &allocated);
    res = rtcpAddRTCPPacketType(hRTCP, RTCP_BYE, 0, 0, (RvUint8*)reason, length, &buf, &allocated);

    /*  return RvSocketSendBuffer(&s->socket, buf.buffer, (RvSize_t)allocated, &s->remoteAddress, logMgr, NULL);*/
    res = rtcpSessionRawSend(hRTCP, buf.buffer, allocated, MAXRTCPPACKET, NULL);
    if (res >= 0)
    {
       RvLogInfo(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendBye -  manual RTCP BYE message was sent"));
    }
    else
    {
       RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendBye - failed to send manual RTCP BYE message, error=%d", res));
    }

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    RvLockRelease(&s->lock, logMgr);
#endif
    //SPIRENT_END

    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendBye"));
    return res;
}




RvStatus  rtcpSessionSendShutdownBye(
    IN  RvRtcpSession hRTCP,
    IN  const char *reason,
    IN  RvInt32 length)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvUint32 allocated = 0;
    RvUint32 data[MAXRTCPPACKET/sizeof(RvUint32)+1];
    RvRtpBuffer buf;
    RvStatus res = RV_OK;
    RvUint32 packLen = 0;

    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendBye"));

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    RvLockGet(&s->lock, logMgr);
#endif
    //SPIRENT_END

    if (!s->isShutdown)
        s->isShutdown = RV_TRUE;
    buf = buffCreate(data+1, MAXRTCPPACKET-4); /* resulting packet */
    
    /* send Bye message immediately */
    
    res = rtcpAddRTCPPacketType(hRTCP, RTCP_EMPTY_RR, 0, 0, NULL, 0, &buf, &allocated);
    res = rtcpAddRTCPPacketType(hRTCP, RTCP_SDES, 1/* set inside*/, 0, NULL, 0, &buf, &allocated);
    res = rtcpAddRTCPPacketType(hRTCP, RTCP_BYE, 0, 0, (RvUint8*)reason, length, &buf, &allocated);
    
    /*  return RvSocketSendBuffer(&s->socket, buf.buffer, (RvSize_t)allocated, &s->remoteAddress, logMgr, NULL);*/
    res = rtcpSessionRawSend(hRTCP, buf.buffer, allocated, MAXRTCPPACKET, &packLen);
    s->txInterval.lastRTCPPacketSize = packLen + RTCP_PACKETOVERHEAD /* IP/UDP headers ? */;

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    RvLockRelease(&s->lock, logMgr);
#endif
    //SPIRENT_END

    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSessionSendBye"));
    return res;
}

static RvBool rtcpByeTimerCallback(IN void* key)
{
    rtcpSession* s = (rtcpSession*)key;
    RvLogEnter(rvLogPtr, (rvLogPtr, "rtcpByeTimerCallback"));
    RvRtcpSessionSendBye((RvRtcpSession)s, s->byeReason, s->byeReasonLength);
    if (NULL!=s->rtpShutdownCompletedCallback)
        s->rtpShutdownCompletedCallback(s->rtpSession, s->byeReason, s->rtcpShutdownCompletedContext);
    RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpByeTimerCallback"));
    return RV_FALSE;
}
/************************************************************************************
 * RvRtpSetShutdownCompletedEventHandler
 * description: Set an Event Handler for the RTP session. The application must set
 *              this Event Handler for each RTP session, if RvRtpSessionShutdown
 *              will be used.
 * input: hRTP          - Handle of the RTP session to be called with the eventHandler.
 *        hRTCP         - Handle of the RTCP session.
 *        eventHandler  - Pointer to the callback function that is called each time  when
 *                        shutdown is completed.
 *        context       - The parameter is an application handle that identifies the
 *                        particular RTP session. The application passes the handle to
 *                        the Event Handler.
 * output: none.
 * return value: none.
 ***********************************************************************************/
RVAPI
void RVCALLCONV RvRtpSetShutdownCompletedEventHandler(
        IN  RvRtpSession        hRTP,
        IN  RvRtcpSession       hRTCP,
        IN  RvRtpShutdownCompleted_CB  eventHandler,
        IN  void *                     context)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpSetShutdownCompletedEventHandler"));
    if (NULL == hRTP || NULL ==hRTCP)
    {
        RvLogError(rvLogPtr, (rvLogPtr, "RvRtpSetShutdownCompletedEventHandler: NULL pointers"));
        RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpSetShutdownCompletedEventHandler"));
        return; /* error */
    }
    s->rtpSession = hRTP;
    s->rtpShutdownCompletedCallback = eventHandler;
    s->rtcpShutdownCompletedContext = context;
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpSetShutdownCompletedEventHandler"));
}
/*******************************************************************************************
 * RvRtcpSessionShutdown
 * description:  Sends a BYE packet after the appropriate (algorithmically calculated) delay.
 * input:  hRTCP - Handle of RTP session.
 *         reason - for BYE message
 *         reasonLength - length of reason
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value
 * Notes:
 *  1)  Don't release the reason pointer until RvRtpShutdownCompleted_CB is callback called
 *  2)  Don't call to RvRtpClose until RvRtpShutdownCompleted_CB is callback called
 ******************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvRtcpSessionShutdown(IN RvRtcpSession  hRTCP,
                                          IN RvChar *reason,
                                          IN RvUint8 reasonLength)
{
    RvTimerQueue *timerQ = NULL;
    RvChar *reasonText = NULL;
    RvUint32 textLength = 0, delay = 0;
    RvStatus status;
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSessionShutdown"));
    if (NULL==s)
    {
        RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpSessionShutdown: NULL pointer or reason is too big"));
        RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSessionShutdown"));
        return RV_ERROR_UNKNOWN;
    }
    s->isShutdown = RV_TRUE;
    if (!s->isTimerSet)
    {
        RvLogWarning(rvLogPtr, (rvLogPtr, "RvRtcpSessionShutdown: no need for shutdown - session does not send data"));
        RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSessionShutdown"));
        return RV_OK;
    }
    /* Find the timer queue we are working with */
    status = RvSelectGetTimeoutInfo(s->selectEngine, NULL, &timerQ);
    if (status != RV_OK)
        timerQ = NULL;

    if (timerQ == NULL)
        timerQ = rvRtcpInstance.timersQueue;

    reasonText = reason;
    textLength = reasonLength;

    RvTimerCancel(&s->timer, RV_TIMER_CANCEL_DONT_WAIT_FOR_CB);
    s->myInfo.active = 0;      /* no senders from now */
    if (s->sessionMembers < RTCP_BYEBACKOFFMINIMUM)
    {
        /* send Bye message immediately */
        rtcpSessionSendShutdownBye(hRTCP, reasonText, reasonLength);
        if (NULL!=s->rtpShutdownCompletedCallback)
            s->rtpShutdownCompletedCallback(s->rtpSession, reason, s->rtcpShutdownCompletedContext);
    }
    else
    {
        RvInt64 currentTime = RvTimestampGet(logMgr);
        delay = rtcpGetTimeInterval(s); /* in ms */
        s->txInterval.nextTime = RvInt64Add(s->txInterval.previousTime,
            RvInt64Mul(RvInt64FromRvUint32(delay), RV_TIME64_NSECPERMSEC));

        if (s->txInterval.nextTime > currentTime)
        {           /* We can't send BYEs now, so remember reason for later */
            s->byeReason       = reason;
            s->byeReasonLength = reasonLength;
            status = RvTimerStart(&s->timer, timerQ, RV_TIMER_TYPE_ONESHOT,
                RvInt64Mul(RV_TIME64_NSECPERMSEC,
                RvInt64Sub(s->txInterval.nextTime, currentTime)), rtcpByeTimerCallback, s);
        }
        else
        {
            rtcpSessionSendShutdownBye(hRTCP, reasonText, reasonLength);
            if (NULL!=s->rtpShutdownCompletedCallback)
                s->rtpShutdownCompletedCallback(s->rtpSession, reason, s->rtcpShutdownCompletedContext);
        }
    }
    if (reasonLength>0)
    {
        RvLogInfo(rvLogPtr, (rvLogPtr, "RvRtcpSessionShutdown -  called to RTCP Shutdown, reason =%s", reason));
    }
    else
    {
        RvLogInfo(rvLogPtr, (rvLogPtr, "RvRtcpSessionShutdown -  called to RTCP Shutdown"));
    }
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSessionShutdown"));
	RV_UNUSED_ARG(textLength);
    return RV_OK;
}


RVAPI
RvStatus RVCALLCONV RvRtcpSetAppEventHandler(
                                             IN RvRtcpSession            hRTCP,
                                             IN RvRtcpAppEventHandler_CB pAppEventHandler,
                                             IN void*                    rtcpAppContext)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSetAppEventHandler"));
    s->rtcpAppRecvCallback = pAppEventHandler;
    s->rtcpAppContext = rtcpAppContext; /*context is Event to inform Ring3 about RTCP arrival*/
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetAppEventHandler"));
    return RV_OK;
}
#if defined(UPDATED_BY_SPIRENT)
RVAPI
RvStatus RVCALLCONV RvRtcpSetAppTIPEventHandler(
                                             IN RvRtcpSession            hRTCP,
                                             IN RvRtcpAppTIPEventHandler_CB pAppTIPEventHandler,
                                             IN void*                    rtcpAppTIPContext)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSetAppTIPEventHandler"));
    s->rtcpAppTIPRecvCallback = pAppTIPEventHandler;
    s->rtcpAppTIPContext = rtcpAppTIPContext; /*context is Event to inform Ring3 about RTCP arrival*/
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetAppTIPEventHandler"));
    return RV_OK;
}
#endif


RVAPI
RvStatus RVCALLCONV RvRtcpSetByeEventHandler(
                                             IN RvRtcpSession            hRTCP,
                                             IN RvRtcpByeEventHandler_CB pByeEventHandler,
                                             IN void*                    rtcpByeContext)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSetByeEventHandler"));
    s->rtcpByeRecvCallback = pByeEventHandler;
    s->rtcpByeContext = rtcpByeContext; /*context is Event to inform Ring3 about RTCP arrival*/
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetByeEventHandler"));
    return RV_OK;
}

RVAPI
RvStatus RVCALLCONV RvRtcpSetSDESReceivedEventHandler(
      IN RvRtcpSession            hRTCP,
      IN RvRtcpSDESMessageReceived_CB SDESMessageReceived)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSetSDESReceivedEventHandler"));
    s->rtcpSDESMessageArrivedCallback = SDESMessageReceived;
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetSDESReceivedEventHandler"));
    return RV_OK;
}

RVAPI
RvInt32 RVCALLCONV RvRtcpGetSocket(IN RvRtcpSession hRTCP)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    if ((NULL != s) && s->isAllocated)
        return s->socket;

    return -1;
}

#if defined(RVRTP_SECURITY)
RVAPI
RvStatus RVCALLCONV RvRtcpSetEncryption(
    IN RvRtcpSession hRTCP,
    IN RvRtpEncryptionMode mode,
    IN RvRtpEncryptionPlugIn* enctyptorPlugInPtr,
    IN RvRtpEncryptionKeyPlugIn* keyPlugInPtr)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSetEncryption"));
    if (NULL==s||enctyptorPlugInPtr==NULL||NULL == keyPlugInPtr)
        return -1;
    RvRtcpSessionSetEncryptionMode (hRTCP, mode);
    s->encryptionKeyPlugInPtr  = keyPlugInPtr;
    s->encryptionPlugInPtr = (RvRtpEncryptionPlugIn*)enctyptorPlugInPtr;
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetEncryption"));
    return RV_OK;
}
#endif


RVAPI
RvStatus RVCALLCONV RvRtcpSetEncryptionNone(IN RvRtcpSession hRTCP)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSetEncryptionNone"));
    if (NULL == s)
    {
        RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpSetEncryptionNone: hRTCP points to NULL"));
        return -1;
    }
    s->encryptionPlugInPtr = NULL;
    s->encryptionKeyPlugInPtr = NULL;
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSetEncryptionNone"));
    return RV_OK;
}

RVAPI
RvStatus RVCALLCONV RvRtcpSessionSetEncryptionMode(IN RvRtcpSession hRTCP, IN RvRtpEncryptionMode mode)
{
    rtcpSession *s = (rtcpSession *)hRTCP;

    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSessionSetEncryptionMode"));
    if (NULL == s)
        return RV_ERROR_UNKNOWN;

    s->encryptionMode = mode;
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSessionSetEncryptionMode"));
    return RV_OK;
}

/************************************************************************************
 * RvRtcpSessionSetParam
 * description:  Sets session parameters for the session
 * input:  hRTCP - Handle of RTCP session.
 *         param - type of parameter
 *         data  - pointer to parameter
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a RV_OK value.
 * Note:1)   For RTP session with dynamic payload (with param = RVRTCP_PARAMS_RTPCLOCKRATE)
 *           this function should be called
 *           after RTP session opening for accurate RTCP timestamp calculation
 *           For standard payload types this call can be omitted.
 ***********************************************************************************/

RVAPI
RvStatus RVCALLCONV RvRtcpSessionSetParam(
    IN RvRtcpSession hRTCP,
    IN RvRtcpParameters param,
    IN void* data)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtcpSessionSetParam"));
   
    if (NULL==s || data == NULL)
        return RV_ERROR_UNKNOWN;    

    switch (param)
    {
    case RVRTCP_PARAMS_RTPCLOCKRATE:
        RvLockGet(&s->lock, logMgr);
        s->myInfo.rtpClockRate = *((RvUint32*) data);
        RvLockRelease(&s->lock, logMgr);
        break;
    case RVRTCP_PARAMS_RTP_PAYLOADTYPE:
        if (s->myInfo.rtpPayloadType != *((RvUint8*) data))
        {
            RvLockGet(&s->lock, logMgr);
            s->myInfo.rtpPayloadType = *((RvUint8*) data);
            RvLockRelease(&s->lock, logMgr);
        }
        break;
    default:
        return RV_ERROR_UNKNOWN;
    }
    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtcpSessionSetParam"));
    return RV_OK;
}

RVAPI
RvStatus  RVCALLCONV RvRtcpSetProfilePlugin(
                                            IN RvRtcpSession hRTCP, 
                                            IN RtpProfilePlugin* profilePlugin)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvLogEnter(rvLogPtr, (rvLogPtr, "rtcpSetRtpSessionProfilePlugin"));
    if (NULL == hRTCP)
    {
        RvLogError(rvLogPtr, (rvLogPtr, "rtcpSetRtpSessionProfilePlugin"));
        RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpSetRtpSessionProfilePlugin"));
        return RV_ERROR_NULLPTR;
    }
    s->profilePlugin = profilePlugin;
    RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpSetRtpSessionProfilePlugin"));
    return RV_OK;
}

#undef FUNC_NAME
#define FUNC_NAME(name) "rtcp" #name
RVAPI
RvUint32  RVCALLCONV rtcpSessionGetIndex(
                                         IN RvRtcpSession hRTCP)
{
    rtcpSession *s = (rtcpSession *)hRTCP;

    RTPLOG_ENTER(SessionGetIndex);
    if (NULL==s)
    {
        RTPLOG_ERROR_LEAVE(SessionGetIndex, "NULL session pointer");
        return 0;
    }
    RTPLOG_LEAVE(SessionGetIndex);
    return s->index;
}


/********* STATIC FUNCTIONS *************************************************************************/

#define RV_READ2BYTES(a,b)   ((b)[0]=(a)[0],(b)[1]=(a)[1])
#define RV_WRITE2BYTES(a,b)  ((a)[0]=(b)[0],(a)[1]=(b)[1])
#define RV_READ4BYTES(a,b)   ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
#define RV_WRITE4BYTES(a,b)  ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

#define  rvDataBufferReadAtUint16(buf, i, s)            (RV_READ2BYTES(buf + (i),(RvUint8*)(s)),*((RvUint16*)s) = (RvInt16 )RvConvertNetworkToHost16(*((RvUint16*)s)))
#define  rvDataBufferWriteAtUint16(buf, i, s)           {RvUint16 t = (RvInt16 )RvConvertNetworkToHost16((RvUint16)(s)); RV_WRITE2BYTES(buf + (i), (RvUint8*)&t);}
#define  rvDataBufferReadAtInt16(buf, i, s)             (RV_READ2BYTES(buf + (i),(RvUint8*)(s)),*((RvUint16*)s) = (RvInt16 )RvConvertNetworkToHost16(*((RvUint16*)s)))
#define  rvDataBufferWriteAtInt16(buf, i, s)            {RvUint16 t = (RvInt16 )RvConvertNetworkToHost16((RvUint16)(s)); RV_WRITE2BYTES(buf + (i), (RvUint8*)&t);}
#define  rvDataBufferReadAtUint32(buf, i, l)            (RV_READ4BYTES(buf + (i),(RvUint8*)(l)),*((RvUint32*)l) = RvConvertNetworkToHost32(*((RvUint32*)l)))
#define  rvDataBufferWriteAtUint32(buf, i, l)           {RvUint32 t = RvConvertNetworkToHost32((RvUint32)(l)); RV_WRITE4BYTES(buf + (i), (RvUint8*)&t);}
#define  rvDataBufferReadAtInt32(buf, i, l)             (RV_READ4BYTES(buf + (i),(RvUint8*)(l)),*((RvUint32*)l) = RvConvertNetworkToHost32(*((RvUint32*)l)))
#define  rvDataBufferWriteAtInt32(buf, i, l)            {RvUint32 t = RvConvertNetworkToHost32((RvUint32)(l)); RV_WRITE4BYTES(buf + (i), (RvUint8*)&t);}

#define RV_FREAD2BYTES(a,b)   ((b)[0] = (*(a)++),(b)[1] = (*(a)++))
#define RV_FWRITE2BYTES(a,b)  ((*--(a))=(b)[1],(*--(a))=(b)[0])
#define RV_FREAD4BYTES(a,b)   ((b)[0]=(*(a)++),(b)[1]=(*(a)++),(b)[2]=(*(a)++),(b)[3]=(*(a)++))
#define RV_FWRITE4BYTES(a,b)  ((*--(a))=(b)[3],(*--(a))=(b)[2],(*--(a))=(b)[1],(*--(a))=(b)[0])

#define  rvDataBufferWriteUint32(buf, l)                {RvUint32 t = RvConvertNetworkToHost32((RvUint32)(l)); RV_FWRITE4BYTES(buf, (RvUint8*)&t);}

RvBool rtcpSessionIsSessionInShutdown(
        IN RvRtcpSession hRTCP)

{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvBool       shutDown = RV_FALSE;
    RvLogEnter(rvLogPtr, (rvLogPtr, "rtcpSessionIsSessionInShutdown"));
    if (NULL==s)
    {
        RvLogError(rvLogPtr, (rvLogPtr, "rtcpSessionIsSessionInShutdown"));
        RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpSessionIsSessionInShutdown"));
        return RV_FALSE;
    }
    RvLockGet(&s->lock, logMgr);
    shutDown = s->isShutdown;
    RvLockRelease(&s->lock, logMgr);
    RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpSessionIsSessionInShutdown"));
    return shutDown;
}

RvStatus rtcpSetRtpSession(IN RvRtcpSession hRTCP, IN RvRtpSession hRTP)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvLogEnter(rvLogPtr, (rvLogPtr, "rtcpSetRtpSession"));
    if (NULL == hRTCP || NULL == hRTP)
    {
        RvLogError(rvLogPtr, (rvLogPtr, "rtcpSetRtpSession"));
        RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpSetRtpSession"));
        return RV_ERROR_NULLPTR;
    }
    s->rtpSession = hRTP;
    RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpSetRtpSession"));
    return RV_OK;
}

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
void RvRtcpSetReportInterval(IN  RvRtcpSession  hRTCP,RvUint32 ri) {
   rtcpSession *s = (rtcpSession *)hRTCP;
   if(s) s->rtcpReportInterval=ri;
}
#endif
//SPIRENT_END

static RvStatus rtcpSessionRawSend(
                                   IN    RvRtcpSession hRTCP,
                                   IN    RvUint8*      bufPtr,
                                   IN    RvInt32       DataLen,
                                   IN    RvInt32       DataCapacity,
                                   INOUT RvUint32*     sentLenPtr)
{
     rtcpSession *s = (rtcpSession *)hRTCP;
     RvStatus status = RV_ERROR_UNKNOWN;

     //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
     RvLockGet(&s->lock, logMgr);
#endif
     //SPIRENT_END

     if (s!=NULL && 
         s->profilePlugin!=NULL &&          
         s->profilePlugin->funcs !=NULL &&
         s->profilePlugin->funcs->xrtcpRawSend!=NULL)
         status = s->profilePlugin->funcs->xrtcpRawSend(s->profilePlugin, hRTCP, bufPtr, DataLen, DataCapacity, sentLenPtr);

     //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
     RvLockRelease(&s->lock, logMgr);
#endif
     //SPIRENT_END

     return status;
}

static RvUint32 rtcpGetSessionMembersInfo(IN rtcpSession* s, OUT RvUint32 *sessionMembersPtr, OUT RvUint32 *senderMembersPtr)
{
    RvInt32 i, senderCount = 0, memberCount = 0;
    rtcpInfo* info;
    RvLogEnter(rvLogPtr, (rvLogPtr, "rtcpGetSenderCount"));
    if (!s->txInterval.initialized)
    {
        for (i = 0; i < s->sessionMembers; ++i)
        {
            info = &s->participantsArray[i];
            if (!info->invalid)
            {
                memberCount++;
                if (info->active)
                    senderCount++;
            }
        }
        if(s->myInfo.active>0)
            senderCount++;
        *sessionMembersPtr = memberCount;
        *senderMembersPtr  = senderCount;
    }
    else
    {
        *sessionMembersPtr  = s->txInterval.members;
        *senderMembersPtr   = s->txInterval.senders + (s->myInfo.active>0); /* second time we_sent>0 */
    }
    RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpGetSenderCount"));
    return senderCount;
}

static RvUint32 rtcpGetTimeInterval(IN rtcpSession* s)
{
    const RvInt32 RTCP_MIN_TIME           = 5000;
    const RvInt32 RTCP_SENDER_BW_FRACTION =  250;
    const RvInt32 RTCP_RCVR_BW_FRACTION   = 1000 - RTCP_SENDER_BW_FRACTION;
    const RvInt32 RTCP_SCALER             = 1000;


    RvUint32 rtcpBandwidth;
    RvUint32 time;
    RvUint32 jitteredTime;
    RvUint32 rtcpMinTime = RTCP_MIN_TIME;
    RvUint32 numMembers;
    RvUint32  senders, sessionMembers;

    RvLogEnter(rvLogPtr, (rvLogPtr, "rtcpGetTimeInterval"));

    rtcpBandwidth = s->txInterval.rtcpBandwidth;
    rtcpGetSessionMembersInfo(s, &sessionMembers, &senders);

    if(s->txInterval.initialized)
    {
        rtcpMinTime /= 2;
 /*       s->txInterval.initialized = RV_FALSE;  set to FALSE after second calculation */
    }

    numMembers = sessionMembers;

    if (senders <= (numMembers * RTCP_SENDER_BW_FRACTION / RTCP_SCALER))
    {
        /* Set our available bandwidth based on whether we are a sender */
        /* or just a receiver                                          */
        if(s->myInfo.active>0) /* we sent */
        {
            rtcpBandwidth = rtcpBandwidth * RTCP_SENDER_BW_FRACTION / RTCP_SCALER;
            numMembers = senders;
        }
        else
        {
            rtcpBandwidth = rtcpBandwidth * RTCP_RCVR_BW_FRACTION / RTCP_SCALER;
            numMembers -= senders;
        }
    }
    /* Set the time based on bandwidth  [bandwidth = data/time] in milliseconds */
    if(rtcpBandwidth != 0)
    {
       //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
       if(s->rtcpReportInterval>0 && s->rtcpReportInterval!=(RvUint32)RTCP_MIN_TIME) {
          jitteredTime = s->rtcpReportInterval;
       } else {
#endif
          //SPIRENT_END
          RvRandom randomValue;
          
          time = s->txInterval.aveRTCPPacketSize * numMembers * 1000 / rtcpBandwidth;
          
          /* Check to see minimum time requirements are made */
          if(time < rtcpMinTime)
             time = rtcpMinTime;
          
          RvRandomGeneratorGetValue(&rvRtcpInstance.randomGenerator, &randomValue);
          jitteredTime = time * (randomValue % 1000 + 500) / 1000;
          
          /* reconsideration compensation - divide by (e - 1.5), which is 1.21828 (as per RFC 3550) */
          jitteredTime = RvUint64ToRvUint32(
             RvUint64Div(
             RvUint64Mul(RvUint64Const(0, jitteredTime),
             RvUint64Const(0, 100000)),
             RvUint64Const(0, 121828)));
          
          //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
       }
#endif
       //SPIRENT_END
       
    }
    else
    {
        jitteredTime = 0; /* no periodic transmitions */
    }

    RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpGetTimeInterval"));
    return  jitteredTime; /* in milliseconds */
}


static RvStatus rtcpSetTimer(
        IN rtcpSession* s,
        IN RvInt64      delay)
{
    RvTimerQueue *timerQ = NULL;
    RvStatus status;

    RvLogEnter(rvLogPtr, (rvLogPtr, "rtcpSetTimer"));
    /* Find the timer queue we are working with */

    status = RvSelectGetTimeoutInfo(s->selectEngine, NULL, &timerQ);
    if (status != RV_OK)
        timerQ = NULL;

    if (timerQ == NULL)
        timerQ = rvRtcpInstance.timersQueue;

    /* Set a timer for a default interval of 5 sec */
    status = RvTimerStart(&s->timer, timerQ, RV_TIMER_TYPE_ONESHOT, delay, rtcpTimerCallback, s);
    RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpSetTimer"));
    return status;
}


static RvBool rtcpTimerCallback(IN void* key)
{
    rtcpSession* s = (rtcpSession*)key;
    RvInt64 currentTime;
    RvUint32 delay;
    RvLogEnter(rvLogPtr, (rvLogPtr, "rtcpTimerCallback"));

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(!s) {
       return RV_FALSE;
    }
#endif
    //SPIRENT_END

    if (s!=NULL)
    {
        RvLockGet(&s->lock, logMgr);
        if (s->isShutdown)
        {
            RvLockRelease(&s->lock, logMgr);    
            /* no more reports */
            RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpTimerCallback"));
            return RV_FALSE;
        }
        RvLockRelease(&s->lock, logMgr);  
    }

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    RvLockGet(&s->lock, logMgr);
#endif
    //SPIRENT_END

    currentTime = RvTimestampGet(logMgr);
    delay = rtcpGetTimeInterval(s); /* in ms */
    s->txInterval.nextTime = RvInt64Add(s->txInterval.previousTime,
        RvInt64Mul(RvInt64FromRvUint32(delay), RV_TIME64_NSECPERMSEC));
    /* Note: time interval calculation should be done before creating the RTCP packet,
             otherwise the "active" attributes of the senders are set to FALSE */
    if (s->txInterval.nextTime > currentTime)
        rtcpSetTimer(s, RvInt64Sub(s->txInterval.nextTime, currentTime));
    else
    {
        RvUint32 data[MAXRTCPPACKET/sizeof(RvUint32)+1];
        RvRtpBuffer buf;
        RvUint32    rtcpPackSize = 0;

        buf = buffCreate(data+1, MAXRTCPPACKET-4); /* 4 bytes in header remained for encryption purposes */
        
        rtcpCreateRTCPPacket((RvRtcpSession)s, &buf);

        //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
        if(s->rtcpSendCallback) {
           (*(s->rtcpSendCallback))((RvRtcpSession)s, s->haRtcpSend, 0 /*unused*/);
        }
#endif
        //SPIRENT_END

        rtcpSessionRawSend((RvRtcpSession)s, buf.buffer, buf.length, MAXRTCPPACKET, &rtcpPackSize);

        s->txInterval.lastRTCPPacketSize = rtcpPackSize + RTCP_PACKETOVERHEAD;

        /* Update the average packet size  section 6.3.3 RFC 3550 */
        s->txInterval.aveRTCPPacketSize =
            (s->txInterval.lastRTCPPacketSize +
            s->txInterval.aveRTCPPacketSize*RvUint32Const(15)) / RvUint32Const(16);
        s->txInterval.previousTime = currentTime;
        delay = rtcpGetTimeInterval(s); /* in ms */
        rtcpSetTimer(s, delay * RV_TIME64_NSECPERMSEC);
        s->txInterval.initialized = RV_FALSE; /* ??? @@@*/
    }
    s->txInterval.pmembers = s->txInterval.members;

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    RvLockRelease(&s->lock, logMgr);
#endif
    //SPIRENT_END

    RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpTimerCallback"));
    return RV_FALSE;
}

static RvStatus rtcpSessionRawReceive(
                                      IN    RvRtcpSession hRTCP,
                                      INOUT RvRtpBuffer* buf,
                                      IN    RvSize_t bytesToReceive,
                                      OUT   RvSize_t*   bytesReceivedPtr,
                                      OUT   RvAddress*  remoteAddressPtr)
{
    RvStatus res = RV_ERROR_UNKNOWN;
    rtcpSession *s = (rtcpSession *)hRTCP;

    RvLogEnter(rvLogPtr, (rvLogPtr, "rtcpSessionRawReceive"));

    if (s != NULL && s->profilePlugin !=NULL && 
        s->profilePlugin->funcs!=NULL &&
        s->profilePlugin->funcs->xrtcpRawReceive!= NULL)
    {
       res = s->profilePlugin->funcs->xrtcpRawReceive(
           s->profilePlugin, hRTCP, buf, bytesToReceive, bytesReceivedPtr, remoteAddressPtr);
    }

    RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpSessionRawReceive"));
    return res;
}

#ifdef OLD_4REF
/* Reads RTCP data to the buffer buf from socket,
   decrypts it, if needed, returns size of RTCP report message and remote address */
static RvStatus rtcpSessionRawReceive(
    IN    RvRtcpSession hRTCP,
    INOUT RvRtpBuffer* buf,
    IN    RvSize_t bytesToReceive,
    OUT   RvSize_t*   bytesReceivedPtr,
    OUT   RvAddress*  remoteAddressPtr)
{
    RvStatus res = RV_ERROR_UNKNOWN;
    RvSize_t bytesReceived=0;
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvLogEnter(rvLogPtr, (rvLogPtr, "rtcpSessionRawReceive"));
    { 
        /* SRTCP */
        RvRtpSession hRTP = s->rtpSession;
        RvRtpSessionInfo* rtp = (RvRtpSessionInfo*) hRTP;
        if ((hRTP!=NULL) && (rtp->srtp != NULL) && (rtp->srtpEncryptionPlugInPtr != NULL))
        {   /* perform SRTCP read */
            return srtcpSessionRawReceive(
                hRTP, &s->socket,buf, bytesToReceive, bytesReceivedPtr, remoteAddressPtr);
        }
    }
    res = RvSocketReceiveBuffer(&s->socket, buf->buffer, bytesToReceive, logMgr, &bytesReceived, remoteAddressPtr);
    if (res == RV_OK)
    {
        /* If the encryption interface is installed, decrypt the data */
        if((s->encryptionPlugInPtr != NULL)&&
            (!RvRtpEncryptionModeIsPlainTextRtcp(s->encryptionMode)))
        {
            RvRtpEncryptionData              encryptionData;
            RvKey                             decryptionKey;

            RvKeyConstruct(&decryptionKey);
            RvRtpEncryptionDataConstruct(&encryptionData);

            RvRtpEncryptionDataSetIsRtp(&encryptionData, RV_FALSE);
            RvRtpEncryptionDataSetIsEncrypting(&encryptionData, RV_FALSE);
            RvRtpEncryptionDataSetMode(&encryptionData, s->encryptionMode);
            RvRtpEncryptionDataSetRtpHeader(&encryptionData, NULL);
            RvRtpEncryptionDataSetLocalAddress(&encryptionData, NULL);
            RvRtpEncryptionDataSetRemoteAddress(&encryptionData, remoteAddressPtr);

            if(s->encryptionKeyPlugInPtr != NULL)
            {
                RvRtpEncryptionKeyPlugInGetKey(s->encryptionKeyPlugInPtr, &encryptionData, &decryptionKey);
            }

            RvRtpEncryptionPlugInDecrypt(s->encryptionPlugInPtr,
                buf->buffer,
                buf->buffer,
                bytesReceived,
                &encryptionData, &decryptionKey);

            /* Remove the 32-bit random number attached to header */
            /*          rvDataBufferSkip(bufferPtr,4);*/
            *buf = buffCreate(buf->buffer+4, MAXRTCPPACKET-4);
            bytesReceived -=4;
            *bytesReceivedPtr = bytesReceived;
            RvRtpEncryptionDataDestruct(&encryptionData);
            RvKeyDestruct(&decryptionKey);
        }
        else
        {
            *bytesReceivedPtr = bytesReceived;
        }
    }
    RvLogLeave(rvLogPtr, (rvLogPtr, "rtcpSessionRawReceive"));
    return res;
}
#endif

static void rtcpEventCallback(
        IN RvSelectEngine*  selectEngine,
        IN RvSelectFd*      fd,
        IN RvSelectEvents   selectEvent,
        IN RvBool           error)
{
    rtcpSession* s;
    RvUint32 buffer[MAXRTCPPACKET/sizeof(RvUint32)+1];
    RvRtpBuffer buf;
    RvStatus res;
    RvAddress remoteAddress, localAddress;
    RvSize_t bytesReceived;

    RvLogEnter(RTP_SOURCE, (RTP_SOURCE, "rtcpEventCallback"));
    RV_UNUSED_ARG(selectEngine);
    RV_UNUSED_ARG(selectEvent);
    RV_UNUSED_ARG(error);

    s = RV_GET_STRUCT(rtcpSession, selectFd, fd);

    /* We register to WRITE event when the session should be destructed */
    if (selectEvent & RvSelectWrite)
    {
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
        RvRtpAddressListDestruct(&s->addressList);
#endif
/* SPIRENT_END */
        RvSelectRemove(s->selectEngine, &s->selectFd);
        RvFdDestruct(&s->selectFd);
        /* Close the socket */
        RvSocketDestruct(&s->socket, RV_FALSE, NULL, logMgr);
        /* Destroy the lock */
        RvLockDestruct(&s->lock, logMgr);
        /* free memory allocated for rtcpSession */
        if (s->isAllocated)
            RvMemoryFree(s, logMgr);
        return;
    }

    if (s!=NULL && s->isShutdown)
    {
        /* session must not receive RTCP packet after sending BYE packet */
        RvLogLeave(RTP_SOURCE, (RTP_SOURCE, "rtcpEventCallback"));
        return;
    }

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)

    if(error) {
       RvLogLeave(RTP_SOURCE, (RTP_SOURCE, "rtcpEventCallback error"));
       return;
    }
#endif
     //SPIRENT_END

    buf = buffCreate(buffer, MAXRTCPPACKET);
    res = rtcpSessionRawReceive((RvRtcpSession) s, &buf, (RvSize_t)buf.length, &bytesReceived, &remoteAddress);
    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if (res == RV_OK && bytesReceived>0)
#else
    if (res == RV_OK)
#endif
     //SPIRENT_END
    {
        buf.length = (RvUint32)bytesReceived;

#if defined(UPDATED_BY_SPIRENT)
        if(s->rtcpAppTIPRecvCallback){
            s->rtcpAppTIPRecvCallback((RvRtcpSession)s,s->rtcpAppTIPContext,buf.buffer,buf.length);
        }
#endif

        RvSocketGetLocalAddress(&s->socket, logMgr, &localAddress);

        if ((RvAddressGetIpPort(&remoteAddress) != RvAddressGetIpPort(&localAddress)) ||
            !isMyIP(&remoteAddress))
        {
            RvInt32 res=rtcpProcessCompoundRTCPPacket((RvRtcpSession)s, &buf, getNNTPTime());
            if ((res==0) && (s->rtcpRecvCallback != NULL))
            {
                RvUint32 ssrc=getSSRCfrom(buf.buffer);
                s->rtcpRecvCallback((RvRtcpSession)s, s->haRtcp, ssrc);
            }
        }
    }

    RvLogLeave(RTP_SOURCE, (RTP_SOURCE, "rtcpEventCallback"));
}

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
RvUint64 getNNTPTime(void)
#else
static RvUint64 getNNTPTime(void)
#endif
//SPIRENT_END
{
    RvTime t;
    RvNtpTime ntptime;
    RvUint64  result;
//SPIRENT_BEGIN
#if !defined(UPDATED_BY_SPIRENT)
    RvLogEnter(rvLogPtr, (rvLogPtr, "getNNTPTime"));
#endif
//SPIRENT_END
    RvTimeConstruct(0, 0, &t); /* create and clear time structure */
#if 1
    RvClockGet(NULL, &t); /* Get current wall-clock time */
#else
    {
        RvUint64  timestamp = RvTimestampGet(NULL);
        RvTimeConstructFrom64(timestamp, &t);
    }
#endif
    RvNtpTimeConstructFromTime(&t, RV_NTPTIME_ABSOLUTE, &ntptime); /* convert to NTP time */
    result = RvNtpTimeTo64(&ntptime, 32, 32); /* Convert to format getNNTPTime returns */
    RvNtpTimeDestruct(&ntptime); /* clean up */
    RvTimeDestruct(&t); /* clean up */
//SPIRENT_BEGIN
#if !defined(UPDATED_BY_SPIRENT)
    RvLogLeave(rvLogPtr, (rvLogPtr, "getNNTPTime"));
#endif
//SPIRENT_END
    return result;
}

static void setSDES(RvRtcpSDesType type, rtcpSDES* sdes, RvUint8 *data, RvInt32 length)
{
    RvLogEnter(rvLogPtr, (rvLogPtr, "setSDES"));
    sdes->type   = (unsigned char)type;
    sdes->length = (unsigned char)length;
    memcpy(sdes->value, data, (RvSize_t)length);
    memset(sdes->value+length, 0, (RvSize_t)( 4-((length+2)%sizeof(RvUint32)) ));
    RvLogLeave(rvLogPtr, (rvLogPtr, "setSDES"));
}

static void init_seq(rtpSource *s, RvUint16 seq)
{
    RvLogEnter(rvLogPtr, (rvLogPtr, "init_seq"));
    s->base_seq       = seq;
    s->max_seq        = seq;
    s->bad_seq        = RTP_SEQ_MOD + 1;
    s->cycles         = 0;
    s->received       = 0;
    s->received_prior = 0;
    s->expected_prior = 0;
    RvLogLeave(rvLogPtr, (rvLogPtr, "init_seq"));
}


static RvInt32 update_seq(rtpSource *s, RvUint16 seq, RvUint32 ts, RvUint32 arrival)
{
    RvUint16 udelta = (RvUint16)(seq - s->max_seq);
    RvLogEnter(rvLogPtr, (rvLogPtr, "update_seq"));

    if (s->probation)
    {
        if (seq == s->max_seq + 1)
        {
           /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
           if (s->probation == 1)
           {
              init_seq(s, seq);
              s->received++;
              s->probation=0;
              RvLogLeave(rvLogPtr, (rvLogPtr, "update_seq"));
              return 1;
           } 
           else 
           {
              s->probation--;
              s->max_seq = seq;
           }
#else
           s->probation--;
           s->max_seq = seq;
           if (s->probation == 0)
           {
              init_seq(s, seq);
              s->received++;
              RvLogLeave(rvLogPtr, (rvLogPtr, "update_seq"));
              return 1;
           }
#endif
           /* SPIRENT_END */

        }
        else
        {
            s->probation = MIN_SEQUENTIAL - 1;
            s->max_seq = seq;
        }
        RvLogLeave(rvLogPtr, (rvLogPtr, "update_seq"));
        return RV_OK;
    }
    else if (udelta < MAX_DROPOUT)
    {
        if (seq < s->max_seq) s->cycles += RTP_SEQ_MOD;
        s->max_seq = seq;
    }
    else if (udelta <= RTP_SEQ_MOD - MAX_MISORDER)
    {
        if (seq == s->bad_seq)
        {
            init_seq(s, seq);
        }
        else
        {
            s->bad_seq = (seq + 1) & (RTP_SEQ_MOD-1);
            RvLogLeave(rvLogPtr, (rvLogPtr, "update_seq"));
            return RV_OK;
        }
    }
    else
    {
   /* duplicate or reordered packet */
    }

    /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    {
        RvInt32  transit = (RvInt32)(arrival - ts);
        RvInt32  d = 0;
        if (s->transit)
        {
            d = (RvInt32)(transit - s->transit);
            if (d < 0) d = -d;
            /* calculation scaled to reduce round off error */
            s->jitter += d - ((s->jitter + 8) >> 4);
        }
        s->transit = transit;
        if ( s->transit == 0 ) s->transit = 1; // to avoid the case in which the arrival time and timestamp matched perfectly
    }
#else
    {
        RvInt32 transit = (RvInt32)(arrival - ts);
        RvInt32 d = (RvInt32)(transit - s->transit);
        s->transit = transit;
        if (d < 0) d = -d;
        /* calculation scaled to reduce round off error */
        s->jitter += d - ((s->jitter + 8) >> 4);
    }
#endif
/* SPIRENT_END */

    s->received++;
    RvLogLeave(rvLogPtr, (rvLogPtr, "update_seq"));
    return 1;
}


/*=========================================================================**
**  == makeHeader() ==                                                     **
**                                                                         **
**  Creates an RTCP packet header.                                         **
**                                                                         **
**  PARAMETERS:                                                            **
**      ssrc       A synchronization source value for the RTCP session.    **
**                                                                         **
**   count (subtype)                                                       **
** [count]     - A count of sender and receiver reports in the packet.     **
** [subtype]   - to allow a set of APP packets to be defined under one     **
**               unique name, or for any application-dependent             **
**               data.                                                     **
**                                                                         **
**                                                                         **
**      type       The RTCP packet type.                                   **
**                                                                         **
**      dataLen    The length of the data in the packet buffer, in         **
**                 octets, including the size of the header.               **
**                                                                         **
**  RETURNS:                                                               **
**      The function returns a header with the appropriate parameters.     **
**                                                                         **
**=========================================================================*/

static rtcpHeader makeHeader(RvUint32 ssrc, RvUint8 count, rtcpType type,
                             RvUint16 dataLen)
{
    rtcpHeader header;

    RvLogEnter(rvLogPtr, (rvLogPtr, "makeHeader"));
    header.ssrc = ssrc;

    header.bits = RTCP_HEADER_INIT;
    header.bits = bitfieldSet(header.bits, count, HEADER_RC, HDR_LEN_RC);
    header.bits = bitfieldSet(header.bits, type,  HEADER_PT, HDR_LEN_PT);
    header.bits = bitfieldSet(header.bits, W32Len(dataLen) - 1, HEADER_len, HDR_LEN_len);

    ConvertToNetwork(&header, 0, W32Len(SIZEOF_RTCPHEADER));
    RvLogLeave(rvLogPtr, (rvLogPtr, "makeHeader"));
    return header;
}

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

static RvUint32 getReceivedPackets(rtpSource *s)
{
    RvInt32 received_interval;

    RvLogEnter(rvLogPtr, (rvLogPtr, __FUNCTION__));

    received_interval = (s->received > s->received_prior ? (s->received - s->received_prior) : 0);

    RvLogLeave(rvLogPtr, (rvLogPtr, __FUNCTION__));

    return received_interval;
}

#endif
/* SPIRENT END */

static RvUint32 getLost(rtpSource *s)
{
    RvUint32 extended_max;
    RvUint32 expected;
    RvInt32 received_interval;
    RvInt32 expected_interval;
    RvInt32 lost;
    RvInt32 lost_interval;
    RvUint8 fraction;

    /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    if(s->probation) {
       return 0;
    }
#endif
    /* SPIRENT_END */

    RvLogEnter(rvLogPtr, (rvLogPtr, "getLost"));

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    /*
     *    we're going to set the results of all following subtractions to zero if knowing its left operand is
     * smaller than its right operand.  The reason is to avoid a huge unsigned result number when subtracting a
     * smaller unsigned number to a larger unsigned number.  
     *
     *    Why is there such situation and is it an error?  We kindof expect such situations occured and they're
     * not considered an error.  For example: the case where the "expected" number is less than "s->received"
     * number. There is no lost in this case because we expect less packets than we actuall received. In addition,
     * since the expected count is calculated from the sequence numbers, it is not guaranteed that it's always greater 
     * than the actual (accumulated) count due to packets' duplicated/reordered. So, just simply readjust the expected 
     * number to avoid large number of packet lost result of the subtraction of two unsigned number.
     */
    extended_max = s->cycles + s->max_seq;
    expected = (extended_max > (s->base_seq + 1) ? (extended_max - s->base_seq + 1) : 0);
    lost = (expected > s->received ? (expected - s->received) : 0);
    expected_interval = (expected > s->expected_prior ? (expected - s->expected_prior) : 0);
    s->expected_prior = expected;

    received_interval = (s->received > s->received_prior ? (s->received - s->received_prior) : 0);
    s->received_prior = s->received;

    lost_interval = (expected_interval > received_interval ? (expected_interval - received_interval) : 0);
#else
    extended_max = s->cycles + s->max_seq;
    expected = extended_max - s->base_seq + 1;
    lost = expected - s->received;
    expected_interval = expected - s->expected_prior;
    s->expected_prior = expected;
    received_interval = s->received - s->received_prior;
    s->received_prior = s->received;
    lost_interval = expected_interval - received_interval;
#endif
    /* SPIRENT_END */

    if (expected_interval == 0  ||  lost_interval <= 0)
        fraction = 0;
    else
        fraction = (RvUint8)((lost_interval << 8) / expected_interval);
    RvLogLeave(rvLogPtr, (rvLogPtr, "getLost"));

    /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    if(s->probation) {
       return 0;
    }
#endif
    /* SPIRENT_END */

    return (fraction << 24) + lost;
}


static RvUint32 getJitter(rtpSource *s)
{
    RvLogEnter(rvLogPtr, (rvLogPtr, "getJitter"));
    RvLogLeave(rvLogPtr, (rvLogPtr, "getJitter"));
    return s->jitter >> 4;
}

static RvUint32 getSequence(rtpSource *s)
{
    RvLogEnter(rvLogPtr, (rvLogPtr, "getSequence"));
    RvLogLeave(rvLogPtr, (rvLogPtr, "getSequence"));
    return s->max_seq + s->cycles;
}



static RvUint32 getSSRCfrom(RvUint8 *head)
{
   RvUint8 *ssrcPtr = (RvUint8 *)head + sizeof(RvUint32);
   RvLogEnter(rvLogPtr, (rvLogPtr, "getSSRCfrom"));
   RvLogLeave(rvLogPtr, (rvLogPtr, "getSSRCfrom"));
   return *(RvUint32 *)(ssrcPtr);
}


static rtcpInfo *findSSrc(rtcpSession *s, RvUint32 ssrc)
{
    RvInt32     index = 0;
    RvBool  doAgain = RV_TRUE;
    rtcpInfo *pInfo;

    RvLogEnter(rvLogPtr, (rvLogPtr, "findSSrc"));

    if (s == NULL)
        return NULL;

    /* Look for the given SSRC */
    while ((doAgain) && (index < s->sessionMembers))
    {
       if (s->participantsArray[index].ssrc == ssrc)
            doAgain = RV_FALSE;
       else
           index ++;

    }
    if (index < s->sessionMembers )
        pInfo = &s->participantsArray[index];
    else
        pInfo = NULL;

    RvLogLeave(rvLogPtr, (rvLogPtr, "findSSrc"));
    return pInfo;
}

/* insert new member to a participants table */
static rtcpInfo *insertNewSSRC(rtcpSession *s, RvUint32 ssrc)
{
    rtcpInfo* pInfo = NULL;
    RvInt32 index;

    RvLogEnter(rvLogPtr, (rvLogPtr, "insertNewSSRC"));

    if (s->sessionMembers >= s->maxSessionMembers)
    {
        /* We've got too many - see if we can remove some old ones */
        for(index=0; index < s->sessionMembers; index++)
            if (s->participantsArray[index].invalid &&
                s->participantsArray[index].ssrc == 0)
                break;
            /* BYE message can transform participant to invalid */
        /* if index < s->sessionMembers found unusable index */
    }
    else
    {
        /* Add it as a new one to the list */
        index = s->sessionMembers;
        s->sessionMembers++;
    }

    if (index < s->sessionMembers)
    {
        /* Got a place for it ! */
        pInfo = &s->participantsArray[index];
        memset(pInfo, 0, sizeof(rtcpInfo));
        pInfo->ssrc             = ssrc;
        pInfo->eToRR.ssrc       = ssrc;
        pInfo->active           = RV_FALSE;
        pInfo->src.probation    = MIN_SEQUENTIAL;
    }

    RvLogLeave(rvLogPtr, (rvLogPtr, "insertNewSSRC"));
    return pInfo;
}

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

void RvRtcpInsertNewSSRC(RvRtcpSession  hRTCP, RvUint32 ssrc, int sequence) {
   if(hRTCP) {
      rtcpInfo *fInfo = insertNewSSRC((rtcpSession *)hRTCP,ssrc);
      if(fInfo) {
         init_seq(&(fInfo->src),sequence - 1);
      }
   }
}

RvBool RvRtcpIsNewSsrc(RvRtcpSession  hRTCP, RvUint32 ssrc) {
   return (hRTCP && (findSSrc((rtcpSession*)hRTCP,ssrc)==NULL));
}

void RvRtcpDisconnectOnCollision(RvRtcpSession  hRTCP, RvUint32 ssrc) {
   
   //Send BYE to the other side, as defined in the RFC1889:
   rtcpSession *s = (rtcpSession *)hRTCP;
   RvUint32 allocated = 0;
   RvUint32 data[MAXRTCPPACKET/sizeof(RvUint32)+1];
   RvRtpBuffer bufC;
   RvUint32 packLen = 0;
   const char* reason="collision";
   RvInt32 length=strlen(reason);
   
   bufC = buffCreate(data+1, MAXRTCPPACKET-4); /* resulting packet */

   RvLockGet(&s->lock, logMgr);
   
   /* send Bye message immediately */
   
   rtcpAddRTCPPacketType(hRTCP, RTCP_EMPTY_RR, 0, 0, NULL, 0, &bufC, &allocated);
   rtcpAddRTCPPacketType(hRTCP, RTCP_SDES, 1/* set inside*/, 0, NULL, 0, &bufC, &allocated);
   rtcpAddRTCPPacketType(hRTCP, RTCP_BYE, 0, 0, (RvUint8*)reason, length, &bufC, &allocated);
   
   rtcpSessionRawSend(hRTCP, bufC.buffer, allocated, MAXRTCPPACKET, &packLen);
   
   RvLockRelease(&s->lock, logMgr);
   
   dprintf("RTCP: %s: session %d disconnected on collision\n",__FUNCTION__,ssrc);
}

#endif
/* SPIRENT_END */

                  /* == ENDS: Internal RTCP Functions == */

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)

//RTCP "no-network" functions

static 
RVAPI
RvRtcpSession RVCALLCONV RvRtcpOpenFromNoNet(
                                      IN  RvUint32    ssrc,
                                      IN  char *      cname,
                                      IN  RvInt32         maxSessionMembers,
                                      IN  void *      buffer,
                                      IN  RvInt32         bufferSize)
{
    rtcpSession* s=(rtcpSession*)buffer;
    RvSize_t allocSize=RvRtcpGetAllocationSize(maxSessionMembers);
    RvInt32 participantsArrayOfs;
    RvStatus status;

    RvLogEnter(rvLogPtr, (rvLogPtr, __FUNCTION__));

    if(!s) return NULL;

    if ((cname  == NULL) || (strlen(cname) > RTCP_MAXSDES_LENGTH) || ((RvInt32)allocSize > bufferSize))
    {
        RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpOpenFromNoNet:wrong parameter"));
        RvLogLeave(rvLogPtr, (rvLogPtr, __FUNCTION__));
        return NULL;
    }
    memset(buffer, 0, allocSize);
    
    RvLogDebug(rvLogPtr, (rvLogPtr, "RvRtcpOpenFrom: RTCP session opening"));
	
    s->sessionMembers = 1; /* my SSRC added to participant array */
    s->maxSessionMembers = maxSessionMembers;
    s->rtcpRecvCallback = NULL;
    s->isAllocated = RV_FALSE;
    s->isShutdown  = RV_FALSE;

    participantsArrayOfs = RvRoundToSize(sizeof(rtcpSession), ALIGNMENT);
    s->participantsArray = (rtcpInfo *) ((char *)s + participantsArrayOfs);
    s->participantsArray[0].ssrc = s->myInfo.ssrc = ssrc;
    s->txInterval.rtcpBandwidth = RV_RTCP_BANDWIDTH_DEFAULT;  /* an arbitrary default value */

    /* Initialize lock in supplied buffer */
    if (RvLockConstruct(logMgr, &s->lock) != RV_OK)
    {
        RvSocketDestruct(&s->socket, RV_FALSE, NULL, logMgr);
        RvLogError(rvLogPtr, (rvLogPtr, "RvRtcpOpenFromNoNet: failed to open RTCP session: RvLockConstruct() failed"));
        RvLogLeave(rvLogPtr, (rvLogPtr, __FUNCTION__));
        return NULL;
    }

    memset(&(s->myInfo.SDESbank), 0, sizeof(s->myInfo.SDESbank)); /* initialization */
    setSDES(RV_RTCP_SDES_CNAME, &(s->myInfo.SDESbank[RV_RTCP_SDES_CNAME]), (RvUint8*)cname, (RvInt32)strlen(cname));

    s->myInfo.lastPackNTP = RvUint64Const(0,0);
    s->myInfo.rtpClockRate = 0;
    RvLogLeave(rvLogPtr, (rvLogPtr, __FUNCTION__));
    return (RvRtcpSession)s;
}

RVAPI
RvRtcpSession RVCALLCONV RvRtcpOpenNoNet(
                                  IN  RvUint32    ssrc,
                                  IN  char *      cname)
{
    rtcpSession* s;
    RvInt32 allocSize = RvRtcpGetAllocationSize(RTCP_MAXRTPSESSIONMEMBERS);
    RvLogEnter(rvLogPtr, (rvLogPtr, __FUNCTION__));
    if(RvMemoryAlloc(NULL, (RvSize_t)allocSize, logMgr, (void**)&s) != RV_OK)
        return NULL;

    if((rtcpSession*)RvRtcpOpenFromNoNet(ssrc, cname, RTCP_MAXRTPSESSIONMEMBERS, (void*)s, allocSize)==NULL)
    {
        RvMemoryFree(s, logMgr);
        return NULL;
    }

    s->isAllocated = RV_TRUE;
    RvLogLeave(rvLogPtr, (rvLogPtr, __FUNCTION__));
    return (RvRtcpSession)s;
}

RVAPI
RvInt32 RVCALLCONV RvRtcpCloseNoNet(
                IN  RvRtcpSession  hRTCP)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvLogEnter(rvLogPtr, (rvLogPtr, __FUNCTION__));
    if (hRTCP == NULL)
        return RV_ERROR_UNKNOWN;

    /* Destroy the lock */
    RvLockDestruct(&s->lock, logMgr);

    /* free memory allocated for rtcpSession */
    if (s->isAllocated)
        RvMemoryFree(s, logMgr);
    RvLogLeave(rvLogPtr, (rvLogPtr, __FUNCTION__));
    return RV_OK;
}

RvStatus  RvRtcpSessionMakeShutdownByeNoNet(
    IN  RvRtcpSession hRTCP,
    IN  const char *reason,
    IN  RvInt32 length,
    INOUT  RvUint8 *data,
    INOUT  RvInt32 *dataLength)
{
    rtcpSession *s = (rtcpSession *)hRTCP;
    RvUint32 allocated = 0;
    RvRtpBuffer buf;
    RvStatus res = RV_OK;

    RvLogEnter(rvLogPtr, (rvLogPtr, __FUNCTION__));

    if(!data || !dataLength || !(*dataLength>4)) return -1;

    buf = buffCreate(data, *dataLength); /* resulting packet */

    RvLockGet(&s->lock, logMgr);

    if (!s->isShutdown)
        s->isShutdown = RV_TRUE;
    
    res = rtcpAddRTCPPacketType(hRTCP, RTCP_EMPTY_RR, 0, 0, NULL, 0, &buf, &allocated);
    res = rtcpAddRTCPPacketType(hRTCP, RTCP_SDES, 1/* set inside*/, 0, NULL, 0, &buf, &allocated);
    res = rtcpAddRTCPPacketType(hRTCP, RTCP_BYE, 0, 0, (RvUint8*)reason, length, &buf, &allocated);

    RvLockRelease(&s->lock, logMgr);

    *dataLength = allocated;

    RvLogLeave(rvLogPtr, (rvLogPtr, __FUNCTION__));
    return res;
}

RVAPI
RvInt32 RVCALLCONV RvRtcpCreateRTCPPacketNoNet(
                                               IN      RvRtcpSession  hRTCP,
                                               INOUT  RvUint8 *data,
                                               INOUT  RvInt32 *dataLength) {
   
   RvRtpBuffer buf;
   RvStatus res = RV_OK;
   
   RvLogEnter(rvLogPtr, (rvLogPtr, __FUNCTION__));
   
   if(!data || !dataLength || !(*dataLength>4)) return -1;

   buf = buffCreate(data, *dataLength); /* resulting packet */

   res = rtcpCreateRTCPPacket(hRTCP,&buf);

   if(res>=0) {
      *dataLength = buf.length;
   }
   
   RvLogLeave(rvLogPtr, (rvLogPtr, __FUNCTION__));
   return res;
}

RVAPI
RvStatus RVCALLCONV RvRtcpProcessCompoundRTCPPacketNoNet(
                                                         IN      RvRtcpSession  hRTCP,
                                                         IN RvUint8 *buffer,
                                                         RvSize_t bytesReceived)
{
   if(!hRTCP || !buffer || (bytesReceived<1)) return -1;
   else {
      
      rtcpSession* s = (rtcpSession*)hRTCP;
      RvRtpBuffer buf = {length : bytesReceived, buffer : buffer};
      RvStatus res = RV_OK;
      RvUint64      myTime = getNNTPTime();

      res=rtcpProcessCompoundRTCPPacket((RvRtcpSession)s, &buf, myTime);
      
      if ((res==0) && (s->rtcpRecvCallback != NULL))
      {
         RvUint32 ssrc=getSSRCfrom(buf.buffer);
         s->rtcpRecvCallback((RvRtcpSession)s, s->haRtcp, ssrc);
      }
      
      return res;
   }
}

#endif
/* SPIRENT_END */

#ifdef __cplusplus
}
#endif
