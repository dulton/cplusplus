#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

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

class TestAttackInstance : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestAttackInstance);
    CPPUNIT_TEST(testStartClientAttack);
    CPPUNIT_TEST(testStartTCPServerAttack);
    CPPUNIT_TEST(testStartUDPServerAttack);
    CPPUNIT_TEST(testNoPacketsAttack);
    CPPUNIT_TEST(testAbortAttack);
    CPPUNIT_TEST(testSuccessfulTCPClient);
    CPPUNIT_TEST(testSuccessfulUDPClient);
    CPPUNIT_TEST(testSuccessfulTCPServer);
    CPPUNIT_TEST(testSuccessfulUDPServer);
    CPPUNIT_TEST(testPktLoopClient);
    CPPUNIT_TEST(testUDPInactivityTimer);
    CPPUNIT_TEST(testTCPInactivityTimer);
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
    void testStartClientAttack(void);
    void testStartTCPServerAttack(void);
    void testStartUDPServerAttack(void);
    void testNoPacketsAttack(void);
    void testAbortAttack(void);
    void testSuccessfulTCPClient(void);
    void testSuccessfulUDPClient(void);
    void testSuccessfulTCPServer(void);
    void testSuccessfulUDPServer(void);
    void testPktLoopClient(void);
    void testUDPInactivityTimer(void);
    void testTCPInactivityTimer(void);

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

void TestAttackInstance::testStartClientAttack(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.playType = FlowConfig::P_ATTACK;
    flowConfig.layer = FlowConfig::TCP;
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
    PlaylistConfig playlistConfig;
    playlistConfig.type = PlaylistConfig::ATTACK_ONLY;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 3;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].minPktDelay = 200;
    playlistConfig.flows[0].socketTimeout = 100000;
    playlistConfig.flows[0].serverPort = 123;

    MockAttackPlaylistInstance * pi = new MockAttackPlaylistInstance (*pe, *sm, playlistConfig, true);

    AttackInstance *fi = static_cast<AttackInstance *>(&pi->GetFlow(0));
    fi->SetFd(1); // fake fd

    fi->Start();

    // should grouped all packets
    CPPUNIT_ASSERT_EQUAL(size_t(1), fi->mClientPayload.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), fi->mClientPayload[0]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), fi->mServerPayload[0]);

    // should setup 2 timers
    CPPUNIT_ASSERT_EQUAL(size_t(2), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(fi->mInactivityTimeout, tm->GetTimers()[fi->mInactivityTimer].msDelay);
    CPPUNIT_ASSERT_EQUAL(size_t(fi->mWaitTimeout/3), tm->GetTimers()[fi->mTimer].msDelay);

    // should received a packet
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mFailedAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mSuccessfulAttacks);
}

///////////////////////////////////////////////////////////////////////////////

void TestAttackInstance::testStartTCPServerAttack(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.playType = FlowConfig::P_ATTACK;
    flowConfig.layer = FlowConfig::TCP;
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
    PlaylistConfig playlistConfig;
    playlistConfig.type = PlaylistConfig::ATTACK_ONLY;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 3;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].minPktDelay = 200;
    playlistConfig.flows[0].socketTimeout = 100000;
    playlistConfig.flows[0].serverPort = 123;

    MockAttackPlaylistInstance * pi = new MockAttackPlaylistInstance (*pe, *sm, playlistConfig, false);

    AttackInstance *fi = static_cast<AttackInstance *>(&pi->GetFlow(0));
    fi->SetFd(1); // fake fd

    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());

    fi->Start();

    // should grouped all packets
    CPPUNIT_ASSERT_EQUAL(size_t(1), fi->mClientPayload.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), fi->mClientPayload[0]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), fi->mServerPayload[0]);

    // should setup 2 timers
    CPPUNIT_ASSERT_EQUAL(size_t(2), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(fi->mInactivityTimeout, tm->GetTimers()[fi->mInactivityTimer].msDelay);
    CPPUNIT_ASSERT_EQUAL(size_t(fi->mWaitTimeout), tm->GetTimers()[fi->mTimer].msDelay);

    // should received a packet
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mFailedAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mSuccessfulAttacks);
}

///////////////////////////////////////////////////////////////////////////////

void TestAttackInstance::testStartUDPServerAttack(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.playType = FlowConfig::P_ATTACK;
    flowConfig.layer = FlowConfig::UDP;
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
    PlaylistConfig playlistConfig;
    playlistConfig.type = PlaylistConfig::ATTACK_ONLY;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 3;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].minPktDelay = 200;
    playlistConfig.flows[0].socketTimeout = 10000;
    playlistConfig.flows[0].serverPort = 123;

    MockAttackPlaylistInstance * pi = new MockAttackPlaylistInstance (*pe, *sm, playlistConfig, false);

    AttackInstance *fi = static_cast<AttackInstance *>(&pi->GetFlow(0));
    fi->SetFd(1); // fake fd

    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());

    fi->Start();

    // should grouped all packets
    CPPUNIT_ASSERT_EQUAL(size_t(1), fi->mClientPayload.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), fi->mClientPayload[0]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), fi->mServerPayload[0]);

    // should setup 1 timer
    CPPUNIT_ASSERT_EQUAL(size_t(1), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(fi->mInactivityTimeout), tm->GetTimers()[fi->mInactivityTimer].msDelay);

    // should received a packet
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());

    // fire the delegate of the first receive should set up the wait timer
    sm->GetRecvs().begin()->recvDelegate(&flowConfig.pktList[1].data[0], flowConfig.pktList[1].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(fi->mInactivityTimeout), tm->GetTimers()[fi->mInactivityTimer].msDelay);
    // The first receive should be immediately followed by the first send
    CPPUNIT_ASSERT_EQUAL(size_t(0), tm->GetTimers()[fi->mTimer].msDelay);

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mFailedAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mSuccessfulAttacks);
}

///////////////////////////////////////////////////////////////////////////////

void TestAttackInstance::testNoPacketsAttack(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.playType = FlowConfig::P_ATTACK;
    flowConfig.layer = FlowConfig::TCP;
    flowConfig.pktList.resize(0);
    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(flowConfig));

    // create a mock stream playlist instance
    PlaylistConfig playlistConfig;
    playlistConfig.type = PlaylistConfig::ATTACK_ONLY;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 3;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].minPktDelay = 200;
    playlistConfig.flows[0].serverPort = 123;

    MockAttackPlaylistInstance * pi = new MockAttackPlaylistInstance (*pe, *sm, playlistConfig, true);

    AttackInstance *fi = static_cast<AttackInstance *>(&pi->GetFlow(0));
    fi->SetFd(1); // fake fd

    fi->Start();

    // should grouped all packets
    CPPUNIT_ASSERT_EQUAL(size_t(0), fi->mClientPayload.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), fi->mServerPayload.size());

    // should not setup any timer
    CPPUNIT_ASSERT_EQUAL(size_t(0), tm->GetTimers().size());

    // should not send or receive any packet
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetRecvs().size());

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mFailedAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mSuccessfulAttacks);
}

///////////////////////////////////////////////////////////////////////////////

void TestAttackInstance::testAbortAttack(void)
{
    uint32_t handle = 1;
    FlowConfig config;
    config.playType = FlowConfig::P_ATTACK;
    config.layer = FlowConfig::TCP;
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
    PlaylistConfig playlistConfig;
    playlistConfig.type = PlaylistConfig::ATTACK_ONLY;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].minPktDelay = 200;
    playlistConfig.flows[0].socketTimeout = 100000;
    playlistConfig.flows[0].serverPort = 123;

    MockAttackPlaylistInstance * pi = new MockAttackPlaylistInstance (*pe, *sm, playlistConfig, true);

    AttackInstance *fi = static_cast<AttackInstance *>(&pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    CPPUNIT_ASSERT_EQUAL(size_t(0), sm->GetSends().size());

    fi->Start();
    // fire off wait timer
    tm->GetTimers()[fi->mTimer].delegate();

    // should send data
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());


    // fire off send delegate to wait for next packet
    sm->GetSends().begin()->sendDelegate();

    // new wait timer
    CPPUNIT_ASSERT_EQUAL(size_t(4), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(fi->mWaitTimeout), tm->GetTimers()[fi->mTimer].msDelay);
    // new inactivity timer
    handle_t timerHandle = fi->mInactivityTimer;
    CPPUNIT_ASSERT_EQUAL(fi->mInactivityTimeout, tm->GetTimers()[timerHandle].msDelay);

    // fire off recv delegate
    sm->GetRecvs().begin()->recvDelegate(&config.pktList[1].data[0], config.pktList[1].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());

    fi->Failed();

    // should cancel timer
    CPPUNIT_ASSERT_EQUAL(size_t(4), tm->GetCancelledTimers().size());
    CPPUNIT_ASSERT_EQUAL(tm->InvalidTimerHandle(), fi->mTimer);
    CPPUNIT_ASSERT_EQUAL(tm->InvalidTimerHandle(), fi->mInactivityTimer);

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mFailedAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mSuccessfulAttacks);
}

