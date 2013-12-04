/// @file
/// @brief DPG Stream Connection Handler header file
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _DPG_STREAM_CONNECTION_HANDLER_H_
#define _DPG_STREAM_CONNECTION_HANDLER_H_

#include <app/StreamSocketHandler.h>
#include "DpgConnectionHandler.h"

class TestDpgStreamConnectionHandler;

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

class DpgStreamConnectionHandler : public L4L7_APP_NS::StreamSocketHandler, public DpgConnectionHandler
{
  public:
    explicit DpgStreamConnectionHandler(uint32_t serial = 0);
    ~DpgStreamConnectionHandler();

    void ForceClose(closeType_t closeType)
    {
        // Remove the handler's close notification delegate (informing ourselves of a close that is self-initiated creates unnecessary overhead)
        ClearCloseNotificationDelegate();
        close();
        HandleClose(closeType);
    }

    void PurgeTimers()
    {
        // forward to the proper base
        L4L7_APP_NS::StreamSocketHandler::PurgeTimers(); 
    }

    bool AsyncSend(messagePtr_t mb, sendDelegate_t delegate) 
    {   
        RegisterSendDelegate(mb->total_length(), delegate);
        return L4L7_APP_NS::StreamSocketHandler::Send(mb);
    }

    void SetSerial(uint32_t serial) 
    { 
        const_cast<uint32_t&>(mSerial) = serial; 
    }

    uint32_t GetSerial()
    { 
        return mSerial;
    }

    bool GetLocalAddr(ACE_INET_Addr& addr)
    {
        return (peer().get_local_addr(addr) != -1);
    } 

    bool GetRemoteAddr(ACE_INET_Addr& addr)
    {
        return (peer().get_remote_addr(addr) != -1);
    }

    void CloseNotification(L4L7_APP_NS::StreamSocketHandler& socketHandler);


  private:
    /// Socket open handler method (from StreamSocketHandler interface)
    int HandleOpenHook(void);

    /// Socket input handler method (from StreamSocketHandler interface)
    int HandleInputHook(void);

    friend class TestDpgStreamConnectionHandler;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
