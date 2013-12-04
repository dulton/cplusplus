/// @file
/// @brief Enc RTP Utils
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#if defined YOCTO_I686
#include <time.h>
#else
#include <posix_time.h>
#endif

#include <algorithm>
#include <string>
#include <cstring>
#include <iterator>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "EncRTPUtils.h"
#include "EncRTPRvInc.h"

///////////////////////////////////////////////////////////////////////////////

namespace std {
  template<>
  void swap(::ENCRTP_NS::EncRTPUtils::AtomicBool& a, ::ENCRTP_NS::EncRTPUtils::AtomicBool& b) { 
    a.swap(b);
  }
}

////////////////////////////////////////////////////////////////////////////////
