#include <memory>
#include <string>
#include <sys/resource.h>

#include <ace/Signal.h>
#include <ildaemon/ActiveScheduler.h>
#include <ildaemon/BoundMethodRequestImpl.tcc>
#include <base/BaseCommon.h>
#include <boost/bind.hpp>
#include <vif/IfSetupCommon.h>
#include <phxlog/phxlog.h>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "DpgTestUtils.h"
#include "DpgApplication.h"

#undef EXTENSIVE_UNIT_TESTS

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestDpgApplication : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestDpgApplication);
    CPPUNIT_TEST(testOneServerManyClients);
#ifdef EXTENSIVE_UNIT_TESTS
    CPPUNIT_TEST(testNoServerManyClients);
    CPPUNIT_TEST(testOneServerManyClientsWithSingleThread);
    CPPUNIT_TEST(testNoServerManyClientsWithSingleThread);
    CPPUNIT_TEST(testOneServerManyClientsWithSinusoid);
    CPPUNIT_TEST(testNoServerManyClientsWithSinusoid);
#endif
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void);
    void tearDown(void) { }
    
protected:
    static const size_t NUM_IO_THREADS = 4;
    static const rlim_t MAXIMUM_OPEN_FILE_HARD_LIMIT;
    
    void testOneServerManyClients(void);
    void testNoServerManyClients(void);
    void testOneServerManyClientsWithSingleThread(void);
    void testNoServerManyClientsWithSingleThread(void);
    void testOneServerManyClientsWithSinusoid(void);
    void testNoServerManyClientsWithSinusoid(void);
};

///////////////////////////////////////////////////////////////////////////////

const rlim_t TestDpgApplication::MAXIMUM_OPEN_FILE_HARD_LIMIT = (32 * 1024);

///////////////////////////////////////////////////////////////////////////////

