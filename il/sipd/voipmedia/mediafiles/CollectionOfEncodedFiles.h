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

#ifndef _MEDIAFILES_FILECOLLEC_H_
#define _MEDIAFILES_FILECOLLEC_H_

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

#include "MediaFilesCommon.h"

DECL_MEDIAFILES_NS

/////////////////////////////////////////////////

class CollectionOfEncodedFilesImpl;

class CollectionOfEncodedFiles : public boost::noncopyable {
 public:
  CollectionOfEncodedFiles();
  virtual ~CollectionOfEncodedFiles();
  EncodedMediaStreamIndex getStreamIndex(const MediaStreamInfo *streamInfo) const;
  EncodedMediaStreamHandler getEncodedStreamByIndex(EncodedMediaStreamIndex index) const;
  EncodedMediaStreamIndex addEncodedStream(EncodedMediaStreamHandler streamHandler);
  void removeEncodedStream(EncodedMediaStreamIndex index);
  void removeAllEncodedStreams();
 private:
  boost::scoped_ptr<CollectionOfEncodedFilesImpl> mPimpl;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_MEDIAFILES_NS

#endif //_MEDIAFILES_FILECOLLEC_H_

