#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "PortDelegateMap.h"

#include "MockEngine.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestPortDelegateMap : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestPortDelegateMap);
    CPPUNIT_TEST(testBasic);
    CPPUNIT_TEST(testMultiRoot);
    CPPUNIT_TEST_SUITE_END();

public:
    TestPortDelegateMap()
    {
    }

    ~TestPortDelegateMap()
    {
    }

    void setUp() 
    { 
    }

    void tearDown() 
    { 
        mConnectDelegate = TestablePortDelegateMap::connectDelegate_t();
        mCloseDelegate = TestablePortDelegateMap::closeDelegate_t();
    }

    typedef PortDelegateMap<TestPortDelegateMap> TestablePortDelegateMap;

    void RegisterDelegates(const TestablePortDelegateMap::connectDelegate_t& conDel, const TestablePortDelegateMap::closeDelegate_t& cloDel) { mConnectDelegate = conDel; mCloseDelegate = cloDel; }
    
protected:
    void testBasic();
    void testMultiRoot();

private:
    TestablePortDelegateMap::connectDelegate_t mConnectDelegate;
    TestablePortDelegateMap::closeDelegate_t mCloseDelegate;
};

///////////////////////////////////////////////////////////////////////////////

void TestPortDelegateMap::testBasic()
{
    MockEngine rootEngine;

    PortDelegateMap<TestPortDelegateMap> map(ACE_INET_Addr(), rootEngine.GetConnectDelegate(), rootEngine.GetCloseDelegate());

    CPPUNIT_ASSERT_EQUAL(size_t(0), rootEngine.GetConnects().size());
    map.mRoot.first(123);
    CPPUNIT_ASSERT_EQUAL(size_t(1), rootEngine.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(int(123), rootEngine.GetConnects()[0].fd);

    CPPUNIT_ASSERT_EQUAL(size_t(0), rootEngine.GetCloses().size());
    map.mRoot.second(L4L7_ENGINE_NS::AbstSocketManager::RX_RST);
    CPPUNIT_ASSERT_EQUAL(size_t(1), rootEngine.GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(int(L4L7_ENGINE_NS::AbstSocketManager::RX_RST), int(rootEngine.GetCloses()[0].closeType));

    // If no other sources are added, always return the root
    map.register_and_pop(ACE_INET_Addr(), *this);
    mConnectDelegate(234);
    CPPUNIT_ASSERT_EQUAL(size_t(2), rootEngine.GetConnects().size());
    map.register_and_pop(ACE_INET_Addr(uint16_t(1), 0x01020304), *this);
    mConnectDelegate(345);
    CPPUNIT_ASSERT_EQUAL(size_t(3), rootEngine.GetConnects().size());
    map.register_and_pop(ACE_INET_Addr(uint16_t(65535), 0x01020304), *this);
    mConnectDelegate(456);
    CPPUNIT_ASSERT_EQUAL(size_t(4), rootEngine.GetConnects().size());

    // add children

    MockEngine child0, child1, child2;

    ACE_INET_Addr addr0(uint16_t(9832), 0x01020304);
    ACE_INET_Addr addr1(uint16_t(9833), 0x01020304);
    ACE_INET_Addr addr2(uint16_t(9832), 0x01020303);

    map.push(addr0, child0.GetConnectDelegate(), child0.GetCloseDelegate());
    map.push(addr1, child1.GetConnectDelegate(), child1.GetCloseDelegate());
    map.push(addr2, child2.GetConnectDelegate(), child2.GetCloseDelegate());

    // test register - root
    map.register_and_pop(ACE_INET_Addr(uint16_t(1), 0x01020304), *this);
    mConnectDelegate(567);
    CPPUNIT_ASSERT_EQUAL(size_t(5), rootEngine.GetConnects().size());
    mCloseDelegate(L4L7_ENGINE_NS::AbstSocketManager::TX_RST);
    CPPUNIT_ASSERT_EQUAL(size_t(2), rootEngine.GetCloses().size());

    // child1
    map.register_and_pop(addr1, *this);
    CPPUNIT_ASSERT_EQUAL(size_t(0), child1.GetConnects().size());
    mConnectDelegate(123);
    CPPUNIT_ASSERT_EQUAL(size_t(1), child1.GetConnects().size());

    // child1 again -> root
    map.register_and_pop(addr1, *this);
    mConnectDelegate(678);
    CPPUNIT_ASSERT_EQUAL(size_t(6), rootEngine.GetConnects().size());

    // child0
    map.register_and_pop(addr0, *this);
    CPPUNIT_ASSERT_EQUAL(size_t(0), child0.GetConnects().size());
    mConnectDelegate(123);
    CPPUNIT_ASSERT_EQUAL(size_t(1), child0.GetConnects().size());

    // child2
    map.register_and_pop(addr2, *this);
    CPPUNIT_ASSERT_EQUAL(size_t(0), child2.GetConnects().size());
    mConnectDelegate(345);
    CPPUNIT_ASSERT_EQUAL(size_t(1), child2.GetConnects().size());
}

///////////////////////////////////////////////////////////////////////////////

void TestPortDelegateMap::testMultiRoot()
{
    MockEngine rootEngine0;

    PortDelegateMap<TestPortDelegateMap> map(ACE_INET_Addr(uint16_t(1234), uint32_t(0x01020304)), rootEngine0.GetConnectDelegate(), rootEngine0.GetCloseDelegate());

    // If no other sources are added, always return the root
    map.register_and_pop(ACE_INET_Addr(uint16_t(1), 0x01020304), *this);
    mConnectDelegate(234);
    CPPUNIT_ASSERT_EQUAL(size_t(1), rootEngine0.GetConnects().size());

    // add root-children

    MockEngine rootEngine1, rootEngine2;

    ACE_INET_Addr rootAddr1(uint16_t(0), uint32_t(0x01020305));
    ACE_INET_Addr rootAddr2(uint16_t(0), uint32_t(0x01020306));

    map.push(rootAddr1, rootEngine1.GetConnectDelegate(), rootEngine1.GetCloseDelegate());
    map.push(rootAddr2, rootEngine2.GetConnectDelegate(), rootEngine2.GetCloseDelegate());

    // verify that different roots accept incoming connections from any port
    map.register_and_pop(ACE_INET_Addr(uint16_t(999), 0x01020304), *this);
    mConnectDelegate(567);
    CPPUNIT_ASSERT_EQUAL(size_t(2), rootEngine0.GetConnects().size());
    map.register_and_pop(ACE_INET_Addr(uint16_t(999), 0x01020306), *this);
    mConnectDelegate(678);
    CPPUNIT_ASSERT_EQUAL(size_t(1), rootEngine2.GetConnects().size());
    map.register_and_pop(ACE_INET_Addr(uint16_t(999), 0x01020305), *this);
    mConnectDelegate(789);
    CPPUNIT_ASSERT_EQUAL(size_t(1), rootEngine1.GetConnects().size());
    
    // verify that different roots are never popped
    map.register_and_pop(ACE_INET_Addr(uint16_t(888), 0x01020305), *this);
    mConnectDelegate(901);
    CPPUNIT_ASSERT_EQUAL(size_t(2), rootEngine1.GetConnects().size());
    map.register_and_pop(ACE_INET_Addr(uint16_t(888), 0x01020306), *this);
    mConnectDelegate(123);
    CPPUNIT_ASSERT_EQUAL(size_t(2), rootEngine2.GetConnects().size());
    map.register_and_pop(ACE_INET_Addr(uint16_t(888), 0x01020304), *this);
    mConnectDelegate(234);
    CPPUNIT_ASSERT_EQUAL(size_t(3), rootEngine0.GetConnects().size());

    // add non-root children
    
    MockEngine child0, child1, child2;

    ACE_INET_Addr childAddr0(uint16_t(9832), 0x01020305);
    ACE_INET_Addr childAddr1(uint16_t(9833), 0x01020305);
    ACE_INET_Addr childAddr2(uint16_t(9832), 0x01020306);

    map.push(childAddr0, child0.GetConnectDelegate(), child0.GetCloseDelegate());
    map.push(childAddr1, child1.GetConnectDelegate(), child1.GetCloseDelegate());
    map.push(childAddr2, child2.GetConnectDelegate(), child2.GetCloseDelegate());

    // child1
    map.register_and_pop(childAddr1, *this);
    CPPUNIT_ASSERT_EQUAL(size_t(0), child1.GetConnects().size());
    mConnectDelegate(123);
    CPPUNIT_ASSERT_EQUAL(size_t(1), child1.GetConnects().size());

    // child1 again -> root1
    map.register_and_pop(childAddr1, *this);
    mConnectDelegate(678);
    CPPUNIT_ASSERT_EQUAL(size_t(3), rootEngine1.GetConnects().size());

    // child0
    map.register_and_pop(childAddr0, *this);
    CPPUNIT_ASSERT_EQUAL(size_t(0), child0.GetConnects().size());
    mConnectDelegate(123);
    CPPUNIT_ASSERT_EQUAL(size_t(1), child0.GetConnects().size());

    // child2
    map.register_and_pop(childAddr2, *this);
    CPPUNIT_ASSERT_EQUAL(size_t(0), child2.GetConnects().size());
    mConnectDelegate(345);
    CPPUNIT_ASSERT_EQUAL(size_t(1), child2.GetConnects().size());

    // child2 again - root2
    map.register_and_pop(childAddr2, *this);
    mConnectDelegate(345);
    CPPUNIT_ASSERT_EQUAL(size_t(1), child2.GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), rootEngine2.GetConnects().size());
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestPortDelegateMap);
