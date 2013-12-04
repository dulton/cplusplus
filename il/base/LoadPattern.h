/// @file
/// @brief Load Pattern base class header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_LOAD_PATTERN_H_
#define _L4L7_LOAD_PATTERN_H_

#include <base/BaseCommon.h>

// Forward declarations (global)
class ACE_Time_Value;

DECL_L4L7_BASE_NS

///////////////////////////////////////////////////////////////////////////////

class LoadPattern
{
  public:
    virtual ~LoadPattern() { }
    
    /// @brief Return duration of the load pattern
    /// @retval Duration of the load pattern
    virtual const ACE_Time_Value& Duration(void) const = 0;
    
    /// @brief Set the Y-intercept value (height at t=0) for the pattern
    /// @param y0 Y-intercept value
    virtual void SetYIntercept(int32_t y0) = 0;

    /// @brief Evaluate the load at a given time
    /// @param t Current time
    /// @param next Next eval time
    /// @retval Current load
    virtual int32_t Eval(const ACE_Time_Value& t, ACE_Time_Value& next) = 0;

  protected:
    static const ACE_Time_Value MinQuantum(ClientLoadPhaseTimeUnitOptions timeUnits);
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_BASE_NS

#endif
