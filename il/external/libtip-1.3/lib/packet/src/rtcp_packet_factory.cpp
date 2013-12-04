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

#include "tip_time.h"
#include "rtcp_packet_factory.h"
#include "rtcp_rr_packet.h"
#include "rtcp_sdes_packet.h"
#include "rtcp_tip_ack_packet.h"
#include "rtcp_tip_echo_packet.h"
#include "rtcp_tip_feedback_packet.h"
#include "rtcp_tip_flowctrl_packet.h"
#include "rtcp_tip_mediaopts_packet.h"
#include "rtcp_tip_muxctrl_packet.h"
#include "rtcp_tip_refresh_packet.h"
#include "rtcp_tip_reqtosend_packet.h"
#include "rtcp_tip_spimap_packet.h"
#include "rtcp_tip_notify_packet.h"
using namespace LibTip;

// template mapping function, can be used for constructing any simple
// (e.g. not versioned) TIP packet.
template< class T > static CRtcpTipPacket* mapTipPacket(CPacketBuffer& buffer);

// predeclare mapping funcs
static CRtcpTipPacket* mapMuxCtrl(CPacketBuffer& buffer);
static CRtcpTipPacket* mapAck(CPacketBuffer& buffer);

typedef CRtcpTipPacket* (*MapFunc)(CPacketBuffer& buffer);
static MapFunc gTipMapFunc[] = {
    NULL, // RESERVED0
    mapMuxCtrl, // MUXCTRL
    NULL, // UNUSED
    mapTipPacket<CRtcpAppNotifyPacket>, // NOTIFY
    mapTipPacket<CRtcpAppEchoPacket>, // ECHO
    mapTipPacket<CRtcpAppTXFlowCtrlPacket>, // TXFLOWCTRL
    mapTipPacket<CRtcpAppRXFlowCtrlPacket>, // RXFLOWCTRL
    mapTipPacket<CRtcpAppMediaoptsPacket>, // MEDIAOPTS
    mapTipPacket<CRtcpAppRefreshPacket>, // REFRESH
    NULL, // UNUSED
    NULL, // UNUSED
    NULL, // UNUSED
    mapTipPacket<CRtcpAppSpiMapPacket>, // SPIMAP
    NULL, // UNUSED
    NULL, // UNUSED
    mapTipPacket<CRtcpAppReqToSendPacket>, // REQTOSEND
    NULL, // ACK_UNUSED
    mapAck, // ACK_MUXCTRL
    NULL, // ACK_UNUSED
    mapAck, // ACK_NOTIFY
    NULL, // ACK_ECHO
    mapAck, // ACK_TXFLOWCTRL
    mapAck, // ACK_RXFLOWCTRL
    mapAck, // ACK_MEDIAOPTS
    mapAck, // ACK_REFRESH
    NULL, // ACK_UNUSED
    NULL, // ACK_UNUSED
    NULL, // ACK_UNUSED
    mapAck, // ACK_SPIMAP
    NULL, // ACK_UNUSED
    NULL, // ACK_UNUSED
    mapTipPacket<CRtcpAppReqToSendAckPacket> // ACK_REQTOSEND
};

CRtcpPacket* CRtcpPacketFactory::CreatePacketFromBuffer(CPacketBuffer& buffer)
{
    // the offset into the buffer that we started at
    uint32_t sOffset = buffer.GetBufferOffset();

    // the length (in bytes) of this part of the packet
    uint32_t length = 0;

    // unpack as a minimum rtcp packet to figure out what we are
    // dealing with.
    CRtcpPacket rtcpPacket;
    if (rtcpPacket.Unpack(buffer) != 0) {
        // set buffer to empty as we cannot do anything more with it.
        buffer.RemAll();
        return NULL;
    }

    // reset back to the beginning to unpack as the correct type
    buffer.ResetHead(sOffset);

    // create a temp buffer with just the data from this RTCP packet.
    // this prevents an RTCP Unpack from consuming more data than it
    // should.
    length = CRtcpPacket::RtcpLengthToBytes(rtcpPacket.GetLength());
    CPacketBuffer tmp(buffer.GetBuffer(), length);

    // move the overall buffer forward past the consumed data
    buffer.ResetHead((sOffset + length));
    
    switch (rtcpPacket.GetType()) {
    case CRtcpPacket::RR:
        return CreateRRFromBuffer(tmp);

    case CRtcpPacket::SDES:
        return CreateSDESFromBuffer(tmp);

    case CRtcpPacket::RTPFB:
        return CreateFBFromBuffer(tmp);
        
    case CRtcpPacket::APP:
        return CreateAPPFromBuffer(tmp);

    default:
        // nothing else handled yet
        return NULL;
    }
}
    
