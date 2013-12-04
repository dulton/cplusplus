/// @file
/// @brief EncRTP MediaChannel class
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

#include "EncRTPUtils.h"
#include "EncRTPMediaChannelAdapter.h"
#include "EncRTPMediaManager.h"
#include "MediaFilesManager.h"

DECL_ENCRTP_NS

USING_MEDIAFILES_NS;
USING_MEDIA_NS;
USING_VQSTREAM_NS;

/////////////////////////////////////////////////

class EncRTPMediaChannelAdapterImpl {

 public:

  EncRTPMediaChannelAdapterImpl(EncRTPMediaManager* erManager, 
				MediaFilesManager* mfManager) : mIdChannel(mIdChannelCounter++),
								mCallType(DEFAULT_CALL_TYPE),
								mIpServiceLevel(0),
								mEncRtpManager(erManager),
								mMediaFilesManager(mfManager)
  {
    int i=0;
    for(i=0;i<TOTAL_STREAM_TYPES;i++) {
      mPort[i]=0;
      mVDevBlock[i]=0;
      mId[i]=INVALID_STREAM_INDEX;
      mMediaStreamIndex[i]=INVALID_STREAM_INDEX;
      mLocalTransportPort[i]=0;
      mRemoteTransportPort[i]=0;
      mIfIndex[i]=0;
      mStatFlag[i]=false;      
      mChannelOpened[i] = false;
    }
  }
  
  ~EncRTPMediaChannelAdapterImpl() {
    if(mEncRtpManager) {
      int i=0;
      for(i=0;i<TOTAL_STREAM_TYPES;i++) {
	if(mId[i]!=INVALID_STREAM_INDEX ) {
	  mEncRtpManager->close(mId[i]);
	  mEncRtpManager->release(mId[i]);
	}
      }
    }
  }

  CALL_TYPE getCallType() const {
    return mCallType;
  }

  int setCallType(CALL_TYPE callType) {
    this->mCallType=callType;
    allocateStreams();
    return 0;
  }

  int setStatFlag(STREAM_TYPE streamType, bool val) {
    this->mStatFlag[streamType] = val;
    return 0;
  }

  bool getStatFlag(STREAM_TYPE streamType) const {
    if(streamType>=0 && streamType<TOTAL_STREAM_TYPES) {
      return mStatFlag[streamType];
    }
    return false;
  }	  

  VoiceVQualResultsHandler getVoiceVQStatsHandler() {
    VoiceVQualResultsHandler ret;
    if(mEncRtpManager) ret = mEncRtpManager->getVoiceVQStatsHandler(mId[VOICE_STREAM]);
    return ret;
  }

  AudioHDVQualResultsHandler getAudioHDVQStatsHandler() {
    AudioHDVQualResultsHandler ret;
    if(mEncRtpManager) ret = mEncRtpManager->getAudioHDVQStatsHandler(mId[AUDIOHD_STREAM]);
    return ret;
  }

  VideoVQualResultsHandler getVideoVQStatsHandler() {
    VideoVQualResultsHandler ret;
    if(mEncRtpManager) ret = mEncRtpManager->getVideoVQStatsHandler(mId[VIDEO_STREAM]);
    return ret;
  }

  int setFPGAStreamIndex(STREAM_TYPE streamType, RTP_STREAM_TYPE rtpStreamType, uint32_t streamIndex) {
    return 0;
  }

  uint32_t getPort(STREAM_TYPE streamType) const {
    if(streamType>=0 && streamType<TOTAL_STREAM_TYPES) {
      return mPort[streamType];
    }
    return mPort[DEFAULT_STREAM];
  }

  uint32_t getVDevBlock(STREAM_TYPE streamType) const {
    if(streamType>=0 && streamType<TOTAL_STREAM_TYPES) {
      return mVDevBlock[streamType];
    }
    return mVDevBlock[DEFAULT_STREAM];
  }

