/// @file
/// @brief SIP utilities header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _SIP_UTILS_H_
#define _SIP_UTILS_H_

#include <sstream>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>

#include <ace/INET_Addr.h>
#include <any_iterator/any_iterator.hpp>
#include <boost/range/iterator_range.hpp>

#include "SipCommon.h"

DECL_SIP_NS

///////////////////////////////////////////////////////////////////////////////

namespace SipUtils
{
    inline void SipAddressToInetAddress(const Sip_1_port_server::Ipv4Address_t& from, ACE_INET_Addr& to) { to.set_address(reinterpret_cast<const char *>(from.address), 4, 0, 0); }
    inline void SipAddressToInetAddress(const Sip_1_port_server::Ipv6Address_t& from, ACE_INET_Addr& to) { to.set_address(reinterpret_cast<const char *>(from.address), 16, 0, 0); }

    inline const ACE_INET_Addr SockAddressToInetAddress(const struct sockaddr_in *sin) { return ACE_INET_Addr(sin, sizeof(struct sockaddr_in)); }
    inline const ACE_INET_Addr SockAddressToInetAddress(const struct sockaddr_in6 *sin6) { return ACE_INET_Addr(reinterpret_cast<const sockaddr_in *>(sin6), sizeof(struct sockaddr_in6)); }
    
    const std::string InetAddressToString(const ACE_INET_Addr& addr);

    typedef boost::iterator_range<IteratorTypeErasure::any_iterator<std::string, std::input_iterator_tag, const std::string&, ptrdiff_t> > string_range_t;
    const std::string BuildDelimitedList(const string_range_t& list, const std::string& delimiter);

    static inline bool is_default_sip_port(uint16_t port) {
      return (port==0 || port==DEFAULT_SIP_PORT_NUMBER);
    }

};

///////////////////////////////////////////////////////////////////////////////

END_DECL_SIP_NS

#endif
