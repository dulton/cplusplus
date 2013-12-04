#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <boost/foreach.hpp>

#include "DpgDgramAcceptor.tcc"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class MockPseudoHandler;

class TestDpgDgramAcceptor : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestDpgDgramAcceptor);
    CPPUNIT_TEST(testAccept);
    CPPUNIT_TEST_SUITE_END();

public:
    TestDpgDgramAcceptor()
    {
    }

    ~TestDpgDgramAcceptor()
    {
    }

    void setUp() 
    { 
        mServerPortNumber = SERVER_PORT_NUMBER_BASE + (getpid() % 1023); 
        mClientPortNumber = mServerPortNumber + 1;
    }

    void tearDown();
    
protected:
    void testAccept();

private:
    bool HandleAccept(MockPseudoHandler& handler)
    {
        mHandlers.insert(&handler);
        return true;
    }

    static const unsigned short SERVER_PORT_NUMBER_BASE = 10009;

    unsigned short mServerPortNumber;
    unsigned short mClientPortNumber;
    std::set<MockPseudoHandler*> mHandlers;
};

///////////////////////////////////////////////////////////////////////////////

class MockPseudoHandler
{
  public:
    MockPseudoHandler(DpgDgramAcceptor<MockPseudoHandler> * socket)
        : mSocket(*socket)
    {
    }

    void SetRemoteAddr(const ACE_INET_Addr& addr)
    {
        mAddr = addr;
    }

    bool GetRemoteAddr(ACE_INET_Addr& addr)
    {
        addr = mAddr;
        return true;
    }

  private:
    DpgDgramAcceptor<MockPseudoHandler>& mSocket;
    ACE_INET_Addr mAddr;
};

///////////////////////////////////////////////////////////////////////////////

void TestDpgDgramAcceptor::tearDown()
{ 
    BOOST_FOREACH(MockPseudoHandler * handler, mHandlers)
    {
        delete handler;
    }
    mHandlers.clear();
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgDgramAcceptor::testAccept()
{
    ACE_Reactor reactor;

    {
    // Open passive server socket
    DpgDgramAcceptor<MockPseudoHandler> acceptor(1024);

    acceptor.SetAcceptNotificationDelegate(fastdelegate::MakeDelegate(this, &TestDpgDgramAcceptor::HandleAccept));

    ACE_INET_Addr serverAddr(mServerPortNumber, INADDR_LOOPBACK);
    CPPUNIT_ASSERT(acceptor.open(serverAddr, &reactor) == 0);

    ACE_SOCK_Dgram client;
    ACE_INET_Addr clientAddr(mClientPortNumber, INADDR_LOOPBACK);
    CPPUNIT_ASSERT(client.open(clientAddr) == 0);

    CPPUNIT_ASSERT(mHandlers.empty());

    // send directly 
    const char * buf = "abcdefghijklmnopqrstuvwyz";
    CPPUNIT_ASSERT(client.send(buf, strlen(buf), serverAddr) == (int)strlen(buf)); 

    // Wait for packet to arrive
    while (mHandlers.empty())
        reactor.handle_events();

    CPPUNIT_ASSERT_EQUAL(size_t(1), mHandlers.size());

    MockPseudoHandler * handler = *mHandlers.begin();
    ACE_INET_Addr handlerAddr;

    CPPUNIT_ASSERT(handler->GetRemoteAddr(handlerAddr));
    CPPUNIT_ASSERT(handlerAddr == clientAddr);
    
    client.close();
    }

    // Shutdown reactor
    reactor.close();
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestDpgDgramAcceptor);
