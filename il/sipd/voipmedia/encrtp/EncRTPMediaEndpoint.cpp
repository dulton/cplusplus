/// @file
/// @brief Enc RTP Media Endpoint (UA) object
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#if defined YOCTO_I686
#include <time.h>
#else
#include <posix_time.h>
#endif

#include <algorithm>
#include <string>
#include <cstring>
#include <iterator>
#include <sstream>

using std::string;

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <ace/INET_Addr.h>
#include <ace/SOCK_Dgram.h>
#include <ace/Reactor.h>

#include "VoIPUtils.h"
#include "VQStatsData.h"
#include "MediaStreamInfo.h"
#include "EncodedMediaFile.h"
#include "EncRTPUtils.h"
#include "EncRTPGenSession.h"
#include "EncRTPMediaEndpoint.h"

#include "MediaPacket.h"

///////////////////////////////////////////////////////////////////////////////

USING_MEDIAFILES_NS;
USING_VQSTREAM_NS;
USING_ENC_TIP_NAMESPACE;
DECL_ENCRTP_NS
extern "C" {
    static RvBool EncRTPTIPEventHandler(RvRtcpSession hRTCP,void *rtcpAppTIPContext,RvUint8* userData,RvUint32 userDataLen);

}
///////////////////////////////////////////////////////////////////////////////

//aux data buffer length for recvmsg
#define CONTROL_BUF_LEN (32)


///////////////////////////////////////////////////////////////////////////////

class EncRTPTransportEndpointImpl : public boost::noncopyable {
 public:
  EncRTPTransportEndpointImpl() : mRtpSession(0), mIsValid(false), mVersion(0) {
    ;
  }

  void setRtpSession(RvRtpSession session) {
    mRtpSession=session;
    mVersion++;
    mIsValid=(mRtpSession!=0 && RvRtpGetSocket(mRtpSession)!=ACE_INVALID_HANDLE);
  }

  RvRtpSession getRtpSession() const {
    return mRtpSession;
  }

  uint32_t getSockRcvBufferSize() const {
     uint32_t rcvBufSize = 0;
     int size = sizeof(rcvBufSize);
     
     if(mRtpSession && getsockopt(RvRtpGetSocket(mRtpSession), SOL_SOCKET, SO_RCVBUF, &rcvBufSize, (socklen_t *)&size) == -1)
        return 0;
     else
        return rcvBufSize; 
  }

  bool setSockRcvBufferSize(uint32_t rcvBufSize) {
     int size = sizeof(rcvBufSize);

     if(rcvBufSize == 0)
        return false;

     if(mRtpSession && setsockopt(RvRtpGetSocket(mRtpSession), SOL_SOCKET, SO_RCVBUF, &rcvBufSize, (socklen_t)size) == -1)
        return false;
     else
        return true;
  }
  bool setSockSndBufferSize(uint32_t sndBufSize) {
     int size = sizeof(sndBufSize);

     if(sndBufSize == 0)
        return false;

     if(mRtpSession && setsockopt(RvRtpGetSocket(mRtpSession), SOL_SOCKET, SO_SNDBUF, &sndBufSize, (socklen_t)size) == -1)
        return false;
     else
        return true;
  }

  virtual ~EncRTPTransportEndpointImpl() {
    this->closeTransport();
  }

  void closeTransport() {
      if(mRtpSession) {
          RvRtpClose(mRtpSession);
          mRtpSession=0;
      }
      mIsValid=false;
  }

  void lock() const {
    mMutex.acquire();
  }

  void unlock() const {
    mMutex.release();
  }

  ACE_HANDLE get_handle(void) const {
    if(mRtpSession) return (ACE_HANDLE)RvRtpGetSocket(mRtpSession);
    return ACE_INVALID_HANDLE;
  }

  bool isValid(void) const {
    if(mRtpSession==0 || RvRtpGetSocket(mRtpSession)==ACE_INVALID_HANDLE) mIsValid=false;
    return mIsValid;
  }

  void setValid(bool val) {
    mIsValid=val;
  }

  TransportVersion getVersion() {
    return mVersion;
  }

 private:
  mutable EncRTPUtils::MutexType mMutex;
  RvRtpSession mRtpSession;
  mutable bool mIsValid;
  TransportVersion mVersion;
};

///////////////////////////////////////////////////////////////////////////////

