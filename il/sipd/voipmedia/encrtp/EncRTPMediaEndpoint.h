/// @file
/// @brief Enc RTP Endpoint object
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ENCRTP_ENDPOINT_H_
#define _ENCRTP_ENDPOINT_H_

#include <list>

#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include "EncRTPCommon.h"
#include "EncRTPRvInc.h"
#include "MediaFilesCommon.h"
#include "EncRTPGenSession.h"
#include "VQStream.h"
#include "VoIPMediaChannel.h"

class ACE_INET_Addr;
class ACE_SOCK_Dgram;
class ACE_Reactor;

DECL_ENCRTP_NS

/////////////////////////////////////////////////////////////////////////

typedef uint64_t TransportVersion;

class EncRTPTransportEndpointImpl;

class EncRTPTransportEndpoint : public boost::noncopyable {
 public:
  EncRTPTransportEndpoint();
  virtual ~EncRTPTransportEndpoint();
  void closeTransport();
  void lock() const;
  void unlock() const;
  RvRtpSession getRtpSession() const;
  void setRtpSession(RvRtpSession session);
  uint32_t getSockRcvBufferSize() const;
  bool setSockRcvBufferSize(uint32_t);
  bool setSockSndBufferSize(uint32_t);
  ACE_HANDLE get_handle(void) const;
  bool isValid(void) const;
  void setValid(bool val);
  TransportVersion getVersion();
 private:
  boost::scoped_ptr<EncRTPTransportEndpointImpl> mPimpl;
};

/////////////////////////////////////////////////////////////////////////

typedef boost::shared_ptr<EncRTPTransportEndpoint> EncRTPTransportEndpointHandler;

typedef std::list<MediaPacketDescHandler> PacketContainerType;

typedef boost::function2<int,uint64_t,MediaPacketDescHandler> PacketSchedulerFunctionType;

/////////////////////////////////////////////////////////////////////////

class EncRTPMediaEndpointImpl;

class EncRTPMediaEndpoint : public boost::noncopyable {

public:

	explicit EncRTPMediaEndpoint(EncRTPIndex id);
	void init() ;
	virtual ~EncRTPMediaEndpoint();

	bool isInUse() const;
	void setInUse(bool value);

	EncRTPIndex get_id() const;

	VQSTREAM_NS::VoiceVQualResultsHandler getVoiceVQStatsHandler();
	VQSTREAM_NS::VideoVQualResultsHandler getVideoVQStatsHandler();
	VQSTREAM_NS::AudioHDVQualResultsHandler getAudioHDVQStatsHandler();

	int openTransport(MEDIA_NS::STREAM_TYPE type,string ifName, uint16_t vdevblock, ACE_INET_Addr *thisAddr, ACE_INET_Addr *peerAddr, uint8_t tos,uint32_t csrc,int cputype);
	int openMEP(ACE_Reactor *reactor);
	int closeTransport();
	int closeMEP(ACE_Reactor *reactor);
	void lock() const;
	void unlock() const;
	int startListening(ACE_Reactor *reactor,
			   MEDIAFILES_NS::EncodedMediaStreamHandler file,
			   VQSTREAM_NS::VQStreamManager *vqStreamManager,
			   VQSTREAM_NS::VQInterfaceHandler interface);
	int stopListening(ACE_Reactor *reactor);
	int startTalking(ACE_Reactor *reactor,PacketSchedulerFunctionType psfunc);
	int stopTalking(ACE_Reactor *reactor);
	int release(ACE_Reactor *reactor,TrivialFunctionType f);

	bool isOpen(void) const;
	bool isReady(void) const;
	void setReady(bool value);
	bool isListening(void) const;
	bool isTalking(void) const;

	uint32_t getProcessID() const;
	void setProcessID(uint32_t pid);
    ///TIP interfaces
    int startTIPNegotiate(ACE_Reactor *reactor,TipStatusDelegate_t cb);
    int stopTIPNegotiate(void);
    int recvPacket(EndpointHandler ep);



	static int sendPacket(MediaPacketDescHandler pi,int cpu);
    EncRTPTransportEndpointHandler getTransportHandler();

private:
	boost::scoped_ptr<EncRTPMediaEndpointImpl> mPimpl;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_ENCRTP_NS

///////////////////////////////////////////////////////////////////////////////

#endif //_ENCRTP_ENDPOINT_H_

