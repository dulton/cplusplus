/// @file
/// @brief Dpg socket manager implementation
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <boost/foreach.hpp>

#include <app/CodgramSocketConnector.tcc>
#include <app/StreamSocketConnector.tcc>
#include <utils/MessageUtils.h>
#include "CodgramBlockAcceptor.h"
#include "StreamBlockAcceptor.h"
#include "DpgStreamConnectionHandler.h"
#include "DpgCodgramConnectionHandler.h"
#include "DpgPodgramConnectionHandler.h"
#include "DpgRawStats.h"
#include "TcpPortUtils.h"

#include "DpgdLog.h"
#include "SocketManager.h"
#include <engine/PlaylistInstance.h>

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

SocketManager::SocketManager(ACE_Reactor * ioReactor) : 
  mIpv4Tos(0),
  mIpv6TrafficClass(0),
  mTcpWindowSizeLimit(0),
  mTcpDelayedAck(true),
  mTcpNoDelay(false),
  mStats(0),
  mStopped(false),
  mStreamAcceptor(new streamAcceptor_t(ioReactor)),
  mCodgramAcceptor(new codgramAcceptor_t(ioReactor)),
  mStreamConnector(new streamConnector_t(ioReactor)),
  mCodgramConnector(new codgramConnector_t(ioReactor))
{
    mStreamConnector->SetConnectNotificationDelegate(fastdelegate::MakeDelegate(this, &SocketManager::HandleStreamConnectionNotification));
    mStreamConnector->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &SocketManager::HandleStreamCloseNotification));
    mCodgramConnector->SetConnectNotificationDelegate(fastdelegate::MakeDelegate(this, &SocketManager::HandleCodgramConnectionNotification));
    mCodgramConnector->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &SocketManager::HandleCodgramCloseNotification));
    mStreamAcceptor->SetAcceptNotificationDelegate(fastdelegate::MakeDelegate(this, &SocketManager::HandleStreamAcceptNotification));
    mCodgramAcceptor->SetAcceptNotificationDelegate(fastdelegate::MakeDelegate(this, &SocketManager::HandlePodgramAcceptNotification));
}

///////////////////////////////////////////////////////////////////////////////

SocketManager::~SocketManager()
{
    Stop();

    connHandlerMap_t::delegate_t forceClose(boost::bind(&DpgConnectionHandler::ForceClose, _1, TX_FIN));
    mConnHandlerMap.foreach(forceClose);

    Reset();
}

///////////////////////////////////////////////////////////////////////////////

bool SocketManager::Accept(playlistInstance_t * pi, uint32_t serial)
{
    if(mStopped)
       return false;
    // Pass an incoming connection on to the proper acceptor/delegate
    connHandlerSharedPtr_t connHandler = mConnHandlerMap.find(uint32_t(serial));
    if (connHandler.get() == 0)
    {
        // connection not found 
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "accept attempted on unknown connection: " << uint32_t(serial));
        return false;
    }

    playAddrMap_t::iterator addrIter = mPlayAddrMap.find(pi);
    if (addrIter == mPlayAddrMap.end())
    {
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "accept attempted on unknown playlist instance");
        return false;
    }

    DpgStreamConnectionHandler* streamConHandler = dynamic_cast<DpgStreamConnectionHandler *>(connHandler.get());
    if (streamConHandler)
    {
        return mStreamAcceptor->Accept(*streamConHandler, addrIter->second);
    }

    DpgPodgramConnectionHandler* podgramConHandler = dynamic_cast<DpgPodgramConnectionHandler *>(connHandler.get());
    if (podgramConHandler)
    {
        return mCodgramAcceptor->Accept(*podgramConHandler, addrIter->second);
    }

    TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "accept attempted on unknown stream type");
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void SocketManager::Stop()
{
    mStopped = true;
    if (mStreamConnector)
    {
        ACE_GUARD(connLock_t, guard, mConnLock);
        mStreamConnector->close();
    }

    if (mCodgramConnector)
    {
        ACE_GUARD(connLock_t, guard, mConnLock);
        mCodgramConnector->close();
    }

    if (mStreamAcceptor)
    {
        mStreamAcceptor->StopAll();
        mStreamAcceptor->CloseAll();
    }

    if (mCodgramAcceptor)
    {
        mCodgramAcceptor->StopAll();
        mCodgramAcceptor->CloseAll();
    }

    connHandlerMap_t::delegate_t purgeTimers(boost::bind(&DpgConnectionHandler::PurgeTimers, _1));
    mConnHandlerMap.foreach(purgeTimers);
}

