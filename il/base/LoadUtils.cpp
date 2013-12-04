/// @file
/// @brief Load utilities implementation
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

#include "LoadUtils.h"

///////////////////////////////////////////////////////////////////////////////

const ACE_Time_Value L4L7_BASE_NS::NormalizeLoadTime(uint32_t time, ClientLoadPhaseTimeUnitOptions timeUnits)
{
    switch (timeUnits)
    {
      case ClientLoadPhaseTimeUnitOptions_MILLISECONDS:
      {
          ACE_Time_Value theTime;
          theTime.msec(time);
          return theTime;
      }
      
      case ClientLoadPhaseTimeUnitOptions_SECONDS:
          return ACE_Time_Value(time, 0);
          
      case ClientLoadPhaseTimeUnitOptions_MINUTES:
          return ACE_Time_Value(time * 60, 0);
          
      case ClientLoadPhaseTimeUnitOptions_HOURS:
          return ACE_Time_Value(time * 3600, 0);

      default:
          throw EPHXBadConfig("NormalizeLoadTime");
    }
}

///////////////////////////////////////////////////////////////////////////////
