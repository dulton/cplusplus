/// @file
/// @brief HTTP Server Block implementation
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

#include <phxexception/PHXException.h>

#include "HttpdLog.h"
#include "HttpServer.h"
#include "AbrServer.h"
#include "HttpServerBlock.h"

USING_HTTP_NS;

///////////////////////////////////////////////////////////////////////////////

HttpServerBlock::HttpServerBlock(uint16_t port, const config_t& config, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier)
    : mPort(port),
      mConfig(config),
      mStats(BllHandle()),
      mVideoStats(BllHandle()),
      mAppReactor(appReactor),
      mIOReactor(ioReactor),
      mIOBarrier(ioBarrier),
      mEnabled(true),
      mRunning(false),
      mPlaylist(0),
      mEnableVideoServer(mConfig.ProtocolProfile.EnableVideoServer),
      mIfEnum(new ifEnum_t(port, IfHandle()))
{
    // Configure the endpoint enumerator
    mIfEnum->SetPortNum(mConfig.ProtocolProfile.ServerPortNum);

    // Create a HttpServer instance for each endpoint in our block
    const size_t numServers = mIfEnum->TotalCount();
    mServerVec.reserve(numServers);


    if (mConfig.ProtocolProfile.EnableVideoServer) {
        ABR_UTILS_NS::videoServerBitrateList_t bitrateList;
        bitrateList = mConfig.ProtocolProfile.VideoServerBitrateList;

        //TODO:  Add check for other server types: Microsoft, Adobe
        switch(mConfig.ProtocolProfile.VideoServerType) {
            case http_1_port_server::HttpVideoServerTypeOptions_LIVE_STREAMING:
                mPlaylist.reset(new ABR_NS::HlsAbrPlaylist( mConfig.ProtocolProfile.VideoServerVersion,  	 // Video server version
                                                        mConfig.ProtocolProfile.ServerTargetDuration,  		 // Target duration.  Fragment length.
                                                        mConfig.ProtocolProfile.VideoServerStreamType,    	 // Video stream type.  Progressive/adaptive.
                                                        0,    							 // Window type.  Simple/Sliding.  Not being used.
                                                        mConfig.ProtocolProfile.ServerMediaSeqNum,  		 // Media sequence number
                                                        mConfig.ProtocolProfile.ServerSlidingWindowPlaylistSize, // Max sliding window size
                                                        mConfig.ProtocolProfile.ServerVideoLength, 		 // Video length.  Used for VOD
                                                        ACE_OS::gettimeofday(),            			 // Start time
                                                        bitrateList)                      			 // Bitrate list
                                );

                if(!mPlaylist.get()) {
                    TC_LOG_ERR(0, "[HttpServerBlock] unable to instantiate ABR playlist");
                    throw EPHXInternal();
                    return;
                }
                break;
            default:
                TC_LOG_ERR(0, "[HttpServerBlock] invalid video server type received from BLL");
                throw EPHXBadConfig();
                break;
        }

        for (size_t i = 0; i < numServers; i++)
        {
            serverPtr_t server(new AbrServer(mPort, i, mConfig, mStats, mVideoStats, mAppReactor, mIOReactor, mIOBarrier, mPlaylist.get(),mEnableVideoServer));
            mServerVec.push_back(server);
        }
    } else{
        for (size_t i = 0; i < numServers; i++)
        {
            serverPtr_t server(new HttpServer(mPort, i, mConfig, mStats, mAppReactor, mIOReactor, mIOBarrier));
            mServerVec.push_back(server);
        }
    }

    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "HTTP server block [" << Name() << "] created with " << mServerVec.size() << " servers");
}

///////////////////////////////////////////////////////////////////////////////

HttpServerBlock::~HttpServerBlock()
{
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "HTTP server block [" << Name() << "] being destroyed");
}

///////////////////////////////////////////////////////////////////////////////
    
void HttpServerBlock::Start(void)
{
    if (mRunning)
        return;
    
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "HTTP server block [" << Name() << "] starting");

    mRunning = true;

    // Reset endpoint enumerator
    mIfEnum->Reset();
    
    // Start stats calculations
    mStats.Start();
    if (mEnableVideoServer) {
        mVideoStats.Start();
    }

    // Start HttpServer objects
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

void HttpServerBlock::Stop(void)
{
	TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "HTTP server block [" << Name() << "] stopping");

	mRunning = false;

	// Stop HttpServer objects
	//    for_each(mServerVec.begin(), mServerVec.end(), boost::bind(&HttpServer::Stop, _1));
	// replace with boost later
	const size_t numServers = mServerVec.size();
	for (size_t i = 0; i < numServers; i++)
	{
		mServerVec[i]->Stop();
	}

	// Stop stats timer
	mStats.CancelTimer();
}

///////////////////////////////////////////////////////////////////////////////

void HttpServerBlock::NotifyInterfaceDisabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "[HttpServerBlock::NotifyInterfaceDisabled] HTTP server block [" << Name() << "] notified interface is disabled");

    // Notify all HttpServer objects
//    for_each(mServerVec.begin(), mServerVec.end(), boost::mem_fn(&HttpServer::NotifyInterfaceDisabled));
    // replace with boost later
    const size_t numServers = mServerVec.size();
      for (size_t i = 0; i < numServers; i++)
      {
          mServerVec[i]->NotifyInterfaceDisabled();
      }

    mEnabled = false;
}

///////////////////////////////////////////////////////////////////////////////

void HttpServerBlock::NotifyInterfaceEnabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_SERVER, "[HttpServerBlock::NotifyInterfaceEnabled] HTTP server block [" << Name() << "] notified interface is enabled");

    // Notify all HttpServer objects
//    for_each(mServerVec.begin(), mServerVec.end(), boost::mem_fn(&HttpServer::NotifyInterfaceEnabled));
    // replace with boost later
    const size_t numServers = mServerVec.size();
      for (size_t i = 0; i < numServers; i++)
      {
          mServerVec[i]->NotifyInterfaceEnabled();
      }
    mEnabled = true;
}

///////////////////////////////////////////////////////////////////////////////
