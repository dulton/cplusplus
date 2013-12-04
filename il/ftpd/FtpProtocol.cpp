/// @file
/// @brief FTP protocol implementation
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
#include <boost/unordered_map.hpp>
#include <phxexception/PHXException.h>

#include "FtpProtocol.h"

#include <netinet/in.h>

using namespace boost::spirit::classic;

DECL_FTP_NS

namespace FtpProtocol {

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

typedef boost::unordered_map<std::string, MethodType, NoCaseHashString, NoCaseCompareString> MethodTypeMap_t;
typedef boost::unordered_map<MethodType, std::string> MethodStringMap_t;

typedef boost::unordered_map<StatusCode, std::string> ReasonPhraseStringMap_t;
        
///////////////////////////////////////////////////////////////////////////////


static const std::string MethodUser = "USER";
static const std::string MethodPassword = "PASS";
static const std::string MethodXferType = "TYPE" ;
static const std::string MethodPort = "PORT" ;
static const std::string MethodPasv = "PASV" ;
static const std::string MethodMode = "MODE";
static const std::string MethodStore = "STOR";
static const std::string MethodRetrieve = "RETR";
static const std::string MethodQuit = "QUIT";


static const std::string Reason125 = "Data connection already open; transfer starting." ;
static const std::string Reason150 = "File status okay; about to open data connection." ;
static const std::string Reason200 = "Command okay.";
static const std::string Reason220 = "Service ready, proceed." ;
static const std::string Reason226 = "File transfer complete, closing data connection." ;
static const std::string Reason227 = "Entering passive mode " ;
static const std::string Reason230 = "User Logged in, proceed." ;
static const std::string Reason250 = "Requested file action okay, completed." ;
static const std::string Reason331 = "User name okay, need password.";
static const std::string Reason425 = "Can't open data connection." ;
static const std::string Reason426 = "Connection closed; transfer aborted." ;
static const std::string Reason452 = "Requested action not taken." ;
static const std::string Reason500 = "Syntax Error, command unrecognized." ;
static const std::string Reason502 = "Command not implemented." ;
static const std::string Reason530 = "Not logged in." ;

///////////////////////////////////////////////////////////////////////////////
    
static MethodTypeMap_t MethodTypeMap;
static MethodStringMap_t MethodStringMap;

static ReasonPhraseStringMap_t ReasonPhraseStringMap;
       
static bool _Init = false;

///////////////////////////////////////////////////////////////////////////////

void Init(void)
{
    if (_Init)
        return;
    
    MethodTypeMap[MethodUser]     = FTP_METHOD_USER ; 
    MethodTypeMap[MethodPassword] = FTP_METHOD_PASSWORD; 
    MethodTypeMap[MethodMode]     = FTP_METHOD_MODE;
    MethodTypeMap[MethodXferType] = FTP_METHOD_TYPE ;
    MethodTypeMap[MethodPort]     = FTP_METHOD_PORT ;
    MethodTypeMap[MethodPasv]     = FTP_METHOD_PASV ;
    MethodTypeMap[MethodStore]    = FTP_METHOD_STOR;
    MethodTypeMap[MethodRetrieve] = FTP_METHOD_RETR;
    MethodTypeMap[MethodQuit]     = FTP_METHOD_QUIT;


    MethodStringMap[FTP_METHOD_USER]     = MethodUser;
    MethodStringMap[FTP_METHOD_PASSWORD] = MethodPassword;
    MethodStringMap[FTP_METHOD_MODE]     = MethodMode;
    MethodStringMap[FTP_METHOD_TYPE]     = MethodXferType ;
    MethodStringMap[FTP_METHOD_PORT ]    = MethodPort ;
    MethodStringMap[FTP_METHOD_PASV]     = MethodPasv ;
    MethodStringMap[FTP_METHOD_STOR]     = MethodStore;
    MethodStringMap[FTP_METHOD_RETR]     = MethodRetrieve;
    MethodStringMap[FTP_METHOD_QUIT]     = MethodQuit;


    ReasonPhraseStringMap[FTP_STATUS_DATA_CONN_ALREADY_OPEN]            = Reason125;
    ReasonPhraseStringMap[FTP_STATUS_FILE_STATUS_OK_OPENING_DATA_CONN]  = Reason150 ;
    ReasonPhraseStringMap[FTP_STATUS_OK]                                = Reason200 ;
    ReasonPhraseStringMap[FTP_STATUS_SERVICE_READY]                     = Reason220 ;
    ReasonPhraseStringMap[FTP_STATUS_CLOSING_DATA_CONN]                 = Reason226 ;
    ReasonPhraseStringMap[FTP_STATUS_ENTERING_PASSIVE_MODE]             = Reason227 ;
    ReasonPhraseStringMap[FTP_STATUS_USER_LOGGED_IN]                    = Reason230 ;
    ReasonPhraseStringMap[FTP_STATUS_FILE_ACTION_COMPLETE]              = Reason250 ;
    ReasonPhraseStringMap[FTP_STATUS_NEED_PASSWORD]                     = Reason331 ;
    ReasonPhraseStringMap[FTP_STATUS_DATA_CONN_FAILED]                  = Reason425 ;
    ReasonPhraseStringMap[FTP_STATUS_CONN_CLOSED_TRANSFER_ABORTED]      = Reason426 ;
    ReasonPhraseStringMap[FTP_STATUS_ACTION_NOT_TAKEN]                  = Reason452 ;
    ReasonPhraseStringMap[FTP_STATUS_SYNTAX_ERROR]                      = Reason500 ;
    ReasonPhraseStringMap[FTP_STATUS_COMMAND_NOT_IMPLEMENTED]           = Reason502 ;
    ReasonPhraseStringMap[FTP_STATUS_NOT_LOGGED_IN]                     = Reason530 ;

    
    _Init = true;
}
    
///////////////////////////////////////////////////////////////////////////////
const std::string BuildRequestLine(MethodType method, const std::string& reqToken)
{
    if (!_Init)
        Init();

    MethodStringMap_t::const_iterator methodIter = MethodStringMap.find(method);
    if (methodIter == MethodStringMap.end())
        throw EPHXInternal();
    
    std::ostringstream oss;
    oss << methodIter->second ;
    if (reqToken != "") 
        oss << " " << reqToken ;
    oss << "\r\n";
    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////


const std::string BuildStatusLine(StatusCode code, const std::string &optionalData)
{
    if (!_Init)
        Init();

    ReasonPhraseStringMap_t::const_iterator reasonPhraseIter = ReasonPhraseStringMap.find(code);
    if (reasonPhraseIter == ReasonPhraseStringMap.end())
        throw EPHXInternal();
    
    std::ostringstream oss;
    oss << code << " " << reasonPhraseIter->second ;
    if (optionalData != "") 
        oss << " " << optionalData ;
    oss << "\r\n";
    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

const std::string BuildPortRequestString(const ACE_INET_Addr& addr) 
{
    std::ostringstream oss ;    
    struct sockaddr *sockaddr = (struct sockaddr *) addr.get_addr() ;
    if (sockaddr->sa_family == AF_INET)
    {
        uint32_t ip = addr.get_ip_address() ;
        oss        << (unsigned int)((ip >> 24) & 0xFF)
            << "," << (unsigned int)((ip >> 16) & 0xFF)
            << "," << (unsigned int)((ip >> 8) & 0xFF) 
            << "," << (unsigned int)(ip  & 0xFF) 
            << "," ;
            
    }
    else if (sockaddr->sa_family == AF_INET6)
    {
        struct sockaddr_in6 *sockIpv6 = (struct sockaddr_in6 *) sockaddr ;
        for (int i= 0 ; i  < 16; i++)
             oss << (unsigned int) sockIpv6->sin6_addr.s6_addr[i] << "," ;
    }
    else
        throw EPHXInternal() ;
    
    // encode the port number
    uint16_t port = addr.get_port_number() ;
    oss        << (unsigned int) ((port >> 8) & 0xFF)
        << "," << (unsigned int) (port & 0xFF) ;
    return oss.str() ;    
}

///////////////////////////////////////////////////////////////////////////////

const std::string BuildPasvPortString(const ACE_INET_Addr& addr) 
{
    std::string portStr = BuildPortRequestString(addr) ;

    std::ostringstream oss ;
    oss << "(" << portStr << ")"  ;

    return oss.str() ;    
}

///////////////////////////////////////////////////////////////////////////////

////////////////////// Methods from FtpRequestParser //////////////////////////

FtpRequestParser::FtpRequestParser() 
{			  
}

FtpRequestParser::~FtpRequestParser() 
{
}

const bool FtpRequestParser::Parse(L4L7_UTILS_NS::MessageConstIterator begin, 
                                   L4L7_UTILS_NS::MessageConstIterator end)  
{
#if 0
    Clear() ;
    mPi = parse(begin, end, mRule) ;    
    if (mPi.full)
    {
        CalculateMethodType() ;
        return true ;
    }
    return false ;
#else
    return Parse(std::string(begin, end)) ;
#endif
}
  
const bool FtpRequestParser::Parse(const std::string &req) 
{
    Clear() ;
    const bool success = parse(req.c_str(),
                               // Begin grammar
                               (
                                 (+alpha_p)[assign_a(mCmd)] >> 
                                  *space_p >> 
                                  (*(anychar_p - eol_p))[assign_a(mReqMsg)] >> 
                                  *eol_p
                               )
                               // End grammar
                               ).full;

    if (success)
    {
        CalculateMethodType() ;
        return true ;
    }
    return false ;
}   
    
void FtpRequestParser::Clear() 
{
    mCmd = "" ;
    mMethod = FTP_METHOD_UNKNOWN ;
    mReqMsg = "" ;
}

void FtpRequestParser::CalculateMethodType()
{
    MethodTypeMap_t::iterator mit = MethodTypeMap.find(mCmd) ;
    if (mit == MethodTypeMap.end())
        return ;
    mMethod = mit->second ;
}

////////////////////// Methods from FtpResponsearser //////////////////////////

FtpResponseParser::FtpResponseParser()
{

}

FtpResponseParser::~FtpResponseParser() 
{
}

const bool FtpResponseParser::Parse(L4L7_UTILS_NS::MessageConstIterator begin, 
                                   L4L7_UTILS_NS::MessageConstIterator end)                                            
{
#if 0
    Clear() ;
    mPi = parse(begin, end, mRule) ;
    if (mPi.full)
    {
        CalculateRespCode() ;
        return true ;
    }
    return false ;  
#else
    return Parse(std::string(begin, end)) ;
#endif
}

const bool FtpResponseParser::Parse(const std::string &resp) 
{
    Clear() ;
    const bool success = parse(resp.c_str(),
                               // Begin grammar
                               (
                                  int_p[assign_a(mRespInt)] >> 
                                  space_p >> 
                                  (+(anychar_p - eol_p))[assign_a(mRespMsg)] >> 
                                  *eol_p
                               ) 
                               // End grammar
                               ).full;

    if (success)
    {
        CalculateRespCode() ;
        return true ;
    }
    return false ;
}

void FtpResponseParser::Clear()
{
    mRespInt = 0 ;
    mRespCode = FTP_STATUS_CODE_INVALID ;
    mRespMsg = "" ;
}

void FtpResponseParser::CalculateRespCode()
{
    ReasonPhraseStringMap_t::iterator rmit = ReasonPhraseStringMap.find((StatusCode) mRespInt) ;
    if (rmit == ReasonPhraseStringMap.end())
        return ;
    mRespCode = rmit->first ;
}

////////////////////// Methods from FtpPortRequestParser //////////////////////////

FtpPortRequestParser::FtpPortRequestParser() 
{       
}

FtpPortRequestParser::~FtpPortRequestParser() 
{
} 

const bool FtpPortRequestParser::Parse(L4L7_UTILS_NS::MessageConstIterator begin, 
                                      L4L7_UTILS_NS::MessageConstIterator end)  
{
#if 0
    Clear() ;
    mPi = parse(begin, end, mRule) ;
    return mPi.hit && IsPortSpecValid() ;
#else
    return Parse(std::string(begin, end)) ;
#endif

}

const bool FtpPortRequestParser::Parse(const std::string &portString)  
{
    Clear() ;
    const bool success = parse(portString.c_str(),
                               // Begin grammar
                               (
                                 (*space_p) >> 
                                 (list_p((*int_p[push_back_a(mPortInfo)]), ','))
                               ) 
                               // End grammar
                               ).hit;

    return success && IsPortSpecValid() ;
}

void FtpPortRequestParser::Clear()
{
    mPortInfo.clear() ;
}

const bool FtpPortRequestParser::IsPortSpecValid() 
{
    std::vector<unsigned char>::size_type sz = mPortInfo.size() ;
    /* The numbers 6 and 18 come from the sum of the number of bytes in Ipv4 (4)
       or an IPv6 address (16) plus 2 bytes for the TCP port number */
    if (sz == 6)
    {
        struct sockaddr_in in4 ;
        memset(&in4, 0, sizeof(struct sockaddr_in)) ;
        in4.sin_family = AF_INET ;
        in4.sin_addr.s_addr = htonl( (uint32_t) ((mPortInfo[0] << 24) |
                                                 (mPortInfo[1] << 16) |
                                                 (mPortInfo[2] << 8)  | 
                                                 (mPortInfo[3]))
                                   ) ;
        in4.sin_port =  htons( (uint16_t) ((mPortInfo[4] << 8) | mPortInfo[5]) );
        mInetAddr.set(&in4, sizeof(struct sockaddr_in)) ;
        return true ;
    }
    else if (sz == 18)
    {
        struct sockaddr_in6 in6 ;
        memset(&in6, 0, sizeof(struct sockaddr_in6)) ;
        in6.sin6_family = AF_INET6 ;
        in6.sin6_port = htons ( (uint16_t) ((mPortInfo[16] << 8) | mPortInfo[17]) );

        for (std::vector<unsigned char>::size_type sz  = 0 ; sz < 16; ++sz)
            in6.sin6_addr.s6_addr[sz] = mPortInfo[sz] ;

        mInetAddr.set(reinterpret_cast<const struct sockaddr_in *>(&in6), sizeof(struct sockaddr_in6)) ;
        return true ;
    }
    return false ;
}

////////////////////// Methods from FtpRequestParser //////////////////////////
FtpPasvResponseParser::FtpPasvResponseParser()
{
}

FtpPasvResponseParser::~FtpPasvResponseParser() 
{
} 

const bool FtpPasvResponseParser::Parse(L4L7_UTILS_NS::MessageConstIterator begin, 
                                        L4L7_UTILS_NS::MessageConstIterator end) 
{
#if 0
    parse(begin, end, mMsgRule) ;
    if (mPi.hit && mPortStr != "")
        return FtpPortRequestParser::Parse(mPortStr) ;

    return false ;
#else
    return Parse(std::string(begin, end)) ;
#endif

}

const bool FtpPasvResponseParser::Parse(const std::string &pasvString)  
{	
    const bool success = parse(pasvString.c_str(),
                               // Begin grammar
                               (
                                 *(~ch_p('(')) >>
                                 ch_p('(') >>
                                 ((+(anychar_p - ')'))[assign_a(mPortStr)]) >>
                                 ')' >>
                                 *eol_p
                               )
                               // End grammar
                               ).hit;

    if (success && mPortStr != "")
	{
		FtpPortRequestParser portParser ;
		const bool res = portParser.Parse(mPortStr) ;
		if (!res) 
			return false ;
		mInetAddr = portParser.GetPortInetAddr() ;
		return true ;
	}        

    return false ;
}

void FtpPasvResponseParser::Clear()
{
    mPortStr = "" ;
}

///////////////////////////////////////////////////////////////////////////////

}

END_DECL_FTP_NS

