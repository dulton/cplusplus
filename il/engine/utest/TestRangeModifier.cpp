#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "Uint48.h"
#include "Uint128.h"
#include "MockModifier.h"
#include "RangeModifier.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class TestRangeModifier : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestRangeModifier);
    CPPUNIT_TEST(testUint8Mod);
    CPPUNIT_TEST(testUint16Mod);
    CPPUNIT_TEST(testUint32Mod);
    CPPUNIT_TEST(testUint48Mod);
    CPPUNIT_TEST(testUint64Mod);
    CPPUNIT_TEST(testUint128Mod);
    CPPUNIT_TEST(testChain);
    CPPUNIT_TEST_SUITE_END();

public:
    TestRangeModifier()
    {
    }

    ~TestRangeModifier()
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

private:
};

///////////////////////////////////////////////////////////////////////////////

void TestRangeModifier::testUint8Mod()
{
    // 12, 13, 14, 15, 12, etc.
    RangeModifier<uint8_t> i(12, 1, 0xff, 0, 4);

    uint8_t value = 0;
    
    CPPUNIT_ASSERT_EQUAL(size_t(1), i.GetSize());

    i.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint8_t(12), value);

    // verify swapping does nothing here
    i.GetValue(&value, true);
    CPPUNIT_ASSERT_EQUAL(uint8_t(12), value);

    i.Next();
    i.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint8_t(13), value);

    i.Next();
    i.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint8_t(14), value);

    i.Next();
    i.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint8_t(15), value);

    i.Next();
    i.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint8_t(12), value);

    i.SetCursor(0);
    i.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint8_t(12), value);

    i.SetCursor(5);
    i.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint8_t(13), value);

    // 253, 255, 1, 3, 5, 253, etc.
    RangeModifier<uint8_t> j(253, 2, 0xff, 0, 5);

    value = 0;
    
    j.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint8_t(253), value);

    j.Next();
    j.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint8_t(255), value);

    j.Next();
    j.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint8_t(1), value);

    j.Next();
    j.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint8_t(3), value);

    j.Next();
    j.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint8_t(5), value);

    j.SetCursor(0);
    j.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint8_t(253), value);

    j.SetCursor(6);
    j.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint8_t(255), value);

    // 0x?1, 0x?1, 0x?3, 0x?3, 0x?1, etc. 
    RangeModifier<uint8_t> k(1, 2, 0x0f, 1, 2);
    value = 0xab;

    k.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0xa1), uint32_t(value));
    
    k.Next();
    k.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint8_t(0xa1), value);

    k.Next();
    k.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0xa3), uint32_t(value));

    k.Next();
    k.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint8_t(0xa3), value);

    value = 0xbc;
    k.Next();
    k.GetValue(&value);
    CPPUNIT_ASSERT_EQUAL(uint8_t(0xb1), value);
}

///////////////////////////////////////////////////////////////////////////////

