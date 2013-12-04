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

#include <algorithm>

#include "rtcp_tip_types.h"
#include "rtcp_tip_echo_packet.h"
#include "rtcp_packet_factory.h"
#include "tip_time.h"
#include "tip_debug_print.h"
#include "private/tip_negotiate_state.h"
#include "private/tip_impl.h"
using namespace LibTip;

CTipImpl::CTipImpl(CTipPacketTransmit& xmit) :
    mPacketXmit(xmit), mPresImpl(this)
{
    mSystem.SetTipVersion(SUPPORTED_VERSION_MAX);
    SetRetransmissionInterval(DEFAULT_RETRANS_INTERVAL);
    SetRetransmissionLimit(DEFAULT_RETRANS_LIMIT);

    CTipCallback* callback = new CTipCallback();
    mpCallback = new CTipCallbackWrapper(callback);

    // seed random with time()
    srand(time(NULL));
    
    for (MediaType mType = VIDEO; mType < MT_MAX; ++mType) {
        mpTipNegLocalState[mType] = &gStopLocalState;
        mpTipNegRemoteState[mType] = &gStopRemoteState;

        mMuxCtrlTime[mType] = 0;

        SetRTCPSSRC(mType, rand());
    }
}

CTipImpl::~CTipImpl() {
    delete mpCallback;
}

const CTipSystem& CTipImpl::GetTipNegotiatedSystem() const
{
    return mNegotiatedSystem;
}

void CTipImpl::SetCallback(CTipCallback* callback)
{
    mpCallback->SetCallback(callback);
}

Status CTipImpl::StartTipNegotiate(MediaType mType)
{
    AMDEBUG(USER, ("enter %s", GetMediaString(mType)));
    
    if (mType >= MT_MAX) {
        return TIP_ERROR;
    }

    if (mpTipNegLocalState[mType]->Start(this, mType) != TIP_OK) {
        AMDEBUG(USER, ("error starting local %s tip negotiation",
                       GetMediaString(mType)));
        return TIP_ERROR;
    }

    if (mpTipNegRemoteState[mType]->Start(this, mType) != TIP_OK) {
        AMDEBUG(USER, ("error starting remote %s tip negotiation",
                       GetMediaString(mType)));
        mpTipNegLocalState[mType]->Stop(this, mType);
        return TIP_ERROR;
    }

    AMDEBUG(USER, ("exit %s OK", GetMediaString(mType)));
    return TIP_OK;
}

Status CTipImpl::StopTipNegotiate(MediaType mType)
{
    if (mType >= MT_MAX) {
        return TIP_ERROR;
    }

    mpTipNegLocalState[mType]->Stop(this, mType);
    mpTipNegRemoteState[mType]->Stop(this, mType);
    
    return TIP_OK;
}

Status CTipImpl::StartPresentation()
{
    AMDEBUG(USER, ("enter"));
    
    // cannot do presentation until local tip negotation has finished
    // successfully for video
    if (mpTipNegLocalState[VIDEO] != &gDoneLocalState) {
        return TIP_ERROR;
    }
    
    return mPresImpl.StartPresentation();
}

Status CTipImpl::StopPresentation()
{
    AMDEBUG(USER, ("enter"));
    return mPresImpl.StopPresentation();
}

Status CTipImpl::ReceivePacket(uint8_t* buffer, uint32_t size, MediaType mType)
{
    // only return an error if the packet did not contain any TIP
    // packets
    Status ret = TIP_ERROR;
    
    if (buffer == NULL) {
        return ret;
    }
    
    CPacketBuffer packetBuf(buffer, size);

    AMDEBUG(RECV, ("recv %s rtcp packet buffer size %d bytes",
                   GetMediaString(mType), size));
    
    while (packetBuf.GetBufferSize()) {
        CRtcpPacket* rtcp = CRtcpPacketFactory::CreatePacketFromBuffer(packetBuf);

        if (rtcp == NULL) {
            // something we couldn't parse, keep going until the
            // buffer is empty
            continue;
        }

        // only handle TIP packets
        CRtcpTipPacket* packet = dynamic_cast<CRtcpTipPacket*>(rtcp);
        if (packet == NULL) {
            // might just be a compound packet, ignore it and try the
            // next one
            delete rtcp;
            continue;
        }

        // if we make it to here then we have at least one TIP packet
        // to process.
        ret = TIP_OK;
        
        // process ACK packets
        if (IsAckTipPacketType(packet->GetTipPacketType())) {
            ProcessAckPacket(packet, mType);
            delete packet;
            continue;
        }

        CTipPacketReceiver::Action action = mPacketReceiver[mType].ProcessPacket(packet);

        if (action == CTipPacketReceiver::AMPR_DROP) {
            AMDEBUG(RECV, ("dropping old %s packet type %s ntp time %llu",
                           GetMediaString(mType),
                           packet->GetTipPacketTypeString(),
                           packet->GetNtpTime()));

            delete packet;
            
        } else if (action == CTipPacketReceiver::AMPR_DUP) {
            AMDEBUG(RECV, ("acking dup %s packet type %s ntp time %llu",
                           GetMediaString(mType),
                           packet->GetTipPacketTypeString(),
                           packet->GetNtpTime()));

            AckDuplicatePacket(packet, mType);
            delete packet;
            
        } else {

            // in this case the packet receiver has taken ownership of
            // the received packet.  when/if the packet is acked that
            // memory will be freed.  if it is never acked then the
            // next packet of the same time will free it or it will be
            // freed when the receiver is destroyed.
            
            ProcessPacket(packet, mType);
        }
    }

    return ret;
}

Status CTipImpl::SendDelayedAck(void* id, MediaType mType)
{
    // we only allow users to send delayed ack for packets that we
    // know are pending.  these packets will be stored in the Tip
    // receiver, so look for it there.
    CRtcpTipPacket* packet = mPacketReceiver[mType].FindID(id);
    if (packet == NULL) {
        return TIP_ERROR;
    }
    
    AckPacket(packet, mType);
    return TIP_OK;
}

void CTipImpl::ProcessPacket(CRtcpTipPacket* packet, MediaType mType)
{
    TipPacketType pType = packet->GetTipPacketType();
    AMDEBUG(RECV, ("recv %s rtcp app packet type %s size %d bytes",
                   GetMediaString(mType),
                   GetTipPacketTypeString(pType),
                   packet->GetPackSize()));
    
    // process packet based on type
    if (pType == MUXCTRL) {

        CRtcpAppMuxCtrlPacketBase* muxctrl = dynamic_cast<CRtcpAppMuxCtrlPacketBase*>(packet);
        if (muxctrl == NULL) {
            // internal error, packet type is MUXCTRL byt object is not?
            AMDEBUG(INTERR, ("recv packet with MUXCTRL type but not MUXCTRL object"));
            return;
        }

        PrintPacket(muxctrl, mType, true);
                
        mpTipNegRemoteState[mType]->RxMC(this, *muxctrl, mType);
        
    } else if (pType == MEDIAOPTS) {

        CRtcpAppMediaoptsPacket* mo = dynamic_cast<CRtcpAppMediaoptsPacket*>(packet);
        if (mo == NULL) {
            // internal error, packet type is MEDIAOPTS but object is not?
            AMDEBUG(INTERR, ("recv packet with MEDIAOPTS type but not MEDIAOPTS object"));
            return;
        }

        PrintPacket(mo, mType, true);
        mpTipNegRemoteState[mType]->RxMO(this, *mo, mType);

    } else if (pType == REQTOSEND) {

        CRtcpAppReqToSendPacket* rts = dynamic_cast<CRtcpAppReqToSendPacket*>(packet);
        if (rts == NULL) {
            // internal error, packet type is REQTOSEND but object is not?
            AMDEBUG(INTERR, ("recv packet with REQTOSEND type but not REQTOSEND object"));
            return;
        }

        PrintPacket(rts, mType, true);
        mpTipNegRemoteState[mType]->RxReqToSend(this, *rts, mType);
        
    } else if (pType == TIPECHO) {

        CRtcpAppEchoPacket* echo = dynamic_cast<CRtcpAppEchoPacket*>(packet);
        if (echo == NULL) {
            // internal error, packet type is ECHO byt object is not?
            AMDEBUG(INTERR, ("recv packet with ECHO type but not ECHO object"));
            return;
        }

        // received an echo, send ack response
        AckPacket(echo, mType);

    } else if (pType == SPIMAP) {

        CRtcpAppSpiMapPacket* spimap = dynamic_cast<CRtcpAppSpiMapPacket*>(packet);
        if (spimap == NULL) {
            // internal error, packet type is SPIMAP but object is not?
            AMDEBUG(INTERR, ("recv packet with SPIMAP type but not SPIMAP object"));
            return;
        }

        PrintPacket(spimap, mType, true);
        mpTipNegRemoteState[mType]->RxSpiMap(this, *spimap, mType);

    } else if (pType == NOTIFY) {

        CRtcpAppNotifyPacket* notify = dynamic_cast<CRtcpAppNotifyPacket*>(packet);
        if (notify == NULL) {
            // internal error, packet type is NOTIFY but object is not?
            AMDEBUG(INTERR, ("recv packet with NOTIFY type but not NOTIFY object"));
            return;
        }

        PrintPacket(notify, mType, true);
        mpTipNegRemoteState[mType]->RxNotify(this, *notify, mType);

    } else {

        // un-handled packet types
        AMDEBUG(PKTERR, ("recv unhandled %s packet type %s size %d bytes",
                         GetMediaString(mType),
                         GetTipPacketTypeString(pType),
                         packet->GetPackSize()));
    }
}

