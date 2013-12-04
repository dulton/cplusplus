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

#ifndef RTCP_TIP_FEEDBACK_PACKET_H
#define RTCP_TIP_FEEDBACK_PACKET_H

#include "rtcp_packet.h"

namespace LibTip {

    class CRtcpAppFeedbackPacket : public CRtcpPacketSSRC {
    public:
        CRtcpAppFeedbackPacket();
        virtual ~CRtcpAppFeedbackPacket();

        // RTPFB subtype used by APP feedback packets
        static const uint8_t APPFB_SUBTYPE = 30;

        // get/set the target SSRC/CSRC
        uint32_t GetTarget() const { return mFB.mTarget; }
        void SetTarget(uint32_t target) { mFB.mTarget = target; }
        
        // get/set the packet id
        uint16_t GetPacketID() const { return mFB.mPacketID; }
        void SetPacketID(uint16_t pid) { mFB.mPacketID = pid; }

        // constants used in the ACK/NACK bitfield.  these are often
        // used in math related to RTP sequence numbers so make them
        // uint16_t to match.
        static const uint16_t NUM_ACK_BITS = 112;
        static const uint16_t NUM_ACK_BYTES = (NUM_ACK_BITS / 8);

        // possible values for an ACK bit
        enum AckValue {
            APP_FB_NACK = 0,
            APP_FB_ACK
        };

        // get/set the whole ACK bitset
        const uint8_t* GetPacketAcks() const { return mFB.mAcks; }
        void SetPacketAcks(const uint8_t* buffer) {
            memcpy(mFB.mAcks, buffer, NUM_ACK_BYTES);
        }
        
        // get/set a single ACK bit by index (ie the Nth bit in the
        // bitfield).  out of range requests will return NACK or set
        // nothing.
        AckValue GetPacketAckByIndex(uint16_t index) const;
        void SetPacketAckByIndex(uint16_t index, AckValue ack);

        // get/set a single ACK bit by sequence number (ie relative
        // to the packet id).  out of range requests will return NACK
        // or set nothing.
        AckValue GetPacketAckBySeqNum(uint16_t seqno) const;
        void SetPacketAckBySeqNum(uint16_t seqno, AckValue ack);

        virtual void ToStream(std::ostream& o, MediaType mType = MT_MAX) const;
        
    protected:
        // convert a sequence number into an index
        uint16_t ConvertSeqNumToIndex(uint16_t seqno) const;
        
        // pack the component data into the packet buffer.
        virtual uint32_t PackData(CPacketBuffer& buffer) const;

        // unpack from a network packet buffer
        virtual int UnpackData(CPacketBuffer& buffer);

        struct FeedbackData {
            uint32_t        mTarget;
            uint16_t        mPacketID;
            uint8_t         mAcks[NUM_ACK_BYTES];
        };
        
        FeedbackData mFB;
    };

    // extended version of the feedback packet which includes a mask
    // which marks some ACK bits as valid or invalid.
    class CRtcpAppExtendedFeedbackPacket : public CRtcpAppFeedbackPacket {
    public:
        CRtcpAppExtendedFeedbackPacket();
        virtual ~CRtcpAppExtendedFeedbackPacket();

        // possible values for an ACK valid bit
        enum AckValidValue {
            APP_FB_INVALID = 0,
            APP_FB_VALID
        };
        
        // get/set the whole ACK valid bitset
        const uint8_t* GetPacketAcksValid() const { return mExtFB.mAcksValid; }
        void SetPacketAcksValid(const uint8_t* buffer) {
            memcpy(mExtFB.mAcksValid, buffer, NUM_ACK_BYTES);
        }
        
        // get/set a single ACK valid bit by index (ie the Nth bit in
        // the bitfield).  out of range requests will return VALID or
        // set nothing, when combined with the GetPacketAckByIndex()
        // method an out of range request will return a VALID NACK.
        AckValidValue GetPacketAckValidByIndex(uint16_t index) const;
        void SetPacketAckValidByIndex(uint16_t index, AckValidValue valid);

        // get/set a single ACK valid bit by sequence number (ie
        // relative to the packet id).  out of range requests will
        // return VALID or set nothing.  when combined with the
        // GetPacketAckBySeqNum() method an out of range request will
        // return a VALID NACK.
        AckValidValue GetPacketAckValidBySeqNum(uint16_t seqno) const;
        void SetPacketAckValidBySeqNum(uint16_t seqno, AckValidValue valid);

        virtual void ToStream(std::ostream& o, MediaType mType = MT_MAX) const;

    protected:
        // pack the component data into the packet buffer.
        virtual uint32_t PackData(CPacketBuffer& buffer) const;

        // unpack from a network packet buffer
        virtual int UnpackData(CPacketBuffer& buffer);

        struct ExtFeedbackData {
            uint16_t mPadding;
            uint8_t  mAcksValid[NUM_ACK_BYTES];
        };
        ExtFeedbackData mExtFB;
    };        
};

#endif
