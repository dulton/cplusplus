/// @file
/// @brief SIP core "engine" header.
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef SIPENGINE_H_
#define SIPENGINE_H_

#include "RvSipHeaders.h"
#include "RvSipUtils.h"
#include "SipChannel.h"
#include "SipCallSM.h"

#include <set>
#include <vector>

#ifdef UNIT_TEST
	class TestStatefulSip;
	class TestSipDynProxy;
#endif

BEGIN_DECL_RV_SIP_ENGINE_NAMESPACE

class SipChannelFinder {
 public:
  SipChannelFinder() {}
  virtual ~SipChannelFinder() {}
  virtual SipChannel* findSipChannel(const char* userName, int handle) const = 0;
  virtual void put(SipChannel* channel) {}
  virtual void remove(SipChannel* channel) {}
};

class MD5Processor {
 public:
  MD5Processor() {}
  virtual ~MD5Processor() {}
  
  /* Define the state of the MD5 Algorithm. */
  typedef struct {
    uint32_t count[2];	/* message length in bits, lsw first */
    uint32_t abcd[4];		/* digest buffer */
    unsigned char buf[64];		/* accumulate block */
  } state_t;

  virtual void init(state_t *pms) const = 0;

  /* Append a string to the message. */
  virtual void update(state_t *pms, const unsigned char *data, int nbytes) const = 0;

  /* Finish the message and return the digest. */
  virtual void finish(state_t *pms, unsigned char digest[16]) const = 0;
};

class DnsServerCfg{
    public:
        DnsServerCfg();
        DnsServerCfg(int timeout,int tries,int numofservers);
        ~DnsServerCfg(){};
        DnsServerCfg(const DnsServerCfg &cfg);
        const DnsServerCfg & operator =(const DnsServerCfg &cfg);

        int timeout(void) const {return m_timeout;}
        void timeout(int t){m_timeout = t;}
        int tries(void) const {return m_tries;}
        void tries(int t){m_tries = t;}
        int numofservers(void) const {return m_numservers;}
        void numofservers(int n){m_numservers = n;}
    private:
        int m_timeout;
        int m_tries;
        int m_numservers;

};

class SipEngine {
  #ifdef UNIT_TEST
  	friend class ::TestStatefulSip;
  	static bool reliableFlag;
  	friend class ::TestSipDynProxy;
  #endif

 public:
  
  static const char* SIP_DEFAULT_ADDRESS;
  static const char* SIP_DEFAULT_LOOPBACK_ADDRESS;
  static const char* SIP_DEFAULT_LOOPBACK_INTERFACE;
  static const char* SIP_DEFAULT_INTERFACE;
  static const char* SIP_UDP_SUPPORTED;
  static const char* SIP_TCP_SUPPORTED;
  static const char* SIP_REG_SUPPORTED;
  static const int MAX_NUM_OF_DNS_SERVERS;
  static const int MAX_NUM_OF_DNS_DOMAINS;
  
  static SipEngine * getInstance();
  
  static int init(int numOfThreads,int numChannels, 
		  SipChannelFinder& scFinder,
		  const MD5Processor &md5Processor);
  static void destroy();

  static int32_t getMaxPendingChannels();

  SipChannelFinder& getChannelFinder() const { return mScFinder; }

  int run_immediate_events(uint32_t delay);
  
  void appCallLegCreatedEvHandler(
				  IN  RvSipCallLegHandle			hCallLeg,
				  OUT RvSipAppCallLegHandle 	  * phAppCallLeg);
  
  void appStateChangedEvHandler(
				IN  RvSipCallLegHandle            hCallLeg,
				IN  RvSipAppCallLegHandle         hAppCallLeg,
				IN  RvSipCallLegState             eState,
				IN  RvSipCallLegStateChangeReason eReason);
  
  void appPrackStateChangedEvHandler(
				     IN  RvSipCallLegHandle            hCallLeg,
				     IN  RvSipAppCallLegHandle         hAppCallLeg,
				     IN  RvSipCallLegPrackState        eState,
				     IN  RvSipCallLegStateChangeReason eReason,
				     IN  RvInt16                       prackResponseCode);
  
  RV_Status appRegClientStateChangedEvHandler(
					      IN RvSipRegClientHandle            hRegClient,
					      IN RvSipAppRegClientHandle         hAppRegClient,
					      IN  RvSipRegClientState             eNewState,
					      IN  RvSipRegClientStateChangeReason eReason);
  
  RV_Status appCallLegMsgToSendEvHandler(
					 IN RvSipCallLegHandle            hCallLeg,
					 IN RvSipAppCallLegHandle         hAppCallLeg,
					 IN RvSipMsgHandle                hMsg);
  
  RV_Status appRegClientMsgToSendEvHandler(
					   IN RvSipRegClientHandle            hRegClient,
					   IN RvSipAppRegClientHandle         hAppRegClient,
					   IN RvSipMsgHandle                hMsg);

  RV_Status appSubsMsgToSendEvHandler(IN  RvSipSubsHandle          hSubsClient,
				      IN  RvSipAppSubsHandle       hAppSubsClient,
				      IN  RvSipNotifyHandle        hNotify,
				      IN  RvSipAppNotifyHandle     hAppNotify,
				      IN  RvSipMsgHandle           hMsg);

  void appCallLegReInviteCreatedEvHandler(
          IN RvSipCallLegHandle               hCallLeg,
          IN RvSipAppCallLegHandle            hAppCallLeg, 
          IN RvSipCallLegInviteHandle         hReInvite,
          OUT RvSipAppCallLegInviteHandle     *phAppReInvite,
          OUT RvUint16                        *pResponseCode);
  void appCallLegReInviteStateChangedEvHandler(
          IN RvSipCallLegHandle               hCallLeg,
          IN RvSipAppCallLegHandle            hAppCallLeg, 
          IN RvSipCallLegInviteHandle         hReInvite,
          IN RvSipAppCallLegInviteHandle      hAppReInvite,
          IN RvSipCallLegModifyState          eModifyState,
          IN RvSipCallLegStateChangeReason    eReason );
  
  RV_Status appRegClientMsgReceivedEvHandler(
					   IN RvSipRegClientHandle            hRegClient,
					   IN RvSipAppRegClientHandle         hAppRegClient,
					   IN RvSipMsgHandle                hMsg);	
  
  RV_Status appCallLegMsgReceivedEvHandler(
					   IN RvSipCallLegHandle            hCallLeg,
					   IN RvSipAppCallLegHandle         hAppCallLeg,
					   IN RvSipMsgHandle                hMsg);
  
  RvStatus appCallLegTimerRefreshAlertEvHandler(IN    RvSipCallLegHandle       hCallLeg,
						IN    RvSipAppCallLegHandle    hAppCallLeg);
  
  RvStatus appCallLegSessionTimerNotificationEv(
						IN RvSipCallLegHandle                         hCallLeg,
						IN RvSipAppCallLegHandle                      hAppCallLeg,
						IN RvSipCallLegSessionTimerNotificationReason eReason);

  void appMD5AuthenticationExHandler (
				      IN    RvSipAuthenticatorHandle        hAuthenticator,
				      IN    RvSipAppAuthenticatorHandle     hAppAuthenticator,
				      IN  RPOOL_Ptr                     *pRpoolMD5Input,
				      IN  RV_UINT32                     length,
				      OUT RPOOL_Ptr                     *pRpoolMD5Output);

  void appGetSharedSecretAuthenticationHandler(
					       IN    RvSipAuthenticatorHandle       hAuthenticator,
					       IN    RvSipAppAuthenticatorHandle    hAppAuthenticator,
					       IN    void*                          hObject,
					       IN    void*                          peObjectType,
					       IN    RPOOL_Ptr                     *pRpoolRealm,
					       INOUT RPOOL_Ptr                     *pRpoolUserName,
					       OUT   RPOOL_Ptr                     *pRpoolPassword);

  void appAuthenticatorNonceCountUsageEv(
					 IN    RvSipAuthenticatorHandle        hAuthenticator,
					 IN    RvSipAppAuthenticatorHandle     hAppAuthenticator,
					 IN    void*                           hObject,
					 IN    void*                           peObjectType,
					 IN    RvSipAuthenticationHeaderHandle hAuthenticationHeader,
					 INOUT RvInt32*                        pNonceCount);

  void appAuthenticatorMD5EntityBodyEv(
				       IN     RvSipAuthenticatorHandle      hAuthenticator,
				       IN     RvSipAppAuthenticatorHandle   hAppAuthenticator,
				       IN     void*                         hObject,
				       IN     void*                         peObjectType,
				       IN     RvSipMsgHandle                hMsg,
				       OUT    RPOOL_Ptr                     *pRpoolMD5Output,
				       OUT    RvUint32                      *pLength);

