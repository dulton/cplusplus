#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "TimingPredicate.tcc"

USING_L4L7_UTILS_NS;

///////////////////////////////////////////////////////////////////////////////

class TestTimingPredicate : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestTimingPredicate);
    CPPUNIT_TEST(testWhile);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() { }
    void tearDown() { }
    
protected:
    void testWhile();

private:
};

///////////////////////////////////////////////////////////////////////////////

void TestTimingPredicate::testWhile()
{
    ACE_Time_Value delay(2, 0); // 2 seconds
    TimingPredicate<2000> loop;

    ACE_Time_Value begin = ACE_OS::gettimeofday();

    while (loop())
    {
        ACE_OS::sleep(1);
    }

    ACE_Time_Value end = ACE_OS::gettimeofday();

    CPPUNIT_ASSERT(end - begin >= delay); // went more than delay
    CPPUNIT_ASSERT(end - begin - delay <= ACE_Time_Value(0, 100000)); // but not much more 
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestTimingPredicate);
