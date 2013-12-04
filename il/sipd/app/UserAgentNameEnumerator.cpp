/// @file
/// @brief SIP User Agent Name Enumerator implementation
///
///  Copyright (c) 2008 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <sstream>

#include <boost/format.hpp>
#include <phxexception/PHXException.h>

#include "VoIPLog.h"
#include "UserAgentNameEnumerator.h"

USING_APP_NS;

///////////////////////////////////////////////////////////////////////////////

const std::string UserAgentNameEnumerator::DEFAULT_UA_NAME_FORMAT = "%u";

///////////////////////////////////////////////////////////////////////////////

UserAgentNameEnumerator::UserAgentNameEnumerator(uint64_t start, uint64_t step, const std::string& format)
    : mStart(start),
      mStep(step),
      mEnd(static_cast<uint64_t>(-1)),
      mFormat(format.empty() ? DEFAULT_UA_NAME_FORMAT : format),
      mCurr(start)
{
}

///////////////////////////////////////////////////////////////////////////////

UserAgentNameEnumerator::UserAgentNameEnumerator(uint64_t start, uint64_t step, uint64_t end, const std::string& format)
    : mStart(start),
      mStep(step),
      mEnd(end),
      mFormat(format.empty() ? DEFAULT_UA_NAME_FORMAT : format),
      mCurr(start)
{
    if (mStart > mEnd)
    {
        std::ostringstream oss;
        oss << "[UserAgentNameEnumerator ctor] Start value (" << mStart << ") cannot be greater than end value (" << mEnd << ")";
        const std::string err(oss.str());
        TC_LOG_ERR_LOCAL(0, LOG_SIP, err);
        throw EPHXBadConfig(err);
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentNameEnumerator::SetPosition(size_t pos)
{
    const uint64_t curr = mStart + (mStep * pos);
    if (curr >= mEnd)
        throw EFault("UserAgentNameEnumerator::SetPosition");

    mCurr = curr;
}
    
///////////////////////////////////////////////////////////////////////////////

const std::string UserAgentNameEnumerator::GetName(void) const
{
    std::ostringstream oss;

    try
    {
        oss << boost::format(mFormat) % mCurr;
    }
    catch (boost::io::too_many_args&)
    {
        oss << boost::format(mFormat);
    }
    catch (...)
    {
        oss << boost::format(DEFAULT_UA_NAME_FORMAT) % mCurr;
    }
    
    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////
