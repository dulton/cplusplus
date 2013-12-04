/// @file
/// @brief Load Profile header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_LOAD_PROFILE_H_
#define _L4L7_LOAD_PROFILE_H_

#include <memory>
#include <utility>
#include <vector>

#include <ace/Time_Value.h>
#include <boost/function.hpp>
#include <boost/utility.hpp>
#include <Tr1Adapter.h>

#include <base/BaseCommon.h>

DECL_L4L7_BASE_NS

///////////////////////////////////////////////////////////////////////////////

// Forward declarations
class LoadPhase;

class LoadProfile : boost::noncopyable
{
  public:
    typedef boost::function1<void, bool> activeStateChangeDelegate_t;
    
    /// @brief Construct an empty load profile
    /// @note For this object to be functional you must add load phases
    explicit LoadProfile(const ClientLoadProfile_t& profile);

    /// @brief Construct a complete load profile
    explicit LoadProfile(const L4L7Base_1_port_server::ClientLoadConfig_t& config);
    
    ~LoadProfile();

    /// @brief Set the load profile's active state change delegate
    void SetActiveStateChangeDelegate(activeStateChangeDelegate_t delegate) { mActiveStateChangeDelegate = delegate; }
    
    /// @brief Return the load profile's active status
    bool Active(void) const { return mActive; }
    
    /// @brief Reset load phases
    void ResetLoadPhases(void);
    
    /// @brief Add a load phase to the profile
    /// @param phase #ClientLoadPhase_t configuration struct
    /// @param patternDesc Specific pattern descriptor
    void AddLoadPhase(const ClientLoadPhase_t& phase, const void *patternDesc);

    /// @brief Add a load phase to the profile
    /// @param Existing #LoadPhase object
    void AddLoadPhase(std::auto_ptr<LoadPhase> phase);

    /// @brief Return duration of the profile (all phases)
    /// @retval Duration of the profile
    const ACE_Time_Value& Duration(void) const;
    
    /// @brief Start the load profile at time zero
    void Start(void);

    /// @brief Evaluate the load at a given time
    /// @note The #LoadProject object's concept of time starts at from the start (time zero) and runs through the last load phase
    /// @param t Current time
    /// @param next Next eval time
    /// @retval Current load
    int32_t Eval(const ACE_Time_Value& t, ACE_Time_Value& next);

    /// @brief Stop the load profile
    void Stop(void);
    
  private:
    /// Implementation-private functor
    struct LoadPhaseTimeLessThanComp;
    
    typedef std::tr1::shared_ptr<LoadPhase> loadPhasePtr_t;
    typedef std::vector<loadPhasePtr_t> loadPhaseVec_t;
    typedef std::pair<ACE_Time_Value, size_t> loadPhaseTime_t;
    typedef std::vector<loadPhaseTime_t> loadPhaseTimeVec_t;
    typedef std::vector<loadPhaseTime_t>::iterator loadPhaseTimeIter_t;
    
    const ClientLoadProfile_t mProfile;                         ///< profile configuration
    loadPhaseVec_t mPhaseVec;                                   ///< phase objects
    loadPhaseTimeVec_t mPhaseTimeVec;                           ///< "run until" time of each phase

    bool mActive;                                               ///< when true, profile is active (being scheduled)
    activeStateChangeDelegate_t mActiveStateChangeDelegate;     ///< owner's active state change delegate
    loadPhaseTimeIter_t mCurrentPhase;                          ///< current phase iterator
    ACE_Time_Value mCurrentPhaseStart;                          ///< start time of current phase
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_BASE_NS

#endif
