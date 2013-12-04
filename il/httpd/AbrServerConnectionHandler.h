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

#ifndef _ABR_SERVER_CONNECTION_HANDLER_H_
#define _ABR_SERVER_CONNECTION_HANDLER_H_

#include "ServerConnectionHandler.h"

#include "AbrPlaylist.h"

DECL_HTTP_NS

namespace abr {
    class AbrPlaylist;
}

///////////////////////////////////////////////////////////////////////////////

class AbrServerConnectionHandler : public ServerConnectionHandler
{
  public:
    typedef ABR_UTILS_NS::requestType_t requestType_t;
    typedef ABR_UTILS_NS::sessionType_t sessionType_t;
    typedef HttpServerRawStats stats_t;
    typedef HttpVideoServerRawStats videoStats_t;
    typedef http::abr::AbrPlaylist AbrPlaylist;

    explicit AbrServerConnectionHandler(uint32_t serial = 0);
    ~AbrServerConnectionHandler();

    AbrPlaylist *GetAbrPlaylist() { return mPlaylist; }
    void SetAbrPlaylist(AbrPlaylist * playlist) { mPlaylist = playlist; }

    void SetVideoStatsInstance(videoStats_t& videoStats) { mVideoStats = &videoStats; }
    videoStats_t* GetVideoStatsInstance() { return mVideoStats; }


  private:
    ssize_t ServiceResponseQueue(const ACE_Time_Value& absTime);
    videoStats_t *mVideoStats;                                    ///< video server stats
    AbrPlaylist *mPlaylist;                             ///< playlist object >
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_HTTP_NS

#endif

