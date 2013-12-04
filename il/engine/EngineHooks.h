/// @file
/// @brief L4L7 engine hooks API
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_ENGINE_HOOKS_H_
#define _L4L7_ENGINE_HOOKS_H_

#include <boost/function.hpp>

#include <engine/EngineCommon.h>

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

class EngineHooks
{
  public:
    typedef boost::function2<void, uint16_t, const std::string&> logDelegate_t;

    static void SetLogs(logDelegate_t errLog, logDelegate_t warnLog, logDelegate_t infoLog)
    {
        mErrLog = errLog;
        mWarnLog = warnLog;
        mInfoLog = infoLog;
    }
   
    static void LogErr(uint16_t port, const std::string& msg)  
    {
        if (mErrLog) mErrLog(port, msg);
    }

    static void LogWarn(uint16_t port, const std::string& msg)  
    {
        if (mWarnLog) mWarnLog(port, msg);
    }

    static void LogInfo(uint16_t port, const std::string& msg)  
    {
        if (mInfoLog) mInfoLog(port, msg);
    }

  private:
    static logDelegate_t mErrLog;
    static logDelegate_t mWarnLog;
    static logDelegate_t mInfoLog;
};


///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
