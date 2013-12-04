/// @file
/// @brief Dpg timer manager implementation
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <ace/Reactor.h>
#include <boost/foreach.hpp>

#include "DpgdLog.h"
#include "TimerManager.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

TimerManager::TimerManager(ACE_Reactor * ioReactor) : 
  ACE_Event_Handler(ioReactor)
{
}


///////////////////////////////////////////////////////////////////////////////

TimerManager::~TimerManager()
{
    ACE_Reactor *reactor(this->reactor());
    if (reactor)
    {
        ACE_GUARD(lock_t, set_guard, mLock);
        BOOST_FOREACH(handle_t handle, mHandleSet)
        {
            if (handle != INVALID_TIMER_ID)
                reactor->cancel_timer(handle);
        }

        this->reactor(0);
    }
}

///////////////////////////////////////////////////////////////////////////////

TimerManager::handle_t TimerManager::CreateTimer(timerDelegate_t delegate, uint32_t msDelay) 
{ 
    ACE_Time_Value delay;
    delay.msec(msDelay);
    ACE_Reactor * reactor = this->reactor();
    handle_t timerId = INVALID_TIMER_ID;

    if (reactor)
    {
        DelegateArgPair * delegatePair = new DelegateArgPair(delegate); 
        timerId = delegatePair->timerId = (handle_t)reactor->schedule_timer(this, delegatePair, delay, ACE_Time_Value::zero); 

        if (delegatePair->fired)
        {
            // it's possible for the timer to be handled prior to returning
            timerId = INVALID_TIMER_ID;
            delete delegatePair;
        }
        else
        {
            ACE_GUARD_RETURN(lock_t, set_guard, mLock, INVALID_TIMER_ID);
            mHandleSet.insert(delegatePair->timerId);
        }
    }

    return timerId;
}

///////////////////////////////////////////////////////////////////////////////

void TimerManager::CancelTimer(handle_t timerId)
{
    ACE_Reactor * reactor = this->reactor();

    if (reactor)
    {
        const void * arg = 0;
        int cancelled = reactor->cancel_timer(timerId, &arg);
        if (cancelled && arg)
        {
            const DelegateArgPair *delegatePair = static_cast<const DelegateArgPair*>(arg);
            if (delegatePair->timerId != INVALID_TIMER_ID)
            {
                // it's possible for the timer to be handled and cancelled during Create
                delete delegatePair;
            }
        }
    }
    
    {
        ACE_GUARD(lock_t, set_guard, mLock);
        mHandleSet.erase(timerId);
    }
}

///////////////////////////////////////////////////////////////////////////////

int TimerManager::handle_timeout(const ACE_Time_Value &current_time, const void *act)
{
    const DelegateArgPair *delegatePair = static_cast<const DelegateArgPair*>(act);

    if (delegatePair->timerId != INVALID_TIMER_ID)
    {
        {
            ACE_GUARD_RETURN(lock_t, set_guard, mLock, 0);
            mHandleSet.erase(delegatePair->timerId);
        }
        (delegatePair->delegate)();
        delete delegatePair;
    }
    else
    {
        // it's possible for the timeout to fire before the timer id is set 
        // let the scheduler delete it
        (delegatePair->delegate)();
        const_cast<DelegateArgPair*>(delegatePair)->fired = true;
    }


    return 0;
}

///////////////////////////////////////////////////////////////////////////////

uint64_t TimerManager::GetTimeOfDayMsec() const
{
    uint64_t result;
    const ACE_Time_Value now = ACE_OS::gettimeofday();
    now.msec(result); // const is required to fix ambiguity
    return result;
}

///////////////////////////////////////////////////////////////////////////////
