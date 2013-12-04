#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <boost/foreach.hpp>
#include <engine/FlowConfig.h>

#include "FlowConfigProxy.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestFlowConfigProxy : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestFlowConfigProxy);
    CPPUNIT_TEST(testBasic);
    CPPUNIT_TEST(testBadConfig);
    CPPUNIT_TEST(testCopyOver);
    CPPUNIT_TEST(testCloseTypes);
    CPPUNIT_TEST(testPlayTypes);
    CPPUNIT_TEST(testLayers);
    CPPUNIT_TEST(testLoops);
    CPPUNIT_TEST(testVariables);
    CPPUNIT_TEST(testNoData);
    CPPUNIT_TEST_SUITE_END();

public:
    TestFlowConfigProxy()
    {
    }

    ~TestFlowConfigProxy()
    {
    }

    void setUp() 
    { 
        m_buffer.resize(100);
        uint8_t data_val = 0;
        BOOST_FOREACH(uint8_t& data, m_buffer)
        {
            data = data_val++;
        }
        m_binary = new Tm_binary(&m_buffer[0], m_buffer.size());
    }

    void tearDown() 
    { 
        m_buffer.resize(0);
        delete m_binary;
    }
    
protected:
    void testBasic();
    void testCopyOver();
    void testBadConfig();
    void testCloseTypes();
    void testPlayTypes();
    void testLayers();
    void testLoops();
    void testVariables();
    void testNoData();

private:
    Tm_binary * m_binary;
    std::vector<uint8_t> m_buffer;
};

///////////////////////////////////////////////////////////////////////////////

