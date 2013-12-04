#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "PlaylistInstance.h"
#include "FlowEngine.h"
#include "PlaylistEngine.h"
#include "StreamPlaylistInstance.h"
#include "StreamInstance.h"

#include "MockFlowConfigProxy.h"
#include "MockSocketManager.h"
#include "MockTimerManager.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class TestStreamInstance : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestStreamInstance);
    CPPUNIT_TEST(testStartClientStream);
    CPPUNIT_TEST(testStartServerStream);
    CPPUNIT_TEST(testNoPacketsStream);
    CPPUNIT_TEST(testAbortStream);
    CPPUNIT_TEST(testSuccessfulStream);
    CPPUNIT_TEST(testAdjustedPktDelays);
    CPPUNIT_TEST(testStreamWithAllRx);
    CPPUNIT_TEST(testStreamWithAllTx);
    CPPUNIT_TEST(testRxTimeoutStream);
    CPPUNIT_TEST(testTxTimeoutStream);
    CPPUNIT_TEST(testTooLongPacketDelay);
    CPPUNIT_TEST(testPktLoopsStream);
    CPPUNIT_TEST(testPktLoopsStream2);
    CPPUNIT_TEST(testNestedPktLoopsStream);
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
    void testStartClientStream(void);
    void testStartServerStream(void);
    void testNoPacketsStream(void);
    void testAbortStream(void);
    void testSuccessfulStream(void);
    void testAdjustedPktDelays(void);
    void testStreamWithAllRx(void);
    void testStreamWithAllTx(void);
    void testRxTimeoutStream(void);
    void testTxTimeoutStream(void);
    void testTooLongPacketDelay(void);
    void testPktLoopsStream(void);
    void testPktLoopsStream2(void);
    void testNestedPktLoopsStream(void);

private:
    FlowEngine * fe;
    MockSocketMgr * sm;
    MockTimerMgr * tm;
    PlaylistEngine * pe;
};

class MockStreamPlaylistInstance : public StreamPlaylistInstance
{
  public:
    MockStreamPlaylistInstance(PlaylistEngine& pEngine, AbstSocketManager& socketMgr, const PlaylistConfig& config, handle_t handle, bool client) :
                            StreamPlaylistInstance(pEngine, socketMgr, config, handle, ModifierBlock(config, 0), client),
                            mTotalStreams(config.flows.size()),
                            mFailedStreams(0),
                            mSuccessfulStreams(0)
    {};

    void FlowDone(size_t flowIndex, bool successful)
    {
        if (successful)
        {
            mSuccessfulStreams++;
        }
        else
        {
            mFailedStreams++;
        }
        mSocketMgr.Close(mFlows.at(flowIndex)->GetFd(), false);
    }

    size_t mTotalStreams;
    size_t mFailedStreams;
    size_t mSuccessfulStreams;
};

///////////////////////////////////////////////////////////////////////////////

void TestStreamInstance::testStartClientStream(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.pktList.resize(2);
    flowConfig.pktList[0].data.push_back(1);
    flowConfig.pktList[0].pktDelay = 100;
    flowConfig.pktList[0].clientTx = true;
    flowConfig.pktList[1].data.push_back(1);
    flowConfig.pktList[1].pktDelay = 100;
    flowConfig.pktList[1].clientTx = false;

    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(flowConfig));

    // create a mock stream playlist instance
    uint32_t pl_handle = 1;
    PlaylistConfig playlistConfig;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].serverPort = 123;

    MockStreamPlaylistInstance * pi = new MockStreamPlaylistInstance (*pe, *sm, playlistConfig, pl_handle, true);

    FlowInstance *fi = &(pi->GetFlow(0));
    fi->SetFd(1); // fake fd

    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());

    fi->Start();

    // should send data
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    // and prepare to receive
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(fi->GetFd(), sm->GetSends().begin()->fd);
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].data.size(), sm->GetSends().begin()->dataLength);
    CPPUNIT_ASSERT(memcmp(&flowConfig.pktList[0].data[0], sm->GetSends().begin()->data, flowConfig.pktList[0].data.size()) == 0);

    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(DPG_FLOW_INACTIVITY_TIMEOUT), tm->GetTimers()[fi->mInactivityTimer].msDelay);

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mFailedStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mSuccessfulStreams);
}

