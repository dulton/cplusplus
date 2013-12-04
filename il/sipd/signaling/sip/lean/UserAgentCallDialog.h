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

#ifndef _USER_AGENT_CALL_DIALOG_H_
#define _USER_AGENT_CALL_DIALOG_H_

#include <stdint.h>

#include <list>

#include <ace/Time_Value.h>
#include <ildaemon/GenericTimer.h>
#include <boost/scoped_ptr.hpp>

#include "SipCommon.h"
#include "SipMessage.h"
#include "SipSessionExpiration.h"
#include "UserAgentDialog.h"

// Forward declarations (global)
class ACE_Reactor;

DECL_SIPLEAN_NS

///////////////////////////////////////////////////////////////////////////////

// Forward declarations
class SdpMessage;

class UserAgentCallDialog : public UserAgentDialog
{
  public:
    // Convenience typedefs
    typedef boost::function1<void, bool> inviteCompletionDelegate_t;
    typedef boost::function0<void> answerCompletionDelegate_t;
    typedef boost::function0<void> callCompletionDelegate_t;
    typedef boost::function0<void> refreshCompletionDelegate_t;
    typedef boost::function1<void, SdpMessage&> makeMediaOfferDelegate_t;
    typedef boost::function1<bool, const SdpMessage&> consumeMediaOfferDelegate_t;
    typedef boost::function2<void, bool, bool> mediaControlDelegate_t;

    //Supported default refresher options, we do not support the "toggle" option:
    enum DefaultRefresherOptionType {
      DEFAULT_UAC_REFRESHER=0,
      DEFAULT_UAS_REFRESHER
    };
    
    UserAgentCallDialog(const uint16_t port, UserAgentBinding& binding, const ACE_INET_Addr& peer, 
			const SipUri& remoteUri, ACE_Reactor& reactor, bool usingProxy);
    UserAgentCallDialog(const uint16_t port, UserAgentBinding& binding, const SipMessage& msg, 
			ACE_Reactor& reactor, bool usingProxy);
    ~UserAgentCallDialog();

    // Ring/call time mutators
    void SetRingTime(const ACE_Time_Value& ringTime) { mRingTime = ringTime; }
    void SetCallTime(const ACE_Time_Value& callTime) { mCallTime = callTime; }
    
    // Delegate mutators
    void SetInviteCompletionDelegate(const inviteCompletionDelegate_t& delegate) { mNotifyInviteComplete = delegate; }
    void SetAnswerCompletionDelegate(const answerCompletionDelegate_t& delegate) { mNotifyAnswerComplete = delegate; }
    void SetCallCompletionDelegate(const callCompletionDelegate_t& delegate) { mNotifyCallComplete = delegate; }
    void SetRefreshCompletionDelegate(const refreshCompletionDelegate_t& delegate) { mNotifyRefreshComplete = delegate; }
    void SetMakeMediaOfferDelegate(const makeMediaOfferDelegate_t& delegate) { mMakeMediaOffer = delegate; }
    void SetConsumeMediaOfferDelegate(const consumeMediaOfferDelegate_t& delegate) { mConsumeMediaOffer = delegate; }
    void SetMediaControlDelegate(const mediaControlDelegate_t& delegate) { mMediaControl = delegate; }
    
    // Minimum session expiration mutator
    void SetMinSessionExpiration(uint32_t minSessionExpires);
    
    // Control methods
    bool StartInvite(void) { return SendInvite(); }
    
    // Call dialog-specific accessors (from UserAgentDialog)
    bool Active(void) const { return mState != reinterpret_cast<const State *>(&INACTIVE_STATE); }
    
    // Call dialog-specific controls (from UserAgentDialog)
    void Abort(void);
    
    // Call dialog-specific events (from UserAgentDialog)
    void ProcessRequest(const SipMessage& msg);
    void ProcessResponse(const SipMessage& msg);
    void TransactionError(void);

    // Media control events
    void MediaControlComplete(bool success);
    void MediaControlCompleteDelayed(bool generate,bool success);

    void SetDefaultRefresherOption(DefaultRefresherOptionType value);
    DefaultRefresherOptionType GetDefaultRefresherOption();
    
private:
    // Convenience typedefs
    typedef std::list<std::string> routeList_t;

    // Private inner state class forward declarations (see UserAgentCallDialogState.h)
    struct State;
    friend struct State;
    struct CallingState;
    struct AnsweringState;
    struct OffHookState;
    struct LocalAbortingState;
    struct RemoteAbortingState;
    struct InactiveState;

    // Timer id's
    enum TimerId
    {
        RING_TIMER = 1,
        _100REL_TIMER,
        ACK_WAIT_TIMER,
        SESSION_TIMER,
        CALL_TIMER,
	MEDIA_TIMER
    };
    
    // Private accessors/mutators
    bool Caller(void) const { return mCaller; }
    uint32_t InviteSeq(void) const { return mInviteSeq; }
    uint32_t InviteSeq(uint32_t inviteSeq) { return (mInviteSeq = inviteSeq); }
    
