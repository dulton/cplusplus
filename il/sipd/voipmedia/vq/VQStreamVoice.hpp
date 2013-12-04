/// @file
/// @brief VQStreamVoice class
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

class VQMonSessionVoice : public VQMonSession {

public:

  explicit VQMonSessionVoice(VQInterfaceHandler interface,
			     CodecType codec, PayloadType payload, 
			     int bitRate, uint32_t ssrc, int jbSize) : VQMonSession(interface,codec, payload, bitRate, ssrc, jbSize)
  {
    if(init()==-1) {
      mInitFailed=true;
    }
  }

  virtual ~VQMonSessionVoice() {
    destroy();
  }
  
  /**
   * Get current VQ values for the stream.
   * Return -1 if the stream has not been fed properly.
   */
  virtual int getVoiceMetrics(VoiceVQualResultsHandler metrics) {
    
    if(metrics && mPacketCounter>=10) {
      
      vqmon_result_t result;
      
      vqmon_streammetrics_voicequal_t _tVoiceQuality;
      tcmyU32                         _nSize;
      
      _nSize = sizeof(vqmon_streammetrics_voicequal_t);
      memset(&_tVoiceQuality, 0, _nSize);
      result = VQmonStreamMetricsGet(mStreamHandle,
				     VQMON_STREAMMETRICBLOCKID_VOICEQUAL,
				     &_nSize,
				     &_tVoiceQuality);
      
      if (result != VQMON_ESUCCESS)
         return -1;

      vqmon_streammetrics_jitter_t _tVoiceJitter;

      _nSize = sizeof(vqmon_streammetrics_jitter_t);
      memset(&_tVoiceJitter, 0, _nSize);
      result = VQmonStreamMetricsGet(mStreamHandle,
                      VQMON_STREAMMETRICBLOCKID_JITTER,
                      &_nSize,
                      &_tVoiceJitter);

      if (result != VQMON_ESUCCESS)
         return -1;

      vqmon_streammetrics_pkttrans_t _tVoicePkttans;

      _nSize = sizeof(vqmon_streammetrics_pkttrans_t);
      memset(&_tVoicePkttans, 0, _nSize);
      result = VQmonStreamMetricsGet(mStreamHandle,
                     VQMON_STREAMMETRICBLOCKID_PKTTRANS,
                     &_nSize,
                     &_tVoicePkttans);
      
      if (result != VQMON_ESUCCESS)
         return -1;

      VoiceVQualResultsRawData::VoiceVQData data;

      data.MOS_LQ = _tVoiceQuality.nMOS_LQ; 
      data.MOS_CQ = _tVoiceQuality.nMOS_CQ; 
      data.R_LQ   = _tVoiceQuality.nR_LQ; 
      data.R_CQ   = _tVoiceQuality.nR_CQ;
      data.PPDV   = _tVoiceJitter.nPPDVMs;
      data.PktsRcvd = _tVoicePkttans.nPktsRcvd;
      data.PktsLost = _tVoicePkttans.nPktsLost;
      data.PktsDiscarded = _tVoicePkttans.nPktsDiscarded;

      metrics->set(data);

      return 0;
    }

    return 0;
  }

  virtual int getVideoMetrics(VideoVQualResultsHandler metrics) {
    return -1;
  }

  virtual int getAudioHDMetrics(AudioHDVQualResultsHandler metrics) {
    return -1;
  }

  virtual VQMediaType getMediaType() const {
    return VQ_VOICE;
  }

protected:

