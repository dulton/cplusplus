#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <ace/OS.h>
#include <ace/Task.h>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "MockPlaylistEngine.h"
#include "MockReactor.h"
#include "TimerManager.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestTimerManager : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestTimerManager);
    CPPUNIT_TEST(testBasic);
    CPPUNIT_TEST(testMultiple);
    CPPUNIT_TEST(testCancel);
    CPPUNIT_TEST(testGetTimeOfDay);
    CPPUNIT_TEST_SUITE_END();

public:
    TestTimerManager()
    {
    }

    ~TestTimerManager()
    {
    }

    void setUp() 
    { 
    }

    void tearDown() 
    { 
    }
    
protected:
    void testBasic();
    void testMultiple();
    void testCancel();
    void testGetTimeOfDay();

private:
    class MockEngine;

    // fudge factors
    static const int NEG_FF = 1;
    static const int POS_FF = 7;
};

class TestTimerManager::MockEngine
{
public:
    MockEngine()
    {
    }

    TimerManager::timerDelegate_t GetDelegate(int arg)
    {
        return TimerManager::timerDelegate_t(boost::bind(&MockEngine::OnExpire, this, arg));
    }


    void ResetMock()
    {
        mExpires.clear();  
    }

    struct ExpireInfo { int arg; ACE_Time_Value time;};
    const std::vector<ExpireInfo>& GetExpires() { return mExpires; }

private:
    void OnExpire(int arg)
    {
        ExpireInfo info;
        info.arg = arg;
        info.time = ACE_OS::gettimeofday();
        mExpires.push_back(info);
    }


    std::vector<ExpireInfo> mExpires;
};

///////////////////////////////////////////////////////////////////////////////

void TestTimerManager::testBasic()
{
    ACE_Reactor ioReactor;
    {
    MockEngine engine;
    TimerManager timerMgr(&ioReactor);

    TimerManager::handle_t timerId = timerMgr.CreateTimer(engine.GetDelegate(1), 20);
    
    CPPUNIT_ASSERT(timerId != timerMgr.InvalidTimerHandle());

    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetExpires().size());

    // handle will block until it has an event (the timer should be the only thing)
    ACE_Time_Value before = ACE_OS::gettimeofday();
    while (engine.GetExpires().size() < 1)
    {
        ioReactor.handle_events();
    }

    // should have fired
    CPPUNIT_ASSERT_EQUAL(size_t(1), engine.GetExpires().size());
    CPPUNIT_ASSERT_EQUAL(1, engine.GetExpires()[0].arg);
    CPPUNIT_ASSERT((engine.GetExpires()[0].time - before).msec() >= 20-NEG_FF);
    CPPUNIT_ASSERT((engine.GetExpires()[0].time - before).msec() <= 20+POS_FF);

    CPPUNIT_ASSERT(timerMgr.mHandleSet.empty());
    }
}

///////////////////////////////////////////////////////////////////////////////

void TestTimerManager::testMultiple()
{
    ACE_Reactor ioReactor;
    {
    MockEngine engine;
    TimerManager timerMgr(&ioReactor);

    TimerManager::handle_t timerId0 = timerMgr.CreateTimer(engine.GetDelegate(0), 20);
    TimerManager::handle_t timerId1 = timerMgr.CreateTimer(engine.GetDelegate(1), 100);
    TimerManager::handle_t timerId2 = timerMgr.CreateTimer(engine.GetDelegate(2), 50);
    CPPUNIT_ASSERT(timerId0 != timerMgr.InvalidTimerHandle());
    CPPUNIT_ASSERT(timerId1 != timerMgr.InvalidTimerHandle());
    CPPUNIT_ASSERT(timerId2 != timerMgr.InvalidTimerHandle());
    CPPUNIT_ASSERT(timerId0 != timerId1);
    CPPUNIT_ASSERT(timerId1 != timerId2);

    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetExpires().size());

    // handle will block until it has an event (the timer should be the only thing)
    ACE_Time_Value before = ACE_OS::gettimeofday();
    while (engine.GetExpires().size() < 3)
    {
        ioReactor.handle_events();
    }

    CPPUNIT_ASSERT_EQUAL(size_t(3), engine.GetExpires().size());
    CPPUNIT_ASSERT_EQUAL(0, engine.GetExpires()[0].arg);
    CPPUNIT_ASSERT((engine.GetExpires()[0].time - before).msec() >= 20-NEG_FF);
    CPPUNIT_ASSERT((engine.GetExpires()[0].time - before).msec() <= 20+POS_FF);

    CPPUNIT_ASSERT_EQUAL(2, engine.GetExpires()[1].arg);
    CPPUNIT_ASSERT((engine.GetExpires()[1].time - before).msec() >= 50-NEG_FF);
    CPPUNIT_ASSERT((engine.GetExpires()[1].time - before).msec() <= 50+POS_FF);

    CPPUNIT_ASSERT_EQUAL(1, engine.GetExpires()[2].arg);
    CPPUNIT_ASSERT((engine.GetExpires()[2].time - before).msec() >= 100-NEG_FF);
    CPPUNIT_ASSERT((engine.GetExpires()[2].time - before).msec() <= 100+POS_FF);

    CPPUNIT_ASSERT(timerMgr.mHandleSet.empty());
    }
}

