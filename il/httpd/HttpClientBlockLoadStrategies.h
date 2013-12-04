/// @file
/// @brief HTTP Client Block Load Strategies header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _HTTP_CLIENT_BLOCK_LOAD_STRATEGIES_H_
#define _HTTP_CLIENT_BLOCK_LOAD_STRATEGIES_H_

#include <base/RateBasedLoadStrategy.h>
#include <base/StaticLoadStrategy.h>

#include "HttpClientBlock.h"

DECL_HTTP_NS

///////////////////////////////////////////////////////////////////////////////

class HttpClientBlock::LoadStrategy : virtual public L4L7_BASE_NS::LoadStrategy
{
  public:
    LoadStrategy(HttpClientBlock& clientBlock)
        : mClientBlock(clientBlock)
    {
    }

    // Connection events
    virtual void ConnectionClosed(void) { }
    
  protected:
    HttpClientBlock& mClientBlock;
};

///////////////////////////////////////////////////////////////////////////////

class HttpClientBlock::ConnectionsLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::StaticLoadStrategy
{
  public:
    ConnectionsLoadStrategy(HttpClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void ConnectionClosed(void);
    
  private:
    void LoadChangeHook(int32_t load);
};

///////////////////////////////////////////////////////////////////////////////

class HttpClientBlock::ConnectionsOverTimeLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::RateBasedLoadStrategy
{
  public:
    ConnectionsOverTimeLoadStrategy(HttpClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void ConnectionClosed(void);
    
  private:
    void LoadChangeHook(double load);
};

///////////////////////////////////////////////////////////////////////////////

class HttpClientBlock::TransactionsLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::StaticLoadStrategy
{
  public:
    TransactionsLoadStrategy(HttpClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void ConnectionClosed(void);
    
  private:
    void LoadChangeHook(int32_t load);
};

///////////////////////////////////////////////////////////////////////////////

class HttpClientBlock::TransactionsOverTimeLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::RateBasedLoadStrategy
{
  public:
    TransactionsOverTimeLoadStrategy(HttpClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void ConnectionClosed(void);
    
  private:
    void LoadChangeHook(double load);
};

///////////////////////////////////////////////////////////////////////////////

class HttpClientBlock::BandwidthLoadStrategy : public LoadStrategy, public
L4L7_BASE_NS::StaticLoadStrategy
{
  public:
    BandwidthLoadStrategy(HttpClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void ConnectionClosed(void);

  private:
    void LoadChangeHook(int32_t load);
};

///////////////////////////////////////////////////////////////////////////////
END_DECL_HTTP_NS

#endif
