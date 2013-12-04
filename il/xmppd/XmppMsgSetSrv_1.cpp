///
/// @file
/// @brief Message set handlers for XmppvJ_1 messages
///
/// Copyright (c) 2007 by Spirent Communications Inc.
/// All Rights Reserved.
///
/// This software is confidential and proprietary to Spirent Communications Inc.
/// No part of this software may be reproduced, transmitted, disclosed or used
/// in violation of the Software License Agreement without the expressed
/// written consent of Spirent Communications Inc.
///
/// $File: //TestCenter/integration/content/traffic/l4l7/il/xmppd/XmppMsgSetSrv_1.cpp $
/// $Revision: #1 $
/// \n<b>Last submission by:</b>
/// <ul>
/// <li>$Author: Chinh.Le $</li>
/// <li>$DateTime: 2011/10/20 20:02:30 $</li>
/// <li>$Change: 584803 $</li>
/// </ul>
///

#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <set>
#include <algorithm>

#include <ildaemon/LocalInterfaceValidator.h>
#include <hal/HwInfo.h>
#include <ifmgrclient/IfMgrClient.h>
#include <mps/mps_event_handler.h>
#include <vif/IfSetupInterfaceBlock.h>
#include <PHXException.h>

#include "xmppvj_1_port_server.h"
#include "XmppApplicationProxy.h"
#include "XmppdLog.h"
#include "XmppMsgSetSrv_1.h"

USING_XMPP_NS;
using namespace XmppvJ_1_port_server;
namespace fs = boost::filesystem;

///////////////////////////////////////////////////////////////////////////////

class XmppMsgSetSrv_1::MpsPeerHandler : public MPSEventHandler
{
  public:
    MpsPeerHandler(XmppMsgSetSrv_1& msgSet)
        : mMsgSet(msgSet)
    {
    }

    void handleEvent(MPS::MPSEvent event)
    {
        if (event == MPS::PEER_DOWN)
            mMsgSet.MpsPeerDownEvent(this->getMPSID(), this->getModuleName());
    }

  private:
    XmppMsgSetSrv_1& mMsgSet;
};

///////////////////////////////////////////////////////////////////////////////

class XmppMsgSetSrv_1::IfmgrClientHandler : public IFMGR_CLIENT_NS::Client::NotificationHandler
{
  public:
    IfmgrClientHandler(XmppMsgSetSrv_1& msgSet)
        : mMsgSet(msgSet)
    {
    }

    void HandleEnableNotification(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableVec)
    {
        mMsgSet.mApp.NotifyInterfaceEnablePending(port, ifEnableVec);
    }

  private:
    XmppMsgSetSrv_1& mMsgSet;
};

///////////////////////////////////////////////////////////////////////////////

XmppMsgSetSrv_1::XmppMsgSetSrv_1(XmppApplicationProxy& app)
    : mApp(app),
      mIfSetupObserver(fastdelegate::MakeDelegate(this, &XmppMsgSetSrv_1::IfSetupObserver)),
      mMpsHandler(new MpsPeerHandler(*this)),
      mIfmgrClientHandler(new IfmgrClientHandler(*this))
{
    setMessageSetName("XmppvJ_1");

    const int portCount = Hal::HwInfo::getPortGroupPortCount();
    for (uint16_t port = 0; port < portCount; port++){
        mLoadProfileNotifiers.push_back(std::tr1::shared_ptr<loadProfileNotifierDB_t>(new loadProfileNotifierDB_t()));
        mXmppClientRegStateNotifiers.push_back(std::tr1::shared_ptr<XmppClientRegStateNotifierDB_t>(new XmppClientRegStateNotifierDB_t()));
    }

    if (registerMessageHandler("ConfigureXmppvJClients", MakeHandler(&XmppMsgSetSrv_1::ConfigureXmppvJClients)) ||
        registerMessageHandler("UpdateXmppvJClients", MakeHandler(&XmppMsgSetSrv_1::UpdateXmppvJClients)) ||
        registerMessageHandler("DeleteXmppvJClients", MakeHandler(&XmppMsgSetSrv_1::DeleteXmppvJClients)) ||
        registerMessageHandler("EnumerateCapabilities", MakeHandler(&XmppMsgSetSrv_1::EnumerateCapabilities)) ||
        registerMessageHandler("RegisterXmppvJClients", MakeHandler(&XmppMsgSetSrv_1::RegisterXmppvJClients)) ||
        registerMessageHandler("UnregisterXmppvJClients", MakeHandler(&XmppMsgSetSrv_1::UnregisterXmppvJClients)) ||
        registerMessageHandler("CancelXmppvJClientsRegistrations", MakeHandler(&XmppMsgSetSrv_1::CancelXmppvJClientsRegistrations)) ||
        registerMessageHandler("ResetProtocol", MakeHandler(&XmppMsgSetSrv_1::ResetProtocol)) ||
        registerMessageHandler("StartProtocol", MakeHandler(&XmppMsgSetSrv_1::StartProtocol)) ||
        registerMessageHandler("StopProtocol", MakeHandler(&XmppMsgSetSrv_1::StopProtocol)) ||
        registerMessageHandler("ClearResults", MakeHandler(&XmppMsgSetSrv_1::ClearResults)) ||
        registerMessageHandler("SetDynamicLoad", MakeHandler(&XmppMsgSetSrv_1::SetDynamicLoad)) ||
        !mIfSetupMsgSetSrv.RegisterMessages(this) ||
        !StatsBase::RegisterMessages(this))
    {
        TC_LOG_ERR_LOCAL(0, LOG_MPS, "Failed to register at least one message handler");
    }

    mIfSetupMsgSetSrv.AttachObserver(mIfSetupObserver);
    IFMGR_CLIENT_NS::Client::AttachNotificationHandler(*mIfmgrClientHandler);
}

