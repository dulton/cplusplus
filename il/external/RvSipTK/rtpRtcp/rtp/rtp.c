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


#include "rvstdio.h"
#include "rvmemory.h"
#include "rvtimestamp.h"
#include "rvrandomgenerator.h"
#include "rvhost.h"
#include "rvsocket.h"
#include "rvselect.h"
#include "rvcbase.h"

#include "bitfield.h"
#include "rtputil.h"

#include "rtp.h"
#include "rtcp.h"
#include "rvlog.h"
#include "rvrtpaddresslist.h"

#include "RtpProfileRfc3550.h"

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
#include <net/if.h>
#include <arpa/inet.h>

#include "rtp_spirent.h"
#include "rvexternal.h"
#if defined(UPDATED_BY_SPIRENT_ABACUS)
#include <abmem/abmem.h>
#endif
#include "RtcpTypes.h"
#include "rvaddress.h"
#endif
//SPIRENT_END

#if(RV_LOGMASK != RV_LOGLEVEL_NONE)   
static  RvLogSource*     localLogSource = NULL;
#define RTP_SOURCE      (localLogSource!=NULL?localLogSource:(localLogSource=rtpGetSource(RVRTP_RTP_MODULE)))
#define rvLogPtr        (localLogSource!=NULL?localLogSource:(localLogSource=rtpGetSource(RVRTP_RTP_MODULE)))
static  RvRtpLogger      rtpLogManager = NULL;
#define logMgr          (RvRtpGetLogManager(&rtpLogManager),((RvLogMgr*)rtpLogManager))
#else
#define logMgr          (NULL)
#define rvLogPtr        (NULL)
#endif
#include "rtpLogFuncs.h"

#ifdef __cplusplus
extern "C" {
#endif


#define BUILD_VER_NUM(_max, _min, _dot1, _dot2) \
    ((RvUint32)((_max << 24) + (_min << 16) + (_dot1 << 8) + _dot2))

#define VERSION_STR    "Add-on RTP/RTCP Version 2.5.0.8" 
#define VERSION_NUM    BUILD_VER_NUM(2, 5, 0, 8)

/* RTP instance to use */
static RvUint32      rtpTimesInitialized   = 0;       /* Times the RTP was initialized */
RvRtpInstance        rvRtpInstance;

/* local functions */

static void rtpEvent(
        IN RvSelectEngine*  selectEngine,
        IN RvSelectFd*      fd,
        IN RvSelectEvents   selectEvent,
        IN RvBool           error);

                       /* == Basic RTP Functions == */
/************************************************************************************
 * RvRtpInit
 * description: Initializes the instance of the RTP module. 
 * input: none
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 * Note: RvRtpInit() sets local address as IPV4 any address (IP=0.0.0.0; Port = 0)
 *       In IPV6 envinriment use RvRtpInitEx(). 
 ***********************************************************************************/

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)

RVAPI RvStatus RVCALLCONV RvRtpSeliSelectUntilPrivate(IN RvUint32 delay)
{
    RvSelectEngine* selectEngine = rvRtpInstance.selectEngine;
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
RvInt32 RVCALLCONV RvRtpInit(void)
{
    RvStatus status;
    
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpInit"));
    status = RvCBaseInit();
    if (status != RV_OK)
        return status;

    if (rtpTimesInitialized == 0)
    {
        RvUint32 numAddrs = RV_RTP_MAXIPS;
        RvUint32 countModules = 0;
        
        for (countModules = 0; countModules < RVRTP_MODULES_NUMBER; countModules++)
        {
            rvRtpInstance.logManager.bLogSourceInited[countModules] = RV_FALSE;
        }

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
        {
           unsigned int i=0;
           RvNetAddress **ips=RTP_getHostAddresses();

           numAddrs=0;
           
           for (i=0; ((i<RV_RTP_MAXIPS) && ips[i]); i++) {

              rvRtpInstance.hostIPs[numAddrs++] = *((RvAddress*)(ips[i]->address));

           }
        }
        
#else
        /* Find the list of host addresses we have */
        status = RvHostLocalGetAddress(logMgr, &numAddrs, rvRtpInstance.hostIPs);
        if (status != RV_OK)
        {
            RvCBaseEnd();
            return status;
        }
#endif
//SPIRENT_END

        rvRtpInstance.addressesNum = numAddrs;

        /* Create a random generator */
        RvRandomGeneratorConstruct(&rvRtpInstance.randomGenerator,
            (RvRandom)(RvTimestampGet(logMgr)>>8));

        /* Creating the destination addresses pool */
#if defined (RV_RTP_DEST_POOL_OBJ)

        //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
        if((status=RvRtpDestinationAddressPoolConstruct())<0) {
           return status;
        }
#else
        RvRtpDestinationAddressPoolConstruct();
#endif
//SPIRENT_END

#endif
        if (numAddrs>0)
            RvAddressCopy(&rvRtpInstance.hostIPs[0], &rvRtpInstance.rvLocalAddress);
        else
#if (RV_NET_TYPE & RV_NET_IPV6)
            RvAddressConstruct(RV_ADDRESS_TYPE_IPV6, &rvRtpInstance.rvLocalAddress);
#else
            RvAddressConstruct(RV_ADDRESS_TYPE_IPV4, &rvRtpInstance.rvLocalAddress);
#endif
    }
    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    status = RvSelectConstruct(SPIRENT_RTP_ENGINE_SIZE*2, SPIRENT_RTP_ENGINE_SIZE, logMgr, &rvRtpInstance.selectEngine);
#else
    status = RvSelectConstruct(2048, 1024, logMgr, &rvRtpInstance.selectEngine);
#endif
    //SPIRENT_END
    if (status != RV_OK)
    {
      //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
      rvRtpInstance.selectEngine=NULL;
#endif
      //SPIRENT_END
      RvCBaseEnd();
      return status;
    }
    rtpTimesInitialized++;
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpInit"));
    return RV_OK;
}

#ifdef RVRTP_OLD_CONVENTION_API
RVAPI
RvInt32 RVCALLCONV rtpInitEx(RvUint32 ip)
{
    RvInt32 rc;
    if ((rc = RvRtpInit()) != RV_ERROR_UNKNOWN)		
    {
        RvAddressConstructIpv4(&rvRtpInstance.rvLocalAddress, ip, RV_ADDRESS_IPV4_ANYPORT);
    }
    return rc;
}
#endif

/************************************************************************************
 * RvRtpInitEx
 * description: Initializes the RTP Stack and specifies the local IP address 
 *              to which all RTP sessions will be bound. 
 * input: pRvRtpAddress - pointer to RvNetAddress, which specifies 
 *		the local IP (IPV4/6) address to which all RTP sessions will be bound. 
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 * Remarks 
 * - This function can be used instead of RvRtpInit().
 * - RvRtpInit() binds to the “any” IPV4 address. 
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtpInitEx(RvNetAddress* pRtpAddress)
{
    RvInt32 rc = -1;
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpInitEx"));
	if (NULL!=pRtpAddress && RvNetGetAddressType(pRtpAddress)!=RVNET_ADDRESS_NONE)
	{
		if ((rc=RvRtpInit()) != RV_ERROR_UNKNOWN)
            RvAddressCopy((RvAddress*)pRtpAddress->address, &rvRtpInstance.rvLocalAddress);
	}
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpInitEx"));
    return rc;
}

/************************************************************************************
 * rtpSetLocalAddress
 * description: Set the local address to use for calls to rtpOpenXXX functions.
 *              This parameter overrides the value given in rtpInitEx() for all
 *              subsequent calls.
 * input: ip    - Local IP address to use
 * output: none.
 * return value: Non-negative value on success
 *               negative value on failure
 ***********************************************************************************/
#ifdef RVRTP_OLD_CONVENTION_API
RVAPI
RvInt32 RVCALLCONV rtpSetLocalAddress(IN RvUint32 ip)
{
	RvNetAddress rtpAddress;
    RvNetIpv4   Ipv4;
    Ipv4.ip = ip;
    Ipv4.port = 0;
    RvNetCreateIpv4(&rtpAddress, &Ipv4);
	return RvRtpSetLocalAddress(&rtpAddress);
}
#endif

RVAPI
RvInt32 RVCALLCONV RvRtpSetLocalAddress(IN RvNetAddress* pRtpAddress)
{
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpSetLocalAddress"));
	if (NULL!=pRtpAddress)
	{
        RvAddressCopy((RvAddress*)pRtpAddress->address, &rvRtpInstance.rvLocalAddress);
	}
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpSetLocalAddress"));
    return RV_OK;
}
/************************************************************************************
 * RvRtpEnd
 * description: Shuts down an instance of an RTP module. 
 * input: none.
 * output: none.
 * return value: none.
 ***********************************************************************************/
RVAPI
void RVCALLCONV RvRtpEnd(void)
{
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpEnd"));
    rtpTimesInitialized--;

    if (rtpTimesInitialized == 0)
    {
      RvRandomGeneratorDestruct(&rvRtpInstance.randomGenerator);
      //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
      RvSelectDestruct(rvRtpInstance.selectEngine, SPIRENT_RTP_ENGINE_SIZE);
      rvRtpInstance.selectEngine=NULL;
#else
      RvSelectDestruct(rvRtpInstance.selectEngine, 0);
#endif
      //SPIRENT_END

#if defined(RV_RTP_DEST_POOL_OBJ)
      RvRtpDestinationAddressPoolDestruct();
#endif
      RvCBaseEnd();    
    }
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpEnd"));
}

/************************************************************************************
 * RvRtpGetAllocationSize
 * description: returns the number of bytes that the application should allocate 
 *              in memory for an RTP session data structure. 
 * input:  none.
 * output: none.
 * return value: The number of bytes required for (internal) RTP session data structure.
 * Note:
 *            This function works together with the RvRtpOpenFrom() function. 
 *            RvRtpGetAllocationSize() indicates to the application the size of the buffer, 
 *            which is one of the parameters required by the RvRtpOpenFrom() function. 
 ***********************************************************************************/

RVAPI
RvInt32 RVCALLCONV RvRtpGetAllocationSize(void)
{
    return (RvRoundToSize(sizeof(RvRtpSessionInfo), RV_ALIGN_SIZE) 
          + RvRoundToSize(sizeof(RtpProfilePlugin), RV_ALIGN_SIZE));
}

/************************************************************************************
 * RvRtpOpen
 * description: Opens an RTP session. The RTP Stack allocates an object and the
 *              memory needed for the RTP session. It also opens a socket and waits
 *              for packets. RvRtpOpen() also returns the handle of this session to
 *              the application.
 * input: pRtpAddress contains  the UDP port number to be used for the RTP session.
 *        ssrcPattern - Synchronization source Pattern value for the RTP session.
 *        ssrcMask    - Synchronization source Mask value for the RTP session.
 * output: none.
 * return value: If no error occurs, the function returns the handle for the opened RTP
 *               session. Otherwise, it returns NULL.
 * Note:
 *     RvRtpOpen opens one socket with the same port for receiving and for sending.
 ***********************************************************************************/
#ifdef RVRTP_OLD_CONVENTION_API
RVAPI
RvRtpSession RVCALLCONV rtpOpenFrom(
        IN  RvUint16    port,
        IN  RvUint32    ssrcPattern,
        IN  RvUint32    ssrcMask,
        IN  void*       buffer,
        IN  RvInt32         bufferSize)
{
	RvNetAddress rtpAddress;
    RvNetIpv4   Ipv4;
    Ipv4.ip =   0;
    Ipv4.port = port;
    RvNetCreateIpv4(&rtpAddress, &Ipv4);
	return RvRtpOpenFrom(&rtpAddress, ssrcPattern, ssrcMask, buffer, bufferSize);
}
#endif

RVAPI
RvRtpSession RVCALLCONV RvRtpOpenFrom(
									IN  RvNetAddress* pRtpAddress,
									IN  RvUint32    ssrcPattern,
									IN  RvUint32    ssrcMask,
									IN  void*       buffer,
									IN  RvInt32         bufferSize)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)buffer;
    RvRandom         randomValue;
    RvStatus         res;
    RvAddress        localAddress;
    RtpProfileRfc3550* rtpProfilePtr = NULL;

	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpOpenFrom"));
	
	if (NULL==pRtpAddress||RvNetGetAddressType(pRtpAddress)==RVNET_ADDRESS_NONE)
	{
		RvLogError(rvLogPtr, (rvLogPtr, "RvRtpOpenFrom: NULL pointer of wrong address type"));
		RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpOpenFrom"));
        return NULL;
	}
    if (bufferSize < RvRtpGetAllocationSize())
	{
		RvLogError(rvLogPtr, (rvLogPtr, "RvRtpOpenFrom: no room to open session"));
		RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpOpenFrom"));	
		return NULL;
	}

    memset(buffer, 0 , (RvSize_t)RvRtpGetAllocationSize());

	if (RvNetGetAddressType(pRtpAddress) == RVNET_ADDRESS_IPV6)
	{
#if (RV_NET_TYPE & RV_NET_IPV6)
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
      RvAddressConstructIpv6(&localAddress, 
			RvAddressIpv6GetIp(RvAddressGetIpv6((RvAddress*)pRtpAddress->address)),
			RvAddressGetIpPort((RvAddress*)pRtpAddress->address),
			RvAddressGetIpv6Scope((RvAddress*)pRtpAddress->address),
         RvAddressIpv6GetFlowinfo(RvAddressGetIpv6((RvAddress*)pRtpAddress->address)));
#else
		RvAddressConstructIpv6(&localAddress, 
			RvAddressIpv6GetIp(RvAddressGetIpv6((RvAddress*)pRtpAddress->address)),
			RvAddressGetIpPort((RvAddress*)pRtpAddress->address),
			RvAddressGetIpv6Scope((RvAddress*)pRtpAddress->address));
#endif
//SPIRENT_END
		
		/* Open and bind the socket */
		res = RvSocketConstruct(RV_ADDRESS_TYPE_IPV6, RvSocketProtocolUdp, logMgr, &s->socket);
#else
		RvLogError(rvLogPtr, (rvLogPtr, "RvRtpOpenFrom: IPV6 is not supported in current configuration"));
		RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpOpenFrom"));	
		return NULL;
#endif
	}
    else
	{
		RvAddressConstructIpv4(&localAddress, 
			RvAddressIpv4GetIp(RvAddressGetIpv4((RvAddress*)pRtpAddress->address)),
			RvAddressGetIpPort((RvAddress*)pRtpAddress->address));
		/* Open and bind the socket */
		res = RvSocketConstruct(RV_ADDRESS_TYPE_IPV4, RvSocketProtocolUdp, logMgr, &s->socket);
	}
    /* Get a random value for the beginning sequence number */
    RvRandomGeneratorGetValue(&rvRtpInstance.randomGenerator, &randomValue);
	RvLogDebug(rvLogPtr, (rvLogPtr, "RvRtpOpenFrom:generated random value %#x", randomValue));	
    /* Encryption callbacks, key and e. mode must be set after opening of session
	s->encryptionPlugInPtr = NULL;
	s->encryptionKeyPlugInPtr = NULL;   NULL callbacks automatically by memset*/
	s->isAllocated    = RV_FALSE;
    s->sSrcPattern    = ssrcPattern;
    s->sSrcMask       = ssrcMask;
    s->sequenceNumber = (RvUint16)randomValue;

    if (res == RV_OK)
    {
      /* SPIRENT_BEGIN */
      /*
      * COMMENTS: Socket HW binding to Ethernet Interface
      */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
      if(smp_system()) {
        RvSocketSetBuffers(&s->socket, 16384, 16384, logMgr);
      } else {
        RvSocketSetBuffers(&s->socket, 8192, 8192, logMgr);
      }
#elif defined(UPDATED_BY_SPIRENT)
      RvSocketSetBuffers(&s->socket, 4096, 4096, logMgr);
#else
      RvSocketSetBuffers(&s->socket, 8192, 8192, logMgr);
#endif  //#if defined(UPDATED_BY_SPIRENT)
      /* SPIRENT_END */
      res = RvSocketSetBroadcast(&s->socket, RV_TRUE, logMgr);
      if (res == RV_OK)
        res = RvSocketSetBlocking(&s->socket, RV_TRUE, logMgr);
      /* SPIRENT_BEGIN */
      /*
      * COMMENTS: Socket HW binding to Ethernet Interface
      */
#if defined(UPDATED_BY_SPIRENT)
            Spirent_RvSocketBindDevice(&s->socket, pRtpAddress->if_name, logMgr);
#endif  //#if defined(UPDATED_BY_SPIRENT)
/* SPIRENT_END */
        if (res == RV_OK)
            res = RvSocketBind(&s->socket, &localAddress, NULL, logMgr);

        if (res != RV_OK)
            RvSocketDestruct(&s->socket, RV_FALSE, NULL, logMgr);
    }
	RvLogDebug(rvLogPtr, (rvLogPtr, "RvRtpOpenFrom: socket related initializations completed, status = %d", res));
    if (res == RV_OK)
    {
        res = RvLockConstruct(logMgr, &s->lock); 
    }
    else
    {	
        RTPLOG_ERROR_LEAVE(OpenFrom, "failed");
        return (RvRtpSession)NULL;
    }
    if (res == RV_OK)
    {
        RvRtpRegenSSRC((RvRtpSession)s);
	    RvRtpAddressListConstruct(&s->addressList);	
       //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
       memset(&s->remoteAddress,0,sizeof(s->remoteAddress));
       s->socklen=0;
#endif
       //SPIRENT_END
    }
    else
    {	
        RvLockDestruct(&s->lock, logMgr); 
        RvSocketDestruct(&s->socket, RV_FALSE, NULL, logMgr);
        RTPLOG_ERROR_LEAVE(OpenFrom, "failed");
        return (RvRtpSession)NULL;
    }

