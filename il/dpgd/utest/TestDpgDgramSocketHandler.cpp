#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <boost/bind.hpp>

#include <utils/MessageUtils.h>

#include "MockEngine.h"
#include "DpgDgramSocketHandler.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestDpgDgramSocketHandler : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestDpgDgramSocketHandler);
    CPPUNIT_TEST(testRecv);
    CPPUNIT_TEST(testSendRecv);
    CPPUNIT_TEST(testDefaultRecv);
    CPPUNIT_TEST_SUITE_END();

public:
    TestDpgDgramSocketHandler()
    {
    }

    ~TestDpgDgramSocketHandler()
    {
    }

    void setUp() 
    { 
        mServerPortNumber = SERVER_PORT_NUMBER_BASE + (getpid() % 1023); 
        mClientPortNumber = mServerPortNumber + 1;
        mServerSocket = 0;
        mHandleRecvCalled = 0;
    }

    void tearDown() 
    { 
    }
    
protected:
    void testRecv();
    void testSendRecv();
    void testDefaultRecv();

private:
    void HandleClose(L4L7_APP_NS::DatagramSocketHandler& socket)
    {
        if (static_cast<DpgDgramSocketHandler*>(&socket) == mServerSocket)
            mServerSocket = 0;

        socket.remove_reference();
    }

    void HandleRecv(const ACE_INET_Addr& addr, MockEngine& engine)
    {
        // on a default receive, register for the receive
        mServerSocket->AsyncRecv(addr, engine.GetRecvDelegate()); 
        ++mHandleRecvCalled;
    }

    static const unsigned short SERVER_PORT_NUMBER_BASE = 10009;

    unsigned short mServerPortNumber;
    unsigned short mClientPortNumber;
    unsigned int mHandleRecvCalled;
    DpgDgramSocketHandler *mServerSocket;
};

///////////////////////////////////////////////////////////////////////////////

