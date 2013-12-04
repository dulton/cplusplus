/// @file
/// @brief Flow engine implementation
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <boost/foreach.hpp>

#include "AbstFlowConfigProxy.h"
#include "FlowEngine.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

void FlowEngine::UpdateFlow(handle_t handle, const AbstFlowConfigProxy& config) 
{
    config.CopyTo(mConfigMap[handle]);

    // The following is some necessary initialization.

    // fill in the packet loop info into all the packets that are the last packet of a loop
    // This is to avoid repeating searching in the loop map for every flow instance.
    FlowConfig &flow = mConfigMap[handle];
    LinkLoopInfoToPkts(flow);

    // for attacks only, group packets into two vectors for different transmission directions, respectively
    // NOTE: this has to be after loop info is linked to packets,
    // because the loop info will be appened to both sides now.
    flow.mClientPayload.clear();
    flow.mServerPayload.clear();
    if (flow.playType == FlowConfig::P_ATTACK)
    {
        AttackInitialization(flow);
    }
}

///////////////////////////////////////////////////////////////////////////////

const FlowEngine::Config_t* FlowEngine::GetFlow(handle_t handle) const 
{
    MapConstIter_t iter = mConfigMap.find(handle);
    if (iter == mConfigMap.end())
        return 0;

    return &(iter->second);
}

///////////////////////////////////////////////////////////////////////////////

void FlowEngine::DeleteFlow(handle_t handle) 
{
    MapIter_t iter = mConfigMap.find(handle);
    if (iter == mConfigMap.end())
        throw DPG_EInternal("unable to delete flow with unknown handle");

    mConfigMap.erase(iter);
}


///////////////////////////////////////////////////////////////////////////////

void FlowEngine::LinkLoopInfoToPkts(FlowConfig& flow)
{
    //clear loop info in the packets
    BOOST_FOREACH (FlowConfig::Packet & packet, flow.pktList)
    {
        packet.loopInfo = 0;
    }

    for (LoopMapIter_t iter_loop = flow.loopMap.begin(); iter_loop!=flow.loopMap.end(); iter_loop++)
    {
        uint16_t endIdx = iter_loop->first;
        FlowConfig::LoopInfo & loop = iter_loop->second;
        flow.pktList[endIdx].loopInfo = &loop;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// This routine also removes empty packets
//
void FlowEngine::AttackInitialization(FlowConfig& flow)
{
    uint16_t idx = 0;
    uint16_t newIdx = 0;
    uint16_t otherSide = 0;
    uint16_t beg = 0;
    uint16_t newBeg = 0;
    uint16_t newClientEnd = 0;
    uint16_t newServerEnd = 0;

    flow.mNumOfClientPktsToPlay = 0;
    flow.mNumOfServerPktsToPlay = 0;

    BOOST_FOREACH (const FlowConfig::Packet &packet, flow.pktList)
    {
        if (packet.data.size() || !packet.varMap.empty())
        {
            if (packet.clientTx)
            {
                flow.mClientPayload.push_back(idx);
                flow.mNumOfClientPktsToPlay++;
                newClientEnd++;
            }
            else
            {
                flow.mServerPayload.push_back(idx);
                flow.mNumOfServerPktsToPlay++;
                newServerEnd++;
            }
            // update loop info
            FlowConfig::LoopInfo *loop = packet.loopInfo;
            if (loop)
            {
                std::vector<uint16_t>::const_iterator iter, iter2;
                // Update loop info for this side
                // The begin index of each loop needs to be updated to use the index to the two one-direction packet lists, namely mClientPayload and mServerPayload. Because attacks do not use the original packet list directly; instead they work with two separate packet lists, for Tx and Rx, respectively.
                beg = loop->begIdx;
                newBeg = beg;
                // if beg is not the same direction, adjust it to the nearest next pkt with the same direction
                while ( ((flow.pktList[newBeg].clientTx) ^ packet.clientTx) && newBeg <= idx)
                {
                    newBeg++;
                }
                // find the new idx
                if (packet.clientTx)
                {
                    iter = flow.mClientPayload.begin();
                }
                else
                {
                    iter = flow.mServerPayload.begin();
                }
                newIdx = 0;
                for (;;iter++)
                {
                    if ((*iter)==newBeg)
                    {
                        //found
                        loop->begIdx = newIdx;
                        // update packet count

                        if (packet.clientTx)
                        {
                            flow.mNumOfClientPktsToPlay += (newClientEnd - newIdx) * (loop->count - 1);
                        }
                        else
                        {
                            flow.mNumOfServerPktsToPlay += (newServerEnd - newIdx) * (loop->count - 1);
                        }
                        break;
                    }
                    newIdx++;
                }
                // Add the loop to the loop map for the other side, if necessary
                // Each loop now has to be marked to TWO packets if the loop contains packets of BOTH directions. For example, if the end of a loop is a client Tx packet, then the last server tx packet within the loop also needs to be marked with a loop, which is created with a new begin index and inserted to the loop map in the configuration.
                otherSide = idx;
                // find the end of the new loop
                while ( (flow.pktList[otherSide].clientTx ^ (!packet.clientTx)) && otherSide >= beg )
                {
                    otherSide--;
                }
                if (otherSide >= beg)
                {
                    // find the new idx for beg idx of this new loop
                    newBeg = beg;
                    // if beg is not the other direction, adjust it to the nearest next packet with the other direction
                    while (!(flow.pktList[newBeg].clientTx ^ packet.clientTx))
                    {
                        newBeg++;
                    }
                    // find the new idx
                    if (packet.clientTx)
                    {
                        iter2 = flow.mServerPayload.begin();
                    }
                    else
                    {
                        iter2 = flow.mClientPayload.begin();
                    }
                    newIdx = 0;
                    for (;;iter2++)
                    {
                        if ((*iter2)==newBeg)
                        {
                            //found
                            FlowConfig::LoopInfo newLoop;
                            newLoop.begIdx = newIdx;
                            newLoop.count = loop->count;
                            flow.loopMap.insert(std::make_pair(otherSide, newLoop));
                            flow.pktList[otherSide].loopInfo = &(flow.loopMap[otherSide]);
                            // update packet count
                            // find the new index for the pkt with index "otherSide"
                            uint16_t newOtherSide = 0;
                            std::vector<uint16_t>::const_iterator iter_new;
                            if (packet.clientTx)
                            {
                                iter_new = flow.mServerPayload.begin();
                            }
                            else
                            {
                                iter_new = flow.mClientPayload.begin();
                            }
                            for (;;iter_new++)
                            {
                                if (((*iter_new)==otherSide))
                                {
                                    if (packet.clientTx)
                                    {

                                        flow.mNumOfServerPktsToPlay += (newOtherSide - newIdx + 1) * (loop->count - 1);
                                    }
                                    else
                                    {
                                        flow.mNumOfClientPktsToPlay += (newOtherSide - newIdx + 1) * (loop->count - 1);
                                    }
                                    break;
                                }
                                newOtherSide++;
                            }

                            break;
                        }
                        newIdx++;
                    }
                }
            }
        }

        idx++;
    }
}

///////////////////////////////////////////////////////////////////////////////
