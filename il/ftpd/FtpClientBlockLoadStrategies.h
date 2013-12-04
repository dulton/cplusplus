/// @file
/// @brief FTP Client Block Load Strategies header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _FTP_CLIENT_BLOCK_LOAD_STRATEGIES_H_
#define _FTP_CLIENT_BLOCK_LOAD_STRATEGIES_H_

#include <base/RateBasedLoadStrategy.h>
#include <base/StaticLoadStrategy.h>

#include "FtpClientBlock.h"

DECL_FTP_NS

///////////////////////////////////////////////////////////////////////////////

class FtpClientBlock::LoadStrategy : virtual public L4L7_BASE_NS::LoadStrategy
{
  public:
    LoadStrategy(FtpClientBlock& clientBlock)
        : mClientBlock(clientBlock)
    {
    }

    // Connection events
    virtual void ConnectionClosed(void) { }
    
  protected:
    FtpClientBlock& mClientBlock;
};

///////////////////////////////////////////////////////////////////////////////

class FtpClientBlock::ConnectionsLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::StaticLoadStrategy
{
  public:
    ConnectionsLoadStrategy(FtpClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void ConnectionClosed(void);
    
  private:
    void LoadChangeHook(int32_t load);
};

///////////////////////////////////////////////////////////////////////////////

class FtpClientBlock::ConnectionsOverTimeLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::RateBasedLoadStrategy
{
  public:
    ConnectionsOverTimeLoadStrategy(FtpClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void ConnectionClosed(void);
    
  private:
    void LoadChangeHook(double load);
};

///////////////////////////////////////////////////////////////////////////////

class FtpClientBlock::TransactionsLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::StaticLoadStrategy
{
  public:
    TransactionsLoadStrategy(FtpClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void ConnectionClosed(void);
    
  private:
    void LoadChangeHook(int32_t load);
};

///////////////////////////////////////////////////////////////////////////////

class FtpClientBlock::TransactionsOverTimeLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::RateBasedLoadStrategy
{
  public:
    TransactionsOverTimeLoadStrategy(FtpClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void ConnectionClosed(void);
    
  private:
    void LoadChangeHook(double load);
};

///////////////////////////////////////////////////////////////////////////////

class FtpClientBlock::BandwidthLoadStrategy : public LoadStrategy, public
L4L7_BASE_NS::StaticLoadStrategy
{
  public:
    BandwidthLoadStrategy(FtpClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void ConnectionClosed(void);

  private:
    void LoadChangeHook(int32_t load);
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_FTP_NS

#endif
