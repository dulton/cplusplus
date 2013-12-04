/// @file
/// @brief Dpg Application Proxy header file
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _DPG_APPLICATION_PROXY_H_
#define _DPG_APPLICATION_PROXY_H_

#include <vector>

#include <ildaemon/Application.h>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <ifmgrclient/InterfaceEnable.h>

#include "DpgCommon.h"

// Forward declarations (global)
namespace IL_DAEMON_LIB_NS 
{
    class ActiveScheduler;
}

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

// Forward declaration
class DpgApplication;
class DpgMsgSetSrv_1;

class DpgApplicationProxy : public IL_DAEMON_LIB_NS::Application
{
  public:
    IL_APPLICATION_CTOR_DTOR(DpgApplicationProxy);
    
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
    typedef Dpg_1_port_server::DpgFlowConfig_t flowConfig_t;
    typedef std::vector<flowConfig_t> flowConfigList_t;
    typedef Dpg_1_port_server::DpgPlaylistConfig_t playlistConfig_t;
    typedef std::vector<playlistConfig_t> playlistConfigList_t;
    typedef Dpg_1_port_server::DpgClientConfig_t clientConfig_t;
    typedef std::vector<clientConfig_t> clientConfigList_t;
    typedef boost::function2<void, uint32_t, bool> loadProfileNotifier_t;
    typedef Dpg_1_port_server::DpgServerConfig_t serverConfig_t;
    typedef std::vector<serverConfig_t> serverConfigList_t;
    typedef Dpg_1_port_server::DynamicLoadPair_t loadPair_t;
    typedef std::vector<loadPair_t> loadPairList_t;

    void UpdateFlows(uint16_t port, const handleList_t& handles, const flowConfigList_t& flows);
    void DeleteFlows(uint16_t port, const handleList_t& handles);

    void UpdatePlaylists(uint16_t port, const handleList_t& handles, const playlistConfigList_t& playlists);
    void DeletePlaylists(uint16_t port, const handleList_t& handles);

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

    uint16_t mNumPorts;                                                 ///< number of physical ports
    boost::scoped_ptr<DpgApplication> mApp;                            ///< application object that we are proxying for
    boost::scoped_ptr<DpgMsgSetSrv_1> mMsgSet;                         ///< message set handlers
    boost::scoped_ptr<IL_DAEMON_LIB_NS::ActiveScheduler> mScheduler;         ///< active object scheduler
    size_t mNumIOThreadsPerPort;                                        ///< number of I/O worker threads per port
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
