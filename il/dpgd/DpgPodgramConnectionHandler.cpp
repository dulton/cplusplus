/// @file
/// @brief DPG pseudo-connection-oriented dgram Connection Handler impl
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include "DpgDgramAcceptor.tcc"
#include "DpgRawStats.h"
#include "DpgPodgramConnectionHandler.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

DpgPodgramConnectionHandler::DpgPodgramConnectionHandler(socket_t* socket) : 
    mSerial(0),
    mSocket(socket)
{
    mSocket->RegisterUpdateTxCountDelegate(boost::bind(&DpgPodgramConnectionHandler::UpdateGoodputTxSent, this, _1));
    mSocket->add_reference();
    this->reference_counting_policy().value(Reference_Counting_Policy::ENABLED);
}

///////////////////////////////////////////////////////////////////////////////

DpgPodgramConnectionHandler::~DpgPodgramConnectionHandler()
{
    mSocket->UnregisterUpdateTxCountDelegate();
    mSocket->remove_reference();
} 

///////////////////////////////////////////////////////////////////////////////

bool DpgPodgramConnectionHandler::GetLocalAddr(ACE_INET_Addr& addr)
{
    return mSocket->GetLocalAddr(addr);
}

///////////////////////////////////////////////////////////////////////////////

bool DpgPodgramConnectionHandler::AsyncSend(messagePtr_t mb, sendDelegate_t delegate) 
{
    return mSocket->AsyncSend(mb, mRemoteAddr, delegate);
}

///////////////////////////////////////////////////////////////////////////////

void DpgPodgramConnectionHandler::AsyncRecv(size_t dataLength, recvDelegate_t delegate) 
{
    // We want to update stats on recv, so add a thunk
    recvDelegate_t thunk = boost::bind(&DpgPodgramConnectionHandler::HandleRecv, this, delegate, _1, _2);
    return mSocket->AsyncRecv(mRemoteAddr, thunk);
}

///////////////////////////////////////////////////////////////////////////////

void DpgPodgramConnectionHandler::HandleRecv(recvDelegate_t& delegate, const uint8_t * buf, size_t len)
{
    // Update receive stats
    if (mStats)
    {
        const size_t rxBytes = len;
        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
        mStats->goodputRxBytes += rxBytes;
    }

    delegate(buf, len);
}

///////////////////////////////////////////////////////////////////////////////


void DpgPodgramConnectionHandler::HandleClose(closeType_t closeType)
{
  mTxDelegator.Reset(); // turn off send delegates (race condition) -- stats will still be updated, but the flow will not continue if a packet finishes sending after close
  if (mCloseDelegate) mCloseDelegate(closeType);
  mSocket->CancelRecv(mRemoteAddr);
}

///////////////////////////////////////////////////////////////////////////////
