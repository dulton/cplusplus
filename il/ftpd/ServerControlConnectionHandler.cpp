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
#include <boost/foreach.hpp>
#include <utils/MessageUtils.h>
#include "FtpdLog.h"
#include "ServerControlConnectionHandler.h"
#include "ServerControlConnectionHandlerStates.h"
#include "DataTransactionManager.h"

USING_FTP_NS;

///////////////////////////////////////////////////////////////////////////////

const ServerControlConnectionHandler::LoginState     ServerControlConnectionHandler::LOGIN_STATE;
const ServerControlConnectionHandler::WaitingState   ServerControlConnectionHandler::WAIT_STATE;
const ServerControlConnectionHandler::XferSetupState ServerControlConnectionHandler::XFER_SETUP_STATE ;
const ServerControlConnectionHandler::DataXferState  ServerControlConnectionHandler::DATA_STATE;

const ACE_Time_Value ServerControlConnectionHandler::INACTIVITY_TIMEOUT(5 * 60, 0);  // 5 minutes

///////////////////////////////////////////////////////////////////////////////

////////////////////// Public methods ////////////////////////////////////////

struct ServerControlConnectionHandler::RequestRecord
{
    RequestRecord(
        const ACE_Time_Value& theAbsTime, 
        const std::string& theRequest) :                  
        absTime(theAbsTime),
        request(theRequest)
    {
    }

    ACE_Time_Value          absTime;  ///< absolute time that request should be processed
    std::string             request ;
} ;

///////////////////////////////////////////////////////////////////////////////

ServerControlConnectionHandler::ServerControlConnectionHandler(uint32_t serial)
    : L4L7_APP_NS::StreamSocketHandler(serial),
      mRandomEngine(std::max(ACE_OS::gettimeofday().usec(), static_cast<suseconds_t>(1))),
      mRespLatency(mRandomEngine),
      mStats(0),
      mMaxRequests(0),
      mRequestCount(0),
      mTransactionsActive(0),
      mL4L7Profile(0),
      mState(&LOGIN_STATE),
      mCloseFlag(false),
      mActiveDataConnMode(true),
      mActiveDataMgr(0),
      mReqQueueTimeoutDelegate(fastdelegate::MakeDelegate(this, &ServerControlConnectionHandler::HandleRequestQueueTimeout)),
      mInactivityTimeoutDelegate(fastdelegate::MakeDelegate(this, &ServerControlConnectionHandler::HandleInactivityTimeout)),
      mPort(0)
{
}

///////////////////////////////////////////////////////////////////////////////
void ServerControlConnectionHandler::NotifyDataConnectionOpen(const ACE_Time_Value& connectTime) 
{
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : NotifyDataConnectionOpen Start") ;
    if (mStats)
    {
        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock()) ;
        mStats->totalDataConnections++ ;
    }    
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : NotifyDataConnectionOpen End") ;
}

///////////////////////////////////////////////////////////////////////////////
void ServerControlConnectionHandler::NotifyIncrementalTxBytes(uint32_t bytes)  
{
    // Even with UpdateGoodputTx don't remove this.  The call to send bypasses the callback in StreamSocketHandler.

    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : NotifyIncrementalTxBytes Start") ;
    if (mStats)
    {
        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock()) ;
        mStats->goodputTxBytes += bytes ; 
    }  
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : NotifyIncrementalTxBytes End") ;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Notification that Data connection traffic was received
void ServerControlConnectionHandler::NotifyIncrementalRxBytes(uint32_t bytes) 
{
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : NotifyIncrementalRxBytes Start") ;
    if (mStats)
    {
        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock()) ;
        mStats->goodputRxBytes += bytes ; 
    }
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : NotifyIncrementalRxBytes End") ;
}

///////////////////////////////////////////////////////////////////////////////

ServerControlConnectionHandler::~ServerControlConnectionHandler()
{
    UnregisterUpdateTxCountDelegate();
}

///////////////////////////////////////////////////////////////////////////////
void ServerControlConnectionHandler::SetStatsInstance(stats_t& stats)
{
    mStats = &stats;
    RegisterUpdateTxCountDelegate(boost::bind(&ServerControlConnectionHandler::UpdateGoodputTxSent, this, _1));
}

///////////////////////////////////////////////////////////////////////////////

void ServerControlConnectionHandler::UpdateGoodputTxSent(uint64_t sent)
{
    if (mStats)
    {
        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
        mStats->goodputTxBytes += sent;
    }
}

