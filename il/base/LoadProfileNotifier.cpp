///
/// @file
/// @brief Load Profile Async Notifier implementation
///
/// Copyright (c) 2008 by Spirent Communications Inc.
/// All Rights Reserved.
///
/// This software is confidential and proprietary to Spirent Communications Inc.
/// No part of this software may be reproduced, transmitted, disclosed or used
/// in violation of the Software License Agreement without the expressed
/// written consent of Spirent Communications Inc.
///
/// $File: //TestCenter/integration/content/traffic/l4l7/il/base/LoadProfileNotifier.cpp $
/// $Revision: #1 $
/// \n<b>Last submission by:</b>
/// <ul>
/// <li>$Author: songkamongkol $</li>
/// <li>$DateTime: 2011/08/05 16:25:08 $</li>
/// <li>$Change: 572841 $</li>
/// </ul>
///

#include <sstream>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include "LoadProfileNotifier.h"

USING_L4L7_BASE_NS;

///////////////////////////////////////////////////////////////////////////////

const uint32_t LoadProfileNotifier::DEFAULT_NOTIFIER_INTERVAL_MSEC = 1000;

///////////////////////////////////////////////////////////////////////////////

LoadProfileNotifier::LoadProfileNotifier(const MPS_Id mps_id, const uint32_t asyncNotifierId, const uint16_t port, const uint32_t asyncNotifierIntervalMsec,
                                         const LoadProfileNotifierParams& params) 
    : Notifier<LoadProfileNotifierParams>(mps_id, asyncNotifierId, port, asyncNotifierIntervalMsec),
      mRegisterNotifiersDelegate(params.registerNotifiersDelegate),
      mUnregisterNotifiersDelegate(params.unregisterNotifiersDelegate)
{
    std::ostringstream oss;
    oss << params.messageSet << ".LoadProfileNotification";
    mMessageName = oss.str();
    
    RegisterNotifiers(params.handles);
}

///////////////////////////////////////////////////////////////////////////////

LoadProfileNotifier::~LoadProfileNotifier(void)
{
    UnregisterNotifiers();
}

///////////////////////////////////////////////////////////////////////////////

bool LoadProfileNotifier::populateNotification(Message *pMsg)
{
    updateMap_t updateMap;

    // Acquire mutex and swap update maps
    {
        ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, mUpdateMutex, false);
        mUpdateMap.swap(updateMap);
    }
    
    if (updateMap.empty())
        return false;

    const size_t numUpdates = updateMap.size();
    
    std::vector<uint32_t> handles;
    std::vector<bool> activeVec;

    handles.reserve(numUpdates);
    activeVec.reserve(numUpdates);

    BOOST_FOREACH(const updateMap_t::value_type& pair, updateMap)
    {
        handles.push_back(pair.first);
        activeVec.push_back(pair.second);
    }
    
    L4L7Base_1_port_server::create_req_LoadProfileNotification(pMsg, handles, activeVec);
    pMsg->set_request_name(mMessageName);
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool LoadProfileNotifier::updateConfig(const LoadProfileNotifierParams& params)
{
    RegisterNotifiers(params.handles);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void LoadProfileNotifier::RegisterNotifiers(const handleList_t& handles)
{
    mRegisterNotifiersDelegate(handles, boost::bind(&LoadProfileNotifier::ActiveStateNotifierHandler, this, _1, _2));
    mHandles.insert(handles.begin(), handles.end());
}

///////////////////////////////////////////////////////////////////////////////

void LoadProfileNotifier::UnregisterNotifiers(void) const
{
    mUnregisterNotifiersDelegate(handleList_t(mHandles.begin(), mHandles.end()));
}

///////////////////////////////////////////////////////////////////////////////

void LoadProfileNotifier::ActiveStateNotifierHandler(uint32_t handle, bool active)
{
    ACE_GUARD(ACE_Thread_Mutex, guard, mUpdateMutex);
    mUpdateMap[handle] = active;
}

///////////////////////////////////////////////////////////////////////////////
