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

#include "XmppCommon.h"

// Forward declarations (global)
class ExtremeStatsDBFactory;
class UniqueIndexTracker;

///////////////////////////////////////////////////////////////////////////////

DECL_XMPP_NS

// Forward declarations
class XmppClientRawStats;

class McoDriver
{
  public:
    static void StaticInitialize(void);
    
    explicit McoDriver(uint16_t port);
    ~McoDriver();
    
    // ClientResults table related methods
    void InsertXmppClientResult(uint32_t handle);
    void UpdateXmppClientResult(XmppClientRawStats& stats);
    void DeleteXmppClientResult(uint32_t handle);
    
  private:
    const uint16_t mPort;                                               ///< physical port number
    boost::scoped_ptr<UniqueIndexTracker> mClientIndexTracker;          ///< tracker for unique ClientIndex values
    ExtremeStatsDBFactory *mDBFactory;                                  ///< ExtremeStatsDB factory object
};

END_DECL_XMPP_NS

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// mode:c++
// End:

#endif
