/// @file
/// @brief FPGARTP common header file
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _FPGARTP_COMMON_H_
#define _FPGARTP_COMMON_H_

///////////////////////////////////////////////////////////////////////////////

#include "VoIPCommon.h"

///////////////////////////////////////////////////////////////////////////////

#ifndef FPGARTP_NS
# define FPGARTP_NS MEDIA_NS::fpgartp
#endif

#ifndef USING_FPGARTP_NS
# define USING_FPGARTP_NS using namespace MEDIA_NS::fpgartp
#endif

#ifndef DECL_FPGARTP_NS
# define DECL_FPGARTP_NS DECL_MEDIA_NS namespace fpgartp {
# define END_DECL_FPGARTP_NS END_DECL_MEDIA_NS }
#endif

#ifndef DECL_CLASS_FORWARD_FPGARTP_NS
# define DECL_CLASS_FORWARD_FPGARTP_NS(CLASSNAME) DECL_FPGARTP_NS class CLASSNAME; END_DECL_FPGARTP_NS
#endif

///////////////////////////////////////////////////////////////////////////////

#endif
