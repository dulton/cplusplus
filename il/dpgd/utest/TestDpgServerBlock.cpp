#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <list>

#include <ace/Lock_Adapter_T.h>
#include <ace/Null_Mutex.h>

#include <ildaemon/LocalInterfaceEnumerator.h>

#include "MockPlaylistEngine.h"
#include "MockPlaylistInstance.h"
#include "MockReactor.h"
#include "DpgTestUtils.h"
#include "AddrInfo.h"
#include "SocketManager.h"
#include "DpgServerBlock.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestDpgServerBlock : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestDpgServerBlock);
    CPPUNIT_TEST(testStart);
    CPPUNIT_TEST(testMultiServerStart);
    CPPUNIT_TEST(testServerSpawn);
    CPPUNIT_TEST_SUITE_END();

public:
    TestDpgServerBlock()
    {
    }

    ~TestDpgServerBlock()
    {
    }

    void setUp() 
    { 
    }

    void tearDown() 
    { 
    }
    
protected:
    void testStart();
    void testMultiServerStart();
    void testServerSpawn();

private:
};

///////////////////////////////////////////////////////////////////////////////

void TestDpgServerBlock::testStart()
{
    MockPlaylistEngine playlistEngine;
    MockReactor appReactor;
    MockReactor ioReactor;
    Dpg_1_port_server::DpgServerConfig_t config;
    ACE_Lock_Adapter<ACE_Null_Mutex> lock;

    uint32_t playlistHandle0 = 100;
    uint32_t playlistHandle1 = 200;

    L4L7_ENGINE_NS::PlaylistConfig playlistConfig;
    playlistEngine.UpdatePlaylist(playlistHandle0, playlistConfig);
    playlistEngine.UpdatePlaylist(playlistHandle1, playlistConfig);

    DpgTestUtils::InitCommonProfile(&config.Common.Profile.L4L7Profile, "TestDpgServerBlock");
    config.Common.Endpoint.SrcIfHandle = 0;

    //empty struct -- config.ProtocolProfile;
    config.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestDpgServer";
    config.ProtocolConfig.MaxSimultaneousClients = 0;

    config.Playlists.resize(2);
    config.Playlists[0].Handle = playlistHandle0;
    config.Playlists[1].Handle = playlistHandle1;

    config.DstIfs.resize(2);

    // 127.0.0.1
    DpgTestUtils::InitServerIf(&config.DstIfs[0]);

    // 127.0.0.2
    DpgTestUtils::InitServerIf(&config.DstIfs[1]);
    config.DstIfs[0].Ipv4InterfaceList[0].Address.address[3] = 2;

    
    DpgServerBlock server(0, config, &playlistEngine, &appReactor, &ioReactor, &lock);

    // Start 
    server.Start();
    server.mRunning = false; // unit test hack to prevent stop messaging (hangs without actual threads)

    CPPUNIT_ASSERT_EQUAL(size_t(0), server.mActivePlaylists);

    // Verify config applied to socket manager
    SocketManager* sockMgr = server.mSocketMgr.get();
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.Ipv4Tos, sockMgr->GetIpv4Tos());
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.Ipv6TrafficClass, sockMgr->GetIpv6TrafficClass());
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.RxWindowSizeLimit, sockMgr->GetTcpWindowSizeLimit());
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.EnableDelayedAck, sockMgr->GetTcpDelayedAck());

