/// @file
/// @brief VQStats implementation
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <math.h>

#include <set>
#include <list>

#include <boost/utility.hpp>

#include "VoIPUtils.h"
#include "VQStats.h"
#include "VQStatsData.h"

////////////////////////////////////////////////////////////////////////

USING_VOIP_NS;

DECL_VQSTREAM_NS

#define DCOPY(member) data.member = other.member

////////////////////////////////////////////////////////////////////////

void VoiceVQualResultsRawData::set(VoiceVQData& other) {

   VoIPGuard(mMutex);
   DCOPY(MOS_LQ);
   DCOPY(MOS_CQ);
   DCOPY(R_LQ);
   DCOPY(R_CQ);
   DCOPY(PPDV);
   DCOPY(PktsRcvd);
   DCOPY(PktsLost);
   DCOPY(PktsDiscarded);

   dirty = true;
}

void AudioHDVQualResultsRawData::set(AudioHDVQData& other)
{
  VoIPGuard(mMutex);

  DCOPY(InterMOS_A);
  DCOPY(AudioInterPktsRcvd);
  DCOPY(InterPktsLost);
  DCOPY(InterPktsDiscarded);
  DCOPY(EffPktsLossRate);
  DCOPY(MinMOS_A);
  DCOPY(AvgMOS_A);
  DCOPY(MaxMOS_A);
  DCOPY(PropBelowTholdMOS_A);
  DCOPY(LossDeg);
  DCOPY(PktsRcvd);
  DCOPY(PktsLoss);
  DCOPY(UncorrectedLossProp);
  DCOPY(BurstCount);
  DCOPY(BurstLossRate);
  DCOPY(BurstLength);
  DCOPY(GapCount);
  DCOPY(GapLossRate);
  DCOPY(GapLength);
  DCOPY(PPDV);
  
  dirty = true;  
}

void VideoVQualResultsRawData::set(VideoVQData& other) 
{
  VoIPGuard(mMutex);

  DCOPY(InterAbsoluteMOS_V);
  DCOPY(InterRelativeMOS_V);
  DCOPY(InterPktsRcv);
  DCOPY(InterPktsLost);
  DCOPY(InterPktsDiscarded);
  DCOPY(InterEffPktsLossRate);
  DCOPY(InterPPDV);
  DCOPY(AvgVideoBandwidth);
  DCOPY(MinAbsoluteMOS_V);
  DCOPY(AvgAbsoluteMOS_V);
  DCOPY(MaxAbsoluteMOS_V);
  DCOPY(MinRelativeMOS_V);
  DCOPY(AvgRelativeMOS_V);
  DCOPY(MaxRelativeMOS_V);
  DCOPY(LossDeg);
  DCOPY(PktsRcvd);
  DCOPY(PktsLost);
  DCOPY(UncorrectedLossProp);
  DCOPY(BurstCount);
  DCOPY(BurstLossRate);
  DCOPY(BurstLength);
  DCOPY(GapCount);
  DCOPY(GapLossRate);
  DCOPY(GapLength);
  DCOPY(IFrameInterArrivalJitter);
  DCOPY(PPDV);

  dirty = true;  
}  

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class VQualResultsBlockImpl : public boost::noncopyable {
  
public:

  typedef std::set<VoiceVQualResultsRawData*>  VoiceContainerType;
  typedef std::list<VoiceVQualResultsRawData*>  SeqVoiceContainerType;

  typedef std::set<AudioHDVQualResultsRawData*>  AudioHDContainerType;
  typedef std::list<AudioHDVQualResultsRawData*>  SeqAudioHDContainerType;

  typedef std::set<VideoVQualResultsRawData*>  VideoContainerType;
  typedef std::list<VideoVQualResultsRawData*>  SeqVideoContainerType;

  VQualResultsBlockImpl() {
    resetData();
  }
  virtual ~VQualResultsBlockImpl() {
    ;
  }
  
  void resetData() {
    resetVoiceData();
    resetAudioHDData();
    resetVideoData();
  }

  void resetVoiceData() {
    memset(&mVoiceData,0,sizeof(mVoiceData));
    mVoiceCtime=VoIPUtils::getMilliSeconds();
    mVoiceLastTimeWhenDirty=VoIPUtils::getMilliSeconds();
    mVoicePktsDirty = false;
  }

  void resetAudioHDData() {
    memset(&mAudioHDData,0,sizeof(mAudioHDData));
    mAudioHDCtime=VoIPUtils::getMilliSeconds();
    mAudioHDLastTimeWhenDirty=VoIPUtils::getMilliSeconds();
  }

  void resetVideoData() {
    memset(&mVideoData,0,sizeof(mVideoData));
    mVideoCtime=VoIPUtils::getMilliSeconds();
    mVideoLastTimeWhenDirty=VoIPUtils::getMilliSeconds();
  }

  void addHandler(VoiceVQualResultsRawData* handler) {
    if(handler) {
      VoIPGuard(mMutex);
      mVoiceHandlers.insert(handler);
    }
  }

  void removeHandler(VoiceVQualResultsRawData* handler) {
    if(handler) {
      VoIPGuard(mMutex);

      mVoiceData.totalElapsPktsRcvd += handler->data.PktsRcvd;
      mVoiceData.totalElapsPktsLost += handler->data.PktsLost;
      mVoiceData.totalElapsPktsDiscarded += handler->data.PktsDiscarded;

      mVoicePktsDirty = true;

      mVoiceHandlers.erase(handler);
    }
  }

  void addHandler(AudioHDVQualResultsRawData* handler) {
    if(handler) {
      VoIPGuard(mMutex);
      mAudioHDHandlers.insert(handler);
    }
  }

  void removeHandler(AudioHDVQualResultsRawData* handler) {
    if(handler) {
      VoIPGuard(mMutex);
      mAudioHDHandlers.erase(handler);
    }
  }

  void addHandler(VideoVQualResultsRawData* handler) {
    if(handler) {
      VoIPGuard(mMutex);
      mVideoHandlers.insert(handler);
    }
  }

  void removeHandler(VideoVQualResultsRawData* handler) {
    if(handler) {
      VoIPGuard(mMutex);
      mVideoHandlers.erase(handler);
    }
  }

  bool isVoiceHandlersEmpty()
  {
     VoIPGuard(mMutex);
     if(mVoiceHandlers.empty()) return true;

     return false;
  }

  bool isVoicePktsDirty()
  {
     VoIPGuard(mMutex);
     if(mVoicePktsDirty) return true;

     return false;
  }

  bool isDirty() {
    return isVoiceDirty()||isAudioHDDirty()||isVideoDirty();
  }

  bool isVoiceDirty() {

    {
      VoIPGuard(mMutex);
      if(mVoiceHandlers.empty()) return false;
    }
    
    if(VoIPUtils::time_after64(VoIPUtils::getMilliSeconds(),
			       mVoiceCtime+VoiceVQualResultsBlockSnapshot::SnapshotSyncTimeout)) return true;
    
    return false;
  }

  bool isAudioHDDirty() {

    {
      VoIPGuard(mMutex);
      if(mAudioHDHandlers.empty()) return false;
    }
    
    if(VoIPUtils::time_after64(VoIPUtils::getMilliSeconds(),
			       mAudioHDCtime+AudioHDVQualResultsBlockSnapshot::SnapshotSyncTimeout)) return true;
    
    return false;
  }

  bool isVideoDirty() {

    {
      VoIPGuard(mMutex);
      if(mVideoHandlers.empty()) return false;
    }

    if(VoIPUtils::time_after64(VoIPUtils::getMilliSeconds(),
			       mVideoCtime+VideoVQualResultsBlockSnapshot::SnapshotSyncTimeout)) return true;

    return false;
  }

  VoiceVQualResultsBlockSnapshot getVoiceBlockSnapshot() {
     double avgCurrMOS_LQ = 0.0;
     uint32_t sizeavgMOS_LQ = 0;
     double minMOS_LQ = mVoiceData.minMOS_LQ;
     double avgMOS_LQ = mVoiceData.avgMOS_LQ;
     double maxMOS_LQ = mVoiceData.maxMOS_LQ;

     double avgCurrMOS_CQ = 0.0;
     uint32_t sizeavgMOS_CQ = 0;
     double minMOS_CQ = mVoiceData.minMOS_CQ;
     double avgMOS_CQ = mVoiceData.avgMOS_CQ;
     double maxMOS_CQ = mVoiceData.maxMOS_CQ;

     uint32_t totalR_LQ = 0;
     uint32_t sizeR_LQ = 0;
     uint8_t minR_LQ = mVoiceData.minR_LQ;
     uint8_t avgR_LQ = mVoiceData.avgR_LQ;
     uint8_t maxR_LQ = mVoiceData.maxR_LQ;

     uint32_t totalR_CQ = 0;
     uint32_t sizeR_CQ = 0;
     uint8_t minR_CQ = mVoiceData.minR_CQ;
     uint8_t avgR_CQ = mVoiceData.avgR_CQ;
     uint8_t maxR_CQ = mVoiceData.maxR_CQ;

    long double totalPPDV = 0;
    uint32_t sizePPDV = 0;
    double maxPPDV = mVoiceData.maxPPDV;

    uint64_t totalPktsRcvd = 0;
    uint64_t totalPktsLost = 0;
    uint64_t totalPktsDiscarded = 0;
    uint32_t sizePktsCounter = 0;

    uint32_t minPktsLost = 0;
    uint32_t maxPktsLost = 0;
    uint32_t minPktsDiscarded = 0;
    uint32_t maxPktsDiscarded = 0;

    SeqVoiceContainerType handlers;
    {
       VoIPGuard(mMutex);
       handlers.insert(handlers.begin(),mVoiceHandlers.begin(),mVoiceHandlers.end());
    }

    for(SeqVoiceContainerType::iterator iter = handlers.begin(); iter != handlers.end(); ++iter) {
       VoiceVQualResultsRawData* handler=*iter;

       VoIPGuard(handler->mMutex);

       if(handler->data.MOS_LQ > 0)
       {
          double MOS_LQ = VoiceVQualResultsBlockSnapshot::mostog(handler->data.MOS_LQ);

          if(minMOS_LQ == 0) minMOS_LQ = MOS_LQ;
          else if(MOS_LQ < minMOS_LQ) minMOS_LQ = MOS_LQ;
          if(MOS_LQ > maxMOS_LQ) maxMOS_LQ = MOS_LQ;

          avgCurrMOS_LQ += MOS_LQ;
          sizeavgMOS_LQ++;
       }

       if(handler->data.MOS_CQ > 0)
       {
          double MOS_CQ = VoiceVQualResultsBlockSnapshot::mostog(handler->data.MOS_CQ);

          if(minMOS_CQ == 0) minMOS_CQ = MOS_CQ;
          else if(MOS_CQ < minMOS_CQ) minMOS_CQ = MOS_CQ;
          if(MOS_CQ > maxMOS_CQ) maxMOS_CQ = MOS_CQ;

          avgCurrMOS_CQ += MOS_CQ;
          sizeavgMOS_CQ++;
       }

       if(handler->data.R_LQ > 0)
       {
          uint8_t R_LQ = handler->data.R_LQ;

          if(minR_LQ == 0) minR_LQ = R_LQ;
          else if(R_LQ < minR_LQ) minR_LQ = R_LQ;
          if(R_LQ > maxR_LQ) maxR_LQ = R_LQ;

          totalR_LQ += R_LQ;
          sizeR_LQ++;
       }

       if(handler->data.R_CQ > 0)
       {
          uint8_t R_CQ = handler->data.R_CQ;

          if(minR_CQ == 0) minR_CQ = R_CQ;
          else if(R_CQ < minR_CQ) minR_CQ = R_CQ;
          if(R_CQ > maxR_CQ) maxR_CQ = R_CQ;

          totalR_CQ += R_CQ;
          sizeR_CQ++;
       }

       if(handler->data.PPDV > 0) 
       {
          double PPDV = VoiceVQualResultsBlockSnapshot::fpq4tog(handler->data.PPDV);

          if(PPDV > maxPPDV) maxPPDV = PPDV;
          
          totalPPDV += PPDV;
          sizePPDV++;
       }

       totalPktsRcvd += handler->data.PktsRcvd; 
       totalPktsLost += handler->data.PktsLost; 
       totalPktsDiscarded += handler->data.PktsDiscarded; 

       sizePktsCounter++;

       if(iter == handlers.begin())
       {
          minPktsLost = handler->data.PktsLost;
          maxPktsLost = handler->data.PktsLost;
       }else
       {
          if(handler->data.PktsLost < minPktsLost) minPktsLost = handler->data.PktsLost;
          if(handler->data.PktsLost > maxPktsLost) maxPktsLost = handler->data.PktsLost;
       }

       if(iter == handlers.begin())
       {
          minPktsDiscarded = handler->data.PktsDiscarded;
          maxPktsDiscarded = handler->data.PktsDiscarded;
       }else
       {
          if(handler->data.PktsDiscarded < minPktsDiscarded) minPktsDiscarded = handler->data.PktsDiscarded;
          if(handler->data.PktsDiscarded > maxPktsDiscarded) maxPktsDiscarded = handler->data.PktsDiscarded;
       }

       handler->dirty = false;
    }

    if(sizeavgMOS_LQ > 0)
       avgMOS_LQ = avgCurrMOS_LQ / sizeavgMOS_LQ;

    avgMOS_LQ = ::round(avgMOS_LQ * 100.00) / 100.00;
    minMOS_LQ = ::round(minMOS_LQ * 100.00) / 100.00;
    maxMOS_LQ = ::round(maxMOS_LQ * 100.00) / 100.00;

    mVoiceData.avgMOS_LQ = avgMOS_LQ;
    mVoiceData.minMOS_LQ = minMOS_LQ;
    mVoiceData.maxMOS_LQ = maxMOS_LQ;

    if(sizeavgMOS_CQ > 0)
       avgMOS_CQ = avgCurrMOS_CQ / sizeavgMOS_CQ;

    avgMOS_CQ = ::round(avgMOS_CQ * 100.00) / 100.00;
    minMOS_CQ = ::round(minMOS_CQ * 100.00) / 100.00;
    maxMOS_CQ = ::round(maxMOS_CQ * 100.00) / 100.00;

    mVoiceData.avgMOS_CQ = avgMOS_CQ;
    mVoiceData.minMOS_CQ = minMOS_CQ;
    mVoiceData.maxMOS_CQ = maxMOS_CQ;

    if(sizeR_LQ > 0)
       avgR_LQ = totalR_LQ / sizeR_LQ;

    mVoiceData.avgR_LQ = avgR_LQ;
    mVoiceData.minR_LQ = minR_LQ;
    mVoiceData.maxR_LQ = maxR_LQ;

    if(sizeR_CQ > 0)
       avgR_CQ = totalR_CQ / sizeR_CQ;

    mVoiceData.avgR_CQ = avgR_CQ;
    mVoiceData.minR_CQ = minR_CQ;
    mVoiceData.maxR_CQ = maxR_CQ;

    if(sizePPDV > 0)
       mVoiceData.avgPPDV = ::round(totalPPDV / sizePPDV * 100.00) / 100.00;

    mVoiceData.maxPPDV = ::round(maxPPDV *100.00) / 100.00;

    mVoiceData.totalPktsRcvd = totalPktsRcvd + mVoiceData.totalElapsPktsRcvd;
    mVoiceData.totalPktsLost = totalPktsLost + mVoiceData.totalElapsPktsLost;
    mVoiceData.totalPktsDiscarded = totalPktsDiscarded + mVoiceData.totalElapsPktsDiscarded;

    mVoicePktsDirty = false;

    if(sizePktsCounter > 0){
        mVoiceData.minPktsLost = minPktsLost;
        mVoiceData.avgPktsLost = totalPktsLost / sizePktsCounter;
        mVoiceData.maxPktsLost = maxPktsLost;
    }

    if(sizePktsCounter > 0){
        mVoiceData.minPktsDiscarded = minPktsDiscarded;
        mVoiceData.avgPktsDiscarded = totalPktsDiscarded / sizePktsCounter;
        mVoiceData.maxPktsDiscarded = maxPktsDiscarded;
    }

    return mVoiceData;
  }
  
  VoiceVQualResultsBlockSnapshot getVoiceBlockPktsSnapshot() {
     
    mVoiceData.totalPktsRcvd = mVoiceData.totalElapsPktsRcvd;
    mVoiceData.totalPktsLost = mVoiceData.totalElapsPktsLost;
    mVoiceData.totalPktsDiscarded = mVoiceData.totalElapsPktsDiscarded;

    mVoicePktsDirty = false;

    return mVoiceData;
  }

  AudioHDVQualResultsBlockSnapshot getAudioHDBlockSnapshot() {
    long double totalInterMOS_A = 0;
    uint32_t sizeInterMOS_A = 0;
    uint32_t totalAudioInterPktsRcvd = 0;
    uint32_t totalInterPktsLost = 0; /* May overflow ? */
    uint32_t totalInterPktsDiscarded = 0;
    long double totalEffPktsLossRate = 0;
    uint32_t sizeEffPktsLossRate = 0;
    double MinMOS_A = 0.0;
    long double totalAvgMOS_A = 0;
    uint32_t sizeAvgMOS_A = 0;
    double MaxMOS_A = 0.0;
    long double totalPropBelowTholdMOS_A = 0;
    uint32_t sizePropBelowTholdMOS_A = 0;
    uint32_t totalLossDeg = 0;
    uint32_t sizeLossDeg = 0;
    uint32_t totalPktsRcvd = 0;
    uint32_t totalPktsLoss = 0;
    long double totalUncorrectedLossProp = 0;
    uint32_t sizeUncorrectedLossProp = 0;
    uint32_t totalBurstCount = 0;
    long double totalBurstLossRate = 0;
    uint32_t sizeBurstLossRate = 0;
    uint32_t totalBurstLength = 0;
    uint32_t sizeBurstLength = 0;
    uint32_t totalGapCount = 0;
    long double totalGapLossRate = 0;
    uint32_t sizeGapLossRate = 0;
    uint32_t totalGapLength = 0;
    uint32_t sizeGapLength = 0;
    long double totalPPDV = 0;
    uint32_t sizePPDV = 0;

    SeqAudioHDContainerType handlers;
    {
      VoIPGuard(mMutex);
      handlers.insert(handlers.begin(), mAudioHDHandlers.begin(), mAudioHDHandlers.end());
    }

    for(SeqAudioHDContainerType::iterator iter = handlers.begin(); iter != handlers.end(); ++iter) {
      AudioHDVQualResultsRawData* handler=*iter;
      VoIPGuard(handler->mMutex);

      if(handler->data.InterMOS_A > 0) {
        totalInterMOS_A += AudioHDVQualResultsBlockSnapshot::mostog(handler->data.InterMOS_A);
        sizeInterMOS_A++;
      }

      totalAudioInterPktsRcvd += handler->data.AudioInterPktsRcvd;
      totalInterPktsLost += handler->data.InterPktsLost;
      totalInterPktsDiscarded += handler->data.InterPktsDiscarded;

      if(handler->data.EffPktsLossRate >= 0) {
        totalEffPktsLossRate += AudioHDVQualResultsBlockSnapshot::proptog(handler->data.EffPktsLossRate);
        sizeEffPktsLossRate++;
      }

      if(handler->data.MinMOS_A > 0) {
        double uaMinMOS_A = AudioHDVQualResultsBlockSnapshot::mostog(handler->data.MinMOS_A);
	if(MinMOS_A == 0.0) MinMOS_A = uaMinMOS_A;
        else if(uaMinMOS_A < MinMOS_A) MinMOS_A = uaMinMOS_A;
      }

      if(handler->data.MaxMOS_A > 0) {
        double uaMaxMOS_A = AudioHDVQualResultsBlockSnapshot::mostog(handler->data.MaxMOS_A);
        if(MaxMOS_A == 0.0) MaxMOS_A = uaMaxMOS_A;
        else if(uaMaxMOS_A > MaxMOS_A) MaxMOS_A = uaMaxMOS_A;
      }

      if(handler->data.AvgMOS_A > 0) {
        totalAvgMOS_A += AudioHDVQualResultsBlockSnapshot::mostog(handler->data.AvgMOS_A);
        sizeAvgMOS_A++;
      }

      if(handler->data.PropBelowTholdMOS_A > 0) {
        totalPropBelowTholdMOS_A += AudioHDVQualResultsBlockSnapshot::proptog(handler->data.PropBelowTholdMOS_A);
        sizePropBelowTholdMOS_A++;
      }

      if(handler->data.LossDeg > 0) {
        totalLossDeg += handler->data.LossDeg;
        sizeLossDeg++;
      }

      totalPktsRcvd += handler->data.PktsRcvd;
      totalPktsLoss += handler->data.PktsLoss;

      if(handler->data.UncorrectedLossProp > 0) {
        totalUncorrectedLossProp += AudioHDVQualResultsBlockSnapshot::proptog(handler->data.UncorrectedLossProp);
        sizeUncorrectedLossProp++;
      }

      totalBurstCount += handler->data.BurstCount;

      if(handler->data.BurstLossRate > 0) {
        totalBurstLossRate += AudioHDVQualResultsBlockSnapshot::proptog(handler->data.BurstLossRate);
        sizeBurstLossRate++;
      }

      if(handler->data.BurstLength > 0) {
        totalBurstLength += handler->data.BurstLength;
        sizeBurstLength++;
      }

      totalGapCount += handler->data.GapCount;

      if(handler->data.GapLossRate > 0) {
        totalGapLossRate += AudioHDVQualResultsBlockSnapshot::proptog(handler->data.GapLossRate);
        sizeGapLossRate++;
      }

      if(handler->data.GapLength > 0) {
        totalGapLength += handler->data.GapLength;
        sizeGapLength++;
      }

      if(handler->data.PPDV > 0) {
        totalPPDV += AudioHDVQualResultsBlockSnapshot::fpq4tog(handler->data.PPDV);
        sizePPDV++;
      }

      handler->dirty = false;
    }

    if(sizeInterMOS_A > 0)
      mAudioHDData.InterMOS_A = ::round(totalInterMOS_A / sizeInterMOS_A * 100.00) / 100.00;

    mAudioHDData.AudioInterPktsRcvd = totalAudioInterPktsRcvd;
    mAudioHDData.InterPktsLost = totalInterPktsLost;
    mAudioHDData.InterPktsDiscarded = totalInterPktsDiscarded;

    if(sizeEffPktsLossRate > 0)
      mAudioHDData.EffPktsLossRate = ::round(totalEffPktsLossRate / sizeEffPktsLossRate * 100.00) / 100.00;

    mAudioHDData.MinMOS_A = ::round(MinMOS_A * 100.00) / 100.00;

    mAudioHDData.MaxMOS_A = ::round(MaxMOS_A * 100.00) / 100.00;
    
    if(sizeAvgMOS_A > 0)
      mAudioHDData.AvgMOS_A = ::round(totalAvgMOS_A / sizeAvgMOS_A * 100.00) / 100.00;

    if(sizePropBelowTholdMOS_A > 0)
      mAudioHDData.PropBelowTholdMOS_A = ::round(totalPropBelowTholdMOS_A / sizePropBelowTholdMOS_A * 100.00) / 100.00;

    if(sizeLossDeg > 0)
      mAudioHDData.LossDeg = totalLossDeg / sizeLossDeg;

    mAudioHDData.PktsRcvd = totalPktsRcvd;
    mAudioHDData.PktsLoss = totalPktsLoss;

    if(sizeUncorrectedLossProp > 0)
      mAudioHDData.UncorrectedLossProp = ::round(totalUncorrectedLossProp / sizeUncorrectedLossProp * 100.00) / 100.00;

    mAudioHDData.BurstCount = totalBurstCount;

    if(sizeBurstLossRate > 0)
      mAudioHDData.BurstLossRate = ::round(totalBurstLossRate / sizeBurstLossRate * 100.00) / 100.00;

    if(sizeBurstLength > 0)
      mAudioHDData.BurstLength = totalBurstLength / sizeBurstLength;

    mAudioHDData.GapCount = totalGapCount;

    if(sizeGapLossRate > 0)
      mAudioHDData.GapLossRate = ::round(totalGapLossRate / sizeGapLossRate * 100.00) / 100.00;

    if(sizeGapLength > 0)
      mAudioHDData.GapLength = totalGapLength / sizeGapLength;

    if(sizePPDV > 0)
      mAudioHDData.PPDV = ::round(totalPPDV / sizePPDV * 100.00) / 100.00;

    return mAudioHDData;    
  }

  VideoVQualResultsBlockSnapshot getVideoBlockSnapshot() {
    long double totalInterAbsoluteMOS_V = 0.0;
    uint32_t sizeInterAbsoluteMOS_V = 0;
    long double totalInterRelativeMOS_V = 0.0;
    uint32_t sizeInterRelativeMOS_V = 0;
    uint32_t totalInterPktsRcv = 0;
    uint32_t totalInterPktsLost = 0;
    uint32_t totalInterPktsDiscarded = 0;
    long double totalInterEffPktsLossRate = 0.0;
    uint32_t sizeInterEffPktsLossRate = 0;
    long double totalInterPPDV = 0.0;
    uint32_t sizeInterPPDV = 0;
    uint32_t totalAvgVideoBandwidth = 0;
    uint32_t sizeAvgVideoBandwidth = 0;
    double MinAbsoluteMOS_V = 0.0;
    long double totalAvgAbsoluteMOS_V = 0.0;
    uint32_t sizeAvgAbsoluteMOS_V = 0;
    double MaxAbsoluteMOS_V = 0.0;
    double MinRelativeMOS_V = 0.0;
    long double totalAvgRelativeMOS_V = 0.0;
    uint32_t sizeAvgRelativeMOS_V = 0;
    double MaxRelativeMOS_V = 0.0;
    uint32_t totalLossDeg = 0;
    uint32_t sizeLossDeg = 0;
    uint32_t totalPktsRcvd = 0;
    uint32_t sizePktsRcvd = 0;
    uint32_t totalPktsLost = 0;
    uint32_t sizePktsLost = 0;
    long double totalUncorrectedLossProp = 0;
    uint32_t sizeUncorrectedLossProp = 0;
    uint32_t totalBurstCount = 0;
    long double totalBurstLossRate = 0;
    uint32_t sizeBurstLossRate = 0;
    uint32_t totalBurstLength = 0;
    uint32_t sizeBurstLength = 0;
    uint32_t totalGapCount = 0;
    long double totalGapLossRate = 0;
    uint32_t sizeGapLossRate = 0;
    uint32_t totalGapLength = 0;
    uint32_t sizeGapLength = 0;
    long double totalIFrameInterArrivalJitter = 0.0;
    uint32_t sizeIFrameInterArrivalJitter = 0;   
    long double totalPPDV = 0;
    uint32_t sizePPDV = 0;       

    SeqVideoContainerType handlers;
    {
      VoIPGuard(mMutex);
      handlers.insert(handlers.begin(), mVideoHandlers.begin(), mVideoHandlers.end());
    }

    for(SeqVideoContainerType::iterator iter = handlers.begin(); iter != handlers.end(); ++iter) {
      VideoVQualResultsRawData* handler=*iter;
      VoIPGuard(handler->mMutex);

      if(handler->data.InterAbsoluteMOS_V > 0) {
        totalInterAbsoluteMOS_V += VideoVQualResultsBlockSnapshot::mostog(handler->data.InterAbsoluteMOS_V);
        sizeInterAbsoluteMOS_V++;
      }

      if(handler->data.InterRelativeMOS_V > 0) {
        totalInterRelativeMOS_V += VideoVQualResultsBlockSnapshot::mostog(handler->data.InterRelativeMOS_V);
        sizeInterRelativeMOS_V++;
      }
      
      totalInterPktsRcv += handler->data.InterPktsRcv;
      totalInterPktsLost += handler->data.InterPktsLost;
      totalInterPktsDiscarded += handler->data.InterPktsDiscarded;

      if(handler->data.InterEffPktsLossRate >= 0) {
        totalInterEffPktsLossRate += VideoVQualResultsBlockSnapshot::proptog(handler->data.InterEffPktsLossRate);
        sizeInterEffPktsLossRate++;
      }
      
      if(handler->data.InterPPDV > 0) {
        totalInterPPDV += VideoVQualResultsBlockSnapshot::fpq4tog(handler->data.InterPPDV);
        sizeInterPPDV++;
      }

      if(handler->data.AvgVideoBandwidth > 0) {
        totalAvgVideoBandwidth += handler->data.AvgVideoBandwidth;
        sizeAvgVideoBandwidth++;        
      }

      if(handler->data.MinAbsoluteMOS_V > 0) {
        double uaMinAbsoluteMOS_V = VideoVQualResultsBlockSnapshot::mostog(handler->data.MinAbsoluteMOS_V);
        if(MinAbsoluteMOS_V == 0.0) MinAbsoluteMOS_V = uaMinAbsoluteMOS_V;
        else if(uaMinAbsoluteMOS_V < MinAbsoluteMOS_V) MinAbsoluteMOS_V = uaMinAbsoluteMOS_V;
      }
      
      if(handler->data.AvgAbsoluteMOS_V > 0) {
        totalAvgAbsoluteMOS_V += VideoVQualResultsBlockSnapshot::mostog(handler->data.AvgAbsoluteMOS_V);
        sizeAvgAbsoluteMOS_V++;
      }

      if(handler->data.MaxAbsoluteMOS_V > 0) {
        double uaMaxAbsoluteMOS_V = VideoVQualResultsBlockSnapshot::mostog(handler->data.MaxAbsoluteMOS_V);
        if(MaxAbsoluteMOS_V == 0.0) MaxAbsoluteMOS_V = uaMaxAbsoluteMOS_V;
        else if(uaMaxAbsoluteMOS_V > MaxAbsoluteMOS_V) MaxAbsoluteMOS_V = uaMaxAbsoluteMOS_V;
      }

      if(handler->data.MinRelativeMOS_V > 0) {
        double uaMinRelativeMOS_V = VideoVQualResultsBlockSnapshot::mostog(handler->data.MinRelativeMOS_V);
        if(MinRelativeMOS_V == 0.0) MinRelativeMOS_V = uaMinRelativeMOS_V;
        else if(uaMinRelativeMOS_V < MinRelativeMOS_V) MinRelativeMOS_V= uaMinRelativeMOS_V;
      }
      
      if(handler->data.AvgRelativeMOS_V > 0) {
        totalAvgRelativeMOS_V += VideoVQualResultsBlockSnapshot::mostog(handler->data.AvgRelativeMOS_V);
        sizeAvgRelativeMOS_V++;
      }

      if(handler->data.MaxRelativeMOS_V > 0) {
        double uaMaxRelativeMOS_V = VideoVQualResultsBlockSnapshot::mostog(handler->data.MaxRelativeMOS_V);
        if(MaxRelativeMOS_V == 0.0) MaxRelativeMOS_V = uaMaxRelativeMOS_V;
        else if(uaMaxRelativeMOS_V > MaxRelativeMOS_V) MaxRelativeMOS_V= uaMaxRelativeMOS_V;
      }

      if(handler->data.LossDeg > 0) {
        totalLossDeg += handler->data.LossDeg;
        sizeLossDeg++;       
      }

      if(handler->data.PktsRcvd > 0) {
        totalPktsRcvd += handler->data.PktsRcvd;
        sizePktsRcvd++;        
      }

      if(handler->data.PktsLost > 0) {
        totalPktsLost += handler->data.PktsLost;
        sizePktsLost++;        
      }
      
      if(handler->data.UncorrectedLossProp > 0) {
        totalUncorrectedLossProp += VideoVQualResultsBlockSnapshot::proptog(handler->data.UncorrectedLossProp);
        sizeUncorrectedLossProp++;              
      }
      
      totalBurstCount += handler->data.BurstCount;

      if(handler->data.BurstLossRate > 0) {
        totalBurstLossRate += VideoVQualResultsBlockSnapshot::proptog(handler->data.BurstLossRate);
        sizeBurstLossRate++;       
      }

      if(handler->data.BurstLength > 0) {
        totalBurstLength += handler->data.BurstLength;
        sizeBurstLength++;        
      }
      
      totalGapCount += handler->data.GapCount;

      if(handler->data.GapLossRate > 0) {
        totalGapLossRate += VideoVQualResultsBlockSnapshot::proptog(handler->data.GapLossRate);
        sizeGapLossRate++;
      }

      if(handler->data.GapLength > 0) {
        totalGapLength += handler->data.GapLength;
        sizeGapLength++;
      }

      if(handler->data.IFrameInterArrivalJitter > 0) {
        totalIFrameInterArrivalJitter += VideoVQualResultsBlockSnapshot::fpq8tog(handler->data.IFrameInterArrivalJitter);
        sizeIFrameInterArrivalJitter++;        
      }      

      if(handler->data.PPDV > 0) {
        totalPPDV += VideoVQualResultsBlockSnapshot::fpq4tog(handler->data.PPDV);
        sizePPDV++;
      }      

      handler->dirty = false;
    }

    if(sizeInterAbsoluteMOS_V > 0)
      mVideoData.InterAbsoluteMOS_V = ::round(totalInterAbsoluteMOS_V / sizeInterAbsoluteMOS_V * 100.00) / 100.00;

    if(sizeInterRelativeMOS_V > 0)
      mVideoData.InterRelativeMOS_V = ::round(totalInterRelativeMOS_V / sizeInterRelativeMOS_V * 100.00) / 100.00;

    mVideoData.InterPktsRcv = totalInterPktsRcv;
    mVideoData.InterPktsLost = totalInterPktsLost;
    mVideoData.InterPktsDiscarded = totalInterPktsDiscarded;

    if(sizeInterEffPktsLossRate > 0)
      mVideoData.InterEffPktsLossRate = ::round(totalInterEffPktsLossRate / sizeInterEffPktsLossRate * 100.00) / 100.00;

    if(sizeInterPPDV > 0)
      mVideoData.InterPPDV = ::round(totalInterPPDV / sizeInterPPDV * 100.00) / 100.00;

    if(sizeAvgVideoBandwidth > 0)
      mVideoData.AvgVideoBandwidth = totalAvgVideoBandwidth / sizeAvgVideoBandwidth;

    mVideoData.MinAbsoluteMOS_V = ::round(MinAbsoluteMOS_V * 100.00) / 100.00;

    if(sizeAvgAbsoluteMOS_V > 0)
      mVideoData.AvgAbsoluteMOS_V = ::round(totalAvgAbsoluteMOS_V / sizeAvgAbsoluteMOS_V * 100.00) / 100.00;

    mVideoData.MaxAbsoluteMOS_V = ::round(MaxAbsoluteMOS_V * 100.00) / 100.00;
    
    mVideoData.MinRelativeMOS_V = ::round(MinRelativeMOS_V * 100.00) / 100.00;

    if(sizeAvgRelativeMOS_V > 0)
      mVideoData.AvgRelativeMOS_V = ::round(totalAvgRelativeMOS_V / sizeAvgRelativeMOS_V * 100.00) / 100.00;

    mVideoData.MaxRelativeMOS_V = ::round(MaxRelativeMOS_V * 100.00) / 100.00;
    
    if(sizeLossDeg > 0)
      mVideoData.LossDeg = totalLossDeg / sizeLossDeg;

    mVideoData.PktsRcvd = totalPktsRcvd;
    mVideoData.PktsLost = totalPktsLost;

    if(sizeUncorrectedLossProp > 0)
      mVideoData.UncorrectedLossProp = ::round(totalUncorrectedLossProp / sizeUncorrectedLossProp * 100.00) / 100.00;

    mVideoData.BurstCount = totalBurstCount;

    if(sizeBurstLossRate > 0)
      mVideoData.BurstLossRate = ::round(totalBurstLossRate / sizeBurstLossRate * 100.00) / 100.00;

    if(sizeBurstLength > 0)
      mVideoData.BurstLength = totalBurstLength / sizeBurstLength;

    mVideoData.GapCount = totalGapCount;

    if(sizeGapLossRate > 0)
      mVideoData.GapLossRate = ::round(totalGapLossRate / sizeGapLossRate * 100.00) / 100.00;

    if(sizeGapLength > 0)
      mVideoData.GapLength = totalGapLength / sizeGapLength;

    if(sizeIFrameInterArrivalJitter > 0)
      mVideoData.IFrameInterArrivalJitter = ::round(totalIFrameInterArrivalJitter / sizeIFrameInterArrivalJitter * 100.00) / 100.00;

    if(sizePPDV > 0)
      mVideoData.PPDV = ::round(totalPPDV / sizePPDV * 100.00) / 100.00;

    return mVideoData;
  }
  
private:
  VoIPUtils::MutexType mMutex;
  uint64_t mVoiceCtime;
  uint64_t mAudioHDCtime;
  uint64_t mVideoCtime;
  bool mVoicePktsDirty;
  VoiceContainerType mVoiceHandlers;
  AudioHDContainerType mAudioHDHandlers;
  VideoContainerType mVideoHandlers;
  VoiceVQualResultsBlockSnapshot mVoiceData;
  AudioHDVQualResultsBlockSnapshot mAudioHDData;
  VideoVQualResultsBlockSnapshot mVideoData;
  uint64_t mVoiceLastTimeWhenDirty;
  uint64_t mAudioHDLastTimeWhenDirty;
  uint64_t mVideoLastTimeWhenDirty;
};

