#include <memory>
#include <string>

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

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "AsyncCompletionHandler.h"
#include "SipApplication.h"
#include "VoIPLog.h"
#include "TestUtilities.h"
#include "UdpSocketHandler.h"
#include "UdpSocketHandlerFactory.h"
#include "UserAgentConfig.h"
#include "UserAgentLean.h"
#include "UserAgentRawStats.h"
#include "VoIPMediaManager.h"

USING_VOIP_NS;
USING_MEDIA_NS;
USING_APP_NS;
USING_SIP_NS;
USING_SIPLEAN_NS;

///////////////////////////////////////////////////////////////////////////////

class TestSipApplication : public CPPUNIT_NS::TestCase
{
   
  CPPUNIT_TEST_SUITE(TestSipApplication); 
#ifndef QUICK_TEST
  CPPUNIT_TEST(testRegistrationRetries); 
#endif
  CPPUNIT_TEST(testRegistration);
  CPPUNIT_TEST(testTipRegistration); 
#ifndef QUICK_TEST  
  _CPPUNIT_TEST(testDirectCallsIpv6);
  _CPPUNIT_TEST(testDirectCallsIpv6MUAs);
  _CPPUNIT_TEST(testDirectCallsIpv6Proxy);
  _CPPUNIT_TEST(testDirectCallsEncRTP);  
  _CPPUNIT_TEST(testDirectCallsTIPCTSMedia);
  _CPPUNIT_TEST(testDirectCallsTIPTandbergMedia);
  _CPPUNIT_TEST(testDirectCalls);
  _CPPUNIT_TEST(testDirectCallsDupImpairment);
  _CPPUNIT_TEST(testDirectCalls33PctLossImpairment);
  _CPPUNIT_TEST(testDirectCallWithRefresh); 
  _CPPUNIT_TEST(testDirectCallWithGracefulStop);
#endif
  CPPUNIT_TEST_SUITE_END();
  
public:
    void setUp(void)
    {
        ACE_OS::signal(SIGPIPE, SIG_IGN);
        //PHX_LOG_REDIRECTLOG_INIT("", "0x1-App,0x2-MPS,0x4-SOCKET,0x8-SIP,0x10-Unused", LOG_UPTO(PHX_LOG_INFO), PHX_LOCAL_ID_1 | PHX_LOCAL_ID_2 | PHX_LOCAL_ID_3 | PHX_LOCAL_ID_4);
    }
    
    void tearDown(void) { }
    
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
        
    struct DupSender
    {
        typedef bool result_type;
        
        bool operator()(UdpSocketHandler& sock, const Packet& packet) const
        {
            return sock.SendImmediate(packet) && sock.SendImmediate(packet);
        }
    };

    struct LossySender
    {
        typedef bool result_type;

        LossySender(size_t recycle)
            : mRecycle(recycle),
              mCount(0)
        {
        }
        
        bool operator()(UdpSocketHandler& sock, const Packet& packet)
        {
            if (++mCount < mRecycle)
                sock.SendImmediate(packet);
            else
                mCount = 0;

            return true;
        }

      private:
        const size_t mRecycle;
        size_t mCount;
    };
    void testDirectCalls(void);
    void testDirectCallsIpv6(void);
    void testDirectCallsIpv6MUAs(void);
    void testDirectCallsIpv6Proxy(void);
    void testDirectCallsEncRTP(void);
    void testDirectCallsTIPCTSMedia(void);
    void testDirectCallsTIPTandbergMedia(void);    
    void testDirectCallsDupImpairment(void);
    void testDirectCalls33PctLossImpairment(void);
    void testDirectCalls50PctLossImpairment(void);
    void testDirectCallWithRefresh(void);
    void testDirectCallWithGracefulStop(void);
    void testRegistrationRetries(void);
    void testRegistration(void);
    void testTipRegistration(void);
};

///////////////////////////////////////////////////////////////////////////////

ACE_Time_Value TestSipApplication::MockNotifier::NO_BLOCK(ACE_Time_Value::zero);

///////////////////////////////////////////////////////////////////////////////
void TestSipApplication::testDirectCallsEncRTP(void)
{
    // Start application
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
    SipApplication app(1);
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Activate, &app, boost::ref(*scheduler.AppReactor()), &asyncHandler, voipMediaManager.get());
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure two blocks with one UA each
    SipApplication::uaConfigVec_t configs;
    SipApplication::handleList_t handles;
    
    std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().Load.LoadProfile.MaxConnectionsAttempted = 3;
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolProfile.CallType = sip_1_port_server::EnumSipCallType_AUDIO_ONLY;
    config->Config().ProtocolProfile.RtpTrafficType = sip_1_port_server::EnumRtpTrafficType_ENCODED_RTP;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 1;
    config->Config().ProtocolConfig.UaNumStart = 1000;
    config->Config().ProtocolConfig.LocalPort = 4071;
    config->Config().Endpoint.DstPort = 5071;
    config->Config().ProtocolProfile.AudioSrcPort = 50050;
    config->Config().ProtocolProfile.VideoSrcPort = 50052;

    configs.push_back(*config);

    config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolProfile.CallType = sip_1_port_server::EnumSipCallType_AUDIO_ONLY;
    config->Config().ProtocolProfile.RtpTrafficType = sip_1_port_server::EnumRtpTrafficType_ENCODED_RTP;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config->Config().ProtocolConfig.UaNumStart = 2000;
    config->Config().ProtocolConfig.LocalPort = 5071;
    config->Config().Endpoint.DstPort = 4071;
    config->Config().ProtocolProfile.AudioSrcPort = 50060;
    config->Config().ProtocolProfile.VideoSrcPort = 50062;

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

    // Let the calls run for a bit
    {
      unsigned int i=0;
      for(i=0;i<60;i++) {
	sleep(1);
	std::vector<UserAgentRawStats> stats;
	app.GetUserAgentStats(0, handles, stats);
	unsigned int j=0;
	for(j=0;j<stats.size();j++) {
	  if (stats[j].isDirty()) {
	    stats[j].Sync();
	    stats[j].setDirty(false);
	  }
	}
      }
    }

    // Stop calls
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::StopProtocol, &app, 0, SipApplication::handleList_t(1, handles[0]));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Get UA block stats
    std::vector<UserAgentRawStats> stats;
    app.GetUserAgentStats(0, handles, stats);

    _CPPUNIT_ASSERT(stats.size()>0);
    _CPPUNIT_ASSERT(stats[0].vqResultsBlock);
    VQSTREAM_NS::VoiceVQualResultsBlockSnapshot snapshot = stats[0].vqResultsBlock->getVoiceBlockSnapshot();

    _CPPUNIT_ASSERT(stats.size() == 2);
    _CPPUNIT_ASSERT(stats[0].Handle() == handles[0]);
    _CPPUNIT_ASSERT(stats[1].Handle() == handles[1]);

    _CPPUNIT_ASSERT(stats[0].callAttempts == 3);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[0].callSuccesses);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[1].callAnswers);
    _CPPUNIT_ASSERT(stats[0].callFailures == 0);

    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

void TestSipApplication::testDirectCallsTIPCTSMedia(void)
{
    // Start application
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
    SipApplication app(1);
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Activate, &app, boost::ref(*scheduler.AppReactor()), &asyncHandler, voipMediaManager.get());
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure two blocks with one UA each
    SipApplication::uaConfigVec_t configs;
    SipApplication::handleList_t handles;
    
    std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().Load.LoadProfile.MaxConnectionsAttempted = 3;
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolProfile.CallType = sip_1_port_server::EnumSipCallType_TELEPRESENCE;
    config->Config().ProtocolProfile.RtpTrafficType = sip_1_port_server::EnumRtpTrafficType_ENCODED_RTP;
    config->Config().ProtocolProfile.SipMessagesTransport = sip_1_port_server::EnumSipMessagesTransport_TCP;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 1;
    config->Config().ProtocolConfig.UaNumStart = 1000;
    config->Config().ProtocolConfig.LocalPort = 4071;
    config->Config().Endpoint.DstPort = 5071;
    config->Config().ProtocolProfile.AudioSrcPort = 50050;
    config->Config().ProtocolProfile.AudioCodec = sip_1_port_server::EnumSipAudioCodec_AAC_LD;
    config->Config().ProtocolProfile.VideoSrcPort = 50052;
    config->Config().ProtocolProfile.VideoCodec = sip_1_port_server::EnumSipVideoCodec_H_264_720p;
    config->Config().ProtocolProfile.TipDeviceType = sip_1_port_server::EnumTipDevice_CTS500_32;
    config->Config().ProtocolProfile.EnableVoiceStatistic = false;
    config->Config().ProtocolProfile.EnableAudioStatistic = true;
    config->Config().ProtocolProfile.EnableVideoStatistic = true;

    configs.push_back(*config);

    config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolProfile.CallType = sip_1_port_server::EnumSipCallType_TELEPRESENCE;
    config->Config().ProtocolProfile.RtpTrafficType = sip_1_port_server::EnumRtpTrafficType_ENCODED_RTP;
    config->Config().ProtocolProfile.SipMessagesTransport = sip_1_port_server::EnumSipMessagesTransport_TCP;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config->Config().ProtocolConfig.UaNumStart = 2000;
    config->Config().ProtocolConfig.LocalPort = 5071;
    config->Config().Endpoint.DstPort = 4071;
    config->Config().ProtocolProfile.AudioSrcPort = 50060;
    config->Config().ProtocolProfile.AudioCodec = sip_1_port_server::EnumSipAudioCodec_AAC_LD;
    config->Config().ProtocolProfile.VideoSrcPort = 50062;
    config->Config().ProtocolProfile.VideoCodec = sip_1_port_server::EnumSipVideoCodec_H_264_720p;
    config->Config().ProtocolProfile.TipDeviceType = sip_1_port_server::EnumTipDevice_CTS500_32;
    config->Config().ProtocolProfile.EnableVoiceStatistic = false;
    config->Config().ProtocolProfile.EnableAudioStatistic = true;
    config->Config().ProtocolProfile.EnableVideoStatistic = true;


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

    // Let the calls run for a bit
    {
      unsigned int i=0;
      for(i=0;i<60;i++) {
	sleep(1);
	std::vector<UserAgentRawStats> stats;
	app.GetUserAgentStats(0, handles, stats);
	unsigned int j=0;
	for(j=0;j<stats.size();j++) {
	  if (stats[j].isDirty()) {
	    stats[j].Sync();
	    stats[j].setDirty(false);
	  }
	}
      }
    }

    // Stop calls
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::StopProtocol, &app, 0, SipApplication::handleList_t(1, handles[0]));
        scheduler.Enqueue(req);
        req->Wait();
    }

    // Get UA block stats
    std::vector<UserAgentRawStats> stats;
    app.GetUserAgentStats(0, handles, stats);

    _CPPUNIT_ASSERT(stats.size()>0);
    _CPPUNIT_ASSERT(stats[0].vqResultsBlock);
    VQSTREAM_NS::VoiceVQualResultsBlockSnapshot snapshot = stats[0].vqResultsBlock->getVoiceBlockSnapshot();

    _CPPUNIT_ASSERT(stats.size() == 2);
    _CPPUNIT_ASSERT(stats[0].Handle() == handles[0]);
    _CPPUNIT_ASSERT(stats[1].Handle() == handles[1]);

    _CPPUNIT_ASSERT(stats[0].callAttempts == 3);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[0].callSuccesses);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[1].callAnswers);
    _CPPUNIT_ASSERT(stats[0].callFailures == 0);

    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

