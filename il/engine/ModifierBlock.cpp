/// @file
/// @brief Modifier block implementation
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <netinet/in.h>

#include <boost/foreach.hpp>

#include "RandomModifier.h"
#include "RangeModifier.h"
#include "TableModifier.h"
#include "Uint48.h"
//#include "Uint128.h"

#include "ModifierBlock.h"


USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

ModifierBlock::ModifierBlock(const PlaylistConfig& config, uint32_t randSeed)
    : mCursor(0)
{
    uint16_t flowIdx = 0;
    uint8_t  varIdx;
    BOOST_FOREACH(const PlaylistConfig::Flow& flow, config.flows)
    {
        BOOST_FOREACH(const PlaylistConfig::Flow::RangeModMap_t::value_type& rangeModPair, flow.rangeModMap)
        {
            varIdx = rangeModPair.first;
            mMap[FlowVarIdx_t(flowIdx, varIdx)] = ModifierPtr_t(MakeRangeMod(rangeModPair.second, randSeed));
        }

        BOOST_FOREACH(const PlaylistConfig::Flow::TableModMap_t::value_type& tableModPair, flow.tableModMap)
        {
            varIdx = tableModPair.first;
            mMap[FlowVarIdx_t(flowIdx, varIdx)] = ModifierPtr_t(MakeTableMod(tableModPair.second));
        }

        BOOST_FOREACH(const PlaylistConfig::Flow::LinkModMap_t::value_type& linkModPair, flow.linkModMap)
        {
            uint8_t parentVarIdx = linkModPair.first;
            uint8_t childVarIdx = linkModPair.second;
            mMap[FlowVarIdx_t(flowIdx, parentVarIdx)]->SetChild(mMap[FlowVarIdx_t(flowIdx, childVarIdx)].get());
        }

        // NOTE: overlapping modifiers should be rejected by the application
        // NOTE: circular linked modifiers should be rejected by the application
        
        ++flowIdx;
    }
}

///////////////////////////////////////////////////////////////////////////////