///////////////////////////////////////////////////////////////////////////////

void SocketManager::Reset()
{
    mStopped = false;
    if (mStreamConnector)
    {
        ACE_GUARD(connLock_t, guard, mConnLock);
        mStreamConnector.reset();
    }

    if (mCodgramConnector)
    {
        ACE_GUARD(connLock_t, guard, mConnLock);
        mCodgramConnector.reset();
    }

    if (mStreamAcceptor)
    {
        mStreamAcceptor->CloseAll();
        mStreamAcceptor.reset();
    }

    if (mCodgramAcceptor)
    {
        mCodgramAcceptor->CloseAll();
        mCodgramAcceptor.reset();
    }
}

///////////////////////////////////////////////////////////////////////////////

void SocketManager::SetIpv4Tos(uint8_t tos) 
{
    mStreamAcceptor->SetIpv4Tos(tos);
    mCodgramAcceptor->SetIpv4Tos(tos);
    mStreamConnector->SetIpv4Tos(tos);
    mCodgramConnector->SetIpv4Tos(tos);
    mIpv4Tos = tos; 
}

///////////////////////////////////////////////////////////////////////////////

void SocketManager::SetIpv6TrafficClass(uint8_t trafficClass) 
{  
    mStreamAcceptor->SetIpv6TrafficClass(trafficClass);
    mCodgramAcceptor->SetIpv6TrafficClass(trafficClass);
    mStreamConnector->SetIpv6TrafficClass(trafficClass);
    mCodgramConnector->SetIpv6TrafficClass(trafficClass);
    mIpv6TrafficClass = trafficClass; 
}

///////////////////////////////////////////////////////////////////////////////

void SocketManager::SetTcpWindowSizeLimit(uint32_t tcpWindowSizeLimit) 
{   
    mStreamAcceptor->SetTcpWindowSizeLimit(tcpWindowSizeLimit);
    mStreamConnector->SetTcpWindowSizeLimit(tcpWindowSizeLimit);
    mTcpWindowSizeLimit = tcpWindowSizeLimit; 
}

///////////////////////////////////////////////////////////////////////////////

void SocketManager::SetTcpDelayedAck(bool delayedAck) 
{   
    mStreamAcceptor->SetTcpDelayedAck(delayedAck);
    mStreamConnector->SetTcpDelayedAck(delayedAck);
    mTcpDelayedAck = delayedAck; 
}

///////////////////////////////////////////////////////////////////////////////

void SocketManager::SetTcpNoDelay(bool noDelay)
{   
    mStreamAcceptor->SetTcpNoDelay(noDelay);
    mStreamConnector->SetTcpNoDelay(noDelay);
    mTcpNoDelay = noDelay;
}

///////////////////////////////////////////////////////////////////////////////

void SocketManager::SetStatsInstance(stats_t& stats)
{
    mStats = &stats;
}

///////////////////////////////////////////////////////////////////////////////

void SocketManager::AddPlaylist(playlistInstance_t * pi, const std::string& srcIfName, const ACE_INET_Addr& srcAddr, const ACE_INET_Addr& dstAddr)
{
    AddrInfo& info(mPlayAddrMap[pi]);
    info.locIfName = srcIfName;
    info.locAddr   = srcAddr;
    info.remAddr   = dstAddr;
    info.hasRem    = true;
}

