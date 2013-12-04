/// @file
/// @brief Client Connection Handler States header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _CLIENT_CONNECTION_HANDLER_STATES_H_
#define _CLIENT_CONNECTION_HANDLER_STATES_H_

#include <utils/MessageIterator.h>
#include <utils/MessageUtils.h>

#include "ClientControlConnectionHandler.h"

DECL_FTP_NS

///////////////////////////////////////////////////////////////////////////////

struct ClientControlConnectionHandler::State
{
    virtual ~State() { }

    /// @brief Return state name
    virtual std::string Name(void) const = 0;
    
    /// @brief Process protocol input
    virtual bool ProcessInput(ClientControlConnectionHandler& connHandler, const ACE_Time_Value& tv, 
                              FtpProtocol::StatusCode code, const std::string& respLine) const = 0;

    /// @brief Process event input
    virtual bool ProcessEvent(ClientControlConnectionHandler& connHandler, ClientControlConnectionHandler::EventTypes eventType) const = 0 ;

protected:
    // Central state change method
    void ChangeState(ClientControlConnectionHandler& connHandler, const ClientControlConnectionHandler::State *toState) const
    {
        connHandler.ChangeState(toState);
    }

    // get the current data connection mode config on the connection handler (i.e. passive or active mode)
    bool GetActiveDataConnectionFlag(const ClientControlConnectionHandler& connHandler) const
    {
        return connHandler.mActiveServerConnection ;
    }

    // get the current data connection mode config on the connection handler (i.e. passive or active mode)
    bool GetFileOpTxFlag(const ClientControlConnectionHandler& connHandler) const
    {
        return connHandler.mFileOpTx  ;
    }

    // utility to make the next file-name for stor/retr
    std::string GetOpFileName(const ClientControlConnectionHandler& connHandler) const
    {
        static const std::string constName = "SpirentFile" ;
        std::ostringstream oss ;
        oss << constName << "." << connHandler.mAttemptedTransactions ;
        return oss.str() ;
    }

    // Get the last request time
    const ACE_Time_Value& GetLastRequestTime(const ClientControlConnectionHandler& connHandler) const
    {
        return connHandler.mLastReqTime;
    }

    // get the configured value of max transactions
    uint32_t GetMaxTransactions(const ClientControlConnectionHandler& connHandler) const
    {
        return connHandler.mMaxTransactions ;
    }

    // get the last received message code from server
    FtpProtocol::MethodType GetLastCmd(const ClientControlConnectionHandler& connHandler) const
    {
        return connHandler.mLastCmd;
    }

    // set (by coercion) the last cmd sent. Normally, the lastCmd parameter is auto-updated.
    void SetLastCmd(ClientControlConnectionHandler& connHandler, FtpProtocol::MethodType latest) const
    {
        connHandler.mLastCmd = latest;
    }

    // Update the remote data address beign used by the connection handler
    void UpateRemoteDataAddress(ClientControlConnectionHandler& connHandler, const ACE_INET_Addr& updated) const
    {
        connHandler.mPeerDataAddr = updated ;
    }

    // Method to record the time when a new data transfer is initiated
    void MarkDataConnectionRequestTime(ClientControlConnectionHandler& connHandler, const ACE_Time_Value &now) const
    {
        connHandler.mDataConnReqTime = now ;
    }

    // Mark the connection handler with given data connection error flag
    void MarkDataConnectionError(ClientControlConnectionHandler& connHandler, bool err) const
    {
        connHandler.mDataConnErrorFlag = err;
    }

    // make a data connection
    ACE_INET_Addr MakeDataConnection(ClientControlConnectionHandler& connHandler) const
    {
        return connHandler.MakeDataConnection() ;
    }

    // start tx on a data connection (when user sends RETR)
    bool StartDataTx(ClientControlConnectionHandler& connHandler) const
    {
        return connHandler.StartDataTx() ;
    }

    // start rx on a data connection (when user sends STOR)
    bool StartDataRx(ClientControlConnectionHandler& connHandler) const 
    {
        return connHandler.StartDataRx() ;
    }

    // send a request to server
    bool SendRequest(ClientControlConnectionHandler& connHandler,
                     FtpProtocol::MethodType method, 
                     const std::string &reqToken) const 
    {
        return connHandler.SendRequest(method, reqToken) ;
    }

