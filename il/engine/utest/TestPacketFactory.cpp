#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "FlowConfig.h"
#include "PacketFactory.h"
#include "ModifierBlock.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class TestPacketFactory : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestPacketFactory);
    CPPUNIT_TEST(testNoVars);
    CPPUNIT_TEST(testNoData);
    CPPUNIT_TEST(testAllDefaults);
    CPPUNIT_TEST(testLittleEndianVars);
    CPPUNIT_TEST(testVarModifierSizeMismatch);
    CPPUNIT_TEST(testOverlappingVars);
    CPPUNIT_TEST(testNonDefaults);
    CPPUNIT_TEST(testRangeModifiers);
    CPPUNIT_TEST(testMaskedModifiers);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) 
    {
    }

    void tearDown(void) 
    {
    }

protected:
    void testNoVars(void);
    void testNoData(void);
    void testAllDefaults(void);
    void testLittleEndianVars(void);
    void testVarModifierSizeMismatch(void);
    void testOverlappingVars(void);
    void testNonDefaults(void);
    void testRangeModifiers(void);
    void testMaskedModifiers(void);

private:
};

///////////////////////////////////////////////////////////////////////////////

void TestPacketFactory::testNoVars(void)
{
    FlowConfig flowConfig;
    PlaylistConfig playConfig;

    playConfig.flows.resize(1);

    flowConfig.pktList.resize(2);
    flowConfig.pktList[0].data.resize(4, 0);
    flowConfig.pktList[0].data[0] = 0x12;
    flowConfig.pktList[0].data[0] = 0x34;
    flowConfig.pktList[0].data[0] = 0x56;
    flowConfig.pktList[0].data[0] = 0x78;
    flowConfig.pktList[1].data.resize(16, 0xff);

    ModifierBlock modifiers(playConfig, 0 /* seed */);
    FlowModifierBlock flowModifiers(modifiers, 0 /* flowIdx */);

    PacketFactory factory0(playConfig.flows[0].varMap, flowConfig.pktList[0], flowConfig.varList, flowModifiers);
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].data.size(), factory0.BufferLength());
    CPPUNIT_ASSERT(memcmp(&flowConfig.pktList[0].data[0], factory0.Buffer(), factory0.BufferLength()) == 0);
    
    PacketFactory factory1(playConfig.flows[0].varMap, flowConfig.pktList[1], flowConfig.varList, flowModifiers);
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].data.size(), factory1.BufferLength());
    CPPUNIT_ASSERT(memcmp(&flowConfig.pktList[1].data[0], factory1.Buffer(), factory1.BufferLength()) == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestPacketFactory::testNoData(void)
{
    FlowConfig flowConfig;
    PlaylistConfig playConfig;
    typedef FlowConfig::Packet::VarIdxMap_t::value_type varMapValue_t;

    playConfig.flows.resize(1);

    flowConfig.pktList.resize(2);
    // leave pktList[0] and pktList[1] empty

    flowConfig.varList.resize(3);
    // first variable, beginning of packet 0, 1 byte 0x99
    flowConfig.varList[0].value.resize(1, 0x99);
    flowConfig.varList[0].targetList.resize(1);
    flowConfig.varList[0].targetList[0].pktIdx = 0;
    flowConfig.varList[0].targetList[0].byteIdx = 0;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(0, 0));

    // second variable, beginning of packet 1, 5 bytes 0x01, 0x00, 0x00, 0x00, 0x00
    flowConfig.varList[1].value.resize(5, 0);
    flowConfig.varList[1].value[0] = 1;
    flowConfig.varList[1].targetList.resize(1);
    flowConfig.varList[1].targetList[0].pktIdx = 0;
    flowConfig.varList[1].targetList[0].byteIdx = 0;
    flowConfig.pktList[1].varMap.insert(varMapValue_t(0, 1));

    // third variable, after variables in packet 0 AND packet 1, 2 bytes 0xab 0xcd
    flowConfig.varList[2].value.resize(2, 0xab);
    flowConfig.varList[2].value[1] = 0xcd;
    flowConfig.varList[2].targetList.resize(2);
    flowConfig.varList[2].targetList[0].pktIdx = 0;
    flowConfig.varList[2].targetList[0].byteIdx = 0;
    flowConfig.varList[2].targetList[1].pktIdx = 0;
    flowConfig.varList[2].targetList[1].byteIdx = 0;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(0, 2));
    flowConfig.pktList[1].varMap.insert(varMapValue_t(0, 2));

    ModifierBlock modifiers(playConfig, 0 /* seed */);
    FlowModifierBlock flowModifiers(modifiers, 0 /* flowIdx */);

    PacketFactory factory0(playConfig.flows[0].varMap, flowConfig.pktList[0], flowConfig.varList, flowModifiers);
    char exp0[] = {0x99, 0xab, 0xcd};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp0), factory0.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp0, factory0.Buffer(), factory0.BufferLength()) == 0);
    
    PacketFactory factory1(playConfig.flows[0].varMap, flowConfig.pktList[1], flowConfig.varList, flowModifiers);
    char exp1[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0xab, 0xcd};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp1), factory1.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp1, factory1.Buffer(), factory1.BufferLength()) == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestPacketFactory::testAllDefaults(void)
{
    FlowConfig flowConfig;
    PlaylistConfig playConfig;
    typedef FlowConfig::Packet::VarIdxMap_t::value_type varMapValue_t;

    playConfig.flows.resize(1);

    flowConfig.pktList.resize(2);
    flowConfig.pktList[0].data.resize(4, 0);
    flowConfig.pktList[0].data[0] = 0x12;
    flowConfig.pktList[0].data[1] = 0x34;
    flowConfig.pktList[0].data[2] = 0x56;
    flowConfig.pktList[0].data[3] = 0x78;
    flowConfig.pktList[1].data.resize(16, 0xff);

    flowConfig.varList.resize(3);
    // first variable, beginning of packet 0, 1 byte 0x99
    flowConfig.varList[0].value.resize(1, 0x99);
    flowConfig.varList[0].targetList.resize(1);
    flowConfig.varList[0].targetList[0].pktIdx = 0;
    flowConfig.varList[0].targetList[0].byteIdx = 0;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(0, 0));

    // second variable, end of packet 1, 5 bytes 0x01, 0x00, 0x00, 0x00, 0x00
    flowConfig.varList[1].value.resize(5, 0);
    flowConfig.varList[1].value[0] = 1;
    flowConfig.varList[1].targetList.resize(1);
    flowConfig.varList[1].targetList[0].pktIdx = 1;
    flowConfig.varList[1].targetList[0].byteIdx = 16;
    flowConfig.pktList[1].varMap.insert(varMapValue_t(16, 1));

    // third variable, after byte 1 of packet 0 AND packet 1, 2 bytes 0xab 0xcd
    flowConfig.varList[2].value.resize(2, 0xab);
    flowConfig.varList[2].value[1] = 0xcd;
    flowConfig.varList[2].targetList.resize(2);
    flowConfig.varList[2].targetList[0].pktIdx = 0;
    flowConfig.varList[2].targetList[0].byteIdx = 1;
    flowConfig.varList[2].targetList[1].pktIdx = 1;
    flowConfig.varList[2].targetList[1].byteIdx = 1;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(1, 2));
    flowConfig.pktList[1].varMap.insert(varMapValue_t(1, 2));

    ModifierBlock modifiers(playConfig, 0 /* seed */);
    FlowModifierBlock flowModifiers(modifiers, 0 /* flowIdx */);

    PacketFactory factory0(playConfig.flows[0].varMap, flowConfig.pktList[0], flowConfig.varList, flowModifiers);
    char exp0[] = {0x99, 0x12, 0xab, 0xcd, 0x34, 0x56, 0x78};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp0), factory0.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp0, factory0.Buffer(), factory0.BufferLength()) == 0);
    
    PacketFactory factory1(playConfig.flows[0].varMap, flowConfig.pktList[1], flowConfig.varList, flowModifiers);
    char exp1[] = {0xff, 0xab, 0xcd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                   0x01, 0x00, 0x00, 0x00, 0x00};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp1), factory1.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp1, factory1.Buffer(), factory1.BufferLength()) == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestPacketFactory::testLittleEndianVars(void)
{
    FlowConfig flowConfig;
    PlaylistConfig playConfig;
    typedef FlowConfig::Packet::VarIdxMap_t::value_type varMapValue_t;

    playConfig.flows.resize(1);

    flowConfig.pktList.resize(1);
    flowConfig.pktList[0].data.resize(4, 0);
    flowConfig.pktList[0].data[0] = 0x12;
    flowConfig.pktList[0].data[1] = 0x34;
    flowConfig.pktList[0].data[2] = 0x56;
    flowConfig.pktList[0].data[3] = 0x78;

    flowConfig.varList.resize(3);
    // first variable, beginning of packet 0, 2 bytes 0x99 0x88, LE (swapped)
    flowConfig.varList[0].value.resize(2, 0x99);
    flowConfig.varList[0].value[1] = 0x88;
    flowConfig.varList[0].targetList.resize(1);
    flowConfig.varList[0].targetList[0].pktIdx = 0;
    flowConfig.varList[0].targetList[0].byteIdx = 0;
    flowConfig.varList[0].endian = FlowConfig::LITTLE;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(0, 0));

    // second variable, end of packet 0, 5 bytes 0x01, 0x00, 0x00, 0x00, 0x00, LE (swapped)
    flowConfig.varList[1].value.resize(5, 0);
    flowConfig.varList[1].value[0] = 1;
    flowConfig.varList[1].targetList.resize(1);
    flowConfig.varList[1].targetList[0].pktIdx = 0;
    flowConfig.varList[1].targetList[0].byteIdx = 4;
    flowConfig.varList[1].endian = FlowConfig::LITTLE;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(4, 1));

    // third variable, after byte 1 of packet 0, 4 bytes 0xab 0xcd 0xef 0x11, LE (swapped)
    flowConfig.varList[2].value.resize(4, 0xab);
    flowConfig.varList[2].value[1] = 0xcd;
    flowConfig.varList[2].value[2] = 0xef;
    flowConfig.varList[2].value[3] = 0x11;
    flowConfig.varList[2].targetList.resize(1);
    flowConfig.varList[2].targetList[0].pktIdx = 0;
    flowConfig.varList[2].targetList[0].byteIdx = 1;
    flowConfig.varList[2].endian = FlowConfig::LITTLE;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(1, 2));

    ModifierBlock modifiers(playConfig, 0 /* seed */);
    FlowModifierBlock flowModifiers(modifiers, 0 /* flowIdx */);

    PacketFactory factory0(playConfig.flows[0].varMap, flowConfig.pktList[0], flowConfig.varList, flowModifiers);
    char exp0[] = {0x88, 0x99, 0x12, 0x11, 0xef, 0xcd, 0xab, 0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x01};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp0), factory0.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp0, factory0.Buffer(), factory0.BufferLength()) == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestPacketFactory::testVarModifierSizeMismatch(void)
{
    FlowConfig flowConfig;
    PlaylistConfig playConfig;
    typedef FlowConfig::Packet::VarIdxMap_t::value_type varMapValue_t;

    playConfig.flows.resize(1);

    flowConfig.pktList.resize(1);
    flowConfig.pktList[0].data.resize(4, 0);
    flowConfig.pktList[0].data[0] = 0x12;
    flowConfig.pktList[0].data[1] = 0x34;
    flowConfig.pktList[0].data[2] = 0x56;
    flowConfig.pktList[0].data[3] = 0x78;

    flowConfig.varList.resize(4);
    // first variable, beginning of packet 0, 2 bytes LE
    flowConfig.varList[0].value.resize(2, 0x01);
    flowConfig.varList[0].value[1] = 0x00;
    flowConfig.varList[0].targetList.resize(1);
    flowConfig.varList[0].targetList[0].pktIdx = 0;
    flowConfig.varList[0].targetList[0].byteIdx = 0;
    flowConfig.varList[0].endian = FlowConfig::LITTLE;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(0, 0));

    // second variable, end of packet 0, 5 bytes 
    flowConfig.varList[1].value.resize(5, 0x12);
    flowConfig.varList[1].value[3] = 0x01;
    flowConfig.varList[1].value[4] = 0x00;
    flowConfig.varList[1].targetList.resize(1);
    flowConfig.varList[1].targetList[0].pktIdx = 0;
    flowConfig.varList[1].targetList[0].byteIdx = 4;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(4, 1));

    // third variable, after byte 1 of packet 0, 4 bytes LE 
    flowConfig.varList[2].value.resize(4, 0x23);
    flowConfig.varList[2].value[3] = 0x00;
    flowConfig.varList[2].targetList.resize(1);
    flowConfig.varList[2].targetList[0].pktIdx = 0;
    flowConfig.varList[2].targetList[0].byteIdx = 1;
    flowConfig.varList[2].endian = FlowConfig::LITTLE;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(1, 2));

    // fourth variable, after byte 2 of packet 0, 3 bytes 
    flowConfig.varList[3].value.resize(3, 0x34);
    flowConfig.varList[3].value[2] = 0x01;
    flowConfig.varList[3].targetList.resize(1);
    flowConfig.varList[3].targetList[0].pktIdx = 0;
    flowConfig.varList[3].targetList[0].byteIdx = 2;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(2, 3));

    // first variable, mod size of 4 (2 bytes bigger)

    playConfig.flows[0].rangeModMap[0].start.assign(4, 0x44);
    playConfig.flows[0].rangeModMap[0].start[3] = 0x11;
    playConfig.flows[0].rangeModMap[0].step.assign(4, 0);
    playConfig.flows[0].rangeModMap[0].mask.assign(4, 0xff);
    playConfig.flows[0].rangeModMap[0].recycleCount = 1;
    playConfig.flows[0].rangeModMap[0].stutterCount = 0;
    playConfig.flows[0].rangeModMap[0].mode = PlaylistConfig::MOD_INCR;

    // second variable, mod size of 4 (1 byte smaller)  
    playConfig.flows[0].rangeModMap[1].start.assign(4, 0x55);
    playConfig.flows[0].rangeModMap[1].start[3] = 0x22;
    playConfig.flows[0].rangeModMap[1].step.assign(4, 0);
    playConfig.flows[0].rangeModMap[1].mask.assign(4, 0xff);
    playConfig.flows[0].rangeModMap[1].recycleCount = 1;
    playConfig.flows[0].rangeModMap[1].stutterCount = 0;
    playConfig.flows[0].rangeModMap[1].mode = PlaylistConfig::MOD_INCR;

    // third variable, mod size of 2 (2 bytes smaller)
    playConfig.flows[0].rangeModMap[2].start.assign(2, 0x66);
    playConfig.flows[0].rangeModMap[2].start[1] = 0x55;
    playConfig.flows[0].rangeModMap[2].step.assign(2, 0);
    playConfig.flows[0].rangeModMap[2].mask.assign(2, 0xff);
    playConfig.flows[0].rangeModMap[2].recycleCount = 1;
    playConfig.flows[0].rangeModMap[2].stutterCount = 0;
    playConfig.flows[0].rangeModMap[2].mode = PlaylistConfig::MOD_INCR;

    // fourth variable, mod size of 4 (1 byte bigger)
    playConfig.flows[0].rangeModMap[3].start.assign(4, 0x77);
    playConfig.flows[0].rangeModMap[3].start[3] = 0x44;
    playConfig.flows[0].rangeModMap[3].step.assign(4, 0);
    playConfig.flows[0].rangeModMap[3].mask.assign(4, 0xff);
    playConfig.flows[0].rangeModMap[3].recycleCount = 1;
    playConfig.flows[0].rangeModMap[3].stutterCount = 0;
    playConfig.flows[0].rangeModMap[3].mode = PlaylistConfig::MOD_INCR;

    {
    ModifierBlock mods(playConfig, 0 /* seed */);
    FlowModifierBlock f_mods(mods, 0 /* flowIdx */);

    PacketFactory factory(playConfig.flows[0].varMap, flowConfig.pktList[0], flowConfig.varList, f_mods);
    char exp[] = {0x11, 0x44, 0x44, 0x44, 0x12, 0x55, 0x66, 0x34, 0x77, 0x77, 0x77, 0x44, 0x56, 0x78, 0x55, 0x55, 0x55, 0x22};

    CPPUNIT_ASSERT_EQUAL(sizeof(exp), factory.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp, factory.Buffer(), factory.BufferLength()) == 0);
    }

    // set masks to zero to make sure variables are stuffed right
    playConfig.flows[0].rangeModMap[0].mask.assign(4, 0);
    playConfig.flows[0].rangeModMap[1].mask.assign(4, 0);
    playConfig.flows[0].rangeModMap[2].mask.assign(2, 0);
    playConfig.flows[0].rangeModMap[3].mask.assign(4, 0);

    {
    ModifierBlock mods(playConfig, 0 /* seed */);
    FlowModifierBlock f_mods(mods, 0 /* flowIdx */);

    PacketFactory factory(playConfig.flows[0].varMap, flowConfig.pktList[0], flowConfig.varList, f_mods);
    char exp[] = {0x00, 0x01, 0x00, 0x00, 0x12, 0x00, 0x23, 0x34, 0x34, 0x34, 0x01, 0x00, 0x56, 0x78, 0x12, 0x12, 0x12, 0x01};

    CPPUNIT_ASSERT_EQUAL(sizeof(exp), factory.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp, factory.Buffer(), factory.BufferLength()) == 0);
    }
}

