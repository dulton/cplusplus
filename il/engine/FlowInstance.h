/// @file
/// @brief Flow instance header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _FLOW_INSTANCE_H_
#define _FLOW_INSTANCE_H_

#include <boost/tr1/memory.hpp>
#include <engine/FlowConfig.h>
#include <engine/AbstSocketManager.h>
#include <engine/EngineCommon.h>
#include <engine/EngineLock.h>

#define DPG_FLOW_INACTIVITY_TIMEOUT 100000

class TestStreamInstance;
class TestAttackInstance;
class TestPktLoopHandler;
class TestFlowLoopHandler;

DECL_L4L7_ENGINE_NS

class FlowEngine;
class PlaylistInstance;
class FlowLoopHandler;

///////////////////////////////////////////////////////////////////////////////

class FlowInstance
{
  public:
    typedef enum {
        WAIT_TO_START,
        STARTED,
        ABORTED,
        DONE, //data transmission done
        FAIL_TO_CONNECT // connect attempt fails
    } FlowState;

    typedef std::tr1::shared_ptr<FlowLoopHandler> flowLpHdlrSharedPtr_t;

    FlowInstance(FlowEngine& engine, handle_t flowHandle, PlaylistInstance *pi, size_t flowIdx, bool client, flowLpHdlrSharedPtr_t loopHandler);

#ifdef UNIT_TEST
    FlowInstance(const FlowInstance& other);
#endif

    virtual ~FlowInstance();

    bool IsClient()
    {
        return mClient;
    }

    void SetFd(int fd) { mFd = fd; }

    int  GetFd() const { return mFd; } 

    handle_t GetHandle() const { return mFlowHandle; }

    size_t GetIndex() const { return mFlowIndex; };

    const FlowConfig& GetConfig() const { return *mConfig; }

    FlowState GetState() const { return mState; }
    void SetState(FlowState st) { mState = st; }

    virtual void Start() = 0;

    void Stop();

    bool IsInFlowLoop(void) {return (mFlowLoopHandler.get()!=0);};

    flowLpHdlrSharedPtr_t GetFlowLoopHandler(void) {return mFlowLoopHandler;};

  protected:
    void SetupInactivityTimer();
    void SetupInactivityTimer(uint32_t timeout);
    void CancelInactivityTimer();
    void CancelTxRxTimer();
    virtual void DoNextPacket() = 0;
    void Complete();
    void Failed();
    void FlowTimeout();
    void SendPacket(const FlowConfig::Packet* packet, AbstSocketManager::sendDelegate_t& delegate);
    size_t GetPacketLength(const FlowConfig::Packet* packet);

    FlowEngine& mEngine;
    handle_t mFlowHandle; // help find the config in the flow engine
    size_t mFlowIndex; // help find the flow instance in the palylist instance
    const FlowConfig* mConfig;
    PlaylistInstance *mPlaylistInstance; // point back to its playlist instance
    bool mClient;
    int mFd;
    size_t mPacketIdx;

    handle_t mTimer; // in streams, for Rx; in attacks, for Tx
    FlowState mState;

    struct LoopCounter
    {
        uint16_t loopId; // end pkt index
        uint16_t count;
    };
    std::vector<LoopCounter> mPktLoopCounters;
    uint16_t mCurPktLoopId;
    size_t mNextPktIdx;

    flowLpHdlrSharedPtr_t  mFlowLoopHandler;

#ifdef UNIT_TEST
    friend class TestStreamInstance;
    friend class TestAttackInstance;
    friend class TestPktLoopHandler;
    friend class TestFlowLoopHandler;
#endif

  private:
    handle_t mInactivityTimer;

    typedef RecursiveThreadMutex lock_t;
    lock_t * mInactivityTimerLock;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
