/************************************************************************
 File Name	   : rvsrtp.h
 Description   :
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
#if !defined(RVSRTPENCRYPTIONPLUGIN_H)
#define RVSRTPENCRYPTIONPLUGIN_H


#include "rvtypes.h"
#include "rtp.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    

/************* Encryption PlugIn Interface **************/

/*$
{callback:
	{name:	  RvRtpEncryptionPlugInRegisterCB}
	{class:   RvRtpEncryptionPlugIn}
	{include: rvrtpsession.h}
	{description:
		{p: This callback is called when the plugin is being registered
			with a session. This allows the plugin to do any operations
			that are required, which need to be thread safe.}
	}
	{proto: void RvRtpEncryptionPlugInRegisterCB(RvRtpSession *thisPtr, void *encryptionPluginData, RvBool enableRtp, RvBool enableRtcp, RvUint32 ssrc, RvUint16 rtpSeqNum, RvUint32 rtpRoc, RvUint32 rtcpIndex);}
	{params:
		{param: {n:thisPtr}    {d:A pointer to the session.}}
		{param: {n:encryptionPluginData} {d:Special user defined data for this callback.}} 
		{param: {n:enableRtp}  {d:RV_TRUE if this session uses RTP, otherwise RV_FALSE.}}
		{param: {n:enableRtcp} {d:RV_TRUE if this session uses RTCP, otherwise RV_FALSE.}}
		{param: {n:ssrc}       {d:Current value of the master local source SSRC.}}
		{param: {n:rtpSeqNum}  {d:Sequence number of first local RTP packet that will be sent.}}
		{param: {n:rtpRoc}     {d:Roll over counter of first local RTP packet that will be sent.}}
		{param: {n:rtcpIndex}  {d:Index number of first local RTCP packet that will be sent.}}
	}
	{returns: RV_RTPSTATUS_Succeeded if successfull, otherwise an error.}
}
$*/
typedef RvRtpStatus (*RvSrtpEncryptionPlugInRegisterCB)(
     RvRtpSession  *thisPtr, 
     void          *encryptionPluginData, 
     RvBool        enableRtp, 
     RvBool        enableRtcp, 
     RvUint32      ssrc, 
     RvUint16      rtpSeqNum, 
     RvUint32      rtpRoc, 
     RvUint32      rtcpIndex);

/*$
{callback:
	{name:	  RvRtpEncryptionPlugInUnregisterCB}
	{class:   RvRtpEncryptionPlugIn}
	{include: rvrtpsession.h}
	{description:
		{p: This callback is called when the plugin is being
			unregistered from a session. This allows the plugin to do any
			operations that are required, which need to be thread safe.}
	}
	{proto: void RvRtpEncryptionPlugInUnregisterCB(RvRtpSession *thisPtr, void *encryptionPluginData);}
	{params:
		{param: {n:thisPtr}    {d:A pointer to the session.}}
		{param: {n:encryptionPluginData} {d:Special user defined data for this callback.}} 
	}
	{returns: RV_RTPSTATUS_Succeeded if successfull, otherwise an error.}
}
$*/
typedef RvRtpStatus (*RvSrtpEncryptionPlugInUnregisterCB)( RvRtpSession *thisPtr, void *encryptionPluginData);

/*$
{callback:
	{name:	  RvRtpEncryptionPlugInOpenCB}
	{class:   RvRtpEncryptionPlugIn}
	{include: rvrtpsession.h}
	{description:
		{p: This callback is called when the RTP session is being opened.}
	}
	{proto: void RvRtpEncryptionPlugInOpenCB(RvRtpSession *thisPtr, void *encryptionPluginData);}
	{params:
		{param: {n:thisPtr}    {d:A pointer to the session.}}
		{param: {n:encryptionPluginData} {d:Special user defined data for this callback.}} 
	}
	{returns: RV_RTPSTATUS_Succeeded if successfull, otherwise an error.}
}
$*/
typedef RvRtpStatus (*RvSrtpEncryptionPlugInOpenCB)(RvRtpSession *thisPtr, void *encryptionPluginData);

