/// @file
/// @brief FTP protocol header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _FTP_PROTOCOL_H_
#define _FTP_PROTOCOL_H_

#include <string>

#include <utils/MessageIterator.h>
#include "FtpCommon.h"
#include <ace/INET_Addr.h>
#include <boost/spirit/include/classic.hpp>

// Forward declarations (global)
class ACE_Time_Value;

DECL_FTP_NS

namespace FtpProtocol
{
    
///////////////////////////////////////////////////////////////////////////////

enum MethodType
{
    FTP_METHOD_UNKNOWN = 0,
    FTP_METHOD_USER,
    FTP_METHOD_PASSWORD,
    FTP_METHOD_TYPE,
    FTP_METHOD_PORT,
    FTP_METHOD_PASV,
    FTP_METHOD_MODE,
    FTP_METHOD_STOR,
    FTP_METHOD_RETR,
    FTP_METHOD_QUIT
};

enum StatusCode
{
    FTP_STATUS_DATA_CONN_ALREADY_OPEN = 125, // Unused right now. TODO remove
    FTP_STATUS_FILE_STATUS_OK_OPENING_DATA_CONN = 150,
    FTP_STATUS_OK = 200,
    FTP_STATUS_SERVICE_READY = 220,
    FTP_STATUS_CLOSING_DATA_CONN = 226,
    FTP_STATUS_ENTERING_PASSIVE_MODE = 227,
    FTP_STATUS_USER_LOGGED_IN = 230,
    FTP_STATUS_FILE_ACTION_COMPLETE = 250, // Unused right now. TODO remove
    FTP_STATUS_NEED_PASSWORD = 331,
    FTP_STATUS_DATA_CONN_FAILED = 425,
    FTP_STATUS_CONN_CLOSED_TRANSFER_ABORTED = 426,
    FTP_STATUS_ACTION_NOT_TAKEN = 452,
    FTP_STATUS_SYNTAX_ERROR = 500,
    FTP_STATUS_COMMAND_NOT_IMPLEMENTED = 502,
    FTP_STATUS_NOT_LOGGED_IN = 530,
    FTP_STATUS_CODE_INVALID = -1
};

///////////////////////////////////////////////////////////////////////////////

class FtpParserInterface
{
public:
    FtpParserInterface() {} ;
    virtual ~FtpParserInterface() {};

    virtual const bool Parse(L4L7_UTILS_NS::MessageConstIterator begin, 
                            L4L7_UTILS_NS::MessageConstIterator end) = 0 ;
    virtual const bool Parse(const std::string& ) = 0;
protected:
    virtual void Clear() = 0 ;

} ;

///////////////////////////////////////////////////////////////////////////

class FtpRequestParser : public FtpParserInterface
{
public:
    FtpRequestParser() ;
   ~FtpRequestParser() ;

    const bool Parse(L4L7_UTILS_NS::MessageConstIterator begin, 
                     L4L7_UTILS_NS::MessageConstIterator end)  ;  
  
    const bool Parse(const std::string &req) ;

    const std::string GetMethodTypeString() const { return mCmd ; }
    const MethodType GetMethodType() const { return mMethod ; } ;
    const std::string GetRequestString() const { return mReqMsg ; };
    const std::string& GetConstRequestString() const { return mReqMsg ; }

protected:
    void Clear() ;
    void CalculateMethodType() ;

private:
    std::string mCmd ;
    MethodType mMethod ;
    std::string mReqMsg ;
} ;

///////////////////////////////////////////////////////////////////////////

class FtpResponseParser : public FtpParserInterface
{
public:
    FtpResponseParser() ;
    ~FtpResponseParser() ;

    const bool Parse(L4L7_UTILS_NS::MessageConstIterator begin, 
                     L4L7_UTILS_NS::MessageConstIterator end)  ;  
    const bool Parse(const std::string &resp) ;
   
    const StatusCode GetStatusCode() const { return (StatusCode) mRespCode ; }
    const std::string GetStatusMsg() const { return mRespMsg ; }

protected:
    void Clear() ;
    void CalculateRespCode() ;

private:
    int         mRespInt ;
    StatusCode  mRespCode ;
    std::string mRespMsg ;
} ;

///////////////////////////////////////////////////////////////////////////

class FtpPortRequestParser : public FtpParserInterface
{
public:
    FtpPortRequestParser() ;
    virtual ~FtpPortRequestParser() ; 

    virtual const bool Parse(L4L7_UTILS_NS::MessageConstIterator begin, 
                     L4L7_UTILS_NS::MessageConstIterator end)  ;  
    virtual const bool Parse(const std::string &portString)  ;
 
    const std::vector<unsigned char>& GetPortInfo() const { return mPortInfo ; }
    const ACE_INET_Addr GetPortInetAddr() const { return mInetAddr ; }

protected:
    void Clear() ;
    const bool IsPortSpecValid() ;

    std::vector<unsigned char> mPortInfo ;
    ACE_INET_Addr mInetAddr ;
} ; 

///////////////////////////////////////////////////////////////////////////

class FtpPasvResponseParser : public FtpParserInterface
{
public:
    FtpPasvResponseParser() ;
    ~FtpPasvResponseParser() ; 

    const bool Parse(L4L7_UTILS_NS::MessageConstIterator begin, 
                     L4L7_UTILS_NS::MessageConstIterator end)  ;  
    const bool Parse(const std::string &pasvString)  ;

    ACE_INET_Addr GetPasvInetAddr() const { return mInetAddr ; }
    
private:
    std::string mPortStr ;

    void Clear() ;
    ACE_INET_Addr mInetAddr ;
} ;

///////////////////////////////////////////////////////////////////////////////

void Init(void);

const std::string BuildRequestLine(MethodType method, const std::string& reqToken) ;

const std::string BuildStatusLine(StatusCode code, const std::string &optionalData = "") ;

const std::string BuildPortRequestString(const ACE_INET_Addr& addr) ;

const std::string BuildPasvPortString(const ACE_INET_Addr& addr) ;

///////////////////////////////////////////////////////////////////////////////

}  // namespace FtpProtocol

END_DECL_FTP_NS

#endif
