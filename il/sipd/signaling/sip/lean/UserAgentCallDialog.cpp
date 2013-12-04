/// @file
/// @brief SIP User Agent (UA) Call Dialog object
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
#include <sstream>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <string_token_iterator.h>

#include "SdpMessage.h"
#include "VoIPUtils.h"
#include "VoIPLog.h"
#include "SipMessage.h"
#include "SipProtocol.h"
#include "SipSessionExpiration.h"
#include "SipUtils.h"
#include "UserAgentCallDialog.h"

USING_SIP_NS;
USING_SIPLEAN_NS;

///////////////////////////////////////////////////////////////////////////////

#define _DEFINE_USER_AGENT_CALL_DIALOG_STATES_
#include "UserAgentCallDialogState.h"

///////////////////////////////////////////////////////////////////////////////

const uint32_t UserAgentCallDialog::MIN_SESSION_EXPIRES_TIME = 90;        // 90 seconds
const uint32_t UserAgentCallDialog::DEFAULT_SESSION_EXPIRES_TIME = 1800;  // 30 minutes

const ACE_Time_Value UserAgentCallDialog::TIMER_T1(0, 500000);
const ACE_Time_Value UserAgentCallDialog::TIMER_T2(4, 0);

///////////////////////////////////////////////////////////////////////////////

UserAgentCallDialog::UserAgentCallDialog(const uint16_t port, UserAgentBinding& binding, const ACE_INET_Addr& peer, 
					 const SipUri& toUri, ACE_Reactor& reactor,bool usingProxy)
    : UserAgentDialog(port, binding, peer, toUri),
      mCaller(true),
      mInviteSeq(0),
      mSessionExpires(DEFAULT_SESSION_EXPIRES_TIME),
      mMinSessionExpires(DEFAULT_SESSION_EXPIRES_TIME),
      mSessionRefresher(false),
      mLooseRouting(ALWAYS_USE_LOOSE_ROUTING_ON_UE),
      mState(&CALLING_STATE),
      mRingTimer(boost::bind(&UserAgentCallDialog::TimerExpired, this, RING_TIMER), &reactor),
      m100RelTimer(boost::bind(&UserAgentCallDialog::TimerExpired, this, _100REL_TIMER), &reactor),
      mMediaTimer(boost::bind(&UserAgentCallDialog::TimerExpired, this, MEDIA_TIMER), &reactor),
      mAckWaitTimer(boost::bind(&UserAgentCallDialog::TimerExpired, this, ACK_WAIT_TIMER), &reactor),
      mSessionTimer(boost::bind(&UserAgentCallDialog::TimerExpired, this, SESSION_TIMER), &reactor),
      mCallTimer(boost::bind(&UserAgentCallDialog::TimerExpired, this, CALL_TIMER), &reactor),
      mDefaultRefresherOption(DEFAULT_UAC_REFRESHER),
      mUsingProxy(usingProxy)
{
}

///////////////////////////////////////////////////////////////////////////////

UserAgentCallDialog::UserAgentCallDialog(const uint16_t port, UserAgentBinding& binding, const SipMessage& msg, 
					 ACE_Reactor& reactor,bool usingProxy)
    : UserAgentDialog(port, binding, msg),
      mCaller(false),
      mInviteSeq(0),
      mSessionExpires(DEFAULT_SESSION_EXPIRES_TIME),
      mMinSessionExpires(DEFAULT_SESSION_EXPIRES_TIME),
      mSessionRefresher(false),
      mLooseRouting(ALWAYS_USE_LOOSE_ROUTING_ON_UE),
      mState(&ANSWERING_STATE),
      mRingTimer(boost::bind(&UserAgentCallDialog::TimerExpired, this, RING_TIMER), &reactor),
      m100RelTimer(boost::bind(&UserAgentCallDialog::TimerExpired, this, _100REL_TIMER), &reactor),
      mMediaTimer(boost::bind(&UserAgentCallDialog::TimerExpired, this, MEDIA_TIMER), &reactor),
      mAckWaitTimer(boost::bind(&UserAgentCallDialog::TimerExpired, this, ACK_WAIT_TIMER), &reactor),
      mSessionTimer(boost::bind(&UserAgentCallDialog::TimerExpired, this, SESSION_TIMER), &reactor),
      mCallTimer(boost::bind(&UserAgentCallDialog::TimerExpired, this, CALL_TIMER), &reactor),
      mDefaultRefresherOption(DEFAULT_UAC_REFRESHER),
      mUsingProxy(usingProxy)
{
    if (msg.type != SipMessage::SIP_MSG_REQUEST || msg.method != SipMessage::SIP_METHOD_INVITE)
        throw EPHXInternal("UserAgentCallDialog::UserAgentCallDialog: expected INVITE request");
}

