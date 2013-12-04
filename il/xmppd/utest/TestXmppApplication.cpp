#include <memory>
#include <string>
#include <sys/resource.h>

#include <ace/Signal.h>
#include <ildaemon/ActiveScheduler.h>
#include <ildaemon/BoundMethodRequestImpl.tcc>
#include <base/BaseCommon.h>
#include <boost/bind.hpp>
#include <vif/IfSetupCommon.h>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "XmppApplication.h"
#include "XmppCommon.h"

USING_XMPP_NS;

class TestXmppApplication : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestXmppApplication);
//    CPPUNIT_TEST(testRegistrationRetries);
    CPPUNIT_TEST(testNoServerManyClients);
    CPPUNIT_TEST(testNoServerManyClientsWithSingleThread);
    CPPUNIT_TEST(testNoServerManyClientsWithSinusoid);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void);
    void tearDown(void) { }

protected:
    static const size_t NUM_IO_THREADS = 4;
//    void testRegistrationRetries(void);
    void testNoServerManyClients(void);
    void testNoServerManyClientsWithSingleThread(void);
    void testNoServerManyClientsWithSinusoid(void);
};

///////////////////////////////////////////////////////////////////////////////

void TestXmppApplication::setUp(void)
{
    ACE_OS::signal(SIGPIPE, SIG_IGN);
}

///////////////////////////////////////////////////////////////////////////////

void TestXmppApplication::testNoServerManyClients(void)
{
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    XmppApplication::handleList_t clientHandles;
	XmppApplication app(1);

    CPPUNIT_ASSERT(scheduler.Start(1, NUM_IO_THREADS) == 0);

    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));

        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }

    XmppApplication::clientConfig_t clientConfig;

    //Configure clients
    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_CONNECTIONS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.UseDynamicLoad = true;
    clientConfig.Common.Load.LoadPhases.resize(4);
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 10;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.Height = 300;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.RampTime = 10;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.SteadyTime = 0;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.Height = 300;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.SteadyTime = 30;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.RampTime = 30;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.SteadyTime = 5;
    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestXmppApplication";
    clientConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    clientConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    clientConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    clientConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    clientConfig.Common.Endpoint.SrcIfHandle = 0;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors.resize(1);
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].ifType = IFSETUP_NS::STC_DM_IFC_IPv4;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].indexInList = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList.resize(1);
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsRange = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsDirectlyConnected = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfCountPerLowerIf = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfRecycleCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.TotalCount = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.BllHandle = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.AffiliatedInterface = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[0] = 127;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[3] = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[0] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[3] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].SkipReserved = false;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrRepeatCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].PrefixLength = 8;
    clientConfig.ProtocolProfile.SessionType = xmppvj_1_port_server::EnumSessionType_LOGIN_ONLY;
    clientConfig.ProtocolProfile.ClientMode = xmppvj_1_port_server::EnumClientMode_PUB;
    clientConfig.ProtocolProfile.ServerDomain = "SJC-6DNFLN1";
   // clientConfig.ProtocolProfile.ServerIpv4Addr = "0.0.0.0";
   // clientConfig.ProtocolProfile.ServerIpv6Addr = "::";
    clientConfig.ProtocolProfile.SessionDuration = 300;
    clientConfig.ProtocolProfile.TimedPubSubSessionRampType = xmppvj_1_port_server::EnumTimedPubSubSessionRamp_RAMP_UP;
    clientConfig.ProtocolProfile.PubInterval = 300;
    clientConfig.ProtocolProfile.PubItemId = "@jid";
    clientConfig.ProtocolProfile.FirstPubMsgInterval = 300;
    clientConfig.ProtocolProfile.RegsPerSecond = 10;
    clientConfig.ProtocolProfile.RegBurstSize = 1;
    clientConfig.ProtocolProfile.RegRetries = 3;
    clientConfig.ProtocolProfile.ServerPort = 5222;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestXmppApplication";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.AutoGenUserInfo = false;
