// @file
/// @brief Client Control Connection Handler implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <cstdlib>
#include <sstream>

#include <ace/Message_Block.h>
#include <boost/foreach.hpp>

#include "ClientControlConnectionHandler.h"
#include "ClientControlConnectionHandlerStates.h"
#include "DataTransactionManager.h"
#include "FtpClientRawStats.h"
#include "FtpProtocol.h"
#include "FtpdLog.h"

USING_FTP_NS;

///////////////////////////////////////////////////////////////////////////////

const ClientControlConnectionHandler::LoginState                        ClientControlConnectionHandler::LOGIN_STATE;
const ClientControlConnectionHandler::XferSetupState                    ClientControlConnectionHandler::XFERSETUP_STATE;
const ClientControlConnectionHandler::PasvDataXferState                 ClientControlConnectionHandler::PASV_DATAXFER_STATE;
const ClientControlConnectionHandler::PortDataXferState                 ClientControlConnectionHandler::PORT_DATAXFER_STATE;
const ClientControlConnectionHandler::DataXferPendingConfirmationState  ClientControlConnectionHandler::DATAXFER_PENDING_CONFIRMATION; 
const ClientControlConnectionHandler::ExitState                         ClientControlConnectionHandler::EXIT_STATE;

const ACE_Time_Value ClientControlConnectionHandler::SERVER_RESPONSE_TIMEOUT(5 * 60, 0);  // 5 minutes
///////////////////////////////////////////////////////////////////////////////////
//// Public APIs
//////////////////////////////////////////////////////////////////////////////////

ClientControlConnectionHandler::ClientControlConnectionHandler(uint32_t serial)
    : L4L7_APP_NS::StreamSocketHandler(serial),
      mMaxTransactions(1),
      mAttemptedTransactions(0),
      mTransactionActive(false),
      mStats(0),
      mL4L7Profile(0),
      mState(&LOGIN_STATE),
      mActiveServerConnection(false),
      mFileOpTx(false),
      mDataConnErrorFlag(false) ,
      mSzType(0),
      mContentType(0),
      mFixedSz(0),
      mRandomSzMean(0),
      mRandomStdDev(0),
      mActiveDataMgr(0),
      mPort(0),
      mComplete(false),
      mLocalIfName(""),
      mLastCmd(FtpProtocol::FTP_METHOD_UNKNOWN),
      mDataConnXferBytes(0),
      mLastReqTime(0,0),
      mDataConnReqTime(0,0),
      mDataConnOpenTime(0,0),
      mDataConnCloseTime(0,0),
      mResponseTimeoutDelegate(fastdelegate::MakeDelegate(this, &ClientControlConnectionHandler::HandleResponseTimeout))
{
}

///////////////////////////////////////////////////////////////////////////////

ClientControlConnectionHandler::~ClientControlConnectionHandler()
{
    UnregisterUpdateTxCountDelegate();
}

///////////////////////////////////////////////////////////////////////////////
void ClientControlConnectionHandler::SetStatsInstance(stats_t& stats)
{
    mStats = &stats;
    RegisterUpdateTxCountDelegate(boost::bind(&ClientControlConnectionHandler::UpdateGoodputTxSent, this, _1));
}

///////////////////////////////////////////////////////////////////////////////

void ClientControlConnectionHandler::UpdateGoodputTxSent(uint64_t sent)
{
    if (mStats)
    {
        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
        mStats->goodputTxBytes += sent;
    }
}

///////////////////////////////////////////////////////////////////////////////
void ClientControlConnectionHandler::NotifyDataConnectionOpen(const ACE_Time_Value& connectTime) 
{
    FTP_LOG_TRACE(mPort, LOG_CLIENT, "NotifyDataConnectionOpen Start") ;

    mDataConnOpenTime = connectTime ;

    FTP_LOG_TRACE(mPort, LOG_CLIENT, "NotifyDataConnectionOpen End") ;
}

///////////////////////////////////////////////////////////////////////////////
void ClientControlConnectionHandler::NotifyIncrementalTxBytes(uint32_t bytes)
{
    FTP_LOG_TRACE(mPort, LOG_CLIENT, "NotifyIncrementalTxBytes Start") ;

    mDataConnXferBytes += bytes ;
    UpdateGoodputTxSent(bytes);

    FTP_LOG_TRACE(mPort, LOG_CLIENT, "NotifyIncrementalTxBytes End") ;
}

