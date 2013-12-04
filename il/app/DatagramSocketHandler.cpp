/// @file
/// @brief Socket (ACE) Handler implementation
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <ace/Message_Block.h>
#include <ace/Reactor.h>
#include <phxexception/PHXException.h>
#include <phxlog/phxlog.h>
#include <utils/MessageUtils.h>

#include "DatagramSocketHandler.h"

USING_L4L7_APP_NS;
///////////////////////////////////////////////////////////////////////////////

DatagramSocketHandler::DatagramSocketHandler(size_t mtu)
    : ACE_Svc_Handler<ACE_SOCK_Dgram, ACE_NULL_SYNCH>(0, 0, 0),
      mMtu(mtu)
{
    this->reference_counting_policy().value(Reference_Counting_Policy::ENABLED);
}

///////////////////////////////////////////////////////////////////////////////

DatagramSocketHandler::~DatagramSocketHandler()
{
    ACE_Reactor *reactor(this->reactor());
    if (reactor)
    {
        this->reactor(0);
    }
}

///////////////////////////////////////////////////////////////////////////////

int DatagramSocketHandler::open(const ACE_INET_Addr& local_addr, ACE_Reactor * reactor)
{
    this->reactor(reactor);
    int result = peer().open(local_addr);
    if (result != 0)
        return result;

    return open((void *)0);
}

///////////////////////////////////////////////////////////////////////////////

