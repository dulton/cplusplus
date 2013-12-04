/// @file
/// @brief SIP channel header.
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef SIPCHANNEL_H_
#define SIPCHANNEL_H_

#include <string>
#include <vector>
#include <map>

#include <ace/Reactor.h>

#include "RvSipHeaders.h"
#include "RvSipUtils.h"
#include "RvSipAka.h"

#include "SipChannelTimer.h"
//#include "EncTIPCommon.h"
#include "EncTIP.h"

#ifdef UNIT_TEST
	class TestStatefulSip;
	class TestSipDynProxy;
#endif

BEGIN_DECL_RV_SIP_ENGINE_NAMESPACE

typedef enum {
  RVSIP_PRIVACY_HEADER_OPTION_NONE = 1,
  RVSIP_PRIVACY_HEADER_OPTION_HEADER = 2,
  RVSIP_PRIVACY_HEADER_OPTION_SESSION = 4,
  RVSIP_PRIVACY_HEADER_OPTION_USER = 8,
  RVSIP_PRIVACY_HEADER_OPTION_CRITICAL = 16
} RVSIP_PRIVACY_HEADER_OPTION;

enum ChannelFunctionalType {
  CFT_UNKNOWN=-1,
  CFT_BASIC=0,
  CFT_DEFAULT=CFT_BASIC,
  CFT_TIP,
  CFT_TOTAL
};

#define NON_TIP_DEVICE_TYPE 0

typedef enum {
  RVSIP_MOBILE_TYPE_NONE=0,
  RVSIP_MOBILE_TYPE_ISIM,
  RVSIP_MOBILE_TYPE_USIM
} RVSIP_MOBILE_TYPE;

typedef enum {
  TERMINATE = 0,
  ORIGINATE = 1
} eRole;

typedef enum {
    RAN_TYPE_3GPP_GERAN=0,
    RAN_TYPE_3GPP_UTRAN_FDD,
    RAN_TYPE_3GPP_UTRAN_TDD
} eRanType;
typedef enum {
    MNC_LEN_2=2,
    MNC_LEN_3
} eMNCLen;

class CallStateNotifier {
 public:
  CallStateNotifier() {}
  virtual ~CallStateNotifier() {}
  virtual void regSuccess() = 0;
  virtual void regFailure(uint32_t regExpires, int regRetries) = 0;
  virtual void inviteCompleted(bool success) = 0;
  virtual void callAnswered(void) = 0;
  virtual void callCompleted(void) = 0;
  virtual void refreshCompleted(void) = 0;
  virtual void responseSent(int msgNum) = 0;
  virtual void responseReceived(int msgNum) = 0;
  virtual void inviteSent(bool reinvite) = 0;
  virtual void inviteReceived(int hopsPerRequest) = 0;
  virtual void callFailed(bool transportError, bool timeoutError, bool serverError) = 0;
  virtual void ackSent(void) = 0;
  virtual void ackReceived(void) = 0;
  virtual void byeSent(bool retrying, ACE_Time_Value *timeDelta) = 0;
  virtual void byeReceived(void) = 0;
  virtual void byeStateDisconnected(ACE_Time_Value *startTime, ACE_Time_Value *endTime) = 0;
  virtual void byeFailed(void) = 0;
  virtual void inviteResponseReceived(bool is180) = 0;
  virtual void inviteAckReceived(ACE_Time_Value *timeDelta) = 0;
  virtual void nonInviteSessionInitiated(void) = 0;
  virtual void nonInviteSessionSuccessful(void) = 0;
  virtual void nonInviteSessionFailed(bool transportError, bool timeoutError) = 0;
};

class MediaInterface {
 public:
  MediaInterface() {}
  virtual ~MediaInterface() {}
  //Originate side:
  virtual RvStatus media_beforeConnect(IN  RvSipCallLegHandle            hCallLeg) = 0; //before we send INVITE
  //Terminate side:
  virtual RvStatus media_onOffering(IN  RvSipCallLegHandle            hCallLeg, RvUint16& code,bool bMandatory=true) = 0; //after we got INVITE
  virtual RvStatus media_beforeAccept(IN  RvSipCallLegHandle            hCallLeg) = 0; //before we send 200OK on INVITE
  //General callbacks:
  virtual RvStatus media_onConnected(IN  RvSipCallLegHandle            hCallLeg, RvUint16 &code,bool parseSdp=true) = 0; //after we got 200OK or we sent 200OK
  virtual RvStatus media_beforeDisconnect(IN  RvSipCallLegHandle            hCallLeg) = 0; //before disconnector send BYE
  virtual RvStatus media_onDisconnecting(IN  RvSipCallLegHandle            hCallLeg) = 0; //after disconnector sent BYE
  virtual RvStatus media_onByeReceived(IN  RvSipCallLegHandle            hCallLeg, bool &hold) = 0; //after BYE received, before 200OK
  virtual RvStatus media_onCallExpired(IN  RvSipCallLegHandle            hCallLeg, bool &hold) = 0; //before BYE sent
  virtual RvStatus media_onDisconnected(IN  RvSipCallLegHandle            hCallLeg) = 0; //both sides, after 200 sent/received
  virtual RvStatus media_stop() = 0; //just stop
  virtual int      media_startTip(uint32_t mux_csrc) = 0;
  virtual RvStatus media_prepareConnect(IN  RvSipCallLegHandle  hCallLeg, RvUint16 &code,RvSipMsgHandle hMsg)=0;
};

class TipStatus{
    public:
        TipStatus();
        ~TipStatus(){}
        void setStatus(TIP_NS::TIPMediaType type,TIP_NS::TIPNegoStatus status,void *arg = NULL);
        TIP_NS::TIPNegoStatus getStatus(TIP_NS::TIPMediaType type);
        bool localNegoDone(TIP_NS::TIPMediaType type);
        bool remoteNegoDone(TIP_NS::TIPMediaType type);
        bool NegoFail();
        bool NegoFail(TIP_NS::TIPMediaType type);
        bool NegoDone();
        void reset();
        bool toSendReInvite() const {return mToSendReInvite;}

    private:
        TIP_NS::TIPNegoStatus mTipStatus[TIP_NS::TIPMediaEnd];
        bool mLocalDone[TIP_NS::TIPMediaEnd];
        bool mRemoteDone[TIP_NS::TIPMediaEnd]; 
        bool mFail[TIP_NS::TIPMediaEnd];
        bool mToSendReInvite;

};




  
class SipChannel {

  friend class SipEngine;
  friend class SipCallSMBasic;
  friend class SipCallSMTip;
  
  #ifdef UNIT_TEST
  	friend class ::TestStatefulSip;
  	friend class ::TestSipDynProxy;
  #endif

  typedef SipChannelTimer* TimerHandle;
  
 public:

  static const unsigned short SIP_DEFAULT_PORT;

  static const uint32_t DEFAULT_TIME_BEFORE_REREGISTRATION;
  static const uint32_t MIN_REREGISTRATION_SESSION_TIME;

  static const uint32_t MANY_UAS_PER_DEVICE_THRESHOLD;
  static const uint32_t BULK_UAS_PER_DEVICE_THRESHOLD;
  static const uint32_t MULTIPLE_UAS_PER_DEVICE_THRESHOLD;
  static const uint32_t LARGE_SIGNALING_SOCKET_SIZE;
  static const uint32_t BULK_SIGNALING_SOCKET_SIZE;
  static const uint32_t NORMAL_SIGNALING_SOCKET_SIZE;
  static const uint32_t MODEST_SIGNALING_SOCKET_SIZE;
  static const uint32_t SMALL_SIGNALING_SOCKET_SIZE;
  static const uint32_t POST_RING_PRACK_RCV_TIMEOUT;
  static const uint32_t POST_CALL_TERMINATE_TIMEOUT;
  static const uint32_t CALL_TIMER_MEDIA_POSTCALL_TIMEOUT;
  static const std::string DEFAULT_NETWORKINFO_ID;
  static const std::string DEFAULT_IMSI;
  static const int       DEFAULT_REG_SUBS_EXPIRES;
  static const uint32_t  CALL_STOP_GRACEFUL_TIME;
  static const uint32_t  DEFAULT_TIP_DEVICE_TYPE;

  static const char* ALLOWED_METHODS;


  void setProcessingType(bool rel100, 
			 bool shortForm,
			 bool sigOnly,
			 bool secureReqURI,
			 bool useHostInCallID,
			 bool anonymous,
			 bool useProxy,
			 uint16_t privacyOptions = 0);

  void setIdentity(const std::string& userName, 
		   const std::string& userNumber, 
		   const std::string& userNumberContext,
		   const std::string& displayName, 
		   bool telUriLocal);
  void setIMSIdentity(const std::string& impu,
           const std::string& impi);

  void setLocalURIDomain(const std::string& localURIDomain,
			 unsigned short localURIDomainPort);
  
  void setContactDomain(const std::string& contactDomain,
			unsigned short contactDomainPort);

  void setSessionExpiration(int sessionExpires, 
			    RvSipSessionExpiresRefresherType rtype); 

  void setMediaInterface(MediaInterface *mediaHandler);
  void clearMediaInterface();

  void setAuth(const std::string& authName, const std::string& authRealm, const std::string& authPasswd, const char* akaOP = NULL);
  void getAuth(std::string &authName, std::string &authRealm, std::string &authPasswd) const;
  std::string getAuthNamePrivate() const;
  std::string getAuthRealmPrivate() const;
  
  bool isFree() const { return mIsFree; }

  ChannelFunctionalType getSMType() const { return mSMType; }
  void setSMType(ChannelFunctionalType smType) { mSMType=smType; }
 
  CallStateNotifier* getCallStateNotifier() const { RVSIPLOCK(mLocker); return mCallStateNotifier; }
  void setCallStateNotifier(CallStateNotifier* callStateNotifier) { RVSIPLOCK(mLocker); mCallStateNotifier=callStateNotifier; }
  void clearCallStateNotifier() { RVSIPLOCK(mLocker); mCallStateNotifier=NULL; }

  RvStatus startRegistration();
  RvStatus startDeregistration();
  RvStatus stopRegistration();
  RvStatus stopSubsCallLegPrivate();

  RvStatus connectSession();
  RvStatus connectSessionOnTimer();
  RvStatus disconnectSession();
  RvStatus stopCallSession(bool bGraceful=false);

  RvStatus setLocal(const RvSipTransportAddr& addr, uint8_t ipServiceLevel, uint32_t uasPerDevice);
  RvStatus setRegistration(const std::string& regName, 
			   uint32_t expires,
			   uint32_t reregSecsBeforeExp,
			   uint32_t regRetries,
			   bool refreshRegistration);

RvStatus setProxy(const std::string& ProxyName, 
				     const std::string& ProxyIP, 
				     const std::string& ProxyDomain,
				     unsigned short ProxyPort, 
				     bool useProxyDiscovery); 
RvStatus setRegistrar(const std::string& regServerName, 
				     const std::string& regServerIP, 
				     const std::string& regDomain,
				     unsigned short regServerPort,
                     bool useRegistrarDiscovery);
  int getHandle() const { RVSIPLOCK(mLocker); return mHandle; }
  uint32_t getRegRetries() const { RVSIPLOCK(mLocker); return mRegRetries; }
  void setRegRetries(uint32_t value) { RVSIPLOCK(mLocker); mRegRetries = value; }
  void setExtraContacts(const std::vector<std::string> &ecs);
  void clearExtraContacts();
  void setRemote(const std::string& userName, 
		 const std::string& userNumberContext,
		 const std::string& displayName,
		 const RvSipTransportAddr& addr,
		 const std::string& hostName,
		 const std::string& remoteDomain,
		 unsigned short remoteDomainPort,
		 bool telUriRemote);
  bool isSigOnly() const { RVSIPLOCK(mLocker); return mSigOnly; }
  bool isOriginate() const { RVSIPLOCK(mLocker); return mRole == ORIGINATE; }
  bool isTerminate() const { RVSIPLOCK(mLocker); return mRole == TERMINATE; }
  eRole getRole() const { RVSIPLOCK(mLocker); return mRole; }
  std::string getLocalIfName() const { RVSIPLOCK(mLocker); return mLocalAddress.if_name; }
  std::string getLocalIP() const { RVSIPLOCK(mLocker); return mLocalAddress.strIP; }
  int getLocalPort() const { RVSIPLOCK(mLocker); return mLocalAddress.port; }
  const RvSipTransportAddr& getLocalAddress() const { RVSIPLOCK(mLocker); return mLocalAddress;}

  const std::string& getLocalUserName() const { RVSIPLOCK(mLocker);return mUseIMSI? mImsi:mUserName; }
  const std::string& getRegName() const { RVSIPLOCK(mLocker); return mRegName; }
  const std::string& getLocalUserNumber() const { RVSIPLOCK(mLocker); return mUserNumber;  }
  const std::string& getLocalUserNumberContext() const { RVSIPLOCK(mLocker); return mUserNumberContext;}
  const std::string& getLocalDisplayName() const { RVSIPLOCK(mLocker); return mDisplayName;}
  const RvSipTransportAddr& getRemoteAddress() const { RVSIPLOCK(mLocker); return mAddressRemote;}
  const std::string& getRemoteUserName() const { RVSIPLOCK(mLocker); return mUserNameRemote;}
  const std::string& getRemoteUserNumberContext() const { RVSIPLOCK(mLocker); return mUserNumberContextRemote;}
  const std::string& getRemoteDisplayName() const { RVSIPLOCK(mLocker); return mDisplayNameRemote;}
  const std::string& getLocalURIDomain() const {  RVSIPLOCK(mLocker);  return mLocalURIDomain; }
  unsigned short getLocalURIDomainPort() const {  RVSIPLOCK(mLocker);  return mLocalURIDomainPort; }
  const std::string& getContactDomain() const {  RVSIPLOCK(mLocker);  return mContactDomain; }
  unsigned short getContactDomainPort() const {  RVSIPLOCK(mLocker);  return mContactDomainPort; }
  const std::string& getRegDomain() const { RVSIPLOCK(mLocker); return mRegDomain; }

  uint32_t getRegExpiresTime() const { RVSIPLOCK(mLocker); return mRegExpiresTime; }
  void setRegExpiresTime(uint32_t value) { RVSIPLOCK(mLocker); mRegExpiresTime = value; }
  uint32_t getReregSecsBeforeExpTime() const { RVSIPLOCK(mLocker); return mReregSecsBeforeExpTime; }
  bool isShortForm() const { return mShortForm; }
  bool isSecureReqURI() const { RVSIPLOCK(mLocker); return mSecureReqURI; }
  bool useHostInCallID() const { RVSIPLOCK(mLocker); return mUseHostInCallID; }
  bool isAnonymous() const { RVSIPLOCK(mLocker); return mAnonymous; }
  uint16_t getPrivacyOptions() const { RVSIPLOCK(mLocker); return mPrivacyOptions; }
  bool isRel100() const { RVSIPLOCK(mLocker); return mRel100; }
  void setRel100(bool v) { RVSIPLOCK(mLocker); mRel100 = v; }
  bool isTelUriRemote() const { RVSIPLOCK(mLocker); return mTelUriRemote; }
  bool isTelUriLocal() const { RVSIPLOCK(mLocker); return mTelUriLocal; }
  void setTelUriRemote(bool v) { RVSIPLOCK(mLocker); mTelUriRemote = v; }
  void setTelUriLocal(bool v) { RVSIPLOCK(mLocker); mTelUriLocal = v; }
  bool useProxy() const { RVSIPLOCK(mLocker); return mUsingProxy && (!mProxyName.empty() || !mProxyIP.empty() || !mProxyDomain.empty()); } 
  bool isDeregistering() const { RVSIPLOCK(mLocker); return mIsDeregistering; }
  bool isEnabled() const { RVSIPLOCK(mLocker); return mIsEnabled; }

  const std::string& getProxyIP() const { RVSIPLOCK(mLocker); return mProxyIP; }
  const std::string& getProxyName() const { RVSIPLOCK(mLocker); return mProxyName; }
  const std::string& getProxyDomain() const { RVSIPLOCK(mLocker); return mProxyDomain; }
  unsigned short getProxyPort() const { RVSIPLOCK(mLocker); return mProxyPort; }
  unsigned short getRemoteDomainPort() const { RVSIPLOCK(mLocker); return mRemoteDomainPort; }
  void setTipDeviceType(uint32_t type) { RVSIPLOCK(mLocker); mTipDeviceType = type; }
  bool isTipDevice(void) const{ RVSIPLOCK(mLocker); return (mTipDeviceType != NON_TIP_DEVICE_TYPE); }

  
  void NotifyInterfaceDisabled(void);
  void NotifyInterfaceEnabled(void);
  
  std::string getTransportStr() const;
  bool isTCP() const;
  std::string getRemoteDomain() const { RVSIPLOCK(mLocker); return mRemoteDomain; }

  RvSipCallLegState getImmediateCallLegState() const;

  uint32_t getCallTime() const { RVSIPLOCK(mLocker); return mCallTime; }
  void setCallTime(uint32_t value) { RVSIPLOCK(mLocker); mCallTime = value; }

  uint32_t getPostCallTime() const { RVSIPLOCK(mLocker); return mPostCallTime; }
  void setPostCallTime(uint32_t value) { RVSIPLOCK(mLocker); mPostCallTime = value; }

  uint32_t getRingTime() const { RVSIPLOCK(mLocker); return mRingTime; }
  void setRingTime(uint32_t value) { RVSIPLOCK(mLocker); mRingTime = value; }

  uint32_t getPrackTime() const { RVSIPLOCK(mLocker); return mPrackTime; }
  void setPrackTime(uint32_t value) { RVSIPLOCK(mLocker); mPrackTime = value; }
  
  bool isInviting() const { RVSIPLOCK(mLocker); return mIsInviting; }
  void setInviting(bool value) { RVSIPLOCK(mLocker); mIsInviting = value; }

  bool isDisconnecting() const { RVSIPLOCK(mLocker); return mIsDisconnecting; }
  void setDisconnecting(bool value) { RVSIPLOCK(mLocker); mIsDisconnecting = value; }

  bool isConnected() const { RVSIPLOCK(mLocker); return mIsConnected; }
  void setConnected(bool value) { RVSIPLOCK(mLocker); mIsConnected = value; }

  HRPOOL getAppPool() const { return mAppPool; }

