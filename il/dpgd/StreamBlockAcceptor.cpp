/// @file
/// @brief Block (multiple) acceptor implementation
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

#include <app/StreamSocketAcceptor.tcc>
#include "DpgStreamConnectionHandler.h"

#include "DpgdLog.h"
#include "StreamBlockAcceptor.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

StreamBlockAcceptor::StreamBlockAcceptor(ACE_Reactor * ioReactor) : 
  mSerial(0),
  mIpv4Tos(0),
  mIpv6TrafficClass(0),
  mTcpWindowSizeLimit(0),
  mTcpDelayedAck(true),
  mTcpNoDelay(false),
  mIOReactor(ioReactor)
{
}

///////////////////////////////////////////////////////////////////////////////

StreamBlockAcceptor::~StreamBlockAcceptor()
{
    // FIXME clean up acceptors/handlers?
}

///////////////////////////////////////////////////////////////////////////////

void StreamBlockAcceptor::SetIpv4Tos(uint8_t tos) 
{
    mIpv4Tos = tos; 
}

///////////////////////////////////////////////////////////////////////////////

void StreamBlockAcceptor::SetIpv6TrafficClass(uint8_t trafficClass) 
{  
    mIpv6TrafficClass = trafficClass; 
}

///////////////////////////////////////////////////////////////////////////////

void StreamBlockAcceptor::SetTcpWindowSizeLimit(uint32_t tcpWindowSizeLimit) 
{   
    mTcpWindowSizeLimit = tcpWindowSizeLimit; 
}

///////////////////////////////////////////////////////////////////////////////

void StreamBlockAcceptor::SetTcpDelayedAck(bool delayedAck) 
{   
    mTcpDelayedAck = delayedAck; 
}

///////////////////////////////////////////////////////////////////////////////

void StreamBlockAcceptor::SetTcpNoDelay(bool noDelay) 
{   
    mTcpNoDelay = noDelay; 
}

///////////////////////////////////////////////////////////////////////////////

StreamBlockAcceptor::acceptor_t * StreamBlockAcceptor::MakeAcceptor()
{
    return new acceptor_t;
}

///////////////////////////////////////////////////////////////////////////////

