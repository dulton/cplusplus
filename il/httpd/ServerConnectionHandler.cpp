/// @file
/// @brief Server Connection Handler implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <algorithm>
#include <sstream>
#include <utility>

#include <ace/Message_Block.h>
#include <utils/AppOctetStreamMessageBody.h>
#include <utils/MessageUtils.h>
#include <utils/TextHtmlMessageBody.h>

#include "HttpdLog.h"
#include "HttpServerRawStats.h"
#include "ServerConnectionHandler.h"
#include "ServerConnectionHandlerStates.h"

USING_HTTP_NS;

///////////////////////////////////////////////////////////////////////////////

const ServerConnectionHandler::IdleState ServerConnectionHandler::IDLE_STATE;
const ServerConnectionHandler::HeaderState ServerConnectionHandler::HEADER_STATE;
const ServerConnectionHandler::FlushState ServerConnectionHandler::FLUSH_STATE;

const ACE_Time_Value ServerConnectionHandler::INACTIVITY_TIMEOUT(5 * 60, 0);  // 5 minutes

///////////////////////////////////////////////////////////////////////////////

ServerConnectionHandler::ServerConnectionHandler(uint32_t serial)
    : L4L7_APP_NS::StreamSocketHandler(serial),
      mRandomEngine(std::max(ACE_OS::gettimeofday().usec(), static_cast<suseconds_t>(1))),
      mVersion(http_1_port_server::HttpVersionOptions_VERSION_1_1),
      mType(http_1_port_server::HttpServerType_APACHE),
      mMaxRequests(0),
      mBodyType(l4l7Base_1_port_server::BodyContentOptions_ASCII),
      mBodySize(mRandomEngine),
      mRespLatency(mRandomEngine),
      mStats(0),
      mState(&IDLE_STATE),
      mRequestCount(0),
      mCloseFlag(false),
      mRespQueueTimeoutDelegate(fastdelegate::MakeDelegate(this, &ServerConnectionHandler::HandleResponseQueueTimeout)),
      mInactivityTimeoutDelegate(fastdelegate::MakeDelegate(this, &ServerConnectionHandler::HandleInactivityTimeout))
{
}

///////////////////////////////////////////////////////////////////////////////

ServerConnectionHandler::~ServerConnectionHandler()
{
    UnregisterUpdateTxCountDelegate();
}
///////////////////////////////////////////////////////////////////////////////

void ServerConnectionHandler::SetStatsInstance(stats_t& stats)
{
    mStats = &stats;
    RegisterUpdateTxCountDelegate(boost::bind(&ServerConnectionHandler::UpdateGoodputTxSent, this, _1));
}

///////////////////////////////////////////////////////////////////////////////

void ServerConnectionHandler::UpdateGoodputTxSent(uint64_t sent)
{
    if (mStats)
    {
        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
        mStats->goodputTxBytes += sent;
    }
}

///////////////////////////////////////////////////////////////////////////////

int ServerConnectionHandler::HandleOpenHook(void)
{
    ScheduleTimer(INACTIVITY_TIMER, 0, mInactivityTimeoutDelegate, ACE_OS::gettimeofday() + INACTIVITY_TIMEOUT);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ServerConnectionHandler::HandleInputHook(void)
{
    // Receive pending data into message blocks and append to current input buffer
    messagePtr_t mb(Recv());
    if (!mb)
        return -1;

    const ACE_Time_Value now(ACE_OS::gettimeofday());
    
    mInBuffer = L4L7_UTILS_NS::MessageAppend(mInBuffer, mb);
    ScheduleTimer(INACTIVITY_TIMER, 0, mInactivityTimeoutDelegate, now + INACTIVITY_TIMEOUT);

    // Update receive stats
    if (mStats)
    {
        const size_t rxBytes = mb->total_length();
        ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats->Lock(), -1);
        mStats->goodputRxBytes += rxBytes;
    }
    
    // Process the input buffer line-by-line
    L4L7_UTILS_NS::MessageIterator begin = L4L7_UTILS_NS::MessageBegin(mInBuffer.get());
    L4L7_UTILS_NS::MessageIterator end = L4L7_UTILS_NS::MessageEnd(mInBuffer.get());
    L4L7_UTILS_NS::MessageIterator pos = begin;
    
    while ((pos = std::find(pos, end, '\r')) != end)
    {
        // HTTP uses "\r\n" as the line delimiter so we need to check the next character after '\r'
        if (++pos == end)
            break;

        if (*pos != '\n')
            continue;
        
        // Process a complete line of input
        mState->ProcessInput(*this, begin, ++pos, now);

        // Advance message read pointer and trim input buffer (discarding completely parsed message blocks)
        pos.block()->rd_ptr(pos.ptr());
        mInBuffer = L4L7_UTILS_NS::MessageTrim(mInBuffer, pos.block());
        
        begin = pos;
    }

    // Service the response queue in case responses can be sent immediately
    const ssize_t respCount = ServiceResponseQueue(now);
    if (respCount < 0)
        return -1;
    
    // [Re]start the response queue timer if responses were sent or the timer wasn't running
    if (!mRespQueue.empty() && (respCount || !IsTimerRunning(RESP_QUEUE_TIMER)))
        ScheduleTimer(RESP_QUEUE_TIMER, 0, mRespQueueTimeoutDelegate, mRespQueue.front().absTime);
    
    return mState->ConnectionShouldClose(*this) ? -1 : 0;
}

///////////////////////////////////////////////////////////////////////////////

int ServerConnectionHandler::HandleInactivityTimeout(const ACE_Time_Value& tv, const void *act)
{
    // Inactivity timeout: close the connection
    return -1;
}

///////////////////////////////////////////////////////////////////////////////

int ServerConnectionHandler::HandleResponseQueueTimeout(const ACE_Time_Value& tv, const void *act)
{
    // Service the response queue
    const ssize_t respCount = ServiceResponseQueue(tv);
    if (respCount < 0)
        return -1;

    // Restart the response queue timer as necessary
    if (!mRespQueue.empty())
        ScheduleTimer(RESP_QUEUE_TIMER, 0, mRespQueueTimeoutDelegate, mRespQueue.front().absTime);
    
    // Close the connection once all of the responses have been sent
    return mState->ConnectionShouldClose(*this) ? -1 : 0;
}

///////////////////////////////////////////////////////////////////////////////

void ServerConnectionHandler::QueueResponse(const ACE_Time_Value& absTime, HttpProtocol::StatusCode errorCode, bool closeFlag)
{
    mRespQueue.push(ResponseRecord(absTime, errorCode, L4L7_BASE_NS::BodyContentOptions_ASCII, 0, closeFlag, ""));
}

///////////////////////////////////////////////////////////////////////////////

void ServerConnectionHandler::QueueResponse(const ACE_Time_Value& absTime, bodyType_t bodyType, uint32_t bodySize, bool closeFlag, std::string uri)
{
    mRespQueue.push(ResponseRecord(absTime, HttpProtocol::HTTP_STATUS_OK, bodyType, bodySize, closeFlag, uri));
}

///////////////////////////////////////////////////////////////////////////////

ssize_t ServerConnectionHandler::ServiceResponseQueue(const ACE_Time_Value& absTime)
{
    ssize_t respCount = 0;

    while (!mRespQueue.empty())
    {
        // If response record at head of queue has an absolute time less than time now, we need to send that response
        const ResponseRecord &resp(mRespQueue.front());
        if (absTime < resp.absTime)
            break;
        
        // Build response header
        std::ostringstream oss;
        
        switch (mVersion)
        {
          case http_1_port_server::HttpVersionOptions_VERSION_1_0:
              oss << HttpProtocol::BuildStatusLine(HttpProtocol::HTTP_VER_1_0, resp.statusCode);
              break;
              
          case http_1_port_server::HttpVersionOptions_VERSION_1_1:
              oss << HttpProtocol::BuildStatusLine(HttpProtocol::HTTP_VER_1_1, resp.statusCode);
              break;
              
          default:
              throw EPHXInternal();
        }

        switch (mType)
        {
          case http_1_port_server::HttpServerType_MICROSOFT_IIS:
              oss << HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_SERVER, "Microsoft-IIS/6.0");
              break;
              
          case http_1_port_server::HttpServerType_APACHE:
              oss << HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_SERVER, "Apache/2.0.51");
              break;

          default:
              throw EPHXInternal();
        }

        messagePtr_t respBody;
        switch (resp.bodyType)
        {
          case L4L7_BASE_NS::BodyContentOptions_ASCII:
              respBody = L4L7_UTILS_NS::MessageAlloc(new L4L7_UTILS_NS::TextHtmlMessageBody(resp.bodySize));
              oss << HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_CONTENT_TYPE, "text/html");
              break;

          case L4L7_BASE_NS::BodyContentOptions_BINARY:
              respBody = L4L7_UTILS_NS::MessageAlloc(new L4L7_UTILS_NS::AppOctetStreamMessageBody(resp.bodySize));
              oss << HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_CONTENT_TYPE, "appliation/octet-stream");
              break;

          default:
              throw EPHXInternal();
        }
        
        oss << HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_LAST_MODIFIED, resp.absTime);
        oss << HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_EXPIRES, resp.absTime);
        oss << HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_CONTENT_LENGTH, resp.bodySize);

        if (resp.closeFlag)
            oss << HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_CONNECTION, "close");

        oss << "\r\n";

        const std::string respHeader(oss.str());
        
        // Send response
        if (!Send(respHeader) || !Send(respBody))
            return -1;

        mRespQueue.pop();
        respCount++;

        // Update transmit stats
        if (mStats)
        {
            ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats->Lock(), -1);

            switch (resp.statusCode)
            {
              case 200:
                  mStats->successfulTransactions++;
                  mStats->txResponseCode200++;
                  break;
                  
              case 400:
                  mStats->unsuccessfulTransactions++;
                  mStats->txResponseCode400++;
                  break;
                  
              case 404:
                  mStats->unsuccessfulTransactions++;
                  mStats->txResponseCode404++;
                  break;
                  
              case 405:
                  mStats->unsuccessfulTransactions++;
                  mStats->txResponseCode405++;
                  break;

              default:
                  break;
            }
        }
    }

    return respCount;
}

///////////////////////////////////////////////////////////////////////////////

void ServerConnectionHandler::IdleState::ProcessInput(ServerConnectionHandler& connHandler,
                                                      const L4L7_UTILS_NS::MessageConstIterator& begin, const L4L7_UTILS_NS::MessageConstIterator& end,
                                                      const ACE_Time_Value& now) const
{
    // Parse request line
    HttpProtocol::MethodType method;
    std::string uri;
    HttpProtocol::Version version;

    if (!HttpProtocol::ParseRequestLine(begin, end, method, uri, version))
    {
        QueueResponse(connHandler, now, HttpProtocol::HTTP_STATUS_BAD_REQUEST, true);
        ChangeState(connHandler, &ServerConnectionHandler::FLUSH_STATE);
        return;
    }

    // Validate the method
    switch (method)
    {
      case HttpProtocol::HTTP_METHOD_BLANK_LINE:
          // Blank lines preceding the request line SHOULD be tolerated (RFC 2616, Section 4.1)
          return;

      case HttpProtocol::HTTP_METHOD_GET:
          break;
          
      default:
          // All other methods are not allowed
          QueueResponse(connHandler, now, HttpProtocol::HTTP_STATUS_METHOD_NOT_ALLOWED, true);
          ChangeState(connHandler, &ServerConnectionHandler::FLUSH_STATE);
          return;
    }

    connHandler.mUri = uri;     

    // Set the close flag for HTTP/0.9 and HTTP/1.0 clients
    SetCloseFlag(connHandler, (version < HttpProtocol::HTTP_VER_1_1));

    // Transition to the HEADER state
    ChangeState(connHandler, &ServerConnectionHandler::HEADER_STATE);
}

bool ServerConnectionHandler::IdleState::ConnectionShouldClose(ServerConnectionHandler& connHandler) const
{
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void ServerConnectionHandler::HeaderState::ProcessInput(ServerConnectionHandler& connHandler,
                                                        const L4L7_UTILS_NS::MessageConstIterator& begin, const L4L7_UTILS_NS::MessageConstIterator& end,
                                                        const ACE_Time_Value& now) const
{
    // Parse the header line
    HttpProtocol::HeaderType header;
    std::string value;
    
    if (!HttpProtocol::ParseHeaderLine(begin, end, header, value))
    {
        QueueResponse(connHandler, now, HttpProtocol::HTTP_STATUS_BAD_REQUEST, true);
        ChangeState(connHandler, &ServerConnectionHandler::FLUSH_STATE);
        return;
    }

    // Handle the header line
    switch (header)
    {
      case HttpProtocol::HTTP_HEADER_BLANK_LINE:
      {
          // Blank line signals end of headers: complete the request
          RequestCompleted(connHandler);

          // Queue the response
          const ACE_Time_Value absTime(now + GetResponseLatency(connHandler));
          const bool closeFlag = (GetCloseFlag(connHandler) || !RequestsRemaining(connHandler));

          QueueResponse(connHandler, absTime, GetBodyType(connHandler), GetBodySize(connHandler), closeFlag, connHandler.mUri);

          if (closeFlag)
          {
              // Transition to the FLUSH state so that connection will automatically close after all responses have been sent
              ChangeState(connHandler, &ServerConnectionHandler::FLUSH_STATE);
          }
          else
          {
              // Transition to the IDLE state for the next request
              ChangeState(connHandler, &ServerConnectionHandler::IDLE_STATE);
          }
          break;
      }
      
      case HttpProtocol::HTTP_HEADER_CONNECTION:
          SetCloseFlag(connHandler, (value == "close"));
          break;

      default:
          // Quietly ignore all other headers
          break;
    }
}

bool ServerConnectionHandler::HeaderState::ConnectionShouldClose(ServerConnectionHandler& connHandler) const
{
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void ServerConnectionHandler::FlushState::ProcessInput(ServerConnectionHandler& connHandler,
                                                       const L4L7_UTILS_NS::MessageConstIterator& begin, const L4L7_UTILS_NS::MessageConstIterator& end,
                                                       const ACE_Time_Value& now) const
{
}

bool ServerConnectionHandler::FlushState::ConnectionShouldClose(ServerConnectionHandler& connHandler) const
{
    // Close connection when there are no more queued responses and all output data has been sent
    return ResponseQueueEmpty(connHandler) && OutputQueueEmpty(connHandler);
}

///////////////////////////////////////////////////////////////////////////////