/* SPIRENT_BEGIN */
/*
 * COMMENTS: Socket timestamp service
 */
#if defined(UPDATED_BY_SPIRENT)
		{
         RvUint32 yes = RV_TRUE;
         if( RvSocketSetSockOpt(&s->socket, SOL_SOCKET, SO_TIMESTAMP , (char*)&yes, sizeof(yes)) != 0)
         {
            RTPLOG_ERROR_LEAVE(OpenFrom,"Setsockopt(SO_TIMESTAMP) parameter setting failed");
         }
		}
#endif
/* SPIRENT_END */
    if (res == RV_OK)
    {
        rtpProfilePtr = (RtpProfileRfc3550*) (((RvUint8*)buffer) + RvRoundToSize(sizeof(RvRtpSessionInfo), RV_ALIGN_SIZE));
        RtpProfileRfc3550Construct(rtpProfilePtr);
        s->profilePlugin = RtpProfileRfc3550GetPlugIn(rtpProfilePtr);
        RTPLOG_INFO((RTP_SOURCE, "RTP session opened (handle = %#x)", (RvUint32)s));        
        RTPLOG_LEAVE(OpenFrom);
        return (RvRtpSession)s;
    }
    else
    {
        RvLockDestruct(&s->lock, logMgr); 
        RvRtpAddressListDestruct(&s->addressList);	
        //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
        memset(&s->remoteAddress,0,sizeof(s->remoteAddress));
        s->socklen=0;
#endif
       //SPIRENT_END
        RvSocketDestruct(&s->socket, RV_FALSE, NULL, logMgr);
        RTPLOG_ERROR_LEAVE(OpenFrom, "failed");
        return (RvRtpSession)NULL;
    }
}

/************************************************************************************
 * RvRtpOpen
 * description: Opens an RTP session. The RTP Stack allocates an object and the
 *              memory needed for the RTP session. It also opens a socket and waits
 *              for packets. RvRtpOpen() also returns the handle of this session to
 *              the application.
 * input: pRtpAddress contains  the UDP port number to be used for the RTP session.
 *        ssrcPattern - Synchronization source Pattern value for the RTP session.
 *        ssrcMask    - Synchronization source Mask value for the RTP session.
 * output: none.
 * return value: If no error occurs, the function returns the handle for the opened RTP
 *               session. Otherwise, it returns NULL.
 * Note:
 *     RvRtpOpen opens one socket with the same port for receiving and for sending.
 ***********************************************************************************/
#ifdef RVRTP_OLD_CONVENTION_API
RVAPI
RvRtpSession RVCALLCONV rtpOpen(
        IN  RvUint16    port,
        IN  RvUint32    ssrcPattern,
        IN  RvUint32    ssrcMask)
{
	RvNetAddress rtpAddress;
    RvNetIpv4   Ipv4;
    Ipv4.ip = 0;
    Ipv4.port = port;
    RvNetCreateIpv4(&rtpAddress, &Ipv4);
    return RvRtpOpenEx(&rtpAddress, ssrcPattern, ssrcMask, NULL);
}
#endif
RVAPI
RvRtpSession RVCALLCONV RvRtpOpen(	  IN  RvNetAddress* pRtpAddress,
									  IN  RvUint32    ssrcPattern,
									  IN  RvUint32    ssrcMask)
{
    RvRtpSession s = NULL;
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpOpen"));
	s = RvRtpOpenEx(pRtpAddress, ssrcPattern, ssrcMask, NULL);
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpOpen"));
	return s;
}
/************************************************************************************
 * RvRtpOpenEx
 * description: Opens an RTP session and an associated RTCP session.
 * input: pRtpAddress contains  the UDP port number to be used for the RTP session.
 *        ssrcPattern - Synchronization source Pattern value for the RTP session.
 *        ssrcMask    - Synchronization source Mask value for the RTP session.
 *        cname       - The unique name representing the source of the RTP data.
 * output: none.
 * return value: If no error occurs, the function returns the handle for the open
 *               RTP session. Otherwise, the function returns NULL.
 * Note:
 * RvRtpOpenEx opens one socket for RTP session with the same port for receiving
 * and for sending, and one for RTCP session with the next port for receiving
 * and for sending.
 ***********************************************************************************/
#ifdef RVRTP_OLD_CONVENTION_API
RVAPI
RvRtpSession RVCALLCONV rtpOpenEx(
        IN  RvUint16    port,
        IN  RvUint32    ssrcPattern,
        IN  RvUint32    ssrcMask,
        IN  char *      cname)
{
	RvNetAddress rtpAddress;
    RvNetIpv4 Ipv4;
    Ipv4.ip = 0;
    Ipv4.port = port;
    RvNetCreateIpv4(&rtpAddress, &Ipv4);
	return RvRtpOpenEx(&rtpAddress, ssrcPattern, ssrcMask, cname);
}
#endif
RVAPI
RvRtpSession RVCALLCONV RvRtpOpenEx(
								  IN  RvNetAddress* pRtpAddress,
								  IN  RvUint32    ssrcPattern,
								  IN  RvUint32    ssrcMask,
								  IN  char *      cname)
{
    /* allocate rtp session.*/
    RvRtpSessionInfo* s;
    RvStatus res;
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpOpenEx"));
	
	if (NULL==pRtpAddress||RvNetGetAddressType(pRtpAddress)==RVNET_ADDRESS_NONE)
	{
		RvLogError(rvLogPtr, (rvLogPtr, "RvRtpOpenEx: NULL pointer or wrong address type"));
		RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpOpenEx"));
        return NULL;
	}
    res = RvMemoryAlloc(NULL, (RvSize_t)RvRtpGetAllocationSize(), logMgr, (void**)&s);
    if (res != RV_OK)
	{
		RvLogError(rvLogPtr, (rvLogPtr, "RvRtpOpenEx: cannot allocate RTP session"));
		RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpOpenEx"));
        return NULL;
	}
    if ((RvRtpSessionInfo *)RvRtpOpenFrom(pRtpAddress, ssrcPattern, ssrcMask, (void*)s, RvRtpGetAllocationSize())==NULL)
    {
        RvMemoryFree(s, logMgr);
		RvLogError(rvLogPtr, (rvLogPtr, "RvRtpOpenEx: RvRtpOpenFrom() failed"));
		RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpOpenEx"));
        return NULL;
    }
    s->isAllocated=RV_TRUE;
    if (cname)
    {
        /* Open a new rtcp session.The port for an RTCP session is always
           (RTP port + 1).*/
        RvNetAddress rtcpAddress;
		
		RvLogDebug(rvLogPtr, (rvLogPtr, "RvRtpOpenEx: RTP openning with automatic RTCP"));
		memcpy(&rtcpAddress, pRtpAddress, sizeof(RvNetAddress));
		if (RvNetGetAddressType(pRtpAddress)==RVNET_ADDRESS_IPV6)
		{
#if (RV_NET_TYPE & RV_NET_IPV6)
			RvNetIpv6 Ipv6;		
			RvNetGetIpv6(&Ipv6, pRtpAddress);
			Ipv6.port = (RvUint16)((Ipv6.port)?Ipv6.port+1:Ipv6.port);
			RvNetCreateIpv6(&rtcpAddress,&Ipv6);
#else
			RvLogError(rvLogPtr, (rvLogPtr, "RvRtpOpenEx: IPV6 is not supported in current configuration"));
		    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpOpenEx"));
			return NULL;
#endif
		}
        else	 
		{
			RvNetIpv4 Ipv4;	
			RvNetGetIpv4(&Ipv4, pRtpAddress);
			Ipv4.port = (RvUint16)((Ipv4.port)?Ipv4.port+1:Ipv4.port);
#if defined(UPDATED_BY_SPIRENT)
                        // CR#318341741
                        RvAddressConstructIpv4((RvAddress*)rtcpAddress.address, Ipv4.ip, Ipv4.port);
#else
			RvNetCreateIpv4(&rtcpAddress,&Ipv4);
#endif
		}

        s->hRTCP = RvRtcpOpen(s->sSrc, &rtcpAddress, cname);
        if (s->hRTCP == NULL)
        {
            /* Bad RTCP - let's just kill the RTP and be done with it */
            RvRtpClose((RvRtpSession)s);
            s = NULL;
			RvLogError(rvLogPtr, (rvLogPtr, "RvRtpOpenEx: RvRtcpOpen failed"));
		    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpOpenEx"));
			return NULL;
        }
		else
		{
            rtcpSetRtpSession(s->hRTCP, (RvRtpSession)s);
			RvRtcpSetEncryptionNone(s->hRTCP);
			RvRtcpSessionSetEncryptionMode(s->hRTCP, RV_RTPENCRYPTIONMODE_RFC1889);
            RvRtcpSetProfilePlugin(s->hRTCP, s->profilePlugin);
		}
    }
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpOpenEx"));
    return (RvRtpSession)s;
}
/************************************************************************************
 * RvRtpClose
 * description: Close RTP session.
 * input: hRTP - Handle of the RTP session.
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/
RVAPI
RvUint32 RVCALLCONV RvRtpClose(
        IN  RvRtpSession  hRTP)
{
    RvRtpSessionInfo *s            = (RvRtpSessionInfo *)hRTP;
    
	RTPLOG_ENTER(Close);
    if (s)
    {
        if (s->hRTCP)
		{
            RTPLOG_DEBUG((RTP_SOURCE, "RvRtpClose(%#x):  closing RTCP session", (RvUint32)s));
            RvRtcpClose(s->hRTCP);
			s->hRTCP = NULL;
		}

        if (s->eventHandler != NULL)
        {
              /* We're probably also in non-blocking mode */

              /* Don't free session here.
                 Instead of this register to WRITE event and free it
                 on the WRITE event handling.
                 The event handling is performed from select() thread.
                 In this way we ensure that the session memory is not freed
                 from the current thread, while any event is being handled
                 from the select() thread. */
              RvSelectUpdate(s->selectEngine, &s->selectFd, RvSelectWrite, rtpEvent);
              return RV_OK;
        }
        else
        {
            /* Send UDP data through specified socket to the local host */
            RvRtpResume(hRTP);
        }

        RTPLOG_DEBUG((RTP_SOURCE, "RvRtpClose(%#x): closing the socket", (RvUint32)s));
        /* This function closes the specified IP socket and all the socket's connections.*/
        RvSocketDestruct(&s->socket, RV_FALSE, NULL, logMgr);
			
		RvLogDebug(rvLogPtr, (rvLogPtr, "RvRtpClose: removing the address list"));
        RvRtpRemoveAllRemoteAddresses(hRTP);
		RvRtpAddressListDestruct(&s->addressList);
      //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
      memset(&s->remoteAddress,0,sizeof(s->remoteAddress));
      s->socklen=0;
#endif
       //SPIRENT_END
        {
            RtpProfilePlugin* pluginPtr = s->profilePlugin;
            if (pluginPtr != NULL && pluginPtr->funcs!=NULL && pluginPtr->funcs->release != NULL)
                pluginPtr->funcs->release(pluginPtr, hRTP);
            s->profilePlugin = NULL;         
        }          
		if (s->encryptionKeyPlugInPtr!=NULL && s->encryptionKeyPlugInPtr->release!=NULL)
		{
		    RvLogDebug(rvLogPtr, (rvLogPtr, "RvRtpClose: releasing the encryption key plug-in"));
			s->encryptionKeyPlugInPtr->release(s->encryptionKeyPlugInPtr);
		}
        RvLockDestruct(&s->lock, logMgr);
        if (s->isAllocated)
		{
            RTPLOG_DEBUG((RTP_SOURCE, "RvRtpClose(%#x): releasing the memory ", (RvUint32)s));
            RvMemoryFree(s, logMgr);
		}
        RTPLOG_INFO((RTP_SOURCE, "Closed RTP session (%#x)", (RvUint32)s));
    }
	RTPLOG_LEAVE(Close);
    return RV_OK;
}

/************************************************************************************
 * rtpGetSSRC
 * description: Returns the current SSRC (synchronization source value) of the RTP session.
 * input: hRTP - Handle of the RTP session.
 * output: none.
 * return value: If no error occurs, the function returns the current SSRC value.
 *               Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
RvUint32 RVCALLCONV RvRtpGetSSRC(
        IN  RvRtpSession  hRTP)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpGetSSRC"));
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpGetSSRC"));
    return s->sSrc;
}

/************************************************************************************
 * RvRtpSetEventHandler
 * description: Set an Event Handler for the RTP session. The application must set
 *              an Event Handler for each RTP session.
 * input: hRTP          - Handle of the RTP session.
 *        eventHandler  - Pointer to the callback function that is called each time a
 *                        new RTP packet arrives to the RTP session.
 *        context       - The parameter is an application handle that identifies the
 *                        particular RTP session. The application passes the handle to
 *                        the Event Handler.
 * output: none.
 * return value: none.
 ***********************************************************************************/
RVAPI
void RVCALLCONV RvRtpSetEventHandler(
        IN  RvRtpSession        hRTP,
        IN  RvRtpEventHandler_CB  eventHandler,
        IN  void *             context)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
    RvBool hasFd, addFd;
    
    RTPLOG_ENTER(SetEventHandler);
    if (s == NULL)
    {
        RTPLOG_ERROR_LEAVE(SetEventHandler, "NULL session handle");
        return;
    }
    
    hasFd = (s->eventHandler != NULL);
    addFd = (eventHandler != NULL);
    s->eventHandler = eventHandler;
    s->context      = context;
    
    /* Seems like we have to add it to our list of fds */
    if (addFd && (!hasFd))
    {
        RvStatus status;
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
            s->selectEngine = rvRtpInstance.selectEngine;

        RvSocketSetBlocking(&s->socket, RV_FALSE, logMgr);
        RvFdConstruct(&s->selectFd, &s->socket, logMgr);
        RvSelectAdd(s->selectEngine, &s->selectFd, RvSelectRead, rtpEvent);
    }
    
    /* Seems like we have to remove it from our list of fds */
    if (!addFd && hasFd)
    {
        RvSelectRemove(s->selectEngine, &s->selectFd);
        RvFdDestruct(&s->selectFd);
        RvSocketSetBlocking(&s->socket, RV_TRUE, logMgr);
    }
    if (eventHandler != NULL)
    {            
        RTPLOG_INFO((RTP_SOURCE, "Set RTP Event Handler for non-blocking read for session %#x",s));
    }
    else
    {
        RTPLOG_INFO((RTP_SOURCE, "Removed RTP Event Handler for non-blocking read for session %#x",s));
    }
    RTPLOG_LEAVE(SetEventHandler);
}

/************************************************************************************
 * rtpSetRemoteAddress
 * description: Defines the address of the remote peer or the address of a multicast
 *              group to which the RTP stream will be sent.
 * input: hRTP  - Handle of the RTP session.
 *        ip    - IP address to which RTP packets should be sent.
 *        port  - UDP port to which RTP packets should be sent.
 * output: none.
 * return value: none.
 ***********************************************************************************/
#ifdef RVRTP_OLD_CONVENTION_API
RVAPI
void RVCALLCONV rtpSetRemoteAddress(
        IN RvRtpSession  hRTP,   /* RTP Session Opaque Handle */
        IN RvUint32     ip,
        IN RvUint16     port)
{
	RvNetAddress rtpAddress;
    RvNetIpv4 Ipv4;
    Ipv4.ip = ip;
    Ipv4.port = port;
    RvNetCreateIpv4(&rtpAddress, &Ipv4);
	RvRtpSetRemoteAddress(hRTP, &rtpAddress);
}
#endif
RVAPI
void RVCALLCONV RvRtpSetRemoteAddress(
									IN RvRtpSession  hRTP,   /* RTP Session Opaque Handle */
									IN RvNetAddress* pRtpAddress)

{
    RvRtpSessionInfo* s = (RvRtpSessionInfo *)hRTP;
    RvAddress* pRvAddress =  NULL;
    RTPLOG_ENTER(SetRemoteAddress);    
    
    if ((s == NULL)||(pRtpAddress == NULL)||(RvNetGetAddressType(pRtpAddress)== RVNET_ADDRESS_NONE))
    {
        RTPLOG_ERROR_LEAVE(SetRemoteAddress, "NULL pointer or wrong address type");      
        return;
    }

    RvLockGet(&s->lock, logMgr);  
    if (s != NULL && pRtpAddress!=NULL && RvNetGetAddressType(pRtpAddress)!= RVNET_ADDRESS_NONE)
    {
        if (s->remoteAddressSet)
        {
            RvRtpRemoveAllRemoteAddresses(hRTP);
        }
        pRvAddress = (RvAddress*) pRtpAddress->address;
        if (s->profilePlugin != NULL && 
            s->profilePlugin->funcs != NULL &&
            s->profilePlugin->funcs->addRemAddress != NULL)
        {
            s->profilePlugin->funcs->addRemAddress(s->profilePlugin, hRTP, RV_TRUE, pRtpAddress);
        }        
		RvRtpAddressListAddAddress(&s->addressList, pRvAddress);
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
      memcpy(&s->remoteAddress,pRvAddress,sizeof(s->remoteAddress));
      RvSocketAddressToSockAddrExternal(pRvAddress,(RvUint8*)s->sockdata,&s->socklen);
#endif
/* SPIRENT_END */
        s->remoteAddressSet = RV_TRUE;
    }  
    RvLockRelease(&s->lock, logMgr);
    RTPLOG_LEAVE(SetRemoteAddress);
}
/************************************************************************************
 * RvRtpAddRemoteAddress
 * description: Adds the new RTP address of the remote peer or the address of a multicast
 *              group or of multiunicast address list to which the RTP stream will be sent.
 * input: hRTP  - Handle of the RTP session.
 *        pRtpAddress contains
 *            ip    - IP address to which RTP packets should be sent.
 *            port  - UDP port to which RTP packets should be sent.
 * output: none.
 * return value: none.
 ***********************************************************************************/
RVAPI
void RVCALLCONV RvRtpAddRemoteAddress(
	IN RvRtpSession  hRTP,   /* RTP Session Opaque Handle */
	IN RvNetAddress* pRtpAddress)
									  
{
    RvRtpSessionInfo* s = (RvRtpSessionInfo *)hRTP;
    RvAddress* pRvAddress =  NULL;

    RTPLOG_ENTER(AddRemoteAddress);    
    if ((s == NULL)||(pRtpAddress == NULL)||(RvNetGetAddressType(pRtpAddress)== RVNET_ADDRESS_NONE))
    {
        RTPLOG_ERROR_LEAVE(AddRemoteAddress, "NULL pointer or wrong address type");      
        return;
    }
    RvLockGet(&s->lock, logMgr);
    if (s != NULL && pRtpAddress!=NULL && RvNetGetAddressType(pRtpAddress)!= RVNET_ADDRESS_NONE)
    {
        RvChar addressStr[64];
        pRvAddress = (RvAddress*) pRtpAddress->address;
        
        RV_UNUSED_ARG(addressStr); /* in case of RV_LOGLEVEL_NONE warning fix */

        if (s->profilePlugin != NULL && 
            s->profilePlugin->funcs != NULL &&
            s->profilePlugin->funcs->addRemAddress != NULL)
        {
            s->profilePlugin->funcs->addRemAddress(s->profilePlugin, hRTP, RV_TRUE, pRtpAddress);
        }

		RvRtpAddressListAddAddress(&s->addressList, pRvAddress);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
      memcpy(&s->remoteAddress,pRvAddress,sizeof(s->remoteAddress));
      RvSocketAddressToSockAddrExternal(pRvAddress,(RvUint8*)s->sockdata,&s->socklen);
#endif
/* SPIRENT_END */
        s->remoteAddressSet = RV_TRUE;

        RTPLOG_INFO((RTP_SOURCE,"Added remote address %s port =%d to the RTP session %#x", 
            RvAddressGetString(pRvAddress, sizeof(addressStr), addressStr),
            RvAddressGetIpPort(pRvAddress), (RvUint32)hRTP));
#if OLD_4_REF
		if (s->hRTCP!=NULL) /* automatic RTCP session opening needed */
		{
            RvNetAddress rtcpAddress;
			memcpy(&rtcpAddress, pRtpAddress, sizeof(RvNetAddress));
            RvAddressSetIpPort((RvAddress*)&rtcpAddress, (RvUint16)(RvAddressGetIpPort((RvAddress*)&rtcpAddress)+1));
            RTPLOG_INFO((RTP_SOURCE,"Trying to add remote address %s port =%d to the RTCP session %#x", 
                RvAddressGetString((RvAddress*)&rtcpAddress, sizeof(addressStr), addressStr),
                RvAddressGetIpPort(pRvAddress)+1, (RvUint32)s->hRTCP));
            RvRtcpAddRemoteAddress(s->hRTCP, &rtcpAddress);
		}
#endif
    } 
    RvLockRelease(&s->lock, logMgr);
    RTPLOG_LEAVE(AddRemoteAddress);
}
/************************************************************************************
 * RvRtpRemoveRemoteAddress
 * description: removes the specified RTP address of the remote peer or the address 
 *              of a multicast group or of multiunicast address list to which the 
 *              RTP stream was sent.
 * input: hRTP  - Handle of the RTP session.
 *        pRtpAddress contains
 *            ip    - IP address to which RTP packets should be sent.
 *            port  - UDP port to which RTP packets should be sent.
 * output: none.
 * return value:If an error occurs, the function returns a negative value.
 *              If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvRtpRemoveRemoteAddress(
	IN RvRtpSession  hRTP,
	IN RvNetAddress* pRtpAddress)
{
    RvRtpSessionInfo* s = (RvRtpSessionInfo *)hRTP;		
    RvAddress* pRvAddress =  NULL;
    RvChar addressStr[64];
  
    RV_UNUSED_ARG(addressStr); /* in case of RV_LOGLEVEL_NONE warning fix */    
    RTPLOG_ENTER(RemoveRemoteAddress);	
    if ((s == NULL)||(pRtpAddress == NULL)||(RvNetGetAddressType(pRtpAddress)== RVNET_ADDRESS_NONE))
    {
        RTPLOG_ERROR_LEAVE(RemoveRemoteAddress, "NULL pointer or wrong address type");      
        return RV_ERROR_UNKNOWN;
    }
    RvLockGet(&s->lock, logMgr);
    pRvAddress = (RvAddress*) pRtpAddress->address;
    RvRtpAddressListRemoveAddress(&s->addressList, pRvAddress);
    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(RvAddressCompare(&s->remoteAddress, pRvAddress, RV_ADDRESS_FULLADDRESS) == RV_TRUE) {
       memset(&s->remoteAddress,0,sizeof(s->remoteAddress));
       s->socklen=0;
    }
#endif
       //SPIRENT_END
    if (s->profilePlugin != NULL && s->profilePlugin->funcs!=NULL &&
        s->profilePlugin->funcs->removeRemAddress != NULL)
    {
        s->profilePlugin->funcs->removeRemAddress(s->profilePlugin, hRTP, pRtpAddress);
    }
   /* srtpRemoveRemoteAddress(hRTP, pRtpAddress);  if SRTP is not inited returns error. that's ok */
    RTPLOG_INFO((RTP_SOURCE,"Removed remote address %s port =%d from the RTP session %#x", 
        RvAddressGetString(pRvAddress, sizeof(addressStr), addressStr),
        RvAddressGetIpPort(pRvAddress), (RvUint32)hRTP));    
#if OLD_4_REF
    if (s->hRTCP!=NULL) /* automatic RTCP session opening needed */
    {
        RvNetAddress rtcpAddress;
        memcpy(&rtcpAddress, pRtpAddress, sizeof(RvNetAddress));
        RvAddressSetIpPort((RvAddress*)&rtcpAddress, (RvUint16)(RvAddressGetIpPort((RvAddress*)&rtcpAddress)+1));
        RTPLOG_INFO((RTP_SOURCE,"Trying to remove remote address %s port =%d from the RTCP session %#x", 
            RvAddressGetString((RvAddress*)&rtcpAddress, sizeof(addressStr), addressStr),
            RvAddressGetIpPort(pRvAddress), (RvUint32)s->hRTCP));
        RvRtcpRemoveRemoteAddress(s->hRTCP, &rtcpAddress);
    }
#endif
    pRvAddress = RvRtpAddressListGetNext(&s->addressList, NULL);
    if (NULL == pRvAddress)
        s->remoteAddressSet = RV_FALSE;
    RvLockRelease(&s->lock, logMgr);    
    RTPLOG_LEAVE(RemoveRemoteAddress);	
    return RV_OK;
}
/************************************************************************************
 * RvRtpRemoveAllRemoteAddresses
 * description: removes all RTP addresses of the remote peer or the address 
 *              of a multicast group or of multiunicast address list to which the 
 *              RTP stream was sent.
 * input: hRTP  - Handle of the RTP session.
 * output: none.
 * return value:If an error occurs, the function returns a negative value.
 *              If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvRtpRemoveAllRemoteAddresses(IN RvRtpSession  hRTP)
{
    RvRtpSessionInfo* s          = (RvRtpSessionInfo *)hRTP;	
    RvAddress*        pRvAddress = NULL;

    RTPLOG_ENTER(RemoveAllRemoteAddresses);
    if (s == NULL)
	{
        RTPLOG_ERROR_LEAVE(RemoveRemoteAddress, "NULL RTP session pointer"); 
		return RV_ERROR_UNKNOWN;
	}
    RvLockGet(&s->lock, logMgr);
    
    while((pRvAddress = RvRtpAddressListGetNext(&s->addressList, NULL))!= NULL)
    {   
        if (s->profilePlugin != NULL && s->profilePlugin->funcs != NULL &&
            s->profilePlugin->funcs->removeRemAddress != NULL)
            s->profilePlugin->funcs->removeRemAddress(s->profilePlugin, hRTP, (RvNetAddress *) pRvAddress);
        RvRtpAddressListRemoveAddress(&s->addressList, pRvAddress);
    }
#if OLD_4_REF
	if (s->hRTCP!=NULL) /* automatic RTCP session opening needed */
    {
        RTPLOG_INFO((RTP_SOURCE,"Trying to remove all remote addresses for the RTCP session %#x", (RvUint32)s->hRTCP));    
		RvRtcpRemoveAllRemoteAddresses(s->hRTCP);
    }
#endif
   //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
   memset(&s->remoteAddress,0,sizeof(s->remoteAddress));
   s->socklen=0;
#endif
       //SPIRENT_END
	s->remoteAddressSet = RV_FALSE;
    RvLockRelease(&s->lock, logMgr);   
    RTPLOG_INFO((RTP_SOURCE,"Removed all remote addresses for the RTP session %#x", (RvUint32)hRTP));
    RTPLOG_LEAVE(RemoveAllRemoteAddresses);
	return RV_OK;
}
#ifdef RVRTP_OLD_CONVENTION_API
/************************************************************************************
 * RvRtpOLDPack
 * description: This routine sets the RTP header.
 * input: hRTP  - Handle of the RTP session.
 *        buf   - Pointer to buffer containing the RTP packet with room before first
 *                payload byte for RTP header.
 *        len   - Length in bytes of buf.
 *        p     - A struct of RTP param.
 * output: none.
 * return value:  If no error occurs, the function returns the non-neagtive value.
 *                Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtpOLDPack(
        IN  RvRtpSession  hRTP,
        IN  void *       buf,
        IN  RvInt32      len,
        IN  RvRtpParam *   p)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
    RvUint32 *header;
    RvUint32 seq;

    RTPLOG_ENTER(OLDPack);
    p->sByte-=12;
    p->len=len - p->sByte;

    if (s->useSequenceNumber)
        s->sequenceNumber=p->sequenceNumber;
    p->sequenceNumber=s->sequenceNumber;
    seq = s->sequenceNumber;

    /* sets the fields inside RTP message.*/
    header=(RvUint32*)((char*)buf + p->sByte);
    header[0]=0;
    header[0]=bitfieldSet(header[0],2,30,2);
    header[0]=bitfieldSet(header[0],p->paddingBit,29,1);	/* padding bit if exist */
    header[0]=bitfieldSet(header[0],p->marker,23,1);
    header[0]=bitfieldSet(header[0],p->payload,16,7);
    header[0]=bitfieldSet(header[0],seq,0,16);
    header[1]=p->timestamp;
    header[2]=s->sSrc;

    /* increment the internal sequence number for this session */

    s->sequenceNumber++;
    if(s->sequenceNumber == RvUint16Const(0)) /* added for SRTP support */
        s->roc++;
    /* converts an array of 4-byte integers from host format to network format.*/
    ConvertToNetwork(header, 0, 3);
    RTPLOG_LEAVE(OLDPack);
    return RV_OK;
}
#else
/************************************************************************************
 * RvRtpPack
 * description: This routine sets the RTP header.
 * input: hRTP  - Handle of the RTP session.
 *        buf   - Pointer to buffer containing the RTP packet with room before first
 *                payload byte for RTP header.
 *        len   - Length in bytes of buf.
 *        p     - A struct of RTP param.
 * output: none.
 * return value:  If no error occurs, the function returns the non-neagtive value.
 *                Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtpPack(
        IN  RvRtpSession  hRTP,
        IN  void *       buf,
        IN  RvInt32      len,
        IN  RvRtpParam *   p)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
    RvUint32 *header;
    RvUint32 seq;

    RTPLOG_ENTER(Pack);
    if (p->extensionBit && p->extensionLength>0 && p->extensionData==NULL)
    {
        
        RTPLOG_ERROR_LEAVE(Pack, "NULL RTP header extension data");
        return RV_ERROR_NULLPTR;
    }
    p->sByte-=(12+(p->extensionBit)*(4+(p->extensionLength<<2)));
    p->len=len - p->sByte;

    if (s->useSequenceNumber)
        s->sequenceNumber=p->sequenceNumber;
    p->sequenceNumber=s->sequenceNumber;
    seq = s->sequenceNumber;

    /* sets the fields inside RTP message.*/
    header=(RvUint32*)((char*)buf + p->sByte);
    header[0]=0;
    header[0]=bitfieldSet(header[0],2,30,2);                /* protocol version 2 */
    header[0]=bitfieldSet(header[0],p->paddingBit,29,1);	/* padding bit if exist */
    header[0]=bitfieldSet(header[0],p->extensionBit,28,1);	/* extension bit if exist */
