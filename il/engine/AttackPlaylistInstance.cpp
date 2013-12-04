/// @file
/// @brief Attack playlist instance implementation
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

#include "FlowInstance.h"

#include "AbstSocketManager.h"
#include "AbstTimerManager.h"
#include "EngineHooks.h"
#include "PlaylistConfig.h"
#include "PlaylistEngine.h"
#include "PlaylistInstance.h"
#include "AttackPlaylistInstance.h"
#include "AttackInstance.h"
#include "AttackLoopHandler.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

AttackPlaylistInstance::AttackPlaylistInstance(PlaylistEngine& pEngine, AbstSocketManager& socketMgr, const PlaylistConfig& config, const ModifierBlock& modifiers, bool client) :
                PlaylistInstance(pEngine, socketMgr, config, modifiers, client),
                mLoopIterOverride(0)
{
    FlowEngine& fEngine(pEngine.GetFlowEngine());
    size_t idx = 0;
    BOOST_FOREACH(const PlaylistConfig::Flow& flow, mConfig.flows)
    {
        // If this attack is in a flow loop, first create the AttackLoopHandler if it's not there
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
                loopHandler = flowLpHdlrSharedPtr_t(new AttackLoopHandler(this, *(flow.loopInfo), endIdx));
                mFlowLoopMap.insert(std::make_pair(flow.loopInfo->begIdx, loopHandler));
             }
             else
             {
                loopHandler = loop_iter->second;
             }
        }

        mFlows.push_back(flowInstSharedPtr_t(new AttackInstance(fEngine, flow.flowHandle, this, idx++, client ^ flow.reversed, loopHandler)));
    }
}


///////////////////////////////////////////////////////////////////////////////

