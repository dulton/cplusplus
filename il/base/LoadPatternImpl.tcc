/// @file
/// @brief Load Pattern implementation class header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_LOAD_PATTERN_IMPL_TCC_
#define _L4L7_LOAD_PATTERN_IMPL_TCC_

#include <cmath>
#include <cstdlib>
#include <utility>

#include <ace/Time_Value.h>

#include "LoadPattern.h"
#include "LoadUtils.h"

DECL_L4L7_BASE_NS

///////////////////////////////////////////////////////////////////////////////

template<class PatternDescriptorType, class PatternStateType>
class LoadPatternImpl : public LoadPattern
{
  public:
    LoadPatternImpl(const PatternDescriptorType& desc, ClientLoadPhaseTimeUnitOptions timeUnits) 
        : mY0(0)
    {
        this->InitializeHook(desc, timeUnits);
    }

    const ACE_Time_Value& Duration(void) const;

    void SetYIntercept(int32_t y0)
    {
        mY0 = y0;
        this->SetYInterceptHook();
    }
    
    int32_t Eval(const ACE_Time_Value& t, ACE_Time_Value& next);
    
  private:
    void InitializeHook(const PatternDescriptorType& desc, ClientLoadPhaseTimeUnitOptions timeUnits);
    void SetYInterceptHook(void) { }
    
    int32_t mY0;                        ///< Y-intercept value
    PatternStateType mState;            ///< pattern-specific state information
};

///////////////////////////////////////////////////////////////////////////////

struct FlatPatternState
{
    ClientLoadPhaseTimeUnitOptions timeUnits;
    int32_t height;
    long double rampSlope;
    ACE_Time_Value rampQuantum;
    ACE_Time_Value rampTime;
    ACE_Time_Value endTime;
};

typedef LoadPatternImpl<FlatPatternDescriptor_t, FlatPatternState> FlatLoadPattern;

template<>
const ACE_Time_Value& FlatLoadPattern::Duration(void) const
{
    return mState.endTime;
}

template<>
int32_t FlatLoadPattern::Eval(const ACE_Time_Value& t, ACE_Time_Value& next)
{
    if (t < mState.rampTime)
    {
        // On ramp
        next = std::min(t + mState.rampQuantum, mState.rampTime);
        return static_cast<int32_t>(mState.rampSlope * t.msec()) + mY0;
    }
    else if (t < mState.endTime)
    {
        // In steady state
        next = mState.endTime;
        return mState.height;
    }
    else
    {
        // Done
        next = ACE_Time_Value::zero;
        return mState.height;
    }
}

template<>
void FlatLoadPattern::InitializeHook(const FlatPatternDescriptor_t& desc, ClientLoadPhaseTimeUnitOptions timeUnits)
{
    mState.timeUnits = timeUnits;
    mState.height = desc.Height;
    mState.rampSlope = 0.0;
    mState.rampTime = NormalizeLoadTime(desc.RampTime, timeUnits);
    mState.endTime = mState.rampTime + NormalizeLoadTime(desc.SteadyTime, timeUnits);
}

template<>
void FlatLoadPattern::SetYInterceptHook(void)
{
    const unsigned long rampTimeMsec = mState.rampTime.msec();
    mState.rampSlope = rampTimeMsec ? (static_cast<long double>(mState.height - mY0) / rampTimeMsec) : 0.0;
    mState.rampQuantum.msec(mState.rampSlope ? abs(static_cast<long>(static_cast<long double>(1) / mState.rampSlope)) : 0);
    mState.rampQuantum = std::max(mState.rampQuantum, MinQuantum(mState.timeUnits));
}

///////////////////////////////////////////////////////////////////////////////

struct StairPatternState
{
    int32_t height;
    uint32_t repetitions;
    long double rampSlope;
    ACE_Time_Value rampQuantum;
    ACE_Time_Value rampTime;
    ACE_Time_Value steadyTime;
    ACE_Time_Value stairTime;
    ACE_Time_Value endTime;
};

typedef LoadPatternImpl<StairPatternDescriptor_t, StairPatternState> StairLoadPattern;