///////////////////////////////////////////////////////////////////////////////

XmppMsgSetSrv_1::~XmppMsgSetSrv_1()
{
    mIfSetupMsgSetSrv.DetachObserver(mIfSetupObserver);
    IFMGR_CLIENT_NS::Client::DetachNotificationHandler(*mIfmgrClientHandler);
}

///////////////////////////////////////////////////////////////////////////////

bool XmppMsgSetSrv_1::RegisterMPSHook(MPS *mps)
{
    if (mps->registerMPSEventHandler(MPS::PEER_DOWN, mMpsHandler.get()) < 0)
    {
        TC_LOG_ERR_LOCAL(0, LOG_MPS, "Failed to register MPS peer down event handler");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void XmppMsgSetSrv_1::UnregisterMPSHook(MPS *mps)
{
    mps->unregisterMPSEventHandler(MPS::PEER_DOWN);
}

///////////////////////////////////////////////////////////////////////////////

void XmppMsgSetSrv_1::IfSetupObserver(const IfSetupMsgSetSrv_1::Notification& notif)
{
    switch (notif.event)
    {
      case IfSetupMsgSetSrv_1::Notification::ATTACH_COMPLETED_EVENT:
      {
          // Validate local interfaces
          std::for_each(notif.ifBlockList.begin(), notif.ifBlockList.end(), IL_DAEMON_LIB_NS::LocalInterfaceValidator(notif.port));
          break;
      }

      case IfSetupMsgSetSrv_1::Notification::DETACH_PENDING_EVENT:
      {
          // Transform block list to handle list
          std::vector<uint32_t> ifHandleVec;
          ifHandleVec.reserve(notif.ifBlockList.size());
          transform(notif.ifBlockList.begin(), notif.ifBlockList.end(), std::back_inserter(ifHandleVec), boost::mem_fn(&IFSETUP_NS::InterfaceBlock::HandleValue));

          // Virtual interfaces are being detached -- inform all associated clients and servers
          mApp.NotifyInterfaceDeletePending(notif.port, ifHandleVec);
          break;
      }

      case IfSetupMsgSetSrv_1::Notification::UPDATE_PENDING_EVENT:
      {
          // Transform block list to handle list
          std::vector<uint32_t> ifHandleVec;
          ifHandleVec.reserve(notif.ifBlockList.size());
          transform(notif.ifBlockList.begin(), notif.ifBlockList.end(), std::back_inserter(ifHandleVec), boost::mem_fn(&IFSETUP_NS::InterfaceBlock::HandleValue));

          // Virtual interfaces are about to be updated -- inform all associated clients and servers
          mApp.NotifyInterfaceUpdatePending(notif.port, ifHandleVec);
          break;
      }

      case IfSetupMsgSetSrv_1::Notification::DELETE_PENDING_EVENT:
      {
          // Transform block list to handle list
          std::vector<uint32_t> ifHandleVec;
          ifHandleVec.reserve(notif.ifBlockList.size());
          transform(notif.ifBlockList.begin(), notif.ifBlockList.end(), std::back_inserter(ifHandleVec), boost::mem_fn(&IFSETUP_NS::InterfaceBlock::HandleValue));

          // Virtual interfaces are being deleted -- inform all associated clients and servers
          mApp.NotifyInterfaceDeletePending(notif.port, ifHandleVec);
          break;
      }

      case IfSetupMsgSetSrv_1::Notification::UPDATE_COMPLETED_EVENT:
      default:
          break;
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppMsgSetSrv_1::MpsPeerDownEvent(const MPS_Id &mpsId, const std::string& moduleName)
{
    // Remove all async notifiers for this peer
    for_each(mLoadProfileNotifiers.begin(), mLoadProfileNotifiers.end(), boost::bind(&loadProfileNotifierDB_t::remove, _1, mpsId));
    for_each(mXmppClientRegStateNotifiers.begin(), mXmppClientRegStateNotifiers.end(), boost::bind(&XmppClientRegStateNotifierDB_t::remove, _1, mpsId));
    StatsBase::StopNotifications(mpsId);
}

///////////////////////////////////////////////////////////////////////////////

void XmppMsgSetSrv_1::ConfigureXmppvJClients(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<XmppvJClient_t> clients;
    parse_req_ConfigureXmppvJClients(req, &clients);

    // Create clients
    std::vector<uint32_t> handles;
    mApp.ConfigClients(port, clients, handles);

    // Create load profile async notifier
    const L4L7_BASE_NS::LoadProfileNotifierParams notifierParams(std::string(getMessageSetName()),
                                                                 boost::bind(&XmppApplicationProxy::RegisterClientLoadProfileNotifier, &mApp, port, _1, _2),
                                                                 boost::bind(&XmppApplicationProxy::UnregisterClientLoadProfileNotifier, &mApp, port, _1),
                                                                 handles);
    if (mLoadProfileNotifiers[port]->add(handle, port, req, 0, L4L7_BASE_NS::LoadProfileNotifier::DEFAULT_NOTIFIER_INTERVAL_MSEC, notifierParams) == false)
    {
        const std::string err("[XmppMsgSetSrv_1::ConfigureXmppvJClients] Unable to create async notifier");
        TC_LOG_ERR_LOCAL(port, LOG_MPS, err);
        throw EInval(err);
    }

    // Send response
    Message resp;
    create_resp_ConfigureXmppvJClients(&resp, handles, false);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void XmppMsgSetSrv_1::UpdateXmppvJClients(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    std::vector<XmppvJClient_t> clients;
    parse_req_UpdateXmppvJClients(req, &handles, &clients);

    // Update clients
    mApp.UpdateClients(port, handles, clients);

    // Create/update load profile async notifier
    const L4L7_BASE_NS::LoadProfileNotifierParams notifierParams(std::string(getMessageSetName()),
                                                                 boost::bind(&XmppApplicationProxy::RegisterClientLoadProfileNotifier, &mApp, port, _1, _2),
                                                                 boost::bind(&XmppApplicationProxy::UnregisterClientLoadProfileNotifier, &mApp, port, _1),
                                                                 handles);
    if (mLoadProfileNotifiers[port]->add(handle, port, req, 0, L4L7_BASE_NS::LoadProfileNotifier::DEFAULT_NOTIFIER_INTERVAL_MSEC, notifierParams) == false)
    {
        const std::string err("[XmppMsgSetSrv_1::UpdateXmppvJClients] Unable to create/update async notifier");
        TC_LOG_ERR_LOCAL(port, LOG_MPS, err);
        throw EInval(err);
    }

    // Send response
    Message resp;
    create_resp_UpdateXmppvJClients(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void XmppMsgSetSrv_1::DeleteXmppvJClients(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_DeleteXmppvJClients(req, &handles);

    // Delete clients
    mApp.DeleteClients(port, handles);

    // Send response
    Message resp;
    create_resp_DeleteXmppvJClients(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void XmppMsgSetSrv_1::EnumerateCapabilities(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::string globSpec;
    parse_req_EnumerateCapabilities(req, &globSpec);

    // Get the list of capabilities files.
    // NOTE: for now, ignore globSpec and just return everything in the directory
    fs::path full_path(fs::system_complete(fs::path(CapabilitiesFilePath)));
    if (!fs::exists(full_path))
    {
        std::ostringstream err;
        err << "Invalid path specified: " << full_path.string();
        throw EInval(err.str());
    }
    std::set<std::string> sortedCapabilityFiles;
    fs::directory_iterator end_itr;
    for (fs::directory_iterator itr(full_path); itr != end_itr; ++itr)
        sortedCapabilityFiles.insert(itr->leaf());
    std::vector<std::string> capabilityFiles(sortedCapabilityFiles.begin(), sortedCapabilityFiles.end());
    
    // Send response
    Message resp;
    create_resp_EnumerateCapabilities(&resp, capabilityFiles, false);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void XmppMsgSetSrv_1::ResetProtocol(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_ResetProtocol(req, &handles);

    // For XMPP, reset and stop are synonymous
    mApp.StopProtocol(port, handles);

    // Send response
    Message resp;
    create_resp_ResetProtocol(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void XmppMsgSetSrv_1::StartProtocol(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_StartProtocol(req, &handles);

    // Start protocols
    mApp.StartProtocol(port, handles);

    // Send response
    Message resp;
    create_resp_StartProtocol(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void XmppMsgSetSrv_1::StopProtocol(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_StopProtocol(req, &handles);

    // Stop protocols
    mApp.StopProtocol(port, handles);

    // Send response
    Message resp;
    create_resp_StopProtocol(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void XmppMsgSetSrv_1::ClearResults(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    uint64_t absExecTime;
    parse_req_ClearResults(req, &handles, &absExecTime);

    // Clear results
    mApp.ClearResults(port, handles, absExecTime);

    // Send response
    Message resp;
    create_resp_ClearResults(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void XmppMsgSetSrv_1::SetDynamicLoad(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<DynamicLoadPair_t> loadPairs;
    XmppvJ_1_port_server::parse_req_SetDynamicLoad(req, &loadPairs);

    // Set Dynamic Load
    mApp.SetDynamicLoad(port, loadPairs);

    // Send response
    Message resp;
    create_resp_SetDynamicLoad(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////
//Registration Commands
///////////////////////////////////////////////////////////////////////////////
void XmppMsgSetSrv_1::RegisterXmppvJClients(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    bool asyncNotification;
    uint32_t asyncNotifierId;
    uint32_t asyncNotifierIntervalMsec;
    parse_req_RegisterXmppvJClients(req, &handles, &asyncNotification, &asyncNotifierId, &asyncNotifierIntervalMsec);

    TC_LOG_ERR(0,"[XmppMsgSetSrv_1::RegisterXmppvJClients] asyncNotification:"<<asyncNotification<<" asyncNotifierId:"<<asyncNotifierId<<" asyncNotifierIntervalMsec:"<<asyncNotifierIntervalMsec);

    // Create registration state async notifier as necessary
    if (asyncNotification)
    {
        if (mXmppClientRegStateNotifiers[port]->add(handle, port, req, asyncNotifierId, asyncNotifierIntervalMsec, XmppClientRegStateNotifierParams(mApp, handles)) == false)
        {
            const std::string err("[XmppMsgSetSrv_1::RegisterXmppvJClients] Unable to create async notifier");
            TC_LOG_ERR_LOCAL(port, LOG_MPS, err);
            throw EInval(err);
        }
    }

    // Register Xmpp Clients
    mApp.RegisterXmppClients(port, handles);

    // Send response
    Message resp;
    create_resp_RegisterXmppvJClients(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

void XmppMsgSetSrv_1::UnregisterXmppvJClients(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_UnregisterXmppvJClients(req, &handles);

    // Start Xmpp Client unregistrations
    mApp.UnregisterXmppClients(port, handles);

    // Send response
    Message resp;
    create_resp_UnregisterXmppvJClients(&resp);
    handle->sendResponse(req, &resp);
                                    
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

void XmppMsgSetSrv_1::CancelXmppvJClientsRegistrations(MPS_Handle *handle, uint16_t port, Message *req)
{   
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    TC_LOG_ERR(0,"cncl [XmppMsgSetSrv_1::CancelXmppvJClientsRegistrations] called");

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_CancelXmppvJClientsRegistrations(req, &handles);

    // Cancel Xmpp Client registrations
    mApp.CancelXmppClientsRegistrations(port, handles);
                     
    // Send response
    Message resp;
    create_resp_CancelXmppvJClientsRegistrations(&resp);
    handle->sendResponse(req, &resp);
                                        
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}
