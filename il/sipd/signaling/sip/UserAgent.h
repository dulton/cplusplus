/// @file
/// @brief SIP User Agent (UA) object
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _USER_AGENT_H_
#define _USER_AGENT_H_

#include <stdint.h>

#include <string>
#include <vector>

#include <ace/INET_Addr.h>
#include <ace/Time_Value.h>
#include <ace/Reactor.h>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>
#include <Tr1Adapter.h>

#include "SipCommon.h"
#include "UserAgentBinding.h"
#include "VoIPMediaChannel.h"

DECL_CLASS_FORWARD_MEDIA_NS(VoIPMediaManager);

#ifdef UNIT_TEST
class TestUserAgentBlock;
class TestStatefulSip;
class TestSipDynProxy;
#endif

DECL_SIP_NS

///////////////////////////////////////////////////////////////////////////////

class UserAgent : boost::noncopyable {

 public:
  enum StatusNotification {
    STATUS_REGISTRATION_SUCCEEDED,
    STATUS_REGISTRATION_RETRYING,
    STATUS_REGISTRATION_FAILED,
    STATUS_CALL_SUCCEEDED,
    STATUS_CALL_FAILED,
    STATUS_CALL_ANSWERED,
    STATUS_CALL_COMPLETED,
    STATUS_REFRESH_COMPLETED,
    STATUS_RESPONSE1XX_SENT,
    STATUS_RESPONSE2XX_SENT,
    STATUS_RESPONSE3XX_SENT,    
    STATUS_RESPONSE4XX_SENT,
    STATUS_RESPONSE5XX_SENT,
    STATUS_RESPONSE6XX_SENT,
    STATUS_RESPONSE1XX_RECEIVED,
    STATUS_RESPONSE2XX_RECEIVED,
    STATUS_RESPONSE3XX_RECEIVED,    
    STATUS_RESPONSE4XX_RECEIVED,
    STATUS_RESPONSE5XX_RECEIVED,
    STATUS_RESPONSE6XX_RECEIVED,
    STATUS_INVITE_SENT,
    STATUS_INVITE_RECEIVED,
    STATUS_INVITE_RETRYING,
    STATUS_CALL_FAILED_TIMER_B,
    STATUS_CALL_FAILED_TRANSPORT_ERROR,
    STATUS_CALL_FAILED_5XX,
    STATUS_ACK_SENT,
    STATUS_ACK_RECEIVED,
    STATUS_BYE_SENT,
    STATUS_BYE_RECEIVED,
    STATUS_BYE_RETRYING,
    STATUS_BYE_DISCONNECTED,
    STATUS_BYE_FAILED,
    STATUS_INVITE_180_RESPONSE_RECEIVED,
    STATUS_INVITE_1XX_RESPONSE_RECEIVED,
    STATUS_INVITE_ACK_RECEIVED,
    STATUS_NONINVITE_SESSION_INITIATED,
    STATUS_NONINVITE_SESSION_SUCCESSFUL,
    STATUS_NONINVITE_SESSION_FAILED,
    STATUS_NONINVITE_SESSION_FAILED_TIMER_F,
    STATUS_NONINVITE_SESSION_FAILED_TRANSPORT,
    STATUS_UNKNOWN
  };
  
  typedef Sip_1_port_server::SipUaConfig_t config_t;
  typedef boost::function3<void, size_t, StatusNotification, const ACE_Time_Value&> statusDelegate_t;
  
  UserAgent(uint16_t port, 
	    uint16_t vdevblock,
	    size_t index, size_t numUaObjects, size_t indexInDevice, size_t deviceIndex, 
	    const std::string& name, 
        const config_t& config, 
	    ACE_Reactor& reactor, 
	    MEDIA_NS::VoIPMediaManager *voipMediaManager,const std::string& imsi="",const std::string& ranid="");
  virtual ~UserAgent();
  
  // Config accessors
  uint16_t Port(void) const { return mPort; }
  uint16_t VDevBlock(void) const { return mVDevBlock; }
  size_t Index(void) const { return mIndex; }
  size_t DeviceIndex(void) const { return mDeviceIndex; }
  unsigned int IfIndex(void) const { return mIfIndex; }
  
