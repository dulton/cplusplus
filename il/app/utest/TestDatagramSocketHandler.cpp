#include <deque>
#include <string>

#include <ace/Reactor.h>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "DatagramSocketHandler.h"

USING_L4L7_APP_NS;

///////////////////////////////////////////////////////////////////////////////

class TestDatagramSocketHandler : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestDatagramSocketHandler);
    CPPUNIT_TEST(testRecv);
    CPPUNIT_TEST(testSendRecv);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) 
    { 
        mServerPortNumber = SERVER_PORT_NUMBER_BASE + (getpid() % 1023); 
        mClientPortNumber = mServerPortNumber + 1;
        mServerSocket = 0;
        mTxCountList.clear();
        mTxAddrList.clear();
    }
    void tearDown(void) { }
    
protected:
    void testRecv();
    void testSendRecv();

private:
    class MockSocketHandler : public DatagramSocketHandler
    {
      public:
        explicit MockSocketHandler(size_t mtu)
            : DatagramSocketHandler(mtu), outputCount(0)
        {
        }

        int HandleInputHook(messagePtr_t& msg, const ACE_INET_Addr& addr)
        {
            msgList.push_back(msg);
            addrList.push_back(addr);
            return 0;
        }

        int HandleOutputHook()
        {
            ++outputCount;
            return 0;
        }

        size_t outputCount;
        std::vector<messagePtr_t> msgList;
        std::vector<ACE_INET_Addr> addrList;
    };

    void HandleClose(DatagramSocketHandler& socket)
    {
        if ((MockSocketHandler*)&socket == mServerSocket)
            mServerSocket = 0;

        socket.remove_reference();
    }

    void UpdateTxCount(uint64_t sent, const ACE_INET_Addr& remote_addr)
    {
        mTxCountList.push_back(sent);
        mTxAddrList.push_back(remote_addr);
    }
    
    static const unsigned short SERVER_PORT_NUMBER_BASE = 10007;

    unsigned short mServerPortNumber;
    unsigned short mClientPortNumber;
    MockSocketHandler *mServerSocket;

    std::vector<uint64_t> mTxCountList;
    std::vector<ACE_INET_Addr> mTxAddrList;
};

///////////////////////////////////////////////////////////////////////////////

void TestDatagramSocketHandler::testRecv()
{
    ACE_Reactor reactor;
    
    // Open passive server socket
    mServerSocket = new MockSocketHandler(1024);
    mServerSocket->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &TestDatagramSocketHandler::HandleClose));

    ACE_INET_Addr serverAddr(mServerPortNumber, INADDR_LOOPBACK);
    CPPUNIT_ASSERT(mServerSocket->open(serverAddr, &reactor) == 0);

    ACE_SOCK_Dgram client;
    ACE_INET_Addr clientAddr(mClientPortNumber, INADDR_LOOPBACK);
    CPPUNIT_ASSERT(client.open(clientAddr) == 0);

    // send directly 
    const char * buf = "abcdefghijklmnopqrstuvwyz";
    CPPUNIT_ASSERT(client.send(buf, strlen(buf), serverAddr) == (int)strlen(buf)); 

    // Wait for packet to arrive
    while (mServerSocket->msgList.empty())
        reactor.handle_events();

    CPPUNIT_ASSERT_EQUAL(size_t(1), mServerSocket->msgList.size());
    // assume block is not split
    CPPUNIT_ASSERT_EQUAL(strlen(buf), mServerSocket->msgList[0]->length());
    CPPUNIT_ASSERT_EQUAL(int(0), memcmp(buf, mServerSocket->msgList[0]->rd_ptr(), mServerSocket->msgList[0]->length()));

    CPPUNIT_ASSERT_EQUAL(size_t(1), mServerSocket->addrList.size());
    CPPUNIT_ASSERT(clientAddr == mServerSocket->addrList[0]);
    
    client.close();

    // Close socket
    DatagramSocketHandler * serverSocket = mServerSocket;
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

