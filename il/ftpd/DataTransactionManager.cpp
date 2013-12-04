// @file
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

#include <ace/Message_Block.h>
#include <boost/bind.hpp>
#include "FtpdLog.h"

#include "DataTransactionManager.h"
#include "ServerControlConnectionHandler.h"
#include "DataConnectionHandler.h"

#define STD_FTP_CONTROL_PORT 21
#define STD_FTP_DATA_PORT    20

USING_FTP_NS;

///////////////////////////////////////////////////////////////////////////////

DataTransactionManager::DataTransactionManager(
    uint16_t                             portNumber,
    FtpControlConnectionHandlerInterface &parent,
    ACE_Reactor                         *useReactor,
    bool                                 initiateConnection,
    bool                                 isServer,
    const dataCloseHandler_t&            connCloseHdlr,
    bool                                 useRxInactivityTimeout,
    bool                                 enableBwCtrl) :
mPort(portNumber),
mParent(parent),
mReactor(useReactor),
mConnectionMode(initiateConnection),
mConnOpened(false),
mIsShutdown(false),
mConnCloseHdlr(connCloseHdlr),
mRxInactivityTimeout(useRxInactivityTimeout),
mInTxMode(true),
mLocalAddr(parent.GetLocalDataInetAddr()),
mEnableBwCtrl(enableBwCtrl)
{
    if (isServer && initiateConnection)
    {
        // active servers always connect from 20
        mLocalAddr.set_port_number(STD_FTP_DATA_PORT) ; 
#ifdef UNIT_TEST
        // using port 20 requires admin
        mLocalAddr.set_port_number(0) ; 
#endif
    }
    else
    {
        // ensure local port is set to 0 so that OS will allocate
        mLocalAddr.set_port_number(0) ; 
    }


    if (!mConnectionMode)
    {
        mAcceptor = acceptorPtr_t(new acceptor_t(), boost::bind(&acceptor_t::remove_reference, _1)) ;
        mAcceptor->reference_counting_policy().value(acceptor_t::Reference_Counting_Policy::ENABLED) ;
        mAcceptor->SetAcceptNotificationDelegate(
                    fastdelegate::MakeDelegate(this, 
                    &DataTransactionManager::HandleConnectionNotification));
        mAcceptor->SetIpv4Tos(mParent.GetIpv4Tos());
        mAcceptor->SetIpv6TrafficClass(mParent.GetIpv6TrafficClass());
        mAcceptor->SetTcpWindowSizeLimit(mParent.GetTcpWindowSizeLimit());
        mAcceptor->SetTcpDelayedAck(mParent.GetTcpDelayedAck());

        // Open the acceptor right away
        mConnOpened = OpenAcceptor() ;
    }
    else  
    {
        mConnector = connectorPtr_t(new connector_t(mReactor), boost::bind(&connector_t::remove_reference, _1)) ;
        mConnector->reference_counting_policy().value(connector_t::Reference_Counting_Policy::ENABLED) ;
        mConnector->SetIpv4Tos(mParent.GetIpv4Tos());
        mConnector->SetIpv6TrafficClass(mParent.GetIpv6TrafficClass());
        mConnector->SetTcpWindowSizeLimit(mParent.GetTcpWindowSizeLimit());
        mConnector->SetTcpDelayedAck(mParent.GetTcpDelayedAck());
        mConnector->SetConnectNotificationDelegate(
                        fastdelegate::MakeDelegate(this, 
                        &DataTransactionManager::HandleConnectionNotification));
        mConnector->SetCloseNotificationDelegate(
                        fastdelegate::MakeDelegate(this, 
                        &DataTransactionManager::HandleCloseNotification)) ;
    }
}

///////////////////////////////////////////////////////////////////////////////                                      
DataTransactionManager::~DataTransactionManager() 
{
    // If shutdown was never called, try doing so now.
    if (!mIsShutdown)
        Shutdown() ;
}

///////////////////////////////////////////////////////////////////////////////
int DataTransactionManager::Shutdown() 
{
    ACE_GUARD_RETURN(lock_t, guard, mLock, -1) ;
    
    // if already shutdown, simply return
    if (mIsShutdown)
        return 0 ;

    mIsShutdown = true ;

    if (mHdlr) 
        // no need to have Close Notification or other delegates fired now.
        mHdlr->ClearCloseNotificationDelegate() ;
   
    return CloseConnectorOrAcceptor() ? 0 : -1 ;
}

///////////////////////////////////////////////////////////////////////////////
bool DataTransactionManager::StartDataTx() 
{
    ACE_GUARD_RETURN(lock_t, guard, mLock, 0) ;
    
    if (mIsShutdown)
        return false ;

    mInTxMode = true ;   
 
    if (mConnectionMode)
        mConnOpened = OpenConnector() ;

    return mConnOpened;
}