///////////////////////////////////////////////////////////////////////////////
void ServerControlConnectionHandler::GetTxBodyConfig(
    uint8_t &szType, 
    uint32_t &fixedSz, 
    uint32_t &randomSzMean, 
    uint32_t &randomStdDev, 
    uint8_t &contentType) const
{
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : GetTxBodyConfig Start") ;
    szType = mRespConfig.BodySizeType ;
    fixedSz = mRespConfig.FixedBodySize ;
    randomSzMean = mRespConfig.RandomBodySizeMean ;
    randomStdDev = mRespConfig.RandomBodySizeStdDeviation ;
    contentType = mRespConfig.BodyContentType ;
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : GetTxBodyConfig End") ;
}

///////////////////////////////////////////////////////////////////////////////

//////////////////////////// Protected Methods ////////////////////////////////
/// NOTE that FtpServer itself is marked as a protected friend ////////////////

///////////////////////////////////////////////////////////////////////////////
int ServerControlConnectionHandler::HandleOpenHook(void)
{
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : HandleOpenHook Start") ;
    if (mStats)
    {
        ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats->Lock(), -1) ;
        mStats->totalControlConnections++ ;
        mStats->activeControlConnections++ ;
    }   

    // get and save the local and peer's address
    this->peer().get_local_addr(mServerAddr) ;

    // server required to use port (L-1) where L is control port number
    mServerDataAddr = mServerAddr ;
    mServerDataAddr.set_port_number(mServerAddr.get_port_number() - 1) ; 

    this->peer().get_remote_addr(mPeerAddr) ;
    // default peer port number is also current port number - 1
    mPeerDataAddr = mPeerAddr ;
    mPeerDataAddr.set_port_number(mPeerAddr.get_port_number() - 1) ;

    char addrStr[64];
    if (mPeerAddr.addr_to_string(addrStr, sizeof(addrStr)) == -1)
        strcpy(addrStr, "invalid");

    FTP_LOG_DEBUG(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : Opened control connection to peer " << addrStr) ;

    // schedule the inactivity timeout
    StartInactivityTimer() ;    

    // prepare and queue "READY" status response
    PostResponse(FtpProtocol::FTP_STATUS_SERVICE_READY, "") ;
    
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : HandleOpenHook End") ;
    return  0 ;
}

///////////////////////////////////////////////////////////////////////////////

int ServerControlConnectionHandler::HandleInputHook(void)
{
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : HandleInputHook Start") ;
        // Is Connection closed 
    if (!IsOpen())
    {
        FTP_LOG_DEBUG(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : Already closed - ignoring input") ;
        return -1 ; 
    }

    // Receive pending data into message blocks and append to current input buffer
    std::tr1::shared_ptr<ACE_Message_Block> mb(Recv());
    if (!mb)
    {
        FTP_LOG_DEBUG(mPort, LOG_SERVER, "Bad Input MessageBlock") ;
        return -1;
    }

    if (mStats)
    {
        ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats->Lock(), -1) ;
        mStats->goodputRxBytes += mb->total_length() ; 
    }

    mInBuffer = L4L7_UTILS_NS::MessageAppend(mInBuffer, mb);
    if (!mState->IsBusy()) 
        StartInactivityTimer() ;    

    // Process the input buffer line-by-line
    L4L7_UTILS_NS::MessageIterator begin = L4L7_UTILS_NS::MessageBegin(mInBuffer.get());
    L4L7_UTILS_NS::MessageIterator end = L4L7_UTILS_NS::MessageEnd(mInBuffer.get());
    L4L7_UTILS_NS::MessageIterator pos = begin;
    
    while ((pos = std::find(pos, end, '\r')) != end)
    {
        // FTP uses "\r\n" as the line delimiter so we need to check the next character after '\r'
        if (++pos == end)
            break;

        if (*pos != '\n')
            continue;

        // Process a complete line of input
        QueueRequest((ACE_OS::gettimeofday() + GetResponseLatency()), std::string(begin, ++pos)) ;

        // Advance message read pointer and trim input buffer (discarding completely parsed message blocks)
        pos.block()->rd_ptr(pos.ptr());
        mInBuffer = L4L7_UTILS_NS::MessageTrim(mInBuffer, pos.block());
        
        begin = pos;
    }

    // Service the response queue in case responses can be sent immediately
    const ssize_t respCount = ServiceRequestQueue();
    if (respCount < 0)
    {
        FTP_LOG_DEBUG(mPort, LOG_SERVER, "ServiceRequestQueue() returned -1") ;
        return -1;
    }
    
    // [Re]start the response queue timer if responses were sent or the timer wasn't running
    if (!mReqQueue.empty() && (respCount || !IsTimerRunning(REQ_QUEUE_TIMER)))
        ScheduleTimer(REQ_QUEUE_TIMER, 0, mReqQueueTimeoutDelegate, mReqQueue.front().absTime);

    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : HandleInputHook End. mCloseFlag = " << mCloseFlag) ;
    return mCloseFlag ? -1 : 0;
}

