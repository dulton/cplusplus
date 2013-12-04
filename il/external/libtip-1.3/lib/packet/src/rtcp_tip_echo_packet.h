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

#ifndef RTCP_TIP_ECHO_PACKET_H
#define RTCP_TIP_ECHO_PACKET_H

#include "rtcp_packet.h"

namespace LibTip {

    class CRtcpAppEchoPacket : public CRtcpTipPacket {
    public:
        CRtcpAppEchoPacket();
        virtual ~CRtcpAppEchoPacket();

        // get/set receive ntp time
        uint64_t GetRcvNtpTime() const {
            return mEcho.mRcvNtpTime;
        }
        void SetRcvNtpTime(uint64_t ntpTime) {
            mEcho.mRcvNtpTime = ntpTime;
        }

        // ECHO packets do not use the ACK_TIPECHO type to specify
        // acks.  instead if the receive ntp time is non-zero then
        // this is an ACK.  return true if the given packet is the ack
        // of this object.
        virtual bool IsMyAck(const CRtcpTipPacket& ack) const;

        virtual void ToStream(std::ostream& o, MediaType mType = MT_MAX) const;

    protected:
        // pack the component data into the packet buffer.
        virtual uint32_t PackData(CPacketBuffer& buffer) const;

        // unpack from a network packet buffer
        virtual int UnpackData(CPacketBuffer& buffer);

        struct EchoData {
            uint64_t mRcvNtpTime;
        };
        EchoData mEcho;
    };

};

#endif
