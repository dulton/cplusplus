/************************************************************************
 File Name	   : srtcpProfileRfc3711.h
 Description   : scope: Private
     declaration of inplementation of RtpProfilePlugin for SRTCP (RFC 3711)
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

#ifndef __SRTCP_PROFILE_RFC_3711_H__
#define __SRTCP_PROFILE_RFC_3711_H__

#include "srtpProfileRfc3711.h"

#ifdef __cplusplus
extern "C" {
#endif

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
        INOUT RvUint32*     sentLenPtr);

/************************************************************************************
 * SrtcpProfileRfc3711RawReceive
 * description: This routine receives the RTCP compound packet filled buffer.
 *              It dencrypts the buffer according to SRTCP rules,
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
     struct __RtpProfilePlugin* plugin,
     IN    RvRtcpSession hRTCP,
     INOUT RvRtpBuffer* buf,
     IN    RvSize_t bytesToReceive,
     OUT   RvSize_t*   bytesReceivedPtr,
     OUT   RvAddress*  remoteAddressPtr);

#ifdef __cplusplus
}
#endif
    
#endif /* __SRTCP_PROFILE_RFC_3711_H__ */
