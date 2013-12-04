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

#include "FtpCommon.h"

// Forward declarations (global)
class ExtremeStatsDBFactory;

///////////////////////////////////////////////////////////////////////////////

DECL_FTP_NS

// Forward declarations
class FtpClientRawStats;
class FtpServerRawStats;

class McoDriver
{
  public:
    static void StaticInitialize(void);

    explicit McoDriver(uint16_t port);
    ~McoDriver();
    
    // FtpClientResults table related methods
    void InsertFtpClientResult(uint32_t handle);
    void UpdateFtpClientResult(FtpClientRawStats& stats);
    void DeleteFtpClientResult(uint32_t handle);

    // FtpServerResults table related methods
    void InsertFtpServerResult(uint32_t handle);
    void UpdateFtpServerResult(FtpServerRawStats& stats);
    void DeleteFtpServerResult(uint32_t handle);
    
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

END_DECL_FTP_NS

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// mode:c++
// End:

#endif
