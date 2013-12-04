/// @file
/// @brief Flow instance implementation
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <boost/bind.hpp>
#include <sstream>

#include "EngineHooks.h"
#include "FlowEngine.h"
#include "FlowInstance.h"
#include "PacketFactory.h"
#include "PlaylistInstance.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

FlowInstance::FlowInstance(FlowEngine& engine, handle_t flowHandle, PlaylistInstance *pi, size_t flowIdx, bool client, flowLpHdlrSharedPtr_t loopHandler) :
    mEngine(engine),
    mFlowHandle(flowHandle),
    mFlowIndex(flowIdx),
    mConfig(engine.GetFlow(flowHandle)),
    mPlaylistInstance(pi),
    mClient(client),
    mFd(0),
    mPacketIdx(0),
    mTimer(engine.GetTimerMgr().InvalidTimerHandle()),
    mState(WAIT_TO_START),
    mFlowLoopHandler(loopHandler),
    mInactivityTimer(engine.GetTimerMgr().InvalidTimerHandle()),
    mInactivityTimerLock(new lock_t())
{
    if (!mConfig)
    {
        std::ostringstream oss;
        oss << "[FlowInstance::FlowInstance] Could not find flow with handle " << flowHandle;
        const std::string err(oss.str());
        EngineHooks::LogErr(0, err);
        throw DPG_EInternal(err);
    }
}

///////////////////////////////////////////////////////////////////////////////
#ifdef UNIT_TEST
FlowInstance::FlowInstance(const FlowInstance& other) :
    mEngine(other.mEngine),
    mFlowHandle(other.mFlowHandle),
    mFlowIndex(other.mFlowIndex),
    mConfig(other.mConfig),
    mPlaylistInstance(other.mPlaylistInstance),
    mClient(other.mClient),
    mFd(0),
    mPacketIdx(0),
    mTimer(other.mTimer),
    mState(WAIT_TO_START),
    mFlowLoopHandler(other.mFlowLoopHandler),
    mInactivityTimer(other.mInactivityTimer),
    mInactivityTimerLock(new lock_t())
{
}
#endif
///////////////////////////////////////////////////////////////////////////////

FlowInstance::~FlowInstance()
{
    CancelTxRxTimer();
    CancelInactivityTimer();
    delete mInactivityTimerLock;
}

///////////////////////////////////////////////////////////////////////////////


void FlowInstance::SetupInactivityTimer()
{
    ENGINE_GUARD(lock_t, guard, *mInactivityTimerLock);
    CancelInactivityTimer();
    bool preStop = mPlaylistInstance->GetPreStop();
    if( !preStop)
    {
      AbstTimerManager::timerDelegate_t delegate(boost::bind(&FlowInstance::FlowTimeout, this));

      handle_t timerHandle = mEngine.GetTimerMgr().CreateTimer(delegate, DPG_FLOW_INACTIVITY_TIMEOUT);

      mInactivityTimer = timerHandle;
    }
    else
    {
    	mInactivityTimer = mEngine.GetTimerMgr().InvalidTimerHandle();
    }
}

///////////////////////////////////////////////////////////////////////////////

void FlowInstance::SetupInactivityTimer(uint32_t timeout)
{
    ENGINE_GUARD(lock_t, guard, *mInactivityTimerLock);
    CancelInactivityTimer();
    bool preStop = mPlaylistInstance->GetPreStop();
    if( !preStop)
    {
      AbstTimerManager::timerDelegate_t delegate(boost::bind(&FlowInstance::FlowTimeout, this));

      handle_t timerHandle;
      timerHandle = mEngine.GetTimerMgr().CreateTimer(delegate, timeout);

      mInactivityTimer = timerHandle;
    }
    else
    {
    	mInactivityTimer = mEngine.GetTimerMgr().InvalidTimerHandle();
    }
}
///////////////////////////////////////////////////////////////////////////////

void FlowInstance::CancelInactivityTimer()
{
    ENGINE_GUARD(lock_t, guard, *mInactivityTimerLock);
    if (mInactivityTimer != mEngine.GetTimerMgr().InvalidTimerHandle())
    {
       mEngine.GetTimerMgr().CancelTimer(mInactivityTimer);
       mInactivityTimer = mEngine.GetTimerMgr().InvalidTimerHandle();
    }
}

///////////////////////////////////////////////////////////////////////////////

void FlowInstance::CancelTxRxTimer()
{
    if (mTimer != mEngine.GetTimerMgr().InvalidTimerHandle())
    {
       mEngine.GetTimerMgr().CancelTimer(mTimer);
       mTimer = mEngine.GetTimerMgr().InvalidTimerHandle();
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Called from the parent playlist instance
//
void FlowInstance::Stop()
{
// NOTE: For now, we do not check the close type, because it's currently not supported

    CancelTxRxTimer();

    CancelInactivityTimer();

    if (mState == WAIT_TO_START || mState == STARTED)
    {
        mState = ABORTED;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// All packets are processed successfully
//
void FlowInstance::Complete()
{
    mState = DONE;

    CancelTxRxTimer();

    CancelInactivityTimer();
    // notify its PlaylistInstance that we're done
    mPlaylistInstance->FlowDone(mFlowIndex, true);
}

///////////////////////////////////////////////////////////////////////////////
//
// Data transmission failed
//
void FlowInstance::Failed()
{
    mState = ABORTED;

    CancelTxRxTimer();

    CancelInactivityTimer();
    // notify its PlaylistInstance that we're aborting
    mPlaylistInstance->FlowDone(mFlowIndex, false);
}

///////////////////////////////////////////////////////////////////////////////

void FlowInstance::FlowTimeout()
{
    Failed();
}

///////////////////////////////////////////////////////////////////////////////

void FlowInstance::SendPacket(const FlowConfig::Packet* packet, AbstSocketManager::sendDelegate_t& delegate)
{
    FlowModifierBlock flowModifiers(mPlaylistInstance->GetModifiers(), mFlowIndex);
    PacketFactory factory(mPlaylistInstance->GetVarValues(mFlowIndex), *packet, mConfig->varList, flowModifiers);

    mPlaylistInstance->GetSocketMgr().AsyncSend(mFd, factory.Buffer(), factory.BufferLength(), delegate);
}

///////////////////////////////////////////////////////////////////////////////

size_t FlowInstance::GetPacketLength(const FlowConfig::Packet* packet)
{
    FlowModifierBlock flowModifiers(mPlaylistInstance->GetModifiers(), mFlowIndex);
    return PacketFactory::CalculateSize(mPlaylistInstance->GetVarValues(mFlowIndex), *packet, mConfig->varList, flowModifiers);
}

///////////////////////////////////////////////////////////////////////////////

