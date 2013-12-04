#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "MockModifier.h"
#include "TableModifier.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class TestTableModifier : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestTableModifier);
    CPPUNIT_TEST(testBasic);
    CPPUNIT_TEST(testSwap);
    CPPUNIT_TEST(testStutter);
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST(testSingle);
    CPPUNIT_TEST(testChain);
    CPPUNIT_TEST_SUITE_END();

public:
    TestTableModifier()
    {
    }

    ~TestTableModifier()
    {
    }

    void setUp() 
    { 
    }

    void tearDown() 
    { 
    }
    
protected:
    void testBasic();
    void testSwap();
    void testStutter();
    void testEmpty();
    void testSingle();
    void testChain();

private:
};

///////////////////////////////////////////////////////////////////////////////

void TestTableModifier::testBasic()
{
    TableModifier::Table_t table;
    table.resize(4);
    table[0].resize(1, 0x01);
    table[1].resize(2, 0x02);
    table[2].resize(3, 0x03);
    table[3].resize(4, 0x04);

    // 01, 02 02, 03 03 03, 04 04 04 04
    TableModifier t(table, 0);

    uint8_t value[4] = {};
    
    CPPUNIT_ASSERT_EQUAL(size_t(1), t.GetSize());

    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value[0]));

    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(2), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(2), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(2), int(value[1]));

    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(3), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[2]));

    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(4), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(4), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(4), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(4), int(value[2]));
    CPPUNIT_ASSERT_EQUAL(int(4), int(value[3]));

    // repeats
    
    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(1), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value[0]));


    // set cursor
    
    t.SetCursor(0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value[0]));

    t.SetCursor(6);
    CPPUNIT_ASSERT_EQUAL(size_t(3), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[2]));
}

///////////////////////////////////////////////////////////////////////////////

void TestTableModifier::testSwap()
{
    TableModifier::Table_t table;
    table.resize(4);
    table[0].resize(1, 0x01);

    table[1].resize(2, 0x02);
    table[1][1] = 0x20;

    table[2].resize(3, 0x03);
    table[2][1] = 0x30;
    table[2][2] = 0x33;

    table[3].resize(4, 0x04);
    table[3][1] = 0x40;
    table[3][2] = 0x44;
    table[3][3] = 0x88;

    // 01, 02 20, 03 30 33, 04 40 44 88
    TableModifier t(table, 0);

    uint8_t value[4] = {};
    
    CPPUNIT_ASSERT_EQUAL(size_t(1), t.GetSize());

    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value[0]));

    // 01 swapped == 01
    t.GetValue(&value[0], true);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value[0]));

    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(2), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0x02), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(0x20), int(value[1]));

    // 02 20 swapped == 20 02
    t.GetValue(&value[0], true);
    CPPUNIT_ASSERT_EQUAL(int(0x20), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(0x02), int(value[1]));


    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(3), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0x03), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(0x30), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(0x33), int(value[2]));

    // 03 30 33 swapped == 33 30 03
    t.GetValue(&value[0], true);
    CPPUNIT_ASSERT_EQUAL(int(0x33), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(0x30), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(0x03), int(value[2]));

    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(4), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0x04), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(0x40), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(0x44), int(value[2]));
    CPPUNIT_ASSERT_EQUAL(int(0x88), int(value[3]));

    // 04 40 44 88 swapped == 88 44 40 04
    t.GetValue(&value[0], true);
    CPPUNIT_ASSERT_EQUAL(int(0x88), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(0x44), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(0x40), int(value[2]));
    CPPUNIT_ASSERT_EQUAL(int(0x04), int(value[3]));
}

///////////////////////////////////////////////////////////////////////////////

void TestTableModifier::testStutter()
{
    TableModifier::Table_t table;
    table.resize(3);
    table[0].resize(1, 0x01);
    table[1].resize(2, 0x02);
    table[2].resize(3, 0x03);

    // 01, 01, 01, 02 02, 02 02, 02 02, 03 03 03, 03 03 03, 03 03 03
    TableModifier t(table, 2);

    uint8_t value[4] = {};
    
    CPPUNIT_ASSERT_EQUAL(size_t(1), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value[0]));
    
    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(1), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value[0]));

    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(1), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value[0]));

    // done with first stutter
    
    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(2), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(2), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(2), int(value[1]));

    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(2), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(2), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(2), int(value[1]));

    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(2), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(2), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(2), int(value[1]));

    // done with second stutter
    
    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(3), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[2]));

    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(3), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[2]));

    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(3), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[2]));

    // done with third stutter, repeats
    
    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(1), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value[0]));

    // set cursor
    
    t.SetCursor(0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value[0]));

    t.SetCursor(84); // at 81 it repeats the sequence, 84 should be the second value
    CPPUNIT_ASSERT_EQUAL(size_t(2), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(2), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(2), int(value[1]));
}

///////////////////////////////////////////////////////////////////////////////

