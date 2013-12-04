/// @file
/// @brief McObjects (MCO) embedded database driver implementation
///
///  Copyright (c) 2009 by Spirent Communications Inc.
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

#include "RawTcpClientRawStats.h"
#include "RawtcpdLog.h"
#include "RawTcpServerRawStats.h"
#include "McoDriver.h"
#include "statsdb.h"

USING_RAWTCP_NS;

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
    ExtremeStatsDBFactory::setDictionary(statsdb_get_dictionary());  // required
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

void McoDriver::InsertRawTcpClientResult(uint32_t handle)
{
    // Select a new index value for this handle
    uint32_t index = mClientIndexTracker->Assign();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Insert a new row in the table
    RawTcpClientResults res;

    // Create a new RawTcpClientResults object
    if (RawTcpClientResults_new(t, &res) != MCO_S_OK)
    {
        // Rollback transaction
        mco_trans_rollback(t);

        // Return unused client index
        mClientIndexTracker->Release(index);
        
        TC_LOG_ERR(mPort, "Could not create a RawTcpClientResults object");
        throw EPHXInternal();
    }

    // Initialize the results fields
    RawTcpClientResults_BlockIndex_put(&res, index);
    RawTcpClientResults_Handle_put(&res, handle);
    RawTcpClientResults_ElapsedSeconds_put(&res, 0);
    RawTcpClientResults_IntendedLoad_put(&res, 0);
    RawTcpClientResults_GoodputRxBytes_put(&res, 0);
    RawTcpClientResults_GoodputRxBps_put(&res, 0);
    RawTcpClientResults_GoodputMinRxBps_put(&res, 0);
    RawTcpClientResults_GoodputMaxRxBps_put(&res, 0);
    RawTcpClientResults_GoodputAvgRxBps_put(&res, 0);
    RawTcpClientResults_GoodputTxBytes_put(&res, 0);
    RawTcpClientResults_GoodputTxBps_put(&res, 0);
    RawTcpClientResults_GoodputMinTxBps_put(&res, 0);
    RawTcpClientResults_GoodputMaxTxBps_put(&res, 0);
    RawTcpClientResults_GoodputAvgTxBps_put(&res, 0);
    RawTcpClientResults_ActiveConnections_put(&res, 0);
    RawTcpClientResults_AttemptedConnections_put(&res, 0);
    RawTcpClientResults_AttemptedConnectionsPerSecond_put(&res, 0);
    RawTcpClientResults_SuccessfulConnections_put(&res, 0);
    RawTcpClientResults_SuccessfulConnectionsPerSecond_put(&res, 0);
    RawTcpClientResults_UnsuccessfulConnections_put(&res, 0);
    RawTcpClientResults_UnsuccessfulConnectionsPerSecond_put(&res, 0);
    RawTcpClientResults_AbortedConnections_put(&res, 0);
    RawTcpClientResults_AbortedConnectionsPerSecond_put(&res, 0);
    RawTcpClientResults_AttemptedTransactions_put(&res, 0);
    RawTcpClientResults_AttemptedTransactionsPerSecond_put(&res, 0);
    RawTcpClientResults_SuccessfulTransactions_put(&res, 0);
    RawTcpClientResults_SuccessfulTransactionsPerSecond_put(&res, 0);
    RawTcpClientResults_UnsuccessfulTransactions_put(&res, 0);
    RawTcpClientResults_UnsuccessfulTransactionsPerSecond_put(&res, 0);
    RawTcpClientResults_AbortedTransactions_put(&res, 0);
    RawTcpClientResults_AbortedTransactionsPerSecond_put(&res, 0);
    RawTcpClientResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    
    // Commit insert into the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        mClientIndexTracker->Release(index);
        
        TC_LOG_ERR(mPort, "Could not commit insertion of RawTcpClientResults object");
        throw EPHXInternal();
    }

    TC_LOG_INFO(mPort, "Created RawTcpClientResults object with index " << index << " for handle " << handle);
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::UpdateRawTcpClientResult(RawTcpClientRawStats& stats)
{
    const uint32_t handle = stats.Handle();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    RawTcpClientResults res;
    mco_cursor_t csr;

    if ((RawTcpClientResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (RawTcpClientResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (RawTcpClientResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not update RawTcpClientResults object with handle " << handle);
        return;
    }

    // Update the results fields
    {
        ACE_GUARD(RawTcpClientRawStats::lock_t, guard, stats.Lock());
        
        RawTcpClientResults_ElapsedSeconds_put(&res, stats.elapsedSeconds);
        RawTcpClientResults_IntendedLoad_put(&res, stats.intendedLoad);
        RawTcpClientResults_GoodputRxBytes_put(&res, stats.goodputRxBytes);
        RawTcpClientResults_GoodputRxBps_put(&res, stats.goodputRxBps);
        RawTcpClientResults_GoodputMinRxBps_put(&res, stats.goodputMinRxBps);
        RawTcpClientResults_GoodputMaxRxBps_put(&res, stats.goodputMaxRxBps);
        RawTcpClientResults_GoodputAvgRxBps_put(&res, stats.goodputAvgRxBps);
        RawTcpClientResults_GoodputTxBytes_put(&res, stats.goodputTxBytes);
        RawTcpClientResults_GoodputTxBps_put(&res, stats.goodputTxBps);
        RawTcpClientResults_GoodputMinTxBps_put(&res, stats.goodputMinTxBps);
        RawTcpClientResults_GoodputMaxTxBps_put(&res, stats.goodputMaxTxBps);
        RawTcpClientResults_GoodputAvgTxBps_put(&res, stats.goodputAvgTxBps);
        RawTcpClientResults_ActiveConnections_put(&res, stats.activeConnections);
        RawTcpClientResults_AttemptedConnections_put(&res, stats.attemptedConnections);
        RawTcpClientResults_AttemptedConnectionsPerSecond_put(&res, stats.attemptedCps);
        RawTcpClientResults_SuccessfulConnections_put(&res, stats.successfulConnections);
        RawTcpClientResults_SuccessfulConnectionsPerSecond_put(&res, stats.successfulCps);
        RawTcpClientResults_UnsuccessfulConnections_put(&res, stats.unsuccessfulConnections);
        RawTcpClientResults_UnsuccessfulConnectionsPerSecond_put(&res, stats.unsuccessfulCps);
        RawTcpClientResults_AbortedConnections_put(&res, stats.abortedConnections);
        RawTcpClientResults_AbortedConnectionsPerSecond_put(&res, stats.abortedCps);
        RawTcpClientResults_AttemptedTransactions_put(&res, stats.attemptedTransactions);
        RawTcpClientResults_AttemptedTransactionsPerSecond_put(&res, stats.attemptedTps);
        RawTcpClientResults_SuccessfulTransactions_put(&res, stats.successfulTransactions);
        RawTcpClientResults_SuccessfulTransactionsPerSecond_put(&res, stats.successfulTps);
        RawTcpClientResults_UnsuccessfulTransactions_put(&res, stats.unsuccessfulTransactions);
        RawTcpClientResults_UnsuccessfulTransactionsPerSecond_put(&res, stats.unsuccessfulTps);
        RawTcpClientResults_AbortedTransactions_put(&res, stats.abortedTransactions);
        RawTcpClientResults_AbortedTransactionsPerSecond_put(&res, stats.abortedTps);
        RawTcpClientResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    }
    
    // Commit the update to the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of RawTcpClientResults object for handle " << handle);
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::DeleteRawTcpClientResult(uint32_t handle)
{
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    RawTcpClientResults res;
    mco_cursor_t csr;

    if ((RawTcpClientResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (RawTcpClientResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (RawTcpClientResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not delete RawTcpClientResults object with handle " << handle);
        return;
    }

    uint32_t index;
    if (RawTcpClientResults_BlockIndex_get(&res, &index) != MCO_S_OK)
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not get RawTcpClientResults index value for handle " << handle);
        return;
    }

    // Delete the row
    RawTcpClientResults_delete(&res);

    // Commit the deletion from the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of RawTcpClientResults object for handle " << handle);
        return;
    }

    // Return now unused client index
    mClientIndexTracker->Release(index);
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::InsertRawTcpServerResult(uint32_t handle)
{
    // Select a new ServerIndex value for this handle
    uint32_t index = mServerIndexTracker->Assign();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Insert a new row in the table
    RawTcpServerResults res;

    // Create a new RawTcpServerResults object
    if (RawTcpServerResults_new(t, &res) != MCO_S_OK)
    {
        // Rollback transaction
        mco_trans_rollback(t);

        // Return unused server index
        mServerIndexTracker->Release(index);
        
        TC_LOG_ERR(mPort, "Could not create a RawTcpServerResults object");
        throw EPHXInternal();
    }

    // Initialize the results fields
    RawTcpServerResults_BlockIndex_put(&res, index);
    RawTcpServerResults_Handle_put(&res, handle);
    RawTcpServerResults_ElapsedSeconds_put(&res, 0);
    RawTcpServerResults_GoodputRxBytes_put(&res, 0);
    RawTcpServerResults_GoodputRxBps_put(&res, 0);
    RawTcpServerResults_GoodputMinRxBps_put(&res, 0);
    RawTcpServerResults_GoodputMaxRxBps_put(&res, 0);
    RawTcpServerResults_GoodputAvgRxBps_put(&res, 0);
    RawTcpServerResults_GoodputTxBytes_put(&res, 0);
    RawTcpServerResults_GoodputTxBps_put(&res, 0);
    RawTcpServerResults_GoodputMinTxBps_put(&res, 0);
    RawTcpServerResults_GoodputMaxTxBps_put(&res, 0);
    RawTcpServerResults_GoodputAvgTxBps_put(&res, 0);
    RawTcpServerResults_ActiveConnections_put(&res, 0);
    RawTcpServerResults_TotalConnections_put(&res, 0);
    RawTcpServerResults_TotalConnectionsPerSecond_put(&res, 0);
    RawTcpServerResults_SuccessfulTransactions_put(&res, 0);
    RawTcpServerResults_SuccessfulTransactionsPerSecond_put(&res, 0);
    RawTcpServerResults_UnsuccessfulTransactions_put(&res, 0);
    RawTcpServerResults_UnsuccessfulTransactionsPerSecond_put(&res, 0);
    RawTcpServerResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    
    // Commit insert into the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        // Return unused server index
        mServerIndexTracker->Release(index);
        
        TC_LOG_ERR(mPort, "Could not commit insertion of RawTcpServerResults object");
        throw EPHXInternal();
    }

    TC_LOG_INFO(mPort, "Created RawTcpServerResults object with index " << index << " for handle " << handle);
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::UpdateRawTcpServerResult(RawTcpServerRawStats& stats)
{
    const uint32_t handle = stats.Handle();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    RawTcpServerResults res;
    mco_cursor_t csr;

    if ((RawTcpServerResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (RawTcpServerResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (RawTcpServerResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not update RawTcpServerResults object with handle " << handle);
        return;
    }

    // Update the results fields
    {
        ACE_GUARD(RawTcpServerRawStats::lock_t, guard, stats.Lock());
        
        RawTcpServerResults_ElapsedSeconds_put(&res, stats.elapsedSeconds);
        RawTcpServerResults_GoodputRxBytes_put(&res, stats.goodputRxBytes);
        RawTcpServerResults_GoodputRxBps_put(&res, stats.goodputRxBps);
        RawTcpServerResults_GoodputMinRxBps_put(&res, stats.goodputMinRxBps);
        RawTcpServerResults_GoodputMaxRxBps_put(&res, stats.goodputMaxRxBps);
        RawTcpServerResults_GoodputAvgRxBps_put(&res, stats.goodputAvgRxBps);
        RawTcpServerResults_GoodputTxBytes_put(&res, stats.goodputTxBytes);
        RawTcpServerResults_GoodputTxBps_put(&res, stats.goodputTxBps);
        RawTcpServerResults_GoodputMinTxBps_put(&res, stats.goodputMinTxBps);
        RawTcpServerResults_GoodputMaxTxBps_put(&res, stats.goodputMaxTxBps);
        RawTcpServerResults_GoodputAvgTxBps_put(&res, stats.goodputAvgTxBps);
        RawTcpServerResults_ActiveConnections_put(&res, stats.activeConnections);
        RawTcpServerResults_TotalConnections_put(&res, stats.totalConnections);
        RawTcpServerResults_TotalConnectionsPerSecond_put(&res, stats.totalCps);
        RawTcpServerResults_SuccessfulTransactions_put(&res, stats.successfulTransactions);
        RawTcpServerResults_SuccessfulTransactionsPerSecond_put(&res, stats.successfulTps);
        RawTcpServerResults_UnsuccessfulTransactions_put(&res, stats.unsuccessfulTransactions);
        RawTcpServerResults_UnsuccessfulTransactionsPerSecond_put(&res, stats.unsuccessfulTps);
        RawTcpServerResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    }
    
    // Commit the update to the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of RawTcpServerResults object for handle " << handle);
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::DeleteRawTcpServerResult(uint32_t handle)
{
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    RawTcpServerResults res;
    mco_cursor_t csr;

    if ((RawTcpServerResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (RawTcpServerResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (RawTcpServerResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not delete RawTcpServerResults object with handle " << handle);
        return;
    }

    uint32_t index;
    if (RawTcpServerResults_BlockIndex_get(&res, &index) != MCO_S_OK)
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not get RawTcpServerResults index value for handle " << handle);
        return;
    }

    // Delete the row
    RawTcpServerResults_delete(&res);

    // Commit the deletion from the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of RawTcpServerResults object for handle " << handle);
        return;
    }

    // Return now unused server index
    mServerIndexTracker->Release(index);
}

///////////////////////////////////////////////////////////////////////////////