  int init() {

    if(!mInterface) return -1;

    VQInterfaceImpl* interfaceImpl=static_cast<VQInterfaceImpl*>(mInterface.get());

    
    InterfaceType interfaceHandle = interfaceImpl->getInterface();
    
    if(!interfaceHandle) return -1;

    vqmon_interface_t* pInterface = VQMON_INTERFACE_HANDLETOPTR(interfaceHandle);
    if(!pInterface) return -1;

    vqmon_streamtype_t _eStreamType = vqmonStreamTypeVoice;
    vqmon_streamcfg_codec_t _tCodecCfg;
    vqmon_streamcfg_netepdesc_t _tEptDesc;
    vqmon_streamcfg_transprotoext_t _tTransProto;
    vqmon_stream_commoninfo_t   *_pStream=0;
    vqmon_streamcfg_voicegencfg_t _mosScale;
    
    
    memset(&_tEptDesc,0,sizeof(_tEptDesc));
    memset(&_tTransProto,0,sizeof(_tTransProto));
    memset(&_mosScale,0,sizeof(_mosScale));
    
    _tEptDesc.eType = vqmonStreamTypeVoice;
    _tCodecCfg.idCODECType = getVQmonCodec(mCodec,mPayload,mBitRate);
    interfaceImpl->setVoiceCodec(_tCodecCfg.idCODECType);
    _tCodecCfg.bActivityDetectAvail = (mCodec == G729B) || (mCodec == G729AB);
    if(mBitRate<1) _tCodecCfg.nCurrentRate = -1;
    else _tCodecCfg.nCurrentRate = mBitRate;
    
    _mosScale.eMOSScale = vqmonVoiceMOSScaleACR;
    
    VQmonStreamCreate(&mStreamHandle, interfaceHandle, _eStreamType);
    
    if (!VQMON_STREAM_ISVALID(mStreamHandle)) {
      return -1;
    }
      
    _pStream = (vqmon_stream_commoninfo_t *) mStreamHandle;
    
    /*
     * Set the CODEC type here.
     */
    vqmon_result_t res = VQmonStreamConfigSet(
					      mStreamHandle,
					      VQMON_STREAMCFGBLOCKID_CODECINFO,
					      sizeof(_tCodecCfg),
					      &_tCodecCfg
					      );
    if(res != VQMON_ESUCCESS) {
      return -1;
    }
    
    /*
     * Set the MOS scale
     */
    res = VQmonStreamConfigSet(
			       mStreamHandle,
			       VQMON_STREAMCFGBLOCKID_VOICEGENCFG,
			       sizeof(_mosScale),
			       &_mosScale
			       );
    if(res != VQMON_ESUCCESS) {
      return -1;
    }
    
    /*
     * Set the endpoint description.
     */
    
    // Fake addresses, we do not need real ones:
    
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
    
    _tEptDesc.eTransportProtos = (vqmon_streamtransproto_t)(vqmonStreamTransProtoUDP|vqmonStreamTransProtoRTP);
    
    res = VQmonStreamConfigSet(
			       mStreamHandle,
			       VQMON_STREAMCFGBLOCKID_NETEPDESC,
			       sizeof(vqmon_streamcfg_netepdesc_t),
			       &_tEptDesc
			       );
    if(res != VQMON_ESUCCESS) {
      return -1;
    }
    
    _tTransProto.eTransportProto = vqmonStreamTransProtoRTP;
    _tTransProto.proto.rtp.nSSRC = mSSRC;
    
    VQmonStreamConfigSet(
			 mStreamHandle,
			 VQMON_STREAMCFGBLOCKID_TRANSPORTINFO,
			 sizeof(_tTransProto),
			 &_tTransProto
			 );
    
    if(res != VQMON_ESUCCESS) {
      return -1;
    }
    
    if(mJBSize>0) {
      
      vqmon_streamcfg_voicejitterbuffer_t jitterBufferCfg;
      memset(&jitterBufferCfg,0,sizeof(jitterBufferCfg));
      
      jitterBufferCfg.eMode=vqmonJitterBufferModeTelchemyFixed;
      
      jitterBufferCfg.nNomDelay = (mJBSize*60)/200;
      if(jitterBufferCfg.nNomDelay>5) jitterBufferCfg.nMinDelay = 5;
      else jitterBufferCfg.nMinDelay = jitterBufferCfg.nNomDelay;
      jitterBufferCfg.nMaxDelay = mJBSize;
      
      VQmonSAVoiceStreamJitterBufferCfg(mStreamHandle,&jitterBufferCfg);
      
      if(res != VQMON_ESUCCESS) {
	return -1;
      }
    }
    
    /////////////////////////////////////////////////////////////////////
    // temporal re-use of vqmonCODECVoiceGIPSiPCMwb for G711.1(WB);
    
    vqmon_codec_bitrate_map_t *pBitrateMap = g_VocoderBitrateMap;
    while( pBitrateMap->eCODEC != vqmonCODECVoiceMax ) {
      if( pBitrateMap->eCODEC == vqmonCODECVoiceGIPSiPCMwb )
	{
	  pBitrateMap->nBitrate = 96000;
	  break;
	}
      pBitrateMap++;
    }
    
    vqmon_codec_bitrate_t *pBitrate = g_VocoderBitrateTable;
    while( pBitrate->eCODEC != vqmonCODECVoiceMax ) {
      if( pBitrate->eCODEC == vqmonCODECVoiceGIPSiPCMwb )
	{
	  pBitrate->aValidRates[0] = 2;
	  pBitrate->aValidRates[1] = 0;
	  break;
	}
      pBitrate++;
    }
    vqmon_codec_voice_extprops_t codecProp, codecPropExt;
    tcmyU32  nCodecPropSz = sizeof(vqmon_codec_voice_extprops_t);
    VQmonCODECPropertiesGet(vqmonCODECVoiceGIPSiPCMwb,  VQMON_CODECPROPBLOCKID_EXTPROPS, &codecPropExt, &nCodecPropSz, &codecProp );
    codecProp.nBaseIe = 0;
    codecProp.nFrameSize = 48;
    VQmonCODECPropertiesSet(vqmonCODECVoiceGIPSiPCMwb, VQMON_CODECPROPBLOCKID_EXTPROPS, &codecPropExt, nCodecPropSz, &codecProp );

    return 0;
  }
};

////////////////////////////////////////////////////////////////////////////////

