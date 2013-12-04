/// @file
/// @brief Media Files common objects
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <netinet/in.h>

#include <boost/scoped_array.hpp>

#include "VoIPUtils.h"
#include "MediaFilesCommon.h"
#include "MediaCodecs.h"
#include "MediaPayloads.h"
#include "MediaPacket.h"
#include "MediaStreamInfo.h"

///////////////////////////////////////////////////////////////////////////////

DECL_MEDIAFILES_NS

///////////////////////////////////////////////////////////////////////////////

struct MediaPacketHeader
{
    int32_t word1;
    int32_t word2;
    int32_t word3;
};

class MediaPacketImplNew
{

public:

    MediaPacketImplNew() :
    mPayloadSize(MediaPacket::DEFAULT_RTP_PKT_SIZE),
    mMaxSize(MediaPacket::DEFAULT_RTP_PKT_SIZE),
    mTime(0),mCsrcc(0)
    {
        mBody.reset(new uint8_t[MediaPacket::DEFAULT_RTP_PKT_SIZE]);
    }

    MediaPacketImplNew(int pl) : mTime(0),mCsrcc(0)
    {
        if(pl<0) pl=0;
        else if(pl>MediaPacket::PAYLOAD_MAX_SIZE) pl=MediaPacket::PAYLOAD_MAX_SIZE;
        mMaxSize=pl+MediaPacket::BUFFER_SIZE;
        mBody.reset(new uint8_t[mMaxSize]);
        mPayloadSize=pl;
    }

    virtual ~MediaPacketImplNew()
    {
        ;
    }

    MediaPacketImplNew(const MediaPacketImplNew& other)
    {
        mPayloadSize=other.mPayloadSize;
        mMaxSize=other.mMaxSize;
        mBody.reset(new uint8_t[mMaxSize]);
        memcpy(mBody.get(),other.mBody.get(),mMaxSize);
        mTime=other.mTime;
        mCsrcc = other.mCsrcc;

    }

    MediaPacketImplNew& operator=(const MediaPacketImplNew& other)
    {
        if(&other!=this)
        {
            mPayloadSize=other.mPayloadSize;
            if(mMaxSize<other.mMaxSize)
            {
                mBody.reset(new uint8_t[other.mMaxSize]);
            }
            mMaxSize=other.mMaxSize;
            memcpy(mBody.get(),other.mBody.get(),other.mMaxSize);
            mTime=other.mTime;
            mCsrcc = other.mCsrcc;
        }
        return *this;
    }

    uint64_t getTime() const
    {
        return mTime;
    }

    void setTime(uint64_t ctime)
    {
        mTime=ctime;
    }

    static int getMinSize()
    {   return MediaPacket::HEADER_SIZE+MediaPacket::BUFFER_SIZE;}

    int getMaxSize() const
    {
        return mMaxSize;
    }

    int getSize() const
    {
        return mPayloadSize+MediaPacket::HEADER_SIZE;
    }
    void setSize(int size)
    {
        if(size<MediaPacket::HEADER_SIZE) size=MediaPacket::HEADER_SIZE;
        if(size>mMaxSize) size=mMaxSize;
        mPayloadSize=size-MediaPacket::HEADER_SIZE;
    }

    int getPayloadSize() const
    {
        return mPayloadSize;
    }
    void setPayloadSize(int pl)
    {
        if(pl<0) pl=0;
        else if(pl>mMaxSize-MediaPacket::HEADER_SIZE) pl=mMaxSize-MediaPacket::HEADER_SIZE;
        mPayloadSize=pl;
    }

    uint8_t* getStartPtr()
    {
        return mBody.get();
    }
    uint8_t* getPayloadPtr()
    {
        return mBody.get()+MediaPacket::HEADER_SIZE + MediaPacket::BUFFER_SIZE;
    }
    uint8_t* getLiveDataPtr(){
        return mBody.get()+MediaPacket::BUFFER_SIZE-4*mCsrcc;
    }
    uint32_t getLiveDataSize(){
        return mPayloadSize+MediaPacket::HEADER_SIZE+4*mCsrcc;
    }

    void getLiveRtpHeaderParam(RtpHeaderParam &rtpParam){
        const MediaPacketHeader* packet=(MediaPacketHeader*)(mBody.get()+MediaPacket::BUFFER_SIZE-4*mCsrcc);
        uint32_t var;
        var = ntohl(packet->word1);
        rtpParam.rtp_pl = (0x0000007F &(var>>16));
        rtpParam.rtp_mark = (0x00000001 &(var>>23));
        rtpParam.rtp_csrcc = (0x0000000F &(var>>24));
        rtpParam.rtp_ext = (0x00000001 &(var>>28));
        rtpParam.rtp_pad = (0x00000001 &(var>>29));
        rtpParam.rtp_v = (0x00000003 &(var>>30));
        rtpParam.rtp_sn = (0x0000FFFF & var );
        rtpParam.rtp_ts = ntohl(packet->word2);
        rtpParam.rtp_ssrc = ntohl(packet->word3);

    }

