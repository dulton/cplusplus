/// @file
/// @brief VQStats header
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _VQSTATSDATA_H_
#define _VQSTATSDATA_H_

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "VoIPUtils.h"
#include "VQCommon.h"
#include "VQStats.h"

////////////////////////////////////////////////////////////////////////

DECL_VQSTREAM_NS

///////////////////////////////////////////////////////////////////////////////

struct VoiceVQualResultsRawData {

  volatile bool dirty;
  VQualResultsBlockHandler block;

  typedef struct {
     uint16_t MOS_LQ;
     uint16_t MOS_CQ;
     uint8_t R_LQ;
     uint8_t R_CQ;
     uint16_t PPDV;
     uint32_t PktsRcvd;
     uint32_t PktsLost;
     uint32_t PktsDiscarded;
  } VoiceVQData;

  VoiceVQualResultsRawData() {
    memset(&data, 0, sizeof(data));
    dirty=false;
  }
  
  void set(VoiceVQData& other);

  void clean() {
     VoIPGuard(mMutex);
     memset(&data, 0, sizeof(data));
     dirty=false;
  }

  void start() {
    if(block) block->addHandler(this);
  }
  
  void stop() {
    if(block) block->removeHandler(this);
  }

  VoIPUtils::MutexType mMutex;
  VoiceVQData data;
};

/////////////////////////////////////////////////////////////////////////////////////////////

struct AudioHDVQualResultsRawData {

  volatile bool dirty;

  VQualResultsBlockHandler block;

  typedef struct {
    uint16_t InterMOS_A;
    uint32_t AudioInterPktsRcvd;
    uint32_t InterPktsLost;
    uint32_t InterPktsDiscarded;
    uint32_t EffPktsLossRate;
    uint16_t MinMOS_A;
    uint16_t AvgMOS_A;
    uint16_t MaxMOS_A;
    uint32_t PropBelowTholdMOS_A;
    uint8_t  LossDeg;
    uint32_t PktsRcvd;
    uint32_t PktsLoss;
    uint32_t UncorrectedLossProp;
    uint16_t BurstCount;
    uint32_t BurstLossRate;
    uint32_t BurstLength;
    uint16_t GapCount;
    uint32_t GapLossRate;
    uint32_t GapLength;
    uint16_t PPDV;
  } AudioHDVQData;  

  AudioHDVQualResultsRawData() {
    memset(&data, 0, sizeof(data));    
    dirty=false;
  }

  void set(AudioHDVQData& other);  

  void clean() {
    VoIPGuard(mMutex);
    memset(&data, 0, sizeof(data));    
    dirty=false;
  }

  void start() {
    if(block) block->addHandler(this);
  }
  
  void stop() {
    if(block) block->removeHandler(this);
  }

  VoIPUtils::MutexType mMutex;
  AudioHDVQData data;   
};

/////////////////////////////////////////////////////////////////////////////////////////////

struct VideoVQualResultsRawData {

  volatile bool dirty;

  VQualResultsBlockHandler block;

  typedef struct {
    uint16_t InterAbsoluteMOS_V;
    uint16_t InterRelativeMOS_V;
    uint32_t InterPktsRcv;
    uint32_t InterPktsLost;
    uint32_t InterPktsDiscarded;
    uint32_t InterEffPktsLossRate;
    uint16_t InterPPDV;
    uint32_t AvgVideoBandwidth;
    uint16_t MinAbsoluteMOS_V;
    uint16_t AvgAbsoluteMOS_V;
    uint16_t MaxAbsoluteMOS_V;
    uint16_t MinRelativeMOS_V;
    uint16_t AvgRelativeMOS_V;
    uint16_t MaxRelativeMOS_V;
    uint8_t  LossDeg;
    uint32_t PktsRcvd;
    uint32_t PktsLost;
    uint32_t UncorrectedLossProp;
    uint16_t BurstCount;
    uint32_t BurstLossRate;
    uint32_t BurstLength;
    uint16_t GapCount;
    uint32_t GapLossRate;
    uint32_t GapLength;
    int16_t  IFrameInterArrivalJitter;
    uint16_t PPDV;
  } VideoVQData; 
  
  VideoVQualResultsRawData() {
    memset(&data, 0, sizeof(data));
    dirty=false;
  }

  void clean() {
    VoIPGuard(mMutex);
    memset(&data, 0, sizeof(data));    
    dirty=false;
  }

  void set(VideoVQData& other);  

  void start() {
    if(block) block->addHandler(this);
  }
  
  void stop() {
    if(block) block->removeHandler(this);
  }

  VoIPUtils::MutexType mMutex;
  VideoVQData data;      
};

/////////////////////////////////////////////////////////////////////////////////////////////

struct VoiceVQualResultsSnapshot {
    
  uint8_t r107;
  uint8_t r_LQ;
  uint8_t r_CQ;
  double MOS_LQ;
  double MOS_CQ;
  
  VoiceVQualResultsSnapshot() : r107(0),r_LQ(0),r_CQ(0),MOS_LQ(0),MOS_CQ(0) {}
  VoiceVQualResultsSnapshot(const VoiceVQualResultsRawData &rawData);
  VoiceVQualResultsSnapshot(VoiceVQualResultsRawData* const rawData);
  VoiceVQualResultsSnapshot(const VoiceVQualResultsSnapshot &other);
  VoiceVQualResultsSnapshot(const VoiceVQualResultsHandler &other);
  VoiceVQualResultsSnapshot& operator=(const VoiceVQualResultsSnapshot &other);
  VoiceVQualResultsSnapshot& operator=(const VoiceVQualResultsRawData &otherRawData);
  VoiceVQualResultsSnapshot& operator=(VoiceVQualResultsRawData* const otherRawData);
};

///////////////////////////////////////////////////////////////////////////

END_DECL_VQSTREAM_NS

#endif //_VQSTATSDATA_H_

