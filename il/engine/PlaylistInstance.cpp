/// @file
/// @brief Playlist instance implementation
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
#include <boost/foreach.hpp>
#include <sstream>

#include "EngineHooks.h"
#include "FlowInstance.h"
#include "AbstSocketManager.h"
#include "AbstTimerManager.h"
#include "PlaylistConfig.h"
#include "FlowConfig.h"
#include "PlaylistEngine.h"
#include "ReferenceHolder.h"
#include "PlaylistInstance.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

PlaylistInstance::PlaylistInstance(PlaylistEngine& pEngine, AbstSocketManager& socketMgr, const PlaylistConfig& config, const ModifierBlock& modifiers, bool client) :
    mEngine(pEngine),
    mSocketMgr(socketMgr),
    mConfig(config),
    mModifiers(modifiers),
    mClient(client),
    mForceStop(false),
    mState(WAIT_TO_START),
    mStartMsec(0),
    mStopMsec(0),
    mRefCount(1),
    mTimers(config.flows.size(), pEngine.GetTimerMgr().InvalidTimerHandle()),
    mNumOfRunningFlows(0)
            {
            }
///////////////////////////////////////////////////////////////////////////////
//
// Called by the engine when there's no running flows.
// Or, called from the DPG application when "Stop" button is clicked
//
void PlaylistInstance::Stop()
{
    // NOTE: we may need to put a lock here
    if (mState >= STOPPING)
        return;

    mState = STOPPING;
    
    // Cancel all timers
    AbstTimerManager& timerMgr(mEngine.GetTimerMgr());
    const handle_t invalidTimer = timerMgr.InvalidTimerHandle();
    BOOST_FOREACH(handle_t& hTimer, mTimers)
    {
        if (hTimer != invalidTimer)
        {
            timerMgr.CancelTimer(hTimer);
            hTimer = invalidTimer;
        }
    }

    bool aborted = false; // this aborted means we were running when stopped
    bool successful = true; // this means we failed any of the flows
    uint32_t numFailConnect = 0; // number of flows that failed a connection attempt in the stack

    // Close all open sockets
    // Update state for each flow
    BOOST_FOREACH(PlaylistInstance::flowInstSharedPtr_t flow, mFlows)
    {
        FlowInstance::FlowState state = flow->GetState();
        if (state == FlowInstance::STARTED || state == FlowInstance::WAIT_TO_START)
        {
            flow->Stop();
            aborted = true;
        }
        else if (mForceStop)
        {
            // The client notified us that a stop would be coming, then
            // turned off the connector/acceptor, which caused us to stop
            aborted = true;
        }

        if (int fd = flow->GetFd())
        {
            flow->SetFd(0);
            mSocketMgr.Close(fd, state!=FlowInstance::DONE);
        }
        if (flow->GetState() != FlowInstance::DONE && flow->GetState() != FlowInstance::FAIL_TO_CONNECT)
        {
            successful = false;
        }
        else if (flow->GetState() == FlowInstance::FAIL_TO_CONNECT)
        {
          ++numFailConnect;
        }
    }

    // Update state and statistics for this playlist instance
    // if number of failed to connect is equal to number of flows, then set playlist state
    // to FAIL_TO_CONNECT, else just use the state of the other flows.
    if (numFailConnect == mFlows.size())
    {
      mState = FAIL_TO_CONNECT;
      std::ostringstream oss;
      oss << "[" << __PRETTY_FUNCTION__ << ":" << __LINE__ << "] All flows in Playlist(" << this << ") failed to connect.";
      const std::string err(oss.str());
      EngineHooks::LogErr(0, err);
    }
    else
    {
        if (aborted) mState = ABORTED;
        else if (!successful) mState = FAILED;
        else mState = SUCCESSFUL;
    }
    mStopMsec = GetCurrTimeMsec();
    if (mStoppedDelegate)
    {
        mStoppedDelegate(this);
    }
}

///////////////////////////////////////////////////////////////////////////////

