/// @file
/// @brief Enc RTP Radvision include defs
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ENCRTP_RVNET_H_
#define _ENCRTP_RVNET_H_

//////////////////////////////////////////////////////////////////////////////////////////////

//Radvision media stack section:


#if !defined(UPDATED_BY_SPIRENT)
#define UPDATED_BY_SPIRENT
#endif

#define FSTART rtpprintf("%s:%s:%d: begin\n", __FILE__, __FUNCTION__, __LINE__)
#define FEND rtpprintf("%s:%s:%d: end\n", __FILE__, __FUNCTION__, __LINE__)

#include <rvexternal.h>
#include <rvconfig.h>
#include <rtp.h>
#include <rtcp.h>
#include <rvrtpseli.h>

#include "rtp_spirent.h"

///////////////////////////////////////////////////////////////////////////////

#endif //_ENCRTP_RVNET_H_

