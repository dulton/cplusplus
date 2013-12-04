/// @file
/// @brief Address information structure
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ADDR_INFO_H_
#define _ADDR_INFO_H_

#include <ace/INET_Addr.h>
#include <boost/functional/hash.hpp>
#include <ostream>

#include "DpgCommon.h"

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

struct AddrInfo
{
    std::string locIfName;
    ACE_INET_Addr locAddr;
    ACE_INET_Addr remAddr;
    bool hasRem;
}; 

///////////////////////////////////////////////////////////////////////////////

struct AddrInfoSrcHash : std::unary_function<AddrInfo, std::size_t> 
{
    size_t operator()(const AddrInfo& x) const
    {
        size_t seed = 0;
        boost::hash_combine(seed, x.locIfName);
        boost::hash_combine(seed, x.locAddr.hash());
        return seed;
    };
};

///////////////////////////////////////////////////////////////////////////////

struct AddrInfoSrcEqual : public std::binary_function<AddrInfo, AddrInfo, bool>
{
    bool operator()(const AddrInfo& a1, const AddrInfo& a2) const { return a1.locIfName == a2.locIfName && a1.locAddr == a2.locAddr; }
};

///////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const AddrInfo& addr);

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
