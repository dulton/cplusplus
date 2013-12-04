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

#include "RawTcpCommon.h"

// Forward declarations (global)
class ExtremeStatsDBFactory;

///////////////////////////////////////////////////////////////////////////////

DECL_RAWTCP_NS

// Forward declarations
class RawTcpClientRawStats;
class RawTcpServerRawStats;

class McoDriver
{
  public:
    static void StaticInitialize(void);

    explicit McoDriver(uint16_t port);
    ~McoDriver();

    // RawTcpClientResults table related methods
    void InsertRawTcpClientResult(uint32_t handle);
    void UpdateRawTcpClientResult(RawTcpClientRawStats& stats);
    void DeleteRawTcpClientResult(uint32_t handle);

    // RawTcpServerResults table related methods
    void InsertRawTcpServerResult(uint32_t handle);
    void UpdateRawTcpServerResult(RawTcpServerRawStats& stats);
    void DeleteRawTcpServerResult(uint32_t handle);

  private:
    // Implementation-private inner class
    class UniqueIndexTracker;

    static const size_t MAX_CLIENT_BLOCKS = 32767;
    static const size_t MAX_SERVER_BLOCKS = 32767;

    const uint16_t mPort;                                               ///< physical port number

    boost::scoped_ptr<UniqueIndexTracker> mClientIndexTracker;          ///< tracker for unique ClientIndex values
    boost::scoped_ptr<UniqueIndexTracker> mServerIndexTracker;          ///< tracker for unique ServerIndex values

    ExtremeStatsDBFactory *mDBFactory;                                  ///< ExtremeStatsDB factory object
};

END_DECL_RAWTCP_NS

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// mode:c++
// End:

#endif
