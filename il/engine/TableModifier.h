/// @file
/// @brief Table modifiers
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _TABLE_MODIFIERS_H_
#define _TABLE_MODIFIERS_H_

#include <engine/EngineCommon.h>

#include "AbstModifier.h"

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

class TableModifier : public AbstModifierCrtp<TableModifier>
{
  public:
    typedef std::vector<uint8_t> Value_t;
    typedef std::vector<Value_t> Table_t;

    TableModifier(const Table_t& table, int32_t stutter) :
        mTable(table), 
        mStutterMax(stutter > 0 ? stutter : 0), 
        mCursor(0), 
        mStutterCount(0)
    {
    }

    void SetCursor(int32_t posIndex)
    {
        if (mTable.empty())
        {
            SetChildCursor(posIndex / (mStutterMax + 1));   
            return;
        }

        if (mStutterMax > 0)
        {
            mCursor = uint32_t(posIndex / (mStutterMax + 1) % mTable.size());
            mStutterCount = uint32_t(posIndex % (mStutterMax + 1) % mTable.size());
            SetChildCursor(posIndex / ((mStutterMax + 1) * mTable.size()));
        }
        else
        {
            mCursor = posIndex % mTable.size();
            SetChildCursor(posIndex / mTable.size());
        }
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

        if (mCursor >= mTable.size())
        {
            mCursor = 0;
            if (this->HasChild()) this->GetChild()->Next(); // chain
        }
    }
    
    void GetValue(uint8_t * buffer, bool reverse = false) const
    {
        if (mCursor < mTable.size())
        {
            if (!reverse)
                std::copy(&mTable[mCursor][0], &mTable[mCursor][0] + mTable[mCursor].size(), buffer);
            else
                std::reverse_copy(&mTable[mCursor][0], &mTable[mCursor][0] + mTable[mCursor].size(), buffer);
                
        }
    }

    size_t GetSize() const
    {
        if (mCursor < mTable.size())
        {
            return mTable[mCursor].size();
        }

        return 0;
    }

  private:
    void SetChildCursor(int32_t childCursor)
    {
        if (this->HasChild()) 
        {
            this->GetChild()->SetCursor(childCursor);
        }
    }

    const Table_t& mTable;
    uint32_t mStutterMax;
    uint32_t mCursor;
    uint32_t mStutterCount;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
