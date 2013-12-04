/// @file
/// @brief Dpg Server Block header file 
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _DPG_SERVER_BLOCK_H_
#define _DPG_SERVER_BLOCK_H_

#include <ace/Thread_Mutex.h>
#include <app/AppCommon.h>
#include <base/BaseCommon.h>
#include <boost/scoped_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>
#include <Tr1Adapter.h>
#include <utils/TimingPredicate.tcc>

#include "DpgServerRawStats.h"
#include "DpgCommon.h"

// Forward declarations (global)
class ACE_Reactor;
class TestDpgServerBlock;

namespace L4L7_APP_NS
{
    template<class, class> class AsyncMessageInterface;
    template<class> class StreamSocketConnector;
    class StreamSocketHandler;
}

namespace L4L7_ENGINE_NS
{
    class ModifierBlock;
    class PlaylistEngine;
    class PlaylistInstance; 
}

namespace IL_DAEMON_LIB_NS
{
    class LocalInterfaceEnumerator;
}

namespace DPG_NS
{
    class ServerConnectionHandler;
    class SocketManager;
}

///////////////////////////////////////////////////////////////////////////////

DECL_DPG_NS

class DpgServerBlock : boost::noncopyable
{
  public:
    /// Convenience typedefs
    typedef DpgIf_1_port_server::DpgServerConfig_t config_t;
    typedef DpgServerRawStats stats_t;
    typedef L4L7_ENGINE_NS::PlaylistEngine playlistEngine_t;
    typedef std::set<uint32_t> handleSet_t;
    
    DpgServerBlock(uint16_t port, const config_t& config, playlistEngine_t *engine, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier);
    ~DpgServerBlock();

    uint16_t Port(void) const { return mPort; }

    /// Handle accessors
    uint32_t BllHandle(void) const { return mConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle; }
    uint32_t IfHandle(void) const { return mConfig.Common.Endpoint.SrcIfHandle; }
    
    /// Config accessors
    const config_t& Config(void) const { return mConfig; }
    const std::string& Name(void) const { return mConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName; }
    
    // Stats accessors
    stats_t& Stats(void) { return mStats; }
    
    // Protocol methods
    void Start(void);
    void Stop(void);

    // Playlist stop notification handler
    void HandleStopNotification(L4L7_ENGINE_NS::PlaylistInstance *);
    
    // Interface notification methods
    void NotifyInterfaceDisabled(void);
    void NotifyInterfaceEnabled(void);
    
    // Playlist notification
    void NotifyPlaylistUpdate(const handleSet_t& playlistHandles);
    
  private:
    /// Implementation-private inner classes
    struct IOLogicMessage;
    
    /// Convenience typedefs
    typedef IL_DAEMON_LIB_NS::LocalInterfaceEnumerator ifEnum_t;
    typedef L4L7_UTILS_NS::TimingPredicate<5> MsgLimit_t; // 5 msec
    typedef L4L7_ENGINE_NS::ModifierBlock modifierBlock_t;
    typedef L4L7_APP_NS::AsyncMessageInterface<IOLogicMessage, MsgLimit_t> ioMessageInterface_t;
    typedef std::tr1::shared_ptr<L4L7_ENGINE_NS::PlaylistInstance> playInstSharedPtr_t;
    typedef boost::unordered_map<uint32_t, playInstSharedPtr_t> playInstMap_t;
    typedef ACE_Thread_Mutex playInstLock_t;

    /// Private message handlers
    void HandleIOLogicMessage(const IOLogicMessage& msg);

    /// Connection event handler
    bool ServerSpawn(L4L7_ENGINE_NS::PlaylistInstance * pi, int32_t fd, const ACE_INET_Addr& localAddr, const ACE_INET_Addr& remoteAddr);

    void PreStopPlaylists();
    const uint16_t mPort;                                       ///< physical port number
    const config_t mConfig;                                     ///< client block config, profile, etc.
    stats_t mStats;                                             ///< client stats
    
    playlistEngine_t * const mPlaylistEngine;                   ///< playlist engine

    boost::scoped_ptr<modifierBlock_t> mModifierBlock;          ///< current variable state

    ACE_Reactor * const mAppReactor;                            ///< application reactor instance
    ACE_Reactor * const mIOReactor;                             ///< I/O reactor instance
    ACE_Lock * const mIOBarrier;                                ///< I/O barrier instance

    bool mEnabled;                                              ///< app logic: block-level enable flag
    bool mRunning;                                              ///< app logic: true when running
    
    size_t mActivePlaylists;                                    ///< I/O logic: number of active playlists 
    // FIXME -- need a kind of endpoint map boost::scoped_ptr<endpointEnum_t> mEndpointEnum;            ///< I/O logic: endpoint enumerator
    
    uint32_t mLastSerial;

    boost::scoped_ptr<SocketManager> mSocketMgr;                ///< I/O logic: socket manager

    playInstMap_t mPlayInstMap;                                 ///< I/O logic: active playlist map, indexed by playlist instance serial number
    playInstLock_t mPlayInstLock;                               /// Lock for above map
    boost::scoped_ptr<ioMessageInterface_t> mIOLogicInterface;  ///< app logic -> I/O logic
    boost::scoped_ptr<ifEnum_t> mIfEnum;                        ///< interface enumerator

    friend class ::TestDpgServerBlock;
};

END_DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

#endif
