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
#include "tip_time.h"
#include "rtcp_tip_types.h"
#include "rtcp_packet_factory.h"
#include "rtcp_tip_flowctrl_packet.h"
#include "rtcp_tip_refresh_packet.h"
#include "rtcp_tip_feedback_packet.h"
#include "tip_media.h"
using namespace LibTip;

CTipMedia::CTipMedia(MediaType type, uint32_t ssrc, uint32_t csrc,
                     CTipPacketTransmit& xmit, const char* string) :
    mMediaType(type), mSSRC(ssrc), mCSRC(csrc), mPacketXmit(xmit)
{
    mPacketManager.EnableWrapper(mSSRC);

    std::stringstream stream;
    stream << GetMediaString(type) << ' ' << string << " 0x"
           << std::hex << mCSRC.GetCSRC();

    mLogPrefix = stream.str();
}

CTipMedia::~CTipMedia()
{

}

Status CTipMedia::ReceivePacket(uint8_t* buffer, uint32_t size)
{
    Status ret = TIP_ERROR;
    
    if (buffer == NULL) {
        return ret;
    }

    CPacketBuffer packetBuf(buffer, size);

    while (packetBuf.GetBufferSize()) {
        CRtcpPacket* rtcp = CRtcpPacketFactory::CreatePacketFromBuffer(packetBuf);

        if (rtcp == NULL) {
            // something we couldn't parse, keep going until the
            // buffer is empty
            continue;
        }

        // if we make it to here then we have at least one TIP packet
        // to process.
        ret = TIP_OK;
        
        // handle feedback packets
        if (rtcp->GetType() == CRtcpPacket::RTPFB) {
            CRtcpAppFeedbackPacket* fb = dynamic_cast<CRtcpAppFeedbackPacket*>(rtcp);

            if (fb == NULL) {
                delete rtcp;
                continue;
            }

            ProcessFBPacket(fb);
            delete rtcp;
            continue;
        }
        
        // only APP packets below here
        CRtcpTipPacket* packet = dynamic_cast<CRtcpTipPacket*>(rtcp);
        if (packet == NULL) {
            // might just be a compound packet, ignore it and try the
            // next one
            delete rtcp;
            continue;
        }
        
        AMDEBUG(RECV, ("%s recv packet type %s",
                       mLogPrefix.c_str(), packet->GetTipPacketTypeString()));
        
        // process ACK packets first
        if (IsAckTipPacketType(packet->GetTipPacketType())) {
            ProcessAckPacket(packet);
            delete packet;
            continue;
        }

        CTipPacketReceiver::Action action = mPacketReceiver.ProcessPacket(packet);
        if (action == CTipPacketReceiver::AMPR_DROP) {
            AMDEBUG(RECV, ("%s dropping old packet type %s ntp time %llu",
                           mLogPrefix.c_str(), packet->GetTipPacketTypeString(),
                           packet->GetNtpTime()));

            delete packet;
            
        } else if (action == CTipPacketReceiver::AMPR_DUP) {
            AMDEBUG(RECV, ("%s acking dup packet type %s ntp time %llu",
                           mLogPrefix.c_str(), packet->GetTipPacketTypeString(),
                           packet->GetNtpTime()));

            AckPacket(packet);
            delete packet;
            
        } else {

            // in this case the packet receiver has taken ownership of
            // the received packet.  when/if the packet is acked that
            // memory will be freed.  if it is never acked then the
            // next packet of the same time will free it or it will be
            // freed when the receiver is destroyed.
            
            ProcessPacket(packet);
        }
    }

    return ret;
}

void CTipMedia::SetRetransmissionInterval(uint32_t intervalMS)
{
    mPacketManager.SetRetransmissionInterval(intervalMS);
}

void CTipMedia::SetRetransmissionLimit(uint32_t limit)
{
    mPacketManager.SetRetransmissionLimit(limit);
}

uint64_t CTipMedia::GetIdleTime() const
{
    return mPacketManager.GetNextTransmitTime();
}