void TestSipApplication::testDirectCallsTIPTandbergMedia(void)
{
    // Start application
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
    SipApplication app(1);
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Activate, &app, boost::ref(*scheduler.AppReactor()), &asyncHandler, voipMediaManager.get());
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure two blocks with one UA each
    SipApplication::uaConfigVec_t configs;
    SipApplication::handleList_t handles;
    
    std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().Load.LoadProfile.MaxConnectionsAttempted = 3;
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolProfile.CallType = sip_1_port_server::EnumSipCallType_TELEPRESENCE;
    config->Config().ProtocolProfile.RtpTrafficType = sip_1_port_server::EnumRtpTrafficType_ENCODED_RTP;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 1;
    config->Config().ProtocolConfig.UaNumStart = 1000;
    config->Config().ProtocolConfig.LocalPort = 4071;
    config->Config().Endpoint.DstPort = 5071;
    config->Config().ProtocolProfile.AudioSrcPort = 50050;
    config->Config().ProtocolProfile.AudioCodec = sip_1_port_server::EnumSipAudioCodec_G_711;
    config->Config().ProtocolProfile.VideoSrcPort = 50052;
    config->Config().ProtocolProfile.VideoCodec = sip_1_port_server::EnumSipVideoCodec_H_264_1080p;
    config->Config().ProtocolProfile.TipDeviceType = sip_1_port_server::EnumTipDevice_CEX60;
    config->Config().ProtocolProfile.EnableVoiceStatistic = true;
    config->Config().ProtocolProfile.EnableAudioStatistic = false;
    config->Config().ProtocolProfile.EnableVideoStatistic = true;

    configs.push_back(*config);

    config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolProfile.CallType = sip_1_port_server::EnumSipCallType_TELEPRESENCE;
    config->Config().ProtocolProfile.RtpTrafficType = sip_1_port_server::EnumRtpTrafficType_ENCODED_RTP;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config->Config().ProtocolConfig.UaNumStart = 2000;
    config->Config().ProtocolConfig.LocalPort = 5071;
    config->Config().Endpoint.DstPort = 4071;
    config->Config().ProtocolProfile.AudioSrcPort = 50060;
    config->Config().ProtocolProfile.AudioCodec = sip_1_port_server::EnumSipAudioCodec_G_711;
    config->Config().ProtocolProfile.VideoSrcPort = 50062;
    config->Config().ProtocolProfile.VideoCodec = sip_1_port_server::EnumSipVideoCodec_H_264_1080p;
    config->Config().ProtocolProfile.TipDeviceType = sip_1_port_server::EnumTipDevice_CEX60;
    config->Config().ProtocolProfile.EnableVoiceStatistic = true;
    config->Config().ProtocolProfile.EnableAudioStatistic = false;
    config->Config().ProtocolProfile.EnableVideoStatistic = true;


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

    // Let the calls run for a bit
    {
      unsigned int i=0;
      for(i=0;i<60;i++) {
	sleep(1);
	std::vector<UserAgentRawStats> stats;
	app.GetUserAgentStats(0, handles, stats);
	unsigned int j=0;
	for(j=0;j<stats.size();j++) {
	  if (stats[j].isDirty()) {
	    stats[j].Sync();
	    stats[j].setDirty(false);
	  }
	}
      }
    }

    // Stop calls
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::StopProtocol, &app, 0, SipApplication::handleList_t(1, handles[0]));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Get UA block stats
    std::vector<UserAgentRawStats> stats;
    app.GetUserAgentStats(0, handles, stats);

    _CPPUNIT_ASSERT(stats.size()>0);
    _CPPUNIT_ASSERT(stats[0].vqResultsBlock);
    VQSTREAM_NS::VoiceVQualResultsBlockSnapshot snapshot = stats[0].vqResultsBlock->getVoiceBlockSnapshot();

    _CPPUNIT_ASSERT(stats.size() == 2);
    _CPPUNIT_ASSERT(stats[0].Handle() == handles[0]);
    _CPPUNIT_ASSERT(stats[1].Handle() == handles[1]);

    _CPPUNIT_ASSERT(stats[0].callAttempts == 3);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[0].callSuccesses);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[1].callAnswers);
    _CPPUNIT_ASSERT(stats[0].callFailures == 0);

    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}
