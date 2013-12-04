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

class TestCifs : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestCifs);
//    CPPUNIT_TEST(testOneServerOneClientAttackLoop);
    CPPUNIT_TEST(testCifsOneMessageType);
//    CPPUNIT_TEST(testOneServerManyClients);
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
    static const size_t NUM_IO_THREADS = 1; // 1; //4;
    static const rlim_t MAXIMUM_OPEN_FILE_HARD_LIMIT;
    
    void testOneServerOneClientAttackLoop(void);
    void testCifsOneMessageType();
    void testOneServerManyClients(void);
    void testOneServerManyClientsBody(bool isTcp, bool isStream);
    void testNoServerManyClients(void);
    void testOneServerManyClientsWithSingleThread(void);
    void testNoServerManyClientsWithSingleThread(void);
    void testOneServerManyClientsWithSinusoid(void);
    void testNoServerManyClientsWithSinusoid(void);
};

///////////////////////////////////////////////////////////////////////////////

const rlim_t TestCifs::MAXIMUM_OPEN_FILE_HARD_LIMIT = (32 * 1024);

///////////////////////////////////////////////////////////////////////////////

void TestCifs::setUp(void)
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

void TestCifs::testOneServerOneClientAttackLoop()
{
//    PHX_LOG_REDIRECTLOG_INIT("", "0x1-MPS,0x2-Client,0x4-Server,0x8-Socket,0x10-Unused", LOG_UPTO(PHX_LOG_DEBUG), PHX_LOCAL_ID_1 | PHX_LOCAL_ID_2 | PHX_LOCAL_ID_3 | PHX_LOCAL_ID_4);

    bool isTcp = true, isStream = false;
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

    // Configure flow
    uint32_t flowHandle = 8;
    DpgApplication::flowConfig_t flowConfig;
    flowConfig.Data.set_size(40);
    for (size_t i = 0; i < 40; ++i) flowConfig.Data[i] = i;
    flowConfig.PktLen.resize(4, 10);
    flowConfig.PktDelay.resize(4, 10);
    flowConfig.ClientTx.resize(4, true);
    flowConfig.ClientTx[1] = false;
    flowConfig.ClientTx[3] = false;
    flowConfig.Layer = isTcp ? 
        DpgIf_1_port_server::LAYER_TCP : DpgIf_1_port_server::LAYER_UDP;
    flowConfig.Close = DpgIf_1_port_server::CLOSE_TYPE_S_FIN;
    flowConfig.Type  = isStream ? 
        DpgIf_1_port_server::PLAY_TYPE_STREAM : DpgIf_1_port_server::PLAY_TYPE_ATTACK;
    
#if 0
    // Loop everything 5 times
    flowConfig.Loops.resize(1);
    flowConfig.Loops[0].BegIdx = 0;
    flowConfig.Loops[0].EndIdx = 3;
    flowConfig.Loops[0].Count  = 5;
#endif

    // FIXME add variables?

    {
        const DpgApplication::flowConfigList_t flows(1, flowConfig);
        std::vector<uint32_t> flowHandles(1, flowHandle);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::UpdateFlows, &app, 0, boost::cref(flowHandles), boost::cref(flows));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure playlist
    uint32_t playlistHandle = 9;
    DpgApplication::playlistConfig_t playlistConfig;
    if (isStream)
    {
        playlistConfig.Streams.resize(1);
        playlistConfig.Streams[0].Handle = flowHandle;
        // FIXME -- add variables and modifiers
        playlistConfig.Streams[0].FlowParam.ServerPort = 3357;
        playlistConfig.Streams[0].StreamParam.StartTime = 0;
        playlistConfig.Streams[0].StreamParam.DataDelay = 10;
        playlistConfig.Streams[0].StreamParam.CloseType = DpgIf_1_port_server::CLOSE_TYPE_S_FIN;
        playlistConfig.Streams[0].StreamParam.MinPktDelay = 0;
        playlistConfig.Streams[0].StreamParam.MaxPktDelay = 10000;
    }
    else
    {
        playlistConfig.Attacks.resize(1);
        playlistConfig.Attacks[0].Handle = flowHandle;
        // FIXME -- add variables and modifiers
        playlistConfig.Attacks[0].FlowParam.ServerPort = 3357;
        playlistConfig.Attacks[0].AttackParam.PktDelay = 10;
        playlistConfig.Attacks[0].AttackParam.InactivityTimeout = 100000;
    }

    {
        const DpgApplication::playlistConfigList_t playlists(1, playlistConfig);
        std::vector<uint32_t> playlistHandles(1, playlistHandle);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::UpdatePlaylists, &app, 0, boost::cref(playlistHandles), boost::cref(playlists));
        scheduler.Enqueue(req);
        req->Wait();
    }

    
    // Configure server block
    DpgApplication::serverConfig_t serverConfig;

    DpgTestUtils::InitCommonProfile(&serverConfig.Common.Profile.L4L7Profile, "TestNfs");
    serverConfig.Common.Endpoint.SrcIfHandle = 0;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 1;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestNfs";
    serverConfig.ProtocolConfig.MaxSimultaneousClients = 0;
    serverConfig.Playlists.resize(1);
    serverConfig.Playlists[0].Handle = playlistHandle;

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
    clientConfig.Common.Load.LoadProfile.UseDynamicLoad = false;
    clientConfig.Common.Load.LoadPhases.resize(1);
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.Height = 2;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 15;
#if 0
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
#endif
    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestNfs";
    clientConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    clientConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    clientConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    clientConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    DpgTestUtils::InitClientEndpoint(&clientConfig.Common.Endpoint);
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestNfs";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.MaxTransactionsPerServer = 16;
    clientConfig.Playlists.resize(1);
    clientConfig.Playlists[0].Handle = playlistHandle;

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
fprintf(stderr, "BEGIN STOP\n");

        // Stop the server and client blocks
        {
            DpgApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
            handles.push_back(serverHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
fprintf(stderr, "DONE STOP\n");
    }

    // It's possible some stoppy things may take longer? 
    sleep(1);
    
fprintf(stderr, "BEGIN SHUTDOWN\n");
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
fprintf(stderr, "DONE SHUTDOWN\n");
    
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
fprintf(stderr, "DONE DONE\n");

    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
}

///////////////////////////////////////////////////////////////////////////////

void TestCifs::testCifsOneMessageType()
{
//    PHX_LOG_REDIRECTLOG_INIT("", "0x1-MPS,0x2-Client,0x4-Server,0x8-Socket,0x10-Unused", LOG_UPTO(PHX_LOG_DEBUG), PHX_LOCAL_ID_1 | PHX_LOCAL_ID_2 | PHX_LOCAL_ID_3 | PHX_LOCAL_ID_4);
   
    bool isTcp = true, isStream = true;
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    DpgApplication::handleList_t serverHandles;
    DpgApplication::handleList_t clientHandles;
	DpgApplication app(1);

    // Start application
fprintf(stderr, "BEGIN START\n");
    CPPUNIT_ASSERT(scheduler.Start(1, NUM_IO_THREADS) == 0);

    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }

    DpgApplication::flowConfigList_t flows;
    std::vector<uint32_t> flowHandles;

    // Configure flow1 - mount directory
    flowHandles.push_back(8);
    DpgApplication::flowConfig_t flowConfig;
    unsigned char mount_dir_data0[] = {
      0x00, 0x00, 0x00, 0xdb, 0xff, 0x53, 0x4d, 0x42, 0x72, 0x00, 0x00, 0x00, 0x00, 0x08, 0x43, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0xb8, 0x00, 0x02, 0x50, 0x43, 0x20, 0x4e, 0x45, 0x54, 0x57, 0x4f, 0x52, 0x4b, 0x20, 0x50, 0x52, 0x4f, 0x47, 0x52, 0x41, 0x4d, 0x20, 0x31, 0x2e, 0x30, 0x00, 0x02, 0x4d, 0x49, 0x43, 0x52, 0x4f, 0x53, 0x4f, 0x46, 0x54, 0x20, 0x4e, 0x45, 0x54, 0x57, 0x4f, 0x52, 0x4b, 0x53, 0x20, 0x31, 0x2e, 0x30, 0x33, 0x00, 0x02, 0x4d, 0x49, 0x43, 0x52, 0x4f, 0x53, 0x4f, 0x46, 0x54, 0x20, 0x4e, 0x45, 0x54, 0x57, 0x4f, 0x52, 0x4b, 0x53, 0x20, 0x33, 0x2e, 0x30, 0x00, 0x02, 0x4c, 0x41, 0x4e, 0x4d, 0x41, 0x4e, 0x31, 0x2e, 0x30, 0x00, 0x02, 0x57, 0x69, 0x6e, 0x64, 0x6f, 0x77, 0x73, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x57, 0x6f, 0x72, 0x6b, 0x67, 0x72, 0x6f, 0x75, 0x70, 0x73, 0x20, 0x33, 0x2e, 0x31, 0x61, 0x00, 0x02, 0x4c, 0x4d, 0x31, 0x2e, 0x32, 0x58, 0x30, 0x30, 0x32, 0x00, 0x02, 0x44, 0x4f, 0x53, 0x20, 0x4c, 0x41, 0x4e, 0x4d, 0x41, 0x4e, 0x32, 0x2e, 0x31, 0x00, 0x02, 0x4c, 0x41, 0x4e, 0x4d, 0x41, 0x4e, 0x32, 0x2e, 0x31, 0x00, 0x02, 0x53, 0x61, 0x6d, 0x62, 0x61, 0x00, 0x02, 0x4e, 0x54, 0x20, 0x4c, 0x41, 0x4e, 0x4d, 0x41, 0x4e, 0x20, 0x31, 0x2e, 0x30, 0x00, 0x02, 0x4e, 0x54, 0x20, 0x4c, 0x4d, 0x20, 0x30, 0x2e, 0x31, 0x32, 0x00};
    unsigned char mount_dir_data1[] = {
      0x00, 0x00, 0x00, 0x95, 0xff, 0x53, 0x4d, 0x42, 0x72, 0x00, 0x00, 0x00, 0x00, 0x88, 0x43, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x11, 0x0a, 0x00, 0x03, 0x0a, 0x00, 0x01, 0x00, 0x04, 0x11, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd, 0xe3, 0x03, 0x80, 0xa8, 0x61, 0xb7, 0xcb, 0xb9, 0xec, 0xc5, 0x01, 0x6c, 0xfd, 0x00, 0x50, 0x00, 0xbd, 0xad, 0xb6, 0xaf, 0x27, 0x30, 0x66, 0x4c, 0x9e, 0xcd, 0x6c, 0x03, 0x4b, 0x35, 0x29, 0x4b, 0x60, 0x3e, 0x06, 0x06, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x02, 0xa0, 0x34, 0x30, 0x32, 0xa0, 0x30, 0x30, 0x2e, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x82, 0xf7, 0x12, 0x01, 0x02, 0x02, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x12, 0x01, 0x02, 0x02, 0x06, 0x0a, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x12, 0x01, 0x02, 0x02, 0x03, 0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x02, 0x02, 0x0a};
    flowConfig.Data.set_size(sizeof(mount_dir_data0) + sizeof(mount_dir_data1));
    {
        size_t i = 0;
        for (size_t j = 0; j < sizeof(mount_dir_data0); ++i, ++j)
        {
            flowConfig.Data[i] = mount_dir_data0[j];
        }
        for (size_t j = 0; j < sizeof(mount_dir_data1); ++i, ++j)
        {
            flowConfig.Data[i] = mount_dir_data1[j];
        }
    }
    flowConfig.PktLen.clear();
    flowConfig.PktLen.push_back(sizeof(mount_dir_data0));
    flowConfig.PktLen.push_back(sizeof(mount_dir_data1));
    flowConfig.PktDelay.clear();
    flowConfig.PktDelay.push_back(0);
    flowConfig.PktDelay.push_back(1);
    flowConfig.ClientTx.clear();
    flowConfig.ClientTx.push_back(true);
    flowConfig.ClientTx.push_back(false);
    flowConfig.Layer = isTcp ? 
        DpgIf_1_port_server::LAYER_TCP : DpgIf_1_port_server::LAYER_UDP;
    flowConfig.Close = DpgIf_1_port_server::CLOSE_TYPE_S_FIN;
    flowConfig.Type  = isStream ? 
        DpgIf_1_port_server::PLAY_TYPE_STREAM : DpgIf_1_port_server::PLAY_TYPE_ATTACK;
    flows.push_back(flowConfig);

    // FIXME add variables?

