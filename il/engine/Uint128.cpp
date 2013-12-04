/// @file
/// @brief uint128_t implementation
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include "Uint128.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

DECL_L4L7_ENGINE_NS

std::ostream& operator<<(std::ostream& os, const uint128_t& i)
{
    //return os << '(' << i.mHi << ',' << i.mLo << ')';
    return os << '(' << std::hex << i.mHi << ',' << i.mLo << ')';
}

END_DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

static void carry_up(uint64_t* val, size_t start) 
{
    for (size_t i = start; i < 3; ++i)
    {
        if (val[i] > 0xffffffff)
        {
            val[i + 1] += val[i] >> 32;
            val[i] &= 0xffffffff;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

const uint128_t uint128_t::operator*(uint32_t other) const 
{
    uint64_t res[4] = { mLo & 0xffffffff, mLo >> 32, mHi & 0xffffffff, mHi >> 32 };

    // Basic algorithm is analogous to this:
    //
    // 9876 * 5:
    //
    // Step 1: split up the big number so that per-digit overflow is not lost
    //
    // 9, 8, 7, 6
    //
    // Step 2: multiply the highest digit
    //
    // 45, 8, 7, 6
    //
    // Step 3: multiply the next highest digit
    //
    // 45, 40, 7, 6
    //
    // Step 4: carry any overflow
    //
    // 49, 0, 7, 6
    //
    // Repeat 3-4 on each digit
    //
    // 49, 0, 35, 6
    // 49, 3, 5, 6
    // 49, 3, 5, 30
    // 49, 3, 8, 0
    //
    // Step 5: fit back in the original container (overflow is lost)
    //
    //  = [4]9380

    res[3] *= other;
    //carry_up(res, 3);
    res[2] *= other;
    carry_up(res, 2);
    res[1] *= other;
    carry_up(res, 1);
    res[0] *= other;
    carry_up(res, 0);

    uint128_t result((res[3] << 32) | res[2], (res[1] << 32) | res[0]); 

    return result;
}

////////////////////////////////////////////////////////////////////////////////
