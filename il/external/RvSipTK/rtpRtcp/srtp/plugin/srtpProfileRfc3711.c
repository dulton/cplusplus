/************************************************************************
 File Name	   : RtpProfileRfc3711.c
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

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)

#include <errno.h>

#endif
//SPIRENT_END

#include "rtputil.h"
#include "rvaddress.h"
#include "rvrtpbuffer.h"
#include "rvnetaddress.h"
#include "rv_srtp.h"
#include "srtpProfileRfc3711.h"
#include "srtcpProfileRfc3711.h"

#if(RV_LOGMASK != RV_LOGLEVEL_NONE)
#define RTP_SOURCE      rtpGetSource(RVRTP_SRTP_MODULE)
#endif
#include "rtpLogFuncs.h"
#undef FUNC_NAME
#define FUNC_NAME(name) "SrtpProfileRfc3711" #name

#define RTP_DESTINATION_UNREACHEBLE  (RvSocketErrorCode(RV_ERROR_UNKNOWN))

#ifdef __cplusplus
extern "C" {
#endif

#if(RV_LOGMASK != RV_LOGLEVEL_NONE)
    static RvRtpLogger rtpLogManager = NULL;
#define logMngr (RvRtpGetLogManager(&rtpLogManager),((RvLogMgr*)rtpLogManager))
#else
#define logMngr (NULL)
#endif /* (RV_LOGMASK != RV_LOGLEVEL_NONE) */

RvInt32  SrtpProfileRfc3711Read(
        struct __RtpProfilePlugin* plugin,
        IN   RvRtpSession          hRTP,
        IN   void *                buf,
        IN   RvInt32               len,
        OUT  RvRtpParam *          p,
        OUT  RvNetAddress*         remAddressPtr);

RvInt32  SrtpProfileRfc3711Write(
    struct __RtpProfilePlugin* plugin,
    IN  RvRtpSession           hRTP,
    IN  void *                 buf,
    IN  RvInt32                len,
    IN  RvRtpParam*            p);

RvInt32  SrtpProfileRfc3711RemoveRemoteAddress(
    IN struct __RtpProfilePlugin* plugin,
    IN RvRtpSession               session,
    IN RvNetAddress*              address);

/************************************************************************************
 * SrtpProfileRfc3711AddRemoteAddress
 * description: This function adds destination for the session.
 * input: plugin       - pointer to this plugin
 *        hRTP         - Handle of the RTP session.
 *        destType     - RTP TRUE, RTCP FALSE
 *        address      - pointer to destination address
 * output: (-)
 * return value:  If no error occurs, the function returns the non-neagtive value.
 *                Otherwise, it returns a negative value.
 ***********************************************************************************/
RvInt32  SrtpProfileRfc3711AddRemoteAddress(
    IN struct __RtpProfilePlugin* plugin,
    IN RvRtpSession               session,
    IN RvBool                     destType, /* TRUE for RTP, FALSE for RTCP */
    IN RvNetAddress*              address);


void  SrtpProfileRfc3711Release(
    struct __RtpProfilePlugin* profilePlugin,
    IN    RvRtpSession         hRTP);

static RtpProfilePluginCallbacks SrtpProfileRfc3711Callbacks =
{
        SrtpProfileRfc3711Read,
        SrtpProfileRfc3711Write,
        SrtcpProfileRfc3711RawReceive,
        SrtcpProfileRfc3711RawSend,
        SrtpProfileRfc3711RemoveRemoteAddress,
        SrtpProfileRfc3711AddRemoteAddress,
        SrtpProfileRfc3711Release
};

/************************************************************************************
 * SrtpProfileRfc3711Read
 * description: This routine reads the SRTP message and sets the header of the RTP message.
 * input: hRTP  - Handle of the RTP session.
 *        buf   - Pointer to buffer containing the RTP packet with room before first
 *                payload byte for RTP header.
 *        len   - Length in bytes of buf.
 *
 * output: p              - A struct of RTP param,contain the fields of RTP header.
 *         remAddressPtr  - remote address
 * return value: If no error occurs, the function returns the non-negative value.
 *                Otherwise, it returns a negative value.
 * Notes:
 * 1) This function returns an error, when RvRtpWrite was called to unreachable destination.
 * 2) This function returns an error, when RvRtpClose was called to close the session
 *    (for exiting from blocked mode).
 ***********************************************************************************/
