/// @file
/// @brief Testable (instrumented) Dpg client stats object
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _TESTABLE_DPG_CLIENT_RAW_STATS_H_
#define _TESTABLE_DPG_CLIENT_RAW_STATS_H_

#include "DpgClientRawStats.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestableDpgClientRawStats : public DpgClientRawStats
{
  public:
    TestableDpgClientRawStats(uint32_t handle) : 
        DpgClientRawStats(handle) 
    {
    }

    void SetNow(ACE_Time_Value now) { mNow = now; }

    void IncrStats()
    {
        elapsedSeconds += 1;
        intendedLoad += 2;

        activeConnections += 4;
        attemptedConnections += 8;
        successfulConnections += 16;
        unsuccessfulConnections += 32;
        abortedConnections += 64;

        activePlaylists += 128;
        attemptedPlaylists += 256;
        successfulPlaylists += 512;
        unsuccessfulPlaylists += 1024;
        abortedPlaylists += 2048;

        goodputRxBytes += 4096;
        goodputTxBytes += 8192;

        txAttack += 16384;
    }

  protected:
    ACE_Time_Value Now() const { return mNow; }

  private:
    ACE_Time_Value mNow;
}; 

///////////////////////////////////////////////////////////////////////////////

#endif
