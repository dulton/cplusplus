/// @file
/// @brief Async completion handler header file
///
///  Copyright (c) 2008 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ASYNC_COMPLETION_HANDLER_H_
#define _ASYNC_COMPLETION_HANDLER_H_

#include <boost/utility.hpp>

#include "AsyncCompletionToken.h"
#include "VoIPCommon.h"

DECL_VOIP_NS

///////////////////////////////////////////////////////////////////////////////

class AsyncCompletionHandler : public boost::noncopyable
{
  public:
    typedef boost::function2<void, WeakAsyncCompletionToken, int> methodCompletionDelegate_t;
    
    void SetMethodCompletionDelegate(methodCompletionDelegate_t delegate) { mMethodCompletionDelegate = delegate; }
    void ClearMethodCompletionDelegate(void) { mMethodCompletionDelegate.clear(); }
    void QueueAsyncCompletion(WeakAsyncCompletionToken weakAct, int data = 0) { mMethodCompletionDelegate(weakAct, data); }
    
  private:
    methodCompletionDelegate_t mMethodCompletionDelegate;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_VOIP_NS

#endif
