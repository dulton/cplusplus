#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "StreamLoopHandler.h"
#include "AttackLoopHandler.h"
#include "StreamPlaylistInstance.h"
#include "AttackPlaylistInstance.h"
#include "FlowEngine.h"
#include "PlaylistEngine.h"
#include "FlowInstance.h"
#include "StreamInstance.h"
#include "AttackInstance.h"

#include "MockFlowConfigProxy.h"
#include "MockSocketManager.h"
#include "MockTimerManager.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class TestFlowLoopHandler : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestFlowLoopHandler);
    CPPUNIT_TEST(testInitialization);
    CPPUNIT_TEST(testStartStreamLoop);
    CPPUNIT_TEST(testRepeatStreams);
    CPPUNIT_TEST(testSuccessfulStreamLoop);
    CPPUNIT_TEST(testFailedStreamLoop);

    CPPUNIT_TEST(testStartAttackLoop);
    CPPUNIT_TEST(testSuccessfulAttackLoop);
    CPPUNIT_TEST(testFailedAttackLoop);
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

    void testInitialization(void);
    void testStartStreamLoop(void);
    void testRepeatStreams(void);
    void testSuccessfulStreamLoop(void);
    void testFailedStreamLoop(void);

    void testStartAttackLoop(void);
    void testSuccessfulAttackLoop(void);
    void testFailedAttackLoop(void);

private:
    FlowEngine * fe;
    MockSocketMgr * sm;
    MockTimerMgr * tm;
    PlaylistEngine * pe;
};

class MockStreamInstance : public StreamInstance
{
  public:
    MockStreamInstance(const StreamInstance& other) : StreamInstance(other) {}

    void Start() { mState = STARTED;}

    bool IsStarted() { return (mState == STARTED); }

};

class MockAttackInstance : public AttackInstance
{
  public:
    MockAttackInstance(const AttackInstance& other) : AttackInstance(other) {}

    void Start() { mState = STARTED; }

    bool IsStarted() { return (mState == STARTED); }
};

class MockStreamPLInstForLoop : public StreamPlaylistInstance
{
  public:
    MockStreamPLInstForLoop(PlaylistEngine& pEngine, AbstSocketManager& socketMgr, const PlaylistConfig& config, handle_t handle, bool client) :
                            StreamPlaylistInstance(pEngine, socketMgr, config, handle, ModifierBlock(config, 0), client),
                            mTotalLoops(config.loopMap.size()),
                            mFailedLoops(0),
                            mSuccessfulLoops(0)
    {};

    uint32_t GetCurrTimeMsec()
    {
        mTimeStamp += 10;  // this is to simulate different values as each time this is called
        return mTimeStamp;
    }

    void Start()
    {
        for (std::vector<flowInstSharedPtr_t>::iterator flow_it = mFlows.begin(); flow_it != mFlows.end(); flow_it++)
        {
            StartData((*flow_it)->GetIndex());
        }
    }

    void NotifyLoopDone (uint16_t loopId)
    {
        mSuccessfulLoops++;
    }
    void NotifyLoopFailed (uint16_t loopId)
    {
        mFailedLoops++;
    }

    uint16_t mTotalLoops;
    uint16_t mFailedLoops;
    uint16_t mSuccessfulLoops;

    static uint32_t mTimeStamp;
};

class MockAttackPLInstForLoop : public AttackPlaylistInstance
{
  public:
    MockAttackPLInstForLoop(PlaylistEngine& pEngine, AbstSocketManager& socketMgr, const PlaylistConfig& config, bool client) :
                            AttackPlaylistInstance(pEngine, socketMgr, config, ModifierBlock(config, 0), client),
                            mTotalLoops(config.loopMap.size()),
                            mFailedLoops(0),
                            mSuccessfulLoops(0)
    {};

    void DoNextAttack(bool currSuccessful)
    {
        if (!mFlows.at(mCurrAttackIdx)->IsInFlowLoop()
              && mCurrAttackIdx < (mFlows.size() - 1)
              && !mFlows.at(mCurrAttackIdx + 1)->IsInFlowLoop())
        {
            ProcessAttack(++mCurrAttackIdx);
        }
    }

    void ProcessAttack(size_t flowIndex)
    {
        mCurrAttackIdx = flowIndex;
        StartData(flowIndex);
    }