///////////////////////////////////////////////////////////////////////////////

void SocketManager::AddChildPlaylist(playlistInstance_t * pi, playlistInstance_t * parentPi, const ACE_INET_Addr& remoteAddr)
{
    AddrInfo& info(mPlayAddrMap[pi]);
    const AddrInfo& parentInfo(mPlayAddrMap[parentPi]);
    info.locIfName = parentInfo.locIfName;
    info.locAddr   = parentInfo.locAddr;
    info.remAddr   = remoteAddr;
    info.hasRem    = true;
}

///////////////////////////////////////////////////////////

void SocketManager::AddListeningPlaylist(playlistInstance_t * pi, const std::string& srcIfName, const ACE_INET_Addr& srcAddr, const addrSet_t& remAddrSet, const spawnAcceptDelegate_t& spawnDelegate)
{
    AddrInfo& info(mPlayAddrMap[pi]);
    info.locIfName = srcIfName;
    info.locAddr   = srcAddr;
    info.hasRem = false; // dest addr is unknown until a connection is made

    spawnInfo_t& spawnInfo(mPlaySpawnMap[pi]);
    spawnInfo.delegate = spawnDelegate;
    spawnInfo.addrSet = remAddrSet;
}

///////////////////////////////////////////////////////////////////////////////

void SocketManager::RemovePlaylist(playlistInstance_t * pi)
{
    playAddrMap_t::iterator iter = mPlayAddrMap.find(pi);
    if (iter != mPlayAddrMap.end())
    {
        mPlayAddrMap.erase(pi);
    }
}

///////////////////////////////////////////////////////////////////////////////

int SocketManager::AsyncConnect(playlistInstance_t * pi, uint16_t serverPort, bool isTcp, int flowIdx, connectDelegate_t connectDelegate, closeDelegate_t closeDelegate)
{
    if(mStopped)
       return 0;
    playAddrMap_t::iterator addrIter = mPlayAddrMap.find(pi);
    if (addrIter == mPlayAddrMap.end())
    {
        std::string err("connect attempted on unknown playlist instance");
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, err);
        throw EPHXInternal(err);
    }

    AddrInfo& addr(addrIter->second);

    uint16_t srcPort = TcpPortUtils::MakePort(flowIdx, pi);
    if (isTcp) 
    {
        return AsyncTcpConnect(addr, srcPort, serverPort, connectDelegate, closeDelegate);
    }
    else
    {
        return AsyncUdpConnect(addr, srcPort, serverPort, connectDelegate, closeDelegate);
    }
}

///////////////////////////////////////////////////////////////////////////////

