#include <algorithm>

#include <ace/Reactor.h>
#include <ace/Time_Value.h>

#include <boost/foreach.hpp>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "AsyncMessageInterface.tcc"
#include "utils/CountingPredicate.tcc"

USING_L4L7_APP_NS;

///////////////////////////////////////////////////////////////////////////////

class TestAsyncMessageInterface : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestAsyncMessageInterface);
    CPPUNIT_TEST(testSingleMessageNotification);
    CPPUNIT_TEST(testMultiMessageNotification);
    CPPUNIT_TEST(testLoopingPredicate);
    CPPUNIT_TEST(testLoopingPredicateRenotify);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() { }
    void tearDown() { }
    
protected:
    void testSingleMessageNotification();
    void testMultiMessageNotification();
    void testLoopingPredicate();
    void testLoopingPredicateRenotify();

private:
    class MockReactor;
    class MockDelegate;

    typedef AsyncMessageInterface<int> Ami_t;

    typedef Ami_t::messagePtr_t msgPtr_t;
};

///////////////////////////////////////////////////////////////////////////////

class TestAsyncMessageInterface::MockDelegate 
{
public:
    MockDelegate() : 
        mLastMessage(0),
        mMessageCount(0),
        mMessageSum(0)
    {
    }

    Ami_t::messageDelegate_t delegate() { return fastdelegate::MakeDelegate(this, &TestAsyncMessageInterface::MockDelegate::HandleMessage); }

    void HandleMessage(const int& msg)
    {
        mLastMessage = msg;
        ++mMessageCount;
        mMessageSum += msg;
    }

    // mock getters
    int GetLastMessage() { return mLastMessage; }
    size_t GetMessageCount() { return mMessageCount; }
    int GetMessageSum() { return mMessageSum; }

private:
    int mLastMessage;
    size_t mMessageCount;
    int mMessageSum;
};

///////////////////////////////////////////////////////////////////////////////

class TestAsyncMessageInterface::MockReactor : public ACE_Reactor
{
public:
    MockReactor() :
        mLastEventHandler(0),
        mLastMask(0),
        mDoNotify(false)
    {
    }

    int notify(ACE_Event_Handler *event_handler, ACE_Reactor_Mask mask, ACE_Time_Value *timeout)
    {
        if (event_handler->reactor() == 0)
        {
            event_handler->reactor(this);
        }

        mLastEventHandler = event_handler;
        mLastMask = mask;
        mDoNotify = true;
        return 0;
    }

    int DoNotify()
    {
        if (mLastEventHandler && mDoNotify)
        {
            mDoNotify = false;
            return mLastEventHandler->handle_input(ACE_INVALID_HANDLE);
        }

        return 0;
    }

    // mock getters
    ACE_Event_Handler * GetLastEventHandler() { return mLastEventHandler; }
    ACE_Reactor_Mask GetLastMask() { return mLastMask; }

    ACE_Event_Handler * mLastEventHandler;
    ACE_Reactor_Mask mLastMask;
    bool mDoNotify;
};

///////////////////////////////////////////////////////////////////////////////

void TestAsyncMessageInterface::testSingleMessageNotification()
{
    MockReactor reactor;
    Ami_t ami(&reactor);
    MockDelegate delegate;
    ami.SetMessageDelegate(delegate.delegate());

    CPPUNIT_ASSERT_EQUAL(true, ami.IsEmpty());
    
    msgPtr_t msg(ami.Allocate());
    *msg = 1;
    ami.Send(msg);

    CPPUNIT_ASSERT_EQUAL(false, ami.IsEmpty());
    CPPUNIT_ASSERT_EQUAL((ACE_Reactor_Mask)ACE_Event_Handler::READ_MASK, reactor.GetLastMask());
    CPPUNIT_ASSERT_EQUAL((ACE_Event_Handler*)&ami, reactor.GetLastEventHandler());

    // force reactor to process notification
    reactor.DoNotify();

    CPPUNIT_ASSERT_EQUAL(size_t(1), delegate.GetMessageCount());
    CPPUNIT_ASSERT_EQUAL(1, delegate.GetLastMessage());
    CPPUNIT_ASSERT_EQUAL(true, ami.IsEmpty());
}

