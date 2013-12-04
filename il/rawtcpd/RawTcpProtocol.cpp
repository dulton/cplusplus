/// @file
/// @brief Raw tcp protocol implementation
///
///  Copyright (c) 2009 by Spirent Communications Inc.
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

#include "RawTcpProtocol.h"

using namespace boost::spirit::classic;

DECL_RAWTCP_NS

namespace RawTcpProtocol
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

typedef boost::unordered_map<std::string, HeaderType, NoCaseHashString, NoCaseCompareString> HeaderTypeMap_t;
typedef boost::unordered_map<HeaderType, std::string> HeaderStringMap_t;

///////////////////////////////////////////////////////////////////////////////

static const std::string MethodRequest = "SPIRENT_RAW_TCP_CLIENT_REQ";
static const std::string MethodResponse = "SPIRENT_RAW_TCP_SERVER_RESP";

static const std::string HeaderLength = "Length";
static const std::string HeaderConnection = "Connection";


///////////////////////////////////////////////////////////////////////////////

static MethodTypeMap_t MethodTypeMap;
static MethodStringMap_t MethodStringMap;

static HeaderTypeMap_t HeaderTypeMap;
static HeaderStringMap_t HeaderStringMap;

static bool _Init = false;

///////////////////////////////////////////////////////////////////////////////

void Init(void)
{
    if (_Init)
        return;

    MethodTypeMap[MethodRequest]  = RAWTCP_METHOD_REQUEST;
    MethodTypeMap[MethodResponse] = RAWTCP_METHOD_RESPONSE;

    MethodStringMap[RAWTCP_METHOD_REQUEST]  = MethodRequest;
    MethodStringMap[RAWTCP_METHOD_RESPONSE] = MethodResponse;

    HeaderTypeMap[HeaderLength] = RAWTCP_HEADER_LENGTH;
    HeaderTypeMap[HeaderConnection] = RAWTCP_HEADER_CONNECTION;

    HeaderStringMap[RAWTCP_HEADER_LENGTH] = HeaderLength;
    HeaderStringMap[RAWTCP_HEADER_CONNECTION] = HeaderConnection;

    _Init = true;
}
    
///////////////////////////////////////////////////////////////////////////////
    
struct AssignMethodType : public std::binary_function<L4L7_UTILS_NS::MessageConstIterator, L4L7_UTILS_NS::MessageConstIterator, void>
{
    AssignMethodType(MethodType &theMethod) : method(theMethod) { }
    
    void operator()(const L4L7_UTILS_NS::MessageConstIterator& begin, const L4L7_UTILS_NS::MessageConstIterator& end) const
    {
        MethodTypeMap_t::const_iterator iter = MethodTypeMap.find(std::string(begin, end));
        method = (iter == MethodTypeMap.end()) ? RAWTCP_METHOD_UNKNOWN : iter->second;
    }

    MethodType &method;
};
    
///////////////////////////////////////////////////////////////////////////////
    
struct AssignHeaderType : public std::binary_function<L4L7_UTILS_NS::MessageConstIterator, L4L7_UTILS_NS::MessageConstIterator, void>
{
    AssignHeaderType(HeaderType &theHeader) : header(theHeader) { }
    
    void operator()(const L4L7_UTILS_NS::MessageConstIterator& begin, const L4L7_UTILS_NS::MessageConstIterator& end) const
    {
        HeaderTypeMap_t::const_iterator iter = HeaderTypeMap.find(std::string(begin, end));
        header = (iter == HeaderTypeMap.end()) ? RAWTCP_HEADER_UNKNOWN : iter->second;
    }

    HeaderType &header;
};

///////////////////////////////////////////////////////////////////////////////

const std::string BuildRequestLine( uint32_t requestNumber)
{
    if (!_Init)
        Init();

    std::ostringstream oss;
    oss << MethodRequest << " " << requestNumber << "\r\n";
    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

bool ParseRequestLine(L4L7_UTILS_NS::MessageConstIterator begin, L4L7_UTILS_NS::MessageConstIterator end, MethodType& method, uint32_t& requestNum)
{
    if (!_Init)
        Init();

    method = RAWTCP_METHOD_BLANK_LINE;

    const bool success = parse(begin, end,

                               // Begin grammar
                               (
                                  (
                                      (+(alpha_p | ch_p('_')))[AssignMethodType(method)]
                                      >> space_p
                                      >> uint_p[assign_a(requestNum)]
                                      >> *eol_p
                                  ) | (*eol_p)
                               )
                               // End grammar
                               ).full;
    return success;
}

///////////////////////////////////////////////////////////////////////////////

const std::string BuildStatusLine(MethodType methodType, uint32_t responseNum)
{
    if (!_Init)
        Init();
    MethodStringMap_t::const_iterator methodIter = MethodStringMap.find(methodType);
    if (methodIter == MethodStringMap.end())
        throw EPHXInternal();

    std::ostringstream oss;
    oss << methodIter->second << " " << responseNum << "\r\n";
    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

bool ParseStatusLine(L4L7_UTILS_NS::MessageConstIterator begin, L4L7_UTILS_NS::MessageConstIterator end, uint32_t& responseNum)
{
    if (!_Init)
        Init();

    const bool success = parse(begin, end,

                               // Begin grammar
                               (
                                   str_p(MethodResponse.c_str())
                                   >> space_p
                                   >> uint_p[assign_a(responseNum)]
                                   >> *eol_p
                               )
                               // End grammar

                               ).full;

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

    header = RAWTCP_HEADER_BLANK_LINE;
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

}  // namespace RawTcpProtocol

END_DECL_RAWTCP_NS

