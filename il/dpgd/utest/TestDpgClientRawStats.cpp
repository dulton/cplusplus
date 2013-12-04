#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "TestableDpgClientRawStats.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestDpgClientRawStats : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestDpgClientRawStats);
    CPPUNIT_TEST(testCreate);
    CPPUNIT_TEST(testSync);
    CPPUNIT_TEST(testSyncZeroRates);
    CPPUNIT_TEST(testReset);
    CPPUNIT_TEST(testCompletePlaylist);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() 
    { 
        mHandle = 1;
        mStats = mTestableStats = new TestableDpgClientRawStats(mHandle);
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
    void testCompletePlaylist();

    void assertAllStatsZero();

private:
    uint32_t mHandle;
    DpgClientRawStats * mStats;
    TestableDpgClientRawStats * mTestableStats;
};

///////////////////////////////////////////////////////////////////////////////

void TestDpgClientRawStats::testCreate()
{
    CPPUNIT_ASSERT_EQUAL(mHandle, mStats->Handle());
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), mStats->intendedLoad);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), mStats->activeConnections);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), mStats->activePlaylists);
    assertAllStatsZero();
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgClientRawStats::testSync()
{
    mStats->Start();
    mTestableStats->IncrStats();
    mStats->Sync(); // take a snapshot

    // elapsed time here is 0, so rates should be zero
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->attemptedCps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->successfulCps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->unsuccessfulCps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->abortedCps);
    
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->attemptedPps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->successfulPps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->unsuccessfulPps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->abortedPps);

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
    CPPUNIT_ASSERT_EQUAL(mStats->attemptedConnections/2, mStats->attemptedCps);
    CPPUNIT_ASSERT_EQUAL(mStats->successfulConnections/2, mStats->successfulCps);
    CPPUNIT_ASSERT_EQUAL(mStats->unsuccessfulConnections/2, mStats->unsuccessfulCps);
    CPPUNIT_ASSERT_EQUAL(mStats->abortedConnections/2, mStats->abortedCps);
    
    CPPUNIT_ASSERT_EQUAL(mStats->attemptedPlaylists/2, mStats->attemptedPps);
    CPPUNIT_ASSERT_EQUAL(mStats->successfulPlaylists/2, mStats->successfulPps);
    CPPUNIT_ASSERT_EQUAL(mStats->unsuccessfulPlaylists/2, mStats->unsuccessfulPps);
    CPPUNIT_ASSERT_EQUAL(mStats->abortedPlaylists/2, mStats->abortedPps);
    
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

void TestDpgClientRawStats::testSyncZeroRates()
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
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->attemptedCps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->successfulCps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->unsuccessfulCps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->abortedCps);
    
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->attemptedPps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->successfulPps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->unsuccessfulPps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->abortedPps);

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

void TestDpgClientRawStats::testReset()
{
    mTestableStats->IncrStats();
    mStats->Reset();
    assertAllStatsZero();

    // these should not be cleared
    CPPUNIT_ASSERT_EQUAL(uint32_t(2), mStats->intendedLoad);
    CPPUNIT_ASSERT_EQUAL(uint32_t(4), mStats->activeConnections);
    CPPUNIT_ASSERT_EQUAL(uint32_t(128), mStats->activePlaylists);
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgClientRawStats::testCompletePlaylist()
{
    mStats->Start();
    mStats->CompletePlaylist(1000, true, false); // successful
    mStats->Sync(); // take a snapshot

    CPPUNIT_ASSERT_EQUAL(uint64_t(1), mStats->successfulPlaylists);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->unsuccessfulPlaylists);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->abortedPlaylists);
    CPPUNIT_ASSERT_EQUAL(uint32_t(1000), mStats->playlistMinDuration);
    CPPUNIT_ASSERT_EQUAL(uint32_t(1000), mStats->playlistMaxDuration);
    CPPUNIT_ASSERT_EQUAL(uint32_t(1000), mStats->playlistAvgDuration);

    mStats->CompletePlaylist(2000, false, true); // aborted 
    mStats->Sync(); // take a snapshot

    CPPUNIT_ASSERT_EQUAL(uint64_t(1), mStats->successfulPlaylists);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->unsuccessfulPlaylists);
    CPPUNIT_ASSERT_EQUAL(uint64_t(1), mStats->abortedPlaylists);
    CPPUNIT_ASSERT_EQUAL(uint32_t(1000), mStats->playlistMinDuration);
    CPPUNIT_ASSERT_EQUAL(uint32_t(2000), mStats->playlistMaxDuration);
    CPPUNIT_ASSERT_EQUAL(uint32_t(1500), mStats->playlistAvgDuration);

    mStats->CompletePlaylist(300, false, false); // unsuccessful
    mStats->Sync(); // take a snapshot

    CPPUNIT_ASSERT_EQUAL(uint64_t(1), mStats->successfulPlaylists);
    CPPUNIT_ASSERT_EQUAL(uint64_t(1), mStats->unsuccessfulPlaylists);
    CPPUNIT_ASSERT_EQUAL(uint64_t(1), mStats->abortedPlaylists);
    CPPUNIT_ASSERT_EQUAL(uint32_t(300), mStats->playlistMinDuration);
    CPPUNIT_ASSERT_EQUAL(uint32_t(2000), mStats->playlistMaxDuration);
    CPPUNIT_ASSERT_EQUAL(uint32_t(1100), mStats->playlistAvgDuration);
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgClientRawStats::assertAllStatsZero()
{
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->elapsedSeconds);

    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->attemptedConnections);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->successfulConnections);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->unsuccessfulConnections);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->abortedConnections);

    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->attemptedCps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->successfulCps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->unsuccessfulCps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->abortedCps);

    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->attemptedPlaylists);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->successfulPlaylists);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->unsuccessfulPlaylists);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->abortedPlaylists);

    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->attemptedPps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->successfulPps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->unsuccessfulPps);
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), mStats->abortedPps);

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

    CPPUNIT_ASSERT_EQUAL(uint32_t(0), mStats->playlistAvgDuration);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), mStats->playlistMaxDuration);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), mStats->playlistMinDuration);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestDpgClientRawStats);
