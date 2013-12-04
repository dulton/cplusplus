/// @file
/// @brief HTTP Application header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _HTTP_APPLICATION_H_
#define _HTTP_APPLICATION_H_

#include <vector>

#include <ace/Time_Value.h>
#include <app/AppCommon.h>
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>
#include <ifmgrclient/InterfaceEnable.h>

#include "HttpCommon.h"

// Forward declarations (global)
class ACE_Reactor;

#ifdef UNIT_TEST
class TestHttpApplication;
class TestHttpAbrApplication;
#endif

DECL_HTTP_NS

///////////////////////////////////////////////////////////////////////////////

// Forward declarations
#ifdef UNIT_TEST
class HttpClientRawStats;
class HttpServerRawStats;
class HttpVideoClientRawStats;
class HttpVideoServerRawStats;
#endif

class HttpApplication : boost::noncopyable
{
  public:
    // Convenience typedefs
    typedef std::vector<uint32_t> handleList_t;
    typedef Http_1_port_server::HttpClientConfig_t clientConfig_t;
    typedef std::vector<clientConfig_t> clientConfigList_t;
    typedef boost::function2<void, uint32_t, bool> loadProfileNotifier_t;
    typedef Http_1_port_server::HttpServerConfig_t serverConfig_t;
    typedef std::vector<serverConfig_t> serverConfigList_t;
    typedef Http_1_port_server::DynamicLoadPair_t loadPair_t;
    typedef std::vector<loadPair_t> loadPairList_t;
    
    HttpApplication(uint16_t ports);
    ~HttpApplication();

    void Activate(ACE_Reactor *appReactor, const std::vector<ACE_Reactor *>& ioReactorVec, const std::vector<ACE_Lock *>& ioBarrierVec);
    void Deactivate(void);
    
    void ConfigClients(uint16_t port, const clientConfigList_t& clients, handleList_t& handles);
    void UpdateClients(uint16_t port, const handleList_t& handles, const clientConfigList_t& clients);
    void DeleteClients(uint16_t port, const handleList_t& handles);

    void RegisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles, loadProfileNotifier_t notifier);
    void UnregisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles);
    
    void ConfigServers(uint16_t port, const serverConfigList_t& servers, handleList_t& handles);
    void UpdateServers(uint16_t port, const handleList_t& handles, const serverConfigList_t& servers);
    void DeleteServers(uint16_t port, const handleList_t& handles);

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

    void AddPort(uint16_t port, ACE_Reactor *ioReactor = NULL, ACE_Lock *ioBarrier = NULL);
    void DoStatsClear(uint16_t port, const handleList_t& handles);
    void StatsClearTimeoutHandler(const ACE_Time_Value& tv, const void *act);

#ifdef UNIT_TEST
    void GetHttpClientStats(uint16_t port, const handleList_t& handles, std::vector<HttpClientRawStats>& stats) const;
    void GetHttpServerStats(uint16_t port, const handleList_t& handles, std::vector<HttpServerRawStats>& stats) const;
    void GetHttpVideoClientStats(uint16_t port, const handleList_t& handles, std::vector<HttpVideoClientRawStats>& stats) const;
    void GetHttpVideoServerStats(uint16_t port, const handleList_t& handles, std::vector<HttpVideoServerRawStats>& stats) const;
#endif
    
    static const ACE_Time_Value MAX_STATS_AGE;
    
    ACE_Reactor *mAppReactor;                           ///< application reactor instance
    typedef boost::unordered_map<uint16_t, PortContext> PortContextMap_t;
    PortContextMap_t  mPorts;                           ///< per-port data structures
    uint16_t          mNumPorts;

#ifdef UNIT_TEST
    friend class ::TestHttpApplication;
    friend class ::TestHttpAbrApplication;
#endif    
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_HTTP_NS

#endif
