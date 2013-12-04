/// @file
/// @brief Playlist engine implementation
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <boost/foreach.hpp>
#include <sstream>

#include "EngineHooks.h"
#include "FlowEngine.h"
#include "PlaylistInstance.h"
#include "AttackPlaylistInstance.h"
#include "StreamPlaylistInstance.h"
#include "ModifierBlock.h"
#include "PlaylistEngine.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

PlaylistEngine::~PlaylistEngine()
{
}

///////////////////////////////////////////////////////////////////////////////

void PlaylistEngine::UpdatePlaylist(handle_t handle, const Config_t& config) 
{
    ValidateConfig(config);
    mConfigMap[handle] = config;
    ClearDelayMap();
    LinkLoopInfoToFlows(mConfigMap[handle]);
}

///////////////////////////////////////////////////////////////////////////////

const PlaylistEngine::Config_t* PlaylistEngine::GetPlaylist(handle_t handle) const 
{
    MapConstIter_t iter = mConfigMap.find(handle);
    if (iter == mConfigMap.end())
        return 0;

    return &(iter->second);
}

///////////////////////////////////////////////////////////////////////////////

void PlaylistEngine::DeletePlaylist(handle_t handle) 
{ 
    MapIter_t iter = mConfigMap.find(handle); 

    if (iter == mConfigMap.end())
        throw DPG_EInternal("unable to delete playlist with unknown handle");

    mConfigMap.erase(iter);
    ClearDelayMap();
}

///////////////////////////////////////////////////////////////////////////////

void PlaylistEngine::ValidateConfig(const Config_t& config) const
{
    BOOST_FOREACH(const Flow_t& flow, config.flows)
    {
        if (mFlowEngine.GetFlow(flow.flowHandle) == 0)
        {
            std::ostringstream oss;
            oss << "[PlaylistEngine::ValidateConfig] Could not find flow with handle " << flow.flowHandle;
            const std::string err(oss.str());
            EngineHooks::LogErr(0, err);
            throw DPG_EInternal(err);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

ModifierBlock * PlaylistEngine::MakeModifierBlock(handle_t handle, uint32_t randomSeed)
{
    const PlaylistConfig * pc = GetPlaylist(handle);
    if (!pc)
    {
        std::ostringstream oss;
        oss << "[PlaylistEngine::MakeModifierBlock] Could not find playlist with handle " << handle;
        const std::string err(oss.str());
        EngineHooks::LogErr(0, err);
        throw DPG_EInternal(err);
    }

    return new ModifierBlock(*pc, randomSeed);
}
   
///////////////////////////////////////////////////////////////////////////////

PlaylistInstance * PlaylistEngine::MakePlaylist(handle_t handle, const ModifierBlock& modifiers, AbstSocketManager& socketMgr, bool client)
{
    // create an individual playlist instance 

    const PlaylistConfig * pc = GetPlaylist(handle);
    if (!pc)
    {
        std::ostringstream oss;
        oss << "[PlaylistEngine::MakePlaylist] Could not find playlist with handle " << handle;
        const std::string err(oss.str());
        EngineHooks::LogErr(0, err);
        throw DPG_EInternal(err);
    }

    PlaylistInstance * pi;
    if (pc->type == PlaylistConfig::ATTACK_ONLY)
    {
        pi = new AttackPlaylistInstance(*this, socketMgr, *pc, (const ModifierBlock&)modifiers, client);
    }
    else if (pc->type == PlaylistConfig::STREAM_ONLY)
    {
        pi = new StreamPlaylistInstance(*this, socketMgr, *pc, handle, (const ModifierBlock&)modifiers, client);
    }
    else
    {
        std::ostringstream oss;
        oss << "[PlaylistEngine::MakePlaylist] Playlist with wrong type configured " << handle;
        const std::string err(oss.str());
        EngineHooks::LogErr(0, err);
        throw DPG_EInternal(err);
    }
    return pi;
}

///////////////////////////////////////////////////////////////////////////////

PlaylistInstance * PlaylistEngine::ClonePlaylist(const PlaylistInstance * parent, const ModifierBlock& modBlock)
{
    return parent->Clone();
}

///////////////////////////////////////////////////////////////////////////////

void PlaylistEngine::LinkLoopInfoToFlows(PlaylistConfig &playlist)
{
    // clear loop info in the flows
    // maybe not necessary?
    BOOST_FOREACH (PlaylistConfig::Flow & flow, playlist.flows)
    {
        flow.loopInfo = 0;
    }
    for (LoopMapIter_t iter_loop = playlist.loopMap.begin(); iter_loop!=playlist.loopMap.end(); iter_loop++)
    {
        uint16_t endIdx = iter_loop->first;
        PlaylistConfig::LoopInfo & loop = iter_loop->second;
        uint16_t begIdx = loop.begIdx;

        // append the pointer to the loop to EACH flow in the loop
        for (uint16_t idx = begIdx; idx <= endIdx; idx++)
        {
            playlist.flows[idx].loopInfo = &loop;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
