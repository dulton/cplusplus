/// @file
/// @brief MediaFiles processing classes
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _MEDIAFILES_FILESMANAGER_H_
#define _MEDIAFILES_FILESMANAGER_H_

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

#include "MediaFilesCommon.h"

DECL_MEDIAFILES_NS

//////////////////////////////////////////////////////////////

class MediaFilesManagerImpl;

class MediaFilesManager : public boost::noncopyable {
 public:
  MediaFilesManager();
  virtual ~MediaFilesManager();
  EncodedMediaStreamIndex prepareEncodedStream(const MediaStreamInfo *streamInfo);
  EncodedMediaStreamHandler getEncodedStreamByIndex(EncodedMediaStreamIndex index) const;
  EncodedMediaStreamIndex getStreamIndex(const MediaStreamInfo *streamInfo) const;
  void removeEncodedStream(EncodedMediaStreamIndex index);
  void removeAllEncodedStreams();
 private:
  boost::scoped_ptr<MediaFilesManagerImpl> mPimpl;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_MEDIAFILES_NS

#endif //_MEDIAFILES_FILESMANAGER_H_

