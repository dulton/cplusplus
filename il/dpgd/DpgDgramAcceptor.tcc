/// @file
/// @brief DPG Datagram Acceptor file
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///
///  Builds an acceptor on top of the dpg connection handler

#ifndef _DPG_DGRAM_ACCEPTOR_TCC_
#define _DPG_DGRAM_ACCEPTOR_TCC_

#include <set>
#include <boost/bind.hpp>
#include <Tr1Adapter.h>

#include "DpgDgramSocketHandler.h"

class TestDpgDgramAcceptor;

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

template<class PseudoHandlerType>
class DpgDgramAcceptor : public DpgDgramSocketHandler
{
  public:
    typedef boost::function1<bool, PseudoHandlerType &> acceptNotificationDelegate_t;

    explicit DpgDgramAcceptor(size_t mtu) : DpgDgramSocketHandler(mtu) 
    {
        SetDefaultRecvDelegate(boost::bind(&DpgDgramAcceptor<PseudoHandlerType>::HandleDefaultRecv, this, _1));
    }

    ~DpgDgramAcceptor() {}

    void SetAcceptNotificationDelegate(acceptNotificationDelegate_t delegate) { mAcceptNotificationDelegate = delegate; }

  protected:
    acceptNotificationDelegate_t mAcceptNotificationDelegate;
    
  private:
    /// Un-registered packet received
    void HandleDefaultRecv(const ACE_INET_Addr& remote_addr);

    typedef std::set<ACE_INET_Addr> handlerSet_t;
    typedef handlerSet_t::iterator handlerIter_t;
    handlerSet_t mHandlerSet;

    friend class TestDpgDgramAcceptor;
};

///////////////////////////////////////////////////////////////////////////////

template<class PseudoHandlerType>
void DpgDgramAcceptor<PseudoHandlerType>::HandleDefaultRecv(const ACE_INET_Addr& remote_addr)
{
    handlerIter_t iter = mHandlerSet.find(remote_addr);
    if (iter != mHandlerSet.end())
        return; // already existing handler - keep packet until requested

    std::auto_ptr<PseudoHandlerType> handler(new PseudoHandlerType(this));
    handler->SetRemoteAddr(remote_addr);
    
    if(mAcceptNotificationDelegate && mAcceptNotificationDelegate(*handler))
    {
        handler.release();
    }
}

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