///////////////////////////////////////////////////////////////////////////////

void ClientControlConnectionHandler::NotifyIncrementalRxBytes(uint32_t bytes) 
{
    FTP_LOG_TRACE(mPort, LOG_CLIENT, "NotifyIncrementalRxBytes Start") ;

    mDataConnXferBytes += bytes ;
    if (mStats)
    {
        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock()) ;
        mStats->goodputRxBytes += bytes ; 
    }

    FTP_LOG_TRACE(mPort, LOG_CLIENT, "NotifyIncrementalRxBytes End") ;
}

///////////////////////////////////////////////////////////////////////////////
void ClientControlConnectionHandler::NotifyDataConnectionClose(const ACE_Time_Value& closeTime) 
{
    FTP_LOG_TRACE(mPort, LOG_CLIENT, "NotifyDataConnectionClose Start") ;

    mDataConnCloseTime = closeTime;

    FTP_LOG_TRACE(mPort, LOG_CLIENT, "NotifyDataConnectionClose End") ;
}

///////////////////////////////////////////////////////////////////////////////
//// Protected APIs
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////	
void ClientControlConnectionHandler::Close(void) 
{
    FTP_LOG_TRACE(mPort, LOG_CLIENT, "Close Start") ;

    // Do any data connection clean-up
    Finalize() ;  

    // close the control connection itself
    close() ;      

    FTP_LOG_TRACE(mPort, LOG_CLIENT, "Close End") ;
}

///////////////////////////////////////////////////////////////////////////////	
void ClientControlConnectionHandler::Finalize(void) 
{
    FTP_LOG_TRACE(mPort, LOG_CLIENT, "Finalize Start") ;
    
    ACE_GUARD(lock_t, guard, HandlerLock());

    CloseDataConnection() ;

    if (mTransactionActive && mStats)
    {    
        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock()) ;
        ++mStats->abortedTransactions ;
    }

    mTransactionActive = false ;
    FTP_LOG_TRACE(mPort, LOG_CLIENT, "Finalize End") ;
}
///////////////////////////////////////////////////////////////////////////////

void ClientControlConnectionHandler::DataCloseCb(void *arg) 
{
    FTP_LOG_TRACE(mPort, LOG_CLIENT, "DataCloseCb Start") ;

    dataMgrRawPtr_t dataMgr = reinterpret_cast<dataMgrRawPtr_t>(arg) ;

    ACE_GUARD(lock_t, guard, HandlerLock());

    // expect to have a valid state and the input to be non-null
    if (!mState || !dataMgr)
    {
        FTP_LOG_ERR(mPort, LOG_CLIENT, "EXCEPTION: Invalid state or dataMgr:" << mState << ", " << dataMgr) ;
        throw EPHXInternal() ;   
    }
    
    // Close the data connection, if open
    CloseDataConnection(dataMgr) ;

    // inform the state machine about this event...
    mState->ProcessEvent(*this, DATA_CONN_CLOSED) ;

    FTP_LOG_TRACE(mPort, LOG_CLIENT, "DataCloseCb End") ;
}

///////////////////////////////////////////////////////////////////////////////
//// Private APIs
///////////////////////////////////////////////////////////////////////////////


