/// @file
/// @brief HTTP protocol implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <functional>
#include <sstream>

#include <ace/OS_NS_time.h>
#include <ace/Time_Value.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/functional/hash.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_loops.hpp>
#include <boost/unordered_map.hpp>
#include <phxexception/PHXException.h>

#include "HttpProtocol.h"

using namespace boost::spirit::classic;

DECL_HTTP_NS

namespace HttpProtocol
{

///////////////////////////////////////////////////////////////////////////////

struct NoCaseHashString : std::unary_function<std::string, std::size_t>
{
    size_t operator()(const std::string& x) const
    {
        size_t seed = 0;

        std::string::const_iterator end = x.end();
        for (std::string::const_iterator it = x.begin(); it != end; ++it)
        {
            const char ch = *it;
            boost::hash_combine(seed, (ch - 32 * (ch >='a' && ch <= 'z')));
        }

        return seed;
    }
};

struct NoCaseCompareString : public std::binary_function<std::string, std::string, bool>
{
    bool operator()(const std::string& s1, const std::string& s2) const { return boost::iequals(s1, s2); }
};

typedef boost::unordered_map<std::string, MethodType> MethodTypeMap_t;
typedef boost::unordered_map<MethodType, std::string> MethodStringMap_t;

typedef boost::unordered_map<Version, std::string> VersionStringMap_t;

typedef boost::unordered_map<std::string, StatusCode> StatusCodeMap_t;
typedef boost::unordered_map<StatusCode, std::string> ReasonPhraseStringMap_t;
    
typedef boost::unordered_map<std::string, HeaderType, NoCaseHashString, NoCaseCompareString> HeaderTypeMap_t;
typedef boost::unordered_map<HeaderType, std::string> HeaderStringMap_t;
    
///////////////////////////////////////////////////////////////////////////////

static const std::string MethodOptions = "OPTIONS";
static const std::string MethodGet = "GET";
static const std::string MethodHead = "HEAD";
static const std::string MethodPost = "POST";
static const std::string MethodPut = "PUT";
static const std::string MethodDelete = "DELETE";
static const std::string MethodTrace = "TRACE";
static const std::string MethodConnect = "CONNECT";

static const std::string Version_0_9 = "HTTP/0.9";
static const std::string Version_1_0 = "HTTP/1.0";
static const std::string Version_1_1 = "HTTP/1.1";

static const std::string StatusOk = "200";
static const std::string StatusBadRequest = "400";
static const std::string StatusNotFound = "404";
static const std::string StatusMethodNotAllowed = "405";
    
static const std::string ReasonPhraseOk = "OK";
static const std::string ReasonPhraseBadRequest = "Bad Request";
static const std::string ReasonPhraseNotFound = "Not Found";
static const std::string ReasonPhraseMethodNotAllowed = "Method Not Allowed";
    
static const std::string HeaderConnection = "Connection";
static const std::string HeaderContentLength = "Content-Length";
static const std::string HeaderContentType = "Content-Type";
static const std::string HeaderExpires = "Expires";
static const std::string HeaderHost = "Host";
static const std::string HeaderLastModified = "Last-Modified";
static const std::string HeaderServer = "Server";
static const std::string HeaderUserAgent = "User-Agent";

///////////////////////////////////////////////////////////////////////////////
    
static MethodTypeMap_t MethodTypeMap;
static MethodStringMap_t MethodStringMap;

static VersionStringMap_t VersionStringMap;

static StatusCodeMap_t StatusCodeMap;

static ReasonPhraseStringMap_t ReasonPhraseStringMap;
    
static HeaderTypeMap_t HeaderTypeMap;
static HeaderStringMap_t HeaderStringMap;
    
static bool _Init = false;
    
///////////////////////////////////////////////////////////////////////////////

void Init(void)
{
    if (_Init)
        return;
    
    MethodTypeMap[MethodOptions] = HTTP_METHOD_OPTIONS;
    MethodTypeMap[MethodGet]     = HTTP_METHOD_GET;
    MethodTypeMap[MethodHead]    = HTTP_METHOD_HEAD;
    MethodTypeMap[MethodPost]    = HTTP_METHOD_POST;
    MethodTypeMap[MethodPut]     = HTTP_METHOD_PUT;
    MethodTypeMap[MethodDelete]  = HTTP_METHOD_DELETE;
    MethodTypeMap[MethodTrace]   = HTTP_METHOD_TRACE;
    MethodTypeMap[MethodConnect] = HTTP_METHOD_CONNECT;

    MethodStringMap[HTTP_METHOD_OPTIONS] = MethodOptions;
    MethodStringMap[HTTP_METHOD_GET]     = MethodGet;
    MethodStringMap[HTTP_METHOD_HEAD]    = MethodHead;
    MethodStringMap[HTTP_METHOD_POST]    = MethodPost;
    MethodStringMap[HTTP_METHOD_PUT]     = MethodPut;
    MethodStringMap[HTTP_METHOD_DELETE]  = MethodDelete;
    MethodStringMap[HTTP_METHOD_TRACE]   = MethodTrace;
    MethodStringMap[HTTP_METHOD_CONNECT] = MethodConnect;

    VersionStringMap[HTTP_VER_0_9] = Version_0_9;
    VersionStringMap[HTTP_VER_1_0] = Version_1_0;
    VersionStringMap[HTTP_VER_1_1] = Version_1_1;

    StatusCodeMap[StatusOk]               = HTTP_STATUS_OK;
    StatusCodeMap[StatusBadRequest]       = HTTP_STATUS_BAD_REQUEST;
    StatusCodeMap[StatusNotFound]         = HTTP_STATUS_NOT_FOUND;
    StatusCodeMap[StatusMethodNotAllowed] = HTTP_STATUS_METHOD_NOT_ALLOWED;
    
    ReasonPhraseStringMap[HTTP_STATUS_OK]                 = ReasonPhraseOk;
    ReasonPhraseStringMap[HTTP_STATUS_BAD_REQUEST]        = ReasonPhraseBadRequest;
    ReasonPhraseStringMap[HTTP_STATUS_NOT_FOUND]          = ReasonPhraseNotFound;
    ReasonPhraseStringMap[HTTP_STATUS_METHOD_NOT_ALLOWED] = ReasonPhraseMethodNotAllowed;
    
    HeaderTypeMap[HeaderConnection]    = HTTP_HEADER_CONNECTION;
    HeaderTypeMap[HeaderContentLength] = HTTP_HEADER_CONTENT_LENGTH;
    HeaderTypeMap[HeaderContentType]   = HTTP_HEADER_CONTENT_TYPE;
    HeaderTypeMap[HeaderExpires]       = HTTP_HEADER_EXPIRES;
    HeaderTypeMap[HeaderHost]          = HTTP_HEADER_HOST;
    HeaderTypeMap[HeaderLastModified]  = HTTP_HEADER_LAST_MODIFIED;
    HeaderTypeMap[HeaderServer]        = HTTP_HEADER_SERVER;
    HeaderTypeMap[HeaderUserAgent]     = HTTP_HEADER_USER_AGENT;

    HeaderStringMap[HTTP_HEADER_CONNECTION]     = HeaderConnection;
    HeaderStringMap[HTTP_HEADER_CONTENT_LENGTH] = HeaderContentLength;
    HeaderStringMap[HTTP_HEADER_CONTENT_TYPE]   = HeaderContentType;
    HeaderStringMap[HTTP_HEADER_EXPIRES]        = HeaderExpires;
    HeaderStringMap[HTTP_HEADER_HOST]           = HeaderHost;
    HeaderStringMap[HTTP_HEADER_LAST_MODIFIED]  = HeaderLastModified;
    HeaderStringMap[HTTP_HEADER_SERVER]         = HeaderServer;
    HeaderStringMap[HTTP_HEADER_USER_AGENT]     = HeaderUserAgent;
    
    _Init = true;
}
    
///////////////////////////////////////////////////////////////////////////////
    
struct AssignMethodType : public std::binary_function<L4L7_UTILS_NS::MessageConstIterator, L4L7_UTILS_NS::MessageConstIterator, void>
{
    AssignMethodType(MethodType &theMethod) : method(theMethod) { }
    
