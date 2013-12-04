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
#include "rtcp_packet_factory.h"
#include "private/tip_impl.h"
#include "private/tip_pres_impl.h"
using namespace LibTip;

CTipPresImpl::CTipPresImpl(CTipImpl* impl) :
    mpImpl(impl), mpState(&gNoPresState), mpPacketToAck(NULL),
    mPacketToAckMediaType(MT_MAX),
    mRemoteAssertVideoPos(0), mRemoteAssertAckedVideoPos(0)
{

}

CTipPresImpl::~CTipPresImpl()
{

}

Status CTipPresImpl::StartPresentation()
{
    return mpState->LocalAssert(this);
}

Status CTipPresImpl::StopPresentation()
{
    return mpState->LocalRelease(this);
}

Status CTipPresImpl::ProcessPacket(CRtcpAppReqToSendPacket* rts, MediaType mType)
{
    Status ret;

    // save off this packet as the one to ACK
    mpPacketToAck = rts;
    mPacketToAckMediaType = mType;
    
    if (rts->GetFlags() == CRtcpAppReqToSendPacket::REQTOSEND_STOP) {
        ret = mpState->RemoteRelease(this, *rts);
    } else if (rts->GetFlags() == CRtcpAppReqToSendPacket::REQTOSEND_START) {
        ret = mpState->RemoteAssert(this, *rts);
    } else {
        AMDEBUG(TIPNEG, ("invalid REQTOSEND flags received 0x%x", rts->GetFlags()));
        return TIP_ERROR;
    }

    if (ret != TIP_OK) {
        AMDEBUG(TIPNEG, ("error occured while processing REQTOSEND packet"));
    }

    return ret;
}

Status CTipPresImpl::ProcessPacket(CRtcpAppMuxCtrlPacketBase* muxctrl, MediaType mType)
{
    // this function is invoked when we are in V6 mode and a new
    // MUXCTRL packet is received after initial tip negotiation.
    // convert the packet into a REQTOSEND and pass it into the normal
    // logic.  note that V6 only supports a single position.

    // save off this packet as the one to ACK.  the internally
    // generated REQTOSEND packet will not be ACKed.
    mpPacketToAck = muxctrl;
    mPacketToAckMediaType = mType;
    
    uint16_t v6pos = PositionToMask(POS_VIDEO_AUX_1_5FPS);

    CRtcpAppReqToSendPacket rts;
    rts.SetSSRC(muxctrl->GetSSRC());
    rts.SetNtpTime(muxctrl->GetNtpTime());
    rts.SetVideoPos(v6pos);
    
    if ((muxctrl->GetXmitPositions() & v6pos)) {
        rts.SetFlags(CRtcpAppReqToSendPacket::REQTOSEND_START);
        mpState->RemoteAssert(this, rts);
    } else {
        rts.SetFlags(CRtcpAppReqToSendPacket::REQTOSEND_STOP);
        mpState->RemoteRelease(this, rts);
    }        

    // note that one of the problems with the V6 presentation logic
    // (and hence why it was changed in V7) is you cannot easily tell
    // the intent of the remote party when sending a new MUXCTRL.
    // were they trying to assert control but messed up the packet or
    // did the packet simply not have anything to do with
    // presentation.  for purposes of this function if we sent an ACK
    // then assume the packet was for presentation.
    if (mpPacketToAck == NULL) {
        return TIP_OK;
    }

    return TIP_ERROR;
}

void CTipPresImpl::ProcessAckPacket(CRtcpAppReqToSendAckPacket* packet)
{
    Status ret = mpState->LocalAssertAck(this, *packet);
    if (ret != TIP_OK) {
        AMDEBUG(TIPNEG, ("error occured while processing REQTOSEND ACK packet"));
    }
}

