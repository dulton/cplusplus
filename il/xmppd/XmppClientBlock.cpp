/// @file
/// @brief XMPP Client Block implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <vector>
#include <fstream>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <ildaemon/LocalInterfaceEnumerator.h>
#include <app/AsyncMessageInterface.tcc>
#include <app/StreamSocketConnector.tcc>
#include <app/StreamSocketHandler.h>
#include <base/LoadProfile.h>
#include <base/LoadScheduler.h>
#include <base/LoadStrategy.h>
#include <phxexception/PHXExceptionFactory.h>

#include "XmppdLog.h"
#include "XmppClientBlock.h"
#include "XmppClientBlockLoadStrategies.h"
#include "CapabilitiesParser.h"


USING_XMPP_NS;

///////////////////////////////////////////////////////////////////////////////

struct XmppClientBlock::IOLogicMessage
{
    enum
    {
        STOP_COMMAND,
        CLOSE_COMMAND,
        INTENDED_LOAD_NOTIFICATION,
        AVAILABLE_LOAD_NOTIFICATION,
        SET_DYNAMIC_LOAD,
        ENABLE_DYNAMIC_LOAD,
        CLOSE_NOTIFICATION
    } type;

    // Intended and available load
    uint32_t intendedLoad;
    uint32_t availableLoad;

    // Bandwidth parameters
    bool enableDynamicLoad;
    int32_t dynamicLoad;
    
    // Close notification parameters
    uint32_t connHandlerSerial;
};

struct XmppClientBlock::AppLogicMessage
{
    enum
    {
        CLOSE_NOTIFICATION
    } type;
};

///////////////////////////////////////////////////////////////////////////////
XmppClientBlock::XmppClientBlock(uint16_t port, const config_t& config,
                   ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier)
    : mPort(port),
      mConfig(config),
      mStats(BllHandle()),
      mAppReactor(appReactor),
      mIOReactor(ioReactor),
      mIOBarrier(ioBarrier),
      mEnabled(true),
      mRunning(false),  
      mLoadProfile(new loadProfile_t(mConfig.Common.Load)),
      mLoadStrategy(MakeLoadStrategy()),
      mRegStrategy(MakeRegistrationLoadStrategy()),
      mRegIndex(0),
      mRegConnSpawned(0),
      mLoadScheduler(new loadScheduler_t(*mLoadProfile, *mLoadStrategy)),
      mAttemptedConnCount(0),
      mActiveConnCount(0),
      mPendingConnCount(0),
      mEndpointEnum(new endpointEnum_t(port, IfHandle())),
      mIOLogicInterface(new ioMessageInterface_t(mIOReactor)),
      mAppLogicInterface(new appMessageInterface_t(mAppReactor)),
      mAvailableLoadMsgsOut(0),
      mRegState(xmppvj_1_port_server::EnumXmppvJRegistrationState_NOT_REGISTERED),
      mRegProcess(false),
      mMaxClients(getMaxClients()),
 //     mRegIndexTracker(new UniqueIndexTracker()),
      mLoginIndexTracker(new UniqueIndexTracker),
      mIndex(0)
{ 

    mLoadProfile->SetActiveStateChangeDelegate(boost::bind(&XmppClientBlock::LoadProfileHandler, this, _1));

    mEndpointEnum->SetPortNum(0);

    mIOLogicInterface->SetMessageDelegate(fastdelegate::MakeDelegate(this, &XmppClientBlock::HandleIOLogicMessage));
    mAppLogicInterface->SetMessageDelegate(fastdelegate::MakeDelegate(this, &XmppClientBlock::HandleAppLogicMessage));

    InitDebugInfo("XMPP", BllHandle());
    TC_LOG_DEBUG_LOCAL(mPort, LOG_CLIENT, "XMPP client block [" << Name() << "] created");

    SetDynamicLoad(mConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.DynamicLoad);
    EnableDynamicLoad(mConfig.Common.Load.LoadProfile.UseDynamicLoad);


}

///////////////////////////////////////////////////////////////////////////////

XmppClientBlock::~XmppClientBlock()
{
    Stop();
}

///////////////////////////////////////////////////////////////////////////////

std::auto_ptr<XmppClientBlock::RegistrationLoadStrategy> XmppClientBlock::MakeRegistrationLoadStrategy(void)
{

    std::auto_ptr<RegistrationLoadStrategy> loadStrategy(new RegistrationLoadStrategy(*this));

    const XmppvJ_1_port_server::XmppvJProtocolProfile_t& protoProfile(mConfig.ProtocolProfile);
    loadStrategy->SetLoad(static_cast<int32_t>(protoProfile.RegsPerSecond), protoProfile.RegBurstSize);

    return loadStrategy;
}

///////////////////////////////////////////////////////////////////////////////