EncRTPTransportEndpoint::EncRTPTransportEndpoint() {
  mPimpl.reset(new EncRTPTransportEndpointImpl());
}
EncRTPTransportEndpoint::~EncRTPTransportEndpoint() {
  ;
}
RvRtpSession EncRTPTransportEndpoint::getRtpSession() const {
  return mPimpl->getRtpSession();
}
void EncRTPTransportEndpoint::setRtpSession(RvRtpSession session) {
  mPimpl->setRtpSession(session);
}

uint32_t EncRTPTransportEndpoint::getSockRcvBufferSize() const {
   return mPimpl->getSockRcvBufferSize();
}

bool EncRTPTransportEndpoint::setSockRcvBufferSize(uint32_t rcvBufSize) {
   return mPimpl->setSockRcvBufferSize(rcvBufSize);
}

bool EncRTPTransportEndpoint::setSockSndBufferSize(uint32_t sndBufSize) {
   return mPimpl->setSockSndBufferSize(sndBufSize);
}
 
void EncRTPTransportEndpoint::closeTransport() {
  mPimpl->closeTransport();
}
void EncRTPTransportEndpoint::lock() const {
  mPimpl->lock();
}
void EncRTPTransportEndpoint::unlock() const {
  mPimpl->unlock();
}
ACE_HANDLE EncRTPTransportEndpoint::get_handle(void) const {
  return mPimpl->get_handle();
}
bool EncRTPTransportEndpoint::isValid(void) const {
  return mPimpl->isValid();
}
void EncRTPTransportEndpoint::setValid(bool val) {
  mPimpl->setValid(val);
}
TransportVersion EncRTPTransportEndpoint::getVersion() {
  return mPimpl->getVersion();
}
///////////////////////////////////////////////////////////////////////////////

class EncRTPMediaEndpointImpl : public boost::noncopyable {
    typedef enum EncRTPTimerType{
        ENCRTP_TIMER_RTP=1,
        ENCRTP_TIMER_TIP
    };

public:

  EncRTPMediaEndpointImpl(EncRTPIndex id) : mIsInUse(false), mId(id), mVQTimeoutCycle(0), mPid(0),mTipScheduleLastTime (0),mPktScheduleLastTime(0) {
    mSock.reset(new EncRTPTransportEndpoint());
    mVoiceStatsHandler.reset(new VoiceVQualResultsRawData());
    mVideoStatsHandler.reset(new VideoVQualResultsRawData());
    mAudioHDStatsHandler.reset(new AudioHDVQualResultsRawData());
    mTipHandler.reset(new EncTIP());
    cleanStatsHandler();
    init();
  }
  
  void init() {
    mIsTalking=false;
    mIsListening=false;
    mIsOpen=false;
    mIsReady=false;
    mRtpGenSession.reset();
    mPsfunc=0;
    mCsrc = 0;
    mVQTimeoutCycle=0;
  }
  
  virtual ~EncRTPMediaEndpointImpl() {
    this->closeMEP(0);
  }

  bool isInUse() const {
    return mIsInUse;
  }

  void setInUse(bool value) {
    mIsInUse=value;
  }
  RvBool TIPEventHandler(RvRtcpSession hRTCP,void *rtcpAppTIPContext,RvUint8* userData,RvUint32 userDataLen){
      if(mTipHandler)
          return mTipHandler->receivePacket(userData,userDataLen);
      return RV_TRUE;
  }
  ACE_HANDLE get_handle(void) const {
    if(mIsOpen && mSock->isValid()) {
      return mSock->get_handle();
    } else {
      return ACE_INVALID_HANDLE;
    }
  }

