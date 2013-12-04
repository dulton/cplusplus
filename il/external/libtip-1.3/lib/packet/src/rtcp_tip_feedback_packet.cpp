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

#include <iomanip>

#include "rtcp_tip_feedback_packet.h"
using namespace LibTip;

CRtcpAppFeedbackPacket::CRtcpAppFeedbackPacket()
{
    memset(&mFB, 0, sizeof(mFB));
    IncrSize(sizeof(mFB));

    SetType(RTPFB);
    SetSubType(APPFB_SUBTYPE);
}

CRtcpAppFeedbackPacket::~CRtcpAppFeedbackPacket()
{
    
}

CRtcpAppFeedbackPacket::AckValue
CRtcpAppFeedbackPacket::GetPacketAckByIndex(uint16_t index) const
{
    // special case, the index corresponding to the packet id is
    // always an ACK
    if (index == NUM_ACK_BITS) {
        return APP_FB_ACK;
    }
    
    if (index < NUM_ACK_BITS) {
        uint8_t bit = (mFB.mAcks[(index/8)] & (1 << (index%8)));
        if (bit) {
            return APP_FB_ACK;
        }
    }

    return APP_FB_NACK;
}

void CRtcpAppFeedbackPacket::SetPacketAckByIndex(uint16_t index, AckValue ack)
{
    if (index >= NUM_ACK_BITS) {
        return;
    }

    uint8_t byte = (index / 8);
    uint8_t mask = (1 << (index % 8));
    
    if (ack == APP_FB_NACK) {
        // clear the bit
        mFB.mAcks[byte] &= ~mask;
    } else {
        // set the bit
        mFB.mAcks[byte] |= mask;
    }
}

CRtcpAppFeedbackPacket::AckValue
CRtcpAppFeedbackPacket::GetPacketAckBySeqNum(uint16_t seqno) const
{
    return GetPacketAckByIndex(ConvertSeqNumToIndex(seqno));
}

void CRtcpAppFeedbackPacket::SetPacketAckBySeqNum(uint16_t seqno, AckValue ack)
{
    SetPacketAckByIndex(ConvertSeqNumToIndex(seqno), ack);
}

uint16_t CRtcpAppFeedbackPacket::ConvertSeqNumToIndex(uint16_t seqno) const
{
    // convert to an index.  16 bit math will handle wrap arounds for us
    return ((NUM_ACK_BITS + seqno) - mFB.mPacketID);
}

uint32_t CRtcpAppFeedbackPacket::PackData(CPacketBuffer& buffer) const
{
    CRtcpPacketSSRC::PackData(buffer);

    buffer.Add(mFB.mTarget);
    buffer.Add(mFB.mPacketID);
    buffer.Add(mFB.mAcks, sizeof(mFB.mAcks));
    
    return buffer.GetBufferSize();
}

int CRtcpAppFeedbackPacket::UnpackData(CPacketBuffer& buffer)
{
    int ret = CRtcpPacketSSRC::UnpackData(buffer);
    if (ret != 0) {
        return ret;
    }

    if (GetType() != RTPFB) {
        return -1;
    }

    if (GetSubType() != APPFB_SUBTYPE) {
        return -1;
    }
    
    buffer.Rem(mFB.mTarget);
    buffer.Rem(mFB.mPacketID);
    buffer.Rem(mFB.mAcks, sizeof(mFB.mAcks));

    return ret;
}

void CRtcpAppFeedbackPacket::ToStream(std::ostream& o, MediaType mType) const
{
    o << "\n\tTYPE:     FEEDBACK"
      << "\n\tTARGET:   " << std::hex << mFB.mTarget
      << "\n\tPACKID #: " << std::dec << mFB.mPacketID
      << "\n\tACKS:     ";

    for (uint16_t i = 0; i < NUM_ACK_BYTES; i++) {
        o << std::hex << std::setfill('0') << std::setw(2)
          << static_cast<int>(mFB.mAcks[i]) << ' ';
    }
}

CRtcpAppExtendedFeedbackPacket::CRtcpAppExtendedFeedbackPacket() {
    memset(&mExtFB, 0, sizeof(mExtFB));
    IncrSize(sizeof(mExtFB));
}

CRtcpAppExtendedFeedbackPacket::~CRtcpAppExtendedFeedbackPacket() {

}

CRtcpAppExtendedFeedbackPacket::AckValidValue
CRtcpAppExtendedFeedbackPacket::GetPacketAckValidByIndex(uint16_t index) const
{
    // out of range bits are VALID, this also catches the special case
    // of the packet id index which is also always valid.
    if (index < NUM_ACK_BITS) {
        uint8_t bit = (mExtFB.mAcksValid[(index/8)] & (1 << (index%8)));
        if (! bit) {
            return APP_FB_INVALID;
        }
    }

    return APP_FB_VALID;
}

void CRtcpAppExtendedFeedbackPacket::SetPacketAckValidByIndex(uint16_t index,
                                                              AckValidValue valid)
{
    if (index >= NUM_ACK_BITS) {
        return;
    }

    uint8_t byte = (index / 8);
    uint8_t mask = (1 << (index % 8));

    if (valid == APP_FB_INVALID) {
        // clear the bit
        mExtFB.mAcksValid[byte] &= ~mask;
    } else {
        // set the bit
        mExtFB.mAcksValid[byte] |= mask;
    }
}

CRtcpAppExtendedFeedbackPacket::AckValidValue
CRtcpAppExtendedFeedbackPacket::GetPacketAckValidBySeqNum(uint16_t seqno) const
{
    return GetPacketAckValidByIndex(ConvertSeqNumToIndex(seqno));
}

void CRtcpAppExtendedFeedbackPacket::SetPacketAckValidBySeqNum(uint16_t seqno,
                                                               AckValidValue valid)
{
    SetPacketAckValidByIndex(ConvertSeqNumToIndex(seqno), valid);
}

uint32_t CRtcpAppExtendedFeedbackPacket::PackData(CPacketBuffer& buffer) const
{
    CRtcpAppFeedbackPacket::PackData(buffer);

    buffer.Add(mExtFB.mPadding);
    buffer.Add(mExtFB.mAcksValid, sizeof(mExtFB.mAcksValid));
    
    return buffer.GetBufferSize();
}

int CRtcpAppExtendedFeedbackPacket::UnpackData(CPacketBuffer& buffer)
{
    int ret = CRtcpAppFeedbackPacket::UnpackData(buffer);
    if (ret != 0) {
        return ret;
    }
    
    buffer.Rem(mExtFB.mPadding);
    buffer.Rem(mExtFB.mAcksValid, sizeof(mExtFB.mAcksValid));

    return ret;
}

void CRtcpAppExtendedFeedbackPacket::ToStream(std::ostream& o, MediaType mType) const
{
    CRtcpAppFeedbackPacket::ToStream(o, mType);
    
    o << "\n\tVALID:    ";
    for (uint16_t i = 0; i < NUM_ACK_BYTES; i++) {
        o << std::hex << std::setfill('0') << std::setw(2)
          << static_cast<int>(mExtFB.mAcksValid[i]) << ' ';
    }
}