/*$
{callback:
	{name:	  RvRtpEncryptionPlugInCloseCB}
	{class:   RvRtpEncryptionPlugIn}
	{include: rvrtpsession.h}
	{description:
		{p: This callback is called when the RTP session is being closed.}
	}
	{proto: RvRtpStatus RvRtpEncryptionPlugInCloseCB(RvRtpSession *thisPtr, void *encryptionPluginData);}
	{params:
		{param: {n:thisPtr}    {d:A pointer to the session.}}
		{param: {n:encryptionPluginData} {d:Special user defined data for this callback.}} 
	}
	{returns: RV_RTPSTATUS_Succeeded if successfull, otherwise an error.}
}
$*/
typedef RvRtpStatus (*RvSrtpEncryptionPlugInCloseCB)(RvRtpSession *thisPtr, void *encryptionPluginData);

/*$
{callback:
	{name:	  RvRtpEncryptionPlugInEncryptRtpCB}
	{class:   RvRtpEncryptionPlugIn}
	{include: rvrtpsession.h}
	{description:
		{p: This callback is called when the session is needs to
			encrypt an outgoing RTP packet.}
		{p: The data buffer containing the packet must have the
			required amout of header and footer space (in "Front/Back
			Capacity"). The "Back Capacity" must also have enough space
			to add any padding that might be necessary due to the
			minumum block size (worst case is block size minus 1).}
		{p: The data buffer where the encrypted packet is to be stored
			will have a length of zero. The "Front Capacity" of the
			data buffer must be large enough for the entire resulting
			packet (source packet data plus its required "Front and Back
			Capacity").}
	}
	{proto: RvRtpStatus RvRtpEncryptionPlugInEncryptRtpCB(RvRtpSession *thisPtr, void *encryptionPluginData, RvDataBuffer *srcBuffer, RvDataBuffer *dstBuffer, RvAddress *srcAddress, RvAddress *dstAddress, RvUint32 roc);}
	{params:
		{param: {n:thisPtr}    {d:A pointer to the session.}}
		{param: {n:encryptionPluginData} {d:Special user defined data for this callback.}} 
		{param: {n:srcBuffer}  {d:The data buffer containing the packet to encrypt.}}
		{param: {n:dstBuffer}  {d:The data buffer where the encrypted packet will be stored.}}
		{param: {n:srcAddress} {d:Address packet is to be transmitted from.}}
		{param: {n:dstAddress} {d:Address packet is to be transmitted to.}}
		{param: {n:roc} {d:Current value of RTP rollover counter.}}
	}
	{returns: RV_RTPSTATUS_Succeeded if successfull, otherwise an error.}
}
$*/
typedef RvRtpStatus (*RvSrtpEncryptionPlugInEncryptRtpCB)(RvRtpSession thisPtr, void *encryptionPluginData, RvDataBuffer *srcBuffer, RvDataBuffer *dstBuffer, RvAddress *srcAddress, RvAddress *dstAddress, RvUint32 roc);

/*$
{callback:
	{name:	  RvRtpEncryptionPlugInEncryptRtcpCB}
	{class:   RvRtpEncryptionPlugIn}
	{include: rvrtpsession.h}
	{description:
		{p: This callback is called when the session is needs to
			encrypt an outgoing RTCP packet.}
		{p: The data buffer containing the packet must have the
			required amout of header and footer space (in "Front/Back
			Capacity"). The "Back Capacity" must also have enough space
			to add any padding that might be necessary due to the
			minumum block size (worst case is block size minus 1).}
		{p: The data buffer where the encrypted packet is to be stored
			will have a length of zero. The "Front Capacity" of the
			data buffer must be large enough for the entire resulting
			packet (source packet data plus its required Front and Back
			Capacity).}
	}
	{proto: RvRtpStatus RvRtpEncryptionPlugInEncryptRtcpCB(RvRtpSession *thisPtr, void *encryptionPluginData, RvDataBuffer *srcBuffer, RvDataBuffer *dstBuffer, RvAddress *srcAddress, RvAddress *dstAddress, RvUint32 index, RvBool encrypt);}
	{params:
		{param: {n:thisPtr}    {d:A pointer to the session.}}
		{param: {n:encryptionPluginData} {d:Special user defined data for this callback.}} 
		{param: {n:srcBuffer}  {d:The data buffer containing the packet to encrypt.}}
		{param: {n:dstBuffer}  {d:The data buffer where the encrypted packet will be stored.}}
		{param: {n:srcAddress} {d:Address packet is to be transmitted from.}}
		{param: {n:dstAddress} {d:Address packet is to be transmitted to.}}
		{param: {n:index} {d:Current value of RTCP index counter.}}
		{param: {n:encrypt} {d:RV_TRUE = encyrpt packet, RV_FALSE = use NULL encyrption.}}
	}
	{returns: RV_RTPSTATUS_Succeeded if successfull, otherwise an error.}
}
$*/
typedef RvRtpStatus (*RvSrtpEncryptionPlugInEncryptRtcpCB)(
    IN    RvRtpSession  thisPtr, 
    IN    void*         encryptionPluginData, 
    IN    RvDataBuffer* srcBuffer, 
    INOUT RvDataBuffer* dstBuffer, 
    IN    RvAddress*    srcAddress,
    IN    RvAddress*    dstAddress, 
    IN    RvUint32      index, 
    IN    RvBool        encrypt);