void TestTableModifier::testEmpty()
{
    TableModifier::Table_t table;

    // nothing in the table
    TableModifier t(table, 0);

    uint8_t value[4] = {1, 2, 3, 4};
    
    CPPUNIT_ASSERT_EQUAL(size_t(0), t.GetSize());

    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(2), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[2]));
    CPPUNIT_ASSERT_EQUAL(int(4), int(value[3]));

    // repeats
    
    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(0), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(2), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[2]));
    CPPUNIT_ASSERT_EQUAL(int(4), int(value[3]));

    // set cursor
    
    t.SetCursor(0);
    CPPUNIT_ASSERT_EQUAL(size_t(0), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(2), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[2]));
    CPPUNIT_ASSERT_EQUAL(int(4), int(value[3]));

    t.SetCursor(6);
    CPPUNIT_ASSERT_EQUAL(size_t(0), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(2), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[2]));
    CPPUNIT_ASSERT_EQUAL(int(4), int(value[3]));
}

///////////////////////////////////////////////////////////////////////////////

void TestTableModifier::testSingle()
{
    TableModifier::Table_t table;
    table.resize(1);
    table[0].resize(3, 0x33);

    // just one entry in the table
    TableModifier t(table, 0);

    uint8_t value[3] = {};
    
    CPPUNIT_ASSERT_EQUAL(size_t(3), t.GetSize());

    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0x33), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(0x33), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(0x33), int(value[2]));
    memset(value, 0, sizeof(value));

    // repeats
    
    t.Next();
    CPPUNIT_ASSERT_EQUAL(size_t(3), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0x33), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(0x33), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(0x33), int(value[2]));
    memset(value, 0, sizeof(value));

    // set cursor
    
    t.SetCursor(0);
    CPPUNIT_ASSERT_EQUAL(size_t(3), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0x33), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(0x33), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(0x33), int(value[2]));
    memset(value, 0, sizeof(value));

    t.SetCursor(6);
    CPPUNIT_ASSERT_EQUAL(size_t(3), t.GetSize());
    t.GetValue(&value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0x33), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(0x33), int(value[1]));
    CPPUNIT_ASSERT_EQUAL(int(0x33), int(value[2]));
    memset(value, 0, sizeof(value));
}

///////////////////////////////////////////////////////////////////////////////

void TestTableModifier::testChain()
{
    TableModifier::Table_t table;
    MockModifier mock;

    // empty table
    TableModifier t0(table, 0);
    t0.SetChild(&mock);

               CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    t0.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    t0.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(2), mock.GetCursor());
    t0.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(3), mock.GetCursor());
    t0.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(4), mock.GetCursor());

    t0.SetCursor(0); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    t0.SetCursor(3); CPPUNIT_ASSERT_EQUAL(int32_t(3), mock.GetCursor());
    t0.SetCursor(29); CPPUNIT_ASSERT_EQUAL(int32_t(29), mock.GetCursor());

    // just one entry in the table
    table.resize(1);
    table[0].resize(3, 0x33);
    TableModifier t1(table, 0);
    t1.SetChild(&mock);
    mock.SetCursor(0);

               CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    t1.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    t1.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(2), mock.GetCursor());
    t1.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(3), mock.GetCursor());
    t1.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(4), mock.GetCursor());

    t1.SetCursor(0); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    t1.SetCursor(3); CPPUNIT_ASSERT_EQUAL(int32_t(3), mock.GetCursor());
    t1.SetCursor(29); CPPUNIT_ASSERT_EQUAL(int32_t(29), mock.GetCursor());

    // three entries
    table.resize(3);
    TableModifier t3(table, 0);
    t3.SetChild(&mock);
    mock.SetCursor(0);

               CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    t3.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    t3.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    t3.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    t3.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    t3.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    t3.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(2), mock.GetCursor());

    t3.SetCursor(0); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    t3.SetCursor(3); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    t3.SetCursor(29); CPPUNIT_ASSERT_EQUAL(int32_t(9), mock.GetCursor());
    t3.SetCursor(30); CPPUNIT_ASSERT_EQUAL(int32_t(10), mock.GetCursor());
    t3.SetCursor(31); CPPUNIT_ASSERT_EQUAL(int32_t(10), mock.GetCursor());
    t3.SetCursor(32); CPPUNIT_ASSERT_EQUAL(int32_t(10), mock.GetCursor());

    // stuttered
    TableModifier t6(table, 1);
    t6.SetChild(&mock);
    mock.SetCursor(0);

               CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    t6.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    t6.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    t6.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    t6.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    t6.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    t6.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    t6.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    t6.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    t6.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    t6.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    t6.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    t6.Next(); CPPUNIT_ASSERT_EQUAL(int32_t(2), mock.GetCursor());

    t6.SetCursor(0); CPPUNIT_ASSERT_EQUAL(int32_t(0), mock.GetCursor());
    t6.SetCursor(6); CPPUNIT_ASSERT_EQUAL(int32_t(1), mock.GetCursor());
    t6.SetCursor(59); CPPUNIT_ASSERT_EQUAL(int32_t(9), mock.GetCursor());
    t6.SetCursor(60); CPPUNIT_ASSERT_EQUAL(int32_t(10), mock.GetCursor());
    t6.SetCursor(61); CPPUNIT_ASSERT_EQUAL(int32_t(10), mock.GetCursor());
    t6.SetCursor(62); CPPUNIT_ASSERT_EQUAL(int32_t(10), mock.GetCursor());
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestTableModifier);
