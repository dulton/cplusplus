/// @file
/// @brief Server Connection Handler implementation
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
#include <sstream>
#include <utility>

#include <ace/Message_Block.h>
#include <utils/AppOctetStreamMessageBody.h>
#include <utils/MessageUtils.h>

#include "FtpdLog.h"
#include "DataConnectionHandler.h"
#include "DataTransactionManager.h"

USING_FTP_NS;


///////////////////////////////////////////////////////////////////////////////

DataConnectionHandler::DataConnectionHandler(uint32_t serial) :
    L4L7_APP_NS::StreamSocketHandler(serial),
    mContentType(L4L7_BASE_NS::BodyContentOptions_BINARY),
    mRandomEngine(std::max(ACE_OS::gettimeofday().usec(), static_cast<suseconds_t>(1))),
    mBodySize(mRandomEngine),
    mDoTx(true)
{
#ifdef UNIT_TEST
    FTP_LOG_DEBUG(-1, -1, "Construct DataConnectionHandler") ;
#endif
}

///////////////////////////////////////////////////////////////////////////////

DataConnectionHandler::~DataConnectionHandler()
{
#ifdef UNIT_TEST
    FTP_LOG_DEBUG(-1, -1, "Destruct DataConnectionHandler") ;
#endif
}

///////////////////////////////////////////////////////////////////////////////
void DataConnectionHandler::SetTxBodyConfig(
    uint8_t &szType, 
    uint32_t &fixedSz, 
    uint32_t &randomSzMean, 
    uint32_t &randomStdDev, 
    uint8_t &contentType) 
{
    if (szType == L4L7_BASE_NS::BodySizeOptions_FIXED)
    {
        mBodySize.Set(fixedSz);
    }
    else if (szType == L4L7_BASE_NS::BodySizeOptions_RANDOM)
    {
        mBodySize.Set(randomSzMean, randomStdDev) ;
    }
    else
    {
        throw EPHXBadConfig();
    }
    mContentType = contentType ;
}
                                                         
///////////////////////////////////////////////////////////////////////////////

int DataConnectionHandler::HandleOpenHook(void)
{    
    const ACE_Time_Value now = ACE_OS::gettimeofday() ;

    if (mDoTx)
    {       
        uint32_t sz = GetBodySize() ;
        messagePtr_t respBody = L4L7_UTILS_NS::MessageAlloc(new L4L7_UTILS_NS::AppOctetStreamMessageBody(sz)); 
        if (!Send(respBody))
            return -1 ;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int DataConnectionHandler::HandleInputHook(void)
{
    messagePtr_t mb(Recv());

    NotifyRx((uint32_t) mb->total_length()) ;

    if (!mDoTx)
    {
    }
    else
    {
        // we are sending a file to client -- why are we receiving data???? Log errors
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
int DataConnectionHandler::HandleOutputHook(void)
{
    if (mDoTx)
        return -1 ; // will close connection on a Tx connection
    return 0 ;
}

///////////////////////////////////////////////////////////////////////////////


