/// @file
/// @brief SIP User Agent Raw Statistics header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _USER_AGENT_RAW_STATS_H_
#define _USER_AGENT_RAW_STATS_H_

#include "VoIPCommon.h"
#include "VQStats.h"

DECL_APP_NS

///////////////////////////////////////////////////////////////////////////////

class UserAgentRawStats
{
public:
#ifdef UNIT_TEST
    UserAgentRawStats(void)
        : mHandle(static_cast<uint32_t>(-1))
    {
      vqResultsBlock.reset(new VQSTREAM_NS::VQualResultsBlock());
      ResetInternal();
    }
#endif

    UserAgentRawStats(uint32_t handle)
        : intendedRegLoad(0),
          intendedCallLoad(0),
          mHandle(handle)
    {
      vqResultsBlock.reset(new VQSTREAM_NS::VQualResultsBlock());
      ResetInternal();
    }

    /// @brief Block handle accessor
    const uint32_t Handle(void) const { return mHandle; }
    
    /// @brief Reset stats
    void Reset(void)
    {
        ResetInternal();
    }

    /// @brief Synchronize stats
    /// @note Recalculates averages
    void Sync(void)
    {
      regAvgTimeMsec = regSuccesses ? static_cast<uint64_t>((static_cast<double>(regCummTimeMsec) / regSuccesses)) : 0;
      callAvgTimeMsec = callSuccesses ? static_cast<uint64_t>((static_cast<double>(callCummTimeMsec) / callSuccesses)) : 0;
      avgResponseDelay = callSuccesses ? static_cast<uint64_t>((static_cast<double>(cummResponseDelay) / callSuccesses)) : 0;
      avgPostDialDelay = callSuccesses ? static_cast<uint64_t>((static_cast<double>(cummPostDialDelay) / callSuccesses)) : 0;
      avgSessionAckDelay = callAnswers ? static_cast<uint64_t>((static_cast<double>(cummSessionAckDelay) / callAnswers)) : 0;
      avgFailedSessionSetupDelay = callFailures ? static_cast<uint64_t>((static_cast<double>(cummFailedSessionSetupDelay) / callFailures)) : 0;

      size_t byeSessionsInitiated = byeRequestsSent + byeMessagesRetransmitted;
      avgDisconnectResponseDelay = byeSessionsInitiated ? static_cast<uint64_t>((static_cast<double>(cummDisconnectResponseDelay) / byeSessionsInitiated)) : 0;
      avgFailedRegistrationDelay = regFailures ? static_cast<uint64_t>((static_cast<double>(cummFailedRegistrationDelay) / regFailures)) : 0;
      avgHopsPerReq = inviteRequestsReceived ? static_cast<uint64_t>((static_cast<double>(cummHopsPerReq) / inviteRequestsReceived)) : 0;

      if(vqResultsBlock->isVoiceDirty()) {
        VQSTREAM_NS::VoiceVQualResultsBlockSnapshot VqmonStat = vqResultsBlock->getVoiceBlockSnapshot();
        MosLqAvgScore = VqmonStat.avgMOS_LQ;
        MosLqMinScore = VqmonStat.minMOS_LQ;
        MosLqMaxScore = VqmonStat.maxMOS_LQ;

        AverageMosCQ = VqmonStat.avgMOS_CQ;
        MinMosCQ = VqmonStat.minMOS_CQ;
        MaxMosCQ = VqmonStat.maxMOS_CQ;

        MinRFactorLQ = VqmonStat.minR_LQ;
        MaxRFactorLQ = VqmonStat.maxR_LQ;
        AverageRFactorLQ = VqmonStat.avgR_LQ;
        
        MinRFactorCQ = VqmonStat.minR_CQ;
        MaxRFactorCQ = VqmonStat.maxR_CQ;
        AverageRFactorCQ = VqmonStat.avgR_CQ;       

        AverageVoIPPpdv = VqmonStat.avgPPDV;
        MaxVoIPPpdv = VqmonStat.maxPPDV;

        RtpPacketsRxCount = VqmonStat.totalPktsRcvd;
        RtpPacketsLostCount = VqmonStat.totalPktsLost;
        RtpPacketsDiscardCount = VqmonStat.totalPktsDiscarded;
        
        MinRtpPacketsLost = VqmonStat.minPktsLost;
        MaxRtpPacketsLost = VqmonStat.maxPktsLost;
        AverageRtpPacketsLost = VqmonStat.avgPktsLost;
        
        MinRtpPacketsDiscard = VqmonStat.minPktsDiscarded;
        MaxRtpPacketsDiscard = VqmonStat.maxPktsDiscarded;
        AverageRtpPacketsDiscard = VqmonStat.avgPktsDiscarded;       
      }else if(vqResultsBlock->isVoiceHandlersEmpty()&& vqResultsBlock->isVoicePktsDirty())
      {
        VQSTREAM_NS::VoiceVQualResultsBlockSnapshot VqmonStat = vqResultsBlock->getVoiceBlockPktsSnapshot();

        RtpPacketsRxCount = VqmonStat.totalPktsRcvd;
        RtpPacketsLostCount = VqmonStat.totalPktsLost;
        RtpPacketsDiscardCount = VqmonStat.totalPktsDiscarded;
      }
 
      if(vqResultsBlock->isAudioHDDirty()) {
        VQSTREAM_NS::AudioHDVQualResultsBlockSnapshot VqmonStat = vqResultsBlock->getAudioHDBlockSnapshot();
        IntervalAbsMosAScore = VqmonStat.InterMOS_A;
        AudioIntervalTransportPktRcvd = VqmonStat.AudioInterPktsRcvd;
        AudioIntervalTransportPktLost = VqmonStat.InterPktsLost;
        AudioIntervalTransportPktEffLoss = VqmonStat.EffPktsLossRate;
        AudioIntervalTransportPktDiscarded = VqmonStat.InterPktsDiscarded;
        MinAbsMosAScore = VqmonStat.MinMOS_A;
        AvgAbsMosAScore = VqmonStat.AvgMOS_A;
        MaxAbsMosAScore = VqmonStat.MaxMOS_A;
        AbsMosABelowThresholdProp = VqmonStat.PropBelowTholdMOS_A;
        AudioPktLossDegradSev = VqmonStat.LossDeg;
        AudioTransportPkt = VqmonStat.PktsRcvd;
        AudioTransportPktLost = VqmonStat.PktsLoss;
        AudioUncorrLossProp = VqmonStat.UncorrectedLossProp;
        AudioBurst = VqmonStat.BurstCount;
        AudioBurstLoss = VqmonStat.BurstLossRate;
        AudioBurstLength = VqmonStat.BurstLength;
        AudioGap = VqmonStat.GapCount;
        AudioGapLossRate = VqmonStat.GapLossRate;
        AudioGapLength = VqmonStat.GapLength;
        AudioPpdv = VqmonStat.PPDV;
      }

      if(vqResultsBlock->isVideoDirty()) {
        VQSTREAM_NS::VideoVQualResultsBlockSnapshot VqmonStat = vqResultsBlock->getVideoBlockSnapshot();
        IntervalAbsMosVScore = VqmonStat.InterAbsoluteMOS_V;
        IntervalRelMosVScore = VqmonStat.InterRelativeMOS_V;
        IntervalTransportPkt = VqmonStat.InterPktsRcv;
        VideoIntervalTransportPktLost = VqmonStat.InterPktsLost;
        VideoIntervalTransportPktDiscarded = VqmonStat.InterPktsDiscarded;        
        VideoIntervalTransportPktEffLossRate = VqmonStat.InterEffPktsLossRate;
        VideoIntervalAvgPpdv = VqmonStat.InterPPDV;
        AvgStreamVideoBw = VqmonStat.AvgVideoBandwidth;
        MinAbsMosVScore = VqmonStat.MinAbsoluteMOS_V;
        AvgAbsMosVScore = VqmonStat.AvgAbsoluteMOS_V;
        MaxAbsMosVScore = VqmonStat.MaxAbsoluteMOS_V;
        MinRelMosVScore = VqmonStat.MinRelativeMOS_V;
        AvgRelMosVScore = VqmonStat.AvgRelativeMOS_V;
        MaxRelMosVScore = VqmonStat.MaxRelativeMOS_V;
        VideoPktLossDegradSevScore = VqmonStat.LossDeg;
        VideoTransportPktCountScore = VqmonStat.PktsRcvd;
        VideoTransportPktLost = VqmonStat.PktsLost;
        VideoUncorrLossProp = VqmonStat.UncorrectedLossProp;
        VideoBurst = VqmonStat.BurstCount;
        VideoBurstLength = VqmonStat.BurstLength;
        VideoBurstLossRate = VqmonStat.BurstLossRate;
        VideoGap = VqmonStat.GapCount;
        VideoGapLossRate = VqmonStat.GapLossRate;
        VideoGapLength = VqmonStat.GapLength;
        IframeInterarrivalJitter = VqmonStat.IFrameInterArrivalJitter;
        VideoPpdv = VqmonStat.PPDV;       
      }
    }

