#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <ace/OS.h>
#include <boost/bind.hpp>

#include "MockEngine.h"
#include "MockPlaylistEngine.h"
#include "MockEngine.h"
#include "MockReactor.h"
#include "SocketManager.h"
#include "TcpPortUtils.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestSocketManager : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestSocketManager);
    CPPUNIT_TEST(testTcpConnect);
    CPPUNIT_TEST(testTcpListen);
    CPPUNIT_TEST(testTcpListeningListen);
    CPPUNIT_TEST(testTcpMultiListen);
    CPPUNIT_TEST(testTcpMultiFlowListen);
    CPPUNIT_TEST(testTcpMultiFlowSharedListen);
    CPPUNIT_TEST(testTcpMultiFlowMultiPlaylistListen);
    CPPUNIT_TEST(testTcpSendRecv);
    CPPUNIT_TEST(testTcpSendRecvOutOfOrder);
    CPPUNIT_TEST(testTcpSendRecvReassemble);
    CPPUNIT_TEST(testTcpSendRecvAlot);
    CPPUNIT_TEST(testUdpConnect);
#ifndef QUICK_TEST
    CPPUNIT_TEST(testUdpSendRecvExternal);
#endif
//    CPPUNIT_TEST(testUdpListen);
//    CPPUNIT_TEST(testUdpMultiListen);
//    CPPUNIT_TEST(testUdpListeningListen);
    CPPUNIT_TEST(testUdpMultiFlowMultiPlaylistListen);
    CPPUNIT_TEST(testUdpSendRecv);
    CPPUNIT_TEST_SUITE_END();

public:
    TestSocketManager() :
        mSelectTimeout(0,10000) // 10 ms
    {
    }

    ~TestSocketManager()
    {
    }

    void setUp() 
    { 
        L4L7_ENGINE_NS::PlaylistConfig config;
        mModifiers = new L4L7_ENGINE_NS::ModifierBlock(config, 0);
    }

    void tearDown() 
    { 
        delete mModifiers;
    }
    
protected:
    void testTcpConnect();
    void testTcpListen();
    void testTcpMultiListen();
    void testTcpMultiFlowListen();
    void testTcpMultiFlowSharedListen();
    void testTcpMultiFlowMultiPlaylistListen();
    void testTcpListeningListen();
    void testTcpSendRecv();
    void testTcpSendRecvOutOfOrder();
    void testTcpSendRecvReassemble();
    void testTcpSendRecvAlot();

    void testUdpConnect();
    void testUdpSendRecvExternal();
    void testUdpListen()              {}
    void testUdpMultiListen()         {}
    void testUdpMultiFlowMultiPlaylistListen();
    void testUdpListeningListen()     {}
    void testUdpSendRecv();
    void testUdpSendRecvReassemble()  {}
    void testUdpSendRecvAlot()        {}

private:
    L4L7_ENGINE_NS::ModifierBlock * mModifiers;
    ACE_Time_Value mSelectTimeout;
};

///////////////////////////////////////////////////////////////////////////////

void TestSocketManager::testTcpConnect()
{
    ACE_Reactor ioReactor;
    MockEngine engine;
    {
    SocketManager socketMgr(&ioReactor);
    MockPlaylistEngine pEngine(true);

    socketMgr.SetIpv4Tos(0xc0);
    socketMgr.SetTcpDelayedAck(true);

    L4L7_ENGINE_NS::PlaylistInstance * pi = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, true);
    
    std::string srcIfName = "lo";
    ACE_INET_Addr srcAddr(uint16_t(0), uint32_t(0x7f000001));
    ACE_INET_Addr dstAddr(uint16_t(0), uint32_t(0x7f000001));
    socketMgr.AddPlaylist(pi, srcIfName, srcAddr, dstAddr);

    // try to connect
    SocketManager::connectDelegate_t connDelegate;
    SocketManager::closeDelegate_t closeDelegate;
    bool isTcp = true;
    socketMgr.AsyncConnect(pi, 79, isTcp, 0, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    // should attempt to connect but be blocked
    // any misconfig would mean immediate error+close
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    // do select
    ioReactor.handle_events();

    // should close
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetCloses().size());

    // try to connect to a known site
    srcIfName = "eth0";
    srcAddr.set(uint16_t(0), INADDR_ANY);
    dstAddr.set(uint16_t(0), "jabber.ad.spirentcom.com");
    pi = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, true);
    engine.ResetMock();

    socketMgr.AddPlaylist(pi, srcIfName, srcAddr, dstAddr);
    int pendingFd = socketMgr.AsyncConnect(pi, 80, isTcp, 0, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    CPPUNIT_ASSERT(pendingFd != 0);

    // do select
    if (engine.GetConnects().empty())
        ioReactor.handle_events();

    // should have connected
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    // fds should match
    CPPUNIT_ASSERT_EQUAL(pendingFd, engine.GetConnects()[0].fd); 
    }

    // destruction should close
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetCloses().size());

// FIXME check the ports
// FIXME verify the close types
}

///////////////////////////////////////////////////////////////////////////////

void TestSocketManager::testUdpConnect()
{
    ACE_Reactor ioReactor;
    MockEngine engine;
    {
    SocketManager socketMgr(&ioReactor);
    MockPlaylistEngine pEngine(true);

    socketMgr.SetIpv4Tos(0xc0);
    socketMgr.SetTcpDelayedAck(true);

    L4L7_ENGINE_NS::PlaylistInstance * pi = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, true);
    
    std::string srcIfName = "lo";
    ACE_INET_Addr srcAddr(uint16_t(0), uint32_t(0x7f000001));
    ACE_INET_Addr dstAddr(uint16_t(0), uint32_t(0x7f000001));
    socketMgr.AddPlaylist(pi, srcIfName, srcAddr, dstAddr);

    // try to connect
    SocketManager::connectDelegate_t connDelegate;
    SocketManager::closeDelegate_t closeDelegate;
    bool isTcp = false;
    int pendingFd = socketMgr.AsyncConnect(pi, 40003, isTcp, 0, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    CPPUNIT_ASSERT(pendingFd != 0);

    // do select
    ioReactor.handle_events();

    // should have connected
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    // fds should match
    CPPUNIT_ASSERT_EQUAL(pendingFd, engine.GetConnects()[0].fd); 
    }

    // destruction should close
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetCloses().size());

// FIXME check the ports
// FIXME verify the close types
}

///////////////////////////////////////////////////////////////////////////////

