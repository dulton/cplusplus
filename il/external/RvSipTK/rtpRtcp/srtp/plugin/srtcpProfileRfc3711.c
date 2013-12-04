/************************************************************************
 File Name	   : srtcpProfileRfc3711.c
 Description   : scope: Private
     declaration of inplementation of RtpProfilePlugin for SRTP (RFC 3711)
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

#include "rtputil.h"
#include "rvaddress.h"
#include "rvrtpbuffer.h"
#include "rvnetaddress.h"
#include "rvsrtp.h"
#include "RtcpTypes.h"
#include "srtcpProfileRfc3711.h"

#if(RV_LOGMASK != RV_LOGLEVEL_NONE)
#define RTP_SOURCE      rtpGetSource(RVRTP_SRTP_MODULE)
#endif
#include "rtpLogFuncs.h"
#undef FUNC_NAME
#define FUNC_NAME(name) "SrtcpProfileRfc3711" #name

#ifdef __cplusplus
extern "C" {
#endif
    
#if(RV_LOGMASK != RV_LOGLEVEL_NONE)    
    static RvRtpLogger rtpLogManager = NULL;
#define logMngr (RvRtpGetLogManager(&rtpLogManager),((RvLogMgr*)rtpLogManager))
#else
#define logMngr (NULL)
#endif /* (RV_LOGMASK != RV_LOGLEVEL_NONE) */
#define RV_SEND_BUFFER_SIZE (4296)
/************************************************************************************
 * SrtcpProfileRfc3711RawSend
 * description: this function is called, when plugin have to receive 
 *              the SRTCP compound packet filled buffer.
 * input: hRTCP            - Handle of the SRTCP session.
 *        bytesToReceive   - The maximal length to receive.
 * output:
 *        buf              - Pointer to buffer for receiving a message.
 *        bytesReceivedPtr - pointer to received RTCP size
 *        remoteAddressPtr - Pointer to the remote address 
 * return value:  If no error occurs, the function returns the non-neagtive value.
 *                Otherwise, it returns a negative value.
 ***********************************************************************************/
