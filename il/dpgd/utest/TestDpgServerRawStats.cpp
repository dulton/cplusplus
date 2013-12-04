#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "TestableDpgServerRawStats.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestDpgServerRawStats : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestDpgServerRawStats);
    CPPUNIT_TEST(testCreate);
    CPPUNIT_TEST(testSync);
    CPPUNIT_TEST(testSyncZeroRates);
    CPPUNIT_TEST(testReset);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() 
    { 
        mHandle = 1;
        mStats = mTestableStats = new TestableDpgServerRawStats(mHandle);
    }

    void tearDown() 
    { 
        delete mStats;
        mStats = 0;
        mHandle = 0;
    }
    
protected:
    void testCreate();
    void testSync();
    void testSyncZeroRates();
    void testReset();

    void assertAllStatsZero();

private:
    uint32_t mHandle;
    DpgServerRawStats * mStats;
    TestableDpgServerRawStats * mTestableStats;
};

///////////////////////////////////////////////////////////////////////////////

void TestDpgServerRawStats::testCreate()
{
    CPPUNIT_ASSERT_EQUAL(mHandle, mStats->Handle());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), mStats->activeConnections);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), mStats->activePlaylists);
    assertAllStatsZero();
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgServerRawStats::testSync()
{
    mStats->Start();
    mTestableStats->IncrStats();
    mStats->Sync(); // take a snapshot

    // elapsed time here is 0, so rates should be zero
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->totalCps);
    
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->totalPps);

    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputRxBps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputTxBps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputAvgRxBps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputAvgTxBps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputMaxRxBps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputMaxTxBps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputMinRxBps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputMinTxBps);

    mTestableStats->SetNow(ACE_Time_Value(1,0)); // Now it's 1 second later
    mTestableStats->IncrStats();
    mStats->Sync(); // calc rates

    // elapsed time here is 1 and we've incremented twice, 
    // so rates should be 1/2 counts
    CPPUNIT_ASSERT_EQUAL(mStats->totalConnections/2, mStats->totalCps);
    
    CPPUNIT_ASSERT_EQUAL(mStats->totalPlaylists/2, mStats->totalPps);
    
    CPPUNIT_ASSERT_EQUAL(mStats->goodputRxBytes*8/2, mStats->goodputRxBps);
    CPPUNIT_ASSERT_EQUAL(mStats->goodputTxBytes*8/2, mStats->goodputTxBps);

    // avg should be same as counts
    CPPUNIT_ASSERT_EQUAL(mStats->goodputRxBytes*8, mStats->goodputAvgRxBps);
    CPPUNIT_ASSERT_EQUAL(mStats->goodputTxBytes*8, mStats->goodputAvgTxBps);

    // min/max all the same
    CPPUNIT_ASSERT_EQUAL(mStats->goodputRxBytes*8/2, mStats->goodputMaxRxBps);
    CPPUNIT_ASSERT_EQUAL(mStats->goodputTxBytes*8/2, mStats->goodputMaxTxBps);
    CPPUNIT_ASSERT_EQUAL(mStats->goodputRxBytes*8/2, mStats->goodputMinRxBps);
    CPPUNIT_ASSERT_EQUAL(mStats->goodputTxBytes*8/2, mStats->goodputMinTxBps);
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgServerRawStats::testSyncZeroRates()
{
    mStats->Start();
    mTestableStats->IncrStats();
    mTestableStats->SetNow(ACE_Time_Value(1,0)); // Now it's 1 second later
    mStats->Sync(); // take a snapshot

    // elapsed time here is 1, and we incremented once, 
    // so rates should == counts 
    CPPUNIT_ASSERT_EQUAL(mStats->goodputRxBytes*8, mStats->goodputAvgRxBps);
    CPPUNIT_ASSERT_EQUAL(mStats->goodputTxBytes*8, mStats->goodputAvgTxBps);
    CPPUNIT_ASSERT_EQUAL(mStats->goodputRxBytes*8, mStats->goodputMaxRxBps);
    CPPUNIT_ASSERT_EQUAL(mStats->goodputTxBytes*8, mStats->goodputMaxTxBps);
    CPPUNIT_ASSERT_EQUAL(mStats->goodputRxBytes*8, mStats->goodputMinRxBps);
    CPPUNIT_ASSERT_EQUAL(mStats->goodputTxBytes*8, mStats->goodputMinTxBps);

    mTestableStats->IncrStats();
    mTestableStats->SetNow(ACE_Time_Value(2,0)); // Now it's 1 second later
    mStats->Sync(true); // take a rate-clearing snapshot
    
    // rates should be zeroed
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->totalCps);
    
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->totalPps);

    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputRxBps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputTxBps);

    // elapsed time here is 1, and we incremented twice, 
    // so rates should == counts/2 
    CPPUNIT_ASSERT_EQUAL(mStats->goodputRxBytes*8/2, mStats->goodputAvgRxBps);
    CPPUNIT_ASSERT_EQUAL(mStats->goodputTxBytes*8/2, mStats->goodputAvgTxBps);
    CPPUNIT_ASSERT_EQUAL(mStats->goodputRxBytes*8/2, mStats->goodputMaxRxBps);
    CPPUNIT_ASSERT_EQUAL(mStats->goodputTxBytes*8/2, mStats->goodputMaxTxBps);
    CPPUNIT_ASSERT_EQUAL(mStats->goodputRxBytes*8/2, mStats->goodputMinRxBps);
    CPPUNIT_ASSERT_EQUAL(mStats->goodputTxBytes*8/2, mStats->goodputMinTxBps);
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgServerRawStats::testReset()
{
    mTestableStats->IncrStats();
    mStats->Reset();
    assertAllStatsZero();

    // these should not be cleared
    CPPUNIT_ASSERT_EQUAL(uint32_t(4), mStats->activeConnections);
    CPPUNIT_ASSERT_EQUAL(uint32_t(128), mStats->activePlaylists);
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgServerRawStats::assertAllStatsZero()
{
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->elapsedSeconds);

    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->totalConnections);

    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->totalCps);

    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->totalPlaylists);

    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->totalPps);

    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputRxBytes);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputTxBytes);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputRxBps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputTxBps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputAvgRxBps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputAvgTxBps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputMaxRxBps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputMaxTxBps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputMinRxBps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->goodputMinTxBps);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestDpgServerRawStats);
