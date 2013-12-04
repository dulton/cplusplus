/// @file
/// @brief Dpg Server Block implementation
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <ildaemon/LocalInterfaceEnumerator.h>

#include <app/AsyncMessageInterface.tcc>
#include <app/StreamSocketConnector.tcc>
#include <app/StreamSocketHandler.h>
#include <engine/ModifierBlock.h>
#include <engine/PlaylistEngine.h>
#include <engine/PlaylistInstance.h>
#include <phxexception/PHXExceptionFactory.h>

#include "DpgdLog.h"
#include "SocketManager.h"
#include "RemAddrUtils.h"
#include "TcpPortUtils.h"
#include "DpgServerBlock.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

struct DpgServerBlock::IOLogicMessage
{
    enum
    {
        STOP_COMMAND,
        CLOSE_COMMAND,
    } type;
};

///////////////////////////////////////////////////////////////////////////////

DpgServerBlock::DpgServerBlock(uint16_t port, const config_t& config, playlistEngine_t * playlistEngine, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier)
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
      mActivePlaylists(0),
      mLastSerial(0),
      // FIXME -- need a kind of endpoint map mEndpointEnum(new endpointEnum_t(port, IfHandle(), mConfig.Common.Endpoint.DstIf)),
      mIOLogicInterface(new ioMessageInterface_t(mIOReactor)),
      mIfEnum(new ifEnum_t(port, IfHandle()))
{
    mIOLogicInterface->SetMessageDelegate(fastdelegate::MakeDelegate(this, &DpgServerBlock::HandleIOLogicMessage));

    TC_LOG_DEBUG_LOCAL(mPort, LOG_CLIENT, "DPG server block [" << Name() << "] created");
    mSocketMgr.reset(new SocketManager(mIOReactor));

}

///////////////////////////////////////////////////////////////////////////////

DpgServerBlock::~DpgServerBlock()
{
    Stop();
}

///////////////////////////////////////////////////////////////////////////////

void DpgServerBlock::Start(void)
{
    if (!mEnabled || mRunning)
        return;

    mRunning = true;

    TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "DPG server block [" << Name() << "] starting");

    // Reset endpoint enumerator
    mIfEnum->Reset();

    // Reset our internal state
    mSocketMgr->SetIpv4Tos(mConfig.Common.Profile.L4L7Profile.Ipv4Tos);
    mSocketMgr->SetIpv6TrafficClass(mConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass);
    mSocketMgr->SetTcpWindowSizeLimit(mConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit);
    mSocketMgr->SetTcpDelayedAck(mConfig.Common.Profile.L4L7Profile.EnableDelayedAck);
    mSocketMgr->SetTcpNoDelay(true);
    mSocketMgr->SetStatsInstance(mStats);

    // Start servers
    std::string ifName;
    ACE_INET_Addr addr;
    const size_t numServers = mIfEnum->TotalCount();
    const size_t numPlaylists = mConfig.Playlists.size();
    bool needsRemAddrSet = RemAddrUtils::NeedsAddrSet(*mPlaylistEngine, mConfig);
    for (size_t i = 0; i < numServers; ++i)
    {
        mIfEnum->SetPortNum(0 /* to be re-set at Listen time */);
        mIfEnum->GetInterface(ifName, addr);
        mIfEnum->Next();

        for (size_t p = 0; p < numPlaylists; ++p)
        {
            // For a server, special no-variable playlists are created and started
            // When an actual connection comes in, regular playlists are created and hooked up

            L4L7_ENGINE_NS::PlaylistInstance * pi = mPlaylistEngine->MakePlaylist(mConfig.Playlists[p].Handle, *mModifierBlock, *mSocketMgr, false);
            pi->SetStoppedDelegate(boost::bind(&DpgServerBlock::HandleStopNotification, this, _1));

            SocketManager::addrSet_t remAddrSet;
            if (needsRemAddrSet)
            {
                RemAddrUtils::GetAddrSet(remAddrSet, mConfig.DstIfs[p]);
            }

            mSocketMgr->AddListeningPlaylist(pi, ifName, addr, remAddrSet, boost::bind(&DpgServerBlock::ServerSpawn, this, _1, _2, _3, _4));

            playInstSharedPtr_t playInst(pi, boost::bind(&L4L7_ENGINE_NS::PlaylistInstance::RemoveReference, _1));

            {
                ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
                // FIXME -- any stats?
            }

            {
                ACE_GUARD(playInstLock_t, play_guard, mPlayInstLock);
                pi->SetSerial(++mLastSerial);
                mPlayInstMap[mLastSerial] = playInst;
            }

            pi->Start();
        }
    } 
		
	// Set stats Reactor instance
	mStats.SetReactorInstance(mAppReactor);
}

///////////////////////////////////////////////////////////////////////////////

