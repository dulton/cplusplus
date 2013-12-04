#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "DpgTestUtils.h"
#include "MockPlaylistEngine.h"
#include "engine/FlowEngine.h"

#include "RemAddrUtils.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestRemAddrUtils : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestRemAddrUtils);
    CPPUNIT_TEST(testGetServerPortSet);
    CPPUNIT_TEST(testGetAddrSet);
    CPPUNIT_TEST(testPortsOverlap);
    CPPUNIT_TEST(testNeeds);
    CPPUNIT_TEST_SUITE_END();

public:
    TestRemAddrUtils()
    {
    }

    ~TestRemAddrUtils()
    {
    }

    void setUp() 
    { 
    }

    void tearDown() 
    { 
    }
    
protected:
    void testGetServerPortSet();
    void testGetAddrSet();
    void testPortsOverlap();
    void testNeeds();

private:
    void initServerConfig(RemAddrUtils::serverConfig_t& config);
};

///////////////////////////////////////////////////////////////////////////////

void TestRemAddrUtils::initServerConfig(RemAddrUtils::serverConfig_t& config)
{
    DpgTestUtils::InitCommonProfile(&config.Common.Profile.L4L7Profile, "TestDpgServerBlock");
    config.Common.Endpoint.SrcIfHandle = 0;

    //empty struct -- config.ProtocolProfile;
    config.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestDpgServer";
    config.ProtocolConfig.MaxSimultaneousClients = 0;
}

///////////////////////////////////////////////////////////////////////////////

void TestRemAddrUtils::testGetServerPortSet()
{
    typedef RemAddrUtils::portSet_t portSet_t;
    portSet_t portSet;
    portSet_t expSet;

    // single flow, port 1234
    L4L7_ENGINE_NS::PlaylistConfig playlistConfig;
    playlistConfig.flows.push_back(L4L7_ENGINE_NS::PlaylistConfig::Flow());
    playlistConfig.flows[0].serverPort = 1234;

    RemAddrUtils::GetServerPortSet(portSet, playlistConfig);

    expSet.insert(1234);
    CPPUNIT_ASSERT(portSet == expSet);

    // verify input is cleared first
    portSet.clear();
    portSet.insert(9999); 

    RemAddrUtils::GetServerPortSet(portSet, playlistConfig);

    CPPUNIT_ASSERT(portSet == expSet);

    // many ports
    playlistConfig.flows.push_back(L4L7_ENGINE_NS::PlaylistConfig::Flow());
    playlistConfig.flows.push_back(L4L7_ENGINE_NS::PlaylistConfig::Flow());
    playlistConfig.flows.push_back(L4L7_ENGINE_NS::PlaylistConfig::Flow());
    playlistConfig.flows.push_back(L4L7_ENGINE_NS::PlaylistConfig::Flow());
    playlistConfig.flows.push_back(L4L7_ENGINE_NS::PlaylistConfig::Flow());
    playlistConfig.flows.push_back(L4L7_ENGINE_NS::PlaylistConfig::Flow());
    playlistConfig.flows[0].serverPort = 6666;
    playlistConfig.flows[1].serverPort = 2222;
    playlistConfig.flows[2].serverPort = 1111;
    playlistConfig.flows[3].serverPort = 7777;
    playlistConfig.flows[4].serverPort = 5555;
    playlistConfig.flows[5].serverPort = 3333;
    playlistConfig.flows[6].serverPort = 4444;
    
    RemAddrUtils::GetServerPortSet(portSet, playlistConfig);

    expSet.clear();
    expSet.insert(1111);
    expSet.insert(2222);
    expSet.insert(3333);
    expSet.insert(4444);
    expSet.insert(5555);
    expSet.insert(6666);
    expSet.insert(7777);
    CPPUNIT_ASSERT(portSet == expSet);

    // reversed flows are ignored

    playlistConfig.flows[1].reversed = true;
    playlistConfig.flows[3].reversed = true;
    playlistConfig.flows[5].reversed = true;

    RemAddrUtils::GetServerPortSet(portSet, playlistConfig);

    expSet.clear();
    expSet.insert(1111);
    expSet.insert(4444);
    expSet.insert(5555);
    expSet.insert(6666);
    CPPUNIT_ASSERT(portSet == expSet);
}

///////////////////////////////////////////////////////////////////////////////

