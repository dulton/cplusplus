/// @file
/// @brief Client Connection Handler header file
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _DPG_CONNECTION_HANDLER_H_
#define _DPG_CONNECTION_HANDLER_H_

#include <deque>
#include <string>
#include <utility>

#include <app/StreamSocketHandler.h>
#include <engine/AbstSocketManager.h>

#include "DpgCommon.h"
#include "CountingDelegator.h"

DECL_DPG_NS

// Forward declarations
class DpgRawStats;

///////////////////////////////////////////////////////////////////////////////

class DpgConnectionHandler
{
  public:
    typedef DpgRawStats stats_t;
    typedef std::tr1::shared_ptr<ACE_Message_Block> messageSharedPtr_t;
    typedef L4L7_ENGINE_NS::AbstSocketManager::connectDelegate_t connectDelegate_t;
    typedef L4L7_ENGINE_NS::AbstSocketManager::closeDelegate_t closeDelegate_t;
    typedef L4L7_ENGINE_NS::AbstSocketManager::sendDelegate_t sendDelegate_t;
    typedef L4L7_ENGINE_NS::AbstSocketManager::recvDelegate_t recvDelegate_t;
    typedef L4L7_ENGINE_NS::AbstSocketManager::CloseType closeType_t;
    
    DpgConnectionHandler();
    virtual ~DpgConnectionHandler();

    /// @brief Register connect/close delegates
    void RegisterDelegates(const connectDelegate_t& conDel, const closeDelegate_t& cloDel) { mConnectDelegate = conDel; mCloseDelegate = cloDel; }

    bool HandleConnect() { return mConnectDelegate(GetSerial()); }

    void HandleClose(closeType_t closeType);

    virtual void ForceClose(closeType_t closeType) = 0;

    virtual void PurgeTimers() = 0;

    virtual bool AsyncSend(messageSharedPtr_t mb, sendDelegate_t delegate) = 0;

    virtual void AsyncRecv(size_t dataLength, recvDelegate_t delegate);

    /// @brief Set stats instance
    void SetStatsInstance(stats_t& stats);

    /// @brief Update Goodput Tx
    void UpdateGoodputTxSent(uint64_t sent);

    /// @brief Called by DpgClientBlock to Set/Get completion (connection status logged) flag
    void SetComplete(bool complete) { mComplete = complete; }
    bool IsComplete() const { return mComplete; }

    /// @brief Update the normally const serial number - be careful
    virtual void SetSerial(uint32_t serial) = 0;

    virtual uint32_t GetSerial() = 0;

    virtual bool GetLocalAddr(ACE_INET_Addr& addr) = 0;

    virtual bool GetRemoteAddr(ACE_INET_Addr& addr) = 0;
    
  protected:
    /// Private state class forward declarations
    struct State;
    struct InitState;

    void RegisterSendDelegate(uint32_t dataLength, const sendDelegate_t&);

    /// Central state change method
    void ChangeState(const State *toState) { mState = toState; }
    
    /// Class data
    static const InitState INIT_STATE;

    stats_t *mStats;                            ///< client/server stats

    messageSharedPtr_t mInBuffer;               ///< input buffer
    const State *mState;                        ///< current state

    bool mComplete;                             ///< conn. complete flag
    bool mDelegate;                             ///< safe to delegate flag

    typedef std::pair<size_t, recvDelegate_t> recvInfo_t;
    std::deque<recvInfo_t> mRecvList;

    connectDelegate_t mConnectDelegate;
    closeDelegate_t mCloseDelegate;

    CountingDelegator<uint64_t, sendDelegate_t> mTxDelegator;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
