


#ifndef _UNIQUE_INDEX_TRACKER_H_
#define _UNIQUE_INDEX_TRACKER_H_

#include <boost/dynamic_bitset.hpp>
#include <ace/Thread_Mutex.h>

#include "XmppCommon.h"

DECL_XMPP_NS

class UniqueIndexTracker
{
public:
    UniqueIndexTracker(void);

    uint32_t Assign(void);
    void Release(uint32_t index);
    bool GetIndex(uint32_t index) { return mBitset[index]; }
    void Flip() { mBitset.flip(); }

    static const uint32_t INVALID_INDEX = static_cast<const uint32_t>(-1);
private:
    typedef ACE_Thread_Mutex lock_t;
    typedef boost::dynamic_bitset<unsigned long> bitset_t;


    lock_t mLock;               ///< protects against concurrent access
    size_t mCount;              ///< number of index values assigned
    bitset_t mBitset;           ///< dynamic bitset tracking index values (0 = assigned, 1 = unassigned)
};

END_DECL_XMPP_NS

#endif
