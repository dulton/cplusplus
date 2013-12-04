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
/********************************************************************
 *                       srtpSession.c                              *
 *                                                                  *
 * The following program is a simple console application that       *
 * demonstrates how to implement SRTP sending and                   *
 * receiving capabilities using the                                 *
 * RADVISION Advanced RTP Protocol Toolkit.                         *
 * The program does the following :                                 *
 * 1. Initializes the RTP and RTCP Stacks.                          *
 * 2. Opens RTP and RTCP sessions on a local address.               *
 * 3. Sets "external" encryption and authentication                 *
 *    plug-ins for the SRTP add-on.                                 *
 * 4. Initializes SRTP session plug-in.                             *
 * 5. Sets RTP and RTCP destination encryption contexts.            *
 * 6. Sets RTP and RTCP remote source encryption contexts.          *
 * 7. Sets remote addresses for RTP and RTCP sessions.              *
 * 8  Sends "dummy" g.711 PCMU 20 ms packets.                       *
 * 9. Automatically sends and receives RTCP reports.                *
 * 10. Closes RTP and RTCP sessions.                                *
 * 11. Shuts down RTP and RTCP Stacks.                              *
 * This program must be compiled with #define APP_1 and without one.*
 * Both applications must be launched at the same time on           *
 * the same platform for SRTP demonstration                         *
 ********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include "osDependency.h"
#include "rtp.h"
#include "rtcp.h"
#include "rvrtpseli.h"
#include "payload.h"
#include "rv_srtp.h"
#include "rvsrtpaesplugin.h"
#include "rtptSrtpaesencryption.h"
#include "rtptSha1.h"
#include "rvsrtpsha1.h"


#define RTP_NUMBER_PACKETS_TO_SEND         (1000)
#if defined(APP_1)
   #define RTP_SESSION_RECEIVING_PORT          (5500)
   #define RTP_SESSION_SENDING_PORT            (6500)
   #define RTP_LOCAL_SESSION_SSRC             (0xBEEFDEAD)
   #define RTP_REMOTE_SESSION_SSRC            (0xDEADBEEF)     
   #define RTP_INITIAL_LOCAL_SEQUECE_NUMBER   (0xDEAD)
   #define RTP_INITIAL_REMOTE_SEQUECE_NUMBER  (0xBEEF)
   #define RTCP_SDES                          "RTP App1"
#else
   #define RTP_SESSION_RECEIVING_PORT          (6500)
   #define RTP_SESSION_SENDING_PORT            (5500)
   #define RTP_LOCAL_SESSION_SSRC             (0xDEADBEEF)
   #define RTP_REMOTE_SESSION_SSRC            (0xBEEFDEAD)   
   #define RTP_INITIAL_LOCAL_SEQUECE_NUMBER   (0xBEEF)
   #define RTP_INITIAL_REMOTE_SEQUECE_NUMBER  (0xDEAD)
   #define RTCP_SDES                          "RTP App2"
#endif

#define SRTP_MKI_SIZE_DEFAULT        (4)
#define SRTP_MASTER_KEY_SIZE_DEFAULT (16)
#define SRTP_SALT_KEY_SIZE_DEFAULT   (14)

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

RvRtpSession rtpSession   = NULL;                        /* Stores RTP session handle  */
RvRtcpSession rtcpSession = NULL;                        /* Stores RTCP session handle */
RvUint32 receivedPackets  = 0;                           /* Counter of received RTP packets */
RvUint32 rtpTimestamp     = 12345;                       /* Stores current RTP timestamp */
unsigned char mki      [SRTP_MKI_SIZE_DEFAULT];          /* Stores SRTP MKI */
unsigned char masterkey[SRTP_MASTER_KEY_SIZE_DEFAULT];   /* Stores master key */
unsigned char saltkey  [SRTP_SALT_KEY_SIZE_DEFAULT];     /* Stores salt key */

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS PROTOTYPES                 */
/*-----------------------------------------------------------------------*/

static int SendRTPPacket(int packetNumber);

static void rtpEventHandler(
        IN  RvRtpSession  hRTP,
        IN  void *       context);

