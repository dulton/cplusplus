/// @file
/// @brief Flow loop handler header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _FLOW_LOOP_HANDLER_H_
#define _FLOW_LOOP_HANDLER_H_

#include <vector>

#include <engine/EngineCommon.h>
#include <engine/PlaylistInstance.h>

#define DELAY_IN_MS_BETWEEN_STREAMLOOP_ITERATIONS 1

class TestFlowLoopHandler;

DECL_L4L7_ENGINE_NS

class FlowLoopHandler
{
public:

    enum FlowLoopState {
        WAIT_TO_START,
        STARTED,
        SUCCESSFUL,
        FAILED,
	CLOSING
    };

    FlowLoopHandler(const PlaylistConfig::LoopInfo & loopInfo, uint16_t endIdx);
    virtual ~FlowLoopHandler(){};

    uint16_t GetBeginIdx() {return mBeginFlowIdx;};
    uint16_t GetEndIdx() {return mEndFlowIdx;};
    uint16_t GetCurrAttackIdx(void) {return mStatus.currFlowIdx;};
    uint16_t GetCurrIteration() {return mMaxCount - mStatus.numOfRemainedIterations;}
    bool     IsLastIteration() {return mStatus.numOfRemainedIterations == 1;}
    FlowLoopState GetState() {return mStatus.state;};
    void     SetClosing() {mStatus.state = CLOSING;};

    virtual void StartLoop(uint16_t flowIndex) = 0;
    virtual void FlowInLoopDone(uint16_t flowIdx) = 0;
    virtual void FlowInLoopFailed(uint16_t flowIdx) = 0;

protected:
    void ResetStatus(void);
    void SetStatusDone(bool successful);

    uint16_t                    mBeginFlowIdx;
    uint16_t                    mEndFlowIdx;
    const PlaylistConfig::LoopInfo &  mLoopInfo; // maybe not necessary
    uint16_t                    mMaxCount; // the loop count from the configuration
    uint16_t                    mIdxOfFirstFlowInst; // flow instance idx got from the first iteration
    std::vector<uint32_t>       mRelativeDataDelay; // indexed by flow instance, data delay relative to the first flow

    struct FlowLoopStatus
    {
        FlowLoopState   state;
        uint16_t        numOfRemainedIterations;
        uint16_t        numOfRemainedFlowsInThisIteration;
        uint32_t        lastStartTime; // for streams only
        uint32_t        lastStopTime;  // for streams only
        uint16_t        currFlowIdx;  // for attacks only
    };
    FlowLoopStatus              mStatus;

#ifdef UNIT_TEST
    friend class TestFlowLoopHandler;
#endif

};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif

