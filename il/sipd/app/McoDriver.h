/// @file
/// @brief McObjects (MCO) embedded database header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _MCO_DRIVER_H_
#define _MCO_DRIVER_H_

#include <boost/scoped_ptr.hpp>
#include <Tr1Adapter.h>

#include "VoIPCommon.h"

// Forward declarations (global)
class ExtremeStatsDBFactory;

///////////////////////////////////////////////////////////////////////////////

DECL_APP_NS

// Forward declarations
class UserAgentRawStats;

class McoDriver
{
  private:
    // Implementation-private inner classes
    class UniqueIndexTracker;
    class TransactionImpl;
    
  public:
    typedef std::tr1::shared_ptr<TransactionImpl> Transaction;

    static void StaticInitialize(void);

    explicit McoDriver(uint16_t port);
    ~McoDriver();

    // Transaction related methods
    Transaction StartTransaction(void);
    void CommitTransaction(Transaction t);
    void RollbackTransaction(Transaction t);
    
    // SipUaResults table related methods
    void InsertSipUaResult(uint32_t handle, uint64_t totalUaCount);

    bool UpdateSipUaResult(UserAgentRawStats& stats, Transaction& t);
    bool UpdateSipUaResult(UserAgentRawStats& stats);

    bool DeleteSipUaResult(uint32_t handle, Transaction& t);
    bool DeleteSipUaResult(uint32_t handle);

  private:
    static const size_t MAX_UA_BLOCKS = 32767;
    
    const uint16_t mPort;                                       ///< physical port number
    
    boost::scoped_ptr<UniqueIndexTracker> mSipUaIndexTracker;   ///< tracker for unique BlockIndex values
    
    ExtremeStatsDBFactory *mDBFactory;                          ///< ExtremeStatsDB factory object
};

END_DECL_APP_NS

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// mode:c++
// End:

#endif
