/// @file
/// @brief Mock playlist instance
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _MOCK_PLAYLIST_INSTANCE_PROXY_H_
#define _MOCK_PLAYLIST_INSTANCE_PROXY_H_

#include <engine/PlaylistInstance.h>

///////////////////////////////////////////////////////////////////////////////

class MockPlaylistInstance : public L4L7_ENGINE_NS::PlaylistInstance
{
public:
    MockPlaylistInstance(L4L7_ENGINE_NS::PlaylistEngine& pEngine, L4L7_ENGINE_NS::AbstSocketManager& socketMgr, const L4L7_ENGINE_NS::PlaylistConfig& config, bool client) :
        L4L7_ENGINE_NS::PlaylistInstance(pEngine, socketMgr, config, L4L7_ENGINE_NS::ModifierBlock(config, 0), client),
        mStarted(false)
    {
    }

    void Start ()
    {
        mStarted = true;
    }

    void FlowDone(size_t flowIndex, bool successful) {}

    MockPlaylistInstance * Clone() const { return 0; }

    bool OnClose(size_t flowIndex, L4L7_ENGINE_NS::AbstSocketManager::CloseType closeType) { return true; }

    void StartData(size_t flowIndex) {}

    // Mock getters

    bool IsStarted() { return mStarted; }

private:
    bool mStarted;
};

///////////////////////////////////////////////////////////////////////////////

#endif

