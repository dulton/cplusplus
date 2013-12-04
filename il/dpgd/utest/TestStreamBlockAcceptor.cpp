#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "StreamBlockAcceptor.h"

#include <app/StreamSocketAcceptor.tcc>
#include "DpgStreamConnectionHandler.h"

#include "MockEngine.h"
#include "MockReactor.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestStreamBlockAcceptor : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestStreamBlockAcceptor);
    CPPUNIT_TEST(testAcceptorMap);
    CPPUNIT_TEST(testDestBasedAccept);
    CPPUNIT_TEST(testNoDestAccept);
    CPPUNIT_TEST_SUITE_END();

public:
    TestStreamBlockAcceptor()
    {
    }

    ~TestStreamBlockAcceptor()
    {
    }

    void setUp() 
    { 
    }

    void tearDown() 
    { 
    }
    
protected:
    void testAcceptorMap();
    void testDestBasedAccept();
    void testNoDestAccept();

private:
};

///////////////////////////////////////////////////////////////////////////////

// Mock Connection Handler to wrap a given remote address
class MockConnectionHandler : public DpgStreamConnectionHandler
{
  public:
    MockConnectionHandler(const ACE_INET_Addr& addr)
        : mAddr(addr)
    {
    }

    bool GetRemoteAddr(ACE_INET_Addr& addr)
    {
        addr = mAddr;
        return true;
    }

  private:
    ACE_INET_Addr mAddr;
};

// Mock Acceptor that allows the test to similate an incoming connection from a
// given address.
class MockAcceptor : public L4L7_APP_NS::StreamSocketAcceptor<DpgStreamConnectionHandler> 
{
public:
    void MockAccept(const ACE_INET_Addr& remoteAddr)
    {
         MockConnectionHandler handler(remoteAddr);
         mAcceptNotificationDelegate && mAcceptNotificationDelegate(handler); 
    }
};


// Testable StreamBlockAcceptor that builds a Mock Acceptor instead of a real one
class TestableStreamBlockAcceptor : public StreamBlockAcceptor
{
public:
    TestableStreamBlockAcceptor(ACE_Reactor * ioReactor)
        : StreamBlockAcceptor(ioReactor)
    {
        SetAcceptNotificationDelegate(fastdelegate::MakeDelegate(this, &TestableStreamBlockAcceptor::ParentHandleAcceptNotification));
    }
    
    virtual ~TestableStreamBlockAcceptor() {}

    bool MockAccept(const std::string& localIfName, const ACE_INET_Addr& localAddr, const ACE_INET_Addr& remoteAddr)
    {
        // pass to the appropriate local acceptor (normally done by the OS/ACE)
        AddrInfo localAddrInfo;
        localAddrInfo.locIfName = localIfName;
        localAddrInfo.locAddr = localAddr;
        localAddrInfo.hasRem = false;
        acceptorMap_t::const_iterator accept_iter = FindAcceptor(localAddrInfo); 
        if (accept_iter == EndAcceptor())
            return false; 

        ((MockAcceptor *)accept_iter->second.first.get())->MockAccept(remoteAddr);
        return true;
    }

protected:
    acceptor_t * MakeAcceptor()
    {
        return (acceptor_t *)new MockAcceptor();
    }

    bool ParentHandleAcceptNotification(DpgStreamConnectionHandler& rawConnHandler)
    {
        // In a non-test this would be registered to the SocketManager
        return rawConnHandler.HandleConnect();
    }
};

///////////////////////////////////////////////////////////////////////////////

void TestStreamBlockAcceptor::testAcceptorMap()
{
    StreamBlockAcceptor::acceptorMap_t map;
    AddrInfo addr0;
    addr0.locIfName = "foo";
    addr0.locAddr.set(uint16_t(1234), uint32_t(0x7f000001));
    addr0.hasRem = false;

    StreamBlockAcceptor::connectDelegate_t nullConDel;
    StreamBlockAcceptor::closeDelegate_t nullCloDel;
    StreamBlockAcceptor::acceptorDelegatePair_t acc(StreamBlockAcceptor::acceptorSharedPtr_t(), StreamBlockAcceptor::portDelegateMap_t(ACE_INET_Addr(), nullConDel, nullCloDel));

    map.insert(std::make_pair(addr0, acc));
    CPPUNIT_ASSERT_EQUAL(size_t(1), map.size());

    // modify dst, should hash to same value
    addr0.hasRem = true;
    addr0.remAddr.set(uint16_t(4321), uint32_t(0x01020304));

    map.insert(std::make_pair(addr0, acc));
    CPPUNIT_ASSERT_EQUAL(size_t(1), map.size());

    // modify srcIfName, should hash to new value
    addr0.locIfName = "bar";
    map.insert(std::make_pair(addr0, acc));
    CPPUNIT_ASSERT_EQUAL(size_t(2), map.size());
    map.insert(std::make_pair(addr0, acc));
    CPPUNIT_ASSERT_EQUAL(size_t(2), map.size());

    // modify srcAddr port, should hash to new value
    addr0.locAddr.set_port_number(80); 
    map.insert(std::make_pair(addr0, acc));
    CPPUNIT_ASSERT_EQUAL(size_t(3), map.size());
    map.insert(std::make_pair(addr0, acc));
    CPPUNIT_ASSERT_EQUAL(size_t(3), map.size());

    // modify srcAddr addr should hash to new value
    addr0.locAddr.set(uint16_t(80), uint32_t(0x7f000002));
    map.insert(std::make_pair(addr0, acc));
    CPPUNIT_ASSERT_EQUAL(size_t(4), map.size());
    map.insert(std::make_pair(addr0, acc));
    CPPUNIT_ASSERT_EQUAL(size_t(4), map.size());
}


