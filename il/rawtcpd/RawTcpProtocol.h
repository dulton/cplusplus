/// @file
/// @brief RAWTCP protocol header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _RAW_TCP_PROTOCOL_H_
#define _RAW_TCP_PROTOCOL_H_

#include <string>

#include <utils/MessageIterator.h>

#include "RawTcpCommon.h"

// Forward declarations (global)
class ACE_Time_Value;

DECL_RAWTCP_NS

namespace RawTcpProtocol
{

///////////////////////////////////////////////////////////////////////////////

enum MethodType
{
    RAWTCP_METHOD_UNKNOWN = 0,
    RAWTCP_METHOD_BLANK_LINE,
    RAWTCP_METHOD_REQUEST,
    RAWTCP_METHOD_RESPONSE
};

enum HeaderType
{
    RAWTCP_HEADER_UNKNOWN = 0,
    RAWTCP_HEADER_BLANK_LINE,
    RAWTCP_HEADER_CONNECTION,
    RAWTCP_HEADER_LENGTH
};

///////////////////////////////////////////////////////////////////////////////

void Init(void);

const std::string BuildRequestLine(uint32_t requestCount);

bool ParseRequestLine(L4L7_UTILS_NS::MessageConstIterator begin, L4L7_UTILS_NS::MessageConstIterator end, MethodType& method, uint32_t& requestNum);

const std::string BuildStatusLine(MethodType methodType, uint32_t responseNum);

bool ParseStatusLine(L4L7_UTILS_NS::MessageConstIterator begin, L4L7_UTILS_NS::MessageConstIterator end, uint32_t& responseNum);

const std::string BuildHeaderLine(HeaderType header, const std::string& value);
const std::string BuildHeaderLine(HeaderType header, const ACE_Time_Value& value);
const std::string BuildHeaderLine(HeaderType header, uint32_t value);

bool ParseHeaderLine(L4L7_UTILS_NS::MessageConstIterator begin, L4L7_UTILS_NS::MessageConstIterator end, HeaderType& header, std::string& value);

///////////////////////////////////////////////////////////////////////////////

}  // namespace RawTcpProtocol

END_DECL_RAWTCP_NS

#endif
