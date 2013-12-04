#include <cppunit/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <ace/Event_Handler.h>
#include <ace/Message_Queue.h>
#include <ace/Reactor_Notification_Strategy.h>
#include <ace/Signal.h>
#include <ildaemon/ActiveScheduler.h>
#include <ildaemon/BoundMethodRequestImpl.tcc>
#include <base/BaseCommon.h>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include "AsyncCompletionHandler.h"
#include "SipApplication.h"
#include "VoIPLog.h"
#include "UdpSocketHandler.h"
#include "UdpSocketHandlerFactory.h"
#include "UserAgentLean.h"
#include "UserAgentRawStats.h"
#include <ace/Reactor.h>
#include <ace/Timer_Queue.h>
#include <ildaemon/ThreadSpecificStorage.h>
#include <base/LoadScheduler.h>
#include <phxexception/PHXException.h>

#include "TestUtilities.h"
#include "UserAgent.h"
#include "UserAgentBlock.h"
#include "UserAgentConfig.h"
#include "UserAgentFactory.h"
#include "UserAgentNameEnumerator.h"
#include "VoIPMediaManager.h"
#include "SipUtils.h"
#include "SipEngine.h"

USING_VOIP_NS;
USING_SIP_NS;
USING_RVSIP_NS;
USING_RV_SIP_ENGINE_NAMESPACE;
USING_APP_NS;
USING_MEDIA_NS;
USING_SIPLEAN_NS;

///////////////////////////////////////////////////////////////////////////////

class TestSipDynProxy : public CPPUNIT_NS::TestCase
{
  CPPUNIT_TEST_SUITE(TestSipDynProxy);
  CPPUNIT_TEST(testConfigs);
  CPPUNIT_TEST(testRegistrarProxyConfig);
  CPPUNIT_TEST(testConfiguredLocalUaUri);
  CPPUNIT_TEST(testConfiguredRegistrarIP);
  CPPUNIT_TEST(testConfiguredRegistrarNameAndDomain);
  CPPUNIT_TEST(testConfiguredRegistrarDomainOnly);
#ifndef QUICK_TEST
  CPPUNIT_TEST(testDomainProfileConfigs);
#endif
  CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void)
    {
        ACE_OS::signal(SIGPIPE, SIG_IGN);
    }

protected:
    class MockNotifier : public ACE_Event_Handler, boost::noncopyable
    {
      public:
        explicit MockNotifier(ACE_Reactor& reactor)
            : mNotifier(&reactor, this, ACE_Event_Handler::READ_MASK)
        {
            mQueue.notification_strategy(&mNotifier);
        }
        
        ~MockNotifier()
        {
            mQueue.notification_strategy(0);
        }
        
        void EnqueueNotification(WeakAsyncCompletionToken weakAct, int data)
        {
            mQueue.enqueue(new asyncNotification_t(weakAct, data));
        }
        
      private:
        int handle_input(ACE_HANDLE)
        {
            asyncNotification_t *rawNotif;

            while (1)
            {
                rawNotif = 0;
                mQueue.dequeue(rawNotif, &NO_BLOCK);

                if (!rawNotif)
                    break;

                boost::scoped_ptr<asyncNotification_t> notif(rawNotif);
                if (AsyncCompletionToken act = notif->first.lock())
                    act->Call(notif->second);
            }

            return 0;
        }
        
        typedef std::pair<WeakAsyncCompletionToken, int> asyncNotification_t;
        typedef ACE_Message_Queue_Ex<asyncNotification_t, ACE_MT_SYNCH> asyncNotificationQueue_t;
        
        static ACE_Time_Value NO_BLOCK;
        
        asyncNotificationQueue_t mQueue;
        ACE_Reactor_Notification_Strategy mNotifier;
    };
        
   
    
    void testConfigs(void);
    void testRegistrarProxyConfig(void);
    void testConfiguredLocalUaUri(void);
    void testConfiguredRegistrarIP(void);
    void testConfiguredRegistrarNameAndDomain(void);
    void testConfiguredRegistrarDomainOnly(void);
    void testDomainProfileConfigs(void);

    static const size_t NUM_TEST_VDEV = 256;
    static const size_t NUM_TEST_UAS = 1;
};

///////////////////////////////////////////////////////////////////////////////

ACE_Time_Value TestSipDynProxy::MockNotifier::NO_BLOCK(ACE_Time_Value::zero);

///////////////////////////////////////////////////////////////////////////////

void TestSipDynProxy::testConfigs(void)
{
    boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> voipMediaManager;
    voipMediaManager.reset(new MEDIA_NS::VoIPMediaManager());
    voipMediaManager->initialize(0);

    boost::scoped_ptr<SIP_NS::UserAgentFactory> userAgentFactory;
    userAgentFactory.reset(new SIP_NS::UserAgentFactory(*TSS_Reactor,voipMediaManager.get()));
    userAgentFactory->activate();

    unsigned short vdev;


    std::vector<std::string> dnsServersList;
    dnsServersList.clear();
    dnsServersList.push_back("1.1.1.1");
    dnsServersList.push_back("1.1.1.2");
    dnsServersList.push_back("1.1.1.3");
    dnsServersList.push_back("1.1.1.4");
    for(vdev = 0 ;vdev < NUM_TEST_VDEV;vdev++){
        _CPPUNIT_ASSERT(userAgentFactory->setDNSServers(dnsServersList,vdev) == static_cast<int>(dnsServersList.size()));
        _CPPUNIT_ASSERT(userAgentFactory->setDNSTimeout(-1,-1,vdev) == 0);
    }
    _CPPUNIT_ASSERT(userAgentFactory->setDNSServers(dnsServersList,vdev) == static_cast<int>(dnsServersList.size()));
    _CPPUNIT_ASSERT(userAgentFactory->setDNSTimeout(-1,-1,vdev) == 0);

    dnsServersList.push_back("2.2");
    _CPPUNIT_ASSERT(userAgentFactory->setDNSServers(dnsServersList,0) == -1);

}

///////////////////////////////////////////////////////////////////////////////

void TestSipDynProxy::testRegistrarProxyConfig(void) 
{
	boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> voipMediaManager;
    voipMediaManager.reset(new MEDIA_NS::VoIPMediaManager());
    voipMediaManager->initialize(0);

    boost::scoped_ptr<SIP_NS::UserAgentFactory> userAgentFactory;
    userAgentFactory.reset(new SIP_NS::UserAgentFactory(*TSS_Reactor,voipMediaManager.get()));
    userAgentFactory->activate();

    boost::scoped_ptr<UserAgentBlock> uaBlock;
    
    // Test IPv4 UA-to-UA config for Registrar config
    std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().ProtocolConfig.UaNumsPerDevice = NUM_TEST_UAS;
    
    // Using proxy information for registrar
    bool useProxyAsRegistrar = true;
    
    std::string proxyDomain = "proxy.foo.com";
    
    // Proxy info already set in TestUtilities, but reassigning, for clarity
	config->Config().ProtocolProfile.UseUaToUaSignaling = false;
    config->Config().ProtocolProfile.ProxyDomain = proxyDomain;
    config->Config().ProtocolProfile.ProxyIpv4Addr.address[0] = 127;
    config->Config().ProtocolProfile.ProxyIpv4Addr.address[1] = 0;
    config->Config().ProtocolProfile.ProxyIpv4Addr.address[2] = 0;
    config->Config().ProtocolProfile.ProxyIpv4Addr.address[3] = 1;
    
    config->Config().RegistrarProfile.RegistrarDomain = "";
    config->Config().RegistrarProfile.UseProxyAsRegistrar = useProxyAsRegistrar;
    config->Config().RegistrarProfile.RegistrarIpv4Addr.address[0] = 0;
	config->Config().RegistrarProfile.RegistrarIpv4Addr.address[1] = 0;
	config->Config().RegistrarProfile.RegistrarIpv4Addr.address[2] = 0;
	config->Config().RegistrarProfile.RegistrarIpv4Addr.address[3] = 0;
	
    _CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));
    _CPPUNIT_ASSERT(uaBlock->TotalCount() == NUM_TEST_UAS);
                                    
    uaSharedPtr_t ua = uaBlock->mUaVec[0];
    _CPPUNIT_ASSERT(ua->mUsingProxyAsRegistrar == useProxyAsRegistrar);
    _CPPUNIT_ASSERT(strcmp(ua->mRegistrarDomain.c_str(), proxyDomain.c_str()) == 0);
    _CPPUNIT_ASSERT(strcmp(SipUtils::InetAddressToString(ua->mProxyAddr).c_str(), SipUtils::InetAddressToString(ua->mRegistrarAddr).c_str()) == 0);
    
    uaBlock.reset();

	// Not using proxy information
    useProxyAsRegistrar = false;
    
    // Proxy info already set in TestUtilities, but reassigning, for clarity
	config->Config().ProtocolProfile.UseUaToUaSignaling = false;
    config->Config().ProtocolProfile.ProxyDomain = proxyDomain;
    config->Config().ProtocolProfile.ProxyIpv4Addr.address[0] = 127;
    config->Config().ProtocolProfile.ProxyIpv4Addr.address[1] = 0;
    config->Config().ProtocolProfile.ProxyIpv4Addr.address[2] = 0;
    config->Config().ProtocolProfile.ProxyIpv4Addr.address[3] = 1;
    
    config->Config().RegistrarProfile.RegistrarDomain = "regdomain.com";
    config->Config().RegistrarProfile.UseProxyAsRegistrar = useProxyAsRegistrar;
    config->Config().RegistrarProfile.RegistrarIpv4Addr.address[0] = 1;
	config->Config().RegistrarProfile.RegistrarIpv4Addr.address[1] = 1;
	config->Config().RegistrarProfile.RegistrarIpv4Addr.address[2] = 1;
	config->Config().RegistrarProfile.RegistrarIpv4Addr.address[3] = 1;
    
    _CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));
    ua = uaBlock->mUaVec[0];
    
    _CPPUNIT_ASSERT(ua->mUsingProxyAsRegistrar == useProxyAsRegistrar);
    _CPPUNIT_ASSERT(strcmp(ua->mRegistrarDomain.c_str(), "regdomain.com") == 0);
    _CPPUNIT_ASSERT(strcmp(ua->mRegistrarDomain.c_str(), ua->mProxyDomain.c_str()) != 0);
    _CPPUNIT_ASSERT(strcmp(SipUtils::InetAddressToString(ua->mProxyAddr).c_str(), SipUtils::InetAddressToString(ua->mRegistrarAddr).c_str()) != 0);
    _CPPUNIT_ASSERT(strcmp(SipUtils::InetAddressToString(ua->mRegistrarAddr).c_str(), "1.1.1.1") == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipDynProxy::testConfiguredLocalUaUri(void)
{
	boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> voipMediaManager;
	voipMediaManager.reset(new MEDIA_NS::VoIPMediaManager());
	voipMediaManager->initialize(0);

	boost::scoped_ptr<SIP_NS::UserAgentFactory> userAgentFactory;
	userAgentFactory.reset(new SIP_NS::UserAgentFactory(*TSS_Reactor,voipMediaManager.get()));
	userAgentFactory->activate();
    
	boost::scoped_ptr<UserAgentBlock> uaBlock;
    
	std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
	config->Config().RegistrarProfile.RegistrarPort = 4090;
	config->Config().ProtocolConfig.UaNumsPerDevice = NUM_TEST_UAS;
	config->Config().DomainProfile.LocalUaUriDomainName = "localuadomain.com";
	config->Config().DomainProfile.LocalUaUriPort = 4099;
	
	_CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));
    
	SipEngine * se = SipEngine::getInstance();
	SipChannel * sc = se->mSipChannelsAll.back();

	_CPPUNIT_ASSERT(strcmp(sc->getLocalURIDomain().c_str(), "localuadomain.com") == 0);
	_CPPUNIT_ASSERT(sc->getLocalURIDomainPort() == 4099);
	
    std::string strExternalContactAddress;
	
    sc->getFromStrPrivate(sc->getOurDomainStrPrivate(),strExternalContactAddress);

	_CPPUNIT_ASSERT(strcmp(strExternalContactAddress.c_str(), "\"1000\" <sip:1000@localuadomain.com:4099;transport=UDP>") == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipDynProxy::testConfiguredRegistrarIP(void) 
{
	boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> voipMediaManager;
	voipMediaManager.reset(new MEDIA_NS::VoIPMediaManager());
	voipMediaManager->initialize(0);

	boost::scoped_ptr<SIP_NS::UserAgentFactory> userAgentFactory;
	userAgentFactory.reset(new SIP_NS::UserAgentFactory(*TSS_Reactor,voipMediaManager.get()));
	userAgentFactory->activate();
    
	boost::scoped_ptr<UserAgentBlock> uaBlock;
    
	std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
	config->Config().RegistrarProfile.RegistrarPort = 4090;
	config->Config().ProtocolConfig.UaNumsPerDevice = NUM_TEST_UAS;	
	config->Config().DomainProfile.LocalUaUriDomainName = "";
	config->Config().DomainProfile.LocalUaUriPort = 0;
	config->Config().ProtocolProfile.UseUaToUaSignaling = false;
	config->Config().RegistrarProfile.RegistrarDomain = "";
    config->Config().RegistrarProfile.UseProxyAsRegistrar = false;
    config->Config().RegistrarProfile.RegistrarIpv4Addr.address[0] = 1;
	config->Config().RegistrarProfile.RegistrarIpv4Addr.address[1] = 2;
	config->Config().RegistrarProfile.RegistrarIpv4Addr.address[2] = 3;
	config->Config().RegistrarProfile.RegistrarIpv4Addr.address[3] = 4;
	
	_CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));
	uaSharedPtr_t ua = uaBlock->mUaVec[0];

	// Set registrar information
    ua->Register();
    
    _CPPUNIT_ASSERT(strcmp(SipUtils::InetAddressToString(ua->mRegistrarAddr).c_str(), "1.2.3.4") == 0);

	SipEngine * se = SipEngine::getInstance();
	SipChannel * sc = se->mSipChannelsAll.back();
	
    std::string strExternalContactAddress;

    sc->getFromStrPrivate(sc->getOurDomainStrPrivate(),strExternalContactAddress);
	
	
	_CPPUNIT_ASSERT(strcmp(strExternalContactAddress.c_str(), "\"1000\" <sip:1000@1.2.3.4:4090;transport=UDP>") == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipDynProxy::testConfiguredRegistrarNameAndDomain(void) 
{
	boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> voipMediaManager;
	voipMediaManager.reset(new MEDIA_NS::VoIPMediaManager());
	voipMediaManager->initialize(0);

	boost::scoped_ptr<SIP_NS::UserAgentFactory> userAgentFactory;
	userAgentFactory.reset(new SIP_NS::UserAgentFactory(*TSS_Reactor,voipMediaManager.get()));
	userAgentFactory->activate();
    
	boost::scoped_ptr<UserAgentBlock> uaBlock;
    
	std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
	config->Config().RegistrarProfile.RegistrarPort = 4090;
	config->Config().ProtocolConfig.UaNumsPerDevice = NUM_TEST_UAS;	
	config->Config().DomainProfile.LocalUaUriDomainName = "";
	config->Config().DomainProfile.LocalUaUriPort = 0;
	config->Config().ProtocolProfile.UseUaToUaSignaling = false;
    config->Config().RegistrarProfile.UseProxyAsRegistrar = false;
	config->Config().RegistrarProfile.RegistrarName = "regname";
	config->Config().RegistrarProfile.RegistrarDomain = "regdomain.com";
	config->Config().RegistrarProfile.RegistrarIpv4Addr.address[0] = 0;
	config->Config().RegistrarProfile.RegistrarIpv4Addr.address[1] = 0;
	config->Config().RegistrarProfile.RegistrarIpv4Addr.address[2] = 0;
	config->Config().RegistrarProfile.RegistrarIpv4Addr.address[3] = 0;
	
	_CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));
	uaSharedPtr_t ua = uaBlock->mUaVec[0];
	
	// Set registrar information
    ua->Register();
    
	SipEngine * se = SipEngine::getInstance();
	SipChannel * sc = se->mSipChannelsAll.back();
	
	_CPPUNIT_ASSERT(strcmp(sc->mRegServerName.c_str(), "regname") == 0);
	
    std::string strExternalContactAddress;

    sc->getFromStrPrivate(sc->getOurDomainStrPrivate(),strExternalContactAddress);

	_CPPUNIT_ASSERT(strcmp(strExternalContactAddress.c_str(), "\"1000\" <sip:1000@regdomain.com:4090;transport=UDP>") == 0);	
}