  void MediaMethodCompletionAsyncNotification(bool generate, bool initiatorParty, bool success);
  void TipStateUpdate(TIP_NS::TIPMediaType type,TIP_NS::TIPNegoStatus status,EncTipHandle handle,void *arg);
  unsigned int generate_mux_csrc(void) const;

  void setAka(bool value) { RVSIPLOCK(mLocker); mUseAka=value; }
  bool useAka() const { RVSIPLOCK(mLocker); return mUseAka; }
  bool useSubOnReg() const { RVSIPLOCK(mLocker); return mSubOnReg; }
  void setSubOnReg(bool value) { RVSIPLOCK(mLocker); mSubOnReg = value; }

  void setMobile(bool value) { RVSIPLOCK(mLocker); mUseMobile=value; }
  bool useMobile() const { RVSIPLOCK(mLocker); return mUseMobile; }
  bool setImsi(const std::string &imsi,eMNCLen mnc_len=MNC_LEN_2);
  void setRanType(eRanType type) {RVSIPLOCK(mLocker);mNetworkAccessType = type;}
  void setRanID(std::string id) {RVSIPLOCK(mLocker);mNetworkAccessCellID = id;}
  void setMNCLen(eMNCLen len) {RVSIPLOCK(mLocker);mMncLen = len;}
  void setImsSubsExpires(int expires){RVSIPLOCK(mLocker); mImsSubsExpires = expires;}
  int getImsSubsExpires(void){RVSIPLOCK(mLocker); return mImsSubsExpires;}

  bool RemoteSDPOffered(void) {RVSIPLOCK(mLocker); return mRemoteSDPOffered;}
  void RemoteSDPOffered(bool offered){RVSIPLOCK(mLocker); mRemoteSDPOffered = offered;}

  bool TipDone(void) {RVSIPLOCK(mLocker); return mTipDone;}
  void TipDone(bool done){RVSIPLOCK(mLocker); mTipDone = done;}

  bool TipStarted(void) {RVSIPLOCK(mLocker); return mTipStarted;}
  void TipStarted(bool started){RVSIPLOCK(mLocker); mTipStarted = started;}
  static void fixIpv6Address(char* s, int len);
  static void fixIpv6Address(std::string& s);
  void regTimerExpirationEventCB();
  void callTimerExpirationEventCB();
  void byeAcceptTimerMediaExpirationEventCB();
  void postCallTimerExpirationEventCB();
  void ringTimerExpirationEventCB();
  void prackTimerExpirationEventCB();
  void callTimerMediaExpirationEventCB();

  void setInitialInviteSent(bool sent) {RVSIPLOCK(mLocker); mInitialInviteSent = sent;}
  bool getInitialInviteSent() {RVSIPLOCK(mLocker); return mInitialInviteSent;}
  void resetInitialInviteSentStatus() {RVSIPLOCK(mLocker); mInitialInviteSent = false;}

  void setInitialByeSent(bool sent) {RVSIPLOCK(mLocker); mInitialByeSent = sent;}
  bool getInitialByeSent() {RVSIPLOCK(mLocker); return mInitialByeSent;}
  void resetInitialByeSentStatus() {RVSIPLOCK(mLocker); mInitialByeSent = false;}

  void markInvite200TxStartTime() {RVSIPLOCK(mLocker); mInvite200TxStartTime = ACE_OS::gettimeofday();}
  void markByeStartTime() {RVSIPLOCK(mLocker); mByeStartTime = ACE_OS::gettimeofday();}
  void markByeEndTime() {RVSIPLOCK(mLocker); mByeEndTime = ACE_OS::gettimeofday();}
  void markAckStartTime() {RVSIPLOCK(mLocker); mAckStartTime = ACE_OS::gettimeofday();}
  ACE_Time_Value *getInvite200TxStartTime() {RVSIPLOCK(mLocker); return &mInvite200TxStartTime;}
  ACE_Time_Value *getByeStartTime() {RVSIPLOCK(mLocker); return &mByeStartTime;}
  ACE_Time_Value *getByeEndTime() {RVSIPLOCK(mLocker); return &mByeEndTime;}
  ACE_Time_Value *getAckStartTime() {RVSIPLOCK(mLocker); return &mAckStartTime;}

 private:

  SipChannel(); 
  virtual ~SipChannel();

  static uint32_t getTimeVariationPrivate();
  static void fixIpv6AddressStrPrivate(std::string &str);
  static void fixUserNamePrivate(std::string& s);


  RvSipTransport getTransportPrivate() const;
  RvStatus UnsubscribeRegEventPrivate(void);
  RvStatus subscribeRegEventPrivate(void);
  RvStatus deregisterOnUnSubsPrivate();

