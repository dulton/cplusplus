#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <boost/foreach.hpp>

#include <statsframework/ExtremeSqlMsgAdapter.h>

#include "TestableDpgClientRawStats.h"
#include "TestableDpgServerRawStats.h"

#include "McoDriver.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestMcoDriver : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestMcoDriver);
    CPPUNIT_TEST(testClientCreateDelete);
    CPPUNIT_TEST(testServerCreateDelete);
    CPPUNIT_TEST(testClientUpdate);
    CPPUNIT_TEST(testServerUpdate);
    CPPUNIT_TEST_SUITE_END();

public:
    TestMcoDriver()
    {
        McoDriver::StaticInitialize();
    }

    ~TestMcoDriver()
    {
    }

    void setUp() 
    { 
        mMcoDriver = new McoDriver(0); 
        // there's only one 'port' in the unit test build, so the 
        // database is shared. tests try to clean up, but failures could cascade
    }

    void tearDown() 
    { 
        delete mMcoDriver;
    }
    
protected:
    void testClientCreateDelete();
    void testServerCreateDelete();
    void testClientUpdate();
    void testServerUpdate();

private:
    McoDriver * mMcoDriver;
};

///////////////////////////////////////////////////////////////////////////////

void TestMcoDriver::testClientCreateDelete()
{
    uint32_t clientHandle = 1;
    mMcoDriver->InsertDpgClientResult(clientHandle);
    
    ExtremeSqlMsgAdapter sqlDb(mMcoDriver->mDBFactory);
    std::vector<std::string> sql;
    sql.push_back("select * from DpgClientResults");  

    std::vector<Stats_1_port_server::commandResultFaster_t> results;

    sqlDb.processPhxRpcSql(sql, &results);

    CPPUNIT_ASSERT_EQUAL(size_t(1), results.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), results[0].rows.size());

    CPPUNIT_ASSERT_EQUAL(size_t(37), results[0].rows[0].int64Values.size());

    CPPUNIT_ASSERT_EQUAL(int64_t(clientHandle), results[0].rows[0].int64Values[1]);

    int index = 0;
    BOOST_FOREACH(int64_t value, results[0].rows[0].int64Values)
    {
        if (index > 1 && index < 36) // exclude BlockIndex,Handle,LastModified  
        {
            CPPUNIT_ASSERT_EQUAL(int64_t(0), value);
        }
        ++index;
    }

    mMcoDriver->DeleteDpgClientResult(clientHandle);

    // check for cleaned database
    sqlDb.processPhxRpcSql(sql, &results);
    CPPUNIT_ASSERT_EQUAL(size_t(1), results.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), results[0].rows.size());
}

///////////////////////////////////////////////////////////////////////////////

void TestMcoDriver::testServerCreateDelete()
{
    uint32_t serverHandle = 1;
    mMcoDriver->InsertDpgServerResult(serverHandle);
    
    ExtremeSqlMsgAdapter sqlDb(mMcoDriver->mDBFactory);
    std::vector<std::string> sql;
    sql.push_back("select * from DpgServerResults");  

    std::vector<Stats_1_port_server::commandResultFaster_t> results;

    sqlDb.processPhxRpcSql(sql, &results);

    CPPUNIT_ASSERT_EQUAL(size_t(1), results.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), results[0].rows.size());

    CPPUNIT_ASSERT_EQUAL(size_t(20), results[0].rows[0].int64Values.size());

    CPPUNIT_ASSERT_EQUAL(int64_t(serverHandle), results[0].rows[0].int64Values[1]);

    int index = 0;
    BOOST_FOREACH(int64_t value, results[0].rows[0].int64Values)
    {
        if (index > 1 && index < 19) // exclude BlockIndex,Handle,LastModified  
        {
            CPPUNIT_ASSERT_EQUAL(int64_t(0), value);
        }
        ++index;
    }

    mMcoDriver->DeleteDpgServerResult(serverHandle);
   
    // check for cleaned database
    sqlDb.processPhxRpcSql(sql, &results);
    CPPUNIT_ASSERT_EQUAL(size_t(1), results.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), results[0].rows.size());
}

///////////////////////////////////////////////////////////////////////////////

