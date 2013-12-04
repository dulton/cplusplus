/// @file
/// @brief SIP User Agent Block Load Strategies implementations
///
///  Copyright (c) 2008 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include "UserAgentBlockLoadStrategies.h"
#include "VoIPLog.h"

USING_APP_NS;

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::RegistrationLoadStrategy::LoadChangeHook(double load)
{
    const size_t deltaLoad = static_cast<size_t>(load);

    TC_LOG_INFO_LOCAL(mUaBlock.mPort, LOG_SIP, "[UserAgentBlock::RegistrationLoadStrategy::LoadChangeHook] User Agent block (" << mUaBlock.BllHandle() << ") registration load changed to "
                      << load << ", delta is " << deltaLoad);
    
    if (deltaLoad && ConsumeLoad(static_cast<double>(deltaLoad)))
    {
        if (mUaBlock.IsRegistering())
            mUaBlock.StartRegistrations(deltaLoad);
        else
            mUaBlock.StartUnregistrations(deltaLoad);
    }

    mUaBlock.mStats.intendedRegLoad = GetLoad();
    mUaBlock.mStats.setDirty(true);
}

void UserAgentBlock::RegistrationLoadStrategy::RegistrationCompleted(void)
{
    const size_t deltaLoad = static_cast<size_t>(AvailableLoad());
    
    if (deltaLoad && ConsumeLoad(static_cast<double>(deltaLoad)))
    {
        if (mUaBlock.IsRegistering())
            mUaBlock.StartRegistrations(deltaLoad);
        else
            mUaBlock.StartUnregistrations(deltaLoad);
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::CallsLoadStrategy::LoadChangeHook(int32_t load)
{
    const ssize_t deltaLoad = load - mUaBlock.ActiveCalls();

    TC_LOG_INFO_LOCAL(mUaBlock.mPort, LOG_SIP, "[UserAgentBlock::CallsLoadStrategy::LoadChangeHook] User Agent block (" << mUaBlock.BllHandle() << ") call load changed to "
                      << load << ", delta is " << deltaLoad);

    if (deltaLoad > 0)
        mUaBlock.InitiateCalls(deltaLoad);
    else if (deltaLoad < 0)
        mUaBlock.AbortCalls(-deltaLoad,true);

    mUaBlock.mStats.intendedCallLoad = load;
    mUaBlock.mStats.setDirty(true);
}

void UserAgentBlock::CallsLoadStrategy::CallCompleted(void)
{
    const ssize_t deltaLoad = GetLoad() - mUaBlock.ActiveCalls();

    if (deltaLoad > 0)
        mUaBlock.InitiateCalls(deltaLoad);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentBlock::CallsOverTimeLoadStrategy::LoadChangeHook(double load)
{
    const size_t deltaLoad = static_cast<size_t>(load);

    TC_LOG_INFO_LOCAL(mUaBlock.mPort, LOG_SIP, "[UserAgentBlock::CallsOverTimeLoadStrategy::LoadChangeHook] User Agent block (" << mUaBlock.BllHandle() << ") load changed to "
                      << load << ", delta is " << deltaLoad);
    
    if (deltaLoad && ConsumeLoad(static_cast<double>(deltaLoad)))
        mUaBlock.InitiateCalls(deltaLoad);

    mUaBlock.mStats.intendedCallLoad = GetLoad();
    mUaBlock.mStats.setDirty(true);
}

void UserAgentBlock::CallsOverTimeLoadStrategy::CallCompleted(void)
{
    const size_t deltaLoad = static_cast<size_t>(AvailableLoad());
    
    if (deltaLoad && ConsumeLoad(static_cast<double>(deltaLoad)))
        mUaBlock.InitiateCalls(deltaLoad);
}

///////////////////////////////////////////////////////////////////////////////