///////////////////////////////////////////////////////////////////////////////

void TestStreamInstance::testStartServerStream(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.pktList.resize(1);
    flowConfig.pktList[0].data.push_back(1);
    flowConfig.pktList[0].pktDelay = 100;
    flowConfig.pktList[0].clientTx = true;

    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(flowConfig));

    // create a mock stream playlist instance
    uint32_t pl_handle = 1;
    PlaylistConfig playlistConfig;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].serverPort = 123;

    MockStreamPlaylistInstance * pi = new MockStreamPlaylistInstance (*pe, *sm, playlistConfig, pl_handle, false);

    FlowInstance *fi = &(pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());

    fi->Start();

    // should recv data
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(fi->GetFd(), sm->GetRecvs().begin()->fd);
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].data.size(), sm->GetRecvs().begin()->dataLength);

    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(DPG_FLOW_INACTIVITY_TIMEOUT), tm->GetTimers()[fi->mInactivityTimer].msDelay);

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mFailedStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mSuccessfulStreams);
}

///////////////////////////////////////////////////////////////////////////////

void TestStreamInstance::testNoPacketsStream(void)
{
    // verify that sockets are immediately closed when no packets are defined
    uint32_t handle = 1;
    FlowConfig config;

    // create flow 
    fe->UpdateFlow(handle, MockFlowConfigProxy(config));

    // create a mock stream playlist instance
    uint32_t pl_handle = 1;
    PlaylistConfig playlistConfig;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].serverPort = 123;

    MockStreamPlaylistInstance * pi = new MockStreamPlaylistInstance (*pe, *sm, playlistConfig, pl_handle, false);

    FlowInstance *fi = &(pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    fi->Start();

    // should send no data
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(fi->GetFd(), *sm->GetCloses().begin());

    CPPUNIT_ASSERT_EQUAL(size_t(0), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(tm->InvalidTimerHandle(), fi->mInactivityTimer);

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mFailedStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mSuccessfulStreams);
}

///////////////////////////////////////////////////////////////////////////////

void TestStreamInstance::testAbortStream(void)
{
    uint32_t handle = 1;
    FlowConfig config;
    config.pktList.resize(3);
    config.pktList[0].data.push_back(1);
    config.pktList[0].pktDelay = 99;
    config.pktList[0].clientTx = true;
    config.pktList[1].data.push_back(1);
    config.pktList[1].pktDelay = 100;
    config.pktList[1].clientTx = false;
    config.pktList[2].data.push_back(1);
    config.pktList[2].pktDelay = 101;
    config.pktList[2].clientTx = true;

    // create flow 
    fe->UpdateFlow(handle, MockFlowConfigProxy(config)); 

    // create a mock stream playlist instance
    uint32_t pl_handle = 1;
    PlaylistConfig playlistConfig;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].serverPort = 123;

    MockStreamPlaylistInstance * pi = new MockStreamPlaylistInstance (*pe, *sm, playlistConfig, pl_handle, true);

    FlowInstance *fi = &(pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());

    fi->Start();

    // should send data
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    // and prepare to receive
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());

    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());
    handle_t timerHandle = fi->mInactivityTimer;
    CPPUNIT_ASSERT_EQUAL(size_t(DPG_FLOW_INACTIVITY_TIMEOUT), tm->GetTimers()[timerHandle].msDelay);

    // fire off send delegate to wait for next packet
    sm->GetSends().begin()->sendDelegate();

    // fire off recv delegate to send next packet
    sm->GetRecvs().begin()->recvDelegate(&config.pktList[1].data[0], config.pktList[1].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    // timer setup for next Tx
    timerHandle = fi->mTimer;
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(config.pktList[1].pktDelay, tm->GetTimers()[timerHandle].msDelay);

    fi->Failed();

    // should cancel timer
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetCancelledTimers().size());
    CPPUNIT_ASSERT_EQUAL(timerHandle, *(++++tm->GetCancelledTimers().begin()));
    CPPUNIT_ASSERT_EQUAL(tm->InvalidTimerHandle(), fi->mTimer);
    CPPUNIT_ASSERT_EQUAL(tm->InvalidTimerHandle(), fi->mInactivityTimer);

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mFailedStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mSuccessfulStreams);
}

