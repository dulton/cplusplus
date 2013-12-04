/// @file
/// @brief SIP protocol header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _SIP_PROTOCOL_H_
#define _SIP_PROTOCOL_H_

#include <string>
#include <utility>
#include <vector>

#include "SipCommon.h"
#include "SipMessage.h"
#include "SipSessionExpiration.h"

// Forward declarations (global)
class Packet;

DECL_CLASS_FORWARD_SIP_NS(SipUri);

DECL_SIPLEAN_NS

class SipDigestAuthChallenge;
class SipDigestAuthCredentials;

///////////////////////////////////////////////////////////////////////////////

// Forward declarations
class SdpMessage;

namespace SipProtocol
{
    const std::string TOKEN_PATH("path");
    const std::string TOKEN_100REL("100rel");
    const std::string TOKEN_TIMER("timer");
    const std::string BRANCH_PREFIX("z9hG4bK");
    const size_t TAG_RANDOMNESS = 8;
    const size_t BRANCH_RANDOMNESS = 32;
    const size_t CALL_ID_RANDOMNESS = 32;
    const uint32_t DEFAULT_MAX_FORWARDS = 70;
    
    bool ParseUriString(const std::string& uriStr, SipUri& uri);

    const std::string BuildViaString(const SipUri& uri, const std::string *branch = 0);
    bool ParseViaString(const std::string& viaStr, SipUri& uri, std::string *branch = 0);
    
    const std::string BuildAddressString(const SipUri& uri, const std::string *tag = 0);
    bool ParseAddressString(const std::string& nameAddrStr, SipUri *uri = 0, std::string *tag = 0);

    const std::string BuildCallIdString(const std::string& host, size_t randomness);

    const std::string BuildCSeqString(uint32_t cseq, SipMessage::MethodType method);
    bool ParseCSeqString(const std::string& cseqStr, uint32_t& cseq, SipMessage::MethodType& method);

    const std::string BuildRAckString(uint32_t rseq, uint32_t cseq, const SipMessage::MethodType method);
    bool ParseRAckString(const std::string& rackStr, uint32_t& rseq, uint32_t& cseq, SipMessage::MethodType& method);
    
    const std::string BuildMethodString(const SipMessage::MethodType& method);
    const std::string BuildMethodListString(const std::vector<SipMessage::MethodType>& methodTypeVec);

    const std::string BuildSessionExpirationString(const SipSessionExpiration& se);
    bool ParseSessionExpirationString(const std::string& seStr, SipSessionExpiration& se);

    bool ParseDigestAuthChallenge(const std::string& chalStr, SipDigestAuthChallenge& chal);
    const std::string BuildDigestAuthCredentialsString(const SipDigestAuthCredentials& cred);
    
    const std::string BuildSdpString(const SdpMessage& sdp);
    bool ParseSdpString(const std::string& sdpStr, SdpMessage& sdp);
    
    bool BuildPacket(const SipMessage& msg, Packet& packet);
    bool ParsePacket(const Packet& packet, SipMessage& msg);
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_SIPLEAN_NS

#endif