RvInt32  SrtpProfileRfc3711Read(
        struct __RtpProfilePlugin* plugin,
        IN   RvRtpSession          hRTP,
        IN   void *                buf,
        IN   RvInt32               len,
        OUT  RvRtpParam *          p,
        OUT  RvNetAddress*         remAddressPtr)
{
    RvRtpSessionInfo*      s          = (RvRtpSessionInfo*) hRTP;
    RvSrtpEncryptionPlugIn *plugInPtr = NULL;
    RvSrtp*                srtpPtr    = NULL;
    RvDataBuffer           buffer;
    RvDataBuffer           encryptBuf;
    RvDataBuffer           *recvBufPtr;
    RvRtpStatus            status;
    RvStatus               res, recvStatus;
    RvSize_t               packetLength;
    RvAddress              sourceAddr, localAddress;
    RvUint8*               tempPtr = NULL;

    RTPLOG_ENTER(Read);

    if (s==NULL||plugin==NULL)
    {
        RTPLOG_ERROR_LEAVE(Read,"SRTP is not constructed and inited or NULL session pointer.");
        return RV_ERROR_BADPARAM;
    }
    srtpPtr = (RvSrtp*)&(((RtpProfileRfc3711*)plugin->userData)->srtp);
    plugInPtr = &srtpPtr->plugIn;
    /* get local address */
    RvSocketGetLocalAddress(&s->socket, logMngr, &localAddress);

    rvDataBufferConstructImport(&buffer, (RvUint8 *)buf, len, len, 0, RV_FALSE);
    /* Set up for decryption if it is being used */
    if(plugInPtr != NULL) {
        /* Encryption: create a duplicate buffer to received encrypted data into. */
        rvDataBufferConstruct(&encryptBuf, rvDataBufferGetCapacity(&buffer), 0, NULL);
        recvBufPtr = &encryptBuf;
    } else
        recvBufPtr = &buffer; /* No encryption, just receive into user's buffer */


    /* Receive data into a buffer -- NEED TO UNLOCK in sync mode!!! */
    recvStatus = RvSocketReceiveBuffer(
        &s->socket,
        rvDataBufferGetBuffer(recvBufPtr),
        rvDataBufferGetCapacity(recvBufPtr),
        NULL,
        &packetLength,
        &sourceAddr);

    if (recvStatus == RTP_DESTINATION_UNREACHEBLE)
    {
        rvDataBufferDestruct(&buffer);
        rvDataBufferDestruct(&encryptBuf);
       /* error of sending from socket to an unreachable destination.
           Since used the same socket for sending and receiving, if sending is failed,
           here we receive the sending error. In this case logging will be bombed by error messages.
           This is the design defect, but it allow to use 1 socket per RTP session instead of two.*/
        /* RTPLOG_ERROR_LEAVE(Read, "RvSocketReceiveBuffer(). The sending destination is unreachable.");
           it is intentionally omitted not to produce printings loops */
        return recvStatus;
    }

    if (((RvInt32)packetLength) < RvRtpGetHeaderLength())
    {
        rvDataBufferDestruct(&buffer);
        rvDataBufferDestruct(&encryptBuf);
        RTPLOG_ERROR_LEAVE(Read, "This packet is probably corrupted or session is closed in blocking mode.");
        return RV_ERROR_UNKNOWN;
    }
    if (remAddressPtr!=NULL)
    {
        if (RvAddressGetType(&sourceAddr) == RV_ADDRESS_TYPE_IPV6)
        {
#if (RV_NET_TYPE & RV_NET_IPV6)
            RvNetIpv6 Ipv6;
            memcpy(Ipv6.ip, RvAddressGetIpv6(&sourceAddr)->ip, sizeof(Ipv6.ip));
            Ipv6.port = RvAddressGetIpv6(&sourceAddr)->port;
            Ipv6.scopeId = RvAddressGetIpv6(&sourceAddr)->scopeId;
            RvNetCreateIpv6(remAddressPtr, &Ipv6);
#else
            rvDataBufferDestruct(&buffer);
            rvDataBufferDestruct(&encryptBuf);
            RTPLOG_ERROR_LEAVE(Read,"IPV6 is not supported in current configuration. remAddressPtr was not filled.");
            return RV_ERROR_NOTSUPPORTED;
#endif
        }
        else
        {
            RvNetIpv4 Ipv4;
            if (RvAddressGetIpv4(&sourceAddr)!=NULL)
            {
                Ipv4.ip = RvAddressGetIpv4(&sourceAddr)->ip;
                Ipv4.port = RvAddressGetIpv4(&sourceAddr)->port;
                RvNetCreateIpv4(remAddressPtr, &Ipv4);
            }
        }
    }
    rvDataBufferSetDataPosition(recvBufPtr, RvUint32Const(0), (RvUint32)packetLength);
    /* If encryption is being used, decrypt the packet into the user's buffer. */
    if(plugInPtr != NULL) {
        status = plugInPtr->decryptRtp(
            hRTP,
            plugInPtr->userData,
            &encryptBuf,
            &buffer,
            &sourceAddr,
            &localAddress); /* may be use local address from rtpInstatnce */

        rvDataBufferDestruct(&encryptBuf); /* No longer needed */
        if(status != RV_RTPSTATUS_Succeeded) {
            /* Error, just abort */
            RTPLOG_ERROR_LEAVE(Read, "Decryption Error");
            return status;
        }
    }
    tempPtr = rvDataBufferGetData(&buffer);
    ConvertFromNetwork(tempPtr, 0, 3);
    /* If the SSRC is ours and the is our IP, we should ignore this packet - it probably came
    in as a multicast packet we sent out. */
    if ( (s->sSrc == ((RvUint32*)tempPtr)[2]) && isMyIP(&sourceAddr))
    {
        rvDataBufferDestruct(&buffer);
        RvAddressDestruct(&sourceAddr);
        RTPLOG_ERROR_LEAVE(Read, "this packet probably came in as a multicast packet we sent out.");
        return RV_ERROR_UNKNOWN;
    }
    p->len = (RvInt32)rvDataBufferGetLength(&buffer);
    /* Extract the RTP header from the packet */
    res = RvRtpUnpack(hRTP, rvDataBufferGetData(&buffer), rvDataBufferGetLength(&buffer), p);
    if  (res < 0)
    {
        rvDataBufferDestruct(&buffer);
        RTPLOG_ERROR_LEAVE(Read, "This packet is probably corrupted.");
        return RV_ERROR_UNKNOWN;
    }
    RTPLOG_LEAVE(Read)
    return res;
}