XmppClientBlock::LoadStrategy *XmppClientBlock::MakeLoadStrategy(void)
{
    SetEnableBwCtrl(false);

    const L4L7_BASE_NS::LoadTypes loadType = static_cast<const L4L7_BASE_NS::LoadTypes>(mConfig.Common.Load.LoadProfile.LoadType);
    switch (loadType)
    {
      case L4L7_BASE_NS::LoadTypes_CONNECTIONS:
      case L4L7_BASE_NS::LoadTypes_TRANSACTIONS:
          return new ConnectionsLoadStrategy(*this);

      case L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT:
      case L4L7_BASE_NS::LoadTypes_TRANSACTIONS_PER_TIME_UNIT:
          return new ConnectionsOverTimeLoadStrategy(*this);
          
      default:
          TC_LOG_ERR_LOCAL(mPort, LOG_CLIENT, "Cannot create load strategy for unknown load type (" << tms_enum_to_string(loadType, L4L7_BASE_NS::em_LoadTypes) << ")");
          throw EPHXBadConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::Start(void)
{
    if (!mEnabled || mRunning)
        return;
    mIndex = 0;
    

    // Allocate and initialize a new connector for the I/O logic to use
    mConnector.reset(new connector_t(mIOReactor));
    mConnector->SetIpv4Tos(mConfig.Common.Profile.L4L7Profile.Ipv4Tos);
    mConnector->SetIpv6TrafficClass(mConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass);
    mConnector->SetTcpWindowSizeLimit(mConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit);
    mConnector->SetTcpDelayedAck(mConfig.Common.Profile.L4L7Profile.EnableDelayedAck);
    mConnector->SetConnectNotificationDelegate(fastdelegate::MakeDelegate(this, &XmppClientBlock::HandleConnectionNotification));
    mConnector->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &XmppClientBlock::HandleCloseNotification));

    mRunning = true;
    mRegProcess = false;

    TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "XMPP client block [" << Name() << "] starting");

    // Reset our internal load state
    mAttemptedConnCount = 0;
    mPendingConnCount = 0;
    mEndpointEnum->Reset();

    // Read the contents of the pub capabilities file
    if (!mConfig.ProtocolConfig.PubCapabilities.empty())
    {
        //clear previous capabilities vector
        if (!mPubCapabilities.empty())
            mPubCapabilities.clear();


        std::ostringstream capFileName;
        capFileName << CapabilitiesFilePath << "/" << mConfig.ProtocolConfig.PubCapabilities;
        std::ifstream ifs(capFileName.str().c_str());
        if (!ifs.is_open())
        {
            std::ostringstream err;
            err << "File not found: " << mConfig.ProtocolConfig.PubCapabilities;
            TC_LOG_ERR_LOCAL(mPort, LOG_CLIENT, err.str());
            throw EInval(err.str());
        }
        std::string pubCapabilities;
        pubCapabilities.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
        ifs.close();

        CapabilitiesParser parser(mPubCapabilities);
//        parser.Parse(pubCapabilities);
        parser.Parse(capFileName.str());   //new parser takes the filepath as input
#if 0
        for (int i=0; i< (int)mPubCapabilities.size(); i++)
            TC_LOG_ERR(0,"tag number "<<i<<" is "<<mPubCapabilities[i]->xml());
#endif
    }
    //read in the publish stanza format
