/// @file
/// @brief Mock playlist engine
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _MOCK_PLAYLIST_ENGINE_PROXY_H_
#define _MOCK_PLAYLIST_ENGINE_PROXY_H_

#include <list>
#include <boost/foreach.hpp>

#include <engine/PlaylistEngine.h>
#include "MockPlaylistInstance.h"

///////////////////////////////////////////////////////////////////////////////

class MockPlaylistEngine : public L4L7_ENGINE_NS::PlaylistEngine
{
  public:
    MockPlaylistEngine(bool clearOnExit = false) :
        L4L7_ENGINE_NS::PlaylistEngine(*(L4L7_ENGINE_NS::FlowEngine*)0, *(L4L7_ENGINE_NS::AbstTimerManager*)0),
        mClearOnExit(clearOnExit)
    {
    }

    ~MockPlaylistEngine()
    {
        if (mClearOnExit) Clear();
    }

    L4L7_ENGINE_NS::PlaylistInstance * MakePlaylist(L4L7_ENGINE_NS::handle_t handle, const L4L7_ENGINE_NS::ModifierBlock& modBlock, L4L7_ENGINE_NS::AbstSocketManager& socketMgr, bool client)
    {
        PlaylistInfo info(handle, modBlock, socketMgr, client, 
            new MockPlaylistInstance(*this, socketMgr, mEmptyConfig, client));
        mPlaylists.push_back(info);

        return info.pi;
    }

    L4L7_ENGINE_NS::PlaylistInstance * ClonePlaylist(const L4L7_ENGINE_NS::PlaylistInstance * parentPi, const L4L7_ENGINE_NS::ModifierBlock& modBlock)
    {
        const MockPlaylistInstance * parentMpi = static_cast<const MockPlaylistInstance *>(parentPi);

        PlaylistInfoList_t::const_iterator infoIter = mPlaylists.begin();
        while (infoIter != mPlaylists.end())
        {
            if (infoIter->pi == parentMpi)
                break;
            ++infoIter;
        }

        if (infoIter == mPlaylists.end())
        {
            return 0; // something very wrong, just barf
        }

        // Make a new playlist using the information from the parent
        PlaylistInfo info(infoIter->handle, modBlock, infoIter->socketMgr, infoIter->client, new MockPlaylistInstance(*this, infoIter->socketMgr, mEmptyConfig, infoIter->client));
        mPlaylists.push_back(info); 

        return info.pi;
    }

  protected:
    void ValidateConfig(const Config_t& config) const {}

  public:
    // Mock methods
    struct PlaylistInfo
    {
        PlaylistInfo(L4L7_ENGINE_NS::handle_t inHandle, const L4L7_ENGINE_NS::ModifierBlock& inModBlock, L4L7_ENGINE_NS::AbstSocketManager& inSocketMgr, bool inClient, MockPlaylistInstance * inPi) :
            handle(inHandle), modBlock(inModBlock), socketMgr(inSocketMgr), client(inClient), pi(inPi)
        {}

        L4L7_ENGINE_NS::handle_t handle;
        const L4L7_ENGINE_NS::ModifierBlock& modBlock;
        L4L7_ENGINE_NS::AbstSocketManager& socketMgr;
        bool client;
        MockPlaylistInstance * pi;
    };

    typedef std::list<PlaylistInfo> PlaylistInfoList_t;

    PlaylistInfoList_t& GetPlaylists() { return mPlaylists; }

    void Clear()
    {
        BOOST_FOREACH(PlaylistInfo& info, mPlaylists)
        {
            delete info.pi;
        }
        mPlaylists.clear();
    }


  private:
    PlaylistInfoList_t mPlaylists;
    L4L7_ENGINE_NS::PlaylistConfig mEmptyConfig;
    bool mClearOnExit;

};

///////////////////////////////////////////////////////////////////////////////

#endif

