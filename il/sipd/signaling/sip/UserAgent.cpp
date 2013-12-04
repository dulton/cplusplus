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

#include <algorithm>
#include <cstring>
#include <iterator>
#include <sstream>

#include <ace/INET_Addr.h>
#include <ace/Time_Value.h>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <phxexception/PHXException.h>

#include "VoIPMediaManager.h"
#include "SipUtils.h"
#include "UserAgent.h"
#include "VQStatsData.h"

USING_VOIP_NS;
USING_SIP_NS;
USING_MEDIA_NS;
USING_MEDIAFILES_NS;

///////////////////////////////////////////////////////////////////////////////

const uint32_t UserAgent::DEFAULT_REG_EXPIRES_TIME = 604800;  // 1 week
const uint16_t UserAgent::DEFAULT_RTP_PORT = 50050;

///////////////////////////////////////////////////////////////////////////////

UserAgent::UserAgent(uint16_t port, 
		     uint16_t vdevblock,
		     size_t index, size_t numUaObjects, size_t indexInDevice, size_t deviceIndex, 
		     const std::string& name, 
		     const config_t& config, 
		     ACE_Reactor& reactor, 
		     VoIPMediaManager *voipMediaManager,
		     const std::string& imsi, 
		     const std::string& ranid )
    : mRegistered(false), 
      mPort(port),
      mVDevBlock(vdevblock),
      mIndex(index),
      mTotalBlockSize(numUaObjects),
      mIndexInDevice(indexInDevice),
      mDeviceIndex(deviceIndex),
      mIfIndex(0),
      mName(name),
      mImsi(imsi),
      mRanID(ranid),
      mConfig(config),
      mReactor(&reactor),
      mUsingProxy(!mConfig.ProtocolProfile.UseUaToUaSignaling),
      mUsingProxyAsRegistrar(mConfig.RegistrarProfile.UseProxyAsRegistrar),
      mCalleeUriScheme(SipUri::SIP_URI_SCHEME),
      mAudioCall(mConfig.ProtocolProfile.CallType == sip_1_port_server::EnumSipCallType_AUDIO_ONLY || mConfig.ProtocolProfile.CallType == sip_1_port_server::EnumSipCallType_AUDIO_VIDEO || (mConfig.ProtocolProfile.CallType == sip_1_port_server::EnumSipCallType_TELEPRESENCE && mConfig.ProtocolProfile.AudioCodec != sip_1_port_server::EnumSipAudioCodec_AAC_LD)),
      mAudioHDCall(mConfig.ProtocolProfile.CallType == sip_1_port_server::EnumSipCallType_TELEPRESENCE && mConfig.ProtocolProfile.AudioCodec == sip_1_port_server::EnumSipAudioCodec_AAC_LD),
      mVideoCall(mConfig.ProtocolProfile.CallType == sip_1_port_server::EnumSipCallType_AUDIO_VIDEO || mConfig.ProtocolProfile.CallType == sip_1_port_server::EnumSipCallType_TELEPRESENCE),
			mAudioStat(mConfig.ProtocolProfile.EnableVoiceStatistic),
			mAudioHDStat(mConfig.ProtocolProfile.EnableAudioStatistic),
			mVideoStat(mConfig.ProtocolProfile.EnableVideoStatistic),			
      mRealm(mConfig.ProtocolConfig.RealmName),
      mRanType(mConfig.ProtocolConfig.RanType),
      mVoipMediaManager(voipMediaManager)
{
  if(voipMediaManager) {

    CHANNEL_TYPE channelType = SIMULATED_RTP;
    
    if(mConfig.ProtocolProfile.CallType == sip_1_port_server::EnumSipCallType_SIGNALING_ONLY) {
      channelType = NOP_RTP;
    } else if(mConfig.ProtocolProfile.RtpTrafficType == 
	      sip_1_port_server::EnumRtpTrafficType_ENCODED_RTP) {
      channelType = ENCODED_RTP;
    }

    mMediaChannel.reset(voipMediaManager->getNewMediaChannel(channelType));
    
    if(mMediaChannel) {
      
      if(mAudioCall) mMediaChannel->setLocalTransportPort(MEDIA_NS::AUDIO_STREAM,DEFAULT_RTP_PORT);
      else if(mAudioHDCall) mMediaChannel->setLocalTransportPort(MEDIA_NS::AUDIOHD_STREAM,DEFAULT_RTP_PORT);
      if(mVideoCall) mMediaChannel->setLocalTransportPort(MEDIA_NS::VIDEO_STREAM,DEFAULT_RTP_PORT+2);
      
      if(mAudioCall) {
	if(mVideoCall) mMediaChannel->setCallType(AUDIO_VIDEO);
	else mMediaChannel->setCallType(AUDIO_ONLY);
      } else if(mAudioHDCall) {
	if(mVideoCall) mMediaChannel->setCallType(AUDIOHD_VIDEO);
	else mMediaChannel->setCallType(AUDIOHD_ONLY);
      } else if(mVideoCall) {
	mMediaChannel->setCallType(VIDEO_ONLY);
      } else {
	mMediaChannel->setCallType(SIG_ONLY);
      }

      if(mAudioStat) mMediaChannel->setStatFlag(VOICE_STREAM, true);
      if(mAudioHDStat) mMediaChannel->setStatFlag(AUDIOHD_STREAM, true);
      if(mVideoStat) mMediaChannel->setStatFlag(VIDEO_STREAM, true);			
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

UserAgent::~UserAgent()
{
  ;
}

///////////////////////////////////////////////////////////////////////////////

void UserAgent::Reset(void)
{
    if (!Enabled())
        throw EPHXInternal("UserAgent::Reset");
    
    TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgent::Reset] " << ContactAddress() << " resetting");

    ResetDialogs();

    setRegistered(false);

    mMediaChannel->stopAll(0);
}

/////////////////////////////////////////////////////////////////////////////////

bool UserAgent::Register(const std::vector<SipUri> *extraContactAddresses)
{
    if (!Enabled())
        throw EPHXInternal("UserAgent::Register");
    
    TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgent::Register] " << ContactAddress() << " registering with proxy server");
    
    // If available, include extra contact addresses in our registration request
    setExtraContactAddresses(extraContactAddresses);
    // Reset register retry count
    setRegRetryCount(mConfig.ProtocolProfile.RegRetries);
    setRegistered(false);

    return StartRegisterDialog();
}

/////////////////////////////////////////////////////////////////////////////////////////

void UserAgent::AbortRegistration(void)
{
    if (!Enabled())
        throw EPHXInternal("UserAgent::AbortRegistration");

    TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgent::AbortRegistration] " << ContactAddress() << " aborting registration");

    AbortRegistrationDialog();
}

