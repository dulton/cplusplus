/// @file
/// @brief VQStreamAudioHD class
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

////////////////////////////////////////////////////////////////////////////////

class VQMonSessionAudioHD : public VQMonSession {

public:

  explicit VQMonSessionAudioHD(VQInterfaceHandler interface,
			       CodecType codec, PayloadType payload, 
			       int bitRate, uint32_t ssrc, int jbSize) : VQMonSession(interface,codec, payload, bitRate, ssrc, jbSize)
  {
    if(init()==-1) {
      mInitFailed=true;
    }
  }

  virtual ~VQMonSessionAudioHD() {
    destroy();
  }
  
  /**
   * Get current VQ values for the stream.
   * Return -1 if the stream has not been fed properly.
   */
  virtual int getAudioHDMetrics(AudioHDVQualResultsHandler metrics) {
    if(metrics && mPacketCounter>=10) {
      vqmon_result_t result;
      tcmyU32        _nSize;

      struct timeval maskedNow;
      TimeUtils::GetTimestamp(&maskedNow);

      result = VQmonStreamIndicateEvent(mStreamHandle,
                                        vqmonStreamProgEventIntervalEnd,
                                        maskedNow,
                                        0,
                                        0);
      if(result != VQMON_ESUCCESS)
        return -1;

      vqmon_streammetrics_audiointerval_t _tAudioInterval;
      _nSize = sizeof(vqmon_streammetrics_audiointerval_t);
      memset(&_tAudioInterval, 0, _nSize);
      result = VQmonStreamMetricsGet(mStreamHandle,
                                     VQMON_STREAMMETRICBLOCKID_AUDIOINTERVAL,
                                     &_nSize,
                                     &_tAudioInterval);
      if(result != VQMON_ESUCCESS)
        return -1;

      vqmon_streammetrics_audioqual_t _tAudioQual;
      _nSize = sizeof(vqmon_streammetrics_audioqual_t);
      result = VQmonStreamMetricsGet(mStreamHandle,
                                     VQMON_STREAMMETRICBLOCKID_AUDIOQUAL,
                                     &_nSize,
                                     &_tAudioQual);
      if(result != VQMON_ESUCCESS)
        return -1;

      vqmon_streammetrics_pkttrans_t _tPktTrans;
      _nSize = sizeof(vqmon_streammetrics_pkttrans_t);
      result = VQmonStreamMetricsGet(mStreamHandle,
                                     VQMON_STREAMMETRICBLOCKID_PKTTRANS,
                                     &_nSize,
                                     &_tPktTrans);
      if(result != VQMON_ESUCCESS)
        return -1;      

      vqmon_streammetrics_jitter_t _tJitter;
      _nSize = sizeof(vqmon_streammetrics_jitter_t);
      result = VQmonStreamMetricsGet(mStreamHandle,
                                     VQMON_STREAMMETRICBLOCKID_JITTER,
                                     &_nSize,
                                     &_tJitter);
      if(result != VQMON_ESUCCESS)
        return -1;

      AudioHDVQualResultsRawData::AudioHDVQData data;

      data.InterMOS_A = _tAudioInterval.nMOS_A;
      data.AudioInterPktsRcvd = _tAudioInterval.nPktsRcvd;
      data.InterPktsLost = _tAudioInterval.nPktsLost;
      data.InterPktsDiscarded = _tAudioInterval.nPktsDiscarded;
      data.EffPktsLossRate = _tAudioInterval.nAvgEffPktLossRate;
      data.MinMOS_A = _tAudioQual.nMinMOS_A;
      data.AvgMOS_A = _tAudioQual.nAvgMOS_A;
      data.MaxMOS_A = _tAudioQual.nMaxMOS_A;
      data.PropBelowTholdMOS_A = _tAudioQual.nPropBelowTholdMOS_A;
      data.LossDeg = _tAudioQual.deg_factors.nLossDeg;
      data.PktsRcvd = _tPktTrans.nPktsRcvd;
      data.PktsLoss = _tPktTrans.nPktsLost;
      data.UncorrectedLossProp = _tPktTrans.nUncorrectedLossProp;
      data.BurstCount = _tPktTrans.nBurstCount;
      data.BurstLossRate = _tPktTrans.nBurstLossRate;
      data.BurstLength = _tPktTrans.nBurstLengthPkts;
      data.GapCount = _tPktTrans.nGapCount;
      data.GapLossRate = _tPktTrans.nGapLossRate;
      data.GapLength = _tPktTrans.nGapLengthPkts;
      data.PPDV = _tJitter.nPPDVMs;

      metrics->set(data);

      return 0;
    }   
    
    return 0;
  }

