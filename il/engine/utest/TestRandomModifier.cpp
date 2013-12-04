#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <netinet/in.h> // ntoh*
#include "MockModifier.h"
#include "RandomModifier.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class TestRandomModifiers : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestRandomModifiers);
    CPPUNIT_TEST(testUint8Mod);
    CPPUNIT_TEST(testUint16Mod);
    CPPUNIT_TEST(testUint32Mod);
    CPPUNIT_TEST(testUint48Mod);
    CPPUNIT_TEST(testUint64Mod);
    CPPUNIT_TEST(testUint128Mod);
    CPPUNIT_TEST(testChain);
    CPPUNIT_TEST(testZeroCount);
    CPPUNIT_TEST_SUITE_END();

public:
    TestRandomModifiers()
    {
    }

    ~TestRandomModifiers()
    {
    }

    void setUp() 
    { 
    }

    void tearDown() 
    { 
    }
    
protected:
    void testUint8Mod();
    void testUint16Mod();
    void testUint32Mod();
    void testUint48Mod();
    void testUint64Mod();
    void testUint128Mod();
    void testChain();
    void testZeroCount();

private:
};

///////////////////////////////////////////////////////////////////////////////

void TestRandomModifiers::testUint8Mod()
{
    uint32_t seed = 0x12345678;

    RandomModifier<uint8_t> i(0xff, 3, seed);

    uint8_t value0 = 0;
    uint8_t value1 = 0;
    uint8_t value2 = 0;
    uint8_t value  = 0;
    
    CPPUNIT_ASSERT_EQUAL(size_t(1), i.GetSize());

    i.GetValue(&value0);
    i.Next();

    i.GetValue(&value1);
    i.Next();

    i.GetValue(&value2);
    i.Next();

    // Note: this is not a true test of randomness, we just want to make sure
    //       it's generating something reasonable
    CPPUNIT_ASSERT(value0 != value1);
    CPPUNIT_ASSERT(value1 != value2);

    // should repeat
    i.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL((int)value0, (int)value);

    i.SetCursor(1);
    i.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(value1, value);

    i.SetCursor(0);
    i.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(value0, value);

    // test different seed
    seed ^= 0x98765432;
    RandomModifier<uint8_t> j(0xff, 3, seed);

    j.GetValue(&value);
    CPPUNIT_ASSERT(value != value0);

    // test with mask
    RandomModifier<uint8_t> k(0xaa, 100, seed);

    value0 = 0;
    k.GetValue(&value0);
    
    k.Next();
    k.GetValue(&value1);

    k.Next();
    k.GetValue(&value2);

    for (size_t num = 0; num < 97; ++num)
    {
        k.Next();
        value = 0;
        k.GetValue(&value);
        CPPUNIT_ASSERT((value & 0xaa) == value);

        value = 0xff;
        k.GetValue(&value);
        CPPUNIT_ASSERT((value | 0xaa) == 0xff);
    }

    k.Next();
    value = 0;
    k.GetValue(&value);

    CPPUNIT_ASSERT_EQUAL(int(value0), int(value));

    // test with stutter
    RandomModifier<uint8_t> l(0xff, 10, seed, 1);
    
    l.GetValue(&value0);
   
    l.Next();
    l.GetValue(&value1);

    l.Next();
    l.GetValue(&value2);

    CPPUNIT_ASSERT_EQUAL((int)value0, (int)value1);
    CPPUNIT_ASSERT(value2 != value1);

    l.SetCursor(1);
    l.GetValue(&value);

    CPPUNIT_ASSERT_EQUAL((int)value, (int)value0);

    l.SetCursor(22);
    l.GetValue(&value);

    CPPUNIT_ASSERT_EQUAL((int)value, (int)value2);

    // verify swapping does nothing
    l.GetValue(&value1, true);
    CPPUNIT_ASSERT_EQUAL((int)value, (int)value1);
}

///////////////////////////////////////////////////////////////////////////////

