/// @file
/// @brief HTTP Server Raw Statistics header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _HTTP_VIDEO_SERVER_BLOCK_RAW_STATS_H_
#define _HTTP_VIDEO_SERVER_BLOCK_RAW_STATS_H_

#include <utility>

#include <ace/Guard_T.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Time_Value.h>

#include "HttpCommon.h"

DECL_HTTP_NS

///////////////////////////////////////////////////////////////////////////////

class HttpVideoServerRawStats
{
  public:
    typedef ACE_Recursive_Thread_Mutex lock_t;

    HttpVideoServerRawStats(uint32_t handle)
        : activeConnections(0),
          mHandle(handle)
    {
        ResetInternal();
    }

#ifdef UNIT_TEST    
    HttpVideoServerRawStats& operator=(const HttpVideoServerRawStats& other)
    {
        activeConnections = other.activeConnections;
        // ABR Related statistics
        totalTxManifestFiles = other.totalTxManifestFiles;
        txMediaFragments64Kbps = other.txMediaFragments64Kbps;
        txMediaFragments96Kbps = other.txMediaFragments96Kbps;
        txMediaFragments150Kbps = other.txMediaFragments150Kbps;
        txMediaFragments240Kbps = other.txMediaFragments240Kbps;
        txMediaFragments256Kbps = other.txMediaFragments256Kbps;
        txMediaFragments440Kbps = other.txMediaFragments440Kbps;
        txMediaFragments640Kbps = other.txMediaFragments640Kbps;
        txMediaFragments800Kbps = other.txMediaFragments800Kbps;
        txMediaFragments840Kbps = other.txMediaFragments840Kbps;
        txMediaFragments1240Kbps = other.txMediaFragments1240Kbps;
        mHandle = other.mHandle;
        return *this;
    }

    HttpVideoServerRawStats(const HttpVideoServerRawStats& other) { *this = other; }
#endif

    /// @brief Block handle accessor
    const uint32_t Handle(void) const { return mHandle; }
    
    /// @brief Lock accessor
    lock_t& Lock(void) { return mLock; }
    
    /// @brief Start stats
    /// @note Clears statistics counters and internally latches "start" timestamp for elapsed time and rate calculations
    void Start(void) { Reset(); }

    /// @brief Start stats
    /// @note Clears statistics counters and internally latches "start" timestamp for elapsed time and rate calculations
    void Reset(void)
    {
        ACE_GUARD(lock_t, guard, mLock);
        mStartTime = mLastTime = ACE_OS::gettimeofday();
        ResetInternal();
    }

    /// @brief Synchronize stats
    /// @note Updates elapsed time and recalculates rate-based statistics
    void Sync(bool zeroRates = false)
    {
        const ACE_Time_Value now = ACE_OS::gettimeofday();
        
        // Calculate elapsed time and goodput rates
        const ACE_Time_Value elapsedSinceLast = now - mLastTime;
        const ACE_Time_Value elapsedSinceStart = now - mStartTime;

        ACE_GUARD(lock_t, guard, mLock);

        mLastTime = now;

    }
    
    /// @note Public data members, must be accessed with lock held
    uint32_t activeConnections;

    uint64_t totalTxManifestFiles;
    uint64_t txMediaFragments64Kbps;
    uint64_t txMediaFragments96Kbps;
    uint64_t txMediaFragments150Kbps;
    uint64_t txMediaFragments240Kbps;
    uint64_t txMediaFragments256Kbps;
    uint64_t txMediaFragments440Kbps;
    uint64_t txMediaFragments640Kbps;
    uint64_t txMediaFragments800Kbps;
    uint64_t txMediaFragments840Kbps;
    uint64_t txMediaFragments1240Kbps;

  private:
    void ResetInternal(void)
    {
        totalTxManifestFiles = 0;
        txMediaFragments64Kbps = 0;
        txMediaFragments96Kbps = 0;
        txMediaFragments150Kbps = 0;
        txMediaFragments240Kbps = 0;
        txMediaFragments256Kbps = 0;
        txMediaFragments440Kbps = 0;
        txMediaFragments640Kbps = 0;
        txMediaFragments800Kbps = 0;
        txMediaFragments840Kbps = 0;
        txMediaFragments1240Kbps = 0;

    }
            
    uint32_t mHandle;                       ///< block handle

    lock_t mLock;                           ///< protects against concurrent access
    ACE_Time_Value mStartTime;              ///< start time for elapsed seconds calculation
    ACE_Time_Value mLastTime;               ///< last time for goodput rate calculations
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_HTTP_NS

#endif
