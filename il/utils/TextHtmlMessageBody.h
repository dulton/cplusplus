/// @file
/// @brief Text/HTML Message Body header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_UTILS_TEXT_HTML_MESSAGE_BODY_H_
#define _L4L7_UTILS_TEXT_HTML_MESSAGE_BODY_H_

#include <string>

#include <ace/Message_Block.h>
#include <utils/UtilsCommon.h>

DECL_L4L7_UTILS_NS

///////////////////////////////////////////////////////////////////////////////

class TextHtmlMessageBody : public ACE_Message_Block
{
  public:
    explicit TextHtmlMessageBody(size_t size);

  private:
    static const std::string HTML_HEADER;
    static const std::string HTML_TRAILER;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_UTILS_NS

#endif
