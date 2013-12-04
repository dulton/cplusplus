#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "AddrInfo.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestAddrInfo : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestAddrInfo);
    CPPUNIT_TEST(testToString);
    CPPUNIT_TEST_SUITE_END();

public:
    TestAddrInfo()
    {
    }

    ~TestAddrInfo()
    {
    }

    void setUp() 
    { 
    }

    void tearDown() 
    { 
    }
    
protected:
    void testToString();

private:
};

///////////////////////////////////////////////////////////////////////////////

void TestAddrInfo::testToString()
{
    AddrInfo addr;
    addr.locIfName = "foo";
    addr.locAddr.set(uint16_t(1234), uint32_t(0x7f000001));
    addr.hasRem = false;

    std::ostringstream ostr;
    ostr << addr;

    CPPUNIT_ASSERT_EQUAL(std::string("foo/127.0.0.1:1234"), ostr.str());

    ostr.str("");
    addr.locIfName = "bar";
    addr.locAddr.set(uint16_t(123), uint32_t(0x01020304));
    addr.remAddr.set(uint16_t(456), uint32_t(0x05060708));
    addr.hasRem = true;
    ostr << addr;

    CPPUNIT_ASSERT_EQUAL(std::string("bar/1.2.3.4:123/5.6.7.8:456"), ostr.str());
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestAddrInfo);