///////////////////////////////////////////////////////////////////////////////

void TestStreamInstance::testSuccessfulStream(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
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

    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(flowConfig));

    // create a mock stream playlist instance
    uint32_t pl_handle = 1;
    PlaylistConfig playlistConfig;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].serverPort = 123;

    MockStreamPlaylistInstance * pi = new MockStreamPlaylistInstance (*pe, *sm, playlistConfig, pl_handle, true);

    FlowInstance *fi = &(pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());

    fi->Start();

    // should send data
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    // and prepare for receive
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].data.size(), sm->GetSends().begin()->dataLength);

    // fire off send delegate - 2nd packet: Rx
    sm->GetSends().begin()->sendDelegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(fi->GetFd(), sm->GetRecvs().begin()->fd);
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].data.size(), sm->GetRecvs().begin()->dataLength);

    // fire off recv delegate - setup timer for 3rd packet: Tx
    sm->GetRecvs().begin()->recvDelegate(&flowConfig.pktList[1].data[0], flowConfig.pktList[1].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].pktDelay, tm->GetTimers()[fi->mTimer].msDelay);

    // new inactivity timer
    CPPUNIT_ASSERT_EQUAL(size_t(DPG_FLOW_INACTIVITY_TIMEOUT), tm->GetTimers()[fi->mInactivityTimer].msDelay);

    // fire off timer - 3rd packet:Tx
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    // new inactivity timer
    CPPUNIT_ASSERT_EQUAL(size_t(4), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), tm->GetCancelledTimers().size());
    const handle_t invalidTimer = tm->InvalidTimerHandle();
    CPPUNIT_ASSERT_EQUAL(invalidTimer, fi->mTimer);
    CPPUNIT_ASSERT_EQUAL(size_t(DPG_FLOW_INACTIVITY_TIMEOUT), tm->GetTimers()[fi->mInactivityTimer].msDelay);

    // fire off send delegate - 4th packet:Rx
    (++sm->GetSends().begin())->sendDelegate();
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(fi->GetFd(), (++sm->GetRecvs().begin())->fd);
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[3].data.size(), (++sm->GetRecvs().begin())->dataLength);
    // new inactivity timer
    CPPUNIT_ASSERT_EQUAL(size_t(4), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), tm->GetCancelledTimers().size());

    // fire off recv delegate - setup timer for 5th packet: Tx
    (++sm->GetRecvs().begin())->recvDelegate(&flowConfig.pktList[3].data[0], flowConfig.pktList[3].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(6), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[3].pktDelay, tm->GetTimers()[fi->mTimer].msDelay);
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetCancelledTimers().size());

    // fire off timer - 5th packet:Tx
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(fi->GetFd(), (++++sm->GetSends().begin())->fd);
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[4].data.size(), (++++sm->GetSends().begin())->dataLength);
    // new inactivity timer
    CPPUNIT_ASSERT_EQUAL(size_t(7), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(4), tm->GetCancelledTimers().size());
    CPPUNIT_ASSERT_EQUAL(invalidTimer, fi->mTimer);
    CPPUNIT_ASSERT_EQUAL(size_t(DPG_FLOW_INACTIVITY_TIMEOUT), tm->GetTimers()[fi->mInactivityTimer].msDelay);

    // fire off send delegate - complete the flow
    (++++sm->GetSends().begin())->sendDelegate();
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(7), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(5), tm->GetCancelledTimers().size());

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mFailedStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mSuccessfulStreams);
}

///////////////////////////////////////////////////////////////////////////////

