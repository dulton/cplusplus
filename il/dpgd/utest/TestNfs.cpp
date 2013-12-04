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

class TestNfs : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestNfs);
//    CPPUNIT_TEST(testOneServerOneClientAttackLoop);
    CPPUNIT_TEST(testNfs);
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
    static const size_t NUM_IO_THREADS = 2; // 1; //4;
    static const rlim_t MAXIMUM_OPEN_FILE_HARD_LIMIT;
    
    void testOneServerOneClientAttackLoop(void);
    void testNfs();
    void testOneServerManyClients(void);
    void testOneServerManyClientsBody(bool isTcp, bool isStream);
    void testNoServerManyClients(void);
    void testOneServerManyClientsWithSingleThread(void);
    void testNoServerManyClientsWithSingleThread(void);
    void testOneServerManyClientsWithSinusoid(void);
    void testNoServerManyClientsWithSinusoid(void);
};

///////////////////////////////////////////////////////////////////////////////

const rlim_t TestNfs::MAXIMUM_OPEN_FILE_HARD_LIMIT = (32 * 1024);

///////////////////////////////////////////////////////////////////////////////

void TestNfs::setUp(void)
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

void TestNfs::testOneServerOneClientAttackLoop()
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

void TestNfs::testNfs()
{
//    PHX_LOG_REDIRECTLOG_INIT("", "0x1-MPS,0x2-Client,0x4-Server,0x8-Socket,0x10-Unused", LOG_UPTO(PHX_LOG_DEBUG), PHX_LOCAL_ID_1 | PHX_LOCAL_ID_2 | PHX_LOCAL_ID_3 | PHX_LOCAL_ID_4);

    bool isTcp = false, isStream = true;
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
        0x02, 0x9F, 0x04, 0x18, 0x00, 0x7C, 0x1A, 0x15, 0x38, 0x41, 0x16, 0x9F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x86, 0xA5, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x34, 0x38, 0x47, 0x75, 0xD0, 0x00, 0x00, 0x00, 0x09, 0x77, 0x65, 0x72, 0x72, 0x6D, 0x73, 0x63, 0x68, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x2F, 0x68, 0x6F, 0x6D, 0x65, 0x2F, 0x67, 0x69, 0x72, 0x6C, 0x69, 0x63, 0x68, 0x2F, 0x65, 0x78, 0x70, 0x6F, 0x72, 0x74};
    unsigned char mount_dir_data1[] = {
        0x04, 0x18, 0x02, 0x9F, 0x00, 0x44, 0xED, 0x9B, 0x38, 0x41, 0x16, 0x9F, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x85, 0x00, 0x00, 0x03, 0xE7, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0xB2, 0x5A, 0x00, 0x00, 0x00, 0x29, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0xB2, 0x5A, 0x00, 0x00, 0x00, 0x29};
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
    flowConfig.PktDelay.push_back(20);
    flowConfig.PktDelay.push_back(0);
    flowConfig.ClientTx.clear();
    flowConfig.ClientTx.push_back(true);
    flowConfig.ClientTx.push_back(false);
    flowConfig.Layer = isTcp ? 
        DpgIf_1_port_server::LAYER_TCP : DpgIf_1_port_server::LAYER_UDP;
    flowConfig.Close = DpgIf_1_port_server::CLOSE_TYPE_S_FIN;
    flowConfig.Type  = isStream ? 
        DpgIf_1_port_server::PLAY_TYPE_STREAM : DpgIf_1_port_server::PLAY_TYPE_ATTACK;
    flows.push_back(flowConfig);

    // Configure flow2 - portmap
    flowHandles.push_back(9);
    unsigned char portmap_data0[] = {
        0x0C, 0xDB, 0x00, 0x6F, 0x00, 0x48, 0xD9, 0xA0, 0x38, 0x41, 0xAA, 0xDF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x86, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x86, 0xA3, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x75, 0x64, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char portmap_data1[] = {
        0x00, 0x6F, 0x0C, 0xDB, 0x00, 0x38, 0x02, 0xE3, 0x38, 0x41, 0xAA, 0xDF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x31, 0x33, 0x39, 0x2E, 0x32, 0x35, 0x2E, 0x32, 0x32, 0x2E, 0x31, 0x30, 0x32, 0x2E, 0x38, 0x2E, 0x31, 0x00, 0x00, 0x00};
    flowConfig.Data.set_size(sizeof(portmap_data0) + sizeof(portmap_data1));
    {
        size_t i = 0;
        for (size_t j = 0; j < sizeof(portmap_data0); ++i, ++j)
        {
            flowConfig.Data[i] = portmap_data0[j];
        }
        for (size_t j = 0; j < sizeof(portmap_data1); ++i, ++j)
        {
            flowConfig.Data[i] = portmap_data1[j];
        }
    }
    flowConfig.PktLen.clear();
    flowConfig.PktLen.push_back(sizeof(portmap_data0));
    flowConfig.PktLen.push_back(sizeof(portmap_data1));
    flowConfig.PktDelay.clear();
    flowConfig.PktDelay.push_back(10);
    flowConfig.PktDelay.push_back(0);
    flowConfig.ClientTx.clear();
    flowConfig.ClientTx.push_back(true);
    flowConfig.ClientTx.push_back(false);
    flowConfig.Layer = isTcp ? 
        DpgIf_1_port_server::LAYER_TCP : DpgIf_1_port_server::LAYER_UDP;
    flowConfig.Close = DpgIf_1_port_server::CLOSE_TYPE_S_FIN;
    flowConfig.Type  = isStream ? 
        DpgIf_1_port_server::PLAY_TYPE_STREAM : DpgIf_1_port_server::PLAY_TYPE_ATTACK;
    flows.push_back(flowConfig);

    // Configure flow3 - queries
    flowHandles.push_back(10);
    flowConfig.Data.set_size(15000);
    for (size_t j = 0; j < 15000; ++j)
    {
        flowConfig.Data[j] = j;
    }
    flowConfig.PktLen.clear();
    flowConfig.PktLen.resize(150, 100);
    flowConfig.PktDelay.clear();
    flowConfig.PktDelay.resize(150, 0);
    flowConfig.PktDelay[15] = 10;
    flowConfig.PktDelay[30] = 10;
    flowConfig.PktDelay[45] = 10;
    flowConfig.PktDelay[60] = 10;
    flowConfig.PktDelay[75] = 10;
    flowConfig.PktDelay[90] = 10;
    flowConfig.PktDelay[105] = 10;
    flowConfig.PktDelay[120] = 10;
    flowConfig.PktDelay[135] = 10;
    flowConfig.ClientTx.clear();
    flowConfig.ClientTx.resize(150);
    for (size_t j = 0; j < 150; ++j)
    {
        flowConfig.ClientTx[j] = !(j % 2);
    }
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
        playlistConfig.Streams.resize(3);
        playlistConfig.Streams[0].Handle = flowHandles[0];
        playlistConfig.Streams[0].FlowParam.ServerPort = 2001;
        playlistConfig.Streams[0].StreamParam.StartTime = 0;
        playlistConfig.Streams[0].StreamParam.DataDelay = 20;
        playlistConfig.Streams[0].StreamParam.CloseType = DpgIf_1_port_server::CLOSE_TYPE_C_FIN;
        playlistConfig.Streams[0].StreamParam.MinPktDelay = 0;
        playlistConfig.Streams[0].StreamParam.MaxPktDelay = 1000000;

        playlistConfig.Streams[1].Handle = flowHandles[1];
        playlistConfig.Streams[1].FlowParam.ServerPort = 2011;
        playlistConfig.Streams[1].StreamParam.StartTime = 0;
        playlistConfig.Streams[1].StreamParam.DataDelay = 30;
        playlistConfig.Streams[1].StreamParam.CloseType = DpgIf_1_port_server::CLOSE_TYPE_C_FIN;
        playlistConfig.Streams[1].StreamParam.MinPktDelay = 0;
        playlistConfig.Streams[1].StreamParam.MaxPktDelay = 1000000;

        playlistConfig.Streams[2].Handle = flowHandles[1];
        playlistConfig.Streams[2].FlowParam.ServerPort = 2010;
        playlistConfig.Streams[2].StreamParam.StartTime = 0;
        playlistConfig.Streams[2].StreamParam.DataDelay = 80;
        playlistConfig.Streams[2].StreamParam.CloseType = DpgIf_1_port_server::CLOSE_TYPE_C_FIN;
        playlistConfig.Streams[2].StreamParam.MinPktDelay = 0;
        playlistConfig.Streams[2].StreamParam.MaxPktDelay = 1000000;

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

    DpgTestUtils::InitCommonProfile(&serverConfig.Common.Profile.L4L7Profile, "TestNfs");
    serverConfig.Common.Endpoint.SrcIfHandle = 0;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 1;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestNfs";
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
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;
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

fprintf(stderr, "BEGIN CONFIG CLIENTS\n");
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
        sleep(40);
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
}

///////////////////////////////////////////////////////////////////////////////


void TestNfs::testOneServerManyClients(void)
{
                              // TCP    Stream
    testOneServerManyClientsBody(true,  true);
    testOneServerManyClientsBody(false, true);
    testOneServerManyClientsBody(true,  false);
    testOneServerManyClientsBody(false, false);
}

///////////////////////////////////////////////////////////////////////////////

void TestNfs::testOneServerManyClientsBody(bool isTcp, bool isStream)
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

void TestNfs::testNoServerManyClients(void)
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

void TestNfs::testOneServerManyClientsWithSingleThread(void)
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

void TestNfs::testNoServerManyClientsWithSingleThread(void)
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

void TestNfs::testOneServerManyClientsWithSinusoid(void)
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

void TestNfs::testNoServerManyClientsWithSinusoid(void)
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

CPPUNIT_TEST_SUITE_REGISTRATION(TestNfs);
