/// @file
/// @brief Testable (instrumented) Dpg server stats object
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _TESTABLE_DPG_SERVER_RAW_STATS_H_
#define _TESTABLE_DPG_SERVER_RAW_STATS_H_

#include "DpgServerRawStats.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestableDpgServerRawStats : public DpgServerRawStats
{
  public:
    TestableDpgServerRawStats(uint32_t handle) : 
        DpgServerRawStats(handle) 
    {
    }

    void SetNow(ACE_Time_Value now) { mNow = now; }

    void IncrStats()
    {
        elapsedSeconds += 1;

        activeConnections += 4;
        totalConnections += 64;

        activePlaylists += 128;
        totalPlaylists += 2048;

        goodputRxBytes += 4096;
        goodputTxBytes += 8192;
    }

  protected:
    ACE_Time_Value Now() const { return mNow; }

  private:
    ACE_Time_Value mNow;
}; 

///////////////////////////////////////////////////////////////////////////////

#endif