void TestStreamInstance::testAdjustedPktDelays(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.pktList.resize(5);
    flowConfig.pktList[0].data.push_back(1);
    flowConfig.pktList[0].pktDelay = 5;
    flowConfig.pktList[0].clientTx = true;
    flowConfig.pktList[1].data.push_back(2);
    flowConfig.pktList[1].pktDelay = 8;
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

    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(flowConfig));

    // create a mock stream playlist instance
    uint32_t pl_handle = 1;
    PlaylistConfig playlistConfig;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].minPktDelay = 10;
    playlistConfig.flows[0].maxPktDelay =102;
    playlistConfig.flows[0].serverPort = 123;

    MockStreamPlaylistInstance * pi = new MockStreamPlaylistInstance (*pe, *sm, playlistConfig, pl_handle, true);

    FlowInstance *fi = &(pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());

    fi->Start();

    // fire off send delegate - 2nd packet: Rx
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    sm->GetSends().begin()->sendDelegate();

    // fire off recv delegate - setup timer for 3rd packet: Tx
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    sm->GetRecvs().begin()->recvDelegate(&flowConfig.pktList[1].data[0], flowConfig.pktList[1].data.size());
    // should be the min delay
    CPPUNIT_ASSERT_EQUAL(playlistConfig.flows[0].minPktDelay, tm->GetTimers()[fi->mTimer].msDelay);

    // fire off timer - 3rd packet:Tx
    tm->GetTimers()[fi->mTimer].delegate();

    // fire off send delegate - 4th packet:Rx
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    (++sm->GetSends().begin())->sendDelegate();

    // fire off recv delegate - setup timer for 5th packet: Tx
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size());
    (++sm->GetRecvs().begin())->recvDelegate(&flowConfig.pktList[3].data[0], flowConfig.pktList[3].data.size());
    // should be the max delay
    CPPUNIT_ASSERT_EQUAL(playlistConfig.flows[0].maxPktDelay, tm->GetTimers()[fi->mTimer].msDelay);

    // fire off timer - 5th packet:Tx
    tm->GetTimers()[fi->mTimer].delegate();

    // fire off send delegate - complete the flow
    (++++sm->GetSends().begin())->sendDelegate();

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mFailedStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mSuccessfulStreams);
}

///////////////////////////////////////////////////////////////////////////////

void TestStreamInstance::testStreamWithAllRx(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.pktList.resize(3);
    flowConfig.pktList[0].data.push_back(1);
    flowConfig.pktList[0].pktDelay = 100;
    flowConfig.pktList[0].clientTx = false;
    flowConfig.pktList[1].data.push_back(2);
    flowConfig.pktList[1].pktDelay = 101;
    flowConfig.pktList[1].clientTx = false;
    flowConfig.pktList[2].data.push_back(3);
    flowConfig.pktList[2].pktDelay = 102;
    flowConfig.pktList[2].clientTx = false;

    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(flowConfig));

    // create a mock stream playlist instance
    uint32_t pl_handle = 1;
    PlaylistConfig playlistConfig;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].serverPort = 123;

    MockStreamPlaylistInstance * pi = new MockStreamPlaylistInstance (*pe, *sm, playlistConfig, pl_handle, true);

    FlowInstance *fi = &(pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());

    fi->Start();

    // should receive data
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].data.size(), sm->GetRecvs().begin()->dataLength);

    // fire off recv delegate - 2nd Rx
    sm->GetRecvs().begin()->recvDelegate(&flowConfig.pktList[0].data[0], flowConfig.pktList[0].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetCancelledTimers().size());

    // fire off recv delegate - 3rd Rx
    (++sm->GetRecvs().begin())->recvDelegate(&flowConfig.pktList[1].data[0], flowConfig.pktList[1].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), tm->GetCancelledTimers().size());

    // fire off recv delegate - Complete
    (++++sm->GetRecvs().begin())->recvDelegate(&flowConfig.pktList[2].data[0], flowConfig.pktList[2].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetCancelledTimers().size());

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mFailedStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mSuccessfulStreams);
}

///////////////////////////////////////////////////////////////////////////////