void DpgServerBlock::Stop(void)
{
    if (!mEnabled || !mRunning)
        return;

    mRunning = false;

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
		
	// Stop stats timer
	mStats.CancelTimer();
}

///////////////////////////////////////////////////////////////////////////////

void DpgServerBlock::NotifyInterfaceDisabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "[DpgServerBlock::NotifyInterfaceDisabled] DPG server block [" << Name() << "] notified interface is disabled");

    Stop();
    mEnabled = false;
}

///////////////////////////////////////////////////////////////////////////////

void DpgServerBlock::NotifyInterfaceEnabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "[DpgServerBlock::NotifyInterfaceEnabled] DPG server block [" << Name() << "] notified interface is enabled");
    mEnabled = true;
}

///////////////////////////////////////////////////////////////////////////////

void DpgServerBlock::NotifyPlaylistUpdate(const handleSet_t& playlistHandles)
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

void DpgServerBlock::HandleIOLogicMessage(const IOLogicMessage& msg)
{
    switch (msg.type)
    {
      case IOLogicMessage::STOP_COMMAND:
      {

    	  PreStopPlaylists();
          TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "DPG server block [" << Name() << "] stopping accepting new connections");

          mSocketMgr->Stop();
          
          // Signal that the stop is complete
          mIOLogicInterface->Signal();
          break;
      }

      case IOLogicMessage::CLOSE_COMMAND:
      {
          TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "DPG server block [" << Name() << "] stopping " << mActivePlaylists << " active playlists");
            
          {
              ACE_GUARD(playInstLock_t, play_guard, mPlayInstLock);
              playInstMap_t::iterator iter = mPlayInstMap.begin(); 
              while (iter != mPlayInstMap.end())
              {
                  iter->second->ClearStoppedDelegate(); 
                  iter->second->Stop();
                  mSocketMgr->RemovePlaylist(iter->second.get());
                  iter = mPlayInstMap.erase(iter);
              }
    
              {
                  ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
                  mStats.activePlaylists = mActivePlaylists = 0;
              }
          }

          mSocketMgr->Reset();
          
         mSocketMgr.reset(new SocketManager(mIOReactor));


          // Signal that the close is complete
          mIOLogicInterface->Signal();
          break;
      }

      default:
          throw EPHXInternal("unknown IOLogicMessage");
    }
}

///////////////////////////////////////////////////////////////////////////////

void DpgServerBlock::HandleStopNotification(L4L7_ENGINE_NS::PlaylistInstance * pi)
{ 
    ACE_GUARD(playInstLock_t, play_guard, mPlayInstLock);
    if (mPlayInstMap.erase(pi->GetSerial()))
    {
        ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
        mStats.activePlaylists = --mActivePlaylists;
    }
    mSocketMgr->RemovePlaylist(pi);
}

bool DpgServerBlock::ServerSpawn(L4L7_ENGINE_NS::PlaylistInstance * piParent, int32_t fd, const ACE_INET_Addr& localAddr, const ACE_INET_Addr& remoteAddr)
{
    // FIXME -- HACK for now, always make a new server playlist

    // Correct method is to check "child" instances for one that will correspond to the given srcPort
    uint16_t srcPort = remoteAddr.get_port_number();

    L4L7_ENGINE_NS::ModifierBlock spawnedModifierBlock(*mModifierBlock);
    spawnedModifierBlock.SetCursor(TcpPortUtils::GetCursor(srcPort, piParent));

    // FIXME flowIdx = TcpPortUtils::GetFlowIdx(srcPort);
    
    L4L7_ENGINE_NS::PlaylistInstance * pi = mPlaylistEngine->ClonePlaylist(piParent, spawnedModifierBlock);
    pi->SetStoppedDelegate(boost::bind(&DpgServerBlock::HandleStopNotification, this, _1)); 

    mSocketMgr->AddChildPlaylist(pi, piParent, remoteAddr); 

    playInstSharedPtr_t playInst(pi, boost::bind(&L4L7_ENGINE_NS::PlaylistInstance::RemoveReference, _1));

    {
        ACE_GUARD_RETURN(stats_t::lock_t, stats_guard, mStats.Lock(), false);
        mStats.activePlaylists = ++mActivePlaylists;
        ++mStats.totalPlaylists;
    }

    {
        ACE_GUARD_RETURN(playInstLock_t, play_guard, mPlayInstLock, false);
        pi->SetSerial(++mLastSerial);
        mPlayInstMap[mLastSerial] = playInst;
    }

    // Start will register this playlist for connect callbacks at the proper 
    // local/remote address
    pi->Start();

    // Forward this connection to the right flow
    return mSocketMgr->Accept(pi, fd);
}
///////////////////////////////////////////////////////////////////////////////

void DpgServerBlock::PreStopPlaylists()
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
