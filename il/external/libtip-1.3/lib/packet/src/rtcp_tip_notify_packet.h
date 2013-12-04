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

#ifndef RTCP_TIP_NOTIFY_PACKET_H
#define RTCP_TIP_NOTIFY_PACKET_H

#include <list>

#include "rtcp_tip_tlv.h"
#include "rtcp_packet.h"

namespace LibTip {

    class CRtcpAppNotifyPacket : public CRtcpTipPacket {
    public:
        CRtcpAppNotifyPacket();
        virtual ~CRtcpAppNotifyPacket();

        enum NotifyTag {
            RESERVED = 0,
            SECURITYICON = 3
        };

        // add a TLV to the packet, returns 0 on success.  duplicate TLV
        // tags are allowed.
        int AddTLV(NotifyTag tag, uint8_t* data, uint32_t len);

        // remote the first TLV with the given tag, returns 0 on success
        int RemTLV(NotifyTag tag);
    
        // get a TLV of the given type from the packet.  length of the
        // TLV is returned, -1 bytes means it was not found.  if found
        // and data is not NULL, the TLV data will be copied into
        // data.  the size of data is not checked, the maximum size is
        // CAppMuxTlv::MAX_TLV_DATA_LEN
        int GetTLVByTag(NotifyTag tag, uint8_t* data) const;

        // same as above but return the Nth TLV
        int GetTLVByIndex(uint32_t index, uint8_t* data) const;

        // number of TLVs in the packet
        uint32_t GetTLVCount() const { return mTlv.size(); }
    
        virtual void ToStream(std::ostream& o, MediaType mType = MT_MAX) const;
        
    protected:
        // pack the component data into the packet buffer.
        virtual uint32_t PackData(CPacketBuffer& buffer) const;

        // unpack from a network packet buffer
        virtual int UnpackData(CPacketBuffer& buffer);
    
        // clear out dynamic data prior to unpack
        virtual void ClearData();

        typedef std::list<CTipTlv*> CTlvpList;
        CTlvpList mTlv;
    };

};

#endif
