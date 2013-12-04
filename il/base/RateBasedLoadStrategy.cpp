/// @file
/// @brief Rate-Based Load Strategy implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <sys/param.h>
#include <utility>

#include <ace/Reactor.h>

#include "RateBasedLoadStrategy.h"

USING_L4L7_BASE_NS;

///////////////////////////////////////////////////////////////////////////////

RateBasedLoadStrategy::RateBasedLoadStrategy(void)
    : mRunning(false),
      mTimerId(INVALID_TIMER_ID),
      mLoad(0),
      mBucket(0),
      mMaxBurst(0)
{
}

///////////////////////////////////////////////////////////////////////////////

RateBasedLoadStrategy::~RateBasedLoadStrategy()
{
    ACE_Reactor *reactor(this->reactor());
    if (reactor)
    {
        if (mTimerId != INVALID_TIMER_ID)
            reactor->cancel_timer(mTimerId);

        this->reactor(0);
    }
}

///////////////////////////////////////////////////////////////////////////////

void RateBasedLoadStrategy::Start(ACE_Reactor *reactor, uint32_t hz)
{
    if (mRunning)
        Stop();

    // Hz must be in the range [0, HZ) with a default value of HZ
    mHz = hz ? std::min(hz, static_cast<const uint32_t>(HZ)) : HZ;
    mTimerInterval.usec(1000000 / mHz);
    
    // Schedule an immediate timeout to start things off
    mLastTimeout = ACE_Time_Value::zero;
    mTimerId = reactor->schedule_timer(this, 0, ACE_Time_Value::zero, ACE_Time_Value::zero);
    mRunning = true;
}

///////////////////////////////////////////////////////////////////////////////

void RateBasedLoadStrategy::Stop(void)
{
    if (!mRunning)
        return;
    
    if (mTimerId != INVALID_TIMER_ID)
    {
        this->reactor()->cancel_timer(mTimerId);
        mTimerId = INVALID_TIMER_ID;
    }

    this->reactor(0);
    mRunning = false;
}

///////////////////////////////////////////////////////////////////////////////

void RateBasedLoadStrategy::SetLoad(int32_t load)
{
    // Update load value and recalculate max burst for uniform distribution
    mLoad = static_cast<uint32_t>(load);

    if (mLoad == 0)
        mMaxBurst = 0;
    else if (mLoad <= mHz)
        mMaxBurst = 2.0;
    else
        mMaxBurst = static_cast<double>(mLoad * 2) / mHz;

    // Clip bucket if max burst has decreased
    mBucket = std::min(mBucket, mMaxBurst);

    // Notify user of load change
    this->LoadChangeHook(mBucket);
    
    // Schedule a timeout if timer isn't running and bucket isn't full
    if (mRunning && mTimerId == INVALID_TIMER_ID && mBucket < mMaxBurst)
        mTimerId = this->reactor()->schedule_timer(this, 0, mTimerInterval, ACE_Time_Value::zero);
}

///////////////////////////////////////////////////////////////////////////////

void RateBasedLoadStrategy::SetLoad(int32_t load, uint32_t maxBurst)
{
    // Update load value and max burst
    mLoad = static_cast<uint32_t>(load);
    mMaxBurst = static_cast<double>(maxBurst);
    
    // Clip bucket if max burst has decreased
    mBucket = std::min(mBucket, mMaxBurst);

    // Notify user of load change
    this->LoadChangeHook(mBucket);
    
    // Schedule a timeout if timer isn't running and bucket isn't full
    if (mRunning && mTimerId == INVALID_TIMER_ID && mBucket < mMaxBurst)
        mTimerId = this->reactor()->schedule_timer(this, 0, mTimerInterval, ACE_Time_Value::zero);
}

///////////////////////////////////////////////////////////////////////////////

int RateBasedLoadStrategy::handle_timeout(const ACE_Time_Value& tv, const void *act)
{
    mTimerId = INVALID_TIMER_ID;

    // Calculate elapsed time since last service
    const ACE_Time_Value elapsed = tv - mLastTimeout;
    mLastTimeout = tv;
    
    // Add tokens to bucket
    double tokens = static_cast<double>(mLoad) * (static_cast<double>(elapsed.sec()) + (static_cast<double>(elapsed.usec()) * 0.000001));
    mBucket += tokens;
    mBucket = std::min(mBucket, mMaxBurst);

    // Notify user of load availability
    if (mBucket)
        this->LoadChangeHook(mBucket);

    // Reschedule interval timer while bucket isn't full
    if (mRunning && mBucket < mMaxBurst)
        mTimerId = this->reactor()->schedule_timer(this, 0, mTimerInterval, ACE_Time_Value::zero);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
