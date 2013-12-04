/// @file
/// @brief L4L7 base library common header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_BASE_COMMON_H_
#define _L4L7_BASE_COMMON_H_

///////////////////////////////////////////////////////////////////////////////

#include "l4l7Base_1_port_server.h"

namespace l4l7Base_1_port_server
{
    using namespace contentBaseLoad_1_port_server;

    typedef contentBaseLoad_1_port_server::ProtocolLoadProfile_t ClientLoadProfile_t;
    typedef contentBaseLoad_1_port_server::ProtocolLoadPhase_t ClientLoadPhase_t;
}

namespace L4L7Base_1_port_server
{
    using namespace ContentBaseLoad_1_port_server;

    typedef ContentBaseLoad_1_port_server::ProtocolLoadConfig_t ClientLoadConfig_t;
    typedef ContentBaseLoad_1_port_server::ProtocolLoadPhaseConfig_t ClientLoadPhaseConfig_t;
}


#ifndef L4L7_BASE_NS
# define L4L7_BASE_NS l4l7Base_1_port_server
#endif

#ifndef USING_L4L7_BASE_NS
# define USING_L4L7_BASE_NS using namespace l4l7Base_1_port_server
#endif

#ifndef USING_CONTENTBASE_LOAD_NS
#define USING_CONTENTBASE_LOAD_NS using namespace contentBaseLoad_1_port_server
#endif


#ifndef DECL_L4L7_BASE_NS
# define DECL_L4L7_BASE_NS namespace l4l7Base_1_port_server {
# define END_DECL_L4L7_BASE_NS }
#endif

///////////////////////////////////////////////////////////////////////////////

#endif
