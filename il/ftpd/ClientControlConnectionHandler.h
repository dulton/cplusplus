/// @file
/// @brief Client Connection Handler header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _CLIENT_CONTROL_CONNECTION_HANDLER_H_
#define _CLIENT_CONTROL_CONNECTION_HANDLER_H_

#include <string>
#include <utility>

#include <app/StreamSocketHandler.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <ildaemon/TimerQueue.h>

#include "FtpCommon.h"
#include "FtpProtocol.h"
#include "FtpControlConnectionHandlerInterface.h"
#include "DataTransactionManager.h"

namespace FTP_NS
{
    class FtpClientBlock ;
}

DECL_FTP_NS

// Forward declarations
class FtpClientRawStats;

///////////////////////////////////////////////////////////////////////////////

class ClientControlConnectionHandler : public L4L7_APP_NS::StreamSocketHandler, FtpControlConnectionHandlerInterface
{
  public:
    typedef FtpClientRawStats                stats_t;
    typedef Ftp_1_port_server::L4L7Profile_t l4l7Profile_t ;
    typedef DataTransactionManager::dataCloseHandler_t  dataCloseNtfyCb_t; 

    explicit ClientControlConnectionHandler(uint32_t serial = 0);
    ~ClientControlConnectionHandler();

    ///////  APIs from FtpControlConnectionHandlerInterface ////////////////////////
    const ACE_INET_Addr& GetLocalDataInetAddr() const { return mLocalDataAddr ; }

    const ACE_INET_Addr& GetRemoteDataInetAddr() const { return mPeerDataAddr ; }

    const std::string& GetLocalIfName() const  { return mLocalIfName ; }

    // this method is essentiall a no-op on the client since it only retrieves data for now.
    void GetTxBodyConfig(uint8_t &szType, 
                         uint32_t &fixedSz, 
                         uint32_t &randomSzMean, 
                         uint32_t &randomStdDev, 
                         uint8_t &contentType) const 
    { 
        szType = mSzType ;
        fixedSz = mFixedSz ;
        randomSzMean = mRandomSzMean ;
        randomStdDev = mRandomStdDev ;
        contentType = mContentType ; 
    }

    /// @brief Notification that Data was established
    void NotifyDataConnectionOpen(const ACE_Time_Value& connectTime) ;

    /// @brief Notification that Data connection traffic was sent
    void NotifyIncrementalTxBytes(uint32_t bytes) ;

    /// @brief Notification that Data connection traffic was received
    void NotifyIncrementalRxBytes(uint32_t bytes) ;

    /// @brief Notification that Data was closed
    void NotifyDataConnectionClose(const ACE_Time_Value& connectTime) ;

    /// @brief Get the IPv4 TOS socket option
    uint8_t GetIpv4Tos() const { return (mL4L7Profile ? mL4L7Profile->Ipv4Tos : 0); }

    /// @brief Get the IPv6 traffic class socket option
    uint8_t GetIpv6TrafficClass() const { return (mL4L7Profile ? mL4L7Profile->Ipv6TrafficClass : 0); }

    /// @brief Get the TCP window size socket option
    uint32_t GetTcpWindowSizeLimit() const { return (mL4L7Profile ? mL4L7Profile->RxWindowSizeLimit : 0); }

    /// @brief Get the TCP delayed ack socket option
    bool GetTcpDelayedAck() const { return (mL4L7Profile ? mL4L7Profile->EnableDelayedAck : 0); }

    ////////////////////////////////////////////////////////////////////////////////


  protected:
    /// @brief Set maximum number of transations
    void SetMaxTransactions(uint32_t maxTransactions) { mMaxTransactions = std::max(maxTransactions, 1U); }

    /// @brief API to set the port number
    void SetPortNumber(uint16_t port)  { mPort = port ; }

    /// @brief API to tell the handler its source interface name...(needed for the data connection)
    void SetSourceIfName(const std::string& srcIfName) { mLocalIfName = srcIfName ; }

    /// @brief API informing client if it should use PASV mode for data transfer.
    void SetPassiveDataXferEnabled(bool enablePassiveMode) { mActiveServerConnection = enablePassiveMode ; }

    /// @brief File operation to perform - TX or RX. Default is Rx (i.e. false)
    void SetFileOpAsTx(bool tx)               { mFileOpTx = tx ; }

    /// @brief In case of Tx, setup Tx body configuration
    void SetTxBodyConfig(uint8_t  szType, 
                         uint32_t fixedSz, 
                         uint32_t randomSzMean, 
                         uint32_t randomStdDev, 
                         uint8_t  contentType)
    { 
        mSzType = szType ;
        mFixedSz = fixedSz ;
        mRandomSzMean = randomSzMean ;
        mRandomStdDev = randomStdDev ;
        mContentType = contentType ; 
    }
	/// @brief Called by FtpClientBlock to Close the control (and data, if applicable) connection.
	void Close(void) ;

    /// @brief Called by FtpClientBlock when Control Connection is closing to clean-up
    void Finalize(void) ;

    /// @brief Callback from FTP Client Block informing closure of data connection owned by this Control Connection
    void DataCloseCb(void *dataMgr) ;

    /// @brief Set stats instance
    void SetStatsInstance(stats_t& stats);

    /// @brief Update Goodput Tx
    void UpdateGoodputTxSent(uint64_t sent);

    /// @brief Set a pointer to the L4/L7 profile configuration for the client
    void SetL4L7Profile(const l4l7Profile_t &l4l7Profile) { mL4L7Profile = &l4l7Profile ; }
   
