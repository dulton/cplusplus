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

#include "tip_relay.h"
#include "tip_constants.h"
#include "tip_csrc.h"
#include "rtcp_packet.h"
#include "rtcp_packet_factory.h"
#include "rtcp_tip_flowctrl_packet.h"
#include "rtcp_tip_refresh_packet.h"
#include "rtcp_tip_feedback_packet.h"

using namespace LibTip;

CTipRelay::CTipRelay() {}
CTipRelay::~CTipRelay() {}

CTipRelay::Classification
CTipRelay::Classify(uint8_t* buffer, uint32_t size, uint16_t& pos)
{
    CTipRelay::Classification ret = NOT_TIP;
    
    // create an RTCP tip packet from the given buffer
    CPacketBuffer packet_buffer(buffer, size);
    
    // tip packets may be contained in compound rtcp packets.  find
    // the part of the packet we care about and forward based on that.
    CRtcpPacket* rtcp;
    
    while (ret == NOT_TIP && packet_buffer.GetBufferSize() > 0) {
        rtcp = CRtcpPacketFactory::CreatePacketFromBuffer(packet_buffer);

        // if we can't create a packet then we don't need to process it
        if (rtcp == NULL) {
            // something we couldn't parse, keep going until the
            // buffer is empty
            continue;
        }

        if (rtcp->GetType() == CRtcpPacket::APP) {
            ret = ClassifyAPP(rtcp, pos);
        } else if (rtcp->GetType() == CRtcpPacket::RTPFB) {
            ret = ClassifyFB(rtcp, pos);
        }

        delete rtcp;
    }
    
    return ret;
}

CTipRelay::Classification
CTipRelay::ClassifyAPP(CRtcpPacket* rtcp, uint16_t& pos)
{
    CRtcpTipPacket* packet = dynamic_cast<CRtcpTipPacket*>(rtcp);
    if (packet == NULL) {
        return NOT_TIP;
    }
    
    // init return positions to 0
    pos = 0;
    
    // some kind of tip packet, classify based on type
    switch (packet->GetTipPacketType()) {
    case MUXCTRL:
        return SYSTEM;

    case NOTIFY:
        return SYSTEM;

    case TIPECHO:
        return SYSTEM;

    case TXFLOWCTRL:
        GetPositionForPacket(packet, pos);
        return MEDIA_SOURCE;

    case RXFLOWCTRL:
        GetPositionForPacket(packet, pos);
        return MEDIA_SINK;

    case MEDIAOPTS:
        return SYSTEM;

    case REFRESH:
        GetPositionForPacket(packet, pos);
        return MEDIA_SOURCE;

    case SPIMAP:
        return SYSTEM;

    case REQTOSEND:
        return SYSTEM;

    case ACK_MUXCTRL:
        return SYSTEM;

    case ACK_NOTIFY:
        return SYSTEM;

    case ACK_TIPECHO:
        return SYSTEM;
        
    case ACK_TXFLOWCTRL:
        pos = 0xFFFF;
        return MEDIA_SINK;

    case ACK_RXFLOWCTRL:
        pos = 0xFFFF;
        return MEDIA_SOURCE;

    case ACK_MEDIAOPTS:
        return SYSTEM;

    case ACK_REFRESH:
        pos = 0xFFFF;
        return MEDIA_SINK;

    case ACK_SPIMAP:
        return SYSTEM;

    case ACK_REQTOSEND:
        return SYSTEM;

    default:
        return NOT_TIP;
    }
}

CTipRelay::Classification
CTipRelay::ClassifyFB(CRtcpPacket* rtcp, uint16_t& pos)
{
    CTipCSRC csrc(0);

    CRtcpAppFeedbackPacket* fb = dynamic_cast<CRtcpAppFeedbackPacket*>(rtcp);
    if (fb != NULL) {
        csrc.SetCSRC(fb->GetTarget());
    }
    
    pos = (1 << csrc.GetSourcePos());
    return MEDIA_SOURCE;
}

// CSRC is in different places in different packets, so pull the CSRC
// and the affected position out based on packet type.
void CTipRelay::GetPositionForPacket(CRtcpTipPacket* packet, uint16_t& pos)
{
    CTipCSRC csrc(0);
    
    switch (packet->GetTipPacketType()) {
      case TXFLOWCTRL: {
          CRtcpAppTXFlowCtrlPacket* tx = dynamic_cast<CRtcpAppTXFlowCtrlPacket*>(packet);
          if (tx != NULL) {
              csrc.SetCSRC(tx->GetTarget());
          }
          break;
      }
        
      case RXFLOWCTRL: {
          CRtcpAppRXFlowCtrlPacket* rx = dynamic_cast<CRtcpAppRXFlowCtrlPacket*>(packet);
          if (rx != NULL) {
              csrc.SetCSRC(rx->GetTarget());
          }
          break;
      }
        
      case REFRESH: {
          CRtcpAppRefreshPacket* ref = dynamic_cast<CRtcpAppRefreshPacket*>(packet);
          if (ref != NULL) {
              csrc.SetCSRC(ref->GetTarget());
          }
          break;
      }

    default:
        break;
    }

    pos = (1 << csrc.GetSourcePos());
}
