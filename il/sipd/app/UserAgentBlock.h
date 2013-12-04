/// @file
/// @brief SIP User Agent Block class header
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _USER_AGENT_BLOCK_H_
#define _USER_AGENT_BLOCK_H_

#include <deque>
#include <memory>
#include <string>
#include <vector>

#include <ace/Time_Value.h>
#include <base/BaseCommon.h>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>
#include <Tr1Adapter.h>
#include <utils/AgingQueue.h>
#include <ildaemon/ilDaemonCommon.h>

#include "AsyncCompletionToken.h"
#include "VoIPCommon.h"
#include "UserAgent.h"
#include "UserAgentConfig.h"
#include "UserAgentRawStats.h"
#include "SipUri.h"

// Forward declarations (global)
class ACE_Reactor;

namespace L4L7_BASE_NS
{
    class LoadProfile;
    class LoadScheduler;
}
namespace IL_DAEMON_LIB_NS
{
    class RemoteInterfaceEnumerator;
}

#ifdef UNIT_TEST
class TestUserAgentBlock;
class TestStatefulSip;
class TestSipDynProxy;
#endif

// Forward declarations
DECL_CLASS_FORWARD_VOIP_NS(AsyncCompletionHandler);
DECL_CLASS_FORWARD_SIP_NS(UserAgentFactory);

DECL_APP_NS

class UserAgentNameEnumerator;

///////////////////////////////////////////////////////////////////////////////

class UserAgentBlock : boost::noncopyable
{
  public:
    typedef UserAgentConfig config_t;
    typedef UserAgentRawStats stats_t;
    typedef Sip_1_port_server::EnumSipUaRegState regState_t;
    typedef boost::function1<void, const SIP_NS::SipUri&> contactAddressConsumer_t;
    typedef boost::function2<void, uint32_t, const contactAddressConsumer_t&> contactAddressResolver_t;
    typedef boost::function2<void, uint32_t, bool> loadProfileNotifier_t;
    typedef boost::function3<void, uint32_t, regState_t, regState_t> regStateNotifier_t;

    UserAgentBlock(uint16_t port, uint16_t vdevblock, const config_t& config, ACE_Reactor& reactor, 
		   VOIP_NS::AsyncCompletionHandler *asyncCompletionHandler, 
		   SIP_NS::UserAgentFactory &userAgentFactory);
    ~UserAgentBlock();

    uint16_t Port(void) const { return mPort; }
    uint16_t VDevBlock(void) const { return mVDevBlock; }
    
    // Handle accessors
    uint32_t BllHandle(void) const { return mConfig.Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle; }
    uint32_t IfHandle(void) const { return mConfig.Config().Endpoint.SrcIfHandle; }

    // Config accessors
    const config_t& Config(void) const { return mConfig; }
    size_t TotalCount(void) const { return mUaVec.size(); }
    const std::string& Name(void) const { return mConfig.Config().ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName; }
    void EnumerateContactAddresses(const contactAddressConsumer_t& consumer) const;
    
    // Load profile notifier methods
    void RegisterLoadProfileNotifier(loadProfileNotifier_t notifier) { mLoadProfileNotifier = notifier; }
    void UnregisterLoadProfileNotifier(void) { mLoadProfileNotifier.clear(); }
    
    // Registration state notifier methods
    void RegisterRegStateNotifier(regStateNotifier_t notifier) { mRegStateNotifier = notifier; }
    void UnregisterRegStateNotifier(void) { mRegStateNotifier.clear(); }
    
    // Stats accessors
    stats_t& Stats(void) { return mStats; }

    // Reset registrations and calls
    void Reset(void);
    
    // Registration load generation commands
    void Register(const contactAddressResolver_t& contactAddressResolver = contactAddressResolver_t());
    void Unregister(void);
    void CancelRegistrations(void);

    // Protocol load generation methods
    void Start(void);
    void Stop(void);

    // Interface notification methods
    void NotifyInterfaceDisabled(void);
    void NotifyInterfaceEnabled(void);
    
  private:
    /// Implementation-private inner classes
    class RegistrationLoadStrategy;
    class ProtocolLoadStrategy;
    class CallsLoadStrategy;
    class CallsOverTimeLoadStrategy;

    /// Convenience typedefs
    typedef L4L7_BASE_NS::LoadProfile loadProfile_t;
    typedef L4L7_BASE_NS::LoadScheduler loadScheduler_t;
    typedef IL_DAEMON_LIB_NS::RemoteInterfaceEnumerator dstIfEnum_t;
    typedef std::vector<SIP_NS::uaSharedPtr_t> uaVec_t;
    typedef std::deque<SIP_NS::uaSharedPtr_t> uaIdleDeque_t;
    typedef L4L7_UTILS_NS::AgingQueue<SIP_NS::uaSharedPtr_t> uaAgingQueue_t;
    typedef std::vector<SIP_NS::SipUri> uriVec_t;
    