void PlaylistInstance::PostConnect(size_t flowIndex)
{
    // prevent the instance from being destroyed during this method
    ReferenceHolder<PlaylistInstance> holder(*this);

    mTimers[flowIndex] = mEngine.GetTimerMgr().InvalidTimerHandle();

    AbstSocketManager::connectDelegate_t conDelegate(boost::bind(&PlaylistInstance::OnConnect, this, flowIndex, _1));
    AbstSocketManager::closeDelegate_t cloDelegate(boost::bind(&PlaylistInstance::OnClose, this, flowIndex, _1));
    bool isTcp = (GetFlow(flowIndex).GetConfig().layer == FlowConfig::TCP);

    int pendingFd = mSocketMgr.AsyncConnect(this, mConfig.flows.at(flowIndex).serverPort, isTcp, flowIndex, conDelegate, cloDelegate);
    if (pendingFd)
        mFlows.at(flowIndex)->SetFd(pendingFd);
}

///////////////////////////////////////////////////////////////////////////////

void PlaylistInstance::PostServerListen(size_t flowIndex)
{
    AbstSocketManager::connectDelegate_t conDelegate(boost::bind(&PlaylistInstance::OnConnect, this, flowIndex, _1));
    AbstSocketManager::closeDelegate_t cloDelegate(boost::bind(&PlaylistInstance::OnClose, this, flowIndex, _1));
    bool isTcp = (GetFlow(flowIndex).GetConfig().layer == FlowConfig::TCP);

    mSocketMgr.Listen(this, mConfig.flows.at(flowIndex).serverPort, isTcp, flowIndex, conDelegate, cloDelegate);
}

///////////////////////////////////////////////////////////////////////////////

void PlaylistInstance::WaitToConnect(size_t flowIndex)
{
    if(mForceStop)
    {
       mTimers[flowIndex] = mEngine.GetTimerMgr().InvalidTimerHandle();
       return;
    }
    AbstTimerManager::timerDelegate_t delegate(boost::bind(&PlaylistInstance::PostConnect, this, flowIndex));

    handle_t timerHandle = mEngine.GetTimerMgr().CreateTimer(delegate, mConfig.flows.at(flowIndex).startTime);

    mTimers[flowIndex] = timerHandle;
}

///////////////////////////////////////////////////////////////////////////////

bool PlaylistInstance::OnConnect(size_t flowIndex, int fd)
{
    // prevent the instance from being destroyed during this method
    ReferenceHolder<PlaylistInstance> holder(*this);

    PlaylistInstance::flowInstSharedPtr_t& flow = mFlows.at(flowIndex);
    int oldFd = flow->GetFd();
    if (oldFd && oldFd != fd)
    {
        // We are receiving a connection when we already have a different one.
        // This is possible with attack loops on the server side - close the
        // current connection. 
        flow->SetFd(0);
        mSocketMgr.Close(oldFd, flow->GetState() != FlowInstance::DONE);
    }

    flow->SetFd(fd);

    if (mConfig.type == PlaylistConfig::STREAM_ONLY
            && GetFlow(flowIndex).GetConfig().layer == FlowConfig::TCP
            && GetFlow(flowIndex).GetConfig().pktList.size()
            && mConfig.flows.at(flowIndex).dataDelay)
    {
        // If the first packet is rx, ignore the data delay
        bool clientTx0 = GetFlow(flowIndex).GetConfig().pktList[0].clientTx;
        if ((!mClient && clientTx0) || (mClient && !clientTx0))
        {
            StartData(flowIndex);
        }
        else
        {
            WaitToStartData(flowIndex);
        }
    }
    else
    {
        StartData(flowIndex);
    }

    // true if not closed already
    return flow->GetFd() != 0;
}

///////////////////////////////////////////////////////////////////////////////