// FIXME -- how to make it start?

    // Verify playlists are made in the engine but not made active
    CPPUNIT_ASSERT_EQUAL(size_t(0), server.mActivePlaylists);
    CPPUNIT_ASSERT_EQUAL(size_t(2), playlistEngine.GetPlaylists().size());
    std::set<L4L7_ENGINE_NS::handle_t> engineHandleSet;
    engineHandleSet.insert(playlistEngine.GetPlaylists().begin()->handle);
    engineHandleSet.insert((++playlistEngine.GetPlaylists().begin())->handle);
    CPPUNIT_ASSERT_EQUAL(size_t(1), engineHandleSet.count(playlistHandle0));
    CPPUNIT_ASSERT_EQUAL(false, playlistEngine.GetPlaylists().begin()->client);
    CPPUNIT_ASSERT_EQUAL(size_t(1), engineHandleSet.count(playlistHandle1));
    CPPUNIT_ASSERT_EQUAL(false, (++playlistEngine.GetPlaylists().begin())->client);

    // Verify they are registered with sock mgr
    CPPUNIT_ASSERT_EQUAL(size_t(2), sockMgr->mPlayAddrMap.size());
    MockPlaylistInstance * pi0 = playlistEngine.GetPlaylists().begin()->pi;
    CPPUNIT_ASSERT_EQUAL(size_t(1), sockMgr->mPlayAddrMap.count(pi0));
    MockPlaylistInstance * pi1 = (++playlistEngine.GetPlaylists().begin())->pi;
    CPPUNIT_ASSERT_EQUAL(size_t(1), sockMgr->mPlayAddrMap.count(pi1));

    // Verify they are started
    CPPUNIT_ASSERT_EQUAL(true, pi0->IsStarted());
    CPPUNIT_ASSERT_EQUAL(true, pi1->IsStarted());

    // Verify endpoint -- no dest yet
    AddrInfo& addrInfo(sockMgr->mPlayAddrMap[pi0]);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0x7f000001), addrInfo.locAddr.get_ip_address());
    CPPUNIT_ASSERT_EQUAL(false, addrInfo.hasRem);
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgServerBlock::testMultiServerStart()
{
    MockPlaylistEngine playlistEngine;
    MockReactor appReactor;
    MockReactor ioReactor;
    Dpg_1_port_server::DpgServerConfig_t config;
    ACE_Lock_Adapter<ACE_Null_Mutex> lock;

    uint32_t playlistHandle0 = 100;
    uint32_t playlistHandle1 = 200;

    L4L7_ENGINE_NS::PlaylistConfig playlistConfig;
    playlistEngine.UpdatePlaylist(playlistHandle0, playlistConfig);
    playlistEngine.UpdatePlaylist(playlistHandle1, playlistConfig);

    DpgTestUtils::InitCommonProfile(&config.Common.Profile.L4L7Profile, "TestDpgServerBlock");
    config.Common.Endpoint.SrcIfHandle = 0;

    //empty struct -- config.ProtocolProfile;
    config.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestDpgServer";
    config.ProtocolConfig.MaxSimultaneousClients = 0;

    config.Playlists.resize(2);
    config.Playlists[0].Handle = playlistHandle0;
    config.Playlists[1].Handle = playlistHandle1;

    config.DstIfs.resize(2);

    // 127.0.0.1
    DpgTestUtils::InitServerIf(&config.DstIfs[0]);

    // 127.0.0.2
    DpgTestUtils::InitServerIf(&config.DstIfs[1]);
    config.DstIfs[0].Ipv4InterfaceList[0].Address.address[3] = 2;

    
    DpgServerBlock server(0, config, &playlistEngine, &appReactor, &ioReactor, &lock);
    
    // inject multiple local addresses (device count == 2)
    std::vector<ACE_INET_Addr> serverAddrs;
    ACE_INET_Addr addr0(uint16_t(0), uint32_t(0x01020304)); // 1.2.3.4
    ACE_INET_Addr addr1(uint16_t(0), uint32_t(0x01020305)); // 1.2.3.5
    serverAddrs.push_back(addr0);
    serverAddrs.push_back(addr1);
    server.mIfEnum->SetInterfaces(serverAddrs);

    // Start 
    server.Start();
    server.mRunning = false; // unit test hack to prevent stop messaging (hangs without actual threads)

    CPPUNIT_ASSERT_EQUAL(size_t(0), server.mActivePlaylists);

    // Verify config applied to socket manager
    SocketManager* sockMgr = server.mSocketMgr.get();
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.Ipv4Tos, sockMgr->GetIpv4Tos());
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.Ipv6TrafficClass, sockMgr->GetIpv6TrafficClass());
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.RxWindowSizeLimit, sockMgr->GetTcpWindowSizeLimit());
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.EnableDelayedAck, sockMgr->GetTcpDelayedAck());

    // Verify playlists are made in the engine but not made active
    CPPUNIT_ASSERT_EQUAL(size_t(0), server.mActivePlaylists);
    // Verify we get 2 playlists X 2 servers
    CPPUNIT_ASSERT_EQUAL(size_t(4), playlistEngine.GetPlaylists().size());
    std::multiset<L4L7_ENGINE_NS::handle_t> engineHandleSet;
    engineHandleSet.insert(playlistEngine.GetPlaylists().begin()->handle);
    engineHandleSet.insert((++playlistEngine.GetPlaylists().begin())->handle);
    engineHandleSet.insert((++++playlistEngine.GetPlaylists().begin())->handle);
    engineHandleSet.insert((++++++playlistEngine.GetPlaylists().begin())->handle);
    
    CPPUNIT_ASSERT_EQUAL(size_t(2), engineHandleSet.count(playlistHandle0));
    CPPUNIT_ASSERT_EQUAL(size_t(2), engineHandleSet.count(playlistHandle1));
    CPPUNIT_ASSERT_EQUAL(false, playlistEngine.GetPlaylists().begin()->client);
    CPPUNIT_ASSERT_EQUAL(false, (++playlistEngine.GetPlaylists().begin())->client);
    CPPUNIT_ASSERT_EQUAL(false, (++++playlistEngine.GetPlaylists().begin())->client);
    CPPUNIT_ASSERT_EQUAL(false, (++++++playlistEngine.GetPlaylists().begin())->client);

    // Verify they are registered with sock mgr
    CPPUNIT_ASSERT_EQUAL(size_t(4), sockMgr->mPlayAddrMap.size());
    MockPlaylistInstance * pi0 = playlistEngine.GetPlaylists().begin()->pi;
    CPPUNIT_ASSERT_EQUAL(size_t(1), sockMgr->mPlayAddrMap.count(pi0));
    MockPlaylistInstance * pi1 = (++playlistEngine.GetPlaylists().begin())->pi;
    CPPUNIT_ASSERT_EQUAL(size_t(1), sockMgr->mPlayAddrMap.count(pi1));
    MockPlaylistInstance * pi2 = (++++playlistEngine.GetPlaylists().begin())->pi;
    CPPUNIT_ASSERT_EQUAL(size_t(1), sockMgr->mPlayAddrMap.count(pi2));
    MockPlaylistInstance * pi3 = (++++playlistEngine.GetPlaylists().begin())->pi;
    CPPUNIT_ASSERT_EQUAL(size_t(1), sockMgr->mPlayAddrMap.count(pi3));

    // Verify they are started
    CPPUNIT_ASSERT_EQUAL(true, pi0->IsStarted());
    CPPUNIT_ASSERT_EQUAL(true, pi1->IsStarted());
    CPPUNIT_ASSERT_EQUAL(true, pi2->IsStarted());
    CPPUNIT_ASSERT_EQUAL(true, pi3->IsStarted());

    // Verify endpoints -- no dest yet
    std::multiset<uint32_t> locAddrs;
    locAddrs.insert(sockMgr->mPlayAddrMap[pi0].locAddr.get_ip_address());
    locAddrs.insert(sockMgr->mPlayAddrMap[pi1].locAddr.get_ip_address());
    locAddrs.insert(sockMgr->mPlayAddrMap[pi2].locAddr.get_ip_address());
    locAddrs.insert(sockMgr->mPlayAddrMap[pi3].locAddr.get_ip_address());
    
    CPPUNIT_ASSERT_EQUAL(size_t(2), locAddrs.count(0x01020304));
    CPPUNIT_ASSERT_EQUAL(size_t(2), locAddrs.count(0x01020305));
    CPPUNIT_ASSERT_EQUAL(false, sockMgr->mPlayAddrMap[pi0].hasRem);
    CPPUNIT_ASSERT_EQUAL(false, sockMgr->mPlayAddrMap[pi1].hasRem);
    CPPUNIT_ASSERT_EQUAL(false, sockMgr->mPlayAddrMap[pi2].hasRem);
    CPPUNIT_ASSERT_EQUAL(false, sockMgr->mPlayAddrMap[pi3].hasRem);

