#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "PlaylistConfig.h"
#include "ModifierBlock.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class TestModifierBlock : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestModifierBlock);
    CPPUNIT_TEST(testMakeRangeMod);
    CPPUNIT_TEST(testRangeModifiers);
    CPPUNIT_TEST(testTableModifiers);
    CPPUNIT_TEST(testRandomModifiers);
    CPPUNIT_TEST(testLinkModifiers);
    CPPUNIT_TEST(testEmptyBlock);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) 
    {
    }

    void tearDown(void) 
    {
    }

protected:
    void testMakeRangeMod(void);
    void testRangeModifiers(void);
    void testTableModifiers(void);
    void testRandomModifiers(void);
    void testLinkModifiers(void);
    void testEmptyBlock(void);

private:
};

///////////////////////////////////////////////////////////////////////////////

void TestModifierBlock::testMakeRangeMod(void)
{
    PlaylistConfig::RangeMod config;
    uint32_t randSeed = 0;

    std::vector<uint8_t> vec0001(2, 0x00);
    vec0001[1] = 0x01;
    std::vector<uint8_t> vecffff(2, 0xff);

    // 1 byte, 254, 2, 6, 10
    config.start.resize(1, 254);
    config.step.resize(1, 4);
    config.mask.resize(1, 0xff);
    config.recycleCount = 4;
    config.stutterCount = 0;
    config.mode = PlaylistConfig::MOD_INCR;

    {
        std::auto_ptr<AbstModifier> modPtr(ModifierBlock::MakeRangeMod(config, randSeed));  
        CPPUNIT_ASSERT(modPtr.get() != 0);
        CPPUNIT_ASSERT_EQUAL(size_t(1), modPtr->GetSize());
       
        uint8_t value = 0;
        modPtr->GetValue(&value); 

        CPPUNIT_ASSERT_EQUAL(int(254), int(value));

        modPtr->Next(); 
        modPtr->GetValue(&value); 
        CPPUNIT_ASSERT_EQUAL(int(2), int(value));

        modPtr->Next(); 
        modPtr->GetValue(&value); 
        CPPUNIT_ASSERT_EQUAL(int(6), int(value));
    }

    // 1 byte, 4, 255, 250, 245
    config.start.assign(1, 4);
    config.step.assign(1, 5);
    config.mask.assign(1, 0xff);
    config.recycleCount = 4;
    config.stutterCount = 0;
    config.mode = PlaylistConfig::MOD_DECR;
    
    {
        std::auto_ptr<AbstModifier> modPtr(ModifierBlock::MakeRangeMod(config, randSeed));  
        CPPUNIT_ASSERT(modPtr.get() != 0);
        CPPUNIT_ASSERT_EQUAL(size_t(1), modPtr->GetSize());
       
        uint8_t value = 0;
        modPtr->GetValue(&value); 

        CPPUNIT_ASSERT_EQUAL(int(4), int(value));

        modPtr->Next(); 
        modPtr->GetValue(&value); 
        CPPUNIT_ASSERT_EQUAL(int(255), int(value));

        modPtr->Next(); 
        modPtr->GetValue(&value); 
        CPPUNIT_ASSERT_EQUAL(int(250), int(value));
    }

    
    // 2 bytes, 1, 2, 3, 4

    config.start = vec0001;
    config.step  = vec0001;
    config.mask  = vecffff;
    config.recycleCount = 4;
    config.stutterCount = 0;
    config.mode = PlaylistConfig::MOD_INCR;

    {
        std::auto_ptr<AbstModifier> modPtr(ModifierBlock::MakeRangeMod(config, randSeed));  
        CPPUNIT_ASSERT(modPtr.get() != 0);
        CPPUNIT_ASSERT_EQUAL(size_t(2), modPtr->GetSize());
       
        uint8_t value[2] = {};
        modPtr->GetValue(&value[0]); 

        CPPUNIT_ASSERT(strncmp("\x00\x01", (char *)value, 2) == 0);

        modPtr->Next(); 
        CPPUNIT_ASSERT(strncmp("\x00\x02", (char *)value, 2) == 0);
    }

    // 4 bytes, 0x03030303, 0x02020202, 0x01010101, 0, -0x01010101

    config.start.assign(4, 0x03);
    config.step.assign(4, 0x01);
    config.mask.assign(4, 0xff);
    config.recycleCount = 5;
    config.stutterCount = 0;
    config.mode = PlaylistConfig::MOD_DECR;

    {
        std::auto_ptr<AbstModifier> modPtr(ModifierBlock::MakeRangeMod(config, randSeed));  
        CPPUNIT_ASSERT(modPtr.get() != 0);
        CPPUNIT_ASSERT_EQUAL(size_t(4), modPtr->GetSize());
       
        uint8_t value[4] = {};
        modPtr->GetValue(&value[0]); 

        CPPUNIT_ASSERT(strncmp("\x03\x03\x03\x03", (char *)value, 4) == 0);

        modPtr->Next(); 
        modPtr->GetValue(&value[0]); 
        CPPUNIT_ASSERT(strncmp("\x02\x02\x02\x02", (char *)value, 4) == 0);
    }

    // 8 bytes, 0x22222222_222222222, 0x33333333_33333333, 0, 0x111111111_11111111
    
    config.start.assign(8, 0x22);
    config.step.assign(8, 0x11);
    config.mask.assign(8, 0x33);
    config.recycleCount = 9;
    config.stutterCount = 0;
    config.mode = PlaylistConfig::MOD_INCR;

    {
        std::auto_ptr<AbstModifier> modPtr(ModifierBlock::MakeRangeMod(config, randSeed));  
        CPPUNIT_ASSERT(modPtr.get() != 0);
        CPPUNIT_ASSERT_EQUAL(size_t(8), modPtr->GetSize());
       
        uint8_t value[8] = {};
        modPtr->GetValue(&value[0]); 

        CPPUNIT_ASSERT(strncmp("\x22\x22\x22\x22\x22\x22\x22\x22", (char *)value, 8) == 0);

        modPtr->Next(); 
        modPtr->GetValue(&value[0]); 
        CPPUNIT_ASSERT(strncmp("\x33\x33\x33\x33\x33\x33\x33\x33", (char *)value, 8) == 0);

        modPtr->Next(); 
        modPtr->GetValue(&value[0]); 
        CPPUNIT_ASSERT(strncmp("\x00\x00\x00\x00\x00\x00\x00\x00", (char *)value, 8) == 0);
    }

    // test invalid configs 
    
    // start/step size mismatch
    config.start.assign(1, 0x22);
    config.step.assign(2, 0x11);
    config.mask.assign(1, 0x33);
    CPPUNIT_ASSERT_THROW(ModifierBlock::MakeRangeMod(config, randSeed), DPG_EInternal);
    
    // verify we can fix it
    config.step.assign(1, 0x11);
    ModifierBlock::MakeRangeMod(config, randSeed);

    // start/mask mismatch
    config.start.assign(1, 0x22);
    config.step.assign(1, 0x11);
    config.mask.assign(2, 0x33);
    CPPUNIT_ASSERT_THROW(ModifierBlock::MakeRangeMod(config, randSeed), DPG_EInternal);

    // verify we can fix it
    config.mask.assign(1, 0x33);
    ModifierBlock::MakeRangeMod(config, randSeed);

    // unsupported modifier size
    config.start.assign(7, 0x22);
    config.step.assign(7, 0x11);
    config.mask.assign(7, 0x33);
    CPPUNIT_ASSERT_THROW(ModifierBlock::MakeRangeMod(config, randSeed), DPG_EInternal);

    // verify we can fix it
    config.start.assign(1, 0x22);
    config.step.assign(1, 0x11);
    config.mask.assign(1, 0x33);
    ModifierBlock::MakeRangeMod(config, randSeed);

    // unsupported random modifier size
    config.mode = PlaylistConfig::MOD_RANDOM;
    config.mask.assign(3, 0x33);
    CPPUNIT_ASSERT_THROW(ModifierBlock::MakeRangeMod(config, randSeed), DPG_EInternal);

    // verify we can fix it (still random)
    config.mask.assign(4, 0x33);
    ModifierBlock::MakeRangeMod(config, randSeed);
}