void TestRemAddrUtils::testGetAddrSet()
{
    RemAddrUtils::addrSet_t addrSet;
    RemAddrUtils::stack_t dstIf;

    // single IPv4 - 127.0.0.99
    DpgTestUtils::InitServerIf(&dstIf);
    dstIf.Ipv4InterfaceList[0].Address.address[3] = 99;

    RemAddrUtils::GetAddrSet(addrSet, dstIf);
    CPPUNIT_ASSERT_EQUAL(size_t(1), addrSet.size());
    CPPUNIT_ASSERT_EQUAL(PF_INET, addrSet.begin()->get_type());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0x7f000063), addrSet.begin()->get_ip_address());

    // verify it's cleared first
    dstIf.Ipv4InterfaceList[0].Address.address[3] = 98;
    RemAddrUtils::GetAddrSet(addrSet, dstIf);
    CPPUNIT_ASSERT_EQUAL(size_t(1), addrSet.size());
    CPPUNIT_ASSERT_EQUAL(PF_INET, addrSet.begin()->get_type());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0x7f000062), addrSet.begin()->get_ip_address());

    // stepping by 0.0.0.1
    dstIf.Ipv4InterfaceList[0].NetworkInterface.TotalCount = 5;
    dstIf.Ipv4InterfaceList[0].Address.address[3] = 9;
    dstIf.Ipv4InterfaceList[0].AddrStep.address[3] = 1;

    RemAddrUtils::GetAddrSet(addrSet, dstIf);
    CPPUNIT_ASSERT_EQUAL(size_t(5), addrSet.size());
    CPPUNIT_ASSERT_EQUAL(PF_INET, addrSet.begin()->get_type());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0x7f000009), addrSet.begin()->get_ip_address());
    CPPUNIT_ASSERT_EQUAL(PF_INET, (++addrSet.begin())->get_type());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0x7f00000a), (++addrSet.begin())->get_ip_address());
    CPPUNIT_ASSERT_EQUAL(PF_INET, (++++addrSet.begin())->get_type());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0x7f00000b), (++++addrSet.begin())->get_ip_address());
    CPPUNIT_ASSERT_EQUAL(PF_INET, (++++++addrSet.begin())->get_type());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0x7f00000c), (++++++addrSet.begin())->get_ip_address());
    CPPUNIT_ASSERT_EQUAL(PF_INET, (++++++++addrSet.begin())->get_type());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0x7f00000d), (++++++++addrSet.begin())->get_ip_address());
    
    // NOTE: what about recycle count?
    
    // three IPv6 - 1234::5678, 1234::5679, 1234::567a
    dstIf.ifDescriptors[0].ifType = IFSETUP_NS::STC_DM_IFC_IPv6;
    dstIf.Ipv4InterfaceList.resize(0);
    dstIf.Ipv6InterfaceList.resize(1);
    Dpg_1_port_server::Ipv6If_t_clear(&dstIf.Ipv6InterfaceList[0]);
    dstIf.Ipv6InterfaceList[0].NetworkInterface.EmulatedIf.IsRange = true;
    dstIf.Ipv6InterfaceList[0].NetworkInterface.EmulatedIf.IsDirectlyConnected = true;
    dstIf.Ipv6InterfaceList[0].NetworkInterface.IfCountPerLowerIf = 1;
    // dstIf.Ipv6InterfaceList[0].NetworkInterface.IfRecycleCount = 0;
    dstIf.Ipv6InterfaceList[0].NetworkInterface.TotalCount = 3;
    // dstIf.Ipv6InterfaceList[0].NetworkInterface.BllHandle = 0;
    // dstIf.Ipv6InterfaceList[0].NetworkInterface.AffiliatedInterface = 0;
    // memset(&dstIf.Ipv6InterfaceList[0].Address.address[0], 0, 16);
    dstIf.Ipv6InterfaceList[0].Address.address[0] = 0x12;
    dstIf.Ipv6InterfaceList[0].Address.address[1] = 0x34;
    dstIf.Ipv6InterfaceList[0].Address.address[14] = 0x56;
    dstIf.Ipv6InterfaceList[0].Address.address[15] = 0x78;
    // memset(&dstIf.Ipv6InterfaceList[0].AddrStep.address[0], 0, 16);
    dstIf.Ipv6InterfaceList[0].AddrStep.address[15] = 1;
    memset(&dstIf.Ipv6InterfaceList[0].AddrStepMask.address[0], 0xff, 16);
    // dstIf.Ipv6InterfaceList[0].SkipReserved = false;
    // dstIf.Ipv6InterfaceList[0].AddrRepeatCount = 0;
    dstIf.Ipv6InterfaceList[0].PrefixLength = 8;

    RemAddrUtils::GetAddrSet(addrSet, dstIf);
    CPPUNIT_ASSERT_EQUAL(size_t(3), addrSet.size());
    CPPUNIT_ASSERT_EQUAL(PF_INET6, addrSet.begin()->get_type());
    char addrBuf[128];
    CPPUNIT_ASSERT(!addrSet.begin()->addr_to_string(addrBuf, sizeof(addrBuf)));
    CPPUNIT_ASSERT_EQUAL(std::string("[1234::5678]:0"), std::string(addrBuf));
    CPPUNIT_ASSERT_EQUAL(PF_INET6, (++addrSet.begin())->get_type());
    CPPUNIT_ASSERT(!(++addrSet.begin())->addr_to_string(addrBuf, sizeof(addrBuf)));
    CPPUNIT_ASSERT_EQUAL(std::string("[1234::5679]:0"), std::string(addrBuf));
    CPPUNIT_ASSERT_EQUAL(PF_INET6, (++++addrSet.begin())->get_type());
    CPPUNIT_ASSERT(!(++++addrSet.begin())->addr_to_string(addrBuf, sizeof(addrBuf)));
    CPPUNIT_ASSERT_EQUAL(std::string("[1234::567a]:0"), std::string(addrBuf));
}