    // Session expiration methods
    bool UsingSessionRefresh(void) const { return mSessionExpires != 0; }
    bool SessionRefresher(void) const { return mSessionRefresher && UsingSessionRefresh(); }
    
    // Request-URI method
    const SipUri BuildRequestUri(void) const
    {
        // See RFC 3261, section 12.2.1.1 for Request-URI rules
        if (!mRouteList.empty())
        {
            if (!mLooseRouting)
    {
                SipUri uri;
                SipProtocol::ParseAddressString(mRouteList.front(), &uri);
                return uri;
            }
            else
                return RemoteTarget();
        }
        else
            return RemoteTarget();
    }
    
    // Route list methods
    bool HasRouteList(void) const { return !mRouteList.empty(); }
    const std::string BuildRouteString(void) const;

    // Update methods
    void UpdateDialog(const SipMessage& msg);
    void UpdateSessionExpiration(const SipMessage &msg);
    void UpdateRouteList(const SipMessage& msg);
            
    // Client protocol methods
    bool SendInvite(void);
    bool SendPrack(uint32_t rseq, const SipMessage& resp);
    bool SendAcknowledge(void);
    bool SendUpdate(void);
    bool SendBye(void);

    // Server protocol methods
    bool SendResponse(uint16_t statusCode, const::string& message, const SipMessage *req = 0);

    // Central state change method
    void ChangeState(const State *toState);

    // Timer expiration methods
    void TimerExpired(TimerId which);

    //Build first route header for loose-routing (RFC3261, section 12.2.1.1)
    const std::string BuildFirstRoute();
    
    // Class data
    static const CallingState CALLING_STATE;
    static const AnsweringState ANSWERING_STATE;
    static const OffHookState OFF_HOOK_STATE;
    static const LocalAbortingState LOCAL_ABORTING_STATE;
    static const RemoteAbortingState REMOTE_ABORTING_STATE;
    static const InactiveState INACTIVE_STATE;

    static const uint32_t MIN_SESSION_EXPIRES_TIME;
    static const uint32_t DEFAULT_SESSION_EXPIRES_TIME;
    static const ACE_Time_Value TIMER_T1;
    static const ACE_Time_Value TIMER_T2;

    const bool mCaller;

    uint32_t mInviteSeq; 
    uint32_t mSessionExpires;                           //< current session expiration value
    uint32_t mMinSessionExpires;                        //< maximum of Min-SE values heard
    bool mSessionRefresher;                             //< when true this UA is the session refresher
    routeList_t mRouteList;                             //< route list
    bool mLooseRouting;                                 //< when true route set is using LR

    const State *mState;                                //< current state
    
    ACE_Time_Value mRTT;                                //< current round-trip-time estimate (100rel or 2xx retransmissions)
    std::pair<uint32_t, uint32_t> mRSeqState;           //< first, last transmitted 100rel sequence number
    std::pair<bool, uint32_t> mRAckState;               //< 100rel response pending, last received 100rel sequence number
    
    IL_DAEMON_LIB_NS::GenericTimer mRingTimer;          //< ring (wait to answer) timer
    IL_DAEMON_LIB_NS::GenericTimer m100RelTimer;        //< 100rel (UAS 1xx retransmission) timer
    IL_DAEMON_LIB_NS::GenericTimer mMediaTimer;        //< Media timer
    bool mMediaLastResult;                             //Keeps result for the timer
    IL_DAEMON_LIB_NS::GenericTimer mAckWaitTimer;       //< ACK wait (UAS 2xx retransmission) timer
    uint64_t mAckWaitTimerTimeout;                    //Start time when we started ACK waiting
    IL_DAEMON_LIB_NS::GenericTimer mSessionTimer;       //< session refresh timer
    IL_DAEMON_LIB_NS::GenericTimer mCallTimer;          //< call timer

    boost::scoped_ptr<SipMessage> mHeldRequest;         //< current request (for UAS deferred responses)
    boost::scoped_ptr<SipMessage> mHeldStatus;          //< current status (for 100rel retransmissions)

    ACE_Time_Value mRingTime;                           //< ring duration (callee)
    ACE_Time_Value mCallTime;                           //< call duration (caller)
    
    inviteCompletionDelegate_t mNotifyInviteComplete;   //< invite completion delegate (caller: CALLING->OFF_HOOK)
    answerCompletionDelegate_t mNotifyAnswerComplete;   //< answer completion delegate (callee: ANSWERING->OFF_HOOK)
    callCompletionDelegate_t mNotifyCallComplete;       //< call completion delegate (caller: OFF_HOOK->*)
    refreshCompletionDelegate_t mNotifyRefreshComplete; //< refreshe completion delegate (caller/callee: received 2xx to UPDATE)
    makeMediaOfferDelegate_t mMakeMediaOffer;           //< make media offer (SDP) delegate
    consumeMediaOfferDelegate_t mConsumeMediaOffer;     //< consume media offer (SDP) delegate
    mediaControlDelegate_t mMediaControl;               //< media control (start/stop) delegate
    DefaultRefresherOptionType mDefaultRefresherOption; //See RFC 4028, Table 2, line 4
    bool mUsingProxy;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_SIPLEAN_NS

#endif
