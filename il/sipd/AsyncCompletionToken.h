/// @file
/// @brief Asynchronous completion token header file
///
///  Copyright (c) 2008 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ASYNC_COMPLETION_TOKEN_H_
#define _ASYNC_COMPLETION_TOKEN_H_

#include <boost/function.hpp>
#include <phxexception/PHXException.h>
#include <Tr1Adapter.h>

#include "VoIPCommon.h"

DECL_VOIP_NS

///////////////////////////////////////////////////////////////////////////////

class AsyncCompletionFunction
{
  public:
    typedef boost::function1<void, int> callback_t;
    
    explicit AsyncCompletionFunction(callback_t callback)
        : mCallback(callback)
    {
    }

    void Call(int data) 
    {
        if (mCallback)
            mCallback(data);
        else
            throw EPHXInternal("AsyncCompletionFunction::Call");
    }

  private:
    callback_t mCallback;
};

///////////////////////////////////////////////////////////////////////////////

typedef std::tr1::shared_ptr<AsyncCompletionFunction> AsyncCompletionToken;
typedef std::tr1::weak_ptr<AsyncCompletionFunction> WeakAsyncCompletionToken;

typedef WeakAsyncCompletionToken asyncCompletionToken_t;
typedef boost::function2<void, WeakAsyncCompletionToken, int> methodCompletionDelegate_t;

inline AsyncCompletionToken MakeAsyncCompletionToken(AsyncCompletionFunction::callback_t callback)
{
    return AsyncCompletionToken(new AsyncCompletionFunction(callback));
}

///////////////////////////////////////////////////////////////////////////////

END_DECL_VOIP_NS

#endif
