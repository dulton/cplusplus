/// @file
/// @brief HTTP Client Socket (ACE) Handler implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///
#include <sstream>

#include <ace/Message_Block.h>
#include <ace/Reactor.h>
#include <ace/Timer_Queue.h>
#include <boost/scoped_ptr.hpp>
#include <phxexception/PHXException.h>
#include <phxlog/phxlog.h>
#include <utils/MessageUtils.h>

#include "StreamSocketHandler.h"

USING_L4L7_APP_NS;
///////////////////////////////////////////////////////////////////////////////

StreamSocketHandler::StreamSocketHandler(uint32_t serial)
    : ACE_Svc_Handler<ACE_SOCK_Stream, ACE_NULL_SYNCH>(0, 0, 0),
      mSerial(serial),
      mErrInfo(0),
      mIsPending(false),
      mIsOpen(false),
      mIsClientBwCtrl(false),
      mInputBlockSize(DEFAULT_INPUT_BLOCK_SIZE),
      mOutputBlockSize(0),
      mTimerQueue(new IL_DAEMON_LIB_NS::TimerQueue),
      mTimerId(INVALID_TIMER_ID),
      mNextTimerExpires(ACE_Time_Value::zero)
{
    this->reference_counting_policy().value(Reference_Counting_Policy::ENABLED);
}

///////////////////////////////////////////////////////////////////////////////

StreamSocketHandler::~StreamSocketHandler()
{
    ACE_Reactor *reactor(this->reactor());
    if (reactor)
    {
        if (mTimerId != INVALID_TIMER_ID)
            reactor->cancel_timer(mTimerId);

        this->reactor(0);
    }
}

///////////////////////////////////////////////////////////////////////////////

