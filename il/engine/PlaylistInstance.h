/// @file
/// @brief Playlist instance header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _PLAY_INSTANCE_H_
#define _PLAY_INSTANCE_H_

#include <set>
#include <vector>

#include <boost/tr1/memory.hpp>

#include <engine/EngineCommon.h>
#include <engine/FlowInstance.h>
#include <engine/AbstSocketManager.h>
#include <engine/ModifierBlock.h>
#include <engine/AbstTimerManager.h>
#include "PlaylistConfig.h"
#include "FlowLoopHandler.h"
#include "PlaylistEngine.h"

class TestPlaylistInstance;
class TestFlowLoopHandler;

#ifdef UNIT_TEST
#define UNIT_TEST_VIRTUAL virtual
#define UNIT_TEST_PRIVATE protected
#else
#define UNIT_TEST_VIRTUAL
#define UNIT_TEST_PRIVATE private
#endif

DECL_L4L7_ENGINE_NS

class FlowEngine;

///////////////////////////////////////////////////////////////////////////////

class PlaylistInstance
{
  public:
    typedef boost::function1<void, PlaylistInstance *> stoppedDelegate_t;
    typedef boost::function0<void> statDelegate_t;

    enum PlaylistState {
        WAIT_TO_START,
        STARTED,
        // Stopping/stopped states after this point
        STOPPING,
        ABORTED,
        SUCCESSFUL,
        FAILED,
        FAIL_TO_CONNECT // a flow failed to connect
    };

    PlaylistInstance(PlaylistEngine& pEngine, AbstSocketManager& socketMgr, const PlaylistConfig& config, const ModifierBlock& modifiers, bool client); 

    bool IsClient()
    {
        return mClient;
    }

    void AddReference()
    {
        ++mRefCount;
    }

    void RemoveReference()
    {
        if (--mRefCount == 0)
            delete this;
    }

    void SetSerial(uint32_t serial)
    {
        mSerial = serial;
    }

    uint32_t GetSerial() const
    {
        return mSerial;
    }

    FlowInstance& GetFlow(size_t flowIndex)
    {
        return *(mFlows.at(flowIndex).get());
    }

    virtual uint16_t GetAtkLoopIteration(size_t flowIndex)
    {
        return 0;
    }

    virtual bool IsAttack() 
    {
       return false; 
    }

    uint32_t GetWaitTimeout(size_t flowIndex)
    {
        return mConfig.flows.at(flowIndex).minPktDelay;
    }

    uint32_t GetInactivityTimeout(size_t flowIndex)
    {
        return mConfig.flows.at(flowIndex).socketTimeout;
    }

    uint32_t GetDurationMsec(void) { return mStopMsec - mStartMsec; }
 
    const PlaylistConfig::Flow::VarMap_t& GetVarValues(size_t flowIndex)
    {
        return mConfig.flows.at(flowIndex).varMap;
    }

    const ModifierBlock& GetModifiers() const { return mModifiers; }

    ModifierBlock& GetModifiers() { return mModifiers; }

    UNIT_TEST_VIRTUAL uint32_t GetCurrTimeMsec(void) {return mEngine.GetTimerMgr().GetTimeOfDayMsec();}

    virtual void Start() = 0;

 
    void PreStop() { mForceStop = true;}
	
    bool GetPreStop(){ return mForceStop;}

    void Stop();

    PlaylistState GetState() const { return mState; }

    void SetStoppedDelegate(const stoppedDelegate_t& delegate)
    {
        mStoppedDelegate = delegate;
    } 

    void ClearStoppedDelegate()
    {
        mStoppedDelegate.clear();
    } 

    void SetStatDelegate(const statDelegate_t& delegate)
    {
        mStatDelegate = delegate;
    } 

    // callback from flows
    virtual void FlowDone(size_t flowIndex, bool successful) = 0;

    // make a new playlist identical to this one at its creation (not a copy of current state)
    virtual PlaylistInstance * Clone() const = 0;

    AbstSocketManager& GetSocketMgr() { return mSocketMgr; }

    typedef std::tr1::shared_ptr<FlowLoopHandler> flowLpHdlrSharedPtr_t;

    FlowLoopHandler & GetLoopHandler(uint16_t loopId);
    void CloseConnsInLoop (flowLpHdlrSharedPtr_t loopHdlr);
    // called by the stream loop handlers
    UNIT_TEST_VIRTUAL void NotifyLoopDone (uint16_t loopId); // all iterations completed successfully
    UNIT_TEST_VIRTUAL void NotifyLoopFailed (uint16_t loopId); // at least one iteration of at least one flow failed

  protected:
    // can't be deleted by delete
    virtual ~PlaylistInstance() {}

    //TODO: It might be changed to get from playlist config and override the one in flow
    FlowConfig::eCloseType GetFlowCloseType(size_t flowIndex)
    {
        return  mFlows.at(flowIndex)->GetConfig().closeType;
    }

    void PostClientListen(size_t flowIndex) {}
    void PostServerListen(size_t flowIndex);
    void PostConnect(size_t flowIndex);
    void WaitToConnect(size_t flowIndex);

    virtual bool OnClose(size_t flowIndex, AbstSocketManager::CloseType closeType) = 0;
    bool OnConnect(size_t flowIndex, int fd);
    void WaitToStartData(size_t flowIndex);
    virtual void StartData(size_t flowIndex) = 0;

    typedef std::tr1::shared_ptr<FlowInstance> flowInstSharedPtr_t;
    typedef std::vector<flowInstSharedPtr_t> flowInstList_t;
    typedef std::vector<handle_t> timerList_t;

    PlaylistEngine& mEngine;
    AbstSocketManager& mSocketMgr;
    const PlaylistConfig& mConfig;
    flowInstList_t mFlows;
    ModifierBlock mModifiers;
    bool mClient;
    bool mForceStop;
    PlaylistState mState;
    uint32_t mStartMsec;
    uint32_t mStopMsec;
    uint32_t mSerial;
    uint32_t mRefCount;

    timerList_t mTimers;
    stoppedDelegate_t mStoppedDelegate;
    statDelegate_t mStatDelegate;

    uint16_t    mNumOfRunningFlows;

    std::map<uint16_t /* loop id  = begin flow index */, flowLpHdlrSharedPtr_t>        mFlowLoopMap;

  friend class StreamLoopHandler;
  friend class AttackLoopHandler;

#ifdef UNIT_TEST
    friend class TestPlaylistInstance;
    friend class TestFlowLoopHandler;
#endif
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
