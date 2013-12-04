/// @file
/// @brief HTTP Client Block Load Strategies implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include "HttpClientBlockLoadStrategies.h"
#include "HttpdLog.h"

USING_HTTP_NS;

///////////////////////////////////////////////////////////////////////////////

void HttpClientBlock::ConnectionsLoadStrategy::LoadChangeHook(int32_t load)
{
    TC_LOG_INFO_LOCAL(mClientBlock.mPort, LOG_CLIENT, "HTTP client block [" << mClientBlock.Name() << "] connections load changed to " << load);

    mClientBlock.SetIntendedLoad(static_cast<uint32_t>(load));

    // Update the intended load value in the block stats
    ACE_GUARD(stats_t::lock_t, guard, mClientBlock.mStats.Lock());
    mClientBlock.mStats.intendedLoad = static_cast<uint32_t>(load);
}

void HttpClientBlock::ConnectionsLoadStrategy::ConnectionClosed(void)
{
    mClientBlock.SetIntendedLoad(static_cast<uint32_t>(GetLoad()));
}

///////////////////////////////////////////////////////////////////////////////

void HttpClientBlock::ConnectionsOverTimeLoadStrategy::LoadChangeHook(double load)
{
    TC_LOG_INFO_LOCAL(mClientBlock.mPort, LOG_CLIENT, "HTTP client block [" << mClientBlock.Name() << "] connections over time load changed to " << load);

    const size_t delta = static_cast<size_t>(load);
    if (delta && ConsumeLoad(static_cast<double>(delta)))
        mClientBlock.SetAvailableLoad(delta, static_cast<uint32_t>(GetLoad()));

    // Update the intended load value in the block stats
    ACE_GUARD(stats_t::lock_t, guard, mClientBlock.mStats.Lock());
    mClientBlock.mStats.intendedLoad = static_cast<uint32_t>(GetLoad());
}

void HttpClientBlock::ConnectionsOverTimeLoadStrategy::ConnectionClosed(void)
{
    const size_t delta = static_cast<size_t>(AvailableLoad());
    if (delta && ConsumeLoad(static_cast<double>(delta)))
        mClientBlock.SetAvailableLoad(delta, static_cast<uint32_t>(GetLoad()));
}

///////////////////////////////////////////////////////////////////////////////

// Currently this implementation mirrors ConnectionsLoadStrategy since the HttpClientBlock limits each
// connection to one transaction each.  It is much simpler to control the number of transactions this
// way rather than trying to deal with variable numbers of connections and transactions simultaneously.

void HttpClientBlock::TransactionsLoadStrategy::LoadChangeHook(int32_t load)
{
    TC_LOG_INFO_LOCAL(mClientBlock.mPort, LOG_CLIENT, "HTTP client block [" << mClientBlock.Name() << "] transaction load changed to " << load);

    mClientBlock.SetIntendedLoad(static_cast<uint32_t>(load));

    // Update the intended load value in the block stats
    ACE_GUARD(stats_t::lock_t, guard, mClientBlock.mStats.Lock());
    mClientBlock.mStats.intendedLoad = static_cast<uint32_t>(load);
}

void HttpClientBlock::TransactionsLoadStrategy::ConnectionClosed(void)
{
    mClientBlock.SetIntendedLoad(static_cast<uint32_t>(GetLoad()));
}

///////////////////////////////////////////////////////////////////////////////

// Currently this implementation mirrors ConnectionsOverTimeLoadStrategy since the HttpClientBlock limits each
// connection to one transaction each.  It is much simpler to control the number of transactions this
// way rather than trying to deal with variable numbers of connections and transactions simultaneously.

void HttpClientBlock::TransactionsOverTimeLoadStrategy::LoadChangeHook(double load)
{
    TC_LOG_INFO_LOCAL(mClientBlock.mPort, LOG_CLIENT, "HTTP client block [" << mClientBlock.Name() << "] transaction over time load changed to " << load);

    const size_t delta = static_cast<size_t>(load);
    if (delta && ConsumeLoad(static_cast<double>(delta)))
        mClientBlock.SetAvailableLoad(delta, static_cast<uint32_t>(GetLoad()));

    // Update the intended load value in the block stats
    ACE_GUARD(stats_t::lock_t, guard, mClientBlock.mStats.Lock());
    mClientBlock.mStats.intendedLoad = static_cast<uint32_t>(GetLoad());
}

void HttpClientBlock::TransactionsOverTimeLoadStrategy::ConnectionClosed(void)
{
    const size_t delta = static_cast<size_t>(AvailableLoad());
    if (delta && ConsumeLoad(static_cast<double>(delta)))
        mClientBlock.SetAvailableLoad(delta, static_cast<uint32_t>(GetLoad()));
}

///////////////////////////////////////////////////////////////////////////////

void HttpClientBlock::BandwidthLoadStrategy::LoadChangeHook(int32_t load)
{
    TC_LOG_INFO_LOCAL(mClientBlock.mPort, LOG_CLIENT, "HTTP client block [" << mClientBlock.Name() << "] bandwidth load changed to " << load);

    // Ignore load if using Dynamic Mode
    if (!mClientBlock.GetEnableDynamicLoad())
        mClientBlock.SetDynamicLoad(load);
    else
        load = mClientBlock.GetDynamicLoadTotal();
    
    //  Always limit to 1 connection
    if (load > 0)
      mClientBlock.SetIntendedLoad(1);

    // Update the intended load value in the block stats
    ACE_GUARD(stats_t::lock_t, guard, mClientBlock.mStats.Lock());
    mClientBlock.mStats.intendedLoad = static_cast<uint32_t>(load * 1024);  // convert kbps to bps
}

void HttpClientBlock::BandwidthLoadStrategy::ConnectionClosed(void)
{
    // Always limit to 1 connection
    mClientBlock.SetIntendedLoad(1);
}

///////////////////////////////////////////////////////////////////////////////