//    clientConfig.ProtocolConfig.RegistrationState = 0; //xmppvj_1_port_server::EnumCiscoXmppRegistrationState_NOT_REGISTERED;
    clientConfig.ProtocolConfig.PubCapabilities = "";
    clientConfig.ProtocolConfig.UserNameNumStart = 0;
    clientConfig.ProtocolConfig.UserNameNumStep = 1;
    clientConfig.ProtocolConfig.UserNameFormat = "testXmppApplication%u";
    clientConfig.ProtocolConfig.PasswordNumStart = 0;
    clientConfig.ProtocolConfig.PasswordNumStep = 1;
    clientConfig.ProtocolConfig.PasswordFormat = "password%u";
    clientConfig.ProtocolConfig.ClientNumsPerDevice = 1;

    {
        const XmppApplication::clientConfigList_t clients(1, clientConfig);

        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    //check client handles and bll handle created
    CPPUNIT_ASSERT(clientHandles.size() == 1);
    CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);

    for (size_t run = 0; run < 1; run++)
    {
        {
            XmppApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);

            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::StartProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }

        sleep(60);

        {
            XmppApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);

            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
    }

    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestXmppApplication::testNoServerManyClientsWithSingleThread(void)
{
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    XmppApplication::handleList_t clientHandles;
	XmppApplication app(1);

    CPPUNIT_ASSERT(scheduler.Start(1, 1) == 0);

    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));

        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }

    XmppApplication::clientConfig_t clientConfig;

    //Configure clients
    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_CONNECTIONS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.UseDynamicLoad = true;
    clientConfig.Common.Load.LoadPhases.resize(4);
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 10;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.Height = 300;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.RampTime = 10;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.SteadyTime = 0;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.Height = 300;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.SteadyTime = 30;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.RampTime = 30;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.SteadyTime = 5;
    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestXmppApplication";
    clientConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    clientConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    clientConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    clientConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    clientConfig.Common.Endpoint.SrcIfHandle = 0;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors.resize(1);
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].ifType = IFSETUP_NS::STC_DM_IFC_IPv4;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].indexInList = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList.resize(1);
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsRange = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsDirectlyConnected = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfCountPerLowerIf = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfRecycleCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.TotalCount = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.BllHandle = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.AffiliatedInterface = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[0] = 127;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[3] = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[0] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[3] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].SkipReserved = false;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrRepeatCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].PrefixLength = 8;
    clientConfig.ProtocolProfile.SessionType = xmppvj_1_port_server::EnumSessionType_LOGIN_ONLY;
    clientConfig.ProtocolProfile.ClientMode = xmppvj_1_port_server::EnumClientMode_PUB;
    clientConfig.ProtocolProfile.ServerDomain = "SJC-6DNFLN1";
   // clientConfig.ProtocolProfile.ServerIpv4Addr = "0.0.0.0";
   // clientConfig.ProtocolProfile.ServerIpv6Addr = "::";
    clientConfig.ProtocolProfile.SessionDuration = 300;
    clientConfig.ProtocolProfile.TimedPubSubSessionRampType = xmppvj_1_port_server::EnumTimedPubSubSessionRamp_RAMP_UP;
    clientConfig.ProtocolProfile.PubInterval = 300;
    clientConfig.ProtocolProfile.PubItemId = "@jid";
    clientConfig.ProtocolProfile.FirstPubMsgInterval = 300;
    clientConfig.ProtocolProfile.RegsPerSecond = 10;
    clientConfig.ProtocolProfile.RegBurstSize = 1;
    clientConfig.ProtocolProfile.RegRetries = 3;
    clientConfig.ProtocolProfile.ServerPort = 5222;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestXmppApplication";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.AutoGenUserInfo = false;