#ifdef UPDATED_BY_SPIRENT
    header[0]=bitfieldSet(header[0],p->cSrcc,24,4);	/* CSRC count.*/
#endif
    header[0]=bitfieldSet(header[0],p->marker,23,1);
    header[0]=bitfieldSet(header[0],p->payload,16,7);
    header[0]=bitfieldSet(header[0],seq,0,16);
    header[1]=p->timestamp;
    header[2]=s->sSrc;

    /* increment the internal sequence number for this session */

    s->sequenceNumber++;
    if(s->sequenceNumber == RvUint16Const(0)) /* added for SRTP support */
        s->roc++;

    if (p->extensionBit)
    {
#ifdef UPDATED_BY_SPIRENT
        int index = 3+p->cSrcc;

        header[index] = 0;
        header[index] = bitfieldSet(header[index], p->extensionLength,  0,16);
        header[index] = bitfieldSet(header[index], p->extensionProfile,16,16);
        index++;
        if (p->extensionLength>0)
        {
            RvUint32 count = 0;
            for (count=0;count<p->extensionLength;count++)
            {
                header[index+count] = p->extensionData[count];
            }
        }
        ConvertToNetwork(header, 0, index + p->extensionLength);
#else
        header[3] = 0;
        header[3] = bitfieldSet(header[3], p->extensionLength,  0,16);
        header[3] = bitfieldSet(header[3], p->extensionProfile,16,16);
        if (p->extensionLength>0)
        {
            RvUint32 count = 0;
            for (count=0;count<p->extensionLength;count++)
            {
                header[4+count] = p->extensionData[count];
            }
        }
        ConvertToNetwork(header, 0, 4 + p->extensionLength);
#endif
    }
    else
    {        
#ifdef UPDATED_BY_SPIRENT
        ConvertToNetwork(header, 0, 3+p->cSrcc);
#else
        /* converts an array of 4-byte integers from host format to network format.*/
        ConvertToNetwork(header, 0, 3);
#endif
    }
    RTPLOG_LEAVE(Pack);
    return RV_OK;
}
#endif

/************************************************************************************
 * RvRtpSetRtpSequenceNumber 
 * description: This routine gets the RTP sequence number for packet that will be sent
 *              This is not documented function, used only for IOT.
 * input: hRTP  - Handle of the RTP session.
 *        sn    - sequence number to set
 * return value:  If no error occurs, the function returns 0.
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvRtpSetRtpSequenceNumber(IN  RvRtpSession  hRTP, RvUint16 sn)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;

    RTPLOG_ENTER(SetRtpSequenceNumber); 
    if (NULL==s)
    {
        RTPLOG_ERROR_LEAVE(SetRtpSequenceNumber, "NULL RTP session handle");   
        return RV_ERROR_NULLPTR;
    }
    s->sequenceNumber = sn;
    RTPLOG_LEAVE(SetRtpSequenceNumber);
	return RV_OK;
}
/************************************************************************************
 * RvRtpGetRtpSequenceNumber
 * description: This routine gets the RTP sequence number for packet that will be sent
 * input: hRTP  - Handle of the RTP session.
 * return value:  If no error occurs, the function returns RTP sequence number.
 *                Otherwise, it returns 0.
 ***********************************************************************************/
RVAPI
RvUint32 RVCALLCONV RvRtpGetRtpSequenceNumber(IN  RvRtpSession  hRTP)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;

    RTPLOG_ENTER(GetRtpSequenceNumber); 
    if (NULL==s)
    {
        RTPLOG_ERROR_LEAVE(GetRtpSequenceNumber, "NULL RTP session handle");   
        return 0;
    }
    RTPLOG_LEAVE(GetRtpSequenceNumber);
	return s->sequenceNumber;
}

/************************************************************************************
 * RvRtpWrite
 * description: This routine sets the RTP header.
 * input: hRTP  - Handle of the RTP session.
 *        buf   - Pointer to buffer containing the RTP packet with room before first
 *                payload byte for RTP header.
 *        len   - Length in bytes of buf (RTP header + RTP data).
 *        p     - A struct of RTP param.
 * output: none.
 * return value:  If no error occurs, the function returns the non-negative value.
 *                Otherwise, it returns a negative value.
 * IMPORTANT NOTE: in case of encryption according to RFC 3550 (9. Security) or in case
 *   of SRTP p.len must contain the actual length of buffer, which must be
 *   more then len, because of encryption padding or SRTP authentication tag (recommended)
 *   or/and SRTP MKI string (optional).
 ***********************************************************************************/
#define RTP_WORK_BUFFER_SIZE (1600)
RVAPI
RvInt32 RVCALLCONV RvRtpWrite(
        IN  RvRtpSession  hRTP,
        IN  void *       buf,
        IN  RvInt32      len,
        IN  RvRtpParam *   p)
{
    RvStatus res = RV_ERROR_UNKNOWN;
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
    
    RTPLOG_ENTER(Write); 
    
    if (NULL == s || NULL == s->profilePlugin)
    {
        RTPLOG_ERROR_LEAVE(Write, "NULL session handle or session is not opened");        
        return RV_ERROR_NULLPTR;
    }
    if (rtcpSessionIsSessionInShutdown(s->hRTCP))
    {
        RTPLOG_ERROR_LEAVE(Write, "Can not send. The session is in shutdown or RTCP BYE report was sent");
        return RV_ERROR_NOTSUPPORTED;
    }
    RvLockGet(&s->lock, logMgr);
    if (s->profilePlugin != NULL &&
        s->profilePlugin->funcs != NULL && 
        s->profilePlugin->funcs->writeXRTP != NULL)
    {
        res = s->profilePlugin->funcs->writeXRTP(s->profilePlugin, hRTP, buf, len, p);
    }
    if ((s->hRTCP != NULL) &&  (res == RV_OK))
    {	
        RTPLOG_DEBUG((RTP_SOURCE, "RvRtpWrite: informing RTCP stack"));
        RvRtcpSessionSetParam(s->hRTCP, RVRTCP_PARAMS_RTP_PAYLOADTYPE, &p->payload);
        /* inform the RTCP session about a packet that was sent in the corresponding RTP session.*/
        res = RvRtcpRTPPacketSent(s->hRTCP, p->len, p->timestamp);
        
    } 
    RvLockRelease(&s->lock, logMgr);
    RTPLOG_LEAVE(Write); 
    return res;
}

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
RVAPI
RvInt32 RVCALLCONV RvRtpWriteLight(
        IN  RvRtpSession  hRTP,
        IN  void *       buf,
        IN  RvInt32      len,
        IN  RvRtpParam *   p)
{
    RvStatus res = RV_ERROR_UNKNOWN;
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
    rtcpSession *srtcp=((rtcpSession*)(s->hRTCP));
    
    RTPLOG_ENTER(Write); 
    
    if (NULL == s || NULL == s->profilePlugin)
    {
        RTPLOG_ERROR_LEAVE(Write, "NULL session handle or session is not opened");        
        return RV_ERROR_NULLPTR;
    }
    if (srtcp && srtcp->isShutdown)
    {
        RTPLOG_ERROR_LEAVE(Write, "Can not send. The session is in shutdown or RTCP BYE report was sent");
        return RV_ERROR_NOTSUPPORTED;
    }

    if (s->profilePlugin != NULL &&
        s->profilePlugin->funcs != NULL && 
        s->profilePlugin->funcs->writeXRTP != NULL)
    {
        res = s->profilePlugin->funcs->writeXRTP(s->profilePlugin, hRTP, buf, len, p);
    }
    if ((srtcp != NULL) &&  (res == RV_OK))
    {	
        RTPLOG_DEBUG((RTP_SOURCE, "RvRtpWrite: informing RTCP stack"));

        srtcp->myInfo.rtpPayloadType = p->payload;

        /* inform the RTCP session about a packet that was sent in the corresponding RTP session.*/
        
        srtcp->myInfo.active = 2;
        srtcp->myInfo.eSR.nPackets++;
        srtcp->myInfo.eSR.nBytes += p->len;
        srtcp->myInfo.lastPackNTP   = getNNTPTime();
        srtcp->myInfo.lastPackRTPts = p->timestamp;
        
        if (srtcp->myInfo.collision)
        {
           res=ERR_RTCP_SSRCCOLLISION;
        }   

    } 
    RTPLOG_LEAVE(Write); 
    return res;
}

RVAPI
RvInt32 RVCALLCONV RvRtpWriteLightNoRtcp(
        IN  RvRtpSession  hRTP,
        IN  void *       buf,
        IN  RvInt32      len,
        IN  RvRtpParam *   p)
{
    RvStatus res = RV_ERROR_UNKNOWN;
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
    
    RTPLOG_ENTER(Write); 
    
    if (NULL == s || NULL == s->profilePlugin)
    {
        RTPLOG_ERROR_LEAVE(Write, "NULL session handle or session is not opened");        
        return RV_ERROR_NULLPTR;
    }

    if (s->profilePlugin != NULL &&
        s->profilePlugin->funcs != NULL && 
        s->profilePlugin->funcs->writeXRTP != NULL)
    {
        res = s->profilePlugin->funcs->writeXRTP(s->profilePlugin, hRTP, buf, len, p);
    } 
    RTPLOG_LEAVE(Write); 
    return res;
}