///////////////////////////////////////////////////////////////////////////////

void TestAttackInstance::testSuccessfulTCPClient(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.playType = FlowConfig::P_ATTACK;
    flowConfig.layer = FlowConfig::TCP;
    flowConfig.pktList.resize(3);
    flowConfig.pktList[0].data.push_back(1);
    flowConfig.pktList[0].pktDelay = 100;
    flowConfig.pktList[0].clientTx = true;
    flowConfig.pktList[1].data.push_back(2);
    flowConfig.pktList[1].pktDelay = 101;
    flowConfig.pktList[1].clientTx = false;
    flowConfig.pktList[2].data.push_back(3);
    flowConfig.pktList[2].pktDelay = 102;
    flowConfig.pktList[2].clientTx = true;

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

    MockAttackPlaylistInstance * pi = new MockAttackPlaylistInstance (*pe, *sm, playlistConfig, true);

    AttackInstance *fi = static_cast<AttackInstance *>(&pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    fi->Start();

    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].data.size(), sm->GetRecvs().begin()->dataLength);

    // fire off wait timer
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].data.size(), sm->GetSends().begin()->dataLength);

    // fire off send delegate
    sm->GetSends().begin()->sendDelegate();

    //fire off the receive
    sm->GetRecvs().begin()->recvDelegate(&flowConfig.pktList[1].data[0], flowConfig.pktList[1].data.size());

    // fire off wait timer again
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[2].data.size(), (++sm->GetSends().begin())->dataLength);

    // fire off send delegate
    (++sm->GetSends().begin())->sendDelegate();

    // fire off wait timer - complete the flow
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(7), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(4), tm->GetCancelledTimers().size());

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mFailedAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mSuccessfulAttacks);
}

///////////////////////////////////////////////////////////////////////////////

void TestAttackInstance::testSuccessfulUDPClient(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.playType = FlowConfig::P_ATTACK;
    flowConfig.layer = FlowConfig::UDP;
    flowConfig.pktList.resize(3);
    flowConfig.pktList[0].data.push_back(1);
    flowConfig.pktList[0].pktDelay = 100;
    flowConfig.pktList[0].clientTx = true;
    flowConfig.pktList[1].data.push_back(2);
    flowConfig.pktList[1].pktDelay = 101;
    flowConfig.pktList[1].clientTx = false;
    flowConfig.pktList[2].data.push_back(3);
    flowConfig.pktList[2].pktDelay = 102;
    flowConfig.pktList[2].clientTx = true;

    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(flowConfig));

    // create a mock attack playlist instance
    PlaylistConfig playlistConfig;
    playlistConfig.type = PlaylistConfig::ATTACK_ONLY;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].minPktDelay = 200;
    playlistConfig.flows[0].serverPort = 123;

    MockAttackPlaylistInstance * pi = new MockAttackPlaylistInstance (*pe, *sm, playlistConfig, true);

    AttackInstance *fi = static_cast<AttackInstance *>(&pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    fi->Start();

    // started to receive already
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].data.size(), sm->GetRecvs().begin()->dataLength);

    // fire off wait timer
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].data.size(), sm->GetSends().begin()->dataLength);

    // fire off send delegate
    sm->GetSends().begin()->sendDelegate();

    // fire off wait timer again
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[2].data.size(), sm->GetSends().begin()->dataLength);

    // fire off send delegate
    (++sm->GetSends().begin())->sendDelegate();

    // fire off wait timer - complete the flow
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(6), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), tm->GetCancelledTimers().size());

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mFailedAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mSuccessfulAttacks);
}

