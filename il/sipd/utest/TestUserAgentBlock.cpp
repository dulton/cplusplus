#include <cppunit/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <ace/Reactor.h>
#include <ace/Timer_Queue.h>
#include <ildaemon/ThreadSpecificStorage.h>
#include <base/LoadScheduler.h>
#include <boost/scoped_ptr.hpp>
#include <phxexception/PHXException.h>

#include "TestUtilities.h"
#include "UserAgent.h"
#include "UserAgentBlock.h"
#include "UserAgentConfig.h"
#include "UserAgentFactory.h"
#include "UserAgentNameEnumerator.h"
#include "VoIPMediaManager.h"

USING_SIP_NS;
USING_APP_NS;
USING_MEDIA_NS;

///////////////////////////////////////////////////////////////////////////////

class TestUserAgentBlock : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestUserAgentBlock);
    CPPUNIT_TEST(testConfigs);
    CPPUNIT_TEST(testRegistrations);
#ifndef QUICK_TEST
    CPPUNIT_TEST(testCalls);
#endif
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) { }
    void tearDown(void) { }

protected:
    void testConfigs(void);
    void testRegistrations(void);
    void testCalls(void);

    static const size_t NUM_TEST_UAS = 10;
};

///////////////////////////////////////////////////////////////////////////////

void TestUserAgentBlock::testConfigs(void)
{
    boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> voipMediaManager;
    voipMediaManager.reset(new MEDIA_NS::VoIPMediaManager());
    voipMediaManager->initialize(0);

    boost::scoped_ptr<SIP_NS::UserAgentFactory> userAgentFactory;
    userAgentFactory.reset(new SIP_NS::UserAgentFactory(*TSS_Reactor,voipMediaManager.get()));
    userAgentFactory->activate();

    boost::scoped_ptr<UserAgentBlock> uaBlock;
    
    // Test IPv4 UA-to-UA config for A/V calls
    std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().ProtocolConfig.UaNumsPerDevice = NUM_TEST_UAS;

    _CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));
    _CPPUNIT_ASSERT(uaBlock->TotalCount() == NUM_TEST_UAS);

    UserAgentNameEnumerator uaNameEnum(config->Config().ProtocolConfig.UaNumStart,
                                       config->Config().ProtocolConfig.UaNumStep,
                                       config->Config().ProtocolConfig.UaNumFormat);
                                       
    for (size_t i = 0; i < NUM_TEST_UAS; i++)
    {
        uaSharedPtr_t ua = uaBlock->mUaVec[i];
        _CPPUNIT_ASSERT(ua->mIndex == i);
        _CPPUNIT_ASSERT(ua->mName == uaNameEnum.GetName());
	_CPPUNIT_ASSERT(ua->mMediaChannel->getLocalTransportPort(AUDIO_STREAM) == (config->Config().ProtocolProfile.AudioSrcPort + (i * 4)));
        _CPPUNIT_ASSERT(ua->mMediaChannel->getLocalTransportPort(VIDEO_STREAM) == (config->Config().ProtocolProfile.VideoSrcPort + (i * 4)));
        uaNameEnum.Next();
    }

    uaBlock.reset();
    
    // Test IPv4 UA-to-UA config for audio-only calls
    config->Config().ProtocolProfile.CallType = sip_1_port_server::EnumSipCallType_AUDIO_ONLY;
    _CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));

    for (size_t i = 0; i < NUM_TEST_UAS; i++)
    {
        uaSharedPtr_t ua = uaBlock->mUaVec[i];
        _CPPUNIT_ASSERT(ua->mMediaChannel->getLocalTransportPort(AUDIO_STREAM) == (config->Config().ProtocolProfile.AudioSrcPort + (i * 2)));
    }

    uaBlock.reset();
    
    // Test IPv4 UA-to-server config w/valid proxy address
    config->Config().ProtocolProfile.UseUaToUaSignaling = false;
    _CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));

    uaBlock.reset();
    
    // Test IPv4 UA-to-server config w/unspecified proxy address but proxyDomain: should not throw exception
    config->Config().ProtocolProfile.ProxyName = "";
    config->Config().ProtocolProfile.ProxyDomain = "proxydomain";
    memset(config->Config().ProtocolProfile.ProxyIpv4Addr.address, 0, 4);
    _CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));

    uaBlock.reset();
    
    // Test IPv4 UA-to-server config w/unspecified proxy address but proxyName: should not throw exception
    config->Config().ProtocolProfile.ProxyName = "proxyname";
    config->Config().ProtocolProfile.ProxyDomain = "";
    memset(config->Config().ProtocolProfile.ProxyIpv4Addr.address, 0, 4);
    _CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));

    uaBlock.reset();
    
    // Test IPv4 UA-to-server config w/unspecified proxy address nor proxy name/domain: should throw exception
    memset(config->Config().ProtocolProfile.ProxyIpv4Addr.address, 0, 4);
    config->Config().ProtocolProfile.ProxyDomain = "";
    config->Config().ProtocolProfile.ProxyName = "";
    _CPPUNIT_ASSERT_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())), EPHXBadConfig);

    // Test IPv6 UA-to-UA config with local address: should not throw exception
    config = TestUtilities::MakeUserAgentConfigIpv6();
    _CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));
}