int ClientControlConnectionHandler::HandleOpenHook(void)
{    
    FTP_LOG_TRACE(mPort, LOG_CLIENT, "HandleOpenHook Start") ;

    // Get Local address and setup local data address
    peer().get_local_addr(mLocalAddr);
    mLocalDataAddr = mLocalAddr ;
    mLocalDataAddr.set_port_number(mLocalAddr.get_port_number()  - 1) ;

    // Get remote address and setup default remote data address
    peer().get_remote_addr(mPeerAddr) ;
    mPeerDataAddr = mPeerAddr ;
    mPeerDataAddr.set_port_number(mPeerAddr.get_port_number()  - 1) ;

    FTP_LOG_TRACE(mPort, LOG_CLIENT, "HandleOpenHook End") ;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ClientControlConnectionHandler::HandleInputHook(void)
{
    FTP_LOG_TRACE(mPort, LOG_CLIENT, "HandleInputHook Start") ;

    // Is Connection closed 
    if (!IsOpen())
    {
        FTP_LOG_DEBUG(mPort, LOG_SERVER, "FTP Client : Already closed - ignoring input") ;
        return -1 ; 
    }

    // Receive pending data into message blocks and append to current input buffer
    messagePtr_t mb(Recv());
    if (!mb)
        return -1;

    if (mStats)
    {
        const size_t rxBytes = mb->total_length() ;
        ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats->Lock(), -1) ;
        mStats->goodputRxBytes += rxBytes ;
    }

    mInBuffer = L4L7_UTILS_NS::MessageAppend(mInBuffer, mb);	

	// Process the input buffer line-by-line
    L4L7_UTILS_NS::MessageIterator begin = L4L7_UTILS_NS::MessageBegin(mInBuffer.get());
    L4L7_UTILS_NS::MessageIterator end = L4L7_UTILS_NS::MessageEnd(mInBuffer.get());
    L4L7_UTILS_NS::MessageIterator pos = begin;

	const ACE_Time_Value now(ACE_OS::gettimeofday());

    while ((pos = std::find(pos, end, '\r')) != end)
    {
        // FTP uses "\r\n" as the line delimiter so we need to check the next character after '\r'
        if (++pos == end)
            break;

        if (*pos != '\n')
            continue;


        FtpProtocol::FtpResponseParser parser ;
        const bool parseResult = parser.Parse(begin, ++pos) ;
        if (!parseResult ||
            // if we are tallking to our server, this is an error case too!
            (parser.GetStatusCode() == FtpProtocol::FTP_STATUS_CODE_INVALID) 
           )
        {
            FTP_LOG_ERR(mPort, LOG_CLIENT, "Detected parse failure or invalid code (" << 
                                             parser.GetStatusCode() << 
                                             ") on server response...Exitting client" << 
                                            std::endl) ;
            return  -1 ;
        }

        CancelResponseTimer() ;

        if (mStats)
        {
            ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats->Lock(), -1) ;
            switch (parser.GetStatusCode())
            {            
            case FtpProtocol::FTP_STATUS_FILE_STATUS_OK_OPENING_DATA_CONN:
                ++mStats->rxResponseCode150 ;
                break;

            case FtpProtocol::FTP_STATUS_OK:
                ++mStats->rxResponseCode200 ;
                break;

            case FtpProtocol::FTP_STATUS_SERVICE_READY:
                ++mStats->rxResponseCode220 ;
                break;

            case FtpProtocol::FTP_STATUS_CLOSING_DATA_CONN:
                ++mStats->rxResponseCode226 ;
                break;

            case FtpProtocol::FTP_STATUS_ENTERING_PASSIVE_MODE:
                ++mStats->rxResponseCode227 ;
                break;
                
            case FtpProtocol::FTP_STATUS_USER_LOGGED_IN :
                ++mStats->rxResponseCode230 ;
                break;

            case FtpProtocol::FTP_STATUS_NEED_PASSWORD:
                ++mStats->rxResponseCode331 ;
                break;

            case FtpProtocol::FTP_STATUS_DATA_CONN_FAILED:
                ++mStats->rxResponseCode425 ;
                break;

            case FtpProtocol::FTP_STATUS_CONN_CLOSED_TRANSFER_ABORTED:
                ++mStats->rxResponseCode426 ;
                break;

            case FtpProtocol::FTP_STATUS_ACTION_NOT_TAKEN:
                ++mStats->rxResponseCode452 ;
                break;

            case FtpProtocol::FTP_STATUS_SYNTAX_ERROR:
                ++mStats->rxResponseCode500 ;
                break;

            case FtpProtocol::FTP_STATUS_COMMAND_NOT_IMPLEMENTED:
                ++mStats->rxResponseCode502 ;
                break;

            case FtpProtocol::FTP_STATUS_NOT_LOGGED_IN:
                ++mStats->rxResponseCode530 ;
                break;

            case FtpProtocol::FTP_STATUS_DATA_CONN_ALREADY_OPEN: // Unused right now. TODO remove
            case FtpProtocol::FTP_STATUS_FILE_ACTION_COMPLETE:   // Unused right now. TODO remove
            default:
                break;
            }
        }

        if (!mState->ProcessInput(*this, now, parser.GetStatusCode(), parser.GetStatusMsg()))
        {
            FTP_LOG_DEBUG(mPort, LOG_CLIENT, "HandleInputHook: ProcessInput returned -1. Current state = " << 
                               mState->Name()) ;
			return -1; 
        }

        if (mActiveDataMgr && mDataConnErrorFlag)
        {
            FTP_LOG_DEBUG(mPort, LOG_CLIENT, "HandleInputHook: Detected data connection error") ;

            if (mDataCloseCb)
                mDataCloseCb((FtpControlConnectionHandlerInterface *) this, mActiveDataMgr) ;

            mActiveDataMgr = 0; 
        }

        // Advance message read pointer and trim input buffer (discarding completely parsed message blocks)
        pos.block()->rd_ptr(pos.ptr());
        mInBuffer = L4L7_UTILS_NS::MessageTrim(mInBuffer, pos.block());
        
        begin = pos;
    }

    FTP_LOG_TRACE(mPort, LOG_CLIENT, "HandleInputHook End") ;
    return 0 ;
}

