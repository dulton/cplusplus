/// @file
/// @brief uint128_t declaration
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _UINT128_T_
#define _UINT128_T_

#include <engine/EngineCommon.h>

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

class uint128_t
{
  public:
    uint128_t()
        : mHi(0), mLo(0)
    {
    }

    uint128_t(uint64_t hi, uint64_t lo)
        : mHi(hi), mLo(lo)
    {
    }

    explicit uint128_t(uint16_t u16)
        : mHi(0), mLo(u16)
    {
    }

    explicit uint128_t(int i)
        : mHi(0), mLo(i)
    {
    }

    explicit uint128_t(uint32_t u32)
        : mHi(0), mLo(u32)
    {
    }

    explicit uint128_t(uint64_t u64)
        : mHi(0), mLo(u64)
    {
    }

    uint128_t& operator<<=(int shift)
    {
        if (shift <= 0)
            return *this;

        if (shift >= 128)
        {
            mHi = 0;
            mLo = 0;
        }
        else if (shift >= 64)
        {
            mHi = mLo << (shift - 64);
            mLo = 0;
        }
        else
        {
            mHi = (mHi << shift) | (mLo >> 64 - shift);
            mLo <<= shift;
        }
        return *this;
    } 

    uint128_t operator<<(int shift) const
    {
        uint128_t result = *this;
        result <<= shift;
        return result;
    }

    uint128_t& operator>>=(int shift)
    {
        if (shift <= 0)
            return *this;
        
        if (shift >= 128)
        {
            mHi = 0;
            mLo = 0;
        }
        else if (shift >= 64)
        {
            mLo = mHi >> (shift - 64);
            mHi = 0;
        }
        else 
        {
            mLo = (mLo >> shift) | (mHi << (64 - shift));
            mHi >>= shift;
        }
        return *this;
    } 

    uint128_t operator>>(int shift) const
    {
        uint128_t result = *this;
        result >>= shift;
        return result;
    }

    uint128_t& operator*=(uint32_t other)
    {
        *this = *this * other;
        return *this;
    }

    const uint128_t operator*(uint32_t other) const;

    uint128_t& operator+=(const uint128_t& other)
    {
        mHi += other.mHi;
        mLo += other.mLo;
        if (mLo < other.mLo)
            ++mHi; // CARRY
        return *this;
    }

    const uint128_t operator+(const uint128_t& other) const 
    {
        uint128_t result = *this;
        result += other;
        return result;
    }

    uint128_t& operator-=(const uint128_t& other)
    {
        if (mLo < other.mLo)
            --mHi; // BORROW
        mHi -= other.mHi;
        mLo -= other.mLo;
        return *this;
    }

    const uint128_t operator-(const uint128_t& other) const 
    {
        uint128_t result = *this;
        result -= other;
        return result;
    }

    const uint128_t operator-() const 
    {
        uint128_t result; // zero
        result -= *this;
        return result;
    }

    const uint128_t operator~() const 
    {
        return uint128_t(~mHi, ~mLo);
    }

    // lowest byte mask operator
    uint8_t operator&(uint8_t byte)
    {
        return mLo & byte;
    }

    uint128_t& operator&=(const uint128_t& other)
    {
        mHi &= other.mHi;
        mLo &= other.mLo;
        return *this;
    }

    const uint128_t operator&(const uint128_t& other) const 
    {
        uint128_t result = *this;
        result &= other;
        return result;
    }

    uint128_t& operator|=(const uint128_t& other)
    {
        mHi |= other.mHi;
        mLo |= other.mLo;
        return *this;
    }

    const uint128_t operator|(const uint128_t& other) const 
    {
        uint128_t result = *this;
        result |= other;
        return result;
    }

    bool operator==(const uint128_t& other) const 
    {
        return (other.mHi == mHi) && (other.mLo == mLo);
    }

    bool operator!=(const uint128_t& other) const
    {
        return !(*this == other);
    }

#ifndef UNIT_TEST
  private:
#endif
    uint64_t mHi;
    uint64_t mLo;

    friend std::ostream& operator<<(std::ostream& os, const uint128_t& i);
}; 

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
