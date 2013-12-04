/// @file
/// @brief Flow config proxy -- wraps the IDL flow config for the engine
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///
///  Note that this proxy is backed by the IDL flowConfig, which must continue
///  to exist as long as the proxy is in use.

#ifndef _FLOW_CONFIG_PROXY_H_
#define _FLOW_CONFIG_PROXY_H_

#include "DpgCommon.h"
#include <engine/AbstFlowConfigProxy.h>

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

class FlowConfigProxy : public L4L7_ENGINE_NS::AbstFlowConfigProxy
{
  public:
    // Convenience typedefs
    typedef DpgIf_1_port_server::DpgFlowConfig_t flowConfig_t;

    FlowConfigProxy(const flowConfig_t&);
    
    virtual ~FlowConfigProxy() {}

    void CopyTo(L4L7_ENGINE_NS::FlowConfig& config) const;

  private:
    typedef DpgIf_1_port_server::DpgVariableDef_t varConfig_t;
    typedef DpgIf_1_port_server::DpgLoopConfig_t loopConfig_t;
    void ValidateLoops();
    void ValidateVars();

    const flowConfig_t& mConfig;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
