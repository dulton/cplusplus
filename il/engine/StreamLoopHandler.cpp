/// @file
/// @brief Stream loop handler implementation
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

#include "StreamLoopHandler.h"
#include "FlowInstance.h"
#include "FlowEngine.h"

#include "AbstSocketManager.h"
#include "AbstTimerManager.h"
#include "PlaylistConfig.h"
#include "PlaylistEngine.h"
#include "PlaylistInstance.h"
#include "StreamInstance.h"

USING_L4L7_ENGINE_NS;

/////////////////////////////////////////////////////////////////////////////

StreamLoopHandler::StreamLoopHandler(StreamPlaylistInstance *playlist, const PlaylistConfig::LoopInfo &loopInfo, uint16_t endIdx)
                        : FlowLoopHandler(loopInfo, endIdx),
                          mPlaylistInst(playlist)
{
    mRelativeDataDelay.resize(mPlaylistInst->mConfig.flows.size(), 0);
}

///////////////////////////////////////////////////////////////////////////////

void StreamLoopHandler::StartLoop(uint16_t flowIndex)
{
    if (mStatus.state==FlowLoopHandler::WAIT_TO_START)
    {
        mStatus.state = FlowLoopHandler::STARTED;
        mStatus.lastStartTime = mPlaylistInst->GetCurrTimeMsec();
        mIdxOfFirstFlowInst = flowIndex;
        mRelativeDataDelay[flowIndex] = 0; // not necessary but not hurt to do this
    }
    else
    {
        uint32_t delay = mPlaylistInst->GetCurrTimeMsec() - mStatus.lastStartTime;
        mRelativeDataDelay[flowIndex] = delay;
    }
    mPlaylistInst->GetFlow(flowIndex).Start();
}

///////////////////////////////////////////////////////////////////////////////

void StreamLoopHandler::FlowInLoopDone(uint16_t flowIdx)
{
    if (mStatus.numOfRemainedIterations == 1 && mStatus.numOfRemainedFlowsInThisIteration == 1)
    {
        // all iterations are done
        SetStatusDone(true);
        mPlaylistInst->NotifyLoopDone(mBeginFlowIdx);
    }
    else if (mStatus.numOfRemainedFlowsInThisIteration > 1)
    {
        // if this iteration is not done, wait
        mStatus.numOfRemainedFlowsInThisIteration--;
    }
    else // this iteration is done
    {
        mStatus.numOfRemainedFlowsInThisIteration = mEndFlowIdx - mBeginFlowIdx + 1; // reset this
        mStatus.numOfRemainedIterations--;
        // schedule next iteration
        for (uint16_t idx = mBeginFlowIdx; idx <= mEndFlowIdx; idx++)
        {
            FlowInstance &flowInst = mPlaylistInst->GetFlow(idx);
            if (flowInst.GetIndex() == mIdxOfFirstFlowInst)
            {
                WaitToRepeatData(idx, DELAY_IN_MS_BETWEEN_STREAMLOOP_ITERATIONS);
            }
            else
            {
                WaitToRepeatData(idx, mRelativeDataDelay[flowInst.GetIndex()] + DELAY_IN_MS_BETWEEN_STREAMLOOP_ITERATIONS);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void StreamLoopHandler::FlowInLoopFailed(uint16_t flowIdx)
{
    SetStatusDone(false);
    mPlaylistInst->NotifyLoopFailed(mBeginFlowIdx);
}

///////////////////////////////////////////////////////////////////////////////

void StreamLoopHandler::WaitToRepeatData(uint16_t flowIndex, uint32_t delay)
{

    if(mPlaylistInst->mForceStop)
    {
       mPlaylistInst->mTimers[flowIndex] = mPlaylistInst->mEngine.GetTimerMgr().InvalidTimerHandle();
       return;
    }
    AbstTimerManager::timerDelegate_t delegate(boost::bind(&StreamLoopHandler::RepeatData, this, flowIndex));

    handle_t timerHandle = mPlaylistInst->mEngine.GetTimerMgr().CreateTimer(delegate, delay);
    // playlist instance only maintains one timer handle for each flow instance.
    // Yet it's fine, because start timer and data delay timer do not exist at the same time.
    mPlaylistInst->mTimers[flowIndex] = timerHandle;
}

///////////////////////////////////////////////////////////////////////////////

void StreamLoopHandler::RepeatData(uint16_t flowIndex)
{
    if(mPlaylistInst)
    {
        mPlaylistInst->mTimers[flowIndex] = mPlaylistInst->mEngine.GetTimerMgr().InvalidTimerHandle();
        if (mStatus.state <= (int)FlowLoopHandler::STARTED && mPlaylistInst->GetState() <= (int)PlaylistInstance::STARTED)
        {
           mPlaylistInst->mFlows.at(flowIndex)->Start();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