/*$
{callback:
	{name:	  RvRtpEncryptionPlugInDecryptRtpCB}
	{class:   RvRtpEncryptionPlugIn}
	{include: rvrtpsession.h}
	{description:
		{p: This callback is called when the session is needs to
			decrypt an incoming RTP packet.}
		{p: The data buffer where the decrypted packet is to be stored
			will have a length of zero. The "Front Capacity" of the
			data buffer must be large enough for the entire resulting
			packet (source packet data plus its required Front and Back
			Capacity).}
		{p: Note that after decryption, the decrypted packet buffer
			will usually end up with the header and footer space as unused
			extra "Capacity" in the buffer, but is required to exist so
			that the decryption does not require any intermediate
			buffers. This is usually not an issue since it simply makes
			all buffers the same size, regardless of encryption,
			decryption, source, or destination.}
	}
	{proto: RvRtpStatus RvRtpEncryptionPlugInDecryptRtpCB(RvRtpSession *thisPtr, void *encryptionPluginData, RvDataBuffer *srcBuffer, RvDataBuffer *dstBuffer, RvAddress *srcAddress, RvAddress *dstAddress);}
	{params:
		{param: {n:thisPtr}    {d:A pointer to the session.}}
		{param: {n:encryptionPluginData} {d:Special user defined data for this callback.}} 
		{param: {n:srcBuffer}  {d:The data buffer containing the packet to decrypt.}}
		{param: {n:dstBuffer}  {d:The data buffer where the unencrypted packet will be stored.}}
		{param: {n:srcAddress} {d:Address packet was received from.}}
		{param: {n:dstAddress} {d:Address packet was sent to.}}
	}
	{returns: RV_RTPSTATUS_Succeeded if successfull, otherwise an error.}
}
$*/
typedef RvRtpStatus (*RvSrtpEncryptionPlugInDecryptRtpCB)(RvRtpSession thisPtr, 
        void *encryptionPluginData, 
        RvDataBuffer *srcBuffer, 
        RvDataBuffer *dstBuffer, 
        RvAddress *srcAddress, 
        RvAddress *dstAddress);