//    std::ostringstream stanzaFileName;
//    stanzaFileName << CapabilitiesFilePath << "/default_stanza_format.txt";
//    std::ifstream pfs(stanzaFileName.str().c_str());
//    if (!pfs.is_open())
//    {
//        std::ostringstream err;
//        err << "File not found: " << stanzaFileName.str();
//        TC_LOG_ERR_LOCAL(mPort, LOG_CLIENT, err.str());
//        throw EInval(err.str());
//    }
//    mPubFormat.assign(std::istreambuf_iterator<char>(pfs), std::istreambuf_iterator<char>());
//    pfs.close();

    
    // Start load scheduler
    mLoadScheduler->Start(mAppReactor);
    InitBwTimeStorage();
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::Stop(void)
{
    if (!mEnabled || !mRunning)
        return;

    // Stop load scheduler
    mLoadScheduler->Stop();
    mRunning = false;

    // Reset the intended load
    {
        ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
        mStats.intendedLoad = 0;
    }

    // Ask the I/O logic to stop initiating new connections
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
    
    // Ask the I/O logic to close any remaining connections
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
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::SetDynamicLoad(int32_t dynamicLoad)
{
    // Ask the I/O logic to set the block load
    ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
    IOLogicMessage& msg(*msgPtr);
    msg.type = IOLogicMessage::SET_DYNAMIC_LOAD;
    msg.dynamicLoad = dynamicLoad;
    mIOLogicInterface->Send(msgPtr);
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::EnableDynamicLoad(bool enable)
{
    // Ask the I/O logic to enable block load
    ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
    IOLogicMessage& msg(*msgPtr);
    msg.type = IOLogicMessage::ENABLE_DYNAMIC_LOAD;
    msg.enableDynamicLoad = enable;
    mIOLogicInterface->Send(msgPtr);
}

///////////////////////////////////////////////////////////////////////////////

int32_t XmppClientBlock::getMaxClients(void)
{
    int32_t maxClients=0;
    int32_t prevHeight=0;

//    for (int i = 0; i<numPhases; ++i)
    BOOST_FOREACH(const L4L7Base_1_port_server::ClientLoadPhaseConfig_t phase, mConfig.Common.Load.LoadPhases)
    {
        switch(phase.LoadPhase.LoadPattern)
        {
          case L4L7_BASE_NS::LoadPatternTypes_STAIR:
              maxClients = std::max(prevHeight + (phase.StairDescriptor.Height*static_cast<int32_t>(phase.StairDescriptor.Repetitions)),maxClients);
              prevHeight= prevHeight + (phase.StairDescriptor.Height*static_cast<int32_t>(phase.StairDescriptor.Repetitions));
              break;
          case L4L7_BASE_NS::LoadPatternTypes_FLAT:
              maxClients = std::max(phase.FlatDescriptor.Height,maxClients);
              prevHeight= phase.FlatDescriptor.Height;
              break;
          case L4L7_BASE_NS::LoadPatternTypes_BURST:
              maxClients = std::max(prevHeight + phase.BurstDescriptor.Height,maxClients);
              break;
          case L4L7_BASE_NS::LoadPatternTypes_SINUSOID:
              maxClients = std::max(prevHeight + phase.SinusoidDescriptor.Height,maxClients);
              break;
          case L4L7_BASE_NS::LoadPatternTypes_RANDOM:
              maxClients = std::max(prevHeight + phase.RandomDescriptor.Height,maxClients);
              prevHeight= prevHeight + phase.RandomDescriptor.Height;
              break;
          case L4L7_BASE_NS::LoadPatternTypes_SAWTOOTH:
              maxClients = std::max(prevHeight + phase.SawToothDescriptor.Height,maxClients);
              break;
          default:
              break;
        }
    }
    return maxClients;
}


void XmppClientBlock::Register(void)
{
    if (!mEnabled || mRunning)
        return;


    // Allocate and initialize a new connector for the I/O logic to use
    mConnector.reset(new connector_t(mIOReactor));
    mConnector->SetIpv4Tos(mConfig.Common.Profile.L4L7Profile.Ipv4Tos);
    mConnector->SetIpv6TrafficClass(mConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass);
    mConnector->SetTcpWindowSizeLimit(mConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit);
    mConnector->SetTcpDelayedAck(mConfig.Common.Profile.L4L7Profile.EnableDelayedAck);
    mConnector->SetConnectNotificationDelegate(fastdelegate::MakeDelegate(this, &XmppClientBlock::HandleConnectionNotification));
    mConnector->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &XmppClientBlock::HandleCloseNotification));

    mAttemptedConnCount = 0;
    mEndpointEnum->Reset();
    mRegProcess = true;


    TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "[XmppClientBlock::Register] Xmpp Client block (" << BllHandle() << ") starting registration");
    ChangeRegState(xmppvj_1_port_server::EnumXmppvJRegistrationState_REGISTERING);

    // Reset the registration variables and (re)start the registration load strategy
    mRegIndex = 0;
    mRegConnSpawned = 0;
    mRegOutstanding = 0;
    mRegStrategy->Start(mAppReactor);
    InitBwTimeStorage();
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::Unregister(void)
{
    if (!mEnabled || mRunning)
        return;

    mRegProcess = true;

    // Allocate and initialize a new connector for the I/O logic to use
    mConnector.reset(new connector_t(mIOReactor));
    mConnector->SetIpv4Tos(mConfig.Common.Profile.L4L7Profile.Ipv4Tos);
    mConnector->SetIpv6TrafficClass(mConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass);
    mConnector->SetTcpWindowSizeLimit(mConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit);
    mConnector->SetTcpDelayedAck(mConfig.Common.Profile.L4L7Profile.EnableDelayedAck);
    mConnector->SetConnectNotificationDelegate(fastdelegate::MakeDelegate(this, &XmppClientBlock::HandleConnectionNotification));
    mConnector->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &XmppClientBlock::HandleCloseNotification));

    mAttemptedConnCount = 0;
    mEndpointEnum->Reset();
    mRegProcess = true;

    TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "[XmppClientBlock::Unregister] Xmpp Client block (" << BllHandle() << ") starting unregistration");
    ChangeRegState(xmppvj_1_port_server::EnumXmppvJRegistrationState_UNREGISTERING);

    // Reset the registration index and (re)start the registration load strategy
    mRegIndex = 0;
    mRegConnSpawned = 0;
    mRegOutstanding = 0;
    mRegStrategy->Start(mAppReactor);
    InitBwTimeStorage();
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::CancelRegistrations(void)
{

    if (!mEnabled || !mRegStrategy->Running())
        return;

    mRegProcess = true;

    if (mRegStrategy->Running())
        mRegStrategy->Stop();
    TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "shig [XmppClientBlock::CancelRegistrations] Xmpp Client block (" << BllHandle() << ") canceing registration");

    // Reset the intended load
    {
        ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
        mStats.intendedRegistrationLoad = 0;
    }

    // Ask the I/O logic to stop initiating new connections
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
    
    // Ask the I/O logic to close any remaining connections
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

    ChangeRegState(xmppvj_1_port_server::EnumXmppvJRegistrationState_REGISTRATION_CANCELED);
    mRegCompNotifier();
}

