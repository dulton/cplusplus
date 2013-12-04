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

#ifndef RTCP_TIP_ACK_PACKET_H
#define RTCP_TIP_ACK_PACKET_H

#include "rtcp_packet.h"

namespace LibTip {
    
    class CRtcpTipAckPacket : public CRtcpTipPacket {
    public:
        // consturuct an ack of the given type
        CRtcpTipAckPacket(TipPacketType ackType);

        // construct this ack to be an ack of the passed in packet
        CRtcpTipAckPacket(const CRtcpTipPacket& packet);
    
        virtual ~CRtcpTipAckPacket();

        // get the tip type of the packet this ack corresponds to
        TipPacketType GetAckedType() const {
            return ConvertTipAckToNonAck(GetTipPacketType());
        }
    };

};

#endif
