/// @file
/// @brief FPGARTP MediaChannel class
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <ace/INET_Addr.h>

#include "VoIPCommon.h"
#include "VoIPUtils.h"

#include "FPGARTPMediaChannelAdapter.h"

USING_MEDIAFILES_NS;
USING_MEDIA_NS;
USING_VQSTREAM_NS;

DECL_FPGARTP_NS

/////////////////////////////////////////////////

class FPGARTPMediaChannelAdapterImpl {
  
public:
  
  FPGARTPMediaChannelAdapterImpl(GeneratorClientHandlerSharedPtr genClientHandler) : 
    mCallType(DEFAULT_CALL_TYPE),
    mPort(0),
    mGenClientHandler(genClientHandler)
  {
    int i=0;
    int j=0;
    for(i=0;i<TOTAL_STREAM_TYPES;i++) {
      mVDevBlock[i]=0;
      mLocalTransportPort[i]=0;
      mRemoteTransportPort[i]=0;
      mIfIndex[i]=0;
      for(j=0;j<TOTAL_RTP_STREAM_TYPES;j++) {
	mStreamIndex[i][j]=INVALID_STREAM_INDEX;
      }
    }
  }
  
  ~FPGARTPMediaChannelAdapterImpl() {
    stopAll(0);
  }

  CALL_TYPE getCallType() const {
    return mCallType;
  }

  int setCallType(CALL_TYPE callType) {
    this->mCallType=callType;
    return 0;
  }

  int setStatFlag(STREAM_TYPE streamType, bool val) {
    return 0;
  }

  bool getStatFlag(STREAM_TYPE streamType) const {
    return false;
  }		  

  VoiceVQualResultsHandler getVoiceVQStatsHandler() {
    VoiceVQualResultsHandler ret;
    return ret;
  }

  AudioHDVQualResultsHandler getAudioHDVQStatsHandler() {
    AudioHDVQualResultsHandler ret;
    return ret;
  }

  VideoVQualResultsHandler getVideoVQStatsHandler() {
    VideoVQualResultsHandler ret;
    return ret;
  }

  int setFPGAStreamIndex(STREAM_TYPE streamType, RTP_STREAM_TYPE rtpStreamType, uint32_t streamIndex) {
    if(streamType>=0 && streamType<TOTAL_STREAM_TYPES) {
      if(rtpStreamType>=0 && rtpStreamType<TOTAL_RTP_STREAM_TYPES) {
	mStreamIndex[streamType][rtpStreamType]=streamIndex;
      }
    }
    return 0;
  }

  uint32_t getPort(STREAM_TYPE streamType) const {
    return mPort;
  }

  uint32_t getIfIndex(STREAM_TYPE streamType) const {
    if(streamType>=0 && streamType<TOTAL_STREAM_TYPES) {
      return mIfIndex[streamType];
    }
    return mIfIndex[DEFAULT_STREAM];
  }

  uint16_t getVDevBlock(STREAM_TYPE streamType) const {
    if(streamType>=0 && streamType<TOTAL_STREAM_TYPES) {
      return mVDevBlock[streamType];
    }
    return mVDevBlock[DEFAULT_STREAM];
  }

  const ACE_INET_Addr* getLocalAddr(STREAM_TYPE streamType) const {
    if(streamType>=0 && streamType<TOTAL_STREAM_TYPES) {
      return &mThisAddr[streamType];
    }
    return &mThisAddr[DEFAULT_STREAM];
  }

  uint32_t getLocalTransportPort(STREAM_TYPE streamType) const {
    if(streamType>=0 && streamType<TOTAL_STREAM_TYPES) {
      return mLocalTransportPort[streamType];
    }
    return mLocalTransportPort[DEFAULT_STREAM];
  }

  const ACE_INET_Addr* getRemoteAddr(STREAM_TYPE streamType) const {
    if(streamType>=0 && streamType<TOTAL_STREAM_TYPES) {
      return &mPeerAddr[streamType];
    }
    return &mPeerAddr[DEFAULT_STREAM];
  }
    
  uint32_t getRemoteTransportPort(STREAM_TYPE streamType) const {
    if(streamType>=0 && streamType<TOTAL_STREAM_TYPES) {
      return mRemoteTransportPort[streamType];
    }
    return mRemoteTransportPort[DEFAULT_STREAM];
  }

