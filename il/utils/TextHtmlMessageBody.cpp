/// @file
/// @brief Text/HTML Message Body implementation
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
#include <string.h>
#include <utility>

#include "StaticMessageBody.h"
#include "TextHtmlMessageBody.h"

USING_L4L7_UTILS_NS;

///////////////////////////////////////////////////////////////////////////////

const std::string TextHtmlMessageBody::HTML_HEADER("<html><pre>");
const std::string TextHtmlMessageBody::HTML_TRAILER("</pre></html>\r\n");

///////////////////////////////////////////////////////////////////////////////

TextHtmlMessageBody::TextHtmlMessageBody(size_t size)
{
    const ACE_Data_Block * const staticBody = &StaticMessageBody::DEFAULT_BODY;
    const size_t headerSize = HTML_HEADER.size();
    const size_t trailerSize = HTML_TRAILER.size();
        
    if (size < (headerSize + trailerSize))
    {
        // Single message block contains a small static body with no header or trailer
        this->data_block(const_cast<ACE_Data_Block *>(staticBody));
        this->set_self_flags(DONT_DELETE);
        this->wr_ptr(size);
    }
    else
    {
        // Initial message block contains the header
        this->init(headerSize);
        strncpy(this->wr_ptr(), HTML_HEADER.data(), headerSize);
        this->wr_ptr(headerSize);

        // Chain continuation message blocks with static body
        ACE_Message_Block *tail = this;
        size -= headerSize + trailerSize;

        while (size)
        {
            std::auto_ptr<ACE_Message_Block> body(new ACE_Message_Block(const_cast<ACE_Data_Block *>(staticBody), DONT_DELETE));

            const size_t mbSize = std::min(size, StaticMessageBody::DEFAULT_SIZE);
            body->wr_ptr(mbSize);
            size -= mbSize;

            tail->cont(body.release());
            tail = tail->cont();
        }

        // Final message block contains the trailer
        std::auto_ptr<ACE_Message_Block> trailer(new ACE_Message_Block(trailerSize));
        strncpy(trailer->wr_ptr(), HTML_TRAILER.data(), trailerSize);
        trailer->wr_ptr(trailerSize);
        tail->cont(trailer.release());
    }
}

///////////////////////////////////////////////////////////////////////////////
