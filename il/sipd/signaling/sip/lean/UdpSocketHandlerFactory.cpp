/// @file
/// @brief UDP socket handler factory
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include "UdpSocketHandler.h"
#include "UdpSocketHandlerFactory.h"

USING_SIPLEAN_NS;

///////////////////////////////////////////////////////////////////////////////

DECL_SIPLEAN_NS

ACE_TSS<UdpSocketHandlerFactory> TSS_UdpSocketHandlerFactory;

END_DECL_SIPLEAN_NS

///////////////////////////////////////////////////////////////////////////////

UdpSocketHandlerFactory::sendDelegate_t UdpSocketHandlerFactory::mSender;

UdpSocketHandlerFactory::udpSocketHandlerSharedPtr_t UdpSocketHandlerFactory::Make(ACE_Reactor& reactor, uint16_t portNumber)
{
    udpSocketHandlerSharedPtr_t sock;

    portMap_t::const_iterator mapIter = mPortMap.find(portNumber);
    if (mapIter != mPortMap.end())
    {
        try
        {
            sock = udpSocketHandlerSharedPtr_t(mapIter->second);
        }
        catch (const std::tr1::bad_weak_ptr&) 
        {
        }
    }

    if (!sock)
    {
        sock = udpSocketHandlerSharedPtr_t(new UdpSocketHandler(reactor, portNumber));
        sock->SetSendDelegate(mSender);
        mPortMap[portNumber] = sock;
    }

    return sock;
}

///////////////////////////////////////////////////////////////////////////////
