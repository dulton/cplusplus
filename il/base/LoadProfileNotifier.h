///
/// @file
/// @brief Load Profile Async Notifier header file
///
/// Copyright (c) 2008 by Spirent Communications Inc.
/// All Rights Reserved.
///
/// This software is confidential and proprietary to Spirent Communications Inc.
/// No part of this software may be reproduced, transmitted, disclosed or used
/// in violation of the Software License Agreement without the expressed
/// written consent of Spirent Communications Inc.
///
/// $File: //TestCenter/integration/content/traffic/l4l7/il/base/LoadProfileNotifier.h $
/// $Revision: #1 $
/// \n<b>Last submission by:</b>
/// <ul>
/// <li>$Author: songkamongkol $</li>
/// <li>$DateTime: 2011/08/05 16:25:08 $</li>
/// <li>$Change: 572841 $</li>
/// </ul>
///

#ifndef _L4L7_LOAD_PROFILE_STATE_NOTIFIER_H_
#define _L4L7_LOAD_PROFILE_STATE_NOTIFIER_H_

#include <vector>

#include <ace/Thread_Mutex.h>
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <Notifier.h>

#include <base/BaseCommon.h>

DECL_L4L7_BASE_NS

///////////////////////////////////////////////////////////////////////////////

struct LoadProfileNotifierParams
{
    typedef std::vector<uint32_t> handleList_t;
    typedef boost::function2<void, uint32_t, bool> loadProfileNotifier_t;
    typedef boost::function2<void, const handleList_t&, loadProfileNotifier_t> registerNotifiersDelegate_t;
    typedef boost::function1<void, const handleList_t&> unregisterNotifiersDelegate_t;

    LoadProfileNotifierParams(const std::string& theMessageSet,
                              const registerNotifiersDelegate_t& theRegisterNotifiersDelegate,
                              const unregisterNotifiersDelegate_t& theUnregisterNotifiersDelegate,
                              const handleList_t& theHandles)
        : messageSet(theMessageSet),
          registerNotifiersDelegate(theRegisterNotifiersDelegate),
          unregisterNotifiersDelegate(theUnregisterNotifiersDelegate),
          handles(theHandles)
    {
    }

    const std::string messageSet;
    const registerNotifiersDelegate_t registerNotifiersDelegate;
    const unregisterNotifiersDelegate_t unregisterNotifiersDelegate;
    const handleList_t& handles;
};

class LoadProfileNotifier : public Notifier<LoadProfileNotifierParams>
{
  public:
    LoadProfileNotifier(const MPS_Id mps_id, const uint32_t asyncNotifierId, const uint16_t port, const uint32_t ayncNotifierIntervalMsec,
                        const LoadProfileNotifierParams& params);

    ~LoadProfileNotifier();
    
    bool populateNotification(Message *pMsg);
    bool updateConfig(const LoadProfileNotifierParams& params);

    static const uint32_t DEFAULT_NOTIFIER_INTERVAL_MSEC;
    
  private:
    typedef std::vector<uint32_t> handleList_t;
    typedef boost::unordered_set<uint32_t> handleSet_t;
    typedef boost::unordered_map<uint32_t, bool> updateMap_t;

    void RegisterNotifiers(const handleList_t& handles);
    void UnregisterNotifiers(void) const;
    void ActiveStateNotifierHandler(uint32_t handle, bool active);

    std::string mMessageName;
    const LoadProfileNotifierParams::registerNotifiersDelegate_t mRegisterNotifiersDelegate;
    const LoadProfileNotifierParams::unregisterNotifiersDelegate_t mUnregisterNotifiersDelegate; 
    handleSet_t mHandles;
   
    ACE_Thread_Mutex mUpdateMutex;
    updateMap_t mUpdateMap;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_BASE_NS

#endif
