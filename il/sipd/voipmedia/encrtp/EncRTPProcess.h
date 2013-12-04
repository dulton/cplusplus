/// @file
/// @brief Enc RTP Process object
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ENCRTP_PROCESS_H_
#define _ENCRTP_PROCESS_H_

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

#include <ace/Select_Reactor.h>

#include "EncRTPCommon.h"
#include "MediaFilesCommon.h"
#include "VQStream.h"

class ACE_INET_Addr;

DECL_ENCRTP_NS

///////////////////////////////////////////////////////////////////////////////

typedef ACE_Select_Reactor ReactorType;

///////////////////////////////////////////////////////////////////////////////

class EncRTPProcessImpl;

class EncRTPProcess : public boost::noncopyable {
  
 public:

  static const size_t MAX_PACKETS_IN_BUCKET = 512;
  
  EncRTPProcess(int size,int cpu);
  virtual ~EncRTPProcess();
  int open(EndpointHandler ep,STREAM_TYPE type,
	   std::string ifName, uint16_t vdevblock, ACE_INET_Addr *thisAddr, ACE_INET_Addr *peerAddr,
	   uint8_t tos,uint32_t csrc);
  int startListening(EndpointHandler ep,
		     MEDIAFILES_NS::EncodedMediaStreamHandler file,
		     VQSTREAM_NS::VQStreamManager *vqStreamManager,
		     VQSTREAM_NS::VQInterfaceHandler interface);
  int stopListening(EndpointHandler ep);
  int startTalking(EndpointHandler ep);
  int stopTalking(EndpointHandler ep);
  int close(EndpointHandler ep);
  int release(EndpointHandler ep, TrivialFunctionType f);
  int startTip(EndpointHandler ep,TipStatusDelegate_t cb);
  
 private:
  boost::scoped_ptr<EncRTPProcessImpl> mPimpl;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_ENCRTP_NS

///////////////////////////////////////////////////////////////////////////////

#endif //_ENCRTP_PROCESS_H_

