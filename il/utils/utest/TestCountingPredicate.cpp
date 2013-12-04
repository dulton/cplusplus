#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "CountingPredicate.tcc"

USING_L4L7_UTILS_NS;

///////////////////////////////////////////////////////////////////////////////

class TestCountingPredicate : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestCountingPredicate);
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

void TestCountingPredicate::testWhile()
{
    CountingPredicate<size_t, 5> loop;
    size_t count = 0;

    while (loop())
    {
        ++count;
    }

    CPPUNIT_ASSERT_EQUAL(size_t(5), count);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestCountingPredicate);
