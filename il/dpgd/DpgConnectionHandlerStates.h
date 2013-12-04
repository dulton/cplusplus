/// @file
/// @brief Server Connection Handler States header file
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _CLIENT_CONNECTION_HANDLER_STATES_H_
#define _CLIENT_CONNECTION_HANDLER_STATES_H_

#include <utils/MessageIterator.h>
#include <utils/MessageUtils.h>

#include "DpgConnectionHandler.h"

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

struct DpgConnectionHandler::State
{
    virtual ~State() { }

    /// @brief Return state name
    virtual std::string Name(void) const = 0;
    
    /// @brief Process protocol input
    virtual bool ProcessInput(DpgConnectionHandler& connHandler, const ACE_Time_Value& tv) const = 0;

    // Input buffer check
    bool InBufferEmpty(const DpgConnectionHandler& connHandler) const { return !connHandler.mInBuffer; }
    
protected:
    // Central state change method
    void ChangeState(DpgConnectionHandler& connHandler, const DpgConnectionHandler::State *toState) const
    {
        connHandler.ChangeState(toState);
    }

    // Input buffer accessors / mutator
    const L4L7_UTILS_NS::MessageIterator InBufferBegin(DpgConnectionHandler& connHandler) const
    {
        return L4L7_UTILS_NS::MessageBegin(connHandler.mInBuffer.get());
    }

    const L4L7_UTILS_NS::MessageIterator InBufferEnd(DpgConnectionHandler& connHandler) const
    {
        return L4L7_UTILS_NS::MessageEnd(connHandler.mInBuffer.get());
    }

    size_t InBufferSize(DpgConnectionHandler& connHandler) const
    {
        return connHandler.mInBuffer->total_length();
    }

    void InBufferTrim(DpgConnectionHandler& connHandler, ACE_Message_Block *mb) const
    {
        connHandler.mInBuffer = L4L7_UTILS_NS::MessageTrim(connHandler.mInBuffer, mb);
    }

    bool RecvListEmpty(DpgConnectionHandler& connHandler) const
    {
        return connHandler.mRecvList.empty();  
    }

    size_t RecvListFirstSize(DpgConnectionHandler& connHandler) const
    {
        return connHandler.mRecvList.empty() ? 0 : connHandler.mRecvList[0].first;
    }

    void RecvListDelegateAndPop(DpgConnectionHandler& connHandler, const uint8_t * data, size_t dataLength) const
    {
        if (connHandler.mDelegate) connHandler.mRecvList[0].second(data, dataLength);
        connHandler.mRecvList.pop_front();
    }

    // Stats accessor
   stats_t *Stats(DpgConnectionHandler& connHandler) const { return connHandler.mStats; }
};

///////////////////////////////////////////////////////////////////////////////

struct DpgConnectionHandler::InitState : public DpgConnectionHandler::State
{
    std::string Name(void) const { return "IDLE"; }
    bool ProcessInput(DpgConnectionHandler& connHandler, const ACE_Time_Value& tv) const;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