void CTipMedia::DoPeriodicActivity()
{
    if (mPacketManager.GetNextTransmitTime() == 0) {
        // time to send out some packets
        bool expired;
        CPacketBuffer* buffer;

        CRtcpTipPacket* packet = mPacketManager.GetPacket(expired, &buffer);
        while (packet != NULL) {
            if (expired) {
                // packet has timed out
                AMDEBUG(USER, ("%s timeout for packet type %s",
                               mLogPrefix.c_str(),
                               packet->GetTipPacketTypeString()));
                delete packet;
            } else {
                // packet should be sent
                AMDEBUG(XMIT, ("%s xmit packet type %s size %d bytes",
                               mLogPrefix.c_str(),
                               packet->GetTipPacketTypeString(),
                               buffer->GetBufferSize()));

                mPacketXmit.Transmit(buffer->GetBuffer(), buffer->GetBufferSize(),
                                     mMediaType);
            }
            
            packet = mPacketManager.GetPacket(expired, &buffer);
        }
    }
}

void CTipMedia::AckPacket(const CRtcpTipPacket* packet)
{
    CRtcpTipPacket* ack = CRtcpPacketFactory::CreateAckPacket(*packet);
    if (ack == NULL) {
        AMDEBUG(USER, ("%s ack failed for packet type %d",
                       mLogPrefix.c_str(), packet->GetTipPacketType()));
        return;
    }

    // setup header
    ack->SetSSRC(mSSRC);
    
    // pack ACK into a buffer and send it out
    CPacketBufferData buffer;
    mPacketManager.Pack(*ack, buffer);

    AMDEBUG(XMIT, ("%s xmit ack for packet type %s.  ack type %s size %d bytes",
                   mLogPrefix.c_str(),
                   packet->GetTipPacketTypeString(),
                   ack->GetTipPacketTypeString(), buffer.GetBufferSize()));
    

    mPacketXmit.Transmit(buffer.GetBuffer(), buffer.GetBufferSize(),
                         mMediaType);

    // notify our receiver that this packet has been acked.
    // duplicates will now be acked as well.  do not delete ack,
    // receiver now owns it.
    mPacketReceiver.RegisterAck(*packet, ack);
}

void CTipMedia::StartPacketTx(CRtcpTipPacket* packet)
{
    AMDEBUG(XMIT, ("%s starting packet tx for type %s",
                   mLogPrefix.c_str(),
                   GetTipPacketTypeString(packet->GetTipPacketType())));
    
    StopPacketTx(packet->GetTipPacketType());

    packet->SetSSRC(mSSRC);
    packet->SetNtpTime(GetNtpTimestamp());
    mPacketManager.Add(packet);
}

void CTipMedia::StopPacketTx(TipPacketType pType)
{
    CRtcpTipPacket* packet = mPacketManager.Remove(pType);
    delete packet;
}

void CTipMedia::ProcessPacket(CRtcpTipPacket* packet)
{

}

void CTipMedia::ProcessAckPacket(CRtcpTipPacket* packet)
{
    CRtcpTipPacket* acked = mPacketManager.Ack(*packet);
    if (acked == NULL) {
        // received an ACK for something we don't know anything about.  just drop it.
        return;
    }
        
    TipPacketType ackedType = acked->GetTipPacketType();
    AMDEBUG(RECV, ("%s recv ack for xmit packet type %s",
                   mLogPrefix.c_str(), GetTipPacketTypeString(ackedType)));
        
    delete acked;
}

void CTipMedia::ProcessFBPacket(CRtcpAppFeedbackPacket* packet)
{
}

void CTipMedia::PrintPacketTx(const CRtcpPacket& packet, MediaType mType) const
{
    std::ostringstream stream;
    packet.ToStream(stream, mType);
    AMDEBUG(XMIT, ("%s packet dump:%s",
                   mLogPrefix.c_str(), stream.str().c_str()));
}

void CTipMedia::PrintPacketRx(const CRtcpPacket& packet, MediaType mType) const
{
    std::ostringstream stream;
    packet.ToStream(stream, mType);
    AMDEBUG(RECV, ("%s packet dump:%s",
                   mLogPrefix.c_str(), stream.str().c_str()));
}

void CTipMedia::SetLogPrefix(const char* string)
{
    if (string == NULL) {
        mLogPrefix = "";
    } else {
        mLogPrefix = string;
    }
}