  ssize_t handle_input(ACE_HANDLE fd) {
      int pkt_received = 0;

      if(mSock) {
          mSock->lock();
          try {

              do{
                  if(mIsReady && mIsListening && mSock->isValid() && (mSock->get_handle() == fd) && (fd != ACE_INVALID_HANDLE)) {

                      MediaPacketHandler cpacket(new MediaPacket(MediaPacket::PAYLOAD_MAX_SIZE));

                      int len = 0;
                      struct msghdr msg;
                      struct iovec  iov;
                      unsigned char ctlmsgbuf[CONTROL_BUF_LEN];
                      int recvmsg_flags=MSG_DONTWAIT;
                      uint64_t localTimestamp=0;
                      struct timeval localTimestampStruct;
                      struct timeval *localTimestampStructPtr=&localTimestampStruct;

                      iov.iov_base        = cpacket->getStartPtr();
                      iov.iov_len         = cpacket->getMaxSize();
                      msg.msg_name        = 0;
                      msg.msg_namelen     = 0;
                      msg.msg_iov         = &iov;
                      msg.msg_iovlen      = 1;
                      msg.msg_control     = ctlmsgbuf;
                      msg.msg_controllen  = CONTROL_BUF_LEN;
                      msg.msg_flags       = 0;

                      do {
                          len = recvmsg(RvRtpGetSocket(mSock->getRtpSession()), &msg, recvmsg_flags );
                      } while(len==-1 && errno==EINTR);
                      if(len>0) {
                          pkt_received = 1;

                          cpacket->setSize(len);

                          localTimestamp = rtpGetRecvTimestamp(ctlmsgbuf, msg.msg_controllen, localTimestampStructPtr);

                          RtpHeaderParam hdrParam;
                          cpacket->getRtpHeaderParam(hdrParam);
                          if(hdrParam.rtp_v == 2 && hdrParam.rtp_pl != 73){

                              if(mVQStream) {
                                  VqmaRtpPacketInfo vqPacketInfo;
                                  vqPacketInfo.localTimestamp=*localTimestampStructPtr;
                                  vqPacketInfo.packet = cpacket;
                                  if(mVQStream->getStreamInited())
                                      if(mVQStream->packetRecv(vqPacketInfo)==-1) {
                                          TC_LOG_WARN_LOCAL(0, LOG_ENCRTP, "Cannot feed packet to VQmon");
                                      }
                              }

                              RvRtcpSession  hRTCP = RvRtpGetRTCPSession(mSock->getRtpSession());

                              if(hRTCP) {
                                  if(mRtpGenSession) {
                                      EncodedMediaStreamHandler file = mRtpGenSession->getEncodedMediaStream();
                                      if(file) {
                                          const MediaStreamInfo* msInfo = file->getStreamInfo();
                                          if(msInfo) {
                                              uint64_t clts = localTimestamp * msInfo->getSamplingRate();
                                              RvRtcpRTPPacketRecvLight(hRTCP,
                                                      hdrParam.rtp_ssrc, 
                                                      hdrParam.rtp_ts, 
                                                      clts, 
                                                      hdrParam.rtp_sn);
                                          }
                                      }
                                  }
                              }
                          }
                      }else{
                          break;
                      }
                  }
              }while(0);

          } catch(...) {
              ;
          }

          mSock->unlock();
      }

      return pkt_received;
  }
  
  EncRTPIndex get_id() const {
    return mId;
  }

  VoiceVQualResultsHandler getVoiceVQStatsHandler() {
    return mVoiceStatsHandler;
  }

  AudioHDVQualResultsHandler getAudioHDVQStatsHandler() {
    return mAudioHDStatsHandler;
  }

  VideoVQualResultsHandler getVideoVQStatsHandler() {
    return mVideoStatsHandler;
  }

