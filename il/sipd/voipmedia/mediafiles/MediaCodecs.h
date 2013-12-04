/// @file
/// @brief Media Files Common defs
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _MEDIAFILES_CODECS_H_
#define _MEDIAFILES_CODECS_H_

#include "MediaFilesCommon.h"

DECL_MEDIAFILES_NS

#define  AAC_LD_PT         96
#define  AAC_LD_BR         64000
#define  VH264_PT          112
#define  VH264_BR          300000

enum CODEC_NAMES
{
    CODEC_UNDEFINED = -1,

    FIRST_VOICE_CODEC=0,

    G711u=FIRST_VOICE_CODEC,
    G711A,
    G723_1,
    G729,
    G729A,
    G729B,
    G729AB,
    G726,
    G726ITU,
    G728,
    CODEC_TYPE_AMR,
    CODEC_TYPE_AMR_WB,
    CODEC_TYPE_EFR,
    EVRC_HEADERFREE,
    EVRC_INTERLEAVED,
    EVRCB_HEADERFREE,
    EVRCB_INTERLEAVED,
    CODEC_TYPE_ILBC20,
    CODEC_TYPE_ILBC30,
    CODEC_TYPE_GSM_EFR,
    CODEC_TYPE_GSM_FR,
    CODEC_TYPE_GSM_HR,
    CODEC_TYPE_G722,
    CODEC_TYPE_G722_1_32,
    CODEC_TYPE_G722_1_24,
    CODEC_TYPE_G711_1u,
    CODEC_TYPE_G711_1A,

    AMRNB_0_OA_475,
    AMRNB_1_OA_515,
    AMRNB_2_OA_590,
    AMRNB_3_OA_670,
    AMRNB_4_OA_740,
    AMRNB_5_OA_795,
    AMRNB_6_OA_102,
    AMRNB_7_OA_122,

    AMRNB_0_BE_475,
    AMRNB_1_BE_515,
    AMRNB_2_BE_590,
    AMRNB_3_BE_670,
    AMRNB_4_BE_740,
    AMRNB_5_BE_795,
    AMRNB_6_BE_102,
    AMRNB_7_BE_122,

    AMRWB_0_OA_660,
    AMRWB_1_OA_885,
    AMRWB_2_OA_1265,
    AMRWB_3_OA_1425,
    AMRWB_4_OA_1585,
    AMRWB_5_OA_1825,
    AMRWB_6_OA_1985,
    AMRWB_7_OA_2305,
    AMRWB_8_OA_2385,

    AMRWB_0_BE_660,
    AMRWB_1_BE_885,
    AMRWB_2_BE_1265,
    AMRWB_3_BE_1425,
    AMRWB_4_BE_1585,
    AMRWB_5_BE_1825,
    AMRWB_6_BE_1985,
    AMRWB_7_BE_2305,
    AMRWB_8_BE_2385,

//    LAST_VOICE_CODEC=CODEC_TYPE_G711_1A,
    LAST_VOICE_CODEC=AMRWB_8_BE_2385,

    FIRST_VIDEO_CODEC,

    VH263=FIRST_VIDEO_CODEC,
    VH264,

    VH264_CTS_720P,
    VH264_CTS_1080P,
    VH264_TBG_720P,
    VH264_TBG_1080P,
    VH264_TBG_CIF,
    VH264_TBG_XGA,

    MP4V_ES,

    LAST_VIDEO_CODEC=MP4V_ES,

    FIRST_AUDIOHD_CODEC,

    AAC_LD=FIRST_AUDIOHD_CODEC,

    LAST_AUDIOHD_CODEC=AAC_LD,

    CODEC_TOTAL
};

enum CODEC_GROUP_NAMES
{
  CODEC_GROUP_UNDEFINED = -1,
  CODEC_GROUP_G711,
  CODEC_GROUP_G723,
  CODEC_GROUP_G726,
  CODEC_GROUP_G729,
  CODEC_GROUP_G711_1,
  CODEC_GROUP_CUSTOM,
  CODEC_GROUP_H264,
  CODEC_GROUP_TOTAL
};



END_DECL_MEDIAFILES_NS

#endif //_MEDIAFILES_CODECS_H_