void TestDatagramSocketHandler::testSendRecv()
{
    ACE_Reactor reactor;
    
    // Open server socket
    mServerSocket = new MockSocketHandler(1024);
    mServerSocket->RegisterUpdateTxCountDelegate(fastdelegate::MakeDelegate(this, &TestDatagramSocketHandler::UpdateTxCount));
    mServerSocket->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &TestDatagramSocketHandler::HandleClose));

    ACE_INET_Addr serverAddr(mServerPortNumber, INADDR_LOOPBACK);
    CPPUNIT_ASSERT(mServerSocket->open(serverAddr, &reactor) == 0);

    ACE_SOCK_Dgram client0;
    ACE_INET_Addr clientAddr0(mClientPortNumber, INADDR_LOOPBACK);
    CPPUNIT_ASSERT(client0.open(clientAddr0) == 0);

    ACE_SOCK_Dgram client1;
    ACE_INET_Addr clientAddr1(mClientPortNumber+1, INADDR_LOOPBACK);
    CPPUNIT_ASSERT(client1.open(clientAddr1) == 0);

    // send directly 
    const char * buf = "abcdefghijklmnopqrstuvwyz";
    CPPUNIT_ASSERT(client0.send(buf, strlen(buf), serverAddr) == (int)strlen(buf)); 
    CPPUNIT_ASSERT(client1.send(buf, strlen(buf), serverAddr) == (int)strlen(buf)); 

    // Wait for packets to arrive
    while (mServerSocket->msgList.size() < 2)
        reactor.handle_events();

    CPPUNIT_ASSERT_EQUAL(size_t(2), mServerSocket->msgList.size());
    // assume block is not split
    CPPUNIT_ASSERT_EQUAL(strlen(buf), mServerSocket->msgList[0]->length());
    CPPUNIT_ASSERT_EQUAL(strlen(buf), mServerSocket->msgList[1]->length());
    CPPUNIT_ASSERT_EQUAL(int(0), memcmp(buf, mServerSocket->msgList[0]->rd_ptr(), mServerSocket->msgList[0]->length()));
    CPPUNIT_ASSERT_EQUAL(int(0), memcmp(buf, mServerSocket->msgList[1]->rd_ptr(), mServerSocket->msgList[1]->length()));

    CPPUNIT_ASSERT_EQUAL(size_t(2), mServerSocket->addrList.size());
    CPPUNIT_ASSERT(clientAddr0 == mServerSocket->addrList[0]);
    CPPUNIT_ASSERT(clientAddr1 == mServerSocket->addrList[1]);

    // send from server to clients
    std::string serverString0 = "FOOOOOOOOOOOOOOOOOOOOOOOOOOO!";
    std::string serverString1 = "SNOOOOOOOOOOOOOOOOOOOOOOOOOOO!";
    mServerSocket->Send(serverString0, clientAddr0);
    mServerSocket->Send(serverString1, clientAddr1);

    // Wait for packets to go out
    while (mServerSocket->HasOutputData())
        reactor.handle_events();
    
    // TxUpdate was called twice
    CPPUNIT_ASSERT_EQUAL(size_t(2), mTxCountList.size());
    CPPUNIT_ASSERT_EQUAL(serverString0.size(), size_t(mTxCountList[0]));
    CPPUNIT_ASSERT_EQUAL(serverString1.size(), size_t(mTxCountList[1]));
    CPPUNIT_ASSERT_EQUAL(size_t(2), mTxAddrList.size());
    CPPUNIT_ASSERT(clientAddr0 == mTxAddrList[0]);
    CPPUNIT_ASSERT(clientAddr1 == mTxAddrList[1]);
    
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
    DatagramSocketHandler * serverSocket = mServerSocket;
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

CPPUNIT_TEST_SUITE_REGISTRATION(TestDatagramSocketHandler);