ModifierBlock::ModifierBlock(const ModifierBlock& copy)
    : mCursor (copy.mCursor)
{
    typedef std::map<AbstModifier *, AbstModifier *> modPtrMap_t;
    modPtrMap_t ptrMap;

    BOOST_FOREACH(const ModMap_t::value_type& value, copy.mMap)
    {
        // we want a deep copy here, not shared
        ModifierPtr_t mod(value.second->Clone());
        mMap[value.first] = mod;

        // old pointer -> new pointer
        ptrMap[value.second.get()] = mod.get();
    }

    BOOST_FOREACH(const ModMap_t::value_type& value, copy.mMap)
    {
        // second pass - set children
        if (value.second->HasChild())
            mMap[value.first]->SetChild(ptrMap[value.second->GetChild()]);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ModifierBlock::Next()
{
    ++mCursor;
    BOOST_FOREACH(const ModMap_t::value_type& value, mMap)
    {
        // only hit root modifiers
        if (!value.second->HasParent())
            value.second->Next();
    }
}

///////////////////////////////////////////////////////////////////////////////

void ModifierBlock::SetCursor(uint32_t index)
{
    mCursor = index;
    BOOST_FOREACH(const ModMap_t::value_type& value, mMap)
    {
        // only hit root modifiers
        if (!value.second->HasParent())
            value.second->SetCursor(index);
    }
}

///////////////////////////////////////////////////////////////////////////////

static uint16_t vec_to_u16(const std::vector<uint8_t>& vec)
{
    return (uint16_t(vec[0]) << 8) | (uint16_t(vec[1]) & 0xff); 
}

static uint32_t vec_to_u32(const std::vector<uint8_t>& vec)
{
    return (uint32_t(vec[0]) << 24) | (uint32_t(vec[1]) << 16) | 
           (uint32_t(vec[2]) << 8) | (uint32_t(vec[3]) & 0xff); 
}

static uint48_t vec_to_u48(const std::vector<uint8_t>& vec)
{
    uint16_t hi16 = 
           (uint16_t(vec[0]) << 8)  | (uint16_t(vec[1]) & 0xff); 
    uint32_t lo32 =
           (uint32_t(vec[2]) << 24) | (uint32_t(vec[3]) << 16) | 
           (uint32_t(vec[4]) << 8)  | (uint32_t(vec[5]) & 0xff); 

    uint48_t result(hi16, lo32);
    return uint48_t(hi16, lo32);
}

static uint64_t vec_to_u64(const std::vector<uint8_t>& vec)
{
    uint32_t hi32 = 
           (uint32_t(vec[0]) << 24) | (uint32_t(vec[1]) << 16) | 
           (uint32_t(vec[2]) << 8)  | (uint32_t(vec[3]) & 0xff); 
    uint32_t lo32 =
           (uint32_t(vec[4]) << 24) | (uint32_t(vec[5]) << 16) | 
           (uint32_t(vec[6]) << 8)  | (uint32_t(vec[7]) & 0xff); 

    uint64_t result = (uint64_t(hi32) << 32) | (uint64_t(lo32) & 0xffffffff);
    return result;
}

#if 0
static uint128_t vec_to_u128(const std::vector<uint8_t>& vec)
{
    uint32_t hi32 = 
           (uint32_t(vec[0]) << 24) | (uint32_t(vec[1]) << 16) | 
           (uint32_t(vec[2]) << 8)  | (uint32_t(vec[3]) & 0xff); 
    uint32_t lo32 =
           (uint32_t(vec[4]) << 24) | (uint32_t(vec[5]) << 16) | 
           (uint32_t(vec[6]) << 8)  | (uint32_t(vec[7]) & 0xff); 

    uint64_t hi64 = (uint64_t(hi32) << 32) | (uint64_t(lo32) & 0xffffffff);

    hi32 = 
           (uint32_t(vec[8]) << 24)  | (uint32_t(vec[9]) << 16) | 
           (uint32_t(vec[10]) << 8)  | (uint32_t(vec[11]) & 0xff); 
    lo32 =
           (uint32_t(vec[12]) << 24) | (uint32_t(vec[13]) << 16) | 
           (uint32_t(vec[14]) << 8)  | (uint32_t(vec[15]) & 0xff); 

    uint64_t lo64 = (uint64_t(hi32) << 32) | (uint64_t(lo32) & 0xffffffff);

    return (uint128_t(hi64) << 64) | uint128_t(low64);
}
#endif

///////////////////////////////////////////////////////////////////////////////

AbstModifier * ModifierBlock::MakeRandomMod(const PlaylistConfig::RangeMod& config, uint32_t& randSeed)
{
    randSeed = randSeed + 0x3ce; // seeds must not be the same across modifiers

    if (config.start.size() == 4)
    {
        uint32_t perModSeed = 0;
        memcpy(&perModSeed, &config.start[0], 4);
        randSeed ^= ntohl(perModSeed);
    }

    if (config.mask.size() == 1)
    {
        return new RandomModifier<uint8_t>(config.mask[0], config.recycleCount, randSeed, config.stutterCount);
    }
    else if (config.mask.size() == 2)
    {
        return new RandomModifier<uint16_t>(vec_to_u16(config.mask), config.recycleCount, randSeed, config.stutterCount);
    }
    else if (config.mask.size() == 4)
    {
        return new RandomModifier<uint32_t>(vec_to_u32(config.mask), config.recycleCount, randSeed, config.stutterCount);
    }
    else if (config.mask.size() == 6)
    {
        return new RandomModifier<uint48_t>(vec_to_u48(config.mask), config.recycleCount, randSeed, config.stutterCount);
    }
    else if (config.mask.size() == 8)
    {
        return new RandomModifier<uint64_t>(vec_to_u64(config.mask), config.recycleCount, randSeed, config.stutterCount);
    }
#if 0
    else if (config.mask.size() == 16)
    {
        return new RandomModifier<uint128_t>(vec_to_u128(config.mask), config.recycleCount, randSeed);
    }
#endif

    throw DPG_EInternal("unsupported modifier size");
}

///////////////////////////////////////////////////////////////////////////////

AbstModifier * ModifierBlock::MakeRangeMod(const PlaylistConfig::RangeMod& config, uint32_t& randSeed)
{
    if (config.mode == PlaylistConfig::MOD_RANDOM)
    {
        return MakeRandomMod(config, randSeed);
    }

    if (config.start.size() != config.step.size())
        throw DPG_EInternal("modifier start and step size mismatch");

    if (config.start.size() != config.mask.size())
        throw DPG_EInternal("modifier start and mask size mismatch");
    
    if (config.start.size() == 1)
    {
        return new RangeModifier<uint8_t>(config.start[0], (config.mode == PlaylistConfig::MOD_DECR) ? -config.step[0] : config.step[0], config.mask[0], config.stutterCount, config.recycleCount);
    }
    else if (config.start.size() == 2)
    {
        return new RangeModifier<uint16_t>(vec_to_u16(config.start), (config.mode == PlaylistConfig::MOD_DECR) ? -vec_to_u16(config.step) : vec_to_u16(config.step), vec_to_u16(config.mask), config.stutterCount, config.recycleCount);
    }
    else if (config.start.size() == 4)
    {
        return new RangeModifier<uint32_t>(vec_to_u32(config.start), (config.mode == PlaylistConfig::MOD_DECR) ? -vec_to_u32(config.step) : vec_to_u32(config.step), vec_to_u32(config.mask), config.stutterCount, config.recycleCount);
    }
    else if (config.start.size() == 6)
    {
        return new RangeModifier<uint48_t>(vec_to_u48(config.start), (config.mode == PlaylistConfig::MOD_DECR) ? -vec_to_u48(config.step) : vec_to_u48(config.step), vec_to_u48(config.mask), config.stutterCount, config.recycleCount);
    }
    else if (config.start.size() == 8)
    {
        return new RangeModifier<uint64_t>(vec_to_u64(config.start), (config.mode == PlaylistConfig::MOD_DECR) ? -vec_to_u64(config.step) : vec_to_u64(config.step), vec_to_u64(config.mask), config.stutterCount, config.recycleCount);
    }
#if 0
    else if (config.start.size() == 16)
    {
        return new RangeModifier<uint128_t>(vec_to_u128(config.start), (config.mode == PlaylistConfig::MOD_DECR) ? -vec_to_u128(config.step) : vec_to_u128(config.step), vec_to_u128(config.mask), config.stutterCount, config.recycleCount);
    }
#endif

    throw DPG_EInternal("unsupported modifier size");
}

///////////////////////////////////////////////////////////////////////////////

AbstModifier * ModifierBlock::MakeTableMod(const PlaylistConfig::TableMod& config)
{
    return new TableModifier(config.table, config.stutterCount);
}

///////////////////////////////////////////////////////////////////////////////

