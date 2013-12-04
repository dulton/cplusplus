/// @file
/// @brief SIP User Agent (UA) Call Dialog State class definitions
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _USER_AGENT_CALL_DIALOG_STATE_H_
#define _USER_AGENT_CALL_DIALOG_STATE_H_

#include "VoIPUtils.h"
#include "SipCommon.h"

#if !defined(_DEFINE_USER_AGENT_CALL_DIALOG_STATES_) && !defined(_DECLARE_USER_AGENT_CALL_DIALOG_STATES_)
# error UserAgentCallDialogState.h is an implementation-private header file and should not be \
included by files other than those implementing UserAgentCallDialog or UserAgentCallDialogState classes.
#endif

DECL_SIPLEAN_NS

///////////////////////////////////////////////////////////////////////////////

// Base state class definition
struct UserAgentCallDialog::State
{
  static const unsigned int ACK_WAIT_TIMEOUT = 32000; //In millisecs; SIP default for connection establishment.

    virtual ~State() { }

    // State name
    virtual const std::string Name(void) const = 0;

    // Control events
    virtual void Abort(UserAgentCallDialog& dialog, bool force) const = 0;
    
    // Protocol events
    virtual void ProcessRequest(UserAgentCallDialog& dialog, const SipMessage& msg) const = 0;
    virtual void ProcessResponse(UserAgentCallDialog& dialog, const SipMessage& msg) const = 0;
    virtual void TransactionError(UserAgentCallDialog& dialog) const = 0;

    // Timer events
    virtual void TimerExpired(UserAgentCallDialog& dialog, TimerId which) const = 0;

    // Media events
    virtual void MediaControlComplete(UserAgentCallDialog& dialog, bool success) const = 0;
    
protected:
    // Dialog accessors
    bool Caller(const UserAgentCallDialog& dialog) const { return dialog.Caller(); }
    bool SessionRefresher(const UserAgentCallDialog& dialog) const { return dialog.SessionRefresher(); }

    // Central state change method
    void ChangeState(UserAgentCallDialog& dialog, const UserAgentCallDialog::State *toState) const { dialog.ChangeState(toState); }

    // Client protocol methods
    void UpdateDialog(UserAgentCallDialog& dialog, const SipMessage& msg) const { dialog.UpdateDialog(msg); }
    bool SendAcknowledge(UserAgentCallDialog& dialog) const { return dialog.SendAcknowledge(); }
    bool SendUpdate(UserAgentCallDialog& dialog) const { return dialog.SendUpdate(); }
    bool SendBye(UserAgentCallDialog& dialog) const { return dialog.SendBye(); }

    // Server protocol methods
    void HoldRequest(UserAgentCallDialog& dialog, const SipMessage& msg) const { dialog.mHeldRequest.reset(new SipMessage(msg)); }
    bool SendResponse(UserAgentCallDialog& dialog, uint16_t statusCode, const::string& message, const SipMessage *req = 0) const { return dialog.SendResponse(statusCode, message, req); }

    // Media methods
    bool ConsumeMediaOffer(UserAgentCallDialog& dialog, const SdpMessage& sdp) const { return dialog.mConsumeMediaOffer ? dialog.mConsumeMediaOffer(sdp) : false; }
    void StartMedia(UserAgentCallDialog& dialog, bool wantComplNotification = false) const { if (dialog.mMediaControl) dialog.mMediaControl(true, wantComplNotification); }
    void StopMedia(UserAgentCallDialog& dialog, bool wantComplNotification = false) const { if (dialog.mMediaControl) dialog.mMediaControl(false, wantComplNotification); }
    
    // Status notifications
    void NotifyInviteComplete(UserAgentCallDialog& dialog, bool success) const { if (dialog.mNotifyInviteComplete) dialog.mNotifyInviteComplete(success); }
    void NotifyAnswerComplete(UserAgentCallDialog& dialog) const { if (dialog.mNotifyAnswerComplete) dialog.mNotifyAnswerComplete(); }
    void NotifyCallComplete(UserAgentCallDialog& dialog) const { if (dialog.mNotifyCallComplete) dialog.mNotifyCallComplete(); }
    void NotifyRefreshComplete(UserAgentCallDialog& dialog) const { if (dialog.mNotifyRefreshComplete) dialog.mNotifyRefreshComplete(); }
    
    // Timer control methods
    void StartRingTimer(UserAgentCallDialog& dialog) const { dialog.mRingTimer.Start(dialog.mRingTime); }
    void StopRingTimer(UserAgentCallDialog& dialog) const { dialog.mRingTimer.Stop(); }

