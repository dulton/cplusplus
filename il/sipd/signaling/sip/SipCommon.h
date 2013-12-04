/// @file
/// @brief SIP common header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _SIP_COMMON_H_
#define _SIP_COMMON_H_

///////////////////////////////////////////////////////////////////////////////

#include "VoIPCommon.h"

#ifndef SIP_NS
# define SIP_NS SIG_NS::sip
#endif

#ifndef USING_SIP_NS
# define USING_SIP_NS using namespace SIG_NS::sip
#endif

#ifndef DECL_SIP_NS
# define DECL_SIP_NS DECL_SIG_NS namespace sip {
# define END_DECL_SIP_NS END_DECL_SIG_NS }
#endif

#ifndef DECL_CLASS_FORWARD_SIP_NS
# define DECL_CLASS_FORWARD_SIP_NS(CLASSNAME) DECL_SIP_NS class CLASSNAME; END_DECL_SIP_NS
#endif

///////////////////////////////////////////////////////////////////////////////

#ifndef SIPLEAN_NS
# define SIPLEAN_NS SIP_NS::lean
#endif

#ifndef USING_SIPLEAN_NS
# define USING_SIPLEAN_NS using namespace SIP_NS::lean
#endif

#ifndef DECL_SIPLEAN_NS
# define DECL_SIPLEAN_NS DECL_SIP_NS namespace lean {
# define END_DECL_SIPLEAN_NS END_DECL_SIP_NS }
#endif

#ifndef DECL_CLASS_FORWARD_SIPLEAN_NS
# define DECL_CLASS_FORWARD_SIPLEAN_NS(CLASSNAME) DECL_SIPLEAN_NS class CLASSNAME; END_DECL_SIPLEAN_NS
#endif

///////////////////////////////////////////////////////////////////////////////

#ifndef RVSIP_NS
# define RVSIP_NS SIP_NS::lean
#endif

#ifndef USING_RVSIP_NS
# define USING_RVSIP_NS using namespace SIP_NS::rv
#endif

#ifndef DECL_RVSIP_NS
# define DECL_RVSIP_NS DECL_SIP_NS namespace rv {
# define END_DECL_RVSIP_NS END_DECL_SIP_NS }
#endif

#ifndef DECL_CLASS_FORWARD_RVSIP_NS
# define DECL_CLASS_FORWARD_RVSIP_NS(CLASSNAME) DECL_RVSIP_NS class CLASSNAME; END_DECL_RVSIP_NS
#endif

///////////////////////////////////////////////////////////////////////////////

#define ALWAYS_USE_LOOSE_ROUTING_ON_UE (true)

///////////////////////////////////////////////////////////////////////////////

#endif