void CTipPresImpl::ProcessAckPacket(CRtcpTipPacket* packet)
{
    CRtcpAppReqToSendAckPacket ack;
    ack.SetVideoPos(PositionToMask(POS_VIDEO_AUX_1_5FPS));
    
    Status ret = mpState->LocalAssertAck(this, ack);
    if (ret != TIP_OK) {
        AMDEBUG(TIPNEG, ("error occured while processing MUXCTRL-REQTOSEND ACK packet"));
    }
}

void CTipPresImpl::ProcessPacketTimeout()
{
    Status ret = mpState->LocalAssertTimeout(this);
    if (ret != TIP_OK) {
        AMDEBUG(TIPNEG, ("error occured while processing timeout"));
    }
}

void CTipPresImpl::NegotiationUpdate(CTipSystem::PresentationFrameRate oldRate)
{
    mpState->OptionUpdate(this, oldRate);
}

Status CTipPresImpl::OnLocalAssert()
{
    // we are asserting control over presentation, for V6 send out a
    // MUXCTRL, for V7 send out a REQTOSEND packet.
    if (mpImpl->mSystem.GetTipVersion() < TIP_V7) {

        // add presentation transmitter
        mpImpl->mSystem.AddTransmitter(VIDEO, POS_VIDEO_AUX_1_5FPS);

        // get MUXCTRL packet, VIDEO only
        CRtcpAppMuxCtrlPacketBase* packet = mpImpl->mSystem.MapToMuxCtrl(VIDEO);
        if (packet == NULL) {
            AMDEBUG(INTERR, ("error creating VIDEO MUXCTRL packet"));
            return TIP_ERROR;
        }

        // always VIDEO only
        mpImpl->StartPacketTx(packet, VIDEO);
        
    } else {
        CRtcpAppReqToSendPacket* packet = mpImpl->mNegotiatedSystem.MapToReqToSend();
        if (packet == NULL) {
            AMDEBUG(INTERR, ("error creating REQTOSEND packet"));
            return TIP_ERROR;
        }

        packet->SetFlags(CRtcpAppReqToSendPacket::REQTOSEND_START);

        // always send REQTOSEND on video only
        mpImpl->StartPacketTx(packet, VIDEO);
    }
    
    // transition to the local assert state (waiting on ack)
    ChangeState(&gLocalAssertPresState);
    
    return TIP_OK;
}

Status CTipPresImpl::OnLocalAssertAck(CRtcpAppReqToSendAckPacket& packet)
{
    // for V6 a MUXCTRL ack is not actually permission to send, it is
    // simply an ACK of the MUXCTRL packet itself.  the permission to
    // send is granted by the aux receive bit being set to 1.  so if
    // that bit is set then proceed as normal.  if it is not, then we
    // go into the override state waiting for the remote to grant
    // permission.
    if (mpImpl->mSystem.GetTipVersion() < TIP_V7) {
        if (! mpImpl->mRemoteSystem.GetReceivers(VIDEO).test(POS_VIDEO_AUX_1_5FPS)) {
            ChangeState(&gOverridePresState);
            return TIP_OK;
        }
    }
    
    uint16_t lVideoPos;
    mpImpl->mNegotiatedSystem.MapToActiveSharedPos(VIDEO, lVideoPos);

    // figure out which streams should be active.
    uint16_t rVideoPos      = packet.GetVideoPos();
    uint16_t commonVideoPos = (lVideoPos & rVideoPos);

    if (commonVideoPos == 0) {
        // no positions in common, shouldn't happen
        AMDEBUG(TIPNEG, ("error in remote REQTOSEND ACK, no common positions.  "
                         "video local = 0x%hx, remote = 0x%hx.",
                         lVideoPos, rVideoPos));
        return TIP_ERROR;
    }

    // tell user to start presentation.  invoke callback once per set bit.
    const uint8_t pos[] = { POS_VIDEO_AUX_1_5FPS, POS_VIDEO_AUX_30FPS };
    for (uint32_t i = 0; i < sizeof(pos); i++) {
    
        if (commonVideoPos & PositionToMask(pos[i])) {
            CTipSystem::PresentationStreamFrameRate rate =
                mpImpl->mNegotiatedSystem.MapPositionToFrameRate(pos[i]);
            
            AMDEBUG(TIPNEG, ("invoking LocalPresentationStart for position %d rate %d",
                             pos[i], rate));
        
            mpImpl->mpCallback->LocalPresentationStart(pos[i], rate);
        }
    }

    // transition to the local state
    ChangeState(&gLocalPresState);
    
    return TIP_OK;
}

