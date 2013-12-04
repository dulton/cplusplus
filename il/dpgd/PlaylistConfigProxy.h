/// @file
/// @brief Playlist config proxy -- wraps the IDL playlist config for the engine
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///
///  Note that this proxy is backed by the IDL playlistConfig, which must continue
///  to exist as long as the proxy is in use.

#ifndef _PLAYLIST_CONFIG_PROXY_H_
#define _PLAYLIST_CONFIG_PROXY_H_

#include "DpgCommon.h"

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

class PlaylistConfigProxy 
{
  public:
    // Convenience typedefs
    typedef Dpg_1_port_server::DpgPlaylistConfig_t playlistConfig_t;

    PlaylistConfigProxy(const playlistConfig_t&);
    
    virtual ~PlaylistConfigProxy() {}

    void CopyTo(L4L7_ENGINE_NS::PlaylistConfig& config) const;

  private:
    typedef DpgIf_1_port_server::DpgStreamReference_t streamRef_t;
    typedef DpgIf_1_port_server::DpgAttackReference_t attackRef_t;
    typedef DpgIf_1_port_server::DpgLoopConfig_t loopConfig_t;
    typedef DpgIf_1_port_server::DpgVariableValue_t varConfig_t;
    typedef DpgIf_1_port_server::rangeModifier_t rangeMod_t;
    typedef DpgIf_1_port_server::tableModifier_t tableMod_t;
    typedef DpgIf_1_port_server::modifierLongData_t row_t;
    typedef DpgIf_1_port_server::modLinkDesc_t linkDesc_t;
    void ValidateFlows();
    void ValidateLoops();
    void ValidateVars();
    void ValidateMods();
    void ValidateLinks();

    const playlistConfig_t& mConfig;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
