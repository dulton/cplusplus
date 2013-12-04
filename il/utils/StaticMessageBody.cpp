/// @file
/// @brief Static Message Body implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <string.h>

#include "StaticMessageBody.h"

USING_L4L7_UTILS_NS;

///////////////////////////////////////////////////////////////////////////////

const size_t StaticMessageBody::DEFAULT_SIZE = 128 * 1024;
const StaticMessageBody StaticMessageBody::DEFAULT_BODY(DEFAULT_SIZE, '*');

///////////////////////////////////////////////////////////////////////////////

StaticMessageBody::StaticMessageBody(size_t size, char fill)
    : ACE_Data_Block(size, ACE_Message_Block::MB_DATA, 0, 0, 0, 0, 0)
{
    memset(base(), fill, size);
}

///////////////////////////////////////////////////////////////////////////////
