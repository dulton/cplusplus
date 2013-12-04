/// @file
/// @brief SIP application class header
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _SIP_APPLICATION_H_
#define _SIP_APPLICATION_H_

#include <vector>

#include <app/AppCommon.h>
#include <boost/function.hpp>
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include <ifmgrclient/InterfaceEnable.h>

#include "AsyncCompletionToken.h"
#include "SipCommon.h"
#include "UserAgentConfig.h"

// Forward declarations (global)
class ACE_Reactor;
class ACE_Time_Value;

#ifdef UNIT_TEST
class TestSipApplication;
class TestStatefulSip;
#endif

DECL_CLASS_FORWARD_VOIP_NS(AsyncCompletionHandler);
DECL_CLASS_FORWARD_MEDIA_NS(VoIPMediaManager);
DECL_CLASS_FORWARD_SIP_NS(SipUri);
DECL_CLASS_FORWARD_SIP_NS(UserAgentFactory);

#ifdef UNIT_TEST
DECL_CLASS_FORWARD_APP_NS(UserAgentRawStats);
#endif

DECL_APP_NS

class UserAgentConfig;

///////////////////////////////////////////////////////////////////////////////

class SipApplication : boost::noncopyable
{
  public:
    explicit SipApplication(uint16_t numPorts);
    ~SipApplication();

    void Activate(ACE_Reactor& reactor, 
		  VOIP_NS::AsyncCompletionHandler *asyncCompletionHandler, 
		  MEDIA_NS::VoIPMediaManager *voipMediaManager);
    void Deactivate(void);

    // Convenience typedefs
    typedef std::vector<uint32_t> handleList_t;
    typedef std::vector<UserAgentConfig> uaConfigVec_t;
    typedef boost::function2<void, uint32_t, bool> loadProfileNotifier_t;
    typedef boost::function3<void, uint32_t, Sip_1_port_server::EnumSipUaRegState, Sip_1_port_server::EnumSipUaRegState> regStateNotifier_t;
    
    /// Application methods
    void ConfigUserAgents(uint16_t port, const uaConfigVec_t& uaConfigs, handleList_t& handles);
    void UpdateUserAgents(uint16_t port, const handleList_t& handles, const uaConfigVec_t& uaConfigs);
    void DeleteUserAgents(uint16_t port, const handleList_t& handles);

    void RegisterUserAgentLoadProfileNotifier(uint16_t port, const handleList_t& handles, loadProfileNotifier_t notifier);
    void UnregisterUserAgentLoadProfileNotifier(uint16_t port, const handleList_t& handles);
    
    void RegisterUserAgents(uint16_t port, const handleList_t& handles);
    void UnregisterUserAgents(uint16_t port, const handleList_t& handles);
    void CancelUserAgentsRegistrations(uint16_t port, const handleList_t& handles);
    
    void RegisterUserAgentRegStateNotifier(uint16_t port, const handleList_t& handles, regStateNotifier_t notifier);
    void UnregisterUserAgentRegStateNotifier(uint16_t port, const handleList_t& handles);

    void ResetProtocol(uint16_t port, const handleList_t& handles);
    void StartProtocol(uint16_t port, const handleList_t& handles);
    void StopProtocol(uint16_t port, const handleList_t& handles);
    
    void ClearResults(uint16_t port, const handleList_t& handles, uint64_t absExecTime);
    void SyncResults(uint16_t port);

    /// Interface notification methods
    void NotifyInterfaceEnablePending(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableVec);
    void NotifyInterfaceUpdatePending(uint16_t port, const std::vector<uint32_t>& ifHandleVec);
    void NotifyInterfaceUpdateCompleted(uint16_t port, const std::vector<uint32_t>& ifHandleVec);
    void NotifyInterfaceDeletePending(uint16_t port, const std::vector<uint32_t>& ifHandleVec);

    /// Async completion methods
    void DispatchAsyncCompletion(WeakAsyncCompletionToken weakAct, int data);
    
  private:
    /// Implementation-private port context data structure
    struct PortContext;

    typedef boost::function1<void, const SIP_NS::SipUri&> contactAddressConsumer_t;
    void EnumerateContactAddresses(uint16_t port, uint32_t bllHandle, const contactAddressConsumer_t& consumer) const;
    
    void DoStatsClear(uint16_t port, const handleList_t& handles);
    void StatsClearTimeoutHandler(const ACE_Time_Value& tv, const void *act);

#ifdef UNIT_TEST
    void GetUserAgentStats(uint16_t port, const handleList_t& handles, std::vector<UserAgentRawStats>& stats) const;
#endif    
    
    ACE_Reactor *mReactor;                              ///< application reactor instance
    VOIP_NS::AsyncCompletionHandler *mAsyncCompletionHandler; ///< asynchronous completion handler
    boost::scoped_ptr<SIP_NS::UserAgentFactory> mUserAgentFactory;      ///SIP User Agent Factory
    std::vector<PortContext> mPorts;                    ///< per-port data structures
    uint16_t mUserAgentBlockCounter;                    ///< counter for user agent block ID

#ifdef UNIT_TEST
    friend class ::TestSipApplication;
    friend class ::TestStatefulSip;
#endif    
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_APP_NS

#endif