///////////////////////////////////////////////////////////////////////////////
bool DataTransactionManager::StartDataRx() 
{
    ACE_GUARD_RETURN(lock_t, guard, mLock, 0) ;

    if (mIsShutdown)
        return false ;

    mInTxMode = false;

    if (mConnectionMode)
        mConnOpened = OpenConnector() ;

    return mConnOpened;
}

///////////////////////////////////////////////////////////////////////////////
bool DataTransactionManager::IsConnectionActive(void)
{
    ACE_GUARD_RETURN(lock_t, guard, mLock, 0) ;
    if (mIsShutdown || !mHdlr || !mHdlr->IsOpen())
        return false ;
    return true ;
}

///////////////////////////////////////////////////////////////////////////////
////////////////// Private Methods ////////////////////////////////////////////
bool DataTransactionManager::HandleConnectionNotification(
    DataConnectionHandler& connHandler)
{
    // acquire lock to prevent shutdown while we are handling a new connection.
    ACE_GUARD_RETURN(lock_t, guard, mLock, 0) ;

    if (mIsShutdown)
        return false ;

    // Store the connection handler in a shared pointer with a custom deleter -- 
    // this will call remove_reference() when the shared copy is deleted
    if (!mHdlr)
        mHdlr = dataHdlrPtr_t(&connHandler, 
                              boost::bind(&DataConnectionHandler::remove_reference, 
                                         _1));

    // Register close notification delegate
    mHdlr->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &DataTransactionManager::HandleCloseNotification));

    if (!mInTxMode)
    {
        // Since we're likely to be sinking a lot of data from our peer, increase the input block size up to SO_RCVBUF
        size_t soRcvBuf;
        if (!mHdlr->GetSoRcvBuf(soRcvBuf))
            return false;

        mHdlr->SetInputBlockSize(soRcvBuf);
    }
    
    // setup config on the connection handler
    uint8_t szType, bodyType ;
    uint32_t fixedBodySize, randomBodySizeMean, randomBodySizeStdDeviation ;
    mParent.GetTxBodyConfig(szType, 
                            fixedBodySize, 
                            randomBodySizeMean,
                            randomBodySizeStdDeviation,
                            bodyType) ;    
    mHdlr->SetTxBodyConfig(szType, 
                                 fixedBodySize, 
                                 randomBodySizeMean,
                                 randomBodySizeStdDeviation,
                                 bodyType) ;
    mHdlr->SetDoTx(mInTxMode) ;

    // setup callbacks for stats notifications
    mHdlr->RegisterUpdateTxCountDelegate(fastdelegate::MakeDelegate(this, &DataTransactionManager::BytesTx));
    mHdlr->SetIncrementalRxCallback(fastdelegate::MakeDelegate(this, &DataTransactionManager::BytesRx)) ;

    if (mHdlr)
    {
            // setup dynamic load
            RegisterDynamicLoadDelegates(mHdlr.get());
            mHdlr->EnableBwCtrl(mEnableBwCtrl);
    }

    // let parent control connection know that data connection is open
    mParent.NotifyDataConnectionOpen(ACE_OS::gettimeofday()) ;

    return true ;
}

/////////////////////////////////////////////////////////////////////////////////
void DataTransactionManager::HandleCloseNotification(L4L7_APP_NS::StreamSocketHandler& socketHandler) 
{
    mParent.NotifyDataConnectionClose(ACE_OS::gettimeofday()) ;

    if (!mConnCloseHdlr.empty())
        mConnCloseHdlr(&mParent, this) ;
}

/////////////////////////////////////////////////////////////////////////////////
bool DataTransactionManager::OpenConnector(void)
{
    static const int RETRY_CNT = 2 ;
    
    if (!mConnectionMode)
    {
        FTP_LOG_ERR(mPort, LOG_DTM, "Bad Open connector call") ;
        return false ;
    }

    const std::string &ifName = mParent.GetLocalIfName() ;
    ACE_INET_Addr remote(mParent.GetRemoteDataInetAddr()) ;

    for (int loop = 1 ; loop <= RETRY_CNT; loop++)
    {

#ifdef UNIT_TEST
        char sbuf[256], rbuf[256] ;
        mLocalAddr.addr_to_string(sbuf, 256) ;
        remote.addr_to_string(rbuf, 256) ;
        FTP_LOG_DEBUG(mPort, LOG_DTM, "Opening Data connection: \nLocal address is: " << 
                                      sbuf << " and Remote address is: " << rbuf) ;
#endif
 
        DataConnectionHandler *rawConnHandler = 0;
        int err(0);
        {                     
            if (!ifName.empty())
                mConnector->BindToIfName(&ifName);

            err = mConnector->connect(rawConnHandler, 
                                      remote, 
                                      ACE_Synch_Options(ACE_Synch_Options::USE_REACTOR), 
                                      mLocalAddr, 
                                     1);
        }

        // If the connection attempt failed right away then we must treat
        // this as though the connection was closed
        if (err == -1 && errno != EWOULDBLOCK)
        {
            if (rawConnHandler) 
                rawConnHandler->remove_reference() ;

            rawConnHandler = 0 ;

            FTP_LOG_INFO(mPort, LOG_DTM, "DataMgr: " << std::hex << this << "Data Connectin attempt failed with : " << err << "Errno string = " << strerror(errno)) ;

            if ((loop == RETRY_CNT) || (errno == EMFILE))
            {
                FTP_LOG_ERR(mPort, LOG_DTM, "Connector failed. Local port = " << mLocalAddr.get_port_number()) ;
                return false ;
            }
            else
            {
                FTP_LOG_INFO(mPort, LOG_DTM, "Retrying with different local port number..") ;
                mLocalAddr.set_port_number(0) ; // let system auto-allocate...
                continue ;
            }
        }

        // Ok, we have a pending connection. Save the handler and exit.
        if (!mHdlr)
        {
            // It's possible that HandleConnectionNotification has already
            // been called and placed the handler in the shared pointer.
            // Doing it again here would cause a reference count decrement
            // and early destruction.
            mHdlr = dataHdlrPtr_t(rawConnHandler, 
                                  boost::bind(&DataConnectionHandler::remove_reference, 
                                             _1));
        }
        break ;
    }

    return true ;
}

/////////////////////////////////////////////////////////////////////////////////
bool DataTransactionManager::OpenAcceptor(void)
{       
    const std::string &ifName = mParent.GetLocalIfName() ;
    
    if (mConnectionMode)
    {
        FTP_LOG_ERR(mPort, LOG_DTM, "Bad Open Acceptor call") ;
        return false ;
    }
 
    if (!ifName.empty())
        mAcceptor->BindToIfName(ifName);

    const int err = mAcceptor->accept(mLocalAddr, mReactor) ;
    if (err < 0)
    {
        FTP_LOG_ERR(mPort, LOG_DTM, "Failed to open acceptor. Local port = " << 
                                     mLocalAddr.get_port_number()) ;


        // Close and release the acceptor right away			
        mAcceptor->cancel() ;
        mAcceptor->close();          
        mAcceptor.reset() ;
        return false ;
    }
 
#ifdef UNIT_TEST
    char buffer[256] ;
    mLocalAddr.addr_to_string(buffer, 256) ;
    FTP_LOG_DEBUG(mPort, LOG_DTM, "Opened listener at local address: " << buffer) ;
#endif
    
    return true ;
}

/////////////////////////////////////////////////////////////////////////////////
bool DataTransactionManager::CloseConnectorOrAcceptor()
{     
    FTP_LOG_DEBUG(mPort, LOG_DTM, "CloseConnectorOrAcceptor called. mAcceptor = " << mAcceptor.get() << " mConnector = " << mConnector.get() << " mHdlr = " << mHdlr.get());
     
    if (mConnectionMode)
    {
        if (mConnector)
        {
            // cancel the connection if was still pending...
            if (mHdlr && mHdlr->IsPending())
                mConnector->cancel(mHdlr.get()) ;           

            // close connector
            mConnector->close() ;
        }
    }
    else 
    {
        if (mAcceptor)
        {
            // cancel any pending accepts
            mAcceptor->cancel() ;
            // close the acceptor
            mAcceptor->close() ;          		    
        }
    }

    // Close the connection, if open.
    if (mHdlr)
    {
        UnregisterDynamicLoadDelegates(mHdlr.get());
        mHdlr->close() ;
    }

    return true ;
}

/////////////////////////////////////////////////////////////////////////////////

void DataTransactionManager::BytesTx(uint32_t bytes) 
{
    if (!mIsShutdown)
        mParent.NotifyIncrementalTxBytes(bytes) ; 
}

/////////////////////////////////////////////////////////////////////////////////
void DataTransactionManager::BytesRx(uint32_t bytes)  
{
    if (!mIsShutdown)
       mParent.NotifyIncrementalRxBytes(bytes) ; 
}

/////////////////////////////////////////////////////////////////////////////////

void DataTransactionManager::RegisterDynamicLoadDelegates(DataConnectionHandler *dch)
{
    if (getDlbDelegate.empty() || produceDlbDelegate.empty() || consumeDlbDelegate.empty())
        return;

    dch->RegisterDynamicLoadDelegates(getDlbDelegate, produceDlbDelegate, consumeDlbDelegate);
}

/////////////////////////////////////////////////////////////////////////////////

void DataTransactionManager::UnregisterDynamicLoadDelegates(DataConnectionHandler *dch)
{
    dch->UnregisterDynamicLoadDelegates();
}

/////////////////////////////////////////////////////////////////////////////////

