/// @file
/// @brief HTTP Server implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <algorithm>
#include <netinet/in.h>

#include <app/AsyncMessageInterface.tcc>
#include <app/StreamSocketAcceptor.tcc>
#include <base/BaseCommon.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <phxexception/PHXExceptionFactory.h>

#include "HttpdLog.h"
#include "HttpServer.h"
#include "HttpServerRawStats.h"
#include "ServerConnectionHandler.h"

USING_HTTP_NS;

///////////////////////////////////////////////////////////////////////////////

struct HttpServer::IOLogicMessage
{
    enum
    {
        STOP_COMMAND,
        CLOSE_COMMAND,
    } type;
};

///////////////////////////////////////////////////////////////////////////////

HttpServer::HttpServer(uint16_t port, size_t blockIndex, const config_t& config, stats_t& stats, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier)
    : mPort(port),
      mBlockIndex(blockIndex),
      mConfig(config),
      mStats(stats),
      mAppReactor(appReactor),
      mIOReactor(ioReactor),
      mIOBarrier(ioBarrier),
      mRunning(false),
      mAcceptor(new acceptor_t),
      mActiveConnCount(0),
      mIOLogicInterface(new ioMessageInterface_t(mIOReactor))
{
    mAcceptor->SetAcceptNotificationDelegate(fastdelegate::MakeDelegate(this, &HttpServer::HandleAcceptNotification));

    mIOLogicInterface->SetMessageDelegate(fastdelegate::MakeDelegate(this, &HttpServer::HandleIOLogicMessage));

    TC_LOG_DEBUG_LOCAL(mPort, LOG_SERVER, "HTTP server [" << Name() << "] created");
}

///////////////////////////////////////////////////////////////////////////////

HttpServer::~HttpServer()
{
    Stop();
}

///////////////////////////////////////////////////////////////////////////////

bool HttpServer::Start(const std::string& ifName, const ACE_INET_Addr& addr)
{
    if (mRunning)
        throw EPHXInternal();

    char addrStr[64];
    if (addr.addr_to_string(addrStr, sizeof(addrStr)) == -1)
        strcpy(addrStr, "invalid");
        
    // Open acceptor socket
    if (mAcceptor->open(addr, mIOReactor) == -1)
    {
        TC_LOG_ERR_LOCAL(mPort, LOG_SERVER, "HTTP server [" << Name() << "] failed to start on " << addrStr << ": " << strerror(errno));
        return false;
    }

    mRunning = true;

    if (addr.get_type() == AF_INET)
        mAcceptor->SetIpv4Tos(mConfig.Common.Profile.L4L7Profile.Ipv4Tos);

    if (addr.get_type() == AF_INET6)
        mAcceptor->SetIpv6TrafficClass(mConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass);

    mAcceptor->SetTcpWindowSizeLimit(mConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit);
    mAcceptor->SetTcpDelayedAck(mConfig.Common.Profile.L4L7Profile.EnableDelayedAck);
    
    // Bind acceptor to given interface name
    if (!ifName.empty())
        mAcceptor->BindToIfName(ifName);
    
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "HTTP server [" << Name() << "] started on " << addrStr << " (" << ifName << ")");
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void HttpServer::Stop(void)
{
    if (!mRunning)
        return;
    
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "HTTP server [" << Name() << "] stopping");
    mRunning = false;

    // Ask the I/O logic to stop accepting incoming connections
    {
        ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
        IOLogicMessage& msg(*msgPtr);
        msg.type = IOLogicMessage::STOP_COMMAND;
        mIOLogicInterface->Send(msgPtr);
    }
    
    // Wait for the stop message to be processed
    mIOLogicInterface->Wait();

    // Wait for all I/O threads to exit
    {
        ACE_WRITE_GUARD(ACE_Lock, guard, *mIOBarrier);
    }
    
    // Ask the I/O logic to close any remaining connections
    {
        ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
        IOLogicMessage& msg(*msgPtr);
        msg.type = IOLogicMessage::CLOSE_COMMAND;
        mIOLogicInterface->Send(msgPtr);
    }
    
    // Wait for the close message to be processed
    mIOLogicInterface->Wait();

    // Wait for all I/O threads to exit
    {
        ACE_WRITE_GUARD(ACE_Lock, guard, *mIOBarrier);
    }
}