  bool isSecureReqURIPrivate() const { return (mSecureReqURI); }
  bool useAkaPrivate() const { return mUseAka; }
  bool useIMSIPrivate() const { return mUseIMSI; }
  bool useMobilePrivate() const { return mUseMobile; }
  bool useSubOnRegPrivate() const {return mSubOnReg;}
  unsigned short getNextHopPortPrivate() const;
  std::string getContactDomainPrivate() const;
  unsigned short getContactDomainPortPrivate() const;
  std::string getLocalUserNamePrivate(bool useTel) const;
  std::string getRegNamePrivate() const;
  std::string getLocalUserNumberPrivate() const;
  std::string getLocalDisplayNamePrivate() const;
  std::string getDisplayNameRemotePrivate() const;

  bool useProxyPrivate() const { return mUsingProxy && (!mProxyName.empty() || !mProxyIP.empty() || !mProxyDomain.empty()); } 
  bool useProxyDiscoveryPrivate() const { return mUsingProxy && mUseProxyDiscovery; } 
  bool useRegistrarDiscoveryPrivate() const { return mUsingProxy && mUseRegistrarDiscovery; } 
  bool usePeer2PeerPrivate() const {return !mUsingProxy;}
  bool useRegistrarPrivate() const { return (!mRegServerIP.empty() || !mRegServerName.empty() || !mRegDomain.empty()) && mHRegMgr; } 

  bool isOriginatePrivate() const { return mRole == ORIGINATE; }
  bool isTerminatePrivate() const { return mRole == TERMINATE; }

  bool isRegSubs(RvSipSubsHandle hSubs);
  void subsTerminated(RvSipSubsHandle hSubs);
  void subsUnsubscribed(RvSipSubsHandle hSubs);
  void getPreferredIdentityPrivate();

  void set(RvSipCallLegMgrHandle hMgr,
	   RvSipMsgMgrHandle hMsgMgr,
	   RvSipMidMgrHandle hMidMgr,
	   RvSipRegClientMgrHandle hRegMgr,
	   RvSipTransportMgrHandle hTrMgr,
	   RvSipAuthenticatorHandle hAuthMgr,
	   HRPOOL appPool,
	   RvSipSubsMgrHandle hSubsMgr); 

  void setFree(bool value) { mIsFree = value; }

  void setCallTimer();
  void callTimerStopPrivate();

  void setCallTimerMediaPrivate();
  void callTimerMediaStopPrivate();

  void setByeAcceptTimerMediaPrivate();
  void byeAcceptTimerMediaStopPrivate();

  void setPostCallTimerPrivate();

  void setRingTimer();
  void ringTimerStopPrivate();

  void setPrackTimer();
  void prackTimerStop();

  RvStatus setReadyToAccept(bool value,bool bAccept=false);

  RvStatus startReregistrationPrivate();
  void regTimerResetPrivate();
  RvStatus startRegistrationProcessPrivate(uint32_t expires);
  
  RvStatus acceptPrivate();
  RvStatus disconnectPrivate();
  
  RvStatus addLocalAddressPrivate();
  RvStatus removeLocalAddressPrivate();

  RvStatus setRefreshTimer(bool isOriginate);
  void clearRefreshTimerPrivate();

  void setCallLeg(RvSipCallLegHandle cl);
  void setReinviteHandle(RvSipCallLegInviteHandle rh);
  
  RvStatus sessionSetLocalCallLegDataPrivate(std::string& strFrom);
  RvStatus sessionSetRemoteCallLegDataPrivate(std::string& strTo);
  
  RvStatus callLegSessionTimerRefreshAlertEv(RvSipCallLegHandle hCallLeg);
  RvStatus callLegSessionTimerNotificationEv(RvSipCallLegHandle                         hCallLeg,
					     RvSipCallLegSessionTimerNotificationReason eReason);
  
  void regSuccessNotify();
  void regFailureNotify();

  int32_t setNonceCount(const std::string &realm, const std::string &nonce, int32_t nonceCount);
  
  std::string SIP_GetProxyRouteHeaderStringPrivate();

  void callCompletionNotification();
  void callCompletionNotifyPrivate();

  void getLocalContactAddressForGeneralRequestPrivate(char* strLocalContactAddress, size_t size);
  void getLocalContactAddressForRegisterPrivate(char* strLocalContactAddress, size_t size);

  void stopAllTimers();
  void clearBasicData();

