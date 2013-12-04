/// @file
/// @brief Mock reactor
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _MOCK_REACTOR_H_
#define _MOCK_REACTOR_H_

#include <ace/Reactor.h>

///////////////////////////////////////////////////////////////////////////////

class MockReactor : public ACE_Reactor
{
public:
    MockReactor() :
        mLastEventHandler(0),
        mLastMask(0),
        mDoNotify(false)
    {
    }

    int notify(ACE_Event_Handler *event_handler, ACE_Reactor_Mask mask, ACE_Time_Value *timeout)
    {
        if (event_handler->reactor() == 0)
        {
            event_handler->reactor(this);
        }

        mLastEventHandler = event_handler;
        mLastMask = mask;
        mDoNotify = true;
        return 0;
    }

    int DoNotify()
    {
        if (mLastEventHandler && mDoNotify)
        {
            mDoNotify = false;
            return mLastEventHandler->handle_input(ACE_INVALID_HANDLE);
        }

        return 0;
    }

    // mock getters
    ACE_Event_Handler * GetLastEventHandler() { return mLastEventHandler; }
    ACE_Reactor_Mask GetLastMask() { return mLastMask; }

    ACE_Event_Handler * mLastEventHandler;
    ACE_Reactor_Mask mLastMask;
    bool mDoNotify;
};

///////////////////////////////////////////////////////////////////////////////

#endif