    void NotifyLoopDone (uint16_t loopId)
    {
        mSuccessfulLoops++;
    }
    void NotifyLoopFailed (uint16_t loopId)
    {
        mFailedLoops++;
    }

    uint16_t mTotalLoops;
    uint16_t mFailedLoops;
    uint16_t mSuccessfulLoops;
};

uint32_t MockStreamPLInstForLoop::mTimeStamp = 0;

///////////////////////////////////////////////////////////////////////////////

void TestFlowLoopHandler::testInitialization(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.type = PlaylistConfig::STREAM_ONLY;

    config.flows.resize(4);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 100;
    config.flows[0].dataDelay = 10;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 10;
    config.flows[1].dataDelay = 20;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 120;
    config.flows[2].dataDelay = 30;

    config.flows[3].flowHandle = flowHandle;
    config.flows[3].startTime = 130;
    config.flows[3].dataDelay = 0;

    // stream loop, includes flow #1, #2, and #3, loop 2 times
    PlaylistConfig::LoopInfo loop;
    loop.count = 2;
    loop.begIdx = 1;
    config.loopMap.insert(std::make_pair(3,loop));

    // create playlist 
    pe->UpdatePlaylist(pl_handle, config);

    const PlaylistEngine::Config_t *pc = pe->GetPlaylist(pl_handle);

    MockStreamPLInstForLoop * pi = new MockStreamPLInstForLoop(*pe, *sm, *pc, pl_handle, true);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(1).get())));
    MockStreamInstance * mockFlowInst2 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(2).get())));
    MockStreamInstance * mockFlowInst3 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(3).get())));
    pi->mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi->mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi->mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);
    pi->mFlows.at(3) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst3);

    // a StreamLoopHandler object should be created within the playlist instance
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mFlowLoopMap.size());

    CPPUNIT_ASSERT(pi->mFlows.at(0)->mFlowLoopHandler==0);

    PlaylistInstance::flowLpHdlrSharedPtr_t loopHdlr = pi->mFlows.at(1)->mFlowLoopHandler;

    CPPUNIT_ASSERT(loopHdlr!=0);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mBeginFlowIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), loopHdlr->mEndFlowIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mMaxCount);
    // unknown yet
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), loopHdlr->mIdxOfFirstFlowInst);
    CPPUNIT_ASSERT_EQUAL(size_t(4), loopHdlr->mRelativeDataDelay.size());

    CPPUNIT_ASSERT(loopHdlr->mStatus.state == FlowLoopHandler::WAIT_TO_START);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedIterations);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), loopHdlr->mStatus.numOfRemainedFlowsInThisIteration);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), loopHdlr->mStatus.lastStartTime);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), loopHdlr->mStatus.lastStopTime);