///////////////////////////////////////////////////////////////////////////////

UserAgentCallDialog::~UserAgentCallDialog()
{
    mState->Abort(*this, false);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::SetDefaultRefresherOption(UserAgentCallDialog::DefaultRefresherOptionType value) {
  mDefaultRefresherOption=value;
}

UserAgentCallDialog::DefaultRefresherOptionType UserAgentCallDialog::GetDefaultRefresherOption() {
  return mDefaultRefresherOption;
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::SetMinSessionExpiration(uint32_t minSessionExpires)
{
    mMinSessionExpires = minSessionExpires ? std::max(minSessionExpires, MIN_SESSION_EXPIRES_TIME) : 0;
    mSessionExpires = mMinSessionExpires;
}

///////////////////////////////////////////////////////////////////////////////

const std::string UserAgentCallDialog::BuildRouteString(void) const
{
    if (mRouteList.empty())
        throw EPHXInternal("UserAgentCallDialog::BuildRouteString");

    // See RFC 3261, section 12.2.1.1 for Route value rules
    if (!mLooseRouting)
    {
        routeList_t::const_reverse_iterator endIter = mRouteList.rbegin();
        if (endIter != mRouteList.rend())
            ++endIter;

        std::ostringstream oss;
        SipUtils::string_range_t routeRange(mRouteList.begin(), endIter.base());
        if (routeRange)
            std::copy(routeRange.begin(), routeRange.end(), std::ostream_iterator<std::string>(oss, ","));
        oss << "<" << RemoteTarget().BuildUriString() << ">";
        return oss.str();
    }
    else
        return SipUtils::BuildDelimitedList(mRouteList, ",");
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::UpdateDialog(const SipMessage& msg)
{
    switch (msg.method)
    {
      case SipMessage::SIP_METHOD_INVITE:
        // Update dialog target
        if (msg.type == SipMessage::SIP_MSG_STATUS)
            UpdateTarget(msg);

        // Update session expiration
        UpdateSessionExpiration(msg);

        // Update the route list
        UpdateRouteList(msg);
        break;

      case SipMessage::SIP_METHOD_UPDATE:
        // Update session expiration
        UpdateSessionExpiration(msg);
        break;

      default:
        throw EPHXInternal("UserAgentCallDialog::UpdateDialog");
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::UpdateSessionExpiration(const SipMessage& msg)
{
    // Assume session refresh is off unless we find headers that tell us otherwise
    mSessionExpires = 0;
    mSessionRefresher = false;

    // Find instances of 'timer' in Required and Supported headers
    boost::function1<bool, const std::string&> tokenComparator = (boost::bind((int (std::string::*)(const std::string&) const) &std::string::compare, &SipProtocol::TOKEN_TIMER, _1) == 0);
    string_token_iterator end;

    const SipMessage::HeaderField *requireHdr = msg.FindHeader(SipMessage::SIP_HEADER_REQUIRE);
    const bool timerRequired = requireHdr ? (std::find_if(string_token_iterator(requireHdr->value, ","), end, tokenComparator) != end) : false;

    const SipMessage::HeaderField *supportedHdr = msg.FindHeader(SipMessage::SIP_HEADER_SUPPORTED);
    const bool timerSupported = supportedHdr ? (std::find_if(string_token_iterator(supportedHdr->value, ","), end, tokenComparator) != end) : false;

    if (msg.type == SipMessage::SIP_MSG_STATUS)
    {
        // Process 2xx response, see RFC 4028, Section 7.2
        if (msg.status.Category() == SipMessage::SIP_STATUS_SUCCESS)
        {
            // If response doesn't include 'timer' in the Required header, we have nothing to do
            if (!timerRequired)
                return;

            // Session-Expires with refresher parameter must be specified in the response
            const SipMessage::HeaderField *sessionExpiresHdr = msg.FindHeader(SipMessage::SIP_HEADER_SESSION_EXPIRES);
            if (sessionExpiresHdr)
            {
                SipSessionExpiration sessionExpires;
                if (!SipProtocol::ParseSessionExpirationString(sessionExpiresHdr->value, sessionExpires))
                    return;

                mSessionExpires = sessionExpires.deltaSeconds;
                mSessionRefresher = (sessionExpires.refresher == SipSessionExpiration::UAC_REFRESHER);
            }
        }
        // Process 422 response, see RFC 4028, Section 7.3
        else if (msg.status.code == 422)
        {
            const SipMessage::HeaderField *minSessionExpiresHdr = msg.FindHeader(SipMessage::SIP_HEADER_MIN_SE);
            if (minSessionExpiresHdr)
            {
                SipSessionExpiration minSessionExpires;
                if (SipProtocol::ParseSessionExpirationString(minSessionExpiresHdr->value, minSessionExpires))
                {
                    mMinSessionExpires = std::max(mMinSessionExpires, minSessionExpires.deltaSeconds);
                    mSessionExpires = std::max(mSessionExpires, mMinSessionExpires);
                }
            }
        }
    }
    else
    {
        // If request doesn't include 'timer' in a Required or Supported header, we have nothing to do
        if (!timerRequired && !timerSupported)
            return;

        // Parse Session-Expires header
        SipSessionExpiration sessionExpires;
        const SipMessage::HeaderField *sessionExpiresHdr = msg.FindHeader(SipMessage::SIP_HEADER_SESSION_EXPIRES);
        if (sessionExpiresHdr)
        {
            if (!SipProtocol::ParseSessionExpirationString(sessionExpiresHdr->value, sessionExpires))
                return;
        }

        // Impose Min-SE minimums that may be inserted by proxy servers
        const SipMessage::HeaderField *minSessionExpiresHdr = msg.FindHeader(SipMessage::SIP_HEADER_MIN_SE);
        if (minSessionExpiresHdr)
        {
            SipSessionExpiration minSessionExpires;
            if (SipProtocol::ParseSessionExpirationString(minSessionExpiresHdr->value, minSessionExpires))
            {
                mMinSessionExpires = std::max(mMinSessionExpires, minSessionExpires.deltaSeconds);
                sessionExpires.deltaSeconds = std::max(sessionExpires.deltaSeconds, mMinSessionExpires);
            }
        }

        mSessionExpires = sessionExpires.deltaSeconds;
        mSessionRefresher = (sessionExpires.refresher == SipSessionExpiration::SipSessionExpiration::UAS_REFRESHER);

        // Select a refresher if none specified, see RFC 4028, Table 2
        if (UsingSessionRefresh() && sessionExpires.refresher == SipSessionExpiration::NO_REFRESHER)
        {
            if (timerSupported)
            {
	      // The UAC supports refresh but hasn't indicated a refresher preference (nor has any proxy) 
	      //then we get to decide.
	      if(GetDefaultRefresherOption()==DEFAULT_UAS_REFRESHER) {
                mSessionRefresher = !Caller();
	      } else {
		mSessionRefresher = Caller();
	      }
            }
            else
            {
                // The UAC doesn't support refresh but a proxy server has required it, so we must act as the refresher
                mSessionRefresher = true;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::UpdateRouteList(const SipMessage& msg)
{
    mRouteList.clear();
    mLooseRouting = ALWAYS_USE_LOOSE_ROUTING_ON_UE;

    const SipMessage::HeaderField *rrHdr = msg.FindHeader(SipMessage::SIP_HEADER_RECORD_ROUTE);
    if (rrHdr)
    {
        std::copy(string_token_iterator(rrHdr->value, ","), string_token_iterator(), std::front_inserter(mRouteList));
        if (!mRouteList.empty())
        {
            SipUri firstRouteUri;
            mLooseRouting = SipProtocol::ParseAddressString(mRouteList.front(), &firstRouteUri) && firstRouteUri.GetLooseRouting();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::Abort(void)
{
    mState->Abort(*this, false);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::ProcessRequest(const SipMessage& msg)
{
    // Check for correct sequencing (excepting ACK and CANCEL requests whose sequence number is the same as the request they are acking or canceling)
    if (msg.method != SipMessage::SIP_METHOD_ACK && msg.method != SipMessage::SIP_METHOD_CANCEL && !InSeq(msg.seq))
    {
        TC_LOG_INFO_LOCAL(Port(), LOG_SIP, "[UserAgentCallDialog::ProcessRequest] ignoring out-of-sequence request");
        SendResponse(500, "Server Internal Error", &msg);
        return;
    }

    switch (msg.method)
    {
      case SipMessage::SIP_METHOD_INVITE:
      case SipMessage::SIP_METHOD_ACK:
      case SipMessage::SIP_METHOD_UPDATE:
      case SipMessage::SIP_METHOD_BYE:
          mState->ProcessRequest(*this, msg);
          break;

      case SipMessage::SIP_METHOD_PRACK:
          // Handle PRACK requests completely here -- these are never dispatched to the state machine
          if (mHeldStatus)
          {
              const SipMessage::HeaderField *rackHeader = msg.FindHeader(SipMessage::SIP_HEADER_RACK);
              uint32_t rseq, cseq;
              SipMessage::MethodType method;

              if (rackHeader && SipProtocol::ParseRAckString(rackHeader->value, rseq, cseq, method) && mHeldStatus->seq == cseq && mHeldStatus->method == method && rseq >= mRSeqState.first && rseq <= mRSeqState.second)
              {
                  mHeldStatus.reset();
                  m100RelTimer.Stop();
              }
          }
          SendResponse(200, "OK", &msg);
          break;

      case SipMessage::SIP_METHOD_CANCEL:  // TODO -- handle CANCEL
      default:
          SendResponse(405, "Method Not Allowed", &msg);
          break;
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::ProcessResponse(const SipMessage& msg)
{
    // Toss PRACK responses immediately -- these are never dispatched to the state machine
    if (msg.method == SipMessage::SIP_METHOD_PRACK)
        return;

    bool deliver100Rel = false;
    if (msg.status.Category() == SipMessage::SIP_STATUS_PROVISIONAL && msg.status.code != 100 && mRAckState.first)
    {
        bool using100Rel = false;

        const SipMessage::HeaderField *requireHdr = msg.FindHeader(SipMessage::SIP_HEADER_REQUIRE);
        if (requireHdr)
        {
            boost::function1<bool, const std::string&> tokenComparator = (boost::bind((int (std::string::*)(const std::string&) const) &std::string::compare, &SipProtocol::TOKEN_100REL, _1) == 0);
            string_token_iterator end;
            using100Rel = (std::find_if(string_token_iterator(requireHdr->value, ","), end, tokenComparator) != end);
        }

        if (using100Rel)
        {
            // This provisional response is being reliably delivered and may require a PRACK
            // Note: This check hardcodes the assumption that we only support 100rel for INVITE requests.  If that assumption changes in the
            // future this code will need to be revisited.  For now it isn't worth hanging on to that additional request state.
            const SipMessage::HeaderField *rseqHdr = msg.FindHeader(SipMessage::SIP_HEADER_RSEQ);
            if (rseqHdr && msg.method == SipMessage::SIP_METHOD_INVITE && msg.seq == InviteSeq())
            {
                const uint32_t rseq = boost::lexical_cast<uint32_t>(rseqHdr->value);
                if (rseq > mRAckState.second)
                {
                    mRAckState.second = rseq;
                    deliver100Rel = true;
                }
            }
        }
    }

    switch (msg.status.Category())
    {
      case SipMessage::SIP_STATUS_PROVISIONAL:
          // Deliver reliable 1xx response (excepting 100) to state machine
          if (deliver100Rel)
          {
              mState->ProcessResponse(*this, msg);
              SendPrack(mRAckState.second, msg);
          }
          break;

      default:
          // We've received a final response, turn off PRACK
          mRAckState.first = false;

          // Respond to proxy authentication challenges if possible
          if ((msg.status.code == 401 && WWWAuthenticateChallenge(msg)) || (msg.status.code == 407 && ProxyAuthenticateChallenge(msg)))
          {
              bool retryingRequest;

              switch (msg.method)
              {
                case SipMessage::SIP_METHOD_INVITE:
                    retryingRequest = SendInvite();
                    break;

                case SipMessage::SIP_METHOD_UPDATE:
                    retryingRequest = SendUpdate();
                    break;

                default:
                    retryingRequest = false;
                    break;
              }

              // If we're retrying, don't deliver 407 response to state machine
              if (retryingRequest)
                  break;
          }

          mState->ProcessResponse(*this, msg);
          break;
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::TransactionError(void)
{
    mState->TransactionError(*this);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::MediaControlComplete(bool success)
{
    mState->MediaControlComplete(*this, success);
}

void UserAgentCallDialog::MediaControlCompleteDelayed(bool generate, bool success)
{
  if(generate) {
    MediaControlComplete(success);
  } else {
    mMediaLastResult=success;
    ACE_Time_Value mediaDelay(0, 20000);
    mMediaTimer.Start(mediaDelay);
  }
}

///////////////////////////////////////////////////////////////////////////////

const std::string UserAgentCallDialog::BuildFirstRoute() {
  SipUri proxy = RemoteTarget();
  proxy.SetLooseRouting(ALWAYS_USE_LOOSE_ROUTING_ON_UE);
  return proxy.BuildRouteUriString();
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentCallDialog::SendInvite(void)
{
    SipMessage msg;
    msg.peer = Peer();
    msg.type = SipMessage::SIP_MSG_REQUEST;
    msg.compactHeaders = CompactHeaders();
    msg.fromTag = FromTag();
    msg.toTag = ToTag();
    msg.method = SipMessage::SIP_METHOD_INVITE;
    msg.seq = InviteSeq(NextSeq());
    msg.request.uri = RemoteTarget();
    msg.request.version = SipMessage::SIP_VER_2_0;

    const std::string branch(SipProtocol::BRANCH_PREFIX + VoIPUtils::MakeRandomHexString(SipProtocol::BRANCH_RANDOMNESS));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_VIA, SipProtocol::BuildViaString(Contact(), &branch)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_TO, SipProtocol::BuildAddressString(ToUri(), &ToTag())));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_FROM, SipProtocol::BuildAddressString(FromUri(), &FromTag())));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CALL_ID, CallId()));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CSEQ, SipProtocol::BuildCSeqString(msg.seq, msg.method)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_MAX_FORWARDS, boost::lexical_cast<std::string>(SipProtocol::DEFAULT_MAX_FORWARDS)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_SUBJECT, "Spirent TestCenter Invite"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTACT, SipProtocol::BuildAddressString(Contact())));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_USER_AGENT, "STC-SIP/1.0 (www.spirentcom.com)"));
    //If this SIP Agent has "path" list, it means that this is a part of the core infrasture (RFC3327);
    //I doubt that we really need it and we really support it; 
    //Anyway, we do not need to act as the SIP UE in this case (see RFC3251). 
    //This whole path-related RFC3327 stuff probably must be removed.
    //(Oleg)
    if(mUsingProxy && PathList().empty()) {
      msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_ROUTE, BuildFirstRoute()));
    }
    if (!PathList().empty())
        msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_ROUTE, SipUtils::BuildDelimitedList(PathList(), ",")));
    if (!ServiceRouteList().empty())
        msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_ROUTE, SipUtils::BuildDelimitedList(ServiceRouteList(), ",")));
    if (!AssocUriList().empty())
        msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_P_PREFERRED_IDENTITY, AssocUriList().front()));  // FUTURE: may need more intelligent mechanism for selecting from multiple values
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_ALLOW, ALLOWED_METHODS));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_SUPPORTED, SipProtocol::TOKEN_100REL + "," + SipProtocol::TOKEN_TIMER));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_SESSION_EXPIRES, SipProtocol::BuildSessionExpirationString(SipSessionExpiration(mSessionExpires))));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_MIN_SE, boost::lexical_cast<std::string>(mMinSessionExpires)));

    if (!mMakeMediaOffer)
        throw EPHXInternal("UserAgentCallDialog::SendInvite: empty MakeMediaOffer delegate");

    SdpMessage sdp;
    mMakeMediaOffer(sdp);
    msg.body = SipProtocol::BuildSdpString(sdp);

    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTENT_TYPE, "application/sdp"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTENT_LENGTH, boost::lexical_cast<std::string>(msg.body.size())));

    mRAckState.first = true;
    mRAckState.second = 0;

    TC_LOG_INFO_LOCAL(Port(), LOG_SIP, "[UserAgentCallDialog::SendInvite] " << Contact() << " sending INVITE");
    return SendRequestInternal(msg);
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentCallDialog::SendPrack(uint32_t rseq, const SipMessage& resp)
{
    SipMessage msg;
    msg.peer = Peer();
    msg.type = SipMessage::SIP_MSG_REQUEST;
    msg.compactHeaders = CompactHeaders();
    msg.fromTag = FromTag();
    msg.toTag = ToTag();
    msg.method = SipMessage::SIP_METHOD_PRACK;
    msg.seq = NextSeq();
    msg.request.uri = BuildRequestUri();
    msg.request.version = SipMessage::SIP_VER_2_0;

    const std::string branch(SipProtocol::BRANCH_PREFIX + VoIPUtils::MakeRandomHexString(SipProtocol::BRANCH_RANDOMNESS));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_VIA, SipProtocol::BuildViaString(Contact(), &branch)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_TO, SipProtocol::BuildAddressString(ToUri(), &ToTag())));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_FROM, SipProtocol::BuildAddressString(FromUri(), &FromTag())));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CALL_ID, CallId()));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CSEQ, SipProtocol::BuildCSeqString(msg.seq, msg.method)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_RACK, SipProtocol::BuildRAckString(rseq, resp.seq, resp.method)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_MAX_FORWARDS, boost::lexical_cast<std::string>(SipProtocol::DEFAULT_MAX_FORWARDS)));
    if (HasRouteList())
        msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_ROUTE, BuildRouteString()));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTENT_LENGTH, "0"));

    TC_LOG_INFO_LOCAL(Port(), LOG_SIP, "[UserAgentCallDialog::SendPrack] " << Contact() << " sending PRACK");
    return SendRequestInternal(msg);
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentCallDialog::SendAcknowledge(void)
{
    SipMessage msg;
    msg.peer = Peer();
    msg.type = SipMessage::SIP_MSG_REQUEST;
    msg.compactHeaders = CompactHeaders();
    msg.fromTag = FromTag();
    msg.toTag = ToTag();
    msg.method = SipMessage::SIP_METHOD_ACK;
    msg.seq = InviteSeq();
    msg.request.uri = BuildRequestUri();
    msg.request.version = SipMessage::SIP_VER_2_0;

    const std::string branch(SipProtocol::BRANCH_PREFIX + VoIPUtils::MakeRandomHexString(SipProtocol::BRANCH_RANDOMNESS));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_VIA, SipProtocol::BuildViaString(Contact(), &branch)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_TO, SipProtocol::BuildAddressString(ToUri(), &ToTag())));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_FROM, SipProtocol::BuildAddressString(FromUri(), &FromTag())));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CALL_ID, CallId()));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CSEQ, SipProtocol::BuildCSeqString(msg.seq, msg.method)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_MAX_FORWARDS, boost::lexical_cast<std::string>(SipProtocol::DEFAULT_MAX_FORWARDS)));
    if (HasRouteList())
        msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_ROUTE, BuildRouteString()));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTENT_LENGTH, "0"));

    TC_LOG_INFO_LOCAL(Port(), LOG_SIP, "[UserAgentCallDialog::SendAcknowledge] " << Contact() << " sending ACK");
    return SendRequestInternal(msg);
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentCallDialog::SendUpdate(void)
{
    SipMessage msg;
    msg.peer = Peer();
    msg.type = SipMessage::SIP_MSG_REQUEST;
    msg.compactHeaders = CompactHeaders();
    msg.fromTag = FromTag();
    msg.toTag = ToTag();
    msg.method = SipMessage::SIP_METHOD_UPDATE;
    msg.seq = NextSeq();
    msg.request.uri = BuildRequestUri();
    msg.request.version = SipMessage::SIP_VER_2_0;

    const std::string branch(SipProtocol::BRANCH_PREFIX + VoIPUtils::MakeRandomHexString(SipProtocol::BRANCH_RANDOMNESS));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_VIA, SipProtocol::BuildViaString(Contact(), &branch)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_TO, SipProtocol::BuildAddressString(ToUri(), &ToTag())));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_FROM, SipProtocol::BuildAddressString(FromUri(), &FromTag())));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CALL_ID, CallId()));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CSEQ, SipProtocol::BuildCSeqString(msg.seq, msg.method)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_MAX_FORWARDS, boost::lexical_cast<std::string>(SipProtocol::DEFAULT_MAX_FORWARDS)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTACT, SipProtocol::BuildAddressString(Contact())));
    if (HasRouteList())
        msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_ROUTE, BuildRouteString()));
    if (UsingSessionRefresh())
    {
        msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_SUPPORTED, SipProtocol::TOKEN_TIMER));
        msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_SESSION_EXPIRES, SipProtocol::BuildSessionExpirationString(SipSessionExpiration(mSessionExpires, SessionRefresher() ? SipSessionExpiration::UAC_REFRESHER : SipSessionExpiration::UAS_REFRESHER))));
        msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_MIN_SE, boost::lexical_cast<std::string>(mMinSessionExpires)));
    }
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTENT_LENGTH, "0"));

    TC_LOG_INFO_LOCAL(Port(), LOG_SIP, "[UserAgentCallDialog::SendUpdate] " << Contact() << " sending UPDATE");
    return SendRequestInternal(msg);
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentCallDialog::SendBye(void)
{
    SipMessage msg;
    msg.peer = Peer();
    msg.type = SipMessage::SIP_MSG_REQUEST;
    msg.compactHeaders = CompactHeaders();
    msg.fromTag = FromTag();
    msg.toTag = ToTag();
    msg.method = SipMessage::SIP_METHOD_BYE;
    msg.seq = NextSeq();
    msg.request.uri = BuildRequestUri();
    msg.request.version = SipMessage::SIP_VER_2_0;

    const std::string branch(SipProtocol::BRANCH_PREFIX + VoIPUtils::MakeRandomHexString(SipProtocol::BRANCH_RANDOMNESS));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_VIA, SipProtocol::BuildViaString(Contact(), &branch)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_TO, SipProtocol::BuildAddressString(ToUri(), &ToTag())));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_FROM, SipProtocol::BuildAddressString(FromUri(), &FromTag())));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CALL_ID, CallId()));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CSEQ, SipProtocol::BuildCSeqString(msg.seq, msg.method)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_MAX_FORWARDS, boost::lexical_cast<std::string>(SipProtocol::DEFAULT_MAX_FORWARDS)));
    if (HasRouteList())
        msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_ROUTE, BuildRouteString()));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTENT_LENGTH, "0"));

    TC_LOG_INFO_LOCAL(Port(), LOG_SIP, "[UserAgentCallDialog::SendBye] " << Contact() << " sending BYE");
    return SendRequestInternal(msg);
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentCallDialog::SendResponse(uint16_t statusCode, const std::string& message, const SipMessage *req)
{
    // If request isn't provided, check that we have held request to use
    if (!req)
        req = mHeldRequest.get();

    if (!req)
        throw EPHXInternal("UserAgentCallDialog::SendResponse");

    std::auto_ptr<SipMessage> msg(new SipMessage);
    msg->transactionId = req->transactionId;
    msg->peer = req->peer;
    msg->type = SipMessage::SIP_MSG_STATUS;
    msg->compactHeaders = CompactHeaders();
    msg->fromTag = ToTag();
    msg->toTag = FromTag();
    msg->method = req->method;
    msg->seq = req->seq;
    msg->status.code = statusCode;
    msg->status.version = SipMessage::SIP_VER_2_0;
    msg->status.message = message;

    req->ForEachHeader(SipMessage::SIP_HEADER_VIA, SipMessage::PushHeader(*msg));
    msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_TO, SipProtocol::BuildAddressString(FromUri(), &FromTag())));
    req->ForEachHeader(SipMessage::SIP_HEADER_FROM, SipMessage::PushHeader(*msg));
    msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CALL_ID, CallId()));
    msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CSEQ, SipProtocol::BuildCSeqString(msg->seq, msg->method)));
    req->ForEachHeader(SipMessage::SIP_HEADER_RECORD_ROUTE, SipMessage::PushHeader(*msg));
    msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_SUPPORTED, SipProtocol::TOKEN_100REL + "," + SipProtocol::TOKEN_TIMER));

    std::list<std::string> requiredExtensions;

    // If we're sending a 1xx provisional response, it may need to be sent reliably
    bool using100Rel = false;
    if (msg->status.Category() == SipMessage::SIP_STATUS_PROVISIONAL && statusCode != 100)
    {
        boost::function1<bool, const std::string&> tokenComparator = (boost::bind((int (std::string::*)(const std::string&) const) &std::string::compare, &SipProtocol::TOKEN_100REL, _1) == 0);
        string_token_iterator end;

        const SipMessage::HeaderField *requireHdr = req->FindHeader(SipMessage::SIP_HEADER_REQUIRE);
        const SipMessage::HeaderField *supportedHdr = req->FindHeader(SipMessage::SIP_HEADER_SUPPORTED);

        if (requireHdr)
            using100Rel = (std::find_if(string_token_iterator(requireHdr->value, ","), end, tokenComparator) != end);

        if (!using100Rel && supportedHdr)
            using100Rel = (std::find_if(string_token_iterator(supportedHdr->value, ","), end, tokenComparator) != end);

        if (using100Rel)
        {
            requiredExtensions.push_back(SipProtocol::TOKEN_100REL);

            uint32_t rseq = VoIPUtils::MakeRandomInteger() & 0x7fffffff;  // RSeq chosen from interval [1, 2^31-1]
            if (!rseq)
                rseq = 1;

            msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_RSEQ, boost::lexical_cast<std::string>(rseq)));
            mRSeqState.first = mRSeqState.second = rseq;
        }
    }

    if ((msg->method == SipMessage::SIP_METHOD_INVITE && msg->status.Category() == SipMessage::SIP_STATUS_SUCCESS) || statusCode == 405)
        msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_ALLOW, ALLOWED_METHODS));

    if (UsingSessionRefresh() && (msg->method == SipMessage::SIP_METHOD_INVITE || msg->method == SipMessage::SIP_METHOD_UPDATE) && msg->status.Category() == SipMessage::SIP_STATUS_SUCCESS)
    {
        requiredExtensions.push_back(SipProtocol::TOKEN_TIMER);
        msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_SESSION_EXPIRES, SipProtocol::BuildSessionExpirationString(SipSessionExpiration(mSessionExpires, SessionRefresher() ? SipSessionExpiration::UAS_REFRESHER : SipSessionExpiration::UAC_REFRESHER))));
    }

    if ((msg->method == SipMessage::SIP_METHOD_INVITE || msg->method == SipMessage::SIP_METHOD_UPDATE) && (msg->status.Category() == SipMessage::SIP_STATUS_PROVISIONAL || msg->status.Category() == SipMessage::SIP_STATUS_SUCCESS))
        msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTACT, SipProtocol::BuildAddressString(Contact())));

    if (msg->method == SipMessage::SIP_METHOD_INVITE && msg->status.Category() == SipMessage::SIP_STATUS_SUCCESS)
    {
        // Include media offer
        if (!mMakeMediaOffer)
            throw EPHXInternal("UserAgentCallDialog::SendResponse: empty MakeMediaOffer delegate");

        SdpMessage sdp;
        mMakeMediaOffer(sdp);
        msg->body = SipProtocol::BuildSdpString(sdp);

        msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTENT_TYPE, "application/sdp"));
        msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTENT_LENGTH, boost::lexical_cast<std::string>(msg->body.size())));
    }
    else
        msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTENT_LENGTH, "0"));

    if (!requiredExtensions.empty())
        msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_REQUIRE, SipUtils::BuildDelimitedList(requiredExtensions, ",")));

    TC_LOG_INFO_LOCAL(Port(), LOG_SIP, "[UserAgentCallDialog::SendResponse] " << Contact() << " sending status \"" << statusCode << " " << message << "\"");
    const bool success = SendResponseInternal(*msg);

    if (success && using100Rel)
    {
        // Hold status message and start the retransmit timer
        mHeldStatus.reset(msg.release());
        m100RelTimer.Start(mRTT = TIMER_T1);
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::ChangeState(const State *toState)
{
    // Log state change
    TC_LOG_INFO_LOCAL(Port(), LOG_SIP, "[UserAgentCallDialog::ChangeState] " << Contact() << " changing state from " << mState->Name() << " to " << toState->Name());
    mState = toState;
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::TimerExpired(TimerId which)
{
    TC_LOG_INFO_LOCAL(Port(), LOG_SIP, "[UserAgentCallDialog::TimerExpired] " << Contact() << " timer id " << which << " expired");

    if (which == MEDIA_TIMER) 
    {
      MediaControlComplete(mMediaLastResult);
      return;
    } 

    if (which == _100REL_TIMER)
    {
        // Handle retransmission of 1xx responses (RFC 3262, Section 3)
        if (mHeldStatus)
        {
            mRTT = std::min(2 * mRTT, TIMER_T2);
            if (mRTT < (64 * TIMER_T1))
            {
                // Increment sequence number
                SipMessage::HeaderField *rseqHdr = mHeldStatus->FindHeader(SipMessage::SIP_HEADER_RSEQ);
                if (!rseqHdr)
                    throw EPHXInternal("UserAgentCallDialog::TimerExpired: missing RSeq header");

                rseqHdr->value = boost::lexical_cast<std::string>(++mRSeqState.second);

                // Retransmit held status
                if (!SendResponseInternal(*mHeldStatus))
                {
                    mHeldStatus.reset();
                    mState->TimerExpired(*this, which);
                }
                else
                    m100RelTimer.Start(mRTT);
            }
            else
            {
                // Strip RSeq and Require headers from final 500 response
                SipMessage::HeaderField *hdr = mHeldStatus->FindHeader(SipMessage::SIP_HEADER_RSEQ);
                if (hdr)
                    hdr->type = SipMessage::SIP_HEADER_UNKNOWN;

                hdr = mHeldStatus->FindHeader(SipMessage::SIP_HEADER_REQUIRE);
                if (hdr)
                    hdr->type = SipMessage::SIP_HEADER_UNKNOWN;

                mHeldStatus->status.code = 500;
                mHeldStatus->status.message = "Server Internal Error";

                // Reject the original request, release held status and notify state machine
                SendResponseInternal(*mHeldStatus);
                mHeldStatus.reset();
                mState->TimerExpired(*this, which);
            }
        }
    }
    else
    {
        // Notify state machine
        mState->TimerExpired(*this, which);
    }
}

///////////////////////////////////////////////////////////////////////////////
