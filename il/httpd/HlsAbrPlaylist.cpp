/// @file
/// @brief HTTP HLS ABR Playlist implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///
#include <sstream>

#include <phxexception/PHXException.h>

#include "HlsAbrPlaylist.h"
#include "HttpdLog.h"

USING_ABR_NS;
USING_ABR_UTILS_NS;

///////////////////////////////////////////////////////////////////////////////

HlsAbrPlaylist::HlsAbrPlaylist(uint8_t serverVersion,
                uint8_t targetDuration,
                uint8_t streamType,
                uint8_t windowType,
                uint16_t mediaSequenceNumber,
                uint16_t maxSlidingWindowSize,
                uint16_t videoLength,
                ACE_Time_Value startTime,
                videoServerBitrateList_t bitrateList)
    :    AbrPlaylist(serverVersion, targetDuration, streamType, windowType, mediaSequenceNumber, maxSlidingWindowSize, videoLength, startTime, bitrateList)
{
    // Set mime types
    SetPlaylistMimeType(APPLE_PLAYLIST_MIME);
    SetFragmentMimeType(APPLE_FRAGMENT_MIME);

    // Create all static data: variant playlist for VOD/Live, and VOD playlists for the selected bitrates
    CreateVariantPlaylists();
    CreateFragments();

    // Create Progressive or ABR bitrate playlist based on server config
    if(GetVideoStreamType() == http_1_port_server::HttpVideoServerStreamTypeOptions_PROGRESSIVE)
        CreateProgressiveBitratePlaylist();
    else
        CreateAbrVodBitratePlaylists();
}

///////////////////////////////////////////////////////////////////////////////

void HlsAbrPlaylist::CreateVariantPlaylists()
{
    CreateVodVariantPlaylist();
    
    // Live variant playlists should not be created for progressive session
    if(GetVideoStreamType() != http_1_port_server::HttpVideoServerStreamTypeOptions_PROGRESSIVE)
        CreateLiveVariantPlaylist();
}

///////////////////////////////////////////////////////////////////////////////

void HlsAbrPlaylist::CreateProgressiveBitratePlaylist()
{
    std::ostringstream oss;
    uint8_t lowestBitrate = GetFrontOfBitrateList();
    std::string selectedBitrate = BitrateToString(lowestBitrate);
    size_t videoLength = GetServerVideoLength();
    uint16_t videoTargetDuration = GetVideoTargetDuration();
    uint16_t mediaSequenceNumber = GetMediaSequenceNumber();

    oss << "#EXTM3U\n";
    oss << "#EXT-X-VERSION:" << (size_t)VersionToInteger(GetVideoServerVersion()) << "\n";

    // Check if fragment duration is equal to video length, or if video target duration not set
    if ((videoLength == videoTargetDuration) || (videoTargetDuration == 0))
    {
        oss << "#EXT-X-TARGETDURATION:" << videoLength << "\n";
        oss << "#EXTINF:" << videoLength << ",\n";
        oss << selectedBitrate << "/" << "HLSvideo-entire.ts\n";
    }
    else    // fragmented progressive playlist
    {
        size_t fragmentCount = videoLength / videoTargetDuration;
        size_t remainingSeconds = videoLength % videoTargetDuration;
        size_t i;

        // Continue playlist header
        oss << "#EXT-X-TARGETDURATION:" << videoTargetDuration << "\n";
        oss << "#EXT-X-MEDIA-SEQUENCE:" << mediaSequenceNumber << "\n";

        for(i = 0; i < fragmentCount; i++)
        {
            oss << "#EXTINF:" << videoTargetDuration << ",\n";
            oss << selectedBitrate << "/" << "HLSvideo-" << i + mediaSequenceNumber << ".ts\n";
        }

        // Handle remaining seconds case
        if (remainingSeconds)
        {
            oss << "#EXTINF:" << remainingSeconds << ",\n";
            oss << selectedBitrate << "/" << "HLSvideo-" << i + mediaSequenceNumber << ".ts\n";
        }
    }

    // Make end tag
    oss << "#EXT-X-ENDLIST\n";

    // Update bitrate list pointer array
    SetBitrateInPlaylistMapAt(oss.str(), lowestBitrate);
}

///////////////////////////////////////////////////////////////////////////////

