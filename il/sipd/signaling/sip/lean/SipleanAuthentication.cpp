/// @file
/// @brief SIP authentication implementation
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <sstream>

#include <boost/format.hpp>
#include <phxexception/PHXException.h>

#include <utils/md5.h>
#include "SipleanAuthentication.h"
#include "SipMessage.h"
#include "SipProtocol.h"
#include "SipUtils.h"

#include "VoIPUtils.h"

USING_SIP_NS;

DECL_SIPLEAN_NS

namespace SipleanAuthentication
{

///////////////////////////////////////////////////////////////////////////////

inline void Md5DigestToHexString(const md5_byte_t *digest, std::string& str)
{
    std::ostringstream oss;
    for (size_t i = 0; i < 16; i++)
        oss << boost::format("%02x") % (int) digest[i];
    str = oss.str();
}
    
///////////////////////////////////////////////////////////////////////////////
    
static void Md5DigestCalcHA1(SipAuthAlgorithm algorithm, const std::string& username, const std::string& realm, const std::string& password, const std::string& nonce, const::std::string& cnonce, std::string& ha1)
{
    static const char *colon = ":";

    // Calculate H(A1) per RFC 2617, Section 3.2.2.2
    md5_state_t state;
    md5_byte_t digest[16];
    
    md5_init(&state);
    md5_append(&state, (const md5_byte_t *) username.data(), username.size());
    md5_append(&state, (const md5_byte_t *) colon, 1);
    md5_append(&state, (const md5_byte_t *) realm.data(), realm.size());
    md5_append(&state, (const md5_byte_t *) colon, 1);
    md5_append(&state, (const md5_byte_t *) password.data(), password.size());
    md5_finish(&state, digest);
    Md5DigestToHexString(digest, ha1);
    
    if (algorithm == SIP_AUTH_MD5_SESS)
    {
        md5_init(&state);
        md5_append(&state, (const md5_byte_t *) ha1.data(), ha1.size());
        md5_append(&state, (const md5_byte_t *) colon, 1);
        md5_append(&state, (const md5_byte_t *) nonce.data(), nonce.size());
        md5_append(&state, (const md5_byte_t *) colon, 1);
        md5_append(&state, (const md5_byte_t *) cnonce.data(), cnonce.size());
        md5_finish(&state, digest);
        Md5DigestToHexString(digest, ha1);
    }
}
    
///////////////////////////////////////////////////////////////////////////////
    
static void Md5DigestCalcHEntityBody(const std::string& entityBody, std::string& hEntityBody)
{
    // Calculate H(entity-body)
    md5_state_t state;
    md5_byte_t digest[16];
    
    md5_init(&state);
    if (!entityBody.empty())
        md5_append(&state, (const md5_byte_t *) entityBody.data(), entityBody.size());
    md5_finish(&state, digest);
    Md5DigestToHexString(digest, hEntityBody);
}
    
///////////////////////////////////////////////////////////////////////////////

static void Md5DigestCalcResponse(const std::string& ha1, const std::string& nonce, size_t nonceCount, const::std::string& cnonce, SipQualityOfProtection qop, const std::string& method, const std::string& uri, const std::string& entityDigest, std::string& response)
{
    static const char *colon = ":";

    // Calculate H(A2) per RFC 2617, Section 3.2.2.3
    md5_state_t state;
    md5_byte_t digest[16];
    std::string ha2;
    
    md5_init(&state);
    md5_append(&state, (const md5_byte_t *) method.data(), method.size());
    md5_append(&state, (const md5_byte_t *) colon, 1);
    md5_append(&state, (const md5_byte_t *) uri.data(), uri.size());
    if (qop == SIP_QOP_AUTH_INT)
    {
        md5_append(&state, (const md5_byte_t *) colon, 1);
        md5_append(&state, (const md5_byte_t *) entityDigest.data(), entityDigest.size());
    }
    md5_finish(&state, digest);
    Md5DigestToHexString(digest, ha2);

    // Calculate the response per RFC 2617, Section 3.2.2.1
    md5_init(&state);
    md5_append(&state, (const md5_byte_t *) ha1.data(), ha1.size());
    md5_append(&state, (const md5_byte_t *) colon, 1);
    md5_append(&state, (const md5_byte_t *) nonce.data(), nonce.size());
    md5_append(&state, (const md5_byte_t *) colon, 1);
    if (qop != SIP_QOP_NONE)
    {
        const std::string nonceCountStr = (boost::format("%08x") % nonceCount).str();
        std::string qopStr;

        switch (qop)
        {
          case SIP_QOP_AUTH:
              qopStr = "auth";
              break;
              
          case SIP_QOP_AUTH_INT:
              qopStr = "auth-int";
              break;

          default:
              throw EPHXInternal("SipleanAuthentication::Md5DigestCalcResponse: invalid qop");
        }
        
        md5_append(&state, (const md5_byte_t *) nonceCountStr.data(), nonceCountStr.size());
        md5_append(&state, (const md5_byte_t *) colon, 1);
        md5_append(&state, (const md5_byte_t *) cnonce.data(), cnonce.size());
        md5_append(&state, (const md5_byte_t *) colon, 1);
        md5_append(&state, (const md5_byte_t *) qopStr.data(), qopStr.size());
        md5_append(&state, (const md5_byte_t *) colon, 1);
    }
    md5_append(&state, (const md5_byte_t *) ha2.data(), ha2.size());
    md5_finish(&state, digest);
    Md5DigestToHexString(digest, response);
}
    
///////////////////////////////////////////////////////////////////////////////

void Md5Digest(const SipDigestAuthChallenge& chal, const SipMessage& msg, const std::string& username, 
	       const std::string& password, SipDigestAuthCredentials& cred, const std::string* cnonce)
{
    if (msg.type != SipMessage::SIP_MSG_REQUEST)
        throw EPHXInternal("SipleanAuthentication::Md5Digest");
        
    cred.username = username;
    cred.realm = chal.realm;

    if (cred.nonce != chal.nonce)
    {
        cred.nonce = chal.nonce;
        cred.nonceCount = 1;
    }
    else
        cred.nonceCount++;

    cred.uri = msg.request.uri.BuildUriString();
    cred.algorithm = chal.algorithm;
    cred.cnonce = cnonce ? *cnonce : VoIPUtils::MakeRandomHexString(CNONCE_RANDOMNESS);
    cred.opaque = chal.opaque;

    // Favor qop=auth-int over qop=auth when both are available
    if (chal.qop & SIP_QOP_AUTH_INT)
        cred.qop = SIP_QOP_AUTH_INT;
    else if (chal.qop & SIP_QOP_AUTH)
        cred.qop = SIP_QOP_AUTH;

    std::string ha1;
    Md5DigestCalcHA1(cred.algorithm, cred.username, cred.realm, password, cred.nonce, cred.cnonce, ha1);
    
    std::string ha2;
    Md5DigestCalcHEntityBody(msg.body, ha2);
    Md5DigestCalcResponse(ha1, cred.nonce, cred.nonceCount, cred.cnonce, cred.qop, SipProtocol::BuildMethodString(msg.method), cred.uri, ha2, cred.response);
}
    
///////////////////////////////////////////////////////////////////////////////

};

END_DECL_SIPLEAN_NS
    