void TestDpgDgramSocketHandler::testRecv()
{
    ACE_Reactor reactor;
    MockEngine engine;

    // Open passive server socket
    mServerSocket = new DpgDgramSocketHandler(1024);
    mServerSocket->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &TestDpgDgramSocketHandler::HandleClose));

    ACE_INET_Addr serverAddr(mServerPortNumber, INADDR_LOOPBACK);
    CPPUNIT_ASSERT(mServerSocket->open(serverAddr, &reactor) == 0);

    ACE_SOCK_Dgram client;
    ACE_INET_Addr clientAddr(mClientPortNumber, INADDR_LOOPBACK);
    CPPUNIT_ASSERT(client.open(clientAddr) == 0);

    // request receive from the client
    mServerSocket->AsyncRecv(clientAddr, engine.GetRecvDelegate());

    CPPUNIT_ASSERT(mServerSocket->mRecvDelegateQueueMap.find(clientAddr) != 
                   mServerSocket->mRecvDelegateQueueMap.end());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mServerSocket->mRecvDelegateQueueMap[clientAddr].size());


    // send directly 
    const char * buf = "abcdefghijklmnopqrstuvwyz";
    CPPUNIT_ASSERT(client.send(buf, strlen(buf), serverAddr) == (int)strlen(buf)); 

    // Wait for packet to arrive
    while (engine.GetRecvs().empty())
        reactor.handle_events();

    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(strlen(buf), engine.GetRecvs()[0].data.size());
    CPPUNIT_ASSERT_EQUAL(int(0), memcmp(buf, &engine.GetRecvs()[0].data[0], strlen(buf)));

    CPPUNIT_ASSERT_EQUAL(size_t(0), mServerSocket->mInputList.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), mServerSocket->mInputAddrList.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), mServerSocket->mRecvDelegateQueueMap[clientAddr].size());
    
    client.close();

    // Close socket
    DpgDgramSocketHandler * serverSocket = mServerSocket;
    serverSocket->add_reference();
    serverSocket->close();
    serverSocket->remove_reference();
    
    // Wait for close notifications to complete
    while (mServerSocket)
        reactor.handle_events();

    // Shutdown reactor
    reactor.close();
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgDgramSocketHandler::testSendRecv()
{
    ACE_Reactor reactor;
    MockEngine engine;
    
    // Open server socket
    mServerSocket = new DpgDgramSocketHandler(1024);
    mServerSocket->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &TestDpgDgramSocketHandler::HandleClose));

    ACE_INET_Addr serverAddr(mServerPortNumber, INADDR_LOOPBACK);
    CPPUNIT_ASSERT(mServerSocket->open(serverAddr, &reactor) == 0);

    ACE_SOCK_Dgram client0;
    ACE_INET_Addr clientAddr0(mClientPortNumber, INADDR_LOOPBACK);
    CPPUNIT_ASSERT(client0.open(clientAddr0) == 0);

    ACE_SOCK_Dgram client1;
    ACE_INET_Addr clientAddr1(mClientPortNumber+1, INADDR_LOOPBACK);
    CPPUNIT_ASSERT(client1.open(clientAddr1) == 0);

    // request receive
    mServerSocket->AsyncRecv(clientAddr0, engine.GetRecvDelegate());
    mServerSocket->AsyncRecv(clientAddr1, engine.GetRecvDelegate());

    CPPUNIT_ASSERT(mServerSocket->mRecvDelegateQueueMap.find(clientAddr0) != 
                   mServerSocket->mRecvDelegateQueueMap.end());
    CPPUNIT_ASSERT(mServerSocket->mRecvDelegateQueueMap.find(clientAddr1) != 
                   mServerSocket->mRecvDelegateQueueMap.end());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mServerSocket->mRecvDelegateQueueMap[clientAddr0].size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mServerSocket->mRecvDelegateQueueMap[clientAddr1].size());


    // send directly 
    const char * buf = "abcdefghijklmnopqrstuvwyz";
    CPPUNIT_ASSERT(client0.send(buf, strlen(buf), serverAddr) == (int)strlen(buf)); 
    CPPUNIT_ASSERT(client1.send(buf, strlen(buf), serverAddr) == (int)strlen(buf)); 

    // Wait for packets to arrive
    while (engine.GetRecvs().size() < 2)
        reactor.handle_events();

    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(strlen(buf), engine.GetRecvs()[0].data.size());
    CPPUNIT_ASSERT_EQUAL(strlen(buf), engine.GetRecvs()[1].data.size());
    CPPUNIT_ASSERT_EQUAL(int(0), memcmp(buf, &engine.GetRecvs()[0].data[0], strlen(buf)));
    CPPUNIT_ASSERT_EQUAL(int(0), memcmp(buf, &engine.GetRecvs()[1].data[0], strlen(buf)));

    CPPUNIT_ASSERT_EQUAL(size_t(0), mServerSocket->mInputList.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), mServerSocket->mInputAddrList.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), mServerSocket->mRecvDelegateQueueMap[clientAddr0].size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), mServerSocket->mRecvDelegateQueueMap[clientAddr1].size());

    // send from server to clients
    std::string serverString0 = "FOOOOOOOOOOOOOOOOOOOOOOOOOOO!";
    std::string serverString1 = "SNOOOOOOOOOOOOOOOOOOOOOOOOOOO!";

    DpgDgramSocketHandler::messagePtr_t mb0 = L4L7_UTILS_NS::MessageAlloc(new ACE_Message_Block(serverString0.size())); 
    memcpy(mb0->base(), serverString0.c_str(), serverString0.size());
    mb0->wr_ptr(serverString0.size());

    DpgDgramSocketHandler::messagePtr_t mb1 = L4L7_UTILS_NS::MessageAlloc(new ACE_Message_Block(serverString1.size())); 
    memcpy(mb1->base(), serverString1.c_str(), serverString1.size());
    mb1->wr_ptr(serverString1.size());

    mServerSocket->AsyncSend(mb0, clientAddr0, engine.GetSendDelegate());
    mServerSocket->AsyncSend(mb1, clientAddr1, engine.GetSendDelegate());

    // Wait for packets to go out
    while (engine.GetSends().size() < 2)
        reactor.handle_events();

    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetSends().size());
    
    ACE_INET_Addr recvAddr;
    char recvBuf[128];
    CPPUNIT_ASSERT_EQUAL(ssize_t(serverString0.size()), client0.recv(recvBuf, sizeof(recvBuf), recvAddr));
    CPPUNIT_ASSERT_EQUAL(0, memcmp(recvBuf, serverString0.c_str(), serverString0.size()));
    CPPUNIT_ASSERT(recvAddr == serverAddr);

    CPPUNIT_ASSERT_EQUAL(ssize_t(serverString1.size()), client1.recv(recvBuf, sizeof(recvBuf), recvAddr));
    CPPUNIT_ASSERT_EQUAL(0, memcmp(recvBuf, serverString1.c_str(), serverString1.size()));
    CPPUNIT_ASSERT(recvAddr == serverAddr);

    client0.close();
    client1.close();

    // Close socket
    DpgDgramSocketHandler * serverSocket = mServerSocket;
    serverSocket->add_reference();
    serverSocket->close();
    serverSocket->remove_reference();
    
    // Wait for close notifications to complete
    while (mServerSocket)
        reactor.handle_events();

    // Shutdown reactor
    reactor.close();
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgDgramSocketHandler::testDefaultRecv()
{
    ACE_Reactor reactor;
    MockEngine engine;

    // Open passive server socket
    mServerSocket = new DpgDgramSocketHandler(1024);
    mServerSocket->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &TestDpgDgramSocketHandler::HandleClose));

    ACE_INET_Addr serverAddr(mServerPortNumber, INADDR_LOOPBACK);
    CPPUNIT_ASSERT(mServerSocket->open(serverAddr, &reactor) == 0);

    ACE_SOCK_Dgram client;
    ACE_INET_Addr clientAddr(mClientPortNumber, INADDR_LOOPBACK);
    CPPUNIT_ASSERT(client.open(clientAddr) == 0);

    // request receive from the client
    mServerSocket->SetDefaultRecvDelegate(boost::bind(&TestDpgDgramSocketHandler::HandleRecv, this, _1, boost::ref(engine)));

    CPPUNIT_ASSERT_EQUAL(size_t(0), mHandleRecvCalled);
    CPPUNIT_ASSERT(mServerSocket->mRecvDelegateQueueMap.find(clientAddr) == 
                   mServerSocket->mRecvDelegateQueueMap.end());


    // send directly 
    const char * buf = "abcdefghijklmnopqrstuvwyz";
    CPPUNIT_ASSERT(client.send(buf, strlen(buf), serverAddr) == (int)strlen(buf)); 

    // Wait for packet to arrive
    while (engine.GetRecvs().empty())
        reactor.handle_events();

    // on default recv, the recv should be registered and passed through
    CPPUNIT_ASSERT_EQUAL(size_t(1), mHandleRecvCalled);
    CPPUNIT_ASSERT(mServerSocket->mRecvDelegateQueueMap.find(clientAddr) != 
                   mServerSocket->mRecvDelegateQueueMap.end());
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(strlen(buf), engine.GetRecvs()[0].data.size());
    CPPUNIT_ASSERT_EQUAL(int(0), memcmp(buf, &engine.GetRecvs()[0].data[0], strlen(buf)));

    CPPUNIT_ASSERT_EQUAL(size_t(0), mServerSocket->mInputList.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), mServerSocket->mInputAddrList.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), mServerSocket->mRecvDelegateQueueMap[clientAddr].size());
    
    client.close();

    // Close socket
    DpgDgramSocketHandler * serverSocket = mServerSocket;
    serverSocket->add_reference();
    serverSocket->close();
    serverSocket->remove_reference();
    
    // Wait for close notifications to complete
    while (mServerSocket)
        reactor.handle_events();

    // Shutdown reactor
    reactor.close();
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestDpgDgramSocketHandler);
