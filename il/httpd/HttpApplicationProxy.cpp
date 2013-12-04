/// @file
/// @brief HTTP Application Proxy implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <boost/bind.hpp>

#include <ildaemon/ActiveScheduler.h>
#include <ildaemon/BoundMethodRequestImpl.tcc>
#include <ildaemon/CommandLineOpts.h>
#include <statsframework/StatsDBFactory.h>

#include "HttpApplication.h"
#include "HttpApplicationProxy.h"
#include "HttpdLog.h"
#include "HttpMsgSetSrv_1.h"

USING_HTTP_NS;

///////////////////////////////////////////////////////////////////////////////

IL_APPLICATION_REGISTRATION(HttpApplicationProxy, "HTTP");

///////////////////////////////////////////////////////////////////////////////

bool HttpApplicationProxy::InitializeHook(uint16_t ports, const IL_DAEMON_LIB_NS::CommandLineOpts& opts)
{
    mNumPorts = ports;
    mApp.reset(new HttpApplication(ports));
    mMsgSet.reset(new HttpMsgSetSrv_1(*this));
    mScheduler.reset(new IL_DAEMON_LIB_NS::ActiveScheduler);

    mNumPorts = ports;
    mNumIOThreadsPerPort = opts.NumIOThreadsPerPort();
    if (mNumIOThreadsPerPort < 1)
    {
        TC_LOG_WARN(0, "HttpApplication must use at least one I/O thread per port, using one for now");
        mNumIOThreadsPerPort = 1;
    }

    AddMessageSet(*mMsgSet);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool HttpApplicationProxy::RegisterMPSHook(MPS *mps)
{
    return mMsgSet->RegisterMPSHook(mps);
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::UnregisterMPSHook(MPS *mps)
{
    mMsgSet->UnregisterMPSHook(mps);
}

///////////////////////////////////////////////////////////////////////////////

bool HttpApplicationProxy::ActivateHook(void)
{
    if (mScheduler->Start(mNumPorts, mNumIOThreadsPerPort) == -1)
    {
        TC_LOG_ERR(0, "Failed to start HttpApplication active object");
        throw EPHXInternal("[HttpApplicationProxy::ActivateHook] Failed to start HttpApplication active object");
    }

    ACE_Reactor *appReactor = mScheduler->AppReactor();

    std::vector<ACE_Reactor *> ioReactorVec;
    std::vector<ACE_Lock *> ioBarrierVec;
    
    ioReactorVec.reserve(mNumPorts);
    ioBarrierVec.reserve(mNumPorts);
    
    for (uint16_t port = 0; port < mNumPorts; port++)
    {
        ioReactorVec.push_back(mScheduler->IOReactor(port));
        ioBarrierVec.push_back(mScheduler->IOBarrier(port));
    }
    
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::Activate, mApp.get(), appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
    mScheduler->Enqueue(req);
    req->Wait();
    
    for (uint16_t port = 0; port < mNumPorts; port++)
    {
        StatsDBFactory * const dbFactory = StatsDBFactory::instance(port);
        dbFactory->getSqlMsgAdapter()->setPreQueryDelegate(boost::bind(&HttpApplicationProxy::SyncResults, this, port));
    }

    TC_LOG_WARN(0, "Activated HttpApplication");
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::DeactivateHook(void)
{
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::Deactivate, mApp.get());
    mScheduler->Enqueue(req);
    req->Wait();
    
    if (mScheduler->Stop() == -1)
    {
        TC_LOG_ERR(0, "Failed to stop HttpApplication active object");
    }
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::ConfigClients(uint16_t port, const clientConfigList_t& clients, handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::ConfigClients, mApp.get(), port, boost::cref(clients), boost::ref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::UpdateClients(uint16_t port, const handleList_t& handles, const clientConfigList_t& clients)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::UpdateClients, mApp.get(), port, boost::cref(handles), boost::cref(clients));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::DeleteClients(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::DeleteClients, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::RegisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles, loadProfileNotifier_t notifier)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::RegisterClientLoadProfileNotifier, mApp.get(), port, boost::cref(handles), notifier);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::UnregisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::UnregisterClientLoadProfileNotifier, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::ConfigServers(uint16_t port, const serverConfigList_t& servers, handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::ConfigServers, mApp.get(), port, boost::cref(servers), boost::ref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::UpdateServers(uint16_t port, const handleList_t& handles, const serverConfigList_t& servers)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::UpdateServers, mApp.get(), port, boost::cref(handles), boost::cref(servers));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::DeleteServers(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::DeleteServers, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::StartProtocol(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::StartProtocol, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::StopProtocol(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::StopProtocol, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::ClearResults(uint16_t port, const handleList_t& handles, uint64_t absExecTime)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::ClearResults, mApp.get(), port, boost::cref(handles), absExecTime);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::SyncResults(uint16_t port)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::SyncResults, mApp.get(), port);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::SetDynamicLoad(uint16_t port, const loadPairList_t& loadList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::SetDynamicLoad, mApp.get(), port, boost::cref(loadList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::NotifyInterfaceEnablePending(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::NotifyInterfaceEnablePending, mApp.get(), port, boost::cref(ifEnableList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::NotifyInterfaceUpdatePending(uint16_t port, const handleList_t& ifHandleList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::NotifyInterfaceUpdatePending, mApp.get(), port, boost::cref(ifHandleList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void HttpApplicationProxy::NotifyInterfaceDeletePending(uint16_t port, const handleList_t& ifHandleList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::NotifyInterfaceDeletePending, mApp.get(), port, boost::cref(ifHandleList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////
