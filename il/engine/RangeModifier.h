/// @file
/// @brief Range modifier header
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _RANGE_MODIFIER_H_
#define _RANGE_MODIFIER_H_

#define EXCLUDE_TYPES
#include <utils/GenericFieldModifierImpl.h>

#include <engine/EngineCommon.h>

#include "AbstModifier.h"

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

template<typename T>
class RangeModifier : public AbstModifierCrtp<RangeModifier<T> >
{
  public:
    RangeModifier(T start, T step, T mask, int32_t stutter, uint32_t recycle)
        : mImpl(start, step, mask, stutter, recycle), 
          mMask(mask),
          mWrapCount((stutter + 1) * recycle) // not protected against overflow
    {
    }

    void SetCursor(int32_t posIndex)
    {
        mImpl.SetReadCursor(posIndex);
        if (this->HasChild()) this->GetChild()->SetCursor(posIndex / mWrapCount);
    }

    void Next()
    {
        int32_t posIndex = mImpl.GetReadCursor() + 1;

        if (posIndex >= mWrapCount)
        {
            posIndex = 0;
            if (this->HasChild()) this->GetChild()->Next(); // chain
        }

        mImpl.SetReadCursor(posIndex);
    }
   
    void GetValue(uint8_t * buffer, bool reverse = false) const
    {
        T value;
        const_cast<Impl_t&>(mImpl).GetCurrValue(value);
        size_t bytes = GetSize();
        ssize_t incr;

        if (!reverse)
        {
            // confusingly, if we are not reversing the bytes
            // we write them backwards because we start at the 
            // least significant byte
            buffer += bytes - 1;
            incr = -1;
        }
        else
        {
            incr = 1;
        }

        if (!HasMask())
        {
            while (bytes--)
            {
                *buffer = value & uint8_t(0xff);
                buffer += incr;
                value >>= 8;
            }
        } 
        else
        {
            T mask = mMask;
            while (bytes--)
            {
                uint8_t byteMask = mask & uint8_t(0xff);
                *buffer &= ~byteMask;
                *buffer |= value & byteMask;
                buffer += incr;
                value >>= 8;
                mask >>= 8;
            }
        }
    }

    size_t GetSize() const
    {
        return sizeof(T);
    }

  private:
    bool HasMask() const
    {
        return (T(~mMask) != T(0));
    }

    typedef stc::common::CGenericFieldModifierImpl<T, T> Impl_t;
    Impl_t mImpl;
    T mMask;
    int32_t mWrapCount;    
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
