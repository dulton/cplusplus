/// @file
/// @brief SIP User Agent Radvision-based (UA) object
/// SDP Media part
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <sstream>
#include <string>

#include <boost/bind.hpp>

#include "SipUtils.h"
#include "RvUserAgent.h"

#include "SipEngine.h"
#include "MediaCodecs.h"

USING_SIP_NS;
USING_RVSIP_NS;
USING_RV_SIP_ENGINE_NAMESPACE;

//////////////////////// Media Interface //////////////////////////////////////

/////////////////////////// SDP helper methods ////////////////////////////////

#define SDP_BUFF_SIZE (1025) //max SDP buf size
#define SDP_CONTENT_TYPE "application/sdp"

#define  SDP_VERSION       "0"
#define  SDP_USERNAME      (mChannel->getLocalUserName().c_str())
#define  SESSION_NAME      "Spirent TestCenter"

#define  STR_RTP_AVP_PROTOCOL       "RTP/AVP"

RvSdpStatus RvUserAgent::SDP_InitSdpMsg(RvSdpMsg* sdpMsg) {

    const ACE_Time_Value now(ACE_OS::gettimeofday());

    unsigned long sessionId = now.msec();
    char ssid[33];
    sprintf(ssid, "%lu", sessionId);

    unsigned long timestamp = 0;
  if(mConfig.ProtocolProfile.IncludeSdpStartTs ) {
        timestamp = now.msec();
    }

    RvSdpStatus rv = RV_SDPSTATUS_OK;

    rv = rvSdpMsgSetVersionN(sdpMsg, SDP_VERSION);
  if(rv>=0) {
    const RvSipTransportAddr& addr = mChannel->getLocalAddress();
    RvSdpAddrType addrType = (addr.eAddrType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP6) ? RV_SDPADDRTYPE_IP6 : RV_SDPADDRTYPE_IP4;
    char strIP[sizeof(addr.strIP)+1];
    if(addr.eAddrType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP6) adjustIPv6ForSdp(strIP,addr.strIP);
    else strcpy(strIP,addr.strIP);
    rv = rvSdpMsgSetOrigin (sdpMsg, SDP_USERNAME, ssid, "1", RV_SDPNETTYPE_IN, addrType, strIP);
    if(rv >= 0) {
      rv = rvSdpMsgSetSessionName (sdpMsg, SESSION_NAME);
      if(rv >= 0) {
                rvSdpMsgAddSessionTime(sdpMsg, timestamp, 0);
            }
        }
    }

    return rv;
}

const char* RvUserAgent::SDP_GetSdpAudioEncodingName(Sip_1_port_server::EnumSipAudioCodec codec, int &payloadType, int &samplingRate, unsigned char dynPlType) {

  switch(codec) {
        case sip_1_port_server::EnumSipAudioCodec_G_711_A_LAW:
            payloadType = 8;
            samplingRate = 8000;
            return "PCMA";
        case sip_1_port_server::EnumSipAudioCodec_G_726_32:
            payloadType = static_cast<int>(dynPlType);
            samplingRate = 8000;
            return "G726-32";
        case sip_1_port_server::EnumSipAudioCodec_G_723_1:
        case sip_1_port_server::EnumSipAudioCodec_G_723_1_5_3:
            payloadType = 4;
            samplingRate = 8000;
            return "G723";
        case sip_1_port_server::EnumSipAudioCodec_G_729:
        case sip_1_port_server::EnumSipAudioCodec_G_729_A:
        case sip_1_port_server::EnumSipAudioCodec_G_729_B:
        case sip_1_port_server::EnumSipAudioCodec_G_729_AB:
            payloadType = 18;
            samplingRate = 8000;
            return "G729";
        case sip_1_port_server::EnumSipAudioCodec_G_711_1_MU_LAW_96:
            payloadType = static_cast<int>(dynPlType);
            samplingRate = 16000;
            return "PCMU-WB";
        case sip_1_port_server::EnumSipAudioCodec_G_711_1_A_LAW_96:
            payloadType = static_cast<int>(dynPlType);
            samplingRate = 16000;
            return "PCMA-WB";

        case sip_1_port_server::EnumSipAudioCodec_AMRNB_0_OA_475:
        case sip_1_port_server::EnumSipAudioCodec_AMRNB_1_OA_515:
        case sip_1_port_server::EnumSipAudioCodec_AMRNB_2_OA_590:
        case sip_1_port_server::EnumSipAudioCodec_AMRNB_3_OA_670:
        case sip_1_port_server::EnumSipAudioCodec_AMRNB_4_OA_740:
        case sip_1_port_server::EnumSipAudioCodec_AMRNB_5_OA_795:
        case sip_1_port_server::EnumSipAudioCodec_AMRNB_6_OA_102:
        case sip_1_port_server::EnumSipAudioCodec_AMRNB_7_OA_122:

        case sip_1_port_server::EnumSipAudioCodec_AMRNB_0_BE_475:
        case sip_1_port_server::EnumSipAudioCodec_AMRNB_1_BE_515:
        case sip_1_port_server::EnumSipAudioCodec_AMRNB_2_BE_590:
        case sip_1_port_server::EnumSipAudioCodec_AMRNB_3_BE_670:
        case sip_1_port_server::EnumSipAudioCodec_AMRNB_4_BE_740:
        case sip_1_port_server::EnumSipAudioCodec_AMRNB_5_BE_795:
        case sip_1_port_server::EnumSipAudioCodec_AMRNB_6_BE_102:
        case sip_1_port_server::EnumSipAudioCodec_AMRNB_7_BE_122:
            payloadType = static_cast<int>(dynPlType);
            samplingRate = 8000;
            return "AMR";

        case sip_1_port_server::EnumSipAudioCodec_AMRWB_0_OA_660:
        case sip_1_port_server::EnumSipAudioCodec_AMRWB_1_OA_885:
        case sip_1_port_server::EnumSipAudioCodec_AMRWB_2_OA_1265:
        case sip_1_port_server::EnumSipAudioCodec_AMRWB_3_OA_1425:
        case sip_1_port_server::EnumSipAudioCodec_AMRWB_4_OA_1585:
        case sip_1_port_server::EnumSipAudioCodec_AMRWB_5_OA_1825:
        case sip_1_port_server::EnumSipAudioCodec_AMRWB_6_OA_1985:
        case sip_1_port_server::EnumSipAudioCodec_AMRWB_7_OA_2305:
        case sip_1_port_server::EnumSipAudioCodec_AMRWB_8_OA_2385:

        case sip_1_port_server::EnumSipAudioCodec_AMRWB_0_BE_660:
        case sip_1_port_server::EnumSipAudioCodec_AMRWB_1_BE_885:
        case sip_1_port_server::EnumSipAudioCodec_AMRWB_2_BE_1265:
        case sip_1_port_server::EnumSipAudioCodec_AMRWB_3_BE_1425:
        case sip_1_port_server::EnumSipAudioCodec_AMRWB_4_BE_1585:
        case sip_1_port_server::EnumSipAudioCodec_AMRWB_5_BE_1825:
        case sip_1_port_server::EnumSipAudioCodec_AMRWB_6_BE_1985:
        case sip_1_port_server::EnumSipAudioCodec_AMRWB_7_BE_2305:
        case sip_1_port_server::EnumSipAudioCodec_AMRWB_8_BE_2385:

            payloadType = static_cast<int>(dynPlType);
            samplingRate = 16000;
            return "AMR-WB";

        case sip_1_port_server::EnumSipAudioCodec_G_711:
        default:
            payloadType = 0;
            samplingRate = 8000;
            return "PCMU";
    };
    return 0;
}

