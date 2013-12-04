/// @file
/// @brief VQStreamVideo class
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

class VQMonSessionVideo : public VQMonSession {

public:

  explicit VQMonSessionVideo(VQInterfaceHandler interface,
			     CodecType codec, PayloadType payload, 
			     int bitRate, uint32_t ssrc, int jbSize) : VQMonSession(interface,codec, payload, bitRate, ssrc, jbSize)
  {
    if(init()==-1) {
      mInitFailed=true;
    }
  }

  virtual ~VQMonSessionVideo() {
    destroy();
  }
  
  /**
   * Get current VQ values for the stream.
   * Return -1 if the stream has not been fed properly.
   */
  virtual int getVideoMetrics(VideoVQualResultsHandler metrics) {    

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

      vqmon_streammetrics_videointerval_t _tVideoInterval;
      _nSize = sizeof(vqmon_streammetrics_videointerval_t);
      result = VQmonStreamMetricsGet(mStreamHandle,
                                     VQMON_STREAMMETRICBLOCKID_VIDEOINTERVAL,
                                     &_nSize,
                                     &_tVideoInterval);
      if(result != VQMON_ESUCCESS)
        return -1;     

      vqmon_streammetrics_videobw_t _tVideoBW;
      _nSize = sizeof(vqmon_streammetrics_videobw_t);
      result = VQmonStreamMetricsGet(mStreamHandle,
                                     VQMON_STREAMMETRICBLOCKID_VIDEOBW,
                                     &_nSize,
                                     &_tVideoBW);
      if(result != VQMON_ESUCCESS)
        return -1;      
      
      vqmon_streammetrics_videoqual_t _tVideoQual;
      _nSize = sizeof(vqmon_streammetrics_videoqual_t);
      result = VQmonStreamMetricsGet(mStreamHandle,
                                     VQMON_STREAMMETRICBLOCKID_VIDEOQUAL,
                                     &_nSize,
                                     &_tVideoQual);
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

      vqmon_streammetrics_videojitter_t _tVideoJitter;
      _nSize = sizeof(vqmon_streammetrics_videojitter_t);
      result = VQmonStreamMetricsGet(mStreamHandle,
                                     VQMON_STREAMMETRICBLOCKID_VIDEOJITTER,
                                     &_nSize,
                                     &_tVideoJitter);
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

      VideoVQualResultsRawData::VideoVQData data;

      data.InterAbsoluteMOS_V = _tVideoInterval.nAbsoluteMOS_V;
      data.InterRelativeMOS_V = _tVideoInterval.nRelativeMOS_V;
      data.InterPktsRcv = _tVideoInterval.nPktsRcvd;
      data.InterPktsLost = _tVideoInterval.nPktsLost;
      data.InterPktsDiscarded = _tVideoInterval.nPktsDiscarded;
      data.InterEffPktsLossRate = _tVideoInterval.nAvgEffPktLossRate;
      data.InterPPDV = _tVideoInterval.nPPDVMs;
      data.AvgVideoBandwidth = _tVideoBW.nAvgVideoBandwidth;
      data.MinAbsoluteMOS_V = _tVideoQual.nMinAbsoluteMOS_V;
      data.AvgAbsoluteMOS_V = _tVideoQual.nAvgAbsoluteMOS_V;
      data.MaxAbsoluteMOS_V = _tVideoQual.nMaxAbsoluteMOS_V;
      data.MinRelativeMOS_V = _tVideoQual.nMinRelativeMOS_V;
      data.AvgRelativeMOS_V = _tVideoQual.nAvgRelativeMOS_V;
      data.MaxRelativeMOS_V = _tVideoQual.nMaxRelativeMOS_V;
      data.LossDeg = _tVideoQual.deg_factors.nLossDeg;
      data.PktsRcvd = _tPktTrans.nPktsRcvd;
      data.PktsLost = _tPktTrans.nPktsLost;
      data.UncorrectedLossProp = _tPktTrans.nUncorrectedLossProp;
      data.BurstCount = _tPktTrans.nBurstCount;
      data.BurstLossRate = _tPktTrans.nBurstLossRate;
      data.BurstLength = _tPktTrans.nBurstLengthPkts;
      data.GapCount = _tPktTrans.nGapCount;
      data.GapLossRate = _tPktTrans.nGapLossRate;
      data.GapLength = _tPktTrans.nGapLengthPkts;
      data.IFrameInterArrivalJitter = _tVideoJitter.nIFrameInterArrivalJitterMs;
      data.PPDV = _tJitter.nPPDVMs;      

      metrics->set(data);

      return 0;
    }    
    
