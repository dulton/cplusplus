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

#ifndef TIP_PACKET_TRANSMIT_H
#define TIP_PACKET_TRANSMIT_H

#include "tip_constants.h"

namespace LibTip {

    /**
     * Interface class for transmitting tip packets.  Users should
     * inherit from this class and provide an implementation for the
     * Transmit() method.  This method will be invoked each time
     * the Tip library needs to send a packet.
     */
    class CTipPacketTransmit {
    public:
        CTipPacketTransmit() {}
        virtual ~CTipPacketTransmit() {}

        /**
         * Transmit the given packet.  This method is invoked whenever
         * the Tip library needs to transmit a packet.
         *
         * @param pktBuffer pointer to the packet buffer
         * @param pktSize size of the buffer
         * @param mType whether the packet is related to AUDIO or VIDEO
         * @return TIP_OK if transmission suceeds, otherwise TIP_ERRO
         */
        virtual Status Transmit(const uint8_t* pktBuffer, uint32_t pktSize,
                                MediaType mType) = 0;
    };

};

#endif
