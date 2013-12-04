/// @file
/// @brief Client Connection Handler implementation
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <cstdlib>
#include <sstream>

#include <ace/Message_Block.h>

#include "DpgConnectionHandler.h"
#include "DpgConnectionHandlerStates.h"
#include "DpgClientRawStats.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

const DpgConnectionHandler::InitState DpgConnectionHandler::INIT_STATE;

///////////////////////////////////////////////////////////////////////////////

DpgConnectionHandler::DpgConnectionHandler()
    : mStats(0),
      mState(&INIT_STATE),
      mComplete(false),
      mDelegate(true)
{
}

///////////////////////////////////////////////////////////////////////////////

DpgConnectionHandler::~DpgConnectionHandler()
{
}

///////////////////////////////////////////////////////////////////////////////

void DpgConnectionHandler::AsyncRecv(size_t dataLength, recvDelegate_t delegate)
{
    bool wasEmpty = mRecvList.empty();
    mRecvList.push_back(recvInfo_t(dataLength, delegate));
    if (wasEmpty && !mState->InBufferEmpty(*this))
    {
        // NOTE: this means that the callback may be called immediately before
        // this function returns
        const ACE_Time_Value now(ACE_OS::gettimeofday());
        mState->ProcessInput(*this, now);
    }
}

///////////////////////////////////////////////////////////////////////////////

void DpgConnectionHandler::SetStatsInstance(stats_t& stats)
{
    mStats = &stats;
}

///////////////////////////////////////////////////////////////////////////////

void DpgConnectionHandler::UpdateGoodputTxSent(uint64_t sent)
{
    if (mStats)
    {
        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
        mStats->goodputTxBytes += sent;
    }

    if (mDelegate) mTxDelegator.UpdateCount(sent);
}

///////////////////////////////////////////////////////////////////////////////

void DpgConnectionHandler::RegisterSendDelegate(uint32_t dataLength, const sendDelegate_t& sendDelegate)
{
    if (mDelegate) mTxDelegator.Register(dataLength, sendDelegate);
}

///////////////////////////////////////////////////////////////////////////////

bool DpgConnectionHandler::InitState::ProcessInput(DpgConnectionHandler& connHandler, const ACE_Time_Value& now) const
{
    if (RecvListEmpty(connHandler))
    {
        // not waiting for anything yet, let it pile up
        return true;
    }

    size_t inputSize = InBufferSize(connHandler);
    size_t expSize = RecvListFirstSize(connHandler);

    while (expSize && inputSize >= expSize)
    {
        L4L7_UTILS_NS::MessageIterator pos = InBufferBegin(connHandler);

        // FIXME -- make receiver accept non-contiguous data
        std::vector<uint8_t> contiguous_data;
        contiguous_data.reserve(expSize);
        while (expSize--)
        {
            contiguous_data.push_back(*pos++);
        }

        RecvListDelegateAndPop(connHandler, &contiguous_data[0], contiguous_data.size());

        // Advance message read pointer and trim input buffer (discarding completely parsed message blocks)
        pos.block()->rd_ptr(pos.ptr());
        InBufferTrim(connHandler, pos.block());

        inputSize = InBufferEmpty(connHandler) ? 0 : InBufferSize(connHandler);
        expSize = RecvListFirstSize(connHandler);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void DpgConnectionHandler::HandleClose(closeType_t closeType)
{
    bool initClose = true;
    if (mCloseDelegate) 
        initClose = mCloseDelegate(closeType);

    if (initClose)
    {
        // once close, remove callback to mCloseDelegate so we dont call it again.
        mCloseDelegate = closeDelegate_t();

        mTxDelegator.Reset(); // turn off send delegates (race condition) -- stats will still be updated, but the flow will not continue if a packet finishes sending after close
        mDelegate = false;
    }
}

///////////////////////////////////////////////////////////////////////////////
