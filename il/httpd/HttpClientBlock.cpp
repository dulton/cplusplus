/// @file
/// @brief HTTP Client Block implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <vector>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <app/AsyncMessageInterface.tcc>
#include <app/StreamSocketConnector.tcc>
#include <app/StreamSocketHandler.h>
#include <base/EndpointPairEnumerator.h>
#include <base/LoadProfile.h>
#include <base/LoadScheduler.h>
#include <base/LoadStrategy.h>
#include <phxexception/PHXExceptionFactory.h>

#include "ClientConnectionHandler.h"
#include "HttpdLog.h"
#include "HttpClientBlock.h"
#include "HttpClientBlockLoadStrategies.h"

USING_HTTP_NS;

///////////////////////////////////////////////////////////////////////////////

struct HttpClientBlock::IOLogicMessage
{
    enum
    {
        STOP_COMMAND,
        CLOSE_COMMAND,
        INTENDED_LOAD_NOTIFICATION,
        AVAILABLE_LOAD_NOTIFICATION,
        SET_DYNAMIC_LOAD,
        ENABLE_DYNAMIC_LOAD,
        CLOSE_NOTIFICATION
    } type;

    // Intended and available load
    uint32_t intendedLoad;
    uint32_t availableLoad;

    // Bandwidth parameters
    bool enableDynamicLoad;
    int32_t dynamicLoad;
    
    // Close notification parameters
    uint32_t connHandlerSerial;
};

struct HttpClientBlock::AppLogicMessage
{
    enum
    {
        CLOSE_NOTIFICATION
    } type;
};

///////////////////////////////////////////////////////////////////////////////

HttpClientBlock::HttpClientBlock(uint16_t port, const config_t& config, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier)
    : ClientBlock(config),
       mPort(port),
//      mConfig(config),
      mStats(BllHandle()),
      mVideoStats(BllHandle()),
      mAppReactor(appReactor),
      mIOReactor(ioReactor),
      mIOBarrier(ioBarrier),
      mEnabled(true),
      mRunning(false),  
      mLoadProfile(new loadProfile_t(mConfig.Common.Load)),
      mLoadStrategy(MakeLoadStrategy()),
      mLoadScheduler(new loadScheduler_t(*mLoadProfile, *mLoadStrategy)),
      mAttemptedConnCount(0),
      mActiveConnCount(0),
      mPendingConnCount(0),
      mEndpointEnum(new endpointEnum_t(port, IfHandle(), mConfig.Common.Endpoint.DstIf)),
      mIOLogicInterface(new ioMessageInterface_t(mIOReactor)),
      mAppLogicInterface(new appMessageInterface_t(mAppReactor)),
      mAvailableLoadMsgsOut(0),
      mStoringStopNotification(false)
{
    mLoadProfile->SetActiveStateChangeDelegate(boost::bind(&HttpClientBlock::LoadProfileHandler, this, _1));
    
    mEndpointEnum->SetPattern(static_cast<endpointEnum_t::pattern_t>(mConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern));
    mEndpointEnum->SetSrcPortNum(0);
    mEndpointEnum->SetDstPortNum(mConfig.ServerPortInfo);

    mIOLogicInterface->SetMessageDelegate(fastdelegate::MakeDelegate(this, &HttpClientBlock::HandleIOLogicMessage));
    mAppLogicInterface->SetMessageDelegate(fastdelegate::MakeDelegate(this, &HttpClientBlock::HandleAppLogicMessage));

    SetDynamicLoad(mConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.DynamicLoad);
    EnableDynamicLoad(mConfig.Common.Load.LoadProfile.UseDynamicLoad);

    InitDebugInfo("HTTP", BllHandle());

    TC_LOG_DEBUG_LOCAL(mPort, LOG_CLIENT, "HTTP client block [" << Name() << "] created");
}

///////////////////////////////////////////////////////////////////////////////

HttpClientBlock::~HttpClientBlock()
{
    PreStop();
	Stop();
}

///////////////////////////////////////////////////////////////////////////////

