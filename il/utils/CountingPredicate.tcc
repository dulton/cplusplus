/// @file
/// @brief Counting predicate implementation
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_UTILS_COUNTING_PREDICATE_H_
#define _L4L7_UTILS_COUNTING_PREDICATE_H_

#include <ace/OS.h>
#include <ace/Time_Value.h>
#include <utils/UtilsCommon.h>

DECL_L4L7_UTILS_NS

///////////////////////////////////////////////////////////////////////////////

// returns false until it's called N times

template <typename INT, INT COUNT>
class CountingPredicate
{
public:
    CountingPredicate() : 
        mI(0)
    {
    }

    bool operator()() 
    {
        return mI++ < COUNT;
    }

private:
    INT mI;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_UTILS_NS

#endif

