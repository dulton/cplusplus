/// @file
/// @brief SIP User Agent Config header
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _USER_AGENT_CONFIG_H_
#define _USER_AGENT_CONFIG_H_

#include <vector>

#include <Tr1Adapter.h>

#include "VoIPCommon.h"

// Forward declarations (global)
DECL_CLASS_FORWARD_MEDIA_NS(VoIPMediaManager);
#ifdef UNIT_TEST
class TestUserAgentBlock;
class TestStatefulSip;
class TestSipDynProxy;
#endif

DECL_APP_NS

///////////////////////////////////////////////////////////////////////////////

class UserAgentConfig
{
  public:
    typedef Sip_1_port_server::SipUaConfig_t config_t;
    typedef std::vector<uint32_t> streamIndexVec_t;
    
    UserAgentConfig(uint16_t port, const config_t& config, MEDIA_NS::VoIPMediaManager *voipMediaManager = 0);

    config_t& Config(void) { return *mConfig; }
    const config_t& Config(void) const { return *mConfig; }

    enum StreamIndexType
    {
        AUDIO_RTP_STREAM_INDEXES,
        AUDIO_RTCP_STREAM_INDEXES,
        VIDEO_RTP_STREAM_INDEXES,
        VIDEO_RTCP_STREAM_INDEXES
    };
    
    const streamIndexVec_t& GetStreamIndexes(StreamIndexType which) const;

  private:
    std::tr1::shared_ptr<streamIndexVec_t> MakeStreamIndexVec(MEDIA_NS::VoIPMediaManager *voipMediaManager, uint16_t port, uint32_t streamBlock);
    
    // Class data
    static const uint16_t DEFAULT_AUDIO_PORT_NUMBER;
    static const uint16_t DEFAULT_VIDEO_PORT_NUMBER;
    static const uint32_t INVALID_STREAM_BLOCK_INDEX;

    std::tr1::shared_ptr<config_t> mConfig;
    std::tr1::shared_ptr<streamIndexVec_t> mAudioRtpStreamIndexes;
    std::tr1::shared_ptr<streamIndexVec_t> mAudioRtcpStreamIndexes;
    std::tr1::shared_ptr<streamIndexVec_t> mVideoRtpStreamIndexes;
    std::tr1::shared_ptr<streamIndexVec_t> mVideoRtcpStreamIndexes;

#ifdef UNIT_TEST
    friend class ::TestUserAgentBlock;
    friend class ::TestStatefulSip;
    friend class ::TestSipDynProxy;
#endif
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_APP_NS

#endif
