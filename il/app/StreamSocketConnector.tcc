/// @file
/// @brief Stream Socket (ACE) Connector templated implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_STREAM_SOCKET_CONNECTOR_TCC_
#define _L4L7_STREAM_SOCKET_CONNECTOR_TCC_

#include <string>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <ace/Connector.h>
#include <ace/SOCK_Connector.h>
#include <FastDelegate.h>

#include <app/AppCommon.h>
#include <app/StreamSocketHandler.h>

#ifndef IPV6_TCLASS
# define IPV6_TCLASS 67         // ref include/linux/in6.h in kernel 2.6 sources
#endif

DECL_L4L7_APP_NS

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
class StreamSocketConnector : public ACE_Connector<ConcreteHandlerType, ACE_SOCK_Connector>
{
  public:
    typedef fastdelegate::FastDelegate1<ConcreteHandlerType &, bool> connectNotificationDelegate_t;
    typedef fastdelegate::FastDelegate1<StreamSocketHandler &> closeNotificationDelegate_t;

    StreamSocketConnector(ACE_Reactor *ioReactor, int flags = 0);

    /// @brief Set the IPv4 TOS socket option
    void SetIpv4Tos(uint8_t tos) { mIpv4Tos = tos; }

    /// @brief Set the IPv6 traffic class socket option
    void SetIpv6TrafficClass(uint8_t trafficClass) { mIpv6TrafficClass = trafficClass; }

    /// @brief Set the TCP window size limit socket option
    void SetTcpWindowSizeLimit(uint32_t tcpWindowSizeLimit) { mTcpWindowSizeLimit = tcpWindowSizeLimit; }

    /// @brief Set the TCP delayed ack socket option
    void SetTcpDelayedAck(bool delayedAck) { mTcpDelayedAck = delayedAck; }

    /// @brief Set the TCP no delay socket option
    void SetTcpNoDelay(bool noDelay) { mTcpNoDelay = noDelay; }

    /// @brief Set the bound interface for the next connection
    void BindToIfName(const std::string *ifName) { mIfName = ifName; }
    
    /// @brief Set the connect event delegate
    /// @note Invoked when an "active" socket connection is succesfully completed
    void SetConnectNotificationDelegate(connectNotificationDelegate_t delegate) { mConnectNotificationDelegate = delegate; }

    /// @brief Set the close event delegate
    /// @note Invoked when a socket connection is closed
    void SetCloseNotificationDelegate(closeNotificationDelegate_t delegate) { mCloseNotificationDelegate = delegate; }

    int MakeHandler(ConcreteHandlerType *&handler) { return make_svc_handler(handler); }
    
  private:
    int make_svc_handler(ConcreteHandlerType *&handler);
    int connect_svc_handler(ConcreteHandlerType *&handler, const ACE_INET_Addr& remote_addr, ACE_Time_Value *timeout, const ACE_INET_Addr& local_addr, int reuse_addr, int flags, int perms);
    int activate_svc_handler(ConcreteHandlerType *handler);

    uint32_t mSerial;
    uint8_t mIpv4Tos;
    uint8_t mIpv6TrafficClass;
    uint32_t mTcpWindowSizeLimit;
    bool mTcpDelayedAck;
    bool mTcpNoDelay;
    const std::string *mIfName;
    connectNotificationDelegate_t mConnectNotificationDelegate;
    closeNotificationDelegate_t mCloseNotificationDelegate;
};

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
StreamSocketConnector<ConcreteHandlerType>::StreamSocketConnector(ACE_Reactor *ioReactor, int flags)
    : ACE_Connector<ConcreteHandlerType, ACE_SOCK_Connector>(ioReactor, flags),
      mSerial(0),
      mIpv4Tos(0),
      mIpv6TrafficClass(0),
      mTcpWindowSizeLimit(0),
      mTcpDelayedAck(true),
      mTcpNoDelay(false),
      mIfName(0)
{
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
int StreamSocketConnector<ConcreteHandlerType>::make_svc_handler(ConcreteHandlerType *&handler)
{
    if (!handler)
    {
        try
        {
            handler = new ConcreteHandlerType(++mSerial);
        }
        catch (std::bad_alloc&)
        {
            return -1;
        }
    }

    // Set new connection's close notification delegate
    handler->SetCloseNotificationDelegate(mCloseNotificationDelegate);
    
    handler->reactor(this->reactor());
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
int StreamSocketConnector<ConcreteHandlerType>::connect_svc_handler(ConcreteHandlerType *&handler, const ACE_INET_Addr& remote_addr, ACE_Time_Value *timeout, const ACE_INET_Addr& local_addr, int reuse_addr, int flags, int perms)
{
    ACE_SOCK_Stream &peer = handler->peer();

    // Peer socket shouldn't be open yet, so we have to open it ourselves (note: ACE will use the pre-opened socket without opening a new one)
    if (peer.get_handle() == ACE_INVALID_HANDLE)
    {
        if (peer.open(SOCK_STREAM, remote_addr.get_type(), 0, reuse_addr) == -1)
            return -1;
    }

    // Set socket options before connection process is started
    if (local_addr.get_type() == AF_INET)
    {
        const int tos = mIpv4Tos;
        peer.set_option(IPPROTO_IP, IP_TOS, (char *) &tos, sizeof(tos));
    }

    if (local_addr.get_type() == AF_INET6)
    {
        const int tclass = mIpv6TrafficClass;
        peer.set_option(IPPROTO_IPV6, IPV6_TCLASS, (char *) &tclass, sizeof(tclass));
    }

    if (mTcpWindowSizeLimit)
    {
        int windowClamp = static_cast<int>(mTcpWindowSizeLimit);
        peer.set_option(SOL_SOCKET, SO_RCVBUF, &windowClamp, sizeof(windowClamp));
    }

    const int quickAck = mTcpDelayedAck ? 0 : 1;
    peer.set_option(IPPROTO_TCP, TCP_QUICKACK, (char *) &quickAck, sizeof(quickAck));

    if (mTcpNoDelay)
    {
        const int intNoDelay = 1;
        peer.set_option(IPPROTO_TCP, TCP_NODELAY, (char *) &intNoDelay, sizeof(intNoDelay));
    }

    // If the user is requesting a specific interface, we need to bind to that device before the connection process is started
    if (mIfName && !mIfName->empty())
        peer.set_option(SOL_SOCKET, SO_BINDTODEVICE, const_cast<char *>(mIfName->c_str()), mIfName->size() + 1);

    mIfName = 0;

    // Start the connection process
    const int err = ACE_Connector<ConcreteHandlerType, ACE_SOCK_Connector>::connect_svc_handler(handler, remote_addr, timeout, local_addr, reuse_addr, flags, perms);
    if (err == -1 && errno == EWOULDBLOCK)
    {
        // Connection is pending
        handler->IsPending(true);
    }
    else if (err == -1)
    {
        // Connection errored out
        handler->ErrInfo(errno);
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
int StreamSocketConnector<ConcreteHandlerType>::activate_svc_handler(ConcreteHandlerType *handler)
{
    ACE_Event_Handler_var safeHandler(handler); 
    handler->add_reference(); // add a reference b/c activate can block

    if (handler->IsPending())
    {
        // Connection is no longer pending
        handler->IsPending(false);
    }

    // Notify the user of connection completion
    if (mConnectNotificationDelegate && !mConnectNotificationDelegate(*handler))
        return -1;

    const int err = ACE_Connector<ConcreteHandlerType, ACE_SOCK_Connector>::activate_svc_handler(handler);

    return err;
}

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_APP_NS

// Local Variables:
// mode:c++
// End:

#endif
