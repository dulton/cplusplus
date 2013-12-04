/// @file
/// @brief ABR Utils header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ABR_UTILS_H_
#define _ABR_UTILS_H_

#include <ace/Message_Block.h>

#include <boost/unordered_map.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include "http_1_port_server.h"

#include <string>
#include <utility>
#include <memory>

///////////////////////////////////////////////////////////////////////////////

#define ABR_UTILS_NS abr::utils
#define ABR_NS abr
#define BEGIN_DECL_ABR_NS namespace http { namespace abr {
#define END_DECL_ABR_NS }}
#define USING_ABR_UTILS_NS using namespace http::abr::utils
#define USING_ABR_NS using namespace http::abr

///////////////////////////////////////////////////////////////////////////////

BEGIN_DECL_ABR_NS

namespace utils {

///////////////////////////////////////////////////////////////////////////////
/*
enum bitrate_t {
    BR_96 = 0,
    BR_150,
    BR_240,
    BR_256,
    BR_440,
    BR_640,
    BR_800,
    BR_840,
    BR_1240,
    BR_AUDIO_64,
    BR_UNKNOWN
};
*/
enum requestType_t {
    VARIANT_PLAYLIST = 0,
    MANIFEST,
    FRAGMENT,
    TEXT,
    UNKNOWN
};

enum sessionType_t {
    LIVE = 0,
    VOD
};


/////////////////////////////////////////////////////////////////////

//typedef Http_1_port_server::Enum_VideoServerBitrates videoServerBitrates
typedef std::vector<uint32_t> videoServerBitrateList_t;

// Maps
typedef boost::unordered_map<uint8_t, std::string> playlistMap_t;
typedef boost::unordered_map<uint8_t, size_t> fragmentSizeMap_t;

/////////////////////////////////////////////////////////////////////

#define APPLE_PLAYLIST_MIME        "application/x-mpegURL"
#define APPLE_FRAGMENT_MIME        "video/MP2T"

#define AUDIO_BITRATE              64

#define DEFAULT_MSGBODY_SIZE       128 * 1024
#define INITIAL_LIVE_PLAYLIST_SIZE 3

/////////////////////////////////////////////////////////////////////

const uint8_t IntegerToBitrate(uint16_t bitrate);
const std::string BitrateToString(uint8_t bitrate);
const uint16_t BitrateToInteger(uint8_t bitrate);
const size_t VersionToInteger(uint8_t serverVersion);
const bool ParseUri(const std::string *uri, sessionType_t *sessionType, requestType_t *requestType, uint16_t *mediaSequence, uint8_t *bitrate);

/////////////////////////////////////////////////////////////////////

class AbrStaticMessageBody : public ACE_Data_Block
{
  public:
    AbrStaticMessageBody(size_t size, std::string fill);
};

///////////////////////////////////////////////////////////////////////////////

class AbrStreamMessageBody : public ACE_Message_Block
{
public:
    explicit AbrStreamMessageBody(std::string *msg);
};

///////////////////////////////////////////////////////////////////////////////

} // utils namespace

END_DECL_ABR_NS

#endif
