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

#ifndef _MEDIAFILES_FILE_H_
#define _MEDIAFILES_FILE_H_

#include <vector>

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

#include "MediaFilesCommon.h"

DECL_MEDIAFILES_NS

/////////////////////////////////////////////////

class MediaCodecPreparator;

class EncodedMediaFile : public boost::noncopyable {

 public:
  EncodedMediaFile(const MediaStreamInfo *streamInfo);
  virtual ~EncodedMediaFile();
  const MediaStreamInfo* getStreamInfo() const;
  int getSize() const;
  MediaPacket* getPacket(int index,int cpu) const;
  void addPacket(MediaPacketHandler packet);
 private:
  boost::scoped_ptr<MediaStreamInfo> mStreamInfo;
  std::vector<MediaPacketHandler> mFile;
  unsigned int mNumberOfCpus;
  unsigned int mPkts;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_MEDIAFILES_NS

#endif //_MEDIAFILES_FILE_H_