///////////////////////////////////////////////////////////////////////////////

void TestRemAddrUtils::testPortsOverlap()
{
    typedef RemAddrUtils::portSet_t portSet_t;

    portSet_t newSet;
    portSet_t superSet;

    // test first set never overlaps
    newSet.insert(1);
    newSet.insert(3);
    newSet.insert(5);
    CPPUNIT_ASSERT_EQUAL(false, RemAddrUtils::PortsOverlap(superSet, newSet));

    // test superset now equals newSet
    CPPUNIT_ASSERT(newSet == superSet);

    // test adding same set overlaps
    CPPUNIT_ASSERT_EQUAL(true, RemAddrUtils::PortsOverlap(superSet, newSet));

    // test adding any value from that set overlaps
    newSet.clear();
    newSet.insert(1); 
    CPPUNIT_ASSERT_EQUAL(true, RemAddrUtils::PortsOverlap(superSet, newSet));

    newSet.clear();
    newSet.insert(3); 
    CPPUNIT_ASSERT_EQUAL(true, RemAddrUtils::PortsOverlap(superSet, newSet));

    newSet.clear();
    newSet.insert(5); 
    CPPUNIT_ASSERT_EQUAL(true, RemAddrUtils::PortsOverlap(superSet, newSet));

    // add non-overlap
    newSet.clear();
    newSet.insert(0);
    newSet.insert(2);
    newSet.insert(4);
    newSet.insert(6);
    CPPUNIT_ASSERT_EQUAL(false, RemAddrUtils::PortsOverlap(superSet, newSet));

    CPPUNIT_ASSERT_EQUAL(size_t(7), superSet.size());

    // add more non-overlap
    newSet.clear();
    newSet.insert(7);
    newSet.insert(8);
    newSet.insert(9);
    CPPUNIT_ASSERT_EQUAL(false, RemAddrUtils::PortsOverlap(superSet, newSet));

    CPPUNIT_ASSERT_EQUAL(size_t(10), superSet.size());

    // mostly non-overlap, but one overlap
    newSet.clear();
    newSet.insert(10);
    newSet.insert(11);
    newSet.insert(12);
    newSet.insert(7);
    CPPUNIT_ASSERT_EQUAL(true, RemAddrUtils::PortsOverlap(superSet, newSet));
}

///////////////////////////////////////////////////////////////////////////////

