#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <boost/bind.hpp>

#include "DpgStreamConnectionHandler.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestDpgStreamConnectionHandler : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestDpgStreamConnectionHandler);
    CPPUNIT_TEST(testSetSerial);
    CPPUNIT_TEST(testTxDelegate);
    CPPUNIT_TEST_SUITE_END();

public:
    TestDpgStreamConnectionHandler()
    {
    }

    ~TestDpgStreamConnectionHandler()
    {
    }

    void setUp() 
    { 
        mCount = 0;
    }

    void tearDown() 
    { 
    }
    
protected:
    void testSetSerial();
    void testTxDelegate();

private:
    void incrementCount(int incr) 
    { 
        mCount += incr; 
    }

    int mCount;
};

///////////////////////////////////////////////////////////////////////////////

void TestDpgStreamConnectionHandler::testSetSerial()
{
    DpgStreamConnectionHandler connHandler(123);

    CPPUNIT_ASSERT_EQUAL(uint32_t(123), connHandler.Serial());

    connHandler.SetSerial(9876);
    CPPUNIT_ASSERT_EQUAL(uint32_t(9876), connHandler.Serial());
}

///////////////////////////////////////////////////////////////////////////////

void TestDpgStreamConnectionHandler::testTxDelegate()
{
    DpgStreamConnectionHandler connHandler(0);

    connHandler.RegisterSendDelegate(1, boost::bind(&TestDpgStreamConnectionHandler::incrementCount, this, 1));

    connHandler.RegisterSendDelegate(200, boost::bind(&TestDpgStreamConnectionHandler::incrementCount, this, 200));

    // assert neither called
    CPPUNIT_ASSERT_EQUAL(int(0), mCount);

    connHandler.UpdateGoodputTxSent(200);
    // assert only +1 called
    CPPUNIT_ASSERT_EQUAL(int(1), mCount);
   
    connHandler.UpdateGoodputTxSent(1);
    // assert +200 called
    CPPUNIT_ASSERT_EQUAL(int(201), mCount);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestDpgStreamConnectionHandler);
