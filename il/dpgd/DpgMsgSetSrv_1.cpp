///
/// @file
/// @brief Message set handlers for Dpg_1 messages
///
/// Copyright (c) 2009 by Spirent Communications Inc.
/// All Rights Reserved.
///
/// This software is confidential and proprietary to Spirent Communications Inc.
/// No part of this software may be reproduced, transmitted, disclosed or used
/// in violation of the Software License Agreement without the expressed
/// written consent of Spirent Communications Inc.
///
/// $File: //TestCenter/integration/content/traffic/l4l7/il/dpgd/DpgMsgSetSrv_1.cpp $
/// $Revision: #2 $
/// \n<b>Last submission by:</b>
/// <ul>
/// <li>$Author: byoshino $</li>
/// <li>$DateTime: 2011/09/02 19:36:56 $</li>
/// <li>$Change: 577743 $</li>
/// </ul>
///

#include <algorithm>

#include <ildaemon/LocalInterfaceValidator.h>
#include <boost/bind.hpp>
#include <hal/HwInfo.h>
#include <ifmgrclient/IfMgrClient.h>
#include <mps/mps_event_handler.h>
#include <vif/IfSetupInterfaceBlock.h>

#include "dpgIf_1_port_server.h"
#include "DpgApplicationProxy.h"
#include "DpgdLog.h"
#include "DpgMsgSetSrv_1.h"

USING_DPG_NS;
using namespace DpgIf_1_port_server;

///////////////////////////////////////////////////////////////////////////////

class DpgMsgSetSrv_1::MpsPeerHandler : public MPSEventHandler
{
  public:
    MpsPeerHandler(DpgMsgSetSrv_1& msgSet)
        : mMsgSet(msgSet)
    {
    }

    void handleEvent(MPS::MPSEvent event)
    {
        if (event == MPS::PEER_DOWN)
            mMsgSet.MpsPeerDownEvent(this->getMPSID(), this->getModuleName());
    }

  private:
    DpgMsgSetSrv_1& mMsgSet;
};

///////////////////////////////////////////////////////////////////////////////

class DpgMsgSetSrv_1::IfmgrClientHandler : public IFMGR_CLIENT_NS::Client::NotificationHandler
{
  public:
    IfmgrClientHandler(DpgMsgSetSrv_1& msgSet)
        : mMsgSet(msgSet)
    {
    }

    void HandleEnableNotification(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableVec)
    {
        mMsgSet.mApp.NotifyInterfaceEnablePending(port, ifEnableVec);
    }

  private:
    DpgMsgSetSrv_1& mMsgSet;
};

///////////////////////////////////////////////////////////////////////////////

DpgMsgSetSrv_1::DpgMsgSetSrv_1(DpgApplicationProxy& app, std::string& messageSetName)
    : mApp(app),
      mIfSetupObserver(fastdelegate::MakeDelegate(this, &DpgMsgSetSrv_1::IfSetupObserver)),
      mMpsHandler(new MpsPeerHandler(*this)),
      mIfmgrClientHandler(new IfmgrClientHandler(*this))
{
  if (messageSetName != "")
  {
    setMessageSetName(messageSetName);
    TC_LOG_ERR_LOCAL(0, LOG_MPS, "Register message set " << messageSetName);
  }
  else
  {
    setMessageSetName("Dpg_1");
    TC_LOG_ERR_LOCAL(0, LOG_MPS, "Register message set Dpg_1");

  }

    const int portCount = Hal::HwInfo::getPortGroupPortCount();
    for (uint16_t port = 0; port < portCount; port++)
        mLoadProfileNotifiers.push_back(std::tr1::shared_ptr<loadProfileNotifierDB_t>(new loadProfileNotifierDB_t()));

    if (registerMessageHandler("UpdateDpgFlows", MakeHandler(&DpgMsgSetSrv_1::UpdateDpgFlows)) ||
        registerMessageHandler("DeleteDpgFlows", MakeHandler(&DpgMsgSetSrv_1::DeleteDpgFlows)) ||
        registerMessageHandler("UpdateDpgPlaylists", MakeHandler(&DpgMsgSetSrv_1::UpdateDpgPlaylists)) ||
        registerMessageHandler("DeleteDpgPlaylists", MakeHandler(&DpgMsgSetSrv_1::DeleteDpgPlaylists)) ||
        registerMessageHandler("ConfigureDpgClients", MakeHandler(&DpgMsgSetSrv_1::ConfigureDpgClients)) ||
        registerMessageHandler("UpdateDpgClients", MakeHandler(&DpgMsgSetSrv_1::UpdateDpgClients)) ||
        registerMessageHandler("DeleteDpgClients", MakeHandler(&DpgMsgSetSrv_1::DeleteDpgClients)) ||
        registerMessageHandler("ConfigureDpgServers", MakeHandler(&DpgMsgSetSrv_1::ConfigureDpgServers)) ||
        registerMessageHandler("UpdateDpgServers", MakeHandler(&DpgMsgSetSrv_1::UpdateDpgServers)) ||
        registerMessageHandler("DeleteDpgServers", MakeHandler(&DpgMsgSetSrv_1::DeleteDpgServers)) ||
        registerMessageHandler("ResetProtocol", MakeHandler(&DpgMsgSetSrv_1::ResetProtocol)) ||
        registerMessageHandler("StartProtocol", MakeHandler(&DpgMsgSetSrv_1::StartProtocol)) ||
        registerMessageHandler("StopProtocol", MakeHandler(&DpgMsgSetSrv_1::StopProtocol)) ||
        registerMessageHandler("ClearResults", MakeHandler(&DpgMsgSetSrv_1::ClearResults)) ||
        registerMessageHandler("SetDynamicLoad", MakeHandler(&DpgMsgSetSrv_1::SetDynamicLoad)) ||
        !mIfSetupMsgSetSrv.RegisterMessages(this) ||
        !StatsBase::RegisterMessages(this))
    {
        TC_LOG_ERR_LOCAL(0, LOG_MPS, "Failed to register at least one message handler");
    }

    mIfSetupMsgSetSrv.AttachObserver(mIfSetupObserver);
    IFMGR_CLIENT_NS::Client::AttachNotificationHandler(*mIfmgrClientHandler);
}

