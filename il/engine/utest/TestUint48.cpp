#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "Uint48.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class TestUint48 : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestUint48);
    CPPUNIT_TEST(testCompare);
    CPPUNIT_TEST(testShift);
    CPPUNIT_TEST(testAnd);
    CPPUNIT_TEST(testOr);
    CPPUNIT_TEST(testNot);
    CPPUNIT_TEST(testAdd);
    CPPUNIT_TEST(testSub);
    CPPUNIT_TEST(testMult);
    CPPUNIT_TEST_SUITE_END();

public:
    TestUint48()
    {
    }

    ~TestUint48()
    {
    }

    void setUp() 
    { 
    }

    void tearDown() 
    { 
    }
    
protected:
    void testCompare();
    void testShift();
    void testAnd();
    void testOr();
    void testNot();
    void testAdd();
    void testSub();
    void testMult();

private:
};

///////////////////////////////////////////////////////////////////////////////

// NOTE: if we ever need this in non-unit test code, add it to a cpp file someplace else
std::ostream& L4L7_ENGINE_NS::operator<<(std::ostream& os, const uint48_t& i)
{
    return os << '(' << i.mHi << ',' << i.mLo << ')';
}

///////////////////////////////////////////////////////////////////////////////

void TestUint48::testCompare()
{
    uint48_t zero;
    uint48_t zero_x(0, 0);
    uint48_t one(0, 1);
    uint48_t big(1, 0);

    CPPUNIT_ASSERT(zero == zero_x);
    CPPUNIT_ASSERT(zero != one);
    CPPUNIT_ASSERT(zero != big);
    CPPUNIT_ASSERT(big != one);
    CPPUNIT_ASSERT(big == big);

    uint48_t big2 = big;
    CPPUNIT_ASSERT_EQUAL(big2, big);
}

///////////////////////////////////////////////////////////////////////////////

void TestUint48::testShift()
{
    uint48_t zero;
    uint48_t one(0, 1);
    uint48_t big(1, 0);
    uint48_t other(0x1001, 0x10000001);

    CPPUNIT_ASSERT_EQUAL(zero, zero << 0);
    CPPUNIT_ASSERT_EQUAL(zero, zero << 1);
    CPPUNIT_ASSERT_EQUAL(zero, zero << 15);
    CPPUNIT_ASSERT_EQUAL(zero, zero << 31);
    CPPUNIT_ASSERT_EQUAL(zero, zero << 47);

    CPPUNIT_ASSERT_EQUAL(one, one << 0 );
    CPPUNIT_ASSERT_EQUAL(uint48_t(0, 0x2), one << 1 );
    CPPUNIT_ASSERT_EQUAL(uint48_t(0, 0x8000), one << 15);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0, 0x80000000), one << 31);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0x1, 0), one << 32);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0x2, 0), one << 33);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0x8000, 0), one << 47);
    CPPUNIT_ASSERT_EQUAL(zero, one << 48);

    CPPUNIT_ASSERT_EQUAL(big, big << 0 );
    CPPUNIT_ASSERT_EQUAL(uint48_t(0x2, 0), big << 1);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0x8000, 0), big << 15);
    CPPUNIT_ASSERT_EQUAL(zero, big << 16);
    CPPUNIT_ASSERT_EQUAL(zero, big << 31);
    CPPUNIT_ASSERT_EQUAL(zero, big << 32);
    CPPUNIT_ASSERT_EQUAL(zero, big << 47);
    CPPUNIT_ASSERT_EQUAL(zero, big << 48);

    CPPUNIT_ASSERT_EQUAL(other, other << 0);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0x2002, 0x20000002), other << 1);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0x8800, 0x00008000), other << 15);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0, 0x80000000), other << 31);
    CPPUNIT_ASSERT_EQUAL(uint48_t(1, 0), other << 32);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0x8000, 0), other << 47);
    CPPUNIT_ASSERT_EQUAL(zero, other << 48);

    // right shift
    
    CPPUNIT_ASSERT_EQUAL(zero, zero >> 0);
    CPPUNIT_ASSERT_EQUAL(zero, zero >> 1);
    CPPUNIT_ASSERT_EQUAL(zero, zero >> 15);
    CPPUNIT_ASSERT_EQUAL(zero, zero >> 31);
    CPPUNIT_ASSERT_EQUAL(zero, zero >> 47);

    CPPUNIT_ASSERT_EQUAL(one, one >> 0 );
    CPPUNIT_ASSERT_EQUAL(zero, one >> 1 );
    CPPUNIT_ASSERT_EQUAL(zero, one >> 15);
    CPPUNIT_ASSERT_EQUAL(zero, one >> 31);
    CPPUNIT_ASSERT_EQUAL(zero, one >> 32);
    CPPUNIT_ASSERT_EQUAL(zero, one >> 33);
    CPPUNIT_ASSERT_EQUAL(zero, one >> 47);
    CPPUNIT_ASSERT_EQUAL(zero, one >> 48);

    CPPUNIT_ASSERT_EQUAL(big, big >> 0 );
    CPPUNIT_ASSERT_EQUAL(uint48_t(0, 0x80000000), big >> 1);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0, 0x00020000), big >> 15);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0, 0x00010000), big >> 16);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0, 0x00000002), big >> 31);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0, 0x00000001), big >> 32);
    CPPUNIT_ASSERT_EQUAL(zero, big >> 33);
    CPPUNIT_ASSERT_EQUAL(zero, big >> 47);
    CPPUNIT_ASSERT_EQUAL(zero, big >> 48);

    CPPUNIT_ASSERT_EQUAL(other, other >> 0);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0x0800, 0x88000000), other >> 1);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0x0000, 0x20022000), other >> 15);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0x0000, 0x10011000), other >> 16);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0, 0x00002002), other >> 31);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0, 0x00001001), other >> 32);
    CPPUNIT_ASSERT_EQUAL(zero, other >> 47);
    CPPUNIT_ASSERT_EQUAL(zero, other >> 48);

}

