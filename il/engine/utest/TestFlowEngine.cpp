#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "PlaylistInstance.h"
#include "FlowEngine.h"

#include "MockFlowConfigProxy.h"
#include "MockSocketManager.h"
#include "MockTimerManager.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class TestFlowEngine : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestFlowEngine);
    CPPUNIT_TEST(testCreateFlow);
    CPPUNIT_TEST(testUpdateFlow);
    CPPUNIT_TEST(testDeleteFlow);
    CPPUNIT_TEST(testLinkLoopInfo2Pkts);
    CPPUNIT_TEST(testAttackInitialization);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) 
    {
        tm = new MockTimerMgr(); 
        fe = new FlowEngine(*tm); 
    }

    void tearDown(void) 
    {
        delete fe;
        delete tm;
    }

protected:
    void testCreateFlow(void);
    void testUpdateFlow(void);
    void testDeleteFlow(void);
    void testLinkLoopInfo2Pkts(void);
    void testAttackInitialization(void);

private:
    FlowEngine * fe;
    MockTimerMgr * tm;
};

static bool operator==(const FlowEngine::Packet_t& a, const FlowEngine::Packet_t& b)
{
    if (a.pktDelay != b.pktDelay)
        return false;

    if (a.clientTx != b.clientTx)
        return false;

    if (a.data.size() != b.data.size())
        return false;

    std::vector<uint8_t>::const_iterator aIter = a.data.begin();
    std::vector<uint8_t>::const_iterator bIter = b.data.begin();

    while (aIter != a.data.end())
    {
        if (*aIter != *bIter)
            return false;

        ++aIter;
        ++bIter;
    }

    return true;
}

static bool operator==(const FlowEngine::Config_t& a, const FlowEngine::Config_t& b)
{
    if (a.pktList.size() != b.pktList.size())
        return false;

    std::vector<FlowEngine::Packet_t>::const_iterator aIter = a.pktList.begin();
    std::vector<FlowEngine::Packet_t>::const_iterator bIter = b.pktList.begin();

    while (aIter != a.pktList.end())
    {
        if (!(*aIter == *bIter))
            return false;

        ++aIter;
        ++bIter;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowEngine::testCreateFlow(void)
{
    uint32_t handle = 1;
    FlowEngine::Config_t config;

    CPPUNIT_ASSERT(fe->GetFlow(handle) == 0);

    fe->UpdateFlow(handle, MockFlowConfigProxy(config));

    CPPUNIT_ASSERT(fe->GetFlow(handle));
    CPPUNIT_ASSERT(*fe->GetFlow(handle) == config);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowEngine::testUpdateFlow(void)
{
    uint32_t handle = 1;
    FlowEngine::Config_t config;

    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(config));

    FlowEngine::Config_t config2;
    config2.pktList.resize(2);

    // change flow    
    fe->UpdateFlow(handle, MockFlowConfigProxy(config2));

    CPPUNIT_ASSERT(fe->GetFlow(handle));
    CPPUNIT_ASSERT(*fe->GetFlow(handle) == config2);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowEngine::testDeleteFlow(void)
{
    uint32_t handle = 1;
    FlowEngine::Config_t config;

    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(config));

    // delete it
    fe->DeleteFlow(handle);

    // it's gone
    CPPUNIT_ASSERT(fe->GetFlow(handle) == 0);

    // throw an error if someone deletes it again
    CPPUNIT_ASSERT_THROW(fe->DeleteFlow(1), DPG_EInternal);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowEngine::testLinkLoopInfo2Pkts(void)
{
    uint32_t handle = 1;
    FlowEngine::Config_t config;
    config.pktList.resize(5);
    config.pktList[0].data.push_back(1);
    config.pktList[0].pktDelay = 100;
    config.pktList[0].clientTx = true;
    config.pktList[1].data.push_back(2);
    config.pktList[1].pktDelay = 101;
    config.pktList[1].clientTx = false;
    config.pktList[2].data.push_back(3);
    config.pktList[2].pktDelay = 102;
    config.pktList[2].clientTx = true;
    config.pktList[3].data.push_back(4);
    config.pktList[3].pktDelay = 103;
    config.pktList[3].clientTx = false;
    config.pktList[4].data.push_back(5);
    config.pktList[4].pktDelay = 104;
    config.pktList[4].clientTx = true;

    FlowConfig::LoopInfo firstLoop; // from pkt 2 to pkt 2, loop 3 times
    firstLoop.count = 3;
    firstLoop.begIdx = 2;
    config.loopMap.insert(std::make_pair(2,firstLoop));

    fe->UpdateFlow(handle, MockFlowConfigProxy(config));

    const FlowEngine::Config_t * flow = fe->GetFlow(handle);
    CPPUNIT_ASSERT(flow->pktList[2].loopInfo!=0);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2),flow->pktList[2].loopInfo->begIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3),flow->pktList[2].loopInfo->count);

}