///////////////////////////////////////////////////////////////////////////////

void HttpServer::NotifyInterfaceDisabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "HTTP server [" << Name() << "] disabling protocol");
    Stop();
}

///////////////////////////////////////////////////////////////////////////////

void HttpServer::NotifyInterfaceEnabled(void)
{
}

///////////////////////////////////////////////////////////////////////////////

void HttpServer::HandleIOLogicMessage(const IOLogicMessage& msg)
{
    switch (msg.type)
    {
      case IOLogicMessage::STOP_COMMAND:
      {
          TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "HTTP server [" << Name() << "] stopping accepting incoming connections");

          // Suspend the acceptor
          mAcceptor->reactor()->suspend_handler(mAcceptor.get());
          
          // need a temp map to avoid deadlock
          connHandlerMap_t tempMap; 

          {
              ACE_GUARD(connMapLock_t, guard, mConnMapLock);
              tempMap = mConnHandlerMap;
          }

          // Stop timers in all active connections
          BOOST_FOREACH(const connHandlerMap_t::value_type& pair, tempMap)
          {  
              pair.second->PurgeTimers();
          }
          
          // Signal that the stop is complete
          mIOLogicInterface->Signal();
          break;
      }
      
      case IOLogicMessage::CLOSE_COMMAND:
      {
          TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "HTTP server [" << Name() << "] closing " << mActiveConnCount.value() << " active connections");
          
          // Close the acceptor
          mAcceptor->close();
          
          // need a temp map to avoid deadlock
          connHandlerMap_t tempMap; 

          {
              ACE_GUARD(connMapLock_t, guard, mConnMapLock);
              tempMap = mConnHandlerMap;
          }

          // Close all active connections
          BOOST_FOREACH(const connHandlerMap_t::value_type& pair, tempMap)
          {
              ServerConnectionHandler &connHandler(*(pair.second));

              // Remove the handler's close notification delegate (informing ourselves of a close that is self-initiated creates unnecessary overhead)
              connHandler.ClearCloseNotificationDelegate();
              
              // Close the connection
              connHandler.close();
          }
          
          {
              ACE_GUARD(connMapLock_t, guard, mConnMapLock);
              mConnHandlerMap.clear();
          }

          {
              ACE_GUARD(stats_t::lock_t, guard, mStats.Lock());
              mStats.activeConnections -= mActiveConnCount.value();
          }

          // Zero out the active connection count
          mActiveConnCount = 0;

          // Signal that the close is complete
          mIOLogicInterface->Signal();
          break;
      }

      default:
          throw EPHXInternal();
    }
}

///////////////////////////////////////////////////////////////////////////////

