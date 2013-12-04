/// @file
/// @brief Dpg Client Block implementation
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <vector>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <app/AsyncMessageInterface.tcc>
#include <app/StreamSocketConnector.tcc>
#include <app/StreamSocketHandler.h>
#include <base/EndpointPairEnumerator.h>
#include <base/LoadProfile.h>
#include <base/LoadScheduler.h>
#include <base/LoadStrategy.h>
#include <engine/ModifierBlock.h>
#include <engine/PlaylistEngine.h>
#include <engine/PlaylistInstance.h>
#include <phxexception/PHXExceptionFactory.h>

#include "DpgdLog.h"
#include "SocketManager.h"
#include "DpgClientBlock.h"
#include "DpgClientBlockLoadStrategies.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

struct DpgClientBlock::IOLogicMessage
{
    enum
    {
        STOP_COMMAND,
        CLOSE_COMMAND,
        INTENDED_LOAD_NOTIFICATION,
        AVAILABLE_LOAD_NOTIFICATION,
        SET_DYNAMIC_LOAD,
        ENABLE_DYNAMIC_LOAD,
    } type;

    // Intended and available load
    uint32_t intendedLoad;
    uint32_t availableLoad;

    // Bandwidth parameters
    bool enableDynamicLoad;
    int32_t dynamicLoad;    

    // Close notification parameters
    uint32_t playInstSerial;
};

struct DpgClientBlock::AppLogicMessage
{
    enum
    {
        PLAYLIST_STOP_NOTIFICATION
    } type;
};

///////////////////////////////////////////////////////////////////////////////

DpgClientBlock::DpgClientBlock(uint16_t port, const config_t& config, playlistEngine_t * playlistEngine, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier)
    : mPort(port),
      mConfig(config),
      mStats(BllHandle()),
      mPlaylistEngine(playlistEngine),
      mModifierBlock(playlistEngine->MakeModifierBlock(config.Playlists[0].Handle, 0xabcdef12 /* FIXME - randSeed */)),
      mAppReactor(appReactor),
      mIOReactor(ioReactor),
      mIOBarrier(ioBarrier),
      mEnabled(true),
      mRunning(false),
      mLoadProfile(new loadProfile_t(mConfig.Common.Load)),
      mLoadStrategy(MakeLoadStrategy()),
      mLoadScheduler(new loadScheduler_t(*mLoadProfile, *mLoadStrategy)),
      mAttemptedConnCount(0),
      mActivePlaylists(0),
      mEndpointEnum(new endpointEnum_t(port, IfHandle(), mConfig.Common.Endpoint.DstIf)),
      mIOLogicInterface(new ioMessageInterface_t(mIOReactor)),
      mAppLogicInterface(new appMessageInterface_t(mAppReactor)),
      mAvailableLoadMsgsOut(0),
      mLastSerial(0),
      mStoringStopNotification(false)
{
    mLoadProfile->SetActiveStateChangeDelegate(boost::bind(&DpgClientBlock::LoadProfileHandler, this, _1));
    
    mEndpointEnum->SetPattern(static_cast<endpointEnum_t::pattern_t>(mConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern));
    mEndpointEnum->SetSrcPortNum(0);
    mEndpointEnum->SetDstPortNum(0);

    mIOLogicInterface->SetMessageDelegate(fastdelegate::MakeDelegate(this, &DpgClientBlock::HandleIOLogicMessage));
    mAppLogicInterface->SetMessageDelegate(fastdelegate::MakeDelegate(this, &DpgClientBlock::HandleAppLogicMessage));

    SetDynamicLoad(mConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.DynamicLoad);
    EnableDynamicLoad(mConfig.Common.Load.LoadProfile.UseDynamicLoad);

    InitDebugInfo("DPG", BllHandle());

    TC_LOG_DEBUG_LOCAL(mPort, LOG_CLIENT, "DPG client block [" << Name() << "] created");
}

///////////////////////////////////////////////////////////////////////////////

DpgClientBlock::~DpgClientBlock()
{
    Stop();
}

///////////////////////////////////////////////////////////////////////////////

