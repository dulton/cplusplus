/// @file
/// @brief VoIP Utils
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <sys/types.h>
#include <sys/time.h>
#if defined YOCTO_I686
#include <time.h>
#else
#include <posix_time.h>
#endif
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/sysinfo.h>
#include <net/if.h>

#include <cstdlib>
#include <sstream>

#include "rvexternal.h"

#include "VoIPUtils.h"

#ifdef UNIT_TEST
#define VOIPCLOCK CLOCK_MONOTONIC
#else
#if defined YOCTO_I686
#define VOIPCLOCK CLOCK_MONOTONIC
#else
#define VOIPCLOCK CLOCK_MONOTONIC_HR
#endif
#endif

///////////////////////////////////////////////////////////////////////////////

bool operator==(const ACE_INET_Addr& lhs, const ACE_INET_Addr& rhs) {
  //this code is here only to produce ambig. compile-time error when two ACE_INET_Addr are compared.
  return lhs.operator==(rhs);
}

///////////////////////////////////////////////////////////////////////////////

DECL_VOIP_NS

namespace VoIPUtils {

  ///////////////////////////////////////////////////////////////////////////////
  
  static bool _SrandomInit = false;
  
  ///////////////////////////////////////////////////////////////////////////////
  
  uint32_t MakeRandomInteger(void)
  {
    if (!_SrandomInit)
      {
        struct timeval tv;
        gettimeofday(&tv, 0);
        
        srandom(static_cast<unsigned int>(tv.tv_usec ^ tv.tv_sec));
        _SrandomInit = true;
      }
    
    return static_cast<uint32_t>(::random());
  }
  void calculateCPUNumber(unsigned int &nbrofCpus) {
    
    nbrofCpus=0;
    
    FILE* f=fopen("/proc/cpuinfo","r");
    if(f) {
      char buffer[1024];
      const char* prefix="processor";
      int plen=strlen(prefix);
      while(fgets(buffer,sizeof(buffer)-1,f)) {
	if(!strncmp(buffer,prefix,plen)) nbrofCpus++;
      }
      fclose(f);
    }
    
    if(nbrofCpus<1) nbrofCpus=1;  
  }

  bool isIntelCPU() {
      bool ret = false;

      FILE* f=fopen("/proc/cpuinfo","r");
      if(f) {
          char buffer[1024];
          const char* check="Intel";
          while(fgets(buffer,sizeof(buffer)-1,f)) {
              if(strstr(buffer,check)) {
                  ret = true;
                  break;
              }

          }
          fclose(f);
      }
      return ret;
  }
  unsigned int SipCallCapacity(void){
      struct sysinfo sinfo;
      unsigned int max_channels = 1024;
      unsigned int numberOfCpus = 1;
      calculateCPUNumber(numberOfCpus);
      if(!sysinfo(&sinfo)) {
          uint64_t freemem = (uint64_t)((uint64_t)sinfo.freeram*(uint64_t)sinfo.mem_unit);
          if((freemem/2)>1500000000) {//4G
              max_channels = 16384;//65536;
          }else if(freemem>2000000000) {
              max_channels = 16384;//32768;
          }else if(freemem>1000000000) {
              if(numberOfCpus<2) 
                  max_channels = 8192;
              else
                  max_channels = 16384;
          }else if(freemem>400000000){ 
              max_channels = 4096;
          }else if(freemem>250000000){ 
              max_channels = 2048;
          }
      }
      return max_channels;
  }
  int ExtendsFdLimitation(unsigned int nbrofChannels){
      struct rlimit r;
      unsigned int max_channels = nbrofChannels*5;
      if(max_channels == 0)
          max_channels = SipCallCapacity()*5;

      if(getrlimit(RLIMIT_NOFILE,&r) < 0)
          return -1;
      if(r.rlim_cur < max_channels)
          r.rlim_cur = max_channels;
      if(r.rlim_max < max_channels)
          r.rlim_max = max_channels;
      if(setrlimit(RLIMIT_NOFILE,&r) < 0)
          return -1;
      return 0;
  }
  
  ///////////////////////////////////////////////////////////////////////////////
  
  const std::string MakeRandomHexString(size_t length)
  {
    std::string hexStr;
    hexStr.reserve(length);
    
    uint32_t num = 0;
    for (size_t i = 0; i < length; i++)
      {
        if (!_SrandomInit)
	  {
            struct timeval tv;
            gettimeofday(&tv, 0);
	    
            srandom(static_cast<unsigned int>(tv.tv_usec ^ tv.tv_sec));
            _SrandomInit = true;
	  }
	
        if ((i % 8) == 0)
	  num = static_cast<uint32_t>(::random());
	
        char nibble = static_cast<char>(num & 0x0f);
        hexStr.append(1, static_cast<char>((nibble > 9) ? ('a' + (nibble - 10)) : ('0' + nibble)));
        num >>= 4;
      }
    
    return hexStr;
  }
  
  ///////////////////////////////////////////////////////////////////////////////

  uint64_t getMilliSeconds() {
    return _getMilliSeconds();
  }

  int mksSleep(uint32_t mks) {
    
    struct timespec tp={0,0};
    
    clock_gettime(VOIPCLOCK, &tp);
    
    tp.tv_nsec+=mks*1000;
    if(tp.tv_nsec>=1000000000) {
      tp.tv_nsec-=1000000000;
      tp.tv_sec+=1;
    }
    
    return clock_nanosleep(VOIPCLOCK, TIMER_ABSTIME, &tp, 0);
  }
  void getInfMac(std::string ifname,std::string& mac){
      mac.clear();
      if(ifname.empty())
          return;
      int fd;
      struct ifreq ifr;

      fd = socket(AF_INET,SOCK_DGRAM,0);
      if(-1 != fd){
          ifr.ifr_addr.sa_family = AF_INET;
          strncpy(ifr.ifr_name,ifname.c_str(),IFNAMSIZ-1);
          ioctl(fd,SIOCGIFHWADDR,&ifr);
          close(fd);
          char _mac[16] = {0};
          snprintf(_mac,sizeof(_mac),"%02x%02x%02x%02x%02x%02x",
                  (uint8_t)ifr.ifr_hwaddr.sa_data[0],
                  (uint8_t)ifr.ifr_hwaddr.sa_data[1],
                  (uint8_t)ifr.ifr_hwaddr.sa_data[2],
                  (uint8_t)ifr.ifr_hwaddr.sa_data[3],
                  (uint8_t)ifr.ifr_hwaddr.sa_data[4],
                  (uint8_t)ifr.ifr_hwaddr.sa_data[5]);
          mac = _mac;
      }
  }

  ///////////////////////////////////////////////////////////////////////////////
  
  sip_1_port_server::Ipv4Address_t& StringToSipAddress(const std::string& addrStr, sip_1_port_server::Ipv4Address_t& addr)
  {
    memset(addr.address, 0, 4);
    inet_pton(AF_INET, addrStr.c_str(), addr.address);
    return addr;
  }
  
  ///////////////////////////////////////////////////////////////////////////////
  
  sip_1_port_server::Ipv6Address_t& StringToSipAddress(const std::string& addrStr, sip_1_port_server::Ipv6Address_t& addr)
  {
    memset(addr.address, 0, 16);
    inet_pton(AF_INET6, addrStr.c_str(), addr.address);
    return addr;
  }
  
} //namespace VoIPUtils

END_DECL_VOIP_NS