RvStatus SrtcpProfileRfc3711RawSend(
        struct __RtpProfilePlugin* plugin,
        IN    RvRtcpSession hRTCP,
        IN    RvUint8*      bufPtr,
        IN    RvInt32       DataLen,
        IN    RvInt32       DataCapacity,
        INOUT RvUint32*     sentLenPtr)
{
    RvStatus                status = RV_OK;
    rtcpSession*            thisPtr = (rtcpSession *)hRTCP;
//    RvUint8*                bufferPtr = (RvUint8 *)bufPtr;
    RvRtpSessionInfo*       s = NULL;     
    RvUint32                index = 0;       
    RvSocket*               rtcpSockPtr = NULL;
    RvRtpAddressList*       rtcpAddrListPtr =  NULL;
    RvUint32*               packetSizePtr = sentLenPtr;
    RvSize_t                bytesSent  = 0;
    RvDataBuffer            buffer, encryptBuf;
    RvDataBuffer*           bufferPtr  = NULL;
    RvDataBuffer*           sendBufferPtr = &encryptBuf;
    RvAddress*              destAddressPtr = NULL;
    RvSize_t                packetSize     = 0;
    RvBool                  bDestAddressAvailable  = RV_FALSE;
    RvAddress               localAddress;
    RvSrtp*                 srtpPtr    = NULL;
    RvSrtpEncryptionPlugIn* plugInPtr  = NULL;
    RvUint8                 workingBuffer[RV_SEND_BUFFER_SIZE];

    RTPLOG_ENTER(RawSend);
    /* Check to see if sending has been disabled, if yes, abort. */
    if(thisPtr == NULL || (!thisPtr->isAllocated))
        return RV_ERROR_UNKNOWN;
    
    if (thisPtr->rtpSession == NULL)
        return RV_ERROR_UNKNOWN;
    
    s = (RvRtpSessionInfo*) thisPtr->rtpSession; 
   
    rtcpSockPtr = &thisPtr->socket;
    rtcpAddrListPtr = &thisPtr->addressList;

    srtpPtr    =  &(((RtpProfileRfc3711*) plugin->userData)->srtp);
    plugInPtr  =  &srtpPtr->plugIn;
    
    index = thisPtr->index;
    thisPtr->index++;
    if (plugInPtr!=NULL && plugInPtr->rtcpIndexChanged != NULL)
    {
        plugInPtr->rtcpIndexChanged(&hRTCP, srtpPtr, thisPtr->index); /* Notify user */
    }
    if (rtcpSockPtr == NULL)
    {
        RTPLOG_ERROR_LEAVE(RawSend, "NULL socket pointer error");  
        return RV_ERROR_BADPARAM; /* NULL socket pointer */
    } 
    /* setting RvDataBuffer for encryption */
    bufferPtr             = rvDataBufferConstructImport(
        &buffer, bufPtr, 
        DataCapacity, 
        0, 
        DataLen, 
        RV_FALSE);    
    RvSocketGetLocalAddress(rtcpSockPtr, logMngr, &localAddress);  /* get local address */
    destAddressPtr        = RvRtpAddressListGetNext(rtcpAddrListPtr, NULL);
    bDestAddressAvailable = (destAddressPtr!=NULL);
    if (!bDestAddressAvailable)
    {
        RTPLOG_ERROR_LEAVE(RawSend, "No Available RTCP destination");
        return RV_ERROR_NOT_FOUND; /* No Available RTCP destination */
    }
    
    while(destAddressPtr != NULL) 
    {
        if(plugInPtr != NULL) 
        {
            /* Create a data buffer to use for encryption - same size as user's buffer */
#ifdef DYNAMIC_BUFFERS            
            rvDataBufferConstruct(&encryptBuf, rvDataBufferGetCapacity(bufferPtr), 0, NULL);
#else
            rvDataBufferConstructImport(
                &encryptBuf, 
                (RvUint8 *)workingBuffer, 
                sizeof(workingBuffer), 
                100,   /* data offset (front capacity) */
                4096,  /* data length */
                RV_FALSE);
#endif
            sendBufferPtr = &encryptBuf; /* Send from encrypted buffer */
            
            /* Encrypt the packet */
            status = plugInPtr->encryptRtcp(
                thisPtr->rtpSession, 
                plugInPtr->userData, 
                bufferPtr, 
                &encryptBuf,  
                &localAddress, /* open address */
                destAddressPtr, 
                index, 
                (!(srtpPtr->rtcpEncAlg == RV_SRTP_ENCYRPTIONALG_NULL)));
            
            if(status != RV_RTPSTATUS_Succeeded) 
            {
                rvDataBufferDestruct(&encryptBuf);
                RTPLOG_ERROR_LEAVE(RawSend, "Encryption Error");
                return -1; /* Abort send */
            }
            
            /* Send the buffer */
            packetSize = rvDataBufferGetLength(sendBufferPtr); /* for debugging */
            if(rtcpSockPtr != NULL)
            {
                status = RvSocketSendBuffer(
                    rtcpSockPtr, 
                    rvDataBufferGetData(sendBufferPtr), 
                    packetSize, 
                    destAddressPtr, 
                    logMngr, 
                    &bytesSent);
                if (status!=RV_OK || (bytesSent == 0))
                {
                    RTPLOG_ERROR_LEAVE(RawSend, "Sending Error"); 
                    /* no need to abort send; try other destinations */
                }
                else
                {   /* It must be the same size for each destination */
                    if (packetSizePtr!=NULL)
                        *packetSizePtr = packetSize;
                }
            }
            rvDataBufferDestruct(&encryptBuf);
        }
        destAddressPtr = RvRtpAddressListGetNext(rtcpAddrListPtr, destAddressPtr);
    }
    RTPLOG_LEAVE(RawSend);
    RV_UNUSED_ARG(DataCapacity);
    return status;
}

/************************************************************************************
 * SrtcpProfileRfc3711RawReceive
 * description: This routine receives the RTCP compound packet filled buffer.
 *              It decrypts the buffer according to SRTCP rules,
 *              Checks optional SRTCP MKI, if needed, and authentication tag.
 * input: hRTP             - Handle of the RTP session.
 *        rtcpSockPtr      - pointer to the RTCP socket
 *        bytesToReceive   - The maximal length to receive.
 * output:
 *        buf              - Pointer to buffer for receiving a message.
 *        bytesReceivedPtr - pointer to received RTCP size
 *        remoteAddressPtr - Pointer to the remote address 
 * return value:  If no error occurs, the function returns the non-neagtive value.
 *                Otherwise, it returns a negative value.
 ***********************************************************************************/
