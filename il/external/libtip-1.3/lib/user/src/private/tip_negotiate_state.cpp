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

#include "tip_debug_print.h"
#include "private/tip_impl.h"
#include "private/tip_negotiate_state.h"
using namespace LibTip;

// global objs
CStopLocalState      LibTip::gStopLocalState;
CMCTxLocalState      LibTip::gMCTxLocalState;
CMOTxLocalState      LibTip::gMOTxLocalState;
CTimeoutLocalState   LibTip::gTimeoutLocalState;
CDoneLocalState      LibTip::gDoneLocalState;

CStopRemoteState     LibTip::gStopRemoteState;
CNoRxRemoteState     LibTip::gNoRxRemoteState;
CMCRxRemoteState     LibTip::gMCRxRemoteState;
CTimeoutRemoteState  LibTip::gTimeoutRemoteState;
CDoneRemoteState     LibTip::gDoneRemoteState;

CLocalState::CLocalState()
{

}

CLocalState::~CLocalState()
{

}
        
Status CLocalState::Start(CTipImpl* am, MediaType mType)
{
    return TIP_ERROR;
}

void CLocalState::RxMCAck(CTipImpl* am, CRtcpTipPacket& ack, MediaType mType)
{

}

void CLocalState::RxMOAck(CTipImpl* am, MediaType mType)
{

}

void CLocalState::Timeout(CTipImpl* am, MediaType mType)
{

}

void CLocalState::Stop(CTipImpl* am, MediaType mType)
{

}

void CLocalState::Restart(CTipImpl* am, MediaType mType)
{

}

CStopLocalState::CStopLocalState()
{

}

CStopLocalState::~CStopLocalState()
{

}

Status CStopLocalState::Start(CTipImpl* am, MediaType mType)
{
    return am->OnLocalStart(mType);
}

const char* const CStopLocalState::GetName() const
{
    return "STOPPED";
}

CMCTxLocalState::CMCTxLocalState()
{

}

CMCTxLocalState::~CMCTxLocalState()
{

}

void CMCTxLocalState::RxMCAck(CTipImpl* am, CRtcpTipPacket& ack, MediaType mType)
{
    am->OnRxMCAck(mType);
}

void CMCTxLocalState::Timeout(CTipImpl* am, MediaType mType)
{
    am->OnLocalTimeout(mType);
}

void CMCTxLocalState::Stop(CTipImpl* am, MediaType mType)
{
    am->OnLocalStop(mType);
}

void CMCTxLocalState::Restart(CTipImpl* am, MediaType mType)
{
    am->OnLocalStop(mType);
    am->OnLocalStart(mType);
}

const char* const CMCTxLocalState::GetName() const
{
    return "MCTX";
}

CMOTxLocalState::CMOTxLocalState()
{

}

CMOTxLocalState::~CMOTxLocalState()
{

}

void CMOTxLocalState::RxMOAck(CTipImpl* am, MediaType mType)
{
    am->OnRxMOAck(mType);
}

void CMOTxLocalState::Timeout(CTipImpl* am, MediaType mType)
{
    am->OnLocalTimeout(mType);
}

void CMOTxLocalState::Stop(CTipImpl* am, MediaType mType)
{
    am->OnLocalStop(mType);
}

void CMOTxLocalState::Restart(CTipImpl* am, MediaType mType)
{
    am->OnLocalStop(mType);
    am->OnLocalStart(mType);
}

const char* const CMOTxLocalState::GetName() const
{
    return "MOTX";
}

CTimeoutLocalState::CTimeoutLocalState()
{

}

CTimeoutLocalState::~CTimeoutLocalState()
{
    
}

void CTimeoutLocalState::Stop(CTipImpl* am, MediaType mType)
{
    am->OnLocalStop(mType);
}

void CTimeoutLocalState::Restart(CTipImpl* am, MediaType mType)
{
    am->OnLocalStop(mType);
    am->OnLocalStart(mType);
}

const char* const CTimeoutLocalState::GetName() const
{
    return "TIMEOUT";
}

CDoneLocalState::CDoneLocalState()
{

}

CDoneLocalState::~CDoneLocalState()
{

}

void CDoneLocalState::RxMCAck(CTipImpl* am, CRtcpTipPacket& ack, MediaType mType)
{
    am->OnRxMCAckAfterNegotiate(ack, mType);
}

void CDoneLocalState::Timeout(CTipImpl* am, MediaType mType)
{
    am->OnLocalTimeoutAfterNegotiate(mType);
}

void CDoneLocalState::Stop(CTipImpl* am, MediaType mType)
{
    am->OnLocalStop(mType);
}

void CDoneLocalState::Restart(CTipImpl* am, MediaType mType)
{
    am->OnLocalStop(mType);
    am->OnLocalStart(mType);
}

const char* const CDoneLocalState::GetName() const
{
    return "DONE";
}

CRemoteState::CRemoteState()
{

}

CRemoteState::~CRemoteState()
{

}

Status CRemoteState::Start(CTipImpl* am, MediaType mType)
{
    return TIP_ERROR;
}

void CRemoteState::RxMC(CTipImpl* am, CRtcpAppMuxCtrlPacketBase& muxctrl,
                        MediaType mType)
{

}

void CRemoteState::RxMO(CTipImpl* am, CRtcpAppMediaoptsPacket& mo, MediaType mType)
{

}

