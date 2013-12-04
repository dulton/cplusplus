/// @file
/// @brief Datagram Socket (ACE) Handler header file
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_DATAGRAM_SOCKET_HANDLER_H_
#define _L4L7_DATAGRAM_SOCKET_HANDLER_H_

#include <deque>
#include <ace/Svc_Handler.h>
#include <ace/SOCK_Dgram.h>
#include <FastDelegate.h>
#include <Tr1Adapter.h>

#include <app/AppCommon.h>

#ifndef IPV6_TCLASS
# define IPV6_TCLASS 67         // ref include/linux/in6.h in kernel 2.6 sources
#endif

// Forward declarations (global)
class ACE_Message_Block;
class TestDatagramSocketHandler;

DECL_L4L7_APP_NS

///////////////////////////////////////////////////////////////////////////////

class DatagramSocketHandler : public ACE_Svc_Handler<ACE_SOCK_Dgram, ACE_NULL_SYNCH>
{
  public:
    typedef std::tr1::shared_ptr<ACE_Message_Block> messagePtr_t;
    typedef fastdelegate::FastDelegate1<DatagramSocketHandler &> closeNotificationDelegate_t;

    typedef boost::function2<void, uint64_t, const ACE_INET_Addr&> updateTxCountDelegate_t;

    explicit DatagramSocketHandler(size_t mtu);
    virtual ~DatagramSocketHandler();

    int open(const ACE_INET_Addr& local_addr, ACE_Reactor * reactor);

    /// @brief Send a string buffer
    /// @retval True if buffer was sent successfully
    bool Send(const std::string& buffer, const ACE_INET_Addr& remote_addr);

    /// @brief Send an #ACE_Message_Block
    /// @retval True if mesage block was sent successfully
    bool Send(messagePtr_t mb, const ACE_INET_Addr& remote_addr);

    /// @brief Get the current SO_RCVBUF socket option value
    bool GetSoRcvBuf(size_t &soRcvBuf) const 
    {
        int optLen = sizeof(soRcvBuf);
        return (this->peer().get_option(SOL_SOCKET, SO_RCVBUF, &soRcvBuf, &optLen) == 0);
    }
    
    /// @brief Set the SO_RCVBUF socket option value
    bool SetSoRcvBuf(size_t soRcvBuf) { return (this->peer().set_option(SOL_SOCKET, SO_RCVBUF, &soRcvBuf, sizeof(soRcvBuf)) == 0); }
    
    /// @brief Get the current SO_SNDBUF socket option value
    bool GetSoSndBuf(size_t &soSndBuf) const 
    {
        int optLen = sizeof(soSndBuf);
        return (this->peer().get_option(SOL_SOCKET, SO_SNDBUF, &soSndBuf, &optLen) == 0);
    }
    
    /// @brief Set the SO_SNDBUF socket option value
    bool SetSoSndBuf(size_t soSndBuf) { return (this->peer().set_option(SOL_SOCKET, SO_SNDBUF, &soSndBuf, sizeof(soSndBuf)) == 0); }
    
    void SetIpv4Tos(uint8_t theTos)
    {
        const int tos = theTos;
        this->peer().set_option(IPPROTO_IP, IP_TOS, (char *) &tos, sizeof(tos));
    }

    void SetIpv6TrafficClass(uint8_t trafficClass)
    {
        const int tclass = trafficClass;
        this->peer().set_option(IPPROTO_IPV6, IPV6_TCLASS, (char *) &tclass, sizeof(tclass));
    }

    void BindToIfName(const std::string& ifName)
    {
        // Bind to device
        this->peer().set_option(SOL_SOCKET, SO_BINDTODEVICE, const_cast<char *>(ifName.c_str()), ifName.size() + 1);
    }

    /// @brief Return output queue status
    /// @retval True when ouput queue has data, false otherwise
    bool HasOutputData(void) const { return !mOutputQueue.empty(); }
    
    /// @brief ACE event handler methods
    /// @note Because of the way the ACE framework is written, most of these need to be public however you must never call these directly
    int open(void *factory);
    int handle_input(ACE_HANDLE fd);
    int handle_output(ACE_HANDLE fd);
    int handle_close(ACE_HANDLE fd, ACE_Reactor_Mask mask);

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

    /// @brief Update Client Goodput Tx methods
    void RegisterUpdateTxCountDelegate(updateTxCountDelegate_t delegate) { mUpdateTxCountDelegate = delegate; }
    void UnregisterUpdateTxCountDelegate(void) { mUpdateTxCountDelegate.clear(); }

  protected:
    /// Convenience typedefs
    typedef ACE_Recursive_Thread_Mutex lock_t;    

    /// @note Concrete handler classes can override
    virtual int HandleInputHook(messagePtr_t& msg, const ACE_INET_Addr& addr) { return 0; }

    /// @note Concrete handler classes can override
    virtual int HandleOutputHook() { return 0; }

    /// @brief Handler lock accessor
    lock_t& HandlerLock(void) { return mHandlerLock; }

    /// @brief State lock accessor
    lock_t& StateLock(void) { return mStateLock; }

    /// @brief Update Client Goodput Tx methods
    void UpdateTxCountHandler(uint64_t sent, const ACE_INET_Addr& remote_addr);

    size_t GetOutputQueueSize() const { return mOutputQueue.size(); }

  private:
    /// I/O methods
    ssize_t SendData(const void *buffer, size_t len, const ACE_INET_Addr& remote_addr);
    ssize_t SendData(const iovec *iov, size_t len, const ACE_INET_Addr& remote_addr);
    ssize_t ReceiveData(void *buffer, size_t len, ACE_INET_Addr& remote_addr);

    /// Convenience typedefs
    typedef std::deque<messagePtr_t> messageQueue_t;
    typedef std::deque<ACE_INET_Addr> addrQueue_t;

    /// Class data
    static const size_t MAX_IOVEC_COUNT = 8;                    ///< kernel's UIO_FASTIOV; keeps kernel from having to kmalloc struct iovec's

  protected:
    const size_t mMtu;
  
  private:
    lock_t mHandlerLock;                                        ///< protects against concurrent handle_* method calls
    lock_t mStateLock;                                          ///< protects against concurrent state accesses/mutations
    updateTxCountDelegate_t mUpdateTxCountDelegate;             ///< delegate to update client goodput tx
    messageQueue_t mOutputQueue;                                ///< output message queue
    addrQueue_t mAddrQueue;                                     ///< output address queue
    closeNotificationDelegate_t mCloseNotificationDelegate;     ///< close notification delegate

    friend class TestDatagramSocketHandler;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_APP_NS

#endif
