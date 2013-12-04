/// @file
/// @brief Connection-oriented Datagram Socket (ACE) Connector implementation
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///
///  There is no such thing as a datagram connection really, but we pretend
///  there is in order to make handling UDP packets easier.

#ifndef _L4L7_CODGRAM_SOCKET_CONNECTOR_TCC_
#define _L4L7_CODGRAM_SOCKET_CONNECTOR_TCC_

#include <string>
#include <netinet/in.h>

#include <FastDelegate.h>

#include <app/AppCommon.h>
#include <app/CodgramSocketHandler.h>

#ifndef IPV6_TCLASS
# define IPV6_TCLASS 67         // ref include/linux/in6.h in kernel 2.6 sources
#endif

DECL_L4L7_APP_NS

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
class CodgramSocketConnector
{
  public:
    // Traits taken from ACE_Connector
    typedef typename ConcreteHandlerType::addr_type        addr_type;

    typedef fastdelegate::FastDelegate1<ConcreteHandlerType &, bool> connectNotificationDelegate_t;
    typedef fastdelegate::FastDelegate1<CodgramSocketHandler &> closeNotificationDelegate_t;

    CodgramSocketConnector(ACE_Reactor *ioReactor, int flags = 0);

    /// @brief Set the IPv4 TOS socket option
    void SetIpv4Tos(uint8_t tos) { mIpv4Tos = tos; }

    /// @brief Set the IPv6 traffic class socket option
    void SetIpv6TrafficClass(uint8_t trafficClass) { mIpv6TrafficClass = trafficClass; }

    /// @brief Set the bound interface for the next connection
    void BindToIfName(const std::string *ifName) { mIfName = ifName; }
    
    /// @brief Set the connect event delegate
    /// @note Invoked when an "active" socket connection is succesfully completed
    void SetConnectNotificationDelegate(connectNotificationDelegate_t delegate) { mConnectNotificationDelegate = delegate; }

    /// @brief Set the close event delegate
    /// @note Invoked when a socket connection is closed
    void SetCloseNotificationDelegate(closeNotificationDelegate_t delegate) { mCloseNotificationDelegate = delegate; }

    int MakeHandler(ConcreteHandlerType *&handler) { return make_svc_handler(handler); }

    int connect(ConcreteHandlerType *& sh, const ACE_INET_Addr& remote_addr, const ACE_INET_Addr& local_addr, const ACE_Synch_Options& synch_options = ACE_Synch_Options::defaults);

    void close();
   
  private:
    int make_svc_handler(ConcreteHandlerType *&handler);
    int connect_svc_handler(ConcreteHandlerType *&handler, const ACE_INET_Addr& remote_addr, ACE_Time_Value *timeout, const ACE_INET_Addr& local_addr, int reuse_addr, int flags, int perms);
    int activate_svc_handler(ConcreteHandlerType *handler);

    ACE_Reactor * mReactor;
    int mFlags;
    uint32_t mSerial;
    uint8_t mIpv4Tos;
    uint8_t mIpv6TrafficClass;
    bool mClosed;
    const std::string *mIfName;
    connectNotificationDelegate_t mConnectNotificationDelegate;
    closeNotificationDelegate_t mCloseNotificationDelegate;
};

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
CodgramSocketConnector<ConcreteHandlerType>::CodgramSocketConnector(ACE_Reactor *ioReactor, int flags)
    : mReactor(ioReactor),
      mFlags(flags),
      mSerial(0),
      mIpv4Tos(0),
      mIpv6TrafficClass(0),
      mClosed(false),
      mIfName(0)
{
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
void CodgramSocketConnector<ConcreteHandlerType>::close()
{
    // close just stops subsequent connects from working
    mClosed = true;
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
int CodgramSocketConnector<ConcreteHandlerType>::connect(ConcreteHandlerType *& sh, const ACE_INET_Addr& remote_addr, const ACE_INET_Addr& local_addr, const ACE_Synch_Options& synch_options)
{
    if (mClosed)
        return -1;

    if (make_svc_handler(sh) == -1)
        return -1;

    // taken from ACE_Connector::connect_i
    ACE_Time_Value *timeout = 0;
    int const use_reactor = synch_options[ACE_Synch_Options::USE_REACTOR];

    if (use_reactor)
        timeout = const_cast<ACE_Time_Value *> (&ACE_Time_Value::zero);
    else
        timeout = const_cast<ACE_Time_Value *> (synch_options.time_value ());

    int result = connect_svc_handler(sh, remote_addr, timeout, local_addr, 1, O_RDWR, 0);
    if (result != -1)
        return activate_svc_handler(sh);

    return result;
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
int CodgramSocketConnector<ConcreteHandlerType>::make_svc_handler(ConcreteHandlerType *&handler)
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
    
    handler->reactor(mReactor);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
int CodgramSocketConnector<ConcreteHandlerType>::connect_svc_handler(ConcreteHandlerType *&handler, const ACE_INET_Addr& remote_addr, ACE_Time_Value *timeout, const ACE_INET_Addr& local_addr, int reuse_addr, int flags, int perms)
{
    ACE_SOCK_CODgram &peer = handler->peer();

    // Peer socket shouldn't be open yet, so we have to open it ourselves (note: ACE will use the pre-opened socket without opening a new one)
    if (peer.get_handle() == ACE_INVALID_HANDLE)
    {
        if (peer.open(remote_addr, local_addr, remote_addr.get_type(), IPPROTO_UDP, reuse_addr) == -1)
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

    // If the user is requesting a specific interface, we need to bind to that device before the connection process is started
    if (mIfName && !mIfName->empty())
        peer.set_option(SOL_SOCKET, SO_BINDTODEVICE, const_cast<char *>(mIfName->c_str()), mIfName->size() + 1);

    mIfName = 0;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
int CodgramSocketConnector<ConcreteHandlerType>::activate_svc_handler(ConcreteHandlerType *handler)
{
    ACE_Event_Handler_var safeHandler(handler); 
    handler->add_reference(); // add a reference b/c activate can block

    // Logic below taken from ACE_Connector::activate_svc_handler
   
    int err = 0; 
    if (ACE_BIT_ENABLED(mFlags, ACE_NONBLOCK))
    {
        if (handler->peer().enable(ACE_NONBLOCK) == -1)
            err = 1;
    }
    else if (handler->peer().disable(ACE_NONBLOCK) == -1)
    {
        err = 1;
    }

    if (err || handler->open((void *) this) == -1)
    {
      // Close down the handler to avoid descriptor leaks.
      handler->close(0);
      return -1;
    }

    ////

    if (handler->IsPending())
    {
        // Connection is no longer pending
        handler->IsPending(false);
    }

    // Notify the user of connection completion
    if (mConnectNotificationDelegate && !mConnectNotificationDelegate(*handler))
        return -1;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_APP_NS

// Local Variables:
// mode:c++
// End:

#endif
