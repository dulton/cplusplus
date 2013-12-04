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

#ifndef TIP_NEG_STATE_H
#define TIP_NEG_STATE_H

#include "tip_constants.h"
#include "rtcp_packet.h"
#include "rtcp_tip_muxctrl_packet.h"
#include "rtcp_tip_mediaopts_packet.h"
#include "rtcp_tip_spimap_packet.h"
#include "rtcp_tip_notify_packet.h"
#include "rtcp_tip_reqtosend_packet.h"

namespace LibTip {

    // the following classes implement a state pattern based state
    // machine for controlling tip negotiation.

    // pre-declare classes
    class CTipImpl;
    
    // abstract base class for the local state half of tip negotiation
    class CLocalState {
    protected:
        CLocalState();
        virtual ~CLocalState() = 0;
        
    public:
        // event methods
        virtual Status Start(CTipImpl* am, MediaType mType);
        virtual void RxMCAck(CTipImpl* am, CRtcpTipPacket& ack, MediaType mType);
        virtual void RxMOAck(CTipImpl* am, MediaType mType);
        virtual void Timeout(CTipImpl* am, MediaType mType);
        virtual void Stop(CTipImpl* am, MediaType mType);
        virtual void Restart(CTipImpl* am, MediaType mType);

        // debug methods
        virtual const char* const GetName() const = 0;

    protected:
        CTipImpl* mpTip;
        MediaType    mType;
    };

    class CStopLocalState : public CLocalState {
    public:
        CStopLocalState();
        virtual ~CStopLocalState();

        virtual Status Start(CTipImpl* am, MediaType mType);
        virtual const char* const GetName() const;
    };

    class CMCTxLocalState : public CLocalState {
    public:
        CMCTxLocalState();
        virtual ~CMCTxLocalState();

        virtual void RxMCAck(CTipImpl* am, CRtcpTipPacket& ack, MediaType mType);
        virtual void Timeout(CTipImpl* am, MediaType mType);
        virtual void Stop(CTipImpl* am, MediaType mType);
        virtual void Restart(CTipImpl* am, MediaType mType);
        virtual const char* const GetName() const;
    };

    class CMOTxLocalState : public CLocalState {
    public:
        CMOTxLocalState();
        virtual ~CMOTxLocalState();

        virtual void RxMOAck(CTipImpl* am, MediaType mType);
        virtual void Timeout(CTipImpl* am, MediaType mType);
        virtual void Stop(CTipImpl* am, MediaType mType);
        virtual void Restart(CTipImpl* am, MediaType mType);
        virtual const char* const GetName() const;
    };

    class CTimeoutLocalState : public CLocalState {
    public:
        CTimeoutLocalState();
        virtual ~CTimeoutLocalState();

        virtual void Stop(CTipImpl* am, MediaType mType);
        virtual void Restart(CTipImpl* am, MediaType mType);
        virtual const char* const GetName() const;
    };

    class CDoneLocalState : public CLocalState {
    public:
        CDoneLocalState();
        virtual ~CDoneLocalState();

        virtual void RxMCAck(CTipImpl* am, CRtcpTipPacket& ack, MediaType mType);
        virtual void Timeout(CTipImpl* am, MediaType mType);
        virtual void Stop(CTipImpl* am, MediaType mType);
        virtual void Restart(CTipImpl* am, MediaType mType);
        virtual const char* const GetName() const;
    };

    // abstract base class for the remote state half of tip negotiation
    class CRemoteState {
    protected:
        CRemoteState();
        virtual ~CRemoteState() = 0;