void TestRandomModifiers::testUint16Mod()
{
    uint32_t seed = 0x12345678;

    RandomModifier<uint16_t> i(0xffff, 3, seed);

    uint16_t value0 = 0;
    uint16_t value1 = 0;
    uint16_t value2 = 0;
    uint16_t value  = 0;
    
    CPPUNIT_ASSERT_EQUAL(size_t(2), i.GetSize());

    i.GetValue((uint8_t*)&value0);
    i.Next();

    i.GetValue((uint8_t*)&value1);
    i.Next();

    i.GetValue((uint8_t*)&value2);
    i.Next();

    // Note: this is not a true test of randomness, we just want to make sure
    //       it's generating something reasonable
    CPPUNIT_ASSERT(value0 != value1);
    CPPUNIT_ASSERT(value1 != value2);

    // should repeat
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL((int)value0, (int)value);

    i.SetCursor(1);
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value1, value);

    i.SetCursor(0);
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value0, value);

    // test different seed
    seed ^= 0x98765432;
    RandomModifier<uint16_t> j(0xffff, 3, seed);

    j.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT(value != value0);

    // test with mask
    RandomModifier<uint16_t> k(0xabcd, 100, seed);

    value0 = 0;
    k.GetValue((uint8_t*)&value0);
    
    k.Next();
    k.GetValue((uint8_t*)&value1);

    k.Next();
    k.GetValue((uint8_t*)&value2);

    for (size_t num = 0; num < 97; ++num)
    {
        uint16_t value_buf = 0;
        k.Next();
        k.GetValue((uint8_t*)&value_buf);
        value = ntohs(value_buf);
        CPPUNIT_ASSERT((value & 0xabcd) == value);

        value_buf = 0xffff;
        k.GetValue((uint8_t*)&value_buf);
        value = ntohs(value_buf);
        CPPUNIT_ASSERT((value | 0xabcd) == 0xffff);
    }

    k.Next();
    value = 0;
    k.GetValue((uint8_t*)&value);

    CPPUNIT_ASSERT_EQUAL(int(value0), int(value));

    // test with stutter
    RandomModifier<uint16_t> l(0xffff, 10, seed, 1);
    
    l.GetValue((uint8_t*)&value0);
   
    l.Next();
    l.GetValue((uint8_t*)&value1);

    l.Next();
    l.GetValue((uint8_t*)&value2);

    CPPUNIT_ASSERT_EQUAL((int)value0, (int)value1);
    CPPUNIT_ASSERT(value2 != value1);

    l.SetCursor(1);
    l.GetValue((uint8_t*)&value);

    CPPUNIT_ASSERT_EQUAL((int)value, (int)value0);

    l.SetCursor(22);
    l.GetValue((uint8_t*)&value);

    CPPUNIT_ASSERT_EQUAL((int)value, (int)value2);

    // verify swapping
    l.GetValue((uint8_t*)&value1, true);
    CPPUNIT_ASSERT_EQUAL(value & 0xff, (value1 >> 8) & 0xff);
    CPPUNIT_ASSERT_EQUAL(value & 0xff00, (value1 << 8) & 0xff00);
}

///////////////////////////////////////////////////////////////////////////////

