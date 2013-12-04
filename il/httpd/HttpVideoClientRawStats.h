/// @file
/// @brief HTTP Client Raw Statistics header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _HTTP_VIDEO_CLIENT_BLOCK_RAW_STATS_H_
#define _HTTP_VIDEO_CLIENT_BLOCK_RAW_STATS_H_

#include <utility>

#include <ace/Guard_T.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Time_Value.h>

#include "HttpCommon.h"

DECL_HTTP_NS

///////////////////////////////////////////////////////////////////////////////

class HttpVideoClientRawStats
{
  public:
    typedef ACE_Recursive_Thread_Mutex lock_t;

    HttpVideoClientRawStats(uint32_t handle)
        : intendedLoad(0),
          activeConnections(0),
          mHandle(handle)
    {
        ResetInternal();
    }

#ifdef UNIT_TEST    
    HttpVideoClientRawStats& operator=(const HttpVideoClientRawStats& other)
    {
        intendedLoad = other.intendedLoad;
        activeConnections = other.activeConnections;
        asSessionsAttempted = other.asSessionsAttempted;
        asSessionsSuccessful = other.asSessionsSuccessful;
        asSessionsUnsuccessful = other.asSessionsUnsuccessful;
        mHandle = other.mHandle;
        return *this;
    }

    HttpVideoClientRawStats(const HttpVideoClientRawStats& other) { *this = other; }
#endif
    
    /// @brief Block handle accessor
    const uint32_t Handle(void) const { return mHandle; }
    
    /// @brief Lock accessor
    lock_t& Lock(void) { return mLock; }

    /// @brief Start stats
    /// @note Clears statistics counters and internally latches "start" timestamp for elapsed time and rate calculations
    void Start(void) { Reset(); }

    /// @brief Reset sets
    /// @note Clears statistics counters and internally latches "start" timestamp for elapsed time and rate calculations
    void Reset(void)
    {
        ACE_GUARD(lock_t, guard, mLock);
        mStartTime = mLastTime = ACE_OS::gettimeofday();
        mLastAsSessionsAttempted = mLastAsSessionsSuccessful = mLastAsSessionsUnsuccessful = 0;
        ResetInternal();
    }

    /// @brief Synchronize stats
    /// @note Updates elapsed time and recalculates rate-based statistics
    void Sync(bool zeroRates = false)
    {
        // Calculate elapsed time and goodput rates
        const ACE_Time_Value now = ACE_OS::gettimeofday();
        const ACE_Time_Value elapsedSinceLast = now - mLastTime;
        const ACE_Time_Value elapsedSinceStart = now - mStartTime;

        ACE_GUARD(lock_t, guard, mLock);

        mLastTime = now;
        mLastAsSessionsAttempted = asSessionsAttempted;
        mLastAsSessionsSuccessful = asSessionsSuccessful;
        mLastAsSessionsUnsuccessful = asSessionsUnsuccessful;

        if (countBufferingWaitTime)
        {
        	avgBufferingWaitTime = static_cast<uint64_t>(static_cast<double>(sumBufferingWaitTime) / countBufferingWaitTime);
            if(avgBufferingWaitTime > maxBufferingWaitTime) maxBufferingWaitTime = avgBufferingWaitTime;
            // minBufferingWaitTime: always zero??? TODO

        }
        else
        	avgBufferingWaitTime = 0;

    }
    
    /// @note Public data members, must be accessed with lock held
    uint32_t intendedLoad;
    uint32_t activeConnections;

    uint64_t asSessionsAttempted;
    uint64_t asSessionsSuccessful;
    uint64_t asSessionsUnsuccessful;
    uint64_t manifestReqsAttempted;
    uint64_t manifestReqsSuccessful;
    uint64_t manifestReqsUnsuccessful;
    uint64_t minBufferingWaitTime;
    uint64_t avgBufferingWaitTime;
    uint64_t maxBufferingWaitTime;
    uint64_t sumBufferingWaitTime;
    uint64_t countBufferingWaitTime;

    uint64_t mediaFragmentsRx64kbps;
    uint64_t mediaFragmentsRx96kbps;
    uint64_t mediaFragmentsRx150kbps;
    uint64_t mediaFragmentsRx240kbps;
    uint64_t mediaFragmentsRx256kbps;
    uint64_t mediaFragmentsRx440kbps;
    uint64_t mediaFragmentsRx640kbps;
    uint64_t mediaFragmentsRx800kbps;
    uint64_t mediaFragmentsRx840kbps;
    uint64_t mediaFragmentsRx1240kbps;

  private:
    void ResetInternal(void)
    {
        asSessionsAttempted = 0;
        asSessionsSuccessful = 0;
        asSessionsUnsuccessful = 0;
        manifestReqsAttempted = 0;
        manifestReqsSuccessful = 0;
        manifestReqsUnsuccessful = 0;
        minBufferingWaitTime = 0;
        avgBufferingWaitTime = 0;
        maxBufferingWaitTime = 0;
        sumBufferingWaitTime = 0;
        countBufferingWaitTime = 0;

        mediaFragmentsRx64kbps = 0;
        mediaFragmentsRx96kbps = 0;
        mediaFragmentsRx150kbps = 0;
        mediaFragmentsRx240kbps = 0;
        mediaFragmentsRx256kbps = 0;
        mediaFragmentsRx440kbps = 0;
        mediaFragmentsRx640kbps = 0;
        mediaFragmentsRx800kbps = 0;
        mediaFragmentsRx840kbps = 0;
        mediaFragmentsRx1240kbps = 0;
    }
            
    uint32_t mHandle;                       ///< block handle

    lock_t mLock;                           ///< protects against concurrent access
    ACE_Time_Value mStartTime;              ///< start time for elapsed seconds calculation
    ACE_Time_Value mLastTime;               ///< last time for goodput rate calculations
    uint64_t mLastAsSessionsAttempted;     ///< last attempted connections value
    uint64_t mLastAsSessionsSuccessful;    ///< last successful connections value
    uint64_t mLastAsSessionsUnsuccessful;  ///< last unsuccessful connections value
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_HTTP_NS

#endif