#define RV_SEND_BUFFER_SIZE (4296)
/************************************************************************************
 * SrtpProfileRfc3711Write
 * description: this callback is called, when plugin have to send the XRTP packet.
 * input: plugin - pointer to this plugin
 *        hRTP   - Handle of the RTP session.
 *        buf    - Pointer to buffer containing the RTP packet with room before first
 *                payload byte for RTP header.
 *        len    - Length in bytes of buf.
 *        p      - A struct of RTP param.
 * output: none.
 * return value:  If no error occurs, the function returns the non-neagtive value.
 *                Otherwise, it returns a negative value.
 ***********************************************************************************/
RvInt32  SrtpProfileRfc3711Write(
        struct __RtpProfilePlugin* plugin,
        IN  RvRtpSession           hRTP,
        IN  void *                 buf,
        IN  RvInt32                len,
        IN  RvRtpParam*            p)
{
    RvStatus                res = RV_OK;
    RvSrtpEncryptionPlugIn *plugInPtr = NULL;
    RvSrtp*                 srtpPtr    = NULL;
    RvRtpSessionInfo        *s  = (RvRtpSessionInfo *)hRTP;
/* the next variables are added  for encryption */
    RvUint8                 workingBuffer[RV_SEND_BUFFER_SIZE];
    RvDataBuffer            buffer;
	RvDataBuffer*           bufferPtr;
	RvAddress               localAddress, *destAddressPtr = NULL;
    RvBool                  bDestAddressAvailable = RV_FALSE;
	RvDataBuffer            encryptBuf;
	RvDataBuffer*           sendBufferPtr = &encryptBuf;
    RvSize_t                bytesSent  = 0;
    RvInt32                UserAllocationSize = p->len; /* if encryption is used interface feature */
    RvUint32               roc;

    RTPLOG_ENTER(Write);
    if (s==NULL||plugin==NULL)
    {
        RTPLOG_ERROR_LEAVE(Write, "SRTP is not constructed and inited or NULL session pointer.");
        return RV_ERROR_BADPARAM;
    }
    srtpPtr = (RvSrtp*)&(((RtpProfileRfc3711*)plugin->userData)->srtp);
    plugInPtr = &srtpPtr->plugIn;

    p->paddingBit = RV_FALSE; /* for security purpose only */

    RvSocketGetLocalAddress(&s->socket, logMngr, &localAddress);  /* get local address */
    bufferPtr = rvDataBufferConstructImport(
        &buffer, (RvUint8 *)buf, UserAllocationSize, 0, len, RV_FALSE);
	/* Get the number of padding bytes required  */
	/* Note: Perform this after adding CSRCs to header so RvRtpGetHeaderLength */
	/* gives the correct value  */
    /* taking the roll over counter */
    roc = s->roc;
    RTPLOG_DEBUG((RTP_SOURCE, "SrtpProfileRfc3711Write():: calling RvRtpPack()"));
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(p->packFunction) {
      res=(*(p->packFunction))((void*)hRTP, rvDataBufferGetData(bufferPtr), len, p, p->pfarg);
    } else {
      res = RvRtpPack(hRTP, rvDataBufferGetData(bufferPtr), len, p);
    }
#else
    res = RvRtpPack(hRTP, rvDataBufferGetData(bufferPtr), len, p);
#endif
//SPIRENT_END
    if (plugInPtr!=NULL && plugInPtr->rtpSeqNumChanged!=NULL)
    {   /* sequence number changed callback */
        plugInPtr->rtpSeqNumChanged(&hRTP, srtpPtr, s->sequenceNumber, s->roc);
    }
    destAddressPtr = RvRtpAddressListGetNext(&s->addressList, NULL);
    bDestAddressAvailable = (destAddressPtr!=NULL);
    while(destAddressPtr != NULL)
    {
        if(plugInPtr != NULL)
        {
            /* Create a data buffer to use for encryption - same size as user's buffer */
#ifdef DYNAMIC_BUFFERS
            sendBufferPtr = rvDataBufferConstruct(
                &encryptBuf,
                rvDataBufferGetCapacity(bufferPtr),
                0,
                NULL);
#else
            sendBufferPtr = rvDataBufferConstructImport(
                &encryptBuf,
                (RvUint8 *)workingBuffer,
                sizeof(workingBuffer),
                100,   /* data offset (front capacity) */
                4096,  /* data length */
                RV_FALSE);
#endif
            /* Encrypt the packet */
            res = plugInPtr->encryptRtp(
                hRTP,
                plugInPtr->userData,
                bufferPtr,
                &encryptBuf,
                &localAddress,
                destAddressPtr,
                roc);

            if(res != RV_RTPSTATUS_Succeeded)
            {
                rvDataBufferDestruct(&encryptBuf);
                RTPLOG_ERROR_LEAVE(Write, "rvRtpSessionRtpSendPacket: Encryption Error");
                /*return RvUint32Const(0);  Abort send */
            }
            else
            {
               //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
               if(s->remoteAddressSet && p->singlePeer && s->socklen>0) {

                  while((res=RvUdpSend(s->socket,
                     rvDataBufferGetData(sendBufferPtr),
                     (RvSize_t)(p->len=rvDataBufferGetLength(sendBufferPtr)),
                     0, (RvUint8*)s->sockdata, s->socklen)==-1) && errno==EINTR);

                  if(res>=0) bytesSent=res;

               } else {
#endif
                  //SPIRENT_END

                res = RvSocketSendBuffer(&s->socket,
                    rvDataBufferGetData(sendBufferPtr),
                    (RvSize_t)(p->len=rvDataBufferGetLength(sendBufferPtr)),
                    destAddressPtr,
                    logMngr,
                    &bytesSent);

                //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
               }
#endif
                  //SPIRENT_END

                if ((res == RV_OK) && (bytesSent > 0))
                {
                    RTPLOG_DEBUG((RTP_SOURCE, "SrtpProfileRfc3711Write(): packet was sent"));
                }
                else
                {
                    RTPLOG_ERROR1(Write, "RvSocketSendBuffer() failed");
                }
            }
        }
        destAddressPtr = RvRtpAddressListGetNext(&s->addressList, destAddressPtr);
    }
    if (res != RV_OK || !bDestAddressAvailable)
    {
        RTPLOG_ERROR_LEAVE(Write, "unsuccessful sending or no destination to send.");
        return -1;
    }
    RTPLOG_LEAVE(Write);
    return res;
}