void TestRandomModifiers::testUint32Mod()
{
    uint32_t seed = 0x12345678;

    RandomModifier<uint32_t> i(0xffffffff, 3, seed);

    uint32_t value0 = 0;
    uint32_t value1 = 0;
    uint32_t value2 = 0;
    uint32_t value  = 0;
    
    CPPUNIT_ASSERT_EQUAL(size_t(4), i.GetSize());

    i.GetValue((uint8_t*)&value0);
    i.Next();

    i.GetValue((uint8_t*)&value1);
    i.Next();

    i.GetValue((uint8_t*)&value2);
    i.Next();

    // Note: this is not a true test of randomness, we just want to make sure
    //       it's generating something reasonable
    CPPUNIT_ASSERT(value0 != value1);
    CPPUNIT_ASSERT(value1 != value2);

    // should repeat
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL((int)value0, (int)value);

    i.SetCursor(1);
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value1, value);

    i.SetCursor(0);
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value0, value);

    // test different seed
    seed ^= 0x98765432;
    RandomModifier<uint32_t> j(0xffffffff, 3, seed);

    j.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT(value != value0);

    // test with mask
    RandomModifier<uint32_t> k(0xabcdabcd, 100, seed);

    value0 = 0;
    k.GetValue((uint8_t*)&value0);
    
    k.Next();
    k.GetValue((uint8_t*)&value1);

    k.Next();
    k.GetValue((uint8_t*)&value2);

    for (size_t num = 0; num < 97; ++num)
    {
        uint32_t value_buf = 0;
        k.Next();
        k.GetValue((uint8_t*)&value_buf);
        value = ntohl(value_buf);
        CPPUNIT_ASSERT((value & 0xabcdabcd) == value);

        value_buf = 0xffffffff;
        k.GetValue((uint8_t*)&value_buf);
        value = ntohl(value_buf);
        CPPUNIT_ASSERT((value | 0xabcdabcd) == 0xffffffff);
    }

    k.Next();
    value = 0;
    k.GetValue((uint8_t*)&value);

    CPPUNIT_ASSERT_EQUAL(int(value0), int(value));

    // test with stutter
    RandomModifier<uint32_t> l(0xffffffff, 10, seed, 1);
    
    l.GetValue((uint8_t*)&value0);
   
    l.Next();
    l.GetValue((uint8_t*)&value1);

    l.Next();
    l.GetValue((uint8_t*)&value2);

    CPPUNIT_ASSERT_EQUAL((int)value0, (int)value1);
    CPPUNIT_ASSERT(value2 != value1);

    l.SetCursor(1);
    l.GetValue((uint8_t*)&value);

    CPPUNIT_ASSERT_EQUAL((int)value, (int)value0);

    l.SetCursor(22);
    l.GetValue((uint8_t*)&value);

    CPPUNIT_ASSERT_EQUAL((int)value, (int)value2);

    // verify swapping
    l.GetValue((uint8_t*)&value1, true);
    CPPUNIT_ASSERT_EQUAL(value & 0xff, (value1 >> 24) & 0xff);
    CPPUNIT_ASSERT_EQUAL(value & 0xff00, (value1 >> 8) & 0xff00);
    CPPUNIT_ASSERT_EQUAL(value & 0xff0000, (value1 << 8) & 0xff0000);
    CPPUNIT_ASSERT_EQUAL(value & 0xff000000, (value1 << 24) & 0xff000000);
}

///////////////////////////////////////////////////////////////////////////////

void TestRandomModifiers::testUint48Mod()
{
    uint32_t seed = 0x12345678;

    RandomModifier<uint48_t> i(uint48_t(0xffff, 0xffffffff), 3, seed);

    uint48_t value0;
    uint48_t value1;
    uint48_t value2;
    uint48_t value;
    
    CPPUNIT_ASSERT_EQUAL(size_t(6), i.GetSize());

    i.GetValue((uint8_t*)&value0);
    i.Next();

    i.GetValue((uint8_t*)&value1);
    i.Next();

    i.GetValue((uint8_t*)&value2);
    i.Next();

    // Note: this is not a true test of randomness, we just want to make sure
    //       it's generating something reasonable
    CPPUNIT_ASSERT(value0 != value1);
    CPPUNIT_ASSERT(value1 != value2);

    // should repeat
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value0, value);

    i.SetCursor(1);
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value1, value);

    i.SetCursor(0);
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value0, value);

    // test different seed
    seed ^= 0x98765432;
    RandomModifier<uint48_t> j(uint48_t(0xffff, 0xffffffff), 3, seed);

    j.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT(value != value0);

    // test with mask (palindromic to avoid endian issues)
    RandomModifier<uint48_t> k(uint48_t(0xabab, 0xcdefefcd), 100, seed);

    value0 = uint48_t(0);
    k.GetValue((uint8_t*)&value0);
    
    k.Next();
    k.GetValue((uint8_t*)&value1);

    k.Next();
    k.GetValue((uint8_t*)&value2);

    for (size_t num = 0; num < 97; ++num)
    {
        k.Next();
        value = uint48_t(0); 
        k.GetValue((uint8_t*)&value);
        CPPUNIT_ASSERT((value & uint48_t(0xabab, 0xcdefefcd)) == value);

        value = uint48_t(0xffff, 0xffffffff);
        k.GetValue((uint8_t*)&value);
        CPPUNIT_ASSERT((value | uint48_t(0xabab, 0xcdefefcd)) == uint48_t(0xffff, 0xffffffff));
    }

    k.Next();
    value = uint48_t(0);
    k.GetValue((uint8_t*)&value);

    CPPUNIT_ASSERT_EQUAL(value0, value);

    // test with stutter
    RandomModifier<uint48_t> l(uint48_t(0xffff, 0xffffffff), 10, seed, 1);
    
    l.GetValue((uint8_t*)&value0);
   
    l.Next();
    l.GetValue((uint8_t*)&value1);

    l.Next();
    l.GetValue((uint8_t*)&value2);

    CPPUNIT_ASSERT_EQUAL(value0, value1);
    CPPUNIT_ASSERT(value2 != value1);

    l.SetCursor(1);
    l.GetValue((uint8_t*)&value);

    CPPUNIT_ASSERT_EQUAL(value, value0);

    l.SetCursor(22);
    l.GetValue((uint8_t*)&value);

    CPPUNIT_ASSERT_EQUAL(value, value2);

    // verify swapping 
    uint8_t val[6], sval[6];
    l.GetValue(&val[0]);
    l.GetValue(&sval[0], true);
    CPPUNIT_ASSERT_EQUAL((int)val[0], (int)sval[5]);
    CPPUNIT_ASSERT_EQUAL((int)val[1], (int)sval[4]);
    CPPUNIT_ASSERT_EQUAL((int)val[2], (int)sval[3]);
    CPPUNIT_ASSERT_EQUAL((int)val[3], (int)sval[2]);
    CPPUNIT_ASSERT_EQUAL((int)val[4], (int)sval[1]);
    CPPUNIT_ASSERT_EQUAL((int)val[5], (int)sval[0]);
}