void TestMcoDriver::testClientUpdate()
{
    uint32_t clientHandle = 2;
    mMcoDriver->InsertDpgClientResult(clientHandle);

    TestableDpgClientRawStats stats(clientHandle);
    stats.Start();
    stats.IncrStats();
    stats.SetNow(ACE_Time_Value(1,0)); // Now it's 1 second later
    stats.Sync(); // calculate rates (time is 1, so counts should == rates)

    mMcoDriver->UpdateDpgClientResult(stats);
    
    ExtremeSqlMsgAdapter sqlDb(mMcoDriver->mDBFactory);
    std::vector<std::string> sql;
    sql.push_back("select * from DpgClientResults");  

    std::vector<Stats_1_port_server::commandResultFaster_t> results;

    sqlDb.processPhxRpcSql(sql, &results);

    CPPUNIT_ASSERT_EQUAL(size_t(1), results.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), results[0].rows.size());

    CPPUNIT_ASSERT_EQUAL(size_t(37), results[0].rows[0].int64Values.size());

    CPPUNIT_ASSERT_EQUAL(int64_t(clientHandle), results[0].rows[0].int64Values[1]);

    int index = 2;

    // elapsed, intended
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.elapsedSeconds), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.intendedLoad), results[0].rows[0].int64Values[index++]);

    // connections
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.activeConnections), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.attemptedConnections), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.successfulConnections), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.unsuccessfulConnections), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.abortedConnections), results[0].rows[0].int64Values[index++]);

    // connections rates (== counts)
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.attemptedConnections), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.successfulConnections), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.unsuccessfulConnections), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.abortedConnections), results[0].rows[0].int64Values[index++]);

    // playlists
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.activePlaylists), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.attemptedPlaylists), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.successfulPlaylists), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.unsuccessfulPlaylists), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.abortedPlaylists), results[0].rows[0].int64Values[index++]);

    // playlist rates
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.attemptedPlaylists), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.successfulPlaylists), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.unsuccessfulPlaylists), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.abortedPlaylists), results[0].rows[0].int64Values[index++]);

    // goodput
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputRxBytes), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputTxBytes), results[0].rows[0].int64Values[index++]);

    // goodput rates (== counts * 8 bits/byte)
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputRxBytes*8), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputTxBytes*8), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputRxBytes*8), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputTxBytes*8), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputRxBytes*8), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputTxBytes*8), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputRxBytes*8), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputTxBytes*8), results[0].rows[0].int64Values[index++]);

    // Tx Attack
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.txAttack), results[0].rows[0].int64Values[index++]);

    // Durations
    CPPUNIT_ASSERT_EQUAL(int64_t(0), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(0), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(0), results[0].rows[0].int64Values[index++]);

    // LastModified
    CPPUNIT_ASSERT(results[0].rows[0].int64Values[index++] != 0);
    CPPUNIT_ASSERT_EQUAL(int(37), index);
}

///////////////////////////////////////////////////////////////////////////////

void TestMcoDriver::testServerUpdate()
{
    uint32_t serverHandle = 2;
    mMcoDriver->InsertDpgServerResult(serverHandle);

    TestableDpgServerRawStats stats(serverHandle);
    stats.Start();
    stats.IncrStats();
    stats.SetNow(ACE_Time_Value(1,0)); // Now it's 1 second later
    stats.Sync(); // calculate rates (time is 1, so counts should == rates)

    mMcoDriver->UpdateDpgServerResult(stats);
    
    ExtremeSqlMsgAdapter sqlDb(mMcoDriver->mDBFactory);
    std::vector<std::string> sql;
    sql.push_back("select * from DpgServerResults");  

    std::vector<Stats_1_port_server::commandResultFaster_t> results;

    sqlDb.processPhxRpcSql(sql, &results);

    CPPUNIT_ASSERT_EQUAL(size_t(1), results.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), results[0].rows.size());

    CPPUNIT_ASSERT_EQUAL(size_t(20), results[0].rows[0].int64Values.size());

    CPPUNIT_ASSERT_EQUAL(int64_t(serverHandle), results[0].rows[0].int64Values[1]);

    int index = 2;

    // elapsed
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.elapsedSeconds), results[0].rows[0].int64Values[index++]);

    // connections
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.activeConnections), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.totalConnections), results[0].rows[0].int64Values[index++]);

    // connections rates (== counts)
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.totalConnections), results[0].rows[0].int64Values[index++]);

    // playlists
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.activePlaylists), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.totalPlaylists), results[0].rows[0].int64Values[index++]);

    // playlist rates (== counts)
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.totalPlaylists), results[0].rows[0].int64Values[index++]);

    // goodput
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputRxBytes), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputTxBytes), results[0].rows[0].int64Values[index++]);

    // goodput rates (== counts * 8 bits/byte)
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputRxBytes*8), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputTxBytes*8), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputRxBytes*8), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputTxBytes*8), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputRxBytes*8), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputTxBytes*8), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputRxBytes*8), results[0].rows[0].int64Values[index++]);
    CPPUNIT_ASSERT_EQUAL(int64_t(stats.goodputTxBytes*8), results[0].rows[0].int64Values[index++]);

    // LastModified
    CPPUNIT_ASSERT(results[0].rows[0].int64Values[index++] != 0);
    CPPUNIT_ASSERT_EQUAL(int(20), index);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestMcoDriver);
