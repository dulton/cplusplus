/// @file
/// @brief Attack instance implementation
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
#include <boost/foreach.hpp>
#include <sstream>

#include "EngineHooks.h"
#include "FlowEngine.h"
#include "FlowInstance.h"
#include "AttackInstance.h"
#include "PlaylistInstance.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

AttackInstance::AttackInstance(FlowEngine& engine, handle_t flowHandle, PlaylistInstance *pi, size_t flowIdx, bool client, flowLpHdlrSharedPtr_t loopHandler) : FlowInstance(engine, flowHandle, pi, flowIdx, client, loopHandler),
                        mWaitTimeout(mPlaylistInstance->GetWaitTimeout(flowIdx)),
                        mInactivityTimeout(mPlaylistInstance->GetInactivityTimeout(flowIdx)),
                        mClientPayload(mConfig->mClientPayload),
                        mServerPayload(mConfig->mServerPayload),
                        mCurrentClientPayloadIndex(0),
                        mCurrentServerPayloadIndex(0),
                        mNumOfPktsReceived(0),
                        mNumOfResponsesExpected(0),
                        mGracePeriodStarted(false),
                        mTxPktLoopHandler(mConfig),
                        mRxPktLoopHandler(mConfig)
{
    if (mClient)
    {
        mNumOfPktsToSend = mConfig->mNumOfClientPktsToPlay;
    }
    else
    {
        mNumOfPktsToSend = mConfig->mNumOfServerPktsToPlay;
    }
}

///////////////////////////////////////////////////////////////////////////////

