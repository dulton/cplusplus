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
#include "SipEngine.h"
#include "RvSipHeaders.h"
#include "RvSipUtils.h"

#include "AsyncCompletionHandler.h"
#include "SipApplication.h"
#include "VoIPLog.h"
#include "UserAgentRawStats.h"

#include <sched.h>
#include <utils/md5.h>

USING_VOIP_NS;
USING_SIP_NS;
USING_RVSIP_NS;
USING_RV_SIP_ENGINE_NAMESPACE;
USING_APP_NS;
USING_MEDIA_NS;

///////////////////////////////////////////////////////////////////////////////

class TestStatefulSip : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestStatefulSip);
    CPPUNIT_TEST(testHostInCallIdConfig);
    CPPUNIT_TEST(testHostInCallId);
    CPPUNIT_TEST(testSecureRequestUri);
    CPPUNIT_TEST(testAnonymousConfig);
    CPPUNIT_TEST(testAnonymous);
    CPPUNIT_TEST(testReliable1xxResponseDisabled);
    CPPUNIT_TEST(testReliable1xxResponseEnabled);
    CPPUNIT_TEST(testRegistrationExpireTimeDefault);
    CPPUNIT_TEST(testSessionExpiresConfig);
    CPPUNIT_TEST(testResponseCount);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void)
    {
        ACE_OS::signal(SIGPIPE, SIG_IGN);
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
    
    void testHostInCallIdConfig(void);
		void testHostInCallId(void);
    void testSecureRequestUri(void);
    void testAnonymousConfig(void);
    void testAnonymous(void);
    void testReliable1xxResponseEnabled(void);
    void testReliable1xxResponseDisabled(void);
    void testRegistrationExpireTimeDefault(void);
    void testSessionExpiresConfig(void);
    void testResponseCount(void);

    static const size_t NUM_TEST_UAS = 1;
};
//////////////////////////////////////////////////////////////

ACE_Time_Value TestStatefulSip::MockNotifier::NO_BLOCK(ACE_Time_Value::zero);

//////////////////////////////////////////////////////////////

class MD5ProcessorSipTrial : public MD5Processor {
 public:
  MD5ProcessorSipTrial() {}
  virtual ~MD5ProcessorSipTrial() {}

  virtual void init(state_t *pms) const {
    md5_init((md5_state_t*)pms);
  }
  virtual void update(state_t *pms, const unsigned char *data, int nbytes) const {
    md5_append((md5_state_t*)pms,(const md5_byte_t*)data,nbytes);
  }

  virtual void finish(state_t *pms, unsigned char digest[16]) const {
    md5_finish((md5_state_t*)pms,(md5_byte_t*)digest);
  }
};

class SipTrialMediaInterface : public MediaInterface {
 public:
  SipTrialMediaInterface() {}
  virtual ~SipTrialMediaInterface() {}

    virtual RvStatus media_onConnected(IN  RvSipCallLegHandle            hCallLeg, RvUint16 &code,bool parseSdp=true) {
    return 0;
  }

  virtual RvStatus media_onCallExpired(IN  RvSipCallLegHandle            hCallLeg, bool &hold) {
    hold=false;
    return 0;
  }

  virtual RvStatus media_onDisconnecting(IN  RvSipCallLegHandle            hCallLeg) {
    return 0;
  }
      
  virtual RvStatus media_onDisconnected(IN  RvSipCallLegHandle            hCallLeg) {
    return 0;
  }

  virtual RvStatus media_stop() {
    return 0;
  }

  virtual RvStatus media_beforeConnect(IN  RvSipCallLegHandle            hCallLeg) {
    return 0;
  }

  virtual RvStatus media_onOffering(IN  RvSipCallLegHandle            hCallLeg, RvUint16& code,bool bMandatory=true) {
    return 0;
  }

  virtual RvStatus media_beforeAccept(IN  RvSipCallLegHandle            hCallLeg) {
    return 0;
  }


  virtual RvStatus media_beforeDisconnect(IN  RvSipCallLegHandle            hCallLeg) {
    return 0;
  }

  virtual RvStatus media_onByeReceived(IN  RvSipCallLegHandle            hCallLeg, bool &hold) {
    hold=false;
    return 0;
  }

