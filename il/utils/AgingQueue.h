/// @file
/// @brief Aging Queue templated implementation
///
///  Copyright (c) 2008 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_UTILS_AGING_QUEUE_H_
#define _L4L7_UTILS_AGING_QUEUE_H_

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <utils/UtilsCommon.h>

using namespace boost::multi_index;

DECL_L4L7_UTILS_NS

///////////////////////////////////////////////////////////////////////////////

template<class ValueType> 
class AgingQueue
{
  public:
    void clear(void) { mQueue.clear(); }
    
    bool empty(void) const { return mQueue.empty(); }
    size_t size(void) const { return mQueue.size(); }
    
    const ValueType& front(void) const { return mQueue.front(); }
    const ValueType& back(void) const { return mQueue.back(); }

    void push(const ValueType& x) { mQueue.push_back(x); }
    void pop(void) { mQueue.pop_front(); }

    size_t erase(const ValueType& x) { return mQueue.get<1>().erase(x); }
    
  private:
    multi_index_container<ValueType,
                          indexed_by<
                              sequenced<>,                              // index 0: list-type index
                              ordered_unique<identity<ValueType> >      // index 1: map-type index by ValueType
                          >
                         > mQueue;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_UTILS_NS

#endif