DpgClientBlock::LoadStrategy *DpgClientBlock::MakeLoadStrategy(void)
{
    SetEnableBwCtrl(false);

    const L4L7_BASE_NS::LoadTypes loadType = static_cast<const L4L7_BASE_NS::LoadTypes>(mConfig.Common.Load.LoadProfile.LoadType);
    switch (loadType)
    {
        case L4L7_BASE_NS::LoadTypes_PLAYLISTS:
            return new PlaylistsLoadStrategy(*this);

        case L4L7_BASE_NS::LoadTypes_BANDWIDTH:
            SetEnableBwCtrl(true);
            return new BandwidthLoadStrategy(*this);             

        default:
        {
            std::ostringstream err;
            err << "Cannot create load strategy for unsupported load type (" << tms_enum_to_string(loadType, L4L7_BASE_NS::em_LoadTypes) << ")";
            TC_LOG_ERR_LOCAL(mPort, LOG_CLIENT, err.str());
            throw EPHXBadConfig(err.str());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::Start(void)
{
    if (!mEnabled || mRunning)
        return;

    mRunning = true;

    TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "DPG client block [" << Name() << "] starting");

    // Reset our internal load state
    mAttemptedConnCount = 0;
    mEndpointEnum->Reset();
    
    mSocketMgr.reset(new SocketManager(mIOReactor));
    mSocketMgr->SetIpv4Tos(mConfig.Common.Profile.L4L7Profile.Ipv4Tos);
    mSocketMgr->SetIpv6TrafficClass(mConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass);
    mSocketMgr->SetTcpWindowSizeLimit(mConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit);
    mSocketMgr->SetTcpDelayedAck(mConfig.Common.Profile.L4L7Profile.EnableDelayedAck);
    mSocketMgr->SetTcpNoDelay(true);
    mSocketMgr->SetStatsInstance(mStats);
    
    // Start load scheduler
    mLoadScheduler->Start(mAppReactor);
    InitBwTimeStorage();
		
	// Set stats Reactor instance
	mStats.SetReactorInstance(mAppReactor);
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::Stop(void)
{
    if (!mEnabled || !mRunning)
        return;

    // Stop load scheduler
    mLoadScheduler->Stop();
    mRunning = false;

    // Reset the intended load
    {
        ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
        mStats.intendedLoad = 0;
    }

    // Ask the I/O logic to stop initiating new connections
    {
        ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
        IOLogicMessage& msg(*msgPtr);
        msg.type = IOLogicMessage::STOP_COMMAND;
        mIOLogicInterface->Send(msgPtr);
    }
    
    // Wait for the stop message to be processed
    mIOLogicInterface->Wait();

    // Wait for all I/O threads to exit
    {
        ACE_WRITE_GUARD(ACE_Lock, guard, *mIOBarrier);
    }
    
    // Ask the I/O logic to close any remaining connections
    {
        ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
        IOLogicMessage& msg(*msgPtr);
        msg.type = IOLogicMessage::CLOSE_COMMAND;
        mIOLogicInterface->Send(msgPtr);
    }
    
    // Wait for the close message to be processed
    mIOLogicInterface->Wait();

    // Wait for all I/O threads to exit
    {
        ACE_WRITE_GUARD(ACE_Lock, guard, *mIOBarrier);
    }

    // Stop load profile last (triggers notification if not already at end)
    mLoadProfile->Stop();

	// Stop stats timer
	mStats.CancelTimer();
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::SetDynamicLoad(int32_t dynamicLoad)
{
    // Ask the I/O logic to set the block load
    ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
    IOLogicMessage& msg(*msgPtr);
    msg.type = IOLogicMessage::SET_DYNAMIC_LOAD;
    msg.dynamicLoad = dynamicLoad;
    mIOLogicInterface->Send(msgPtr);
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::EnableDynamicLoad(bool enable)
{
    // Ask the I/O logic to enable block load
    ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
    IOLogicMessage& msg(*msgPtr);
    msg.type = IOLogicMessage::ENABLE_DYNAMIC_LOAD;
    msg.enableDynamicLoad = enable;
    mIOLogicInterface->Send(msgPtr);
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::NotifyInterfaceDisabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "[DpgClientBlock::NotifyInterfaceDisabled] DPG client block [" << Name() << "] notified interface is disabled");

    Stop();
    mEnabled = false;
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::NotifyInterfaceEnabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "[DpgClientBlock::NotifyInterfaceEnabled] DPG client block [" << Name() << "] notified interface is enabled");
    mEnabled = true;
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::NotifyPlaylistUpdate(const handleSet_t& playlistHandles)
{
    // Check if our playlist is there
    handleSet_t::const_iterator iter = playlistHandles.find(mConfig.Playlists[0].Handle);
    if (iter != playlistHandles.end())
    {
        if (mRunning)
        {
            std::ostringstream oss;
            oss << "Failed to update modifiers used on running client w/ playlist " << *iter;
            TC_LOG_ERR(0, oss.str());
            throw EPHXBadConfig(oss.str());
        }

        // recreate modifier block
        mModifierBlock.reset(mPlaylistEngine->MakeModifierBlock(mConfig.Playlists[0].Handle, 0xabcdef12 /* FIXME - randSeed */));
    }
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::SetIntendedLoad(uint32_t intendedLoad)
{
    // Ask the I/O logic to set the intended load
    ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
    IOLogicMessage& msg(*msgPtr);
    msg.type = IOLogicMessage::INTENDED_LOAD_NOTIFICATION;
    msg.intendedLoad = intendedLoad;
    mIOLogicInterface->Send(msgPtr);
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::SetAvailableLoad(uint32_t availableLoad, uint32_t intendedLoad)
{
    const uint32_t MAX_AVAILABLE_OUT = 2;

    // don't accumulate too many messages
    if (mAvailableLoadMsgsOut >= MAX_AVAILABLE_OUT)
        return;

    ++mAvailableLoadMsgsOut;

    // Ask the I/O logic to set the intended and available load
    ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
    IOLogicMessage& msg(*msgPtr);
    msg.type = IOLogicMessage::AVAILABLE_LOAD_NOTIFICATION;
    msg.availableLoad = availableLoad;
    msg.intendedLoad = intendedLoad;
    mIOLogicInterface->Send(msgPtr);
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::HandleIOLogicMessage(const IOLogicMessage& msg)
{
    switch (msg.type)
    {
      case IOLogicMessage::STOP_COMMAND:
      {
          TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "DPG client block [" << Name() << "] notifying playlists of upcoming stop");
          PreStopPlaylists();

          TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "DPG client block [" << Name() << "] stopping initiating new connections");

          mSocketMgr->Stop();

          // Signal that the stop is complete
          mIOLogicInterface->Signal();
          break;
      }

      case IOLogicMessage::CLOSE_COMMAND:
      {
          TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "DPG client block [" << Name() << "] stopping " << mActivePlaylists << " active playlists");

          // Stop all active playlists 
          ReapPlaylists(mActivePlaylists);

          mSocketMgr->Reset();

          // Signal that the close command was processed
          mIOLogicInterface->Signal();
          break;
      }

      case IOLogicMessage::SET_DYNAMIC_LOAD:  // Limited to 1 Connection
      {
          DynamicLoadHandler(mActivePlaylists, msg.dynamicLoad);
          break;
      }      

      case IOLogicMessage::ENABLE_DYNAMIC_LOAD:
      { 
          SetEnableDynamicLoad(msg.enableDynamicLoad);
          mLoadScheduler->SetEnableDynamicLoad(msg.enableDynamicLoad);
          break;
      }      

      case IOLogicMessage::INTENDED_LOAD_NOTIFICATION:
      {
          const ssize_t delta = msg.intendedLoad - mActivePlaylists;

          if (delta > 0 && mEnabled && mRunning)
              SpawnPlaylists(static_cast<size_t>(delta));
          // ignore negative delta in DPG
          break;
      }

      case IOLogicMessage::AVAILABLE_LOAD_NOTIFICATION:
      {
          if (msg.availableLoad && mEnabled && mRunning)
              SpawnPlaylists(msg.availableLoad);

          --mAvailableLoadMsgsOut;
          break;
      }
      
      default:
          throw EPHXInternal("unknown IOLogicMessage");
    }
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::SpawnPlaylists(size_t count)
{
    // New connections may be limited by the L4-L7 load profile
    const uint32_t maxConnAttempted = mConfig.Common.Load.LoadProfile.MaxConnectionsAttempted;
    if (maxConnAttempted)
        count = std::min(maxConnAttempted - mAttemptedConnCount, count);
        
    const uint32_t maxOpenConn = mConfig.Common.Load.LoadProfile.MaxOpenConnections;
    if (maxOpenConn)
        count = std::min(maxOpenConn - mActivePlaylists, count);

    size_t countDecr = 1;

    for (; count != 0; count -= countDecr)
    {
        // Get the next endpoint pair in our block
        std::string srcIfName;
        ACE_INET_Addr srcAddr;
        ACE_INET_Addr dstAddr;
              
        mEndpointEnum->GetEndpointPair(srcIfName, srcAddr, dstAddr);
        mEndpointEnum->Next();

        if (mConfig.Playlists.empty())
        {
            std::ostringstream err;
            err << "Cannot start client block with no playlist";
            TC_LOG_ERR_LOCAL(mPort, LOG_CLIENT, err.str());
            throw EPHXBadConfig(err.str());
        }

        playInstSharedPtr_t pi(mPlaylistEngine->MakePlaylist(mConfig.Playlists[0].Handle, *mModifierBlock, *mSocketMgr, true), boost::bind(&L4L7_ENGINE_NS::PlaylistInstance::RemoveReference, _1));

        // FIXME - do we need a lock around here?
        mModifierBlock->Next();

        pi->SetStatDelegate(boost::bind(&DpgClientBlock::HandleStatNotification, this)); 
        pi->SetStoppedDelegate(boost::bind(&DpgClientBlock::HandleStopNotification, this, _1)); 
        mSocketMgr->AddPlaylist(pi.get(), srcIfName, srcAddr, dstAddr);

        mActivePlaylists++;

        {
            ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
            mStats.activePlaylists = mActivePlaylists;
            ++mStats.attemptedPlaylists;
        }

        {
            ACE_GUARD(playInstLock_t, play_guard, mPlayInstLock);
            pi->SetSerial(++mLastSerial);
            mPlayInstMap[mLastSerial] = pi;
            mPlayInstAgingQueue.push(mLastSerial);
        }

        pi->Start();
    }
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::ReapPlaylists(size_t count)
{

    count = std::min(mActivePlaylists, count);
    
    // Stop up to the specified number of playlists, oldest first
    size_t closed = 0;
    while (!mPlayInstAgingQueue.empty() && closed < count)
    {
        uint32_t playInstSerial = mPlayInstAgingQueue.front();
        mPlayInstAgingQueue.pop();

        playInstSharedPtr_t playInst;

        {
            ACE_GUARD(playInstLock_t, play_guard, mPlayInstLock);
            playInstMap_t::iterator iter = mPlayInstMap.find(playInstSerial);
            if (iter == mPlayInstMap.end())
                continue;

            playInst = iter->second;
        }

        playInst->ClearStoppedDelegate();
        playInst->Stop();
        mSocketMgr->RemovePlaylist(playInst.get());
        bool erased = false;

        {
            // Delete the connection handler from our map
            ACE_GUARD(playInstLock_t, play_guard, mPlayInstLock);
            erased = mPlayInstMap.erase(playInstSerial);
        }

        closed++;

        if (erased)
        {
            ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
            mActivePlaylists--;
            mStats.activePlaylists = mActivePlaylists;
            if ( playInst->GetState() == L4L7_ENGINE_NS::PlaylistInstance::FAIL_TO_CONNECT)
            {
                // if we fail to connect, we didn't actually attempt it the playlist. decrement count.
                --mStats.attemptedPlaylists;
            }
            else
            {
                mStats.CompletePlaylist(playInst->GetDurationMsec(), playInst->GetState() == L4L7_ENGINE_NS::PlaylistInstance::SUCCESSFUL, playInst->GetState() == L4L7_ENGINE_NS::PlaylistInstance::ABORTED);
            }
        }
    }

    ForwardStopNotification();
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::PreStopPlaylists()
{
    // Notify any running playlists of the coming stop so that when things
    // stop working they are considered "aborts"

    ACE_GUARD(playInstLock_t, play_guard, mPlayInstLock);
    BOOST_FOREACH(playInstMap_t::value_type& playInstPair, mPlayInstMap)
    {
        playInstPair.second->PreStop();
    }
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::DynamicLoadHandler(size_t conn, int32_t dynamicLoad)
{
    int32_t load = ConfigDynamicLoad(conn, dynamicLoad);
    if (load < 0)
        return;

    if (load == 0)
        ReapPlaylists(mActivePlaylists);

    ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
    mStats.intendedLoad = static_cast<uint32_t>(load);
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::HandleAppLogicMessage(const AppLogicMessage& msg)
{
    switch (msg.type)
    {
      case AppLogicMessage::PLAYLIST_STOP_NOTIFICATION:
      {
          // Notify the load strategy that a connection was closed
          if (mLoadScheduler->Running())
              mLoadStrategy->PlaylistComplete();

          break;
      }

      default:
          throw EPHXInternal("unknown AppLogicMessage");
    }
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::HandleStatNotification()
{
    // For now, the only stat we get is +1 to TxAttack. If we need more stats
    // add arguments/processing here
    ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
    ++mStats.txAttack;
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::HandleStopNotification(L4L7_ENGINE_NS::PlaylistInstance * pi)
{
    // keep the erase from destroying the instance until this method returns
    playInstSharedPtr_t tempPi;

    bool erased = false;

    {
        ACE_GUARD(playInstLock_t, play_guard, mPlayInstLock);
        playInstMap_t::iterator iter = mPlayInstMap.find(pi->GetSerial());
        if (iter != mPlayInstMap.end())
        {
            tempPi = iter->second;
            mSocketMgr->RemovePlaylist(tempPi.get());
        }
        erased = mPlayInstMap.erase(pi->GetSerial());
    }

    if (erased)
    {
        mActivePlaylists--;
        ForwardStopNotification();
        ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
        mStats.activePlaylists = mActivePlaylists;
        if ( pi->GetState() == L4L7_ENGINE_NS::PlaylistInstance::FAIL_TO_CONNECT)
        {
          // if we fail to connect, we didn't actually attempt it the playlist. decrement count.
          --mStats.attemptedPlaylists;
        }
        else
        {
            mStats.CompletePlaylist(pi->GetDurationMsec(), pi->GetState() == L4L7_ENGINE_NS::PlaylistInstance::SUCCESSFUL, pi->GetState() == L4L7_ENGINE_NS::PlaylistInstance::ABORTED);
        }

    }

    mPlayInstAgingQueue.erase(pi->GetSerial());

    // Notify the application logic that a playlist was stopped
    {
        appMessageInterface_t::messagePtr_t msgPtr(mAppLogicInterface->Allocate());
        AppLogicMessage& msg(*msgPtr);
        msg.type = AppLogicMessage::PLAYLIST_STOP_NOTIFICATION;
        mAppLogicInterface->Send(msgPtr);
    }

}

///////////////////////////////////////////////////////////////////////////////

size_t DpgClientBlock::GetMaxTransactions(size_t count, size_t& countDecr)
{
    const L4L7_BASE_NS::LoadTypes loadType = static_cast<const L4L7_BASE_NS::LoadTypes>(mConfig.Common.Load.LoadProfile.LoadType);

    if (loadType == L4L7_BASE_NS::LoadTypes_TRANSACTIONS)
    {
        // If our load type is transactions, we limit each client connection to one transaction
        return 1;
    }
    else if (loadType == L4L7_BASE_NS::LoadTypes_TRANSACTIONS_PER_TIME_UNIT)
    {
        // For transactions/second, we use as many as we can per connection, subtracting transactions from the count
        return countDecr = std::min(count, mConfig.ProtocolConfig.MaxTransactionsPerServer);
    }
    else
    {
        return mConfig.ProtocolConfig.MaxTransactionsPerServer;
    }
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::LoadProfileHandler(bool running)
{
    if (running || !mActivePlaylists)
    {
        // Cascade notification
        if (!mLoadProfileNotifier.empty())
            mLoadProfileNotifier(BllHandle(), running);
    }
    else 
    {
        // Delay stopped notification until active connections == 0
        mStoringStopNotification = true;
    }
}

///////////////////////////////////////////////////////////////////////////////

void DpgClientBlock::ForwardStopNotification() 
{ 
    if (mStoringStopNotification && !mActivePlaylists) 
    { 
        mStoringStopNotification = false;
        if (!mLoadProfileNotifier.empty())
            mLoadProfileNotifier(BllHandle(), false);
    }
}

///////////////////////////////////////////////////////////////////////////////
