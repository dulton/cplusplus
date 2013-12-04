/// @file
/// @brief FTP common header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _FTP_COMMON_H_
#define _FTP_COMMON_H_

///////////////////////////////////////////////////////////////////////////////

#include "ftp_1_port_server.h"

#ifndef FTP_NS
# define FTP_NS ftp
#endif

#ifndef USING_FTP_NS
# define USING_FTP_NS using namespace ftp
#endif

#ifndef DECL_FTP_NS
# define DECL_FTP_NS namespace ftp {
# define END_DECL_FTP_NS }
#endif

#define FTP_IDL_NS Ftp_1_port_server
///////////////////////////////////////////////////////////////////////////////

#endif