///////////////////////////////////////////////////////////////////////////////
void TestSipApplication::testDirectCalls(void)
{
    // Start application
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
    SipApplication app(1);
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Activate, &app, boost::ref(*scheduler.AppReactor()), &asyncHandler, voipMediaManager.get());
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure two blocks with one UA each
    SipApplication::uaConfigVec_t configs;
    SipApplication::handleList_t handles;
    
    std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().Load.LoadProfile.MaxConnectionsAttempted = 3;
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 1;
    config->Config().ProtocolConfig.UaNumStart = 1000;
    config->Config().ProtocolConfig.LocalPort = 5071;
    config->Config().Endpoint.DstPort = 4071;
    configs.push_back(*config);

    config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config->Config().ProtocolConfig.UaNumStart = 2000;
    config->Config().ProtocolConfig.LocalPort = 4071;
    config->Config().Endpoint.DstPort = 5071;
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

    // Let the calls run for a bit
    sleep(60);

    // Stop calls
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::StopProtocol, &app, 0, SipApplication::handleList_t(1, handles[0]));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Get UA block stats
    std::vector<UserAgentRawStats> stats;
    app.GetUserAgentStats(0, handles, stats);

    _CPPUNIT_ASSERT(stats.size() == 2);
    _CPPUNIT_ASSERT(stats[0].Handle() == handles[0]);
    _CPPUNIT_ASSERT(stats[1].Handle() == handles[1]);

    _CPPUNIT_ASSERT(stats[0].callAttempts == 3);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[0].callSuccesses);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[1].callAnswers);
    _CPPUNIT_ASSERT(stats[0].callFailures == 0);

    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }

    _CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipApplication::testDirectCallsIpv6(void)
{
    // Start application
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
    SipApplication app(1);
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Activate, &app, boost::ref(*scheduler.AppReactor()), &asyncHandler, voipMediaManager.get());
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure two blocks with one UA each
    SipApplication::uaConfigVec_t configs;
    SipApplication::handleList_t handles;
    
    std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv6();
    config->Config().Load.LoadProfile.MaxConnectionsAttempted = 3;
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 1;
    config->Config().ProtocolConfig.UaNumStart = 1000;
    config->Config().ProtocolConfig.LocalPort = 5071;
    config->Config().Endpoint.DstPort = 5072;
    configs.push_back(*config);

    config = TestUtilities::MakeUserAgentConfigIpv6();
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config->Config().ProtocolConfig.UaNumStart = 2000;
    config->Config().ProtocolConfig.LocalPort = 5072;
    config->Config().Endpoint.DstPort = 5071;
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

    // Let the calls run for a bit
    sleep(60);

    // Stop calls
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::StopProtocol, &app, 0, SipApplication::handleList_t(1, handles[0]));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Get UA block stats
    std::vector<UserAgentRawStats> stats;
    app.GetUserAgentStats(0, handles, stats);

    _CPPUNIT_ASSERT(stats.size() == 2);
    _CPPUNIT_ASSERT(stats[0].Handle() == handles[0]);
    _CPPUNIT_ASSERT(stats[1].Handle() == handles[1]);

    _CPPUNIT_ASSERT(stats[0].callAttempts == 3);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[0].callSuccesses);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[1].callAnswers);
    _CPPUNIT_ASSERT(stats[0].callFailures == 0);

    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }

    _CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipApplication::testDirectCallsIpv6MUAs(void)
{
    // Start application
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
    SipApplication app(1);
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Activate, &app, boost::ref(*scheduler.AppReactor()), &asyncHandler, voipMediaManager.get());
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure two blocks with one UA each
    SipApplication::uaConfigVec_t configs;
    SipApplication::handleList_t handles;
    
    std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv6();
    config->Config().Load.LoadProfile.MaxConnectionsAttempted = 33;
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 1;
    config->Config().ProtocolConfig.UaNumStart = 1000;
    config->Config().ProtocolConfig.LocalPort = 5071;
    config->Config().Endpoint.DstPort = 5072;
    config->Config().ProtocolConfig.UaNumsPerDevice=11;
    configs.push_back(*config);

    config = TestUtilities::MakeUserAgentConfigIpv6();
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config->Config().ProtocolConfig.UaNumStart = 2000;
    config->Config().ProtocolConfig.LocalPort = 5072;
    config->Config().Endpoint.DstPort = 5071;
    config->Config().ProtocolConfig.UaNumsPerDevice=11;
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

    // Let the calls run for a bit
    sleep(60);

    // Stop calls
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::StopProtocol, &app, 0, SipApplication::handleList_t(1, handles[0]));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Get UA block stats
    std::vector<UserAgentRawStats> stats;
    app.GetUserAgentStats(0, handles, stats);

    _CPPUNIT_ASSERT(stats.size() == 2);
    _CPPUNIT_ASSERT(stats[0].Handle() == handles[0]);
    _CPPUNIT_ASSERT(stats[1].Handle() == handles[1]);

    _CPPUNIT_ASSERT(stats[0].callAttempts == 33);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[0].callSuccesses);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[1].callAnswers);
    _CPPUNIT_ASSERT(stats[0].callFailures == 0);

    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }

    _CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipApplication::testDirectCallsIpv6Proxy(void)
{
    // Start application
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
    SipApplication app(1);
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Activate, &app, boost::ref(*scheduler.AppReactor()), &asyncHandler, voipMediaManager.get());
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure two blocks with one UA each
    SipApplication::uaConfigVec_t configs;
    SipApplication::handleList_t handles;
    
    std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv6();
    config->Config().Load.LoadProfile.MaxConnectionsAttempted = 3;
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 1;
    config->Config().ProtocolConfig.UaNumStart = 1000;
    config->Config().ProtocolConfig.LocalPort = 5071;
    config->Config().Endpoint.DstPort = 5072;
    config->Config().ProtocolProfile.UseUaToUaSignaling = false;
    config->Config().ProtocolProfile.ProxyPort = 5072;
    configs.push_back(*config);

    config = TestUtilities::MakeUserAgentConfigIpv6();
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config->Config().ProtocolConfig.UaNumStart = 2000;
    config->Config().ProtocolConfig.LocalPort = 5072;
    config->Config().Endpoint.DstPort = 5071;
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

    // Let the calls run for a bit
    sleep(60);

    // Stop calls
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::StopProtocol, &app, 0, SipApplication::handleList_t(1, handles[0]));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Get UA block stats
    std::vector<UserAgentRawStats> stats;
    app.GetUserAgentStats(0, handles, stats);

    _CPPUNIT_ASSERT(stats.size() == 2);
    _CPPUNIT_ASSERT(stats[0].Handle() == handles[0]);
    _CPPUNIT_ASSERT(stats[1].Handle() == handles[1]);

    _CPPUNIT_ASSERT(stats[0].callAttempts == 3);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[0].callSuccesses);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[1].callAnswers);
    _CPPUNIT_ASSERT(stats[0].callFailures == 0);

    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }

    _CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipApplication::testDirectCallsDupImpairment(void)
{
    UdpSocketHandlerFactory::SetSendDelegate(boost::bind(DupSender(), _1, _2));  // for every packet sent by transaction layer sends 2 copies
    testDirectCalls();
    UdpSocketHandlerFactory::ClearSendDelegate();
}