/*$
{callback:
	{name:	  RvRtpEncryptionPlugInDecryptRtcpCB}
	{class:   RvRtpEncryptionPlugIn}
	{include: rvrtpsession.h}
	{description:
		{p: This callback is called when the session is needs to
			decrypt an incoming RTCP packet.}
		{p: The data buffer where the decrypted packet is to be stored
			will have a length of zero. The "Front Capacity" of the
			data buffer must be large enough for the entire resulting
			packet (source packet data plus its required Front and Back
			Capacity).}
		{p: Note that after decryption, the decrypted packet buffer
			will usually end up with the header and footer space as unused
			extra "Capacity" in the buffer, but is required to exist so
			that the decryption does not require any intermediate
			buffers. This is usually not an issue since it simply makes
			all buffers the same size, regardless of encryption,
			decryption, source, or destination.}
	}
	{proto: RvRtpStatus RvRtpEncryptionPlugInDecryptRtcpCB(RvRtpSession *thisPtr, void *encryptionPluginData, RvDataBuffer *srcBuffer, RvDataBuffer *dstBuffer, RvAddress *srcAddress, RvAddress *dstAddress);}
	{params:
		{param: {n:thisPtr}    {d:A pointer to the session.}}
		{param: {n:encryptionPluginData} {d:Special user defined data for this callback.}} 
		{param: {n:srcBuffer}  {d:The data buffer containing the packet to decrypt.}}
		{param: {n:dstBuffer}  {d:The data buffer where the unencrypted packet will be stored.}}
		{param: {n:srcAddress} {d:Address packet was received from.}}
		{param: {n:dstAddress} {d:Address packet was sent to.}}
	}
	{returns: RV_RTPSTATUS_Succeeded if successfull, otherwise an error.}
}
$*/
typedef RvRtpStatus (*RvSrtpEncryptionPlugInDecryptRtcpCB)(RvRtpSession thisPtr, void *encryptionPluginData, RvDataBuffer *srcBuffer, RvDataBuffer *dstBuffer, RvAddress *srcAddress, RvAddress *dstAddress);

/*$
{callback:
	{name:	  RvRtpEncryptionPlugInValidateSsrcCB}
	{class:   RvRtpEncryptionPlugIn}
	{include: rvrtpsession.h}
	{description:
		{p: This callback is called when a new SSRC has been generated
			in order to confirm that this new ssrc does not cause any
			encryption issues.}
	}
	{proto: RvBool RvRtpEncryptionPlugInValidateSsrcCB(RvRtpSession *thisPtr, void *encryptionPluginData, RvUint32 ssrc);}
	{params:
		{param: {n:thisPtr}    {d:A pointer to the session.}}
		{param: {n:encryptionPluginData} {d:Special user defined data for this callback.}} 
		{param: {n:ssrc} {d:The ssrc to validate.}}
	}
	{returns: RV_TRUE if SSRC is OK, RV_FALSE if it is not.}
}
$*/
typedef RvBool (*RvSrtpEncryptionPlugInValidateSsrcCB)(RvRtpSession *thisPtr, void *encryptionPluginData, RvUint32 ssrc);

/*$
{callback:
	{name:	  RvRtpEncryptionPlugInSsrcChangedCB}
	{class:   RvRtpEncryptionPlugIn}
	{include: rvrtpsession.h}
	{description:
		{p: This callback is called when the SSRC is being changed
			(after it has been validated). Cleanup of destinations for
			the old ssrc should be done and any setup for those
			destination to use the new ssrc should be done.}
	}
	{proto: RvRtpStatus RvRtpEncryptionPlugInSsrcChangedCB(RvRtpSession *thisPtr, void *encryptionPluginData, RvUint32 oldSsrc, RvUint32 newSsrc);}
	{params:
		{param: {n:thisPtr}    {d:A pointer to the session.}}
		{param: {n:encryptionPluginData} {d:Special user defined data for this callback.}} 
		{param: {n:oldSsrc} {d:The old ssrc that will no longer be used.}}
		{param: {n:newSsrc} {d:The new ssrc that will be used.}}
	}
	{returns: RV_RTPSTATUS_Succeeded if successful, otherwise an error.}
}
$*/
typedef RvRtpStatus (*RvSrtpEncryptionPlugInSsrcChangedCB)(RvRtpSession *thisPtr, void *encryptionPluginData, RvUint32 oldSsrc, RvUint32 newSsrc);

