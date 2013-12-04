/// @file
/// @brief Load utilities header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_LOAD_UTILS_H_
#define _L4L7_LOAD_UTILS_H_

#include <base/BaseCommon.h>

// Forward declarations (global)
class ACE_Time_Value;

DECL_L4L7_BASE_NS

///////////////////////////////////////////////////////////////////////////////

const ACE_Time_Value NormalizeLoadTime(uint32_t time, ClientLoadPhaseTimeUnitOptions timeUnits);

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_BASE_NS

#endif