Status CTipPresImpl::OnLocalAssertTimeout()
{
    // nothing to do really, just notify user and transition back to
    // the no presentation state
    AMDEBUG(TIPNEG, ("timeout on REQTOSEND, no ack from remote Tip"));
    mpImpl->mpCallback->LocalPresentationFailure();
    ChangeState(&gNoPresState);

    return TIP_OK;
}

Status CTipPresImpl::OnRemoteAssert(CRtcpAppReqToSendPacket& packet)
{
    Status ret = ProcessRemoteAssert(packet);
    if (ret != TIP_OK) {
        return ret;
    }
    
    // transition to the remote state
    ChangeState(&gRemotePresState);
    
    return TIP_OK;
}

Status CTipPresImpl::OnRemoteAssertUpdate(CRtcpAppReqToSendPacket& packet)
{
    AMDEBUG(TIPNEG, ("old offer 0x%x new offer 0x%x",
                     mRemoteAssertVideoPos, packet.GetVideoPos()));
            
    // check to see if the offer has changed
    if (mRemoteAssertVideoPos == packet.GetVideoPos()) {
        // no change so nothing to do
        return TIP_OK;
    }

    // something in the offer has changed, stop the current
    // presentation and re-evaluate what we want
    mpImpl->mpCallback->RemotePresentationStop();
    
    Status ret = ProcessRemoteAssert(packet);
    if (ret != TIP_OK) {
        return ret;
    }
    
    // transition to the remote state
    ChangeState(&gRemotePresState);
    
    return TIP_OK;
}

Status CTipPresImpl::OnLocalRelease()
{
    Status ret = ProcessLocalRelease();
    if (ret != TIP_OK) {
        return ret;
    }

    // transition to the no presentation state
    ChangeState(&gNoPresState);
    
    return TIP_OK;
}

Status CTipPresImpl::OnLocalReleaseOverride()
{
    Status ret = ProcessLocalRelease();
    if (ret != TIP_OK) {
        return ret;
    }

    // transition to the remote state
    ChangeState(&gRemotePresState);
    
    return TIP_OK;
}