///////////////////////////////////////////////////////////////////////////////
void ServerControlConnectionHandler::Abort() 
{
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : Abort() Start" ) ;

    // Close/Abort data connection if open
    Finalize() ;

    // Close the server connection handler
    close() ; 

    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : Abort() End") ;
}

void ServerControlConnectionHandler::Finalize(void)
{
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : Finalize Start" ) ;

    ACE_GUARD(lock_t, guard, HandlerLock());

    AbortDataConnection() ;

    // close any "to-be-cleaned" data connections now.
    CloseDataConnection() ;

    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : Finalize End" ) ;
}

///////////////////////////////////////////////////////////////////////////////

void ServerControlConnectionHandler::DataCloseCb(void *arg) 
{
    dataMgrRawPtr_t dataMgr = reinterpret_cast<dataMgrRawPtr_t>(arg) ;

    ACE_GUARD(lock_t, guard, HandlerLock());

	if (!mState || !dataMgr)
    {
        FTP_LOG_ERR(mPort, LOG_SERVER, "EXCEPTION: Invalid state or dataMgr:" << mState << ", " << dataMgr) ;
	    throw EPHXInternal() ;
    }

    // Close any active data connection for this data mgr
    CloseDataConnection(dataMgr) ;
}

///////////////////////////////////////////////////////////////////////////////
//////                    Private Methods                       ///////////////
///////////////////////////////////////////////////////////////////////////////

ACE_INET_Addr ServerControlConnectionHandler::MakeDataConnection(void) 
{    
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : MakeDataConnection Start. mDataMgr is " << (mActiveDataMgr ? "not NULL" : "NULL")) ;

    if (mActiveDataMgr)
    {
        FTP_LOG_ERR(mPort, LOG_SERVER, "EXCEPTION: Attempt to create new data connection, while current is still active") ;
        throw EPHXInternal("Attempt to create new data connection, while current is still active") ;
    }

    mActiveDataMgr = new DataTransactionManager(
        mPort,
        (FtpControlConnectionHandlerInterface&) *this, // reference to its controlling parent
        this->reactor(),      // the reactor to use.
        mActiveDataConnMode,  // active or pasv mode data connection
        true,                 // isServer
        mDataCloseCb) ;      // Data connection close notification delegate

    if (!mDataMgrs.insert(std::make_pair(
              mActiveDataMgr, dataMgrPtr_t(mActiveDataMgr))).second)
        throw EPHXInternal("Unable to add entry to data mgr map") ;

    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : MakeDataConnection End") ;

    return mActiveDataMgr->GetDataSourceAddr() ;
}

///////////////////////////////////////////////////////////////////////////////
void ServerControlConnectionHandler::CloseDataConnection(dataMgrRawPtr_t dataMgr) 
{
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : CloseDataConnection Start") ;
    
    if (!dataMgr)
    {
        // Shutdown each data connection. Do not delete since that is handled
        // by disposing the control connection in a "safe" context. 
        BOOST_FOREACH(const dataMgrMap_t::value_type& pair, mDataMgrs)
            pair.second->Shutdown() ;
    }
    else
    {
        if (dataMgr == mActiveDataMgr)
        {
            // send status notification to client.
            PostResponse(FtpProtocol::FTP_STATUS_CLOSING_DATA_CONN, "") ; 

            // close down data connection
            mActiveDataMgr->Shutdown() ;

            // reset active data pointer
            mActiveDataMgr = 0 ;

            // notify state machine to process close of the active data connection
			mState->ProcessEvent(*this, DATA_CONN_CLOSED) ;
        }
        else
        {
            dataMgrMap_t::iterator mit = mDataMgrs.find(dataMgr) ;
            if (mit != mDataMgrs.end())
            {
                mit->second->Shutdown() ;
                mDataMgrs.erase(mit) ;
            }
        }
    }

    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : CloseDataConnection End") ;
}