void CTipImpl::ProcessAckPacket(CRtcpTipPacket* packet, MediaType mType)
{
    CRtcpTipPacket* acked = mPacketManager[mType].Ack(*packet);
    if (acked == NULL) {
        // received an ACK for something we don't know anything about.  just drop it.
        return;
    }
        
    TipPacketType ackedType = acked->GetTipPacketType();
    AMDEBUG(RECV, ("recv ack for xmit packet type %s",
                   GetTipPacketTypeString(ackedType)));
        
    if (ackedType == MUXCTRL) {
        mpTipNegLocalState[mType]->RxMCAck(this, *packet, mType);
            
    } else if (ackedType == MEDIAOPTS) {
        mpTipNegLocalState[mType]->RxMOAck(this, mType);

    } else if (ackedType == REQTOSEND) {
        CRtcpAppReqToSendAckPacket* rtsack =
            dynamic_cast<CRtcpAppReqToSendAckPacket*>(packet);
        if (rtsack == NULL) {
            AMDEBUG(TIPNEG, ("invalid REQTOSEND ack packet received"));
        } else {
            mPresImpl.ProcessAckPacket(rtsack);
        }
    }
    
    delete acked;
}

void CTipImpl::SetRetransmissionInterval(uint32_t intervalMS)
{
    mPacketManager[VIDEO].SetRetransmissionInterval(intervalMS);
    mPacketManager[AUDIO].SetRetransmissionInterval(intervalMS);
}

void CTipImpl::SetRetransmissionLimit(uint32_t limit)
{
    mPacketManager[VIDEO].SetRetransmissionLimit(limit);
    mPacketManager[AUDIO].SetRetransmissionLimit(limit);
}

void CTipImpl::StartPacketTx(CRtcpTipPacket* packet, MediaType mType)
{
    AMDEBUG(XMIT, ("starting %s packet tx for type %s",
                   GetMediaString(mType),
                   GetTipPacketTypeString(packet->GetTipPacketType())));
             
    TipPacketType pType = packet->GetTipPacketType();
    StopPacketTx(pType, mType);

    uint64_t ntpTime = GetNtpTimestamp();
    if (pType == MUXCTRL) {
        // save off this one
        mMuxCtrlTime[mType] = ntpTime;
    }

    packet->SetSSRC(mSSRC[mType]);
    packet->SetNtpTime(ntpTime);
    mPacketManager[mType].Add(packet);
    PrintPacket(packet, mType, false);
}

void CTipImpl::StopPacketTx(TipPacketType pType, MediaType mType)
{
    CRtcpTipPacket* packet = mPacketManager[mType].Remove(pType);
    delete packet;
}

uint64_t CTipImpl::GetIdleTime() const
{
    // return lowest of the packet manager and timer idle times
    uint64_t retV = mPacketManager[VIDEO].GetNextTransmitTime();
    uint64_t retA = mPacketManager[AUDIO].GetNextTransmitTime();
    uint64_t retT = mTimer.GetNextExpiredTime();

    return std::min(std::min(retV, retA), retT);
}

void CTipImpl::DoPeriodicActivity()
{
    for (MediaType mType = VIDEO; mType < MT_MAX; ++mType) {
        if (mPacketManager[mType].GetNextTransmitTime() == 0) {
            // time to send out some packets
            bool expired;
            CPacketBuffer* buffer;
        
            CRtcpTipPacket* packet = mPacketManager[mType].GetPacket(expired, &buffer);
            while (packet != NULL) {
                if (expired) {
                    // packet has timed out, what to do, what to do
                    HandleTimeout(packet, mType);
                } else {
                    // packet should be sent
                    AMDEBUG(XMIT, ("xmit %s packet type %s size %d bytes",
                                   GetMediaString(mType),
                                   packet->GetTipPacketTypeString(),
                                   buffer->GetBufferSize()));

                    RelayPacket(buffer, mType);
                }
            
                packet = mPacketManager[mType].GetPacket(expired, &buffer);
            }
        }
    }

    CTipTimer::TimerType type;
    uint32_t data;
    
    while (mTimer.GetExpired(type, data) != TIP_ERROR) {
        if (type == CTipTimer::AMT_TIP_NEGOTIATE) {
            MediaType mType = (MediaType) data;

            mpTipNegRemoteState[mType]->Timeout(this, mType);
        }
    }
}

uint32_t CTipImpl::GetRTCPSSRC(MediaType mType) const
{
    if (mType >= MT_MAX) {
        return 0;
    }

    return mSSRC[mType];
}

void CTipImpl::SetRTCPSSRC(MediaType mType, uint32_t ssrc)
{
    if (mType >= MT_MAX) {
        return;
    }

    // SSRC used must end in 00 (per spec).
    mSSRC[mType] = (ssrc & 0xFFFFFF00);

    // configure packet managers to add an empty RR and SDES
    // before each Tip packet
    mPacketManager[mType].EnableWrapper(mSSRC[mType]);
}

void CTipImpl::HandleTimeout(CRtcpTipPacket* packet, MediaType mType)
{
    TipPacketType pType = packet->GetTipPacketType();

    AMDEBUG(USER, ("timeout for packet type %s", GetTipPacketTypeString(pType)));
    
    if (pType == MUXCTRL || pType == MEDIAOPTS) {
        // timeout on a tip negotiation related packet, feed that into
        // the state machine.
        mpTipNegLocalState[mType]->Timeout(this, mType);

    } else if (pType == REQTOSEND) {
        // timeout on a pres negotiation related packet
        mPresImpl.ProcessPacketTimeout();
    }

    delete packet;
}

void CTipImpl::RelayPacket(CPacketBuffer* buffer, MediaType mType)
{
    mPacketXmit.Transmit(buffer->GetBuffer(), buffer->GetBufferSize(), mType);
}

void CTipImpl::AckPacket(const CRtcpTipPacket* packet, MediaType mType)
{
    CRtcpTipPacket* ack = CRtcpPacketFactory::CreateAckPacket(*packet);
    if (ack == NULL) {
        AMDEBUG(USER, ("ack failed for packet type %d", packet->GetTipPacketType()));
        return;
    }

    PrintPacket(ack, mType, false);
    
    // note that we do not free the memory for the ACK, TransmitAck
    // passes ownership of the ack memory to the packet receiver.
    TransmitAck(ack, packet, mType);
}