///////////////////////////////////////////////////////////////////////////////

void TestTimerManager::testCancel()
{
    ACE_Reactor ioReactor;
    {
    MockEngine engine;
    TimerManager timerMgr(&ioReactor);

    TimerManager::handle_t timerId0 = timerMgr.CreateTimer(engine.GetDelegate(0), 100);
    TimerManager::handle_t timerId1 = timerMgr.CreateTimer(engine.GetDelegate(1), 20);
    TimerManager::handle_t timerId2 = timerMgr.CreateTimer(engine.GetDelegate(2), 50);
    CPPUNIT_ASSERT(timerId0 != timerMgr.InvalidTimerHandle());
    CPPUNIT_ASSERT(timerId1 != timerMgr.InvalidTimerHandle());
    CPPUNIT_ASSERT(timerId2 != timerMgr.InvalidTimerHandle());
    CPPUNIT_ASSERT(timerId0 != timerId1);
    CPPUNIT_ASSERT(timerId1 != timerId2);

    CPPUNIT_ASSERT_EQUAL(size_t(0), engine.GetExpires().size());

    // handle will block until it has an event (the timer should be the only thing)
    ACE_Time_Value before = ACE_OS::gettimeofday();
    while (engine.GetExpires().size() < 1)
    {
        ioReactor.handle_events();
    }

    // first event fired, now cancel the second one
    timerMgr.CancelTimer(timerId2);

    while (engine.GetExpires().size() < 2)
    {
        ioReactor.handle_events();
    }

    CPPUNIT_ASSERT_EQUAL(size_t(2), engine.GetExpires().size());
    CPPUNIT_ASSERT_EQUAL(1, engine.GetExpires()[0].arg);

    CPPUNIT_ASSERT((engine.GetExpires()[0].time - before).msec() >= 20-NEG_FF);
    CPPUNIT_ASSERT((engine.GetExpires()[0].time - before).msec() <= 20+POS_FF);

    CPPUNIT_ASSERT_EQUAL(0, engine.GetExpires()[1].arg);
    CPPUNIT_ASSERT((engine.GetExpires()[1].time - before).msec() >= 100-NEG_FF);
    CPPUNIT_ASSERT((engine.GetExpires()[1].time - before).msec() <= 100+POS_FF);

    CPPUNIT_ASSERT(timerMgr.mHandleSet.empty());
    }
}

///////////////////////////////////////////////////////////////////////////////

void TestTimerManager::testGetTimeOfDay()
{
    ACE_Reactor ioReactor;
    TimerManager timerMgr(&ioReactor);

    uint64_t now1 = timerMgr.GetTimeOfDayMsec();
    uint64_t now2 = timerMgr.GetTimeOfDayMsec();

    CPPUNIT_ASSERT(now2 >= now1);
    CPPUNIT_ASSERT((now2 - now1) < 5);

    ACE_OS::sleep(1);
    uint64_t now3 = timerMgr.GetTimeOfDayMsec();

    CPPUNIT_ASSERT(now3 - now2 > 950);
    CPPUNIT_ASSERT(now3 - now2 < 1050);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestTimerManager);
