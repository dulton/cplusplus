/// @file
/// @brief Enc RTP Media Manager object
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <algorithm>
#include <string>
#include <cstring>
#include <iterator>
#include <sstream>
#include <list>
#include <vector>

using std::string;

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <ace/OS_NS_sys_socket.h>

#include "EncRTPUtils.h"
#include "EncRTPRvInc.h"
#include "EncRTPMediaEndpoint.h"
#include "EncRTPProcess.h"
#include "EncRTPMediaManager.h"
#include "MediaFilesCommon.h"
#include "MediaStreamInfo.h"
#include "EncodedMediaFile.h"
#include "VQStream.h"

///////////////////////////////////////////////////////////////////////////////

class ACE_INET_Addr;

USING_MEDIAFILES_NS;
USING_VQSTREAM_NS;

DECL_ENCRTP_NS

///////////////////////////////////////////////////////////////////////////////

class EncRTPMediaManagerImpl {

public:
  
  typedef std::vector<EndpointHandler> ContainerType;
  typedef std::list<EndpointHandler> FreeContainerType;
  typedef boost::shared_ptr<EncRTPProcess> ProcessType;
  typedef std::vector<ProcessType> ProcessContainerType;
  
  EncRTPMediaManagerImpl() : mCurrentIndex(0), mNumberOfCpus(0) {
      VoIPUtils::calculateCPUNumber(mNumberOfCpus);
    uint32_t i=0;
    while(i++<EncRTPProcessingParams::MAX_IDS) {
      EndpointHandler ep;
      mEndpointsAll.push_back(ep);
    }
    i=0;
    unsigned int processNum=getProcessesNumber();
    while(i<processNum) {
      ProcessType p;
      p.reset(new EncRTPProcess(EncRTPProcessingParams::MAX_IDS/processNum,i));
      mRtpProcesses.push_back(p);
      i++;
    }
    mVQStreamManager.reset(new VQStreamManager());
  }
  
  virtual ~EncRTPMediaManagerImpl() {
    EncRTPGuard(mMutex);
    ContainerType::iterator iter = mEndpointsAll.begin();
    while (iter != mEndpointsAll.end()) {
      EndpointHandler ep = *iter;
      if(ep) getProcess(ep)->close(ep);
      ++iter;
    }
  }
  
  EncRTPIndex getNewEndpointIndex(uint32_t parentID) {
    EncRTPGuard(mMutex);
    EndpointHandler ep;
    FreeContainerType::iterator iter = mEndpointsAvailable.begin();
    while(iter != mEndpointsAvailable.end()) {
      EndpointHandler epi=*iter;
      if(!epi->isInUse()) {
	ep=epi;
	mEndpointsAvailable.erase(iter);
	break;
      }
      ++iter;
    }
    
    if(!ep && (mCurrentIndex < EncRTPProcessingParams::MAX_IDS)) {
      mEndpointsAll[mCurrentIndex].reset(new EndpointType(mCurrentIndex));
      ep=mEndpointsAll[mCurrentIndex];
      ++mCurrentIndex;
    }
    
    if(!ep) {
      ep.reset(new EndpointType(INVALID_STREAM_INDEX));
    }
    ep->setProcessID(parentID % getProcessesNumber());
    ep->setInUse(true);
    return ep->get_id();
  }

  VoiceVQualResultsHandler getVoiceVQStatsHandler(EncRTPIndex id) {
    VoiceVQualResultsHandler ret;
    EndpointHandler ep = getHandler(id);
    if(ep) {
      ret = ep->getVoiceVQStatsHandler();
    }
    return ret;
  }

  VideoVQualResultsHandler getVideoVQStatsHandler(EncRTPIndex id) {
    VideoVQualResultsHandler ret;
    EndpointHandler ep = getHandler(id);
    if(ep) {
      ret = ep->getVideoVQStatsHandler();
    }
    return ret;
  }

  AudioHDVQualResultsHandler getAudioHDVQStatsHandler(EncRTPIndex id) {
    AudioHDVQualResultsHandler ret;
    EndpointHandler ep = getHandler(id);
    if(ep) {
      ret = ep->getAudioHDVQStatsHandler();
    }
    return ret;
  }

  int open(EncRTPIndex id,STREAM_TYPE type, 
	   unsigned int ifIndex, uint16_t vdevblock, ACE_INET_Addr *thisAddr, ACE_INET_Addr *peerAddr,uint8_t tos,uint32_t csrc) {
    EncRTPGuard(mMutex);

    char ifNameC[512];
    
    ACE_OS::if_indextoname(ifIndex,ifNameC);
    string ifName=ifNameC;

    EndpointHandler ep = getHandler(id);

    return getProcess(ep)->open(ep,type,ifName, vdevblock, thisAddr, peerAddr, tos,csrc);
  }
  int close(EncRTPIndex id) {
    EncRTPGuard(mMutex);

    EndpointHandler ep = getHandler(id);

    return getProcess(ep)->close(ep);
  }
  int release(EncRTPIndex id) {
    int ret=0;
    EncRTPGuard(mMutex);
    if (isValidId(id)) {
      EndpointHandler ep = getHandler(id);
      if(ep) {
	ret=getProcess(ep)->release(ep,boost::bind(&EndpointType::setInUse,ep.get(),false));
	mEndpointsAvailable.push_back(ep);
      }
    }
    return ret;
  }
  int startListening(EncRTPIndex id,EncodedMediaStreamHandler fh,VQInterfaceHandler interface) {
    EncRTPGuard(mMutex);
    if(!fh) {
      //Default stream
      MediaStreamInfo msi;
      fh.reset(new EncodedMediaFile(&msi));
    }
    EndpointHandler ep = getHandler(id);
    return getProcess(ep)->startListening(ep,fh,mVQStreamManager.get(),interface);
  }
  int stopListening(EncRTPIndex id) {
    EncRTPGuard(mMutex);
    EndpointHandler ep = getHandler(id);
    return getProcess(ep)->stopListening(ep);
  }
  int startTalking(EncRTPIndex id) {
    EncRTPGuard(mMutex);
    EndpointHandler ep = getHandler(id);
    return getProcess(ep)->startTalking(ep);
  }
  int stopTalking(EncRTPIndex id) {
    EncRTPGuard(mMutex);
    EndpointHandler ep = getHandler(id);
    return getProcess(ep)->stopTalking(ep);
  }
  