  virtual int      media_startTip(uint32_t mux_csrc) {
    return 0;
  }
  
  virtual RvStatus media_prepareConnect(IN  RvSipCallLegHandle            hCallLeg, RvUint16 &code,RvSipMsgHandle hMsg) {
    return 0;
  }
};


//=============================================================================
// local types
//=============================================================================

typedef struct __TestOptions
{
public:
  /* get property */
  int       getNumChannels()  { return m_numChannels; }
  int       getCallTime() { return m_callTime; }
  int       getNumCalls()     { return m_numCalls; }
  const char* getBaseIpAddr()   { return m_baseIpAddr; }
  const char* getBaseInterfaceName() {
    return m_ifName;
  }
  const char* getRemoteHostName() {
    return mRemoteHostName;
  }
  const char* getRemoteIP() {
    return mRemoteIP;
  }
  const char* getDNSIP() {
    return mDNSIP;
  }
  const char* getProxy() {
    return mProxy;
  }
  const char* getRemoteDomain() {
    return mRemoteDomain;
  }
  const char* getLocalDomain() {
    return mLocalDomain;
  }
  const char* getRegDomain() {
    return mRegDomain;
  }
  RvSipTransport getSipTransport() const {
    return mTransport;
  }
  bool getRegistrationOnly() {
      return mregistration_only;
  }
  bool getDebug(){
      return mDebug;
  }
  unsigned short     getBaseIpPort()   { return m_baseIpPort; }
  unsigned short     getProxyPort()   { return m_ProxyPort; }

  bool isAllOriginate() { return mOriginate;}
  bool isAllTerminate() { return mTerminate;}
  
  /* set property */
  void setNumChannels(int numChannels)   { m_numChannels = numChannels; }
  void setCallTime(int callTime)   { m_callTime = callTime; }
  void setNumCalls(int numCalls)         { m_numCalls = numCalls; }
  void setBaseIpAddr(const char* baseIpAddr) { strcpy(m_baseIpAddr,baseIpAddr); }
  void setBaseInterfaceName(const char* ifName) { if(ifName) strncpy(m_ifName,ifName,sizeof(m_ifName)-1); }
  void setRemoteHostName(const char* hostName) { if(hostName) strncpy(mRemoteHostName,hostName,sizeof(mRemoteHostName)-1); }
  void setRemoteIP(const char* ip) { if(ip) strncpy(mRemoteIP,ip,sizeof(mRemoteIP)-1); }
  void setDNSIP(const char* ip) { if(ip) strncpy(mDNSIP,ip,sizeof(mDNSIP)-1); }
  void setProxy(const char* proxy) { if(proxy) strncpy(mProxy,proxy,sizeof(mProxy)-1); }
  void setRemoteDomain(const char* domain) { if(domain) strncpy(mRemoteDomain,domain,sizeof(mRemoteDomain)-1); }
  void setLocalDomain(const char* domain) { if(domain) strncpy(mLocalDomain,domain,sizeof(mLocalDomain)-1); }
  void setRegDomain(const char* domain) { if(domain) strncpy(mRegDomain,domain,sizeof(mRegDomain)-1); }
  void setSipTransport(RvSipTransport tr) { mTransport=tr; }
  void setBaseIpPort(unsigned short baseIpPort)   { m_baseIpPort = baseIpPort; }
  void setProxyPort(unsigned short pPort)   { m_ProxyPort = pPort; }
  void setAllOriginate() { mOriginate=true; }
  void setAllTerminate() { mTerminate=true;}
  void setRegistrationOnly(bool rr) { mregistration_only = rr;}
  void setDebug(bool d) { mDebug = d;}

private:
  int 									m_numChannels;
  int 									m_callTime;
  int										m_numCalls;
  char		        			m_baseIpAddr[129];
  char                  m_ifName[65];
  char                  mRemoteHostName[65];
  char                  mRemoteIP[65];
  char                  mDNSIP[65];
  char                  mProxy[65];
  char                  mRemoteDomain[129];
  char                  mLocalDomain[129];
  char                  mRegDomain[129];
  unsigned short        m_baseIpPort;
  unsigned short        m_ProxyPort;
  RvSipTransport        mTransport;
  bool                  mOriginate;
  bool                  mTerminate;
  bool                  mregistration_only;
  bool                  mDebug;
  
} TestOptions;

