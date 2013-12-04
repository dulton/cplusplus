/// @file
/// @brief Abstract modifier header
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ABST_MODIFIER_H_
#define _ABST_MODIFIER_H_

#include <engine/EngineCommon.h>

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

class AbstModifier
{
  public:
    AbstModifier() : mChild(0), mParented(false) {}

    virtual ~AbstModifier() {}

    virtual void SetCursor(int32_t posIndex) = 0;

    virtual void Next() = 0;
    
    virtual void GetValue(uint8_t * buffer, bool reverse = false) const = 0;

    virtual size_t GetSize() const = 0;

    virtual AbstModifier * Clone() const = 0;

    // Parent-Child composition for linked modifiers
    
    void SetChild(AbstModifier * child) 
    {   
        mChild = child; 
        mChild->mParented = true;
    }

    AbstModifier * GetChild() const { return mChild; }

    bool HasChild() const { return mChild != 0; }

    bool HasParent() const { return mParented; }

  private:
    AbstModifier * mChild;
    bool mParented;
};

///////////////////////////////////////////////////////////////////////////////

// Uses the CRTP to share the implementation of Clone

template <typename T>
class AbstModifierCrtp : public AbstModifier
{
public:
    virtual AbstModifier * Clone() const { return new T((const T&)(*this)); }
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