///////////////////////////////////////////////////////////////////////////////

DpgMsgSetSrv_1::~DpgMsgSetSrv_1()
{
    mIfSetupMsgSetSrv.DetachObserver(mIfSetupObserver);
    IFMGR_CLIENT_NS::Client::DetachNotificationHandler(*mIfmgrClientHandler);
}

///////////////////////////////////////////////////////////////////////////////

bool DpgMsgSetSrv_1::RegisterMPSHook(MPS *mps)
{
    if (mps->registerMPSEventHandler(MPS::PEER_DOWN, mMpsHandler.get()) < 0)
    {
        TC_LOG_ERR_LOCAL(0, LOG_MPS, "Failed to register MPS peer down event handler");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void DpgMsgSetSrv_1::UnregisterMPSHook(MPS *mps)
{
    mps->unregisterMPSEventHandler(MPS::PEER_DOWN);
}

///////////////////////////////////////////////////////////////////////////////

void DpgMsgSetSrv_1::IfSetupObserver(const IfSetupMsgSetSrv_1::Notification& notif)
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

void DpgMsgSetSrv_1::MpsPeerDownEvent(const MPS_Id &mpsId, const std::string& moduleName)
{
    // Remove all async notifiers for this peer
    for_each(mLoadProfileNotifiers.begin(), mLoadProfileNotifiers.end(), boost::bind(&loadProfileNotifierDB_t::remove, _1, mpsId));
    StatsBase::StopNotifications(mpsId);
}

///////////////////////////////////////////////////////////////////////////////

void DpgMsgSetSrv_1::UpdateDpgFlows(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    std::vector<DpgFlowConfig_t> flows;
    parse_req_UpdateDpgFlows(req, &handles, &flows);

    // Update flows
    mApp.UpdateFlows(port, handles, flows);

    // Send response
    Message resp;
    create_resp_UpdateDpgFlows(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void DpgMsgSetSrv_1::DeleteDpgFlows(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_DeleteDpgFlows(req, &handles);

    // Delete flows
    mApp.DeleteFlows(port, handles);

    // Send response
    Message resp;
    create_resp_DeleteDpgFlows(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void DpgMsgSetSrv_1::UpdateDpgPlaylists(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    std::vector<DpgPlaylistConfig_t> playlists;
    parse_req_UpdateDpgPlaylists(req, &handles, &playlists);

    // Update playlists
    mApp.UpdatePlaylists(port, handles, playlists);

    // Send response
    Message resp;
    create_resp_UpdateDpgPlaylists(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void DpgMsgSetSrv_1::DeleteDpgPlaylists(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_DeleteDpgPlaylists(req, &handles);

    // Delete playlists
    mApp.DeletePlaylists(port, handles);

    // Send response
    Message resp;
    create_resp_DeleteDpgPlaylists(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void DpgMsgSetSrv_1::ConfigureDpgClients(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<DpgClientConfig_t> clients;
    parse_req_ConfigureDpgClients(req, &clients);

    // Create clients
    std::vector<uint32_t> handles;
    mApp.ConfigClients(port, clients, handles);

    // Create load profile async notifier
    const L4L7_BASE_NS::LoadProfileNotifierParams notifierParams(std::string(getMessageSetName()),
                                                                 boost::bind(&DpgApplicationProxy::RegisterClientLoadProfileNotifier, &mApp, port, _1, _2),
                                                                 boost::bind(&DpgApplicationProxy::UnregisterClientLoadProfileNotifier, &mApp, port, _1),
                                                                 handles);
    if (mLoadProfileNotifiers[port]->add(handle, port, req, 0, L4L7_BASE_NS::LoadProfileNotifier::DEFAULT_NOTIFIER_INTERVAL_MSEC, notifierParams) == false)
    {
        const std::string err("[DpgMsgSetSrv_1::ConfigureDpgClients] Unable to create async notifier");
        TC_LOG_ERR_LOCAL(port, LOG_MPS, err);
        throw EInval(err);
    }

    // Send response
    Message resp;
    create_resp_ConfigureDpgClients(&resp, handles);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void DpgMsgSetSrv_1::UpdateDpgClients(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    std::vector<DpgClientConfig_t> clients;
    parse_req_UpdateDpgClients(req, &handles, &clients);

    // Update clients
    mApp.UpdateClients(port, handles, clients);

    // Create/update load profile async notifier
    const L4L7_BASE_NS::LoadProfileNotifierParams notifierParams(std::string(getMessageSetName()),
                                                                 boost::bind(&DpgApplicationProxy::RegisterClientLoadProfileNotifier, &mApp, port, _1, _2),
                                                                 boost::bind(&DpgApplicationProxy::UnregisterClientLoadProfileNotifier, &mApp, port, _1),
                                                                 handles);
    if (mLoadProfileNotifiers[port]->add(handle, port, req, 0, L4L7_BASE_NS::LoadProfileNotifier::DEFAULT_NOTIFIER_INTERVAL_MSEC, notifierParams) == false)
    {
        const std::string err("[DpgMsgSetSrv_1::UpdateDpgClients] Unable to create/update async notifier");
        TC_LOG_ERR_LOCAL(port, LOG_MPS, err);
        throw EInval(err);
    }

    // Send response
    Message resp;
    create_resp_UpdateDpgClients(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void DpgMsgSetSrv_1::DeleteDpgClients(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_DeleteDpgClients(req, &handles);

    // Delete clients
    mApp.DeleteClients(port, handles);

    // Send response
    Message resp;
    create_resp_DeleteDpgClients(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void DpgMsgSetSrv_1::ConfigureDpgServers(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<DpgServerConfig_t> servers;
    parse_req_ConfigureDpgServers(req, &servers);

    // Create servers
    std::vector<uint32_t> handles;
    mApp.ConfigServers(port, servers, handles);

    // Send response
    Message resp;
    create_resp_ConfigureDpgServers(&resp, handles);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void DpgMsgSetSrv_1::UpdateDpgServers(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    std::vector<DpgServerConfig_t> servers;
    parse_req_UpdateDpgServers(req, &handles, &servers);

    // Update servers
    mApp.UpdateServers(port, handles, servers);

    // Send response
    Message resp;
    create_resp_UpdateDpgServers(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void DpgMsgSetSrv_1::DeleteDpgServers(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_DeleteDpgServers(req, &handles);

    // Delete servers
    mApp.DeleteServers(port, handles);

    // Send response
    Message resp;
    create_resp_DeleteDpgServers(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void DpgMsgSetSrv_1::ResetProtocol(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_ResetProtocol(req, &handles);

    // For DPG, reset and stop are synonymous
    mApp.StopProtocol(port, handles);

    // Send response
    Message resp;
    create_resp_ResetProtocol(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void DpgMsgSetSrv_1::StartProtocol(MPS_Handle *handle, uint16_t port, Message *req)
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

void DpgMsgSetSrv_1::StopProtocol(MPS_Handle *handle, uint16_t port, Message *req)
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

void DpgMsgSetSrv_1::ClearResults(MPS_Handle *handle, uint16_t port, Message *req)
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

void DpgMsgSetSrv_1::SetDynamicLoad(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);
    
    // Parse incoming parameters
    std::vector<DynamicLoadPair_t> loadPairs;
    DpgIf_1_port_server::parse_req_SetDynamicLoad(req, &loadPairs);
    
    // Set Dynamic Load
    mApp.SetDynamicLoad(port, loadPairs);
    
    // Send response
    Message resp;
    create_resp_SetDynamicLoad(&resp);
    handle->sendResponse(req, &resp); 

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

