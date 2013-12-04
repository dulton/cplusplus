///
/// @file
/// @brief This file contains the constants and macros specific to ftpd logging
///
/// Copyright (c) 2007 by Spirent Communications Inc.
/// All Rights Reserved.
///
/// This software is confidential and proprietary to Spirent Communications Inc.
/// No part of this software may be reproduced, transmitted, disclosed or used
/// in violation of the Software License Agreement without the expressed
/// written consent of Spirent Communications Inc.
///
/// $File: //TestCenter/integration/content/traffic/l4l7/il/ftpd/FtpdLog.h $
/// $Revision: #1 $
/// \n<b>Last submission by:</b>
/// <ul>
/// <li>$Author: songkamongkol $</li>
/// <li>$DateTime: 2011/08/05 16:25:08 $</li>
/// <li>$Change: 572841 $</li>
/// </ul>
///

#ifndef _FTPD_LOG_H_
#define _FTPD_LOG_H_

#include <phxlog/phxlog.h>
#include <iostream>
#include <sstream>

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
#define LOG_DTM_LOGGER          logger_local_id_5

#define LOG_MPS_FLAG            PHX_LOCAL_ID_1_FLAG
#define LOG_CLIENT_FLAG         PHX_LOCAL_ID_2_FLAG
#define LOG_SERVER_FLAG         PHX_LOCAL_ID_3_FLAG
#define LOG_SOCKET_FLAG         PHX_LOCAL_ID_4_FLAG
#define LOG_DTM_FLAG            PHX_LOCAL_ID_5_FLAG
#define LOG_ALL_FLAG            (LOG_MPS_FLAG | LOG_CLIENT_FLAG | LOG_SERVER_FLAG | LOG_SOCKET_FLAG | LOG_DTM_FLAG)

#define PHX_LOG_OFF             PHX_LOG_PRIORITY_NONE
#define PHX_LOG_ALL             PHX_LOG_DEBUG

#define LOG_MPS                 PHX_LOCAL_ID_1
#define LOG_CLIENT              PHX_LOCAL_ID_2
#define LOG_SERVER              PHX_LOCAL_ID_3
#define LOG_SOCKET              PHX_LOCAL_ID_4
#define LOG_DTM                 PHX_LOCAL_ID_5
#define LOG_ALL                 (LOG_MPS | LOG_CLIENT | LOG_SERVER | LOG_SOCKET | PHX_LOCAL_ID_5)

#define FTP_QUIET_LOG(port, logger, msg) void(0) 

#ifdef UNIT_TEST

// define to control how much stuff comes out of the unit test
// 0 means TRACE level logs, 1 is DEBUG, 2 is INFO, 3 is ERR, 4 or higher is no log
#define UNIT_TEST_VERBOSITY 2 


#define FTP_CERR_LOG(port, logger, msg)  do {\
		std::ostringstream os ; \
        os << "Port: " << port << " Logger: " << logger << " Msg: " << msg ; \
        std::cerr << os.str() << std::endl ; \
    } while(0) ;


#ifndef UNIT_TEST_VERBOSITY
 #error "Reminder: Add UNIT_TEST_VERBOSITY definition to FtpdLog.h"
#endif

#if UNIT_TEST_VERBOSITY == 0	
 #define FTP_LOG_TRACE  FTP_CERR_LOG
 #define FTP_LOG_DEBUG  FTP_CERR_LOG
 #define FTP_LOG_INFO   FTP_CERR_LOG
 #define FTP_LOG_ERR    FTP_CERR_LOG 
#elif  UNIT_TEST_VERBOSITY == 1
 #define FTP_LOG_TRACE  FTP_QUIET_LOG 
 #define FTP_LOG_DEBUG  FTP_CERR_LOG
 #define FTP_LOG_INFO   FTP_CERR_LOG
 #define FTP_LOG_ERR    FTP_CERR_LOG
#elif UNIT_TEST_VERBOSITY == 2
 #define FTP_LOG_TRACE  FTP_QUIET_LOG
 #define FTP_LOG_DEBUG  FTP_QUIET_LOG
 #define FTP_LOG_INFO   FTP_CERR_LOG
 #define FTP_LOG_ERR    FTP_CERR_LOG
#elif  UNIT_TEST_VERBOSITY == 3
 #define FTP_LOG_TRACE  FTP_QUIET_LOG
 #define FTP_LOG_DEBUG  FTP_QUIET_LOG
 #define FTP_LOG_INFO   FTP_QUIET_LOG
 #define FTP_LOG_ERR    FTP_CERR_LOG 
#else 
 #define FTP_LOG_TRACE  FTP_QUIET_LOG
 #define FTP_LOG_DEBUG  FTP_QUIET_LOG
 #define FTP_LOG_INFO   FTP_QUIET_LOG
 #define FTP_LOG_ERR    FTP_QUIET_LOG 
#endif


#else // UNIT_TEST

#define FTP_LOG_TRACE  FTP_QUIET_LOG
#define FTP_LOG_DEBUG  TC_LOG_DEBUG_LOCAL
#define FTP_LOG_INFO   TC_LOG_INFO_LOCAL
#define FTP_LOG_ERR    TC_LOG_ERR_LOCAL
#endif

///////////////////////////////////////////////////////////////////////////////

#endif
