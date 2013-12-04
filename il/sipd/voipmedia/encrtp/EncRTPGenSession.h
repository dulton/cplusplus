/// @file
/// @brief Enc RTP Gen object
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ENCRTP_GENSESS_H_
#define _ENCRTP_GENSESS_H_

#include <boost/scoped_ptr.hpp>

#include "EncRTPCommon.h"
#include "MediaPacket.h"
#include "MediaFilesCommon.h"

DECL_ENCRTP_NS

//////////////////////////////////////////////////////////////
//


class MediaPacketDesc : public boost::noncopyable{

 public:

  MediaPacketDesc();
  virtual ~MediaPacketDesc(){}

  uint64_t getTime() const{return mTime;}
  void setTime(uint64_t ctime){mTime = ctime;}
  void setEp(EndpointHandler ep); 
  void setmFile(MEDIAFILES_NS::EncodedMediaStreamHandler file){mFile = file;}
  EndpointHandler getEp() const {return mEp;}

  void setIndex(uint32_t index) {mIndex = index;}

  MEDIAFILES_NS::MediaPacket* getPacket(int cpu);

  void setModifier(MEDIAFILES_NS::RtpHeaderModifier &mo);


 private:
  uint64_t mTime;
  MEDIAFILES_NS::EncodedMediaStreamHandler mFile;
  MEDIAFILES_NS::RtpHeaderModifier mHeaderModifier;
  uint32_t mIndex;
  EndpointHandler mEp;
};

typedef boost::shared_ptr<MediaPacketDesc> MediaPacketDescHandler;

class EncRTPGenSessionImpl;

class EncRTPGenSession : public boost::noncopyable {
 public:
  explicit EncRTPGenSession(MEDIAFILES_NS::EncodedMediaStreamHandler file);
  virtual ~EncRTPGenSession();
  MEDIAFILES_NS::EncodedMediaStreamHandler getEncodedMediaStream() const;
  int start(uint64_t startTime, uint64_t schedulerInterval);
  void nextSchedulingInterval();
  MediaPacketDescHandler getNextPacketDesc(EndpointHandler ep,uint32_t csrc); 
  int stop();
 private:
  boost::scoped_ptr<EncRTPGenSessionImpl> mPimpl;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_ENCRTP_NS

#endif //_ENCRTP_GENSESS_H_