HttpClientBlock::LoadStrategy *HttpClientBlock::MakeLoadStrategy(void)
{
    SetEnableBwCtrl(false);

    const L4L7_BASE_NS::LoadTypes loadType = static_cast<const L4L7_BASE_NS::LoadTypes>(mConfig.Common.Load.LoadProfile.LoadType);
    switch (loadType)
    {
      case L4L7_BASE_NS::LoadTypes_CONNECTIONS:
          return new ConnectionsLoadStrategy(*this);
          
      case L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT:
          return new ConnectionsOverTimeLoadStrategy(*this);
          
      case L4L7_BASE_NS::LoadTypes_TRANSACTIONS:
          return new TransactionsLoadStrategy(*this);
          
      case L4L7_BASE_NS::LoadTypes_TRANSACTIONS_PER_TIME_UNIT:
          return new TransactionsOverTimeLoadStrategy(*this);
          
      case L4L7_BASE_NS::LoadTypes_BANDWIDTH:
          SetEnableBwCtrl(true);
          return new BandwidthLoadStrategy(*this);         

      default:
          TC_LOG_ERR_LOCAL(mPort, LOG_CLIENT, "Cannot create load strategy for unknown load type (" << tms_enum_to_string(loadType, L4L7_BASE_NS::em_LoadTypes) << ")");
          throw EPHXBadConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////

void HttpClientBlock::Start(void)
{
    if (!mEnabled || mRunning)
        return;

    mRunning = true;

    TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "HTTP client block [" << Name() << "] starting");

    // Reset our internal load state
    mAttemptedConnCount = 0;
    mPendingConnCount = 0;
    mEndpointEnum->Reset();
    
    // Allocate and initialize a new connector for the I/O logic to use
    mConnector.reset(new connector_t(mIOReactor));
    mConnector->SetIpv4Tos(mConfig.Common.Profile.L4L7Profile.Ipv4Tos);
    mConnector->SetIpv6TrafficClass(mConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass);
    mConnector->SetTcpWindowSizeLimit(mConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit);
    mConnector->SetTcpDelayedAck(mConfig.Common.Profile.L4L7Profile.EnableDelayedAck);
    mConnector->SetConnectNotificationDelegate(fastdelegate::MakeDelegate(this, &HttpClientBlock::HandleConnectionNotification));
    mConnector->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &HttpClientBlock::HandleCloseNotification));
    
    // Start load scheduler
    mLoadScheduler->Start(mAppReactor);
    InitBwTimeStorage();
	
	// Set stats Reactor instance
	mStats.SetReactorInstance(mAppReactor);
}

///////////////////////////////////////////////////////////////////////////////


void HttpClientBlock::PreStop(void)
{
    if (!mEnabled || !mRunning)
        return;
 
    // Stop load scheduler
    mLoadScheduler->Stop();
    mRunning = false;

    BOOST_FOREACH(const connHandlerMap_t::value_type& pair,mConnHandlerMap)
    {  
        pair.second->SetStopFlag(true);
    }

}

void HttpClientBlock::Stop(void)
{
    if (!mEnabled)
        return;

    // Reset the intended load
    {
        ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
        mStats.intendedLoad = 0;
    }

    // Ask the I/O logic to stop initiating new connections
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

    // Stop load profile last (triggers notification if not already at end)
    mLoadProfile->Stop();

	// Stop stats timer
	mStats.CancelTimer();
}

///////////////////////////////////////////////////////////////////////////////

void HttpClientBlock::SetDynamicLoad(int32_t dynamicLoad)
{
    // Ask the I/O logic to set the block load
    ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
    IOLogicMessage& msg(*msgPtr);
    msg.type = IOLogicMessage::SET_DYNAMIC_LOAD;
    msg.dynamicLoad = dynamicLoad;
    mIOLogicInterface->Send(msgPtr);
}

///////////////////////////////////////////////////////////////////////////////

