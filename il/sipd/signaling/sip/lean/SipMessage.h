/// @file
/// @brief SIP message header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _SIP_MESSAGE_H_
#define _SIP_MESSAGE_H_

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

#include <ace/INET_Addr.h>
#include <boost/foreach.hpp>
#include <boost/function.hpp>

#include "SipCommon.h"
#include "SipUri.h"

DECL_SIPLEAN_NS

///////////////////////////////////////////////////////////////////////////////

struct SipMessage
{
    // Enumerated protocol constants
    enum Type
    {
        SIP_MSG_UNKNOWN,
        SIP_MSG_REQUEST,
        SIP_MSG_STATUS
    };
        
    enum Version
    {
        SIP_VER_UNKNOWN = 0,
        SIP_VER_2_0
    };

    enum MethodType
    {
        SIP_METHOD_UNKNOWN = 0,
        SIP_METHOD_REGISTER,
        SIP_METHOD_INVITE,
        SIP_METHOD_PRACK,
        SIP_METHOD_ACK,
        SIP_METHOD_CANCEL,
        SIP_METHOD_BYE,
        SIP_METHOD_UPDATE,
        SIP_METHOD_OPTIONS
    };

    enum StatusCodeCategory
    {
        SIP_STATUS_PROVISIONAL = 1,
        SIP_STATUS_SUCCESS = 2,
        SIP_STATUS_REDIRECTION = 3,
        SIP_STATUS_CLIENT_ERROR = 4,
        SIP_STATUS_SERVER_ERROR = 5,
        SIP_STATUS_GLOBAL_FAILURE = 6
    };
            
    enum HeaderType
    {
        SIP_HEADER_UNKNOWN = 0,
        SIP_HEADER_VIA,
        SIP_HEADER_FROM,
        SIP_HEADER_TO,
        SIP_HEADER_CALL_ID,
        SIP_HEADER_CSEQ,
        SIP_HEADER_MAX_FORWARDS,
        SIP_HEADER_EXPIRES,
        SIP_HEADER_ROUTE,
        SIP_HEADER_RECORD_ROUTE,
        SIP_HEADER_SUBJECT,
        SIP_HEADER_CONTACT,
        SIP_HEADER_USER_AGENT,
        SIP_HEADER_CONTENT_TYPE,
        SIP_HEADER_CONTENT_LENGTH,
        SIP_HEADER_WWW_AUTHENTICATE,
        SIP_HEADER_AUTHORIZATION,
        SIP_HEADER_PROXY_AUTHENTICATE,
        SIP_HEADER_PROXY_AUTHORIZATION,
        SIP_HEADER_ALLOW,
        SIP_HEADER_REQUIRE,
        SIP_HEADER_SUPPORTED,
        SIP_HEADER_RSEQ,
        SIP_HEADER_RACK,
        SIP_HEADER_SESSION_EXPIRES,
        SIP_HEADER_MIN_SE,
        SIP_HEADER_PATH,
        SIP_HEADER_SERVICE_ROUTE,
        SIP_HEADER_P_ASSOCIATED_URI,
        SIP_HEADER_P_PREFERRED_IDENTITY,
        SIP_HEADER_X_PASSWORD_DEBUG
    };

    // Message components
    struct Request
    {
        SIP_NS::SipUri uri;
        Version version;
    };

    struct Status
    {
        const StatusCodeCategory Category(void) const 
        {
            return static_cast<StatusCodeCategory>(code / 100);
        }
                
        Version version;
        uint16_t code;
        std::string message;
    };
            
    struct HeaderField
    {
        HeaderField(HeaderType theType, const std::string& theValue)
            : type(theType),
              value(theValue)
        {
        }
        
        HeaderType type;
        std::string value;
    };

    // Message methods
    SipMessage(void) 
      : transactionId(0),
        type(SIP_MSG_UNKNOWN),
        method(SIP_METHOD_UNKNOWN),
        compactHeaders(false),
        seq(0)
    {
        headers.reserve(NUM_RESERVED_HEADERS);
    }

    // Functors
    struct PushHeader : public std::unary_function<HeaderField, void>
    {
        PushHeader(SipMessage &msg) : mMsg(msg) { }
        
        void operator()(const HeaderField& f) const { mMsg.headers.push_back(f); }
        void operator()(const HeaderField *f) const { mMsg.headers.push_back(*f); }

      private:
        SipMessage& mMsg;
    };
    
    struct IsHeaderType : public std::unary_function<HeaderField, bool>
    {
        IsHeaderType(HeaderType type) : mType(type) { }
        bool operator()(const HeaderField& f) const { return f.type == mType; }

      private:
        const HeaderType mType;
    };

    // Utility methods
    HeaderField *FindHeader(HeaderType type)
    {
        std::vector<HeaderField>::iterator iter = find_if(headers.begin(), headers.end(), IsHeaderType(type));
        return (iter != headers.end()) ? iter.operator->() : 0;
    }

    const HeaderField *FindHeader(HeaderType type) const
    {
        std::vector<HeaderField>::const_iterator iter = find_if(headers.begin(), headers.end(), IsHeaderType(type));
        return (iter != headers.end()) ? iter.operator->() : 0;
    }

    void ForEachHeader(HeaderType type, boost::function1<void, const HeaderField&> f) const
    {
        BOOST_FOREACH(const HeaderField& header, headers)
        {
            if (header.type == type)
                f(header);
        }
    }

    // Class data
    static const size_t NUM_RESERVED_HEADERS = 16;

    // Message meta-data
    uint32_t transactionId;                     //< tx/rx: opaque transaction id for SipTransactionLayer
    ACE_INET_Addr peer;                         //< tx/rx: peer IP address
    Type type;                                  //< tx/rx: request or response/status?
    MethodType method;                          //< tx/rx: method
    bool compactHeaders;                        //< tx: use compact headers?
    SIP_NS::SipUri fromUri;                             //< rx: From URI
    std::string fromTag;                        //< tx/rx: From tag
    SIP_NS::SipUri toUri;                               //< rx: To URI
    std::string toTag;                          //< tx/rx: To tag
    std::string callId;                         //< rx: dialog Call-ID
    uint32_t seq;                               //< tx/rx: dialog CSeq number
    
    // Message data (FUTURE: request/status should be merged into a boost::variant)
    Request request;                            //< request data (type == SIP_MSG_REQUEST)
    Status status;                              //< status data (type == SIP_MSG_STATUS)
    std::vector<HeaderField> headers;           //< header data
    std::string body;                           //< message body (SDP payload)
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_SIPLEAN_NS

#endif
