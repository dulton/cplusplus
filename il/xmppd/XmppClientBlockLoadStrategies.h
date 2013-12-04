/// @file
/// @brief XMPP Client Block Load Strategies header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _XMPP_CLIENT_BLOCK_LOAD_STRATEGIES_H_
#define _XMPP_CLIENT_BLOCK_LOAD_STRATEGIES_H_

#include <base/RateBasedLoadStrategy.h>
#include <base/StaticLoadStrategy.h>

#include "XmppClientBlock.h"

DECL_XMPP_NS

///////////////////////////////////////////////////////////////////////////////

class XmppClientBlock::RegistrationLoadStrategy : public L4L7_BASE_NS::RateBasedLoadStrategy
{
  public:
    RegistrationLoadStrategy(XmppClientBlock& clientBlock)
        : mClientBlock(clientBlock)
    {
    }

    // Registration events
    void RegistrationCompleted(void);

  private:
    void LoadChangeHook(double load);

    XmppClientBlock& mClientBlock;
};

///////////////////////////////////////////////////////////////////////////////

class XmppClientBlock::LoadStrategy : virtual public L4L7_BASE_NS::LoadStrategy
{
  public:
    LoadStrategy(XmppClientBlock& clientBlock)
        : mClientBlock(clientBlock)
    {
    }

    // Connection events
    virtual void ConnectionClosed(void) { }
    
  protected:
    XmppClientBlock& mClientBlock;
};

///////////////////////////////////////////////////////////////////////////////

class XmppClientBlock::ConnectionsLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::StaticLoadStrategy
{
  public:
    ConnectionsLoadStrategy(XmppClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void ConnectionClosed(void);
    
  private:
    void LoadChangeHook(int32_t load);
};

///////////////////////////////////////////////////////////////////////////////

class XmppClientBlock::ConnectionsOverTimeLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::RateBasedLoadStrategy
{
  public:
    ConnectionsOverTimeLoadStrategy(XmppClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void ConnectionClosed(void);
    
  private:
    void LoadChangeHook(double load);
};

///////////////////////////////////////////////////////////////////////////////
END_DECL_XMPP_NS

#endif