/*$
{callback:
	{name:	  RvRtpEncryptionPlugInGetPaddingSizeCB}	
	{class:   RvRtpEncryptionPlugIn}
	{include: rvrtpsession.h}
	{description:
		{p: This callback is called when the RTP/RTCP session needs to 
			find out what the worst case padding size that will be
			added to a packet by the encryption plugin. The padding
			size is in bytes. If the encryption algorithm is  a stream
			algorithm this function should return a value of 0.}
	}
	{proto: RvUint32 RvRtpEncryptionPlugInGetPaddingSizeCB(RvRtpSession *thisPtr, void *encryptionPluginData, RvBool forRtp);}
	{params:
		{param: {n:thisPtr}    {d:A pointer to the session.}}
		{param: {n:encryptionPluginData} {d:Special user defined data for this callback.}} 
		{param: {n:forRtp}     {d:RV_TRUE = Return padding size for RTP, RV_FALSE = for RTCP.}}
	}
	{returns: RvUint32, The maximum padding size in bytes. }
}
$*/
typedef RvUint32 (*RvSrtpEncryptionPlugInGetPaddingSizeCB)(RvRtpSession *thisPtr, void *encryptionPluginData, RvBool forRtp);

/*$
{callback:
	{name:	  RvRtpEncryptionPlugInGetHeaderSizeCB} 
	{class:   RvRtpEncryptionPlugIn}
	{include: rvrtpsession.h}
	{description:
		{p: This callback is called when the RTP/RTCP session needs to 
			find out what additional header space is required by the
			encryption plugin.}
	}
	{proto: RvUint32 RvRtpEncryptionPlugInGetHeaderSizeCB(RvRtpSession *thisPtr, void *encryptionPluginData, RvBool forRtp);}
	{params:
		{param: {n:thisPtr}    {d:A pointer to the session.}}
		{param: {n:encryptionPluginData} {d:Special user defined data for this callback.}} 
		{param: {n:forRtp}     {d:RV_TRUE = Return header size for RTP, RV_FALSE = for RTCP.}}
	}
	{returns: RvUint32, The required header size in bytes. }
}
$*/
typedef RvUint32 (*RvSrtpEncryptionPlugInGetHeaderSizeCB)(RvRtpSession *thisPtr, void *encryptionPluginData, RvBool forRtp); 

/*$
{callback:
	{name:	  RvRtpEncryptionPlugInGetFooterSizeCB} 
	{class:   RvRtpEncryptionPlugIn}
	{include: rvrtpsession.h}
	{description:
		{p: This callback is called when the RTP/RTCP session needs to 
			find out what additional footer space is required by the
			encryption plugin.}
	}
	{proto: RvUint32 RvRtpEncryptionPlugInGetFooterSizeCB(RvRtpSession *thisPtr, void *encryptionPluginData, RvBool forRtp);}
	{params:
		{param: {n:thisPtr}    {d:A pointer to the session.}}
		{param: {n:encryptionPluginData} {d:Special user defined data for this callback.}} 
		{param: {n:forRtp}     {d:RV_TRUE = Return header size for RTP, RV_FALSE = for RTCP.}}
	}
	{returns: RvUint32, The required footer size in bytes. }
}
$*/
typedef RvUint32 (*RvSrtpEncryptionPlugInGetFooterSizeCB)(RvRtpSession *thisPtr, void *encryptionPluginData, RvBool forRtp);

/*$
{callback:
	{name:	  RvRtpEncryptionPlugInGetTransmitSizeCB} 
	{class:   RvRtpEncryptionPlugIn}
	{include: rvrtpsession.h}
	{description:
		{p: This callback is called when the RTP/RTCP session needs to 
			find out the actual packet size that will be transmitted
			for an unencrypted packet of a specified size. This is used
			when the size of the final packet needs to be known before
			the packet is encrypted (or perhaps even before it is
			created).}
	}
	{proto: RvUint32 RvRtpEncryptionPlugInGetTransmitSizeCB(RvRtpSession *thisPtr, void *encryptionPluginData, RvUint32 packetSize, RvUint32 rtpHeaderSize, RvBool encrypt, RvBool forRtp);}
	{params:
		{param: {n:thisPtr}    {d:A pointer to the session.}}
		{param: {n:encryptionPluginData} {d:Special user defined data for this callback.}}
		{param: {n:packetSize} {d:Size of unencrypted packet.}}
		{param: {n:rtpHeaderSize} {d:RTP only: size of the RTP header in the unencrypted packet.}}
		{param: {n:encrypt}    {d:RTCP only: RV_TRUE = encyrpt packet, RV_FALSE = use NULL encyrption.}}
		{param: {n:forRtp}     {d:RV_TRUE = Return transmit size for RTP, RV_FALSE = for RTCP.}}
	}
	{returns: The actual size of the packet that would be transmitted. }
}
$*/
typedef RvUint32 (*RvSrtpEncryptionPlugInGetTransmitSizeCB)(RvRtpSession *thisPtr, void *encryptionPluginData, RvUint32 packetSize, RvUint32 rtpHeaderSize, RvBool encrypt, RvBool forRtp);

