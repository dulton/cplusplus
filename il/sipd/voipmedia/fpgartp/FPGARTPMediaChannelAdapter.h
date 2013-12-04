/// @file
/// @brief FPGARTP MediaChannel class
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _FPGARTPMEDIACHANNEL_H_
#define _FPGARTPMEDIACHANNEL_H_

#include <boost/scoped_ptr.hpp>

#include "VoIPMediaChannel.h"

#include "FPGARTPCommon.h"
#include "GeneratorClientHandler.h"

DECL_FPGARTP_NS

/////////////////////////////////////////////////

class FPGARTPMediaChannelAdapterImpl;

class FPGARTPMediaChannelAdapter : public MEDIA_NS::VoIPMediaChannel {

 public:

  FPGARTPMediaChannelAdapter(GeneratorClientHandlerSharedPtr genClientHandler);
  virtual ~FPGARTPMediaChannelAdapter();

  virtual CHANNEL_TYPE getChannelType() const { return SIMULATED_RTP; }

  virtual MEDIA_NS::CALL_TYPE getCallType() const;
  virtual int setCallType(MEDIA_NS::CALL_TYPE callType);

  virtual int setStatFlag(STREAM_TYPE streamType, bool val);
  virtual bool getStatFlag(STREAM_TYPE streamType) const;		  

  virtual VQSTREAM_NS::VoiceVQualResultsHandler getVoiceVQStatsHandler();
  virtual VQSTREAM_NS::AudioHDVQualResultsHandler getAudioHDVQStatsHandler();
  virtual VQSTREAM_NS::VideoVQualResultsHandler getVideoVQStatsHandler();

  virtual int setFPGAStreamIndex(MEDIA_NS::STREAM_TYPE streamType, MEDIA_NS::RTP_STREAM_TYPE rtpStreamType, 
				 uint32_t streamIndex);

  virtual uint32_t getPort(MEDIA_NS::STREAM_TYPE streamType) const;
  virtual uint16_t getVDevBlock(MEDIA_NS::STREAM_TYPE streamType) const;
  virtual uint32_t getIfIndex(MEDIA_NS::STREAM_TYPE streamType) const;
  virtual const ACE_INET_Addr* getLocalAddr(MEDIA_NS::STREAM_TYPE streamType) const;
  virtual uint32_t getLocalTransportPort(MEDIA_NS::STREAM_TYPE streamType) const;
  virtual uint32_t getRemoteTransportPort(MEDIA_NS::STREAM_TYPE streamType) const;
  virtual const ACE_INET_Addr* getRemoteAddr(MEDIA_NS::STREAM_TYPE streamType) const;
  virtual MEDIAFILES_NS::EncodedMediaStreamIndex getFileIndex(MEDIA_NS::STREAM_TYPE streamType) const;

  virtual int setLocalAddr(MEDIA_NS::STREAM_TYPE streamType,
			   uint32_t port,
			   uint32_t ifIndex,
			   uint16_t vdevblock,
			   const ACE_INET_Addr *thisAddr,
			   uint32_t transportPort = 0);
  virtual int setLocalTransportPort(MEDIA_NS::STREAM_TYPE streamType,
			      uint32_t transportPort);
  virtual int setRemoteAddr(MEDIA_NS::STREAM_TYPE streamType,
			    const ACE_INET_Addr* destAddr, 
			    uint16_t destTransportPort = 0);
  virtual void setFileIndex(MEDIA_NS::STREAM_TYPE streamType,MEDIAFILES_NS::EncodedMediaStreamIndex file);

  virtual int start(MEDIA_NS::STREAM_TYPE streamType,TrivialBoolFunctionType act=0);
  virtual int stop(MEDIA_NS::STREAM_TYPE streamType,TrivialBoolFunctionType act=0);
  virtual int startAll(TrivialBoolFunctionType act=0);
  virtual int stopAll(TrivialBoolFunctionType act=0);

  virtual void NotifyInterfaceDisabled(void);
  virtual void setIpServiceLevel(uint8_t ipServiceLevel); 
  virtual uint8_t getIpServiceLevel(); 

  virtual bool isAsync() const { return true; }
  virtual int startTip(TipStatusDelegate_t act,uint32_t mux_csrc){return 0;}

 private:
  boost::scoped_ptr<FPGARTPMediaChannelAdapterImpl> mPimpl;
};

//////////////////////////////////////////////////

END_DECL_FPGARTP_NS

#endif //_FPGARTPMEDIACHANNEL_H_

