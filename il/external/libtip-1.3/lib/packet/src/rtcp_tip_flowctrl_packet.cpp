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

#include "rtcp_tip_flowctrl_packet.h"
using namespace LibTip;

CRtcpAppFlowCtrlPacket::CRtcpAppFlowCtrlPacket(TipPacketType flowCtrlType) :
    CRtcpTipPacket(flowCtrlType)
{
    memset(&mFlowCtrl, 0, sizeof(mFlowCtrl));
    IncrSize(sizeof(mFlowCtrl));
}

CRtcpAppFlowCtrlPacket::~CRtcpAppFlowCtrlPacket()
{

}

uint32_t CRtcpAppFlowCtrlPacket::PackData(CPacketBuffer& buffer) const
{
    CRtcpTipPacket::PackData(buffer);

    buffer.Add(mFlowCtrl.mOpcode);
    buffer.Add(mFlowCtrl.mTarget);
    return buffer.GetBufferSize();
}

int CRtcpAppFlowCtrlPacket::UnpackData(CPacketBuffer& buffer)
{
    int ret = CRtcpTipPacket::UnpackData(buffer);
    if (ret != 0) {
        return ret;
    }

    buffer.Rem(mFlowCtrl.mOpcode);
    buffer.Rem(mFlowCtrl.mTarget);
    return ret;
}

void CRtcpAppFlowCtrlPacket::ToStream(std::ostream& o, MediaType mType) const
{
    CRtcpTipPacket::ToStream(o, mType);

    o << "\n\tOPCODE:   " << OpcodeToString(mFlowCtrl.mOpcode)
      << "\n\tTARGET:   " << std::hex << std::showbase << mFlowCtrl.mTarget;
}

const char* CRtcpAppFlowCtrlPacket::OpcodeToString(uint16_t opcode) const
{
    switch (opcode) {
    case OPCODE_START:
        return "START";

    case OPCODE_STOP:
        return "STOP";

    default:
        return "UNKNOWN";
    }
}

CRtcpAppTXFlowCtrlPacket::CRtcpAppTXFlowCtrlPacket() :
    CRtcpAppFlowCtrlPacket(TXFLOWCTRL)
{

}

CRtcpAppTXFlowCtrlPacket::~CRtcpAppTXFlowCtrlPacket()
{

}

CRtcpAppRXFlowCtrlPacket::CRtcpAppRXFlowCtrlPacket() :
    CRtcpAppFlowCtrlPacket(RXFLOWCTRL)
{

}

CRtcpAppRXFlowCtrlPacket::~CRtcpAppRXFlowCtrlPacket()
{

}
