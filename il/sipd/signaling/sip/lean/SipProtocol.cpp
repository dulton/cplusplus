/// @file
/// @brief SIP protocol implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <algorithm>
#include <functional>
#include <iterator>
#include <sstream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/format.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_chset.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_lists.hpp>
#include <boost/unordered_map.hpp>
#include <string_token_iterator.h>
#include <vif/Packet.h>

#include "SdpMessage.h"
#include "SipleanAuthentication.h"
#include "SipProtocol.h"
#include "SipUri.h"
#include "SipUtils.h"

#include "VoIPLog.h"
#include "VoIPUtils.h"

using namespace boost::spirit::classic;

DECL_SIPLEAN_NS

namespace SipProtocol
{

///////////////////////////////////////////////////////////////////////////////

struct StaticData
{
    StaticData(void);

    const std::string SipUriScheme;
    const std::string TelUriScheme;
    const std::string CrLf;
    const std::string Sip2dot0;

    const std::string MethodRegister;
    const std::string MethodInvite;
    const std::string MethodPrAck;
    const std::string MethodAck;
    const std::string MethodCancel;
    const std::string MethodBye;
    const std::string MethodUpdate;
    const std::string MethodOptions;

    const std::string HeaderVia;
    const std::string HeaderViaCompact;
    const std::string HeaderFrom;
    const std::string HeaderFromCompact;
    const std::string HeaderTo;
    const std::string HeaderToCompact;
    const std::string HeaderCallId;
    const std::string HeaderCallIdCompact;
    const std::string HeaderCSeq;
    const std::string HeaderMaxForwards;
    const std::string HeaderExpires;
    const std::string HeaderRoute;
    const std::string HeaderRecordRoute;
    const std::string HeaderSubject;
    const std::string HeaderSubjectCompact;
    const std::string HeaderContact;
    const std::string HeaderContactCompact;
    const std::string HeaderUserAgent;
    const std::string HeaderContentType;
    const std::string HeaderContentTypeCompact;
    const std::string HeaderContentLength;
    const std::string HeaderContentLengthCompact;
    const std::string HeaderWWWAuthenticate;
    const std::string HeaderAuthorization;
    const std::string HeaderProxyAuthenticate;
    const std::string HeaderProxyAuthorization;
    const std::string HeaderAllow;
    const std::string HeaderRequire;
    const std::string HeaderSupported;
    const std::string HeaderSupportedCompact;
    const std::string HeaderRSeq;
    const std::string HeaderRAck;
    const std::string HeaderSessionExpires;
    const std::string HeaderSessionExpiresCompact;
    const std::string HeaderMinSE;
    const std::string HeaderPath;
    const std::string HeaderServiceRoute;
    const std::string HeaderPAssociatedUri;
    const std::string HeaderPPreferredIdentity;
    const std::string HeaderXPasswordDebug;

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

    typedef boost::unordered_map<std::string, SipMessage::MethodType, NoCaseHashString, NoCaseCompareString> MethodTypeMap_t;
    typedef boost::unordered_map<SipMessage::MethodType, std::string> MethodStringMap_t;

    typedef boost::unordered_map<std::string, SipMessage::HeaderType, NoCaseHashString, NoCaseCompareString> HeaderTypeMap_t;
    typedef boost::unordered_map<SipMessage::HeaderType, std::string> HeaderStringMap_t;