void TestRangeModifier::testUint16Mod()
{
    // 0x1200, 0x2301, 0x3402, 0x4503, 0x1200, etc.
    RangeModifier<uint16_t> i(0x1200, 0x1101, 0xffff, 0, 4);
    CPPUNIT_ASSERT_EQUAL(size_t(2), i.GetSize());

    uint8_t value[] = {0, 0};
    
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x12\x00", (const char *)value, 2) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x23\x01", (const char *)value, 2) == 0);

    // verify swapping
    i.GetValue(&value[0], true);
    CPPUNIT_ASSERT(strncmp("\x01\x23", (const char *)value, 2) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x34\x02", (const char *)value, 2) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x45\x03", (const char *)value, 2) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x12\x00", (const char *)value, 2) == 0);

    i.SetCursor(0);
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x12\x00", (const char *)value, 2) == 0);

    i.SetCursor(5);
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x23\x01", (const char *)value, 2) == 0);

    // 0xff00, 0x80, 0x180, 0x280, 0x380, 0xff00, etc.
    RangeModifier<uint16_t> j(0xff80, 0x100, 0xffff, 0, 5);

    memset(&value, 0, sizeof(value));
    
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xff\x80", (const char *)value, 2) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x80", (const char *)value, 2) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x01\x80", (const char *)value, 2) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x02\x80", (const char *)value, 2) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x03\x80", (const char *)value, 2) == 0);

    j.SetCursor(0);
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xff\x80", (const char *)value, 2) == 0);

    j.SetCursor(6);
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x80", (const char *)value, 2) == 0);

    // 0x??11, 0x??11, 0x??22, 0x??22, 0x??11, etc.
    RangeModifier<uint16_t> k(0x11, 0x11, 0x00ff, 1, 2);

    memset(&value, 0xaa, sizeof(value));
    
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xaa\x11", (const char *)value, 2) == 0);

    k.Next();
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xaa\x11", (const char *)value, 2) == 0);

    k.Next();
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xaa\x22", (const char *)value, 2) == 0);

    k.Next();
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xaa\x22", (const char *)value, 2) == 0);

    memset(&value, 0xbb, sizeof(value));

    k.Next();
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xbb\x11", (const char *)value, 2) == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestRangeModifier::testUint32Mod()
{
    // 0x20000000, 0x20000000, 0x21000000, 0x21000000, 0x20000000, etc.
    RangeModifier<uint32_t> i(0x20000000, 0x1000000, 0xffffffff, 1 /*stutter */, 2);
    CPPUNIT_ASSERT_EQUAL(size_t(4), i.GetSize());

    uint8_t value[] = {0, 0, 0, 0};
    
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x20\x00\x00\x00", (const char *)value, 4) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x20\x00\x00\x00", (const char *)value, 4) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x21\x00\x00\x00", (const char *)value, 4) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x21\x00\x00\x00", (const char *)value, 4) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x20\x00\x00\x00", (const char *)value, 4) == 0);

    i.SetCursor(0);
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x20\x00\x00\x00", (const char *)value, 4) == 0);

    i.SetCursor(5);
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x20\x00\x00\x00", (const char *)value, 4) == 0);

    // 0xd0000000, 0xf0000001, 0x10000002, 0x30000003, 0x50000004, 0xd0000000, etc.
    RangeModifier<uint32_t> j(0xd0000000ul, 0x20000001, 0xffffffff, 0, 5);

    memset(&value, 0, sizeof(value));
    
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xd0\x00\x00\x00", (const char *)value, 4) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xf0\x00\x00\x01", (const char *)value, 4) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x10\x00\x00\x02", (const char *)value, 4) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x30\x00\x00\x03", (const char *)value, 4) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x50\x00\x00\x04", (const char *)value, 4) == 0);

    // verify swapping
    j.GetValue(&value[0], true);
    CPPUNIT_ASSERT(strncmp("\x04\x00\x00\x50", (const char *)value, 4) == 0);

    j.SetCursor(0);
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xd0\x00\x00\x00", (const char *)value, 4) == 0);


    j.SetCursor(6);
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xf0\x00\x00\x01", (const char *)value, 4) == 0);

    // 0x????1111, 0x????1111, 0x????2222, 0x????2222, 0x????1111, etc.
    RangeModifier<uint32_t> k(0x1111, 0x1111, 0xffff, 1, 2);

    memset(&value, 0xaa, sizeof(value));
    
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xaa\xaa\x11\x11", (const char *)value, 4) == 0);

    k.Next();
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xaa\xaa\x11\x11", (const char *)value, 4) == 0);

    k.Next();
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xaa\xaa\x22\x22", (const char *)value, 4) == 0);

    k.Next();
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xaa\xaa\x22\x22", (const char *)value, 4) == 0);

    memset(&value, 0xbb, sizeof(value));

    k.Next();
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xbb\xbb\x11\x11", (const char *)value, 4) == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestRangeModifier::testUint48Mod()
{
    // 0x100000000000,
    // 0x180000000000,
    // 0x200000000000,
    // 0x280000000000,
    // 0x100000000000, etc.
    RangeModifier<uint48_t> i(uint48_t(0x1000, 0), uint48_t(0x0800, 0), uint48_t(0xffff, 0xffffffff), 0, 4);
    CPPUNIT_ASSERT_EQUAL(size_t(6), i.GetSize());

    uint8_t value[6] = {};
    
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x10\x00\x00\x00\x00\x00", (const char *)value, 6) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x18\x00\x00\x00\x00\x00", (const char *)value, 6) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x20\x00\x00\x00\x00\x00", (const char *)value, 6) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x28\x00\x00\x00\x00\x00", (const char *)value, 6) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x10\x00\x00\x00\x00\x00", (const char *)value, 6) == 0);

    i.SetCursor(0);
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x10\x00\x00\x00\x00\x00", (const char *)value, 6) == 0);

    i.SetCursor(5);
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x18\x00\x00\x00\x00\x00", (const char *)value, 6) == 0);

    // 0x600000000, 
    // 0x640000000, 
    // 0x680000000,
    // 0x000000000,
    // 0x040000000,
    // 0x600000000, etc.
    RangeModifier<uint48_t> j(uint48_t(0x6, 0), uint48_t(0, 0x40000000), uint48_t(0xff, 0xffffffff), 0, 5);

    memset(&value, 0, sizeof(value));
    
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x06\x00\x00\x00\x00", (const char *)value, 6) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x06\x40\x00\x00\x00", (const char *)value, 6) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x06\x80\x00\x00\x00", (const char *)value, 6) == 0);

    // verify swapping
    j.GetValue(&value[0], true);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x00\x80\x06\x00", (const char *)value, 6) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x00\x00\x00\x00", (const char *)value, 6) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x40\x00\x00\x00", (const char *)value, 6) == 0);

    j.SetCursor(0);
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x06\x00\x00\x00\x00", (const char *)value, 6) == 0);

    j.SetCursor(6);
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x06\x40\x00\x00\x00", (const char *)value, 6) == 0);

    // 0x?1?1?1?1?,
    // 0x?1?1?1?1?,
    // 0x?2?2?2?2?,
    // 0x?2?2?2?2?,
    // 0x?1?1?1?1?, etc.
    RangeModifier<uint48_t> k(uint48_t(0x0101, 0x01010101), uint48_t(0x0101, 0x01010101), uint48_t(0x0f0f, 0x0f0f0f0f), 1, 2);

    memset(&value, 0xaa, sizeof(value));
    
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xa1\xa1\xa1\xa1\xa1\xa1", (const char *)value, 6) == 0);

    k.Next();
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xa1\xa1\xa1\xa1\xa1\xa1", (const char *)value, 6) == 0);

    k.Next();
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xa2\xa2\xa2\xa2\xa2\xa2", (const char *)value, 6) == 0);

    k.Next();
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xa2\xa2\xa2\xa2\xa2\xa2", (const char *)value, 6) == 0);

    memset(&value, 0xbb, sizeof(value));

    k.Next();
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xb1\xb1\xb1\xb1\xb1\xb1", (const char *)value, 6) == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestRangeModifier::testUint64Mod()
{
    // 0x1000000000000000,
    // 0x1800000000000000,
    // 0x2000000000000000,
    // 0x2800000000000000,
    // 0x1000000000000000, etc.
    RangeModifier<uint64_t> i(0x1000000000000000ll, 0x800000000000000ll, 0xffffffffffffffffull, 0, 4);
    CPPUNIT_ASSERT_EQUAL(size_t(8), i.GetSize());

    uint8_t value[8] = {};
    
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x10\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 8) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x18\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 8) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x20\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 8) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x28\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 8) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x10\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 8) == 0);

    i.SetCursor(0);
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x10\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 8) == 0);

    i.SetCursor(5);
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x18\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 8) == 0);

    // 0x60000000000, 
    // 0x64000000000, 
    // 0x68000000000,
    // 0x00000000000,
    // 0x04000000000,
    // 0x60000000000, etc.
    RangeModifier<uint64_t> j(0x60000000000ull, 0x4000000000ull, 0xffffffffffull, 0, 5);

    memset(&value, 0, sizeof(value));
    
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x60\x00\x00\x00\x00\x00", (const char *)value, 8) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x64\x00\x00\x00\x00\x00", (const char *)value, 8) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x68\x00\x00\x00\x00\x00", (const char *)value, 8) == 0);

    // verify swapping
    j.GetValue(&value[0], true);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x00\x00\x00\x68\x00\x00", (const char *)value, 8) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 8) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x04\x00\x00\x00\x00\x00", (const char *)value, 8) == 0);

    j.SetCursor(0);
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x60\x00\x00\x00\x00\x00", (const char *)value, 8) == 0);

    j.SetCursor(6);
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x64\x00\x00\x00\x00\x00", (const char *)value, 8) == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestRangeModifier::testUint128Mod()
{
    // 0x1000000000000000 0x1000000000000000,
    // 0x1800000000000000 0x1800000000000000,
    // 0x2000000000000000 0x2000000000000000,
    // 0x2800000000000000 0x2800000000000000,
    // 0x1000000000000000 0x1000000000000000, etc.
    RangeModifier<uint128_t> i(uint128_t(0x1000000000000000ull, 0x1000000000000000ull), uint128_t(0x800000000000000ll, 0x800000000000000ll), uint128_t(0xffffffffffffffffull, 0xffffffffffffffffull), 0, 4);
    CPPUNIT_ASSERT_EQUAL(size_t(16), i.GetSize());

    uint8_t value[16] = {};
    
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x10\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 16) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x18\x00\x00\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 16) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x20\x00\x00\x00\x00\x00\x00\x00\x20\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 16) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x28\x00\x00\x00\x00\x00\x00\x00\x28\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 16) == 0);

    i.Next();
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x10\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 16) == 0);

    i.SetCursor(0);
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x10\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 16) == 0);

    i.SetCursor(5);
    i.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x18\x00\x00\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 8) == 0);

    // 0x60000000000 0x8000000000000000ull, 
    // 0x64000000001 0x0000000000000000ull, 
    // 0x68000000001 0x8000000000000000ull,
    // 0x00000000000 0x0000000000000000ull,
    // 0x04000000000 0x8000000000000000ull,
    // 0x60000000000 0x8000000000000000ull, etc.
    RangeModifier<uint128_t> j(uint128_t(0x60000000000ull, 0x8000000000000000ull), uint128_t(0x4000000000ull, 0x8000000000000000ull), uint128_t(0xffffffffffull, 0xf000000000000000ull), 0, 5);

    memset(&value, 0, sizeof(value));
    
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x60\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 16) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 16) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x68\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 16) == 0);

    // verify swapping
    j.GetValue(&value[0], true);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x68\x00\x00", (const char *)value, 16) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 16) == 0);

    j.Next();
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x04\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 16) == 0);

    j.SetCursor(0);
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x60\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 16) == 0);

    j.SetCursor(6);
    j.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\x00\x00\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", (const char *)value, 16) == 0);

    // 0x??????????1 0x???????????????1ull, 
    // 0x??????????1 0x???????????????1ull, 
    // 0x??????????2 0x???????????????2ull, 
    // 0x??????????2 0x???????????????2ull, 
    // 0x??????????1 0x???????????????1ull, etc.
    RangeModifier<uint128_t> k(uint128_t(1, 1), uint128_t(1, 1), uint128_t(0xf, 0xf), 1, 2);

    memset(&value, 0xaa, sizeof(value));
    
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xa1\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xa1", (const char *)value, 16) == 0);

    k.Next();
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xa1\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xa1", (const char *)value, 16) == 0);

    k.Next();
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xa2\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xa2", (const char *)value, 16) == 0);

    k.Next();
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xa2\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xa2", (const char *)value, 16) == 0);

    memset(&value, 0xbb, sizeof(value));

    k.Next();
    k.GetValue(&value[0]);
    CPPUNIT_ASSERT(strncmp("\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xb1\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xb1", (const char *)value, 16) == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestRangeModifier::testChain()
{
    RangeModifier<uint32_t> i(0, 1, 0xffffffff, 0, 3);
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
    RangeModifier<uint32_t> j(0, 1, 0xffffffff, 0, 1);
    j.SetChild(&mock);
    mock.SetCursor(0);

    j.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    j.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(2), mock.GetCursor());
    j.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(3), mock.GetCursor());

    j.SetCursor(0); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    j.SetCursor(3); CPPUNIT_ASSERT_EQUAL(int32_t(3), mock.GetCursor());
    j.SetCursor(29); CPPUNIT_ASSERT_EQUAL(int32_t(29), mock.GetCursor());

    // try with suttering
    RangeModifier<uint32_t> k(0, 1, 0xffffffff, 1, 3);
    k.SetChild(&mock);
    mock.SetCursor(0);

              CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    k.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    k.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    k.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    k.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    k.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    k.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    k.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    k.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    k.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    k.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    k.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    k.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(2), mock.GetCursor());

    k.SetCursor(0); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    k.SetCursor(6); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    k.SetCursor(59); CPPUNIT_ASSERT_EQUAL(int32_t(9), mock.GetCursor());
    k.SetCursor(60); CPPUNIT_ASSERT_EQUAL(int32_t(10), mock.GetCursor());
    k.SetCursor(61); CPPUNIT_ASSERT_EQUAL(int32_t(10), mock.GetCursor());
    k.SetCursor(62); CPPUNIT_ASSERT_EQUAL(int32_t(10), mock.GetCursor());
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestRangeModifier);
