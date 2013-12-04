/// @file
/// @brief Flow config definition
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _FLOW_CONFIG_H_
#define _FLOW_CONFIG_H_

#include <map>
#include <vector>
#include <boost/tr1/memory.hpp>

#include <engine/EngineCommon.h>

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

struct FlowConfig
{
    struct LoopInfo
    {
        uint16_t begIdx;
        uint16_t count;
    };

    struct Target
    {
        uint16_t pktIdx;
        uint16_t byteIdx;
    };

    enum eEndian { BIG, LITTLE };

    struct Variable
    {
        Variable() : endian(BIG) {}
        std::string name;
        std::vector<uint8_t> value;
        std::vector<Target>  targetList;
        bool fixedLen;
        eEndian endian;
    };

    struct Packet
    {
        std::vector<uint8_t> data;
        uint32_t pktDelay;
        bool clientTx;
        LoopInfo * loopInfo; // points to loopInfo in loopMap
    
        typedef std::multimap<uint16_t, uint8_t> VarIdxMap_t;
        VarIdxMap_t varMap;  // points to a variables in varList that affect this packet, ordered by byte index; duplicates are kept in insertion order
    };

    typedef std::vector<Packet> PktList_t;
    PktList_t pktList;

    typedef std::vector<Variable> VarList_t;
    VarList_t varList;

    std::map<uint16_t /* end index */, LoopInfo> loopMap;

    enum eCloseType { C_FIN, C_RST, S_FIN, S_RST };
    eCloseType closeType;

    enum ePlayType { P_STREAM, P_ATTACK };
    ePlayType playType;

    enum eLayer { TCP, UDP };
    eLayer layer;

    // Used for attacks only
    // Current Attack Instance implementation needs to have two seperate packet lists for each direction,
    // instead of mixing all packets in one list.
    std::vector<uint16_t>         mClientPayload; // an array of pkt indices
    std::vector<uint16_t>         mServerPayload; // an array of pkt indices

    size_t                        mNumOfClientPktsToPlay;
    size_t                        mNumOfServerPktsToPlay;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
