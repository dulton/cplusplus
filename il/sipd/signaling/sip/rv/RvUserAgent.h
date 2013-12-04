/// @file
/// @brief SIP User Agent Radvision-based (UA) object
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _RVUSER_AGENT_H_
#define _RVUSER_AGENT_H_

#include <boost/function.hpp>
#include <boost/tuple/tuple.hpp>

#include "UserAgent.h"
#include "SipEngine.h"

DECL_RVSIP_NS

///////////////////////////////////////////////////////////////////////////////

class RvUserAgent : public UserAgent, 
  public virtual RV_SIP_ENGINE_NAMESPACE::CallStateNotifier, 
  public virtual RV_SIP_ENGINE_NAMESPACE::MediaInterface
{
  public:

  typedef boost::function0<bool> notification_t;
  typedef boost::function1<void, const notification_t&> run_notification_t;
  typedef boost::tuple<uint16_t,unsigned int,notification_t> lpcommand_t;
  typedef boost::function1<void, const lpcommand_t&> run_lpcommand_t;

  RvUserAgent(uint16_t port, 
	      uint16_t vdevblock,
	      size_t index, size_t numUaObjects, size_t indexInDevice, size_t deviceIndex, 
	      const std::string& name, 
	      const std::string& imsi, 
	      const std::string& ranid, 
          const config_t& config, 
	      ACE_Reactor& reactor, 
	      MEDIA_NS::VoIPMediaManager *voipMediaManager, 
	      const run_notification_t& notificator,
	      const run_lpcommand_t& lpnotificator);
  virtual ~RvUserAgent();
  
  //Virtual public:
  
  virtual bool Enabled(void) const;

  // Registration methods
  virtual uint32_t GetRegExpiresTime();
  virtual void SetRegExpiresTime(uint32_t regExpires);
  virtual uint32_t getRegRetryCount() const;
  virtual void setRegRetryCount(uint32_t value);
  
  // Reset dialogs: registration and call
  virtual void ResetDialogs(void);
  
  // Registration methods
  virtual bool StartRegisterDialog(void);
  virtual void AbortRegistrationDialog(void);
  virtual void StartUnregisterDialog(void);
  
  // Call control methods
  virtual bool StartOutgoingDialog(const ACE_INET_Addr& toAddr, bool isProxy, SipUri remoteUri);
  virtual void AbortCallDialog(bool bGraceful = false);
  
  // Interface notification methods
  virtual void NotifyInterfaceDisabled(void);
  virtual void NotifyInterfaceEnabled(void);

  bool NotifyInviteCompletedTop(bool success) const;
  bool NotifyInviteAckReceivedTop(ACE_Time_Value *timeDelta) const;
  bool NotifyByeStateDisconnectedTop(ACE_Time_Value *timeDelta) const;
  bool NotifyByeSentTop(bool retrying, ACE_Time_Value *timeDelta) const;

  virtual const SipUri& ContactAddress(void) const;

  virtual void SetAuthCredentials(const std::string& username, const std::string& password);
  virtual void SetAuthPasswordDebug(bool debug);

  //Call State notifier:
  virtual void regSuccess();
  virtual void regFailure(uint32_t regExpires, int regRetries);
  virtual void inviteCompleted(bool success);
  virtual void callAnswered(void);
  virtual void callCompleted(void);
  virtual void refreshCompleted(void);
  virtual void responseSent(int msgNum);
  virtual void responseReceived(int msgNum);
  virtual void inviteSent(bool reinvite);
  virtual void inviteReceived(int hopsPerRequest);
  virtual void callFailed(bool transportError, bool timeoutError, bool serverError);
  virtual void ackSent(void);
  virtual void ackReceived(void);
  virtual void byeSent(bool retrying, ACE_Time_Value *timeDelta);
  virtual void byeReceived(void);
  virtual void inviteResponseReceived(bool is180);
  virtual void inviteAckReceived(ACE_Time_Value *timeDelta);
  virtual void byeFailed(void);
  virtual void byeStateDisconnected(ACE_Time_Value *startTime, ACE_Time_Value *endTime);
  virtual void nonInviteSessionInitiated(void);
  virtual void nonInviteSessionSuccessful(void);
  virtual void nonInviteSessionFailed(bool transportError, bool timeoutError);
  
  //Media control mehods
  virtual RvStatus media_beforeConnect(IN  RvSipCallLegHandle            hCallLeg);
  virtual RvStatus media_onOffering(IN  RvSipCallLegHandle            hCallLeg, RvUint16& code,bool bMandatory=true);
  virtual RvStatus media_beforeAccept(IN  RvSipCallLegHandle            hCallLeg);
  virtual RvStatus media_onConnected(IN  RvSipCallLegHandle            hCallLeg, RvUint16 &code,bool parseSdp=true);
  virtual RvStatus media_onCallExpired(IN  RvSipCallLegHandle            hCallLeg, bool &hold);
  virtual RvStatus media_beforeDisconnect(IN  RvSipCallLegHandle            hCallLeg);
  virtual RvStatus media_onByeReceived(IN  RvSipCallLegHandle            hCallLeg, bool &hold);
  virtual RvStatus media_onDisconnecting(IN  RvSipCallLegHandle            hCallLeg);
  virtual RvStatus media_onDisconnected(IN  RvSipCallLegHandle            hCallLeg);
  virtual RvStatus media_stop();
  virtual int media_startTip(uint32_t mux_csrc);
  virtual RvStatus media_prepareConnect(IN  RvSipCallLegHandle hCallLeg, RvUint16 &code,RvSipMsgHandle hMsg);

  static int32_t getPendingChannels() { return mPendingChannels; }
  static void clearPendingChannels() { mPendingChannels=0; }
  
 protected:
  
  //Virtual protected:
  virtual void setExtraContactAddresses(const std::vector<SipUri> *extraContactAddresses);
  virtual void SetInterfaceTransport(unsigned int ifIndex, const ACE_INET_Addr& addr, int ipServiceLevel, uint32_t uasPerDevice);
  virtual void MediaMethodComplete(bool generate, bool initiatorParty, bool success);

  bool realStartOutgoingDialog(const ACE_INET_Addr& addr, bool isProxy, SipUri remoteUri);

  bool isActive() const { return mActive; }
  void setActive(bool value) { mActive=value; }
  int  TipNegoStatusUpdate(TIPMediaType type,TIPNegoStatus status,EncTipHandle handle,void *arg);
  int TipStateUpdate(TIPMediaType type,TIPNegoStatus status,EncTipHandle handle,void *arg);

  RvSdpStatus SDP_InitSdpMsg(RvSdpMsg* sdpMsg);
  RvSdpStatus SDP_FillSdpMsg(RvSdpMsg* sdpMsg);
  const char* SDP_GetSdpAudioEncodingName(Sip_1_port_server::EnumSipAudioCodec codec, int &payloadType, int &samplingRate, unsigned char dynPlType);
  const char* SDP_GetSdpVideoEncodingName(Sip_1_port_server::EnumSipVideoCodec codec, int &payloadType, int &samplingRate);
  RvStatus SDP_addSdpToOutgoingMsg(IN  RvSipCallLegHandle            hCallLeg);
  
  RvSdpConnection* SDP_getAddrFromConnDescr(RvSdpConnection *conn_descr, char* remoteIP, int remoteIPLen);
  RvSdpStatus SDP_readSdpFromMsg(IN  RvSipCallLegHandle            hCallLeg, RvSipMsgHandle SipMsg,
			     bool &audioFound, int audioPayloadType, 
			     bool &videoFound, int videoPayloadType, 
			     char* remoteIP, int remoteIPLen,
			     int &audioPort, int &videoPort,bool &hasSdp);
  RvStatus SDP_parseRemoteSdp(IN  RvSipCallLegHandle hCallLeg, RvUint16& code,RvSipMsgHandle hMsg,bool bMandatory=true);
  RV_Status SDP_MsgGetSdp (IN RvSipMsgHandle hMsg, char *sdp, RvUint32 sdpSize, RvUint32 *len);

  void adjustIPv6ForSdp(char* s1, const char* s2);

 private:

  bool mActive; //check whether the channel is disabled; it is used to disable channels already scheduled for the start 
                //when we want to stop the test

  mutable volatile bool mIsPending; 
  static int32_t mPendingChannels;                    //<number of pending channels (invite incompleted)

  SipUri mContactAddress;
  RV_SIP_ENGINE_NAMESPACE::SipChannel* mChannel;
  const run_notification_t& mNotificator;
  const run_lpcommand_t& mLPNotificator;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_RVSIP_NS

#endif //_RVUSER_AGENT_H_