/*$
{callback:
	{name:	  RvRtpEncryptionPlugInRtpSeqNumChangedCB} 
	{class:   RvRtpEncryptionPlugIn}
	{include: rvrtpsession.h}
	{description:
		{p: This callback is called when the RTP sequence number has
			been updated. The sequence number (and roc) passed is the
			sequence number that will be used for the next RTP packet that
			is to be sent from the local master source.}
		{p: The roll over counter (ROC) is a count of the number of
			times the sequence number has wrapped back to 0.}
		{p: Note that forwarded RTP packets do not update the sequence
			number since they must provide their own sequence number and roc.}
	}
	{proto: void RvRtpEncryptionPlugInRtpSeqNumChangedCB(RvRtpSession *thisPtr, void *encryptionPluginData, RvUint16 sequenceNum, RvUint32 roc);}
	{params:
		{param: {n:thisPtr}    {d:A pointer to the session.}}
		{param: {n:encryptionPluginData} {d:Special user defined data for this callback.}}
		{param: {n:sequenceNum} {d:The RTP sequence number.}}
		{param: {n:roc}         {d:The RTP roll over counter.}}
	}
}
$*/
typedef void (*RvSrtpEncryptionPlugInRtpSeqNumChangedCB)(RvRtpSession *thisPtr, void *encryptionPluginData, RvUint16 sequenceNum, RvUint32 roc);

/*$
{callback:
	{name:	  RvRtpEncryptionPlugInRtcpIndexChangedCB} 
	{class:   RvRtpEncryptionPlugIn}
	{include: rvrtpsession.h}
	{description:
		{p: This callback is called when the RTCP index number has been
			updated. The index number passed is the index number of the
			next RTCP packet that will be sent.}
		{p: Note that forwarded RTCP packets do not update the index
			since they must provide their own index number.}
	}
	{proto: void RvRtpEncryptionPlugInRtcpIndexChangedCB(RvRtpSession *thisPtr, void *encryptionPluginData, RvUint32 index);}
	{params:
		{param: {n:thisPtr} {d:A pointer to the session.}}
		{param: {n:encryptionPluginData} {d:Special user defined data for this callback.}}
		{param: {n:roc}     {d:The RTCP index.}}
	}
}
$*/
typedef void (*RvSrtpEncryptionPlugInRtcpIndexChangedCB)(RvRtcpSession *thisPtr, void *encryptionPluginData, RvUint32 index);

