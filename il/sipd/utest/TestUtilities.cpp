#include <base/BaseCommon.h>
#include <vif/IfSetupCommon.h>

#include "VoIPCommon.h"
#include "SipCommon.h"

#include "TestUtilities.h"

USING_APP_NS;

namespace TestUtilities
{

std::auto_ptr<UserAgentConfig> MakeUserAgentConfigCommon(void)
{
    Sip_1_port_server::SipUaConfig_t config;

    config.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_CONNECTIONS;
    config.Load.LoadProfile.RandomizationSeed = 0;
    config.Load.LoadProfile.MaxConnectionsAttempted = 0;
    config.Load.LoadProfile.MaxOpenConnections = 0;
    config.Load.LoadProfile.MaxTransactionsAttempted = 0;
    config.Load.LoadPhases.resize(2);
    config.Load.LoadPhases[0].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    config.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    config.Load.LoadPhases[0].FlatDescriptor.Height = 100;
    config.Load.LoadPhases[0].FlatDescriptor.RampTime = 0;
    config.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 60;
    config.Load.LoadPhases[1].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    config.Load.LoadPhases[1].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    config.Load.LoadPhases[1].FlatDescriptor.Height = 0;
    config.Load.LoadPhases[1].FlatDescriptor.RampTime = 0;
    config.Load.LoadPhases[1].FlatDescriptor.SteadyTime = 0;
    config.Profile.L4L7Profile.ProfileName = "Test";
    config.Profile.L4L7Profile.Ipv4Tos = 0;
    config.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    config.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    config.Profile.L4L7Profile.EnableDelayedAck = true;
    config.Endpoint.SrcIfHandle = 0;
    config.Endpoint.DstUaNumStart = 2000;
    config.Endpoint.DstUaNumEnd = 3000;
    config.Endpoint.DstUaNumStep = 1;
    config.Endpoint.DstUaNumFormat = "%u";
    config.Endpoint.DstPort = 0;
    config.ProtocolProfile.CallType = sip_1_port_server::EnumSipCallType_AUDIO_VIDEO;
    config.ProtocolProfile.RtpTrafficType = sip_1_port_server::EnumRtpTrafficType_SIMULATED_RTP;
    config.ProtocolProfile.UseUaToUaSignaling = true;
    config.ProtocolProfile.UseCompactHeaders = false;
    config.ProtocolProfile.ProxyDomain = "proxy.foo.com";
    config.ProtocolProfile.ProxyPort = 0;
    config.ProtocolProfile.RingTime = 1;
    config.ProtocolProfile.CallTime = 5;
    config.ProtocolProfile.AudioSrcPort = 50050;
    config.ProtocolProfile.AudioCodec = sip_1_port_server::EnumSipAudioCodec_G_711;
    config.ProtocolProfile.VideoSrcPort = 50052;
    config.ProtocolProfile.VideoCodec = sip_1_port_server::EnumSipVideoCodec_H_263;
    config.ProtocolProfile.RegsPerSecond = 10;
    config.ProtocolProfile.RegBurstSize = 1;
    config.ProtocolProfile.RegRetries = 0;
    config.ProtocolProfile.RegExpires = 604800;
    config.ProtocolProfile.EnableRegRefresh = false;
    config.ProtocolProfile.MinSessionExpiration = 90;
    config.ProtocolProfile.DefaultRefresher=sip_1_port_server::EnumDefaultRefresher_REFRESHER_UAC;
    config.ProtocolProfile.CalleeUriScheme = sip_1_port_server::EnumSipUriScheme_SIP;
    config.ProtocolProfile.SecureRequestURI = false;
    config.ProtocolProfile.SipMessagesTransport = sip_1_port_server::EnumSipMessagesTransport_UDP;
    config.ProtocolProfile.CallIDDomainName = true;
    config.ProtocolProfile.ReliableProvisonalResponse = false;
    config.ProtocolProfile.AnonymousCall = false;
    config.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 1;
    config.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "Test";
    config.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_PAIR;
    config.ProtocolConfig.LocalPort = 5070;
    config.ProtocolConfig.UaNumsPerDevice = 1;
    config.ProtocolConfig.UaNumStart = 1000;
    config.ProtocolConfig.UaNumStep = 1;
    config.ProtocolConfig.UaNumFormat = "%u";
    config.ProtocolProfile.AudioPayloadType = 96;
    config.ProtocolProfile.VideoPayloadType = 112;
    config.MobileProfile.MobileSignalingChoice = 0;
    config.MobileProfile.AkaAuthentication = false;
    
    return std::auto_ptr<UserAgentConfig>(new UserAgentConfig(0, config));
}
    
std::auto_ptr<UserAgentConfig> MakeUserAgentConfigIpv4(void)
{
    std::auto_ptr<UserAgentConfig> uaConfig(MakeUserAgentConfigCommon());
    Sip_1_port_server::SipUaConfig_t& config(uaConfig->Config());

    config.Endpoint.DstIf.ifDescriptors.resize(1);
    config.Endpoint.DstIf.ifDescriptors[0].ifType = IFSETUP_NS::STC_DM_IFC_IPv4;
    config.Endpoint.DstIf.ifDescriptors[0].indexInList = 0;
    config.Endpoint.DstIf.Ipv4InterfaceList.resize(1);
    config.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsRange = true;
    config.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsDirectlyConnected = true;
    config.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfCountPerLowerIf = 1;
    config.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfRecycleCount = 0;
    config.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.TotalCount = 1;
    config.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.BllHandle = 0;
    config.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.AffiliatedInterface = 0;
    config.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[0] = 127;
    config.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[1] = 0;
    config.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[2] = 0;
    config.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[3] = 1;
    config.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[0] = 0;
    config.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[1] = 0;
    config.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[2] = 0;
    config.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[3] = 0;
    config.Endpoint.DstIf.Ipv4InterfaceList[0].SkipReserved = false;
    config.Endpoint.DstIf.Ipv4InterfaceList[0].AddrRepeatCount = 0;
    config.Endpoint.DstIf.Ipv4InterfaceList[0].PrefixLength = 8;

    config.ProtocolProfile.ProxyIpv4Addr.address[0] = 127;
    config.ProtocolProfile.ProxyIpv4Addr.address[1] = 0;
    config.ProtocolProfile.ProxyIpv4Addr.address[2] = 0;
    config.ProtocolProfile.ProxyIpv4Addr.address[3] = 1;
    config.MobileProfile.MobileSignalingChoice = 0;

    return uaConfig;
}

std::auto_ptr<UserAgentConfig> MakeUserAgentConfigIpv6(void)
{
    std::auto_ptr<UserAgentConfig> uaConfig(MakeUserAgentConfigCommon());
    Sip_1_port_server::SipUaConfig_t& config(uaConfig->Config());

    config.Endpoint.DstIf.ifDescriptors.resize(1);
    config.Endpoint.DstIf.ifDescriptors[0].ifType = IFSETUP_NS::STC_DM_IFC_IPv6;
    config.Endpoint.DstIf.ifDescriptors[0].indexInList = 0;
    config.Endpoint.DstIf.Ipv6InterfaceList.resize(1);
    config.Endpoint.DstIf.Ipv6InterfaceList[0].NetworkInterface.EmulatedIf.IsRange = true;
    config.Endpoint.DstIf.Ipv6InterfaceList[0].NetworkInterface.EmulatedIf.IsDirectlyConnected = true;
    config.Endpoint.DstIf.Ipv6InterfaceList[0].NetworkInterface.IfCountPerLowerIf = 1;
    config.Endpoint.DstIf.Ipv6InterfaceList[0].NetworkInterface.IfRecycleCount = 0;
    config.Endpoint.DstIf.Ipv6InterfaceList[0].NetworkInterface.TotalCount = 1;
    config.Endpoint.DstIf.Ipv6InterfaceList[0].NetworkInterface.BllHandle = 0;
    config.Endpoint.DstIf.Ipv6InterfaceList[0].NetworkInterface.AffiliatedInterface = 0;
    memset(config.Endpoint.DstIf.Ipv6InterfaceList[0].Address.address, 0, 16);
    config.Endpoint.DstIf.Ipv6InterfaceList[0].Address.address[15] = 1;
    memset(config.Endpoint.DstIf.Ipv6InterfaceList[0].AddrStep.address, 0, 16);
    config.Endpoint.DstIf.Ipv6InterfaceList[0].SkipReserved = false;
    config.Endpoint.DstIf.Ipv6InterfaceList[0].AddrRepeatCount = 0;
    config.Endpoint.DstIf.Ipv6InterfaceList[0].PrefixLength = 127;

    memset(config.ProtocolProfile.ProxyIpv6Addr.address, 0, 16);
    config.ProtocolProfile.ProxyIpv6Addr.address[15] = 1;
    config.MobileProfile.MobileSignalingChoice = 0;

    return uaConfig;
}
    
};
