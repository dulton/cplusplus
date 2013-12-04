/// @file
/// @brief Timing predicate implementation
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_UTILS_TIMING_PREDICATE_H_
#define _L4L7_UTILS_TIMING_PREDICATE_H_

#include <ace/OS.h>
#include <ace/Time_Value.h>
#include <utils/UtilsCommon.h>

DECL_L4L7_UTILS_NS

///////////////////////////////////////////////////////////////////////////////

// returns false until the given time elapses

template <long MSEC_DELAY>
class TimingPredicate
{
public:
    TimingPredicate() 
    {
        ACE_Time_Value delay;
        delay.msec(MSEC_DELAY);
        mEnd = ACE_OS::gettimeofday() + delay;
    }

    bool operator()() 
    {
        return ACE_OS::gettimeofday() < mEnd;
    }

private:
    ACE_Time_Value mEnd;
};


///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_UTILS_NS

#endif
