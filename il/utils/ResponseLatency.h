/// @file
/// @brief fixed/random response latency utility class
///
///  Copyright (c) 2008 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_UTILS_RESP_LATENCY_H_
#define _L4L7_UTILS_RESP_LATENCY_H_

#include <ace/Time_Value.h>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <utils/UtilsCommon.h>

DECL_L4L7_UTILS_NS

///////////////////////////////////////////////////////////////////////////////

class ResponseLatency
{
    typedef boost::minstd_rand randomEngine_t;
    typedef boost::normal_distribution<double> normalDistribution_t;
    typedef boost::variate_generator<randomEngine_t&, normalDistribution_t> normalRandomGen_t;

  public:
    ResponseLatency(randomEngine_t& randomEngine) :
        mIsFixed(true),
        mFixed(0, 0),
        mRandom(randomEngine, normalDistribution_t())
    {
    }

    void Set(uint32_t fixed)
    {
        mIsFixed = true;
        // do the conversion ourselves, ACE only supports signed long msecs
        uint32_t secs = fixed / 1000;
        uint32_t usecs = (fixed - secs * 1000) * 1000;
        mFixed.set(secs, usecs);
    }

    void Set(uint32_t mean, uint32_t stddev)
    {
        mIsFixed = false;
        mRandom.distribution() = normalDistribution_t(static_cast<double>(mean), static_cast<double>(stddev));
    }

    ACE_Time_Value Get()
    {
        if (mIsFixed)
        {
            return mFixed;
        }
        else
        {
            ACE_Time_Value result(0, 0);
 
            const double tmpMsec = mRandom();
            if (tmpMsec < 0.0)
                return result;

            result.set(tmpMsec / 1000.0);
            return result;
        }
    }
    
  private:
    bool mIsFixed;                           ///< fixed/random flag
    ACE_Time_Value mFixed;                   ///< fixed response latency
    normalRandomGen_t mRandom;               ///< random response latency generator
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_UTILS_NS

#endif
