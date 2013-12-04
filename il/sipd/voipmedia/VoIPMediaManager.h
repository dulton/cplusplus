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

#ifndef _MEDIAMANAGER_H_
#define _MEDIAMANAGER_H_

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

#include <ildaemon/CommandLineOpts.h>

#include "VoIPCommon.h"
#include "AsyncCompletionHandler.h"

#include "VoIPMediaChannel.h"
#include "MediaFilesCommon.h"

class ACE_Reactor;

DECL_MEDIA_NS

/////////////////////////////////////////////////

class VoIPMediaManagerImpl;

class VoIPMediaManager : public boost::noncopyable {

 public:

  VoIPMediaManager();
  virtual ~VoIPMediaManager();
  
  void initialize(ACE_Reactor* reactor);

  void Activate(methodCompletionDelegate_t delegate);
  void Deactivate();
  bool GetFPGAStreamIndexes(uint16_t port, 
			    uint32_t streamBlock, 
			    std::vector<uint32_t>& streamIndexes);

  VoIPMediaChannel* getNewMediaChannel(CHANNEL_TYPE implType);

  MEDIAFILES_NS::EncodedMediaStreamIndex prepareEncodedStream(const MEDIAFILES_NS::MediaStreamInfo *streamInfo);
  MEDIAFILES_NS::EncodedMediaStreamHandler getEncodedStreamByIndex(MEDIAFILES_NS::EncodedMediaStreamIndex index) const;
  MEDIAFILES_NS::EncodedMediaStreamIndex getStreamIndex(const MEDIAFILES_NS::MediaStreamInfo *streamInfo) const;
  void removeEncodedStream(MEDIAFILES_NS::EncodedMediaStreamIndex index);
  void removeAllEncodedStreams();
  void setMediaCompletionDelegation(methodCompletionDelegate_t delegate); 

 private:
  boost::scoped_ptr<VoIPMediaManagerImpl> mPimpl;
};

//////////////////////////////////////////////////

END_DECL_MEDIA_NS

#endif //_MEDIAMANAGER_H_