fprintf(stderr, "BEGIN UPDATE FLOWS\n");
try
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::UpdateFlows, &app, 0, boost::cref(flowHandles), boost::cref(flows));
        scheduler.Enqueue(req);
        req->Wait();
    }
catch (PHXException & p)
{
fprintf(stderr, "got exception p: %s\n", p.GetErrorString().c_str());
}
    
    // Configure playlist
    uint32_t playlistHandle = 12;
    DpgApplication::playlistConfig_t playlistConfig;
    if (isStream)
    {
        playlistConfig.Streams.resize(1);
        playlistConfig.Streams[0].Handle = flowHandles[0];
        playlistConfig.Streams[0].FlowParam.ServerPort = 445;
        playlistConfig.Streams[0].StreamParam.StartTime = 0;
        playlistConfig.Streams[0].StreamParam.DataDelay = 0;
        playlistConfig.Streams[0].StreamParam.CloseType = DpgIf_1_port_server::CLOSE_TYPE_C_FIN;
        playlistConfig.Streams[0].StreamParam.MinPktDelay = 0;
        playlistConfig.Streams[0].StreamParam.MaxPktDelay = 1000000;

    }
    else
    {
        playlistConfig.Attacks.resize(1);
        playlistConfig.Attacks[0].Handle = flowHandles[0];
        // FIXME -- add variables and modifiers
        playlistConfig.Attacks[0].FlowParam.ServerPort = 3357;
        playlistConfig.Attacks[0].AttackParam.PktDelay = 10;
        playlistConfig.Attacks[0].AttackParam.InactivityTimeout = 100000;
    }