// verify source/destination ports
// verify missing playlist fails to start
// FOR SERVER -- verify variables are constructed properly
// verify source/destination ports
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgServerBlock::testServerSpawn()
{
    MockPlaylistEngine playlistEngine;
    MockReactor appReactor;
    MockReactor ioReactor;
    Dpg_1_port_server::DpgServerConfig_t config;
    ACE_Lock_Adapter<ACE_Null_Mutex> lock;

    uint32_t playlistHandle0 = 100;
    uint32_t playlistHandle1 = 200;

    L4L7_ENGINE_NS::PlaylistConfig playlistConfig;
    playlistEngine.UpdatePlaylist(playlistHandle0, playlistConfig);
    playlistEngine.UpdatePlaylist(playlistHandle1, playlistConfig);

    DpgTestUtils::InitCommonProfile(&config.Common.Profile.L4L7Profile, "TestDpgServerBlock");
    config.Common.Endpoint.SrcIfHandle = 0;

    //empty struct -- config.ProtocolProfile;
    config.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestDpgServer";
    config.ProtocolConfig.MaxSimultaneousClients = 0;

    config.Playlists.resize(2);
    config.Playlists[0].Handle = playlistHandle0;
    config.Playlists[1].Handle = playlistHandle1;

    config.DstIfs.resize(2);

    // 127.0.0.1
    DpgTestUtils::InitServerIf(&config.DstIfs[0]);

    // 127.0.0.2
    DpgTestUtils::InitServerIf(&config.DstIfs[1]);
    config.DstIfs[0].Ipv4InterfaceList[0].Address.address[3] = 2;

    
    DpgServerBlock server(0, config, &playlistEngine, &appReactor, &ioReactor, &lock);

    // Start 
    server.Start();
    server.mRunning = false; // unit test hack to prevent stop messaging (hangs without actual threads)

    CPPUNIT_ASSERT_EQUAL(size_t(0), server.mActivePlaylists);

    // Verify config applied to socket manager
    SocketManager* sockMgr = server.mSocketMgr.get();
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.Ipv4Tos, sockMgr->GetIpv4Tos());
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.Ipv6TrafficClass, sockMgr->GetIpv6TrafficClass());
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.RxWindowSizeLimit, sockMgr->GetTcpWindowSizeLimit());
    CPPUNIT_ASSERT_EQUAL(config.Common.Profile.L4L7Profile.EnableDelayedAck, sockMgr->GetTcpDelayedAck());

    // Verify playlists are made in the engine but not made active
    CPPUNIT_ASSERT_EQUAL(size_t(0), server.mActivePlaylists);
    CPPUNIT_ASSERT_EQUAL(size_t(2), playlistEngine.GetPlaylists().size());
    std::set<L4L7_ENGINE_NS::handle_t> engineHandleSet;
    engineHandleSet.insert(playlistEngine.GetPlaylists().begin()->handle);
    engineHandleSet.insert((++playlistEngine.GetPlaylists().begin())->handle);
    CPPUNIT_ASSERT_EQUAL(size_t(1), engineHandleSet.count(playlistHandle0));
    CPPUNIT_ASSERT_EQUAL(false, playlistEngine.GetPlaylists().begin()->client);
    CPPUNIT_ASSERT_EQUAL(size_t(1), engineHandleSet.count(playlistHandle1));
    CPPUNIT_ASSERT_EQUAL(false, (++playlistEngine.GetPlaylists().begin())->client);

    // Verify they are registered with sock mgr
    CPPUNIT_ASSERT_EQUAL(size_t(2), sockMgr->mPlayAddrMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sockMgr->mPlaySpawnMap.size());
    MockPlaylistInstance * pi0 = playlistEngine.GetPlaylists().begin()->pi;
    CPPUNIT_ASSERT_EQUAL(size_t(1), sockMgr->mPlayAddrMap.count(pi0));
    CPPUNIT_ASSERT_EQUAL(size_t(1), sockMgr->mPlaySpawnMap.count(pi0));
    MockPlaylistInstance * pi1 = (++playlistEngine.GetPlaylists().begin())->pi;
    CPPUNIT_ASSERT_EQUAL(size_t(1), sockMgr->mPlayAddrMap.count(pi1));
    CPPUNIT_ASSERT_EQUAL(size_t(1), sockMgr->mPlaySpawnMap.count(pi1));

    // Verify they are started
    CPPUNIT_ASSERT_EQUAL(true, pi0->IsStarted());
    CPPUNIT_ASSERT_EQUAL(true, pi1->IsStarted());

    // Verify endpoint -- no dest yet
    AddrInfo& addrInfo(sockMgr->mPlayAddrMap[pi0]);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0x7f000001), addrInfo.locAddr.get_ip_address());
    CPPUNIT_ASSERT_EQUAL(false, addrInfo.hasRem);

    // Force a spawn
    int fd = 2;
    ACE_INET_Addr remoteAddr(uint16_t(3), uint32_t(0x01020304));
    sockMgr->mPlaySpawnMap[pi0].delegate(pi0, fd, addrInfo.locAddr, remoteAddr);

    // Verify new non-spawning playlist instance is registered
    CPPUNIT_ASSERT_EQUAL(size_t(3), sockMgr->mPlayAddrMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sockMgr->mPlaySpawnMap.size());
    MockPlaylistInstance * pi2 = (++++playlistEngine.GetPlaylists().begin())->pi;
    CPPUNIT_ASSERT_EQUAL(size_t(1), sockMgr->mPlayAddrMap.count(pi2));
    CPPUNIT_ASSERT_EQUAL(size_t(0), sockMgr->mPlaySpawnMap.count(pi2));

    // Verify that it is started
    CPPUNIT_ASSERT_EQUAL(true, pi2->IsStarted());

    // Verify endpoint -- has dest yet
    AddrInfo& addrInfo2(sockMgr->mPlayAddrMap[pi2]);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0x7f000001), addrInfo2.locAddr.get_ip_address());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0x01020304), addrInfo2.remAddr.get_ip_address());
    CPPUNIT_ASSERT_EQUAL(true, addrInfo2.hasRem);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestDpgServerBlock);
