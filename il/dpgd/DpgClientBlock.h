/// @file
/// @brief Dpg Client Block header file 
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _DPG_CLIENT_BLOCK_H_
#define _DPG_CLIENT_BLOCK_H_

#include <ace/Thread_Mutex.h>
#include <app/AppCommon.h>
#include <app/BandwidthLoadManager.h>
#include <base/BaseCommon.h>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>
#include <set>
#include <Tr1Adapter.h>
#include <utils/AgingQueue.h>
#include <utils/TimingPredicate.tcc>

#include "DpgClientRawStats.h"
#include "DpgCommon.h"

// Forward declarations (global)
class ACE_Reactor;
class TestDpgClientBlock;

namespace L4L7_APP_NS
{
    template<class, class> class AsyncMessageInterface;
    template<class> class StreamSocketConnector;
    class StreamSocketHandler;
}

namespace L4L7_BASE_NS
{
    class EndpointPairEnumerator;
    class LoadProfile;
    class LoadScheduler;
    class LoadStrategy;
}

namespace L4L7_ENGINE_NS
{
    class ModifierBlock;
    class PlaylistEngine;
    class PlaylistInstance; 
}

namespace DPG_NS
{
    class SocketManager;
}

///////////////////////////////////////////////////////////////////////////////

DECL_DPG_NS

class DpgClientBlock : public L4L7_APP_NS::BandwidthLoadManager, boost::noncopyable
{
  public:
    /// Convenience typedefs
    typedef Dpg_1_port_server::DpgClientConfig_t config_t;
    typedef DpgClientRawStats stats_t;
    typedef L4L7_ENGINE_NS::PlaylistEngine playlistEngine_t;
    typedef boost::function2<void, uint32_t, bool> loadProfileNotifier_t;
    typedef std::set<uint32_t> handleSet_t;
    
    DpgClientBlock(uint16_t port, const config_t& config, playlistEngine_t *engine, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier);
    ~DpgClientBlock();

    uint16_t Port(void) const { return mPort; }

    /// Handle accessors
    uint32_t BllHandle(void) const { return mConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle; }
    uint32_t IfHandle(void) const { return mConfig.Common.Endpoint.SrcIfHandle; }
    
    /// Config accessors
    const config_t& Config(void) const { return mConfig; }
    const std::string& Name(void) const { return mConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName; }
    
    // Load profile notifier methods
    void RegisterLoadProfileNotifier(loadProfileNotifier_t notifier) { mLoadProfileNotifier = notifier; }
    void UnregisterLoadProfileNotifier(void) { mLoadProfileNotifier.clear(); }
    
    // Stats accessors
    stats_t& Stats(void) { return mStats; }
    
    // Protocol load generation methods
    void Start(void);
    void Stop(void);

    // Dynamic load messaging methods
    void SetDynamicLoad(int32_t load);
    void EnableDynamicLoad(bool enable);

    // Interface notification methods
    void NotifyInterfaceDisabled(void);
    void NotifyInterfaceEnabled(void);

    // Playlist notification
    void NotifyPlaylistUpdate(const handleSet_t& playlistHandles);
    
  private:
    /// Implementation-private inner classes
    class LoadStrategy;
    class PlaylistsLoadStrategy;
    class BandwidthLoadStrategy;

    struct IOLogicMessage;
    struct AppLogicMessage;
    
    /// Convenience typedefs
    typedef L4L7_BASE_NS::LoadProfile loadProfile_t;
    typedef L4L7_BASE_NS::LoadScheduler loadScheduler_t;
    typedef L4L7_BASE_NS::EndpointPairEnumerator endpointEnum_t;
    typedef L4L7_ENGINE_NS::ModifierBlock modifierBlock_t;
    typedef L4L7_UTILS_NS::TimingPredicate<5> MsgLimit_t; // 5 msec
    typedef L4L7_APP_NS::AsyncMessageInterface<IOLogicMessage, MsgLimit_t> ioMessageInterface_t;
    typedef L4L7_APP_NS::AsyncMessageInterface<AppLogicMessage, MsgLimit_t> appMessageInterface_t;
    typedef std::tr1::shared_ptr<L4L7_ENGINE_NS::PlaylistInstance> playInstSharedPtr_t;

    typedef boost::unordered_map<uint32_t, playInstSharedPtr_t> playInstMap_t;
    typedef ACE_Thread_Mutex playInstLock_t;
    typedef L4L7_UTILS_NS::AgingQueue<uint32_t> playInstAgingQueue_t;

    /// Factory methods
    LoadStrategy *MakeLoadStrategy(void);

    /// Load strategy convenience methods
    void SetIntendedLoad(uint32_t intendedLoad);
    void SetAvailableLoad(uint32_t availableLoad, uint32_t intendedLoad);
    
    /// Private message handlers
    void HandleIOLogicMessage(const IOLogicMessage& msg);
    void SpawnPlaylists(size_t count);
    void ReapPlaylists(size_t count);
    void PreStopPlaylists();
    void DynamicLoadHandler(size_t conn, int32_t dynamicLoad);
    void HandleAppLogicMessage(const AppLogicMessage& msg);

    // Load profile notification handler
    void LoadProfileHandler(bool running);
    void ForwardStopNotification();

    // Playlist statistic notification handler
    void HandleStatNotification();
    
    // Playlist stop notification handler
    void HandleStopNotification(L4L7_ENGINE_NS::PlaylistInstance *);
    
    /// Calculate transactions per connection
    size_t GetMaxTransactions(size_t count, size_t& countDecr);

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
    boost::scoped_ptr<loadProfile_t> mLoadProfile;              ///< app logic: load profile
    std::tr1::shared_ptr<LoadStrategy> mLoadStrategy;           ///< app logic: load strategy
    std::tr1::shared_ptr<loadScheduler_t> mLoadScheduler;       ///< app logic: load scheduler
    loadProfileNotifier_t mLoadProfileNotifier;                 ///< app logic: owner's load profile notifier delegate
    
    size_t mAttemptedConnCount;                                 ///< I/O logic: number of attempted connections
    size_t mActivePlaylists;                                    ///< I/O logic: number of active playlists 
    boost::scoped_ptr<endpointEnum_t> mEndpointEnum;            ///< I/O logic: endpoint enumerator

    boost::scoped_ptr<SocketManager> mSocketMgr;                ///< I/O logic: socket manager

    playInstMap_t mPlayInstMap;                                 ///< I/O logic: active playlist map, indexed by playlist instance serial number
    playInstLock_t mPlayInstLock;                               /// Lock for above map
    playInstAgingQueue_t mPlayInstAgingQueue;                   ///< I/O logic: active connection aging deque, oldest first

    boost::scoped_ptr<ioMessageInterface_t> mIOLogicInterface;  ///< app logic -> I/O logic
    boost::scoped_ptr<appMessageInterface_t> mAppLogicInterface;///< I/O logic -> app logic

    uint32_t mAvailableLoadMsgsOut;
    uint32_t mLastSerial;
    bool mStoringStopNotification;

    friend class ::TestDpgClientBlock;
};

END_DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

#endif