  bool isOpen(EncRTPIndex id) const {
    EncRTPGuard(mMutex);
    return getHandler(id)->isOpen();
  }
  bool isListening(EncRTPIndex id) const {
    EncRTPGuard(mMutex);
    return getHandler(id)->isListening();
  }
  bool isTalking(EncRTPIndex id) const {
    EncRTPGuard(mMutex);
    return getHandler(id)->isTalking();
  }
  VQInterfaceHandler createNewInterface() {
    return mVQStreamManager->createNewInterface();
  }
  int startTip(EncRTPIndex id,TipStatusDelegate_t act){
      EncRTPGuard(mMutex);
      EndpointHandler ep = getHandler(id);
      return getProcess(ep)->startTip(ep,act);
  }

  
protected:
  
  bool isValidId(EncRTPIndex id) const {
    return (id >= 0 && id < EncRTPProcessingParams::MAX_IDS);
  }
  
  EndpointHandler& getHandler(EncRTPIndex id) {
    if(isValidId(id) && mEndpointsAll[id]) return mEndpointsAll[id];
    static EndpointHandler ep(new EndpointType(INVALID_STREAM_INDEX));
    return ep;
  }

  const EndpointHandler& getHandler(EncRTPIndex id) const {
    if(isValidId(id) && mEndpointsAll[id]) return mEndpointsAll[id];
    static EndpointHandler ep(new EndpointType(INVALID_STREAM_INDEX));
    return ep;
  }
  
  ProcessType getProcess(EndpointHandler ep) const {
    EncRTPIndex id=0;
    if(ep) {
      id = ep->getProcessID();
    }
    int index=id;
    return mRtpProcesses[index];
  }
  
  unsigned int getProcessesNumber() const {
    unsigned int ret = (mNumberOfCpus);
    if(ret<1 || ret>32) ret=1;
    return ret;
  }
  
private:
  mutable EncRTPUtils::MutexType mMutex;

  boost::scoped_ptr<VQStreamManager> mVQStreamManager;
  EncRTPIndex mCurrentIndex;
  ContainerType mEndpointsAll;
  FreeContainerType mEndpointsAvailable;
  ProcessContainerType mRtpProcesses;
  unsigned int mNumberOfCpus;
};

////////////////////////////////////////////////////////////////////

EncRTPMediaManager::EncRTPMediaManager() {
  mPimpl.reset(new EncRTPMediaManagerImpl());
}

EncRTPMediaManager::~EncRTPMediaManager() {}

EncRTPIndex EncRTPMediaManager::getNewEndpointIndex(uint32_t parentID) {
  return mPimpl->getNewEndpointIndex(parentID);
}
VoiceVQualResultsHandler EncRTPMediaManager::getVoiceVQStatsHandler(EncRTPIndex id) {
  return mPimpl->getVoiceVQStatsHandler(id);
}
VideoVQualResultsHandler EncRTPMediaManager::getVideoVQStatsHandler(EncRTPIndex id) {
  return mPimpl->getVideoVQStatsHandler(id);
}
AudioHDVQualResultsHandler EncRTPMediaManager::getAudioHDVQStatsHandler(EncRTPIndex id) {
  return mPimpl->getAudioHDVQStatsHandler(id);
}
int EncRTPMediaManager::open(EncRTPIndex id,MEDIA_NS::STREAM_TYPE type, 
			     unsigned int ifIndex, uint16_t vdevblock, 
			     ACE_INET_Addr *thisAddr, ACE_INET_Addr *peerAddr, uint8_t tos,uint32_t csrc) {
  return mPimpl->open(id,type,ifIndex,vdevblock,thisAddr,peerAddr,tos,csrc);
}
int EncRTPMediaManager::close(EncRTPIndex id) {
  return mPimpl->close(id);
}
int EncRTPMediaManager::release(EncRTPIndex id) {
  return mPimpl->release(id);
}
int EncRTPMediaManager::startListening(EncRTPIndex id,EncodedMediaStreamHandler fh,VQInterfaceHandler interface) {
  return mPimpl->startListening(id,fh,interface);
}
int EncRTPMediaManager::stopListening(EncRTPIndex id) {
  return mPimpl->stopListening(id);
}
int EncRTPMediaManager::startTalking(EncRTPIndex id) {
  return mPimpl->startTalking(id);
}
int EncRTPMediaManager::stopTalking(EncRTPIndex id) {
  return mPimpl->stopTalking(id);
}
bool EncRTPMediaManager::isOpen(EncRTPIndex id) const {
  return mPimpl->isOpen(id);
}
bool EncRTPMediaManager::isListening(EncRTPIndex id) const {
  return mPimpl->isListening(id);
}
bool EncRTPMediaManager::isTalking(EncRTPIndex id) const {
  return mPimpl->isTalking(id);
}
VQInterfaceHandler EncRTPMediaManager::createNewInterface() {
  return mPimpl->createNewInterface();
}
int EncRTPMediaManager::startTip(EncRTPIndex id,TipStatusDelegate_t act){
    return mPimpl->startTip(id,act);
}
END_DECL_ENCRTP_NS

////////////////////////////////////////////////////////////////////////////////


