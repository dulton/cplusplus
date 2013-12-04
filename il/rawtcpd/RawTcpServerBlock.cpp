/// @file
/// @brief Raw tcp Server Block implementation
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <algorithm>

#include <ace/INET_Addr.h>
#include <ildaemon/LocalInterfaceEnumerator.h>
#include <boost/bind.hpp>

#include "RawtcpdLog.h"
#include "RawTcpServer.h"
#include "RawTcpServerBlock.h"

USING_RAWTCP_NS;

///////////////////////////////////////////////////////////////////////////////

RawTcpServerBlock::RawTcpServerBlock(uint16_t port, const config_t& config, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier)
    : mPort(port),
      mConfig(config),
      mStats(BllHandle()),
      mAppReactor(appReactor),
      mIOReactor(ioReactor),
      mIOBarrier(ioBarrier),
      mEnabled(true),
      mRunning(false),
      mIfEnum(new ifEnum_t(port, IfHandle()))
{
    // Configure the endpoint enumerator
    mIfEnum->SetPortNum(mConfig.ProtocolProfile.ServerPortNum);

    // Create a RawTcpServer instance for each endpoint in our block
    const size_t numServers = mIfEnum->TotalCount();
    mServerVec.reserve(numServers);
    
    for (size_t i = 0; i < numServers; i++)
    {
        serverPtr_t server(new RawTcpServer(mPort, i, mConfig, mStats, mAppReactor, mIOReactor, mIOBarrier));
        mServerVec.push_back(server);
    }
    
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "RAWTCP server block [" << Name() << "] created with " << mServerVec.size() << " servers");
}

///////////////////////////////////////////////////////////////////////////////

RawTcpServerBlock::~RawTcpServerBlock()
{
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "RAWTCP server block [" << Name() << "] being destroyed");
}

///////////////////////////////////////////////////////////////////////////////
    
void RawTcpServerBlock::Start(void)
{
    if (mRunning)
        return;
    
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "RAWTCP server block [" << Name() << "] starting");

    mRunning = true;

    // Reset endpoint enumerator
    mIfEnum->Reset();
    
    // Start RawTcpServer objects
    std::string ifName;
    ACE_INET_Addr addr;

    const size_t numServers = mServerVec.size();
    for (size_t i = 0; i < numServers; i++)
    {
        mIfEnum->GetInterface(ifName, addr);
        mIfEnum->Next();
        mServerVec[i]->Start(ifName, addr);
    }

	// Set stats Reactor instance
	mStats.SetReactorInstance(mAppReactor);
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpServerBlock::Stop(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "RAWTCP server block [" << Name() << "] stopping");

    mRunning = false;
    
    // Stop RawTcpServer objects
    for_each(mServerVec.begin(), mServerVec.end(), boost::bind(&RawTcpServer::Stop, _1));

	// Stop stats timer
	mStats.CancelTimer();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpServerBlock::NotifyInterfaceDisabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "[RawTcpServerBlock::NotifyInterfaceDisabled] RAWTCP server block [" << Name() << "] notified interface is disabled");

    // Notify all RawTcpServer objects
    for_each(mServerVec.begin(), mServerVec.end(), boost::mem_fn(&RawTcpServer::NotifyInterfaceDisabled));
    mEnabled = false;
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpServerBlock::NotifyInterfaceEnabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "[RawTcpServerBlock::NotifyInterfaceEnabled] RAWTCP server block [" << Name() << "] notified interface is enabled");

    // Notify all RawTcpServer objects
    for_each(mServerVec.begin(), mServerVec.end(), boost::mem_fn(&RawTcpServer::NotifyInterfaceEnabled));
    mEnabled = true;
}

///////////////////////////////////////////////////////////////////////////////
