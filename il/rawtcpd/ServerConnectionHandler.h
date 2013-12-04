/// @file
/// @brief Server Connection Handler header file
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _SERVER_CONNECTION_HANDLER_H_
#define _SERVER_CONNECTION_HANDLER_H_

#include <queue>

#include <ace/Time_Value.h>
#include <app/StreamSocketHandler.h>
#include <base/BaseCommon.h>
#include <boost/random/linear_congruential.hpp>
#include <Tr1Adapter.h>
#include <utils/BodySize.h>
#include <utils/ResponseLatency.h>

#include "RawTcpCommon.h"
#include "RawTcpProtocol.h"


#ifndef IPV6_TCLASS
# define IPV6_TCLASS 67         // ref include/linux/in6.h in kernel 2.6 sources
#endif

// Forward declarations (global)
class ACE_Message_Block;

DECL_RAWTCP_NS

// Forward declarations
class RawTcpServerRawStats;

///////////////////////////////////////////////////////////////////////////////

class ServerConnectionHandler : public L4L7_APP_NS::StreamSocketHandler
{
  public:
//    typedef RawTcp_1_port_server::RawTcpVersionOptions version_t;
//    typedef RawTcp_1_port_server::RawTcpServerType type_t;
    typedef RawTcp_1_port_server::BodyContentOptions bodyType_t;
    typedef RawTcpServerRawStats stats_t;
    
    explicit ServerConnectionHandler(uint32_t serial = 0);
    ~ServerConnectionHandler();

//    /// @brief Set server version
//    void SetVersion(version_t version) { mVersion = version; }

//    /// @brief Set server type
//    void SetType(type_t type) { mType = type; }

    /// @brief Set maximum number of requests
    void SetMaxRequests(uint32_t maxRequests) { mMaxRequests = maxRequests; }

    /// @brief Set body type
    void SetBodyType(bodyType_t bodyType) { mBodyType = bodyType; }
    
    /// @brief Set fixed body size
    void SetBodySize(uint32_t fixed) { mBodySize.Set(fixed); }

    /// @brief Set random body size
    void SetBodySize(uint32_t mean, uint32_t stddev) { mBodySize.Set(mean, stddev); }

    /// @brief Set fixed response latency
    void SetResponseLatency(uint32_t fixed) { mRespLatency.Set(fixed); }

    /// @brief Set random response latency
    void SetResponseLatency(uint32_t mean, uint32_t stddev) { mRespLatency.Set(mean, stddev); }

    /// @brief Set stats instance
    void SetStatsInstance(stats_t& stats);

    /// @brief Update Goodput Tx
    void UpdateGoodputTxSent(uint64_t sent);
 
    /// @brief Get Address Type
    int GetAddrType(void)
    {
         ACE_INET_Addr addr;
         if(this->peer().get_local_addr(addr) == -1)
         {
              return -1;         
         }
         return addr.get_type();
    }

    /// @brief Set the IPv4 TOS socket option
    int SetIpv4Tos(int tos)
    {
        const int thetos = tos;
        return this->peer().set_option(IPPROTO_IP, IP_TOS, (char *)&thetos, sizeof(tos));
    }
 
    /// @brief Set the IPv6 traffic class socket option
    int SetIpv6TrafficClass(int trafficClass)
    {
        const int tclass = trafficClass;
        return this->peer().set_option(IPPROTO_IPV6, IPV6_TCLASS, (char *)&tclass, sizeof(tclass));
    }
    
    private:
    /// Private timer types
    enum
    {
        INACTIVITY_TIMER,
        RESP_QUEUE_TIMER
    };
    
    /// Private state class forward declarations
    struct State;
    struct IdleState;
    struct HeaderState;
    struct FlushState;

    /// Private response record class
    struct ResponseRecord;
    
    /// Convenience methods
    uint32_t GetBodySize(void) { return mBodySize.Get(); }
    
    ACE_Time_Value GetResponseLatency(void) { return mRespLatency.Get(); }

    /// Socket open handler method (from StreamSocketHandler interface)
    int HandleOpenHook(void);

    /// Socket input handler method (from StreamSocketHandler interface)
    int HandleInputHook(void);

    /// Timeout handler methods
    int HandleResponseQueueTimeout(const ACE_Time_Value& tv, const void *act);
    int HandleInactivityTimeout(const ACE_Time_Value& tv, const void *act);
    
    /// Response queue methods
    void QueueResponse(const ACE_Time_Value& absTime, bool closeFlag);
    void QueueResponse(const ACE_Time_Value& absTime, bodyType_t bodyType, uint32_t bodySize, uint32_t responseNum, bool closeFlag);
    ssize_t ServiceResponseQueue(const ACE_Time_Value& absTime);
    
    /// Central state change method
    void ChangeState(const State *toState) { mState = toState; }
    
    /// Class data
    static const IdleState IDLE_STATE;
    static const HeaderState HEADER_STATE;
    static const FlushState FLUSH_STATE;

    static const ACE_Time_Value INACTIVITY_TIMEOUT;

    /// Convenience typedefs
    typedef boost::minstd_rand randomEngine_t;

    randomEngine_t mRandomEngine;                       ///< randomness engine
//    version_t mVersion;                                 ///< server version
//    type_t mType;                                       ///< server type
    uint32_t mMaxRequests;                              ///< max number of requests to allow
    bodyType_t mBodyType;                               ///< body type
    L4L7_UTILS_NS::BodySize mBodySize;                  ///< body size

    L4L7_UTILS_NS::ResponseLatency mRespLatency;        ///< response latency

    stats_t *mStats;                                    ///< server stats
    
    messagePtr_t mInBuffer;                             ///< input buffer
    const State *mState;                                ///< current state

    uint32_t mRequestCount;                             ///< number of requests processed thus far
    uint32_t mCurRequestNum;                            ///< current request number
    bool mCloseFlag;                                    ///< close connection after this request?

    std::queue<ResponseRecord> mRespQueue;              ///< queue of pending responses

    IL_DAEMON_LIB_NS::TimerQueue::delegate_t mRespQueueTimeoutDelegate;
    IL_DAEMON_LIB_NS::TimerQueue::delegate_t mInactivityTimeoutDelegate;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_RAWTCP_NS

#endif
