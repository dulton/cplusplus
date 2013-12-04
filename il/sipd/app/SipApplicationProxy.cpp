/// @file
/// @brief SIP Application Proxy implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <algorithm>
#include <functional>

#include <ildaemon/ActiveScheduler.h>
#include <ildaemon/BoundMethodRequestImpl.tcc>
#include <ildaemon/CommandLineOpts.h>
#include <ildaemon/ThreadSpecificStorage.h>
#include <boost/bind.hpp>
#include <statsframework/StatsDBFactory.h>

#include "AsyncCompletionHandler.h"
#include "VoIPMediaManager.h"
#include "SipApplication.h"
#include "SipApplicationProxy.h"
#include "SipMsgSetSrv_1.h"
#include "UserAgentConfig.h"

using MEDIA_NS::VoIPMediaManager;

USING_APP_NS;

///////////////////////////////////////////////////////////////////////////////

IL_APPLICATION_REGISTRATION(SipApplicationProxy, "SIP");

///////////////////////////////////////////////////////////////////////////////

bool SipApplicationProxy::InitializeHook(uint16_t numPorts, const IL_DAEMON_LIB_NS::CommandLineOpts& opts)
{
    if (opts.NumIOThreadsPerPort())
        TC_LOG_WARN(0, "SipApplication does not use I/O threads, ignoring -n command line parameter");

    mApp.reset(new SipApplication(numPorts));
    mMsgSet.reset(new SipMsgSetSrv_1(*this));
    mAsyncCompletionHandler.reset(new AsyncCompletionHandler);
    mVoipMediaManager.reset(new VoIPMediaManager());
    mVoipMediaManager->initialize(TSS_Reactor);
    mScheduler.reset(new IL_DAEMON_LIB_NS::ActiveScheduler);
    mNumPorts = numPorts;
    
    AddMessageSet(*mMsgSet);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool SipApplicationProxy::RegisterMPSHook(MPS *mps)
{
    return mMsgSet->RegisterMPSHook(mps);
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::UnregisterMPSHook(MPS *mps)
{
    mMsgSet->UnregisterMPSHook(mps);
}

///////////////////////////////////////////////////////////////////////////////

bool SipApplicationProxy::ActivateHook(void)
{
    methodCompletionDelegate_t activateDelegate = 
      boost::bind(&SipApplicationProxy::DispatchAsyncCompletion, this, _1, _2);
    mAsyncCompletionHandler->SetMethodCompletionDelegate(activateDelegate);
    mVoipMediaManager->Activate(activateDelegate);
    
    if (mScheduler->Start() == -1)
    {
        TC_LOG_ERR(0, "Failed to start SipApplication active object");
        throw EPHXInternal("[SipApplicationProxy::ActivateHook] Failed to start SipApplication active object");
    }

    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Activate, mApp.get(), boost::ref(*mScheduler->AppReactor()), mAsyncCompletionHandler.get(), mVoipMediaManager.get());
    mScheduler->Enqueue(req);
    req->Wait();
    
    for (uint16_t port = 0; port < mNumPorts; port++)
    {
        StatsDBFactory * const dbFactory = StatsDBFactory::instance(port);
        dbFactory->getSqlMsgAdapter()->setPreQueryDelegate(boost::bind(&SipApplicationProxy::SyncResults, this, port));
    }
    
    TC_LOG_WARN(0, "Activated SipApplication");
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::DeactivateHook(void)
{
    mAsyncCompletionHandler->ClearMethodCompletionDelegate();
    mVoipMediaManager->Deactivate();
    
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Deactivate, mApp.get());
    mScheduler->Enqueue(req);
    req->Wait();
    
    if (mScheduler->Stop() == -1)
    {
        TC_LOG_ERR(0, "Failed to stop SipApplication active object");
    }
}

///////////////////////////////////////////////////////////////////////////////

struct SipApplicationProxy::MakeUserAgentConfig : public std::unary_function<sipUaConfig_t, UserAgentConfig>
{
    MakeUserAgentConfig(uint16_t port, VoIPMediaManager &voipMediaManager) 
        : mPort(port),
          mVoipMediaManager(voipMediaManager)
    {
    }
    
    const UserAgentConfig operator()(const sipUaConfig_t& config) const
    {
        return UserAgentConfig(mPort, config, &mVoipMediaManager);
    }

private:
    const uint16_t mPort;
    VoIPMediaManager& mVoipMediaManager;
};

///////////////////////////////////////////////////////////////////////////////
    
void SipApplicationProxy::ConfigUserAgents(uint16_t port, const sipUaConfigVec_t& userAgents, handleList_t& handles)
{
    Activate();
    
    // Transform incoming IDL-oriented configs into fully populated (with stream index lists) UserAgentConfig objects
    std::vector<UserAgentConfig> uaConfigs;
    uaConfigs.reserve(userAgents.size());
    transform(userAgents.begin(), userAgents.end(), std::back_inserter(uaConfigs), MakeUserAgentConfig(port, *mVoipMediaManager));
    
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::ConfigUserAgents, mApp.get(), port, boost::cref(uaConfigs), boost::ref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::UpdateUserAgents(uint16_t port, const handleList_t& handles, const sipUaConfigVec_t& userAgents)
{
    Activate();

    // Transform incoming IDL-oriented configs into fully populated (with stream index lists) UserAgentConfig objects
    std::vector<UserAgentConfig> uaConfigs;
    uaConfigs.reserve(userAgents.size());
    transform(userAgents.begin(), userAgents.end(), std::back_inserter(uaConfigs), MakeUserAgentConfig(port, *mVoipMediaManager));

    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::UpdateUserAgents, mApp.get(), port, boost::cref(handles), boost::cref(uaConfigs));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::DeleteUserAgents(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::DeleteUserAgents, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::RegisterUserAgentLoadProfileNotifier(uint16_t port, const handleList_t& handles, loadProfileNotifier_t notifier)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::RegisterUserAgentLoadProfileNotifier, mApp.get(), port, boost::cref(handles), notifier);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::UnregisterUserAgentLoadProfileNotifier(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::UnregisterUserAgentLoadProfileNotifier, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////
    
void SipApplicationProxy::RegisterUserAgents(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::RegisterUserAgents, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::UnregisterUserAgents(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::UnregisterUserAgents, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::CancelUserAgentsRegistrations(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::CancelUserAgentsRegistrations, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::RegisterUserAgentRegStateNotifier(uint16_t port, const handleList_t& handles, regStateNotifier_t notifier)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::RegisterUserAgentRegStateNotifier, mApp.get(), port, boost::cref(handles), notifier);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::UnregisterUserAgentRegStateNotifier(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::UnregisterUserAgentRegStateNotifier, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::ResetProtocol(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::ResetProtocol, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::StartProtocol(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::StartProtocol, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////
    
void SipApplicationProxy::StopProtocol(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::StopProtocol, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::ClearResults(uint16_t port, const handleList_t& handles, uint64_t absExecTime)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::ClearResults, mApp.get(), port, boost::cref(handles), absExecTime);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::SyncResults(uint16_t port)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::SyncResults, mApp.get(), port);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::NotifyInterfaceEnablePending(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::NotifyInterfaceEnablePending, mApp.get(), port, boost::cref(ifEnableList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::NotifyInterfaceUpdatePending(uint16_t port, const handleList_t& ifHandleList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::NotifyInterfaceUpdatePending, mApp.get(), port, boost::cref(ifHandleList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::NotifyInterfaceUpdateCompleted(uint16_t port, const handleList_t& ifHandleList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::NotifyInterfaceUpdateCompleted, mApp.get(), port, boost::cref(ifHandleList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::NotifyInterfaceDeletePending(uint16_t port, const handleList_t& ifHandleList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::NotifyInterfaceDeletePending, mApp.get(), port, boost::cref(ifHandleList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplicationProxy::DispatchAsyncCompletion(WeakAsyncCompletionToken weakAct, int data)
{
    IL_DAEMON_LIB_MAKE_ASYNC_REQUEST(req, void, &SipApplication::DispatchAsyncCompletion, mApp.get(), weakAct, data);
    mScheduler->Enqueue(req);
}

///////////////////////////////////////////////////////////////////////////////