  virtual int getVoiceMetrics(VoiceVQualResultsHandler metrics) {
    return -1;
  }
  virtual int getVideoMetrics(VideoVQualResultsHandler metrics) {
    return -1;
  }

  virtual VQMediaType getMediaType() const {
    return VQ_AUDIOHD;
  }

protected:

  int init() {

    if(!mInterface) return -1;

    VQInterfaceImpl* interfaceImpl=static_cast<VQInterfaceImpl*>(mInterface.get());
    
    InterfaceType interfaceHandle = interfaceImpl->getInterface();

    if(!interfaceHandle) return -1;

    vqmon_interface_t* pInterface = VQMON_INTERFACE_HANDLETOPTR(interfaceHandle);
    if(!pInterface) return -1;

    vqmon_streamtype_t _eStreamType = vqmonStreamTypeAudio;
    VQmonStreamCreate(&mStreamHandle, interfaceHandle, _eStreamType);
    if (!VQMON_STREAM_ISVALID(mStreamHandle)) {
      return -1;
    }

    vqmon_streamcfg_codec_t _tCodecCfg;
    memset(&_tCodecCfg,0,sizeof(_tCodecCfg));

    _tCodecCfg.idCODECType = getVQmonCodec(mCodec,mPayload,mBitRate);
    interfaceImpl->setAudioHDCodec(_tCodecCfg.idCODECType);
    _tCodecCfg.bActivityDetectAvail = (mCodec == AAC_LD);
    if(mBitRate<1) _tCodecCfg.nCurrentRate = -1;
    else _tCodecCfg.nCurrentRate = mBitRate;

    vqmon_result_t res = VQmonStreamConfigSet(
                            mStreamHandle,
                            VQMON_STREAMCFGBLOCKID_CODECINFO,
                            sizeof(_tCodecCfg),
                            &_tCodecCfg);

    if(res != VQMON_ESUCCESS) {
      return -1;
    }

    vqmon_streamcfg_transprotoext_t _tTransProto;
    memset(&_tTransProto,0,sizeof(_tTransProto));

    _tTransProto.eTransportProto = vqmonStreamTransProtoRTP;
    _tTransProto.proto.rtp.nSSRC = mSSRC;

    VQmonStreamConfigSet(
                 mStreamHandle,
                 VQMON_STREAMCFGBLOCKID_TRANSPORTINFO,
                 sizeof(_tTransProto),
                 &_tTransProto);

    if(res != VQMON_ESUCCESS) {
      return -1;
    }

    vqmon_streamcfg_netepdesc_t _tEptDesc;
    memset(&_tEptDesc,0,sizeof(_tEptDesc));

    _tEptDesc.eType = vqmonStreamTypeAudio;

    int cct=VoIPUtils::getMilliSeconds() % 100;

    _tEptDesc.tSrcAddress.ip.v4.octet[0]=192;
    _tEptDesc.tSrcAddress.ip.v4.octet[1]=168;
    _tEptDesc.tSrcAddress.ip.v4.octet[2]=cct/256;
    _tEptDesc.tSrcAddress.ip.v4.octet[3]=cct%256;

    _tEptDesc.tDstAddress.ip.v4.octet[0]=10;
    _tEptDesc.tDstAddress.ip.v4.octet[1]=14;
    _tEptDesc.tDstAddress.ip.v4.octet[2]=cct/256;
    _tEptDesc.tDstAddress.ip.v4.octet[3]=cct%256;

    _tEptDesc.nSrcPort = 6000+cct;
    _tEptDesc.nDstPort = 6002+cct;

    _tEptDesc.eTransportProtos = (vqmon_streamtransproto_t)vqmonStreamTransProtoRTP;
    
    res = VQmonStreamConfigSet(
                       mStreamHandle,
                       VQMON_STREAMCFGBLOCKID_NETEPDESC,
                       sizeof(vqmon_streamcfg_netepdesc_t),
                       &_tEptDesc);

    if(res != VQMON_ESUCCESS) {
      return -1;
    }

    vqmon_streamcfg_audiogencfg_t _tAudioGenCfg;

    memset(&_tAudioGenCfg, 0, sizeof(_tAudioGenCfg));

    _tAudioGenCfg.nChannels = 1;
    _tAudioGenCfg.nRefClockRateHz = 48000;    /* 48 kHz */

    res = VQmonStreamConfigSet(
                       mStreamHandle,
                       VQMON_STREAMCFGBLOCKID_AUDIOGENCFG,
                       sizeof(vqmon_streamcfg_audiogencfg_t),
                       &_tAudioGenCfg);

    if(res != VQMON_ESUCCESS) {
      return -1;
    }         

    return 0;
  }
};

////////////////////////////////////////////////////////////////////////////////

