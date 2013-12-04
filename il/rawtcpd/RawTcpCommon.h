/// @file
/// @brief raw tcp common header file
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _RAWTCP_COMMON_H_
#define _RAWTCP_COMMON_H_

///////////////////////////////////////////////////////////////////////////////

#include "rawTcp_1_port_server.h"

#ifndef RAWTCP_NS
# define RAWTCP_NS RawTcp
#endif

#ifndef USING_RAWTCP_NS
# define USING_RAWTCP_NS using namespace RawTcp
#endif

#ifndef DECL_RAWTCP_NS
# define DECL_RAWTCP_NS namespace RawTcp {
# define END_DECL_RAWTCP_NS }
#endif

///////////////////////////////////////////////////////////////////////////////

#endif