  RvStatus addPreferredIdentity(void *msg = NULL);
  RvStatus addNetworkAccessInfoToMsg(void *msg);
  RvStatus addUserAgentInfo(void *msg);
  RvStatus setOutboundCallLegMsg();
  RvStatus sessionSetCallLegPrivacy(void *msg = NULL);
  RvStatus sessionSetCallLegAllowPrivate();
  RvStatus sessionSetRegClientAllow();
  RvStatus sessionSetAllowPrivate(RvSipMsgHandle hMsg);
  RvStatus getDynamicRoutesPrivate(std::string &route, std::string &serviceRoutes);
  RvStatus addDynamicRoutes(RvSipCallLegHandle hCallLeg = 0);
  RvStatus addDynamicRoutesToSubscribePrivate(RvSipCallLegHandle hCallLeg);
  RvStatus addDynamicRoutes(RvSipMsgHandle hMsg);
  RvStatus addDynamicRoutes(RvSipMsgHandle hMsg, const char* s);
  bool containsRoutesPrivate(RvSipMsgHandle hMsg);
  void setOutboundCfgPrivate(bool isReg,RvSipTransportOutboundProxyCfg &cfg);
  void getNextHopPrivate(bool isReg,std::string &next_hop,uint16_t &port,bool &isAddr);

  std::string getRemoteDomainStrPrivate() const;
  std::string getOurDomainStrPrivate() const;
  void getFromStrPrivate(std::string domain,std::string &from);
  RvStatus addSdp();
  void setLocalCallLegData();

  //Data part ...

  //Type Of Channel (Basic, TIP, etc)
  ChannelFunctionalType mSMType;

  const RvSipUtils::Locker mLocker; //public const data to be used for external locking
  SipChannelTimerContainer      mTimers; //timers container

  //Status of the object in the pool

  volatile bool mIsFree; //is free ?

  //Basic call modifiers

  eRole	mRole;				// Originate / terminate
  bool mRel100; //use reliable provisional responses ?
  bool mSigOnly; //no media ?
  volatile bool mShortForm; //use short abbreviated form for SIP message headers ?
  uint32_t mUasPerDevice; //for optimization purposes: number of UAs per device for this particular user block
  bool mSecureReqURI; //use secure request URI ?
  bool mUseHostInCallID; //use IP/Host name in CallID
  bool mAnonymous; //Use "Anonymous" as display name
  uint16_t mPrivacyOptions; //options for Privacy header
  std::string mPreferredIdentity;

  //Basic call life time values

  uint32_t mCallTime; //in seconds
  uint32_t mRingTime; //in seconds
  uint32_t mPrackTime; //in seconds
  uint32_t mPostCallTime; // !!! in milliseconds !!!

  //Basic call statuses

  bool mIsInviting;
  bool mIsDisconnecting;
  bool mIsConnected;
  bool mReadyToAccept;
  bool mInitialInviteSent;  // whether the initial invite has been sent.
  bool mInitialByeSent;     // whether the initial bye has been sent

  //Local UA data:

  int mHandle; //abstract "socket" handle (not the real socket)
  bool mIsEnabled; //is local addr set ?
  RvSipTransportAddr mLocalAddress; // Local address
  std::string    mInfMacAddress; // Mac address of interface.
  RvSipTransport mTransport; //transport used
  uint8_t mIpServiceLevel; //type of service on the local socket
  std::string    mUserName;	// Local UserName
  std::string    mUserNumber;	// Local UserNumber
  std::string    mUserNumberContext;	// Local UserNumberContext
  std::string    mDisplayName;	// Local display name
  std::string    mContactDomain; //Contact SIP domain name
  unsigned short mContactDomainPort; //Contact SIP domain port
  std::string    mLocalURIDomain; //Local UA URI SIP domain name
  unsigned short mLocalURIDomainPort; //LocalUA URI SIP domain port

  //Remote UA data:

  RvSipTransportAddr mAddressRemote; // Remote address
  std::string    mUserNameRemote;	// Remote UserName
  std::string    mHostNameRemote;	// Remote HostName
  std::string    mUserNumberContextRemote;	// Remote UserNumberContext
  std::string    mDisplayNameRemote;	// Remote display name
  std::string    mRemoteDomain; //Remote SIP domain name
  unsigned short mRemoteDomainPort; //Port in remote domain
  bool mTelUriRemote; //use TelUri for remote party ?

  //Proxy

  std::string    mProxyIP; //Proxy IP 
  std::string    mProxyName; //... or DNS name
  std::string    mProxyDomain; //... or DNS domain name
  unsigned short mProxyPort; //Proxy port
  bool mUseProxyDiscovery; //discovery Proxy according to the proxy or remote domain
  bool mUsingProxy;

  //Registration:

