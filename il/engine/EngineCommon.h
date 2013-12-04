/// @file
/// @brief L4L7 engine library common header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_ENGINE_COMMON_H_
#define _L4L7_ENGINE_COMMON_H_

///////////////////////////////////////////////////////////////////////////////

#ifndef L4L7_ENGINE_NS
# define L4L7_ENGINE_NS l4l7Engine
#endif

#ifndef USING_L4L7_ENGINE_NS
# define USING_L4L7_ENGINE_NS using namespace l4l7Engine
#endif

#ifndef DECL_L4L7_ENGINE_NS
# define DECL_L4L7_ENGINE_NS namespace l4l7Engine {
# define END_DECL_L4L7_ENGINE_NS }
#endif

///////////////////////////////////////////////////////////////////////////////

#include <engine/config.h>

///////////////////////////////////////////////////////////////////////////////

DECL_L4L7_ENGINE_NS

typedef uint32_t handle_t;

END_DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

#endif