Status CTipPresImpl::OnConflictRemoteAssert(CRtcpAppReqToSendPacket& packet)
{
    bool havewinner = false;
    bool wewin = false;

    // for V7 look for REQTOSEND, earlier look for MUXCTRL
    TipPacketType ptype = REQTOSEND;
    if (mpImpl->mSystem.GetTipVersion() < TIP_V7) {
        ptype = MUXCTRL;
    }
    
    CRtcpTipPacket* lpkt = mpImpl->mPacketManager[VIDEO].Find(ptype);
    if (lpkt == NULL) {
        // this shouldn't happen
        AMDEBUG(INTERR, ("error in presentation logic, cannot find pending REQTOSEND"));
        return TIP_ERROR;
    }

    // we are trying to assert control and at the same time the remote
    // is trying to assert control.  for V7 and later determine winner
    // based on...
    //
    // 1.  if we are not an MCU and the remote is, then we win
    // 2.  if the ntp timestamp in our REQTOSEND is smaller then we win
    // 3.  if our ssrc is smaller then we win
    // 4.  ask the user what to do
    //
    // the inverse of each rule applies as well to determine if we
    // are the loser

    // for V6 only apply rule #4

    if (mpImpl->mSystem.GetTipVersion() >= TIP_V7) {
    
        // rule #1
        if (mpImpl->mSystem.GetMCUState() == false) {
            if (mpImpl->mRemoteSystem.GetMCUState() == true) {
                AMDEBUG(TIPNEG,
                        ("conflict resolved we win, they are MCU and we are not"));
                havewinner = true;
                wewin = true;
        }

        } else {
            // inverse of rule #1
            if (mpImpl->mRemoteSystem.GetMCUState() == false) {
                AMDEBUG(TIPNEG,
                        ("conflict resolved they win, we are MCU and they are not"));
                havewinner = true;
                wewin = false;
            }
        }
        
        if (!havewinner) {
            // rule #2
            if (lpkt->GetNtpTime() < packet.GetNtpTime()) {
                AMDEBUG(TIPNEG, ("conflict resolved we win, our ntp time is smaller"));
                havewinner = true;
                wewin = true;
                
            } else if (lpkt->GetNtpTime() > packet.GetNtpTime()) {
                // inverse of rule #2
                AMDEBUG(TIPNEG, ("conflict resolved they win, our ntp time is larger"));
                havewinner = true;
                wewin = false;
            }
        }
        
        if (!havewinner) {
            // rule #3
            if (lpkt->GetSSRC() < packet.GetSSRC()) {
                AMDEBUG(TIPNEG, ("conflict resolved we win, our ssrc is smaller"));
                havewinner = true;
                wewin = true;
                
            } else if (lpkt->GetSSRC() > packet.GetSSRC()) {
                // inverse of rule #3
                AMDEBUG(TIPNEG, ("conflict resolved they win, our ssrc is larger"));
                havewinner = true;
                wewin = false;
            }
        }
    }

    if (!havewinner) {
        // rule 4 is common for all versions
        if (mpImpl->mpCallback->ResolvePresentationConflict()) {
            AMDEBUG(TIPNEG, ("conflict resolved we win, user callback"));
            havewinner = true;
            wewin = true;
        } else {
            AMDEBUG(TIPNEG, ("conflict resolved they win, user callback"));
            havewinner = true;
            wewin = false;
        }
    }

    AMDEBUG(TIPNEG, ("presentation conflict resolved, winner is %s.  mcu state %s %s.  "
                     "ntp timestamp %llu %llu.  ssrc %x %x",
                     (wewin ? "local" : "remote"),
                     (mpImpl->mSystem.GetMCUState() ? "true" : "false"),
                     (mpImpl->mRemoteSystem.GetMCUState() ? "true" : "false"),
                     lpkt->GetNtpTime(), packet.GetNtpTime(),
                     lpkt->GetSSRC(), packet.GetSSRC()));

    // if we are the winner, we must wait for the ACK to proceed
    if (wewin) {
        return TIP_OK;
    }

    // if they are the winner then stop sending our assert packets
    mpImpl->StopPacketTx(ptype, VIDEO);
    
    // if they are the winner then we need to process and ACK
    Status ret = ProcessRemoteAssert(packet);
    if (ret != TIP_OK) {
        return ret;
    }
    
    // transition to the remote override state
    ChangeState(&gOverridePresState);

    return TIP_OK;
}

Status CTipPresImpl::OnRemoteOverride(CRtcpAppReqToSendPacket& packet)
{
    // we have control but the remote is taking over.  stop local
    // presentation first.
    mpImpl->mpCallback->LocalPresentationStop();

    // now enable remote presentation
    Status ret = ProcessRemoteAssert(packet);
    if (ret != TIP_OK) {
        return ret;
    }

    // transition to the override state
    ChangeState(&gOverridePresState);

    return TIP_OK;
}

Status CTipPresImpl::OnRemoteRelease(CRtcpAppReqToSendPacket& packet)
{
    // tell user to stop showing remote presentation
    mpImpl->mpCallback->RemotePresentationStop();

    // ack release packet
    TransmitAck();
    
    // change state to no presentation
    ChangeState(&gNoPresState);

    // clear out old data
    mRemoteAssertVideoPos = 0;
    mRemoteAssertAckedVideoPos = 0;
    
    return TIP_OK;
}