void CTipImpl::AckDuplicatePacket(const CRtcpTipPacket* packet, MediaType mType)
{
    CRtcpTipPacket* ack = mPacketReceiver[mType].FindDupAck(*packet);
    if (ack == NULL) {
        AMDEBUG(INTERR, ("failed to find duplicate ack for  packet type %d",
                         packet->GetTipPacketType()));
        return;
    }

    TransmitAck(ack, packet, mType);
}

void CTipImpl::TransmitAck(CRtcpTipPacket* ack, const CRtcpTipPacket* packet,
                           MediaType mType)
{
    // setup header
    ack->SetSSRC(mSSRC[mType]);
    
    // pack ACK into a buffer and sent it out
    CPacketBufferData buffer;
    mPacketManager[mType].Pack(*ack, buffer);

    AMDEBUG(XMIT, ("xmit ack for %s packet type %s.  ack type %s size %d bytes",
                   GetMediaString(mType),
                   packet->GetTipPacketTypeString(),
                   ack->GetTipPacketTypeString(), buffer.GetBufferSize()));
    
    RelayPacket(&buffer, mType);

    // notify our receiver that this packet has been acked.
    // duplicates will now be acked as well.  receiver now owns the
    // ack.
    mPacketReceiver[mType].RegisterAck(*packet, ack);
}

Status CTipImpl::OnLocalStart(MediaType mType)
{
    CRtcpAppMuxCtrlPacketBase* packet = mSystem.MapToMuxCtrl(mType);
    if (packet == NULL) {
        AMDEBUG(INTERR, ("error creating %s MUXCTRL packet", GetMediaString(mType)));
        return TIP_ERROR;
    }

    StartPacketTx(packet, mType);
    SetLocalState(mType, &gMCTxLocalState);
    return TIP_OK;
}

void CTipImpl::OnRxMCAck(MediaType mType)
{
    CRtcpAppMediaoptsPacket* packet = mSystem.MapToMediaOpts(mType);
    if (packet == NULL) {
        AMDEBUG(INTERR, ("error creating %s MEDIAOPTS packet", GetMediaString(mType)));
        
        mpCallback->TipNegotiationFailed(mType);
        StopTipNegotiate(mType);
        return;
    }
    
    StartPacketTx(packet, mType);
    SetLocalState(mType, &gMOTxLocalState);
}

void CTipImpl::OnRxMCAckAfterNegotiate(CRtcpTipPacket& ack, MediaType mType)
{
    // this should only happen when we are in V6 mode
    if (mSystem.GetTipVersion() >= TIP_V7) {
        AMDEBUG(TIPNEG, ("error received MC ACK after negotiate, but not in V6 mode"));
        return;
    }

    // this should only happen on VIDEO
    if (mType != VIDEO) {
        AMDEBUG(TIPNEG, ("error received MC ACK after negotiate for audio"));
        return;
    }

    // for V6 video, this is a presentation related event so feed it
    // into the presentation logic.
    mPresImpl.ProcessAckPacket(&ack);
}

void CTipImpl::OnRxMOAck(MediaType mType)
{
    SetLocalState(mType, &gDoneLocalState);
    UpdateNegotiatedSystem();
    mpCallback->TipNegotiationLastAckReceived(mType);
}

void CTipImpl::OnLocalTimeout(MediaType mType)
{
    SetLocalState(mType, &gTimeoutLocalState);
    mpCallback->TipNegotiationFailed(mType);
    StopTipNegotiate(mType);
}

void CTipImpl::OnLocalTimeoutAfterNegotiate(MediaType mType)
{
    // should only happen when we are in V6 mode and have a pending
    // MUXCTRL packet
    if (mType == VIDEO && mSystem.GetTipVersion() < TIP_V7) {
        mPresImpl.ProcessPacketTimeout();
    }
}

void CTipImpl::OnLocalStop(MediaType mType)
{
    SetLocalState(mType, &gStopLocalState);

    mTimer.Cancel(mTipNegTimerId[mType]);

    StopPacketTx(MUXCTRL, mType);
    StopPacketTx(MEDIAOPTS, mType);
}

