/// @file
/// @brief Rate-Based Load Strategy header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_RATE_BASED_LOAD_STRATEGY_H_
#define _L4L7_RATE_BASED_LOAD_STRATEGY_H_

#include <ace/Time_Value.h>
#include <base/LoadStrategy.h>

DECL_L4L7_BASE_NS

///////////////////////////////////////////////////////////////////////////////

class RateBasedLoadStrategy : virtual public LoadStrategy
{
  public:
    RateBasedLoadStrategy(void);
    virtual ~RateBasedLoadStrategy();
    
    /// @brief Return running status
    bool Running(void) const { return mRunning; }
    
    /// @brief Start the load strategy (from the LoadStrategy interface)
    void Start(ACE_Reactor *reactor, uint32_t hz = 0);

    /// @brief Stop the load strategy (from the LoadStrategy interface)
    void Stop(void);

    /// @brief Set the instantaneous load (from the LoadStrategy interface)
    void SetLoad(int32_t load);

    /// @brief Set the instantaneous load and max burst
    void SetLoad(int32_t load, uint32_t maxBurst);

    /// @brief Return the maximum load
    /// @note This is probably not the method you want, see AvailableLoad()
    const int32_t GetLoad(void) const { return mLoad; }
    
    /// @brief Return the available load at this instant
    const double AvailableLoad(void) const { return mBucket; }

    /// @brief Consume load
    /// @param load Amount of load used
    /// @retval True if requested amount of load was available, false otherwise
    bool ConsumeLoad(double load)
    {
        if (load <= mBucket)
        {
            mBucket -= load;
            return true;
        }
        else
            return false;
    }
    
  private:
    /// @note Concrete strategy classes can override
    virtual void LoadChangeHook(double load) { }
    
    int handle_timeout(const ACE_Time_Value& tv, const void *act);

    static const long INVALID_TIMER_ID = -1;

    bool mRunning;                                      ///< true when load strategy is running
    uint32_t mHz;                                       ///< timer frequency
    ACE_Time_Value mTimerInterval;                      ///< timer interval (1/hz)
    ACE_Time_Value mLastTimeout;                        ///< last timeout
    long mTimerId;                                      ///< timer id
    
    uint32_t mLoad;                                     ///< current load value
    double mBucket;                                     ///< current token bucket value
    double mMaxBurst;                                   ///< maximum token bucket value
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_BASE_NS

#endif
