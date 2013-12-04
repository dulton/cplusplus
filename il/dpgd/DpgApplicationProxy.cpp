/// @file
/// @brief Dpg Application Proxy implementation
///
///  Copyright (c) 2009 by Spirent Communications Inc.
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

#include "DpgApplication.h"
#include "DpgApplicationProxy.h"
#include "DpgdLog.h"
#include "DpgMsgSetSrv_1.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

IL_APPLICATION_REGISTRATION(DpgApplicationProxy, "DPG");

///////////////////////////////////////////////////////////////////////////////

bool DpgApplicationProxy::InitializeHook(uint16_t ports, const IL_DAEMON_LIB_NS::CommandLineOpts& opts)
{
    mNumPorts = ports;
    mApp.reset(new DpgApplication(ports));
    std::string messageSetName = std::string("");
    if (opts.NewMessageSetName())
    {
      messageSetName = opts.MessageSetName();
    }
    mMsgSet.reset(new DpgMsgSetSrv_1(*this, messageSetName));
    mScheduler.reset(new IL_DAEMON_LIB_NS::ActiveScheduler);

    mNumPorts = ports;
    mNumIOThreadsPerPort = opts.NumIOThreadsPerPort();
    if (mNumIOThreadsPerPort < 1)
    {
        TC_LOG_WARN(0, "DpgApplication must use at least one I/O thread per port, using one for now");
        mNumIOThreadsPerPort = 1;
    }

    AddMessageSet(*mMsgSet);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool DpgApplicationProxy::RegisterMPSHook(MPS *mps)
{
    return mMsgSet->RegisterMPSHook(mps);
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::UnregisterMPSHook(MPS *mps)
{
    mMsgSet->UnregisterMPSHook(mps);
}

///////////////////////////////////////////////////////////////////////////////

bool DpgApplicationProxy::ActivateHook(void)
{
    if (mScheduler->Start(mNumPorts, mNumIOThreadsPerPort) == -1)
    {
        TC_LOG_ERR(0, "Failed to start DpgApplication active object");
        throw EPHXInternal("[DpgApplicationProxy::ActivateHook] Failed to start DpgApplication active object");
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
    
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Activate, mApp.get(), appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
    mScheduler->Enqueue(req);
    req->Wait();
    
    for (uint16_t port = 0; port < mNumPorts; port++)
    {
        StatsDBFactory * const dbFactory = StatsDBFactory::instance(port);
        dbFactory->getSqlMsgAdapter()->setPreQueryDelegate(boost::bind(&DpgApplicationProxy::SyncResults, this, port));
    }

    TC_LOG_WARN(0, "Activated DpgApplication");
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::DeactivateHook(void)
{
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Deactivate, mApp.get());
    mScheduler->Enqueue(req);
    req->Wait();
    
    if (mScheduler->Stop() == -1)
    {
        TC_LOG_ERR(0, "Failed to stop DpgApplication active object");
    }
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::UpdateFlows(uint16_t port, const handleList_t& handles, const flowConfigList_t& flows)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::UpdateFlows, mApp.get(), port, boost::cref(handles), boost::cref(flows));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::DeleteFlows(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::DeleteFlows, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::UpdatePlaylists(uint16_t port, const handleList_t& handles, const playlistConfigList_t& playlists)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::UpdatePlaylists, mApp.get(), port, boost::cref(handles), boost::cref(playlists));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::DeletePlaylists(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::DeletePlaylists, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::ConfigClients(uint16_t port, const clientConfigList_t& clients, handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::ConfigClients, mApp.get(), port, boost::cref(clients), boost::ref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::UpdateClients(uint16_t port, const handleList_t& handles, const clientConfigList_t& clients)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::UpdateClients, mApp.get(), port, boost::cref(handles), boost::cref(clients));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::DeleteClients(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::DeleteClients, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::RegisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles, loadProfileNotifier_t notifier)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::RegisterClientLoadProfileNotifier, mApp.get(), port, boost::cref(handles), notifier);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::UnregisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::UnregisterClientLoadProfileNotifier, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::ConfigServers(uint16_t port, const serverConfigList_t& servers, handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::ConfigServers, mApp.get(), port, boost::cref(servers), boost::ref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::UpdateServers(uint16_t port, const handleList_t& handles, const serverConfigList_t& servers)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::UpdateServers, mApp.get(), port, boost::cref(handles), boost::cref(servers));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::DeleteServers(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::DeleteServers, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::StartProtocol(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::StartProtocol, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::StopProtocol(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::StopProtocol, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::ClearResults(uint16_t port, const handleList_t& handles, uint64_t absExecTime)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::ClearResults, mApp.get(), port, boost::cref(handles), absExecTime);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::SyncResults(uint16_t port)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::SyncResults, mApp.get(), port);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::SetDynamicLoad(uint16_t port, const loadPairList_t& loadList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::SetDynamicLoad, mApp.get(), port, boost::cref(loadList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::NotifyInterfaceEnablePending(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::NotifyInterfaceEnablePending, mApp.get(), port, boost::cref(ifEnableList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::NotifyInterfaceUpdatePending(uint16_t port, const handleList_t& ifHandleList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::NotifyInterfaceUpdatePending, mApp.get(), port, boost::cref(ifHandleList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void DpgApplicationProxy::NotifyInterfaceDeletePending(uint16_t port, const handleList_t& ifHandleList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::NotifyInterfaceDeletePending, mApp.get(), port, boost::cref(ifHandleList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////
