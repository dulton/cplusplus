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

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <phxexception/PHXException.h>

#include "VoIPMediaManager.h"
#include "SdpMessage.h"
#include "SipProtocol.h"
#include "SipTransactionLayer.h"
#include "SipUtils.h"
#include "UdpSocketHandler.h"
#include "UdpSocketHandlerFactory.h"
#include "UserAgentLean.h"
#include "UserAgentCallDialog.h"
#include "UserAgentRegDialog.h"
#include "VQStatsData.h"

USING_SIP_NS;
USING_SIPLEAN_NS;
USING_MEDIA_NS;
USING_MEDIAFILES_NS;

///////////////////////////////////////////////////////////////////////////////

UserAgentLean::UserAgentLean(uint16_t port, 
			     uint16_t vdevblock,
			     size_t index, size_t numUaObjects, size_t indexInDevice, size_t deviceIndex, 
			     const std::string& name, 
			     const config_t& config, 
			     ACE_Reactor& reactor, 
			     VoIPMediaManager *voipMediaManager)
  : UserAgent(port,vdevblock,index,numUaObjects,indexInDevice,deviceIndex,name,config,reactor,voipMediaManager),
    mRegRetryCount(0), mRegExpires(DEFAULT_REG_EXPIRES_TIME)
{
  ;
}

///////////////////////////////////////////////////////////////////////////////

UserAgentLean::~UserAgentLean()
{
    mCallDialog.reset();
    mRegDialog.reset();

    if (mTransLayer && mTransport)
        mTransport->DetachReceiver(mTransLayer->Address(),mTransLayer->IfIndex());
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentLean::Enabled(void) const
{
    return mTransLayer && mTransLayer->Enabled();
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentLean::SetInterfaceTransport(unsigned int ifIndex, const ACE_INET_Addr& addr, int ipServiceLevel, uint32_t uasPerDevice)
{
  const std::string localAddrStr = SipUtils::InetAddressToString(addr);
  if (localAddrStr.empty()) {
    const std::string err("[UserAgentLean::SetInterfaceTransport] Invalid or unusable interface address");
    TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
    throw EPHXBadConfig(err);
  }
  
  // Detach previously existing objects
  if (mTransLayer && mTransport) {
    mTransport->DetachReceiver(mTransLayer->Address(),mTransLayer->IfIndex());
    mTransLayer.reset();
  }

  // Build a new transaction layer and UDP transport
  mTransLayer.reset(new SipTransactionLayer(mPort, ifIndex, addr, mReactor));
  mTransport = TSS_UdpSocketHandlerFactory->Make(*mReactor, addr.get_port_number());

  // Dynamically bind our new objects
  mTransLayer->SetSendDelegate(boost::bind(&UdpSocketHandler::Send, mTransport.get(), _1));
  mTransport->AttachReceiver(addr, ifIndex, boost::bind(&SipTransactionLayer::ReceivePacket, mTransLayer.get(), _1));
  mTransLayer->SetClientMessageDelegate(boost::bind(&UserAgentLean::StatusMessageHandler, this, _1, _2));
  mTransLayer->SetServerMessageDelegate(boost::bind(&UserAgentLean::RequestMessageHandler, this, _1, _2));

  mTransLayer->SetIpServiceLevel(ipServiceLevel);

  mBinding.contactAddress.SetSip(mName, localAddrStr, 
				 (addr.get_port_number()==DEFAULT_SIP_PORT_NUMBER ? 0 : addr.get_port_number()));

  mBinding.addressOfRecord = mUsingProxy ? SipUri(mName, mProxyDomain, 
						  (mProxyAddr.get_port_number()==DEFAULT_SIP_PORT_NUMBER ? 0 : mProxyAddr.get_port_number())) : 
    mBinding.contactAddress;  

  mBinding.extraContactAddresses.clear();
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentLean::ResetDialogs(void)
{
  if (mCallDialog && mCallDialog->Active())
    mCallDialog->Abort();
  
  if (mRegDialog)
    mRegDialog->Abort();
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentLean::StartRegisterDialog(void)
{
  // Create a new registration dialog and start the request
  MakeRegDialog();
  mRegDialog->SetRefresh(mConfig.ProtocolProfile.EnableRegRefresh);
  return mRegDialog->StartRegister(mRegExpires);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentLean::StartUnregisterDialog(void)
{
  // Create a new registration dialog and start the request with a registration expiration time of 0 seconds (immediate)
  MakeRegDialog();
  mRegDialog->StartRegister(0);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentLean::AbortRegistrationDialog(void)
{
  if (mRegDialog)
    mRegDialog->Abort();
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentLean::StartOutgoingDialog(const ACE_INET_Addr& toAddr, bool isProxy, SipUri remoteUri) {
  // Create a new call dialog and start the request
  MakeCallDialog(toAddr, remoteUri);
  return mCallDialog->StartInvite();
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentLean::AbortCallDialog(bool bGraceful)
{
    if (mCallDialog && mCallDialog->Active())
    {
        TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgentLean::AbortCall] " << mBinding.contactAddress << " aborting call");
        mCallDialog->Abort();
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentLean::NotifyInterfaceDisabled(void)
{
    if (!mTransLayer->Enabled())
        return;

    TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgentLean::NotifyInterfaceDisabled] " << mBinding.contactAddress << " disabling SIP");
    
    mTransLayer->Enable(false);
    mCallDialog.reset();
    mRegDialog.reset();
    setRegistered(false);
    mMediaChannel->NotifyInterfaceDisabled();
    ControlStreamGeneration(false, false, false);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentLean::NotifyInterfaceEnabled(void)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgentLean::NotifyInterfaceEnabled] " << mBinding.contactAddress << " enabling SIP");
    mTransLayer->Enable(true);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentLean::MakeCallDialog(const ACE_INET_Addr& peer, const SipUri& remoteUri)
{
    if (mCallDialog && mCallDialog->Active())
        TC_LOG_WARN_LOCAL(mPort, LOG_SIP, "[UserAgentLean::MakeCallDialog] replacing active call dialog with outgoing call to " << remoteUri);
        
    mCallDialog.reset(new UserAgentCallDialog(mPort, mBinding, peer, remoteUri, *mReactor, mUsingProxy));
    mCallDialog->SetDefaultRefresherOption(mConfig.ProtocolProfile.DefaultRefresher==sip_1_port_server::EnumDefaultRefresher_REFRESHER_UAS ? UserAgentCallDialog::DEFAULT_UAS_REFRESHER : UserAgentCallDialog::DEFAULT_UAC_REFRESHER);
    mCallDialog->SetSendRequestDelegate(boost::bind(&UserAgentLean::SendRequest, this, _1));
    mCallDialog->SetSendResponseDelegate(boost::bind(&UserAgentLean::SendResponse, this, _1));
    mCallDialog->SetCompactHeaders(mConfig.ProtocolProfile.UseCompactHeaders);
    mCallDialog->SetMinSessionExpiration(mConfig.ProtocolProfile.MinSessionExpiration);
    mCallDialog->SetCallTime(ACE_Time_Value(mConfig.ProtocolProfile.CallTime, 0));
    mCallDialog->SetInviteCompletionDelegate(boost::bind(&UserAgentLean::NotifyInviteCompleted, this, _1));
    mCallDialog->SetAnswerCompletionDelegate(boost::bind(&UserAgentLean::NotifyCallAnswered, this));
    mCallDialog->SetCallCompletionDelegate(boost::bind(&UserAgentLean::NotifyCallCompleted, this));
    mCallDialog->SetRefreshCompletionDelegate(boost::bind(&UserAgentLean::NotifyRefreshCompleted, this));
    mCallDialog->SetMakeMediaOfferDelegate(boost::bind(&UserAgentLean::BuildSdpMessageBody, this, _1));
    mCallDialog->SetConsumeMediaOfferDelegate(boost::bind(&UserAgentLean::UpdateStreamConfiguration, this, _1));
    mCallDialog->SetMediaControlDelegate(boost::bind(&UserAgentLean::ControlStreamGeneration, this, _1, false, _2));
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentLean::MakeCallDialog(const SipMessage& msg)
{
    if (mCallDialog && mCallDialog->Active())
        TC_LOG_WARN_LOCAL(mPort, LOG_SIP, "[UserAgentLean::MakeCallDialog] replacing active call dialog with incoming call from " << msg.fromUri);
        
    mCallDialog.reset(new UserAgentCallDialog(mPort, mBinding, msg, *mReactor, mUsingProxy));
    mCallDialog->SetDefaultRefresherOption(mConfig.ProtocolProfile.DefaultRefresher==sip_1_port_server::EnumDefaultRefresher_REFRESHER_UAS ? UserAgentCallDialog::DEFAULT_UAS_REFRESHER : UserAgentCallDialog::DEFAULT_UAC_REFRESHER);
    mCallDialog->SetSendRequestDelegate(boost::bind(&UserAgentLean::SendRequest, this, _1));
    mCallDialog->SetSendResponseDelegate(boost::bind(&UserAgentLean::SendResponse, this, _1));
    mCallDialog->SetCompactHeaders(mConfig.ProtocolProfile.UseCompactHeaders);
    mCallDialog->SetMinSessionExpiration(mConfig.ProtocolProfile.MinSessionExpiration);
    mCallDialog->SetRingTime(ACE_Time_Value(mConfig.ProtocolProfile.RingTime, 0));
    mCallDialog->SetInviteCompletionDelegate(boost::bind(&UserAgentLean::NotifyInviteCompleted, this, _1));
    mCallDialog->SetAnswerCompletionDelegate(boost::bind(&UserAgentLean::NotifyCallAnswered, this));
    mCallDialog->SetCallCompletionDelegate(boost::bind(&UserAgentLean::NotifyCallCompleted, this));
    mCallDialog->SetRefreshCompletionDelegate(boost::bind(&UserAgentLean::NotifyRefreshCompleted, this));
    mCallDialog->SetMakeMediaOfferDelegate(boost::bind(&UserAgentLean::BuildSdpMessageBody, this, _1));
    mCallDialog->SetConsumeMediaOfferDelegate(boost::bind(&UserAgentLean::UpdateStreamConfiguration, this, _1));
    mCallDialog->SetMediaControlDelegate(boost::bind(&UserAgentLean::ControlStreamGeneration, this, _1, false, _2));
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentLean::MakeRegDialog(void)
{
    mRegDialog.reset(new UserAgentRegDialog(mPort, mBinding, mProxyAddr, mProxyDomain, *mReactor));
    mRegDialog->SetSendRequestDelegate(boost::bind(&UserAgentLean::SendRequest, this, _1));
    mRegDialog->SetCompactHeaders(mConfig.ProtocolProfile.UseCompactHeaders);
    mRegDialog->SetCompletionDelegate(boost::bind(&UserAgentLean::NotifyRegistrationCompletedAndRetry, this, _1, _2));
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentLean::SendRequest(SipMessage& msg)
{
    if (!mTransLayer || msg.type != SipMessage::SIP_MSG_REQUEST)
        throw EPHXInternal("UserAgentLean::SendRequest");

    switch (msg.method)
    {
      case SipMessage::SIP_METHOD_REGISTER:
	markRegStartTime();
	break;

      case SipMessage::SIP_METHOD_INVITE:
	markInviteStartTime();
	break;
          
      default:
          break;
    }
    
    return mTransLayer->SendClientRequest(msg);
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentLean::SendResponse(const SipMessage& msg)
{
    if (!mTransLayer || msg.type != SipMessage::SIP_MSG_STATUS)
        throw EPHXInternal("UserAgentLean::SendResponse");

    return mTransLayer->SendServerStatus(msg);
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentLean::SendResponse(uint16_t statusCode, const::string& message, const SipMessage& req)
{
    SipMessage msg;

    //
    // Turn a response message around outside of a dialog.  Note that we don't set fromTag/toTag in
    // the message header.  This is meant to be used in situations where we're just echoing a request
    // with an errored response.
    //
    msg.transactionId = req.transactionId;
    msg.peer = req.peer;
    msg.type = SipMessage::SIP_MSG_STATUS;
    msg.compactHeaders = mConfig.ProtocolProfile.UseCompactHeaders;
    msg.method = req.method;
    msg.seq = req.seq;
    msg.status.code = statusCode;
    msg.status.version = SipMessage::SIP_VER_2_0;
    msg.status.message = message;

    msg.ForEachHeader(SipMessage::SIP_HEADER_VIA, SipMessage::PushHeader(msg));
    msg.ForEachHeader(SipMessage::SIP_HEADER_TO, SipMessage::PushHeader(msg));
    msg.ForEachHeader(SipMessage::SIP_HEADER_FROM, SipMessage::PushHeader(msg));
    msg.ForEachHeader(SipMessage::SIP_HEADER_CALL_ID, SipMessage::PushHeader(msg));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CSEQ, SipProtocol::BuildCSeqString(msg.seq, msg.method)));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTENT_LENGTH, "0"));

    return mTransLayer->SendServerStatus(msg);
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentLean::RequestMessageHandler(const SipMessage *msg, const std::string& localTag)
{
    if (msg)
    {
    	TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgentLean::RequestMessageHandler] " << mBinding.contactAddress << " received " << SipProtocol::BuildMethodString(msg->method) << " request");

        const bool sentInDialog = !msg->fromTag.empty() && !msg->toTag.empty();
        if (sentInDialog)
        {
        	// Dispatch request to matching dialog
            if (mCallDialog && mCallDialog->Matches(*msg))
            {
            	mCallDialog->ProcessRequest(*msg);
            }
            else
            {
                TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgentLean::RequestMessageHandler] " << mBinding.contactAddress << " ignoring request that doesn't match existing dialogs");
                SendResponse(481, "Call/Transaction Does Not Exist", *msg);
            }
        }
        else
        {
            // Only INVITE requests may establish a new dialog (RFC 3261, Section 12.1)
            if (msg->method == SipMessage::SIP_METHOD_INVITE)
            {
                if (!mCallDialog || !mCallDialog->Active())
                {
                    // Create a new call dialog and dispatch the request
                    MakeCallDialog(*msg);
                    mCallDialog->ProcessRequest(*msg);
                }
                else {
		  if(msg->fromTag == mCallDialog->ToTag()) {
		    mCallDialog->ProcessRequest(*msg);
		  } else {
		    SendResponse(486, "Busy Here", *msg);
		  }
		}
            }
            else
                SendResponse(403, "Forbidden", *msg);
        }
    }
    else
    {
        // Pass error notice to matching call dialog
        if (mCallDialog && mCallDialog->Active() && mCallDialog->FromTag() == localTag)
        {
            TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgentLean::RequestMessageHandler] " << mBinding.contactAddress << " received notice that transport layer error occurred for call dialog");
            mCallDialog->TransactionError();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentLean::StatusMessageHandler(const SipMessage *msg, const std::string& localTag)
{
    if (msg)
    {
        TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgentLean::StatusMessageHandler] " << mBinding.contactAddress << " received response \"" << msg->status.code << " " << msg->status.message << "\"");

        // Dispatch response to matching dialog
        if (mCallDialog && mCallDialog->Matches(*msg))
        {
            mCallDialog->ProcessResponse(*msg);
        }
        else if (mRegDialog && mRegDialog->Matches(*msg))
        {
            mRegDialog->ProcessResponse(*msg);
        }
    }
    else
    {
        // Pass error notice to matching dialog
        if (mCallDialog && mCallDialog->Active() && mCallDialog->FromTag() == localTag)
        {
            TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgentLean::StatusMessageHandler] " << mBinding.contactAddress << " received notice that transaction expired or transport layer error occurred for call dialog");
            mCallDialog->TransactionError();
        }
        else if (mRegDialog && mRegDialog->Active() && mRegDialog->FromTag() == localTag)
        {
            TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgentLean::StatusMessageHandler] " << mBinding.contactAddress << " received notice that transaction expired or transport layer error occurred for reg dialog");
            mRegDialog->TransactionError();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentLean::BuildSdpMessageBody(SdpMessage& sdp) const
{
    const ACE_Time_Value now(ACE_OS::gettimeofday());
    
    sdp.sessionId = now.msec();
    if(mConfig.ProtocolProfile.IncludeSdpStartTs ) {
      sdp.timestamp = now.msec();
    } else {
      sdp.timestamp = 0;
    }
    sdp.addrType = (mTransLayer->AddressFamily() == AF_INET) ? SdpMessage::SDP_ADDR_TYPE_IPV4 : SdpMessage::SDP_ADDR_TYPE_IPV6;
    sdp.addrStr = mBinding.contactAddress.GetSipHost();

    if (mAudioCall)
    {
        switch (static_cast<Sip_1_port_server::EnumSipAudioCodec>(mConfig.ProtocolProfile.AudioCodec))
        {
          case sip_1_port_server::EnumSipAudioCodec_G_711:
          default:
              sdp.audioMediaFormat = SdpMessage::SDP_MEDIA_FORMAT_PCMU;
              break;
          
          case sip_1_port_server::EnumSipAudioCodec_G_723_1:
          case sip_1_port_server::EnumSipAudioCodec_G_723_1_5_3:
            sdp.audioMediaFormat = SdpMessage::SDP_MEDIA_FORMAT_G723;
              break;
          
          case sip_1_port_server::EnumSipAudioCodec_G_729:
          case sip_1_port_server::EnumSipAudioCodec_G_729_A:
          case sip_1_port_server::EnumSipAudioCodec_G_729_B:
          case sip_1_port_server::EnumSipAudioCodec_G_729_AB:
             sdp.audioMediaFormat = SdpMessage::SDP_MEDIA_FORMAT_G729;
              break;

          case sip_1_port_server::EnumSipAudioCodec_G_711_A_LAW:
             sdp.audioMediaFormat = SdpMessage::SDP_MEDIA_FORMAT_PCMA;
             break;

          case sip_1_port_server::EnumSipAudioCodec_G_726_32:
             sdp.audioMediaFormat = SdpMessage::SDP_MEDIA_FORMAT_G726_32;
             sdp.audioRtpDynamicPayloadType = mConfig.ProtocolProfile.AudioPayloadType;
             break;

         case sip_1_port_server::EnumSipAudioCodec_G_711_1_MU_LAW_96:
             sdp.audioMediaFormat = SdpMessage::SDP_MEDIA_FORMAT_PCMU_WB;
             sdp.audioRtpDynamicPayloadType = mConfig.ProtocolProfile.AudioPayloadType;
             break;

         case sip_1_port_server::EnumSipAudioCodec_G_711_1_A_LAW_96:
             sdp.audioMediaFormat = SdpMessage::SDP_MEDIA_FORMAT_PCMA_WB;
             sdp.audioRtpDynamicPayloadType = mConfig.ProtocolProfile.AudioPayloadType;
             break;


        }

	if(mMediaChannel) sdp.audioTransportPort = mMediaChannel->getLocalTransportPort(MEDIA_NS::AUDIO_STREAM);
    }

    if (mVideoCall)
    {
        switch (static_cast<Sip_1_port_server::EnumSipVideoCodec>(mConfig.ProtocolProfile.VideoCodec))
        {
          case sip_1_port_server::EnumSipVideoCodec_H_264:
              sdp.videoMediaFormat = SdpMessage::SDP_MEDIA_FORMAT_H264;
              break;

          case sip_1_port_server::EnumSipVideoCodec_MP4V_ES:
              sdp.videoMediaFormat = SdpMessage::SDP_MEDIA_FORMAT_MP4V_ES;
              break;

          case sip_1_port_server::EnumSipVideoCodec_H_263:      
          default:
              sdp.videoMediaFormat = SdpMessage::SDP_MEDIA_FORMAT_H263;
              break;
        }

	if(mMediaChannel) sdp.videoTransportPort = mMediaChannel->getLocalTransportPort(MEDIA_NS::VIDEO_STREAM);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool UserAgentLean::UpdateStreamConfiguration(const SdpMessage& sdp)
{
    if (!mMediaChannel)
        return true;
    
    switch (sdp.addrType)
    {
      case SdpMessage::SDP_ADDR_TYPE_IPV4:
      {
          if (mTransLayer->AddressFamily() != AF_INET)
              return false;

	  if (mAudioCall) {
	    ACE_INET_Addr rAddr(sdp.audioTransportPort, sdp.addrStr.c_str(), PF_INET);
	    mMediaChannel->setRemoteAddr(AUDIO_STREAM,
					 &rAddr,
					 sdp.audioTransportPort);
	  }

	  if(mVideoCall) {
	    ACE_INET_Addr rAddr(sdp.videoTransportPort, sdp.addrStr.c_str(), PF_INET);
	    mMediaChannel->setRemoteAddr(VIDEO_STREAM,
					 &rAddr,
					 sdp.videoTransportPort);
	  }
          break;
      }
      
      case SdpMessage::SDP_ADDR_TYPE_IPV6:
      {
          if (mTransLayer->AddressFamily() != AF_INET6)
              return false;

	  if (mAudioCall) {
	    ACE_INET_Addr rAddr(sdp.audioTransportPort, sdp.addrStr.c_str(), PF_INET6);
	    mMediaChannel->setRemoteAddr(AUDIO_STREAM,
					 &rAddr,
					 sdp.audioTransportPort);
	  }

	  if(mVideoCall) {
	    ACE_INET_Addr rAddr(sdp.videoTransportPort, sdp.addrStr.c_str(), PF_INET6);
	    mMediaChannel->setRemoteAddr(VIDEO_STREAM,
					 &rAddr,
					 sdp.videoTransportPort);
	  }
          break;
      }

      default:
	return false;
    }

    return true;
}
///////////////////////////////////////////////////////////////////////////////

const SipUri& UserAgentLean::ContactAddress(void) const
{
  return mBinding.contactAddress;
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentLean::setExtraContactAddresses(const std::vector<SipUri> *extraContactAddresses) {
  mBinding.extraContactAddresses.clear();
  if (extraContactAddresses) {
    size_t ecIndex=0;
    const std::vector<SipUri> &eca=*extraContactAddresses;
    for(ecIndex=mIndex;ecIndex<extraContactAddresses->size();ecIndex+=mTotalBlockSize) {
      const SipUri &econtact=eca[ecIndex];
      mBinding.extraContactAddresses.push_back(econtact);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentLean::MediaMethodComplete(bool generate, bool initiatorParty, bool success)
{
    TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgentLean::GeneratorMethodComplete] " << mBinding.contactAddress << " Generator method completed with success = " << success);
    
    // Notify call dialog
    if (mCallDialog && mCallDialog->Active()) {
      mCallDialog->MediaControlCompleteDelayed(generate,success);
    }
}

///////////////////////////////////////////////////////////////////////////////

void UserAgentLean::NotifyRegistrationCompletedAndRetry(uint32_t regExpires, bool success)
{
  // Notify owner of status update

  StatusNotification status = NotifyRegistrationCompleted(regExpires, success, getRegRetryCount());
  
  // If we need to retry the registration, start that now
  if (status == STATUS_REGISTRATION_RETRYING) {
    decRegRetryCount();
    TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[UserAgentLean::NotifyRegisterCompleted] " << mBinding.contactAddress << " retrying registration, " << mRegRetryCount << " retry(s) remaining");
    if (!mRegDialog->StartRegister(mRegExpires)) {
      if (mStatusDelegate)
	mStatusDelegate(mIndex, STATUS_REGISTRATION_FAILED, ACE_Time_Value(ACE_OS::gettimeofday() - mRegStartTime));
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