void CTipImpl::OnRemoteStart(MediaType mType)
{
    // forget any previously received tip packets
    mPacketReceiver[mType].Forget(MUXCTRL);
    mPacketReceiver[mType].Forget(MEDIAOPTS);
    
    // start timer waiting for tip negotiation to timeout.  timeout
    // interval is the packet timeout interval.
    uint64_t timeout = mPacketManager[mType].GetPacketTimeoutMsec();
    mTipNegTimerId[mType] = mTimer.Register(CTipTimer::AMT_TIP_NEGOTIATE,
                                            timeout, mType);
    
    SetRemoteState(mType, &gNoRxRemoteState);
}

void CTipImpl::OnRxMCBeforeNegotiate(const CRtcpAppMuxCtrlPacketBase& mPacket,
                                     MediaType mType)
{
    mpCallback->TipNegotiationEarly(mType);
}

void CTipImpl::OnRxMCDuringNegotiate(CRtcpAppMuxCtrlPacketBase& muxctrl,
                                     MediaType mType)
{
    if (mRemoteSystem.MapFromMuxCtrl(muxctrl, mType) != TIP_OK) {
        // something wrong with this MUXCTRL, perhaps something
        // corrupted during transmission.  forget about it and process
        // the next one like it was new.
        mPacketReceiver[mType].Forget(muxctrl.GetTipPacketType());
        return;
    }

    // version handling rules as follows:
    //
    // 1. if the remote version is higher than our version then do
    // nothing and do not ack.  the remote side should downgrade to
    // our version.  if it does not we will timeout.
    //
    // 2. if the remote version is less than our version then do not
    // ACK and...
    //
    // 2a.  if the remote version is within our supported range,
    // restart tip negotiation with lower version based on user
    // callback.
    //
    // 2b.  else abort tip negotiation and inform user
    //
    // 3.  if the versions are the same then continue with tip
    // negotiation.

    if (mRemoteSystem.GetTipVersion() > mSystem.GetTipVersion()) {
        // rule #1
        AMDEBUG(TIPNEG,
                ("received higher %s tip version, our version %hhu their version %hhu",
                 GetMediaString(mType),
                 mSystem.GetTipVersion(),
                 mRemoteSystem.GetTipVersion()));

    } else if (mRemoteSystem.GetTipVersion() < mSystem.GetTipVersion()) {
        // rule #2
        
        if (mRemoteSystem.GetTipVersion() >= SUPPORTED_VERSION_MIN &&
            mRemoteSystem.GetTipVersion() <= SUPPORTED_VERSION_MAX) {

            // rule #2a
            AMDEBUG(TIPNEG,
                    ("received lower %s tip version, our version %hhu their version %hhu",
                     GetMediaString(mType),
                     mSystem.GetTipVersion(),
                     mRemoteSystem.GetTipVersion()));

            // we have a mismatch we can handle, inform the user and
            // if they don't stop us restart tip negotiation with the
            // lower version.  note that in either case we are *not*
            // acking the MUXCTRL.  the state machine requires a 2nd
            // MUXCTRL to be received with a matching version.
            if (mpCallback->TipNegotiationMismatch(mType, mRemoteSystem.GetTipVersion())) {

                mSystem.SetTipVersion(mRemoteSystem.GetTipVersion());

                // note that we restart all media types here as there
                // is only a single TIP version for all media types.
                // if tip negotiation was not running on the media
                // type then it will not be restarted.
                for (MediaType mt = VIDEO; mt < MT_MAX; ++mt) {
                    mpTipNegLocalState[mt]->Restart(this, mt);
                    mpTipNegRemoteState[mt]->Restart(this, mt);
                }

            } else {

                // user does not want us to continue so stop tip negotiation
                AMDEBUG(TIPNEG, ("user stopped tip negotiation via mismatch callback"));
                StopTipNegotiate(mType);
                    
            }

        } else {

            // rule #2b
            AMDEBUG(TIPNEG,
                    ("received incompatible %s tip version, our version %hhu their version %hhu",
                     GetMediaString(mType),
                     mSystem.GetTipVersion(),
                     mRemoteSystem.GetTipVersion()));
            
            // we have a mismatch that we cannot handle, inform
            // the user.  we do not stop tip negotiation here, the
            // remote side may downgrade to our version so we will
            // wait for that.
            mpCallback->TipNegotiationIncompatible(mType, mRemoteSystem.GetTipVersion());
            StopTipNegotiate(mType);
        }
        
    } else {
        // rule #3
        AckPacket(&muxctrl, mType);
        SetRemoteState(mType, &gMCRxRemoteState);
    }
}