template<>
const ACE_Time_Value& StairLoadPattern::Duration(void) const
{
    return mState.endTime;
}

template<>
int32_t StairLoadPattern::Eval(const ACE_Time_Value& t, ACE_Time_Value& next)
{
    if (t < mState.endTime)
    {
        // On staircase
        const uint32_t stairNum = t.msec() / mState.stairTime.msec();
        const ACE_Time_Value stairStartTime = mState.stairTime * stairNum;
        const ACE_Time_Value stairDeltaTime = t - stairStartTime;

        if (stairDeltaTime < mState.rampTime)
        {
            // On ramp portion of stair
            next = std::min(t + mState.rampQuantum, stairStartTime + mState.stairTime);
            return static_cast<int32_t>(mState.rampSlope * stairDeltaTime.msec()) + (mState.height * stairNum) + mY0;
        }
        else
        {
            // On flat portion of stair
            next = stairStartTime + mState.stairTime;
            return (mState.height * (stairNum + 1)) + mY0;
        }
    }
    else
    {
        // Done
        next = ACE_Time_Value::zero;
        return (mState.height * mState.repetitions) + mY0;
    }
}

template<>
void StairLoadPattern::InitializeHook(const StairPatternDescriptor_t& desc, ClientLoadPhaseTimeUnitOptions timeUnits)
{
    mState.height = desc.Height;
    mState.repetitions = desc.Repetitions;
    mState.rampTime = NormalizeLoadTime(desc.RampTime, timeUnits);
    mState.rampSlope = desc.RampTime ? (static_cast<long double>(mState.height) / mState.rampTime.msec()) : 0.0;
    mState.rampQuantum.msec(mState.rampSlope ? abs(static_cast<long>(static_cast<long double>(1) / mState.rampSlope)) : 0);
    mState.rampQuantum = std::max(mState.rampQuantum, MinQuantum(timeUnits));
    mState.steadyTime = NormalizeLoadTime(desc.SteadyTime, timeUnits);
    mState.stairTime = mState.rampTime + mState.steadyTime;
    mState.endTime = mState.stairTime * mState.repetitions;
}

///////////////////////////////////////////////////////////////////////////////

struct BurstPatternState
{
    ClientLoadPhaseTimeUnitOptions timeUnits;
    int32_t height;
    uint32_t repetitions;
    long double rampSlope;
    ACE_Time_Value rampTime;
    ACE_Time_Value pauseTime;
    ACE_Time_Value burstTime;
    ACE_Time_Value endTime;
};

typedef LoadPatternImpl<BurstPatternDescriptor_t, BurstPatternState> BurstLoadPattern;

template<>
const ACE_Time_Value& BurstLoadPattern::Duration(void) const
{
    return mState.endTime;
}

template<>
int32_t BurstLoadPattern::Eval(const ACE_Time_Value& t, ACE_Time_Value& next)
{
    if (t < mState.endTime)
    {
        const uint32_t burstNum = t.msec() / mState.burstTime.msec();
        const ACE_Time_Value burstStartTime = mState.burstTime * burstNum;
        const ACE_Time_Value burstDeltaTime = t - burstStartTime;
        
        if (burstDeltaTime < (mState.rampTime * 2))
        {
            // In ramp portion of burst
            const uint32_t slopeMultiplier = burstNum + 1;

            ACE_Time_Value rampQuantum;
            rampQuantum.msec(mState.rampSlope ? abs(static_cast<long>(static_cast<long double>(1) / (mState.rampSlope * slopeMultiplier))) : 0);
            rampQuantum = std::max(rampQuantum, MinQuantum(mState.timeUnits));

            next = std::min(t + rampQuantum, burstStartTime + mState.burstTime);

            if (burstDeltaTime < mState.rampTime)
            {
                // Up-ramp
                return (static_cast<int32_t>(mState.rampSlope * slopeMultiplier * burstDeltaTime.msec()) + mY0);
            }
            else
            {
                // Down-ramp
                return (static_cast<int32_t>(mState.rampSlope * slopeMultiplier * -1 * (burstDeltaTime.msec() - mState.rampTime.msec())) + (mState.height * slopeMultiplier) + mY0);
            }
        }
        else
        {
            // In pause portion of burst
            next = burstStartTime + mState.burstTime;
            return mY0;
        }
    }
    else
    {
        // Done
        next = ACE_Time_Value::zero;
        return mY0;
    }
}

