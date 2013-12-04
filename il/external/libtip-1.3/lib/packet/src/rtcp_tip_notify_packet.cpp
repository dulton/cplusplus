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

#include <iostream>
#include <iomanip>

#include "rtcp_tip_notify_packet.h"
using namespace LibTip;

CRtcpAppNotifyPacket::CRtcpAppNotifyPacket() :
    CRtcpTipPacket(NOTIFY)
{

}

CRtcpAppNotifyPacket::~CRtcpAppNotifyPacket()
{
    while (! mTlv.empty()) {
        CTipTlv* ptlv = mTlv.front();
        mTlv.pop_front();
        delete ptlv;
    }
}

int CRtcpAppNotifyPacket::AddTLV(NotifyTag tag, uint8_t* data, uint32_t len)
{
    CTipTlv* tlv = new CTipTlv();
    if (tlv == NULL) {
        return -1;
    }
    
    tlv->SetTag(tag);
    tlv->SetData(data, len);

    mTlv.push_back(tlv);
    IncrSize(tlv->GetPackSize());
    
    return 0;
}

int CRtcpAppNotifyPacket::RemTLV(NotifyTag tag)
{
    CTlvpList::iterator iter;
    for (iter = mTlv.begin(); iter != mTlv.end(); ++iter) {
        if ((*iter)->GetTag() == tag) {
            CTipTlv* tlv = *iter;
            mTlv.erase(iter);

            DecrSize(tlv->GetPackSize());
            
            delete tlv;
            return 0;
        }
    }

    return -1;
}
    
int CRtcpAppNotifyPacket::GetTLVByTag(NotifyTag tag, uint8_t* data) const
{
    CTlvpList::const_iterator iter;
    for (iter = mTlv.begin(); iter != mTlv.end(); ++iter) {
        if ((*iter)->GetTag() == tag) {

            if (data != NULL) {
                memcpy(data, (*iter)->GetData(), (*iter)->GetLength());
            }
            return (*iter)->GetLength();
        }
    }

    return -1;
}

int CRtcpAppNotifyPacket::GetTLVByIndex(uint32_t index, uint8_t* data) const
{
    CTlvpList::const_iterator iter = mTlv.begin();
    std::advance(iter, index);

    if (iter == mTlv.end()) {
        return -1;
    }

    if (data != NULL) {
        memcpy(data, (*iter)->GetData(), (*iter)->GetLength());
    }
    return (*iter)->GetLength();
}

uint32_t CRtcpAppNotifyPacket::PackData(CPacketBuffer& buffer) const
{
    CRtcpTipPacket::PackData(buffer);
    
    CTlvpList::const_iterator iter;
    for (iter = mTlv.begin(); iter != mTlv.end(); ++iter) {
        (*iter)->Pack(buffer);
    }

    return buffer.GetBufferSize();
}

int CRtcpAppNotifyPacket::UnpackData(CPacketBuffer& buffer)
{
    int ret = CRtcpTipPacket::UnpackData(buffer);
    if (ret != 0) {
        return ret;
    }

    while (buffer.GetBufferSize() >= CTipTlv::MIN_TLV_PACK_LEN) {
        CTipTlv* tlv = new CTipTlv();
        if (tlv == NULL) {
            // oh well, we tried
            ret = -1;
            break;
        }

        // if the tlv cannot unpacket properly then the packet is malformed
        if (tlv->Unpack(buffer) != 0) {
            ret = -1;
            delete tlv;
            break;
        }

        mTlv.push_back(tlv);
        IncrSize(tlv->GetPackSize());
    }

    return ret;
}

void CRtcpAppNotifyPacket::ClearData()
{
    while (! mTlv.empty()) {
        CTipTlv* ptlv = mTlv.front();
        mTlv.pop_front();

        DecrSize(ptlv->GetPackSize());
        delete ptlv;
    }
}

void CRtcpAppNotifyPacket::ToStream(std::ostream& o, MediaType mType) const
{
    CRtcpTipPacket::ToStream(o, mType);

    uint32_t count = 1;
    CTlvpList::const_iterator iter;
    for (iter = mTlv.begin(); iter != mTlv.end(); ++iter) {
        o << "\n\tTLV #" << std::dec << count++
          << "\n\t  TYPE:   " << static_cast<int>((*iter)->GetTag())
          << "\n\t  LENGTH: " << static_cast<int>((*iter)->GetLength())
          << "\n\t  DATA:   ";

        const uint8_t* data = (*iter)->GetData();
        for (uint32_t i = 0; i < (*iter)->GetLength(); i++) {
            o << std::hex << std::setfill('0') << std::setw(2)
              << static_cast<int>(data[i]) << ' ';
        }
    }
}

    
