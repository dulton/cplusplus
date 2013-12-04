///
/// @file
/// @brief Message set handlers for Ftp_1 messages
///
/// Copyright (c) 2007 by Spirent Communications Inc.
/// All Rights Reserved.
///
/// This software is confidential and proprietary to Spirent Communications Inc.
/// No part of this software may be reproduced, transmitted, disclosed or used
/// in violation of the Software License Agreement without the expressed
/// written consent of Spirent Communications Inc.
///
/// $File: //TestCenter/integration/content/traffic/l4l7/il/ftpd/FtpMsgSetSrv_1.cpp $
/// $Revision: #1 $
/// \n<b>Last submission by:</b>
/// <ul>
/// <li>$Author: songkamongkol $</li>
/// <li>$DateTime: 2011/08/05 16:25:08 $</li>
/// <li>$Change: 572841 $</li>
/// </ul>
///

#include <algorithm>

#include <ildaemon/LocalInterfaceValidator.h>
#include <boost/bind.hpp>
#include <hal/HwInfo.h>
#include <ifmgrclient/IfMgrClient.h>
#include <mps/mps_event_handler.h>
#include <vif/IfSetupInterfaceBlock.h>

#include "ftp_1_port_server.h"
#include "FtpApplicationProxy.h"
#include "FtpdLog.h"
#include "FtpMsgSetSrv_1.h"

USING_FTP_NS;
using namespace Ftp_1_port_server;

///////////////////////////////////////////////////////////////////////////////

class FtpMsgSetSrv_1::MpsPeerHandler : public MPSEventHandler
{
  public:
    MpsPeerHandler(FtpMsgSetSrv_1& msgSet)
        : mMsgSet(msgSet)
    {
    }

    void handleEvent(MPS::MPSEvent event)
    {
        if (event == MPS::PEER_DOWN)
            mMsgSet.MpsPeerDownEvent(this->getMPSID(), this->getModuleName());
    }

  private:
    FtpMsgSetSrv_1& mMsgSet;
};

///////////////////////////////////////////////////////////////////////////////

class FtpMsgSetSrv_1::IfmgrClientHandler : public IFMGR_CLIENT_NS::Client::NotificationHandler
{
  public:
    IfmgrClientHandler(FtpMsgSetSrv_1& msgSet)
        : mMsgSet(msgSet)
    {
    }

    void HandleEnableNotification(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableVec)
    {
        mMsgSet.mApp.NotifyInterfaceEnablePending(port, ifEnableVec);
    }

  private:
    FtpMsgSetSrv_1& mMsgSet;
};

///////////////////////////////////////////////////////////////////////////////

FtpMsgSetSrv_1::FtpMsgSetSrv_1(FtpApplicationProxy& app)
    : mApp(app),
      mIfSetupObserver(fastdelegate::MakeDelegate(this, &FtpMsgSetSrv_1::IfSetupObserver)),
      mMpsHandler(new MpsPeerHandler(*this)),
      mIfmgrClientHandler(new IfmgrClientHandler(*this))
{
    setMessageSetName("Ftp_1");

    const int portCount = Hal::HwInfo::getPortGroupPortCount();
    for (uint16_t port = 0; port < portCount; port++)
        mLoadProfileNotifiers.push_back(std::tr1::shared_ptr<loadProfileNotifierDB_t>(new loadProfileNotifierDB_t()));

    if (registerMessageHandler("ConfigureFtpClients", MakeHandler(&FtpMsgSetSrv_1::ConfigureFtpClients)) ||
        registerMessageHandler("UpdateFtpClients", MakeHandler(&FtpMsgSetSrv_1::UpdateFtpClients)) ||
        registerMessageHandler("DeleteFtpClients", MakeHandler(&FtpMsgSetSrv_1::DeleteFtpClients)) ||
        registerMessageHandler("ConfigureFtpServers", MakeHandler(&FtpMsgSetSrv_1::ConfigureFtpServers)) ||
        registerMessageHandler("UpdateFtpServers", MakeHandler(&FtpMsgSetSrv_1::UpdateFtpServers)) ||
        registerMessageHandler("DeleteFtpServers", MakeHandler(&FtpMsgSetSrv_1::DeleteFtpServers)) ||
        registerMessageHandler("ResetProtocol", MakeHandler(&FtpMsgSetSrv_1::ResetProtocol)) ||
        registerMessageHandler("StartProtocol", MakeHandler(&FtpMsgSetSrv_1::StartProtocol)) ||
        registerMessageHandler("StopProtocol", MakeHandler(&FtpMsgSetSrv_1::StopProtocol)) ||
        registerMessageHandler("ClearResults", MakeHandler(&FtpMsgSetSrv_1::ClearResults)) ||
        registerMessageHandler("SetDynamicLoad", MakeHandler(&FtpMsgSetSrv_1::SetDynamicLoad)) ||
        !mIfSetupMsgSetSrv.RegisterMessages(this) ||
        !StatsBase::RegisterMessages(this))
    {
        TC_LOG_ERR_LOCAL(0, LOG_MPS, "Failed to register at least one message handler");
    }

    mIfSetupMsgSetSrv.AttachObserver(mIfSetupObserver);
    IFMGR_CLIENT_NS::Client::AttachNotificationHandler(*mIfmgrClientHandler);
}

