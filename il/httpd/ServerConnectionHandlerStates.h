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

#ifndef _SERVER_CONNECTION_HANDLER_STATES_H_
#define _SERVER_CONNECTION_HANDLER_STATES_H_

#include <utils/MessageIterator.h>

#include "ServerConnectionHandler.h"

DECL_HTTP_NS

///////////////////////////////////////////////////////////////////////////////

struct ServerConnectionHandler::State
{
    virtual ~State() { }

    /// @brief Return state name
    virtual std::string Name(void) const = 0;
    
    /// @brief Process protocol input
    virtual void ProcessInput(ServerConnectionHandler& connHandler, const L4L7_UTILS_NS::MessageConstIterator& begin, const L4L7_UTILS_NS::MessageConstIterator& end, const ACE_Time_Value& tv) const = 0;

    /// @brief Connection close check
    virtual bool ConnectionShouldClose(ServerConnectionHandler& connHandler) const = 0;
    
protected:
    // Central state change method
    void ChangeState(ServerConnectionHandler& connHandler, const ServerConnectionHandler::State *toState) const
    {
        connHandler.ChangeState(toState);
    }

    // Connection handler member accessors
    bodyType_t GetBodyType(const ServerConnectionHandler& connHandler) const { return connHandler.mBodyType; }
    uint32_t GetBodySize(ServerConnectionHandler& connHandler) const { return connHandler.GetBodySize(); }
    ACE_Time_Value GetResponseLatency(ServerConnectionHandler& connHandler) const { return connHandler.GetResponseLatency(); }
    bool GetCloseFlag(const ServerConnectionHandler& connHandler) const { return connHandler.mCloseFlag; }
    bool OutputQueueEmpty(const ServerConnectionHandler& connHandler) const { return !connHandler.HasOutputData(); }
    
    // Connection handler member mutators
    void SetCloseFlag(ServerConnectionHandler& connHandler, bool closeFlag) const { connHandler.mCloseFlag = closeFlag; }

    // Response queue methods
    bool ResponseQueueEmpty(const ServerConnectionHandler& connHandler) const
    {
        return connHandler.mRespQueue.empty();
    }
    
    void QueueResponse(ServerConnectionHandler& connHandler, const ACE_Time_Value& absTime, HttpProtocol::StatusCode errorCode, bool closeFlag) const
    {
        connHandler.QueueResponse(absTime, errorCode, closeFlag);
    }
    
    void QueueResponse(ServerConnectionHandler& connHandler, const ACE_Time_Value& absTime, bodyType_t bodyType, uint32_t bodySize, bool closeFlag, std::string uri) const
    {
        connHandler.QueueResponse(absTime, bodyType, bodySize, closeFlag, uri);
    }
    
    // Request counting methods
    void RequestCompleted(ServerConnectionHandler& connHandler) const
    {
        connHandler.mRequestCount++;
    }
    
    bool RequestsRemaining(const ServerConnectionHandler& connHandler) const
    {
        return (connHandler.mMaxRequests && connHandler.mRequestCount >= connHandler.mMaxRequests) ? false : true;
    }
};

///////////////////////////////////////////////////////////////////////////////

struct ServerConnectionHandler::IdleState : public ServerConnectionHandler::State
{
    std::string Name(void) const { return "IDLE"; }
    void ProcessInput(ServerConnectionHandler& connHandler, const L4L7_UTILS_NS::MessageConstIterator& begin, const L4L7_UTILS_NS::MessageConstIterator& end, const ACE_Time_Value& tv) const;
    bool ConnectionShouldClose(ServerConnectionHandler& connHandler) const;
};

///////////////////////////////////////////////////////////////////////////////

struct ServerConnectionHandler::HeaderState : public ServerConnectionHandler::State
{
    std::string Name(void) const { return "HEADER"; }
    void ProcessInput(ServerConnectionHandler& connHandler, const L4L7_UTILS_NS::MessageConstIterator& begin, const L4L7_UTILS_NS::MessageConstIterator& end, const ACE_Time_Value& tv) const;
    bool ConnectionShouldClose(ServerConnectionHandler& connHandler) const;
};

///////////////////////////////////////////////////////////////////////////////

struct ServerConnectionHandler::FlushState : public ServerConnectionHandler::State
{
    std::string Name(void) const { return "FLUSH"; }
    void ProcessInput(ServerConnectionHandler& connHandler, const L4L7_UTILS_NS::MessageConstIterator& begin, const L4L7_UTILS_NS::MessageConstIterator& end, const ACE_Time_Value& tv) const;
    bool ConnectionShouldClose(ServerConnectionHandler& connHandler) const;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_HTTP_NS

#endif