    bool isDirty() {
      return dirty || vqResultsBlock->isDirty();
    }

    void setDirty(bool value) {
      dirty=value;
    }

    /// @note Public data members

    uint32_t intendedRegLoad;
    uint64_t regAttempts;
    uint64_t regSuccesses;
    uint64_t regRetries;
    uint64_t regFailures;
    uint64_t regMinTimeMsec;
    uint64_t regMaxTimeMsec;
    uint64_t regCummTimeMsec;
    uint64_t regAvgTimeMsec;

    uint64_t minResponseDelay;
    uint64_t avgResponseDelay;
    uint64_t maxResponseDelay;
    uint64_t cummResponseDelay;
    uint64_t minPostDialDelay;
    uint64_t avgPostDialDelay;
    uint64_t maxPostDialDelay;
    uint64_t cummPostDialDelay;
    uint64_t minSessionAckDelay;
    uint64_t avgSessionAckDelay;
    uint64_t maxSessionAckDelay;
    uint64_t cummSessionAckDelay;
    uint64_t minFailedSessionSetupDelay;
    uint64_t avgFailedSessionSetupDelay;
    uint64_t maxFailedSessionSetupDelay;
    uint64_t cummFailedSessionSetupDelay;
    uint64_t minDisconnectResponseDelay;
    uint64_t avgDisconnectResponseDelay;
    uint64_t maxDisconnectResponseDelay;
    uint64_t cummDisconnectResponseDelay;
    uint64_t minFailedRegistrationDelay;
    uint64_t avgFailedRegistrationDelay;
    uint64_t maxFailedRegistrationDelay;
    uint64_t cummFailedRegistrationDelay;

    uint64_t minSuccessfulSessionDuration;
    uint64_t avgSuccessfulSessionDuration;
    uint64_t maxSuccessfulSessionDuration;
    uint64_t cummSuccessfulSessionDuration;

    uint32_t intendedCallLoad;
    uint64_t callAttempts;
    uint64_t callSuccesses;
    uint64_t callFailures;
    uint64_t callAnswers;
    uint64_t callMinTimeMsec;
    uint64_t callMaxTimeMsec;
    uint64_t callCummTimeMsec;
    uint64_t callAvgTimeMsec;
    uint64_t callsActive;
    uint64_t callsFailedTimerB;
    uint64_t callsFailedTransportError;
    uint64_t callsFailed5xx;
    uint64_t failedSessionDisconnectCount;

    uint64_t inviteRequestsSent;
    uint64_t inviteRequestsReceived;
    uint64_t inviteMessagesRetransmitted;

    uint64_t ackRequestsSent;
    uint64_t ackRequestsReceived;

    uint64_t byeRequestsSent;
    uint64_t byeRequestsReceived;
    uint64_t byeMessagesRetransmitted;

    uint64_t txResponseCode1XXCount;
    uint64_t txResponseCode2XXCount;
    uint64_t txResponseCode3XXCount;
    uint64_t txResponseCode4XXCount;
    uint64_t txResponseCode5XXCount;
    uint64_t txResponseCode6XXCount;
    uint64_t rxResponseCode1XXCount;
    uint64_t rxResponseCode2XXCount;
    uint64_t rxResponseCode3XXCount;
    uint64_t rxResponseCode4XXCount;
    uint64_t rxResponseCode5XXCount;
    uint64_t rxResponseCode6XXCount;
    
    uint64_t minHopsPerReq;
    uint64_t avgHopsPerReq;
    uint64_t maxHopsPerReq;
    uint64_t cummHopsPerReq;

    uint64_t nonInviteSessionsInitiated;
    uint64_t nonInviteSessionsSucceeded;
    uint64_t nonInviteSessionsFailed;
    uint64_t nonInviteSessionsFailedTimerF;
    uint64_t nonInviteSessionsFailedTransport;

    double MosLqMinScore;
    double MosLqMaxScore;
    double MosLqAvgScore;

    double MinMosCQ;
    double MaxMosCQ;
    double AverageMosCQ;

    uint8_t MinRFactorLQ;
    uint8_t MaxRFactorLQ;
    uint8_t AverageRFactorLQ;

    uint8_t MinRFactorCQ;
    uint8_t MaxRFactorCQ;
    uint8_t AverageRFactorCQ;

