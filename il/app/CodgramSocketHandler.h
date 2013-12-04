/// @file
/// @brief Connection-oriented datagram Socket (ACE) Handler header file
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_CODGRAM_SOCKET_HANDLER_H_
#define _L4L7_CODGRAM_SOCKET_HANDLER_H_

#include <deque>
#include <string>

#include <ace/Message_Queue.h>
#include <ace/OS_NS_sys_uio.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Svc_Handler.h>
#include <ace/SOCK_CODgram.h>
#include <boost/scoped_ptr.hpp>
#include <FastDelegate.h>
#include <Tr1Adapter.h>

#include <app/AppCommon.h>
#include <ildaemon/TimerQueue.h>

// Forward declarations (global)
class ACE_Message_Block;
class TestCodgramSocketHandler;

DECL_L4L7_APP_NS

///////////////////////////////////////////////////////////////////////////////

class CodgramSocketHandler : public ACE_Svc_Handler<ACE_SOCK_CODgram, ACE_NULL_SYNCH>
{
  public:
    typedef std::tr1::shared_ptr<ACE_Message_Block> messagePtr_t;
    typedef fastdelegate::FastDelegate1<CodgramSocketHandler &> closeNotificationDelegate_t;

    // Dynamic Load Delegates
    typedef boost::function1<void, uint64_t> updateTxCountDelegate_t;

    explicit CodgramSocketHandler(uint32_t serial);
    virtual ~CodgramSocketHandler();

    /// @brief Return socket handler serial number
    uint32_t Serial(void) const { return mSerial; }
    
    /// @brief Set socket handler pending status
    void IsPending(bool isPending) { mIsPending = isPending; }
    
    /// @brief Return socket handler pending status
    /// @retval True when socket handler is pending, false otherwise
    bool IsPending(void) const { return mIsPending; }
    
    /// @brief Return socket handler open status
    /// @retval True when socket handler is fully open, false otherwise
    bool IsOpen(void) const { return mIsOpen; }

    /// @brief Return socket handler connected status
    /// @retval Always same as IsOpen -- this is connectionless
    bool IsConnected(void) const { return false; }

    /// @brief Get the current SO_RCVBUF socket option value
    bool GetSoRcvBuf(size_t &soRcvBuf) const 
    {
        int optLen = sizeof(soRcvBuf);
        return (this->peer().get_option(SOL_SOCKET, SO_RCVBUF, &soRcvBuf, &optLen) == 0);
    }
    
    /// @brief Set the SO_RCVBUF socket option value
    bool SetSoRcvBuf(size_t soRcvBuf) { return (this->peer().set_option(SOL_SOCKET, SO_RCVBUF, &soRcvBuf, sizeof(soRcvBuf)) == 0); }
    
    /// @brief Set the input block size (maximum buffer size for single receive operation)
    /// @note It doesn't make sense to set this any higher than SO_RCVBUF
    void SetInputBlockSize(size_t inputBlockSize) { mInputBlockSize = inputBlockSize; }

    /// @brief Return input queue status
    /// @retval True when input queue has data, false otherwise
    bool HasInputData(void) const { return !mInputQueue.empty(); }

    /// @brief Get the current SO_SNDBUF socket option value
    bool GetSoSndBuf(size_t &soSndBuf) const 
    {
        int optLen = sizeof(soSndBuf);
        return (this->peer().get_option(SOL_SOCKET, SO_SNDBUF, &soSndBuf, &optLen) == 0);
    }
    
    /// @brief Set the SO_SNDBUF socket option value
    bool SetSoSndBuf(size_t soSndBuf) { return (this->peer().set_option(SOL_SOCKET, SO_SNDBUF, &soSndBuf, sizeof(soSndBuf)) == 0); }
    
    /// @brief Set the output block size (maximum buffer size for single transmit operation)
    /// @note It doesn't make sense to set this any higher than SO_SNDBUF
    void SetOutputBlockSize(size_t outputBlockSize) { mOutputBlockSize = outputBlockSize; }

    /// @brief Return output queue status
    /// @retval True when ouput queue has data, false otherwise
    bool HasOutputData(void) const { return !mOutputQueue.empty(); }
    
    /// @brief ACE event handler methods
    /// @note Because of the way the ACE framework is written, most of these need to be public however you must never call these directly
    int open(void *factory);
    int handle_input(ACE_HANDLE fd);
    int handle_output(ACE_HANDLE fd);
    int handle_close(ACE_HANDLE fd, ACE_Reactor_Mask mask);
    int handle_timeout(const ACE_Time_Value& tv, const void *act);

    int handle_output_send(size_t bytesNeeded, bool bandwidthSend = false);

    /// @brief Atomically clear the close event delegate
    void ClearCloseNotificationDelegate(void)
    {
        ACE_GUARD(lock_t, guard, mStateLock);
        mCloseNotificationDelegate.clear();
    }
    
