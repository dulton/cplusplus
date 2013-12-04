/// @file
/// @brief Flow engine header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _FLOW_ENGINE_H_
#define _FLOW_ENGINE_H_

#include <map>
#include <vector>

#include <engine/EngineCommon.h>
#include <engine/FlowConfig.h>
#include <engine/AbstTimerManager.h>

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

class AbstFlowConfigProxy;
class PlaylistInstance;

class FlowEngine
{
  public:
    // public typedefs
    typedef FlowConfig Config_t;
    typedef FlowConfig::Packet Packet_t;

    FlowEngine(AbstTimerManager& timerMgr) :
        mTimerMgr(timerMgr) {}

    ~FlowEngine() {}

    // Config -- should happen when the playlist is stopped
    
    void UpdateFlow(handle_t handle, const AbstFlowConfigProxy& config);

    void DeleteFlow(handle_t handle);

    const Config_t* GetFlow(handle_t handle) const;

    // Accessors -- for use only by FlowInstance
    
    AbstTimerManager& GetTimerMgr() { return mTimerMgr; }

  protected:

  private:
    typedef std::map<handle_t, Config_t> ConfigMap_t;
    typedef ConfigMap_t::iterator MapIter_t;
    typedef ConfigMap_t::const_iterator MapConstIter_t;

    typedef std::map<uint16_t, FlowConfig::LoopInfo>::iterator LoopMapIter_t;

    ConfigMap_t mConfigMap;

    AbstTimerManager& mTimerMgr;

    void LinkLoopInfoToPkts(FlowConfig& flow);

    void AttackInitialization(FlowConfig& flow);
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