///////////////////////////////////////////////////////////////////////////////

void TestRandomModifiers::testUint64Mod()
{
    uint32_t seed = 0x12345678;

    RandomModifier<uint64_t> i(0xffffffffffffffffull, 3, seed);

    uint64_t value0 = 0;
    uint64_t value1 = 0;
    uint64_t value2 = 0;
    uint64_t value  = 0;
    
    CPPUNIT_ASSERT_EQUAL(size_t(8), i.GetSize());

    i.GetValue((uint8_t*)&value0);
    i.Next();

    i.GetValue((uint8_t*)&value1);
    i.Next();

    i.GetValue((uint8_t*)&value2);
    i.Next();

    // Note: this is not a true test of randomness, we just want to make sure
    //       it's generating something reasonable
    CPPUNIT_ASSERT(value0 != value1);
    CPPUNIT_ASSERT(value1 != value2);

    // should repeat
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value0, value);

    i.SetCursor(1);
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value1, value);

    i.SetCursor(0);
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value0, value);

    // test different seed
    seed ^= 0x98765432;
    RandomModifier<uint64_t> j(0xffffffffffffffffull, 3, seed);

    j.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT(value != value0);

    // test with mask (palindromic to avoid endian issues)
    RandomModifier<uint64_t> k(0xabcdef1212efcdabull, 100, seed);

    value0 = 0;
    k.GetValue((uint8_t*)&value0);
    
    k.Next();
    k.GetValue((uint8_t*)&value1);

    k.Next();
    k.GetValue((uint8_t*)&value2);

    for (size_t num = 0; num < 97; ++num)
    {
        k.Next();
        value = 0;
        k.GetValue((uint8_t*)&value);
        CPPUNIT_ASSERT((value & 0xabcdef1212efcdabull) == value);

        value = 0xffffffffffffffffull;
        k.GetValue((uint8_t*)&value);
        CPPUNIT_ASSERT((value | 0xabcdef1212efcdabull) == 0xffffffffffffffffull);
    }

    k.Next();
    value = 0;
    k.GetValue((uint8_t*)&value);

    CPPUNIT_ASSERT_EQUAL(value0, value);

    // test with stutter
    RandomModifier<uint64_t> l(0xffffffffffffffffull, 10, seed, 1);
    
    l.GetValue((uint8_t*)&value0);
   
    l.Next();
    l.GetValue((uint8_t*)&value1);

    l.Next();
    l.GetValue((uint8_t*)&value2);

    CPPUNIT_ASSERT_EQUAL(value0, value1);
    CPPUNIT_ASSERT(value2 != value1);

    l.SetCursor(1);
    l.GetValue((uint8_t*)&value);

    CPPUNIT_ASSERT_EQUAL(value, value0);

    l.SetCursor(22);
    l.GetValue((uint8_t*)&value);

    CPPUNIT_ASSERT_EQUAL(value, value2);

    // verify swapping 
    uint8_t val[8], sval[8];
    l.GetValue(&val[0]);
    l.GetValue(&sval[0], true);
    CPPUNIT_ASSERT_EQUAL((int)val[0], (int)sval[7]);
    CPPUNIT_ASSERT_EQUAL((int)val[1], (int)sval[6]);
    CPPUNIT_ASSERT_EQUAL((int)val[2], (int)sval[5]);
    CPPUNIT_ASSERT_EQUAL((int)val[3], (int)sval[4]);
    CPPUNIT_ASSERT_EQUAL((int)val[4], (int)sval[3]);
    CPPUNIT_ASSERT_EQUAL((int)val[5], (int)sval[2]);
    CPPUNIT_ASSERT_EQUAL((int)val[6], (int)sval[1]);
    CPPUNIT_ASSERT_EQUAL((int)val[7], (int)sval[0]);
}