///////////////////////////////////////////////////////////////////////////////

void TestPacketFactory::testOverlappingVars(void)
{
    // variables targetted at the same byte are inserted in the order they are 
    // inserted into the varMap (this is guaranteed by multimap in C++0x and 
    // happens to be a feature of all known pre-C++0x implementations)
    
    FlowConfig flowConfig;
    PlaylistConfig playConfig;
    typedef FlowConfig::Packet::VarIdxMap_t::value_type varMapValue_t;

    playConfig.flows.resize(1);

    flowConfig.pktList.resize(2);
    flowConfig.pktList[0].data.resize(4, 0);
    flowConfig.pktList[0].data[0] = 0x12;
    flowConfig.pktList[0].data[1] = 0x34;
    flowConfig.pktList[0].data[2] = 0x56;
    flowConfig.pktList[0].data[3] = 0x78;
    flowConfig.pktList[1].data.resize(16, 0xff);

    flowConfig.varList.resize(4);
    // first variable, beginning of packet 0, 1 byte 0x99
    flowConfig.varList[0].value.resize(1, 0x99);
    flowConfig.varList[0].targetList.resize(1);
    flowConfig.varList[0].targetList[0].pktIdx = 0;
    flowConfig.varList[0].targetList[0].byteIdx = 0;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(0, 0));

    // second variable, beginning of packet 1, 5 bytes 0x01, 0x00, 0x00, 0x00, 0x00
    flowConfig.varList[1].value.resize(5, 0);
    flowConfig.varList[1].value[0] = 1;
    flowConfig.varList[1].targetList.resize(1);
    flowConfig.varList[1].targetList[0].pktIdx = 1;
    flowConfig.varList[1].targetList[0].byteIdx = 0;
    flowConfig.pktList[1].varMap.insert(varMapValue_t(0, 1));

    // third variable, beginning of packet 0 AND packet 1, 2 bytes 0xab 0xcd
    flowConfig.varList[2].value.resize(2, 0xab);
    flowConfig.varList[2].value[1] = 0xcd;
    flowConfig.varList[2].targetList.resize(2);
    flowConfig.varList[2].targetList[0].pktIdx = 0;
    flowConfig.varList[2].targetList[0].byteIdx = 0;
    flowConfig.varList[2].targetList[1].pktIdx = 1;
    flowConfig.varList[2].targetList[1].byteIdx = 0;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(0, 2));
    flowConfig.pktList[1].varMap.insert(varMapValue_t(0, 2));

    // fourth variable, beginning of packet 1, 1 byte 0x88
    flowConfig.varList[3].value.resize(1, 0x88);
    flowConfig.varList[3].targetList.resize(1);
    flowConfig.varList[3].targetList[0].pktIdx = 1;
    flowConfig.varList[3].targetList[0].byteIdx = 0;
    flowConfig.pktList[1].varMap.insert(varMapValue_t(0, 3));

    ModifierBlock modifiers(playConfig, 0 /* seed */);
    FlowModifierBlock flowModifiers(modifiers, 0 /* flowIdx */);

    PacketFactory factory0(playConfig.flows[0].varMap, flowConfig.pktList[0], flowConfig.varList, flowModifiers);
    char exp0[] = {0x99, 0xab, 0xcd, 0x12, 0x34, 0x56, 0x78};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp0), factory0.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp0, factory0.Buffer(), factory0.BufferLength()) == 0);
    
    PacketFactory factory1(playConfig.flows[0].varMap, flowConfig.pktList[1], flowConfig.varList, flowModifiers);
    char exp1[] = {0x01, 0x00, 0x00, 0x00, 0x00, 
                   0xab, 0xcd, 
                   0x88,
                   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                   };
    CPPUNIT_ASSERT_EQUAL(sizeof(exp1), factory1.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp1, factory1.Buffer(), factory1.BufferLength()) == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestPacketFactory::testNonDefaults(void)
{
    FlowConfig flowConfig;
    typedef FlowConfig::Packet::VarIdxMap_t::value_type varMapValue_t;

    flowConfig.pktList.resize(2);
    flowConfig.pktList[0].data.resize(4, 0);
    flowConfig.pktList[0].data[0] = 0x12;
    flowConfig.pktList[0].data[1] = 0x34;
    flowConfig.pktList[0].data[2] = 0x56;
    flowConfig.pktList[0].data[3] = 0x78;
    flowConfig.pktList[1].data.resize(16, 0xff);

    flowConfig.varList.resize(3);
    // first variable, beginning of packet 0, default: 1 byte 0x99
    flowConfig.varList[0].value.resize(1, 0x99);
    flowConfig.varList[0].targetList.resize(1);
    flowConfig.varList[0].targetList[0].pktIdx = 0;
    flowConfig.varList[0].targetList[0].byteIdx = 0;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(0, 0));

    // second variable, end of packet 1, default: 5 bytes 0x01, 0x00, 0x00, 0x00, 0x00
    flowConfig.varList[1].value.resize(5, 0);
    flowConfig.varList[1].value[0] = 1;
    flowConfig.varList[1].targetList.resize(1);
    flowConfig.varList[1].targetList[0].pktIdx = 1;
    flowConfig.varList[1].targetList[0].byteIdx = 16;
    flowConfig.pktList[1].varMap.insert(varMapValue_t(16, 1));

    // third variable, after byte 1 of packet 0 AND packet 1, default: 2 bytes 0xab 0xcd
    flowConfig.varList[2].value.resize(2, 0xab);
    flowConfig.varList[2].value[1] = 0xcd;
    flowConfig.varList[2].targetList.resize(2);
    flowConfig.varList[2].targetList[0].pktIdx = 0;
    flowConfig.varList[2].targetList[0].byteIdx = 1;
    flowConfig.varList[2].targetList[1].pktIdx = 1;
    flowConfig.varList[2].targetList[1].byteIdx = 1;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(1, 2));
    flowConfig.pktList[1].varMap.insert(varMapValue_t(1, 2));

    PlaylistConfig playConfig0, playConfig1;
    playConfig0.flows.resize(1);
    playConfig1.flows.resize(1);
   
    // first variable, beginning of packet 0, set to 0x77 in playlist 0, 0x12 0x34 in playlist 1
    playConfig0.flows[0].varMap[0].resize(1, 0x77);
    playConfig1.flows[0].varMap[0].resize(2, 0x12);
    playConfig1.flows[0].varMap[0][1] = 0x34;

    // second variable, end of packet 1, set to 0x66 in playlist 0, leave at default (0x01, 0x00, 0x00, 0x00, 0x00) in playlist 1
    playConfig0.flows[0].varMap[1].resize(1, 0x66);

    // third variable, after byte 1 of packet 0 AND packet 1, set to 0xdede for playlist0, 0xf0f0 for playlist1
    playConfig0.flows[0].varMap[2].resize(2, 0xde);
    playConfig1.flows[0].varMap[2].resize(2, 0xf0);

    ModifierBlock modifiers0(playConfig0, 0 /* seed */);
    ModifierBlock modifiers1(playConfig1, 0 /* seed */);
    FlowModifierBlock flowModifiers0(modifiers0, 0 /* flowIdx */);
    FlowModifierBlock flowModifiers1(modifiers1, 0 /* flowIdx */);

    PacketFactory factory0_0(playConfig0.flows[0].varMap, flowConfig.pktList[0], flowConfig.varList, flowModifiers0);
    char exp0_0[] = {0x77, 0x12, 0xde, 0xde, 0x34, 0x56, 0x78};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp0_0), factory0_0.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp0_0, factory0_0.Buffer(), factory0_0.BufferLength()) == 0);
    
    PacketFactory factory0_1(playConfig0.flows[0].varMap, flowConfig.pktList[1], flowConfig.varList, flowModifiers0);
    char exp0_1[] = {0xff, 0xde, 0xde, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                   0x66};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp0_1), factory0_1.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp0_1, factory0_1.Buffer(), factory0_1.BufferLength()) == 0);

    PacketFactory factory1_0(playConfig1.flows[0].varMap, flowConfig.pktList[0], flowConfig.varList, flowModifiers1);
    char exp1_0[] = {0x12, 0x34, 0x12, 0xf0, 0xf0, 0x34, 0x56, 0x78};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp1_0), factory1_0.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp1_0, factory1_0.Buffer(), factory1_0.BufferLength()) == 0);
    
    PacketFactory factory1_1(playConfig1.flows[0].varMap, flowConfig.pktList[1], flowConfig.varList, flowModifiers1);
    char exp1_1[] = {0xff, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                   0x01, 0x00, 0x00, 0x00, 0x00};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp1_1), factory1_1.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp1_1, factory1_1.Buffer(), factory1_1.BufferLength()) == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestPacketFactory::testRangeModifiers(void)
{
    FlowConfig flowConfig;
    typedef FlowConfig::Packet::VarIdxMap_t::value_type varMapValue_t;

    flowConfig.pktList.resize(2);
    flowConfig.pktList[0].data.resize(4, 0);
    flowConfig.pktList[0].data[0] = 0x12;
    flowConfig.pktList[0].data[1] = 0x34;
    flowConfig.pktList[0].data[2] = 0x56;
    flowConfig.pktList[0].data[3] = 0x78;
    flowConfig.pktList[1].data.resize(16, 0xff);

    flowConfig.varList.resize(3);
    // first variable, beginning of packet 0, default: 1 byte 0x99
    flowConfig.varList[0].value.resize(1, 0x99);
    flowConfig.varList[0].targetList.resize(1);
    flowConfig.varList[0].targetList[0].pktIdx = 0;
    flowConfig.varList[0].targetList[0].byteIdx = 0;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(0, 0));

    // second variable, end of packet 1, default: 5 bytes 0x01, 0x00, 0x00, 0x00, 0x00
    flowConfig.varList[1].value.resize(5, 0);
    flowConfig.varList[1].value[0] = 1;
    flowConfig.varList[1].targetList.resize(1);
    flowConfig.varList[1].targetList[0].pktIdx = 1;
    flowConfig.varList[1].targetList[0].byteIdx = 16;
    flowConfig.pktList[1].varMap.insert(varMapValue_t(16, 1));

    // third variable, after byte 1 of packet 0 AND packet 1, default: 2 bytes 0xab 0xcd, LE (swapped)
    flowConfig.varList[2].value.resize(2, 0xab);
    flowConfig.varList[2].value[1] = 0xcd;
    flowConfig.varList[2].targetList.resize(2);
    flowConfig.varList[2].targetList[0].pktIdx = 0;
    flowConfig.varList[2].targetList[0].byteIdx = 1;
    flowConfig.varList[2].targetList[1].pktIdx = 1;
    flowConfig.varList[2].targetList[1].byteIdx = 1;
    flowConfig.varList[2].endian = FlowConfig::LITTLE;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(1, 2));
    flowConfig.pktList[1].varMap.insert(varMapValue_t(1, 2));

    PlaylistConfig playConfig0, playConfig1;
    playConfig0.flows.resize(1);
    playConfig1.flows.resize(1);

    std::vector<uint8_t> vec0001(2, 0x00); 
    vec0001[1] = 0x01;
    std::vector<uint8_t> vec1234(2, 0x12); 
    vec1234[1] = 0x34;
    std::vector<uint8_t> vecffff(2, 0xff); 
   
    // first variable, beginning of packet 0, modified to step up from 00 01 in playlist 0, down from 0x12 0x34 in playlist 1

    playConfig0.flows[0].rangeModMap[0].start = vec0001;
    playConfig0.flows[0].rangeModMap[0].step  = vec0001;
    playConfig0.flows[0].rangeModMap[0].mask  = vecffff;
    playConfig0.flows[0].rangeModMap[0].recycleCount = 4;
    playConfig0.flows[0].rangeModMap[0].stutterCount = 0;
    playConfig0.flows[0].rangeModMap[0].mode = PlaylistConfig::MOD_INCR;

    playConfig1.flows[0].rangeModMap[0].start = vec1234;
    playConfig1.flows[0].rangeModMap[0].step  = vec0001;
    playConfig1.flows[0].rangeModMap[0].mask  = vecffff;
    playConfig1.flows[0].rangeModMap[0].recycleCount = 4;
    playConfig1.flows[0].rangeModMap[0].stutterCount = 0;
    playConfig1.flows[0].rangeModMap[0].mode = PlaylistConfig::MOD_DECR;

    // second variable, end of packet 1, modified to step from 0x66, 0x67, etc. in playlist 0, leave at default (0x01, 0x00, 0x00, 0x00, 0x00) in playlist 1
    playConfig0.flows[0].rangeModMap[1].start.assign(1, 0x66);
    playConfig0.flows[0].rangeModMap[1].step.assign(1, 0x01);
    playConfig0.flows[0].rangeModMap[1].mask.assign(1, 0xff);
    playConfig0.flows[0].rangeModMap[1].recycleCount = 4;
    playConfig0.flows[0].rangeModMap[1].stutterCount = 0;
    playConfig0.flows[0].rangeModMap[1].mode = PlaylistConfig::MOD_INCR;

    // third variable, after byte 1 of packet 0 AND packet 1, set to step from 0xbeb0, 0xcec0, etc. for playlist0, step from 0xf0f1, 0xe0e1, etc. for playlist1
    playConfig0.flows[0].rangeModMap[2].start.assign(2, 0xbe);
    playConfig0.flows[0].rangeModMap[2].start[1] = 0xb0;
    playConfig0.flows[0].rangeModMap[2].step.assign(2, 0x10);
    playConfig0.flows[0].rangeModMap[2].mask.assign(2, 0xff);
    playConfig0.flows[0].rangeModMap[2].recycleCount = 4;
    playConfig0.flows[0].rangeModMap[2].stutterCount = 0;
    playConfig0.flows[0].rangeModMap[2].mode = PlaylistConfig::MOD_INCR;
    playConfig1.flows[0].rangeModMap[2].start.assign(2, 0xf0);
    playConfig1.flows[0].rangeModMap[2].start[1] = 0xf1;
    playConfig1.flows[0].rangeModMap[2].step.assign(2, 0x10);
    playConfig1.flows[0].rangeModMap[2].mask.assign(2, 0xff);
    playConfig1.flows[0].rangeModMap[2].recycleCount = 4;
    playConfig1.flows[0].rangeModMap[2].stutterCount = 0;
    playConfig1.flows[0].rangeModMap[2].mode = PlaylistConfig::MOD_DECR;

    ModifierBlock mods0(playConfig0, 0 /* seed */);
    FlowModifierBlock f_mods0(mods0, 0 /* flowIdx */);

    {
    // playlist 0, packet 0, initial variable values (00 01, b0 be)
    PacketFactory factory(playConfig0.flows[0].varMap, flowConfig.pktList[0], flowConfig.varList, f_mods0);
    char exp[] = {0x00, 0x01, 0x12, 0xb0, 0xbe, 0x34, 0x56, 0x78};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp), factory.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp, factory.Buffer(), factory.BufferLength()) == 0);
    }

    {
    // playlist 0, packet 1, initial variable values (66, b0 be)
    PacketFactory factory(playConfig0.flows[0].varMap, flowConfig.pktList[1], flowConfig.varList, f_mods0);
    char exp[] = {0xff, 0xb0, 0xbe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                  0x66};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp), factory.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp, factory.Buffer(), factory.BufferLength()) == 0);
    }

    mods0.Next();
    {
    // playlist 0, packet 0, next variable values (00 02, c0 ce)
    PacketFactory factory(playConfig0.flows[0].varMap, flowConfig.pktList[0], flowConfig.varList, f_mods0);
    char exp[] = {0x00, 0x02, 0x12, 0xc0, 0xce, 0x34, 0x56, 0x78};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp), factory.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp, factory.Buffer(), factory.BufferLength()) == 0);
    }
    
    {
    // playlist 0, packet 1, next variable values (67, c0 ce)
    PacketFactory factory(playConfig0.flows[0].varMap, flowConfig.pktList[1], flowConfig.varList, f_mods0);
    char exp[] = {0xff, 0xc0, 0xce, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                  0x67};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp), factory.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp, factory.Buffer(), factory.BufferLength()) == 0);
    }

    ModifierBlock mods1(playConfig1, 0 /* seed */);
    FlowModifierBlock f_mods1(mods1, 0 /* flowIdx */);

    {
    // playlist 1, packet 0, initial variable values (12 34, f1 f0)
    PacketFactory factory(playConfig1.flows[0].varMap, flowConfig.pktList[0], flowConfig.varList, f_mods1);
    char exp[] = {0x12, 0x34, 0x12, 0xf1, 0xf0, 0x34, 0x56, 0x78};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp), factory.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp, factory.Buffer(), factory.BufferLength()) == 0);
    }

    {
    // playlist 1, packet 1, initial variable values (01 00 00 00 00<def>, f0 f0)
    PacketFactory factory(playConfig1.flows[0].varMap, flowConfig.pktList[1], flowConfig.varList, f_mods1);
    char exp[] = {0xff, 0xf1, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                  0x01, 0x00, 0x00, 0x00, 0x00};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp), factory.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp, factory.Buffer(), factory.BufferLength()) == 0);
    }

    mods1.Next();
    {
    // playlist 1, packet 0, next variable values (12 33, e1 e0)
    PacketFactory factory(playConfig1.flows[0].varMap, flowConfig.pktList[0], flowConfig.varList, f_mods1);
    char exp[] = {0x12, 0x33, 0x12, 0xe1, 0xe0, 0x34, 0x56, 0x78};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp), factory.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp, factory.Buffer(), factory.BufferLength()) == 0);
    }
    
    {
    // playlist 1, packet 1, next variable values (01 00 00 00 00<def>, e1 e0)
    PacketFactory factory(playConfig1.flows[0].varMap, flowConfig.pktList[1], flowConfig.varList, f_mods1);
    char exp[] = {0xff, 0xe1, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                  0x01, 0x00, 0x00, 0x00, 0x00};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp), factory.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp, factory.Buffer(), factory.BufferLength()) == 0);
    }
}

