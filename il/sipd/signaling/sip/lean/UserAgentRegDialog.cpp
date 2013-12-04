/// @file
/// @brief SIP User Agent (UA) Registration Dialog object
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <algorithm>
#include <list>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <string_token_iterator.h>

#include "VoIPLog.h"
#include "VoIPUtils.h"
#include "SipMessage.h"
#include "SipProtocol.h"
#include "SipUtils.h"
#include "UserAgentBinding.h"
#include "UserAgentRegDialog.h"

USING_SIP_NS;
USING_SIPLEAN_NS;

///////////////////////////////////////////////////////////////////////////////

UserAgentRegDialog::UserAgentRegDialog(const uint16_t port, UserAgentBinding& binding, const ACE_INET_Addr& peer, const std::string& domain, ACE_Reactor& reactor)
    : UserAgentDialog(port, binding, peer, binding.addressOfRecord),
      mDomain(domain),
      mRefresh(false),
      mActive(false),
      mRegExpires(0),
      mRefreshTimer(boost::bind(&UserAgentRegDialog::RefreshTimerExpired, this), &reactor),
      mNotifiedComplete(false)
{
}

///////////////////////////////////////////////////////////////////////////////

UserAgentRegDialog::~UserAgentRegDialog()
{
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentRegDialog::StartRegister(uint32_t regExpires)
{
    mRegExpires = regExpires;
    mNotifiedComplete = false;
    mActive = SendRegister();
    return mActive;
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentRegDialog::Abort(void)
{
    if (mActive)
    {
        // TODO: send CANCEL?
        mActive = false;
    }
    
    mRefreshTimer.Stop();
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentRegDialog::ProcessRequest(const SipMessage& msg)
{
    throw EPHXInternal("UserAgentRegDialog::ProcessRequest");
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentRegDialog::ProcessResponse(const SipMessage& msg)
{
    if (msg.method != SipMessage::SIP_METHOD_REGISTER)
        throw EPHXInternal("UserAgentRegDialog::ProcessResponse: unexpected method " + boost::lexical_cast<std::string>(msg.method));

    switch (msg.status.Category())
    {
      case SipMessage::SIP_STATUS_PROVISIONAL:
          break;
          
      case SipMessage::SIP_STATUS_SUCCESS:
          UpdateExtensionHeaders(msg);

          if (!mNotifiedComplete && mNotifyComplete)
          {
              mNotifiedComplete = true;
              mNotifyComplete(mRegExpires, true);
          }

          if (mRefresh && mRegExpires)
              mRefreshTimer.Start(ACE_Time_Value(0, mRegExpires * 500000));  // mRegExpires / 2.0
          else
              mActive = false;
          break;
          
      default:
          if (((msg.status.code == 401 && WWWAuthenticateChallenge(msg)) || (msg.status.code == 407 && ProxyAuthenticateChallenge(msg))) && SendRegister())
              break;

          mActive = false;

          if (!mNotifiedComplete && mNotifyComplete)
          {
              mNotifiedComplete = true;
              mNotifyComplete(mRegExpires, false);
          }

          break;
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentRegDialog::TransactionError(void)
{
    mActive = false;

    if (!mNotifiedComplete && mNotifyComplete)
    {
        mNotifiedComplete = true;
        mNotifyComplete(mRegExpires, false);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentRegDialog::SendRegister(void)
{
    SipMessage msg;
    msg.peer = Peer();
    msg.type = SipMessage::SIP_MSG_REQUEST;
    msg.compactHeaders = CompactHeaders();
    msg.fromTag = FromTag();
    msg.toTag = ToTag();
    msg.method = SipMessage::SIP_METHOD_REGISTER;
    msg.seq = NextSeq();
    msg.request.uri = SipUri(mDomain, 0);
    msg.request.version = SipMessage::SIP_VER_2_0;

    const std::string branch(SipProtocol::BRANCH_PREFIX + VoIPUtils::MakeRandomHexString(SipProtocol::BRANCH_RANDOMNESS));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_VIA, SipProtocol::BuildViaString(Contact(), &branch)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_TO, SipProtocol::BuildAddressString(FromUri())));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_FROM, SipProtocol::BuildAddressString(FromUri(), &FromTag())));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CALL_ID, CallId()));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CSEQ, SipProtocol::BuildCSeqString(msg.seq, msg.method)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_MAX_FORWARDS, boost::lexical_cast<std::string>(SipProtocol::DEFAULT_MAX_FORWARDS)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_EXPIRES, boost::lexical_cast<std::string>(mRegExpires)));
    std::list<std::string> contactAddressList;
    ForEachContactAddress(boost::bind(&std::list<std::string>::push_back, &contactAddressList, boost::bind(&SipProtocol::BuildAddressString, _1, (const std::string *) 0)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTACT, SipUtils::BuildDelimitedList(contactAddressList, ",")));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_ALLOW, ALLOWED_METHODS));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_SUPPORTED, SipProtocol::TOKEN_PATH));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTENT_LENGTH, "0")); 

    TC_LOG_INFO_LOCAL(Port(), LOG_SIP, "[UserAgentRegDialog::StartRegister] " << Contact() << " sending REGISTER with expiration " << mRegExpires);
    return SendRequestInternal(msg);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentRegDialog::UpdateExtensionHeaders(const SipMessage& msg)
{
    PathList().clear();
    const SipMessage::HeaderField *pathHdr = msg.FindHeader(SipMessage::SIP_HEADER_PATH);
    if (pathHdr)
        std::copy(string_token_iterator(pathHdr->value, ","), string_token_iterator(), std::front_inserter(PathList()));

    ServiceRouteList().clear();
    const SipMessage::HeaderField *serviceRouteHdr = msg.FindHeader(SipMessage::SIP_HEADER_SERVICE_ROUTE);
    if (serviceRouteHdr)
        std::copy(string_token_iterator(serviceRouteHdr->value, ","), string_token_iterator(), std::front_inserter(ServiceRouteList()));

    const SipMessage::HeaderField *assocUriHdr = msg.FindHeader(SipMessage::SIP_HEADER_P_ASSOCIATED_URI);
    if (assocUriHdr)
        AssocUriList().assign(string_token_iterator(assocUriHdr->value, ","), string_token_iterator());
    else
        AssocUriList().clear();
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentRegDialog::RefreshTimerExpired(void)
{
    TC_LOG_INFO_LOCAL(Port(), LOG_SIP, "[UserAgentRegDialog::RefreshTimerExpired] " << Contact() << " auto-refreshing registration");
    RefreshFromTag();
    SendRegister();
}

///////////////////////////////////////////////////////////////////////////////
    
