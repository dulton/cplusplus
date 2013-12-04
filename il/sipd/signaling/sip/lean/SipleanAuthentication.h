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

#ifndef _SIP_PROXY_LEANAUTHENTICATION_H_
#define _SIP_PROXY_LEANAUTHENTICATION_H_

#include <stdint.h>
#include <string>

#include "SipCommon.h"

#include "SipAuthentication.h"

// Forward declarations

DECL_SIPLEAN_NS

class SipMessage;

///////////////////////////////////////////////////////////////////////////////

namespace SipleanAuthentication
{
    const size_t CNONCE_RANDOMNESS = 8;
    
    void Md5Digest(const SipDigestAuthChallenge& chal, const SipMessage& msg, const std::string& username, 
		   const std::string& password, SipDigestAuthCredentials& cred, const std::string* cnonce = 0);
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_SIPLEAN_NS

#endif