//     CPPUNIT_ASSERT(loopHdlr->mPlaylistInst == pi);

    CPPUNIT_ASSERT(pi->mFlows.at(2)->mFlowLoopHandler==loopHdlr);
    CPPUNIT_ASSERT(pi->mFlows.at(3)->mFlowLoopHandler==loopHdlr);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowLoopHandler::testStartStreamLoop(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.type = PlaylistConfig::STREAM_ONLY;

    config.flows.resize(4);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 100;
    config.flows[0].dataDelay = 10;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 10;
    config.flows[1].dataDelay = 20;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 120;
    config.flows[2].dataDelay = 30;

    config.flows[3].flowHandle = flowHandle;
    config.flows[3].startTime = 130;
    config.flows[3].dataDelay = 0;

    // stream loop, includes flow #1, #2, and #3, loop 2 times
    PlaylistConfig::LoopInfo loop;
    loop.count = 2;
    loop.begIdx = 1;
    config.loopMap.insert(std::make_pair(3,loop));

    // create playlist 
    pe->UpdatePlaylist(pl_handle, config);

    const PlaylistEngine::Config_t *pc = pe->GetPlaylist(pl_handle);

    MockStreamPLInstForLoop * pi = new MockStreamPLInstForLoop(*pe, *sm, *pc, pl_handle, true);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(1).get())));
    MockStreamInstance * mockFlowInst2 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(2).get())));
    MockStreamInstance * mockFlowInst3 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(3).get())));
    pi->mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi->mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi->mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);
    pi->mFlows.at(3) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst3);

    PlaylistInstance::flowLpHdlrSharedPtr_t loopHdlr = pi->mFlows.at(1)->mFlowLoopHandler;

    pi->Start();

    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mBeginFlowIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), loopHdlr->mEndFlowIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mMaxCount);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mIdxOfFirstFlowInst);
    CPPUNIT_ASSERT_EQUAL(size_t(4), loopHdlr->mRelativeDataDelay.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), loopHdlr->mRelativeDataDelay[0]);
    CPPUNIT_ASSERT_EQUAL(size_t(0), loopHdlr->mRelativeDataDelay[1]);
    CPPUNIT_ASSERT_EQUAL(size_t(10), loopHdlr->mRelativeDataDelay[2]);
    CPPUNIT_ASSERT_EQUAL(size_t(20), loopHdlr->mRelativeDataDelay[3]);

    CPPUNIT_ASSERT(loopHdlr->mStatus.state == FlowLoopHandler::STARTED);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedIterations);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), loopHdlr->mStatus.numOfRemainedFlowsInThisIteration);
    CPPUNIT_ASSERT_EQUAL(uint32_t(10), loopHdlr->mStatus.lastStartTime);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), loopHdlr->mStatus.lastStopTime);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowLoopHandler::testRepeatStreams(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.type = PlaylistConfig::STREAM_ONLY;

    config.flows.resize(4);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 100;
    config.flows[0].dataDelay = 10;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 10;
    config.flows[1].dataDelay = 20;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 120;
    config.flows[2].dataDelay = 30;

    config.flows[3].flowHandle = flowHandle;
    config.flows[3].startTime = 130;
    config.flows[3].dataDelay = 0;

    // stream loop, includes flow #1, #2, and #3, loop 2 times
    PlaylistConfig::LoopInfo loop;
    loop.count = 2;
    loop.begIdx = 1;
    config.loopMap.insert(std::make_pair(3,loop));

    // create playlist 
    pe->UpdatePlaylist(pl_handle, config);

    const PlaylistEngine::Config_t *pc = pe->GetPlaylist(pl_handle);

    MockStreamPLInstForLoop * pi = new MockStreamPLInstForLoop(*pe, *sm, *pc, pl_handle, true);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(1).get())));
    MockStreamInstance * mockFlowInst2 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(2).get())));
    MockStreamInstance * mockFlowInst3 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(3).get())));
    pi->mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi->mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi->mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);
    pi->mFlows.at(3) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst3);

    PlaylistInstance::flowLpHdlrSharedPtr_t loopHdlr = pi->mFlows.at(1)->mFlowLoopHandler;

    pi->Start(); // this mock playlist lets all data flows start right away

    loopHdlr->FlowInLoopDone(2);
    // nothing should change except number of remained flows in this iteration
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mBeginFlowIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), loopHdlr->mEndFlowIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mMaxCount);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mIdxOfFirstFlowInst);
    CPPUNIT_ASSERT_EQUAL(size_t(4), loopHdlr->mRelativeDataDelay.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), loopHdlr->mRelativeDataDelay[0]);
    CPPUNIT_ASSERT_EQUAL(size_t(0), loopHdlr->mRelativeDataDelay[1]);
    CPPUNIT_ASSERT_EQUAL(size_t(10), loopHdlr->mRelativeDataDelay[2]);
    CPPUNIT_ASSERT_EQUAL(size_t(20), loopHdlr->mRelativeDataDelay[3]);
    CPPUNIT_ASSERT(loopHdlr->mStatus.state == FlowLoopHandler::STARTED);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedIterations);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedFlowsInThisIteration);

    loopHdlr->FlowInLoopDone(3);
    // nothing should change except for number of remained flows in this iteration
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mBeginFlowIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), loopHdlr->mEndFlowIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mMaxCount);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mIdxOfFirstFlowInst);
    CPPUNIT_ASSERT_EQUAL(size_t(4), loopHdlr->mRelativeDataDelay.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), loopHdlr->mRelativeDataDelay[0]);
    CPPUNIT_ASSERT_EQUAL(size_t(0), loopHdlr->mRelativeDataDelay[1]);
    CPPUNIT_ASSERT_EQUAL(size_t(10), loopHdlr->mRelativeDataDelay[2]);
    CPPUNIT_ASSERT_EQUAL(size_t(20), loopHdlr->mRelativeDataDelay[3]);
    CPPUNIT_ASSERT(loopHdlr->mStatus.state == FlowLoopHandler::STARTED);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedIterations);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mStatus.numOfRemainedFlowsInThisIteration);

    loopHdlr->FlowInLoopDone(1);

    // This iteration should be done now;
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mBeginFlowIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), loopHdlr->mEndFlowIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mMaxCount);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mIdxOfFirstFlowInst);
    CPPUNIT_ASSERT_EQUAL(size_t(4), loopHdlr->mRelativeDataDelay.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), loopHdlr->mRelativeDataDelay[0]);
    CPPUNIT_ASSERT_EQUAL(size_t(0), loopHdlr->mRelativeDataDelay[1]);
    CPPUNIT_ASSERT_EQUAL(size_t(10), loopHdlr->mRelativeDataDelay[2]);
    CPPUNIT_ASSERT_EQUAL(size_t(20), loopHdlr->mRelativeDataDelay[3]);
    CPPUNIT_ASSERT(loopHdlr->mStatus.state == FlowLoopHandler::STARTED);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mStatus.numOfRemainedIterations);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), loopHdlr->mStatus.numOfRemainedFlowsInThisIteration);

    // all three streams should be scheduled to repeat
    CPPUNIT_ASSERT(pi->mTimers[1]!=tm->InvalidTimerHandle());
    CPPUNIT_ASSERT(pi->mTimers[2]!=tm->InvalidTimerHandle());
    CPPUNIT_ASSERT(pi->mTimers[3]!=tm->InvalidTimerHandle());
    CPPUNIT_ASSERT_EQUAL(loopHdlr->mRelativeDataDelay[1] + DELAY_IN_MS_BETWEEN_STREAMLOOP_ITERATIONS, tm->GetTimers()[pi->mTimers[1]].msDelay);
    CPPUNIT_ASSERT_EQUAL(loopHdlr->mRelativeDataDelay[2] + DELAY_IN_MS_BETWEEN_STREAMLOOP_ITERATIONS, tm->GetTimers()[pi->mTimers[2]].msDelay);
    CPPUNIT_ASSERT_EQUAL(loopHdlr->mRelativeDataDelay[3] + DELAY_IN_MS_BETWEEN_STREAMLOOP_ITERATIONS, tm->GetTimers()[pi->mTimers[3]].msDelay);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowLoopHandler::testSuccessfulStreamLoop(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.type = PlaylistConfig::STREAM_ONLY;

    config.flows.resize(4);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 100;
    config.flows[0].dataDelay = 10;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 10;
    config.flows[1].dataDelay = 20;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 120;
    config.flows[2].dataDelay = 30;

    config.flows[3].flowHandle = flowHandle;
    config.flows[3].startTime = 130;
    config.flows[3].dataDelay = 0;

    // stream loop, includes flow #1, #2, and #3, loop 2 times
    PlaylistConfig::LoopInfo loop;
    loop.count = 2;
    loop.begIdx = 1;
    config.loopMap.insert(std::make_pair(3,loop));

    // create playlist 
    pe->UpdatePlaylist(pl_handle, config);

    const PlaylistEngine::Config_t *pc = pe->GetPlaylist(pl_handle);

    MockStreamPLInstForLoop * pi = new MockStreamPLInstForLoop(*pe, *sm, *pc, pl_handle, true);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(1).get())));
    MockStreamInstance * mockFlowInst2 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(2).get())));
    MockStreamInstance * mockFlowInst3 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(3).get())));
    pi->mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi->mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi->mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);
    pi->mFlows.at(3) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst3);

    PlaylistInstance::flowLpHdlrSharedPtr_t loopHdlr = pi->mFlows.at(1)->mFlowLoopHandler;

    pi->Start(); // this mock playlist lets all data flows start right away
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi->mTotalLoops);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pi->mSuccessfulLoops);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pi->mFailedLoops);

    loopHdlr->FlowInLoopDone(2);
    loopHdlr->FlowInLoopDone(3);
    loopHdlr->FlowInLoopDone(1);

    // This iteration should be done now;
    // all three streams should be scheduled to repeat
    CPPUNIT_ASSERT(pi->mTimers[1]!=tm->InvalidTimerHandle());
    CPPUNIT_ASSERT(pi->mTimers[2]!=tm->InvalidTimerHandle());
    CPPUNIT_ASSERT(pi->mTimers[3]!=tm->InvalidTimerHandle());

    tm->GetTimers()[pi->mTimers[1]].delegate();
    tm->GetTimers()[pi->mTimers[2]].delegate();
    tm->GetTimers()[pi->mTimers[3]].delegate();

    loopHdlr->FlowInLoopDone(2);
    loopHdlr->FlowInLoopDone(3);
    loopHdlr->FlowInLoopDone(1);

    // Now the whole loop is done.
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi->mTotalLoops);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi->mSuccessfulLoops);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pi->mFailedLoops);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowLoopHandler::testFailedStreamLoop(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.type = PlaylistConfig::STREAM_ONLY;

    config.flows.resize(4);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 100;
    config.flows[0].dataDelay = 10;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 10;
    config.flows[1].dataDelay = 20;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 120;
    config.flows[2].dataDelay = 30;

    config.flows[3].flowHandle = flowHandle;
    config.flows[3].startTime = 130;
    config.flows[3].dataDelay = 0;

    // stream loop, includes flow #1, #2, and #3, loop 2 times
    PlaylistConfig::LoopInfo loop;
    loop.count = 2;
    loop.begIdx = 1;
    config.loopMap.insert(std::make_pair(3,loop));

    // create playlist 
    pe->UpdatePlaylist(pl_handle, config);

    const PlaylistEngine::Config_t *pc = pe->GetPlaylist(pl_handle);

    MockStreamPLInstForLoop * pi = new MockStreamPLInstForLoop(*pe, *sm, *pc, pl_handle, true);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(1).get())));
    MockStreamInstance * mockFlowInst2 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(2).get())));
    MockStreamInstance * mockFlowInst3 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi->mFlows.at(3).get())));
    pi->mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi->mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi->mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);
    pi->mFlows.at(3) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst3);

    PlaylistInstance::flowLpHdlrSharedPtr_t loopHdlr = pi->mFlows.at(1)->mFlowLoopHandler;

    pi->Start(); // this mock playlist lets all data flows start right away
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi->mTotalLoops);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pi->mSuccessfulLoops);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pi->mFailedLoops);

    loopHdlr->FlowInLoopDone(2);
    loopHdlr->FlowInLoopDone(3);
    loopHdlr->FlowInLoopDone(1);

    // This iteration should be done now;
    // all three streams should be scheduled to repeat
    CPPUNIT_ASSERT(pi->mTimers[1]!=tm->InvalidTimerHandle());
    CPPUNIT_ASSERT(pi->mTimers[2]!=tm->InvalidTimerHandle());
    CPPUNIT_ASSERT(pi->mTimers[3]!=tm->InvalidTimerHandle());

    tm->GetTimers()[pi->mTimers[1]].delegate();
    tm->GetTimers()[pi->mTimers[2]].delegate();
    tm->GetTimers()[pi->mTimers[3]].delegate();

    loopHdlr->FlowInLoopDone(2);
    loopHdlr->FlowInLoopFailed(3);

    // Now the whole loop is done.
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi->mTotalLoops);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pi->mSuccessfulLoops);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi->mFailedLoops);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowLoopHandler::testStartAttackLoop(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.type = PlaylistConfig::ATTACK_ONLY;

    config.flows.resize(4);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 100;
    config.flows[0].dataDelay = 10;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 10;
    config.flows[1].dataDelay = 20;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 120;
    config.flows[2].dataDelay = 30;

    config.flows[3].flowHandle = flowHandle;
    config.flows[3].startTime = 130;
    config.flows[3].dataDelay = 0;

    // attack loop, includes flow #1, #2, loop 3 times
    PlaylistConfig::LoopInfo loop;
    loop.count = 3;
    loop.begIdx = 1;
    config.loopMap.insert(std::make_pair(2,loop));

    // create playlist 
    pe->UpdatePlaylist(pl_handle, config);

    const PlaylistEngine::Config_t *pc = pe->GetPlaylist(pl_handle);

    MockAttackPLInstForLoop * pi = new MockAttackPLInstForLoop(*pe, *sm, *pc, true);

    // replace FlowInstances with Mock
    MockAttackInstance * mockFlowInst0 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi->mFlows.at(0).get())));
    MockAttackInstance * mockFlowInst1 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi->mFlows.at(1).get())));
    MockAttackInstance * mockFlowInst2 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi->mFlows.at(2).get())));
    MockAttackInstance * mockFlowInst3 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi->mFlows.at(3).get())));
    pi->mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi->mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi->mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);
    pi->mFlows.at(3) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst3);

    PlaylistInstance::flowLpHdlrSharedPtr_t loopHdlr = pi->mFlows.at(1)->mFlowLoopHandler;

    pi->Start();

    CPPUNIT_ASSERT(loopHdlr.get()!=0);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mBeginFlowIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mEndFlowIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), loopHdlr->mMaxCount);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), loopHdlr->mIdxOfFirstFlowInst); //TODO:update?
    CPPUNIT_ASSERT_EQUAL(size_t(0), loopHdlr->mRelativeDataDelay.size());
    CPPUNIT_ASSERT(loopHdlr->mStatus.state == FlowLoopHandler::WAIT_TO_START);

    pi->FlowDone(0,true); // This is the mock one, which will not trigger the loop

    loopHdlr->StartLoop(1);
    // now should be in the loop
    CPPUNIT_ASSERT(loopHdlr->mStatus.state == FlowLoopHandler::STARTED);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), loopHdlr->mStatus.numOfRemainedIterations);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedFlowsInThisIteration);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mStatus.currFlowIdx);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowLoopHandler::testSuccessfulAttackLoop(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.type = PlaylistConfig::ATTACK_ONLY;

    config.flows.resize(4);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 100;
    config.flows[0].dataDelay = 10;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 10;
    config.flows[1].dataDelay = 20;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 120;
    config.flows[2].dataDelay = 30;

    config.flows[3].flowHandle = flowHandle;
    config.flows[3].startTime = 130;
    config.flows[3].dataDelay = 0;

    // attack loop, includes flow #1, #2, loop 2 times
    PlaylistConfig::LoopInfo loop;
    loop.count = 2;
    loop.begIdx = 1;
    config.loopMap.insert(std::make_pair(2,loop));

    // create playlist 
    pe->UpdatePlaylist(pl_handle, config);

    const PlaylistEngine::Config_t *pc = pe->GetPlaylist(pl_handle);

    MockAttackPLInstForLoop * pi = new MockAttackPLInstForLoop(*pe, *sm, *pc, true);

    // replace FlowInstances with Mock
    MockAttackInstance * mockFlowInst0 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi->mFlows.at(0).get())));
    MockAttackInstance * mockFlowInst1 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi->mFlows.at(1).get())));
    MockAttackInstance * mockFlowInst2 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi->mFlows.at(2).get())));
    MockAttackInstance * mockFlowInst3 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi->mFlows.at(3).get())));
    pi->mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi->mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi->mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);
    pi->mFlows.at(3) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst3);

    PlaylistInstance::flowLpHdlrSharedPtr_t loopHdlr = pi->mFlows.at(1)->mFlowLoopHandler;

    pi->Start();

    CPPUNIT_ASSERT(loopHdlr.get()!=0);

    pi->FlowDone(0,true); // This is the mock one, which will not trigger the loop

    loopHdlr->StartLoop(1);
    // now should be in the loop
    CPPUNIT_ASSERT(loopHdlr->mStatus.state == FlowLoopHandler::STARTED);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedIterations);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedFlowsInThisIteration);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mStatus.currFlowIdx);

    loopHdlr->FlowInLoopDone(1);
    // should start flow #2
    CPPUNIT_ASSERT(loopHdlr->mStatus.state == FlowLoopHandler::STARTED);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedIterations);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mStatus.numOfRemainedFlowsInThisIteration);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.currFlowIdx);

    pi->ProcessAttack(2);
    loopHdlr->FlowInLoopDone(2);
    // should loop back to flow #1
    CPPUNIT_ASSERT(loopHdlr->mStatus.state == FlowLoopHandler::STARTED);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mStatus.numOfRemainedIterations);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedFlowsInThisIteration);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mStatus.currFlowIdx);

    loopHdlr->FlowInLoopDone(1);
    // should start flow #2
    CPPUNIT_ASSERT(loopHdlr->mStatus.state == FlowLoopHandler::STARTED);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mStatus.numOfRemainedIterations);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mStatus.numOfRemainedFlowsInThisIteration);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.currFlowIdx);

    pi->ProcessAttack(2);
    loopHdlr->FlowInLoopDone(2);
    // should end the loop
    CPPUNIT_ASSERT(loopHdlr->mStatus.state == FlowLoopHandler::SUCCESSFUL);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedIterations);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedFlowsInThisIteration);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), loopHdlr->mStatus.currFlowIdx);

    // Now the whole loop is done.
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi->mTotalLoops);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi->mSuccessfulLoops);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pi->mFailedLoops);
}