void TestFlowConfigProxy::testBasic()
{
    FlowConfigProxy::flowConfig_t config;

    config.Data = *m_binary;
    config.PktLen.resize(4);
    config.PktDelay.resize(4);
    config.ClientTx.resize(4);
    config.Type  = Dpg_1_port_server::playType_t(0);
    config.Layer = Dpg_1_port_server::layer_t(0);
    config.Close = Dpg_1_port_server::closeType_t(0);

    uint32_t pkt_len_val = 22;
    BOOST_FOREACH(uint32_t& pkt_len, config.PktLen)
    {
        pkt_len = pkt_len_val;
        pkt_len_val += 2;
    }
    // 22, 24, 26, 28 -> 100
    
    uint32_t pkt_delay_val = 0; 
    BOOST_FOREACH(uint32_t& pkt_delay, config.PktDelay)
    {
        pkt_delay = pkt_delay_val++;
    }
    // 0, 1, 2, 3

    bool client_tx_val = false;
    // ARGH vector<bool> is evil
    for (std::vector<bool>::iterator iter = config.ClientTx.begin();
         iter != config.ClientTx.end();
         ++iter)
    {
        *iter = client_tx_val;
        client_tx_val = !client_tx_val;
    }
    // false, true, false, true

    FlowConfigProxy proxy(config);

    L4L7_ENGINE_NS::FlowConfig copy;
    proxy.CopyTo(copy); 

    CPPUNIT_ASSERT_EQUAL(size_t(4), copy.pktList.size());

    CPPUNIT_ASSERT_EQUAL(config.PktLen[0], copy.pktList[0].data.size());
    CPPUNIT_ASSERT_EQUAL(config.PktLen[1], copy.pktList[1].data.size());
    CPPUNIT_ASSERT_EQUAL(config.PktLen[2], copy.pktList[2].data.size());
    CPPUNIT_ASSERT_EQUAL(config.PktLen[3], copy.pktList[3].data.size());

    CPPUNIT_ASSERT_EQUAL(config.PktDelay[0], copy.pktList[0].pktDelay);
    CPPUNIT_ASSERT_EQUAL(config.PktDelay[1], copy.pktList[1].pktDelay);
    CPPUNIT_ASSERT_EQUAL(config.PktDelay[2], copy.pktList[2].pktDelay);
    CPPUNIT_ASSERT_EQUAL(config.PktDelay[3], copy.pktList[3].pktDelay);

    CPPUNIT_ASSERT_EQUAL((bool)config.ClientTx[0], copy.pktList[0].clientTx);
    CPPUNIT_ASSERT_EQUAL((bool)config.ClientTx[1], copy.pktList[1].clientTx);
    CPPUNIT_ASSERT_EQUAL((bool)config.ClientTx[2], copy.pktList[2].clientTx);
    CPPUNIT_ASSERT_EQUAL((bool)config.ClientTx[3], copy.pktList[3].clientTx);

    CPPUNIT_ASSERT(memcmp(config.Data.get_buffer(), &copy.pktList[0].data[0], copy.pktList[0].data.size()) == 0);
    CPPUNIT_ASSERT(memcmp(config.Data.get_buffer()+22, &copy.pktList[1].data[0], copy.pktList[1].data.size()) == 0);
    CPPUNIT_ASSERT(memcmp(config.Data.get_buffer()+22+24, &copy.pktList[2].data[0], copy.pktList[2].data.size()) == 0);
    CPPUNIT_ASSERT(memcmp(config.Data.get_buffer()+22+24+26, &copy.pktList[3].data[0], copy.pktList[3].data.size()) == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowConfigProxy::testCopyOver()
{
    FlowConfigProxy::flowConfig_t config;

    config.Data = *m_binary;
    config.PktLen.assign(4, 25);
    config.PktDelay.assign(4, 10);
    config.ClientTx.assign(4, true);
    config.Type  = Dpg_1_port_server::playType_t(0);
    config.Layer = Dpg_1_port_server::layer_t(0);
    config.Close = Dpg_1_port_server::closeType_t(0);

    FlowConfigProxy proxy(config);

    L4L7_ENGINE_NS::FlowConfig copy;

    // pre-fill with garbage
    copy.pktList.resize(15);
    copy.pktList[0].data.resize(123);
    copy.pktList[1].data.resize(321);

    proxy.CopyTo(copy); 

    CPPUNIT_ASSERT_EQUAL(size_t(4), copy.pktList.size());

    CPPUNIT_ASSERT_EQUAL(config.PktLen[0], copy.pktList[0].data.size());
    CPPUNIT_ASSERT_EQUAL(config.PktLen[1], copy.pktList[1].data.size());
    CPPUNIT_ASSERT_EQUAL(config.PktLen[2], copy.pktList[2].data.size());
    CPPUNIT_ASSERT_EQUAL(config.PktLen[3], copy.pktList[3].data.size());

    CPPUNIT_ASSERT_EQUAL(config.PktDelay[0], copy.pktList[0].pktDelay);
    CPPUNIT_ASSERT_EQUAL(config.PktDelay[1], copy.pktList[1].pktDelay);
    CPPUNIT_ASSERT_EQUAL(config.PktDelay[2], copy.pktList[2].pktDelay);
    CPPUNIT_ASSERT_EQUAL(config.PktDelay[3], copy.pktList[3].pktDelay);

    CPPUNIT_ASSERT_EQUAL((bool)config.ClientTx[0], copy.pktList[0].clientTx);
    CPPUNIT_ASSERT_EQUAL((bool)config.ClientTx[1], copy.pktList[1].clientTx);
    CPPUNIT_ASSERT_EQUAL((bool)config.ClientTx[2], copy.pktList[2].clientTx);
    CPPUNIT_ASSERT_EQUAL((bool)config.ClientTx[3], copy.pktList[3].clientTx);

    CPPUNIT_ASSERT(memcmp(config.Data.get_buffer(), &copy.pktList[0].data[0], copy.pktList[0].data.size()) == 0);
    CPPUNIT_ASSERT(memcmp(config.Data.get_buffer()+25, &copy.pktList[1].data[0], copy.pktList[1].data.size()) == 0);
    CPPUNIT_ASSERT(memcmp(config.Data.get_buffer()+50, &copy.pktList[2].data[0], copy.pktList[2].data.size()) == 0);
    CPPUNIT_ASSERT(memcmp(config.Data.get_buffer()+75, &copy.pktList[3].data[0], copy.pktList[3].data.size()) == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowConfigProxy::testBadConfig()
{
    FlowConfigProxy::flowConfig_t config;

    config.Data = *m_binary;
    config.PktLen.resize(4);
    config.PktDelay.resize(4);
    config.ClientTx.resize(4);
    config.Type  = Dpg_1_port_server::playType_t(0);
    config.Layer = Dpg_1_port_server::layer_t(0);
    config.Close = Dpg_1_port_server::closeType_t(0);

    uint32_t pkt_len_val = 22;
    BOOST_FOREACH(uint32_t& pkt_len, config.PktLen)
    {
        pkt_len = pkt_len_val;
        pkt_len_val += 2;
    }
    // 22, 24, 26, 28 -> 100
    
    // no throw here
    FlowConfigProxy proxy(config);

    // break packet length:
    ++config.PktLen[0];

    // should throw
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy2(config), EPHXBadConfig);
    
    // fix packet length, break clientTx array size
    --config.PktLen[0];
    config.ClientTx.resize(5);

    // should throw
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy3(config), EPHXBadConfig);

    // fix clientTx array size, break delay array size
    config.ClientTx.resize(4);
    config.PktDelay.resize(5);

    // should throw
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy4(config), EPHXBadConfig);

    // verify we can fix it
    config.PktDelay.resize(4);
    FlowConfigProxy proxy5(config);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowConfigProxy::testCloseTypes()
{
    FlowConfigProxy::flowConfig_t config;

    config.Data = *m_binary;
    config.PktLen.push_back(100);
    config.PktDelay.push_back(50);
    config.ClientTx.push_back(true);
    config.Type  = DpgIf_1_port_server::playType_t(0);
    config.Layer = DpgIf_1_port_server::layer_t(0);
    config.Close = DpgIf_1_port_server::closeType_t(0);

    config.Close = DpgIf_1_port_server::CLOSE_TYPE_C_FIN;
    FlowConfigProxy proxy(config);
    L4L7_ENGINE_NS::FlowConfig copy;
    proxy.CopyTo(copy); 
    CPPUNIT_ASSERT_EQUAL(L4L7_ENGINE_NS::FlowConfig::C_FIN, copy.closeType);

    config.Close = DpgIf_1_port_server::CLOSE_TYPE_C_RST;
    FlowConfigProxy proxy2(config);
    proxy2.CopyTo(copy); 
    CPPUNIT_ASSERT_EQUAL(L4L7_ENGINE_NS::FlowConfig::C_RST, copy.closeType);

    config.Close = DpgIf_1_port_server::CLOSE_TYPE_S_FIN;
    FlowConfigProxy proxy3(config);
    proxy3.CopyTo(copy); 
    CPPUNIT_ASSERT_EQUAL(L4L7_ENGINE_NS::FlowConfig::S_FIN, copy.closeType);

    config.Close = DpgIf_1_port_server::CLOSE_TYPE_S_RST;
    FlowConfigProxy proxy4(config);
    proxy4.CopyTo(copy); 
    CPPUNIT_ASSERT_EQUAL(L4L7_ENGINE_NS::FlowConfig::S_RST, copy.closeType);

    // invalid close types
    config.Close = DpgIf_1_port_server::closeType_t(-1);
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy5(config), EPHXBadConfig);

    config.Close = DpgIf_1_port_server::closeType_t(5);
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy6(config), EPHXBadConfig);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowConfigProxy::testPlayTypes()
{
    FlowConfigProxy::flowConfig_t config;

    config.Data = *m_binary;
    config.PktLen.push_back(100);
    config.PktDelay.push_back(50);
    config.ClientTx.push_back(true);
    config.Type  = DpgIf_1_port_server::playType_t(0);
    config.Layer = DpgIf_1_port_server::layer_t(0);
    config.Close = DpgIf_1_port_server::closeType_t(0);

    config.Type = DpgIf_1_port_server::PLAY_TYPE_STREAM;
    FlowConfigProxy proxy(config);
    L4L7_ENGINE_NS::FlowConfig copy;
    proxy.CopyTo(copy); 
    CPPUNIT_ASSERT_EQUAL(L4L7_ENGINE_NS::FlowConfig::P_STREAM, copy.playType);

    config.Type = DpgIf_1_port_server::PLAY_TYPE_ATTACK;
    FlowConfigProxy proxy2(config);
    proxy2.CopyTo(copy); 
    CPPUNIT_ASSERT_EQUAL(L4L7_ENGINE_NS::FlowConfig::P_ATTACK, copy.playType);

    // invalid play types
    config.Type = DpgIf_1_port_server::playType_t(-1);
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy3(config), EPHXBadConfig);

    config.Type = DpgIf_1_port_server::playType_t(3);
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy4(config), EPHXBadConfig);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowConfigProxy::testLayers()
{
    FlowConfigProxy::flowConfig_t config;

    config.Data = *m_binary;
    config.PktLen.push_back(100);
    config.PktDelay.push_back(50);
    config.ClientTx.push_back(true);
    config.Type  = DpgIf_1_port_server::playType_t(0);
    config.Layer = DpgIf_1_port_server::layer_t(0);
    config.Close = DpgIf_1_port_server::closeType_t(0);

    config.Layer = DpgIf_1_port_server::LAYER_TCP;
    FlowConfigProxy proxy(config);
    L4L7_ENGINE_NS::FlowConfig copy;
    proxy.CopyTo(copy); 
    CPPUNIT_ASSERT_EQUAL(L4L7_ENGINE_NS::FlowConfig::TCP, copy.layer);

    config.Layer = DpgIf_1_port_server::LAYER_UDP;
    FlowConfigProxy proxy2(config);
    proxy2.CopyTo(copy); 
    CPPUNIT_ASSERT_EQUAL(L4L7_ENGINE_NS::FlowConfig::UDP, copy.layer);

    // invalid layers
    config.Layer = DpgIf_1_port_server::layer_t(-1);
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy3(config), EPHXBadConfig);

    config.Layer = DpgIf_1_port_server::layer_t(3);
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy4(config), EPHXBadConfig);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowConfigProxy::testLoops()
{
    FlowConfigProxy::flowConfig_t config;

    config.Data = *m_binary;
    config.PktDelay.resize(4, 100);
    config.PktLen.resize(4, 25);
    config.ClientTx.resize(4, true);
    config.Type  = Dpg_1_port_server::playType_t(0);
    config.Layer = Dpg_1_port_server::layer_t(0);
    config.Close = Dpg_1_port_server::closeType_t(0);

    config.Loops.resize(2); 
    config.Loops[0].BegIdx = 0;
    config.Loops[0].EndIdx = 1;
    config.Loops[0].Count = 10;
    config.Loops[1].BegIdx = 3;
    config.Loops[1].EndIdx = 3;
    config.Loops[1].Count = 100;

    FlowConfigProxy proxy(config);
    L4L7_ENGINE_NS::FlowConfig copy;
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
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy2(config), EPHXBadConfig);

    config.Loops[0].BegIdx = 0;
    config.Loops[0].EndIdx = 1;
    config.Loops[1].BegIdx = 0;
    config.Loops[1].EndIdx = 3;
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy3(config), EPHXBadConfig);
    
    // out of range idx
    config.Loops[0].BegIdx = 0;
    config.Loops[0].EndIdx = 1;
    config.Loops[1].BegIdx = 2;
    config.Loops[1].EndIdx = 4;
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy4(config), EPHXBadConfig);

    config.Loops[0].BegIdx = 0;
    config.Loops[0].EndIdx = 1;
    config.Loops[1].BegIdx = 4;
    config.Loops[1].EndIdx = 5;
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy5(config), EPHXBadConfig);

    // reversed beg/end
    config.Loops[0].BegIdx = 0;
    config.Loops[0].EndIdx = 1;
    config.Loops[1].BegIdx = 3;
    config.Loops[1].EndIdx = 2;
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy6(config), EPHXBadConfig);

    config.Loops[0].BegIdx = 1;
    config.Loops[0].EndIdx = 0;
    config.Loops[1].BegIdx = 2;
    config.Loops[1].EndIdx = 3;
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy7(config), EPHXBadConfig);

    // out of order loops
    config.Loops[0].BegIdx = 2;
    config.Loops[0].EndIdx = 3;
    config.Loops[1].BegIdx = 0;
    config.Loops[1].EndIdx = 1;
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy8(config), EPHXBadConfig);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowConfigProxy::testVariables()
{
    FlowConfigProxy::flowConfig_t config;

    config.Data = *m_binary;
    config.PktDelay.resize(4, 100);
    config.PktLen.resize(4, 25);
    config.ClientTx.resize(4, true);
    config.Type  = DpgIf_1_port_server::playType_t(0);
    config.Layer = DpgIf_1_port_server::layer_t(0);
    config.Close = DpgIf_1_port_server::closeType_t(0);

    config.Vars.resize(2); 
    config.Vars[0].Name = "foo";
    config.Vars[0].Value.resize(2, 0xff);
    config.Vars[0].PktIdx.resize(1, 1);
    config.Vars[0].ByteIdx.resize(1, 12);
    config.Vars[0].FixedLen = true;
    config.Vars[0].Endian = DpgIf_1_port_server::ENDIAN_BIG;
    config.Vars[1].Name = "bar";
    config.Vars[1].Value.resize(4, 1);
    config.Vars[1].PktIdx.resize(2);
    config.Vars[1].ByteIdx.resize(2);
    config.Vars[1].PktIdx[0] = 3;
    config.Vars[1].ByteIdx[0] = 25;
    config.Vars[1].PktIdx[1] = 1;
    config.Vars[1].ByteIdx[1] = 13;
    config.Vars[1].FixedLen = false;
    config.Vars[1].Endian = DpgIf_1_port_server::ENDIAN_LITTLE;

    FlowConfigProxy proxy(config);
    L4L7_ENGINE_NS::FlowConfig copy;
    proxy.CopyTo(copy); 
    CPPUNIT_ASSERT_EQUAL(size_t(2), copy.varList.size());

    CPPUNIT_ASSERT_EQUAL(config.Vars[0].Name, copy.varList[0].name);
    CPPUNIT_ASSERT_EQUAL(config.Vars[0].Value.size(), copy.varList[0].value.size());
    CPPUNIT_ASSERT(config.Vars[0].Value == copy.varList[0].value);
    CPPUNIT_ASSERT_EQUAL(config.Vars[0].PktIdx.size(), copy.varList[0].targetList.size());
    CPPUNIT_ASSERT_EQUAL(config.Vars[0].PktIdx[0], copy.varList[0].targetList[0].pktIdx);
    CPPUNIT_ASSERT_EQUAL(config.Vars[0].ByteIdx.size(), copy.varList[0].targetList.size());
    CPPUNIT_ASSERT_EQUAL(config.Vars[0].ByteIdx[0], copy.varList[0].targetList[0].byteIdx);
    CPPUNIT_ASSERT_EQUAL(config.Vars[0].FixedLen, copy.varList[0].fixedLen);
    CPPUNIT_ASSERT_EQUAL(L4L7_ENGINE_NS::FlowConfig::BIG, copy.varList[0].endian);

    CPPUNIT_ASSERT_EQUAL(config.Vars[1].Name, copy.varList[1].name);
    CPPUNIT_ASSERT_EQUAL(config.Vars[1].Value.size(), copy.varList[1].value.size());
    CPPUNIT_ASSERT(config.Vars[1].Value == copy.varList[1].value);
    CPPUNIT_ASSERT_EQUAL(config.Vars[1].PktIdx.size(), copy.varList[1].targetList.size());
    CPPUNIT_ASSERT_EQUAL(config.Vars[1].PktIdx[0], copy.varList[1].targetList[0].pktIdx);
    CPPUNIT_ASSERT_EQUAL(config.Vars[1].PktIdx[1], copy.varList[1].targetList[1].pktIdx);
    CPPUNIT_ASSERT_EQUAL(config.Vars[1].ByteIdx.size(), copy.varList[1].targetList.size());
    CPPUNIT_ASSERT_EQUAL(config.Vars[1].ByteIdx[0], copy.varList[1].targetList[0].byteIdx);
    CPPUNIT_ASSERT_EQUAL(config.Vars[1].ByteIdx[1], copy.varList[1].targetList[1].byteIdx);
    CPPUNIT_ASSERT_EQUAL(config.Vars[1].FixedLen, copy.varList[1].fixedLen);
    CPPUNIT_ASSERT_EQUAL(L4L7_ENGINE_NS::FlowConfig::LITTLE, copy.varList[1].endian);

    // convenience pointers
    CPPUNIT_ASSERT_EQUAL(size_t(4), copy.pktList.size());
    CPPUNIT_ASSERT(copy.pktList[0].varMap.empty());
    CPPUNIT_ASSERT(copy.pktList[1].varMap.size() == 2);
    CPPUNIT_ASSERT(copy.pktList[1].varMap.count(12) == 1);
    CPPUNIT_ASSERT_EQUAL(0, int(copy.pktList[1].varMap.lower_bound(12)->second));
    CPPUNIT_ASSERT(copy.pktList[1].varMap.count(13) == 1);
    CPPUNIT_ASSERT_EQUAL(1, int(copy.pktList[1].varMap.lower_bound(13)->second));
    CPPUNIT_ASSERT(copy.pktList[2].varMap.empty());
    CPPUNIT_ASSERT(copy.pktList[3].varMap.size() == 1);
    CPPUNIT_ASSERT(copy.pktList[3].varMap.count(25) == 1);
    CPPUNIT_ASSERT_EQUAL(1, int(copy.pktList[3].varMap.lower_bound(25)->second));
    
    // invalid variable configs
    
    // bad packet index
    config.Vars[1].PktIdx[0] = 4;
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy2(config), EPHXBadConfig);

    // back to valid
    config.Vars[1].PktIdx[0] = 3;
    FlowConfigProxy proxy2b(config);


    // bad byte index
    config.Vars[0].ByteIdx[0] = 26;
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy3(config), EPHXBadConfig);

    // back to valid
    config.Vars[0].ByteIdx[0] = 12;
    FlowConfigProxy proxy3b(config);

    // packet index / byte index array length mismatch
    config.Vars[1].PktIdx.push_back(2);
    CPPUNIT_ASSERT_THROW(FlowConfigProxy proxy4(config), EPHXBadConfig);

    // back to valid
    config.Vars[1].PktIdx.pop_back();
    FlowConfigProxy proxy4b(config);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowConfigProxy::testNoData()
{
    // not an error condition, just all variables
    FlowConfigProxy::flowConfig_t config;

    Tm_binary empty_binary;
    config.Data = empty_binary;
    config.PktLen.resize(2, 0);
    config.PktDelay.resize(2, 1);
    config.ClientTx.resize(2, true);
    config.Type  = DpgIf_1_port_server::playType_t(0);
    config.Layer = DpgIf_1_port_server::layer_t(0);
    config.Close = DpgIf_1_port_server::closeType_t(0);

    config.Vars.resize(2); 
    config.Vars[0].Name = "foo";
    config.Vars[0].Value.resize(2, 0xff);
    config.Vars[0].PktIdx.resize(1, 0);
    config.Vars[0].ByteIdx.resize(1, 0);
    config.Vars[0].FixedLen = true;
    config.Vars[1].Name = "bar";
    config.Vars[1].Value.resize(4, 1);
    config.Vars[1].PktIdx.resize(2);
    config.Vars[1].ByteIdx.resize(2);
    config.Vars[1].PktIdx[0] = 0;
    config.Vars[1].ByteIdx[0] = 0;
    config.Vars[1].PktIdx[1] = 1;
    config.Vars[1].ByteIdx[1] = 0;
    config.Vars[1].FixedLen = false;

    FlowConfigProxy proxy(config);

    L4L7_ENGINE_NS::FlowConfig copy;
    proxy.CopyTo(copy); 

    CPPUNIT_ASSERT_EQUAL(size_t(2), copy.varList.size());

    CPPUNIT_ASSERT_EQUAL(config.Vars[0].Name, copy.varList[0].name);
    CPPUNIT_ASSERT_EQUAL(config.Vars[0].Value.size(), copy.varList[0].value.size());
    CPPUNIT_ASSERT(config.Vars[0].Value == copy.varList[0].value);
    CPPUNIT_ASSERT_EQUAL(config.Vars[0].PktIdx.size(), copy.varList[0].targetList.size());
    CPPUNIT_ASSERT_EQUAL(config.Vars[0].PktIdx[0], copy.varList[0].targetList[0].pktIdx);
    CPPUNIT_ASSERT_EQUAL(config.Vars[0].ByteIdx.size(), copy.varList[0].targetList.size());
    CPPUNIT_ASSERT_EQUAL(config.Vars[0].ByteIdx[0], copy.varList[0].targetList[0].byteIdx);
    CPPUNIT_ASSERT_EQUAL(config.Vars[0].FixedLen, copy.varList[0].fixedLen);

    CPPUNIT_ASSERT_EQUAL(config.Vars[1].Name, copy.varList[1].name);
    CPPUNIT_ASSERT_EQUAL(config.Vars[1].Value.size(), copy.varList[1].value.size());
    CPPUNIT_ASSERT(config.Vars[1].Value == copy.varList[1].value);
    CPPUNIT_ASSERT_EQUAL(config.Vars[1].PktIdx.size(), copy.varList[1].targetList.size());
    CPPUNIT_ASSERT_EQUAL(config.Vars[1].PktIdx[0], copy.varList[1].targetList[0].pktIdx);
    CPPUNIT_ASSERT_EQUAL(config.Vars[1].PktIdx[1], copy.varList[1].targetList[1].pktIdx);
    CPPUNIT_ASSERT_EQUAL(config.Vars[1].ByteIdx.size(), copy.varList[1].targetList.size());
    CPPUNIT_ASSERT_EQUAL(config.Vars[1].ByteIdx[0], copy.varList[1].targetList[0].byteIdx);
    CPPUNIT_ASSERT_EQUAL(config.Vars[1].ByteIdx[1], copy.varList[1].targetList[1].byteIdx);
    CPPUNIT_ASSERT_EQUAL(config.Vars[1].FixedLen, copy.varList[1].fixedLen);

    // pktList should contain two empty packets
    CPPUNIT_ASSERT_EQUAL(size_t(2), copy.pktList.size());

    CPPUNIT_ASSERT(copy.pktList[0].data.empty());
    CPPUNIT_ASSERT(copy.pktList[1].data.empty());

    // convenience pointers
    CPPUNIT_ASSERT(copy.pktList[0].varMap.size() == 2);
    CPPUNIT_ASSERT(copy.pktList[0].varMap.count(0) == 2); // pkt 0 byte 0
    CPPUNIT_ASSERT_EQUAL(0, int(copy.pktList[0].varMap.lower_bound(0)->second));
    // pkt 0, byte 0 -> var 0
    CPPUNIT_ASSERT_EQUAL(1, int((++copy.pktList[0].varMap.lower_bound(0))->second));
    // pkt 0, byte 0 -> var 1
    
    CPPUNIT_ASSERT(copy.pktList[1].varMap.size() == 1);
    CPPUNIT_ASSERT(copy.pktList[1].varMap.count(0) == 1); // pkt 1 byte 0
    CPPUNIT_ASSERT_EQUAL(1, int(copy.pktList[1].varMap.lower_bound(0)->second));
    // pkt 1, byte 0 -> var 1
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestFlowConfigProxy);
