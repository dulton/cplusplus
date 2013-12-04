/// @file
/// @brief Media Files Common defs
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _MEDIAFILES_PACKET_H_
#define _MEDIAFILES_PACKET_H_

#include <stdint.h>

#include <boost/scoped_ptr.hpp>

#include "VoIPUtils.h"
#include "MediaFilesCommon.h"

DECL_MEDIAFILES_NS

//////////////////////////////////////////////////////////////////////////////

typedef struct tRtpHeaderParam
{
    uint16_t  rtp_sn;      // sequence number
    uint8_t   rtp_pl;      // payload type
    int32_t   rtp_mark;    // marker bit
    uint8_t   rtp_v;       // version
    uint8_t   rtp_pad;     // padding
    uint8_t   rtp_ext;     // extension
    uint8_t   rtp_csrcc;   // csrc count
    uint32_t  rtp_ts;      // timestamp
    uint32_t  rtp_ssrc;    //synchronization source
} RtpHeaderParam;

typedef struct tRtpHeaderModifier{
    uint16_t rtp_mf_sn;
    uint32_t rtp_mf_csrcc;
    uint32_t rtp_mf_ts;
    uint32_t rtp_mf_ssrc;

}RtpHeaderModifier;

//////////////////////////////////////////////////////////////////////////////

class MediaPacketImplNew;

class MediaPacket {

 public:

  static const int HEADER_SIZE = 12;
  static const int BODY_MAX_SIZE = 2048;
  static const int PAYLOAD_MAX_SIZE = 2036;
  static const int BODY_DEFAULT_SIZE = 200;
  static const int DEFAULT_RTP_PKT_SIZE = 256;
  static const int BUFFER_SIZE = 20;
  static const uint8_t DUMMY_PAYLOAD_TYPE  = (72);

  MediaPacket();
  explicit MediaPacket(int pl);
  virtual ~MediaPacket();
  MediaPacket(const MediaPacket& other);
  MediaPacket(const MediaPacket& other,uint32_t csrc);
  MediaPacket& operator=(const MediaPacket& other);
  bool operator<(const MediaPacket& other) const;

  uint64_t getTime() const;
  void setTime(uint64_t ctime);

  static int getMinSize();
  static int getMinPayloadSize();

  int getMaxSize() const;
  void setMaxSize();
  void setMaxSize(int size);

  int getMaxPayloadSize() const;
  void setMaxPayloadSize();
  void setMaxPayloadSize(int pl);

  int getSize() const;
  void setSize(int size);

  int getPayloadSize() const;
  void setPayloadSize(int pl);

  uint8_t* getStartPtr();
  uint8_t* getPayloadPtr();
  uint8_t* getLiveDataPtr();
  uint32_t getLiveDataSize();

  
  void setRtpHeaderParam ( RtpHeaderParam &rtpParam );
  void getRtpHeaderParam ( RtpHeaderParam &rtpParam ) const;
  void getLiveRtpHeaderParam ( RtpHeaderParam &rtpParam ) const;
  void RtpHeaderParamModify(RtpHeaderModifier &modifier);
 private:
  boost::scoped_ptr<MediaPacketImplNew> mPimpl;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_MEDIAFILES_NS

#endif //_MEDIAFILES_PACKET_H_