/*$
{type:
	{name: RvRtpEncryptionPlugIn}
	{superpackage: RvRtp}
	{include: rvrtpsession.h}
	{description:	
		{p: This interface is used to allow the RTP Session to encrypt/decrypt
			data. To use this interface the user MUST implement all of the 
            callbacks and assign them to the appropriate elemements of
            this structure.}
        {p: User defined data may be assigned to the userData element
			of the structure. This value will be passed to each callback
			function when it is called.}
		{p: It is important to note than the RTP/RTCP session will be
			locked during all of these callbacks, so a plugin needs to
			behave appropriately.}
		{p: Using one of the prebuilt plugins (RvRtpDesEncryption,
			RvRtp3DesEncryption, or optional RvSrtp) does not require the
			application to use this plugin interface directly, the plugins
			do everything automatcially. Refer to the documentation for the
			plugins themselves for more information.}
	}
	{attributes:
		{attribute: {t: RvRtpEncryptionPlugInRegisterCB} {n: registerPlugin}
					{d: Called when plugin is registered with a session.}}
		{attribute: {t: RvRtpEncryptionPlugInUnregisterCB} {n: unregisterPlugin}
					{d: Called when plugin is unregistered from a session.}}
		{attribute: {t: RvRtpEncryptionPlugInOpenCB} {n: sessionOpen}
					{d: Called when the session is being opened.}}
		{attribute: {t: RvRtpEncryptionPlugInCloseCB} {n: sessionClose}
					{d: Called when the session is being closed.}}
		{attribute: {t: RvRtpEncryptionPlugInEncryptRtpCB} {n: encryptRtp}
					{d: Called to encrypt an RTP packet.}}
		{attribute: {t: RvRtpEncryptionPlugInEncryptRtcpCB} {n: encryptRtcp}
					{d: Called to encrypt an RTCP packet.}}
		{attribute: {t: RvRtpEncryptionPlugInDecryptRtpCB} {n: decryptRtp}
					{d: Called to decrypt an RTP packet.}}
		{attribute: {t: RvRtpEncryptionPlugInDecryptRtcpCB} {n: decryptRtcp}
					{d: Called to decrypt an RTCP packet.}}
		{attribute: {t: RvRtpEncryptionPlugInValidateSsrcCB} {n: validateSsrc}
					{d: Called to validate a new ssrc when it is generated.}}
		{attribute: {t: RvRtpEncryptionPlugInSsrcChangedCB} {n: ssrcChanged}
					{d: Called when an encryption source has changed to a new ssrc.}}
		{attribute: {t: RvRtpEncryptionPlugInGetPaddingSizeCB} {n: getPaddingSize}
					{d: Called to request the worst case padding size that encryption will add to a packet.}}
		{attribute: {t: RvRtpEncryptionPlugInGetHeaderSizeCB} {n: getHeaderSize}
					{d: Called to request the header size requirement needed by the encryption.}}
		{attribute: {t: RvRtpEncryptionPlugInGetFooterSizeCB} {n: getFooterSize}
					{d: Called to request the footer size requirement needed by the encryption.}}
		{attribute: {t: RvRtpEncryptionPlugInGetTransmitSizeCB} {n: getTransmitSize}
					{d: Called to request the actual transmission size of a packet.}}
		{attribute: {t: RvRtpEncryptionPlugInRtpSeqNumChangedCB} {n: rtpSeqNumChanged}
					{d: Called when the RTP sequence number has changed.}}
		{attribute: {t: RvRtpEncryptionPlugInRtcpIndexChangedCB} {n: rtcpIndexChanged}
					{d: Called when the RTCP index number has changed.}}
		{attribute: {t: void *} {n: userData}
					{d: A user defined pointer.}}
	}
	{see_also:
		{n:RvRtpDesEncryption}
		{n:RvSrtp}
	}
}
$*/
typedef struct {
	RvSrtpEncryptionPlugInRegisterCB          registerPlugin;
	RvSrtpEncryptionPlugInUnregisterCB        unregisterPlugin;
	RvSrtpEncryptionPlugInOpenCB              sessionOpen;
	RvSrtpEncryptionPlugInCloseCB             sessionClose;
	RvSrtpEncryptionPlugInEncryptRtpCB        encryptRtp;
	RvSrtpEncryptionPlugInEncryptRtcpCB       encryptRtcp;
	RvSrtpEncryptionPlugInDecryptRtpCB        decryptRtp;
	RvSrtpEncryptionPlugInDecryptRtcpCB       decryptRtcp;
	RvSrtpEncryptionPlugInValidateSsrcCB      validateSsrc;
	RvSrtpEncryptionPlugInSsrcChangedCB       ssrcChanged;
	RvSrtpEncryptionPlugInGetPaddingSizeCB    getPaddingSize;
	RvSrtpEncryptionPlugInGetHeaderSizeCB     getHeaderSize;
	RvSrtpEncryptionPlugInGetFooterSizeCB     getFooterSize;
	RvSrtpEncryptionPlugInGetTransmitSizeCB   getTransmitSize;
	RvSrtpEncryptionPlugInRtpSeqNumChangedCB  rtpSeqNumChanged;
	RvSrtpEncryptionPlugInRtcpIndexChangedCB  rtcpIndexChanged;
	void                                      *userData;

} RvSrtpEncryptionPlugIn;


#ifdef __cplusplus
}
#endif

#endif
