/*
 *   Copyright 2010 IMTC, Inc.  All rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "tip_time.h"
#include "private/tip_timer.h"
using namespace LibTip;

CTipTimer::CTipTimer() :
    mNextId(0), mNextExpired((uint64_t) -1)
{

}

CTipTimer::~CTipTimer()
{

}

uint32_t CTipTimer::Register(TimerType type, uint64_t timeoutMsec, uint32_t data)
{
    TimerData tdata;

    tdata.mId      = mNextId++;
    tdata.mType    = type;
    tdata.mExpires = (GetMsecTimestamp() + timeoutMsec);
    tdata.mData    = data;

    mList.push_back(tdata);

    if (timeoutMsec < mNextExpired) {
        mNextExpired = timeoutMsec;
    }
    
    return tdata.mId;
}

Status CTipTimer::GetExpired(TimerType& type, uint32_t& data)
{
    uint64_t nowMsec = GetMsecTimestamp();
    
    TimerList::iterator iter;
    for (iter = mList.begin(); iter != mList.end(); iter++) {
        if (iter->mExpires <= nowMsec) {
            type = iter->mType;
            data = iter->mData;

            mList.erase(iter);
            return TIP_OK;
        }
    }

    CalcNextExpired();
    return TIP_ERROR;
}

Status CTipTimer::Cancel(uint32_t id)
{
    TimerList::iterator iter;
    for (iter = mList.begin(); iter != mList.end(); iter++) {
        if (iter->mId == id) {
            mList.erase(iter);
            CalcNextExpired();
            return TIP_OK;
        }
    }

    return TIP_ERROR;
}

uint64_t CTipTimer::GetNextExpiredTime() const
{
    return mNextExpired;
}

void CTipTimer::CalcNextExpired()
{
    uint64_t nowMsec = GetMsecTimestamp();
    mNextExpired = (uint64_t) -1;
    
    TimerList::iterator iter;
    for (iter = mList.begin(); iter != mList.end(); iter++) {

        if (iter->mExpires <= nowMsec) {
            // doh!  we are late already
            mNextExpired = 0;
            return;
        }

        uint64_t diffMsec = (iter->mExpires - nowMsec);
        if (diffMsec < mNextExpired) {
            mNextExpired = diffMsec;
        }
    }
}
