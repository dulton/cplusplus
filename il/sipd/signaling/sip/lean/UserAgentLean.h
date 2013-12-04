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

#ifndef _USER_AGENT_LEAN_H_
#define _USER_AGENT_LEAN_H_

#include "UserAgent.h"

DECL_SIPLEAN_NS

///////////////////////////////////////////////////////////////////////////////

// Forward declarations
class SdpMessage;
class SipMessage;
class SipTransactionLayer;
class UdpSocketHandler;
class UserAgentCallDialog;
class UserAgentRegDialog;

class UserAgentLean : public UserAgent
{
 public:
  
  UserAgentLean(uint16_t port, 
		uint16_t vdevblock,
		size_t index, size_t numUaObjects, size_t indexInDevice, size_t deviceIndex, 
		const std::string& name, const config_t& config, 
		ACE_Reactor& reactor, 
		MEDIA_NS::VoIPMediaManager *voipMediaManager);
  virtual ~UserAgentLean();

  //proprietary methods
  void decRegRetryCount() { if(mRegRetryCount>0) --mRegRetryCount; }
  
  // Config accessors
  virtual bool Enabled(void) const;

  // Registration methods
  virtual uint32_t getRegRetryCount() const { return mRegRetryCount; }
  virtual void setRegRetryCount(uint32_t value) { mRegRetryCount = value; }
  virtual uint32_t GetRegExpiresTime() { return mRegExpires; }
  virtual void SetRegExpiresTime(uint32_t regExpires) { mRegExpires = regExpires ? regExpires : DEFAULT_REG_EXPIRES_TIME; }
  
  // Reset dialogs: registration and call
  virtual void ResetDialogs(void);
  
  // Registration methods
  virtual bool StartRegisterDialog(void);
  virtual void AbortRegistrationDialog(void);
  virtual void StartUnregisterDialog(void);
  
  // Call control methods
  virtual bool StartOutgoingDialog(const ACE_INET_Addr& toAddr, bool isProxy, SipUri remoteUri);
  virtual void AbortCallDialog(bool bGraceful);
  
  // Interface notification methods
  virtual void NotifyInterfaceDisabled(void);
  virtual void NotifyInterfaceEnabled(void);

  virtual const SipUri& ContactAddress(void) const;

  virtual void SetAuthCredentials(const std::string& username, const std::string& password) { 
    mBinding.authCred.username = username; mBinding.authCred.password = password; 
  }
  virtual void SetAuthPasswordDebug(bool debug) { mBinding.authCred.passwordDebug = debug; }
  
 protected:
  
  // Post-ctor config methods
  virtual void setExtraContactAddresses(const std::vector<SipUri> *extraContactAddresses);
  virtual void SetInterfaceTransport(unsigned int ifIndex, const ACE_INET_Addr& addr, int ipServiceLevel, uint32_t uasPerDevice);
  virtual void MediaMethodComplete(bool generate, bool initiatorParty, bool success);

 private:
  
  // Dialog factory methods
  void MakeCallDialog(const ACE_INET_Addr& peer, const SipUri& remoteUri);
  void MakeCallDialog(const SipMessage& msg);
  void MakeRegDialog(void);
  
  // Protocol methods (Tx methods)
  bool SendRequest(SipMessage& msg);
  bool SendResponse(const SipMessage& msg);
  bool SendResponse(uint16_t statusCode, const::string& message, const SipMessage& req);
  
  // Transaction event notifications (Rx methods)
  void RequestMessageHandler(const SipMessage *msg, const std::string& localTag);
  void StatusMessageHandler(const SipMessage *msg, const std::string& localTag);
  
  // Stream control methods
  void BuildSdpMessageBody(SdpMessage& sdp) const;
  bool UpdateStreamConfiguration(const SdpMessage& sdp);
  
  // Dialog notification methods
  void NotifyRegistrationCompletedAndRetry(uint32_t regExpires, bool success);

  //Data

  UserAgentBinding mBinding;                          //< our address binding
  uint32_t mRegRetryCount;                            //< current REGISTER retry count
  uint32_t mRegExpires;                               //< registration expiration time

  boost::scoped_ptr<SipTransactionLayer> mTransLayer; //< transaction layer
  std::tr1::shared_ptr<UdpSocketHandler> mTransport;  //< UDP transport
  
  boost::scoped_ptr<UserAgentCallDialog> mCallDialog; //< call dialog
  boost::scoped_ptr<UserAgentRegDialog> mRegDialog;   //< registration dialog
  
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_SIPLEAN_NS

#endif //_USER_AGENT_LEAN_H_
