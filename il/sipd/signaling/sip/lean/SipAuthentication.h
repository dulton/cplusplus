/// @file
/// @brief SIP Proxy Authentication header file
///
///  Copyright (c) 2008 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _SIP_PROXY_AUTHENTICATION_H_
#define _SIP_PROXY_AUTHENTICATION_H_

#include <stdint.h>
#include <string>

#include "SipCommon.h"

DECL_SIPLEAN_NS

///////////////////////////////////////////////////////////////////////////////

enum SipAuthAlgorithm
{
    SIP_AUTH_NONE = 0,
    SIP_AUTH_MD5,
    SIP_AUTH_MD5_SESS
};

///////////////////////////////////////////////////////////////////////////////

enum SipQualityOfProtection
{
    SIP_QOP_NONE = 0,
    SIP_QOP_AUTH = 1,
    SIP_QOP_AUTH_INT = 2
};

///////////////////////////////////////////////////////////////////////////////

struct SipDigestAuthChallenge
{
    SipDigestAuthChallenge(void)
    {
        Reset();
    }
    
    void Reset(void)
    {
        realm.clear();
        domain.clear();
        nonce.clear();
        opaque.clear();
        stale = false;
        algorithm = SIP_AUTH_NONE;
        qop = SIP_QOP_NONE;
    }
    
    std::string realm;
    std::string domain;
    std::string nonce;
    std::string opaque;
    bool stale;
    SipAuthAlgorithm algorithm;
    SipQualityOfProtection qop;
};

///////////////////////////////////////////////////////////////////////////////

struct SipDigestAuthCredentials
{
    SipDigestAuthCredentials(void)
    {
        Reset();
    }
    
    void Reset(void)
    {
        username.clear();
        realm.clear();
        nonce.clear();
        uri.clear();
        response.clear();
        opaque.clear();
        algorithm = SIP_AUTH_NONE;
        qop = SIP_QOP_NONE;
    }
    
    std::string username;
    std::string realm;
    std::string nonce;
    std::string uri;
    std::string response;
    SipAuthAlgorithm algorithm;
    std::string cnonce;
    size_t nonceCount;
    std::string opaque;
    SipQualityOfProtection qop;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_SIPLEAN_NS

#endif
