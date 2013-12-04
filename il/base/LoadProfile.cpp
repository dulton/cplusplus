/// @file
/// @brief Load Profile implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <algorithm>
#include <functional>

#include <boost/foreach.hpp>
#include <phxexception/PHXException.h>
#include <phxlog/phxlog.h>

#include "LoadPhase.h"
#include "LoadProfile.h"

USING_L4L7_BASE_NS;

///////////////////////////////////////////////////////////////////////////////

struct LoadProfile::LoadPhaseTimeLessThanComp : public std::binary_function<loadPhaseTime_t, loadPhaseTime_t, bool>
{
    bool operator()(const loadPhaseTime_t& lhs, const loadPhaseTime_t& rhs) const { return lhs.first < rhs.first; }
};
    
///////////////////////////////////////////////////////////////////////////////

LoadProfile::LoadProfile(const ClientLoadProfile_t& profile)
    : mProfile(profile),
      mActive(false),
      mCurrentPhase(mPhaseTimeVec.end())
{
}

///////////////////////////////////////////////////////////////////////////////

LoadProfile::LoadProfile(const L4L7Base_1_port_server::ClientLoadConfig_t& config)
    : mProfile(config.LoadProfile),
      mActive(false),
      mCurrentPhase(mPhaseTimeVec.end())
{
    BOOST_FOREACH(const L4L7Base_1_port_server::ClientLoadPhaseConfig_t phase, config.LoadPhases)
    {
        const void *patternDesc;
        const LoadPatternTypes patternType = static_cast<const LoadPatternTypes>(phase.LoadPhase.LoadPattern);
        
        switch (patternType)
        {
          case LoadPatternTypes_STAIR:
              patternDesc = &phase.StairDescriptor;
              break;

          case LoadPatternTypes_FLAT:
              patternDesc = &phase.FlatDescriptor;
              break;
              
          case LoadPatternTypes_BURST:
              patternDesc = &phase.BurstDescriptor;
              break;
              
          case LoadPatternTypes_SINUSOID:
              patternDesc = &phase.SinusoidDescriptor;
              break;
              
          case LoadPatternTypes_RANDOM:
              patternDesc = &phase.RandomDescriptor;
              break;
              
          case LoadPatternTypes_SAWTOOTH:
              patternDesc = &phase.SawToothDescriptor;
              break;

          default:
          {
              std::ostringstream oss;
              oss << "[LoadProfile ctor] Cannot create load profile due to unknown load pattern type (" << tms_enum_to_string(patternType, em_LoadPatternTypes) << ")";
              const std::string err(oss.str());
              TC_LOG_ERR(0, err);
              throw EPHXBadConfig(err);
          }
        }
        
        std::auto_ptr<LoadPhase> phasePtr(new LoadPhase(phase.LoadPhase, patternDesc));
        AddLoadPhase(phasePtr);
    }
}

///////////////////////////////////////////////////////////////////////////////

LoadProfile::~LoadProfile()
{
}

///////////////////////////////////////////////////////////////////////////////

void LoadProfile::ResetLoadPhases(void)
{
    if (mActive)
        throw EPHXInternal("LoadProfile::ResetLoadPhases");
    
    mPhaseVec.clear();
}

///////////////////////////////////////////////////////////////////////////////

void LoadProfile::AddLoadPhase(const ClientLoadPhase_t& phase, const void *patternDesc)
{
    if (mActive)
        throw EPHXInternal("LoadProfile::AddLoadPhase");

    std::auto_ptr<LoadPhase> phasePtr(new LoadPhase(phase, patternDesc));
    AddLoadPhase(phasePtr);
}

///////////////////////////////////////////////////////////////////////////////

void LoadProfile::AddLoadPhase(std::auto_ptr<LoadPhase> phase)
{
    if (mActive)
        throw EPHXInternal("LoadProfile::AddLoadPhase");

    loadPhasePtr_t phasePtr(phase.release());
    const ACE_Time_Value phaseDuration(phasePtr->Duration());

    if (phaseDuration)
    {
        const size_t phaseNum(mPhaseVec.size());
        const ACE_Time_Value& lastPhaseTime(phaseNum ? mPhaseTimeVec.back().first : ACE_Time_Value::zero);
        
        mPhaseVec.push_back(phasePtr);
        mPhaseTimeVec.push_back(loadPhaseTime_t(lastPhaseTime + phaseDuration, phaseNum));
    }
}

///////////////////////////////////////////////////////////////////////////////

const ACE_Time_Value& LoadProfile::Duration(void) const
{
    if (mPhaseVec.empty())
        throw EPHXBadConfig("LoadProfile::Duration");
    
    return mPhaseTimeVec.back().first;
}

///////////////////////////////////////////////////////////////////////////////

void LoadProfile::Start(void)
{
    if (mPhaseVec.empty())
        throw EPHXBadConfig("LoadProfile::Start");

    const bool stateChange = !mActive;
    mActive = true;
    mCurrentPhase = mPhaseTimeVec.begin();
    mCurrentPhaseStart = ACE_Time_Value::zero;
    mPhaseVec.front()->SetYIntercept(0);
    if (mProfile.RandomizationSeed) srand(mProfile.RandomizationSeed);
    if (stateChange && !mActiveStateChangeDelegate.empty())
        mActiveStateChangeDelegate(mActive);
}

///////////////////////////////////////////////////////////////////////////////

int32_t LoadProfile::Eval(const ACE_Time_Value& t, ACE_Time_Value& next)
{
    // upper_bound finds the next phase whose "run until" time is greater than "t"
    loadPhaseTimeIter_t end = mPhaseTimeVec.end();
    loadPhaseTimeIter_t nextPhase = upper_bound(mCurrentPhase, end, loadPhaseTime_t(t, 0), LoadPhaseTimeLessThanComp());

    // If "t" has advanced to or beyond the end of the current phase, we need to advance to the next phase
    while ((t >= mCurrentPhase->first) && (mCurrentPhase < nextPhase))
    {
        // Evaluate the ending load value in the current phase
        loadPhasePtr_t& currentPhasePtr(mPhaseVec.at(mCurrentPhase->second));

        // This is the last phase?
        if ((mCurrentPhase->second + 1) == mPhaseVec.size())
        {
            // We've reached the end of the profile
            next = ACE_Time_Value::zero;

            mActive = false;
            if (!mActiveStateChangeDelegate.empty())
                mActiveStateChangeDelegate(mActive);
            
            return 0;
        }
        else
        {
            // Advance to the next phase
            mCurrentPhaseStart = mCurrentPhase->first;
            ++mCurrentPhase;
        }

        // Set Y-intercept value of the new phase to the end load value of previous phase
        const int32_t endLoad = currentPhasePtr->Eval(currentPhasePtr->Duration(), next);
        mPhaseVec.at(mCurrentPhase->second)->SetYIntercept(endLoad);
    }

    // Evaluate load value in the current phase
    const int32_t load = mPhaseVec.at(mCurrentPhase->second)->Eval(t - mCurrentPhaseStart, next);
    next += mCurrentPhaseStart;
    
    return load;
}

///////////////////////////////////////////////////////////////////////////////

void LoadProfile::Stop(void)
{
    const bool stateChange = mActive;
    mActive = false;
    mCurrentPhase = mPhaseTimeVec.end();

    if (stateChange && !mActiveStateChangeDelegate.empty())
        mActiveStateChangeDelegate(mActive);
}

///////////////////////////////////////////////////////////////////////////////
