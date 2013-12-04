/// @file
/// @brief MediaManager class
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

#include "EncRTPMediaManager.h"
#include "EncRTPMediaChannelAdapter.h"
#include "FPGARTPMediaChannelAdapter.h"
#include "GeneratorClientHandler.h"
#include "VoIPMediaManager.h"
#include "MediaFilesManager.h"

DECL_MEDIA_NS

USING_MEDIAFILES_NS;
USING_ENCRTP_NS;
USING_FPGARTP_NS;
USING_VQSTREAM_NS;

/////////////////////////////////////////////////

class VoIPMediaChannelCreator : public boost::noncopyable {
public:
  VoIPMediaChannelCreator() {}
  virtual ~VoIPMediaChannelCreator() {}
  virtual VoIPMediaChannel* getNewMediaChannel() = 0;
};

class EncRTPMediaChannelCreator : public VoIPMediaChannelCreator {
public:

  EncRTPMediaChannelCreator(MediaFilesManager* filesManager) : mFilesManager(filesManager) {
    mManager.reset(new EncRTPMediaManager());
  }

  virtual ~EncRTPMediaChannelCreator() {
    ;
  }

  virtual VoIPMediaChannel* getNewMediaChannel() {
    VoIPMediaChannel* channel = new EncRTPMediaChannelAdapter(mManager.get(), mFilesManager);
    return channel;
  }

private:
  boost::scoped_ptr<EncRTPMediaManager> mManager;
  MediaFilesManager* mFilesManager;
};

/////////////////////////////////////////////////////////////////////////////////

class FPGARTPMediaChannelCreator : public VoIPMediaChannelCreator {
public:

  FPGARTPMediaChannelCreator(GeneratorClientHandlerSharedPtr genClientHandler) : mGenClientHandler(genClientHandler) {
    ;
  }

  virtual ~FPGARTPMediaChannelCreator() {
    ;
  }

  virtual VoIPMediaChannel* getNewMediaChannel() {
    VoIPMediaChannel* channel = new FPGARTPMediaChannelAdapter(mGenClientHandler);
    return channel;
  }

private:
  GeneratorClientHandlerSharedPtr mGenClientHandler;
};

//////////////////////////////////////////////////////////////////////////////////////////

class NOPRTPMediaChannel : public VoIPMediaChannel {
  
public:
  
  NOPRTPMediaChannel() :
    mPort(0),mVDevBlock(0)
  {
    int i=0;
    for(i=0;i<TOTAL_STREAM_TYPES;i++) {
      mLocalTransportPort[i]=0;
      mRemoteTransportPort[i]=0;
      mIfIndex[i]=0;
    }
  };

  virtual ~NOPRTPMediaChannel() {
    stopAll(0);
  }

  virtual CHANNEL_TYPE getChannelType() const { return NOP_RTP; }
  
  virtual CALL_TYPE getCallType() const { return SIG_ONLY; }
  
  virtual int setCallType(CALL_TYPE callType) { return 0; }

  virtual int setStatFlag(STREAM_TYPE streamType, bool val) { return 0; };
  virtual bool getStatFlag(STREAM_TYPE streamType) const { return false; };

  virtual VoiceVQualResultsHandler getVoiceVQStatsHandler() {
    VoiceVQualResultsHandler ret;
    return ret;
  }

  virtual AudioHDVQualResultsHandler getAudioHDVQStatsHandler() {
    AudioHDVQualResultsHandler ret;
    return ret;
  }

  virtual VideoVQualResultsHandler getVideoVQStatsHandler() {
    VideoVQualResultsHandler ret;
    return ret;
  }
  
  virtual int setFPGAStreamIndex(STREAM_TYPE streamType, RTP_STREAM_TYPE rtpRtreamType,
				 uint32_t streamIndex) { return 0; }
  
  virtual uint32_t getPort(STREAM_TYPE streamType) const { return mPort; }
  virtual uint16_t getVDevBlock(STREAM_TYPE streamType) const { return mVDevBlock; }
  virtual uint32_t getIfIndex(STREAM_TYPE streamType) const { 
    if(streamType>=0 && streamType<TOTAL_STREAM_TYPES) {
      return mIfIndex[streamType];
    }
    return mIfIndex[DEFAULT_STREAM];
  }
  virtual const ACE_INET_Addr* getLocalAddr(STREAM_TYPE streamType) const {
    if(streamType>=0 && streamType<TOTAL_STREAM_TYPES) {
      return &mThisAddr[streamType];
    }
    return &mThisAddr[DEFAULT_STREAM];
  }
  