///////////////////////////////////////////////////////////////////////////////

void TestUserAgentBlock::testRegistrations(void)
{
    boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> voipMediaManager;
    voipMediaManager.reset(new MEDIA_NS::VoIPMediaManager());
    voipMediaManager->initialize(0);

    boost::scoped_ptr<SIP_NS::UserAgentFactory> userAgentFactory;
    userAgentFactory.reset(new SIP_NS::UserAgentFactory(*TSS_Reactor,voipMediaManager.get()));
    userAgentFactory->activate();

    boost::scoped_ptr<UserAgentBlock> uaBlock;
    
    // Test IPv4 UA-to-server ability to start/stop registrations
    std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().ProtocolConfig.UaNumsPerDevice = NUM_TEST_UAS;
    config->Config().ProtocolProfile.UseUaToUaSignaling = false;
    // ADDED: CL
    config->Config().ProtocolProfile.RegRetries = 2;
    config->Config().ProtocolProfile.ProxyDomain = "proxy.foo.com";
    config->Config().ProtocolProfile.ProxyPort = config->Config().ProtocolConfig.LocalPort;
    config->Config().RegistrarProfile.UseProxyAsRegistrar = true;

    _CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));
    _CPPUNIT_ASSERT(uaBlock->mRegState == sip_1_port_server::EnumSipUaRegState_NOT_REGISTERED);

    // Start registrations
    _CPPUNIT_ASSERT_NO_THROW(uaBlock->Register());
    _CPPUNIT_ASSERT(uaBlock->mRegState == sip_1_port_server::EnumSipUaRegState_REGISTERING);
    
    // Run ACE reactor until all registrations have started
    const ACE_Time_Value regStartTime = ACE_OS::gettimeofday();
    while (uaBlock->mRegIndex < NUM_TEST_UAS)
        TSS_Reactor->handle_events();

    const ACE_Time_Value regRunTime = ACE_OS::gettimeofday() - regStartTime;
    ACE_Time_Value minRunTime;
    minRunTime.set(static_cast<double>(NUM_TEST_UAS - config->Config().ProtocolProfile.RegBurstSize) / config->Config().ProtocolProfile.RegsPerSecond);
    _CPPUNIT_ASSERT(minRunTime < regRunTime);

    // Verify stats
    _CPPUNIT_ASSERT(uaBlock->mStats.regAttempts == NUM_TEST_UAS);
    
    // Start unregistrations
    _CPPUNIT_ASSERT_NO_THROW(uaBlock->Unregister());
    _CPPUNIT_ASSERT(uaBlock->mRegState == sip_1_port_server::EnumSipUaRegState_UNREGISTERING);

    // Run ACE reactor until all unregistrations have started
    while (uaBlock->mRegIndex < NUM_TEST_UAS)
        TSS_Reactor->handle_events();

    _CPPUNIT_ASSERT(uaBlock->mRegState == sip_1_port_server::EnumSipUaRegState_NOT_REGISTERED);
}

///////////////////////////////////////////////////////////////////////////////

void TestUserAgentBlock::testCalls(void)
{
    boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> voipMediaManager;
    voipMediaManager.reset(new MEDIA_NS::VoIPMediaManager());
    voipMediaManager->initialize(0);

    boost::scoped_ptr<SIP_NS::UserAgentFactory> userAgentFactory;
    userAgentFactory.reset(new SIP_NS::UserAgentFactory(*TSS_Reactor,voipMediaManager.get()));
    userAgentFactory->activate();

    boost::scoped_ptr<UserAgentBlock> uaBlock;
    
    // Test IPv4 UA-to-UA ability to initiate calls
    std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().ProtocolConfig.UaNumsPerDevice = NUM_TEST_UAS;

    _CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));

    // Start calls
    _CPPUNIT_ASSERT_NO_THROW(uaBlock->Start());
    
    // Run ACE reactor until load scheduler is done
    const ACE_Time_Value callStartTime = ACE_OS::gettimeofday();
    while (uaBlock->mLoadScheduler->Running())
        TSS_Reactor->handle_events();

    const ACE_Time_Value callRunTime = ACE_OS::gettimeofday() - callStartTime;
    _CPPUNIT_ASSERT((callRunTime + TSS_Reactor->timer_queue()->timer_skew()) >= ACE_Time_Value(config->Config().Load.LoadPhases[0].FlatDescriptor.SteadyTime));

    // Verify stats
    _CPPUNIT_ASSERT(uaBlock->mStats.callAttempts > 0);
    
    // Stop calls
    _CPPUNIT_ASSERT_NO_THROW(uaBlock->Stop());
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestUserAgentBlock);