///////////////////////////////////////////////////////////////////////////////

void TestRandomModifiers::testUint128Mod()
{
    uint32_t seed = 0x12345678;

    RandomModifier<uint128_t> i(uint128_t(0xffffffffffffffffull, 0xffffffffffffffffull), 3, seed);

    uint128_t value0;
    uint128_t value1;
    uint128_t value2;
    uint128_t value;
    
    CPPUNIT_ASSERT_EQUAL(size_t(16), i.GetSize());

    i.GetValue((uint8_t*)&value0);
    i.Next();

    i.GetValue((uint8_t*)&value1);
    i.Next();

    i.GetValue((uint8_t*)&value2);
    i.Next();

    // Note: this is not a true test of randomness, we just want to make sure
    //       it's generating something reasonable
    CPPUNIT_ASSERT(value0 != value1);
    CPPUNIT_ASSERT(value1 != value2);

    // should repeat
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value0, value);

    i.SetCursor(1);
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value1, value);

    i.SetCursor(0);
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value0, value);

    // test different seed
    seed ^= 0x98765432;
    RandomModifier<uint128_t> j(uint128_t(0xffffffffffffffffull, 0xffffffffffffffffull), 3, seed);

    j.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT(value != value0);

    // test with mask (palindromic to avoid endian issues)
    uint128_t mask(0xabcdef1212efcdabull, 0xabcdef1212efcdabull);
    RandomModifier<uint128_t> k(mask, 100, seed);

    value0 = uint128_t(0);
    k.GetValue((uint8_t*)&value0);
    
    k.Next();
    k.GetValue((uint8_t*)&value1);

    k.Next();
    k.GetValue((uint8_t*)&value2);

    for (size_t num = 0; num < 97; ++num)
    {
        k.Next();
        value = uint128_t(0);
        k.GetValue((uint8_t*)&value);
        CPPUNIT_ASSERT((value & mask) == value);

        value = uint128_t(0xffffffffffffffffull, 0xffffffffffffffffull);
        k.GetValue((uint8_t*)&value);
        CPPUNIT_ASSERT((value | mask) == uint128_t(0xffffffffffffffffull, 0xffffffffffffffffull));
    }

    k.Next();
    value = uint128_t(0);
    k.GetValue((uint8_t*)&value);

    CPPUNIT_ASSERT_EQUAL(value0, value);

    // test with stutter
    RandomModifier<uint128_t> l(uint128_t(0xffffffffffffffffull, 0xffffffffffffffffull), 10, seed, 1);
    
    l.GetValue((uint8_t*)&value0);
   
    l.Next();
    l.GetValue((uint8_t*)&value1);

    l.Next();
    l.GetValue((uint8_t*)&value2);

    CPPUNIT_ASSERT_EQUAL(value0, value1);
    CPPUNIT_ASSERT(value2 != value1);

    l.SetCursor(1);
    l.GetValue((uint8_t*)&value);

    CPPUNIT_ASSERT_EQUAL(value, value0);

    l.SetCursor(22);
    l.GetValue((uint8_t*)&value);

    CPPUNIT_ASSERT_EQUAL(value, value2);

    // verify swapping 
    uint8_t val[16], sval[16];
    l.GetValue(&val[0]);
    l.GetValue(&sval[0], true);
    CPPUNIT_ASSERT_EQUAL((int)val[0], (int)sval[15]);
    CPPUNIT_ASSERT_EQUAL((int)val[1], (int)sval[14]);
    CPPUNIT_ASSERT_EQUAL((int)val[2], (int)sval[13]);
    CPPUNIT_ASSERT_EQUAL((int)val[3], (int)sval[12]);
    CPPUNIT_ASSERT_EQUAL((int)val[4], (int)sval[11]);
    CPPUNIT_ASSERT_EQUAL((int)val[5], (int)sval[10]);
    CPPUNIT_ASSERT_EQUAL((int)val[6], (int)sval[9]);
    CPPUNIT_ASSERT_EQUAL((int)val[7], (int)sval[8]);
    CPPUNIT_ASSERT_EQUAL((int)val[8], (int)sval[7]);
    CPPUNIT_ASSERT_EQUAL((int)val[9], (int)sval[6]);
    CPPUNIT_ASSERT_EQUAL((int)val[10], (int)sval[5]);
    CPPUNIT_ASSERT_EQUAL((int)val[11], (int)sval[4]);
    CPPUNIT_ASSERT_EQUAL((int)val[12], (int)sval[3]);
    CPPUNIT_ASSERT_EQUAL((int)val[13], (int)sval[2]);
    CPPUNIT_ASSERT_EQUAL((int)val[14], (int)sval[1]);
    CPPUNIT_ASSERT_EQUAL((int)val[15], (int)sval[0]);
}