int StreamSocketHandler::open(void *factory)
{
    // If output block size hasn't been set, default to the SO_SNDBUF value
    if (!mOutputBlockSize && !this->GetSoSndBuf(mOutputBlockSize))
        return -1;
    
    // Make socket non-blocking
    this->peer().enable(ACE_NONBLOCK);

    // Acquire the lock since event handlers may be dispatched immediately after the register_handler() call below
    ACE_GUARD_RETURN(lock_t, guard, mHandlerLock, -1);
    
    // Socket handler is now considered open
    mIsOpen = true;
    
    // Register with reactor for input and output events
    ACE_Reactor *reactor(this->reactor());
    if (!reactor || reactor->register_handler(this, ACE_Event_Handler::READ_MASK | ACE_Event_Handler::WRITE_MASK) == -1)
        return -1;

    // Notify user that the socket is open
    if (HandleOpenHook() == -1)
        return -1;

    TC_LOG_DEBUG(0, "Opened stream socket handler " << this);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int StreamSocketHandler::handle_input(ACE_HANDLE fd)
{
    size_t bytesNeeded = 0;

    // Exclude other event handlers and timers
    ACE_GUARD_RETURN(lock_t, guard, mHandlerLock, -1);

    // Handle non BW controlled inputs
    if (!mIsClientBwCtrl)
        return handle_input_reg();

    // Handle BW controlled inputs
    bytesNeeded = ProduceDynamicLoadBytesHandler(true, ACE_OS::gettimeofday());

    if (bytesNeeded <= 0)
        return 0;

    return handle_input_bw(bytesNeeded);
}

///////////////////////////////////////////////////////////////////////////////

int StreamSocketHandler::handle_input_reg()
{
    while (1)
    {
        // Allocate a new message block
        messagePtr_t mb = L4L7_UTILS_NS::MessageAlloc(new ACE_Message_Block(mInputBlockSize));

        // Receive message block data
        const ssize_t bytesRecv = ReceiveData(mb->wr_ptr(), mb->space());
        if (bytesRecv < 0)
            return -1;
        else if (bytesRecv == 0)
            return 0;

        mb->wr_ptr(static_cast<size_t>(bytesRecv));

        // Enqueue message block in the input queue
        mInputQueue.push_back(mb);

        // Notify user that the socket has input
        if (HandleInputHook() == -1)
            return -1;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int StreamSocketHandler::handle_input_bw(size_t bytesNeeded)
{
    int ret = 0;
    size_t bytesToRead = mInputBlockSize;

    while (bytesNeeded > 0)
    {
        // Update input block size for this read
        if (mInputBlockSize > bytesNeeded)
            bytesToRead = bytesNeeded;

        // Allocate a new message block
        messagePtr_t mb = L4L7_UTILS_NS::MessageAlloc(new ACE_Message_Block(bytesToRead));

        // Receive message block data
        const ssize_t bytesRecv = ReceiveData(mb->wr_ptr(), mb->space());
        
        if (bytesRecv > 0)
        {
            bytesNeeded = ConsumeDynamicLoadBytesHandler(true, bytesRecv);
        }
        else if (bytesRecv < 0)
        {
            ret = -1;
            break;
        }
        else if (bytesRecv == 0)
        {
            ret = 0;
            break;
        }

        mb->wr_ptr(static_cast<size_t>(bytesRecv));

        // Enqueue message block in the input queue
        mInputQueue.push_back(mb);

        // Notify user that the socket has input
        if (HandleInputHook() == -1)
        {
            ret = -1;
            break;
        }
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

int StreamSocketHandler::handle_output(ACE_HANDLE fd)
{
    // SendData can complete a send, delegate to the engine, and close
    ACE_Event_Handler_var safeHandler(this);
    this->add_reference();

    size_t bytesNeeded = 0;

    // Exclude other event handlers and timers
    ACE_GUARD_RETURN(lock_t, guard, mHandlerLock, -1);

    // Handle non BW controlled outputs
    if (!mIsClientBwCtrl)
        return handle_output_send(bytesNeeded, false);

    // Handle BW controlled inputs
    bytesNeeded = ProduceDynamicLoadBytesHandler(false, ACE_OS::gettimeofday());

    if (bytesNeeded <= 0)
        return 0;

    return handle_output_send(bytesNeeded, true);
}

///////////////////////////////////////////////////////////////////////////////

int StreamSocketHandler::handle_output_send(size_t bytesNeeded, bool bandwidthSend)
{
    // Exclude other event handlers and timers
    ACE_GUARD_RETURN(lock_t, guard, mHandlerLock, -1);
    
    while (!mOutputQueue.empty())
    {
        // Dequeue a message block from the output queue
        messagePtr_t mb(mOutputQueue.front());
        mOutputQueue.pop_front();

        //
        // Count the number of blocks/bytes in this chain, including continuation blocks.
        // We may be limited by the maximum number of iovec entries per system call or
        // the output block size limit.
        //
        size_t blockCount = 0;
        size_t byteCount = 0;
        ACE_Message_Block *pos = mb.get();
        
        while (pos && blockCount < MAX_IOVEC_COUNT && byteCount < mOutputBlockSize)
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
        if (byteCount <= bytesNeeded || !bandwidthSend)
        {
            // Send message block data
            bytesSent = SendData(iov, blockCount);
            
            if (bytesSent < 0)
                return -1;
            if (bandwidthSend)
                bytesNeeded = ConsumeDynamicLoadBytesHandler(false, bytesSent);
        }

        // The iovec's were partially sent?
        while (bytesSent > 0)
        {
            if (mb->length() <= static_cast<size_t>(bytesSent))
            {
                // Block was completely sent, strip it from the chain and free it
                bytesSent -= mb->length();
                messagePtr_t temp = L4L7_UTILS_NS::MessageAlloc(mb->cont());
                mb->cont(0);
                mb = temp;
            }
            else
            {
                // Block was partially sent, update its read pointer and enqueue the remainder
                mb->rd_ptr(static_cast<size_t>(bytesSent));
                break;
            }
        }

        if (mb)
        {
            // Requeue the remnant message block in the output queue
            mOutputQueue.push_front(mb);
            break;
        }

        // Loop through once when using bandwidth
        if (bandwidthSend)
            break;
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

int StreamSocketHandler::handle_close(ACE_HANDLE fd, ACE_Reactor_Mask mask)
{
    TC_LOG_DEBUG(0, "Closing stream socket handler " << this);

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

    // Socket handler is now considered closed
    mIsOpen = false;

    // Shutdown the socket handler
    shutdown();
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int StreamSocketHandler::handle_timeout(const ACE_Time_Value& tv, const void *act)
{
    // Exclude other event handlers
    ACE_GUARD_RETURN(lock_t, guard, mHandlerLock, -1);

    // If our timer was stopped after a reactor thread selected us for dispatch but before the lock was taken
    // above then we need to exit without taking any action
    if (mTimerId == INVALID_TIMER_ID)
        return 0;
    
    // Flag timer as no longer running
    mTimerId = INVALID_TIMER_ID;

    // Dispatch expiring timers
    size_t numTimers = 0;
    const int retval = mTimerQueue->Dispatch(tv + this->reactor()->timer_queue()->timer_skew(), numTimers);

    if (retval == 0 && !mTimerQueue->Empty() && mIsOpen)
    {
        // The connection doesn't need to close and the timer queue is non-empty -- reschedule our timeout with the reactor
        const ACE_Time_Value now = ACE_OS::gettimeofday();
        mNextTimerExpires = mTimerQueue->EarliestTime();
        mTimerId = this->reactor()->schedule_timer(this, 0, (mNextTimerExpires > now) ? (mNextTimerExpires - now) : ACE_Time_Value::zero, ACE_Time_Value::zero);

        // Check for a potential race -- if we've been closed since the timer was scheduled, cancel it immediately to avoid leaving a dangling reference
        // to this event handler in the reactor
        if (!mIsOpen)
        {
            this->reactor()->cancel_timer(mTimerId);
            mTimerId = INVALID_TIMER_ID;
        }
    }
    else if (retval == -1)
    {
        // One of the expiring timers wants the connection to close -- this is complicated since a TP reactor worker thread may have already
        // selected this event handler for socket I/O dispatch... this thread could be executing anywhere leading up to the lock acquisition
        // in handle_input/handle_output above.

        // Our best bet is to shutdown the underlying socket.  If either of the handle_input or handle_output methods are already being dispatched, this will
        // eventually cause an I/O error and then method will return -1 resulting in an orderly close.  If the handler isn't already executing, shutting the
        // socket will cause a spurious read event leading to the same result in handle_input.
        ACE_OS::shutdown(this->peer().get_handle(), ACE_SHUTDOWN_BOTH);
    }
    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void StreamSocketHandler::Recv(std::string& buffer)
{
    messagePtr_t head = Recv();
    if (!head)
        return;

    // Enlarge string storage to hold complete copy of message block data
    buffer.resize(head->total_length());

    char *pos = const_cast<char *>(buffer.data());
    for (ACE_Message_Block *mb = head.get(); mb; mb = mb->next())
    {
        memcpy(pos, mb->rd_ptr(), mb->length());
        pos += mb->length();
    }
}

///////////////////////////////////////////////////////////////////////////////

StreamSocketHandler::messagePtr_t StreamSocketHandler::Recv(void)
{
    messagePtr_t head;
    ACE_Message_Block *tail = 0;

    // Dequeue message blocks from input queue and return them as a single chain
    while (!mInputQueue.empty())
    {
        messagePtr_t temp = mInputQueue.front();
        mInputQueue.pop_front();

        if (tail)
            tail->cont(temp->duplicate());
        else
            head = temp;

        tail = temp.get();
    }

    return head;
}

///////////////////////////////////////////////////////////////////////////////

bool StreamSocketHandler::Send(const std::string& buffer)
{
    // Need this because the UpdateTxCountHandler can delete us
    ACE_Event_Handler_var safeHandler(this);
    this->add_reference();

    size_t len = buffer.size();
    ssize_t bytesSent = 0;

    if (!mIsClientBwCtrl)
    {
        // Try to send immediately as long as output buffer is empty -- this is the ideal case
        if (mOutputQueue.empty() && ((bytesSent = SendData(buffer.data(), len)) == static_cast<ssize_t>(len)))
        {
            reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
            return true;
        }

        if (bytesSent < 0)       
            return false;
       
        len -= bytesSent;
    }

    // Enqueue a new message block in the output queue containing a copy of the unsent data
    messagePtr_t mb = L4L7_UTILS_NS::MessageAlloc(new ACE_Message_Block(len));
    memcpy(mb->base(), buffer.data() + bytesSent, len);
    mb->wr_ptr(len);
    mOutputQueue.push_back(mb);

    // Tell the reactor to pay attention to write event notifications
    reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool StreamSocketHandler::Send(messagePtr_t mb)
{
    // Need this because the UpdateTxCountHandler can delete us
    ACE_Event_Handler_var safeHandler(this);
    this->add_reference();

    // Don't use in BW control mode
    // Try to send immediately as long as output buffer is empty -- this is the ideal case
    if (!mIsClientBwCtrl && mOutputQueue.empty())
    {
        //
        // Count the number of blocks/bytes in this chain, including continuation blocks.
        // We may be limited by the maximum number of iovec entries per system call or
        // the output block size limit.
        //
        size_t blockCount = 0;
        size_t byteCount = 0;
        ACE_Message_Block *pos = mb.get();

        while (pos && blockCount < MAX_IOVEC_COUNT && byteCount < mOutputBlockSize)
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
        ssize_t bytesSent = SendData(iov, blockCount);
        if (bytesSent < 0)
            return false;
   
        // The iovec's were partially sent?
        while (bytesSent > 0)
        {
            if (mb->length() <= static_cast<size_t>(bytesSent))
            {
                // Block was completely sent, strip it from the chain and free it
                bytesSent -= mb->length();
                messagePtr_t temp = L4L7_UTILS_NS::MessageAlloc(mb->cont());
                mb->cont(0);
                mb = temp;
            }
            else
            {
                // Block was partially sent, update its read pointer and break loop to enqueue the remainder
                mb->rd_ptr(static_cast<size_t>(bytesSent));
                break;
            }
        }
    }

    if (mb)
    {
        // Enqueue message block in the output queue
        mOutputQueue.push_back(mb);
    }

    // Tell the reactor to pay attention to write event notifications
    reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

ssize_t StreamSocketHandler::SendData(const void *buffer, size_t len)
{
    const ssize_t bytesSent = this->peer().send(buffer, len);

    // send() returns -1 on error
    if (bytesSent < 0)
    {
        if (errno == EAGAIN || errno == EINTR)
            return 0;
        else
        {
            TC_LOG_DEBUG(0, "Stream socket handler " << this << " encountered error while sending: " << strerror(errno));
            return -1;
        }
    }

    UpdateTxCountHandler(bytesSent);
    return bytesSent;
}

///////////////////////////////////////////////////////////////////////////////

ssize_t StreamSocketHandler::SendData(const iovec *iov, size_t len)
{
    const ssize_t bytesSent = this->peer().sendv(iov, len);

    // sendv() returns -1 on error
    if (bytesSent < 0)
    {
        if (errno == EAGAIN || errno == EINTR)
            return 0;
        else
        {
            TC_LOG_DEBUG(0, "Stream socket handler " << this << " encountered error while sending: " << strerror(errno));
            return -1;
        }
    }

    UpdateTxCountHandler(bytesSent);
    return bytesSent;
}

///////////////////////////////////////////////////////////////////////////////

ssize_t StreamSocketHandler::ReceiveData(void *buffer, size_t len)
{
    const ssize_t bytesRecv = this->peer().recv(buffer, len);

    // recv() returns -1 on error, 0 on EOF (socket is closed)
    if (bytesRecv == 0)
    {
        TC_LOG_DEBUG(0, "Stream socket handler " << this << " closed");
        return -1;
    }
    else if (bytesRecv < 0)
    {
        if (errno == EAGAIN || errno == EINTR)
            return 0;
        else
        {
            TC_LOG_DEBUG(0, "Stream socket handler " << this << " encountered error while receiving: " << strerror(errno));
            return -1;
        }
    }

    return bytesRecv;
}

///////////////////////////////////////////////////////////////////////////////

void StreamSocketHandler::ScheduleTimer(IL_DAEMON_LIB_NS::TimerQueue::type_t type, IL_DAEMON_LIB_NS::TimerQueue::act_t *act, IL_DAEMON_LIB_NS::TimerQueue::delegate_t delegate, const ACE_Time_Value& delay)
{
    if (!mTimerQueue)
        return;
    
    mTimerQueue->Schedule(type, act, delegate, delay);
    const ACE_Time_Value& nextExpiration = mTimerQueue->EarliestTime();

    if (mTimerId != INVALID_TIMER_ID)
    {
        // If the new timer needs to expire before the current next expiring timer, we need to reschedule our timeout with the ACE reactor
        if (nextExpiration < mNextTimerExpires)
        {
            mNextTimerExpires = nextExpiration;

            const ACE_Time_Value delay = mNextTimerExpires - ACE_OS::gettimeofday();
            long oldTimerId = mTimerId;
            mTimerId = this->reactor()->schedule_timer(this, 0, delay, ACE_Time_Value::zero);
            this->reactor()->cancel_timer(oldTimerId);
        }
    }
    else
    {
        // This is the first timer in the queue, schedule a timeout with the ACE reactor
        mNextTimerExpires = nextExpiration;
        mTimerId = this->reactor()->schedule_timer(this, 0, delay, ACE_Time_Value::zero);
    }
}

///////////////////////////////////////////////////////////////////////////////

void StreamSocketHandler::PurgeTimers(void)
{
    // Exclude other event handlers and timers
    ACE_GUARD(lock_t, guard, mHandlerLock);

    if (mTimerId != INVALID_TIMER_ID)
    {
        this->reactor()->cancel_timer(mTimerId);
        mTimerId = INVALID_TIMER_ID;
    }

    mTimerQueue.reset();
}

///////////////////////////////////////////////////////////////////////////////

void StreamSocketHandler::UpdateTxCountHandler(uint64_t sent)
{
    if (!mUpdateTxCountDelegate.empty())
        mUpdateTxCountDelegate(sent);
}

///////////////////////////////////////////////////////////////////////////////

size_t StreamSocketHandler::GetDynamicLoadBytesHandler(bool isInput)
{
    size_t bytesNeeded = 0;

    if (!mGetDynamicLoadBytesDelegate.empty())
        bytesNeeded = mGetDynamicLoadBytesDelegate(isInput);
    
    return bytesNeeded;
}

///////////////////////////////////////////////////////////////////////////////

size_t StreamSocketHandler::ProduceDynamicLoadBytesHandler(bool isInput, const ACE_Time_Value& currTime)
{
    size_t bytesNeeded = 0;

    if (!mProduceDynamicLoadBytesDelegate.empty())
        bytesNeeded = mProduceDynamicLoadBytesDelegate(isInput, currTime);

    return bytesNeeded;
}

///////////////////////////////////////////////////////////////////////////////

size_t StreamSocketHandler::ConsumeDynamicLoadBytesHandler(bool isInput, size_t consumed)
{
    size_t bytesNeeded = 0;

    if (!mConsumeDynamicLoadBytesDelegate.empty())
        bytesNeeded = mConsumeDynamicLoadBytesDelegate(isInput, consumed);

    return bytesNeeded;
}

///////////////////////////////////////////////////////////////////////////////

