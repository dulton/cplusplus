/// @file
/// @brief MediaStreamInfo defs
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _MEDIAFILES_MSI_H_
#define _MEDIAFILES_MSI_H_

#include <string>

#include <boost/scoped_ptr.hpp>

#include "MediaFilesCommon.h"

DECL_MEDIAFILES_NS

/////////////////////////////////////////////////////////////////////////////

typedef std::string MediaFilePath;

struct MediaStreamData;

class MediaStreamInfo {

 public:

  MediaStreamInfo();
  MediaStreamInfo(const MediaStreamInfo* other);
  MediaStreamInfo(const MediaStreamInfo& other);

  virtual ~MediaStreamInfo();

  MediaStreamInfo& operator=(const MediaStreamInfo& other);

  bool operator<(const MediaStreamInfo& other) const;
  bool operator>(const MediaStreamInfo& other) const;
  bool operator==(const MediaStreamInfo& other) const;
  bool operator!=(const MediaStreamInfo& other) const;

  void init();

  CodecType getCodec() const;
  CodecGroupType getCodecGroup() const;
  void setCodecAndPayloadNumber(CodecType codec,PayloadType pn);
  PayloadType getPayloadNumber() const;
  int getDefaultPacketLength() const; //in milliseconds
  void setDefaultPacketLength(int ms);
  int getBitRate() const; //bits per second
  void setBitRate(int br);
  int getSamplingRate() const; //samples per millisecond
  void setSamplingRate(int sr);
  MediaFilePath getFilePath() const;
  void setFilePath(MediaFilePath filePath);
  bool getFakeEncStreamGeneration()const;
  void setFakeEncStreamGeneration(bool FakeEncodedStream);
 private:

  boost::scoped_ptr<MediaStreamData> mData;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_MEDIAFILES_NS

#endif //_MEDIAFILES_MSI_H_

