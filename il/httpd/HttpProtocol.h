/// @file
/// @brief HTTP protocol header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _HTTP_PROTOCOL_H_
#define _HTTP_PROTOCOL_H_

#include <string>

#include <utils/MessageIterator.h>

#include "HttpCommon.h"

// Forward declarations (global)
class ACE_Time_Value;

DECL_HTTP_NS

namespace HttpProtocol
{
    
///////////////////////////////////////////////////////////////////////////////

enum MethodType
{
    HTTP_METHOD_UNKNOWN = 0,
    HTTP_METHOD_BLANK_LINE,
    HTTP_METHOD_OPTIONS,
    HTTP_METHOD_GET,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_TRACE,
    HTTP_METHOD_CONNECT
};

enum Version
{
    HTTP_VER_UNKNOWN = 0,
    HTTP_VER_0_9,
    HTTP_VER_1_0,
    HTTP_VER_1_1
};

enum HeaderType
{
    HTTP_HEADER_UNKNOWN = 0,
    HTTP_HEADER_BLANK_LINE,
    HTTP_HEADER_CONNECTION,
    HTTP_HEADER_CONTENT_LENGTH,
    HTTP_HEADER_CONTENT_TYPE,
    HTTP_HEADER_EXPIRES,
    HTTP_HEADER_HOST,
    HTTP_HEADER_LAST_MODIFIED,
    HTTP_HEADER_SERVER,
    HTTP_HEADER_USER_AGENT
};

enum StatusCodeCategory
{
    HTTP_STATUS_INFORMATIONAL = 1,
    HTTP_STATUS_SUCCESS = 2,
    HTTP_STATUS_REDIRECTION = 3,
    HTTP_STATUS_CLIENT_ERROR = 4,
    HTTP_STATUS_SERVER_ERROR = 5
};

enum StatusCode
{
    HTTP_STATUS_UNKNOWN = 0,
    HTTP_STATUS_OK = 200,
    HTTP_STATUS_BAD_REQUEST = 400,
    HTTP_STATUS_NOT_FOUND = 404,
    HTTP_STATUS_METHOD_NOT_ALLOWED = 405
};

///////////////////////////////////////////////////////////////////////////////

void Init(void);

const std::string BuildRequestLine(MethodType method, const std::string& uri, Version version);

bool ParseRequestLine(L4L7_UTILS_NS::MessageConstIterator begin, L4L7_UTILS_NS::MessageConstIterator end, MethodType& method, std::string& uri, Version& version);

const std::string BuildStatusLine(Version version, StatusCode statusCode);

bool ParseStatusLine(L4L7_UTILS_NS::MessageConstIterator begin, L4L7_UTILS_NS::MessageConstIterator end, Version& version, StatusCode& statusCode);

const std::string BuildHeaderLine(HeaderType header, const std::string& value);
const std::string BuildHeaderLine(HeaderType header, const ACE_Time_Value& value);
const std::string BuildHeaderLine(HeaderType header, uint32_t value);

bool ParseHeaderLine(L4L7_UTILS_NS::MessageConstIterator begin, L4L7_UTILS_NS::MessageConstIterator end, HeaderType& header, std::string& value);

///////////////////////////////////////////////////////////////////////////////

}  // namespace HttpProtocol

END_DECL_HTTP_NS

#endif
