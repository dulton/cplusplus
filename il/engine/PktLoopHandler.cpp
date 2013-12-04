/// @file
/// @brief Packet loop handler implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.

#include <sstream>

#include "PktLoopHandler.h"
#include "EngineHooks.h"

USING_L4L7_ENGINE_NS;

////////////////////////////////////////////////////////////////////////////////

PktLoopHandler::PktLoopHandler(const FlowConfig * config)
                  : mConfig(config),
                    mCurPktLoopId(0)
{
//     mPktLoopCounters.clear();
    if (mConfig)
    {
        mCurPktLoopId = mConfig->pktList.size();
    }
    else
    {
	const std::string err("PktLoopHandler initialized with no flow config.");
        EngineHooks::LogErr(0, err);
        throw DPG_EInternal(err);
    }
}

////////////////////////////////////////////////////////////////////////////////

void PktLoopHandler::Reset()
{
    mPktLoopCounters.clear();
    
    if (mConfig)
    {
        mCurPktLoopId = mConfig->pktList.size();
    }
    else
    {
    	// error
	const std::string err("PktLoopHandler initialized with no flow config.");
        EngineHooks::LogErr(0, err);
        throw DPG_EInternal(err);
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// This routine checks the pkt with idx currIdx in the idxVector and returns the next index
//
uint16_t PktLoopHandler::FindNextPktIdx(const std::vector<uint16_t> & idxVector, uint16_t currIdx)
{
   uint16_t nextIdx;
   const FlowConfig::Packet& packet(mConfig->pktList[idxVector[currIdx]]);

    // if this is the end of a packet loop
    if (packet.loopInfo)
    {
        // first iteration
        if  (mCurPktLoopId != currIdx)
        {
            if (packet.loopInfo->count > 1)
            {
                LoopCounter counter = {currIdx, packet.loopInfo->count-1};
                mPktLoopCounters.push_back(counter); // already iterated once
                nextIdx = packet.loopInfo->begIdx;
                mCurPktLoopId = currIdx; // set the flag that we're currently doing THIS loop
            }
            else // only need to "repeat" this packet once, so not actually a "loop"
            {
                nextIdx = currIdx + 1;
                return nextIdx;
            }
        }
        // last iteration
        else if (mPktLoopCounters.back().count == 1)
        {
            nextIdx = currIdx + 1;
            mPktLoopCounters.pop_back();
            if (mPktLoopCounters.size())
            {
                mCurPktLoopId = mPktLoopCounters.back().loopId; // the outer loop
            }
            else
            {
                mCurPktLoopId = idxVector.size(); // set to some invalid index
            }
        }
        // need more iteration
        else
        {
            nextIdx = packet.loopInfo->begIdx;
            mPktLoopCounters[mPktLoopCounters.size()-1].count = mPktLoopCounters.back().count - 1;
        }
    }
    else
    {
        nextIdx = currIdx + 1;
    }

    return nextIdx;
}

////////////////////////////////////////////////////////////////////////////////