  int openTransport(STREAM_TYPE type,string ifName, uint16_t vdevblock, ACE_INET_Addr *thisAddr, ACE_INET_Addr *peerAddr, uint8_t tos,uint32_t csrc,int cputype) {
    
    int ret=-1;

    mSock->lock();

    ACE_INET_Addr lThisAddr;
    ACE_INET_Addr lPeerAddr;
    RvNetAddress lThisAddrRv;
    RvNetAddress lPeerAddrRv;
    
    if(thisAddr) {
      lThisAddr=*thisAddr;
    }
    if(peerAddr) lPeerAddr=*peerAddr;
    
    try {
      
      char local_ip[128];
      local_ip[0] = 0;
      lThisAddr.get_host_addr(local_ip, sizeof(local_ip) - 1);
      
      char remote_ip[128];
      remote_ip[0] = 0;
      lPeerAddr.get_host_addr(remote_ip, sizeof(remote_ip) - 1);
      
      char cname[256];
      
      RTP_initRvNetAddress ( lThisAddr.get_type(), (ifName.c_str()), vdevblock, local_ip, &lThisAddrRv );
      RTP_setPort(&lThisAddrRv, lThisAddr.get_port_number());
      
      RTP_initRvNetAddress ( lPeerAddr.get_type(), 0, 0, remote_ip, &lPeerAddrRv );
      RTP_setPort(&lPeerAddrRv, lPeerAddr.get_port_number());
      
      sprintf(cname,"%s[%d]:%s[%d]",local_ip,lThisAddr.get_port_number(),ifName.c_str(),(int)vdevblock);
      RvRtpSession rtpSession = RvRtpOpenEx(&lThisAddrRv,0,0,cname);
      if(!rtpSession) throw (string("Cannot open RTP session on ")+local_ip);
      mSock->setRtpSession(rtpSession);

      if(cputype == EncRTPOptimizationParams::CPU_ARCH_INTEL){
          if(type == VIDEO_STREAM){

              mSock->setSockRcvBufferSize(EncRTPOptimizationParams::VIDEO_SOCK_BUFFER_SIZE*4);
              mSock->setSockSndBufferSize(EncRTPOptimizationParams::VIDEO_SOCK_BUFFER_SIZE*4);
          } else{
              mSock->setSockRcvBufferSize(EncRTPOptimizationParams::VOICE_SOCK_BUFFER_SIZE);
              mSock->setSockSndBufferSize(EncRTPOptimizationParams::VOICE_SOCK_BUFFER_SIZE);
          }
      }else{
          if(type == VIDEO_STREAM){
              mSock->setSockRcvBufferSize(EncRTPOptimizationParams::VIDEO_SOCK_BUFFER_SIZE);
              mSock->setSockSndBufferSize(EncRTPOptimizationParams::VIDEO_SOCK_BUFFER_SIZE);
          } else{
              mSock->setSockRcvBufferSize(EncRTPOptimizationParams::VOICE_SOCK_BUFFER_SIZE);
              mSock->setSockSndBufferSize(EncRTPOptimizationParams::VOICE_SOCK_BUFFER_SIZE);
          }
      }
      
      const RvInt32 itos = static_cast<RvInt32>(tos);
      RvRtpSetTypeOfServiceSpirent(rtpSession,itos,&lPeerAddrRv);
      mCsrc = csrc;
      RvRtcpSetTypeOfServiceSpirent(RvRtpGetRTCPSession(rtpSession),itos,&lPeerAddrRv);
      
      RvRtpSetRemoteAddress(rtpSession,&lPeerAddrRv);
      if(RvRtpGetRTCPSession(rtpSession)) {
          RTP_setPort(&lPeerAddrRv, lPeerAddr.get_port_number()+1);
          RvRtcpSetRemoteAddress(RvRtpGetRTCPSession(rtpSession),&lPeerAddrRv);
          setupTip(type,RvRtcpGetSocket(RvRtpGetRTCPSession(rtpSession)),peerAddr);
          RTP_setPort(&lPeerAddrRv, lPeerAddr.get_port_number());
          RvRtcpSetAppTIPEventHandler(RvRtpGetRTCPSession(rtpSession),EncRTPTIPEventHandler,this);
      }
      
      ret=0;
      
    } catch(...) {
    }

    mSock->unlock();

    return ret;
  }

  int openMEP(ACE_Reactor *reactor) {
    
    int ret=-1;

    mSock->lock();
    
    if(mIsOpen) {
      
      ret=0;
      
    } else {
      
      mIsOpen=true;
      mIsReady=true;

      ret=0;
    }

    mSock->unlock();
    
    return ret;
  }

  static int sendPacket(MediaPacketDescHandler pi,int cpu) {
		   
    ssize_t ret = -1;

    if(pi) {

      MediaPacket *packet = pi->getPacket(cpu);

      if(packet) {

          EndpointHandler ep = pi->getEp();

          EncRTPTransportEndpointHandler sock = ep->getTransportHandler();

          if(sock) {

              sock->lock();

                  try {

                      if(sock->get_handle() != ACE_INVALID_HANDLE && sock->isValid()) {

                          RvRtpParam param;
                          memset(&param,0,sizeof(param));

                          RtpHeaderParam hdrParam;
                          packet->getLiveRtpHeaderParam(hdrParam);
                          param.sByte     = MediaPacket::HEADER_SIZE;
                          param.payload   = hdrParam.rtp_pl;
                          param.len       = packet->getLiveDataSize();
                          param.timestamp = hdrParam.rtp_ts;
                          param.singlePeer = 1;
                          param.marker    = hdrParam.rtp_mark;
                          param.cSrcc     = hdrParam.rtp_csrcc;
                          ret = RvRtpWriteLightNoRtcp(sock->getRtpSession(), 
                          //ret = RvRtpWriteLight(sock->getRtpSession(), 
                          //ret = RvRtpWriteLight(sock->getRtpSession(), 
                                  packet->getLiveDataPtr(), 
                                  param.len,
                                  &param);
                      }
                  } catch (...) {
                      ret=-1;
                  }

              sock->unlock();
          }
      }
    }

    return ret;
  }

