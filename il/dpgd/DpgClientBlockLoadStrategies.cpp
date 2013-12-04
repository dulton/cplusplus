/// @file
/// @brief Dpg Client Block Load Strategies implementation
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include "DpgClientBlockLoadStrategies.h"
#include "DpgdLog.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::PlaylistsLoadStrategy::LoadChangeHook(int32_t load)
{
    TC_LOG_INFO_LOCAL(mClientBlock.mPort, LOG_CLIENT, "DPG client block [" << mClientBlock.Name() << "] playlists load changed to " << load);

    mClientBlock.SetIntendedLoad(static_cast<uint32_t>(load));

    // Update the intended load value in the block stats
    ACE_GUARD(stats_t::lock_t, guard, mClientBlock.mStats.Lock());
    mClientBlock.mStats.intendedLoad = static_cast<uint32_t>(load);
}

void DpgClientBlock::PlaylistsLoadStrategy::PlaylistComplete(void)
{
    mClientBlock.SetIntendedLoad(static_cast<uint32_t>(GetLoad()));
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::BandwidthLoadStrategy::LoadChangeHook(int32_t load)
{
    TC_LOG_INFO_LOCAL(mClientBlock.mPort, LOG_CLIENT, "DPG client block [" << mClientBlock.Name() << "] bandwidth load changed to " << load);

    // Ignore load if using Dynamic Mode
    if (!mClientBlock.GetEnableDynamicLoad())
        mClientBlock.SetDynamicLoad(load);
    else
        load = mClientBlock.GetDynamicLoadTotal();
    
    //  Always limit to 1 playlist
    if (load > 0)
      mClientBlock.SetIntendedLoad(1);

    // Update the intended load value in the block stats
    ACE_GUARD(stats_t::lock_t, guard, mClientBlock.mStats.Lock());
    mClientBlock.mStats.intendedLoad = static_cast<uint32_t>(load * 1024);  // convert kbps to bps
}

void DpgClientBlock::BandwidthLoadStrategy::PlaylistComplete(void)
{
    // Always limit to 1 connection
    mClientBlock.SetIntendedLoad(1);
}

///////////////////////////////////////////////////////////////////////////////

