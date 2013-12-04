/// @file
/// @brief Shared map - controls access to a map of shared pointers
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_UTILS_SHARED_MAP_H_
#define _L4L7_UTILS_SHARED_MAP_H_

#include <ace/Guard_T.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <Tr1Adapter.h>
#include <utils/UtilsCommon.h>

class TestSharedMap;

DECL_L4L7_UTILS_NS

///////////////////////////////////////////////////////////////////////////////

template<class KEY, class VALUE> 
class SharedMap
{
  public:
    typedef std::tr1::shared_ptr<VALUE> shared_ptr_t;
    typedef boost::function1<void, VALUE *> delegate_t;

    bool empty();

    size_t size();

    size_t erase(const KEY& key);

    void insert(const KEY& key, shared_ptr_t& val);

    shared_ptr_t find(const KEY& key);

    void foreach(delegate_t& delegate);

  private:
    typedef ACE_Recursive_Thread_Mutex lock_t;
    typedef boost::unordered_map<KEY, shared_ptr_t> map_t;

    lock_t mLock;
    map_t  mMap;

    friend class TestSharedMap;
};

///////////////////////////////////////////////////////////////////////////////

template<class KEY, class VALUE> 
bool SharedMap<KEY, VALUE>::empty()
{ 
    ACE_GUARD_RETURN(lock_t, guard, mLock, false);
    return mMap.empty(); 
}

///////////////////////////////////////////////////////////////////////////////

template<class KEY, class VALUE> 
size_t SharedMap<KEY, VALUE>::size()
{ 
    ACE_GUARD_RETURN(lock_t, guard, mLock, 0);
    return mMap.size(); 
}

///////////////////////////////////////////////////////////////////////////////

template<class KEY, class VALUE> 
size_t SharedMap<KEY, VALUE>::erase(const KEY& key) 
{ 
    ACE_GUARD_RETURN(lock_t, guard, mLock, 0);
    return mMap.erase(key); 
}

///////////////////////////////////////////////////////////////////////////////

template<class KEY, class VALUE> 
void SharedMap<KEY, VALUE>::insert(const KEY& key, shared_ptr_t& val)
{
    ACE_GUARD(lock_t, guard, mLock);
    mMap[key] = val;
}

///////////////////////////////////////////////////////////////////////////////

template<class KEY, class VALUE> 
typename SharedMap<KEY, VALUE>::shared_ptr_t SharedMap<KEY, VALUE>::find(const KEY& key)
{
    shared_ptr_t retval;
    ACE_GUARD_RETURN(lock_t, guard, mLock, retval);
    typename map_t::iterator iter = mMap.find(key);
    if (iter != mMap.end())
    {
        retval = iter->second;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////

template<class KEY, class VALUE> 
void SharedMap<KEY, VALUE>::foreach(delegate_t& delegate)
{
    ACE_GUARD(lock_t, guard, mLock);
    BOOST_FOREACH(typename map_t::value_type& value, mMap)
    {
        // because the map is locked we can return a raw pointer here
        delegate(value.second.get());
    }
}
///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_UTILS_NS

#endif
