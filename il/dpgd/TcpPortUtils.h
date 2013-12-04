/// @file
/// @brief TCP source-port utilities
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///
///  DPG encodes information into the TCP port (flow index & playlist instance)
///  using the logic contained here.
///

#ifndef _TCP_PORT_UTILS_H_
#define _TCP_PORT_UTILS_H_

#include "DpgdLog.h"
#include "DpgCommon.h"

class TestTcpPortUtils;

namespace L4L7_ENGINE_NS
{
    class PlaylistInstance;
};

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

class TcpPortUtils
{
private:
    static const uint16_t ATK_LOOP_MASK  = 0xf000;
    static const int      ATK_LOOP_SHIFT = 12;
    static const uint16_t ATK_LOOP_MIN   = 1;
    static const uint16_t ATK_LOOP_MAX   = ATK_LOOP_MASK >> ATK_LOOP_SHIFT;

    static const uint16_t ATK_FLOW_IDX_MASK   = 0x0f80;
    static const int      ATK_FLOW_IDX_SHIFT  = 7;
    static const uint16_t ATK_FLOW_IDX_MAX    = ATK_FLOW_IDX_MASK >> ATK_FLOW_IDX_SHIFT;

    static const uint16_t ATK_PLAY_INST_MASK  = 0x007f;
    static const uint16_t ATK_PLAY_INST_HALF  = 0x0040;
    static const int      ATK_PLAY_INST_SHIFT = 0;
    static const uint16_t ATK_PLAY_INST_MAX   = ATK_PLAY_INST_MASK >> ATK_PLAY_INST_SHIFT;


    static const uint16_t STM_FLOW_IDX_MASK   = 0xff00;
    static const int      STM_FLOW_IDX_SHIFT  = 8;
    static const uint16_t STM_FLOW_IDX_MIN    = 0x10;
    static const uint16_t STM_FLOW_IDX_MAX    = (STM_FLOW_IDX_MASK >> STM_FLOW_IDX_SHIFT) - STM_FLOW_IDX_MIN;

    static const uint16_t STM_PLAY_INST_MASK  = 0x00ff;
    static const uint16_t STM_PLAY_INST_HALF  = 0x0080;
    static const int      STM_PLAY_INST_SHIFT = 0;
    static const uint16_t STM_PLAY_INST_MAX   = STM_PLAY_INST_MASK >> STM_PLAY_INST_SHIFT;

public:
    // Attack bit allocation:
    //
    // 15-12     11-7     6-0       
    // AtkLoop   FlowIdx  PlayInst   
    //
    // AtkLoop is 1-15 and indicates the current iteration of the attack loop
    // (or 1 for streams and non-looped attacks)
    //
    // FlowIdx is 0-31 and indicates the instance of the flow within a given
    // playlist
    //
    // PlayInst is 0-127 and indicates the instance of playlist playing.
    //
    static uint16_t MakeAtkPort(uint16_t flowIdx, uint16_t playInst, uint16_t atkLoop)
    {
        if (atkLoop >= ATK_LOOP_MAX)
        {
            // atkLoop cycles from 0 to 14 (1 to 15 below) so we need a mod here
            atkLoop = (atkLoop % ATK_LOOP_MAX);
        }

        return (((atkLoop + 1) << ATK_LOOP_SHIFT) & ATK_LOOP_MASK) | 
               ((flowIdx  << ATK_FLOW_IDX_SHIFT)  & ATK_FLOW_IDX_MASK) | 
               ((playInst << ATK_PLAY_INST_SHIFT) & ATK_PLAY_INST_MASK); 
    }

    //
    // Stream bit allocation:
    //
    // 15-8     7-0       
    // FlowIdx  PlayInst   
    //
    // FlowIdx is 32 - 255 and indicates the instance of the flow within a given
    // playlist
    //
    // PlayInst is 0-255 and indicates the instance of playlist playing.
    //
    static uint16_t MakeStmPort(uint16_t flowIdx, uint16_t playInst)
    {

        return (((flowIdx + STM_FLOW_IDX_MIN) << STM_FLOW_IDX_SHIFT) & STM_FLOW_IDX_MASK) | 
               ((playInst << STM_PLAY_INST_SHIFT) & STM_PLAY_INST_MASK); 
    }

    static uint16_t MakePort(uint16_t flowIdx, L4L7_ENGINE_NS::PlaylistInstance * pi);

    static uint16_t GetFlowIdx(uint16_t port, bool isAttack)
    {
        return isAttack ? 
               (port & ATK_FLOW_IDX_MASK) >> ATK_FLOW_IDX_SHIFT :
               ((port & STM_FLOW_IDX_MASK) >> STM_FLOW_IDX_SHIFT) - STM_FLOW_IDX_MIN ;
    }

    static uint16_t GetPlayInst(uint16_t port, bool isAttack)
    {
        return isAttack ? 
            (port & ATK_PLAY_INST_MASK) >> ATK_PLAY_INST_SHIFT :
            (port & STM_PLAY_INST_MASK) >> STM_PLAY_INST_SHIFT ;
    }

    static uint16_t GetAtkLoop(uint16_t port)
    {
        return ((port & ATK_LOOP_MASK) >> ATK_LOOP_SHIFT) - 1;
    }

    // Get the cursor based on the port and given playlist, and update the playlist's cursor 
    static uint32_t GetCursor(uint16_t port, L4L7_ENGINE_NS::PlaylistInstance * pi);

    static uint32_t GetCursor(uint32_t oldCursor, uint16_t port, bool isAttack)
    {
        uint16_t playInst = GetPlayInst(port, isAttack);
        uint16_t oldPlayInst = isAttack ? 
                     ((oldCursor & ATK_PLAY_INST_MASK) >> ATK_PLAY_INST_SHIFT) :
                     ((oldCursor & STM_PLAY_INST_MASK) >> STM_PLAY_INST_SHIFT) ;
        const uint16_t mask = isAttack ? ATK_PLAY_INST_MASK : STM_PLAY_INST_MASK;
        uint16_t diff = (playInst - oldPlayInst) & mask;
        const uint16_t half = isAttack ? ATK_PLAY_INST_HALF : STM_PLAY_INST_HALF;

        if (diff < half)
        {
            return oldCursor + diff;
        }
        else
        {
            diff = mask - diff + 1;
            return oldCursor - diff;
        } 
    }

    static uint16_t MaxFlowIdx(bool isAttack)
    {
        return isAttack ? ATK_FLOW_IDX_MAX : STM_FLOW_IDX_MAX;
    }

    static uint16_t MaxPlayInst(bool isAttack)
    {
        return isAttack ? ATK_PLAY_INST_MAX : STM_PLAY_INST_MAX;
    }

    static uint16_t MaxAtkLoop()
    {
        return ATK_LOOP_MAX - 1; 
    }

    friend class TestTcpPortUtils;
};


///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
