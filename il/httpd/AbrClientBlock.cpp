/// @file
/// @brief Abr Client Block implementation
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <vector>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <app/AsyncMessageInterface.tcc>
#include <app/StreamSocketConnector.tcc>
#include <app/StreamSocketHandler.h>
#include <base/EndpointPairEnumerator.h>
#include <base/LoadProfile.h>
#include <base/LoadScheduler.h>
#include <base/LoadStrategy.h>
#include <phxexception/PHXExceptionFactory.h>

#include "AbrPlayer.h"
#include "AbrClientBlock.h"

#include "HttpdLog.h"
#include "HttpMsgSetSrv_1.h"

USING_HTTP_NS;

struct AbrClientBlock::IOLogicMessage
{
    enum
    {
        STOP_COMMAND,
        CLOSE_COMMAND,
        INTENDED_LOAD_NOTIFICATION,
        AVAILABLE_LOAD_NOTIFICATION,
        CLOSE_NOTIFICATION
    } type;

    // Intended and available load
    uint32_t intendedLoad;
    uint32_t availableLoad;

    // Close notification parameters
    int playerID;
 };

struct AbrClientBlock::AppLogicMessage
{
    enum
    {
        CLOSE_NOTIFICATION
    } type;
};

///////////////////////////////////////////////////////////////////////////////



AbrClientBlock::AbrClientBlock(uint16_t port, const config_t& config, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier)
    : ClientBlock(config),
      mPort(port),
//      mConfig(config),
      mStats(BllHandle()),
      mVideoStats(BllHandle()),
      mAppReactor(appReactor),
      mIOReactor(ioReactor),
      mIOBarrier(ioBarrier),
      mEnabled(true),
      mRunning(false),
      mLoadProfile(new loadProfile_t(mConfig.Common.Load)),
      mLoadStrategy(MakeLoadStrategy()),
      mLoadScheduler(new loadScheduler_t(*mLoadProfile, *mLoadStrategy)),
      mAttemptedSessionCount(0),
      mActiveSessionCount(0),
      mEndpointEnum(new endpointEnum_t(port, IfHandle(), mConfig.Common.Endpoint.DstIf)),
      mIOLogicInterface(new ioMessageInterface_t(mIOReactor)),
      mAppLogicInterface(new appMessageInterface_t(mAppReactor)),
      mAvailableLoadMsgsOut(0),
      mStoringStopNotification(false),
      mPlayerSerial(0)
{
      mLoadProfile->SetActiveStateChangeDelegate(boost::bind(&AbrClientBlock::LoadProfileHandler, this, _1));

      mEndpointEnum->SetPattern(static_cast<endpointEnum_t::pattern_t>(mConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern));
      mEndpointEnum->SetSrcPortNum(0);
      mEndpointEnum->SetDstPortNum(mConfig.ServerPortInfo);

      mIOLogicInterface->SetMessageDelegate(fastdelegate::MakeDelegate(this, &AbrClientBlock::HandleIOLogicMessage));
      mAppLogicInterface->SetMessageDelegate(fastdelegate::MakeDelegate(this, &AbrClientBlock::HandleAppLogicMessage));

}

///////////////////////////////////////////////////////////////////////////////

AbrClientBlock::~AbrClientBlock()
{
    PreStop();
	Stop();
}
///////////////////////////////////////////////////////////////////////////////