int SocketManager::AsyncTcpConnect(AddrInfo& addr, uint16_t srcPort, uint16_t serverPort, const connectDelegate_t& connectDelegate, const closeDelegate_t& closeDelegate)
{
    // Temporary block scope for connector access
    DpgStreamConnectionHandler *rawConnHandler = 0;
    connHandlerSharedPtr_t connHandler;
    int err = -1;
    {
        ACE_GUARD_RETURN(connLock_t, guard, mConnLock, 0)
        mStreamConnector->BindToIfName(&addr.locIfName);
        addr.locAddr.set_port_number(srcPort);
        addr.remAddr.set_port_number(serverPort);

        // Avoid the string operations if the log level is < INFO
        if ((log4cplus_table[0].LevelMask & PHX_LOG_INFO_FLAG) == PHX_LOG_INFO_FLAG)
        {
            char locAddrStr[64], remAddrStr[64];

            if (addr.locAddr.addr_to_string(locAddrStr, sizeof(locAddrStr)) == -1)
                strcpy(locAddrStr, "invalid");

            if (addr.remAddr.addr_to_string(remAddrStr, sizeof(remAddrStr)) == -1)
                strcpy(remAddrStr, "invalid");
    
            TC_LOG_INFO_LOCAL(0, LOG_CLIENT, "DPG connecting " << locAddrStr << "->" << remAddrStr << " using " << addr.locIfName);
        }

        err = mStreamConnector->MakeHandler(rawConnHandler);
        if (rawConnHandler)
        {
            // fix up the serial number to make it unique per block acceptor + streamconnector + codgramconnector
            rawConnHandler->SetSerial((0x3fffffff & rawConnHandler->Serial()) | 0x40000000);  
            rawConnHandler->RegisterDelegates(connectDelegate, closeDelegate);
            if (mStats) rawConnHandler->SetStatsInstance(*mStats);

            // Insert the connection handler into our map before connect 
            // NOTE: Store the connection handler in a shared pointer with a custom deleter -- this will call remove_reference() when the map entry is erased
            connHandler.reset(rawConnHandler, boost::bind(&DpgStreamConnectionHandler::remove_reference, _1));
            mConnHandlerMap.insert(rawConnHandler->Serial(), connHandler);

        }

        // Initiate a new connection
        err = mStreamConnector->connect(rawConnHandler, addr.remAddr, ACE_Synch_Options(ACE_Synch_Options::USE_REACTOR), addr.locAddr, 1); 
    }

    // FIXME -- bump attempted connections count

    // If the connection attempt failed right away then we must treat this as though the connection was closed
    if (err == -1 && errno != EWOULDBLOCK)
    {
        if (rawConnHandler)
        {
            if (mConnHandlerMap.erase(rawConnHandler->Serial()))
                connHandler.reset();
        }

        // Avoid the string operations if the log level is < INFO
        if ((log4cplus_table[0].LevelMask & PHX_LOG_INFO_FLAG) == PHX_LOG_INFO_FLAG)
        {
            char locAddrStr[64], remAddrStr[64], errorStr[64];

            if (addr.locAddr.addr_to_string(locAddrStr, sizeof(locAddrStr)) == -1)
                strcpy(locAddrStr, "invalid");

            if (addr.remAddr.addr_to_string(remAddrStr, sizeof(remAddrStr)) == -1)
                strcpy(remAddrStr, "invalid");

            TC_LOG_INFO_LOCAL(0, LOG_CLIENT, "DPG failed to connect " << locAddrStr << "->" << remAddrStr << ": " << strerror_r(errno, errorStr, sizeof(errorStr)));
        }

        return 0;
    }
    else
    {
        // FIXME bump active connection count
        // It's possible for the connection to already be closed/removed at this point
        if (mConnHandlerMap.find(rawConnHandler->Serial()))
            return rawConnHandler->Serial();
        else return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

int SocketManager::AsyncUdpConnect(AddrInfo& addr, uint16_t srcPort, uint16_t serverPort, const connectDelegate_t& connectDelegate, const closeDelegate_t& closeDelegate)
{
    // Temporary block scope for connector access
    DpgCodgramConnectionHandler *rawConnHandler = 0; 
    connHandlerSharedPtr_t connHandler;
    int err = -1;
    {
        ACE_GUARD_RETURN(connLock_t, guard, mConnLock, 0)
        mCodgramConnector->BindToIfName(&addr.locIfName);
        addr.locAddr.set_port_number(srcPort);
        addr.remAddr.set_port_number(serverPort);

        // Avoid the string operations if the log level is < INFO
        if ((log4cplus_table[0].LevelMask & PHX_LOG_INFO_FLAG) == PHX_LOG_INFO_FLAG)
        {
            char locAddrStr[64], remAddrStr[64];

            if (addr.locAddr.addr_to_string(locAddrStr, sizeof(locAddrStr)) == -1)
                strcpy(locAddrStr, "invalid");

            if (addr.remAddr.addr_to_string(remAddrStr, sizeof(remAddrStr)) == -1)
                strcpy(remAddrStr, "invalid");
    
            TC_LOG_INFO_LOCAL(0, LOG_CLIENT, "DPG connecting " << locAddrStr << "->" << remAddrStr << " using " << addr.locIfName);
        }

        err = mCodgramConnector->MakeHandler(rawConnHandler);
        if (rawConnHandler)
        {
            // fix up the serial number to make it unique per block acceptor + streamconnector + codgramconnector
            rawConnHandler->SetSerial(0x3fffffff & rawConnHandler->Serial());  
            rawConnHandler->RegisterDelegates(connectDelegate, closeDelegate);
            if (mStats) rawConnHandler->SetStatsInstance(*mStats);
        }

        // UDP "connections" are instant so we need to add them to the map first
        
        // Insert the connection handler into our map 
        // NOTE: Store the connection handler in a shared pointer with a custom deleter -- this will call remove_reference() when the map entry is erased
        connHandler.reset(rawConnHandler, boost::bind(&DpgStreamConnectionHandler::remove_reference, _1));
        mConnHandlerMap.insert(rawConnHandler->Serial(), connHandler);

        // Initiate a new connection
        err = mCodgramConnector->connect(rawConnHandler, addr.remAddr, addr.locAddr, ACE_Synch_Options(ACE_Synch_Options::USE_REACTOR)); 
    }

    // FIXME -- bump attempted connections count
    
    // If the connection attempt failed right away then we must treat this as though the connection was closed
    if (err == -1 && errno != EWOULDBLOCK)
    {
        if (rawConnHandler)
        {
            rawConnHandler->HandleClose(CON_ERR);
            if (mConnHandlerMap.erase(rawConnHandler->Serial()))
                connHandler.reset();
        }
                      
        // Avoid the string operations if the log level is < INFO
        if ((log4cplus_table[0].LevelMask & PHX_LOG_INFO_FLAG) == PHX_LOG_INFO_FLAG)
        {
            char locAddrStr[64], remAddrStr[64], errorStr[64];

            if (addr.locAddr.addr_to_string(locAddrStr, sizeof(locAddrStr)) == -1)
                strcpy(locAddrStr, "invalid");

            if (addr.remAddr.addr_to_string(remAddrStr, sizeof(remAddrStr)) == -1)
                strcpy(remAddrStr, "invalid");

            TC_LOG_INFO_LOCAL(0, LOG_CLIENT, "DPG failed to connect " << locAddrStr << "->" << remAddrStr << ": " << strerror_r(errno, errorStr, sizeof(errorStr)));
        }

        return 0;
    }
    else
    {
        // FIXME bump active connection count
        // It's possible for the connection to already be closed/removed at this point
        if (mConnHandlerMap.find(rawConnHandler->Serial()))
            return rawConnHandler->Serial();
        else return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

void SocketManager::Listen(playlistInstance_t * pi, uint16_t serverPort, bool isTcp, int flowIdx, connectDelegate_t conDelegate, closeDelegate_t closeDelegate)
{
    if(mStopped)
       return;
    playAddrMap_t::iterator addrIter = mPlayAddrMap.find(pi);
    if (addrIter == mPlayAddrMap.end())
    {
        std::string err("listen attempted on unknown playlist instance");
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, err);
        throw EPHXInternal(err);
    }

    AddrInfo addr(addrIter->second);
    addrSet_t remAddrSet;

    if (!addr.hasRem)
    {
        // This is a listen-only playlist, add a thunk to create a new 
        // playlist instance on connect
        conDelegate = boost::bind(&SocketManager::HandleSpawningAccept, this, pi, _1);

        // listen-only playlists can't close
        closeDelegate = closeDelegate_t();

        playSpawnMap_t::iterator spawnIter = mPlaySpawnMap.find(pi);
        if (spawnIter == mPlaySpawnMap.end())
        {
            std::string err("listen (spawn) attempted on unknown playlist instance");
            TC_LOG_ERR_LOCAL(0, LOG_SOCKET, err);
            throw EPHXInternal(err);
        }

        remAddrSet = spawnIter->second.addrSet;
    }
    else
    {
        uint16_t remPort = TcpPortUtils::MakePort(flowIdx, pi);
        addr.remAddr.set_port_number(remPort);
    }

    if (isTcp)
    {
        TcpListen(addr, serverPort, remAddrSet, conDelegate, closeDelegate);
    }
    else
    {
        UdpListen(addr, serverPort, remAddrSet, conDelegate, closeDelegate);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool SocketManager::TcpListen(const AddrInfo& addr, uint16_t serverPort, const addrSet_t& remAddrSet, const connectDelegate_t& conDelegate, const closeDelegate_t& closeDelegate)
{
    bool result = mStreamAcceptor->Listen(addr, serverPort, remAddrSet, conDelegate, closeDelegate);

    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool SocketManager::UdpListen(const AddrInfo& addr, uint16_t serverPort, const addrSet_t& remAddrSet, const connectDelegate_t& conDelegate, const closeDelegate_t& closeDelegate)
{
    bool result = mCodgramAcceptor->Listen(addr, serverPort, remAddrSet, conDelegate, closeDelegate);

    return result;
}

///////////////////////////////////////////////////////////////////////////////

void SocketManager::AsyncSend(int serial, const uint8_t * data, size_t dataLength, sendDelegate_t delegate) 
{
    if(mStopped)
       return;
    connHandlerSharedPtr_t connHandler = mConnHandlerMap.find(uint32_t(serial));
    if (connHandler.get() == 0)
    {
        // connection not found 
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "send attempted on unknown connection: " << uint32_t(serial) << " size: " << mConnHandlerMap.size() << " " << (int)&mConnHandlerMap);
        return;
    }
  
    L4L7_APP_NS::StreamSocketHandler::messagePtr_t mb = L4L7_UTILS_NS::MessageAlloc(new ACE_Message_Block(dataLength)); 
    memcpy(mb->base(), data, dataLength); 
    mb->wr_ptr(dataLength);

    connHandler->AsyncSend(mb, delegate);
}


///////////////////////////////////////////////////////////////////////////////

void SocketManager::AsyncRecv(int serial, size_t dataLength, recvDelegate_t delegate) 
{
    if(mStopped)
       return;
    connHandlerSharedPtr_t connHandler = mConnHandlerMap.find(uint32_t(serial));
    if (connHandler.get() == 0)
    {
        // connection not found 
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "receive attempted on unknown connection: " << uint32_t(serial) << " size: " << mConnHandlerMap.size() << " " << (int)&mConnHandlerMap);
        return;
    }

    connHandler->AsyncRecv(dataLength, delegate);
}

///////////////////////////////////////////////////////////////////////////////

void SocketManager::Close(int serial, bool reset)
{
    connHandlerSharedPtr_t connHandler = mConnHandlerMap.find(uint32_t(serial));
    if (connHandler.get() == 0)
    {
        // connection not found 
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "close attempted on unknown connection: " << uint32_t(serial) << " size: " << mConnHandlerMap.size() << " " << (int)&mConnHandlerMap);
        return;
    }

    // FIXME notify whoever needs to know, update stats
    connHandler->ForceClose(TX_FIN);

    // Remove from the map
    mConnHandlerMap.erase(uint32_t(serial));
}

///////////////////////////////////////////////////////////////////////////////

void SocketManager::Clean(int serial)
{
  connHandlerSharedPtr_t connHandler = mConnHandlerMap.find(uint32_t(serial));
  if (connHandler.get() == 0)
  {
    // connection not found
    TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "close attempted on unknown connection: " << uint32_t(serial) << " size: " << mConnHandlerMap.size() << " " << (int)&mConnHandlerMap);
    return;
  }

  // Remove from the map
  mConnHandlerMap.erase(uint32_t(serial));
}

///////////////////////////////////////////////////////////////////////////////

bool SocketManager::HandleStreamAcceptNotification(DpgStreamConnectionHandler& rawConnHandler) 
{
    if(mStopped)
       return false;
    connHandlerSharedPtr_t connHandler(&rawConnHandler, boost::bind(&DpgStreamConnectionHandler::remove_reference, _1));
    mConnHandlerMap.insert(rawConnHandler.Serial(), connHandler);
    if (mStats) rawConnHandler.SetStatsInstance(*mStats);
    return rawConnHandler.HandleConnect();
}

///////////////////////////////////////////////////////////////////////////////

bool SocketManager::HandlePodgramAcceptNotification(DpgPodgramConnectionHandler& rawConnHandler) 
{
    if(mStopped)
       return false;
    connHandlerSharedPtr_t connHandler(&rawConnHandler, boost::bind(&DpgPodgramConnectionHandler::remove_reference, _1));
    mConnHandlerMap.insert(rawConnHandler.GetSerial(), connHandler);
    if (mStats) rawConnHandler.SetStatsInstance(*mStats);
    return rawConnHandler.HandleConnect();
}

///////////////////////////////////////////////////////////////////////////////

bool SocketManager::HandleStreamConnectionNotification(DpgStreamConnectionHandler& connHandler) 
{
    if(mStopped)
       return false;
    return connHandler.HandleConnect(); 
}

///////////////////////////////////////////////////////////////////////////////

bool SocketManager::HandleCodgramConnectionNotification(DpgCodgramConnectionHandler& connHandler) 
{
    if(mStopped)
       return false;
    return connHandler.HandleConnect(); 
}

///////////////////////////////////////////////////////////////////////////////

void SocketManager::HandleStreamCloseNotification(L4L7_APP_NS::StreamSocketHandler& socketHandler)
{
    DpgStreamConnectionHandler& connHandler(static_cast<DpgStreamConnectionHandler&>(socketHandler));
    if (connHandler.IsPending())
    {
      ACE_GUARD(connLock_t, guard, mConnLock);
        if (mStreamConnector)
            mStreamConnector->cancel(&connHandler);
    }

    if ((connHandler.ErrInfo() == EADDRNOTAVAIL) || (connHandler.ErrInfo() == EADDRINUSE))
    {
        connHandler.HandleClose(CON_ERR /*FIXME CLOSETYPE */);
    }
    else
    {
        connHandler.HandleClose(RX_FIN /*FIXME CLOSETYPE */);
    }
    mConnHandlerMap.erase(connHandler.GetSerial());
}

///////////////////////////////////////////////////////////////////////////////

void SocketManager::HandleCodgramCloseNotification(L4L7_APP_NS::CodgramSocketHandler& socketHandler)
{
    DpgCodgramConnectionHandler& connHandler(static_cast<DpgCodgramConnectionHandler&>(socketHandler));
    // No cancel
    connHandler.HandleClose(RX_FIN /*FIXME CLOSETYPE */); 
    mConnHandlerMap.erase(connHandler.GetSerial());
}

///////////////////////////////////////////////////////////////////////////////

bool SocketManager::HandleSpawningAccept(playlistInstance_t * pi, uint32_t serial)
{
    if(mStopped)
       return false;
    playSpawnMap_t::iterator spawnIter = mPlaySpawnMap.find(pi);
    if (spawnIter == mPlaySpawnMap.end())
    {
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "connect received on unknown playlist instance");
        return false; // don't throw -- could be a race condition
    }

    connHandlerSharedPtr_t connHandler = mConnHandlerMap.find(uint32_t(serial));
    if (connHandler.get() == 0)
    {
        // connection not found 
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "connect received on unknown connection: " << serial << " size: " << mConnHandlerMap.size() << " " << (int)&mConnHandlerMap);
        return false;
    }
   
    ACE_INET_Addr localAddr, remoteAddr; 
    bool result = connHandler->GetLocalAddr(localAddr) && connHandler->GetRemoteAddr(remoteAddr);
    if (!result)
    {
        // connection not found 
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "connect received with no local/remote address");
        return false;
    }

    return spawnIter->second.delegate(pi, (int32_t)serial, localAddr, remoteAddr);
}

///////////////////////////////////////////////////////////////////////////////
