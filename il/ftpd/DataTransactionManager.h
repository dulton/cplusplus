/// @file
/// @brief Data Transaction Manager
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _DATA_TRANSACTION_MANAGER_H
#define _DATA_TRANSACTION_MANAGER_H

#include <ace/Thread_Mutex.h>

#include <app/StreamSocketHandler.h>
#include <base/BaseCommon.h>
#include <app/StreamSocketConnector.tcc>
#include <app/StreamSocketOneshotAcceptor.tcc>

#include <boost/shared_ptr.hpp>

#include "FtpCommon.h"
#include "FtpProtocol.h"
#include "FtpControlConnectionHandlerInterface.h"

// Forward declarations (global)
class ACE_Message_Block;

namespace FTP_NS
{
    class ServerControlConnectionHandler;
    class DataConnectionHandler ;
    class FtpServer ;
}

DECL_FTP_NS

///////////////////////////////////////////////////////////////////////////////

class DataTransactionManager
{
public:    
    typedef Ftp_1_port_server::FtpServerResponseConfig_t            serverResponseConfig_t ;
    typedef fastdelegate::FastDelegate2 < FtpControlConnectionHandlerInterface *, 
                                         DataTransactionManager * > dataCloseHandler_t; 

    // Dynamic Load Delegates
    typedef boost::function1<size_t, bool> getDynamicLoadBytesDelegate_t;
    typedef boost::function2<size_t, bool, const ACE_Time_Value&> produceDynamicLoadBytesDelegate_t;
    typedef boost::function2<size_t, bool, size_t> consumeDynamicLoadBytesDelegate_t;

    DataTransactionManager(uint16_t                        portNumber,
                           FtpControlConnectionHandlerInterface &parent,
                           ACE_Reactor                    *useReactor,
                           bool                            initiateConnection,
                           bool                            isServer,
                           const dataCloseHandler_t&       connCloseHdlr,
                           bool                            useRxInactivityTimeout = true,
                           bool                            enableBwCtrl = false);
                                          
    ~DataTransactionManager() ;

    int Shutdown(void) ;
    
    bool StartDataTx(void) ;

    bool StartDataRx(void) ;

    ACE_INET_Addr GetDataSourceAddr(void) const { return mLocalAddr ; }

    void ClearCloseNotificationDelegate(void) { mConnCloseHdlr.clear(); }

    bool IsConnectionActive(void) ;
  
    void SetDynamicLoadDelegates(getDynamicLoadBytesDelegate_t getDelegate,
                                 produceDynamicLoadBytesDelegate_t produceDelegate,
                                 consumeDynamicLoadBytesDelegate_t consumeDelegate) {
        getDlbDelegate = getDelegate;
        produceDlbDelegate = produceDelegate;
        consumeDlbDelegate = consumeDelegate;
    }
  
private:
    typedef L4L7_APP_NS::StreamSocketConnector<DataConnectionHandler>       connector_t;
    typedef L4L7_APP_NS::StreamSocketOneshotAcceptor<DataConnectionHandler> acceptor_t;
    typedef std::tr1::shared_ptr<connector_t>                               connectorPtr_t ;
    typedef std::tr1::shared_ptr<acceptor_t>                                acceptorPtr_t ;
    typedef std::tr1::shared_ptr<DataConnectionHandler>                     dataHdlrPtr_t ;
    typedef ACE_Recursive_Thread_Mutex                                      lock_t;
    
    /// Connection event handlers
    // Method to handle connect/accept completion events from connector/acceptor
    bool HandleConnectionNotification(DataConnectionHandler& connHandler);

    // Method to handle close notification from a data connection
    void HandleCloseNotification(L4L7_APP_NS::StreamSocketHandler& socketHandler)  ;

    // Method to open acceptor for data connection
    bool OpenAcceptor(void) ;

    // Method to startup the data connection
    bool OpenConnector(void) ;

    // Method to close data connections
    bool CloseConnectorOrAcceptor(void) ;

    // method that receives Tx stats update from Data connection handler
    void BytesTx(uint32_t bytes) ;  
    
    // method that receives Rx stats update from Data connection handler
    void BytesRx(uint32_t bytes)  ; 

    void RegisterDynamicLoadDelegates(DataConnectionHandler *dch);
    void UnregisterDynamicLoadDelegates(DataConnectionHandler *dch);

    ///////////////////////////////////////////////////////////////////////////
    uint16_t                                mPort ;
    FtpControlConnectionHandlerInterface&   mParent ;
    ACE_Reactor *const                      mReactor ;
    bool                                    mConnectionMode; 
    bool                                    mConnOpened ;
    bool                                    mIsShutdown ;
    dataCloseHandler_t                      mConnCloseHdlr ;
    bool                                    mRxInactivityTimeout ;
    bool                                    mInTxMode ;
    ACE_INET_Addr                           mLocalAddr ;
    connectorPtr_t                          mConnector ;
    acceptorPtr_t                           mAcceptor;
    bool                                    mEnableBwCtrl;

    // Dynamic Load Delegates
    getDynamicLoadBytesDelegate_t       getDlbDelegate;
    produceDynamicLoadBytesDelegate_t   produceDlbDelegate;
    consumeDynamicLoadBytesDelegate_t   consumeDlbDelegate;

    // pointer to the actual data connection handler
    dataHdlrPtr_t mHdlr ;
    // local locl object
    lock_t mLock ;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_FTP_NS

#endif //_SERVER_DATA_TRANSACTION_MANAGER_H


