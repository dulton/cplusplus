/// @file
/// @brief HTTP HLS ABR Playlist header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _HLS_ABR_PLAYLIST_H_
#define _HLS_ABR_PLAYLIST_H_

#include "AbrPlaylist.h"

BEGIN_DECL_ABR_NS

class HlsAbrPlaylist : public AbrPlaylist
{
public:
    HlsAbrPlaylist( uint8_t serverVersion,
                    uint8_t targetDuration,
                    uint8_t streamType,
                    uint8_t windowType,
                    uint16_t mediaSequenceNumber,
                    uint16_t maxSlidingWindowSize,
                    uint16_t videoLength,
                    ACE_Time_Value startTime,
                    videoServerBitrateList_t bitrateList);

    HlsAbrPlaylist();

    void CreateVariantPlaylists();
    std::string CreateAbrLiveBitratePlaylist(uint8_t bitrate, const ACE_Time_Value *currentTime);

    ~HlsAbrPlaylist();

private:
    void CreateVodVariantPlaylist();
    void CreateLiveVariantPlaylist();
    void CreateProgressiveBitratePlaylist();
    void CreateAbrVodBitratePlaylists();
    void CreateFragments();

};

END_DECL_ABR_NS

#endif
