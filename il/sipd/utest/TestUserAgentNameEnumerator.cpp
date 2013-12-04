#include <cppunit/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <boost/scoped_ptr.hpp>
#include <phxexception/PHXException.h>

#include "VoIPCommon.h"

#include "UserAgentNameEnumerator.h"

#include "TestUtilities.h"

USING_APP_NS;

///////////////////////////////////////////////////////////////////////////////

class TestUserAgentNameEnumerator : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestUserAgentNameEnumerator);
    CPPUNIT_TEST(testNormalEnumerator);
    CPPUNIT_TEST(testStaticEnumerator);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) { }
    void tearDown(void) { }

protected:
    void testNormalEnumerator(void);
    void testStaticEnumerator(void);
};

///////////////////////////////////////////////////////////////////////////////

void TestUserAgentNameEnumerator::testNormalEnumerator(void)
{
    boost::scoped_ptr<UserAgentNameEnumerator> uaNameEnum(new UserAgentNameEnumerator(1000, 1, 1004, ""));

    _CPPUNIT_ASSERT(uaNameEnum->TotalCount() == 4);

    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "1000");
    uaNameEnum->Next();
    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "1001");
    uaNameEnum->Next();
    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "1002");
    uaNameEnum->Next();
    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "1003");
    uaNameEnum->Next();
    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "1000");
    uaNameEnum->Next();
    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "1001");
    uaNameEnum->Reset();
    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "1000");

    uaNameEnum->SetPosition(1);
    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "1001");
    uaNameEnum->SetPosition(3);
    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "1003");

    _CPPUNIT_ASSERT_THROW(uaNameEnum->SetPosition(4), EFault);
}

///////////////////////////////////////////////////////////////////////////////

void TestUserAgentNameEnumerator::testStaticEnumerator(void)
{
    boost::scoped_ptr<UserAgentNameEnumerator> uaNameEnum(new UserAgentNameEnumerator(1000, 1, 1004, "name"));

    _CPPUNIT_ASSERT(uaNameEnum->TotalCount() == 4);

    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "name");
    uaNameEnum->Next();
    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "name");
    uaNameEnum->Next();
    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "name");
    uaNameEnum->Next();
    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "name");
    uaNameEnum->Next();
    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "name");
    uaNameEnum->Next();
    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "name");
    uaNameEnum->Reset();
    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "name");

    uaNameEnum->SetPosition(1);
    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "name");
    uaNameEnum->SetPosition(3);
    _CPPUNIT_ASSERT(uaNameEnum->GetName() == "name");

    _CPPUNIT_ASSERT_THROW(uaNameEnum->SetPosition(4), EFault);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestUserAgentNameEnumerator);
