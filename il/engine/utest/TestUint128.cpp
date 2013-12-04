#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "Uint128.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class TestUint128 : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestUint128);
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
    TestUint128()
    {
    }

    ~TestUint128()
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

void TestUint128::testCompare()
{
    uint128_t zero;
    uint128_t zero_x(0, 0);
    uint128_t one(0, 1);
    uint128_t big(1, 0);

    CPPUNIT_ASSERT(zero == zero_x);
    CPPUNIT_ASSERT(zero != one);
    CPPUNIT_ASSERT(zero != big);
    CPPUNIT_ASSERT(big != one);
    CPPUNIT_ASSERT(big == big);

    uint128_t big2 = big;
    CPPUNIT_ASSERT_EQUAL(big2, big);
}

///////////////////////////////////////////////////////////////////////////////

void TestUint128::testShift()
{
    uint128_t zero;
    uint128_t one(0, 1);
    uint128_t big(1, 0);
    uint128_t other(0x1001, 0x10000001);

    CPPUNIT_ASSERT_EQUAL(zero, zero << 0);
    CPPUNIT_ASSERT_EQUAL(zero, zero << 1);
    CPPUNIT_ASSERT_EQUAL(zero, zero << 15);
    CPPUNIT_ASSERT_EQUAL(zero, zero << 31);
    CPPUNIT_ASSERT_EQUAL(zero, zero << 47);

    CPPUNIT_ASSERT_EQUAL(one, one << 0 );
    CPPUNIT_ASSERT_EQUAL(uint128_t(0, 0x2), one << 1 );
    CPPUNIT_ASSERT_EQUAL(uint128_t(0, 0x8000), one << 15);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0, 0x80000000), one << 31);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x0, 0x100000000ull), one << 32);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x1, 0), one << 64);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x2, 0), one << 65);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x8000000000000000ull, 0), one << 127);
    CPPUNIT_ASSERT_EQUAL(zero, one << 128);

    CPPUNIT_ASSERT_EQUAL(big, big << 0 );
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x2, 0), big << 1);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x8000, 0), big << 15);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x80000000, 0), big << 31);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x8000000000000000ull, 0), big << 63);
    CPPUNIT_ASSERT_EQUAL(zero, big << 64);
    CPPUNIT_ASSERT_EQUAL(zero, big << 127);
    CPPUNIT_ASSERT_EQUAL(zero, big << 128);

    CPPUNIT_ASSERT_EQUAL(other, other << 0);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x2002, 0x20000002), other << 1);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x8008000, 0x80000008000ull), other << 15);
    CPPUNIT_ASSERT_EQUAL(uint128_t(     0x80080000000ull,  0x800000080000000ull), other << 31);
    CPPUNIT_ASSERT_EQUAL(uint128_t(    0x100100000000ull, 0x1000000100000000ull), other << 32);
    CPPUNIT_ASSERT_EQUAL(uint128_t(   0x1001000000001ull,       0x1000000000ull), other << 36);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x8008000000008000ull,    0x8000000000000ull), other << 51);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x8000000008000000ull, 0x8000000000000000ull), other << 63);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x10000001, 0), other << 64);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x8000000000000000ull, 0), other << 127);
    CPPUNIT_ASSERT_EQUAL(zero, other << 128);

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
    CPPUNIT_ASSERT_EQUAL(uint128_t(0, 0x8000000000000000ull), big >> 1);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0,    0x2000000000000ull), big >> 15);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0,         0x80000000), big >> 33);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0,                0x1), big >> 64);
    CPPUNIT_ASSERT_EQUAL(zero, big >> 65);
    CPPUNIT_ASSERT_EQUAL(zero, big >> 128);

    CPPUNIT_ASSERT_EQUAL(other, other >> 0);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x0800, 0x8000000008000000ull), other >> 1);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0,      0x2002000000002000ull), other >> 15);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0,      0x1001000000001000ull), other >> 16);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0,          0x100100000000ull), other >> 32);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0,                  0x1001ull), other >> 64);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0,                     0x1ull), other >> 76);
    CPPUNIT_ASSERT_EQUAL(zero, other >> 77);
    CPPUNIT_ASSERT_EQUAL(zero, other >> 128);
}

///////////////////////////////////////////////////////////////////////////////

