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

#include <sstream>
#include <boost/format.hpp>
#include "MessageDigest5.h"

DECL_L4L7_UTILS_NS

///////////////////////////////////////////////////////////////////////////////

MessageDigest5::MessageDigest5(const std::string& data) :
    mFinished(false)
{
    md5_init(&mState);
    *this << data;
}

///////////////////////////////////////////////////////////////////////////////

MessageDigest5& MessageDigest5::operator<<(const std::string& data)
{
    if (data.empty())
        return *this;
    if (mFinished)
    {
        md5_init(&mState);
        mFinished = false;
    }
    md5_append(&mState, (const md5_byte_t *) data.data(), data.size());
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

std::string MessageDigest5::ToHexString(void)
{
    if (!mFinished)
    {
        md5_finish(&mState, mDigest);
        mFinished = true;
    }
    std::ostringstream oss;
    for (size_t i = 0; i < 16; i++)
        oss << boost::format("%02x") % (int) mDigest[i];
    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_UTILS_NS