//////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::NotifyInterfaceDisabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "[XmppClientBlock::NotifyInterfaceDisabled] XMPP client block [" << Name() << "] notified interface is disabled");

    if (mRegStrategy)
    {
        // Transition to NOT_REGISTERED state and halt all registration activity
        ChangeRegState(xmppvj_1_port_server::EnumXmppvJRegistrationState_NOT_REGISTERED);
        
        if (mRegStrategy->Running())
        {
            mRegStrategy->Stop();
            // Reset the intended load
            {
                ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
                mStats.intendedRegistrationLoad = 0;
            }
            mRegCompNotifier();
        }
    }
    // Notify all Clients
    //     std::for_each(mUaVec.begin(), mUaVec.end(), boost::mem_fn(&UserAgent::NotifyInterfaceDisabled));
    Stop();
    mEnabled = false;
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::NotifyInterfaceEnabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "[XmppClientBlock::NotifyInterfaceEnabled] XMPP client block [" << Name() << "] notified interface is enabled");
    mEnabled = true;
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::StartRegistrations(size_t count)
{

    //get number of clients that should be registering
    const size_t numClientObjects = mMaxClients;

    //Spawn connections if number of connections started is less than the number of clients desired
    if ( count && mRegConnSpawned < numClientObjects)
    {
        size_t numConnSpawn = static_cast<size_t>(std::min(static_cast<int>(count), static_cast<int>(numClientObjects - mRegConnSpawned)));
        SetAvailableLoad(numConnSpawn,static_cast<uint32_t>(mRegStrategy->GetLoad()));
    }

    // no registrations outstanding, and no registrations have failed we can transition to REGISTRATION_SUCCEEDED
    if (!mRegOutstanding && mRegIndex == numClientObjects )
    {
        if ( mRegState != xmppvj_1_port_server::EnumXmppvJRegistrationState_REGISTRATION_FAILED )
            ChangeRegState(xmppvj_1_port_server::EnumXmppvJRegistrationState_REGISTRATION_SUCCEEDED);
        mRegStrategy->Stop();
        // Reset the intended load
        {
            ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
            mStats.intendedRegistrationLoad = 0;
        }
        mRegCompNotifier();
    }
}

void XmppClientBlock::ClientRegister(ClientConnectionHandler& client)
{

    //Set Username, Password, and ClientIndex
    uint32_t clientIndex = mRegIndex;  //mRegIndexTracker->Assign();

    SetClientLogin(client,clientIndex);
    client.SetClientType(ClientConnectionHandler::CLIENT_REGISTER);
    client.SetRegRetryCount(mConfig.ProtocolProfile.RegRetries);
    client.SetStatusDelegate(boost::bind(&XmppClientBlock::NotifyXmppClientStatus, this, _1, _2, _3));

    ++mRegOutstanding;
    ++mRegIndex;
}
///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::StartUnregistrations(size_t count)
{
    //get number of clients that should be registering
    const size_t numClientObjects = mMaxClients;

    //Spawn connections if number of connections started is less than the number of clients desired
    if ( count && mRegConnSpawned < numClientObjects)
    {
        size_t numConnSpawn = static_cast<size_t>(std::min(static_cast<int>(count), static_cast<int>(numClientObjects - mRegConnSpawned)));
        SetAvailableLoad(numConnSpawn,static_cast<uint32_t>(mRegStrategy->GetLoad()));
    }

    // no registrations outstanding, and no registrations have failed we can transition to REGISTRATION_SUCCEEDED
    if ( !mRegOutstanding &&  mRegIndex == numClientObjects )
    {
        ChangeRegState(xmppvj_1_port_server::EnumXmppvJRegistrationState_NOT_REGISTERED);
        mRegStrategy->Stop();
        // Reset the intended load
        {
            ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
            mStats.intendedRegistrationLoad = 0;
        }
        mRegCompNotifier();
    } //this assumes that registration will never fail
}

void XmppClientBlock::ClientUnRegister(ClientConnectionHandler& client)
{
    //Set Username, Password, and client index
    uint32_t clientIndex = mRegIndex;  //mRegIndexTracker->Assign();
    SetClientLogin(client,clientIndex);
    client.SetClientType(ClientConnectionHandler::CLIENT_UNREGISTER);

    ++mRegOutstanding;
    ++mRegIndex;
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::ChangeRegState(regState_t toState)
{
    if (toState != mRegState)
    {
        const regState_t fromState(mRegState);

        TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "[XmppClientBlock::ChangeRegState] XmppClientBlock (" << BllHandle() << ") changing registration state from "
                          << tms_enum_to_string(fromState, XmppvJ_1_port_server::em_EnumXmppvJRegistrationState) << " to " << tms_enum_to_string(toState, XmppvJ_1_port_server::em_EnumXmppvJRegistrationState));
        mRegState = toState;

        if (!mRegStateNotifier.empty())
            mRegStateNotifier(BllHandle(), fromState, toState);
    }
}

