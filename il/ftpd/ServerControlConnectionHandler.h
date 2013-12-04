/// @file
/// @brief Server Connection Handler header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _SERVER_CONTROL_CONNECTION_HANDLER_H_
#define _SERVER_CONTROL_CONNECTION_HANDLER_H_

#include <queue>

#include <ace/Time_Value.h>
#include <app/StreamSocketHandler.h>
#include <base/BaseCommon.h>
#include <boost/random/linear_congruential.hpp>
#include <Tr1Adapter.h>
#include <utils/MessageIterator.h>
#include <utils/ResponseLatency.h>
#include <ildaemon/TimerQueue.h>

#include "FtpCommon.h"
#include "FtpProtocol.h"
#include "FtpServer.h"
#include "FtpControlConnectionHandlerInterface.h"
#include "DataTransactionManager.h"

#ifndef IPV6_TCLASS
    #define IPV6_TCLASS 67         // ref include/linux/in6.h in kernel 2.6 sources
#endif

// Forward declarations (global)
class ACE_Message_Block;

namespace FTP_NS
{
    class FtpServerRawStats;

}

DECL_FTP_NS

///////////////////////////////////////////////////////////////////////////////

class ServerControlConnectionHandler : public L4L7_APP_NS::StreamSocketHandler, FtpControlConnectionHandlerInterface
{
public:
    typedef Ftp_1_port_server::FtpServerResponseConfig_t serverResponseConfig_t ;
    typedef Ftp_1_port_server::L4L7Profile_t             l4l7Profile_t ;
    typedef fastdelegate::FastDelegate0 < uint32_t >     responseLatencyGetter_t ;
    typedef FtpServerRawStats                            stats_t ;
    typedef DataTransactionManager::dataCloseHandler_t   dataCloseNtfyCb_t; 

    explicit ServerControlConnectionHandler(uint32_t serial = 0);
    ~ServerControlConnectionHandler();

    ///////  APIs from FtpControlConnectionHandlerInterface ////////////////////////
    const ACE_INET_Addr& GetLocalDataInetAddr() const { return mServerDataAddr ; }

    const ACE_INET_Addr& GetRemoteDataInetAddr() const { return mPeerDataAddr ; }

    const std::string& GetLocalIfName() const  { return mServerIfNameGetter() ; }

    void GetTxBodyConfig(uint8_t &szType, 
                         uint32_t &fixedSz, 
                         uint32_t &randomSzMean, 
                         uint32_t &randomStdDev, 
                         uint8_t &contentType) const ; 

    /// @brief Notification that Data was established
    void NotifyDataConnectionOpen(const ACE_Time_Value& connectTime) ;

    /// @brief Notification that Data connection traffic was sent
    void NotifyIncrementalTxBytes(uint32_t bytes)  ;

    /// @brief Notification that Data connection traffic was received
    void NotifyIncrementalRxBytes(uint32_t bytes) ;

    /// @brief Notification that Data was closed
    void NotifyDataConnectionClose(const ACE_Time_Value& closeTime) {}

    /// @brief Get the IPv4 TOS socket option
    uint8_t GetIpv4Tos() const { return (mL4L7Profile ? mL4L7Profile->Ipv4Tos : 0); }

    /// @brief Get the IPv6 traffic class socket option
    uint8_t GetIpv6TrafficClass() const { return (mL4L7Profile ? mL4L7Profile->Ipv6TrafficClass : 0); }

    /// @brief Get the TCP window size limit socket option
    uint32_t GetTcpWindowSizeLimit() const { return (mL4L7Profile ? mL4L7Profile->RxWindowSizeLimit : 0); }

    /// @brief Get the TCP delayed ack socket option
    bool GetTcpDelayedAck() const { return (mL4L7Profile ? mL4L7Profile->EnableDelayedAck : 0); }

    ////////////////////////////////////////////////////////////////////////////////

    /// @brief API to retrive server's name
    const std::string&  GetServerName() const { return mServerName ; }
    
    /// @brief Get Address Type
    int GetAddrType(void)
    {
         ACE_INET_Addr addr;
         if(this->peer().get_local_addr(addr) == -1)
         {
              return -1;         
         }
         return addr.get_type();
    }

