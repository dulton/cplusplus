/// @file
/// @brief Server Connection Handler States header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
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

#include "ClientConnectionHandler.h"

DECL_HTTP_NS

///////////////////////////////////////////////////////////////////////////////

struct ClientConnectionHandler::State
{
    virtual ~State() { }

    /// @brief Return state name
    virtual std::string Name(void) const = 0;
    
    /// @brief Process protocol input
    virtual bool ProcessInput(ClientConnectionHandler& connHandler, const ACE_Time_Value& tv) const = 0;

protected:
    // Central state change method
    void ChangeState(ClientConnectionHandler& connHandler, const ClientConnectionHandler::State *toState) const
    {
        connHandler.ChangeState(toState);
    }

    // Input buffer accessors / mutator
    bool InBufferEmpty(const ClientConnectionHandler& connHandler) const { return !connHandler.mInBuffer; }
    
    const L4L7_UTILS_NS::MessageIterator InBufferBegin(ClientConnectionHandler& connHandler) const
    {
        return L4L7_UTILS_NS::MessageBegin(connHandler.mInBuffer.get());
    }

    const L4L7_UTILS_NS::MessageIterator InBufferEnd(ClientConnectionHandler& connHandler) const
    {
        return L4L7_UTILS_NS::MessageEnd(connHandler.mInBuffer.get());
    }

    void InBufferTrim(ClientConnectionHandler& connHandler, ACE_Message_Block *mb) const
    {
        connHandler.mInBuffer = L4L7_UTILS_NS::MessageTrim(connHandler.mInBuffer, mb);
    }

    // Stats accessor
    stats_t *Stats(ClientConnectionHandler& connHandler) const { return connHandler.mStats; }

    // Content-length accessor / mutator
    void SetContentLength(ClientConnectionHandler& connHandler, size_t contentLength) const { connHandler.mContentLength = contentLength; }
    size_t GetContentLength(const ClientConnectionHandler& connHandler) const { return connHandler.mContentLength; }

    // Request methods
    void SetCloseFlag(ClientConnectionHandler& connHandler, bool closeFlag) const { connHandler.mCloseFlag = closeFlag; }
    bool GetCloseFlag(const ClientConnectionHandler& connHandler) const { return connHandler.mCloseFlag; }
    
    /// Logic connection  control method
    bool GetStopFlag(const ClientConnectionHandler& connHandler) const { return connHandler.mStopFlag; }
    
    bool RequestsRemaining(const ClientConnectionHandler& connHandler) const { return connHandler.RequestsRemaining(); }
    bool SendRequest(ClientConnectionHandler& connHandler) const { return connHandler.SendRequests(1); }

    // Request queue accessors / mutator
    bool RequestQueueEmpty(const ClientConnectionHandler& connHandler) const { return connHandler.mReqQueue.empty(); }
    const RequestRecord& RequestQueueFront(const ClientConnectionHandler& connHandler) const { return connHandler.mReqQueue.front(); }
    void RequestQueuePop(ClientConnectionHandler& connHandler) const { connHandler.mReqQueue.pop_front(); }
    
    // Response queue accessors
    void RespStatusQueueAdd(ClientConnectionHandler& connHandler, const uint16_t& statusCode) const { connHandler.mRespStatusCode.push_back(statusCode); }
    const uint16_t RespStatusQueueFront(const ClientConnectionHandler& connHandler) const { return connHandler.mRespStatusCode.front(); }
    void RespStatusQueuePop(ClientConnectionHandler& connHandler) const { connHandler.mRespStatusCode.pop_front(); }
};

///////////////////////////////////////////////////////////////////////////////

struct ClientConnectionHandler::IdleState : public ClientConnectionHandler::State
{
    std::string Name(void) const { return "IDLE"; }
    bool ProcessInput(ClientConnectionHandler& connHandler, const ACE_Time_Value& tv) const;
};

///////////////////////////////////////////////////////////////////////////////

struct ClientConnectionHandler::HeaderState : public ClientConnectionHandler::State
{
    std::string Name(void) const { return "HEADER"; }
    bool ProcessInput(ClientConnectionHandler& connHandler, const ACE_Time_Value& tv) const;
};

///////////////////////////////////////////////////////////////////////////////

struct ClientConnectionHandler::BodyState : public ClientConnectionHandler::State
{
    std::string Name(void) const { return "BODY"; }
    bool ProcessInput(ClientConnectionHandler& connHandler, const ACE_Time_Value& tv) const;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_HTTP_NS

#endif
