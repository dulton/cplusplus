/// @file
/// @brief FTP Server implementation
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
#include <app/StreamSocketConnector.tcc>
#include <base/BaseCommon.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <phxexception/PHXExceptionFactory.h>

#include "FtpdLog.h"
#include "FtpServer.h"
#include "FtpServerRawStats.h"
#include "ServerControlConnectionHandler.h"

USING_FTP_NS;


///////////////////////////////////////////////////////////////////////////////
const size_t FtpServer::PURGE_HWM = 128; // high water mark for purging control connections

///////////////////////////////////////////////////////////////////////////////

struct FtpServer::FtpIOLogicMessage
{
    enum Command
    {
        STOP_COMMAND,
        CLOSE_COMMAND,    
        NOOP_COMMAND,
    
        ACCEPT_NOTIFICATION,
        CLOSE_NOTIFICATION,
		
        CONTROL_CONNECTION_DATACLOSE_CB
    } ;

    /// the command type
    enum Command type ;

    // Parameter for ACCEPT
    controlHdlrSharedPtr_t connHandlerSp ;

    /// Parameter for CLOSE, MAKE_DATA and CLOSE_DATA
    ServerControlConnectionHandler *connHandlerRawPtr ;

    /// Parameter for CONTROL_CONNECTION_DATACLOSE_CB
    void *arg ;
};	  

///////////////////////////////////////////////////////////////////////////////

struct FtpServer::FtpAppLogicMessage
{
    enum Command
    {
        PURGE_NOTIFICATION
    } ;

    // the command type
    enum Command type ;
} ;

///////////////////////////////////////////////////////////////////////////////
FtpServer::FtpServer(uint16_t port, size_t blockIndex, const config_t& config, stats_t& stats, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier)
    : mPort(port),
      mBlockIndex(blockIndex),
      mConfig(config),
      mStats(stats),
      mAppReactor(appReactor),
      mIOReactor(ioReactor),
      mIOBarrier(ioBarrier),
      mRunning(false),
      mAcceptor(new controlAcceptor_t),
      mActiveConnCount(0),
      mIOLogicInterface(new ioMessageInterface_t(mIOReactor)),
      mAppLogicInterface(new appMessageInterface_t(mAppReactor)),
      mServerIfName("")
{
    mAcceptor->SetAcceptNotificationDelegate(
        fastdelegate::MakeDelegate(this, &FtpServer::HandleAcceptNotification));

    mIOLogicInterface->SetMessageDelegate(fastdelegate::MakeDelegate(this, &FtpServer::HandleIOLogicMessage));
    mAppLogicInterface->SetMessageDelegate(fastdelegate::MakeDelegate(this, &FtpServer::HandleAppLogicMessage));

    TC_LOG_DEBUG_LOCAL(mPort, LOG_SERVER, "FTP server [" << Name() << "] created");
}

///////////////////////////////////////////////////////////////////////////////

FtpServer::~FtpServer()
{
    Stop();
}

///////////////////////////////////////////////////////////////////////////////