    void operator()(const L4L7_UTILS_NS::MessageConstIterator& begin, const L4L7_UTILS_NS::MessageConstIterator& end) const
    {
        MethodTypeMap_t::const_iterator iter = MethodTypeMap.find(std::string(begin, end));
        method = (iter == MethodTypeMap.end()) ? HTTP_METHOD_UNKNOWN : iter->second;
    }

    MethodType &method;
};
    
///////////////////////////////////////////////////////////////////////////////
    
struct AssignStatusCode : public std::binary_function<L4L7_UTILS_NS::MessageConstIterator, L4L7_UTILS_NS::MessageConstIterator, void>
{
    AssignStatusCode(StatusCode &theStatusCode) : statusCode(theStatusCode) { }
    
    void operator()(const L4L7_UTILS_NS::MessageConstIterator& begin, const L4L7_UTILS_NS::MessageConstIterator& end) const
    {
        StatusCodeMap_t::const_iterator iter = StatusCodeMap.find(std::string(begin, end));
        statusCode = (iter == StatusCodeMap.end()) ? HTTP_STATUS_UNKNOWN : iter->second;
    }

    StatusCode &statusCode;
};
    
///////////////////////////////////////////////////////////////////////////////
    
struct AssignHeaderType : public std::binary_function<L4L7_UTILS_NS::MessageConstIterator, L4L7_UTILS_NS::MessageConstIterator, void>
{
    AssignHeaderType(HeaderType &theHeader) : header(theHeader) { }
    