static RvBool Rv_inet_pton4(const RvChar *src, RvUint8 *dst);


/************************************************************************************
 * RvRtpSetRtpSequenceNumber 
 * description: This function sets the RTP sequence number for a packet that will be sent.
 *              This is not a documented function, used only for debugging.
 *              In real life, there is no need to call this function because the local sequence
 *              number must be random and has to be transfered to the remote 
 *              session through the signaling Stack (SDP or H.245).
 *              We use this function here in order to simplify the sample.                         
 *              Please do not use this function in your application. 
 * input: hRTP  - Handle of the RTP session.
 *        sn    - Sequence number to set
 * return value:  If no error occurs, the function returns 0.
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvRtpSetRtpSequenceNumber(IN  RvRtpSession  hRTP, RvUint16 sn);


/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/* Main application */
#if (RV_OS_TYPE != RV_OS_TYPE_VXWORKS)
int AppMain(int argc, char* argv[])
#else
int AppMain()
#endif
{
    RvStatus               status =  RV_ERROR_UNKNOWN;
    RvNetAddress           rtpReceivingAddress;
    RvNetAddress           rtcpReceivingAddress;
    RvNetAddress           rtpSendingAddress;
    RvNetAddress           rtcpSendingAddress;
    RvInt32       pack       = 0;                /* packet number */
    RvSrtpAesEncryption    aesEncryption;        /* AES encryption plugin implementation */
    RvNetIpv4              address;

#if (RV_OS_TYPE != RV_OS_TYPE_VXWORKS)
    RV_UNUSED_ARG(argc); /* Just to remove warning */
    RV_UNUSED_ARG(argv); /* Just to remove warning */
#endif

    /* Initialization of RTP Stack */
    status = RvRtpInit();
    if (status != RV_OK)
    {
        printf("ERROR: RvRtpInit() failed\n");
        return status;
    }
    
    /* Initialization of RTCP Stack (Requires SELI interface for automatic RTCP reports) */
    status = RvRtcpInit();

    
    if (status != RV_OK)
    {
        printf("ERROR: RvRtcpInit() failed\n");
        return status;
    }

    /* Creating RTP listening address (ip:port [:scope - for IPV6] )*/
    address.port   = RTP_SESSION_RECEIVING_PORT;
    Rv_inet_pton4("127.0.0.1", (RvUint8 *)&address.ip); /* local address */

    /* Creating IPV4 RTP listening address */
    RvNetCreateIpv4(&rtpReceivingAddress, &address);


    /* Filling SRTCP receiving address (structure rtcpSendingAddress) */
    address.port    = RTP_SESSION_RECEIVING_PORT + 1;  

    /* Creating RTCP sending address (RTP port+1) */
    RvNetCreateIpv4(&rtcpReceivingAddress, &address);


    /* Opening RTP and RTCP sessions. */
    /* In real life, SSRC must be generated as a random or pseudo random number, and must be transfered *
     * to the remote session by a mechanism that is out of the scope of SRTP.                           *
     * In order to demonstrate SRTP, we set fixed SSRC and sequence numbers to the RTP session.         *
     * SDES must be unique for all RTP session participants.                                            */

    rtpSession  = RvRtpOpenEx(&rtpReceivingAddress, RTP_LOCAL_SESSION_SSRC, 0xFFFFFFFF, RTCP_SDES);

    if (rtpSession == NULL)
    {
        /* For example it can occur when the specified port is busy. */
        printf("ERROR: RvRtpOpenEx() cannot open RTP session.\n");
        return -1;
    }
    printf("RTP session is opened on port %d\n", RTP_SESSION_RECEIVING_PORT);
    rtcpSession = RvRtpGetRTCPSession(rtpSession);

    /**************************************************************************************
     * IMPORTANT:                                                                         *
     * The following function sets the RTP sequence number for a packet that will be sent.*
     * This is non-documented function, used only for debugging.                          *
     * In real life, there is no need to call this function, because the local sequence   *
     * number with local SSRC must be random and has to be transfered to the remote       *
     * session via the signaling Stack (SDP or H.245).                                    *
     * We use this function here in order to simplify the sample.                         *
     * Please, do not use this function in your application.                              *
     **************************************************************************************/
    RvRtpSetRtpSequenceNumber(rtpSession, RTP_INITIAL_LOCAL_SEQUECE_NUMBER);
    
    /*  Constructing and allocating the SRTP plug-in for the RTP session from Dynamic Memory.       *
     *  The same plug-in will be used for the RTCP session.                                         */

    status = RvSrtpConstruct(rtpSession);
    if (status != RV_OK)
    {
        printf("ERROR: RvSrtpConstruct() failed. Cannot construct plugin.\n");
        return -1;
    }

    /* Constructing AES external to SRTP library plug-in. */ 
    RvSrtpAesEncryptionConstruct(&aesEncryption);

    /* Setting HMAC-SHA1 external to SRTP library callback. */     
    RvRtpSetSha1EncryptionCallback(RvHmacSha1);

    /****************************************************************************************
     * At this point (between RvSrtpConstruct() and RvSrtpInit()) the user can configure    *
     * non-default values for SRTP/SRTCP sessions.                                          *
     * These configurations for local and remote sessions must be the same.                 *
     * For more information, see the SRTP configuration chapter and the rvsrtpconfig.h file *
     ****************************************************************************************/    

    /*  Initializing (registering) the SRTP plug-in with the RTP/RTCP sessions and          *
     *  allocating SRTP plug-in hashes, pools and databases.                                */
    status = RvSrtpInit(rtpSession, RvSrtpAesEncryptionGetPlugIn(&aesEncryption));
    if (status != RV_OK)
    {
        printf("ERROR: RvSrtpInit() failed. Cannot register SRTP plugin or allocate SRTP plugin databases.\n");
        return -1;
    }

    /* Initializing of MKI, master and salt keys */
    memcpy(mki,       (RvUint8*)"1234", SRTP_MKI_SIZE_DEFAULT);
    memcpy(masterkey, (RvUint8*)"1234567890123456", SRTP_MASTER_KEY_SIZE_DEFAULT);
    memcpy(saltkey,   (RvUint8*)"12345678901234", SRTP_SALT_KEY_SIZE_DEFAULT);

    /* Adding  MKI, master and salt keys to the session */
    status = RvSrtpMasterKeyAdd(rtpSession, mki, masterkey, saltkey);


    /* Filling SRTP sending address (structure rtpSendingAddress) */

    address.port    = RTP_SESSION_SENDING_PORT;  
    Rv_inet_pton4("127.0.0.1", (RvUint8 *)&address.ip); /* local address */

    /* Creating IPV4 RTP sending address. */
    RvNetCreateIpv4(&rtpSendingAddress, &address);

    /* Filling SRTCP sending address (structure rtcpSendingAddress) */
    address.port    = RTP_SESSION_SENDING_PORT + 1;  

    /* Creating RTCP sending address (RTP port+1). */
    RvNetCreateIpv4(&rtcpSendingAddress, &address);

    /* Setting SRTP destination encryption contexts for RTP and for RTCP. */
    
    RvSrtpAddDestinationContext(rtpSession, &rtpSendingAddress, RvSrtpStreamTypeSRTP, mki, RV_FALSE);
    RvSrtpAddDestinationContext(rtpSession, &rtcpSendingAddress, RvSrtpStreamTypeSRTCP, mki, RV_FALSE);

    /* Setting SRTP remote source encryption contexts for RTP and for RTCP (2 steps): */
    /* 1. Adds remote sources for RTP and for RTCP */
    RvSrtpRemoteSourceAddRtp (rtpSession, RTP_REMOTE_SESSION_SSRC, 0, RTP_INITIAL_REMOTE_SEQUECE_NUMBER);
    RvSrtpRemoteSourceAddRtcp(rtpSession, RTP_REMOTE_SESSION_SSRC, 0);
    /* 2. Sets remote source encryption contexts for RTP and for RTCP */
    RvSrtpRemoteSourceChangeKey(rtpSession, RTP_REMOTE_SESSION_SSRC, RvSrtpStreamTypeSRTP,  mki, RV_FALSE);
    RvSrtpRemoteSourceChangeKey(rtpSession, RTP_REMOTE_SESSION_SSRC, RvSrtpStreamTypeSRTCP, mki, RV_FALSE);

    /* Initializing SELI interface. */
    status = RvRtpSeliInit();
    
    /* Setting event handler for non-blocking RTP reading. */
    /* Don't use this function for blocking reading. */
    RvRtpSetEventHandler(rtpSession, (RvRtpEventHandler_CB)rtpEventHandler, NULL);    
    
    /* Synchronization point: both local and Remote RTP sessions must be in the same state at this point. */
    /* We do not perform real synchronization, we just wait up to 20 seconds.                             */
    printf("waiting for synchronization\n");
    RvRtpSeliSelectUntil(20000);


    RvRtpAddRemoteAddress(rtpSession, &rtpSendingAddress);
    if (rtcpSession != NULL)
    {
        /* Remote RTCP address setting for RTCP session */
        RvRtcpAddRemoteAddress(rtcpSession, &rtcpSendingAddress);
        /* RvRtcpSessionSetParam(rtcpSession, RVRTCP_PARAMS_RTPCLOCKRATE, 8000);  
           clock rate - needed for dynamic payload types */
    }
    
    for (pack=0; pack < RTP_NUMBER_PACKETS_TO_SEND; pack++)
    {
      /* 
         The following calls are responsible for activating the process that selects
         all registered file descriptors and calls to the appropriate callback
         function when an event occurs.
         Actually we have a number of cases when RvRtpSeliSelectUntil(20) exits without waiting
         20 ms for RTCP automatic periodic report processing (for sending and receiving).
       */
             RvRtpSeliSelectUntil(0);
             RvRtpSeliSelectUntil(0);
             RvRtpSeliSelectUntil(20);

        /********************************************************************************
         * IMPORTANT:                                                                   *
         * The following function SendRTPPacket() is usually called                     *
         * in a separate thread with delays of 20 ms,                                   *
         * as it is required for g.711 20 ms packets.                                   *
         * It is called here just for the demonstration of RvRtpWrite() usage.          *
         * We have no common interface for creating threads on many operating systems   *
         * that we support. Therefore we demonstrate RvRtpWrite usage in this way.      *
         * Pay attention that RvRtpSeliSelectUntil() also invokes RTCP periodic         *
         * sending and receiving. In case such events occur,                            *
         * the delay between RTP sending will be lower than 20 ms.                      *
         ********************************************************************************/
        SendRTPPacket(pack);
    }

    /* Just to receive the remained packets */
    for (pack=0; pack < RTP_NUMBER_PACKETS_TO_SEND; pack++)
        RvRtpSeliSelectUntil(20);

    /* closing RTP and RTCP sessions */
    status = RvRtpClose(rtpSession);
    if (status != RV_OK)	
    {
        printf("ERROR: RvRtpClose() cannot close RTP session.\n");
        return -1;
    }

    /* Ending SELI interface. */
    status = RvRtpSeliEnd();
    if (status != RV_OK)
    {
        printf("ERROR: RvRtpSeliEnd() failed.\n");
        return -1;
    }

    /* Shuts down the RTCP module.  */
    status = RvRtcpEnd();
    if (status != RV_OK)
    {
        printf("ERROR: RvRtcpEnd() cannot finish to work with RTCP Stack.\n");
        return -1;
    }

    /* Shutting down the RTP module.  */
    RvRtpEnd();

    printf( "\nreceived %d packets\n", receivedPackets);

    return status;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/*******************************************************************************
 * SendRTPPacket() sends dummy g.711 PCMU RTP packet to the remote RTP session *
 * packetNumber - packet number                                                *
 ******************************************************************************/

int SendRTPPacket(int packetNumber)
{
    unsigned char       buffer2send[200];
    RvRtpParam p;
    
    /* Initializing of RvRtpParam structure. */
    RvRtpParamConstruct(&p);
    
    /* Initializing of buffer. */
    memset(buffer2send, 0, sizeof(buffer2send));

    /* Getting the RTP header size with payload header size (0 - in case of g.711). */
    p.sByte = RvRtpPCMUGetHeaderLength();

    /* Setting the payload header - no payload header in g.711 case. */
    /* Putting RTP payload type into structure p (for RTP header). */
    RvRtpPCMUPack(buffer2send, 160, &p, NULL);

    /* "Dummy RTP packet number %d" - just to fill the buffer */
    /* Pay attention that the payload information begins after p.sByte bytes. */
    sprintf(buffer2send+p.sByte, "Dummy RTP packet number %d", packetNumber); 
    rtpTimestamp += 160;
    p.timestamp = rtpTimestamp; /* 20 ms packet, G.711  sample rate = 8000 samples/sec, 20ms packet => 160 */
    /* p.sequenceNumber will be incremented automatically. */

    /* 160 bytes - packet size */
    /* Sequence number is calculated automatically */
    /* RvRtpWrite sends the RTP packet. 
       160+p.sByte is RTP header size (12) + payload header(0) +  payload(160) = 172 */
    return RvRtpWrite(rtpSession, buffer2send, 160 + p.sByte, &p); /* 20 ms of g.711 PCMU packet sending */ 
}

/*****************************************************************
 * This function is the implementation of the
 * RTP event handler for the RTP session. 
 * It is called when RTP packets are received from the network
 * parameters:
 * hRTP - Handle of RTP session
 * context - Context that was set by RvRtpSetEventHandler().
 * returns: NONE.
 *****************************************************************/
void rtpEventHandler(
        IN  RvRtpSession  hRTP,
        IN  void *       context)
{
    int status = -1;
    unsigned char bufferToreceive[200];
    RvRtpParam p;

    RV_UNUSED_ARG(context); /* just to remove warning */

    /* Reading the RTP message */
    status = RvRtpRead    (hRTP, bufferToreceive, sizeof(bufferToreceive), &p);
    if (status < 0)
        return; /* error */

    RvRtpPCMUUnpack(bufferToreceive,  sizeof(bufferToreceive), &p, NULL);

    /* Here, p contains all related to RTP header information and 
       the actual processing of the payload may be performed. */

    if (memcmp(bufferToreceive + p.sByte, "Dummy RTP packet number", strlen("Dummy RTP packet number"))==0)
    {
        printf ("*"); /* received right packet */
        receivedPackets++;
    }
    else
    {
        printf ("\nreceived wrong packet\n");
    }
    return;
}


/* int
 * inet_pton4(src, dst)
 *  like inet_aton() but without all the hexadecimal and shorthand.
 * return:
 *  1 if `src' is a valid dotted quad, else 0.
 * notice:
 *  does not touch `dst' unless it's returning 1.
 * author:
 *  Paul Vixie, 1996.
 */
static RvBool Rv_inet_pton4(const RvChar *src, RvUint8 *dst)
{
    static RvChar digits[] = "0123456789";
    RvInt saw_digit, octets, ch;
    RvUint8 tmp[4], *tp;

    saw_digit = 0;
    octets = 0;
    *(tp = tmp) = 0;
    while ((ch = *src++) != '\0' && (ch != ':')) {
        RvChar *pch;

        if ((pch = strchr(digits, ch)) != NULL) {
            RvUint newValue = *tp * 10 + (pch - digits);

            if (newValue > 255)
                return RV_FALSE;
            *tp = (RvUint8)newValue;
            if (! saw_digit) {
                if (++octets > 4)
                    return RV_FALSE;
                saw_digit = 1;
            }
        } else if (ch == '.' && saw_digit) {
            if (octets == 4)
                return RV_FALSE;
            *++tp = 0;
            saw_digit = 0;
        } else
            return RV_FALSE;
    }
    if (octets < 4)
        return RV_FALSE;
    memcpy(dst, tmp, 4);
    return RV_TRUE;
}



#ifdef __cplusplus
}
#endif