RvStatus  SrtcpProfileRfc3711RawReceive(
     struct __RtpProfilePlugin* profilePlugin,
     IN    RvRtcpSession hRTCP,
     INOUT RvRtpBuffer* buf,
     IN    RvSize_t bytesToReceive,
     OUT   RvSize_t*   bytesReceivedPtr,
     OUT   RvAddress*  remoteAddressPtr)
{
    RvStatus                res             = RV_ERROR_UNKNOWN;
    RvSize_t                packetLength    = 0;
    rtcpSession*            s = (rtcpSession*)hRTCP;
    RvSocket*  rtcpSockPtr  = NULL; 
    RvDataBuffer            encryptBuf;
    RvDataBuffer*           recvBufPtr;
    RvDataBuffer            buffer;
    RvDataBuffer*           bufferPtr;
    RvRtpStatus             status;
    RvAddress               localAddress;
    RvSrtp*                 srtpPtr    =  NULL;
    RvSrtpEncryptionPlugIn* plugInPtr  =  NULL;

    RTPLOG_ENTER(RawReceive);

    if (hRTCP == NULL || profilePlugin == NULL || ((rtcpSockPtr=&s->socket) == NULL) )
    {
        
        RTPLOG_ERROR_LEAVE(RawReceive, "NULL parameter");
        return RV_ERROR_NULLPTR;
    }

    srtpPtr    =  &(((RtpProfileRfc3711*) profilePlugin->userData)->srtp);
    plugInPtr  =  &srtpPtr->plugIn;
    bufferPtr = rvDataBufferConstructImport(&buffer, buf->buffer, bytesToReceive, 0, 0, RV_FALSE);

    if(plugInPtr != NULL) 
    {
        /* Encryption: create a duplicate buffer to received encrypted data into. */
        rvDataBufferConstruct(&encryptBuf, bytesToReceive, 0, NULL);
        recvBufPtr = &encryptBuf;
    }
    else 
    {
        recvBufPtr = bufferPtr; /* No encryption, just receive into user's buffer */
    }
    
    RvSocketGetLocalAddress(rtcpSockPtr, logMngr, &localAddress);  /* get local address */

    res = RvSocketReceiveBuffer(
        rtcpSockPtr, 
        rvDataBufferGetBuffer(recvBufPtr),   /* pointer to read from */
        rvDataBufferGetCapacity(recvBufPtr), /* Maximum size to read */
        logMngr, 
        &packetLength, 
        remoteAddressPtr);

	if((res != RV_OK) || (packetLength == 0))
    {
        if(plugInPtr != NULL)
            rvDataBufferDestruct(&encryptBuf);
        RTPLOG_ERROR((RTP_SOURCE, "SrtcpProfileRfc3711RawReceive: failed to read from socket %d" , *rtcpSockPtr));
        return RV_ERROR_NULLPTR;        
    }
    /* Adjust buffer to reflect data received. */
    rvDataBufferSetDataPosition(recvBufPtr, RvUint32Const(0), (RvUint32)packetLength);
    
    /* For Windows Multicast, ignore looped back packets */
    /* If encryption is being used, decrypt the packet into the user's buffer. */
    if(plugInPtr != NULL) 
    {
        status = plugInPtr->decryptRtcp(
            s->rtpSession, 
            plugInPtr->userData, 
            &encryptBuf, 
            bufferPtr, 
            remoteAddressPtr, 
            &localAddress);
        
        rvDataBufferDestruct(&encryptBuf); /* No longer needed */
        if(status != RV_RTPSTATUS_Succeeded) 
        {
            /* Error, just abort */
            RTPLOG_ERROR_LEAVE(RawReceive, "Decryption Error");
            return status;
        }
    } 
    /* Remove the 32-bit random number attached to header */
    /*          rvDataBufferSkip(bufferPtr,4);*/
    *buf = buffCreate(rvDataBufferGetData(bufferPtr), rvDataBufferGetLength(bufferPtr));
    *bytesReceivedPtr = rvDataBufferGetLength(bufferPtr);
        

    RTPLOG_LEAVE(RawReceive);
    return res;
}

#ifdef __cplusplus
}
#endif
