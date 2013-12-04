#include <boost/foreach.hpp>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <engine/PlaylistConfig.h>

#include "PlaylistConfigProxy.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestPlaylistConfigProxy : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestPlaylistConfigProxy);
    CPPUNIT_TEST(testStream);
    CPPUNIT_TEST(testAttack);
    CPPUNIT_TEST(testLoops);
    CPPUNIT_TEST(testVariables);
    CPPUNIT_TEST(testRangeModifiers);
    CPPUNIT_TEST(testRandomModifiers);
    CPPUNIT_TEST(testTableModifiers);
    CPPUNIT_TEST(testLinkedModifiers);
    CPPUNIT_TEST_SUITE_END();

public:
    TestPlaylistConfigProxy()
    {
    }

    ~TestPlaylistConfigProxy()
    {
    }

    void setUp() 
    { 
    }

    void tearDown() 
    { 
    }
    
protected:
    void testStream();
    void testAttack();
    void testLoops();
    void testVariables();
    void testRangeModifiers();
    void testRandomModifiers();
    void testTableModifiers();
    void testLinkedModifiers();

    void makeStreamsValid(PlaylistConfigProxy::playlistConfig_t& config);

private:
};

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistConfigProxy::testStream()
{
    PlaylistConfigProxy::playlistConfig_t config;
    config.Streams.resize(3);
    Dpg_1_port_server::DpgStreamReference_t& stream0(config.Streams[0]);
    Dpg_1_port_server::DpgStreamReference_t_clear(&stream0);
    stream0.Handle = 1;
    stream0.StreamParam.StartTime = 2;
    stream0.StreamParam.DataDelay = 3;
    stream0.FlowParam.ServerPort = 4;
    stream0.StreamParam.MinPktDelay = 5;
    stream0.StreamParam.MaxPktDelay = 6;
    Dpg_1_port_server::DpgStreamReference_t& stream1(config.Streams[1]);
    Dpg_1_port_server::DpgStreamReference_t_clear(&stream1);
    stream1.Handle = 7;
    stream1.StreamParam.StartTime = 8;
    stream1.StreamParam.DataDelay = 9;
    stream1.FlowParam.ServerPort = 10;
    stream1.StreamParam.MinPktDelay = 11;
    stream1.StreamParam.MaxPktDelay = 12;
    Dpg_1_port_server::DpgStreamReference_t& stream2(config.Streams[2]);
    Dpg_1_port_server::DpgStreamReference_t_clear(&stream2);
    stream2.Handle = 13;
    stream2.StreamParam.StartTime = 14;
    stream2.StreamParam.DataDelay = 15;
    stream2.FlowParam.ServerPort = 16;
    stream2.StreamParam.MinPktDelay = 17;
    stream2.StreamParam.MaxPktDelay = 18;

    PlaylistConfigProxy proxy(config);

    L4L7_ENGINE_NS::PlaylistConfig copy;
    // add some bogus data to make sure the proxy clears it out
    copy.flows.resize(10);
    memset(&copy.flows[0], 0xff, 31);
   
    proxy.CopyTo(copy); 

    CPPUNIT_ASSERT_EQUAL(size_t(3), copy.flows.size());

    CPPUNIT_ASSERT_EQUAL(stream0.Handle,      copy.flows[0].flowHandle);
    CPPUNIT_ASSERT_EQUAL(stream0.StreamParam.StartTime,   copy.flows[0].startTime);
    CPPUNIT_ASSERT_EQUAL(stream0.StreamParam.DataDelay,   copy.flows[0].dataDelay);
    CPPUNIT_ASSERT_EQUAL(stream0.FlowParam.ServerPort,  copy.flows[0].serverPort);
    CPPUNIT_ASSERT_EQUAL(stream0.StreamParam.MinPktDelay, copy.flows[0].minPktDelay);
    CPPUNIT_ASSERT_EQUAL(stream0.StreamParam.MaxPktDelay, copy.flows[0].maxPktDelay);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), copy.flows[0].socketTimeout);
    CPPUNIT_ASSERT_EQUAL(false, copy.flows[0].reversed);
    CPPUNIT_ASSERT_EQUAL(stream1.Handle,      copy.flows[1].flowHandle);
    CPPUNIT_ASSERT_EQUAL(stream1.StreamParam.StartTime,   copy.flows[1].startTime);
    CPPUNIT_ASSERT_EQUAL(stream1.StreamParam.DataDelay,   copy.flows[1].dataDelay);
    CPPUNIT_ASSERT_EQUAL(stream1.FlowParam.ServerPort,  copy.flows[1].serverPort);
    CPPUNIT_ASSERT_EQUAL(stream1.StreamParam.MinPktDelay, copy.flows[1].minPktDelay);
    CPPUNIT_ASSERT_EQUAL(stream1.StreamParam.MaxPktDelay, copy.flows[1].maxPktDelay);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), copy.flows[1].socketTimeout);
    CPPUNIT_ASSERT_EQUAL(false, copy.flows[1].reversed);
    CPPUNIT_ASSERT_EQUAL(stream2.Handle,      copy.flows[2].flowHandle);
    CPPUNIT_ASSERT_EQUAL(stream2.StreamParam.StartTime,   copy.flows[2].startTime);
    CPPUNIT_ASSERT_EQUAL(stream2.StreamParam.DataDelay,   copy.flows[2].dataDelay);
    CPPUNIT_ASSERT_EQUAL(stream2.FlowParam.ServerPort,  copy.flows[2].serverPort);
    CPPUNIT_ASSERT_EQUAL(stream2.StreamParam.MinPktDelay, copy.flows[2].minPktDelay);
    CPPUNIT_ASSERT_EQUAL(stream2.StreamParam.MaxPktDelay, copy.flows[2].maxPktDelay);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), copy.flows[2].socketTimeout);
    CPPUNIT_ASSERT_EQUAL(false, copy.flows[2].reversed);

    // invalid streams

    // unsupported close type C_RST
    stream1.StreamParam.CloseType = DpgIf_1_port_server::CLOSE_TYPE_C_RST;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy2(config), EPHXBadConfig);

    // back to valid
    stream1.StreamParam.CloseType = DpgIf_1_port_server::CLOSE_TYPE_C_FIN;
    PlaylistConfigProxy proxy2b(config);

    // unsupported close type S_RST
    stream2.StreamParam.CloseType = DpgIf_1_port_server::CLOSE_TYPE_S_RST;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy3(config), EPHXBadConfig);

    // back to valid
    stream2.StreamParam.CloseType = DpgIf_1_port_server::CLOSE_TYPE_S_FIN;
    PlaylistConfigProxy proxy3b(config);
   
    // zero server port
    stream0.FlowParam.ServerPort = 0;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy4(config), EPHXBadConfig);

    // back to valid
    stream0.FlowParam.ServerPort = 4;
    PlaylistConfigProxy proxy4b(config);

    // min > max
    stream1.StreamParam.MinPktDelay = 2;
    stream1.StreamParam.MaxPktDelay = 1;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy5(config), EPHXBadConfig);
   
    // back to valid
    stream1.StreamParam.MinPktDelay = 11;
    stream1.StreamParam.MaxPktDelay = 12;
    PlaylistConfigProxy proxy5b(config);

    // mix of stream/attacks 
    config.Attacks.resize(1);
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy6(config), EPHXBadConfig);
}