void TestStreamInstance::testStreamWithAllTx(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.pktList.resize(3);
    flowConfig.pktList[0].data.push_back(1);
    flowConfig.pktList[0].pktDelay = 100;
    flowConfig.pktList[0].clientTx = true;
    flowConfig.pktList[1].data.push_back(2);
    flowConfig.pktList[1].pktDelay = 101;
    flowConfig.pktList[1].clientTx = true;
    flowConfig.pktList[2].data.push_back(3);
    flowConfig.pktList[2].pktDelay = 102;
    flowConfig.pktList[2].clientTx = true;

    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(flowConfig));

    // create a mock stream playlist instance
    uint32_t pl_handle = 1;
    PlaylistConfig playlistConfig;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].serverPort = 123;

    MockStreamPlaylistInstance * pi = new MockStreamPlaylistInstance (*pe, *sm, playlistConfig, pl_handle, true);

    FlowInstance *fi = &(pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());

    fi->Start();

    // should send data
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].data.size(), sm->GetSends().begin()->dataLength);

    // fire off send delegate - setup timer for 2nd packet: Tx
    sm->GetSends().begin()->sendDelegate();
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(fi->GetFd(), sm->GetSends().begin()->fd);
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].data.size(), sm->GetSends().begin()->dataLength);
    // we may have two active timers at the same time: one for send, one for flow timeout.
    CPPUNIT_ASSERT_EQUAL(size_t(2), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetCancelledTimers().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].pktDelay, tm->GetTimers()[fi->mTimer].msDelay);

    // fire off timer - 2nd packet:Tx
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetCancelledTimers().size());

    // fire off send delegate - setup timer for 3rd packet: Tx
    (++sm->GetSends().begin())->sendDelegate();
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(fi->GetFd(), sm->GetSends().begin()->fd);
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].data.size(), sm->GetSends().begin()->dataLength);
    CPPUNIT_ASSERT_EQUAL(size_t(4), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), tm->GetCancelledTimers().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].pktDelay, tm->GetTimers()[fi->mTimer].msDelay);

    // fire off timer - 3rd packet:Tx
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetSends().size());
    // new inactivity timer
    CPPUNIT_ASSERT_EQUAL(size_t(5), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), tm->GetCancelledTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(DPG_FLOW_INACTIVITY_TIMEOUT), tm->GetTimers()[fi->mInactivityTimer].msDelay);

    // fire off send delegate - Complete
    (++++sm->GetSends().begin())->sendDelegate();
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(fi->GetFd(), (++++sm->GetSends().begin())->fd);
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[2].data.size(), (++++sm->GetSends().begin())->dataLength);
    CPPUNIT_ASSERT_EQUAL(size_t(5), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(tm->InvalidTimerHandle(), fi->mTimer);
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetCancelledTimers().size());

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mFailedStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mSuccessfulStreams);
}

///////////////////////////////////////////////////////////////////////////////
//
// flow timeout while waiting for an incoming packet
//
void TestStreamInstance::testRxTimeoutStream(void)
{
    uint32_t handle = 1;
    FlowConfig config;
    config.pktList.resize(3);
    config.pktList[0].data.push_back(1);
    config.pktList[0].pktDelay = 99;
    config.pktList[0].clientTx = true;
    config.pktList[1].data.push_back(1);
    config.pktList[1].pktDelay = 100;
    config.pktList[1].clientTx = false;
    config.pktList[2].data.push_back(1);
    config.pktList[2].pktDelay = 101;
    config.pktList[2].clientTx = true;

    // create flow 
    fe->UpdateFlow(handle, MockFlowConfigProxy(config)); 

    // create a mock stream playlist instance
    uint32_t pl_handle = 1;
    PlaylistConfig playlistConfig;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].serverPort = 123;

    MockStreamPlaylistInstance * pi = new MockStreamPlaylistInstance (*pe, *sm, playlistConfig, pl_handle, true);

    FlowInstance *fi = &(pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());

    fi->Start();

    // should send data
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    // and prepare to receive
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());

    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());
    handle_t timerHandle = fi->mInactivityTimer;
    CPPUNIT_ASSERT_EQUAL(size_t(DPG_FLOW_INACTIVITY_TIMEOUT), tm->GetTimers()[timerHandle].msDelay);

    // fire off send delegate to wait for next packet
    sm->GetSends().begin()->sendDelegate();

    // new inactivity timer
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), tm->GetCancelledTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(DPG_FLOW_INACTIVITY_TIMEOUT), tm->GetTimers()[fi->mInactivityTimer].msDelay);

    // packet never received, fire inactivity timer
    tm->GetTimers()[fi->mInactivityTimer].delegate();

    // should cancel timer
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetCancelledTimers().size());
    CPPUNIT_ASSERT_EQUAL(tm->InvalidTimerHandle(), fi->mTimer);
    CPPUNIT_ASSERT_EQUAL(tm->InvalidTimerHandle(), fi->mInactivityTimer);

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mFailedStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mSuccessfulStreams);
}

