/// @file
/// @brief Message Utilities header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_MESSAGE_UTILS_H_
#define _L4L7_MESSAGE_UTILS_H_

#include <ace/Message_Block.h>
#include <boost/bind.hpp>
#include <Tr1Adapter.h>

#include <utils/UtilsCommon.h>

// Forward declaration (global)
class ACE_Message_Block;

DECL_L4L7_UTILS_NS

///////////////////////////////////////////////////////////////////////////////

inline std::tr1::shared_ptr<ACE_Message_Block> MessageAlloc(ACE_Message_Block *mb)
{
    return std::tr1::shared_ptr<ACE_Message_Block>(mb, boost::bind(static_cast<ACE_Message_Block *(*)(ACE_Message_Block *)>(&ACE_Message_Block::release), _1));
}

///////////////////////////////////////////////////////////////////////////////

std::tr1::shared_ptr<ACE_Message_Block> MessageAppend(std::tr1::shared_ptr<ACE_Message_Block> begin, std::tr1::shared_ptr<ACE_Message_Block> tail);

std::tr1::shared_ptr<ACE_Message_Block> MessageTrim(std::tr1::shared_ptr<ACE_Message_Block> begin, const ACE_Message_Block *end);

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_UTILS_NS

#endif