/************************************************************************************
 * srtpRemoveRemoteAddress
 * (Private function)
 * description: This function removes a destination for the master local
 *              source from the session.
 *              Translators that need to remove destinations for other
 *              sources should use the rvSrtpForwardDestinationRemove function. (Future)
 * input: plugin       - pointer to this plugin
 *        hRTP         - Handle of the RTP session.
 *        address      - pointer to destination address
 * output: (-)
 * return value:  If no error occurs, the function returns the non-neagtive value.
 *                Otherwise, it returns a negative value.
 ***********************************************************************************/
RvInt32  SrtpProfileRfc3711RemoveRemoteAddress(
    IN struct __RtpProfilePlugin* plugin,
    IN RvRtpSession    session,
    IN RvNetAddress*   address)
{
    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;
    RvSrtpEncryptionPlugIn *plugInPtr = NULL;
    RvSrtp*                 srtpPtr    = NULL;
    RvStatus          result  = RV_OK;
    RvNetAddressType  type;
    RvChar            addrString[32];
    RvInt16           port    = 0;
    if (s == NULL || plugin==NULL || address == NULL)
    {
        RTPLOG_ERROR_LEAVE(Write, "SRTP is not constructed and inited or NULL session pointer.");
        return RV_ERROR_BADPARAM;
    }
    srtpPtr = (RvSrtp*)&(((RtpProfileRfc3711*)plugin->userData)->srtp);
    plugInPtr = &srtpPtr->plugIn;

    type = RvNetGetAddressType(address);
    if (type == RVNET_ADDRESS_NONE)
        return RV_ERROR_UNKNOWN;

    memset(addrString, 0, sizeof(addrString));
    if (type == RVNET_ADDRESS_IPV4)
    {
        RvNetIpv4 ipv4;
        RvNetGetIpv4(&ipv4 ,address);
        RvAddressIpv4ToString(addrString, sizeof(addrString), ipv4.ip);
        port = ipv4.port;
    }
    else
    {
#if (RV_NET_TYPE & RV_NET_IPV6)
        RvNetIpv6 ipv6;
        RvNetGetIpv6(&ipv6 ,address);
        RvAddressIpv6ToString(addrString, sizeof(addrString), ipv6.ip);
        port = ipv6.port;
#else
        return RV_ERROR_NOTSUPPORTED;
#endif
    }
    result = rvSrtpDestinationRemove(srtpPtr , addrString, port);
    if (result == RV_RTPSTATUS_Succeeded)
    {
        return RV_OK;
    }

    return RV_ERROR_UNKNOWN;
}

/************************************************************************************
 * SrtpProfileRfc3711AddRemoteAddress
 * description: This function adds destination for the session.
 * input: plugin       - pointer to this plugin
 *        hRTP         - Handle of the RTP session.
 *        destType     - RTP TRUE, RTCP FALSE
 *        address      - pointer to destination address
 * output: (-)
 * return value:  If no error occurs, the function returns the non-neagtive value.
 *                Otherwise, it returns a negative value.
 ***********************************************************************************/
RvInt32  SrtpProfileRfc3711AddRemoteAddress(
    IN struct __RtpProfilePlugin* plugin,
    IN RvRtpSession               session,
    IN RvBool                     destType, /* TRUE for RTP, FALSE for RTCP */
    IN RvNetAddress*              address)
{
    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;
    RvSrtpEncryptionPlugIn *plugInPtr = NULL;
    RvSrtp*                 srtpPtr    = NULL;
    RvNetAddressType  type;
    RvChar            addrString[32];
    RvInt16           port    = 0;
    RvStatus          result  = RV_OK;

    if (s==NULL||plugin==NULL)
    {
        RTPLOG_ERROR_LEAVE(Write, "SRTP is not constructed and inited or NULL session pointer.");
        return RV_ERROR_BADPARAM;
    }
    srtpPtr = (RvSrtp*)&(((RtpProfileRfc3711*)plugin->userData)->srtp);
    plugInPtr = &srtpPtr->plugIn;

    type = RvNetGetAddressType(address);
    if (type == RVNET_ADDRESS_NONE)
        return RV_ERROR_UNKNOWN;
    if (type == RVNET_ADDRESS_IPV4)
    {
        RvNetIpv4 ipv4;
        RvNetGetIpv4(&ipv4 ,address);
        RvAddressIpv4ToString(addrString, sizeof(addrString), ipv4.ip);
        port = ipv4.port;
    }
    else
    {
#if (RV_NET_TYPE & RV_NET_IPV6)
        RvNetIpv6 ipv6;
        RvNetGetIpv6(&ipv6 ,address);
        RvAddressIpv6ToString(addrString, sizeof(addrString), ipv6.ip);
        port = ipv6.port;
#else
        return RV_ERROR_NOTSUPPORTED;
#endif
    }
    if (destType)
        result = rvSrtpDestinationAddRtp(srtpPtr, addrString, port);
    else
        result = rvSrtpDestinationAddRtcp(srtpPtr, addrString, port);

    if (result == RV_RTPSTATUS_Succeeded)
    {
        return RV_OK;
    }

    return RV_ERROR_UNKNOWN;
}

