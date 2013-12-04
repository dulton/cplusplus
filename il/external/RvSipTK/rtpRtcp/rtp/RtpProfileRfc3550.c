/************************************************************************
 File Name	   : RtpProfileRfc3550.c
 Description   : scope: Private
                 implementation of RtpProfilePlugin for RTP (RFC 3550)
*************************************************************************/
/***********************************************************************
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
#include "rtputil.h"
#include "RtpProfileRfc3550.h"
#include "RtcpProfileRfc3550.h"
#include "rvrtplogger.h"
#include "rtcp.h" /* for RvRtcpSessionSetParam */

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
#include <errno.h>
#endif
//SPIRENT_END

#define RTP_DESTINATION_UNREACHEBLE  (RvSocketErrorCode(RV_ERROR_UNKNOWN))

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
#undef FUNC_NAME
#define FUNC_NAME(name) "RtpProfileRfc3550" #name


#ifdef __cplusplus
extern "C" {
#endif

extern RvRtpInstance rvRtpInstance;
static RtpProfilePluginCallbacks RtpProfileRfc3550Callbacks =
{
        RtpProfileRfc3550Read,
        RtpProfileRfc3550Write,
        RtpProfileRfc3550RtcpRawReceive,
        RtpProfileRfc3550RtcpRawSend,
        /*RtpProfilePluginRemoveRemoteAddressCB*/      NULL,
        /*RtpProfilePluginRemoveRemoteAddressesAllCB*/ NULL,
        RtpProfileRfc3550Release
};
/************************************************************************************
* RtpProfileRfc3550Read
* description: This routine sets the header of the RTP message.
* input: 
*        plugin - pointer to this plugin
*        hRTP  - Handle of the RTP session.
*        buf   - Pointer to buffer containing the RTP packet with room before first
*                payload byte for RTP header.
*        len   - Length in bytes of buf.
*
* output: p            - A struct of RTP param,contain the fields of RTP header.
*        remAddressPtr - pointer to the remote address
* return value: If no error occurs, the function returns the non-negative value.
*                Otherwise, it returns a negative value.
***********************************************************************************/
RvInt32  RtpProfileRfc3550Read(
        struct __RtpProfilePlugin* plugin,
		IN  RvRtpSession  hRTP,
		IN  void *buf,
		IN  RvInt32 len,
		OUT RvRtpParam* p,
		OUT RvNetAddress* remAddressPtr)
{
    RvRtpSessionInfo* s = (RvRtpSessionInfo *)hRTP;
    RvAddress remoteAddress;
    RvSize_t messageSize;
    RvStatus res;
    RvUint8* bufferPtr = (RvUint8*) buf;

    RV_UNUSED_ARG(plugin);
    
    RTPLOG_ENTER(Read);
    res = RvSocketReceiveBuffer(&s->socket, (RvUint8*)buf, (RvSize_t)len, logMgr, &messageSize, &remoteAddress);

    if (res == RTP_DESTINATION_UNREACHEBLE)
    {
       /* error of sending from socket to an unreachable destination.
           Since used the same socket for sending and receiving, if sending is failed,
           here we receive the sending error. In this case logging will be bombed by error messages.
           This is the design defect, but it allow to use 1 socket per RTP session instead of two.*/ 
        /* RTPLOG_ERROR_LEAVE(Read, "RvSocketReceiveBuffer(). The sending destination is unreachable."); 
           it is intentionally omitted not to produce printings loops */
        return res; 
    }
    if (res != RV_OK)
    {
        RTPLOG_ERROR_LEAVE(Read, "RvSocketReceiveBuffer() Read failed failed.");  
        return res;
    }
    p->len = (RvInt32)messageSize;
	
    if (p->len <= RvRtpGetHeaderLength())
	{
		RvAddressDestruct(&remoteAddress);
        RTPLOG_ERROR_LEAVE(Read, "This packet is probably corrupted or session is closed in blocking mode.");		
        return RV_ERROR_UNKNOWN;
	}
	if (remAddressPtr!=NULL)
	{
		if (RvAddressGetType(&remoteAddress) == RV_ADDRESS_TYPE_IPV6)
        {
#if (RV_NET_TYPE & RV_NET_IPV6)
            RvNetIpv6 Ipv6; 
            memcpy(Ipv6.ip, RvAddressGetIpv6(&remoteAddress)->ip, sizeof(Ipv6.ip));
            Ipv6.port = RvAddressGetIpv6(&remoteAddress)->port;
            Ipv6.scopeId = RvAddressGetIpv6(&remoteAddress)->scopeId;
            //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
            Ipv6.flowinfo = RvAddressGetIpv6(&remoteAddress)->flowinfo;
#endif
            //SPIRENT_END
            RvNetCreateIpv6(remAddressPtr, &Ipv6);			
#else
            RTPLOG_ERROR_LEAVE(Read, "IPV6 is not supported in current configuration. remAddressPtr was not filled.");
            return RV_ERROR_NOTSUPPORTED;
#endif
        }
        else  
        {
            RvNetIpv4 Ipv4; 
            if (RvAddressGetIpv4(&remoteAddress)!=NULL)
            {        
                Ipv4.ip = RvAddressGetIpv4(&remoteAddress)->ip;
                Ipv4.port = RvAddressGetIpv4(&remoteAddress)->port;
                RvNetCreateIpv4(remAddressPtr, &Ipv4);
            }
		}
	}
	if(s->encryptionPlugInPtr != NULL)
    {
        RvRtpEncryptionData encryptionData;
        RvKey               decryptionKey;
		
        RTPLOG_DEBUG((RTP_SOURCE,"RtpProfileRfc3550Read: Decrypting"));
        RvKeyConstruct(&decryptionKey);
        RvRtpEncryptionDataConstruct(&encryptionData);
		
        /* Setup the Encryption data */
        RvRtpEncryptionDataSetIsRtp(&encryptionData, RV_TRUE);
        RvRtpEncryptionDataSetIsEncrypting(&encryptionData, RV_FALSE);
        RvRtpEncryptionDataSetMode(&encryptionData, s->encryptionMode);
        RvRtpEncryptionDataSetLocalAddress(&encryptionData, NULL);
        RvRtpEncryptionDataSetRemoteAddress(&encryptionData, (RvNetAddress*)&remoteAddress);
		
        if(RvRtpEncryptionModeIsPlainTextRtpHeader(s->encryptionMode))
        {
            /* Extract the RTP header from the packet */
			/* !!!           rvRtpHeaderUnserialize(headerPtr, bufferPtr);*/
			ConvertFromNetwork(buf, 0, 3);
			
			/* If the SSRC is ours and the is our IP, we should ignore this packet - it probably came
			in as a multicast packet we sent out. */
			if ( (s->sSrc == ((RvUint32*)buf)[2]) && isMyIP(&remoteAddress) )
			{
				RvAddressDestruct(&remoteAddress);
				RvKeyDestruct(&decryptionKey);
				RvRtpEncryptionDataDestruct(&encryptionData);
                RTPLOG_ERROR_LEAVE(Read, "failed.");						
				return RV_ERROR_UNKNOWN;
			}
			res = RvRtpUnpack(hRTP, buf, len, p);
			
            RvRtpEncryptionDataSetRtpHeader(&encryptionData, p);
			bufferPtr += RvRtpGetHeaderLength();
			messageSize -= RvRtpGetHeaderLength();

        }
        else
        {
            RvRtpEncryptionDataSetRtpHeader(&encryptionData, NULL);
        }
		
        if(s->encryptionKeyPlugInPtr != NULL)
        {
            RvRtpEncryptionKeyPlugInGetKey(s->encryptionKeyPlugInPtr, &encryptionData, &decryptionKey);
        }
		
        /* Decrypt the data */
        RvRtpEncryptionPlugInDecrypt(s->encryptionPlugInPtr,
			bufferPtr,
			bufferPtr,
			/*rvDataBufferGetLength(bufferPtr)*/messageSize,
			&encryptionData, &decryptionKey);
		
        if(!RvRtpEncryptionModeIsPlainTextRtpHeader(s->encryptionMode))
        {
            /* Extract the RTP header from the packet */
			/* !!!           rvRtpHeaderUnserialize(headerPtr, bufferPtr);*/
			ConvertFromNetwork(buf, 0, 3);
			
			/* If the SSRC is ours and the is our IP, we should ignore this packet - it probably came
			in as a multicast packet we sent out. */
			if ( (s->sSrc == ((RvUint32*)buf)[2]) && isMyIP(&remoteAddress) )
			{
				RvAddressDestruct(&remoteAddress);				
				RvKeyDestruct(&decryptionKey);
				RvRtpEncryptionDataDestruct(&encryptionData);
                RTPLOG_ERROR_LEAVE(Read, "failed.");	
				return RV_ERROR_UNKNOWN;
			}
            /* Extract the RTP header from the packet */
            /*rvRtpHeaderUnserialize(headerPtr, bufferPtr);*/
			res = RvRtpUnpack(hRTP, buf, len, p);
        }
		
		RvKeyDestruct(&decryptionKey);
        RvRtpEncryptionDataDestruct(&encryptionData);
    }
    else
    {
        RTPLOG_DEBUG((RTP_SOURCE,"RtpProfileRfc3550Read: no need for decrypting"));
	    RvAddressDestruct(&remoteAddress);	
		ConvertFromNetwork(buf, 0, 3);
		
		/* If the SSRC is ours and the is our IP, we should ignore this packet - it probably came
		in as a multicast packet we sent out. */
		if ( (s->sSrc == ((RvUint32*)buf)[2]) && isMyIP(&remoteAddress) )
        {
            RTPLOG_ERROR_LEAVE(Read, "failed.");	
            return RV_ERROR_UNKNOWN;
        }
        /* Extract the RTP header from the packet */
		res = RvRtpUnpack(hRTP, buf, len, p);
    }
    RTPLOG_LEAVE(Read);
    return res;
}

/************************************************************************************
 * RtpProfileRfc3550Write
 * description: this function is called, when plugin have to send the XRTP packet.
 * input: plugin - pointer to this plugin
 *        hRTP   - Handle of the RTP session.
 *        buf    - Pointer to buffer containing the RTP packet with room before first
 *                payload byte for RTP header.
 *        len    - Length in bytes of buf.
 *        p      - A struct of RTP param.
 * output: none.
 * return value:  If no error occurs, the function returns the non-neagtive value.
 *                Otherwise, it returns a negative value.
 * IMPORTANT NOTE: in case of encryption according to RFC 3550 (9. Security) 
 *   p.len must contain the actual length of buffer, which must be
 *   more then len, because of encryption padding
 ***********************************************************************************/
#define RTP_WORK_BUFFER_SIZE (1600)
RvInt32  RtpProfileRfc3550Write(
        struct __RtpProfilePlugin* plugin,
        IN  RvRtpSession           hRTP,
        IN  void *                 buf,
        IN  RvInt32                len,
        IN  RvRtpParam*            p)
{
    RvStatus res;
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
/* the next variables are added  for encription */
	RvUint8* bufferPtr = (RvUint8*) buf;
	RvUint8 workBuffer[RTP_WORK_BUFFER_SIZE];
	RvRtpEncryptionData     encryptionData;
	RvKey                   encryptionKey;
	RvUint32				headerSize = 0;
    RvInt8                  padding    = 0;
	RvUint32                dataSize   = len-p->sByte;
    RvInt32                 UserAllocationSize = p->len; /* if encription is used interface feature */
	RvAddress                *destAddress = NULL;
    RvBool                  bDestAddressAvailable = RV_FALSE;
	
    RTPLOG_ENTER(Write); 
	if (NULL == s || NULL == plugin)
    {
        RTPLOG_ERROR_LEAVE(Write, "NULL session handle or session is not opened");        
        return RV_ERROR_NULLPTR;
    }

   //SPIRENT_BEGIN
#if !defined(UPDATED_BY_SPIRENT)
	if (rtcpSessionIsSessionInShutdown(s->hRTCP))
	{
        RTPLOG_ERROR_LEAVE(Write, "Can not send. The session is in shutdown or RTCP BYE report was sent");
		return RV_ERROR_NOTSUPPORTED;
	}
#endif
   //SPIRENT_END

    p->paddingBit = RV_FALSE; /* for security purpose only */
    
	/* Get the number of padding bytes required  */
	/* Note: Perform this after adding CSRCs to header so RvRtpGetHeaderLength */
	/* gives the correct value  */
	if(s->encryptionPlugInPtr != NULL)
	{
        RTPLOG_DEBUG((RTP_SOURCE, "RvRtpWrite: encrypting the RTP packet"));
		RvKeyConstruct(&encryptionKey);
		RvRtpEncryptionDataConstruct(&encryptionData);
		/* Setup the Encryption data */
		RvRtpEncryptionDataSetIsRtp(&encryptionData, RV_TRUE);
		RvRtpEncryptionDataSetIsEncrypting(&encryptionData, RV_TRUE);
		RvRtpEncryptionDataSetMode(&encryptionData, s->encryptionMode);
		RvRtpEncryptionDataSetLocalAddress(&encryptionData, (RvNetAddress*) &rvRtpInstance.rvLocalAddress);
		if(RvRtpEncryptionModeIsPlainTextRtpHeader(s->encryptionMode))
			RvRtpEncryptionDataSetRtpHeader(&encryptionData, p);
		else
			RvRtpEncryptionDataSetRtpHeader(&encryptionData, NULL);
		
		headerSize = RvRtpGetHeaderLength() + p->extensionBit*sizeof(RvUint32)*(p->extensionLength+1); 
                                             /* IN the FUTURE: RvRtpGetHeaderLength p 
												must be adjusted to number of csrcs */
		
		if(!RvRtpEncryptionModeIsNoPadding(s->encryptionMode))
        {
            RvUint8 blockSize;
            
            blockSize = (RvUint8) RvRtpEncryptionPlugInGetBlockSize(s->encryptionPlugInPtr);
            
            if(RvRtpEncryptionModeIsPlainTextRtpHeader(s->encryptionMode))
                padding = (RvUint8) (blockSize - (dataSize % blockSize));
            else
                padding = (RvUint8) (blockSize - ((dataSize + headerSize) % blockSize));
            
            if(padding == blockSize)
                padding = 0;
            RTPLOG_DEBUG((RTP_SOURCE, "RvRtpWrite: padding = %d, blockSize = %d",	padding, blockSize));
        }
	}
    p->paddingBit = (padding != 0) ? RV_TRUE: RV_FALSE;
    
    RTPLOG_DEBUG((RTP_SOURCE, "RvRtpWrite: calling RvRtpPack()"));
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(p->packFunction) {
      res=(*(p->packFunction))((void*)hRTP, buf, len, p, p->pfarg);
    } else {
      res = RvRtpPack(hRTP, buf, len, p);
    }
#else
    res = RvRtpPack(hRTP, buf, len, p);
#endif
//SPIRENT_END
	/* Encrypt the data */
	if(s->encryptionPlugInPtr != NULL)
	{
		/* Add any required padding */
		if(padding > 0)
		{
			if(UserAllocationSize-len >= padding)
			{
				
				/* Add the padding to the buffer */
				len += (padding - 1);
				
				/* Write the amount of padding as the last byte */
				bufferPtr[len] = padding;
                RTPLOG_DEBUG((RTP_SOURCE,  "RvRtpWrite: setting padding in last byte of packet"));
			}
			else
            {
                p->len = len + padding; /* requred length */
                RvRtpEncryptionDataDestruct(&encryptionData);
                RvKeyDestruct(&encryptionKey);	
                RTPLOG_ERROR_LEAVE(Write, "Not enough room at end of RTP buffer to add required padding.");
                return -1;
            }
		}
		
		/* Make sure the work buffer is large enough for the current packet */
		if(sizeof(workBuffer) < (RvUint32)len)
        {            
            RvRtpEncryptionDataDestruct(&encryptionData);
            RvKeyDestruct(&encryptionKey);
            RTPLOG_ERROR_LEAVE(Write, "Not enough room for working buffer.");            
            return -2;
        }	
		/* Set up the work buffer data position */
/*		rvDataBufferSetDataPosition(&thisPtr->workBuffer,0,rvDataBufferGetLength(bufferPtr));*/
		
		/* Copy the unencrypted header to the work buffer*/
		if(RvRtpEncryptionModeIsPlainTextRtpHeader(s->encryptionMode))
			memcpy(workBuffer, bufferPtr, headerSize);
	}
    if (res >= 0)
    {	
       //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
      if(s->remoteAddressSet && p->singlePeer && s->encryptionPlugInPtr == NULL && s->socklen>0) {

         destAddress = &s->remoteAddress;
		   bDestAddressAvailable = RV_TRUE;

			/* send UDP data through the specified socket to the remote host.*/
			while((res=RvUdpSend(s->socket, (RvUint8*)buf+p->sByte, (RvSize_t)p->len, 0, (RvUint8*)s->sockdata, s->socklen)==-1) && errno==EINTR);

         RTPLOG_DEBUG((RTP_SOURCE, "RvRtpWrite: packet was sent"));

      } else {
#endif
       //SPIRENT_END
		/* Send packet to all remote addresses */
		destAddress = RvRtpAddressListGetNext(&s->addressList, NULL);
		bDestAddressAvailable = (destAddress!=NULL);
		while(destAddress != NULL) 
		{
			if(s->encryptionPlugInPtr != NULL)
			{	
				/* Set the remote address in the encryption data */
				RvRtpEncryptionDataSetRemoteAddress(&encryptionData, (RvNetAddress*)destAddress);
				
				if(s->encryptionKeyPlugInPtr != NULL)
				{
					RvRtpEncryptionKeyPlugInGetKey(s->encryptionKeyPlugInPtr, &encryptionData, &encryptionKey);
				}
				
				/* Encrypt the buffer */
				if(RvRtpEncryptionModeIsPlainTextRtpHeader(s->encryptionMode))
				{
					/* Encrypt the data */
					RvRtpEncryptionPlugInEncrypt(s->encryptionPlugInPtr,
						bufferPtr + headerSize, /* without header */
						workBuffer + headerSize,
						dataSize + padding,
						&encryptionData, &encryptionKey);
				}
				else
				{
					RvRtpEncryptionPlugInEncrypt(s->encryptionPlugInPtr,
						bufferPtr, /* with header */
						workBuffer,
						dataSize + headerSize + padding,
						&encryptionData, &encryptionKey);
				}
				/* send UDP data through the specified socket to the remote host.*/
				res = RvSocketSendBuffer(&s->socket, (RvUint8*)workBuffer+p->sByte, (RvSize_t)p->len + padding, destAddress, logMgr, NULL);
         }
			else
				/* send UDP data through the specified socket to the remote host.*/
				res = RvSocketSendBuffer(&s->socket, (RvUint8*)buf+p->sByte, (RvSize_t)p->len, destAddress, logMgr, NULL);
         RTPLOG_DEBUG((RTP_SOURCE, "RvRtpWrite: packet was sent"));
         destAddress = RvRtpAddressListGetNext(&s->addressList, destAddress);
		}
      //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
      }
#endif
       //SPIRENT_END
    }
    if(s->encryptionPlugInPtr != NULL)
    {
        RvRtpEncryptionDataDestruct(&encryptionData);
        RvKeyDestruct(&encryptionKey);
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
 * RtpProfileRfc3550Release
 * description: this callback is called, when plugin have to release data related to the
 *              plugin when session is closing.
 * input: plugin           - pointer to this plugin 
 *        hRTP             - Handle of the RTP session.
 * output: none.
 * return value:  If no error occurs, the function returns the non-neagtive value.
 *                Otherwise, it returns a negative value.
 ***********************************************************************************/
void  RtpProfileRfc3550Release(
        struct __RtpProfilePlugin* profilePlugin,                                   
        IN    RvRtpSession hRTP)
{
    RtpProfileRfc3550*      rfc3550Plugin = (RtpProfileRfc3550*) profilePlugin->userData;
    
    RtpProfileRfc3550Destruct(rfc3550Plugin);
    RV_UNUSED_ARG(hRTP);
}
/***************************************************************************************
 * RtpProfileRfc3550Construct
 * description:  This method constructs a RtpProfilePlugin. All of
 *               the callbacks must be suppled for this plugin to work.
 * parameters:
 *    plugin - the RvRtpEncryptionPlugIn object.
 *   userData - the user data associated with the object.
 * returns: A pointer to the object, if successful. NULL, otherwise.
 ***************************************************************************************/
RtpProfileRfc3550* RtpProfileRfc3550Construct(
     IN RtpProfileRfc3550* rtpPlugin)
{
    RtpProfilePluginConstruct(
        &rtpPlugin->plugin,
        rtpPlugin,
        &RtpProfileRfc3550Callbacks);
   return rtpPlugin;
}

/***************************************************************************************
 * RtpProfilePluginDestruct
 * description:  This method destructs a RtpProfilePlugin. 
 * parameters:
 *    plugin - the RvRtpEncryptionPlugIn object.
 * returns: none
 ***************************************************************************************/
void RtpProfileRfc3550Destruct(
     IN RtpProfileRfc3550*  plugin)
{
   RtpProfilePluginDestruct(&plugin->plugin);
}

#ifdef __cplusplus
}
#endif
    

