#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "ModifierBlock.h"
#include "FlowEngine.h"
#include "PlaylistEngine.h"

#include "MockFlowConfigProxy.h"
#include "MockSocketManager.h"
#include "MockTimerManager.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class TestPlaylistEngine : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestPlaylistEngine);
    CPPUNIT_TEST(testCreatePlaylist);
    CPPUNIT_TEST(testUpdatePlaylist);
    CPPUNIT_TEST(testBadPlaylist);
    CPPUNIT_TEST(testDeletePlaylist);
    CPPUNIT_TEST(testMakePlaylist);
    CPPUNIT_TEST(testLinkLoopInfoToFlows);
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
        delete pe;
        delete fe;
        delete tm;
        delete sm;
    }

protected:
    void testCreatePlaylist(void);
    void testUpdatePlaylist(void);
    void testBadPlaylist(void);
    void testDeletePlaylist(void);
    void testMakePlaylist(void);
    void testLinkLoopInfoToFlows(void);

private:
    FlowEngine * fe;
    PlaylistEngine * pe;
    MockSocketMgr * sm;
    MockTimerMgr * tm;
};


static bool operator==(const PlaylistConfig::Flow& a, const PlaylistConfig::Flow& b)
{
    if (a.flowHandle != b.flowHandle)
    {
        return false;
    }
    if (a.startTime != b.startTime)
    {
        return false;
    }
    if (a.dataDelay != b.dataDelay)
    {
        return false;
    }
    if (a.minPktDelay != b.minPktDelay)
    {
        return false;
    }
    if (a.maxPktDelay != b.maxPktDelay)
    {
        return false;
    }
    if (a.socketTimeout != b.socketTimeout)
    {
        return false;
    }
    if (a.serverPort != b.serverPort)
    {
        return false;
    }
    if (a.reversed != b.reversed)
    {
        return false;
    }
    return true;
}

static bool operator!=(const PlaylistConfig::Flow& a, const PlaylistConfig::Flow& b)
{
    return !(a==b);
}

static bool operator==(const PlaylistConfig::LoopInfo& a, const PlaylistConfig::LoopInfo& b)
{
    if (a.begIdx != b.begIdx)
    {
        return false;
    }
    if (a.count != b.count)
    {
        return false;
    }
    return true;
}

static bool operator!=(const PlaylistConfig::LoopInfo& a, const PlaylistConfig::LoopInfo& b)
{
    return !(a==b);
}

