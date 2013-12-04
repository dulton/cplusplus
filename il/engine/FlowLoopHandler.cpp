/// @file
/// @brief Flow loop handler implementation
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <boost/bind.hpp>

#include "FlowLoopHandler.h"
#include "FlowInstance.h"
#include "FlowEngine.h"

#include "AbstSocketManager.h"
#include "AbstTimerManager.h"
#include "PlaylistConfig.h"
#include "PlaylistEngine.h"
#include "PlaylistInstance.h"

USING_L4L7_ENGINE_NS;

//////////////////////////////////////////////////////////////////////////////////

FlowLoopHandler::FlowLoopHandler(const PlaylistConfig::LoopInfo & loopInfo, uint16_t endIdx)
                                : mBeginFlowIdx(loopInfo.begIdx),
                                  mEndFlowIdx(endIdx),
                                  mLoopInfo(loopInfo),
                                  mMaxCount(loopInfo.count),
                                  mIdxOfFirstFlowInst(0)
{
    ResetStatus();
}

///////////////////////////////////////////////////////////////////////////////

void FlowLoopHandler::ResetStatus(void)
{
    mStatus.state = WAIT_TO_START;
    mStatus.numOfRemainedIterations = mMaxCount;
    mStatus.numOfRemainedFlowsInThisIteration = mEndFlowIdx - mBeginFlowIdx + 1;
    mStatus.lastStartTime = 0;
    mStatus.lastStopTime = 0;
    mStatus.currFlowIdx = mEndFlowIdx + 1; // some value out of loop scope indicats inactive loop
}

///////////////////////////////////////////////////////////////////////////////

void FlowLoopHandler::SetStatusDone(bool successful)
{
    ResetStatus();
    mStatus.state = (successful? SUCCESSFUL : FAILED);
}

///////////////////////////////////////////////////////////////////////////////