int DatagramSocketHandler::open(void *factory)
{
    // Make socket non-blocking
    this->peer().enable(ACE_NONBLOCK);

    // Acquire the lock since event handlers may be dispatched immediately after the register_handler() call below
    ACE_GUARD_RETURN(lock_t, guard, mHandlerLock, -1);
    
    // Register with reactor for input events
    ACE_Reactor *reactor(this->reactor());
    if (!reactor || reactor->register_handler(this, ACE_Event_Handler::READ_MASK) == -1)
        return -1;

    TC_LOG_DEBUG(0, "Opened datagram socket handler " << this);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int DatagramSocketHandler::handle_input(ACE_HANDLE fd)
{
    // Exclude other event handlers and timers
    ACE_GUARD_RETURN(lock_t, guard, mHandlerLock, -1);

    while (1)
    {
        // Allocate a new message block
        messagePtr_t mb = L4L7_UTILS_NS::MessageAlloc(new ACE_Message_Block(mMtu));
        ACE_INET_Addr remote_addr;

        // Receive message block data
        const ssize_t bytesRecv = ReceiveData(mb->wr_ptr(), mb->space(), remote_addr);
        if (bytesRecv < 0)
            return -1;
        else if (bytesRecv == 0)
            return 0;

        mb->wr_ptr(static_cast<size_t>(bytesRecv));

        // Notify user that the socket has input
        if (HandleInputHook(mb, remote_addr) == -1)
            return -1;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int DatagramSocketHandler::handle_output(ACE_HANDLE fd)
{
    // SendData can complete a send, delegate to the engine, and close
    ACE_Event_Handler_var safeHandler(this);
    this->add_reference();

    // Exclude other event handlers and timers
    ACE_GUARD_RETURN(lock_t, guard, mHandlerLock, -1);

    while (!mOutputQueue.empty())
    {
        // Dequeue a message block from the output queue
        messagePtr_t mb(mOutputQueue.front());
        mOutputQueue.pop_front();

        ACE_INET_Addr remote_addr(mAddrQueue.front());
        mAddrQueue.pop_front();

        //
        // Count the number of blocks/bytes in this chain, including continuation blocks.
        // We may be limited by the maximum number of iovec entries per system call or
        // the output block size limit.
        //
        size_t blockCount = 0;
        size_t byteCount = 0;
        ACE_Message_Block *pos = mb.get();
        
        while (pos && blockCount < MAX_IOVEC_COUNT && byteCount < mMtu)
        {
            blockCount++;
            byteCount += pos->length();
            pos = pos->cont();
        }
            
        // Build an array of iovec structures, one for each block
        iovec iov[blockCount];

        pos = mb.get();
        for (size_t i = 0; i < blockCount; i++, pos = pos->cont())
        {
            iov[i].iov_base = pos->rd_ptr();
            iov[i].iov_len = pos->length();
        }

        // Send message
        ssize_t bytesSent = 0;
        // Send message block data
        bytesSent = SendData(iov, blockCount, remote_addr);
            
        if (bytesSent < 0)
            return -1;
        
        if (bytesSent == 0)
        {
            // Requeue the remnant message block in the output queue
            mOutputQueue.push_front(mb);
            mAddrQueue.push_front(remote_addr);
            break;
        }
        // Partially sent packets are partially sent, remainder dropped
    }

    // If outgoing queue is empty, notify user that the socket is writeable again
    if (mOutputQueue.empty() && HandleOutputHook() == -1)
        return -1;
    
    // If output queue is still empty, cancel write event notifications from the reactor
    if (mOutputQueue.empty())
        this->reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int DatagramSocketHandler::handle_close(ACE_HANDLE fd, ACE_Reactor_Mask mask)
{
    TC_LOG_DEBUG(0, "Closing datagram socket handler " << this);

    ACE_Event_Handler_var safeHandler(this);
    this->add_reference();

    // Remove handler from reactor
    ACE_Reactor *reactor(this->reactor());
    if (reactor)
        reactor->remove_handler(this, ACE_Event_Handler::ALL_EVENTS_MASK | ACE_Event_Handler::DONT_CALL);

    // Notify delegate (likely the socket's owner) that the socket is closing
    closeNotificationDelegate_t closeNotificationDelegate;
    PopCloseNotificationDelegate(closeNotificationDelegate);
    if (closeNotificationDelegate)
    {
        closeNotificationDelegate(*this);
    }

    // Shutdown the socket handler
    shutdown();
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

bool DatagramSocketHandler::Send(const std::string& buffer, const ACE_INET_Addr& remote_addr)
{
    // SendData can complete a send, delegate to the engine, and close
    ACE_Event_Handler_var safeHandler(this);
    this->add_reference();

    size_t len = buffer.size();
    ssize_t bytesSent = 0;

    // Try to send immediately as long as output buffer is empty -- this is the ideal case
    if (mOutputQueue.empty() && ((bytesSent = SendData(buffer.data(), len, remote_addr)) == static_cast<ssize_t>(len)))
    {
        reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
        return true;
    }

    if (bytesSent < 0)       
        return false;
       
    len -= bytesSent;

    // Enqueue a new message block in the output queue containing a copy of the unsent data
    messagePtr_t mb = L4L7_UTILS_NS::MessageAlloc(new ACE_Message_Block(len));
    memcpy(mb->base(), buffer.data() + bytesSent, len);
    mb->wr_ptr(len);
    mOutputQueue.push_back(mb);
    mAddrQueue.push_back(remote_addr);

    // Tell the reactor to pay attention to write event notifications
    reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool DatagramSocketHandler::Send(messagePtr_t mb, const ACE_INET_Addr& remote_addr)
{
    // SendData can complete a send, delegate to the engine, and close
    ACE_Event_Handler_var safeHandler(this);
    this->add_reference();

    // Try to send immediately as long as output buffer is empty -- this is the ideal case
    if (mOutputQueue.empty())
    {
        //
        // Count the number of blocks/bytes in this chain, including continuation blocks.
        // We may be limited by the maximum number of iovec entries per system call or
        // the output block size limit.
        //
        size_t blockCount = 0;
        size_t byteCount = 0;
        ACE_Message_Block *pos = mb.get();

        while (pos && blockCount < MAX_IOVEC_COUNT && byteCount < mMtu)
        {
            blockCount++;
            byteCount += pos->length();
            pos = pos->cont();
        }

        // Build an array of iovec structures, one for each block
        iovec iov[blockCount];
        
        pos = mb.get();
        for (size_t i = 0; i < blockCount; i++, pos = pos->cont())
        {
            iov[i].iov_base = pos->rd_ptr();
            iov[i].iov_len = pos->length();
        }
            
        // Send message block data
        ssize_t bytesSent = SendData(iov, blockCount, remote_addr);
        if (bytesSent < 0)
            return false;

        if (bytesSent == 0)
        {
            // Requeue the remnant message block in the output queue
            mOutputQueue.push_front(mb);
            mAddrQueue.push_front(remote_addr);
        }
        else
        {
            // Partially sent packets are partially sent, remainder dropped
            mb.reset();
        }
    }

    if (mb)
    {
        // Enqueue message block in the output queue
        mOutputQueue.push_back(mb);
        mAddrQueue.push_back(remote_addr);
    }

    // Tell the reactor to pay attention to write event notifications
    reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

ssize_t DatagramSocketHandler::SendData(const void *buffer, size_t len, const ACE_INET_Addr& remote_addr)
{
    const ssize_t bytesSent = this->peer().send(buffer, len, remote_addr);

    // send() returns -1 on error
    if (bytesSent < 0)
    {
        if (errno == EAGAIN || errno == EINTR)
            return 0;
        else
        {
            TC_LOG_DEBUG(0, "Datagram socket handler " << this << " encountered error while sending: " << strerror(errno));
            return -1;
        }
    }

    UpdateTxCountHandler(bytesSent, remote_addr);
    return bytesSent;
}

///////////////////////////////////////////////////////////////////////////////

ssize_t DatagramSocketHandler::SendData(const iovec *iov, size_t len, const ACE_INET_Addr& remote_addr)
{
    const ssize_t bytesSent = this->peer().send(iov, (int)len, remote_addr);

    // send() returns -1 on error
    if (bytesSent < 0)
    {
        if (errno == EAGAIN || errno == EINTR)
            return 0;
        else
        {
            TC_LOG_DEBUG(0, "Datagram socket handler " << this << " encountered error while sending: " << strerror(errno));
            return -1;
        }
    }

    UpdateTxCountHandler(bytesSent, remote_addr);
    return bytesSent;
}

///////////////////////////////////////////////////////////////////////////////

ssize_t DatagramSocketHandler::ReceiveData(void *buffer, size_t len, ACE_INET_Addr& remote_addr)
{
    const ssize_t bytesRecv = this->peer().recv(buffer, len, remote_addr);

    // recv() returns -1 on error
    if (bytesRecv == 0)
    {
        TC_LOG_WARN(0, "Datagram socket handler " << this << " got 0 byte datagram");
        return 0;
    }
    else if (bytesRecv < 0)
    {
        if (errno == EAGAIN || errno == EINTR)
            return 0;
        else
        {
            TC_LOG_DEBUG(0, "Datagram socket handler " << this << " encountered error while receiving: " << strerror(errno));
            return -1;
        }
    }

    return bytesRecv;
}

///////////////////////////////////////////////////////////////////////////////

void DatagramSocketHandler::UpdateTxCountHandler(uint64_t sent, const ACE_INET_Addr& remote_addr)
{
    if (!mUpdateTxCountDelegate.empty())
        mUpdateTxCountDelegate(sent, remote_addr);
}

///////////////////////////////////////////////////////////////////////////////

