///
/// @file
/// @brief Message set handlers for RawTcp_1 messages
///
/// Copyright (c) 2009 by Spirent Communications Inc.
/// All Rights Reserved.
///
/// This software is confidential and proprietary to Spirent Communications Inc.
/// No part of this software may be reproduced, transmitted, disclosed or used
/// in violation of the Software License Agreement without the expressed
/// written consent of Spirent Communications Inc.
///
/// $File: //TestCenter/integration/content/traffic/l4l7/il/rawtcpd/RawTcpMsgSetSrv_1.cpp $
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

#include "rawTcp_1_port_server.h"
#include "RawTcpApplicationProxy.h"
#include "RawtcpdLog.h"
#include "RawTcpMsgSetSrv_1.h"

USING_RAWTCP_NS;
using namespace RawTcp_1_port_server;

///////////////////////////////////////////////////////////////////////////////

class RawTcpMsgSetSrv_1::MpsPeerHandler : public MPSEventHandler
{
  public:
    MpsPeerHandler(RawTcpMsgSetSrv_1& msgSet)
        : mMsgSet(msgSet)
    {
    }

    void handleEvent(MPS::MPSEvent event)
    {
        if (event == MPS::PEER_DOWN)
            mMsgSet.MpsPeerDownEvent(this->getMPSID(), this->getModuleName());
    }

  private:
    RawTcpMsgSetSrv_1& mMsgSet;
};

///////////////////////////////////////////////////////////////////////////////

class RawTcpMsgSetSrv_1::IfmgrClientHandler : public IFMGR_CLIENT_NS::Client::NotificationHandler
{
  public:
    IfmgrClientHandler(RawTcpMsgSetSrv_1& msgSet)
        : mMsgSet(msgSet)
    {
    }

    void HandleEnableNotification(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableVec)
    {
        mMsgSet.mApp.NotifyInterfaceEnablePending(port, ifEnableVec);
    }

  private:
    RawTcpMsgSetSrv_1& mMsgSet;
};

///////////////////////////////////////////////////////////////////////////////

RawTcpMsgSetSrv_1::RawTcpMsgSetSrv_1(RawTcpApplicationProxy& app)
    : mApp(app),
      mIfSetupObserver(fastdelegate::MakeDelegate(this, &RawTcpMsgSetSrv_1::IfSetupObserver)),
      mMpsHandler(new MpsPeerHandler(*this)),
      mIfmgrClientHandler(new IfmgrClientHandler(*this))
{
    setMessageSetName("RawTcp_1");

    const int portCount = Hal::HwInfo::getPortGroupPortCount();
    for (uint16_t port = 0; port < portCount; port++)
        mLoadProfileNotifiers.push_back(std::tr1::shared_ptr<loadProfileNotifierDB_t>(new loadProfileNotifierDB_t()));

    if (registerMessageHandler("ConfigureRawTcpClients", MakeHandler(&RawTcpMsgSetSrv_1::ConfigureRawTcpClients)) ||
        registerMessageHandler("UpdateRawTcpClients", MakeHandler(&RawTcpMsgSetSrv_1::UpdateRawTcpClients)) ||
        registerMessageHandler("DeleteRawTcpClients", MakeHandler(&RawTcpMsgSetSrv_1::DeleteRawTcpClients)) ||
        registerMessageHandler("ConfigureRawTcpServers", MakeHandler(&RawTcpMsgSetSrv_1::ConfigureRawTcpServers)) ||
        registerMessageHandler("UpdateRawTcpServers", MakeHandler(&RawTcpMsgSetSrv_1::UpdateRawTcpServers)) ||
        registerMessageHandler("DeleteRawTcpServers", MakeHandler(&RawTcpMsgSetSrv_1::DeleteRawTcpServers)) ||
        registerMessageHandler("ResetProtocol", MakeHandler(&RawTcpMsgSetSrv_1::ResetProtocol)) ||
        registerMessageHandler("StartProtocol", MakeHandler(&RawTcpMsgSetSrv_1::StartProtocol)) ||
        registerMessageHandler("StopProtocol", MakeHandler(&RawTcpMsgSetSrv_1::StopProtocol)) ||
        registerMessageHandler("ClearResults", MakeHandler(&RawTcpMsgSetSrv_1::ClearResults)) ||
        registerMessageHandler("SetDynamicLoad", MakeHandler(&RawTcpMsgSetSrv_1::SetDynamicLoad)) ||
        !mIfSetupMsgSetSrv.RegisterMessages(this) ||
        !StatsBase::RegisterMessages(this))
    {
        TC_LOG_ERR_LOCAL(0, LOG_MPS, "Failed to register at least one message handler");
    }

    mIfSetupMsgSetSrv.AttachObserver(mIfSetupObserver);
    IFMGR_CLIENT_NS::Client::AttachNotificationHandler(*mIfmgrClientHandler);
}

///////////////////////////////////////////////////////////////////////////////

RawTcpMsgSetSrv_1::~RawTcpMsgSetSrv_1()
{
    mIfSetupMsgSetSrv.DetachObserver(mIfSetupObserver);
    IFMGR_CLIENT_NS::Client::DetachNotificationHandler(*mIfmgrClientHandler);
}

///////////////////////////////////////////////////////////////////////////////