template<>
void BurstLoadPattern::InitializeHook(const BurstPatternDescriptor_t& desc, ClientLoadPhaseTimeUnitOptions timeUnits)
{
    mState.timeUnits = timeUnits;
    mState.height = desc.Height;
    mState.repetitions = desc.Repetitions;
    mState.rampTime = NormalizeLoadTime(desc.BurstTime, timeUnits) * 0.5;
    mState.rampSlope = mState.rampTime.msec() ? (static_cast<long double>(mState.height) / mState.rampTime.msec()) : 0.0;
    mState.pauseTime = NormalizeLoadTime(desc.PauseTime, timeUnits);
    mState.burstTime = mState.rampTime + mState.rampTime + mState.pauseTime;
    mState.endTime = mState.burstTime * mState.repetitions;
}

///////////////////////////////////////////////////////////////////////////////

struct SinusoidPatternState
{
    long double height;
    uint32_t repetitions;
    ACE_Time_Value waveQuantum;
    ACE_Time_Value period;
    ACE_Time_Value endTime;
};

typedef LoadPatternImpl<SinusoidPatternDescriptor_t, SinusoidPatternState> SinusoidLoadPattern;

template<>
const ACE_Time_Value& SinusoidLoadPattern::Duration(void) const
{
    return mState.endTime;
}

template<>
int32_t SinusoidLoadPattern::Eval(const ACE_Time_Value& t, ACE_Time_Value& next)
{
    static const long double pi2 = 3.1415926535897932384626433832795028841972L * 2.0;

    if (t < mState.endTime)
    {
        // In wave
        next = std::min(t + mState.waveQuantum, mState.endTime);

        const long double rad = (static_cast<long double>(t.msec()) / mState.period.msec()) * pi2;
        return (static_cast<int32_t>(mState.height * sin(rad)) + mY0);
    }
    else
    {
        // Done
        next = ACE_Time_Value::zero;
        return mY0;
    }
}

template<>
void SinusoidLoadPattern::InitializeHook(const SinusoidPatternDescriptor_t& desc, ClientLoadPhaseTimeUnitOptions timeUnits)
{
    mState.height = static_cast<long double>(desc.Height);
    mState.repetitions = desc.Repetitions;
    mState.waveQuantum = MinQuantum(timeUnits);
    mState.period = NormalizeLoadTime(desc.Period, timeUnits);
    mState.endTime = mState.period * mState.repetitions;
}

///////////////////////////////////////////////////////////////////////////////

struct RandomPatternState
{
    ClientLoadPhaseTimeUnitOptions timeUnits;
    long double height;
    uint32_t repetitions;
    uint32_t lastRepetition;
    int32_t lastRandomValue;
    int32_t currentRandomValue;
    long double rampSlope;
    ACE_Time_Value rampQuantum;
    ACE_Time_Value rampTime;
    ACE_Time_Value steadyTime;
    ACE_Time_Value repTime;
    ACE_Time_Value endTime;
};

typedef LoadPatternImpl<RandomPatternDescriptor_t, RandomPatternState> RandomLoadPattern;

template<>
const ACE_Time_Value& RandomLoadPattern::Duration(void) const
{
    return mState.endTime;
}