void CRemoteState::Timeout(CTipImpl* am, MediaType mType)
{

}

void CRemoteState::Stop(CTipImpl* am, MediaType mType)
{

}

void CRemoteState::Restart(CTipImpl* am, MediaType mType)
{

}

void CRemoteState::RxReqToSend(CTipImpl* am, CRtcpAppReqToSendPacket& rts, MediaType mType)
{
    AMDEBUG(RECV, ("receive REQTOSEND in %s state, ignored", GetName()));
}

void CRemoteState::RxSpiMap(CTipImpl* am, CRtcpAppSpiMapPacket& spimap, MediaType mType)
{
    AMDEBUG(RECV, ("receive SPIMAP in %s state, ignored", GetName()));    
}

void CRemoteState::RxNotify(CTipImpl* am, CRtcpAppNotifyPacket& notify, MediaType mType)
{
    AMDEBUG(RECV, ("receive NOTIFY in %s state, ignored", GetName()));    
}

CStopRemoteState::CStopRemoteState()
{

}

CStopRemoteState::~CStopRemoteState()
{

}

Status CStopRemoteState::Start(CTipImpl* am, MediaType mType)
{
    am->OnRemoteStart(mType);
    return TIP_OK;
}

void CStopRemoteState::RxMC(CTipImpl* am, CRtcpAppMuxCtrlPacketBase& muxctrl,
                            MediaType mType)
{
    am->OnRxMCBeforeNegotiate(muxctrl, mType);
}

const char* const CStopRemoteState::GetName() const
{
    return "STOPPED";
}

CNoRxRemoteState::CNoRxRemoteState()
{

}

CNoRxRemoteState::~CNoRxRemoteState()
{

}

void CNoRxRemoteState::RxMC(CTipImpl* am, CRtcpAppMuxCtrlPacketBase& muxctrl,
                            MediaType mType)
{
    am->OnRxMCDuringNegotiate(muxctrl, mType);
}

void CNoRxRemoteState::Timeout(CTipImpl* am, MediaType mType)
{
    am->OnRemoteTimeout(mType);
}

void CNoRxRemoteState::Stop(CTipImpl* am, MediaType mType)
{
    am->OnRemoteStop(mType);
}

void CNoRxRemoteState::Restart(CTipImpl* am, MediaType mType)
{
    am->OnRemoteStop(mType);
    am->OnRemoteStart(mType);
}

const char* const CNoRxRemoteState::GetName() const
{
    return "NORX";
}

CMCRxRemoteState::CMCRxRemoteState()
{

}

CMCRxRemoteState::~CMCRxRemoteState()
{

}

void CMCRxRemoteState::RxMO(CTipImpl* am, CRtcpAppMediaoptsPacket& mo, MediaType mType)
{
    am->OnRxMODuringNegotiate(mo, mType);
}

void CMCRxRemoteState::Timeout(CTipImpl* am, MediaType mType)
{
    am->OnRemoteTimeout(mType);
}

void CMCRxRemoteState::Stop(CTipImpl* am, MediaType mType)
{
    am->OnRemoteStop(mType);
}

void CMCRxRemoteState::Restart(CTipImpl* am, MediaType mType)
{
    am->OnRemoteStop(mType);
    am->OnRemoteStart(mType);
}

const char* const CMCRxRemoteState::GetName() const
{
    return "MCRX";
}

CTimeoutRemoteState::CTimeoutRemoteState()
{

}

CTimeoutRemoteState::~CTimeoutRemoteState()
{

}

void CTimeoutRemoteState::Stop(CTipImpl* am, MediaType mType)
{
    am->OnRemoteStop(mType);
}

void CTimeoutRemoteState::Restart(CTipImpl* am, MediaType mType)
{
    am->OnRemoteStop(mType);
    am->OnRemoteStart(mType);
}

const char* const CTimeoutRemoteState::GetName() const
{
    return "TIMEOUT";
}

CDoneRemoteState::CDoneRemoteState()
{

}

CDoneRemoteState::~CDoneRemoteState()
{

}

void CDoneRemoteState::RxMC(CTipImpl* am, CRtcpAppMuxCtrlPacketBase& muxctrl, MediaType mType)
{
    am->OnRxMCAfterNegotiate(muxctrl, mType);
}

void CDoneRemoteState::RxMO(CTipImpl* am, CRtcpAppMediaoptsPacket& mo, MediaType mType)
{
    am->OnRxMOAfterNegotiate(mo, mType);
}

void CDoneRemoteState::Stop(CTipImpl* am, MediaType mType)
{
    am->OnRemoteStop(mType);
}

void CDoneRemoteState::Restart(CTipImpl* am, MediaType mType)
{
    am->OnRemoteStop(mType);
    am->OnRemoteStart(mType);
}

void CDoneRemoteState::RxReqToSend(CTipImpl* am, CRtcpAppReqToSendPacket& rts,
                                   MediaType mType)
{
    am->OnRxReqToSend(rts, mType);
}

void CDoneRemoteState::RxSpiMap(CTipImpl* am, CRtcpAppSpiMapPacket& spimap, MediaType mType)
{
    am->OnRxSpiMap(spimap, mType);
}

void CDoneRemoteState::RxNotify(CTipImpl* am, CRtcpAppNotifyPacket& notify, MediaType mType)
{
    am->OnRxNotify(notify, mType);
}

const char* const CDoneRemoteState::GetName() const
{
    return "DONE";
}