///////////////////////////////////////////////////////////////////////////////
void ClientControlConnectionHandler::StartResponseTimer() 
{
    ScheduleTimer(RESPONSE_TIMEOUT, 0, mResponseTimeoutDelegate, 
                  ACE_OS::gettimeofday() + SERVER_RESPONSE_TIMEOUT);
}    

///////////////////////////////////////////////////////////////////////////////
void ClientControlConnectionHandler::CancelResponseTimer() 
{
    CancelTimer(RESPONSE_TIMEOUT) ;
}

///////////////////////////////////////////////////////////////////////////////

int ClientControlConnectionHandler::HandleResponseTimeout(const ACE_Time_Value& tv, const void *act)
{
    // use debug log levels here since this is a fairly signficant event
    FTP_LOG_DEBUG(mPort, LOG_CLIENT, "HandleResponseTimeout Start") ;

    if (mActiveDataMgr && mActiveDataMgr->IsConnectionActive())
    {
        FTP_LOG_DEBUG(mPort, LOG_CLIENT, "Response timeout auto cancelled since data connection is present") ;
        return 0 ; // if data connection is active, then simply return.
    }

    if (mStats && (mState == &DATAXFER_PENDING_CONFIRMATION))
    {
        ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats->Lock(), -1) ;
        ++mStats->abortedTransactions ;
    }

    // Inactivity timeout: close the connection
    FTP_LOG_DEBUG(mPort, LOG_CLIENT, "HandleResponseTimeout End") ;
    return -1;
}

///////////////////////////////////////////////////////////////////////////////
ACE_INET_Addr ClientControlConnectionHandler::MakeDataConnection(void) 
{
    FTP_LOG_TRACE(mPort, LOG_CLIENT, "MakeDataConnection Start") ;

    mActiveDataMgr = new DataTransactionManager(
        mPort,
        (FtpControlConnectionHandlerInterface&) *this, // reference to its controlling parent
        this->reactor(),      // the reactor to use.
        mActiveServerConnection,  // active or pasv mode data connectionnn
        false,         // !isServer
        mDataCloseCb,  // data close notification callback
        false,  // do not use inactivity timeout
        GetEnableBwCtrl()  // Is BW enabled on the client
        );

    if (mActiveDataMgr)
        mActiveDataMgr->SetDynamicLoadDelegates(mGetDynamicLoadBytesDelegate, mProduceDynamicLoadBytesDelegate, mConsumeDynamicLoadBytesDelegate);

    if (!mDataMgrs.insert(std::make_pair(
              mActiveDataMgr, dataMgrPtr_t(mActiveDataMgr))).second)
        throw EPHXInternal("Unable to add entry to data mgr map") ;

    FTP_LOG_TRACE(mPort, LOG_CLIENT, "MakeDataConnection End") ;

    return mActiveDataMgr->GetDataSourceAddr() ;
}

/////////////////////////////////////////////////////////////////////////////////
void ClientControlConnectionHandler::CloseDataConnection(dataMgrRawPtr_t dataMgr) 
{
    FTP_LOG_TRACE(mPort, LOG_CLIENT, "CloseDataConnection Start") ;
  
    // Shutdown and release memory associated with given dataMgr(s).
    
    if (!dataMgr)
    {
        // Shutdown each data connection. Do not delete since that is handled
        // by disposing the control connection in a "safe" context. 
        BOOST_FOREACH(const dataMgrMap_t::value_type& pair, mDataMgrs)
            pair.second->Shutdown() ;
    }
    else
    {
        dataMgrMap_t::iterator mit = mDataMgrs.find(dataMgr) ;
        if (mit != mDataMgrs.end())
        {
            mit->second->Shutdown() ;
            mDataMgrs.erase(mit) ;
        }
        if (dataMgr == mActiveDataMgr)
            mActiveDataMgr = 0 ;
    }

    FTP_LOG_TRACE(mPort, LOG_CLIENT, "CloseDataConnection End") ;
}

///////////////////////////////////////////////////////////////////////////////

bool ClientControlConnectionHandler::StartDataTx(void) 
{
    FTP_LOG_TRACE(mPort, LOG_CLIENT, "StartDataTx Start") ;
    
    if (!mActiveDataMgr)
        MakeDataConnection() ;

    assert (mActiveDataMgr) ;

    const bool res = mActiveDataMgr->StartDataTx() ;

    if (!res)
    {
        FTP_LOG_ERR(mPort, LOG_CLIENT, "Client unable to Tx data. Shutdown data mgr and reset.") ;
        if (mDataCloseCb)
            mDataCloseCb((FtpControlConnectionHandlerInterface *) this, mActiveDataMgr) ; 
    }

    FTP_LOG_TRACE(mPort, LOG_CLIENT, "StartDataTx End") ;

    return res ;
}

///////////////////////////////////////////////////////////////////////////////

bool ClientControlConnectionHandler::StartDataRx(void) 
{
    FTP_LOG_TRACE(mPort, LOG_CLIENT, "StartDataRx Start") ;

    if (!mActiveDataMgr)
        MakeDataConnection() ;

    assert (mActiveDataMgr) ;

    const bool res = mActiveDataMgr->StartDataRx() ;
    if (!res)
    {
        FTP_LOG_ERR(mPort, LOG_CLIENT, "Client unable to open Rx data connection. Shutdown data mgr and reset.") ;
        if (mDataCloseCb)
            mDataCloseCb((FtpControlConnectionHandlerInterface *) this, mActiveDataMgr) ; 
    }

    FTP_LOG_TRACE(mPort, LOG_CLIENT, "StartDataRx End") ;
    return res ;
}

///////////////////////////////////////////////////////////////////////////////

bool ClientControlConnectionHandler::SendRequest(
    FtpProtocol::MethodType method, 
    const std::string &reqToken)
{ 
    std::string reqString = FtpProtocol::BuildRequestLine(method, reqToken) ;
   
    mLastReqTime = ACE_OS::gettimeofday() ; 
    mLastCmd = method ;

    FTP_LOG_DEBUG(mPort, LOG_CLIENT, "Send Request: ClientState: " << mState->Name() << 
                                     " : Sending request: " << 
                                      reqString) ;

    const bool ret = Send(reqString) ;

    // start response timer
    if (ret)
        StartResponseTimer() ;

    FTP_LOG_TRACE(mPort, LOG_CLIENT, "SendRequest End") ;
    return ret ;
}

///////////////////////////////////////////////////////////////////////////////

void ClientControlConnectionHandler::TransactionInitated() 
{
    FTP_LOG_TRACE(mPort, LOG_CLIENT, "TransactionInitated Start") ;

    ++mAttemptedTransactions ;
    mTransactionActive = true ;

    if (mActiveDataMgr)
    {
        FTP_LOG_INFO(mPort, LOG_CLIENT, "Active Data Connection while new transaction is being initiated") ;
    }

    // reset data connection related parameters
    mDataConnXferBytes = 0 ;
    mDataConnOpenTime.set(0,0) ;
    mDataConnCloseTime.set(0,0) ;
    mDataConnReqTime.set(0,0) ;

    if (mStats)
    {
        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock()) ;
        ++mStats->attemptedTransactions ;
    }

    FTP_LOG_TRACE(mPort, LOG_CLIENT, "TransactionInitated End") ;
}

///////////////////////////////////////////////////////////////////////////////


void ClientControlConnectionHandler::TransactionComplete(bool successful) 
{
    FTP_LOG_TRACE(mPort, LOG_CLIENT, "TransactionComplete Start") ;

    mActiveDataMgr = 0 ;
    mTransactionActive = false ;

    if (mStats)
    {       
        if (successful)
        {
            if (mDataConnCloseTime.msec() == 0)
            {
                // sometimes transactions complete without close notification
                mDataConnCloseTime = ACE_OS::gettimeofday();
            }

            ACE_Time_Value dataConnDuration(mDataConnCloseTime - mDataConnOpenTime) ;
            unsigned long msec = std::max<unsigned long>(dataConnDuration.msec(), 1) ;
            uint64_t bps = static_cast<uint64_t>(static_cast<double>(mDataConnXferBytes) / msec) * 8000 ;
            {
                ACE_GUARD(stats_t::lock_t, guard, mStats->Lock()) ;
                ++mStats->successfulTransactions ;
                mStats->mCumulativeFileTransferBps += bps;
            }
        }                
        else
        {
            ACE_GUARD(stats_t::lock_t, guard, mStats->Lock()) ;
            ++mStats->unsuccessfulTransactions;
        }            
    }

    FTP_LOG_TRACE(mPort, LOG_CLIENT, "TransactionComplete End") ;
}

