/// @file
/// @brief SIP core package utilities.
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <pthread.h>

#include <string>

#include "RvSipUtils.h"

BEGIN_DECL_RV_SIP_ENGINE_NAMESPACE namespace RvSipUtils {

  class Lock {
  public:
    Lock() {
      pthread_mutexattr_t ma;
      if(pthread_mutexattr_init(&ma)!=0) throw std::string("Mutex cannot be initialized 1");
      if(pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE)!=0) {pthread_mutexattr_destroy(&ma); throw std::string("Mutex cannot be initialized 2");}
      if(pthread_mutex_init(&mutex, &ma)!=0) {pthread_mutexattr_destroy(&ma); throw std::string("Mutex cannot be initialized 3");}
      pthread_mutexattr_destroy(&ma);
    }
    virtual ~Lock() {
      pthread_mutex_destroy(&mutex);
    }
    int lock() const {
      return pthread_mutex_lock(&mutex);
    }
    int unlock() const {
      return pthread_mutex_unlock(&mutex);
    }
  private:
    Lock(const Lock& other) {}
    const Lock& operator=(const Lock& other) { return *this; }
    mutable pthread_mutex_t mutex;
  };

  Locker::Locker() : mLock(new Lock()) {
    ;
  }
  
  Locker::~Locker() {
    delete mLock;
  }

  int Locker::lock() const {
    return mLock->lock();
  }

  int Locker::unlock() const {
    return mLock->unlock();
  }
  
  int SIP_AddrUrlGetUrlHostPort(RvSipAddressHandle hAddr, 
				char* URL, int urlSize, unsigned int *lenUrl, 
				char* addrBuffer, int addrBufferSize, unsigned int *actAddrBufferSize, 
				int *port) {
    
    // Find matching channel and alias from given addr.
    // Return cctUH. If not found, then cctUH < 0.
    
    RV_Status rv=RV_OK;
    unsigned int lenUrlTmp=0;
    int portTmp=0;
    char addrBufferTmp[MAX_LENGTH_OF_URL];
    char URLTmp[MAX_LENGTH_OF_URL];
    unsigned int actAddrBufferSizeTmp=0;
    RvSipAddressType addrType;
    
    if(!hAddr) return -1;
    
    addrType = RvSipAddrGetAddrType(hAddr);
    
    if(!URL) {
      URL=URLTmp;
      urlSize=sizeof(URLTmp)-1;
    }
    if(!lenUrl) lenUrl=&lenUrlTmp;
    if(!port) port=&portTmp;
    if(!actAddrBufferSize) actAddrBufferSize=&actAddrBufferSizeTmp;
    if(!addrBuffer) {
      addrBuffer=addrBufferTmp;
      addrBufferSize=sizeof(addrBufferTmp)-1;
    }
    
    memset(URL,0,urlSize);
    memset(addrBuffer,0,addrBufferSize);
    
    if(addrType == RVSIP_ADDRTYPE_ABS_URI) {
      // get URL=username@host[:port]
      RvSipAddrAbsUriGetIdentifier(hAddr, URL, urlSize, lenUrl);
      
      {        
	char* p = strchr(URL, '@');
	
	*actAddrBufferSize=0;
	addrBuffer[0]=0;
	
	if(p && ++p) {
	  // copy host to addrBuffer
	  strncpy(addrBuffer, p, addrBufferSize-1);
	  
	  // remove the trailing port if exists
	  p = strchr(addrBuffer, ':');
	  if(p) {  
	    *p = '\0';
	    p++;
	    
	    // if there is no known port, then take the port from the URL
	    if(*p && *port < 0) {
	      char* tmp = strchr(p, ';');  
	      if(tmp)  *tmp = '\0';         // chop off the rest of URI params (if exist)
	      
	      // get the port value.
	      *port = atoi(p);
	    }
	  }
	}
	
	*actAddrBufferSize=strlen(addrBuffer)+1;
      }
    } else if(addrType == RVSIP_ADDRTYPE_URL) {
      // Get URL name.
      rv = RvSipAddrUrlGetUser( hAddr, URL, urlSize, lenUrl );
      if ( rv != RV_Success ) {
	return -1;
      }
      
      if(*lenUrl>1) {
	(*lenUrl)--;
	URL[*lenUrl] = '@';
	rv = RvSipAddrUrlGetHost( hAddr, &URL[*lenUrl + 1], urlSize-*lenUrl-1-1, actAddrBufferSize );
      } else {
	rv = RvSipAddrUrlGetHost( hAddr, URL, urlSize, lenUrl );
      }
      
      // Get host
      RvSipAddrUrlGetHost(hAddr,addrBuffer,addrBufferSize,actAddrBufferSize);
      
      // Get port
      if(*port<1) {
	*port = RvSipAddrUrlGetPortNum(hAddr);
      }
    } else if(addrType == RVSIP_ADDRTYPE_TEL) {
      
      char Context[1024];
      unsigned int len=0;
      
      rv = RvSipAddrTelUriGetContext( hAddr, Context, urlSize, &len );
      
      if(len > 0) {
	
	char PhoneNum[1024];
	
	rv = RvSipAddrTelUriGetPhoneNumber( hAddr, PhoneNum, urlSize, &len );
	
	snprintf(URL, urlSize, "%s;phone-context=%s", PhoneNum, Context);
	
	*lenUrl=strlen(URL);
	
      } else {
	
	URL[0] = '+';
	rv = RvSipAddrTelUriGetPhoneNumber( hAddr, &URL[1], urlSize, lenUrl );
      }
      
      if(rv != RV_Success) {
	return -1;
      }
    }
    
    if(port && *port<0) *port=0;
    
    return 0;
  }
  
  RvStatus SIP_AddrUrlGetUser(RvSipAddressHandle hAddr,
			      char*           URL,
			      unsigned int    urlSize,
			      unsigned int*   actualLen) {
    
    if(hAddr && URL) {
      
      RvSipAddressType addrType;
      
      addrType = RvSipAddrGetAddrType(hAddr);
      
      if(addrType == RVSIP_ADDRTYPE_ABS_URI) {
	
	return SIP_AddrUrlGetUrlHostPort(hAddr, URL, urlSize, actualLen, NULL, 0, NULL, NULL);
	
      } else if(addrType == RVSIP_ADDRTYPE_URL) {
	
	return RvSipAddrUrlGetUser(hAddr, URL, urlSize, actualLen);
	
      } else if(addrType == RVSIP_ADDRTYPE_TEL) {
	
	return RvSipAddrTelUriGetPhoneNumber( hAddr, URL, urlSize, actualLen );
	
      } else {
	
	return RV_ERROR_BADPARAM;
      }
      
    } else {
      
      return RV_ERROR_NULLPTR;
    }
  }
  
  RvStatus SIP_PhoneNumberExtract(RvSipPartyHeaderHandle hHeader,char *buf, int buflen)
  {
    RvSipAddressHandle     sipUrl;
    RV_Status rv=RV_OK;
    unsigned int len=0;
    
    if(!buf) return rv;
    
    *buf = '\0';
    
    /* Get the Sip-Url of the transaction's From header */
    sipUrl = RvSipPartyHeaderGetAddrSpec(hHeader);
    if (NULL == sipUrl) {
      return RV_ERROR_BADPARAM;
    }
    
    return SIP_AddrUrlGetUser(sipUrl,buf,buflen, &len);
  }
  
  RvStatus SIP_ToPhoneNumberExtract(RvSipMsgHandle hMsg,char *buf, int buflen)
  {
    RvSipPartyHeaderHandle hToHeader;
    *buf = '\0';
    hToHeader = RvSipMsgGetToHeader(hMsg);
    if (NULL == hToHeader) {
      return RV_ERROR_BADPARAM;
    }
    return SIP_PhoneNumberExtract(hToHeader, buf, buflen);
  }
  
  RvStatus SIP_FromPhoneNumberExtract(RvSipMsgHandle hMsg,char *buf, int buflen)
  {
    RvSipPartyHeaderHandle hFromHeader;
    *buf = '\0';
    hFromHeader = RvSipMsgGetFromHeader(hMsg);
    if (NULL == hFromHeader) {
      return RV_ERROR_BADPARAM;
    }
    return SIP_PhoneNumberExtract(hFromHeader, buf, buflen);
  }
  
  //=============================================================================
  
  RvStatus SIP_AddrUrlGetUrl( RvSipAddressHandle hAddr, char* URL, int urlSize)
  {
    // Find matching channel and alias from given addr.
    // Return cctUH. If not found, then cctUH < 0.
    
    RV_Status rv=RV_OK;
    
    if(hAddr && URL && urlSize>0) {
      
      unsigned int lenUrl=0;
      RvSipAddressType addrType;
      
      addrType = RvSipAddrGetAddrType(hAddr);
      
      memset(URL,0,urlSize);
      
      if(addrType == RVSIP_ADDRTYPE_ABS_URI) {
	// get URL=username@host[:port]
	RvSipAddrAbsUriGetIdentifier(hAddr, URL, urlSize, &lenUrl);
	
	{     
	  // remove the trailing port if exists
	  char* p = strchr(URL, ':');
	  if(p) *p = '\0';
	}
      } else if(addrType == RVSIP_ADDRTYPE_URL) {
	// Get URL name.
	rv = RvSipAddrUrlGetUser( hAddr, URL, urlSize, &lenUrl );
	if ( rv != RV_Success ) {
	  URL[0]=0;
	}
	
	if(lenUrl>1) {
	  lenUrl--;
	  URL[lenUrl] = '@';
	  rv = RvSipAddrUrlGetHost( hAddr, &URL[lenUrl + 1], urlSize-lenUrl-1-1, &lenUrl );
	} else {
	  rv = RvSipAddrUrlGetHost( hAddr, URL, urlSize, &lenUrl );
	}
	
      } else if(addrType == RVSIP_ADDRTYPE_TEL) {
	
	char Context[1024];
	unsigned int len=0;
	
	rv = RvSipAddrTelUriGetContext( hAddr, Context, sizeof(Context)-1, &len );
	
	if(len > 0) {
	  
	  char PhoneNum[1024];
	  
	  rv = RvSipAddrTelUriGetPhoneNumber( hAddr, PhoneNum, urlSize, &len );
	  snprintf(URL, urlSize, "%s;phone-context=%s", PhoneNum, Context);
	} else {
	  URL[0] = '+';
	  rv = RvSipAddrTelUriGetPhoneNumber( hAddr, &URL[1], urlSize, &len );
	}      
      }
    }    
    
    return rv;
  }
  
  RvStatus SIP_AddrUrlMsgGetUrl( RvSipMsgHandle hMsg, char* pAddr, int size, bool useFrom ){
    // Get user Address Of Record from To header.
    // Return 0 for success, -1 for failure.
    RvSipPartyHeaderHandle hHeader;
    RvSipAddressHandle addr;
    
    // Get user AOR from To header.
    
    if(useFrom) hHeader =  RvSipMsgGetFromHeader( hMsg );
    else hHeader =  RvSipMsgGetToHeader( hMsg );
    
    if ( !hHeader ) {
      return RV_ERROR_UNKNOWN;
    }
    
    addr = RvSipPartyHeaderGetAddrSpec( hHeader );
    if( !addr ) {
      return RV_ERROR_UNKNOWN;
    }
    
    return SIP_AddrUrlGetUrl( addr, pAddr, size );
  }
  
  RvStatus SIP_AddrUrlTranscGetUrl( RvSipTranscHandle hTransaction, char* pAddr, int size, bool useFrom )
  {
    // Get user Address Of Record from To header.
    // Return 0 for success, -1 for failure.
    RvStatus   rv=RV_OK;
    RvSipPartyHeaderHandle hHeader;
    RvSipAddressHandle addr;
    
    // Get user AOR from To header.
    
    if(useFrom) rv =  RvSipTransactionGetFromHeader( hTransaction, &hHeader );
    else rv =  RvSipTransactionGetToHeader( hTransaction, &hHeader );
    
    if ( rv != RV_OK ) {
      return rv;
    }
    
    addr = RvSipPartyHeaderGetAddrSpec( hHeader );
    if( !addr ) {
      return RV_ERROR_UNKNOWN;
    }
    
    return SIP_AddrUrlGetUrl( addr, pAddr, size );
  }
  
  RvStatus SIP_AddrUrlGetHost(RvSipAddressHandle hAddr,
			      char* addrBuffer, 
			      unsigned int addrBufferSize,
			      unsigned int*          actualLen) {
    
    if(hAddr) {
      
      RvSipAddressType addrType;
      
      addrType = RvSipAddrGetAddrType(hAddr);
      
      if(addrType == RVSIP_ADDRTYPE_ABS_URI) {
	
	return SIP_AddrUrlGetUrlHostPort(hAddr, NULL, 0, NULL, addrBuffer, addrBufferSize, actualLen, NULL);
	
      } else if(addrType == RVSIP_ADDRTYPE_URL) {
	
	return RvSipAddrUrlGetHost(hAddr, addrBuffer, addrBufferSize, actualLen);
	
      } else {
	
	return RV_ERROR_BADPARAM;
      }
      
    } else {
      
      return RV_ERROR_NULLPTR;
    }
  }
  
  int SIP_AddrUrlGetPortNum(RvSipAddressHandle hAddr) {
    
    int port=-1;
    
    if(hAddr) {
      
      RvSipAddressType addrType;
      
      addrType = RvSipAddrGetAddrType(hAddr);
      
      if(addrType == RVSIP_ADDRTYPE_ABS_URI) {
	
	SIP_AddrUrlGetUrlHostPort(hAddr, NULL, 0, NULL, NULL, 0, NULL, &port);
	
	if(port<0) {
	  static int port_error_reported=0;
	  if(!port_error_reported) {
	    port_error_reported=1;
	  }
	  port=0;
	}
	
      } else if(addrType == RVSIP_ADDRTYPE_URL) {
	
	port = RvSipAddrUrlGetPortNum(hAddr); 
	
	if(port<0) {
	  static int port_error_reported=0;
	  if(!port_error_reported) {
	    port_error_reported=1;
	  }
	  port=0;
	}
	
      }
    }
    
    if(port<0) {
      static int port_error_reported=0;
      if(!port_error_reported) {
	port_error_reported=1;
      }
      port=0;
    }
    
    return port;
  }
  
  RvBool SIP_AddrUrlGetLrParam(RvSipAddressHandle hAddr) {
    
    if(hAddr) {
      
      RvSipAddressType addrType;
      
      addrType = RvSipAddrGetAddrType(hAddr);
      
      if(addrType == RVSIP_ADDRTYPE_ABS_URI) {
	
	char URL[MAX_LENGTH_OF_URL];
	unsigned int tmp;
	
	memset(URL,0,sizeof(URL));
	
	// get URL=username@host[:port]
	RvSipAddrAbsUriGetIdentifier(hAddr, URL, sizeof(URL)-1, &tmp);
	
	return (strstr(URL,";lr")!=NULL);
	
      } else {      
	return RvSipAddrUrlGetLrParam(hAddr); 
      }
      
    }
    
    return RV_FALSE;
  }
  
  RvBool SIP_AddrIsEqual(RvSipAddressHandle hAddress,RvSipAddressHandle hOtherAddress) {
    
    return (hAddress==hOtherAddress) || RvSipAddrIsEqual(hAddress,hOtherAddress);
  }
  bool isEmptyIPAddress(const std::string& ipstring){
      return (ipstring.empty() || ipstring == "0.0.0.0" || ipstring == "::" || ipstring == "[::]");
  }
  bool isEmptyIPAddress(const char *ipstring){
      
      return (!ipstring 
              || !ipstring[0]  
              || !strcmp(ipstring,"0.0.0.0")
              || !strcmp(ipstring,"::") 
              || !strcmp(ipstring,"[::]"));
  }

  std::string getFQDN(const std::string& host, const std::string& domain) {

    if(host.empty()){
      if(domain.empty()){
	return "";
      } else return domain;
    } else if(domain.empty()) {
      return host;
    } else {
      size_t hsize=host.size();
      size_t dsize=domain.size();
      if(hsize>=dsize) {
	std::string s = host.substr(hsize-dsize);
	if(s == domain) return host;
      }
      return host+"."+domain;
    }

  }
  int HexStringToKey (const char * hexString, uint8_t *key, int maxLength)
  {
      const char    * pHex = hexString;
      unsigned char * pKey = key;
      int             length = 0;
      unsigned char   digit;

      while (*pHex && length < maxLength)
      {
          digit = (unsigned char)*pHex++;
          if (digit < '0') {
              return -1;
          } else if (digit <= '9') {
              digit -= '0';
          } else {
              digit = tolower(digit);
              if (digit >= 'a' && digit <= 'f') {
                  digit -= ('a' - 10);
              } else {
                  return -1;
              }
          }

          *pKey = digit << 4;  // left digit

          if (*pHex)
          {
              digit = (unsigned char)*pHex++;
              if (digit < '0') {
                  return -1;
              } else if (digit <= '9') {
                  digit -= '0';
              } else {
                  digit = tolower(digit);
                  if (digit >= 'a' && digit <= 'f') {
                      digit -= ('a' - 10);
                  } else {
                      return -1;
                  }
              }

              *pKey |= digit;   // right digit
              ++pKey;
          }
          // else last byte is incomplete - left digit only
          ++length;
      }

      return length;
  }


  
  RvStatus SIP_RemoveAuthHeader(RvSipRegClientHandle hRegClient) {

    RvStatus rv = RV_OK;

    // we get in here because the registrar told us that our previous request either doesn't have an 
    // authorization header or has an invalid authorization header.
    // If the previous request contains an invalid authorization header, we should remove the authentication 
    // header based upon which we had generated the invalid authorization header.
    // The removal of an authentication header will prevent the SIP stack from continuing generate invalid 
    // authorization for future requests.
    
    // The SIP stack saves authentication headers according to the order this client receives (from oldest to newest).
    // So, the authentication header which we're intereseted in is the one (if exists) right next to the last header. 

    RvSipAuthObjHandle hLastAuthObj     = NULL;
    RvSipAuthObjHandle hInvalidAuthObj  = NULL;
    if(RvSipRegClientAuthObjGet(hRegClient, RVSIP_LAST_ELEMENT, NULL, &hLastAuthObj) == RV_OK &&
       hLastAuthObj != NULL) {
      if(RvSipRegClientAuthObjGet(hRegClient, RVSIP_PREV_ELEMENT, hLastAuthObj, &hInvalidAuthObj) == RV_OK &&
	 hInvalidAuthObj != NULL) {
	RvSipAuthenticationHeaderHandle hTemp1 = NULL, hTemp2 = NULL;
	RvChar  realm1[MAX_LENGTH_OF_REALM_NAME] = {'\0'};
	RvUint length1 = 0;
	RvChar  realm2[MAX_LENGTH_OF_REALM_NAME] = {'\0'};
	RvUint length2 = 0;
	
	if(RvSipAuthObjGetAuthenticationHeader(hLastAuthObj, &hTemp1, NULL)                       == RV_OK &&
	   RvSipAuthenticationHeaderGetRealm(hTemp1, realm1, MAX_LENGTH_OF_REALM_NAME, &length1)  == RV_OK &&
	   RvSipAuthObjGetAuthenticationHeader(hInvalidAuthObj, &hTemp2, NULL)                    == RV_OK &&
	   RvSipAuthenticationHeaderGetRealm(hTemp2, realm2, MAX_LENGTH_OF_REALM_NAME, &length2)  == RV_OK) {
	  if(length1 == length2 && strcmp(realm1,realm2) == 0) {
	    rv = RvSipRegClientAuthObjRemove (hRegClient, hInvalidAuthObj);
	  }
	}
      }    
    }

    return rv;
  }

  RvStatus SIP_RemoveAuthHeader(RvSipCallLegHandle hCallLeg)  {

    RvStatus rv = RV_OK;

    // we get in here because the registrar told us that our previous request either doesn't have an 
    // authorization header or has an invalid authorization header.
    // If the previous request contains an invalid authorization header, we should remove the authentication 
    // header based upon which we had generated the invalid authorization header.
    // The removal of an authentication header will prevent the SIP stack from continuing generate invalid 
    // authorization for future requests.
    
    // The SIP stack saves authentication headers according to the order this client receives (from oldest to newest).
    // So, the authentication header which we're intereseted in is the one (if exists) right next to the last header. 

    RvSipAuthObjHandle hLastAuthObj     = NULL;
    RvSipAuthObjHandle hInvalidAuthObj  = NULL;
    if(RvSipCallLegAuthObjGet(hCallLeg, RVSIP_LAST_ELEMENT, NULL, &hLastAuthObj) == RV_OK &&
       hLastAuthObj != NULL) {
      if(RvSipCallLegAuthObjGet(hCallLeg, RVSIP_PREV_ELEMENT, hLastAuthObj, &hInvalidAuthObj) == RV_OK &&
	 hInvalidAuthObj != NULL) {
	RvSipAuthenticationHeaderHandle hTemp1 = NULL, hTemp2 = NULL;
	RvChar  realm1[MAX_LENGTH_OF_REALM_NAME] = {'\0'};
	RvUint length1 = 0;
	RvChar  realm2[MAX_LENGTH_OF_REALM_NAME] = {'\0'};
	RvUint length2 = 0;
	
	if(RvSipAuthObjGetAuthenticationHeader(hLastAuthObj, &hTemp1, NULL)                       == RV_OK &&
	   RvSipAuthenticationHeaderGetRealm(hTemp1, realm1, MAX_LENGTH_OF_REALM_NAME, &length1)  == RV_OK &&
	   RvSipAuthObjGetAuthenticationHeader(hInvalidAuthObj, &hTemp2, NULL)                    == RV_OK &&
	   RvSipAuthenticationHeaderGetRealm(hTemp2, realm2, MAX_LENGTH_OF_REALM_NAME, &length2)  == RV_OK) {
	  if(length1 == length2 && strcmp(realm1,realm2) == 0) {
	    rv = RvSipCallLegAuthObjRemove (hCallLeg, hInvalidAuthObj);
	  }
	}
      }    
    }

    return rv;
  }
  
} END_DECL_RV_SIP_ENGINE_NAMESPACE;

extern "C" void printfToNull(const char* format,...) {
  
}