//=============================================================================
// local data
//=============================================================================

static TestOptions topt;

//=============================================================================
static void set_default_test_options(void)
{
  topt.setNumChannels(0);
  topt.setCallTime(1);
  topt.setNumCalls(-1);
  topt.setBaseIpAddr(SipEngine::SIP_DEFAULT_ADDRESS);
  topt.setBaseInterfaceName("lo");
  topt.setRemoteHostName("");
  topt.setRemoteIP("");
  topt.setDNSIP("");
  topt.setProxy("");
  topt.setRemoteDomain("");
  topt.setLocalDomain("");
  topt.setRegDomain("");
  topt.setSipTransport(RVSIP_TRANSPORT_UNDEFINED);
  topt.setBaseIpPort(4071);
  topt.setProxyPort(4071);
  topt.setRegistrationOnly(false);
  topt.setDebug(false);
}

//=============================================================================

class TrialCallStateNotifier : public CallStateNotifier {
public:
  mutable unsigned int counter;
  TrialCallStateNotifier() {
    counter=0;
  }
  virtual ~TrialCallStateNotifier() {}
  virtual void regSuccess() {
    counter++;
  }
  virtual void regFailure(uint32_t regExpires, int regRetries) {
    if(regRetries<1) counter++;
  }
  virtual void inviteCompleted(bool success) {}
  virtual void callAnswered(void) {}
  virtual void callCompleted(void) {}
  virtual void refreshCompleted(void) {}
  virtual void responseSent(int msgNum) {}
  virtual void responseReceived(int msgNum) {}
  virtual void inviteSent(bool reinvite){} 
  virtual void inviteReceived(int hopsPerRequest){} 
  virtual void callFailed(bool transportError, bool timeoutError, bool serverError){}
  virtual void ackSent(void){}
  virtual void ackReceived(void) {}
  virtual void byeSent(bool retrying, ACE_Time_Value *timeDelta){}
  virtual void byeReceived(void){}
  virtual void byeStateDisconnected(ACE_Time_Value *startTime, ACE_Time_Value *endTime){}
  virtual void byeFailed(void){}
  virtual void inviteResponseReceived(bool is180){}
  virtual void inviteAckReceived(ACE_Time_Value *timeDelta){}
  virtual void nonInviteSessionInitiated(void){}
  virtual void nonInviteSessionSuccessful(void){}
  virtual void nonInviteSessionFailed(bool transportError, bool timeoutError) {}
};

static   TrialCallStateNotifier rn;

static int reg(SipEngine * se, SipChannel **scv, int numChannels)
{
  ENTER();

  int cct = 0;

  if(topt.getProxy()[0]==0 && topt.getRegDomain()[0] == 0) 
      return 0;

  rn.counter=0;

  for (cct = 0; cct < numChannels; cct++) {
    if (scv[cct]->startRegistration() < 0)
      RETCODE(-1);
  }

#ifdef QUICK_TEST
  int regTimeLeft = 3000;
#else
  int regTimeLeft = 300000;
#endif
  int sleepInterval=10;

  do {

    se->run_immediate_events(1);

    usleep(sleepInterval * 1000);
    regTimeLeft-=sleepInterval;
	
  } while (regTimeLeft > 0 && rn.counter<(unsigned int)numChannels);
  
  RETCODE(0);
}

static int unreg(SipEngine * se, SipChannel **scv, int numChannels)
{
  ENTER();

  int cct = 0;

  if(topt.getProxy()[0]==0) return 0;

  rn.counter=0;

  for (cct = 0; cct < numChannels; cct++) {
    if (scv[cct]->startDeregistration() < 0)
      RETCODE(-1);
  }

#ifdef QUICK_TEST
  int regTimeLeft = 3000;
#else
  int regTimeLeft = 30000;
#endif
  int sleepInterval=10;

  do {

    se->run_immediate_events(1);

    usleep(sleepInterval * 1000);
    regTimeLeft-=sleepInterval;
	
  } while (regTimeLeft > 0 && rn.counter<(unsigned int)numChannels);
  
  RETCODE(0);
}