void TestSocketManager::testUdpSendRecvExternal()
{
    ACE_Reactor ioReactor;
    MockEngine engine;
    {
    SocketManager socketMgr(&ioReactor);
    MockPlaylistEngine pEngine(true);

    socketMgr.SetIpv4Tos(0xc0);
    socketMgr.SetTcpDelayedAck(true);

    L4L7_ENGINE_NS::PlaylistInstance * pi = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, true);
    
    std::string srcIfName = "eth0";
    ACE_INET_Addr srcAddr(uint16_t(0), INADDR_ANY);
    std::string chassis = "10.14.16.20"; // any chassis will do
    ACE_INET_Addr dstAddr(uint16_t(0), chassis.c_str()); 
    socketMgr.AddPlaylist(pi, srcIfName, srcAddr, dstAddr);

    // try to connect to a known site
    SocketManager::connectDelegate_t connDelegate;
    SocketManager::closeDelegate_t closeDelegate;
    bool isTcp = false;
    socketMgr.AsyncConnect(pi, 40004 /* Spirent */, isTcp, 0, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    // do select
    ioReactor.handle_events();

    // should have connected
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    int serial = engine.GetConnects()[0].fd;

    socketMgr.AsyncSend(serial, (const uint8_t*)"AreYouSpirent\0", 13, engine.GetSendDelegate());
    socketMgr.AsyncRecv(serial, 10, engine.GetRecvDelegate());

    // do select
    int loop = 0;
    const int max_loop = 500;
    while(engine.GetRecvs().empty() && ++loop < max_loop)
    {
        ioReactor.handle_events(mSelectTimeout);
        usleep(1000);
    }

    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetSends().size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Nothing received from " + chassis, size_t(1), engine.GetRecvs().size());

    CPPUNIT_ASSERT_EQUAL(size_t(10), engine.GetRecvs()[0].data.size());
    CPPUNIT_ASSERT_EQUAL(std::string("IAmSpirent"), std::string((char *)&engine.GetRecvs()[0].data[0], 10));
    }

    // destruction should close
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetCloses().size());

// FIXME check the ports
// FIXME verify the close types
}

///////////////////////////////////////////////////////////////////////////////

void TestSocketManager::testTcpListen()
{
    ACE_Reactor ioReactor;
    MockEngine engine;
    {
    SocketManager socketMgr(&ioReactor);
    MockPlaylistEngine pEngine(true);

    socketMgr.SetIpv4Tos(0xc0);
    socketMgr.SetTcpDelayedAck(true);

    L4L7_ENGINE_NS::PlaylistInstance * pi = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, true);
    
    std::string srcIfName = "lo";
    ACE_INET_Addr srcAddr(uint16_t(0), uint32_t(0x7f000001));
    ACE_INET_Addr dstAddr(uint16_t(0), uint32_t(0x7f000001));
    socketMgr.AddPlaylist(pi, srcIfName, srcAddr, dstAddr);

    uint16_t serverPort = 12342;
    int flowIdx = 3;

    // start listening
    bool isTcp = true;
    socketMgr.Listen(pi, serverPort, isTcp, flowIdx, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    
    // nothing should happen
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());
    
    // try to connect
    socketMgr.AsyncConnect(pi, serverPort, isTcp, flowIdx, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    // should attempt to connect but be blocked
    // any misconfig would mean immediate error+close
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    // do select
    while (engine.GetConnects().empty() || engine.GetAccepts().empty())
    {
        // do select
        ioReactor.handle_events();
    }

    // should have connected
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    int clientSerial = engine.GetConnects()[0].fd;
    int serverSerial = engine.GetAccepts()[0].fd;

    CPPUNIT_ASSERT(clientSerial != serverSerial);
    }

    // destruction should close
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetCloses().size());

// FIXME check the ports
// FIXME verify the close types
}

///////////////////////////////////////////////////////////////////////////////