///////////////////////////////////////////////////////////////////////////////
//
// flow timeout while waiting for callback after sending packet
//
void TestStreamInstance::testTxTimeoutStream(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.pktList.resize(3);
    flowConfig.pktList[0].data.push_back(1);
    flowConfig.pktList[0].pktDelay = 100;
    flowConfig.pktList[0].clientTx = true;
    flowConfig.pktList[1].data.push_back(2);
    flowConfig.pktList[1].pktDelay = 101;
    flowConfig.pktList[1].clientTx = true;
    flowConfig.pktList[2].data.push_back(3);
    flowConfig.pktList[2].pktDelay = 102;
    flowConfig.pktList[2].clientTx = true;

    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(flowConfig));

    // create a mock stream playlist instance
    uint32_t pl_handle = 1;
    PlaylistConfig playlistConfig;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].serverPort = 123;

    MockStreamPlaylistInstance * pi = new MockStreamPlaylistInstance (*pe, *sm, playlistConfig, pl_handle, true);

    FlowInstance *fi = &(pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());

    fi->Start();

    // should send data
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].data.size(), sm->GetSends().begin()->dataLength);

    // never fire off send delegate, flow times out.
    tm->GetTimers()[fi->mInactivityTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetCancelledTimers().size());

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mFailedStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mSuccessfulStreams);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// In the rare case, if packet delay larger than the flow timeout value (currently 100s)
//
void TestStreamInstance::testTooLongPacketDelay(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.pktList.resize(3);
    flowConfig.pktList[0].data.push_back(1);
    flowConfig.pktList[0].pktDelay = 100;
    flowConfig.pktList[0].clientTx = true;
    flowConfig.pktList[1].data.push_back(2);
    flowConfig.pktList[1].pktDelay = 101000;
    flowConfig.pktList[1].clientTx = true;
    flowConfig.pktList[2].data.push_back(3);
    flowConfig.pktList[2].pktDelay = 102;
    flowConfig.pktList[2].clientTx = false;

    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(flowConfig));

    // create a mock stream playlist instance
    uint32_t pl_handle = 1;
    PlaylistConfig playlistConfig;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].serverPort = 123;

    MockStreamPlaylistInstance * pi = new MockStreamPlaylistInstance (*pe, *sm, playlistConfig, pl_handle, true);

    FlowInstance *fi = &(pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());

    fi->Start();

    // should send data
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].data.size(), sm->GetSends().begin()->dataLength);

    // fire off send delegate - setup timer for 2nd packet: Tx
    sm->GetSends().begin()->sendDelegate();
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(fi->GetFd(), sm->GetSends().begin()->fd);
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].data.size(), sm->GetSends().begin()->dataLength);
    // we may have two active timers at the same time: one for send, one for flow timeout.
    CPPUNIT_ASSERT_EQUAL(size_t(2), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetCancelledTimers().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].pktDelay, tm->GetTimers()[fi->mTimer].msDelay);

    // fire off timer - 2nd packet:Tx
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetCancelledTimers().size());

    // fire off send delegate - setup timer for 3rd packet: Rx
    (++sm->GetSends().begin())->sendDelegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(fi->GetFd(), sm->GetSends().begin()->fd);
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].data.size(), sm->GetSends().begin()->dataLength);
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetCancelledTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(DPG_FLOW_INACTIVITY_TIMEOUT), tm->GetTimers()[fi->mInactivityTimer].msDelay);

    // flow times out first before the timer for the 3rd Rx
    tm->GetTimers()[fi->mInactivityTimer].delegate();
    // inactivity timer cancelled
    CPPUNIT_ASSERT_EQUAL(size_t(2), tm->GetCancelledTimers().size());

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mFailedStreams);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mSuccessfulStreams);
}