    void operator()(const L4L7_UTILS_NS::MessageConstIterator& begin, const L4L7_UTILS_NS::MessageConstIterator& end) const
    {
        HeaderTypeMap_t::const_iterator iter = HeaderTypeMap.find(std::string(begin, end));
        header = (iter == HeaderTypeMap.end()) ? HTTP_HEADER_UNKNOWN : iter->second;
    }

    HeaderType &header;
};

///////////////////////////////////////////////////////////////////////////////

const std::string BuildRequestLine(MethodType method, const std::string& uri, Version version)
{
    if (!_Init)
        Init();

    MethodStringMap_t::const_iterator methodIter = MethodStringMap.find(method);
    if (methodIter == MethodStringMap.end())
        throw EPHXInternal();
    
    VersionStringMap_t::const_iterator versionIter = VersionStringMap.find(version);
    if (versionIter == VersionStringMap.end())
        throw EPHXInternal();

    std::ostringstream oss;
    oss << methodIter->second << " " << uri << " " << versionIter->second << "\r\n";
    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

bool ParseRequestLine(L4L7_UTILS_NS::MessageConstIterator begin, L4L7_UTILS_NS::MessageConstIterator end, MethodType& method, std::string& uri, Version& version)
{
    if (!_Init)
        Init();

    method = HTTP_METHOD_BLANK_LINE;
    uri.clear();
    version = HTTP_VER_UNKNOWN;
    
    // Note the HTTP major/minor numbers are parsed as uints rather than digits because they are allowed to have leading zeroes
    unsigned int major = 0, minor = 0;
    const bool success = parse(begin, end,

                               // Begin grammar
                               (
                                   (
                                       (+alpha_p)[AssignMethodType(method)]
                                       >> space_p
                                       >> (+(~space_p))[assign_a(uri)]
                                       >> space_p
                                       >> str_p("HTTP/")
                                       >> uint_p[assign_a(major)]
                                       >> ch_p('.')
                                       >> uint_p[assign_a(minor)]
                                       >> *eol_p
                                   ) | (*eol_p)
                               )
                               // End grammar
                               
                               ).full;

    if (success && method != HTTP_METHOD_BLANK_LINE)
    {
        if (major == 1 && minor == 1)
            version = HTTP_VER_1_1;
        else if (major == 1 && minor == 0)
            version = HTTP_VER_1_0;
        else if (major == 0 && minor == 9)
            version = HTTP_VER_0_9;
    }
    
    return success;
}

///////////////////////////////////////////////////////////////////////////////

const std::string BuildStatusLine(Version version, StatusCode statusCode)
{
    if (!_Init)
        Init();

    VersionStringMap_t::const_iterator versionIter = VersionStringMap.find(version);
    if (versionIter == VersionStringMap.end())
        throw EPHXInternal();

    ReasonPhraseStringMap_t::const_iterator reasonPhraseIter = ReasonPhraseStringMap.find(statusCode);
    if (reasonPhraseIter == ReasonPhraseStringMap.end())
        throw EPHXInternal();
    
    std::ostringstream oss;
    oss << versionIter->second << " " << statusCode << " " << reasonPhraseIter->second << "\r\n";
    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

bool ParseStatusLine(L4L7_UTILS_NS::MessageConstIterator begin, L4L7_UTILS_NS::MessageConstIterator end, Version& version, StatusCode& statusCode)
{
    if (!_Init)
        Init();

    version = HTTP_VER_UNKNOWN;
    statusCode = HTTP_STATUS_UNKNOWN;

    // Note the HTTP major/minor numbers are parsed as uints rather than digits because they are allowed to have leading zeroes
    unsigned int major = 0, minor = 0;
    const bool success = parse(begin, end,

                               // Begin grammar
                               (
                                   str_p("HTTP/")
                                   >> uint_p[assign_a(major)]
                                   >> ch_p('.')
                                   >> uint_p[assign_a(minor)]
                                   >> space_p
                                   >> (repeat_p(3))[digit_p][AssignStatusCode(statusCode)]
                                   >> space_p
                                   >> *(anychar_p - eol_p)
                                   >> *eol_p
                               )
                               // End grammar
                               
                               ).full;
    
    if (success)
    {
        if (major == 1 && minor == 1)
            version = HTTP_VER_1_1;
        else if (major == 1 && minor == 0)
            version = HTTP_VER_1_0;
        else if (major == 0 && minor == 9)
            version = HTTP_VER_0_9;
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////

const std::string BuildHeaderLine(HeaderType header, const std::string& value)
{
    if (!_Init)
        Init();

    HeaderStringMap_t::const_iterator headerIter = HeaderStringMap.find(header);
    if (headerIter == HeaderStringMap.end())
        throw EPHXInternal();

    std::ostringstream oss;
    oss << headerIter->second << ": " << value << "\r\n";
    return oss.str();
}
    
///////////////////////////////////////////////////////////////////////////////

const std::string BuildHeaderLine(HeaderType header, const ACE_Time_Value& value)
{
    if (!_Init)
        Init();

    HeaderStringMap_t::const_iterator headerIter = HeaderStringMap.find(header);
    if (headerIter == HeaderStringMap.end())
        throw EPHXInternal();

    const time_t sec = value.sec();
    struct tm tm;
    char tmStr[64];
    ACE_OS::gmtime_r(&sec, &tm);
    ACE_OS::strftime(tmStr, 64, "%a, %d %b %Y %H:%M:%S GMT", &tm);
    
    std::ostringstream oss;
    oss << headerIter->second << ": " << tmStr << "\r\n";
    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

const std::string BuildHeaderLine(HeaderType header, uint32_t value)
{
    if (!_Init)
        Init();

    HeaderStringMap_t::const_iterator headerIter = HeaderStringMap.find(header);
    if (headerIter == HeaderStringMap.end())
        throw EPHXInternal();

    std::ostringstream oss;
    oss << headerIter->second << ": " << value << "\r\n";
    return oss.str();
}
    
///////////////////////////////////////////////////////////////////////////////
    
bool ParseHeaderLine(L4L7_UTILS_NS::MessageConstIterator begin, L4L7_UTILS_NS::MessageConstIterator end, HeaderType& header, std::string& value)
{
    if (!_Init)
        Init();

    header = HTTP_HEADER_BLANK_LINE;
    value.clear();
    
    const bool success = parse(begin, end,

                               // Begin grammar
                               (
                                   (
                                       (+(alpha_p | ch_p('-')))[AssignHeaderType(header)]
                                       >> ch_p(':')
                                       >> *(space_p | ch_p('\t'))
                                       >> (*(anychar_p - eol_p))[assign_a(value)]
                                       >> *eol_p
                                   ) | (*eol_p)
                               )
                               // End grammar
                               
                               ).full;
    return success;
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace HttpProtocol

END_DECL_HTTP_NS

