/// @file
/// @brief VOIP common header file
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _VOIP_COMMON_H_
#define _VOIP_COMMON_H_

///////////////////////////////////////////////////////////////////////////////

#include "sip_1_port_server.h"

///////////////////////////////////////////////////////////////////////////////

#ifndef VOIP_NS
# define VOIP_NS voip
#endif

#ifndef USING_VOIP_NS
# define USING_VOIP_NS using namespace voip
#endif

#ifndef DECL_VOIP_NS
# define DECL_VOIP_NS namespace voip {
# define END_DECL_VOIP_NS }
#endif

#ifndef DECL_CLASS_FORWARD_VOIP_NS
# define DECL_CLASS_FORWARD_VOIP_NS(CLASSNAME) DECL_VOIP_NS class CLASSNAME; END_DECL_VOIP_NS
#endif

/////////////////////////////////////// APP ////////////////////////////

#ifndef APP_NS
# define APP_NS VOIP_NS::app
#endif

#ifndef USING_APP_NS
# define USING_APP_NS using namespace VOIP_NS::app
#endif

#ifndef DECL_APP_NS
# define DECL_APP_NS DECL_VOIP_NS namespace app {
# define END_DECL_APP_NS END_DECL_VOIP_NS }
#endif

#ifndef DECL_CLASS_FORWARD_APP_NS
# define DECL_CLASS_FORWARD_APP_NS(CLASSNAME) DECL_APP_NS class CLASSNAME; END_DECL_APP_NS
#endif

/////////////////////////////////////// SIGNALING ////////////////////////////

#ifndef SIG_NS
# define SIG_NS VOIP_NS::signaling
#endif

#ifndef USING_SIG_NS
# define USING_SIG_NS using namespace VOIP_NS::signaling
#endif

#ifndef DECL_SIG_NS
# define DECL_SIG_NS DECL_VOIP_NS namespace signaling {
# define END_DECL_SIG_NS END_DECL_VOIP_NS }
#endif

#ifndef DECL_CLASS_FORWARD_SIG_NS
# define DECL_CLASS_FORWARD_SIG_NS(CLASSNAME) DECL_SIG_NS class CLASSNAME; END_DECL_SIG_NS
#endif

/////////////////////////////////////// MEDIA ////////////////////////////

#ifndef MEDIA_NS
# define MEDIA_NS VOIP_NS::voipmedia
#endif

#ifndef USING_MEDIA_NS
# define USING_MEDIA_NS using namespace VOIP_NS::voipmedia
#endif

#ifndef DECL_MEDIA_NS
# define DECL_MEDIA_NS DECL_VOIP_NS namespace voipmedia {
# define END_DECL_MEDIA_NS END_DECL_VOIP_NS }
#endif

#ifndef DECL_CLASS_FORWARD_MEDIA_NS
# define DECL_CLASS_FORWARD_MEDIA_NS(CLASSNAME) DECL_MEDIA_NS class CLASSNAME; END_DECL_MEDIA_NS
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined(UNIT_TEST)
#define isUnitTest (true)
#else
#define isUnitTest (false)
#endif

///////////////////////////////////////////////////////////////////////////////

#include "VoIPLog.h"

///////////////////////////////////////////////////////////////////////////////

#include <boost/function.hpp>

//////////////////////////////////////////////////////////////////////////////

DECL_VOIP_NS

static const uint16_t DEFAULT_SIP_PORT_NUMBER = 5060;
static const uint32_t INVALID_STREAM_INDEX = static_cast<const uint32_t>(-1);

typedef boost::function1<void,bool> TrivialBoolFunctionType;

END_DECL_VOIP_NS

///////////////////////////////////////////////////////////////////////////////

#include <ace/INET_Addr.h>

  //this code is here only to produce ambig. compile-time error when two ACE_INET_Addr are compared.
bool operator==(const ACE_INET_Addr& lhs, const ACE_INET_Addr& rhs);

///////////////////////////////////////////////////////////////////////////////

#endif