Status CTipPresImpl::OnRemoteReleaseLocalAssert(CRtcpAppReqToSendPacket& packet)
{
    // this function is invoked when...
    // 1.  we have presentation to share
    // 2.  but the remote has control
    // 3.  and the remote is giving up control
    //
    // so first we stop the remote presentation, then we re-assert
    // control.
    mpImpl->mpCallback->RemotePresentationStop();

    // ack release packet
    TransmitAck();

    // clear out old data
    mRemoteAssertVideoPos = 0;
    mRemoteAssertAckedVideoPos = 0;
    
    // for V6 we can start sending right away, for V7 we need to
    // re-request permission.
    if (mpImpl->mSystem.GetTipVersion() < TIP_V7) {
        CRtcpAppReqToSendAckPacket ack(packet);
        return OnLocalAssertAck(ack);
    }
    
    return OnLocalAssert();
}

Status CTipPresImpl::OnOptionUpdateLocalAssert(CTipSystem::PresentationFrameRate oldRate)
{
    CTipSystem::PresentationFrameRate newRate = mpImpl->mNegotiatedSystem.GetPresentationFrameRate();
    AMDEBUG(TIPNEG, ("old rate %u new rate %u", oldRate, newRate));

    // we are trying to get control of presentation and the options
    // may have changed.
    if (oldRate == newRate) {
        // rate has not changed, nothing to do
        return TIP_OK;
    }

    // something has changed, abort the current assert and start a new
    // one
    return OnLocalAssert();
}

Status CTipPresImpl::OnOptionUpdateLocal(CTipSystem::PresentationFrameRate oldRate)
{
    CTipSystem::PresentationFrameRate newRate = mpImpl->mNegotiatedSystem.GetPresentationFrameRate();
    AMDEBUG(TIPNEG, ("old rate %u new rate %u", oldRate, newRate));

    // we have control of presentation and the options have changed
    if (oldRate == newRate) {
        // rate has not changed, nothing to do
        return TIP_OK;
    }

    // in V6 mode we just need to start transmitting the new rate
    if (mpImpl->mSystem.GetTipVersion() < TIP_V7) {

        // stop the old rate
        mpImpl->mpCallback->LocalPresentationStop();

        // start the correct new rate
        CRtcpAppReqToSendAckPacket ack;
        ack.SetVideoPos(PositionToMask(POS_VIDEO_AUX_1_5FPS));

        return OnLocalAssertAck(ack);
    }

    // in V7 mode we need to send a new RTS any time a new position is
    // required or available.  if only the frame rate of an enabled
    // position is changing then we can just start transmitting the
    // new rate.  in either case, the first step is to stop the
    // current transmission.
    mpImpl->mpCallback->LocalPresentationStop();

    // per this implementation, if the local system is capable of
    // generating multiple presentation streams then it will always
    // offer to do so regardless of the remote system's restrictions.
    // so we don't need to handle upgrades from a single stream to
    // multiple streams or from multiple streams to single streams.
    
    bool needRTS = false;

    switch (oldRate) {
    case CTipSystem::PRES_1FPS_ONLY:
    case CTipSystem::PRES_5FPS_ONLY:
        // only need a new RTS if changing to 30FPS
        if (newRate == CTipSystem::PRES_30FPS_ONLY) {
            needRTS = true;
        }
        break;

    case CTipSystem::PRES_30FPS_ONLY:
        // need a new RTS if changing to 1 or 5FPS which should be
        // every time in this case
        if (newRate == CTipSystem::PRES_1FPS_ONLY || newRate == CTipSystem::PRES_5FPS_ONLY) {
            needRTS = true;
        }
        break;

    default:
        // all other modes are just rate changes not position changes
        // so a new RTS is not needed.
        needRTS = false;
        break;
    }

    if (needRTS) {
        // we need to send a new RTS to advertise the new positions,
        // so re-assert control
        return OnLocalAssert();
    }

    // we just need to change the rates but we do not need to
    // re-assert control
    uint16_t lVideoPos;
    mpImpl->mNegotiatedSystem.MapToActiveSharedPos(VIDEO, lVideoPos);

    CRtcpAppReqToSendAckPacket ack;
    ack.SetVideoPos(lVideoPos);

    return OnLocalAssertAck(ack);
}

