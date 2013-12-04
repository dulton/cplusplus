/// @file
/// @brief SIP User Agent Block Load Strategies header file 
///
///  Copyright (c) 2008 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _SIP_USER_AGENT_BLOCK_LOAD_STRATEGIES_H_
#define _SIP_USER_AGENT_BLOCK_LOAD_STRATEGIES_H_

#include <base/RateBasedLoadStrategy.h>
#include <base/StaticLoadStrategy.h>

#include "UserAgentBlock.h"

DECL_APP_NS

///////////////////////////////////////////////////////////////////////////////

class UserAgentBlock::RegistrationLoadStrategy : public L4L7_BASE_NS::RateBasedLoadStrategy
{
  public:
    RegistrationLoadStrategy(UserAgentBlock& uaBlock)
        : mUaBlock(uaBlock)
    {
    }

    // Registration events
    void RegistrationCompleted(void);
    
  private:
    void LoadChangeHook(double load);

    UserAgentBlock& mUaBlock;
};

///////////////////////////////////////////////////////////////////////////////

class UserAgentBlock::ProtocolLoadStrategy : virtual public L4L7_BASE_NS::LoadStrategy
{
  public:
    ProtocolLoadStrategy(UserAgentBlock& uaBlock)
        : mUaBlock(uaBlock)
    {
    }

    // Protocol events
    virtual void CallCompleted(void) { }
    
  protected:
    UserAgentBlock& mUaBlock;
};

///////////////////////////////////////////////////////////////////////////////

class UserAgentBlock::CallsLoadStrategy : public ProtocolLoadStrategy, public L4L7_BASE_NS::StaticLoadStrategy
{
  public:
    CallsLoadStrategy(UserAgentBlock& uaBlock)
        : ProtocolLoadStrategy(uaBlock)
    {
    }

    void CallCompleted(void);
    
  private:
    void LoadChangeHook(int32_t load);
};

///////////////////////////////////////////////////////////////////////////////

class UserAgentBlock::CallsOverTimeLoadStrategy : public ProtocolLoadStrategy, public L4L7_BASE_NS::RateBasedLoadStrategy
{
  public:
    CallsOverTimeLoadStrategy(UserAgentBlock& uaBlock)
        : ProtocolLoadStrategy(uaBlock)
    {
    }

    void CallCompleted(void);
    
  private:
    void LoadChangeHook(double load);
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_APP_NS

#endif
