/// @file
/// @brief Dpg timer manager header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _DPG_TIMER_MANAGER_H_
#define _DPG_TIMER_MANAGER_H_

#include <set>
#include <ace/Event_Handler.h>
#include <ace/Thread_Mutex.h>

#include <engine/AbstTimerManager.h>
#include "DpgCommon.h"

class ACE_Reactor;
class TestTimerManager;

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

class TimerManager : public L4L7_ENGINE_NS::AbstTimerManager, public ACE_Event_Handler
{
  public:
    TimerManager(ACE_Reactor * reactor);
    ~TimerManager();

    handle_t CreateTimer(timerDelegate_t delegate, uint32_t msDelay); 
 
    void CancelTimer(handle_t);

    uint64_t GetTimeOfDayMsec() const;

  private:
    // from ACE_Event_Handler 
    int handle_timeout (const ACE_Time_Value &current_time, const void *act);
 
    struct DelegateArgPair
    {
        DelegateArgPair(timerDelegate_t d) : 
            delegate(d), 
            timerId(INVALID_TIMER_ID),
            fired(false)
        {
        }
        timerDelegate_t delegate;
        handle_t timerId;    
        bool fired;
    };

    typedef std::set<handle_t> handleSet_t;
    handleSet_t mHandleSet;

    typedef ACE_Thread_Mutex lock_t;
    lock_t mLock;

    static const handle_t INVALID_TIMER_ID = handle_t(-1);

    friend class TestTimerManager;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