void HttpClientBlock::EnableDynamicLoad(bool enable)
{
    // Ask the I/O logic to enable block load
    ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
    IOLogicMessage& msg(*msgPtr);
    msg.type = IOLogicMessage::ENABLE_DYNAMIC_LOAD;
    msg.enableDynamicLoad = enable;
    mIOLogicInterface->Send(msgPtr);
}

///////////////////////////////////////////////////////////////////////////////

void HttpClientBlock::NotifyInterfaceDisabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "[HttpClientBlock::NotifyInterfaceDisabled] HTTP client block [" << Name() << "] notified interface is disabled");
    PreStop();
    Stop();
    mEnabled = false;
}

///////////////////////////////////////////////////////////////////////////////

void HttpClientBlock::NotifyInterfaceEnabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "[HttpClientBlock::NotifyInterfaceEnabled] HTTP client block [" << Name() << "] notified interface is enabled");
    mEnabled = true;
}

///////////////////////////////////////////////////////////////////////////////

void HttpClientBlock::SetIntendedLoad(uint32_t intendedLoad)
{
    // Ask the I/O logic to set the intended load
    ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
    IOLogicMessage& msg(*msgPtr);
    msg.type = IOLogicMessage::INTENDED_LOAD_NOTIFICATION;
    msg.intendedLoad = intendedLoad;
    mIOLogicInterface->Send(msgPtr);
}

///////////////////////////////////////////////////////////////////////////////

void HttpClientBlock::SetAvailableLoad(uint32_t availableLoad, uint32_t intendedLoad)
{
    const uint32_t MAX_AVAILABLE_OUT = 2;

    // don't accumulate too many messages
    if (mAvailableLoadMsgsOut >= MAX_AVAILABLE_OUT)
        return;

    ++mAvailableLoadMsgsOut;

    // Ask the I/O logic to set the intended and available load
    ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
    IOLogicMessage& msg(*msgPtr);
    msg.type = IOLogicMessage::AVAILABLE_LOAD_NOTIFICATION;
    msg.availableLoad = availableLoad;
    msg.intendedLoad = intendedLoad;
    mIOLogicInterface->Send(msgPtr);
}

///////////////////////////////////////////////////////////////////////////////

void HttpClientBlock::HandleIOLogicMessage(const IOLogicMessage& msg)
{
   
    switch (msg.type)
    {
      case IOLogicMessage::STOP_COMMAND:
      {
          TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "HTTP client block [" << Name() << "] stopping initiating new connections");

          if (mConnector)
          {
              ACE_GUARD(connLock_t, guard, mConnLock);
              
              // Close connector
              mConnector->close();
          }

          // Stop timers in all active connections
          BOOST_FOREACH(const connHandlerMap_t::value_type& pair, mConnHandlerMap)
              pair.second->PurgeTimers();
          
          // Signal that the stop is complete
          mIOLogicInterface->Signal();
          break;
      }

      case IOLogicMessage::CLOSE_COMMAND:
      {
          TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "HTTP client block [" << Name() << "] closing " << mActiveConnCount << " active connections");

          if (mConnector)
          {
              ACE_GUARD(connLock_t, guard, mConnLock);
              
              // Destroy the connector
              mConnector.reset();
          }
          
          // Close all active connections
          ReapConnections(mActiveConnCount);

          // Signal that the close command was processed
          mIOLogicInterface->Signal();
          break;
      }

      case IOLogicMessage::SET_DYNAMIC_LOAD:  // Limited to 1 Connection
      {
          DynamicLoadHandler(mActiveConnCount, msg.dynamicLoad);
          break;
      }      

      case IOLogicMessage::ENABLE_DYNAMIC_LOAD:
      { 
          SetEnableDynamicLoad(msg.enableDynamicLoad);
          mLoadScheduler->SetEnableDynamicLoad(msg.enableDynamicLoad);
          break;
      }

      case IOLogicMessage::INTENDED_LOAD_NOTIFICATION:
      {
          const ssize_t delta = msg.intendedLoad - mActiveConnCount;

          if (delta > 0 && mConnector && mRunning)
              SpawnConnections(static_cast<size_t>(delta));
          else if (delta < 0)
              ReapConnections(static_cast<size_t>(-delta));
          break;
      }

      case IOLogicMessage::AVAILABLE_LOAD_NOTIFICATION:
      {
          if (msg.availableLoad && mConnector && mRunning)
              SpawnConnections(msg.availableLoad);

          --mAvailableLoadMsgsOut;
          break;
      }
      
      case IOLogicMessage::CLOSE_NOTIFICATION:
      {
          if (mConnHandlerMap.find(msg.connHandlerSerial) == mConnHandlerMap.end())
              mPendingConnCount--;
          // Delete the connection handler from our map and aging queue
          if (mConnHandlerMap.erase(msg.connHandlerSerial))
          {
              mActiveConnCount--;
              ForwardStopNotification();
              ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
              mStats.activeConnections = mActiveConnCount;
          }

          mConnHandlerAgingQueue.erase(msg.connHandlerSerial);
          break;
      }

      default:
          throw EPHXInternal();
    }
}

