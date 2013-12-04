/// @file
/// @brief Raw tcp Server header file 
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _RAWTCP_SERVER_H_
#define _RAWTCP_SERVER_H_

#include <string>
#include <sstream>

#include <app/AppCommon.h>
#include <ace/Atomic_Op_T.h>
#include <boost/scoped_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>
#include <Tr1Adapter.h>
#include <utils/TimingPredicate.tcc>

#include "RawTcpCommon.h"

// Forward declarations (global)
class ACE_INET_Addr;
class ACE_Reactor;

namespace L4L7_APP_NS
{
    template<class, class> class AsyncMessageInterface;
    template<class> class StreamSocketAcceptor;
    class StreamSocketHandler;
}

namespace RAWTCP_NS
{
    class RawTcpServerRawStats;
    class ServerConnectionHandler;
}

///////////////////////////////////////////////////////////////////////////////

DECL_RAWTCP_NS

class RawTcpServer : boost::noncopyable
{
  public:
    /// Convenience typedefs
    typedef RawTcp_1_port_server::RawTcpServerConfig_t config_t;
    typedef RawTcpServerRawStats stats_t;
    
    RawTcpServer(uint16_t port, size_t blockIndex, const config_t& config, stats_t& stats, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier);
    ~RawTcpServer();

    uint16_t Port(void) const { return mPort; }
                 
    /// Config accessors
    const std::string Name(void) const
    {
        std::stringstream ss;
        ss << mConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName << ":" << mBlockIndex;
        return ss.str();
    }
    
    // Protocol methods
    bool Start(const std::string& ifName, const ACE_INET_Addr& addr);
    void Stop(void);

    // Interface notification methods
    void NotifyInterfaceDisabled(void);
    void NotifyInterfaceEnabled(void);
    
  private:
    /// Implementation-private inner classes
    struct IOLogicMessage;

    /// Convenience typedefs
    typedef L4L7_UTILS_NS::TimingPredicate<5> MsgLimit_t; // 5 msec
    typedef L4L7_APP_NS::AsyncMessageInterface<IOLogicMessage, MsgLimit_t> ioMessageInterface_t;
    typedef L4L7_APP_NS::StreamSocketAcceptor<ServerConnectionHandler> acceptor_t;
    typedef std::tr1::shared_ptr<ServerConnectionHandler> connHandlerSharedPtr_t;
    typedef boost::unordered_map<uint32_t, connHandlerSharedPtr_t> connHandlerMap_t;
    typedef ACE_Atomic_Op<ACE_Thread_Mutex, size_t> atomic_size_t;
    typedef ACE_Thread_Mutex connMapLock_t;
    
    /// Private message handlers
    void HandleIOLogicMessage(const IOLogicMessage& msg);
    
    /// Connection event handlers
    bool HandleAcceptNotification(ServerConnectionHandler& connHandler);
    void HandleCloseNotification(L4L7_APP_NS::StreamSocketHandler& socketHandler);

    const uint16_t mPort;                                       ///< physical port number
    const size_t mBlockIndex;                                   ///< server index (in #RawTcpServerBlock)
    const config_t& mConfig;                                    ///< server config, profile, etc.
    stats_t& mStats;                                            ///< server stats
    
    ACE_Reactor * const mAppReactor;                            ///< application reactor instance
    ACE_Reactor * const mIOReactor;                             ///< I/O reactor instance
    ACE_Lock * const mIOBarrier;                                ///< I/O barrier instance

    bool mRunning;                                              ///< app logic: true when running
    
    boost::scoped_ptr<acceptor_t> mAcceptor;                    ///< I/O logic: client connection acceptor
    atomic_size_t mActiveConnCount;                             ///< I/O logic: number of active connections
    connHandlerMap_t mConnHandlerMap;                           ///< I/O logic: active connection map, indexed by connection handler
    connMapLock_t mConnMapLock;                                 ///< active connection map lock

    boost::scoped_ptr<ioMessageInterface_t> mIOLogicInterface;  ///< app logic -> I/O logic
};

END_DECL_RAWTCP_NS

///////////////////////////////////////////////////////////////////////////////

#endif
