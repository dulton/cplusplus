/// @file
/// @brief DPG Connection-oriented dgram Connection Handler implementation
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <utils/MessageUtils.h>
#include "DpgClientRawStats.h"
#include "DpgDgramSocketHandler.h"

USING_DPG_NS;

DpgDgramSocketHandler::DpgDgramSocketHandler(size_t mtu)
    : L4L7_APP_NS::DatagramSocketHandler(mtu)
{
}

///////////////////////////////////////////////////////////////////////////////

DpgDgramSocketHandler::~DpgDgramSocketHandler()
{
}

///////////////////////////////////////////////////////////////////////////////

int DpgDgramSocketHandler::HandleOpenHook(void)
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int DpgDgramSocketHandler::HandleInputHook(messagePtr_t& msg, const ACE_INET_Addr& remote_addr)
{
    // FIXME - Need a way to remove delegates when the pseudo-socket is closed
    // FIXME - what if we get a packet and get the delegate later?
    mInputList.push_back(msg);
    mInputAddrList.push_back(remote_addr);

    inputListIter_t iIter = mInputList.begin();
    inputAddrListIter_t aIter = mInputAddrList.begin();

    while (iIter != mInputList.end())
    {
        // get the delegate queue for this address
        recvDelegateQueueMapIter_t recvDelQueueIter = mRecvDelegateQueueMap.find(*aIter);

        if (recvDelQueueIter == mRecvDelegateQueueMap.end() ||
            recvDelQueueIter->second.empty())
        {
            if (mDefaultRecvDelegate)
            {
                mDefaultRecvDelegate(*aIter);

                // check again
                recvDelQueueIter = mRecvDelegateQueueMap.find(*aIter);
                if (recvDelQueueIter == mRecvDelegateQueueMap.end() ||
                    recvDelQueueIter->second.empty())
                {
                    // not found or empty, save for later
                    ++iIter;
                    ++aIter;
                    continue;
                }
            }
            else
            {
                // not found or empty, save for later
                ++iIter;
                ++aIter;
                continue;
            }
        }

        // pop the delegate
        recvDelegate_t delegate = recvDelQueueIter->second.front();
        recvDelQueueIter->second.pop_front();

        // assume datagrams always come one to a message block
        delegate((uint8_t *)((*iIter)->rd_ptr()), (*iIter)->total_length());

        // remove packet and address from the input list
        iIter = mInputList.erase(iIter);
        aIter = mInputAddrList.erase(aIter);
    }
    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int DpgDgramSocketHandler::HandleOutputHook()
{
    size_t outputQueueSize = GetOutputQueueSize();

    while (outputQueueSize < mSendDelegateQueue.size())
    {
        sendDelegate_t delegate = mSendDelegateQueue.front();
        mSendDelegateQueue.pop_front();
        delegate();
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void DpgDgramSocketHandler::CancelRecv(const ACE_INET_Addr& remoteAddr)
{
  recvDelegateQueueMapIter_t recvDelQueueIter = mRecvDelegateQueueMap.find(remoteAddr);
  if (recvDelQueueIter != mRecvDelegateQueueMap.end())
  {
    recvDelQueueIter->second.clear();
  }
}