const char* RvUserAgent::SDP_GetSdpVideoEncodingName(Sip_1_port_server::EnumSipVideoCodec codec, int &payloadType, int &samplingRate) {

  switch(codec) {
  case sip_1_port_server::EnumSipVideoCodec_H_264:
  case sip_1_port_server::EnumSipVideoCodec_H_264_720p:
  case sip_1_port_server::EnumSipVideoCodec_H_264_1080p:
  case sip_1_port_server::EnumSipVideoCodec_H_264_512_288:
  case sip_1_port_server::EnumSipVideoCodec_H_264_768_448:
    if(mConfig.ProtocolProfile.CallType == sip_1_port_server::EnumSipCallType_AUDIO_VIDEO &&
        mConfig.ProtocolProfile.RtpTrafficType == sip_1_port_server::EnumRtpTrafficType_SIMULATED_RTP)
        payloadType = 96;
    else
      payloadType = mConfig.ProtocolProfile.VideoPayloadType;
    samplingRate = 90000;
    return "H264";
  case sip_1_port_server::EnumSipVideoCodec_MP4V_ES:
    payloadType = 98;
    samplingRate = 90000;
    return "MP4V-ES";
  case sip_1_port_server::EnumSipVideoCodec_H_263:
    payloadType = 34;
    samplingRate = 90000;
  default:
    return "H263";
  };
}

