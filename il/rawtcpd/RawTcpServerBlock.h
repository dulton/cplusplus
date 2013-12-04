/// @file
/// @brief Raw tcp Server Block header file 
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _RAWTCP_SERVER_BLOCK_H_
#define _RAWTCP_SERVER_BLOCK_H_

#include <vector>

#include <app/AppCommon.h>
#include <base/BaseCommon.h>
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>
#include <Tr1Adapter.h>

#include "RawTcpCommon.h"
#include "RawTcpServerRawStats.h"

// Forward declarations (global)
class ACE_Reactor;

namespace IL_DAEMON_LIB_NS
{
    class LocalInterfaceEnumerator;
}

DECL_RAWTCP_NS

///////////////////////////////////////////////////////////////////////////////

// Forward declarations
class RawTcpServer;

class RawTcpServerBlock : boost::noncopyable
{
  public:
    /// Convenience typedefs
    typedef RawTcp_1_port_server::RawTcpServerConfig_t config_t;
    typedef RawTcpServerRawStats stats_t;
    
    RawTcpServerBlock(uint16_t port, const config_t& config, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier);
    ~RawTcpServerBlock();

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

    // Interface notification methods
    void NotifyInterfaceDisabled(void);
    void NotifyInterfaceEnabled(void);
    
  private:
    typedef IL_DAEMON_LIB_NS::LocalInterfaceEnumerator ifEnum_t;
    typedef std::tr1::shared_ptr<RawTcpServer> serverPtr_t;
    typedef std::vector<serverPtr_t> serverVec_t;

    const uint16_t mPort;                               ///< physical port number
    const config_t mConfig;                             ///< server block config, profile, etc.
    stats_t mStats;                                     ///< server stats

    ACE_Reactor * const mAppReactor;                    ///< application reactor instance
    ACE_Reactor * const mIOReactor;                     ///< I/O reactor instance
    ACE_Lock * const mIOBarrier;                        ///< I/O barrier instance

    bool mEnabled;                                      ///< block-level enable flag
    bool mRunning;                                      ///< true when running, false when stopped
    boost::scoped_ptr<ifEnum_t> mIfEnum;                ///< interface enumerator
    serverVec_t mServerVec;                             ///< expanded vector of #RawTcpServer objects
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_RAWTCP_NS

#endif
