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

#include <boost/bind.hpp>

#include <ace/OS_NS_sys_socket.h>

#include "SipUtils.h"
#include "RvUserAgent.h"

#include "SipEngine.h"

USING_SIP_NS;
USING_RVSIP_NS;
USING_RV_SIP_ENGINE_NAMESPACE;

///////////////////////////////////////////////////////////////////////////////

int32_t RvUserAgent::mPendingChannels = 0;

///////////////////////////////////////////////////////////////////////////////

RvUserAgent::RvUserAgent(uint16_t port, 
			 uint16_t vdevblock,
			 size_t index, size_t numUaObjects, size_t indexInDevice, size_t deviceIndex, 
			 const std::string& name, 
			 const std::string& imsi, 
			 const std::string& ranid, 
             const config_t& config, 
			 ACE_Reactor& reactor, 
			 MEDIA_NS::VoIPMediaManager *voipMediaManager, 
			 const run_notification_t& notificator,
			 const run_lpcommand_t& lpnotificator) : 
  UserAgent(port, 
	    vdevblock,
	    index, numUaObjects, indexInDevice, deviceIndex, 
	    name, 
            config, 
	    reactor, 
	    voipMediaManager,imsi,ranid), mActive(true), mIsPending(false), mNotificator(notificator), mLPNotificator(lpnotificator) {

  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::constructor] " << mName);

  mChannel = SipEngine::getInstance()->newSipChannel();
  if(!mChannel) throw EPHXBadConfig("Too many SIP UAs configured");

  mChannel->setSMType(CFT_BASIC);
  mChannel->setTipDeviceType(NON_TIP_DEVICE_TYPE);

  if(mConfig.ProtocolProfile.CallType == sip_1_port_server::EnumSipCallType_TELEPRESENCE){
      uint32_t tipdevice_type = 0;
      bool bTipSignaling = true;
      switch(mConfig.ProtocolProfile.TipDeviceType){
          case sip_1_port_server::EnumTipDevice_CTS500_32:
              tipdevice_type = 590;
              break;
          case sip_1_port_server::EnumTipDevice_CTS1000:
              tipdevice_type = 478;
              break;
          case sip_1_port_server::EnumTipDevice_CTS1100:
              tipdevice_type = 520;
              break;
          case sip_1_port_server::EnumTipDevice_CTS1300_65:
              tipdevice_type = 505;
              break;
          case sip_1_port_server::EnumTipDevice_CTS1300_47:
              tipdevice_type = 591;
              break;
          case sip_1_port_server::EnumTipDevice_CTS1310_65:
              tipdevice_type = 596;
              break;
          case sip_1_port_server::EnumTipDevice_CTS3000:
              tipdevice_type = 479;
              break;
          case sip_1_port_server::EnumTipDevice_CTS3200:
              tipdevice_type = 480;
              break;
          case sip_1_port_server::EnumTipDevice_CE20:
              tipdevice_type = 580;
              bTipSignaling = false;
              break;
          case sip_1_port_server::EnumTipDevice_CEX60:
              tipdevice_type = 604;
              bTipSignaling = false;
              break;
          case sip_1_port_server::EnumTipDevice_CEX90:
              tipdevice_type = 584;
              bTipSignaling = false;
              break;
          case sip_1_port_server::EnumTipDevice_CTQSC90:
              tipdevice_type = 606;
              break;
          case sip_1_port_server::EnumTipDevice_CTQSC60:
              tipdevice_type = 607;
              break;
          case sip_1_port_server::EnumTipDevice_CTQSC40:
              tipdevice_type = 608;
              break;
          case sip_1_port_server::EnumTipDevice_CTQSC20:
              tipdevice_type = 609;
              break;
          case sip_1_port_server::EnumTipDevice_CTP42C60:
              tipdevice_type = 610;
              break;
          case sip_1_port_server::EnumTipDevice_CTP42C40:
              tipdevice_type = 633;
              break;
          case sip_1_port_server::EnumTipDevice_CTP42C20:
              tipdevice_type = 611;
              break;
          case sip_1_port_server::EnumTipDevice_CTP52C40:
              tipdevice_type = 612;
              break;
          case sip_1_port_server::EnumTipDevice_CTP52C60:
              tipdevice_type = 613;
              break;
          case sip_1_port_server::EnumTipDevice_CTP52DC60:
              tipdevice_type = 614;
              break;
          case sip_1_port_server::EnumTipDevice_CTP65:
              tipdevice_type = 615;
              break;
          case sip_1_port_server::EnumTipDevice_CTP65DC90:
              tipdevice_type = 616;
              break;
          case sip_1_port_server::EnumTipDevice_CTMX200:
              tipdevice_type = 617;
              break;
          case sip_1_port_server::EnumTipDevice_CTMX300:
              tipdevice_type = 627;
              break;
          case sip_1_port_server::EnumTipDevice_CTSX20:
              tipdevice_type = 626;
              break;
          case sip_1_port_server::EnumTipDevice_CTX9000:
              tipdevice_type = 619;
              break;
          case sip_1_port_server::EnumTipDevice_CTX9200:
              tipdevice_type = 620;
              break;
          case sip_1_port_server::EnumTipDevice_CTS500_37:
          default:
              tipdevice_type = 481;
              break;
      }
      if(bTipSignaling)
          mChannel->setSMType(CFT_TIP);
      mChannel->setTipDeviceType(tipdevice_type);
  }

  uint16_t privacy=0;
  if(mConfig.MobileProfile.MobileSignalingChoice){

      mChannel->setMobile(true);
      mChannel->setRanID(mRanID);
      mChannel->setRanType(static_cast<eRanType>(mRanType));
      if(mConfig.MobileProfile.MobileSignalingChoice == RVSIP_MOBILE_TYPE_USIM){
          mChannel->setMNCLen(static_cast<eMNCLen>(mConfig.ProtocolConfig.MncLength));
          mChannel->setImsi(mImsi,static_cast<eMNCLen>(mConfig.ProtocolConfig.MncLength));
      }else
          mChannel->setImsi("",MNC_LEN_2);
  }else
      mChannel->setMobile(false);

  if(mConfig.MobileProfile.PrivacyHeaderNone)
      privacy = mConfig.MobileProfile.PrivacyHeaderNone;
  else{
      privacy |= mConfig.MobileProfile.PrivacyHeaderHeader? RVSIP_PRIVACY_HEADER_OPTION_HEADER:0;
      privacy |= mConfig.MobileProfile.PrivacyHeaderSession? RVSIP_PRIVACY_HEADER_OPTION_SESSION:0;
      privacy |= mConfig.MobileProfile.PrivacyHeaderUser? RVSIP_PRIVACY_HEADER_OPTION_USER:0;
      privacy |= mConfig.MobileProfile.CriticalPrivacyHeader? RVSIP_PRIVACY_HEADER_OPTION_CRITICAL:0;
  }
  mChannel->setSubOnReg(mConfig.MobileProfile.SubscribeRegEvent);
  mChannel->setAka(mConfig.MobileProfile.AkaAuthentication);

  mChannel->setProcessingType(mConfig.ProtocolProfile.ReliableProvisonalResponse,
			      mConfig.ProtocolProfile.UseCompactHeaders, //short SIP messages form
			      (mConfig.ProtocolProfile.CallType == sip_1_port_server::EnumSipCallType_SIGNALING_ONLY),
			      mConfig.ProtocolProfile.SecureRequestURI,
			      mConfig.ProtocolProfile.CallIDDomainName,
			      mConfig.ProtocolProfile.AnonymousCall,
			      !mConfig.ProtocolProfile.UseUaToUaSignaling,
			      privacy);//privacy
  
  std::string userNumber = ""; //TODO support this tel uri in STC
  std::string userNumberContext = ""; //TODO support this tel uri in STC
  mChannel->setIdentity(mName, userNumber, userNumberContext,
			mName, //TODO: local display name
			false //register this tel uri; TODO tuneable in STC 
			);
  
  mChannel->setSessionExpiration(mConfig.ProtocolProfile.MinSessionExpiration,
				 mConfig.ProtocolProfile.DefaultRefresher==sip_1_port_server::EnumDefaultRefresher_REFRESHER_UAS ? 
				 RVSIP_SESSION_EXPIRES_REFRESHER_UAS : RVSIP_SESSION_EXPIRES_REFRESHER_UAC);
  mChannel->setMediaInterface(this);
  mChannel->setCallStateNotifier(this);
  mChannel->setRingTime(mConfig.ProtocolProfile.RingTime);
  if(mConfig.ProtocolProfile.CallTime>0) {
    mChannel->setCallTime(mConfig.ProtocolProfile.CallTime);
  } else {
    mChannel->setCallTime(1);
  }
  if(mAudioCall || mAudioHDCall || mVideoCall) {
    mChannel->setPostCallTime(500);
  } else {
    mChannel->setPostCallTime(0);
  }
  
  uint32_t prackTime = mConfig.ProtocolProfile.RingTime + SipChannel::POST_RING_PRACK_RCV_TIMEOUT;
  mChannel->setPrackTime(prackTime);
}