///////////////////////////////////////////////////////////////////////////////

void TestAttackInstance::testSuccessfulTCPServer(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.playType = FlowConfig::P_ATTACK;
    flowConfig.layer = FlowConfig::TCP;
    flowConfig.pktList.resize(3);
    flowConfig.pktList[0].data.push_back(1);
    flowConfig.pktList[0].pktDelay = 100;
    flowConfig.pktList[0].clientTx = true;
    flowConfig.pktList[1].data.push_back(2);
    flowConfig.pktList[1].pktDelay = 101;
    flowConfig.pktList[1].clientTx = false;
    flowConfig.pktList[2].data.push_back(3);
    flowConfig.pktList[2].pktDelay = 102;
    flowConfig.pktList[2].clientTx = true;

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

    MockAttackPlaylistInstance * pi = new MockAttackPlaylistInstance (*pe, *sm, playlistConfig, false);

    AttackInstance *fi = static_cast<AttackInstance *>(&pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    fi->Start();

    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].data.size(), sm->GetRecvs().begin()->dataLength);

    // fire off wait timer
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].data.size(), sm->GetSends().begin()->dataLength);

    // fire off send delegate
    sm->GetSends().begin()->sendDelegate();

    // fire off wait timer again, should be waiting for more packets from the client
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mFailedAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mSuccessfulAttacks);

    // fire off recv delegate
    sm->GetRecvs().begin()->recvDelegate(&flowConfig.pktList[0].data[0], flowConfig.pktList[0].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size());

    // fire off 2nd recv delegate
    (++sm->GetRecvs().begin())->recvDelegate(&flowConfig.pktList[2].data[0], flowConfig.pktList[2].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size());

    // no wait timer set
    CPPUNIT_ASSERT_EQUAL(tm->InvalidTimerHandle(), fi->mTimer);

    // server will wait for client to close the socket
    // so there should not have called FlowDone
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mFailedAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mSuccessfulAttacks);
}

///////////////////////////////////////////////////////////////////////////////

void TestAttackInstance::testSuccessfulUDPServer(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.playType = FlowConfig::P_ATTACK;
    flowConfig.layer = FlowConfig::UDP;
    flowConfig.pktList.resize(3);
    flowConfig.pktList[0].data.push_back(1);
    flowConfig.pktList[0].pktDelay = 100;
    flowConfig.pktList[0].clientTx = true;
    flowConfig.pktList[1].data.push_back(2);
    flowConfig.pktList[1].pktDelay = 101;
    flowConfig.pktList[1].clientTx = false;
    flowConfig.pktList[2].data.push_back(3);
    flowConfig.pktList[2].pktDelay = 102;
    flowConfig.pktList[2].clientTx = true;

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

    MockAttackPlaylistInstance * pi = new MockAttackPlaylistInstance (*pe, *sm, playlistConfig, false);

    AttackInstance *fi = static_cast<AttackInstance *>(&pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    fi->Start();

    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].data.size(), sm->GetRecvs().begin()->dataLength);

    // fire off first receive to set up wait timer
    sm->GetRecvs().begin()->recvDelegate(&flowConfig.pktList[0].data[0], flowConfig.pktList[0].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size());

    // fire off wait timer
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].data.size(), sm->GetSends().begin()->dataLength);

    // fire off send delegate
    sm->GetSends().begin()->sendDelegate();

    // fire off the second receive
    (++sm->GetRecvs().begin())->recvDelegate(&flowConfig.pktList[2].data[0], flowConfig.pktList[2].data.size());

    // fire off wait timer
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());

    // the flow will be completed, since all Client Tx packet are received.
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(6), tm->GetTimers().size());
    CPPUNIT_ASSERT_EQUAL(size_t(4), tm->GetCancelledTimers().size());

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mFailedAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mSuccessfulAttacks);
}



