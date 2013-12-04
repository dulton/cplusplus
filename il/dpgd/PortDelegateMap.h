/// @file
/// @brief TCP source address (including port) to connect/close delegates map
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _PORT_DELEGATE_MAP_H_
#define _PORT_DELEGATE_MAP_H_

#include <ace/INET_Addr.h>
#include <engine/AbstSocketManager.h> 

#include "DpgCommon.h"

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

struct AddrSansPortLess : public std::binary_function<ACE_INET_Addr, ACE_INET_Addr, bool>
{
    bool operator()(const ACE_INET_Addr& a1, const ACE_INET_Addr& a2) const 
    {   
        // taken and modified from ace/INET_Addr.inl
        if (a1.get_type() != a2.get_type())
        {
            return a1.get_type() < a2.get_type();
        }

        if (a1.get_type() == PF_INET6)
        {
            return memcmp(&static_cast<sockaddr_in6*>(a1.get_addr())->sin6_addr,
                          &static_cast<sockaddr_in6*>(a2.get_addr())->sin6_addr,
                          16) < 0;
        }

        return a1.get_ip_address() < a2.get_ip_address();
    }
};


///////////////////////////////////////////////////////////////////////////////

template <class REGISTERER>
class PortDelegateMap
{
public:
    typedef L4L7_ENGINE_NS::AbstSocketManager::connectDelegate_t connectDelegate_t;
    typedef L4L7_ENGINE_NS::AbstSocketManager::closeDelegate_t closeDelegate_t;
    typedef std::pair<connectDelegate_t, closeDelegate_t> delPair_t; 

    PortDelegateMap(const ACE_INET_Addr& remAddr, const connectDelegate_t& conDel, const closeDelegate_t& cloDel)
    {
        rootMap_t::iterator iter = mRootMap.insert(map_t::value_type(remAddr, delPair_t())).first;
        iter->second.first = conDel;
        iter->second.second = cloDel;

        mRoot.first = conDel;
        mRoot.second = cloDel;
    }

    void push(const ACE_INET_Addr& remAddr, const connectDelegate_t& conDel, const closeDelegate_t& cloDel)
    {
        // FIXME -- check for conflict?

        if (remAddr.get_port_number() == 0)
        {
            // Insert in root map (no port)
            rootMap_t::iterator iter = mRootMap.insert(rootMap_t::value_type(remAddr, delPair_t())).first;
            iter->second.first = conDel;
            iter->second.second = cloDel;
        }
        else
        {
            map_t::iterator iter = mMap.insert(map_t::value_type(remAddr, delPair_t())).first;
            iter->second.first = conDel;
            iter->second.second = cloDel;
        }
    }

    bool register_and_pop(const ACE_INET_Addr& remAddr, REGISTERER& reg, bool forceExact = false)
    {
        map_t::iterator iter = mMap.find(remAddr);
        if (iter != mMap.end())
        {
            reg.RegisterDelegates(iter->second.first, iter->second.second);
            mMap.erase(iter);
            return true;
        }
        else if (!forceExact)
        {
            // no exact match found, check the root
            rootMap_t::iterator rootIter = mRootMap.find(remAddr);
            if (rootIter != mRootMap.end())
            {
                reg.RegisterDelegates(rootIter->second.first, rootIter->second.second);
            }
            else
            {
                reg.RegisterDelegates(mRoot.first, mRoot.second);
            }
            return true;
        }

        return false;
    }

private:
    typedef std::map<ACE_INET_Addr, delPair_t> map_t; // addr + port
    typedef std::map<ACE_INET_Addr, delPair_t, AddrSansPortLess> rootMap_t; // addr only
    map_t mMap;
    rootMap_t mRootMap;
    delPair_t mRoot;
    
    friend class TestPortDelegateMap;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
