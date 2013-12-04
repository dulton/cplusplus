/// @file
/// @brief XMPP Client Raw Statistics header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _XMPP_CLIENT_BLOCK_RAW_STATS_H_
#define _XMPP_CLIENT_BLOCK_RAW_STATS_H_

#include <utility>

#include <ace/Guard_T.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Time_Value.h>

#include "XmppCommon.h"

DECL_XMPP_NS

///////////////////////////////////////////////////////////////////////////////

class XmppClientRawStats
{
  public:
    typedef ACE_Recursive_Thread_Mutex lock_t;

    XmppClientRawStats(uint32_t handle)
        : intendedLoad(0),
          activeConnections(0),
          totalClientCount(0),
          intendedRegistrationLoad(0),
          mHandle(handle)
    {
        ResetInternal();
    }

#ifdef UNIT_TEST    
    XmppClientRawStats& operator=(const XmppClientRawStats& other)
    {
        intendedLoad = other.intendedLoad;
        activeConnections = other.activeConnections;
        attemptedConnections = other.attemptedConnections;
        successfulConnections = other.successfulConnections;
        unsuccessfulConnections = other.unsuccessfulConnections;
        abortedConnections = other.abortedConnections;
        totalClientCount = other.totalClientCount;
        intendedRegistrationLoad = other.intendedRegistrationLoad;
        registrationAttemptCount = other.registrationAttemptCount;
        registrationSuccessCount = other.registrationSuccessCount;
        registrationFailureCount = other.registrationFailureCount;
        loginAttemptCount = other.loginAttemptCount;
        loginSuccessCount = other.loginSuccessCount;
        loginFailureCount = other.loginFailureCount;
        pubAttemptCount = other.pubAttemptCount;
        pubSuccessCount = other.pubSuccessCount;
        pubFailureCount = other.pubFailureCount;
        serverResponseMinTime = other.serverResponseMinTime;
        serverResponseMaxTime = other.serverResponseMaxTime;
        serverResponseCumulativeTime = other.serverResponseCumulativeTime;
        serverResponseAvgTime = other.serverResponseAvgTime;
        rxResponseCode302 = other.rxResponseCode302;
        rxResponseCode400 = other.rxResponseCode400;
        rxResponseCode401 = other.rxResponseCode401;
        rxResponseCode402 = other.rxResponseCode402;
        rxResponseCode403 = other.rxResponseCode403;
        rxResponseCode404 = other.rxResponseCode404;
        rxResponseCode405 = other.rxResponseCode405;
        rxResponseCode406 = other.rxResponseCode406;
        rxResponseCode407 = other.rxResponseCode407;
        rxResponseCode409 = other.rxResponseCode409;
        rxResponseCode500 = other.rxResponseCode500;
        rxResponseCode501 = other.rxResponseCode501;
        rxResponseCode503 = other.rxResponseCode503;
        rxResponseCode504 = other.rxResponseCode504;
        mHandle = other.mHandle;
        return *this;
    }

    XmppClientRawStats(const XmppClientRawStats& other) { *this = other; }
#endif
    
    /// @brief Block handle accessor
    const uint32_t Handle(void) const { return mHandle; }
    
    /// @brief Lock accessor
    lock_t& Lock(void) { return mLock; }

    /// @brief Reset sets
    /// @note Clears statistics counters
    void Reset(void)
    {
        ACE_GUARD(lock_t, guard, mLock);
        ResetInternal();
    }

    /// @brief Synchronize stats
    /// @note Recalculates averages
    void Sync(void)
    {
      serverResponseAvgTime = registrationSuccessCount ? static_cast<uint64_t>((static_cast<double>(serverResponseCumulativeTime) / registrationSuccessCount)) : 0;
    }

    /// @note Public data members, must be accessed with lock held
    uint32_t intendedLoad;
    uint32_t activeConnections;
    uint64_t attemptedConnections;
    uint64_t successfulConnections;
    uint64_t unsuccessfulConnections;
    uint64_t abortedConnections;
    
    uint32_t totalClientCount;

    uint32_t intendedRegistrationLoad;
    uint64_t registrationAttemptCount;
    uint64_t registrationSuccessCount;
    uint64_t registrationFailureCount;

    uint64_t loginAttemptCount;
    uint64_t loginSuccessCount;
    uint64_t loginFailureCount;

    uint64_t pubAttemptCount;
    uint64_t pubSuccessCount;
    uint64_t pubFailureCount;

    uint64_t serverResponseMinTime;
    uint64_t serverResponseMaxTime;
    uint64_t serverResponseCumulativeTime;
    uint64_t serverResponseAvgTime;

    uint64_t rxResponseCode302;
    uint64_t rxResponseCode400;
    uint64_t rxResponseCode401;
    uint64_t rxResponseCode402;
    uint64_t rxResponseCode403;
    uint64_t rxResponseCode404;
    uint64_t rxResponseCode405;
    uint64_t rxResponseCode406;
    uint64_t rxResponseCode407;
    uint64_t rxResponseCode409;
    uint64_t rxResponseCode500;
    uint64_t rxResponseCode501;
    uint64_t rxResponseCode503;    
    uint64_t rxResponseCode504;

  private:
    void ResetInternal(void)
    {
        attemptedConnections = 0;
        successfulConnections = 0;
        unsuccessfulConnections = 0;
        abortedConnections = 0;
        
        registrationAttemptCount = 0;
        registrationSuccessCount = 0;
        registrationFailureCount = 0;

        loginAttemptCount = 0;
        loginSuccessCount = 0;
        loginFailureCount = 0;

        pubAttemptCount = 0;
        pubSuccessCount = 0;
        pubFailureCount = 0;

        serverResponseMinTime = 0;
        serverResponseMaxTime = 0;
        serverResponseCumulativeTime = 0;
        serverResponseAvgTime = 0;

        rxResponseCode302 = 0;
        rxResponseCode400 = 0;
        rxResponseCode401 = 0;
        rxResponseCode402 = 0;
        rxResponseCode403 = 0;
        rxResponseCode404 = 0;
        rxResponseCode405 = 0;
        rxResponseCode406 = 0;
        rxResponseCode407 = 0;
        rxResponseCode409 = 0;
        rxResponseCode500 = 0;
        rxResponseCode501 = 0;
        rxResponseCode503 = 0; 
        rxResponseCode504 = 0;

    }
            
    uint32_t mHandle;                       ///< block handle
    lock_t mLock;                           ///< protects against concurrent access
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_XMPP_NS

#endif
