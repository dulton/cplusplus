/// @file
/// @brief Stream instance header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _STREAM_INSTANCE_H_
#define _STREAM_INSTANCE_H_

#include "PlaylistEngine.h"

class TestStreamInstance;

#ifdef UNIT_TEST
#define UNIT_TEST_VIRTUAL virtual
#define UNIT_TEST_PRIVATE protected
#else
#define UNIT_TEST_VIRTUAL
#define UNIT_TEST_PRIVATE private
#endif

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

class StreamInstance: public FlowInstance
{
  public:

    StreamInstance(FlowEngine& engine, handle_t flowHandle, PlaylistInstance *pi, size_t flowIdx,  PlaylistEngine::PktDelays_t *pktDelays, bool client, flowLpHdlrSharedPtr_t loopHandler) :
                                 FlowInstance(engine, flowHandle, pi, flowIdx, client, loopHandler),
                                 mAdjustedPktDelays(pktDelays)
    {
        mPktLoopCounters.clear();
        mCurPktLoopId = mConfig->pktList.size();
        mNextPktIdx = mConfig->pktList.size();
    };

    void Start();

  UNIT_TEST_PRIVATE:
    void Reset();
    void DoNextPacket();
    void Wait(uint32_t delay);
    void CancelTimeoutAndWait(uint32_t delay);
    void PacketReceived();
    const FlowConfig::Packet * GetNextPkt();
    void UpdateCurrentPktIdx();

    PlaylistEngine::PktDelays_t * mAdjustedPktDelays;

private:

#ifdef UNIT_TEST
    friend class TestStreamInstance;
#endif
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
