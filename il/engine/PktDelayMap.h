/// @file
/// @brief Packet delay map
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.

#ifndef _PKT_DELAY_MAP_H_
#define _PKT_DELAY_MAP_H_

#include <map>
#include <vector>

#include <engine/EngineCommon.h>

class TestPktDelayMap;

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

class PktDelayMap
{
public:
    typedef std::vector<uint32_t> delays_t;
private:
    typedef std::pair<handle_t, handle_t> key_t;
    typedef std::map<key_t, delays_t> map_t;
public:
    typedef map_t::iterator iterator;

    iterator find(handle_t playHnd, handle_t flowHnd) 
    { 
        return mMap.find(key_t(playHnd, flowHnd)); 
    }

    iterator end() 
    { 
        return mMap.end(); 
    } 

    iterator insert(handle_t playHnd, handle_t flowHnd)
    {
        return mMap.insert(map_t::value_type(key_t(playHnd, flowHnd), delays_t())).first;
    }

    void clear() 
    { 
        mMap.clear(); 
    }

private:
    map_t mMap;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
