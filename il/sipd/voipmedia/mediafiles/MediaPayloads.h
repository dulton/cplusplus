/// @file
/// @brief Media Files Common defs
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _MEDIAFILES_PAYLOADS_H_
#define _MEDIAFILES_PAYLOADS_H_

#include "MediaFilesCommon.h"

DECL_MEDIAFILES_NS

enum PAYLOAD_NUMBERS
{
  PAYLOAD_711u = 0,
  PAYLOAD_726  = 2,
  PAYLOAD_723  = 4,
  PAYLOAD_711A = 8,
  PAYLOAD_722  = 9,
  PAYLOAD_SID  = 13,
  PAYLOAD_728  = 15,
  PAYLOAD_729  = 18, 
  PAYLOAD_SID_19 = 19,
  VPAYLOAD_263 = 34,
  PAYLOAD_UNDEFINED = 0xFF
};

END_DECL_MEDIAFILES_NS

#endif //_MEDIAFILES_PAYLOADS_H_

