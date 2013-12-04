/// @file
/// @brief Raw tcp Client Block Load Strategies header file 
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _RAWTCP_CLIENT_BLOCK_LOAD_STRATEGIES_H_
#define _RAWTCP_CLIENT_BLOCK_LOAD_STRATEGIES_H_

#include <base/RateBasedLoadStrategy.h>
#include <base/StaticLoadStrategy.h>

#include "RawTcpClientBlock.h"

DECL_RAWTCP_NS

///////////////////////////////////////////////////////////////////////////////

class RawTcpClientBlock::LoadStrategy : virtual public L4L7_BASE_NS::LoadStrategy
{
  public:
    LoadStrategy(RawTcpClientBlock& clientBlock)
        : mClientBlock(clientBlock)
    {
    }

    // Connection events
    virtual void ConnectionClosed(void) { }
    
  protected:
    RawTcpClientBlock& mClientBlock;
};

///////////////////////////////////////////////////////////////////////////////

class RawTcpClientBlock::ConnectionsLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::StaticLoadStrategy
{
  public:
    ConnectionsLoadStrategy(RawTcpClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void ConnectionClosed(void);
    
  private:
    void LoadChangeHook(int32_t load);
};

///////////////////////////////////////////////////////////////////////////////

class RawTcpClientBlock::ConnectionsOverTimeLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::RateBasedLoadStrategy
{
  public:
    ConnectionsOverTimeLoadStrategy(RawTcpClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void ConnectionClosed(void);
    
  private:
    void LoadChangeHook(double load);
};

///////////////////////////////////////////////////////////////////////////////

class RawTcpClientBlock::TransactionsLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::StaticLoadStrategy
{
  public:
    TransactionsLoadStrategy(RawTcpClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void ConnectionClosed(void);
    
  private:
    void LoadChangeHook(int32_t load);
};

///////////////////////////////////////////////////////////////////////////////

class RawTcpClientBlock::TransactionsOverTimeLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::RateBasedLoadStrategy
{
  public:
    TransactionsOverTimeLoadStrategy(RawTcpClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void ConnectionClosed(void);
    
  private:
    void LoadChangeHook(double load);
};

///////////////////////////////////////////////////////////////////////////////

class RawTcpClientBlock::BandwidthLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::StaticLoadStrategy
{
  public:
    BandwidthLoadStrategy(RawTcpClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void ConnectionClosed(void);

  private:
    void LoadChangeHook(int32_t load);
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_RAWTCP_NS

#endif
