/// @file
/// @brief HTTP ABR Playlist implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include "AbrPlaylist.h"

USING_ABR_NS;

///////////////////////////////////////////////////////////////////////////////

AbrPlaylist::AbrPlaylist(uint8_t serverVersion,
                uint8_t targetDuration,
                uint8_t streamType,
                uint8_t windowType,
                uint16_t mediaSequenceNumber,
                uint16_t maxSlidingWindowSize,
                uint16_t videoLength,
                ACE_Time_Value startTime,
                videoServerBitrateList_t bitrateList)
    :   mVideoServerVersion(serverVersion),
        mVideoTargetDuration(targetDuration),
        mVideoStreamType(streamType),
        mWindowType(windowType),
        mMediaSequenceNumber(mediaSequenceNumber),
        mMaxSlidingWindowSize(maxSlidingWindowSize),
        mServerVideoLength(videoLength),
        mSessionStartTime(startTime),
        mBitrateList(bitrateList)
{
}

///////////////////////////////////////////////////////////////////////////////

AbrPlaylist::~AbrPlaylist()
{
}

///////////////////////////////////////////////////////////////////////////////