bool FtpServer::Start(const std::string& ifName, const ACE_INET_Addr& addr)
{
    if (mRunning)
        throw EPHXInternal();


    char addrStr[64];
    if (addr.addr_to_string(addrStr, sizeof(addrStr)) == -1)
        strcpy(addrStr, "invalid");      

    // Open acceptor socket
    if (mAcceptor->open(addr, mIOReactor) == -1)
    {
        FTP_LOG_ERR(mPort, LOG_SERVER, "FTP server [" << Name() << "] failed to start on " << addrStr << ": " << strerror(errno));
        return false;
    }

    mRunning = true ;

    if (addr.get_type() == AF_INET)
        mAcceptor->SetIpv4Tos(mConfig.Common.Profile.L4L7Profile.Ipv4Tos);

    if (addr.get_type() == AF_INET6)
        mAcceptor->SetIpv6TrafficClass(mConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass);

    mAcceptor->SetTcpWindowSizeLimit(mConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit);
    mAcceptor->SetTcpDelayedAck(mConfig.Common.Profile.L4L7Profile.EnableDelayedAck);
    
    // Bind acceptor to given interface name
    if (!ifName.empty())
        mAcceptor->BindToIfName(ifName);
    
    mServerIfName = ifName ;

    FTP_LOG_INFO(mPort, LOG_SERVER, "FTP server [" << Name() << "] started on " << addrStr << " (" << ifName << ")");
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void FtpServer::Stop(void)
{
    if (!mRunning)
        return;

    mRunning = false ;

    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "FTP server [" << Name() << "] stopping");

    // Ask the I/O logic to stop all new connections
    {
        ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
        FtpIOLogicMessage& msg(*msgPtr);
        msg.type = FtpIOLogicMessage::STOP_COMMAND;
        mIOLogicInterface->Send(msgPtr);
    }
    
    // Wait for the stop message to be processed
    mIOLogicInterface->Wait();

    // Wait for all I/O threads to exit
    {
        ACE_WRITE_GUARD(ACE_Lock, guard, *mIOBarrier);
    }
    
    // Allow I/O logic to close any remaining connections
    {
        ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
        FtpIOLogicMessage& msg(*msgPtr);
        msg.type = FtpIOLogicMessage::CLOSE_COMMAND;
        mIOLogicInterface->Send(msgPtr);
    }

    // Wait for the CLOSE to be processed. 
    mIOLogicInterface->Wait();

    // Wait for all I/O threads to exit after CLOSE_COMMAND
    {
        ACE_WRITE_GUARD(ACE_Lock, guard, *mIOBarrier);
    }

    // Ensure IO logic message queue is empty
    {
        ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
        FtpIOLogicMessage& msg(*msgPtr);
        msg.type = FtpIOLogicMessage::NOOP_COMMAND;
        mIOLogicInterface->Send(msgPtr);
    }
  
    // Wait for app logic to finish processing.
    mIOLogicInterface->Wait() ;

    // Wait for all I/O threads to exit reactor after NOOP
    {
        ACE_WRITE_GUARD(ACE_Lock, guard, *mIOBarrier);
    }

    //Finally, clear connection handler list.
    {     
        ACE_WRITE_GUARD(ACE_Lock, guard, *mIOBarrier);

       // clear our pending purge list here 
        mPendingPurge.clear() ;
    }

    // Clear the queues.
    mIOLogicInterface->PurgePendingNotifications();
    if (!mIOLogicInterface->IsEmpty())
    {
        FTP_LOG_ERR(mPort, LOG_SERVER, "Server still has messages in IO queue") ;
    }

    // Wait for all I/O threads to exit reactor after Stopping the Message Queue
    {
        ACE_WRITE_GUARD(ACE_Lock, guard, *mIOBarrier);
    }

    mAppLogicInterface->Flush();
    mAppLogicInterface->PurgePendingNotifications();
    if (!mAppLogicInterface->IsEmpty())
    {
        FTP_LOG_ERR(mPort, LOG_SERVER, "Server still has messages in App queue") ;
    }
}

///////////////////////////////////////////////////////////////////////////////

void FtpServer::NotifyInterfaceDisabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "FTP server [" << Name() << "] disabling protocol");
    Stop();
}

///////////////////////////////////////////////////////////////////////////////

void FtpServer::NotifyInterfaceEnabled(void)
{
}

///////////////////////////////////////////////////////////////////////////////

