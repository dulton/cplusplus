/// @file
/// @brief Packet factory header file 
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _PACKET_FACTORY_H_
#define _PACKET_FACTORY_H_

#include <engine/EngineCommon.h>
#include <engine/FlowConfig.h>
#include <engine/PlaylistConfig.h>

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

class FlowInstance;
class PlaylistInstance;
class FlowModifierBlock;

class PacketFactory
{
  public:
    // public typedefs
    typedef FlowConfig::Packet Packet_t;
    typedef FlowConfig::Variable Variable_t;
    typedef FlowConfig::VarList_t VarList_t;
    typedef FlowConfig::Packet::VarIdxMap_t VarIdxMap_t;

    typedef PlaylistConfig::Flow::VarMap_t VarValueMap_t;

    PacketFactory(const VarValueMap_t& values, const Packet_t& packet, const VarList_t& varList, const FlowModifierBlock& modifiers);

    static size_t CalculateSize(const VarValueMap_t& values, const Packet_t& packet, const VarList_t& varList, const FlowModifierBlock& modifiers);

    ~PacketFactory();

    const uint8_t * Buffer() const { return &mBuffer->at(0); } 

    size_t BufferLength() const { return mBuffer->size(); }

  private:
    size_t CalculateBufferSize() const;

    void FillBuffer(std::vector<uint8_t> * buffer) const;

    const VarValueMap_t& mValues;
    const Packet_t& mPacket;
    const VarList_t& mVarList;
    const FlowModifierBlock& mModifiers;
    const std::vector<uint8_t> * mBuffer;
    bool mBufferOwned;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