//    clientConfig.ProtocolConfig.RegistrationState = 0; //xmppvj_1_port_server::EnumCiscoXmppRegistrationState_NOT_REGISTERED;
    clientConfig.ProtocolConfig.PubCapabilities = "";
    clientConfig.ProtocolConfig.UserNameNumStart = 0;
    clientConfig.ProtocolConfig.UserNameNumStep = 1;
    clientConfig.ProtocolConfig.UserNameFormat = "testXmppApplication%u";
    clientConfig.ProtocolConfig.PasswordNumStart = 0;
    clientConfig.ProtocolConfig.PasswordNumStep = 1;
    clientConfig.ProtocolConfig.PasswordFormat = "password%u";
    clientConfig.ProtocolConfig.ClientNumsPerDevice = 1;

    {
        const XmppApplication::clientConfigList_t clients(1, clientConfig);

        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    //check client handles and bll handle created
    CPPUNIT_ASSERT(clientHandles.size() == 1);
    CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);

    for (size_t run = 0; run < 1; run++)
    {
        {
            XmppApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);

            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::StartProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }

        sleep(60);

        {
            XmppApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);

            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
    }

    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestXmppApplication::testNoServerManyClientsWithSinusoid(void)
{
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    XmppApplication::handleList_t clientHandles;
	XmppApplication app(1);

    CPPUNIT_ASSERT(scheduler.Start(1, NUM_IO_THREADS) == 0);

    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));

        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }

    //Configure clients
    XmppApplication::clientConfig_t clientConfig;

    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_CONNECTIONS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.UseDynamicLoad = true;
    clientConfig.Common.Load.LoadPhases.resize(4);
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 10;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_STAIR;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[1].StairDescriptor.Height = 300;
    clientConfig.Common.Load.LoadPhases[1].StairDescriptor.Repetitions = 1;
    clientConfig.Common.Load.LoadPhases[1].StairDescriptor.RampTime = 30;
    clientConfig.Common.Load.LoadPhases[1].StairDescriptor.SteadyTime = 0;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_SINUSOID;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[2].SinusoidDescriptor.Height = 100;
    clientConfig.Common.Load.LoadPhases[2].SinusoidDescriptor.Repetitions = 3;
    clientConfig.Common.Load.LoadPhases[2].SinusoidDescriptor.Period = 5;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.RampTime = 1;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.SteadyTime = 10;
    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestXmppApplication";
    clientConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    clientConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    clientConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    clientConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    clientConfig.Common.Endpoint.SrcIfHandle = 0;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors.resize(1);
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].ifType = IFSETUP_NS::STC_DM_IFC_IPv4;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].indexInList = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList.resize(1);
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsRange = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsDirectlyConnected = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfCountPerLowerIf = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfRecycleCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.TotalCount = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.BllHandle = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.AffiliatedInterface = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[0] = 127;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[3] = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[0] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[3] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].SkipReserved = false;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrRepeatCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].PrefixLength = 8;
    clientConfig.ProtocolProfile.SessionType = xmppvj_1_port_server::EnumSessionType_LOGIN_ONLY;
    clientConfig.ProtocolProfile.ClientMode = xmppvj_1_port_server::EnumClientMode_PUB;
    clientConfig.ProtocolProfile.ServerDomain = "SJC-6DNFLN1";
   // clientConfig.ProtocolProfile.ServerIpv4Addr = "0.0.0.0";
   // clientConfig.ProtocolProfile.ServerIpv6Addr = "::";
    clientConfig.ProtocolProfile.SessionDuration = 300;
    clientConfig.ProtocolProfile.TimedPubSubSessionRampType = xmppvj_1_port_server::EnumTimedPubSubSessionRamp_RAMP_UP;
    clientConfig.ProtocolProfile.PubInterval = 300;
    clientConfig.ProtocolProfile.PubItemId = "@jid";
    clientConfig.ProtocolProfile.FirstPubMsgInterval = 300;
    clientConfig.ProtocolProfile.RegsPerSecond = 10;
    clientConfig.ProtocolProfile.RegBurstSize = 1;
    clientConfig.ProtocolProfile.RegRetries = 3;
    clientConfig.ProtocolProfile.ServerPort = 5222;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestXmppApplication";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.AutoGenUserInfo = false;
//    clientConfig.ProtocolConfig.RegistrationState = 0; //xmppvj_1_port_server::EnumCiscoXmppRegistrationState_NOT_REGISTERED;
    clientConfig.ProtocolConfig.PubCapabilities = "";
    clientConfig.ProtocolConfig.UserNameNumStart = 0;
    clientConfig.ProtocolConfig.UserNameNumStep = 1;
    clientConfig.ProtocolConfig.UserNameFormat = "testXmppApplication%u";
    clientConfig.ProtocolConfig.PasswordNumStart = 0;
    clientConfig.ProtocolConfig.PasswordNumStep = 1;
    clientConfig.ProtocolConfig.PasswordFormat = "password%u";
    clientConfig.ProtocolConfig.ClientNumsPerDevice = 1;

    {
        const XmppApplication::clientConfigList_t clients(1, clientConfig);

        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    //check client handles and bll handle created
    CPPUNIT_ASSERT(clientHandles.size() == 1);
    CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);

    for (size_t run = 0; run < 1; run++)
    {
        {
            XmppApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);

            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::StartProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }

        sleep(60);

        {
            XmppApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);

            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
    }

    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

