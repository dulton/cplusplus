/// @file
/// @brief Attack loop handler implementation
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
#include <sstream>

#include "AttackLoopHandler.h"
#include "FlowInstance.h"
#include "FlowEngine.h"
#include "EngineHooks.h"
#include "AbstSocketManager.h"
#include "AbstTimerManager.h"
#include "PlaylistConfig.h"
#include "PlaylistEngine.h"
#include "PlaylistInstance.h"
#include "AttackInstance.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

AttackLoopHandler::AttackLoopHandler(AttackPlaylistInstance *playlist, const PlaylistConfig::LoopInfo &loopInfo, uint16_t endIdx)
                        : FlowLoopHandler(loopInfo, endIdx),
                          mPlaylistInst(playlist)
{
}

///////////////////////////////////////////////////////////////////////////////

void AttackLoopHandler::StartLoop(uint16_t flowIndex)
{
    if (mStatus.state==FlowLoopHandler::WAIT_TO_START && flowIndex == mBeginFlowIdx)
    {
        mStatus.state = FlowLoopHandler::STARTED;
        mStatus.currFlowIdx = flowIndex;
        mPlaylistInst->ProcessAttack(flowIndex);
    }
    else
    {
        //error
        std::ostringstream oss;
        oss << "[AttackLoopHandler::StartLoop] Trying to start an attack loop twice.";
        const std::string err(oss.str());
        EngineHooks::LogErr(0, err);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AttackLoopHandler::FlowInLoopDone(uint16_t flowIdx)
{
    if (mStatus.numOfRemainedIterations == 1 && mStatus.numOfRemainedFlowsInThisIteration == 1)
    {
        // all iterations are done
        SetStatusDone(true);
        mStatus.currFlowIdx = mEndFlowIdx + 1;
        mPlaylistInst->NotifyLoopDone(mBeginFlowIdx);
        mPlaylistInst->DoNextAttack(true);
    }
    else if (mStatus.numOfRemainedFlowsInThisIteration > 1)
    {
        // if this iteration is not done, proceed to the next attack
        mStatus.numOfRemainedFlowsInThisIteration--;
        mStatus.currFlowIdx++;
        // close the socket and open socket for next attack
        mPlaylistInst->DoNextAttack(true);
    }
    else // this iteration is done
    {
        mStatus.numOfRemainedFlowsInThisIteration = mEndFlowIdx - mBeginFlowIdx + 1; // reset this
        mStatus.numOfRemainedIterations--;
        // close the socket and loop back
        mStatus.currFlowIdx = mBeginFlowIdx;
        mPlaylistInst->DoNextAttack(true);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AttackLoopHandler::FlowInLoopFailed(uint16_t flowIdx)
{
    SetStatusDone(false);
    mPlaylistInst->NotifyLoopFailed(mBeginFlowIdx);
    mPlaylistInst->DoNextAttack(false);
}

///////////////////////////////////////////////////////////////////////////////
