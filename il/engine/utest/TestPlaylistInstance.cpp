#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "FlowEngine.h" 

#include "PlaylistConfig.h"
#include "PlaylistEngine.h"
#include "PlaylistInstance.h"
#include "StreamPlaylistInstance.h"
#include "AttackPlaylistInstance.h"
#include "StreamInstance.h"
#include "AttackInstance.h"

#include "MockFlowConfigProxy.h"
#include "MockSocketManager.h"
#include "MockTimerManager.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class TestPlaylistInstance : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestPlaylistInstance);
    CPPUNIT_TEST(testStreamCreateTimers);
    CPPUNIT_TEST(testAttackCreateTimers);
    CPPUNIT_TEST(testCancelTimers);
    CPPUNIT_TEST(testCleanTimers);

    CPPUNIT_TEST(testZeroStartDelay);
    CPPUNIT_TEST(testDataDelay);
    CPPUNIT_TEST(testUDPDataDelay);
    CPPUNIT_TEST(testZeroDataDelay);
    CPPUNIT_TEST(testStopStreamClient);
    CPPUNIT_TEST(testStopAttackClient);
    CPPUNIT_TEST(testSuccessfulStreamClient);
    CPPUNIT_TEST(testSuccessfulAttackClient);
    CPPUNIT_TEST(testFailedStreamClient);
    CPPUNIT_TEST(testFailedAttackClient);

    CPPUNIT_TEST(testStartServer);
    CPPUNIT_TEST(testStartServerManyInstances);
    CPPUNIT_TEST(testServerDataDelay);
    CPPUNIT_TEST(testStopStreamServer);
    CPPUNIT_TEST(testStopAttackServer);
    CPPUNIT_TEST(testStreamConnClosedServer);
    CPPUNIT_TEST(testAttackConnClosedServer);
    CPPUNIT_TEST(testSuccessfulStreamServer);
    CPPUNIT_TEST(testSuccessfulAttackServer);
    CPPUNIT_TEST(testFailedStreamServer);
    CPPUNIT_TEST(testFailedAttackServer);

    CPPUNIT_TEST(testCloneStream);
    CPPUNIT_TEST(testCloneAttack);

    CPPUNIT_TEST(testSuccessfulStreamLoops);
    CPPUNIT_TEST(testSuccessfulAttackLoops);
    CPPUNIT_TEST(testFailedStreamLoops);
    CPPUNIT_TEST(testFailedAttackLoops);

    CPPUNIT_TEST(testStreamClientClsCFIN);
    CPPUNIT_TEST(testStreamClientClsSFIN);
    CPPUNIT_TEST(testStreamServerClsCFIN);
    CPPUNIT_TEST(testStreamServerClsSFIN);

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
        delete pe; 
        delete fe; 
        delete tm;
        delete sm;
    }

protected:
    void testStreamCreateTimers(void);
    void testAttackCreateTimers(void);
    void testCancelTimers(void);
    void testCleanTimers(void);

    void testZeroStartDelay(void);
    void testDataDelay(void);
    void testUDPDataDelay(void);
    void testZeroDataDelay(void);
    void testStopStreamClient(void);
    void testStopAttackClient(void);
    void testSuccessfulStreamClient(void);
    void testSuccessfulAttackClient(void);
    void testFailedStreamClient(void);
    void testFailedAttackClient(void);

    void testStartServer(void);
    void testStartServerManyInstances(void);
    void testServerDataDelay(void);
    void testStopStreamServer(void);
    void testStopAttackServer(void);
    void testStreamConnClosedServer(void);
    void testAttackConnClosedServer(void);
    void testSuccessfulStreamServer(void);
    void testSuccessfulAttackServer(void);
    void testFailedStreamServer(void);
    void testFailedAttackServer(void);

    void testCloneStream(void);
    void testCloneAttack(void);

    void testSuccessfulStreamLoops(void);
    void testSuccessfulAttackLoops(void);
    void testFailedStreamLoops(void);
    void testFailedAttackLoops(void);

    void testStreamClientClsCFIN(void);
    void testStreamClientClsSFIN(void);
    void testStreamServerClsCFIN(void);
    void testStreamServerClsSFIN(void);

private:
    FlowEngine * fe;
    PlaylistEngine * pe;
    MockSocketMgr * sm;
    MockTimerMgr * tm;
};

///////////////////////////////////////////////////////////////////////////////

class MockStreamInstance : public StreamInstance
{
  public:
    MockStreamInstance(const StreamInstance& other) : StreamInstance(other) {}

    void Start() { mState = STARTED; }

    bool IsStarted() { return (mState == STARTED); }
};

///////////////////////////////////////////////////////////////////////////////

class MockAttackInstance : public AttackInstance
{
  public:
    MockAttackInstance(const AttackInstance& other) : AttackInstance(other) {}

    void Start() { mState = STARTED; }

    bool IsStarted() { return (mState == STARTED); }
};

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testStreamCreateTimers(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig; 
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 100;
    config.flows[0].dataDelay = 10;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 10;
    config.flows[1].dataDelay = 10;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 120;
    config.flows[2].dataDelay = 10;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), true);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(1).get())));
    MockStreamInstance * mockFlowInst2 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(2).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    pi.Start();

//     should create timers for each flow
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetTimers().size());

    std::set<uint32_t> delays;
    delays.insert(tm->GetTimers()[1].msDelay);
    delays.insert(tm->GetTimers()[2].msDelay);
    delays.insert(tm->GetTimers()[3].msDelay);

    CPPUNIT_ASSERT(delays.find(10) != delays.end());
    CPPUNIT_ASSERT(delays.find(100) != delays.end());
    CPPUNIT_ASSERT(delays.find(120) != delays.end());
}