RvSdpStatus RvUserAgent::SDP_FillSdpMsg(RvSdpMsg* sdpMsg) {

  RvSdpStatus rv = RV_SDPSTATUS_OK;

  if(!mChannel) return rv;
  if(!mMediaChannel) return rv;
  if(!mVoipMediaManager) return rv;

  {
    //Connection field:
    const RvSipTransportAddr& addr = mChannel->getLocalAddress();
    RvSdpAddrType addrType = (addr.eAddrType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP6) ? RV_SDPADDRTYPE_IP6 : RV_SDPADDRTYPE_IP4;
    char strIP[sizeof(addr.strIP)+1];
    if(addr.eAddrType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP6) adjustIPv6ForSdp(strIP,addr.strIP);
    else strcpy(strIP,addr.strIP);
    rvSdpMsgSetConnection(sdpMsg, RV_SDPNETTYPE_IN, addrType, strIP);
  }

  if(mAudioCall ||mChannel->isSigOnly()) {

      RvSdpMediaDescr *sdpMediaDescr = 0;

      sdpMediaDescr = rvSdpMsgAddMediaDescr(sdpMsg, RV_SDPMEDIATYPE_AUDIO,
              mMediaChannel->getLocalTransportPort(MEDIA_NS::AUDIO_STREAM),
              RV_SDPPROTOCOL_RTP);

      if(!sdpMediaDescr) {
          return RV_SDPSTATUS_SET_FAILED;
      }

      Sip_1_port_server::EnumSipAudioCodec codec = sip_1_port_server::EnumSipAudioCodec_G_711;
      Sip_1_port_server::EnumRtpTrafficType rtpTrafficType = sip_1_port_server::EnumRtpTrafficType_SIMULATED_RTP;

      if(!mChannel->isSigOnly())
      {
          codec = static_cast<Sip_1_port_server::EnumSipAudioCodec>(mConfig.ProtocolProfile.AudioCodec);
          rtpTrafficType =  static_cast<Sip_1_port_server::EnumRtpTrafficType> (mConfig.ProtocolProfile.RtpTrafficType);
      }
      int payloadType=0;
      int samplingRate=0;
      const char* encodingName = SDP_GetSdpAudioEncodingName(codec,payloadType,samplingRate, mConfig.ProtocolProfile.AudioPayloadType);

      rvSdpMediaDescrAddPayloadNumber(sdpMediaDescr, (int)payloadType);

      rvSdpMediaDescrAddRtpMap(sdpMediaDescr, payloadType, encodingName, samplingRate);

        switch (codec)
        {
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_0_OA_475:
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_1_OA_515:
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_2_OA_590:
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_3_OA_670:
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_4_OA_740:
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_5_OA_795:
            {
                   std::ostringstream oss;
                   oss << payloadType << " mode-set="<< codec - sip_1_port_server::EnumSipAudioCodec_AMRNB_0_OA_475<<";" << " octet-align=1";
                   std::string strMod(oss.str());
                   rvSdpMediaDescrAddAttr(sdpMediaDescr, "fmtp", strMod.c_str());
                   if( rtpTrafficType == sip_1_port_server::EnumRtpTrafficType_ENCODED_RTP)
                       rvSdpMediaDescrAddAttr(sdpMediaDescr, "maxptime", "20");
                   else
                   {
                       rvSdpMediaDescrAddAttr(sdpMediaDescr, "ptime", "40");
                       rvSdpMediaDescrAddAttr(sdpMediaDescr, "maxptime", "40");
                   }
                   oss.clear();
            }
             break;

            case sip_1_port_server::EnumSipAudioCodec_AMRNB_6_OA_102:
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_7_OA_122:
            {
                std::ostringstream oss;
                oss << payloadType << " mode-set="<< codec - sip_1_port_server::EnumSipAudioCodec_AMRNB_0_OA_475<<";" << " octet-align=1";
                std::string strMod(oss.str());
                rvSdpMediaDescrAddAttr(sdpMediaDescr, "fmtp", strMod.c_str());
                rvSdpMediaDescrAddAttr(sdpMediaDescr, "maxptime", "20");
                oss.clear();
            }
                break;

            case sip_1_port_server::EnumSipAudioCodec_AMRWB_0_OA_660:
            {
                  std::ostringstream oss;
                  oss << payloadType << " mode-set="<< codec - sip_1_port_server::EnumSipAudioCodec_AMRWB_0_OA_660<<";" << " octet-align=1";
                  std::string strMod(oss.str());
                  rvSdpMediaDescrAddAttr(sdpMediaDescr, "fmtp", strMod.c_str());
                  if( rtpTrafficType == sip_1_port_server::EnumRtpTrafficType_ENCODED_RTP)
                      rvSdpMediaDescrAddAttr(sdpMediaDescr, "maxptime", "20");
                  else
                  {
                      rvSdpMediaDescrAddAttr(sdpMediaDescr, "ptime", "40");
                      rvSdpMediaDescrAddAttr(sdpMediaDescr, "maxptime", "40");
                  }
            }
             break;

            case sip_1_port_server::EnumSipAudioCodec_AMRWB_1_OA_885:
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_2_OA_1265:
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_3_OA_1425:
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_4_OA_1585:
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_5_OA_1825:
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_6_OA_1985:
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_7_OA_2305:
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_8_OA_2385:
            {
                std::ostringstream oss;
                oss << payloadType << " mode-set="<< codec - sip_1_port_server::EnumSipAudioCodec_AMRWB_0_OA_660<<";" << " octet-align=1";
                std::string strMod(oss.str());
                rvSdpMediaDescrAddAttr(sdpMediaDescr, "fmtp", strMod.c_str());
                rvSdpMediaDescrAddAttr(sdpMediaDescr, "maxptime","20");
            }
                break;

            case sip_1_port_server::EnumSipAudioCodec_AMRNB_0_BE_475:
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_1_BE_515:
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_2_BE_590:
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_3_BE_670:
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_4_BE_740:
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_5_BE_795:
            {
                  std::ostringstream oss;
                  oss << payloadType << " mode-set="<< codec - sip_1_port_server::EnumSipAudioCodec_AMRNB_0_BE_475<<";" << " octet-align=0";
                  std::string strMod(oss.str());
                  rvSdpMediaDescrAddAttr(sdpMediaDescr, "fmtp", strMod.c_str());
                  if( rtpTrafficType == sip_1_port_server::EnumRtpTrafficType_ENCODED_RTP)
                      rvSdpMediaDescrAddAttr(sdpMediaDescr, "maxptime", "20");
                  else
                  {
                      rvSdpMediaDescrAddAttr(sdpMediaDescr, "ptime", "40");
                      rvSdpMediaDescrAddAttr(sdpMediaDescr, "maxptime", "40");
                  }

            }
            break;

            case sip_1_port_server::EnumSipAudioCodec_AMRNB_6_BE_102:
            case sip_1_port_server::EnumSipAudioCodec_AMRNB_7_BE_122:
            {
                std::ostringstream oss;
                oss << payloadType << " mode-set="<< codec - sip_1_port_server::EnumSipAudioCodec_AMRNB_0_BE_475<<";" << " octet-align=0";
                std::string strMod(oss.str());
                rvSdpMediaDescrAddAttr(sdpMediaDescr, "fmtp", strMod.c_str());
                rvSdpMediaDescrAddAttr(sdpMediaDescr, "maxptime", "20");
            }
            break;

            case sip_1_port_server::EnumSipAudioCodec_AMRWB_0_BE_660:
            {
                  std::ostringstream oss;
                  oss << payloadType << " mode-set="<< codec - sip_1_port_server::EnumSipAudioCodec_AMRWB_0_BE_660<<";" << " octet-align=0";
                  std::string strMod(oss.str());
                  rvSdpMediaDescrAddAttr(sdpMediaDescr, "fmtp", strMod.c_str());
                  if( rtpTrafficType == sip_1_port_server::EnumRtpTrafficType_ENCODED_RTP)
                      rvSdpMediaDescrAddAttr(sdpMediaDescr, "maxptime", "20");
                  else
                  {
                      rvSdpMediaDescrAddAttr(sdpMediaDescr, "ptime", "40");
                      rvSdpMediaDescrAddAttr(sdpMediaDescr, "maxptime", "40");
                  }

            }
            break;

            case sip_1_port_server::EnumSipAudioCodec_AMRWB_1_BE_885:
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_2_BE_1265:
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_3_BE_1425:
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_4_BE_1585:
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_5_BE_1825:
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_6_BE_1985:
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_7_BE_2305:
            case sip_1_port_server::EnumSipAudioCodec_AMRWB_8_BE_2385:
            {
                std::ostringstream oss;
                oss << payloadType << " mode-set="<< codec - sip_1_port_server::EnumSipAudioCodec_AMRWB_0_BE_660<<";" << " octet-align=0";
                std::string strMod(oss.str());
                rvSdpMediaDescrAddAttr(sdpMediaDescr, "fmtp", strMod.c_str());
                rvSdpMediaDescrAddAttr(sdpMediaDescr, "maxptime","20");

            }
            break;

            case sip_1_port_server::EnumSipAudioCodec_G_711_1_MU_LAW_96:
            case sip_1_port_server::EnumSipAudioCodec_G_711_1_A_LAW_96:
            {
                std::ostringstream oss;
                oss << payloadType << " mode-set=4";
                std::string strMod(oss.str());
                rvSdpMediaDescrAddAttr(sdpMediaDescr, "fmtp", strMod.c_str());
            }
                break;
          default: break;
        }

        rvSdpMediaDescrAddAttr(sdpMediaDescr, "rtpmap", "100 telephone-event/8000");
        rvSdpMediaDescrAddAttr(sdpMediaDescr, "ptime", "20");
        rvSdpMediaDescrAddAttr(sdpMediaDescr, "fmtp", "100 0-15");

      if(mAudioHDCall) {
          char  valuer[1024] = "";
          char  valuef[1024] = "";
          char* pvalue = valuer;

          sprintf( pvalue, "%d ", AAC_LD_PT);
          pvalue += strlen(pvalue);
          strcpy(pvalue,"mpeg4-generic/");
          pvalue += strlen(pvalue);
          sprintf( pvalue, "%d ", 48000);
          rvSdpMediaDescrAddAttr(sdpMediaDescr, "rtpmap",valuer);

          pvalue = valuef;
          sprintf( pvalue, "%d ", AAC_LD_PT);
          pvalue += strlen(pvalue);
          strcpy(pvalue,"profile-level-id=16;streamtype=5;config=B98C00;mode=AAC-hbr;sizeLength=13;indexLength=3;indexDeltaLength=3;constantDuration=480");
          rvSdpMediaDescrAddAttr(sdpMediaDescr, "fmtp",valuef);

      }
  }else if(mAudioHDCall){
      RvSdpMediaDescr *sdpMediaDescr = 0;
      char  valuer[1024] = "";
      char  valuef[1024] = "";
      char* pvalue = valuer;

      sdpMediaDescr = rvSdpMsgAddMediaDescr(sdpMsg, RV_SDPMEDIATYPE_AUDIO,
              mMediaChannel->getLocalTransportPort(MEDIA_NS::AUDIOHD_STREAM),
              RV_SDPPROTOCOL_RTP);

      if(!sdpMediaDescr) {
          return RV_SDPSTATUS_SET_FAILED;
      }

      rvSdpMediaDescrAddBandwidth(sdpMediaDescr,"TIAS", 64000);

      rvSdpMediaDescrAddPayloadNumber(sdpMediaDescr, AAC_LD_PT);

      sprintf( pvalue, "%d ", AAC_LD_PT);
      pvalue += strlen(pvalue);
      strcpy(pvalue,"mpeg4-generic/");
      pvalue += strlen(pvalue);
      sprintf( pvalue, "%d ", 48000);
      rvSdpMediaDescrAddAttr(sdpMediaDescr, "rtpmap",valuer);

      pvalue = valuef;
      sprintf( pvalue, "%d ", AAC_LD_PT);
      pvalue += strlen(pvalue);
      strcpy(pvalue,"profile-level-id=16;streamtype=5;config=B98C00;mode=AAC-hbr;sizeLength=13;indexLength=3;indexDeltaLength=3;constantDuration=480");
      rvSdpMediaDescrAddAttr(sdpMediaDescr, "fmtp",valuef);

  }

  if(mVideoCall) {

    RvSdpMediaDescr *sdpMediaDescr = 0;

    sdpMediaDescr = rvSdpMsgAddMediaDescr(sdpMsg, RV_SDPMEDIATYPE_VIDEO,
					  mMediaChannel->getLocalTransportPort(MEDIA_NS::VIDEO_STREAM),
					  RV_SDPPROTOCOL_RTP);

    if(!sdpMediaDescr) {
      return RV_SDPSTATUS_SET_FAILED;
    }

    Sip_1_port_server::EnumSipVideoCodec codec = static_cast<Sip_1_port_server::EnumSipVideoCodec>(mConfig.ProtocolProfile.VideoCodec);
    int payloadType=0;
    int samplingRate=0;
    const char* encodingName = SDP_GetSdpVideoEncodingName(codec,payloadType,samplingRate);

    rvSdpMediaDescrAddPayloadNumber(sdpMediaDescr, (int)payloadType);

    rvSdpMediaDescrAddRtpMap(sdpMediaDescr, payloadType, encodingName, samplingRate);

    if( codec == sip_1_port_server::EnumSipVideoCodec_H_263 ) {
      char  fmtp_value[1000] = "";
      char* pDst = fmtp_value;

      sprintf( pDst, "%d ", (int)payloadType );
      pDst += strlen(pDst);
      strcpy(pDst,"QCIF=2");

      rvSdpMediaDescrAddAttr(sdpMediaDescr,"fmtp",fmtp_value);
    } else if( codec == sip_1_port_server::EnumSipVideoCodec_MP4V_ES ) {
      char  fmtp_value[1000] = "";
      char* pDst = fmtp_value;

      sprintf( pDst, "%d ", (int)payloadType );
      pDst += strlen(pDst);
      strcpy(pDst,"profile-level-id=3;framesize=VGA");

      rvSdpMediaDescrAddAttr(sdpMediaDescr,"fmtp",fmtp_value);
      rvSdpMediaDescrAddAttr(sdpMediaDescr,"framerate","15");

      rvSdpMsgAddBandwidth(sdpMsg,"AS",2048);

    } else if( codec == sip_1_port_server::EnumSipVideoCodec_H_264 ) {
      rvSdpMediaDescrAddBandwidth(sdpMediaDescr,"TIAS",2250000);
      char  fmtp_value[1024] = "";
      char* pDst = fmtp_value;

      sprintf( pDst, "%d ", (int)payloadType );
      pDst += strlen(pDst);
      strcpy(pDst,"profile-level-id=4D0028;packetization-mode=1");

      rvSdpMediaDescrAddAttr(sdpMediaDescr,"fmtp",fmtp_value);
    }else if( codec == sip_1_port_server::EnumSipVideoCodec_H_264_720p ) {
       if(mConfig.ProtocolProfile.TipDeviceType <= sip_1_port_server::EnumTipDevice_CTS3200) {
          rvSdpMediaDescrAddBandwidth(sdpMediaDescr,"TIAS",2250000);
          char  fmtp_value[1024] = "";
          char* pDst = fmtp_value;

          sprintf( pDst, "%d ", (int)payloadType );
          pDst += strlen(pDst);
          strcpy(pDst,"profile-level-id=4D0028;packetization-mode=1");
          
          rvSdpMediaDescrAddAttr(sdpMediaDescr,"fmtp",fmtp_value);
       }else {
          rvSdpMediaDescrAddBandwidth(sdpMediaDescr,"TIAS",1152000);
          char  fmtp_value[1024] = "";
          char* pDst = fmtp_value;
          
          sprintf( pDst, "%d ", (int)payloadType );
          pDst += strlen(pDst);
          strcpy(pDst,"profile-level-id=428014;packetization-mode=1");
          
          rvSdpMediaDescrAddAttr(sdpMediaDescr,"fmtp",fmtp_value); 
       }
     }else if( codec == sip_1_port_server::EnumSipVideoCodec_H_264_1080p ) {
       if(mConfig.ProtocolProfile.TipDeviceType <= sip_1_port_server::EnumTipDevice_CTS3200) {
          rvSdpMediaDescrAddBandwidth(sdpMediaDescr,"TIAS",4000000);
          char  fmtp_value[1024] = "";
          char* pDst = fmtp_value;

          sprintf( pDst, "%d ", (int)payloadType );
          pDst += strlen(pDst);
          strcpy(pDst,"profile-level-id=4D0028;packetization-mode=1");
          
          rvSdpMediaDescrAddAttr(sdpMediaDescr,"fmtp",fmtp_value);
       }else {
          rvSdpMediaDescrAddBandwidth(sdpMediaDescr,"TIAS",6000000);
          char  fmtp_value[1024] = "";
          char* pDst = fmtp_value;
          
          sprintf( pDst, "%d ", (int)payloadType );
          pDst += strlen(pDst);
          strcpy(pDst,"profile-level-id=428014;packetization-mode=1");
          
          rvSdpMediaDescrAddAttr(sdpMediaDescr,"fmtp",fmtp_value); 
       }
     }else if( codec == sip_1_port_server::EnumSipVideoCodec_H_264_512_288 ) {
        rvSdpMediaDescrAddBandwidth(sdpMediaDescr,"TIAS",128000);
        char  fmtp_value[1024] = "";
        char* pDst = fmtp_value;
        
        sprintf( pDst, "%d ", (int)payloadType );
        pDst += strlen(pDst);
        strcpy(pDst,"profile-level-id=428014;packetization-mode=1");
        
        rvSdpMediaDescrAddAttr(sdpMediaDescr,"fmtp",fmtp_value); 
     }else if( codec == sip_1_port_server::EnumSipVideoCodec_H_264_768_448 ) {
        rvSdpMediaDescrAddBandwidth(sdpMediaDescr,"TIAS",512000);
        char  fmtp_value[1024] = "";
        char* pDst = fmtp_value;
        
        sprintf( pDst, "%d ", (int)payloadType );
        pDst += strlen(pDst);
        strcpy(pDst,"profile-level-id=428014;packetization-mode=1");
        
        rvSdpMediaDescrAddAttr(sdpMediaDescr,"fmtp",fmtp_value); 
     }
  }

  return rv;
}

