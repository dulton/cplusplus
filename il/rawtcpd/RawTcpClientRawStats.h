/// @file
/// @brief Raw tcp Client Raw Statistics header file
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _RAWTCP_CLIENT_BLOCK_RAW_STATS_H_
#define _RAWTCP_CLIENT_BLOCK_RAW_STATS_H_

#include <utility>

#include <ace/Guard_T.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Time_Value.h>

#include <utils/StatsTimerManager.h>
#include "RawTcpCommon.h"

USING_L4L7_UTILS_NS;

DECL_RAWTCP_NS

///////////////////////////////////////////////////////////////////////////////

class RawTcpClientRawStats : public L4L7_UTILS_NS::StatsTimerManager
{
  public:
    typedef ACE_Recursive_Thread_Mutex lock_t;

    RawTcpClientRawStats(uint32_t handle)
        : intendedLoad(0),
          activeConnections(0),
          mHandle(handle)
    {
        ResetInternal();
    }

#ifdef UNIT_TEST    
    RawTcpClientRawStats& operator=(const RawTcpClientRawStats& other)
    {
        elapsedSeconds = other.elapsedSeconds;
        intendedLoad = other.intendedLoad;
        goodputRxBytes = other.goodputRxBytes;
        goodputRxBps = other.goodputRxBps;
        goodputMinRxBps = other.goodputMinRxBps;
        goodputMaxRxBps = other.goodputMaxRxBps;
        goodputAvgRxBps = other.goodputAvgRxBps;
        goodputTxBytes = other.goodputTxBytes;
        goodputTxBps = other.goodputTxBps;
        goodputMinTxBps = other.goodputMinTxBps;
        goodputMaxTxBps = other.goodputMaxTxBps;
        goodputAvgTxBps = other.goodputAvgTxBps;
        activeConnections = other.activeConnections;
        attemptedConnections = other.attemptedConnections;
        attemptedCps = other.attemptedCps;
        successfulConnections = other.successfulConnections;
        successfulCps = other.successfulCps;
        unsuccessfulConnections = other.unsuccessfulConnections;
        unsuccessfulCps = other.unsuccessfulCps;
        abortedConnections = other.abortedConnections;
        abortedCps = other.abortedCps;
        attemptedTransactions = other.attemptedTransactions;
        attemptedTps = other.attemptedTps;
        successfulTransactions = other.successfulTransactions;
        successfulTps = other.successfulTps;
        unsuccessfulTransactions = other.unsuccessfulTransactions;
        unsuccessfulTps = other.unsuccessfulTps;
        abortedTransactions = other.abortedTransactions;
        abortedTps = other.abortedTps;
        mHandle = other.mHandle;
        return *this;
    }

    RawTcpClientRawStats(const RawTcpClientRawStats& other) { *this = other; }
#endif
    
    /// @brief Block handle accessor
    const uint32_t Handle(void) const { return mHandle; }
    
    /// @brief Lock accessor
    lock_t& Lock(void) { return mLock; }

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
        mStartTime = mLastTime = ACE_OS::gettimeofday();
        mLastGoodputRxBytes = mLastGoodputTxBytes = 0;
        mLastAttemptedConnections = mLastSuccessfulConnections = mLastUnsuccessfulConnections = mLastAbortedConnections = 0;
        mLastAttemptedTransactions = mLastSuccessfulTransactions = mLastUnsuccessfulTransactions = mLastAbortedTransactions = 0;
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

        if (elapsedSinceLast && !zeroRates)
        {
            goodputRxBps = static_cast<uint64_t>(8000 * static_cast<double>(goodputRxBytes - mLastGoodputRxBytes) / elapsedSinceLast.msec());
            goodputTxBps = static_cast<uint64_t>(8000 * static_cast<double>(goodputTxBytes - mLastGoodputTxBytes) / elapsedSinceLast.msec());

            attemptedCps = static_cast<uint64_t>(1000 * static_cast<double>(attemptedConnections - mLastAttemptedConnections) / elapsedSinceLast.msec());
            successfulCps = static_cast<uint64_t>(1000 * static_cast<double>(successfulConnections - mLastSuccessfulConnections) / elapsedSinceLast.msec());
            unsuccessfulCps = static_cast<uint64_t>(1000 * static_cast<double>(unsuccessfulConnections - mLastUnsuccessfulConnections) / elapsedSinceLast.msec());
            abortedCps = static_cast<uint64_t>(1000 * static_cast<double>(abortedConnections - mLastAbortedConnections) / elapsedSinceLast.msec());

            attemptedTps = static_cast<uint64_t>(1000 * static_cast<double>(attemptedTransactions - mLastAttemptedTransactions) / elapsedSinceLast.msec());
            successfulTps = static_cast<uint64_t>(1000 * static_cast<double>(successfulTransactions - mLastSuccessfulTransactions) / elapsedSinceLast.msec());
            unsuccessfulTps = static_cast<uint64_t>(1000 * static_cast<double>(unsuccessfulTransactions - mLastUnsuccessfulTransactions) / elapsedSinceLast.msec());
            abortedTps = static_cast<uint64_t>(1000 * static_cast<double>(abortedTransactions - mLastAbortedTransactions) / elapsedSinceLast.msec());
        }
        else
        {
            goodputRxBps = 0;
            goodputTxBps = 0;
            
            attemptedCps = 0;
            successfulCps = 0;
            unsuccessfulCps = 0;
            abortedCps = 0;

            attemptedTps = 0;
            successfulTps = 0;
            unsuccessfulTps = 0;
            abortedTps = 0;
        }