    MethodTypeMap_t MethodTypeMap;
    MethodStringMap_t MethodStringMap;
    HeaderTypeMap_t HeaderTypeMap;
    HeaderStringMap_t HeaderStringMap;
    HeaderStringMap_t CompactHeaderStringMap;
};

///////////////////////////////////////////////////////////////////////////////

StaticData::StaticData(void)
    : SipUriScheme("sip"),
      TelUriScheme("tel"),
      CrLf("\r\n"),
      Sip2dot0("SIP/2.0"),
      MethodRegister("REGISTER"),
      MethodInvite("INVITE"),
      MethodPrAck("PRACK"),
      MethodAck("ACK"),
      MethodCancel("CANCEL"),
      MethodBye("BYE"),
      MethodUpdate("UPDATE"),
      MethodOptions("OPTIONS"),
      HeaderVia("Via"),
      HeaderViaCompact("v"),
      HeaderFrom("From"),
      HeaderFromCompact("f"),
      HeaderTo("To"),
      HeaderToCompact("t"),
      HeaderCallId("Call-ID"),
      HeaderCallIdCompact("i"),
      HeaderCSeq("CSeq"),
      HeaderMaxForwards("Max-Forwards"),
      HeaderExpires("Expires"),
      HeaderRoute("Route"),
      HeaderRecordRoute("Record-Route"),
      HeaderSubject("Subject"),
      HeaderSubjectCompact("s"),
      HeaderContact("Contact"),
      HeaderContactCompact("m"),
      HeaderUserAgent("User-Agent"),
      HeaderContentType("Content-Type"),
      HeaderContentTypeCompact("c"),
      HeaderContentLength("Content-Length"),
      HeaderContentLengthCompact("l"),
      HeaderWWWAuthenticate("WWW-Authenticate"),
      HeaderAuthorization("Authorization"),
      HeaderProxyAuthenticate("Proxy-Authenticate"),
      HeaderProxyAuthorization("Proxy-Authorization"),
      HeaderAllow("Allow"),
      HeaderRequire("Require"),
      HeaderSupported("Supported"),
      HeaderSupportedCompact("k"),
      HeaderRSeq("RSeq"),
      HeaderRAck("RAck"),
      HeaderSessionExpires("Session-Expires"),
      HeaderSessionExpiresCompact("x"),
      HeaderMinSE("Min-SE"),
      HeaderPath("Path"),
      HeaderServiceRoute("Service-Route"),
      HeaderPAssociatedUri("P-Associated-URI"),
      HeaderPPreferredIdentity("P-Preferred-Identity"),
      HeaderXPasswordDebug("X-Password-Debug")
{
    MethodTypeMap = boost::assign::map_list_of(MethodRegister, SipMessage::SIP_METHOD_REGISTER)
        (MethodInvite, SipMessage::SIP_METHOD_INVITE)
        (MethodPrAck, SipMessage::SIP_METHOD_PRACK)
        (MethodAck, SipMessage::SIP_METHOD_ACK)
        (MethodCancel, SipMessage::SIP_METHOD_CANCEL)
        (MethodBye, SipMessage::SIP_METHOD_BYE)
        (MethodUpdate, SipMessage::SIP_METHOD_UPDATE)
        (MethodOptions, SipMessage::SIP_METHOD_OPTIONS);

    MethodStringMap = boost::assign::map_list_of(SipMessage::SIP_METHOD_REGISTER, MethodRegister)
        (SipMessage::SIP_METHOD_INVITE, MethodInvite)
        (SipMessage::SIP_METHOD_PRACK, MethodPrAck)
        (SipMessage::SIP_METHOD_ACK, MethodAck)
        (SipMessage::SIP_METHOD_CANCEL, MethodCancel)
        (SipMessage::SIP_METHOD_BYE, MethodBye)
        (SipMessage::SIP_METHOD_UPDATE, MethodUpdate)
        (SipMessage::SIP_METHOD_OPTIONS, MethodOptions);

    HeaderTypeMap = boost::assign::map_list_of(HeaderVia, SipMessage::SIP_HEADER_VIA)
        (HeaderViaCompact, SipMessage::SIP_HEADER_VIA)
        (HeaderFrom, SipMessage::SIP_HEADER_FROM)
        (HeaderFromCompact, SipMessage::SIP_HEADER_FROM)
        (HeaderTo, SipMessage::SIP_HEADER_TO)
        (HeaderToCompact, SipMessage::SIP_HEADER_TO)
        (HeaderCallId, SipMessage::SIP_HEADER_CALL_ID)
        (HeaderCallIdCompact, SipMessage::SIP_HEADER_CALL_ID)
        (HeaderCSeq, SipMessage::SIP_HEADER_CSEQ)
        (HeaderMaxForwards, SipMessage::SIP_HEADER_MAX_FORWARDS)
        (HeaderExpires, SipMessage::SIP_HEADER_EXPIRES)
        (HeaderRoute, SipMessage::SIP_HEADER_ROUTE)
        (HeaderRecordRoute, SipMessage::SIP_HEADER_RECORD_ROUTE)
        (HeaderSubject, SipMessage::SIP_HEADER_SUBJECT)
        (HeaderSubjectCompact, SipMessage::SIP_HEADER_SUBJECT)
        (HeaderContact, SipMessage::SIP_HEADER_CONTACT)
        (HeaderContactCompact, SipMessage::SIP_HEADER_CONTACT)
        (HeaderUserAgent, SipMessage::SIP_HEADER_USER_AGENT)
        (HeaderContentType, SipMessage::SIP_HEADER_CONTENT_TYPE)
        (HeaderContentTypeCompact, SipMessage::SIP_HEADER_CONTENT_TYPE)
        (HeaderContentLength, SipMessage::SIP_HEADER_CONTENT_LENGTH)
        (HeaderContentLengthCompact, SipMessage::SIP_HEADER_CONTENT_LENGTH)
        (HeaderWWWAuthenticate, SipMessage::SIP_HEADER_WWW_AUTHENTICATE)
        (HeaderAuthorization, SipMessage::SIP_HEADER_AUTHORIZATION)
        (HeaderProxyAuthenticate, SipMessage::SIP_HEADER_PROXY_AUTHENTICATE)
        (HeaderProxyAuthorization, SipMessage::SIP_HEADER_PROXY_AUTHORIZATION)
        (HeaderAllow, SipMessage::SIP_HEADER_ALLOW)
        (HeaderRequire, SipMessage::SIP_HEADER_REQUIRE)
        (HeaderSupported, SipMessage::SIP_HEADER_SUPPORTED)
        (HeaderSupportedCompact, SipMessage::SIP_HEADER_SUPPORTED)
        (HeaderRSeq, SipMessage::SIP_HEADER_RSEQ)
        (HeaderRAck, SipMessage::SIP_HEADER_RACK)
        (HeaderSessionExpires, SipMessage::SIP_HEADER_SESSION_EXPIRES)
        (HeaderSessionExpiresCompact, SipMessage::SIP_HEADER_SESSION_EXPIRES)
        (HeaderMinSE, SipMessage::SIP_HEADER_MIN_SE)
        (HeaderPath, SipMessage::SIP_HEADER_PATH)
        (HeaderServiceRoute, SipMessage::SIP_HEADER_SERVICE_ROUTE)
        (HeaderPAssociatedUri, SipMessage::SIP_HEADER_P_ASSOCIATED_URI)
        (HeaderPPreferredIdentity, SipMessage::SIP_HEADER_P_PREFERRED_IDENTITY)
        (HeaderXPasswordDebug, SipMessage::SIP_HEADER_X_PASSWORD_DEBUG);

    HeaderStringMap = boost::assign::map_list_of(SipMessage::SIP_HEADER_VIA, HeaderVia)
        (SipMessage::SIP_HEADER_FROM, HeaderFrom)
        (SipMessage::SIP_HEADER_TO, HeaderTo)
        (SipMessage::SIP_HEADER_CALL_ID, HeaderCallId)
        (SipMessage::SIP_HEADER_CSEQ, HeaderCSeq)
        (SipMessage::SIP_HEADER_MAX_FORWARDS, HeaderMaxForwards)
        (SipMessage::SIP_HEADER_EXPIRES, HeaderExpires)
        (SipMessage::SIP_HEADER_ROUTE, HeaderRoute)
        (SipMessage::SIP_HEADER_RECORD_ROUTE, HeaderRecordRoute)
        (SipMessage::SIP_HEADER_SUBJECT, HeaderSubject)
        (SipMessage::SIP_HEADER_CONTACT, HeaderContact)
        (SipMessage::SIP_HEADER_USER_AGENT, HeaderUserAgent)
        (SipMessage::SIP_HEADER_CONTENT_TYPE, HeaderContentType)
        (SipMessage::SIP_HEADER_CONTENT_LENGTH, HeaderContentLength)
        (SipMessage::SIP_HEADER_WWW_AUTHENTICATE, HeaderWWWAuthenticate)
        (SipMessage::SIP_HEADER_AUTHORIZATION, HeaderAuthorization)
        (SipMessage::SIP_HEADER_PROXY_AUTHENTICATE, HeaderProxyAuthenticate)
        (SipMessage::SIP_HEADER_PROXY_AUTHORIZATION, HeaderProxyAuthorization)
        (SipMessage::SIP_HEADER_ALLOW, HeaderAllow)
        (SipMessage::SIP_HEADER_REQUIRE, HeaderRequire)
        (SipMessage::SIP_HEADER_SUPPORTED, HeaderSupported)
        (SipMessage::SIP_HEADER_RSEQ, HeaderRSeq)
        (SipMessage::SIP_HEADER_RACK, HeaderRAck)
        (SipMessage::SIP_HEADER_SESSION_EXPIRES, HeaderSessionExpires)
        (SipMessage::SIP_HEADER_MIN_SE, HeaderMinSE)
        (SipMessage::SIP_HEADER_PATH, HeaderPath)
        (SipMessage::SIP_HEADER_SERVICE_ROUTE, HeaderServiceRoute)
        (SipMessage::SIP_HEADER_P_ASSOCIATED_URI, HeaderPAssociatedUri)
        (SipMessage::SIP_HEADER_P_PREFERRED_IDENTITY, HeaderPPreferredIdentity)
        (SipMessage::SIP_HEADER_X_PASSWORD_DEBUG, HeaderXPasswordDebug);

    CompactHeaderStringMap = boost::assign::map_list_of(SipMessage::SIP_HEADER_VIA, HeaderViaCompact)
        (SipMessage::SIP_HEADER_FROM, HeaderFromCompact)
        (SipMessage::SIP_HEADER_TO, HeaderToCompact)
        (SipMessage::SIP_HEADER_CALL_ID, HeaderCallIdCompact)
        (SipMessage::SIP_HEADER_CSEQ, HeaderCSeq)
        (SipMessage::SIP_HEADER_MAX_FORWARDS, HeaderMaxForwards)
        (SipMessage::SIP_HEADER_EXPIRES, HeaderExpires)
        (SipMessage::SIP_HEADER_ROUTE, HeaderRoute)
        (SipMessage::SIP_HEADER_RECORD_ROUTE, HeaderRecordRoute)
        (SipMessage::SIP_HEADER_SUBJECT, HeaderSubjectCompact)
        (SipMessage::SIP_HEADER_CONTACT, HeaderContactCompact)
        (SipMessage::SIP_HEADER_USER_AGENT, HeaderUserAgent)
        (SipMessage::SIP_HEADER_CONTENT_TYPE, HeaderContentTypeCompact)
        (SipMessage::SIP_HEADER_CONTENT_LENGTH, HeaderContentLengthCompact)
        (SipMessage::SIP_HEADER_WWW_AUTHENTICATE, HeaderWWWAuthenticate)
        (SipMessage::SIP_HEADER_AUTHORIZATION, HeaderAuthorization)
        (SipMessage::SIP_HEADER_PROXY_AUTHENTICATE, HeaderProxyAuthenticate)
        (SipMessage::SIP_HEADER_PROXY_AUTHORIZATION, HeaderProxyAuthorization)
        (SipMessage::SIP_HEADER_ALLOW, HeaderAllow)
        (SipMessage::SIP_HEADER_REQUIRE, HeaderRequire)
        (SipMessage::SIP_HEADER_SUPPORTED, HeaderSupportedCompact)
        (SipMessage::SIP_HEADER_PATH, HeaderPath)
        (SipMessage::SIP_HEADER_RSEQ, HeaderRSeq)
        (SipMessage::SIP_HEADER_RACK, HeaderRAck)
        (SipMessage::SIP_HEADER_SESSION_EXPIRES, HeaderSessionExpiresCompact)
        (SipMessage::SIP_HEADER_MIN_SE, HeaderMinSE)
        (SipMessage::SIP_HEADER_SERVICE_ROUTE, HeaderServiceRoute)
        (SipMessage::SIP_HEADER_P_ASSOCIATED_URI, HeaderPAssociatedUri)
        (SipMessage::SIP_HEADER_P_PREFERRED_IDENTITY, HeaderPPreferredIdentity)
        (SipMessage::SIP_HEADER_X_PASSWORD_DEBUG, HeaderXPasswordDebug);
}

///////////////////////////////////////////////////////////////////////////////

StaticData& _StaticData(void)
{
    static StaticData *data = new StaticData;
    return *data;
}

///////////////////////////////////////////////////////////////////////////////

struct LookupMethodType : public std::unary_function<SipMessage::MethodType, std::string>
{
    const std::string operator()(SipMessage::MethodType method) const
    {
        StaticData::MethodStringMap_t::const_iterator methodIter = _StaticData().MethodStringMap.find(method);
        if (methodIter == _StaticData().MethodStringMap.end())
        {
            TC_LOG_ERR_LOCAL(0, LOG_SIP, "[SipProtocol::LookupMethodType] Invalid method type (" << method << ")");
            throw EPHXInternal("SipProtocol::LookupMethodType::operator()");
        }

        return methodIter->second;
    }
};

struct AssignMethodType : public std::binary_function<std::string::const_iterator, std::string::const_iterator, void>
{
    AssignMethodType(SipMessage::MethodType &theMethod) : method(theMethod) { }

