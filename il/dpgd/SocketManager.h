/// @file
/// @brief Dpg socket manager header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _DPG_SOCKET_MANAGER_H_
#define _DPG_SOCKET_MANAGER_H_

#include <ace/INET_Addr.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <boost/scoped_ptr.hpp>
#include <Tr1Adapter.h>
#include <utils/SharedMap.h>

#include <engine/AbstSocketManager.h>

#include "DpgCommon.h"

class ACE_Reactor;
class TestDpgClientBlock;
class TestDpgServerBlock;
class TestSocketManager;

namespace L4L7_APP_NS
{
    template<class> class StreamSocketConnector;
    template<class> class CodgramSocketConnector;
    class StreamSocketHandler;
    class CodgramSocketHandler;
};

DECL_DPG_NS

class AddrInfo;
class StreamBlockAcceptor;
class CodgramBlockAcceptor;
class DpgConnectionHandler;
class DpgCodgramConnectionHandler;
class DpgPodgramConnectionHandler;
class DpgRawStats;
class DpgStreamConnectionHandler;

///////////////////////////////////////////////////////////////////////////////

class SocketManager : public L4L7_ENGINE_NS::AbstSocketManager
{
  public:
    typedef L4L7_ENGINE_NS::PlaylistInstance playlistInstance_t;
    typedef boost::function4<bool, playlistInstance_t *, int32_t, const ACE_INET_Addr&, const ACE_INET_Addr&> spawnAcceptDelegate_t;
    typedef DpgRawStats stats_t;
    typedef std::set<ACE_INET_Addr> addrSet_t;

    SocketManager(ACE_Reactor * ioReactor);

    virtual ~SocketManager();

    // AbstSocketManager methods
    
    int AsyncConnect(playlistInstance_t * pi, uint16_t serverPort, bool isTcp, int flowIdx, connectDelegate_t conDelegate, closeDelegate_t closeDelegate); 

    void Listen(playlistInstance_t * pi, uint16_t serverPort, bool isTcp, int flowIdx, connectDelegate_t conDelegate, closeDelegate_t closeDelegate);
    // FIXME make sure we cancel any in-progress accepts on playlist stop

    void AsyncSend(int fd, const uint8_t * data, size_t dataLength, sendDelegate_t); 

    void AsyncRecv(int fd, size_t dataLength, recvDelegate_t);

    void Close(int fd, bool reset);

    // Clean out the connHandler from mConnHandlerMap, should be only used when close comes from remote side of connection
    void Clean(int fd);

    // Control (by owner) methods
    bool Accept(playlistInstance_t * pi, uint32_t serial);

    void Stop();

    void Reset();

    // Config methods
   
    /// @brief Set the IPv4 TOS socket option
    void SetIpv4Tos(uint8_t tos);

    /// @brief Get the IPv4 TOS socket option
    uint8_t GetIpv4Tos() { return mIpv4Tos; }

    /// @brief Set the IPv6 traffic class socket option
    void SetIpv6TrafficClass(uint8_t trafficClass);

    /// @brief Get the IPv6 traffic class socket option
    uint8_t GetIpv6TrafficClass() { return mIpv6TrafficClass; }

    /// @brief Set the TCP window size limit socket option
    void SetTcpWindowSizeLimit(uint32_t tcpWindowSizeLimit);

    /// @brief Get the TCP window size limit socket option
    uint32_t GetTcpWindowSizeLimit() { return mTcpWindowSizeLimit; }

    /// @brief Set the TCP delayed ack socket option
    void SetTcpDelayedAck(bool delayedAck);

    /// @brief Get the TCP delayed ack socket option
    bool GetTcpDelayedAck() { return mTcpDelayedAck; }

    /// @brief Set the TCP no delay socket option
    void SetTcpNoDelay(bool noDelay);

    /// @brief Get the TCP no delay socket option
    bool GetTcpNoDelay() { return mTcpNoDelay; }

    /// @brief Set stats instance
    void SetStatsInstance(stats_t& stats);

    // Playlist management 

