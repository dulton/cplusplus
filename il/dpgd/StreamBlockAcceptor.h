/// @file
/// @brief Stream block (multiple) acceptor header file
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _STREAM_BLOCK_ACCEPTOR_
#define _STREAM_BLOCK_ACCEPTOR_

#include <set>
#include <ace/INET_Addr.h>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>
#include <FastDelegate.h>
#include <Tr1Adapter.h>

#include <engine/AbstSocketManager.h>

#include "AddrInfo.h"
#include "DpgCommon.h"
#include "PortDelegateMap.h"
#include "DpgStreamConnectionHandler.h"

class ACE_Reactor;

namespace L4L7_APP_NS
{
    template<class> class StreamSocketAcceptor;
    class StreamSocketHandler;
};

class TestStreamBlockAcceptor;

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

class StreamBlockAcceptor : boost::noncopyable
{
  public:
    typedef L4L7_ENGINE_NS::AbstSocketManager::connectDelegate_t connectDelegate_t;
    typedef L4L7_ENGINE_NS::AbstSocketManager::closeDelegate_t closeDelegate_t;
    typedef std::set<ACE_INET_Addr> addrSet_t;

    StreamBlockAcceptor(ACE_Reactor * ioReactor);

    UNIT_TEST_VIRTUAL ~StreamBlockAcceptor();

    bool Listen(AddrInfo addr, uint16_t serverPort, const addrSet_t& remAddrSet, connectDelegate_t conDelegate, closeDelegate_t closeDelegate);

    bool Accept(DpgStreamConnectionHandler& rawConnHandler, const AddrInfo& addrInfo);

    void StopAll();

    void CloseAll();

    // Config methods
   
    /// @brief Set the IPv4 TOS socket option
    void SetIpv4Tos(uint8_t tos);

    /// @brief Set the IPv6 traffic class socket option
    void SetIpv6TrafficClass(uint8_t trafficClass);

    /// @brief Set the TCP window size limit socket option
    void SetTcpWindowSizeLimit(uint32_t tcpWindowSizeLimit);

    /// @brief Set the TCP delayed ack socket option
    void SetTcpDelayedAck(bool delayedAck);

    /// @brief Set the TCP no delay socket option
    void SetTcpNoDelay(bool noDelay);

    /// @brief Register for Notifications
    typedef fastdelegate::FastDelegate1<DpgStreamConnectionHandler&, bool> acceptNotificationDelegate_t; 
    void SetAcceptNotificationDelegate(acceptNotificationDelegate_t delegate) { mAcceptNotificationDelegate = delegate; }

  protected:
    typedef L4L7_APP_NS::StreamSocketAcceptor<DpgStreamConnectionHandler> acceptor_t;
    typedef std::tr1::shared_ptr<acceptor_t> acceptorSharedPtr_t;
    typedef PortDelegateMap<DpgStreamConnectionHandler> portDelegateMap_t;
    typedef std::pair<acceptorSharedPtr_t, portDelegateMap_t> acceptorDelegatePair_t;
    typedef boost::unordered_map<AddrInfo, acceptorDelegatePair_t, AddrInfoSrcHash, AddrInfoSrcEqual> acceptorMap_t;   

    UNIT_TEST_VIRTUAL acceptor_t * MakeAcceptor();

    /// @brief Look up the acceptor for the (src) address
    acceptorMap_t::iterator FindAcceptor(const AddrInfo& addr);

    /// @brief End acceptor iterator 
    acceptorMap_t::iterator EndAcceptor(); 

  private:
    // handlers
    bool HandleAcceptNotification(DpgStreamConnectionHandler& rawConnHandler, portDelegateMap_t& delegateMap);
    void HandleCloseNotification(L4L7_APP_NS::StreamSocketHandler& socketHandler);

    uint32_t mSerial;
    uint8_t mIpv4Tos;
    uint8_t mIpv6TrafficClass;
    uint32_t mTcpWindowSizeLimit;
    bool mTcpDelayedAck;
    bool mTcpNoDelay;
    acceptorMap_t mAcceptorMap;
    ACE_Reactor * mIOReactor;
    acceptNotificationDelegate_t mAcceptNotificationDelegate;

    friend class TestStreamBlockAcceptor;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
