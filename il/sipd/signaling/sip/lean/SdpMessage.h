/// @file
/// @brief SDP message header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _SDP_MESSAGE_H_
#define _SDP_MESSAGE_H_

#include <stdint.h>
#include <string>

#include "SipCommon.h"

DECL_SIPLEAN_NS

///////////////////////////////////////////////////////////////////////////////

struct SdpMessage
{
    // Enumerated protocol constants
    enum AddressType
    {
        SDP_ADDR_TYPE_UNKNOWN = 0,
        SDP_ADDR_TYPE_IPV4,
        SDP_ADDR_TYPE_IPV6
    };

    enum MediaFormat
    {
        SDP_MEDIA_FORMAT_UNKNOWN = 0,
        SDP_MEDIA_FORMAT_PCMU,
        SDP_MEDIA_FORMAT_G723,
        SDP_MEDIA_FORMAT_G729,
        SDP_MEDIA_FORMAT_H263,
        SDP_MEDIA_FORMAT_H264,
        SDP_MEDIA_FORMAT_MP4V_ES,
        SDP_MEDIA_FORMAT_PCMA,
        SDP_MEDIA_FORMAT_G726_32,
        SDP_MEDIA_FORMAT_PCMU_WB,
        SDP_MEDIA_FORMAT_PCMA_WB
    };

    // Default ctor
    SdpMessage(void) 
    {
        Reset();
    }

    // SDP message methods
    void Reset(void)
    {
        sessionId = 0;
        timestamp = 0;
        addrType = SDP_ADDR_TYPE_UNKNOWN;
        addrStr.clear();
        audioMediaFormat = SDP_MEDIA_FORMAT_UNKNOWN;
        audioTransportPort = 0;
        videoMediaFormat = SDP_MEDIA_FORMAT_UNKNOWN;
        videoTransportPort = 0;
        audioRtpDynamicPayloadType  = 0xFF;
    }
    
    // Session description
    uint32_t sessionId;
    uint32_t timestamp;
    AddressType addrType;
    std::string addrStr;

    // Audio media description
    MediaFormat audioMediaFormat;
    uint16_t audioTransportPort;

    // Video media description
    MediaFormat videoMediaFormat;
    uint16_t videoTransportPort;

    // RTP dynamic payload type
    uint8_t audioRtpDynamicPayloadType;
};
    
///////////////////////////////////////////////////////////////////////////////

END_DECL_SIPLEAN_NS

#endif
