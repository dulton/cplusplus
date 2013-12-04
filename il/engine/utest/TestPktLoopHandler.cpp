#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "PktLoopHandler.h"
#include "PlaylistInstance.h"
#include "FlowEngine.h"
#include "PlaylistEngine.h"
#include "AttackPlaylistInstance.h"
#include "AttackInstance.h"

#include "MockFlowConfigProxy.h"
#include "MockSocketManager.h"
#include "MockTimerManager.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class TestPktLoopHandler : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestPktLoopHandler);
    CPPUNIT_TEST(testPktLoopHandler);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) 
    {
        sm = new MockSocketMgr();
        tm = new MockTimerMgr();
        fe = new FlowEngine(*tm);
        pe = new PlaylistEngine(*fe, *tm);
    }

    void tearDown(void) 
    {
        delete fe;
        delete tm;
        delete sm;
        delete pe;
    }

protected:

    void testPktLoopHandler(void);

private:
    FlowEngine * fe;
    MockSocketMgr * sm;
    MockTimerMgr * tm;
    PlaylistEngine * pe;
};

class MockAttackPlaylistInstance : public AttackPlaylistInstance
{
  public:
    MockAttackPlaylistInstance(PlaylistEngine& pEngine, AbstSocketManager& socketMgr, const PlaylistConfig& config, bool client) :
                            AttackPlaylistInstance(pEngine, socketMgr, config, ModifierBlock(config, 0), client),
                            mTotalAttacks(config.flows.size()),
                            mFailedAttacks(0),
                            mSuccessfulAttacks(0)
    {};

    void FlowDone(size_t flowIndex, bool successful)
    {
        if (successful)
        {
            mSuccessfulAttacks++;
        }
        else
        {
            mFailedAttacks++;
        }
        mSocketMgr.Close(mFlows.at(flowIndex)->GetFd(), false);
    }

    size_t mTotalAttacks;
    size_t mFailedAttacks;
    size_t mSuccessfulAttacks;
};

///////////////////////////////////////////////////////////////////////////////

void TestPktLoopHandler::testPktLoopHandler(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.playType = FlowConfig::P_ATTACK;
    flowConfig.layer = FlowConfig::TCP;
    flowConfig.pktList.resize(5);
    flowConfig.pktList[0].data.push_back(1);
    flowConfig.pktList[0].pktDelay = 100;
    flowConfig.pktList[0].clientTx = true;
    flowConfig.pktList[1].data.push_back(2);
    flowConfig.pktList[1].pktDelay = 101;
    flowConfig.pktList[1].clientTx = false;
    flowConfig.pktList[2].data.push_back(3);
    flowConfig.pktList[2].pktDelay = 102;
    flowConfig.pktList[2].clientTx = true;
    flowConfig.pktList[3].data.push_back(4);
    flowConfig.pktList[3].pktDelay = 103;
    flowConfig.pktList[3].clientTx = false;
    flowConfig.pktList[4].data.push_back(5);
    flowConfig.pktList[4].pktDelay = 104;
    flowConfig.pktList[4].clientTx = true;

    FlowConfig::LoopInfo loop; // from pkt 3 to pkt 1, loop 3 times
    loop.count = 3;
    loop.begIdx = 1;
    flowConfig.loopMap.insert(std::make_pair(3,loop));

    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(flowConfig));

    // create a mock stream playlist instance
    PlaylistConfig playlistConfig;
    playlistConfig.type = PlaylistConfig::ATTACK_ONLY;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].minPktDelay = 200;
    playlistConfig.flows[0].serverPort = 123;

/// client side
    MockAttackPlaylistInstance * pi = new MockAttackPlaylistInstance (*pe, *sm, playlistConfig, true);

    AttackInstance *fi = static_cast<AttackInstance *>(&pi->GetFlow(0));

    CPPUNIT_ASSERT_EQUAL(size_t(3), fi->mClientPayload.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), fi->mCurrentClientPayloadIndex);

    fi->mCurrentClientPayloadIndex = fi->mTxPktLoopHandler.FindNextPktIdx(fi->mClientPayload, fi->mCurrentClientPayloadIndex);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), fi->mCurrentClientPayloadIndex);

    fi->mCurrentClientPayloadIndex = fi->mTxPktLoopHandler.FindNextPktIdx(fi->mClientPayload, fi->mCurrentClientPayloadIndex);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), fi->mCurrentClientPayloadIndex);

    fi->mCurrentClientPayloadIndex = fi->mTxPktLoopHandler.FindNextPktIdx(fi->mClientPayload, fi->mCurrentClientPayloadIndex);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), fi->mCurrentClientPayloadIndex);

    fi->mCurrentClientPayloadIndex = fi->mTxPktLoopHandler.FindNextPktIdx(fi->mClientPayload, fi->mCurrentClientPayloadIndex);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), fi->mCurrentClientPayloadIndex);

    fi->mCurrentClientPayloadIndex = fi->mTxPktLoopHandler.FindNextPktIdx(fi->mClientPayload, fi->mCurrentClientPayloadIndex);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), fi->mCurrentClientPayloadIndex);

/// server side
    MockAttackPlaylistInstance * pi2 = new MockAttackPlaylistInstance (*pe, *sm, playlistConfig, false);

    AttackInstance *fi2 = static_cast<AttackInstance *>(&pi2->GetFlow(0));

    CPPUNIT_ASSERT_EQUAL(size_t(2), fi2->mServerPayload.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), fi2->mCurrentServerPayloadIndex);

    fi2->mCurrentServerPayloadIndex = fi2->mTxPktLoopHandler.FindNextPktIdx(fi2->mServerPayload, fi2->mCurrentServerPayloadIndex);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), fi2->mCurrentServerPayloadIndex);

    fi2->mCurrentServerPayloadIndex = fi2->mTxPktLoopHandler.FindNextPktIdx(fi2->mServerPayload, fi2->mCurrentServerPayloadIndex);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), fi2->mCurrentServerPayloadIndex);

    fi2->mCurrentServerPayloadIndex = fi2->mTxPktLoopHandler.FindNextPktIdx(fi2->mServerPayload, fi2->mCurrentServerPayloadIndex);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), fi2->mCurrentServerPayloadIndex);

    fi2->mCurrentServerPayloadIndex = fi2->mTxPktLoopHandler.FindNextPktIdx(fi2->mServerPayload, fi2->mCurrentServerPayloadIndex);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), fi2->mCurrentServerPayloadIndex);

    fi2->mCurrentServerPayloadIndex = fi2->mTxPktLoopHandler.FindNextPktIdx(fi2->mServerPayload, fi2->mCurrentServerPayloadIndex);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), fi2->mCurrentServerPayloadIndex);

    fi2->mCurrentServerPayloadIndex = fi2->mTxPktLoopHandler.FindNextPktIdx(fi2->mServerPayload, fi2->mCurrentServerPayloadIndex);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), fi2->mCurrentServerPayloadIndex);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestPktLoopHandler);