void HlsAbrPlaylist::CreateVodVariantPlaylist()
{
    std::ostringstream oss;

    //Bitrate list should not be empty
    if(GetBitrateList()->empty())
    {
        TC_LOG_ERR(0, "[HlsAbrPlaylist] bitrate list is empty");
        throw EPHXInternal();
    }

    size_t programId = 1;

    oss << "#EXTM3U\n";

    for (videoServerBitrateList_t::iterator iter = GetBitrateList()->begin(); iter != GetBitrateList()->end(); ++iter)
    {
        switch(*iter)
        {
            case http_1_port_server::HttpVideoServerBitrateOptions_BR_64:
            {
                oss << "#EXT-X-STREAM-INF:PROGRAM-ID=" << programId << ",BANDWIDTH=64000" << "\n";
                oss << "64k-audio-only.m3u8" << "\n";
                break;
            }
            case http_1_port_server::HttpVideoServerBitrateOptions_BR_96:
            {
                oss << "#EXT-X-STREAM-INF:PROGRAM-ID=" << programId << ",BANDWIDTH=96000" << "\n";
                oss << "96k-video.m3u8" << "\n";
                break;
            }
            case http_1_port_server::HttpVideoServerBitrateOptions_BR_150:
            {
                oss << "#EXT-X-STREAM-INF:PROGRAM-ID=" << programId << ",BANDWIDTH=150000" << "\n";
                oss << "150k-video.m3u8" << "\n";
                break;
            }
            case http_1_port_server::HttpVideoServerBitrateOptions_BR_240:
            {
                oss << "#EXT-X-STREAM-INF:PROGRAM-ID=" << programId << ",BANDWIDTH=240000" << "\n";
                oss << "240k-video.m3u8" << "\n";
                break;
            }
            case http_1_port_server::HttpVideoServerBitrateOptions_BR_256:
            {
                oss << "#EXT-X-STREAM-INF:PROGRAM-ID=" << programId << ",BANDWIDTH=256000" << "\n";
                oss << "256k-video.m3u8" << "\n";
                break;
            }
            case http_1_port_server::HttpVideoServerBitrateOptions_BR_440:
            {
                oss << "#EXT-X-STREAM-INF:PROGRAM-ID=" << programId << ",BANDWIDTH=440000" << "\n";
                oss << "440k-video.m3u8" << "\n";
                break;
            }
            case http_1_port_server::HttpVideoServerBitrateOptions_BR_640:
            {
                oss << "#EXT-X-STREAM-INF:PROGRAM-ID=" << programId << ",BANDWIDTH=640000" << "\n";
                oss << "640k-video.m3u8" << "\n";
                break;
            }
            case http_1_port_server::HttpVideoServerBitrateOptions_BR_800:
            {
                oss << "#EXT-X-STREAM-INF:PROGRAM-ID=" << programId << ",BANDWIDTH=800000" << "\n";
                oss << "800k-video.m3u8" << "\n";
                break;
            }
            case http_1_port_server::HttpVideoServerBitrateOptions_BR_840:
            {
                oss << "#EXT-X-STREAM-INF:PROGRAM-ID=" << programId << ",BANDWIDTH=840000" << "\n";
                oss << "840k-video.m3u8" << "\n";
                break;
            }
            case http_1_port_server::HttpVideoServerBitrateOptions_BR_1240:
            {
                oss << "#EXT-X-STREAM-INF:PROGRAM-ID=" << programId << ",BANDWIDTH=1240000" << "\n";
                oss << "1240k-video.m3u8" << "\n";
                break;
            }
            default:
                TC_LOG_ERR(0, "[HlsAbrPlaylist] unknown bitrate");
                throw EPHXInternal();
        }

        // If progressive, we only want the first value, which is the lowest bitrate selected
        if (GetVideoStreamType() == http_1_port_server::HttpVideoServerStreamTypeOptions_PROGRESSIVE)
            break;
    }

    SetVodVariantPlaylist(oss.str());
}

///////////////////////////////////////////////////////////////////////////////

void HlsAbrPlaylist::CreateLiveVariantPlaylist()
{
    // For Apple HLS, there is no difference between live and VOD for variant playlist
    SetLiveVariantPlaylist(*(GetVodVariantPlaylist()));
}

///////////////////////////////////////////////////////////////////////////////

