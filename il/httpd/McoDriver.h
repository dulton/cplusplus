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

#include "HttpCommon.h"

// Forward declarations (global)
class ExtremeStatsDBFactory;

///////////////////////////////////////////////////////////////////////////////

DECL_HTTP_NS

// Forward declarations
class HttpClientRawStats;
class HttpServerRawStats;
class HttpVideoClientRawStats;
class HttpVideoServerRawStats;

class McoDriver
{
  public:
    static void StaticInitialize(void);
    
    explicit McoDriver(uint16_t port);
    ~McoDriver();
    
    // HttpClientResults table related methods
    void InsertHttpClientResult(uint32_t handle);
    void UpdateHttpClientResult(HttpClientRawStats& stats);
    void DeleteHttpClientResult(uint32_t handle);

    // HttpServerResults table related methods
    void InsertHttpServerResult(uint32_t handle);
    void UpdateHttpServerResult(HttpServerRawStats& stats);
    void DeleteHttpServerResult(uint32_t handle);
    
    // HttpClientResults table related methods
    void InsertHttpVideoClientResult(uint32_t handle);
    void UpdateHttpVideoClientResult(HttpVideoClientRawStats& stats);
    void DeleteHttpVideoClientResult(uint32_t handle);

    // HttpServerResults table related methods
    void InsertHttpVideoServerResult(uint32_t handle);
    void UpdateHttpVideoServerResult(HttpVideoServerRawStats& stats);
    void DeleteHttpVideoServerResult(uint32_t handle);

  private:
    // Implementation-private inner class
    class UniqueIndexTracker;
    
    static const size_t MAX_CLIENT_BLOCKS = 32767;
    static const size_t MAX_SERVER_BLOCKS = 32767;
    
    const uint16_t mPort;                                               ///< physical port number
    
    boost::scoped_ptr<UniqueIndexTracker> mClientIndexTracker;          ///< tracker for unique ClientIndex values
    boost::scoped_ptr<UniqueIndexTracker> mServerIndexTracker;          ///< tracker for unique ServerIndex values
    boost::scoped_ptr<UniqueIndexTracker> mVideoClientIndexTracker;          ///< tracker for unique video ClientIndex values
    boost::scoped_ptr<UniqueIndexTracker> mVideoServerIndexTracker;          ///< tracker for unique video ServerIndex values
    
    ExtremeStatsDBFactory *mDBFactory;                                  ///< ExtremeStatsDB factory object
};

END_DECL_HTTP_NS

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// mode:c++
// End:

#endif