  int closeTransport() {
      if(mSock) {
          mSock->lock();
          try {
              stopTIPNegotiate();
              mSock->closeTransport();
              mSock->unlock();
          } catch(...) {
              mSock->unlock();
              throw;
          }
      }
      return 0;
  }
  
  int closeMEP(ACE_Reactor *reactor) {

    if(mIsOpen) {

      mSock->lock();

      stopTalking(reactor);
      stopListening(reactor);
      
      mIsOpen=false;
      mIsReady=false;

      mSock->unlock();
      
      this->init();
    }

    return 0;
  }
  
  int release(ACE_Reactor *reactor,TrivialFunctionType f) {
    closeMEP(reactor);
    f();
    cleanStatsHandler();
    return 0;
  }
  
  int startListening(ACE_Reactor *reactor,
		     EncodedMediaStreamHandler file,
		     VQSTREAM_NS::VQStreamManager *vqStreamManager,
             VQInterfaceHandler interface) {
      if (mIsOpen && !mIsListening) {
          mInterface=interface;
          mIsListening=true;
          mRtpGenSession.reset(new EncRTPGenSession(file));
          if(mInterface){
              if(mRtpGenSession && vqStreamManager) {
                  EncodedMediaStreamHandler file = mRtpGenSession->getEncodedMediaStream();
                  if(file) {
                      const MediaStreamInfo* msInfo = file->getStreamInfo();
                      if(msInfo) {
                          uint32_t ssrc=0;
                          mSock->lock();
                          try {
                              if(mSock->isValid()) {
                                  RvRtpSession rtpSession = mSock->getRtpSession();
                                  if(rtpSession) {
                                      ssrc=(uint32_t)RvRtpGetSSRC(rtpSession);
                                  }
                              }
                              startStatsHandler(vqStreamManager,msInfo,ssrc);
                              mSock->unlock();
                          } catch(...) {
                              mSock->unlock();
                              throw;
                          }
                      }
                  }
              }
          }
      }
      return mIsListening ? 0 : -1;
  }
  
  int stopListening(ACE_Reactor *reactor) {
      if (mIsListening) {
          if(mVQStream && mInterface)
          {
             VQMediaType media_t = mVQStream->getMediaType();
             if(media_t == VQ_VOICE && mVQStream->getStreamInited())
                mVQStream->getVoiceMetrics(mVoiceStatsHandler);
#if 0
             else if(media_t == VQ_AUDIOHD && mVQStream->getStreamInited())
                 mVQStream->getAudioHDMetrics(mAudioHDStatsHandler);
             else if(media_t == VQ_VIDEO && mVQStream->getStreamInited())
                 mVQStream->getVideoMetrics(mVideoStatsHandler);
#endif
          }

          cleanStatsHandler();
          mIsListening=false;
      }
      return 0;
  }

  int startTalking(ACE_Reactor *reactor,PacketSchedulerFunctionType psfunc) {
      if(mIsOpen && !mIsTalking && mIsListening) {
              uint64_t ctime=VoIPUtils::getMilliSeconds();
              uint64_t schedulerInterval = EncRTPProcessingParams::PacketSchedulerIntervalInMs;
              mPsfunc=0;
              if(mRtpGenSession->start(ctime+EncRTPProcessingParams::InitialSessionDelay+(VoIPUtils::MakeRandomInteger() & 3),
                          schedulerInterval)!=-1) {
                  mPsfunc=psfunc;
                  mIsTalking=true;
              }
      }
      return mIsTalking ? 0 : -1;
  }

  int stopTalking(ACE_Reactor* reactor) {
      if(mIsTalking) {
          mPsfunc=0;
          mRtpGenSession->stop();
          mIsTalking=false;
      }
      return 0;
  }
  
  bool isOpen(void) const {
    return mIsOpen;
  }
  
  bool isReady(void) const {
    return mIsReady;
  }
  
  void setReady(bool value) {
    mIsReady=value;
  }

  bool isListening(void) const {
    return mIsListening;
  }

  bool isTalking(void) const {
    return mIsTalking;
  }

  void lock() const {
    mSock->lock();
  }

  void unlock() const {
    mSock->unlock();
  }

  uint32_t getProcessID() const {
    return mPid;
  }

