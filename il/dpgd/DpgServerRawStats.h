/// @file
/// @brief Dpg Server Raw Statistics header file
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _DPG_SERVER_BLOCK_RAW_STATS_H_
#define _DPG_SERVER_BLOCK_RAW_STATS_H_

#include "DpgRawStats.h"

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

class DpgServerRawStats : public DpgRawStats
{
  public:
    DpgServerRawStats(uint32_t handle)
        : DpgRawStats(handle),
          activeConnections(0),
          activePlaylists(0)
    {
        ResetInternal();
    }

    UNIT_TEST_VIRTUAL ~DpgServerRawStats() {}

#ifdef UNIT_TEST    
    DpgServerRawStats& operator=(const DpgServerRawStats& other)
    {
        elapsedSeconds = other.elapsedSeconds;

        activeConnections = other.activeConnections;
        totalConnections = other.totalConnections;
        totalCps = other.totalCps;

        activePlaylists = other.activePlaylists;
        totalPlaylists = other.totalPlaylists;
        totalPps = other.totalPps;

        goodputRxBytes = other.goodputRxBytes;
        goodputTxBytes = other.goodputTxBytes;
        goodputRxBps = other.goodputRxBps;
        goodputTxBps = other.goodputTxBps;
        goodputAvgRxBps = other.goodputAvgRxBps;
        goodputAvgTxBps = other.goodputAvgTxBps;
        goodputMaxRxBps = other.goodputMaxRxBps;
        goodputMaxTxBps = other.goodputMaxTxBps;
        goodputMinRxBps = other.goodputMinRxBps;
        goodputMinTxBps = other.goodputMinTxBps;

        mHandle = other.mHandle;
        return *this;
    }

    DpgServerRawStats(const DpgServerRawStats& other) : DpgRawStats(other.mHandle) { *this = other; }
#endif

    /// @brief Start stats
    /// @note Clears statistics counters and internally latches "start" timestamp for elapsed time and rate calculations
    void Start(void) 
	{ 
		Reset(); 
		CreateTimer();
	}

    /// @brief Start stats
    /// @note Clears statistics counters and internally latches "start" timestamp for elapsed time and rate calculations
    void Reset(void)
    {
        ACE_GUARD(lock_t, guard, mLock);
        mStartTime = mLastTime = Now();
        mLastGoodputRxBytes = mLastGoodputTxBytes = 0;
        mLastTotalConnections = 0;
        mLastTotalPlaylists = 0;
        ResetInternal();
    }

    /// @brief Synchronize stats
    /// @note Updates elapsed time and recalculates rate-based statistics
    void Sync(bool zeroRates = false)
    {
        const ACE_Time_Value now = Now();

        // Calculate elapsed time and goodput rates
        const ACE_Time_Value elapsedSinceLast = now - mLastTime;
        const ACE_Time_Value elapsedSinceStart = now - mStartTime;
        bool updatedRates = false;

        ACE_GUARD(lock_t, guard, mLock);

        if (elapsedSinceLast.msec() && !zeroRates)
        {
            goodputRxBps = static_cast<uint64_t>(8000 * static_cast<double>(goodputRxBytes - mLastGoodputRxBytes) / elapsedSinceLast.msec());
            goodputTxBps = static_cast<uint64_t>(8000 * static_cast<double>(goodputTxBytes - mLastGoodputTxBytes) / elapsedSinceLast.msec());

            totalCps = static_cast<uint64_t>(1000 * static_cast<double>(totalConnections - mLastTotalConnections) / elapsedSinceLast.msec());
            totalPps = static_cast<uint64_t>(1000 * static_cast<double>(totalPlaylists - mLastTotalPlaylists) / elapsedSinceLast.msec());

            updatedRates = true;
        }
        else
        {
            goodputRxBps = 0;
            goodputTxBps = 0;

            totalCps = 0;
            totalPps = 0;
        }

        mLastTime = now;
        mLastGoodputRxBytes = goodputRxBytes;
        mLastGoodputTxBytes = goodputTxBytes;

        mLastTotalConnections = totalConnections;
        mLastTotalPlaylists = totalPlaylists;

        elapsedSeconds = elapsedSinceStart.sec();

        if (updatedRates) goodputMinRxBps = goodputMinRxBps ? std::min(goodputMinRxBps, goodputRxBps) : goodputRxBps;
        goodputMaxRxBps = std::max(goodputMaxRxBps, goodputRxBps);
        goodputAvgRxBps = static_cast<uint64_t>(8000 * static_cast<double>(goodputRxBytes) / elapsedSinceStart.msec());

        if (updatedRates) goodputMinTxBps = goodputMinTxBps ? std::min(goodputMinTxBps, goodputTxBps) : goodputTxBps;
        goodputMaxTxBps = std::max(goodputMaxTxBps, goodputTxBps);
        goodputAvgTxBps = static_cast<uint64_t>(8000 * static_cast<double>(goodputTxBytes) / elapsedSinceStart.msec());
    }

    /// @note Public data members, must be accessed with lock held
    // uint64_t elapsedSeconds; -- DpgRawStats

    uint32_t activeConnections;
    uint64_t totalConnections;
    uint64_t totalCps;

    uint32_t activePlaylists;
    uint64_t totalPlaylists;
    uint64_t totalPps;

    /* DpgRawStats: 
    uint64_t goodputRxBytes;
    uint64_t goodputTxBytes;
    uint64_t goodputRxBps;
    uint64_t goodputTxBps;
    uint64_t goodputAvgRxBps;
    uint64_t goodputAvgTxBps;
    uint64_t goodputMaxRxBps;
    uint64_t goodputMaxTxBps;
    uint64_t goodputMinRxBps;
    uint64_t goodputMinTxBps;
    */

  private:
    void ResetInternal(void)
    {
        elapsedSeconds = 0;

        totalConnections = 0;
        totalCps = 0;

        totalPlaylists = 0;
        totalPps = 0;

        goodputRxBytes = 0;
        goodputTxBytes = 0;
        goodputRxBps = 0;
        goodputTxBps = 0;
        goodputAvgRxBps = 0;
        goodputAvgTxBps = 0;
        goodputMaxRxBps = 0;
        goodputMaxTxBps = 0;
        goodputMinRxBps = 0;
        goodputMinTxBps = 0;

    }

    ACE_Time_Value mStartTime;              ///< start time for elapsed seconds calculation
    ACE_Time_Value mLastTime;               ///< last time for goodput rate calculations
    uint64_t mLastGoodputRxBytes;           ///< last goodput rx bytes value
    uint64_t mLastGoodputTxBytes;           ///< last goodput tx bytes value
    uint64_t mLastTotalConnections;         ///< last total connections value
    uint64_t mLastTotalPlaylists;           ///< last total playlists value
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
