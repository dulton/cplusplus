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
#include <hal/TimeStamp.h>
#include <phxexception/PHXException.h>
#include <statsframework/ExtremeStatsDBFactory.h>

#include "UniqueIndexTracker.h"
#include "XmppClientRawStats.h"
#include "XmppdLog.h"
#include "McoDriver.h"
#include "statsdb.h"

USING_XMPP_NS;

   
///////////////////////////////////////////////////////////////////////////////

void McoDriver::StaticInitialize(void)
{
    // Configure the stats db factory parameters before it instantiates any databases
    ExtremeStatsDBFactory::setDictionary(statsdb_get_dictionary());  // required
}

///////////////////////////////////////////////////////////////////////////////

McoDriver::McoDriver(uint16_t port)
    : mPort(port),
      mClientIndexTracker(new UniqueIndexTracker)
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

void McoDriver::InsertXmppClientResult(uint32_t handle)
{
    // Select a new index value for this handle
    uint32_t index = mClientIndexTracker->Assign();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Insert a new row in the table
    ClientResults res;

    // Create a new ClientResults object
    if (ClientResults_new(t, &res) != MCO_S_OK)
    {
        // Rollback transaction
        mco_trans_rollback(t);

        // Return unused client index
        mClientIndexTracker->Release(index);
        
        TC_LOG_ERR(mPort, "Could not create a ClientResults object");
        throw EPHXInternal();
    }

    // Initialize the results fields
    ClientResults_BlockIndex_put(&res, index);
    ClientResults_Handle_put(&res, handle);
    ClientResults_IntendedLoad_put(&res, 0);
    ClientResults_ActiveConnections_put(&res, 0);
    ClientResults_AttemptedConnections_put(&res, 0);
    ClientResults_SuccessfulConnections_put(&res, 0);
    ClientResults_UnsuccessfulConnections_put(&res, 0);
    ClientResults_AbortedConnections_put(&res, 0);
    ClientResults_TotalClientCount_put(&res, 0);
    ClientResults_IntendedRegLoad_put(&res, 0);
    ClientResults_RegAttempts_put(&res, 0);
    ClientResults_RegSuccesses_put(&res, 0);
    ClientResults_RegFailures_put(&res, 0);
    ClientResults_LoginAttempts_put(&res, 0);
    ClientResults_LoginSuccesses_put(&res, 0);
    ClientResults_LoginFailures_put(&res, 0);
    ClientResults_PubAttempts_put(&res, 0);
    ClientResults_PubSuccesses_put(&res, 0);
    ClientResults_PubFailures_put(&res, 0);
    ClientResults_ServerResponseMinTimeMsec_put(&res, 0);
    ClientResults_ServerResponseMaxTimeMsec_put(&res, 0);
    ClientResults_ServerResponseCumulativeTimeMsec_put(&res, 0);
    ClientResults_ServerResponseAvgTimeMsec_put(&res, 0);
    ClientResults_RxResponseCode302_put(&res,0);
    ClientResults_RxResponseCode400_put(&res,0);
    ClientResults_RxResponseCode401_put(&res,0);
    ClientResults_RxResponseCode402_put(&res,0);
    ClientResults_RxResponseCode403_put(&res,0);
    ClientResults_RxResponseCode404_put(&res,0);
    ClientResults_RxResponseCode405_put(&res,0);
    ClientResults_RxResponseCode406_put(&res,0);
    ClientResults_RxResponseCode407_put(&res,0);
    ClientResults_RxResponseCode409_put(&res,0);
    ClientResults_RxResponseCode500_put(&res,0);
    ClientResults_RxResponseCode501_put(&res,0);
    ClientResults_RxResponseCode503_put(&res,0);
    ClientResults_RxResponseCode504_put(&res,0);
    ClientResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    
    // Commit insert into the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        mClientIndexTracker->Release(index);
        
        TC_LOG_ERR(mPort, "Could not commit insertion of ClientResults object");
        throw EPHXInternal();
    }

    TC_LOG_INFO(mPort, "Created ClientResults object with index " << index << " for handle " << handle);
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::UpdateXmppClientResult(XmppClientRawStats& stats)
{
    const uint32_t handle = stats.Handle();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    ClientResults res;
    mco_cursor_t csr;

    if ((ClientResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (ClientResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (ClientResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not update ClientResults object with handle " << handle);
        return;
    }

    // Update the results fields
    {
        ACE_GUARD(XmppClientRawStats::lock_t, guard, stats.Lock());

        ClientResults_IntendedLoad_put(&res, stats.intendedLoad);
        ClientResults_ActiveConnections_put(&res, stats.activeConnections);
        ClientResults_AttemptedConnections_put(&res, stats.attemptedConnections);
        ClientResults_SuccessfulConnections_put(&res, stats.successfulConnections);
        ClientResults_UnsuccessfulConnections_put(&res, stats.unsuccessfulConnections);
        ClientResults_AbortedConnections_put(&res, stats.abortedConnections);
        ClientResults_TotalClientCount_put(&res, stats.totalClientCount);
        ClientResults_IntendedRegLoad_put(&res, stats.intendedRegistrationLoad);
        ClientResults_RegAttempts_put(&res, stats.registrationAttemptCount);
        ClientResults_RegSuccesses_put(&res, stats.registrationSuccessCount);
        ClientResults_RegFailures_put(&res, stats.registrationFailureCount);
        ClientResults_LoginAttempts_put(&res, stats.loginAttemptCount);
        ClientResults_LoginSuccesses_put(&res, stats.loginSuccessCount);
        ClientResults_LoginFailures_put(&res, stats.loginFailureCount);
        ClientResults_PubAttempts_put(&res, stats.pubAttemptCount);
        ClientResults_PubSuccesses_put(&res, stats.pubSuccessCount);
        ClientResults_PubFailures_put(&res, stats.pubFailureCount);
        ClientResults_ServerResponseMinTimeMsec_put(&res, stats.serverResponseMinTime);
        ClientResults_ServerResponseMaxTimeMsec_put(&res, stats.serverResponseMaxTime);
        ClientResults_ServerResponseCumulativeTimeMsec_put(&res, stats.serverResponseCumulativeTime);
        ClientResults_ServerResponseAvgTimeMsec_put(&res, stats.serverResponseAvgTime);
        ClientResults_RxResponseCode302_put(&res, stats.rxResponseCode302);
        ClientResults_RxResponseCode400_put(&res, stats.rxResponseCode400);
        ClientResults_RxResponseCode401_put(&res, stats.rxResponseCode401);
        ClientResults_RxResponseCode402_put(&res, stats.rxResponseCode402);
        ClientResults_RxResponseCode403_put(&res, stats.rxResponseCode403);
        ClientResults_RxResponseCode404_put(&res, stats.rxResponseCode404);
        ClientResults_RxResponseCode405_put(&res, stats.rxResponseCode405);
        ClientResults_RxResponseCode406_put(&res, stats.rxResponseCode406);
        ClientResults_RxResponseCode407_put(&res, stats.rxResponseCode407);
        ClientResults_RxResponseCode409_put(&res, stats.rxResponseCode409);
        ClientResults_RxResponseCode500_put(&res, stats.rxResponseCode500);
        ClientResults_RxResponseCode501_put(&res, stats.rxResponseCode501);
        ClientResults_RxResponseCode503_put(&res, stats.rxResponseCode503);
        ClientResults_RxResponseCode504_put(&res, stats.rxResponseCode504);

        ClientResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    }
    
    // Commit the update to the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of ClientResults object for handle " << handle);
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::DeleteXmppClientResult(uint32_t handle)
{
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Lookup the row in the table
    ClientResults res;
    mco_cursor_t csr;

    if ((ClientResults_HandleIndex_index_cursor(t, &csr) != MCO_S_OK) ||
        (ClientResults_HandleIndex_search(t, &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (ClientResults_from_cursor(t, &csr, &res) != MCO_S_OK))
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not delete ClientResults object with handle " << handle);
        return;
    }

    uint32_t index;
    if (ClientResults_BlockIndex_get(&res, &index) != MCO_S_OK)
    {
        mco_trans_rollback(t);
        TC_LOG_ERR(mPort, "Could not get ClientResults index value for handle " << handle);
        return;
    }

    // Delete the row
    ClientResults_delete(&res);

    // Commit the deletion from the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "Could not commit deletion of ClientResults object for handle " << handle);
        return;
    }

    // Return now unused client index
    mClientIndexTracker->Release(index);
}

///////////////////////////////////////////////////////////////////////////////