///////////////////////////////////////////////////////////////////////////////

void TestSipApplication::testDirectCalls33PctLossImpairment(void)
{
    UdpSocketHandlerFactory::SetSendDelegate(boost::bind(LossySender(3), _1, _2));  // for every 3 packets sent by transaction layer: sends 2, drops 1
    testDirectCalls();
    UdpSocketHandlerFactory::ClearSendDelegate();
}

///////////////////////////////////////////////////////////////////////////////

void TestSipApplication::testDirectCallWithRefresh(void)
{
    // Start application
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
    SipApplication app(1);
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Activate, &app, boost::ref(*scheduler.AppReactor()), static_cast<AsyncCompletionHandler *>(0), voipMediaManager.get());
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure two blocks with one UA each
    SipApplication::uaConfigVec_t configs;
    SipApplication::handleList_t handles;
    
    std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().Load.LoadProfile.MaxConnectionsAttempted = 1;
    config->Config().ProtocolProfile.CallTime = 95;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 1;
    config->Config().ProtocolConfig.UaNumStart = 1000;
    config->Config().ProtocolConfig.LocalPort = 4071;
    config->Config().Endpoint.DstPort = 5071;
    configs.push_back(*config);

    config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().ProtocolProfile.CallTime = 95;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config->Config().ProtocolConfig.UaNumStart = 2000;
    config->Config().ProtocolConfig.LocalPort = 5071;
    config->Config().Endpoint.DstPort = 4071;
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

    // Let the calls run for a bit
    sleep(120);

    // Stop calls
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::StopProtocol, &app, 0, SipApplication::handleList_t(1, handles[0]));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Get UA block stats
    std::vector<UserAgentRawStats> stats;
    app.GetUserAgentStats(0, handles, stats);

    _CPPUNIT_ASSERT(stats.size() == 2);
    _CPPUNIT_ASSERT(stats[0].Handle() == handles[0]);
    _CPPUNIT_ASSERT(stats[1].Handle() == handles[1]);

    _CPPUNIT_ASSERT(stats[0].callAttempts == 1);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[0].callSuccesses);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[1].callAnswers);
    _CPPUNIT_ASSERT(stats[0].callFailures == 0);
    _CPPUNIT_ASSERT(stats[0].sessionRefreshes > 0);
    _CPPUNIT_ASSERT(stats[1].sessionRefreshes == 0);
    
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipApplication::testRegistrationRetries(void)
{
    // Start application
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    CPPUNIT_ASSERT(scheduler.Start() == 0);

    boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> voipMediaManager;
    voipMediaManager.reset(new VoIPMediaManager());
    voipMediaManager->initialize(0);

    // Create mock notifier for ACT dispatch
    MockNotifier mockNotifier(*scheduler.AppReactor());
    AsyncCompletionHandler asyncHandler;
    asyncHandler.SetMethodCompletionDelegate(boost::bind(&MockNotifier::EnqueueNotification, &mockNotifier, _1, _2));

    // Activate application
    SipApplication app(1);
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Activate, &app, boost::ref(*scheduler.AppReactor()), static_cast<AsyncCompletionHandler *>(0), voipMediaManager.get());
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure one UA block with one UA
    SipApplication::uaConfigVec_t configs;
    SipApplication::handleList_t handles;
    
    std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().ProtocolProfile.UseUaToUaSignaling = false;
    config->Config().ProtocolProfile.RegRetries = 2;
    config->Config().ProtocolProfile.ProxyDomain = "proxy.foo.com";
    config->Config().ProtocolProfile.ProxyPort = config->Config().ProtocolConfig.LocalPort+999;
    config->Config().RegistrarProfile.UseProxyAsRegistrar = true;
    configs.push_back(*config);

    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::ConfigUserAgents, &app, 0, boost::cref(configs), boost::ref(handles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(handles.size() == 1);
    CPPUNIT_ASSERT(handles[0] == 1);
    
    // Start registration attempts
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::RegisterUserAgents, &app, 0, boost::cref(handles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    // Let the registration attempts run for a bit
    sleep(200); 

    // Cancel any remaining registration attempts
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::CancelUserAgentsRegistrations, &app, 0, boost::cref(handles));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Get UA block stats
    std::vector<UserAgentRawStats> stats;
    app.GetUserAgentStats(0, handles, stats);

    CPPUNIT_ASSERT(stats.size() == 1);
    CPPUNIT_ASSERT(stats[0].Handle() == handles[0]);

    CPPUNIT_ASSERT(stats[0].regAttempts == 3);
    CPPUNIT_ASSERT(stats[0].regSuccesses == 0);
    CPPUNIT_ASSERT(stats[0].regRetries == 2);
    CPPUNIT_ASSERT(stats[0].regFailures == 1); //if failed, then may be the sleep interval is not enough ?
    
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipApplication::testRegistration(void)
{
    // Start application
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    CPPUNIT_ASSERT(scheduler.Start() == 0);

    boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> voipMediaManager;
    voipMediaManager.reset(new VoIPMediaManager());
    voipMediaManager->initialize(0);

    // Create mock notifier for ACT dispatch
    MockNotifier mockNotifier(*scheduler.AppReactor());
    AsyncCompletionHandler asyncHandler;
    asyncHandler.SetMethodCompletionDelegate(boost::bind(&MockNotifier::EnqueueNotification, &mockNotifier, _1, _2));

    // Activate application
    SipApplication app(1);
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Activate, &app, boost::ref(*scheduler.AppReactor()), static_cast<AsyncCompletionHandler *>(0), voipMediaManager.get());
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure one UA block with one UA
    SipApplication::uaConfigVec_t configs;
    SipApplication::handleList_t handles;
    
    std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().ProtocolProfile.UseUaToUaSignaling = false;
    config->Config().ProtocolProfile.RegRetries = 2;
    config->Config().ProtocolProfile.ProxyDomain = "proxy.foo.com";
    config->Config().ProtocolProfile.ProxyPort = config->Config().ProtocolConfig.LocalPort;
    config->Config().RegistrarProfile.UseProxyAsRegistrar = true;
    configs.push_back(*config);

    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::ConfigUserAgents, &app, 0, boost::cref(configs), boost::ref(handles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(handles.size() == 1);
    CPPUNIT_ASSERT(handles[0] == 1);
    
    // Start registration attempts
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::RegisterUserAgents, &app, 0, boost::cref(handles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    // Let the registration attempts run for a bit
    sleep(10); //we do not need a long sleep interval - we are trying to connect to ourselves and 
    //we are receiving the reply immediately

    // Cancel any remaining registration attempts
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::CancelUserAgentsRegistrations, &app, 0, boost::cref(handles));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Get UA block stats
    std::vector<UserAgentRawStats> stats;
    app.GetUserAgentStats(0, handles, stats);

    CPPUNIT_ASSERT(stats.size() == 1);
    CPPUNIT_ASSERT(stats[0].Handle() == handles[0]);

    CPPUNIT_ASSERT(stats[0].regAttempts == 1);
    CPPUNIT_ASSERT(stats[0].regSuccesses == 1);
    CPPUNIT_ASSERT(stats[0].regRetries == 0);
    CPPUNIT_ASSERT(stats[0].regFailures == 0); 
    
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipApplication::testTipRegistration(void)
{
    // Start application
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    CPPUNIT_ASSERT(scheduler.Start() == 0);

    boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> voipMediaManager;
    voipMediaManager.reset(new VoIPMediaManager());
    voipMediaManager->initialize(0);

    // Create mock notifier for ACT dispatch
    MockNotifier mockNotifier(*scheduler.AppReactor());
    AsyncCompletionHandler asyncHandler;
    asyncHandler.SetMethodCompletionDelegate(boost::bind(&MockNotifier::EnqueueNotification, &mockNotifier, _1, _2));

    // Activate application
    SipApplication app(1);
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Activate, &app, boost::ref(*scheduler.AppReactor()), static_cast<AsyncCompletionHandler *>(0), voipMediaManager.get());
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure one UA block with one UA
    SipApplication::uaConfigVec_t configs;
    SipApplication::handleList_t handles;
    
    std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().ProtocolProfile.UseUaToUaSignaling = false;
    config->Config().ProtocolProfile.RegRetries = 2;
    config->Config().ProtocolProfile.ProxyDomain = "proxy.foo.com";
    config->Config().ProtocolProfile.ProxyPort = config->Config().ProtocolConfig.LocalPort;
    config->Config().RegistrarProfile.UseProxyAsRegistrar = true;
    config->Config().ProtocolProfile.CallType = sip_1_port_server::EnumSipCallType_TELEPRESENCE;
    config->Config().ProtocolProfile.AudioSrcPort = 50050;
    config->Config().ProtocolProfile.AudioCodec = sip_1_port_server::EnumSipAudioCodec_AAC_LD;
    config->Config().ProtocolProfile.VideoSrcPort = 50052;
    config->Config().ProtocolProfile.VideoCodec = sip_1_port_server::EnumSipVideoCodec_H_264_720p;
    config->Config().ProtocolProfile.TipDeviceType = sip_1_port_server::EnumTipDevice_CTS500_32;
    config->Config().ProtocolProfile.EnableVoiceStatistic = false;
    config->Config().ProtocolProfile.EnableAudioStatistic = false;
    config->Config().ProtocolProfile.EnableVideoStatistic = false;

   
    configs.push_back(*config);

    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::ConfigUserAgents, &app, 0, boost::cref(configs), boost::ref(handles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(handles.size() == 1);
    CPPUNIT_ASSERT(handles[0] == 1);
    
    // Start registration attempts
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::RegisterUserAgents, &app, 0, boost::cref(handles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    // Let the registration attempts run for a bit
    sleep(10); //we do not need a long sleep interval - we are trying to connect to ourselves and 
    //we are receiving the reply immediately

    // Cancel any remaining registration attempts
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::CancelUserAgentsRegistrations, &app, 0, boost::cref(handles));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Get UA block stats
    std::vector<UserAgentRawStats> stats;
    app.GetUserAgentStats(0, handles, stats);

    CPPUNIT_ASSERT(stats.size() == 1);
    CPPUNIT_ASSERT(stats[0].Handle() == handles[0]);

    CPPUNIT_ASSERT(stats[0].regAttempts == 1);
    CPPUNIT_ASSERT(stats[0].regSuccesses == 1);
    CPPUNIT_ASSERT(stats[0].regRetries == 0);
    CPPUNIT_ASSERT(stats[0].regFailures == 0); 
    
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}
///////////////////////////////////////////////////////////////////////////////

void TestSipApplication::testDirectCallWithGracefulStop(void)
{
    // Start application
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
    SipApplication app(1);
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
    config->Config().Load.LoadPhases[0].FlatDescriptor.Height = 1;
    config->Config().Load.LoadPhases[0].FlatDescriptor.RampTime = 0;
    config->Config().Load.LoadPhases[0].FlatDescriptor.SteadyTime = 10;
    config->Config().ProtocolProfile.CallTime = 15;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 1;
    config->Config().ProtocolConfig.UaNumStart = 1000;
    config->Config().ProtocolConfig.LocalPort = 5071;
    config->Config().Endpoint.DstPort = 4071;
    configs.push_back(*config);

    config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().ProtocolProfile.CallTime = 15;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config->Config().ProtocolConfig.UaNumStart = 2000;
    config->Config().ProtocolConfig.LocalPort = 4071;
    config->Config().Endpoint.DstPort = 5071;
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

    // Let the calls run for a bit
    sleep(60);

    // Stop calls
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::StopProtocol, &app, 0, SipApplication::handleList_t(1, handles[0]));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Get UA block stats
    std::vector<UserAgentRawStats> stats;
    app.GetUserAgentStats(0, handles, stats);

    _CPPUNIT_ASSERT(stats.size() == 2);
    _CPPUNIT_ASSERT(stats[0].Handle() == handles[0]);
    _CPPUNIT_ASSERT(stats[1].Handle() == handles[1]);

    _CPPUNIT_ASSERT(stats[0].callAttempts == 1);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[0].callSuccesses);
    _CPPUNIT_ASSERT(stats[0].callAttempts == stats[1].callAnswers);
    _CPPUNIT_ASSERT(stats[0].callFailures == 0);
    _CPPUNIT_ASSERT(stats[0].rxResponseCode2XXCount == 2);

    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }

    _CPPUNIT_ASSERT(scheduler.Stop() == 0);
}
///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestSipApplication);
