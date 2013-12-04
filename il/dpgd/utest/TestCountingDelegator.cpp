#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "CountingDelegator.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestCountingDelegator : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestCountingDelegator);
    CPPUNIT_TEST(testBasic);
    CPPUNIT_TEST(testAsync);
    CPPUNIT_TEST(testRecursive);
    CPPUNIT_TEST(testReset);
    CPPUNIT_TEST_SUITE_END();

public:
    TestCountingDelegator()
    {
    }

    ~TestCountingDelegator()
    {
    }

    void setUp() 
    { 
        mDelegator = new delegator_t;
        mCount = 0;
    }

    void tearDown() 
    { 
        delete mDelegator;
    }
    
protected:
    void testBasic();
    void testAsync();
    void testRecursive();
    void testReset();

private:
    void incrementCount(int incr) 
    { 
        mCount += incr; 
    }

    void incrementAndUpdateCount(int incr);

    typedef boost::function0<void> delegate_t;
    typedef CountingDelegator<int, delegate_t> delegator_t;
    delegator_t * mDelegator;
    int mCount;
};

///////////////////////////////////////////////////////////////////////////////

void TestCountingDelegator::testBasic()
{
    mDelegator->Register(1, boost::bind(&TestCountingDelegator::incrementCount, this, 1));

    // registering doesn't call delegate
    CPPUNIT_ASSERT_EQUAL(int(0), mCount);

    mDelegator->UpdateCount(0);
    // zero count does nothing
    CPPUNIT_ASSERT_EQUAL(int(0), mCount);

    mDelegator->UpdateCount(1);
    // exact count works
    CPPUNIT_ASSERT_EQUAL(int(1), mCount);

    mDelegator->UpdateCount(1);
    // another update doesn't delegate again
    CPPUNIT_ASSERT_EQUAL(int(1), mCount);

    mCount = 0;
    mDelegator->Register(10, boost::bind(&TestCountingDelegator::incrementCount, this, 100));
    
    mDelegator->UpdateCount(8);
    // one less byte doesn't delegate (also note it uses the extra count from above -- by design)
    CPPUNIT_ASSERT_EQUAL(int(0), mCount);

    mDelegator->UpdateCount(2);
    // one extra byte does delegate
    CPPUNIT_ASSERT_EQUAL(int(100), mCount);

    // double check nothing waiting
    CPPUNIT_ASSERT(mDelegator->Empty());

    // double check internal count
    CPPUNIT_ASSERT_EQUAL((int)12, mDelegator->mCount);
}

///////////////////////////////////////////////////////////////////////////////

void TestCountingDelegator::testAsync()
{
    mDelegator->Register(1, boost::bind(&TestCountingDelegator::incrementCount, this, 1));
    mDelegator->Register(2, boost::bind(&TestCountingDelegator::incrementCount, this, 1));
    mDelegator->Register(3, boost::bind(&TestCountingDelegator::incrementCount, this, 1));


    CPPUNIT_ASSERT_EQUAL(int(0), mCount);

    mDelegator->UpdateCount(1);
    CPPUNIT_ASSERT_EQUAL(int(1), mCount);

    mDelegator->UpdateCount(1);
    CPPUNIT_ASSERT_EQUAL(int(1), mCount);

    mDelegator->UpdateCount(1);
    CPPUNIT_ASSERT_EQUAL(int(2), mCount);

    mDelegator->UpdateCount(2);
    CPPUNIT_ASSERT_EQUAL(int(2), mCount);

    mDelegator->UpdateCount(1);
    CPPUNIT_ASSERT_EQUAL(int(3), mCount);

    // double check nothing waiting
    CPPUNIT_ASSERT(mDelegator->Empty());

    // double check internal count
    CPPUNIT_ASSERT_EQUAL((int)6, mDelegator->mCount);
}

///////////////////////////////////////////////////////////////////////////////

void TestCountingDelegator::incrementAndUpdateCount(int incr)
{
    incrementCount(incr);
    mDelegator->UpdateCount(incr);
}

///////////////////////////////////////////////////////////////////////////////

void TestCountingDelegator::testRecursive()
{
    mDelegator->Register(1, boost::bind(&TestCountingDelegator::incrementAndUpdateCount, this, 1));
    mDelegator->Register(1, boost::bind(&TestCountingDelegator::incrementAndUpdateCount, this, 1));
    mDelegator->Register(4, boost::bind(&TestCountingDelegator::incrementAndUpdateCount, this, 1));


    CPPUNIT_ASSERT_EQUAL(int(0), mCount);

    mDelegator->UpdateCount(1);
    CPPUNIT_ASSERT_EQUAL(int(2), mCount); // at 1, calls back, adds 1, at 2 calls back, adds 1
    CPPUNIT_ASSERT_EQUAL(int(3), mDelegator->mCount); 

    mDelegator->UpdateCount(1);
    // no callback
    CPPUNIT_ASSERT_EQUAL(int(2), mCount);
    CPPUNIT_ASSERT_EQUAL(int(4), mDelegator->mCount); 

    mDelegator->UpdateCount(1);
    // no callback
    CPPUNIT_ASSERT_EQUAL(int(2), mCount);
    CPPUNIT_ASSERT_EQUAL(int(5), mDelegator->mCount); 

    mDelegator->UpdateCount(1);
    // calls back, adds 1, done
    CPPUNIT_ASSERT_EQUAL(int(3), mCount);
    CPPUNIT_ASSERT_EQUAL(int(7), mDelegator->mCount); 

    // double check nothing waiting
    CPPUNIT_ASSERT(mDelegator->Empty());
}

///////////////////////////////////////////////////////////////////////////////

void TestCountingDelegator::testReset()
{
    mDelegator->Register(1, boost::bind(&TestCountingDelegator::incrementCount, this, 1));
    mDelegator->Register(2, boost::bind(&TestCountingDelegator::incrementCount, this, 1));
    mDelegator->Register(3, boost::bind(&TestCountingDelegator::incrementCount, this, 1));

    mDelegator->UpdateCount(1);
    CPPUNIT_ASSERT_EQUAL(int(1), mCount);

    mDelegator->UpdateCount(1);
    mDelegator->Reset();

    mDelegator->UpdateCount(1);
    // doesn't callback
    CPPUNIT_ASSERT_EQUAL(int(1), mCount);

    mDelegator->UpdateCount(1000);
    // doesn't ever callback
    CPPUNIT_ASSERT_EQUAL(int(1), mCount);

    // new registration, starts from scratch
    mCount = 0;
    mDelegator->Register(1, boost::bind(&TestCountingDelegator::incrementCount, this, 1));
    
    mDelegator->UpdateCount(1);
    CPPUNIT_ASSERT_EQUAL(int(1), mCount);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestCountingDelegator);
