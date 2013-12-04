/// @file
/// @brief SIP User Agent Block object
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <algorithm>
#include <netinet/in.h>
#include <sstream>
#include <utility>

#include <base/LoadProfile.h>
#include <base/LoadScheduler.h>
#include <base/LoadStrategy.h>
#include <ildaemon/LocalInterfaceEnumerator.h>
#include <ildaemon/RemoteInterfaceEnumerator.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <phxexception/PHXException.h>

#include "VoIPUtils.h"

#include "AsyncCompletionHandler.h"
#include "SipUri.h"
#include "UserAgentBlock.h"
#include "UserAgentFactory.h"
#include "UserAgentBlockLoadStrategies.h"
#include "UserAgentNameEnumerator.h"

#include "VoIPMediaManager.h"
#include "MediaStreamInfo.h"
#include "MediaCodecs.h"
#include "MediaPayloads.h"
#include "SipUtils.h"

USING_MEDIAFILES_NS;
USING_MEDIA_NS;
USING_APP_NS;
USING_SIP_NS;
USING_VOIP_NS;

using namespace Sip_1_port_server;

///////////////////////////////////////////////////////////////////////////////

UserAgentBlock::UserAgentBlock(uint16_t port, uint16_t vdevblock, const config_t& config, ACE_Reactor& reactor, 
			       AsyncCompletionHandler *asyncCompletionHandler, 
			       UserAgentFactory &userAgentFactory)
    : mPort(port),
      mVDevBlock(vdevblock),
      mConfig(config),
      mReactor(&reactor),
      mAsyncCompletionHandler(asyncCompletionHandler),
      mEnabled(true),
      mStats(BllHandle()),
      mUsingProxy(!mConfig.Config().ProtocolProfile.UseUaToUaSignaling),
      mUsingMobile(mConfig.Config().MobileProfile.MobileSignalingChoice),
      mUsingIMSI(mConfig.Config().MobileProfile.MobileSignalingChoice == sip_1_port_server::EnumMoblielSignalingChoice_MOBILE_USIM),
      mRegStrategy(MakeRegistrationLoadStrategy()),
      mRegIndex(0),
      mUaNumsPerDevice(1),
      mDevicesTotal(0),
      mLoadProfile(new loadProfile_t(mConfig.Config().Load)),
      mLoadStrategy(MakeProtocolLoadStrategy()),
      mLoadScheduler(new loadScheduler_t(*mLoadProfile, *mLoadStrategy)),
      mAttemptedCallCount(0),
      mRegState(sip_1_port_server::EnumSipUaRegState_NOT_REGISTERED),
      mUserAgentFactory(userAgentFactory)
{
    if (mAsyncCompletionHandler) mCallCompleteAct = MakeAsyncCompletionToken(boost::bind(&UserAgentBlock::CallComplete, this));

    mLoadProfile->SetActiveStateChangeDelegate(boost::bind(&UserAgentBlock::NotifyLoadProfileRunning, this, _1));

    const SipUaProtocolConfig_t& protoConfig(mConfig.Config().ProtocolConfig);
    const SipUaProtocolProfile_t& protoProfile(mConfig.Config().ProtocolProfile);
    const SipUaDnsProfile_t& dnsProfile(mConfig.Config().DnsProfile);
    const SipUaDomainProfile_t domainProfile(mConfig.Config().DomainProfile);
    const SipUaEndpointConfig_t& endpointConfig(mConfig.Config().Endpoint);
    const SipUaMobileProfile_t& mobileConfig(mConfig.Config().MobileProfile);

    // Sanity check callee URI scheme
    SipUri::Scheme calleeUriScheme;
    std::string calleeUaNumFormat;

    switch (protoProfile.CalleeUriScheme)
    {
        case sip_1_port_server::EnumSipUriScheme_SIP:
            calleeUriScheme = SipUri::SIP_URI_SCHEME;
            calleeUaNumFormat = endpointConfig.DstUaNumFormat;
            break;

        case sip_1_port_server::EnumSipUriScheme_TEL:
            calleeUriScheme = SipUri::TEL_URI_SCHEME;
            if (protoProfile.LocalTelUriFormat)
            {
                calleeUaNumFormat = endpointConfig.DstUaNumFormat;
            }
            else
            {
                calleeUaNumFormat = "%u";
            }
            break;

        default:
        {
            std::ostringstream oss;
            oss << "[UserAgentBlock ctor] Invalid protocol profile, unknown callee URI scheme (" << protoProfile.CalleeUriScheme
                    << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
            throw EPHXBadConfig(err);
        }
    }

    // Create a LocalInterfaceEnumerator so that we can instantiate a UserAgent object for each emulated UA in the block
    IL_DAEMON_LIB_NS::LocalInterfaceEnumerator ifEnum(mPort, IfHandle());
    if (domainProfile.LocalContactSipDomainName.empty()) ifEnum.SetPortNum(
            protoConfig.LocalPort ? protoConfig.LocalPort : DEFAULT_SIP_PORT_NUMBER);
    else ifEnum.SetPortNum(domainProfile.LocalContactSipPort ? domainProfile.LocalContactSipPort : DEFAULT_SIP_PORT_NUMBER);

    // Create an appropriate destination enumerator
    {
        if (mUsingMobile && mUsingIMSI)
        {
            mDstImsiEnum.reset(
                    new UserAgentNameEnumerator(endpointConfig.DstUaImsiStart, endpointConfig.DstUaImsiStep,
                                                endpointConfig.DstUaImsiEnd, ""));
        }
        else mDstNameEnum.reset(
                new UserAgentNameEnumerator(endpointConfig.DstUaNumStart, endpointConfig.DstUaNumStep, endpointConfig.DstUaNumEnd,
                                            calleeUaNumFormat));

        if (mUsingProxy)
        {
            if (ifEnum.Type() == IFSETUP_NS::STC_DM_IFC_IPv4
                    && *(reinterpret_cast<const in_addr_t *>(protoProfile.ProxyIpv4Addr.address)) == INADDR_ANY)
            {
                if (!protoProfile.ProxyDomain.length() && !protoProfile.ProxyName.length())
                {
                    const std::string err("[UserAgentBlock ctor] Invalid protocol config, ProxyIpv4Addr is unspecified");
                    TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
                    throw EPHXBadConfig(err);
                }
            }
            else if (ifEnum.Type() == IFSETUP_NS::STC_DM_IFC_IPv6
                    && memcmp(protoProfile.ProxyIpv6Addr.address, &in6addr_any, sizeof(struct in6_addr)) == 0)
            {
                if (!protoProfile.ProxyDomain.length() && !protoProfile.ProxyName.length())
                {
                    const std::string err("[UserAgentBlock ctor] Invalid protocol config, ProxyIpv6Addr is unspecified");
                    TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
                    throw EPHXBadConfig(err);
                }
            }
        }
    }

    if (!mUsingProxy)
    {
        if (!endpointConfig.DstIf.ifDescriptors.empty())
        {
            mDstIfEnum.reset(new dstIfEnum_t(mPort, endpointConfig.DstIf));
            if (ifEnum.Type() != mDstIfEnum->Type())
#ifdef UNIT_TEST
            if(mDstIfEnum->Type() != IFSETUP_NS::STC_DM_IFC_IPv6)
#endif
            {
                std::ostringstream oss;
                oss << "[UserAgentBlock ctor] Invalid protocol config, source interface type ("
                        << tms_enum_to_string(ifEnum.Type(), IFSETUP_NS::em_InterfaceTypeList_t)
                        << ") mismatched with destination interface type ("
                        << tms_enum_to_string(mDstIfEnum->Type(), IFSETUP_NS::em_InterfaceTypeList_t) << ")";
                const std::string err(oss.str());
                TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
                throw EPHXBadConfig(err);
            }

            mDstIfEnum->SetPortNum(endpointConfig.DstPort ? endpointConfig.DstPort : DEFAULT_SIP_PORT_NUMBER);
        }
    }

    // If we're creating multiple UA's, make sure the UA number step is non-zero
    mUaNumsPerDevice = protoConfig.UaNumsPerDevice;
    mDevicesTotal = ifEnum.TotalCount();
    const size_t numUaObjects = mUaNumsPerDevice * mDevicesTotal;
    if (numUaObjects > 1 && !protoConfig.UaNumStep)
    {
        const std::string err("[UserAgentBlock ctor] Invalid protocol config, UaNumStep is zero");
        TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
        throw EPHXBadConfig(err);
    }

    const std::vector<uint32_t>& audioRtpStreamIndexes = mConfig.GetStreamIndexes(UserAgentConfig::AUDIO_RTP_STREAM_INDEXES);
    const std::vector<uint32_t>& audioRtcpStreamIndexes = mConfig.GetStreamIndexes(UserAgentConfig::AUDIO_RTCP_STREAM_INDEXES);
    const std::vector<uint32_t>& videoRtpStreamIndexes = mConfig.GetStreamIndexes(UserAgentConfig::VIDEO_RTP_STREAM_INDEXES);
    const std::vector<uint32_t>& videoRtcpStreamIndexes = mConfig.GetStreamIndexes(UserAgentConfig::VIDEO_RTCP_STREAM_INDEXES);

    if (protoProfile.RtpTrafficType == sip_1_port_server::EnumRtpTrafficType_SIMULATED_RTP)
    {

        if (protoProfile.CallType == sip_1_port_server::EnumSipCallType_AUDIO_ONLY
                || protoProfile.CallType == sip_1_port_server::EnumSipCallType_AUDIO_VIDEO)
        {

            if (audioRtpStreamIndexes.size() < numUaObjects) TC_LOG_WARN_LOCAL(
                    mPort,
                    LOG_SIP,
                    "[UserAgentBlock ctor] Audio RTP stream index list is shorter than expected (got "
                            << audioRtpStreamIndexes.size() << " indexes, expected " << numUaObjects << ")");

            if (audioRtcpStreamIndexes.size() < numUaObjects) TC_LOG_WARN_LOCAL(
                    mPort,
                    LOG_SIP,
                    "[UserAgentBlock ctor] Audio RTCP stream index list is shorter than expected (got "
                            << audioRtcpStreamIndexes.size() << " indexes, expected " << numUaObjects << ")");
        }

        if (protoProfile.CallType == sip_1_port_server::EnumSipCallType_AUDIO_VIDEO)
        {

            if (videoRtpStreamIndexes.size() < numUaObjects) TC_LOG_WARN_LOCAL(
                    mPort,
                    LOG_SIP,
                    "[UserAgentBlock ctor] Video RTP stream index list is shorter than expected (got "
                            << videoRtpStreamIndexes.size() << " indexes, expected " << numUaObjects << ")");

            if (videoRtcpStreamIndexes.size() < numUaObjects) TC_LOG_WARN_LOCAL(
                    mPort,
                    LOG_SIP,
                    "[UserAgentBlock ctor] Video RTCP stream index list is shorter than expected (got "
                            << videoRtcpStreamIndexes.size() << " indexes, expected " << numUaObjects << ")");
        }
    }

    EncodedMediaStreamIndex audioFileIndex = INVALID_STREAM_INDEX; // FPGA traffic or Signaling only
    EncodedMediaStreamIndex audioHDFileIndex = INVALID_STREAM_INDEX; // FPGA traffic or Signaling only
    EncodedMediaStreamIndex videoFileIndex = INVALID_STREAM_INDEX; // FPGA traffic or Signaling only

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    if (protoProfile.CallType == sip_1_port_server::EnumSipCallType_TELEPRESENCE)
    {

        MediaStreamInfo streamInfo;
        switch(protoProfile.AudioCodec)
        {
           case sip_1_port_server::EnumSipAudioCodec_AAC_LD:
              streamInfo.setBitRate(AAC_LD_BR);
              streamInfo.setCodecAndPayloadNumber(AAC_LD, protoProfile.AudioPayloadType);
              break;
           case sip_1_port_server::EnumSipAudioCodec_G_711_A_LAW:
              streamInfo.setCodecAndPayloadNumber(G711A, PAYLOAD_711A);
              break;
           case sip_1_port_server::EnumSipAudioCodec_G_711:
              streamInfo.setCodecAndPayloadNumber(G711u, PAYLOAD_711u);
              break;
           case sip_1_port_server::EnumSipAudioCodec_G_729:
              streamInfo.setCodecAndPayloadNumber(G729, PAYLOAD_729);
              break;
           case sip_1_port_server::EnumSipAudioCodec_G_729_A:
              streamInfo.setCodecAndPayloadNumber(G729A, PAYLOAD_729);
              break;
           case sip_1_port_server::EnumSipAudioCodec_G_729_B:
              streamInfo.setCodecAndPayloadNumber(G729B, PAYLOAD_729);
              break;
           case sip_1_port_server::EnumSipAudioCodec_G_729_AB:
              streamInfo.setCodecAndPayloadNumber(G729AB, PAYLOAD_729);
              break;
           default:
            {
              streamInfo.setCodecAndPayloadNumber(CODEC_UNDEFINED, PAYLOAD_UNDEFINED);
              std::ostringstream oss;
              oss << "[UserAgentBlock ctor] Encoded RTP traffic Type: Invalid TIP Audio Codec( " << protoProfile.AudioCodec << ")";
              const std::string err(oss.str());
              TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
              throw EPHXBadConfig(err);
            }
              break;
        }

        if (protoProfile.AudioCodec == sip_1_port_server::EnumSipAudioCodec_AAC_LD &&
            (audioHDFileIndex = userAgentFactory.getVoipMediaManager()->getStreamIndex(&streamInfo)) == INVALID_STREAM_INDEX) 
           audioHDFileIndex = userAgentFactory.getVoipMediaManager()->prepareEncodedStream(&streamInfo);
        else if((audioFileIndex = userAgentFactory.getVoipMediaManager()->getStreamIndex(&streamInfo)) == INVALID_STREAM_INDEX)
           audioFileIndex = userAgentFactory.getVoipMediaManager()->prepareEncodedStream(&streamInfo);


        if (audioHDFileIndex == INVALID_STREAM_INDEX && audioFileIndex == INVALID_STREAM_INDEX)
        {
            std::ostringstream oss;
            oss << "[UserAgentBlock ctor] Encoded RTP traffic Type: Cannot open Encoded Stream File ( "
                    << streamInfo.getFilePath() << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
            throw EPHXBadConfig(err);

        }

        ///////////////////////////////

        MediaStreamInfo streamInfoH264;
        switch(protoProfile.VideoCodec)
        {
           case sip_1_port_server::EnumSipVideoCodec_H_264_720p:
              streamInfoH264.setBitRate(VH264_BR);
              streamInfoH264.setCodecAndPayloadNumber(protoProfile.TipDeviceType <= sip_1_port_server::EnumTipDevice_CTS3200 ? VH264_CTS_720P : VH264_TBG_720P, mConfig.Config().ProtocolProfile.VideoPayloadType);
              break;
           case sip_1_port_server::EnumSipVideoCodec_H_264_1080p:
              streamInfoH264.setBitRate(VH264_BR);
              streamInfoH264.setCodecAndPayloadNumber(protoProfile.TipDeviceType <= sip_1_port_server::EnumTipDevice_CTS3200 ? VH264_CTS_1080P : VH264_TBG_1080P, mConfig.Config().ProtocolProfile.VideoPayloadType);
              break;
           case sip_1_port_server::EnumSipVideoCodec_H_264_512_288:
              streamInfoH264.setBitRate(VH264_BR);
              streamInfoH264.setCodecAndPayloadNumber(VH264_TBG_CIF, mConfig.Config().ProtocolProfile.VideoPayloadType);
              break;
           case sip_1_port_server::EnumSipVideoCodec_H_264_768_448:
              streamInfoH264.setBitRate(VH264_BR);
              streamInfoH264.setCodecAndPayloadNumber(VH264_TBG_XGA, mConfig.Config().ProtocolProfile.VideoPayloadType);
              break;
           default:
            {
              streamInfoH264.setCodecAndPayloadNumber(CODEC_UNDEFINED, PAYLOAD_UNDEFINED);
              std::ostringstream oss;
              oss << "[UserAgentBlock ctor] Encoded RTP traffic Type: Invalid TIP Video Codec( " << protoProfile.VideoCodec << ")";
              const std::string err(oss.str());
              TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
              throw EPHXBadConfig(err);               
            }
            break;
        }

        if ((videoFileIndex = userAgentFactory.getVoipMediaManager()->getStreamIndex(&streamInfoH264)) == INVALID_STREAM_INDEX) videoFileIndex =
                userAgentFactory.getVoipMediaManager()->prepareEncodedStream(&streamInfoH264);

        if (videoFileIndex == INVALID_STREAM_INDEX)
        {
            std::ostringstream oss;
            oss << "[UserAgentBlock ctor] Encoded RTP traffic Type: Cannot open Encoded Stream File ( "
                    << streamInfoH264.getFilePath() << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
            throw EPHXBadConfig(err);

        }

    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    if ((protoProfile.RtpTrafficType == sip_1_port_server::EnumRtpTrafficType_ENCODED_RTP)
            && (protoProfile.CallType != sip_1_port_server::EnumSipCallType_SIGNALING_ONLY)
            && (protoProfile.CallType != sip_1_port_server::EnumSipCallType_TELEPRESENCE))
    {
        MediaStreamInfo streamInfo;
        switch (protoProfile.AudioCodec)
        {

            case sip_1_port_server::EnumSipAudioCodec_AMRNB_0_OA_475: streamInfo.setBitRate(4750); streamInfo.setCodecAndPayloadNumber(AMRNB_0_OA_475, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_1_OA_515: streamInfo.setBitRate(5150); streamInfo.setCodecAndPayloadNumber(AMRNB_1_OA_515, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_2_OA_590: streamInfo.setBitRate(5900); streamInfo.setCodecAndPayloadNumber(AMRNB_2_OA_590, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_3_OA_670: streamInfo.setBitRate(6700); streamInfo.setCodecAndPayloadNumber(AMRNB_3_OA_670, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_4_OA_740: streamInfo.setBitRate(7400); streamInfo.setCodecAndPayloadNumber(AMRNB_4_OA_740, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_5_OA_795: streamInfo.setBitRate(7950); streamInfo.setCodecAndPayloadNumber(AMRNB_5_OA_795, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_6_OA_102: streamInfo.setBitRate(10200); streamInfo.setCodecAndPayloadNumber(AMRNB_6_OA_102, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_7_OA_122: streamInfo.setBitRate(12200); streamInfo.setCodecAndPayloadNumber(AMRNB_7_OA_122, protoProfile.AudioPayloadType); break;

            case sip_1_port_server::EnumSipAudioCodec_AMRNB_0_BE_475: streamInfo.setBitRate(4750); streamInfo.setCodecAndPayloadNumber(AMRNB_0_BE_475, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_1_BE_515: streamInfo.setBitRate(5150); streamInfo.setCodecAndPayloadNumber(AMRNB_1_BE_515, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_2_BE_590: streamInfo.setBitRate(5900); streamInfo.setCodecAndPayloadNumber(AMRNB_2_BE_590, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_3_BE_670: streamInfo.setBitRate(6700); streamInfo.setCodecAndPayloadNumber(AMRNB_3_BE_670, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_4_BE_740: streamInfo.setBitRate(7400); streamInfo.setCodecAndPayloadNumber(AMRNB_4_BE_740, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_5_BE_795: streamInfo.setBitRate(7950); streamInfo.setCodecAndPayloadNumber(AMRNB_5_BE_795, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_6_BE_102: streamInfo.setBitRate(10200); streamInfo.setCodecAndPayloadNumber(AMRNB_6_BE_102, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_7_BE_122: streamInfo.setBitRate(12200); streamInfo.setCodecAndPayloadNumber(AMRNB_7_BE_122, protoProfile.AudioPayloadType); break;

            case sip_1_port_server::EnumSipAudioCodec_AMRWB_0_OA_660 : streamInfo.setBitRate(6600); streamInfo.setCodecAndPayloadNumber(AMRWB_0_OA_660 , protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_1_OA_885 : streamInfo.setBitRate(8850); streamInfo.setCodecAndPayloadNumber(AMRWB_1_OA_885 , protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_2_OA_1265: streamInfo.setBitRate(12650); streamInfo.setCodecAndPayloadNumber(AMRWB_2_OA_1265, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_3_OA_1425: streamInfo.setBitRate(14250); streamInfo.setCodecAndPayloadNumber(AMRWB_3_OA_1425, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_4_OA_1585: streamInfo.setBitRate(15850); streamInfo.setCodecAndPayloadNumber(AMRWB_4_OA_1585, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_5_OA_1825: streamInfo.setBitRate(18250); streamInfo.setCodecAndPayloadNumber(AMRWB_5_OA_1825, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_6_OA_1985: streamInfo.setBitRate(19850); streamInfo.setCodecAndPayloadNumber(AMRWB_6_OA_1985, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_7_OA_2305: streamInfo.setBitRate(23050); streamInfo.setCodecAndPayloadNumber(AMRWB_7_OA_2305, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_8_OA_2385: streamInfo.setBitRate(23850); streamInfo.setCodecAndPayloadNumber(AMRWB_8_OA_2385, protoProfile.AudioPayloadType); break;

            case sip_1_port_server::EnumSipAudioCodec_AMRWB_0_BE_660 : streamInfo.setBitRate(6600); streamInfo.setCodecAndPayloadNumber(AMRWB_0_BE_660 , protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_1_BE_885 : streamInfo.setBitRate(8850); streamInfo.setCodecAndPayloadNumber(AMRWB_1_BE_885 , protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_2_BE_1265: streamInfo.setBitRate(12650); streamInfo.setCodecAndPayloadNumber(AMRWB_2_BE_1265, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_3_BE_1425: streamInfo.setBitRate(14250); streamInfo.setCodecAndPayloadNumber(AMRWB_3_BE_1425, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_4_BE_1585: streamInfo.setBitRate(15850); streamInfo.setCodecAndPayloadNumber(AMRWB_4_BE_1585, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_5_BE_1825: streamInfo.setBitRate(18250); streamInfo.setCodecAndPayloadNumber(AMRWB_5_BE_1825, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_6_BE_1985: streamInfo.setBitRate(19850); streamInfo.setCodecAndPayloadNumber(AMRWB_6_BE_1985, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_7_BE_2305: streamInfo.setBitRate(23050); streamInfo.setCodecAndPayloadNumber(AMRWB_7_BE_2305, protoProfile.AudioPayloadType); break;
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_8_BE_2385: streamInfo.setBitRate(23850); streamInfo.setCodecAndPayloadNumber(AMRWB_8_BE_2385, protoProfile.AudioPayloadType); break;

            case sip_1_port_server::EnumSipAudioCodec_G_711:
                streamInfo.setCodecAndPayloadNumber(G711u, PAYLOAD_711u);
                break;

            case sip_1_port_server::EnumSipAudioCodec_G_723_1:
                streamInfo.setBitRate(6300);
                streamInfo.setCodecAndPayloadNumber(G723_1, PAYLOAD_723);
                break;

            case sip_1_port_server::EnumSipAudioCodec_G_729:
                streamInfo.setCodecAndPayloadNumber(G729, PAYLOAD_729);
                break;

            case sip_1_port_server::EnumSipAudioCodec_G_711_A_LAW:
                streamInfo.setCodecAndPayloadNumber(G711A, PAYLOAD_711A);
                break;

            case sip_1_port_server::EnumSipAudioCodec_G_723_1_5_3:
                streamInfo.setBitRate(5300);
                streamInfo.setCodecAndPayloadNumber(G723_1, PAYLOAD_723);
                break;

            case sip_1_port_server::EnumSipAudioCodec_G_729_A:
                streamInfo.setCodecAndPayloadNumber(G729A, PAYLOAD_729);
                break;

            case sip_1_port_server::EnumSipAudioCodec_G_729_B:
                streamInfo.setCodecAndPayloadNumber(G729B, PAYLOAD_729);
                break;

            case sip_1_port_server::EnumSipAudioCodec_G_729_AB:
                streamInfo.setCodecAndPayloadNumber(G729AB, PAYLOAD_729);
                break;

            case sip_1_port_server::EnumSipAudioCodec_G_726_32:
                streamInfo.setBitRate(32000);
                streamInfo.setCodecAndPayloadNumber(G726, protoProfile.AudioPayloadType);
                break;

            case sip_1_port_server::EnumSipAudioCodec_G_711_LOWRATE:
            {
                streamInfo.setCodecAndPayloadNumber(CODEC_UNDEFINED, PAYLOAD_UNDEFINED);
                std::ostringstream oss;
                oss << "[UserAgentBlock ctor] Encoded RTP traffic Type: G711(LowRate) is not supported";
                const std::string err(oss.str());
                TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
                throw EPHXBadConfig(err);
            }
                break;

            case sip_1_port_server::EnumSipAudioCodec_G_711_1_MU_LAW_96:
                streamInfo.setBitRate(96000);
                streamInfo.setCodecAndPayloadNumber(CODEC_TYPE_G711_1u, protoProfile.AudioPayloadType);
                break;

            case sip_1_port_server::EnumSipAudioCodec_G_711_1_A_LAW_96:
                streamInfo.setBitRate(96000);
                streamInfo.setCodecAndPayloadNumber(CODEC_TYPE_G711_1A, protoProfile.AudioPayloadType);
                break;

            default:
            {
                streamInfo.setCodecAndPayloadNumber(CODEC_UNDEFINED, PAYLOAD_UNDEFINED);
                std::ostringstream oss;
                oss << "[UserAgentBlock ctor] Encoded RTP traffic Type: Invalid Audio Codec( " << protoProfile.AudioCodec << ")";
                const std::string err(oss.str());
                TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
                throw EPHXBadConfig(err);
            }
                break;
        }

        if ((audioFileIndex = userAgentFactory.getVoipMediaManager()->getStreamIndex(&streamInfo)) == INVALID_STREAM_INDEX) audioFileIndex =
                userAgentFactory.getVoipMediaManager()->prepareEncodedStream(&streamInfo);

        if (audioFileIndex == INVALID_STREAM_INDEX)
        {
            std::ostringstream oss;
            oss << "[UserAgentBlock ctor] Encoded RTP traffic Type: Cannot open Audio Encoded Stream File ( "
                    << streamInfo.getFilePath() << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
            throw EPHXBadConfig(err);

        }

        if (protoProfile.CallType == sip_1_port_server::EnumSipCallType_AUDIO_VIDEO)
        {

            MediaStreamInfo streamInfo;
            switch (protoProfile.VideoCodec)
            {

                case sip_1_port_server::EnumSipVideoCodec_H_264:
                    streamInfo.setBitRate(VH264_BR);
                    streamInfo.setCodecAndPayloadNumber(VH264, mConfig.Config().ProtocolProfile.VideoPayloadType);
                    break;

                default:
                    streamInfo.setCodecAndPayloadNumber(CODEC_UNDEFINED, PAYLOAD_UNDEFINED);
                    std::ostringstream oss;
                    oss << "[UserAgentBlock ctor] Encoded RTP traffic Type: Invalid Video Codec( " << protoProfile.VideoCodec
                            << ")";
                    const std::string err(oss.str());
                    TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
                    throw EPHXBadConfig(err);
                    break;
            }

            if ((videoFileIndex = userAgentFactory.getVoipMediaManager()->getStreamIndex(&streamInfo)) == INVALID_STREAM_INDEX) videoFileIndex =
                    userAgentFactory.getVoipMediaManager()->prepareEncodedStream(&streamInfo);

            if (videoFileIndex == INVALID_STREAM_INDEX)
            {
                std::ostringstream oss;
                oss << "[UserAgentBlock ctor] Encoded RTP traffic Type: Cannot open Encoded Stream File ( "
                        << streamInfo.getFilePath() << ")";
                const std::string err(oss.str());
                TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
                throw EPHXBadConfig(err);
            }
        }
    }

    // Create the UserAgent objects
    mUaVec.reserve(numUaObjects);
    UserAgentNameEnumerator uaNameEnum(protoConfig.UaNumStart, std::max(protoConfig.UaNumStep, static_cast<uint64_t>(1)),
                                       protoConfig.UaNumFormat);
    UserAgentNameEnumerator uaImsiEnum(protoConfig.Imsi, std::max(protoConfig.ImsiStep, static_cast<uint64_t>(1)), "");
    UserAgentNameEnumerator uaRanIDEnum(protoConfig.RanID, std::max(protoConfig.RanIDStep, static_cast<uint64_t>(1)), "");
    UserAgentNameEnumerator uaAuthUsernameEnum(protoConfig.AuthUserNameNumStart, protoConfig.AuthUserNameNumStep,
                                               protoConfig.AuthUserNameFormat);
    UserAgentNameEnumerator uaAuthPasswordEnum(protoConfig.AuthPasswordNumStart, protoConfig.AuthPasswordNumStep,
                                               protoConfig.AuthPasswordFormat);

    int ret = -1;
    std::vector<std::string> dnsServers;
    dnsServers.clear();
    int i;

    if (ifEnum.Type() == IFSETUP_NS::STC_DM_IFC_IPv4)
    {
        int sz = dnsProfile.Ipv4DnsServers.size();
        for (i = 0; i < sz; i++)
        {
            ACE_INET_Addr dnsip;
            std::string dnsip_str;
            SipUtils::SipAddressToInetAddress(dnsProfile.Ipv4DnsServers[i], dnsip);

            dnsip_str = SipUtils::InetAddressToString(dnsip);
            if (dnsip_str.length()) dnsServers.push_back(dnsip_str);
        }
    }
    else
    {
        int sz = dnsProfile.Ipv6DnsServers.size();
        for (i = 0; i < sz; i++)
        {
            ACE_INET_Addr dnsip;
            std::string dnsip_str;
            SipUtils::SipAddressToInetAddress(dnsProfile.Ipv6DnsServers[i], dnsip);

            dnsip_str = SipUtils::InetAddressToString(dnsip);
            if (dnsip_str.length()) dnsServers.push_back(dnsip_str);
        }
    }
    ret = mUserAgentFactory.setDNSServers(dnsServers, vdevblock);
    if (ret < 0)
    {
        std::ostringstream oss;
        oss << "[UserAgentBlock ctor] set Dns Servers failed .\n";
        const std::string err(oss.str());
        TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
        throw EPHXBadConfig(err);
    }

    mUserAgentFactory.setDNSTimeout(dnsProfile.DnsNetworkTimeout, dnsProfile.DnsRetryNumber + 1, vdevblock);

    size_t deviceIndex = 0;

    for (size_t i = 0; i < numUaObjects;)
    {
        unsigned int ifIndex = 0;
        ACE_INET_Addr ifAddr;
        ifEnum.GetInterface(ifIndex, ifAddr);

        uint16_t audioPort = protoProfile.AudioSrcPort;
        uint16_t audioHDPort = protoProfile.AudioSrcPort;
        uint16_t videoPort = protoProfile.VideoSrcPort;

        for (size_t j = 0; j < mUaNumsPerDevice; i++, j++)
        {

#ifdef UNIT_TEST
            {
                ACE_INET_Addr localAddr("127.0.0.1:0");
                const char* loopbackInterface="lo";

                if(mConfig.Config().Endpoint.DstIf.ifDescriptors[0].ifType == IFSETUP_NS::STC_DM_IFC_IPv6)
                {
                    ACE_INET_Addr localAddrIpv6(0,"::1",PF_INET6);
                    const char* loopbackInterfaceIpv6="lo";
                    ifAddr = localAddrIpv6;
                    ifAddr.set_port_number(protoConfig.LocalPort,1);
                    ifIndex = ACE_OS::if_nametoindex(loopbackInterfaceIpv6);
                }
                else if(VoIPUtils::AddrKey::equal(ifAddr,localAddr,false))
                {
                    ifIndex = ACE_OS::if_nametoindex(loopbackInterface);
                }
            }
#endif

            // Create and configure UserAgent object
            uaSharedPtr_t ua = userAgentFactory.getNewUserAgent(mPort, mVDevBlock, i, numUaObjects, j, deviceIndex,
                                                                uaNameEnum.GetName(), uaImsiEnum.GetName(), uaRanIDEnum.GetName(),
                                                                mConfig.Config());

            ua->SetStatusDelegate(boost::bind(&UserAgentBlock::NotifyUaStatus, this, _1, _2, _3));
            ua->RegisterVQStatsHandlers(mStats.vqResultsBlock);
            ua->SetInterface(ifIndex, ifAddr, mUaNumsPerDevice);

            ua->SetCalleeUriScheme(calleeUriScheme);
            size_t idx = i/mUaNumsPerDevice + (i%mUaNumsPerDevice)*mDevicesTotal;

            if (idx < audioRtpStreamIndexes.size() && idx < audioRtcpStreamIndexes.size()) ua->SetAudioStreamIndexes(
                    audioRtpStreamIndexes[idx], audioRtcpStreamIndexes[idx]);

            if (idx < videoRtpStreamIndexes.size() && idx < videoRtcpStreamIndexes.size()) ua->SetVideoStreamIndexes(
                    videoRtpStreamIndexes[idx], videoRtcpStreamIndexes[idx]);

            ua->SetAudioSourcePort(audioPort);
            ua->SetAudioHDSourcePort(audioHDPort);
            ua->SetVideoSourcePort(videoPort);
            ua->SetRegExpiresTime(protoProfile.RegExpires);

            if (mobileConfig.AkaAuthentication || (protoConfig.AuthType == sip_1_port_server::EnumAuthentication_MD5_DIGEST))
            {

                ua->SetAuthCredentials(uaAuthUsernameEnum.GetName(), uaAuthPasswordEnum.GetName());
                ua->SetAuthPasswordDebug(protoConfig.AuthPasswordDebug);
                uaAuthUsernameEnum.Next();
                uaAuthPasswordEnum.Next();
            }

            mUaVec.push_back(ua);

            // Increment local port numbers
            ifAddr.set_port_number(ifAddr.get_port_number());
            audioPort += (protoProfile.CallType == sip_1_port_server::EnumSipCallType_AUDIO_VIDEO || (protoProfile.CallType == sip_1_port_server::EnumSipCallType_TELEPRESENCE && protoProfile.AudioCodec != sip_1_port_server::EnumSipAudioCodec_AAC_LD)) ? 4 : 2;
            audioHDPort += 4;
            videoPort += 4;
            uaNameEnum.Next();
            uaImsiEnum.Next();
            uaRanIDEnum.Next();
            ua->SetAudioFileStreamIndex(audioFileIndex);
            ua->SetAudioHDFileStreamIndex(audioHDFileIndex);
            ua->SetVideoFileStreamIndex(videoFileIndex);
        }

        deviceIndex++;
        ifEnum.Next();
    }

    TC_LOG_DEBUG_LOCAL(
            mPort, LOG_SIP,
            "[UserAgentBlock ctor] User Agent block (" << BllHandle() << ") created with " << TotalCount() << " UserAgent objects");
}

///////////////////////////////////////////////////////////////////////////////

UserAgentBlock::~UserAgentBlock()
{
    try
    {
        Stop();
    }
    catch (...)
    {
    }
}

///////////////////////////////////////////////////////////////////////////////

std::auto_ptr<UserAgentBlock::RegistrationLoadStrategy> UserAgentBlock::MakeRegistrationLoadStrategy(void)
{
    std::auto_ptr<RegistrationLoadStrategy> loadStrategy(new RegistrationLoadStrategy(*this));

    const SipUaProtocolProfile_t& protoProfile(mConfig.Config().ProtocolProfile);
    loadStrategy->SetLoad(static_cast<int32_t>(protoProfile.RegsPerSecond), protoProfile.RegBurstSize);

    return loadStrategy;
}

///////////////////////////////////////////////////////////////////////////////

std::auto_ptr<UserAgentBlock::ProtocolLoadStrategy> UserAgentBlock::MakeProtocolLoadStrategy(void)
{
    const L4L7_BASE_NS::LoadTypes loadType = static_cast<const L4L7_BASE_NS::LoadTypes>(mConfig.Config().Load.LoadProfile.LoadType);
    switch (loadType)
    {
        case L4L7_BASE_NS::LoadTypes_CONNECTIONS:
        case L4L7_BASE_NS::LoadTypes_TRANSACTIONS:
            return std::auto_ptr<ProtocolLoadStrategy>(new CallsLoadStrategy(*this));

        case L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT:
        case L4L7_BASE_NS::LoadTypes_TRANSACTIONS_PER_TIME_UNIT:
            return std::auto_ptr<ProtocolLoadStrategy>(new CallsOverTimeLoadStrategy(*this));

        default:
        {
            std::ostringstream oss;
            oss << "[UserAgentBlock::MakeLoadStrategy] Cannot create load strategy for unknown load type ("
                    << tms_enum_to_string(loadType, L4L7_BASE_NS::em_LoadTypes) << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
            throw EPHXBadConfig(err);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::EnumerateContactAddresses(const contactAddressConsumer_t& consumer) const
{
    BOOST_FOREACH(const uaSharedPtr_t& ua, mUaVec)
    {
        if (ua->Enabled()) consumer(ua->ContactAddress());
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::Reset(void)
{
    if (!mEnabled) return;

    TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgentBlock::Register] User Agent block (" << BllHandle() << ") resetting");

    // Transition to NOT_REGISTERED state and halt all registration activity
    ChangeRegState(sip_1_port_server::EnumSipUaRegState_NOT_REGISTERED);

    if (mRegStrategy->Running()) mRegStrategy->Stop();

    // Halt all protocol load generation activity
    if (mLoadScheduler->Running()) mLoadScheduler->Stop();

    // Reset all UA's
    std::for_each(mUaVec.begin(), mUaVec.end(), boost::mem_fn(&UserAgent::Reset));
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::Register(const contactAddressResolver_t& contactAddressResolver)
{
    if (!mEnabled) return;

    TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgentBlock::Register] User Agent block (" << BllHandle() << ") starting registration");
    ChangeRegState(sip_1_port_server::EnumSipUaRegState_REGISTERING);

    // Build the extra contact address list
    const std::vector<uint32_t>& extraContactAddressesHandleList(mConfig.Config().ExtraContactAddressesHandleList);
    if (contactAddressResolver && !extraContactAddressesHandleList.empty())
    {
        mExtraContactAddresses.reset(new uriVec_t);
        mExtraContactAddresses->reserve(extraContactAddressesHandleList.size());
        contactAddressConsumer_t pushBackUri = boost::bind(&uriVec_t::push_back, mExtraContactAddresses.get(), _1);
        std::for_each(extraContactAddressesHandleList.begin(), extraContactAddressesHandleList.end(),
                      boost::bind(contactAddressResolver, _1, pushBackUri));
    }

    // Reset the registration variables and (re)start the registration load strategy
    mRegIndex = 0;
    mRegOutstanding = 0;
    mRegStrategy->Start(mReactor);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::Unregister(void)
{
    if (!mEnabled) return;

    TC_LOG_INFO_LOCAL(mPort, LOG_SIP,
                      "[UserAgentBlock::Unregister] User Agent block (" << BllHandle() << ") starting unregistration");
    ChangeRegState(sip_1_port_server::EnumSipUaRegState_UNREGISTERING);

    // Reset the registration index and (re)start the registration load strategy
    mRegIndex = 0;
    mRegStrategy->Start(mReactor);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::CancelRegistrations(void)
{
    if (!mEnabled || !mRegStrategy->Running()) return;

    TC_LOG_INFO_LOCAL(mPort, LOG_SIP,
                      "[UserAgentBlock::CancelRegistrations] User Agent block (" << BllHandle() << ") canceing registration");

    // Cancel registrations in progress
    std::for_each(mUaVec.begin(), mUaVec.end(), boost::mem_fn(&UserAgent::AbortRegistration));

    ChangeRegState(sip_1_port_server::EnumSipUaRegState_REGISTRATION_CANCELED);
    mRegStrategy->Stop();
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::Start(void)
{
    if (!mEnabled || mLoadScheduler->Running()) return;

    TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgentBlock::Start] User Agent block (" << BllHandle() << ") starting protocol");

    // Reset our internal load state
    mAttemptedCallCount = 0;

    // Reset our destination enumerators

    if (mUsingMobile && mUsingIMSI) mDstImsiEnum->Reset();
    else mDstNameEnum->Reset();

    if (!mUsingProxy)
    {
        if (!mDstIfEnum)
        {
            TC_LOG_ERR_LOCAL(
                    mPort,
                    LOG_SIP,
                    "[UserAgentBlock::Start] User Agent block (" << BllHandle()
                            << ") has no destination interface info, cannot start");
            return;
        }

        mDstIfEnum->Reset();
    }

    // Reload the idle UA deque
    mUaIdleDeque.assign(mUaVec.begin(), mUaVec.end());

    // Clear the UA aging queue
    mUaAgingQueue.clear();

    // Start load scheduler
    mLoadScheduler->Start(mReactor);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::Stop(void)
{
    if (!mEnabled) return;

    TC_LOG_INFO_LOCAL(
            mPort,
            LOG_SIP,
            "[UserAgentBlock::Stop] User Agent block (" << BllHandle() << ") stopping protocol with " << ActiveCalls()
                    << " active calls");

    // Stop load scheduler
    if (mLoadScheduler->Running()) mLoadScheduler->Stop();

    // Stop all active calls
    AbortCalls(ActiveCalls());
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::NotifyInterfaceDisabled(void)
{
    TC_LOG_INFO_LOCAL(
            mPort, LOG_SIP,
            "[UserAgentBlock::NotifyInterfaceDisabled] User Agent block (" << BllHandle() << ") notified interface is disabled");

    // Transition to NOT_REGISTERED state and halt all registration activity
    ChangeRegState(sip_1_port_server::EnumSipUaRegState_NOT_REGISTERED);

    if (mRegStrategy->Running()) mRegStrategy->Stop();

    // Halt all protocol load generation activity
    if (mLoadScheduler->Running()) mLoadScheduler->Stop();

    //Remove from start queue
    {
        unsigned int prevIfIndex = 0xFFFFFFFF;
        SIP_NS::UserAgentFactory::InterfaceContainer ifs;
        uaVec_t::iterator iter = mUaVec.begin();
        while (iter != mUaVec.end())
        {
            SIP_NS::uaSharedPtr_t ua=*iter;
            if(ua)
            {
                if(mPort == ua->Port())
                {
                    unsigned int ifIndex = ua->IfIndex();
                    if(ifIndex!=prevIfIndex)
                    {
                        ifs.insert(ifIndex);
                        prevIfIndex=ifIndex;
                    }
                }
            }
            ++iter;
        }
        mUserAgentFactory.stopPortInterfaces(mPort, ifs);
    }

    // Notify all UA's
    std::for_each(mUaVec.begin(), mUaVec.end(), boost::mem_fn(&UserAgent::NotifyInterfaceDisabled));
    mEnabled = false;
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::NotifyInterfaceEnabled(void)
{
    TC_LOG_INFO_LOCAL(
            mPort, LOG_SIP,
            "[UserAgentBlock::NotifyInterfaceEnabled] User Agent block (" << BllHandle() << ") notified interface is enabled");

    // Notify all UA's
    std::for_each(mUaVec.begin(), mUaVec.end(), boost::mem_fn(&UserAgent::NotifyInterfaceEnabled));
    mEnabled = true;
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::StartRegistrations(size_t count)
{
    const size_t numUaObjects = mUaVec.size();
    while (count && mRegIndex < numUaObjects)
    {
        uaSharedPtr_t ua(mUaVec[mRegIndex]);

        // If UA is already registered, skip it
        if (ua->IsRegistered())
        {
            mRegIndex++;
            continue;
        }

        // Start UA registration
        const bool started = ua->Register(mExtraContactAddresses.get());

        // Bump attempted registration counter
        mStats.nonInviteSessionsInitiated++;
        mStats.regAttempts++;
        mStats.setDirty(true);

        if (started) mRegOutstanding++;
        else mStats.regFailures++;

        count--;
        mRegIndex++;
    }

    // If we've reached the end of the UA vector, no registrations outstanding, and no registrations have failed we can transition to REGISTRATION_SUCCEEDED
    if (!mRegOutstanding && mRegIndex == numUaObjects && mRegState != sip_1_port_server::EnumSipUaRegState_REGISTRATION_FAILED)
    {
        ChangeRegState(sip_1_port_server::EnumSipUaRegState_REGISTRATION_SUCCEEDED);
        mRegStrategy->Stop();
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::StartUnregistrations(size_t count)
{
    const size_t numUaObjects = mUaVec.size();

    while (count && mRegIndex < numUaObjects)
    {
        uaSharedPtr_t ua(mUaVec[mRegIndex]);

        // If UA isn't registered, skip it
        if (!ua->IsRegistered())
        {
            mRegIndex++;
            continue;
        }

        // Start UA unregistration
        ua->Unregister();

        count--;
        mRegIndex++;
        mStats.nonInviteSessionsInitiated++;
    }

    // If we've reached the end of the UA vector we can transition to NOT_REGISTERED
    if (mRegIndex == numUaObjects)
    {
        ChangeRegState(sip_1_port_server::EnumSipUaRegState_NOT_REGISTERED);
        mRegStrategy->Stop();
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::ChangeRegState(regState_t toState)
{
    if (toState != mRegState)
    {
        const regState_t fromState(mRegState);

        TC_LOG_INFO_LOCAL(
                mPort,
                LOG_SIP,
                "[UserAgentBlock::ChangeRegState] UserAgentBlock (" << BllHandle() << ") changing registration state from "
                        << tms_enum_to_string(fromState, em_EnumSipUaRegState) << " to "
                        << tms_enum_to_string(toState, em_EnumSipUaRegState));
        mRegState = toState;

        if (!mRegStateNotifier.empty()) mRegStateNotifier(BllHandle(), fromState, toState);
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::InitiateCalls(size_t count)
{
    // New calls may be limited by the L4-L7 load profile
    const uint32_t maxConnAttempted = mConfig.Config().Load.LoadProfile.MaxConnectionsAttempted;
    if (maxConnAttempted)
    {
        if (maxConnAttempted <= mAttemptedCallCount) count = 0;
        else count = std::min(static_cast<size_t>(maxConnAttempted - mAttemptedCallCount), count);
    }

    const uint32_t maxOpenConn = mConfig.Config().Load.LoadProfile.MaxOpenConnections;
    if (maxOpenConn)
    {
        if (maxOpenConn <= ActiveCalls()) count = 0;
        else count = std::min(maxOpenConn - ActiveCalls(), count);
    }

    // We can only initiate as many calls as we have currently idle UA's
    count = std::min(count, mUaIdleDeque.size());

    if (!count) return;

    mUaNumsPerDevice = std::max(static_cast<size_t>(1), mUaNumsPerDevice);
    mDevicesTotal = std::max(static_cast<size_t>(1), mDevicesTotal);

    size_t remoteUaNumsPerDevice = 0;

    if (!mUsingProxy)
    {
        if (mUsingMobile && mUsingIMSI) remoteUaNumsPerDevice = (size_t) (mDstImsiEnum->TotalCount() / mDstIfEnum->TotalCount());
        else remoteUaNumsPerDevice = (size_t) (mDstNameEnum->TotalCount() / mDstIfEnum->TotalCount());
    }

    ACE_INET_Addr dstAddr; // Note: intentionally hoisted out of loop
    while (count)
    {
        // Pop most idle UA from deque
        uaSharedPtr_t ua(mUaIdleDeque.front());
        mUaIdleDeque.pop_front();

        // Initiate a call from the UA to its configured destination
        bool started;
        if (mUsingProxy)
        {
            if (mUsingMobile && mUsingIMSI)
            {
                mDstImsiEnum->SetPosition(ua->Index() % mDstImsiEnum->TotalCount());
                started = ua->Call(mDstImsiEnum->GetName());
            }
            else
            {
                mDstNameEnum->SetPosition(ua->Index() % mDstNameEnum->TotalCount());
                started = ua->Call(mDstNameEnum->GetName());
            }
        }
        else
        {
            if (mUsingMobile && mUsingIMSI)
            {
                mDstImsiEnum->SetPosition(ua->Index() % mDstImsiEnum->TotalCount());
                size_t deviceBank = (size_t) (ua->Index() / remoteUaNumsPerDevice);
                mDstIfEnum->SetPosition(deviceBank % mDstIfEnum->TotalCount());
                mDstIfEnum->GetInterface(dstAddr);
                started = ua->Call(mDstImsiEnum->GetName(), dstAddr);
            }
            else
            {
                mDstNameEnum->SetPosition(ua->Index() % mDstNameEnum->TotalCount());
                size_t deviceBank = (size_t) (ua->Index() / remoteUaNumsPerDevice);
                mDstIfEnum->SetPosition(deviceBank % mDstIfEnum->TotalCount());
                mDstIfEnum->GetInterface(dstAddr);
                started = ua->Call(mDstNameEnum->GetName(), dstAddr);
            }
        }

        // Bump attempted call counters
        mAttemptedCallCount++;
        mStats.callAttempts++;
        mStats.setDirty(true);

        if (started)
        {
            // Insert the UA into the aging queue
            mUaAgingQueue.push(ua);
            mStats.callsActive++;
        }
        else
        {
            // Push the UA back onto the idle queue
            mUaIdleDeque.push_back(ua);
            mStats.callFailures++;
        }

        count--;
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::AbortCalls(size_t count,bool bGraceful)
{
    // We can only abort as many calls as we have currently active UA's
    count = std::min(count, mUaAgingQueue.size());
    while (count--)
    {
        // Pop oldest active UA from queue
        uaSharedPtr_t ua(mUaAgingQueue.front());
        mUaAgingQueue.pop();

        // Abort the call
        ua->AbortCall(bGraceful);
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::NotifyLoadProfileRunning(bool running) const
{
    // Cascade notification
    if (!mLoadProfileNotifier.empty()) mLoadProfileNotifier(BllHandle(), running);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::NotifyUaStatus(size_t uaIndex, UserAgent::StatusNotification status, const ACE_Time_Value& deltaTime)
{
    switch (status)
    {
        case UserAgent::STATUS_REGISTRATION_SUCCEEDED:
            // If all registrations were successful, we can transition to REGISTRATION_SUCCEEDED
            // (otherwise it will already indicate REGISTRATION_FAILED)
            if (--mRegOutstanding == 0 && mRegIndex == mUaVec.size()
                    && mRegState == sip_1_port_server::EnumSipUaRegState_REGISTERING)
            {
                ChangeRegState(sip_1_port_server::EnumSipUaRegState_REGISTRATION_SUCCEEDED);
                mRegStrategy->Stop();
            }

            {
                const uint32_t regTimeMsec = deltaTime.msec();

                mStats.regSuccesses++;
                mStats.regMinTimeMsec =
                        mStats.regMinTimeMsec ? std::min(mStats.regMinTimeMsec, static_cast<uint64_t>(regTimeMsec)) :
                                                static_cast<uint64_t>(regTimeMsec);
                mStats.regMaxTimeMsec = std::max(mStats.regMaxTimeMsec, static_cast<uint64_t>(regTimeMsec));
                mStats.regCummTimeMsec += regTimeMsec;
                mStats.setDirty(true);
            }
            break;

        case UserAgent::STATUS_REGISTRATION_RETRYING:
            mStats.regAttempts++;
            mStats.regRetries++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_REGISTRATION_FAILED: 
	{
            // Once a single registration fails, we can transition to REGISTRATION_FAILED immediately
            ChangeRegState(sip_1_port_server::EnumSipUaRegState_REGISTRATION_FAILED);
            mRegOutstanding--;
            mStats.regFailures++;

            const uint32_t responseTime = deltaTime.msec();
          
            mStats.minFailedRegistrationDelay = mStats.minFailedRegistrationDelay ? std::min(mStats.minFailedRegistrationDelay, static_cast<uint64_t>(responseTime)) : static_cast<uint64_t>(responseTime);
            mStats.maxFailedRegistrationDelay = std::max(mStats.maxFailedRegistrationDelay, static_cast<uint64_t>(responseTime));
            mStats.cummFailedRegistrationDelay += responseTime;

            mStats.setDirty(true);
	}
        break;
        case UserAgent::STATUS_CALL_SUCCEEDED:
        {
            const uint32_t callTimeMsec = deltaTime.msec();

            mStats.callSuccesses++;
            mStats.callMinTimeMsec =
                    mStats.callMinTimeMsec ? std::min(mStats.callMinTimeMsec, static_cast<uint64_t>(callTimeMsec)) :
                                             static_cast<uint64_t>(callTimeMsec);
            mStats.callMaxTimeMsec = std::max(mStats.callMaxTimeMsec, static_cast<uint64_t>(callTimeMsec));
            mStats.callCummTimeMsec += callTimeMsec;
            mStats.setDirty(true);
        }
        break;

        case UserAgent::STATUS_CALL_FAILED:
	{
            // A call failed -- move the UA from active to idle
            mUaAgingQueue.erase(mUaVec.at(uaIndex));
            mUaIdleDeque.push_back(mUaVec.at(uaIndex));

            // Decrement number of active calls
            mStats.callsActive--;

            mStats.callFailures++;

            const uint32_t failedTimeMsec = deltaTime.msec();
          
            mStats.minFailedSessionSetupDelay = mStats.minFailedSessionSetupDelay ? std::min(mStats.minFailedSessionSetupDelay, static_cast<uint64_t>(failedTimeMsec)) : static_cast<uint64_t>(failedTimeMsec);
            mStats.maxFailedSessionSetupDelay = std::max(mStats.maxFailedSessionSetupDelay, static_cast<uint64_t>(failedTimeMsec));
            mStats.cummFailedSessionSetupDelay += failedTimeMsec;

            mStats.setDirty(true);

            if (mLoadScheduler->Running())
            {
                // Inform the load strategy that a call completed
                if (mAsyncCompletionHandler) mAsyncCompletionHandler->QueueAsyncCompletion(mCallCompleteAct);
                else CallComplete();
            }
	}
        break;

        case UserAgent::STATUS_CALL_ANSWERED:
            mStats.callAnswers++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_CALL_COMPLETED:
            // A call was completed -- move the UA from active to idle
            mUaAgingQueue.erase(mUaVec.at(uaIndex));
            mUaIdleDeque.push_back(mUaVec.at(uaIndex));

            // Decrement number of active calls
            mStats.callsActive--;

            mStats.setDirty(true);

            if (mLoadScheduler->Running())
            {
                // Inform the load strategy that a call completed
                if (mAsyncCompletionHandler) mAsyncCompletionHandler->QueueAsyncCompletion(mCallCompleteAct);
                else CallComplete();
            }
            break;

        case UserAgent::STATUS_REFRESH_COMPLETED:
            mStats.sessionRefreshes++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_RESPONSE1XX_SENT:
            mStats.txResponseCode1XXCount++;
            mStats.setDirty(true);
            break;
      
        case UserAgent::STATUS_RESPONSE2XX_SENT:
            mStats.txResponseCode2XXCount++;
            mStats.setDirty(true);
            break;
      
        case UserAgent::STATUS_RESPONSE3XX_SENT:
            mStats.txResponseCode3XXCount++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_RESPONSE4XX_SENT:
            mStats.txResponseCode4XXCount++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_RESPONSE5XX_SENT:
            mStats.txResponseCode5XXCount++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_RESPONSE6XX_SENT:
            mStats.txResponseCode6XXCount++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_RESPONSE1XX_RECEIVED:
            mStats.rxResponseCode1XXCount++;
            mStats.setDirty(true);
            break;
      
        case UserAgent::STATUS_RESPONSE2XX_RECEIVED:
            mStats.rxResponseCode2XXCount++;
            mStats.setDirty(true);
            break;
      
        case UserAgent::STATUS_RESPONSE3XX_RECEIVED:
            mStats.rxResponseCode3XXCount++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_RESPONSE4XX_RECEIVED:
            mStats.rxResponseCode4XXCount++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_RESPONSE5XX_RECEIVED:
            mStats.rxResponseCode5XXCount++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_RESPONSE6XX_RECEIVED:
            mStats.rxResponseCode6XXCount++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_INVITE_SENT:
            mStats.inviteRequestsSent++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_INVITE_RECEIVED:
	{
            mStats.inviteRequestsReceived++;

            const uint32_t hops = 70 - deltaTime.sec();

            mStats.minHopsPerReq = mStats.minHopsPerReq ? std::min(mStats.minHopsPerReq, static_cast<uint64_t>(hops)) : static_cast<uint64_t>(hops);
            mStats.maxHopsPerReq = std::max(mStats.maxHopsPerReq, static_cast<uint64_t>(hops));
            mStats.cummHopsPerReq += hops;
            mStats.setDirty(true);
	}
        break;

        case UserAgent::STATUS_INVITE_RETRYING:
            mStats.inviteRequestsSent++;
            mStats.inviteMessagesRetransmitted++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_CALL_FAILED_TIMER_B:
            mStats.callsFailedTimerB++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_CALL_FAILED_TRANSPORT_ERROR:
            mStats.callsFailedTransportError++;
            mStats.setDirty(true);
            break;
          
        case UserAgent::STATUS_CALL_FAILED_5XX:
            mStats.callsFailed5xx++;
            mStats.setDirty(true);
            break;    

        case UserAgent::STATUS_ACK_SENT:
            mStats.ackRequestsSent++;
            mStats.setDirty(true);
            break;  
          
        case UserAgent::STATUS_ACK_RECEIVED:
            mStats.ackRequestsReceived++;
            mStats.setDirty(true);
            break;  

        case UserAgent::STATUS_BYE_SENT:
            {
                mStats.byeRequestsSent++;

                const uint32_t responseTime = deltaTime.sec();  

                mStats.minSuccessfulSessionDuration = mStats.minSuccessfulSessionDuration ? std::min(mStats.minSuccessfulSessionDuration, static_cast<uint64_t>(responseTime)) : static_cast<uint64_t>(responseTime);
                mStats.maxSuccessfulSessionDuration = std::max(mStats.maxSuccessfulSessionDuration, static_cast<uint64_t>(responseTime));
                mStats.cummSuccessfulSessionDuration += responseTime;

                mStats.avgSuccessfulSessionDuration = (mStats.byeRequestsSent - mStats.byeMessagesRetransmitted) > 0 ? static_cast<uint64_t>((static_cast<double>(mStats.cummSuccessfulSessionDuration) / (mStats.byeRequestsSent - mStats.byeMessagesRetransmitted))) : 0;
                mStats.setDirty(true);
            }
            break;  
          
        case UserAgent::STATUS_BYE_RECEIVED:
            mStats.byeRequestsReceived++;
            mStats.setDirty(true);
            break;      

        case UserAgent::STATUS_BYE_RETRYING:
            mStats.byeRequestsSent++;
            mStats.byeMessagesRetransmitted++;
            mStats.setDirty(true);
            break;  

        case UserAgent::STATUS_BYE_DISCONNECTED:
            {
                const uint32_t responseTime = deltaTime.msec();  

                mStats.minDisconnectResponseDelay = mStats.minDisconnectResponseDelay ? std::min(mStats.minDisconnectResponseDelay, static_cast<uint64_t>(responseTime)) : static_cast<uint64_t>(responseTime);
                mStats.maxDisconnectResponseDelay = std::max(mStats.maxDisconnectResponseDelay, static_cast<uint64_t>(responseTime));
                mStats.cummDisconnectResponseDelay += responseTime;
                mStats.setDirty(true);
            }
            break;
          
        case UserAgent::STATUS_BYE_FAILED:
            mStats.failedSessionDisconnectCount++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_INVITE_1XX_RESPONSE_RECEIVED:
            {
                const uint32_t responseTime = deltaTime.msec();

                mStats.minResponseDelay = mStats.minResponseDelay ? std::min(mStats.minResponseDelay, static_cast<uint64_t>(responseTime)) : static_cast<uint64_t>(responseTime);
                mStats.maxResponseDelay = std::max(mStats.maxResponseDelay, static_cast<uint64_t>(responseTime));
                mStats.cummResponseDelay += responseTime;
                  
                mStats.setDirty(true);
            }
            break;

        case UserAgent::STATUS_INVITE_180_RESPONSE_RECEIVED:
            {
                const uint32_t responseTime = deltaTime.msec();

                mStats.minPostDialDelay = mStats.minPostDialDelay ? std::min(mStats.minPostDialDelay, static_cast<uint64_t>(responseTime)) : static_cast<uint64_t>(responseTime);
                mStats.maxPostDialDelay = std::max(mStats.maxPostDialDelay, static_cast<uint64_t>(responseTime));
                mStats.cummPostDialDelay += responseTime;
                mStats.setDirty(true);
            }
            break;
          
        case UserAgent::STATUS_INVITE_ACK_RECEIVED:
            {
                const uint32_t responseTime = deltaTime.msec();

                mStats.minSessionAckDelay = mStats.minSessionAckDelay ? std::min(mStats.minSessionAckDelay, static_cast<uint64_t>(responseTime)) : static_cast<uint64_t>(responseTime);
                mStats.maxSessionAckDelay = std::max(mStats.maxSessionAckDelay, static_cast<uint64_t>(responseTime));
                mStats.cummSessionAckDelay += responseTime;
                mStats.setDirty(true);
            }
            break;

        case UserAgent::STATUS_NONINVITE_SESSION_INITIATED:
            mStats.nonInviteSessionsInitiated++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_NONINVITE_SESSION_SUCCESSFUL:
            mStats.nonInviteSessionsSucceeded++;
            mStats.setDirty(true);
            break;
          
        case UserAgent::STATUS_NONINVITE_SESSION_FAILED:
            mStats.nonInviteSessionsFailed++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_NONINVITE_SESSION_FAILED_TIMER_F:
            mStats.nonInviteSessionsFailed++;
            mStats.nonInviteSessionsFailedTimerF++;
            mStats.setDirty(true);
            break;

        case UserAgent::STATUS_NONINVITE_SESSION_FAILED_TRANSPORT:
            mStats.nonInviteSessionsFailed++;
            mStats.nonInviteSessionsFailedTransport++;
            mStats.setDirty(true);
            break;

        default:
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::CallComplete(void)
{
    // If we're still running ask for more load
    if (mLoadScheduler->Running()) mLoadStrategy->CallCompleted();
}

///////////////////////////////////////////////////////////////////////////////
