/// @file
/// @brief Load Phase header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_LOAD_PHASE_H_
#define _L4L7_LOAD_PHASE_H_

#include <utility>

#include <ace/Time_Value.h>
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include <base/BaseCommon.h>
#include <base/LoadPattern.h>

DECL_L4L7_BASE_NS

///////////////////////////////////////////////////////////////////////////////

class LoadPhase : boost::noncopyable
{
  public:
    LoadPhase(const ClientLoadPhase_t& phase, const void *patternDesc);
    ~LoadPhase();

    /// @brief Return duration of the load phase
    /// @retval Duration of the load phase
    const ACE_Time_Value& Duration(void) const { return mPattern->Duration(); }
    
    /// @brief Start the load phase at time zero
    /// @param y0 Y-intercept value
    void SetYIntercept(int32_t y0) { mPattern->SetYIntercept(y0); }

    /// @brief Evaluate the load at a given time
    /// @note The #LoadPhase object's concept of time starts at the beginning of the phase (time zero) and runs through the end of the phase
    /// @param t Current time
    /// @param next Next eval time as determined by load pattern
    /// @retval Current load
    int32_t Eval(const ACE_Time_Value& t, ACE_Time_Value& next) { return std::max(0, mPattern->Eval(t, next)); }
    
  private:
    static LoadPattern *MakePatternImpl(LoadPatternTypes patternType, const void *patternDesc, ClientLoadPhaseTimeUnitOptions timeUnits);

    boost::scoped_ptr<LoadPattern> mPattern;    ///< #LoadPattern implementation
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_BASE_NS

#endif
