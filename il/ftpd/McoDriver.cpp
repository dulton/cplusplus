/// @file
/// @brief McObjects (MCO) embedded database driver implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <ace/Thread_Mutex.h>
#include <boost/dynamic_bitset.hpp>
#include <hal/TimeStamp.h>
#include <phxexception/PHXException.h>
#include <statsframework/ExtremeStatsDBFactory.h>

#include "FtpClientRawStats.h"
#include "FtpdLog.h"
#include "FtpServerRawStats.h"
#include "ftpstatsdb.h"
#include "McoDriver.h"

USING_FTP_NS;

///////////////////////////////////////////////////////////////////////////////

class McoDriver::UniqueIndexTracker
{
public:
    UniqueIndexTracker(void)
        : mCount(0)
    {
    }

    uint32_t Assign(void)
    {
        ACE_GUARD_RETURN(lock_t, guard, mLock, INVALID_INDEX);

        if (mCount < mBitset.size())
        {
            const size_t pos = mBitset.find_first();
            if (pos == bitset_t::npos)
                throw EPHXInternal();

            mBitset.reset(pos);
            mCount++;

            return static_cast<uint32_t>(pos);
        }
        else
        {
            mBitset.push_back(false);
            mCount++;
            
            return static_cast<uint32_t>(mBitset.size() - 1);
        }
    }

    void Release(uint32_t index)
    {
        ACE_GUARD(lock_t, guard, mLock);
        
        mBitset.set(index);
        mCount--;
    }
    
private:
    typedef ACE_Thread_Mutex lock_t;
    typedef boost::dynamic_bitset<unsigned long> bitset_t;

    static const uint32_t INVALID_INDEX = static_cast<const uint32_t>(-1);
    
    lock_t mLock;               ///< protects against concurrent access
    size_t mCount;              ///< number of index values assigned
    bitset_t mBitset;           ///< dynamic bitset tracking index values (0 = assigned, 1 = unassigned)
};
    
///////////////////////////////////////////////////////////////////////////////

void McoDriver::StaticInitialize(void)
{
    // Configure the stats db factory parameters before it instantiates any databases
    ExtremeStatsDBFactory::setDictionary(ftpstatsdb_get_dictionary());  // required
}

///////////////////////////////////////////////////////////////////////////////

McoDriver::McoDriver(uint16_t port)
    : mPort(port),
      mClientIndexTracker(new UniqueIndexTracker),
      mServerIndexTracker(new UniqueIndexTracker)
{
    // Get the handle to the McObjects database for this port from the StatsDBFactory wrapper class
    StatsDBFactory *dbFactory = StatsDBFactory::instance(mPort);
    mDBFactory = dynamic_cast<ExtremeStatsDBFactory *>(dbFactory);
    if (!mDBFactory)
        throw EPHXInternal();
}

///////////////////////////////////////////////////////////////////////////////