    public:
        // event methods
        virtual Status Start(CTipImpl* am, MediaType mType);
        virtual void RxMC(CTipImpl* am, CRtcpAppMuxCtrlPacketBase& muxctrl, MediaType mType);
        virtual void RxMO(CTipImpl* am, CRtcpAppMediaoptsPacket& mo, MediaType mType);
        virtual void Timeout(CTipImpl* am, MediaType mType);
        virtual void Stop(CTipImpl* am, MediaType mType);
        virtual void Restart(CTipImpl* am, MediaType mType);
        virtual void RxReqToSend(CTipImpl* am, CRtcpAppReqToSendPacket& rts, MediaType mType);
        virtual void RxSpiMap(CTipImpl* am, CRtcpAppSpiMapPacket& spimap, MediaType mType);
        virtual void RxNotify(CTipImpl* am, CRtcpAppNotifyPacket& notify, MediaType mType);

        // debug methods
        virtual const char* const GetName() const = 0;

    protected:
        CTipImpl* mpTip;
        MediaType    mType;
    };

    class CStopRemoteState : public CRemoteState {
    public:
        CStopRemoteState();
        virtual ~CStopRemoteState();

        virtual Status Start(CTipImpl* am, MediaType mType);
        virtual void RxMC(CTipImpl* am, CRtcpAppMuxCtrlPacketBase& muxctrl, MediaType mType);
        virtual const char* const GetName() const;
    };

    class CNoRxRemoteState : public CRemoteState {
    public:
        CNoRxRemoteState();
        virtual ~CNoRxRemoteState();

        virtual void RxMC(CTipImpl* am, CRtcpAppMuxCtrlPacketBase& mPacket, MediaType mType);
        virtual void Timeout(CTipImpl* am, MediaType mType);
        virtual void Stop(CTipImpl* am, MediaType mType);
        virtual void Restart(CTipImpl* am, MediaType mType);
        virtual const char* const GetName() const;
    };

    class CMCRxRemoteState : public CRemoteState {
    public:
        CMCRxRemoteState();
        virtual ~CMCRxRemoteState();

        virtual void RxMO(CTipImpl* am, CRtcpAppMediaoptsPacket& mo, MediaType mType);
        virtual void Timeout(CTipImpl* am, MediaType mType);
        virtual void Stop(CTipImpl* am, MediaType mType);
        virtual void Restart(CTipImpl* am, MediaType mType);
        virtual const char* const GetName() const;
    };

    class CTimeoutRemoteState : public CRemoteState {
    public:
        CTimeoutRemoteState();
        virtual ~CTimeoutRemoteState();

        virtual void Stop(CTipImpl* am, MediaType mType);
        virtual void Restart(CTipImpl* am, MediaType mType);
        virtual const char* const GetName() const;
    };

    class CDoneRemoteState : public CRemoteState {
    public:
        CDoneRemoteState();
        virtual ~CDoneRemoteState();

        virtual void RxMC(CTipImpl* am, CRtcpAppMuxCtrlPacketBase& muxctrl, MediaType mType);
        virtual void RxMO(CTipImpl* am, CRtcpAppMediaoptsPacket& mo, MediaType mType);
        virtual void Stop(CTipImpl* am, MediaType mType);
        virtual void Restart(CTipImpl* am, MediaType mType);
        virtual void RxReqToSend(CTipImpl* am, CRtcpAppReqToSendPacket& rts, MediaType mType);
        virtual void RxSpiMap(CTipImpl* am, CRtcpAppSpiMapPacket& spimap, MediaType mType);
        virtual void RxNotify(CTipImpl* am, CRtcpAppNotifyPacket& notify, MediaType mType);
        virtual const char* const GetName() const;
    };

    // global instances of each state class.  as each class has no
    // internal data we can just use globals instead of multiple
    // copies.
    extern CStopLocalState      gStopLocalState;
    extern CMCTxLocalState      gMCTxLocalState;
    extern CMOTxLocalState      gMOTxLocalState;
    extern CTimeoutLocalState   gTimeoutLocalState;
    extern CDoneLocalState      gDoneLocalState;
    
    extern CStopRemoteState     gStopRemoteState;
    extern CNoRxRemoteState     gNoRxRemoteState;
    extern CMCRxRemoteState     gMCRxRemoteState;
    extern CTimeoutRemoteState  gTimeoutRemoteState;
    extern CDoneRemoteState     gDoneRemoteState;
};

#endif