///////////////////////////////////////////////////////////////////////////////
//// Private:: Client state machine 
///////////////////////////////////////////////////////////////////////////////

bool ClientControlConnectionHandler::LoginState::ProcessInput(ClientControlConnectionHandler& connHandler, 
                                                             const ACE_Time_Value& now,
                                                             FtpProtocol::StatusCode code, 
                                                             const std::string& respLine) const
{
    bool status(false) ;
    FtpProtocol::MethodType lastCmd = GetLastCmd(connHandler) ;

    switch (lastCmd)
    {
    case FtpProtocol::FTP_METHOD_UNKNOWN:
        {
            if (code == FtpProtocol::FTP_STATUS_SERVICE_READY)
                status = SendRequest(connHandler, FtpProtocol::FTP_METHOD_USER, "spirent") ;
        }
        break;

    case FtpProtocol::FTP_METHOD_USER:
        {
            if (code == FtpProtocol::FTP_STATUS_NEED_PASSWORD)
                status = SendRequest(connHandler, FtpProtocol::FTP_METHOD_PASSWORD, "spirent") ;
        }
        break;

    case FtpProtocol::FTP_METHOD_PASSWORD:
        {
            if (code == FtpProtocol::FTP_STATUS_USER_LOGGED_IN)    
            {
                ChangeState(connHandler, &ClientControlConnectionHandler::XFERSETUP_STATE) ;
                status = SendRequest(connHandler, FtpProtocol::FTP_METHOD_TYPE, "I") ;
            }                
        }
        break;

    default:
        break; // this is an error case in the client/server state machine 
    }

    if (!status)
    {
        ChangeState(connHandler, &ClientControlConnectionHandler::EXIT_STATE) ;
        SendRequest(connHandler, FtpProtocol::FTP_METHOD_QUIT, "") ;
    }
    return true;
}

bool ClientControlConnectionHandler::LoginState::ProcessEvent(ClientControlConnectionHandler& connHandler, 
                                                             ClientControlConnectionHandler::EventTypes eventType) const
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool ClientControlConnectionHandler::XferSetupState::ProcessInput(ClientControlConnectionHandler& connHandler, 
                                                             const ACE_Time_Value& now,
                                                             FtpProtocol::StatusCode code, 
                                                             const std::string& respLine) const
{
    bool status(false) ;
    FtpProtocol::MethodType lastCmd = GetLastCmd(connHandler) ;

    switch (lastCmd)
    {
    case FtpProtocol::FTP_METHOD_TYPE:
        {
            if (code == FtpProtocol::FTP_STATUS_OK)
            {
                if (TransactionsRemaining(connHandler))
                {
                    TransactionInitated(connHandler) ;                                      
                    if (GetActiveDataConnectionFlag(connHandler))
                    {
                        ChangeState(connHandler, &ClientControlConnectionHandler::PASV_DATAXFER_STATE) ;
                        status = SendRequest(connHandler, FtpProtocol::FTP_METHOD_PASV, "") ;
                    }                        
                    else
                    {
                        ChangeState(connHandler, &ClientControlConnectionHandler::PORT_DATAXFER_STATE) ;
                        ACE_INET_Addr local = MakeDataConnection(connHandler) ;
                        if (local.get_port_number() != 0)
                            status = SendRequest(connHandler, FtpProtocol::FTP_METHOD_PORT, FtpProtocol::BuildPortRequestString(local)) ;
                        else
                        {
                             status = false ;
                             FTP_LOG_ERR(connHandler.mPort, LOG_CLIENT, "Client unable to create acceptor for port mode data xfer") ;
                        }                       
                    }                
                }                
            }                
        }
        break;

    default:
        break; 
    }

    if (!status)
    {
        ChangeState(connHandler, &ClientControlConnectionHandler::EXIT_STATE) ;
        SendRequest(connHandler, FtpProtocol::FTP_METHOD_QUIT, "") ;
    }
    return true;
}

