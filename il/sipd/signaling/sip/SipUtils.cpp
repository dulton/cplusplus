/// @file
/// @brief SIP utilities implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <string>

#include <arpa/inet.h>
#include <sys/time.h>

#include "SipUtils.h"

DECL_SIP_NS

namespace SipUtils
{

///////////////////////////////////////////////////////////////////////////////

const std::string InetAddressToString(const ACE_INET_Addr& addr)
{
    char addrStr[INET6_ADDRSTRLEN];
    return std::string(addr.get_host_addr(addrStr, sizeof(addrStr)));
}

///////////////////////////////////////////////////////////////////////////////

const std::string BuildDelimitedList(const string_range_t& list, const std::string& delimiter)
{
    std::ostringstream oss;
    std::copy(list.begin(), list.end(), std::ostream_iterator<std::string>(oss, delimiter.c_str()));
    std::string temp = oss.str();
    if (!temp.empty())
        temp.resize(temp.size() - delimiter.size());  // strips trailing delimiter
    return temp;
}

///////////////////////////////////////////////////////////////////////////////

};

END_DECL_SIP_NS
