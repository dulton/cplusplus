/// @file
/// @brief Mock flow config proxy -- wraps a flow config
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _MOCK_FLOW_CONFIG_PROXY_H_
#define _MOCK_FLOW_CONFIG_PROXY_H_

#include "AbstFlowConfigProxy.h"
#include "FlowConfig.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class MockFlowConfigProxy : public AbstFlowConfigProxy
{
  public:
    MockFlowConfigProxy(const FlowConfig& config) :
        mConfig(config)
    {}

    virtual ~MockFlowConfigProxy() {}

    void CopyTo(FlowConfig& config) const
    {
        config = mConfig;
    }

  private:
    FlowConfig mConfig;
};

///////////////////////////////////////////////////////////////////////////////

#endif
