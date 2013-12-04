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

#ifndef _USER_AGENT_REG_DIALOG_H_
#define _USER_AGENT_REG_DIALOG_H_

#include <string>

#include <ace/Time_Value.h>
#include <ildaemon/GenericTimer.h>

#include "SipCommon.h"
#include "UserAgentDialog.h"

// Forward declarations (global)
class ACE_Reactor;

DECL_SIPLEAN_NS

///////////////////////////////////////////////////////////////////////////////

class UserAgentRegDialog : public UserAgentDialog
{
  public:
    typedef boost::function2<void, uint32_t, bool> completionDelegate_t;
    
    UserAgentRegDialog(const uint16_t port, UserAgentBinding& binding, const ACE_INET_Addr& peer, const std::string& domain, ACE_Reactor& reactor);
    ~UserAgentRegDialog();

    // Registration refresh mutator
    void SetRefresh(bool enable) { mRefresh = enable; }
    
    // Delegate mutators
    void SetCompletionDelegate(const completionDelegate_t& delegate) { mNotifyComplete = delegate; }

    // Control methods
    bool StartRegister(uint32_t regExpires);
    
    // Registration dialog-specific accessors (from UserAgentDialog)
    bool Active(void) const { return mActive; }
    
    // Registration dialog-specific controls (from UserAgentDialog)
    void Abort(void);
    
    // Registration dialog-specific events (from UserAgentDialog)
    void ProcessRequest(const SipMessage& msg);
    void ProcessResponse(const SipMessage& msg);
    void TransactionError(void);

  private:
    // Protocol methods
    bool SendRegister(void);
    bool SendStatus(uint16_t statusCode, const::string& message, const SipMessage *req = 0);
    void UpdateExtensionHeaders(const SipMessage& msg);

    // Registration refresh timer expiration method
    void RefreshTimerExpired(void);
    
    const std::string& mDomain;                 //< location service domain
    bool mRefresh;                              //< automatically refresh registration?

    bool mActive;                               //< registration in progress?
    uint32_t mRegExpires;                       //< registration expiration
    IL_DAEMON_LIB_NS::GenericTimer mRefreshTimer;//< auto-refresh timer

    completionDelegate_t mNotifyComplete;       //< registration completion delegate
    bool mNotifiedComplete;                     //< true once owner has been notified of registration completion
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_SIPLEAN_NS

#endif
