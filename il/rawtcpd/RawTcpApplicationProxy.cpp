/// @file
/// @brief Raw tcp Application Proxy implementation
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

#include "RawTcpApplication.h"
#include "RawTcpApplicationProxy.h"
#include "RawtcpdLog.h"
#include "RawTcpMsgSetSrv_1.h"

USING_RAWTCP_NS;

///////////////////////////////////////////////////////////////////////////////

IL_APPLICATION_REGISTRATION(RawTcpApplicationProxy, "RAWTCP");

///////////////////////////////////////////////////////////////////////////////

bool RawTcpApplicationProxy::InitializeHook(uint16_t ports, const IL_DAEMON_LIB_NS::CommandLineOpts& opts)
{
    mNumPorts = ports;
    mApp.reset(new RawTcpApplication(ports));
    mMsgSet.reset(new RawTcpMsgSetSrv_1(*this));
    mScheduler.reset(new IL_DAEMON_LIB_NS::ActiveScheduler);

    mNumPorts = ports;
    mNumIOThreadsPerPort = opts.NumIOThreadsPerPort();
    if (mNumIOThreadsPerPort < 1)
    {
        TC_LOG_WARN(0, "RawTcpApplication must use at least one I/O thread per port, using one for now");
        mNumIOThreadsPerPort = 1;
    }

    AddMessageSet(*mMsgSet);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool RawTcpApplicationProxy::RegisterMPSHook(MPS *mps)
{
    return mMsgSet->RegisterMPSHook(mps);
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::UnregisterMPSHook(MPS *mps)
{
    mMsgSet->UnregisterMPSHook(mps);
}

///////////////////////////////////////////////////////////////////////////////

bool RawTcpApplicationProxy::ActivateHook(void)
{
    if (mScheduler->Start(mNumPorts, mNumIOThreadsPerPort) == -1)
    {
        TC_LOG_ERR(0, "Failed to start RawTcpApplication active object");
        throw EPHXInternal("[RawTcpApplicationProxy::ActivateHook] Failed to start RawTcpApplication active object");
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
    
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::Activate, mApp.get(), appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
    mScheduler->Enqueue(req);
    req->Wait();
    
    for (uint16_t port = 0; port < mNumPorts; port++)
    {
        StatsDBFactory * const dbFactory = StatsDBFactory::instance(port);
        dbFactory->getSqlMsgAdapter()->setPreQueryDelegate(boost::bind(&RawTcpApplicationProxy::SyncResults, this, port));
    }

    TC_LOG_WARN(0, "Activated RawTcpApplication");
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::DeactivateHook(void)
{
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::Deactivate, mApp.get());
    mScheduler->Enqueue(req);
    req->Wait();
    
    if (mScheduler->Stop() == -1)
    {
        TC_LOG_ERR(0, "Failed to stop RawTcpApplication active object");
    }
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::ConfigClients(uint16_t port, const clientConfigList_t& clients, handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::ConfigClients, mApp.get(), port, boost::cref(clients), boost::ref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::UpdateClients(uint16_t port, const handleList_t& handles, const clientConfigList_t& clients)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::UpdateClients, mApp.get(), port, boost::cref(handles), boost::cref(clients));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::DeleteClients(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::DeleteClients, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::RegisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles, loadProfileNotifier_t notifier)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::RegisterClientLoadProfileNotifier, mApp.get(), port, boost::cref(handles), notifier);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::UnregisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::UnregisterClientLoadProfileNotifier, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::ConfigServers(uint16_t port, const serverConfigList_t& servers, handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::ConfigServers, mApp.get(), port, boost::cref(servers), boost::ref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::UpdateServers(uint16_t port, const handleList_t& handles, const serverConfigList_t& servers)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::UpdateServers, mApp.get(), port, boost::cref(handles), boost::cref(servers));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::DeleteServers(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::DeleteServers, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::StartProtocol(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::StartProtocol, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::StopProtocol(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::StopProtocol, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::ClearResults(uint16_t port, const handleList_t& handles, uint64_t absExecTime)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::ClearResults, mApp.get(), port, boost::cref(handles), absExecTime);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::SyncResults(uint16_t port)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::SyncResults, mApp.get(), port);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::SetDynamicLoad(uint16_t port, const loadPairList_t& loadList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::SetDynamicLoad, mApp.get(), port, boost::cref(loadList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::NotifyInterfaceEnablePending(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::NotifyInterfaceEnablePending, mApp.get(), port, boost::cref(ifEnableList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::NotifyInterfaceUpdatePending(uint16_t port, const handleList_t& ifHandleList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::NotifyInterfaceUpdatePending, mApp.get(), port, boost::cref(ifHandleList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplicationProxy::NotifyInterfaceDeletePending(uint16_t port, const handleList_t& ifHandleList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &RawTcpApplication::NotifyInterfaceDeletePending, mApp.get(), port, boost::cref(ifHandleList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////
