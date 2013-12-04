/// @file
/// @brief Flow config proxy implementation
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <boost/foreach.hpp>

#include <engine/FlowConfig.h>
#include <boost/static_assert.hpp>

#include "DpgdLog.h"
#include "FlowConfigProxy.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

FlowConfigProxy::FlowConfigProxy(const flowConfig_t& config)
    : mConfig(config)
{
    if (config.PktLen.size() != config.PktDelay.size() ||
        config.PktLen.size() != config.ClientTx.size())
    {
        std::string err("Internal flow array sizes are mismatched");
        TC_LOG_ERR(0, err);
        throw EPHXBadConfig(err);
    }  

    size_t total_pkt_len = 0;
    BOOST_FOREACH(uint32_t pkt_len, config.PktLen)
    {
        total_pkt_len += pkt_len;
    }

    if (total_pkt_len != config.Data.get_buffer_size())
    {
        std::string err("Packet lengths don't match data buffer size");
        TC_LOG_ERR(0, err);
        throw EPHXBadConfig(err);
    }  

    if (config.Close < DpgIf_1_port_server::CLOSE_TYPE_C_FIN ||
        config.Close > DpgIf_1_port_server::CLOSE_TYPE_S_RST)
    {
        std::string err("Invalid close type");
        TC_LOG_ERR(0, err);
        throw EPHXBadConfig(err);
    }

    if (config.Type != DpgIf_1_port_server::PLAY_TYPE_STREAM &&
        config.Type != DpgIf_1_port_server::PLAY_TYPE_ATTACK)
    {
        std::string err("Invalid play type");
        TC_LOG_ERR(0, err);
        throw EPHXBadConfig(err);
    }

    if (config.Layer != DpgIf_1_port_server::LAYER_TCP &&
        config.Layer != DpgIf_1_port_server::LAYER_UDP)
    {
        std::string err("Invalid layer type");
        TC_LOG_ERR(0, err);
        throw EPHXBadConfig(err);
    }

    ValidateLoops();
    ValidateVars();
}

///////////////////////////////////////////////////////////////////////////////

void FlowConfigProxy::ValidateLoops()
{
    uint16_t lastIdx = 0;

    BOOST_FOREACH(const loopConfig_t& loop, mConfig.Loops)
    {
        if (loop.BegIdx > loop.EndIdx)
        {
            std::string err("Loop indices reversed");
            TC_LOG_ERR(0, err);
            throw EPHXBadConfig(err);
        }

        if (lastIdx && loop.BegIdx <= lastIdx)
        {
            std::string err("Loops overlap");
            TC_LOG_ERR(0, err);
            throw EPHXBadConfig(err);
        } 

        if (loop.EndIdx >= mConfig.PktLen.size()) 
        {
            std::string err("Loop indices out of range");
            TC_LOG_ERR(0, err);
            throw EPHXBadConfig(err);
        }

        lastIdx = loop.EndIdx;
    }
}

///////////////////////////////////////////////////////////////////////////////

void FlowConfigProxy::ValidateVars()
{
    BOOST_FOREACH(const varConfig_t& var, mConfig.Vars)
    {
        if (var.PktIdx.size() != var.ByteIdx.size())
        {
            std::string err("Packet and byte index array size mismatch");
            TC_LOG_ERR(0, err);
            throw EPHXBadConfig(err);
        }

        for (size_t idx = 0; idx < var.PktIdx.size(); ++idx)
        {
            uint16_t pktIdx  = var.PktIdx[idx];
            uint16_t byteIdx = var.ByteIdx[idx];

            if (pktIdx >= mConfig.PktLen.size())
            {
                std::string err("Variable packet index invalid");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }

            if (byteIdx > mConfig.PktLen[pktIdx])
            {
                // note that byte == length is okay, that's inserting at end
                std::string err("Variable byte index invalid");
                TC_LOG_ERR(0, err);
                throw EPHXBadConfig(err);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void FlowConfigProxy::CopyTo(L4L7_ENGINE_NS::FlowConfig& copy) const
{
    const uint8_t * buffer = mConfig.Data.get_buffer();

    copy.pktList.resize(mConfig.PktDelay.size());

    for (size_t idx = 0; idx < copy.pktList.size(); ++idx)
    {
        size_t pkt_len = mConfig.PktLen[idx];

        copy.pktList[idx].data.assign(buffer, buffer + pkt_len);
        buffer += pkt_len;

        copy.pktList[idx].pktDelay = mConfig.PktDelay[idx];
        copy.pktList[idx].clientTx = mConfig.ClientTx[idx];
        copy.pktList[idx].loopInfo = 0;
        copy.pktList[idx].varMap.clear();
    }

    copy.varList.clear();
    copy.varList.reserve(mConfig.Vars.size());
    BOOST_FOREACH(const varConfig_t& var, mConfig.Vars)
    {
        L4L7_ENGINE_NS::FlowConfig::Variable varInfo;
        varInfo.name     = var.Name;
        varInfo.value    = var.Value;
        varInfo.fixedLen = var.FixedLen;
        varInfo.endian   = L4L7_ENGINE_NS::FlowConfig::eEndian(var.Endian);

        varInfo.targetList.resize(var.PktIdx.size());
        for (size_t vIdx = 0; vIdx < var.PktIdx.size(); ++vIdx)
        {
            varInfo.targetList[vIdx].pktIdx  = var.PktIdx[vIdx];
            varInfo.targetList[vIdx].byteIdx = var.ByteIdx[vIdx];
        }

        copy.varList.push_back(varInfo);
        for (size_t vIdx = 0; vIdx < var.PktIdx.size(); ++vIdx)
        {
            typedef L4L7_ENGINE_NS::FlowConfig::Packet::VarIdxMap_t::value_type varMapValue_t;
            copy.pktList[var.PktIdx[vIdx]].varMap.insert(                                       varMapValue_t(var.ByteIdx[vIdx], copy.varList.size() - 1));
        }
    }

    copy.loopMap.clear();
    BOOST_FOREACH(const loopConfig_t& loop, mConfig.Loops)
    {
        L4L7_ENGINE_NS::FlowConfig::LoopInfo info;
        info.begIdx = loop.BegIdx;
        info.count  = loop.Count;

        copy.loopMap[loop.EndIdx] = info;
    }

    copy.closeType = L4L7_ENGINE_NS::FlowConfig::eCloseType(mConfig.Close);
    copy.playType  = L4L7_ENGINE_NS::FlowConfig::ePlayType(mConfig.Type);
    copy.layer     = L4L7_ENGINE_NS::FlowConfig::eLayer(mConfig.Layer);
}

///////////////////////////////////////////////////////////////////////////////

BOOST_STATIC_ASSERT((int)DpgIf_1_port_server::CLOSE_TYPE_C_FIN == (int)L4L7_ENGINE_NS::FlowConfig::C_FIN);
BOOST_STATIC_ASSERT((int)DpgIf_1_port_server::CLOSE_TYPE_C_RST == (int)L4L7_ENGINE_NS::FlowConfig::C_RST);
BOOST_STATIC_ASSERT((int)DpgIf_1_port_server::CLOSE_TYPE_S_FIN == (int)L4L7_ENGINE_NS::FlowConfig::S_FIN);
BOOST_STATIC_ASSERT((int)DpgIf_1_port_server::CLOSE_TYPE_S_RST == (int)L4L7_ENGINE_NS::FlowConfig::S_RST);

///////////////////////////////////////////////////////////////////////////////

BOOST_STATIC_ASSERT((int)DpgIf_1_port_server::PLAY_TYPE_STREAM == (int)L4L7_ENGINE_NS::FlowConfig::P_STREAM);
BOOST_STATIC_ASSERT((int)DpgIf_1_port_server::PLAY_TYPE_ATTACK == (int)L4L7_ENGINE_NS::FlowConfig::P_ATTACK);


///////////////////////////////////////////////////////////////////////////////

BOOST_STATIC_ASSERT((int)DpgIf_1_port_server::LAYER_TCP == (int)L4L7_ENGINE_NS::FlowConfig::TCP);
BOOST_STATIC_ASSERT((int)DpgIf_1_port_server::LAYER_UDP == (int)L4L7_ENGINE_NS::FlowConfig::UDP);

///////////////////////////////////////////////////////////////////////////////

BOOST_STATIC_ASSERT((int)DpgIf_1_port_server::ENDIAN_BIG == (int)L4L7_ENGINE_NS::FlowConfig::BIG);
BOOST_STATIC_ASSERT((int)DpgIf_1_port_server::ENDIAN_LITTLE == (int)L4L7_ENGINE_NS::FlowConfig::LITTLE);

///////////////////////////////////////////////////////////////////////////////