    /// Factory methods
    std::auto_ptr<RegistrationLoadStrategy> MakeRegistrationLoadStrategy(void);
    std::auto_ptr<ProtocolLoadStrategy> MakeProtocolLoadStrategy(void);

    /// Registration load strategy convenience methods
    bool IsRegistering(void) const { return (mRegState != sip_1_port_server::EnumSipUaRegState_UNREGISTERING); }
    bool IsUnregistering(void) const { return (mRegState == sip_1_port_server::EnumSipUaRegState_UNREGISTERING); }
    void StartRegistrations(size_t count);
    void StartUnregistrations(size_t count);
    void ChangeRegState(regState_t toState);
    
    /// Protocol load strategy convenience methods
    size_t ActiveCalls(void) const { return mUaAgingQueue.size(); }
    void InitiateCalls(size_t count);
    void AbortCalls(size_t count,bool bGraceful = false);

    // Load profile notification handler
    void NotifyLoadProfileRunning(bool running) const;
    
    // User agent status notification handler
    void NotifyUaStatus(size_t uaIndex, SIP_NS::UserAgent::StatusNotification status, const ACE_Time_Value& deltaTime);

    // Call complete callback
    void CallComplete(void);
    
    const uint16_t mPort;                                       ///< physical port number
    const uint16_t mVDevBlock;                                  ///< device block ID
    const config_t mConfig;                                     ///< UA config, profile, etc.
    ACE_Reactor * const mReactor;                               ///< ACE reactor instance
    VOIP_NS::AsyncCompletionHandler * const mAsyncCompletionHandler; ///< async completion handler

    bool mEnabled;                                              ///< block-level enable flag
    stats_t mStats;                                             ///< raw UA block stats
    uaVec_t mUaVec;                                             ///< UA objects

    const bool mUsingProxy;                                     ///< when true, UA's must register with proxy server
    const bool mUsingMobile;                                    ///< when Mobile feature is enabled.
    const bool mUsingIMSI;                                      ///< when Mobile type is USIM.
    boost::scoped_ptr<RegistrationLoadStrategy> mRegStrategy;   ///< registration load strategy
    size_t mRegIndex;                                           ///< index of next UA to register
    size_t mRegOutstanding;                                     ///< registration requests outstanding
    size_t mUaNumsPerDevice;                                    ///< number of UAs per device
    size_t mDevicesTotal;                                       ///< number of devices
    boost::scoped_ptr<uriVec_t> mExtraContactAddresses;         ///< extra contact address list
    
    boost::scoped_ptr<loadProfile_t> mLoadProfile;              ///< protocol load profile
    boost::scoped_ptr<ProtocolLoadStrategy> mLoadStrategy;      ///< protocol load strategy
    boost::scoped_ptr<loadScheduler_t> mLoadScheduler;          ///< protocol load scheduler
    loadProfileNotifier_t mLoadProfileNotifier;                 ///< owner's load profile notifier delegate
    size_t mAttemptedCallCount;                                 ///< number of attempted calls
    
    boost::scoped_ptr<UserAgentNameEnumerator> mDstNameEnum;    ///< destination name enumerator
    boost::scoped_ptr<UserAgentNameEnumerator> mDstImsiEnum;    ///< destination imsi enumerator
    boost::scoped_ptr<UserAgentNameEnumerator> mDstRanIdEnum;   ///< destination RAN ID enumerator
    boost::scoped_ptr<dstIfEnum_t> mDstIfEnum;                  ///< destination IP interface enumerator (UA-to-UA)

    uaIdleDeque_t mUaIdleDeque;                                 ///< UA idle deque, ordered most idle first
    uaAgingQueue_t mUaAgingQueue;                               ///< UA aging queue, ordered oldest call first
    VOIP_NS::AsyncCompletionToken mCallCompleteAct;             ///< call completion async completion token
    
    regState_t mRegState;                                       ///< externally visible block registration state
    regStateNotifier_t mRegStateNotifier;                       ///< owner's registration state notifier delegate

    SIP_NS::UserAgentFactory &mUserAgentFactory;

#ifdef UNIT_TEST
    friend class ::TestUserAgentBlock;
    friend class ::TestStatefulSip;
    friend class ::TestSipDynProxy;
#endif
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_APP_NS

#endif
