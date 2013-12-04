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

#ifndef RTCP_SDES_PACKET_H
#define RTCP_SDES_PACKET_H

#include <vector>

#include "rtcp_packet.h"

namespace LibTip {

    // simple SDES packet class, only supports empty SDES for now
    class CRtcpSDESPacket : public CRtcpPacket {
    public:
        CRtcpSDESPacket();
        ~CRtcpSDESPacket();

        // add an empty SDES chunk to the packet
        void AddChunk(uint32_t ssrc);

        // get the SSRC of the Nth SDES chunk
        uint32_t GetChunkSSRC(uint32_t index) const;

    protected:
        virtual uint32_t PackData(CPacketBuffer& buffer) const;
        virtual int UnpackData(CPacketBuffer& buffer);

        virtual void ClearData();

        struct EmptyChunk {
            uint32_t mSSRC;
            uint32_t mPad;
        };

        typedef std::vector<EmptyChunk> EmptyChunkVector;
        EmptyChunkVector mChunk;
    };
};

#endif