CRtcpPacket* CRtcpPacketFactory::CreateRRFromBuffer(CPacketBuffer& buffer)
{
    CRtcpRRPacket* rr = new CRtcpRRPacket();
    if (rr != NULL) {
        if (rr->Unpack(buffer) != 0) {
            delete rr;
            rr = NULL;
        }
    }
    
    return rr;
}

CRtcpPacket* CRtcpPacketFactory::CreateSDESFromBuffer(CPacketBuffer& buffer)
{
    CRtcpSDESPacket* sdes = new CRtcpSDESPacket();
    if (sdes != NULL) {
        if (sdes->Unpack(buffer) != 0) {
            delete sdes;
            sdes = NULL;
        }
    }
    
    return sdes;
}

CRtcpPacket* CRtcpPacketFactory::CreateFBFromBuffer(CPacketBuffer& buffer)
{
    // try to create extended first.  if that doesn't work, create
    // non-extended.
    CRtcpAppExtendedFeedbackPacket* ext = new CRtcpAppExtendedFeedbackPacket();
    if (ext != NULL) {
        if (ext->Unpack(buffer) != 0) {
            delete ext;
            ext = NULL;
        }
    }

    if (ext != NULL) {
        return ext;
    }
    
    CRtcpAppFeedbackPacket* fb = new CRtcpAppFeedbackPacket();
    if (fb != NULL) {
        if (fb->Unpack(buffer) != 0) {
            delete fb;
            fb = NULL;
        }
    }
    
    return fb;
}

CRtcpPacket* CRtcpPacketFactory::CreateAPPFromBuffer(CPacketBuffer& buffer)
{
    CRtcpAppPacket appPacket;
    CRtcpTipPacket* retPacket = NULL;

    // unpack just the header so we can figure out what type it is
    if (appPacket.Unpack(buffer) != 0) {
        // something wrong with the app portion of the header
        return NULL;
    }

    TipPacketType type = ConvertRtcpToTip(appPacket.GetSubType(),
                                          appPacket.GetAppName().mName);
    if (type == MAX_PACKET_TYPE) {
        // something wrong with the app name and/or type.  we
        // shouldn't get here.
        return NULL;
    }
    
    // reset buffer back to the start so we can unpack it again as a
    // full packet instead of just the header.
    buffer.ResetHead();

    if (gTipMapFunc[type] != NULL) {
        retPacket = gTipMapFunc[type](buffer);
    }
    
    return retPacket;
}

static CRtcpTipPacket* mapMuxCtrl(CPacketBuffer& buffer)
{
    CRtcpAppMuxCtrlPacketBase* packet = new CRtcpAppMuxCtrlPacket();
    if (packet == NULL) {
        return NULL;
    }

    // unpack as V6 first
    if (packet->Unpack(buffer) == 0) {
        return packet;
    }

    // if that fails, try to unpack as V7 instead.
    delete packet;
    buffer.ResetHead();

    packet = new CRtcpAppMuxCtrlV7Packet();
    if (packet == NULL) {
        return NULL;
    }
        
    if (packet->Unpack(buffer) != 0) {
        delete packet;
        return NULL;
    }
    
    return packet;
}

static CRtcpTipPacket* mapAck(CPacketBuffer& buffer)
{
    // create with temp type until we know the real type
    CRtcpTipAckPacket* packet = new CRtcpTipAckPacket(RESERVED0);
    if (packet != NULL) {
        if (packet->Unpack(buffer) != 0) {
            delete packet;
            packet = NULL;
        }
    }
    return packet;
}

template< class T > static CRtcpTipPacket* mapTipPacket(CPacketBuffer& buffer) {
    T* packet = new T();
    if (packet != NULL) {
        if (packet->Unpack(buffer) != 0) {
            delete packet;
            packet = NULL;
        }
    }
    return packet;
}

CRtcpTipPacket* CRtcpPacketFactory::CreateAckPacket(const CRtcpTipPacket& packet)
{
    TipPacketType type = packet.GetTipPacketType();
    if (type == REQTOSEND) {
        // REQTOSEND packets have a special ACK format, use that here
        const CRtcpAppReqToSendPacket* rts =
            dynamic_cast<const CRtcpAppReqToSendPacket*>(&packet);
        if (rts == NULL) {
            return NULL;
        }
        
        return new CRtcpAppReqToSendAckPacket(*rts);

    } else if (type == TIPECHO) {
        // ECHO packets have a special ACK format, use that here
        const CRtcpAppEchoPacket* echo =
            dynamic_cast<const CRtcpAppEchoPacket*>(&packet);
        if (echo == NULL) {
            return NULL;
        }

        CRtcpAppEchoPacket* ack = new CRtcpAppEchoPacket(*echo);
        if (ack == NULL) {
            return NULL;
        }

        ack->SetRcvNtpTime(GetNtpTimestamp());
        return ack;
    }

    // all other types get the generic ack
    return new CRtcpTipAckPacket(packet);
}

