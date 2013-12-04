

#include <ace/Guard_T.h>
#include "UniqueIndexTracker.h"
#include "XmppdLog.h"

USING_XMPP_NS;


UniqueIndexTracker::UniqueIndexTracker(void) : mCount(0)
{
}

uint32_t UniqueIndexTracker::Assign(void)
{
    ACE_GUARD_RETURN(lock_t, guard, mLock, INVALID_INDEX);

    if (mCount < mBitset.size())
    {
        const size_t pos = mBitset.find_first();
        if (pos == bitset_t::npos)
            throw EPHXInternal();

        mBitset.reset(pos);
        mCount++;
        TC_LOG_DEBUG(0,"[UniqueIndexTracker::Assign] enough spaces assigning:"<<static_cast<uint32_t>(pos));
        return static_cast<uint32_t>(pos);
    }
    else
    {
        mBitset.push_back(false);
        mCount++;
        TC_LOG_DEBUG(0,"shig [UniqueIndexTracker::Assign] out of spaces adding and assigning:"<<static_cast<uint32_t>(mBitset.size() - 1));
        return static_cast<uint32_t>(mBitset.size() - 1);
    }
}

void UniqueIndexTracker::Release(uint32_t index)
{
    ACE_GUARD(lock_t, guard, mLock);
    if(index != INVALID_INDEX){
        mBitset.set(index);
        mCount--;
    }
}

