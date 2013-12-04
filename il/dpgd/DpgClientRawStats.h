/// @file
/// @brief Dpg Client Raw Statistics header file
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _DPG_CLIENT_BLOCK_RAW_STATS_H_
#define _DPG_CLIENT_BLOCK_RAW_STATS_H_

#include "DpgRawStats.h"

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

class DpgClientRawStats : public DpgRawStats
{
  public:
    DpgClientRawStats(uint32_t handle)
        : DpgRawStats(handle),
          intendedLoad(0),
          activeConnections(0),
          activePlaylists(0)
    {
        ResetInternal();
    }

    UNIT_TEST_VIRTUAL ~DpgClientRawStats() {}

#ifdef UNIT_TEST    
    DpgClientRawStats& operator=(const DpgClientRawStats& other)
    {
        elapsedSeconds = other.elapsedSeconds;
        intendedLoad = other.intendedLoad;

        activeConnections = other.activeConnections;
        attemptedConnections = other.attemptedConnections;
        successfulConnections = other.successfulConnections;
        unsuccessfulConnections = other.unsuccessfulConnections;
        abortedConnections = other.abortedConnections;

        attemptedCps = other.attemptedCps;
        successfulCps = other.successfulCps;
        unsuccessfulCps = other.unsuccessfulCps;
        abortedCps = other.abortedCps;

        activePlaylists = other.activePlaylists;
        attemptedPlaylists = other.attemptedPlaylists;
        successfulPlaylists = other.successfulPlaylists;
        unsuccessfulPlaylists = other.unsuccessfulPlaylists;
        abortedPlaylists = other.abortedPlaylists;

        attemptedPps = other.attemptedPps;
        successfulPps = other.successfulPps;
        unsuccessfulPps = other.unsuccessfulPps;
        abortedPps = other.abortedPps;

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

        txAttack = other.txAttack;

        mPlaylistAccumDuration = other.mPlaylistAccumDuration;
        playlistAvgDuration = other.playlistAvgDuration;
        playlistMaxDuration = other.playlistMaxDuration;
        playlistMinDuration = other.playlistMinDuration;

        mHandle = other.mHandle;
        return *this;
    }

    DpgClientRawStats(const DpgClientRawStats& other): DpgRawStats(other.mHandle) { *this = other; }
#endif
    
    /// @brief Start stats
    /// @note Clears statistics counters and internally latches "start" timestamp for elapsed time and rate calculations
    void Start(void) 
	{ 
		Reset(); 
		CreateTimer();
	}

    /// @brief Reset sets
    /// @note Clears statistics counters and internally latches "start" timestamp for elapsed time and rate calculations
    void Reset(void)
    {
        ACE_GUARD(lock_t, guard, mLock);
        mStartTime = mLastTime = Now();
        mLastGoodputRxBytes = mLastGoodputTxBytes = 0;
        mLastAttemptedConnections = mLastSuccessfulConnections = mLastUnsuccessfulConnections = mLastAbortedConnections = 0;
        mLastAttemptedPlaylists = mLastSuccessfulPlaylists = mLastUnsuccessfulPlaylists = mLastAbortedPlaylists = 0;
        ResetInternal();
    }

    /// @brief Synchronize stats
    /// @note Updates elapsed time and recalculates rate-based statistics
    void Sync(bool zeroRates = false)
    {
        // Calculate elapsed time and goodput rates
        const ACE_Time_Value now = Now();
        const ACE_Time_Value elapsedSinceLast = now - mLastTime;
        const ACE_Time_Value elapsedSinceStart = now - mStartTime;
        bool updatedRates = false;

        ACE_GUARD(lock_t, guard, mLock);

        if (elapsedSinceLast.msec() && !zeroRates)
        {
            goodputRxBps = static_cast<uint64_t>(8000 * static_cast<double>(goodputRxBytes - mLastGoodputRxBytes) / elapsedSinceLast.msec());
            goodputTxBps = static_cast<uint64_t>(8000 * static_cast<double>(goodputTxBytes - mLastGoodputTxBytes) / elapsedSinceLast.msec());

            attemptedCps = static_cast<uint64_t>(1000 * static_cast<double>(attemptedConnections - mLastAttemptedConnections) / elapsedSinceLast.msec());
            successfulCps = static_cast<uint64_t>(1000 * static_cast<double>(successfulConnections - mLastSuccessfulConnections) / elapsedSinceLast.msec());
            unsuccessfulCps = static_cast<uint64_t>(1000 * static_cast<double>(unsuccessfulConnections - mLastUnsuccessfulConnections) / elapsedSinceLast.msec());
            abortedCps = static_cast<uint64_t>(1000 * static_cast<double>(abortedConnections - mLastAbortedConnections) / elapsedSinceLast.msec());

            attemptedPps = static_cast<uint64_t>(1000 * static_cast<double>(attemptedPlaylists - mLastAttemptedPlaylists) / elapsedSinceLast.msec());
            successfulPps = static_cast<uint64_t>(1000 * static_cast<double>(successfulPlaylists - mLastSuccessfulPlaylists) / elapsedSinceLast.msec());
            unsuccessfulPps = static_cast<uint64_t>(1000 * static_cast<double>(unsuccessfulPlaylists - mLastUnsuccessfulPlaylists) / elapsedSinceLast.msec());
            abortedPps = static_cast<uint64_t>(1000 * static_cast<double>(abortedPlaylists - mLastAbortedPlaylists) / elapsedSinceLast.msec());

            updatedRates = true;
        }
        else
        {
            goodputRxBps = 0;
            goodputTxBps = 0;
            
            attemptedCps = 0;
            successfulCps = 0;
            unsuccessfulCps = 0;
            abortedCps = 0;
            
            attemptedPps = 0;
            successfulPps = 0;
            unsuccessfulPps = 0;
            abortedPps = 0;
        }

        mLastTime = now;
        mLastGoodputRxBytes = goodputRxBytes;
        mLastGoodputTxBytes = goodputTxBytes;
        
        mLastAttemptedConnections = attemptedConnections;
        mLastSuccessfulConnections = successfulConnections;
        mLastUnsuccessfulConnections = unsuccessfulConnections;
        mLastAbortedConnections = abortedConnections;

        mLastAttemptedPlaylists = attemptedPlaylists;
        mLastSuccessfulPlaylists = successfulPlaylists;
        mLastUnsuccessfulPlaylists = unsuccessfulPlaylists;
        mLastAbortedPlaylists = abortedPlaylists;

        elapsedSeconds = elapsedSinceStart.sec();

        if (updatedRates) goodputMinRxBps = goodputMinRxBps ? std::min(goodputMinRxBps, goodputRxBps) : goodputRxBps;
        goodputMaxRxBps = std::max(goodputMaxRxBps, goodputRxBps);
        goodputAvgRxBps = static_cast<uint64_t>(8000 * static_cast<double>(goodputRxBytes) / elapsedSinceStart.msec());

        if (updatedRates) goodputMinTxBps = goodputMinTxBps ? std::min(goodputMinTxBps, goodputTxBps) : goodputTxBps;
        goodputMaxTxBps = std::max(goodputMaxTxBps, goodputTxBps);
        goodputAvgTxBps = static_cast<uint64_t>(8000 * static_cast<double>(goodputTxBytes) / elapsedSinceStart.msec());

        uint64_t numPlaylists = successfulPlaylists + abortedPlaylists + unsuccessfulPlaylists;
        playlistAvgDuration = (uint32_t)(numPlaylists ? (mPlaylistAccumDuration / numPlaylists) : 0);
    }

