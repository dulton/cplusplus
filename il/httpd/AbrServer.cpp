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
#include <vector>

#include <app/AsyncMessageInterface.tcc>
#include <app/StreamSocketAcceptor.tcc>
#include <base/BaseCommon.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <phxexception/PHXExceptionFactory.h>

#include "HttpdLog.h"
#include "AbrServer.h"
#include "HttpServerRawStats.h"
#include "HttpVideoServerRawStats.h"
#include "AbrServerConnectionHandler.h"

USING_HTTP_NS;

///////////////////////////////////////////////////////////////////////////////

struct AbrServer::IOLogicMessage
{
    enum
    {
        STOP_COMMAND,
        CLOSE_COMMAND,
        ACCEPT_NOTIFICATION,
        CLOSE_NOTIFICATION
    } type;

    // Accept notification parameters
    abrConnHandlerSharedPtr_t acceptedConnHandler;
    
    // Close notification parameters
    AbrServerConnectionHandler *closedConnHandler;
};

///////////////////////////////////////////////////////////////////////////////

AbrServer::AbrServer(uint16_t port, size_t blockIndex, const config_t& config, stats_t& stats, videoStats_t& videoStats, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier, AbrPlaylist *playlist, bool enableVideoServer)
    : mPort(port),
      mBlockIndex(blockIndex),
      mConfig(config),
      mStats(stats),
      mVideoStats(videoStats),
      mAppReactor(appReactor),
      mIOReactor(ioReactor),
      mIOBarrier(ioBarrier),
      mPlaylist(playlist),
      mRunning(false),
      mAbrAcceptor(new abrAcceptor_t),
      mActiveConnCount(0),
      mIOLogicInterface(new ioMessageInterface_t(mIOReactor)),
      mVideoServerEnabled(enableVideoServer)
{
    mAbrAcceptor->SetAcceptNotificationDelegate(fastdelegate::MakeDelegate(this, &AbrServer::HandleAcceptNotification));

    mIOLogicInterface->SetMessageDelegate(fastdelegate::MakeDelegate(this, &AbrServer::HandleIOLogicMessage));

    TC_LOG_DEBUG_LOCAL(mPort, LOG_SERVER, "HTTP ABR server [" << Name() << "] created");
}

///////////////////////////////////////////////////////////////////////////////

AbrServer::~AbrServer()
{
    Stop();
}

///////////////////////////////////////////////////////////////////////////////

bool AbrServer::Start(const std::string& ifName, const ACE_INET_Addr& addr)
{
	if (mRunning)
        throw EPHXInternal();

    char addrStr[64];
    if (addr.addr_to_string(addrStr, sizeof(addrStr)) == -1)
        strcpy(addrStr, "invalid");

    // Open acceptor socket
    if (mAbrAcceptor->open(addr, mIOReactor) == -1)
    {
        TC_LOG_ERR_LOCAL(mPort, LOG_SERVER, "HTTP server [" << Name() << "] failed to start on " << addrStr << ": " << strerror(errno));
        return false;
    }

    mRunning = true;

    if (addr.get_type() == AF_INET)
        mAbrAcceptor->SetIpv4Tos(mConfig.Common.Profile.L4L7Profile.Ipv4Tos);

    if (addr.get_type() == AF_INET6)
        mAbrAcceptor->SetIpv6TrafficClass(mConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass);

    mAbrAcceptor->SetTcpWindowSizeLimit(mConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit);
    mAbrAcceptor->SetTcpDelayedAck(mConfig.Common.Profile.L4L7Profile.EnableDelayedAck);

    // Bind acceptor to given interface name
    if (!ifName.empty())
        mAbrAcceptor->BindToIfName(ifName);

    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "HTTP ABR server [" << Name() << "] started on " << addrStr << " (" << ifName << ")");
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AbrServer::Stop(void)
{
    if (!mRunning)
        return;
    
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "HTTP ABR server [" << Name() << "] stopping");
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

void AbrServer::NotifyInterfaceDisabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "HTTP ABR server [" << Name() << "] disabling protocol");
    Stop();
}

///////////////////////////////////////////////////////////////////////////////

void AbrServer::NotifyInterfaceEnabled(void)
{
}

///////////////////////////////////////////////////////////////////////////////