///////////////////////////////////////////////////////////////////////////////

void HttpClientBlock::SpawnConnections(size_t count)
{
    // New connections may be limited by the L4-L7 load profile
    const uint32_t maxConnAttempted = mConfig.Common.Load.LoadProfile.MaxConnectionsAttempted;
    if (maxConnAttempted)
        count = std::min(maxConnAttempted - mAttemptedConnCount, count);
        
    const uint32_t maxOpenConn = mConfig.Common.Load.LoadProfile.MaxOpenConnections;
    if (maxOpenConn)
        count = std::min(maxOpenConn - mActiveConnCount, count);

    count = (count > mPendingConnCount) ? (count - mPendingConnCount) : 0;
    size_t countDecr = 1;

    for (; count != 0; count -= countDecr)
    {
        // Get the next endpoint pair in our block
        std::string srcIfName;
        ACE_INET_Addr srcAddr;
        ACE_INET_Addr dstAddr;
              
        mEndpointEnum->GetEndpointPair(srcIfName, srcAddr, dstAddr);
        mEndpointEnum->Next();
              
        // Temporary block scope for connector access
        ClientConnectionHandler *rawConnHandler = 0;
        int err;

        {
            ACE_GUARD(connLock_t, guard, mConnLock);

            // Instruct the connector to bind the next connection to our source interface
            mConnector->BindToIfName(&srcIfName);

            // Avoid the string operations if the log level is < INFO
            if ((log4cplus_table[mPort].LevelMask & PHX_LOG_INFO_FLAG) == PHX_LOG_INFO_FLAG)
            {
                char srcAddrStr[64], dstAddrStr[64];

                if (srcAddr.addr_to_string(srcAddrStr, sizeof(srcAddrStr)) == -1)
                    strcpy(srcAddrStr, "invalid");

                if (dstAddr.addr_to_string(dstAddrStr, sizeof(dstAddrStr)) == -1)
                    strcpy(dstAddrStr, "invalid");
    
                TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "HTTP client block [" << Name() << "] connecting " << srcAddrStr << "->" << dstAddrStr << " using " << srcIfName);
            }

            err = mConnector->MakeHandler(rawConnHandler);
            if (rawConnHandler)
            {
                rawConnHandler->SetMaxRequests(GetMaxTransactions(count, countDecr));

                // Recalculate the load over all the connections, including the new one
                DynamicLoadHandler(mActiveConnCount + 1, GetDynamicLoadTotal());
                RegisterDynamicLoadDelegates(rawConnHandler);
                rawConnHandler->EnableBwCtrl(GetEnableBwCtrl());
            }
                  
            // Initiate a new connection         
            err = mConnector->connect(rawConnHandler, dstAddr, ACE_Synch_Options(ACE_Synch_Options::USE_REACTOR), srcAddr, 1);
        }

        // Bump attempted connections counter
        mAttemptedConnCount++;  
        {
            ACE_GUARD(stats_t::lock_t, guard, mStats.Lock());
            mStats.attemptedConnections++;
        }
        
        // If the connection attempt failed right away then we must treat this as though the connection was closed
        if (err == -1 && errno != EWOULDBLOCK)
        {
            if (rawConnHandler)
                rawConnHandler->remove_reference();
                      
            // Avoid the string operations if the log level is < INFO
            if ((log4cplus_table[mPort].LevelMask & PHX_LOG_INFO_FLAG) == PHX_LOG_INFO_FLAG)
            {
                char srcAddrStr[64], dstAddrStr[64], errorStr[64];

                if (srcAddr.addr_to_string(srcAddrStr, sizeof(srcAddrStr)) == -1)
                    strcpy(srcAddrStr, "invalid");

                if (dstAddr.addr_to_string(dstAddrStr, sizeof(dstAddrStr)) == -1)
                    strcpy(dstAddrStr, "invalid");

                TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "HTTP client block [" << Name() << "] failed to connect " << srcAddrStr << "->" << dstAddrStr << ": " << strerror_r(errno, errorStr, sizeof(errorStr)));
            }

            mPendingConnCount++;

            countDecr = 1;
        }
        else
        {
            mActiveConnCount++;

            {
                ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
                mStats.activeConnections = mActiveConnCount;
            }

            // Insert the connection handler into our map and aging queue
            // NOTE: Store the connection handler in a shared pointer with a custom deleter -- this will call remove_reference() when the map entry is erased
            connHandlerSharedPtr_t connHandler(rawConnHandler, boost::bind(&ClientConnectionHandler::remove_reference, _1));
            mConnHandlerMap[rawConnHandler->Serial()] = connHandler;
            mConnHandlerAgingQueue.push(rawConnHandler->Serial());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void HttpClientBlock::ReapConnections(size_t count)
{
    count = std::min(mActiveConnCount, count);
    
    // Close up to the specified number of connections, oldest first
    size_t closed = 0;
    while (!mConnHandlerAgingQueue.empty() && closed < count)
    {
        const uint32_t connHandlerSerial = mConnHandlerAgingQueue.front();
        mConnHandlerAgingQueue.pop();

        connHandlerMap_t::iterator iter = mConnHandlerMap.find(connHandlerSerial);
        if (iter == mConnHandlerMap.end())
            continue;

        connHandlerSharedPtr_t& connHandler(iter->second);

        // Remove the handler's close notification delegate (informing ourselves of a close that is self-initiated creates unnecessary overhead)
        connHandler->ClearCloseNotificationDelegate();
              
        // If this connection is pending, cancel it in the connector
        if (connHandler->IsPending())
        {
            if (mConnector)
            {
                ACE_GUARD(connLock_t, guard, mConnLock);
                mConnector->cancel(connHandler.get());
            }

            // bump aborted connections counter if it's really not connected
            if (!connHandler->IsConnected())
            {
                ACE_GUARD(stats_t::lock_t, guard, mStats.Lock());
                if (!connHandler->IsComplete())
                {
                    // didn't already flag this as [un]successful
                    mStats.abortedConnections++;
                    connHandler->SetComplete(true);
                }
            }
            else
            {
                ACE_GUARD(stats_t::lock_t, guard, mStats.Lock());
                if (!connHandler->IsComplete())
                {
                    // didn't already flag this as [un]successful
                    mStats.successfulConnections++;
                    connHandler->SetComplete(true);
                }
            }            
        }

        // Close the connection
        UnregisterDynamicLoadDelegates(connHandler.get());
        connHandler->close();
        mActiveConnCount--;

        // Delete the connection handler from our map
        mConnHandlerMap.erase(iter);
        closed++;

        {
            ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
            mStats.activeConnections = mActiveConnCount;
        }
    }

    ForwardStopNotification();
}

///////////////////////////////////////////////////////////////////////////////

void HttpClientBlock::DynamicLoadHandler(size_t conn, int32_t dynamicLoad)
{
    int32_t load = ConfigDynamicLoad(conn, dynamicLoad);
    if (load < 0)
        return;

    if (load == 0)
        ReapConnections(mActiveConnCount);

    ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
    mStats.intendedLoad = static_cast<uint32_t>(load);
}

///////////////////////////////////////////////////////////////////////////////

void HttpClientBlock::HandleAppLogicMessage(const AppLogicMessage& msg)
{
    switch (msg.type)
    {
      case AppLogicMessage::CLOSE_NOTIFICATION:
      {
          // Notify the load strategy that a connection was closed
          if (mLoadScheduler->Running())
              mLoadStrategy->ConnectionClosed();

          break;
      }

      default:
          throw EPHXInternal();
    }
}

///////////////////////////////////////////////////////////////////////////////

size_t HttpClientBlock::GetMaxTransactions(size_t count, size_t& countDecr)
{
    const L4L7_BASE_NS::LoadTypes loadType = static_cast<const L4L7_BASE_NS::LoadTypes>(mConfig.Common.Load.LoadProfile.LoadType);

    if (loadType == L4L7_BASE_NS::LoadTypes_TRANSACTIONS)
    {
        // If our load type is transactions, we limit each client connection to one transaction
        return 1;
    }
    else if (loadType == L4L7_BASE_NS::LoadTypes_TRANSACTIONS_PER_TIME_UNIT)
    {
        // For transactions/second, we use as many as we can per connection, subtracting transactions from the count
        return countDecr = std::min(count, 
            mConfig.ProtocolProfile.EnableKeepAlive ? 
            mConfig.ProtocolConfig.MaxTransactionsPerServer : 1);
    }
    else
    {
        return mConfig.ProtocolConfig.MaxTransactionsPerServer;
    }
}

///////////////////////////////////////////////////////////////////////////////

void HttpClientBlock::LoadProfileHandler(bool running)
{
    if (running || !mActiveConnCount)
    {
        // Cascade notification
        if (!mLoadProfileNotifier.empty())
            mLoadProfileNotifier(BllHandle(), running);
    }
    else 
    {
        // Delay stopped notification until active connections == 0
        mStoringStopNotification = true;
    }
}

///////////////////////////////////////////////////////////////////////////////

void HttpClientBlock::ForwardStopNotification() 
{ 
    if (mStoringStopNotification && !mActiveConnCount) 
    { 
        mStoringStopNotification = false;
        if (!mLoadProfileNotifier.empty())
            mLoadProfileNotifier(BllHandle(), false);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool HttpClientBlock::HandleConnectionNotification(ClientConnectionHandler& connHandler)
{
    // Since we're likely to be sinking a lot of data from the server, increase the input block size up to SO_RCVBUF
    size_t soRcvBuf;
    if (!connHandler.GetSoRcvBuf(soRcvBuf))
        return false;

    connHandler.SetInputBlockSize(soRcvBuf);
    
    // Initialize the protocol-related aspects of the connection handler
    connHandler.SetVersion(static_cast<Http_1_port_server::HttpVersionOptions>(mConfig.ProtocolProfile.HttpVersion));
    connHandler.SetKeepAlive(mConfig.ProtocolProfile.EnableKeepAlive);
    connHandler.SetUserAgent(mConfig.ProtocolProfile.UserAgentHeader);
    connHandler.SetMaxPipelineDepth(mConfig.ProtocolProfile.EnablePipelining ? mConfig.ProtocolProfile.MaxPipelineDepth : 1);
    connHandler.SetStatsInstance(mStats);

    // Bump successful connections counter
    {
        ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats.Lock(), false);
        if (!connHandler.IsComplete())
        {
            mStats.successfulConnections++;
            connHandler.SetComplete(true);
        }
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#if 0
static void DumpTcpInfo(L4L7_APP_NS::StreamSocketHandler& socketHandler)
{
    struct tcp_info info = {};
    if (socketHandler.GetTcpInfo(info))
    {
        printf("---Info:---\n");
        printf("state: %d\n", info.tcpi_state);
        printf("ca_state: %d\n", info.tcpi_ca_state);
        printf("retransmits: %d\n", info.tcpi_retransmits);
        printf("probes: %d\n", info.tcpi_probes);
        printf("tcpi_backoff: %d\n", info.tcpi_backoff);
        printf("tcpi_options: %d\n", info.tcpi_options);
        printf("tcpi_snd_wscale: %d\n", info.tcpi_snd_wscale);
        printf("tcpi_rcv_wscale: %d\n", info.tcpi_rcv_wscale);

        printf("tcpi_rto: %d\n", info.tcpi_rto);
        printf("tcpi_ato: %d\n", info.tcpi_ato);
        printf("tcpi_snd_mss: %d\n", info.tcpi_snd_mss);
        printf("tcpi_rcv_mss: %d\n", info.tcpi_rcv_mss);

        printf("tcpi_unacked: %d\n", info.tcpi_unacked);
        printf("tcpi_sacked: %d\n", info.tcpi_sacked);
        printf("tcpi_lost: %d\n", info.tcpi_lost);
        printf("tcpi_retrans: %d\n", info.tcpi_retrans);
        printf("tcpi_fackets: %d\n", info.tcpi_fackets);

        printf("tcpi_last_data_sent: %d\n", info.tcpi_last_data_sent);
        printf("tcpi_last_ack_sent: %d\n", info.tcpi_last_ack_sent);
        printf("tcpi_last_data_recv: %d\n", info.tcpi_last_data_recv);
        printf("tcpi_last_ack_recv: %d\n", info.tcpi_last_ack_recv);

        printf("tcpi_pmtu: %d\n", info.tcpi_pmtu);
        printf("tcpi_rcv_ssthresh: %d\n", info.tcpi_rcv_ssthresh);
        printf("tcpi_rtt: %d\n", info.tcpi_rtt);
        printf("tcpi_rttvar: %d\n", info.tcpi_rttvar);
        printf("tcpi_snd_ssthresh: %d\n", info.tcpi_snd_ssthresh);
        printf("tcpi_snd_cwnd: %d\n", info.tcpi_snd_cwnd);
        printf("tcpi_advmss: %d\n", info.tcpi_advmss);
        printf("tcpi_reordering: %d\n", info.tcpi_reordering);
        printf("\n");
    }
    else
    {
        perror("get info failed\n");
    }
}
#endif

void HttpClientBlock::HandleCloseNotification(L4L7_APP_NS::StreamSocketHandler& socketHandler)
{
    ClientConnectionHandler *connHandler = reinterpret_cast<ClientConnectionHandler *>(&socketHandler);

    // If this connection is pending, cancel it in the connector
    if (connHandler->IsPending())
    {
        ACE_GUARD(connLock_t, guard, mConnLock);
        if (mConnector)
            mConnector->cancel(connHandler);
    }

    // DumpTcpInfo(socketHandler);

    // We don't use an "else if" here because that misses the case
    // where a connection immediately fails (e.g. no server) 

    if (!connHandler->IsOpen())
    {
        if (connHandler->IsConnected())
        {
            // Wasn't "open" but it was connected. Mark it successful here
            ACE_GUARD(stats_t::lock_t, guard, mStats.Lock());
            if (!connHandler->IsComplete())
            {
                mStats.successfulConnections++;
                connHandler->SetComplete(true);
            }
        }
        else
        {
            // Not open at all, bump unsuccessful connections counter
            ACE_GUARD(stats_t::lock_t, guard, mStats.Lock());
            if (!connHandler->IsComplete())
            {
                mStats.unsuccessfulConnections++;
                connHandler->SetComplete(true);
            }
        }
    }

    // Notify the I/O logic that a connection was closed
    {
        ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
        IOLogicMessage& msg(*msgPtr);
        msg.type = IOLogicMessage::CLOSE_NOTIFICATION;
        msg.connHandlerSerial = connHandler->Serial();
        mIOLogicInterface->Send(msgPtr);
    }

    // Notify the application logic that a connection was closed
    {
        appMessageInterface_t::messagePtr_t msgPtr(mAppLogicInterface->Allocate());
        AppLogicMessage& msg(*msgPtr);
        msg.type = AppLogicMessage::CLOSE_NOTIFICATION;
        mAppLogicInterface->Send(msgPtr);
    }
}

///////////////////////////////////////////////////////////////////////////////