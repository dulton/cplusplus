/// @file
/// @brief XMPP common header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _XMPP_COMMON_H_
#define _XMPP_COMMON_H_

///////////////////////////////////////////////////////////////////////////////

#include "xmppvj_1_port_server.h"

#ifndef XMPP_NS
# define XMPP_NS xmpp
#endif

#ifndef USING_XMPP_NS
# define USING_XMPP_NS using namespace xmpp
#endif

#ifndef DECL_XMPP_NS
# define DECL_XMPP_NS namespace xmpp {
# define END_DECL_XMPP_NS }
#endif

extern const std::string CapabilitiesFilePath;

///////////////////////////////////////////////////////////////////////////////

#endif
