#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <list>

#include <ace/Lock_Adapter_T.h>
#include <ace/Null_Mutex.h>

#include <base/EndpointPairEnumerator.h>

#include "MockPlaylistEngine.h"
#include "MockPlaylistInstance.h"
#include "MockReactor.h"
#include "DpgTestUtils.h"
#include "AddrInfo.h"
#include "SocketManager.h"
#include "DpgClientBlock.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestDpgClientBlock : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestDpgClientBlock);
    CPPUNIT_TEST(testSpawn);
    CPPUNIT_TEST(testMultiClientSpawn);
    CPPUNIT_TEST_SUITE_END();

public:
    TestDpgClientBlock()
    {
    }

    ~TestDpgClientBlock()
    {
    }

    void setUp() 
    { 
    }

    void tearDown() 
    { 
    }
    
protected:
    void testSpawn();
    void testMultiClientSpawn();

private:
};

///////////////////////////////////////////////////////////////////////////////

void TestDpgClientBlock::testSpawn()
{
    MockPlaylistEngine playlistEngine;
    MockReactor appReactor;
    MockReactor ioReactor;
    Dpg_1_port_server::DpgClientConfig_t config;
    ACE_Lock_Adapter<ACE_Null_Mutex> lock;

    uint32_t playlistHandle = 100;

    L4L7_ENGINE_NS::PlaylistConfig playlistConfig;
    playlistEngine.UpdatePlaylist(playlistHandle, playlistConfig);

    memset(&config.Common.Load.LoadProfile, 0, sizeof(config.Common.Load.LoadProfile));
    config.Common.Load.LoadProfile.LoadType = l4l7Base_1_port_server::LoadTypes_PLAYLISTS;
    config.Common.Load.LoadPhases.resize(1);
    memset(&config.Common.Load.LoadPhases[0], 0, sizeof(config.Common.Load.LoadPhases[0]));
    config.Common.Load.LoadPhases[0].LoadPhase.LoadPattern = l4l7Base_1_port_server::LoadPatternTypes_FLAT;
    config.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = l4l7Base_1_port_server::ClientLoadPhaseTimeUnitOptions_SECONDS;
    config.Common.Load.LoadPhases[0].FlatDescriptor.Height = 0;
    // we will manually spawn, official load is 0
    config.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 0;
    config.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 10;


    DpgTestUtils::InitCommonProfile(&config.Common.Profile.L4L7Profile, "TestDpgClientBlock");
    DpgTestUtils::InitClientEndpoint(&config.Common.Endpoint);

    //empty struct -- config.ProtocolProfile;
    config.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestDpgClient";
    config.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    config.ProtocolConfig.L4L7ProtocolConfigClientBase.DynamicLoad = 0;
    config.ProtocolConfig.MaxTransactionsPerServer = 0;
    config.Playlists.resize(1);
    config.Playlists[0].Handle = playlistHandle;

    DpgClientBlock client(0, config, &playlistEngine, &appReactor, &ioReactor, &lock);

    // Start to set things up; no load so shouldn't make any playlists
    client.Start();
    client.mRunning = false; // unit test hack to prevent stop messaging (hangs without actual threads)

    CPPUNIT_ASSERT_EQUAL(size_t(0), client.mActivePlaylists);

    // Verify config applied to socket manager
    SocketManager* sockMgr = client.mSocketMgr.get();
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.Ipv4Tos, sockMgr->GetIpv4Tos());
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.Ipv6TrafficClass, sockMgr->GetIpv6TrafficClass());
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.RxWindowSizeLimit, sockMgr->GetTcpWindowSizeLimit());
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.EnableDelayedAck, sockMgr->GetTcpDelayedAck());
    CPPUNIT_ASSERT_EQUAL(size_t(0), sockMgr->mPlayAddrMap.size());

    client.SpawnPlaylists(1);

    // Verify a playlist is made
    CPPUNIT_ASSERT_EQUAL(size_t(1), client.mActivePlaylists);
    CPPUNIT_ASSERT_EQUAL(size_t(1), playlistEngine.GetPlaylists().size());
    CPPUNIT_ASSERT_EQUAL(playlistHandle, playlistEngine.GetPlaylists().begin()->handle);
    CPPUNIT_ASSERT_EQUAL(true, playlistEngine.GetPlaylists().begin()->client);

    // Verify it's registered with sock mgr
    CPPUNIT_ASSERT_EQUAL(size_t(1), sockMgr->mPlayAddrMap.size());
    MockPlaylistInstance * pi0 = playlistEngine.GetPlaylists().begin()->pi;
    CPPUNIT_ASSERT_EQUAL(size_t(1), sockMgr->mPlayAddrMap.count(pi0));

    // Verify it's started
    CPPUNIT_ASSERT_EQUAL(true, pi0->IsStarted());

    // Verify endpoint
    AddrInfo& addrInfo(sockMgr->mPlayAddrMap[pi0]);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0x7f000001), addrInfo.locAddr.get_ip_address());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0x7f000001), addrInfo.remAddr.get_ip_address());
    CPPUNIT_ASSERT_EQUAL(true, addrInfo.hasRem);

