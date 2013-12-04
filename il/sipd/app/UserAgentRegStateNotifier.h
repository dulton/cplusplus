///
/// @file
/// @brief SIP User Agent Registration State Async Notifier header file
///
/// Copyright (c) 2007 by Spirent Communications Inc.
/// All Rights Reserved.
///
/// This software is confidential and proprietary to Spirent Communications Inc.
/// No part of this software may be reproduced, transmitted, disclosed or used
/// in violation of the Software License Agreement without the expressed
/// written consent of Spirent Communications Inc.
///
/// $File: //TestCenter/integration/content/traffic/l4l7/il/sipd/app/UserAgentRegStateNotifier.h $
/// $Revision: #1 $
/// \n<b>Last submission by:</b>
/// <ul>
/// <li>$Author: songkamongkol $</li>
/// <li>$DateTime: 2011/08/05 16:25:08 $</li>
/// <li>$Change: 572841 $</li>
/// </ul>
///

#ifndef _USER_AGENT_REG_STATE_NOTIFIER_H_
#define _USER_AGENT_REG_STATE_NOTIFIER_H_

#include <vector>

#include <ace/Thread_Mutex.h>
#include <boost/tuple/tuple.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <Notifier.h>

#include "VoIPCommon.h"

DECL_APP_NS

///////////////////////////////////////////////////////////////////////////////

// Forward declarations
class SipApplicationProxy;

typedef boost::tuple<SipApplicationProxy&, const std::vector<uint32_t>&> UserAgentRegStateNotifierParams;

class UserAgentRegStateNotifier : public Notifier<UserAgentRegStateNotifierParams>
{
  public:
    UserAgentRegStateNotifier(const MPS_Id mps_id,
                              const uint32_t asyncNotifierId,
                              const uint16_t port,
                              const uint32_t ayncNotifierIntervalMsec,
                              const UserAgentRegStateNotifierParams& params);

    ~UserAgentRegStateNotifier();
    
    bool populateNotification(Message *pMsg);

    bool updateConfig(const UserAgentRegStateNotifierParams& params);

  private:
    typedef std::vector<uint32_t> handleList_t;
    typedef boost::unordered_set<uint32_t> handleSet_t;
    typedef Sip_1_port_server::EnumSipUaRegState regState_t;
    typedef boost::unordered_map<uint32_t, regState_t> updateMap_t;

    void RegisterNotifiers(const handleList_t& handles);
    void UnregisterNotifiers(void) const;
    void RegStateNotifierHandler(uint32_t handle, regState_t fromState, regState_t toState);
    
    SipApplicationProxy& mApp;
    handleSet_t mHandles;

    ACE_Thread_Mutex mUpdateMutex;
    updateMap_t mUpdateMap;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_APP_NS

#endif