/////////////////////////////////////////////////////////////////////////////////////////

void UserAgent::Unregister(void)
{
    if (!Enabled())
        throw EPHXInternal("UserAgent::Unregister");

    TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgent::Unregister] " << ContactAddress() << " unregistering with proxy server");
    
    // Zero register retry count so we don't retry
    setRegRetryCount(0);
    setRegistered(false);

    StartUnregisterDialog();
}

/////////////////////////////////////////////////////////////////////////////////

bool UserAgent::Call(const std::string& toUser)
{
    if (!mUsingProxy || !Enabled())
        throw EPHXInternal("UserAgent::Call(toUser)");

    // Build remote URI using sip: or tel: scheme -- it is up to the proxy server to route to remote UA
    SipUri remoteUri;
    switch (mCalleeUriScheme)
    { 
    case SipUri::TEL_URI_SCHEME:
      {
	bool isLocalTel=mConfig.ProtocolProfile.LocalTelUriFormat;
	if(isLocalTel) {
	  std::string phoneContext=mConfig.ProtocolProfile.CalleeNumberContext;
	  remoteUri.SetTelLocal(toUser,phoneContext);
	} else {
	  remoteUri.SetTelGlobal(toUser);
	}
	break;
      }

    case SipUri::SIP_URI_SCHEME:
    default:
      {
	remoteUri.SetSip(toUser, mProxyDomain, mProxyAddr.get_port_number());
	break;
      }
    }

    TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgent::Call] " << ContactAddress() << " calling " << remoteUri);
    
    // Create a new call dialog and start the request
    return StartOutgoingDialog(mProxyAddr, true, remoteUri);
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::Call(const std::string& toUser,const ACE_INET_Addr& toAddr0)
{
  if (!Enabled())
    throw EPHXInternal("UserAgent::Call(toUser, toAddr)");

    ACE_INET_Addr toAddr = toAddr0;

    // Build remote URI using sip: or tel: scheme
    SipUri remoteUri;

    switch (mCalleeUriScheme)
    { 
    case SipUri::TEL_URI_SCHEME:
      {
	bool isLocalTel=mConfig.ProtocolProfile.LocalTelUriFormat;
	if(isLocalTel) {
	  std::string phoneContext=mConfig.ProtocolProfile.CalleeNumberContext;
	  remoteUri.SetTelLocal(toUser,phoneContext);
	} else {
	  remoteUri.SetTelGlobal(toUser);
	}
	break;
      }

    case SipUri::SIP_URI_SCHEME:
    default:
      {
	std::string domain = SipUtils::InetAddressToString(toAddr);
	remoteUri.SetSip(toUser, domain, 
			 toAddr.get_port_number()==DEFAULT_SIP_PORT_NUMBER ? 0 : toAddr.get_port_number());
	break;
      }
    }

    TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgent::Call] " << ContactAddress() << " calling " << remoteUri);

    // Create a new call dialog and start the request
    return StartOutgoingDialog(toAddr, false, remoteUri);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgent::AbortCall(bool bGraceful)
{
    if (!Enabled())
        throw EPHXInternal("UserAgent::AbortCall");

    AbortCallDialog(bGraceful);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgent::RegisterVQStatsHandlers(VQSTREAM_NS::VQualResultsBlockHandler vqResultsBlock) 
{
  if(mMediaChannel && vqResultsBlock) {
    if(mAudioCall) {
      VQSTREAM_NS::VoiceVQualResultsHandler vqHandler=mMediaChannel->getVoiceVQStatsHandler();
      if(vqHandler) {
	vqHandler->block=vqResultsBlock;
      }
    }
    if(mAudioHDCall) {
      VQSTREAM_NS::AudioHDVQualResultsHandler vqHandler=mMediaChannel->getAudioHDVQStatsHandler();
      if(vqHandler) {
	vqHandler->block=vqResultsBlock;
      }
    }
    if(mVideoCall) {
      VQSTREAM_NS::VideoVQualResultsHandler vqHandler=mMediaChannel->getVideoVQStatsHandler();
      if(vqHandler) {
	vqHandler->block=vqResultsBlock;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgent::ControlStreamGeneration(bool generate, bool initiatorParty, bool wantComplNotification) {

  TrivialBoolFunctionType act = 0;
    
  if (wantComplNotification) {
    act = boost::bind(&UserAgent::MediaMethodComplete, this, generate, initiatorParty, _1);
  }
    
  if(mMediaChannel) {
    if (generate) {
      mMediaChannel->startAll(act);
    } else {
      mMediaChannel->stopAll(act);
    }
  } else if(act) {
    act(true);
  }

}
int UserAgent::StartTipNegotiation(uint32_t mux_csrc){
  TipStatusDelegate_t act = 0;
  act = boost::bind(&UserAgent::TipStateUpdate, this,_1,_2,_3,_4);
  if(mMediaChannel){
      return mMediaChannel->startTip(act,mux_csrc);
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////

void UserAgent::SetInterface(unsigned int ifIndex, const ACE_INET_Addr& addr, size_t uasPerDevice)
{
  // Build local address string
 
  mIfIndex=ifIndex;

  if(mMediaChannel) {
    ACE_INET_Addr mediaAddr=addr;
    mediaAddr.set_port_number(0);
    if(mAudioCall) mMediaChannel->setLocalAddr(AUDIO_STREAM, mPort, ifIndex, mVDevBlock, &mediaAddr);
    if(mAudioHDCall) mMediaChannel->setLocalAddr(AUDIOHD_STREAM, mPort, ifIndex, mVDevBlock, &mediaAddr);
    if(mVideoCall) mMediaChannel->setLocalAddr(VIDEO_STREAM, mPort, ifIndex, mVDevBlock, &mediaAddr);
  }
  
  // Select proxy and rebuild its address string
  if (mUsingProxy) {
    if (addr.get_type() == AF_INET)
      SipUtils::SipAddressToInetAddress(mConfig.ProtocolProfile.ProxyIpv4Addr, mProxyAddr);
    else
      SipUtils::SipAddressToInetAddress(mConfig.ProtocolProfile.ProxyIpv6Addr, mProxyAddr);
    mProxyAddr.set_port_number(mConfig.ProtocolProfile.ProxyPort ? mConfig.ProtocolProfile.ProxyPort : DEFAULT_SIP_PORT_NUMBER);
    
      mProxyDomain = mConfig.ProtocolProfile.ProxyDomain;
  }

  if(mUsingProxyAsRegistrar){
      if(!mUsingProxy){
          if (addr.get_type() == AF_INET)
              SipUtils::SipAddressToInetAddress(mConfig.ProtocolProfile.ProxyIpv4Addr, mRegistrarAddr);
          else
              SipUtils::SipAddressToInetAddress(mConfig.ProtocolProfile.ProxyIpv6Addr, mRegistrarAddr);
          mRegistrarAddr.set_port_number(mConfig.ProtocolProfile.ProxyPort ? mConfig.ProtocolProfile.ProxyPort : DEFAULT_SIP_PORT_NUMBER);

              mRegistrarDomain = mConfig.ProtocolProfile.ProxyDomain;
      }else{
          mRegistrarAddr = mProxyAddr;
          mRegistrarDomain = mProxyDomain;
      }
  }else{
      if (addr.get_type() == AF_INET)
          SipUtils::SipAddressToInetAddress(mConfig.RegistrarProfile.RegistrarIpv4Addr, mRegistrarAddr);
      else
          SipUtils::SipAddressToInetAddress(mConfig.RegistrarProfile.RegistrarIpv6Addr, mRegistrarAddr);
      mRegistrarAddr.set_port_number(mConfig.RegistrarProfile.RegistrarPort?  mConfig.RegistrarProfile.RegistrarPort: DEFAULT_SIP_PORT_NUMBER);
      mRegistrarDomain = mConfig.RegistrarProfile.RegistrarDomain;
  }


  
  // Set IP service level
  uint8_t ipServiceLevel=((addr.get_type() == AF_INET) ? mConfig.Profile.L4L7Profile.Ipv4Tos : mConfig.Profile.L4L7Profile.Ipv6TrafficClass);
  if(mMediaChannel) mMediaChannel->setIpServiceLevel(ipServiceLevel);
  
  SetInterfaceTransport(ifIndex,addr,ipServiceLevel,(uint32_t)uasPerDevice);
}

////////////////////////////////////////////////////////////////////////////////

UserAgent::StatusNotification UserAgent::NotifyRegistrationCompleted(uint32_t regExpires, bool success, int regRetriesCount)
{
    // Ignore unregistration notifications
    if (!regExpires)
        return STATUS_UNKNOWN;

    // Notify owner of status update
    StatusNotification status;
    if (success) {
      setRegistered(true);
      status = STATUS_REGISTRATION_SUCCEEDED;
    } else if (regRetriesCount>0) {
      status = STATUS_REGISTRATION_RETRYING;
    } else {
      status = STATUS_REGISTRATION_FAILED;
    }

    if (mStatusDelegate) {
      mStatusDelegate(mIndex, status, ACE_Time_Value(ACE_OS::gettimeofday() - mRegStartTime));
    }
    
    return status;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyInviteCompleted(bool success) const
{
    // Notify owner of status update
    const StatusNotification status = success ? STATUS_CALL_SUCCEEDED : STATUS_CALL_FAILED;
    mStatusDelegate(mIndex, status, ACE_Time_Value(ACE_OS::gettimeofday() - mInviteStartTime));

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyCallAnswered(void) const
{
    // Notify owner of status update
  mStatusDelegate(mIndex, STATUS_CALL_ANSWERED, ACE_Time_Value::zero);

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyCallCompleted(void) const
{
    // Notify owner of status update
  mStatusDelegate(mIndex, STATUS_CALL_COMPLETED, ACE_Time_Value::zero);

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyRefreshCompleted(void) const
{
    // Notify owner of status update
    mStatusDelegate(mIndex, STATUS_REFRESH_COMPLETED, ACE_Time_Value::zero);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyResponseSent(int msgNum) const
{
    StatusNotification status = STATUS_UNKNOWN;
    // Notify owner of status update
    if(msgNum >= 100 && msgNum <= 199) 
      status = STATUS_RESPONSE1XX_SENT;
    else if(msgNum >= 200 && msgNum <= 299)
      status = STATUS_RESPONSE2XX_SENT;
    else if(msgNum >= 300 && msgNum <= 399)
      status = STATUS_RESPONSE3XX_SENT;
    else if(msgNum >= 400 && msgNum <= 499) 
    	status = STATUS_RESPONSE4XX_SENT;
    else if(msgNum >= 500 && msgNum <= 599)
    	status = STATUS_RESPONSE5XX_SENT;
    else if(msgNum >= 600 && msgNum <= 699)
     	status = STATUS_RESPONSE6XX_SENT;
     	
    mStatusDelegate(mIndex, status, ACE_Time_Value::zero);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyResponseReceived(int msgNum) const
{
    StatusNotification status = STATUS_UNKNOWN;
    // Notify owner of status update
    if(msgNum >= 100 && msgNum <= 199)
      status = STATUS_RESPONSE1XX_RECEIVED;
    else if(msgNum >= 200 && msgNum <= 299)
      status = STATUS_RESPONSE2XX_RECEIVED;
    else if(msgNum >= 300 && msgNum <= 399)
      status = STATUS_RESPONSE3XX_RECEIVED;
    else if(msgNum >= 400 && msgNum <= 499)  
    	status = STATUS_RESPONSE4XX_RECEIVED;
    else if(msgNum >= 500 && msgNum <= 599)
    	status = STATUS_RESPONSE5XX_RECEIVED;
    else if(msgNum >= 600 && msgNum <= 699)
     	status = STATUS_RESPONSE6XX_RECEIVED;
     	
    mStatusDelegate(mIndex, status, ACE_Time_Value::zero);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyInviteSent(bool reinvite) const
{
  StatusNotification status = STATUS_UNKNOWN;

  if(reinvite)
    status = STATUS_INVITE_RETRYING;
  else
    status = STATUS_INVITE_SENT;

    // Notify owner of status update
  mStatusDelegate(mIndex, status, ACE_Time_Value::zero);

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyInviteReceived(int hopsPerRequest) const
{
    // Notify owner of status update
  mStatusDelegate(mIndex, STATUS_INVITE_RECEIVED, ACE_Time_Value(hopsPerRequest));

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyCallFailed(bool transportError, bool timeoutError, bool serverError) const
{
  StatusNotification status = STATUS_UNKNOWN;

  if(transportError)
    status = STATUS_CALL_FAILED_TRANSPORT_ERROR;
  else if(timeoutError)
    status = STATUS_CALL_FAILED_TIMER_B;
  else if(serverError)
    status = STATUS_CALL_FAILED_5XX;

    // Notify owner of status update
  mStatusDelegate(mIndex, status, ACE_Time_Value::zero);

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyAckSent(void) const
{
    // Notify owner of status update
  mStatusDelegate(mIndex, STATUS_ACK_SENT, ACE_Time_Value::zero);

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyAckReceived(void) const
{
    // Notify owner of status update
  mStatusDelegate(mIndex, STATUS_ACK_RECEIVED, ACE_Time_Value::zero);

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyByeSent(bool retrying, ACE_Time_Value *timeDelta) const
{
  StatusNotification status = STATUS_UNKNOWN;
  ACE_Time_Value newTime;

  if(retrying) {
    status = STATUS_BYE_RETRYING;
    newTime = ACE_Time_Value::zero;
  }
  else {
    status = STATUS_BYE_SENT;
    newTime = ACE_Time_Value(ACE_OS::gettimeofday() - *timeDelta);
  }

    // Notify owner of status update
  mStatusDelegate(mIndex, status, newTime);

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyByeReceived(void) const
{
    // Notify owner of status update
  mStatusDelegate(mIndex, STATUS_BYE_RECEIVED, ACE_Time_Value::zero);

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyInviteResponseReceived(bool is180) const
{
  StatusNotification status = STATUS_UNKNOWN;

  if(is180)
    status = STATUS_INVITE_180_RESPONSE_RECEIVED;
  else
    status = STATUS_INVITE_1XX_RESPONSE_RECEIVED;

  // Notify owner of status update
  mStatusDelegate(mIndex, status, ACE_Time_Value(ACE_OS::gettimeofday() - mInviteStartTime));

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyInviteAckReceived(ACE_Time_Value *timeDelta) const
{
  // Notify owner of status update.  Time value will be passed as seconds
  mStatusDelegate(mIndex, STATUS_INVITE_ACK_RECEIVED, ACE_Time_Value(ACE_OS::gettimeofday() - *timeDelta));

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyByeStateDisconnected(ACE_Time_Value *startTime, ACE_Time_Value *endTime) const
{
  // Notify owner of status update.  Time value will be passed as seconds
  mStatusDelegate(mIndex, STATUS_BYE_DISCONNECTED, ACE_Time_Value(*endTime - *startTime));

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyByeFailed(void) const
{
  // Notify owner of status update.  Time value will be passed as seconds
  mStatusDelegate(mIndex, STATUS_BYE_FAILED, ACE_Time_Value::zero);

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyNonInviteSessionInitiated(void) const
{
  // Notify owner of status update
  mStatusDelegate(mIndex, STATUS_NONINVITE_SESSION_INITIATED, ACE_Time_Value::zero);

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyNonInviteSessionSuccessful(void) const
{
  // Notify owner of status update
  mStatusDelegate(mIndex, STATUS_NONINVITE_SESSION_SUCCESSFUL, ACE_Time_Value::zero);

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgent::NotifyNonInviteSessionFailed(bool transportError, bool timeoutError) const
{
  StatusNotification status = STATUS_UNKNOWN;

  if(transportError)
    status = STATUS_NONINVITE_SESSION_FAILED_TRANSPORT;
  else if(timeoutError)
    status = STATUS_NONINVITE_SESSION_FAILED_TIMER_F;
  else
    status = STATUS_NONINVITE_SESSION_FAILED;

  // Notify owner of status update
  mStatusDelegate(mIndex, status, ACE_Time_Value::zero);

  return true;
}

///////////////////////////////////////////////////////////////////////////////
