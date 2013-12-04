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

#ifndef _L4L7_STREAM_SOCKET_ONESHOT_ACCEPTOR_TCC_
#define _L4L7_STREAM_SOCKET_ONESHOT_ACCEPTOR_TCC_

#include <string>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>
#include <FastDelegate.h>

#include <app/AppCommon.h>

#ifndef IPV6_TCLASS
# define IPV6_TCLASS 67         // ref include/linux/in6.h in kernel 2.6 sources
#endif


#if 0  // change this to enable debug logs from one shot acceptor.
#define DEBUG_ONESHOT
#endif

DECL_L4L7_APP_NS

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
class StreamSocketOneshotAcceptor : public ACE_Oneshot_Acceptor<ConcreteHandlerType, ACE_SOCK_Acceptor>
{
  public:
    typedef ACE_Oneshot_Acceptor<ConcreteHandlerType, ACE_SOCK_Acceptor> ParentType ;
    typedef fastdelegate::FastDelegate1<ConcreteHandlerType &, bool> acceptNotificationDelegate_t;

    StreamSocketOneshotAcceptor();   
 
    /// @brief Start the acceptor
    int accept(ACE_INET_Addr &local, ACE_Reactor *reactor) ;

    /// @brief Set the IPv4 TOS socket option
    void SetIpv4Tos(uint8_t tos);

    /// @brief Set the IPv6 traffic class socket option
    void SetIpv6TrafficClass(uint8_t trafficClass);

    /// @brief Set the TCP window size limit socket option
    void SetTcpWindowSizeLimit(uint32_t tcpWindowSizeLimit);

    /// @brief Set the TCP delayed ack socket option
    void SetTcpDelayedAck(bool delayedAck);

    /// @brief Set the bound interface for the next connection
    void BindToIfName(const std::string& ifName);
    
    /// @brief Set the accept event delegate
    /// @note Invoked when a new "active" socket is created via the accept() system call
    void SetAcceptNotificationDelegate(acceptNotificationDelegate_t delegate) { mAcceptNotificationDelegate = delegate; }

  private:
    // convinience typedefs
    typedef std::auto_ptr<ConcreteHandlerType>  handlerPtr_t ;

    int activate_svc_handler(ConcreteHandlerType *handler) ;