bool StreamBlockAcceptor::Listen(AddrInfo addr, uint16_t serverPort, const addrSet_t& remAddrSet, connectDelegate_t conDelegate, closeDelegate_t closeDelegate)
{
    addr.locAddr.set_port_number(serverPort);

    acceptorMap_t::iterator accept_iter = FindAcceptor(addr);

    if (accept_iter == mAcceptorMap.end())
    {
        accept_iter = mAcceptorMap.insert(acceptorMap_t::value_type(addr,acceptorDelegatePair_t(acceptorSharedPtr_t(MakeAcceptor()), portDelegateMap_t(addr.remAddr, conDelegate, closeDelegate)))).first;

        // add additional remote addresses
        portDelegateMap_t& portDelegateMap(accept_iter->second.second);
        BOOST_FOREACH(const ACE_INET_Addr& remAddr, remAddrSet)
        {
            portDelegateMap.push(remAddr, conDelegate, closeDelegate);
        }

        acceptorSharedPtr_t acceptor(accept_iter->second.first);

        acceptor->SetAcceptNotificationDelegate(boost::bind(&StreamBlockAcceptor::HandleAcceptNotification, this, _1, boost::ref(accept_iter->second.second)));

        if (acceptor->open(addr.locAddr, mIOReactor) == -1)
        {
            char addrStr[64];
            
            if (addr.locAddr.addr_to_string(addrStr, sizeof(addrStr)) == -1)
                strcpy(addrStr, "invalid");

            TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "DPG listen " << addrStr << " using " << addr.locIfName << " failed");

            return false;
        }

        if (addr.locAddr.get_type() == AF_INET)
            acceptor->SetIpv4Tos(mIpv4Tos);

        if (addr.locAddr.get_type() == AF_INET6)
            acceptor->SetIpv6TrafficClass(mIpv6TrafficClass);

        acceptor->SetTcpWindowSizeLimit(mTcpWindowSizeLimit);
        acceptor->SetTcpDelayedAck(mTcpDelayedAck);
        acceptor->SetTcpNoDelay(mTcpNoDelay);
    
        // Bind acceptor to given interface name
        if (!addr.locIfName.empty())
            acceptor->BindToIfName(addr.locIfName);
    }
    else
    {
        // already listening, add new remote address-based delegate
        portDelegateMap_t& portDelegateMap(accept_iter->second.second);
        portDelegateMap.push(addr.remAddr, conDelegate, closeDelegate);

        // add additional remote addresses
        BOOST_FOREACH(const ACE_INET_Addr& remAddr, remAddrSet)
        {
            portDelegateMap.push(remAddr, conDelegate, closeDelegate);
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void StreamBlockAcceptor::StopAll()
{
    // should only be called once before destruction
    BOOST_FOREACH(acceptorMap_t::value_type& item, mAcceptorMap)
    {
        ACE_Reactor * reactor = item.second.first->reactor();
        if (reactor) reactor->suspend_handler(item.second.first.get());
    }
}

///////////////////////////////////////////////////////////////////////////////

void StreamBlockAcceptor::CloseAll()
{
    // should only be called once before destruction
    BOOST_FOREACH(acceptorMap_t::value_type& item, mAcceptorMap)
    {
        item.second.first->close();
    }
}

///////////////////////////////////////////////////////////////////////////////

bool StreamBlockAcceptor::Accept(DpgStreamConnectionHandler& rawConnHandler, const AddrInfo& playlistAddr)
{
    AddrInfo connAddr(playlistAddr);

    ACE_INET_Addr localAddr;
    if (!rawConnHandler.GetLocalAddr(localAddr))
    {
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "Accept failed to find local address");
        return false;
    }

    // propagate server port number -- the playlist address passed in
    // will always have 0 for its local port number
    connAddr.locAddr.set_port_number(localAddr.get_port_number());

    acceptorMap_t::iterator accept_iter = FindAcceptor(connAddr);
    if (accept_iter == mAcceptorMap.end())
    {
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "Accept failed to find acceptor for " << connAddr);
        return false;
    }

    portDelegateMap_t& delegateMap(accept_iter->second.second);
    if (!delegateMap.register_and_pop(connAddr.remAddr, rawConnHandler, true))
    {
        // NOTE: this is to prevent an infinite recursion if spawning and 
        // starting a playlist doesn't create the proper listener
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "Accept failed to find exact match listener for " << connAddr);
        return false;
    }

    return rawConnHandler.HandleConnect();
}

///////////////////////////////////////////////////////////////////////////////

bool StreamBlockAcceptor::HandleAcceptNotification(DpgStreamConnectionHandler& rawConnHandler, portDelegateMap_t& delegateMap)
{
    ACE_INET_Addr remoteAddr;
    bool result = rawConnHandler.GetRemoteAddr(remoteAddr);
    if (!result)
    {
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "DPG accept failed: unable to retrieve remote address");
        return false;
    }
    
    // fix up the serial number to make it unique per block acceptor + connector
    // this must happen before the delegates are called below
    rawConnHandler.SetSerial(0x80000000 | mSerial++);

    //TCP_WINDOW_CLAMP is not supported by USS and socket recv buffer is used to
    //handle RxWindowSizeLimit.
    rawConnHandler.SetSoRcvBuf(mTcpWindowSizeLimit);

    
    delegateMap.register_and_pop(remoteAddr, rawConnHandler);

    // notify our owner (the socket manager) 
    return mAcceptNotificationDelegate(rawConnHandler);
}

///////////////////////////////////////////////////////////////////////////////

void StreamBlockAcceptor::HandleCloseNotification(L4L7_APP_NS::StreamSocketHandler& socketHandler)
{
    DpgStreamConnectionHandler& connHandler(static_cast<DpgStreamConnectionHandler&>(socketHandler));
    connHandler.HandleClose(L4L7_ENGINE_NS::AbstSocketManager::RX_FIN /*FIXME CLOSETYPE */); 
}

///////////////////////////////////////////////////////////////////////////////

StreamBlockAcceptor::acceptorMap_t::iterator StreamBlockAcceptor::FindAcceptor(const AddrInfo& addr)
{
    return mAcceptorMap.find(addr);
}

///////////////////////////////////////////////////////////////////////////////

StreamBlockAcceptor::acceptorMap_t::iterator StreamBlockAcceptor::EndAcceptor() 
{
    return mAcceptorMap.end();
}

///////////////////////////////////////////////////////////////////////////////