///////////////////////////////////////////////////////////////////////////////

void TestModifierBlock::testRangeModifiers(void)
{
    PlaylistConfig playConfig;
    playConfig.flows.resize(2);

    std::vector<uint8_t> vec0001(2, 0x00);
    vec0001[1] = 0x01;
    std::vector<uint8_t> vec1200(2, 0x12);
    vec1200[1] = 0x00;
    std::vector<uint8_t> vecffff(2, 0xff);

    // first variable in first flow, modified to step up from 00 01

    playConfig.flows[0].rangeModMap[0].start = vec0001;
    playConfig.flows[0].rangeModMap[0].step  = vec0001;
    playConfig.flows[0].rangeModMap[0].mask  = vecffff;
    playConfig.flows[0].rangeModMap[0].recycleCount = 4;
    playConfig.flows[0].rangeModMap[0].stutterCount = 0;
    playConfig.flows[0].rangeModMap[0].mode = PlaylistConfig::MOD_INCR;

    // second variable in second flow, down from 0x12 0x00 
    
    playConfig.flows[1].rangeModMap[1].start = vec1200;
    playConfig.flows[1].rangeModMap[1].step  = vec0001;
    playConfig.flows[1].rangeModMap[1].mask  = vecffff;
    playConfig.flows[1].rangeModMap[1].recycleCount = 3;
    playConfig.flows[1].rangeModMap[1].stutterCount = 0;
    playConfig.flows[1].rangeModMap[1].mode = PlaylistConfig::MOD_DECR;

    ModifierBlock mods(playConfig, 0);

    // check size (flow, var) 
    CPPUNIT_ASSERT_EQUAL(size_t(2), mods.GetSize(0, 0));
    CPPUNIT_ASSERT_EQUAL(size_t(0), mods.GetSize(0, 1));
    CPPUNIT_ASSERT_EQUAL(size_t(0), mods.GetSize(0, 2));
    CPPUNIT_ASSERT_EQUAL(size_t(0), mods.GetSize(1, 0));
    CPPUNIT_ASSERT_EQUAL(size_t(2), mods.GetSize(1, 1));
    CPPUNIT_ASSERT_EQUAL(size_t(0), mods.GetSize(1, 2));

    // check initial values
    std::vector<uint8_t> value(2, 0x00);
    
    mods.GetValue(0, 0, &value[0]);
    CPPUNIT_ASSERT(value == vec0001);
    mods.GetValue(1, 1, &value[0]);
    CPPUNIT_ASSERT(value == vec1200);

    // check next values
    mods.Next();
    mods.GetValue(0, 0, &value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(2), int(value[1]));
    mods.GetValue(1, 1, &value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0x11), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(0xff), int(value[1]));

    // check third values 
    mods.Next();
    mods.GetValue(0, 0, &value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[1]));
    mods.GetValue(1, 1, &value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0x11), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(0xfe), int(value[1]));

    // make a copy and verify it's the same
    ModifierBlock mods2(mods);
    mods2.GetValue(0, 0, &value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(3), int(value[1]));
    mods2.GetValue(1, 1, &value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0x11), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(0xfe), int(value[1]));

    // check fourth values (mods @1,1 should repeat)
    mods.Next();
    mods.GetValue(0, 0, &value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(4), int(value[1]));
    mods.GetValue(1, 1, &value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0x12), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(0x00), int(value[1]));

    // double check the copy
    mods2.Next();
    mods2.GetValue(0, 0, &value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(4), int(value[1]));
    mods2.GetValue(1, 1, &value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0x12), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(0x00), int(value[1]));

    // check cursor setting (99 % 4 == 3, 99 % 3 == 0)
    mods.SetCursor(99);
    mods.GetValue(0, 0, &value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(4), int(value[1]));
    mods.GetValue(1, 1, &value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0x12), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(0x00), int(value[1]));

    // check index setting (197 % 4 == 1, 197 % 3 == 2)
    mods.SetCursor(197);
    mods.GetValue(0, 0, &value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(2), int(value[1]));
    mods.GetValue(1, 1, &value[0]);
    CPPUNIT_ASSERT_EQUAL(int(0x11), int(value[0]));
    CPPUNIT_ASSERT_EQUAL(int(0xfe), int(value[1]));
}

