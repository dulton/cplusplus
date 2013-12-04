/// @file
/// @brief Playlist engine header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _PLAYLIST_ENGINE_H_
#define _PLAYLIST_ENGINE_H_

#include <map>
#include <vector>

#include <engine/EngineCommon.h>
#include <engine/PlaylistConfig.h>
#include <engine/PktDelayMap.h>

#ifdef UNIT_TEST
#define UNIT_TEST_VIRTUAL virtual
#else
#define UNIT_TEST_VIRTUAL 
#endif

DECL_L4L7_ENGINE_NS

class FlowEngine;

class PlaylistInstance;

class AbstSocketManager;
class AbstTimerManager;
class ModifierBlock;

///////////////////////////////////////////////////////////////////////////////

class PlaylistEngine
{
  public:
    // public typedefs
    typedef PlaylistConfig Config_t; 
    typedef PlaylistConfig::Flow Flow_t;

    PlaylistEngine(FlowEngine& flowEngine, AbstTimerManager& timerMgr) :
        mFlowEngine(flowEngine),
        mTimerMgr(timerMgr) {}

    UNIT_TEST_VIRTUAL ~PlaylistEngine();

    // Config -- should happen when the playlist is stopped

    void UpdatePlaylist(handle_t handle, const Config_t& config);

    void DeletePlaylist(handle_t handle);
   
    void UpdatedFlows() { ClearDelayMap(); } 

    const Config_t* GetPlaylist(handle_t handle) const;

    // Instantiate a new variable state object -- playlists subsequently 
    // created with this object will modify their variables in sequence
    
    ModifierBlock * MakeModifierBlock(handle_t handle, uint32_t randSeed); 

    // Instantiate a Playlist for playing
    
    UNIT_TEST_VIRTUAL PlaylistInstance * MakePlaylist(handle_t handle, const ModifierBlock& modBlock, AbstSocketManager& socketMgr, bool client);

    // Make a new Playlist identical to the given playlist at its creation - not a strict copy
    
    UNIT_TEST_VIRTUAL PlaylistInstance * ClonePlaylist(const PlaylistInstance * parent, const ModifierBlock& modBlock);

    // Accessors -- for use only by PlaylistInstance

    FlowEngine& GetFlowEngine() { return mFlowEngine; }

    AbstTimerManager& GetTimerMgr() { return mTimerMgr; }
    
    typedef PktDelayMap::delays_t PktDelays_t;
    PktDelayMap& GetPktDelayMap() { return mPktDelayMap; }

  protected:
    UNIT_TEST_VIRTUAL void ValidateConfig(const Config_t& config) const;

  private:
    void LinkLoopInfoToFlows(PlaylistConfig &playlist);

    void ClearDelayMap() { mPktDelayMap.clear(); }

    typedef std::map<handle_t, Config_t> ConfigMap_t;
    typedef ConfigMap_t::iterator MapIter_t;
    typedef ConfigMap_t::const_iterator MapConstIter_t;
    typedef std::map<uint16_t, PlaylistConfig::LoopInfo>::iterator LoopMapIter_t;

    ConfigMap_t mConfigMap;

    FlowEngine& mFlowEngine;
    AbstTimerManager& mTimerMgr;

    // This is to cache the adjusted packet delays for each flow in a context of a particular playlist.
    // Filled in by StreamPlaylistInstance and used by StreamInstance.
    // Attacks do not need this.
    PktDelayMap mPktDelayMap;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