///////////////////////////////////////////////////////////////////////////////
void ServerControlConnectionHandler::AbortDataConnection(void) 
{
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : AbortDataConnection Start") ;
    
    // Need to check if Transactions are active outside of checking for mDataMgr
    // becaise a transaction starts w/ PORT or PASV command and in case of former,
    // we are in a transaction even if the mDataMgr is zero (since we might in the 
    // middle of setting up the transfer.
    if (mTransactionsActive && mStats)
    {
        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock()) ;
        ++mStats->abortedTransactions ;
    }

    if (mActiveDataMgr)
    {
        // Close the data connection channels right away
        mActiveDataMgr->Shutdown() ;

        // Reset active data mgr.
        mActiveDataMgr = 0 ;

        if (mTransactionsActive)
        {
            FTP_LOG_DEBUG(mPort, LOG_SERVER, "Aborting data connection...") ;
            // send back status
            PostResponse(FtpProtocol::FTP_STATUS_CONN_CLOSED_TRANSFER_ABORTED, "") ;
        }
    }

    mTransactionsActive =  0;
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : AbortDataConnection End") ;
}

///////////////////////////////////////////////////////////////////////////////
bool ServerControlConnectionHandler::HandleQuit(void)
{
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : HandleQuit Start") ;

    // Request abort of data connection
    AbortDataConnection() ;

    // set the close flag.
    mCloseFlag = true ;

    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : HandleQuit End") ;
    return PostResponse(FtpProtocol::FTP_STATUS_OK, "");
}

///////////////////////////////////////////////////////////////////////////////
void ServerControlConnectionHandler::MarkAttemptedTransaction(void)
{
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : MarkAttemptedTransaction Start") ;

    if (mActiveDataMgr)
    {
        FTP_LOG_INFO(mPort, LOG_SERVER, "FTP server [" << mServerName << 
                    "] : Detected new transaction attempt while one is already active.") ;
    }

    ++mTransactionsActive ;

    if (mStats)
    {
        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock()) ;
        ++mStats->attemptedTransactions ;
    } 
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : MarkAttempted End") ;
}

///////////////////////////////////////////////////////////////////////////////
void ServerControlConnectionHandler::MarkTransactionComplete(bool successfulTransaction) 
{
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : MarkTransactionComplete Start. sucess = " << successfulTransaction) ;

    --mTransactionsActive ;
    mActiveDataMgr = 0 ;

    if (mStats)
    {
        if (successfulTransaction)
        {
           ACE_GUARD(stats_t::lock_t, guard, mStats->Lock()) ;
           ++mStats->successfulTransactions ;
        }
        else
        {
           ACE_GUARD(stats_t::lock_t, guard, mStats->Lock()) ;
           ++mStats->unsuccessfulTransactions ;
        }
    }
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : MarkTransactionComplete End") ;
}

///////////////////////////////////////////////////////////////////////////////

bool ServerControlConnectionHandler::StartDataTx(void) 
{
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : StartDataTx Start") ;
    
    if (!mActiveDataMgr)
         MakeDataConnection() ;

    assert(mActiveDataMgr) ;

    const bool res = mActiveDataMgr->StartDataTx() ;

    if (!res)
    {
        FTP_LOG_ERR(mPort, LOG_SERVER, "Server unable to Tx. Shutdown data mgr and reset \n") ;
        ReleaseActiveDataMgr() ;  
    }

    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : StartDataTx End. res = " << res) ;
    return res ;
}

///////////////////////////////////////////////////////////////////////////////
bool ServerControlConnectionHandler::StartDataRx(void) 
{
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] :  StartDataRx Start") ;
  
    if (!mActiveDataMgr)
         MakeDataConnection() ;

    assert(mActiveDataMgr) ;

    const bool res = mActiveDataMgr->StartDataRx() ;

    if (!res)
    {
        FTP_LOG_ERR(mPort, LOG_SERVER, "Server unable to open Rx data connection. Shutdown data mgr and reset \n") ;
        ReleaseActiveDataMgr() ;
    }

    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : StartDataRx End. res = " << res) ;
    return res ;
}

///////////////////////////////////////////////////////////////////////////////
void ServerControlConnectionHandler::StartInactivityTimer(void) 
{
    ScheduleTimer(INACTIVITY_TIMER, 0, mInactivityTimeoutDelegate, 
                  ACE_OS::gettimeofday() + INACTIVITY_TIMEOUT);
}

///////////////////////////////////////////////////////////////////////////////
void ServerControlConnectionHandler::CancelInactivityTimer(void) 
{
    CancelTimer(INACTIVITY_TIMER) ;
}

///////////////////////////////////////////////////////////////////////////////
int ServerControlConnectionHandler::HandleInactivityTimeout(const ACE_Time_Value& tv, const void *act)
{
    FTP_LOG_INFO(mPort, LOG_SERVER, "FTP server [" << mServerName 
                       << "] : Inactivity timeout! Closing control connection.") ;
    // Inactivity timeout: close the connection
    return -1;
}