    return 0;
  }

  virtual int getVoiceMetrics(VoiceVQualResultsHandler metrics) {
    return -1;
  }
  virtual int getAudioHDMetrics(AudioHDVQualResultsHandler metrics) {
    return -1;
  }

  virtual VQMediaType getMediaType() const {
    return VQ_VIDEO;
  }

protected:

  int init() {

    if(!mInterface) return -1;

    VQInterfaceImpl* interfaceImpl=static_cast<VQInterfaceImpl*>(mInterface.get());

    InterfaceType interfaceHandle = interfaceImpl->getInterface();
    if(!interfaceHandle) return -1;

    vqmon_interface_t* pInterface = VQMON_INTERFACE_HANDLETOPTR(interfaceHandle);
    if(!pInterface) return -1;

    vqmon_streamtype_t _eStreamType = vqmonStreamTypeVideo;

    VQmonStreamCreate(&mStreamHandle, interfaceHandle, _eStreamType);
    if (!VQMON_STREAM_ISVALID(mStreamHandle)) {
      return -1;
    }

    vqmon_streamcfg_codec_t _tCodecCfg;
    memset(&_tCodecCfg,0,sizeof(_tCodecCfg));

    _tCodecCfg.idCODECType = getVQmonCodec(mCodec,mPayload,mBitRate);
    interfaceImpl->setVideoCodec(_tCodecCfg.idCODECType);
    _tCodecCfg.bActivityDetectAvail = 0;
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

    _tEptDesc.eType = vqmonStreamTypeVideo;

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

    vqmon_streamcfg_videoframeres_t _tVideoFrameres;
    memset(&_tVideoFrameres,0,sizeof(_tVideoFrameres));

    switch(mCodec) {
       case VH264_CTS_720P:
       case VH264_TBG_720P:
          _tVideoFrameres.nFrameWidth  = 1280;
          _tVideoFrameres.nFrameHeight = 720;
          break;
       case VH264_CTS_1080P:
       case VH264_TBG_1080P:
          _tVideoFrameres.nFrameWidth  = 1920;
          _tVideoFrameres.nFrameHeight = 1080;
          break;
       case VH264_TBG_CIF:
          _tVideoFrameres.nFrameWidth  = 512;
          _tVideoFrameres.nFrameHeight = 288;
          break;
       case VH264_TBG_XGA:
          _tVideoFrameres.nFrameWidth  = 768;
          _tVideoFrameres.nFrameHeight = 448;
          break;
       default:
          _tVideoFrameres.nFrameWidth  = 1280;
          _tVideoFrameres.nFrameHeight = 720;
          break;
    }

    res = VQmonStreamConfigSet(
                     mStreamHandle,
                     VQMON_STREAMCFGBLOCKID_VIDEOFRAMERES,
                     sizeof(vqmon_streamcfg_videoframeres_t),
                     &_tVideoFrameres);

    if(res != VQMON_ESUCCESS) {
      return -1;
    }

    vqmon_streamcfg_videoframerate_t _tVideoFramerate;
    memset(&_tVideoFramerate,0,sizeof(_tVideoFramerate));

    _tVideoFramerate.nFrameRate = 30000;
    _tVideoFramerate.bInterlaced = false;

    res = VQmonStreamConfigSet(
                     mStreamHandle,
                     VQMON_STREAMCFGBLOCKID_VIDEOFRAMERATE,
                     sizeof(vqmon_streamcfg_videoframerate_t),
                     &_tVideoFramerate);

    if(res != VQMON_ESUCCESS) {
      return -1;
    }
    
    return 0;
  }
};

////////////////////////////////////////////////////////////////////////////////