    uint32_t mSerial;
    uint8_t mIpv4Tos;
    uint8_t mIpv6TrafficClass;
    uint32_t mTcpWindowSizeLimit;
    bool mTcpDelayedAck;
    acceptNotificationDelegate_t mAcceptNotificationDelegate;
    handlerPtr_t mHandler ;
};

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
StreamSocketOneshotAcceptor<ConcreteHandlerType>::StreamSocketOneshotAcceptor() :
    ParentType(),
    mSerial(0),
    mIpv4Tos(0),
    mIpv6TrafficClass(0),
    mTcpWindowSizeLimit(0),
    mTcpDelayedAck(true)
{
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
int StreamSocketOneshotAcceptor<ConcreteHandlerType>::accept(
    ACE_INET_Addr &local, 
    ACE_Reactor *reactor) 
{
    if (ParentType::open(local, reactor) == 0)
    {
        if (mTcpWindowSizeLimit)
        {
            const int windowClamp = static_cast<const int>(mTcpWindowSizeLimit);
            this->acceptor().set_option(IPPROTO_TCP, TCP_WINDOW_CLAMP, (char *) &windowClamp, sizeof(windowClamp));
        }

        const int quickAck = mTcpDelayedAck ? 0 : 1;
        this->acceptor().set_option(IPPROTO_TCP, TCP_QUICKACK, (char *) &quickAck, sizeof(quickAck));

        // set peer acceptor into non-blocking mode. See Acceptor.cpp, line 97
        this->acceptor().enable(ACE_NONBLOCK) ;

        // reload our local address.
        this->acceptor().get_local_addr(local) ; 

        if (local.get_type() == AF_INET)
        {
            const int tos = mIpv4Tos;
            this->acceptor().set_option(IPPROTO_IP, IP_TOS, (char *) &tos, sizeof(tos));
        }

        if (local.get_type() == AF_INET6)
        {
            const int tclass = mIpv6TrafficClass;
            this->acceptor().set_option(IPPROTO_IPV6, IPV6_TCLASS, (char *) &tclass, sizeof(tclass));
        }

        // Allocate new handler
        mHandler.reset(new ConcreteHandlerType(++mSerial)) ;
        mHandler->reactor(this->reactor()) ;

        // Try to setup acceptor.
        const int status = ParentType::accept(mHandler.get(), 0, ACE_Synch_Options(ACE_Synch_Options::USE_REACTOR)) ;
        if ( (status == 0) || 
             ((status == -1) && (errno == EWOULDBLOCK || errno == ENOENT)) 
           )
        {
#ifdef DEBUG_ONESHOT
           {
               ACE_Event_Handler_var safeHandler(mHandler.get());
               std::cerr << "Handler: " << mHandler.get() << ". Status: " << ((status == 0) ? "Accepted" : "Pending") << 
                            ". ReferenceCount = " << (mHandler->add_reference() - 1) << std::endl ;
           }
#endif
            //mHandler.release() ;
            return 0 ; 
        }

#ifdef DEBUG_ONESHOT
        std::cerr << "Failed. Uknown error in oneshot acceptor. errno = " << errno <<
				  " and msg = " << strerror(errno) << ". Handler = " << mHandler.get() << 
                  " and reference count = " << mHandler.release()->remove_reference() << std::endl ;
#else
        mHandler.release()->remove_reference() ; // leave it reactor to deal with this.
#endif

    }

    return -1 ;
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
void StreamSocketOneshotAcceptor<ConcreteHandlerType>::SetIpv4Tos(uint8_t theTos)
{
    mIpv4Tos = theTos;
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
void StreamSocketOneshotAcceptor<ConcreteHandlerType>::SetIpv6TrafficClass(uint8_t trafficClass)
{
    mIpv6TrafficClass = trafficClass;
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
void StreamSocketOneshotAcceptor<ConcreteHandlerType>::SetTcpWindowSizeLimit(uint32_t tcpWindowSizeLimit)
{
    mTcpWindowSizeLimit = tcpWindowSizeLimit;
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
void StreamSocketOneshotAcceptor<ConcreteHandlerType>::SetTcpDelayedAck(bool delayedAck)
{
    mTcpDelayedAck = delayedAck;
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
void StreamSocketOneshotAcceptor<ConcreteHandlerType>::BindToIfName(const std::string& ifName)
{
    // Bind to device
    this->acceptor().set_option(SOL_SOCKET, SO_BINDTODEVICE, const_cast<char *>(ifName.c_str()), ifName.size() + 1);
}

///////////////////////////////////////////////////////////////////////////////

template<class ConcreteHandlerType>
int StreamSocketOneshotAcceptor<ConcreteHandlerType>::activate_svc_handler(ConcreteHandlerType *handler)
{   
    assert(mHandler.get() == handler) ;

    if (mAcceptNotificationDelegate && !mAcceptNotificationDelegate(*handler))
    {
#ifdef DEBUG_ONESHOT
        {    
            ACE_Event_Handler_var safeHandler(handler);
		    std::cerr << "activate_failed: Current reference: " << handler << "::" << (handler->add_reference() - 1) << std::endl ;
        }
#endif
        // Close handler to avoid leaks.
        handler->close() ;
        return -1 ;
    }

    // At the time of calling accept(), we save the handler in our local pointer -- just in case accept
    // fails. However, activate_svc_handler indicates that accept() has completed, so, we release our
    // reference to that pointer at this time.
    mHandler.release() ;

    // set peer connection into non-blocking mode. See Acceptor.cpp, line 326
    handler->peer().enable(ACE_NONBLOCK) ;

    return ParentType::activate_svc_handler(handler) ;
}

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_APP_NS

// Local Variables:
// mode:c++
// End:

#endif