  void setProcessID(uint32_t pid) {
    mPid=pid;
  }
  int startTIPNegotiate(ACE_Reactor* reactor,TipStatusDelegate_t cb){
      //TODO
      mTipHandler->setTipCB(cb);
      mTipHandler->startNegotiate(reactor);
      return 0;

  }
  int stopTIPNegotiate(void){
      //TODO
      int ret=0;

      if(mTipHandler){
          mTipHandler->setTipCB(0);
          ret = mTipHandler->stopNegotiate();
      }
      return ret;
  }
  static bool receiveTIPPkt(void *handler,uint8_t* buf,uint32_t len);
  int recvPacket(EndpointHandler ep){
      if(VoIPUtils::getMilliSeconds() - mTipScheduleLastTime >= EncRTPOptimizationParams::TIPPacketProcessIntervalInMs){
          doTIP();
          mTipScheduleLastTime = VoIPUtils::getMilliSeconds();
      }
      if(VoIPUtils::getMilliSeconds() - mPktScheduleLastTime >= EncRTPProcessingParams::PacketSchedulerIntervalInMs){
          schedulePacket(ep);
          mPktScheduleLastTime = VoIPUtils::getMilliSeconds();
      }
      int ret = 0;
      if(mSock){
          if(mInterface)
              ret = this->handle_input(mSock->get_handle());
          else
              ret = 1;
      }
      return ret;
  }
  uint32_t schedulePacket(EndpointHandler ep){
      uint32_t cnt=0;
      if(mSock) {
          mSock->lock();
          try {
              if(mIsReady && mIsTalking && mSock->isValid()) {

                  if(mRtpGenSession && mPsfunc) {


                      unsigned int i=0;

                      for(i=0;i<EncRTPProcessingParams::MaxNumberOfPacketsScheduledOnce;i++) {

                          MediaPacketDescHandler pktdesc = mRtpGenSession->getNextPacketDesc(ep,mCsrc);

                          if(!pktdesc) {
                              break;
                          }

                          //SendPacketInfoHandler pi(new SendPacketInfo(pktdesc,mSock,mSock->getVersion()));

                          mPsfunc(pktdesc->getTime(),pktdesc);
                          cnt++;
                      }

                      mRtpGenSession->nextSchedulingInterval();

                      if(mIsListening && mVQStream && mInterface) {
                          ++mVQTimeoutCycle;
                          if( (mVQTimeoutCycle==EncRTPProcessingParams::VQStatsCyclesInitial) || 
                                  ((mVQTimeoutCycle & EncRTPProcessingParams::VQStatsCycles)==0) ) {

                              VQMediaType media_t = mVQStream->getMediaType();
                              if(media_t == VQ_VOICE && mVQStream->getStreamInited())
                                  mVQStream->getVoiceMetrics(mVoiceStatsHandler);
                              else if(media_t == VQ_AUDIOHD && mVQStream->getStreamInited())
                                  mVQStream->getAudioHDMetrics(mAudioHDStatsHandler);
                              else if(media_t == VQ_VIDEO && mVQStream->getStreamInited())
                                  mVQStream->getVideoMetrics(mVideoStatsHandler);
                          }
                      }
                  }
              }
          } catch(...) {
          }

          mSock->unlock();
      }
      return cnt;
  }

  void doTIP(){
      if(mTipHandler && mTipHandler->isActive()){
          mTipHandler->doPeriodicActivity();
      }
  }
  EncRTPTransportEndpointHandler getTransportHandler(){return mSock;}

private:

  void startStatsHandler(VQSTREAM_NS::VQStreamManager *vqStreamManager, const MediaStreamInfo* msInfo, uint32_t ssrc) {
    mVQTimeoutCycle=0;
    mVQStream=vqStreamManager->createNewStream(mInterface,
					       msInfo->getCodec(),
					       msInfo->getPayloadNumber(),
					       msInfo->getBitRate(),
					       ssrc);
    if(mVoiceStatsHandler) {
      if(mVQStream && mVQStream->getMediaType()==VQ_VOICE && mVQStream->getStreamInited()) mVoiceStatsHandler->start();
    }
    if(mAudioHDStatsHandler) {
      if(mVQStream && mVQStream->getMediaType()==VQ_AUDIOHD && mVQStream->getStreamInited()) mAudioHDStatsHandler->start(); 
    }
    if(mVideoStatsHandler) {
      if(mVQStream && mVQStream->getMediaType()==VQ_VIDEO && mVQStream->getStreamInited()) mVideoStatsHandler->start();    
    }
  }

