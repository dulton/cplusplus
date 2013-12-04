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
///  Playlists sometimes share the same server port and need to be muxed based on the remote address
///

#ifndef _REM_ADDR_UTILS_H_
#define _REM_ADDR_UTILS_H_

#include <ace/INET_Addr.h>
#include <set>

#include "DpgCommon.h"

namespace L4L7_ENGINE_NS
{
    struct PlaylistConfig;
    class PlaylistEngine;
};

class TestRemAddrUtils;

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

class RemAddrUtils
{
public:
    typedef DpgIf_1_port_server::DpgServerConfig_t serverConfig_t;
    typedef DpgIf_1_port_server::NetworkInterfaceStack_t stack_t;
    typedef std::set<ACE_INET_Addr> addrSet_t;

    static bool NeedsAddrSet(const L4L7_ENGINE_NS::PlaylistEngine& engine, const serverConfig_t& serverConfig);

    static void GetAddrSet(addrSet_t& addrSet, const stack_t& stack);

private:
    typedef std::set<uint16_t> portSet_t;

    static void GetServerPortSet(portSet_t& portSet, const L4L7_ENGINE_NS::PlaylistConfig& playlistConfig);

    // true if any ports in the new set overlap with any ports in the super set
    // side-effect of adding all ports in the new set to the super set
    static bool PortsOverlap(portSet_t& superSet, const portSet_t& newSet);

    friend class TestRemAddrUtils;
};


///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
