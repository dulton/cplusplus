/// @file
/// @brief HTTP Server header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ABR_SERVER_H_
#define _ABR_SERVER_H_

#include "HttpServer.h"

namespace HTTP_NS
{
    class HttpServerRawStats;
    class HttpVideoServerRawStats;
    class ServerConnectionHandler;
    class AbrServerConnectionHandler;
}

namespace http {
    namespace abr {
        class AbrPlaylist;
    }
}

///////////////////////////////////////////////////////////////////////////////

DECL_HTTP_NS

class AbrServer : public Server, boost::noncopyable
{
  public:

    /// Convenience typedefs
    typedef Http_1_port_server::HttpServerConfig_t config_t;
    typedef HttpServerRawStats stats_t;
    typedef HttpVideoServerRawStats videoStats_t;
    typedef http::abr::AbrPlaylist AbrPlaylist;

    AbrServer(uint16_t port, size_t blockIndex, const config_t& config, stats_t& stats, videoStats_t& videoStats, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier, AbrPlaylist *playlist, bool enableVideoServer);
    ~AbrServer();

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
    typedef L4L7_APP_NS::StreamSocketAcceptor<AbrServerConnectionHandler> abrAcceptor_t;
    typedef std::tr1::shared_ptr<AbrServerConnectionHandler> abrConnHandlerSharedPtr_t;
    typedef boost::unordered_map<AbrServerConnectionHandler *, abrConnHandlerSharedPtr_t> abrConnHandlerMap_t;
    typedef ACE_Atomic_Op<ACE_Thread_Mutex, size_t> atomic_size_t;
    
    /// Private message handlers
    void HandleIOLogicMessage(const IOLogicMessage& msg);
    /// Connection event handlers
    bool HandleAcceptNotification(AbrServerConnectionHandler& connHandler);
    void HandleCloseNotification(L4L7_APP_NS::StreamSocketHandler& socketHandler);

    const uint16_t mPort;                                       ///< physical port number
    const size_t mBlockIndex;                                   ///< server index (in #HttpServerBlock)
    const config_t& mConfig;                                    ///< abr server config, profile, etc.
    stats_t& mStats;                                            ///< server stats
    videoStats_t& mVideoStats;                                  ///< server stats
    
    ACE_Reactor * const mAppReactor;                            ///< application reactor instance
    ACE_Reactor * const mIOReactor;                             ///< I/O reactor instance
    ACE_Lock * const mIOBarrier;                                ///< I/O barrier instance

    AbrPlaylist * const mPlaylist;

    bool mRunning;                                              ///< app logic: true when running
    
    boost::scoped_ptr<abrAcceptor_t> mAbrAcceptor;             ///< I/O logic: abr server connection acceptor
    atomic_size_t mActiveConnCount;                             ///< I/O logic: number of active connections
    abrConnHandlerMap_t mConnHandlerMap;                           ///< I/O logic: active connection map, indexed by connection handler

    boost::scoped_ptr<ioMessageInterface_t> mIOLogicInterface;  ///< app logic -> I/O logic
    bool mVideoServerEnabled;
};

END_DECL_HTTP_NS

///////////////////////////////////////////////////////////////////////////////

#endif
