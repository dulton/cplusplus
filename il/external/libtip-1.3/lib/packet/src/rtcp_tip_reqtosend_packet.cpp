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

#include "rtcp_tip_reqtosend_packet.h"
using namespace LibTip;

CRtcpAppReqToSendPacketBase::CRtcpAppReqToSendPacketBase(TipPacketType type) :
    CRtcpTipPacket(type)
{
    memset(&mReqToSend, 0, sizeof(mReqToSend));
    IncrSize(sizeof(mReqToSend));
}

CRtcpAppReqToSendPacketBase::~CRtcpAppReqToSendPacketBase()
{

}

uint32_t CRtcpAppReqToSendPacketBase::PackData(CPacketBuffer& buffer) const
{
    CRtcpTipPacket::PackData(buffer);

    buffer.Add(mReqToSend.mFlags);
    buffer.Add(mReqToSend.mVideoPos);
    buffer.Add(mReqToSend.mAudioPos);
    
    return buffer.GetBufferSize();
}

int CRtcpAppReqToSendPacketBase::UnpackData(CPacketBuffer& buffer)
{
    int ret = CRtcpTipPacket::UnpackData(buffer);
    if (ret != 0) {
        return ret;
    }

    buffer.Rem(mReqToSend.mFlags);
    buffer.Rem(mReqToSend.mVideoPos);
    buffer.Rem(mReqToSend.mAudioPos);

    return ret;
}

void CRtcpAppReqToSendPacketBase::ToStream(std::ostream& o, MediaType mType) const
{
    CRtcpTipPacket::ToStream(o, mType);

    o << "\n\tFLAGS:    " << std::dec << mReqToSend.mFlags;

    if (mReqToSend.mFlags == CRtcpAppReqToSendPacketBase::REQTOSEND_STOP) {
        o << " (STOP)";
    } else if (mReqToSend.mFlags == CRtcpAppReqToSendPacketBase::REQTOSEND_START) {
        o << " (START)";
    } else {
        o << " (UNKNOWN)";
    }

    o << "\n\tVIDEOPOS: " << std::showbase << std::hex << mReqToSend.mVideoPos << ' ';
    PositionsToStream(o, mReqToSend.mVideoPos, VIDEO);
    
    o << "\n\tAUDIOPOS: " << std::showbase << std::hex << mReqToSend.mAudioPos << ' ';
    PositionsToStream(o, mReqToSend.mAudioPos, AUDIO);
}


CRtcpAppReqToSendPacket::CRtcpAppReqToSendPacket() :
    CRtcpAppReqToSendPacketBase(REQTOSEND)
{

}

CRtcpAppReqToSendPacket::~CRtcpAppReqToSendPacket()
{

}

CRtcpAppReqToSendAckPacket::CRtcpAppReqToSendAckPacket() :
    CRtcpAppReqToSendPacketBase(ACK_REQTOSEND)
{

}

CRtcpAppReqToSendAckPacket::CRtcpAppReqToSendAckPacket(const CRtcpAppReqToSendPacket& packet) :
    CRtcpAppReqToSendPacketBase(ACK_REQTOSEND)
{
    SetNtpTime(packet.GetNtpTime());
    SetFlags(packet.GetFlags());
    SetVideoPos(packet.GetVideoPos());
    SetAudioPos(packet.GetAudioPos());
}

CRtcpAppReqToSendAckPacket::~CRtcpAppReqToSendAckPacket()
{

}