  void appByeCreatedEvHandler(
			      IN  RvSipCallLegHandle            hCallLeg,
			      IN  RvSipAppCallLegHandle         hAppCallLeg,
			      IN  RvSipTranscHandle             hTransc,
			      OUT RvSipAppTranscHandle          *hAppTransc,
			      OUT RvBool                       *bAppHandleTransc);
  void appByeStateChangedEvHandler(
				   IN  RvSipCallLegHandle                hCallLeg,
				   IN  RvSipAppCallLegHandle             hAppCallLeg,
				   IN  RvSipTranscHandle                 hTransc,
				   IN  RvSipAppTranscHandle              hAppTransc,
				   IN  RvSipCallLegByeState              eByeState,
				   IN  RvSipTransactionStateChangeReason eReason);
  void appSubsStateChangedEvHandler( IN  RvSipSubsHandle            hSubs,
                                     IN  RvSipAppSubsHandle         hAppSubs,
                                     IN  RvSipSubsState             eState,
                                     IN  RvSipSubsStateChangeReason eReason); 
  void appSubsExpirationAlertEv(IN  RvSipSubsHandle            hSubs,
                                IN  RvSipAppSubsHandle         hAppSubs);
  void appSubsNotifyEvHandler( 
                                     IN  RvSipSubsHandle        hSubs,
                                     IN  RvSipAppSubsHandle     hAppSubs,
                                     IN  RvSipNotifyHandle      hNotification,
                                     IN  RvSipAppNotifyHandle   hAppNotification,
                                     IN  RvSipSubsNotifyStatus  eNotifyStatus,
                                     IN  RvSipSubsNotifyReason  eNotifyReason,
                                     IN  RvSipMsgHandle         hNotifyMsg);

  void appRetransmitEvHandler(IN  RvSipMethodType          method,
                              IN  void                     *owner);
  
  void appRetransmitRcvdEvHandler(IN  RvSipMsgHandle       hMsg,
                              IN  void                     *owner);
 
  SipChannel* newSipChannel(); 
  void releaseChannel(SipChannel* sc);
  size_t getNumberOfBusyChannels() const;

  void setlogmask( char *src,char *filter);
  void setlogmask(void);

  //return number of really added servers, or -1 in the case of error
  int setDNSServers(const std::vector<std::string> &dnsServers, uint16_t vdevblock, bool add);

  //set timeout and number of tries on dns network messages
  RvStatus setDNSTimeout(int timeout, int tries,uint16_t vdevblock);
  
 private:
  
  SipEngine(int numOfThreads, int numChannels, SipChannelFinder& scFinder, const MD5Processor &md5Processor);
  virtual ~SipEngine();

  RV_Status setSupportedTCP(RvSipMsgHandle hMsg);
  RV_Status setSupportedReg(RvSipMsgHandle hMsg);
  
  void setEvHandlers();
  
  void destroySdpStack();
  RvStatus initSdpStack();
  void destroySipStack();
  RvStatus initSipStack(int numOfThreads);

  RvStatus removeContactHeader(RvSipMsgHandle hMsg);
  
  SipCallSM& getCSM(SipChannel* sc) const;

  //Data 
  
  RvSipStackHandle mHSipStack;
  RvSipMidMgrHandle mHMidMgr;

  const RvSipUtils::Locker mLocker;

  HRPOOL mAppPool;
  
  static SipEngine * mInstance;	// instance of the class

  SipCallSM* mStateMachines[CFT_TOTAL];

  int mNumChannels;
  int mAllocatedChannels;

  bool mDnsAltered;
  
  SipChannelFinder& mScFinder;
  const MD5Processor& mMD5Processor;

  typedef SipChannel* SipChannelHandler;
  typedef std::vector<SipChannelHandler> ContainerType;
  typedef std::set<SipChannelHandler> FreeContainerType;
  ContainerType mSipChannelsAll;
  FreeContainerType mSipChannelsAvailable;

  //Logging:
  typedef std::map<std::string,int> LogSrcType;
  typedef std::map<std::string,int> LogFilterType;
  typedef std::map<std::string,int>::iterator MapIterator;

  LogSrcType mLogModuleTypes;
  LogFilterType mLogFilterTypes;

  //DNS CFG
  typedef std::map<RvUint16,DnsServerCfg > DnsCfg;
  typedef std::map<RvUint16,DnsServerCfg >::iterator DnsCfgIterator;

  DnsCfg    mDnsCfgs;
};

END_DECL_RV_SIP_ENGINE_NAMESPACE;

#endif /*SIPENGINE_H_*/
