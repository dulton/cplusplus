/// @file
/// @brief FTP Application Proxy implementation
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

#include "FtpApplication.h"
#include "FtpApplicationProxy.h"
#include "FtpdLog.h"
#include "FtpMsgSetSrv_1.h"

USING_FTP_NS;

///////////////////////////////////////////////////////////////////////////////

IL_APPLICATION_REGISTRATION(FtpApplicationProxy, "FTP");

///////////////////////////////////////////////////////////////////////////////

bool FtpApplicationProxy::InitializeHook(uint16_t ports, const IL_DAEMON_LIB_NS::CommandLineOpts& opts)
{
    mNumPorts = ports;
    mApp.reset(new FtpApplication(ports));
    mMsgSet.reset(new FtpMsgSetSrv_1(*this));
    mScheduler.reset(new IL_DAEMON_LIB_NS::ActiveScheduler);

    mNumPorts = ports;
    mNumIOThreadsPerPort = opts.NumIOThreadsPerPort();
    if (mNumIOThreadsPerPort < 1)
    {
        TC_LOG_WARN(0, "FtpApplication must use at least one I/O thread per port, using one for now");
        mNumIOThreadsPerPort = 1;
    }
    
    AddMessageSet(*mMsgSet);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool FtpApplicationProxy::RegisterMPSHook(MPS *mps)
{
    return mMsgSet->RegisterMPSHook(mps);
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::UnregisterMPSHook(MPS *mps)
{
    mMsgSet->UnregisterMPSHook(mps);
}

///////////////////////////////////////////////////////////////////////////////

bool FtpApplicationProxy::ActivateHook(void)
{
    if (mScheduler->Start(mNumPorts, mNumIOThreadsPerPort) == -1)
    {
        TC_LOG_ERR(0, "Failed to start FtpApplication active object");
        throw EPHXInternal("[FtpApplicationProxy::ActivateHook] Failed to start FtpApplication active object");
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
    
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::Activate, mApp.get(), appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
    mScheduler->Enqueue(req);
    req->Wait();
    
    for (uint16_t port = 0; port < mNumPorts; port++)
    {
        StatsDBFactory * const dbFactory = StatsDBFactory::instance(port);
        dbFactory->getSqlMsgAdapter()->setPreQueryDelegate(boost::bind(&FtpApplicationProxy::SyncResults, this, port));
    }
    
    TC_LOG_WARN(0, "Activated FtpApplication");
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::DeactivateHook(void)
{
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::Deactivate, mApp.get());
    mScheduler->Enqueue(req);
    req->Wait();
    
    if (mScheduler->Stop() == -1)
    {
        TC_LOG_ERR(0, "Failed to stop FtpApplication active object");
    }
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::ConfigClients(uint16_t port, const clientConfigList_t& clients, handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::ConfigClients, mApp.get(), port, boost::cref(clients), boost::ref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::UpdateClients(uint16_t port, const handleList_t& handles, const clientConfigList_t& clients)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::UpdateClients, mApp.get(), port, boost::cref(handles), boost::cref(clients));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::DeleteClients(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::DeleteClients, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::RegisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles, loadProfileNotifier_t notifier)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::RegisterClientLoadProfileNotifier, mApp.get(), port, boost::cref(handles), notifier);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::UnregisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::UnregisterClientLoadProfileNotifier, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::ConfigServers(uint16_t port, const serverConfigList_t& servers, handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::ConfigServers, mApp.get(), port, boost::cref(servers), boost::ref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::UpdateServers(uint16_t port, const handleList_t& handles, const serverConfigList_t& servers)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::UpdateServers, mApp.get(), port, boost::cref(handles), boost::cref(servers));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::DeleteServers(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::DeleteServers, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::StartProtocol(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::StartProtocol, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::StopProtocol(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::StopProtocol, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::ClearResults(uint16_t port, const handleList_t& handles, uint64_t absExecTime)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::ClearResults, mApp.get(), port, boost::cref(handles), absExecTime);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::SyncResults(uint16_t port)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::SyncResults, mApp.get(), port);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::SetDynamicLoad(uint16_t port, const loadPairList_t& loadList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::SetDynamicLoad, mApp.get(), port, boost::cref(loadList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::NotifyInterfaceEnablePending(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::NotifyInterfaceEnablePending, mApp.get(), port, boost::cref(ifEnableList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::NotifyInterfaceUpdatePending(uint16_t port, const handleList_t& ifHandleList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::NotifyInterfaceUpdatePending, mApp.get(), port, boost::cref(ifHandleList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplicationProxy::NotifyInterfaceDeletePending(uint16_t port, const handleList_t& ifHandleList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::NotifyInterfaceDeletePending, mApp.get(), port, boost::cref(ifHandleList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////