///////////////////////////////////////////////////////////////////////////////

void TestUint48::testAnd()
{
    uint48_t zero;
    uint48_t one(0, 1);
    uint48_t big(1, 0);
    uint48_t all_ones(0xffff, 0xffffffff);
    uint48_t other(0x1001, 0x10000001);

    CPPUNIT_ASSERT_EQUAL(one, all_ones & one);
    CPPUNIT_ASSERT_EQUAL(other, other & all_ones);
    CPPUNIT_ASSERT_EQUAL(zero, one & big);
    CPPUNIT_ASSERT_EQUAL(one, one & other);
    CPPUNIT_ASSERT_EQUAL(big, big & other);
}

///////////////////////////////////////////////////////////////////////////////

void TestUint48::testOr()
{
    uint48_t zero;
    uint48_t one(0, 1);
    uint48_t big(1, 0);
    uint48_t all_ones(0xffff, 0xffffffff);
    uint48_t other(0x1001, 0x10000001);

    CPPUNIT_ASSERT_EQUAL(all_ones, all_ones | one);
    CPPUNIT_ASSERT_EQUAL(all_ones, other | all_ones);
    CPPUNIT_ASSERT_EQUAL(one, one | zero);
    CPPUNIT_ASSERT_EQUAL(big, zero | big);
    CPPUNIT_ASSERT_EQUAL(other, big | other);
    CPPUNIT_ASSERT_EQUAL(other, one | other);
    CPPUNIT_ASSERT_EQUAL(other, zero | other);
    CPPUNIT_ASSERT_EQUAL(uint48_t(1,1), one | big);
}

///////////////////////////////////////////////////////////////////////////////

void TestUint48::testNot()
{
    uint48_t zero;
    uint48_t one(0, 1);
    uint48_t big(1, 0);
    uint48_t all_ones(0xffff, 0xffffffff);
    uint48_t other(0x1001, 0x10000001);

    CPPUNIT_ASSERT_EQUAL(zero, ~all_ones);
    CPPUNIT_ASSERT_EQUAL(all_ones, ~zero);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0xffff, 0xfffffffe), ~one);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0xfffe, 0xffffffff), ~big);
}

///////////////////////////////////////////////////////////////////////////////

void TestUint48::testAdd()
{
    uint48_t zero;
    uint48_t one(0, 1);
    uint48_t big(1, 0);
    uint48_t other(0x1001, 0xffffffff);

    CPPUNIT_ASSERT_EQUAL(one, zero + one);
    CPPUNIT_ASSERT_EQUAL(big, big + zero);
    CPPUNIT_ASSERT_EQUAL(uint48_t(1, 1), big + one);
    CPPUNIT_ASSERT_EQUAL(uint48_t(2, 0), big + big);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0x1002, 0), other + one);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0x1003, 1), other + one + one + big);
}

///////////////////////////////////////////////////////////////////////////////

void TestUint48::testSub()
{
    uint48_t zero;
    uint48_t one(0, 1);
    uint48_t big(1, 0);
    uint48_t other(0x1001, 0xffffffff);

    CPPUNIT_ASSERT_EQUAL(one, one - zero);
    CPPUNIT_ASSERT_EQUAL(uint48_t(0, 0xffffffff), big - one);
    CPPUNIT_ASSERT_EQUAL(big, uint48_t(1, 1) - one);
    CPPUNIT_ASSERT_EQUAL(big, uint48_t(2, 0) - big);
    CPPUNIT_ASSERT_EQUAL(one, uint48_t(0x1002, 0) - other);
    CPPUNIT_ASSERT_EQUAL(other, uint48_t(0x1003, 1) - one - one - big);
}

///////////////////////////////////////////////////////////////////////////////

void TestUint48::testMult()
{
    uint48_t zero;
    uint48_t one(0, 1);
    uint48_t big(1, 0);
    uint48_t other(0x1001, 0x10000002);

    CPPUNIT_ASSERT_EQUAL(one, one * uint32_t(1));
    CPPUNIT_ASSERT_EQUAL(other, other * uint32_t(1));
    CPPUNIT_ASSERT_EQUAL(uint48_t(0, 1000), one * uint32_t(1000));
    CPPUNIT_ASSERT_EQUAL(uint48_t(60000, 0), big * uint32_t(60000));
    CPPUNIT_ASSERT_EQUAL(uint48_t(0x0022, 0x40), other * uint32_t(32));
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestUint48);
