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

#ifndef TIP_PACKET_RECEIVER_H
#define TIP_PACKET_RECEIVER_H

#include "rtcp_packet.h"

namespace LibTip {
    
    class CTipPacketReceiver {
    public:
        CTipPacketReceiver();
        ~CTipPacketReceiver();

        // possible actions to take for a given packet
        enum Action {
            AMPR_DROP, // an old or invalid packet, drop it
            AMPR_DUP,  // a duplication packet, ack it
            AMPR_NEW   // a new packet, process it
        };

        // process a packet and provide the correct action as a return
        // value.  if NEW is returned receiver now owns the packet.
        Action ProcessPacket(CRtcpTipPacket* packet);

        // a packet has been acked, register the last acked time.
        // receiver now owns the ack.
        void RegisterAck(const CRtcpTipPacket& packet, CRtcpTipPacket* ack);

        // for a duplicate packet get the ACK sent the first time this
        // packet was received.
        CRtcpTipPacket* FindDupAck(const CRtcpTipPacket& packet);

        // get the last NEW packet of a type
        CRtcpTipPacket* Find(TipPacketType pType);
        
        // forget that a packet has been received
        void Forget(TipPacketType pType);
        
        // find a packet based on the given id
        CRtcpTipPacket* FindID(void* id);
        
    protected:
        // data structure to track previously received packets
        struct ReceiveData {
            CRtcpTipPacket* mLastPacket;
            CRtcpTipPacket* mLastAckPacket;
            uint64_t        mLastReceived;
            uint64_t        mLastAcked;
            uint64_t        mNumUniqueReceived;
            uint64_t        mNumUniqueAcked;
        };
        ReceiveData mReceiveData[MAX_PACKET_TYPE];

    private:
        // no copy or assignment
        CTipPacketReceiver(const CTipPacketReceiver&);
        CTipPacketReceiver& operator=(const CTipPacketReceiver&);
    };
};

#endif