void TestSocketManager::testTcpMultiListen()
{
    ACE_Reactor ioReactor;
    MockEngine engine;
    {
    SocketManager socketMgr(&ioReactor);
    MockPlaylistEngine pEngine(true);

    socketMgr.SetIpv4Tos(0xc0);
    socketMgr.SetTcpDelayedAck(true);

    L4L7_ENGINE_NS::PlaylistInstance * pi = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, true);
    
    std::string srcIfName = "lo";
    ACE_INET_Addr srcAddr(uint16_t(0), uint32_t(0x7f000001));
    ACE_INET_Addr dstAddr(uint16_t(0), uint32_t(0x7f000001));
    socketMgr.AddPlaylist(pi, srcIfName, srcAddr, dstAddr);

    // two share the same underlying acceptor/port, one does not
    uint16_t serverPort0 = 12343;
    uint16_t serverPort1 = 12343;
    uint16_t serverPort2 = 12344;
    int flowIdx0 = 3;
    int flowIdx1 = 4;
    int flowIdx2 = 5;
    bool isTcp = true;

    // start listening
    socketMgr.Listen(pi, serverPort0, isTcp, flowIdx0, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    socketMgr.Listen(pi, serverPort1, isTcp, flowIdx1, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    socketMgr.Listen(pi, serverPort2, isTcp, flowIdx2, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    
    // nothing should happen
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());
    
    // try to connect
    socketMgr.AsyncConnect(pi, serverPort0, isTcp, flowIdx0, engine.GetConnectDelegate(), engine.GetCloseDelegate());
    socketMgr.AsyncConnect(pi, serverPort1, isTcp, flowIdx1, engine.GetConnectDelegate(), engine.GetCloseDelegate());
    socketMgr.AsyncConnect(pi, serverPort2, isTcp, flowIdx2, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    // should attempt to connect but be blocked
    // any misconfig would mean immediate error+close
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    const size_t exp_conn = 3;
    const size_t exp_acpt = 3;

    // do select
    int events = 0;
    const int MAX_EVENTS = 100;
    while ((engine.GetConnects().size() < exp_conn || engine.GetAccepts().size() < exp_acpt) &&
           ++events < MAX_EVENTS)
    {
        // do select
        ioReactor.handle_events();
    }

    // should have connected
    CPPUNIT_ASSERT_EQUAL(exp_conn, engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(exp_acpt, engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    int clientSerial = engine.GetConnects()[0].fd;
    int serverSerial = engine.GetAccepts()[0].fd;
    CPPUNIT_ASSERT(clientSerial != serverSerial);

    clientSerial = engine.GetConnects()[1].fd;
    serverSerial = engine.GetAccepts()[1].fd;
    CPPUNIT_ASSERT(clientSerial != serverSerial);

    clientSerial = engine.GetConnects()[2].fd;
    serverSerial = engine.GetAccepts()[2].fd;
    CPPUNIT_ASSERT(clientSerial != serverSerial);
    }

    // destruction should close
    CPPUNIT_ASSERT_EQUAL(size_t(6), engine.GetCloses().size());

// FIXME check the ports
// FIXME verify the close types
}

///////////////////////////////////////////////////////////////////////////////

void TestSocketManager::testTcpMultiFlowListen()
{
    ACE_Reactor ioReactor;
    MockEngine engine;
    {
    SocketManager socketMgr(&ioReactor);
    MockPlaylistEngine pEngine(true);

    socketMgr.SetIpv4Tos(0xc0);
    socketMgr.SetTcpDelayedAck(true);

    L4L7_ENGINE_NS::PlaylistInstance * piListen = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, false);
    
    L4L7_ENGINE_NS::PlaylistInstance * piConnect = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, true);

    std::string srcIfName = "lo";
    ACE_INET_Addr srcAddr(uint16_t(0), uint32_t(0x7f000001));
    ACE_INET_Addr dstAddr(uint16_t(0), uint32_t(0x7f000001));
    SocketManager::addrSet_t remAddrSet;
    socketMgr.AddListeningPlaylist(piListen, srcIfName, srcAddr, remAddrSet, engine.GetSpawnAcceptDelegate());
    socketMgr.AddPlaylist(piConnect, srcIfName, srcAddr, dstAddr);

    uint16_t serverPort0 = 12345;
    uint16_t serverPort1 = 12346;
    uint16_t serverPort2 = 12347;
    int flowIdx0 = 3;
    int flowIdx1 = 4;
    int flowIdx2 = 5;
    bool isTcp = true;

    // start listening
    socketMgr.Listen(piListen, serverPort0, isTcp, flowIdx0, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    socketMgr.Listen(piListen, serverPort1, isTcp, flowIdx1, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    socketMgr.Listen(piListen, serverPort2, isTcp, flowIdx2, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    
    // nothing should happen
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());
    
    // try to connect first connection
    socketMgr.AsyncConnect(piConnect, serverPort0, isTcp, flowIdx0, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    // should attempt to connect but be blocked
    // any misconfig would mean immediate error+close
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    // do select
    while (engine.GetConnects().empty() || engine.GetSpawnAccepts().empty())
    {
        // do select
        ioReactor.handle_events();
    }

    // should have connected one
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    int clientSerial = engine.GetConnects()[0].fd;
    int serverSerial = engine.GetSpawnAccepts()[0].fd;
    CPPUNIT_ASSERT(clientSerial != serverSerial);

    CPPUNIT_ASSERT(piListen == engine.GetSpawnAccepts()[0].pi);

    // register new spawned playlist
    L4L7_ENGINE_NS::PlaylistInstance * piSpawned = pEngine.ClonePlaylist(piListen, *mModifiers);
    socketMgr.AddChildPlaylist(piSpawned, piListen, engine.GetSpawnAccepts()[0].remoteAddr); 

    // new playlist starts listening
    socketMgr.Listen(piSpawned, serverPort0, isTcp, flowIdx0, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    socketMgr.Listen(piSpawned, serverPort1, isTcp, flowIdx1, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    socketMgr.Listen(piSpawned, serverPort2, isTcp, flowIdx2, engine.GetAcceptDelegate(), engine.GetCloseDelegate());

    // forward initial connection to new playlist
    CPPUNIT_ASSERT(socketMgr.Accept(piSpawned, engine.GetSpawnAccepts()[0].fd));

    // check it was forwarded
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetAccepts().size());
    int spawnSerial = engine.GetAccepts()[0].fd;
    CPPUNIT_ASSERT_EQUAL(serverSerial, spawnSerial);

    // make other connections
    socketMgr.AsyncConnect(piConnect, serverPort1, isTcp, flowIdx1, engine.GetConnectDelegate(), engine.GetCloseDelegate());
    socketMgr.AsyncConnect(piConnect, serverPort2, isTcp, flowIdx2, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    // should attempt to connect but be blocked
    // any misconfig would mean immediate error+close
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    const size_t exp_conn = 3;
    const size_t exp_acpt = 3;

    // do select
    int events = 0;
    const int MAX_EVENTS = 100;
    while ((engine.GetConnects().size() < exp_conn || engine.GetAccepts().size() < exp_acpt) &&
            ++events < MAX_EVENTS)
    {
        // do select
        ioReactor.handle_events();
    }

    // check they were forwarded and NOT spawned
    CPPUNIT_ASSERT_EQUAL(exp_acpt, engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(exp_conn, engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());
    }

    // destruction should close
    CPPUNIT_ASSERT_EQUAL(size_t(6), engine.GetCloses().size());

// FIXME check the ports
// FIXME verify the close types
}

///////////////////////////////////////////////////////////////////////////////

void TestSocketManager::testTcpMultiFlowSharedListen()
{
    // Same as above test but with two flows on the same server port
    ACE_Reactor ioReactor;
    MockEngine engine;
    {
    SocketManager socketMgr(&ioReactor);
    MockPlaylistEngine pEngine(true);

    socketMgr.SetIpv4Tos(0xc0);
    socketMgr.SetTcpDelayedAck(true);

    L4L7_ENGINE_NS::PlaylistInstance * piListen = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, false);
    
    L4L7_ENGINE_NS::PlaylistInstance * piConnect = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, true);

    std::string srcIfName = "lo";
    ACE_INET_Addr srcAddr(uint16_t(0), uint32_t(0x7f000001));
    ACE_INET_Addr dstAddr(uint16_t(0), uint32_t(0x7f000001));
    SocketManager::addrSet_t remAddrSet;
    socketMgr.AddListeningPlaylist(piListen, srcIfName, srcAddr, remAddrSet, engine.GetSpawnAcceptDelegate());
    socketMgr.AddPlaylist(piConnect, srcIfName, srcAddr, dstAddr);

    // two share the same underlying acceptor/port, one does not
    uint16_t serverPort0 = 12348;
    uint16_t serverPort1 = 12348;
    uint16_t serverPort2 = 12349;
    int flowIdx0 = 3;
    int flowIdx1 = 4;
    int flowIdx2 = 5;
    bool isTcp = true;

    // start listening
    socketMgr.Listen(piListen, serverPort0, isTcp, flowIdx0, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    socketMgr.Listen(piListen, serverPort1, isTcp, flowIdx1, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    socketMgr.Listen(piListen, serverPort2, isTcp, flowIdx2, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    
    // nothing should happen
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());
    
    // try to connect first connection
    socketMgr.AsyncConnect(piConnect, serverPort0, isTcp, flowIdx0, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    // should attempt to connect but be blocked
    // any misconfig would mean immediate error+close
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    // do select
    while (engine.GetConnects().empty() || engine.GetSpawnAccepts().empty())
    {
        // do select
        ioReactor.handle_events();
    }

    // should have connected one
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    int clientSerial = engine.GetConnects()[0].fd;
    int serverSerial = engine.GetSpawnAccepts()[0].fd;
    CPPUNIT_ASSERT(clientSerial != serverSerial);

    CPPUNIT_ASSERT(piListen == engine.GetSpawnAccepts()[0].pi);

    // register new spawned playlist
    L4L7_ENGINE_NS::PlaylistInstance * piSpawned = pEngine.ClonePlaylist(piListen, *mModifiers);
    socketMgr.AddChildPlaylist(piSpawned, piListen, engine.GetSpawnAccepts()[0].remoteAddr); 

    // new playlist starts listening
    socketMgr.Listen(piSpawned, serverPort0, isTcp, flowIdx0, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    socketMgr.Listen(piSpawned, serverPort1, isTcp, flowIdx1, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    socketMgr.Listen(piSpawned, serverPort2, isTcp, flowIdx2, engine.GetAcceptDelegate(), engine.GetCloseDelegate());

    // forward initial connection to new playlist
    CPPUNIT_ASSERT(socketMgr.Accept(piSpawned, engine.GetSpawnAccepts()[0].fd));

    // check it was forwarded
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetAccepts().size());
    int spawnSerial = engine.GetAccepts()[0].fd;
    CPPUNIT_ASSERT_EQUAL(serverSerial, spawnSerial);

    // make other connections
    socketMgr.AsyncConnect(piConnect, serverPort1, isTcp, flowIdx1, engine.GetConnectDelegate(), engine.GetCloseDelegate());
    socketMgr.AsyncConnect(piConnect, serverPort2, isTcp, flowIdx2, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    // should attempt to connect but be blocked
    // any misconfig would mean immediate error+close
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    const size_t exp_conn = 3;
    const size_t exp_acpt = 3;

    // do select
    int events = 0;
    const int MAX_EVENTS = 100;
    while ((engine.GetConnects().size() < exp_conn || engine.GetAccepts().size() < exp_acpt) &&
           ++events < MAX_EVENTS)
    {
        // do select
        ioReactor.handle_events();
    }

    // check they were forwarded and NOT spawned
    CPPUNIT_ASSERT_EQUAL(exp_acpt, engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(exp_conn, engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());
    }

    // destruction should close
    CPPUNIT_ASSERT_EQUAL(size_t(6), engine.GetCloses().size());

// FIXME check the ports
// FIXME verify the close types
}

///////////////////////////////////////////////////////////////////////////////

void TestSocketManager::testTcpMultiFlowMultiPlaylistListen()
{
    // Same as above test but with two playlists on the same server port
    ACE_Reactor ioReactor;
    MockEngine engine;
    {
    SocketManager socketMgr(&ioReactor);
    MockPlaylistEngine pEngine(true);

    socketMgr.SetIpv4Tos(0xc0);
    socketMgr.SetTcpDelayedAck(true);

    L4L7_ENGINE_NS::PlaylistInstance * piListen0 = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, false);
    L4L7_ENGINE_NS::PlaylistInstance * piListen1 = pEngine.MakePlaylist(uint32_t(2), *mModifiers, socketMgr, false);
    
    L4L7_ENGINE_NS::PlaylistInstance * piConnect0 = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, true);
    L4L7_ENGINE_NS::PlaylistInstance * piConnect1 = pEngine.MakePlaylist(uint32_t(2), *mModifiers, socketMgr, true);

    std::string ifName = "lo";
    ACE_INET_Addr serverAddr(uint16_t(0), uint32_t(0x7f000001));
    ACE_INET_Addr clientAddr0(uint16_t(0), uint32_t(0x7f000002));
    ACE_INET_Addr clientAddr1(uint16_t(0), uint32_t(0x7f000003));
    SocketManager::addrSet_t remAddrSet0; remAddrSet0.insert(clientAddr0);
    SocketManager::addrSet_t remAddrSet1; remAddrSet1.insert(clientAddr1);
    socketMgr.AddListeningPlaylist(piListen0, ifName, serverAddr, remAddrSet0, engine.GetSpawnAcceptDelegate());
    socketMgr.AddListeningPlaylist(piListen1, ifName, serverAddr, remAddrSet1, engine.GetSpawnAcceptDelegate());
    socketMgr.AddPlaylist(piConnect0, ifName, clientAddr0, serverAddr);
    socketMgr.AddPlaylist(piConnect1, ifName, clientAddr1, serverAddr);

    // two share the same underlying acceptor/port, one does not
    uint16_t serverPort0 = 12350;
    int flowIdx0 = 3;
    int flowIdx1 = 4;
    bool isTcp = true;

    // start listening
    socketMgr.Listen(piListen0, serverPort0, isTcp, flowIdx0, engine.GetAcceptDelegate(), engine.GetCloseDelegate());

    socketMgr.Listen(piListen1, serverPort0, isTcp, flowIdx0, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    
    // nothing should happen
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());
    
    // try to connect first connection
    socketMgr.AsyncConnect(piConnect0, serverPort0, isTcp, flowIdx0, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    // should attempt to connect but be blocked
    // any misconfig would mean immediate error+close
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    // do select
    while (engine.GetConnects().empty() || engine.GetSpawnAccepts().empty())
    {
        // do select
        ioReactor.handle_events();
    }

    // should have connected one
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    int clientSerial = engine.GetConnects()[0].fd;
    int serverSerial = engine.GetSpawnAccepts()[0].fd;
    CPPUNIT_ASSERT(clientSerial != serverSerial);

    // check that playlist is correct based on remote address
    CPPUNIT_ASSERT(piListen0 == engine.GetSpawnAccepts()[0].pi);

    // register new spawned playlist
    L4L7_ENGINE_NS::PlaylistInstance * piSpawned0 = pEngine.ClonePlaylist(piListen0, *mModifiers);
    socketMgr.AddChildPlaylist(piSpawned0, piListen0, engine.GetSpawnAccepts()[0].remoteAddr); 

    // new playlist starts listening
    socketMgr.Listen(piSpawned0, serverPort0, isTcp, flowIdx0, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    socketMgr.Listen(piSpawned0, serverPort0, isTcp, flowIdx1, engine.GetAcceptDelegate(), engine.GetCloseDelegate());

    // forward initial connection to new playlist
    CPPUNIT_ASSERT(socketMgr.Accept(piSpawned0, engine.GetSpawnAccepts()[0].fd));

    // check it was forwarded
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetAccepts().size());
    int spawnSerial = engine.GetAccepts()[0].fd;
    CPPUNIT_ASSERT_EQUAL(serverSerial, spawnSerial);


    // try to connect other playlist
    socketMgr.AsyncConnect(piConnect1, serverPort0, isTcp, flowIdx0, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    // should attempt to connect but be blocked
    // any misconfig would mean immediate error+close
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    size_t exp_conn = 2;
    const size_t exp_spwn = 2;

    // do select
    int events = 0;
    const int MAX_EVENTS = 100;
    while ((engine.GetConnects().size() < exp_conn || engine.GetSpawnAccepts().size() < exp_spwn) &&
            ++events < MAX_EVENTS)
    {
        // do select
        ioReactor.handle_events();
    }

    // should have connected another
    CPPUNIT_ASSERT_EQUAL(exp_conn, engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(exp_spwn, engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    clientSerial = engine.GetConnects()[1].fd;
    serverSerial = engine.GetSpawnAccepts()[1].fd;
    CPPUNIT_ASSERT(clientSerial != serverSerial);

    // check that playlist is correct based on remote address
    CPPUNIT_ASSERT(piListen1 == engine.GetSpawnAccepts()[1].pi);

    // register new spawned playlist
    L4L7_ENGINE_NS::PlaylistInstance * piSpawned1 = pEngine.ClonePlaylist(piListen1, *mModifiers);

    socketMgr.AddChildPlaylist(piSpawned1, piListen1, engine.GetSpawnAccepts()[1].remoteAddr); 

    // new playlist starts listening
    socketMgr.Listen(piSpawned1, serverPort0, isTcp, flowIdx0, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    socketMgr.Listen(piSpawned1, serverPort0, isTcp, flowIdx1, engine.GetAcceptDelegate(), engine.GetCloseDelegate());

    // forward initial connection to new playlist
    CPPUNIT_ASSERT(socketMgr.Accept(piSpawned1, engine.GetSpawnAccepts()[1].fd));

    // check it was forwarded
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetAccepts().size());
    spawnSerial = engine.GetAccepts()[1].fd;
    CPPUNIT_ASSERT_EQUAL(serverSerial, spawnSerial);
   

    // make other connections
    socketMgr.AsyncConnect(piConnect0, serverPort0, isTcp, flowIdx1, engine.GetConnectDelegate(), engine.GetCloseDelegate());
    socketMgr.AsyncConnect(piConnect1, serverPort0, isTcp, flowIdx1, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    // should attempt to connect but be blocked
    // any misconfig would mean immediate error+close
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    exp_conn = 4;
    const size_t exp_acpt = 4;

    // do select
    events = 0;
    while ((engine.GetConnects().size() < exp_conn || engine.GetAccepts().size() < exp_acpt) &&
           ++events < MAX_EVENTS)
    {
        // do select
        ioReactor.handle_events();
    }

    // check they were forwarded and NOT spawned
    CPPUNIT_ASSERT_EQUAL(exp_acpt, engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(exp_conn, engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());
    }

    // destruction should close
    CPPUNIT_ASSERT_EQUAL(size_t(8), engine.GetCloses().size());
}

///////////////////////////////////////////////////////////////////////////////

void TestSocketManager::testTcpListeningListen()
{
    ACE_Reactor ioReactor;
    MockEngine engine;
    {
    SocketManager socketMgr(&ioReactor);
    MockPlaylistEngine pEngine(true);

    socketMgr.SetIpv4Tos(0xc0);
    socketMgr.SetTcpDelayedAck(true);

    L4L7_ENGINE_NS::PlaylistInstance * piListen = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, false);
    
    L4L7_ENGINE_NS::PlaylistInstance * piConnect = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, true);

    std::string srcIfName = "lo";
    ACE_INET_Addr srcAddr(uint16_t(0), uint32_t(0x7f000001));
    ACE_INET_Addr dstAddr(uint16_t(0), uint32_t(0x7f000001));
    SocketManager::addrSet_t remAddrSet;
    socketMgr.AddListeningPlaylist(piListen, srcIfName, srcAddr, remAddrSet, engine.GetSpawnAcceptDelegate());
    socketMgr.AddPlaylist(piConnect, srcIfName, srcAddr, dstAddr);

    // two share the same underlying acceptor/port, one does not
    uint16_t serverPort0 = 12351;
    uint16_t serverPort1 = 12351;
    uint16_t serverPort2 = 12352;
    int flowIdx0 = 3;
    int flowIdx1 = 4;
    int flowIdx2 = 5;
    bool isTcp = true;

    // start listening
    socketMgr.Listen(piListen, serverPort0, isTcp, flowIdx0, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    socketMgr.Listen(piListen, serverPort1, isTcp, flowIdx1, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    socketMgr.Listen(piListen, serverPort2, isTcp, flowIdx2, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    
    // nothing should happen
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());
    
    // try to connect
    socketMgr.AsyncConnect(piConnect, serverPort0, isTcp, flowIdx0, engine.GetConnectDelegate(), engine.GetCloseDelegate());
    socketMgr.AsyncConnect(piConnect, serverPort1, isTcp, flowIdx1, engine.GetConnectDelegate(), engine.GetCloseDelegate());
    socketMgr.AsyncConnect(piConnect, serverPort2, isTcp, flowIdx2, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    // should attempt to connect but be blocked
    // any misconfig would mean immediate error+close
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    const size_t exp_conn = 3;
    const size_t exp_spwn = 3;

    // do select
    int events = 0;
    const int MAX_EVENTS = 100;
    while ((engine.GetConnects().size() < exp_conn || engine.GetSpawnAccepts().size() < exp_spwn) &&
            ++events < MAX_EVENTS)
    {
        // do select
        ioReactor.handle_events();
    }

    // should have connected
    CPPUNIT_ASSERT_EQUAL(exp_conn, engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(exp_spwn, engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    int clientSerial = engine.GetConnects()[0].fd;
    int serverSerial = engine.GetSpawnAccepts()[0].fd;
    CPPUNIT_ASSERT(clientSerial != serverSerial);

    clientSerial = engine.GetConnects()[1].fd;
    serverSerial = engine.GetSpawnAccepts()[1].fd;
    CPPUNIT_ASSERT(clientSerial != serverSerial);

    clientSerial = engine.GetConnects()[2].fd;
    serverSerial = engine.GetSpawnAccepts()[2].fd;
    CPPUNIT_ASSERT(clientSerial != serverSerial);

    CPPUNIT_ASSERT(piListen == engine.GetSpawnAccepts()[0].pi);
    CPPUNIT_ASSERT(piListen == engine.GetSpawnAccepts()[1].pi);
    CPPUNIT_ASSERT(piListen == engine.GetSpawnAccepts()[2].pi);
    }

    // destruction should close - but listening/spawning playlists don't delegate closes
    CPPUNIT_ASSERT_EQUAL(size_t(3), engine.GetCloses().size());

// FIXME check the ports
// FIXME verify the close types
}

///////////////////////////////////////////////////////////////////////////////

void TestSocketManager::testTcpSendRecv()
{
    ACE_Reactor ioReactor;
    MockEngine engine;
    {
    SocketManager socketMgr(&ioReactor);
    MockPlaylistEngine pEngine(true);

    socketMgr.SetIpv4Tos(0xc0);
    socketMgr.SetTcpDelayedAck(true);

    L4L7_ENGINE_NS::PlaylistInstance * pi = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, true);
    
    std::string srcIfName = "lo";
    ACE_INET_Addr srcAddr(uint16_t(0), uint32_t(0x7f000001));
    ACE_INET_Addr dstAddr(uint16_t(0), uint32_t(0x7f000001));
    socketMgr.AddPlaylist(pi, srcIfName, srcAddr, dstAddr);

    uint16_t serverPort = 12353;
    int flowIdx = 3;
    bool isTcp = true;

    // start listening
    socketMgr.Listen(pi, serverPort, isTcp, flowIdx, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    
    // try to connect
    socketMgr.AsyncConnect(pi, serverPort, isTcp, flowIdx, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    while (engine.GetConnects().empty() || engine.GetAccepts().empty())
    {
        // do select
        ioReactor.handle_events();
    }

    // should have connected
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetSends().size());

    int clientSerial = engine.GetConnects()[0].fd;
    int serverSerial = engine.GetAccepts()[0].fd;

    socketMgr.AsyncSend(clientSerial, (const uint8_t*)"foobar\0", 7, engine.GetSendDelegate());
    socketMgr.AsyncRecv(serverSerial, 7, engine.GetRecvDelegate());

    // do select
    {
        usleep(1000);
        ioReactor.handle_events();
    }

    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetRecvs().size());

    CPPUNIT_ASSERT_EQUAL(size_t(7), engine.GetRecvs()[0].data.size());
    CPPUNIT_ASSERT_EQUAL(std::string("foobar"), std::string((char *)&engine.GetRecvs()[0].data[0]));
    }

    // destruction should close
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetCloses().size());
}

///////////////////////////////////////////////////////////////////////////////

void TestSocketManager::testUdpSendRecv()
{
    ACE_Reactor ioReactor;
    MockEngine engine;
    {
    SocketManager socketMgr(&ioReactor);
    MockPlaylistEngine pEngine(true);

    socketMgr.SetIpv4Tos(0xc0);

    L4L7_ENGINE_NS::PlaylistInstance * piC = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, true);

    L4L7_ENGINE_NS::PlaylistInstance * piS = pEngine.MakePlaylist(uint32_t(2), *mModifiers, socketMgr, true);
    
    std::string srcIfName = "lo";
    int flowIdx0 = 3, flowIdx1 = 4;
    uint16_t clientPort = TcpPortUtils::MakePort(flowIdx0, piC);
    uint16_t serverPort = TcpPortUtils::MakePort(flowIdx1, piS);
    ACE_INET_Addr clientAddr(clientPort, uint32_t(0x7f000001));
    ACE_INET_Addr serverAddr(serverPort, uint32_t(0x7f000001));
    socketMgr.AddPlaylist(piC, srcIfName, clientAddr, serverAddr);
    socketMgr.AddPlaylist(piS, srcIfName, serverAddr, clientAddr);

    bool isTcp = false;

    // connect
    socketMgr.AsyncConnect(piC, serverPort, isTcp, flowIdx0, engine.GetConnectDelegate(), engine.GetCloseDelegate());
    socketMgr.AsyncConnect(piS, clientPort, isTcp, flowIdx1, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    while (engine.GetConnects().size() != 2)
    {
        // do select
        ioReactor.handle_events();
    }

    // should have connected
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetRecvs().size());

    int clientSerial = engine.GetConnects()[0].fd;
    int serverSerial = engine.GetConnects()[1].fd;

    socketMgr.AsyncSend(clientSerial, (const uint8_t*)"foobar\0", 7, engine.GetSendDelegate());
    socketMgr.AsyncRecv(serverSerial, 7, engine.GetRecvDelegate());

    // do select
    while(engine.GetSends().empty() || engine.GetRecvs().empty())
    {
        usleep(1000);
        ioReactor.handle_events();
    }

    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetRecvs().size());

    CPPUNIT_ASSERT_EQUAL(size_t(7), engine.GetRecvs()[0].data.size());
    CPPUNIT_ASSERT_EQUAL(std::string("foobar"), std::string((char *)&engine.GetRecvs()[0].data[0]));
    }

    // destruction should close
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetCloses().size());
}

///////////////////////////////////////////////////////////////////////////////

void TestSocketManager::testUdpMultiFlowMultiPlaylistListen()
{
    // Same as above test but with two playlists on the same server port
    ACE_Reactor ioReactor;
    MockEngine engine;
    {
    SocketManager socketMgr(&ioReactor);
    MockPlaylistEngine pEngine(true);

    socketMgr.SetIpv4Tos(0xc0);
    socketMgr.SetTcpDelayedAck(true);

    L4L7_ENGINE_NS::PlaylistInstance * piListen0 = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, false);
    L4L7_ENGINE_NS::PlaylistInstance * piListen1 = pEngine.MakePlaylist(uint32_t(2), *mModifiers, socketMgr, false);
    
    L4L7_ENGINE_NS::PlaylistInstance * piConnect0 = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, true);
    L4L7_ENGINE_NS::PlaylistInstance * piConnect1 = pEngine.MakePlaylist(uint32_t(2), *mModifiers, socketMgr, true);

    std::string ifName = "lo";
    ACE_INET_Addr serverAddr(uint16_t(0), uint32_t(0x7f000001));
    ACE_INET_Addr clientAddr0(uint16_t(0), uint32_t(0x7f000002));
    ACE_INET_Addr clientAddr1(uint16_t(0), uint32_t(0x7f000003));
    SocketManager::addrSet_t remAddrSet0; remAddrSet0.insert(clientAddr0);
    SocketManager::addrSet_t remAddrSet1; remAddrSet1.insert(clientAddr1);
    socketMgr.AddListeningPlaylist(piListen0, ifName, serverAddr, remAddrSet0, engine.GetSpawnAcceptDelegate());
    socketMgr.AddListeningPlaylist(piListen1, ifName, serverAddr, remAddrSet1, engine.GetSpawnAcceptDelegate());
    socketMgr.AddPlaylist(piConnect0, ifName, clientAddr0, serverAddr);
    socketMgr.AddPlaylist(piConnect1, ifName, clientAddr1, serverAddr);

    // two share the same underlying acceptor/port, one does not
    uint16_t serverPort0 = 12354;
    int flowIdx0 = 3;
    int flowIdx1 = 4;
    bool isTcp = false;

    // start listening
    socketMgr.Listen(piListen0, serverPort0, isTcp, flowIdx0, engine.GetAcceptDelegate(), engine.GetCloseDelegate());

    socketMgr.Listen(piListen1, serverPort0, isTcp, flowIdx0, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    
    // nothing should happen
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());
    
    // try to connect first connection
    socketMgr.AsyncConnect(piConnect0, serverPort0, isTcp, flowIdx0, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    // UDP "connections" are instant
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetConnects().size());

    // send a packet to force accept
    uint8_t packet[1] = {'X'};
    socketMgr.AsyncSend(engine.GetConnects()[0].fd, &packet[0], 1, engine.GetSendDelegate());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    // do select
    while (engine.GetSpawnAccepts().empty())
    {
        // do select
        ioReactor.handle_events();
    }

    // should have connected one
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    // recv the packet from above
    socketMgr.AsyncRecv(engine.GetSpawnAccepts()[0].fd, 1, engine.GetRecvDelegate());

    int clientSerial = engine.GetConnects()[0].fd;
    int serverSerial = engine.GetSpawnAccepts()[0].fd;
    CPPUNIT_ASSERT(clientSerial != serverSerial);

    // check that playlist is correct based on remote address
    CPPUNIT_ASSERT(piListen0 == engine.GetSpawnAccepts()[0].pi);

    // register new spawned playlist
    L4L7_ENGINE_NS::PlaylistInstance * piSpawned0 = pEngine.ClonePlaylist(piListen0, *mModifiers);
    socketMgr.AddChildPlaylist(piSpawned0, piListen0, engine.GetSpawnAccepts()[0].remoteAddr); 

    // new playlist starts listening
    socketMgr.Listen(piSpawned0, serverPort0, isTcp, flowIdx0, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    socketMgr.Listen(piSpawned0, serverPort0, isTcp, flowIdx1, engine.GetAcceptDelegate(), engine.GetCloseDelegate());

    // forward initial connection to new playlist
    CPPUNIT_ASSERT(socketMgr.Accept(piSpawned0, engine.GetSpawnAccepts()[0].fd));

    // check it was forwarded
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetAccepts().size());
    int spawnSerial = engine.GetAccepts()[0].fd;
    CPPUNIT_ASSERT_EQUAL(serverSerial, spawnSerial);


    // try to connect other playlist
    socketMgr.AsyncConnect(piConnect1, serverPort0, isTcp, flowIdx0, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    // UDP "connections" are instant
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetConnects().size());

    // send a packet to force accept
    socketMgr.AsyncSend(engine.GetConnects()[1].fd, &packet[0], 1, engine.GetSendDelegate());

    // should attempt to connect but be blocked
    // any misconfig would mean immediate error+close
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    // do select
    while (engine.GetSpawnAccepts().size() == 1)
    {
        // do select
        ioReactor.handle_events();
    }

    // should have connected another
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    // recv the packet from above
    socketMgr.AsyncRecv(engine.GetSpawnAccepts()[1].fd, 1, engine.GetRecvDelegate());

    clientSerial = engine.GetConnects()[1].fd;
    serverSerial = engine.GetSpawnAccepts()[1].fd;
    CPPUNIT_ASSERT(clientSerial != serverSerial);

    // check that playlist is correct based on remote address
    CPPUNIT_ASSERT(piListen1 == engine.GetSpawnAccepts()[1].pi);

    // register new spawned playlist
    L4L7_ENGINE_NS::PlaylistInstance * piSpawned1 = pEngine.ClonePlaylist(piListen1, *mModifiers);

    socketMgr.AddChildPlaylist(piSpawned1, piListen1, engine.GetSpawnAccepts()[1].remoteAddr); 

    // new playlist starts listening
    socketMgr.Listen(piSpawned1, serverPort0, isTcp, flowIdx0, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    socketMgr.Listen(piSpawned1, serverPort0, isTcp, flowIdx1, engine.GetAcceptDelegate(), engine.GetCloseDelegate());

    // forward initial connection to new playlist
    CPPUNIT_ASSERT(socketMgr.Accept(piSpawned1, engine.GetSpawnAccepts()[1].fd));

    // check it was forwarded
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetAccepts().size());
    spawnSerial = engine.GetAccepts()[1].fd;
    CPPUNIT_ASSERT_EQUAL(serverSerial, spawnSerial);
   
    // make other connections
    socketMgr.AsyncConnect(piConnect0, serverPort0, isTcp, flowIdx1, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    // UDP "connections" are instant
    CPPUNIT_ASSERT_EQUAL(size_t(3), engine.GetConnects().size());

    // send packets to force accept
    socketMgr.AsyncSend(engine.GetConnects()[2].fd, &packet[0], 1, engine.GetSendDelegate());

    // should attempt to connect but be blocked
    // any misconfig would mean immediate error+close
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    // do select
    while (engine.GetAccepts().size() == 2)
    {
        // do select
        ioReactor.handle_events();
    }

    // check it was forwarded and NOT spawned
    CPPUNIT_ASSERT_EQUAL(size_t(3), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetSpawnAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());
    }

    // destruction should close (It now is 6)
    CPPUNIT_ASSERT_EQUAL(size_t(6), engine.GetCloses().size());
}

///////////////////////////////////////////////////////////////////////////////

void TestSocketManager::testTcpSendRecvOutOfOrder()
{
    ACE_Reactor ioReactor;
    MockEngine engine;
    {
    SocketManager socketMgr(&ioReactor);
    MockPlaylistEngine pEngine(true);

    socketMgr.SetIpv4Tos(0xc0);
    socketMgr.SetTcpDelayedAck(true);

    L4L7_ENGINE_NS::PlaylistInstance * pi = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, true);
    
    std::string srcIfName = "lo";
    ACE_INET_Addr srcAddr(uint16_t(0), uint32_t(0x7f000001));
    ACE_INET_Addr dstAddr(uint16_t(0), uint32_t(0x7f000001));
    socketMgr.AddPlaylist(pi, srcIfName, srcAddr, dstAddr);

    uint16_t serverPort = 12355;
    int flowIdx = 3;
    bool isTcp = true;

    // start listening
    socketMgr.Listen(pi, serverPort, isTcp, flowIdx, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    
    // try to connect
    socketMgr.AsyncConnect(pi, serverPort, isTcp, flowIdx, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    while (engine.GetConnects().empty() || engine.GetAccepts().empty())
    {
        // do select
        ioReactor.handle_events();
    }

    // should have connected
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetSends().size());

    int clientSerial = engine.GetConnects()[0].fd;
    int serverSerial = engine.GetAccepts()[0].fd;

    socketMgr.AsyncRecv(serverSerial, 7, engine.GetRecvDelegate());
    socketMgr.AsyncSend(clientSerial, (const uint8_t*)"foobar\0whobar\0", 14, engine.GetSendDelegate());

    // do select
    while(engine.GetRecvs().empty())
    {
        usleep(1000);
        ioReactor.handle_events();
    }

    // now ask for the rest
    socketMgr.AsyncRecv(serverSerial, 7, engine.GetRecvDelegate());
    while(engine.GetRecvs().size() < 2)
    {
        usleep(1000);
        ioReactor.handle_events();
    }

    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetRecvs().size());

    CPPUNIT_ASSERT_EQUAL(size_t(7), engine.GetRecvs()[0].data.size());
    CPPUNIT_ASSERT_EQUAL(std::string("foobar\0", 7), std::string((char *)&engine.GetRecvs()[0].data[0], 7));

    CPPUNIT_ASSERT_EQUAL(size_t(7), engine.GetRecvs()[1].data.size());
    CPPUNIT_ASSERT_EQUAL(std::string("whobar\0", 7), std::string((char *)&engine.GetRecvs()[1].data[0], 7));
    }

    // destruction should close
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetCloses().size());
}

///////////////////////////////////////////////////////////////////////////////

void TestSocketManager::testTcpSendRecvReassemble()
{
    ACE_Reactor ioReactor;
    MockEngine engine;
    {
    SocketManager socketMgr(&ioReactor);
    MockPlaylistEngine pEngine(true);

    socketMgr.SetIpv4Tos(0xc0);
    socketMgr.SetTcpDelayedAck(true);

    L4L7_ENGINE_NS::PlaylistInstance * pi = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, true);
    
    std::string srcIfName = "lo";
    ACE_INET_Addr srcAddr(uint16_t(0), uint32_t(0x7f000001));
    ACE_INET_Addr dstAddr(uint16_t(0), uint32_t(0x7f000001));
    socketMgr.AddPlaylist(pi, srcIfName, srcAddr, dstAddr);

    uint16_t serverPort = 12356;
    int flowIdx = 3;
    bool isTcp = true;

    // start listening
    socketMgr.Listen(pi, serverPort, isTcp, flowIdx, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    
    // try to connect
    socketMgr.AsyncConnect(pi, serverPort, isTcp, flowIdx, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    while (engine.GetConnects().empty() || engine.GetAccepts().empty())
    {
        // do select
        ioReactor.handle_events();
    }

    // should have connected
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetSends().size());

    int clientSerial = engine.GetConnects()[0].fd;
    int serverSerial = engine.GetAccepts()[0].fd;

    socketMgr.AsyncSend(clientSerial, (const uint8_t*)"foobar\0", 7, engine.GetSendDelegate());
    socketMgr.AsyncSend(clientSerial, (const uint8_t*)"whobar\0", 7, engine.GetSendDelegate());
    socketMgr.AsyncSend(clientSerial, (const uint8_t*)"choobar\0", 8, engine.GetSendDelegate());
    socketMgr.AsyncRecv(serverSerial, 14, engine.GetRecvDelegate());
    socketMgr.AsyncRecv(serverSerial, 8, engine.GetRecvDelegate());

    // do select
    while(engine.GetRecvs().empty())
    {
        usleep(1000);
        ioReactor.handle_events();
    }

    CPPUNIT_ASSERT_EQUAL(size_t(3), engine.GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetRecvs().size());

    CPPUNIT_ASSERT_EQUAL(size_t(14), engine.GetRecvs()[0].data.size());
    CPPUNIT_ASSERT_EQUAL(std::string("foobar\0whobar\0", 14), std::string((char *)&engine.GetRecvs()[0].data[0], 14));

    CPPUNIT_ASSERT_EQUAL(size_t(8), engine.GetRecvs()[1].data.size());
    CPPUNIT_ASSERT_EQUAL(std::string("choobar"), std::string((char *)&engine.GetRecvs()[1].data[0]));
    }

    // destruction should close
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetCloses().size());
}

///////////////////////////////////////////////////////////////////////////////

void TestSocketManager::testTcpSendRecvAlot()
{
    ACE_Reactor ioReactor;
    MockEngine engine;
    {
    SocketManager socketMgr(&ioReactor);
    MockPlaylistEngine pEngine(true);

    socketMgr.SetIpv4Tos(0xc0);
    socketMgr.SetTcpDelayedAck(true);

    L4L7_ENGINE_NS::PlaylistInstance * pi = pEngine.MakePlaylist(uint32_t(1), *mModifiers, socketMgr, true);
    
    std::string srcIfName = "lo";
    ACE_INET_Addr srcAddr(uint16_t(0), uint32_t(0x7f000001));
    ACE_INET_Addr dstAddr(uint16_t(0), uint32_t(0x7f000001));
    socketMgr.AddPlaylist(pi, srcIfName, srcAddr, dstAddr);

    uint16_t serverPort = 12357;
    int flowIdx = 3;
    bool isTcp = true;

    // start listening
    socketMgr.Listen(pi, serverPort, isTcp, flowIdx, engine.GetAcceptDelegate(), engine.GetCloseDelegate());
    
    // try to connect
    socketMgr.AsyncConnect(pi, serverPort, isTcp, flowIdx, engine.GetConnectDelegate(), engine.GetCloseDelegate());

    while (engine.GetConnects().empty() || engine.GetAccepts().empty())
    {
        // do select
        ioReactor.handle_events();
    }

    // should have connected
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetSends().size());

    int clientSerial = engine.GetConnects()[0].fd;
    int serverSerial = engine.GetAccepts()[0].fd;

    std::string x_64k(65536, 'x');
    std::string y_1m(1048576, 'y');

    socketMgr.AsyncSend(clientSerial, (const uint8_t*)&x_64k[0], x_64k.size(), engine.GetSendDelegate());
    socketMgr.AsyncSend(clientSerial, (const uint8_t*)&y_1m[0], y_1m.size(), engine.GetSendDelegate());
    socketMgr.AsyncRecv(serverSerial, x_64k.size(), engine.GetRecvDelegate());
    socketMgr.AsyncRecv(serverSerial, y_1m.size(), engine.GetRecvDelegate());

    // do select
    while(engine.GetRecvs().size() < 2)
    {
        usleep(1000);
        ioReactor.handle_events();
    }

    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetRecvs().size());

    CPPUNIT_ASSERT_EQUAL(x_64k.size(), engine.GetRecvs()[0].data.size());
    CPPUNIT_ASSERT_EQUAL(x_64k, std::string((char *)&engine.GetRecvs()[0].data[0], x_64k.size()));

    CPPUNIT_ASSERT_EQUAL(y_1m.size(), engine.GetRecvs()[1].data.size());
    CPPUNIT_ASSERT_EQUAL(y_1m, std::string((char *)&engine.GetRecvs()[1].data[0], y_1m.size()));
    }

    // destruction should close
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetCloses().size());
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestSocketManager);
