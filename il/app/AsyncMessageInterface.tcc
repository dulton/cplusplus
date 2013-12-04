/// @file
/// @brief Asychronous Message Interface templated implementation
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_ASYNC_MESSAGE_INTERFACE_TCC_
#define _L4L7_ASYNC_MESSAGE_INTERFACE_TCC_

#include <queue>

#include <ace/Condition_Thread_Mutex.h>
#include <ace/Event_Handler.h>
#include <ace/Reactor.h>
#include <ace/Reactor_Notification_Strategy.h>
#include <ace/Thread_Mutex.h>
#include <boost/bind.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/utility.hpp>
#include <FastDelegate.h>
#include <phxexception/PHXException.h>
#include <Tr1Adapter.h>

#include <app/AppCommon.h>

// Forward declarations (global)
class ACE_Reactor;

DECL_L4L7_APP_NS

///////////////////////////////////////////////////////////////////////////////

class Tautology
{
public:
    bool operator()() { return true; }
};

///////////////////////////////////////////////////////////////////////////////

template<typename MessageType, typename LoopPredicate = Tautology>
class AsyncMessageInterface : public ACE_Event_Handler, boost::noncopyable
{
  public:
    typedef std::tr1::shared_ptr<MessageType> messagePtr_t;
    typedef fastdelegate::FastDelegate1<const MessageType&> messageDelegate_t;
    
    explicit AsyncMessageInterface(ACE_Reactor *reactor);

    /// @brief Allocate a new message instance
    messagePtr_t Allocate(void);

    /// @brief Send a message
    void Send(const messagePtr_t& msg);

    /// @brief Wait for signal
    /// @retval 0 if successful, -1 otherwise
    int Wait(void);

    /// @brief Signal waiter
    void Signal(void);

    /// @brief Check queue empty status
    /// @retval True if queue is empty, false otherwise
    bool IsEmpty(void)
    {
        ACE_GUARD_RETURN(lock_t, guard, mQueueLock, false);
        return mQueue.empty();
    }
    
    /// @brief Purge pending reactor notifications
    void PurgePendingNotifications(void);
    
    /// @brief Flush the message interface
    void Flush(void) { this->handle_input(ACE_INVALID_HANDLE); }

    /// @brief Set the message delegate
    void SetMessageDelegate(messageDelegate_t delegate) { mMessageDelegate = delegate; }
    
  private:
    /// @brief Deallocate a message instance
    void Deallocate(MessageType *msg);

    /// @brief ACE event handler method
    int handle_input(ACE_HANDLE fd);

    /// Convenience typedefs
    typedef boost::object_pool<MessageType> pool_t;
    typedef std::queue<messagePtr_t> messageQueue_t;
    typedef ACE_Thread_Mutex lock_t;
    typedef ACE_Condition_Thread_Mutex cond_t;
    
    lock_t mPoolLock;                                   ///< lock for pool access
    pool_t mPool;                                       ///< simple memory pool
    
    lock_t mHandlerLock;                                ///< lock for handle_input atomicity

    lock_t mQueueLock;                                  ///< lock for message queue
    ACE_Reactor_Notification_Strategy mNotifier;        ///< reactor notification strategy
    messageQueue_t mQueue;                              ///< message queue
    messageDelegate_t mMessageDelegate;                 ///< message notification delegate

    lock_t mSignalLock;                                 ///< lock for signal condition
    bool mSignal;                                       ///< signal variable
    cond_t mSignalCond;                                 ///< condition variable for signalling
};

///////////////////////////////////////////////////////////////////////////////

template<typename MessageType, typename LoopPredicate>
AsyncMessageInterface<MessageType, LoopPredicate>::AsyncMessageInterface(ACE_Reactor *reactor)
    : mNotifier(reactor, this, ACE_Event_Handler::READ_MASK),
      mSignal(false),
      mSignalCond(mSignalLock)
{
}

///////////////////////////////////////////////////////////////////////////////

template<typename MessageType, typename LoopPredicate>
typename AsyncMessageInterface<MessageType, LoopPredicate>::messagePtr_t AsyncMessageInterface<MessageType, LoopPredicate>::Allocate(void)
{
    MessageType *msg;
    {
        ACE_GUARD_RETURN(lock_t, guard, mPoolLock, messagePtr_t());
        msg = mPool.construct();
    }
    
    if (!msg)
        throw std::bad_alloc();

    return messagePtr_t(msg, boost::bind(&AsyncMessageInterface<MessageType, LoopPredicate>::Deallocate, this, _1));
}

///////////////////////////////////////////////////////////////////////////////

template<typename MessageType, typename LoopPredicate>
void AsyncMessageInterface<MessageType, LoopPredicate>::Deallocate(MessageType *msg)
{
    ACE_GUARD(lock_t, guard, mPoolLock);
    mPool.destroy(msg);
}

///////////////////////////////////////////////////////////////////////////////

template<typename MessageType, typename LoopPredicate>
void AsyncMessageInterface<MessageType, LoopPredicate>::Send(const messagePtr_t& msg)
{
    ACE_GUARD(lock_t, guard, mQueueLock);
    const bool wasEmpty = mQueue.empty();
    mQueue.push(msg);
    if (wasEmpty)
        mNotifier.notify();
}

///////////////////////////////////////////////////////////////////////////////

template<typename MessageType, typename LoopPredicate>
int AsyncMessageInterface<MessageType, LoopPredicate>::Wait(void)
{
    ACE_GUARD_RETURN(lock_t, guard, mSignalLock, -1);
    while (!mSignal)
    {
        if (mSignalCond.wait(0) == -1)
            return -1;
    }

    mSignal = false;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

template<typename MessageType, typename LoopPredicate>
void AsyncMessageInterface<MessageType, LoopPredicate>::Signal(void)
{
    ACE_GUARD(lock_t, guard, mSignalLock);
    mSignal = true;
    mSignalCond.signal();
}

///////////////////////////////////////////////////////////////////////////////

template<typename MessageType, typename LoopPredicate>
void AsyncMessageInterface<MessageType, LoopPredicate>::PurgePendingNotifications(void)
{
    ACE_GUARD(lock_t, guard, mHandlerLock);
    mNotifier.reactor()->purge_pending_notifications(this);
}

///////////////////////////////////////////////////////////////////////////////

template<typename MessageType, typename LoopPredicate>
int AsyncMessageInterface<MessageType, LoopPredicate>::handle_input(ACE_HANDLE)
{
    messagePtr_t msg;
    LoopPredicate loop;
    
    ACE_GUARD_RETURN(lock_t, guard, mHandlerLock, -1);
    while (loop())
    {
        {
            ACE_GUARD_RETURN(lock_t, guard2, mQueueLock, -1);
            if (!mQueue.empty())
            {
                msg = mQueue.front();
                mQueue.pop();
            }
            else
                break;
        }
        
        if (mMessageDelegate)
            mMessageDelegate(*msg);
    }

    {
        ACE_GUARD_RETURN(lock_t, guard2, mQueueLock, -1);
        if (!mQueue.empty())
        {
            // we're leaving early, prime the notifier
            mNotifier.notify();
        }
    }



    return 0;
}

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_APP_NS

// Local Variables:
// mode:c++
// End:

#endif