RVAPI
RvInt32 RVCALLCONV RvRtpWriteLightToRtcp(
        IN  RvRtpSession  hRTP,
        IN  void *       buf,
        IN  RvInt32      len,
	IN  RvUint8      payload,
        IN  RvUint32     ts)
{
    RvStatus res = RV_OK;
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;

    RTPLOG_ENTER(Write); 

    if(s) {
      rtcpSession *srtcp=((rtcpSession*)(s->hRTCP));
    
      if (srtcp && srtcp->isShutdown) {
	RTPLOG_ERROR_LEAVE(Write, "Can not send. The session is in shutdown or RTCP BYE report was sent");
	return RV_ERROR_NOTSUPPORTED;
      }

      if (srtcp && len && buf) {	
        RTPLOG_DEBUG((RTP_SOURCE, "RvRtpWrite: informing RTCP stack"));
	
        srtcp->myInfo.rtpPayloadType = payload;
	
        /* inform the RTCP session about a packet that was sent in the corresponding RTP session.*/
        
        srtcp->myInfo.active = 2;
        srtcp->myInfo.eSR.nPackets++;
        srtcp->myInfo.eSR.nBytes += len;
        srtcp->myInfo.lastPackNTP   = getNNTPTime();
        srtcp->myInfo.lastPackRTPts = ts;
        
        if (srtcp->myInfo.collision) {
	  res=ERR_RTCP_SSRCCOLLISION;
	}   
      } 
    }
    RTPLOG_LEAVE(Write); 
    return res;
}

#endif
//SPIRENT_END

/************************************************************************************
 * RvRtpUnpack
 * description: Gets the RTP header from a buffer. 
 * input: hRTP  - Handle of the RTP session.
 *        buf   - Pointer to buffer containing the RTP packet
 *        len   - The length in bytes of the RTP packet.
 *        p     - A structure of rtpParam with the RTP header information.
 * output: none.
 * return value:  If an error occurs, the function returns a negative value.
 *                If no error occurs, the function returns a non-negative value. 
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtpUnpack(
        IN  RvRtpSession  hRTP,
        IN  void *buf,
        IN  RvInt32 len,
        OUT RvRtpParam* p)
{
    RvUint32 *header=(RvUint32*)buf;

    RTPLOG_ENTER(Unpack);

    RV_UNUSED_ARG(len);
    RV_UNUSED_ARG(hRTP);

    if (p->len < 12)
    {
        RTPLOG_ERROR_LEAVE(Unpack, "This packet is probably corrupted.");
        return RV_ERROR_UNKNOWN;
    }
    ConvertFromNetwork(buf, 3, (RvInt32)bitfieldGet(header[0], 24, 4));
	if (bitfieldGet(header[0],30,2)!=2)
    {
        RTPLOG_ERROR_LEAVE(Unpack, "Incorrect RTP version number.");
        return RV_ERROR_UNKNOWN;
    }
    p->timestamp=header[1];
    p->sequenceNumber=(RvUint16)bitfieldGet(header[0],0,16);
    p->sSrc=header[2];
    p->marker=bitfieldGet(header[0],23,1);
    p->payload=(unsigned char)bitfieldGet(header[0],16,7);
    p->extensionBit = bitfieldGet(header[0],28,1);
    p->sByte=12+bitfieldGet(header[0],24,4)*sizeof(RvUint32);

    if (p->extensionBit)/*Extension Bit Set*/
    {
        RvInt32 xStart=p->sByte / sizeof(RvUint32);

        ConvertFromNetwork(buf, xStart, 1);
        p->extensionProfile = (RvUint16) bitfieldGet(header[xStart], 16, 16);
        p->extensionLength  = (RvUint16) bitfieldGet(header[xStart],  0, 16);
        p->sByte += ((p->extensionLength+1)*sizeof(RvUint32));
        
        if (p->sByte > p->len)
        {
            /* This packet is probably corrupted */
            p->sByte = 12;
            RTPLOG_ERROR_LEAVE(Unpack, "This packet is probably corrupted.");
            return RV_ERROR_UNKNOWN;
        }
        if (p->extensionLength>0)
        {
            p->extensionData    =  &header[xStart+1];
            ConvertFromNetwork(buf, xStart+1, p->extensionLength);
        }
        else
        {
            p->extensionData    = NULL;
        }
    }

    if (bitfieldGet(header[0],29,1))/*Padding Bit Set*/
    {
        p->len-=((char*)buf)[p->len-1];
		if (p->len < p->sByte)
        {
            RTPLOG_ERROR_LEAVE(Unpack, "This packet is probably corrupted.");	
            return RV_ERROR_UNKNOWN;
        }
    }
    RTPLOG_LEAVE(Unpack);  
    return RV_OK;
}

/************************************************************************************
 * RvRtpReadWithRemoteAddress
 * description: This routine reads the RTP message and sets the header of the RTP message.
 *              It also retrieves the address of the RTP message sender.
 * input: hRTP  - Handle of the RTP session.
 *        buf   - Pointer to buffer containing the RTP packet with room before first
 *                payload byte for RTP header.
 *        len   - Length in bytes of buf.
 *
 * output: p            - A struct of RTP param,contain the fields of RTP header.
 *        remAddressPtr - pointer to the remote address
 * return value: If no error occurs, the function returns the non-negative value.
 *                Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtpReadWithRemoteAddress(
		IN  RvRtpSession  hRTP,
		IN  void *buf,
		IN  RvInt32 len,
		OUT RvRtpParam* p,
		OUT RvNetAddress* remAddressPtr)
{
    RvRtpSessionInfo* s = (RvRtpSessionInfo *)hRTP;
    RvStatus res = RV_ERROR_UNKNOWN;
    
    RTPLOG_ENTER(ReadWithRemoteAddress);
    if (s==NULL || s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(ReadWithRemoteAddress, "RTP session is not opened");
        return res; 
    }
    if (s->profilePlugin != NULL &&
        s->profilePlugin->funcs != NULL &&
        s->profilePlugin->funcs->readXRTP != NULL)
    {
        res = s->profilePlugin->funcs->readXRTP(s->profilePlugin, hRTP, buf, len, p, remAddressPtr);  
    }
    RTPLOG_LEAVE(ReadWithRemoteAddress);
    return res;
}
/************************************************************************************
 * RvRtpRead
 * description: This routine sets the header of the RTP message.
 * input: hRTP  - Handle of the RTP session.
 *        buf   - Pointer to buffer containing the RTP packet with room before first
 *                payload byte for RTP header.
 *        len   - Length in bytes of buf.
 *
 * output: p    - A struct of RTP param,contain the fields of RTP header.
 * return value: If no error occurs, the function returns the non-negative value.
 *                Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtpRead(
							 IN  RvRtpSession  hRTP,
							 IN  void *buf,
							 IN  RvInt32 len,
							 OUT RvRtpParam* p)
{
    RvNetAddress  remAddress;
    RvInt32 res = 0;
    RTPLOG_ENTER(Read);   
    res = RvRtpReadWithRemoteAddress(hRTP, buf, len, p, &remAddress);
    RTPLOG_LEAVE(Read);
    return res;
}

/************************************************************************************
 * RvRtpReadEx
 * description: Receives an RTP packet and updates the corresponding RTCP session.
 * input: hRTP      - Handle of the RTP session.
 *        buf       - Pointer to buffer containing the RTP packet with room before first
 *                    payload byte for RTP header.
 *        len       - Length in bytes of buf.
 *        timestamp - The local RTP timestamp when the received packet arrived. 
 *        p         - A struct of RTP param,contain the fields of RTP header.
 * output: none.
 * return value: If no error occurs, the function returns the non-neagtive value.
 *               Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtpReadEx(
        IN  RvRtpSession  hRTP,
        IN  void *       buf,
        IN  RvInt32      len,
        IN  RvUint32     timestamp,
        OUT RvRtpParam *   p)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
    RvInt32 retVal;

    RTPLOG_ENTER(ReadEx);       
    retVal = RvRtpRead(hRTP, buf, len, p);

    if (s->hRTCP  &&  retVal >= 0)
    {
    /* Informs the RTCP session about a packet that was received in the corresponding
        RTP session.*/
        RTPLOG_DEBUG((RTP_SOURCE, "RvRtpReadEx: informing RTCP session about received RTP packet"));	
        RvRtcpRTPPacketRecv(s->hRTCP, p->sSrc, timestamp,  p->timestamp, p->sequenceNumber);
    }
    RTPLOG_LEAVE(ReadEx);      
    return retVal;
}


/************************************************************************************
 * RvRtpGetPort
 * description: Returns the current port of the RTP session.
 * input: hRTP - Handle of the RTP session.
 * output: none.
 * return value: If no error occurs, the function returns the current port value.
 *               Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
RvUint16 RVCALLCONV RvRtpGetPort(
        IN RvRtpSession  hRTP)   /* RTP Session Opaque Handle */
{
    RvRtpSessionInfo* s = (RvRtpSessionInfo *)hRTP;
    RvAddress localAddress;
    RvUint16 sockPort = 0;
    RvStatus res;

    RTPLOG_ENTER(GetPort);   
    res = RvSocketGetLocalAddress(&s->socket, logMgr, &localAddress);

    if (res == RV_OK)
    {
        sockPort = RvAddressGetIpPort(&localAddress);

        RvAddressDestruct(&localAddress);
    }
    RTPLOG_LEAVE(GetPort);   
    return sockPort;
}

/************************************************************************************
 * RvRtpGetVersion
 * description:  Returns the RTP version of the installed RTP Stack.
 * input:  none.
 * output: none.
 * return value: If no error occurs, the function returns the current version value.
 *               Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
char * RVCALLCONV RvRtpGetVersion(void)
{
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpGetVersion"));
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpGetVersion"));
    return (char*)VERSION_STR;
}

/************************************************************************************
 * RvRtpGetVersionNum
 * description:  Returns the RTP version of the installed RTP Stack.
 * input:  none.
 * output: none.
 * return value: If no error occurs, the function returns the current version value.
 *               Otherwise, it returns a negative value.
 ***********************************************************************************/
RVAPI
RvUint32 RVCALLCONV RvRtpGetVersionNum(void)
{
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpGetVersionNum"));
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpGetVersionNum"));
    return VERSION_NUM;
}


                    /* == ENDS: Basic RTP Functions == */



                     /* == Accessory RTP Functions == */

/************************************************************************************
 * RvRtpGetRTCPSession
 * description:  Returns the RTCP session.
 * input:  hRTP - Handle of RTP session.
 * output: none.
 * return value: hRTCP - Handle of RTCP session.
 ***********************************************************************************/
RVAPI
RvRtcpSession RVCALLCONV RvRtpGetRTCPSession(
        IN  RvRtpSession  hRTP)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpGetRTCPSession"));
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpGetRTCPSession"));
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
   return (s ? s->hRTCP : NULL);
#else
    return (s->hRTCP);
#endif
//SPIRENT_END
}

/************************************************************************************
 * RvRtpSetRTCPSession
 * description:  set the RTCP session.
 * input:  hRTP  - Handle of RTP session.
 *         hRTCP - Handle of RTCP session.
 * output: none.
 * return value:return 0.
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtpSetRTCPSession(
        IN  RvRtpSession   hRTP,
        IN  RvRtcpSession  hRTCP)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpSetRTCPSession"));
    s->hRTCP = hRTCP;
    rtcpSetRtpSession(hRTCP, hRTP);
    RvRtcpSetProfilePlugin(s->hRTCP, s->profilePlugin);
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpSetRTCPSession"));
    return RV_OK;
}

/************************************************************************************
 * RvRtpGetHeaderLength
 * description:  return the header of RTP message.
 * input:  none.
 * output: none.
 * return value:The return value is twelve.
 * NOTE:  in the FUTURE: RvRtpGetHeaderLength p must be adjusted to number of csrcs 
 *        and  will be with the following semantics: RvRtpGetHeaderLength(IN RvRtpParam*p)
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtpGetHeaderLength(void)
{
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpGetHeaderLength"));
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpGetHeaderLength"));
    return 12;
}

/************************************************************************************
 * RvRtpRegenSSRC
 * description:  Generates a new synchronization source value for the RTP session.
 *               This function, in conjunction with RvRtpGetSSRC() may be used to
 *               change the SSRC value when an SSRC collision is detected.
 * input:  hRTP  - Handle of RTP session.
 * output: none.
 * return value: ssrc 
 ***********************************************************************************/