    void setRtpHeaderParam ( RtpHeaderParam &rtpParam )
    {
        MediaPacketHeader* packet=(MediaPacketHeader*)mBody.get();
        uint32_t var;
        var = 0;
        var |= (( 0x00000003 & 2 )<<30); // ver   instead of  rtpParam.rtp_v
        var |= (( 0x00000001 & rtpParam.rtp_pad )<<29);// padding
        var |= (( 0x00000001 & rtpParam.rtp_ext )<<28);// extension
        var |= (( 0x0000000F & rtpParam.rtp_csrcc )<<24);//csrc count
        var |= (( 0x00000001 & rtpParam.rtp_mark)<<23);// mark
        var |= (( 0x0000007F & rtpParam.rtp_pl )<<16);// pl_type
        var |= (( 0x0000FFFF & rtpParam.rtp_sn ));// sn
        mCsrcc = rtpParam.rtp_csrcc;
        if(mCsrcc > 1)
            mCsrcc = 1;

        packet->word1=htonl(var);
        packet->word2=htonl(rtpParam.rtp_ts);
        packet->word3=htonl(rtpParam.rtp_ssrc);

    }

    void getRtpHeaderParam ( RtpHeaderParam &rtpParam ) const
    {
        const MediaPacketHeader* packet=(MediaPacketHeader*)mBody.get();
        uint32_t var;
        var = ntohl(packet->word1);
        rtpParam.rtp_pl = (0x0000007F &(var>>16));
        rtpParam.rtp_mark = (0x00000001 &(var>>23));
        rtpParam.rtp_csrcc = (0x0000000F &(var>>24));
        rtpParam.rtp_ext = (0x00000001 &(var>>28));
        rtpParam.rtp_pad = (0x00000001 &(var>>29));
        rtpParam.rtp_v = (0x00000003 &(var>>30));
        rtpParam.rtp_sn = (0x0000FFFF & var );
        rtpParam.rtp_ts = ntohl(packet->word2);
        rtpParam.rtp_ssrc = ntohl(packet->word3);
    }

    void RtpHeaderParamModify (RtpHeaderModifier &modifier)
    {

        MediaPacketHeader* packet=(MediaPacketHeader*)mBody.get();
        RtpHeaderParam rtpParam;

        uint32_t var;
        var = ntohl(packet->word1);
        rtpParam.rtp_pl = (0x0000007F &(var>>16));
        rtpParam.rtp_mark = (0x00000001 &(var>>23));
        rtpParam.rtp_csrcc = (0x0000000F &(var>>24));
        rtpParam.rtp_ext = (0x00000001 &(var>>28));
        rtpParam.rtp_pad = (0x00000001 &(var>>29));
        rtpParam.rtp_v = (0x00000003 &(var>>30));
        rtpParam.rtp_sn = (0x0000FFFF & var );
        rtpParam.rtp_ts = ntohl(packet->word2);

        if(modifier.rtp_mf_csrcc > 0)
            packet=(MediaPacketHeader*)(mBody.get()+(MediaPacket::BUFFER_SIZE-4));
        else
            packet=(MediaPacketHeader*)(mBody.get()+MediaPacket::BUFFER_SIZE);

        var = 0;
        var |= (( 0x00000003 & 2 )<<30); // ver   instead of  rtpParam.rtp_v
        var |= (( 0x00000001 & rtpParam.rtp_pad )<<29);// padding
        var |= (( 0x00000001 & rtpParam.rtp_ext )<<28);// extension
        var |= (( 0x0000000F & (modifier.rtp_mf_csrcc>0?1:0))<<24);//csrc count
        var |= (( 0x00000001 & rtpParam.rtp_mark)<<23);// mark
        var |= (( 0x0000007F & rtpParam.rtp_pl )<<16);// pl_type
        var |= (( 0x0000FFFF & modifier.rtp_mf_sn));// sn
        mCsrcc = (modifier.rtp_mf_csrcc > 0) ? 1:0;

        packet->word1= htonl(var);
        packet->word2= htonl(modifier.rtp_mf_ts+rtpParam.rtp_ts);
        packet->word3= htonl(modifier.rtp_mf_ssrc);

        if(modifier.rtp_mf_csrcc){
            uint32_t *cscr = (uint32_t *)((uint8_t *)packet+MediaPacket::HEADER_SIZE);
            *cscr = modifier.rtp_mf_csrcc;
        }

    }

private:
    int mPayloadSize;
    int mMaxSize;
    uint64_t mTime;
    boost::scoped_array<uint8_t> mBody;
    uint32_t mCsrcc;
};

