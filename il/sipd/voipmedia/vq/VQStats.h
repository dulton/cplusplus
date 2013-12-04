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

#ifndef _VQSTATS_H_
#define _VQSTATS_H_

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "VQCommon.h"

////////////////////////////////////////////////////////////////////////

DECL_VQSTREAM_NS

///////////////////////////////////////////////////////////////////////////////

struct VoiceVQualResultsRawData;
struct AudioHDVQualResultsRawData;
struct VideoVQualResultsRawData;

typedef boost::shared_ptr<VoiceVQualResultsRawData> VoiceVQualResultsHandler;
typedef boost::shared_ptr<AudioHDVQualResultsRawData> AudioHDVQualResultsHandler;
typedef boost::shared_ptr<VideoVQualResultsRawData> VideoVQualResultsHandler;

///////////////////////////////////////////////////////////////////////////

struct VoiceVQualResultsBlockSnapshot {
  static const uint64_t SnapshotSyncTimeout = 1000;
  static const uint64_t SnapshotCurrDataTimeout = 5000;
  double minMOS_LQ;
  double avgMOS_LQ;
  long double totalMOS_LQ;
  uint64_t totalMgmts;
  double maxMOS_LQ;
  double avgCurrMOS_LQ;

  double minMOS_CQ;
  double avgMOS_CQ;
  double maxMOS_CQ;

  uint8_t minR_LQ;
  uint8_t avgR_LQ;
  uint8_t maxR_LQ;

  uint8_t minR_CQ;
  uint8_t avgR_CQ;
  uint8_t maxR_CQ;

  double avgPPDV;
  double maxPPDV;

  uint64_t totalPktsRcvd;
  uint64_t totalPktsLost;
  uint64_t totalPktsDiscarded;

  uint64_t totalElapsPktsRcvd;
  uint64_t totalElapsPktsLost;
  uint64_t totalElapsPktsDiscarded;

  uint32_t minPktsLost;
  uint32_t avgPktsLost;
  uint32_t maxPktsLost;

  uint32_t minPktsDiscarded;
  uint32_t avgPktsDiscarded;
  uint32_t maxPktsDiscarded;

  VoiceVQualResultsBlockSnapshot() {
     minMOS_LQ = 0.0;
     avgMOS_LQ = 0.0;
     maxMOS_LQ = 0.0;
     totalMOS_LQ = 0.0;
     totalMgmts = 0;
     avgCurrMOS_LQ = 0.0;

     minMOS_CQ = 0.0;
     avgMOS_CQ = 0.0;
     maxMOS_CQ = 0.0;    

     minR_LQ = 0;
     avgR_LQ = 0;
     maxR_LQ = 0;

     minR_CQ = 0;
     avgR_CQ = 0;
     maxR_CQ = 0;

     avgPPDV = 0.0;
     maxPPDV = 0.0;

     totalPktsRcvd = 0;
     totalPktsLost = 0;
     totalPktsDiscarded = 0;
     
     totalElapsPktsRcvd = 0;
     totalElapsPktsLost = 0;
     totalElapsPktsDiscarded = 0;

     minPktsLost = 0;
     avgPktsLost = 0;
     maxPktsLost = 0;
     
     minPktsDiscarded = 0;
     avgPktsDiscarded = 0;
     maxPktsDiscarded = 0;   
  }

  static double mostog (uint16_t mos)
  {
     return (double)mos / 256.0;
  }


  static double fpq4tog (uint16_t fpq4)
  {
     return (double)fpq4 / 16.0;
  }
};

struct AudioHDVQualResultsBlockSnapshot {
  static const uint64_t SnapshotSyncTimeout = 1000;
  static const uint64_t SnapshotCurrDataTimeout = 5000;
  double InterMOS_A;
  uint32_t AudioInterPktsRcvd;
  uint32_t InterPktsLost;
  uint32_t InterPktsDiscarded;
  double EffPktsLossRate;
  double MinMOS_A;
  double AvgMOS_A;
  double MaxMOS_A;
  double PropBelowTholdMOS_A;
  uint8_t  LossDeg;
  uint32_t PktsRcvd;
  uint32_t PktsLoss;
  double UncorrectedLossProp;
  uint32_t BurstCount;
  double BurstLossRate;
  uint32_t BurstLength;
  uint32_t GapCount;
  double GapLossRate;
  uint32_t GapLength;
  double PPDV;

  AudioHDVQualResultsBlockSnapshot() {
    InterMOS_A = 0.0;
    AudioInterPktsRcvd = 0;
    InterPktsLost = 0;
    InterPktsDiscarded = 0;
    EffPktsLossRate = 0.0;
    MinMOS_A = AvgMOS_A = MaxMOS_A = 0.0;
    PropBelowTholdMOS_A = 0.0;
    LossDeg = 0;
    PktsRcvd = 0;
    PktsLoss = 0;
    UncorrectedLossProp = 0.0;
    BurstCount = 0;
    GapCount = 0;
    GapLossRate = 0.0;
    GapLength = 0;
    PPDV = 0.0;
  }

