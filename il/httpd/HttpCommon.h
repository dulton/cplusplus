/// @file
/// @brief HTTP common header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _HTTP_COMMON_H_
#define _HTTP_COMMON_H_

///////////////////////////////////////////////////////////////////////////////

#include "http_1_port_server.h"

#ifndef HTTP_NS
# define HTTP_NS http
#endif

#ifndef USING_HTTP_NS
# define USING_HTTP_NS using namespace http
#endif

#ifndef DECL_HTTP_NS
# define DECL_HTTP_NS namespace http {
# define END_DECL_HTTP_NS }
#endif

///////////////////////////////////////////////////////////////////////////////

#endif
