/// @file
/// @brief Load Scheduler header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_LOAD_SCHEDULER_H_
#define _L4L7_LOAD_SCHEDULER_H_

#include <ace/Event_Handler.h>
#include <ace/Time_Value.h>
#include <boost/utility.hpp>

#include <base/BaseCommon.h>

DECL_L4L7_BASE_NS

///////////////////////////////////////////////////////////////////////////////

// Forward declarations
class LoadProfile;
class LoadStrategy;

class LoadScheduler : public ACE_Event_Handler, boost::noncopyable
{
  public:
    LoadScheduler(LoadProfile& profile, LoadStrategy& strategy);
    ~LoadScheduler();

    /// @brief Return running status
    bool Running(void) const { return mTimerId != INVALID_TIMER_ID;}
    
    /// @brief Start load profile
    void Start(ACE_Reactor *reactor);

    /// @brief Stop load profile
    void Stop(void);

    void SetEnableDynamicLoad(bool enable) { mEnableDynamicLoad = enable; }    

  private:
    int handle_timeout(const ACE_Time_Value& tv, const void *act);

    static const long INVALID_TIMER_ID = -1;

    LoadProfile& mProfile;              ///< load profile that is being scheduled
    LoadStrategy& mStrategy;            ///< load strategy that is being invoked
    
    ACE_Time_Value mTimerSkew;          ///< reactor's timer skew value
    long mTimerId;                      ///< our ACE timer id
    bool mStarted;                      ///< true once scheduler as run for the first time
    ACE_Time_Value mStartTime;          ///< absolute time that scheduling was started
    int32_t mLastLoad;                  ///< last load value
    bool mEnableDynamicLoad;            ///< enable dynamic load
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_BASE_NS

#endif