void FtpServer::HandleIOLogicMessage(const FtpIOLogicMessage& msg)
{
    switch (msg.type)
    {
      case FtpIOLogicMessage::STOP_COMMAND:
      {
          FTP_LOG_INFO(mPort, LOG_CLIENT, "FTP server [" << Name() << "] stopping accepting incoming connections");

          // Suspend the acceptor, if any connection was created.
          mIOReactor->suspend_handler(mAcceptor->get_handle()) ;
	
          // Stop timers in all active connections
          BOOST_FOREACH(const controlHdlrMap_t::value_type& pair, mConnHandlerMap)
              pair.first->PurgeTimers();

          // Signal that the stop is complete
          mIOLogicInterface->Signal(); 
          break;
      }

      case FtpIOLogicMessage::CLOSE_COMMAND:
      {
          FTP_LOG_INFO(mPort, LOG_CLIENT, "FTP server [" << Name() << "] closing " << mActiveConnCount.value() << " active connections");
          
          // Close the acceptor.
          mAcceptor->close();     

          // Shutdown and close all active connections
          BOOST_FOREACH(const controlHdlrMap_t::value_type& pair, mConnHandlerMap)
          {
              ServerControlConnectionHandler *connHandler(pair.first);

              // Remove the handler's close notification delegate (informing ourselves of a close that is self-initiated creates unnecessary overhead)
              connHandler->ClearCloseNotificationDelegate();
              
              // Ask connection handler to abort immediately.
              connHandler->Abort() ;

              // Put the connection on the purge list.
              mPendingPurge.push_back(pair.second) ; 
          }
          // Clear out active connection handler map.
          mConnHandlerMap.clear() ;  
        
          {
              ACE_GUARD(stats_t::lock_t, guard, mStats.Lock());
              mStats.activeControlConnections -= mActiveConnCount.value();
          }

          // Zero out the active connection count
          mActiveConnCount = 0;   

          // Signal that the flush command was processed
          mIOLogicInterface->Signal();
          break;
      }

      case FtpIOLogicMessage::NOOP_COMMAND:
      {
		  // No-op
          mIOLogicInterface->Signal() ;
          break ;
      }
      
      case FtpIOLogicMessage::ACCEPT_NOTIFICATION:
      {
          // Insert connection handler into handler map
          mConnHandlerMap[msg.connHandlerSp.get()] = msg.connHandlerSp;
          // Increment the active connection count
          ++mActiveConnCount;   
            
          break;
      }
      
      case FtpIOLogicMessage::CLOSE_NOTIFICATION:
      {
          // Delete the connection handler associated with the client socket
          controlHdlrMap_t::iterator it = mConnHandlerMap.find(msg.connHandlerRawPtr);
          if (it != mConnHandlerMap.end())
          {
              // put the connection on the pending purge list.
              mPendingPurge.push_back(it->second) ;
              // erase from connection handler map
              mConnHandlerMap.erase(it) ;
              // Decrement the active connection count
              mActiveConnCount--;
              // Post Purge request
              PostPurgeRequest() ;
          }
          break;
      }

      case FtpIOLogicMessage::CONTROL_CONNECTION_DATACLOSE_CB:
      {
          controlHdlrMap_t::iterator mit = mConnHandlerMap.find(msg.connHandlerRawPtr) ;
          if (mit != mConnHandlerMap.end())
              mit->second->DataCloseCb(msg.arg) ;
          break ;
      }

      default:
          throw EPHXInternal();
    }
}

///////////////////////////////////////////////////////////////////////////////