bool RawTcpMsgSetSrv_1::RegisterMPSHook(MPS *mps)
{
    if (mps->registerMPSEventHandler(MPS::PEER_DOWN, mMpsHandler.get()) < 0)
    {
        TC_LOG_ERR_LOCAL(0, LOG_MPS, "Failed to register MPS peer down event handler");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpMsgSetSrv_1::UnregisterMPSHook(MPS *mps)
{
    mps->unregisterMPSEventHandler(MPS::PEER_DOWN);
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpMsgSetSrv_1::IfSetupObserver(const IfSetupMsgSetSrv_1::Notification& notif)
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

void RawTcpMsgSetSrv_1::MpsPeerDownEvent(const MPS_Id &mpsId, const std::string& moduleName)
{
    // Remove all async notifiers for this peer
    for_each(mLoadProfileNotifiers.begin(), mLoadProfileNotifiers.end(), boost::bind(&loadProfileNotifierDB_t::remove, _1, mpsId));
    StatsBase::StopNotifications(mpsId);
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpMsgSetSrv_1::ConfigureRawTcpClients(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<RawTcpClientConfig_t> clients;
    parse_req_ConfigureRawTcpClients(req, &clients);

    // Create clients
    std::vector<uint32_t> handles;
    mApp.ConfigClients(port, clients, handles);

    // Create load profile async notifier
    const L4L7_BASE_NS::LoadProfileNotifierParams notifierParams(std::string(getMessageSetName()),
                                                                 boost::bind(&RawTcpApplicationProxy::RegisterClientLoadProfileNotifier, &mApp, port, _1, _2),
                                                                 boost::bind(&RawTcpApplicationProxy::UnregisterClientLoadProfileNotifier, &mApp, port, _1),
                                                                 handles);
    if (mLoadProfileNotifiers[port]->add(handle, port, req, 0, L4L7_BASE_NS::LoadProfileNotifier::DEFAULT_NOTIFIER_INTERVAL_MSEC, notifierParams) == false)
    {
        const std::string err("[RawTcpMsgSetSrv_1::ConfigureRawTcpClients] Unable to create async notifier");
        TC_LOG_ERR_LOCAL(port, LOG_MPS, err);
        throw EInval(err);
    }

    // Send response
    Message resp;
    create_resp_ConfigureRawTcpClients(&resp, handles);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpMsgSetSrv_1::UpdateRawTcpClients(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    std::vector<RawTcpClientConfig_t> clients;
    parse_req_UpdateRawTcpClients(req, &handles, &clients);

    // Update clients
    mApp.UpdateClients(port, handles, clients);

    // Create/update load profile async notifier
    const L4L7_BASE_NS::LoadProfileNotifierParams notifierParams(std::string(getMessageSetName()),
                                                                 boost::bind(&RawTcpApplicationProxy::RegisterClientLoadProfileNotifier, &mApp, port, _1, _2),
                                                                 boost::bind(&RawTcpApplicationProxy::UnregisterClientLoadProfileNotifier, &mApp, port, _1),
                                                                 handles);
    if (mLoadProfileNotifiers[port]->add(handle, port, req, 0, L4L7_BASE_NS::LoadProfileNotifier::DEFAULT_NOTIFIER_INTERVAL_MSEC, notifierParams) == false)
    {
        const std::string err("[RawTcpMsgSetSrv_1::UpdateRawTcpClients] Unable to create/update async notifier");
        TC_LOG_ERR_LOCAL(port, LOG_MPS, err);
        throw EInval(err);
    }

    // Send response
    Message resp;
    create_resp_UpdateRawTcpClients(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpMsgSetSrv_1::DeleteRawTcpClients(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_DeleteRawTcpClients(req, &handles);

    // Delete clients
    mApp.DeleteClients(port, handles);

    // Send response
    Message resp;
    create_resp_DeleteRawTcpClients(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpMsgSetSrv_1::ConfigureRawTcpServers(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<RawTcpServerConfig_t> servers;
    parse_req_ConfigureRawTcpServers(req, &servers);

    // Create servers
    std::vector<uint32_t> handles;
    mApp.ConfigServers(port, servers, handles);

    // Send response
    Message resp;
    create_resp_ConfigureRawTcpServers(&resp, handles);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpMsgSetSrv_1::UpdateRawTcpServers(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    std::vector<RawTcpServerConfig_t> servers;
    parse_req_UpdateRawTcpServers(req, &handles, &servers);

    // Update servers
    mApp.UpdateServers(port, handles, servers);

    // Send response
    Message resp;
    create_resp_UpdateRawTcpServers(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpMsgSetSrv_1::DeleteRawTcpServers(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_DeleteRawTcpServers(req, &handles);

    // Delete servers
    mApp.DeleteServers(port, handles);

    // Send response
    Message resp;
    create_resp_DeleteRawTcpServers(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpMsgSetSrv_1::ResetProtocol(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_ResetProtocol(req, &handles);

    // For RAWTCP, reset and stop are synonymous
    mApp.StopProtocol(port, handles);

    // Send response
    Message resp;
    create_resp_ResetProtocol(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpMsgSetSrv_1::StartProtocol(MPS_Handle *handle, uint16_t port, Message *req)
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

void RawTcpMsgSetSrv_1::StopProtocol(MPS_Handle *handle, uint16_t port, Message *req)
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

void RawTcpMsgSetSrv_1::ClearResults(MPS_Handle *handle, uint16_t port, Message *req)
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

void RawTcpMsgSetSrv_1::SetDynamicLoad(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);
    
    // Parse incoming parameters
    std::vector<DynamicLoadPair_t> loadPairs;
    RawTcp_1_port_server::parse_req_SetDynamicLoad(req, &loadPairs);
    
    // Set Dynamic Load
    mApp.SetDynamicLoad(port, loadPairs);
    
    // Send response
    Message resp;
    create_resp_SetDynamicLoad(&resp);
    handle->sendResponse(req, &resp); 

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////