fprintf(stderr, "BEGIN UPDATE PLAYLISTS\n");
    {
        const DpgApplication::playlistConfigList_t playlists(1, playlistConfig);
        std::vector<uint32_t> playlistHandles(1, playlistHandle);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::UpdatePlaylists, &app, 0, boost::cref(playlistHandles), boost::cref(playlists));
        scheduler.Enqueue(req);
        req->Wait();
    }

    
    // Configure server block
    DpgApplication::serverConfig_t serverConfig;

    DpgTestUtils::InitCommonProfile(&serverConfig.Common.Profile.L4L7Profile, "TestCifs");
    serverConfig.Common.Endpoint.SrcIfHandle = 0;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 1;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestCifs";
    serverConfig.ProtocolConfig.MaxSimultaneousClients = 4000000000;
    serverConfig.Playlists.resize(1);
    serverConfig.Playlists[0].Handle = playlistHandle;

fprintf(stderr, "BEGIN CONFIG SERVERS\n");
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
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 123456;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 4294967295;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 4096;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 4294967295;
    clientConfig.Common.Load.LoadProfile.UseDynamicLoad = false;
    clientConfig.Common.Load.LoadPhases.resize(1);
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.Height = 8;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 300;
#if 0
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
#endif
    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestCifs";
    clientConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0xC0;
    clientConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    clientConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 32768;
    clientConfig.Common.Profile.L4L7Profile.EnableDelayedAck = false;
    DpgTestUtils::InitClientEndpoint(&clientConfig.Common.Endpoint);
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestCifs";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.MaxTransactionsPerServer = 16;
    clientConfig.Playlists.resize(1);
    clientConfig.Playlists[0].Handle = playlistHandle;

