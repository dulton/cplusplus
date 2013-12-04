/// @file
/// @brief Enc RTP Media Manager object
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ENCRTP_MEDIAMANAGER_H_
#define _ENCRTP_MEDIAMANAGER_H_

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

#include "EncRTPCommon.h"
#include "VoIPMediaChannel.h"
#include "MediaFilesCommon.h"
#include "VQStats.h"
#include "VQStream.h"

class ACE_INET_Addr;

DECL_ENCRTP_NS

/////////////////////////////////////////

// Forward declarations
class EncRTPMediaManagerImpl;

///////////////////////////////////////////////////////

class EncRTPMediaManager : public boost::noncopyable {
 public:
  EncRTPMediaManager(); //constructor
  virtual ~EncRTPMediaManager(); //destructor

  EncRTPIndex getNewEndpointIndex(uint32_t parentID); //create new enc rtp endpoint

  VQSTREAM_NS::VoiceVQualResultsHandler getVoiceVQStatsHandler(EncRTPIndex id);
  VQSTREAM_NS::AudioHDVQualResultsHandler getAudioHDVQStatsHandler(EncRTPIndex id);
  VQSTREAM_NS::VideoVQualResultsHandler getVideoVQStatsHandler(EncRTPIndex id);

  int open(EncRTPIndex id,MEDIA_NS::STREAM_TYPE type, 
	   unsigned int ifIndex, uint16_t vdevblock, ACE_INET_Addr *thisAddr, ACE_INET_Addr *peerAddr,uint8_t tos,uint32_t csrc=0); //open endpoint
  int close(EncRTPIndex id); //close endpoint
  int release(EncRTPIndex id); //destroy endpoint
  
  int startListening(EncRTPIndex id, MEDIAFILES_NS::EncodedMediaStreamHandler file,VQSTREAM_NS::VQInterfaceHandler interface); //start listening to RTP packets
  int stopListening(EncRTPIndex id); //stop listening
  int startTalking(EncRTPIndex id); //start sending RTP packets
  int stopTalking(EncRTPIndex id); //stop sending
  int startTip(EncRTPIndex id,TipStatusDelegate_t act);
  
  bool isOpen(EncRTPIndex id) const; //check whether open
  bool isListening(EncRTPIndex id) const; //check whether listening
  bool isTalking(EncRTPIndex id) const; //check whether talking

  VQSTREAM_NS::VQInterfaceHandler createNewInterface(); //create new VQmon interface

private:
  boost::scoped_ptr<EncRTPMediaManagerImpl> mPimpl; //internal implementation
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_ENCRTP_NS

#endif //_ENCRTP_MEDIAMANAGER_H_

