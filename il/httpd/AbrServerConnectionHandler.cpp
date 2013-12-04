/// @file
/// @brief ABR Server Connection Handler implementation
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
#include "HttpVideoServerRawStats.h"
#include "ServerConnectionHandlerStates.h"
#include "AbrServerConnectionHandler.h"

USING_HTTP_NS;

///////////////////////////////////////////////////////////////////////////////

AbrServerConnectionHandler::AbrServerConnectionHandler(uint32_t serial)
    : ServerConnectionHandler(serial),
      mVideoStats(0),
      mPlaylist(0)
{
}

///////////////////////////////////////////////////////////////////////////////

AbrServerConnectionHandler::~AbrServerConnectionHandler()
{
}

///////////////////////////////////////////////////////////////////////////////

ssize_t AbrServerConnectionHandler::ServiceResponseQueue(const ACE_Time_Value& absTime)
{
    ssize_t respCount = 0;
    while (!GetResponseQueue()->empty())
    {
        // If response record at head of queue has an absolute time less than time now, we need to send that response
        const ResponseRecord &resp(GetResponseQueue()->front());

        if (absTime < resp.absTime)
            break;

        // Build response header
        std::ostringstream oss;

        requestType_t requestType = ABR_UTILS_NS::UNKNOWN;
        std::string *playlist = 0;
        std::string contentType;
        uint8_t bitrate;
        sessionType_t sessionType;
        uint16_t mediaSequenceNumber = 0;
        std::string tempPlaylist;

        size_t sizeOfBody = 0;

        if (
                !(ABR_UTILS_NS::ParseUri(&(resp.uri), &sessionType, &requestType, &mediaSequenceNumber, &bitrate)) || 
                ((GetAbrPlaylist()->GetVideoStreamType() == http_1_port_server::HttpVideoServerStreamTypeOptions_PROGRESSIVE) && (sessionType == ABR_UTILS_NS::LIVE))
           )
        {
            // Set 404 status
            ResponseRecord &removeRespConstness = const_cast<ResponseRecord &>(resp);

            requestType = ABR_UTILS_NS::TEXT;
            removeRespConstness.statusCode = HttpProtocol::HTTP_STATUS_NOT_FOUND;
            removeRespConstness.bodyType = L4L7_BASE_NS::BodyContentOptions_ASCII;
            removeRespConstness.bodySize = sizeOfBody = 0;
            removeRespConstness.closeFlag = true;
        }
        
        //std::cout << requestType << "sessiontype:" << sessionType << ", uri:" << resp.uri << ", mediaSequenceNumber:" << mediaSequenceNumber << ", bitrate:" << bitrate << std::endl;
        switch (GetVersion())
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

        switch (GetType())
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

        switch(requestType)
        {
          case ABR_UTILS_NS::TEXT:       // Handle non-video cases
          {
            switch (resp.bodyType)
            {
              case L4L7_BASE_NS::BodyContentOptions_ASCII:
              {
                respBody = L4L7_UTILS_NS::MessageAlloc(new L4L7_UTILS_NS::TextHtmlMessageBody(resp.bodySize));
                contentType = "text/html";
                break;
              }
              case L4L7_BASE_NS::BodyContentOptions_BINARY:
              {
                respBody = L4L7_UTILS_NS::MessageAlloc(new L4L7_UTILS_NS::AppOctetStreamMessageBody(resp.bodySize));
                contentType = "application/octet-stream";
                break;
              }
              default:
                  throw EPHXInternal();
            }
            sizeOfBody = resp.bodySize;
            break;
          }
          case ABR_UTILS_NS::VARIANT_PLAYLIST:
          {
            switch(sessionType)
            {
              case ABR_UTILS_NS::LIVE:
                playlist = GetAbrPlaylist()->GetLiveVariantPlaylist();
                break;
              case ABR_UTILS_NS::VOD:
                playlist = GetAbrPlaylist()->GetVodVariantPlaylist();
                break;
            }

            sizeOfBody = playlist->size();
            respBody = L4L7_UTILS_NS::MessageAlloc(new ABR_UTILS_NS::AbrStreamMessageBody(playlist));
            contentType = GetAbrPlaylist()->GetPlaylistMimeType();
            break;
          }
          case ABR_UTILS_NS::MANIFEST:
          {
            switch(sessionType)
            {
              case ABR_UTILS_NS::LIVE:
                tempPlaylist = GetAbrPlaylist()->CreateAbrLiveBitratePlaylist(bitrate, &(resp.absTime));
                sizeOfBody = tempPlaylist.size();
                respBody = L4L7_UTILS_NS::MessageAlloc(new ABR_UTILS_NS::AbrStreamMessageBody(&tempPlaylist));
                break;
              case ABR_UTILS_NS::VOD:
                playlist = GetAbrPlaylist()->GetPlaylistFromBitrateMapAt(bitrate);
                sizeOfBody = playlist->size();
                respBody = L4L7_UTILS_NS::MessageAlloc(new ABR_UTILS_NS::AbrStreamMessageBody(playlist));
                break;
            }

            contentType = GetAbrPlaylist()->GetPlaylistMimeType();

            break;
          }
          case ABR_UTILS_NS::FRAGMENT:
          {
            sizeOfBody = GetAbrPlaylist()->GetSizeFromFragmentSizeMapAt(bitrate);
            respBody = L4L7_UTILS_NS::MessageAlloc(new L4L7_UTILS_NS::AppOctetStreamMessageBody(sizeOfBody));
            contentType = GetAbrPlaylist()->GetFragmentMimeType();
            break;
          }
          default:
          {
            throw EPHXInternal();
          }
        }

        oss << HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_CONTENT_TYPE, contentType);
        oss << HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_LAST_MODIFIED, resp.absTime);
        oss << HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_EXPIRES, resp.absTime);
        oss << HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_CONTENT_LENGTH, sizeOfBody);

        if (resp.closeFlag)
          oss << HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_CONNECTION, "close");

        oss << "\r\n";

        const std::string respHeader(oss.str());

        // Send response
        if (!Send(respHeader) || !Send(respBody))
          return -1;

        GetResponseQueue()->pop();
        respCount++;

        // Update transmit stats
        if (GetStatsInstance())
        {
            ACE_GUARD_RETURN(stats_t::lock_t, guard, GetStatsInstance()->Lock(), -1);

            switch (resp.statusCode)
            {
              case 200:
                  GetStatsInstance()->successfulTransactions++;
                  GetStatsInstance()->txResponseCode200++;
                  break;

              case 400:
                  GetStatsInstance()->unsuccessfulTransactions++;
                  GetStatsInstance()->txResponseCode400++;
                  break;

              case 404:
                  GetStatsInstance()->unsuccessfulTransactions++;
                  GetStatsInstance()->txResponseCode404++;
                  break;

              case 405:
                  GetStatsInstance()->unsuccessfulTransactions++;
                  GetStatsInstance()->txResponseCode405++;
                  break;

              default:
                  break;
            }
        } // end of stats instance

        if (mVideoStats) {

            ACE_GUARD_RETURN(videoStats_t::lock_t, guard, mVideoStats->Lock(), -1);

            if ((requestType == ABR_UTILS_NS::MANIFEST) || (requestType == ABR_UTILS_NS::VARIANT_PLAYLIST)) {
                mVideoStats->totalTxManifestFiles++;
            }

            if (requestType == ABR_UTILS_NS::FRAGMENT) {
                // Update media fragment stats
                switch (bitrate)
                {
                    case http_1_port_server::HttpVideoServerBitrateOptions_BR_96:
                        mVideoStats->txMediaFragments96Kbps++;
                        break;
                    case http_1_port_server::HttpVideoServerBitrateOptions_BR_150:
                        mVideoStats->txMediaFragments150Kbps++;
                        break;
                    case http_1_port_server::HttpVideoServerBitrateOptions_BR_240:
                        mVideoStats->txMediaFragments240Kbps++;
                        break;
                    case http_1_port_server::HttpVideoServerBitrateOptions_BR_256:
                        mVideoStats->txMediaFragments256Kbps++;
                        break;
                    case http_1_port_server::HttpVideoServerBitrateOptions_BR_440:
                        mVideoStats->txMediaFragments440Kbps++;
                        break;
                    case http_1_port_server::HttpVideoServerBitrateOptions_BR_640:
                        mVideoStats->txMediaFragments640Kbps++;
                        break;
                    case http_1_port_server::HttpVideoServerBitrateOptions_BR_800:
                        mVideoStats->txMediaFragments800Kbps++;
                        break;
                    case http_1_port_server::HttpVideoServerBitrateOptions_BR_840:
                        mVideoStats->txMediaFragments840Kbps++;
                        break;
                    case http_1_port_server::HttpVideoServerBitrateOptions_BR_1240:
                        mVideoStats->txMediaFragments1240Kbps++;
                        break;
                    case http_1_port_server::HttpVideoServerBitrateOptions_BR_64:
                        mVideoStats->txMediaFragments64Kbps++;
                        break;
                }
            } // end of stats update for media fragments
        } // end of video stats instance
    }   // end of the master while loop

    return respCount;
}

///////////////////////////////////////////////////////////////////////////////