void TestRemAddrUtils::testNeeds()
{
    MockPlaylistEngine engine;
    RemAddrUtils::serverConfig_t config;
    L4L7_ENGINE_NS::PlaylistConfig playlistConfig;
    uint32_t playlistHandle0 = 100;
    uint32_t playlistHandle1 = 200;

    initServerConfig(config);

    // single playlist can't conflict
    config.Playlists.resize(1);
    config.Playlists[0].Handle = playlistHandle0;
    playlistConfig.flows.push_back(L4L7_ENGINE_NS::PlaylistConfig::Flow());
    engine.UpdatePlaylist(playlistHandle0, playlistConfig);  

    config.DstIfs.resize(1);
    DpgTestUtils::InitServerIf(&config.DstIfs[0]); // 127.0.0.1

    CPPUNIT_ASSERT_EQUAL(false, RemAddrUtils::NeedsAddrSet(engine, config));

    // if ports don't overlap, we're all good
    
    config.Playlists.resize(2);
    config.Playlists[0].Handle = playlistHandle0;
    config.Playlists[1].Handle = playlistHandle1;
    config.DstIfs.resize(2);

    // 127.0.0.1
    DpgTestUtils::InitServerIf(&config.DstIfs[0]);

    // 127.0.0.2
    DpgTestUtils::InitServerIf(&config.DstIfs[1]);
    config.DstIfs[0].Ipv4InterfaceList[0].Address.address[3] = 2;

    // single flow, port 1234
    playlistConfig.flows[0].serverPort = 1234;
    engine.UpdatePlaylist(playlistHandle0, playlistConfig);  

    playlistConfig.flows[0].serverPort = 1235;
    engine.UpdatePlaylist(playlistHandle1, playlistConfig);  

    CPPUNIT_ASSERT_EQUAL(false, RemAddrUtils::NeedsAddrSet(engine, config));

    // overlap ports, should fail
    playlistConfig.flows[0].serverPort = 1234;
    engine.UpdatePlaylist(playlistHandle1, playlistConfig);  

    CPPUNIT_ASSERT_EQUAL(true, RemAddrUtils::NeedsAddrSet(engine, config));

    // multiple flows per playlist, one overlap
    playlistConfig.flows.push_back(L4L7_ENGINE_NS::PlaylistConfig::Flow());
    playlistConfig.flows.push_back(L4L7_ENGINE_NS::PlaylistConfig::Flow());

    playlistConfig.flows[0].serverPort = 1234;
    playlistConfig.flows[1].serverPort = 1235;
    playlistConfig.flows[2].serverPort = 1236;
    engine.UpdatePlaylist(playlistHandle0, playlistConfig);  

    playlistConfig.flows[0].serverPort = 1237;
    playlistConfig.flows[1].serverPort = 1238;
    playlistConfig.flows[2].serverPort = 1236;
    engine.UpdatePlaylist(playlistHandle1, playlistConfig);  
    
    CPPUNIT_ASSERT_EQUAL(true, RemAddrUtils::NeedsAddrSet(engine, config));

    // multiple flows per playlist, overlap on the same playlist, not an issue
    
    playlistConfig.flows[0].serverPort = 1234;
    playlistConfig.flows[1].serverPort = 1234;
    playlistConfig.flows[2].serverPort = 1236;
    engine.UpdatePlaylist(playlistHandle0, playlistConfig);  

    playlistConfig.flows[0].serverPort = 1235;
    playlistConfig.flows[1].serverPort = 1237;
    playlistConfig.flows[2].serverPort = 1238;
    engine.UpdatePlaylist(playlistHandle1, playlistConfig);  

    CPPUNIT_ASSERT_EQUAL(false, RemAddrUtils::NeedsAddrSet(engine, config));

    // overlap but the flow is reversed (should ignore reversed flows)
    playlistConfig.flows[0].serverPort = 1234;
    playlistConfig.flows[1].serverPort = 1235;
    playlistConfig.flows[2].serverPort = 1236;
    engine.UpdatePlaylist(playlistHandle0, playlistConfig);  

    playlistConfig.flows[0].serverPort = 1237;
    playlistConfig.flows[1].serverPort = 1238;
    playlistConfig.flows[2].serverPort = 1236;
    playlistConfig.flows[2].reversed = true;
    engine.UpdatePlaylist(playlistHandle1, playlistConfig);  

    CPPUNIT_ASSERT_EQUAL(false, RemAddrUtils::NeedsAddrSet(engine, config));

    // un-reversed
    playlistConfig.flows[2].reversed = false;
    engine.UpdatePlaylist(playlistHandle1, playlistConfig);  
    CPPUNIT_ASSERT_EQUAL(true, RemAddrUtils::NeedsAddrSet(engine, config));

    // NOTE: technically UDP and TCP could overlap safely but we don't check
    // for that
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestRemAddrUtils);