    /// @brief API to setup a data close notification callback
    void SetDataCloseCb( dataCloseNtfyCb_t cb) { mDataCloseCb = cb ; }

    /// @brief Called by FtpClientBlock to Set/Get completion (connection status logged) flag
    void SetComplete(bool complete) { mComplete = complete; } 
    bool IsComplete() { return mComplete; }

    friend class FtpClientBlock ;
    
  private:
    // convience typedefs
    typedef DataTransactionManager *                         dataMgrRawPtr_t ;
    typedef boost::shared_ptr<DataTransactionManager>        dataMgrPtr_t ;
    typedef std::map<dataMgrRawPtr_t, dataMgrPtr_t>          dataMgrMap_t ;

    enum TimerTypes
    {
        RESPONSE_TIMEOUT
    } ;
	  // Private event types
    enum EventTypes
    {
        DATA_CONN_CLOSED
	} ;

    /// Private state class forward declarations
    struct State;
    struct LoginState;
    struct XferSetupState ;
    struct PasvDataXferState;
    struct PortDataXferState;
    struct DataXferPendingConfirmationState ;
    struct ExitState;

    // static const containing for in-activity timeout
    static const ACE_Time_Value SERVER_RESPONSE_TIMEOUT;

    /// @brief Socket open handler method (from StreamSocketHandler interface)
    int HandleOpenHook(void);

    /// @brief Socket input handler method (from StreamSocketHandler interface)
    int HandleInputHook(void);

    /// @brief ACE timer hook to handle server response timeouts
    int HandleResponseTimeout(const ACE_Time_Value& tv, const void *act) ;

    /// @brief Methods to start a response timeout
    void StartResponseTimer() ;

    /// @brief Method to cancel a response timeout
    void CancelResponseTimer() ;

    /// @brief Central state change method
    void ChangeState(const State *toState) { mState = toState; }

	/// Methods to manage the data connection
    /// @brief Method to create a data connection
    ACE_INET_Addr MakeDataConnection(void) ;

    /// @brief Method to close specified data connection. Close all if input = 0
    void CloseDataConnection(dataMgrRawPtr_t dataMgr = 0) ;

    /// @brief Method to Start Transmit on active data connection. Will create one if none exists
    bool StartDataTx(void) ;

    /// @brief Method to enable Receive on active data connection. Will create one if none exists.
    bool StartDataRx(void) ;

   /// @brief API to indicate a data connection error (based on message from the server)
    void SetDataConnErrorFlag(bool err)       { mDataConnErrorFlag = err ; }

    /// @brief Method to send request to server
    bool SendRequest(FtpProtocol::MethodType method, const std::string &reqToken) ;

    /// @brief Method to update stats indication a new transaction has started
    void TransactionInitated() ;

    /// @brief Method to update stats indicating that the current transaction is complete.
    void TransactionComplete(bool successful) ;    

    /// Class data
    static const LoginState                         LOGIN_STATE;
    static const XferSetupState                     XFERSETUP_STATE ;
    static const PasvDataXferState                  PASV_DATAXFER_STATE;
    static const PortDataXferState                  PORT_DATAXFER_STATE;
    static const DataXferPendingConfirmationState   DATAXFER_PENDING_CONFIRMATION; 
    static const ExitState                          EXIT_STATE;

    uint32_t mMaxTransactions;                          ///< max number of transactions
    uint32_t mAttemptedTransactions ;                   ///< number of attempted transactions
    bool     mTransactionActive ;                       ///< Flag tracking if the client has initiated a transaction

    stats_t *mStats;                                    ///< client stats
    const l4l7Profile_t *mL4L7Profile ;                 ///< pointer to L4/L7 profile
    dataCloseNtfyCb_t    mDataCloseCb ;                 ///< Delegate to register a data close CB in a "safer" thread context.

    std::tr1::shared_ptr<ACE_Message_Block> mInBuffer;  ///< input buffer
    const State *mState;                                ///< current state
    bool mActiveServerConnection,                       ///< Should client initiate or passively handle data connections
         mFileOpTx,                                     ///< file op is STOR or RETR (true for STOR, false for RETR)
         mDataConnErrorFlag;                            ///< Was there an error reported by server when opening the data connection

    // Tx Body Config parameters. Setup by parent ClientBlock.
    uint8_t    mSzType, 
               mContentType ;
    uint32_t   mFixedSz, 
               mRandomSzMean, 
               mRandomStdDev ;

    // the control connection's collection of DataTransactionManagers, including active one.
    dataMgrMap_t    mDataMgrs ;
    dataMgrRawPtr_t mActiveDataMgr ;

    // cookie port number
    uint16_t mPort;   

    // complete flag
    bool mComplete;

    // local Address and interface name
    ACE_INET_Addr mLocalAddr, mLocalDataAddr ;
    std::string   mLocalIfName ;

    // remote control address and data address
    ACE_INET_Addr mPeerAddr, mPeerDataAddr ;

    // last command sent to server
    FtpProtocol::MethodType mLastCmd ;

    // connection private stat of number of data bytes transferred over data connection
    uint32_t mDataConnXferBytes ;

    // Last request Tx timestamp
    ACE_Time_Value mLastReqTime,
                   mDataConnReqTime,
                   mDataConnOpenTime,
                   mDataConnCloseTime;

    // delegate used to handle server response timeouts
    IL_DAEMON_LIB_NS::TimerQueue::delegate_t mResponseTimeoutDelegate;

};

///////////////////////////////////////////////////////////////////////////////

END_DECL_FTP_NS

#endif
