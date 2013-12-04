/// @file
/// @brief Load Strategy header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_LOAD_STRATEGY_H_
#define _L4L7_LOAD_STRATEGY_H_

#include <ace/Event_Handler.h>
#include <base/BaseCommon.h>
#include <boost/utility.hpp>

// Forward declarations (global)
class ACE_Reactor;

DECL_L4L7_BASE_NS

///////////////////////////////////////////////////////////////////////////////

class LoadStrategy : public ACE_Event_Handler, boost::noncopyable
{
  public:
    virtual ~LoadStrategy() { }

    /// @brief Start the load strategy
    virtual void Start(ACE_Reactor *reactor, uint32_t hz = 0) = 0;

    /// @brief Stop the load strategy
    virtual void Stop(void) = 0;
    
    /// @brief Set the instantaneous load
    virtual void SetLoad(int32_t load) = 0;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_BASE_NS

#endif