/////////////////////////////////////////////////////////

MediaPacket::MediaPacket()
{
    mPimpl.reset(new MediaPacketImplNew());
}

MediaPacket::MediaPacket(int payloadSize)
{
    mPimpl.reset(new MediaPacketImplNew(payloadSize));
}

MediaPacket::~MediaPacket()
{
    ;
}

MediaPacket::MediaPacket(const MediaPacket& other)
{
    mPimpl.reset(new MediaPacketImplNew(*(other.mPimpl.get())));
}


MediaPacket& MediaPacket::operator=(const MediaPacket& other)
{
    if(&other!=this)
    {
        mPimpl.reset(new MediaPacketImplNew(*(other.mPimpl.get())));
    }
    return *this;
}

uint64_t MediaPacket::getTime() const
{
    return mPimpl->getTime();
}

void MediaPacket::setTime(uint64_t ctime)
{
    mPimpl->setTime(ctime);
}

int MediaPacket::getMinSize()
{
    return MediaPacketImplNew::getMinSize();
}

int MediaPacket::getMaxSize() const
{
    return mPimpl->getMaxSize();
}
void MediaPacket::setMaxSize()
{
    setMaxSize(MediaPacket::BODY_MAX_SIZE);
}
void MediaPacket::setMaxSize(int size)
{
    if(mPimpl->getMaxSize()<size)
    {
        mPimpl.reset(new MediaPacketImplNew(size));
    }
}

int MediaPacket::getMaxPayloadSize() const
{
    return getMaxSize()-MediaPacket::HEADER_SIZE;
}
void MediaPacket::setMaxPayloadSize()
{
    setMaxPayloadSize(MediaPacket::BODY_MAX_SIZE-MediaPacket::HEADER_SIZE);
}
void MediaPacket::setMaxPayloadSize(int pl)
{
    setMaxSize(pl+MediaPacket::HEADER_SIZE);
}

int MediaPacket::getSize() const
{
    return mPimpl->getSize();
}
void MediaPacket::setSize(int size)
{
    mPimpl->setSize(size);
}

int MediaPacket::getPayloadSize() const
{
    return mPimpl->getPayloadSize();
}
void MediaPacket::setPayloadSize(int pl)
{
    mPimpl->setPayloadSize(pl);
}

uint8_t* MediaPacket::getStartPtr()
{
    return mPimpl->getStartPtr();
}

uint8_t* MediaPacket::getPayloadPtr()
{
    return mPimpl->getPayloadPtr();
}

bool MediaPacket::operator<(const MediaPacket& other) const
{
    return VoIPUtils::time_before64(getTime(),other.getTime());
}
void MediaPacket::setRtpHeaderParam ( RtpHeaderParam &rtpParam )
{
    mPimpl->setRtpHeaderParam(rtpParam);
}

void MediaPacket::getRtpHeaderParam ( RtpHeaderParam &rtpParam ) const
{
    mPimpl->getRtpHeaderParam(rtpParam);
}

void MediaPacket::RtpHeaderParamModify(RtpHeaderModifier &modifier){
    mPimpl->RtpHeaderParamModify(modifier);
}
uint8_t* MediaPacket::getLiveDataPtr(){
    return mPimpl->getLiveDataPtr();
}
uint32_t MediaPacket::getLiveDataSize(){
    return mPimpl->getLiveDataSize();
}
void MediaPacket::getLiveRtpHeaderParam(RtpHeaderParam &rtpParam) const
{
    mPimpl->getLiveRtpHeaderParam(rtpParam);
}

///////////////////////////////////////////////////////////////////////////////

struct MediaStreamData
{

    void init()
    {

        mCodec=CODEC_UNDEFINED;
        mPayloadNumber=PAYLOAD_UNDEFINED;
        mDefaultPacketLength=0;
        mBitRate=0;
        mSamplingRate=0;
        mFilePath=-1;
        mCodecGroup = CODEC_GROUP_UNDEFINED;        
#if defined(UNIT_TEST)
		mEncodedDirPath.assign("/tmp/");
        mFakeEncodedStream = true;
#else
		mEncodedDirPath.assign("/mnt/spirent/ccpu/lib/audio/enc/");
        mFakeEncodedStream = false;
#endif
        setValidMediaData();
    }

    MediaStreamData()
    {
        init();
    }

    MediaStreamData(const MediaStreamData *other)
    {
        if(other)
        {
            mCodec=other->mCodec;
            mPayloadNumber=other->mPayloadNumber;
            mDefaultPacketLength=other->mDefaultPacketLength;
            mBitRate=other->mBitRate;
            mSamplingRate=other->mSamplingRate;
            mFilePath=other->mFilePath;
            mCodecGroup = other->mCodecGroup;
            mEncodedDirPath = other->mEncodedDirPath;
            mFakeEncodedStream = other->mFakeEncodedStream;
        }
        else
        {
            init();
        }
    }

    MediaStreamData(const MediaStreamData& other)
    {
        mCodec=other.mCodec;
        mPayloadNumber=other.mPayloadNumber;
        mDefaultPacketLength=other.mDefaultPacketLength;
        mBitRate=other.mBitRate;
        mSamplingRate=other.mSamplingRate;
        mFilePath=other.mFilePath;
        mCodecGroup = other.mCodecGroup;
        mEncodedDirPath = other.mEncodedDirPath;
        mFakeEncodedStream = other.mFakeEncodedStream;
    }

    MediaStreamData& operator=(const MediaStreamData& other)
    {
        if(&other!=this)
        {
            mCodec=other.mCodec;
            mPayloadNumber=other.mPayloadNumber;
            mDefaultPacketLength=other.mDefaultPacketLength;
            mBitRate=other.mBitRate;
            mSamplingRate=other.mSamplingRate;
            mFilePath=other.mFilePath;
            mCodecGroup = other.mCodecGroup;
            mEncodedDirPath = other.mEncodedDirPath;
            mFakeEncodedStream = other.mFakeEncodedStream;
        }
        return *this;
    }

    bool operator<(const MediaStreamData& other) const
    {
        if(this==&other) return false;
        if(mCodec<other.mCodec) return true;
        if(mCodec>other.mCodec) return false;
        if(mPayloadNumber<other.mPayloadNumber) return true;
        if(mPayloadNumber>other.mPayloadNumber) return false;
        if(mDefaultPacketLength<other.mDefaultPacketLength) return true;
        if(mDefaultPacketLength>other.mDefaultPacketLength) return false;
        if(mBitRate<other.mBitRate) return true;
        if(mBitRate>other.mBitRate) return false;
        if(mSamplingRate<other.mSamplingRate) return true;
        if(mSamplingRate>other.mSamplingRate) return false;
        if(mFilePath<other.mFilePath) return true;
        if(mFilePath>other.mFilePath) return false;
        if(mCodecGroup<other.mCodecGroup) return true;
        if(mCodecGroup>other.mCodecGroup) return false;
        if(mEncodedDirPath<other.mEncodedDirPath) return true;
        if(mEncodedDirPath>other.mEncodedDirPath) return false;
        if(mFakeEncodedStream<other.mFakeEncodedStream) return true;
        if(mFakeEncodedStream>other.mFakeEncodedStream) return false;
        return false;
    }

    bool operator>(const MediaStreamData& other) const
    {
        if(this==&other) return false;
        return other<*this;
    }

    bool operator==(const MediaStreamData& other) const
    {
        if(this==&other) return true;
        return (mCodec==other.mCodec &&
                mPayloadNumber==other.mPayloadNumber &&
                mDefaultPacketLength==other.mDefaultPacketLength &&
                mBitRate==other.mBitRate &&
                mSamplingRate==other.mSamplingRate &&
                mFilePath==other.mFilePath &&
                mCodecGroup == other.mCodecGroup &&
                mEncodedDirPath == other.mEncodedDirPath &&
                mFakeEncodedStream == other.mFakeEncodedStream);
    }

    bool operator!=(const MediaStreamData& other) const
    {
        if(this==&other) return false;
        return !(*this == other);
    }

    void setValidMediaData()
    {

        if(mCodec==CODEC_UNDEFINED)
        {
            switch (mPayloadNumber)
            {
                case PAYLOAD_711u:
                mCodec=G711u;
                break;

                case PAYLOAD_711A:
                mCodec=G711A;
                break;

                case PAYLOAD_726:
                mCodec=G726;
                break;

                case PAYLOAD_723:
                mCodec=G723_1;
                break;

                case PAYLOAD_729:
                mCodec=G729AB;
                break;

                case PAYLOAD_728:
                mCodec=G728;
                break;

                case PAYLOAD_722:
                mCodec=CODEC_TYPE_G722;
                break;

                case VPAYLOAD_263:
                mCodec=VH263;
                break;

                default:
                mCodec=G711A;
                mPayloadNumber=PAYLOAD_711A;
            }
        }

        if(mPayloadNumber==PAYLOAD_UNDEFINED)
        {
            switch (mCodec)
            {
                case G711A:  mPayloadNumber=PAYLOAD_711A;
                break;

                case G711u:
                mPayloadNumber=PAYLOAD_711u;
                break;

                case G726:
                case G726ITU:
                mPayloadNumber=PAYLOAD_726;
                break;

                case CODEC_TYPE_G722:
                case CODEC_TYPE_G722_1_32:
                case CODEC_TYPE_G722_1_24:
                mPayloadNumber=PAYLOAD_722;
                break;

                case G723_1:
                mPayloadNumber=PAYLOAD_723;
                break;

                case G729AB:
                case G729A:
                case G729B:
                case G729:
                mPayloadNumber=PAYLOAD_729;
                break;

                case G728:
                mPayloadNumber=PAYLOAD_728;
                break;

                case VH263:
                mPayloadNumber=VPAYLOAD_263;
                break;

                default:
                mCodec=G711A;
                mPayloadNumber=PAYLOAD_711A;
            }
        }

        switch (mCodec)
        {
            case G711A:
            mPayloadNumber=PAYLOAD_711A;
            if(mBitRate<1) mBitRate=64000;
            if(mDefaultPacketLength<1) mDefaultPacketLength=20;
            if(mSamplingRate<1) mSamplingRate=8;
            break;

            case G711u:
            mPayloadNumber=PAYLOAD_711u;
            if(mBitRate<1) mBitRate=64000;
            if(mDefaultPacketLength<1) mDefaultPacketLength=20;
            if(mSamplingRate<1) mSamplingRate=8;
            break;

            case G726:
            case G726ITU:
            if(mBitRate<1) mBitRate=32000;
            if(mDefaultPacketLength<1) mDefaultPacketLength=20;
            if(mSamplingRate<1) mSamplingRate=8;
            break;

            case CODEC_TYPE_G722:
            mPayloadNumber=PAYLOAD_722;
            if(mBitRate<1) mBitRate=64000;
            if(mDefaultPacketLength<1) mDefaultPacketLength=20;
            if(mSamplingRate<1) mSamplingRate=16;
            break;

            case CODEC_TYPE_G722_1_32:
            mPayloadNumber=PAYLOAD_722;
            if(mBitRate<1) mBitRate=32000;
            if(mDefaultPacketLength<1) mDefaultPacketLength=20;
            if(mSamplingRate<1) mSamplingRate=16;
            break;

            case CODEC_TYPE_G722_1_24:
            mPayloadNumber=PAYLOAD_722;
            if(mBitRate<1) mBitRate=24000;
            if(mDefaultPacketLength<1) mDefaultPacketLength=20;
            if(mSamplingRate<1) mSamplingRate=16;
            break;

            case G723_1:
            mPayloadNumber=PAYLOAD_723;
            if(mBitRate<1) mBitRate=6300;
            mDefaultPacketLength=30;
            if(mSamplingRate<1) mSamplingRate=8;
            break;

            case G729AB:
            case G729A:
            case G729B:
            case G729:
            mPayloadNumber=PAYLOAD_729;
            if(mBitRate<1) mBitRate=8000;
            if(mDefaultPacketLength<1) mDefaultPacketLength=20;
            if(mSamplingRate<1) mSamplingRate=8;
            break;

            case G728:
            mPayloadNumber=PAYLOAD_728;
            if(mBitRate<1) mBitRate=16000;
            if(mDefaultPacketLength<1) mDefaultPacketLength=20;
            if(mSamplingRate<1) mSamplingRate=8;
            break;

            case VH263:
            mPayloadNumber=VPAYLOAD_263;
            if(mSamplingRate<1) mSamplingRate=90;
            break;

            case CODEC_TYPE_G711_1u: break;
            case CODEC_TYPE_G711_1A:break;
            case AAC_LD: break;
            case VH264: break;
            case VH264_CTS_720P: break;
            case VH264_CTS_1080P: break;
            case VH264_TBG_720P: break;
            case VH264_TBG_1080P: break;
            case VH264_TBG_XGA: break;
            case VH264_TBG_CIF: break;

            case AMRNB_0_OA_475: break;
            case AMRNB_1_OA_515: break;
            case AMRNB_2_OA_590: break;
            case AMRNB_3_OA_670: break;
            case AMRNB_4_OA_740: break;
            case AMRNB_5_OA_795: break;
            case AMRNB_6_OA_102: break;
            case AMRNB_7_OA_122: break;

            case AMRNB_0_BE_475: break;
            case AMRNB_1_BE_515: break;
            case AMRNB_2_BE_590: break;
            case AMRNB_3_BE_670: break;
            case AMRNB_4_BE_740: break;
            case AMRNB_5_BE_795: break;
            case AMRNB_6_BE_102: break;
            case AMRNB_7_BE_122: break;

            case AMRWB_0_OA_660: break;
            case AMRWB_1_OA_885: break;
            case AMRWB_2_OA_1265: break;
            case AMRWB_3_OA_1425: break;
            case AMRWB_4_OA_1585: break;
            case AMRWB_5_OA_1825: break;
            case AMRWB_6_OA_1985: break;
            case AMRWB_7_OA_2305: break;
            case AMRWB_8_OA_2385: break;

            case AMRWB_0_BE_660: break;
            case AMRWB_1_BE_885: break;
            case AMRWB_2_BE_1265: break;
            case AMRWB_3_BE_1425: break;
            case AMRWB_4_BE_1585: break;
            case AMRWB_5_BE_1825: break;
            case AMRWB_6_BE_1985: break;
            case AMRWB_7_BE_2305: break;
            case AMRWB_8_BE_2385: break;

            default:
            mCodec=G711A;
            mPayloadNumber=PAYLOAD_711A;
            if(mBitRate<1) mBitRate=64000;
            if(mDefaultPacketLength<1) mDefaultPacketLength=20;
            if(mSamplingRate<1) mSamplingRate=8;
        }

        switch (mPayloadNumber)
        {
            case PAYLOAD_711u:
            mCodec=G711u;
            break;
            case PAYLOAD_711A:
            mCodec=G711A;
            break;
            case PAYLOAD_726:
            if(mCodec!=G726ITU) mCodec=G726;
            break;
            case PAYLOAD_723:
            mCodec=G723_1;
            break;
            case PAYLOAD_729:
            if(mCodec!=G729AB && mCodec!=G729B && mCodec!=G729A && mCodec!=G729) mCodec=G729AB;
            break;
            case PAYLOAD_728:
            mCodec=G728;
            break;
            case PAYLOAD_722:
            if(mCodec!=CODEC_TYPE_G722_1_32 && mCodec!=CODEC_TYPE_G722_1_24) mCodec=CODEC_TYPE_G722;
            case VPAYLOAD_263:
            mCodec=VH263;
            break;
            default:
            if( mPayloadNumber < 96 || mPayloadNumber > 127 )
            {
                mCodec=G711A;
                mPayloadNumber=PAYLOAD_711A;
            }
        }

        mFilePath = mEncodedDirPath;
        switch (mCodec)
        {
            case G711A:
            mFilePath += "OSR_uk_000_0051_8k_cut_G711A_64.enc";
            break;
            case G711u:
            mFilePath += "OSR_uk_000_0051_8k_cut_G711MU_64.enc";
            break;
            case G726:
            if(mBitRate ==32000 )
            mFilePath += "OSR_uk_000_0051_8k_cut_G726_32.enc";
            break;
            case G723_1:
            if(mBitRate ==6300 )
            mFilePath += "OSR_uk_000_0051_8k_cut_G723_6_3.enc";
            else
            mFilePath += "OSR_uk_000_0051_8k_cut_G723_5_3.enc";
            break;
            case G729AB:
            mFilePath += "OSR_uk_000_0051_8k_cut_G729AB_8.enc";
            break;
            case G729A:
            mFilePath += "OSR_uk_000_0051_8k_cut_G729A_8.enc";
            break;
            case G729B:
            mFilePath += "OSR_uk_000_0051_8k_cut_G729B_8.enc";
            break;
            case G729:
            mFilePath += "OSR_uk_000_0051_8k_cut_G729_8.enc";
            break;

            case VH264:
            mFilePath += "VH264video.rtp";
            break;
            case VH264_CTS_720P:
            mFilePath += "VH264video.rtp";
            break;
            case VH264_CTS_1080P:
            mFilePath += "VH264_1080P_video.rtp";
            break;
            case VH264_TBG_720P:
            mFilePath += "VH264_TBG_720P_video.rtp";
            break;
            case VH264_TBG_1080P:
            mFilePath += "VH264_TBG_1080P_video.rtp";
            break;
            case VH264_TBG_CIF:
            mFilePath += "VH264_TBG_CIF_video.rtp";
            break;
            case VH264_TBG_XGA:
            mFilePath += "VH264_TBG_XGA_video.rtp";
            break;            
            case AAC_LD:
            mFilePath += "AACLDaudio.rtp";
            break;
            case AMRNB_0_OA_475: mFilePath += "amr_nb_oa_0.rtp"; break;
            case AMRNB_1_OA_515: mFilePath += "amr_nb_oa_1.rtp"; break;
            case AMRNB_2_OA_590: mFilePath += "amr_nb_oa_2.rtp"; break;
            case AMRNB_3_OA_670: mFilePath += "amr_nb_oa_3.rtp"; break;
            case AMRNB_4_OA_740: mFilePath += "amr_nb_oa_4.rtp"; break;
            case AMRNB_5_OA_795: mFilePath += "amr_nb_oa_5.rtp"; break;
            case AMRNB_6_OA_102: mFilePath += "amr_nb_oa_6.rtp"; break;
            case AMRNB_7_OA_122: mFilePath += "amr_nb_oa_7.rtp"; break;

            case AMRNB_0_BE_475: mFilePath += "amr_nb_be_0.rtp"; break;
            case AMRNB_1_BE_515: mFilePath += "amr_nb_be_1.rtp"; break;
            case AMRNB_2_BE_590: mFilePath += "amr_nb_be_2.rtp"; break;
            case AMRNB_3_BE_670: mFilePath += "amr_nb_be_3.rtp"; break;
            case AMRNB_4_BE_740: mFilePath += "amr_nb_be_4.rtp"; break;
            case AMRNB_5_BE_795: mFilePath += "amr_nb_be_5.rtp"; break;
            case AMRNB_6_BE_102: mFilePath += "amr_nb_be_6.rtp"; break;
            case AMRNB_7_BE_122: mFilePath += "amr_nb_be_7.rtp"; break;

            case AMRWB_0_OA_660: mFilePath += "amr_wb_oa_0.rtp"; break;
            case AMRWB_1_OA_885: mFilePath += "amr_wb_oa_1.rtp"; break;
            case AMRWB_2_OA_1265: mFilePath += "amr_wb_oa_2.rtp"; break;
            case AMRWB_3_OA_1425: mFilePath += "amr_wb_oa_3.rtp"; break;
            case AMRWB_4_OA_1585: mFilePath += "amr_wb_oa_4.rtp"; break;
            case AMRWB_5_OA_1825: mFilePath += "amr_wb_oa_5.rtp"; break;
            case AMRWB_6_OA_1985: mFilePath += "amr_wb_oa_6.rtp"; break;
            case AMRWB_7_OA_2305: mFilePath += "amr_wb_oa_7.rtp"; break;
            case AMRWB_8_OA_2385: mFilePath += "amr_wb_oa_8.rtp"; break;

            case AMRWB_0_BE_660: mFilePath += "amr_wb_be_0.rtp"; break;
            case AMRWB_1_BE_885: mFilePath += "amr_wb_be_1.rtp"; break;
            case AMRWB_2_BE_1265: mFilePath += "amr_wb_be_2.rtp"; break;
            case AMRWB_3_BE_1425: mFilePath += "amr_wb_be_3.rtp"; break;
            case AMRWB_4_BE_1585: mFilePath += "amr_wb_be_4.rtp"; break;
            case AMRWB_5_BE_1825: mFilePath += "amr_wb_be_5.rtp"; break;
            case AMRWB_6_BE_1985: mFilePath += "amr_wb_be_6.rtp"; break;
            case AMRWB_7_BE_2305: mFilePath += "amr_wb_be_7.rtp"; break;
            case AMRWB_8_BE_2385: mFilePath += "amr_wb_be_8.rtp"; break;

            default:
            break;
        }

        mCodecGroup = CODEC_GROUP_UNDEFINED;
        switch ( mCodec )
        {
            case G711u:
            case G711A:
            mCodecGroup = CODEC_GROUP_G711;
            break;
            case G723_1:
            mCodecGroup = CODEC_GROUP_G723;
            break;
            case G726:
            mCodecGroup = CODEC_GROUP_G726;
            break;
            case G729:
            case G729A:
            case G729B:
            case G729AB:
            mCodecGroup = CODEC_GROUP_G729;
            break;
            case CODEC_TYPE_G711_1u:
            case CODEC_TYPE_G711_1A:
            mCodecGroup = CODEC_GROUP_G711_1;
            break;
            case VH264:
            case VH264_CTS_720P:
            case VH264_CTS_1080P:
            case VH264_TBG_720P:
            case VH264_TBG_1080P:
            case VH264_TBG_CIF:
            case VH264_TBG_XGA:
            mCodecGroup = CODEC_GROUP_H264;
            break;

            case AAC_LD:
            mCodecGroup = CODEC_GROUP_CUSTOM;
            break;

            case AMRNB_0_OA_475:
            case AMRNB_1_OA_515:
            case AMRNB_2_OA_590:
            case AMRNB_3_OA_670:
            case AMRNB_4_OA_740:
            case AMRNB_5_OA_795:
            case AMRNB_6_OA_102:
            case AMRNB_7_OA_122:

            case AMRNB_0_BE_475:
            case AMRNB_1_BE_515:
            case AMRNB_2_BE_590:
            case AMRNB_3_BE_670:
            case AMRNB_4_BE_740:
            case AMRNB_5_BE_795:
            case AMRNB_6_BE_102:
            case AMRNB_7_BE_122:

            case AMRWB_0_OA_660:
            case AMRWB_1_OA_885:
            case AMRWB_2_OA_1265:
            case AMRWB_3_OA_1425:
            case AMRWB_4_OA_1585:
            case AMRWB_5_OA_1825:
            case AMRWB_6_OA_1985:
            case AMRWB_7_OA_2305:
            case AMRWB_8_OA_2385:

            case AMRWB_0_BE_660:
            case AMRWB_1_BE_885:
            case AMRWB_2_BE_1265:
            case AMRWB_3_BE_1425:
            case AMRWB_4_BE_1585:
            case AMRWB_5_BE_1825:
            case AMRWB_6_BE_1985:
            case AMRWB_7_BE_2305:
            case AMRWB_8_BE_2385:

            mCodecGroup = CODEC_GROUP_CUSTOM;
            break;

            default : mCodecGroup = CODEC_GROUP_UNDEFINED;
        }

    }

    CodecType mCodec;
    CodecGroupType mCodecGroup;
    PayloadType mPayloadNumber;
    int mDefaultPacketLength;
    int mBitRate;
    int mSamplingRate;
    MediaFilePath mFilePath;
    MediaFilePath mEncodedDirPath;
    bool mFakeEncodedStream;
};

///////////////////////////////////////////////////////////////

MediaStreamInfo::MediaStreamInfo()
{
    mData.reset(new MediaStreamData);
}

MediaStreamInfo::MediaStreamInfo(const MediaStreamInfo* other)
{
    if(other)
    {
        mData.reset(new MediaStreamData(other->mData.get()));
    }
}

MediaStreamInfo::MediaStreamInfo(const MediaStreamInfo& other)
{
    mData.reset(new MediaStreamData(other.mData.get()));
}

MediaStreamInfo& MediaStreamInfo::operator=(const MediaStreamInfo& other)
{
    if(&other!=this)
    {
        mData.reset(new MediaStreamData(other.mData.get()));
    }
    return *this;
}

MediaStreamInfo::~MediaStreamInfo()
{
}

void MediaStreamInfo::init()
{   mData->init();}
CodecType MediaStreamInfo::getCodec() const
{   return mData->mCodec;}
CodecGroupType MediaStreamInfo::getCodecGroup() const
{   return mData->mCodecGroup;}
void MediaStreamInfo::setCodecAndPayloadNumber(CodecType codec,PayloadType pn)
{
    mData->mCodec=codec;
    mData->mPayloadNumber = pn;
    mData->setValidMediaData();
}
PayloadType MediaStreamInfo::getPayloadNumber() const
{   return mData->mPayloadNumber;}
int MediaStreamInfo::getDefaultPacketLength() const
{   return mData->mDefaultPacketLength;}
void MediaStreamInfo::setDefaultPacketLength(int ms)
{
    mData->mDefaultPacketLength=ms;
    mData->setValidMediaData();
}
int MediaStreamInfo::getBitRate() const
{   return mData->mBitRate;}
void MediaStreamInfo::setBitRate(int br)
{
    mData->mBitRate=br;
    mData->setValidMediaData();
}
int MediaStreamInfo::getSamplingRate() const
{   return mData->mSamplingRate;}
void MediaStreamInfo::setSamplingRate(int sr)
{
    mData->mSamplingRate=sr;
    mData->setValidMediaData();
}
MediaFilePath MediaStreamInfo::getFilePath() const
{   return mData->mFilePath;}
void MediaStreamInfo::setFilePath(MediaFilePath filePath)
{
    mData->mFilePath=filePath;
    mData->setValidMediaData();
}
bool MediaStreamInfo::getFakeEncStreamGeneration() const
{   return mData->mFakeEncodedStream;}
void MediaStreamInfo::setFakeEncStreamGeneration (bool FakeEncodedStream)
{
    mData->mFakeEncodedStream=FakeEncodedStream;
}

bool MediaStreamInfo::operator<(const MediaStreamInfo& other) const
{
    if(this==&other) return false;
    return *(mData.get())<*(other.mData.get());
}
bool MediaStreamInfo::operator>(const MediaStreamInfo& other) const
{
    if(this==&other) return false;
    return other<*this;
}
bool MediaStreamInfo::operator==(const MediaStreamInfo& other) const
{
    if(this==&other) return true;
    return *(mData.get()) == *(other.mData.get());
}
bool MediaStreamInfo::operator!=(const MediaStreamInfo& other) const
{
    if(this==&other) return false;
    return *(mData.get()) != *(other.mData.get());
}

///////////////////////////////////////////////////////////////////////////////

END_DECL_MEDIAFILES_NS

////////////////////////////////////////////////////////////////////////////////

