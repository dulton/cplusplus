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
/// $File: //TestCenter/integration/content/traffic/l4l7/il/xmppd/XmppMsgSetSrv_1.h $
/// $Revision: #1 $
/// \n<b>Last submission by:</b>
/// <ul>
/// <li>$Author: Chinh.Le $</li>
/// <li>$DateTime: 2011/10/20 20:02:30 $</li>
/// <li>$Change: 584803 $</li>
/// </ul>
///

#ifndef _XMPP_MSG_SET_SRV_1_H_
#define _XMPP_MSG_SET_SRV_1_H_

#include <vector>

#include <base/LoadProfileNotifier.h>
#include <boost/scoped_ptr.hpp>
#include <DelegatedMessageHandler.h>
#include <mps/mps_message_set.h>
#include <NotifierDB.h>
#include <statsframework/StatsMsgSetSrv_1.h>
#include <vif/IfSetupMsgSetSrv_1.h>

#include "XmppCommon.h"
#include "XmppClientRegStateNotifier.h"

DECL_XMPP_NS

///////////////////////////////////////////////////////////////////////////////

// Forward declarations
class XmppApplicationProxy;

class XmppMsgSetSrv_1 : public MessageSet, public StatsMsgSetSrv_1<&XmppvJ_1_port_server::create_req_DoSQLNotify,
                                                                   &XmppvJ_1_port_server::create_req_TableEventNotify>
{
  public:
    XmppMsgSetSrv_1(XmppApplicationProxy& app);
    ~XmppMsgSetSrv_1();

    bool RegisterMPSHook(MPS *mps);
    void UnregisterMPSHook(MPS *mps);

  private:
    /// Utility method to make DelegateMessageHandlers for this message set
    DelegatedMessageHandler *MakeHandler(void (XmppMsgSetSrv_1::* handler)(MPS_Handle *handle, uint16_t port, Message *req))
    {
        return new DelegatedMessageHandler(fastdelegate::MakeDelegate(this, handler));
    }

    /// Handle interface notifications from ifSetup handlers
    void IfSetupObserver(const IfSetupMsgSetSrv_1::Notification& notif);

    /// Handle MPS peer-down events
    void MpsPeerDownEvent(const MPS_Id &mpsId, const std::string& moduleName);
    
    /// Methods to handle PHX-RPC messages from XmppvJ_1.idl
    void ConfigureXmppvJClients(MPS_Handle *handle, uint16_t port, Message *req);
    void UpdateXmppvJClients(MPS_Handle *handle, uint16_t port, Message *req);
    void DeleteXmppvJClients(MPS_Handle *handle, uint16_t port, Message *req);
    void EnumerateCapabilities(MPS_Handle *handle, uint16_t port, Message *req);

    void RegisterXmppvJClients(MPS_Handle *handle, uint16_t port, Message *req);
    void UnregisterXmppvJClients(MPS_Handle *handle, uint16_t port, Message *req);
    void CancelXmppvJClientsRegistrations(MPS_Handle *handle, uint16_t port, Message *req);

    
    /// Methods to handle PHX-RPC messages from l4l7Base_1.idl
    void ResetProtocol(MPS_Handle *handle, uint16_t port, Message *req);
    void StartProtocol(MPS_Handle *handle, uint16_t port, Message *req);
    void StopProtocol(MPS_Handle *handle, uint16_t port, Message *req);
    void ClearResults(MPS_Handle *handle, uint16_t port, Message *req);
    void SetDynamicLoad(MPS_Handle *handle, uint16_t port, Message *req);
    
    /// Implementation-private classes
    class MpsPeerHandler;
    class IfmgrClientHandler;

    typedef StatsMsgSetSrv_1<&XmppvJ_1_port_server::create_req_DoSQLNotify, &XmppvJ_1_port_server::create_req_TableEventNotify> StatsBase;
    typedef NotifierDB<L4L7_BASE_NS::LoadProfileNotifier, L4L7_BASE_NS::LoadProfileNotifierParams> loadProfileNotifierDB_t;
    typedef NotifierDB<XmppClientRegStateNotifier, XmppClientRegStateNotifierParams> XmppClientRegStateNotifierDB_t;
    
    XmppApplicationProxy& mApp;                                 ///< application that owns us
    
    IfSetupMsgSetSrv_1 mIfSetupMsgSetSrv;                       ///< ifSetup_1 handlers
    IfSetupMsgSetSrv_1::observer_t mIfSetupObserver;            ///< ifSetup_1 observer

    boost::scoped_ptr<MpsPeerHandler> mMpsHandler;              ///< handler for MPS peer-down events
    boost::scoped_ptr<IfmgrClientHandler> mIfmgrClientHandler;  ///< handler for interface enable/disable events

    std::vector<std::tr1::shared_ptr<loadProfileNotifierDB_t> > mLoadProfileNotifiers;
    std::vector<std::tr1::shared_ptr<XmppClientRegStateNotifierDB_t> > mXmppClientRegStateNotifiers;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_XMPP_NS

#endif