///////////////////////////////////////////////////////////////////////////////

void TestRandomModifiers::testChain()
{
    uint32_t seed = 0x12345678;

    RandomModifier<uint32_t> i(0xffffffff, 3, seed);
    MockModifier mock;

    i.SetChild(&mock);

              CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    i.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    i.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    i.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    i.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    i.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    i.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(2), mock.GetCursor());

    i.SetCursor(0); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    i.SetCursor(3); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    i.SetCursor(29); CPPUNIT_ASSERT_EQUAL(int32_t(9), mock.GetCursor());
    i.SetCursor(30); CPPUNIT_ASSERT_EQUAL(int32_t(10), mock.GetCursor());
    i.SetCursor(31); CPPUNIT_ASSERT_EQUAL(int32_t(10), mock.GetCursor());
    i.SetCursor(32); CPPUNIT_ASSERT_EQUAL(int32_t(10), mock.GetCursor());

    // try with a single-count modifier
    RandomModifier<uint32_t> j(0xffffffff, 1, seed);
    j.SetChild(&mock);
    mock.SetCursor(0);

    j.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    j.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(2), mock.GetCursor());
    j.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(3), mock.GetCursor());

    j.SetCursor(0); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    j.SetCursor(3); CPPUNIT_ASSERT_EQUAL(int32_t(3), mock.GetCursor());
    j.SetCursor(29); CPPUNIT_ASSERT_EQUAL(int32_t(29), mock.GetCursor());
}

///////////////////////////////////////////////////////////////////////////////

void TestRandomModifiers::testZeroCount()
{
    // count of zero should probably be invalid at the BLL but 
    // we don't want to divide by zero and crash
    uint32_t seed = 0x12345678;
    uint32_t value0 = 0;
    uint32_t value  = 0;

    RandomModifier<uint32_t> i(0xffffffff, 0, seed);

    CPPUNIT_ASSERT_EQUAL(size_t(4), i.GetSize());

    i.GetValue((uint8_t*)&value0);
    i.Next();

    // should repeat
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value0, value);

    // forever
    i.Next();
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value0, value);

    // even with setcursor
    i.SetCursor(0);
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value0, value);

    i.SetCursor(123);
    i.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value0, value);
    
    // try with stuttering
    RandomModifier<uint32_t> j(0xffffffff, 0, seed);
    CPPUNIT_ASSERT_EQUAL(size_t(4), j.GetSize());

    j.GetValue((uint8_t*)&value0);
    j.Next();

    // should repeat
    j.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value0, value);

    // forever
    j.Next();
    j.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value0, value);

    // even with setcursor
    j.SetCursor(0);
    j.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value0, value);

    j.SetCursor(123);
    j.GetValue((uint8_t*)&value);
    CPPUNIT_ASSERT_EQUAL(value0, value);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestRandomModifiers);
