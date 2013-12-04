/// @file
/// @brief DPG Connection-oriented dgram Connection Handler header file
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _DPG_CODGRAM_CONNECTION_HANDLER_H_
#define _DPG_CODGRAM_CONNECTION_HANDLER_H_

#include <app/CodgramSocketHandler.h>
#include "DpgConnectionHandler.h"

class TestDpgCodgramConnectionHandler;

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

class DpgCodgramConnectionHandler : public L4L7_APP_NS::CodgramSocketHandler, public DpgConnectionHandler
{
  public:
    explicit DpgCodgramConnectionHandler(uint32_t serial = 0);
    ~DpgCodgramConnectionHandler();

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
        L4L7_APP_NS::CodgramSocketHandler::PurgeTimers(); 
    }

    bool AsyncSend(messagePtr_t mb, sendDelegate_t delegate) 
    {   
        RegisterSendDelegate(mb->total_length(), delegate);
        return L4L7_APP_NS::CodgramSocketHandler::Send(mb);
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
    
    
  private:
    /// Socket open handler method (from CodgramSocketHandler interface)
    int HandleOpenHook(void);

    /// Socket input handler method (from CodgramSocketHandler interface)
    int HandleInputHook(void);

    friend class TestDpgCodgramConnectionHandler;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
