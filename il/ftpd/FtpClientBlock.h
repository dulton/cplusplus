/// @file
/// @brief FTP Client Block header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _FTP_CLIENT_BLOCK_H_
#define _FTP_CLIENT_BLOCK_H_

#include <memory>
#include <list>

#include <ace/Thread_Mutex.h>
#include <app/AppCommon.h>
#include <app/BandwidthLoadManager.h>
#include <base/BaseCommon.h>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>
#include <Tr1Adapter.h>
#include <utils/AgingQueue.h>
#include <utils/TimingPredicate.tcc>

#include "FtpCommon.h"
#include "FtpClientRawStats.h"

// Forward declarations (global)
class ACE_Reactor;

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

namespace FTP_NS
{
    class FtpControlConnectionHandlerInterface ;
    class ClientControlConnectionHandler;
    class DataTransactionManager;
}

///////////////////////////////////////////////////////////////////////////////

DECL_FTP_NS

class FtpClientBlock : public L4L7_APP_NS::BandwidthLoadManager, boost::noncopyable
{
  public:
    /// Convenience typedefs
    typedef Ftp_1_port_server::FtpClientConfig_t config_t;
    typedef FtpClientRawStats stats_t;
    typedef boost::function2<void, uint32_t, bool> loadProfileNotifier_t;
    
    FtpClientBlock(uint16_t port, const config_t& config, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier);
    ~FtpClientBlock();

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
    
  private:
    static const size_t PURGE_HWM ;

    /// Implementation-private inner classes
    class LoadStrategy;
    class BandwidthLoadStrategy;
    class ConnectionsLoadStrategy;
    class ConnectionsOverTimeLoadStrategy;
    class TransactionsLoadStrategy;
    class TransactionsOverTimeLoadStrategy;

    struct IOLogicMessage;
    struct AppLogicMessage;
    
    /// Convenience typedefs
    typedef L4L7_BASE_NS::LoadProfile loadProfile_t;
    typedef L4L7_BASE_NS::LoadScheduler loadScheduler_t;
    typedef L4L7_BASE_NS::EndpointPairEnumerator endpointEnum_t;
    typedef ACE_Recursive_Thread_Mutex connLock_t;
    typedef L4L7_APP_NS::StreamSocketConnector<ClientControlConnectionHandler> connector_t;
    typedef L4L7_UTILS_NS::TimingPredicate<5> MsgLimit_t; // 5 msec
    typedef L4L7_APP_NS::AsyncMessageInterface<IOLogicMessage, MsgLimit_t> ioMessageInterface_t;
    typedef L4L7_APP_NS::AsyncMessageInterface<AppLogicMessage, MsgLimit_t> appMessageInterface_t;
    typedef std::tr1::shared_ptr<ClientControlConnectionHandler> connHandlerSharedPtr_t;
    typedef boost::unordered_map<uint32_t, connHandlerSharedPtr_t> connHandlerMap_t;
    typedef L4L7_UTILS_NS::AgingQueue<uint32_t> connHandlerAgingQueue_t;
    typedef std::list<connHandlerSharedPtr_t> pendingPurgeList_t ;

    /// Factory methods
    LoadStrategy *MakeLoadStrategy(void);

    /// Load strategy convenience methods
    void SetIntendedLoad(uint32_t intendedLoad);
    void SetAvailableLoad(uint32_t availableLoad, uint32_t intendedLoad);

    /// Private message handlers
    void HandleIOLogicMessage(const IOLogicMessage& msg);
    void SpawnConnections(size_t count);
    void ReapConnections(size_t count);
    void DynamicLoadHandler(size_t conn, int32_t dynamicLoad);
    void HandleAppLogicMessage(const AppLogicMessage& msg);

    // Load profile notification handler
    void LoadProfileHandler(bool running);
    void ForwardStopNotification();
    
    /// Connection event handlers
    bool HandleConnectionNotification(ClientControlConnectionHandler& connHandler);
    void HandleCloseNotification(L4L7_APP_NS::StreamSocketHandler& socketHandler);

    /// Data Connection management helper facility used by control connections
    void SetupDataCloseCb( FtpControlConnectionHandlerInterface *, DataTransactionManager *arg) ; 

    /// Post Purge, if necessary
    void PostPurgeRequest(void) ;

    /// Calculate transactions per connection
    size_t GetMaxTransactions(size_t count, size_t& countDecr);

    const uint16_t mPort;                                       ///< physical port number
    const config_t mConfig;                                     ///< client block config, profile, etc.
    stats_t mStats;                                             ///< client stats
    
    ACE_Reactor * const mAppReactor;                            ///< application reactor instance
    ACE_Reactor * const mIOReactor;                             ///< I/O reactor instance
    ACE_Lock * const mIOBarrier;                                ///< I/O barrier instance

    bool mEnabled,                                              ///< app logic: block-level enable flag
         mRunning ;                                             ///< app logic: Flag indicating if block is running.
    boost::scoped_ptr<loadProfile_t> mLoadProfile;              ///< app logic: load profile
    std::tr1::shared_ptr<LoadStrategy> mLoadStrategy;           ///< app logic: load strategy
    std::tr1::shared_ptr<loadScheduler_t> mLoadScheduler;       ///< app logic: load scheduler
    loadProfileNotifier_t mLoadProfileNotifier;                 ///< app logic: owner's load profile notifier delegate

    size_t mAttemptedConnCount;                                 ///< I/O logic: number of attempted connections
    size_t mActiveConnCount;                                    ///< I/O logic: number of active connections
    size_t mPendingConnCount;                                   ///< I/O logic: number of pending wait for reclaim connections
    boost::scoped_ptr<endpointEnum_t> mEndpointEnum;            ///< I/O logic: endpoint enumerator
    connLock_t mConnLock;                                       ///< I/O logic: protects connector against concurrent access
    boost::scoped_ptr<connector_t> mConnector;                  ///< I/O logic: socket connector
    connHandlerMap_t mConnHandlerMap;                           ///< I/O logic: active connection map, indexed by connection handler serial number
    connHandlerAgingQueue_t mConnHandlerAgingQueue;             ///< I/O logic: active connection aging deque, oldest first
    pendingPurgeList_t      mPendingPurge ;                     ///< App Logic: Purge client connections from cache.

    boost::scoped_ptr<ioMessageInterface_t> mIOLogicInterface;  ///< app logic -> I/O logic
    boost::scoped_ptr<appMessageInterface_t> mAppLogicInterface;///< I/O logic -> app logic

    uint32_t mAvailableLoadMsgsOut;
    bool mStoringStopNotification;
};

END_DECL_FTP_NS

///////////////////////////////////////////////////////////////////////////////

#endif