///////////////////////////////////////////////////////////////////////////////


int ServerControlConnectionHandler::HandleRequestQueueTimeout(const ACE_Time_Value& tv, const void *act)
{
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : HandleRequestQueueTimeout Start") ;
    // Service the response queue
    if (ServiceRequestQueue() < 0)
    {
        FTP_LOG_DEBUG(mPort, LOG_SERVER, "ServiceRequestQueue() return < 0") ;
        return -1;
    }
    // Restart the response queue timer as necessary
    if (!mReqQueue.empty())
        ScheduleTimer(REQ_QUEUE_TIMER, 0, mReqQueueTimeoutDelegate, mReqQueue.front().absTime);
        
    // If closing, return -1 to force exit.
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : HandleRequestQueueTimeout End. mCloseFlag = " << mCloseFlag) ;
    return mCloseFlag ? -1 : 0;
}

///////////////////////////////////////////////////////////////////////////////

void ServerControlConnectionHandler::QueueRequest(const ACE_Time_Value& absTime, 
                                                   const std::string& req)
{
    FTP_LOG_DEBUG(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : Got request " << req) ;
    mReqQueue.push(RequestRecord(absTime, req)) ;
}

///////////////////////////////////////////////////////////////////////////////

ssize_t ServerControlConnectionHandler::ServiceRequestQueue(void)
{
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : ServiceRequestQueue Start") ;
    const ACE_Time_Value now(ACE_OS::gettimeofday());
    ssize_t respCount = 0;

    while (!mReqQueue.empty())
    {
        // If response record at head of queue has an absolute time less than time now, we need to send that response
        const RequestRecord &req(mReqQueue.front());
        if (now < req.absTime)
            break;

        FtpProtocol::FtpRequestParser parser ;        
        if (!parser.Parse(req.request))
        {         
            if (!PostResponse(FtpProtocol::FTP_STATUS_SYNTAX_ERROR, ""))
            {
                FTP_LOG_DEBUG(mPort, LOG_SERVER, "FTP server [" << mServerName << 
                                   "] : Request parser error. Closing control connection") ;
                return -1 ;
            }
        }
        else
        {
            FtpProtocol::MethodType method = parser.GetMethodType() ;
            if (method ==  FtpProtocol::FTP_METHOD_UNKNOWN)
            {
                if (!PostResponse(FtpProtocol::FTP_STATUS_COMMAND_NOT_IMPLEMENTED, ""))
                    return -1 ;
            }  
            else
            {

                if (mStats)
                {
                    ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats->Lock(), -1) ;
                    switch (method)
                    {
                    // commands currently used by our client
                    case FtpProtocol::FTP_METHOD_USER:
                        mStats->rxUserCmd++ ;
                        break;
                    case FtpProtocol::FTP_METHOD_PASSWORD:
                        mStats->rxPassCmd++ ;
                        break;
                    case FtpProtocol::FTP_METHOD_TYPE:
                        mStats->rxTypeCmd++ ;
                        break;
                    case FtpProtocol::FTP_METHOD_PORT:
                        mStats->rxPortCmd++ ;
                        break;
                    case FtpProtocol::FTP_METHOD_RETR:
                        mStats->rxRetrCmd++ ;                        
                        break;
                    case FtpProtocol::FTP_METHOD_QUIT:            
                        mStats->rxQuitCmd++ ;
                        break;
                    default:
                        break;;
                    }
                }

                if (!mState->ProcessInput(*this, method, parser.GetConstRequestString()))
                {
                    FTP_LOG_DEBUG(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : " << 
                                       mState->Name() << ".ProcessInput returned false processing method " << 
                                       method << " with args of " << parser.GetConstRequestString()) ;
                    return -1 ;        
                }
            }
        }      

        // de-queue the request
        mReqQueue.pop();
        respCount++;

        if (mCloseFlag)
            return -1 ;// this will also force the connection to close
    }
    FTP_LOG_TRACE(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : ServiceRequestQueue End. respCount = " << respCount) ;
    return respCount;
}

///////////////////////////////////////////////////////////////////////////////
bool ServerControlConnectionHandler::PostResponse(
    FtpProtocol::StatusCode theCode, 
    const std::string &optionalText) 
{
    std::string response = FtpProtocol::BuildStatusLine(theCode, optionalText) ;  
    FTP_LOG_DEBUG(mPort, LOG_SERVER, "FTP server [" << mServerName << "] : Posting response: " << response) ;

    if (mStats)
    {
        ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats->Lock(), 0) ;
 
        switch (theCode)
        {
        case FtpProtocol::FTP_STATUS_FILE_STATUS_OK_OPENING_DATA_CONN:
            ++mStats->txResponseCode150 ;
            break;

        case FtpProtocol::FTP_STATUS_OK:
            ++mStats->txResponseCode200 ;
            break;

        case FtpProtocol::FTP_STATUS_SERVICE_READY:
            ++mStats->txResponseCode220 ;
            break;

        case FtpProtocol::FTP_STATUS_CLOSING_DATA_CONN:
            ++mStats->txResponseCode226 ;
            break;

        case FtpProtocol::FTP_STATUS_ENTERING_PASSIVE_MODE:
            ++mStats->txResponseCode227 ;
            break;

        case FtpProtocol::FTP_STATUS_USER_LOGGED_IN :
            ++mStats->txResponseCode230 ;
            break;

        case FtpProtocol::FTP_STATUS_NEED_PASSWORD:
            ++mStats->txResponseCode331 ;
            break;

        case FtpProtocol::FTP_STATUS_DATA_CONN_FAILED:
            ++mStats->txResponseCode425 ;
            break;

        case FtpProtocol::FTP_STATUS_CONN_CLOSED_TRANSFER_ABORTED:
            ++mStats->txResponseCode426 ;
            break;

        case FtpProtocol::FTP_STATUS_ACTION_NOT_TAKEN:
            ++mStats->txResponseCode452 ;
            break;

        case FtpProtocol::FTP_STATUS_SYNTAX_ERROR:
            ++mStats->txResponseCode500 ;
            break;

        case FtpProtocol::FTP_STATUS_COMMAND_NOT_IMPLEMENTED:
            ++mStats->txResponseCode502 ;
            break;

        case FtpProtocol::FTP_STATUS_NOT_LOGGED_IN:
            ++mStats->txResponseCode530 ;
            break;

        case FtpProtocol::FTP_STATUS_DATA_CONN_ALREADY_OPEN: // Unused right now. TODO remove
        case FtpProtocol::FTP_STATUS_FILE_ACTION_COMPLETE:   // Unused right now. TODO remove
        default:
           break;
        }
    }
    return Send(response) ;
}