//=============================================================================

class SipChannelFinderTrial : public SipChannelFinder {
 public:
  SipChannelFinderTrial(SipChannel **s, int n) : scv(s), numChannels(n) {}
  virtual ~SipChannelFinderTrial() {}
  virtual SipChannel* findSipChannel(const char* strUserName, int handle) const {

    int cct=0;

    if (strstr(strUserName, "stc_") == strUserName) {
      cct = atoi(strUserName+strlen("stc_"));
    } else if (strstr(strUserName, "stco_") == strUserName) {
      cct = atoi(strUserName+strlen("stco_"));
    } else if (strstr(strUserName, "stct_") == strUserName) {
      cct = atoi(strUserName+strlen("stct_"));
    } else {
      cct = atoi(strUserName);
    }

    if(topt.isAllTerminate()) cct-=1000;

    if (cct<0 || cct >= numChannels) return NULL;
    
    return scv[cct];
  }
private:
  SipChannel **scv;
  int numChannels;
};

//=============================================================================
//////////////////////////////////////////////////////////////

void TestStatefulSip::testHostInCallIdConfig(void) {
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
	config->Config().ProtocolProfile.CallIDDomainName = true;
	
  _CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));
    
  SipEngine * se = SipEngine::getInstance();
  SipChannel * sc = se->mSipChannelsAll.back();

	_CPPUNIT_ASSERT(sc->useHostInCallID() == true);
	
	config->Config().ProtocolConfig.UaNumsPerDevice = NUM_TEST_UAS;
	config->Config().ProtocolProfile.CallIDDomainName = false;
	
  _CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));
    
  sc = se->mSipChannelsAll.back();
    
  _CPPUNIT_ASSERT(sc->useHostInCallID() == false);
}

//////////////////////////////////////////////////////////////