// verify endpoints are incremented correctly
// verify source/destination ports
// verify destruction does a stop/cleanup
// verify proper playlist is constructed, started
// verify client block makes a client instance
// verify missing playlist fails to start
// FOR SERVER -- verify variables are constructed properly
// verify server makes a server instance
// verify endpoints are incremented correctly
// verify source/destination ports
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgClientBlock::testMultiClientSpawn()
{
    MockPlaylistEngine playlistEngine;
    MockReactor appReactor;
    MockReactor ioReactor;
    Dpg_1_port_server::DpgClientConfig_t config;
    ACE_Lock_Adapter<ACE_Null_Mutex> lock;

    uint32_t playlistHandle = 100;

    L4L7_ENGINE_NS::PlaylistConfig playlistConfig;
    playlistEngine.UpdatePlaylist(playlistHandle, playlistConfig);

    memset(&config.Common.Load.LoadProfile, 0, sizeof(config.Common.Load.LoadProfile));
    config.Common.Load.LoadProfile.LoadType = l4l7Base_1_port_server::LoadTypes_PLAYLISTS;
    config.Common.Load.LoadPhases.resize(1);
    memset(&config.Common.Load.LoadPhases[0], 0, sizeof(config.Common.Load.LoadPhases[0]));
    config.Common.Load.LoadPhases[0].LoadPhase.LoadPattern = l4l7Base_1_port_server::LoadPatternTypes_FLAT;
    config.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = l4l7Base_1_port_server::ClientLoadPhaseTimeUnitOptions_SECONDS;
    config.Common.Load.LoadPhases[0].FlatDescriptor.Height = 0;
    // we will manually spawn, official load is 0
    config.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 0;
    config.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 10;


    DpgTestUtils::InitCommonProfile(&config.Common.Profile.L4L7Profile, "TestDpgClientBlock");
    DpgTestUtils::InitClientEndpoint(&config.Common.Endpoint);

    //empty struct -- config.ProtocolProfile;
    config.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestDpgClient";
    config.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    config.ProtocolConfig.L4L7ProtocolConfigClientBase.DynamicLoad = 0;
    config.ProtocolConfig.MaxTransactionsPerServer = 0;
    config.Playlists.resize(1);
    config.Playlists[0].Handle = playlistHandle;
    
    DpgClientBlock client(0, config, &playlistEngine, &appReactor, &ioReactor, &lock);

    // inject multiple local addresses (device count == 2)
    std::vector<ACE_INET_Addr> clientAddrs;
    ACE_INET_Addr addr0(uint16_t(0), uint32_t(0x01020304)); // 1.2.3.4
    ACE_INET_Addr addr1(uint16_t(0), uint32_t(0x01020305)); // 1.2.3.5
    clientAddrs.push_back(addr0);
    clientAddrs.push_back(addr1);
    client.mEndpointEnum->SetLocalInterfaces(clientAddrs);

    // Start to set things up; no load so shouldn't make any playlists
    client.Start();
    client.mRunning = false; // unit test hack to prevent stop messaging (hangs without actual threads)

    CPPUNIT_ASSERT_EQUAL(size_t(0), client.mActivePlaylists);

    // Verify config applied to socket manager
    SocketManager* sockMgr = client.mSocketMgr.get();
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.Ipv4Tos, sockMgr->GetIpv4Tos());
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.Ipv6TrafficClass, sockMgr->GetIpv6TrafficClass());
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.RxWindowSizeLimit, sockMgr->GetTcpWindowSizeLimit());
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.EnableDelayedAck, sockMgr->GetTcpDelayedAck());
    CPPUNIT_ASSERT_EQUAL(size_t(0), sockMgr->mPlayAddrMap.size());

    client.SpawnPlaylists(1);

    // Verify a playlist is made
    CPPUNIT_ASSERT_EQUAL(size_t(1), client.mActivePlaylists);
    CPPUNIT_ASSERT_EQUAL(size_t(1), playlistEngine.GetPlaylists().size());
    CPPUNIT_ASSERT_EQUAL(playlistHandle, playlistEngine.GetPlaylists().begin()->handle);
    CPPUNIT_ASSERT_EQUAL(true, playlistEngine.GetPlaylists().begin()->client);

    // Verify it's registered with sock mgr
    CPPUNIT_ASSERT_EQUAL(size_t(1), sockMgr->mPlayAddrMap.size());
    MockPlaylistInstance * pi0 = playlistEngine.GetPlaylists().begin()->pi;
    CPPUNIT_ASSERT_EQUAL(size_t(1), sockMgr->mPlayAddrMap.count(pi0));

    // Verify it's started
    CPPUNIT_ASSERT_EQUAL(true, pi0->IsStarted());

    // Verify endpoint
    AddrInfo& addrInfo0(sockMgr->mPlayAddrMap[pi0]);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0x01020304), addrInfo0.locAddr.get_ip_address());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0x7f000001), addrInfo0.remAddr.get_ip_address());
    CPPUNIT_ASSERT_EQUAL(true, addrInfo0.hasRem);

    // Spawn the next one
    client.SpawnPlaylists(1);

    // Verify a playlist is made
    CPPUNIT_ASSERT_EQUAL(size_t(2), client.mActivePlaylists);
    CPPUNIT_ASSERT_EQUAL(size_t(2), playlistEngine.GetPlaylists().size());
    CPPUNIT_ASSERT_EQUAL(playlistHandle, (++playlistEngine.GetPlaylists().begin())->handle);
    CPPUNIT_ASSERT_EQUAL(true, (++playlistEngine.GetPlaylists().begin())->client);

    // Verify it's registered with sock mgr
    CPPUNIT_ASSERT_EQUAL(size_t(2), sockMgr->mPlayAddrMap.size());
    MockPlaylistInstance * pi1 = (++playlistEngine.GetPlaylists().begin())->pi;
    CPPUNIT_ASSERT_EQUAL(size_t(1), sockMgr->mPlayAddrMap.count(pi1));

    // Verify it's started
    CPPUNIT_ASSERT_EQUAL(true, pi1->IsStarted());

    // Verify endpoint
    AddrInfo& addrInfo1(sockMgr->mPlayAddrMap[pi1]);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0x01020305), addrInfo1.locAddr.get_ip_address());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0x7f000001), addrInfo1.remAddr.get_ip_address());
    CPPUNIT_ASSERT_EQUAL(true, addrInfo1.hasRem);

// verify source/destination ports
// verify missing playlist fails to start
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestDpgClientBlock);
