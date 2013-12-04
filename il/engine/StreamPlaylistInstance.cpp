/// @file
/// @brief Stream playlist instance implementation
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
#include <map>

#include "FlowInstance.h"
#include "FlowEngine.h"

#include "AbstSocketManager.h"
#include "AbstTimerManager.h"
#include "PlaylistConfig.h"
#include "PlaylistEngine.h"
#include "PlaylistInstance.h"
#include "ReferenceHolder.h"
#include "StreamPlaylistInstance.h"
#include "StreamInstance.h"
#include "StreamLoopHandler.h"


USING_L4L7_ENGINE_NS;

//////////////////////////////////////////////////////////////////////////////////

StreamPlaylistInstance::StreamPlaylistInstance(PlaylistEngine& pEngine, AbstSocketManager& socketMgr, const PlaylistConfig& config, handle_t handle, const ModifierBlock& modifiers, bool client) :
    PlaylistInstance(pEngine, socketMgr, config, modifiers, client),
    mHandle(handle)
{
    FlowEngine& fEngine(pEngine.GetFlowEngine());
    size_t idx = 0;

    BOOST_FOREACH(const PlaylistConfig::Flow& flow, mConfig.flows)
    {

        // update the packet delay map in the PlaylistEngine
        PktDelayMap::delays_t * delays;
        PktDelayMap::iterator delays_it = pEngine.GetPktDelayMap().find(handle, flow.flowHandle);
        // if not there yet, add one.
        if ( delays_it==pEngine.GetPktDelayMap().end())
        {
            uint32_t minDelay = config.flows[idx].minPktDelay;
            uint32_t maxDelay = config.flows[idx].maxPktDelay;
            uint32_t actualDelay = 0;

            delays_it = pEngine.GetPktDelayMap().insert(handle, flow.flowHandle);
            delays = &delays_it->second;

            const FlowConfig::PktList_t& pktList(fEngine.GetFlow(flow.flowHandle)->pktList);
            delays->reserve(pktList.size());

            BOOST_FOREACH (const FlowConfig::Packet &pkt, pktList)
            {
                actualDelay = (pkt.pktDelay < minDelay? minDelay: pkt.pktDelay);
                actualDelay = (actualDelay > maxDelay? maxDelay: actualDelay);
                delays->push_back(actualDelay);
            }
        }
        // if exists, just use.
        else
        {
            delays = &delays_it->second;
        }

        // If this flow is in a stream loop, first create the StreamLoopHandler if it's not there
        // then pass the pointer to it to the stream instance
        flowLpHdlrSharedPtr_t loopHandler = flowLpHdlrSharedPtr_t();
        if (flow.loopInfo)
        {
             std::map<uint16_t, flowLpHdlrSharedPtr_t>::iterator loop_iter = mFlowLoopMap.find(flow.loopInfo->begIdx);
             if (loop_iter==mFlowLoopMap.end())
             {
                // find the end idx
                uint16_t endIdx = 0;
                for (std::map<uint16_t, PlaylistConfig::LoopInfo>::const_iterator conf_iter = mConfig.loopMap.begin(); conf_iter!=mConfig.loopMap.end(); conf_iter++)
                {
                    if (&(conf_iter->second)==flow.loopInfo)
                    {
                        endIdx = conf_iter->first;
                        break;
                    }
                }
                // pass everything to a new loop handler
                loopHandler = flowLpHdlrSharedPtr_t(new StreamLoopHandler(this, *(flow.loopInfo), endIdx));
                mFlowLoopMap.insert(std::make_pair(flow.loopInfo->begIdx, loopHandler));
             }
             else
             {
                loopHandler = loop_iter->second;
             }
        }

        mNumOfRunningFlows++;

        mFlows.push_back(flowInstSharedPtr_t(new StreamInstance(fEngine, flow.flowHandle, this, idx++, delays, client ^ flow.reversed, loopHandler)));
    }
}

///////////////////////////////////////////////////////////////////////////////

