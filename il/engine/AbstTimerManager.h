/// @file
/// @brief Timer manager abstract base class (to be implemented by engine user)
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ABST_TIMER_MANAGER_H_
#define _ABST_TIMER_MANAGER_H_

#include <boost/function.hpp>

#include <engine/EngineCommon.h>

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

class AbstTimerManager
{
  public:
    typedef uint32_t handle_t;
    typedef boost::function0<void> timerDelegate_t;

    virtual ~AbstTimerManager(){};
    virtual handle_t CreateTimer(timerDelegate_t delegate, uint32_t msDelay) = 0; 
    virtual void CancelTimer(handle_t) = 0;

    virtual uint64_t GetTimeOfDayMsec() const = 0;

    handle_t InvalidTimerHandle() { return handle_t(-1); }
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