template<>
int32_t RandomLoadPattern::Eval(const ACE_Time_Value& t, ACE_Time_Value& next)
{
    if (t < mState.endTime)
    {
        // In randomness
        const uint32_t repNum = t.msec() / mState.repTime.msec();
        const ACE_Time_Value repStartTime = mState.repTime * repNum;
        const ACE_Time_Value repDeltaTime = t - repStartTime;

        // Has repetition changed?
        if (repNum != mState.lastRepetition)
        {
            mState.lastRepetition = repNum;
            mState.lastRandomValue = mState.currentRandomValue;
            
            // Calculate a new random value, slope and ramp quantum
            mState.currentRandomValue = static_cast<int32_t>((static_cast<double>(rand()) / RAND_MAX) * mState.height) + mY0;

            const unsigned long rampTimeMsec = mState.rampTime.msec();
            mState.rampSlope = rampTimeMsec ? (static_cast<long double>(mState.currentRandomValue - mState.lastRandomValue) / rampTimeMsec) : 0.0;
            mState.rampQuantum.msec(mState.rampSlope ? abs(static_cast<long>(static_cast<long double>(1) / mState.rampSlope)) : 0);
            mState.rampQuantum = std::max(mState.rampQuantum, MinQuantum(mState.timeUnits));
        }
        
        if (repDeltaTime < mState.rampTime)
        {
            // On ramp
            next = std::min(t + mState.rampQuantum, repStartTime + mState.rampTime);
            return (static_cast<int32_t>(mState.rampSlope * repDeltaTime.msec()) + mState.lastRandomValue);
        }
        else
        {
            // In steady state
            next = repStartTime + mState.repTime;
            return mState.currentRandomValue;
        }
    }
    else
    {
        // Done
        next = ACE_Time_Value::zero;
        return mY0;
    }
}

template<>
void RandomLoadPattern::InitializeHook(const RandomPatternDescriptor_t& desc, ClientLoadPhaseTimeUnitOptions timeUnits)
{
    mState.timeUnits = timeUnits;
    mState.height = desc.Height;
    mState.repetitions = desc.Repetitions;
    mState.lastRepetition = static_cast<uint32_t>(-1);
    mState.rampTime = NormalizeLoadTime(desc.RampTime, timeUnits);
    mState.steadyTime = NormalizeLoadTime(desc.SteadyTime, timeUnits);
    mState.repTime = mState.rampTime + mState.steadyTime;
    mState.endTime = mState.repTime * mState.repetitions;
}

template<>
void RandomLoadPattern::SetYInterceptHook(void)
{
    mState.lastRepetition = static_cast<uint32_t>(-1);
    mState.currentRandomValue = mY0;
}

///////////////////////////////////////////////////////////////////////////////

struct SawToothPatternState
{
    int32_t height;
    uint32_t repetitions;
    ACE_Time_Value pauseTime;
    ACE_Time_Value steadyTime;
    ACE_Time_Value toothTime;
    ACE_Time_Value endTime;
};

typedef LoadPatternImpl<SawToothPatternDescriptor_t, SawToothPatternState> SawToothLoadPattern;

template<>
const ACE_Time_Value& SawToothLoadPattern::Duration(void) const
{
    return mState.endTime;
}

template<>
int32_t SawToothLoadPattern::Eval(const ACE_Time_Value& t, ACE_Time_Value& next)
{
    if (t < mState.endTime)
    {
        const uint32_t toothNum = t.msec() / mState.toothTime.msec();
        const ACE_Time_Value toothStartTime = mState.toothTime * toothNum;
        const ACE_Time_Value toothDeltaTime = t - toothStartTime;
        
        if (toothDeltaTime < mState.pauseTime)
        {
            // In pause portion of tooth
            next = toothStartTime + mState.pauseTime;
            return mY0;
        }
        else
        {
            // In steady portion of tooth
            next = toothStartTime + mState.toothTime;
            return mState.height + mY0;
        }
    }
    else
    {
        // Done
        next = ACE_Time_Value::zero;
        return mState.height + mY0;
    }
}

template<>
void SawToothLoadPattern::InitializeHook(const SawToothPatternDescriptor_t& desc, ClientLoadPhaseTimeUnitOptions timeUnits)
{
    mState.height = desc.Height;
    mState.repetitions = desc.Repetitions;
    mState.pauseTime = NormalizeLoadTime(desc.PauseTime, timeUnits);
    mState.steadyTime = NormalizeLoadTime(desc.SteadyTime, timeUnits);
    mState.toothTime = mState.pauseTime + mState.steadyTime;
    mState.endTime = mState.toothTime * mState.repetitions;
}

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_BASE_NS

// Local Variables:
// mode:c++
// End:

#endif