///////////////////////////////////////////////////////////////////////////////

void TestSipDynProxy::testDomainProfileConfigs(void) {
	// Start application
    SipApplication app(1);
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    _CPPUNIT_ASSERT(scheduler.Start() == 0);

    boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> voipMediaManager;
    voipMediaManager.reset(new VoIPMediaManager());
    voipMediaManager->initialize(0);

    // Create mock notifier for ACT dispatch
    MockNotifier mockNotifier(*scheduler.AppReactor());
    AsyncCompletionHandler asyncHandler;
    asyncHandler.SetMethodCompletionDelegate(boost::bind(&MockNotifier::EnqueueNotification, &mockNotifier, _1, _2));
    // Activate application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Activate, &app, boost::ref(*scheduler.AppReactor()), &asyncHandler, voipMediaManager.get());
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure two blocks with one UA each
    SipApplication::uaConfigVec_t configs;
    SipApplication::handleList_t handles;
    
    std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().Load.LoadProfile.MaxConnectionsAttempted = 1;
    config->Config().ProtocolProfile.CallTime = 2;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 1;
    config->Config().ProtocolConfig.UaNumStart = 1000;
    config->Config().ProtocolConfig.LocalPort = 4071;
    config->Config().Endpoint.DstPort = 5071;
    config->Config().ProtocolConfig.UaNumsPerDevice = NUM_TEST_UAS;	
    config->Config().ProtocolProfile.UseUaToUaSignaling = false;
    config->Config().RegistrarProfile.UseProxyAsRegistrar = false;
    config->Config().DomainProfile.RemotePeerSipDomainName = "remotedomain.com";
    config->Config().DomainProfile.RemotePeerSipPort = 8888;
    config->Config().DomainProfile.LocalContactSipDomainName = "contactdomain.com";
    config->Config().DomainProfile.LocalContactSipPort = 7777;
    configs.push_back(*config);
    
    config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config->Config().ProtocolConfig.UaNumStart = 2000;
    config->Config().ProtocolConfig.LocalPort = 5071;
    config->Config().Endpoint.DstPort = 0;
    configs.push_back(*config);
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::ConfigUserAgents, &app, 0, boost::cref(configs), boost::ref(handles));
        scheduler.Enqueue(req);
        req->Wait();
    }
		  
    _CPPUNIT_ASSERT(handles.size() == 2);
    _CPPUNIT_ASSERT(handles[0] == 1);
    _CPPUNIT_ASSERT(handles[1] == 2);
    
    // Start calls on the first UA block
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::StartProtocol, &app, 0, SipApplication::handleList_t(1, handles[0]));
        scheduler.Enqueue(req);
        req->Wait();
    }
	
    sleep(60);
		
    // Stop calls
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::StopProtocol, &app, 0, SipApplication::handleList_t(1, handles[0]));
        scheduler.Enqueue(req);
        req->Wait();
    }
	
	SipEngine * se = SipEngine::getInstance();
    SipChannel * sc = se->mSipChannelsAll.front();

	_CPPUNIT_ASSERT(strcmp(sc->mRemoteDomain.c_str(), "remotedomain.com") == 0);
	_CPPUNIT_ASSERT(sc->mRemoteDomainPort == 8888);
	_CPPUNIT_ASSERT(strcmp(sc->mContactDomain.c_str(), "contactdomain.com") == 0);
	_CPPUNIT_ASSERT(sc->mContactDomainPort == 7777);
	
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }

    _CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipDynProxy::testConfiguredRegistrarDomainOnly(void) 
{
	boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> voipMediaManager;
	voipMediaManager.reset(new MEDIA_NS::VoIPMediaManager());
	voipMediaManager->initialize(0);

	boost::scoped_ptr<SIP_NS::UserAgentFactory> userAgentFactory;
	userAgentFactory.reset(new SIP_NS::UserAgentFactory(*TSS_Reactor,voipMediaManager.get()));
	userAgentFactory->activate();
    
	boost::scoped_ptr<UserAgentBlock> uaBlock;
    
	std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
	config->Config().RegistrarProfile.RegistrarPort = 4090;
	config->Config().ProtocolConfig.UaNumsPerDevice = NUM_TEST_UAS;	
	config->Config().DomainProfile.LocalUaUriDomainName = "";
	config->Config().DomainProfile.LocalUaUriPort = 0;
	config->Config().ProtocolProfile.UseUaToUaSignaling = false;
    config->Config().RegistrarProfile.UseProxyAsRegistrar = false;
	config->Config().RegistrarProfile.RegistrarName = "";
	config->Config().RegistrarProfile.RegistrarDomain = "regdomain.com";
	config->Config().RegistrarProfile.RegistrarIpv4Addr.address[0] = 0;
	config->Config().RegistrarProfile.RegistrarIpv4Addr.address[1] = 0;
	config->Config().RegistrarProfile.RegistrarIpv4Addr.address[2] = 0;
	config->Config().RegistrarProfile.RegistrarIpv4Addr.address[3] = 0;
	_CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));
    uaSharedPtr_t ua = uaBlock->mUaVec[0];

	// Set registrar information
    ua->Register();
    
	SipEngine * se = SipEngine::getInstance();
	SipChannel * sc = se->mSipChannelsAll.back();
	
    std::string strExternalContactAddress;

    sc->getFromStrPrivate(sc->getOurDomainStrPrivate(),strExternalContactAddress);
	
    _CPPUNIT_ASSERT(strcmp(strExternalContactAddress.c_str(), "\"1000\" <sip:1000@regdomain.com>") == 0);	
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestSipDynProxy);
