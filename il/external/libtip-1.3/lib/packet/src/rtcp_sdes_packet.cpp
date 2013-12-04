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

#include "rtcp_sdes_packet.h"
using namespace LibTip;

CRtcpSDESPacket::CRtcpSDESPacket() {
    SetType(SDES);
    SetSubType(0); // subtype is the SDES SC field
}

CRtcpSDESPacket::~CRtcpSDESPacket() {

}

void CRtcpSDESPacket::AddChunk(uint32_t ssrc)
{
    EmptyChunk chunk = { ssrc, 0 };
    mChunk.push_back(chunk);
    IncrSize(sizeof(chunk));
    SetSubType(mChunk.size()); // set SDES SC field
}

uint32_t CRtcpSDESPacket::GetChunkSSRC(uint32_t index) const 
{
    if (mChunk.size() <= index) {
        return 0;
    }

    return mChunk[index].mSSRC;
}

uint32_t CRtcpSDESPacket::PackData(CPacketBuffer& buffer) const
{
    CRtcpPacket::PackData(buffer);

    // add all chunks
    EmptyChunkVector::const_iterator i;
    for (i = mChunk.begin(); i != mChunk.end(); ++i) {
        buffer.Add((*i).mSSRC);
        buffer.Add((*i).mPad);
    }

	return buffer.GetBufferSize();
}

int CRtcpSDESPacket::UnpackData(CPacketBuffer& buffer)
{
    int ret = CRtcpPacket::UnpackData(buffer);
    if (ret == 0) {
        if (GetType() != SDES) {
            return -1;
        }

        // if we are supposed to have a sub-type and the length in the
        // RTCP header indicated enough room for one then remove a
        // single SSRC from the packet.  this is most definitely NOT
        // proper SDES handling but its good enough to parse (and
        // ignore) most any SDES which is all we are handling for now.
        if (GetSubType() >= 1 && mRtcpHeader.length > 0) {
            uint32_t ssrc;
            buffer.Rem(ssrc);
            AddChunk(ssrc);

            // we do not unpack any data, just let the base class
            // remove anything that is left.
        }
    }
    
    return ret;
}

void CRtcpSDESPacket::ClearData()
{
    DecrSize(mChunk.size() * sizeof(EmptyChunk));
    mChunk.clear();
}