void TestPlaylistInstance::testAttackCreateTimers(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig; 
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    PlaylistConfig config;
    config.type = PlaylistConfig::ATTACK_ONLY;

    config.flows.resize(2);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].minPktDelay = 200;
    config.flows[0].socketTimeout = 1000;
    config.flows[1].flowHandle = flowHandle;
    config.flows[1].minPktDelay = 200;
    config.flows[1].socketTimeout = 0;

    AttackPlaylistInstance pi(*pe, *sm, config, ModifierBlock(config, 0), true);

    // replace FlowInstances with Mock
    MockAttackInstance * mockFlowInst0 = new MockAttackInstance(*pi.mFlows.at(0).get());
    MockAttackInstance * mockFlowInst1 = new MockAttackInstance(*pi.mFlows.at(1).get());
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);

    pi.Start();
    // should not create any timer for the attacks
    CPPUNIT_ASSERT_EQUAL(size_t(0), tm->GetTimers().size());
    // only the first attack should be started
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetConnects().size());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testCancelTimers(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig; 
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 100;
    config.flows[0].dataDelay = 10;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 10;
    config.flows[1].dataDelay = 10;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 120;
    config.flows[2].dataDelay = 10;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), true);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(1).get())));
    MockStreamInstance * mockFlowInst2 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(2).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    pi.Start();
    pi.Stop();

    // should cancel all 3 timers
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetCancelledTimers().size());

    // handles of created/cancelled timers should match
    MockTimerMgr::timerSet_t& canned(tm->GetCancelledTimers());
    MockTimerMgr::timerSet_t::iterator end = canned.end();

    CPPUNIT_ASSERT(canned.find(tm->GetTimers().begin()->first) != end);
    CPPUNIT_ASSERT(canned.find((++tm->GetTimers().begin())->first) != end);
    CPPUNIT_ASSERT(canned.find((++++tm->GetTimers().begin())->first) != end);
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testCleanTimers(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 100;
    config.flows[0].dataDelay = 0;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 10;
    config.flows[1].dataDelay = 0;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 120;
    config.flows[2].dataDelay = 0;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), true);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(1).get())));
    MockStreamInstance * mockFlowInst2 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(2).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);
    pi.Start();

    // should create timers for each flow
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetTimers().size());

    // fire two timers
    tm->GetTimers().begin()->second.delegate(); 
    (++tm->GetTimers().begin())->second.delegate(); 

    // should leave one timer outstanding
    pi.Stop();

    // should cancel only remaining timer
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetCancelledTimers().size());

    // handle of created/cancelled timer should match

    CPPUNIT_ASSERT_EQUAL((++++tm->GetTimers().begin())->first, *tm->GetCancelledTimers().begin());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testZeroStartDelay(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig; 
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(1);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 0;
    config.flows[0].dataDelay = 10;
    config.flows[0].serverPort = 123;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), true);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);

    pi.Start();

    // starting with a 0 start time means it should try to connect immediately
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].serverPort, sm->GetConnects().begin()->serverPort);

    // fire off the connect delegate
    int fd = 123;
    sm->GetConnects().begin()->connectDelegate(fd);

    // verify the fd was updated
    CPPUNIT_ASSERT_EQUAL(fd, pi.mFlows.at(0)->GetFd());

    // verify the flow instance was started
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, pi.mFlows.at(0)->GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testDataDelay(void)
{
    // create dummy tcp flow config
    FlowEngine::Config_t flowConfig;
    flowConfig.layer = FlowConfig::TCP;
    // note that empty flow ignores the data delay
    flowConfig.pktList.resize(1);

    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(1);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 1;
    config.flows[0].dataDelay = 8;
    config.flows[0].serverPort = 456;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), true);

    // replace FlowInstance with Mock
    MockStreamInstance * mockFlowInst = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst);

    pi.Start();

    // should have started a start timer
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].startTime, tm->GetTimers().begin()->second.msDelay);

    // fire off timer delegate
    tm->GetTimers().begin()->second.delegate();

    // should have started a connect
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].serverPort, sm->GetConnects().begin()->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, sm->GetConnects().begin()->playInst);
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(false, mockFlowInst->IsStarted());

    // fire off the connect delegate
    int fd = 123;
    sm->GetConnects().begin()->connectDelegate(fd);
    // flow should not start
    CPPUNIT_ASSERT_EQUAL(false, mockFlowInst->IsStarted());
    // should have started another data delay timer
    CPPUNIT_ASSERT_EQUAL(size_t(2), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].dataDelay, (++tm->GetTimers().begin())->second.msDelay);

    // fire off data delay timer delegate
    (++tm->GetTimers().begin())->second.delegate();

    // now flow should be started
    CPPUNIT_ASSERT_EQUAL(true, mockFlowInst->IsStarted());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testUDPDataDelay(void)
{
    // create dummy udp flow config
    FlowEngine::Config_t flowConfig;
    flowConfig.layer = FlowConfig::UDP;

    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(1);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 1;
    config.flows[0].dataDelay = 8;
    config.flows[0].serverPort = 456;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), true);

    // replace FlowInstance with Mock
    MockStreamInstance * mockFlowInst = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst);

    pi.Start();

    // should have started a start timer
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());

    // fire off timer delegate
    tm->GetTimers().begin()->second.delegate();

    // should have started a connect
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].serverPort, sm->GetConnects().begin()->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, sm->GetConnects().begin()->playInst);
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(false, mockFlowInst->IsStarted());

    // fire off the connect delegate
    int fd = 123;
    sm->GetConnects().begin()->connectDelegate(fd);
    // should not start the data delay timer
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());
    // flow should start
    CPPUNIT_ASSERT_EQUAL(true, mockFlowInst->IsStarted());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testZeroDataDelay(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig; 
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(1);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 1;
    config.flows[0].dataDelay = 0;
    config.flows[0].serverPort = 456;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), true);

    // replace FlowInstance with Mock
    MockStreamInstance * mockFlowInst = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst);

    pi.Start();

    // should have started a start timer
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());

    // fire off timer delegate
    tm->GetTimers().begin()->second.delegate(); 

    // should have started a connect
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].serverPort, sm->GetConnects().begin()->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, sm->GetConnects().begin()->playInst);
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(false, mockFlowInst->IsStarted());

    // fire off the connect delegate
    int fd = 123;
    sm->GetConnects().begin()->connectDelegate(fd);

    // starting with a 0 data delay means it should try to start the flow immediately after the connect
    CPPUNIT_ASSERT_EQUAL(true, mockFlowInst->IsStarted());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testStopStreamClient(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 0;
    config.flows[0].serverPort = 100;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 20;
    config.flows[1].dataDelay = 0;
    config.flows[1].serverPort = 110;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 30;
    config.flows[2].dataDelay = 0;
    config.flows[2].serverPort = 120;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), true);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(1).get())));
    MockStreamInstance * mockFlowInst2 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(2).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    pi.Start();

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetTimers().size());

    // fire two timers
    tm->GetTimers().begin()->second.delegate(); 
    (++tm->GetTimers().begin())->second.delegate();

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);

    // fire off the connect delegate for the first flow only
    int fd = 123;
    sm->GetConnects().begin()->connectDelegate(fd);

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    // only the first flow was started and got a connection fd
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(fd, mockFlowInst0->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst1->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());

    pi.Stop();

    // all other timers cancelled
    CPPUNIT_ASSERT_EQUAL(size_t(2), tm->GetTimers().size() - tm->GetCancelledTimers().size());
    // should have 1 closed connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());

    // should have aborted all the flows
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst0->GetFd()); // fd cleared
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst2->GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testStopAttackClient(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig; 
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    PlaylistConfig config;
    config.type = PlaylistConfig::ATTACK_ONLY;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].minPktDelay = 200;
    config.flows[0].serverPort = 100;
    config.flows[0].socketTimeout = 1000;
    config.flows[1].flowHandle = flowHandle;
    config.flows[1].minPktDelay = 200;
    config.flows[1].serverPort = 200;
    config.flows[1].socketTimeout = 0;
    config.flows[2].flowHandle = flowHandle;
    config.flows[2].minPktDelay = 200;
    config.flows[2].serverPort = 300;
    config.flows[2].socketTimeout = 0;

    AttackPlaylistInstance pi(*pe, *sm, config, ModifierBlock(config, 0), true);

    // replace FlowInstances with Mock
    MockAttackInstance * mockFlowInst0 = new MockAttackInstance(*pi.mFlows.at(0).get());
    MockAttackInstance * mockFlowInst1 = new MockAttackInstance(*pi.mFlows.at(1).get());
    MockAttackInstance * mockFlowInst2 = new MockAttackInstance(*pi.mFlows.at(2).get());
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    pi.Start();

    // fire off the connect delegate
    int fd = 123;
    CPPUNIT_ASSERT_EQUAL(size_t(1),sm->GetConnects().size());
    sm->GetConnects().begin()->connectDelegate(fd);
    CPPUNIT_ASSERT_EQUAL(fd, mockFlowInst0->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());

    // First flow done
    mockFlowInst0->SetState(FlowInstance::DONE);
    pi.FlowDone(0, true);

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());
    // fd for the first flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst0->GetFd());

    // should have started another connection
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[1].serverPort, (++sm->GetConnects().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++sm->GetConnects().begin())->playInst);

    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());

    // First flow: done and close; second flow: aborted
    pi.Stop();

    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst2->GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testSuccessfulStreamClient(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    flowConfig.closeType = FlowConfig::C_FIN;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 0;
    config.flows[0].serverPort = 100;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 20;
    config.flows[1].dataDelay = 0;
    config.flows[1].serverPort = 110;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 30;
    config.flows[2].dataDelay = 0;
    config.flows[2].serverPort = 120;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), true);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(1).get())));
    MockStreamInstance * mockFlowInst2 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(2).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    pi.Start();

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::STARTED, pi.GetState());

    // fire all three timers
    tm->GetTimers().begin()->second.delegate(); 
    (++tm->GetTimers().begin())->second.delegate();
    (++++tm->GetTimers().begin())->second.delegate();

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetConnects().size());

    // fire off the connect delegate for the first flow
    int fd1 = 123;
    sm->GetConnects().begin()->connectDelegate(fd1);

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    // only the first flow was started and got a connection fd
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst0->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst1->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());

    // fire off the connect delegate for the second flow
    int fd2 = 234;
    (++sm->GetConnects().begin())->connectDelegate(fd2);

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    // now the second flow was started and got a connection fd
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst0->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst1->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());

    // fire off the connect delegate for the third flow
    int fd3 = 345;
    (++++sm->GetConnects().begin())->connectDelegate(fd3);

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    // now the third flow was started and got a connection fd
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst0->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst1->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(fd3, mockFlowInst2->GetFd());

    // first flow done successfully
    mockFlowInst0->SetState(FlowInstance::DONE);
    pi.FlowDone(0, true);

    // should have 1 closed connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst0->GetFd()); // fd cleared

    // third flow done successfully
    mockFlowInst2->SetState(FlowInstance::DONE);
    pi.FlowDone(2, true);

    // should have another closed connection
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd()); // fd cleared

    // second flow done successfully
    mockFlowInst1->SetState(FlowInstance::DONE);
    pi.FlowDone(1, true);

    // should have 3 closed connection
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst1->GetFd()); // fd cleared

    // all other timers cancelled
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetTimers().size() - tm->GetCancelledTimers().size());

    // should have done all the flows
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst2->GetState());

    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::SUCCESSFUL, pi.GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testSuccessfulAttackClient(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig; 
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    PlaylistConfig config;
    config.type = PlaylistConfig::ATTACK_ONLY;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].minPktDelay = 200;
    config.flows[0].serverPort = 100;
    config.flows[0].socketTimeout = 1000;
    config.flows[1].flowHandle = flowHandle;
    config.flows[1].minPktDelay = 200;
    config.flows[1].serverPort = 200;
    config.flows[1].socketTimeout = 0;
    config.flows[2].flowHandle = flowHandle;
    config.flows[2].minPktDelay = 200;
    config.flows[2].serverPort = 300;
    config.flows[2].socketTimeout = 0;

    AttackPlaylistInstance pi(*pe, *sm, config, ModifierBlock(config, 0), true);

    // replace FlowInstances with Mock
    MockAttackInstance * mockFlowInst0 = new MockAttackInstance(*pi.mFlows.at(0).get());
    MockAttackInstance * mockFlowInst1 = new MockAttackInstance(*pi.mFlows.at(1).get());
    MockAttackInstance * mockFlowInst2 = new MockAttackInstance(*pi.mFlows.at(2).get());
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    pi.Start();
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::STARTED, pi.GetState());

    // fire off the connect delegate
    int fd1 = 123;
    CPPUNIT_ASSERT_EQUAL(size_t(1),sm->GetConnects().size());
    sm->GetConnects().begin()->connectDelegate(fd1);
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst0->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());

    // First flow done
    mockFlowInst0->SetState(FlowInstance::DONE);
    pi.FlowDone(0, true);

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());
    // fd for the first flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst0->GetFd());

    // should have started another connection
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[1].serverPort, (++sm->GetConnects().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++sm->GetConnects().begin())->playInst);

    // fire off the connect delegate
    int fd2 = 234;
    (++sm->GetConnects().begin())->connectDelegate(fd2);
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst1->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());

    // Second flow done
    mockFlowInst1->SetState(FlowInstance::DONE);
    pi.FlowDone(1, true);

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetCloses().size());
    // fd for the first flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst1->GetFd());

    // should have started another connection
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[2].serverPort, (++++sm->GetConnects().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++++sm->GetConnects().begin())->playInst);

    // fire off the connect delegate
    int fd3 = 345;
    (++++sm->GetConnects().begin())->connectDelegate(fd3);
    CPPUNIT_ASSERT_EQUAL(fd3, mockFlowInst2->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst2->GetState());

    // last flow done
    mockFlowInst2->SetState(FlowInstance::DONE);
    pi.FlowDone(2, true);

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetCloses().size());
    // fd for the first flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());

    // should have stopped the playlist
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst2->GetState());

    // make sure no outstanding timer
    CPPUNIT_ASSERT_EQUAL(tm->GetTimers().size(), tm->GetCancelledTimers().size());

    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::SUCCESSFUL, pi.GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testFailedStreamClient(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    flowConfig.closeType = FlowConfig::C_FIN;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 0;
    config.flows[0].serverPort = 100;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 20;
    config.flows[1].dataDelay = 0;
    config.flows[1].serverPort = 110;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 30;
    config.flows[2].dataDelay = 0;
    config.flows[2].serverPort = 120;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), true);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(1).get())));
    MockStreamInstance * mockFlowInst2 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(2).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    pi.Start();

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::STARTED, pi.GetState());

    // fire all three timers
    tm->GetTimers().begin()->second.delegate(); 
    (++tm->GetTimers().begin())->second.delegate();
    (++++tm->GetTimers().begin())->second.delegate();

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetConnects().size());

    // fire off the connect delegate for the first flow
    int fd1 = 123;
    sm->GetConnects().begin()->connectDelegate(fd1);

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    // only the first flow was started and got a connection fd
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst0->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst1->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());

    // fire off the connect delegate for the second flow
    int fd2 = 234;
    (++sm->GetConnects().begin())->connectDelegate(fd2);

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    // now the second flow was started and got a connection fd
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst0->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst1->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());

    // fire off the connect delegate for the third flow
    int fd3 = 345;
    (++++sm->GetConnects().begin())->connectDelegate(fd3);

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    // now the third flow was started and got a connection fd
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst0->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst1->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(fd3, mockFlowInst2->GetFd());

    // first flow done successfully
    mockFlowInst0->SetState(FlowInstance::DONE);
    pi.FlowDone(0, true);

    // should have 1 closed connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst0->GetFd()); // fd cleared

    // third flow failed
    mockFlowInst2->SetState(FlowInstance::ABORTED);
    pi.FlowDone(2, false);

    // should have another closed connection
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd()); // fd cleared

    // second flow done successfully
    mockFlowInst1->SetState(FlowInstance::DONE);
    pi.FlowDone(1, true);

    // should have 3 closed connection
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst1->GetFd()); // fd cleared

    // all other timers cancelled
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetTimers().size() - tm->GetCancelledTimers().size());

    // should have stopped all the flows
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst2->GetState());

    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::FAILED, pi.GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testFailedAttackClient(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig; 
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    PlaylistConfig config;
    config.type = PlaylistConfig::ATTACK_ONLY;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].minPktDelay = 200;
    config.flows[0].serverPort = 100;
    config.flows[0].socketTimeout = 1000;
    config.flows[1].flowHandle = flowHandle;
    config.flows[1].minPktDelay = 200;
    config.flows[1].serverPort = 200;
    config.flows[1].socketTimeout = 0;
    config.flows[2].flowHandle = flowHandle;
    config.flows[2].minPktDelay = 200;
    config.flows[2].serverPort = 300;
    config.flows[2].socketTimeout = 0;

    AttackPlaylistInstance pi(*pe, *sm, config, ModifierBlock(config, 0), true);

    // replace FlowInstances with Mock
    MockAttackInstance * mockFlowInst0 = new MockAttackInstance(*pi.mFlows.at(0).get());
    MockAttackInstance * mockFlowInst1 = new MockAttackInstance(*pi.mFlows.at(1).get());
    MockAttackInstance * mockFlowInst2 = new MockAttackInstance(*pi.mFlows.at(2).get());
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    pi.Start();
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::STARTED, pi.GetState());

    // fire off the connect delegate
    int fd1 = 123;
    CPPUNIT_ASSERT_EQUAL(size_t(1),sm->GetConnects().size());
    sm->GetConnects().begin()->connectDelegate(fd1);
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst0->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());

    // First flow done
    mockFlowInst0->SetState(FlowInstance::DONE);
    pi.FlowDone(0, true);

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());
    // fd for the first flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst0->GetFd());

    // should have started another connection
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[1].serverPort, (++sm->GetConnects().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++sm->GetConnects().begin())->playInst);

    // fire off the connect delegate
    int fd2 = 234;
    (++sm->GetConnects().begin())->connectDelegate(fd2);
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst1->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());

    // Second flow failed
    mockFlowInst1->SetState(FlowInstance::ABORTED);
    pi.FlowDone(1, false);

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetCloses().size());
    // fd for the first flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst1->GetFd());

    // should have started another connection
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[2].serverPort, (++++sm->GetConnects().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++++sm->GetConnects().begin())->playInst);

    // fire off the connect delegate
    int fd3 = 345;
    (++++sm->GetConnects().begin())->connectDelegate(fd3);
    CPPUNIT_ASSERT_EQUAL(fd3, mockFlowInst2->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst2->GetState());

    // last flow done
    mockFlowInst2->SetState(FlowInstance::DONE);
    pi.FlowDone(2, true);

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetCloses().size());
    // fd for the first flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());

    // should have stopped the playlist
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst2->GetState());

    // make sure no outstanding timer
    CPPUNIT_ASSERT_EQUAL(tm->GetTimers().size(), tm->GetCancelledTimers().size());

    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::FAILED, pi.GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testStartServer(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig; 
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(2);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 10;
    config.flows[0].serverPort = 80;
    config.flows[1].flowHandle = flowHandle;
    config.flows[1].startTime = 100;
    config.flows[1].dataDelay = 10;
    config.flows[1].serverPort = 23;

    // even with a delay we can't know when the client will start so we listen 
    // immediately for all flows
    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), false);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(1).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);

    pi.Start();

    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetListens().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].serverPort, sm->GetListens().begin()->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *)&pi, sm->GetListens().begin()->playInst);
    CPPUNIT_ASSERT_EQUAL(config.flows[1].serverPort, (++sm->GetListens().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *)&pi, (++sm->GetListens().begin())->playInst);
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testStartServerManyInstances(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig; 
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(2);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 10;
    config.flows[0].serverPort = 100;
    config.flows[1].flowHandle = flowHandle;
    config.flows[1].startTime = 10;
    config.flows[1].dataDelay = 10;
    config.flows[1].serverPort = 120;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), false);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(1).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);

    pi.Start(); // FIXME - when variables work, this probably should be a 
                //         special, non-incrementing start 

    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetListens().size());
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *)&pi, sm->GetListens().begin()->playInst);
    CPPUNIT_ASSERT_EQUAL(int(0), sm->GetListens().begin()->flowIdx);
    CPPUNIT_ASSERT_EQUAL(int(1), (++sm->GetListens().begin())->flowIdx);

    // If a connection is made, application must start an instance for that connection
    StreamPlaylistInstance pi2(pi);
    pi2.Start();
    CPPUNIT_ASSERT_EQUAL(size_t(4), sm->GetListens().size());
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *)&pi2, (++++sm->GetListens().begin())->playInst);
    CPPUNIT_ASSERT_EQUAL(int(0), (++++sm->GetListens().begin())->flowIdx);
    CPPUNIT_ASSERT_EQUAL(int(1), (++++++sm->GetListens().begin())->flowIdx);

    // the application will then route the connect to the proper new instance
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testServerDataDelay(void)
{
    // create dummy tcp flow config
    FlowEngine::Config_t flowConfig;
    flowConfig.layer = FlowConfig::TCP;
    // note that empty flow ignores the data delay
    flowConfig.pktList.resize(1);
    flowConfig.pktList[0].clientTx = false;

    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(1);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 1;
    config.flows[0].dataDelay = 8;
    config.flows[0].serverPort = 456;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), false);

    // replace FlowInstance with Mock
    MockStreamInstance * mockFlowInst = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst);

    pi.Start();

    // no start timer for server
    CPPUNIT_ASSERT_EQUAL(size_t(0), tm->GetTimers().size());

    // should have started a listen
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetListens().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].serverPort, sm->GetListens().begin()->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, sm->GetListens().begin()->playInst);
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(false, mockFlowInst->IsStarted());

    // fire off the listen delegate
    int fd = 123;
    sm->GetListens().begin()->connectDelegate(fd);
    // flow should not start
    CPPUNIT_ASSERT_EQUAL(false, mockFlowInst->IsStarted());
    // should have started a data delay timer
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].dataDelay, tm->GetTimers().begin()->second.msDelay);

    // fire off data delay timer delegate
    tm->GetTimers().begin()->second.delegate();

    // now flow should be started
    CPPUNIT_ASSERT_EQUAL(true, mockFlowInst->IsStarted());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testStopStreamServer(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig; 
    flowConfig.closeType = FlowConfig::C_FIN;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 0;
    config.flows[0].serverPort = 100;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 20;
    config.flows[1].dataDelay = 0;
    config.flows[1].serverPort = 110;

    config.flows[2].flowHandle = flowHandle; 
    config.flows[2].startTime = 20;
    config.flows[2].dataDelay = 0;
    config.flows[2].serverPort = 120;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), false);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(1).get())));
    MockStreamInstance * mockFlowInst2 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(2).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    pi.Start();

    // should have started 3 listening
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetListens().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].serverPort, sm->GetListens().begin()->serverPort);
    CPPUNIT_ASSERT_EQUAL(config.flows[1].serverPort, (++sm->GetListens().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL(config.flows[2].serverPort, (++++sm->GetListens().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, sm->GetListens().begin()->playInst);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++sm->GetListens().begin())->playInst);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++++sm->GetListens().begin())->playInst);

    //fire off the connect delegate for the first two flows
    int fd = 123;
    sm->GetListens().begin()->connectDelegate(fd);
    int fd2 = 456;
    (++sm->GetListens().begin())->connectDelegate(fd2);

    // should have started the first 2 flows
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(fd, mockFlowInst0->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst1->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());

    // the first flow finished data transmission
    mockFlowInst0->SetState(FlowInstance::DONE);
    pi.FlowDone(0, true);

    // should be waiting for client to close the connection
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(fd, mockFlowInst0->GetFd());

    pi.Stop();
    // should have closed all two open connections
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetCloses().size());
    // fd for the three flows cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst0->GetFd());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst1->GetFd());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());

    // should have aborted the other two flows
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst2->GetState());
}