Status CTipPresImpl::OnOptionUpdateRemote(CTipSystem::PresentationFrameRate oldRate)
{
    // remote side has control and is changing presentation mode
    CTipSystem::PresentationFrameRate newRate = mpImpl->mNegotiatedSystem.GetPresentationFrameRate();
    AMDEBUG(TIPNEG, ("old rate %u new rate %u", oldRate, newRate));

    if (oldRate == newRate) {
        // no change, nothing to do
        return TIP_OK;
    }

    if (mpImpl->mSystem.GetTipVersion() < TIP_V7) {
        // for V6 as there is only a single presentatin position we
        // simply stop presentation at the old rate and start it at
        // the new rate.
        mpImpl->mpCallback->RemotePresentationStop();

        CTipSystem::PresentationStreamFrameRate rate =
            mpImpl->mNegotiatedSystem.MapPositionToFrameRate(POS_VIDEO_AUX_1_5FPS);
        mpImpl->mpCallback->RemotePresentationStart(POS_VIDEO_AUX_1_5FPS, rate, NULL);

        return TIP_OK;
    }

    // for V7, the only presentation option that can be changed
    // without a new RTS is changing from 5 to 1 FPS.  so that is all
    // we handle here.
    if (mRemoteAssertAckedVideoPos & PositionToMask(POS_VIDEO_AUX_1_5FPS)) {

        // we are currently receiving something on the 1/5 position
        if (((oldRate & CTipSystem::PRES_1FPS_RATE) && (newRate & CTipSystem::PRES_5FPS_RATE)) ||
            ((oldRate & CTipSystem::PRES_5FPS_RATE) && (newRate & CTipSystem::PRES_1FPS_RATE))) {

            // the 1/5 position frame rate has changed, stop all
            // presentation receivers.
            mpImpl->mpCallback->RemotePresentationStop();

            // restart the 1/5 position with the new rate
            CTipSystem::PresentationStreamFrameRate rate =
                mpImpl->mNegotiatedSystem.MapPositionToFrameRate(POS_VIDEO_AUX_1_5FPS);
            mpImpl->mpCallback->RemotePresentationStart(POS_VIDEO_AUX_1_5FPS, rate, NULL);

            // also restart the 30 position if it was active
            if (mRemoteAssertAckedVideoPos & PositionToMask(POS_VIDEO_AUX_30FPS)) {
                mpImpl->mpCallback->RemotePresentationStart(POS_VIDEO_AUX_30FPS,
                                                            CTipSystem::PRES_30FPS_RATE, NULL);
            }
        }
    }

    return TIP_OK;
}

Status CTipPresImpl::ProcessLocalRelease()
{
    // we are releasing control over presentation, for V6 send out a
    // MUXCTRL, for V7 send out a REQTOSEND packet.
    if (mpImpl->mSystem.GetTipVersion() < TIP_V7) {

        // remove presentation transmitter
        mpImpl->mSystem.RemoveTransmitter(VIDEO, POS_VIDEO_AUX_1_5FPS);

        // get MUXCTRL packet, VIDEO only
        CRtcpAppMuxCtrlPacketBase* packet = mpImpl->mSystem.MapToMuxCtrl(VIDEO);
        if (packet == NULL) {
            AMDEBUG(INTERR, ("error creating VIDEO MUXCTRL packet"));
            return TIP_ERROR;
        }

        // always VIDEO only
        mpImpl->StartPacketTx(packet, VIDEO);

    } else {
        CRtcpAppReqToSendPacket* packet = mpImpl->mNegotiatedSystem.MapToReqToSend();
        if (packet == NULL) {
            AMDEBUG(INTERR, ("error creating REQTOSEND packet"));
            return TIP_ERROR;
        }
        
        // always send REQTOSEND on video only
        mpImpl->StartPacketTx(packet, VIDEO);
    }

    return TIP_OK;
}