  MEDIAFILES_NS::EncodedMediaStreamIndex getFileIndex(STREAM_TYPE streamType) const {
    return INVALID_STREAM_INDEX;
  }

  int setLocalAddr(STREAM_TYPE streamType,
		   uint32_t port,
		   uint32_t ifIndex,
		   uint16_t vdevblock,
		   const ACE_INET_Addr *thisAddr,
		   uint32_t transportPort) {
    if(!(streamType>=0 && streamType<TOTAL_STREAM_TYPES)) streamType=DEFAULT_STREAM;
    mPort=port;
    mVDevBlock[streamType]=vdevblock;
    mIfIndex[streamType]=ifIndex;
    if(thisAddr) mThisAddr[streamType]=*thisAddr;
    if(transportPort>0) {
      mThisAddr[streamType].set_port_number(transportPort);
      mLocalTransportPort[streamType]=transportPort;
    } else if(mThisAddr[streamType].get_port_number()>0) {
      mLocalTransportPort[streamType]=mThisAddr[streamType].get_port_number();
    } else if(mLocalTransportPort[streamType]>0) {
      mThisAddr[streamType].set_port_number(mLocalTransportPort[streamType]);
    }
    return 0;
  }
  
  int setLocalTransportPort(STREAM_TYPE streamType,uint32_t transportPort) {
    if(!(streamType>=0 && streamType<TOTAL_STREAM_TYPES)) streamType=DEFAULT_STREAM;
    mThisAddr[streamType].set_port_number(transportPort);
    mLocalTransportPort[streamType]=transportPort;
    return 0;
  }
  
  int setRemoteAddr(STREAM_TYPE streamType,
		    const ACE_INET_Addr* destAddr, 
		    uint16_t destTransportPort) {
    if(!(streamType>=0 && streamType<TOTAL_STREAM_TYPES)) streamType=DEFAULT_STREAM;
    if(destAddr) mPeerAddr[streamType]=*destAddr;

    if(destTransportPort>0) {
      mPeerAddr[streamType].set_port_number(destTransportPort);
      mRemoteTransportPort[streamType]=destTransportPort;
    } else if(mPeerAddr[streamType].get_port_number()>0) {
      mRemoteTransportPort[streamType]=mPeerAddr[streamType].get_port_number();
    } else if(mRemoteTransportPort[streamType]>0) {
      mPeerAddr[streamType].set_port_number(mRemoteTransportPort[streamType]);
    }

    if(mPeerAddr[streamType].get_type() == PF_INET) {

      Sip_1_port_server::Ipv4Address_t addr;
      VoIPUtils::StringToSipAddress(mPeerAddr[streamType].get_host_addr(), addr);
      std::vector<uint32_t>si;
      si.push_back(mStreamIndex[streamType][RTP_STREAM]);
      si.push_back(mStreamIndex[streamType][RTCP_STREAM]);

      std::vector<uint16_t>pi;
      pi.push_back(mRemoteTransportPort[streamType]);
      pi.push_back(mRemoteTransportPort[streamType]+1);
      mGenClientHandler->ModifyStreamsDestination(mPort,si,reinterpret_cast<const Generator_3_port_client::Ipv4Address_t&>(addr),pi,getIpServiceLevel());

    
    } else if(mPeerAddr[streamType].get_type() == PF_INET6) {

      Sip_1_port_server::Ipv6Address_t addr;
      VoIPUtils::StringToSipAddress(mPeerAddr[streamType].get_host_addr(), addr);

      std::vector<uint32_t>si;
      si.push_back(mStreamIndex[streamType][RTP_STREAM]);
      si.push_back(mStreamIndex[streamType][RTCP_STREAM]);

      std::vector<uint16_t>pi;
      pi.push_back(mRemoteTransportPort[streamType]);
      pi.push_back(mRemoteTransportPort[streamType]+1);
      mGenClientHandler->ModifyStreamsDestination(mPort,si,reinterpret_cast<const Generator_3_port_client::Ipv6Address_t&>(addr),pi,getIpServiceLevel());

    }

    return 0;
  }
  
  void setFileIndex(STREAM_TYPE streamType, MEDIAFILES_NS::EncodedMediaStreamIndex fileIndex) {
    ;
  }
  
