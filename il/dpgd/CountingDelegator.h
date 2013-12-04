/// @file
/// @brief Counting delegator 
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///
///  This class is used to keep track of tx-complete delegates. Each sender will
///  register for a callback when an additional count bytes are sent. When
///  sends happen, UpdateCount is called. Senders are always called in the
///  order registered. 

#ifndef _COUTING_DELEGATOR_H_
#define _COUTING_DELEGATOR_H_

#include "DpgCommon.h"
#include "DpgdLog.h"

#include <boost/utility.hpp>

#include <list>

DECL_DPG_NS

///////////////////////////////////////////////////////////////////////////////

template <class Count, class Delegate>
class CountingDelegator : boost::noncopyable
{
  public:
    CountingDelegator() : mCount(0), mNextCount(0) {}

    void Register(Count count, const Delegate& delegate)
    {
        mNextCount += count;
        if (delegate)
            mDelegateList.push_back(countDelegate_t(mNextCount, delegate));
    }

    void UpdateCount(Count count) 
    {
        mCount += count;

        while (!Empty() && (mDelegateList.front().first <= mCount))
        {
            // pop and _then_ call delegate to avoid recursive issues
            Delegate delegate = mDelegateList.front().second;
            mDelegateList.pop_front();
            if (delegate)
                delegate();
        }
    }

    bool Empty() 
    { 
        return mDelegateList.empty(); 
    }
   
    void Reset()
    {
        mDelegateList.clear();
    }

  private:
    typedef std::pair<Count, Delegate> countDelegate_t;
    typedef std::list<countDelegate_t> delegateList_t;
    delegateList_t mDelegateList;

    Count mCount;          ///< The total count 'sent' as updated by UpdateCount
    Count mNextCount;      ///< The total count registered 

    friend class TestCountingDelegator;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_DPG_NS

#endif