///////////////////////////////////////////////////////////////////////////////

void TestFlowEngine::testAttackInitialization(void)
{
    uint32_t handle = 1;
    FlowEngine::Config_t config;
    config.playType = FlowConfig::P_ATTACK;
    config.pktList.resize(5);
    config.pktList[0].data.push_back(1);
    config.pktList[0].pktDelay = 100;
    config.pktList[0].clientTx = true;
    config.pktList[1].data.push_back(2);
    config.pktList[1].pktDelay = 101;
    config.pktList[1].clientTx = false;
    config.pktList[2].data.push_back(3);
    config.pktList[2].pktDelay = 102;
    config.pktList[2].clientTx = true;
    config.pktList[3].data.push_back(4);
    config.pktList[3].pktDelay = 103;
    config.pktList[3].clientTx = false;
    config.pktList[4].data.push_back(5);
    config.pktList[4].pktDelay = 104;
    config.pktList[4].clientTx = true;

    FlowConfig::LoopInfo firstLoop; // from pkt 2 to pkt 0, loop 2 times
    firstLoop.count = 2;
    firstLoop.begIdx = 0;
    config.loopMap.insert(std::make_pair(2,firstLoop));

    fe->UpdateFlow(handle, MockFlowConfigProxy(config));

    const FlowEngine::Config_t * flow = fe->GetFlow(handle);

    // correctly grouped into two lists
    CPPUNIT_ASSERT_EQUAL(size_t(3),flow->mClientPayload.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2),flow->mServerPayload.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0),flow->mClientPayload[0]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2),flow->mClientPayload[1]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(4),flow->mClientPayload[2]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1),flow->mServerPayload[0]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3),flow->mServerPayload[1]);
    CPPUNIT_ASSERT_EQUAL(size_t(5),flow->mNumOfClientPktsToPlay);
    CPPUNIT_ASSERT_EQUAL(size_t(3),flow->mNumOfServerPktsToPlay);

    // loop info correctly appended to the packets
    CPPUNIT_ASSERT_EQUAL(size_t(2),flow->loopMap.size());
    CPPUNIT_ASSERT(flow->pktList[0].loopInfo==0);
    CPPUNIT_ASSERT(flow->pktList[1].loopInfo!=0);
    CPPUNIT_ASSERT(flow->pktList[2].loopInfo!=0);
    CPPUNIT_ASSERT(flow->pktList[1].loopInfo!=flow->pktList[2].loopInfo);
    CPPUNIT_ASSERT(flow->pktList[3].loopInfo==0);
    CPPUNIT_ASSERT(flow->pktList[4].loopInfo==0);

    // begIdx should be updated to use grouped lists, counts remain the same
    CPPUNIT_ASSERT_EQUAL(uint16_t(0),flow->pktList[1].loopInfo->begIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2),flow->pktList[1].loopInfo->count);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0),flow->pktList[2].loopInfo->begIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2),flow->pktList[2].loopInfo->count);

    // clear the loop
    config.loopMap.clear();
    fe->UpdateFlow(handle, MockFlowConfigProxy(config));

    // correctly grouped into two lists
    CPPUNIT_ASSERT_EQUAL(size_t(3),flow->mClientPayload.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2),flow->mServerPayload.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0),flow->mClientPayload[0]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2),flow->mClientPayload[1]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(4),flow->mClientPayload[2]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1),flow->mServerPayload[0]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3),flow->mServerPayload[1]);
    CPPUNIT_ASSERT_EQUAL(size_t(3),flow->mNumOfClientPktsToPlay);
    CPPUNIT_ASSERT_EQUAL(size_t(2),flow->mNumOfServerPktsToPlay);

    // loop info cleared
    CPPUNIT_ASSERT_EQUAL(size_t(0),flow->loopMap.size());
    CPPUNIT_ASSERT(flow->pktList[0].loopInfo==0);
    CPPUNIT_ASSERT(flow->pktList[1].loopInfo==0);
    CPPUNIT_ASSERT(flow->pktList[2].loopInfo==0);
    CPPUNIT_ASSERT(flow->pktList[3].loopInfo==0);
    CPPUNIT_ASSERT(flow->pktList[4].loopInfo==0);

    // change the loop
    config.loopMap.clear();
    FlowConfig::LoopInfo secondLoop; // from pkt 3 to pkt 1, loop 5 times
    secondLoop.count = 5;
    secondLoop.begIdx = 1;
    config.loopMap.insert(std::make_pair(3,secondLoop));

    fe->UpdateFlow(handle, MockFlowConfigProxy(config));

    // correctly grouped into two lists
    CPPUNIT_ASSERT_EQUAL(size_t(3),flow->mClientPayload.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2),flow->mServerPayload.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0),flow->mClientPayload[0]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2),flow->mClientPayload[1]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(4),flow->mClientPayload[2]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1),flow->mServerPayload[0]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3),flow->mServerPayload[1]);
    CPPUNIT_ASSERT_EQUAL(size_t(7),flow->mNumOfClientPktsToPlay);
    CPPUNIT_ASSERT_EQUAL(size_t(10),flow->mNumOfServerPktsToPlay);

    // loop info correctly appended to the packets
    CPPUNIT_ASSERT_EQUAL(size_t(2),flow->loopMap.size());
    CPPUNIT_ASSERT(flow->pktList[0].loopInfo==0);
    CPPUNIT_ASSERT(flow->pktList[1].loopInfo==0);
    CPPUNIT_ASSERT(flow->pktList[2].loopInfo!=0);
    CPPUNIT_ASSERT(flow->pktList[3].loopInfo!=0);
    CPPUNIT_ASSERT(flow->pktList[2].loopInfo!=flow->pktList[3].loopInfo);
    CPPUNIT_ASSERT(flow->pktList[4].loopInfo==0);

    // begIdx should be updated to use grouped lists, counts remain the same
    CPPUNIT_ASSERT_EQUAL(uint16_t(1),flow->pktList[2].loopInfo->begIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(5),flow->pktList[2].loopInfo->count);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0),flow->pktList[3].loopInfo->begIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(5),flow->pktList[3].loopInfo->count);

    // change the loop
    config.loopMap.clear();
    secondLoop.count = 3; // from pkt 2 to pkt 2, loop 3 time
    secondLoop.begIdx = 2;
    config.loopMap.insert(std::make_pair(2,secondLoop));

    fe->UpdateFlow(handle, MockFlowConfigProxy(config));

    // correctly grouped into two lists
    CPPUNIT_ASSERT_EQUAL(size_t(3),flow->mClientPayload.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2),flow->mServerPayload.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0),flow->mClientPayload[0]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2),flow->mClientPayload[1]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(4),flow->mClientPayload[2]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1),flow->mServerPayload[0]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3),flow->mServerPayload[1]);
    CPPUNIT_ASSERT_EQUAL(size_t(5),flow->mNumOfClientPktsToPlay);
    CPPUNIT_ASSERT_EQUAL(size_t(2),flow->mNumOfServerPktsToPlay);

    // loop info correctly appended to the packets
    CPPUNIT_ASSERT_EQUAL(size_t(1),flow->loopMap.size());
    CPPUNIT_ASSERT(flow->pktList[0].loopInfo==0);
    CPPUNIT_ASSERT(flow->pktList[1].loopInfo==0);
    CPPUNIT_ASSERT(flow->pktList[2].loopInfo!=0);
    CPPUNIT_ASSERT(flow->pktList[3].loopInfo==0);
    CPPUNIT_ASSERT(flow->pktList[4].loopInfo==0);

    // begIdx should be updated to use grouped lists, counts remain the same
    CPPUNIT_ASSERT_EQUAL(uint16_t(1),flow->pktList[2].loopInfo->begIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3),flow->pktList[2].loopInfo->count);

    // change the loop
    config.loopMap.clear();
    secondLoop.count = 5; // from pkt 1 to pkt 1, loop 5 time
    secondLoop.begIdx = 1;
    config.loopMap.insert(std::make_pair(1,secondLoop));

    fe->UpdateFlow(handle, MockFlowConfigProxy(config));

    // correctly grouped into two lists
    CPPUNIT_ASSERT_EQUAL(size_t(3),flow->mClientPayload.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2),flow->mServerPayload.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0),flow->mClientPayload[0]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2),flow->mClientPayload[1]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(4),flow->mClientPayload[2]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1),flow->mServerPayload[0]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3),flow->mServerPayload[1]);
    CPPUNIT_ASSERT_EQUAL(size_t(3),flow->mNumOfClientPktsToPlay);
    CPPUNIT_ASSERT_EQUAL(size_t(6),flow->mNumOfServerPktsToPlay);

    // loop info correctly appended to the packets
    CPPUNIT_ASSERT_EQUAL(size_t(1),flow->loopMap.size());
    CPPUNIT_ASSERT(flow->pktList[0].loopInfo==0);
    CPPUNIT_ASSERT(flow->pktList[1].loopInfo!=0);
    CPPUNIT_ASSERT(flow->pktList[2].loopInfo==0);
    CPPUNIT_ASSERT(flow->pktList[3].loopInfo==0);
    CPPUNIT_ASSERT(flow->pktList[4].loopInfo==0);

    // begIdx should be updated to use grouped lists, counts remain the same
    CPPUNIT_ASSERT_EQUAL(uint16_t(0),flow->pktList[1].loopInfo->begIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(5),flow->pktList[1].loopInfo->count);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestFlowEngine);
