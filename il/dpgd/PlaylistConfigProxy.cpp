/// @file
/// @brief Playlist config proxy implementation
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <set>

#include <boost/foreach.hpp>
#include <boost/static_assert.hpp>

#include <engine/PlaylistConfig.h>

#include "DpgdLog.h"
#include "PlaylistConfigProxy.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

PlaylistConfigProxy::PlaylistConfigProxy(const playlistConfig_t& config)
    : mConfig(config)
{
    if (!mConfig.Streams.empty() && !mConfig.Attacks.empty())
    {
        std::string err("Streams and attacks can't be mixed");
        TC_LOG_ERR(0, err);
        throw EPHXBadConfig(err);
    }
    ValidateFlows();
    ValidateLoops();
    ValidateVars();
    ValidateMods();
    ValidateLinks();
}

///////////////////////////////////////////////////////////////////////////////

void PlaylistConfigProxy::ValidateFlows()
{
    BOOST_FOREACH(const streamRef_t& stream, mConfig.Streams)
    {
      if (stream.StreamParam.CloseType == DpgIf_1_port_server::CLOSE_TYPE_C_RST || 
            stream.StreamParam.CloseType == DpgIf_1_port_server::CLOSE_TYPE_S_RST)
        {
            std::string err("C_RST and S_RST close types not supported");
            TC_LOG_ERR(0, err);
            throw EPHXBadConfig(err);
        }

        if (stream.StreamParam.MinPktDelay > stream.StreamParam.MaxPktDelay)
        {
            std::string err("Min pkt delay > max pkt delay");
            TC_LOG_ERR(0, err);
            throw EPHXBadConfig(err);
        }

        if (stream.FlowParam.ServerPort == 0)
        {
            std::string err("Server port is zero");
            TC_LOG_ERR(0, err);
            throw EPHXBadConfig(err);
        }
    }

    BOOST_FOREACH(const attackRef_t& attack, mConfig.Attacks)
    {
        if (attack.AttackParam.InactivityTimeout < 1000)
        {
            std::string err("Inactivity timeout is less than 1s");
            TC_LOG_ERR(0, err);
            throw EPHXBadConfig(err);
        }
        if (attack.FlowParam.ServerPort == 0)
        {
            std::string err("Server port is zero");
            TC_LOG_ERR(0, err);
            throw EPHXBadConfig(err);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void PlaylistConfigProxy::ValidateLoops()
{
    uint16_t lastIdx = 0;

    BOOST_FOREACH(const loopConfig_t& loop, mConfig.Loops)
    {
        if (loop.BegIdx > loop.EndIdx)
        {
            std::string err("Loop indices reversed");
            TC_LOG_ERR(0, err);
            throw EPHXBadConfig(err);
        }

        if (lastIdx && loop.BegIdx <= lastIdx)
        {
            std::string err("Loops overlap");
            TC_LOG_ERR(0, err);
            throw EPHXBadConfig(err);
        } 

        if (loop.EndIdx >= std::max(mConfig.Streams.size(), mConfig.Attacks.size())) 
        {
            std::string err("Loop indices out of range");
            TC_LOG_ERR(0, err);
            throw EPHXBadConfig(err);
        }

        lastIdx = loop.EndIdx;
    }
}

///////////////////////////////////////////////////////////////////////////////

void PlaylistConfigProxy::ValidateVars()
{
    BOOST_FOREACH(const streamRef_t& stream, mConfig.Streams)
    {
        bool first = true;
        uint8_t lastVarIdx = 0;
        BOOST_FOREACH(const varConfig_t& var, stream.Vars)
        {
            if (var.VarIdx < lastVarIdx)
            {
                std::string err("Variable index out of order");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }

            if (!first && var.VarIdx == lastVarIdx)
            {
                std::string err("Multiple variables with the same index");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }
            lastVarIdx = var.VarIdx;
            first = false;
        }
    }

    BOOST_FOREACH(const attackRef_t& attack, mConfig.Attacks)
    {
        bool first = true;
        uint8_t lastVarIdx = 0;
        BOOST_FOREACH(const varConfig_t& var, attack.Vars)
        {
            if (var.VarIdx < lastVarIdx)
            {
                std::string err("Variable index out of order");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }

            if (!first && var.VarIdx == lastVarIdx)
            {
                std::string err("Multiple variables with the same index");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }
            lastVarIdx = var.VarIdx;
            first = false;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void PlaylistConfigProxy::ValidateMods()
{
    BOOST_FOREACH(const streamRef_t& stream, mConfig.Streams)
    {
        uint8_t lastModIdx = 0;
        std::set<uint8_t> indexSet;
        BOOST_FOREACH(const rangeMod_t& mod, stream.RangeMods)
        {
            if (mod.mode == Modifiers_1_port_server::CONFIG_MOD_MODE_RANDOM)
            {
                // random uses initialValue to hold the 4 byte seed 
                // and doesn't use stepValue at all

                if (mod.initialValue.data.size() != 4)
                {
                    std::string err("Random modifier seed size invalid (initialValue should be 4 bytes)");
                    TC_LOG_ERR(0, err);
                    throw EPHXBadConfig(err);
                }

                if (!mod.stepValue.data.empty())
                {
                    std::string err("Random modifier contains invalid step value (should be empty)");
                    TC_LOG_ERR(0, err);
                    throw EPHXBadConfig(err);
                }
            }
            else
            {
                if (mod.initialValue.data.size() != mod.stepValue.data.size())
                {
                    std::string err("Modifier start and step size mismatch");
                    TC_LOG_ERR(0, err);
                    throw EPHXBadConfig(err);
                }

                if (mod.initialValue.data.size() != mod.mask.data.size())
                {
                    std::string err("Modifier start and mask size mismatch");
                    TC_LOG_ERR(0, err);
                    throw EPHXBadConfig(err);
                }
            }

            if (mod.modIndex < lastModIdx)
            {
                std::string err("Modifier variable index out of order");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }

            if (indexSet.count(mod.modIndex))
            {
                std::string err("Multiple modifiers with the same index");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }
            lastModIdx = mod.modIndex;
            indexSet.insert(lastModIdx);
        }

        lastModIdx = 0;
        BOOST_FOREACH(const tableMod_t& mod, stream.TableMods)
        {
            if (mod.modIndex < lastModIdx)
            {
                std::string err("Modifier variable index out of order");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }

            if (indexSet.count(mod.modIndex))
            {
                std::string err("Multiple modifiers with the same index");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }

            lastModIdx = mod.modIndex;
            indexSet.insert(lastModIdx);
        }
    }

    BOOST_FOREACH(const attackRef_t& attack, mConfig.Attacks)
    {
        uint8_t lastModIdx = 0;
        std::set<uint8_t> indexSet;
        BOOST_FOREACH(const rangeMod_t& mod, attack.RangeMods)
        {
            if (mod.mode == Modifiers_1_port_server::CONFIG_MOD_MODE_RANDOM)
            {
                // random uses initialValue to hold the 4 byte seed 
                // and doesn't use stepValue at all

                if (mod.initialValue.data.size() != 4)
                {
                    std::string err("Random modifier seed size invalid (initialValue should be 4 bytes)");
                    TC_LOG_ERR(0, err);
                    throw EPHXBadConfig(err);
                }

                if (!mod.stepValue.data.empty())
                {
                    std::string err("Random modifier contains invalid step value (should be empty)");
                    TC_LOG_ERR(0, err);
                    throw EPHXBadConfig(err);
                }
            }
            else
            {
                if (mod.initialValue.data.size() != mod.stepValue.data.size())
                {
                    std::string err("Modifier start and step size mismatch");
                    TC_LOG_ERR(0, err);
                    throw EPHXBadConfig(err);
                }

                if (mod.initialValue.data.size() != mod.mask.data.size())
                {
                    std::string err("Modifier start and mask size mismatch");
                    TC_LOG_ERR(0, err);
                    throw EPHXBadConfig(err);
                }
            }

            if (mod.modIndex < lastModIdx)
            {
                std::string err("Modifier variable index out of order");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }

            if (indexSet.count(mod.modIndex))
            {
                std::string err("Multiple modifiers with the same index");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }
            lastModIdx = mod.modIndex;
            indexSet.insert(lastModIdx);
        }

        lastModIdx = 0;
        BOOST_FOREACH(const tableMod_t& mod, attack.TableMods)
        {
            if (mod.modIndex < lastModIdx)
            {
                std::string err("Modifier variable index out of order");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }

            if (indexSet.count(mod.modIndex))
            {
                std::string err("Multiple modifiers with the same index");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }

            lastModIdx = mod.modIndex;
            indexSet.insert(lastModIdx);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void PlaylistConfigProxy::ValidateLinks()
{
    // NOTE: loops (except for parent==child) are not detected here
    //       in case of a loop, none of the modifiers will be incremented
    //       b/c there will be no root to increment
    
    std::map<uint8_t, int> orderMap_t;

    BOOST_FOREACH(const streamRef_t& stream, mConfig.Streams)
    {
        bool first = true;
        uint8_t lastModIdx = 0;

        std::set<uint8_t> idxSet;
        BOOST_FOREACH(const rangeMod_t& mod, stream.RangeMods)
        {
            idxSet.insert(mod.modIndex);
        }
        BOOST_FOREACH(const tableMod_t& mod, stream.TableMods)
        {
            idxSet.insert(mod.modIndex);
        }

        BOOST_FOREACH(const linkDesc_t& link, stream.ModLinks)
        {
            if (!idxSet.count(link.modIndex) || !idxSet.count(link.childModIndex))
            {
                std::string err("Invalid modifier link index");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }

            if (link.modIndex == link.childModIndex)
            {
                std::string err("Modifier link index invalid (self linking)");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }

            if (link.modIndex < lastModIdx)
            {
                std::string err("Modifier link index out of order");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }

            if (!first && link.modIndex == lastModIdx)
            {
                std::string err("Multiple modifier links with the same index");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }

            lastModIdx = link.modIndex;
            first = false;
        }
    }

    BOOST_FOREACH(const attackRef_t& attack, mConfig.Attacks)
    {
        bool first = true;
        uint8_t lastModIdx = 0;

        std::set<uint8_t> idxSet;
        BOOST_FOREACH(const rangeMod_t& mod, attack.RangeMods)
        {
            idxSet.insert(mod.modIndex);
        }
        BOOST_FOREACH(const tableMod_t& mod, attack.TableMods)
        {
            idxSet.insert(mod.modIndex);
        }

        BOOST_FOREACH(const linkDesc_t& link, attack.ModLinks)
        {
            if (!idxSet.count(link.modIndex) || !idxSet.count(link.childModIndex))
            {
                std::string err("Invalid modifier link index");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }

            if (link.modIndex == link.childModIndex)
            {
                std::string err("Modifier link index invalid (self linking)");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }

            if (link.modIndex < lastModIdx)
            {
                std::string err("Modifier link index out of order");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }

            if (!first && link.modIndex == lastModIdx)
            {
                std::string err("Multiple modifier links with the same index");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }

            lastModIdx = link.modIndex;
            first = false;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void PlaylistConfigProxy::CopyTo(L4L7_ENGINE_NS::PlaylistConfig& copy) const
{
    copy.flows.resize(std::max(mConfig.Streams.size(), mConfig.Attacks.size()));
    size_t idx = 0;
    // For now, mixed playlists are not allowed.
    if (mConfig.Attacks.empty())
    {
        copy.type = L4L7_ENGINE_NS::PlaylistConfig::STREAM_ONLY;
    }
    else
    {
        copy.type = L4L7_ENGINE_NS::PlaylistConfig::ATTACK_ONLY;
    }

    BOOST_FOREACH(const streamRef_t& stream, mConfig.Streams)
    {
        // FIXME What about flow.uval.Stream.StreamParam.CloseType?
        copy.flows[idx].flowHandle  = stream.Handle;
        copy.flows[idx].startTime   = stream.StreamParam.StartTime;
        copy.flows[idx].dataDelay   = stream.StreamParam.DataDelay;
        copy.flows[idx].minPktDelay = stream.StreamParam.MinPktDelay;
        copy.flows[idx].maxPktDelay = stream.StreamParam.MaxPktDelay;
        copy.flows[idx].socketTimeout = 0;
        copy.flows[idx].serverPort  = stream.FlowParam.ServerPort;
        copy.flows[idx].reversed    = false;

        copy.flows[idx].varMap.clear();
        BOOST_FOREACH(const varConfig_t& var, stream.Vars)
        {
            copy.flows[idx].varMap[var.VarIdx] = var.Value;
        }

        copy.flows[idx].rangeModMap.clear();
        BOOST_FOREACH(const rangeMod_t& mod, stream.RangeMods)
        {
            copy.flows[idx].rangeModMap[mod.modIndex].start = mod.initialValue.data;
            copy.flows[idx].rangeModMap[mod.modIndex].mask = mod.mask.data;
            copy.flows[idx].rangeModMap[mod.modIndex].recycleCount = mod.recycleCount;
            copy.flows[idx].rangeModMap[mod.modIndex].stutterCount = mod.stutterCount;
            copy.flows[idx].rangeModMap[mod.modIndex].mode = (L4L7_ENGINE_NS::PlaylistConfig::ModMode)mod.mode;
            if (mod.mode != Modifiers_1_port_server::CONFIG_MOD_MODE_RANDOM)
            {
                copy.flows[idx].rangeModMap[mod.modIndex].step = mod.stepValue.data;
            }
            else
            {
                copy.flows[idx].rangeModMap[mod.modIndex].step.clear();
            }
        }

        copy.flows[idx].tableModMap.clear();
        BOOST_FOREACH(const tableMod_t& mod, stream.TableMods)
        {
            copy.flows[idx].tableModMap[mod.modIndex].table.clear();
            BOOST_FOREACH(const row_t& row, mod.data)
            {
                copy.flows[idx].tableModMap[mod.modIndex].table.push_back(row.data);
            }

            copy.flows[idx].tableModMap[mod.modIndex].stutterCount = mod.stutterCount;
        }

        copy.flows[idx].linkModMap.clear();
        BOOST_FOREACH(const linkDesc_t& link, stream.ModLinks)
        {
            copy.flows[idx].linkModMap[link.modIndex] = link.childModIndex;
        }
        ++idx;
    }

    BOOST_FOREACH(const attackRef_t& attack, mConfig.Attacks)
    {
        copy.flows[idx].flowHandle  = attack.Handle;
        copy.flows[idx].startTime   = 0;
        copy.flows[idx].dataDelay   = 0;
        copy.flows[idx].minPktDelay = attack.AttackParam.PktDelay;
        copy.flows[idx].maxPktDelay = attack.AttackParam.PktDelay;
        copy.flows[idx].socketTimeout = attack.AttackParam.InactivityTimeout;
        copy.flows[idx].serverPort  = attack.FlowParam.ServerPort;
        copy.flows[idx].reversed    = false;

        copy.flows[idx].varMap.clear();
        BOOST_FOREACH(const varConfig_t& var, attack.Vars)
        {
            copy.flows[idx].varMap[var.VarIdx] = var.Value;
        }

        copy.flows[idx].rangeModMap.clear();
        BOOST_FOREACH(const rangeMod_t& mod, attack.RangeMods)
        {
            copy.flows[idx].rangeModMap[mod.modIndex].start = mod.initialValue.data;
            copy.flows[idx].rangeModMap[mod.modIndex].mask = mod.mask.data;
            copy.flows[idx].rangeModMap[mod.modIndex].recycleCount = mod.recycleCount;
            copy.flows[idx].rangeModMap[mod.modIndex].stutterCount = mod.stutterCount;
            copy.flows[idx].rangeModMap[mod.modIndex].mode = (L4L7_ENGINE_NS::PlaylistConfig::ModMode)mod.mode;
            if (mod.mode != Modifiers_1_port_server::CONFIG_MOD_MODE_RANDOM)
            {
                copy.flows[idx].rangeModMap[mod.modIndex].step = mod.stepValue.data;
            }
            else
            {
                copy.flows[idx].rangeModMap[mod.modIndex].step.clear();
            }
        }

        copy.flows[idx].tableModMap.clear();
        BOOST_FOREACH(const tableMod_t& mod, attack.TableMods)
        {
            copy.flows[idx].tableModMap[mod.modIndex].table.clear();
            BOOST_FOREACH(const row_t& row, mod.data)
            {
                copy.flows[idx].tableModMap[mod.modIndex].table.push_back(row.data);
            }

            copy.flows[idx].tableModMap[mod.modIndex].stutterCount = mod.stutterCount;
        }

        copy.flows[idx].linkModMap.clear();
        BOOST_FOREACH(const linkDesc_t& link, attack.ModLinks)
        {
            copy.flows[idx].linkModMap[link.modIndex] = link.childModIndex;
        }
        ++idx;
    }

    copy.loopMap.clear();
    BOOST_FOREACH(const loopConfig_t& loop, mConfig.Loops)
    {
        L4L7_ENGINE_NS::PlaylistConfig::LoopInfo info;
        info.begIdx = loop.BegIdx;
        info.count  = loop.Count;

        copy.loopMap[loop.EndIdx] = info;
    }
}

///////////////////////////////////////////////////////////////////////////////

BOOST_STATIC_ASSERT((int)Modifiers_1_port_server::CONFIG_MOD_MODE_INCREMENT == (int)L4L7_ENGINE_NS::PlaylistConfig::MOD_INCR);
BOOST_STATIC_ASSERT((int)Modifiers_1_port_server::CONFIG_MOD_MODE_DECREMENT == (int)L4L7_ENGINE_NS::PlaylistConfig::MOD_DECR);
BOOST_STATIC_ASSERT((int)Modifiers_1_port_server::CONFIG_MOD_MODE_RANDOM == (int)L4L7_ENGINE_NS::PlaylistConfig::MOD_RANDOM);

///////////////////////////////////////////////////////////////////////////////
