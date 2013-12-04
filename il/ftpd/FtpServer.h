/// @file
/// @brief FTP Server header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _FTP_SERVER_H_
#define _FTP_SERVER_H_

#include <string>
#include <sstream>
#include <list>

#include <ace/Atomic_Op_T.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <app/AppCommon.h>
#include <boost/scoped_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>
#include <Tr1Adapter.h>
#include <utils/TimingPredicate.tcc>

#include "FtpCommon.h"

// Forward declarations (global)
class ACE_INET_Addr;
class ACE_Reactor;

namespace L4L7_APP_NS
{
    template<class, class> class AsyncMessageInterface;
    template<class> class StreamSocketAcceptor;
    template<class> class StreamSocketConnector ;
    class StreamSocketHandler;
}

namespace FTP_NS
{
    class FtpControlConnectionHandlerInterface ;
    class ServerControlConnectionHandler;
    class FtpServerRawStats ;
    class DataTransactionManager ;
}

///////////////////////////////////////////////////////////////////////////////

DECL_FTP_NS

/////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FtpServer : boost::noncopyable
{
  public:
    /// Convenience typedefs
    typedef Ftp_1_port_server::FtpServerConfig_t config_t;
    typedef FtpServerRawStats stats_t ;
    
    FtpServer(uint16_t port, size_t blockIndex, const config_t& config, stats_t& stats, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier);
    ~FtpServer();

    uint16_t Port(void) const { return mPort; }
                 
    /// Config accessors
    const std::string Name(void) const
    {
        std::stringstream ss;
        ss << mConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName << ":" << mBlockIndex;
        return ss.str();
    }
    
    // API to start the server. Will transition to "Running() == true" after this call (barring failure)
    bool Start(const std::string& ifName, const ACE_INET_Addr& addr);
    
    // API to stop the Server (i.e. transition to "Running() == false") barring failures
    void Stop(void);

    // API to check if the server is running
    bool Running(void) const { return mRunning ;}

    // Interface notification methods
    void NotifyInterfaceDisabled(void);
    void NotifyInterfaceEnabled(void);
    
  private:
    struct FtpIOLogicMessage ;
    struct FtpAppLogicMessage ;

    /// Convenience typedefs
    typedef ACE_Recursive_Thread_Mutex lock_t;
    typedef L4L7_UTILS_NS::TimingPredicate<5> MsgLimit_t;
    typedef L4L7_APP_NS::AsyncMessageInterface<FtpIOLogicMessage, MsgLimit_t> ioMessageInterface_t;
    typedef L4L7_APP_NS::AsyncMessageInterface<FtpAppLogicMessage, MsgLimit_t> appMessageInterface_t;
    typedef L4L7_APP_NS::StreamSocketAcceptor<ServerControlConnectionHandler> controlAcceptor_t;
    typedef std::tr1::shared_ptr<ServerControlConnectionHandler> controlHdlrSharedPtr_t ;
    typedef boost::unordered_map<ServerControlConnectionHandler *, controlHdlrSharedPtr_t> controlHdlrMap_t;
    typedef std::list<controlHdlrSharedPtr_t> pendingPurgeList_t ;
    typedef ACE_Atomic_Op<ACE_Thread_Mutex, size_t> atomic_size_t;

    // Get the Interface name being used by teh server
    const std::string& GetServerIfName() const { return mServerIfName ; }

    /// Private message handlers
    void HandleIOLogicMessage(const FtpIOLogicMessage& msg);    
    void HandleAppLogicMessage(const FtpAppLogicMessage& msg) ;

    /// Connection event handlers
    bool HandleAcceptNotification(ServerControlConnectionHandler& connHandler); // for acceptor
    void HandleControlCloseNotification(L4L7_APP_NS::StreamSocketHandler& socketHandler); // for acceptor

    /// Data Connection management helper facility used by control connections
    void SetupDataCloseCb(FtpControlConnectionHandlerInterface *connHandler, DataTransactionManager *arg) ; 
    
    /// Post Purge, if necessary
    void PostPurgeRequest(void) ;

    // static purge high water mark.
    static const size_t PURGE_HWM ;

    const uint16_t mPort;                                       ///< physical port number
    const size_t mBlockIndex;                                   ///< server index (in #FtpServerBlock)
    const config_t& mConfig;                                    ///< server config, profile, etc.
    stats_t&  mStats;                                           ///< parent server block's stats object.
    ACE_Reactor * const mAppReactor;                            ///< application reactor instance
    ACE_Reactor * const mIOReactor;                             ///< I/O reactor instance
    ACE_Lock * const mIOBarrier;                                ///< I/O barrier instance
    bool mRunning ;                                             ///< App Logic: true when running 
    lock_t mIOLock;                                             ///< I/O logic: protects against concurrent access from multiple reactor threads
    boost::scoped_ptr<controlAcceptor_t> mAcceptor;             ///< I/O logic: client connection acceptor
    controlHdlrMap_t mConnHandlerMap;                           ///< I/O logic: active control connection map, indexed by connection handler 
    pendingPurgeList_t      mPendingPurge ;                     ///< App Logic: Purge server connections from cache.

    atomic_size_t mActiveConnCount;                             ///< Number of active client connections

    boost::scoped_ptr<ioMessageInterface_t> mIOLogicInterface;  ///< app logic -> I/O logic
    boost::scoped_ptr<appMessageInterface_t> mAppLogicInterface;///< I/O logic -> app logic

    /// record of server's Interface name
    std::string   mServerIfName ;
};

END_DECL_FTP_NS


///////////////////////////////////////////////////////////////////////////////

#endif
