/// @file
/// @brief McObjects (MCO) embedded database driver implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <boost/dynamic_bitset.hpp>
#include <hal/TimeStamp.h>
#include <phxexception/PHXException.h>
#include <statsframework/ExtremeStatsDBFactory.h>

#include "McoDriver.h"
#include "VoIPLog.h"
#include "statsdb.h"
#include "UserAgentRawStats.h"

USING_APP_NS;

///////////////////////////////////////////////////////////////////////////////

class McoDriver::UniqueIndexTracker
{
public:
    UniqueIndexTracker(void)
        : mCount(0)
    {
    }

    uint32_t Assign(void)
    {
        if (mCount < mBitset.size())
        {
            const size_t pos = mBitset.find_first();
            if (pos == bitset_t::npos)
                throw EPHXInternal("McoDriver::UniqueIndexTracker::Assign");

            mBitset.reset(pos);
            mCount++;

            return static_cast<uint32_t>(pos);
        }
        else
        {
            mBitset.push_back(false);
            mCount++;
            
            return static_cast<uint32_t>(mBitset.size() - 1);
        }
    }

    void Release(uint32_t index)
    {
        mBitset.set(index);
        mCount--;
    }
    
private:
    typedef boost::dynamic_bitset<unsigned long> bitset_t;

    static const uint32_t INVALID_INDEX = static_cast<const uint32_t>(-1);
    
    size_t mCount;              ///< number of index values assigned
    bitset_t mBitset;           ///< dynamic bitset tracking index values (0 = assigned, 1 = unassigned)
};

///////////////////////////////////////////////////////////////////////////////

class McoDriver::TransactionImpl
{
  public:
    TransactionImpl(uint16_t port, mco_db_h db);

    ~TransactionImpl()
    {
        Rollback();
    }

    mco_trans_h Trans(void) { return mTrans; }

    void Commit(void);
    void Rollback(void);
    
  private:
    const uint16_t mPort;
    mco_trans_h mTrans;
    bool mActive;
};

///////////////////////////////////////////////////////////////////////////////

