#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "TcpPortUtils.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class TestTcpPortUtils : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestTcpPortUtils);
    CPPUNIT_TEST(testFlowIdx);
    CPPUNIT_TEST(testPlayInst);
    CPPUNIT_TEST(testAtkLoop);
    CPPUNIT_TEST(testGetCursor);
    CPPUNIT_TEST_SUITE_END();

public:
    TestTcpPortUtils()
    {
    }

    ~TestTcpPortUtils()
    {
    }

    void setUp() 
    { 
    }

    void tearDown() 
    { 
    }
    
protected:
    void testFlowIdx();
    void testPlayInst();
    void testAtkLoop();
    void testGetCursor();

private:
    static const uint16_t MAX_PRIV_PORT = 1023;
};

///////////////////////////////////////////////////////////////////////////////

void TestTcpPortUtils::testFlowIdx()
{
    // attacks
    bool isAttack = true;
    for (uint16_t atkLoop = 0; atkLoop <= TcpPortUtils::MaxAtkLoop(); 
         atkLoop = (atkLoop ? atkLoop << 1 : 1))
    {
        for (uint16_t playInst = 0; playInst <= TcpPortUtils::MaxPlayInst(isAttack); 
             playInst = (playInst ? playInst << 1 : 1))
        {
            for (uint16_t flowIdx = 0; flowIdx <= TcpPortUtils::MaxFlowIdx(isAttack); 
                 ++flowIdx)
            {
                uint16_t port = TcpPortUtils::MakeAtkPort(flowIdx, playInst, atkLoop);
                CPPUNIT_ASSERT(port > MAX_PRIV_PORT);
                CPPUNIT_ASSERT_EQUAL(flowIdx, TcpPortUtils::GetFlowIdx(port, isAttack));
                CPPUNIT_ASSERT_EQUAL(playInst, TcpPortUtils::GetPlayInst(port, isAttack));
                CPPUNIT_ASSERT_EQUAL(atkLoop, TcpPortUtils::GetAtkLoop(port));
            }
        }
    }

    // streams
    isAttack = false;
    for (uint16_t playInst = 0; playInst <= TcpPortUtils::MaxPlayInst(isAttack); 
         playInst = (playInst ? playInst << 1 : 1))
    {
        for (uint16_t flowIdx = 0; flowIdx <= TcpPortUtils::MaxFlowIdx(isAttack); 
             ++flowIdx)
        {
            uint16_t port = TcpPortUtils::MakeStmPort(flowIdx, playInst);
            CPPUNIT_ASSERT(port > MAX_PRIV_PORT);
            CPPUNIT_ASSERT_EQUAL(flowIdx, TcpPortUtils::GetFlowIdx(port, isAttack));
            CPPUNIT_ASSERT_EQUAL(playInst, TcpPortUtils::GetPlayInst(port, isAttack));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void TestTcpPortUtils::testPlayInst()
{
    // attacks
    bool isAttack = true;
    for (uint16_t atkLoop = 0; atkLoop <= TcpPortUtils::MaxAtkLoop(); 
         atkLoop = (atkLoop ? atkLoop << 1 : 1))
    {
        for (uint16_t flowIdx = 0; flowIdx <= TcpPortUtils::MaxFlowIdx(isAttack); 
             flowIdx = (flowIdx ? flowIdx << 1 : 1))
        {
            for (uint16_t playInst = 0; playInst <= TcpPortUtils::MaxPlayInst(isAttack); 
                 ++playInst)
            {
                uint16_t port = TcpPortUtils::MakeAtkPort(flowIdx, playInst, atkLoop);
                CPPUNIT_ASSERT(port > MAX_PRIV_PORT);
                CPPUNIT_ASSERT_EQUAL(flowIdx, TcpPortUtils::GetFlowIdx(port, isAttack));
                CPPUNIT_ASSERT_EQUAL(playInst, TcpPortUtils::GetPlayInst(port, isAttack));
                CPPUNIT_ASSERT_EQUAL(atkLoop, TcpPortUtils::GetAtkLoop(port));
            }
        }
    }

    // streams
    isAttack = false;
    for (uint16_t flowIdx = 0; flowIdx <= TcpPortUtils::MaxFlowIdx(isAttack); 
         flowIdx = (flowIdx ? flowIdx << 1 : 1))
    {
        for (uint16_t playInst = 0; playInst <= TcpPortUtils::MaxPlayInst(isAttack); 
             ++playInst)
        {
            uint16_t port = TcpPortUtils::MakeStmPort(flowIdx, playInst);
            CPPUNIT_ASSERT(port > MAX_PRIV_PORT);
            CPPUNIT_ASSERT_EQUAL(flowIdx, TcpPortUtils::GetFlowIdx(port, isAttack));
            CPPUNIT_ASSERT_EQUAL(playInst, TcpPortUtils::GetPlayInst(port, isAttack));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void TestTcpPortUtils::testAtkLoop()
{
    bool isAttack = true;
    for (uint16_t flowIdx = 0; flowIdx <= TcpPortUtils::MaxFlowIdx(isAttack); 
         flowIdx = (flowIdx ? flowIdx << 1 : 1))
    {
        for (uint16_t playInst = 0; playInst <= TcpPortUtils::MaxPlayInst(isAttack); 
             playInst = (playInst ? playInst << 1 : 1))
        {
            for (uint16_t atkLoop = 0; atkLoop <= TcpPortUtils::MaxAtkLoop(); 
                 ++atkLoop)
            {
                uint16_t port = TcpPortUtils::MakeAtkPort(flowIdx, playInst, atkLoop);

                CPPUNIT_ASSERT(port > MAX_PRIV_PORT);
                CPPUNIT_ASSERT_EQUAL(flowIdx, TcpPortUtils::GetFlowIdx(port, isAttack));
                CPPUNIT_ASSERT_EQUAL(playInst, TcpPortUtils::GetPlayInst(port, isAttack));
                CPPUNIT_ASSERT_EQUAL(atkLoop, TcpPortUtils::GetAtkLoop(port));
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void TestTcpPortUtils::testGetCursor()
{
    // Attacks
    bool isAttack = true;
    for (uint16_t atkLoop = 0; atkLoop <= TcpPortUtils::MaxAtkLoop(); 
         atkLoop = (atkLoop ? atkLoop << 1 : 1))
    {
        for (uint16_t flowIdx = 0; flowIdx <= TcpPortUtils::MaxFlowIdx(isAttack); 
             flowIdx = (flowIdx ? flowIdx << 1 : 1))
        {
            for (uint32_t initCursor = 0; initCursor < 0x80000000; 
                 initCursor = (initCursor ? initCursor << 1 : 1))
            {
                for (uint16_t playInst =0; playInst <= TcpPortUtils::MaxPlayInst(isAttack); 
                     playInst = (playInst ? playInst << 1 : 1))
                {
                    uint32_t cursor = (initCursor & ~TcpPortUtils::ATK_PLAY_INST_MASK) | playInst;
                    CPPUNIT_ASSERT_EQUAL(cursor-50, 
                        TcpPortUtils::GetCursor(cursor, 
                        TcpPortUtils::MakeAtkPort(flowIdx, playInst-50, atkLoop), isAttack));
                    CPPUNIT_ASSERT_EQUAL(cursor-10, 
                        TcpPortUtils::GetCursor(cursor, 
                        TcpPortUtils::MakeAtkPort(flowIdx, playInst-10, atkLoop), isAttack));
                    CPPUNIT_ASSERT_EQUAL(cursor-2, 
                        TcpPortUtils::GetCursor(cursor, 
                        TcpPortUtils::MakeAtkPort(flowIdx, playInst-2, atkLoop), isAttack));
                    CPPUNIT_ASSERT_EQUAL(cursor-1, 
                        TcpPortUtils::GetCursor(cursor, 
                        TcpPortUtils::MakeAtkPort(flowIdx, playInst-1, atkLoop), isAttack));
                    CPPUNIT_ASSERT_EQUAL(cursor, 
                        TcpPortUtils::GetCursor(cursor, 
                        TcpPortUtils::MakeAtkPort(flowIdx, playInst, atkLoop), isAttack));
                    CPPUNIT_ASSERT_EQUAL(cursor+1, 
                        TcpPortUtils::GetCursor(cursor, 
                        TcpPortUtils::MakeAtkPort(flowIdx, playInst+1, atkLoop), isAttack));
                    CPPUNIT_ASSERT_EQUAL(cursor+2, 
                        TcpPortUtils::GetCursor(cursor, 
                        TcpPortUtils::MakeAtkPort(flowIdx, playInst+2, atkLoop), isAttack));
                    CPPUNIT_ASSERT_EQUAL(cursor+10, 
                        TcpPortUtils::GetCursor(cursor, 
                        TcpPortUtils::MakeAtkPort(flowIdx, playInst+10, atkLoop), isAttack));
                    CPPUNIT_ASSERT_EQUAL(cursor+50, 
                        TcpPortUtils::GetCursor(cursor, 
                        TcpPortUtils::MakeAtkPort(flowIdx, playInst+50, atkLoop), isAttack));
                }
            }
        }
    }

    // Streams
    isAttack = false;
    for (uint16_t flowIdx = 0; flowIdx <= TcpPortUtils::MaxFlowIdx(isAttack); 
         flowIdx = (flowIdx ? flowIdx << 1 : 1))
    {
        for (uint32_t initCursor = 0; initCursor < 0x80000000; 
             initCursor = (initCursor ? initCursor << 1 : 1))
        {
            for (uint16_t playInst =0; playInst <= TcpPortUtils::MaxPlayInst(isAttack); 
                 playInst = (playInst ? playInst << 1 : 1))
            {
                uint32_t cursor = (initCursor & ~TcpPortUtils::STM_PLAY_INST_MASK) | playInst;
                CPPUNIT_ASSERT_EQUAL(cursor-50, 
                    TcpPortUtils::GetCursor(cursor, 
                    TcpPortUtils::MakeStmPort(flowIdx, playInst-50), isAttack));
                CPPUNIT_ASSERT_EQUAL(cursor-10, 
                    TcpPortUtils::GetCursor(cursor, 
                    TcpPortUtils::MakeStmPort(flowIdx, playInst-10), isAttack));
                CPPUNIT_ASSERT_EQUAL(cursor-2, 
                    TcpPortUtils::GetCursor(cursor, 
                    TcpPortUtils::MakeStmPort(flowIdx, playInst-2), isAttack));
                CPPUNIT_ASSERT_EQUAL(cursor-1, 
                    TcpPortUtils::GetCursor(cursor, 
                    TcpPortUtils::MakeStmPort(flowIdx, playInst-1), isAttack));
                CPPUNIT_ASSERT_EQUAL(cursor, 
                    TcpPortUtils::GetCursor(cursor, 
                    TcpPortUtils::MakeStmPort(flowIdx, playInst), isAttack));
                CPPUNIT_ASSERT_EQUAL(cursor+1, 
                    TcpPortUtils::GetCursor(cursor, 
                    TcpPortUtils::MakeStmPort(flowIdx, playInst+1), isAttack));
                CPPUNIT_ASSERT_EQUAL(cursor+2, 
                    TcpPortUtils::GetCursor(cursor, 
                    TcpPortUtils::MakeStmPort(flowIdx, playInst+2), isAttack));
                CPPUNIT_ASSERT_EQUAL(cursor+10, 
                    TcpPortUtils::GetCursor(cursor, 
                    TcpPortUtils::MakeStmPort(flowIdx, playInst+10), isAttack));
                CPPUNIT_ASSERT_EQUAL(cursor+50, 
                    TcpPortUtils::GetCursor(cursor, 
                    TcpPortUtils::MakeStmPort(flowIdx, playInst+50), isAttack));
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestTcpPortUtils);
