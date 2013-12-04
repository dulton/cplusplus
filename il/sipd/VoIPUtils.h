/// @file
/// @brief VOIP common utility header file
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _VOIP_UTILS_H_
#define _VOIP_UTILS_H_

///////////////////////////////////////////////////////////////////////////////

#include <netinet/in.h>

#include <ace/INET_Addr.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Guard_T.h>

#include "VoIPCommon.h"

#include <stdint.h>

#include <string>

///////////////////////////////////////////////////////////////////////////////

#define VoIPGuard(A) ACE_Guard<ACE_Recursive_Thread_Mutex> __lock__##__LINE__(A)

DECL_VOIP_NS

namespace VoIPUtils {

  class AddrKey {

  private:
    ACE_INET_Addr addr;
    unsigned int ifIndex;

  public:

    explicit AddrKey(const ACE_INET_Addr &a, unsigned int index = 0) : addr(a), ifIndex(index) {
      addr.set_port_number(0);
    }

    static bool equal(const ACE_INET_Addr& lhs, const ACE_INET_Addr& rhs, bool use_port_for_comparision = true) {
      
      if(&lhs==&rhs) return true;
      
      if (lhs.get_type() != rhs.get_type()) return false;
      
      if (use_port_for_comparision && lhs.get_port_number() != rhs.get_port_number()) return false;
      
      switch (lhs.get_type()) {
      case AF_INET:
	return memcmp(&reinterpret_cast<const struct sockaddr_in *>(lhs.get_addr())->sin_addr, &reinterpret_cast<const struct sockaddr_in *>(rhs.get_addr())->sin_addr, 4) == 0;
	
      case AF_INET6:
	return memcmp(&reinterpret_cast<const struct sockaddr_in6 *>(lhs.get_addr())->sin6_addr, &reinterpret_cast<const struct sockaddr_in6 *>(rhs.get_addr())->sin6_addr, 16) == 0;
	
      default:
	return false;
      }
    }

    unsigned int hash() const {
      return ((unsigned int)(addr.hash()) + ifIndex);
    }

    bool operator==(const AddrKey& other) const {
      if(this == &other) return true;
      ACE_INET_Addr otheraddr = other.addr;
      otheraddr.set_port_number(0);
      return equal(this->addr,otheraddr) && (this->ifIndex == other.ifIndex);
    }

    /**
     * Returns @c true if @c this is less than @a rhs.  In this context,
     * "less than" is defined in terms of IP address and TCP port
     * number.  This operator makes it possible to use @c ACE_INET_Addrs
     * in STL maps.
     */
    bool operator < (const AddrKey &rhs) const {
      if(this==&rhs) return false;
      if(ifIndex<rhs.ifIndex) return true;
      if(ifIndex>rhs.ifIndex) return false;
      return addr < rhs.addr;
    }
  };

  typedef ACE_Recursive_Thread_Mutex MutexType;

  uint64_t getMilliSeconds(void);
  int mksSleep(uint32_t mks);
  int ExtendsFdLimitation(unsigned int nbrofChannels);
  unsigned int SipCallCapacity(void);
  void calculateCPUNumber(unsigned int &nbrofCpus);
  bool isIntelCPU();
    
  uint32_t MakeRandomInteger(void);
  const std::string MakeRandomHexString(size_t length);
  
  static inline bool time_after64(uint64_t a, uint64_t b) { return ((int64_t)(b) - (int64_t)(a) < 0); }
  static inline bool time_before64(uint64_t a, uint64_t b) { return time_after64(b,a); }
  static inline uint64_t time_minus64(uint64_t a, uint64_t b) { return ((uint64_t)(a)-(uint64_t)(b)); }
  static inline uint64_t time_plus64(uint64_t a, uint64_t b) { return ((uint64_t)(a)+(uint64_t)(b)); }
  void getInfMac(std::string ifname,std::string& mac);

  sip_1_port_server::Ipv4Address_t& StringToSipAddress(const std::string& addrStr, sip_1_port_server::Ipv4Address_t& addr);
  sip_1_port_server::Ipv6Address_t& StringToSipAddress(const std::string& addrStr, sip_1_port_server::Ipv6Address_t& addr);
}

END_DECL_VOIP_NS

///////////////////////////////////////////////////////////////////////////////

#endif //_VOIP_UTILS_H_

