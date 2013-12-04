/// @file
/// @brief Connection-oriented datagram block (multiple) acceptor implementation
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <deque>
#include <boost/foreach.hpp>

#include <app/DatagramSocketHandler.h>
#include "DpgPodgramConnectionHandler.h"
#include "DpgDgramAcceptor.tcc"

#include "DpgdLog.h"
#include "CodgramBlockAcceptor.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

CodgramBlockAcceptor::CodgramBlockAcceptor(ACE_Reactor * ioReactor) : 
  mSerial(0),
  mIpv4Tos(0),
  mIpv6TrafficClass(0),
  mIOReactor(ioReactor)
{
}

///////////////////////////////////////////////////////////////////////////////

CodgramBlockAcceptor::~CodgramBlockAcceptor()
{
    // FIXME clean up acceptors/handlers?
}

///////////////////////////////////////////////////////////////////////////////

void CodgramBlockAcceptor::SetIpv4Tos(uint8_t tos) 
{
    mIpv4Tos = tos; 
}

///////////////////////////////////////////////////////////////////////////////

void CodgramBlockAcceptor::SetIpv6TrafficClass(uint8_t trafficClass) 
{  
    mIpv6TrafficClass = trafficClass; 
}

///////////////////////////////////////////////////////////////////////////////

CodgramBlockAcceptor::acceptor_t * CodgramBlockAcceptor::MakeAcceptor()
{
    // FIXME -- FIND THE PROPER MTU AND PASS IT IN
    return new acceptor_t(2048);
}

///////////////////////////////////////////////////////////////////////////////

bool CodgramBlockAcceptor::Listen(AddrInfo addr, uint16_t serverPort, const addrSet_t& remAddrSet, connectDelegate_t conDelegate, closeDelegate_t closeDelegate)
{
    addr.locAddr.set_port_number(serverPort);

    acceptorMap_t::iterator accept_iter = FindAcceptor(addr);

    if (accept_iter == mAcceptorMap.end())
    {
#ifdef UNIT_TEST
        // FIXME - when this is validated, remove the ifdef unit test and 
        // use this block for all builds. The issue this fixes is when
        // this object is deleted, it deletes the map which then deletes
        // the acceptor_t even though the acceptor_t may be in use by
        // DpgPodgramConnectionHandler. 
        accept_iter = mAcceptorMap.insert(acceptorMap_t::value_type(addr,acceptorDelegatePair_t(acceptorSharedPtr_t(MakeAcceptor(), boost::bind(&acceptor_t::remove_reference, _1)), portDelegateMap_t(addr.remAddr, conDelegate, closeDelegate)))).first;
#else
        accept_iter = mAcceptorMap.insert(acceptorMap_t::value_type(addr,acceptorDelegatePair_t(acceptorSharedPtr_t(MakeAcceptor()), portDelegateMap_t(addr.remAddr, conDelegate, closeDelegate)))).first;
#endif
        acceptorSharedPtr_t acceptor(accept_iter->second.first);

        // add additional remote addresses
        portDelegateMap_t& portDelegateMap(accept_iter->second.second);
        BOOST_FOREACH(const ACE_INET_Addr& remAddr, remAddrSet)
        {
            portDelegateMap.push(remAddr, conDelegate, closeDelegate);
        }

        acceptor->SetAcceptNotificationDelegate(boost::bind(&CodgramBlockAcceptor::HandleAcceptNotification, this, _1, boost::ref(accept_iter->second.second)));

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

        // Bind acceptor to given interface name
        if (!addr.locIfName.empty())
            acceptor->BindToIfName(addr.locIfName);
    }
    else
    {
        // already listening, add new remote-based delegate
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

bool CodgramBlockAcceptor::Accept(DpgPodgramConnectionHandler& rawConnHandler, const AddrInfo& playlistAddr)
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

void CodgramBlockAcceptor::StopAll()
{
    // should only be called once before destruction
    BOOST_FOREACH(acceptorMap_t::value_type& item, mAcceptorMap)
    {
        item.second.first->reactor()->suspend_handler(item.second.first.get());
    }
}

///////////////////////////////////////////////////////////////////////////////

void CodgramBlockAcceptor::CloseAll()
{
    // should only be called once before destruction
    BOOST_FOREACH(acceptorMap_t::value_type& item, mAcceptorMap)
    {
        item.second.first->close();
    }
}

///////////////////////////////////////////////////////////////////////////////

bool CodgramBlockAcceptor::HandleAcceptNotification(DpgPodgramConnectionHandler& rawConnHandler, portDelegateMap_t& delegateMap)
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
    rawConnHandler.SetSerial(0xc0000000 | mSerial++);
    
    delegateMap.register_and_pop(remoteAddr, rawConnHandler);

    // notify our owner (the socket manager) 
    return mAcceptNotificationDelegate(rawConnHandler);
}

///////////////////////////////////////////////////////////////////////////////

CodgramBlockAcceptor::acceptorMap_t::iterator CodgramBlockAcceptor::FindAcceptor(const AddrInfo& addr)
{
    return mAcceptorMap.find(addr);
}

///////////////////////////////////////////////////////////////////////////////

CodgramBlockAcceptor::acceptorMap_t::iterator CodgramBlockAcceptor::EndAcceptor() 
{
    return mAcceptorMap.end();
}

///////////////////////////////////////////////////////////////////////////////