void TestStatefulSip::testHostInCallId(void) 
{
	uint16_t vdevblock=17;
  
  // 1. Set default options
  set_default_test_options();

	// 2. set options: siptrial -N 1 -T UDP
	topt.setNumCalls(1);
	topt.setSipTransport(RVSIP_TRANSPORT_UDP);
  
  int numChannels = topt.getNumChannels();

  topt.setNumChannels(2);
  numChannels = topt.getNumChannels();

  int callTime = topt.getCallTime();

  // 3. Create SIP Channel objects
  char userName[65];
  char userNumber[65];
  const char *userNumberContext="";

  SipChannel * scv[numChannels];
  int port=topt.getBaseIpPort();

  SipChannelFinderTrial scFinder(scv,numChannels);
  MD5ProcessorSipTrial md5Processor;

  SipEngine::init(10,1024,scFinder,md5Processor);
  
  SipEngine * se = SipEngine::getInstance();
  
  if (se == NULL)
    return;
  if(topt.getDebug())
      se->setlogmask();

  SipTrialMediaInterface mediaHandler;

  bool telUriRemote=false;
  bool telUriLocal=false;

  int cct = 0;

  for (cct = numChannels-1; cct>=0; cct--) {

    std::string displayName;

    sprintf(userNumber,"0%u",cct);

    sprintf (userName, "stc_%u", cct); 
    userNumberContext="sunnyvale.uk";
    
    const char* baseipaddr = topt.getBaseIpAddr();

    RvSipTransportAddr addrDetails;
    const char* ip = baseipaddr;
    addrDetails.eAddrType = (strstr(ip,":") ? RVSIP_TRANSPORT_ADDRESS_TYPE_IP6 : RVSIP_TRANSPORT_ADDRESS_TYPE_IP);
    addrDetails.eTransportType = topt.getSipTransport();
    addrDetails.port = port;
    strcpy(addrDetails.strIP,ip);
    strcpy((char*)addrDetails.if_name,(const char*)(topt.getBaseInterfaceName()));
    addrDetails.vdevblock=vdevblock;
    
    scv[cct] = se->newSipChannel();
 
    if (scv[cct] == NULL)
      return;

		bool hostInCallId = false;

    scv[cct]->setProcessingType(true, 	// Reliable 100
    							false,	// Short form
    							false, 	// Sig only
    							true, 	// Secure request uri
    							hostInCallId, 	// Use host in call ID
    							true, 	// Anonymous
                                false,
				RVSIP_PRIVACY_HEADER_OPTION_HEADER | RVSIP_PRIVACY_HEADER_OPTION_SESSION | RVSIP_PRIVACY_HEADER_OPTION_CRITICAL);

    scv[cct]->setIdentity(userName, userNumber, userNumberContext, displayName, telUriLocal);

    scv[cct]->setContactDomain(topt.getLocalDomain(), addrDetails.port);

    scv[cct]->setSessionExpiration(90, RVSIP_SESSION_EXPIRES_REFRESHER_UAS);
   
    scv[cct]->setMediaInterface(&mediaHandler);

    scv[cct]->setCallStateNotifier(&rn);

    scv[cct]->setRingTime(3);

    {
      int rv = (int)(scv[cct]->setLocal(addrDetails, 192, numChannels));
      if(rv < 0)
			return;
    }
  }

  for (cct = 0; cct < numChannels; cct++) {
    int cctRemote = -1;
    char remoteName[33];
    std::string remoteContext;

    RvSipTransportAddr addrDetails;
    addrDetails.eTransportType = topt.getSipTransport();
    addrDetails.if_name[0]=0;
    addrDetails.vdevblock=23;

    sprintf (userName, "stc_%u", cct); remoteContext="sunnyvale.uk";

    std::string displayNameRemote;
    
    cctRemote = (cct&1) ? cct-1 : cct+1;
    sprintf(remoteName,"stc_%d",cctRemote);
    displayNameRemote=scv[cctRemote]->getLocalDisplayName();
    memcpy(&addrDetails,&scv[cctRemote]->getLocalAddress(),sizeof(addrDetails));
    addrDetails.eTransportType = topt.getSipTransport();

    char authName[33];
    char authPwd[33];
    sprintf(authName,"User%d",cct+1);
    sprintf(authPwd,"User%d",cct+1);

    scv[cct]->setAuth(authName,"",authPwd);

    scv[cct]->setRegistrar("",topt.getProxy(),topt.getRegDomain(),topt.getProxyPort(),false);
    scv[cct]->setProxy("",topt.getProxy(),topt.getRemoteDomain(),topt.getProxyPort(),false);

    scv[cct]->setRemote(remoteName,
			  remoteContext,
			  displayNameRemote,
			  addrDetails,
			  topt.getRemoteHostName(),
			  topt.getRemoteDomain(),
			  topt.getProxyPort(),
			  telUriRemote);
   
    scv[cct]->setRegistration(userName,120,77,3,true);

    std::vector<std::string> ecs;
    ecs.push_back(std::string(userName)+std::string(" <sip:")+std::string(userName)+std::string("@1.2.3.4:5678;transport=UDP>"));
    ecs.push_back(std::string(userName)+std::string(" <sip:")+std::string(userName)+std::string("@11.21.31.41:56781;transport=TCP>"));
    scv[cct]->setExtraContacts(ecs);
  }
  
  reg(se,scv,numChannels);
  
	for (cct = 0; cct < numChannels; cct++) {
		eRole role = (cct%2==0) ? ORIGINATE : TERMINATE;
		if(role==ORIGINATE) {
		  scv[cct]->setPostCallTime(100);
		  scv[cct]->setCallTime(callTime);
		  if (scv[cct]->connectSession() < 0) {
			return;
		  }
		}
	}
	  	
	// Grab Call ID to test if host is in Call ID
	RvChar strCallId[1024];
	RvInt32 size = (RvInt32)0;
		
	RvSipCallLegGetCallId(scv[0]->mHCallLeg, sizeof(strCallId)-1, strCallId, &size);
	// There should not be an @ sign in the Call ID
	_CPPUNIT_ASSERT(strstr(strCallId, "@") == NULL);
	  
	int numSessions = 0;

  int callTimeLeft = callTime * 1000;
  int sleepInterval=10;

	do {
		se->run_immediate_events(0);
		numSessions = 0;

		for (cct = 0; cct < numChannels; cct++) {
			RvSipCallLegState eState = scv[cct]->getImmediateCallLegState();
			if(eState != RVSIP_CALL_LEG_STATE_TERMINATED && eState !=RVSIP_CALL_LEG_STATE_UNDEFINED) {
				numSessions++;
			} else {
					scv[cct]->stopCallSession();
			}
		}

	usleep(sleepInterval * 1000);
	callTimeLeft-=sleepInterval;

		if(callTimeLeft<-130000) {
			for (cct = 0; cct < numChannels; cct++) {
				RvSipCallLegState eState = scv[cct]->getImmediateCallLegState();
				if(eState == RVSIP_CALL_LEG_STATE_CONNECTED && scv[cct]->isOriginate())
					scv[cct]->disconnectSession();
			}
			numSessions=0;
		}
  } while (numSessions > 0);

	{
		int i=0;
		do {
			se->run_immediate_events(0);
			usleep(sleepInterval*1000);
		} while(i++<100);
	}
  
  
	unreg(se,scv,numChannels);
	 
	for (cct = 0; cct < numChannels; cct++) if (scv[cct]) se->releaseChannel(scv[cct]);

	SipEngine::destroy();
}