RVAPI
RvUint32 RVCALLCONV RvRtpRegenSSRC(
        IN  RvRtpSession  hRTP)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
    RvRandom randomValue;
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpRegenSSRC"));
    /* those line is to prevent collision.*/
    RvRandomGeneratorGetValue(&rvRtpInstance.randomGenerator, &randomValue);
    s->sSrc = (RvUint32)randomValue * RvInt64ToRvUint32(RvInt64ShiftRight(RvTimestampGet(logMgr), 16));
    s->sSrc &= ~s->sSrcMask;
    s->sSrc |= s->sSrcPattern;
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpRegenSSRC"));
    return s->sSrc;
}

/************************************************************************************
 * rtpSetGroupAddress
 * description:  Defines a multicast IP for the RTP session.
 * input:  hRTP  - Handle of RTP session.
 *         ip    - Multicast IP address for the RTP session.
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/
#ifdef RVRTP_OLD_CONVENTION_API
RVAPI
RvInt32 RVCALLCONV rtpSetGroupAddress(
        IN RvRtpSession hRTP,    /* RTP Session Opaque Handle */
        IN RvUint32     ip)
{
	RvNetAddress rtpAddress;
    RvNetIpv4   Ipv4;
    Ipv4.ip = ip;
    Ipv4.port = 0;
    RvNetCreateIpv4(&rtpAddress, &Ipv4);
	return	RvRtpSetGroupAddress(hRTP, &rtpAddress);
}
#endif
RVAPI
RvInt32 RVCALLCONV RvRtpSetGroupAddress(
									  IN RvRtpSession hRTP,    /* RTP Session Opaque Handle */
									  IN RvNetAddress* pRtpAddress)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
    RvAddress addresses[2], sockAddress;
    RvStatus res, res1, res2;
    RvNetIpv4 Ipv4;
	RvInt32 addressType = RV_ADDRESS_TYPE_NONE;
	RTPLOG_ENTER(SetGroupAddress);
	
    if (NULL==pRtpAddress||
		RV_ADDRESS_TYPE_NONE == (addressType = RvAddressGetType(&rvRtpInstance.rvLocalAddress)))
	{
		RTPLOG_ERROR_LEAVE(SetGroupAddress, "pRtpAddress==NULL or local address is undefined");
        return RV_ERROR_BADPARAM;
	}
	if (RvNetGetAddressType(pRtpAddress) == RVNET_ADDRESS_IPV6)
	{
#if (RV_NET_TYPE & RV_NET_IPV6)
      //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
      RvAddressConstructIpv6(&addresses[0], 
			RvAddressIpv6GetIp(RvAddressGetIpv6((RvAddress*)pRtpAddress->address)),
			RV_ADDRESS_IPV6_ANYPORT/*RvAddressGetIpPort((RvAddress*)pRtpAddress->address*/,
			RvAddressGetIpv6Scope((RvAddress*)pRtpAddress->address),
         RvAddressIpv6GetFlowinfo(RvAddressGetIpv6((RvAddress*)pRtpAddress->address)));
#else
		RvAddressConstructIpv6(&addresses[0], 
			RvAddressIpv6GetIp(RvAddressGetIpv6((RvAddress*)pRtpAddress->address)),
			RV_ADDRESS_IPV6_ANYPORT/*RvAddressGetIpPort((RvAddress*)pRtpAddress->address*/,
			RvAddressGetIpv6Scope((RvAddress*)pRtpAddress->address));
#endif
      //SPIRENT_END
#else
		RTPLOG_ERROR_LEAVE(SetGroupAddress, "IPV6 is not supported in current configuration");
		return RV_ERROR_BADPARAM;
#endif
	}
	else
	{
		RvNetGetIpv4(&Ipv4, pRtpAddress);
		RvAddressConstructIpv4(&addresses[0], Ipv4.ip, RV_ADDRESS_IPV4_ANYPORT);           
	}
    if (!RvAddressIsMulticastIp(&addresses[0]))
    {
		RTPLOG_ERROR_LEAVE(SetGroupAddress, "Non-multicast IP");
		return RV_ERROR_BADPARAM;
    }
    RvAddressCopy(&rvRtpInstance.rvLocalAddress, &addresses[1]);
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
        RTPLOG_INFO((RTP_SOURCE, "Successed to set multicast group address to %s for session (%#x)", 
            RvAddressGetString(&addresses[0], sizeof(addressStr), addressStr), s));
    }
    else
    {
		RTPLOG_ERROR((RTP_SOURCE, "RvRtpSetGroupAddress - Failed to set multicast group address for session (%#x)", s));
    }
    RvAddressDestruct(&addresses[0]);
    RvAddressDestruct(&addresses[1]);
	RV_UNUSED_ARG(addressType);
	RTPLOG_LEAVE(SetGroupAddress);
    return res;
}
/************************************************************************************
 * RvRtpSetMulticastTTL
 * description:  Defines a multicast Time To Live (TTL) for the RTP session.
 * input:  hRTP  - Handle of RTP session.
 *         ttl   - [0..255] ttl threshold for multicast
 * output: none.
 * return value:  If no error occurs, the function returns the non-negative value.
 *                Otherwise, it returns a negative value.
 * Note: the function is supported with IP stack that has full multicast support 
 ***********************************************************************************/

RVAPI
RvInt32 RVCALLCONV RvRtpSetMulticastTTL(
		IN  RvRtpSession  hRTP,
		IN  RvUint8       ttl)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
	RvStatus res = RV_OK;

	RTPLOG_ENTER(SetMulticastTTL);
    if (NULL==s)
	{
		RTPLOG_ERROR_LEAVE(SetMulticastTTL, "NULL session handle");
		return RV_ERROR_BADPARAM;
	}
    res = RvSocketSetMulticastTtl(&s->socket, (RvInt32) ttl, logMgr);
    if (res==RV_OK)
    {
        RTPLOG_INFO((RTP_SOURCE, "Multicast TTL set to %d for session (%#x)", (RvInt32)ttl, s));
    }
    else
    {   
        RTPLOG_ERROR((RTP_SOURCE, "RvRtpSetMulticastTTL - Failed to set multicast TTL for session (%#x)", s));
    }
	RTPLOG_LEAVE(SetMulticastTTL);
	return res;
}
/************************************************************************************
 * RvRtpResume
 * description:  Causes a blocked RvRtpRead() or RvRtpReadEx() function running in
 *               another thread to fail.
 * input:  hRTP  - Handle of RTP session.
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtpResume(
        IN  RvRtpSession hRTP)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
    RvAddress localAddress;

    RvStatus status;
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpResume"));

    status = RvSocketGetLocalAddress(&s->socket, logMgr, &localAddress);

    if (status == RV_OK)
    {
#ifdef OLD_4_REF
		RvAddressIpv4* localIpV4;
        if (RvAddressGetType(&localAddress) == RV_ADDRESS_TYPE_IPV4)
		{
			localIpV4 = (RvAddressIpv4 *)RvAddressGetIpv4(&localAddress);
			if (RvAddressIpv4GetIp(localIpV4) == RV_ADDRESS_IPV4_ANYADDRESS)
			{
				/* No specific IP on this socket - use our default one */
				RvAddressIpv4* pInstanceIpv4 = (RvAddressIpv4*) RvAddressGetIpv4(&rvRtpInstance.rvLocalAddress);
				if (pInstanceIpv4->ip != RV_ADDRESS_IPV4_ANYADDRESS)
					RvAddressIpv4SetIp(localIpV4, pInstanceIpv4->ip);
				else
					RvAddressIpv4SetIp(localIpV4, RvAddressIpv4GetIp(RvAddressGetIpv4(&rvRtpInstance.hostIPs[0])));
			}
		}
#endif
		if (!isMyIP(&localAddress))
		{
			RvUint16 Port = RvAddressGetIpPort(&localAddress);
			RvInt32  type = RvAddressGetType(&localAddress);
			RvUint32  Count = 0;
			for (Count=0;Count<rvRtpInstance.addressesNum;Count++)
				if (type == RvAddressGetType(&rvRtpInstance.hostIPs[Count]))
				{
					/*			RvAddressCopy(&rvRtpInstance.hostIPs[0], &localAddress);*/
					RvAddressCopy(&rvRtpInstance.hostIPs[Count], &localAddress);
					RvAddressSetIpPort(&localAddress, Port);
					break;
				}
		}
        /* We send a dummy buffer to get this connection released from its blocking mode */
        status = RvSocketSendBuffer(&s->socket, (RvUint8*)"", 1, &localAddress, logMgr, NULL);

        RvAddressDestruct(&localAddress);
    }
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpResume"));
    return status;
}

/************************************************************************************
 * RvRtpUseSequenceNumber
 * description:  Forces the Stack to accept user input for the sequence number of
 *               the RTP packet. The RTP Stack usually determines the sequence number.
 *               However, the application can force its own sequence number.
 *               Call RvRtpUseSequenceNumber() at the beginning of the RTP session and
 *               then specify the sequence number in the RvRtpParam structure of the
 *               RvRtpWrite() function.
 * input:  hRTP  - Handle of RTP session.
 * output: none.
 * return value: return 0.
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtpUseSequenceNumber(
                IN RvRtpSession  hRTP)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpUseSequenceNumber"));
    s->useSequenceNumber = 1;
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpUseSequenceNumber"));
    return RV_OK;
}

/************************************************************************************
 * RvRtpSetReceiveBufferSize
 * description:  Changes the RTP session receive buffer size.
 * input:  hRTP  - Handle of RTP session.
 * output: none.
 * return value: return 0.
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtpSetReceiveBufferSize(
        IN RvRtpSession  hRTP,
        IN RvInt32 size)
{
    RvRtpSessionInfo* s = (RvRtpSessionInfo *)hRTP;
	RvLogEnter(rvLogPtr, (rvLogPtr, "RtpSetReceiveBufferSize"));
	RvLogLeave(rvLogPtr, (rvLogPtr, "RtpSetReceiveBufferSize"));
    /* This function sets the size of the read/write buffers of a socket*/
    return RvSocketSetBuffers(&s->socket, -1, size, logMgr);
}

/************************************************************************************
 * RvRtpSetTransmitBufferSize
 * description:  Changes the RTP session transmit buffer size.
 * input:  hRTP - Handle of RTP session.
 *         size - The new transmit buffer size.
 * output: none.
 * return value: return 0.
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtpSetTransmitBufferSize(
                IN RvRtpSession  hRTP,
                IN RvInt32 size)
{
    RvRtpSessionInfo* s = (RvRtpSessionInfo *)hRTP;
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpSetTransmitBufferSize"));
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpSetTransmitBufferSize"));
    /* This function sets the size of the read/write buffers of a socket*/
    return RvSocketSetBuffers(&s->socket, size, -1, logMgr);
}

/************************************************************************************
 * RvRtpSetTrasmitBufferSize
 * description:  Changes the RTP session transmit buffer size.
 * input:  hRTP - Handle of RTP session.
 *         size - The new transmit buffer size.
 * output: none.
 * return value: return 0.
 * comment     : obsolete function provided for compatibility with prev. version
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtpSetTrasmitBufferSize(
                IN RvRtpSession  hRTP,
                IN RvInt32 size)
{
  return RvRtpSetTransmitBufferSize(hRTP, size);
}


/************************************************************************************
 * RvRtpGetAvailableBytes
 * description:  Gets the number of bytes available for reading in the RTP session.
 * input:  hRTP - Handle of RTP session.
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a non-negative value.
 ***********************************************************************************/