void AbrServer::HandleIOLogicMessage(const IOLogicMessage& msg)
{
    switch (msg.type)
    {
      case IOLogicMessage::STOP_COMMAND:
      {
          TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "HTTP ABR server [" << Name() << "] stopping accepting incoming connections");

          // Suspend the acceptor
          mAbrAcceptor->reactor()->suspend_handler(mAbrAcceptor.get());

          // Stop timers in all active connections
          BOOST_FOREACH(const abrConnHandlerMap_t::value_type& pair, mConnHandlerMap)
              pair.first->PurgeTimers();

          // Signal that the stop is complete
          mIOLogicInterface->Signal();
          break;
      }
      
      case IOLogicMessage::CLOSE_COMMAND:
      {
          TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "HTTP ABR server [" << Name() << "] closing " << mActiveConnCount.value() << " active connections");
          
          // Close the acceptor
          mAbrAcceptor->close();


          // Close all active connections
          BOOST_FOREACH(const abrConnHandlerMap_t::value_type& pair, mConnHandlerMap)
          {
              AbrServerConnectionHandler *connHandler(pair.first);

              // Remove the handler's close notification delegate (informing ourselves of a close that is self-initiated creates unnecessary overhead)
              connHandler->ClearCloseNotificationDelegate();
              
              // Close the connection
              connHandler->close();
          }
          
          mConnHandlerMap.clear();

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

      case IOLogicMessage::ACCEPT_NOTIFICATION:
      {
          // Insert connection handler into handler map
          mConnHandlerMap[msg.acceptedConnHandler.get()] = msg.acceptedConnHandler;
          break;
      }
      
      case IOLogicMessage::CLOSE_NOTIFICATION:
      {
          // Delete the connection handler associated with the client socket
          mConnHandlerMap.erase(msg.closedConnHandler);
          break;
      }
      
      default:
          throw EPHXInternal();
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AbrServer::HandleAcceptNotification(AbrServerConnectionHandler& rawConnHandler)
{
    // NOTE: Store the connection handler in a shared pointer with a custom deleter -- this will call remove_reference() when the shared copy is deleted
    abrConnHandlerSharedPtr_t connHandler(&rawConnHandler, boost::bind(&AbrServerConnectionHandler::remove_reference, _1));

    // Immediately close connection if we've already been stopped
    if (!mRunning)
        return false;

    // Enforce max simultaneous clients constraint
    if (mConfig.ProtocolConfig.MaxSimultaneousClients && mActiveConnCount >= mConfig.ProtocolConfig.MaxSimultaneousClients)
        return false;

    // Initialize connection handler
    connHandler->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &AbrServer::HandleCloseNotification));
    connHandler->SetVersion(static_cast<Http_1_port_server::HttpVersionOptions>(mConfig.ProtocolProfile.HttpVersion));
    connHandler->SetType(static_cast<Http_1_port_server::HttpServerType>(mConfig.ProtocolProfile.HttpServerTypes));
    connHandler->SetMaxRequests(mConfig.ProtocolConfig.MaxRequestsPerClient);
    connHandler->SetBodyType(static_cast<Http_1_port_server::BodyContentOptions>(mConfig.ResponseConfig.BodyContentType));
    connHandler->SetStatsInstance(mStats);
    connHandler->SetVideoStatsInstance(mVideoStats);

    //TCP_WINDOW_CLAMP is not supported by USS and socket recv buffer is used to
    //handle RxWindowSizeLimit.
    connHandler->SetSoRcvBuf(mConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit);

    // ABR Additions
    connHandler->SetAbrPlaylist(mPlaylist);

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

    // Notify the I/O logic that a connection was accepted
    {
        ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
        IOLogicMessage& msg(*msgPtr);
        msg.type = IOLogicMessage::ACCEPT_NOTIFICATION;
        msg.acceptedConnHandler = connHandler;
        mIOLogicInterface->Send(msgPtr);
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

void AbrServer::HandleCloseNotification(L4L7_APP_NS::StreamSocketHandler& socket)
{
    AbrServerConnectionHandler *connHandler = reinterpret_cast<AbrServerConnectionHandler *>(&socket);

    // Notify the I/O logic that a connection was closed
    {
        ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
        IOLogicMessage& msg(*msgPtr);
        msg.type = IOLogicMessage::CLOSE_NOTIFICATION;
        msg.closedConnHandler = connHandler;
        mIOLogicInterface->Send(msgPtr);
    }

    // Decrement the active connection count
    mActiveConnCount--;

    {
        ACE_GUARD(stats_t::lock_t, guard, mStats.Lock());
        mStats.activeConnections--;
    }
}

///////////////////////////////////////////////////////////////////////////////