  virtual uint32_t getLocalTransportPort(STREAM_TYPE streamType) const {
    if(streamType>=0 && streamType<TOTAL_STREAM_TYPES) {
      return mLocalTransportPort[streamType];
    }
    return mLocalTransportPort[DEFAULT_STREAM];
  }
  virtual uint32_t getRemoteTransportPort(STREAM_TYPE streamType) const {
    if(streamType>=0 && streamType<TOTAL_STREAM_TYPES) {
      return mRemoteTransportPort[streamType];
    }
    return mRemoteTransportPort[DEFAULT_STREAM];
  }
  virtual const ACE_INET_Addr* getRemoteAddr(STREAM_TYPE streamType) const {
    if(streamType>=0 && streamType<TOTAL_STREAM_TYPES) {
      return &mPeerAddr[streamType];
    }
    return &mPeerAddr[DEFAULT_STREAM];
  }
  virtual MEDIAFILES_NS::EncodedMediaStreamIndex getFileIndex(STREAM_TYPE streamType) const {
    return INVALID_STREAM_INDEX;
  }
  
  virtual int setLocalAddr(STREAM_TYPE streamType,
			   uint32_t port,
			   uint32_t ifIndex,
			   uint16_t vdevblock,
			   const ACE_INET_Addr *thisAddr,
			   uint32_t transportPort = 0)  {
    if(!(streamType>=0 && streamType<TOTAL_STREAM_TYPES)) streamType=DEFAULT_STREAM;
    mPort=port;
    mVDevBlock=vdevblock;
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
  virtual int setLocalTransportPort(STREAM_TYPE streamType,
				    uint32_t transportPort)  {
    if(!(streamType>=0 && streamType<TOTAL_STREAM_TYPES)) streamType=DEFAULT_STREAM;
    mThisAddr[streamType].set_port_number(transportPort);
    mLocalTransportPort[streamType]=transportPort;
    return 0;
  }
  virtual int setRemoteAddr(STREAM_TYPE streamType,
			    const ACE_INET_Addr* destAddr, 
			    uint16_t destTransportPort = 0)  {
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
  virtual void setFileIndex(STREAM_TYPE streamType, MEDIAFILES_NS::EncodedMediaStreamIndex fileIndex) {}
  
  virtual int startAll(TrivialBoolFunctionType act=0) {
    if(act) act(true);
    return 0;
  }
  virtual int stopAll(TrivialBoolFunctionType act=0) {
    if(act) act(true);
    return 0;
  }
  
  virtual void NotifyInterfaceDisabled(void) {}
  virtual int startTip(TipStatusDelegate_t act,uint32_t mux_csrc){return 0;}
  
private:
  uint32_t mPort;
  uint16_t mVDevBlock;
  ACE_INET_Addr mThisAddr[TOTAL_STREAM_TYPES];
  ACE_INET_Addr mPeerAddr[TOTAL_STREAM_TYPES];
  uint32_t mLocalTransportPort[TOTAL_STREAM_TYPES];
  uint32_t mRemoteTransportPort[TOTAL_STREAM_TYPES];
  unsigned int mIfIndex[TOTAL_STREAM_TYPES];
};

class NOPRTPMediaChannelCreator : public VoIPMediaChannelCreator {
public:
  NOPRTPMediaChannelCreator() {
    ;
  }

  virtual ~NOPRTPMediaChannelCreator() {
    ;
  }

  virtual VoIPMediaChannel* getNewMediaChannel() {
    VoIPMediaChannel* channel = new NOPRTPMediaChannel();
    return channel;
  }

};

/////////////////////////////////////////////////////////////////////////////////////////////

class VoIPMediaManagerImpl : public boost::noncopyable {

 public:

  VoIPMediaManagerImpl() {
    mFilesManager.reset(new MediaFilesManager());
  }

  virtual ~VoIPMediaManagerImpl() {
    ;
  }

  void initialize(ACE_Reactor* reactor) {
    mFpgaRtpCreator.reset();
    if(reactor) {
      mGenClientHandler.reset(new GeneratorClientHandler(*reactor));
      mFpgaRtpCreator.reset(new FPGARTPMediaChannelCreator(mGenClientHandler));
    }
    mNopRtpCreator.reset(new NOPRTPMediaChannelCreator());
    mEncRtpCreator.reset(new EncRTPMediaChannelCreator(mFilesManager.get()));
  }

  void Activate(methodCompletionDelegate_t delegate) {
	;
  }

  void Deactivate() {
    if(mGenClientHandler) {
      mGenClientHandler->ClearMethodCompletionDelegate();
    }
  }
  void setMediaCompletionDelegation(methodCompletionDelegate_t delegate){
      if(mGenClientHandler)
          mGenClientHandler->SetMethodCompletionDelegate(delegate);
  }
  bool GetFPGAStreamIndexes(uint16_t port, 
			    uint32_t streamBlock, 
			    std::vector<uint32_t>& streamIndexes) {
    if(mGenClientHandler) {
      return mGenClientHandler->GetStreamIndexes(port, streamBlock, streamIndexes);
    }
    return true;
  }

  VoIPMediaChannel* getNewMediaChannel(CHANNEL_TYPE implType) {
    if(implType == ENCODED_RTP && mEncRtpCreator) return mEncRtpCreator->getNewMediaChannel();
    else if(implType == NOP_RTP && mNopRtpCreator) return mNopRtpCreator->getNewMediaChannel();
    else if(implType == SIMULATED_RTP && mFpgaRtpCreator) return mFpgaRtpCreator->getNewMediaChannel();
    else if(mNopRtpCreator) return mNopRtpCreator->getNewMediaChannel();
    else return new NOPRTPMediaChannel();
  }

  EncodedMediaStreamIndex prepareEncodedStream(const MediaStreamInfo *streamInfo) {
    return mFilesManager->prepareEncodedStream(streamInfo);
  }

  EncodedMediaStreamHandler getEncodedStreamByIndex(EncodedMediaStreamIndex index) const {
    return mFilesManager->getEncodedStreamByIndex(index);
  }

  EncodedMediaStreamIndex getStreamIndex(const MediaStreamInfo *streamInfo) const {
    return mFilesManager->getStreamIndex(streamInfo);
  }

  void removeEncodedStream(EncodedMediaStreamIndex index) {
    mFilesManager->removeEncodedStream(index);
  }

  void removeAllEncodedStreams() {
    mFilesManager->removeAllEncodedStreams();
  }

 private:
  GeneratorClientHandlerSharedPtr mGenClientHandler;
  boost::scoped_ptr<EncRTPMediaChannelCreator> mEncRtpCreator;
  boost::scoped_ptr<FPGARTPMediaChannelCreator> mFpgaRtpCreator;
  boost::scoped_ptr<NOPRTPMediaChannelCreator> mNopRtpCreator;
  boost::scoped_ptr<MediaFilesManager> mFilesManager;
};

/////////////////////////////////////////////////////

VoIPMediaManager::VoIPMediaManager() {
  mPimpl.reset(new VoIPMediaManagerImpl());
}

VoIPMediaManager::~VoIPMediaManager() {
  ;
}

void VoIPMediaManager::initialize(ACE_Reactor* reactor) {
  mPimpl->initialize(reactor);
}

void VoIPMediaManager::Activate(methodCompletionDelegate_t delegate) {
  mPimpl->Activate(delegate);
}

void VoIPMediaManager::Deactivate() {
  mPimpl->Deactivate();
}

bool VoIPMediaManager::GetFPGAStreamIndexes(uint16_t port, 
					    uint32_t streamBlock, 
					    std::vector<uint32_t>& streamIndexes) {
  return mPimpl->GetFPGAStreamIndexes(port, streamBlock, streamIndexes);
}

VoIPMediaChannel* VoIPMediaManager::getNewMediaChannel(CHANNEL_TYPE implType) {
  return mPimpl->getNewMediaChannel(implType);
}

EncodedMediaStreamIndex VoIPMediaManager::prepareEncodedStream(const MediaStreamInfo *streamInfo) {
  return mPimpl->prepareEncodedStream(streamInfo);
}

EncodedMediaStreamHandler VoIPMediaManager::getEncodedStreamByIndex(EncodedMediaStreamIndex index) const {
  return mPimpl->getEncodedStreamByIndex(index);
}

EncodedMediaStreamIndex VoIPMediaManager::getStreamIndex(const MediaStreamInfo *streamInfo) const {
  return mPimpl->getStreamIndex(streamInfo);
}

void VoIPMediaManager::removeEncodedStream(EncodedMediaStreamIndex index) {
  mPimpl->removeEncodedStream(index);
}

void VoIPMediaManager::removeAllEncodedStreams() {
  mPimpl->removeAllEncodedStreams();
}
void VoIPMediaManager::setMediaCompletionDelegation(methodCompletionDelegate_t delegate) {
    mPimpl->setMediaCompletionDelegation(delegate);
}

//////////////////////////////////////////////////

END_DECL_MEDIA_NS


