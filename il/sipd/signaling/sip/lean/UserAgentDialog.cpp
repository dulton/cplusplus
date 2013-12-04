/// @file
/// @brief SIP User Agent (UA) Dialog object
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <memory>

#include <boost/assign/list_of.hpp>
#include <string_token_iterator.h>

#include "SipleanAuthentication.h"
#include "SipMessage.h"
#include "SipProtocol.h"
#include "SipUtils.h"
#include "UserAgentDialog.h"

#include "VoIPUtils.h"

USING_SIP_NS;
USING_SIPLEAN_NS;

///////////////////////////////////////////////////////////////////////////////

const std::string UserAgentDialog::ALLOWED_METHODS = SipProtocol::BuildMethodListString(boost::assign::list_of(SipMessage::SIP_METHOD_INVITE)
                                                                                                              (SipMessage::SIP_METHOD_PRACK)
                                                                                                              (SipMessage::SIP_METHOD_ACK)
                                                                                                              (SipMessage::SIP_METHOD_CANCEL)
                                                                                                              (SipMessage::SIP_METHOD_BYE)
                                                                                                              (SipMessage::SIP_METHOD_UPDATE));

///////////////////////////////////////////////////////////////////////////////

UserAgentDialog::UserAgentDialog(uint16_t port, UserAgentBinding& binding, const ACE_INET_Addr& peer, const SipUri& toUri)
    : mPort(port),
      mBinding(binding),
      mFromTag(VoIPUtils::MakeRandomHexString(SipProtocol::TAG_RANDOMNESS)),
      mPeer(peer), 
      mToUri(toUri),
      mRemoteTarget(toUri),
      mCallId(SipProtocol::BuildCallIdString(FromUri().GetSipHost(), SipProtocol::CALL_ID_RANDOMNESS)),
      mLocalSeq(0),
      mRemoteSeq(0),
      mCompactHeaders(false)
{
}

///////////////////////////////////////////////////////////////////////////////

UserAgentDialog::UserAgentDialog(uint16_t port, UserAgentBinding& binding, const SipMessage& msg)
    : mPort(port),
      mBinding(binding),
      mFromTag(VoIPUtils::MakeRandomHexString(SipProtocol::TAG_RANDOMNESS)),
      mPeer(msg.peer),
      mToUri(msg.fromUri),
      mFromUri(msg.toUri),
      mToTag(msg.fromTag),
      mCallId(msg.callId),
      mLocalSeq(0),
      mRemoteSeq(msg.seq),
      mCompactHeaders(false)
{
    // Pull remote target out of Contact header
    std::vector<SipMessage::HeaderField>::const_reverse_iterator iter = std::find_if(msg.headers.rbegin(), msg.headers.rend(), SipMessage::IsHeaderType(SipMessage::SIP_HEADER_CONTACT));
    if (iter == msg.headers.rend() || !SipProtocol::ParseAddressString(iter->value, &mRemoteTarget))
        throw EPHXInternal("UserAgentDialog::UserAgentDialog: missing or invalid Contact header");
}

///////////////////////////////////////////////////////////////////////////////