    /// @brief Set the IPv4 TOS socket option
    int SetIpv4Tos(int tos)
    {
        const int thetos = tos;
        return this->peer().set_option(IPPROTO_IP, IP_TOS, (char *)&thetos, sizeof(tos));
    }
 
    /// @brief Set the IPv6 traffic class socket option
    int SetIpv6TrafficClass(int trafficClass)
    {
        const int tclass = trafficClass;
        return this->peer().set_option(IPPROTO_IPV6, IPV6_TCLASS, (char *)&tclass, sizeof(tclass));
    }

protected:
    // local convinience typedefs.
    typedef Ftp_1_port_server::BodyContentOptions bodyType_t;
    typedef fastdelegate::FastDelegate0 < const std::string &>                     serverIfNameGetter_t ;

    /// Private request record class
    struct RequestRecord ;

    /// Socket open handler method (from StreamSocketHandler interface)
    int HandleOpenHook(void);

    /// @brief Socket input handler method (from StreamSocketHandler interface)
    int HandleInputHook(void);

    /// @brief Called by FTP server to force an abort on the session
    void Abort(void) ;

    /// @brief Called by FTP server to give control connection chance to "Finalize" itself before closure.
    void Finalize(void) ;

    /// @brief Callback from FTP server informing closure of data connection owned by this Control Connection
    void DataCloseCb(void *arg) ;

    /// @brief Set delegate to get server Address
    void SetServerIfNameGetter(const serverIfNameGetter_t& nameGettter) { mServerIfNameGetter = nameGettter ; }

    /// @brief Set the server's test port number
    void SetServerPort(uint16_t port) { mPort = port ; }

    /// @brief Set the server's name (used for logging)
    void SetServerName(const std::string &serverName) { mServerName = serverName ; }

    /// @brief Set maximum number of requests
    void SetMaxRequests(uint32_t maxRequests) { mMaxRequests = maxRequests; }

    /// @brief API to setup the data response config for FTP
    void SetResponseConfig(const serverResponseConfig_t &responseConfig) { mRespConfig = responseConfig ; UpdateResponseTimings() ; }

    /// @brief API to setup stats instance of parent Server (block)
    void SetStatsInstance(stats_t &stats);

    /// @brief Update Goodput Tx
    void UpdateGoodputTxSent(uint64_t sent);

    /// @brief API to setup pointer to L4/L7 profile configuration of the server
    void SetL4L7Profile(const l4l7Profile_t &l4l7Profile) { mL4L7Profile = &l4l7Profile ; }

    /// @brief API to setup a data close notification callback
    void SetDataCloseCb( dataCloseNtfyCb_t cb) { mDataCloseCb = cb ; }

    friend class FtpServer ;

private:
    // convience typedefs
    typedef DataTransactionManager *                         dataMgrRawPtr_t ;
    typedef boost::shared_ptr<DataTransactionManager>        dataMgrPtr_t ;
    typedef std::map<dataMgrRawPtr_t , dataMgrPtr_t>         dataMgrMap_t ;

    /// Private timer types
    enum
    {
        INACTIVITY_TIMER,
        REQ_QUEUE_TIMER
    };
    
    // Private event types
    enum EventTypes
    {
        DATA_CONN_CLOSED
    } ;
    /// Private state class forward declarations
    struct State;
    struct LoginState;
    struct WaitingState ;
    struct XferSetupState ;
    struct DataXferState ;

     /// @brief Method to create a data connection
    ACE_INET_Addr MakeDataConnection(void)  ;

    /// @brief Method to close specified data connection. Close all if input = 0.
    void CloseDataConnection(dataMgrRawPtr_t dataMgr = 0) ;

    /// @brief Method to send abort code and immediately abandon specified data connection. Abort all if input=0.
    void AbortDataConnection(void) ;

    /// @brief Method to handle QUIT from client
    bool HandleQuit(void) ;

    /// @brief Method to handle transaction attempted notification from state machine
    void MarkAttemptedTransaction(void) ;

    /// @brief Method to handle transaction completion notification from state machine
    void MarkTransactionComplete(bool successfulTransaction) ;
    
