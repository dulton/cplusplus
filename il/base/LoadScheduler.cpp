/// @file
/// @brief Load Scheduler implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <ace/Reactor.h>
#include <ace/Timer_Queue.h>

#include "LoadProfile.h"
#include "LoadScheduler.h"
#include "LoadStrategy.h"

USING_L4L7_BASE_NS;

///////////////////////////////////////////////////////////////////////////////

LoadScheduler::LoadScheduler(LoadProfile& profile, LoadStrategy& strategy)
    : mProfile(profile),
      mStrategy(strategy),
      mTimerId(INVALID_TIMER_ID),
      mStarted(false),
      mLastLoad(-1),
      mEnableDynamicLoad(false)
{
}

///////////////////////////////////////////////////////////////////////////////

LoadScheduler::~LoadScheduler()
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

void LoadScheduler::Start(ACE_Reactor *reactor)
{
    if (mTimerId != INVALID_TIMER_ID)
        Stop();

    // Start the load profile from the beginning
    mProfile.Start();
    mStarted = false;
    mLastLoad = -1;

    // Start the load strategy
    mStrategy.Start(reactor);

    // Schedule an immediate timeout to get things going
    mTimerSkew = reactor->timer_queue()->timer_skew();
    mTimerId = reactor->schedule_timer(this, 0, ACE_Time_Value::zero, ACE_Time_Value::zero);
}

///////////////////////////////////////////////////////////////////////////////

void LoadScheduler::Stop(void)
{
    // Stop the load strategy
    mStrategy.Stop();
    
    // Cancel pending timeout
    if (mTimerId != INVALID_TIMER_ID)
    {
        this->reactor()->cancel_timer(mTimerId);
        mTimerId = INVALID_TIMER_ID;
    }
}

///////////////////////////////////////////////////////////////////////////////

int LoadScheduler::handle_timeout(const ACE_Time_Value& tv, const void *act)
{
    mTimerId = INVALID_TIMER_ID;

    // Save our start time the first time through
    if (!mStarted)
    {
        mStartTime = tv;
        mStarted = true;
    }

    // Profile time is the dispatch time minus our start time
    const ACE_Time_Value t = tv - mStartTime;
    ACE_Time_Value next;

    // Evaluate the load
    const int32_t load = mProfile.Eval(t + mTimerSkew, next);
    //fprintf(stderr, "t = %u.%06u, next = %u.%06u, load = %d\n", t.sec(), t.usec(), next.sec(), next.usec(), load);
    
    // Reschedule the timer as necessary
    if (mEnableDynamicLoad || next != ACE_Time_Value::zero)
    {
        ACE_Time_Value nextTime;

        if (mEnableDynamicLoad)
          nextTime.sec(60);   // if dynamic load is enabled set to 1 min
        else
          nextTime = next - t;
      
        mTimerId = this->reactor()->schedule_timer(this, 0, nextTime , ACE_Time_Value::zero);
    }

    // Inform the strategy object of new load values
    if (load != mLastLoad)
    {
        mStrategy.SetLoad(load);
        mLastLoad = load;
    }
    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