//////////////////////////////////////////////////////////////

void TestStatefulSip::testSecureRequestUri(void) {
	boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> voipMediaManager;
  voipMediaManager.reset(new MEDIA_NS::VoIPMediaManager());
  voipMediaManager->initialize(0);

  boost::scoped_ptr<SIP_NS::UserAgentFactory> userAgentFactory;
  userAgentFactory.reset(new SIP_NS::UserAgentFactory(*TSS_Reactor,voipMediaManager.get()));
  userAgentFactory->activate();
    
  boost::scoped_ptr<UserAgentBlock> uaBlock;
    
  std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
  config->Config().ProtocolConfig.UaNumsPerDevice = NUM_TEST_UAS;
	config->Config().ProtocolProfile.SecureRequestURI = true;
	config->Config().ProtocolProfile.SipMessagesTransport = sip_1_port_server::EnumSipMessagesTransport_UDP;
	
  _CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));
    
  SipEngine * se = SipEngine::getInstance();
    
  SipChannel * sc = se->mSipChannelsAll.back();

	_CPPUNIT_ASSERT(sc->isSecureReqURI() == true);
	_CPPUNIT_ASSERT(sc->isTCP() == false);
	
    std::string strExternalContactAddress;
	
    sc->getFromStrPrivate(sc->getOurDomainStrPrivate(),strExternalContactAddress);
	
	_CPPUNIT_ASSERT(strstr(strExternalContactAddress.c_str(), "transport") == NULL);
	
	config->Config().ProtocolConfig.UaNumsPerDevice = NUM_TEST_UAS;
	config->Config().ProtocolProfile.SecureRequestURI = false;
	config->Config().ProtocolProfile.SipMessagesTransport = sip_1_port_server::EnumSipMessagesTransport_TCP;
	
  _CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));
    
  sc = se->mSipChannelsAll.back();
	
  _CPPUNIT_ASSERT(sc->isSecureReqURI() == false);
	_CPPUNIT_ASSERT(sc->isTCP() == true);
	
    sc->getFromStrPrivate(sc->getOurDomainStrPrivate(),strExternalContactAddress);
	
	_CPPUNIT_ASSERT(strstr(strExternalContactAddress.c_str(), "transport") != NULL);
}

//////////////////////////////////////////////////////////////

void TestStatefulSip::testAnonymousConfig(void) {
	boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> voipMediaManager;
  voipMediaManager.reset(new MEDIA_NS::VoIPMediaManager());
  voipMediaManager->initialize(0);

  boost::scoped_ptr<SIP_NS::UserAgentFactory> userAgentFactory;
  userAgentFactory.reset(new SIP_NS::UserAgentFactory(*TSS_Reactor,voipMediaManager.get()));
  userAgentFactory->activate();
    
  boost::scoped_ptr<UserAgentBlock> uaBlock;
    
  std::auto_ptr<UserAgentConfig> config = TestUtilities::MakeUserAgentConfigIpv4();
    
  // Test anonymous call disabled
	config->Config().ProtocolProfile.AnonymousCall = false;
	
  _CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));
    
  SipEngine * se = SipEngine::getInstance();
    
  SipChannel * sc = se->mSipChannelsAll.back();

	_CPPUNIT_ASSERT(sc->isAnonymous() == false);
	
	// Test anonymous call enabled
	config->Config().ProtocolProfile.AnonymousCall = true;
	
  _CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));
    
  sc = se->mSipChannelsAll.back();
	
	_CPPUNIT_ASSERT(sc->isAnonymous() == true);
}