McoDriver::TransactionImpl::TransactionImpl(uint16_t port, mco_db_h db)
    : mPort(port),
      mActive(true)
{
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &mTrans);
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::TransactionImpl::Commit(void)
{
    if (mActive)
    {
        mActive = false;
        
        if (mco_trans_commit(mTrans) != MCO_S_OK)
        {
            const std::string err("[McoDriver::TransactionImpl::Commit] Could not commit transaction");
            TC_LOG_ERR(mPort, err);
            throw EPHXInternal(err);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::TransactionImpl::Rollback(void)
{
    if (mActive)
    {
        mActive = false;
        mco_trans_rollback(mTrans);
    }
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::StaticInitialize(void)
{
    // Configure the stats db factory parameters before it instantiates any databases
    ExtremeStatsDBFactory::setDictionary(statsdb_get_dictionary());  // required
}

///////////////////////////////////////////////////////////////////////////////

McoDriver::McoDriver(uint16_t port)
    : mPort(port),
      mSipUaIndexTracker(new UniqueIndexTracker)
{
    // Get the handle to the McObjects database for this port from the StatsDBFactory wrapper class
    StatsDBFactory *dbFactory = StatsDBFactory::instance(mPort);
    mDBFactory = dynamic_cast<ExtremeStatsDBFactory *>(dbFactory);
    if (!mDBFactory)
        throw EPHXInternal("McoDriver::McoDriver");
}

///////////////////////////////////////////////////////////////////////////////

McoDriver::~McoDriver()
{
}

///////////////////////////////////////////////////////////////////////////////

McoDriver::Transaction McoDriver::StartTransaction(void)
{
    return Transaction(new TransactionImpl(mPort, reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle())));
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::CommitTransaction(Transaction t)
{
    t->Commit();
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::RollbackTransaction(Transaction t)
{
    t->Rollback();
}

///////////////////////////////////////////////////////////////////////////////

void McoDriver::InsertSipUaResult(uint32_t handle, uint64_t totalUaCount)
{
    // Select a new index value for this handle
    uint32_t index = mSipUaIndexTracker->Assign();
    
    // All MCO operations must be done in the context of a transaction
    mco_db_h db = reinterpret_cast<mco_db_h>(mDBFactory->getDBHandle());
    mco_trans_h t;
    mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    // Insert a new row in the table
    SipUaResults res;

    // Create a new SipUaResults object
    if (SipUaResults_new(t, &res) != MCO_S_OK)
    {
        // Rollback transaction
        mco_trans_rollback(t);

        // Return unused block index
        mSipUaIndexTracker->Release(index);

        const std::string err("[McoDriver::InsertSipUaResult] Could not create a SipUaResults object");
        TC_LOG_ERR(mPort, err);
        throw EPHXInternal(err);
    }

    // Initialize the results fields
    SipUaResults_BlockIndex_put(&res, index);
    SipUaResults_Handle_put(&res, handle);
    SipUaResults_TotalUaCount_put(&res, totalUaCount);
    SipUaResults_IntendedRegLoad_put(&res, 0);
    SipUaResults_RegAttempts_put(&res, 0);
    SipUaResults_RegSuccesses_put(&res, 0);
    SipUaResults_RegRetries_put(&res, 0);
    SipUaResults_RegFailures_put(&res, 0);
    SipUaResults_RegMinTimeMsec_put(&res, 0);
    SipUaResults_RegMaxTimeMsec_put(&res, 0);
    SipUaResults_RegCummTimeMsec_put(&res, 0);
    SipUaResults_RegAvgTimeMsec_put(&res, 0);
    SipUaResults_IntendedCallLoad_put(&res, 0);
    SipUaResults_CallAttempts_put(&res, 0);
    SipUaResults_CallSuccesses_put(&res, 0);
    SipUaResults_CallFailures_put(&res, 0);
    SipUaResults_CallAnswers_put(&res, 0);
    SipUaResults_CallMinTimeMsec_put(&res, 0);
    SipUaResults_CallMaxTimeMsec_put(&res, 0);
    SipUaResults_CallCummTimeMsec_put(&res, 0);
    SipUaResults_CallAvgTimeMsec_put(&res, 0);
    SipUaResults_MosLqMinScore_put(&res, 0);
    SipUaResults_MosLqMaxScore_put(&res, 0);
    SipUaResults_MosLqAvgScore_put(&res, 0);
    SipUaResults_MinMosCQ_put(&res, 0);
    SipUaResults_MaxMosCQ_put(&res, 0);
    SipUaResults_AverageMosCQ_put(&res, 0);
    SipUaResults_MinRFactorLQ_put(&res, 0);
    SipUaResults_MaxRFactorLQ_put(&res, 0);
    SipUaResults_AverageRFactorLQ_put(&res, 0);    
    SipUaResults_MinRFactorCQ_put(&res, 0);
    SipUaResults_MaxRFactorCQ_put(&res, 0);
    SipUaResults_AverageRFactorCQ_put(&res, 0);    
    SipUaResults_AverageVoIPPpdv_put(&res, 0);    
    SipUaResults_MaxVoIPPpdv_put(&res, 0);    
    SipUaResults_RtpPacketsRx_put(&res, 0);    
    SipUaResults_RtpPacketsLost_put(&res, 0);
    SipUaResults_RtpPacketsDiscard_put(&res, 0);
    SipUaResults_MinRtpPacketsLost_put(&res, 0);
    SipUaResults_MaxRtpPacketsLost_put(&res, 0);
    SipUaResults_AverageRtpPacketsLost_put(&res, 0);
    SipUaResults_MinRtpPacketsDiscard_put(&res, 0);
    SipUaResults_MaxRtpPacketsDiscard_put(&res, 0);
    SipUaResults_AverageRtpPacketsDiscard_put(&res, 0);
    SipUaResults_IntervalAbsMosAScore_put(&res, 0);
    SipUaResults_AudioIntervalTransportPktRcvd_put(&res, 0);
    SipUaResults_AudioIntervalTransportPktLost_put(&res, 0);
    SipUaResults_AudioIntervalTransportPktEffLoss_put(&res, 0);
    SipUaResults_AudioIntervalTransportPktDiscarded_put(&res, 0);    
    SipUaResults_MinAbsMosAScore_put(&res, 0);
    SipUaResults_AvgAbsMosAScore_put(&res, 0);
    SipUaResults_MaxAbsMosAScore_put(&res, 0);
    SipUaResults_AbsMosABelowThresholdProp_put(&res, 0);
    SipUaResults_AudioPktLossDegradSev_put(&res, 0);
    SipUaResults_AudioTransportPkt_put(&res, 0);
    SipUaResults_AudioTransportPktLost_put(&res, 0);
    SipUaResults_AudioUncorrLossProp_put(&res, 0);
    SipUaResults_AudioBurst_put(&res, 0);
    SipUaResults_AudioBurstLoss_put(&res, 0);
    SipUaResults_AudioBurstLength_put(&res, 0);
    SipUaResults_AudioGap_put(&res, 0);
    SipUaResults_AudioGapLossRate_put(&res, 0);
    SipUaResults_AudioGapLength_put(&res, 0);
    SipUaResults_AudioPpdv_put(&res, 0);
    SipUaResults_IntervalAbsMosVScore_put(&res, 0);
    SipUaResults_IntervalRelMosVScore_put(&res, 0);
    SipUaResults_IntervalTransportPkt_put(&res, 0);
    SipUaResults_VideoIntervalTransportPktLost_put(&res, 0);
    SipUaResults_VideoIntervalTransportPktEffLossRate_put(&res, 0);
    SipUaResults_VideoIntervalTransportPktDiscarded_put(&res, 0);
    SipUaResults_VideoIntervalAvgPpdv_put(&res, 0);
    SipUaResults_AvgStreamVideoBw_put(&res, 0);
    SipUaResults_MinAbsMosVScore_put(&res, 0);
    SipUaResults_MaxAbsMosVScore_put(&res, 0);
    SipUaResults_AvgAbsMosVScore_put(&res, 0);
    SipUaResults_MinRelMosVScore_put(&res, 0);
    SipUaResults_MaxRelMosVScore_put(&res, 0);
    SipUaResults_AvgRelMosVScore_put(&res, 0);
    SipUaResults_VideoPktLossDegradSevScore_put(&res, 0);
    SipUaResults_VideoTransportPktCountScore_put(&res, 0);
    SipUaResults_VideoTransportPktLost_put(&res, 0);
    SipUaResults_VideoUncorrLossProp_put(&res, 0);
    SipUaResults_VideoBurst_put(&res, 0);
    SipUaResults_VideoBurstLength_put(&res, 0);
    SipUaResults_VideoBurstLossRate_put(&res, 0);
    SipUaResults_VideoGap_put(&res, 0);
    SipUaResults_VideoGapLossRate_put(&res, 0);
    SipUaResults_VideoGapLength_put(&res, 0);
    SipUaResults_IframeInterarrivalJitter_put(&res, 0);
    SipUaResults_VideoPpdv_put(&res, 0);		
    SipUaResults_TxResponseCode1XX_put(&res, 0);
    SipUaResults_TxResponseCode2XX_put(&res, 0);
    SipUaResults_TxResponseCode3XX_put(&res, 0);    
    SipUaResults_TxResponseCode4XX_put(&res, 0);
    SipUaResults_TxResponseCode5XX_put(&res, 0);
    SipUaResults_TxResponseCode6XX_put(&res, 0);
    SipUaResults_RxResponseCode1XX_put(&res, 0);
    SipUaResults_RxResponseCode2XX_put(&res, 0);
    SipUaResults_RxResponseCode3XX_put(&res, 0);    
    SipUaResults_RxResponseCode4XX_put(&res, 0);
    SipUaResults_RxResponseCode5XX_put(&res, 0);
    SipUaResults_RxResponseCode6XX_put(&res, 0);
    SipUaResults_SessionRefreshes_put(&res, 0);
    SipUaResults_MinResponseDelay_put(&res, 0);
    SipUaResults_AverageResponseDelay_put(&res, 0);
    SipUaResults_MaxResponseDelay_put(&res, 0);
    SipUaResults_MinPostDialDelay_put(&res, 0);
    SipUaResults_AveragePostDialDelay_put(&res, 0);
    SipUaResults_MaxPostDialDelay_put(&res, 0);
    SipUaResults_MinSessionAckDelay_put(&res, 0);
    SipUaResults_AverageSessionAckDelay_put(&res, 0);
    SipUaResults_MaxSessionAckDelay_put(&res, 0);
    SipUaResults_MinFailedSessionSetupDelay_put(&res, 0);
    SipUaResults_AverageFailedSessionSetupDelay_put(&res, 0);
    SipUaResults_MaxFailedSessionSetupDelay_put(&res, 0);
    SipUaResults_MinDisconnectResponseDelay_put(&res, 0);
    SipUaResults_AverageDisconnectResponseDelay_put(&res, 0);
    SipUaResults_MaxDisconnectResponseDelay_put(&res, 0);
    SipUaResults_FailedSessionDisconnectCount_put(&res, 0);
    SipUaResults_MinSucSessionDuration_put(&res, 0);
    SipUaResults_AverageSucSessionDuration_put(&res, 0);
    SipUaResults_MaxSucSessionDuration_put(&res, 0);
    SipUaResults_MinHopsPerReq_put(&res, 0);
    SipUaResults_AverageHopsPerReq_put(&res, 0);
    SipUaResults_MaxHopsPerReq_put(&res, 0);
    SipUaResults_MinUaRegFailedTime_put(&res, 0);
    SipUaResults_AverageUaRegFailedTime_put(&res, 0);
    SipUaResults_MaxUaRegFailedTime_put(&res, 0);
    SipUaResults_ActiveCallCount_put(&res, 0);
    SipUaResults_FailedCallTimerBCount_put(&res, 0);
    SipUaResults_FailedCallTransErrorCount_put(&res, 0);
    SipUaResults_FailedCall5xxCount_put(&res, 0);
    SipUaResults_NonInviteSessionsInitiatedCount_put(&res, 0);
    SipUaResults_NonInviteSessionsSucCount_put(&res, 0);
    SipUaResults_NonInviteSessionsFailCount_put(&res, 0);
    SipUaResults_NonInviteSessionsFailTimerFCount_put(&res, 0);
    SipUaResults_NonInviteSessionsFailTransErrCount_put(&res, 0);
    SipUaResults_TxInviteCount_put(&res, 0);
    SipUaResults_RxInviteCount_put(&res, 0);
    SipUaResults_ReTranInviteCount_put(&res, 0);   
    SipUaResults_TxAckCount_put(&res, 0);
    SipUaResults_RxAckCount_put(&res, 0);
    SipUaResults_TxByeCount_put(&res, 0);
    SipUaResults_RxByeCount_put(&res, 0);
    SipUaResults_ReTranByeCount_put(&res, 0);
    SipUaResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
    
    // Commit insert into the database
    if (mco_trans_commit(t) != MCO_S_OK)
    {
        mSipUaIndexTracker->Release(index);

        const std::string err("[McoDriver::InsertSipUaResult] Could not commit insertion of SipUaResults object");
        TC_LOG_ERR(mPort, err);
        throw EPHXInternal(err);
    }

    TC_LOG_INFO(mPort, "[McoDriver::InsertSipUaResult] Created SipUaResults object with index " << index << " for handle " << handle);
}

///////////////////////////////////////////////////////////////////////////////

bool McoDriver::UpdateSipUaResult(UserAgentRawStats& stats, Transaction& t)
{
    const uint32_t handle = stats.Handle();
    
    // Lookup the row in the table
    SipUaResults res;
    mco_cursor_t csr;

    if ((SipUaResults_HandleIndex_index_cursor(t->Trans(), &csr) != MCO_S_OK) ||
        (SipUaResults_HandleIndex_search(t->Trans(), &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (SipUaResults_from_cursor(t->Trans(), &csr, &res) != MCO_S_OK))
    {
        TC_LOG_ERR(mPort, "[McoDriver::UpdateSipUaResult] Could not update SipUaResults object with handle " << handle);
        return false;
    }

    // Update the results fields
    SipUaResults_IntendedRegLoad_put(&res, stats.intendedRegLoad);
    SipUaResults_RegAttempts_put(&res, stats.regAttempts);
    SipUaResults_RegSuccesses_put(&res, stats.regSuccesses);
    SipUaResults_RegRetries_put(&res, stats.regRetries);
    SipUaResults_RegFailures_put(&res, stats.regFailures);
    SipUaResults_RegMinTimeMsec_put(&res, stats.regMinTimeMsec);
    SipUaResults_RegMaxTimeMsec_put(&res, stats.regMaxTimeMsec);
    SipUaResults_RegCummTimeMsec_put(&res, stats.regCummTimeMsec);
    SipUaResults_RegAvgTimeMsec_put(&res, stats.regAvgTimeMsec);
    SipUaResults_IntendedCallLoad_put(&res, stats.intendedCallLoad);
    SipUaResults_CallAttempts_put(&res, stats.callAttempts);
    SipUaResults_CallSuccesses_put(&res, stats.callSuccesses);
    SipUaResults_CallFailures_put(&res, stats.callFailures);
    SipUaResults_CallAnswers_put(&res, stats.callAnswers);
    SipUaResults_CallMinTimeMsec_put(&res, stats.callMinTimeMsec);
    SipUaResults_CallMaxTimeMsec_put(&res, stats.callMaxTimeMsec);
    SipUaResults_CallCummTimeMsec_put(&res, stats.callCummTimeMsec);
    SipUaResults_CallAvgTimeMsec_put(&res, stats.callAvgTimeMsec);
    SipUaResults_MosLqMinScore_put(&res, stats.MosLqMinScore);
    SipUaResults_MosLqMaxScore_put(&res, stats.MosLqMaxScore);
    SipUaResults_MosLqAvgScore_put(&res, stats.MosLqAvgScore);
    SipUaResults_MinMosCQ_put(&res, stats.MinMosCQ);
    SipUaResults_MaxMosCQ_put(&res, stats.MaxMosCQ);
    SipUaResults_AverageMosCQ_put(&res, stats.AverageMosCQ);
    SipUaResults_MinRFactorLQ_put(&res, stats.MinRFactorLQ);
    SipUaResults_MaxRFactorLQ_put(&res, stats.MaxRFactorLQ);
    SipUaResults_AverageRFactorLQ_put(&res, stats.AverageRFactorLQ);    
    SipUaResults_MinRFactorCQ_put(&res, stats.MinRFactorCQ);
    SipUaResults_MaxRFactorCQ_put(&res, stats.MaxRFactorCQ);
    SipUaResults_AverageRFactorCQ_put(&res, stats.AverageRFactorCQ);    
    SipUaResults_AverageVoIPPpdv_put(&res, stats.AverageVoIPPpdv);    
    SipUaResults_MaxVoIPPpdv_put(&res, stats.MaxVoIPPpdv);    
    SipUaResults_RtpPacketsRx_put(&res, stats.RtpPacketsRxCount);    
    SipUaResults_RtpPacketsLost_put(&res, stats.RtpPacketsLostCount);    
    SipUaResults_RtpPacketsDiscard_put(&res, stats.RtpPacketsDiscardCount);    
    SipUaResults_MinRtpPacketsLost_put(&res, stats.MinRtpPacketsLost);    
    SipUaResults_MaxRtpPacketsLost_put(&res, stats.MaxRtpPacketsLost);    
    SipUaResults_AverageRtpPacketsLost_put(&res, stats.AverageRtpPacketsLost);    
    SipUaResults_MinRtpPacketsDiscard_put(&res, stats.MinRtpPacketsDiscard);    
    SipUaResults_MaxRtpPacketsDiscard_put(&res, stats.MaxRtpPacketsDiscard);    
    SipUaResults_AverageRtpPacketsDiscard_put(&res, stats.AverageRtpPacketsDiscard);    
    SipUaResults_IntervalAbsMosAScore_put(&res, stats.IntervalAbsMosAScore);
    SipUaResults_AudioIntervalTransportPktRcvd_put(&res, stats.AudioIntervalTransportPktRcvd);
    SipUaResults_AudioIntervalTransportPktLost_put(&res, stats.AudioIntervalTransportPktLost);
    SipUaResults_AudioIntervalTransportPktEffLoss_put(&res, stats.AudioIntervalTransportPktEffLoss);
    SipUaResults_AudioIntervalTransportPktDiscarded_put(&res, stats.AudioIntervalTransportPktDiscarded);    
    SipUaResults_MinAbsMosAScore_put(&res, stats.MinAbsMosAScore);
    SipUaResults_AvgAbsMosAScore_put(&res, stats.AvgAbsMosAScore);
    SipUaResults_MaxAbsMosAScore_put(&res, stats.MaxAbsMosAScore);
    SipUaResults_AbsMosABelowThresholdProp_put(&res, stats.AbsMosABelowThresholdProp);
    SipUaResults_AudioPktLossDegradSev_put(&res, stats.AudioPktLossDegradSev);
    SipUaResults_AudioTransportPkt_put(&res, stats.AudioTransportPkt);
    SipUaResults_AudioTransportPktLost_put(&res, stats.AudioTransportPktLost);
    SipUaResults_AudioUncorrLossProp_put(&res, stats.AudioUncorrLossProp);
    SipUaResults_AudioBurst_put(&res, stats.AudioBurst);
    SipUaResults_AudioBurstLoss_put(&res, stats.AudioBurstLoss);
    SipUaResults_AudioBurstLength_put(&res, stats.AudioBurstLength);
    SipUaResults_AudioGap_put(&res, stats.AudioGap);
    SipUaResults_AudioGapLossRate_put(&res, stats.AudioGapLossRate);
    SipUaResults_AudioGapLength_put(&res, stats.AudioGapLength);
    SipUaResults_AudioPpdv_put(&res, stats.AudioPpdv);
    SipUaResults_IntervalAbsMosVScore_put(&res, stats.IntervalAbsMosVScore);
    SipUaResults_IntervalRelMosVScore_put(&res, stats.IntervalRelMosVScore);
    SipUaResults_IntervalTransportPkt_put(&res, stats.IntervalTransportPkt);
    SipUaResults_VideoIntervalTransportPktLost_put(&res, stats.VideoIntervalTransportPktLost);
    SipUaResults_VideoIntervalTransportPktEffLossRate_put(&res, stats.VideoIntervalTransportPktEffLossRate);
    SipUaResults_VideoIntervalTransportPktDiscarded_put(&res, stats.VideoIntervalTransportPktDiscarded);
    SipUaResults_VideoIntervalAvgPpdv_put(&res, stats.VideoIntervalAvgPpdv);
    SipUaResults_AvgStreamVideoBw_put(&res, stats.AvgStreamVideoBw);
    SipUaResults_MinAbsMosVScore_put(&res, stats.MinAbsMosVScore);
    SipUaResults_MaxAbsMosVScore_put(&res, stats.MaxAbsMosVScore);
    SipUaResults_AvgAbsMosVScore_put(&res, stats.AvgAbsMosVScore);
    SipUaResults_MinRelMosVScore_put(&res, stats.MinRelMosVScore);
    SipUaResults_MaxRelMosVScore_put(&res, stats.MaxRelMosVScore);
    SipUaResults_AvgRelMosVScore_put(&res, stats.AvgRelMosVScore);
    SipUaResults_VideoPktLossDegradSevScore_put(&res, stats.VideoPktLossDegradSevScore);
    SipUaResults_VideoTransportPktCountScore_put(&res, stats.VideoTransportPktCountScore);
    SipUaResults_VideoTransportPktLost_put(&res, stats.VideoTransportPktLost);
    SipUaResults_VideoUncorrLossProp_put(&res, stats.VideoUncorrLossProp);
    SipUaResults_VideoBurst_put(&res, stats.VideoBurst);
    SipUaResults_VideoBurstLength_put(&res, stats.VideoBurstLength);
    SipUaResults_VideoBurstLossRate_put(&res, stats.VideoBurstLossRate);
    SipUaResults_VideoGap_put(&res, stats.VideoGap);
    SipUaResults_VideoGapLossRate_put(&res, stats.VideoGapLossRate);
    SipUaResults_VideoGapLength_put(&res, stats.VideoGapLength);
    SipUaResults_IframeInterarrivalJitter_put(&res, stats.IframeInterarrivalJitter);
    SipUaResults_VideoPpdv_put(&res, stats.VideoPpdv);		
    SipUaResults_TxResponseCode1XX_put(&res, stats.txResponseCode1XXCount);
    SipUaResults_TxResponseCode2XX_put(&res, stats.txResponseCode2XXCount);
    SipUaResults_TxResponseCode3XX_put(&res, stats.txResponseCode3XXCount);    
    SipUaResults_TxResponseCode4XX_put(&res, stats.txResponseCode4XXCount);
    SipUaResults_TxResponseCode5XX_put(&res, stats.txResponseCode5XXCount);
    SipUaResults_TxResponseCode6XX_put(&res, stats.txResponseCode6XXCount);
    SipUaResults_RxResponseCode1XX_put(&res, stats.rxResponseCode1XXCount);
    SipUaResults_RxResponseCode2XX_put(&res, stats.rxResponseCode2XXCount);
    SipUaResults_RxResponseCode3XX_put(&res, stats.rxResponseCode3XXCount);    
    SipUaResults_RxResponseCode4XX_put(&res, stats.rxResponseCode4XXCount);
    SipUaResults_RxResponseCode5XX_put(&res, stats.rxResponseCode5XXCount);
    SipUaResults_RxResponseCode6XX_put(&res, stats.rxResponseCode6XXCount);
    SipUaResults_SessionRefreshes_put(&res, stats.sessionRefreshes);
    SipUaResults_MinResponseDelay_put(&res, stats.minResponseDelay);
    SipUaResults_AverageResponseDelay_put(&res, stats.avgResponseDelay);
    SipUaResults_MaxResponseDelay_put(&res, stats.maxResponseDelay);
    SipUaResults_MinPostDialDelay_put(&res, stats.minPostDialDelay);
    SipUaResults_AveragePostDialDelay_put(&res, stats.avgPostDialDelay);
    SipUaResults_MaxPostDialDelay_put(&res, stats.maxPostDialDelay);
    SipUaResults_MinSessionAckDelay_put(&res, stats.minSessionAckDelay);
    SipUaResults_AverageSessionAckDelay_put(&res, stats.avgSessionAckDelay);
    SipUaResults_MaxSessionAckDelay_put(&res, stats.maxSessionAckDelay);
    SipUaResults_MinFailedSessionSetupDelay_put(&res, stats.minFailedSessionSetupDelay);
    SipUaResults_AverageFailedSessionSetupDelay_put(&res, stats.avgFailedSessionSetupDelay);
    SipUaResults_MaxFailedSessionSetupDelay_put(&res, stats.maxFailedSessionSetupDelay);
    SipUaResults_MinDisconnectResponseDelay_put(&res, stats.minDisconnectResponseDelay);
    SipUaResults_AverageDisconnectResponseDelay_put(&res, stats.avgDisconnectResponseDelay);
    SipUaResults_MaxDisconnectResponseDelay_put(&res, stats.maxDisconnectResponseDelay);
    SipUaResults_FailedSessionDisconnectCount_put(&res, stats.failedSessionDisconnectCount);
    SipUaResults_MinSucSessionDuration_put(&res, stats.minSuccessfulSessionDuration);
    SipUaResults_AverageSucSessionDuration_put(&res, stats.avgSuccessfulSessionDuration);
    SipUaResults_MaxSucSessionDuration_put(&res, stats.maxSuccessfulSessionDuration);
    SipUaResults_MinHopsPerReq_put(&res, stats.minHopsPerReq);
    SipUaResults_AverageHopsPerReq_put(&res, stats.avgHopsPerReq);
    SipUaResults_MaxHopsPerReq_put(&res, stats.maxHopsPerReq);
    SipUaResults_MinUaRegFailedTime_put(&res, stats.minFailedRegistrationDelay);
    SipUaResults_AverageUaRegFailedTime_put(&res, stats.avgFailedRegistrationDelay);
    SipUaResults_MaxUaRegFailedTime_put(&res, stats.maxFailedRegistrationDelay);
    SipUaResults_ActiveCallCount_put(&res, stats.callsActive);
    SipUaResults_FailedCallTimerBCount_put(&res, stats.callsFailedTimerB);
    SipUaResults_FailedCallTransErrorCount_put(&res, stats.callsFailedTransportError);
    SipUaResults_FailedCall5xxCount_put(&res, stats.callsFailed5xx);
    SipUaResults_NonInviteSessionsInitiatedCount_put(&res, stats.nonInviteSessionsInitiated);
    SipUaResults_NonInviteSessionsSucCount_put(&res, stats.nonInviteSessionsSucceeded);
    SipUaResults_NonInviteSessionsFailCount_put(&res, stats.nonInviteSessionsFailed);
    SipUaResults_NonInviteSessionsFailTimerFCount_put(&res, stats.nonInviteSessionsFailedTimerF);
    SipUaResults_NonInviteSessionsFailTransErrCount_put(&res, stats.nonInviteSessionsFailedTransport);
    SipUaResults_TxInviteCount_put(&res, stats.inviteRequestsSent);
    SipUaResults_RxInviteCount_put(&res, stats.inviteRequestsReceived);
    SipUaResults_ReTranInviteCount_put(&res, stats.inviteMessagesRetransmitted);   
    SipUaResults_TxAckCount_put(&res, stats.ackRequestsSent);
    SipUaResults_RxAckCount_put(&res, stats.ackRequestsReceived);
    SipUaResults_TxByeCount_put(&res, stats.byeRequestsSent);
    SipUaResults_RxByeCount_put(&res, stats.byeRequestsReceived);
    SipUaResults_ReTranByeCount_put(&res, stats.byeMessagesRetransmitted);
    SipUaResults_LastModified_put(&res, Hal::TimeStamp::getTimeStamp());
        
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool McoDriver::UpdateSipUaResult(UserAgentRawStats& stats)
{
    Transaction t = StartTransaction();
    const bool success = UpdateSipUaResult(stats, t);
    if (success)
        CommitTransaction(t);

    return success;
}

///////////////////////////////////////////////////////////////////////////////

bool McoDriver::DeleteSipUaResult(uint32_t handle, Transaction& t)
{
    // Lookup the row in the table
    SipUaResults res;
    mco_cursor_t csr;

    if ((SipUaResults_HandleIndex_index_cursor(t->Trans(), &csr) != MCO_S_OK) ||
        (SipUaResults_HandleIndex_search(t->Trans(), &csr, MCO_EQ, handle) != MCO_S_OK) ||
        (SipUaResults_from_cursor(t->Trans(), &csr, &res) != MCO_S_OK))
    {
        TC_LOG_ERR(mPort, "[McoDriver::DeleteSipUaResult] Could not delete SipUaResults object with handle " << handle);
        return false;
    }

    uint32_t index;
    if (SipUaResults_BlockIndex_get(&res, &index) != MCO_S_OK)
    {
        TC_LOG_ERR(mPort, "[McoDriver::DeleteSipUaResult] Could not get SipUaResults index value for handle " << handle);
        return false;
    }
    
    // Delete the row
    SipUaResults_delete(&res);

    // Return now unused block index
    mSipUaIndexTracker->Release(index);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool McoDriver::DeleteSipUaResult(uint32_t handle)
{
    Transaction t = StartTransaction();
    const bool success = DeleteSipUaResult(handle, t);
    if (success)
        CommitTransaction(t);

    return success;
}

///////////////////////////////////////////////////////////////////////////////