    // method to check if more transactions need to be performed
    bool TransactionsRemaining(const ClientControlConnectionHandler& connHandler) const
    {
        return ( connHandler.mMaxTransactions && connHandler.mAttemptedTransactions >= connHandler.mMaxTransactions) ? false : true ;
    }

    // increment count of attempted transactions
    void TransactionInitated(ClientControlConnectionHandler& connHandler) const
    {
        connHandler.TransactionInitated() ;
    }

    // increment number of successful transactions
    void TransactionComplete(ClientControlConnectionHandler& connHandler, bool successful) const
    {
        connHandler.TransactionComplete(successful) ;
    }

    // start the response timeout
    void StartResponseTimer(ClientControlConnectionHandler& connHandler) const
    {
        connHandler.StartResponseTimer() ;
    }

    // cancel response timeout
    void CancelResponseTimer(ClientControlConnectionHandler& connHandler) const
    {
        connHandler.CancelResponseTimer() ;
    }

    // Shared method to start another transaction or quit server control connection
    void DoAnotherTransactionOrExit(ClientControlConnectionHandler& connHandler) const
    {
    }
};

///////////////////////////////////////////////////////////////////////////////

struct ClientControlConnectionHandler::LoginState : public ClientControlConnectionHandler::State
{
    std::string Name(void) const { return "LOGIN"; }
    bool ProcessInput(ClientControlConnectionHandler& connHandler, const ACE_Time_Value& tv, 
                      FtpProtocol::StatusCode code, const std::string& respLine) const ;
    bool ProcessEvent(ClientControlConnectionHandler& connHandler, ClientControlConnectionHandler::EventTypes eventType) const ;
};

///////////////////////////////////////////////////////////////////////////////

struct ClientControlConnectionHandler::XferSetupState : public ClientControlConnectionHandler::State
{
    std::string Name(void) const { return "XFERSETUP"; }
    bool ProcessInput(ClientControlConnectionHandler& connHandler, const ACE_Time_Value& tv, 
                      FtpProtocol::StatusCode code, const std::string& respLine) const ;
    bool ProcessEvent(ClientControlConnectionHandler& connHandler, ClientControlConnectionHandler::EventTypes eventType) const ;
};

///////////////////////////////////////////////////////////////////////////////

struct ClientControlConnectionHandler::PasvDataXferState : public ClientControlConnectionHandler::State
{
    std::string Name(void) const { return "PASV_DATAXFER"; }
    bool ProcessInput(ClientControlConnectionHandler& connHandler, const ACE_Time_Value& tv, 
                      FtpProtocol::StatusCode code, const std::string& respLine) const ;
    bool ProcessEvent(ClientControlConnectionHandler& connHandler, ClientControlConnectionHandler::EventTypes eventType) const ;
};

///////////////////////////////////////////////////////////////////////////////

struct ClientControlConnectionHandler::PortDataXferState : public ClientControlConnectionHandler::State
{
    std::string Name(void) const { return "PORT_DATAXFER"; }
    bool ProcessInput(ClientControlConnectionHandler& connHandler, const ACE_Time_Value& tv, 
                      FtpProtocol::StatusCode code, const std::string& respLine) const ;
    bool ProcessEvent(ClientControlConnectionHandler& connHandler, ClientControlConnectionHandler::EventTypes eventType) const ;
};

///////////////////////////////////////////////////////////////////////////////

struct ClientControlConnectionHandler::DataXferPendingConfirmationState : public ClientControlConnectionHandler::State
{
    std::string Name(void) const { return "DATAXFER_CONFIRMATION"; }
    bool ProcessInput(ClientControlConnectionHandler& connHandler, const ACE_Time_Value& tv, 
                      FtpProtocol::StatusCode code, const std::string& respLine) const ;
    bool ProcessEvent(ClientControlConnectionHandler& connHandler, ClientControlConnectionHandler::EventTypes eventType) const ;
};

///////////////////////////////////////////////////////////////////////////////

struct ClientControlConnectionHandler::ExitState : public ClientControlConnectionHandler::State
{
    std::string Name(void) const { return "EXIT"; }
    bool ProcessInput(ClientControlConnectionHandler& connHandler, const ACE_Time_Value& tv, 
                      FtpProtocol::StatusCode code, const std::string& respLine) const ;
    bool ProcessEvent(ClientControlConnectionHandler& connHandler, ClientControlConnectionHandler::EventTypes eventType) const ;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_FTP_NS

#endif
