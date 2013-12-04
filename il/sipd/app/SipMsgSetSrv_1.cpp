///
/// @file
/// @brief Message set handlers for sip_1 messages
///
/// Copyright (c) 2007 by Spirent Communications Inc.
/// All Rights Reserved.
///
/// This software is confidential and proprietary to Spirent Communications Inc.
/// No part of this software may be reproduced, transmitted, disclosed or used
/// in violation of the Software License Agreement without the expressed
/// written consent of Spirent Communications Inc.
///
/// $File: //TestCenter/integration/content/traffic/l4l7/il/sipd/app/SipMsgSetSrv_1.cpp $
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
#include <mps/mps_event_handler.h>
#include <phxexception/PHXException.h>
#include <vif/IfSetupInterfaceBlock.h>

#include "SipApplicationProxy.h"
#include "SipMsgSetSrv_1.h"
#include "VoIPLog.h"

USING_APP_NS;

using namespace Sip_1_port_server;

///////////////////////////////////////////////////////////////////////////////

class SipMsgSetSrv_1::MpsPeerHandler : public MPSEventHandler
{
  public:
    MpsPeerHandler(SipMsgSetSrv_1& msgSet)
        : mMsgSet(msgSet)
    {
    }
    
    void handleEvent(MPS::MPSEvent event)
    {
        if (event == MPS::PEER_DOWN)
            mMsgSet.MpsPeerDownEvent(this->getMPSID(), this->getModuleName());
    }

  private:
    SipMsgSetSrv_1& mMsgSet;
};

///////////////////////////////////////////////////////////////////////////////

class SipMsgSetSrv_1::IfmgrClientHandler : public IFMGR_CLIENT_NS::Client::NotificationHandler
{
  public:
    IfmgrClientHandler(SipMsgSetSrv_1& msgSet)
        : mMsgSet(msgSet)
    {
    }
    
    void HandleEnableNotification(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableVec)
    {
        mMsgSet.mApp.NotifyInterfaceEnablePending(port, ifEnableVec);
    }

  private:
    SipMsgSetSrv_1& mMsgSet;
};

///////////////////////////////////////////////////////////////////////////////

SipMsgSetSrv_1::SipMsgSetSrv_1(SipApplicationProxy& app)
    : mApp(app),
      mIfSetupObserver(fastdelegate::MakeDelegate(this, &SipMsgSetSrv_1::IfSetupObserver)),
      mMpsHandler(new MpsPeerHandler(*this)),
      mIfmgrClientHandler(new IfmgrClientHandler(*this))
{
    setMessageSetName("Sip_1");

    const int portCount = Hal::HwInfo::getPortGroupPortCount();
    for (uint16_t port = 0; port < portCount; port++)
    {
        mLoadProfileNotifiers.push_back(std::tr1::shared_ptr<loadProfileNotifierDB_t>(new loadProfileNotifierDB_t()));
        mUaRegStateNotifiers.push_back(std::tr1::shared_ptr<uaRegStateNotifierDB_t>(new uaRegStateNotifierDB_t()));
    }
    
    if (registerMessageHandler("ConfigureSipUserAgents", MakeHandler(&SipMsgSetSrv_1::ConfigureSipUserAgents)) ||
        registerMessageHandler("UpdateSipUserAgents", MakeHandler(&SipMsgSetSrv_1::UpdateSipUserAgents)) ||
        registerMessageHandler("DeleteSipUserAgents", MakeHandler(&SipMsgSetSrv_1::DeleteSipUserAgents)) ||
        registerMessageHandler("RegisterSipUserAgents", MakeHandler(&SipMsgSetSrv_1::RegisterSipUserAgents)) ||
        registerMessageHandler("UnregisterSipUserAgents", MakeHandler(&SipMsgSetSrv_1::UnregisterSipUserAgents)) ||
        registerMessageHandler("CancelSipUserAgentsRegistrations", MakeHandler(&SipMsgSetSrv_1::CancelSipUserAgentsRegistrations)) ||
        registerMessageHandler("StartProtocol", MakeHandler(&SipMsgSetSrv_1::StartProtocol)) ||
        registerMessageHandler("StopProtocol", MakeHandler(&SipMsgSetSrv_1::StopProtocol)) ||
        registerMessageHandler("ClearResults", MakeHandler(&SipMsgSetSrv_1::ClearResults)) ||
        !mIfSetupMsgSetSrv.RegisterMessages(this) ||
        !StatsBase::RegisterMessages(this))
    {
        TC_LOG_ERR_LOCAL(0, LOG_MPS, "[SipMsgSetSrv_1 ctor] Failed to register at least one message handler");
    }

    mIfSetupMsgSetSrv.AttachObserver(mIfSetupObserver);
    IFMGR_CLIENT_NS::Client::AttachNotificationHandler(*mIfmgrClientHandler);
}

