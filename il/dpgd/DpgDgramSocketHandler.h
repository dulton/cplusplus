/// @file
/// @brief DPG Datagram Connection Handler header file
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///
///  A datagram connection has a single local address but receives
///  packets from multiple remote addresses

#ifndef _DPG_DGRAM_CONNECTION_HANDLER_H_
#define _DPG_DGRAM_CONNECTION_HANDLER_H_

#include <deque>
#include <list>

#include <boost/function.hpp>

#include <app/DatagramSocketHandler.h>
#include <engine/AbstSocketManager.h>

#include "DpgCommon.h"

class TestDpgDgramSocketHandler;

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

class DpgDgramSocketHandler : public L4L7_APP_NS::DatagramSocketHandler
{
  public:
    typedef L4L7_ENGINE_NS::AbstSocketManager::sendDelegate_t sendDelegate_t;
    typedef L4L7_ENGINE_NS::AbstSocketManager::recvDelegate_t recvDelegate_t;
    typedef boost::function1<void, const ACE_INET_Addr&> defaultRecvDelegate_t;

    explicit DpgDgramSocketHandler(size_t mtu);
    ~DpgDgramSocketHandler();

    bool AsyncSend(messagePtr_t mb, const ACE_INET_Addr& remote_addr, const sendDelegate_t& delegate) 
    {
        mSendDelegateQueue.push_back(delegate);
        return L4L7_APP_NS::DatagramSocketHandler::Send(mb, remote_addr);
    }

    void AsyncRecv(const ACE_INET_Addr& remoteAddr, recvDelegate_t delegate)
    {
        mRecvDelegateQueueMap[remoteAddr].push_back(delegate);
    }

    bool GetLocalAddr(ACE_INET_Addr& addr)
    {
        return (peer().get_local_addr(addr) != -1);
    }

    void CancelRecv(const ACE_INET_Addr& remoteAddr);

    /// @brief Set the new address event delegate
    void SetDefaultRecvDelegate(defaultRecvDelegate_t delegate) { mDefaultRecvDelegate = delegate; }

  private:
    /// Socket open handler method (from DatagramSocketHandler interface)
    int HandleOpenHook(void);

    /// Socket input handler method (from DatagramSocketHandler interface)
    int HandleInputHook(messagePtr_t& msg, const ACE_INET_Addr& addr);

    /// Socket output handler method (from DatagramSocketHandler interface)
    int HandleOutputHook(); 

    typedef std::deque<sendDelegate_t> sendDelegateQueue_t;
    sendDelegateQueue_t mSendDelegateQueue;

    typedef std::deque<recvDelegate_t> recvDelegateQueue_t;
    typedef std::map<ACE_INET_Addr,recvDelegateQueue_t> recvDelegateQueueMap_t;
    typedef recvDelegateQueueMap_t::iterator recvDelegateQueueMapIter_t;
    recvDelegateQueueMap_t mRecvDelegateQueueMap;

    typedef std::list<messagePtr_t> inputList_t;
    typedef inputList_t::iterator inputListIter_t;
    typedef std::list<ACE_INET_Addr> inputAddrList_t;
    typedef inputAddrList_t::iterator inputAddrListIter_t;
    inputList_t mInputList;
    inputAddrList_t mInputAddrList; // must always be the same size as above

    defaultRecvDelegate_t mDefaultRecvDelegate;

    friend class TestDpgDgramSocketHandler;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
