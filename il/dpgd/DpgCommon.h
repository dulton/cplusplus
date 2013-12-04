/// @file
/// @brief dpg common header file
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _DPG_COMMON_H_
#define _DPG_COMMON_H_

///////////////////////////////////////////////////////////////////////////////

#include "dpg_1_port_server.h"

#ifndef DPG_NS
# define DPG_NS Dpg
#endif

#ifndef USING_DPG_NS
# define USING_DPG_NS using namespace Dpg
#endif

#ifndef DECL_DPG_NS
# define DECL_DPG_NS namespace Dpg {
# define END_DECL_DPG_NS }
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef UNIT_TEST
#define UNIT_TEST_VIRTUAL virtual
#else
#define UNIT_TEST_VIRTUAL 
#endif

///////////////////////////////////////////////////////////////////////////////

#endif
