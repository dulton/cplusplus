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

#include "rtcp_tip_echo_packet.h"
using namespace LibTip;

CRtcpAppEchoPacket::CRtcpAppEchoPacket() :
    CRtcpTipPacket(TIPECHO)
{
    mEcho.mRcvNtpTime = 0;
    IncrSize(sizeof(mEcho));
}

CRtcpAppEchoPacket::~CRtcpAppEchoPacket()
{

}

bool CRtcpAppEchoPacket::IsMyAck(const CRtcpTipPacket& ack) const
{
    // both must be TIPECHO
    if (GetTipPacketType() != ack.GetTipPacketType()) {
        return false;
    }

    // both must have the same ntp timestamp
    if (GetNtpTime() != ack.GetNtpTime()) {
        return false;
    }

    // we should not be an ack
    if (GetRcvNtpTime() != 0) {
        return false;
    }

    // they must be an ack
    const CRtcpAppEchoPacket* ackp = dynamic_cast<const CRtcpAppEchoPacket*>(&ack);
    if (ackp == NULL || ackp->GetRcvNtpTime() == 0) {
        return false;
    }

    return true;
}

void CRtcpAppEchoPacket::ToStream(std::ostream& o, MediaType mType) const
{
    CRtcpTipPacket::ToStream(o, mType);

    o << "\n\tRCVNTP:   " << std::dec << mEcho.mRcvNtpTime;
}

uint32_t CRtcpAppEchoPacket::PackData(CPacketBuffer& buffer) const
{
    CRtcpTipPacket::PackData(buffer);

    buffer.Add(mEcho.mRcvNtpTime);

    return buffer.GetBufferSize();
}

int CRtcpAppEchoPacket::UnpackData(CPacketBuffer& buffer)
{
    int ret = CRtcpTipPacket::UnpackData(buffer);
    if (ret != 0) {
        return ret;
    }

    buffer.Rem(mEcho.mRcvNtpTime);

    return ret;
}