///////////////////////////////////////////////////////////////////////////////
void XmppClientBlock::NotifyXmppClientStatus(size_t ClientIndex, ClientConnectionHandler::StatusNotification status, const ACE_Time_Value& deltaTime)
{
    switch (status)
    {
      case ClientConnectionHandler::STATUS_REGISTRATION_SUCCEEDED:
      {
          const uint32_t regTimeMsec = deltaTime.msec();
          {
          ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
          mStats.registrationSuccessCount++;
          mStats.serverResponseMinTime = mStats.serverResponseMinTime ? std::min(mStats.serverResponseMinTime, static_cast<uint64_t>(regTimeMsec)) : static_cast<uint64_t>(regTimeMsec);
          mStats.serverResponseMaxTime = std::max(mStats.serverResponseMaxTime, static_cast<uint64_t>(regTimeMsec));
          mStats.serverResponseCumulativeTime += regTimeMsec;
          }
          // If all registrations were successful, we can transition to REGISTRATION_SUCCEEDED
          // (otherwise it will already indicate REGISTRATION_FAILED)
          if (--mRegOutstanding == 0 && mRegIndex == mMaxClients && mRegState == xmppvj_1_port_server::EnumXmppvJRegistrationState_REGISTERING)
          {
              ChangeRegState(xmppvj_1_port_server::EnumXmppvJRegistrationState_REGISTRATION_SUCCEEDED);
              mRegStrategy->Stop();
              // Reset the intended load
              {
                  ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
                  mStats.intendedRegistrationLoad = 0;
              }
              mRegCompNotifier();
          }
          break;
      }

      case ClientConnectionHandler::STATUS_REGISTRATION_RETRYING:
      {
          {
              ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
              mStats.registrationAttemptCount++;
          }
          break;
      }

      case ClientConnectionHandler::STATUS_REGISTRATION_FAILED:
      {

          // Once a single registration fails, we can transition to REGISTRATION_FAILED immediately
          ChangeRegState(xmppvj_1_port_server::EnumXmppvJRegistrationState_REGISTRATION_FAILED);
          {
              ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
              mStats.registrationFailureCount++;
          }
          mRegOutstanding--;
          if (!mRegOutstanding && mRegIndex == mMaxClients)
          {
              mRegStrategy->Stop();
              // Reset the intended load
              {
                  ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
                  mStats.intendedRegistrationLoad = 0;
              }
              mRegCompNotifier();
          }
          break;
      }
      default:
          break;
    }
}


