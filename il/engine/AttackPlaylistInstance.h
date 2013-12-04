/// @file
/// @brief Attack playlist instance header file 
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ATTACK_PLAY_INSTANCE_H_
#define _ATTACK_PLAY_INSTANCE_H_

#include <set>
#include <vector>

#include <boost/tr1/memory.hpp>

#include <engine/EngineCommon.h>
#include <engine/FlowInstance.h>
#include <engine/PlaylistInstance.h>

class TestPlaylistInstance;

DECL_L4L7_ENGINE_NS

class AbstSocketManager;
class PlaylistConfig;
class PlaylistEngine;

///////////////////////////////////////////////////////////////////////////////

class AttackPlaylistInstance : public PlaylistInstance
{
  public:
    AttackPlaylistInstance(PlaylistEngine& pEngine, AbstSocketManager& socketMgr, const PlaylistConfig& config, const ModifierBlock& modifiers, bool client);

    void Start();

    bool OnClose(size_t flowIndex, AbstSocketManager::CloseType closeType);

    void FlowDone(size_t flowIndex, bool successful);

    AttackPlaylistInstance * Clone() const;
    void StartData(size_t flowIndex);

    uint16_t GetAtkLoopIteration(size_t flowIndex);

    bool IsAttack() 
    { 
        return true; 
    }

  private:

    void DoNextAttack(bool currSuccessful);
    void CloseSocket(uint16_t flowIndex, bool successful);
    void ProcessAttack(size_t flowIndex);

    uint16_t mLoopIterOverride;

UNIT_TEST_PRIVATE:
    size_t mCurrAttackIdx; // If playing an attack loop, this should be consistant with the one in the active loop handler.

  friend class AttackLoopHandler;

#ifdef UNIT_TEST
    friend class TestPlaylistInstance;
#endif
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
