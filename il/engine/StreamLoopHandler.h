/// @file
/// @brief Stream loop handler header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _STREAM_LOOP_HANDLER_H_
#define _STREAM_LOOP_HANDLER_H_

#include <vector>

#include <engine/EngineCommon.h>
#include <engine/PlaylistInstance.h>
#include "FlowLoopHandler.h"
#include "StreamPlaylistInstance.h"

DECL_L4L7_ENGINE_NS

class StreamLoopHandler : public FlowLoopHandler
{
public:
    StreamLoopHandler(StreamPlaylistInstance *playlist, const PlaylistConfig::LoopInfo &loopInfo, uint16_t endIdx);

    ~StreamLoopHandler(){};

    // virtual ones
    void StartLoop(uint16_t flowIndex);
    void FlowInLoopDone(uint16_t flowIdx);
    void FlowInLoopFailed(uint16_t flowIdx);

    void WaitToRepeatData(uint16_t flowIndex, uint32_t delay);
    void RepeatData(uint16_t flowIndex);

private:

    StreamPlaylistInstance      *mPlaylistInst; // a pointer to the parent playlist instance
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif

