/// @file
/// @brief FTP Application header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _FTP_APPLICATION_H_
#define _FTP_APPLICATION_H_

#include <vector>

#include <ace/Time_Value.h>
#include <app/AppCommon.h>
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>
#include <ifmgrclient/InterfaceEnable.h>

#include "FtpCommon.h"

// Forward declarations (global)
class ACE_Reactor;

DECL_FTP_NS

///////////////////////////////////////////////////////////////////////////////

class FtpApplication : boost::noncopyable
{
  public:
    // Convenience typedefs
    typedef std::vector<uint32_t> handleList_t;
    typedef Ftp_1_port_server::FtpClientConfig_t clientConfig_t;
    typedef std::vector<clientConfig_t> clientConfigList_t;
    typedef boost::function2<void, uint32_t, bool> loadProfileNotifier_t;
    typedef Ftp_1_port_server::FtpServerConfig_t serverConfig_t;
    typedef std::vector<serverConfig_t> serverConfigList_t;
    typedef Ftp_1_port_server::DynamicLoadPair_t loadPair_t;
    typedef std::vector<loadPair_t> loadPairList_t;
    
    FtpApplication(uint16_t ports);
    ~FtpApplication();

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

    static const ACE_Time_Value MAX_STATS_AGE;
    
    ACE_Reactor *mAppReactor;                           ///< application reactor instance
    typedef boost::unordered_map<uint16_t, PortContext> PortContextMap_t;
    PortContextMap_t    mPorts;                         ///< per-port data structures
    uint16_t            mNumPorts;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_FTP_NS

#endif