/************************************************************************************
 * SrtpProfileRfc3711Release
 * description: this callback is called, when plugin have to release data related to the
 *              plugin when session is closing.
 * input: plugin           - pointer to this plugin
 *        hRTP             - Handle of the RTP session.
 * output: none.
 * return value:  If no error occurs, the function returns the non-neagtive value.
 *                Otherwise, it returns a negative value.
 ***********************************************************************************/
void  SrtpProfileRfc3711Release(
        struct __RtpProfilePlugin* profilePlugin,
        IN    RvRtpSession hRTP)
{
    RtpProfileRfc3711*      rfc3711Plugin = (RtpProfileRfc3711*) profilePlugin->userData;
    if (hRTP!=NULL)
        RvSrtpDestruct(hRTP);

    RtpProfilePluginDestruct(profilePlugin);
    RvMemoryFree(rfc3711Plugin, logMngr);
    RV_UNUSED_ARG(hRTP);
}


/***************************************************************************************
 * RtpProfileRfc3711Construct
 * description:  This method constructs a RtpProfilePlugin. All of
 *               the callbacks must be suppled for this plugin to work.
 * parameters:
 *    plugin - the RtpProfileRfc3711 object.
 *   userData - the user data associated with the object.
 * returns: A pointer to the object, if successful. NULL, otherwise.
 ***************************************************************************************/
RtpProfileRfc3711* RtpProfileRfc3711Construct(
     RtpProfileRfc3711* plugin
#ifdef UPDATED_BY_SPIRENT
    , RvSrtpMasterKeyEventCB masterKeyEventCB
#endif // UPDATED_BY_SPIRENT
)
{
     rvSrtpConstruct(&plugin->srtp
#ifdef UPDATED_BY_SPIRENT
					, masterKeyEventCB
#endif // UPDATED_BY_SPIRENT
	 );
     RtpProfilePluginConstruct(&plugin->plugin,
         plugin,
         &SrtpProfileRfc3711Callbacks);

     return plugin;
}

#ifdef __cplusplus
}
#endif