  uint32_t getIfIndex(STREAM_TYPE streamType) const {
    if(streamType>=0 && streamType<TOTAL_STREAM_TYPES) {
      return mIfIndex[streamType];
    }
    return mIfIndex[DEFAULT_STREAM];
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

  EncodedMediaStreamIndex getFileIndex(STREAM_TYPE streamType) const {
    if(streamType>=0 && streamType<TOTAL_STREAM_TYPES) {
      return mMediaStreamIndex[streamType];
    }
    return mMediaStreamIndex[DEFAULT_STREAM];
  }

  int setLocalAddr(STREAM_TYPE streamType,
		   uint32_t port,
		   uint32_t ifIndex,
		   uint16_t vdevblock,
		   const ACE_INET_Addr* thisAddr,
		   uint32_t transportPort) {
    if(!(streamType>=0 && streamType<TOTAL_STREAM_TYPES)) streamType=DEFAULT_STREAM;
    mPort[streamType]=port;
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
    return 0;
  }
  
  void setFileIndex(STREAM_TYPE streamType, EncodedMediaStreamIndex fileIndex) {
    if(!(streamType>=0 && streamType<TOTAL_STREAM_TYPES)) streamType=DEFAULT_STREAM;
    mMediaStreamIndex[streamType]=fileIndex;
  }
  
  int start(STREAM_TYPE streamType,TrivialBoolFunctionType act) {

    int ret=-1;

    bool actCalled=false;
    bool streamStat=false;
    if(!(streamType>=0 && streamType<TOTAL_STREAM_TYPES)) streamType=DEFAULT_STREAM;

    if(!mChannelOpened[streamType]){
        stop(streamType,0);
        allocateStreams();
        ret = mEncRtpManager->open(mId[streamType],streamType, 
                mIfIndex[streamType], mVDevBlock[streamType], 
                &mThisAddr[streamType], &mPeerAddr[streamType],
                getIpServiceLevel());
        mChannelOpened[streamType] = true;
    }else
        ret = 0;

    if(mId[streamType]!=INVALID_STREAM_INDEX  && mMediaFilesManager && mEncRtpManager) {
        EncodedMediaStreamHandler file=
            mMediaFilesManager->getEncodedStreamByIndex(mMediaStreamIndex[streamType]);
        if(file && (ret >=0)){
            if(!mInterface) {
                mInterface=mEncRtpManager->createNewInterface();
            }

            streamStat = getStatFlag(streamType);
            if(streamStat == true)
              ret = mEncRtpManager->startListening(mId[streamType],file,mInterface);
            else
              ret = mEncRtpManager->startListening(mId[streamType],file,mNullInterface);
            
            ret |= mEncRtpManager->startTalking(mId[streamType]);
            if(act) {
                act(ret==0);
                actCalled=true;
            }
        }
    }

    if(!actCalled && act) {
      act(ret==0);
    }

    return ret;
  }
  
  int stop(STREAM_TYPE streamType,TrivialBoolFunctionType act) {
      bool actCalled=false;
      if(!(streamType>=0 && streamType<TOTAL_STREAM_TYPES)) streamType=DEFAULT_STREAM;
      if(mId[streamType]!=INVALID_STREAM_INDEX && mEncRtpManager) {
          mEncRtpManager->stopTalking(mId[streamType]);
          mEncRtpManager->stopListening(mId[streamType]);
          mEncRtpManager->close(mId[streamType]);
          if(act) {
              act(true);
              actCalled=true;
          }
      }
      if(!actCalled && act) {
          act(true);
      }
      mChannelOpened[streamType] = false;
      return 0;
  }
  
  int startAll(TrivialBoolFunctionType act) {

    int ret=-1;

    switch(mCallType) {
    case AUDIO_ONLY:
      ret = start(AUDIO_STREAM,0);
      break;
    case VIDEO_ONLY:
      ret = start(VIDEO_STREAM,0);
      break;
    case AUDIOHD_ONLY:
      ret = start(AUDIOHD_STREAM,0);
      break;
    case AUDIO_VIDEO:
      ret = start(VIDEO_STREAM,0);
      if(ret==0) {
	ret = start(AUDIO_STREAM,0);
      }
      break;
    case AUDIOHD_VIDEO:
      ret = start(VIDEO_STREAM,0);
      if(ret==0) {
	ret = start(AUDIOHD_STREAM,0);
      }
      break;
    default: //unknown call type:
      ;
    }

    if(act) act(ret==0);

    return ret;
  }

  int stopAll(TrivialBoolFunctionType act) {

    switch(mCallType) {
    case AUDIO_ONLY:
      stop(AUDIO_STREAM,0);
      break;
    case VIDEO_ONLY:
      stop(VIDEO_STREAM,0);
      break;
    case AUDIOHD_ONLY:
      stop(AUDIOHD_STREAM,0);
      break;
    case AUDIO_VIDEO:
      stop(AUDIO_STREAM,0);
      stop(VIDEO_STREAM,0);
      break;
    case AUDIOHD_VIDEO:
      stop(AUDIOHD_STREAM,0);
      stop(VIDEO_STREAM,0);
      break;
    default: //unknown call type:
      ;
    }

    if(act) act(true);

    return 0;
  }
  int _startTip(STREAM_TYPE streamType,TipStatusDelegate_t act,uint32_t mux_csrc){

    if(!mChannelOpened[streamType]){
        stop(streamType,0);
        allocateStreams();
        mEncRtpManager->open(mId[streamType],streamType, 
                mIfIndex[streamType], mVDevBlock[streamType], 
                &mThisAddr[streamType], &mPeerAddr[streamType],
                getIpServiceLevel(),mux_csrc);
        mChannelOpened[streamType] = true;
    }
    return mEncRtpManager->startTip(mId[streamType],act);
  }
  int startTip(TipStatusDelegate_t act,uint32_t mux_csrc){
      int ret = 0;

      switch(mCallType) {
          case AUDIO_ONLY:
              return _startTip(AUDIO_STREAM,act,mux_csrc);
          case VIDEO_ONLY:
              return _startTip(VIDEO_STREAM,act,mux_csrc);
          case AUDIOHD_ONLY:
              return _startTip(AUDIOHD_STREAM,act,mux_csrc);
          case AUDIO_VIDEO:
              ret = _startTip(AUDIO_STREAM,act,mux_csrc);
              ret |= _startTip(VIDEO_STREAM,act,mux_csrc);
              return ret;
          case AUDIOHD_VIDEO:
              ret = _startTip(AUDIOHD_STREAM,act,mux_csrc);
              ret |= _startTip(VIDEO_STREAM,act,mux_csrc);
              return ret;
          default: //unknown call type:
              return ret;

      }
  }

  void NotifyInterfaceDisabled(void) {
    ;
  }

  // IPv4 ToS / IPv6 traffic class mutator
  uint8_t getIpServiceLevel() { return mIpServiceLevel; }
  void setIpServiceLevel(uint8_t ipServiceLevel) { mIpServiceLevel = ipServiceLevel; }

  uint32_t getId() const {
    return mIdChannel;
  }

protected:

  int allocateStreams() {
    if(mEncRtpManager) {
      if(mCallType==AUDIO_ONLY) {
	if(mId[AUDIO_STREAM]==INVALID_STREAM_INDEX) {
	  mId[AUDIO_STREAM]=mEncRtpManager->getNewEndpointIndex(getId());
	}
	if(mId[AUDIOHD_STREAM]!=INVALID_STREAM_INDEX) {
	  mEncRtpManager->close(mId[AUDIOHD_STREAM]);
	  mEncRtpManager->release(mId[AUDIOHD_STREAM]);
	  mId[AUDIOHD_STREAM]=INVALID_STREAM_INDEX;
	}
	if(mId[VIDEO_STREAM]!=INVALID_STREAM_INDEX) {
	  mEncRtpManager->close(mId[VIDEO_STREAM]);
	  mEncRtpManager->release(mId[VIDEO_STREAM]);
	  mId[VIDEO_STREAM]=INVALID_STREAM_INDEX;
	}
      } else if(mCallType==AUDIO_VIDEO) {
	if(mId[AUDIO_STREAM]==INVALID_STREAM_INDEX) {
	  mId[AUDIO_STREAM]=mEncRtpManager->getNewEndpointIndex(getId());
	}
	if(mId[VIDEO_STREAM]==INVALID_STREAM_INDEX) {
	  mId[VIDEO_STREAM]=mEncRtpManager->getNewEndpointIndex(getId());
	}
	if(mId[AUDIOHD_STREAM]!=INVALID_STREAM_INDEX) {
	  mEncRtpManager->close(mId[AUDIOHD_STREAM]);
	  mEncRtpManager->release(mId[AUDIOHD_STREAM]);
	  mId[AUDIOHD_STREAM]=INVALID_STREAM_INDEX;
	}
      } else if(mCallType==AUDIOHD_VIDEO) {
	if(mId[AUDIOHD_STREAM]==INVALID_STREAM_INDEX) {
	  mId[AUDIOHD_STREAM]=mEncRtpManager->getNewEndpointIndex(getId());
	}
	if(mId[VIDEO_STREAM]==INVALID_STREAM_INDEX) {
	  mId[VIDEO_STREAM]=mEncRtpManager->getNewEndpointIndex(getId());
	}
	if(mId[AUDIO_STREAM]!=INVALID_STREAM_INDEX) {
	  mEncRtpManager->close(mId[AUDIO_STREAM]);
	  mEncRtpManager->release(mId[AUDIO_STREAM]);
	  mId[AUDIO_STREAM]=INVALID_STREAM_INDEX;
	}
      } else if(mCallType==VIDEO_ONLY) {
	if(mId[VIDEO_STREAM]==INVALID_STREAM_INDEX) {
	  mId[VIDEO_STREAM]=mEncRtpManager->getNewEndpointIndex(getId());
	}
	if(mId[AUDIO_STREAM]!=INVALID_STREAM_INDEX) {
	  mEncRtpManager->close(mId[AUDIO_STREAM]);
	  mEncRtpManager->release(mId[AUDIO_STREAM]);
	  mId[AUDIO_STREAM]=INVALID_STREAM_INDEX;
	}
	if(mId[AUDIOHD_STREAM]!=INVALID_STREAM_INDEX) {
	  mEncRtpManager->close(mId[AUDIOHD_STREAM]);
	  mEncRtpManager->release(mId[AUDIOHD_STREAM]);
	  mId[AUDIOHD_STREAM]=INVALID_STREAM_INDEX;
	}
      } else if(mCallType==AUDIOHD_ONLY) {
	if(mId[AUDIOHD_STREAM]==INVALID_STREAM_INDEX) {
	  mId[AUDIOHD_STREAM]=mEncRtpManager->getNewEndpointIndex(getId());
	}
	if(mId[AUDIO_STREAM]!=INVALID_STREAM_INDEX) {
	  mEncRtpManager->close(mId[AUDIO_STREAM]);
	  mEncRtpManager->release(mId[AUDIO_STREAM]);
	  mId[AUDIO_STREAM]=INVALID_STREAM_INDEX;
	}
	if(mId[VIDEO_STREAM]!=INVALID_STREAM_INDEX) {
	  mEncRtpManager->close(mId[VIDEO_STREAM]);
	  mEncRtpManager->release(mId[VIDEO_STREAM]);
	  mId[VIDEO_STREAM]=INVALID_STREAM_INDEX;
	}
      }
    }
    return 0;
  }
  
private:
  static uint32_t mIdChannelCounter;
  VQInterfaceHandler mInterface;
  VQInterfaceHandler mNullInterface;
  uint32_t mIdChannel;
  CALL_TYPE mCallType;
  uint8_t mIpServiceLevel;
  EncRTPMediaManager* mEncRtpManager;
  MediaFilesManager* mMediaFilesManager;
  EncRTPIndex mId[TOTAL_STREAM_TYPES];
  uint32_t mPort[TOTAL_STREAM_TYPES];
  uint16_t mVDevBlock[TOTAL_STREAM_TYPES];
  EncodedMediaStreamIndex mMediaStreamIndex[TOTAL_STREAM_TYPES];
  ACE_INET_Addr mThisAddr[TOTAL_STREAM_TYPES];
  ACE_INET_Addr mPeerAddr[TOTAL_STREAM_TYPES];
  uint32_t mLocalTransportPort[TOTAL_STREAM_TYPES];
  uint32_t mRemoteTransportPort[TOTAL_STREAM_TYPES];
  unsigned int mIfIndex[TOTAL_STREAM_TYPES];
  bool mStatFlag[TOTAL_STREAM_TYPES];
  bool mChannelOpened[TOTAL_STREAM_TYPES];
};

uint32_t EncRTPMediaChannelAdapterImpl::mIdChannelCounter=0;

//////////////////////////////////////////////////

EncRTPMediaChannelAdapter::EncRTPMediaChannelAdapter(EncRTPMediaManager* erManager, 
						     MediaFilesManager* mfManager) { 
  mPimpl.reset(new EncRTPMediaChannelAdapterImpl(erManager,mfManager));
}

EncRTPMediaChannelAdapter::~EncRTPMediaChannelAdapter() { 
  ;
}

CALL_TYPE EncRTPMediaChannelAdapter::getCallType() const { 
  return mPimpl->getCallType();
}

int EncRTPMediaChannelAdapter::setCallType(CALL_TYPE callType) { 
  return mPimpl->setCallType(callType);
}

bool EncRTPMediaChannelAdapter::getStatFlag(STREAM_TYPE streamType) const {
  return mPimpl->getStatFlag(streamType);
}

int EncRTPMediaChannelAdapter::setStatFlag(STREAM_TYPE streamType, bool val) {
  return mPimpl->setStatFlag(streamType, val);
}

VoiceVQualResultsHandler EncRTPMediaChannelAdapter::getVoiceVQStatsHandler() {
  return mPimpl->getVoiceVQStatsHandler();
}

AudioHDVQualResultsHandler EncRTPMediaChannelAdapter::getAudioHDVQStatsHandler() {
  return mPimpl->getAudioHDVQStatsHandler();
}

VideoVQualResultsHandler EncRTPMediaChannelAdapter::getVideoVQStatsHandler() {
  return mPimpl->getVideoVQStatsHandler();
}

int EncRTPMediaChannelAdapter::setFPGAStreamIndex(STREAM_TYPE streamType, 
						  RTP_STREAM_TYPE rtpStreamType, 
						  uint32_t streamIndex) {
  return mPimpl->setFPGAStreamIndex(streamType, rtpStreamType, streamIndex);
}

uint32_t EncRTPMediaChannelAdapter::getPort(STREAM_TYPE streamType) const { 
  return mPimpl->getPort(streamType);
}

uint16_t EncRTPMediaChannelAdapter::getVDevBlock(STREAM_TYPE streamType) const { 
  return mPimpl->getVDevBlock(streamType);
}

uint32_t EncRTPMediaChannelAdapter::getIfIndex(STREAM_TYPE streamType) const { 
  return mPimpl->getIfIndex(streamType);
}

const ACE_INET_Addr* EncRTPMediaChannelAdapter::getLocalAddr(STREAM_TYPE streamType) const { 
  return mPimpl->getLocalAddr(streamType);
}

uint32_t EncRTPMediaChannelAdapter::getLocalTransportPort(STREAM_TYPE streamType) const { 
  return mPimpl->getLocalTransportPort(streamType);
}

uint32_t EncRTPMediaChannelAdapter::getRemoteTransportPort(STREAM_TYPE streamType) const { 
  return mPimpl->getRemoteTransportPort(streamType);
}

const ACE_INET_Addr* EncRTPMediaChannelAdapter::getRemoteAddr(STREAM_TYPE streamType) const { 
  return mPimpl->getRemoteAddr(streamType);
}

EncodedMediaStreamIndex EncRTPMediaChannelAdapter::getFileIndex(STREAM_TYPE streamType) const { 
  return mPimpl->getFileIndex(streamType);
}

int EncRTPMediaChannelAdapter::setLocalAddr(STREAM_TYPE streamType,
					    uint32_t port,
					    uint32_t ifIndex,
					    uint16_t vdevblock,
					    const ACE_INET_Addr *thisAddr,
					    uint32_t transportPort) { 
  return mPimpl->setLocalAddr(streamType,port,ifIndex,vdevblock,thisAddr,transportPort);
}

int EncRTPMediaChannelAdapter::setLocalTransportPort(STREAM_TYPE streamType,
		    uint32_t transportPort) { 
  return mPimpl->setLocalTransportPort(streamType,transportPort);
}

int EncRTPMediaChannelAdapter::setRemoteAddr(STREAM_TYPE streamType,
		  const ACE_INET_Addr* destAddr, 
		  uint16_t destTransportPort) { 
  return mPimpl->setRemoteAddr(streamType,destAddr,destTransportPort);
}

void EncRTPMediaChannelAdapter::setFileIndex(STREAM_TYPE streamType,EncodedMediaStreamIndex file) { 
  mPimpl->setFileIndex(streamType,file);
}

int EncRTPMediaChannelAdapter::startAll(TrivialBoolFunctionType act) { 
  return mPimpl->startAll(act);
}

int EncRTPMediaChannelAdapter::stopAll(TrivialBoolFunctionType act) { 
  return mPimpl->stopAll(act);
}

void EncRTPMediaChannelAdapter::NotifyInterfaceDisabled(void) {
  mPimpl->NotifyInterfaceDisabled();
}

uint8_t EncRTPMediaChannelAdapter::getIpServiceLevel() { 
  return mPimpl->getIpServiceLevel();
}

void EncRTPMediaChannelAdapter::setIpServiceLevel(uint8_t ipServiceLevel) {
  mPimpl->setIpServiceLevel(ipServiceLevel);
}

uint32_t EncRTPMediaChannelAdapter::getId() const {
  return mPimpl->getId();
}
int      EncRTPMediaChannelAdapter::startTip(TipStatusDelegate_t act,uint32_t mux_csrc){
    return mPimpl->startTip(act,mux_csrc);
}
//////////////////////////////////////////////////

END_DECL_ENCRTP_NS

