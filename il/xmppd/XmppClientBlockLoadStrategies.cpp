/// @file
/// @brief XMPP Client Block Load Strategies implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include "XmppClientBlockLoadStrategies.h"
#include "XmppdLog.h"

USING_XMPP_NS;

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::RegistrationLoadStrategy::LoadChangeHook(double load)
{
    const size_t deltaLoad = static_cast<size_t>(load);

    if (deltaLoad && ConsumeLoad(static_cast<double>(deltaLoad)))
    {
        if (mClientBlock.IsRegistering())
            mClientBlock.StartRegistrations(deltaLoad);
        else
            mClientBlock.StartUnregistrations(deltaLoad);
    } 

    ACE_GUARD(stats_t::lock_t, guard, mClientBlock.mStats.Lock());
    mClientBlock.mStats.intendedRegistrationLoad = GetLoad();
    //mClientBlock.mStats.setDirty(true);
}

void XmppClientBlock::RegistrationLoadStrategy::RegistrationCompleted(void)
{
    const size_t deltaLoad = static_cast<size_t>(AvailableLoad());

    if (deltaLoad && ConsumeLoad(static_cast<double>(deltaLoad)))
    {
        if (mClientBlock.IsRegistering())
            mClientBlock.StartRegistrations(deltaLoad);
        else
            mClientBlock.StartUnregistrations(deltaLoad);
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::ConnectionsLoadStrategy::LoadChangeHook(int32_t load)
{

    mClientBlock.SetIntendedLoad(static_cast<uint32_t>(load));

    // Update the intended load value in the block stats
    ACE_GUARD(stats_t::lock_t, guard, mClientBlock.mStats.Lock());
    mClientBlock.mStats.intendedLoad = static_cast<uint32_t>(load);
}

void XmppClientBlock::ConnectionsLoadStrategy::ConnectionClosed(void)
{
    mClientBlock.SetIntendedLoad(static_cast<uint32_t>(GetLoad()));
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::ConnectionsOverTimeLoadStrategy::LoadChangeHook(double load)
{

    const size_t delta = static_cast<size_t>(load);
    if (delta && ConsumeLoad(static_cast<double>(delta)))
        mClientBlock.SetAvailableLoad(delta, static_cast<uint32_t>(GetLoad()));

    // Update the intended load value in the block stats
    ACE_GUARD(stats_t::lock_t, guard, mClientBlock.mStats.Lock());
    mClientBlock.mStats.intendedLoad = static_cast<uint32_t>(GetLoad());
}

void XmppClientBlock::ConnectionsOverTimeLoadStrategy::ConnectionClosed(void)
{
    const size_t delta = static_cast<size_t>(AvailableLoad());
    if (delta && ConsumeLoad(static_cast<double>(delta)))
        mClientBlock.SetAvailableLoad(delta, static_cast<uint32_t>(GetLoad()));
}

///////////////////////////////////////////////////////////////////////////////