///////////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testStopAttackServer(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig; 
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    PlaylistConfig config;
    config.type = PlaylistConfig::ATTACK_ONLY;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].minPktDelay = 200;
    config.flows[0].serverPort = 100;
    config.flows[0].socketTimeout = 1000;
    config.flows[1].flowHandle = flowHandle;
    config.flows[1].minPktDelay = 200;
    config.flows[1].serverPort = 200;
    config.flows[1].socketTimeout = 0;
    config.flows[2].flowHandle = flowHandle;
    config.flows[2].minPktDelay = 200;
    config.flows[2].serverPort = 300;
    config.flows[2].socketTimeout = 0;

    AttackPlaylistInstance pi(*pe, *sm, config, ModifierBlock(config, 0), false);

    // replace FlowInstances with Mock
    MockAttackInstance * mockFlowInst0 = new MockAttackInstance(*pi.mFlows.at(0).get());
    MockAttackInstance * mockFlowInst1 = new MockAttackInstance(*pi.mFlows.at(1).get());
    MockAttackInstance * mockFlowInst2 = new MockAttackInstance(*pi.mFlows.at(2).get());
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    pi.Start();
    // should have 3 listenings
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetListens().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].serverPort, sm->GetListens().begin()->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, sm->GetListens().begin()->playInst);
    CPPUNIT_ASSERT_EQUAL(config.flows[1].serverPort, (++sm->GetListens().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++sm->GetListens().begin())->playInst);
    CPPUNIT_ASSERT_EQUAL(config.flows[2].serverPort, (++++sm->GetListens().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++++sm->GetListens().begin())->playInst);

    // fire off the connect delegate
    int fd = 123;
    sm->GetListens().begin()->connectDelegate(fd);
    CPPUNIT_ASSERT_EQUAL(fd, mockFlowInst0->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());

    // First flow: done and close; second flow: aborted
    mockFlowInst0->SetState(FlowInstance::DONE);
    pi.FlowDone(0, true);

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());
    // fd for the first flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst0->GetFd());

    // fire off the connect delegate
    int fd2 = 456;
    (++sm->GetListens().begin())->connectDelegate(fd2);
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst1->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst0->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst1->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());

    pi.Stop();

    // should have closed the second connection
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetCloses().size());
    // fd for the first flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst1->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst2->GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testStreamConnClosedServer(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    flowConfig.closeType = FlowConfig::C_FIN;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 0;
    config.flows[0].serverPort = 100;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 20;
    config.flows[1].dataDelay = 0;
    config.flows[1].serverPort = 110;

    config.flows[2].flowHandle = flowHandle; 
    config.flows[2].startTime = 20;
    config.flows[2].dataDelay = 0;
    config.flows[2].serverPort = 120;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), false);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(1).get())));
    MockStreamInstance * mockFlowInst2 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(2).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    pi.Start();

    // should have started 3 listening
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetListens().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].serverPort, sm->GetListens().begin()->serverPort);
    CPPUNIT_ASSERT_EQUAL(config.flows[1].serverPort, (++sm->GetListens().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL(config.flows[2].serverPort, (++++sm->GetListens().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, sm->GetListens().begin()->playInst);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++sm->GetListens().begin())->playInst);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++++sm->GetListens().begin())->playInst);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);

    // fire off the close delegate for the 1st flow
    sm->GetListens().begin()->closeDelegate(AbstSocketManager::RX_RST);

    //fire off the connect delegate for the 2nd flow
    int fd1 = 123;
    (++sm->GetListens().begin())->connectDelegate(fd1);

    // should have aborted the 1st flow and started the 2nd flow
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pi.mNumOfRunningFlows);

    //fire off the connect delegate for the 3rd flow
    int fd2 = 456;
    (++++sm->GetListens().begin())->connectDelegate(fd2);
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pi.mNumOfRunningFlows);

    // the 2nd flow finished data transmission
    mockFlowInst1->SetState(FlowInstance::DONE);
    pi.FlowDone(1, true);

    // should be waiting for the client to close the connection
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst1->GetFd());

    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst2->GetFd());

    // fire off the close delegate for the 2nd and the 3rd flow
    (++sm->GetListens().begin())->closeDelegate(AbstSocketManager::RX_RST);
    (++++sm->GetListens().begin())->closeDelegate(AbstSocketManager::RX_RST);

    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());

    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pi.mNumOfRunningFlows);
    // verify the playlist instance was called
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::FAILED, pi.GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testAttackConnClosedServer(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig; 
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    PlaylistConfig config;
    config.type = PlaylistConfig::ATTACK_ONLY;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].minPktDelay = 200;
    config.flows[0].serverPort = 100;
    config.flows[0].socketTimeout = 1000;
    config.flows[1].flowHandle = flowHandle;
    config.flows[1].minPktDelay = 200;
    config.flows[1].serverPort = 200;
    config.flows[1].socketTimeout = 0;
    config.flows[2].flowHandle = flowHandle;
    config.flows[2].minPktDelay = 200;
    config.flows[2].serverPort = 300;
    config.flows[2].socketTimeout = 0;

    AttackPlaylistInstance pi(*pe, *sm, config, ModifierBlock(config, 0), false);

    // replace FlowInstances with Mock
    MockAttackInstance * mockFlowInst0 = new MockAttackInstance(*pi.mFlows.at(0).get());
    MockAttackInstance * mockFlowInst1 = new MockAttackInstance(*pi.mFlows.at(1).get());
    MockAttackInstance * mockFlowInst2 = new MockAttackInstance(*pi.mFlows.at(2).get());
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    pi.Start();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi.mCurrAttackIdx);
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::STARTED, pi.GetState());

    // should have 3 listenings
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetListens().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].serverPort, sm->GetListens().begin()->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, sm->GetListens().begin()->playInst);
    CPPUNIT_ASSERT_EQUAL(config.flows[1].serverPort, (++sm->GetListens().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++sm->GetListens().begin())->playInst);
    CPPUNIT_ASSERT_EQUAL(config.flows[2].serverPort, (++++sm->GetListens().begin())->serverPort);

    // fire off the connect delegate for the 1st flow
    int fd = 123;
    sm->GetListens().begin()->connectDelegate(fd);
    CPPUNIT_ASSERT_EQUAL(fd, mockFlowInst0->GetFd());

    // fire off the close delegate for the 1st flow
    sm->GetListens().begin()->closeDelegate(AbstSocketManager::RX_RST);
    // fd in the first flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst0->GetFd());

    // current attack index updated
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi.mCurrAttackIdx);

    //fire off the connect delegate for the 2nd flow
    int fd1 = 123;
    (++sm->GetListens().begin())->connectDelegate(fd1);

    // should have aborted the 1st flow and started the 2nd flow
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi.mCurrAttackIdx);

    // the 2nd flow finished data transmission
    mockFlowInst1->SetState(FlowInstance::DONE);
    pi.FlowDone(1, true);

    // for the 2nd flow, the connection is closed.
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());

    // should have start the 3rd attack
    CPPUNIT_ASSERT_EQUAL(size_t(2), pi.mCurrAttackIdx);
    // should have started another listening
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetListens().size());

    //fire off the connect delegate for the 3rd flow
    int fd2 = 456;
    (++++sm->GetListens().begin())->connectDelegate(fd2);
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst2->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst1->GetState());

    // fire off the close delegate for the 3rd flow
    (++++sm->GetListens().begin())->closeDelegate(AbstSocketManager::RX_RST);
    // fd in the last flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst2->GetState());

    CPPUNIT_ASSERT_EQUAL(size_t(3), pi.mCurrAttackIdx);
    // verify the playlist instance was called
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::FAILED, pi.GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testSuccessfulStreamServer(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig; 
    flowConfig.closeType = FlowConfig::C_FIN;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 0;
    config.flows[0].serverPort = 100;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 20;
    config.flows[1].dataDelay = 0;
    config.flows[1].serverPort = 110;

    config.flows[2].flowHandle = flowHandle; 
    config.flows[2].startTime = 20;
    config.flows[2].dataDelay = 0;
    config.flows[2].serverPort = 120;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), false);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(1).get())));
    MockStreamInstance * mockFlowInst2 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(2).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    pi.Start();
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::STARTED, pi.GetState());
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);

    // should have started 3 listening
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetListens().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].serverPort, sm->GetListens().begin()->serverPort);
    CPPUNIT_ASSERT_EQUAL(config.flows[1].serverPort, (++sm->GetListens().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL(config.flows[2].serverPort, (++++sm->GetListens().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, sm->GetListens().begin()->playInst);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++sm->GetListens().begin())->playInst);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++++sm->GetListens().begin())->playInst);

    //fire off the connect delegate for all three flows
    int fd1 = 123;
    sm->GetListens().begin()->connectDelegate(fd1);
    int fd2 = 234;
    (++sm->GetListens().begin())->connectDelegate(fd2);
    int fd3 = 345;
    (++++sm->GetListens().begin())->connectDelegate(fd3);

    // should have started 3 flows
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst0->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst1->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(fd3, mockFlowInst2->GetFd());

    // the first flow finished data transmission
    mockFlowInst0->SetState(FlowInstance::DONE);
    pi.FlowDone(0, true);

    // Got FIN from client
    sm->GetListens().begin()->closeDelegate(AbstSocketManager::RX_FIN);
    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());
    // fd for the first flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst0->GetFd());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pi.mNumOfRunningFlows);

    // the second flow finished data transmission
    mockFlowInst1->SetState(FlowInstance::DONE);
    pi.FlowDone(1, true);

    // Got FIN from client
    (++sm->GetListens().begin())->closeDelegate(AbstSocketManager::RX_FIN);
    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetCloses().size());
    // fd for the flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst1->GetFd());

    // the third flow finished data transmission
    mockFlowInst2->SetState(FlowInstance::DONE);
    pi.FlowDone(2, true);

    // Got FIN from client
    (++++sm->GetListens().begin())->closeDelegate(AbstSocketManager::RX_FIN);
    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetCloses().size());
    // fd for the flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());

    // should have stopped the playlist instance
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::SUCCESSFUL, pi.GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testSuccessfulAttackServer(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig; 
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    PlaylistConfig config;
    config.type = PlaylistConfig::ATTACK_ONLY;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].minPktDelay = 200;
    config.flows[0].serverPort = 100;
    config.flows[0].socketTimeout = 1000;
    config.flows[1].flowHandle = flowHandle;
    config.flows[1].minPktDelay = 200;
    config.flows[1].serverPort = 200;
    config.flows[1].socketTimeout = 0;
    config.flows[2].flowHandle = flowHandle;
    config.flows[2].minPktDelay = 200;
    config.flows[2].serverPort = 300;
    config.flows[2].socketTimeout = 0;

    AttackPlaylistInstance pi(*pe, *sm, config, ModifierBlock(config, 0), false);

    // replace FlowInstances with Mock
    MockAttackInstance * mockFlowInst0 = new MockAttackInstance(*pi.mFlows.at(0).get());
    MockAttackInstance * mockFlowInst1 = new MockAttackInstance(*pi.mFlows.at(1).get());
    MockAttackInstance * mockFlowInst2 = new MockAttackInstance(*pi.mFlows.at(2).get());
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    pi.Start();
    
    // should have 3 listenings
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetListens().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].serverPort, sm->GetListens().begin()->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, sm->GetListens().begin()->playInst);
    CPPUNIT_ASSERT_EQUAL(config.flows[1].serverPort, (++sm->GetListens().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++sm->GetListens().begin())->playInst);
    CPPUNIT_ASSERT_EQUAL(config.flows[2].serverPort, (++++sm->GetListens().begin())->serverPort);
    
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::STARTED, pi.GetState());

    // fire off the connect delegate
    int fd1 = 123;
    sm->GetListens().begin()->connectDelegate(fd1);
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst0->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());

    // First flow done
    mockFlowInst0->SetState(FlowInstance::DONE);
    pi.FlowDone(0, true);

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());
    // fd for the first flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst0->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());

    // fire off the second connect delegate
    int fd2 = 234;
    (++sm->GetListens().begin())->connectDelegate(fd2);
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst1->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst1->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());

    // Second flow done
    mockFlowInst1->SetState(FlowInstance::DONE);
    pi.FlowDone(1, true);

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetCloses().size());
    // fd for the flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst1->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst1->GetState());

    // should have started another listening
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetListens().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[2].serverPort, (++++sm->GetListens().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++++sm->GetListens().begin())->playInst);

    // fire off the third connect delegate
    int fd3 = 345;
    (++++sm->GetListens().begin())->connectDelegate(fd3);
    CPPUNIT_ASSERT_EQUAL(fd3, mockFlowInst2->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst2->GetState());

    // Third flow done
    mockFlowInst2->SetState(FlowInstance::DONE);
    pi.FlowDone(2, true);

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetCloses().size());
    // fd for the flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst2->GetState());

    // verify the playlist instance was stopped
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst2->GetState());

    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::SUCCESSFUL, pi.GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testFailedStreamServer(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    flowConfig.closeType = FlowConfig::C_FIN; 
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 0;
    config.flows[0].serverPort = 100;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 20;
    config.flows[1].dataDelay = 0;
    config.flows[1].serverPort = 110;

    config.flows[2].flowHandle = flowHandle; 
    config.flows[2].startTime = 20;
    config.flows[2].dataDelay = 0;
    config.flows[2].serverPort = 120;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), false);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(1).get())));
    MockStreamInstance * mockFlowInst2 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(2).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    pi.Start();
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::STARTED, pi.GetState());

    // should have started 3 listening
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetListens().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].serverPort, sm->GetListens().begin()->serverPort);
    CPPUNIT_ASSERT_EQUAL(config.flows[1].serverPort, (++sm->GetListens().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL(config.flows[2].serverPort, (++++sm->GetListens().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, sm->GetListens().begin()->playInst);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++sm->GetListens().begin())->playInst);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++++sm->GetListens().begin())->playInst);

    //fire off the connect delegate for all three flows
    int fd1 = 123;
    sm->GetListens().begin()->connectDelegate(fd1);
    int fd2 = 234;
    (++sm->GetListens().begin())->connectDelegate(fd2);
    int fd3 = 345;
    (++++sm->GetListens().begin())->connectDelegate(fd3);

    // should have started 3 flows
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst0->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst1->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(fd3, mockFlowInst2->GetFd());

    // the first flow finished data transmission
    mockFlowInst0->SetState(FlowInstance::DONE);
    pi.FlowDone(0, true);

    // should be waiting for the client to close the connection
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst0->GetFd());

    // the third flow failed data transmission
    mockFlowInst2->SetState(FlowInstance::ABORTED);
    pi.FlowDone(2, false);

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());
    // fd for the flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());

    // the second flow finished data transmission
    mockFlowInst1->SetState(FlowInstance::DONE);
    pi.FlowDone(1, true);

    // should be waiting for the client to closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst1->GetFd());

    // Got close notifications from the clients
    sm->GetListens().begin()->closeDelegate(AbstSocketManager::RX_FIN);
    (++sm->GetListens().begin())->closeDelegate(AbstSocketManager::RX_FIN);
    // fd for the flows cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst0->GetFd());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst1->GetFd());

    // should have stopped the playlist instance
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::FAILED, pi.GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testFailedAttackServer(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig; 
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    PlaylistConfig config;
    config.type = PlaylistConfig::ATTACK_ONLY;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].minPktDelay = 200;
    config.flows[0].serverPort = 100;
    config.flows[0].socketTimeout = 1000;
    config.flows[1].flowHandle = flowHandle;
    config.flows[1].minPktDelay = 200;
    config.flows[1].serverPort = 200;
    config.flows[1].socketTimeout = 0;
    config.flows[2].flowHandle = flowHandle;
    config.flows[2].minPktDelay = 200;
    config.flows[2].serverPort = 300;
    config.flows[2].socketTimeout = 0;

    AttackPlaylistInstance pi(*pe, *sm, config, ModifierBlock(config, 0), false);

    // replace FlowInstances with Mock
    MockAttackInstance * mockFlowInst0 = new MockAttackInstance(*pi.mFlows.at(0).get());
    MockAttackInstance * mockFlowInst1 = new MockAttackInstance(*pi.mFlows.at(1).get());
    MockAttackInstance * mockFlowInst2 = new MockAttackInstance(*pi.mFlows.at(2).get());
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    pi.Start();
    // should have 3 listenings
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetListens().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].serverPort, sm->GetListens().begin()->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, sm->GetListens().begin()->playInst);
    CPPUNIT_ASSERT_EQUAL(config.flows[1].serverPort, (++sm->GetListens().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++sm->GetListens().begin())->playInst);
    CPPUNIT_ASSERT_EQUAL(config.flows[2].serverPort, (++++sm->GetListens().begin())->serverPort);
    
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::STARTED, pi.GetState());

    // fire off the connect delegate
    int fd1 = 123;
    sm->GetListens().begin()->connectDelegate(fd1);
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst0->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());

    // First flow done
    mockFlowInst0->SetState(FlowInstance::DONE);
    pi.FlowDone(0, true);

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());
    // fd for the first flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst0->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());

    // fire off the second connect delegate
    int fd2 = 234;
    (++sm->GetListens().begin())->connectDelegate(fd2);
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst1->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(fd2, mockFlowInst1->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());

    // Second flow failed
    mockFlowInst1->SetState(FlowInstance::ABORTED);
    pi.FlowDone(1, false);

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetCloses().size());
    // fd for the flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst1->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst1->GetState());

    // should have started another listening
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetListens().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[2].serverPort, (++++sm->GetListens().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, (++++sm->GetListens().begin())->playInst);

    // fire off the third connect delegate
    int fd3 = 345;
    (++++sm->GetListens().begin())->connectDelegate(fd3);
    CPPUNIT_ASSERT_EQUAL(fd3, mockFlowInst2->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst2->GetState());

    // Third flow done
    mockFlowInst2->SetState(FlowInstance::DONE);
    pi.FlowDone(2, true);

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetCloses().size());
    // fd for the flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst2->GetFd());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst2->GetState());

    // verify the playlist instance was stopped
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst2->GetState());

    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::FAILED, pi.GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testCloneStream(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 0;
    config.flows[0].serverPort = 100;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 20;
    config.flows[1].dataDelay = 0;
    config.flows[1].serverPort = 110;

    config.flows[2].flowHandle = flowHandle; 
    config.flows[2].startTime = 20;
    config.flows[2].dataDelay = 0;
    config.flows[2].serverPort = 120;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), false);

    std::auto_ptr<StreamPlaylistInstance> pic(pi.Clone()); 

    CPPUNIT_ASSERT(&pic->mEngine == pe);
    CPPUNIT_ASSERT(&pic->mSocketMgr == sm);
    CPPUNIT_ASSERT(&pic->mConfig == &config);
    CPPUNIT_ASSERT(pic->mClient == false);
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testCloneAttack(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig; 
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    PlaylistConfig config;
    config.type = PlaylistConfig::ATTACK_ONLY;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].minPktDelay = 200;
    config.flows[0].serverPort = 100;
    config.flows[0].socketTimeout = 1000;
    config.flows[1].flowHandle = flowHandle;
    config.flows[1].minPktDelay = 200;
    config.flows[1].serverPort = 200;
    config.flows[1].socketTimeout = 0;
    config.flows[2].flowHandle = flowHandle;
    config.flows[2].minPktDelay = 200;
    config.flows[2].serverPort = 300;
    config.flows[2].socketTimeout = 0;

    AttackPlaylistInstance pi(*pe, *sm, config, ModifierBlock(config, 0), false);

    std::auto_ptr<AttackPlaylistInstance> pic(pi.Clone()); 

    CPPUNIT_ASSERT(&pic->mEngine == pe);
    CPPUNIT_ASSERT(&pic->mSocketMgr == sm);
    CPPUNIT_ASSERT(&pic->mConfig == &config);
    CPPUNIT_ASSERT(pic->mClient == false);
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testSuccessfulStreamLoops(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    flowConfig.layer = FlowConfig::TCP;
    flowConfig.closeType = FlowConfig::C_FIN;
    flowConfig.pktList.resize(1);
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;
    config.type = PlaylistConfig::STREAM_ONLY;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 5;
    config.flows[0].serverPort = 100;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 20;
    config.flows[1].dataDelay = 7;
    config.flows[1].serverPort = 110;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 30;
    config.flows[2].dataDelay = 9;
    config.flows[2].serverPort = 120;

    // stream loop, includes flow #1, and #2, loop 2 times
    PlaylistConfig::LoopInfo loop;
    loop.count = 2;
    loop.begIdx = 1;
    config.loopMap.insert(std::make_pair(2,loop));

    // create playlist 
    pe->UpdatePlaylist(pl_handle, config);

    const PlaylistEngine::Config_t *pc = pe->GetPlaylist(pl_handle);

    StreamPlaylistInstance pi(*pe, *sm, *pc, pl_handle, ModifierBlock(*pc, 0), true);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(1).get())));
    MockStreamInstance * mockFlowInst2 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(2).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    // make sure flow handlers are there
    CPPUNIT_ASSERT(pi.mFlows.at(0)->GetFlowLoopHandler().get()==0);
    FlowLoopHandler *loopHdlr = pi.mFlows.at(1)->GetFlowLoopHandler().get();
    CPPUNIT_ASSERT(loopHdlr!=0);
    CPPUNIT_ASSERT(loopHdlr==pi.mFlows.at(2)->GetFlowLoopHandler().get());

    pi.Start();

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);

    // fire all three timers
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetTimers().size());
    tm->GetTimers().begin()->second.delegate(); 
    (++tm->GetTimers().begin())->second.delegate();
    (++++tm->GetTimers().begin())->second.delegate();

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    // fire off the connect delegates for all the three flows
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetConnects().size());
    int fd1 = 123;
    sm->GetConnects().begin()->connectDelegate(fd1);
    int fd2 = 456;
    (++sm->GetConnects().begin())->connectDelegate(fd2);
    int fd3 = 789;
    (++++sm->GetConnects().begin())->connectDelegate(fd3);
    // should have three new timers for data delays
    CPPUNIT_ASSERT(pi.mTimers[0]!=tm->InvalidTimerHandle());
    CPPUNIT_ASSERT(pi.mTimers[1]!=tm->InvalidTimerHandle());
    CPPUNIT_ASSERT(pi.mTimers[2]!=tm->InvalidTimerHandle());
    // fire off all data delay timers for all three streams
    tm->GetTimers()[pi.mTimers[0]].delegate();
    tm->GetTimers()[pi.mTimers[1]].delegate();
    tm->GetTimers()[pi.mTimers[2]].delegate();

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst2->GetState());

    pi.FlowDone(1, true);
    pi.FlowDone(2, true);

    // stream #1 and #2 should then loop back, and keep running
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst2->GetState());

    pi.FlowDone(1, true);
    pi.FlowDone(2, true);

    // now two iterations of the loop is done, which means the loop is completed
    // should have 2 closed connections for the 2 flows in the loop
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetCloses().size());
    // only flow #0 is still running
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi.mNumOfRunningFlows);

    pi.FlowDone(0, true);
    // should have stopped all the flows
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst2->GetState());

    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pi.mNumOfRunningFlows);
    //verify pi.Stop() was called
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::SUCCESSFUL, pi.GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testSuccessfulAttackLoops(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    flowConfig.layer = FlowConfig::TCP;
    flowConfig.pktList.resize(1);
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;
    config.type = PlaylistConfig::ATTACK_ONLY;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 5;
    config.flows[0].serverPort = 100;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 20;
    config.flows[1].dataDelay = 7;
    config.flows[1].serverPort = 110;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 30;
    config.flows[2].dataDelay = 9;
    config.flows[2].serverPort = 120;

    // attack loop, includes flow #1, and #2, loop 2 times
    PlaylistConfig::LoopInfo loop;
    loop.count = 2;
    loop.begIdx = 1;
    config.loopMap.insert(std::make_pair(2,loop));

    // create playlist 
    pe->UpdatePlaylist(pl_handle, config);

    const PlaylistEngine::Config_t *pc = pe->GetPlaylist(pl_handle);

    AttackPlaylistInstance pi(*pe, *sm, *pc, ModifierBlock(*pc, 0), true);

    // replace FlowInstances with Mock
    MockAttackInstance * mockFlowInst0 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi.mFlows.at(0).get())));
    MockAttackInstance * mockFlowInst1 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi.mFlows.at(1).get())));
    MockAttackInstance * mockFlowInst2 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi.mFlows.at(2).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    // make sure flow handlers are there
    CPPUNIT_ASSERT(pi.mFlows.at(0)->GetFlowLoopHandler().get()==0);
    FlowLoopHandler *loopHdlr = pi.mFlows.at(1)->GetFlowLoopHandler().get();
    CPPUNIT_ASSERT(loopHdlr!=0);
    CPPUNIT_ASSERT(loopHdlr==pi.mFlows.at(2)->GetFlowLoopHandler().get());

    pi.Start();
    // should have initiated one connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetConnects().size());

    // fire off the connect delegate
    int fd = 123;
    sm->GetConnects().begin()->connectDelegate(fd);
    CPPUNIT_ASSERT_EQUAL(fd, mockFlowInst0->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());

    // First flow: done and close
    mockFlowInst0->SetState(FlowInstance::DONE);
    pi.FlowDone(0, true);

    // should have started another connection
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[1].serverPort, (++sm->GetConnects().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());

    // fire off the connect delegate for flow #1, which should start a loop
    int fd1 = 234;
    (++sm->GetConnects().begin())->connectDelegate(fd1);


    pi.FlowDone(1, true);

    // should continue to flow #2
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[2].serverPort, (++++sm->GetConnects().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetCloses().size());

    int fd2 = 345;
    (++++sm->GetConnects().begin())->connectDelegate(fd2);
    pi.FlowDone(2, true);

    // should loop back to flow #1
    CPPUNIT_ASSERT_EQUAL(size_t(4), sm->GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[1].serverPort, (++++++sm->GetConnects().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetCloses().size());

    int fd3 = 456;
    (++++++sm->GetConnects().begin())->connectDelegate(fd3);
    pi.FlowDone(1, true);

    // should continue to flow #2
    CPPUNIT_ASSERT_EQUAL(size_t(5), sm->GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[2].serverPort, (++++++++sm->GetConnects().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL(size_t(4), sm->GetCloses().size());

    int fd4 = 567;
    (++++++++sm->GetConnects().begin())->connectDelegate(fd4);
    pi.FlowDone(2, true);
    CPPUNIT_ASSERT_EQUAL(size_t(3), pi.mCurrAttackIdx);

    // now two iterations of the loop is done, which means the loop is completed
    // should have move on to end the playlist
    CPPUNIT_ASSERT_EQUAL(size_t(5), sm->GetCloses().size());

    // should have stopped all the flows
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst2->GetState());

    //verify pi.Stop() was called
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::SUCCESSFUL, pi.GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testFailedStreamLoops(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    flowConfig.layer = FlowConfig::TCP;
    flowConfig.closeType = FlowConfig::C_FIN;
    flowConfig.pktList.resize(1);
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;
    config.type = PlaylistConfig::STREAM_ONLY;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 5;
    config.flows[0].serverPort = 100;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 20;
    config.flows[1].dataDelay = 7;
    config.flows[1].serverPort = 110;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 30;
    config.flows[2].dataDelay = 9;
    config.flows[2].serverPort = 120;

    // stream loop, includes flow #1, and #2, loop 2 times
    PlaylistConfig::LoopInfo loop;
    loop.count = 2;
    loop.begIdx = 1;
    config.loopMap.insert(std::make_pair(2,loop));

    // create playlist 
    pe->UpdatePlaylist(pl_handle, config);

    const PlaylistEngine::Config_t *pc = pe->GetPlaylist(pl_handle);

    StreamPlaylistInstance pi(*pe, *sm, *pc, pl_handle, ModifierBlock(*pc, 0), true);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    MockStreamInstance * mockFlowInst1 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(1).get())));
    MockStreamInstance * mockFlowInst2 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(2).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    // make sure flow handlers are there
    CPPUNIT_ASSERT(pi.mFlows.at(0)->GetFlowLoopHandler().get()==0);
    FlowLoopHandler *loopHdlr = pi.mFlows.at(1)->GetFlowLoopHandler().get();
    CPPUNIT_ASSERT(loopHdlr!=0);
    CPPUNIT_ASSERT(loopHdlr==pi.mFlows.at(2)->GetFlowLoopHandler().get());

    pi.Start();

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);

    // fire all three timers
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetTimers().size());
    tm->GetTimers().begin()->second.delegate(); 
    (++tm->GetTimers().begin())->second.delegate();
    (++++tm->GetTimers().begin())->second.delegate();

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    // fire off the connect delegates for all the three flows
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetConnects().size());
    int fd1 = 123;
    sm->GetConnects().begin()->connectDelegate(fd1);
    int fd2 = 456;
    (++sm->GetConnects().begin())->connectDelegate(fd2);
    int fd3 = 789;
    (++++sm->GetConnects().begin())->connectDelegate(fd3);
    // should have three new timers for data delays
    CPPUNIT_ASSERT(pi.mTimers[0]!=tm->InvalidTimerHandle());
    CPPUNIT_ASSERT(pi.mTimers[1]!=tm->InvalidTimerHandle());
    CPPUNIT_ASSERT(pi.mTimers[2]!=tm->InvalidTimerHandle());
    // fire off all data delay timers for all three streams
    tm->GetTimers()[pi.mTimers[0]].delegate();
    tm->GetTimers()[pi.mTimers[1]].delegate();
    tm->GetTimers()[pi.mTimers[2]].delegate();

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst2->GetState());

    pi.FlowDone(1, true);
    pi.FlowDone(2, true);

    // stream #1 and #2 should then loop back, and keep running
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst2->GetState());

    // flow #2 failed in the middle of the loop
    pi.FlowDone(1, true);
    pi.FlowDone(2, false);

    // the loop will fail, which means none of the flows in the loop will continue
    // should have 2 closed connections for the 2 flows in the loop
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetCloses().size());
    // only flow #0 is still running
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi.mNumOfRunningFlows);

    pi.FlowDone(0, true);
    // should have stopped all the flows
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    // all flows in the loop will have an ABORTED state
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst2->GetState());

    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pi.mNumOfRunningFlows);
    //verify playlist instance was stopped and state updated
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::FAILED, pi.GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testFailedAttackLoops(void)
{
    // create dummy flow config
    FlowEngine::Config_t flowConfig;
    flowConfig.layer = FlowConfig::TCP;
    flowConfig.pktList.resize(1);
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;
    config.type = PlaylistConfig::ATTACK_ONLY;

    config.flows.resize(3);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 5;
    config.flows[0].serverPort = 100;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 20;
    config.flows[1].dataDelay = 7;
    config.flows[1].serverPort = 110;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 30;
    config.flows[2].dataDelay = 9;
    config.flows[2].serverPort = 120;

    // attack loop, includes flow #1, and #2, loop 2 times
    PlaylistConfig::LoopInfo loop;
    loop.count = 2;
    loop.begIdx = 1;
    config.loopMap.insert(std::make_pair(2,loop));

    // create playlist 
    pe->UpdatePlaylist(pl_handle, config);

    const PlaylistEngine::Config_t *pc = pe->GetPlaylist(pl_handle);

    AttackPlaylistInstance pi(*pe, *sm, *pc, ModifierBlock(*pc, 0), true);

    // replace FlowInstances with Mock
    MockAttackInstance * mockFlowInst0 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi.mFlows.at(0).get())));
    MockAttackInstance * mockFlowInst1 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi.mFlows.at(1).get())));
    MockAttackInstance * mockFlowInst2 = new MockAttackInstance(*(static_cast<AttackInstance *>(pi.mFlows.at(2).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);
    pi.mFlows.at(1) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst1);
    pi.mFlows.at(2) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst2);

    // make sure flow handlers are there
    CPPUNIT_ASSERT(pi.mFlows.at(0)->GetFlowLoopHandler().get()==0);
    FlowLoopHandler *loopHdlr = pi.mFlows.at(1)->GetFlowLoopHandler().get();
    CPPUNIT_ASSERT(loopHdlr!=0);
    CPPUNIT_ASSERT(loopHdlr==pi.mFlows.at(2)->GetFlowLoopHandler().get());

    pi.Start();
    // should have initiated one connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetConnects().size());

    // fire off the connect delegate
    int fd = 123;
    sm->GetConnects().begin()->connectDelegate(fd);
    CPPUNIT_ASSERT_EQUAL(fd, mockFlowInst0->GetFd());

    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::WAIT_TO_START, mockFlowInst2->GetState());

    // First flow: done and close
    mockFlowInst0->SetState(FlowInstance::DONE);
    pi.FlowDone(0, true);

    // should have started another connection
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[1].serverPort, (++sm->GetConnects().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());

    // fire off the connect delegate for flow #1, which should start a loop
    int fd1 = 234;
    (++sm->GetConnects().begin())->connectDelegate(fd1);


    pi.FlowDone(1, true);

    // should continue to flow #2
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[2].serverPort, (++++sm->GetConnects().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetCloses().size());

    int fd2 = 345;
    (++++sm->GetConnects().begin())->connectDelegate(fd2);
    pi.FlowDone(2, true);

    // should loop back to flow #1
    CPPUNIT_ASSERT_EQUAL(size_t(4), sm->GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[1].serverPort, (++++++sm->GetConnects().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetCloses().size());

    int fd3 = 456;
    (++++++sm->GetConnects().begin())->connectDelegate(fd3);
    pi.FlowDone(1, true);

    // should continue to flow #2
    CPPUNIT_ASSERT_EQUAL(size_t(5), sm->GetConnects().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[2].serverPort, (++++++++sm->GetConnects().begin())->serverPort);
    CPPUNIT_ASSERT_EQUAL(size_t(4), sm->GetCloses().size());

    int fd4 = 567;
    (++++++++sm->GetConnects().begin())->connectDelegate(fd4);
    // the flow failed
    pi.FlowDone(2, false);
    CPPUNIT_ASSERT_EQUAL(size_t(3), pi.mCurrAttackIdx);

    // now two iterations of the loop is done, which means the loop is completed
    // should have move on to end the playlist
    CPPUNIT_ASSERT_EQUAL(size_t(5), sm->GetCloses().size());

    // should have stopped all the flows
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    // all flows in the loop will have an ABORTED state
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst1->GetState());
    CPPUNIT_ASSERT_EQUAL(FlowInstance::ABORTED, mockFlowInst2->GetState());

    //verify playlist instance was stopped, and state updated
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::FAILED, pi.GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testStreamClientClsCFIN(void)
{
    // create flow config w/ close type CFIN
    FlowEngine::Config_t flowConfig;
    flowConfig.closeType = FlowConfig::C_FIN;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(1);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 0;
    config.flows[0].serverPort = 100;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), true);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);

    pi.Start();

    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::STARTED, pi.GetState());

    // fire the timer
    tm->GetTimers().begin()->second.delegate(); 

    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetConnects().size());

    // fire off the connect delegate for the flow
    int fd1 = 123;
    sm->GetConnects().begin()->connectDelegate(fd1);

    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi.mNumOfRunningFlows);
    // the flow was started and got a connection fd
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst0->GetFd());

    // first flow done successfully
    mockFlowInst0->SetState(FlowInstance::DONE);
    pi.FlowDone(0, true);

    // should have 1 closed connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pi.mNumOfRunningFlows);

    // all other timers cancelled
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size() - tm->GetCancelledTimers().size());

    // should have done all the flows
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());

    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::SUCCESSFUL, pi.GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testStreamClientClsSFIN(void)
{
    // create flow config w/ close type SFIN
    FlowEngine::Config_t flowConfig;
    flowConfig.closeType = FlowConfig::S_FIN;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(1);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 0;
    config.flows[0].serverPort = 100;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), true);

    // replace FlowInstances with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);

    pi.Start();

    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::STARTED, pi.GetState());

    // fire the timer
    tm->GetTimers().begin()->second.delegate(); 

    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi.mNumOfRunningFlows);
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetConnects().size());

    // fire off the connect delegate for the flow
    int fd1 = 123;
    sm->GetConnects().begin()->connectDelegate(fd1);

    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi.mNumOfRunningFlows);
    // the flow was started and got a connection fd
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst0->GetFd());

    // first flow done successfully
    mockFlowInst0->SetState(FlowInstance::DONE);
    pi.FlowDone(0, true);

    // should leave the connection open, let server to close it.
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pi.mNumOfRunningFlows);

    // Got close notification
    sm->GetConnects().begin()->closeDelegate(AbstSocketManager::RX_FIN);
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pi.mNumOfRunningFlows);
    // all other timers cancelled
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size() - tm->GetCancelledTimers().size());

    // should have done all the flows
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());

    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::SUCCESSFUL, pi.GetState());
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testStreamServerClsCFIN(void)
{
    // create flow config w/ close type CFIN
    FlowEngine::Config_t flowConfig; 
    flowConfig.closeType = FlowConfig::C_FIN;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(1);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 0;
    config.flows[0].serverPort = 100;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), false);

    // replace FlowInstance with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);

    pi.Start();
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::STARTED, pi.GetState());

    // should have started 1 listening
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetListens().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].serverPort, sm->GetListens().begin()->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, sm->GetListens().begin()->playInst);

    //fire off the connect delegate for the flow
    int fd1 = 123;
    sm->GetListens().begin()->connectDelegate(fd1);

    // should have started 1 flow
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst0->GetFd());

    // the flow finished data transmission
    mockFlowInst0->SetState(FlowInstance::DONE);
    pi.FlowDone(0, true);

    // should not close the connection, let client to close it
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetCloses().size());

    // Got close notification
    sm->GetListens().begin()->closeDelegate(AbstSocketManager::RX_FIN);

    // fd for the first flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst0->GetFd());

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst0->GetFd());

    // should have stopped the playlist instance
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::SUCCESSFUL, pi.GetState());
}
///////////////////////////////////////////////////////////////////////////////

