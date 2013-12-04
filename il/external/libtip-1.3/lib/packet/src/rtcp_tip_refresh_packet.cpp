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

#include "rtcp_tip_refresh_packet.h"
using namespace LibTip;

CRtcpAppRefreshPacket::CRtcpAppRefreshPacket() :
    CRtcpTipPacket(REFRESH)
{
    memset(&mRefresh, 0, sizeof(mRefresh));
    IncrSize(sizeof(mRefresh));
}

CRtcpAppRefreshPacket::~CRtcpAppRefreshPacket()
{

}

uint32_t CRtcpAppRefreshPacket::PackData(CPacketBuffer& buffer) const
{
    CRtcpTipPacket::PackData(buffer);

    buffer.Add(mRefresh.mTarget);
    buffer.Add(mRefresh.mFlags);
    
    return buffer.GetBufferSize();
}

int CRtcpAppRefreshPacket::UnpackData(CPacketBuffer& buffer)
{
    int ret = CRtcpTipPacket::UnpackData(buffer);
    if (ret != 0) {
        return ret;
    }

    buffer.Rem(mRefresh.mTarget);

    // in ClearData() we set our minimum size to not include the flags
    // field.  after unpack, however, we will always have flags (from
    // the packet or default ones) so put the size back.  
    IncrSize(sizeof(mRefresh.mFlags));
    
    // flags are required by spec but some implementations of TIP V6
    // do not always include them.  in that case default the flags to
    // PREFER_GDR (which is the most generic thing in the sense that
    // the request does not require one or the other, a GDR is
    // typically better than an IDR, and if GDR is not configured then
    // this will result in an IDR anyways).
    if (buffer.GetBufferSize() >= sizeof(mRefresh.mFlags)) {
        buffer.Rem(mRefresh.mFlags);
    } else {
        mRefresh.mFlags = REFRESH_PREFER_GDR;
    }

    return ret;
}

void CRtcpAppRefreshPacket::ClearData()
{
    // some TIP V6 implementations do not include the flags field.
    // allow for that by decreasing our minimum size.
    DecrSize(sizeof(mRefresh.mFlags));
}