///////////////////////////////////////////////////////////////////////////////
void ServerControlConnectionHandler::ReleaseActiveDataMgr(void)
{
    if (mActiveDataMgr)
    {        
        // Post for callback.
        if (mDataCloseCb)
            mDataCloseCb((FtpControlConnectionHandlerInterface *) this, mActiveDataMgr) ;

	    mActiveDataMgr = 0 ;
    }
}

///////////////////////////////////////////////////////////////////////////////
void ServerControlConnectionHandler::UpdateResponseTimings(void) 
{
    const L4L7_BASE_NS::ResponseTimingOptions responseTimingType = 
        static_cast<L4L7_BASE_NS::ResponseTimingOptions>(mRespConfig.L4L7ServerResponseConfig.ResponseTimingType) ;

    if (responseTimingType == L4L7_BASE_NS::ResponseTimingOptions_FIXED)
        mRespLatency.Set(mRespConfig.L4L7ServerResponseConfig.FixedResponseLatency) ;
    else
    {
        mRespLatency.Set(
            mRespConfig.L4L7ServerResponseConfig.RandomResponseLatencyMean, 
            mRespConfig.L4L7ServerResponseConfig.RandomResponseLatencyStdDeviation) ;
    }
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
////////////     Control Connection State Machine              ////////////////
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
        The state machine for the server is "loosely" implemented. It will
        work as expected only for 'well behaved' clients....
*******************************************************************************/
bool ServerControlConnectionHandler::LoginState::ProcessInput(
    ServerControlConnectionHandler& connHandler, 
    FtpProtocol::MethodType method,
    const std::string &request) const
{
    bool status(false) ;
    // Validate the method
    switch (method)
    {
      case FtpProtocol::FTP_METHOD_USER:
          status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_NEED_PASSWORD, "");
          break;

      case FtpProtocol::FTP_METHOD_PASSWORD:
          status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_USER_LOGGED_IN, "");
          ChangeState(connHandler, &ServerControlConnectionHandler::WAIT_STATE) ;
          break;

      case FtpProtocol::FTP_METHOD_QUIT:
          status = HandleQuit(connHandler) ;
          break ;

      default:
          // All other methods are not allowed
          status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_NOT_LOGGED_IN, "");
          break;
    }
    return status ;
}

bool ServerControlConnectionHandler::LoginState::ProcessEvent(
    ServerControlConnectionHandler& connHandler,
    ServerControlConnectionHandler::EventTypes event) const 
{
    if (event == DATA_CONN_CLOSED)
        return false ;
    return true ;
}

