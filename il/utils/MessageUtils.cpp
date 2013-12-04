/// @file
/// @brief Message Utilities implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <memory>

#include "MessageIterator.h"
#include "MessageUtils.h"

DECL_L4L7_UTILS_NS

///////////////////////////////////////////////////////////////////////////////

std::tr1::shared_ptr<ACE_Message_Block> MessageAppend(std::tr1::shared_ptr<ACE_Message_Block> begin, std::tr1::shared_ptr<ACE_Message_Block> tail)
{
    ACE_Message_Block *end = begin.get();

    // Find the end of the current message block chain
    while (end && end->cont())
        end = end->cont();

    // Append tail
    if (end)
    {
        std::auto_ptr<ACE_Message_Block> temp(tail->duplicate());
        end->cont(temp.release());
        return begin;
    }
    else
        return tail;
}

///////////////////////////////////////////////////////////////////////////////

std::tr1::shared_ptr<ACE_Message_Block> MessageTrim(std::tr1::shared_ptr<ACE_Message_Block> begin, const ACE_Message_Block *end)
{
    // Discard blocks leading up to the end block
    while (begin && begin.get() != end)
    {
        std::tr1::shared_ptr<ACE_Message_Block> temp(MessageAlloc(begin->cont()));
        begin->cont(0);
        begin = temp;
    }

    // Also discard the end block if it is empty
    if (begin && begin->length() == 0)
    {
        std::tr1::shared_ptr<ACE_Message_Block> temp(MessageAlloc(begin->cont()));
        begin->cont(0);
        begin = temp;
    }
    
    return begin;
}
    
///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_UTILS_NS
