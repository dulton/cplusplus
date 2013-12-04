/// @file
/// @brief XMPP Application header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _XMPP_APPLICATION_H_
#define _XMPP_APPLICATION_H_

#include <vector>

#include <ace/Time_Value.h>
#include <app/AppCommon.h>
#include <boost/function.hpp>
#include <boost/utility.hpp>
#include <ifmgrclient/InterfaceEnable.h>
#include <Tr1Adapter.h>

#include "XmppClientBlock.h"
#include "XmppCommon.h"

// Forward declarations (global)
class ACE_Reactor;

#ifdef UNIT_TEST
class TestXmppApplication;
#endif

DECL_XMPP_NS

///////////////////////////////////////////////////////////////////////////////

// Forward declarations
#ifdef UNIT_TEST
class XmppClientRawStats;
#endif

class XmppApplication : boost::noncopyable
{
  public:
    // Convenience typedefs
    typedef std::vector<uint32_t> handleList_t;
    typedef XmppvJ_1_port_server::XmppvJClient_t clientConfig_t;
    typedef std::vector<clientConfig_t> clientConfigList_t;
    typedef boost::function2<void, uint32_t, bool> loadProfileNotifier_t;
    typedef boost::function3<void, uint32_t, XmppvJ_1_port_server::EnumXmppvJRegistrationState, XmppvJ_1_port_server::EnumXmppvJRegistrationState> regStateNotifier_t;

    typedef XmppvJ_1_port_server::DynamicLoadPair_t loadPair_t;
    typedef std::vector<loadPair_t> loadPairList_t;
    
    XmppApplication(uint16_t ports);
    ~XmppApplication();

    void Activate(ACE_Reactor *appReactor, const std::vector<ACE_Reactor *>& ioReactorVec, const std::vector<ACE_Lock *>& ioBarrierVec);
    void Deactivate(void);
    
    void ConfigClients(uint16_t port, const clientConfigList_t& clients, handleList_t& handles);
    void UpdateClients(uint16_t port, const handleList_t& handles, const clientConfigList_t& clients);
    void DeleteClients(uint16_t port, const handleList_t& handles);

    void RegisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles, loadProfileNotifier_t notifier);
    void UnregisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles);

    void RegisterXmppClients(uint16_t port, const handleList_t& handles);
    void UnregisterXmppClients(uint16_t port, const handleList_t& handles);
    void CancelXmppClientsRegistrations(uint16_t port, const handleList_t& handles);

    void RegisterXmppClientRegStateNotifier(uint16_t port, const handleList_t& handles, regStateNotifier_t notifier);
    void UnregisterXmppClientRegStateNotifier(uint16_t port, const handleList_t& handles);

    void StartProtocol(uint16_t port, const handleList_t& handles);
    void StopProtocol(uint16_t port, const handleList_t& handles);

    void ClearResults(uint16_t port, const handleList_t& handles, uint64_t absExecTime);
    void SyncResults(uint16_t port);

    void SetDynamicLoad(uint16_t port, const loadPairList_t& loadList);

    void NotifyInterfaceEnablePending(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableVec);
    void NotifyInterfaceUpdatePending(uint16_t port, const std::vector<uint32_t>& ifHandleVec);
    void NotifyInterfaceDeletePending(uint16_t port, const std::vector<uint32_t>& ifHandleVec);

  private:
    /// Implementation-private port context data structure
    struct PortContext;

    void DoStatsClear(uint16_t port, const handleList_t& handles);
    void StatsClearTimeoutHandler(const ACE_Time_Value& tv, const void *act);

#ifdef UNIT_TEST
    void GetXmppClientStats(uint16_t port, const handleList_t& handles, std::vector<XmppClientRawStats>& stats) const;
#endif
    void RegStateCompletionHandler(std::tr1::shared_ptr<XmppClientBlock> clientBlock, PortContext& thisPort); 
    static const ACE_Time_Value MAX_STATS_AGE;
    
    ACE_Reactor *mAppReactor;                           ///< application reactor instance
    std::vector<PortContext> mPorts;                    ///< per-port data structures

#ifdef UNIT_TEST
    friend class ::TestXmppApplication;
#endif    
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_XMPP_NS

#endif