void TestPlaylistInstance::testStreamServerClsSFIN(void)
{
    // create flow config w/ close type SFIN
    FlowEngine::Config_t flowConfig; 
    flowConfig.closeType = FlowConfig::S_FIN;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    handle_t pl_handle = 1;
    PlaylistConfig config;

    config.flows.resize(1);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 10;
    config.flows[0].dataDelay = 0;
    config.flows[0].serverPort = 100;

    StreamPlaylistInstance pi(*pe, *sm, config, pl_handle, ModifierBlock(config, 0), false);

    // replace FlowInstance with Mock
    MockStreamInstance * mockFlowInst0 = new MockStreamInstance(*(static_cast<StreamInstance *>(pi.mFlows.at(0).get())));
    pi.mFlows.at(0) = PlaylistInstance::flowInstSharedPtr_t(mockFlowInst0);

    pi.Start();
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::STARTED, pi.GetState());

    // should have started 1 listening
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetListens().size());
    CPPUNIT_ASSERT_EQUAL(config.flows[0].serverPort, sm->GetListens().begin()->serverPort);
    CPPUNIT_ASSERT_EQUAL((PlaylistInstance *) &pi, sm->GetListens().begin()->playInst);

    //fire off the connect delegate for the flow
    int fd1 = 123;
    sm->GetListens().begin()->connectDelegate(fd1);

    // should have started 1 flow
    CPPUNIT_ASSERT_EQUAL(FlowInstance::STARTED, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(fd1, mockFlowInst0->GetFd());

    // the flow finished data transmission
    mockFlowInst0->SetState(FlowInstance::DONE);
    pi.FlowDone(0, true);

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());
    // fd for the first flow cleared
    CPPUNIT_ASSERT_EQUAL(int(0), mockFlowInst0->GetFd());

    // should have closed the connection
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetCloses().size());

    // should have stopped the playlist instance
    CPPUNIT_ASSERT_EQUAL(FlowInstance::DONE, mockFlowInst0->GetState());
    CPPUNIT_ASSERT_EQUAL(PlaylistInstance::SUCCESSFUL, pi.GetState());
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestPlaylistInstance);
