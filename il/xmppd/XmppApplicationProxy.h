/// @file
/// @brief XMPP Application Proxy header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _XMPP_APPLICATION_PROXY_H_
#define _XMPP_APPLICATION_PROXY_H_

#include <vector>

#include <ildaemon/Application.h>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <ifmgrclient/InterfaceEnable.h>
#include <ildaemon/ilDaemonCommon.h>

#include "XmppCommon.h"

// Forward declarations (global)
namespace IL_DAEMON_LIB_NS
{
    class ActiveScheduler;
}

DECL_XMPP_NS

///////////////////////////////////////////////////////////////////////////////

// Forward declaration
class XmppApplication;
class XmppMsgSetSrv_1;

class XmppApplicationProxy : public IL_DAEMON_LIB_NS::Application
{
  public:
    IL_APPLICATION_CTOR_DTOR(XmppApplicationProxy);
    
  private:
    // Application hooks from IL_DAEMON_LIB_NS::Application interface
    bool InitializeHook(uint16_t ports, const IL_DAEMON_LIB_NS::CommandLineOpts& opts);
    bool RegisterMPSHook(MPS *mps);
    void UnregisterMPSHook(MPS *mps);
    bool ActivateHook(void);
    void DeactivateHook(void);
    bool DeferActivation(void) const { return true; }

  public:
    // Convenience typedefs
    typedef std::vector<uint32_t> handleList_t;
    typedef XmppvJ_1_port_server::XmppvJClient_t clientConfig_t;
    typedef std::vector<clientConfig_t> clientConfigList_t;
    typedef boost::function2<void, uint32_t, bool> loadProfileNotifier_t;
    typedef boost::function3<void, uint32_t, XmppvJ_1_port_server::EnumXmppvJRegistrationState, XmppvJ_1_port_server::EnumXmppvJRegistrationState> regStateNotifier_t;
    typedef XmppvJ_1_port_server::DynamicLoadPair_t loadPair_t;
    typedef std::vector<loadPair_t> loadPairList_t;

    void ConfigClients(uint16_t port, const clientConfigList_t& clients, handleList_t& handles);
    void UpdateClients(uint16_t port, const handleList_t& handles, const clientConfigList_t& clients);
    void DeleteClients(uint16_t port, const handleList_t& handles);

    void RegisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles, loadProfileNotifier_t notifier);
    void UnregisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles);

    void RegisterXmppClients(uint16_t port, const handleList_t& handles);
    void UnregisterXmppClients(uint16_t port, const handleList_t& handles);
    void CancelXmppClientsRegistrations(uint16_t port, const handleList_t& handles);

    void RegisterXmppClientRegStateNotifier(uint16_t port, const handleList_t& handles, regStateNotifier_t notifier);
    void UnregisterXmppClientRegStateNotifier(uint16_t port, const handleList_t&handles);
    
    void StartProtocol(uint16_t port, const handleList_t& handles);
    void StopProtocol(uint16_t port, const handleList_t& handles);

    void ClearResults(uint16_t port, const handleList_t& handles, uint64_t absExecTime);
    void SyncResults(uint16_t port);

    void SetDynamicLoad(uint16_t port, const loadPairList_t& loadList);

    void NotifyInterfaceEnablePending(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableVec);
    void NotifyInterfaceUpdatePending(uint16_t port, const std::vector<uint32_t>& ifHandleVec);
    void NotifyInterfaceDeletePending(uint16_t port, const std::vector<uint32_t>& ifHandleVec);


    uint16_t mNumPorts;                                                 ///< number of physical ports
    boost::scoped_ptr<XmppApplication> mApp;                            ///< application object that we are proxying for
    boost::scoped_ptr<XmppMsgSetSrv_1> mMsgSet;                         ///< message set handlers
    boost::scoped_ptr<IL_DAEMON_LIB_NS::ActiveScheduler> mScheduler;    ///< active object scheduler
    size_t mNumIOThreadsPerPort;                                        ///< number of I/O worker threads per port
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_XMPP_NS

#endif