    /// @brief Method to start Data Tx to client (i.e. RETR handling)
    bool StartDataTx(void) ;

    /// @brief Method to handle Data Rx from client (i.e. STOR handling)
    bool StartDataRx(void) ;

    /// @brief Start the inactivity timer
    void StartInactivityTimer(void) ;

    /// @brief Start the inactivity timer
    void CancelInactivityTimer(void) ;

    /// Timeout handler methods
    /// @brief Method to handle Request timeout
    int HandleRequestQueueTimeout(const ACE_Time_Value& tv, const void *act);

    /// @brief Method to handle inactivity timeout
    int HandleInactivityTimeout(const ACE_Time_Value& tv, const void *act);
        
    /// @brief Response queue methods
    void QueueRequest(const ACE_Time_Value& absTime, const std::string &request) ;
    
    /// @brief service the request queue
    ssize_t ServiceRequestQueue(void) ;

    /// @brief API to post a response
    bool PostResponse(FtpProtocol::StatusCode theCode, const std::string &optionalText) ;

    /// @brief Central state change method
    void ChangeState(const State *toState) { mState = toState; }

    /// @brief Method to post active data manager for deletion and reset current ptr
    void ReleaseActiveDataMgr(void) ;

    /// @brief method automatically called when config is setup on the control connection handler (by parent)
    void UpdateResponseTimings(void) ;

    /// @brief Method to retrive latency for processing request from client..
    ACE_Time_Value GetResponseLatency(void) { return mRespLatency.Get(); }

    ////////////////////////////////////////////////////////////////////////////////////

    /// Class data
    static const LoginState     LOGIN_STATE;
    static const WaitingState   WAIT_STATE ;
    static const XferSetupState XFER_SETUP_STATE ;
    static const DataXferState  DATA_STATE;


    // static const containing for in-activity timeout
    static const ACE_Time_Value INACTIVITY_TIMEOUT;

    /// Convenience typedefs
    typedef boost::minstd_rand randomEngine_t;

    // fixed and random response latency members
    randomEngine_t mRandomEngine;                       ///< randomness engine
    L4L7_UTILS_NS::ResponseLatency mRespLatency;        ///< response latency

    // stats 
    stats_t *mStats ;                                  ///< pointer to parent server's stats structure.

    // server config members
    uint32_t mMaxRequests;                              ///< max number of requests to allow
    uint32_t mRequestCount ;                            ///< number of data transfer requests on current connection
    uint32_t mTransactionsActive ;                      ///< Count of currently active transactions
    serverResponseConfig_t mRespConfig ;                ///< Configuration of data response handling
    const l4l7Profile_t *mL4L7Profile ;                 ///< pointer to L4/L7 profile
    dataCloseNtfyCb_t    mDataCloseCb ;                 ///< Delegate to register a data close CB in a "safer" thread context.

    // input processing related members
    std::tr1::shared_ptr<ACE_Message_Block> mInBuffer;  ///< input buffer
    const State *mState;                                ///< current state

    // server flags
    bool mCloseFlag;                                    ///< close connection after this request?
    bool mActiveDataConnMode ;                          ///< tracks if data connection is active of passive

    // request queue
    std::queue<RequestRecord>  mReqQueue ;              ///< queue of pending requests

    // the control connection's collection of DataTransactionManagers, including active one.
    dataMgrMap_t    mDataMgrs ;
    dataMgrRawPtr_t mActiveDataMgr ;

    // private delegates for handling timers dipatched by TimerQueue
    IL_DAEMON_LIB_NS::TimerQueue::delegate_t mReqQueueTimeoutDelegate;
    IL_DAEMON_LIB_NS::TimerQueue::delegate_t mInactivityTimeoutDelegate;

    // Delegates to interact with the server
    serverIfNameGetter_t      mServerIfNameGetter;

    // local test port number, server's address and server name
    uint16_t mPort ;
    ACE_INET_Addr mServerAddr ;
    ACE_INET_Addr mServerDataAddr ;

    std::string mServerName ;

    // remote (client) address
    ACE_INET_Addr mPeerAddr ;
    ACE_INET_Addr mPeerDataAddr ;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_FTP_NS

#endif
