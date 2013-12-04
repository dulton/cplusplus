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

#ifndef RTCP_TIP_REQTOSEND_PACKET_H
#define RTCP_TIP_REQTOSEND_PACKET_H

#include "rtcp_packet.h"

namespace LibTip {

    class CRtcpAppReqToSendPacketBase : public CRtcpTipPacket {
    protected:
        // type should be either REQTOSEND or ACK_REQTOSEND
        CRtcpAppReqToSendPacketBase(TipPacketType type);
        
    public:
        virtual ~CRtcpAppReqToSendPacketBase();

        // valid flag values
        enum {
            REQTOSEND_STOP = 0,
            REQTOSEND_START = 1,
        };
            
        // get/set the flags
        uint32_t GetFlags() const { return mReqToSend.mFlags; }
        void SetFlags(uint32_t flags) { mReqToSend.mFlags = flags; }

        // get/set the video positions
        uint16_t GetVideoPos() const { return mReqToSend.mVideoPos; }
        void SetVideoPos(uint16_t pos) { mReqToSend.mVideoPos = pos; }
        
        // get/set the audio positions
        uint16_t GetAudioPos() const { return mReqToSend.mAudioPos; }
        void SetAudioPos(uint16_t pos) { mReqToSend.mAudioPos = pos; }
        
        virtual void ToStream(std::ostream& o, MediaType mType = MT_MAX) const;

    protected:
        // pack the component data into the packet buffer.
        virtual uint32_t PackData(CPacketBuffer& buffer) const;

        // unpack from a network packet buffer
        virtual int UnpackData(CPacketBuffer& buffer);
    
        struct RtcpAppReqToSend {
            uint32_t mFlags;
            uint16_t mVideoPos;
            uint16_t mAudioPos;
        };
        RtcpAppReqToSend mReqToSend;
    };

    class CRtcpAppReqToSendPacket : public CRtcpAppReqToSendPacketBase {
    public:
        CRtcpAppReqToSendPacket();
        virtual ~CRtcpAppReqToSendPacket();
    };

    class CRtcpAppReqToSendAckPacket : public CRtcpAppReqToSendPacketBase {
    public:
        CRtcpAppReqToSendAckPacket();
        CRtcpAppReqToSendAckPacket(const CRtcpAppReqToSendPacket& packet);
        
        virtual ~CRtcpAppReqToSendAckPacket();
    };
};

#endif
