///
/// @file
/// @brief SIP User Agent (UA) Block State Async Notifier implementation
///
/// Copyright (c) 2007 by Spirent Communications Inc.
/// All Rights Reserved.
///
/// This software is confidential and proprietary to Spirent Communications Inc.
/// No part of this software may be reproduced, transmitted, disclosed or used
/// in violation of the Software License Agreement without the expressed
/// written consent of Spirent Communications Inc.
///
/// $File: //TestCenter/integration/content/traffic/l4l7/il/sipd/app/UserAgentRegStateNotifier.cpp $
/// $Revision: #1 $
/// \n<b>Last submission by:</b>
/// <ul>
/// <li>$Author: songkamongkol $</li>
/// <li>$DateTime: 2011/08/05 16:25:08 $</li>
/// <li>$Change: 572841 $</li>
/// </ul>
///

#include <algorithm>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include "SipApplicationProxy.h"
#include "UserAgentRegStateNotifier.h"

USING_APP_NS;

using namespace Sip_1_port_server;

///////////////////////////////////////////////////////////////////////////////

UserAgentRegStateNotifier::UserAgentRegStateNotifier(const MPS_Id mps_id,
                                                     const uint32_t asyncNotifierId,
                                                     const uint16_t port,
                                                     const uint32_t asyncNotifierIntervalMsec,
                                                     const UserAgentRegStateNotifierParams& params) 
    : Notifier<UserAgentRegStateNotifierParams>(mps_id, asyncNotifierId, port, asyncNotifierIntervalMsec),
      mApp(params.get<0>())
{
    RegisterNotifiers(params.get<1>());
}

///////////////////////////////////////////////////////////////////////////////

UserAgentRegStateNotifier::~UserAgentRegStateNotifier(void)
{
    UnregisterNotifiers();
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentRegStateNotifier::populateNotification(Message *pMsg)
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
    std::vector<regState_t> regStates;

    handles.reserve(numUpdates);
    regStates.reserve(numUpdates);

    BOOST_FOREACH(const updateMap_t::value_type& pair, updateMap)
    {
        handles.push_back(pair.first);
        regStates.push_back(pair.second);
    }
    
    create_req_SipUserAgentRegStateNotification(pMsg, getId(), handles, regStates);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentRegStateNotifier::updateConfig(const UserAgentRegStateNotifierParams& params)
{
    RegisterNotifiers(params.get<1>());
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentRegStateNotifier::RegisterNotifiers(const handleList_t& handles)
{
    mApp.RegisterUserAgentRegStateNotifier(getPort(), handles, boost::bind(&UserAgentRegStateNotifier::RegStateNotifierHandler, this, _1, _2, _3));
    mHandles.insert(handles.begin(), handles.end());
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentRegStateNotifier::UnregisterNotifiers(void) const
{
    mApp.UnregisterUserAgentRegStateNotifier(getPort(), handleList_t(mHandles.begin(), mHandles.end()));
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentRegStateNotifier::RegStateNotifierHandler(uint32_t handle, regState_t fromState, regState_t toState)
{
    ACE_GUARD(ACE_Thread_Mutex, guard, mUpdateMutex);
    mUpdateMap[handle] = toState;
}

///////////////////////////////////////////////////////////////////////////////
