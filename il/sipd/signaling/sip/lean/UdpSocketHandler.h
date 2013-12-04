/// @file
/// @brief UDP socket handler header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _UDP_SOCKET_HANDLER_H_
#define _UDP_SOCKET_HANDLER_H_

#include <ace/Event_Handler.h>
#include <ace/INET_Addr.h>
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include "VoIPUtils.h"
#include "SipCommon.h"

// Forward declarations (global)
class Packet;

///////////////////////////////////////////////////////////////////////////////

#include <iostream>

namespace boost
{
  template<> struct hash<VOIP_NS::VoIPUtils::AddrKey>
  {
    size_t operator()(const VOIP_NS::VoIPUtils::AddrKey& x) const { return static_cast<size_t>(x.hash()); }
  };
};

namespace std
{
  template<> struct equal_to<VOIP_NS::VoIPUtils::AddrKey>
  {
    bool operator()(const VOIP_NS::VoIPUtils::AddrKey& lhs, const VOIP_NS::VoIPUtils::AddrKey& rhs) const
    {
      if(&lhs==&rhs) return true;
      return lhs==rhs;
    }
  };
};

///////////////////////////////////////////////////////////////////////////////

DECL_SIPLEAN_NS

///////////////////////////////////////////////////////////////////////////////

class UdpSocketHandler : public ACE_Event_Handler, boost::noncopyable
{
  public:
    typedef boost::function1<void, const Packet&> receiverDelegate_t;
    typedef boost::function2<bool, UdpSocketHandler&, const Packet&> sendDelegate_t;
   
    UdpSocketHandler(ACE_Reactor& reactor, uint16_t portNumber);
    ~UdpSocketHandler();

    uint16_t GetPortNumber(void) const { return mPortNumber; }
    ACE_HANDLE get_handle (void) const { return mSockFd; }

    void AttachReceiver(const ACE_INET_Addr& addr, unsigned int ifIndex, const receiverDelegate_t& delegate);
    void DetachReceiver(const ACE_INET_Addr& addr, unsigned int ifIndex);

    void SetSendDelegate(const sendDelegate_t& delegate) { mSender = delegate; }
    
    bool Send(const Packet& packet) { return mSender ? mSender(*this, packet) : SendImmediate(packet); }
    bool SendImmediate(const Packet& packet);
    
  private:
    /// Called when input events occur
    int handle_input(ACE_HANDLE fd);

    void DumpPacket(const Packet& packet) const;

    typedef boost::unordered_map<VOIP_NS::VoIPUtils::AddrKey, receiverDelegate_t> receiverMap_t;
    
    static const int RCVBUF_SIZE;       ///< socket receive buffer size

    const uint16_t mPortNumber;         ///< UDP port number
    int mSockFd;                        ///< underlying socket descriptor

    receiverMap_t mReceivers;           ///< attached receivers
    sendDelegate_t mSender;             ///< send delegate, typically for unit test indirection

    bool mTxPacketDump;                 ///< when true, log tx packet contents
    bool mRxPacketDump;                 ///< when true, log rx packet contents
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_SIPLEAN_NS

#endif
