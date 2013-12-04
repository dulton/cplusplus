/// @file
/// @brief Dpg Client Block Load Strategies header file 
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _DPG_CLIENT_BLOCK_LOAD_STRATEGIES_H_
#define _DPG_CLIENT_BLOCK_LOAD_STRATEGIES_H_

#include <base/RateBasedLoadStrategy.h>
#include <base/StaticLoadStrategy.h>

#include "DpgClientBlock.h"

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

class DpgClientBlock::LoadStrategy : virtual public L4L7_BASE_NS::LoadStrategy
{
  public:
    LoadStrategy(DpgClientBlock& clientBlock)
        : mClientBlock(clientBlock)
    {
    }

    // Playlist events
    virtual void PlaylistComplete(void) { }
    
  protected:
    DpgClientBlock& mClientBlock;
};

///////////////////////////////////////////////////////////////////////////////

class DpgClientBlock::PlaylistsLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::StaticLoadStrategy
{
  public:
    PlaylistsLoadStrategy(DpgClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void PlaylistComplete(void);
    
  private:
    void LoadChangeHook(int32_t load);
};

///////////////////////////////////////////////////////////////////////////////

class DpgClientBlock::BandwidthLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::StaticLoadStrategy
{
  public:
    BandwidthLoadStrategy(DpgClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void PlaylistComplete(void);

  private:
    void LoadChangeHook(int32_t load);
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