VQualResultsBlock::VQualResultsBlock() {
  mPimpl.reset(new VQualResultsBlockImpl());
}
VQualResultsBlock::~VQualResultsBlock() {
  ;
}
void VQualResultsBlock::addHandler(VoiceVQualResultsRawData* handler) {
  mPimpl->addHandler(handler);
}
void VQualResultsBlock::removeHandler(VoiceVQualResultsRawData* handler) {
  mPimpl->removeHandler(handler);
}
void VQualResultsBlock::addHandler(AudioHDVQualResultsRawData* handler) {
  mPimpl->addHandler(handler);
}
void VQualResultsBlock::removeHandler(AudioHDVQualResultsRawData* handler) {
  mPimpl->removeHandler(handler);
}
void VQualResultsBlock::addHandler(VideoVQualResultsRawData* handler) {
  mPimpl->addHandler(handler);
}
void VQualResultsBlock::removeHandler(VideoVQualResultsRawData* handler) {
  mPimpl->removeHandler(handler);
}
bool VQualResultsBlock::isVoiceHandlersEmpty() {
  return mPimpl->isVoiceHandlersEmpty();
}
bool VQualResultsBlock::isVoicePktsDirty() {
  return mPimpl->isVoicePktsDirty();
}
bool VQualResultsBlock::isDirty() {
  return mPimpl->isDirty();
}
bool VQualResultsBlock::isVoiceDirty() {
  return mPimpl->isVoiceDirty();
}
bool VQualResultsBlock::isAudioHDDirty() {
  return mPimpl->isAudioHDDirty();
}
bool VQualResultsBlock::isVideoDirty() {
  return mPimpl->isVideoDirty();
}
VoiceVQualResultsBlockSnapshot VQualResultsBlock::getVoiceBlockSnapshot() {
  return mPimpl->getVoiceBlockSnapshot();
}
VoiceVQualResultsBlockSnapshot VQualResultsBlock::getVoiceBlockPktsSnapshot() {
  return mPimpl->getVoiceBlockPktsSnapshot();
}
AudioHDVQualResultsBlockSnapshot VQualResultsBlock::getAudioHDBlockSnapshot() {
  return mPimpl->getAudioHDBlockSnapshot();
}
VideoVQualResultsBlockSnapshot VQualResultsBlock::getVideoBlockSnapshot() {
  return mPimpl->getVideoBlockSnapshot();
}
void VQualResultsBlock::resetData() {
  return mPimpl->resetData();
}
void VQualResultsBlock::resetVoiceData() {
  return mPimpl->resetVoiceData();
}
void VQualResultsBlock::resetAudioHDData() {
  return mPimpl->resetAudioHDData();
}
void VQualResultsBlock::resetVideoData() {
  return mPimpl->resetVideoData();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

END_DECL_VQSTREAM_NS