RvUserAgent::~RvUserAgent() {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::destructor] " << mName);
  mChannel->clearMediaInterface();
  mChannel->clearCallStateNotifier();
  SipEngine::getInstance()->getChannelFinder().remove(mChannel);
  SipEngine::getInstance()->releaseChannel(mChannel);
}
  
bool RvUserAgent::Enabled(void) const {
  return mChannel->isEnabled();
}

// Registration status methods
uint32_t RvUserAgent::GetRegExpiresTime() {
  return mChannel->getRegExpiresTime();
}
void RvUserAgent::SetRegExpiresTime(uint32_t regExpires) {
  mChannel->setRegExpiresTime(regExpires);
}
uint32_t RvUserAgent::getRegRetryCount() const {
  return mChannel->getRegRetries();
}
void RvUserAgent::setRegRetryCount(uint32_t value) {
  mChannel->setRegRetries(value);
}

// Reset dialogs: registration and call
void RvUserAgent::ResetDialogs(void) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::ResetDialogs] " << mName);
  mChannel->stopCallSession();
  mChannel->stopRegistration();
}
  
// Registration methods
bool RvUserAgent::StartRegisterDialog(void) {

  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);


  unsigned long expires=GetRegExpiresTime();

  mChannel->setRegistration(mChannel->getLocalUserName(),
			    expires,
			    expires/2,
			    getRegRetryCount(),
			    mConfig.ProtocolProfile.EnableRegRefresh); 
  markRegStartTime();

  bool ret = (mChannel->startRegistration()>=0);

  if(!ret) {
    TC_LOG_ERR_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : failure : " << mName);
  }
  return ret;
}
void RvUserAgent::AbortRegistrationDialog(void) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mChannel->stopRegistration();
}
void RvUserAgent::StartUnregisterDialog(void) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mChannel->startDeregistration();
}