///////////////////////////////////////////////////////////////////////////////

void TestModifierBlock::testTableModifiers(void)
{
    PlaylistConfig playConfig;
    playConfig.flows.resize(2);

    std::vector<uint8_t> vec0001(2, 0x00);
    vec0001[1] = 0x01;
    std::vector<uint8_t> vec1200(2, 0x12);
    vec1200[1] = 0x00;
    std::vector<uint8_t> vecffff(2, 0xff);

    // first variable in first flow, 00 01, 12 00, ff ff

    playConfig.flows[0].tableModMap[0].table.resize(3);
    playConfig.flows[0].tableModMap[0].table[0] = vec0001;
    playConfig.flows[0].tableModMap[0].table[1] = vec1200;
    playConfig.flows[0].tableModMap[0].table[2] = vecffff;
    playConfig.flows[0].tableModMap[0].stutterCount = 0;
    

    // second variable in second flow, ffff, 00 01
    
    playConfig.flows[1].tableModMap[1].table.resize(2);
    playConfig.flows[1].tableModMap[1].table[0] = vecffff;
    playConfig.flows[1].tableModMap[1].table[1] = vec0001;
    playConfig.flows[1].tableModMap[1].stutterCount = 0;
    

    ModifierBlock mods(playConfig, 0);

    // check size (flow, var) 
    CPPUNIT_ASSERT_EQUAL(size_t(2), mods.GetSize(0, 0));
    CPPUNIT_ASSERT_EQUAL(size_t(0), mods.GetSize(0, 1));
    CPPUNIT_ASSERT_EQUAL(size_t(0), mods.GetSize(0, 2));
    CPPUNIT_ASSERT_EQUAL(size_t(0), mods.GetSize(1, 0));
    CPPUNIT_ASSERT_EQUAL(size_t(2), mods.GetSize(1, 1));
    CPPUNIT_ASSERT_EQUAL(size_t(0), mods.GetSize(1, 2));

    // check initial values
    std::vector<uint8_t> value(2, 0x00);
    
    mods.GetValue(0, 0, &value[0]);
    CPPUNIT_ASSERT(value == vec0001);
    mods.GetValue(1, 1, &value[0]);
    CPPUNIT_ASSERT(value == vecffff);

    // check next values
    mods.Next();
    mods.GetValue(0, 0, &value[0]);
    CPPUNIT_ASSERT(value == vec1200);
    mods.GetValue(1, 1, &value[0]);
    CPPUNIT_ASSERT(value == vec0001);

    // make a copy and check it's the same
    ModifierBlock mods2(mods);
    mods2.GetValue(0, 0, &value[0]);
    CPPUNIT_ASSERT(value == vec1200);
    mods2.GetValue(1, 1, &value[0]);
    CPPUNIT_ASSERT(value == vec0001);

    // check third values (mods @1,1 should repeat)
    mods.Next();
    mods.GetValue(0, 0, &value[0]);
    CPPUNIT_ASSERT(value == vecffff);
    mods.GetValue(1, 1, &value[0]);
    CPPUNIT_ASSERT(value == vecffff);

    // double check copy
    mods2.Next();
    mods2.GetValue(0, 0, &value[0]);
    CPPUNIT_ASSERT(value == vecffff);
    mods2.GetValue(1, 1, &value[0]);
    CPPUNIT_ASSERT(value == vecffff);

    // check cursor setting (99 % 3 == 0, 99 % 2 == 1)
    mods.SetCursor(99);
    mods.GetValue(0, 0, &value[0]);
    CPPUNIT_ASSERT(value == vec0001);
    mods.GetValue(1, 1, &value[0]);
    CPPUNIT_ASSERT(value == vec0001);

    // check index setting (200 % 3 == 2, 200 % 2 == 0)
    mods.SetCursor(200);
    mods.GetValue(0, 0, &value[0]);
    CPPUNIT_ASSERT(value == vecffff);
    mods.GetValue(1, 1, &value[0]);
    CPPUNIT_ASSERT(value == vecffff);
}

