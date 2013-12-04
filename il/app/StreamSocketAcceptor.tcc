/// @file
/// @brief Stream Socket (ACE) Acceptor templated implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_STREAM_SOCKET_ACCEPTOR_TCC_
#define _L4L7_STREAM_SOCKET_ACCEPTOR_TCC_

#include <string>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>
#include <boost/function.hpp>

#include <app/AppCommon.h>

#ifndef IPV6_TCLASS
# define IPV6_TCLASS 67         // ref include/linux/in6.h in kernel 2.6 sources
#endif

DECL_L4L7_APP_NS

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
class StreamSocketAcceptor : public ACE_Acceptor<ConcreteHandlerType, ACE_SOCK_Acceptor>
{
  public:
    typedef boost::function1<bool, ConcreteHandlerType &> acceptNotificationDelegate_t;

    StreamSocketAcceptor(void);
    
    /// @brief Set the IPv4 TOS socket option
    void SetIpv4Tos(uint8_t tos);

    /// @brief Set the IPv6 traffic class socket option
    void SetIpv6TrafficClass(uint8_t trafficClass);

    /// @brief Set the TCP window size limit socket option
    void SetTcpWindowSizeLimit(uint32_t tcpWindowSizeLimit);

    /// @brief Set the TCP delayed ack socket option
    void SetTcpDelayedAck(bool delayedAck);

    /// @brief Set the TCP no delay socket option
    void SetTcpNoDelay(bool noDelaye);

    /// @brief Set the bound interface for the next connection
    void BindToIfName(const std::string& ifName);
    
    /// @brief Set the accept event delegate
    /// @note Invoked when a new "active" socket is created via the accept() system call
    void SetAcceptNotificationDelegate(acceptNotificationDelegate_t delegate) { mAcceptNotificationDelegate = delegate; }

  protected:
    acceptNotificationDelegate_t mAcceptNotificationDelegate;

  private:
    int make_svc_handler(ConcreteHandlerType *&handler);
    int accept_svc_handler(ConcreteHandlerType *handler);

    uint32_t mSerial;
};

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
StreamSocketAcceptor<ConcreteHandlerType>::StreamSocketAcceptor(void)
    : mSerial(0)
{
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
void StreamSocketAcceptor<ConcreteHandlerType>::SetIpv4Tos(uint8_t theTos)
{
    const int tos = theTos;
    this->acceptor().set_option(IPPROTO_IP, IP_TOS, (char *) &tos, sizeof(tos));
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
void StreamSocketAcceptor<ConcreteHandlerType>::SetIpv6TrafficClass(uint8_t trafficClass)
{
    const int tclass = trafficClass;
    this->acceptor().set_option(IPPROTO_IPV6, IPV6_TCLASS, (char *) &tclass, sizeof(tclass));
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
void StreamSocketAcceptor<ConcreteHandlerType>::SetTcpWindowSizeLimit(uint32_t tcpWindowSizeLimit)
{
    int windowClamp = static_cast<int>(tcpWindowSizeLimit);
    this->acceptor().set_option(SOL_SOCKET, SO_RCVBUF, &windowClamp, sizeof(windowClamp));
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
void StreamSocketAcceptor<ConcreteHandlerType>::SetTcpDelayedAck(bool delayedAck)
{
    const int quickAck = delayedAck ? 0 : 1;
    this->acceptor().set_option(IPPROTO_TCP, TCP_QUICKACK, (char *) &quickAck, sizeof(quickAck));
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
void StreamSocketAcceptor<ConcreteHandlerType>::SetTcpNoDelay(bool noDelay)
{
    const int intNoDelay = noDelay ? 1 : 0;
    this->acceptor().set_option(IPPROTO_TCP, TCP_NODELAY, (char *) &intNoDelay, sizeof(intNoDelay));
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
void StreamSocketAcceptor<ConcreteHandlerType>::BindToIfName(const std::string& ifName)
{
    // Bind to device
    this->acceptor().set_option(SOL_SOCKET, SO_BINDTODEVICE, const_cast<char *>(ifName.c_str()), ifName.size() + 1);
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
int StreamSocketAcceptor<ConcreteHandlerType>::make_svc_handler(ConcreteHandlerType *&handler)
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

    handler->reactor(this->reactor());
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
int StreamSocketAcceptor<ConcreteHandlerType>::accept_svc_handler(ConcreteHandlerType *handler)
{
    // Accept the new connection
    if (ACE_Acceptor<ConcreteHandlerType, ACE_SOCK_Acceptor>::accept_svc_handler(handler) == -1)
    {
        handler->remove_reference();
        return -1;
    }

    // Notify the user of new connection
    if (mAcceptNotificationDelegate && !mAcceptNotificationDelegate(*handler))
        return -1;
        
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_APP_NS

// Local Variables:
// mode:c++
// End:

#endif