CTipMediaSink::CTipMediaSink(MediaType type, uint32_t ssrc, uint32_t csrc,
                             CTipPacketTransmit& xmit) :
    CTipMedia(type, ssrc, csrc, xmit, "SINK")
{
    mSourceCSRC = 0;
    mHaveLastSeqNum = false;
    mLastSeqNum = 0;

    mAcks.reset();
    
    mpSinkCallback = new CTipMediaSinkCallback();
}

CTipMediaSink::~CTipMediaSink()
{
    delete mpSinkCallback;
}

void CTipMediaSink::SetCallback(CTipMediaSinkCallback* callback)
{
    if (callback == NULL) {
        return;
    }
    
    delete mpSinkCallback;
    mpSinkCallback = callback;
}

void CTipMediaSink::SetSourceCSRC(uint32_t csrc)
{
    mSourceCSRC = csrc;
}

Status CTipMediaSink::RequestRefresh(bool idr)
{
    CRtcpAppRefreshPacket* packet = new CRtcpAppRefreshPacket();
    if (packet == NULL) {
        return TIP_ERROR;
    }

    packet->SetTarget(mSourceCSRC);

    if (idr) {
        packet->SetFlags(CRtcpAppRefreshPacket::REFRESH_REQUIRE_IDR);
    } else {
        packet->SetFlags(CRtcpAppRefreshPacket::REFRESH_PREFER_GDR);
    }

    StartPacketTx(packet);
    return TIP_OK;
}

void CTipMediaSink::ProcessPacket(CRtcpTipPacket* packet)
{
    if (packet->GetTipPacketType() != RXFLOWCTRL) {
        // not something we can handle
        return;
    }

    CRtcpAppRXFlowCtrlPacket* rx = dynamic_cast<CRtcpAppRXFlowCtrlPacket*>(packet);
    if (rx == NULL) {
        AMDEBUG(INTERR, ("%s recv packet with RXFLOWCTRL type but not RXFLOWCTRL object",
                         mLogPrefix.c_str()));
        return;
    }
    

    // devices sending an RXFLOWCTRL may not have any information
    // about a sink's CSRC value.  in this case they will simply place
    // the correct value in the 'source' position of the RXFLOWCTRL
    // target CSRC field.  so that is all we validate here.
    CTipCSRC csrc(rx->GetTarget());
    if (csrc.GetSourcePos() != mCSRC.GetSourcePos()) {
        // not for us, an acceptable error
        return;
    }
    
    if (rx->GetOpcode() == CRtcpAppRXFlowCtrlPacket::OPCODE_START) {
        AMDEBUG(USER, ("%s recv RXFLOWCTRL START", mLogPrefix.c_str()));
        mpSinkCallback->Start();
    } else if (rx->GetOpcode() == CRtcpAppRXFlowCtrlPacket::OPCODE_STOP) {
        AMDEBUG(USER, ("%s recv RXFLOWCTRL STOP", mLogPrefix.c_str()));
        mpSinkCallback->Stop();
    } else {
        // an unknown opcode, do not ack
        AMDEBUG(USER, ("%s recv RXFLOWCTRL invalid opcode %hu",
                       mLogPrefix.c_str(), rx->GetOpcode()));
        return;
    }
    
    AckPacket(packet);
}

void CTipMediaSink::RegisterPacket(uint16_t seqno, bool eof)
{
    // first time through, don't look for gaps
    if (mHaveLastSeqNum == false) {
        mHaveLastSeqNum = true;
        mLastSeqNum = seqno;

        mAcks[seqno] = 1;
        
    } else {
        // not the first time through.  we define the following ranges
        // of seqno jumps:
        //
        // cur - last <= 0x8000 ==> this is a jump "forward"
        // cur - last > 0x8000  ==> this is a jump "backward", ie out of order
        uint16_t diff = (seqno - mLastSeqNum);

        if (diff <= 0x8000) {
            // jump forward, set the received bit
            mAcks[seqno] = 1;

            // 0 anything skipped
            for (uint16_t start = (mLastSeqNum + 1); start < seqno; start++) {
                mAcks[start] = 0;
            }

            // remember this as the latest received seqno
            mLastSeqNum = seqno;
            
        } else {
            // jump backward, just set the bit
            mAcks[seqno] = 1;
        }
    }

    // if this isn't the end of the frame then we are done
    if (eof == false) {
        return;
    }

    // otherwise send out a feedback packet
    CRtcpAppFeedbackPacket packet;
    packet.SetSSRC(mSSRC);
    packet.SetTarget(mSourceCSRC);
    packet.SetPacketID(mLastSeqNum);

    // add the previous 112 packet acks/nacks
    for (uint16_t i = 0; i < CRtcpAppFeedbackPacket::NUM_ACK_BITS; i++) {
        uint16_t num = ((mLastSeqNum - CRtcpAppFeedbackPacket::NUM_ACK_BITS) + i);
        
        packet.SetPacketAckBySeqNum(num, (mAcks[num] ? CRtcpAppFeedbackPacket::APP_FB_ACK : CRtcpAppFeedbackPacket::APP_FB_NACK));
    }

    PrintPacketTx(packet, mMediaType);
    
    // feedback packets are one-shots, so send directly
    CPacketBufferData buffer;
    mPacketManager.Pack(packet, buffer);

    mPacketXmit.Transmit(buffer.GetBuffer(), buffer.GetBufferSize(),
                         mMediaType);
}

