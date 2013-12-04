/// @file
/// @brief Client Connection Handler implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <cstdlib>
#include <sstream>

#include <ace/Message_Block.h>

#include "ClientConnectionHandler.h"
#include "ClientConnectionHandlerStates.h"
#include "HttpClientRawStats.h"
#include "HttpProtocol.h"

USING_HTTP_NS;

///////////////////////////////////////////////////////////////////////////////

const ClientConnectionHandler::IdleState ClientConnectionHandler::IDLE_STATE;
const ClientConnectionHandler::HeaderState ClientConnectionHandler::HEADER_STATE;
const ClientConnectionHandler::BodyState ClientConnectionHandler::BODY_STATE;

///////////////////////////////////////////////////////////////////////////////

struct ClientConnectionHandler::RequestRecord
{
    RequestRecord(const ACE_Time_Value& theAbsTime) 
        : absTime(theAbsTime)
    {
    }

    ACE_Time_Value absTime;     ///< absolute time that request was sent
};

///////////////////////////////////////////////////////////////////////////////

ClientConnectionHandler::ClientConnectionHandler(uint32_t serial)
    : L4L7_APP_NS::StreamSocketHandler(serial),
      mVersion(http_1_port_server::HttpVersionOptions_VERSION_1_1),
      mKeepAlive(true),
      mMaxRequests(1),
      mMaxPipelineDepth(8),
      mStats(0),
      mState(&IDLE_STATE),
      mRequestCount(0),
      mContentLength(0),
      mCloseFlag(false),
      mStopFlag(false),
      mComplete(false)
{
}

///////////////////////////////////////////////////////////////////////////////

ClientConnectionHandler::~ClientConnectionHandler()
{
    if (mStats && !mReqQueue.empty())
    {
      ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
      mStats->abortedTransactions += mReqQueue.size();
    }
    UnregisterUpdateTxCountDelegate();
}

///////////////////////////////////////////////////////////////////////////////

void ClientConnectionHandler::SetStatsInstance(stats_t& stats)
{
    mStats = &stats;
    RegisterUpdateTxCountDelegate(boost::bind(&ClientConnectionHandler::UpdateGoodputTxSent, this, _1));
}

///////////////////////////////////////////////////////////////////////////////

void ClientConnectionHandler::UpdateGoodputTxSent(uint64_t sent)
{
    if (mStats)
    {
        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
        mStats->goodputTxBytes += sent;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool ClientConnectionHandler::SendRequests(uint32_t maxRequests)
{
    std::ostringstream oss;
    uint32_t requestsSent;

    for (requestsSent = 0; RequestsRemaining() && requestsSent < maxRequests; requestsSent++)
    {
        // Build request header
        std::ostringstream uri;
        uri << "/" << ++mRequestCount << ".html";

        switch (mVersion)
        {
          case http_1_port_server::HttpVersionOptions_VERSION_1_0:
              oss << HttpProtocol::BuildRequestLine(HttpProtocol::HTTP_METHOD_GET, uri.str(), HttpProtocol::HTTP_VER_1_0);
              break;

          case http_1_port_server::HttpVersionOptions_VERSION_1_1:
              oss << HttpProtocol::BuildRequestLine(HttpProtocol::HTTP_METHOD_GET, uri.str(), HttpProtocol::HTTP_VER_1_1);
              break;

          default:
              throw EPHXInternal();
        }

        oss << HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_HOST, mPeerAddrStr);

        if (!mUserAgent.empty())
            oss << HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_USER_AGENT, mUserAgent);

        oss << HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_CONNECTION, RequestsRemaining() ? "keepalive" : "close");
        oss << "\r\n";
    }

    const std::string reqHeader(oss.str());
    const ACE_Time_Value now = ACE_OS::gettimeofday();

    if (!Send(reqHeader))
        return false;

    // Save request records for response timing
    for (uint32_t i = 0; i < requestsSent; i++)
        mReqQueue.push_back(RequestRecord(now));

    // Update transmit stats
    if (mStats)
    {
        ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats->Lock(), -1);
        mStats->attemptedTransactions += requestsSent;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

int ClientConnectionHandler::HandleOpenHook(void)
{
    ACE_INET_Addr peerAddr;
    peer().get_remote_addr(peerAddr);

    char buffer[64];
    mPeerAddrStr = peerAddr.get_host_addr(buffer, sizeof(buffer));

    // Send first requests
    return SendRequests(std::min(mMaxRequests, mMaxPipelineDepth)) ? 0 : -1;
}

///////////////////////////////////////////////////////////////////////////////

int ClientConnectionHandler::HandleInputHook(void)
{
    // Receive pending data into message blocks and append to current input buffer
    messagePtr_t mb(Recv());
    if (!mb)
        return -1;

    const ACE_Time_Value now(ACE_OS::gettimeofday());

    mInBuffer = L4L7_UTILS_NS::MessageAppend(mInBuffer, mb);

    // Update receive stats
    if (mStats)
    {
        const size_t rxBytes = mb->total_length();
        ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats->Lock(), -1);
        mStats->goodputRxBytes += rxBytes;
    }

    // Process the input buffer
    int err = 0;
    while (!err)
    {
        const char * const pos = mInBuffer->rd_ptr();

        if (!mState->ProcessInput(*this, now))
            err = -1;

        if (!mInBuffer || pos == mInBuffer->rd_ptr())
            break;
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////

bool ClientConnectionHandler::IdleState::ProcessInput(ClientConnectionHandler& connHandler, const ACE_Time_Value& now) const
{
    // We shouldn't be receiving any server responses if we haven't made any requests
    if (RequestQueueEmpty(connHandler))
        return false;

    L4L7_UTILS_NS::MessageIterator begin = InBufferBegin(connHandler);
    L4L7_UTILS_NS::MessageIterator end = InBufferEnd(connHandler);
    L4L7_UTILS_NS::MessageIterator pos = std::find(begin, end, '\r');

    // Bail unless we have a complete line ending with '\r\n'
    if (pos == end || ++pos == end || *pos != '\n')
        return true;

    ++pos;

    stats_t *stats = Stats(connHandler);

    // Parse status line
    HttpProtocol::Version version;
    HttpProtocol::StatusCode statusCode;

    if (!HttpProtocol::ParseStatusLine(begin, pos, version, statusCode))
    {
        // Pop the unsuccessful request so it is not counted as an aborted transaction
        RequestQueuePop(connHandler);

        ACE_GUARD_RETURN(stats_t::lock_t, guard, stats->Lock(), -1);
        stats->unsuccessfulTransactions++;
        return false;
    }

    // Advance message read pointer and trim input buffer (discarding completely parsed message blocks)
    pos.block()->rd_ptr(pos.ptr());
    InBufferTrim(connHandler, pos.block());

    // Calculate the response time once we've received a final status code
    RespStatusQueueAdd(connHandler, static_cast<uint16_t>(statusCode));

    // Update response statistics
    {
        switch (statusCode)
        {
          case 200:
              stats->rxResponseCode200++;
              break;

          case 400:
              stats->rxResponseCode400++;
              break;

          case 404:
              stats->rxResponseCode404++;
              break;

          case 405:
              stats->rxResponseCode405++;
              break;

          default:
              break;
        }
    }

    ChangeState(connHandler, &ClientConnectionHandler::HEADER_STATE);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool ClientConnectionHandler::HeaderState::ProcessInput(ClientConnectionHandler& connHandler, const ACE_Time_Value& now) const
{
    L4L7_UTILS_NS::MessageIterator begin = InBufferBegin(connHandler);
    L4L7_UTILS_NS::MessageIterator end = InBufferEnd(connHandler);
    L4L7_UTILS_NS::MessageIterator pos = begin;

    while ((pos = std::find(pos, end, '\r')) != end)
    {
        // Parse the header line
        HttpProtocol::HeaderType header;
        std::string value;

        // HTTP uses "\r\n" as the line delimiter so we need to check the next character after '\r'
        if (++pos == end)
            break;

        if (*pos != '\n')
            continue;

        ++pos;

        // Parse status line
        if (!HttpProtocol::ParseHeaderLine(begin, pos, header, value))
            return false;

        // Advance message read pointer and trim input buffer (discarding completely parsed message blocks)
        pos.block()->rd_ptr(pos.ptr());
        InBufferTrim(connHandler, pos.block());

        // Handle the header line
        switch (header)
        {
          case HttpProtocol::HTTP_HEADER_BLANK_LINE:
          {
              // Blank line signals end of headers: optional body follows
              if (!GetContentLength(connHandler))
              {
                  if (!GetCloseFlag(connHandler) && RequestsRemaining(connHandler) && !GetStopFlag(connHandler))
                  {
                      // Send next request
                      if (!SendRequest(connHandler))
                          return false;
                  }
                  else if (RequestQueueEmpty(connHandler))
                  {
                      // If no more requests remaining close connection
                      return false;
                  }

                  ChangeState(connHandler, &ClientConnectionHandler::IDLE_STATE);
              }
              else
                  ChangeState(connHandler, &ClientConnectionHandler::BODY_STATE);
              return true;
          }

          case HttpProtocol::HTTP_HEADER_CONTENT_LENGTH:
              SetContentLength(connHandler, static_cast<size_t>(atol(value.c_str())));
              break;

          case HttpProtocol::HTTP_HEADER_CONNECTION:
              SetCloseFlag(connHandler, (value == "close"));
              break;

          default:
              // Quietly ignore all other headers
              break;
        }

        begin = pos;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool ClientConnectionHandler::BodyState::ProcessInput(ClientConnectionHandler& connHandler, const ACE_Time_Value& now) const
{
    size_t remaining = GetContentLength(connHandler);

    while (remaining && !InBufferEmpty(connHandler))
    {
        L4L7_UTILS_NS::MessageIterator pos = InBufferBegin(connHandler);
        ACE_Message_Block *mb = pos.block();
        const size_t length = mb->length();

        if (length < remaining)
        {
            InBufferTrim(connHandler, mb->cont());
            remaining -= length;
        }
        else if (length == remaining)
        {
            InBufferTrim(connHandler, mb->cont());
            remaining = 0;
        }
        else // length > remaining
        {
            mb->rd_ptr(remaining);
            remaining = 0;
        }
    }

    SetContentLength(connHandler, remaining);

    if (!remaining)
    {
        {
            ACE_Time_Value respTime;

            respTime = now - RequestQueueFront(connHandler).absTime;
            RequestQueuePop(connHandler);

            const uint64_t respTimeMsec = respTime.msec();

            stats_t *stats = Stats(connHandler);
            ACE_GUARD_RETURN(stats_t::lock_t, guard, stats->Lock(), -1);

            // update tranaction statistics
            const HttpProtocol::StatusCodeCategory category = static_cast<HttpProtocol::StatusCodeCategory>(RespStatusQueueFront(connHandler) / 100);

            switch (category)
            {
              case HttpProtocol::HTTP_STATUS_INFORMATIONAL:
                break;

              case HttpProtocol::HTTP_STATUS_SUCCESS:
                stats->successfulTransactions++;
                break;

              case HttpProtocol::HTTP_STATUS_REDIRECTION:
              case HttpProtocol::HTTP_STATUS_CLIENT_ERROR:
              case HttpProtocol::HTTP_STATUS_SERVER_ERROR:
              default:
                stats->unsuccessfulTransactions++;
                break;
            }
            RespStatusQueuePop(connHandler);
            stats->minResponseTimePerUrlMsec = stats->minResponseTimePerUrlMsec ? std::min(stats->minResponseTimePerUrlMsec, respTimeMsec) : respTimeMsec;
            stats->maxResponseTimePerUrlMsec = std::max(stats->maxResponseTimePerUrlMsec, respTimeMsec);
            stats->sumResponseTimePerUrlMsec += respTimeMsec;
        }


        if (!GetCloseFlag(connHandler) && RequestsRemaining(connHandler) && !GetStopFlag(connHandler))
        {
            // Send next request
            if (!SendRequest(connHandler))
                return false;
        }
        else if (RequestQueueEmpty(connHandler))
        {
            // If no more requests remaining close connection
            return false;
        }

        ChangeState(connHandler, &ClientConnectionHandler::IDLE_STATE);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
