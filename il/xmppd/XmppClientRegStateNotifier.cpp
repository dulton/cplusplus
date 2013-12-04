///
/// @file
/// @brief Xmpp Client Block State Async Notifier implementation
///
/// Copyright (c) 2007 by Spirent Communications Inc.
/// All Rights Reserved.
///
/// This software is confidential and proprietary to Spirent Communications Inc.
/// No part of this software may be reproduced, transmitted, disclosed or used
/// in violation of the Software License Agreement without the expressed
/// written consent of Spirent Communications Inc.
///

#include <algorithm>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include "XmppApplicationProxy.h"
#include "XmppClientRegStateNotifier.h"

USING_XMPP_NS;
using namespace xmppvj_1_port_server;
using namespace XmppvJ_1_port_server;
///////////////////////////////////////////////////////////////////////////////

XmppClientRegStateNotifier::XmppClientRegStateNotifier(const MPS_Id mps_id,
                                                     const uint32_t asyncNotifierId,
                                                     const uint16_t port,
                                                     const uint32_t asyncNotifierIntervalMsec,
                                                     const XmppClientRegStateNotifierParams& params) 
    : Notifier<XmppClientRegStateNotifierParams>(mps_id, asyncNotifierId, port, asyncNotifierIntervalMsec),
      mApp(params.get<0>())
{
    RegisterNotifiers(params.get<1>());
}

///////////////////////////////////////////////////////////////////////////////

XmppClientRegStateNotifier::~XmppClientRegStateNotifier(void)
{
    UnregisterNotifiers();
}

///////////////////////////////////////////////////////////////////////////////

bool XmppClientRegStateNotifier::populateNotification(Message *pMsg)
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
    
    create_req_XmppvJClientRegStateNotification(pMsg, getId(), handles, regStates);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool XmppClientRegStateNotifier::updateConfig(const XmppClientRegStateNotifierParams& params)
{
    RegisterNotifiers(params.get<1>());
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientRegStateNotifier::RegisterNotifiers(const handleList_t& handles)
{
    mApp.RegisterXmppClientRegStateNotifier(getPort(), handles, boost::bind(&XmppClientRegStateNotifier::RegStateNotifierHandler, this, _1, _2, _3));
    mHandles.insert(handles.begin(), handles.end());
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientRegStateNotifier::UnregisterNotifiers(void) const
{
    mApp.UnregisterXmppClientRegStateNotifier(getPort(), handleList_t(mHandles.begin(), mHandles.end()));
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientRegStateNotifier::RegStateNotifierHandler(uint32_t handle, regState_t fromState, regState_t toState)
{
    ACE_GUARD(ACE_Thread_Mutex, guard, mUpdateMutex);
    mUpdateMap[handle] = toState;
}

///////////////////////////////////////////////////////////////////////////////
