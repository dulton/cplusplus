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

#ifndef RTCP_PACKET_FACTORY_H
#define RTCP_PACKET_FACTORY_H

#include "packet_buffer.h"
#include "rtcp_packet.h"
#include "rtcp_tip_ack_packet.h"

namespace LibTip {

    class CRtcpPacketFactory {
    public:

        // create an rtcp packet from the given buffer.  packet is
        // returned, caller owns the returned pointer.
        static CRtcpPacket* CreatePacketFromBuffer(CPacketBuffer& buffer);

        // create an ACK packet for the given packet.  caller owns
        // returned packet.
        static CRtcpTipPacket* CreateAckPacket(const CRtcpTipPacket& packet);

    protected:
        // helper functions
        static CRtcpPacket* CreateRRFromBuffer(CPacketBuffer& buffer);
        static CRtcpPacket* CreateSDESFromBuffer(CPacketBuffer& buffer);
        static CRtcpPacket* CreateFBFromBuffer(CPacketBuffer& buffer);
        static CRtcpPacket* CreateAPPFromBuffer(CPacketBuffer& buffer);
        
    private:
        CRtcpPacketFactory();  // do not implement
        ~CRtcpPacketFactory(); // do not implement
    };

};

#endif
