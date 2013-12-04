/// @file
/// @brief Abstract flow config proxy - represents a flow config with getters()
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ABST_FLOW_CONFIG_PROXY_H_
#define _ABST_FLOW_CONFIG_PROXY_H_

#include <engine/EngineCommon.h>

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

struct FlowConfig;

class AbstFlowConfigProxy
{
  public:
    virtual ~AbstFlowConfigProxy(){};
    virtual void CopyTo(FlowConfig& config) const = 0;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
