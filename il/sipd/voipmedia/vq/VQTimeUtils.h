///
/// @file
/// @brief AudioHD & Video quality analyzer time utilities
///
/// Copyright (c) 2011 by Spirent Communications Inc.
/// All Rights Reserved.
///
/// This software is confidential and proprietary to Spirent Communications Inc.
/// No part of this software may be reproduced, transmitted, disclosed or used
/// in violation of the Software License Agreement without the expressed
/// written consent of Spirent Communications Inc.

#include <hal++/common/TimeStamp.h>

///////////////////////////////////////////////////////////////////////////////

class TimeUtils
{
  public:
    static const timestamp_t TIMESTAMP_MASK = 0xFFFFFFFFFFFFULL;
    static void ConvertTimestamp(struct timeval* tv_timestamp, const timestamp_t& tt_timestamp)
    {
        // convert from 2.5 ns to sec,usec -- benchmarked at ~40ns
        tv_timestamp->tv_sec  = tt_timestamp / 400000000;
        timestamp_t rem       = tt_timestamp % 400000000; 
        tv_timestamp->tv_usec = rem / 400; 
    }

    static void GetTimestamp(
        struct timeval* tv_masked_timestamp, 
        struct timeval* tv_timestamp = 0) 
    {
        timestamp_t tt_timestamp = Hal::TimeStamp::getTimeStamp();
        if (tv_timestamp)
        {
            ConvertTimestamp(tv_timestamp, tt_timestamp);
        }

        // we lose the high byte b/c that's what the rx timestamp does
        // FIXME: add high byte to rx timestamp instead of removing it here
        if (tv_masked_timestamp)
        {
            ConvertTimestamp(tv_masked_timestamp, tt_timestamp & TIMESTAMP_MASK);
        }
    }
};

///////////////////////////////////////////////////////////////////////////////