///////////////////////////////////////////////////////////////////////////////

void TestStreamBlockAcceptor::testDestBasedAccept()
{
    // When a destination is specified, only connects from that particular
    // destination are accepted (there should always be an unspecified 
    // destination handler as well to take the unexpected accepts)
    // Also, after a single accept from that destination, we accept no more
    // until we get another registration
   
    // FIXME -- also need to take source port magic into account in future
    MockReactor ioReactor;
    MockEngine  noDestEngine;
    MockEngine  destEngine0;
    TestableStreamBlockAcceptor bac(&ioReactor);

    AddrInfo localAddr;
    localAddr.locIfName = "lo"; 
    localAddr.locAddr = ACE_INET_Addr(uint16_t(8000), uint32_t(0x7f000001));
    localAddr.hasRem = false;
    StreamBlockAcceptor::addrSet_t remAddrSet;

    bool result = bac.Listen(localAddr, localAddr.locAddr.get_port_number(), remAddrSet, noDestEngine.GetAcceptDelegate(), noDestEngine.GetCloseDelegate());

    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT_EQUAL(size_t(0), noDestEngine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), noDestEngine.GetCloses().size());

    AddrInfo bothAddr0;
    bothAddr0.locIfName = localAddr.locIfName;
    bothAddr0.locAddr = localAddr.locAddr;
    bothAddr0.hasRem = true;
    bothAddr0.remAddr = ACE_INET_Addr(uint16_t(123), uint32_t(0x0a010203));

    // wait for a connection specifically from 10.1.2.3
    result = bac.Listen(bothAddr0, bothAddr0.locAddr.get_port_number(), remAddrSet, destEngine0.GetAcceptDelegate(), destEngine0.GetCloseDelegate());

    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT_EQUAL(size_t(0), destEngine0.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), destEngine0.GetCloses().size());

    // simulate a connection from 10.1.2.4/124 - no specific listener 
    ACE_INET_Addr remoteAddrX = ACE_INET_Addr(uint16_t(124), uint32_t(0x0a010204));
    result = bac.MockAccept(localAddr.locIfName, localAddr.locAddr, remoteAddrX);
    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT_EQUAL(size_t(1), noDestEngine.GetAccepts().size());

    // simulate a connection from 10.1.2.3/123 - specific listener
    ACE_INET_Addr remoteAddrY = ACE_INET_Addr(uint16_t(123), uint32_t(0x0a010203));

    result = bac.MockAccept(localAddr.locIfName, localAddr.locAddr, remoteAddrY);
    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT_EQUAL(size_t(1), destEngine0.GetAccepts().size());
}

///////////////////////////////////////////////////////////////////////////////

void TestStreamBlockAcceptor::testNoDestAccept()
{
    MockReactor ioReactor;
    MockEngine  engine;
    TestableStreamBlockAcceptor bac(&ioReactor);

    AddrInfo localAddr;
    localAddr.locIfName = "lo"; 
    localAddr.locAddr = ACE_INET_Addr(uint16_t(8000), uint32_t(0x7f000001));
    localAddr.hasRem = false;
    StreamBlockAcceptor::addrSet_t remAddrSet;

    bool result = bac.Listen(localAddr, localAddr.locAddr.get_port_number(), remAddrSet, engine.GetAcceptDelegate(), engine.GetCloseDelegate());

    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetAccepts().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetCloses().size());

    // simulate an incoming connection from 10.1.2.3 port 123
    ACE_INET_Addr remoteAddr0 = ACE_INET_Addr(uint16_t(123), uint32_t(0x0a010203));
    result = bac.MockAccept(localAddr.locIfName, localAddr.locAddr, remoteAddr0);
    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetAccepts().size());

    // simulate another connection from someplace else
    ACE_INET_Addr remoteAddr1 = ACE_INET_Addr(uint16_t(126), uint32_t(0x0a010204));

    result = bac.MockAccept(localAddr.locIfName, localAddr.locAddr, remoteAddr1);
    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetAccepts().size());
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestStreamBlockAcceptor);
