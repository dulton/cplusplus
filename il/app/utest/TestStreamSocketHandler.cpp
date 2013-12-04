#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <unistd.h>
#include <iostream>

#include <ace/Reactor.h>
#include <ace/Time_Value.h>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "StreamSocketAcceptor.tcc"
#include "StreamSocketConnector.tcc"
#include "StreamSocketHandler.h"

USING_L4L7_APP_NS;

///////////////////////////////////////////////////////////////////////////////

class TestStreamSocketHandler : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestStreamSocketHandler);
    CPPUNIT_TEST(testSuccessfulConnect);
    CPPUNIT_TEST(testFailedConnect);
    CPPUNIT_TEST_SUITE_END();

public:
    TestStreamSocketHandler() { mServerPortNumber = SERVER_PORT_NUMBER_BASE + (getpid() % 1023); }

    void setUp(void) { }
    void tearDown(void) { ++mServerPortNumber; }
    
protected:
    void testSuccessfulConnect(void);
    void testFailedConnect(void);

private:
    class MockSocketHandler : public StreamSocketHandler
    {
      public:
        explicit MockSocketHandler(uint32_t serial = 0)
            : StreamSocketHandler(serial)
        {
        }
    };
    
    typedef StreamSocketAcceptor<MockSocketHandler> MockAcceptor;
    typedef StreamSocketConnector<MockSocketHandler> MockConnector;
    
    bool HandleAccept(MockSocketHandler& socket) 
    {
        mAcceptedSocket = &socket;
        return true;
    }
            
    bool HandleReject(MockSocketHandler& socket)
    {
        mRejectedSocket = &socket;
        return false;
    }
    
    bool HandleConnect(MockSocketHandler& socket)
    {
        mConnectedSocket = &socket;
        return true;
    }

    void HandleClose(StreamSocketHandler& socket)
    {
        if (&socket == mAcceptedSocket)
            mAcceptedSocket = 0;
        else if (&socket == mConnectedSocket)
            mConnectedSocket = 0;

        socket.remove_reference();
    }
    
    static const unsigned short SERVER_PORT_NUMBER_BASE = 10007;

    unsigned short mServerPortNumber;
    StreamSocketHandler *mAcceptedSocket;
    StreamSocketHandler *mRejectedSocket;
    StreamSocketHandler *mConnectedSocket;
};

///////////////////////////////////////////////////////////////////////////////

void TestStreamSocketHandler::testSuccessfulConnect(void)
{
    mAcceptedSocket = mRejectedSocket = mConnectedSocket = 0;
    ACE_Reactor reactor;
    
    // Open passive server socket
    MockAcceptor acceptor;
    acceptor.SetAcceptNotificationDelegate(fastdelegate::MakeDelegate(this, &TestStreamSocketHandler::HandleAccept));
    CPPUNIT_ASSERT(acceptor.open(MockAcceptor::addr_type(mServerPortNumber, INADDR_LOOPBACK), &reactor) == 0);

    // Start connection with active client socket
    MockConnector connector(&reactor);
    connector.SetConnectNotificationDelegate(fastdelegate::MakeDelegate(this, &TestStreamSocketHandler::HandleConnect));

    MockSocketHandler *socket = 0;
	/*
    CPPUNIT_ASSERT(connector.connect(socket, MockConnector::addr_type(mServerPortNumber, INADDR_LOOPBACK), ACE_Synch_Options(ACE_Synch_Options::USE_REACTOR)) == -1 && errno == EWOULDBLOCK);
     **/
   int err = connector.connect(socket, MockConnector::addr_type(mServerPortNumber, INADDR_LOOPBACK), ACE_Synch_Options(ACE_Synch_Options::USE_REACTOR));
   int last_errno = errno;
   
   CPPUNIT_ASSERT_EQUAL(-1, err);
   CPPUNIT_ASSERT_EQUAL(EWOULDBLOCK, last_errno);

    CPPUNIT_ASSERT(socket);
    socket->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &TestStreamSocketHandler::HandleClose));
    
    // Wait for accept and connect notifications to complete
    while (!mAcceptedSocket || !mConnectedSocket)
        reactor.handle_events();

    CPPUNIT_ASSERT(mConnectedSocket == socket);
    mAcceptedSocket->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &TestStreamSocketHandler::HandleClose));
    
    // Close connected socket
    StreamSocketHandler *connectedSocket = mConnectedSocket;
    connectedSocket->add_reference();
    connectedSocket->close();
    connectedSocket->remove_reference();
    
    // Wait for close notifications to complete
    while (mAcceptedSocket && mConnectedSocket)
        reactor.handle_events();

    // Shutdown reactor
    reactor.close();
}

///////////////////////////////////////////////////////////////////////////////

void TestStreamSocketHandler::testFailedConnect(void)
{
    mAcceptedSocket = mRejectedSocket = mConnectedSocket = 0;
    ACE_Reactor reactor;
    
    // Open passive server socket
    MockAcceptor acceptor;
    acceptor.SetAcceptNotificationDelegate(fastdelegate::MakeDelegate(this, &TestStreamSocketHandler::HandleReject));
    CPPUNIT_ASSERT(acceptor.open(MockAcceptor::addr_type(mServerPortNumber, INADDR_LOOPBACK), &reactor) == 0);

    // Start connection with active client socket
    MockConnector connector(&reactor);
    connector.SetConnectNotificationDelegate(fastdelegate::MakeDelegate(this, &TestStreamSocketHandler::HandleConnect));

    MockSocketHandler *socket = 0;
    int err = connector.connect(socket, MockConnector::addr_type(mServerPortNumber, INADDR_LOOPBACK), ACE_Synch_Options(ACE_Synch_Options::USE_REACTOR));
    int last_errno = errno;

    CPPUNIT_ASSERT_EQUAL(-1, err);
    CPPUNIT_ASSERT_EQUAL(EWOULDBLOCK, last_errno);

    CPPUNIT_ASSERT(socket);
    socket->SetCloseNotificationDelegate(fastdelegate::MakeDelegate(this, &TestStreamSocketHandler::HandleClose));
    
    // Wait for reject notification to complete
    while (!mRejectedSocket)
        reactor.handle_events();

    mRejectedSocket->remove_reference();
    
    // Shutdown reactor
    reactor.close();
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestStreamSocketHandler);