    void AddPlaylist(playlistInstance_t * pi, const std::string& srcIfName, const ACE_INET_Addr& srcAddr, const ACE_INET_Addr& dstAddr); 

    void AddListeningPlaylist(playlistInstance_t * pi, const std::string& srcIfName, const ACE_INET_Addr& srcAddr, const addrSet_t& remAddrSet, const spawnAcceptDelegate_t& saDelegate); 

    void AddChildPlaylist(playlistInstance_t * pi, playlistInstance_t * parent, const ACE_INET_Addr& remoteAddr);

    void RemovePlaylist(playlistInstance_t * pi);

  private:
    // implementation
    int AsyncTcpConnect(AddrInfo& addr, uint16_t srcPort, uint16_t serverPort, const connectDelegate_t& conDelegate, const closeDelegate_t& closeDelegate); 
    int AsyncUdpConnect(AddrInfo& addr, uint16_t srcPort, uint16_t serverPort, const connectDelegate_t& conDelegate, const closeDelegate_t& closeDelegate); 
    bool TcpListen(const AddrInfo& addr, uint16_t serverPort, const addrSet_t& remAddrSet, const connectDelegate_t& conDelegate, const closeDelegate_t& closeDelegate);
    bool UdpListen(const AddrInfo& addr, uint16_t serverPort, const addrSet_t& remAddrSet, const connectDelegate_t& conDelegate, const closeDelegate_t& closeDelegate);

    // handler
    bool HandleStreamAcceptNotification(DpgStreamConnectionHandler& connHandler);
    bool HandlePodgramAcceptNotification(DpgPodgramConnectionHandler& connHandler);
    bool HandleStreamConnectionNotification(DpgStreamConnectionHandler& connHandler);
    bool HandleCodgramConnectionNotification(DpgCodgramConnectionHandler& connHandler);
    void HandleStreamCloseNotification(L4L7_APP_NS::StreamSocketHandler& socketHandler);
    void HandleCodgramCloseNotification(L4L7_APP_NS::CodgramSocketHandler& socketHandler);
    bool HandleSpawningAccept(playlistInstance_t * pi, uint32_t serial);

    // FIXME -- switch to using playlist serial numbers?
    typedef std::map<playlistInstance_t *, AddrInfo> playAddrMap_t;

    struct spawnInfo_t
    {
        spawnAcceptDelegate_t delegate;
        addrSet_t addrSet;
    };
    typedef std::map<playlistInstance_t *, spawnInfo_t> playSpawnMap_t;

    typedef ACE_Recursive_Thread_Mutex connLock_t;
    typedef StreamBlockAcceptor streamAcceptor_t;
    typedef CodgramBlockAcceptor codgramAcceptor_t;
    typedef L4L7_APP_NS::StreamSocketConnector<DpgStreamConnectionHandler> streamConnector_t;
    typedef L4L7_APP_NS::CodgramSocketConnector<DpgCodgramConnectionHandler> codgramConnector_t;
    typedef L4L7_UTILS_NS::SharedMap<uint32_t, DpgConnectionHandler> connHandlerMap_t;
    typedef connHandlerMap_t::shared_ptr_t connHandlerSharedPtr_t;

    uint8_t mIpv4Tos;
    uint8_t mIpv6TrafficClass;
    uint32_t mTcpWindowSizeLimit;
    bool mTcpDelayedAck;
    bool mTcpNoDelay;
    stats_t *mStats;
    bool mStopped;

    // FIXME -- when do we remove playlists from these?
    playAddrMap_t mPlayAddrMap;
    playSpawnMap_t mPlaySpawnMap;

    connLock_t mConnLock; 
    boost::scoped_ptr<streamAcceptor_t> mStreamAcceptor;
    boost::scoped_ptr<codgramAcceptor_t> mCodgramAcceptor;
    boost::scoped_ptr<streamConnector_t> mStreamConnector;
    boost::scoped_ptr<codgramConnector_t> mCodgramConnector;
    connHandlerMap_t mConnHandlerMap;                          

    friend class TestDpgClientBlock;
    friend class TestDpgServerBlock;
    friend class TestSocketManager;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