bool ServerControlConnectionHandler::LoginState::IsBusy() const 
{
    return false ;
}

///////////////////////////////////////////////////////////////////////////////

bool ServerControlConnectionHandler::WaitingState::ProcessInput(    
    ServerControlConnectionHandler& connHandler, 
    FtpProtocol::MethodType method,
    const std::string &request) const

{
    bool status(false) ;

    // Validate the method
    switch (method)
    {
    case FtpProtocol::FTP_METHOD_USER:
    case FtpProtocol::FTP_METHOD_PASSWORD:    
        status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_USER_LOGGED_IN, "");
        break;

    case FtpProtocol::FTP_METHOD_TYPE:
        // only supports binary mode data transfers.
        if (request.find("I") != std::string::npos)
            status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_OK, "");
        else
            status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_COMMAND_NOT_IMPLEMENTED, "");                

        break ;          

    case FtpProtocol::FTP_METHOD_PORT:
        MarkAttemptedTransaction(connHandler) ;
        if (RequestsRemaining(connHandler))
        {
            RequestCompleted(connHandler) ;     
            
            FtpProtocol::FtpPortRequestParser portParser ;
            if (!portParser.Parse(request))
            {       
                MarkTransactionComplete(connHandler, false) ;
                status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_SYNTAX_ERROR, "");
                break ;
            }
            else
            {
                UpdatePeerDataAddress(connHandler, portParser.GetPortInetAddr()) ;
                SetInActiveDataConnectionMode(connHandler, true) ;               
                ChangeState(connHandler, &ServerControlConnectionHandler::XFER_SETUP_STATE) ;
                status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_OK, "") ;
            }
        }
        else
        {
            MarkTransactionComplete(connHandler, false) ;
            status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_ACTION_NOT_TAKEN, "") ;
        }

        break ;
          
    case FtpProtocol::FTP_METHOD_PASV:   
        MarkAttemptedTransaction(connHandler) ;
        if (RequestsRemaining(connHandler))
        {
            RequestCompleted(connHandler) ;     

            SetInActiveDataConnectionMode(connHandler, false) ;
            ACE_INET_Addr addr = MakeDataConnection(connHandler) ;
            if (addr.get_port_number() != 0)
            {
                ChangeState(connHandler, &ServerControlConnectionHandler::XFER_SETUP_STATE) ;
                status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_ENTERING_PASSIVE_MODE, FtpProtocol::BuildPasvPortString(addr)) ;
            }
            else
            {
                status = false ;
                FTP_LOG_ERR(connHandler.mPort, LOG_SERVER, "Server unable to create acceptor for passive mode data xfer") ;
            }
        }
        if (!status)
        {
            MarkTransactionComplete(connHandler, false) ;
            status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_ACTION_NOT_TAKEN, "") ;
        }                
        break ;    

    case  FtpProtocol::FTP_METHOD_STOR:
    case  FtpProtocol::FTP_METHOD_RETR:
        status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_ACTION_NOT_TAKEN, "") ;
        break ;

    case FtpProtocol::FTP_METHOD_QUIT:
        FTP_LOG_DEBUG(GetPort(connHandler), LOG_SERVER, "FTP server [" << connHandler.mServerName 
                           << "] : Got QUIT request.") ;
        status = HandleQuit(connHandler) ;
        break ;

    default:
        // No other commands recoginized...
        status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_COMMAND_NOT_IMPLEMENTED, "");
        break;
    }
    return status ;
}

bool ServerControlConnectionHandler::WaitingState::ProcessEvent(
    ServerControlConnectionHandler& connHandler,
    ServerControlConnectionHandler::EventTypes event) const 
{
    if (event == DATA_CONN_CLOSED)
        return false ;
    return true ;
}

bool ServerControlConnectionHandler::WaitingState::IsBusy() const 
{
    return false ;
}

///////////////////////////////////////////////////////////////////////////////

bool ServerControlConnectionHandler::XferSetupState::ProcessInput(    
    ServerControlConnectionHandler& connHandler, 
    FtpProtocol::MethodType method,
    const std::string &request) const