bool RvUserAgent::StartOutgoingDialog(const ACE_INET_Addr& addr, bool isProxy, SipUri remoteUri) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  setActive(true);
  mLPNotificator(lpcommand_t(Port(),IfIndex(),boost::bind(&RvUserAgent::realStartOutgoingDialog, this, addr, isProxy, remoteUri)));  
  return true;
}

// Call control methods
bool RvUserAgent::realStartOutgoingDialog(const ACE_INET_Addr& addr, bool isProxy, SipUri remoteUri) {

  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);

  if(!isActive()) return false;

  std::string userName;
  std::string userNumberContext;
  std::string hostNameRemote;
  RvSipTransportAddr rvaddr;
  std::string proxy="";
  unsigned short proxyPort=0;
  std::string remoteDomain;
  unsigned short remoteDomainPort=0;

  std::string transport="";
  unsigned short remPort=0;
  std::string proxyName="";
  
  // If we gracefully stop test, we need to reset the callTime if the call stage doesn't setup successfully.
  if(mConfig.ProtocolProfile.CallTime>0) {
    mChannel->setCallTime(mConfig.ProtocolProfile.CallTime);
  } else {
    mChannel->setCallTime(1);
  }

  if(remoteUri.GetScheme()==SipUri::TEL_URI_SCHEME) {
    userName = remoteUri.GetTelNumber();
    if(remoteUri.IsLocalTel()) {
      userNumberContext=remoteUri.GetPhoneContext();
    }
  } else {
    userName = remoteUri.GetSipUser();
    transport = remoteUri.GetSipTransport();
    remPort = remoteUri.GetSipPort();
  }

  remoteDomain = mConfig.DomainProfile.RemotePeerSipDomainName;
  remoteDomainPort = mConfig.DomainProfile.RemotePeerSipPort;

  if(isProxy) {
    char paddr[512];
    addr.get_host_addr(paddr,sizeof(paddr)-1);
    proxy=paddr;

    proxyPort=addr.get_port_number();

    proxyName = mConfig.ProtocolProfile.ProxyName;
    
    if(remoteDomain.empty()) {
      hostNameRemote=proxyName;
    remoteDomain=mProxyDomain;
    remoteDomainPort=proxyPort;
  }
  }

  if(proxy == "0.0.0.0" || proxy == "::")
    proxy = "";

  memset(&rvaddr,0,sizeof(rvaddr));

  switch(mConfig.ProtocolProfile.SipMessagesTransport) {
  	case sip_1_port_server::EnumSipMessagesTransport_UDP:
  		rvaddr.eTransportType = RVSIP_TRANSPORT_UDP;
  		break;
  	case sip_1_port_server::EnumSipMessagesTransport_TCP:
  		rvaddr.eTransportType = RVSIP_TRANSPORT_TCP;
  		break;	
  	default:
  		rvaddr.eTransportType = RVSIP_TRANSPORT_UNDEFINED;
  		break;
  }
  
  if(mChannel->isTCP() || transport == "TCP" || transport == "tcp")     rvaddr.eTransportType = RVSIP_TRANSPORT_TCP;
  else if(transport == "UDP" || transport == "udp")     rvaddr.eTransportType = RVSIP_TRANSPORT_UDP;
  
  addr.get_host_addr(rvaddr.strIP, sizeof(rvaddr.strIP) );
  rvaddr.port = addr.get_port_number();

  if(rvaddr.port<1) rvaddr.port=remPort;
  
  if(strstr(rvaddr.strIP,":")) rvaddr.eAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_IP6;
  else rvaddr.eAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_IP;

  bool usingProxyDiscovery=false;
  if(isProxy && !proxy.length() && !mConfig.ProtocolProfile.ProxyName.length() && mProxyDomain.length()) {
    usingProxyDiscovery= true;
    if(remoteDomain.empty()) {
      hostNameRemote="";
      remoteDomain=mProxyDomain;
      remoteDomainPort=0;
    }
  }

  mChannel->setRemote(userName,
		      userNumberContext,
		      userName, //TODO: display name remote
		      rvaddr,
		      hostNameRemote, 
		      remoteDomain, 
		      remoteDomainPort, 
		      (mCalleeUriScheme == SipUri::TEL_URI_SCHEME)); //use callee tel uri 

  markInviteStartTime();

  if(!mIsPending) {
    ++mPendingChannels;
    mIsPending=true;
  }

  mChannel->connectSession();

  return true;
}

bool RvUserAgent::NotifyInviteCompletedTop(bool success) const
{
  bool ret = true;

  if(mIsPending) {
    --mPendingChannels;
    mIsPending=false;
  }

  if(mChannel && !mChannel->isSigOnly()) ret=false;
  
  NotifyInviteCompleted(success);
  
  return ret;
}

void RvUserAgent::AbortCallDialog(bool bGraceful) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  setActive(false);
  if(mIsPending) {
    --mPendingChannels;
    mIsPending=false;
  }
  mChannel->stopCallSession(bGraceful);
  mChannel->resetInitialInviteSentStatus();
  mChannel->resetInitialByeSentStatus();
}
  
// Interface notification methods
void RvUserAgent::NotifyInterfaceDisabled(void) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mChannel->NotifyInterfaceDisabled();
  SipEngine::getInstance()->getChannelFinder().remove(mChannel);
  setRegistered(false);
  mMediaChannel->NotifyInterfaceDisabled();
  ControlStreamGeneration(false, true, false);
}

void RvUserAgent::NotifyInterfaceEnabled(void) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mChannel->NotifyInterfaceEnabled();
  SipEngine::getInstance()->getChannelFinder().put(mChannel);
}

const SipUri& RvUserAgent::ContactAddress(void) const
{
  return mContactAddress;
}

