/// @file
/// @brief Playlist config definition
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _PLAYLIST_CONFIG_H_
#define _PLAYLIST_CONFIG_H_

#include <map>
#include <vector>

#include <engine/EngineCommon.h>

DECL_L4L7_ENGINE_NS

// FIXME: already defined somewhere?
#define MAX_INTER_PACKET_DELAY 0xFFFFFFFF

///////////////////////////////////////////////////////////////////////////////
struct PlaylistConfig
{
    typedef enum {
        ATTACK_ONLY,
        STREAM_ONLY
    } PlaylistType;

    PlaylistConfig():
            type(STREAM_ONLY),
            name("")
    {
    }

    PlaylistType type;
    std::string  name;

    struct LoopInfo
    {
        uint16_t begIdx;
        uint16_t count;
    };

    typedef std::vector<uint8_t> Value_t;
    typedef std::vector<Value_t> Table_t;

    typedef enum 
    {
        MOD_INCR,
        MOD_DECR,
        MOD_RANDOM,
        MOD_SHUFFLE 
    } ModMode;

    struct RangeMod
    {
        Value_t start;
        Value_t step;
        Value_t mask;
        uint32_t recycleCount;
        uint32_t stutterCount;
        ModMode mode;
    };

    struct TableMod
    {
        Table_t table;
        uint32_t stutterCount;
    };

    struct Flow
    {
        Flow() :
            flowHandle(0),
            startTime(0),
            dataDelay(0),
            minPktDelay(0),
            maxPktDelay(MAX_INTER_PACKET_DELAY),
            socketTimeout(0),
            serverPort(0),
            reversed(false),
            loopInfo(0)
        {
        }

        handle_t flowHandle;
        uint32_t startTime;     // stream only
        uint32_t dataDelay;     // stream only
        uint32_t minPktDelay;   // for attacks, this is the delay between sends
        uint32_t maxPktDelay;   // stream only
        uint32_t socketTimeout; // currently only configurable for attacks
        uint16_t serverPort;
        bool reversed; // if true, client and server are swapped -- later phase
        LoopInfo *loopInfo; // pointer to a loop in the loopMap

        typedef std::map<uint8_t /* var index */, Value_t> VarMap_t;
        VarMap_t varMap;

        typedef std::map<uint8_t /* var index */, RangeMod> RangeModMap_t;
        RangeModMap_t rangeModMap;

        typedef std::map<uint8_t /* var index */, TableMod> TableModMap_t;
        TableModMap_t tableModMap;

        typedef std::map<uint8_t /* parent modifier's var index */, uint8_t /* child modifier's var index */> LinkModMap_t;
        LinkModMap_t linkModMap;
    };

    std::vector<Flow> flows;

    std::map<uint16_t /* end index */, LoopInfo> loopMap;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