///////////////////////////////////////////////////////////////////////////////

void TestAsyncMessageInterface::testMultiMessageNotification()
{
    MockReactor reactor;
    Ami_t ami(&reactor);
    MockDelegate delegate;
    ami.SetMessageDelegate(delegate.delegate());

    CPPUNIT_ASSERT_EQUAL(true, ami.IsEmpty());
    
    msgPtr_t msg(ami.Allocate());
    *msg = 1;
    ami.Send(msg);

    CPPUNIT_ASSERT_EQUAL(false, ami.IsEmpty());
    CPPUNIT_ASSERT_EQUAL((ACE_Reactor_Mask)ACE_Event_Handler::READ_MASK, reactor.GetLastMask());
    CPPUNIT_ASSERT_EQUAL((ACE_Event_Handler*)&ami, reactor.GetLastEventHandler());

    msgPtr_t msg2(ami.Allocate());
    *msg2 = 2;
    ami.Send(msg2);

    msgPtr_t msg3(ami.Allocate());
    *msg3 = 3;
    ami.Send(msg3);


    // force reactor to process notification
    reactor.DoNotify();

    CPPUNIT_ASSERT_EQUAL(size_t(3), delegate.GetMessageCount());
    CPPUNIT_ASSERT_EQUAL(3, delegate.GetLastMessage());
    CPPUNIT_ASSERT_EQUAL(6, delegate.GetMessageSum());
    CPPUNIT_ASSERT_EQUAL(true, ami.IsEmpty());
}

///////////////////////////////////////////////////////////////////////////////

void TestAsyncMessageInterface::testLoopingPredicate()
{
    typedef AsyncMessageInterface<int, l4l7Utils::CountingPredicate<size_t, 5> > Ami5_t;

    MockReactor reactor;
    Ami5_t ami(&reactor);
    MockDelegate delegate;
    ami.SetMessageDelegate(delegate.delegate());
 
    std::vector<msgPtr_t> msgs;
    msgs.resize(6);

    BOOST_FOREACH (msgPtr_t& msg, msgs)
    {
        msg = ami.Allocate();
        *msg = 1;
        ami.Send(msg);
    } 

    // force reactor to process notification
    reactor.DoNotify();

    // should only process 5
    CPPUNIT_ASSERT_EQUAL(size_t(5), delegate.GetMessageCount());
    CPPUNIT_ASSERT_EQUAL(1, delegate.GetLastMessage());
    CPPUNIT_ASSERT_EQUAL(5, delegate.GetMessageSum());
    CPPUNIT_ASSERT_EQUAL(false, ami.IsEmpty());
}

///////////////////////////////////////////////////////////////////////////////

void TestAsyncMessageInterface::testLoopingPredicateRenotify()
{
    typedef AsyncMessageInterface<int, l4l7Utils::CountingPredicate<size_t, 1> > Ami1_t;

    MockReactor reactor;
    Ami1_t ami(&reactor);
    MockDelegate delegate;
    ami.SetMessageDelegate(delegate.delegate());
 
    msgPtr_t msg(ami.Allocate());
    *msg = 1;
    ami.Send(msg);

    msgPtr_t msg2(ami.Allocate());
    *msg2 = 2;
    ami.Send(msg2);

    // force reactor to process notification
    reactor.DoNotify();

    // should only process 1
    CPPUNIT_ASSERT_EQUAL(size_t(1), delegate.GetMessageCount());
    CPPUNIT_ASSERT_EQUAL(1, delegate.GetLastMessage());
    CPPUNIT_ASSERT_EQUAL(1, delegate.GetMessageSum());
    CPPUNIT_ASSERT_EQUAL(false, ami.IsEmpty());

    // should be ready to notify again
    reactor.DoNotify();
    CPPUNIT_ASSERT_EQUAL(size_t(2), delegate.GetMessageCount());
    CPPUNIT_ASSERT_EQUAL(2, delegate.GetLastMessage());
    CPPUNIT_ASSERT_EQUAL(3, delegate.GetMessageSum());
    CPPUNIT_ASSERT_EQUAL(true, ami.IsEmpty());
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestAsyncMessageInterface);
