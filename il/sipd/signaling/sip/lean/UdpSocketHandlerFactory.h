/// @file
/// @brief UDP socket handler factory header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _UDP_SOCKET_HANDLER_FACTORY_H_
#define _UDP_SOCKET_HANDLER_FACTORY_H_

#include <stdint.h>

#include <ace/TSS_T.h>
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>
#include <Tr1Adapter.h>

#include "SipCommon.h"

// Forward declarations (global)
class ACE_Reactor;
class Packet;

DECL_SIPLEAN_NS

///////////////////////////////////////////////////////////////////////////////

// Forward declarations
class UdpSocketHandler;

class UdpSocketHandlerFactory : boost::noncopyable
{
  public:
    UdpSocketHandlerFactory() { }
    
    typedef boost::function2<bool, UdpSocketHandler&, const Packet&> sendDelegate_t;
    static void ClearSendDelegate(void) { mSender.clear(); }
    static void SetSendDelegate(const sendDelegate_t& delegate) { mSender = delegate; }
    
    typedef std::tr1::shared_ptr<UdpSocketHandler> udpSocketHandlerSharedPtr_t;
    udpSocketHandlerSharedPtr_t Make(ACE_Reactor& reactor, uint16_t portNumber);
    
  private:
    typedef std::tr1::weak_ptr<UdpSocketHandler> udpSocketHandlerWeakPtr_t;
    typedef boost::unordered_map<uint16_t, udpSocketHandlerWeakPtr_t> portMap_t;

    static sendDelegate_t mSender;      ///< send delegate, installed in new UdpSocketHandler objects
    portMap_t mPortMap;                 ///< map of UdpSocketHandler objects, indexed by port number
};

///////////////////////////////////////////////////////////////////////////////

extern ACE_TSS<UdpSocketHandlerFactory> TSS_UdpSocketHandlerFactory;

///////////////////////////////////////////////////////////////////////////////

END_DECL_SIPLEAN_NS

#endif