  void cleanStatsHandler() {
      mVQStream.reset();
      if(mInterface){
          if(mVoiceStatsHandler) {
              mVoiceStatsHandler->stop();
              mVoiceStatsHandler->clean();
          }
          if(mAudioHDStatsHandler) {
              mAudioHDStatsHandler->stop();
              mAudioHDStatsHandler->clean();
          }
          if(mVideoStatsHandler) {
              mVideoStatsHandler->stop();
              mVideoStatsHandler->clean();
          }
      }
  }

  uint64_t rtpGetRecvTimestamp(void *msgctlbuf, unsigned int msgctllen,struct timeval* ltsPtr) {
    
    struct timeval *recvtime = 0;
    int             ts_found = 0;
    uint64_t ms=0;

    if(msgctllen>0) {
       
      struct msghdr   msg;
      struct cmsghdr *hdr = 0;
       
      memset(&msg, 0, sizeof(msg));
      msg.msg_control = msgctlbuf;
      msg.msg_controllen = msgctllen;
       
      // lookind for packet timestamp data 
      hdr = CMSG_FIRSTHDR(&msg);
      while (hdr) {
	if (hdr->cmsg_type == SO_TIMESTAMP) {
	  // packet arrival timestamp found
	  recvtime = (struct timeval *) CMSG_DATA(hdr);
	  ts_found = 1;
	  break;
	}
	hdr = CMSG_NXTHDR(&msg, hdr);
      }
    }

    if (ts_found && recvtime) {
      // calculate packet arrival timestamp in milliseconds
      ms = (((uint64_t)(recvtime->tv_sec)) * 1000) + (((uint64_t)(recvtime->tv_usec)) / 1000);

      if(ltsPtr) {
	ltsPtr->tv_sec=recvtime->tv_sec;
	ltsPtr->tv_usec=recvtime->tv_usec;
      }
       
    } else {

      ms = VoIPUtils::getMilliSeconds();

      if(ltsPtr) {
	ltsPtr->tv_sec=ms/1000;
	ltsPtr->tv_usec=ms*1000;
      }
    }
            
    return ms;
  }
  void setupTip(STREAM_TYPE type,int rtcpSock,ACE_INET_Addr *remoteAddress){
      TIPMediaType tip_type;

      if(MEDIA_NS::VIDEO_STREAM == type)
          tip_type= TIPMediaVideo;
      else
          tip_type = TIPMediaAudio;
      mTipHandler->setupTip(tip_type,rtcpSock,remoteAddress);
  }
  TIPHandler getTipHandler(void){
      return mTipHandler;
  }


private:
  VQInterfaceHandler mInterface;
  EncRTPTransportEndpointHandler mSock;
  boost::scoped_ptr<EncRTPGenSession>  mRtpGenSession;
  EncRTPUtils::AtomicBool mIsInUse;
  EncRTPUtils::AtomicBool mIsOpen;
  EncRTPUtils::AtomicBool mIsReady;
  EncRTPUtils::AtomicBool mIsListening;
  EncRTPUtils::AtomicBool mIsTalking;
  EncRTPIndex mId;
  PacketSchedulerFunctionType mPsfunc;
  VQStreamHandler mVQStream;
  uint32_t mVQTimeoutCycle;
  VoiceVQualResultsHandler mVoiceStatsHandler;
  AudioHDVQualResultsHandler mAudioHDStatsHandler;
  VideoVQualResultsHandler mVideoStatsHandler;
  TIPHandler               mTipHandler;
  uint32_t mPid;
  uint32_t mCsrc;
  uint64_t mTipScheduleLastTime;
  uint64_t mPktScheduleLastTime;
};
bool EncRTPMediaEndpointImpl::receiveTIPPkt(void *handler,uint8_t* buf,uint32_t len){
    EncRTPMediaEndpointImpl *ep = (EncRTPMediaEndpointImpl *)handler;
    if(ep){
        return ep->getTipHandler()->receivePacket(buf,len);
    }
    return true;
}
extern "C" {
    static RvBool EncRTPTIPEventHandler(RvRtcpSession hRTCP,void *rtcpAppTIPContext,RvUint8* userData,RvUint32 userDataLen){
        return EncRTPMediaEndpointImpl::receiveTIPPkt(rtcpAppTIPContext,userData,userDataLen);

    }
};

//////////////////////////////////////////////////////////////////////////////////////

EncRTPMediaEndpoint::EncRTPMediaEndpoint(EncRTPIndex id) {
  mPimpl.reset(new EncRTPMediaEndpointImpl(id));
}

void EncRTPMediaEndpoint::init() {
    mPimpl->init();
}

