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

#ifndef RTCP_TIP_REFRESH_PACKET_H
#define RTCP_TIP_REFRESH_PACKET_H

#include "rtcp_packet.h"

namespace LibTip {

    class CRtcpAppRefreshPacket : public CRtcpTipPacket {
    public:
        CRtcpAppRefreshPacket();
        virtual ~CRtcpAppRefreshPacket();

        // get/set the target SSRC/CSRC
        uint32_t GetTarget() const { return mRefresh.mTarget; }
        void SetTarget(uint32_t target) { mRefresh.mTarget = target; }

        // valid flag values
        enum {
            // bit 0-1
            REFRESH_REQUIRE_IDR = 0x00000000,
            REFRESH_REQUIRE_GDR = 0x00000001,
            REFRESH_PREFER_IDR  = 0x00000002,
            REFRESH_PREFER_GDR  = 0x00000003,

            // bits 2-31 undefined
        };
    
        // get/set the flags
        uint32_t GetFlags() const { return mRefresh.mFlags; }
        void SetFlags(uint32_t flags) { mRefresh.mFlags = flags; }
    
    protected:
        // pack the component data into the packet buffer.
        virtual uint32_t PackData(CPacketBuffer& buffer) const;

        // unpack from a network packet buffer
        virtual int UnpackData(CPacketBuffer& buffer);
    
        // clear out dynamic data prior to unpack
        virtual void ClearData();

        struct RtcpAppRefresh {
            uint32_t mTarget;
            uint32_t mFlags;
        };
        RtcpAppRefresh mRefresh;
    };

};

#endif