///////////////////////////////////////////////////////////////////////////////

void TestPacketFactory::testMaskedModifiers(void)
{
    FlowConfig flowConfig;
    typedef FlowConfig::Packet::VarIdxMap_t::value_type varMapValue_t;

    flowConfig.pktList.resize(2);
    flowConfig.pktList[0].data.resize(8, 0);
    flowConfig.pktList[0].data[0] = 0x01;
    flowConfig.pktList[0].data[1] = 0x02;
    flowConfig.pktList[0].data[2] = 0x03;
    flowConfig.pktList[0].data[3] = 0x04;
    flowConfig.pktList[0].data[4] = 0x05;
    flowConfig.pktList[0].data[5] = 0x06;
    flowConfig.pktList[0].data[6] = 0x07;
    flowConfig.pktList[0].data[7] = 0x08;

    flowConfig.varList.resize(3);
    // first variable, beginning of packet 0, default: 2 bytes 0x22 0x33
    flowConfig.varList[0].value.resize(2, 0x22);
    flowConfig.varList[0].value[1] = 0x33;
    flowConfig.varList[0].targetList.resize(1);
    flowConfig.varList[0].targetList[0].pktIdx = 0;
    flowConfig.varList[0].targetList[0].byteIdx = 0;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(0, 0));

    // second variable, after byte 4 of packet 0, default: 4 bytes 0xaa 0xbb 0xcc 0xdd
    flowConfig.varList[1].value.resize(4, 0xaa);
    flowConfig.varList[1].value[1] = 0xbb;
    flowConfig.varList[1].value[2] = 0xcc;
    flowConfig.varList[1].value[3] = 0xdd;
    flowConfig.varList[1].targetList.resize(1);
    flowConfig.varList[1].targetList[0].pktIdx = 0;
    flowConfig.varList[1].targetList[0].byteIdx = 4;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(4, 1));

    // third variable, end of packet 0, default: 1 byte 0xee
    flowConfig.varList[2].value.resize(1, 0xee);
    flowConfig.varList[2].targetList.resize(1);
    flowConfig.varList[2].targetList[0].pktIdx = 0;
    flowConfig.varList[2].targetList[0].byteIdx = 8;
    flowConfig.pktList[0].varMap.insert(varMapValue_t(8, 2));

    // so, before any modifiers the packet would be:
    // [22 33] 01 02 03 04 [aa bb cc dd] 05 06 07 08 [ee]

    PlaylistConfig playConfig0;
    playConfig0.flows.resize(1);

    std::vector<uint8_t> vec0001(2, 0x00); vec0001[1] = 0x01;
   
    // first variable, beginning of packet 0, modified to step up from 0xaaaa, mask 0x0f0f

    playConfig0.flows[0].rangeModMap[0].start.assign(2, 0xaa);
    playConfig0.flows[0].rangeModMap[0].step  = vec0001;
    playConfig0.flows[0].rangeModMap[0].mask.assign(2, 0x0f);
    playConfig0.flows[0].rangeModMap[0].recycleCount = 4;
    playConfig0.flows[0].rangeModMap[0].stutterCount = 0;
    playConfig0.flows[0].rangeModMap[0].mode = PlaylistConfig::MOD_INCR;

    // second variable, after byte 3 of packet 0, modified to step up from 0x11, mask 0x0f (note size mismatch - should be supported)
    playConfig0.flows[0].rangeModMap[1].start.assign(1, 0x11);
    playConfig0.flows[0].rangeModMap[1].step.assign(1, 0x01);
    playConfig0.flows[0].rangeModMap[1].mask.assign(1, 0x0f);
    playConfig0.flows[0].rangeModMap[1].recycleCount = 4;
    playConfig0.flows[0].rangeModMap[1].stutterCount = 0;
    playConfig0.flows[0].rangeModMap[1].mode = PlaylistConfig::MOD_INCR;

    // third variable, end of packet 0, modified to step up from 0x2222, mask 0xf0f0 (note size mismatch - should be supported)
    playConfig0.flows[0].rangeModMap[2].start.assign(2, 0x22);
    playConfig0.flows[0].rangeModMap[2].step = vec0001;
    playConfig0.flows[0].rangeModMap[2].mask.assign(2, 0xf0);
    playConfig0.flows[0].rangeModMap[2].recycleCount = 4;
    playConfig0.flows[0].rangeModMap[2].stutterCount = 0;
    playConfig0.flows[0].rangeModMap[2].mode = PlaylistConfig::MOD_INCR;

    ModifierBlock mods0(playConfig0, 0 /* seed */);
    FlowModifierBlock f_mods0(mods0, 0 /* flowIdx */);

    {
    // playlist 0, packet 0, initial variable values
    // aa aa (^0f 0f), 11 (^0f), 22 22 (^f0 f0)
    PacketFactory factory(playConfig0.flows[0].varMap, flowConfig.pktList[0], flowConfig.varList, f_mods0);
    char exp[] = {0x2a, 0x3a, 0x01, 0x02, 0x03, 0x04, 0xa1, 0x05, 0x06, 0x07, 0x08, 0x2e, 0x20};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp), factory.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp, factory.Buffer(), factory.BufferLength()) == 0);
    }

    mods0.Next();
    {
    // playlist 0, packet 0, next variable values
    // aa ab (^0f 0f), 12 (^0f), 22 23 (^f0 f0)
    PacketFactory factory(playConfig0.flows[0].varMap, flowConfig.pktList[0], flowConfig.varList, f_mods0);
    char exp[] = {0x2a, 0x3b, 0x01, 0x02, 0x03, 0x04, 0xa2, 0x05, 0x06, 0x07, 0x08, 0x2e, 0x20};
    CPPUNIT_ASSERT_EQUAL(sizeof(exp), factory.BufferLength());
    CPPUNIT_ASSERT(memcmp(exp, factory.Buffer(), factory.BufferLength()) == 0);
    }
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestPacketFactory);
