/// @file
/// @brief Ftp Server Control Connection Handler States header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _SERVER_CONTROL_CONNECTION_HANDLER_STATES_H_
#define _SERVER_CONTROL_CONNECTION_HANDLER_STATES_H_

#include "ServerControlConnectionHandler.h"
#include "FtpServerRawStats.h"

DECL_FTP_NS

///////////////////////////////////////////////////////////////////////////////

struct ServerControlConnectionHandler::State
{
    virtual ~State() { }

    /// @brief Return state name
    virtual std::string Name(void) const = 0;
    
    /// @brief Process protocol input
    virtual bool ProcessInput(ServerControlConnectionHandler& connHandler, 
                              FtpProtocol::MethodType method,
                              const std::string &request) const = 0 ;

    virtual bool ProcessEvent(ServerControlConnectionHandler& connHandler,
                              ServerControlConnectionHandler::EventTypes event) const = 0 ;

    virtual bool IsBusy() const = 0 ;
protected:
    // Central state change method
    void ChangeState(ServerControlConnectionHandler& connHandler, const ServerControlConnectionHandler::State *toState) const
    {
        connHandler.ChangeState(toState);
    }

    // Connection handler member accessors
    ACE_Time_Value GetResponseLatency(ServerControlConnectionHandler& connHandler) const 
    { 
        return connHandler.GetResponseLatency(); 
    }

    bool GetCloseFlag(const ServerControlConnectionHandler& connHandler) const 
    { 
        return connHandler.mCloseFlag; 
    }
    
    // Connection handler member mutators
    void SetCloseFlag(ServerControlConnectionHandler& connHandler, bool closeFlag) const 
    { 
        connHandler.mCloseFlag = closeFlag; 
    }

    // Response queue methods
    bool RequestQueueEmpty(const ServerControlConnectionHandler& connHandler) const
    {
        return connHandler.mReqQueue.empty();
    }
    
    // Post a response back to client
    bool PostResponse(ServerControlConnectionHandler& connHandler,  
                       FtpProtocol::StatusCode errorCode, 
                       const std::string &optionalText) const 
    {
        return connHandler.PostResponse(errorCode, optionalText);
    }
    
    // set the connection mode (active or passive)
    void SetInActiveDataConnectionMode(ServerControlConnectionHandler& connHandler, 
                                       bool active) const
    {
        connHandler.mActiveDataConnMode = active ;
    }

    // set the connection mode (active or passive)
    bool GetInActiveDataConnectionMode(const ServerControlConnectionHandler& connHandler) const
    {
        return connHandler.mActiveDataConnMode;
    }

    // update the peer address on the connection handler
    void UpdatePeerDataAddress(ServerControlConnectionHandler& connHandler, 
                           const ACE_INET_Addr& addr) const
    {
        connHandler.mPeerDataAddr = addr ;
    }
                  
    // make a data connection
    ACE_INET_Addr MakeDataConnection(ServerControlConnectionHandler& connHandler) const
    {
        return connHandler.MakeDataConnection() ;
    }

    // start tx on a data connection (when user sends RETR)
    bool StartDataTx(ServerControlConnectionHandler& connHandler) const
    {
        return connHandler.StartDataTx() ;
    }

    // start rx on a data connection (when user sends STOR)
    bool StartDataRx(ServerControlConnectionHandler& connHandler) const 
    {
        return connHandler.StartDataRx() ;
    }

    // API to force control connection to abort any active data connections
    void AbortDataConnection(ServerControlConnectionHandler& connHandler) const
    {
        connHandler.AbortDataConnection() ;
    }

    void RestartInactivityTimer(ServerControlConnectionHandler& connHandler) const
    {
        connHandler.StartInactivityTimer() ;
    }

    void CancelInactivityTimer(ServerControlConnectionHandler& connHandler) const
    {
        connHandler.CancelInactivityTimer() ;
    }

    // Common handling for the "QUIT" method
    bool HandleQuit(ServerControlConnectionHandler& connHandler) const
    {
        return connHandler.HandleQuit() ;
    }

    // Request counting methods
    void RequestCompleted(ServerControlConnectionHandler& connHandler) const
    {
        connHandler.mRequestCount++;
    }

    // get the number of remaining client requests the server should handle
    bool RequestsRemaining(const ServerControlConnectionHandler& connHandler) const
    {
        return (connHandler.mMaxRequests && connHandler.mRequestCount >= connHandler.mMaxRequests) ? false : true;
    }

    // Get server cookie port number
    uint16_t GetPort(const ServerControlConnectionHandler& connHandler) const { return connHandler.mPort ; }

    // mark state of a transaction
    void MarkAttemptedTransaction(ServerControlConnectionHandler& connHandler)  const
    {
        connHandler.MarkAttemptedTransaction() ;       
    }

    // mark state of a transaction
    void MarkTransactionComplete(ServerControlConnectionHandler& connHandler, bool success)  const
    {
        connHandler.MarkTransactionComplete(success) ;
    }
};

///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
        The state machine for the server is "loosely" implemented. It will
        work as expected only for 'well behaved' clients....
*******************************************************************************/

struct ServerControlConnectionHandler::LoginState : public ServerControlConnectionHandler::State
{
    std::string Name(void) const { return "LOGIN"; }
    bool ProcessInput(ServerControlConnectionHandler& connHandler, 
                      FtpProtocol::MethodType method,
                      const std::string &request) const;
    bool ProcessEvent(ServerControlConnectionHandler& connHandler,
                      ServerControlConnectionHandler::EventTypes event) const ;
    bool IsBusy() const ;
};

///////////////////////////////////////////////////////////////////////////////

struct ServerControlConnectionHandler::WaitingState : public ServerControlConnectionHandler::State
{
    std::string Name(void) const { return "WAITING"; }
    bool ProcessInput(ServerControlConnectionHandler& connHandler,
                      FtpProtocol::MethodType method,
                      const std::string &request) const;
    bool ProcessEvent(ServerControlConnectionHandler& connHandler,
                      ServerControlConnectionHandler::EventTypes event) const ;
    bool IsBusy() const ;
};

///////////////////////////////////////////////////////////////////////////////

struct ServerControlConnectionHandler::XferSetupState : public ServerControlConnectionHandler::State
{
    std::string Name(void) const { return "XFER_SETUP"; }
    bool ProcessInput(ServerControlConnectionHandler& connHandler, 
                      FtpProtocol::MethodType method,
                      const std::string &request) const;
    bool ProcessEvent(ServerControlConnectionHandler& connHandler,
                      ServerControlConnectionHandler::EventTypes event) const ;
    bool IsBusy() const  ;

};
///////////////////////////////////////////////////////////////////////////////

struct ServerControlConnectionHandler::DataXferState : public ServerControlConnectionHandler::State
{
    std::string Name(void) const { return "DATA"; }
    bool ProcessInput(ServerControlConnectionHandler& connHandler, 
                      FtpProtocol::MethodType method,
                      const std::string &request) const;
    bool ProcessEvent(ServerControlConnectionHandler& connHandler,
                      ServerControlConnectionHandler::EventTypes event) const ;
    bool IsBusy() const  ;

};

///////////////////////////////////////////////////////////////////////////////

END_DECL_FTP_NS

#endif
