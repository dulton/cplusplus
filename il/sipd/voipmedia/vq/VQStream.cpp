/// @file
/// @brief VQStream class
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <string>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/scoped_array.hpp>

#include <vqmon.h>
#include <vqmonsa.h>
#include <vqmapiif.h>

#include "VoIPUtils.h"
#include "MediaPacket.h"
#include "MediaCodecs.h"
#include "MediaPayloads.h"
#include "VQStream.h"
#include "VQStatsData.h"
#include "VQTimeUtils.h"

using std::string;

USING_MEDIAFILES_NS;

///////////////////////////////////////////////////////////////////////////////

DECL_VQSTREAM_NS

///////////////////////////////////////////////////////////////////////////////

#define impairment 0

///////////////////////////////////////////////////////////////////////////////

class VQInterfaceImpl : public VQInterface
{

public:

    typedef vqmon_handle_t InterfaceType;

    VQInterfaceImpl() : mInterfaceHandle(0)
    {

        // Create Logical Streaming Interface
        vqmon_handle_t result;

        vqmon_interfaceprops_t props;
        memset(&props, 0, sizeof(props));

        // According to shane.holthaus@telchemy.com, the cost is just a pointer
        // per bin. We want approximately as many bins as we have streams
        // so we don't have to follow the linked list very far. Shane
        // recommends 256 or 512
        props.nStreamDemuxBins = 16;
        props.eIfState = vqmonInterfaceStateUp;

        // According to shane.holthaus@telchemy.com, this is currently unused
        // by VQmon. For VLANs it'd be hard to know the rate, but the line
        // rate would be the absolute max. For now we're just claiming
        // all IFs use 1Gbps.
        props.nIfRcvBitrate = 1000000000;// FIXME when VQmon uses this

        if (VQmonInterfaceCreate(&result, &props) == VQMON_ESUCCESS && result)
        {
            mInterfaceHandle = result;
            vqmon_interface_t* pInterface = VQMON_INTERFACE_HANDLETOPTR(mInterfaceHandle);
            if(pInterface)
            {
                pInterface->tProperties.pAppData=this;
            }
        }
    }

    virtual ~VQInterfaceImpl()
    {
        if(mInterfaceHandle)
        {
            VQmonInterfaceDestroy(mInterfaceHandle);
            mInterfaceHandle=0;
        }
    }

    InterfaceType getInterface() const
    {
        return mInterfaceHandle;
    }

    vqmon_codecid_t getVoiceCodec() const
    {
        return mVoiceCodec;
    }

    void setVoiceCodec(vqmon_codecid_t c)
    {
        mVoiceCodec=c;
    }

    vqmon_codecid_t getAudioHDCodec() const
    {
        return mAudioHDCodec;
    }

    void setAudioHDCodec(vqmon_codecid_t c)
    {
        mAudioHDCodec=c;
    }

    vqmon_codecid_t getVideoCodec() const
    {
        return mVideoCodec;
    }

    void setVideoCodec(vqmon_codecid_t c)
    {
        mVideoCodec=c;
    }

private:
    InterfaceType mInterfaceHandle;
    vqmon_codecid_t mVoiceCodec;
    vqmon_codecid_t mAudioHDCodec;
    vqmon_codecid_t mVideoCodec;
};

///////////////////////////////////////////////////////////////////////////////

class VQMonSession : public VQStream
{

public:

    typedef vqmon_handle_t InterfaceType;
    typedef vqmon_handle_t StreamType;

    explicit VQMonSession(VQInterfaceHandler interface,
            CodecType codec, PayloadType payload,
            int bitRate, uint32_t ssrc, int jbSize) : mInterface(interface),
    mCodec(codec),
    mPayload(payload),
    mBitRate(bitRate),
    mSSRC(ssrc),
    mJBSize(jbSize),
    mStreamHandle(0),
    mInitFailed(false),
    mPacketCounter(0)
    {
        ;
    }

    virtual ~VQMonSession()
    {
        destroy();
    }

    virtual bool getStreamInited() const
    {
        return !mInitFailed;
    }

