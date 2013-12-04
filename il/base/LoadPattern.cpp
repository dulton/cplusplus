/// @file
/// @brief Load Pattern base class implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <ace/Time_Value.h>
#include <phxexception/PHXException.h>

#include "LoadPattern.h"

USING_L4L7_BASE_NS;

///////////////////////////////////////////////////////////////////////////////

const ACE_Time_Value LoadPattern::MinQuantum(ClientLoadPhaseTimeUnitOptions timeUnits)
{
    switch (timeUnits)
    {
      case ClientLoadPhaseTimeUnitOptions_MILLISECONDS:
          return ACE_Time_Value(0, 10000);      // 10 milliseconds
      
      case ClientLoadPhaseTimeUnitOptions_SECONDS:
          return ACE_Time_Value(0, 50000);      // 50 milliseconds
          
      case ClientLoadPhaseTimeUnitOptions_MINUTES:
      case ClientLoadPhaseTimeUnitOptions_HOURS:
          return ACE_Time_Value(0, 100000);     // 100 milliseconds

      default:
          throw EPHXBadConfig("LoadPattern::MinQuantum");
    }
}

///////////////////////////////////////////////////////////////////////////////