///////////////////////////////////////////////////////////////////////////////

void TestModifierBlock::testRandomModifiers(void)
{
    PlaylistConfig playConfig;
    playConfig.flows.resize(2);

    std::vector<uint8_t> vec0001(2, 0x00);
    vec0001[1] = 0x01;
    std::vector<uint8_t> vec1200(2, 0x12);
    vec1200[1] = 0x00;
    std::vector<uint8_t> vecffff(2, 0xff);

    // first variable in first flow, 2 bytes, 2 count, 1 stutter

    playConfig.flows[0].rangeModMap[0].mask  = vecffff;
    playConfig.flows[0].rangeModMap[0].recycleCount = 2;
    playConfig.flows[0].rangeModMap[0].stutterCount = 1;
    playConfig.flows[0].rangeModMap[0].mode = PlaylistConfig::MOD_RANDOM;

    // second variable in second flow, 2 bytes, 3 count
    
    playConfig.flows[1].rangeModMap[1].mask  = vecffff;
    playConfig.flows[1].rangeModMap[1].recycleCount = 3;
    playConfig.flows[1].rangeModMap[1].stutterCount = 0;
    playConfig.flows[1].rangeModMap[1].mode = PlaylistConfig::MOD_RANDOM;

    ModifierBlock mods(playConfig, 0x12345678);

    // check size (flow, var) 
    CPPUNIT_ASSERT_EQUAL(size_t(2), mods.GetSize(0, 0));
    CPPUNIT_ASSERT_EQUAL(size_t(0), mods.GetSize(0, 1));
    CPPUNIT_ASSERT_EQUAL(size_t(0), mods.GetSize(0, 2));
    CPPUNIT_ASSERT_EQUAL(size_t(0), mods.GetSize(1, 0));
    CPPUNIT_ASSERT_EQUAL(size_t(2), mods.GetSize(1, 1));
    CPPUNIT_ASSERT_EQUAL(size_t(0), mods.GetSize(1, 2));

    // check initial values
    uint16_t value0_init = 0;
    uint16_t value1_init = 0;
    uint16_t value0_1 = 0;
    uint16_t value1_1 = 0;
    uint16_t value0_x = 0;
    uint16_t value1_2 = 0;
    uint16_t value1_x = 0;
    
    mods.GetValue(0, 0, (uint8_t*)&value0_init);
    mods.GetValue(1, 1, (uint8_t*)&value1_init);
    CPPUNIT_ASSERT(value0_init != value1_init);

    // check next values (mods @0,0 should stutter)
    mods.Next();
    mods.GetValue(0, 0, (uint8_t*)&value0_x);
    CPPUNIT_ASSERT_EQUAL(value0_init, value0_x);
    mods.GetValue(1, 1, (uint8_t*)&value1_1);
    CPPUNIT_ASSERT(value1_1 != value1_init);

    // check third values
    mods.Next();
    mods.GetValue(0, 0, (uint8_t*)&value0_1);
    CPPUNIT_ASSERT(value0_1 != value0_init);
    mods.GetValue(1, 1, (uint8_t*)&value1_2);
    CPPUNIT_ASSERT(value1_2 != value1_init);

    // check fifth values (mods @0,0 should repeat) (mods @1,1 should be same as second)
    mods.Next();
    mods.Next();
    mods.GetValue(0, 0, (uint8_t*)&value0_x);
    CPPUNIT_ASSERT_EQUAL(value0_init, value0_x);
    mods.GetValue(1, 1, (uint8_t*)&value1_x);
    CPPUNIT_ASSERT_EQUAL(value1_1, value1_x);

    // make a copy and check it's the same
    ModifierBlock mods2(mods);
    mods2.GetValue(0, 0, (uint8_t*)&value0_x);
    CPPUNIT_ASSERT_EQUAL(value0_init, value0_x);
    mods2.GetValue(1, 1, (uint8_t*)&value1_x);
    CPPUNIT_ASSERT_EQUAL(value1_1, value1_x);

    // check sixth values
    mods.Next();
    mods.GetValue(0, 0, (uint8_t*)&value0_x);
    CPPUNIT_ASSERT_EQUAL(value0_init, value0_x);
    mods.GetValue(1, 1, (uint8_t*)&value1_x);
    CPPUNIT_ASSERT_EQUAL(value1_2, value1_x);

    // double check copy
    mods2.Next();
    mods2.GetValue(0, 0, (uint8_t*)&value0_x);
    CPPUNIT_ASSERT_EQUAL(value0_init, value0_x);
    mods2.GetValue(1, 1, (uint8_t*)&value1_x);
    CPPUNIT_ASSERT_EQUAL(value1_2, value1_x);

    // check cursor setting (99 / 2 % 2 == 1, 99 % 3 == 0)
    mods.SetCursor(99);
    mods.GetValue(0, 0, (uint8_t*)&value0_x);
    CPPUNIT_ASSERT_EQUAL(value0_1, value0_x);
    mods.GetValue(1, 1, (uint8_t*)&value1_x);
    CPPUNIT_ASSERT_EQUAL(value1_init, value1_x);

    // check index setting (200 / 2 % 2 == 0, 200 % 3 == 2)
    mods.SetCursor(200);
    mods.GetValue(0, 0, (uint8_t*)&value0_x);
    CPPUNIT_ASSERT_EQUAL(value0_init, value0_x);
    mods.GetValue(1, 1, (uint8_t*)&value1_x);
    CPPUNIT_ASSERT_EQUAL(value1_2, value1_x);

    // check with another seed
    ModifierBlock mods_b(playConfig, 0x12345679);
    mods_b.GetValue(0, 0, (uint8_t*)&value0_x);
    mods_b.GetValue(1, 1, (uint8_t*)&value1_x);
    CPPUNIT_ASSERT(value0_x != value0_init);
    CPPUNIT_ASSERT(value1_x != value1_init);
}

