/// @file
/// @brief Socket manager abstract base class (to be implemented by engine user)
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ABST_SOCKET_MANAGER_H_
#define _ABST_SOCKET_MANAGER_H_

#include <boost/function.hpp>

#include <engine/EngineCommon.h>

DECL_L4L7_ENGINE_NS

class PlaylistInstance;

///////////////////////////////////////////////////////////////////////////////

class AbstSocketManager
{
  public:
    typedef boost::function1<bool, int> connectDelegate_t;
    enum CloseType { TX_FIN, TX_RST, RX_FIN, RX_RST, CON_ERR };
    typedef boost::function1<bool, CloseType> closeDelegate_t;
    typedef boost::function0<void> sendDelegate_t;
    typedef boost::function2<void, const uint8_t *, size_t> recvDelegate_t;

    virtual ~AbstSocketManager(){};

    virtual int AsyncConnect(PlaylistInstance * pi, uint16_t serverPort, bool isTcp, int flowIdx, connectDelegate_t, closeDelegate_t) = 0;

    virtual void Listen(PlaylistInstance * pi, uint16_t serverPort, bool isTcp, int flowIdx, connectDelegate_t, closeDelegate_t) = 0;

    virtual void AsyncSend(int fd, const uint8_t * data, size_t dataLength, sendDelegate_t) = 0;

    virtual void AsyncRecv(int fd, size_t dataLength, recvDelegate_t) = 0;

    virtual void Close(int fd, bool reset) = 0;

    virtual void Clean(int fd) = 0;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