///////////////////////////////////////////////////////////////////////////////

void TestStreamInstance::testPktLoopsStream(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
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

    FlowConfig::LoopInfo firstLoop; // from pkt 2 to pkt 2, loop 2 times
    firstLoop.count = 2;
    firstLoop.begIdx = 2;
    flowConfig.loopMap.insert(std::make_pair(2,firstLoop));

    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(flowConfig));

    // create a mock stream playlist instance
    uint32_t pl_handle = 1;
    PlaylistConfig playlistConfig;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].serverPort = 123;

    MockStreamPlaylistInstance * pi = new MockStreamPlaylistInstance (*pe, *sm, playlistConfig, pl_handle, true);

    FlowInstance *fi = &(pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());

    fi->Start();

    // should send data
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    // and prepare to receive
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].data.size(), sm->GetSends().begin()->dataLength);

    // fire off send delegate - 2nd packet: Rx
    sm->GetSends().begin()->sendDelegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(fi->GetFd(), sm->GetRecvs().begin()->fd);
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].data.size(), sm->GetRecvs().begin()->dataLength);

    // fire off recv delegate - setup timer for 3rd packet: Tx
    sm->GetRecvs().begin()->recvDelegate(&flowConfig.pktList[1].data[0], flowConfig.pktList[1].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].pktDelay, tm->GetTimers()[fi->mTimer].msDelay);

    // fire off timer - 3rd packet:Tx, which should loop back to itself
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[2].data.size(), (++sm->GetSends().begin())->dataLength);

    // fire off send delegate - 3rd packet:Tx, which should loop back to itself
    (++sm->GetSends().begin())->sendDelegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());

    // fire off timer - second time of 3rd packet:Tx, which should be the last iteration
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size()); // prepare to rx
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[2].data.size(), (++++sm->GetSends().begin())->dataLength);

    // fire off send delegate - 4th packet:Rx
    (++++sm->GetSends().begin())->sendDelegate();
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(fi->GetFd(), (++sm->GetRecvs().begin())->fd);
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[3].data.size(), (++sm->GetRecvs().begin())->dataLength);
}

///////////////////////////////////////////////////////////////////////////////

void TestStreamInstance::testPktLoopsStream2(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
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

    FlowConfig::LoopInfo firstLoop; // from pkt 2 to pkt 2, loop 3 times
    firstLoop.count = 3;
    firstLoop.begIdx = 2;
    flowConfig.loopMap.insert(std::make_pair(2,firstLoop));

    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(flowConfig));

    // create a mock stream playlist instance
    uint32_t pl_handle = 1;
    PlaylistConfig playlistConfig;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].serverPort = 123;

    MockStreamPlaylistInstance * pi = new MockStreamPlaylistInstance (*pe, *sm, playlistConfig, pl_handle, false);

    FlowInstance *fi = &(pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    fi->Start();

    // should receive data
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].data.size(), sm->GetRecvs().begin()->dataLength);

    // fire off recv delegate - setup timer for 2nd packet: Tx
    sm->GetRecvs().begin()->recvDelegate(&flowConfig.pktList[0].data[0], flowConfig.pktList[0].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].pktDelay, tm->GetTimers()[fi->mTimer].msDelay);

    // fire off timer
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size()); // preparing for next rx
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].data.size(), sm->GetSends().begin()->dataLength);

    // fire off send delegate - 3rd packet: Rx, which should loop back to itself
    sm->GetSends().begin()->sendDelegate();
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(fi->GetFd(), (++sm->GetRecvs().begin())->fd);
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[2].data.size(), (++sm->GetRecvs().begin())->dataLength);

    // fire off recv delegate - should loop back to the 3rd packet
    (++sm->GetRecvs().begin())->recvDelegate(&flowConfig.pktList[2].data[0], flowConfig.pktList[2].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());

    // fire off recv delegate - should loop back to itself again
    (++++sm->GetRecvs().begin())->recvDelegate(&flowConfig.pktList[2].data[0], flowConfig.pktList[2].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(4), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());

    // fire off recv delegate - should continue to the 4th packet, which should setup the timer for send
    (++++++sm->GetRecvs().begin())->recvDelegate(&flowConfig.pktList[2].data[0], flowConfig.pktList[2].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(4), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());

    // fire off timer - 4th packet - Tx
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(5), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[3].data.size(), (++sm->GetSends().begin())->dataLength);
}