//////////////////////////////////////////////////////////////

void TestStatefulSip::testAnonymous(void) 
{
	boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> voipMediaManager;
  voipMediaManager.reset(new MEDIA_NS::VoIPMediaManager());
  voipMediaManager->initialize(0);

  boost::scoped_ptr<SIP_NS::UserAgentFactory> userAgentFactory;
  userAgentFactory.reset(new SIP_NS::UserAgentFactory(*TSS_Reactor,voipMediaManager.get()));
  userAgentFactory->activate();
    
  SipEngine * se = SipEngine::getInstance();
    
  SipChannel * sc = se->newSipChannel();
    
  char userName[65];
	char userNumber[65];
	const char *userNumberContext="stc.org";
	std::string displayName;
  bool anonymous = true;

	sprintf(userName, "stc"); 
	sprintf(userNumber, "%d", 0);

	sc->setProcessingType(	true, 	// Rel100
							false, 	// Short form
							false, 	// Sig only
							true, 	// Secure Req URI
							true, 	// Use Host In Call Id
							anonymous, 	// Anonymous
                            false,
							RVSIP_PRIVACY_HEADER_OPTION_HEADER | 
							RVSIP_PRIVACY_HEADER_OPTION_SESSION | 
							RVSIP_PRIVACY_HEADER_OPTION_CRITICAL);
	
	sc->setIdentity(userName, userNumber, userNumberContext, displayName, false);

	_CPPUNIT_ASSERT(sc->isAnonymous() == true);
	
	// Test anonymous option on local
	RvSipTransportAddr addr;
	memset(&addr,0,sizeof(addr));
	
	char ifName[65];
	uint16_t vdevblock=17;
	
	// Set address details for local
	addr.port = 4071;
	strcpy(addr.strIP, "127.0.0.1");
	addr.eTransportType = RVSIP_TRANSPORT_UDP;
	addr.eAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_IP;
	strncpy(ifName, SipEngine::SIP_DEFAULT_LOOPBACK_INTERFACE, sizeof(addr.if_name)-1);
	strcpy((char*)addr.if_name, (const char*)(ifName));
	addr.vdevblock = vdevblock;
	
	sc->setLocal(addr, 192, 2);
	
	std::string strFrom;
	
	// Create call leg
	RvSipCallLegMgrCreateCallLeg(sc->mHMgr, (RvSipAppCallLegHandle)sc, &sc->mHCallLeg, 0, 0xFF);
	_CPPUNIT_ASSERT(sc->sessionSetLocalCallLegDataPrivate(strFrom) >= 0);
	_CPPUNIT_ASSERT(strstr(strFrom.c_str(), "Anonymous") != NULL);
	
	// Test customization of local
	anonymous=false; 
	sc->setProcessingType(	true, 	// Rel100
							false, 	// Short form
							false, 	// Sig only
							true, 	// Secure Req URI
							true, 	// Use Host In Call Id
							anonymous, 	// Anonymous
                            false,
							RVSIP_PRIVACY_HEADER_OPTION_HEADER | 
							RVSIP_PRIVACY_HEADER_OPTION_SESSION | 
							RVSIP_PRIVACY_HEADER_OPTION_CRITICAL);
	
	_CPPUNIT_ASSERT(sc->sessionSetLocalCallLegDataPrivate(strFrom) >= 0);
	_CPPUNIT_ASSERT(strstr(strFrom.c_str(), "Anonymous") == NULL);
}

//////////////////////////////////////////////////////////////