void FtpServer::HandleAppLogicMessage(const FtpAppLogicMessage& msg)
{
    switch (msg.type)
    {
      case FtpAppLogicMessage::PURGE_NOTIFICATION:
      {
         // Wait for all I/O threads to exit
         // Once ensured, clear out pending purge map
         {
             ACE_WRITE_GUARD(ACE_Lock, guard, *mIOBarrier);
             mPendingPurge.clear() ;                 
         }
         break ;
      }

      default:
      {
          FTP_LOG_ERR(mPort, LOG_SERVER, "Handle AppLogic got invalid message type") ;
          throw EPHXInternal();
      }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool FtpServer::HandleAcceptNotification(ServerControlConnectionHandler& rawConnHandler)
{
    // NOTE: Store the connection handler in a shared pointer with a custom deleter -- this will call remove_reference() when the shared copy is deleted
    controlHdlrSharedPtr_t connHandler(&rawConnHandler, boost::bind(&ServerControlConnectionHandler::remove_reference, _1));
    
    if (!mRunning)
        return false ;

    // Enforce max simultaneous clients constraint
    if (mConfig.ProtocolConfig.MaxSimultaneousClients && mActiveConnCount >= mConfig.ProtocolConfig.MaxSimultaneousClients)
        return false;

    /////////////////// Initialize connection handler ///////////////////////////////////
    // initialize close callback
    connHandler->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &FtpServer::HandleControlCloseNotification));

    // initialte config
    connHandler->SetResponseConfig(mConfig.ResponseConfig) ;  
    connHandler->SetL4L7Profile(mConfig.Common.Profile.L4L7Profile) ;
    connHandler->SetMaxRequests(mConfig.ProtocolConfig.MaxRequestsPerClient);
    connHandler->SetServerPort(mPort) ;
    connHandler->SetServerName(Name()) ;

    // initialize callbacks...
    connHandler->SetServerIfNameGetter(fastdelegate::MakeDelegate(this, &FtpServer::GetServerIfName)) ;
    connHandler->SetDataCloseCb(fastdelegate::MakeDelegate(this, &FtpServer::SetupDataCloseCb)) ;

    // setup stats
    connHandler->SetStatsInstance(mStats) ;
    //TCP_WINDOW_CLAMP is not supported by USS and socket recv buffer is used to
    //handle RxWindowSizeLimit.
    connHandler->SetSoRcvBuf(mConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit);
    ///////////////////////////////////
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
    // Notify the I/O logic that a connection was accepted
    ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
    FtpIOLogicMessage& msg(*msgPtr);
    msg.type = FtpIOLogicMessage::ACCEPT_NOTIFICATION;
    msg.connHandlerSp = connHandler;
    mIOLogicInterface->Send(msgPtr);
 
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void FtpServer::HandleControlCloseNotification(L4L7_APP_NS::StreamSocketHandler& socket)
{
    if (!mRunning)
        return ;

    ServerControlConnectionHandler *connHandler = reinterpret_cast<ServerControlConnectionHandler *>(&socket);    

    // Notify the I/O logic that a connection was closed
    ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
    FtpIOLogicMessage& msg(*msgPtr);
    msg.type = FtpIOLogicMessage::CLOSE_NOTIFICATION;
    msg.connHandlerRawPtr = connHandler;
    mIOLogicInterface->Send(msgPtr);

    {
        ACE_GUARD(stats_t::lock_t, guard, mStats.Lock());
        mStats.activeControlConnections--;
    }
}

///////////////////////////////////////////////////////////////////////////////
void FtpServer::SetupDataCloseCb( FtpControlConnectionHandlerInterface *connHandler, DataTransactionManager *arg) 
{
    if (!mRunning)
        return ;

    ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
    FtpIOLogicMessage& msg(*msgPtr);
    msg.type = FtpIOLogicMessage::CONTROL_CONNECTION_DATACLOSE_CB;
    msg.connHandlerRawPtr = static_cast<ServerControlConnectionHandler *>(connHandler);
    assert(msg.connHandlerRawPtr) ;
    msg.arg = (void *) arg ;
    mIOLogicInterface->Send(msgPtr);
}

///////////////////////////////////////////////////////////////////////////////

void FtpServer::PostPurgeRequest(void)
{
    if (!mRunning)
        return ;

    if (mPendingPurge.size() >= PURGE_HWM)
    {
        // Post App logic a message to cleanup
        appMessageInterface_t::messagePtr_t msgPtr(mAppLogicInterface->Allocate());
        FtpAppLogicMessage& msg(*msgPtr);
        msg.type = FtpAppLogicMessage::PURGE_NOTIFICATION;
        mAppLogicInterface->Send(msgPtr);
     }
}

///////////////////////////////////////////////////////////////////////////////