bool ClientControlConnectionHandler::XferSetupState::ProcessEvent(ClientControlConnectionHandler& connHandler, 
                                                             ClientControlConnectionHandler::EventTypes eventType) const
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool ClientControlConnectionHandler::PasvDataXferState::ProcessInput(ClientControlConnectionHandler& connHandler, 
                                                               const ACE_Time_Value& now,
                                                               FtpProtocol::StatusCode code, 
                                                               const std::string& respLine) const
{
    bool status(false) ;
    FtpProtocol::MethodType lastCmd= GetLastCmd(connHandler) ;

    assert(GetActiveDataConnectionFlag(connHandler) == true) ;

    switch (lastCmd)
    {
    case FtpProtocol::FTP_METHOD_PASV:
        {
            if (code == FtpProtocol::FTP_STATUS_ENTERING_PASSIVE_MODE)
            {
                FtpProtocol::FtpPasvResponseParser parser ;
                status = parser.Parse(respLine) ;
                if (status)
                {
                    UpateRemoteDataAddress(connHandler, parser.GetPasvInetAddr()) ;
                    status = SendRequest(connHandler, 
                                         GetFileOpTxFlag(connHandler)? FtpProtocol::FTP_METHOD_STOR : 
                                                                       FtpProtocol::FTP_METHOD_RETR,
                                         GetOpFileName(connHandler)) ;
                }                    
            }
            else
                MarkDataConnectionError(connHandler, true) ;
        }
        break;

    case FtpProtocol::FTP_METHOD_STOR:
    case FtpProtocol::FTP_METHOD_RETR:
        {
            if (code == FtpProtocol::FTP_STATUS_FILE_STATUS_OK_OPENING_DATA_CONN)
            {
                MarkDataConnectionRequestTime(connHandler, now) ;                    

                ChangeState(connHandler, &ClientControlConnectionHandler::DATAXFER_PENDING_CONFIRMATION) ;

                if (GetFileOpTxFlag(connHandler))
                    status = StartDataTx(connHandler) ;
                else
                    status = StartDataRx(connHandler) ;
                
                if (status)
                    StartResponseTimer(connHandler) ;
            }
            else
                MarkDataConnectionError(connHandler, true) ;
        }
        break ;
        
    default:
        return false ;
    }

    if (!status)
    {
        TransactionComplete(connHandler, false) ; // mark the failed transaction
        ChangeState(connHandler, &ClientControlConnectionHandler::EXIT_STATE) ;
        SendRequest(connHandler, FtpProtocol::FTP_METHOD_QUIT, "") ;
    }
    return true;

}

bool ClientControlConnectionHandler::PasvDataXferState::ProcessEvent(ClientControlConnectionHandler& connHandler, 
                                                             ClientControlConnectionHandler::EventTypes eventType) const
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool ClientControlConnectionHandler::PortDataXferState::ProcessInput(ClientControlConnectionHandler& connHandler, 
                                                               const ACE_Time_Value& now,
                                                               FtpProtocol::StatusCode code, 
                                                               const std::string& respLine) const
{
    bool status(false) ;
    FtpProtocol::MethodType lastCmd= GetLastCmd(connHandler) ;

    assert(GetActiveDataConnectionFlag(connHandler) == false) ;

    switch (lastCmd)
    {
    case FtpProtocol::FTP_METHOD_PORT:
        {
            if (code == FtpProtocol::FTP_STATUS_OK)
            {                    
                // since we opened a passive mode socket (i.e. before sending a port command)
                // we must internally setup Tx/Rx mode on the DataTransactionManager before
                // server sets up connection (which will happen as soon as we send the STOR/RETR
                // request
                if (GetFileOpTxFlag(connHandler))
                    status = StartDataTx(connHandler) ;
                else
                    status = StartDataRx(connHandler) ;

                if (status)
                {
                    MarkDataConnectionRequestTime(connHandler, now) ;

                    status = SendRequest(connHandler, 
                                         GetFileOpTxFlag(connHandler)? FtpProtocol::FTP_METHOD_STOR : 
                                                                       FtpProtocol::FTP_METHOD_RETR,
                                         GetOpFileName(connHandler)) ;          
                }
            }  
            else
                MarkDataConnectionError(connHandler, true) ;

        }
        break;

    case FtpProtocol::FTP_METHOD_STOR:
    case FtpProtocol::FTP_METHOD_RETR:
        {
            if (code == FtpProtocol::FTP_STATUS_FILE_STATUS_OK_OPENING_DATA_CONN)
            {
                status = true ;
                ChangeState(connHandler, &ClientControlConnectionHandler::DATAXFER_PENDING_CONFIRMATION) ;                               
                StartResponseTimer(connHandler);     
            }
            else
                MarkDataConnectionError(connHandler, true) ;
        }
        break ;
        
    default:
        return false ; 
    }

    if (!status)
    {
        TransactionComplete(connHandler, false) ; // mark the failed transaction
        ChangeState(connHandler, &ClientControlConnectionHandler::EXIT_STATE) ;
        SendRequest(connHandler, FtpProtocol::FTP_METHOD_QUIT, "") ;
    }
    return true;
}