fprintf(stderr, "BEGIN CONFIG CLIENTS\n");
    {
        const DpgApplication::clientConfigList_t clients(1, clientConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(clientHandles.size() == 1);
    CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);

fprintf(stderr, "BEGIN START\n");

    for (size_t run = 0; run < 2; run++)
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

fprintf(stderr, "END START\n");

fprintf(stderr, "WAITING ");
        
        for (uint32_t i = 0; i < 31; i++)
        {
                // Let the blocks run for a bit
                sleep(10);
                fprintf(stderr, "%d0 SEC ", i);
        }
fprintf(stderr, "BEGIN STOP\n");

        // Stop the server and client blocks
        {
            DpgApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
            handles.push_back(serverHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
fprintf(stderr, "DONE STOP\n");
    }

    // It's possible some stoppy things may take longer? 
    sleep(5);
    
fprintf(stderr, "BEGIN SHUTDOWN\n");
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
fprintf(stderr, "DONE SHUTDOWN\n");
    
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
fprintf(stderr, "DONE DONE\n");
}

///////////////////////////////////////////////////////////////////////////////


void TestCifs::testOneServerManyClients(void)
{
                              // TCP    Stream
    testOneServerManyClientsBody(true,  true);
    testOneServerManyClientsBody(false, true);
    testOneServerManyClientsBody(true,  false);
    testOneServerManyClientsBody(false, false);
}

///////////////////////////////////////////////////////////////////////////////

void TestCifs::testOneServerManyClientsBody(bool isTcp, bool isStream)
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

    // Configure flow
    uint32_t flowHandle = 8;
    DpgApplication::flowConfig_t flowConfig;
    flowConfig.Data.set_size(40);
    for (size_t i = 0; i < 40; ++i) flowConfig.Data[i] = i;
    flowConfig.PktLen.resize(4, 10);
    flowConfig.PktDelay.resize(4, 10);
    flowConfig.ClientTx.resize(4, true);
    flowConfig.ClientTx[1] = false;
    flowConfig.ClientTx[3] = false;
    flowConfig.Layer = isTcp ? 
        DpgIf_1_port_server::LAYER_TCP : DpgIf_1_port_server::LAYER_UDP;
    flowConfig.Close = DpgIf_1_port_server::CLOSE_TYPE_C_FIN;
    flowConfig.Type  = isStream ? 
        DpgIf_1_port_server::PLAY_TYPE_STREAM : DpgIf_1_port_server::PLAY_TYPE_ATTACK;
    // FIXME add variables/loops?

    {
        const DpgApplication::flowConfigList_t flows(1, flowConfig);
        std::vector<uint32_t> flowHandles(1, flowHandle);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::UpdateFlows, &app, 0, boost::cref(flowHandles), boost::cref(flows));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure playlist
    uint32_t playlistHandle = 9;
    DpgApplication::playlistConfig_t playlistConfig;

    if (isStream)
    {
        playlistConfig.Streams.resize(1);
        playlistConfig.Streams[0].Handle = flowHandle;
        // FIXME -- add variables and modifiers
        playlistConfig.Streams[0].FlowParam.ServerPort = 3357;
        playlistConfig.Streams[0].StreamParam.StartTime = 0;
        playlistConfig.Streams[0].StreamParam.DataDelay = 10;
        playlistConfig.Streams[0].StreamParam.CloseType = 0 /*C_FIN*/;
        playlistConfig.Streams[0].StreamParam.MinPktDelay = 0;
        playlistConfig.Streams[0].StreamParam.MaxPktDelay = 10000;
    }
    else
    {
        playlistConfig.Attacks.resize(1);
        playlistConfig.Attacks[0].Handle = flowHandle;
        // FIXME -- add variables and modifiers
        playlistConfig.Attacks[0].FlowParam.ServerPort = 3357;
        playlistConfig.Attacks[0].AttackParam.PktDelay = 10;
        playlistConfig.Attacks[0].AttackParam.InactivityTimeout = 100000;
    }

    {
        const DpgApplication::playlistConfigList_t playlists(1, playlistConfig);
        std::vector<uint32_t> playlistHandles(1, playlistHandle);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &DpgApplication::UpdatePlaylists, &app, 0, boost::cref(playlistHandles), boost::cref(playlists));
        scheduler.Enqueue(req);
        req->Wait();
    }

    
    // Configure server block
    DpgApplication::serverConfig_t serverConfig;

    DpgTestUtils::InitCommonProfile(&serverConfig.Common.Profile.L4L7Profile, "TestNfs");
    serverConfig.Common.Endpoint.SrcIfHandle = 0;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 1;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestNfs";
    serverConfig.ProtocolConfig.MaxSimultaneousClients = 0;
    serverConfig.Playlists.resize(1);
    serverConfig.Playlists[0].Handle = playlistHandle;

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
    clientConfig.Common.Load.LoadProfile.UseDynamicLoad = false;
    clientConfig.Common.Load.LoadPhases.resize(1);
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.Height = 2;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 10;
#if 0
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
#endif
    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestNfs";
    clientConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    clientConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    clientConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    clientConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    DpgTestUtils::InitClientEndpoint(&clientConfig.Common.Endpoint);
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestNfs";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.MaxTransactionsPerServer = 16;
    clientConfig.Playlists.resize(1);
    clientConfig.Playlists[0].Handle = playlistHandle;

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

