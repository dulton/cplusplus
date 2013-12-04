/// @file
/// @brief fixed/random body size utility class
///
///  Copyright (c) 2008 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_UTILS_BODY_SIZE_H_
#define _L4L7_UTILS_BODY_SIZE_H_

#include <boost/random/linear_congruential.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <utils/UtilsCommon.h>

DECL_L4L7_UTILS_NS

///////////////////////////////////////////////////////////////////////////////

class BodySize
{
    typedef boost::minstd_rand randomEngine_t;
    typedef boost::normal_distribution<double> normalDistribution_t;
    typedef boost::variate_generator<randomEngine_t&, normalDistribution_t> normalRandomGen_t;

public:
    BodySize(randomEngine_t& randomEngine) :
        mIsFixed(true),
        mFixed(1),
        mRandom(randomEngine, normalDistribution_t()) 
    {
    }

    void Set(uint32_t fixed)
    {
        mIsFixed = true;
        mFixed = fixed;
    }

    void Set(uint32_t mean, uint32_t stddev)
    {
        mIsFixed = false;
        mRandom.distribution() = normalDistribution_t(static_cast<double>(mean), static_cast<double>(stddev));
    }

    uint32_t Get()
    {
        if (mIsFixed)
        {
            return mFixed;
        }
        else
        {
            const double tmp = mRandom();
            return (tmp < 0.0) ? 0 : static_cast<uint32_t>(tmp);
        }
    }

private:
    bool mIsFixed;                           ///< fixed/random flag
    uint32_t mFixed;                         ///< fixed body size
    normalRandomGen_t mRandom;               ///< random body size generator
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_UTILS_NS

#endif
