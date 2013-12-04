/// @file
/// @brief Application/Octet-Stream Message Body implementation
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
#include <utility>

#include "AppOctetStreamMessageBody.h"
#include "StaticMessageBody.h"

USING_L4L7_UTILS_NS;

///////////////////////////////////////////////////////////////////////////////

AppOctetStreamMessageBody::AppOctetStreamMessageBody(size_t size)
{
    // Chain message blocks with static body
    const ACE_Data_Block * const staticBody = &StaticMessageBody::DEFAULT_BODY;
    ACE_Message_Block *tail = 0;

    while (size)
    {
        const size_t mbSize = std::min(size, StaticMessageBody::DEFAULT_SIZE);
        
        if (!tail)
        {
            // First message block
            this->data_block(const_cast<ACE_Data_Block *>(staticBody));
            this->set_self_flags(DONT_DELETE);
            this->wr_ptr(mbSize);

            tail = this;
        }
        else
        {
            std::auto_ptr<ACE_Message_Block> body(new ACE_Message_Block(const_cast<ACE_Data_Block *>(staticBody), DONT_DELETE));
            body->wr_ptr(mbSize);
        
            tail->cont(body.release());
            tail = tail->cont();
        }

        size -= mbSize;
    }
}

///////////////////////////////////////////////////////////////////////////////