void RvUserAgent::SetAuthCredentials(const std::string& username, const std::string& password) {
  //TODO: realm
    if(mConfig.MobileProfile.AkaAuthentication)
        mChannel->setAuth(username,mRealm,password,(const char *)mConfig.MobileProfile.AkaOpField.c_str());
    else
        mChannel->setAuth(username,mRealm,password);
}
void RvUserAgent::SetAuthPasswordDebug(bool debug) {
  //TODO ???
}
 
///////////////////////////////////////////////////////////////////////////////
 
//Virtual protected:

void RvUserAgent::setExtraContactAddresses(const std::vector<SipUri> *extraContactAddresses) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  if(!extraContactAddresses) mChannel->clearExtraContacts();
  else {
    std::vector<std::string> ecs;
    std::vector<SipUri>::const_iterator iter=extraContactAddresses->begin();
    while(iter!=extraContactAddresses->end()) {
      ecs.push_back(iter->BuildUriString());
      ++iter;
    }
    mChannel->setExtraContacts(ecs);
  }
}

void RvUserAgent::SetInterfaceTransport(unsigned int ifIndex, const ACE_INET_Addr& addr, int ipServiceLevel, uint32_t uasPerDevice) {

  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);

  SipEngine::getInstance()->getChannelFinder().remove(mChannel);

  RvSipTransportAddr rvaddr;
  memset(&rvaddr,0,sizeof(rvaddr));

  ACE_OS::if_indextoname(ifIndex,rvaddr.if_name);

  rvaddr.vdevblock = mVDevBlock;

  addr.get_host_addr(rvaddr.strIP, sizeof(rvaddr.strIP) );

  switch(mConfig.ProtocolProfile.SipMessagesTransport) {
  	case sip_1_port_server::EnumSipMessagesTransport_UDP:
  		rvaddr.eTransportType = RVSIP_TRANSPORT_UDP;
  		break;
  	case sip_1_port_server::EnumSipMessagesTransport_TCP:
  		rvaddr.eTransportType = RVSIP_TRANSPORT_TCP;
  		break;	
  	default:
  		rvaddr.eTransportType = RVSIP_TRANSPORT_UNDEFINED;
  		break;
  }
  
  if(strstr(rvaddr.strIP,":")) rvaddr.eAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_IP6;
  else rvaddr.eAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_IP;

  rvaddr.port = addr.get_port_number();

  mChannel->setLocal(rvaddr, (uint8_t)ipServiceLevel, uasPerDevice);

  std::string contactDomain = mConfig.DomainProfile.LocalContactSipDomainName;
  unsigned short contactDomainPort = mConfig.DomainProfile.LocalContactSipPort;
  mChannel->setContactDomain(contactDomain,contactDomainPort);

  std::string localURIDomain = mConfig.DomainProfile.LocalUaUriDomainName;
  unsigned short localURIDomainPort = mConfig.DomainProfile.LocalUaUriPort;
  mChannel->setLocalURIDomain(localURIDomain,localURIDomainPort);

  SipEngine::getInstance()->getChannelFinder().put(mChannel);

  mContactAddress.SetSip(mChannel->getLocalUserName(), mChannel->getLocalIP(), mChannel->getLocalPort(), mChannel->getTransportStr(), false);

  mChannel->clearExtraContacts();

  std::string registrar;

  char paddr[512];
  mRegistrarAddr.get_host_addr(paddr,sizeof(paddr)-1);
  registrar=paddr;
  if(registrar == "0.0.0.0" || registrar == "::")
      registrar = "";
  std::string registrarName = mUsingProxyAsRegistrar?mConfig.ProtocolProfile.ProxyName:mConfig.RegistrarProfile.RegistrarName;

  bool usingRegistrarDiscovery=false;
  if(registrar.empty() && registrarName.empty() && !mRegistrarDomain.empty())
      usingRegistrarDiscovery = true;
  mChannel->setRegistrar(registrarName,registrar,mRegistrarDomain,mRegistrarAddr.get_port_number(),usingRegistrarDiscovery);

  std::string proxy;

  mProxyAddr.get_host_addr(paddr,sizeof(paddr)-1);
  proxy = paddr;
  if(proxy == "0.0.0.0" || proxy == "::")
      proxy = "";
  std::string proxyName = mConfig.ProtocolProfile.ProxyName;
  bool usingProxyDiscovery =false;
  if(proxy.empty() && proxyName.empty() && !mProxyDomain.empty())
      usingProxyDiscovery = true;

  mChannel->setProxy(proxyName,proxy,mProxyDomain,mProxyAddr.get_port_number(),usingProxyDiscovery);

}

