/// @file
/// @brief L4L7 application library common header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_APP_COMMON_H_
#define _L4L7_APP_COMMON_H_

///////////////////////////////////////////////////////////////////////////////

#ifndef L4L7_APP_NS
# define L4L7_APP_NS l4l7App
#endif

#ifndef USING_L4L7_APP_NS
# define USING_L4L7_APP_NS using namespace l4l7App
#endif

#ifndef DECL_L4L7_APP_NS
# define DECL_L4L7_APP_NS namespace l4l7App {
# define END_DECL_L4L7_APP_NS }
#endif

///////////////////////////////////////////////////////////////////////////////

// Per-daemon globals, must be provided by app-specific code
#include <string>
extern std::string ModuleName;
extern std::string LogMaskDescription;

#ifdef GPROF
# include <sys/time.h>
extern struct itimerval GprofItimer;
#endif

///////////////////////////////////////////////////////////////////////////////

#endif