    void operator()(const std::string::const_iterator& begin, const std::string::const_iterator& end) const
    {
        StaticData::MethodTypeMap_t::const_iterator methodIter = _StaticData().MethodTypeMap.find(std::string(begin, end));
        method = (methodIter == _StaticData().MethodTypeMap.end()) ? SipMessage::SIP_METHOD_UNKNOWN : methodIter->second;
    }

    SipMessage::MethodType &method;
};

///////////////////////////////////////////////////////////////////////////////

struct HeaderIterator : public std::iterator<std::input_iterator_tag, std::string>
{
  public:
    HeaderIterator() : mStr(0), mSize(0), mStart(0), mEnd(0) { }
    HeaderIterator(const std::string &str) : mStr(&str), mSize(mStr->size()), mStart(0), mEnd(mStr->find(_StaticData().CrLf)) { }
    HeaderIterator(const HeaderIterator& copy) : mStr(copy.mStr), mSize(copy.mSize), mStart(copy.mStart), mEnd(copy.mEnd) { }

    HeaderIterator& operator++()
    {
        FindNext();
        return *this;
    }

    HeaderIterator operator++(int)
    {
        HeaderIterator temp(*this);
        ++(*this);
        return temp;
    }

    std::string operator*() const { return std::string(*mStr, mStart, mEnd - mStart); }
    bool operator==(const HeaderIterator& rhs) const { return (rhs.mStr == mStr && rhs.mStart == mStart && rhs.mEnd == mEnd); }
    bool operator!=(const HeaderIterator& rhs) const { return !(rhs == *this); }

