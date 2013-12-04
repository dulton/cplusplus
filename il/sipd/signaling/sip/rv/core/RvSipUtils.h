/// @file
/// @brief SIP core package utils header.
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef RVSIPUTILS_H_
#define RVSIPUTILS_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>

#include "RvSipHeaders.h"

#define MAX_LENGTH_OF_REALM_NAME (256+1)
#define MAX_LENGTH_OF_USER_NAME (256+1)
#define MAX_LENGTH_OF_PWD (256+1)
#define MAX_LENGTH_OF_NONCE (256+1)
#define MAX_LENGTH_OF_METHOD (32+1)
#define MAX_LENGTH_OF_PRIVACY (16+1)
#define MAX_LENGTH_OF_DIGEST_RESPONSE (32+1)
#define MAX_LENGTH_OF_DIGEST (20)
#define MAX_LENGTH_OF_URL (1024+1)

#define RVSIPTOKENPASTE(x, y) x ## y 
#define RVSIPTOKENPASTE2(x, y) RVSIPTOKENPASTE(x, y) 
#define RVSIPLOCK(L) RvSipUtils::Guard RVSIPTOKENPASTE2(av,__LINE__)(L)

//#define RVSIPDEBUG 1

#if defined(RVSIPDEBUG)
extern "C" void rtpprintf(const char* format,...);
#define RVSIPPRINTF rtpprintf
#endif

#if !defined(RVSIPPRINTF)
#define RVSIPPRINTF(...)
#endif

extern "C" void printfToNull(const char* format,...);

#if 0
#define PRINTF RVSIPPRINTF
#define TRACE(s) PRINTF("%s:%s:%s[%d]\n", __FILE__,__FUNCTION__,s,__LINE__)
#define ENTER() \
	do { \
		PRINTF("ENTER %s:%s (line=%d)\n", __FILE__,__FUNCTION__,__LINE__); \
	} while (0)
#define RET \
	do { \
		PRINTF("LEAVE %s:%s (line=%d)\n", __FILE__,__FUNCTION__, __LINE__); \
		return; \
	} while(0)
#define RETCODE(c) \
	do { \
		PRINTF("LEAVE %s:%s (line=%d)(%d)\n", __FILE__,__FUNCTION__, __LINE__, c); \
		return (c); \
	} while(0)
#else
#define TRACE(s)
#define ENTER()
#define RET 	   return
#define RETCODE(c) return (c)
#endif 

BEGIN_DECL_RV_SIP_ENGINE_NAMESPACE

namespace RvSipUtils {

  RvStatus SIP_RemoveAuthHeader(RvSipRegClientHandle hRegClient);
  RvStatus SIP_RemoveAuthHeader(RvSipCallLegHandle hCallLeg);
  
  int SIP_AddrUrlGetUrlHostPort(RvSipAddressHandle hAddr, 
				char* URL, int urlSize, unsigned int *lenUrl, 
				char* addrBuffer, int addrBufferSize, unsigned int *actAddrBufferSize, 
				int *port);
  RvStatus SIP_AddrUrlGetUser(RvSipAddressHandle hAddr,
			      char*           URL,
			      unsigned int    urlSize,
			      unsigned int*   actualLen);
  RvStatus SIP_PhoneNumberExtract(RvSipPartyHeaderHandle hHeader,char *buf, int buflen);
  RvStatus SIP_ToPhoneNumberExtract(RvSipMsgHandle hMsg,char *buf, int buflen);
  RvStatus SIP_FromPhoneNumberExtract(RvSipMsgHandle hMsg,char *buf, int buflen);
  
  RvStatus SIP_AddrUrlGetUrl( RvSipAddressHandle hAddr, char* pAddr, int size);
  RvStatus SIP_AddrUrlMsgGetUrl( RvSipMsgHandle hMsg, char* pAddr, int size, bool useFrom );
  
  RvStatus SIP_AddrUrlTranscGetUrl( RvSipTranscHandle hTransaction, char* pAddr, int size, bool useFrom );
  RvStatus SIP_AddrUrlGetHost(RvSipAddressHandle hAddr,
			      char* addrBuffer, 
			      unsigned int addrBufferSize,
			      unsigned int*          actualLen);
  int SIP_AddrUrlGetPortNum(RvSipAddressHandle hAddr);
  RvBool SIP_AddrUrlGetLrParam(RvSipAddressHandle hAddr);
  RvBool SIP_AddrIsEqual(RvSipAddressHandle hAddress,RvSipAddressHandle hOtherAddress);

  std::string getFQDN(const std::string& host, const std::string& domain);
  int HexStringToKey (const char * hexString, uint8_t *key, int maxLength);
  bool isEmptyIPAddress(const std::string& ipstring);
  bool isEmptyIPAddress(const char *ipstring);

  class Lock;

  class Locker {
  public:
    Locker();
    virtual ~Locker();
    int lock() const;
    int unlock() const;
  private:
    Locker(const Locker& other);
    const Locker& operator=(const Locker& other);
    const Lock* mLock;
  };

  class Guard {
  public:
    explicit Guard(const Locker& locker) : mLocker(locker) {
      mLocker.lock();
    }
    virtual ~Guard() { mLocker.unlock(); } 
  private:
    Guard(const Guard& other);
    const Guard& operator=(const Guard& other);
    const Locker& mLocker;
  };
};

END_DECL_RV_SIP_ENGINE_NAMESPACE;

#endif /*RVSIPUTILS_H_*/