CTipMediaSource::CTipMediaSource(MediaType type, uint32_t ssrc, uint32_t csrc,
                                 CTipPacketTransmit& xmit) :
    CTipMedia(type, ssrc, csrc, xmit, "SOURCE")
{
    mHaveLastSeqNum  = false;
    mLastSeqNum      = 0;
    mpSourceCallback = new CTipMediaSourceCallback();

    // enable all callbacks
    mCallbackEnable.set();
}

CTipMediaSource::~CTipMediaSource()
{
    delete mpSourceCallback;
}

void CTipMediaSource::SetCallback(CTipMediaSourceCallback* callback)
{
    if (callback == NULL) {
        return;
    }
    
    delete mpSourceCallback;
    mpSourceCallback = callback;
}

void CTipMediaSource::SetSequenceNumber(uint16_t seqno)
{
    mHaveLastSeqNum = true;
    mLastSeqNum     = seqno;
}

void CTipMediaSource::ProcessPacket(CRtcpTipPacket* packet)
{
    if (packet->GetTipPacketType() == TXFLOWCTRL) {
        CRtcpAppTXFlowCtrlPacket* tx = dynamic_cast<CRtcpAppTXFlowCtrlPacket*>(packet);
        if (tx == NULL) {
            AMDEBUG(INTERR, ("%s recv packet with TXFLOWCTRL type but not TXFLOWCTRL object",
                             mLogPrefix.c_str()));
            return;
        }

        if (tx->GetTarget() != mCSRC.GetCSRC()) {
            // not for us, an acceptable error
            return;
        }

        switch (tx->GetOpcode()) {
        case CRtcpAppFlowCtrlPacket::OPCODE_START:
            AMDEBUG(USER, ("%s recv TXFLOWCTRL START", mLogPrefix.c_str()));
            mpSourceCallback->Start();
            break;

        case CRtcpAppFlowCtrlPacket::OPCODE_STOP:
            AMDEBUG(USER, ("%s recv TXFLOWCTRL STOP", mLogPrefix.c_str()));
            mpSourceCallback->Stop();
            break;

        default:
            AMDEBUG(USER, ("%s recv TXFLOWCTRL invalid opcode %hu",
                           mLogPrefix.c_str(), tx->GetOpcode()));
            return;
        }

        AckPacket(packet);
        
    } else if (packet->GetTipPacketType() == REFRESH) {
        CRtcpAppRefreshPacket* refresh = dynamic_cast<CRtcpAppRefreshPacket*>(packet);
        if (refresh == NULL) {
            AMDEBUG(INTERR, ("%s recv packet with REFRESH type but not REFRESH object",
                             mLogPrefix.c_str()));
            return;
        }

        if (refresh->GetTarget() != mCSRC.GetCSRC()) {
            // not for us, an acceptable error
            return;
        }

        bool idr = false;
        if (refresh->GetFlags() == CRtcpAppRefreshPacket::REFRESH_REQUIRE_IDR) {
            idr = true;
        }

        AMDEBUG(USER, ("%s recv REFRESH %s", mLogPrefix.c_str(),
                       (idr ? "IDR required" : "IDR not required" )));
        mpSourceCallback->Refresh(idr);

        AckPacket(packet);
    }
}