///////////////////////////////////////////////////////////////////////////////

void TestModifierBlock::testLinkModifiers(void)
{
    PlaylistConfig playConfig;
    playConfig.flows.resize(1);

    std::vector<uint8_t> vec01(1, 0x01);
    std::vector<uint8_t> vec05(1, 0x05);
    std::vector<uint8_t> vecff(1, 0xff);

    // first variable, step up from 01, count 10000

    playConfig.flows[0].rangeModMap[0].start = vec01;
    playConfig.flows[0].rangeModMap[0].step  = vec01;
    playConfig.flows[0].rangeModMap[0].mask  = vecff;
    playConfig.flows[0].rangeModMap[0].recycleCount = 10000;
    playConfig.flows[0].rangeModMap[0].stutterCount = 0;
    playConfig.flows[0].rangeModMap[0].mode = PlaylistConfig::MOD_INCR;

    // second variable, step up from 01, count 2
    
    playConfig.flows[0].rangeModMap[1].start = vec01;
    playConfig.flows[0].rangeModMap[1].step  = vec01;
    playConfig.flows[0].rangeModMap[1].mask  = vecff;
    playConfig.flows[0].rangeModMap[1].recycleCount = 2;
    playConfig.flows[0].rangeModMap[1].stutterCount = 0;
    playConfig.flows[0].rangeModMap[1].mode = PlaylistConfig::MOD_INCR;

    // third variable, step up by 5 from 5, count 2

    playConfig.flows[0].rangeModMap[2].start = vec05;
    playConfig.flows[0].rangeModMap[2].step  = vec05;
    playConfig.flows[0].rangeModMap[2].mask  = vecff;
    playConfig.flows[0].rangeModMap[2].recycleCount = 2;
    playConfig.flows[0].rangeModMap[2].stutterCount = 0;
    playConfig.flows[0].rangeModMap[2].mode = PlaylistConfig::MOD_INCR;

    // fourth variable, step up by 1 from 01, count 10000

    playConfig.flows[0].rangeModMap[3].start = vec01;
    playConfig.flows[0].rangeModMap[3].step  = vec01;
    playConfig.flows[0].rangeModMap[3].mask  = vecff;
    playConfig.flows[0].rangeModMap[3].recycleCount = 10000;
    playConfig.flows[0].rangeModMap[3].stutterCount = 0;
    playConfig.flows[0].rangeModMap[3].mode = PlaylistConfig::MOD_INCR;

    // link variable 2 -> 1 -> 0 (3 stays unlinked)
    
    playConfig.flows[0].linkModMap[2] = 1;
    playConfig.flows[0].linkModMap[1] = 0;

    ModifierBlock mods(playConfig, 0);

    AbstModifier * mod0 = mods.mMap[std::make_pair(uint16_t(0), uint8_t(0))].get();
    AbstModifier * mod1 = mods.mMap[std::make_pair(uint16_t(0), uint8_t(1))].get();
    AbstModifier * mod2 = mods.mMap[std::make_pair(uint16_t(0), uint8_t(2))].get();
    AbstModifier * mod3 = mods.mMap[std::make_pair(uint16_t(0), uint8_t(3))].get();

    // check HasParent

    CPPUNIT_ASSERT(mod0->HasParent());
    CPPUNIT_ASSERT(mod1->HasParent());
    CPPUNIT_ASSERT(!mod2->HasParent());
    CPPUNIT_ASSERT(!mod3->HasParent());

    // check HasChild

    CPPUNIT_ASSERT(!mod0->HasChild());
    CPPUNIT_ASSERT(mod1->HasChild());
    CPPUNIT_ASSERT(mod2->HasChild());
    CPPUNIT_ASSERT(!mod3->HasChild());

    // child Child links
    
    CPPUNIT_ASSERT(mod1->GetChild() == mod0);
    CPPUNIT_ASSERT(mod2->GetChild() == mod1);

    // check initial values
    
    uint8_t value = 0;
    mods.GetValue(0, 0, &value);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value));
    mods.GetValue(0, 1, &value);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value));
    mods.GetValue(0, 2, &value);
    CPPUNIT_ASSERT_EQUAL(int(5), int(value));
    mods.GetValue(0, 3, &value);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value));

    // check next (should only increment roots (2 and 3)

    mods.Next();
    mods.GetValue(0, 0, &value);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value));
    mods.GetValue(0, 1, &value);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value));
    mods.GetValue(0, 2, &value);
    CPPUNIT_ASSERT_EQUAL(int(10), int(value));
    mods.GetValue(0, 3, &value);
    CPPUNIT_ASSERT_EQUAL(int(2), int(value));

    // check next (carry 2 -> 1)

    mods.Next();
    mods.GetValue(0, 0, &value);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value));
    mods.GetValue(0, 1, &value);
    CPPUNIT_ASSERT_EQUAL(int(2), int(value));
    mods.GetValue(0, 2, &value);
    CPPUNIT_ASSERT_EQUAL(int(5), int(value));
    mods.GetValue(0, 3, &value);
    CPPUNIT_ASSERT_EQUAL(int(3), int(value));

    // check next (no carry)

    mods.Next();
    mods.GetValue(0, 0, &value);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value));
    mods.GetValue(0, 1, &value);
    CPPUNIT_ASSERT_EQUAL(int(2), int(value));
    mods.GetValue(0, 2, &value);
    CPPUNIT_ASSERT_EQUAL(int(10), int(value));
    mods.GetValue(0, 3, &value);
    CPPUNIT_ASSERT_EQUAL(int(4), int(value));

    // next (carry 2 -> 1 -> 0)
    
    mods.Next();
    mods.GetValue(0, 0, &value);
    CPPUNIT_ASSERT_EQUAL(int(2), int(value));
    mods.GetValue(0, 1, &value);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value));
    mods.GetValue(0, 2, &value);
    CPPUNIT_ASSERT_EQUAL(int(5), int(value));
    mods.GetValue(0, 3, &value);
    CPPUNIT_ASSERT_EQUAL(int(5), int(value));

    // next (no carry)
    
    mods.Next();
    mods.GetValue(0, 0, &value);
    CPPUNIT_ASSERT_EQUAL(int(2), int(value));
    mods.GetValue(0, 1, &value);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value));
    mods.GetValue(0, 2, &value);
    CPPUNIT_ASSERT_EQUAL(int(10), int(value));
    mods.GetValue(0, 3, &value);
    CPPUNIT_ASSERT_EQUAL(int(6), int(value));

    // next (carry 2 -> 1)
    
    mods.Next();
    mods.GetValue(0, 0, &value);
    CPPUNIT_ASSERT_EQUAL(int(2), int(value));
    mods.GetValue(0, 1, &value);
    CPPUNIT_ASSERT_EQUAL(int(2), int(value));
    mods.GetValue(0, 2, &value);
    CPPUNIT_ASSERT_EQUAL(int(5), int(value));
    mods.GetValue(0, 3, &value);
    CPPUNIT_ASSERT_EQUAL(int(7), int(value));

    // check SetCursor
    mods.SetCursor(999);
    mods.GetValue(0, 0, &value);
    CPPUNIT_ASSERT_EQUAL(int(250), int(value));
    mods.GetValue(0, 1, &value);
    CPPUNIT_ASSERT_EQUAL(int(2), int(value));
    mods.GetValue(0, 2, &value);
    CPPUNIT_ASSERT_EQUAL(int(10), int(value));
    mods.GetValue(0, 3, &value);
    CPPUNIT_ASSERT_EQUAL(int(1000 & 0xff), int(value));

    // make a copy and check child links
    ModifierBlock mods2(mods);

    AbstModifier * mod2_0 = mods2.mMap[std::make_pair(uint16_t(0), uint8_t(0))].get();
    AbstModifier * mod2_1 = mods2.mMap[std::make_pair(uint16_t(0), uint8_t(1))].get();
    AbstModifier * mod2_2 = mods2.mMap[std::make_pair(uint16_t(0), uint8_t(2))].get();
    AbstModifier * mod2_3 = mods2.mMap[std::make_pair(uint16_t(0), uint8_t(3))].get();

    // make sure it's not a pointer copy
    
    CPPUNIT_ASSERT(mod2_0 != mod0); mod0 = mod2_0;
    CPPUNIT_ASSERT(mod2_1 != mod1); mod1 = mod2_1;
    CPPUNIT_ASSERT(mod2_2 != mod2); mod2 = mod2_2;
    CPPUNIT_ASSERT(mod2_3 != mod3); mod3 = mod2_3;

    // check HasParent

    CPPUNIT_ASSERT(mod0->HasParent());
    CPPUNIT_ASSERT(mod1->HasParent());
    CPPUNIT_ASSERT(!mod2->HasParent());
    CPPUNIT_ASSERT(!mod3->HasParent());

    // check HasChild

    CPPUNIT_ASSERT(!mod0->HasChild());
    CPPUNIT_ASSERT(mod1->HasChild());
    CPPUNIT_ASSERT(mod2->HasChild());
    CPPUNIT_ASSERT(!mod3->HasChild());

    // child Child links
    
    CPPUNIT_ASSERT(mod1->GetChild() == mod0);
    CPPUNIT_ASSERT(mod2->GetChild() == mod1);

    // check values were copied
    mods2.GetValue(0, 0, &value);
    CPPUNIT_ASSERT_EQUAL(int(250), int(value));
    mods2.GetValue(0, 1, &value);
    CPPUNIT_ASSERT_EQUAL(int(2), int(value));
    mods2.GetValue(0, 2, &value);
    CPPUNIT_ASSERT_EQUAL(int(10), int(value));
    mods2.GetValue(0, 3, &value);
    CPPUNIT_ASSERT_EQUAL(int(1000 & 0xff), int(value));

    // check next (carry 2 -> 1 -> 0)
    mods2.Next();
    mods2.GetValue(0, 0, &value);
    CPPUNIT_ASSERT_EQUAL(int(251), int(value));
    mods2.GetValue(0, 1, &value);
    CPPUNIT_ASSERT_EQUAL(int(1), int(value));
    mods2.GetValue(0, 2, &value);
    CPPUNIT_ASSERT_EQUAL(int(5), int(value));
    mods2.GetValue(0, 3, &value);
    CPPUNIT_ASSERT_EQUAL(int(1001 & 0xff), int(value));
}

///////////////////////////////////////////////////////////////////////////////

void TestModifierBlock::testEmptyBlock(void)
{
    PlaylistConfig playConfig;

    ModifierBlock mods(playConfig, 0x12345678);

    CPPUNIT_ASSERT_EQUAL(uint32_t(0), mods.GetCursor());

    mods.Next();
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), mods.GetCursor());

    mods.Next();
    mods.Next();
    mods.Next();
    CPPUNIT_ASSERT_EQUAL(uint32_t(4), mods.GetCursor());

    mods.SetCursor(10000);
    CPPUNIT_ASSERT_EQUAL(uint32_t(10000), mods.GetCursor());

    // Test copy
    ModifierBlock mods2(mods);

    CPPUNIT_ASSERT_EQUAL(uint32_t(10000), mods2.GetCursor());

    mods2.Next();
    CPPUNIT_ASSERT_EQUAL(uint32_t(10001), mods2.GetCursor());
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestModifierBlock);