void PlaylistInstance::WaitToStartData(size_t flowIndex)
{
    if(mForceStop)
    {
       mTimers[flowIndex] = mEngine.GetTimerMgr().InvalidTimerHandle();
       return;
    }
    AbstTimerManager::timerDelegate_t delegate(boost::bind(&PlaylistInstance::StartData, this, flowIndex));

    handle_t timerHandle = mEngine.GetTimerMgr().CreateTimer(delegate, mConfig.flows.at(flowIndex).dataDelay);
    // playlist instance only maintains one timer handle for each flow instance.
    // Yet it's fine, because start timer and data delay timer do not exist at the same time.
    mTimers[flowIndex] = timerHandle;
}

///////////////////////////////////////////////////////////////////////////////

FlowLoopHandler & PlaylistInstance::GetLoopHandler(uint16_t loopId)
{
    std::map<uint16_t, flowLpHdlrSharedPtr_t>::iterator iter = mFlowLoopMap.find(loopId);
    if (iter!=mFlowLoopMap.end())
    {
        return *(iter->second);
    }
    else
    {
        std::ostringstream oss;
        oss << "[PlaylistInstance::GetLoopHandler] Could not find flow loop with ID " << loopId;
        const std::string err(oss.str());
        EngineHooks::LogErr(0, err);
        throw DPG_EInternal(err);
    }
}

///////////////////////////////////////////////////////////////////////////////

void PlaylistInstance::CloseConnsInLoop (flowLpHdlrSharedPtr_t loopHdlr)
{
    loopHdlr->SetClosing();
    if (mConfig.type == PlaylistConfig::STREAM_ONLY)
    {
        for (uint16_t idx = loopHdlr->GetBeginIdx(); idx <= loopHdlr->GetEndIdx(); idx++)
        {
            flowInstSharedPtr_t flowInst = mFlows.at(idx);
            bool successful = flowInst->GetState()==FlowInstance::DONE;
            if (int fd = flowInst->GetFd())
            {
                // if flow failed, close the socket by reset
                if (!successful)
                {
                    mNumOfRunningFlows--;
                    flowInst->SetFd(0);
                    mSocketMgr.Close(fd, true);
                }
                // if flow succeeded, check if we're the one to close the socket
                else
                {
                    if ((GetFlowCloseType(idx)==FlowConfig::C_FIN && mClient)
                     || (GetFlowCloseType(idx)==FlowConfig::S_FIN && !mClient)
                     || (flowInst->GetConfig().layer == FlowConfig::UDP))
                    {
               
                        if((flowInst->GetConfig().layer == FlowConfig::UDP) && !mClient) 
                            flowInst->Stop();
                        mNumOfRunningFlows--;
                        flowInst->SetFd(0);
                        mSocketMgr.Close(fd, false);
                    }
                }
            }
            else  // assumed closed
            {
                mNumOfRunningFlows--;
            }
        }
    }
    else
    {
        for (uint16_t idx = loopHdlr->GetBeginIdx(); idx <= loopHdlr->GetEndIdx(); idx++)
        {
            flowInstSharedPtr_t flowInst = mFlows[idx];
            if (int fd = flowInst->GetFd())
            {
                flowInst->SetFd(0);
                mSocketMgr.Close(fd, true);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void PlaylistInstance::NotifyLoopDone (uint16_t loopId)
{
    flowLpHdlrSharedPtr_t loopHdlr = mFlowLoopMap[loopId];
    for (uint16_t idx = loopHdlr->GetBeginIdx(); idx <= loopHdlr->GetEndIdx(); idx++)
    {
        mFlows.at(idx)->SetState(FlowInstance::DONE);
    }
    CloseConnsInLoop(loopHdlr);
}

///////////////////////////////////////////////////////////////////////////////

void PlaylistInstance::NotifyLoopFailed (uint16_t loopId)
{
    flowLpHdlrSharedPtr_t loopHdlr = mFlowLoopMap[loopId];
    for (uint16_t idx = loopHdlr->GetBeginIdx(); idx <= loopHdlr->GetEndIdx(); idx++)
    {
        mFlows.at(idx)->SetState(FlowInstance::ABORTED);
    }
    CloseConnsInLoop(loopHdlr);
}

///////////////////////////////////////////////////////////////////////////////