    double AverageVoIPPpdv;
    double MaxVoIPPpdv;

    uint64_t RtpPacketsRxCount;
    uint64_t RtpPacketsLostCount;
    uint64_t RtpPacketsDiscardCount;

    uint64_t MinRtpPacketsLost;
    uint64_t MaxRtpPacketsLost;
    uint64_t AverageRtpPacketsLost;

    uint64_t MinRtpPacketsDiscard;
    uint64_t MaxRtpPacketsDiscard;
    uint64_t AverageRtpPacketsDiscard;

    double IntervalAbsMosAScore;
    uint32_t AudioIntervalTransportPktRcvd;
    uint32_t AudioIntervalTransportPktLost;
    double AudioIntervalTransportPktEffLoss;
    uint32_t AudioIntervalTransportPktDiscarded;
    double MinAbsMosAScore;
    double AvgAbsMosAScore;
    double MaxAbsMosAScore;
    double AbsMosABelowThresholdProp;
    uint8_t AudioPktLossDegradSev;
    uint32_t AudioTransportPkt;
    uint32_t AudioTransportPktLost;
    double AudioUncorrLossProp;
    uint32_t AudioBurst;
    double AudioBurstLoss;
    uint32_t AudioBurstLength;
    uint32_t AudioGap;
    double AudioGapLossRate;
    uint32_t AudioGapLength;
    double AudioPpdv;

    double IntervalAbsMosVScore;
    double IntervalRelMosVScore;
    uint32_t IntervalTransportPkt;
    uint32_t VideoIntervalTransportPktLost;
    double VideoIntervalTransportPktEffLossRate;
    uint32_t VideoIntervalTransportPktDiscarded;
    double VideoIntervalAvgPpdv;
    uint32_t AvgStreamVideoBw;
    double MinAbsMosVScore;
    double MaxAbsMosVScore;
    double AvgAbsMosVScore;
    double MinRelMosVScore;
    double MaxRelMosVScore;
    double AvgRelMosVScore;
    uint8_t VideoPktLossDegradSevScore;
    uint32_t VideoTransportPktCountScore;
    uint32_t VideoTransportPktLost;
    double VideoUncorrLossProp;
    uint32_t VideoBurst;
    uint32_t VideoBurstLength;
    double VideoBurstLossRate;
    uint32_t VideoGap;
    double VideoGapLossRate;
    uint32_t VideoGapLength;
    double IframeInterarrivalJitter;
    double VideoPpdv;
    
    uint64_t sessionRefreshes;

    VQSTREAM_NS::VQualResultsBlockHandler vqResultsBlock;

private:

    bool dirty;

