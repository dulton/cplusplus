/*
 *   Copyright 2010 IMTC, Inc.  All rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TIP_TIMER_H_
#define TIP_TIMER_H_

#include <list>
#include "tip_constants.h"

namespace LibTip {

    // simple timer class for tracking pending actions
    class CTipTimer {
    public:
        CTipTimer();
        ~CTipTimer();

        // types of timers supported
        enum TimerType {
            AMT_TIP_NEGOTIATE,
            AMT_DELAYED_ACK
        };

        // register a timer, returns a timer id that can be used to
        // cancel the timer
        uint32_t Register(TimerType type, uint64_t timeoutMsec, uint32_t data);

        // get an expired timer, returns TIP_OK if a timer expired,
        // otherwise returns TIP_ERROR.  timer type and user data are
        // returned in the params.  caller should invoke this in a
        // loop until TIP_ERROR is returned, internal state depends on
        // this caller behavior.
        Status GetExpired(TimerType& type, uint32_t& data);

        // cancel a pending timer, return TIP_OK if the timer was removed
        Status Cancel(uint32_t id);

        // get the amount of time until the next timeout.  a return of
        // (uint64_t) -1 indicates no pending timers.
        uint64_t GetNextExpiredTime() const;
        
    protected:
        // calculate the amount of time until the next timer expires
        void CalcNextExpired();
        
        uint32_t mNextId;
        uint64_t mNextExpired;
        
        // structure to hold a single timer
        struct TimerData {
            uint32_t  mId;      // timer id
            TimerType mType;    // type of timer
            uint64_t  mExpires; // time of expiration
            uint32_t  mData;    // user data
        };
        typedef std::list<TimerData> TimerList;
        TimerList mList;
    };

};

#endif

        