  int _run(STREAM_TYPE streamType,TrivialBoolFunctionType act,bool isStart) {
    
    int ret=0;
    
    std::vector<uint32_t> streamIndexes;
    
    int i=0;
    for(i=0;i<TOTAL_STREAM_TYPES;i++) {
      if(i==streamType || streamType==UNKNOWN_STREAM) {
	int j=0;
	for(j=0;j<TOTAL_RTP_STREAM_TYPES;j++) {
	  if(mStreamIndex[i][j]!=INVALID_STREAM_INDEX) {
	    streamIndexes.push_back(mStreamIndex[i][j]);
	  }
	}
      }
    }

    if(!streamIndexes.empty() && !isUnitTest) {
      if(act) {
	mGenClientHandlerAct=MakeAsyncCompletionToken(act);
	if (isStart) {
	  mGenClientHandler->StartStreams(mPort, streamIndexes, VOIP_NS::asyncCompletionToken_t(mGenClientHandlerAct));
	} else {
	  mGenClientHandler->StopStreams(mPort, streamIndexes,  VOIP_NS::asyncCompletionToken_t(mGenClientHandlerAct));
	}
      } else {
	if (isStart) {
	  mGenClientHandler->StartStreams(mPort, streamIndexes);
	} else {
	  mGenClientHandler->StopStreams(mPort, streamIndexes);
	}
      }
    } else {
      if(act) act(true);
    }
    
    return ret;
  }
  
  int start(STREAM_TYPE streamType,TrivialBoolFunctionType act) {
    return _run(streamType,act,true);
  }
  
  int stop(STREAM_TYPE streamType,TrivialBoolFunctionType act) {
    return _run(streamType,act,false);
  }
  
  int startAll(TrivialBoolFunctionType act) {
    int ret=0;
    ret = start(UNKNOWN_STREAM,act);
    return ret;
  }

  int stopAll(TrivialBoolFunctionType act) {
    int ret=0;
    ret = stop(UNKNOWN_STREAM,act);
    return ret;
  }

  void NotifyInterfaceDisabled(void) {
    if(!isUnitTest) {
      mGenClientHandlerAct.reset();
    }
  }
  void setIpServiceLevel(uint8_t ipServiceLevel) {mIpServiceLevel = ipServiceLevel;}
  uint8_t getIpServiceLevel() {return mIpServiceLevel;}

private:
  CALL_TYPE mCallType;
  uint32_t mPort;
  uint16_t mVDevBlock[TOTAL_STREAM_TYPES];
  GeneratorClientHandlerSharedPtr mGenClientHandler;
  AsyncCompletionToken mGenClientHandlerAct;
  ACE_INET_Addr mThisAddr[TOTAL_STREAM_TYPES];
  ACE_INET_Addr mPeerAddr[TOTAL_STREAM_TYPES];
  uint32_t mLocalTransportPort[TOTAL_STREAM_TYPES];
  uint32_t mRemoteTransportPort[TOTAL_STREAM_TYPES];
  unsigned int mIfIndex[TOTAL_STREAM_TYPES];
  uint32_t mStreamIndex[TOTAL_STREAM_TYPES][TOTAL_RTP_STREAM_TYPES];   //< audio/video RTP/RTCP stream indexes
  uint8_t  mIpServiceLevel;
};

//////////////////////////////////////////////////

FPGARTPMediaChannelAdapter::FPGARTPMediaChannelAdapter(GeneratorClientHandlerSharedPtr genClientHandler) { 
  mPimpl.reset(new FPGARTPMediaChannelAdapterImpl(genClientHandler));
}

FPGARTPMediaChannelAdapter::~FPGARTPMediaChannelAdapter() { 
  ;
}

CALL_TYPE FPGARTPMediaChannelAdapter::getCallType() const { 
  return mPimpl->getCallType();
}

int FPGARTPMediaChannelAdapter::setCallType(CALL_TYPE callType) { 
  return mPimpl->setCallType(callType);
}

bool FPGARTPMediaChannelAdapter::getStatFlag(STREAM_TYPE streamType) const { 
  return mPimpl->getStatFlag(streamType);
}

int FPGARTPMediaChannelAdapter::setStatFlag(STREAM_TYPE streamType, bool val) { 
  return mPimpl->setStatFlag(streamType, val);
}

VoiceVQualResultsHandler FPGARTPMediaChannelAdapter::getVoiceVQStatsHandler() {
  return mPimpl->getVoiceVQStatsHandler();
}

AudioHDVQualResultsHandler FPGARTPMediaChannelAdapter::getAudioHDVQStatsHandler() {
  return mPimpl->getAudioHDVQStatsHandler();
}

