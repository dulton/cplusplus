/// @file
/// @brief DPG pseudo-connection-oriented dgram Connection Handler header file
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _DPG_PODGRAM_CONNECTION_HANDLER_H_
#define _DPG_PODGRAM_CONNECTION_HANDLER_H_

#include <ace/Event_Handler.h>
#include "DpgConnectionHandler.h"

class TestDpgPodgramConnectionHandler;

DECL_DPG_NS

template<class Connector> class DpgDgramAcceptor;

///////////////////////////////////////////////////////////////////////////////

class DpgPodgramConnectionHandler : public DpgConnectionHandler, public ACE_Event_Handler /* for reference counting */
{
  public:
    typedef messageSharedPtr_t messagePtr_t;
    typedef DpgDgramAcceptor<DpgPodgramConnectionHandler> socket_t;

    explicit DpgPodgramConnectionHandler(socket_t* socket);

    ~DpgPodgramConnectionHandler();

    void ForceClose(closeType_t closeType)
    {
        // FIXME - delegate to socket
        //
        // Remove the handler's close notification delegate (informing ourselves of a close that is self-initiated creates unnecessary overhead)
        // ClearCloseNotificationDelegate();
        // close();
        HandleClose(closeType);
    }

    void PurgeTimers()
    {
        // no timers; do nothing
    }

    bool AsyncSend(messagePtr_t mb, sendDelegate_t delegate);

    void AsyncRecv(size_t dataLength, recvDelegate_t delegate);

    void SetSerial(uint32_t serial) 
    { 
        const_cast<uint32_t&>(mSerial) = serial; 
    }

    uint32_t GetSerial()
    { 
        return mSerial;
    }

    bool GetLocalAddr(ACE_INET_Addr& addr);

    void SetRemoteAddr(const ACE_INET_Addr& addr)
    {
        mRemoteAddr = addr;
        mHasRemoteAddr = true;
    } 

    bool GetRemoteAddr(ACE_INET_Addr& addr)
    {
        if (mHasRemoteAddr)
        {
            addr = mRemoteAddr;
            return true;
        }

        return false;
    }

    void HandleClose(closeType_t closeType);

  private:
    /// Socket open handler method (from CodgramSocketHandler interface)
    int HandleOpenHook(void)
    {
        return 0;
    }

    /// Socket input handler method (from CodgramSocketHandler interface)
    int HandleInputHook(void)
    {
        // FIXME - delegate
        return 0;
    }

    void HandleRecv(recvDelegate_t& delegate, const uint8_t *, size_t);

    uint32_t mSerial;
    socket_t *mSocket;

    ACE_INET_Addr mRemoteAddr;
    bool mHasLocalAddr;
    bool mHasRemoteAddr;
        
    friend class TestDpgPodgramConnectionHandler;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