        mLastTime = now;
        mLastGoodputRxBytes = goodputRxBytes;
        mLastGoodputTxBytes = goodputTxBytes;
        
        mLastAttemptedConnections = attemptedConnections;
        mLastSuccessfulConnections = successfulConnections;
        mLastUnsuccessfulConnections = unsuccessfulConnections;
        mLastAbortedConnections = abortedConnections;

        mLastAttemptedTransactions = attemptedTransactions;
        mLastSuccessfulTransactions = successfulTransactions;
        mLastUnsuccessfulTransactions = unsuccessfulTransactions;
        mLastAbortedTransactions = abortedTransactions;

        elapsedSeconds = elapsedSinceStart.sec();

        goodputMinRxBps = goodputMinRxBps ? std::min(goodputMinRxBps, goodputRxBps) : goodputRxBps;
        goodputMaxRxBps = std::max(goodputMaxRxBps, goodputRxBps);
        goodputAvgRxBps = static_cast<uint64_t>(8000 * static_cast<double>(goodputRxBytes) / elapsedSinceStart.msec());

        goodputMinTxBps = goodputMinTxBps ? std::min(goodputMinTxBps, goodputTxBps) : goodputTxBps;
        goodputMaxTxBps = std::max(goodputMaxTxBps, goodputTxBps);
        goodputAvgTxBps = static_cast<uint64_t>(8000 * static_cast<double>(goodputTxBytes) / elapsedSinceStart.msec());

    }
    
    /// @note Public data members, must be accessed with lock held
    uint64_t elapsedSeconds;
    uint32_t intendedLoad;
    
    uint64_t goodputRxBytes;
    uint64_t goodputRxBps;
    uint64_t goodputMinRxBps;
    uint64_t goodputMaxRxBps;
    uint64_t goodputAvgRxBps;
    uint64_t goodputTxBytes;
    uint64_t goodputTxBps;
    uint64_t goodputMinTxBps;
    uint64_t goodputMaxTxBps;
    uint64_t goodputAvgTxBps;

    uint32_t activeConnections;

    uint64_t attemptedConnections;
    uint64_t attemptedCps;
    uint64_t successfulConnections;
    uint64_t successfulCps;
    uint64_t unsuccessfulConnections;
    uint64_t unsuccessfulCps;
    uint64_t abortedConnections;
    uint64_t abortedCps;

    uint64_t attemptedTransactions;
    uint64_t attemptedTps;
    uint64_t successfulTransactions;
    uint64_t successfulTps;
    uint64_t unsuccessfulTransactions;
    uint64_t unsuccessfulTps;
    uint64_t abortedTransactions;
    uint64_t abortedTps;

  private:
    void ResetInternal(void)
    {
        elapsedSeconds = 0;

        goodputRxBytes = 0;
        goodputRxBps = 0;
        goodputMinRxBps = 0;
        goodputMaxRxBps = 0;
        goodputAvgRxBps = 0;
        goodputTxBytes = 0;
        goodputTxBps = 0;
        goodputMinTxBps = 0;
        goodputMaxTxBps = 0;
        goodputAvgTxBps = 0;

        attemptedConnections = 0;
        attemptedCps = 0;
        successfulConnections = 0;
        successfulCps = 0;
        unsuccessfulConnections = 0;
        unsuccessfulCps = 0;
        abortedConnections = 0;
        abortedCps = 0;

        attemptedTransactions = 0;
        attemptedTps = 0;
        successfulTransactions = 0;
        successfulTps = 0;
        unsuccessfulTransactions = 0;
        unsuccessfulTps = 0;
        abortedTransactions = 0;
        abortedTps = 0;

    }

    uint32_t mHandle;                       ///< block handle

    lock_t mLock;                           ///< protects against concurrent access
    ACE_Time_Value mStartTime;              ///< start time for elapsed seconds calculation
    ACE_Time_Value mLastTime;               ///< last time for goodput rate calculations
    uint64_t mLastGoodputRxBytes;           ///< last goodput rx bytes value
    uint64_t mLastGoodputTxBytes;           ///< last goodput tx bytes value
    uint64_t mLastAttemptedConnections;     ///< last attempted connections value
    uint64_t mLastSuccessfulConnections;    ///< last successful connections value
    uint64_t mLastUnsuccessfulConnections;  ///< last unsuccessful connections value
    uint64_t mLastAbortedConnections;       ///< last aborted connections value
    uint64_t mLastAttemptedTransactions;    ///< last attempted connections value
    uint64_t mLastSuccessfulTransactions;   ///< last successful connections value
    uint64_t mLastUnsuccessfulTransactions; ///< last unsuccessful connections value
    uint64_t mLastAbortedTransactions;      ///< last aborted connections value
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_RAWTCP_NS

#endif
