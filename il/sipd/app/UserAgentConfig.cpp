/// @file
/// @brief SIP User Agent Config implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <sstream>
#include <utility>

#include <base/BaseCommon.h>
#include <phxexception/PHXException.h>

#include "VoIPMediaManager.h"
#include "UserAgentConfig.h"

USING_APP_NS;
USING_MEDIA_NS;

///////////////////////////////////////////////////////////////////////////////

const uint16_t UserAgentConfig::DEFAULT_AUDIO_PORT_NUMBER = 50050;
const uint16_t UserAgentConfig::DEFAULT_VIDEO_PORT_NUMBER = 50052;
const uint32_t UserAgentConfig::INVALID_STREAM_BLOCK_INDEX = static_cast<const uint32_t>(-1);

///////////////////////////////////////////////////////////////////////////////

UserAgentConfig::UserAgentConfig(const uint16_t port, const config_t& config, 
				 VoIPMediaManager *voipMediaManager)
    : mConfig(new config_t(config))
{
    // Make sure endpoint pattern is PAIR
    if (mConfig->ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern != L4L7_BASE_NS::EndpointConnectionPatternOptions_PAIR)
    {
        std::ostringstream oss;
        oss << "[UserAgentConfig ctor] EndpointConnectionPattern was configured as "
            << tms_enum_to_string(mConfig->ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern, L4L7_BASE_NS::em_EndpointConnectionPatternOptions)
            << ", only PAIR is supported";
        const std::string err(oss.str());
        TC_LOG_ERR_LOCAL(port, LOG_SIP, err);
        throw EPHXBadConfig(err);
    }
    
    // Make sure port numbers are non-zero
    if (!mConfig->ProtocolConfig.LocalPort)
    {
        mConfig->ProtocolConfig.LocalPort = DEFAULT_SIP_PORT_NUMBER;
        TC_LOG_INFO_LOCAL(port, LOG_SIP, "[UserAgentConfig ctor] LocalPort was configured as zero, reset to " << mConfig->ProtocolConfig.LocalPort);
    }
    
    if (!mConfig->Endpoint.DstPort)
    {
        mConfig->Endpoint.DstPort = DEFAULT_SIP_PORT_NUMBER;
        TC_LOG_INFO_LOCAL(port, LOG_SIP, "[UserAgentConfig ctor] DstPort was configured as zero, reset to " << mConfig->Endpoint.DstPort);
    }

    if (!mConfig->ProtocolProfile.AudioSrcPort)
    {
        mConfig->ProtocolProfile.AudioSrcPort = DEFAULT_AUDIO_PORT_NUMBER;
        TC_LOG_INFO_LOCAL(port, LOG_SIP, "[UserAgentConfig ctor] AudioSrcPort was configured as zero, reset to " << mConfig->ProtocolProfile.AudioSrcPort);
    }

    if (!mConfig->ProtocolProfile.VideoSrcPort)
    {
        mConfig->ProtocolProfile.VideoSrcPort = DEFAULT_VIDEO_PORT_NUMBER;
        TC_LOG_INFO_LOCAL(port, LOG_SIP, "[UserAgentConfig ctor] VideoSrcPort was configured as zero, reset to " << mConfig->ProtocolProfile.VideoSrcPort);
    }
    
    // Make sure UA numbers per device is non-zero
    mConfig->ProtocolConfig.UaNumsPerDevice = std::max(mConfig->ProtocolConfig.UaNumsPerDevice, static_cast<uint32_t>(1));

    if (mConfig->ProtocolProfile.CallType == sip_1_port_server::EnumSipCallType_AUDIO_ONLY || 
	mConfig->ProtocolProfile.CallType == sip_1_port_server::EnumSipCallType_AUDIO_VIDEO)
    {
        mAudioRtpStreamIndexes = MakeStreamIndexVec(voipMediaManager, port, mConfig->AudioRtpStreamBlock);
        mAudioRtcpStreamIndexes = MakeStreamIndexVec(voipMediaManager, port, mConfig->AudioRtcpStreamBlock);
    }
    else
    {
        mAudioRtpStreamIndexes = std::tr1::shared_ptr<streamIndexVec_t>(new streamIndexVec_t);
        mAudioRtcpStreamIndexes = mAudioRtpStreamIndexes;
    }

    if (mConfig->ProtocolProfile.CallType == sip_1_port_server::EnumSipCallType_AUDIO_VIDEO)
    {
        mVideoRtpStreamIndexes = MakeStreamIndexVec(voipMediaManager, port, mConfig->VideoRtpStreamBlock);
        mVideoRtcpStreamIndexes = MakeStreamIndexVec(voipMediaManager, port, mConfig->VideoRtcpStreamBlock);
    }
    else
    {
        mVideoRtpStreamIndexes = std::tr1::shared_ptr<streamIndexVec_t>(new streamIndexVec_t);
        mVideoRtcpStreamIndexes = mVideoRtpStreamIndexes;
    }
}

///////////////////////////////////////////////////////////////////////////////

const UserAgentConfig::streamIndexVec_t& UserAgentConfig::GetStreamIndexes(StreamIndexType which) const
{
    switch (which)
    {
      case AUDIO_RTP_STREAM_INDEXES:
          return *mAudioRtpStreamIndexes;
          
      case AUDIO_RTCP_STREAM_INDEXES:
          return *mAudioRtcpStreamIndexes;
        
      case VIDEO_RTP_STREAM_INDEXES:
          return *mVideoRtpStreamIndexes;
        
      case VIDEO_RTCP_STREAM_INDEXES:
          return *mVideoRtcpStreamIndexes;

      default:
          throw EPHXInternal("UserAgentConfig::GetStreamIndexes");
    }
}

///////////////////////////////////////////////////////////////////////////////

std::tr1::shared_ptr<UserAgentConfig::streamIndexVec_t> UserAgentConfig::MakeStreamIndexVec(VoIPMediaManager *voipMediaManager, 
											    uint16_t port, 
											    uint32_t streamBlock)
{
    std::tr1::shared_ptr<streamIndexVec_t> streamIndexes(new streamIndexVec_t);
    
    if(mConfig && mConfig->ProtocolProfile.RtpTrafficType == sip_1_port_server::EnumRtpTrafficType_SIMULATED_RTP) {
      if (voipMediaManager && streamBlock != INVALID_STREAM_BLOCK_INDEX) {
        if (!voipMediaManager->GetFPGAStreamIndexes(port, streamBlock, *streamIndexes))
	  TC_LOG_ERR_LOCAL(port, LOG_SIP, "[UserAgentConfig::MakeStreamIndexVec] Unable to retrieve stream indexes for stream block " << streamBlock);
      }
    }

    return streamIndexes;
}

///////////////////////////////////////////////////////////////////////////////