void TestUint128::testAnd()
{
    uint128_t zero;
    uint128_t one(0, 1);
    uint128_t big(1, 0);
    uint128_t all_ones(0xffffffffffffffffull, 0xffffffffffffffffull);
    uint128_t other(0x1001, 0x10000001);

    CPPUNIT_ASSERT_EQUAL(one, all_ones & one);
    CPPUNIT_ASSERT_EQUAL(other, other & all_ones);
    CPPUNIT_ASSERT_EQUAL(zero, one & big);
    CPPUNIT_ASSERT_EQUAL(one, one & other);
    CPPUNIT_ASSERT_EQUAL(big, big & other);
}

///////////////////////////////////////////////////////////////////////////////

void TestUint128::testOr()
{
    uint128_t zero;
    uint128_t one(0, 1);
    uint128_t big(1, 0);
    uint128_t all_ones(0xffffffffffffffffull, 0xffffffffffffffffull);
    uint128_t other(0x1001, 0x10000001);

    CPPUNIT_ASSERT_EQUAL(all_ones, all_ones | one);
    CPPUNIT_ASSERT_EQUAL(all_ones, other | all_ones);
    CPPUNIT_ASSERT_EQUAL(one, one | zero);
    CPPUNIT_ASSERT_EQUAL(big, zero | big);
    CPPUNIT_ASSERT_EQUAL(other, big | other);
    CPPUNIT_ASSERT_EQUAL(other, one | other);
    CPPUNIT_ASSERT_EQUAL(other, zero | other);
    CPPUNIT_ASSERT_EQUAL(uint128_t(1,1), one | big);
}

///////////////////////////////////////////////////////////////////////////////

void TestUint128::testNot()
{
    uint128_t zero;
    uint128_t one(0, 1);
    uint128_t big(1, 0);
    uint128_t all_ones(0xffffffffffffffffull, 0xffffffffffffffffull);
    uint128_t other(0x1001, 0x10000001);

    CPPUNIT_ASSERT_EQUAL(zero, ~all_ones);
    CPPUNIT_ASSERT_EQUAL(all_ones, ~zero);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0xffffffffffffffffull, 0xfffffffffffffffeull), ~one);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0xfffffffffffffffeull, 0xffffffffffffffffull), ~big);
}

///////////////////////////////////////////////////////////////////////////////

void TestUint128::testAdd()
{
    uint128_t zero;
    uint128_t one(0, 1);
    uint128_t big(1, 0);
    uint128_t other(0x1001, 0xffffffffffffffffull);

    CPPUNIT_ASSERT_EQUAL(one, zero + one);
    CPPUNIT_ASSERT_EQUAL(big, big + zero);
    CPPUNIT_ASSERT_EQUAL(uint128_t(1, 1), big + one);
    CPPUNIT_ASSERT_EQUAL(uint128_t(2, 0), big + big);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x1002, 0), other + one);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x1003, 1), other + one + one + big);
}

///////////////////////////////////////////////////////////////////////////////

void TestUint128::testSub()
{
    uint128_t zero;
    uint128_t one(0, 1);
    uint128_t big(1, 0);
    uint128_t other(0x1001, 0xffffffffffffffffull);

    CPPUNIT_ASSERT_EQUAL(one, one - zero);
    CPPUNIT_ASSERT_EQUAL(uint128_t(0, 0xffffffffffffffffull), big - one);
    CPPUNIT_ASSERT_EQUAL(big, uint128_t(1, 1) - one);
    CPPUNIT_ASSERT_EQUAL(big, uint128_t(2, 0) - big);
    CPPUNIT_ASSERT_EQUAL(one, uint128_t(0x1002, 0) - other);
    CPPUNIT_ASSERT_EQUAL(other, uint128_t(0x1003, 1) - one - one - big);
}

///////////////////////////////////////////////////////////////////////////////

void TestUint128::testMult()
{
    uint128_t zero;
    uint128_t one(0, 1);
    uint128_t big(1, 0);
    uint128_t foo(0x2000000220000002ull,0x1000000110000001ull);
    uint128_t other(0x1001, 0x1000000000000002ull);

    CPPUNIT_ASSERT_EQUAL(one, one * uint32_t(1));
    CPPUNIT_ASSERT_EQUAL(other, other * uint32_t(1));
    CPPUNIT_ASSERT_EQUAL(uint128_t(0, 1000), one * uint32_t(1000));
    CPPUNIT_ASSERT_EQUAL(uint128_t(60000, 0), big * uint32_t(60000));
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x4000000440000004ull, 0x2000000220000002ull), foo * uint32_t(2));
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x0000002200000021ull, 0x0000001100000010ull), foo * uint32_t(0x10));
    CPPUNIT_ASSERT_EQUAL(uint128_t(0x20022, 0x40), other * uint32_t(0x20));
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestUint128);