{
    bool status(false) ;

    // Validate the method
    switch (method)
    {
    case FtpProtocol::FTP_METHOD_STOR:
        {                   
            FTP_LOG_DEBUG(GetPort(connHandler), LOG_SERVER, "FTP server [" << connHandler.mServerName << "] : Got STOR request.") ;
            status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_FILE_STATUS_OK_OPENING_DATA_CONN, "");
            if (status && StartDataRx(connHandler))
            {
                CancelInactivityTimer(connHandler) ;
                ChangeState(connHandler, &ServerControlConnectionHandler::DATA_STATE) ;
            }
            else
            {
                status = false ;
                MarkTransactionComplete(connHandler, false) ;
                ChangeState(connHandler, &ServerControlConnectionHandler::WAIT_STATE) ;
                PostResponse(connHandler, FtpProtocol::FTP_STATUS_DATA_CONN_FAILED, "") ;
            }
        }

        break ;

    case FtpProtocol::FTP_METHOD_RETR:
        {
            FTP_LOG_DEBUG(GetPort(connHandler), LOG_SERVER, "FTP server [" << connHandler.mServerName 
                               << "] : Got RETR request.") ;
            status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_FILE_STATUS_OK_OPENING_DATA_CONN, "");
            if (status && StartDataTx(connHandler))
            {
                CancelInactivityTimer(connHandler) ;
                ChangeState(connHandler, &ServerControlConnectionHandler::DATA_STATE) ;
            }
            else
            {
                status = false ;
                MarkTransactionComplete(connHandler, false) ;
                ChangeState(connHandler, &ServerControlConnectionHandler::WAIT_STATE) ;
                PostResponse(connHandler, FtpProtocol::FTP_STATUS_DATA_CONN_FAILED, "") ;
            }
        }

        break ;

    case FtpProtocol::FTP_METHOD_QUIT:
        FTP_LOG_DEBUG(GetPort(connHandler), LOG_SERVER, "FTP server [" << connHandler.mServerName 
                           << "] : Got QUIT request.") ;
        status = HandleQuit(connHandler) ;
        break ;

    default:
        // No other commands recoginized...
        status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_ACTION_NOT_TAKEN, "");
        break;
    }
    return status ;
}

bool ServerControlConnectionHandler::XferSetupState::ProcessEvent(
    ServerControlConnectionHandler& connHandler,
    ServerControlConnectionHandler::EventTypes event) const 
{
    if (event == DATA_CONN_CLOSED)
        return false ;
    return true ;
}

bool ServerControlConnectionHandler::XferSetupState::IsBusy() const 
{
    return false ;
}

///////////////////////////////////////////////////////////////////////////////

bool ServerControlConnectionHandler::DataXferState::ProcessInput(
    ServerControlConnectionHandler& connHandler, 
    FtpProtocol::MethodType method,
    const std::string &request) const
{
    bool status(false) ;

    // Validate the method
    switch (method)
    {
    case FtpProtocol::FTP_METHOD_USER:
    case FtpProtocol::FTP_METHOD_PASSWORD:
    case FtpProtocol::FTP_METHOD_TYPE:
    case FtpProtocol::FTP_METHOD_STOR:
    case FtpProtocol::FTP_METHOD_RETR:
        status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_ACTION_NOT_TAKEN, "") ;
        break ;

    case FtpProtocol::FTP_METHOD_PORT:
    case FtpProtocol::FTP_METHOD_PASV:
    case FtpProtocol::FTP_METHOD_QUIT:
        {
            FTP_LOG_DEBUG(GetPort(connHandler), LOG_SERVER, "FTP server [" << connHandler.mServerName 
                               << "] : Aborting DataXfer due to request: " << request) ;
            // in this case, we are required to abort data connection and process port/pasv change...
            AbortDataConnection(connHandler) ;
            // make connection handler point to new state.
            ChangeState(connHandler, &ServerControlConnectionHandler::WAIT_STATE) ;
            // restart inactivity timer
            RestartInactivityTimer(connHandler) ;
            // directly ask new state to process input.
            return ServerControlConnectionHandler::WAIT_STATE.ProcessInput(connHandler, method, request) ;
        }
    default:
        status = PostResponse(connHandler, FtpProtocol::FTP_STATUS_COMMAND_NOT_IMPLEMENTED, "") ;
        break ;
    }
    return status ;
}

bool ServerControlConnectionHandler::DataXferState::ProcessEvent(
    ServerControlConnectionHandler& connHandler,
    ServerControlConnectionHandler::EventTypes event) const 
{
    if (event == DATA_CONN_CLOSED)
    {        
        MarkTransactionComplete(connHandler, true) ;
        ChangeState(connHandler, &ServerControlConnectionHandler::WAIT_STATE) ;
        RestartInactivityTimer(connHandler) ;
    }
    return true ;
}

bool ServerControlConnectionHandler::DataXferState::IsBusy() const 
{
    return true ;
}

///////////////////////////////////////////////////////////////////////////////
