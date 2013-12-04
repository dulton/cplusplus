/// @file
/// @brief XMPP Application Proxy implementation
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

#include "XmppApplication.h"
#include "XmppApplicationProxy.h"
#include "XmppdLog.h"
#include "XmppMsgSetSrv_1.h"


USING_XMPP_NS;

///////////////////////////////////////////////////////////////////////////////

IL_APPLICATION_REGISTRATION(XmppApplicationProxy, "XMPP");

///////////////////////////////////////////////////////////////////////////////

bool XmppApplicationProxy::InitializeHook(uint16_t ports, const IL_DAEMON_LIB_NS::CommandLineOpts& opts)
{
    mNumPorts = ports;
    mApp.reset(new XmppApplication(ports));
    mMsgSet.reset(new XmppMsgSetSrv_1(*this));
    mScheduler.reset(new IL_DAEMON_LIB_NS::ActiveScheduler);
    mNumPorts = ports;
    mNumIOThreadsPerPort = opts.NumIOThreadsPerPort();
    if (mNumIOThreadsPerPort < 1)
    {
        TC_LOG_WARN(0, "XmppApplication must use at least one I/O thread per port, using one for now");
        mNumIOThreadsPerPort = 1;
    }

    AddMessageSet(*mMsgSet);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool XmppApplicationProxy::RegisterMPSHook(MPS *mps)
{
    return mMsgSet->RegisterMPSHook(mps);
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::UnregisterMPSHook(MPS *mps)
{
    mMsgSet->UnregisterMPSHook(mps);
}

///////////////////////////////////////////////////////////////////////////////

bool XmppApplicationProxy::ActivateHook(void)
{
    if (mScheduler->Start(mNumPorts, mNumIOThreadsPerPort) == -1)
    {
        TC_LOG_ERR(0, "Failed to start XmppApplication active object");
        throw EPHXInternal("[XmppApplicationProxy::ActivateHook] Failed to start XmppApplication active object");
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
    
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::Activate, mApp.get(), appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
    mScheduler->Enqueue(req);
    req->Wait();
    
    for (uint16_t port = 0; port < mNumPorts; port++)
    {
        StatsDBFactory * const dbFactory = StatsDBFactory::instance(port);
        dbFactory->getSqlMsgAdapter()->setPreQueryDelegate(boost::bind(&XmppApplicationProxy::SyncResults, this, port));
    }

    TC_LOG_WARN(0, "Activated XmppApplication");
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::DeactivateHook(void)
{
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::Deactivate, mApp.get());
    mScheduler->Enqueue(req);
    req->Wait();
    
    if (mScheduler->Stop() == -1)
    {
        TC_LOG_ERR(0, "Failed to stop XmppApplication active object");
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::ConfigClients(uint16_t port, const clientConfigList_t& clients, handleList_t& handles)
{
    TC_LOG_ERR(0,"[XMPPApplicationProxy::Config] function called");
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::ConfigClients, mApp.get(), port, boost::cref(clients), boost::ref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::UpdateClients(uint16_t port, const handleList_t& handles, const clientConfigList_t& clients)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::UpdateClients, mApp.get(), port, boost::cref(handles), boost::cref(clients));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::DeleteClients(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::DeleteClients, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::RegisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles, loadProfileNotifier_t notifier)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::RegisterClientLoadProfileNotifier, mApp.get(), port, boost::cref(handles), notifier);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::UnregisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::UnregisterClientLoadProfileNotifier, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::RegisterXmppClients(uint16_t port, const handleList_t& handles)
{
    TC_LOG_ERR(0,"[XMPPApplicationProxy::RegisterXmppClients] function called");
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::RegisterXmppClients, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::UnregisterXmppClients(uint16_t port, const handleList_t& handles)
{
    TC_LOG_ERR(0,"[XMPPApplicationProxy::UnRegisterXmppClients] function called");
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::UnregisterXmppClients, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::CancelXmppClientsRegistrations(uint16_t port, const handleList_t& handles)
{
    TC_LOG_ERR(0,"cncl [XMPPApplicationProxy::CancelXmppClientsRegistrations] function called");
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::CancelXmppClientsRegistrations, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::RegisterXmppClientRegStateNotifier(uint16_t port, const handleList_t& handles, regStateNotifier_t notifier)
{    TC_LOG_ERR(0,"[XMPPApplicationProxy::RegisterXmppClientRegStateNotifier] function called");
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::RegisterXmppClientRegStateNotifier, mApp.get(), port, boost::cref(handles), notifier);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::UnregisterXmppClientRegStateNotifier(uint16_t port, const handleList_t& handles)
{
    TC_LOG_ERR(0,"[XMPPApplicationProxy::UnRegisterXmppClientRegStateNotifier] function called");
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::UnregisterXmppClientRegStateNotifier, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::StartProtocol(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::StartProtocol, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::StopProtocol(uint16_t port, const handleList_t& handles)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::StopProtocol, mApp.get(), port, boost::cref(handles));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::ClearResults(uint16_t port, const handleList_t& handles, uint64_t absExecTime)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::ClearResults, mApp.get(), port, boost::cref(handles), absExecTime);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::SyncResults(uint16_t port)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::SyncResults, mApp.get(), port);
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::SetDynamicLoad(uint16_t port, const loadPairList_t& loadList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::SetDynamicLoad, mApp.get(), port, boost::cref(loadList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::NotifyInterfaceEnablePending(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::NotifyInterfaceEnablePending, mApp.get(), port, boost::cref(ifEnableList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::NotifyInterfaceUpdatePending(uint16_t port, const handleList_t& ifHandleList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::NotifyInterfaceUpdatePending, mApp.get(), port, boost::cref(ifHandleList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplicationProxy::NotifyInterfaceDeletePending(uint16_t port, const handleList_t& ifHandleList)
{
    Activate();
    IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::NotifyInterfaceDeletePending, mApp.get(), port, boost::cref(ifHandleList));
    mScheduler->Enqueue(req);
    req->Wait();
}

///////////////////////////////////////////////////////////////////////////////