///////////////////////////////////////////////////////////////////////////////

void TestPlaylistConfigProxy::testAttack()
{
    PlaylistConfigProxy::playlistConfig_t config;
    config.Attacks.resize(3);
    Dpg_1_port_server::DpgAttackReference_t& attack0(config.Attacks[0]);
    attack0.Handle = 1;
    attack0.AttackParam.PktDelay = 2;
    attack0.AttackParam.InactivityTimeout = 3000;
    attack0.FlowParam.ServerPort = 4;
    Dpg_1_port_server::DpgAttackReference_t& attack1(config.Attacks[1]);
    attack1.Handle = 5;
    attack1.AttackParam.PktDelay = 6;
    attack1.AttackParam.InactivityTimeout = 7000;
    attack1.FlowParam.ServerPort = 8;
    Dpg_1_port_server::DpgAttackReference_t& attack2(config.Attacks[2]);
    attack2.Handle = 5;
    attack2.AttackParam.PktDelay = 6;
    attack2.AttackParam.InactivityTimeout = 7000;
    attack2.FlowParam.ServerPort = 8;

    PlaylistConfigProxy proxy(config);

    L4L7_ENGINE_NS::PlaylistConfig copy;
    // add some bogus data to make sure the proxy clears it out
    copy.flows.resize(10);
    memset(&copy.flows[0], 0xff, 31);

    proxy.CopyTo(copy); 

    CPPUNIT_ASSERT_EQUAL(size_t(3), copy.flows.size());

    CPPUNIT_ASSERT_EQUAL(attack0.Handle,      copy.flows[0].flowHandle);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0),         copy.flows[0].startTime);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0),         copy.flows[0].dataDelay);
    CPPUNIT_ASSERT_EQUAL(attack0.FlowParam.ServerPort,  copy.flows[0].serverPort);
    CPPUNIT_ASSERT_EQUAL(attack0.AttackParam.PktDelay,    copy.flows[0].minPktDelay);
    CPPUNIT_ASSERT_EQUAL(attack0.AttackParam.PktDelay,    copy.flows[0].maxPktDelay);
    CPPUNIT_ASSERT_EQUAL(attack0.AttackParam.InactivityTimeout,  copy.flows[0].socketTimeout);
    CPPUNIT_ASSERT_EQUAL(false, copy.flows[0].reversed);
    CPPUNIT_ASSERT_EQUAL(attack1.Handle,      copy.flows[1].flowHandle);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0),         copy.flows[1].startTime);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0),         copy.flows[1].dataDelay);
    CPPUNIT_ASSERT_EQUAL(attack1.FlowParam.ServerPort,  copy.flows[1].serverPort);
    CPPUNIT_ASSERT_EQUAL(attack1.AttackParam.PktDelay,    copy.flows[1].minPktDelay);
    CPPUNIT_ASSERT_EQUAL(attack1.AttackParam.PktDelay,    copy.flows[1].maxPktDelay);
    CPPUNIT_ASSERT_EQUAL(attack1.AttackParam.InactivityTimeout,  copy.flows[1].socketTimeout);
    CPPUNIT_ASSERT_EQUAL(false, copy.flows[1].reversed);
    CPPUNIT_ASSERT_EQUAL(attack2.Handle,      copy.flows[2].flowHandle);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0),         copy.flows[2].startTime);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0),         copy.flows[2].dataDelay);
    CPPUNIT_ASSERT_EQUAL(attack2.FlowParam.ServerPort,  copy.flows[2].serverPort);
    CPPUNIT_ASSERT_EQUAL(attack2.AttackParam.PktDelay,    copy.flows[2].minPktDelay);
    CPPUNIT_ASSERT_EQUAL(attack2.AttackParam.PktDelay,    copy.flows[2].maxPktDelay);
    CPPUNIT_ASSERT_EQUAL(attack2.AttackParam.InactivityTimeout,  copy.flows[2].socketTimeout);
    CPPUNIT_ASSERT_EQUAL(false, copy.flows[2].reversed);

    // invalid attacks
    
    // zero server port
    attack0.FlowParam.ServerPort = 0;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy2(config), EPHXBadConfig);

    // back to valid
    attack0.FlowParam.ServerPort = 4;
    PlaylistConfigProxy proxy2b(config);
   
    // mix of stream/attacks 
    config.Streams.resize(1);
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy3(config), EPHXBadConfig);
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistConfigProxy::testLoops()
{
    PlaylistConfigProxy::playlistConfig_t config;

    config.Streams.resize(4);
    makeStreamsValid(config);

    config.Loops.resize(2); 
    config.Loops[0].BegIdx = 0;
    config.Loops[0].EndIdx = 1;
    config.Loops[0].Count = 10;
    config.Loops[1].BegIdx = 3;
    config.Loops[1].EndIdx = 3;
    config.Loops[1].Count = 100;

    PlaylistConfigProxy proxy(config);
    L4L7_ENGINE_NS::PlaylistConfig copy;
    proxy.CopyTo(copy); 
    CPPUNIT_ASSERT_EQUAL(size_t(2), copy.loopMap.size());

    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.loopMap.count(1)); // map is by endidx
    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.loopMap.count(3));

    CPPUNIT_ASSERT_EQUAL(uint16_t(0), copy.loopMap[1].begIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(10), copy.loopMap[1].count);

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), copy.loopMap[3].begIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(100), copy.loopMap[3].count);

    // invalid loop configs

    // overlapping
    config.Loops[0].BegIdx = 0;
    config.Loops[0].EndIdx = 1;
    config.Loops[1].BegIdx = 1;
    config.Loops[1].EndIdx = 3;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy2(config), EPHXBadConfig);

    config.Loops[0].BegIdx = 0;
    config.Loops[0].EndIdx = 1;
    config.Loops[1].BegIdx = 0;
    config.Loops[1].EndIdx = 3;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy3(config), EPHXBadConfig);
    
    // out of range idx
    config.Loops[0].BegIdx = 0;
    config.Loops[0].EndIdx = 1;
    config.Loops[1].BegIdx = 2;
    config.Loops[1].EndIdx = 4;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy4(config), EPHXBadConfig);

    config.Loops[0].BegIdx = 0;
    config.Loops[0].EndIdx = 1;
    config.Loops[1].BegIdx = 4;
    config.Loops[1].EndIdx = 5;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy5(config), EPHXBadConfig);

    // reversed beg/end
    config.Loops[0].BegIdx = 0;
    config.Loops[0].EndIdx = 1;
    config.Loops[1].BegIdx = 3;
    config.Loops[1].EndIdx = 2;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy6(config), EPHXBadConfig);

    config.Loops[0].BegIdx = 1;
    config.Loops[0].EndIdx = 0;
    config.Loops[1].BegIdx = 2;
    config.Loops[1].EndIdx = 3;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy7(config), EPHXBadConfig);

    // out of order loops
    config.Loops[0].BegIdx = 2;
    config.Loops[0].EndIdx = 3;
    config.Loops[1].BegIdx = 0;
    config.Loops[1].EndIdx = 1;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy8(config), EPHXBadConfig);
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistConfigProxy::testVariables()
{
    PlaylistConfigProxy::playlistConfig_t config;

    config.Streams.resize(2);
    makeStreamsValid(config);

    config.Streams[0].Vars.resize(2);
    config.Streams[0].Vars[0].VarIdx = 1;
    config.Streams[0].Vars[0].Value.resize(3, 0xff);
    config.Streams[0].Vars[1].VarIdx = 2;
    config.Streams[0].Vars[1].Value.resize(1, 1);
    config.Streams[1].Vars.resize(1);
    config.Streams[1].Vars[0].VarIdx = 99;
    config.Streams[1].Vars[0].Value.resize(2, 0x12);

    PlaylistConfigProxy proxy(config);
    L4L7_ENGINE_NS::PlaylistConfig copy;
    proxy.CopyTo(copy); 
    CPPUNIT_ASSERT_EQUAL(size_t(2), copy.flows.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), copy.flows[0].varMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[0].varMap.count(config.Streams[0].Vars[0].VarIdx));
    CPPUNIT_ASSERT(config.Streams[0].Vars[0].Value == copy.flows[0].varMap[1]);
    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[0].varMap.count(config.Streams[0].Vars[1].VarIdx));
    CPPUNIT_ASSERT(config.Streams[0].Vars[1].Value == copy.flows[0].varMap[2]);

    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[1].varMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[1].varMap.count(config.Streams[1].Vars[0].VarIdx));
    CPPUNIT_ASSERT(config.Streams[1].Vars[0].Value == copy.flows[1].varMap[99]);

    config.Streams.resize(0);
    config.Attacks.resize(2);
    makeStreamsValid(config);
    config.Attacks[0].Vars.resize(2);
    config.Attacks[0].Vars[0].VarIdx = 1;
    config.Attacks[0].Vars[0].Value.resize(3, 0xff);
    config.Attacks[0].Vars[1].VarIdx = 2;
    config.Attacks[0].Vars[1].Value.resize(1, 1);
    config.Attacks[1].Vars.resize(1);
    config.Attacks[1].Vars[0].VarIdx = 99;
    config.Attacks[1].Vars[0].Value.resize(2, 0x12);

    PlaylistConfigProxy proxy2(config);
    L4L7_ENGINE_NS::PlaylistConfig copy2;
    proxy2.CopyTo(copy2); 
    CPPUNIT_ASSERT_EQUAL(size_t(2), copy2.flows.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), copy2.flows[0].varMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), copy2.flows[0].varMap.count(config.Attacks[0].Vars[0].VarIdx));
    CPPUNIT_ASSERT(config.Attacks[0].Vars[0].Value == copy2.flows[0].varMap[1]);
    CPPUNIT_ASSERT_EQUAL(size_t(1), copy2.flows[0].varMap.count(config.Attacks[0].Vars[1].VarIdx));
    CPPUNIT_ASSERT(config.Attacks[0].Vars[1].Value == copy2.flows[0].varMap[2]);

    CPPUNIT_ASSERT_EQUAL(size_t(1), copy2.flows[1].varMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), copy2.flows[1].varMap.count(config.Attacks[1].Vars[0].VarIdx));
    CPPUNIT_ASSERT(config.Attacks[1].Vars[0].Value == copy2.flows[1].varMap[99]);

    // invalid var configs

    // varIdx out of order
    config.Attacks[0].Vars[0].VarIdx = 3;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy3(config), EPHXBadConfig);

    // back to valid
    config.Attacks[0].Vars[0].VarIdx = 1;
    PlaylistConfigProxy proxy3b(config);

    // duplicate varIdx
    config.Attacks[0].Vars[1].VarIdx = 1;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy4(config), EPHXBadConfig);

    // back to valid
    config.Attacks[0].Vars[1].VarIdx = 2;
    PlaylistConfigProxy proxy4b(config);
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistConfigProxy::testRandomModifiers()
{
    PlaylistConfigProxy::playlistConfig_t config;
    config.Streams.resize(2);
    makeStreamsValid(config);
    config.Streams[0].RangeMods.resize(2);
    config.Streams[0].RangeMods[0].modIndex = 1;
    config.Streams[0].RangeMods[0].initialValue.data.resize(4, 1);
    config.Streams[0].RangeMods[0].mask.data.resize(2, 2);
    config.Streams[0].RangeMods[0].recycleCount = 3;
    config.Streams[0].RangeMods[0].stutterCount = 4;
    config.Streams[0].RangeMods[0].mode = Modifiers_1_port_server::CONFIG_MOD_MODE_RANDOM;
    config.Streams[0].RangeMods[1].modIndex = 2;
    config.Streams[0].RangeMods[1].initialValue.data.resize(4, 2);
    config.Streams[0].RangeMods[1].mask.data.resize(3, 3);
    config.Streams[0].RangeMods[1].recycleCount = 2;
    config.Streams[0].RangeMods[1].stutterCount = 2;
    config.Streams[0].RangeMods[1].mode = Modifiers_1_port_server::CONFIG_MOD_MODE_RANDOM;
    config.Streams[1].RangeMods.resize(1);
    config.Streams[1].RangeMods[0].modIndex = 99;
    config.Streams[1].RangeMods[0].initialValue.data.resize(4, 100);
    config.Streams[1].RangeMods[0].mask.data.resize(1, 100);
    config.Streams[1].RangeMods[0].recycleCount = 101;
    config.Streams[1].RangeMods[0].stutterCount = 102;
    config.Streams[1].RangeMods[0].mode = Modifiers_1_port_server::CONFIG_MOD_MODE_RANDOM;
    
    PlaylistConfigProxy proxy(config);
    L4L7_ENGINE_NS::PlaylistConfig copy;

    // add some bogus data to make sure the proxy clears it out
    copy.flows.resize(10);
    memset(&copy.flows[0], 0xff, 31);

    proxy.CopyTo(copy); 

    CPPUNIT_ASSERT_EQUAL(size_t(2), copy.flows.size());

    CPPUNIT_ASSERT_EQUAL(size_t(2), copy.flows[0].rangeModMap.size());
    Dpg_1_port_server::rangeModifier_t& rangeMod00(config.Streams[0].RangeMods[0]);

    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[0].rangeModMap.count(rangeMod00.modIndex));
    CPPUNIT_ASSERT(rangeMod00.initialValue.data == copy.flows[0].rangeModMap[rangeMod00.modIndex].start);
    CPPUNIT_ASSERT(rangeMod00.stepValue.data == copy.flows[0].rangeModMap[rangeMod00.modIndex].step);
    CPPUNIT_ASSERT(rangeMod00.mask.data == copy.flows[0].rangeModMap[rangeMod00.modIndex].mask);
    CPPUNIT_ASSERT_EQUAL(rangeMod00.recycleCount, copy.flows[0].rangeModMap[rangeMod00.modIndex].recycleCount);
    CPPUNIT_ASSERT_EQUAL(rangeMod00.stutterCount, copy.flows[0].rangeModMap[rangeMod00.modIndex].stutterCount);
    CPPUNIT_ASSERT_EQUAL(int(rangeMod00.mode), int(copy.flows[0].rangeModMap[rangeMod00.modIndex].mode));

    Dpg_1_port_server::rangeModifier_t& rangeMod01(config.Streams[0].RangeMods[1]);

    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[0].rangeModMap.count(rangeMod01.modIndex));
    CPPUNIT_ASSERT(rangeMod01.initialValue.data == copy.flows[0].rangeModMap[rangeMod01.modIndex].start);
    CPPUNIT_ASSERT(rangeMod01.stepValue.data == copy.flows[0].rangeModMap[rangeMod01.modIndex].step);
    CPPUNIT_ASSERT(rangeMod01.mask.data == copy.flows[0].rangeModMap[rangeMod01.modIndex].mask);
    CPPUNIT_ASSERT_EQUAL(rangeMod01.recycleCount, copy.flows[0].rangeModMap[rangeMod01.modIndex].recycleCount);
    CPPUNIT_ASSERT_EQUAL(rangeMod01.stutterCount, copy.flows[0].rangeModMap[rangeMod01.modIndex].stutterCount);
    CPPUNIT_ASSERT_EQUAL(int(rangeMod01.mode), int(copy.flows[0].rangeModMap[rangeMod01.modIndex].mode));

    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[1].rangeModMap.size());
    Dpg_1_port_server::rangeModifier_t& rangeMod10(config.Streams[1].RangeMods[0]);

    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[1].rangeModMap.count(rangeMod10.modIndex));
    CPPUNIT_ASSERT(rangeMod10.initialValue.data == copy.flows[1].rangeModMap[rangeMod10.modIndex].start);
    CPPUNIT_ASSERT(rangeMod10.stepValue.data == copy.flows[1].rangeModMap[rangeMod10.modIndex].step);
    CPPUNIT_ASSERT(rangeMod10.mask.data == copy.flows[1].rangeModMap[rangeMod10.modIndex].mask);
    CPPUNIT_ASSERT_EQUAL(rangeMod10.recycleCount, copy.flows[1].rangeModMap[rangeMod10.modIndex].recycleCount);
    CPPUNIT_ASSERT_EQUAL(rangeMod10.stutterCount, copy.flows[1].rangeModMap[rangeMod10.modIndex].stutterCount);
    CPPUNIT_ASSERT_EQUAL(int(rangeMod10.mode), int(copy.flows[1].rangeModMap[rangeMod10.modIndex].mode));

    // invalid modifier configs

    // start (seed) size mismatch
    config.Streams[0].RangeMods[0].initialValue.data.resize(3, 1);
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy2(config), EPHXBadConfig);

    // fix it
    config.Streams[0].RangeMods[0].initialValue.data.resize(4, 1);
    PlaylistConfigProxy proxy2b(config);

    // step size mismatch
    config.Streams[0].RangeMods[0].stepValue.data.resize(1, 1);
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy3(config), EPHXBadConfig);

    // fix it
    config.Streams[0].RangeMods[0].stepValue.data.clear();
    PlaylistConfigProxy proxy3b(config);

    // variable index out of order
    config.Streams[0].RangeMods[0].modIndex = 5;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy5(config), EPHXBadConfig);

    // fix it
    config.Streams[0].RangeMods[0].modIndex = 1;
    PlaylistConfigProxy proxy5b(config);
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistConfigProxy::testRangeModifiers()
{
    PlaylistConfigProxy::playlistConfig_t config;
    config.Streams.resize(2);
    makeStreamsValid(config);
    config.Streams[0].RangeMods.resize(2);
    config.Streams[0].RangeMods[0].modIndex = 1;
    config.Streams[0].RangeMods[0].initialValue.data.resize(4, 1);
    config.Streams[0].RangeMods[0].stepValue.data.resize(4, 2);
    config.Streams[0].RangeMods[0].mask.data.resize(4, 3);
    config.Streams[0].RangeMods[0].recycleCount = 4;
    config.Streams[0].RangeMods[0].stutterCount = 5;
    config.Streams[0].RangeMods[0].mode = Modifiers_1_port_server::CONFIG_MOD_MODE_INCREMENT;
    config.Streams[0].RangeMods[1].modIndex = 2;
    config.Streams[0].RangeMods[1].initialValue.data.resize(2, 1);
    config.Streams[0].RangeMods[1].stepValue.data.resize(2, 2);
    config.Streams[0].RangeMods[1].mask.data.resize(2, 3);
    config.Streams[0].RangeMods[1].recycleCount = 2;
    config.Streams[0].RangeMods[1].stutterCount = 2;
    config.Streams[0].RangeMods[1].mode = Modifiers_1_port_server::CONFIG_MOD_MODE_DECREMENT;
    config.Streams[1].RangeMods.resize(1);
    config.Streams[1].RangeMods[0].modIndex = 99;
    config.Streams[1].RangeMods[0].initialValue.data.resize(1, 100);
    config.Streams[1].RangeMods[0].stepValue.data.resize(1, 100);
    config.Streams[1].RangeMods[0].mask.data.resize(1, 100);
    config.Streams[1].RangeMods[0].recycleCount = 101;
    config.Streams[1].RangeMods[0].stutterCount = 102;
    config.Streams[1].RangeMods[0].mode = Modifiers_1_port_server::CONFIG_MOD_MODE_INCREMENT;
    
    PlaylistConfigProxy proxy(config);
    L4L7_ENGINE_NS::PlaylistConfig copy;

    // add some bogus data to make sure the proxy clears it out
    copy.flows.resize(10);
    memset(&copy.flows[0], 0xff, 31);

    proxy.CopyTo(copy); 

    CPPUNIT_ASSERT_EQUAL(size_t(2), copy.flows.size());

    CPPUNIT_ASSERT_EQUAL(size_t(2), copy.flows[0].rangeModMap.size());
    Dpg_1_port_server::rangeModifier_t& rangeMod00(config.Streams[0].RangeMods[0]);

    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[0].rangeModMap.count(rangeMod00.modIndex));
    CPPUNIT_ASSERT(rangeMod00.initialValue.data == copy.flows[0].rangeModMap[rangeMod00.modIndex].start);
    CPPUNIT_ASSERT(rangeMod00.stepValue.data == copy.flows[0].rangeModMap[rangeMod00.modIndex].step);
    CPPUNIT_ASSERT(rangeMod00.mask.data == copy.flows[0].rangeModMap[rangeMod00.modIndex].mask);
    CPPUNIT_ASSERT_EQUAL(rangeMod00.recycleCount, copy.flows[0].rangeModMap[rangeMod00.modIndex].recycleCount);
    CPPUNIT_ASSERT_EQUAL(rangeMod00.stutterCount, copy.flows[0].rangeModMap[rangeMod00.modIndex].stutterCount);
    CPPUNIT_ASSERT_EQUAL(int(rangeMod00.mode), int(copy.flows[0].rangeModMap[rangeMod00.modIndex].mode));

    Dpg_1_port_server::rangeModifier_t& rangeMod01(config.Streams[0].RangeMods[1]);

    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[0].rangeModMap.count(rangeMod01.modIndex));
    CPPUNIT_ASSERT(rangeMod01.initialValue.data == copy.flows[0].rangeModMap[rangeMod01.modIndex].start);
    CPPUNIT_ASSERT(rangeMod01.stepValue.data == copy.flows[0].rangeModMap[rangeMod01.modIndex].step);
    CPPUNIT_ASSERT(rangeMod01.mask.data == copy.flows[0].rangeModMap[rangeMod01.modIndex].mask);
    CPPUNIT_ASSERT_EQUAL(rangeMod01.recycleCount, copy.flows[0].rangeModMap[rangeMod01.modIndex].recycleCount);
    CPPUNIT_ASSERT_EQUAL(rangeMod01.stutterCount, copy.flows[0].rangeModMap[rangeMod01.modIndex].stutterCount);
    CPPUNIT_ASSERT_EQUAL(int(rangeMod01.mode), int(copy.flows[0].rangeModMap[rangeMod01.modIndex].mode));

    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[1].rangeModMap.size());
    Dpg_1_port_server::rangeModifier_t& rangeMod10(config.Streams[1].RangeMods[0]);

    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[1].rangeModMap.count(rangeMod10.modIndex));
    CPPUNIT_ASSERT(rangeMod10.initialValue.data == copy.flows[1].rangeModMap[rangeMod10.modIndex].start);
    CPPUNIT_ASSERT(rangeMod10.stepValue.data == copy.flows[1].rangeModMap[rangeMod10.modIndex].step);
    CPPUNIT_ASSERT(rangeMod10.mask.data == copy.flows[1].rangeModMap[rangeMod10.modIndex].mask);
    CPPUNIT_ASSERT_EQUAL(rangeMod10.recycleCount, copy.flows[1].rangeModMap[rangeMod10.modIndex].recycleCount);
    CPPUNIT_ASSERT_EQUAL(rangeMod10.stutterCount, copy.flows[1].rangeModMap[rangeMod10.modIndex].stutterCount);
    CPPUNIT_ASSERT_EQUAL(int(rangeMod10.mode), int(copy.flows[1].rangeModMap[rangeMod10.modIndex].mode));

    // invalid modifier configs

    // start size mismatch
    config.Streams[0].RangeMods[0].initialValue.data.resize(3, 1);
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy2(config), EPHXBadConfig);

    // fix it
    config.Streams[0].RangeMods[0].initialValue.data.resize(4, 1);
    PlaylistConfigProxy proxy2b(config);

    // step size mismatch
    config.Streams[0].RangeMods[0].stepValue.data.resize(5, 2);
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy3(config), EPHXBadConfig);

    // fix it
    config.Streams[0].RangeMods[0].stepValue.data.resize(4, 2);
    PlaylistConfigProxy proxy3b(config);

    // mask size mismatch
    config.Streams[0].RangeMods[0].mask.data.resize(9, 3);
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy4(config), EPHXBadConfig);

    // fix it
    config.Streams[0].RangeMods[0].mask.data.resize(4, 3);
    PlaylistConfigProxy proxy4b(config);

    // variable index out of order
    config.Streams[0].RangeMods[0].modIndex = 5;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy5(config), EPHXBadConfig);

    // fix it
    config.Streams[0].RangeMods[0].modIndex = 1;
    PlaylistConfigProxy proxy5b(config);
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistConfigProxy::testTableModifiers()
{
    PlaylistConfigProxy::playlistConfig_t config;
    config.Streams.resize(2);
    makeStreamsValid(config);
    config.Streams[0].TableMods.resize(2);
    config.Streams[0].TableMods[0].modIndex = 1;
    config.Streams[0].TableMods[0].data.resize(2);
    config.Streams[0].TableMods[0].data[0].data.resize(3, 0x33);
    config.Streams[0].TableMods[0].data[0].data.resize(4, 0x44);
    config.Streams[0].TableMods[0].stutterCount = 5;
    config.Streams[0].TableMods[1].modIndex = 2;
    config.Streams[0].TableMods[1].data.resize(1);
    config.Streams[0].TableMods[1].data[0].data.resize(2, 0x22);
    config.Streams[0].TableMods[1].stutterCount = 1;
    config.Streams[1].TableMods.resize(1);
    config.Streams[1].TableMods[0].modIndex = 99;
    config.Streams[1].TableMods[0].data.resize(1);
    config.Streams[1].TableMods[0].data[0].data.resize(1, 0x11);
    config.Streams[1].TableMods[0].stutterCount = 0;
    
    PlaylistConfigProxy proxy(config);
    L4L7_ENGINE_NS::PlaylistConfig copy;

    // add some bogus data to make sure the proxy clears it out
    copy.flows.resize(10);
    memset(&copy.flows[0], 0xff, 31);

    proxy.CopyTo(copy); 

    CPPUNIT_ASSERT_EQUAL(size_t(2), copy.flows.size());

    CPPUNIT_ASSERT_EQUAL(size_t(0), copy.flows[0].rangeModMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), copy.flows[0].tableModMap.size());
    Dpg_1_port_server::tableModifier_t& tableMod00(config.Streams[0].TableMods[0]);

    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[0].tableModMap.count(tableMod00.modIndex));
    CPPUNIT_ASSERT_EQUAL(size_t(2), copy.flows[0].tableModMap[tableMod00.modIndex].table.size());
    CPPUNIT_ASSERT(tableMod00.data[0].data == copy.flows[0].tableModMap[tableMod00.modIndex].table[0]);
    CPPUNIT_ASSERT(tableMod00.data[1].data == copy.flows[0].tableModMap[tableMod00.modIndex].table[1]);
    CPPUNIT_ASSERT_EQUAL(tableMod00.stutterCount, copy.flows[0].tableModMap[tableMod00.modIndex].stutterCount);

    Dpg_1_port_server::tableModifier_t& tableMod01(config.Streams[0].TableMods[1]);

    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[0].tableModMap.count(tableMod01.modIndex));
    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[0].tableModMap[tableMod01.modIndex].table.size());
    CPPUNIT_ASSERT(tableMod01.data[0].data == copy.flows[0].tableModMap[tableMod01.modIndex].table[0]);
    CPPUNIT_ASSERT_EQUAL(tableMod01.stutterCount, copy.flows[0].tableModMap[tableMod01.modIndex].stutterCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[1].tableModMap.size());
    Dpg_1_port_server::tableModifier_t& tableMod10(config.Streams[1].TableMods[0]);

    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[1].tableModMap.count(tableMod10.modIndex));
    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[1].tableModMap[tableMod10.modIndex].table.size());
    CPPUNIT_ASSERT(tableMod10.data[0].data == copy.flows[1].tableModMap[tableMod10.modIndex].table[0]);
    CPPUNIT_ASSERT_EQUAL(tableMod10.stutterCount, copy.flows[1].tableModMap[tableMod10.modIndex].stutterCount);

    // invalid modifier configs

    // range and table modifier on the same variable
    config.Streams[0].RangeMods.resize(1);
    config.Streams[0].RangeMods[0].modIndex = 2;
    config.Streams[0].RangeMods[0].initialValue.data.resize(1, 100);
    config.Streams[0].RangeMods[0].stepValue.data.resize(1, 100);
    config.Streams[0].RangeMods[0].mask.data.resize(1, 100);
    config.Streams[0].RangeMods[0].recycleCount = 101;
    config.Streams[0].RangeMods[0].stutterCount = 102;
    config.Streams[0].RangeMods[0].mode = Modifiers_1_port_server::CONFIG_MOD_MODE_INCREMENT;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy2(config), EPHXBadConfig);

    // verify it works on a non-overlapping index
    config.Streams[0].RangeMods[0].modIndex = 3;
    PlaylistConfigProxy proxy2b(config);

    // get rid of the range mod completely
    config.Streams[0].RangeMods.clear();

    // variable index out of order
    config.Streams[0].TableMods[0].modIndex = 5;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy3(config), EPHXBadConfig);

    // fix it
    config.Streams[0].TableMods[0].modIndex = 1;
    PlaylistConfigProxy proxy3b(config);
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistConfigProxy::testLinkedModifiers()
{
    PlaylistConfigProxy::playlistConfig_t config;
    config.Streams.resize(2);
    Dpg_1_port_server::DpgStreamReference_t_clear(&config.Streams[0]);
    config.Streams[0].FlowParam.ServerPort = 1;
    config.Streams[0].RangeMods.resize(3);
    config.Streams[0].RangeMods[0].modIndex = 1;
    config.Streams[0].RangeMods[0].initialValue.data.resize(4, 1);
    config.Streams[0].RangeMods[0].stepValue.data.resize(4, 2);
    config.Streams[0].RangeMods[0].mask.data.resize(4, 3);
    config.Streams[0].RangeMods[0].recycleCount = 4;
    config.Streams[0].RangeMods[0].stutterCount = 5;
    config.Streams[0].RangeMods[0].mode = Modifiers_1_port_server::CONFIG_MOD_MODE_INCREMENT;
    config.Streams[0].RangeMods[1].modIndex = 2;
    config.Streams[0].RangeMods[1].initialValue.data.resize(2, 1);
    config.Streams[0].RangeMods[1].stepValue.data.resize(2, 2);
    config.Streams[0].RangeMods[1].mask.data.resize(2, 3);
    config.Streams[0].RangeMods[1].recycleCount = 2;
    config.Streams[0].RangeMods[1].stutterCount = 2;
    config.Streams[0].RangeMods[1].mode = Modifiers_1_port_server::CONFIG_MOD_MODE_DECREMENT;
    config.Streams[0].RangeMods[2].modIndex = 99;
    config.Streams[0].RangeMods[2].initialValue.data.resize(1, 100);
    config.Streams[0].RangeMods[2].stepValue.data.resize(1, 100);
    config.Streams[0].RangeMods[2].mask.data.resize(1, 100);
    config.Streams[0].RangeMods[2].recycleCount = 101;
    config.Streams[0].RangeMods[2].stutterCount = 102;
    config.Streams[0].RangeMods[2].mode = Modifiers_1_port_server::CONFIG_MOD_MODE_INCREMENT;
    config.Streams[1] = config.Streams[0];
    // link 1 -> 2 -> 99
    config.Streams[0].ModLinks.resize(2);
    config.Streams[0].ModLinks[0].modIndex = 1;
    config.Streams[0].ModLinks[0].childModIndex = 2;
    config.Streams[0].ModLinks[1].modIndex = 2;
    config.Streams[0].ModLinks[1].childModIndex = 99;
    // link 2 -> 1 -> 99
    config.Streams[1].ModLinks.resize(2);
    config.Streams[1].ModLinks[0].modIndex = 1;
    config.Streams[1].ModLinks[0].childModIndex = 99;
    config.Streams[1].ModLinks[1].modIndex = 2;
    config.Streams[1].ModLinks[1].childModIndex = 1;

    PlaylistConfigProxy proxy(config);
    L4L7_ENGINE_NS::PlaylistConfig copy;

    // add some bogus data to make sure the proxy clears it out
    copy.flows.resize(10);
    memset(&copy.flows[0], 0xff, 31);

    proxy.CopyTo(copy); 

    CPPUNIT_ASSERT_EQUAL(size_t(2), copy.flows.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), copy.flows[0].linkModMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[0].linkModMap.count(1));
    CPPUNIT_ASSERT_EQUAL(size_t(2), size_t(copy.flows[0].linkModMap[1]));
    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[0].linkModMap.count(2));
    CPPUNIT_ASSERT_EQUAL(size_t(99), size_t(copy.flows[0].linkModMap[2]));

    CPPUNIT_ASSERT_EQUAL(size_t(2), copy.flows[1].linkModMap.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[1].linkModMap.count(1));
    CPPUNIT_ASSERT_EQUAL(size_t(99), size_t(copy.flows[1].linkModMap[1]));
    CPPUNIT_ASSERT_EQUAL(size_t(1), copy.flows[1].linkModMap.count(2));
    CPPUNIT_ASSERT_EQUAL(size_t(1), size_t(copy.flows[1].linkModMap[2]));

    // invalid link configs
    
    // self-linking modifier
    config.Streams[1].ModLinks[1].childModIndex = 2;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy2(config), EPHXBadConfig);

    // fix it
    config.Streams[1].ModLinks[1].childModIndex = 1;
    PlaylistConfigProxy proxy2b(config);

    // modifiers out of order
    config.Streams[1].ModLinks[0].modIndex = 2;
    config.Streams[1].ModLinks[1].modIndex = 0;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy3(config), EPHXBadConfig);

    // fix it
    config.Streams[1].ModLinks[0].modIndex = 1;
    config.Streams[1].ModLinks[1].modIndex = 2;
    PlaylistConfigProxy proxy3b(config);

    // duplicated modifiers
    config.Streams[1].ModLinks[0].modIndex = 0;
    config.Streams[1].ModLinks[1].modIndex = 0;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy4(config), EPHXBadConfig);

    // fix it
    config.Streams[1].ModLinks[0].modIndex = 1;
    config.Streams[1].ModLinks[1].modIndex = 2;
    PlaylistConfigProxy proxy4b(config);

    // link parent referencing a non-existent modifier
    config.Streams[1].ModLinks[1].modIndex = 98;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy5(config), EPHXBadConfig);

    // fix it
    config.Streams[1].ModLinks[1].modIndex = 2;
    PlaylistConfigProxy proxy5b(config);

    // link child referencing a non-existent modifier
    config.Streams[1].ModLinks[1].childModIndex = 98;
    CPPUNIT_ASSERT_THROW(PlaylistConfigProxy proxy6(config), EPHXBadConfig);

    // fix it
    config.Streams[1].ModLinks[1].childModIndex = 1;
    PlaylistConfigProxy proxy6b(config);
}

///////////////////////////////////////////////////////////////////////////////

// prevent ValidateFlows from failing by initializing parameters

void TestPlaylistConfigProxy::makeStreamsValid(PlaylistConfigProxy::playlistConfig_t& config)
{
    typedef DpgIf_1_port_server::DpgStreamReference_t streamRef_t;
    typedef DpgIf_1_port_server::DpgAttackReference_t attackRef_t;

    BOOST_FOREACH(streamRef_t& stream, config.Streams)
    {
        Dpg_1_port_server::DpgStreamReference_t_clear(&stream);
        stream.FlowParam.ServerPort = 1;
    }

    BOOST_FOREACH(attackRef_t& attack, config.Attacks)
    {
        Dpg_1_port_server::DpgAttackReference_t_clear(&attack);
        attack.AttackParam.InactivityTimeout = 1000;
        attack.FlowParam.ServerPort = 1;
    }
}

///////////////////////////////////////////////////////////////////////////////


CPPUNIT_TEST_SUITE_REGISTRATION(TestPlaylistConfigProxy);
