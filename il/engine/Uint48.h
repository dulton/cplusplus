/// @file
/// @brief uint48_t declaration
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _UINT48_T_
#define _UINT48_T_

#include <engine/EngineCommon.h>

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

class uint48_t
{
  public:
    uint48_t()
        : mHi(0), mLo(0)
    {
    }

    uint48_t(uint16_t hi, uint32_t lo)
        : mHi(hi), mLo(lo)
    {
    }

    explicit uint48_t(uint16_t u16)
        : mHi(0), mLo(u16)
    {
    }

    explicit uint48_t(int i)
        : mHi(0), mLo(i)
    {
    }

    explicit uint48_t(uint32_t u32)
        : mHi(0), mLo(u32)
    {
    }

    uint48_t& operator<<=(int shift)
    {
        if (shift <= 0)
            return *this;

        if (shift >= 48)
        {
            mHi = 0;
            mLo = 0;
        }
        else if (shift >= 32)
        {
            mHi = mLo << (shift - 32);
            mLo = 0;
        }
        else if (shift > 16)
        {
            mHi = mLo >> (32 - shift);
            mLo <<= shift; 
        }
        else
        {
            mHi = (mHi << shift) | (mLo >> 32 - shift);
            mLo <<= shift;
        }
        return *this;
    } 

    uint48_t operator<<(int shift) const
    {
        uint48_t result = *this;
        result <<= shift;
        return result;
    }

    uint48_t& operator>>=(int shift)
    {
        if (shift <= 0)
            return *this;
        
        if (shift >= 48)
        {
            mHi = 0;
            mLo = 0;
        }
        else if (shift >= 32)
        {
            mLo = mHi >> (shift - 32);
            mHi = 0;
        }
        else if (shift > 16)
        {
            mLo = (mLo >> shift) | (mHi << (32 - shift));
            mHi = 0;
        }
        else 
        {
            mLo = (mLo >> shift) | (mHi << (32 - shift));
            mHi >>= shift;
        }
        return *this;
    } 

    uint48_t operator>>(int shift) const
    {
        uint48_t result = *this;
        result >>= shift;
        return result;
    }

    uint48_t& operator*=(uint32_t other)
    {
        uint64_t result64 = this->toU64();
        uint64_t other64 = other;
        result64 *= other64;
        mHi = (result64 >> 32);
        mLo = (result64 & 0xffffffff);
        return *this;
    }

    const uint48_t operator*(uint32_t other) const 
    {
        uint48_t result = *this;
        result *= other;
        return result;
    }

    uint48_t& operator+=(const uint48_t& other)
    {
        mHi += other.mHi;
        mLo += other.mLo;
        if (mLo < other.mLo)
            ++mHi; // CARRY
        return *this;
    }

    const uint48_t operator+(const uint48_t& other) const 
    {
        uint48_t result = *this;
        result += other;
        return result;
    }

    uint48_t& operator-=(const uint48_t& other)
    {
        if (mLo < other.mLo)
            --mHi; // BORROW
        mHi -= other.mHi;
        mLo -= other.mLo;
        return *this;
    }

    const uint48_t operator-(const uint48_t& other) const 
    {
        uint48_t result = *this;
        result -= other;
        return result;
    }

    const uint48_t operator-() const 
    {
        uint48_t result; // zero
        result -= *this;
        return result;
    }

    const uint48_t operator~() const 
    {
        return uint48_t(~mHi, ~mLo);
    }

    // lowest byte mask operator
    uint8_t operator&(uint8_t byte)
    {
        return mLo & byte;
    }

    uint48_t& operator&=(const uint48_t& other)
    {
        mHi &= other.mHi;
        mLo &= other.mLo;
        return *this;
    }

    const uint48_t operator&(const uint48_t& other) const 
    {
        uint48_t result = *this;
        result &= other;
        return result;
    }

    uint48_t& operator|=(const uint48_t& other)
    {
        mHi |= other.mHi;
        mLo |= other.mLo;
        return *this;
    }

    const uint48_t operator|(const uint48_t& other) const 
    {
        uint48_t result = *this;
        result |= other;
        return result;
    }

    bool operator==(const uint48_t& other) const 
    {
        return (other.mHi == mHi) && (other.mLo == mLo);
    }

    bool operator!=(const uint48_t& other) const
    {
        return !(*this == other);
    }

    uint64_t toU64() const
    {
        return (uint64_t(mHi) << 32) | uint64_t(mLo);
    }

#ifndef UNIT_TEST
  private:
#endif
    uint16_t mHi;
    uint32_t mLo;
} __attribute__((packed)); // make sure it is exactly 48 bits

extern std::ostream& operator<<(std::ostream& os, const uint48_t& i);

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
