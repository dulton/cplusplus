/// @file
/// @brief Engine hooks implementation
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include "EngineHooks.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

// static definitions
EngineHooks::logDelegate_t EngineHooks::mErrLog;
EngineHooks::logDelegate_t EngineHooks::mWarnLog;
EngineHooks::logDelegate_t EngineHooks::mInfoLog;

///////////////////////////////////////////////////////////////////////////////

