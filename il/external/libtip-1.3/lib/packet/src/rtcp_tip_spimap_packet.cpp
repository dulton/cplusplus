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

#include "rtcp_tip_spimap_packet.h"
using namespace LibTip;

CRtcpAppSpiMapPacket::CRtcpAppSpiMapPacket() :
    CRtcpTipPacket(SPIMAP)
{
    memset(&mSpiMap, 0, sizeof(mSpiMap));
    mSpiMap.srtpProfile = SPIMAP_SRTP_PROFILE_AES128_CM_SHA1_80;
    mSpiMap.ektProfile  = SPIMAP_EKT_PROFILE_AES128_ECB;

    IncrSize(sizeof(mSpiMap));
}

CRtcpAppSpiMapPacket::~CRtcpAppSpiMapPacket()
{

}

void CRtcpAppSpiMapPacket::SetSrtpSalt(uint8_t* data)
{
    if (data == NULL) {
        return;
    }

    memcpy(mSpiMap.srtpSalt, data, SPIMAP_SRTP_SALT_LENGTH);
}

void CRtcpAppSpiMapPacket::SetKek(uint8_t* data)
{
    if (data == NULL) {
        return;
    }

    memcpy(mSpiMap.kek, data, SPIMAP_KEK_LENGTH);
}

uint32_t CRtcpAppSpiMapPacket::PackData(CPacketBuffer& buffer) const
{
    CRtcpTipPacket::PackData(buffer);

    buffer.Add(mSpiMap.spi);
    buffer.Add(mSpiMap.srtpProfile);
    buffer.Add(mSpiMap.ektProfile);
    buffer.Add(mSpiMap.srtpSalt, SPIMAP_SRTP_SALT_LENGTH);
    buffer.Add(mSpiMap.kek, SPIMAP_KEK_LENGTH);

    return buffer.GetBufferSize();
}

int CRtcpAppSpiMapPacket::UnpackData(CPacketBuffer& buffer)
{
    int ret = CRtcpTipPacket::UnpackData(buffer);
    if (ret != 0) {
        return ret;
    }

    buffer.Rem(mSpiMap.spi);
    buffer.Rem(mSpiMap.srtpProfile);
    if (mSpiMap.srtpProfile != SPIMAP_SRTP_PROFILE_AES128_CM_SHA1_80) {
        return -1;
    }

    buffer.Rem(mSpiMap.ektProfile);
    if (mSpiMap.ektProfile != SPIMAP_EKT_PROFILE_AES128_ECB) {
        return -1;
    }

    buffer.Rem(mSpiMap.srtpSalt, SPIMAP_SRTP_SALT_LENGTH);
    buffer.Rem(mSpiMap.kek, SPIMAP_KEK_LENGTH);

    return ret;
}

void CRtcpAppSpiMapPacket::ToStream(std::ostream& o, MediaType mType) const
{
    CRtcpTipPacket::ToStream(o, mType);

    o << "\n\tSPI:      " << std::dec << mSpiMap.spi
      << "\n\tSRTP_PROF:" << static_cast<int>(mSpiMap.srtpProfile)
      << "\n\tEKT_PROF: " << static_cast<int>(mSpiMap.ektProfile)
      << "\n\tSRTP_SALT:**************"
      << "\n\tKEK:      ****************";
}