static bool operator==(const PlaylistEngine::Config_t& a, const PlaylistEngine::Config_t& b)
{
    if (a.type != b.type)
    {
        return false;
    }
    if (a.name != b.name)
    {
        return false;
    }
    if (a.flows.size() != b.flows.size())
    {
        return false;
    }
    std::vector<PlaylistConfig::Flow>::const_iterator aIter = a.flows.begin();
    std::vector<PlaylistConfig::Flow>::const_iterator bIter = b.flows.begin();

    while (aIter != a.flows.end())
    {
        if (*aIter != *bIter)
        {
            return false;
        }

        ++aIter;
        ++bIter;
    }

    if (a.loopMap.size() != b.loopMap.size())
    {
        return false;
    }
    std::map<uint16_t, PlaylistConfig::LoopInfo>::const_iterator aMapIter = a.loopMap.begin();
    std::map<uint16_t, PlaylistConfig::LoopInfo>::const_iterator bMapIter = b.loopMap.begin();

    while (aMapIter != a.loopMap.end())
    {
        if (aMapIter->second != bMapIter->second)
        {
            return false;
        }
        ++aMapIter;
        ++bMapIter;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistEngine::testCreatePlaylist(void)
{
    uint32_t handle = 1;
    PlaylistEngine::Config_t config;

    CPPUNIT_ASSERT(pe->GetPlaylist(handle) == 0);

    pe->UpdatePlaylist(handle, config);

    CPPUNIT_ASSERT(pe->GetPlaylist(handle));
    CPPUNIT_ASSERT(*pe->GetPlaylist(handle) == config);
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistEngine::testUpdatePlaylist(void)
{
    // create flow config
    FlowEngine::Config_t flowConfig;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    uint32_t handle = 1;
    PlaylistEngine::Config_t config;
    config.type = PlaylistEngine::Config_t::STREAM_ONLY;

    config.flows.resize(1);
    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 100;
    config.flows[0].dataDelay = 10;
    config.flows[0].serverPort = 80;

    // create playlist
    pe->UpdatePlaylist(handle, config);

    PlaylistEngine::Config_t config2;
    config2.type = PlaylistEngine::Config_t::ATTACK_ONLY;
    config2.flows.resize(1);
    config2.flows[0].flowHandle = flowHandle;
    config2.flows[0].startTime = 100;
    config2.flows[0].dataDelay = 10;
    config2.flows[0].serverPort = 80;

    PlaylistConfig::LoopInfo loop; // from flow 0 to flow 0, loop 5 times
    loop.count = 5;
    loop.begIdx = 0;
    config2.loopMap.insert(std::make_pair(0, loop));

    // change playlist
    pe->UpdatePlaylist(handle, config2);

    CPPUNIT_ASSERT(pe->GetPlaylist(handle));
    CPPUNIT_ASSERT(*pe->GetPlaylist(handle) == config2);
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistEngine::testBadPlaylist(void)
{
    uint32_t handle = 1;
    PlaylistEngine::Config_t config;
    config.flows.resize(1);
    config.flows[0].flowHandle = 1;

    CPPUNIT_ASSERT(fe->GetFlow(config.flows[0].flowHandle) == 0);

    // throw an error if flow doesn't exist 
    CPPUNIT_ASSERT_THROW(pe->UpdatePlaylist(handle, config), DPG_EInternal);
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistEngine::testDeletePlaylist(void)
{
    uint32_t handle = 1;
    PlaylistEngine::Config_t config;

    // create playlist 
    pe->UpdatePlaylist(handle, config); 

    // delete it
    pe->DeletePlaylist(handle);

    // it's gone
    CPPUNIT_ASSERT(pe->GetPlaylist(handle) == 0);

    // throw an error if someone deletes it again
    CPPUNIT_ASSERT_THROW(pe->DeletePlaylist(1), DPG_EInternal);
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistEngine::testMakePlaylist(void)
{
/// stream playlist 
    // create flow config
    FlowEngine::Config_t flowConfig; 
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    uint32_t handle = 1;
    PlaylistEngine::Config_t config;

    config.flows.resize(5);

    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 100;
    config.flows[0].dataDelay = 10;
    config.flows[0].serverPort = 80;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 10;
    config.flows[1].dataDelay = 10;
    config.flows[1].serverPort = 8080;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 120;
    config.flows[2].dataDelay = 10;
    config.flows[2].serverPort = 2000;

    config.flows[3].flowHandle = flowHandle;
    config.flows[3].startTime = 100;
    config.flows[3].dataDelay = 10;
    config.flows[3].minPktDelay = 50;
    config.flows[3].maxPktDelay = 50,

    config.flows[4].flowHandle = flowHandle;
    config.flows[4].startTime = 100;
    config.flows[4].dataDelay = 10;
    config.flows[4].minPktDelay = 10;
    config.flows[4].serverPort = 65000;

    pe->UpdatePlaylist(handle, config);

    std::auto_ptr<ModifierBlock> mods(pe->MakeModifierBlock(handle, 12345));
    PlaylistInstance * pi = pe->MakePlaylist(handle, *mods, *sm, true);
    CPPUNIT_ASSERT(pi);

///attack playlist
    // create flow config
    FlowEngine::Config_t flowConfig2;
    flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig2));

    handle = 3;
    PlaylistEngine::Config_t config2;
    config2.type = PlaylistEngine::Config_t::ATTACK_ONLY;

    config2.flows.resize(3);

    config2.flows[0].flowHandle = flowHandle;
    config2.flows[0].minPktDelay = 50 ;
    config2.flows[0].socketTimeout = 200;
    config2.flows[0].serverPort = 80;

    config2.flows[1].flowHandle = flowHandle;
    config2.flows[1].socketTimeout = 1000;
    config2.flows[1].serverPort = 3080;

    config2.flows[2].flowHandle = flowHandle;
    config2.flows[2].minPktDelay = 1000;
    config2.flows[2].socketTimeout = 2000;

    pe->UpdatePlaylist(handle, config2);

    std::auto_ptr<ModifierBlock> mods2(pe->MakeModifierBlock(handle, 12345));

    PlaylistInstance * pi2 = pe->MakePlaylist(handle, *mods2, *sm, true);

    CPPUNIT_ASSERT(pi2);
}

///////////////////////////////////////////////////////////////////////////////

void TestPlaylistEngine::testLinkLoopInfoToFlows(void)
{
    // create flow config
    FlowEngine::Config_t flowConfig;
    uint32_t flowHandle = 2;
    fe->UpdateFlow(flowHandle, MockFlowConfigProxy(flowConfig));

    uint32_t handle = 1;
    PlaylistEngine::Config_t config;

    config.flows.resize(5);

    config.flows[0].flowHandle = flowHandle;
    config.flows[0].startTime = 100;
    config.flows[0].dataDelay = 10;
    config.flows[0].serverPort = 80;

    config.flows[1].flowHandle = flowHandle; 
    config.flows[1].startTime = 10;
    config.flows[1].dataDelay = 10;
    config.flows[1].serverPort = 8080;

    config.flows[2].flowHandle = flowHandle;
    config.flows[2].startTime = 120;
    config.flows[2].dataDelay = 10;
    config.flows[2].serverPort = 2000;

    config.flows[3].flowHandle = flowHandle;
    config.flows[3].startTime = 100;
    config.flows[3].dataDelay = 10;
    config.flows[3].minPktDelay = 50;
    config.flows[3].maxPktDelay = 50,

    config.flows[4].flowHandle = flowHandle;
    config.flows[4].startTime = 100;
    config.flows[4].dataDelay = 10;
    config.flows[4].minPktDelay = 10;
    config.flows[4].serverPort = 65000;

    PlaylistConfig::LoopInfo loop; // from flow 1 to flow 3, loop 5 times
    loop.count = 5;
    loop.begIdx = 1;
    config.loopMap.insert(std::make_pair(3, loop));

    // create playlist 
    pe->UpdatePlaylist(handle, config);

    const PlaylistEngine::Config_t *playlist = pe->GetPlaylist(handle);
    CPPUNIT_ASSERT(playlist);
    CPPUNIT_ASSERT(playlist->flows[0].loopInfo==0);
    CPPUNIT_ASSERT(playlist->flows[1].loopInfo!=0);
    CPPUNIT_ASSERT(playlist->flows[2].loopInfo!=0);
    CPPUNIT_ASSERT(playlist->flows[3].loopInfo!=0);
    CPPUNIT_ASSERT(playlist->flows[4].loopInfo==0);

    CPPUNIT_ASSERT_EQUAL(uint16_t(5), playlist->flows[1].loopInfo->count);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), playlist->flows[1].loopInfo->begIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(5), playlist->flows[2].loopInfo->count);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), playlist->flows[2].loopInfo->begIdx);
    CPPUNIT_ASSERT_EQUAL(uint16_t(5), playlist->flows[3].loopInfo->count);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), playlist->flows[3].loopInfo->begIdx);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestPlaylistEngine);
