/// @file
/// @brief Remote address utilities
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <boost/foreach.hpp>

#include <ildaemon/RemoteInterfaceEnumerator.h>

#include <engine/PlaylistConfig.h>
#include <engine/PlaylistEngine.h>

#include "DpgdLog.h"

#include "RemAddrUtils.h"


USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

bool RemAddrUtils::NeedsAddrSet(const L4L7_ENGINE_NS::PlaylistEngine& engine, const serverConfig_t& serverConfig)
{
    portSet_t superSet, portSet;

    BOOST_FOREACH(const DpgIf_1_port_server::DpgPlaylistReference_t& playlistRef, serverConfig.Playlists)
    {
        const L4L7_ENGINE_NS::PlaylistConfig * playlistConfig = engine.GetPlaylist(playlistRef.Handle);
        if (!playlistConfig)
        {
            std::ostringstream err;
            err << "Unable to find playlist with handle " << playlistRef.Handle;
            TC_LOG_ERR(0, err.str());
            throw EPHXBadConfig(err.str());
        }

        GetServerPortSet(portSet, *playlistConfig);

        if (PortsOverlap(superSet, portSet))
            return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

void RemAddrUtils::GetAddrSet(addrSet_t& addrSet, const stack_t& stack)
{
    IL_DAEMON_LIB_NS::RemoteInterfaceEnumerator ifEnum(0, stack); 
    ACE_INET_Addr addr;

    addrSet.clear();

    const size_t numAddrs = ifEnum.TotalCount();
    for (size_t idx = 0; idx < numAddrs; ++idx)
    {
        ifEnum.GetInterface(addr);
        ifEnum.Next();
        addrSet.insert(addr);
    }
}

///////////////////////////////////////////////////////////////////////////////

void RemAddrUtils::GetServerPortSet(portSet_t& portSet, const L4L7_ENGINE_NS::PlaylistConfig& playlistConfig)
{
    portSet.clear();

    BOOST_FOREACH(const L4L7_ENGINE_NS::PlaylistConfig::Flow& flow, playlistConfig.flows)
    {
        if (!flow.reversed)
        {
            portSet.insert(flow.serverPort);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool RemAddrUtils::PortsOverlap(portSet_t& superSet, const portSet_t& newSet)
{
    size_t superSize = superSet.size();
    superSet.insert(newSet.begin(), newSet.end());

    // if there are no overlaps, the new set should be exactly the size 
    // of the old two sets combined 
    return superSet.size() != superSize + newSet.size();
}

///////////////////////////////////////////////////////////////////////////////

