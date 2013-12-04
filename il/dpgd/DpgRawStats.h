/// @file
/// @brief Dpg Raw Statistics header file
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _DPG_RAW_STATS_H_
#define _DPG_RAW_STATS_H_

#include <utility>

#include <ace/Guard_T.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Time_Value.h>

#include <utils/StatsTimerManager.h>
#include "DpgCommon.h"

USING_L4L7_UTILS_NS;

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

class DpgRawStats : public L4L7_UTILS_NS::StatsTimerManager
{
  public:
    typedef ACE_Recursive_Thread_Mutex lock_t;

    DpgRawStats(uint32_t handle)
        : mHandle(handle)
    {
    }

    UNIT_TEST_VIRTUAL ~DpgRawStats() {}

    /// @brief Block handle accessor
    const uint32_t Handle(void) const { return mHandle; }

    /// @brief Lock accessor
    lock_t& Lock(void) { return mLock; }

    /// @note Public data members, must be accessed with lock held
    uint64_t elapsedSeconds;

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

  protected:
    /// @brief Time of day utility
    UNIT_TEST_VIRTUAL ACE_Time_Value Now(void) const { return ACE_OS::gettimeofday(); }

    uint32_t mHandle;                       ///< block handle

    lock_t mLock;                           ///< protects against concurrent access
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