///////////////////////////////////////////////////////////////////////////////

SipMsgSetSrv_1::~SipMsgSetSrv_1()
{
    mIfSetupMsgSetSrv.DetachObserver(mIfSetupObserver);
    IFMGR_CLIENT_NS::Client::DetachNotificationHandler(*mIfmgrClientHandler);
}

///////////////////////////////////////////////////////////////////////////////

bool SipMsgSetSrv_1::RegisterMPSHook(MPS *mps)
{
    if (mps->registerMPSEventHandler(MPS::PEER_DOWN, mMpsHandler.get()) < 0)
    {
        TC_LOG_ERR_LOCAL(0, LOG_MPS, "Failed to register MPS peer down event handler");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void SipMsgSetSrv_1::UnregisterMPSHook(MPS *mps)
{
    mps->unregisterMPSEventHandler(MPS::PEER_DOWN);
}

///////////////////////////////////////////////////////////////////////////////

void SipMsgSetSrv_1::IfSetupObserver(const IfSetupMsgSetSrv_1::Notification& notif)
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
          
      case IfSetupMsgSetSrv_1::Notification::UPDATE_COMPLETED_EVENT:
      {
          // Transform block list to handle list
          std::vector<uint32_t> ifHandleVec;
          ifHandleVec.reserve(notif.ifBlockList.size());
          transform(notif.ifBlockList.begin(), notif.ifBlockList.end(), std::back_inserter(ifHandleVec), boost::mem_fn(&IFSETUP_NS::InterfaceBlock::HandleValue));

          // Virtual interfaces have been updated -- inform all associated clients and servers
          mApp.NotifyInterfaceUpdateCompleted(notif.port, ifHandleVec);
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

      default:
          break;
    }
}

///////////////////////////////////////////////////////////////////////////////

void SipMsgSetSrv_1::MpsPeerDownEvent(const MPS_Id &mpsId, const std::string& moduleName)
{
    // Remove all async notifiers for this peer
    for_each(mLoadProfileNotifiers.begin(), mLoadProfileNotifiers.end(), boost::bind(&loadProfileNotifierDB_t::remove, _1, mpsId));
    for_each(mUaRegStateNotifiers.begin(), mUaRegStateNotifiers.end(), boost::bind(&uaRegStateNotifierDB_t::remove, _1, mpsId));
    StatsBase::StopNotifications(mpsId);
}

///////////////////////////////////////////////////////////////////////////////

void SipMsgSetSrv_1::ConfigureSipUserAgents(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<SipUaConfig_t> userAgents;
    parse_req_ConfigureSipUserAgents(req, &userAgents);

    // Create user agents
    std::vector<uint32_t> handles;
    mApp.ConfigUserAgents(port, userAgents, handles);

    // Create load profile async notifier
    const L4L7_BASE_NS::LoadProfileNotifierParams notifierParams(std::string(getMessageSetName()),
                                                                 boost::bind(&SipApplicationProxy::RegisterUserAgentLoadProfileNotifier, &mApp, port, _1, _2),
                                                                 boost::bind(&SipApplicationProxy::UnregisterUserAgentLoadProfileNotifier, &mApp, port, _1),
                                                                 handles);
    if (mLoadProfileNotifiers[port]->add(handle, port, req, 0, L4L7_BASE_NS::LoadProfileNotifier::DEFAULT_NOTIFIER_INTERVAL_MSEC, notifierParams) == false)
    {
        const std::string err("[SipMsgSetSrv_1::ConfigureSipUserAgents] Unable to create async notifier");
        TC_LOG_ERR_LOCAL(port, LOG_MPS, err);
        throw EInval(err);
    }

    // Send response
    Message resp;
    create_resp_ConfigureSipUserAgents(&resp, handles);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void SipMsgSetSrv_1::UpdateSipUserAgents(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    std::vector<SipUaConfig_t> userAgents;
    parse_req_UpdateSipUserAgents(req, &handles, &userAgents);

    // Update user agents
    mApp.UpdateUserAgents(port, handles, userAgents);

    // Create/update load profile async notifier
    const L4L7_BASE_NS::LoadProfileNotifierParams notifierParams(std::string(getMessageSetName()),
                                                                 boost::bind(&SipApplicationProxy::RegisterUserAgentLoadProfileNotifier, &mApp, port, _1, _2),
                                                                 boost::bind(&SipApplicationProxy::UnregisterUserAgentLoadProfileNotifier, &mApp, port, _1),
                                                                 handles);
    if (mLoadProfileNotifiers[port]->add(handle, port, req, 0, L4L7_BASE_NS::LoadProfileNotifier::DEFAULT_NOTIFIER_INTERVAL_MSEC, notifierParams) == false)
    {
        const std::string err("[SipMsgSetSrv_1::UpdateSipUserAgents] Unable to create/update async notifier");
        TC_LOG_ERR_LOCAL(port, LOG_MPS, err);
        throw EInval(err);
    }
    
    // Send response
    Message resp;
    create_resp_UpdateSipUserAgents(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void SipMsgSetSrv_1::DeleteSipUserAgents(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_DeleteSipUserAgents(req, &handles);

    // Delete user agents
    mApp.DeleteUserAgents(port, handles);

    // Send response
    Message resp;
    create_resp_DeleteSipUserAgents(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void SipMsgSetSrv_1::RegisterSipUserAgents(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    bool asyncNotification;
    uint32_t asyncNotifierId;
    uint32_t asyncNotifierIntervalMsec;
    parse_req_RegisterSipUserAgents(req, &handles, &asyncNotification, &asyncNotifierId, &asyncNotifierIntervalMsec);

    // Create registration state async notifier as necessary
    if (asyncNotification)
    {
        if (mUaRegStateNotifiers[port]->add(handle, port, req, asyncNotifierId, asyncNotifierIntervalMsec, UserAgentRegStateNotifierParams(mApp, handles)) == false)
        {
            const std::string err("[SipMsgSetSrv_1::RegisterSipUserAgents] Unable to create async notifier");
            TC_LOG_ERR_LOCAL(port, LOG_MPS, err);
            throw EInval(err);
        }
    }
    
    // Register user agents
    mApp.RegisterUserAgents(port, handles);

    // Send response
    Message resp;
    create_resp_RegisterSipUserAgents(&resp);
    handle->sendResponse(req, &resp);
    
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void SipMsgSetSrv_1::UnregisterSipUserAgents(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_UnregisterSipUserAgents(req, &handles);

    // Start user agent unregistrations
    mApp.UnregisterUserAgents(port, handles);

    // Send response
    Message resp;
    create_resp_UnregisterSipUserAgents(&resp);
    handle->sendResponse(req, &resp);
    
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void SipMsgSetSrv_1::CancelSipUserAgentsRegistrations(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_CancelSipUserAgentsRegistrations(req, &handles);

    // Cancel user agent registrations
    mApp.CancelUserAgentsRegistrations(port, handles);

    // Send response
    Message resp;
    create_resp_CancelSipUserAgentsRegistrations(&resp);
    handle->sendResponse(req, &resp);
    
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void SipMsgSetSrv_1::ResetProtocol(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_ResetProtocol(req, &handles);

    // Reset protocol (registration and calls)
    mApp.ResetProtocol(port, handles);

    // Send response
    Message resp;
    create_resp_ResetProtocol(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void SipMsgSetSrv_1::StartProtocol(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_StartProtocol(req, &handles);

    // Start protocol (calls)
    mApp.StartProtocol(port, handles);

    // Send response
    Message resp;
    create_resp_StartProtocol(&resp);
    handle->sendResponse(req, &resp);

    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void SipMsgSetSrv_1::StopProtocol(MPS_Handle *handle, uint16_t port, Message *req)
{
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Received message: " << __FUNCTION__);

    // Parse incoming parameters
    std::vector<uint32_t> handles;
    parse_req_StopProtocol(req, &handles);

    // Stop protocol (calls)
    mApp.StopProtocol(port, handles);

    // Send response
    Message resp;
    create_resp_StopProtocol(&resp);
    handle->sendResponse(req, &resp);

    
    TC_LOG_DEBUG_LOCAL(port, LOG_MPS, "Processed message: " << __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////

void SipMsgSetSrv_1::ClearResults(MPS_Handle *handle, uint16_t port, Message *req)
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