///////////////////////////////////////////////////////////////////////////////

void TestFlowLoopHandler::testFailedAttackLoop(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.type = PlaylistConfig::ATTACK_ONLY;

    config.flows.resize(4);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 100;
    config.flows[0].dataDelay = 10;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 10;
    config.flows[1].dataDelay = 20;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 120;
    config.flows[2].dataDelay = 30;

    config.flows[3].flowHandle = flowHandle;
    config.flows[3].startTime = 130;
    config.flows[3].dataDelay = 0;

    // attack loop, includes flow #1, #2, loop 2 times
    PlaylistConfig::LoopInfo loop;
    loop.count = 2;
    loop.begIdx = 1;
    config.loopMap.insert(std::make_pair(2,loop));

    // create playlist 
    pe->UpdatePlaylist(pl_handle, config);

    const PlaylistEngine::Config_t *pc = pe->GetPlaylist(pl_handle);

    MockAttackPLInstForLoop * pi = new MockAttackPLInstForLoop(*pe, *sm, *pc, true);

    // replace FlowInstances with Mock
    MockAttackInstance * mockFlowInst0 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi->mFlows.at(0).get())));
    MockAttackInstance * mockFlowInst1 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi->mFlows.at(1).get())));
    MockAttackInstance * mockFlowInst2 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi->mFlows.at(2).get())));
    MockAttackInstance * mockFlowInst3 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi->mFlows.at(3).get())));
    pi->mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi->mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi->mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);
    pi->mFlows.at(3) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst3);

    PlaylistInstance::flowLpHdlrSharedPtr_t loopHdlr = pi->mFlows.at(1)->mFlowLoopHandler;

    pi->Start();

    CPPUNIT_ASSERT(loopHdlr.get()!=0);

    pi->FlowDone(0,true); // This is the mock one, which will not trigger the loop

    loopHdlr->StartLoop(1);
    // now should be in the loop
    CPPUNIT_ASSERT(loopHdlr->mStatus.state == FlowLoopHandler::STARTED);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedIterations);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedFlowsInThisIteration);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mStatus.currFlowIdx);

    loopHdlr->FlowInLoopDone(1);
    // should start flow #2
    CPPUNIT_ASSERT(loopHdlr->mStatus.state == FlowLoopHandler::STARTED);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedIterations);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mStatus.numOfRemainedFlowsInThisIteration);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.currFlowIdx);

    pi->ProcessAttack(2);
    loopHdlr->FlowInLoopDone(2);
    // should loop back to flow #1
    CPPUNIT_ASSERT(loopHdlr->mStatus.state == FlowLoopHandler::STARTED);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mStatus.numOfRemainedIterations);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedFlowsInThisIteration);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), loopHdlr->mStatus.currFlowIdx);

    loopHdlr->FlowInLoopFailed(1);
    // should end the loop. loop status should be reset
    CPPUNIT_ASSERT(loopHdlr->mStatus.state == FlowLoopHandler::FAILED);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedIterations);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), loopHdlr->mStatus.numOfRemainedFlowsInThisIteration);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), loopHdlr->mStatus.currFlowIdx);

    // Now the whole loop is done.
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi->mTotalLoops);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pi->mSuccessfulLoops);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi->mFailedLoops);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestFlowLoopHandler);
