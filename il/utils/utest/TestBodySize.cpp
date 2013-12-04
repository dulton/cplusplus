#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "BodySize.h"

USING_L4L7_UTILS_NS;

///////////////////////////////////////////////////////////////////////////////

class TestBodySize : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestBodySize);
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

void TestBodySize::testFixed(void)
{
    randomEngine_t re(123);
    BodySize b(re);

    // test default size 1, fixed
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), b.Get());

    b.Set(2);
    CPPUNIT_ASSERT_EQUAL(uint32_t(2), b.Get());

    // 0 is valid
    b.Set(0);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), b.Get());

    // supports max int
    b.Set(0xffffffff);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0xffffffff), b.Get());
}

///////////////////////////////////////////////////////////////////////////////

void TestBodySize::testRandom(void)
{
    randomEngine_t re(123);
    BodySize b(re);

    // test always nonnegative
    b.Set(0, 100);

    CPPUNIT_ASSERT(b.Get() >= 0);
    CPPUNIT_ASSERT(b.Get() >= 0);
    CPPUNIT_ASSERT(b.Get() >= 0);
    CPPUNIT_ASSERT(b.Get() >= 0);
    CPPUNIT_ASSERT(b.Get() >= 0);

    // test stddev of 0
    b.Set(500, 0);
    CPPUNIT_ASSERT_EQUAL(uint32_t(500), b.Get());
    CPPUNIT_ASSERT_EQUAL(uint32_t(500), b.Get());
 
    // test random - fix test if seed or # of Gets() changes
    b.Set(1000, 1000);

    CPPUNIT_ASSERT_EQUAL(uint32_t(427), b.Get());
    CPPUNIT_ASSERT_EQUAL(uint32_t(1935), b.Get());
    CPPUNIT_ASSERT_EQUAL(uint32_t(3284), b.Get());

    // test max mean
    b.Set(0xffffffff, 1000);
    CPPUNIT_ASSERT_EQUAL(uint32_t(4294966307UL), b.Get());
    CPPUNIT_ASSERT_EQUAL(uint32_t(4294966432UL), b.Get());

    // test max stddev
    b.Set(5000, 0xffffffff);
    CPPUNIT_ASSERT_EQUAL(uint32_t(3518616568UL), b.Get());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), b.Get());

    // test max both
    b.Set(0xffffffff, 0xffffffff);
    CPPUNIT_ASSERT_EQUAL(uint32_t(1979439033), b.Get());
    CPPUNIT_ASSERT_EQUAL(uint32_t(467938193), b.Get());
}

///////////////////////////////////////////////////////////////////////////////

void TestBodySize::testBoth(void)
{
    randomEngine_t re(123);
    BodySize b(re);

    // make fixed
    b.Set(1);
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), b.Get());

    // make 'random'
    b.Set(1000, 0);
    CPPUNIT_ASSERT_EQUAL(uint32_t(1000), b.Get());
    
    // make fixed
    b.Set(2000);
    CPPUNIT_ASSERT_EQUAL(uint32_t(2000), b.Get());
    
    // make random - don't change the seed or the number of times Get is called
    b.Set(4000, 1000);
    CPPUNIT_ASSERT_EQUAL(uint32_t(5452), b.Get());
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestBodySize);
