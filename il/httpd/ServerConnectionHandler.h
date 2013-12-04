/// @file
/// @brief Server Connection Handler header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
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

#include "HttpCommon.h"
#include "HttpProtocol.h"

#ifndef IPV6_TCLASS
    #define IPV6_TCLASS 67         // ref include/linux/in6.h in kernel 2.6 sources
#endif

// Forward declarations (global)
class ACE_Message_Block;

DECL_HTTP_NS

// Forward declarations
class HttpServerRawStats;
///////////////////////////////////////////////////////////////////////////////

class ServerConnectionHandler : public L4L7_APP_NS::StreamSocketHandler
{
  public:
    /// Public state class forward declarations
    struct State;
    struct IdleState;
    struct HeaderState;
    struct FlushState;

    typedef Http_1_port_server::HttpVersionOptions version_t;
    typedef Http_1_port_server::HttpServerType type_t;
    typedef Http_1_port_server::BodyContentOptions bodyType_t;
    typedef HttpServerRawStats stats_t;
    
    explicit ServerConnectionHandler(uint32_t serial = 0);
    virtual ~ServerConnectionHandler();

    /// @brief Set/Get server version
    void SetVersion(version_t version) { mVersion = version; }
    version_t GetVersion() { return mVersion ; }

    /// @brief Set/Get server type
    void SetType(type_t type) { mType = type; }
    type_t GetType() { return mType; }

    /// @brief Set/Get maximum number of requests
    void SetMaxRequests(uint32_t maxRequests) { mMaxRequests = maxRequests; }
    uint32_t GetMaxRequests() { return mMaxRequests; }

    /// @brief Set/Get body type
    void SetBodyType(bodyType_t bodyType) { mBodyType = bodyType; }
    bodyType_t GetBodyType() { return mBodyType; }

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
    stats_t* GetStatsInstance() { return mStats; }

    /// @brief Get timeout delegate reference
    IL_DAEMON_LIB_NS::TimerQueue::delegate_t& GetTimeoutDelegate() { return mInactivityTimeoutDelegate; }

    /// @brief Get Response Queue delegate reference
    IL_DAEMON_LIB_NS::TimerQueue::delegate_t& GetResponseQueueDelegate() { return mRespQueueTimeoutDelegate; }

    /// @brief Get State Instance
    const State* GetStateInstance() const { return mState; }

    /// @brief Update Goodput Tx
    void UpdateGoodputTxSent(uint64_t sent);

    struct ResponseRecord
    {
        ResponseRecord(const ACE_Time_Value& theAbsTime, HttpProtocol::StatusCode theStatusCode, bodyType_t theBodyType, uint32_t theBodySize, bool theCloseFlag, std::string theUri)
            : absTime(theAbsTime),
              statusCode(theStatusCode),
              bodyType(theBodyType),
              bodySize(theBodySize),
              closeFlag(theCloseFlag),
              uri(theUri)
        {
        }

        ACE_Time_Value absTime;                     ///< absolute time that response should be sent
        HttpProtocol::StatusCode statusCode;        ///< response status code
        bodyType_t bodyType;                        ///< response body type
        uint32_t bodySize;                          ///< response size in bytes
        bool closeFlag;                             ///< connection closing?
        std::string uri;                             ///< Uri string>
    };

    /// @brief Get State Instance
    std::queue<ResponseRecord> *GetResponseQueue() { return &mRespQueue; }

    /// Private timer types
    /// timer types
    enum
    {
        INACTIVITY_TIMER,
        RESP_QUEUE_TIMER
    };
   
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
    /// Convenience methods
    uint32_t GetBodySize(void) { return mBodySize.Get(); }
        
    ACE_Time_Value GetResponseLatency(void) { return mRespLatency.Get(); }

    /// Socket open handler method (from StreamSocketHandler interface)
    int HandleOpenHook(void);

    /// Socket input handler method (from StreamSocketHandler interface)
    virtual int HandleInputHook(void);

    /// Timeout handler methods
    int HandleResponseQueueTimeout(const ACE_Time_Value& tv, const void *act);
    int HandleInactivityTimeout(const ACE_Time_Value& tv, const void *act);
    
    /// Response queue methods
    virtual void QueueResponse(const ACE_Time_Value& absTime, HttpProtocol::StatusCode errorCode, bool closeFlag);
    virtual void QueueResponse(const ACE_Time_Value& absTime, bodyType_t bodyType, uint32_t bodySize, bool closeFlag, std::string uri);
    virtual ssize_t ServiceResponseQueue(const ACE_Time_Value& absTime);
    
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
    version_t mVersion;                                 ///< server version
    type_t mType;                                       ///< server type
    uint32_t mMaxRequests;                              ///< max number of requests to allow
    bodyType_t mBodyType;                               ///< body type
    L4L7_UTILS_NS::BodySize mBodySize;                  ///< body size

    L4L7_UTILS_NS::ResponseLatency mRespLatency;        ///< response latency

    stats_t *mStats;                                    ///< server stats
    
    messagePtr_t mInBuffer;                             ///< input buffer
    const State *mState;                                ///< current state

    uint32_t mRequestCount;                             ///< number of requests processed thus far
    bool mCloseFlag;                                    ///< close connection after this request?

    std::queue<ResponseRecord> mRespQueue;              ///< queue of pending responses
    std::string mUri;                                   ///< uri from client request
    
    IL_DAEMON_LIB_NS::TimerQueue::delegate_t mRespQueueTimeoutDelegate;
    IL_DAEMON_LIB_NS::TimerQueue::delegate_t mInactivityTimeoutDelegate;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_HTTP_NS

#endif