void CTipImpl::OnRxMCAfterNegotiate(CRtcpAppMuxCtrlPacketBase& muxctrl,
                                    MediaType mType)
{
    if (mRemoteSystem.MapFromMuxCtrl(muxctrl, mType) != TIP_OK) {
        // something wrong with this MUXCTRL, perhaps something
        // corrupted during transmission.  forget about it and process
        // the next one like it was new.
        mPacketReceiver[mType].Forget(muxctrl.GetTipPacketType());
        return;
    }
    
    UpdateNegotiatedSystem();

    // V6 mux uses video MUXCTRL to negotiate presentation, give the
    // presentation logic a shot at those packets.
    if (mType == VIDEO && mRemoteSystem.GetTipVersion() < TIP_V7) {
        if (mPresImpl.ProcessPacket(&muxctrl, mType) == TIP_OK) {
            // consumed by presentation
            return;
        }
    }

    // and give the user a shot
    if (mpCallback->TipNegotiationUpdate(mType, &muxctrl)) {
        AckPacket(&muxctrl, mType);
    }
}

void CTipImpl::OnRxMODuringNegotiate(CRtcpAppMediaoptsPacket& mo, MediaType mType)
{
    if (mRemoteSystem.MapFromMediaOpts(mo, mType) != TIP_OK) {
        // something wrong with this MEDIAOPTS, perhaps something
        // corrupted during transmission.  forget about it and process
        // the next one like it was new.
        mPacketReceiver[mType].Forget(mo.GetTipPacketType());
        return;
    }

    UpdateNegotiatedSystem();
    SetRemoteState(mType, &gDoneRemoteState);
    
    // set reInvite to true if ...
    // 1. tip version is >= 7
    // 2. this is for video
    //
    // and either
    // local side is MCU
    // *or*
    // remote side is NOT MCU and our MUXCTRL NTP timestamp is smaller
    bool reInvite = false;
    if (mSystem.GetTipVersion() >= TIP_V7 && mType == VIDEO) {

        if (mSystem.GetMCUState()) {
            reInvite = true;

        } else if (mRemoteSystem.GetMCUState() == false) {
            CRtcpTipPacket* p = mPacketReceiver[mType].Find(MUXCTRL);
            if (p != NULL && mMuxCtrlTime[mType] < p->GetNtpTime()) {
                reInvite = true;
            }
        }
    }

    if (mpCallback->TipNegotiationLastAckTransmit(mType, reInvite, &mo)) {
        AckPacket(&mo, mType);
    }
}

void CTipImpl::OnRxMOAfterNegotiate(CRtcpAppMediaoptsPacket& mo, MediaType mType)
{
    if (mRemoteSystem.MapFromMediaOpts(mo, mType) != TIP_OK) {
        // something wrong with this MEDIAOPTS, perhaps something
        // corrupted during transmission.  forget about it and process
        // the next one like it was new.
        mPacketReceiver[mType].Forget(mo.GetTipPacketType());
        return;
    }

    // save off previous negotiated presentation mode
    CTipSystem::PresentationFrameRate oldFrameRate = mNegotiatedSystem.GetPresentationFrameRate();
    
    UpdateNegotiatedSystem();

    // video MO updates may affect presentation so feed this into the
    // presentation logic as well
    if (mType == VIDEO) {
        mPresImpl.NegotiationUpdate(oldFrameRate);
    }
        
    if (mpCallback->TipNegotiationUpdate(mType, &mo)) {
        AckPacket(&mo, mType);
    }
}