    /// @brief Atomically get/clear the close event delegate
    /// @note Atomic guarantee is necessary here since calling the delegate copy ctor is not an atomic operation
    void PopCloseNotificationDelegate(closeNotificationDelegate_t& out)
    {
        ACE_GUARD(lock_t, guard, mStateLock);
        out = mCloseNotificationDelegate;
        mCloseNotificationDelegate.clear();
    }
    
    /// @brief Atomically set the close event delegate
    /// @note Invoked when the socket is closed for any reason
    void SetCloseNotificationDelegate(closeNotificationDelegate_t delegate)
    {
        ACE_GUARD(lock_t, guard, mStateLock);
        mCloseNotificationDelegate = delegate;
    }

    /// @brief Purge pending timer expirations and prevent any new timers from being scheduled
    void PurgeTimers(void);

    /// @brief Update Client Goodput Tx methods
    void RegisterUpdateTxCountDelegate(updateTxCountDelegate_t delegate) { mUpdateTxCountDelegate = delegate; }
    void UnregisterUpdateTxCountDelegate(void) { mUpdateTxCountDelegate.clear(); }
    void UpdateTxCountHandler(uint64_t sent);

  protected:
    /// Convenience typedefs
    typedef ACE_Recursive_Thread_Mutex lock_t;    

    /// @brief Receive into a string buffer
    void Recv(std::string& buffer);

    /// @brief Receive an #ACE_Message_Block that may contain a message in multiple continuation blocks
    messagePtr_t Recv(void);
    
    /// @brief Send a string buffer
    /// @retval True if buffer was sent successfully
    bool Send(const std::string& buffer);

    /// @brief Send an #ACE_Message_Block
    /// @retval True if mesage block was sent successfully
    bool Send(messagePtr_t mb);

    ///@brief Schedule a timer
    void ScheduleTimer(IL_DAEMON_LIB_NS::TimerQueue::type_t type, IL_DAEMON_LIB_NS::TimerQueue::act_t *act, IL_DAEMON_LIB_NS::TimerQueue::delegate_t delegate, const ACE_Time_Value& delay);

    ///@brief Cancel a timer
    void CancelTimer(IL_DAEMON_LIB_NS::TimerQueue::type_t type) { if (mTimerQueue) mTimerQueue->Cancel(type); }

    /// @brief Check timer running status
    bool IsTimerRunning(IL_DAEMON_LIB_NS::TimerQueue::type_t type) const { return mTimerQueue ? mTimerQueue->IsScheduled(type) : false; }

    /// @note Concrete handler classes can override
    virtual int HandleOpenHook(void) { return 0; }

    /// @note Concrete handler classes can override
    virtual int HandleInputHook(void) { return 0; }

    /// @note Concrete handler classes can override
    virtual int HandleOutputHook(void) { return 0; }

    /// @brief Handler lock accessor
    lock_t& HandlerLock(void) { return mHandlerLock; }

    /// @brief State lock accessor
    lock_t& StateLock(void) { return mStateLock; }

  private:
    /// I/O methods
    ssize_t SendData(const void *buffer, size_t len);
    ssize_t SendData(const iovec *iov, size_t len);
    ssize_t ReceiveData(void *buffer, size_t len);

    /// Convenience typedefs
    typedef std::deque<messagePtr_t> messageQueue_t;

    /// Class data
    static const size_t MAX_IOVEC_COUNT = 8;                    ///< kernel's UIO_FASTIOV; keeps kernel from having to kmalloc struct iovec's
    static const size_t DEFAULT_INPUT_BLOCK_SIZE = 2048;
    static const long INVALID_TIMER_ID = -1;

  protected:
    const uint32_t mSerial;                                     ///< handler serial number, uniquely identifies this object instance
  
  private:
    lock_t mHandlerLock;                                        ///< protects against concurrent handle_* method calls
    lock_t mStateLock;                                          ///< protects against concurrent state accesses/mutations
    bool mIsPending;                                            ///< pending status
    bool mIsOpen;                                               ///< open status
    

    updateTxCountDelegate_t mUpdateTxCountDelegate;             ///< delegate to update client goodput tx

    size_t mInputBlockSize;                                     ///< maximum buffer size for single receive operation
    messageQueue_t mInputQueue;                                 ///< input message queue
    size_t mOutputBlockSize;                                    ///< maximum buffer size for single transmit operation
    messageQueue_t mOutputQueue;                                ///< output message queue

    boost::scoped_ptr<IL_DAEMON_LIB_NS::TimerQueue> mTimerQueue;///< timer queue
    long mTimerId;                                              ///< "consolidated" timer id
    ACE_Time_Value mNextTimerExpires;                           ///< absolute time of next timer expiration

    closeNotificationDelegate_t mCloseNotificationDelegate;     ///< close notification delegate

    friend class TestCodgramSocketHandler;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_APP_NS

#endif
