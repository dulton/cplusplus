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

#ifndef TIP_TIME_H
#define TIP_TIME_H

#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>

namespace LibTip {

    const uint64_t kNtpTimeScale  = 0x100000000LL;
    const uint64_t kNsecTimeScale = 1000000000LL;
    const uint64_t kUsecTimeScale = 1000000LL;
    const uint64_t kMsecTimeScale = 1000LL;
    const uint64_t kSecTimeScale  = 1LL;

    inline uint64_t RescaleTimestamp(uint64_t inTimestamp, uint64_t inOldScale,
                                     uint64_t inNewScale) {
        // inOldScale should not be zero, rounding errors are
        // possible, overflow is possible
        uint64_t secs = inTimestamp / inOldScale;
        uint64_t subsecs = inTimestamp % inOldScale;
        return (secs * inNewScale + ((subsecs * inNewScale) / inOldScale));
    }

    inline uint64_t GetUsecTimestampFromTimeval(struct timeval* pTimeval) {
        return ((((uint64_t)pTimeval->tv_sec) * kUsecTimeScale) + ((uint64_t)pTimeval->tv_usec));
    }

    inline uint64_t GetUsecTimestamp() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return GetUsecTimestampFromTimeval(&tv);
    }

    inline uint64_t GetMsecTimestamp() {
        return RescaleTimestamp(GetUsecTimestamp(), kUsecTimeScale, kMsecTimeScale);
    }

    inline uint64_t GetNtpTimestampFromTimeval(struct timeval* pTimeval) {
        const uint64_t kNtpToUnixTime = 2208988800LL;
        
        return (((kNtpToUnixTime + (uint64_t)pTimeval->tv_sec) << 32)
                + (((uint64_t)pTimeval->tv_usec) * kNtpTimeScale) / 1000000);
    }

    inline uint64_t GetNtpTimestamp() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return GetNtpTimestampFromTimeval(&tv);
    }
};

#endif


