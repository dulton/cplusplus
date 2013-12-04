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

#ifndef RTCP_TIP_SPIMAP_PACKET_H
#define RTCP_TIP_SPIMAP_PACKET_H

#include <list>
#include "rtcp_packet.h"

namespace LibTip {

    class CRtcpAppSpiMapPacket : public CRtcpTipPacket {
    public:
        CRtcpAppSpiMapPacket();
        virtual ~CRtcpAppSpiMapPacket();

        // get, set the SPI value
        uint16_t GetSPI() const { return mSpiMap.spi; }
        void SetSPI(uint16_t spi) { mSpiMap.spi = spi; }

        // get the SRTP protection profile, only one supported value
        enum {
            SPIMAP_SRTP_PROFILE_AES128_CM_SHA1_80 = 1
        };
        uint8_t GetSrtpProtectionProfile() const { return mSpiMap.srtpProfile; }
        
        // get the EKT protection profile, only one supported value
        enum {
            SPIMAP_EKT_PROFILE_AES128_ECB = 1
        };
        uint8_t GetEktProtectionProfile() const { return mSpiMap.ektProfile; }

        // get/set the SRTP salt value
        enum {
            SPIMAP_SRTP_SALT_LENGTH = 14
        };
        const uint8_t* GetSrtpSalt() const { return mSpiMap.srtpSalt; }
        void SetSrtpSalt(uint8_t* data);

        // get/set the KEK
        enum {
            SPIMAP_KEK_LENGTH = 16
        };
        const uint8_t* GetKek() const { return mSpiMap.kek; }
        void SetKek(uint8_t* data);

        virtual void ToStream(std::ostream& o, MediaType mType = MT_MAX) const;

    protected:
        // pack the component data into the packet buffer.
        virtual uint32_t PackData(CPacketBuffer& buffer) const;

        // unpack from a network packet buffer
        virtual int UnpackData(CPacketBuffer& buffer);

        struct SpiMapData {
            uint16_t spi;
            uint8_t  srtpProfile;
            uint8_t  ektProfile;
            uint8_t  srtpSalt[SPIMAP_SRTP_SALT_LENGTH];
            uint8_t  kek[SPIMAP_KEK_LENGTH];
        };
        SpiMapData mSpiMap;
    };
};

#endif