  std::string    mRegDomain; //Registration domain
  std::string    mRegName;	// Registration Name
  std::string    mRegServerName;	// Registration HostName
  std::string    mRegServerIP;	// Registration Host IP
  unsigned short mRegServerPort; //Port on registration server
  uint32_t  mRegExpiresTime; //Registration expiration time, in secs
  bool mRefreshRegistration; //refresh the expiring registration ?
  uint32_t  mReregSecsBeforeExpTime; //when we are going to re-register, in secs
  bool mTelUriLocal; //use tel uri for the registration request ?
  std::vector<std::string> mExtraContacts; //extra contact addresses
  std::string mServiceRoute; //service route we obtained from Registrar
  std::string mPath; //path we obtained from Registrar
  bool mIsDeregistering; //is the current reg request for de-registering
  bool mIsInRegTimer; //are we in this thread "inside" the registration timer expiration call
  uint32_t mRegRetries; //number of reg retries configured
  bool mIsRegistering; //have we started registration ?
  bool mUseRegistrarDiscovery; //discovery Registrar according to the registrar domain


  //Authentication:

  std::string    mAuthName; //Authentication name
  std::string    mAuthRealm; //Authentication realm
  std::string    mAuthPasswd; //Authentication password
  bool           mUseAka;     //Use AKA Authentication
  bool           mUseMobile;     //Use Mobile headers
  RvSipUtils::RvSipAka::AKA_Av         mAuthVector;    //AKA Auth data vector
  uint8_t        mOP[AKA_OP_LEN];
  int mAuthTry;

  //Nonce handling:

  typedef std::pair<std::string,std::string> NonceMapKeyType;
  typedef int NonceMapValueType;
  typedef std::map<NonceMapKeyType,NonceMapValueType> NonceMapType;
  NonceMapType mNonceMap;

  //Refresh functionality:

  RvSipSessionExpiresRefresherType mRefresher; //who is refresher
  RvSipCallLegSessionTimerRefresherPreference mRefresherPref; //who is refresher - our preference
  bool mRefresherStarted; //refresher timer started ?
  int mSessionExpiresTime; //our MinSE time, in secs
  bool mRefreshed;//whether endpoint sent the fresh request.

  //Callback objects

  CallStateNotifier* mCallStateNotifier; //object to notify about the call state changes
  MediaInterface *mMediaHandler; //media callback object
  
  // RV data part ...

  RvSipCallLegHandle		mHCallLeg;		// Current Call Leg
  RvSipCallLegInviteHandle  mHReInvite;
  RvSipCallLegHandle		mHSubsCallLeg;		// Current Subscribe Call Leg
  RvSipTranscHandle             mHByeTranscPending;     // Pending incoming BYE transaction
  RvSipCallLegMgrHandle         mHMgr;
  RvSipMsgMgrHandle             mHMsgMgr;
  RvSipTransportMgrHandle       mHTrMgr;
  RvSipMidMgrHandle             mHMidMgr;
  RvSipAuthenticatorHandle      mHAuthMgr;
  HRPOOL                        mAppPool;
  RvSipSubsMgrHandle            mHSubsMgr;
  
  RvSipRegClientMgrHandle       mHRegMgr;
  RvSipRegClientHandle          mHRegClient;

  TimerHandle                   mHConnectTimer;
  TimerHandle                   mHRegTimer;
  TimerHandle                   mHCallTimer;
  TimerHandle                   mHCallTimerMedia;
  TimerHandle                   mHByeAcceptTimerMedia;
  TimerHandle                   mHRingTimer;
  TimerHandle                   mHPrackTimer;
  TipStatus                     mTipStatus;


  //IMS && Mobile
  eRanType  mNetworkAccessType;
  std::string mNetworkAccessCellID;
  std::string mImsi;//USIM IMSI number
  bool mUseIMSI;    //Use IMSI generate sip identities
  std::string mIMPU;//public identity
  std::string mIMPI;//private identity
  int  mImsSubsExpires;//reg subscription expires
  eMNCLen mMncLen;
  RvSipSubsHandle               mHImsSubs;

  //Subscription:
  bool           mSubOnReg;     //enable subscription of reg event.
  bool           mSubsToDeregister; //flag that we have to register on unsubscription

  //Tip
  bool           mTipDone;//whether TIP negotiation finished.
  bool           mRemoteSDPOffered;//whether we know remote SDP info.
  bool           mTipStarted;
  uint32_t       mTipDeviceType;//identify device type to CM for registration/call

  ACE_Time_Value mInvite200TxStartTime;               //< start time for 200 response to INVITE
  ACE_Time_Value mByeStartTime;                       //< start time for BYE TX
  ACE_Time_Value mByeEndTime;                         //< time for received 200OK on BYE TX
  ACE_Time_Value mAckStartTime;                       //< start time for Originating ACK TX
};

END_DECL_RV_SIP_ENGINE_NAMESPACE;

#endif /*SIPCHANNEL_H_*/