    void Stop100RelTimer(UserAgentCallDialog& dialog) const { dialog.mHeldStatus.reset(); dialog.m100RelTimer.Stop(); }
    
    void StartAckWaitTimer(UserAgentCallDialog& dialog) const { 
      dialog.mAckWaitTimer.Start(dialog.mRTT = UserAgentCallDialog::TIMER_T1); 
      dialog.mAckWaitTimerTimeout=VOIP_NS::VoIPUtils::time_plus64(VOIP_NS::VoIPUtils::getMilliSeconds(),ACK_WAIT_TIMEOUT);
    }

    bool RestartAckWaitTimer(UserAgentCallDialog& dialog) const
    {
        dialog.mRTT = std::min(2 * dialog.mRTT, UserAgentCallDialog::TIMER_T2);
        if (dialog.mRTT < (64 * UserAgentCallDialog::TIMER_T1))
        {
            dialog.mAckWaitTimer.Start(dialog.mRTT);
            return true;
        }
        else
            return false;
    }
    
    void StopAckWaitTimer(UserAgentCallDialog& dialog) const { dialog.mAckWaitTimer.Stop(); }

    void StartSessionTimer(UserAgentCallDialog& dialog) const { dialog.mSessionTimer.Start(ACE_Time_Value(dialog.mSessionExpires, 0) * 0.5); }
    void StopSessionTimer(UserAgentCallDialog& dialog) const { dialog.mSessionTimer.Stop(); }

    void CleanTimers(UserAgentCallDialog& dialog) const {
      Stop100RelTimer(dialog);
      StopRingTimer(dialog);
      StopSessionTimer(dialog);
      StopAckWaitTimer(dialog);
      StopCallTimer(dialog);
    }
    
    void StartCallTimer(UserAgentCallDialog& dialog) const { dialog.mCallTimer.Start(dialog.mCallTime); }
    void StopCallTimer(UserAgentCallDialog& dialog) const { dialog.mCallTimer.Stop(); }
};

///////////////////////////////////////////////////////////////////////////////

// Concrete state class utility macros
#define CONCRETE_USER_AGENT_CALL_DIALOG_STATE_CLASS(cls, name)                          \
struct UserAgentCallDialog::cls : public UserAgentCallDialog::State                     \
{                                                                                     \
    ~cls() { }                                                                          \
    const std::string Name(void) const { return std::string(#name); }                   \
    void Abort(UserAgentCallDialog& dialog, bool force) const;                          \
    void ProcessRequest(UserAgentCallDialog& dialog, const SipMessage& msg) const;      \
    void ProcessResponse(UserAgentCallDialog& dialog, const SipMessage& msg) const;     \
    void TransactionError(UserAgentCallDialog& dialog) const;                           \
    void TimerExpired(UserAgentCallDialog& dialog, TimerId which) const;                \
    void MediaControlComplete(UserAgentCallDialog& dialog, bool success) const;         \
}

#ifdef _DEFINE_USER_AGENT_CALL_DIALOG_STATES_
# define CONCRETE_USER_AGENT_CALL_DIALOG_STATE(cls, var) CONCRETE_USER_AGENT_CALL_DIALOG_STATE_CLASS(cls, var); const UserAgentCallDialog::cls UserAgentCallDialog::var
#else
# define CONCRETE_USER_AGENT_CALL_DIALOG_STATE(cls, var) CONCRETE_USER_AGENT_CALL_DIALOG_STATE_CLASS(cls, var)
#endif

// Concrete state classes
CONCRETE_USER_AGENT_CALL_DIALOG_STATE(CallingState, CALLING_STATE);
CONCRETE_USER_AGENT_CALL_DIALOG_STATE(AnsweringState, ANSWERING_STATE);
CONCRETE_USER_AGENT_CALL_DIALOG_STATE(OffHookState, OFF_HOOK_STATE);
CONCRETE_USER_AGENT_CALL_DIALOG_STATE(LocalAbortingState, LOCAL_ABORTING_STATE);
CONCRETE_USER_AGENT_CALL_DIALOG_STATE(RemoteAbortingState, REMOTE_ABORTING_STATE);
CONCRETE_USER_AGENT_CALL_DIALOG_STATE(InactiveState, INACTIVE_STATE);

///////////////////////////////////////////////////////////////////////////////

END_DECL_SIPLEAN_NS

#endif
