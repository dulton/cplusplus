/// @file
/// @brief Stream instance implementation
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
#include <sstream>

#include "EngineHooks.h"
#include "FlowEngine.h"
#include "FlowInstance.h"
#include "StreamInstance.h"
#include "PlaylistInstance.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

void StreamInstance::Start()
{
    Reset();
    if (mConfig->pktList.size())
    {
        mState = STARTED;
        DoNextPacket();
    }
    else
    {
        // no packet
        Complete();
    }
}

///////////////////////////////////////////////////////////////////////////////

void StreamInstance::Reset()
{
    CancelTxRxTimer();
    CancelInactivityTimer();
    mPacketIdx = 0;
    mState = WAIT_TO_START;
    mPktLoopCounters.clear();
    mCurPktLoopId = mConfig->pktList.size();
    mNextPktIdx = mConfig->pktList.size();
}

///////////////////////////////////////////////////////////////////////////////

// Use when we want a do-nothing delegate (empty delegates throw when called)
static void DoNothing() 
{
}

///////////////////////////////////////////////////////////////////////////////

void StreamInstance::DoNextPacket()
{
    CancelInactivityTimer();
  // if we get here because of a timeout, clear the timer.
    const handle_t invalidTimer = mEngine.GetTimerMgr().InvalidTimerHandle();
    if (mTimer!=invalidTimer)
    {
        mTimer = invalidTimer;
    }

    if (mPacketIdx >= mConfig->pktList.size())
    {   // no more packets, done
        Complete();
    }
    else
    {
        SetupInactivityTimer();

        const FlowConfig::Packet& packet(mConfig->pktList[mPacketIdx]);
        const FlowConfig::Packet* nextPacket = GetNextPkt();

        if (nextPacket == 0) // last packet
        {
            bool isSend = !(mClient ^ packet.clientTx); // we need to send or wait for packet arrival?

            if (isSend)
            {
                UpdateCurrentPktIdx();
                AbstSocketManager::sendDelegate_t delegate(boost::bind(&StreamInstance::DoNextPacket, this));
                SendPacket(&packet, delegate);
            }
            else
            {
                AbstSocketManager::recvDelegate_t delegate(boost::bind(&StreamInstance::PacketReceived, this));
                mPlaylistInstance->GetSocketMgr().AsyncRecv(mFd, GetPacketLength(&packet), delegate);
            }
        }
        else // at least two more packets left
        {
            bool isSend = !(mClient ^ packet.clientTx); // we need to send or wait for packet arrival?
            bool nextPktIsSend = !(mClient ^ nextPacket->clientTx); // same thing for the next packet

            if (isSend)
            {
                if (nextPktIsSend)  // two Tx-es
                {
                    if (mAdjustedPktDelays)
                    {
                        uint32_t delay = (*mAdjustedPktDelays)[mPacketIdx];
                        UpdateCurrentPktIdx();
                        AbstSocketManager::sendDelegate_t delegate(boost::bind(&StreamInstance::CancelTimeoutAndWait, this, delay));
                        SendPacket(&packet, delegate);
                    }
                    else
                    {
                        std::ostringstream oss;
                        oss << "[StreamInstance::DoNextPacket] Exception occured when trying to read packet delay.";
                        const std::string err(oss.str());
                        EngineHooks::LogErr(0, err);
                        Stop();
                    }
                }
                else // Tx followed by Rx
                {
                    UpdateCurrentPktIdx();

                    // schedule rx if not last packet
                    if (mPacketIdx < mConfig->pktList.size())
                    {
                        GetNextPkt();
                        AbstSocketManager::recvDelegate_t delegate(boost::bind(&StreamInstance::PacketReceived, this));
                        mPlaylistInstance->GetSocketMgr().AsyncRecv(mFd, GetPacketLength(nextPacket), delegate);
                    }
                    AbstSocketManager::sendDelegate_t delegate = AbstSocketManager::sendDelegate_t(DoNothing);
                    SendPacket(&packet, delegate);
                }
            }
            else // Rx followed by whatever
            {
                AbstSocketManager::recvDelegate_t delegate(boost::bind(&StreamInstance::PacketReceived, this));
                mPlaylistInstance->GetSocketMgr().AsyncRecv(mFd, GetPacketLength(&packet), delegate);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void StreamInstance::Wait(uint32_t delay)
{
    bool preStop = mPlaylistInstance->GetPreStop();
    if( preStop )
    {
       mTimer = mEngine.GetTimerMgr().InvalidTimerHandle();
       return;
    }
    AbstTimerManager::timerDelegate_t delegate(boost::bind(&StreamInstance::DoNextPacket, this));
    handle_t invalidHandle = mEngine.GetTimerMgr().InvalidTimerHandle();
    handle_t timerHandle = mEngine.GetTimerMgr().CreateTimer(delegate, delay);

    if (timerHandle != invalidHandle)
    {
        // if the timer handle is invalid, create timer may have already
        // called the delegate, closed the socket, and deleted this object
        mTimer = timerHandle;
    }
}

///////////////////////////////////////////////////////////////////////////////
void StreamInstance::CancelTimeoutAndWait(uint32_t delay)
{
   CancelInactivityTimer();
   Wait(delay);
}
///////////////////////////////////////////////////////////////////////////////

void StreamInstance::PacketReceived()
{
    CancelInactivityTimer();

    if (mPacketIdx >= mConfig->pktList.size())
    {
        Complete();
    }
    else
    {
        const FlowConfig::Packet& packet(mConfig->pktList[mPacketIdx]);

        if (mNextPktIdx >= mConfig->pktList.size()) // last packet
        {
            // NOTE: currently, we do nothing with the incoming packet and proceed to the next packet
            // However, we will do something in the near future

            bool isSend = !(mClient ^ packet.clientTx); // we need to send or wait for packet arrival?
            if (isSend) // received a packet while processing an outgoing packet? State error!
            {
                std::ostringstream oss;
                oss << "[StreamInstance:PacketReceived] Packet received while supposed to be sending. Flow Id: " << mFlowIndex << " Pkt Idx: " << mPacketIdx;
                const std::string err(oss.str());
                EngineHooks::LogErr(0, err);
                Stop();
            }
            else
            {
                UpdateCurrentPktIdx();
                DoNextPacket();
            }
        }
        else
        {
            const FlowConfig::Packet& nextPacket(mConfig->pktList[mNextPktIdx]);
            bool isSend = !(mClient ^ packet.clientTx); // we need to send or wait for packet arrival?
            bool nextPktIsSend = !(mClient ^ nextPacket.clientTx); // same thing for the next packet

            if (isSend) // received a packet while processing an outgoing packet? State error!
            {
                //FIXME: stop or only report a stream failure or ignore?
                Stop();
            }
            else
            {
                // NOTE: currently, we do nothing with the incoming packet and proceed to the next packet
                // However, we will do something in the near future

                if (nextPktIsSend)  // next is Tx
                {
                    uint32_t delay = (*mAdjustedPktDelays)[mPacketIdx];
                    UpdateCurrentPktIdx();
                    SetupInactivityTimer();
                    if (mAdjustedPktDelays)
                    {
                        Wait(delay);
                    }
                    else
                    {
                        std::ostringstream oss;
                        oss << "[StreamInstance::PacketReceived] Exception occured when trying to read packet delay.";
                        const std::string err(oss.str());
                        EngineHooks::LogErr(0, err);
                        Stop();
                    }
                }
                else // next is Rx
                {
                    UpdateCurrentPktIdx();
                    DoNextPacket();
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

const FlowConfig::Packet * StreamInstance::GetNextPkt()
{
    const FlowConfig::Packet& packet(mConfig->pktList[mPacketIdx]);

    // if this is the end of a packet loop
    if (packet.loopInfo)
    {
        // first iteration
        if  (mCurPktLoopId != mPacketIdx)
        {
            LoopCounter counter = {mPacketIdx, packet.loopInfo->count-1};
            mPktLoopCounters.push_back(counter); // already iterated once
            mNextPktIdx = packet.loopInfo->begIdx;
            mCurPktLoopId = mPacketIdx; // set the flag that we're currently doing THIS loop
        }
        // last iteration
        else if (mPktLoopCounters.back().count == 1)
        {
            mNextPktIdx = mPacketIdx + 1;
            mPktLoopCounters.pop_back();
            if (mPktLoopCounters.size())
            {
                mCurPktLoopId = mPktLoopCounters.back().loopId; // the outer loop
            }
            else
            {
                mCurPktLoopId = mConfig->pktList.size(); // set to some invalid index
            }
        }
        // need more iteration
        else
        {
            mNextPktIdx = packet.loopInfo->begIdx;
            mPktLoopCounters[mPktLoopCounters.size()-1].count = mPktLoopCounters.back().count - 1;
        }
    }
    else
    {
        mNextPktIdx = mPacketIdx + 1;
    }

    if ( mNextPktIdx >= mConfig->pktList.size())
    {
        return 0;
    }
    else
    {
        return &(mConfig->pktList[mNextPktIdx]);
    }

}

///////////////////////////////////////////////////////////////////////////////

void StreamInstance::UpdateCurrentPktIdx()
{
    mPacketIdx = mNextPktIdx;
}

///////////////////////////////////////////////////////////////////////////////

