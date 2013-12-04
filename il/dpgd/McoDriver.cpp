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

#include "DpgClientRawStats.h"
#include "DpgdLog.h"
#include "DpgServerRawStats.h"
#include "McoDriver.h"
#include "statsdb.h"

USING_DPG_NS;

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
                throw EPHXInternal("find_first failed");

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
        throw EPHXInternal("db factory null");
}

///////////////////////////////////////////////////////////////////////////////

McoDriver::~McoDriver()
{
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::InsertDpgClientResult(uint32_t handle)
{
    // Select a new index value for this handle
    uint32_t index = mClientIndexTracker->Assign();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Insert a new row in the table
    DpgClientResults res;

    // Create a new DpgClientResults object
    if (DpgClientResults_new(t, &res) != MCO_S_OK)
    {
        // Rollback transaction
        mco_trans_rollback(t);

        // Return unused client index
        mClientIndexTracker->Release(index);
        
        std::string err("Could not create a DpgClientResults object");
        TC_LOG_ERR(mPort, err);
        throw EPHXInternal(err);
    }

    // Initialize the nonzero results fields
    DpgClientResults_BlockIndex_put(&res, index);
    DpgClientResults_Handle_put(&res, handle);
    DpgClientResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    
    // Commit insert into the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        mClientIndexTracker->Release(index);
        
        std::string err("Could not commit insertion of DpgClientResults object");
        TC_LOG_ERR(mPort, err);
        throw EPHXInternal(err);
    }

    TC_LOG_INFO(mPort, "Created DpgClientResults object with index " << index << " for handle " << handle);
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::UpdateDpgClientResult(DpgClientRawStats& stats)
{
    const uint32_t handle = stats.Handle();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    DpgClientResults res;
    mco_cursor_t csr;

    if ((DpgClientResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (DpgClientResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (DpgClientResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not update DpgClientResults object with handle " << handle);
        return;
    }

    // Update the results fields
    {
        ACE_GUARD(DpgClientRawStats::lock_t, guard, stats.Lock());
        
        DpgClientResults_ElapsedSeconds_put(&res, stats.elapsedSeconds);
        DpgClientResults_IntendedLoad_put(&res, stats.intendedLoad);

        DpgClientResults_ActiveConnections_put(&res, stats.activeConnections);
        DpgClientResults_AttemptedConnections_put(&res, stats.attemptedConnections);
        DpgClientResults_SuccessfulConnections_put(&res, stats.successfulConnections);
        DpgClientResults_UnsuccessfulConnections_put(&res, stats.unsuccessfulConnections);
        DpgClientResults_AbortedConnections_put(&res, stats.abortedConnections);


        DpgClientResults_AttemptedConnectionsPerSecond_put(&res, stats.attemptedCps);
        DpgClientResults_SuccessfulConnectionsPerSecond_put(&res, stats.successfulCps);
        DpgClientResults_UnsuccessfulConnectionsPerSecond_put(&res, stats.unsuccessfulCps);
        DpgClientResults_AbortedConnectionsPerSecond_put(&res, stats.abortedCps);

        DpgClientResults_ActivePlaylists_put(&res, stats.activePlaylists);
        DpgClientResults_AttemptedPlaylists_put(&res, stats.attemptedPlaylists);
        DpgClientResults_SuccessfulPlaylists_put(&res, stats.successfulPlaylists);
        DpgClientResults_UnsuccessfulPlaylists_put(&res, stats.unsuccessfulPlaylists);
        DpgClientResults_AbortedPlaylists_put(&res, stats.abortedPlaylists);


        DpgClientResults_AttemptedPlaylistsPerSecond_put(&res, stats.attemptedPps);
        DpgClientResults_SuccessfulPlaylistsPerSecond_put(&res, stats.successfulPps);
        DpgClientResults_UnsuccessfulPlaylistsPerSecond_put(&res, stats.unsuccessfulPps);
        DpgClientResults_AbortedPlaylistsPerSecond_put(&res, stats.abortedPps);

        DpgClientResults_GoodputRxBytes_put(&res, stats.goodputRxBytes);
        DpgClientResults_GoodputTxBytes_put(&res, stats.goodputTxBytes);
        DpgClientResults_GoodputRxBps_put(&res, stats.goodputRxBps);
        DpgClientResults_GoodputTxBps_put(&res, stats.goodputTxBps);
        DpgClientResults_GoodputAvgRxBps_put(&res, stats.goodputAvgRxBps);
        DpgClientResults_GoodputAvgTxBps_put(&res, stats.goodputAvgTxBps);
        DpgClientResults_GoodputMaxRxBps_put(&res, stats.goodputMaxRxBps);
        DpgClientResults_GoodputMaxTxBps_put(&res, stats.goodputMaxTxBps);
        DpgClientResults_GoodputMinRxBps_put(&res, stats.goodputMinRxBps);
        DpgClientResults_GoodputMinTxBps_put(&res, stats.goodputMinTxBps);

        DpgClientResults_TxAttack_put(&res, stats.txAttack);

        DpgClientResults_PlaylistAvgDuration_put(&res, stats.playlistAvgDuration);
        DpgClientResults_PlaylistMaxDuration_put(&res, stats.playlistMaxDuration);
        DpgClientResults_PlaylistMinDuration_put(&res, stats.playlistMinDuration);

        DpgClientResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    }
    
    // Commit the update to the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of DpgClientResults object for handle " << handle);
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::DeleteDpgClientResult(uint32_t handle)
{
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    DpgClientResults res;
    mco_cursor_t csr;

    if ((DpgClientResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (DpgClientResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (DpgClientResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not delete DpgClientResults object with handle " << handle);
        return;
    }

    uint32_t index;
    if (DpgClientResults_BlockIndex_get(&res, &index) != MCO_S_OK)
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not get DpgClientResults index value for handle " << handle);
        return;
    }

    // Delete the row
    DpgClientResults_delete(&res);

    // Commit the deletion from the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of DpgClientResults object for handle " << handle);
        return;
    }

    // Return now unused client index
    mClientIndexTracker->Release(index);
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::InsertDpgServerResult(uint32_t handle)
{
    // Select a new ServerIndex value for this handle
    uint32_t index = mServerIndexTracker->Assign();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Insert a new row in the table
    DpgServerResults res;

    // Create a new DpgServerResults object
    if (DpgServerResults_new(t, &res) != MCO_S_OK)
    {
        // Rollback transaction
        mco_trans_rollback(t);

        // Return unused server index
        mServerIndexTracker->Release(index);
        
        std::string err("Could not create a DpgServerResults object");
        TC_LOG_ERR(mPort, err);
        throw EPHXInternal(err);
    }

    // Initialize the nonzero results fields
    DpgServerResults_BlockIndex_put(&res, index);
    DpgServerResults_Handle_put(&res, handle);
    DpgServerResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    
    // Commit insert into the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        // Return unused server index
        mServerIndexTracker->Release(index);
        
        std::string err("Could not commit insertion of DpgServerResults object");
        TC_LOG_ERR(mPort, err);
        throw EPHXInternal(err);
    }

    TC_LOG_INFO(mPort, "Created DpgServerResults object with index " << index << " for handle " << handle);
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::UpdateDpgServerResult(DpgServerRawStats& stats)
{
    const uint32_t handle = stats.Handle();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    DpgServerResults res;
    mco_cursor_t csr;

    if ((DpgServerResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (DpgServerResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (DpgServerResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not update DpgServerResults object with handle " << handle);
        return;
    }

    // Update the results fields
    {
        ACE_GUARD(DpgServerRawStats::lock_t, guard, stats.Lock());
        
        DpgServerResults_ElapsedSeconds_put(&res, stats.elapsedSeconds);

        DpgServerResults_ActiveConnections_put(&res, stats.activeConnections);
        DpgServerResults_TotalConnections_put(&res, stats.totalConnections);
        DpgServerResults_TotalConnectionsPerSecond_put(&res, stats.totalCps);

        DpgServerResults_ActivePlaylists_put(&res, stats.activePlaylists);
        DpgServerResults_TotalPlaylists_put(&res, stats.totalPlaylists);
        DpgServerResults_TotalPlaylistsPerSecond_put(&res, stats.totalPps);

        DpgServerResults_GoodputRxBytes_put(&res, stats.goodputRxBytes);
        DpgServerResults_GoodputTxBytes_put(&res, stats.goodputTxBytes);
        DpgServerResults_GoodputRxBps_put(&res, stats.goodputRxBps);
        DpgServerResults_GoodputTxBps_put(&res, stats.goodputTxBps);
        DpgServerResults_GoodputAvgRxBps_put(&res, stats.goodputAvgRxBps);
        DpgServerResults_GoodputAvgTxBps_put(&res, stats.goodputAvgTxBps);
        DpgServerResults_GoodputMaxRxBps_put(&res, stats.goodputMaxRxBps);
        DpgServerResults_GoodputMaxTxBps_put(&res, stats.goodputMaxTxBps);
        DpgServerResults_GoodputMinRxBps_put(&res, stats.goodputMinRxBps);
        DpgServerResults_GoodputMinTxBps_put(&res, stats.goodputMinTxBps);

        DpgServerResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    }
    
    // Commit the update to the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of DpgServerResults object for handle " << handle);
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::DeleteDpgServerResult(uint32_t handle)
{
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    DpgServerResults res;
    mco_cursor_t csr;

    if ((DpgServerResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (DpgServerResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (DpgServerResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not delete DpgServerResults object with handle " << handle);
        return;
    }

    uint32_t index;
    if (DpgServerResults_BlockIndex_get(&res, &index) != MCO_S_OK)
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not get DpgServerResults index value for handle " << handle);
        return;
    }

    // Delete the row
    DpgServerResults_delete(&res);

    // Commit the deletion from the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of DpgServerResults object for handle " << handle);
        return;
    }

    // Return now unused server index
    mServerIndexTracker->Release(index);
}

///////////////////////////////////////////////////////////////////////////////