void XmppClientBlock::SetIntendedLoad(uint32_t intendedLoad)
{
    // Ask the I/O logic to set the intended load
    ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
    IOLogicMessage& msg(*msgPtr);
    msg.type = IOLogicMessage::INTENDED_LOAD_NOTIFICATION;
    msg.intendedLoad = intendedLoad;
    mIOLogicInterface->Send(msgPtr);
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::SetAvailableLoad(uint32_t availableLoad, uint32_t intendedLoad)
{
    const uint32_t MAX_AVAILABLE_OUT = 2;

    // don't accumulate too many messages
    if (mAvailableLoadMsgsOut >= MAX_AVAILABLE_OUT)
    {
//        if (mRegProcess)
//            mRegConnSpawned -= availableLoad;
        return;
    }

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

void XmppClientBlock::HandleIOLogicMessage(const IOLogicMessage& msg)
{
    switch (msg.type)
    {
      case IOLogicMessage::STOP_COMMAND:
      {
          TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "XMPP client block [" << Name() << "] stopping initiating new connections");

          if (mConnector)
          {
              ACE_GUARD(connLock_t, guard, mConnLock);
              
              // Close connector
              mConnector->close();
          }

          // Stop timers in all active connections
          BOOST_FOREACH(const connHandlerMap_t::value_type& pair, mConnHandlerMap)
              pair.second->PurgeTimers();
          
          // Signal that the stop is complete
          mIOLogicInterface->Signal();
          break;
      }

      case IOLogicMessage::CLOSE_COMMAND:
      {
          TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "XMPP client block [" << Name() << "] closing " << mActiveConnCount << " active connections");

          if (mConnector)
          {
              ACE_GUARD(connLock_t, guard, mConnLock);
              
              // Destroy the connector
              mConnector.reset();
          }
          
          // Close all active connections
          ReapConnections(mActiveConnCount);

          // Signal that the close command was processed
          mIOLogicInterface->Signal();
          break;
      }

      case IOLogicMessage::SET_DYNAMIC_LOAD:  // Limited to 1 Connection
      {
          DynamicLoadHandler(mActiveConnCount, msg.dynamicLoad);
          break;
      }      

      case IOLogicMessage::ENABLE_DYNAMIC_LOAD:
      { 
          SetEnableDynamicLoad(msg.enableDynamicLoad);
          mLoadScheduler->SetEnableDynamicLoad(msg.enableDynamicLoad);
          break;
      }

      case IOLogicMessage::INTENDED_LOAD_NOTIFICATION:
      {
          const ssize_t delta = msg.intendedLoad - mActiveConnCount;

          if (delta > 0 && mConnector)
              SpawnConnections(static_cast<size_t>(delta));
          else if (delta < 0)
              ReapConnections(static_cast<size_t>(-delta));
          break;
      }

      case IOLogicMessage::AVAILABLE_LOAD_NOTIFICATION:
      {
          if (msg.availableLoad && mConnector)
              SpawnConnections(msg.availableLoad);
          --mAvailableLoadMsgsOut;
          break;
      }
      
      case IOLogicMessage::CLOSE_NOTIFICATION:
      {
          if (mConnHandlerMap.find(msg.connHandlerSerial) == mConnHandlerMap.end())
              mPendingConnCount--;
          // Delete the connection handler from our map and aging queue
          if (mConnHandlerMap.erase(msg.connHandlerSerial))
          {
              mActiveConnCount--;
              ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
              mStats.activeConnections = mActiveConnCount;
          } else
          {
              TC_LOG_ERR(0,"[IOLogicMessage::CLOSE_NOTIFICATION] connhandler not erased");
          }

          mConnHandlerAgingQueue.erase(msg.connHandlerSerial);
          break;
      }

      default:
          TC_LOG_ERR(0,"[IOLogicMessage::Default] message type not found");
          throw EPHXInternal();
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::SpawnConnections(size_t count)
{
    // New connections may be limited by the L4-L7 load profile
    const uint32_t maxConnAttempted = mConfig.Common.Load.LoadProfile.MaxConnectionsAttempted;
    if (maxConnAttempted)
        count = std::min(maxConnAttempted - mAttemptedConnCount, count);
    const uint32_t maxOpenConn = mConfig.Common.Load.LoadProfile.MaxOpenConnections;
    if (maxOpenConn)
        count = std::min(maxOpenConn - mActiveConnCount, count);
    uint32_t dstIp = mConfig.ProtocolProfile.ServerIpv4Addr.address[0] << 24 |
                     mConfig.ProtocolProfile.ServerIpv4Addr.address[1] << 16 |
                     mConfig.ProtocolProfile.ServerIpv4Addr.address[2] << 8 |
                     mConfig.ProtocolProfile.ServerIpv4Addr.address[3];
    ACE_INET_Addr dstAddr(mConfig.ProtocolProfile.ServerPort, dstIp);
        
    if (mRegProcess)
    {
        count = std::min(mMaxClients - mRegConnSpawned, count);
        mRegConnSpawned += count;
    }

    count = count - mPendingConnCount > 0 ? count - mPendingConnCount : 0;
    size_t countDecr = 1;
    for (; count != 0; count -= countDecr)
    {
        // Get the next endpoint pair in our block
        std::string srcIfName;
        ACE_INET_Addr srcAddr;
        mEndpointEnum->GetInterface(srcIfName, srcAddr);
        mEndpointEnum->Next();

        // Temporary block scope for connector access
        ClientConnectionHandler *rawConnHandler = 0;
        int err;

        {
            ACE_GUARD(connLock_t, guard, mConnLock);

            // Instruct the connector to bind the next connection to our source interface
            mConnector->BindToIfName(&srcIfName);

            // Avoid the string operations if the log level is < INFO
            if ((log4cplus_table[mPort].LevelMask & PHX_LOG_INFO_FLAG) == PHX_LOG_INFO_FLAG)
            {
                char srcAddrStr[64], dstAddrStr[64];

                if (srcAddr.addr_to_string(srcAddrStr, sizeof(srcAddrStr)) == -1)
                    strcpy(srcAddrStr, "invalid");

                if (dstAddr.addr_to_string(dstAddrStr, sizeof(dstAddrStr)) == -1)
                    strcpy(dstAddrStr, "invalid");
    
                TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "XMPP client block [" << Name() << "] connecting " << srcAddrStr << "->" << dstAddrStr << " using " << srcIfName);
            }

            err = mConnector->MakeHandler(rawConnHandler);
            if (rawConnHandler)
            {
                // Recalculate the load over all the connections, including the new one
                DynamicLoadHandler(mActiveConnCount + 1, GetDynamicLoadTotal());
                RegisterDynamicLoadDelegates(rawConnHandler);
                rawConnHandler->EnableBwCtrl(GetEnableBwCtrl());
            }
            // Initiate a new connection         
            err = mConnector->connect(rawConnHandler, dstAddr, ACE_Synch_Options(ACE_Synch_Options::USE_REACTOR), srcAddr, 1);
        }
        // Bump attempted connections counter
        mAttemptedConnCount++;  
        {
            ACE_GUARD(stats_t::lock_t, guard, mStats.Lock());
            mStats.attemptedConnections++;
        }
        
        // If the connection attempt failed right away then we must treat this as though the connection was closed
        if (err == -1 && errno != EWOULDBLOCK)
        {
            if (rawConnHandler)
                rawConnHandler->remove_reference();

            // Avoid the string operations if the log level is < INFO
            if ((log4cplus_table[mPort].LevelMask & PHX_LOG_INFO_FLAG) == PHX_LOG_INFO_FLAG)
            {
                char srcAddrStr[64], dstAddrStr[64], errorStr[64];

                if (srcAddr.addr_to_string(srcAddrStr, sizeof(srcAddrStr)) == -1)
                    strcpy(srcAddrStr, "invalid");

                if (dstAddr.addr_to_string(dstAddrStr, sizeof(dstAddrStr)) == -1)
                    strcpy(dstAddrStr, "invalid");

                TC_LOG_INFO_LOCAL(mPort, LOG_CLIENT, "XMPP client block [" << Name() << "] failed to connect " << srcAddrStr << "->" << dstAddrStr << ": " << strerror_r(errno, errorStr, sizeof(errorStr)));
            }

            mPendingConnCount++;

            countDecr = 1;
        }
        else
        {
            mActiveConnCount++;

            {
                ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
                mStats.activeConnections = mActiveConnCount;
            }

            // Insert the connection handler into our map and aging queue
            // NOTE: Store the connection handler in a shared pointer with a custom deleter -- this will call remove_reference() when the map entry is erased
            connHandlerSharedPtr_t connHandler(rawConnHandler, boost::bind(&ClientConnectionHandler::remove_reference, _1));
            mConnHandlerMap[rawConnHandler->Serial()] = connHandler;
            mConnHandlerAgingQueue.push(rawConnHandler->Serial());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::ReapConnections(size_t count)
{
    count = std::min(mActiveConnCount, count);
    
    // Close up to the specified number of connections, oldest first
    size_t closed = 0;
    while (!mConnHandlerAgingQueue.empty() && closed < count)
    {
        const uint32_t connHandlerSerial = mConnHandlerAgingQueue.front();
        mConnHandlerAgingQueue.pop();

        connHandlerMap_t::iterator iter = mConnHandlerMap.find(connHandlerSerial);
        if (iter == mConnHandlerMap.end())
            continue;

        connHandlerSharedPtr_t& connHandler(iter->second);

        // Remove the handler's close notification delegate (informing ourselves of a close that is self-initiated creates unnecessary overhead)
        connHandler->ClearCloseNotificationDelegate();
              
        // If this connection is pending, cancel it in the connector
        if (connHandler->IsPending())
        {
            if (mConnector)
            {
                ACE_GUARD(connLock_t, guard, mConnLock);
                mConnector->cancel(connHandler.get());
            }

            // bump aborted connections counter if it's really not connected
            if (!connHandler->IsConnected())
            {
                ACE_GUARD(stats_t::lock_t, guard, mStats.Lock());
                if (!connHandler->IsComplete())
                {
                    // didn't already flag this as [un]successful
                    mStats.abortedConnections++;
                    connHandler->SetComplete(true);
                }
            }
            else
            {
                ACE_GUARD(stats_t::lock_t, guard, mStats.Lock());
                if (!connHandler->IsComplete())
                {
                    // didn't already flag this as [un]successful
                    mStats.successfulConnections++;
                    connHandler->SetComplete(true);
                }
            }            
        }

        //logout the client before closing the connection
        ACE_Time_Value now = ACE_OS::gettimeofday();
        connHandler->InitiateLogout(now);

        // Close the connection
        UnregisterDynamicLoadDelegates(connHandler.get());
        if (!connHandler->registerClient())
        {
            mLoginIndexTracker->Release(connHandler->GetClientIndex());
        }
        connHandler->close();
        mActiveConnCount--;

        // Delete the connection handler from our map
        mConnHandlerMap.erase(iter);
        closed++;

        {
            ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
            mStats.activeConnections = mActiveConnCount;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::DynamicLoadHandler(size_t conn, int32_t dynamicLoad)
{
    int32_t load = ConfigDynamicLoad(conn, dynamicLoad);
    if (load < 0)
        return;

    if (load == 0)
        ReapConnections(mActiveConnCount);

    ACE_GUARD(stats_t::lock_t, stats_guard, mStats.Lock());
    mStats.intendedLoad = static_cast<uint32_t>(load);
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::HandleAppLogicMessage(const AppLogicMessage& msg)
{
    switch (msg.type)
    {
      case AppLogicMessage::CLOSE_NOTIFICATION:
      {
          // Notify the load strategy that a connection was closed
          if (mLoadScheduler->Running())
          {
              mLoadStrategy->ConnectionClosed();
          }
/*          if (mRegStrategy->Running())
              mRegStrategy->RegistrationCompleted();
          TC_LOG_ERR(0,"[XmppClientBlock::HandleAppLogicMessage] 3");*/

          break;
      }

      default:
          throw EPHXInternal();
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::LoadProfileHandler(bool running) const
{
    // Cascade notification
    if (!mLoadProfileNotifier.empty())
        mLoadProfileNotifier(BllHandle(), running);
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::SetClientLogin(ClientConnectionHandler& connHandler, uint32_t index)
{
    uint32_t unID = mConfig.ProtocolConfig.UserNameNumStart + (mConfig.ProtocolConfig.UserNameNumStep*index);
    uint32_t pwID = mConfig.ProtocolConfig.PasswordNumStart + (mConfig.ProtocolConfig.PasswordNumStep*index);
    char un[1024];
    char pw[1024];

    memset(un,'\0',sizeof(un));
    memset(pw,'\0',sizeof(pw));

    std::sprintf(un,mConfig.ProtocolConfig.UserNameFormat.c_str(),unID);
    std::sprintf(pw,mConfig.ProtocolConfig.PasswordFormat.c_str(),pwID);

    std::string username(un);
    std::string password(pw);
    
    connHandler.SetUsername(username);
    connHandler.SetPassword(password);
    connHandler.SetClientIndex(index);
}


bool XmppClientBlock::HandleConnectionNotification(ClientConnectionHandler& connHandler)
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    // Since we're likely to be sinking a lot of data from the server, increase the input block size up to SO_RCVBUF
    size_t soRcvBuf;
    if (!connHandler.GetSoRcvBuf(soRcvBuf))
        return false;

    connHandler.SetInputBlockSize(soRcvBuf);
    connHandler.SetServerDomain(mConfig.ProtocolProfile.ServerDomain);
    connHandler.SetStatsInstance(mStats);
    connHandler.SetAutoGenID(mConfig.ProtocolConfig.AutoGenUserInfo);

    //if registering or unregistering, set username and pwd and register or unregister
    if (mRegProcess)
    {
        if (this->IsRegistering())
            ClientRegister(connHandler);
        else
            ClientUnRegister(connHandler);
    } else { //standard protocol start
        // Initialize the protocol-related aspects of the connection handler
        connHandler.SetSessionType(mConfig.ProtocolProfile.SessionType);
        connHandler.SetClientMode(mConfig.ProtocolProfile.ClientMode);
        connHandler.SetSessionDuration(mConfig.ProtocolProfile.SessionDuration);
        connHandler.SetTimedPubSubSessionRampType(mConfig.ProtocolProfile.TimedPubSubSessionRampType);
        connHandler.SetPubInterval(mConfig.ProtocolProfile.PubInterval);
        connHandler.SetFirstInterval(mConfig.ProtocolProfile.FirstPubMsgInterval);
        connHandler.SetPubItemId(mConfig.ProtocolProfile.PubItemId);
        connHandler.SetPubCapabilities(mPubCapabilities);
//        connHandler.SetPubFormat(mPubFormat);
        connHandler.SetClientType(ClientConnectionHandler::CLIENT_LOGIN);
        uint32_t clientIndex = mLoginIndexTracker->Assign(); //mIndex++; //mLoginIndexTracker->Assign();   
        connHandler.SetClientIndex(clientIndex);
        connHandler.SetStatusDelegate(boost::bind(&XmppClientBlock::NotifyXmppClientStatus, this, _1, _2, _3));
        if(!mConfig.ProtocolConfig.AutoGenUserInfo)
        {
            SetClientLogin(connHandler, clientIndex);
        }
    }
      
    // Bump successful connections counter
    {
        ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats.Lock(), false);
        if (!connHandler.IsComplete())
        {
            mStats.successfulConnections++;
            connHandler.SetComplete(true);
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void XmppClientBlock::HandleCloseNotification(L4L7_APP_NS::StreamSocketHandler& socketHandler)
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    ClientConnectionHandler *connHandler = reinterpret_cast<ClientConnectionHandler *>(&socketHandler);

    if (!connHandler->registerClient()&& connHandler->IsComplete()) 
    {
        mLoginIndexTracker->Release(connHandler->GetClientIndex());
    }
	if(connHandler->isRegistering())
		connHandler->NotifyRegistrationCompleted(false,true);

    if (mRegProcess && connHandler->IsComplete() && connHandler->GetClientType() == ClientConnectionHandler::CLIENT_UNREGISTER)
        --mRegOutstanding;
    if (mRegProcess && !connHandler->IsComplete())
        --mRegConnSpawned;

    // If this connection is pending, cancel it in the connector
    if (connHandler->IsPending())
    {
        ACE_GUARD(connLock_t, guard, mConnLock);
        if (mConnector)
            mConnector->cancel(connHandler);
    }

    // We don't use an "else if" here because that misses the case
    // where a connection immediately fails (e.g. no server) 

    if (!connHandler->IsOpen())
    {
        if (connHandler->IsConnected())
        {
            // Wasn't "open" but it was connected. Mark it successful here
            ACE_GUARD(stats_t::lock_t, guard, mStats.Lock());
            if (!connHandler->IsComplete())
            {
                mStats.successfulConnections++;
                connHandler->SetComplete(true);
            }
        }
        else
        {
            // Not open at all, bump unsuccessful connections counter
            ACE_GUARD(stats_t::lock_t, guard, mStats.Lock());
            if (!connHandler->IsComplete())
            {
                mStats.unsuccessfulConnections++;
                connHandler->SetComplete(true);
            }
        }
    }

    // Notify the I/O logic that a connection was closed
    {
        ioMessageInterface_t::messagePtr_t msgPtr(mIOLogicInterface->Allocate());
        IOLogicMessage& msg(*msgPtr);
        msg.type = IOLogicMessage::CLOSE_NOTIFICATION;
        msg.connHandlerSerial = connHandler->Serial();
        mIOLogicInterface->Send(msgPtr);
    }

    // Notify the application logic that a connection was closed
    {
        appMessageInterface_t::messagePtr_t msgPtr(mAppLogicInterface->Allocate());
        AppLogicMessage& msg(*msgPtr);
        msg.type = AppLogicMessage::CLOSE_NOTIFICATION;
        mAppLogicInterface->Send(msgPtr);
    }

}

///////////////////////////////////////////////////////////////////////////////