    /**
     * Feed a packet to the stream
     */
    virtual int packetRecv(VqmaRtpPacketInfo packet)
    {

        if(!packet.packet) return -1;

#if impairment
        if(mPacketCounter % impairment == 0)
        {
            return 0;
        }
#endif

        uint8_t* packetStart = packet.packet->getStartPtr();
        uint32_t len=packet.packet->getSize();

        if(!packetStart || len<1) return -1;

        vqmon_pktdesc_t pktDesc;
        memset(&pktDesc, 0, sizeof(pktDesc));

        pktDesc.pPktBuffer = (tcmyU8*)(packetStart);
        pktDesc.nCapturedPktLen = len;
        pktDesc.nTotalPktLen = len;
        pktDesc.ePktStartIndicator = vqmonPktDescStartRTPHeader;

        RtpHeaderParam hdrParam;
        packet.packet->getRtpHeaderParam(hdrParam);

        pktDesc.rtpinfo.nPayloadType=hdrParam.rtp_pl;
        if(packet.packet->getPayloadSize() < hdrParam.rtp_csrcc*4)
            return -1;
        pktDesc.rtpinfo.nPayloadSize=packet.packet->getPayloadSize()- hdrParam.rtp_csrcc*4;
        pktDesc.rtpinfo.nSeqNumber=hdrParam.rtp_sn;
        pktDesc.rtpinfo.nSSRC=hdrParam.rtp_ssrc;
        pktDesc.rtpinfo.nTimestamp=hdrParam.rtp_ts;

        struct timeval nTimestamp = packet.localTimestamp;

        if (mCodec== AAC_LD)
        {
            1[packetStart] |= 0x80;
        }


        vqmon_result_t result = VQmonStreamIndicatePacket(mStreamHandle, (vqmon_time_t)nTimestamp, &pktDesc);

        if (result != VQMON_ESUCCESS)
        {
            return -1;
        }

        ++mPacketCounter;

        return 0;
    }

    /**
     * Destroy the session
     */
    virtual int destroy()
    {
        if(mStreamHandle)
        {
            VQmonStreamDestroy(mStreamHandle);
            mStreamHandle=0;
        }
        return 0;
    }

    static vqmon_func_streammgmtnotifyhdlrcb_t get_stream_notify_handler()
    {
        return VQMonSession::NotifyHandler;
    }

protected:

    static vqmon_result_t StreamCreateHandler(vqmon_handle_t const hInterface,
            vqmon_handle_t const hStream,
            struct _vqmon_streammgmtnotify_info_s* pNotifyInfo)
    {
        return VQMON_ESUCCESS;
    }

    static vqmon_result_t StreamActivateHandler(vqmon_handle_t const hInterface,
            vqmon_handle_t const hStream,
            struct _vqmon_streammgmtnotify_info_s* pNotifyInfo)
    {
        return VQMON_ESUCCESS;
    }

    static vqmon_result_t StreamDeactivateHandler(vqmon_handle_t const hInterface,
            vqmon_handle_t const hStream,
            struct _vqmon_streammgmtnotify_info_s* pNotifyInfo)
    {

        return VQMON_ESUCCESS;
    }

    static vqmon_result_t StreamTerminateHandler(vqmon_handle_t const hInterface,
            vqmon_handle_t const hStream,
            struct _vqmon_streammgmtnotify_info_s* pNotifyInfo)
    {
        return VQMON_ESUCCESS;
    }

    static vqmon_result_t StreamCodecRequestHandler(vqmon_handle_t const hInterface,
            vqmon_handle_t const hStream,
            struct _vqmon_streammgmtnotify_info_s* pNotifyInfo)
    {
        if(pNotifyInfo && pNotifyInfo->eStreamType == vqmonStreamTypeVoice)
        {
            pNotifyInfo->state.codecrequest.eType = vqmonStreamTypeVoice;
            pNotifyInfo->state.codecrequest.idCODECType = vqmonCODECVoiceUnknown;
            vqmon_interface_t* pInterface = VQMON_INTERFACE_HANDLETOPTR(hInterface);
            if(pInterface)
            {
                VQInterfaceImpl* interface = static_cast<VQInterfaceImpl*>(pInterface->tProperties.pAppData);
                if(interface)
                {
                    return interface->getVoiceCodec();
                    return VQMON_ESUCCESS;
                }
            }
            pNotifyInfo->state.codecrequest.idCODECType = getVQmonCodec(-1,
                    pNotifyInfo->state.codecrequest.nRTPPayloadType,
                    0);
        }
        return VQMON_ESUCCESS;
    }

    static vqmon_result_t NotifyHandler (
            vqmon_handle_t const hInterface,
            vqmon_handle_t const hStream,
            const vqmon_streammgmtnotifytype_t eNotification,
            struct _vqmon_streammgmtnotify_info_s* pNotifyInfo)
    {
        if (eNotification == vqmonStreamMgmtNotifyCreated)
        {
            return StreamCreateHandler(hInterface,hStream, pNotifyInfo);
        }
        else if (eNotification == vqmonStreamMgmtNotifyActivating)
        {
            return StreamActivateHandler(hInterface,hStream, pNotifyInfo);
        }
        else if (eNotification == vqmonStreamMgmtNotifyDeactivating)
        {
            return StreamDeactivateHandler(hInterface,hStream, pNotifyInfo);
        }
        else if (eNotification == vqmonStreamMgmtNotifyTerminated)
        {
            return StreamTerminateHandler(hInterface,hStream, pNotifyInfo);
        }
        else if(eNotification == vqmonStreamMgmtNotifyCODECRequest)
        {
            return StreamCodecRequestHandler(hInterface,hStream, pNotifyInfo);
        }

        // terminated streams are removed AFTER their
        // stats are reported at least one additional time,
        // the check is done in the stats reporting

        return VQMON_ESUCCESS;
    }

    static vqmon_codecid_t getVQmonCodec(CodecType codec,PayloadType payload,int bitRate)
    {

        switch(codec)
        {
            case G711u:
            return vqmonCODECVoiceG711u;

            case G711A:
            return vqmonCODECVoiceG711A;

            case G723_1:
            if(bitRate==5300) return vqmonCODECVoiceG723153K;
            else return vqmonCODECVoiceG723163K;

            case G729:
            return vqmonCODECVoiceG729;
            case G729A:
            return vqmonCODECVoiceG729A;
            case G729B:
            return vqmonCODECVoiceG729withVAD;
            case G729AB:
            return vqmonCODECVoiceG729AwithVAD;

            case G726ITU:
            case G726:
            if(bitRate==16000) return vqmonCODECVoiceG72616K;
            else if(bitRate==24000) return vqmonCODECVoiceG72624K;
            else if(bitRate==40000) return vqmonCODECVoiceG72640K;
            else return vqmonCODECVoiceG72632K;

            case G728:
            return vqmonCODECVoiceG728;

            case CODEC_TYPE_G722:
            if(bitRate==56000) return vqmonCODECVoiceG722_56k;
            else if(bitRate==48000) return vqmonCODECVoiceG722_48k;
            else return vqmonCODECVoiceG722_64k;

            case CODEC_TYPE_G722_1_32:
            return vqmonCODECVoiceG7221_32k;

            case CODEC_TYPE_G722_1_24:
            return vqmonCODECVoiceG7221_24k;

            case CODEC_TYPE_G711_1u:
            case CODEC_TYPE_G711_1A:
            return vqmonCODECVoiceGIPSiPCMwb; // temporary

            case AMRNB_0_OA_475: return vqmonCODECVoiceAMRNB4k75;
            case AMRNB_1_OA_515: return vqmonCODECVoiceAMRNB5k15;
            case AMRNB_2_OA_590: return vqmonCODECVoiceAMRNB5k9;
            case AMRNB_3_OA_670: return vqmonCODECVoiceAMRNB6k7;
            case AMRNB_4_OA_740: return vqmonCODECVoiceAMRNB7k4;
            case AMRNB_5_OA_795: return vqmonCODECVoiceAMRNB7k95;
            case AMRNB_6_OA_102: return vqmonCODECVoiceAMRNB10k2;
            case AMRNB_7_OA_122: return vqmonCODECVoiceAMRNB12k2;

            case AMRNB_0_BE_475: return vqmonCODECVoiceAMRNB4k75;
            case AMRNB_1_BE_515: return vqmonCODECVoiceAMRNB5k15;
            case AMRNB_2_BE_590: return vqmonCODECVoiceAMRNB5k9;
            case AMRNB_3_BE_670: return vqmonCODECVoiceAMRNB6k7;
            case AMRNB_4_BE_740: return vqmonCODECVoiceAMRNB7k4;
            case AMRNB_5_BE_795: return vqmonCODECVoiceAMRNB7k95;
            case AMRNB_6_BE_102: return vqmonCODECVoiceAMRNB10k2;
            case AMRNB_7_BE_122: return vqmonCODECVoiceAMRNB12k2;

            case AMRWB_0_OA_660:  return vqmonCODECVoiceG7222_6k6;
            case AMRWB_1_OA_885:  return vqmonCODECVoiceG7222_8k85;
            case AMRWB_2_OA_1265: return vqmonCODECVoiceG7222_12k65;
            case AMRWB_3_OA_1425: return vqmonCODECVoiceG7222_14k25;
            case AMRWB_4_OA_1585: return vqmonCODECVoiceG7222_15k85;
            case AMRWB_5_OA_1825: return vqmonCODECVoiceG7222_18k25;
            case AMRWB_6_OA_1985: return vqmonCODECVoiceG7222_19k85;
            case AMRWB_7_OA_2305: return vqmonCODECVoiceG7222_23k05;
            case AMRWB_8_OA_2385: return vqmonCODECVoiceG7222_23k85;

            case AMRWB_0_BE_660:  return vqmonCODECVoiceG7222_6k6;
            case AMRWB_1_BE_885:  return vqmonCODECVoiceG7222_8k85;
            case AMRWB_2_BE_1265: return vqmonCODECVoiceG7222_12k65;
            case AMRWB_3_BE_1425: return vqmonCODECVoiceG7222_14k25;
            case AMRWB_4_BE_1585: return vqmonCODECVoiceG7222_15k85;
            case AMRWB_5_BE_1825: return vqmonCODECVoiceG7222_18k25;
            case AMRWB_6_BE_1985: return vqmonCODECVoiceG7222_19k85;
            case AMRWB_7_BE_2305: return vqmonCODECVoiceG7222_23k05;
            case AMRWB_8_BE_2385: return vqmonCODECVoiceG7222_23k85;

            case AAC_LD:
            return vqmonCODECAudioMPEG4LDAAC;// AudioHD AAC-LD

            case VH264:
            case VH264_CTS_720P:
            case VH264_CTS_1080P:
            case VH264_TBG_720P:
            case VH264_TBG_1080P:
            case VH264_TBG_CIF:
            case VH264_TBG_XGA:
            return vqmonCODECVideoH264;// Video

            default:
            break;
        }

        switch(payload)
        {
            case PAYLOAD_711u:
            return vqmonCODECVoiceG711u;

            case PAYLOAD_711A:
            return vqmonCODECVoiceG711A;

            case PAYLOAD_723:
            if(bitRate==5300) return vqmonCODECVoiceG723153K;
            else return vqmonCODECVoiceG723163K;

            case PAYLOAD_729:
            return vqmonCODECVoiceG729;

            case PAYLOAD_726:
            if(bitRate==16000) return vqmonCODECVoiceG72616K;
            else if(bitRate==24000) return vqmonCODECVoiceG72624K;
            else if(bitRate==40000) return vqmonCODECVoiceG72640K;
            else return vqmonCODECVoiceG72632K;

            case PAYLOAD_728:
            return vqmonCODECVoiceG728;

            case PAYLOAD_722:
            if(bitRate==56000) return vqmonCODECVoiceG722_56k;
            else if(bitRate==48000) return vqmonCODECVoiceG722_48k;
            else return vqmonCODECVoiceG722_64k;

            default:
            break;
        }

        return vqmonCODECVoiceUnknown;
    }

protected:
    VQInterfaceHandler mInterface;
    CodecType mCodec;
    PayloadType mPayload;
    int mBitRate;
    uint32_t mSSRC;
    int mJBSize;
    StreamType mStreamHandle;
    bool mInitFailed;
    uint32_t mPacketCounter;
};

////////////////////////////////////////////////////////////////////////////////////////////

#include "VQStreamVoice.hpp"
#include "VQStreamVideo.hpp"
#include "VQStreamAudioHD.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

class VQStreamManagerImpl : public boost::noncopyable
{

public:

    VQStreamManagerImpl()
    {
        init();
    }

    virtual ~VQStreamManagerImpl()
    {
        destroy();
    }

    void clean()
    {
        destroy();
        init();
    }

    VQInterfaceHandler createNewInterface()
    {
        VQInterfaceHandler ret(new VQInterfaceImpl());
        return ret;
    }

    VQStreamHandler createNewStream(VQInterfaceHandler interface, CodecType codec, PayloadType payload, int bitRate, uint32_t ssrc, int jbSize)
    {

        VQStreamHandler stream;

        VQMediaType mt = getVQMType(codec);

        if(mt==VQ_VOICE) stream.reset(new VQMonSessionVoice(interface,codec,payload,bitRate,ssrc,jbSize));
        else if(mt==VQ_VIDEO) stream.reset(new VQMonSessionVideo(interface,codec,payload,bitRate,ssrc,jbSize));
        else if(mt==VQ_AUDIOHD) stream.reset(new VQMonSessionAudioHD(interface,codec,payload,bitRate,ssrc,jbSize));

        return stream;
    }

protected:

    void init()
    {

        vqmon_result_t vResult;
        vqmon_initparams_t vInitCfg;

        memset(&vInitCfg, 0, sizeof(vqmon_initparams_t));
        vInitCfg.nVersion = VQMON_IFVERSION_CURRENT;
        vInitCfg.pfAlertHdlr = 0;
        vInitCfg.pfStreamNotifyHdlr = VQMonSession::get_stream_notify_handler();
        vInitCfg.eCMEModelType = vqmonCMEModelNonLinear;

        vResult = VQmonInitialize(&vInitCfg);
        if (VQMON_ESUCCESS != vResult)
        {
            // This is pretty fatal, but should never happen
            throw string("Unable to initialize VQmon/HD");
        }
    }

    static VQMediaType getVQMType(CodecType codec)
    {
        if(codec>=FIRST_VOICE_CODEC && codec<=LAST_VOICE_CODEC)
        {
            return VQ_VOICE;
        }
        else if(codec>=FIRST_VIDEO_CODEC && codec<=LAST_VIDEO_CODEC)
        {
            return VQ_VIDEO;
        }
        else if(codec>=FIRST_AUDIOHD_CODEC && codec<=LAST_AUDIOHD_CODEC)
        {
            return VQ_AUDIOHD;
        }
        return VQ_MEDIA_UNDEFINED;
    }

    void destroy()
    {
        VQmonCleanup();
    }
};

////////////////////////////////////////////////////////////////////////////////////////////

VQStreamManager::VQStreamManager()
{
    mPimpl.reset(new VQStreamManagerImpl());
}

VQStreamManager::~VQStreamManager()
{
}

void VQStreamManager::clean()
{
    mPimpl->clean();
}

VQInterfaceHandler VQStreamManager::createNewInterface()
{
    return mPimpl->createNewInterface();
}

VQStreamHandler VQStreamManager::createNewStream(VQInterfaceHandler interface,
        CodecType codec, PayloadType payload,
        int bitRate, uint32_t ssrc, int jbSize)
{
    return mPimpl->createNewStream(interface, codec, payload, bitRate, ssrc, jbSize);
}

///////////////////////////////////////////////////////////////////////////////

END_DECL_VQSTREAM_NS

////////////////////////////////////////////////////////////////////////////////