    const std::string::size_type Pos(void) const { return mStart; }

  private:
    void FindNext(void)
    {
        if (mEnd == std::string::npos || ((mSize - mEnd) <= _StaticData().CrLf.size()))
        {
            mStr = 0;
            mStart = mEnd = 0;
            return;
        }

        mStart = mEnd + _StaticData().CrLf.size();
        mEnd = mStr->find(_StaticData().CrLf, mStart);
    }

    const std::string *mStr;
    const std::string::size_type mSize;
    std::string::size_type mStart;
    std::string::size_type mEnd;
};

///////////////////////////////////////////////////////////////////////////////

struct AssignUriScheme : public std::binary_function<std::string::const_iterator, std::string::const_iterator, void>
{
    AssignUriScheme(SipUri::Scheme &theScheme) : scheme(theScheme) { }

    void operator()(const std::string::const_iterator& begin, const std::string::const_iterator& end) const
    {
        const std::string schemeStr(begin, end);

        if (schemeStr == _StaticData().SipUriScheme)
            scheme = SipUri::SIP_URI_SCHEME;
        else if (schemeStr == _StaticData().TelUriScheme)
            scheme = SipUri::TEL_URI_SCHEME;
        else
            scheme = SipUri::NO_URI_SCHEME;
    }

    SipUri::Scheme &scheme;
};

///////////////////////////////////////////////////////////////////////////////

bool ParseUriString(const std::string& uriStr, SipUri& uri)
{
    uri.Reset();

    // Pre-parse to determine URI scheme
    SipUri::Scheme scheme = SipUri::NO_URI_SCHEME;
    if (!parse(uriStr.begin(), uriStr.end(), ((+(alpha_p - ch_p(':')))[AssignUriScheme(scheme)] >> *anychar_p)).full)
        return false;

    if (scheme == SipUri::SIP_URI_SCHEME)
    {
        std::string user, host, transport;
        bool userInfoPresent = false;
        uint16_t port = 0;
        bool lr = false;

        const bool success = parse(uriStr.begin(), uriStr.end(),

                                   // Begin complex grammar
                                   (
                                       // Mandatory scheme
                                       str_p("sip:")

                                       // Optional userinfo
                                       >> !(
                                               (+(print_p - chset_p(":@")))[assign_a(user)]
                                               >> !(ch_p(':') >> (+(print_p - ch_p('@'))))  // optional password
                                               >> ch_p('@')[assign_a(userInfoPresent, true)]
                                           )

                                       // Mandatory host
                                       >> ((+(alnum_p | ch_p('-') | ch_p('.')))[assign_a(host)] | confix_p(ch_p('['), (+(xdigit_p | ch_p(':')))[assign_a(host)], ch_p(']')))

                                       // Optional port
                                       >> !(ch_p(':') >> uint_p[assign_a(port)])

                                       // Optional uri-parameters
                                       >> *(
                                           ch_p(';')
                                           >> (
                                                  (str_p("lr")[assign_a(lr, true)] >> *(~ch_p(';'))) |
						  (str_p("transport=") >> (+(print_p - ch_p(";")))[assign_a(transport)]) |
                                                  *(~ch_p(';'))
                                              )
                                           )
                                   )
                                   // End complex grammar
                               ).full;

        if (success)
        {
            if (userInfoPresent)
                uri.SetSip(user, host, port, transport);
            else
                uri.SetSip(host, port, transport);
            uri.SetLooseRouting(lr);
        }

        return success && !host.empty();
    }
    else if (scheme == SipUri::TEL_URI_SCHEME)
    {
        std::string number="";
	bool isGlobalNumber=false;
	std::string phoneContext="";

        const bool success = parse(uriStr.begin(), uriStr.end(),

                                   // Begin complex grammar
                                   (
                                       // Mandatory scheme
                                       str_p("tel:")

				       >>*space_p

				       //Optional '+' to qualify global number
				       >> !(ch_p('+')[assign_a(isGlobalNumber, true)])

				       >> *space_p

                                       // Mandatory number
                                       >> (+(print_p - ch_p(';')))[assign_a(number)]

				       // Optional phone context
                                       >> !(*space_p >> ch_p(';') >> *space_p >>
					    str_p("phone-context=") >> (+(print_p - ch_p(";")))[assign_a(phoneContext)])
				       
                                       // Optional uri-parameters
                                       >> *(
                                           ch_p(';')
                                           >> (
                                                  *(~ch_p(';'))
                                              )
                                           )
                                   )
                                   // End complex grammar
                               ).full;

	if (success) {
	  if(isGlobalNumber) {
            uri.SetTelGlobal(number);
	  } else {
	    uri.SetTelLocal(number,phoneContext);
	  }
	}
        return success;
    }
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////

const std::string BuildViaString(const SipUri& uri, const std::string *branch)
{
    if (uri.GetScheme() != SipUri::SIP_URI_SCHEME)
        throw EPHXInternal("SipProtocol::BuildViaString");

    std::ostringstream oss;

    oss << _StaticData().Sip2dot0 << "/UDP ";

    const std::string& host = uri.GetSipHost();
    const uint16_t port = uri.GetSipPort();

    if (host.find_first_of(":") != std::string::npos)
        oss << "[" << host << "]";
    else
        oss << host;

    if (port)
        oss << ":" << port;

    if (branch && !branch->empty())
        oss << ";branch=" << *branch;

    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

bool ParseViaString(const std::string& viaStr, SipUri& uri, std::string *branch)
{
    std::string host, branchParam;
    uint16_t port = 0;

    uri.Reset();
    if (branch)
        branch->clear();

    const bool success = parse(viaStr.begin(), viaStr.end(),

                               // Begin complex grammar
                               (
                                   // Mandatory transport protocol
                                   str_p("SIP") >> *space_p >> str_p("/") >> *space_p >> str_p("2.0") >> *space_p >> str_p("/") >> *space_p >> str_p("UDP") >> +space_p

                                   // Mandatory host
                                   >> ((+(alnum_p | ch_p('-') | ch_p('.')))[assign_a(host)] | confix_p(ch_p('['), (+(xdigit_p | ch_p(':')))[assign_a(host)], ch_p(']')))

                                   // Optional port
                                   >> !(*space_p >> ch_p(':') >> *space_p >> uint_p[assign_a(port)])

                                   // Optional via-params
                                   >> *(
                                           *space_p >> ch_p(';') >> *space_p
                                           >> (
                                                  (str_p("branch=") >> (+(print_p - ch_p(";")))[assign_a(branchParam)]) |
                                                  *(~ch_p(';'))
                                              )
                                       )
                               )
                               // End complex grammar

                               ).full;

    if (success)
    {
        uri.SetSip(host, port);
        if (branch)
            branch->assign(branchParam);
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////

const std::string BuildAddressString(const SipUri& uri, const std::string *tag)
{
    std::ostringstream oss;

    switch (uri.GetScheme())
    {
      case SipUri::SIP_URI_SCHEME:
          oss << uri.GetSipUser() << " <" << uri.BuildUriString() << ">";
          break;

      case SipUri::TEL_URI_SCHEME:
          oss << uri.GetTelNumber() << " <" << uri.BuildUriString() << ">";
          break;

      default:
          throw EPHXInternal("SipProtocol::BuildAddressString");
    }

    if (tag && !tag->empty())
        oss << ";tag=" << *tag;

    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

bool ParseAddressString(const std::string& nameAddrStr, SipUri *uri, std::string *tag)
{
    std::string uriStr;
    std::string tagParam;

    bool success = parse(nameAddrStr.begin(), nameAddrStr.end(),

                         // Begin complex grammar
                         (
                             // Optional display name
                             *(print_p - ch_p('<'))

                             // Mandatory URI
                             >> confix_p(ch_p('<'), (+print_p)[assign_a(uriStr)], ch_p('>'))

                             // Optional tag-param
                             >> *(
                                     *space_p >> ch_p(';') >> *space_p
                                     >> (
                                            (str_p("tag=") >> (+(print_p - ch_p(";")))[assign_a(tagParam)]) |
                                            *(~ch_p(';'))
                                        )
                                 )
                         )
                         // End complex grammar

                         ).full;

    if (success)
    {
        if (uri)
            success = ParseUriString(uriStr, *uri);
        if (tag)
            tag->assign(tagParam);
    }
    else
    {
        if (uri)
            uri->Reset();
        if (tag)
            tag->clear();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////

const std::string BuildCallIdString(const std::string& host, size_t randomness)
{
    std::ostringstream oss;

    oss << VoIPUtils::MakeRandomHexString(randomness) << "@";
    if (host.find_first_of(":") != std::string::npos)
        oss << "[" << host << "]";
    else
        oss << host;

    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

const std::string BuildCSeqString(uint32_t cseq, SipMessage::MethodType method)
{
    return boost::lexical_cast<std::string>(cseq) + " " + LookupMethodType()(method);
}

///////////////////////////////////////////////////////////////////////////////

bool ParseCSeqString(const std::string& cseqStr, uint32_t& cseq, SipMessage::MethodType& method)
{
    return parse(cseqStr.begin(), cseqStr.end(), (uint_p[assign_a(cseq)] >> +space_p >> (+alpha_p)[AssignMethodType(method)])).full;
}

///////////////////////////////////////////////////////////////////////////////

const std::string BuildRAckString(uint32_t rseq, uint32_t cseq, SipMessage::MethodType method)
{
    return boost::lexical_cast<std::string>(rseq) + " " + boost::lexical_cast<std::string>(cseq) + " " + LookupMethodType()(method);
}

///////////////////////////////////////////////////////////////////////////////

bool ParseRAckString(const std::string& rackStr, uint32_t& rseq, uint32_t& cseq, SipMessage::MethodType& method)
{
    return parse(rackStr.begin(), rackStr.end(), (uint_p[assign_a(rseq)] >> +space_p >> (uint_p[assign_a(cseq)] >> +space_p >> (+alpha_p)[AssignMethodType(method)]))).full;
}

///////////////////////////////////////////////////////////////////////////////

const std::string BuildMethodString(const SipMessage::MethodType& method)
{
    return LookupMethodType()(method);
}

///////////////////////////////////////////////////////////////////////////////

const std::string BuildMethodListString(const std::vector<SipMessage::MethodType>& methodTypeVec)
{
    std::ostringstream oss;
    std::transform(methodTypeVec.begin(), methodTypeVec.end(), std::ostream_iterator<std::string>(oss, ","), LookupMethodType());
    std::string temp = oss.str();
    if (!temp.empty())
        temp.resize(temp.size() - 1);  // strips trailing comma
    return temp;
}

///////////////////////////////////////////////////////////////////////////////

const std::string BuildSessionExpirationString(const SipSessionExpiration& se)
{
    std::ostringstream oss;
    oss << boost::lexical_cast<std::string>(se.deltaSeconds);
    switch (se.refresher)
    {
      case SipSessionExpiration::NO_REFRESHER:
      default:
          break;

      case SipSessionExpiration::UAC_REFRESHER:
          oss << ";refresher=uac";
          break;

      case SipSessionExpiration::UAS_REFRESHER:
          oss << ";refresher=uas";
          break;
    }
    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

bool ParseSessionExpirationString(const std::string& seStr, SipSessionExpiration& se)
{
    se.Reset();

    std::string refresher;
    const bool success = parse(seStr.begin(), seStr.end(),

                               // Begin complex grammar
                               (
                                   // Mandatory delta seconds
                                   uint_p[assign_a(se.deltaSeconds)]

                                   // Optional params
                                   >> *(
                                           *space_p >> ch_p(';') >> *space_p
                                           >> (
                                                  (str_p("refresher=") >> (+(print_p - ch_p(";")))[assign_a(refresher)]) |
                                                  *(~ch_p(';'))
                                              )
                                       )
                               )
                               // End complex grammar

                               ).full;

    if (success && !refresher.empty())
    {
        if (refresher == "uac")
            se.refresher = SipSessionExpiration::UAC_REFRESHER;
        else if (refresher == "uas")
            se.refresher = SipSessionExpiration::UAS_REFRESHER;
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////

bool ParseDigestAuthChallenge(const std::string& chalStr, SipDigestAuthChallenge& chal)
{
    chal.Reset();

    std::string& realm(chal.realm);
    std::string& domain(chal.domain);
    std::string& nonce(chal.nonce);
    std::string& opaque(chal.opaque);
    std::string stale;
    std::string algorithm;
    std::string qopList;

    const bool success = parse(chalStr.begin(), chalStr.end(),

                               // Begin complex grammar
                               (
                                   // Mandatory Digest string
                                   str_p("Digest") >> *space_p

                                   // Digest list
                                   >> list_p(*space_p
                                             >> (
                                                    (str_p("realm=") >> confix_p(ch_p('"'), (*print_p)[assign_a(realm)], ch_p('"'))) |
                                                    (str_p("domain=") >> confix_p(ch_p('"'), (*print_p)[assign_a(domain)], ch_p('"'))) |
                                                    (str_p("nonce=") >> confix_p(ch_p('"'), (*print_p)[assign_a(nonce)], ch_p('"'))) |
                                                    (str_p("opaque=") >> confix_p(ch_p('"'), (*print_p)[assign_a(opaque)], ch_p('"'))) |
                                                    (str_p("stale=") >> (+alpha_p)[assign_a(stale)]) |
                                                    (str_p("algorithm=") >> (+(print_p-','))[assign_a(algorithm)]) |
                                                    (str_p("qop=") >> confix_p(ch_p('"'), (*print_p)[assign_a(qopList)], ch_p('"')))
                                                )
                                             >> *space_p, ch_p(','))
                               )
                               // End complex grammar

                               ).full;

    if (success)
    {
        if (!stale.empty())
        {
            if (boost::iequals(stale, "false"))
                chal.stale = false;
            else if (boost::iequals(stale, "true"))
                chal.stale = true;
            else
                return false;
        }

        if (!algorithm.empty())
        {
            if (boost::iequals(algorithm, "MD5"))
                chal.algorithm = SIP_AUTH_MD5;
            else if (boost::iequals(algorithm, "MD5-sess"))
                chal.algorithm = SIP_AUTH_MD5_SESS;
            else
                return false;
        }

        if (!qopList.empty())
        {
            string_token_iterator end;
            for (string_token_iterator qop = string_token_iterator(qopList, ","); qop != end; ++qop)
            {
                if (boost::iequals(*qop, "auth"))
                    chal.qop = static_cast<SipQualityOfProtection>(chal.qop | SIP_QOP_AUTH);
                else if (boost::iequals(*qop, "auth-int"))
                    chal.qop = static_cast<SipQualityOfProtection>(chal.qop | SIP_QOP_AUTH_INT);
                else
                    return false;
            }
        }

        return true;
    }
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////

const std::string BuildDigestAuthCredentialsString(const SipDigestAuthCredentials& cred)
{
    std::ostringstream oss;

    oss << "Digest username=\"" << cred.username << "\", realm=\"" << cred.realm << "\", nonce=\"" << cred.nonce << "\", uri=\"" << cred.uri << "\", response=\"" << cred.response << "\"";

    switch (cred.algorithm)
    {
      case SIP_AUTH_NONE:
          break;

      case SIP_AUTH_MD5:
          oss << ", algorithm=MD5";
          break;

      case SIP_AUTH_MD5_SESS:
          oss << ", algorithm=MD5-sess";
          break;

      default:
          throw EPHXInternal("SipProtocol::BuildDigestAuthCredentialsString: invalid algorithm");
    }

    if (cred.qop == SIP_QOP_AUTH || cred.qop == SIP_QOP_AUTH_INT)
        oss << ", cnonce=\"" << cred.cnonce << "\", nc=" << (boost::format("%08x") % cred.nonceCount);

    if (!cred.opaque.empty())
        oss << ", opaque=\"" << cred.opaque << "\"";

    switch (cred.qop)
    {
      case SIP_QOP_NONE:
          break;

      case SIP_QOP_AUTH:
          oss << ", qop=auth";
          break;

      case SIP_QOP_AUTH_INT:
          oss << ", qop=auth-int";
          break;

      default:
          throw EPHXInternal("SipProtocol::BuildDigestAuthCredentialsString: invalid qop");
    }

    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

const std::string BuildSdpString(const SdpMessage& sdp)
{
    std::ostringstream oss;

    switch (sdp.addrType)
    {
      case SdpMessage::SDP_ADDR_TYPE_IPV4:
          oss << "IN IP4 " << sdp.addrStr;
          break;

      case SdpMessage::SDP_ADDR_TYPE_IPV6:
          oss << "IN IP6 " << sdp.addrStr;
          break;

      default:
          throw EPHXInternal("SipProtocol::BuildSdpString");
    }

    const std::string connStr = oss.str();
    oss.seekp(0);

    oss << "v=0\r\n";
    oss << "o=- " << sdp.sessionId << " " << sdp.timestamp << " " << connStr << "\r\n";
    oss << "s=Spirent TestCenter\r\n";
    oss << "c=" << connStr << "\r\n";
    oss << "t=" << sdp.timestamp << " 0\r\n";

    if (sdp.audioMediaFormat)
    {
        switch (sdp.audioMediaFormat)
        {
          case SdpMessage::SDP_MEDIA_FORMAT_PCMU:
              oss << "m=audio " << sdp.audioTransportPort << " RTP/AVP 0 100\r\n";
              oss << "a=rtpmap:0 PCMU/8000\r\n";
              break;

          case SdpMessage::SDP_MEDIA_FORMAT_G723:
              oss << "m=audio " << sdp.audioTransportPort << " RTP/AVP 4 100\r\n";
              oss << "a=rtpmap:4 G723/8000\r\n";
              break;

          case SdpMessage::SDP_MEDIA_FORMAT_G729:
              oss << "m=audio " << sdp.audioTransportPort << " RTP/AVP 18 100\r\n";
              oss << "a=rtpmap:18 G729/8000\r\n";
              break;

          case SdpMessage::SDP_MEDIA_FORMAT_PCMA:
              oss << "m=audio " << sdp.audioTransportPort << " RTP/AVP 8 100\r\n";
              oss << "a=rtpmap:8 PCMA/8000\r\n";
              break;

         case SdpMessage::SDP_MEDIA_FORMAT_G726_32:
             {
                 uint16_t rtp_pt;
                 if(sdp.audioRtpDynamicPayloadType >= 96 && sdp.audioRtpDynamicPayloadType <= 127 )
                    rtp_pt = static_cast<uint16_t> (sdp.audioRtpDynamicPayloadType);
                 else
                    rtp_pt = 2;
                 oss << "m=audio " << sdp.audioTransportPort << " RTP/AVP "<< rtp_pt <<" 100\r\n";
                 oss << "a=rtpmap:" << rtp_pt << " G726-32/8000\r\n";
             }
             break;

         case SdpMessage::SDP_MEDIA_FORMAT_PCMU_WB:
        	 if(sdp.audioRtpDynamicPayloadType >= 96 && sdp.audioRtpDynamicPayloadType <= 127 )
        	 {
                 uint16_t rtp_pt = static_cast<uint16_t> (sdp.audioRtpDynamicPayloadType);
                 oss << "m=audio " << sdp.audioTransportPort << " RTP/AVP "<< rtp_pt <<" 100\r\n";
                 oss << "a=rtpmap:" << rtp_pt << " PCMU-WB/16000\r\n";
                 oss << "a=fmtp:" << rtp_pt << " mode-set=4\r\n";
        	 }
        	 else
        	 {
                 throw EPHXInternal("SipProtocol::BuildSdpString");
        	 }
             break;

         case SdpMessage::SDP_MEDIA_FORMAT_PCMA_WB:
        	 if(sdp.audioRtpDynamicPayloadType >= 96 && sdp.audioRtpDynamicPayloadType <= 127 )
        	 {
                 uint16_t rtp_pt = static_cast<uint16_t> (sdp.audioRtpDynamicPayloadType);
                 oss << "m=audio " << sdp.audioTransportPort << " RTP/AVP "<< rtp_pt <<" 100\r\n";
                 oss << "a=rtpmap:" << rtp_pt << " PCMA-WB/16000\r\n";
                 oss << "a=fmtp:" << rtp_pt << " mode-set=4\r\n";
        	 }
        	 else
        	 {
                 throw EPHXInternal("SipProtocol::BuildSdpString");
        	 }
             break;

          default:
              throw EPHXInternal("SipProtocol::BuildSdpString");
        }

        oss << "a=rtpmap:100 telephone-event/8000\r\n";
        oss << "a=ptime:20\r\n";
        oss << "a=fmtp:100 0-15\r\n";
    }

    if (sdp.videoMediaFormat)
    {
        switch (sdp.videoMediaFormat)
        {
          case SdpMessage::SDP_MEDIA_FORMAT_H263:
             oss << "m=video " << sdp.videoTransportPort << " RTP/AVP 34\r\n";
             oss << "a=rtpmap:34 H263/90000\r\n";
             oss << "a=fmtp:34 QCIF=2\r\n";
             break;

          case SdpMessage::SDP_MEDIA_FORMAT_H264:
             oss << "m=video " << sdp.videoTransportPort << " RTP/AVP 96\r\n";
             oss << "a=rtpmap:96 H264/90000\r\n";
             oss << "a=fmtp:96\r\n";
             oss << "a=ptime:15\r\n";
             break;

          case SdpMessage::SDP_MEDIA_FORMAT_MP4V_ES:
             oss << "m=video " << sdp.videoTransportPort << " RTP/AVP 98\r\n";
             oss << "b=AS:2048\r\n";
             oss << "a=rtpmap:98 MP4V-ES/90000\r\n";
             oss << "a=framerate:15\r\n";
             oss << "a=fmtp:98 profile-level-id=3;framesize=VGA\r\n";
             break;

          default:
              throw EPHXInternal("SipProtocol::BuildSdpString");
        }
    }

    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

bool ParseSdpString(const std::string& sdpStr, SdpMessage& sdp)
{
    sdp.Reset();

    // Parse announcement line-by-line
    HeaderIterator hdrIter(sdpStr);

    while (hdrIter != HeaderIterator())
    {
        const std::string hdr(*hdrIter);
        ++hdrIter;

        // Each line should be in the format of <type>=<value>
        char type = 0;
        std::string value;

        if (!parse(hdr.begin(), hdr.end(), alpha_p[assign_a(type)] >> ch_p('=') >> (+print_p)[assign_a(value)]).full)
            continue;

        // We only care to decode certain types
        switch (type)
        {
          case 'c':
          {
              parse(value.begin(), value.end(), (str_p("IN ")
                                                 >> (str_p("IP4 ")[assign_a(sdp.addrType, SdpMessage::SDP_ADDR_TYPE_IPV4)] |
                                                     str_p("IP6 ")[assign_a(sdp.addrType, SdpMessage::SDP_ADDR_TYPE_IPV6)])
                                                 >> (*(alnum_p | chset_p(".:")))[assign_a(sdp.addrStr)]
                                                 >> *anychar_p));
              break;
          }

          case 'm':
          {
              parse(value.begin(), value.end(), ((str_p("audio ") >> uint_p[assign_a(sdp.audioTransportPort)] >> *anychar_p) |
                                                 (str_p("video ") >> uint_p[assign_a(sdp.videoTransportPort)] >> *anychar_p)));
              break;
          }

          default:
              break;
        }
    }

    return (sdp.addrType != SdpMessage::SDP_ADDR_TYPE_UNKNOWN && !sdp.addrStr.empty());
}

///////////////////////////////////////////////////////////////////////////////

static void AppendToPacket(Packet& packet, const std::string& str)
{
    const size_t len = str.size();

    if (len)
    {
        uint8_t *buffer = packet.Put(len);
        memcpy(buffer, str.data(), len);
    }
}

///////////////////////////////////////////////////////////////////////////////

struct DoAppendHeader : public std::unary_function<SipMessage::HeaderField, void>
{
    DoAppendHeader(const StaticData::HeaderStringMap_t& stringMap, Packet& packet, bool& error)
        : mStringMap(stringMap),
          mPacket(packet),
          mError(error)
    {
        mError = false;
    }

    void operator()(const SipMessage::HeaderField& f)
    {
        if (f.type == SipMessage::SIP_HEADER_UNKNOWN)
            return;

        StaticData::HeaderStringMap_t::const_iterator fieldIter = mStringMap.find(f.type);
        if (fieldIter == mStringMap.end())
        {
            TC_LOG_ERR_LOCAL(mPacket.Port, LOG_SIP, "[SipProtocol::DoAppendHeader] Invalid header field type ("
                             << f.type << ") with value (" << f.value << ")");
            mError = true;
            return;
        }

        std::ostringstream oss;
        oss << fieldIter->second << ": " << f.value << _StaticData().CrLf;
        AppendToPacket(mPacket, oss.str());
    }

private:
    const StaticData::HeaderStringMap_t& mStringMap;
    Packet& mPacket;
    bool& mError;
};

///////////////////////////////////////////////////////////////////////////////

bool BuildPacket(const SipMessage& msg, Packet& packet)
{
    switch (msg.type)
    {
      case SipMessage::SIP_MSG_REQUEST:
      {
          // Build request line
          if (msg.request.version != SipMessage::SIP_VER_2_0)
          {
              TC_LOG_ERR_LOCAL(packet.Port, LOG_SIP, "[SipProtocol::BuildPacket] Protocol version is not 2.0");
              return false;
          }

          StaticData::MethodStringMap_t::const_iterator methodIter = _StaticData().MethodStringMap.find(msg.method);
          if (methodIter == _StaticData().MethodStringMap.end())
          {
              TC_LOG_ERR_LOCAL(packet.Port, LOG_SIP, "[SipProtocol::BuildPacket] Invalid request method (" << msg.method << ")");
              return false;
          }

          std::ostringstream oss;
          oss << methodIter->second << " " << msg.request.uri.BuildUriString() << " " << _StaticData().Sip2dot0 << _StaticData().CrLf;
          AppendToPacket(packet, oss.str());
          break;
      }

      case SipMessage::SIP_MSG_STATUS:
      {
          // Build status line
          if (msg.status.version != SipMessage::SIP_VER_2_0)
          {
              TC_LOG_ERR_LOCAL(packet.Port, LOG_SIP, "[SipProtocol::BuildPacket] Protocol version is not 2.0");
              return false;
          }

          std::ostringstream oss;
          oss << _StaticData().Sip2dot0 << " " << msg.status.code;
          if (!msg.status.message.empty())
              oss << " " << msg.status.message;
          oss << _StaticData().CrLf;
          AppendToPacket(packet, oss.str());
          break;
      }

      default:
        TC_LOG_ERR_LOCAL(packet.Port, LOG_SIP, "[SipProtocol::BuildPacket] Message type isn't request or status?!?");
        throw EPHXInternal("SipProtocol::BuildPacket");
    }

    // Build headers
    bool error;
    for_each(msg.headers.begin(), msg.headers.end(), DoAppendHeader(msg.compactHeaders ? _StaticData().CompactHeaderStringMap : _StaticData().HeaderStringMap, packet, error));
    if (error)
        return false;

    // Build body
    AppendToPacket(packet, _StaticData().CrLf);
    AppendToPacket(packet, msg.body);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool ParsePacket(const Packet& packet, SipMessage& msg)
{
    // Reset message
    msg.type = SipMessage::SIP_MSG_UNKNOWN;
    msg.method = SipMessage::SIP_METHOD_UNKNOWN;
    msg.headers.clear();
    msg.compactHeaders = false;
    msg.body.clear();

    // Parse packet line-by-line
    const std::string buffer(reinterpret_cast<const char *>(packet.Data()), static_cast<std::string::size_type>(packet.Length()));
    HeaderIterator hdrIter(buffer);

    while (hdrIter != HeaderIterator())
    {
        const std::string hdr(*hdrIter);
        ++hdrIter;

        if (msg.type == SipMessage::SIP_MSG_UNKNOWN)
        {
            std::istringstream iss(hdr);

            // First keyword determines request or status type
            std::string keyword;
            iss >> std::noskipws >> keyword;
            if (iss.get() != ' ')
                goto invalid;

            if (keyword == _StaticData().Sip2dot0)
            {
                // This is a status message
                msg.type = SipMessage::SIP_MSG_STATUS;
                msg.status.version = SipMessage::SIP_VER_2_0;
                iss >> msg.status.code;
                if (iss.get() == ' ')
                    msg.status.message = hdr.substr(iss.tellg());
                else
                    msg.status.message.clear();
            }
            else
            {
                // This may be a request message -- check method name to be sure
                StaticData::MethodTypeMap_t::const_iterator methodIter = _StaticData().MethodTypeMap.find(keyword);
                if (methodIter != _StaticData().MethodTypeMap.end())
                {
                    msg.type = SipMessage::SIP_MSG_REQUEST;
                    msg.method = methodIter->second;

                    std::string uri;
                    iss >> std::noskipws >> uri;
                    if (!ParseUriString(uri, msg.request.uri) || iss.get() != ' ')
                        goto invalid;

                    iss >> std::noskipws >> keyword;
                    msg.request.version = (keyword == _StaticData().Sip2dot0) ? SipMessage::SIP_VER_2_0 : SipMessage::SIP_VER_UNKNOWN;
                }
                else
                    goto invalid;
            }
        }
        else
        {
            if (hdr.empty())
            {
                // An empty header line signals start of the message body
                break;
            }
            else if (hdr[0] == '\t' || hdr[0] == ' ')
            {
                // If header line starts with whitespace, it is a continuation of a prior line
                if (!msg.headers.empty())
                {
                    SipMessage::HeaderField& f(msg.headers.back());
                    f.value += " ";
                    f.value += hdr.substr(hdr.find_first_not_of("\t "));
                }
                else
                    goto invalid;
            }
            else
            {
                // Parse new header field
                std::string::size_type colon = hdr.find(':');
                if (colon == std::string::npos)
                    goto invalid;

                const std::string name = hdr.substr(0, hdr.find_first_of(":\t "));
                StaticData::HeaderTypeMap_t::const_iterator fieldIter = _StaticData().HeaderTypeMap.find(name);
                if (fieldIter != _StaticData().HeaderTypeMap.end())
                    msg.headers.push_back(SipMessage::HeaderField(fieldIter->second, hdr.substr(hdr.find_first_not_of("\t ", colon + 1))));
            }

            continue;

          invalid:
            TC_LOG_ERR_LOCAL(packet.Port, LOG_SIP, "[SipProtocol::ParsePacket] Invalid header line (" << hdr << ")");
            return false;
        }
    }

    // Handle message body
    if (hdrIter != HeaderIterator())
        msg.body = buffer.substr(hdrIter.Pos());

    return true;
}

///////////////////////////////////////////////////////////////////////////////

};

END_DECL_SIPLEAN_NS
