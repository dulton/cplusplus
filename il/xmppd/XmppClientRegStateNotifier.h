///
/// @file
/// @brief Xmpp Client Registration State Async Notifier header file
///
/// Copyright (c) 2007 by Spirent Communications Inc.
/// All Rights Reserved.
///
/// This software is confidential and proprietary to Spirent Communications Inc.
/// No part of this software may be reproduced, transmitted, disclosed or used
/// in violation of the Software License Agreement without the expressed
/// written consent of Spirent Communications Inc.
///


#ifndef _XMPP_CLIENT_REG_STATE_NOTIFIER_H_
#define _XMPP_CLIENT_REG_STATE_NOTIFIER_H_

#include <vector>

#include <ace/Thread_Mutex.h>
#include <boost/tuple/tuple.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <Notifier.h>

#include "XmppCommon.h"

DECL_XMPP_NS

///////////////////////////////////////////////////////////////////////////////

// Forward declarations
class XmppApplicationProxy;

typedef boost::tuple<XmppApplicationProxy&, const std::vector<uint32_t>&> XmppClientRegStateNotifierParams;

class XmppClientRegStateNotifier : public Notifier<XmppClientRegStateNotifierParams>
{
  public:
    XmppClientRegStateNotifier(const MPS_Id mps_id,
                              const uint32_t asyncNotifierId,
                              const uint16_t port,
                              const uint32_t ayncNotifierIntervalMsec,
                              const XmppClientRegStateNotifierParams& params);

    ~XmppClientRegStateNotifier();
    
    bool populateNotification(Message *pMsg);

    bool updateConfig(const XmppClientRegStateNotifierParams& params);

  private:
    typedef std::vector<uint32_t> handleList_t;
    typedef boost::unordered_set<uint32_t> handleSet_t;
    typedef XmppvJ_1_port_server::EnumXmppvJRegistrationState regState_t;
    typedef boost::unordered_map<uint32_t, regState_t> updateMap_t;

    void RegisterNotifiers(const handleList_t& handles);
    void UnregisterNotifiers(void) const;
    void RegStateNotifierHandler(uint32_t handle, regState_t fromState, regState_t toState);
    
    XmppApplicationProxy& mApp;
    handleSet_t mHandles;

    ACE_Thread_Mutex mUpdateMutex;
    updateMap_t mUpdateMap;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_XMPP_NS

#endif