void CTipImpl::OnRxReqToSend(CRtcpAppReqToSendPacket& rts, MediaType mType)
{
    // do not process REQTOSEND for TIP V6
    if (mSystem.GetTipVersion() < TIP_V7) {
        AMDEBUG(RECV, ("received REQTOSEND in TIP V6 mode, ignoring"));
        return;
    }
    
    mPresImpl.ProcessPacket(&rts, mType);
}

void CTipImpl::OnRxSpiMap(CRtcpAppSpiMapPacket& spimap, MediaType mType)
{
    // do not process SPIMAP for TIP V6
    if (mSystem.GetTipVersion() < TIP_V7) {
        AMDEBUG(RECV, ("received SPIMAP in TIP V6 mode, ignoring"));
        return;
    }
    
    // only invoke user callback if negotiated system is secure,
    // otherwise just ignore
    if (mNegotiatedSystem.GetSecurityState()) {
        bool ret =  mpCallback->TipSecurityKeyUpdate(mType, spimap.GetSPI(), 
                                                     spimap.GetSrtpSalt(),
                                                     spimap.GetKek(), &spimap);
        if (ret) {
            AckPacket(&spimap, mType);
        }
    }
}

void CTipImpl::OnRxNotify(const CRtcpAppNotifyPacket& notify, MediaType mType)
{
    // do not process NOTIFY for TIP V6
    if (mSystem.GetTipVersion() < TIP_V7) {
        AMDEBUG(RECV, ("received NOTIFY in TIP V6 mode, ignoring"));
        return;
    }
    
    // we only care about the security state TLV in the notify
    int length = notify.GetTLVByTag(CRtcpAppNotifyPacket::SECURITYICON, NULL);

    // length must be 1 byte
    if (length == 1) {
        uint8_t data;
        notify.GetTLVByTag(CRtcpAppNotifyPacket::SECURITYICON, &data);

        mpCallback->TipSecurityStateUpdate(mType, ((data == 1) ? true : false));
        
    } else {
        AMDEBUG(RECV, ("invalid NOTIFY with %s SECURITYICON TLV",
                       ((length == 0) ? "MISSING" : "INVALID")));
    }

    // valid or invalid, ack it
    AckPacket(&notify, mType);
}

void CTipImpl::OnRemoteTimeout(MediaType mType)
{
    SetRemoteState(mType, &gTimeoutRemoteState);
    mpCallback->TipNegotiationFailed(mType);
    StopTipNegotiate(mType);
}

void CTipImpl::OnRemoteStop(MediaType mType)
{
    SetRemoteState(mType, &gStopRemoteState);
}

void CTipImpl::SetLocalState(MediaType mType, CLocalState* state)
{
    AMDEBUG(TIPNEG, ("Changing %s local tip state from %s to %s",
                     GetMediaString(mType),
                     mpTipNegLocalState[mType]->GetName(),
                     state->GetName()));
    
    mpTipNegLocalState[mType] = state;
}

void CTipImpl::SetRemoteState(MediaType mType, CRemoteState* state)
{
    AMDEBUG(TIPNEG, ("Changing %s remote tip state from %s to %s",
                     GetMediaString(mType),
                     mpTipNegRemoteState[mType]->GetName(),
                     state->GetName()));
    
    mpTipNegRemoteState[mType] = state;
}

void CTipImpl::UpdateNegotiatedSystem()
{
    mNegotiatedSystem.NegotiateLocalRemote(mSystem, mRemoteSystem);

    std::ostringstream stream;
    stream << mNegotiatedSystem;
    AMDEBUG(TIPNEG, ("negotiated system dump:%s", stream.str().c_str()));
}

void CTipImpl::PrintPacket(const CRtcpTipPacket* packet, MediaType mType, bool isRX) const
{
    std::ostringstream stream;
    packet->ToStream(stream, mType);
    AMDEBUG(TIPNEG, ("%s %s packet dump:%s",
                     GetMediaString(mType),
                     (isRX ? "RX" : "TX"),
                     stream.str().c_str()));
}