RVAPI
RvInt32 RVCALLCONV RvRtpGetAvailableBytes(
                IN RvRtpSession  hRTP)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
    RvSize_t bytes;
    RvStatus ret;
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpGetAvailableBytes"));

    /* This function returns the number of bytes in the specified socket that are
       available for reading.*/

    ret = RvSocketGetBytesAvailable(&s->socket, logMgr, &bytes);

    if (ret != RV_OK)
	{
		RvLogError(rvLogPtr, (rvLogPtr, "RvRtpGetAvailableBytes"));
	    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpGetAvailableBytes"));
        return ret;
	}
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpGetAvailableBytes"));
    return (RvInt32)bytes;
}

RVAPI
RvInt32 RVCALLCONV RvRtpGetSocket(IN RvRtpSession hRTP)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
		
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpGetSocket"));
	if ((NULL != s) && s->isAllocated)
	{
		RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpGetSocket"));
		return s->socket;
	}
		
	RvLogError(rvLogPtr, (rvLogPtr, "RvRtpGetSocket: NULL pointer or RTP session is not opened"));
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpGetSocket"));
	return -1;
}

#ifdef RVRTP_SECURITY
/************************************************************************************
 * RvRtpSetEncryption
 * description:  Sets encryption parameters for RTP session
 * input:  hRTP - Handle of RTP session.
 *         mode - encryption mode for the session
 *         enctyptorPlugInPtr - pointer to user defined encryption plug in
 *         keyPlugInPtr       - pointer to user defined key manager plug in
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a RV_OK value.
 * NOTE: if corresponding automatic RTCP session was opened
 *       the same encryption and decryption plugins and mode are set for it too.
 ***********************************************************************************/ 
RVAPI
RvStatus RVCALLCONV RvRtpSetEncryption(       
	IN RvRtpSession hRTP, 
	IN RvRtpEncryptionMode mode,
	IN RvRtpEncryptionPlugIn* enctyptorPlugInPtr,
	IN RvRtpEncryptionKeyPlugIn* keyPlugInPtr)
{
	RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
	
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpSetEncryption"));
	if (NULL == s || NULL == enctyptorPlugInPtr || NULL == keyPlugInPtr)
	{
		RvLogError(rvLogPtr, (rvLogPtr, "RvRtpSetEncryption: NULL pointer"));
	    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpSetEncryption"));
		return -1;
	}
	s->encryptionMode = mode;
	RvLogDebug(rvLogPtr, (rvLogPtr, "RvRtpSetEncryption: encryption plug-ins setting"));	
	s->encryptionKeyPlugInPtr  = (RvRtpEncryptionKeyPlugIn*) keyPlugInPtr;
	s->encryptionPlugInPtr     = (RvRtpEncryptionPlugIn*)    enctyptorPlugInPtr;
	if (s->hRTCP!=NULL)
		RvRtcpSetEncryption(s->hRTCP, mode, enctyptorPlugInPtr, keyPlugInPtr);
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpSetEncryption"));
	return RV_OK;
}
#endif
/************************************************************************************
 * RvRtpSetDoubleKeyEncryption
 * description:  Sets encryption plug-in and encryption/decryption keys for RTP session.
 *               This function is only an example of usage of RvRtpDoubleKey plugin.
 *               Pay attention that this implementation allocates RvRtpDoubleKey plugin and
 *               releases it, when RvRtpClose is called.
 *               User can implement other key management plugin and call to RvRtpSetEncryption()
 * input:  hRTP - Handle of RTP session.
 *         mode - encryption mode for the session
 *         pEKey - pointer to encryption key
 *         pDKey - pointer to decryption key
 *         pCrypto - pointer to encryption plug in
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a RV_OK value.
 * NOTE: if corresponding automatic RTCP session was opened
 *       the same encryption and decryption keys, encryption plug in and mode are set for it too.
 ***********************************************************************************/ 
#include "rvrtpdoublekey.h"
RVAPI
RvStatus RVCALLCONV RvRtpSetDoubleKeyEncryption(
	IN RvRtpSession hRtp,
	IN RvRtpEncryptionMode mode,
	IN RvKey *pEKey,
	IN RvKey *pDKey,
	IN RvRtpEncryptionPlugIn *pCrypto) 
{
	RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRtp;
	RvRtpEncryptionKeyPlugIn* keypluginPtr = RvRtpDoubleKeyCreate(pEKey, pDKey);

	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpSetDoubleKeyEncryption"));
	if (NULL == s || NULL == pCrypto|| NULL == keypluginPtr)
	{
		RvLogError(rvLogPtr, (rvLogPtr, "RvRtpSetDoubleKeyEncryption: NULL pointer"));
	    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpSetDoubleKeyEncryption"));
		return -1;
	}
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpSetDoubleKeyEncryption"));
	return RvRtpSetEncryption(hRtp, mode, pCrypto, keypluginPtr);
}
/************************************************************************************
 * RvRtpSetEncryptionNone
 * description:  Cancels encryption for the session
 * input:  hRTP - Handle of RTP session.
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a RV_OK value.
 * NOTE: if corresponding automatic RTCP session was opened RvRtpSetEncryptionNone
 *       cancel encryption for it too.
 ***********************************************************************************/    
RVAPI
RvStatus RVCALLCONV RvRtpSetEncryptionNone(IN RvRtpSession hRTP)
{
	RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpSetEncryptionNone"));
	if (NULL == s)	
	{
		RvLogError(rvLogPtr, (rvLogPtr, "RvRtpSetEncryptionNone: NULL pointer"));
	    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpSetEncryptionNone"));
		return -1;	
	}
	s->encryptionPlugInPtr = NULL;
	s->encryptionKeyPlugInPtr = NULL;
	if (s->hRTCP!=NULL)
		RvRtcpSetEncryptionNone(s->hRTCP);
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpSetEncryptionNone"));
	return RV_OK;
}
/************************************************************************************
 * RvRtpSetEncryptionNone
 * description:  Cancels encryption for the session
 * input:  hRTP - Handle of RTP session.
 * output: none.
 * return value: If an error occurs, the function returns a negative value.
 *               If no error occurs, the function returns a RV_OK value.
 * NOTE: if corresponding automatic RTCP session was opened RvRtpSetEncryptionNone
 *       cancel encryption for it too.
 ***********************************************************************************/      
RVAPI
RvStatus RVCALLCONV RvRtpSessionSetEncryptionMode(IN RvRtpSession hRTP, IN RvRtpEncryptionMode mode)
{
	RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;

	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpSessionSetEncryptionMode"));
	if (NULL == s)
	{
		RvLogError(rvLogPtr, (rvLogPtr, "RvRtpSessionSetEncryptionMode: NULL pointer"));
	    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpSessionSetEncryptionMode"));
		return -1;
	}
	s->encryptionMode = mode;
	if (s->hRTCP!=NULL)
		RvRtcpSessionSetEncryptionMode(s->hRTCP, mode);
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpSessionSetEncryptionMode"));
	return RV_OK;
}    

/********************************************************************************************
 * RvRtpSetTypeOfService
 * Set the type of service (DiffServ Code Point) of the socket (IP_TOS)
 * This function is supported by few operating systems.
 * IPV6 does not support type of service.
 * This function is thread-safe.
 * INPUT   : hRTP           - RTP session to set TOS byte
 *           typeOfService  - type of service to set
 * OUTPUT  : None
 * RETURN  : RV_OK on success, other on failure
 ********************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvRtpSetTypeOfService(
        IN RvRtpSession hRTP,
        IN RvInt32        typeOfService)
{
	RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
	RvStatus result = RV_OK;
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpSetTypeOfService"));
	if (NULL == s || !s->isAllocated)
	{
		RvLogError(rvLogPtr, (rvLogPtr, "RvRtpSetTypeOfService: NULL pointer or socket is not allocated"));
	    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpSetTypeOfService"));
		return RV_ERROR_NULLPTR;
	}
	result = RvSocketSetTypeOfService(&s->socket, typeOfService, logMgr);
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpSetTypeOfService"));
	return result;
}
/********************************************************************************************
 * RvRtpGetTypeOfService
 * Get the type of service (DiffServ Code Point) of the socket (IP_TOS)
 * This function is supported by few operating systems.
 * IPV6 does not support type of service.
 * This function is thread-safe.
 * INPUT   : hRTP           - RTP session handle
 * OUTPUT  : typeOfServicePtr  - pointer to type of service to set
 * RETURN  : RV_OK on success, other on failure
 *********************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvRtpGetTypeOfService(
        IN RvRtpSession hRTP,
        OUT RvInt32*     typeOfServicePtr)
{
	RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
	RvStatus result = RV_OK;
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpGetTypeOfService"));
	if (NULL == s || !s->isAllocated)
	{
		RvLogError(rvLogPtr, (rvLogPtr, "RvRtpGetTypeOfService: NULL pointer or socket is not allocated"));
		RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpGetTypeOfService"));
		return RV_ERROR_NULLPTR;
	}	
	result = RvSocketGetTypeOfService(&s->socket, logMgr, typeOfServicePtr);
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpGetTypeOfService"));
	return result;
}

/*******************************************************************************************
 * RvRtpSessionShutdown
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
RvStatus RVCALLCONV RvRtpSessionShutdown(
		IN RvRtpSession  hRTP, 
		IN RvChar *reason, 
		IN RvUint8 reasonLength)
{
	RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
	RvStatus result = RV_OK;
	RvLogEnter(rvLogPtr, (rvLogPtr, "RvRtpSessionShutdown"));
	if (NULL == s || NULL == s->hRTCP)
	{
		RvLogError(rvLogPtr, (rvLogPtr, "RvRtpSessionShutdown: NULL session handle"));
	    RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpSessionShutdown"));
		return RV_ERROR_NULLPTR;
	}	
	result = RvRtcpSessionShutdown(s->hRTCP, reason, reasonLength);
	RvLogLeave(rvLogPtr, (rvLogPtr, "RvRtpSessionShutdown"));
	return result;	
}

RVAPI
RvBool   RVCALLCONV  isMyIP(RvAddress* address)
{
    RvUint32 i;
    RvBool Result = RV_FALSE;
    
    for (i = 0; (i < rvRtpInstance.addressesNum); i++)
    {
        if (RvAddressCompare(&rvRtpInstance.hostIPs[i], address, RV_ADDRESS_BASEADDRESS))
        {
            Result = RV_TRUE;
            break;
        }
    }
    
    return Result;
}
/* == Internal RTP Functions == */


static void rtpEvent(
        IN RvSelectEngine*  selectEngine,
        IN RvSelectFd*      fd,
        IN RvSelectEvents   selectEvent,
        IN RvBool           error)
{
    RvRtpSessionInfo* s;
	RvLogEnter(rvLogPtr, (rvLogPtr, "rtpEvent()"));
    
    RV_UNUSED_ARG(selectEngine);
    RV_UNUSED_ARG(selectEvent);
    RV_UNUSED_ARG(error);

    s = RV_GET_STRUCT(RvRtpSessionInfo, selectFd, fd);

    /* We register to WRITE event when the session should be destructed */
    if (selectEvent & RvSelectWrite)
    {
        RvSelectRemove(s->selectEngine, &s->selectFd);
        RvFdDestruct(&s->selectFd);
        RvSocketDestruct(&s->socket, RV_FALSE, NULL, logMgr);
        if (s->remoteAddressSet)
        {
            RvAddressDestruct(&s->remoteAddress);
        }
        if (s->isAllocated)
            RvMemoryFree(s, logMgr);
        return;
    }

    if (s->eventHandler)
    {
        s->eventHandler((RvRtpSession)s, s->context);
    }
	
	RvLogLeave(rvLogPtr, (rvLogPtr, "rtpEvent()"));	
}



                  /* == ENDS: Internal RTP Functions == */

#ifdef __cplusplus
}
#endif
























