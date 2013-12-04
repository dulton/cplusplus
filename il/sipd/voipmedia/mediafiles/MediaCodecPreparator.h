/// @file
/// @brief MediaFiles processing classes
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _MEDIAFILES_PREPARATOR_H_
#define _MEDIAFILES_PREPARATOR_H_

#include <boost/utility.hpp>

#include "MediaFilesCommon.h"
#include "MediaCodecs.h"

DECL_MEDIAFILES_NS

//////////////////////////////////////////////////////////////

class CollectionOfEncodedFiles;

class MediaCodecPreparator : public boost::noncopyable {
  
 public:
  MediaCodecPreparator() {}
  virtual ~MediaCodecPreparator() {}
  virtual CodecGroupType getCodecGroup() = 0;
  virtual EncodedMediaStreamIndex prepareEncodedStream(const MediaStreamInfo *streamInfo, 
						       CollectionOfEncodedFiles* filesCollection) = 0;
  void addPacket(EncodedMediaFile *file, MediaPacketHandler packet);
  uint32_t getRawStreamSize (const MediaStreamInfo *streamInfo);
  uint32_t getRawData(const MediaStreamInfo *streamInfo, uint8_t *pEnc,  uint32_t size );
protected:
  static const uint32_t RAWDATA_HEADER_SIZE  =  44;
};

class MediaCodecPreparator_CUSTOM : public MediaCodecPreparator
{
public:
  virtual ~MediaCodecPreparator_CUSTOM() {}
    EncodedMediaStreamIndex prepareEncodedStream(const MediaStreamInfo *streamInfo,
        CollectionOfEncodedFiles* filesCollection);
    CodecGroupType getCodecGroup(){ return CODEC_GROUP_CUSTOM; }

    static const int MAXSIZE_OF_RTPPACKET =  4096;
    static const uint32_t FAKE_RTP_SIZE = 512;
    static const uint32_t FAKE_PACKET_COUNT = 5000;
    static const uint32_t FAKE_PACKET_TRANSMIT_TIME = 5000;
private:
};

class MediaCodecPreparator_H264 : public MediaCodecPreparator
{
public:
  virtual ~MediaCodecPreparator_H264() {}
    EncodedMediaStreamIndex prepareEncodedStream(const MediaStreamInfo *streamInfo,
        CollectionOfEncodedFiles* filesCollection);
    CodecGroupType getCodecGroup(){ return CODEC_GROUP_H264; }

    static const int MAXSIZE_OF_RTPPACKET =  4096;
    static const uint32_t FAKE_RTP_SIZE = 512;
    static const uint32_t FAKE_PACKET_COUNT = 5000;
    static const uint32_t FAKE_PACKET_TRANSMIT_TIME = 5000;
};

class MediaCodecPreparator_G711 : public MediaCodecPreparator
{
public:
  virtual ~MediaCodecPreparator_G711() {}
    EncodedMediaStreamIndex prepareEncodedStream(const MediaStreamInfo *streamInfo,
        CollectionOfEncodedFiles* filesCollection);
    CodecGroupType getCodecGroup(){ return CODEC_GROUP_G711; }
private:
    static const int VOICE_FRAME_LTH_SMPL_G711uA  =  80;
    static const int SPEECH_CODED_FRAME_LTH_G711uA = 80;
    static const int MAX_FRAMES_IN_PACKET_G711uA   = 20;
    static const int SMPL_RATE_G711uA              =  8;
};

class MediaCodecPreparator_G723 : public MediaCodecPreparator
{
public:
  virtual ~MediaCodecPreparator_G723() {}
    EncodedMediaStreamIndex prepareEncodedStream(const MediaStreamInfo *streamInfo,
        CollectionOfEncodedFiles* filesCollection);
    CodecGroupType getCodecGroup(){ return CODEC_GROUP_G723; }

private:
    static const int VOICE_FRAME_LTH_SMPL_G723         =  240;
    static const int SPEECH_CODED_FRAME_LTH_G723_5_3   =   20;
    static const int SPEECH_CODED_FRAME_LTH_G723_6_3   =   24;
    static const int SID_CODED_FRAME_LTH_G723          =    4;
    static const int MAX_FRAMES_IN_PACKET_G723         =    6;
    static const int SMPL_RATE_G723                    =    8;

};

class MediaCodecPreparator_G726 : public MediaCodecPreparator
{
public:
  virtual ~MediaCodecPreparator_G726() {}
    EncodedMediaStreamIndex prepareEncodedStream(const MediaStreamInfo *streamInfo,
        CollectionOfEncodedFiles* filesCollection);
    CodecGroupType getCodecGroup(){ return CODEC_GROUP_G726; }
private:
    static const int VOICE_FRAME_LTH_SMPL_G726     =  80;
    static const int SPEECH_CODED_FRAME_LTH_G726_16 = 20;
    static const int SPEECH_CODED_FRAME_LTH_G726_24 = 30;
    static const int SPEECH_CODED_FRAME_LTH_G726_32 = 40;
    static const int SPEECH_CODED_FRAME_LTH_G726_40 = 50;
    static const int MAX_FRAMES_IN_PACKET_G726      = 20;
    static const int SMPL_RATE_G726                 =  8;

};

class MediaCodecPreparator_G729 : public MediaCodecPreparator
{
public:
  virtual ~MediaCodecPreparator_G729() {}
    EncodedMediaStreamIndex prepareEncodedStream(const MediaStreamInfo *streamInfo,
        CollectionOfEncodedFiles* filesCollection);
    CodecGroupType getCodecGroup(){ return CODEC_GROUP_G729; }
private:
    static const int VOICE_FRAME_LTH_SMPL_G729    =  80;
    static const int SPEECH_CODED_FRAME_LTH_G729   = 10;
    static const int SID_CODED_FRAME_LTH_G729      =  2;
    static const int MAX_FRAMES_IN_PACKET_G729     = 20;
    static const int SMPL_RATE_G729                =  8;

};

class MediaCodecPreparator_G711_1 : public MediaCodecPreparator
{
public:
  virtual ~MediaCodecPreparator_G711_1() {}
    EncodedMediaStreamIndex prepareEncodedStream(const MediaStreamInfo *streamInfo,
        CollectionOfEncodedFiles* filesCollection);
    CodecGroupType getCodecGroup(){ return CODEC_GROUP_G711_1; }
private:
    static const int VOICE_FRAME_LTH_SMPL_G711_1uA         =  80;
    static const int G711_1uA_FRAME_LTH_L0                 =  40;
    static const int G711_1uA_FRAME_LTH_L1                 =  10;
    static const int G711_1uA_FRAME_LTH_L2                 =  10;
    static const uint8_t G711_1uA_MODE_INDEX_R1            =   1;
    static const uint8_t G711_1uA_MODE_INDEX_R2a           =   2;
    static const uint8_t G711_1uA_MODE_INDEX_R2b           =   3;
    static const uint8_t G711_1uA_MODE_INDEX_R3            =   4;
    static const int MAX_FRAMES_IN_PACKET_G711_1uA         =  20;
    static const int SMPL_RATE_G711_1uA                    =  16;
};


///////////////////////////////////////////////////////////////////////////////

END_DECL_MEDIAFILES_NS

#endif //_MEDIAFILES_PREPARATOR_H_

