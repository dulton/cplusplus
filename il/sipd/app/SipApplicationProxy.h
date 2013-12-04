/// @file
/// @brief SIP application proxy header
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _SIP_APPLICATION_PROXY_H_
#define _SIP_APPLICATION_PROXY_H_

#include <vector>

#include <ildaemon/Application.h>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <ifmgrclient/InterfaceEnable.h>
#include <ildaemon/ilDaemonCommon.h>

#include "VoIPCommon.h"
#include "AsyncCompletionToken.h"

// Forward declarations (global)
namespace IL_DAEMON_LIB_NS
{
    class ActiveScheduler;
}

DECL_CLASS_FORWARD_VOIP_NS(AsyncCompletionHandler);
DECL_CLASS_FORWARD_MEDIA_NS(VoIPMediaManager);

DECL_APP_NS

///////////////////////////////////////////////////////////////////////////////

// Forward declarations
class SipApplication;
class SipMsgSetSrv_1;

class SipApplicationProxy : public IL_DAEMON_LIB_NS::Application
{
  public:
    IL_APPLICATION_CTOR_DTOR(SipApplicationProxy);
    
  private:
    // Application hooks from IL_DAEMON_LIB_NS::Application interface
    bool InitializeHook(uint16_t ports, const IL_DAEMON_LIB_NS::CommandLineOpts& opts);
    bool RegisterMPSHook(MPS *mps);
    void UnregisterMPSHook(MPS *mps);
    bool ActivateHook(void);
    void DeactivateHook(void);
    bool DeferActivation(void) const { return true; }

  public:
    // Convenience typedefs
    typedef std::vector<uint32_t> handleList_t;
    typedef Sip_1_port_server::SipUaConfig_t sipUaConfig_t;
    typedef std::vector<sipUaConfig_t> sipUaConfigVec_t;
    typedef boost::function2<void, uint32_t, bool> loadProfileNotifier_t;
    typedef boost::function3<void, uint32_t, Sip_1_port_server::EnumSipUaRegState, Sip_1_port_server::EnumSipUaRegState> regStateNotifier_t;
    
    /// Application methods
    void ConfigUserAgents(uint16_t port, const sipUaConfigVec_t& userAgents, handleList_t& handles);
    void UpdateUserAgents(uint16_t port, const handleList_t& handles, const sipUaConfigVec_t& userAgents);
    void DeleteUserAgents(uint16_t port, const handleList_t& handles);

    void RegisterUserAgentLoadProfileNotifier(uint16_t port, const handleList_t& handles, loadProfileNotifier_t notifier);
    void UnregisterUserAgentLoadProfileNotifier(uint16_t port, const handleList_t& handles);
    
    void RegisterUserAgents(uint16_t port, const handleList_t& handles);
    void UnregisterUserAgents(uint16_t port, const handleList_t& handles);
    void CancelUserAgentsRegistrations(uint16_t port, const handleList_t& handles);

    void RegisterUserAgentRegStateNotifier(uint16_t port, const handleList_t& handles, regStateNotifier_t notifier);
    void UnregisterUserAgentRegStateNotifier(uint16_t port, const handleList_t&handles);

    void ResetProtocol(uint16_t port, const handleList_t& handles);
    void StartProtocol(uint16_t port, const handleList_t& handles);
    void StopProtocol(uint16_t port, const handleList_t& handles);
    
    void ClearResults(uint16_t port, const handleList_t& handles, uint64_t absExecTime);
    void SyncResults(uint16_t port);
    
    void NotifyInterfaceEnablePending(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableVec);
    void NotifyInterfaceUpdatePending(uint16_t port, const std::vector<uint32_t>& ifHandleVec);
    void NotifyInterfaceUpdateCompleted(uint16_t port, const std::vector<uint32_t>& ifHandleVec);
    void NotifyInterfaceDeletePending(uint16_t port, const std::vector<uint32_t>& ifHandleVec);
    
  private:
    /// Implementation-private functor
    struct MakeUserAgentConfig;
    
    /// Async completion methods
    void DispatchAsyncCompletion(WeakAsyncCompletionToken weakAct, int data);
    
    boost::scoped_ptr<SipMsgSetSrv_1> mMsgSet;                          ///< message set handlers
    boost::scoped_ptr<AsyncCompletionHandler> mAsyncCompletionHandler;  ///< asynchronous completion handler
    boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> mVoipMediaManager;    ///Media manager
    boost::scoped_ptr<IL_DAEMON_LIB_NS::ActiveScheduler> mScheduler;    ///< active object scheduler
    boost::scoped_ptr<SipApplication> mApp;                             ///< application object that we are proxying for
    uint16_t mNumPorts;                                                 ///< number of ports
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_APP_NS

#endif