void TestStatefulSip::testReliable1xxResponseDisabled(void) {

  SipCallSM::reliableFlag = false;	// Reset value
	
  // Start application
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    SipApplication app(1);
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
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 1;
    config->Config().ProtocolConfig.UaNumStart = 1000;
    config->Config().ProtocolConfig.LocalPort = 5071;
    config->Config().Endpoint.DstPort = 4071;
    config->Config().ProtocolProfile.ReliableProvisonalResponse = false;
    configs.push_back(*config);
    
    config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config->Config().ProtocolConfig.UaNumStart = 2000;
    config->Config().ProtocolConfig.LocalPort = 4071;
    config->Config().Endpoint.DstPort = 0;
    config->Config().ProtocolProfile.ReliableProvisonalResponse = false;
    
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
#ifdef QUICK_TEST
    sleep(5);
#else
    sleep(60);
#endif
	
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
    _CPPUNIT_ASSERT(SipCallSM::reliableFlag == false);

    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }

    _CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

//////////////////////////////////////////////////////////////

void TestStatefulSip::testReliable1xxResponseEnabled(void) {

  SipCallSM::reliableFlag = false;	// Reset value
	
  // Start application   
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    SipApplication app(1);
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
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 1;
    config->Config().ProtocolConfig.UaNumStart = 1000;
    config->Config().ProtocolConfig.LocalPort = 4071;
    config->Config().Endpoint.DstPort = 5071;
    config->Config().ProtocolProfile.ReliableProvisonalResponse = true;
    configs.push_back(*config);
    
    config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    config->Config().ProtocolConfig.UaNumStart = 2000;
    config->Config().ProtocolConfig.LocalPort = 5071;
    config->Config().Endpoint.DstPort = 0;
    config->Config().ProtocolProfile.ReliableProvisonalResponse = true;
    
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
#ifdef QUICK_TEST
    sleep(5);
#else
    sleep(60);
#endif
    
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
    _CPPUNIT_ASSERT(SipCallSM::reliableFlag == true);

    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }

    _CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

//////////////////////////////////////////////////////////////

void TestStatefulSip::testRegistrationExpireTimeDefault(void) 
{
		boost::scoped_ptr<MEDIA_NS::VoIPMediaManager> voipMediaManager;
    voipMediaManager.reset(new MEDIA_NS::VoIPMediaManager());
    voipMediaManager->initialize(0);

    boost::scoped_ptr<SIP_NS::UserAgentFactory> userAgentFactory;
    userAgentFactory.reset(new SIP_NS::UserAgentFactory(*TSS_Reactor,voipMediaManager.get()));
    userAgentFactory->activate();
    
    SipEngine * se = SipEngine::getInstance();
    
    SipChannel * sc = se->newSipChannel();
    
    // Test default value
		_CPPUNIT_ASSERT(sc->getRegExpiresTime() == 3600);
}

//////////////////////////////////////////////////////////////

void TestStatefulSip::testSessionExpiresConfig(void) {
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
		config->Config().ProtocolProfile.MinSessionExpiration = 1800;
	
    _CPPUNIT_ASSERT_NO_THROW(uaBlock.reset(new UserAgentBlock(0, 0, *config, *TSS_Reactor, 0, *userAgentFactory.get())));
    
    SipEngine * se = SipEngine::getInstance();
    
    SipChannel * sc = se->mSipChannelsAll.back();

		_CPPUNIT_ASSERT(sc->mSessionExpiresTime == 1800);
}

//////////////////////////////////////////////////////////////

void TestStatefulSip::testResponseCount(void) {
		// Start application
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    SipApplication app(1);
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
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 1;
    config->Config().ProtocolConfig.UaNumStart = 1000;
    config->Config().ProtocolConfig.LocalPort = 4071;
    config->Config().Endpoint.DstPort = 5071;
    configs.push_back(*config);
    config = TestUtilities::MakeUserAgentConfigIpv4();
    config->Config().ProtocolProfile.CallTime = 10;
    config->Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    
    // Set different UA number than 2000 to generate 404
    config->Config().ProtocolConfig.UaNumStart = 200;
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
	
    // Let the calls run for a bit
#ifdef QUICK_TEST
    sleep(5);
#else
    sleep(60);
#endif

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
    _CPPUNIT_ASSERT(stats[0].callSuccesses == 0);
    _CPPUNIT_ASSERT(stats[1].callAnswers == 0);
    _CPPUNIT_ASSERT(stats[0].callFailures == 1);
		_CPPUNIT_ASSERT(stats[0].rxResponseCode4XXCount == 1);

    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &SipApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }

    _CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestStatefulSip);
