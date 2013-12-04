#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "ResponseLatency.h"

USING_L4L7_UTILS_NS;

///////////////////////////////////////////////////////////////////////////////

class TestResponseLatency : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestResponseLatency);
    CPPUNIT_TEST(testFixed);
    CPPUNIT_TEST(testRandom);
    CPPUNIT_TEST(testBoth);
    CPPUNIT_TEST_SUITE_END();

    typedef boost::minstd_rand randomEngine_t;

public:
    void setUp(void) { }
    void tearDown(void) { }
    
protected:
    void testFixed(void);
    void testRandom(void);
    void testBoth(void);
};

///////////////////////////////////////////////////////////////////////////////

void TestResponseLatency::testFixed(void)
{
    randomEngine_t re(123);
    ResponseLatency r(re);

    // test default latency 0, fixed
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(0, 0), r.Get());

    // test msec conversion
    r.Set(2);
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(0, 2000), r.Get());

    r.Set(2000);
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(2, 0), r.Get());

    r.Set(987654321);
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(987654, 321000), r.Get());

    // supports max int
    r.Set(0xffffffff);
    CPPUNIT_ASSERT(ACE_Time_Value(4294967, 295000) == r.Get());
}

///////////////////////////////////////////////////////////////////////////////

void TestResponseLatency::testRandom(void)
{
    randomEngine_t re(123);
    ResponseLatency r(re);

    // test always nonnegative
    r.Set(0, 100);

    CPPUNIT_ASSERT(r.Get() >= 0);
    CPPUNIT_ASSERT(r.Get() >= 0);
    CPPUNIT_ASSERT(r.Get() >= 0);
    CPPUNIT_ASSERT(r.Get() >= 0);
    CPPUNIT_ASSERT(r.Get() >= 0);

    // test stddev of 0
    r.Set(500, 0);
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(0, 500000), r.Get());
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(0, 500000), r.Get());
 
    // test random - fix test if seed or # of Gets() changes
    r.Set(1000, 1000);

    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(0, 427143), r.Get());
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(1, 935366), r.Get());
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(3, 284358), r.Get());

    // test max mean
    r.Set(0xffffffff, 1000);
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(4294966, 307365), r.Get());
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(4294966, 432494), r.Get());

    // test max stddev
    r.Set(5000, 0xffffffff);
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(3518616, 568200), r.Get());
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(0, 0), r.Get());

    // test max both
    r.Set(0xffffffff, 0xffffffff);
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(6274406, 329391), r.Get());
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(467938, 193876), r.Get());
}

///////////////////////////////////////////////////////////////////////////////

void TestResponseLatency::testBoth(void)
{
    randomEngine_t re(123);
    ResponseLatency r(re);

    // make fixed
    r.Set(1);
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(0, 1000), r.Get());

    // make 'random'
    r.Set(1000, 0);
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(1, 0), r.Get());
    
    // make fixed
    r.Set(2000);
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(2, 0), r.Get());
    
    // make random - don't change the seed or the number of times Get is called
    r.Set(4000, 1000);
    CPPUNIT_ASSERT_EQUAL(ACE_Time_Value(5, 452348), r.Get());
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestResponseLatency);