McoDriver::~McoDriver()
{
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::InsertFtpClientResult(uint32_t handle)
{
    // Select a new index value for this handle
    uint32_t index = mClientIndexTracker->Assign();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Insert a new row in the table
    FtpClientResults res;

    // Create a new FtpClientResults object
    if (FtpClientResults_new(t, &res) != MCO_S_OK)
    {
        // Rollback transaction
        mco_trans_rollback(t);

        // Return unused client index
        mClientIndexTracker->Release(index);
        
        TC_LOG_ERR(mPort, "Could not create a FtpClientResults object");
        throw EPHXInternal();
    }

    // Initialize the results fields
    FtpClientResults_BlockIndex_put(&res, index);
    FtpClientResults_Handle_put(&res, handle);
    FtpClientResults_ElapsedSeconds_put(&res, 0);
    FtpClientResults_IntendedLoad_put(&res, 0);
    FtpClientResults_GoodputRxBytes_put(&res, 0);
    FtpClientResults_GoodputRxBps_put(&res, 0);
    FtpClientResults_GoodputMinRxBps_put(&res, 0);
    FtpClientResults_GoodputMaxRxBps_put(&res, 0);
    FtpClientResults_GoodputAvgRxBps_put(&res, 0);
    FtpClientResults_GoodputTxBytes_put(&res, 0);
    FtpClientResults_GoodputTxBps_put(&res, 0);
    FtpClientResults_GoodputMinTxBps_put(&res, 0);
    FtpClientResults_GoodputMaxTxBps_put(&res, 0);
    FtpClientResults_GoodputAvgTxBps_put(&res, 0);
    FtpClientResults_ActiveConnections_put(&res, 0); 
    FtpClientResults_AttemptedConnections_put(&res, 0); 
    FtpClientResults_AttemptedConnectionsPerSecond_put(&res, 0);
    FtpClientResults_SuccessfulConnections_put(&res, 0);   
    FtpClientResults_SuccessfulConnectionsPerSecond_put(&res, 0);
    FtpClientResults_UnsuccessfulConnections_put(&res, 0); 
    FtpClientResults_UnsuccessfulConnectionsPerSecond_put(&res, 0);
    FtpClientResults_AbortedConnections_put(&res, 0);    
    FtpClientResults_AbortedConnectionsPerSecond_put(&res, 0);
    FtpClientResults_AttemptedTransactions_put(&res, 0);   
    FtpClientResults_AttemptedTransactionsPerSecond_put(&res, 0);
    FtpClientResults_SuccessfulTransactions_put(&res, 0);   
    FtpClientResults_SuccessfulTransactionsPerSecond_put(&res, 0);
    FtpClientResults_UnsuccessfulTransactions_put(&res, 0);  
    FtpClientResults_UnsuccessfulTransactionsPerSecond_put(&res, 0);
    FtpClientResults_AbortedTransactions_put(&res, 0);  
    FtpClientResults_AbortedTransactionsPerSecond_put(&res, 0);
    FtpClientResults_AvgFileTransferBps_put(&res, 0); 
    FtpClientResults_RxResponseCode150_put(&res, 0); 
    FtpClientResults_RxResponseCode200_put(&res, 0); 
    FtpClientResults_RxResponseCode220_put(&res, 0); 
    FtpClientResults_RxResponseCode226_put(&res, 0); 
    FtpClientResults_RxResponseCode227_put(&res, 0); 
    FtpClientResults_RxResponseCode230_put(&res, 0); 
    FtpClientResults_RxResponseCode331_put(&res, 0); 
    FtpClientResults_RxResponseCode425_put(&res, 0); 
    FtpClientResults_RxResponseCode426_put(&res, 0); 
    FtpClientResults_RxResponseCode452_put(&res, 0); 
    FtpClientResults_RxResponseCode500_put(&res, 0); 
    FtpClientResults_RxResponseCode502_put(&res, 0); 
    FtpClientResults_RxResponseCode530_put(&res, 0); 
    FtpClientResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());

    // Commit insert into the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        mClientIndexTracker->Release(index);
        
        TC_LOG_ERR(mPort, "Could not commit insertion of FtpClientResults object");
        throw EPHXInternal();
    }

    TC_LOG_INFO(mPort, "Created FtpClientResults object with index " << index << " for handle " << handle);
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::UpdateFtpClientResult(FtpClientRawStats& stats)
{
    const uint32_t handle = stats.Handle();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    FtpClientResults res;
    mco_cursor_t csr;

    if ((FtpClientResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (FtpClientResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (FtpClientResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not update FtpClientResults object with handle " << handle);
        return;
    }

    // Update the results fields
    {
        ACE_GUARD(FtpClientRawStats::lock_t, guard, stats.Lock());
        
        FtpClientResults_ElapsedSeconds_put(&res, stats.elapsedSeconds);
        FtpClientResults_IntendedLoad_put(&res, stats.intendedLoad);
        FtpClientResults_GoodputRxBytes_put(&res, stats.goodputRxBytes);
        FtpClientResults_GoodputRxBps_put(&res, stats.goodputRxBps);
        FtpClientResults_GoodputMinRxBps_put(&res, stats.goodputMinRxBps);
        FtpClientResults_GoodputMaxRxBps_put(&res, stats.goodputMaxRxBps);
        FtpClientResults_GoodputAvgRxBps_put(&res, stats.goodputAvgRxBps);
        FtpClientResults_GoodputTxBytes_put(&res, stats.goodputTxBytes);
        FtpClientResults_GoodputTxBps_put(&res, stats.goodputTxBps);
        FtpClientResults_GoodputMinTxBps_put(&res, stats.goodputMinTxBps);
        FtpClientResults_GoodputMaxTxBps_put(&res, stats.goodputMaxTxBps);
        FtpClientResults_GoodputAvgTxBps_put(&res, stats.goodputAvgTxBps);
        FtpClientResults_ActiveConnections_put(&res, stats.activeConnections); 
        FtpClientResults_AttemptedConnections_put(&res, stats.attemptedConnections); 
        FtpClientResults_AttemptedConnectionsPerSecond_put(&res, stats.attemptedCps);
        FtpClientResults_SuccessfulConnections_put(&res, stats.successfulConnections);      
        FtpClientResults_SuccessfulConnectionsPerSecond_put(&res, stats.successfulCps);
        FtpClientResults_UnsuccessfulConnections_put(&res, stats.unsuccessfulConnections);   
        FtpClientResults_UnsuccessfulConnectionsPerSecond_put(&res, stats.unsuccessfulCps);
        FtpClientResults_AbortedConnections_put(&res, stats.abortedConnections);     
        FtpClientResults_AbortedConnectionsPerSecond_put(&res, stats.abortedCps);
        FtpClientResults_AttemptedTransactions_put(&res, stats.attemptedTransactions);    
        FtpClientResults_AttemptedTransactionsPerSecond_put(&res, stats.attemptedTps);
        FtpClientResults_SuccessfulTransactions_put(&res, stats.successfulTransactions);    
        FtpClientResults_SuccessfulTransactionsPerSecond_put(&res, stats.successfulTps);
        FtpClientResults_UnsuccessfulTransactions_put(&res, stats.unsuccessfulTransactions); 
        FtpClientResults_UnsuccessfulTransactionsPerSecond_put(&res, stats.unsuccessfulTps);
        FtpClientResults_AbortedTransactions_put(&res, stats.abortedTransactions);      
        FtpClientResults_AbortedTransactionsPerSecond_put(&res, stats.abortedTps);
        FtpClientResults_AvgFileTransferBps_put(&res, stats.avgFileTransferBps); 
        FtpClientResults_RxResponseCode150_put(&res, stats.rxResponseCode150); 
        FtpClientResults_RxResponseCode200_put(&res, stats.rxResponseCode200); 
        FtpClientResults_RxResponseCode220_put(&res, stats.rxResponseCode220); 
        FtpClientResults_RxResponseCode226_put(&res, stats.rxResponseCode226); 
        FtpClientResults_RxResponseCode227_put(&res, stats.rxResponseCode227); 
        FtpClientResults_RxResponseCode230_put(&res, stats.rxResponseCode230); 
        FtpClientResults_RxResponseCode331_put(&res, stats.rxResponseCode331); 
        FtpClientResults_RxResponseCode425_put(&res, stats.rxResponseCode425); 
        FtpClientResults_RxResponseCode426_put(&res, stats.rxResponseCode426); 
        FtpClientResults_RxResponseCode452_put(&res, stats.rxResponseCode452); 
        FtpClientResults_RxResponseCode500_put(&res, stats.rxResponseCode500); 
        FtpClientResults_RxResponseCode502_put(&res, stats.rxResponseCode502); 
        FtpClientResults_RxResponseCode530_put(&res, stats.rxResponseCode530); 
        FtpClientResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    }
    
    // Commit the update to the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of FtpClientResults object for handle " << handle);
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::DeleteFtpClientResult(uint32_t handle)
{
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    FtpClientResults res;
    mco_cursor_t csr;

    if ((FtpClientResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (FtpClientResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (FtpClientResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not delete FtpClientResults object with handle " << handle);
        return;
    }

    uint32_t index;
    if (FtpClientResults_BlockIndex_get(&res, &index) != MCO_S_OK)
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not get FtpClientResults index value for handle " << handle);
        return;
    }

    // Delete the row
    FtpClientResults_delete(&res);

    // Commit the deletion from the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of FtpClientResults object for handle " << handle);
        return;
    }

    // Return now unused client index
    mClientIndexTracker->Release(index);
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::InsertFtpServerResult(uint32_t handle)
{
    // Select a new ServerIndex value for this handle
    uint32_t index = mServerIndexTracker->Assign();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Insert a new row in the table
    FtpServerResults res;

    // Create a new FtpServerResults object
    if (FtpServerResults_new(t, &res) != MCO_S_OK)
    {
        // Rollback transaction
        mco_trans_rollback(t);

        // Return unused server index
        mServerIndexTracker->Release(index);
        
        TC_LOG_ERR(mPort, "Could not create a FtpServerResults object");
        throw EPHXInternal();
    }

    // Initialize the results fields
    FtpServerResults_BlockIndex_put(&res, index);
    FtpServerResults_Handle_put(&res, handle);
    FtpServerResults_ElapsedSeconds_put(&res, 0);
    FtpServerResults_GoodputRxBytes_put(&res, 0);
    FtpServerResults_GoodputRxBps_put(&res, 0);
    FtpServerResults_GoodputMinRxBps_put(&res, 0);
    FtpServerResults_GoodputMaxRxBps_put(&res, 0);
    FtpServerResults_GoodputAvgRxBps_put(&res, 0);
    FtpServerResults_GoodputTxBytes_put(&res, 0);
    FtpServerResults_GoodputTxBps_put(&res, 0);
    FtpServerResults_GoodputMinTxBps_put(&res, 0);
    FtpServerResults_GoodputMaxTxBps_put(&res, 0);
    FtpServerResults_GoodputAvgTxBps_put(&res, 0);
    FtpServerResults_TotalControlConnections_put(&res, 0); 
    FtpServerResults_TotalControlConnectionsPerSecond_put(&res, 0);
    FtpServerResults_AbortedControlConnections_put(&res, 0) ; 
    FtpServerResults_AbortedControlConnectionsPerSecond_put(&res, 0) ;
    FtpServerResults_ActiveControlConnections_put(&res, 0); 
    FtpServerResults_TotalDataConnections_put(&res, 0); 
    FtpServerResults_TotalDataConnectionsPerSecond_put(&res, 0);
    FtpServerResults_RxUserCmd_put(&res, 0);
    FtpServerResults_RxPassCmd_put(&res, 0);
    FtpServerResults_RxTypeCmd_put(&res, 0);
    FtpServerResults_RxPortCmd_put(&res, 0);
    FtpServerResults_RxRetrCmd_put(&res, 0);
    FtpServerResults_RxQuitCmd_put(&res, 0);
    FtpServerResults_AttemptedTransactions_put(&res, 0); 
    FtpServerResults_AttemptedTransactionsPerSecond_put(&res, 0);
    FtpServerResults_SuccessfulTransactions_put(&res, 0); 
    FtpServerResults_SuccessfulTransactionsPerSecond_put(&res, 0);
    FtpServerResults_UnsuccessfulTransactions_put(&res, 0); 
    FtpServerResults_UnsuccessfulTransactionsPerSecond_put(&res, 0);
    FtpServerResults_AbortedTransactions_put(&res, 0); 
    FtpServerResults_AbortedTransactionsPerSecond_put(&res, 0);
    FtpServerResults_TxResponseCode150_put(&res, 0);
    FtpServerResults_TxResponseCode200_put(&res, 0);
    FtpServerResults_TxResponseCode220_put(&res, 0);
    FtpServerResults_TxResponseCode226_put(&res, 0);
    FtpServerResults_TxResponseCode227_put(&res, 0);
    FtpServerResults_TxResponseCode230_put(&res, 0);
    FtpServerResults_TxResponseCode331_put(&res, 0);
    FtpServerResults_TxResponseCode425_put(&res, 0);
    FtpServerResults_TxResponseCode426_put(&res, 0);
    FtpServerResults_TxResponseCode452_put(&res, 0);
    FtpServerResults_TxResponseCode500_put(&res, 0);
    FtpServerResults_TxResponseCode502_put(&res, 0);
    FtpServerResults_TxResponseCode530_put(&res, 0);
    FtpServerResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());

    // Commit insert into the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        // Return unused server index
        mServerIndexTracker->Release(index);
        
        TC_LOG_ERR(mPort, "Could not commit insertion of FtpServerResults object");
        throw EPHXInternal();
    }

    TC_LOG_INFO(mPort, "Created FtpServerResults object with index " << index << " for handle " << handle);
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::UpdateFtpServerResult(FtpServerRawStats& stats)
{
    const uint32_t handle = stats.Handle();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    FtpServerResults res;
    mco_cursor_t csr;

    if ((FtpServerResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (FtpServerResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (FtpServerResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not update FtpServerResults object with handle " << handle);
        return;
    }

    // Update the results fields
    {
        ACE_GUARD(FtpServerRawStats::lock_t, guard, stats.Lock());
        
        FtpServerResults_ElapsedSeconds_put(&res, stats.elapsedSeconds);
        FtpServerResults_GoodputRxBytes_put(&res, stats.goodputRxBytes);
        FtpServerResults_GoodputRxBps_put(&res, stats.goodputRxBps);
        FtpServerResults_GoodputMinRxBps_put(&res, stats.goodputMinRxBps);
        FtpServerResults_GoodputMaxRxBps_put(&res, stats.goodputMaxRxBps);
        FtpServerResults_GoodputAvgRxBps_put(&res, stats.goodputAvgRxBps);
        FtpServerResults_GoodputTxBytes_put(&res, stats.goodputTxBytes);
        FtpServerResults_GoodputTxBps_put(&res, stats.goodputTxBps);
        FtpServerResults_GoodputMinTxBps_put(&res, stats.goodputMinTxBps);
        FtpServerResults_GoodputMaxTxBps_put(&res, stats.goodputMaxTxBps);
        FtpServerResults_GoodputAvgTxBps_put(&res, stats.goodputAvgTxBps);
        FtpServerResults_TotalControlConnections_put(&res, stats.totalControlConnections); 
        FtpServerResults_TotalControlConnectionsPerSecond_put(&res, stats.totalControlCps);
        FtpServerResults_AbortedControlConnections_put(&res, stats.abortedControlConnections);    
        FtpServerResults_AbortedControlConnectionsPerSecond_put(&res, stats.abortedControlCps);
        FtpServerResults_ActiveControlConnections_put(&res, stats.activeControlConnections); 
        FtpServerResults_TotalDataConnections_put(&res, stats.totalDataConnections); 
        FtpServerResults_TotalDataConnectionsPerSecond_put(&res, stats.totalDataCps);
        FtpServerResults_RxUserCmd_put(&res, stats.rxUserCmd);
        FtpServerResults_RxPassCmd_put(&res, stats.rxPassCmd);
        FtpServerResults_RxTypeCmd_put(&res, stats.rxTypeCmd);
        FtpServerResults_RxPortCmd_put(&res, stats.rxPortCmd);
        FtpServerResults_RxRetrCmd_put(&res, stats.rxRetrCmd);
        FtpServerResults_RxQuitCmd_put(&res, stats.rxQuitCmd);
        FtpServerResults_AttemptedTransactions_put(&res, stats.attemptedTransactions); 
        FtpServerResults_AttemptedTransactionsPerSecond_put(&res, stats.attemptedTps);
        FtpServerResults_SuccessfulTransactions_put(&res, stats.successfulTransactions);    
        FtpServerResults_SuccessfulTransactionsPerSecond_put(&res, stats.successfulTps);
        FtpServerResults_UnsuccessfulTransactions_put(&res, stats.unsuccessfulTransactions); 
        FtpServerResults_UnsuccessfulTransactionsPerSecond_put(&res, stats.unsuccessfulTps);
        FtpServerResults_AbortedTransactions_put(&res, stats.abortedTransactions); 
        FtpServerResults_AbortedTransactionsPerSecond_put(&res, stats.abortedTps);
        FtpServerResults_TxResponseCode150_put(&res, stats.txResponseCode150);
        FtpServerResults_TxResponseCode200_put(&res, stats.txResponseCode200);
        FtpServerResults_TxResponseCode220_put(&res, stats.txResponseCode220);
        FtpServerResults_TxResponseCode226_put(&res, stats.txResponseCode226);
        FtpServerResults_TxResponseCode227_put(&res, stats.txResponseCode227);
        FtpServerResults_TxResponseCode230_put(&res, stats.txResponseCode230);
        FtpServerResults_TxResponseCode331_put(&res, stats.txResponseCode331);
        FtpServerResults_TxResponseCode425_put(&res, stats.txResponseCode425);
        FtpServerResults_TxResponseCode426_put(&res, stats.txResponseCode426);
        FtpServerResults_TxResponseCode452_put(&res, stats.txResponseCode452);
        FtpServerResults_TxResponseCode500_put(&res, stats.txResponseCode500);
        FtpServerResults_TxResponseCode502_put(&res, stats.txResponseCode502);
        FtpServerResults_TxResponseCode530_put(&res, stats.txResponseCode530);
        FtpServerResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    }
    
    // Commit the update to the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of FtpServerResults object for handle " << handle);
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::DeleteFtpServerResult(uint32_t handle)
{
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    FtpServerResults res;
    mco_cursor_t csr;

    if ((FtpServerResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (FtpServerResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (FtpServerResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not delete FtpServerResults object with handle " << handle);
        return;
    }

    uint32_t index;
    if (FtpServerResults_BlockIndex_get(&res, &index) != MCO_S_OK)
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not get FtpServerResults index value for handle " << handle);
        return;
    }

    // Delete the row
    FtpServerResults_delete(&res);

    // Commit the deletion from the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of FtpServerResults object for handle " << handle);
        return;
    }

    // Return now unused server index
    mServerIndexTracker->Release(index);
}

///////////////////////////////////////////////////////////////////////////////
