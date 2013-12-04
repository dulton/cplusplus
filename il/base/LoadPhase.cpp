/// @file
/// @brief Load Phase implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <sstream>

#include <phxexception/PHXException.h>
#include <phxlog/phxlog.h>

#include "LoadPatternImpl.tcc"
#include "LoadPhase.h"

USING_L4L7_BASE_NS;

///////////////////////////////////////////////////////////////////////////////

LoadPhase::LoadPhase(const ClientLoadPhase_t& phase, const void *patternDesc)
    : mPattern(MakePatternImpl(static_cast<LoadPatternTypes>(phase.LoadPattern), patternDesc, static_cast<ClientLoadPhaseTimeUnitOptions>(phase.LoadPhaseDurationUnits)))
{
}

///////////////////////////////////////////////////////////////////////////////

LoadPhase::~LoadPhase()
{
}

///////////////////////////////////////////////////////////////////////////////

LoadPattern *LoadPhase::MakePatternImpl(LoadPatternTypes patternType, const void *patternDesc, ClientLoadPhaseTimeUnitOptions timeUnits)
{
    LoadPattern *pattern(0);
    
    switch (patternType)
    {
      case LoadPatternTypes_FLAT:
      {
          const FlatPatternDescriptor_t *flatDesc = reinterpret_cast<const FlatPatternDescriptor_t *>(patternDesc);
          pattern = new FlatLoadPattern(*flatDesc, timeUnits);
          break;
      }

      case LoadPatternTypes_STAIR:
      {
          const StairPatternDescriptor_t *stairDesc = reinterpret_cast<const StairPatternDescriptor_t *>(patternDesc);
          pattern = new StairLoadPattern(*stairDesc, timeUnits);
          break;
      }

      case LoadPatternTypes_BURST:
      {
          const BurstPatternDescriptor_t *burstDesc = reinterpret_cast<const BurstPatternDescriptor_t *>(patternDesc);
          pattern = new BurstLoadPattern(*burstDesc, timeUnits);
          break;
      }

      case LoadPatternTypes_SINUSOID:
      {
          const SinusoidPatternDescriptor_t *sinusoidDesc = reinterpret_cast<const SinusoidPatternDescriptor_t *>(patternDesc);
          pattern = new SinusoidLoadPattern(*sinusoidDesc, timeUnits);
          break;
      }

      case LoadPatternTypes_RANDOM:
      {
          const RandomPatternDescriptor_t *randomDesc = reinterpret_cast<const RandomPatternDescriptor_t *>(patternDesc);
          pattern = new RandomLoadPattern(*randomDesc, timeUnits);
          break;
      }

      case LoadPatternTypes_SAWTOOTH:
      {
          const SawToothPatternDescriptor_t *sawToothDesc = reinterpret_cast<const SawToothPatternDescriptor_t *>(patternDesc);
          pattern = new SawToothLoadPattern(*sawToothDesc, timeUnits);
          break;
      }
      
      default:
      {
          std::ostringstream oss;
          oss << "[LoadPhase::MakePatternImpl] Cannot create implementation for unknown pattern type (" << tms_enum_to_string(patternType, em_LoadPatternTypes) << ")";
          const std::string err(oss.str());
          TC_LOG_ERR(0, err);
          throw EPHXBadConfig(err);
      }
    }

    return pattern;
}

///////////////////////////////////////////////////////////////////////////////