void TestDpgApplication::setUp(void)
{
    ACE_OS::signal(SIGPIPE, SIG_IGN);

    struct rlimit fileLimits;
    getrlimit(RLIMIT_NOFILE, &fileLimits);
    if (fileLimits.rlim_cur < MAXIMUM_OPEN_FILE_HARD_LIMIT || fileLimits.rlim_max < MAXIMUM_OPEN_FILE_HARD_LIMIT)
    {
        fileLimits.rlim_cur = std::max(fileLimits.rlim_cur, MAXIMUM_OPEN_FILE_HARD_LIMIT);
        fileLimits.rlim_max = std::max(fileLimits.rlim_max, MAXIMUM_OPEN_FILE_HARD_LIMIT);
        setrlimit(RLIMIT_NOFILE, &fileLimits);
    }
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgApplication::testOneServerManyClients(void)
{
    PHX_LOG_REDIRECTLOG_INIT("", "0x1-MPS,0x2-Client,0x4-Server,0x8-Socket,0x10-Unused", LOG_UPTO(PHX_LOG_DEBUG), PHX_LOCAL_ID_1 | PHX_LOCAL_ID_2 | PHX_LOCAL_ID_3 | PHX_LOCAL_ID_4);

    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    DpgApplication::handleList_t serverHandles;
    DpgApplication::handleList_t clientHandles;
	DpgApplication app(1);

    // Start application
    CPPUNIT_ASSERT(scheduler.Start(1, NUM_IO_THREADS) == 0);

    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure server block
    DpgApplication::serverConfig_t serverConfig;

    DpgTestUtils::InitCommonProfile(&serverConfig.Common.Profile.L4L7Profile, "TestDpgApplication");
    serverConfig.Common.Endpoint.SrcIfHandle = 0;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 1;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestDpgApplication";
    serverConfig.ProtocolConfig.MaxSimultaneousClients = 0;

    {
        const DpgApplication::serverConfigList_t servers(1, serverConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::ConfigServers, &app, 0, boost::cref(servers), boost::ref(serverHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(serverHandles.size() == 1);
    CPPUNIT_ASSERT(serverHandles[0] == serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle);
    
    // Configure client block
    DpgApplication::clientConfig_t clientConfig;

    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_PLAYLISTS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;
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
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.SteadyTime = 60;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.RampTime = 30;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.SteadyTime = 10;
    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestDpgApplication";
    clientConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    clientConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    clientConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    clientConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    DpgTestUtils::InitClientEndpoint(&clientConfig.Common.Endpoint);
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestDpgApplication";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.MaxTransactionsPerServer = 16;

    {
        const DpgApplication::clientConfigList_t clients(1, clientConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(clientHandles.size() == 1);
    CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);

    for (size_t run = 0; run < 1; run++)
    {
        // Start the server and client blocks
        {
            DpgApplication::handleList_t handles;

            handles.push_back(serverHandles[0]);
            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::StartProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }

        // Let the blocks run for a bit
        sleep(120);

        // Stop the server and client blocks
        {
            DpgApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
            handles.push_back(serverHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
    }
    
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgApplication::testNoServerManyClients(void)
{   
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    DpgApplication::handleList_t clientHandles;
	DpgApplication app(1);

    // Start application
    CPPUNIT_ASSERT(scheduler.Start(1, NUM_IO_THREADS) == 0);

    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure client block
    DpgApplication::clientConfig_t clientConfig;

    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_PLAYLISTS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;
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
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.SteadyTime = 60;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.RampTime = 30;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.SteadyTime = 10;
    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestDpgApplication";
    clientConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    clientConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    clientConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    clientConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    DpgTestUtils::InitClientEndpoint(&clientConfig.Common.Endpoint);
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestDpgApplication";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.MaxTransactionsPerServer = 16;

    {
        const DpgApplication::clientConfigList_t clients(1, clientConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(clientHandles.size() == 1);
    CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);

    for (size_t run = 0; run < 1; run++)
    {
        // Start the server and client blocks
        {
            DpgApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::StartProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }

        // Let the blocks run for a bit
        sleep(120);

        // Stop the server and client blocks
        {
            DpgApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
    }
    
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgApplication::testOneServerManyClientsWithSingleThread(void)
{
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    DpgApplication::handleList_t serverHandles;
    DpgApplication::handleList_t clientHandles;
	DpgApplication app(1);

    // Start application
    CPPUNIT_ASSERT(scheduler.Start(1, 1) == 0);

    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure server block
    DpgApplication::serverConfig_t serverConfig;

    DpgTestUtils::InitCommonProfile(&serverConfig.Common.Profile.L4L7Profile, "TestDpgApplication");
    serverConfig.Common.Endpoint.SrcIfHandle = 0;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 1;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestDpgApplication";
    serverConfig.ProtocolConfig.MaxSimultaneousClients = 0;

    {
        const DpgApplication::serverConfigList_t servers(1, serverConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::ConfigServers, &app, 0, boost::cref(servers), boost::ref(serverHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(serverHandles.size() == 1);
    CPPUNIT_ASSERT(serverHandles[0] == serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle);
    
    // Configure client block
    DpgApplication::clientConfig_t clientConfig;

    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_PLAYLISTS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;
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
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.SteadyTime = 60;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.RampTime = 30;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.SteadyTime = 10;
    DpgTestUtils::InitCommonProfile(&clientConfig.Common.Profile.L4L7Profile, "TestDpgApplication");
    DpgTestUtils::InitClientEndpoint(&clientConfig.Common.Endpoint);
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestDpgApplication";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.MaxTransactionsPerServer = 16;

    {
        const DpgApplication::clientConfigList_t clients(1, clientConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(clientHandles.size() == 1);
    CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);

    for (size_t run = 0; run < 1; run++)
    {
        // Start the server and client blocks
        {
            DpgApplication::handleList_t handles;

            handles.push_back(serverHandles[0]);
            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::StartProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }

        // Let the blocks run for a bit
        sleep(120);

        // Stop the server and client blocks
        {
            DpgApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
            handles.push_back(serverHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
    }
    
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgApplication::testNoServerManyClientsWithSingleThread(void)
{   
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    DpgApplication::handleList_t clientHandles;
	DpgApplication app(1);

    // Start application
    CPPUNIT_ASSERT(scheduler.Start(1, 1) == 0);

    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure client block
    DpgApplication::clientConfig_t clientConfig;

    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_PLAYLISTS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;
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
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.SteadyTime = 60;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.RampTime = 30;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.SteadyTime = 10;
    DpgTestUtils::InitCommonProfile(&clientConfig.Common.Profile.L4L7Profile, "TestDpgApplication");
    DpgTestUtils::InitClientEndpoint(&clientConfig.Common.Endpoint);
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestDpgApplication";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.MaxTransactionsPerServer = 16;

    {
        const DpgApplication::clientConfigList_t clients(1, clientConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(clientHandles.size() == 1);
    CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);

    for (size_t run = 0; run < 1; run++)
    {
        // Start the server and client blocks
        {
            DpgApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::StartProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }

        // Let the blocks run for a bit
        sleep(120);

        // Stop the server and client blocks
        {
            DpgApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
    }
    
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgApplication::testOneServerManyClientsWithSinusoid(void)
{ 
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    DpgApplication::handleList_t serverHandles;
    DpgApplication::handleList_t clientHandles;
	DpgApplication app(1);

    // Start application
    CPPUNIT_ASSERT(scheduler.Start(1, NUM_IO_THREADS) == 0);

    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure server block
    DpgApplication::serverConfig_t serverConfig;

    DpgTestUtils::InitCommonProfile(&serverConfig.Common.Profile.L4L7Profile, "TestDpgApplication");
    serverConfig.Common.Endpoint.SrcIfHandle = 0;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 1;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestDpgApplication";
    serverConfig.ProtocolConfig.MaxSimultaneousClients = 0;

    {
        const DpgApplication::serverConfigList_t servers(1, serverConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::ConfigServers, &app, 0, boost::cref(servers), boost::ref(serverHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(serverHandles.size() == 1);
    CPPUNIT_ASSERT(serverHandles[0] == serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle);
    
    // Configure client block
    DpgApplication::clientConfig_t clientConfig;

    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_PLAYLISTS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;
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
    clientConfig.Common.Load.LoadPhases[2].SinusoidDescriptor.Repetitions = 6;
    clientConfig.Common.Load.LoadPhases[2].SinusoidDescriptor.Period = 10;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.RampTime = 1;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.SteadyTime = 19;
    DpgTestUtils::InitCommonProfile(&clientConfig.Common.Profile.L4L7Profile, "TestDpgApplication");
    DpgTestUtils::InitClientEndpoint(&clientConfig.Common.Endpoint);
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestDpgApplication";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.MaxTransactionsPerServer = 16;

    {
        const DpgApplication::clientConfigList_t clients(1, clientConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(clientHandles.size() == 1);
    CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);

    for (size_t run = 0; run < 1; run++)
    {
        // Start the server and client blocks
        {
            DpgApplication::handleList_t handles;

            handles.push_back(serverHandles[0]);
            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::StartProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }

        // Let the blocks run for a bit
        sleep(115);

        // Stop the server and client blocks
        {
            DpgApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
            handles.push_back(serverHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
    }
    
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgApplication::testNoServerManyClientsWithSinusoid(void)
{  
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    DpgApplication::handleList_t clientHandles;
	DpgApplication app(1);

    // Start application
    CPPUNIT_ASSERT(scheduler.Start(1, NUM_IO_THREADS) == 0);

    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure client block
    DpgApplication::clientConfig_t clientConfig;

    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_PLAYLISTS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;
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
    clientConfig.Common.Load.LoadPhases[2].SinusoidDescriptor.Repetitions = 6;
    clientConfig.Common.Load.LoadPhases[2].SinusoidDescriptor.Period = 10;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.RampTime = 1;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.SteadyTime = 19;
    DpgTestUtils::InitCommonProfile(&clientConfig.Common.Profile.L4L7Profile, "TestDpgApplication");
    DpgTestUtils::InitClientEndpoint(&clientConfig.Common.Endpoint);
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestDpgApplication";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.MaxTransactionsPerServer = 16;

    {
        const DpgApplication::clientConfigList_t clients(1, clientConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(clientHandles.size() == 1);
    CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);

    for (size_t run = 0; run < 1; run++)
    {
        // Start the server and client blocks
        {
            DpgApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::StartProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }

        // Let the blocks run for a bit
        sleep(115);

        // Stop the server and client blocks
        {
            DpgApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
    }
    
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestDpgApplication);