//void TestXmppApplication::testRegistrationRetries(void)
//{
//    XmppApplication app(1);
//    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
//    XmppApplication::handleList_t clientHandles;
//
//    CPPUNIT_ASSERT(scheduler.Start(1, NUM_IO_THREADS) == 0);
//
//    {
//        ACE_Reactor *appReactor = scheduler.AppReactor();
//        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
//        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
//
//        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
//        scheduler.Enqueue(req);
//        req->Wait();
//    }
//
//    XmppApplication::clientConfig_t clientConfig;
//
//    //Configure clients
//    XmppApplication::clientConfig_t clientConfig;
//
//    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_CONNECTIONS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
//    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
//    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
//    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
//    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;
//    clientConfig.Common.Load.LoadProfile.UseDynamicLoad = true;
//    clientConfig.Common.Load.LoadPhases.resize(1);
//    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
//    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
//    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.Height = 1;
//    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 0;
//    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 10;
//    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestXmppApplication";
//    clientConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
//    clientConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
//    clientConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
//    clientConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
//    clientConfig.Common.Endpoint.SrcIfHandle = 0;
//    clientConfig.Common.Endpoint.DstIf.ifDescriptors.resize(1);
//    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].ifType = IFSETUP_NS::STC_DM_IFC_IPv4;
//    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].indexInList = 0;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList.resize(1);
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsRange = true;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsDirectlyConnected = true;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfCountPerLowerIf = 1;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfRecycleCount = 0;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.TotalCount = 1;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.BllHandle = 0;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.AffiliatedInterface = 0;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[0] = 127;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[1] = 0;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[2] = 0;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[3] = 1;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[0] = 0;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[1] = 0;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[2] = 0;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[3] = 0;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].SkipReserved = false;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrRepeatCount = 0;
//    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].PrefixLength = 8;
//    clientConfig.ProtocolProfile.SessionType = xmppvj_1_port_server::EnumSessionType_LOGIN_ONLY;
//    clientConfig.ProtocolProfile.ClientMode = xmppvj_1_port_server::EnumClientMode_PUB;
//    clientConfig.ProtocolProfile.ServerDomain = "SJC-6DNFLN1";
//   // clientConfig.ProtocolProfile.ServerIpv4Addr = "0.0.0.0";
//   // clientConfig.ProtocolProfile.ServerIpv6Addr = "::";
//    clientConfig.ProtocolProfile.SessionDuration = 300;
//    clientConfig.ProtocolProfile.TimedPubSubSessionRampType = xmppvj_1_port_server::EnumTimedPubSubSessionRamp_RAMP_UP;
//    clientConfig.ProtocolProfile.PubInterval = 300;
//    clientConfig.ProtocolProfile.PubItemId = "@jid";
//    clientConfig.ProtocolProfile.FirstPubMsgInterval = 300;
//    clientConfig.ProtocolProfile.RegsPerSecond = 10;
//    clientConfig.ProtocolProfile.RegBurstSize = 1;
//    clientConfig.ProtocolProfile.RegRetries = 3;
//    clientConfig.ProtocolProfile.ServerPort = 5222;
//    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
//    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestXmppApplication";
//    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
//    clientConfig.ProtocolConfig.AutoGenUserInfo = false;
//    clientConfig.ProtocolConfig.RegistrationState = 0; //xmppvj_1_port_server::EnumCiscoXmppRegistrationState_NOT_REGISTERED;
//    clientConfig.ProtocolConfig.PubCapabilities = "";
//    clientConfig.ProtocolConfig.UserNameNumStart = 0;
//    clientConfig.ProtocolConfig.UserNameNumStep = 1;
//    clientConfig.ProtocolConfig.UserNameFormat = "testXmppApplication%u";
//    clientConfig.ProtocolConfig.PasswordNumStart = 0;
//    clientConfig.ProtocolConfig.PasswordNumStep = 1;
//    clientConfig.ProtocolConfig.PasswordFormat = "password%u";
//    clientConfig.ProtocolConfig.ClientNumsPerDevice = 1;
//
//    {
//        const XmppApplication::clientConfigList_t clients(1, clientConfig);
//
//        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
//        scheduler.Enqueue(req);
//        req->Wait();
//    }
//
//    //check client handles and bll handle created
//    CPPUNIT_ASSERT(clientHandles.size() == 1);
//    CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);
//
//    for (size_t run = 0; run < 1; run++)
//    {
//        {
//            XmppApplication::handleList_t handles;
//
//            handles.push_back(clientHandles[0]);
//
//            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::RegisterXmppClients, &app, 0, boost::cref(handles));
//            scheduler.Enqueue(req);
//            req->Wait();
//        }
//
//        sleep(120);
//
//        {
//            XmppApplication::handleList_t handles;
//
//            handles.push_back(clientHandles[0]);
//
//            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::CancelXmppClientsRegistrations, &app, 0, boost::cref(handles));
//            scheduler.Enqueue(req);
//            req->Wait();
//        }
//    }
//
//    {
//        XmppApplication::handleList_t handles;
//        handles.push_back(clientHandles[0]);
//
//        std::vector<XmppClientRawStats> stats;
//        app.GetXmppClientStats(0, handles, stats);
//
//        CPPUNIT_ASSERT(stats.size() == 1);
//        CPPUNIT_ASSERT(stats[0].Handle() == handles[0]);
//        CPPUNIT_ASSERT(stats[0].regAttempts == clientConfig.ProtocolProfile.RegRetries);
//        CPPUNIT_ASSERT(stats[0].
//    }
//
//    {
//        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &XmppApplication::Deactivate, &app);
//        scheduler.Enqueue(req);
//        req->Wait();
//    }
//    CPPUNIT_ASSERT(scheduler.Stop() == 0);
//}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestXmppApplication);