Status CTipPresImpl::ProcessRemoteAssert(CRtcpAppReqToSendPacket& packet)
{
    // save off the remote video positions.  we may need these later
    // if we get an update.
    mRemoteAssertVideoPos = packet.GetVideoPos();

    // note that we use all available shared positions here not just
    // the active ones.
    uint16_t lVideoPos;
    mpImpl->mSystem.MapToMuxCtrlSharedPos(VIDEO, lVideoPos);

    // figure out which streams should be active.
    mRemoteAssertAckedVideoPos = (lVideoPos & mRemoteAssertVideoPos);

    if (mRemoteAssertAckedVideoPos == 0) {
        // no positions in common, shouldn't happen but possible
        AMDEBUG(TIPNEG, ("error in remote REQTOSEND, no common positions.  "
                         "video local = 0x%hx, remote = 0x%hx.",
                         lVideoPos, mRemoteAssertVideoPos));
        return TIP_ERROR;
    }

    // an endpoint can only receive a single stream at a time, so we
    // will prioritize the 30 over the 5.  an MCU can receive both so
    // ACK everything.
    if (mpImpl->mSystem.GetMCUState() == false) {
        if (mRemoteAssertAckedVideoPos & PositionToMask(POS_VIDEO_AUX_30FPS)) {
            // no harm if its already 0
            mRemoteAssertAckedVideoPos &= ~PositionToMask(POS_VIDEO_AUX_1_5FPS);
        }
    }
    
    bool doAck = true;
    
    // invoke callback once for each set position
    const uint8_t pos[] = { POS_VIDEO_AUX_1_5FPS, POS_VIDEO_AUX_30FPS };
    for (uint32_t i = 0; i < sizeof(pos); i++) {
    
        if (mRemoteAssertAckedVideoPos & PositionToMask(pos[i])) {
            CTipSystem::PresentationStreamFrameRate rate =
                mpImpl->mNegotiatedSystem.MapPositionToFrameRate(pos[i]);
            
            AMDEBUG(TIPNEG, ("invoking RemotePresentationStart for position %d rate %d",
                             pos[i], rate));
        
            doAck &= mpImpl->mpCallback->RemotePresentationStart(pos[i], rate,
                                                                 mpPacketToAck);
        }
    }
    
    if (doAck) {
        // in the common TransmitAck method the ack is built from the
        // original REQTOSEND packet.  so adjust that one with the
        // common positions before ack'ing.
        packet.SetVideoPos(mRemoteAssertAckedVideoPos);

        TransmitAck();
        
    } else {
        // if not ack'ing forget about the packet we were supposed to
        // ack.  user callback must remember for us.
        mpPacketToAck = NULL;
    }

    return TIP_OK;
}

void CTipPresImpl::ChangeState(CPresState* newState)
{
    AMDEBUG(TIPNEG, ("presentation state change from %s to %s",
                     mpState->GetName(), newState->GetName()));
    mpState = newState;
}

void CTipPresImpl::TransmitAck()
{
    if (mpPacketToAck == NULL) {
        AMDEBUG(INTERR, ("ERROR attempt to ACK but no pending packet!"));
        return;
    }
    
    CRtcpTipPacket* ack = CRtcpPacketFactory::CreateAckPacket(*mpPacketToAck);
    mpImpl->TransmitAck(ack, mpPacketToAck, mPacketToAckMediaType);
    mpImpl->PrintPacket(ack, mPacketToAckMediaType, false);
    
    mpPacketToAck = NULL;
}

