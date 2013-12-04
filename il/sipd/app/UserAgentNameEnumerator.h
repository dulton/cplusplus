/// @file
/// @brief SIP User Agent Name Enumerator class header
///
///  Copyright (c) 2008 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _USER_AGENT_NAME_ENUMERATOR_H_
#define _USER_AGENT_NAME_ENUMERATOR_H_

#include <string>

#include "VoIPCommon.h"

DECL_APP_NS

///////////////////////////////////////////////////////////////////////////////

class UserAgentNameEnumerator
{
  public:
    UserAgentNameEnumerator(uint64_t start, uint64_t step, const std::string& format);
    UserAgentNameEnumerator(uint64_t start, uint64_t step, uint64_t end, const std::string& format);

    /// @brief Return the total number of names
    size_t TotalCount(void) const { return static_cast<size_t>((mEnd - mStart) / mStep); }

    /// @brief Reset the enumerator to its starting value
    void Reset(void) { mCurr = mStart; }
    
    /// @brief Advance the enumerator to its next value
    void Next(void)
    {
        mCurr += mStep;
        if (mCurr >= mEnd)
            mCurr = mStart;
    }

    /// @brief Set the enumerator position
    void SetPosition(size_t pos);
    
    /// @brief Return the current enumerator value as a formatted string
    const std::string GetName(void) const;

  private:
    // Class data
    static const std::string DEFAULT_UA_NAME_FORMAT;
    
    const uint64_t mStart;
    const uint64_t mStep;
    const uint64_t mEnd;
    std::string mFormat;
    uint64_t mCurr;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_APP_NS

#endif
