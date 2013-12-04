/// @file
/// @brief Enc RTP Common defs
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ENCRTP_UTILS_H_
#define _ENCRTP_UTILS_H_

#include <stdint.h>

#include <string>
#include <vector>

#include <ace/INET_Addr.h>

#include <boost/utility.hpp>
#include <boost/function.hpp>

#include <spt_pthread.h>

/////////////////////////////////////////////////////////////////////////////

#include "VoIPUtils.h"
#include "EncRTPCommon.h"

/////////////////////////////////////////////////////////////////////////////

#define EncRTPGuard VoIPGuard

DECL_ENCRTP_NS

namespace EncRTPUtils {

///////////////////////////////////////////////////////////////////////////////
  
  typedef VOIP_NS::VoIPUtils::MutexType MutexType;

  ///////////////////////////////////////////////////////////////////////////////
  
  /**
   * Primitive AtomicBool implementation 
   */
  class AtomicBool {  
    
  private:
    typedef struct { volatile int mValue; } ValueType;
    ValueType mData;
    
  public:
    
    AtomicBool() {
      mData.mValue = 0;
    }
    
    template<class T>
      explicit AtomicBool(const T &t) {
      mData.mValue = t ? 1 : 0;
    }
    
    template<class T>
      AtomicBool& operator=(const T &t) {
      AtomicBool tmp(t);
      std::swap(mData.mValue,tmp.mData.mValue);
      return *this;
    }
    
    template<class T>
      void swap(T &other) {
      AtomicBool tmp(other);
      other = (T)(mData.mValue);
      std::swap(mData.mValue,tmp.mData.mValue);
    }
    
    template<class T>
      bool operator==(const T& other) const {
      bool res1 = other ? true : false;
      bool res2 = mData.mValue ? true : false;
      return (res1 == res2);
    }
    
    template<class T>
      bool operator!=(const T& other) const {
      return !(*this == other);
    }
    
    bool operator()() const {
      return (mData.mValue!=0);
    }
    
    operator bool() const {
      return (mData.mValue!=0);
    }
    
    bool operator !() const {
      return (mData.mValue==0);
    }
  };
  
  ///////////////////////////////////////////////////////////////////////////////
  
} //namespace EncRTPUtils

END_DECL_ENCRTP_NS

///////////////////////////////////////////////////////////////////////////////

namespace std {
  template<>
    void swap(::ENCRTP_NS::EncRTPUtils::AtomicBool& a, ::ENCRTP_NS::EncRTPUtils::AtomicBool& b);
}

///////////////////////////////////////////////////////////////////////////////

#endif //_ENCRTP_UTILS_H_