void RvUserAgent::MediaMethodComplete(bool generate, bool initiatorParty, bool success) {
  if(mChannel) {
    mChannel->MediaMethodCompletionAsyncNotification(generate,initiatorParty,success);
  }
}
int RvUserAgent::TipNegoStatusUpdate(TIPMediaType type,TIPNegoStatus status,EncTipHandle handle,void *arg){
    if(mChannel){
        mChannel->TipStateUpdate(type,status,handle,arg);
    }
    return 0;
}
///////////////////////////////////////////////////////////////////////////////
//Tip notifer
int RvUserAgent::TipStateUpdate(TIPMediaType type,TIPNegoStatus status,EncTipHandle handle,void *arg){
    TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
    mNotificator(boost::bind(&RvUserAgent::TipNegoStatusUpdate, this, type, status,handle,arg ));
    return 0;
}

//Reg notifier
void RvUserAgent::regSuccess() {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  uint32_t regExpires=0;
  if(!mChannel->isDeregistering()) {
    regExpires=mChannel->getRegExpiresTime();
  }
  mNotificator(boost::bind(&RvUserAgent::NotifyRegistrationCompleted, this, regExpires, true, 0));
}

void RvUserAgent::regFailure(uint32_t regExpires, int regRetries) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyRegistrationCompleted, this, regExpires, false, regRetries));
}

void RvUserAgent::inviteCompleted(bool success) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  notification_t command = boost::bind(&RvUserAgent::NotifyInviteCompletedTop, this, success);
  mNotificator(command);
}
void RvUserAgent::callAnswered(void) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyCallAnswered, this));
}
void RvUserAgent::callCompleted(void) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyCallCompleted, this));
}
void RvUserAgent::refreshCompleted(void) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyRefreshCompleted, this));
}
void RvUserAgent::responseSent(int msgNum) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyResponseSent, this, msgNum));
}
void RvUserAgent::responseReceived(int msgNum) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyResponseReceived, this, msgNum));
}
void RvUserAgent::inviteSent(bool reinvite) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyInviteSent, this, reinvite));
}
void RvUserAgent::inviteReceived(int hopsPerRequest) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyInviteReceived, this, hopsPerRequest));
}
void RvUserAgent::callFailed(bool transportError, bool timeoutError, bool serverError) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyCallFailed, this, transportError, timeoutError, serverError));
}
void RvUserAgent::ackSent(void) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyAckSent, this));
}
void RvUserAgent::ackReceived(void) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyAckReceived, this));
}
void RvUserAgent::byeSent(bool retrying, ACE_Time_Value *timeDelta) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyByeSent, this, retrying, timeDelta));
}
void RvUserAgent::byeReceived(void) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyByeReceived, this));
}
void RvUserAgent::inviteResponseReceived(bool is180) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyInviteResponseReceived, this, is180));
}
void RvUserAgent::inviteAckReceived(ACE_Time_Value *timeDelta) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyInviteAckReceived, this, timeDelta));
}
void RvUserAgent::byeStateDisconnected(ACE_Time_Value *startTime, ACE_Time_Value *endTime) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyByeStateDisconnected, this, startTime, endTime));
}
void RvUserAgent::byeFailed(void) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyByeFailed, this)); 
}
void RvUserAgent::nonInviteSessionInitiated(void) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyNonInviteSessionInitiated, this));
}
void RvUserAgent::nonInviteSessionSuccessful(void) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyNonInviteSessionSuccessful, this));
}
void RvUserAgent::nonInviteSessionFailed(bool transportError, bool timeoutError) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  mNotificator(boost::bind(&RvUserAgent::NotifyNonInviteSessionFailed, this, transportError, timeoutError));
}
///////////////////////////////////////////////////////////////////////////////
