/// @file
/// @brief Wrapper utility class for Message Digest 5 hash
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _MESSAGE_DIGEST_5_H_
#define _MESSAGE_DIGEST_5_H_

#include <utils/UtilsCommon.h>
#include "md5.h"

DECL_L4L7_UTILS_NS

///////////////////////////////////////////////////////////////////////////////

class MessageDigest5
{
  public:
    explicit MessageDigest5(const std::string& data = std::string());
    MessageDigest5& operator<<(const std::string& data);
    std::string ToHexString(void);
    
  private:
    bool mFinished;
    md5_state_t mState;
    md5_byte_t mDigest[16];
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_UTILS_NS

#endif
