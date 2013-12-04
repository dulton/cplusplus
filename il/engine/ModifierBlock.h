/// @file
/// @brief Modifier block class
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _MODIFIER_BLOCK_H_
#define _MODIFIER_BLOCK_H_

#include <boost/tr1/memory.hpp>

#include <engine/EngineCommon.h>
#include "AbstModifier.h"
#include "PlaylistConfig.h"

class TestModifierBlock;

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

class ModifierBlock
{
  public:
    ModifierBlock(const PlaylistConfig& config, uint32_t randSeed);

    ModifierBlock(const ModifierBlock& copy);

    void Next();
    
    void SetCursor(uint32_t index);

    uint32_t GetCursor() const { return mCursor; }

    size_t GetSize(uint16_t flowIdx, uint8_t varIdx) const 
    {
        ModMap_t::const_iterator iter = mMap.find(FlowVarIdx_t(flowIdx, varIdx));
        if (iter == mMap.end())
        {
            return 0;
        }
        return iter->second->GetSize();
    }

    void GetValue(uint16_t flowIdx, uint8_t varIdx, uint8_t * buffer, bool reverse = false) const
    {
        ModMap_t::const_iterator iter = mMap.find(FlowVarIdx_t(flowIdx, varIdx));
        if (iter != mMap.end())
        {
            iter->second->GetValue(buffer, reverse);
        }
    }

  private:
    static AbstModifier * MakeRandomMod(const PlaylistConfig::RangeMod& config, uint32_t& seed);
    static AbstModifier * MakeRangeMod(const PlaylistConfig::RangeMod& config, uint32_t& seed);
    static AbstModifier * MakeTableMod(const PlaylistConfig::TableMod& config);

    typedef std::pair<uint16_t, uint8_t> FlowVarIdx_t;
    typedef std::tr1::shared_ptr<AbstModifier> ModifierPtr_t;

    typedef std::map<FlowVarIdx_t, ModifierPtr_t> ModMap_t;
    ModMap_t mMap;
    uint32_t mCursor;

    friend class TestModifierBlock;
};

///////////////////////////////////////////////////////////////////////////////

class FlowModifierBlock 
{
  public:
    FlowModifierBlock(const ModifierBlock& modifiers, uint16_t flowIdx)
        : mModifiers(modifiers), mFlowIdx(flowIdx)
    {
    }

    size_t GetSize(uint8_t varIdx) const
    {
        return mModifiers.GetSize(mFlowIdx, varIdx);
    }

    void GetValue(uint8_t varIdx, uint8_t * buffer, bool reverse = false) const
    {
        mModifiers.GetValue(mFlowIdx, varIdx, buffer, reverse);
    }

  private:
    const ModifierBlock& mModifiers;
    uint16_t mFlowIdx;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
