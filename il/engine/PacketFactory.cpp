/// @file
/// @brief Packet factory implementation
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

#include "ModifierBlock.h"
#include "PacketFactory.h"


USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

PacketFactory::PacketFactory(const VarValueMap_t& values, const Packet_t& packet, const VarList_t& varList, const FlowModifierBlock& modifiers)
    : mValues(values), mPacket(packet), mVarList(varList), mModifiers(modifiers)
{
    if (mPacket.varMap.empty())
    {
        // no variables, packet is static
        mBuffer = &(packet.data);
        mBufferOwned = false;
    }
    else
    {
        std::vector<uint8_t>* buffer = new std::vector<uint8_t>(CalculateBufferSize());
        FillBuffer(buffer);

        mBuffer = buffer;
        mBufferOwned = true;
    }
}

///////////////////////////////////////////////////////////////////////////////

size_t PacketFactory::CalculateSize(const VarValueMap_t& values, const Packet_t& packet, const VarList_t& varList, const FlowModifierBlock& modifiers)
{
    size_t size = packet.data.size();

    BOOST_FOREACH(const VarIdxMap_t::value_type& varPair, packet.varMap)
    {
        const uint8_t varIdx = varPair.second;

        // NOTE: if we use the fixed length flag we can eliminate the 
        //       lookups for override and modifiers
        // ALSO: zero byte modifiers are not supported since we use
        //       GetSize == 0 to signify no modifier 

        size_t modSize = modifiers.GetSize(varIdx);
        if (modSize)
        {
            size += modSize;
        }
        else
        {
            VarValueMap_t::const_iterator override = values.find(varIdx);
        
            if (override != values.end())
            {
                size += override->second.size();
            }
            else
            {
                // assume fixed length for now
                size += varList[varIdx].value.size();
            }
        }
    }
    
    return size;
}

///////////////////////////////////////////////////////////////////////////////

PacketFactory::~PacketFactory()
{
    if (mBufferOwned) delete mBuffer;
}

///////////////////////////////////////////////////////////////////////////////

void PacketFactory::FillBuffer(std::vector<uint8_t> * buffer) const
{
    if (buffer->empty())
        return;

    // assume non-empty buffer is big enough
    uint8_t * bytePtr = &buffer->at(0);
    size_t byteIdx = 0;

    BOOST_FOREACH(const VarIdxMap_t::value_type& varPair, mPacket.varMap)
    {   
        const uint16_t varByteIdx = varPair.first;
        const uint8_t varIdx = varPair.second;

        if (varByteIdx > byteIdx)
        {
            // write some bytes from the packet template
            size_t byteCount = varByteIdx - byteIdx;
            memcpy(bytePtr, &mPacket.data[byteIdx], byteCount);
            bytePtr += byteCount;
            byteIdx += byteCount;  
        }   

        if (varByteIdx != byteIdx)
        {
            // FIXME LOG AND THROW
            return;
        }

        size_t varSize = 0;
        size_t modSize = mModifiers.GetSize(varIdx);
        FlowConfig::eEndian endian;
        
        {
            // NOTE: we place the variable in the buffer and then put the
            // modifier on top of it. When there's a mask of all ones or
            // a table modifier, this is unnecessary. 
            //
            // A possible optimization would be to place the variable
            // data inside the modifier itself so we don't need to 
            // look at the variable here. The range modifier already
            // supports this, random does not.
            VarValueMap_t::const_iterator override = mValues.find(varIdx);
            const Variable_t& var = mVarList[varIdx];
            endian = var.endian;
        
            if (override != mValues.end())
            {
                varSize = override->second.size();
                size_t sizeDiff = 0;
                if (modSize && modSize < varSize)
                {
                    // if a modifier exists and is smaller than the variable,
                    // don't write out the remaining bytes
                    sizeDiff = varSize - modSize;
                    varSize = modSize;
                }

                if (endian == FlowConfig::LITTLE)
                    std::reverse_copy(&override->second[sizeDiff], &override->second[sizeDiff] + varSize, bytePtr);
                else
                    std::copy(&override->second[0], &override->second[0] + varSize, bytePtr);
            }
            else
            {
                // use the default
                varSize = var.value.size();
                size_t sizeDiff = 0;
                if (modSize && modSize < varSize)
                {
                    // if a modifier exists and is smaller than the variable,
                    // don't write out the remaining bytes
                    sizeDiff = varSize - modSize;
                    varSize = modSize;
                }

                if (endian == FlowConfig::LITTLE)
                    std::reverse_copy(&var.value[sizeDiff], &var.value[sizeDiff] + varSize, bytePtr);
                else
                    std::copy(&var.value[0], &var.value[0] + varSize, bytePtr);
            }
        }
        
        if (modSize)
        {
            if (modSize > varSize)
            {
                // if the modifier is bigger than the variable, fill with zeros
                memset(bytePtr + varSize, 0, modSize - varSize);
            }

            if (endian == FlowConfig::LITTLE)
                mModifiers.GetValue(varIdx, bytePtr, true /* reversed */);
            else
                mModifiers.GetValue(varIdx, bytePtr, false /* non-reversed */);
        } 

        bytePtr += modSize ? modSize : varSize;
    }

    if (byteIdx < mPacket.data.size())
    {
        size_t byteCount = mPacket.data.size() - byteIdx;
        memcpy(bytePtr, &mPacket.data[byteIdx], byteCount);
        bytePtr += byteCount;
    }
}

///////////////////////////////////////////////////////////////////////////////

size_t PacketFactory::CalculateBufferSize() const
{
    return CalculateSize(mValues, mPacket, mVarList, mModifiers); 
}

///////////////////////////////////////////////////////////////////////////////

