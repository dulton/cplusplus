///
/// @file
/// @brief This file contains the constants and macros specific to rawtcpd logging
///
/// Copyright (c) 2007 by Spirent Communications Inc.
/// All Rights Reserved.
///
/// This software is confidential and proprietary to Spirent Communications Inc.
/// No part of this software may be reproduced, transmitted, disclosed or used
/// in violation of the Software License Agreement without the expressed
/// written consent of Spirent Communications Inc.
///
/// $File: //TestCenter/integration/content/traffic/l4l7/il/rawtcpd/RawtcpdLog.h $
/// $Revision: #1 $
/// \n<b>Last submission by:</b>
/// <ul>
/// <li>$Author: songkamongkol $</li>
/// <li>$DateTime: 2011/08/05 16:25:08 $</li>
/// <li>$Change: 572841 $</li>
/// </ul>
///

#ifndef _RAW_TCPD_LOG_H_
#define _RAW_TCPD_LOG_H_

#include <phxlog/phxlog.h>

///////////////////////////////////////////////////////////////////////////////

/*
 * Unfortunately the macro gets resolved before the resolution to PHX_LOCAL_ID_X happens
 * Which means we have to define the logger mapping in here...bugger !
 */
#define LOG_ALL_LOGGER          root_logger
#define LOG_MPS_LOGGER          logger_local_id_1
#define LOG_CLIENT_LOGGER       logger_local_id_2
#define LOG_SERVER_LOGGER       logger_local_id_3
#define LOG_SOCKET_LOGGER       logger_local_id_4

#define LOG_MPS_FLAG            PHX_LOCAL_ID_1_FLAG
#define LOG_CLIENT_FLAG         PHX_LOCAL_ID_2_FLAG
#define LOG_SERVER_FLAG         PHX_LOCAL_ID_3_FLAG
#define LOG_SOCKET_FLAG         PHX_LOCAL_ID_4_FLAG
#define LOG_ALL_FLAG            (LOG_MPS_FLAG | LOG_CLIENT_FLAG | LOG_SERVER_FLAG | LOG_SOCKET_FLAG)

#define PHX_LOG_OFF             PHX_LOG_PRIORITY_NONE
#define PHX_LOG_ALL             PHX_LOG_DEBUG

#define LOG_MPS                 PHX_LOCAL_ID_1
#define LOG_CLIENT              PHX_LOCAL_ID_2
#define LOG_SERVER              PHX_LOCAL_ID_3
#define LOG_SOCKET              PHX_LOCAL_ID_4
#define LOG_ALL                 (LOG_MPS | LOG_CLIENT | LOG_SERVER | LOG_SOCKET)

///////////////////////////////////////////////////////////////////////////////

#endif
