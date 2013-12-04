/// @file
/// @brief SIP User Agent (UA) Call Dialog concrete state class implementations
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <boost/lexical_cast.hpp>

#include "SdpMessage.h"
#include "SipMessage.h"
#include "SipProtocol.h"
#include "VoIPLog.h"
#include "UserAgentCallDialog.h"

USING_SIP_NS;
USING_SIPLEAN_NS;

///////////////////////////////////////////////////////////////////////////////

#define _DECLARE_USER_AGENT_CALL_DIALOG_STATES_
#include "UserAgentCallDialogState.h"

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::CallingState::Abort(UserAgentCallDialog& dialog, bool force) const
{
  CleanTimers(dialog);
  ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
  NotifyInviteComplete(dialog, false);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::CallingState::ProcessRequest(UserAgentCallDialog& dialog, const SipMessage& msg) const
{
    switch (msg.method)
    {
      case SipMessage::SIP_METHOD_UPDATE:
          UpdateDialog(dialog, msg);
          if (!SendResponse(dialog, 200, "OK", &msg))
          {
              // TODO: is it worth sending CANCEL?
              ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
              NotifyInviteComplete(dialog, false);
          }

          if (SessionRefresher(dialog))
              StartSessionTimer(dialog);
          else
              StopSessionTimer(dialog);
          break;
          
      case SipMessage::SIP_METHOD_BYE:
          SendResponse(dialog, 200, "OK", &msg);
          ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
          NotifyInviteComplete(dialog, false);
          break;
          
      default:
          throw EPHXInternal("UserAgentCallDialog::CallingState::ProcessRequest: unexpected method " + boost::lexical_cast<std::string>(msg.method));
    }
}

void UserAgentCallDialog::CallingState::ProcessResponse(UserAgentCallDialog& dialog, const SipMessage& msg) const
{
    if (msg.method != SipMessage::SIP_METHOD_INVITE)
        throw EPHXInternal("UserAgentCallDialog::CallingState::ProcessResponse: unexpected method " + boost::lexical_cast<std::string>(msg.method));

    switch (msg.status.Category())
    {
      case SipMessage::SIP_STATUS_PROVISIONAL:
          UpdateDialog(dialog, msg);  // 100rel response
          break;
          
      case SipMessage::SIP_STATUS_SUCCESS:
      {
          UpdateDialog(dialog, msg);
          if (!SendAcknowledge(dialog))
          {
              ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
	      NotifyInviteComplete(dialog, false);
              break;
          }

          NotifyInviteComplete(dialog, true);
          ChangeState(dialog, &UserAgentCallDialog::OFF_HOOK_STATE);

          if (SessionRefresher(dialog))
              StartSessionTimer(dialog);

          SdpMessage sdp;
          if (SipProtocol::ParseSdpString(msg.body, sdp) && ConsumeMediaOffer(dialog, sdp))
              StartMedia(dialog, true);
          else
              StartCallTimer(dialog);
          break;
      }

      default:
          ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
	  NotifyInviteComplete(dialog, false);
          break;
    }
}

void UserAgentCallDialog::CallingState::TransactionError(UserAgentCallDialog& dialog) const
{
    ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
    NotifyInviteComplete(dialog, false);
}

void UserAgentCallDialog::CallingState::TimerExpired(UserAgentCallDialog& dialog, TimerId which) const
{
    throw EPHXInternal("UserAgentCallDialog::CallingState::TimerExpired: unexpected timer " + boost::lexical_cast<std::string>(which));
}

void UserAgentCallDialog::CallingState::MediaControlComplete(UserAgentCallDialog& dialog, bool success) const
{
    throw EPHXInternal("UserAgentCallDialog::CallingState::MediaControlComplete");
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::AnsweringState::Abort(UserAgentCallDialog& dialog, bool force) const
{
    CleanTimers(dialog);
    SendBye(dialog);
    ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
}

void UserAgentCallDialog::AnsweringState::ProcessRequest(UserAgentCallDialog& dialog, const SipMessage& msg) const
{
    switch (msg.method)
    {
      case SipMessage::SIP_METHOD_INVITE:
      {
          UpdateDialog(dialog, msg);
          
          SdpMessage sdp;
          if (!SipProtocol::ParseSdpString(msg.body, sdp))
          {
              SendResponse(dialog, 400, "Bad Request", &msg);
              ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
              break;
          }
          
          if (!ConsumeMediaOffer(dialog, sdp))
          {
              SendResponse(dialog, 480, "Temporarily Unavailable", &msg);
              ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
              break;
          }
          
          if (!SendResponse(dialog, 180, "Ringing", &msg))
          {
              ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
              break;
          }

          StartRingTimer(dialog);
          
          if (SessionRefresher(dialog))
              StartSessionTimer(dialog);

          HoldRequest(dialog, msg);
          break;
      }

      case SipMessage::SIP_METHOD_ACK:
          StopAckWaitTimer(dialog);
          NotifyAnswerComplete(dialog);
          ChangeState(dialog, &UserAgentCallDialog::OFF_HOOK_STATE);
          StartMedia(dialog, true);
          break;

      case SipMessage::SIP_METHOD_UPDATE:
          UpdateDialog(dialog, msg);
          if (!SendResponse(dialog, 200, "OK", &msg))
          {
              Stop100RelTimer(dialog);
              StopRingTimer(dialog);
              StopSessionTimer(dialog);
              StopAckWaitTimer(dialog);
              ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
              break;
          }

          if (SessionRefresher(dialog))
              StartSessionTimer(dialog);
          else
              StopSessionTimer(dialog);
          break;
          
      case SipMessage::SIP_METHOD_BYE:
          Stop100RelTimer(dialog);
          StopRingTimer(dialog);
          StopSessionTimer(dialog);
          StopAckWaitTimer(dialog);
          SendResponse(dialog, 200, "OK", &msg);
          ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
          break;
          
      default:
          throw EPHXInternal("UserAgentCallDialog::AnsweringState::ProcessRequest: unexpected method " + boost::lexical_cast<std::string>(msg.method));
    }
}

void UserAgentCallDialog::AnsweringState::ProcessResponse(UserAgentCallDialog& dialog, const SipMessage& msg) const
{
    if (msg.method != SipMessage::SIP_METHOD_UPDATE)
        throw EPHXInternal("UserAgentCallDialog::AnsweringState::ProcessResponse: unexpected method " + boost::lexical_cast<std::string>(msg.method));

    switch (msg.status.Category())
    {
      case SipMessage::SIP_STATUS_SUCCESS:
          UpdateDialog(dialog, msg);
          NotifyRefreshComplete(dialog);

          if (SessionRefresher(dialog))
              StartSessionTimer(dialog);
          else
              StopSessionTimer(dialog);
          break;
              
      default:
          // TODO: what do we do if our UPDATE failed?
          break;
    }
}

void UserAgentCallDialog::AnsweringState::TransactionError(UserAgentCallDialog& dialog) const
{
    Stop100RelTimer(dialog);
    StopRingTimer(dialog);
    StopSessionTimer(dialog);
    StopAckWaitTimer(dialog);
    ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
}

void UserAgentCallDialog::AnsweringState::TimerExpired(UserAgentCallDialog& dialog, TimerId which) const
{
    switch (which)
    {
      case RING_TIMER:
          Stop100RelTimer(dialog);
          if (!SendResponse(dialog, 200, "OK"))
          {
              StopSessionTimer(dialog);
              ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
          }
          StartAckWaitTimer(dialog);
          break;

      case _100REL_TIMER:
          // TODO: reject original request with 5xx response (RFC 3262, Section 3)
          StopAckWaitTimer(dialog);
          StopSessionTimer(dialog);
          ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
          break;
          
      case ACK_WAIT_TIMER:
          if (!RestartAckWaitTimer(dialog))
          {
              StopSessionTimer(dialog);
              SendBye(dialog);
              ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
          }
          else if (!SendResponse(dialog, 200, "OK"))
          {
              StopSessionTimer(dialog);
              StopAckWaitTimer(dialog);
              ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
          } else if(VOIP_NS::VoIPUtils::time_after64(VOIP_NS::VoIPUtils::getMilliSeconds(),dialog.mAckWaitTimerTimeout)) {
              StopSessionTimer(dialog);
              StopAckWaitTimer(dialog);
              ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
	  }
          break;

      case SESSION_TIMER:
          SendUpdate(dialog);
          break;
          
      default:
          throw EPHXInternal("UserAgentCallDialog::AnsweringState::TimerExpired: unexpected timer " + boost::lexical_cast<std::string>(which));
    }
}

void UserAgentCallDialog::AnsweringState::MediaControlComplete(UserAgentCallDialog& dialog, bool success) const
{
    throw EPHXInternal("UserAgentCallDialog::AnsweringState::MediaControlComplete");
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::OffHookState::Abort(UserAgentCallDialog& dialog, bool force) const
{
    CleanTimers(dialog);
    if (force)
    {
        ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);
        StopMedia(dialog, false);
    }
    else
    {
        ChangeState(dialog, &UserAgentCallDialog::LOCAL_ABORTING_STATE);
        StopMedia(dialog, true);
    }
}

void UserAgentCallDialog::OffHookState::ProcessRequest(UserAgentCallDialog& dialog, const SipMessage& msg) const
{
    switch (msg.method)
    {
      case SipMessage::SIP_METHOD_ACK:
          break;

      case SipMessage::SIP_METHOD_UPDATE:
          UpdateDialog(dialog, msg);
          if (!SendResponse(dialog, 200, "OK", &msg))
          {
              StopSessionTimer(dialog);
              StopCallTimer(dialog);
              ChangeState(dialog, &UserAgentCallDialog::LOCAL_ABORTING_STATE);
              StopMedia(dialog, true);
          }

          if (SessionRefresher(dialog))
              StartSessionTimer(dialog);
          else
              StopSessionTimer(dialog);
          break;
          
      case SipMessage::SIP_METHOD_BYE:
	  CleanTimers(dialog);
          HoldRequest(dialog, msg);
          ChangeState(dialog, &UserAgentCallDialog::REMOTE_ABORTING_STATE);
          StopMedia(dialog, true);
          break;

      default:
          throw EPHXInternal("UserAgentCallDialog::OffHookState::ProcessRequest: unexpected method " + boost::lexical_cast<std::string>(msg.method));
    }
}

void UserAgentCallDialog::OffHookState::ProcessResponse(UserAgentCallDialog& dialog, const SipMessage& msg) const
{
    if (msg.method == SipMessage::SIP_METHOD_INVITE)  // TODO: why is this here?
    {
        switch (msg.status.Category())
        {
          case SipMessage::SIP_STATUS_SUCCESS:
              if (Caller(dialog))
                  SendAcknowledge(dialog);
              break;
              
          default:
              break;
        }
    }
    else if (msg.method == SipMessage::SIP_METHOD_UPDATE)
    {
        switch (msg.status.Category())
        {
          case SipMessage::SIP_STATUS_SUCCESS:
              UpdateDialog(dialog, msg);
              NotifyRefreshComplete(dialog);

              if (SessionRefresher(dialog))
                  StartSessionTimer(dialog);
              else
                  StopSessionTimer(dialog);
              break;
              
          default:
              // TODO: what do we do if our UPDATE failed?
              break;
        }
    }
    else
        throw EPHXInternal("UserAgentCallDialog::OffHookState::ProcessResponse: unexpected method " + boost::lexical_cast<std::string>(msg.method));
}

void UserAgentCallDialog::OffHookState::TransactionError(UserAgentCallDialog& dialog) const
{
    CleanTimers(dialog);
    ChangeState(dialog, &UserAgentCallDialog::LOCAL_ABORTING_STATE);
    StopMedia(dialog, true);
}

void UserAgentCallDialog::OffHookState::TimerExpired(UserAgentCallDialog& dialog, TimerId which) const
{
    switch (which)
    {
      case SESSION_TIMER:
          SendUpdate(dialog);  // TODO: what do we do if send of UPDATE failed?
          break;
          
      case CALL_TIMER:
          if (!Caller(dialog))
              throw EPHXInternal("UserAgentCallDialog::OffHookState::TimerExpired: CALL_TIMER");
          
          StopSessionTimer(dialog);
          ChangeState(dialog, &UserAgentCallDialog::LOCAL_ABORTING_STATE);
          StopMedia(dialog, true);
          break;
          
      default:
          throw EPHXInternal("UserAgentCallDialog::OffHookState::TimerExpired: unexpected timer " + boost::lexical_cast<std::string>(which));
    }
}

void UserAgentCallDialog::OffHookState::MediaControlComplete(UserAgentCallDialog& dialog, bool success) const
{
    if (Caller(dialog))
        StartCallTimer(dialog);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::LocalAbortingState::Abort(UserAgentCallDialog& dialog, bool force) const
{
  CleanTimers(dialog);
}

void UserAgentCallDialog::LocalAbortingState::ProcessRequest(UserAgentCallDialog& dialog, const SipMessage& msg) const
{
    switch (msg.method)
    {
      case SipMessage::SIP_METHOD_ACK:
          break;

      case SipMessage::SIP_METHOD_UPDATE:
          SendResponse(dialog, 408, "Request Timeout", &msg);
          break;
          
      case SipMessage::SIP_METHOD_BYE:
          SendResponse(dialog, 200, "OK", &msg);
          break;
          
      default:
          throw EPHXInternal("UserAgentCallDialog::LocalAbortingState::ProcessRequest: unexpected method " + boost::lexical_cast<std::string>(msg.method));
    }
}

void UserAgentCallDialog::LocalAbortingState::ProcessResponse(UserAgentCallDialog& dialog, const SipMessage& msg) const
{
    if (msg.method == SipMessage::SIP_METHOD_BYE)
    {
        ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);

        if (Caller(dialog))
            NotifyCallComplete(dialog);
    }
}

void UserAgentCallDialog::LocalAbortingState::TransactionError(UserAgentCallDialog& dialog) const
{
    ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);

    if (Caller(dialog))
        NotifyCallComplete(dialog);
}

void UserAgentCallDialog::LocalAbortingState::TimerExpired(UserAgentCallDialog& dialog, TimerId which) const
{
    throw EPHXInternal("UserAgentCallDialog::LocalAbortingState::TimerExpired: unexpected timer " + boost::lexical_cast<std::string>(which));
}

void UserAgentCallDialog::LocalAbortingState::MediaControlComplete(UserAgentCallDialog& dialog, bool success) const
{
    if (!SendBye(dialog))
    {
        ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);

        if (Caller(dialog))
            NotifyCallComplete(dialog);
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::RemoteAbortingState::Abort(UserAgentCallDialog& dialog, bool force) const
{
  CleanTimers(dialog);
}

void UserAgentCallDialog::RemoteAbortingState::ProcessRequest(UserAgentCallDialog& dialog, const SipMessage& msg) const
{
    switch (msg.method)
    {
      case SipMessage::SIP_METHOD_ACK:
          break;

      case SipMessage::SIP_METHOD_UPDATE:
          SendResponse(dialog, 408, "Request Timeout", &msg);
          break;
          
      case SipMessage::SIP_METHOD_BYE:
          SendResponse(dialog, 200, "OK", &msg);
          break;
          
      default:
          throw EPHXInternal("UserAgentCallDialog::RemoteAbortingState::ProcessRequest: unexpected method " + boost::lexical_cast<std::string>(msg.method));
    }
}

void UserAgentCallDialog::RemoteAbortingState::ProcessResponse(UserAgentCallDialog& dialog, const SipMessage& msg) const
{
}

void UserAgentCallDialog::RemoteAbortingState::TransactionError(UserAgentCallDialog& dialog) const
{
}

void UserAgentCallDialog::RemoteAbortingState::TimerExpired(UserAgentCallDialog& dialog, TimerId which) const
{
    throw EPHXInternal("UserAgentCallDialog::RemoteAbortingState::TimerExpired: unexpected timer " + boost::lexical_cast<std::string>(which));
}

void UserAgentCallDialog::RemoteAbortingState::MediaControlComplete(UserAgentCallDialog& dialog, bool success) const
{
    SendResponse(dialog, 200, "OK");
    
    ChangeState(dialog, &UserAgentCallDialog::INACTIVE_STATE);

    if (Caller(dialog))
        NotifyCallComplete(dialog);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentCallDialog::InactiveState::Abort(UserAgentCallDialog& dialog, bool force) const
{
  CleanTimers(dialog);
}

void UserAgentCallDialog::InactiveState::ProcessRequest(UserAgentCallDialog& dialog, const SipMessage& msg) const
{
    throw EPHXInternal("UserAgentCallDialog::InactiveState::ProcessRequest: unexpected method " + boost::lexical_cast<std::string>(msg.method));
}

void UserAgentCallDialog::InactiveState::ProcessResponse(UserAgentCallDialog& dialog, const SipMessage& msg) const
{
    throw EPHXInternal("UserAgentCallDialog::InactiveState::ProcessResponse: unexpected method " + boost::lexical_cast<std::string>(msg.method));
}

void UserAgentCallDialog::InactiveState::TransactionError(UserAgentCallDialog& dialog) const
{
    throw EPHXInternal("UserAgentCallDialog::InactiveState::TransactionError");
}

void UserAgentCallDialog::InactiveState::TimerExpired(UserAgentCallDialog& dialog, TimerId which) const
{
    throw EPHXInternal("UserAgentCallDialog::InactiveState::TimerExpired: unexpected timer " + boost::lexical_cast<std::string>(which));
}

void UserAgentCallDialog::InactiveState::MediaControlComplete(UserAgentCallDialog& dialog, bool success) const
{
    throw EPHXInternal("UserAgentCallDialog::InactiveState::MediaControlComplete");
}

///////////////////////////////////////////////////////////////////////////////
