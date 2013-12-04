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

#include "rtcp_rr_packet.h"
using namespace LibTip;

CRtcpRRPacket::CRtcpRRPacket() {
    SetType(RR);
    SetSubType(0); // subtype is the RR RC field
}

CRtcpRRPacket::~CRtcpRRPacket() {

}

int CRtcpRRPacket::UnpackData(CPacketBuffer& buffer)
{
    int ret = CRtcpPacketSSRC::UnpackData(buffer);
    if (ret == 0) {
        if (GetType() != RR) {
            return -1;
        }
    }
    
    return ret;
}