  // Post-ctor config methods
  void SetStatusDelegate(statusDelegate_t delegate) { mStatusDelegate = delegate; }
  void SetInterface(unsigned int ifIndex, const ACE_INET_Addr& addr, size_t uasPerDevice);
  void SetCalleeUriScheme(SipUri::Scheme scheme) { mCalleeUriScheme = scheme; }
  SipUri::Scheme GetCalleeUriScheme() { return mCalleeUriScheme; }
  void SetAudioStreamIndexes(uint32_t rtpStreamIndex, uint32_t rtcpStreamIndex) { 
    if(mMediaChannel) {
      mMediaChannel->setFPGAStreamIndex(MEDIA_NS::AUDIO_STREAM,MEDIA_NS::RTP_STREAM,rtpStreamIndex);
      mMediaChannel->setFPGAStreamIndex(MEDIA_NS::AUDIO_STREAM,MEDIA_NS::RTCP_STREAM,rtcpStreamIndex); 
    }
  }
  void SetAudioHDStreamIndexes(uint32_t rtpStreamIndex, uint32_t rtcpStreamIndex) { 
    if(mMediaChannel) {
      mMediaChannel->setFPGAStreamIndex(MEDIA_NS::AUDIOHD_STREAM,MEDIA_NS::RTP_STREAM,rtpStreamIndex);
      mMediaChannel->setFPGAStreamIndex(MEDIA_NS::AUDIOHD_STREAM,MEDIA_NS::RTCP_STREAM,rtcpStreamIndex);
    }
  }
  void SetVideoStreamIndexes(uint32_t rtpStreamIndex, uint32_t rtcpStreamIndex) {
    if(mMediaChannel) {
      mMediaChannel->setFPGAStreamIndex(MEDIA_NS::VIDEO_STREAM,MEDIA_NS::RTP_STREAM,rtpStreamIndex);
      mMediaChannel->setFPGAStreamIndex(MEDIA_NS::VIDEO_STREAM,MEDIA_NS::RTCP_STREAM,rtcpStreamIndex); 
    }
  }
  void SetAudioFileStreamIndex(uint32_t fileIndex) {
    if(mMediaChannel) {
      mMediaChannel->setFileIndex(MEDIA_NS::AUDIO_STREAM,fileIndex);
    }
  }
  void SetAudioHDFileStreamIndex(uint32_t fileIndex) {
    if(mMediaChannel) {
      mMediaChannel->setFileIndex(MEDIA_NS::AUDIOHD_STREAM,fileIndex);
    }
  }
  void SetVideoFileStreamIndex(uint32_t fileIndex) {
    if(mMediaChannel) {
      mMediaChannel->setFileIndex(MEDIA_NS::VIDEO_STREAM,fileIndex);
    }
  }
  void SetAudioSourcePort(uint16_t audioSourcePort) { 
    if(mMediaChannel) {
      mMediaChannel->setLocalTransportPort(MEDIA_NS::AUDIO_STREAM,audioSourcePort);
    }
  }
  void SetAudioHDSourcePort(uint16_t audioSourcePort) { 
    if(mMediaChannel) {
      mMediaChannel->setLocalTransportPort(MEDIA_NS::AUDIOHD_STREAM,audioSourcePort);
    }
  }
  void SetVideoSourcePort(uint16_t videoSourcePort) { 
    if(mMediaChannel) mMediaChannel->setLocalTransportPort(MEDIA_NS::VIDEO_STREAM,videoSourcePort);
  }
  
  void RegisterVQStatsHandlers(VQSTREAM_NS::VQualResultsBlockHandler vqResultsBlock);
  
  // Reset registration and call
  void Reset(void);
  
  // Registration methods
  bool Register(const std::vector<SipUri> *extraContactAddresses = 0);
  void AbortRegistration(void);
  void Unregister(void);

  bool IsRegistered(void) const { return mRegistered; }
  bool IsUnregistered(void) const { return !mRegistered; }
  void setRegistered(bool value) { mRegistered = value; }
  
  bool Call(const std::string& toName);
  bool Call(const std::string& toUser,const ACE_INET_Addr& toAddr);
  void AbortCall(bool bGraceful = false);

  void markInviteStartTime() { mInviteStartTime = ACE_OS::gettimeofday(); }
  void markRegStartTime() { mRegStartTime = ACE_OS::gettimeofday(); }
  
  //Pure virtual public:

  virtual bool Enabled(void) const = 0;
  
  // Registration methods
  virtual uint32_t getRegRetryCount() const = 0;
  virtual void setRegRetryCount(uint32_t value) = 0;
  virtual uint32_t GetRegExpiresTime() = 0;
  virtual void SetRegExpiresTime(uint32_t regExpires) = 0;
  
  // Reset dialogs: registration and call
  virtual void ResetDialogs(void) = 0;
  
  // Registration methods
  virtual bool StartRegisterDialog(void) = 0;
  virtual void AbortRegistrationDialog(void) = 0;
  virtual void StartUnregisterDialog(void) = 0;
  
  // Call control methods
  virtual bool StartOutgoingDialog(const ACE_INET_Addr& toAddr, bool isProxy, SipUri remoteUri)=0;
  virtual void AbortCallDialog(bool bGraceful)=0;
  
  // Interface notification methods
  virtual void NotifyInterfaceDisabled(void)=0;
  virtual void NotifyInterfaceEnabled(void)=0;

  virtual const SipUri& ContactAddress(void) const = 0;
  
  virtual void SetAuthCredentials(const std::string& username, const std::string& password) = 0;
  virtual void SetAuthPasswordDebug(bool debug) = 0;

 protected:
  
