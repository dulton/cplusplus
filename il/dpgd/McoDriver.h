/// @file
/// @brief McObjects (MCO) embedded database header file
///
///  Copyright (c) 2009 by Spirent Communications Inc.
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

#include "DpgCommon.h"

// Forward declarations (global)
class ExtremeStatsDBFactory;
class TestMcoDriver;

///////////////////////////////////////////////////////////////////////////////

DECL_DPG_NS

// Forward declarations
class DpgClientRawStats;
class DpgServerRawStats;

class McoDriver
{
  public:
    static void StaticInitialize(void);

    explicit McoDriver(uint16_t port);
    ~McoDriver();

    // DpgClientResults table related methods
    void InsertDpgClientResult(uint32_t handle);
    void UpdateDpgClientResult(DpgClientRawStats& stats);
    void DeleteDpgClientResult(uint32_t handle);

    // DpgServerResults table related methods
    void InsertDpgServerResult(uint32_t handle);
    void UpdateDpgServerResult(DpgServerRawStats& stats);
    void DeleteDpgServerResult(uint32_t handle);

  private:
    // Implementation-private inner class
    class UniqueIndexTracker;

    static const size_t MAX_CLIENT_BLOCKS = 32767;
    static const size_t MAX_SERVER_BLOCKS = 32767;

    const uint16_t mPort;                                               ///< physical port number

    boost::scoped_ptr<UniqueIndexTracker> mClientIndexTracker;          ///< tracker for unique ClientIndex values
    boost::scoped_ptr<UniqueIndexTracker> mServerIndexTracker;          ///< tracker for unique ServerIndex values

    ExtremeStatsDBFactory *mDBFactory;                                  ///< ExtremeStatsDB factory object

    friend class ::TestMcoDriver;
};

END_DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// mode:c++
// End:

#endif
