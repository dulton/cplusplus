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

#ifndef TIP_RELAY_H_
#define TIP_RELAY_H_

#include <stdint.h>

namespace LibTip {

    /* predeclare used classes */
    class CRtcpPacket;
    class CRtcpTipPacket;
    
    /**
     * User interface class for relay implementations.  CTipRelay
     * provides functions to classify incoming packets and aid users
     * in forwarding those packets to the correct receivers.
     */
    class CTipRelay {
    public:
        /**
         * Public constructor.  Only user accessible constructor.
         */
        CTipRelay();
        ~CTipRelay();

        /**
         * Classification types.  For each packet given to the relay,
         * one of these classification types will be returned,
         * instructing the user on where to forward the packet for
         * further processing.
         */
        enum Classification {
            NOT_TIP,  // not an Tip packet
            SYSTEM,       // system level, forward to CTip instance
            MEDIA_SOURCE, // media source level, forward to CTipMedia instance
            MEDIA_SINK,   // media sink level, forward to CTipMedia instance
        };

        /**
         * Classify a received packet.  This API will process the
         * given packet (without changing it in any way), and return
         * the classification type.  For the MEDIA_* classification
         * types, the parameter "position" will be set to a bitfield
         * of the position(s) of the addressed media instance.  In
         * some cases it is impossible to determine the specific media
         * instance from a packet, in this case the position field
         * will be set to 0xFFFF and the user should forward the
         * packet to all valid media instances.  A position of (1 <<
         * POS_CONTROL) for a MEDIA_* classification indicates that
         * the packet affects all media on the box but that the action
         * typically requires coordination at a higher level (versus
         * simply forwarding the packet to all media instances).  A
         * position bitmask of 0 is invalid and the packet should be
         * dropped.
         *
         * @param buffer pointer to the packet received
         * @param size length of the received packet
         * @param pos position of the associated media instance is set here
         * @return the classification type
         */
        Classification Classify(uint8_t* buffer, uint32_t size, uint16_t& pos);

    protected:
        /**
         * Helper function to classify APP RTCP packets.
         */
        CTipRelay::Classification ClassifyAPP(CRtcpPacket* rtcp, uint16_t& pos);
        
        /**
         * Helper function to classify FB RTCP packets.
         */
        CTipRelay::Classification ClassifyFB(CRtcpPacket* rtcp, uint16_t& pos);
        
        /**
         * Helper function to retrive position information from a given packet.
         */
        void GetPositionForPacket(CRtcpTipPacket* packet, uint16_t& pos);
    };
};

#endif