    /// @brief Complete playlist
    /// @note Updates aborted/successful/unsuccessful count and durations
    void CompletePlaylist(uint64_t duration, bool successful, bool aborted)
    {
        if (successful) ++successfulPlaylists;
        else if (aborted) ++abortedPlaylists;
        else ++unsuccessfulPlaylists;

        mPlaylistAccumDuration += duration;
        if (duration > playlistMaxDuration) playlistMaxDuration = duration;
        if (!playlistMinDuration || (duration < playlistMinDuration)) playlistMinDuration = duration;
    }
    
    /// @note Public data members, must be accessed with lock held
    // uint64_t elapsedSeconds; --DpgRawStats
    uint32_t intendedLoad;
    
    uint32_t activeConnections;
    uint64_t attemptedConnections;
    uint64_t successfulConnections;
    uint64_t unsuccessfulConnections;
    uint64_t abortedConnections;

    uint64_t attemptedCps;
    uint64_t successfulCps;
    uint64_t unsuccessfulCps;
    uint64_t abortedCps;

    uint32_t activePlaylists;
    uint64_t attemptedPlaylists;
    uint64_t successfulPlaylists;
    uint64_t unsuccessfulPlaylists;
    uint64_t abortedPlaylists;

    uint64_t attemptedPps;
    uint64_t successfulPps;
    uint64_t unsuccessfulPps;
    uint64_t abortedPps;

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

    uint64_t txAttack;

    uint32_t playlistAvgDuration;
    uint32_t playlistMaxDuration;
    uint32_t playlistMinDuration;

  private:
    void ResetInternal(void)
    {
        elapsedSeconds = 0;

        attemptedConnections = 0;
        successfulConnections = 0;
        unsuccessfulConnections = 0;
        abortedConnections = 0;

        attemptedCps = 0;
        successfulCps = 0;
        unsuccessfulCps = 0;
        abortedCps = 0;

        attemptedPlaylists = 0;
        successfulPlaylists = 0;
        unsuccessfulPlaylists = 0;
        abortedPlaylists = 0;

        attemptedPps = 0;
        successfulPps = 0;
        unsuccessfulPps = 0;
        abortedPps = 0;

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

        txAttack = 0;

        mPlaylistAccumDuration = 0;
        playlistAvgDuration = 0;
        playlistMaxDuration = 0;
        playlistMinDuration = 0;
    }

    ACE_Time_Value mStartTime;              ///< start time for elapsed seconds calculation
    ACE_Time_Value mLastTime;               ///< last time for goodput rate calculations
    uint64_t mLastGoodputRxBytes;           ///< last goodput rx bytes value
    uint64_t mLastGoodputTxBytes;           ///< last goodput tx bytes value
    uint64_t mLastAttemptedConnections;     ///< last attempted connections value
    uint64_t mLastSuccessfulConnections;    ///< last successful connections value
    uint64_t mLastUnsuccessfulConnections;  ///< last unsuccessful connections value
    uint64_t mLastAbortedConnections;       ///< last aborted connections value
    uint64_t mLastAttemptedPlaylists;       ///< last attempted playlists value
    uint64_t mLastSuccessfulPlaylists;      ///< last successful playlists value
    uint64_t mLastUnsuccessfulPlaylists;    ///< last unsuccessful playlists value
    uint64_t mLastAbortedPlaylists;         ///< last aborted playlists value
    uint64_t mPlaylistAccumDuration;        ///< total of all playlist durations
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
