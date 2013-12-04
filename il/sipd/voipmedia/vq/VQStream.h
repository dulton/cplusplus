/// @file
/// @brief VQStream header
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _VQSTREAM_H_
#define _VQSTREAM_H_

#include <stdint.h>
#include <time.h>

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "VQCommon.h"
#include "VQStats.h"
#include "MediaPacket.h"

////////////////////////////////////////////////////////////////////////

DECL_VQSTREAM_NS

///////////////////////////////////////////////////////////////////////////////

/**
 * The packet information we actually need for the calculations
 */
struct VqmaRtpPacketInfo {
  struct timeval localTimestamp; //timestamp of the packet receiving moment
  MEDIAFILES_NS::MediaPacketHandler packet; //RTP packet
  VqmaRtpPacketInfo() {}
  VqmaRtpPacketInfo(const VqmaRtpPacketInfo& other) {
    localTimestamp=other.localTimestamp;
    packet=other.packet;
  }
  VqmaRtpPacketInfo& operator=(const VqmaRtpPacketInfo& other) {
    if(&other!=this) {
      localTimestamp=other.localTimestamp;
      packet=other.packet;
    }
    return *this;
  }
};

///////////////////////////////////////////////////////////////////////////

class VQStream : public boost::noncopyable {
  
 public:
  
  VQStream() { };
  virtual ~VQStream() { };

  /**
   * Feed a packet to the stream
   */
  virtual int packetRecv(VqmaRtpPacketInfo packetInfo) = 0;
   
  /**
   * Get current VQ values for the stream.
   * Return -1 if the stream has not been fed properly.*/
  virtual int getVoiceMetrics(VoiceVQualResultsHandler metrics) = 0;
  virtual int getAudioHDMetrics(AudioHDVQualResultsHandler metrics) = 0;
  virtual int getVideoMetrics(VideoVQualResultsHandler metrics) = 0;

  virtual VQMediaType getMediaType() const = 0;
  virtual bool getStreamInited() const = 0;  
};

typedef boost::shared_ptr<VQStream> VQStreamHandler;

///////////////////////////////////////////////////////////////////////////

class VQInterface : public boost::noncopyable {
 public:
  VQInterface() {}
  virtual ~VQInterface() {}
};

typedef boost::shared_ptr<VQInterface> VQInterfaceHandler;

///////////////////////////////////////////////////////////////////////////

class VQStreamManagerImpl;

class VQStreamManager : public boost::noncopyable {

 public:

  static const int DEFAULT_JITTER_BUFFER_SIZE = 0;

  VQStreamManager();
  virtual ~VQStreamManager();
  void clean();
  VQInterfaceHandler createNewInterface();
  VQStreamHandler createNewStream(VQInterfaceHandler interface,
				  MEDIAFILES_NS::CodecType codec, 
				  MEDIAFILES_NS::PayloadType payload,
				  int bitRate,
				  uint32_t ssrc,
				  int jbSize = DEFAULT_JITTER_BUFFER_SIZE);

 private:
  boost::scoped_ptr<VQStreamManagerImpl> mPimpl;
};

//////////////////////////////////////////////////////////////

END_DECL_VQSTREAM_NS

#endif //_VQSTREAM_H_