RvStatus RvUserAgent::SDP_addSdpToOutgoingMsg(IN  RvSipCallLegHandle            hCallLeg) {

  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);

  RvStatus rv = RV_OK;

  if(hCallLeg) {
    RvSipMsgHandle            hMsg = (RvSipMsgHandle)0;
    rv = RvSipCallLegGetOutboundMsg(hCallLeg, &hMsg);
    if(rv>=0) {
      if(!hMsg) rv=RV_ERROR_UNKNOWN;
      else {
	RvSdpMsg sdpMsg;
	if(!rvSdpMsgConstruct(&sdpMsg)) rv=RV_ERROR_UNKNOWN;
	else {
	  rv = SDP_InitSdpMsg(&sdpMsg);
	  if(rv>=0) {
	    rv = SDP_FillSdpMsg(&sdpMsg);
	    if(rv>=0) {
	      RvSdpStatus sdpRv=(RvSdpStatus)0;
	      char sdp[SDP_BUFF_SIZE]="\0";
	      rvSdpMsgEncodeToBuf(&sdpMsg, sdp, sizeof(sdp)-1, &sdpRv );
	      if (sdpRv != RV_SDPSTATUS_OK) rv=RV_ERROR_UNKNOWN;
	      else {
		RvSipMsgSetBody( hMsg, sdp);
		char ct[33];
		strcpy(ct,SDP_CONTENT_TYPE);
		RvSipMsgSetContentTypeHeader(hMsg, ct);
	      }
	    }
	  }
	  rvSdpMsgDestruct(&sdpMsg);
	}
      }
    }
  }

  return rv;
}

