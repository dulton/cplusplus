/// @file
/// @brief Connection-oriented datagram block (multiple) acceptor header file
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///
///  This is similar to the stream block acceptor except that it is built on
///  datagram sockets, which don't have the concept of a connection. A packet
///  received on a socket (src ip/udp port + dest ip/udp port tuple) for the 
///  first time is counted as an 'accept' and after that point and until 
///  closed traffic may be sent on that socket. 

#ifndef _CODGRAM_BLOCK_ACCEPTOR_
#define _CODGRAM_BLOCK_ACCEPTOR_

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
#include "DpgPodgramConnectionHandler.h"

class ACE_Reactor;
template<class Connector> class DpgDgramAcceptor;

class TestCodgramBlockAcceptor;

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

class CodgramBlockAcceptor : boost::noncopyable
{
  public:
    typedef L4L7_ENGINE_NS::AbstSocketManager::connectDelegate_t connectDelegate_t;
    typedef L4L7_ENGINE_NS::AbstSocketManager::closeDelegate_t closeDelegate_t;
    typedef std::set<ACE_INET_Addr> addrSet_t;

    CodgramBlockAcceptor(ACE_Reactor * ioReactor);

    UNIT_TEST_VIRTUAL ~CodgramBlockAcceptor();

    bool Listen(AddrInfo addr, uint16_t serverPort, const addrSet_t& remAddrSet, connectDelegate_t conDelegate, closeDelegate_t closeDelegate);

    bool Accept(DpgPodgramConnectionHandler& rawConnHandler, const AddrInfo& addrInfo);

    void StopAll();

    void CloseAll();

    // Config methods
   
    /// @brief Set the IPv4 TOS socket option
    void SetIpv4Tos(uint8_t tos);

    /// @brief Set the IPv6 traffic class socket option
    void SetIpv6TrafficClass(uint8_t trafficClass);

    /// @brief Register for Notifications
    typedef fastdelegate::FastDelegate1<DpgPodgramConnectionHandler&, bool> acceptNotificationDelegate_t; 
    void SetAcceptNotificationDelegate(acceptNotificationDelegate_t delegate) { mAcceptNotificationDelegate = delegate; }

  protected:
    typedef DpgDgramAcceptor<DpgPodgramConnectionHandler> acceptor_t;
    typedef std::tr1::shared_ptr<acceptor_t> acceptorSharedPtr_t;
    typedef PortDelegateMap<DpgPodgramConnectionHandler> portDelegateMap_t;
    typedef std::pair<acceptorSharedPtr_t, portDelegateMap_t> acceptorDelegatePair_t;
    typedef boost::unordered_map<AddrInfo, acceptorDelegatePair_t, AddrInfoSrcHash, AddrInfoSrcEqual> acceptorMap_t;   

    UNIT_TEST_VIRTUAL acceptor_t * MakeAcceptor();

    /// @brief Look up the acceptor for the (src) address
    acceptorMap_t::iterator FindAcceptor(const AddrInfo& addr);

    /// @brief End acceptor iterator 
    acceptorMap_t::iterator EndAcceptor(); 

  private:
    // handlers
    bool HandleAcceptNotification(DpgPodgramConnectionHandler& rawConnHandler, portDelegateMap_t& delegateMap);
    // void HandleCloseNotification(L4L7_APP_NS::StreamSocketHandler& socketHandler);

    uint32_t mSerial;
    uint8_t mIpv4Tos;
    uint8_t mIpv6TrafficClass;
    acceptorMap_t mAcceptorMap;
    ACE_Reactor * mIOReactor;
    acceptNotificationDelegate_t mAcceptNotificationDelegate;

    friend class TestCodgramBlockAcceptor;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