///////////////////////////////////////////////////////////////////////////////

void TestAttackInstance::testPktLoopClient(void)
{
    uint32_t handle = 1;
    FlowEngine::Config_t config;
    config.playType = FlowConfig::P_ATTACK;
    config.layer = FlowConfig::TCP;
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

    // The loop is from pkt 2 to pkt 0, looping 2 times
    // Client side will have pkt #0 and #2 loop 2 times.
    FlowConfig::LoopInfo firstLoop;
    firstLoop.count = 2;
    firstLoop.begIdx = 0;
    config.loopMap.insert(std::make_pair(2,firstLoop));

    fe->UpdateFlow(handle, MockFlowConfigProxy(config));

    // create a mock stream playlist instance
    PlaylistConfig playlistConfig;
    playlistConfig.type = PlaylistConfig::ATTACK_ONLY;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].minPktDelay = 200;
    playlistConfig.flows[0].serverPort = 123;

    MockAttackPlaylistInstance * pi = new MockAttackPlaylistInstance (*pe, *sm, playlistConfig, true);

    AttackInstance *fi = static_cast<AttackInstance *>(&pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    fi->Start();

    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(config.pktList[1].data.size(), sm->GetRecvs().begin()->dataLength);

    // fire off wait timer
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(config.pktList[0].data.size(), sm->GetSends().begin()->dataLength);

    // fire off send delegate
    sm->GetSends().begin()->sendDelegate();

    //fire off the receive, should expect to receive #1 again
    sm->GetRecvs().begin()->recvDelegate(&config.pktList[1].data[0], config.pktList[1].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size());

    // fire off wait timer again
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(config.pktList[2].data.size(), (++sm->GetSends().begin())->dataLength);

    // fire off send delegate
    (++sm->GetSends().begin())->sendDelegate();

    // fire off wait timer - next pkt should be #0 again
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetSends().size());

    // fire off send delegate
    (++++sm->GetSends().begin())->sendDelegate();

    // fire off wait timer again
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(4), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(config.pktList[0].data.size(), (++++sm->GetSends().begin())->dataLength);


    // fire off send delegate
    (++++++sm->GetSends().begin())->sendDelegate();

    // fire off wait timer again
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(5), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(config.pktList[2].data.size(), (++++sm->GetSends().begin())->dataLength);

    // fire off send delegate
    (++++++++sm->GetSends().begin())->sendDelegate();

    // fire off wait timer again
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(5), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(config.pktList[4].data.size(), (++++sm->GetSends().begin())->dataLength);

    // fire off receive delegate
    (++sm->GetRecvs().begin())->recvDelegate(&config.pktList[1].data[0], config.pktList[1].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetRecvs().size());

    // fire off receive delegate, last pkt #3
    (++++sm->GetRecvs().begin())->recvDelegate(&config.pktList[3].data[0], config.pktList[3].data.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetRecvs().size());

    // fire off wait timer - complete the flow
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(3), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(5), sm->GetSends().size());

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mFailedAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mSuccessfulAttacks);
}

///////////////////////////////////////////////////////////////////////////////