UserAgentDialog::~UserAgentDialog()
{
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentDialog::Matches(const SipMessage& msg) const
{
    if (!Active())
        return false;
    
    if (msg.type == SipMessage::SIP_MSG_STATUS)
    {
        const bool earlyDialog = ToTag().empty();
        return (msg.fromTag == FromTag()) && (earlyDialog || (msg.toTag == ToTag())) && (msg.callId == CallId());
    }
    else  // msg.type == SipMessage::SIP_MSG_REQUEST
    {
        return (msg.toTag == FromTag()) && (msg.fromTag == ToTag()) && (msg.callId == CallId());
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentDialog::UpdateTarget(const SipMessage& msg)
{
    if (msg.type != SipMessage::SIP_MSG_STATUS)
        throw EPHXInternal("UserAgentDialog::UpdateTarget");

    // Update remote URI and tag
    mToUri = msg.toUri;
    mToTag = msg.toTag;
        
    // Pull remote target out of Contact header
    std::vector<SipMessage::HeaderField>::const_reverse_iterator iter = std::find_if(msg.headers.rbegin(), msg.headers.rend(), SipMessage::IsHeaderType(SipMessage::SIP_HEADER_CONTACT));
    if (iter != msg.headers.rend())
        SipProtocol::ParseAddressString(iter->value, &mRemoteTarget);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentDialog::RefreshFromTag(void)
{
    mFromTag =VoIPUtils::MakeRandomHexString(SipProtocol::TAG_RANDOMNESS);
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentDialog::WWWAuthenticateChallenge(const SipMessage& msg)
{
    if (msg.type != SipMessage::SIP_MSG_STATUS || msg.status.code != 401)
        throw EPHXInternal("UserAgentDialog::WWWAuthenticateChallenge");

    // Parse WWW auth challenge
    UserAgentBinding::AuthCredentials& authCred = mBinding.authCred;
    const SipMessage::HeaderField *wwwAuthHdr = msg.FindHeader(SipMessage::SIP_HEADER_WWW_AUTHENTICATE);
    std::auto_ptr<SipDigestAuthChallenge> chal(new SipDigestAuthChallenge);
    if (!wwwAuthHdr || !SipProtocol::ParseDigestAuthChallenge(wwwAuthHdr->value, *chal))
        return false;

    // We can respond if we have credentials in the realm (if our realm is empty we assume a realm match)
    if (authCred.realm.empty() || authCred.realm == chal->realm)
    {
        // Avoid loops if we have a bad username/password combo        
        if (!authCred.wwwChal || chal->stale)
        {
            authCred.wwwChal.reset(chal.release());
            return true;
        }
    }
    
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentDialog::WWWAuthenticate(SipMessage& msg) const
{
    if (msg.type != SipMessage::SIP_MSG_REQUEST)
        throw EPHXInternal("UserAgentDialog::WWWAuthenticate");

    if (!mBinding.authCred.wwwChal)
        return;
    
    // Generate MD5 digest for this specific msg
    UserAgentBinding::AuthCredentials& authCred = mBinding.authCred;
    SipDigestAuthCredentials cred;
    SipleanAuthentication::Md5Digest(*authCred.wwwChal, msg, authCred.username, authCred.password, cred);
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_AUTHORIZATION, SipProtocol::BuildDigestAuthCredentialsString(cred)));

    // Send plaintext password for debug purposes
    if (mBinding.authCred.passwordDebug && !msg.FindHeader(SipMessage::SIP_HEADER_X_PASSWORD_DEBUG))
        msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_X_PASSWORD_DEBUG, authCred.password));
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentDialog::ProxyAuthenticateChallenge(const SipMessage& msg)
{
    if (msg.type != SipMessage::SIP_MSG_STATUS || msg.status.code != 407)
        throw EPHXInternal("UserAgentDialog::ProxyAuthenticateChallenge");

    // Parse proxy auth challenge
    UserAgentBinding::AuthCredentials& authCred = mBinding.authCred;
    const SipMessage::HeaderField *proxyAuthHdr = msg.FindHeader(SipMessage::SIP_HEADER_PROXY_AUTHENTICATE);
    std::auto_ptr<SipDigestAuthChallenge> chal(new SipDigestAuthChallenge);
    if (!proxyAuthHdr || !SipProtocol::ParseDigestAuthChallenge(proxyAuthHdr->value, *chal))
        return false;

    // We can respond if we have credentials in the realm (if our realm is empty we assume a realm match)
    if (authCred.realm.empty() || authCred.realm == chal->realm)
    {
        // Avoid loops if we have a bad username/password combo        
        if (!authCred.proxyChal || chal->stale)
        {
            authCred.proxyChal.reset(chal.release());
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentDialog::ProxyAuthenticate(SipMessage& msg) const
{
    if (msg.type != SipMessage::SIP_MSG_REQUEST)
        throw EPHXInternal("UserAgentDialog::ProxyAuthenticate");

    if (!mBinding.authCred.proxyChal)
        return;
    
    // Generate MD5 digest for this specific msg
    UserAgentBinding::AuthCredentials& authCred = mBinding.authCred;
    SipDigestAuthCredentials cred;
    SipleanAuthentication::Md5Digest(*authCred.proxyChal, msg, authCred.username, authCred.password, cred);
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_PROXY_AUTHORIZATION, SipProtocol::BuildDigestAuthCredentialsString(cred)));

    // Send plaintext password for debug purposes
    if (mBinding.authCred.passwordDebug && !msg.FindHeader(SipMessage::SIP_HEADER_X_PASSWORD_DEBUG))
        msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_X_PASSWORD_DEBUG, authCred.password));
}

///////////////////////////////////////////////////////////////////////////////
