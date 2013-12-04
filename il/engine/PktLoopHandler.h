/// @file
/// @brief Packet loop handler header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.

#ifndef _PKT_LOOP_HANDLER_H_
#define _PKT_LOOP_HANDLER_H_

#include "FlowConfig.h"

class TestPktLoopHandler;

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////////

class PktLoopHandler
{
public:
    PktLoopHandler(const FlowConfig * config);
    ~PktLoopHandler() {};

    void Reset();
    uint16_t FindNextPktIdx(const std::vector<uint16_t> & idxVector, uint16_t currIdx);

private:

    const FlowConfig * mConfig;
    struct LoopCounter
    {
        uint16_t loopId; // end pkt index
        uint16_t count;
    };
    std::vector<LoopCounter> mPktLoopCounters;
    uint16_t mCurPktLoopId;

#ifdef UNIT_TEST
    friend class TestPktLoopHandler;
#endif
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
