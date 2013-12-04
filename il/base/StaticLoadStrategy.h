/// @file
/// @brief Static Load Strategy header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_STATIC_LOAD_STRATEGY_H_
#define _L4L7_STATIC_LOAD_STRATEGY_H_

#include <base/LoadStrategy.h>

DECL_L4L7_BASE_NS

///////////////////////////////////////////////////////////////////////////////

class StaticLoadStrategy : virtual public LoadStrategy
{
  public:
    StaticLoadStrategy(void);
    
    /// @brief Start the load strategy (from the LoadStrategy interface)
    void Start(ACE_Reactor *reactor, uint32_t hz = 0);

    /// @brief Stop the load strategy (from the LoadStrategy interface)
    void Stop(void);
    
    /// @brief Return the maximum load
    const int32_t GetLoad(void) const { return mLoad; }
    
  private:
    /// @brief Set the instantaneous load (from the LoadStrategy interface)
    void SetLoad(int32_t load);

    /// @note Concrete strategy classes can override
    virtual void LoadChangeHook(int32_t load) { }
    
    int32_t mLoad;      ///< current load value
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_BASE_NS

#endif
