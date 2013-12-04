/// @file
/// @brief Stream playlist instance header file 
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _STREAM_PLAYLIST_INSTANCE_H_
#define _STREAM_PLAYLIST_INSTANCE_H_

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
class StreamLoopHandler;

///////////////////////////////////////////////////////////////////////////////

class StreamPlaylistInstance : public PlaylistInstance
{
  public:

    StreamPlaylistInstance(PlaylistEngine& pEngine, AbstSocketManager& socketMgr, const PlaylistConfig& config, handle_t handle, const ModifierBlock& modifiers, bool client);

    void Start();

    bool OnClose(size_t flowIndex, AbstSocketManager::CloseType closeType);

    void FlowDone(size_t flowIndex, bool successful);

    StreamPlaylistInstance * Clone() const;

  UNIT_TEST_PRIVATE:
    void StartData(size_t flowIndex);

    handle_t mHandle;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
