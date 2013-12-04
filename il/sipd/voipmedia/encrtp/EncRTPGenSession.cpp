/// @file
/// @brief Enc RTP Gen object
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include "VoIPUtils.h"
#include "EncRTPUtils.h"
#include "EncRTPGenSession.h"

#include "MediaPacket.h"

#include "EncodedMediaFile.h"
///////////////////////////////////////////////////////////////////////////////

USING_MEDIAFILES_NS;

DECL_ENCRTP_NS

///////////////////////////////////////////////////////////////////////////////

class EncRTPGenSessionImpl : public boost::noncopyable {

private:

	void init() {
		mPacketCounter=0;
		mStartTime=0;
		mNextTime=0;
		mSchedulerInterval=0;
		mCurrentSchedulingTime=0;
		mFilePacketCounter=0;
		mStartPacketTs=0;
		mSchedulerIntervalStart = 0;
		mSchedulerIntervalFirst =true;
	}

public:

  explicit EncRTPGenSessionImpl(EncodedMediaStreamHandler file) : 
    mFile(file)
  {
    init();
  }

  virtual ~EncRTPGenSessionImpl() {
    ;
  }

  EncodedMediaStreamHandler getEncodedMediaStream() const {
    return mFile;
  }

  int start(uint64_t startTime, uint64_t schedulerInterval) {
    mStartTime=startTime;
    mSchedulerInterval=schedulerInterval;
    mNextTime=mStartTime;
    mCurrentSchedulingTime=mStartTime;
    mFilePacketCounter = 0;
    mStartPacketTs = VoIPUtils::MakeRandomInteger();
    mPacketCounter = 0;
	mSchedulerIntervalStart = 0;
	mSchedulerIntervalFirst =true;
    return 0;
  }

  void nextSchedulingInterval() {
	  //  uint64_t st = VoIPUtils::getMilliSeconds();
	  //  printf(" diff: %d int: %d\n",(int)(mCurrentSchedulingTime - st) , (int)mSchedulerInterval);

	    if(mSchedulerIntervalFirst)
	    {
	       mSchedulerIntervalStart = VoIPUtils::getMilliSeconds();
	       mSchedulerIntervalFirst =false;
	    }
	    else
	    {
	    	 uint64_t ts = VoIPUtils::getMilliSeconds();
	    	 mSchedulerInterval = ts - mSchedulerIntervalStart;
	    	 mSchedulerIntervalStart = ts;
	    }
    mCurrentSchedulingTime+=mSchedulerInterval;
  }

  MediaPacketDescHandler getNextPacketDesc(EndpointHandler ep,uint32_t csrc) {

      MediaPacketDescHandler pktDesc;

      if(mFile && mSchedulerInterval && VoIPUtils::time_before64(mNextTime,mCurrentSchedulingTime+mSchedulerInterval)) {
          uint64_t pktTimeCurr;
          uint64_t pktTimeDiff;
          uint64_t size = mFile->getSize();


          if( (int)size > 1 ) {

              const MediaPacket* packetInFile = mFile->getPacket(mFilePacketCounter,0);

              if(packetInFile) {
                  pktDesc.reset(new MediaPacketDesc());

                  pktTimeCurr = packetInFile->getTime();

                  pktDesc->setEp(ep);
                  pktDesc->setmFile(mFile);
                  pktDesc->setIndex(mFilePacketCounter);
                  pktDesc->setTime(mNextTime);
                  RtpHeaderModifier mo;
                  mo.rtp_mf_csrcc = csrc;
                  mo.rtp_mf_sn = static_cast<uint16_t>(mPacketCounter);
                  mo.rtp_mf_ts = mStartPacketTs;
                  pktDesc->setModifier(mo);


                  mPacketCounter++;
                  mFilePacketCounter++;

                  const MediaPacket* packetInFileNext = mFile->getPacket(mFilePacketCounter,0);

                  if(packetInFileNext) {
                      pktTimeDiff = VoIPUtils::time_minus64(packetInFileNext->getTime(),pktTimeCurr);
                      mNextTime += pktTimeDiff;

                      if(mFilePacketCounter >= (size-1) ) {
                          mFilePacketCounter = 0;
                          RtpHeaderParam hdrParam;
                          packetInFileNext->getRtpHeaderParam (hdrParam);
                          mStartPacketTs += hdrParam.rtp_ts;
                      }
                  }
              }
          }
      }
      return pktDesc;
  }

  int stop() {
    init();
    return 0;
  }

private:
  uint64_t mPacketCounter;
  uint64_t mStartTime;
  uint64_t mNextTime;
  uint64_t mSchedulerInterval;
  uint64_t mCurrentSchedulingTime;
  EncodedMediaStreamHandler mFile;
  uint64_t mFilePacketCounter;
  uint32_t mStartPacketTs;
  uint64_t mSchedulerIntervalStart;
  bool mSchedulerIntervalFirst;
};

MediaPacketDesc::MediaPacketDesc() {
    mTime = 0;
    mIndex = 0;
    mFile.reset();
    mEp.reset();
}

MediaPacket* MediaPacketDesc::getPacket(int cpu) {
    MediaPacket* pkt = NULL;
    if(mFile){
        pkt = mFile->getPacket(mIndex,cpu);
        if(pkt)
            pkt->RtpHeaderParamModify(mHeaderModifier);
    }
    return pkt;
}

void MediaPacketDesc::setModifier(MEDIAFILES_NS::RtpHeaderModifier &mo){
    mHeaderModifier.rtp_mf_sn = mo.rtp_mf_sn;
    mHeaderModifier.rtp_mf_csrcc = mo.rtp_mf_csrcc;
    mHeaderModifier.rtp_mf_ts = mo.rtp_mf_ts;
    mHeaderModifier.rtp_mf_ssrc = mo.rtp_mf_ssrc;
}

void MediaPacketDesc::setEp(EndpointHandler ep){
    mEp = ep;
}

//////////////////////////////////////////////////////////////////////////////////////////

EncRTPGenSession::EncRTPGenSession(EncodedMediaStreamHandler file) {
  mPimpl.reset(new EncRTPGenSessionImpl(file));
}
EncRTPGenSession::~EncRTPGenSession() {
  ;
}
EncodedMediaStreamHandler EncRTPGenSession::getEncodedMediaStream() const {
  return mPimpl->getEncodedMediaStream();
}
int EncRTPGenSession::start(uint64_t startTime,uint64_t schedulerInterval) {
  return mPimpl->start(startTime,schedulerInterval);
}

void EncRTPGenSession::nextSchedulingInterval() {
  mPimpl->nextSchedulingInterval();
}

MediaPacketDescHandler EncRTPGenSession::getNextPacketDesc(EndpointHandler ep,uint32_t csrc) {
  return mPimpl->getNextPacketDesc(ep,csrc);
}
int EncRTPGenSession::stop() {
  return mPimpl->stop();
}

///////////////////////////////////////////////////////////////////////////////

END_DECL_ENCRTP_NS

////////////////////////////////////////////////////////////////////////////////