///////////////////////////////////////////////////////////////////////////////

FtpMsgSetSrv_1::~FtpMsgSetSrv_1()
{
    mIfSetupMsgSetSrv.DetachObserver(mIfSetupObserver);
    IFMGR_CLIENT_NS::Client::DetachNotificationHandler(*mIfmgrClientHandler);
}

///////////////////////////////////////////////////////////////////////////////

bool FtpMsgSetSrv_1::RegisterMPSHook(MPS *mps)
{
    if (mps->registerMPSEventHandler(MPS::PEER_DOWN, mMpsHandler.get()) < 0)
    {
        TC_LOG_ERR_LOCAL(0, LOG_MPS, "Failed to register MPS peer down event handler");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void FtpMsgSetSrv_1::UnregisterMPSHook(MPS *mps)
{
    mps->unregisterMPSEventHandler(MPS::PEER_DOWN);
}

///////////////////////////////////////////////////////////////////////////////

void FtpMsgSetSrv_1::IfSetupObserver(const IfSetupMsgSetSrv_1::Notification& notif)
{
    switch (notif.event)
    {
      case IfSetupMsgSetSrv_1::Notification::ATTACH_COMPLETED_EVENT:
      {
          // Validate local interfaces
          for_each(notif.ifBlockList.begin(), notif.ifBlockList.end(), IL_DAEMON_LIB_NS::LocalInterfaceValidator(notif.port));
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

void FtpMsgSetSrv_1::MpsPeerDownEvent(const MPS_Id &mpsId, const std::string& moduleName)
{
    // Remove all async notifiers for this peer
    for_each(mLoadProfileNotifiers.begin(), mLoadProfileNotifiers.end(), boost::bind(&loadProfileNotifierDB_t::remove, _1, mpsId));
    StatsBase::StopNotifications(mpsId);
}

///////////////////////////////////////////////////////////////////////////////

void FtpMsgSetSrv_1::ConfigureFtpClients(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<FtpClientConfig_t> clients;
    parse_req_ConfigureFtpClients(req, &clients);

    // Create clients
    std::vector<uint32_t> handles;
    mApp.ConfigClients(port, clients, handles);

    // Create load profile async notifier
    const L4L7_BASE_NS::LoadProfileNotifierParams notifierParams(std::string(getMessageSetName()),
                                                                 boost::bind(&FtpApplicationProxy::RegisterClientLoadProfileNotifier, &mApp, port, _1, _2),
                                                                 boost::bind(&FtpApplicationProxy::UnregisterClientLoadProfileNotifier, &mApp, port, _1),
                                                                 handles);
    if (mLoadProfileNotifiers[port]->add(handle, port, req, 0, L4L7_BASE_NS::LoadProfileNotifier::DEFAULT_NOTIFIER_INTERVAL_MSEC, notifierParams) == false)
    {
        const std::string err("[FtpMsgSetSrv_1::ConfigureFtpClients] Unable to create async notifier");
        TC_LOG_ERR_LOCAL(port, LOG_MPS, err);
        throw EInval(err);
    }

    // Send response
    Message resp;
    create_resp_ConfigureFtpClients(&resp, handles);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void FtpMsgSetSrv_1::UpdateFtpClients(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    std::vector<FtpClientConfig_t> clients;
    parse_req_UpdateFtpClients(req, &handles, &clients);

    // Update clients
    mApp.UpdateClients(port, handles, clients);

    // Create/update load profile async notifier
    const L4L7_BASE_NS::LoadProfileNotifierParams notifierParams(std::string(getMessageSetName()),
                                                                 boost::bind(&FtpApplicationProxy::RegisterClientLoadProfileNotifier, &mApp, port, _1, _2),
                                                                 boost::bind(&FtpApplicationProxy::UnregisterClientLoadProfileNotifier, &mApp, port, _1),
                                                                 handles);
    if (mLoadProfileNotifiers[port]->add(handle, port, req, 0, L4L7_BASE_NS::LoadProfileNotifier::DEFAULT_NOTIFIER_INTERVAL_MSEC, notifierParams) == false)
    {
        const std::string err("[FtpMsgSetSrv_1::UpdateFtpClients] Unable to create/update async notifier");
        TC_LOG_ERR_LOCAL(port, LOG_MPS, err);
        throw EInval(err);
    }

    // Send response
    Message resp;
    create_resp_UpdateFtpClients(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void FtpMsgSetSrv_1::DeleteFtpClients(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_DeleteFtpClients(req, &handles);

    // Delete clients
    mApp.DeleteClients(port, handles);

    // Send response
    Message resp;
    create_resp_DeleteFtpClients(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void FtpMsgSetSrv_1::ConfigureFtpServers(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<FtpServerConfig_t> servers;
    parse_req_ConfigureFtpServers(req, &servers);

    // Create servers
    std::vector<uint32_t> handles;
    mApp.ConfigServers(port, servers, handles);

    // Send response
    Message resp;
    create_resp_ConfigureFtpServers(&resp, handles);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void FtpMsgSetSrv_1::UpdateFtpServers(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    std::vector<FtpServerConfig_t> servers;
    parse_req_UpdateFtpServers(req, &handles, &servers);

    // Update servers
    mApp.UpdateServers(port, handles, servers);

    // Send response
    Message resp;
    create_resp_UpdateFtpServers(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void FtpMsgSetSrv_1::DeleteFtpServers(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_DeleteFtpServers(req, &handles);

    // Delete servers
    mApp.DeleteServers(port, handles);

    // Send response
    Message resp;
    create_resp_DeleteFtpServers(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void FtpMsgSetSrv_1::ResetProtocol(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_ResetProtocol(req, &handles);

    // For FTP, reset and stop are synonymous
    mApp.StopProtocol(port, handles);

    // Send response
    Message resp;
    create_resp_ResetProtocol(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void FtpMsgSetSrv_1::StartProtocol(MPS_Handle *handle, uint16_t port, Message *req)
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

void FtpMsgSetSrv_1::StopProtocol(MPS_Handle *handle, uint16_t port, Message *req)
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

void FtpMsgSetSrv_1::ClearResults(MPS_Handle *handle, uint16_t port, Message *req)
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

void FtpMsgSetSrv_1::SetDynamicLoad(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<DynamicLoadPair_t> loadPairs;
    Ftp_1_port_server::parse_req_SetDynamicLoad(req, &loadPairs);

    // Set Dynamic Load
    mApp.SetDynamicLoad(port, loadPairs);

    // Send response
    Message resp;
    create_resp_SetDynamicLoad(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////