AbrClientBlock::LoadStrategy *AbrClientBlock::MakeLoadStrategy(void)
{

    const L4L7_BASE_NS::LoadTypes loadType = static_cast<const L4L7_BASE_NS::LoadTypes>(mConfig.Common.Load.LoadProfile.LoadType);
    switch (loadType)
    {
      case L4L7_BASE_NS::LoadTypes_CONNECTIONS:
      case L4L7_BASE_NS::LoadTypes_TRANSACTIONS:
          return new SessionsLoadStrategy(*this);

      case L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT:
      case L4L7_BASE_NS::LoadTypes_TRANSACTIONS_PER_TIME_UNIT:
          return new SessionsOverTimeLoadStrategy(*this);


      default:
          TC_LOG_ERR_LOCAL(mPort, LOG_CLIENT, "Cannot create load strategy for unknown load type (" << tms_enum_to_string(loadType, L4L7_BASE_NS::em_LoadTypes) << ")");
          throw EPHXBadConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////
void AbrClientBlock::Start(void)
{
    if (!mEnabled || mRunning)
        return;

    mRunning = true;



     // Reset our internal load state
     mAttemptedSessionCount = 0;
     mPlayerSerial = 0;
     mEndpointEnum->Reset();

     // Start load scheduler
     mLoadScheduler->Start(mAppReactor);


}
///////////////////////////////////////////////////////////////////////////////

void AbrClientBlock::PreStop(void)
{
     if (!mEnabled || !mRunning)
         return;
   
    // Stop load scheduler
    mLoadScheduler->Stop();
    mRunning = false;

}


void AbrClientBlock::Stop(void)
{
   if (!mEnabled)
          return;

      // Reset the intended load
      {
          ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
          mStats.intendedLoad = 0;
      }


      // Ask the I/O logic to stop initiating new Session
      {
          ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
          IOLogicMessage& msg(*msgPtr);
          msg.type = IOLogicMessage::STOP_COMMAND;
          mIOLogicInterface->Send(msgPtr);
      }
      // Wait for the stop message to be processed
      mIOLogicInterface->Wait();

      // Wait for all I/O threads to exit
      {
          ACE_WRITE_GUARD(ACE_Lock, guard, *mIOBarrier);
      }

      // Ask the I/O logic to close any remaining Sessions
      {
          ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
          IOLogicMessage& msg(*msgPtr);
          msg.type = IOLogicMessage::CLOSE_COMMAND;
          mIOLogicInterface->Send(msgPtr);
      }
     // Wait for the close message to be processed
      mIOLogicInterface->Wait();
      // Wait for all I/O threads to exit
      {
          ACE_WRITE_GUARD(ACE_Lock, guard, *mIOBarrier);
      }

      // Stop load profile last (triggers notification if not already at end)
      mLoadProfile->Stop();


}


///////////////////////////////////////////////////////////////////////////////

void AbrClientBlock::NotifyInterfaceDisabled(void)
{
 
    PreStop();
    Stop();
    mEnabled = false;
}

///////////////////////////////////////////////////////////////////////////////

void AbrClientBlock::NotifyInterfaceEnabled(void)
{
     mEnabled = true;
}

///////////////////////////////////////////////////////////////////////////////


void AbrClientBlock::SetIntendedLoad(uint32_t intendedLoad)
{
    // Ask the I/O logic to set the intended load
    ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
    IOLogicMessage& msg(*msgPtr);
    msg.type = IOLogicMessage::INTENDED_LOAD_NOTIFICATION;
    msg.intendedLoad = intendedLoad;
    mIOLogicInterface->Send(msgPtr);
}

///////////////////////////////////////////////////////////////////////////////

void AbrClientBlock::SetAvailableLoad(uint32_t availableLoad, uint32_t intendedLoad)
{
    const uint32_t MAX_AVAILABLE_OUT = 2;

    // don't accumulate too many messages
    if (mAvailableLoadMsgsOut >= MAX_AVAILABLE_OUT)
        return;

    ++mAvailableLoadMsgsOut;

    // Ask the I/O logic to set the intended and available load
    ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
    IOLogicMessage& msg(*msgPtr);
    msg.type = IOLogicMessage::AVAILABLE_LOAD_NOTIFICATION;
    msg.availableLoad = availableLoad;
    msg.intendedLoad = intendedLoad;
    mIOLogicInterface->Send(msgPtr);

}


///////////////////////////////////////////////////////////////////////////////


void AbrClientBlock::HandleIOLogicMessage(const IOLogicMessage& msg)
{
    switch (msg.type)
    {
      case IOLogicMessage::STOP_COMMAND:
      {

         map<int, GenericAbrPlayer*>::iterator it;
         for ( it=mAbrPlayerMap.begin() ; it != mAbrPlayerMap.end(); it++ )
         {
             it->second->Stop();
          }
          mIOLogicInterface->Signal(); //temporary "graceful stop until all stopped
          break;
      }

      case IOLogicMessage::CLOSE_COMMAND:
      {
          // Close all active sessions
          ReapSessions(mActiveSessionCount);
          // Signal that the close command was processed
          mIOLogicInterface->Signal();
          break;
      }

       case IOLogicMessage::INTENDED_LOAD_NOTIFICATION:
      {
          const ssize_t delta = msg.intendedLoad - mActiveSessionCount;
          if (delta > 0 && mRunning)
              SpawnSessions(static_cast<size_t>(delta));
        //  else if (delta < 0)
         //     ReapSessions(static_cast<size_t>(-delta));
          break;
      }

      case IOLogicMessage::AVAILABLE_LOAD_NOTIFICATION:
      {
          if (msg.availableLoad && mRunning)
              SpawnSessions(msg.availableLoad);

          --mAvailableLoadMsgsOut;
          break;
      }

      case IOLogicMessage::CLOSE_NOTIFICATION:
      {
          int uID = msg.playerID;
          GenericAbrPlayer* player =  mAbrPlayerMap[uID];
          if (player)
          {
             delete player;
             mAbrPlayerMap.erase(uID);
             mActiveSessionCount--;
             ForwardStopNotification();
             ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
             mStats.activeConnections = mActiveSessionCount; // mActiveConnCount;
          }
          break;
       }


      default:
          throw EPHXInternal();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AbrClientBlock::SpawnSessions(size_t count)
{
     const uint32_t maxSessionAttempted = mConfig.Common.Load.LoadProfile.MaxConnectionsAttempted;
     if (maxSessionAttempted)
        count = std::min(maxSessionAttempted - mAttemptedSessionCount, count);

    const uint32_t maxOpenSession = mConfig.Common.Load.LoadProfile.MaxOpenConnections;
    if (maxOpenSession)
        count = std::min(maxOpenSession - mActiveSessionCount, count);



    size_t countDecr = 1;

    for (; count != 0; count -= countDecr)
    {
        // Get the next endpoint pair in our block
        std::string srcIfName;
        ACE_INET_Addr srcAddr;
        ACE_INET_Addr dstAddr;

        mEndpointEnum->GetEndpointPair(srcIfName, srcAddr, dstAddr);
        mEndpointEnum->Next();
        char dstAddrStr[64];
        if (dstAddr.addr_to_string(dstAddrStr, sizeof(dstAddrStr)) == -1)
        {
            TC_LOG_WARN(0, "Wrong dstAddr: " << dstAddrStr );
             continue;
        }

        char srcAddrStr[64];
        if (srcAddr.addr_to_string(srcAddrStr, sizeof(srcAddrStr)) == -1)
        {
             TC_LOG_WARN(0, "Wrong srcAddr: " << srcAddrStr );
             continue;
        }

       // if ( mPlayerSerial < 2 )
        {
            ACE_GUARD(sessionLock_t, guard, mSessionLock);
            //GenericAbrPlayer* player = CreateAppleAbrPlayer(mAppReactor, 0);
             GenericAbrPlayer* player = CreateAppleAbrPlayer(mIOReactor, 0);
             player->ReportToWithId(this, mPlayerSerial);
             mAbrPlayerMap[mPlayerSerial]=player;
             mPlayerSerial++;
             player->BindToIfName(srcIfName);
             player->SetIpv4Tos(mConfig.Common.Profile.L4L7Profile.Ipv4Tos);
             player->SetIpv6TrafficClass(mConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass);
             player->SetTcpWindowSizeLimit(mConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit);
             player->SetTcpDelayedAck(mConfig.Common.Profile.L4L7Profile.EnableDelayedAck);
             player->SetSourceAddr(srcAddr);

             switch (mConfig.ProtocolProfile.HttpVersion)
             {
                case http_1_port_server::HttpVersionOptions_VERSION_1_0:
                     player->SetHttpVersion(ConfigurationInterface::HTTP_VERSION_10);
                     break;
                case http_1_port_server::HttpVersionOptions_VERSION_1_1:
                     player->SetHttpVersion(ConfigurationInterface::HTTP_VERSION_11);
                     break;
                default: break;
             }
             if( mConfig.ProtocolProfile.EnableKeepAlive)
                player-> SetKeepAliveFlag(true);
             else
                player-> SetKeepAliveFlag(false);

             player->SetUserAgentString(mConfig.ProtocolProfile.UserAgentHeader);

             switch(mConfig.ProtocolProfile.VideoClientBitrateAlg)
             {
                case http_1_port_server::HttpVideoClientBitrateAlgOptions_SMART:
                     player-> SetShiftAlgorithm(ConfigurationInterface::ALGORITHM_SMART, 50, 80);
                     break;
                case http_1_port_server::HttpVideoClientBitrateAlgOptions_NORMAL:
                     player-> SetShiftAlgorithm(ConfigurationInterface::ALGORITHM_NORM, 50, 80);
                     break;
                case http_1_port_server::HttpVideoClientBitrateAlgOptions_CONSTANT:
                     player-> SetShiftAlgorithm(ConfigurationInterface::ALGORITHM_CONST, 50, 80);
                     break;
                default:break;
             }

             /*
             if ( mConfig.ProtocolProfile.VideoClientPredefineMethod == http_1_port_server::HttpVideoClientStartBitrateOptions_PREDEFINE)
             {
                   switch( mConfig.ProtocolProfile.VideoClientStartBitrate )
                   {
                       case http_1_port_server::HttpVideoClientPredefineMethodOptions_MINIMUM:
                            player-> SetStartingBitrate(ConfigurationInterface::BT_STARTING_MIN);
                            break;
                       case http_1_port_server::HttpVideoClientPredefineMethodOptions_MEDIAN:
                            player-> SetStartingBitrate(ConfigurationInterface::BT_STARTING_MED);
                            break;
                       case http_1_port_server::HttpVideoClientPredefineMethodOptions_MAXIMUM:
                            player-> SetStartingBitrate(ConfigurationInterface::BT_STARTING_MAX);
                            break;
                       default:break;
                   }
             }
             else // User defined  bit rate
             {
                 player->SetUserdDefinedStartingBitrate(mConfig.ProtocolProfile.VideoClientUserDefineValue);// not implemented
             }
              */

             if(mConfig.ProtocolProfile.VideoClientVideoType == http_1_port_server::HttpVideoClientVideoTypeOptions_VOD )
                player->SetPlayDuration(24*60*60*1000); //
             else
                player->SetPlayDuration(mConfig.ProtocolProfile.VideoClientViewTime*1000);

             std::ostringstream uri;
             if(mConfig.ProtocolProfile.VideoClientVideoType == http_1_port_server::HttpVideoClientVideoTypeOptions_VOD )
                 uri << "http://" <<dstAddrStr<< "/VOD/spirentPlaylist.m3u8";
             else
                 uri << "http://" <<dstAddrStr<< "/LIVE/spirentPlaylist.m3u8";

               player->Play(uri.str());
               mActiveSessionCount++;
               ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
               mStats.activeConnections = mActiveSessionCount; // mActiveConnCount;
        }

        mAttemptedSessionCount++;
        {
           ACE_GUARD(videoStats_t::lock_t, guard, mVideoStats.Lock());
           mVideoStats.asSessionsAttempted++;
        }

    }


}

///////////////////////////////////////////////////////////////////////////////

void AbrClientBlock::ReapSessions(size_t count)
{

	count = std::min(mActiveSessionCount, count);

//
//    map<int, GenericAbrPlayer*>::iterator it;
//    for ( it=mAbrPlayerMap.begin() ; it != mAbrPlayerMap.end(); it++ )
//    {
// //   	it->second->Stop();
//        delete it->second;
//        mAbrPlayerMap.erase(it);
//        mActiveSessionCount--;
//    }

    mActiveSessionCount-=count;
    ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
    mStats.activeConnections = mActiveSessionCount; // mActiveConnCount;
    {
      ACE_GUARD(videoStats_t::lock_t, stats_guard, mVideoStats.Lock());
      mVideoStats.asSessionsSuccessful += count;
    }

}


///////////////////////////////////////////////////////////////////////////////

void AbrClientBlock::HandleAppLogicMessage(const AppLogicMessage& msg)
{
    switch (msg.type)
    {
      case AppLogicMessage::CLOSE_NOTIFICATION:
      {
          // Notify the load strategy that a connection was closed
          if (mLoadScheduler->Running())
              mLoadStrategy->SessionClosed();

          break;
      }

      default:
          throw EPHXInternal();
    }
}

void AbrClientBlock::LoadProfileHandler(bool running)
{
    if (running || !mActiveSessionCount)
    {
        // Cascade notification
        if (!mLoadProfileNotifier.empty())
            mLoadProfileNotifier(BllHandle(), running);
    }
    else
    {
        // Delay stopped notification until active connections == 0
        mStoringStopNotification = true;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AbrClientBlock::ForwardStopNotification()
{
    if (mStoringStopNotification && !mActiveSessionCount)
    {
        mStoringStopNotification = false;
        if (!mLoadProfileNotifier.empty())
            mLoadProfileNotifier(BllHandle(), false);
    }
}


/////////////////////////
///////////////////////
void AbrClientBlock::SessionsLoadStrategy::LoadChangeHook(int32_t load)
{
    mClientBlock.SetIntendedLoad(static_cast<uint32_t>(load));
    // Update the intended load value in the block stats
    ACE_GUARD(stats_t::lock_t, guard, mClientBlock.mStats.Lock());
    mClientBlock.mStats.intendedLoad = static_cast<uint32_t>(load);
}

void AbrClientBlock::SessionsLoadStrategy::SessionClosed(void)
{
    mClientBlock.SetIntendedLoad(static_cast<uint32_t>(GetLoad()));
}

///////////////////////////////////////////////////////////////////////////////

void AbrClientBlock::SessionsOverTimeLoadStrategy::LoadChangeHook(double load)
{
   const size_t delta = static_cast<size_t>(load);
    if (delta && ConsumeLoad(static_cast<double>(delta)))
        mClientBlock.SetAvailableLoad(delta, static_cast<uint32_t>(GetLoad()));

    // Update the intended load value in the block stats
    ACE_GUARD(stats_t::lock_t, guard, mClientBlock.mStats.Lock());
    mClientBlock.mStats.intendedLoad = static_cast<uint32_t>(GetLoad());


}

void AbrClientBlock::SessionsOverTimeLoadStrategy::SessionClosed(void)
{
    const size_t delta = static_cast<size_t>(AvailableLoad());
    if (delta && ConsumeLoad(static_cast<double>(delta)))
        mClientBlock.SetAvailableLoad(delta, static_cast<uint32_t>(GetLoad()));
}

void AbrClientBlock::NotifyStatusChanged(int uID, int uStatus)
 {
    GenericAbrPlayer* player =  mAbrPlayerMap[uID];
    if (player)
    {

        if( (uStatus ==GenericAbrObserver::STATUS_REPORT) || (uStatus == GenericAbrObserver::STATUS_STOPPED)|| (uStatus == GenericAbrObserver::STATUS_ABORTED))
        {
            StatusInfo Info;
            player->GetStatusInfo(&Info);

            {
              ACE_GUARD(videoStats_t::lock_t, stats_guard, mVideoStats.Lock());
              int numMnReqSuccess = Info.mGetManifestTransactions[StatusInfo::TRANSACTION_SUCESSFUL];
              int numMnReqFailed  = Info.mGetManifestTransactions[StatusInfo::TRANSACTION_FAILED];
              int numMnReqAttemp  = numMnReqSuccess + numMnReqFailed; // Info.mGetManifestTransactions[StatusInfo::TRANSACTION_ATTEMPTED];
              mVideoStats.manifestReqsAttempted     += numMnReqAttemp;
              mVideoStats.manifestReqsSuccessful    += numMnReqSuccess;
              mVideoStats.manifestReqsUnsuccessful  += numMnReqFailed;
              mVideoStats.sumBufferingWaitTime      += Info.mBufferingDuration;
              mVideoStats.countBufferingWaitTime    += Info.mBufferingCount;
              mVideoStats.mediaFragmentsRx64kbps    += Info.mGetFragmentTransactions[StatusInfo::RATE_0_064];
              mVideoStats.mediaFragmentsRx96kbps    += Info.mGetFragmentTransactions[StatusInfo::RATE_0_096];
              mVideoStats.mediaFragmentsRx150kbps   += Info.mGetFragmentTransactions[StatusInfo::RATE_0_150];
              mVideoStats.mediaFragmentsRx240kbps   += Info.mGetFragmentTransactions[StatusInfo::RATE_0_240];
              mVideoStats.mediaFragmentsRx256kbps   += Info.mGetFragmentTransactions[StatusInfo::RATE_0_256];
              mVideoStats.mediaFragmentsRx440kbps   += Info.mGetFragmentTransactions[StatusInfo::RATE_0_440];
              mVideoStats.mediaFragmentsRx640kbps   += Info.mGetFragmentTransactions[StatusInfo::RATE_0_640];
              mVideoStats.mediaFragmentsRx800kbps   += Info.mGetFragmentTransactions[StatusInfo::RATE_0_800];
              mVideoStats.mediaFragmentsRx840kbps   += Info.mGetFragmentTransactions[StatusInfo::RATE_0_840];
              mVideoStats.mediaFragmentsRx1240kbps  += Info.mGetFragmentTransactions[StatusInfo::RATE_1_240];
            }

            {
              ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
             //TODO

              int numTranSuccess = Info.mGetManifestTransactions[StatusInfo::TRANSACTION_SUCESSFUL];
              int numTranFailed  = Info.mGetManifestTransactions[StatusInfo::TRANSACTION_FAILED];
              int numTranAttemp  = numTranSuccess + numTranFailed; // Info.mGetManifestTransactions[StatusInfo::TRANSACTION_ATTEMPTED];
              for(int i = 0; i<StatusInfo::RATE_MAX; i++)
              {
            	  numTranSuccess += Info.mGetFragmentTransactions[i];
            	  numTranAttemp  += Info.mGetFragmentTransactions[i];
              }

              mStats.attemptedTransactions    += numTranAttemp;
              mStats.successfulTransactions   += numTranSuccess;
              mStats.unsuccessfulTransactions += numTranFailed;
              mStats.abortedTransactions      += numTranFailed;   //????

              mStats.attemptedConnections     += numTranAttemp;  //temporary
              mStats.successfulConnections    += numTranSuccess; //temporary
              mStats.unsuccessfulConnections  += numTranFailed;  //temporary
              mStats.abortedConnections       += numTranFailed;  //????

              mStats.goodputRxBytes    += Info.mBytesReceived;
              mStats.goodputTxBytes    += Info.mBytesSent;

              mStats.sumResponseTimePerUrlMsec += Info.mHttpResponseTime;
              mStats.rxResponseCode200 += Info.mHttpStatus[StatusInfo::HTTP_STATUS_200];
            }

            if (uStatus == GenericAbrObserver::STATUS_ABORTED)
            {
                {
                    ACE_GUARD(videoStats_t::lock_t, stats_guard, mVideoStats.Lock());
                    mVideoStats.asSessionsUnsuccessful += 1;
                }
                // Notify the I/O logic that a player was stopped
                {
                    ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
                    IOLogicMessage& msg(*msgPtr);
                    msg.type = IOLogicMessage::CLOSE_NOTIFICATION;
                    msg.playerID = uID;
                    mIOLogicInterface->Send(msgPtr);
                }

                // Notify the application logic that a player was stopped
                {
                    appMessageInterface_t::messagePtr_t msgPtr(mAppLogicInterface->Allocate());
                    AppLogicMessage& msg(*msgPtr);
                    msg.type = AppLogicMessage::CLOSE_NOTIFICATION;
                    mAppLogicInterface->Send(msgPtr);
                }

            }

            else if( uStatus == GenericAbrObserver::STATUS_STOPPED)
            {

               {
                  ACE_GUARD(videoStats_t::lock_t, stats_guard, mVideoStats.Lock());
                  mVideoStats.asSessionsSuccessful += 1;
                }
               // Notify the I/O logic that a player was stopped
                {
                   ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
                   IOLogicMessage& msg(*msgPtr);
                   msg.type = IOLogicMessage::CLOSE_NOTIFICATION;
                   msg.playerID = uID;
                   mIOLogicInterface->Send(msgPtr);
                }

                // Notify the application logic that a player was stopped
                {
                    appMessageInterface_t::messagePtr_t msgPtr(mAppLogicInterface->Allocate());
                    AppLogicMessage& msg(*msgPtr);
                    msg.type = AppLogicMessage::CLOSE_NOTIFICATION;
                    mAppLogicInterface->Send(msgPtr);
                 }

            }
        }
    }
//    else
//      TC_LOG_WARN(0, "ERROR::NotifyStatusChanged: " << uID << " Status "<< uStatus);

 }