void CTipMediaSource::ProcessFBPacket(CRtcpAppFeedbackPacket* packet)
{
    if (packet->GetTarget() != mCSRC.GetCSRC()) {
        // not for us
        return;
    }

    // if the user did not provide a starting sequence number just
    // assume that we started 112 packets from the packet id of this
    // feedback.  this will almost never be right (typically we will
    // nack a bunch of stuff that was never transmitted) but not much
    // else we can do.
    if (mHaveLastSeqNum == false) {
        mHaveLastSeqNum = true;
        mLastSeqNum     = (packet->GetPacketID() - CRtcpAppFeedbackPacket::NUM_ACK_BITS);
    }
    
    // figure out how many new bits we have to process
    uint16_t new_bits = (packet->GetPacketID() - mLastSeqNum);
    
    // look for out of order FB, if found ignore it as we have already
    // processed these bits
    if (new_bits > 0x8000) {
        return;
    }
    
    // might be extended or not extended
    CRtcpAppExtendedFeedbackPacket* extfb = dynamic_cast<CRtcpAppExtendedFeedbackPacket*>(packet);

    // save off the first invalid seqnum processed as that is where
    // we still start from next time.
    bool haveInvalidSeqNum = false;
    uint16_t firstInvalidSeqNum = (packet->GetPacketID() + 1);
    
    // loop through the new bits in the FB packet invoking the
    // callback for each one.
    for (uint16_t i = 0; i < new_bits; i++) {
        uint16_t curSeqNum = (mLastSeqNum + i);
        
        // in the case of lost feedback packets where we skip more
        // than 112 packets and the seqno is not in the packet, the
        // packet object will return NACK which is what we want.  ie
        // we default to NACK unless we are sure it is an ACK.
        CRtcpAppFeedbackPacket::AckValue ack =
            packet->GetPacketAckBySeqNum(curSeqNum);

        // this will default to VALID if the seqnum is out of range.
        // so the overall default is a VALID NACK which is what we
        // want.
        CRtcpAppExtendedFeedbackPacket::AckValidValue valid =
            CRtcpAppExtendedFeedbackPacket::APP_FB_VALID;
        if (extfb != NULL) {
            valid = extfb->GetPacketAckValidBySeqNum(curSeqNum);
        }

        // if this seqno is marked as invalid
        if (valid == CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID) {
            // if this is the first invalid seqno then remember it
            if (haveInvalidSeqNum == false) {
                firstInvalidSeqNum = curSeqNum;
                haveInvalidSeqNum = true;
            }

            // don't do anything with invalid seqno
            continue;
        }

        if (ack == CRtcpAppFeedbackPacket::APP_FB_NACK) {
            // ok for a VALID NACK to follow an INVALID seqno
            DoAckNackCallback(curSeqNum, false);

        } else {

            // protocol error checking, we should not have a case
            // where a VALID-ACK follows an INVALID seqno.
            if (haveInvalidSeqNum) {
                AMDEBUG(RECV, ("%s invalid FB packet, valid ACK (seqno %hu) follows invalid packet (seqno %hu)",
                               mLogPrefix.c_str(), curSeqNum, firstInvalidSeqNum));
                PrintPacketRx(*packet, VIDEO);

            } else {
                DoAckNackCallback(curSeqNum, true);
            }
        }
    }

    // if there were no invalid seqno, then the packet id is always an
    // ACK
    if (! haveInvalidSeqNum) {
        DoAckNackCallback(packet->GetPacketID(), true);
    }
    
    // remember the last processed sequence number
    mLastSeqNum = firstInvalidSeqNum;
}

void CTipMediaSource::DoAckNackCallback(uint16_t seqNum, bool isAck)
{
    // if we haven't already done a callback for this seqno
    if (mCallbackEnable[seqNum]) {
        if (isAck) {
            mpSourceCallback->ProcessAck(seqNum);
        } else {
            mpSourceCallback->ProcessNack(seqNum);
        }

        // no more callbacks for this seqno until it gets re-enabled
        mCallbackEnable[seqNum] = 0;
    }

    // re-enable the callback for the "flip side" seqno
    uint16_t flipSeqNum = (seqNum + (uint16_t) 0x8000);
    mCallbackEnable[flipSeqNum] = 1;
}