void TestAttackInstance::testUDPInactivityTimer(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.playType = FlowConfig::P_ATTACK;
    flowConfig.layer = FlowConfig::UDP;
    flowConfig.pktList.resize(3);
    flowConfig.pktList[0].data.push_back(1);
    flowConfig.pktList[0].pktDelay = 100;
    flowConfig.pktList[0].clientTx = true;
    flowConfig.pktList[1].data.push_back(2);
    flowConfig.pktList[1].pktDelay = 101;
    flowConfig.pktList[1].clientTx = false;
    flowConfig.pktList[2].data.push_back(3);
    flowConfig.pktList[2].pktDelay = 102;
    flowConfig.pktList[2].clientTx = true;

    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(flowConfig));

    // create a mock attack playlist instance
    PlaylistConfig playlistConfig;
    playlistConfig.type = PlaylistConfig::ATTACK_ONLY;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].minPktDelay = 600;
    playlistConfig.flows[0].socketTimeout = 500; //socket will timeout before the packet delay
    playlistConfig.flows[0].serverPort = 123;

    MockAttackPlaylistInstance * pi = new MockAttackPlaylistInstance (*pe, *sm, playlistConfig, true);

    AttackInstance *fi = static_cast<AttackInstance *>(&pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    fi->Start();

    // inactivity timer correctly set up
    CPPUNIT_ASSERT_EQUAL(size_t(fi->mInactivityTimeout), tm->GetTimers()[fi->mInactivityTimer].msDelay);
    
    // started to receive already
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].data.size(), sm->GetRecvs().begin()->dataLength);

    // fire off wait timer
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].data.size(), sm->GetSends().begin()->dataLength);

    // fire off send delegate
    sm->GetSends().begin()->sendDelegate();

    // fire off wait timer again
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[2].data.size(), sm->GetSends().begin()->dataLength);

    // fire off inactivity timer
    tm->GetTimers()[fi->mInactivityTimer].delegate();

    // should have timeout the flow 
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), tm->GetTimers().size() - tm->GetCancelledTimers().size());

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mFailedAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mSuccessfulAttacks);
}

///////////////////////////////////////////////////////////////////////////////

void TestAttackInstance::testTCPInactivityTimer(void)
{
    uint32_t handle = 1;
    FlowConfig flowConfig;
    flowConfig.playType = FlowConfig::P_ATTACK;
    flowConfig.layer = FlowConfig::TCP;
    flowConfig.pktList.resize(3);
    flowConfig.pktList[0].data.push_back(1);
    flowConfig.pktList[0].pktDelay = 100;
    flowConfig.pktList[0].clientTx = true;
    flowConfig.pktList[1].data.push_back(2);
    flowConfig.pktList[1].pktDelay = 101;
    flowConfig.pktList[1].clientTx = false;
    flowConfig.pktList[2].data.push_back(3);
    flowConfig.pktList[2].pktDelay = 102;
    flowConfig.pktList[2].clientTx = true;

    // create flow
    fe->UpdateFlow(handle, MockFlowConfigProxy(flowConfig));

    // create a mock attack playlist instance
    PlaylistConfig playlistConfig;
    playlistConfig.type = PlaylistConfig::ATTACK_ONLY;

    playlistConfig.flows.resize(1);
    playlistConfig.flows[0].flowHandle = handle;
    playlistConfig.flows[0].startTime = 0;
    playlistConfig.flows[0].dataDelay = 10;
    playlistConfig.flows[0].minPktDelay = 600;
    playlistConfig.flows[0].socketTimeout = 500; //socket will timeout before the packet delay
    playlistConfig.flows[0].serverPort = 123;

    MockAttackPlaylistInstance * pi = new MockAttackPlaylistInstance (*pe, *sm, playlistConfig, true);

    AttackInstance *fi = static_cast<AttackInstance *>(&pi->GetFlow(0));

    fi->SetFd(1); // fake fd

    fi->Start();

    // inactivity timer correctly set up
    CPPUNIT_ASSERT_EQUAL(size_t(fi->mInactivityTimeout), tm->GetTimers()[fi->mInactivityTimer].msDelay);
    
    // started to receive already
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[1].data.size(), sm->GetRecvs().begin()->dataLength);

    // fire off wait timer
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[0].data.size(), sm->GetSends().begin()->dataLength);

    // fire off send delegate
    sm->GetSends().begin()->sendDelegate();

    // fire off wait timer again
    tm->GetTimers()[fi->mTimer].delegate();
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(flowConfig.pktList[2].data.size(), sm->GetSends().begin()->dataLength);

    // fire off inactivity timer
    tm->GetTimers()[fi->mInactivityTimer].delegate();

    // should have timeout the flow 
    CPPUNIT_ASSERT_EQUAL(size_t(1), sm->GetRecvs().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), sm->GetSends().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), tm->GetTimers().size() - tm->GetCancelledTimers().size());

    // should have correct counts for flow status
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mTotalAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pi->mFailedAttacks);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pi->mSuccessfulAttacks);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestAttackInstance);
