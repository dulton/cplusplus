/// @file
/// @brief FTP Server Block implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
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

#include "FtpdLog.h"
#include "FtpServer.h"
#include "FtpServerBlock.h"

USING_FTP_NS;

///////////////////////////////////////////////////////////////////////////////

FtpServerBlock::FtpServerBlock(uint16_t port, const config_t& config, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier)
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

    // Create a FtpServer instance for each endpoint in our block
    const size_t numServers = mIfEnum->TotalCount();
    mServerVec.reserve(numServers);
    
    for (size_t i = 0; i < numServers; i++)
    {
        serverPtr_t server(new FtpServer(mPort, i, mConfig, mStats, mAppReactor, mIOReactor, mIOBarrier));
        mServerVec.push_back(server);
    }
    
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "FTP server block [" << Name() << "] created with " << mServerVec.size() << " servers");
}

///////////////////////////////////////////////////////////////////////////////

FtpServerBlock::~FtpServerBlock()
{
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "FTP server block [" << Name() << "] being destroyed");
}

///////////////////////////////////////////////////////////////////////////////
    
void FtpServerBlock::Start(void)
{
    if (mRunning)
        return;
    
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "FTP server block [" << Name() << "] starting");

    mRunning = true;

    // Reset endpoint enumerator
    mIfEnum->Reset();

    // Start FtpServer objects
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

void FtpServerBlock::Stop(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "FTP server block [" << Name() << "] stopping");

    mRunning = false;
    
    // Stop FtpServer objects
    for_each(mServerVec.begin(), mServerVec.end(), boost::bind(&FtpServer::Stop, _1));
		
	// Stop stats timer
	mStats.CancelTimer();
}

///////////////////////////////////////////////////////////////////////////////

void FtpServerBlock::NotifyInterfaceDisabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "[FtpServerBlock::NotifyInterfaceDisabled] FTP server block [" << Name() << "] notified interface is disabled");

    // Notify all FtpServer objects
    for_each(mServerVec.begin(), mServerVec.end(), boost::mem_fn(&FtpServer::NotifyInterfaceDisabled));
    mEnabled = false;
}

///////////////////////////////////////////////////////////////////////////////

void FtpServerBlock::NotifyInterfaceEnabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "[FtpServerBlock::NotifyInterfaceEnabled] FTP server block [" << Name() << "] notified interface is enabled");

    // Notify all FtpServer objects
    for_each(mServerVec.begin(), mServerVec.end(), boost::mem_fn(&FtpServer::NotifyInterfaceEnabled));
    mEnabled = true;
}

///////////////////////////////////////////////////////////////////////////////