    void ResetInternal(void)
    {
        dirty = false;
        
        regAttempts = 0;
        regSuccesses = 0;
        regRetries = 0;
        regFailures = 0;
        regMinTimeMsec = 0;
        regMaxTimeMsec = 0;
        regCummTimeMsec = 0;
        regAvgTimeMsec = 0;

        callAttempts = 0;
        callSuccesses = 0;
        callFailures = 0;
        callAnswers = 0;
        callMinTimeMsec = 0;
        callMaxTimeMsec = 0;
        callCummTimeMsec = 0;
        callAvgTimeMsec = 0;
        callsActive = 0;
        callsFailedTimerB = 0;
        callsFailedTransportError = 0;
        callsFailed5xx = 0;
        failedSessionDisconnectCount = 0;

        minResponseDelay = 0;
        avgResponseDelay = 0;
        maxResponseDelay = 0;
        cummResponseDelay = 0;
        minPostDialDelay = 0;
        avgPostDialDelay = 0;
        maxPostDialDelay = 0;
        cummPostDialDelay = 0;
        minSessionAckDelay = 0;
        avgSessionAckDelay = 0;
        maxSessionAckDelay = 0;
        cummSessionAckDelay = 0;
        minFailedSessionSetupDelay = 0;
        avgFailedSessionSetupDelay = 0;
        maxFailedSessionSetupDelay = 0;
        cummFailedSessionSetupDelay = 0;
        minDisconnectResponseDelay = 0;
        avgDisconnectResponseDelay = 0;
        maxDisconnectResponseDelay = 0;
        cummDisconnectResponseDelay = 0;
        minFailedRegistrationDelay = 0;
        avgFailedRegistrationDelay = 0;
        maxFailedRegistrationDelay = 0;
        cummFailedRegistrationDelay = 0;

        minSuccessfulSessionDuration = 0;
        avgSuccessfulSessionDuration = 0;
        maxSuccessfulSessionDuration = 0;
        cummSuccessfulSessionDuration = 0;

        inviteRequestsSent = 0;
        inviteRequestsReceived = 0;
        inviteMessagesRetransmitted = 0;

        ackRequestsSent = 0;
        ackRequestsReceived = 0;

        byeRequestsSent = 0;
        byeRequestsReceived = 0;
        byeMessagesRetransmitted = 0;

        txResponseCode1XXCount = 0;
        txResponseCode2XXCount = 0;
        txResponseCode3XXCount = 0;        
        txResponseCode4XXCount = 0;
        txResponseCode5XXCount = 0;
        txResponseCode6XXCount = 0;
        rxResponseCode1XXCount = 0;
        rxResponseCode2XXCount = 0;
        rxResponseCode3XXCount = 0;
        rxResponseCode4XXCount = 0;
        rxResponseCode5XXCount = 0;
        rxResponseCode6XXCount = 0;

        minHopsPerReq = 0;
        avgHopsPerReq = 0;
        maxHopsPerReq = 0;
        cummHopsPerReq = 0;

        nonInviteSessionsInitiated = 0;
        nonInviteSessionsSucceeded = 0;
        nonInviteSessionsFailed = 0;
        nonInviteSessionsFailedTimerF = 0;
        nonInviteSessionsFailedTransport = 0;

        MosLqMinScore  = 0;
        MosLqMaxScore  = 0;
        MosLqAvgScore  = 0;

        MinMosCQ  = 0;
        MaxMosCQ  = 0;
        AverageMosCQ  = 0;
    
        MinRFactorLQ = 0;
        MaxRFactorLQ = 0;
        AverageRFactorLQ = 0;
        
        MinRFactorCQ = 0;
        MaxRFactorCQ = 0;
        AverageRFactorCQ = 0;

        AverageVoIPPpdv = 0;
        MaxVoIPPpdv = 0;

        RtpPacketsRxCount = 0;
        RtpPacketsLostCount = 0;
        RtpPacketsDiscardCount = 0;
        
        MinRtpPacketsLost = 0;
        MaxRtpPacketsLost = 0;
        AverageRtpPacketsLost = 0;
        
        MinRtpPacketsDiscard = 0;
        MaxRtpPacketsDiscard = 0;
        AverageRtpPacketsDiscard = 0;       

        IntervalAbsMosAScore = 0;
        AudioIntervalTransportPktRcvd = 0;
        AudioIntervalTransportPktLost = 0;
        AudioIntervalTransportPktEffLoss = 0;
        AudioIntervalTransportPktDiscarded = 0;
        MinAbsMosAScore = 0;
        AvgAbsMosAScore = 0;
        MaxAbsMosAScore = 0;
        AbsMosABelowThresholdProp = 0;
        AudioPktLossDegradSev = 0;
        AudioTransportPkt = 0;
        AudioTransportPktLost = 0;
        AudioUncorrLossProp = 0;
        AudioBurst = 0;
        AudioBurstLoss = 0;
        AudioBurstLength = 0;
        AudioGap = 0;
        AudioGapLossRate = 0;
        AudioGapLength = 0;
        AudioPpdv = 0;

        IntervalAbsMosVScore = 0;
        IntervalRelMosVScore = 0;
        IntervalTransportPkt = 0;
        VideoIntervalTransportPktLost = 0;
        VideoIntervalTransportPktEffLossRate = 0;
        VideoIntervalTransportPktDiscarded = 0;
        VideoIntervalAvgPpdv = 0;
        AvgStreamVideoBw = 0;
        MinAbsMosVScore = 0;
        MaxAbsMosVScore = 0;
        AvgAbsMosVScore = 0;
        MinRelMosVScore = 0;
        MaxRelMosVScore = 0;
        AvgRelMosVScore = 0;
        VideoPktLossDegradSevScore = 0;
        VideoTransportPktCountScore = 0;
        VideoTransportPktLost = 0;
        VideoUncorrLossProp = 0;
        VideoBurst = 0;
        VideoBurstLength = 0;
        VideoBurstLossRate = 0;
        VideoGap = 0;
        VideoGapLossRate = 0;
        VideoGapLength = 0;
        IframeInterarrivalJitter = 0;
        VideoPpdv = 0;				

        sessionRefreshes = 0;

	vqResultsBlock->resetData();
    }
    
    uint32_t mHandle;   ///< block handle
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_APP_NS

#endif