bool HttpServer::HandleAcceptNotification(ServerConnectionHandler& rawConnHandler)
{
    // NOTE: Store the connection handler in a shared pointer with a custom deleter -- this will call remove_reference() when the shared copy is deleted
    connHandlerSharedPtr_t connHandler(&rawConnHandler, boost::bind(&ServerConnectionHandler::remove_reference, _1));

    // Immediately close connection if we've already been stopped
    if (!mRunning)
        return false;
    
    // Enforce max simultaneous clients constraint
    if (mConfig.ProtocolConfig.MaxSimultaneousClients && mActiveConnCount >= mConfig.ProtocolConfig.MaxSimultaneousClients)
        return false;

    // Initialize connection handler
    connHandler->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &HttpServer::HandleCloseNotification));
    connHandler->SetVersion(static_cast<Http_1_port_server::HttpVersionOptions>(mConfig.ProtocolProfile.HttpVersion));
    connHandler->SetType(static_cast<Http_1_port_server::HttpServerType>(mConfig.ProtocolProfile.HttpServerTypes));
    connHandler->SetMaxRequests(mConfig.ProtocolConfig.MaxRequestsPerClient);
    connHandler->SetBodyType(static_cast<Http_1_port_server::BodyContentOptions>(mConfig.ResponseConfig.BodyContentType));
    connHandler->SetStatsInstance(mStats);

    //TCP_WINDOW_CLAMP is not supported by USS and socket recv buffer is used to
    //handle RxWindowSizeLimit.
    connHandler->SetSoRcvBuf(mConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit);
    
    const Http_1_port_server::BodySizeOptions bodySizeType = static_cast<Http_1_port_server::BodySizeOptions>(mConfig.ResponseConfig.BodySizeType);
    switch (bodySizeType)
    {
      case L4L7_BASE_NS::BodySizeOptions_FIXED:
          connHandler->SetBodySize(mConfig.ResponseConfig.FixedBodySize);
          break;

      case L4L7_BASE_NS::BodySizeOptions_RANDOM:
          connHandler->SetBodySize(mConfig.ResponseConfig.RandomBodySizeMean, mConfig.ResponseConfig.RandomBodySizeStdDeviation);
          break;

      default:
          TC_LOG_ERR_LOCAL(mPort, LOG_CLIENT, "Cannot set body size for unknown body size option type (" << tms_enum_to_string(bodySizeType, Http_1_port_server::em_BodySizeOptions) << ")");
          return false;
    }

    const Http_1_port_server::ResponseTimingOptions responseTimingType = static_cast<Http_1_port_server::ResponseTimingOptions>(mConfig.ResponseConfig.L4L7ServerResponseConfig.ResponseTimingType);
    switch (responseTimingType)
    {
      case L4L7_BASE_NS::ResponseTimingOptions_FIXED:
          connHandler->SetResponseLatency(mConfig.ResponseConfig.L4L7ServerResponseConfig.FixedResponseLatency);
          break;

      case L4L7_BASE_NS::ResponseTimingOptions_RANDOM:
          connHandler->SetResponseLatency(mConfig.ResponseConfig.L4L7ServerResponseConfig.RandomResponseLatencyMean, mConfig.ResponseConfig.L4L7ServerResponseConfig.RandomResponseLatencyStdDeviation);
          break;

      default:
          TC_LOG_ERR_LOCAL(mPort, LOG_CLIENT, "Cannot set response timing for unknown response timing option type (" << tms_enum_to_string(responseTimingType, Http_1_port_server::em_ResponseTimingOptions) << ")");
          return false;
    }
    
    {
        // Set Tos/taffic calss
        // Socket for building tcp connection and sending packets over this session are different
        // Set Tos/Traffic Class to socket which send packets(connHandler)
        int addrType = connHandler->GetAddrType(); 
        if (addrType == -1)
        {
             TC_LOG_WARN_LOCAL(mPort, LOG_CLIENT, "Cannot get local addr");
        }
        else
        {
             if (addrType == AF_INET)
             {
                  if (connHandler->SetIpv4Tos(mConfig.Common.Profile.L4L7Profile.Ipv4Tos) == -1)
                  {
                       TC_LOG_WARN_LOCAL(mPort, LOG_CLIENT, "Failed to  set IPv4 Tos");
                  }
             }
 
             if (addrType == AF_INET6)
             {                   
                  if (connHandler->SetIpv6TrafficClass(mConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass) == -1)
                  {
                       TC_LOG_WARN_LOCAL(mPort, LOG_CLIENT, "Failed to  set IPv6 Traffic Class");
                  }
             }  

        } /* end of else*/ 

    }

    {
        ACE_GUARD_RETURN(connMapLock_t, guard, mConnMapLock, false);
        mConnHandlerMap[connHandler->Serial()] = connHandler;
    }

    // Increment the active connection count
    mActiveConnCount++;

    // Bump total connections counter
    {
        ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats.Lock(), false);
        mStats.totalConnections++;
        mStats.activeConnections++;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void HttpServer::HandleCloseNotification(L4L7_APP_NS::StreamSocketHandler& socket)
{
    ServerConnectionHandler *connHandler = reinterpret_cast<ServerConnectionHandler *>(&socket);

    {
        ACE_GUARD(connMapLock_t, guard, mConnMapLock);
        if (mConnHandlerMap.erase(connHandler->Serial()) == 0)
        {
            TC_LOG_WARN_LOCAL(mPort, LOG_CLIENT, "Failed to erase conn handler");
            // This can happen during stop, don't re-decrement the counts
            return;
        }
    }

    // Decrement the active connection count
    mActiveConnCount--;

    {
        ACE_GUARD(stats_t::lock_t, guard, mStats.Lock());
        mStats.activeConnections--;
    }
}

///////////////////////////////////////////////////////////////////////////////
