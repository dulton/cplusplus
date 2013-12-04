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

#ifndef RTCP_TIP_TYPES_H
#define RTCP_TIP_TYPES_H

#include <stdint.h>
#include <string.h>

namespace LibTip {
    // some constants ...
    const uint8_t kRtcpAppAckMask = 0x10;

    const char* const kRtcpAppExtension[1] = { "xcts" };
    
    // different types of Tip packets.  note that these must be
    // converted before sending on the wire (see functions below).
    enum TipPacketType {
        // first 16 use the 'xcts' extension
        RESERVED0  = 0,
        MUXCTRL    = 1,
        RESERVED2  = 2,
        NOTIFY     = 3,
        TIPECHO    = 4,
        TXFLOWCTRL = 5,
        RXFLOWCTRL = 6,
        MEDIAOPTS  = 7,
        REFRESH    = 8,
        RESERVED9  = 9,
        RESERVED10 = 10,
        RESERVED11 = 11,
        SPIMAP     = 12,
        RESERVED13 = 13,
        RESERVED14 = 14,
        REQTOSEND  = 15,

        // second 16 are ACKs of the first 16, also use 'xcts'
        ACK_RESERVED0  = (kRtcpAppAckMask | RESERVED0),    // 16
        ACK_MUXCTRL    = (kRtcpAppAckMask | MUXCTRL),
        ACK_RESERVED2  = (kRtcpAppAckMask | RESERVED2),
        ACK_NOTIFY     = (kRtcpAppAckMask | NOTIFY),
        ACK_TIPECHO    = (kRtcpAppAckMask | TIPECHO),
        ACK_TXFLOWCTRL = (kRtcpAppAckMask | TXFLOWCTRL),
        ACK_RXFLOWCTRL = (kRtcpAppAckMask | RXFLOWCTRL),
        ACK_MEDIAOPTS  = (kRtcpAppAckMask | MEDIAOPTS),
        ACK_REFRESH    = (kRtcpAppAckMask | REFRESH),
        ACK_RESERVED9  = (kRtcpAppAckMask | RESERVED9),
        ACK_RESERVED10 = (kRtcpAppAckMask | RESERVED10),
        ACK_RESERVED11 = (kRtcpAppAckMask | RESERVED11),
        ACK_SPIMAP     = (kRtcpAppAckMask | SPIMAP),
        ACK_RESERVED13 = (kRtcpAppAckMask | RESERVED13),
        ACK_RESERVED14 = (kRtcpAppAckMask | RESERVED14),
        ACK_REQTOSEND  = (kRtcpAppAckMask | REQTOSEND),   // 31
        
        MAX_PACKET_TYPE = 32,
    };

    // get the correct extension for a given type of packet
    inline const char* GetExtensionForTipPacketType(TipPacketType type) {
        return  kRtcpAppExtension[0];
    }

    // convert from an TipPacketType into the correct RTCP packet type
    inline uint8_t ConvertTipToRtcp(TipPacketType type) {
        return (type & 0x1F);
    }

    // convert from an RTCP packet type into an TipPacketType
    inline TipPacketType ConvertRtcpToTip(uint8_t type,
                                          const char* const extension) {

        if (memcmp(extension, kRtcpAppExtension[0], strlen(kRtcpAppExtension[0])) == 0) {
            return (TipPacketType) type;
        }
        
        return MAX_PACKET_TYPE;
    }

    // is app type an ack
    inline bool IsAckTipPacketType(TipPacketType type) {
        return ((type & kRtcpAppAckMask) == kRtcpAppAckMask);
    }

    // is rtcp packet type an ack
    inline bool IsAckRtcpPacketType(uint8_t type) {
        return ((type & kRtcpAppAckMask) == kRtcpAppAckMask);
    }

    // convert from a Tip type to the corresponding ACK
    inline TipPacketType ConvertTipNonAckToAck(TipPacketType type) {
        return (TipPacketType) (type | kRtcpAppAckMask);
    }

    // convert from a Tip ACK type to the corresponding Tip type
    inline TipPacketType ConvertTipAckToNonAck(TipPacketType type) {
        return (TipPacketType) (type & ~kRtcpAppAckMask);
    }
    
    // convert from an RTCP packet type to the ACK of the packet type
    inline uint8_t ConvertRtcpNonAckToAck(uint8_t type) {
        return (type | kRtcpAppAckMask);
    }

    // get Tip type as a string
    const char* GetTipPacketTypeString(TipPacketType type);
    
};
    
#endif
