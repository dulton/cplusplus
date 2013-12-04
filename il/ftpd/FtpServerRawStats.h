/// @file
/// @brief FTP Server Raw Statistics header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _FTP_SERVER_BLOCK_RAW_STATS_H_
#define _FTP_SERVER_BLOCK_RAW_STATS_H_

#include <utility>

#include <ace/Guard_T.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Time_Value.h>

#include <utils/StatsTimerManager.h>
#include "FtpCommon.h"

USING_L4L7_UTILS_NS;

DECL_FTP_NS

///////////////////////////////////////////////////////////////////////////////

class FtpServerRawStats : public L4L7_UTILS_NS::StatsTimerManager
{
  public:
    typedef ACE_Recursive_Thread_Mutex lock_t;

    FtpServerRawStats(uint32_t handle)
        : activeControlConnections(0),
          mHandle(handle)
    {
        ResetInternal();
    }

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

    /// @brief Start stats
    /// @note Clears statistics counters and internally latches "start" timestamp for elapsed time and rate calculations
    void Reset(void)
    {
        ACE_GUARD(lock_t, guard, mLock);
        mStartTime = mLastTime = ACE_OS::gettimeofday();
        mLastGoodputRxBytes = mLastGoodputTxBytes = 0;
        mLastTotalControlConnections = mLastAbortedControlConnections = mLastTotalDataConnections = 0;
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

            totalControlCps = static_cast<uint64_t>(1000 * static_cast<double>(totalControlConnections - mLastTotalControlConnections) / elapsedSinceLast.msec());
            abortedControlCps = static_cast<uint64_t>(1000 * static_cast<double>(abortedControlConnections - mLastAbortedControlConnections) / elapsedSinceLast.msec());
            totalDataCps = static_cast<uint64_t>(1000 * static_cast<double>(totalDataConnections - mLastTotalDataConnections) / elapsedSinceLast.msec());

            attemptedTps = static_cast<uint64_t>(1000 * static_cast<double>(attemptedTransactions - mLastAttemptedTransactions) / elapsedSinceLast.msec());
            successfulTps = static_cast<uint64_t>(1000 * static_cast<double>(successfulTransactions - mLastSuccessfulTransactions) / elapsedSinceLast.msec());
            unsuccessfulTps = static_cast<uint64_t>(1000 * static_cast<double>(unsuccessfulTransactions - mLastUnsuccessfulTransactions) / elapsedSinceLast.msec());
            abortedTps = static_cast<uint64_t>(1000 * static_cast<double>(abortedTransactions - mLastAbortedTransactions) / elapsedSinceLast.msec());

        }
        else
        {
            goodputRxBps = 0;
            goodputTxBps = 0;

            totalControlCps = 0;
            abortedControlCps = 0;
            totalDataCps = 0;

            attemptedTps = 0;
            successfulTps = 0;
            unsuccessfulTps = 0;
            abortedTps = 0;
        }

        mLastTime = now;
        mLastGoodputRxBytes = goodputRxBytes;
        mLastGoodputTxBytes = goodputTxBytes;
        
        mLastTotalControlConnections = totalControlConnections;
        mLastAbortedControlConnections = abortedControlConnections;
        mLastTotalDataConnections = totalDataConnections;

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

    uint64_t totalControlConnections;
    uint64_t totalControlCps;
    uint64_t abortedControlConnections;
    uint64_t abortedControlCps; 

    uint32_t activeControlConnections;

    uint64_t totalDataConnections;
    uint64_t totalDataCps;

    uint64_t rxUserCmd;
    uint64_t rxPassCmd;
    uint64_t rxTypeCmd;
    uint64_t rxPortCmd;
    uint64_t rxRetrCmd;
    uint64_t rxQuitCmd;

    uint64_t attemptedTransactions;
    uint64_t attemptedTps;
    uint64_t successfulTransactions;
    uint64_t successfulTps;
    uint64_t unsuccessfulTransactions;
    uint64_t unsuccessfulTps;
    uint64_t abortedTransactions;
    uint64_t abortedTps;

    uint64_t txResponseCode150;
    uint64_t txResponseCode200;
    uint64_t txResponseCode220;
    uint64_t txResponseCode226;
    uint64_t txResponseCode227;
    uint64_t txResponseCode230;
    uint64_t txResponseCode331;
    uint64_t txResponseCode425;
    uint64_t txResponseCode426;
    uint64_t txResponseCode452;
    uint64_t txResponseCode500;
    uint64_t txResponseCode502;
    uint64_t txResponseCode530;

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

        totalControlConnections = 0;
        totalControlCps = 0;
        abortedControlConnections = 0;
        abortedControlCps = 0;

        totalDataConnections = 0;
        totalDataCps = 0;

        rxUserCmd = 0;
        rxPassCmd = 0;
        rxTypeCmd = 0;
        rxPortCmd = 0;
        rxRetrCmd = 0;
        rxQuitCmd = 0;

        attemptedTransactions = 0;
        attemptedTps = 0;
        successfulTransactions = 0;
        successfulTps = 0;
        unsuccessfulTransactions = 0;
        unsuccessfulTps = 0;
        abortedTransactions = 0;
        abortedTps = 0;

        txResponseCode150 = 0;
        txResponseCode200 = 0;
        txResponseCode220 = 0;
        txResponseCode226 = 0;
        txResponseCode227 = 0;
        txResponseCode230 = 0;
        txResponseCode331 = 0;
        txResponseCode425 = 0;
        txResponseCode426 = 0;
        txResponseCode452 = 0;
        txResponseCode500 = 0;
        txResponseCode502 = 0;
        txResponseCode530 = 0;
    }
            
    const uint32_t mHandle;                  ///< block handle

    lock_t mLock;                            ///< protects against concurrent access
    ACE_Time_Value mStartTime;               ///< start time for elapsed seconds calculation
    ACE_Time_Value mLastTime;                ///< last time for goodput rate calculations
    uint64_t mLastGoodputRxBytes;            ///< last goodput rx bytes value
    uint64_t mLastGoodputTxBytes;            ///< last goodput tx bytes value
    uint64_t mLastTotalControlConnections;   ///< last total control connections value
    uint64_t mLastAbortedControlConnections; ///< last aborted control connections value
    uint64_t mLastTotalDataConnections;      ///< last total data connections value
    uint64_t mLastAttemptedTransactions;     ///< last attempted connections value
    uint64_t mLastSuccessfulTransactions;    ///< last successful connections value
    uint64_t mLastUnsuccessfulTransactions;  ///< last unsuccessful connections value
    uint64_t mLastAbortedTransactions;       ///< last aborted connections value
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_FTP_NS

#endif