  void ControlStreamGeneration(bool generate, bool initiatorParty, bool wantComplNotification);
  int  StartTipNegotiation(uint32_t mux_csrc);
  
  // Dialog notification methods
  StatusNotification NotifyRegistrationCompleted(uint32_t regExpires, bool success,int regRetriesCount);
  bool NotifyInviteCompleted(bool success) const;
  bool NotifyCallAnswered(void) const;
  bool NotifyCallCompleted(void) const;
  bool NotifyRefreshCompleted(void) const;
  bool NotifyResponseSent(int msgNum) const;
  bool NotifyResponseReceived(int msgNum) const;
  bool NotifyInviteSent(bool reinvite) const;
  bool NotifyInviteReceived(int hopsPerRequest) const;
  bool NotifyCallFailed(bool transportError, bool timeoutError, bool serverError) const;
  bool NotifyAckSent(void) const;
  bool NotifyAckReceived(void) const;
  bool NotifyByeSent(bool retrying, ACE_Time_Value *timeDelta) const;
  bool NotifyByeReceived(void) const;
  bool NotifyByeFailed(void) const;
  bool NotifyByeStateDisconnected(ACE_Time_Value *startTime, ACE_Time_Value *endTime) const;
  bool NotifyInviteResponseReceived(bool is180) const;
  bool NotifyInviteAckReceived(ACE_Time_Value *timeDelta) const;
  bool NotifyNonInviteSessionInitiated(void) const;
  bool NotifyNonInviteSessionSuccessful(void) const;
  bool NotifyNonInviteSessionFailed(bool timeoutError, bool transportError) const;
  
  //Pure virtual protected:
  
  virtual void setExtraContactAddresses(const std::vector<SipUri> *extraContactAddresses) = 0;
  virtual void SetInterfaceTransport(unsigned int ifIndex, const ACE_INET_Addr& addr, int ipServiceLevel, uint32_t uasPerDevice) = 0;
  virtual void MediaMethodComplete(bool generate, bool initiatorParty, bool success) = 0;
  virtual int  TipStateUpdate(TIPMediaType type,TIPNegoStatus status,EncTipHandle handle,void *arg){return 0;}
  
  // Class data
  static const uint32_t DEFAULT_REG_EXPIRES_TIME;
  static const uint16_t DEFAULT_RTP_PORT;

  bool mRegistered; //status of registration
  const uint16_t mPort;                               //< physical port
  const uint16_t mVDevBlock;                          //< device block ID
  const uint32_t mIndex;                              //< owner's index value in the block
  const uint32_t mTotalBlockSize;                     //< Total number of UAs in the block
  const uint32_t mIndexInDevice;                      //< Index in the device
  const uint32_t mDeviceIndex;                       //< Index of the device
  unsigned int mIfIndex;                              //interface index
  const std::string mName;                            //< our user name/number
  const std::string mImsi;                            //< our IMSI
  const std::string mRanID;                              //< our ran ID
  const config_t& mConfig;                            //< UA config, profile, etc.
  ACE_Reactor * const mReactor;                       //< app ACE Reactor instance
  statusDelegate_t mStatusDelegate;                   //< owner's status delegate
  
  const bool mUsingProxy;                             //< when true, using proxy server
  ACE_INET_Addr mProxyAddr;                           //< proxy IP address
  std::string mProxyDomain;                           //< proxy domain

  const bool mUsingProxyAsRegistrar;                  //< when true, using proxy as registrar
  ACE_INET_Addr mRegistrarAddr;                       //< registrar address
  std::string mRegistrarDomain;                       //< registrar domain
  
  SipUri::Scheme mCalleeUriScheme;                    //< URI scheme used to invite callees
  const bool mAudioCall;                              //< when true, sending audio streams
  const bool mAudioHDCall;                            //< when true, sending audioHD streams
  const bool mVideoCall;                              //< when true, sending video streams

	const bool mAudioStat;                              //< when true, audio streams statistics is enabled
  const bool mAudioHDStat;                            //< when true, audioHD streams statistics is enabled
  const bool mVideoStat;                              //< when true, video streams statistics is enabled	
  
  ACE_Time_Value mRegStartTime;                       //< start time for REGISTER transaction
  ACE_Time_Value mInviteStartTime;                    //< start time for INVITE transaction
  
  boost::scoped_ptr<MEDIA_NS::VoIPMediaChannel> mMediaChannel;
  const std::string mRealm;                           //< our realm 
  const uint8_t mRanType;                             //< our RAN type
  MEDIA_NS::VoIPMediaManager *mVoipMediaManager;
  
#ifdef UNIT_TEST
  friend class ::TestUserAgentBlock;
  friend class ::TestStatefulSip;
  friend class ::TestSipDynProxy;
#endif
};

typedef std::tr1::shared_ptr<UserAgent> uaSharedPtr_t;

///////////////////////////////////////////////////////////////////////////////

END_DECL_SIP_NS

#endif //_USER_AGENT_H_