void TestCifs::testNoServerManyClients(void)
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
    clientConfig.Common.Load.LoadProfile.UseDynamicLoad = false;
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
    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestNfs";
    clientConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    clientConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    clientConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    clientConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    DpgTestUtils::InitClientEndpoint(&clientConfig.Common.Endpoint);
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestNfs";
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

void TestCifs::testOneServerManyClientsWithSingleThread(void)
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

    DpgTestUtils::InitCommonProfile(&serverConfig.Common.Profile.L4L7Profile, "TestNfs");
    serverConfig.Common.Endpoint.SrcIfHandle = 0;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 1;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestNfs";
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
    clientConfig.Common.Load.LoadProfile.UseDynamicLoad = false;
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
    DpgTestUtils::InitCommonProfile(&clientConfig.Common.Profile.L4L7Profile, "TestNfs");
    DpgTestUtils::InitClientEndpoint(&clientConfig.Common.Endpoint);
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestNfs";
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

void TestCifs::testNoServerManyClientsWithSingleThread(void)
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
    clientConfig.Common.Load.LoadProfile.UseDynamicLoad = false;
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
    DpgTestUtils::InitCommonProfile(&clientConfig.Common.Profile.L4L7Profile, "TestNfs");
    DpgTestUtils::InitClientEndpoint(&clientConfig.Common.Endpoint);
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestNfs";
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

void TestCifs::testOneServerManyClientsWithSinusoid(void)
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

    DpgTestUtils::InitCommonProfile(&serverConfig.Common.Profile.L4L7Profile, "TestNfs");
    serverConfig.Common.Endpoint.SrcIfHandle = 0;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 1;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestNfs";
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
    clientConfig.Common.Load.LoadProfile.UseDynamicLoad = false;
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
    DpgTestUtils::InitCommonProfile(&clientConfig.Common.Profile.L4L7Profile, "TestNfs");
    DpgTestUtils::InitClientEndpoint(&clientConfig.Common.Endpoint);
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestNfs";
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

void TestCifs::testNoServerManyClientsWithSinusoid(void)
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
    clientConfig.Common.Load.LoadProfile.UseDynamicLoad = false;
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
    DpgTestUtils::InitCommonProfile(&clientConfig.Common.Profile.L4L7Profile, "TestNfs");
    DpgTestUtils::InitClientEndpoint(&clientConfig.Common.Endpoint);
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestNfs";
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

CPPUNIT_TEST_SUITE_REGISTRATION(TestCifs);