AttackInstance::AttackInstance(FlowInstance & flow):FlowInstance(flow),
                        mWaitTimeout(mPlaylistInstance->GetWaitTimeout(flow.GetIndex())),
                        mInactivityTimeout(mPlaylistInstance->GetInactivityTimeout(flow.GetIndex())),
                        mClientPayload(mConfig->mClientPayload),
                        mServerPayload(mConfig->mServerPayload),
                        mCurrentClientPayloadIndex(0),
                        mCurrentServerPayloadIndex(0),
                        mNumOfPktsReceived(0),
                        mNumOfResponsesExpected(0),
                        mGracePeriodStarted(false),
                        mTxPktLoopHandler(mConfig),
                        mRxPktLoopHandler(mConfig)
{
    if (mClient)
    {
        mNumOfPktsToSend = mConfig->mNumOfClientPktsToPlay;
    }
    else
    {
        mNumOfPktsToSend = mConfig->mNumOfServerPktsToPlay;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AttackInstance::Reset()
{
    CancelTxRxTimer();
    CancelInactivityTimer();
    mState = WAIT_TO_START;
    mCurrentClientPayloadIndex = 0;
    mCurrentServerPayloadIndex = 0;
    mNumOfPktsReceived = 0;
    mGracePeriodStarted = false;
    mTxPktLoopHandler.Reset();
    mRxPktLoopHandler.Reset();
}


///////////////////////////////////////////////////////////////////////////////

void AttackInstance::Start()
{
    Reset();
    if (mConfig->pktList.size())
    {
        if (mClientPayload.size()==0)
        {
            std::ostringstream oss;
            oss << "[AttackInstance:Start] empty attack.";
            const std::string msg(oss.str());
            EngineHooks::LogWarn(0, msg);
            Complete();
        }
        else
        {
            Initialize();
            mState = STARTED;
            SetupInactivityTimer(mInactivityTimeout);
            if (GetAttackType()==FlowConfig::TCP)
            {
                TCPStart();
            }
            else
            {
                UDPStart();
            }
        }
    }
    else
    {
        // no packet
        std::ostringstream oss;
        oss << "[AttackInstance:Start] empty attack.";
        const std::string msg(oss.str());
        EngineHooks::LogWarn(0, msg);
        Complete();
    }
}

//////////////////////////////////////////////////////////////////////////////////

void AttackInstance::Initialize()
{
    if (mClient) //client
    {
        if (GetAttackType()==FlowConfig::TCP)
        {
            if ( mServerPayload.size() != 0 && mConfig->mNumOfServerPktsToPlay < mNumOfPktsToSend)
            {
                mNumOfResponsesExpected = mConfig->mNumOfServerPktsToPlay;
            }
            else
            {
                mNumOfResponsesExpected = mNumOfPktsToSend;
            }
        }
    }
    else // server
    {
        // The server will use the default payload, as many times as
        // the number of client's payloads
        mNumOfResponsesExpected = mConfig->mNumOfClientPktsToPlay;
    }

    if (mNumOfResponsesExpected == 0)
    {
        mNumOfResponsesExpected = mNumOfPktsToSend;
    }
}

//////////////////////////////////////////////////////////////////////////////////

void AttackInstance::TCPStart()
{
//client
    if (mClient)
    {
        WaitForSend(mWaitTimeout/3);
        if (mServerPayload.size()==0)
        {
            WaitForIncomingPacket(&ATTACK_DEFAULT_RESPONSE);
        }
        else
        {
            const FlowConfig::Packet& packet(mConfig->pktList[mServerPayload[mCurrentServerPayloadIndex]]);
            WaitForIncomingPacket(&packet);
        }
    }
//server
    else
    {
        WaitForSend(mWaitTimeout);
        const FlowConfig::Packet& packet(mConfig->pktList[mClientPayload[mCurrentClientPayloadIndex]]);
        WaitForIncomingPacket(&packet);
    }
}

//////////////////////////////////////////////////////////////////////////////

void AttackInstance::UDPStart()
{
//client
    if (mClient)
    {
        WaitForSend(mWaitTimeout/3);
        const FlowConfig::Packet& packet(mConfig->pktList[mServerPayload[mCurrentServerPayloadIndex]]);
        WaitForIncomingPacket(&packet);
    }
//server
    else
    {
        const FlowConfig::Packet& packet(mConfig->pktList[mClientPayload[mCurrentClientPayloadIndex]]);
        WaitForIncomingPacket(&packet);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AttackInstance::WaitForSend(uint32_t delay)
{

	bool preStop = mPlaylistInstance->GetPreStop();
	if( preStop)
	{
		mTimer = mEngine.GetTimerMgr().InvalidTimerHandle();
		return;
	}
    AbstTimerManager::timerDelegate_t delegate(boost::bind(&AttackInstance::DoNextPacket, this));

    handle_t timerHandle = mEngine.GetTimerMgr().CreateTimer(delegate, delay);

    mTimer = timerHandle;
}

///////////////////////////////////////////////////////////////////////////////
/** \param delay the delay for sending next packet
*/
void AttackInstance::SendPacket(const FlowConfig::Packet * packet, uint32_t delay)
{
    AbstSocketManager::sendDelegate_t delegate(boost::bind(&AttackInstance::WaitForSend, this, delay));
    FlowInstance::SendPacket(packet, delegate);
}

///////////////////////////////////////////////////////////////////////////////

void AttackInstance::WaitForIncomingPacket(const FlowConfig::Packet* packet)
{
    AbstSocketManager::recvDelegate_t delegate(boost::bind(&AttackInstance::PacketReceived, this));

    if (mFd)
    {
        // it's possible that a close has happened prematurely since attacks aren't synced
        mPlaylistInstance->GetSocketMgr().AsyncRecv(mFd, GetPacketLength(packet), delegate);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// Called when wait timeout timer fires
//
void AttackInstance::DoNextPacket()
{
    CancelInactivityTimer();
    // if we get here because of a timeout, clear the timer.
    const handle_t invalidTimer = mEngine.GetTimerMgr().InvalidTimerHandle();
    if (mTimer!=invalidTimer)
    {
        mTimer = invalidTimer;
    }

    const FlowConfig::Packet* packet = GetNextPktToSend();

//client
    if (mClient)
    {
        if (packet)
        {
            SendPacket(packet, mWaitTimeout);
            SetupInactivityTimer(mInactivityTimeout);
        }
        else
        {
            if ( (GetAttackType()==FlowConfig::TCP) && (mNumOfPktsReceived < mNumOfResponsesExpected) && (false == mGracePeriodStarted) )
            {
                //We are doing this to give a grace period for a session that would be
                //unsuccessful. This takes care of large server's payloads
                mGracePeriodStarted = true;
                WaitForSend(4*mWaitTimeout);
                SetupInactivityTimer(mInactivityTimeout);
            }
            else
            {
                Complete();
            }
        }
    }
//server
    else
    {
        if (packet)
        {
            SendPacket(packet, mWaitTimeout);
            SetupInactivityTimer(mInactivityTimeout);
        }
	else if (GetAttackType()==FlowConfig::UDP && mNumOfPktsReceived >= mNumOfResponsesExpected)
	{
	    // udp server cannot wait for the client to close the socket
	    Complete();
	}
    }
}

////////////////////////////////////////////////////////////////////////////////

void AttackInstance::PacketReceived()
{
    SetupInactivityTimer(mInactivityTimeout);
    mNumOfPktsReceived++;
    // UDP server instance starts the wait timer only when received the first packet
    if (!mClient && mCurrentServerPayloadIndex == 0 && GetAttackType()==FlowConfig::UDP)
    {
        // send delay for the first packet is 0;
        WaitForSend(0);
    }

    const FlowConfig::Packet* pkt = GetNextPktToReceive();

    if (pkt)
    {
        WaitForIncomingPacket(pkt);
    }
}

////////////////////////////////////////////////////////////////////////////////

const FlowConfig::Packet * AttackInstance::GetNextPktToSend()
{
    if (mClient)
    {
        const FlowConfig::Packet * thePacket;
        if (mCurrentClientPayloadIndex < mClientPayload.size())
        {
            thePacket = &(mConfig->pktList[mClientPayload[mCurrentClientPayloadIndex]]);
            mCurrentClientPayloadIndex = mTxPktLoopHandler.FindNextPktIdx(mClientPayload, mCurrentClientPayloadIndex);
        }
        else
        {
            mCurrentClientPayloadIndex++;
            thePacket = 0;
        }
        return thePacket;
    }
    else // server
    {
        if (mServerPayload.size() == 0)
        {
            // return default packet
            mCurrentServerPayloadIndex++;
            return &ATTACK_DEFAULT_RESPONSE;
        }
        else
        {
            if (mCurrentServerPayloadIndex < mServerPayload.size())
            {
                const FlowConfig::Packet& nextPacket(mConfig->pktList[mServerPayload[mCurrentServerPayloadIndex]]);
                mCurrentServerPayloadIndex = mTxPktLoopHandler.FindNextPktIdx(mServerPayload, mCurrentServerPayloadIndex);
                return &nextPacket;
            }
            else
            {
                mCurrentServerPayloadIndex++;
                return 0;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

const FlowConfig::Packet * AttackInstance::GetNextPktToReceive()
{
    if (mClient)
    {
        if (mNumOfPktsReceived < mNumOfResponsesExpected)
        {
            if (mCurrentServerPayloadIndex < mServerPayload.size())
            {
                const FlowConfig::Packet& nextPacket(mConfig->pktList[mServerPayload[mCurrentServerPayloadIndex]]);
                mCurrentServerPayloadIndex = mRxPktLoopHandler.FindNextPktIdx(mServerPayload, mCurrentServerPayloadIndex);
                return &nextPacket;
            }
            else
            {
                // return default packet
                mCurrentServerPayloadIndex++;
                return &ATTACK_DEFAULT_RESPONSE;
            }
        }
        else
        {
            mCurrentServerPayloadIndex++;
            return 0;
        }
    }
    else //server
    {
        if (mCurrentClientPayloadIndex < mNumOfResponsesExpected - 1)
        {
            const FlowConfig::Packet& nextPacket(mConfig->pktList[mClientPayload[mCurrentClientPayloadIndex]]);
            mCurrentClientPayloadIndex = mRxPktLoopHandler.FindNextPktIdx(mClientPayload, mCurrentClientPayloadIndex);
            return &nextPacket;
        }
        else
        {
            if (mServerPayload.size() == 0)
            {
                mCurrentServerPayloadIndex = mNumOfResponsesExpected;
            }
            return 0;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
