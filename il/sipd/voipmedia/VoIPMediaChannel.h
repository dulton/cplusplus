/// @file
/// @brief MediaChannel class
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _MEDIACHANNEL_H_
#define _MEDIACHANNEL_H_

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "VoIPCommon.h"
#include "MediaFilesCommon.h"
#include "VQStats.h"
#include "EncTIP.h"

class ACE_INET_Addr;

DECL_MEDIA_NS

/////////////////////////////////////////////////

enum CHANNEL_TYPE {
  NOP_RTP=0,
  SIMULATED_RTP,
  DEFAULT_CHANNEL_TYPE=SIMULATED_RTP,
  ENCODED_RTP,
  TOTAL_CHANNEL_TYPES
};

enum STREAM_TYPE {
  UNKNOWN_STREAM = -1,
  AUDIO_STREAM = 0,
  VOICE_STREAM=AUDIO_STREAM,
  DEFAULT_STREAM=AUDIO_STREAM,
  VIDEO_STREAM,
  AUDIOHD_STREAM,
  TOTAL_STREAM_TYPES
};

enum CALL_TYPE {
  SIG_ONLY = 0,
  AUDIO_ONLY,
  DEFAULT_CALL_TYPE=AUDIO_ONLY,
  VIDEO_ONLY,
  AUDIOHD_ONLY,
  AUDIO_VIDEO,
  AUDIOHD_VIDEO,
  TOTAL_CALL_TYPES
};

enum RTP_STREAM_TYPE {
  UNKNOWN_RTP_STREAM_TYPE = -1,
  RTP_STREAM = 0,
  DEFAULT_RTP_STREAM_TYPE = RTP_STREAM,
  RTCP_STREAM,
  TOTAL_RTP_STREAM_TYPES
};

/////////////////////////////////////////////////

class VoIPMediaChannel : public boost::noncopyable {

 public:

  VoIPMediaChannel() {};
  virtual ~VoIPMediaChannel() {};

  virtual CHANNEL_TYPE getChannelType() const = 0;

  virtual CALL_TYPE getCallType() const = 0;
  virtual int setCallType(CALL_TYPE callType) = 0;
  virtual int setStatFlag(STREAM_TYPE streamType, bool val) = 0;
  virtual bool getStatFlag(STREAM_TYPE streamType) const = 0;
  
  virtual VQSTREAM_NS::VoiceVQualResultsHandler getVoiceVQStatsHandler() = 0;
  virtual VQSTREAM_NS::AudioHDVQualResultsHandler getAudioHDVQStatsHandler() = 0;
  virtual VQSTREAM_NS::VideoVQualResultsHandler getVideoVQStatsHandler() = 0;

  virtual int setFPGAStreamIndex(STREAM_TYPE streamType, RTP_STREAM_TYPE rtpRtreamType,
				 uint32_t streamIndex) = 0;

  virtual uint32_t getPort(STREAM_TYPE streamType) const = 0;
  virtual uint16_t getVDevBlock(STREAM_TYPE streamType) const = 0;
  virtual uint32_t getIfIndex(STREAM_TYPE streamType) const = 0;
  virtual const ACE_INET_Addr* getLocalAddr(STREAM_TYPE streamType) const = 0;
  virtual uint32_t getLocalTransportPort(STREAM_TYPE streamType) const = 0;
  virtual uint32_t getRemoteTransportPort(STREAM_TYPE streamType) const = 0;
  virtual const ACE_INET_Addr* getRemoteAddr(STREAM_TYPE streamType) const = 0;
  virtual MEDIAFILES_NS::EncodedMediaStreamIndex getFileIndex(STREAM_TYPE streamType) const = 0;

  virtual int setLocalAddr(STREAM_TYPE streamType,
			   uint32_t port,
			   uint32_t ifIndex,
			   uint16_t vdevblock,
			   const ACE_INET_Addr *thisAddr,
			   uint32_t transportPort = 0) = 0;
  virtual int setLocalTransportPort(STREAM_TYPE streamType,
			      uint32_t transportPort) = 0;
  virtual int setRemoteAddr(STREAM_TYPE streamType,
			    const ACE_INET_Addr* destAddr, 
			    uint16_t destTransportPort = 0) = 0;
  virtual void setFileIndex(STREAM_TYPE streamType, MEDIAFILES_NS::EncodedMediaStreamIndex fileIndex) = 0;

  virtual int startAll(VOIP_NS::TrivialBoolFunctionType act=0) = 0;
  virtual int stopAll(VOIP_NS::TrivialBoolFunctionType act=0) = 0;

  virtual void NotifyInterfaceDisabled(void) = 0;

  // IPv4 ToS / IPv6 traffic class mutator
  virtual uint8_t getIpServiceLevel() { return 0; }
  virtual void setIpServiceLevel(uint8_t ipServiceLevel) { }

  //Flag: whether the control commands on this channel are asynchronous
  virtual bool isAsync() const { return false; }
  virtual int startTip(TipStatusDelegate_t act,uint32_t mux_csrc) = 0;
};

typedef boost::shared_ptr<VoIPMediaChannel> MediaChannelHandler;

//////////////////////////////////////////////////

END_DECL_MEDIA_NS

#endif //_MEDIACHANNEL_H_