VideoVQualResultsHandler FPGARTPMediaChannelAdapter::getVideoVQStatsHandler() {
  return mPimpl->getVideoVQStatsHandler();
}

int FPGARTPMediaChannelAdapter::setFPGAStreamIndex(STREAM_TYPE streamType, 
						  RTP_STREAM_TYPE rtpStreamType, 
						  uint32_t streamIndex) {
  return mPimpl->setFPGAStreamIndex(streamType, rtpStreamType, streamIndex);
}

uint32_t FPGARTPMediaChannelAdapter::getPort(STREAM_TYPE streamType) const { 
  return mPimpl->getPort(streamType);
}

uint16_t FPGARTPMediaChannelAdapter::getVDevBlock(STREAM_TYPE streamType) const { 
  return mPimpl->getVDevBlock(streamType);
}

uint32_t FPGARTPMediaChannelAdapter::getIfIndex(STREAM_TYPE streamType) const { 
  return mPimpl->getIfIndex(streamType);
}

const ACE_INET_Addr* FPGARTPMediaChannelAdapter::getLocalAddr(STREAM_TYPE streamType) const { 
  return mPimpl->getLocalAddr(streamType);
}

uint32_t FPGARTPMediaChannelAdapter::getLocalTransportPort(STREAM_TYPE streamType) const { 
  return mPimpl->getLocalTransportPort(streamType);
}

uint32_t FPGARTPMediaChannelAdapter::getRemoteTransportPort(STREAM_TYPE streamType) const { 
  return mPimpl->getRemoteTransportPort(streamType);
}

const ACE_INET_Addr* FPGARTPMediaChannelAdapter::getRemoteAddr(STREAM_TYPE streamType) const { 
  return mPimpl->getRemoteAddr(streamType);
}

MEDIAFILES_NS::EncodedMediaStreamIndex FPGARTPMediaChannelAdapter::getFileIndex(STREAM_TYPE streamType) const { 
  return mPimpl->getFileIndex(streamType);
}

int FPGARTPMediaChannelAdapter::setLocalAddr(STREAM_TYPE streamType,
					     uint32_t port,
					     uint32_t ifIndex,
					     uint16_t vdevblock,
					     const ACE_INET_Addr *thisAddr,
					     uint32_t transportPort) { 
  return mPimpl->setLocalAddr(streamType,port,ifIndex,vdevblock,thisAddr,transportPort);
}

int FPGARTPMediaChannelAdapter::setLocalTransportPort(STREAM_TYPE streamType,
						uint32_t transportPort) { 
  return mPimpl->setLocalTransportPort(streamType,transportPort);
}

int FPGARTPMediaChannelAdapter::setRemoteAddr(STREAM_TYPE streamType,
					      const ACE_INET_Addr* destAddr, 
					      uint16_t destTransportPort) { 
  return mPimpl->setRemoteAddr(streamType,destAddr,destTransportPort);
}

void FPGARTPMediaChannelAdapter::setFileIndex(STREAM_TYPE streamType,MEDIAFILES_NS::EncodedMediaStreamIndex file) { 
  mPimpl->setFileIndex(streamType,file);
}

int FPGARTPMediaChannelAdapter::start(STREAM_TYPE streamType,TrivialBoolFunctionType act) { 
  return mPimpl->start(streamType,act);
}

int FPGARTPMediaChannelAdapter::stop(STREAM_TYPE streamType,TrivialBoolFunctionType act) { 
  return mPimpl->stop(streamType,act);
}

int FPGARTPMediaChannelAdapter::startAll(TrivialBoolFunctionType act) { 
  return mPimpl->startAll(act);
}

int FPGARTPMediaChannelAdapter::stopAll(TrivialBoolFunctionType act) { 
  return mPimpl->stopAll(act);
}

void FPGARTPMediaChannelAdapter::NotifyInterfaceDisabled(void) {
  mPimpl->NotifyInterfaceDisabled();
}
void FPGARTPMediaChannelAdapter::setIpServiceLevel(uint8_t ipServiceLevel) {
  mPimpl->setIpServiceLevel(ipServiceLevel);
}
uint8_t FPGARTPMediaChannelAdapter::getIpServiceLevel() {
    return mPimpl->getIpServiceLevel();
}

//////////////////////////////////////////////////

END_DECL_FPGARTP_NS