EncRTPMediaEndpoint::~EncRTPMediaEndpoint() {
  ;
}

bool EncRTPMediaEndpoint::isInUse() const {
    return mPimpl->isInUse();
}

void EncRTPMediaEndpoint::setInUse(bool value) {
  mPimpl->setInUse(value);
}

EncRTPIndex EncRTPMediaEndpoint::get_id() const {
  return mPimpl->get_id();
}

VoiceVQualResultsHandler EncRTPMediaEndpoint::getVoiceVQStatsHandler() {
  return mPimpl->getVoiceVQStatsHandler();
}

AudioHDVQualResultsHandler EncRTPMediaEndpoint::getAudioHDVQStatsHandler() {
  return mPimpl->getAudioHDVQStatsHandler();
}

VideoVQualResultsHandler EncRTPMediaEndpoint::getVideoVQStatsHandler() {
  return mPimpl->getVideoVQStatsHandler();
}

int EncRTPMediaEndpoint::openTransport(STREAM_TYPE type,string ifName, uint16_t vdevblock, ACE_INET_Addr *thisAddr, ACE_INET_Addr *peerAddr, 
				       uint8_t tos,uint32_t csrc,int cputype) {
      return mPimpl->openTransport(type,ifName,vdevblock,thisAddr,peerAddr,tos,csrc,cputype);
}

int EncRTPMediaEndpoint::openMEP(ACE_Reactor *reactor) {
  return mPimpl->openMEP(reactor);
}

int EncRTPMediaEndpoint::closeTransport() {
  return mPimpl->closeTransport();
}
int EncRTPMediaEndpoint::closeMEP(ACE_Reactor *reactor) {
  return mPimpl->closeMEP(reactor);
}

int EncRTPMediaEndpoint::release(ACE_Reactor *reactor,TrivialFunctionType f) {
  return mPimpl->release(reactor,f);
}

int EncRTPMediaEndpoint::startListening(ACE_Reactor *reactor, 
					EncodedMediaStreamHandler file, 
					VQStreamManager *vqStreamManager,
					VQInterfaceHandler interface) {
  return mPimpl->startListening(reactor,file,vqStreamManager,interface);
}

int EncRTPMediaEndpoint::stopListening(ACE_Reactor *reactor) {
  return mPimpl->stopListening(reactor);
}

int EncRTPMediaEndpoint::startTalking(ACE_Reactor *reactor,PacketSchedulerFunctionType psfunc) {
  return mPimpl->startTalking(reactor,psfunc);
}

int EncRTPMediaEndpoint::stopTalking(ACE_Reactor *reactor) {
  return mPimpl->stopTalking(reactor);
}

bool EncRTPMediaEndpoint::isOpen(void) const {
  return mPimpl->isOpen();
}

bool EncRTPMediaEndpoint::isReady(void) const {
  return mPimpl->isReady();
}

void EncRTPMediaEndpoint::setReady(bool value) {
  mPimpl->setReady(value);
}

bool EncRTPMediaEndpoint::isListening(void) const {
  return mPimpl->isListening();
}

bool EncRTPMediaEndpoint::isTalking(void) const {
  return mPimpl->isTalking();
}

int EncRTPMediaEndpoint::sendPacket(MediaPacketDescHandler pi,int cpu) {
  return EncRTPMediaEndpointImpl::sendPacket(pi,cpu);
}
int EncRTPMediaEndpoint::startTIPNegotiate(ACE_Reactor *reactor,TipStatusDelegate_t act){
    return mPimpl->startTIPNegotiate(reactor,act);
}
int EncRTPMediaEndpoint::stopTIPNegotiate(){
    return mPimpl->stopTIPNegotiate();
}
int EncRTPMediaEndpoint::recvPacket(EndpointHandler ep){
    return mPimpl->recvPacket(ep);
}

void EncRTPMediaEndpoint::lock() const {
  mPimpl->lock();
}

void EncRTPMediaEndpoint::unlock() const {
  mPimpl->unlock();
}

uint32_t EncRTPMediaEndpoint::getProcessID() const {
  return mPimpl->getProcessID();
}

void EncRTPMediaEndpoint::setProcessID(uint32_t pid) {
  mPimpl->setProcessID(pid);
}

EncRTPTransportEndpointHandler EncRTPMediaEndpoint::getTransportHandler(){
    return mPimpl->getTransportHandler();
}

////////////////////////////////////////////////////////////////////////////////

END_DECL_ENCRTP_NS

///////////////////////////////////////////////////////////////////////////////////

