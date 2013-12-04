/// @file
/// @brief Client Connection Handler header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _CLIENT_CONNECTION_HANDLER_H_
#define _CLIENT_CONNECTION_HANDLER_H_

#include <deque>
#include <string>
#include <utility>

#include <app/StreamSocketHandler.h>

#include "HttpCommon.h"

DECL_HTTP_NS

// Forward declarations
class HttpClientRawStats;

///////////////////////////////////////////////////////////////////////////////

class ClientConnectionHandler : public L4L7_APP_NS::StreamSocketHandler
{
  public:
    typedef Http_1_port_server::HttpVersionOptions version_t;
    typedef HttpClientRawStats stats_t;
    
    explicit ClientConnectionHandler(uint32_t serial = 0);
    ~ClientConnectionHandler();

    /// @brief Set client version
    void SetVersion(version_t version) { mVersion = version; }

    /// @brief Set keep alive
    void SetKeepAlive(bool enable) { mKeepAlive = enable; }
    
    /// @brief Set user agent
    void SetUserAgent(const std::string& userAgent) { mUserAgent = userAgent; }

    /// @brief Set maximum number of requests
    void SetMaxRequests(uint32_t maxRequests) { mMaxRequests = std::max(maxRequests, 1U); }

    /// @brief Set max pipeline depth
    void SetMaxPipelineDepth(uint32_t maxPipelineDepth) { mMaxPipelineDepth = std::max(maxPipelineDepth, 1U); }

    /// @brief Set stats instance
    void SetStatsInstance(stats_t& stats);

    /// @brief Update Goodput Tx
    void UpdateGoodputTxSent(uint64_t sent);

    /// @brief Called by HttpClientBlock to Set/Get completion (connection status logged) flag
    void SetComplete(bool complete) { mComplete = complete; }
    bool IsComplete() const { return mComplete; }
    
    /// @brief called by HttoClientBlock to Set StopFlag
    void SetStopFlag(bool stopFlag) {mStopFlag = stopFlag;}
    
     
  private:
    /// Private state class forward declarations
    struct State;
    struct IdleState;
    struct HeaderState;
    struct BodyState;

    /// Private request record class
    struct RequestRecord;

    /// Requests remaining convenience method
    bool RequestsRemaining(void) const { return mKeepAlive ? (mRequestCount < mMaxRequests) : (mRequestCount < 1); }

    /// Send requests
    bool SendRequests(uint32_t maxRequests);

    /// Socket open handler method (from StreamSocketHandler interface)
    int HandleOpenHook(void);

    /// Socket input handler method (from StreamSocketHandler interface)
    int HandleInputHook(void);

    /// Central state change method
    void ChangeState(const State *toState) { mState = toState; }
    
    /// Class data
    static const IdleState IDLE_STATE;
    static const HeaderState HEADER_STATE;
    static const BodyState BODY_STATE;

    version_t mVersion;                         ///< client version
    bool mKeepAlive;                            ///< use keep alive
    std::string mUserAgent;                     ///< user agent header
    uint32_t mMaxRequests;                      ///< max number of requests to allow
    uint32_t mMaxPipelineDepth;                 ///< max pipeline depth

    stats_t *mStats;                            ///< client stats

    messagePtr_t mInBuffer;                     ///< input buffer
    const State *mState;                        ///< current state

    std::string mPeerAddrStr;                   ///< IP address of our peer in string format
    uint32_t mRequestCount;                     ///< number of requests sent thus far
    size_t mContentLength;                      ///< remaining content length
    bool mCloseFlag;                            ///< close connection after this request?
    bool mStopFlag;                           ///< stop sending request 
    bool mComplete;                             ///< conn. complete flag

    std::deque<RequestRecord> mReqQueue;        ///< queue of pending requests
    std::deque<uint16_t> mRespStatusCode;       ///< queue of response categories
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_HTTP_NS

#endif