void AttackPlaylistInstance::Start()
{
    mState = STARTED;
    mStartMsec = GetCurrTimeMsec();

    if (!mClient)
    {
        // server side -- open any listening sockets, don't connect or prepare to connect
        size_t flowIdx = 0;
        BOOST_FOREACH(const PlaylistConfig::Flow& flow, mConfig.flows)
        {
            if (!flow.reversed)
            {
                // server flow -- listen for all loop iterations
                uint16_t loopCount = flow.loopInfo ? flow.loopInfo->count : 1;
                while (loopCount--)
                {
                    mLoopIterOverride = loopCount;
                    PostServerListen(flowIdx);
                }
            }

            ++flowIdx;
        }
    }

    mCurrAttackIdx = 0;
    if (mFlows.at(mCurrAttackIdx)->IsInFlowLoop())
    {
        PlaylistInstance::flowLpHdlrSharedPtr_t loopHdlr = mFlows.at(mCurrAttackIdx)->GetFlowLoopHandler();
        if (loopHdlr->GetState()==FlowLoopHandler::STARTED)
        {
            mCurrAttackIdx = loopHdlr->GetCurrAttackIdx();
            ProcessAttack(mCurrAttackIdx);
        }
        else
        {
            mCurrAttackIdx = loopHdlr->GetBeginIdx();
            loopHdlr->StartLoop(mCurrAttackIdx); // to start first attack in the loop
        }
    }
    else
    {
        ProcessAttack(mCurrAttackIdx);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AttackPlaylistInstance::OnClose(size_t flowIndex, AbstSocketManager::CloseType closeType)
{
    FlowInstance &flowInst = GetFlow(flowIndex);
    int fd = flowInst.GetFd();
    bool client = flowInst.IsClient();

    // Only stop attack playlist instance when mCurrAttackIdx
    // is greater than flow size and last flow loop is done
    // not sure if this works for multiple flow loops in an attack.

    // default to successful if no flow loops exists
    bool loopDone = true;
    for (std::map<uint16_t, flowLpHdlrSharedPtr_t>::iterator loopHdlr = mFlowLoopMap.begin(); loopHdlr != mFlowLoopMap.end(); ++loopHdlr)
    {
        FlowLoopHandler::FlowLoopState state = loopHdlr->second->GetState();
        if (state == FlowLoopHandler::WAIT_TO_START || state == FlowLoopHandler::STARTED)
        {
            loopDone = false;
            break;
        }
    }
    if (!loopDone)
    {
        if (fd && (!client))
           mSocketMgr.Clean(fd);
        return false;
    }

    if (fd == 0)
    {
        // This close was intended, just return
        return true;
    }
    else
    {
        if (!client)
           mSocketMgr.Clean(fd);
    }

    flowInst.SetFd(0);
    FlowInstance::FlowState state = flowInst.GetState();

    if (flowInst.IsInFlowLoop())
    {
        FlowLoopHandler & loopHandler = *(flowInst.GetFlowLoopHandler());
        loopHandler.FlowInLoopFailed(flowIndex);
    }
    else
    {
        if (state == FlowInstance::STARTED || state == FlowInstance::WAIT_TO_START)
            flowInst.Stop();
        
        // start the next attack
        DoNextAttack(false);
    }
    return true;
}

//////////////////////////////////////////////////////////////////////
//
/// @brief Close the socket for the current attack and move on to the next
//
void AttackPlaylistInstance::DoNextAttack(bool currSuccessful)
{
    CloseSocket(mCurrAttackIdx, currSuccessful);

    if (GetFlow(mCurrAttackIdx).IsInFlowLoop())
    {
        PlaylistInstance::flowLpHdlrSharedPtr_t loopHdlr = GetFlow(mCurrAttackIdx).GetFlowLoopHandler();
        mCurrAttackIdx = loopHdlr->GetCurrAttackIdx();
        // loop should be already started
        ProcessAttack(mCurrAttackIdx);
    }
    else if ((mCurrAttackIdx + 1)< mFlows.size() && GetFlow(mCurrAttackIdx + 1).IsInFlowLoop())
    {   // next attack is the first one in a loop
        mCurrAttackIdx++;
        PlaylistInstance::flowLpHdlrSharedPtr_t loopHdlr = GetFlow(mCurrAttackIdx).GetFlowLoopHandler();
        loopHdlr->StartLoop(mCurrAttackIdx);
    }
    else
    {
        mCurrAttackIdx++;
        ProcessAttack(mCurrAttackIdx);
    }
}

//////////////////////////////////////////////////////////////////////

void AttackPlaylistInstance::CloseSocket(uint16_t flowIndex, bool successful)
{
    flowInstSharedPtr_t flowInst = mFlows.at(flowIndex);
    int fd = flowInst->GetFd();
    if (fd)
    {
        flowInst->SetFd(0);
        mSocketMgr.Close(fd, !successful);
    }
}

//////////////////////////////////////////////////////////////////////

void AttackPlaylistInstance::ProcessAttack(size_t flowIndex)
{
    if (mCurrAttackIdx >= mFlows.size())
    {
        Stop();
    }
    else
    {
        const PlaylistConfig::Flow& flowConf = mConfig.flows[flowIndex];
        if (mClient)
        {
            // client side -- open the next listening socket
            if (!flowConf.reversed)
            {
                // no delay for starting attacks
                PostConnect(flowIndex);
            }
            else
            {
                // server flow -- listen
                PostClientListen(flowIndex);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AttackPlaylistInstance::FlowDone(size_t flowIndex, bool successful)
{
    flowInstSharedPtr_t flowInst = mFlows.at(flowIndex);
    if (flowInst->IsInFlowLoop())
    {
        FlowLoopHandler & loopHandler = *(flowInst->GetFlowLoopHandler());
        if (successful)
        {
            loopHandler.FlowInLoopDone(flowIndex);
        }
        else
        {
            loopHandler.FlowInLoopFailed(flowIndex);
        }
    }
    else
    {
        DoNextAttack(successful);
    }
}

///////////////////////////////////////////////////////////////////////////////

AttackPlaylistInstance * AttackPlaylistInstance::Clone() const
{
    return new AttackPlaylistInstance(mEngine, mSocketMgr, mConfig, GetModifiers(), mClient);
}


///////////////////////////////////////////////////////////////////////////////

void AttackPlaylistInstance::StartData(size_t flowIndex)
{
    // clear timer handle so we don't cancel it on stop
    mTimers[flowIndex] = mEngine.GetTimerMgr().InvalidTimerHandle();

    mFlows.at(flowIndex)->Start();

    if (mClient && mStatDelegate)
    {
        mStatDelegate();
    }
}

///////////////////////////////////////////////////////////////////////////////

uint16_t AttackPlaylistInstance::GetAtkLoopIteration(size_t flowIndex)
{
    if (mLoopIterOverride)
    {
        return mLoopIterOverride;
    }

    if (mFlows.at(flowIndex)->IsInFlowLoop())
    {
        PlaylistInstance::flowLpHdlrSharedPtr_t loopHdlr = mFlows.at(flowIndex)->GetFlowLoopHandler();

        return loopHdlr->GetCurrIteration();
    }

    return 0;
}