///////////////////////////////////////////////////////////////////////////////

void TestStreamInstance::testNestedPktLoopsStream(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
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

    FlowConfig::LoopInfo firstLoop; // from pkt 2 to pkt 2, loop 2 times
    firstLoop.count = 2;
    firstLoop.begIdx = 2;
    flowConfig.loopMap.insert(std::make_pair(2,firstLoop));
    FlowConfig::LoopInfo secondLoop; // from pkt 1 to pkt 3, loop 2 times
    secondLoop.count = 2;
    secondLoop.begIdx = 1;
    flowConfig.loopMap.insert(std::make_pair(3,secondLoop));

    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(flowConfig));

    // create a mock stream playlist instance
    uint32_t pl_handle = 1;
    PlaylistConfig playlistConfig;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].serverPort = 123;

    MockStreamPlaylistInstance * pi = new MockStreamPlaylistInstance (*pe, *sm, playlistConfig, pl_handle, true);

    FlowInstance *fi = &(pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());

    fi->Start();

    // should send data
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    // and prepare to receive
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].data.size(), sm->GetSends().begin()->dataLength);

    // fire off send delegate - 2nd packet: Rx
    sm->GetSends().begin()->sendDelegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(fi->GetFd(), sm->GetRecvs().begin()->fd);
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].data.size(), sm->GetRecvs().begin()->dataLength);


    // fire off recv delegate - setup timer for 3rd packet: Tx
    sm->GetRecvs().begin()->recvDelegate(&flowConfig.pktList[1].data[0], flowConfig.pktList[1].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].pktDelay, tm->GetTimers()[fi->mTimer].msDelay);

    // fire off timer - 3rd packet:Tx, which should loop back to itself
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[2].data.size(), (++sm->GetSends().begin())->dataLength);

    // fire off send delegate - 3rd packet:Tx, which should loop back to itself
    (++sm->GetSends().begin())->sendDelegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());

    // fire off timer - second time of 3rd packet:Tx, which should be the last iteration
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[2].data.size(), (++++sm->GetSends().begin())->dataLength);

    // fire off send delegate - 4th packet:Rx, which is the end of another loop
    (++++sm->GetSends().begin())->sendDelegate();
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(fi->GetFd(), (++sm->GetRecvs().begin())->fd);

    // fire off recv delegate, which should loop back to pkt #1- Rx
    (++sm->GetRecvs().begin())->recvDelegate(&flowConfig.pktList[3].data[0], flowConfig.pktList[3].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetSends().size());

    // fireoff recv delegate, should set timer for pkt #2
    (++++sm->GetRecvs().begin())->recvDelegate(&flowConfig.pktList[1].data[0], flowConfig.pktList[1].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetSends().size());

    // fire off timer
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(4), sm->GetSends().size());

    // fire off send delegate, which should loop back to itself
    (++++++sm->GetSends().begin())->sendDelegate();
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(4), sm->GetSends().size());

    // fire off timer
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(4), sm->GetRecvs().size()); // prepare to rx
    CPPUNIT_ASSERT_EQUAL(size_t(5), sm->GetSends().size());

    // fire off send delegate for pkt #2 again, should continue with pkt #3 - Rx
    (++++++++sm->GetSends().begin())->sendDelegate();
    CPPUNIT_ASSERT_EQUAL(size_t(4), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(5), sm->GetSends().size());

    // fire off recv delegate for pkt #3 again, which should be the last iteration
    (++++++sm->GetRecvs().begin())->recvDelegate(&flowConfig.pktList[3].data[0], flowConfig.pktList[3].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(4), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(5), sm->GetSends().size());

    // timer should be setup for pkt #4
    // fire off timer for pkt #4
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(4), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(6), sm->GetSends().size());
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestStreamInstance);