RvSdpConnection* RvUserAgent::SDP_getAddrFromConnDescr(RvSdpConnection *conn_descr, char* remoteIP, int remoteIPLen) {

  if(conn_descr) {

    RvSdpNetType netType = rvSdpConnectionGetNetType(conn_descr);
    if(netType == RV_SDPNETTYPE_IN) {

      RvSdpAddrType addrtype = rvSdpConnectionGetAddrType( conn_descr );
      sa_family_t family = (addrtype == RV_SDPADDRTYPE_IP4) ? AF_INET : (addrtype == RV_SDPADDRTYPE_IP6) ? AF_INET6: AF_UNSPEC;

      if(family !=AF_UNSPEC) {

	const char *ip = rvSdpConnectionGetAddress(conn_descr);
	if(ip) {
	  strncpy(remoteIP,ip,remoteIPLen);
	  return conn_descr;
	}
      }
    }
  }

  return NULL;
}

RvSdpStatus RvUserAgent::SDP_readSdpFromMsg(IN  RvSipCallLegHandle            hCallLeg,RvSipMsgHandle SipMsg,
					bool &audioFound, int audioPayloadType,
					bool &videoFound, int videoPayloadType,
					char* remoteIP, int remoteIPLen,
					int &audioPort, int &videoPort, bool &hasSdp) {

  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);

    RvSdpStatus sdprv = RV_SDPSTATUS_OK;
    RvStatus rv = RV_OK;

  if(hCallLeg) {
    RvSipMsgHandle            hMsg = (RvSipMsgHandle)0;
    if(SipMsg)
        hMsg = SipMsg;
    else
        rv = RvSipCallLegGetReceivedMsg(hCallLeg, &hMsg);

    if(hMsg) {
        RV_CHAR strBody[SDP_BUFF_SIZE+1];
        RvUint32 len=0;
        if(SDP_MsgGetSdp (hMsg, strBody, sizeof(strBody)-1, &len)>=0 && len>0) {

            RvSdpMsg          sdpMsg;
            RvSdpParseStatus  status;

            if(rvSdpMsgConstructParse( &sdpMsg, strBody, (RvInt32*)&len, &status)) {

                {
                    //remote IP Address extraction

                    remoteIP[0]=0;

                    RvSdpConnection * conn_descr = rvSdpMsgGetConnection( &sdpMsg );
                    if(conn_descr) {
                        SDP_getAddrFromConnDescr(conn_descr, remoteIP, remoteIPLen);
                    }

                    if(!remoteIP[0]) {
                        RvSdpOrigin* origin = rvSdpMsgGetOrigin(&sdpMsg);
                        if(origin) {
                            RvSdpNetType netType = rvSdpOriginGetNetType(origin);
                            if(netType == RV_SDPNETTYPE_IN) {
                                RvSdpAddrType addrtype = rvSdpOriginGetAddressType(origin);
                                if(addrtype == RV_SDPADDRTYPE_IP4 || addrtype == RV_SDPADDRTYPE_IP6) {
                                    const char *ip = rvSdpOriginGetAddress(origin);
                                    if(ip) {
                                        strncpy(remoteIP,ip,remoteIPLen);
                                    }
                                }
                            }
                        }
                    }
                }

                int   media_count = rvSdpMsgGetNumOfMediaDescr( &sdpMsg );
                int ii=0;

                for(ii=0;ii<media_count;ii++) {

                    bool foundHere=false;

                    RvSdpMediaDescr * media_descr = rvSdpMsgGetMediaDescr( &sdpMsg, ii );

                    if(!media_descr) continue;

                    RvSdpProtocol protocol = rvSdpMediaDescrGetProtocol(media_descr);
                    if(protocol != RV_SDPPROTOCOL_RTP) continue;

                    RvSdpMediaType mediaTypeRmt = rvSdpMediaDescrGetMediaType(media_descr);

                    int payload_count = rvSdpMediaDescrGetNumOfPayloads (media_descr);

                    int iii=0;
                    for(iii=0;iii<payload_count;iii++) {
                        int rtpPayloadType = rvSdpMediaDescrGetPayloadNumber(media_descr, iii);


                        if (rtpPayloadType >= 0 && rtpPayloadType < 256) {

                            if(mediaTypeRmt == RV_SDPMEDIATYPE_AUDIO && ((rtpPayloadType == audioPayloadType)||(audioPayloadType==-1))) {
                                if(!audioFound) {
                                    foundHere=true;
                                    audioFound=true;
                                    audioPort = rvSdpMediaDescrGetPort(media_descr);
                                }
                            } else if(mediaTypeRmt == RV_SDPMEDIATYPE_VIDEO && ((rtpPayloadType == videoPayloadType)||(videoPayloadType==-1))) {
                                if(!videoFound) {
                                    foundHere=true;
                                    videoFound=true;
                                    videoPort = rvSdpMediaDescrGetPort(media_descr);
                                }
                            }
                        }
                    }

                    {
                        //Try again for Remote IP address
                        if(foundHere && !remoteIP[0]) {
                            RvSdpConnection * conn_descr = rvSdpMediaDescrGetConnection( media_descr );
                            if(conn_descr) {
                                SDP_getAddrFromConnDescr(conn_descr, remoteIP, remoteIPLen);
                            }
                        }
                    }
                }

                rvSdpMsgDestruct(&sdpMsg);
            }
        }else{
            hasSdp = false;
        }
    }
  }

  return sdprv;
}

RV_Status RvUserAgent::SDP_MsgGetSdp (IN RvSipMsgHandle hMsg, char *sdp, RvUint32 sdpSize, RvUint32 *len) {

  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);

  RvStatus rv = RV_OK;

  RvSipBodyHandle hBody = RvSipMsgGetBodyObject (hMsg);
  if (hBody) {
    RvSipContentTypeHeaderHandle      hContentType = RvSipBodyGetContentType (hBody);
    if (hContentType) {
      RvSipMediaType eMediaType = RvSipContentTypeHeaderGetMediaType (hContentType);
      RvSipMediaSubType eMediaSubType = RvSipContentTypeHeaderGetMediaSubType (hContentType);
      /* if the content type of the message is multipart mixed parse the body */
      if ((RVSIP_MEDIATYPE_MULTIPART == eMediaType) && (RVSIP_MEDIASUBTYPE_MIXED == eMediaSubType)) {

	RV_CHAR strBodyStatic[SDP_BUFF_SIZE+SDP_BUFF_SIZE+1];

	RvUint32                        length = (RvUint32)RvSipBodyGetBodyStrLength(hBody);

	if((length+1)*sizeof(RV_CHAR)>sizeof(strBodyStatic)) {

	  RV_CHAR                         *strBody;

	  strBody  = (RV_CHAR*)malloc((length+1)*sizeof(RV_CHAR));
	  rv = RvSipBodyGetBodyStr(hBody, strBody, length, &length);
	  if (RV_Success != rv) {
	    free(strBody);
	    return RV_ERROR_UNKNOWN;
	  }
	  rv = RvSipBodyMultipartParse(hBody, strBody, length);
	  if (RV_Success != rv) {
	    free(strBody);
	    return RV_ERROR_UNKNOWN;
	  }
	  free(strBody);
	} else {
	  rv = RvSipBodyGetBodyStr(hBody, strBodyStatic, length, &length);
	  if (RV_Success != rv) {
	    return RV_ERROR_UNKNOWN;
	  }
	  rv = RvSipBodyMultipartParse(hBody, strBodyStatic, length);
	  if (RV_Success != rv) {
	    return RV_ERROR_UNKNOWN;
	  }
	}

	RvSipBodyPartHandle               hBodyPart = (RvSipBodyPartHandle)0;
	rv = RvSipBodyGetBodyPart (hBody, RVSIP_FIRST_ELEMENT, NULL, &hBodyPart);
	if (RV_Success != rv) {
	  return RV_ERROR_UNKNOWN;
	}
	while (hBodyPart) {

	  HPAGE                             hPage;

	  rv = RvSipBodyPartEncode (hBodyPart, mChannel->getAppPool(), &hPage, &length);
	  if (RV_Success != rv) {
	    *len = 0;
	    break;
	  }
	  if (sdpSize >= length) {
	    char sdpTemp[SDP_BUFF_SIZE];
	    char *start;
	    /* Copy encoded body part to consecutive buffer */
	    rv =  RPOOL_CopyToExternal (mChannel->getAppPool(), hPage, 0, sdpTemp, length);
	    sdpTemp[length] = '\0';

	    // let's search for a media-type.
	    // NOTE: both lowercase "application/sdp" (defined session 8.1 RFC4566) and
	    // uppercase "application/SDP" (RFC3204) are valid.
	    // New Radvision parser only accepts the lowercase value; whereas, the old version doesn't care.
	    // So, we try to accept both version in this case.
	    if (strstr(sdpTemp,"application/SDP") != NULL ||
		strstr(sdpTemp,"application/sdp") != NULL){
	      start = strstr(sdpTemp, "v=");
	      if (start == NULL) {
		strcpy(sdp,sdpTemp);
		*len = length;
	      } else {
		strcpy(sdp,start);
		*len = length - (start - sdpTemp);
	      }
	      RPOOL_FreePage (mChannel->getAppPool(), hPage);
	      break;
	    }
	  }
	  RPOOL_FreePage (mChannel->getAppPool(), hPage);

	  rv = RvSipBodyGetBodyPart(hBody, RVSIP_NEXT_ELEMENT, hBodyPart, &hBodyPart);
	  if (RV_Success != rv) {
	    *len = 0;
	    break;
	  }
	}
      } else {
	rv = RvSipMsgGetBody (hMsg, sdp, sdpSize, len);
      }
    }
  }

  return rv;
}

RvStatus RvUserAgent::SDP_parseRemoteSdp(IN  RvSipCallLegHandle            hCallLeg, RvUint16& code,RvSipMsgHandle hMsg,bool bMandatory) {

  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);

    RvStatus rv = RV_OK;

    int samplingRate = 0;

    int audioPayloadType = -1;
  if(mAudioCall || mChannel->isSigOnly()) {
      Sip_1_port_server::EnumSipAudioCodec codec = sip_1_port_server::EnumSipAudioCodec_G_711;
      if(!mChannel->isSigOnly())
          codec = static_cast<Sip_1_port_server::EnumSipAudioCodec>(mConfig.ProtocolProfile.AudioCodec);
      SDP_GetSdpAudioEncodingName(codec,audioPayloadType,samplingRate, mConfig.ProtocolProfile.AudioPayloadType);
  }

  if(mAudioHDCall) {
      audioPayloadType = AAC_LD_PT;
      samplingRate = 48000;
  }

  int videoPayloadType=-1;
  if(mVideoCall) {
    Sip_1_port_server::EnumSipVideoCodec codec = static_cast<Sip_1_port_server::EnumSipVideoCodec>(mConfig.ProtocolProfile.VideoCodec);
    SDP_GetSdpVideoEncodingName(codec,videoPayloadType,samplingRate);
  }

  bool audioFound=false;
  bool videoFound=false;
  char remoteIP[129]="\0";
  int audioPort=0;
  int videoPort=0;
  bool has_sdp = true;

  RvSdpStatus sdprv = SDP_readSdpFromMsg(hCallLeg,hMsg, 
				     audioFound, audioPayloadType, 
				     videoFound, videoPayloadType, 
				     remoteIP, sizeof(remoteIP)-1,
				     audioPort, videoPort,has_sdp);

  if(!has_sdp && !bMandatory)
      return rv;

  if(sdprv<0) {
    code=415;
    rv=RV_ERROR_UNKNOWN;
    return rv;
  }

  if(mAudioCall || mChannel->isSigOnly()) {
    if(!audioFound ||audioPort<1) {
        code=415;
        rv=RV_ERROR_UNKNOWN;
        return rv;
    }
  }

  if(mAudioHDCall) {
    if(!audioFound ||audioPort<1) {
      code=415;
      rv=RV_ERROR_UNKNOWN;
      return rv;
    }
  }

  if(mVideoCall) {
    if(!videoFound ||videoPort<1) {
        code=415;
        rv=RV_ERROR_UNKNOWN;
        return rv;
    }
  }

  if(!remoteIP[0]) {
      code=415;
      rv=RV_ERROR_UNKNOWN;
      return rv;
  }

    {
    const RvSipTransportAddr& lAddr = mChannel->getLocalAddress();

    int family=PF_INET;
    if(lAddr.eAddrType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP6) family=PF_INET6;

    //Set remote RTP peer
    if(mAudioCall || mChannel->isSigOnly()) {
      ACE_INET_Addr rAddr(audioPort, remoteIP, family);
      mMediaChannel->setRemoteAddr(MEDIA_NS::AUDIO_STREAM,&rAddr,audioPort);
    }
    if(mAudioHDCall) {
      ACE_INET_Addr rAddr(audioPort, remoteIP, family);
      mMediaChannel->setRemoteAddr(MEDIA_NS::AUDIOHD_STREAM,&rAddr,audioPort);
    }
    if(mVideoCall) {
      ACE_INET_Addr rAddr(videoPort, remoteIP, family);
      mMediaChannel->setRemoteAddr(MEDIA_NS::VIDEO_STREAM,&rAddr,videoPort);
    }
    mChannel->RemoteSDPOffered(true);
  }

  return rv;
}

void RvUserAgent::adjustIPv6ForSdp(char* s1, const char* s2) {
  int len=(int)strlen(s2);
  int i=0;
  while(i<len && (s2[i]==' ' || s2[i]=='[')) ++i;
  len-=i;

  s1[0]=0;
  s2+=i;
  i=0;
  while(i<len && s2[i] && s2[i]!=' ' && s2[i]!=']') { s1[i]=s2[i]; s1[i+1]=0; ++i; }
}

//////////////////////////// Public callback interface /////////////////////////////////

RvStatus RvUserAgent::media_beforeConnect(IN  RvSipCallLegHandle            hCallLeg) {

  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);

  RvStatus rv = SDP_addSdpToOutgoingMsg(hCallLeg);

  return rv;
}

RvStatus RvUserAgent::media_onOffering(IN  RvSipCallLegHandle            hCallLeg, RvUint16& code,bool bMandatory) {

  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);

  RvStatus rv = RV_OK;

  rv = SDP_parseRemoteSdp(hCallLeg,code,NULL,bMandatory);

  return rv;
}

RvStatus RvUserAgent::media_beforeAccept(IN  RvSipCallLegHandle            hCallLeg) {

  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);

  RvStatus rv = SDP_addSdpToOutgoingMsg(hCallLeg);

  return rv;
}

RvStatus RvUserAgent::media_onConnected(IN  RvSipCallLegHandle            hCallLeg, RvUint16 &code,bool parseSdp) {

  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);

  RvStatus rv = RV_OK;

  bool isInitiator = (mChannel && mChannel->isOriginate());

  if(isInitiator && parseSdp) {
    rv = SDP_parseRemoteSdp(hCallLeg,code,NULL);
  }
  if(mChannel && !mChannel->isSigOnly())
  	ControlStreamGeneration(true,isInitiator,false);

  return rv;
}

RvStatus RvUserAgent::media_onDisconnecting(IN  RvSipCallLegHandle            hCallLeg) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  return 0;
}

RvStatus RvUserAgent::media_onDisconnected(IN  RvSipCallLegHandle            hCallLeg) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  return 0;
}

RvStatus RvUserAgent::media_onCallExpired(IN  RvSipCallLegHandle            hCallLeg, bool &hold) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  hold = (mMediaChannel && mMediaChannel->isAsync());

  if(!mChannel->isSigOnly())
  	ControlStreamGeneration(false,true,hold);

  return 0;
}

RvStatus RvUserAgent::media_beforeDisconnect(IN  RvSipCallLegHandle            hCallLeg) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  return 0;
}

RvStatus RvUserAgent::media_onByeReceived(IN  RvSipCallLegHandle            hCallLeg, bool &hold) {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  hold = (mMediaChannel && mMediaChannel->isAsync());

  if(!mChannel->isSigOnly())
  	ControlStreamGeneration(false,false,hold);

  return 0;
}

RvStatus RvUserAgent::media_stop() {
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);

  if(!mChannel->isSigOnly())  
	ControlStreamGeneration(false,true,false);

  return 0;
}
RvStatus RvUserAgent::media_prepareConnect(IN  RvSipCallLegHandle            hCallLeg, RvUint16 &code,RvSipMsgHandle hMsg) {

  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);

  RvStatus rv = RV_OK;

  bool isInitiator = (mChannel && mChannel->isOriginate());

  if(isInitiator) {
    rv = SDP_parseRemoteSdp(hCallLeg,code,hMsg,false);
  }else
    rv = SDP_parseRemoteSdp(hCallLeg,code,hMsg);

  return rv;
}
int RvUserAgent::media_startTip(uint32_t mux_csrc){
  TC_LOG_INFO_LOCAL(mPort, LOG_SIP, "[RvUserAgent::" << __FUNCTION__<<"] : " << mName);
  StartTipNegotiation(mux_csrc);
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