void StreamPlaylistInstance::Start()
{
    mState = STARTED;
    mStartMsec = GetCurrTimeMsec();

    size_t flowIdx = 0;

    if (mClient)
    {
        // client side -- open any listening sockets, connect, or prepare to connect
        BOOST_FOREACH(const PlaylistConfig::Flow& flow, mConfig.flows)
        {
            if (!flow.reversed)
            {
                if (flow.startTime == 0) 
                {
                    // no delay
                    PostConnect(flowIdx++);
                }
                else
                {
                    // delay, then connect
                    WaitToConnect(flowIdx++);
                }
            }
            else
            {
                // server flow -- listen
                PostClientListen(flowIdx++);
            }
        }
    }
    else
    {
        // server side -- open any listening sockets, don't connect or prepare to connect
        BOOST_FOREACH(const PlaylistConfig::Flow& flow, mConfig.flows)
        {
            if (!flow.reversed)
            {
                // server flow -- listen
                PostServerListen(flowIdx++);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool StreamPlaylistInstance::OnClose(size_t flowIndex, AbstSocketManager::CloseType closeType)
{
    FlowInstance &flowInst = GetFlow(flowIndex);
    bool CloseNotRequested = flowInst.GetFd();
    int fd = flowInst.GetFd();
    flowInst.SetFd(0);
    FlowInstance::FlowState state = flowInst.GetState();

    if (flowInst.IsInFlowLoop())
    {
        FlowLoopHandler & loopHandler = *(flowInst.GetFlowLoopHandler());
        // if loop is not done, let loop handler to handle; otherwise, do nothing
        if (loopHandler.GetState() <= FlowLoopHandler::STARTED)
        {
            if (state == FlowInstance::DONE)
	    {
                loopHandler.FlowInLoopDone(flowIndex);
            }
            else
            {
                loopHandler.FlowInLoopFailed(flowIndex);
            }
        }
        else if (loopHandler.GetState() == FlowLoopHandler::CLOSING)
	{
	    if (CloseNotRequested)
	    {
	        mNumOfRunningFlows--;
	    }
	}
    }
    else
    {
        if (state == FlowInstance::WAIT_TO_START && closeType == AbstSocketManager::CON_ERR )
        {
            // connection not established yet
            flowInst.SetState(FlowInstance::FAIL_TO_CONNECT);
            mNumOfRunningFlows--;
        }
        else if (state == FlowInstance::WAIT_TO_START)
        {
            // connection not established yet
            flowInst.SetState(FlowInstance::ABORTED);
            mNumOfRunningFlows--;
        }
        if (state == FlowInstance::STARTED || (state == FlowInstance::DONE && CloseNotRequested))
        {
            flowInst.Stop();
            mNumOfRunningFlows--;
        }
    }

    // remove fd from socket manager map
    // must do this before calling stop because, stop could call back
    // to stop delegate which would destroy this object.
    if (fd)
    {
      mSocketMgr.Clean(fd);
    }

    if (!mNumOfRunningFlows)
    {
        Stop();
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void StreamPlaylistInstance::FlowDone(size_t flowIndex, bool successful)
{
    // possible for Close below to result in this being destroyed before returning
    ReferenceHolder<PlaylistInstance> holder(*this);

    PlaylistInstance::flowInstSharedPtr_t flow = mFlows.at(flowIndex);
    // If this flow is in a loop, then let the stream loop handler to handle it.
    if (flow->IsInFlowLoop())
    {
        FlowLoopHandler & loopHandler = *(flow->GetFlowLoopHandler());
        if (successful)
        {
            loopHandler.FlowInLoopDone(flowIndex);
        }
        else
        {
            loopHandler.FlowInLoopFailed(flowIndex);
        }
    }
    else // not in a stream loop
    {
        if (int fd = flow->GetFd())
        {
            flow->SetState(successful? FlowInstance::DONE : FlowInstance::ABORTED);

            // if flow failed, close the socket by reset
            if (!successful)
            {
                mNumOfRunningFlows--;
                flow->SetFd(0);
                mSocketMgr.Close(fd, true);
            }
            // if flow succeeded, check if we're the one to close the socket
            else
            {
                if ((GetFlowCloseType(flowIndex)==FlowConfig::C_FIN && mClient)
                 || (GetFlowCloseType(flowIndex)==FlowConfig::S_FIN && !mClient)
                 || (flow->GetConfig().layer == FlowConfig::UDP))
                {
 
                   if((flow->GetConfig().layer == FlowConfig::UDP) && !mClient)
                       flow->Stop();
                    mNumOfRunningFlows--;
                    flow->SetFd(0);
                    mSocketMgr.Close(fd, false);
                }
            }
        }
    }

    if (!mNumOfRunningFlows)
    {
        Stop();
    }
}

///////////////////////////////////////////////////////////////////////////////

StreamPlaylistInstance * StreamPlaylistInstance::Clone() const
{
    return new StreamPlaylistInstance(mEngine, mSocketMgr, mConfig, mHandle, GetModifiers(), mClient);
}

///////////////////////////////////////////////////////////////////////////////

void StreamPlaylistInstance::StartData(size_t flowIndex)
{
    // clear timer handle so we don't cancel it on stop
    mTimers[flowIndex] = mEngine.GetTimerMgr().InvalidTimerHandle();

    PlaylistInstance::flowInstSharedPtr_t flowInst = mFlows.at(flowIndex);
    if (flowInst->IsInFlowLoop())
    {
        flowInst->GetFlowLoopHandler()->StartLoop(flowIndex);
    }
    else
    {
        flowInst->Start();
    }
}

///////////////////////////////////////////////////////////////////////////////
