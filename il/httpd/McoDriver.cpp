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

#include "HttpClientRawStats.h"
#include "HttpVideoClientRawStats.h"
#include "HttpdLog.h"
#include "HttpServerRawStats.h"
#include "HttpVideoServerRawStats.h"
#include "McoDriver.h"
#include "statsdb.h"

USING_HTTP_NS;

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
      mServerIndexTracker(new UniqueIndexTracker),
      mVideoClientIndexTracker(new UniqueIndexTracker),
      mVideoServerIndexTracker(new UniqueIndexTracker)

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

void McoDriver::InsertHttpClientResult(uint32_t handle)
{
    // Select a new index value for this handle
    uint32_t index = mClientIndexTracker->Assign();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Insert a new row in the table
    HttpClientResults res;

    // Create a new HttpClientResults object
    if (HttpClientResults_new(t, &res) != MCO_S_OK)
    {
        // Rollback transaction
        mco_trans_rollback(t);

        // Return unused client index
        mClientIndexTracker->Release(index);
        
        TC_LOG_ERR(mPort, "Could not create a HttpClientResults object");
        throw EPHXInternal();
    }

    // Initialize the results fields
    HttpClientResults_BlockIndex_put(&res, index);
    HttpClientResults_Handle_put(&res, handle);
    HttpClientResults_ElapsedSeconds_put(&res, 0);
    HttpClientResults_IntendedLoad_put(&res, 0);
    HttpClientResults_GoodputRxBytes_put(&res, 0);
    HttpClientResults_GoodputRxBps_put(&res, 0);
    HttpClientResults_GoodputMinRxBps_put(&res, 0);
    HttpClientResults_GoodputMaxRxBps_put(&res, 0);
    HttpClientResults_GoodputAvgRxBps_put(&res, 0);
    HttpClientResults_GoodputTxBytes_put(&res, 0);
    HttpClientResults_GoodputTxBps_put(&res, 0);
    HttpClientResults_GoodputMinTxBps_put(&res, 0);
    HttpClientResults_GoodputMaxTxBps_put(&res, 0);
    HttpClientResults_GoodputAvgTxBps_put(&res, 0);
    HttpClientResults_ActiveConnections_put(&res, 0);
    HttpClientResults_AttemptedConnections_put(&res, 0);
    HttpClientResults_AttemptedConnectionsPerSecond_put(&res, 0);
    HttpClientResults_SuccessfulConnections_put(&res, 0);
    HttpClientResults_SuccessfulConnectionsPerSecond_put(&res, 0);
    HttpClientResults_UnsuccessfulConnections_put(&res, 0);
    HttpClientResults_UnsuccessfulConnectionsPerSecond_put(&res, 0);
    HttpClientResults_AbortedConnections_put(&res, 0);
    HttpClientResults_AbortedConnectionsPerSecond_put(&res, 0);
    HttpClientResults_AttemptedTransactions_put(&res, 0);
    HttpClientResults_AttemptedTransactionsPerSecond_put(&res, 0);
    HttpClientResults_SuccessfulTransactions_put(&res, 0);
    HttpClientResults_SuccessfulTransactionsPerSecond_put(&res, 0);
    HttpClientResults_UnsuccessfulTransactions_put(&res, 0);
    HttpClientResults_UnsuccessfulTransactionsPerSecond_put(&res, 0);
    HttpClientResults_AbortedTransactions_put(&res, 0);
    HttpClientResults_AbortedTransactionsPerSecond_put(&res, 0);
    HttpClientResults_MinResponseTimePerUrlMsec_put(&res, 0);
    HttpClientResults_MaxResponseTimePerUrlMsec_put(&res, 0);
    HttpClientResults_AvgResponseTimePerUrlMsec_put(&res, 0);
    HttpClientResults_SumResponseTimePerUrlMsec_put(&res, 0);
    HttpClientResults_RxResponseCode200_put(&res, 0);
    HttpClientResults_RxResponseCode400_put(&res, 0);
    HttpClientResults_RxResponseCode404_put(&res, 0);
    HttpClientResults_RxResponseCode405_put(&res, 0);
    HttpClientResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    
    // Commit insert into the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        mClientIndexTracker->Release(index);
        
        TC_LOG_ERR(mPort, "Could not commit insertion of HttpClientResults object");
        throw EPHXInternal();
    }

    TC_LOG_INFO(mPort, "Created HttpClientResults object with index " << index << " for handle " << handle);
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::UpdateHttpClientResult(HttpClientRawStats& stats)
{
    const uint32_t handle = stats.Handle();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    HttpClientResults res;
    mco_cursor_t csr;

    if ((HttpClientResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (HttpClientResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (HttpClientResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not update HttpClientResults object with handle " << handle);
        return;
    }

    // Update the results fields
    {
        ACE_GUARD(HttpClientRawStats::lock_t, guard, stats.Lock());
        
        HttpClientResults_ElapsedSeconds_put(&res, stats.elapsedSeconds);
        HttpClientResults_IntendedLoad_put(&res, stats.intendedLoad);
        HttpClientResults_GoodputRxBytes_put(&res, stats.goodputRxBytes);
        HttpClientResults_GoodputRxBps_put(&res, stats.goodputRxBps);
        HttpClientResults_GoodputMinRxBps_put(&res, stats.goodputMinRxBps);
        HttpClientResults_GoodputMaxRxBps_put(&res, stats.goodputMaxRxBps);
        HttpClientResults_GoodputAvgRxBps_put(&res, stats.goodputAvgRxBps);
        HttpClientResults_GoodputTxBytes_put(&res, stats.goodputTxBytes);
        HttpClientResults_GoodputTxBps_put(&res, stats.goodputTxBps);
        HttpClientResults_GoodputMinTxBps_put(&res, stats.goodputMinTxBps);
        HttpClientResults_GoodputMaxTxBps_put(&res, stats.goodputMaxTxBps);
        HttpClientResults_GoodputAvgTxBps_put(&res, stats.goodputAvgTxBps);
        HttpClientResults_ActiveConnections_put(&res, stats.activeConnections);
        HttpClientResults_AttemptedConnections_put(&res, stats.attemptedConnections);
        HttpClientResults_AttemptedConnectionsPerSecond_put(&res, stats.attemptedCps);
        HttpClientResults_SuccessfulConnections_put(&res, stats.successfulConnections);
        HttpClientResults_SuccessfulConnectionsPerSecond_put(&res, stats.successfulCps);
        HttpClientResults_UnsuccessfulConnections_put(&res, stats.unsuccessfulConnections);
        HttpClientResults_UnsuccessfulConnectionsPerSecond_put(&res, stats.unsuccessfulCps);
        HttpClientResults_AbortedConnections_put(&res, stats.abortedConnections);
        HttpClientResults_AbortedConnectionsPerSecond_put(&res, stats.abortedCps);
        HttpClientResults_AttemptedTransactions_put(&res, stats.attemptedTransactions);
        HttpClientResults_AttemptedTransactionsPerSecond_put(&res, stats.attemptedTps);
        HttpClientResults_SuccessfulTransactions_put(&res, stats.successfulTransactions);
        HttpClientResults_SuccessfulTransactionsPerSecond_put(&res, stats.successfulTps);
        HttpClientResults_UnsuccessfulTransactions_put(&res, stats.unsuccessfulTransactions);
        HttpClientResults_UnsuccessfulTransactionsPerSecond_put(&res, stats.unsuccessfulTps);
        HttpClientResults_AbortedTransactions_put(&res, stats.abortedTransactions);
        HttpClientResults_AbortedTransactionsPerSecond_put(&res, stats.abortedTps);
        HttpClientResults_MinResponseTimePerUrlMsec_put(&res, stats.minResponseTimePerUrlMsec);
        HttpClientResults_MaxResponseTimePerUrlMsec_put(&res, stats.maxResponseTimePerUrlMsec);
        HttpClientResults_AvgResponseTimePerUrlMsec_put(&res, stats.avgResponseTimePerUrlMsec);
        HttpClientResults_SumResponseTimePerUrlMsec_put(&res, stats.sumResponseTimePerUrlMsec);
        HttpClientResults_RxResponseCode200_put(&res, stats.rxResponseCode200);
        HttpClientResults_RxResponseCode400_put(&res, stats.rxResponseCode400);
        HttpClientResults_RxResponseCode404_put(&res, stats.rxResponseCode404);
        HttpClientResults_RxResponseCode405_put(&res, stats.rxResponseCode405);
        HttpClientResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    }
    
    // Commit the update to the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of HttpClientResults object for handle " << handle);
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::DeleteHttpClientResult(uint32_t handle)
{
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    HttpClientResults res;
    mco_cursor_t csr;

    if ((HttpClientResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (HttpClientResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (HttpClientResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not delete HttpClientResults object with handle " << handle);
        return;
    }

    uint32_t index;
    if (HttpClientResults_BlockIndex_get(&res, &index) != MCO_S_OK)
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not get HttpClientResults index value for handle " << handle);
        return;
    }

    // Delete the row
    HttpClientResults_delete(&res);

    // Commit the deletion from the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of HttpClientResults object for handle " << handle);
        return;
    }

    // Return now unused client index
    mClientIndexTracker->Release(index);
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::InsertHttpServerResult(uint32_t handle)
{
    // Select a new ServerIndex value for this handle
    uint32_t index = mServerIndexTracker->Assign();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Insert a new row in the table
    HttpServerResults res;

    // Create a new HttpServerResults object
    if (HttpServerResults_new(t, &res) != MCO_S_OK)
    {
        // Rollback transaction
        mco_trans_rollback(t);

        // Return unused server index
        mServerIndexTracker->Release(index);
        
        TC_LOG_ERR(mPort, "Could not create a HttpServerResults object");
        throw EPHXInternal();
    }

    // Initialize the results fields
    HttpServerResults_BlockIndex_put(&res, index);
    HttpServerResults_Handle_put(&res, handle);
    HttpServerResults_ElapsedSeconds_put(&res, 0);
    HttpServerResults_GoodputRxBytes_put(&res, 0);
    HttpServerResults_GoodputRxBps_put(&res, 0);
    HttpServerResults_GoodputMinRxBps_put(&res, 0);
    HttpServerResults_GoodputMaxRxBps_put(&res, 0);
    HttpServerResults_GoodputAvgRxBps_put(&res, 0);
    HttpServerResults_GoodputTxBytes_put(&res, 0);
    HttpServerResults_GoodputTxBps_put(&res, 0);
    HttpServerResults_GoodputMinTxBps_put(&res, 0);
    HttpServerResults_GoodputMaxTxBps_put(&res, 0);
    HttpServerResults_GoodputAvgTxBps_put(&res, 0);
    HttpServerResults_TotalConnections_put(&res, 0);
    HttpServerResults_TotalConnectionsPerSecond_put(&res, 0);
    HttpServerResults_ActiveConnections_put(&res, 0);
    HttpServerResults_SuccessfulTransactions_put(&res, 0);
    HttpServerResults_SuccessfulTransactionsPerSecond_put(&res, 0);
    HttpServerResults_UnsuccessfulTransactions_put(&res, 0);
    HttpServerResults_UnsuccessfulTransactionsPerSecond_put(&res, 0);
    HttpServerResults_TxResponseCode200_put(&res, 0);
    HttpServerResults_TxResponseCode400_put(&res, 0);
    HttpServerResults_TxResponseCode404_put(&res, 0);
    HttpServerResults_TxResponseCode405_put(&res, 0);
    HttpServerResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    
    // Commit insert into the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        // Return unused server index
        mServerIndexTracker->Release(index);
        
        TC_LOG_ERR(mPort, "Could not commit insertion of HttpServerResults object");
        throw EPHXInternal();
    }

    TC_LOG_INFO(mPort, "Created HttpServerResults object with index " << index << " for handle " << handle);
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::UpdateHttpServerResult(HttpServerRawStats& stats)
{
    const uint32_t handle = stats.Handle();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    HttpServerResults res;
    mco_cursor_t csr;

    if ((HttpServerResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (HttpServerResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (HttpServerResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not update HttpServerResults object with handle " << handle);
        return;
    }

    // Update the results fields
    {
        ACE_GUARD(HttpServerRawStats::lock_t, guard, stats.Lock());
        
        HttpServerResults_ElapsedSeconds_put(&res, stats.elapsedSeconds);
        HttpServerResults_GoodputRxBytes_put(&res, stats.goodputRxBytes);
        HttpServerResults_GoodputRxBps_put(&res, stats.goodputRxBps);
        HttpServerResults_GoodputMinRxBps_put(&res, stats.goodputMinRxBps);
        HttpServerResults_GoodputMaxRxBps_put(&res, stats.goodputMaxRxBps);
        HttpServerResults_GoodputAvgRxBps_put(&res, stats.goodputAvgRxBps);
        HttpServerResults_GoodputTxBytes_put(&res, stats.goodputTxBytes);
        HttpServerResults_GoodputTxBps_put(&res, stats.goodputTxBps);
        HttpServerResults_GoodputMinTxBps_put(&res, stats.goodputMinTxBps);
        HttpServerResults_GoodputMaxTxBps_put(&res, stats.goodputMaxTxBps);
        HttpServerResults_GoodputAvgTxBps_put(&res, stats.goodputAvgTxBps);
        HttpServerResults_TotalConnections_put(&res, stats.totalConnections);
        HttpServerResults_TotalConnectionsPerSecond_put(&res, stats.totalCps);
        HttpServerResults_ActiveConnections_put(&res, stats.activeConnections);
        HttpServerResults_SuccessfulTransactions_put(&res, stats.successfulTransactions);
        HttpServerResults_SuccessfulTransactionsPerSecond_put(&res, stats.successfulTps);
        HttpServerResults_UnsuccessfulTransactions_put(&res, stats.unsuccessfulTransactions);
        HttpServerResults_UnsuccessfulTransactionsPerSecond_put(&res, stats.unsuccessfulTps);
        HttpServerResults_TxResponseCode200_put(&res, stats.txResponseCode200);
        HttpServerResults_TxResponseCode400_put(&res, stats.txResponseCode400);
        HttpServerResults_TxResponseCode404_put(&res, stats.txResponseCode404);
        HttpServerResults_TxResponseCode405_put(&res, stats.txResponseCode405);
        HttpServerResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    }
    
    // Commit the update to the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of HttpServerResults object for handle " << handle);
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::DeleteHttpServerResult(uint32_t handle)
{
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    HttpServerResults res;
    mco_cursor_t csr;

    if ((HttpServerResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (HttpServerResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (HttpServerResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not delete HttpServerResults object with handle " << handle);
        return;
    }

    uint32_t index;
    if (HttpServerResults_BlockIndex_get(&res, &index) != MCO_S_OK)
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not get HttpServerResults index value for handle " << handle);
        return;
    }

    // Delete the row
    HttpServerResults_delete(&res);

    // Commit the deletion from the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of HttpServerResults object for handle " << handle);
        return;
    }

    // Return now unused server index
    mServerIndexTracker->Release(index);
}

///////////////////////////////////////////////////////////////////////////////
// ABR Specific Statistics
///////////////////////////////////////////////////////////////////////////////

void McoDriver::InsertHttpVideoClientResult(uint32_t handle)
{
    // Select a new index value for this handle
    uint32_t index = mVideoClientIndexTracker->Assign();

    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Insert a new row in the table
    HttpVideoClientResults res;

    // Create a new HttpClientResults object
    if (HttpVideoClientResults_new(t, &res) != MCO_S_OK)
    {
        // Rollback transaction
        mco_trans_rollback(t);

        // Return unused client index
        mVideoClientIndexTracker->Release(index);

        TC_LOG_ERR(mPort, "Could not create a HttpVideoClientResults object");
        throw EPHXInternal();
    }

    // Initialize the results fields
    HttpVideoClientResults_Handle_put(&res, handle);
    HttpVideoClientResults_BlockIndex_put(&res, index);
    HttpVideoClientResults_AsSessionsAttempted_put(&res, 0);
    HttpVideoClientResults_AsSessionsSuccessful_put(&res, 0);
    HttpVideoClientResults_AsSessionsUnsuccessful_put(&res, 0);
    HttpVideoClientResults_ManifestReqsAttempted_put(&res, 0);
    HttpVideoClientResults_ManifestReqsSuccessful_put(&res, 0);
    HttpVideoClientResults_ManifestReqsUnsuccessful_put(&res, 0);
    HttpVideoClientResults_MinBufferingWaitTime_put(&res, 0);
    HttpVideoClientResults_AvgBufferingWaitTime_put(&res, 0);
    HttpVideoClientResults_MaxBufferingWaitTime_put(&res, 0);
    HttpVideoClientResults_MediaFragmentsRx64kbps_put(&res, 0);
    HttpVideoClientResults_MediaFragmentsRx96kbps_put(&res, 0);
    HttpVideoClientResults_MediaFragmentsRx150kbps_put(&res, 0);
    HttpVideoClientResults_MediaFragmentsRx240kbps_put(&res, 0);
    HttpVideoClientResults_MediaFragmentsRx256kbps_put(&res, 0);
    HttpVideoClientResults_MediaFragmentsRx440kbps_put(&res, 0);
    HttpVideoClientResults_MediaFragmentsRx640kbps_put(&res, 0);
    HttpVideoClientResults_MediaFragmentsRx800kbps_put(&res, 0);
    HttpVideoClientResults_MediaFragmentsRx840kbps_put(&res, 0);
    HttpVideoClientResults_MediaFragmentsRx1240kbps_put(&res, 0);
    HttpVideoClientResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());

    // Commit insert into the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        mVideoClientIndexTracker->Release(index);

        TC_LOG_ERR(mPort, "Could not commit insertion of HttpClientResults object");
        throw EPHXInternal();
    }

    TC_LOG_INFO(mPort, "Created HttpVideoClientResults object with index " << index << " for handle " << handle);
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::UpdateHttpVideoClientResult(HttpVideoClientRawStats& stats)
{
    const uint32_t handle = stats.Handle();

    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    HttpVideoClientResults res;
    mco_cursor_t csr;

    if ((HttpVideoClientResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (HttpVideoClientResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (HttpVideoClientResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not update HttpVideoClientResults object with handle " << handle);
        return;
    }

    // Update the results fields
    {
        ACE_GUARD(HttpVideoClientRawStats::lock_t, guard, stats.Lock());

        HttpVideoClientResults_AsSessionsAttempted_put(&res, stats.asSessionsAttempted);
        HttpVideoClientResults_AsSessionsSuccessful_put(&res, stats.asSessionsSuccessful);
        HttpVideoClientResults_AsSessionsUnsuccessful_put(&res, stats.asSessionsUnsuccessful);
        HttpVideoClientResults_ManifestReqsAttempted_put(&res, stats.manifestReqsAttempted);
        HttpVideoClientResults_ManifestReqsSuccessful_put(&res, stats.manifestReqsSuccessful);
        HttpVideoClientResults_ManifestReqsUnsuccessful_put(&res, stats.manifestReqsUnsuccessful);
        HttpVideoClientResults_MinBufferingWaitTime_put(&res, stats.minBufferingWaitTime);
        HttpVideoClientResults_AvgBufferingWaitTime_put(&res, stats.avgBufferingWaitTime);
        HttpVideoClientResults_MaxBufferingWaitTime_put(&res, stats.maxBufferingWaitTime);
        HttpVideoClientResults_MediaFragmentsRx64kbps_put(&res, stats.mediaFragmentsRx64kbps);
        HttpVideoClientResults_MediaFragmentsRx96kbps_put(&res, stats.mediaFragmentsRx96kbps);
        HttpVideoClientResults_MediaFragmentsRx150kbps_put(&res, stats.mediaFragmentsRx150kbps);
        HttpVideoClientResults_MediaFragmentsRx240kbps_put(&res, stats.mediaFragmentsRx240kbps);
        HttpVideoClientResults_MediaFragmentsRx256kbps_put(&res, stats.mediaFragmentsRx256kbps);
        HttpVideoClientResults_MediaFragmentsRx440kbps_put(&res, stats.mediaFragmentsRx440kbps);
        HttpVideoClientResults_MediaFragmentsRx640kbps_put(&res, stats.mediaFragmentsRx640kbps);
        HttpVideoClientResults_MediaFragmentsRx800kbps_put(&res, stats.mediaFragmentsRx800kbps);
        HttpVideoClientResults_MediaFragmentsRx840kbps_put(&res, stats.mediaFragmentsRx840kbps);
        HttpVideoClientResults_MediaFragmentsRx1240kbps_put(&res, stats.mediaFragmentsRx1240kbps);
        HttpVideoClientResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    }

    // Commit the update to the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of HttpVideoClientResults object for handle " << handle);
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::DeleteHttpVideoClientResult(uint32_t handle)
{
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    HttpVideoClientResults res;
    mco_cursor_t csr;

    if ((HttpVideoClientResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (HttpVideoClientResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (HttpVideoClientResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not delete HttpVideoClientResults object with handle " << handle);
        return;
    }

    uint32_t index;
    if (HttpVideoClientResults_BlockIndex_get(&res, &index) != MCO_S_OK)
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not get HttpVideoClientResults index value for handle " << handle);
        return;
    }

    // Delete the row
    HttpVideoClientResults_delete(&res);

    // Commit the deletion from the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of HttpVideoClientResults object for handle " << handle);
        return;
    }

    // Return now unused client index
    mVideoClientIndexTracker->Release(index);
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::InsertHttpVideoServerResult(uint32_t handle)
{
    // Select a new ServerIndex value for this handle
    uint32_t index = mVideoServerIndexTracker->Assign();

    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Insert a new row in the table
    HttpVideoServerResults res;

    // Create a new HttpVideoServerResults object
    if (HttpVideoServerResults_new(t, &res) != MCO_S_OK)
    {
        // Rollback transaction
        mco_trans_rollback(t);

        // Return unused server index
        mVideoServerIndexTracker->Release(index);

        TC_LOG_ERR(mPort, "Could not create a HttpVideoServerResults object");
        throw EPHXInternal();
    }

    // Initialize the results fields
    HttpVideoServerResults_BlockIndex_put(&res, index);
    HttpVideoServerResults_Handle_put(&res, handle);
    HttpVideoServerResults_TotalManifestFilesTx_put(&res, 0);
    HttpVideoServerResults_MediaFragmentsTx64kbps_put(&res, 0);
    HttpVideoServerResults_MediaFragmentsTx96kbps_put(&res, 0);
    HttpVideoServerResults_MediaFragmentsTx150kbps_put(&res, 0);
    HttpVideoServerResults_MediaFragmentsTx240kbps_put(&res, 0);
    HttpVideoServerResults_MediaFragmentsTx256kbps_put(&res, 0);
    HttpVideoServerResults_MediaFragmentsTx440kbps_put(&res, 0);
    HttpVideoServerResults_MediaFragmentsTx640kbps_put(&res, 0);
    HttpVideoServerResults_MediaFragmentsTx800kbps_put(&res, 0);
    HttpVideoServerResults_MediaFragmentsTx840kbps_put(&res, 0);
    HttpVideoServerResults_MediaFragmentsTx1240kbps_put(&res, 0);
    HttpVideoServerResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());

    // Commit insert into the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        // Return unused server index
        mVideoServerIndexTracker->Release(index);

        TC_LOG_ERR(mPort, "Could not commit insertion of HttpVideoServerResults object");
        throw EPHXInternal();
    }

    TC_LOG_INFO(mPort, "Created HttpVideoServerResults object with index " << index << " for handle " << handle);
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::UpdateHttpVideoServerResult(HttpVideoServerRawStats& stats)
{
    const uint32_t handle = stats.Handle();

    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    HttpVideoServerResults res;
    mco_cursor_t csr;

    if ((HttpVideoServerResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (HttpVideoServerResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (HttpVideoServerResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not update HttpVideoServerResults object with handle " << handle);
        return;
    }

    // Update the results fields
    {
        ACE_GUARD(HttpVideoServerRawStats::lock_t, guard, stats.Lock());

        HttpVideoServerResults_TotalManifestFilesTx_put(&res, stats.totalTxManifestFiles);
        HttpVideoServerResults_MediaFragmentsTx64kbps_put(&res, stats.txMediaFragments64Kbps);
        HttpVideoServerResults_MediaFragmentsTx96kbps_put(&res, stats.txMediaFragments96Kbps);
        HttpVideoServerResults_MediaFragmentsTx150kbps_put(&res, stats.txMediaFragments150Kbps);
        HttpVideoServerResults_MediaFragmentsTx240kbps_put(&res, stats.txMediaFragments240Kbps);
        HttpVideoServerResults_MediaFragmentsTx256kbps_put(&res, stats.txMediaFragments256Kbps);
        HttpVideoServerResults_MediaFragmentsTx440kbps_put(&res, stats.txMediaFragments440Kbps);
        HttpVideoServerResults_MediaFragmentsTx640kbps_put(&res, stats.txMediaFragments640Kbps);
        HttpVideoServerResults_MediaFragmentsTx800kbps_put(&res, stats.txMediaFragments800Kbps);
        HttpVideoServerResults_MediaFragmentsTx840kbps_put(&res, stats.txMediaFragments840Kbps);
        HttpVideoServerResults_MediaFragmentsTx1240kbps_put(&res, stats.txMediaFragments1240Kbps);
        HttpVideoServerResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    }

    // Commit the update to the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of HttpVideoServerResults object for handle " << handle);
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::DeleteHttpVideoServerResult(uint32_t handle)
{
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    HttpVideoServerResults res;
    mco_cursor_t csr;

    if ((HttpVideoServerResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (HttpVideoServerResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (HttpVideoServerResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not delete HttpVideoServerResults object with handle " << handle);
        return;
    }

    uint32_t index;
    if (HttpVideoServerResults_BlockIndex_get(&res, &index) != MCO_S_OK)
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not get HttpVideoServerResults index value for handle " << handle);
        return;
    }

    // Delete the row
    HttpVideoServerResults_delete(&res);

    // Commit the deletion from the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of HttpVideoServerResults object for handle " << handle);
        return;
    }

    // Return now unused server index
    mVideoServerIndexTracker->Release(index);
}

///////////////////////////////////////////////////////////////////////////////