bool ClientControlConnectionHandler::PortDataXferState::ProcessEvent(ClientControlConnectionHandler& connHandler, 
                                                             ClientControlConnectionHandler::EventTypes eventType) const
{
    // This can happen if the server opens the data connection but the status message response to STOR/RETR
    // comes too late (or is processed after the data connection is already opened.
    // In here, we simply re-start the response timer since server messages on the control connection
    // will appear in order i.e. first a status message for STPR/RETR and then a status message for the data 
    // connection.
    if (eventType == DATA_CONN_CLOSED)
        StartResponseTimer(connHandler) ; //once connection is closed, we need to wait for server to send status message
        
    return true;
}


///////////////////////////////////////////////////////////////////////////////

bool ClientControlConnectionHandler::DataXferPendingConfirmationState::ProcessInput(
    ClientControlConnectionHandler& connHandler, const ACE_Time_Value& tv, 
    FtpProtocol::StatusCode code, 
    const std::string& respLine) const 
{
    bool status(false) ;

    switch (code)
    {
    case FtpProtocol::FTP_STATUS_DATA_CONN_FAILED:
    case FtpProtocol::FTP_STATUS_CONN_CLOSED_TRANSFER_ABORTED:
        TransactionComplete(connHandler, false) ;
        status = true ; 
        break;

    case FtpProtocol::FTP_STATUS_CLOSING_DATA_CONN:
        TransactionComplete(connHandler, true) ;
        status = true ; 
        break;

    default:
        break;
    }

    if (status)
    {
         
	    /* If there is more transactions to setup, then cycle to state machine 
           back around to setup the new connection.
        */
        if (TransactionsRemaining(connHandler))
        {
            SetLastCmd(connHandler, FtpProtocol::FTP_METHOD_TYPE) ; 
            ChangeState(connHandler, &ClientControlConnectionHandler::XFERSETUP_STATE) ;
            return ClientControlConnectionHandler::XFERSETUP_STATE.ProcessInput(
		        connHandler, ACE_OS::gettimeofday(),
	            FtpProtocol::FTP_STATUS_OK, "") ;
         }
        
    }

    // Un-expected response...exit connection.
    ChangeState(connHandler, &ClientControlConnectionHandler::EXIT_STATE) ;
    SendRequest(connHandler, FtpProtocol::FTP_METHOD_QUIT, ""); 
    return true;
}

bool ClientControlConnectionHandler::DataXferPendingConfirmationState::ProcessEvent(
    ClientControlConnectionHandler& connHandler, 
    ClientControlConnectionHandler::EventTypes eventType) const 
{
    if (eventType == DATA_CONN_CLOSED)
        StartResponseTimer(connHandler) ; // wait for server to send data confirmation.

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool ClientControlConnectionHandler::ExitState::ProcessInput(ClientControlConnectionHandler& connHandler, 
                                                             const ACE_Time_Value& now,
                                                             FtpProtocol::StatusCode code, 
                                                             const std::string& respLine) const
{
    bool status(true) ;       

    // recycle the state macine to setup state...
    ChangeState(connHandler, &ClientControlConnectionHandler::LOGIN_STATE) ; // recycle back to setup state.
    return status;
}

bool ClientControlConnectionHandler::ExitState::ProcessEvent(ClientControlConnectionHandler& connHandler, 
                                                             ClientControlConnectionHandler::EventTypes eventType) const
{
    return true;
}
///////////////////////////////////////////////////////////////////////////////


