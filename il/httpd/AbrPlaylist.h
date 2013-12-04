/// @file
/// @brief HTTP ABR Playlist header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ABR_PLAYLIST_H_
#define _ABR_PLAYLIST_H_

#include <ace/Time_Value.h>
#include <boost/unordered_map.hpp>

#include "AbrUtils.h"

BEGIN_DECL_ABR_NS

class AbrPlaylist
{

public:
    typedef ABR_UTILS_NS::videoServerBitrateList_t videoServerBitrateList_t;
    typedef ABR_UTILS_NS::playlistMap_t playlistMap_t;
    typedef ABR_UTILS_NS::fragmentSizeMap_t fragmentSizeMap_t;

    AbrPlaylist(uint8_t serverVersion,
                uint8_t targetDuration,
                uint8_t streamType,
                uint8_t windowType,
                uint16_t mediaSequenceNumber,
                uint16_t maxSlidingWindowSize,
                uint16_t videoLength,
                ACE_Time_Value startTime,
                videoServerBitrateList_t bitrateList);

    // Getters
    uint16_t GetVideoTargetDuration() { return (uint16_t)mVideoTargetDuration; }            // Cast as uint16, to avoid mistake of returning char character!
    uint8_t GetVideoStreamType() { return mVideoStreamType; }
    uint16_t GetMediaSequenceNumber() { return mMediaSequenceNumber; }
    ACE_Time_Value *GetSessionStartTime() { return &mSessionStartTime; }
    size_t GetMaxSlidingWindowSize() { return mMaxSlidingWindowSize; }
    uint16_t GetServerVideoLength() { return mServerVideoLength; }
    videoServerBitrateList_t *GetBitrateList() { return &mBitrateList; }
    uint8_t GetFrontOfBitrateList() { return mBitrateList.front();}
    std::string *GetVodVariantPlaylist() { return &mVodVariantPlaylist; }
    std::string *GetLiveVariantPlaylist() { return &mLiveVariantPlaylist; }
    playlistMap_t *GetBitratePlaylistMap() { return &mBitratePlaylistMap; }
    std::string *GetPlaylistFromBitrateMapAt(uint8_t bitrate) { return &mBitratePlaylistMap.at(bitrate); }
    size_t GetSizeFromFragmentSizeMapAt(uint8_t bitrate) { return mFragmentSizeMap.at(bitrate); }
    std::string GetPlaylistMimeType() { return mPlaylistMimeType; }
    std::string GetFragmentMimeType() { return mFragmentMimeType; }
    uint8_t GetVideoServerVersion() { return mVideoServerVersion; }

    // Setters
    void SetVideoStreamType(uint8_t streamType) { mVideoStreamType = streamType; }
    void SetMaxSlidingWindowSize(size_t newSize) { mMaxSlidingWindowSize = newSize; }
    void SetMediaSequenceNumber(uint16_t newSequenceNumber) { mMediaSequenceNumber = newSequenceNumber; }
    void SetServerVideoLength(uint16_t newVideoLength) { mServerVideoLength = newVideoLength; }
    void SetVodVariantPlaylist(std::string variantPlaylist) { mVodVariantPlaylist = variantPlaylist; }
    void SetLiveVariantPlaylist(std::string variantPlaylist) { mLiveVariantPlaylist = variantPlaylist; }
    void SetBitrateInPlaylistMapAt(std::string playlist, uint8_t bitrate) { mBitratePlaylistMap[bitrate] = playlist; }
    void SetFragmentMimeType(std::string mime) { mFragmentMimeType = mime; }
    void SetPlaylistMimeType(std::string mime) { mPlaylistMimeType = mime; }
    void SetVideoServerVersion(uint8_t version) { mVideoServerVersion = version; }
    void SetSizeInFragmentSizeMapAt(size_t size, uint8_t bitrate) { mFragmentSizeMap[bitrate] = size; }

    virtual std::string CreateAbrLiveBitratePlaylist(uint8_t bitrate, const ACE_Time_Value *currentTime) = 0;
    virtual void CreateVariantPlaylists() = 0;
    virtual ~AbrPlaylist();

private:
    virtual void CreateVodVariantPlaylist() = 0;
    virtual void CreateLiveVariantPlaylist() = 0;
    virtual void CreateProgressiveBitratePlaylist() = 0;
    virtual void CreateAbrVodBitratePlaylists() = 0;
    virtual void CreateFragments() = 0;

    uint8_t mVideoServerVersion;             // Server version
    std::string mPlaylistMimeType;           // Playlist mime type
    std::string mFragmentMimeType;           // Fragment mime type
    uint8_t    mVideoTargetDuration;         // fragment length
    uint8_t mVideoStreamType;         // ADAPTIVE_BITRATE/PROGRESSIVE
    uint8_t mWindowType;             // simple/sliding playlist type?
    uint16_t mMediaSequenceNumber;             // Media sequence number
    size_t mMaxSlidingWindowSize;         // Max sliding window for live playlists
    uint16_t mServerVideoLength;         // How long is the VOD video length?
    ACE_Time_Value mSessionStartTime;     // When the live session started
    videoServerBitrateList_t mBitrateList;     // Bitrate list

    std::string mVodVariantPlaylist;     // Vod variant playlist
    std::string mLiveVariantPlaylist;     // Live variant playlist
    playlistMap_t mBitratePlaylistMap;     // Map of bitrate playlists, VOD only
    fragmentSizeMap_t mFragmentSizeMap;      // Map of fragment sizes for each selected bitrate

};

END_DECL_ABR_NS

#endif
