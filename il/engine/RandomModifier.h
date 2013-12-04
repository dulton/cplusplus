/// @file
/// @brief Random modifier header
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _RANDOM_MODIFIER_H_
#define _RANDOM_MODIFIER_H_

#include <engine/EngineCommon.h>
#include <boost/random/linear_congruential.hpp>

#include "Uint48.h"
#include "Uint128.h"
#include "AbstModifier.h"

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

template<typename T>
class RandomModifier : public AbstModifierCrtp<RandomModifier<T> >
{
  public:
    RandomModifier(T mask, uint32_t recycle, uint32_t seed, int32_t stutter = 0)
        : mMask(mask), 
          mCount((recycle != 0) ? recycle : 1), 
          mSeed(seed), 
          mStutterMax(stutter > 0 ? stutter : 0), 
          mCursor(0), 
          mStutterCount(0)
    {
        MakeValue();
    }

    void SetCursor(int32_t posIndex)
    {
        if (mStutterMax > 0)
        {
            mCursor = uint32_t(posIndex / (mStutterMax + 1) % mCount);
            mStutterCount = uint32_t(posIndex % (mStutterMax + 1) % mCount);
            SetChildCursor(posIndex / ((mStutterMax + 1) * mCount));
        }
        else
        {
            mCursor = posIndex % mCount;
            SetChildCursor(posIndex / mCount);
        }
        MakeValue();
    }

    void Next()
    {
        if (mStutterMax > 0)
        {
            ++mStutterCount;
            if (mStutterCount > mStutterMax)
            {
                mStutterCount = 0;
                ++mCursor;
            }
        }
        else
        {
            ++mCursor;
        }

        if (mCursor >= mCount)
        {
            mCursor = 0;
            if (this->HasChild()) this->GetChild()->Next(); // chain
        }

        MakeValue();
    }
   
    void GetValue(uint8_t * buffer, bool reverse = false) const
    {
        // NOTE: if we don't care about having matching values across different
        //       endian-nesses we can replace this with a simple memcpy
        T value = mValue;
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
    void SetChildCursor(int32_t childCursor)
    {
        if (this->HasChild()) 
        {
            this->GetChild()->SetCursor(childCursor);
        }
    }

    void MakeValue()
    {
        // this re-seeding is probably more expensive than generating 
        // numbers over and over but is necessary to support SetCursor
        mNumberGen.seed(int32_t(mCursor ^ mSeed));
        mValue = T(mNumberGen()) & mMask;
    }

    bool HasMask() const
    {
        return (T(~mMask) != T(0));
    }

    T mMask;
    uint32_t mCount;
    uint32_t mSeed;
    uint32_t mStutterMax;
    uint32_t mCursor;
    uint32_t mStutterCount;
    T mValue;

    typedef boost::rand48 NumberGen_t;
    NumberGen_t mNumberGen;
};

template<>
void RandomModifier<uint48_t>::MakeValue()
{
    // this re-seeding is probably more expensive than generating 
    // numbers over and over but is necessary to support SetCursor
    mNumberGen.seed(int32_t(mCursor ^ mSeed));
    mValue = mMask & uint48_t(mNumberGen(), mNumberGen());
}

template<>
void RandomModifier<uint64_t>::MakeValue()
{
    // this re-seeding is probably more expensive than generating 
    // numbers over and over but is necessary to support SetCursor
    mNumberGen.seed(int32_t(mCursor ^ mSeed));
    mValue = mMask & ((uint64_t(mNumberGen()) << 32) | mNumberGen());
}

template<>
void RandomModifier<uint128_t>::MakeValue()
{
    // this re-seeding is probably more expensive than generating 
    // numbers over and over but is necessary to support SetCursor
    mNumberGen.seed(int32_t(mCursor ^ mSeed));
    mValue = mMask & uint128_t((uint64_t(mNumberGen()) << 32) | mNumberGen(), (uint64_t(mNumberGen()) << 32) | mNumberGen());
}


///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
