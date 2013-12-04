/// @file
/// @brief STC-IL specific config file for DPG engine 
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef STC_IL_CONFIG_H
#define STC_IL_CONFIG_H

#include <phxexception/PHXException.h>

DECL_L4L7_ENGINE_NS

// exceptions -- should take a string in their constructor
typedef EPHXInternal DPG_EInternal;
typedef EPHXBadConfig DPG_EBadConfig;

END_DECL_L4L7_ENGINE_NS


#endif
