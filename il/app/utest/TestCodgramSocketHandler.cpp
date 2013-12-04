#include <ace/Reactor.h>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "CodgramSocketConnector.tcc"
#include "CodgramSocketHandler.h"

USING_L4L7_APP_NS;

///////////////////////////////////////////////////////////////////////////////

class TestCodgramSocketHandler : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestCodgramSocketHandler);
    CPPUNIT_TEST(testSendRecv);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) 
    { 
        mServerPortNumber = SERVER_PORT_NUMBER_BASE + (getpid() % 1023); 
        mClientPortNumber = mServerPortNumber + 1;
    }
    void tearDown(void) { }
    
protected:
    void testSendRecv();

private:
    typedef CodgramSocketConnector<CodgramSocketHandler> Connector;

    bool HandleServerConnect(CodgramSocketHandler& socket)
    {
        mServerSocket = &socket;
        return true;
    }

    bool HandleClientConnect(CodgramSocketHandler& socket)
    {
        mClientSocket = &socket;
        return true;
    }

    void HandleClose(CodgramSocketHandler& socket)
    {
        if (&socket == mServerSocket)
            mServerSocket = 0;
        else if (&socket == mClientSocket)
            mClientSocket = 0;

        socket.remove_reference();
    }
    
    static const unsigned short SERVER_PORT_NUMBER_BASE = 10007;

    unsigned short mServerPortNumber;
    unsigned short mClientPortNumber;
    CodgramSocketHandler *mServerSocket;
    CodgramSocketHandler *mClientSocket;
};

///////////////////////////////////////////////////////////////////////////////

void TestCodgramSocketHandler::testSendRecv(void)
{
    ACE_Reactor reactor;
    
    // Open server socket
    Connector serverConnector(&reactor);
    serverConnector.SetConnectNotificationDelegate(fastdelegate::MakeDelegate(this, &TestCodgramSocketHandler::HandleServerConnect));
    CodgramSocketHandler *socket = 0;
    CPPUNIT_ASSERT(serverConnector.connect(socket, Connector::addr_type(mServerPortNumber, INADDR_LOOPBACK), Connector::addr_type(mClientPortNumber, INADDR_LOOPBACK), ACE_Synch_Options(ACE_Synch_Options::USE_REACTOR)) == 0);
    CPPUNIT_ASSERT(socket);
    socket->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &TestCodgramSocketHandler::HandleClose));

    // Open client socket
    Connector clientConnector(&reactor);
    clientConnector.SetConnectNotificationDelegate(fastdelegate::MakeDelegate(this, &TestCodgramSocketHandler::HandleClientConnect));
    socket = 0;
    CPPUNIT_ASSERT(clientConnector.connect(socket, Connector::addr_type(mClientPortNumber, INADDR_LOOPBACK), Connector::addr_type(mServerPortNumber, INADDR_LOOPBACK), ACE_Synch_Options(ACE_Synch_Options::USE_REACTOR)) == 0);
    CPPUNIT_ASSERT(socket);
    socket->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &TestCodgramSocketHandler::HandleClose));
    
    // Wait for connect notifications to complete
    while (!mServerSocket || !mClientSocket)
        reactor.handle_events();

    // send from client to server
    std::string packet0 = "abcdefghijklmnopqrstuvwyz";
    CPPUNIT_ASSERT(mClientSocket->Send(packet0));

    // Wait for packet to arrive
    while (!mServerSocket->HasInputData())
        reactor.handle_events();

    CPPUNIT_ASSERT_EQUAL(size_t(1), mServerSocket->mInputQueue.size());

    // assume block is not split
    CPPUNIT_ASSERT_EQUAL(packet0.size(), mServerSocket->mInputQueue[0]->length());
    CPPUNIT_ASSERT_EQUAL(int(0), memcmp(packet0.c_str(), mServerSocket->mInputQueue[0]->rd_ptr(), packet0.size()));

    // clear queue
    mServerSocket->mInputQueue.clear();

    // send more packets
    std::string packet1 = "LEEEEEEEEEEEEEEEEEEROOOOOOOOOOOOOOOOOOY";
    std::string packet2 = "JEENNNNNNNNNNNNNNNNNNNNNNNNNNNNNKINS";

    CPPUNIT_ASSERT(mServerSocket->Send(packet1));
    CPPUNIT_ASSERT(mClientSocket->Send(packet2));

    // Wait for packets to arrive
    while (!mClientSocket->HasInputData() || !mServerSocket->HasInputData())
        reactor.handle_events();
    
    CPPUNIT_ASSERT_EQUAL(size_t(1), mClientSocket->mInputQueue.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mServerSocket->mInputQueue.size());

    // assume blocks are not split
    CPPUNIT_ASSERT_EQUAL(packet1.size(), mClientSocket->mInputQueue[0]->length());
    CPPUNIT_ASSERT_EQUAL(int(0), memcmp(packet1.c_str(), mClientSocket->mInputQueue[0]->rd_ptr(), packet1.size()));

    CPPUNIT_ASSERT_EQUAL(packet2.size(), mServerSocket->mInputQueue[0]->length());
    CPPUNIT_ASSERT_EQUAL(int(0), memcmp(packet2.c_str(), mServerSocket->mInputQueue[0]->rd_ptr(), packet2.size()));

    // Close sockets
    socket = mServerSocket;
    socket->add_reference();
    socket->close();
    socket->remove_reference();

    socket = mClientSocket;
    socket->add_reference();
    socket->close();
    socket->remove_reference();

    // Wait for close notifications to complete
    while (mServerSocket && mClientSocket)
        reactor.handle_events();

    // Shutdown reactor
    reactor.close();
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestCodgramSocketHandler);