void HlsAbrPlaylist::CreateAbrVodBitratePlaylists()
{
    std::string selectedBitrate;
    uint16_t videoTargetDuration = GetVideoTargetDuration();
    uint16_t mediaSequenceNumber = GetMediaSequenceNumber();
    size_t fragmentCount;
    size_t videoLength = GetServerVideoLength();
    size_t remainingSeconds;
    size_t i;

    // Create bitrate playlist and store, for all bitrates.  For VOD only.
    for(videoServerBitrateList_t::iterator iter = GetBitrateList()->begin(); iter != GetBitrateList()->end(); iter++)
    {
        selectedBitrate = BitrateToString(*iter);

        std::ostringstream oss;

        oss << "#EXTM3U\n";
        oss << "#EXT-X-VERSION:" << VersionToInteger(GetVideoServerVersion()) << "\n";
        oss << "#EXT-X-TARGETDURATION:" << videoTargetDuration << "\n";

        fragmentCount = videoLength / videoTargetDuration;
        remainingSeconds = videoLength % videoTargetDuration;

        // Continue playlist header
        oss << "#EXT-X-MEDIA-SEQUENCE:" << mediaSequenceNumber << "\n";

        for(i = 0; i < fragmentCount; i++)
        {
            oss << "#EXTINF:" << videoTargetDuration << ",\n";
            oss << selectedBitrate << "/" << "HLSvideo-" << i + mediaSequenceNumber << ".ts\n";
        }

        // Handle case in which there are remaining seconds
        if (remainingSeconds)
        {
            oss << "#EXTINF:" << remainingSeconds << ",\n";
            oss << selectedBitrate << "/" << "HLSvideo-" << i + mediaSequenceNumber << ".ts\n";
        }

        // Make end tag
        oss << "#EXT-X-ENDLIST\n";

        // Update bitrate playlist map
        SetBitrateInPlaylistMapAt(oss.str(), *iter);
    }
}

///////////////////////////////////////////////////////////////////////////////

std::string HlsAbrPlaylist::CreateAbrLiveBitratePlaylist(uint8_t bitrate, const ACE_Time_Value *currentTime)
{
    std::ostringstream oss;
    std::string selectedBitrate = BitrateToString(bitrate);
    uint16_t videoTargetDuration = GetVideoTargetDuration();
    uint16_t mediaSequenceNumber = GetMediaSequenceNumber();
    size_t absTime = currentTime->sec() - GetSessionStartTime()->sec();
    size_t maxSlidingWindowSize = GetMaxSlidingWindowSize();
    size_t fragmentsToAdd = (absTime/videoTargetDuration) + INITIAL_LIVE_PLAYLIST_SIZE;        // Start off with three fragments, at least, then add one fragment every target duration
    size_t sequenceToStopAt = fragmentsToAdd + mediaSequenceNumber;

    oss << "#EXTM3U\n";
    oss << "#EXT-X-VERSION:" << VersionToInteger(GetVideoServerVersion()) << "\n";
    oss << "#EXT-X-TARGETDURATION:" << videoTargetDuration << "\n";

    // Check if number of fragments to add is greater than max sliding window, if so, need to increase sliding window and media seq number
    if (fragmentsToAdd > maxSlidingWindowSize)
    {
        mediaSequenceNumber = (uint16_t)(sequenceToStopAt - maxSlidingWindowSize);    // Get offset for new media sequence number
    }

    // Continue playlist header
    oss << "#EXT-X-MEDIA-SEQUENCE:" << mediaSequenceNumber << "\n";

    // TODO:  Need check for video length?  To identify when we should add END LIST tag?
    for(size_t i = mediaSequenceNumber; i < sequenceToStopAt; i++)
    {
        oss << "#EXTINF:" << videoTargetDuration << ",\n";
        oss << selectedBitrate << "/" << "HLSvideo-" << i << ".ts\n";
    }

    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

void HlsAbrPlaylist::CreateFragments()
{
    // Calculate and store fragment size for each bitrate.
    for(videoServerBitrateList_t::iterator iter = GetBitrateList()->begin(); iter != GetBitrateList()->end(); iter++)
    {
        uint16_t bitrate = BitrateToInteger(*iter);

        // Calculate total size:
        uint32_t fragmentSize = bitrate;

        // Add audio bitrate if selected bitrate is not audio only
        //if(fragmentSize != 64)
        //    fragmentSize += AUDIO_BITRATE;                      // Apple includes audio bitrate in fragment

        fragmentSize *= 1000;                                   // Convert kb bitrate to bits
        fragmentSize *= GetVideoTargetDuration();               // Multiply by target duration
        fragmentSize /= 8;                                      // Convert to bytes

        SetSizeInFragmentSizeMapAt(fragmentSize, *iter);
    }
}

///////////////////////////////////////////////////////////////////////////////

HlsAbrPlaylist::~HlsAbrPlaylist() 
{
}

///////////////////////////////////////////////////////////////////////////////