  static double fpq4tog (uint16_t fpq4)
  {
    return (double)fpq4 / 16.0;
  }

  static double mostog (uint16_t mos)
  {
    return (double)mos / 256.0;
  }

  static double proptog (uint32_t prop)
  {
    return (double)prop / 4294967296.0;
  }
};

struct VideoVQualResultsBlockSnapshot {
  static const uint64_t SnapshotSyncTimeout = 1000;
  static const uint64_t SnapshotCurrDataTimeout = 5000;
  double InterAbsoluteMOS_V;
  double InterRelativeMOS_V;
  uint32_t InterPktsRcv;
  uint32_t InterPktsLost;
  uint32_t InterPktsDiscarded;
  double InterEffPktsLossRate;
  double InterPPDV;
  uint32_t AvgVideoBandwidth;
  double MinAbsoluteMOS_V;
  double AvgAbsoluteMOS_V;
  double MaxAbsoluteMOS_V;
  double MinRelativeMOS_V;
  double AvgRelativeMOS_V;
  double MaxRelativeMOS_V;
  uint32_t LossDeg;
  uint32_t PktsRcvd;
  uint32_t PktsLost;
  double UncorrectedLossProp;
  uint32_t BurstCount;
  double BurstLossRate;
  uint32_t BurstLength;
  uint32_t GapCount;
  double GapLossRate;
  uint32_t GapLength;
  double IFrameInterArrivalJitter;
  double PPDV;

  VideoVQualResultsBlockSnapshot() {
    InterAbsoluteMOS_V = 0.0;
    InterRelativeMOS_V = 0.0;
    InterPktsRcv = 0;
    InterPktsLost = 0;
    InterPktsDiscarded = 0;
    InterEffPktsLossRate = 0.0;
    InterPPDV = 0.0;
    AvgVideoBandwidth = 0;
    MinAbsoluteMOS_V = 0.0;
    AvgAbsoluteMOS_V = 0.0;
    MaxAbsoluteMOS_V = 0.0;
    MinRelativeMOS_V = 0.0;
    AvgRelativeMOS_V = 0.0;
    MaxRelativeMOS_V = 0.0;
    LossDeg = 0;
    PktsRcvd = 0;
    PktsLost = 0;
    UncorrectedLossProp = 0.0;
    BurstCount = 0;
    BurstLossRate = 0.0;
    BurstLength = 0;
    GapCount = 0;
    GapLossRate = 0.0;
    GapLength = 0;
    IFrameInterArrivalJitter = 0.0;
    PPDV = 0.0;		
  }

  static double fpq4tog (uint16_t fpq4)
  {
    return (double)fpq4 / 16.0;
  }

  static double fpq8tog (int16_t fpq8)
  {
    return (double)fpq8 / 256.0;
  }	

  static double mostog (uint16_t mos)
  {
    return (double)mos / 256.0;
  }

  static double proptog (uint32_t prop)
  {
    return (double)prop / 4294967296.0;
  }
};

class VQualResultsBlockImpl;

class VQualResultsBlock {
 public:
  VQualResultsBlock();
  virtual ~VQualResultsBlock();
  void addHandler(VoiceVQualResultsRawData* handler);
  void removeHandler(VoiceVQualResultsRawData* handler);
  void addHandler(AudioHDVQualResultsRawData* handler);
  void removeHandler(AudioHDVQualResultsRawData* handler);
  void addHandler(VideoVQualResultsRawData* handler);
  void removeHandler(VideoVQualResultsRawData* handler);
  bool isVoiceHandlersEmpty();
  bool isVoicePktsDirty();
  bool isDirty();
  bool isVoiceDirty();
  bool isAudioHDDirty();
  bool isVideoDirty();
  VoiceVQualResultsBlockSnapshot getVoiceBlockSnapshot();
  VoiceVQualResultsBlockSnapshot getVoiceBlockPktsSnapshot();
  AudioHDVQualResultsBlockSnapshot getAudioHDBlockSnapshot();
  VideoVQualResultsBlockSnapshot getVideoBlockSnapshot();
  void resetData();
  void resetVoiceData();
  void resetAudioHDData();
  void resetVideoData();
 private:
  //Shared implementation part for all referencing objects:
  boost::shared_ptr<VQualResultsBlockImpl> mPimpl;
};

typedef boost::shared_ptr<VQualResultsBlock> VQualResultsBlockHandler;

///////////////////////////////////////////////////////////////////////////

END_DECL_VQSTREAM_NS

#endif //_VQSTATS_H_

