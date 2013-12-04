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
#include "private/map_tip_system.h"

using namespace LibTip;

CMapTipSystem::CMapTipSystem()
{

}

CMapTipSystem::~CMapTipSystem()
{

}

CRtcpAppMuxCtrlPacketBase* CMapTipSystem::MapToMuxCtrl(MediaType type) const
{
    if (type >= MT_MAX) {
        return NULL;
    }

    CRtcpAppMuxCtrlPacketBase* packet = NULL;
    CRtcpAppMuxCtrlV7Packet* v7packet = NULL;

    if (mVersion >= TIP_V7) {
        v7packet = new CRtcpAppMuxCtrlV7Packet();
        packet = v7packet;
    } else {
        packet = new CRtcpAppMuxCtrlPacket();
    }
    
    if (packet == NULL) {
        return NULL;
    }

    uint8_t profile = CRtcpAppMuxCtrlPacketBase::AVP;
    if (mSecurityState) {
        profile |= CRtcpAppMuxCtrlPacketBase::SECURE_PROFILE;
    }
    if (type == VIDEO && mFeedbackState) {
        profile |= CRtcpAppMuxCtrlPacketBase::FEEDBACK_PROFILE;
    }
    packet->SetProfile(profile);

    if (mMCUState) {
        packet->SetOptions(CRtcpAppMuxCtrlPacketBase::IS_FOCUS);
    } else {
        packet->SetOptions(0);
    }
    
    packet->SetNumXmit(mTxPos[type].count());
    packet->SetNumRcv(mRxPos[type].count());

    packet->SetConfID(mConfID);

    packet->SetXmitPositions(mTxPos[type].to_ulong());
    packet->SetRcvPositions(mRxPos[type].to_ulong());

    if (v7packet != NULL) {

        uint16_t shpos;
        
        v7packet->SetNumShared(MapToMuxCtrlSharedPos(type, shpos));
        v7packet->SetSharedPositions(shpos);
    
        if (mPartIDLen > 0) {
            v7packet->SetParticipantID(mPartID, mPartIDLen);
        }
        
    } else {

        // we are in V6 mode, no shared positions or participant id.
        
        // for video, we will set the aux position receive bit
        // automatically based on whether we are an MCU or not.  for
        // audio, there is no change.
        if (type == VIDEO && ! mMCUState) {
            packet->SetNumRcv((packet->GetNumRcv() + 1));
            packet->SetRcvPositions((packet->GetRcvPositions() | (1 << POS_VIDEO_AUX_1_5FPS)));
        }
    }

    return packet;
}

CRtcpAppMediaoptsPacket* CMapTipSystem::MapToMediaOpts(MediaType type) const
{
    if (type >= MT_MAX) {
        return NULL;
    }

    CRtcpAppMediaoptsPacket::Version version = CRtcpAppMediaoptsPacket::MAXIMUM_VERSION;
    if (mVersion < TIP_V7) {
        version = CRtcpAppMediaoptsPacket::MINIMUM_VERSION;
    }
    
    CRtcpAppMediaoptsPacket* packet = new CRtcpAppMediaoptsPacket(version);
    if (packet == NULL) {
        return NULL;
    }

    // build up the default media options
    uint32_t txOpt = 0;
    uint32_t rxOpt = 0;
    
    MediaOptionMap::const_iterator i;
    for (i = mOptionMap[type].begin(); i != mOptionMap[type].end(); i++) {

        if (type == VIDEO) {
            MapVideoOption((*i).second, txOpt, rxOpt);
        } else {
            MapAudioOption((*i).second, txOpt, rxOpt);
        }
    }

    if (type == VIDEO) {

        uint32_t presOpt = 0;

        // for V6 only a single frame rate is supported and MO
        // declares that rate.  for V7 the MO bits indicate the
        // minimum required frame rate.
        switch (mPresFrameRate) {
        case CTipSystem::PRES_1FPS_ONLY:
            presOpt = CRtcpAppMediaoptsPacket::GetBitValueForAuxFrameRate(CRtcpAppMediaoptsPacket::AUX_1FPS);
            break;

        case CTipSystem::PRES_5FPS_ONLY:
            presOpt = CRtcpAppMediaoptsPacket::GetBitValueForAuxFrameRate(CRtcpAppMediaoptsPacket::AUX_5FPS);
            break;

        default:
            presOpt = CRtcpAppMediaoptsPacket::GetBitValueForAuxFrameRate(CRtcpAppMediaoptsPacket::AUX_30FPS);
            break;
        }

        txOpt |= presOpt;
        rxOpt |= presOpt;
        AMDEBUG(TIPNEG, ("setting pres option to %x %x", presOpt, txOpt));
    }
    
    // add default options
    packet->AddSSRC(0, txOpt, rxOpt);

    return packet;
}

CRtcpAppReqToSendPacket* CMapTipSystem::MapToReqToSend() const
{
    CRtcpAppReqToSendPacket* packet = new CRtcpAppReqToSendPacket();
    if (packet == NULL) {
        return NULL;
    }

    uint16_t pos = 0;
    MapToActiveSharedPos(VIDEO, pos);
    packet->SetVideoPos(pos);

    return packet;
}

Status CMapTipSystem::MapFromMuxCtrl(CRtcpAppMuxCtrlPacketBase& packet, MediaType type)
{
    if (type >= MT_MAX) {
        return TIP_ERROR;
    }

    uint8_t version = packet.GetVersion();
    CRtcpAppMuxCtrlV7Packet* v7packet = NULL;
    
    if (version >= CRtcpAppMuxCtrlV7Packet::DEFAULT_VERSION) {
        mVersion = TIP_V7;
        v7packet = dynamic_cast<CRtcpAppMuxCtrlV7Packet*>(&packet);

        if (v7packet == NULL) {
            AMDEBUG(INTERR, ("recv packet with V7 muxctrl type but not V7 MUXCTRL object"));
            return TIP_ERROR;
        }

    } else {
        mVersion = TIP_V6;
    }
    
    if (packet.GetProfile() & CRtcpAppMuxCtrlPacketBase::SECURE_PROFILE) {
        mSecurityState = true;
    } else {
        mSecurityState = false;
    }
    
    if ((packet.GetOptions() & CRtcpAppMuxCtrlPacketBase::IS_FOCUS)) {
        mMCUState = true;
    } else {
        mMCUState = false;
    }

    // only update feedback state for video
    if (type == VIDEO) {
        if (packet.GetProfile() & CRtcpAppMuxCtrlPacketBase::FEEDBACK_PROFILE) {
            mFeedbackState = true;
        } else {
            mFeedbackState = false;
        }
    }
    
    mConfID = packet.GetConfID();
    
    mTxPos[type] = CPosBitset(packet.GetXmitPositions());
    if (mTxPos[type].count() != packet.GetNumXmit()) {
        AMDEBUG(TIPNEG, ("WARNING numXmit (%hu) does not match # of xmit positions (0x%hx)",
                         packet.GetNumXmit(), packet.GetXmitPositions()));
    }
    
    mRxPos[type] = CPosBitset(packet.GetRcvPositions());
    if (mRxPos[type].count() != packet.GetNumRcv()) {
        AMDEBUG(TIPNEG, ("WARNING numRcv (%hu) does not match # of rcv positions (0x%hx)",
                         packet.GetNumRcv(), packet.GetRcvPositions()));
    }

    if (v7packet != NULL) {

        mPartIDLen = v7packet->GetParticipantID(mPartID);
        
    } else {

        mPartIDLen = 0;
    }

    return TIP_OK;
}

Status CMapTipSystem::MapFromMediaOpts(CRtcpAppMediaoptsPacket& packet, MediaType type)
{
    // NOTE:  only handling default SSRC for now
    
    // build up media option list
    uint32_t txOpt;
    uint32_t rxOpt;

    if (packet.GetSSRC(0, txOpt, rxOpt) != TIP_OK) {
        AMDEBUG(TIPNEG, ("no default SSRC found in packet"));
        return TIP_ERROR;
    }

    if (type == VIDEO) {
        AddVideoOption(CRtcpAppMediaoptsPacket::REFRESH_FLAG, txOpt, rxOpt);
        AddVideoOption(CRtcpAppMediaoptsPacket::INBAND_PARAM_SETS, txOpt, rxOpt);
        AddVideoOption(CRtcpAppMediaoptsPacket::CABAC, txOpt, rxOpt);
        AddVideoOption(CRtcpAppMediaoptsPacket::LTRP, txOpt, rxOpt);
        AddVideoOption(CRtcpAppMediaoptsPacket::GDR, txOpt, rxOpt);
        AddVideoOption(CRtcpAppMediaoptsPacket::HP_8X8_TRANSFORMS, txOpt, rxOpt);
        AddVideoOption(CRtcpAppMediaoptsPacket::UNRESTRICTED_XGA_1_5, txOpt, rxOpt);
        AddVideoOption(CRtcpAppMediaoptsPacket::UNRESTRICTED_720P, txOpt, rxOpt);
        AddVideoOption(CRtcpAppMediaoptsPacket::UNRESTRICTED_1080P, txOpt, rxOpt);
        AddVideoOption(CRtcpAppMediaoptsPacket::UNRESTRICED_XGA_30, txOpt, rxOpt);
        AddVideoOption(CRtcpAppMediaoptsPacket::EKT, txOpt, rxOpt);

        // we only track the minimum required frame rate from the MO
        // packet.  we do not track whether this system is capable of
        // multi-streaming or not.  this is common between V6 and V7.

        CRtcpAppMediaoptsPacket::AuxFrameRate rate = CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(txOpt);

        if (rate == CRtcpAppMediaoptsPacket::AUX_1FPS) {
            mPresFrameRate = PRES_1FPS_ONLY;
        } else if (rate == CRtcpAppMediaoptsPacket::AUX_5FPS) {
            mPresFrameRate = PRES_5FPS_ONLY;
        } else {
            mPresFrameRate = PRES_30FPS_ONLY;
        }
        
        AMDEBUG(TIPNEG, ("pres neg version %hu rate %u pres %u opt %x",
                         mVersion, rate, mPresFrameRate, txOpt));
        
    } else {
        AddAudioOption(CRtcpAppMediaoptsPacket::ACTIVITY_METRIC, txOpt, rxOpt);
        AddAudioOption(CRtcpAppMediaoptsPacket::DYNAMIC_OUTPUT, txOpt, rxOpt);
        AddAudioOption(CRtcpAppMediaoptsPacket::CAPABLE_G722_LEGACY, txOpt, rxOpt);
        AddAudioOption(CRtcpAppMediaoptsPacket::USING_G722_LEGACY, txOpt, rxOpt);
        AddAudioOption(CRtcpAppMediaoptsPacket::EKT, txOpt, rxOpt);
    }

    // NOTE:  need to handle options section of the packet

    return TIP_OK;
}

void CMapTipSystem::NegotiateLocalRemote(const CTipSystem& local,
                                         const CTipSystem& remote)
{
    MediaType mType;
    
    // map all members of the system objects finding the common
    // elements and/or least common denominator for each

    // version, lowest wins
    mVersion = std::min(local.GetTipVersion(), remote.GetTipVersion());

    // transmitter positions, logical and of local transmitters and
    // remote receivers.
    for (mType = VIDEO; mType < MT_MAX; mType++) {
        mTxPos[mType] = (local.GetTransmitters(mType) & remote.GetReceivers(mType));
    }

    // receiver positions, logical and of local receivers and remote
    // transmitters.
    for (mType = VIDEO; mType < MT_MAX; mType++) {
        mRxPos[mType] = (local.GetReceivers(mType) & remote.GetTransmitters(mType));
    }

    // presentation mode.  support the best mode we can while keeping
    // the remote side happy.  if local supports multiple rates then
    // make sure one of those rates matches the remote requested rate
    // (note that 1FPS_AND_30FPS is not a valid local rate but it is a
    // valid negotiated rate).  if local does not support multiple
    // rates then the transmitted rate is the lower of the two rates.
    PresentationFrameRate lRate = local.GetPresentationFrameRate();
    PresentationFrameRate rRate = remote.GetPresentationFrameRate();

    if (lRate == PRES_5FPS_AND_30FPS) {
        if (rRate == PRES_1FPS_ONLY) {
            // if the remote wants 1FPS, then send 1 and 30
            mPresFrameRate = PRES_1FPS_AND_30FPS;
        } else {
            // if the remote wants either 5 or 30, then send 5 and 30
            mPresFrameRate = PRES_5FPS_AND_30FPS;
        }
        
    } else {
        // we can only send a single rate, so send the best rate we
        // can that keeps both sides happy.
        mPresFrameRate = std::min(lRate, rRate);
    }
    
    // conf id is a copy of the local conf id
    mConfID = local.GetConfID();

    // participant id is a copy of the local participant id
    memcpy(mPartID, local.GetParticipantID(), local.GetParticipantIDLength());
    mPartIDLen = local.GetParticipantIDLength();

    // Security is only enabled when both are secure
    mSecurityState = (local.GetSecurityState() && remote.GetSecurityState());

    // MCU states is a copy of the local MCU state
    mMCUState = local.GetMCUState();

    // Feedback is only enabled when both sides enable
    mFeedbackState = (local.GetFeedbackState() && remote.GetFeedbackState());

    // process audio and video options, note that we only process
    // options that this version of the library knows about.  any
    // other options are silently ignored.
    NegotiateOption(CTipVideoMediaOption::REFRESH_FLAG, local, remote);
    NegotiateOption(CTipVideoMediaOption::INBAND_PARAM_SETS, local, remote);
    NegotiateOption(CTipVideoMediaOption::CABAC, local, remote);
    NegotiateOption(CTipVideoMediaOption::LTRP, local, remote);
    NegotiateOption(CTipVideoMediaOption::GDR, local, remote);
    NegotiateOption(CTipVideoMediaOption::HP_8X8_TRANSFORMS, local, remote);
    NegotiateOption(CTipVideoMediaOption::UNRESTRICTED_XGA_1_5, local, remote);
    NegotiateOption(CTipVideoMediaOption::UNRESTRICTED_720P, local, remote);
    NegotiateOption(CTipVideoMediaOption::UNRESTRICTED_1080P, local, remote);
    NegotiateOption(CTipVideoMediaOption::UNRESTRICED_XGA_30, local, remote);
    NegotiateOption(CTipVideoMediaOption::EKT, local, remote);

    NegotiateOption(CTipAudioMediaOption::ACTIVITY_METRIC, local, remote);
    NegotiateOption(CTipAudioMediaOption::DYNAMIC_OUTPUT, local, remote);
    NegotiateOption(CTipAudioMediaOption::CAPABLE_G722_LEGACY, local, remote);
    NegotiateOption(CTipAudioMediaOption::EKT, local, remote);

    // USING_G722_LEGACY has special semantics.  our local state does
    // not affect this, only the rx/tx state of the remote side
    // controls the option.
    NegotiateOption(AUDIO, CTipAudioMediaOption::USING_G722_LEGACY,
                    CTipMediaOption::OPTION_SUPPORTED_BOTH,
                    remote.GetMediaOptionState(CTipAudioMediaOption::USING_G722_LEGACY));
}

uint8_t CMapTipSystem::MapToMuxCtrlSharedPos(MediaType type, uint16_t& shpos) const
{
    uint8_t ret = 0;
    shpos = 0;
    
    if (type != VIDEO) {
        return 0;
    }

    if (mPresFrameRate == PRES_1FPS_ONLY || mPresFrameRate == PRES_5FPS_ONLY) {
        ret = 1;
        shpos = PositionToMask(POS_VIDEO_AUX_1_5FPS);
    } else if (mPresFrameRate == PRES_30FPS_ONLY) {
        ret = 1;
        shpos = (PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS));
    } else {
        ret = 2;
        shpos = (PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS));
    }

    return ret;
}

void CMapTipSystem::MapToActiveSharedPos(MediaType type, uint16_t& shpos) const
{
    shpos = 0;
    
    if (type != VIDEO) {
        return;
    }

    if (mPresFrameRate == PRES_1FPS_ONLY || mPresFrameRate == PRES_5FPS_ONLY) {
        shpos = PositionToMask(POS_VIDEO_AUX_1_5FPS);
    } else if (mPresFrameRate == PRES_30FPS_ONLY) {
        if (mVersion >= TIP_V7) {
            shpos = PositionToMask(POS_VIDEO_AUX_30FPS);
        } else {
            shpos = PositionToMask(POS_VIDEO_AUX_1_5FPS);
        }
    } else {
        shpos = (PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS));
    }
}

CMapTipSystem::PresentationStreamFrameRate
CMapTipSystem::MapPositionToFrameRate(uint8_t position) const
{
    // default to invalid
    PresentationStreamFrameRate rate = PRES_INVALID_RATE;
        
    if (position == POS_VIDEO_AUX_1_5FPS) {

        switch (mPresFrameRate) {
        case PRES_1FPS_ONLY:
            rate = PRES_1FPS_RATE;
            break;

        case PRES_5FPS_ONLY:
            rate = PRES_5FPS_RATE;
            break;

        case PRES_30FPS_ONLY:
            if (mVersion < TIP_V7) {
                rate = PRES_30FPS_RATE;
            } else {
                rate = PRES_5FPS_RATE;
            }
            break;
            
        case PRES_1FPS_AND_30FPS:
            rate = PRES_1FPS_RATE;
            break;

        case PRES_5FPS_AND_30FPS:
            rate = PRES_5FPS_RATE;
            break;
        }

    } else if (position == POS_VIDEO_AUX_30FPS) {
        // this position is only supported in v7
        if (mVersion >= TIP_V7) {
            rate = PRES_30FPS_RATE;
        }
    }

    return rate;
}

void CMapTipSystem::MapVideoOption(const CTipMediaOption& opt, uint32_t& tx, uint32_t& rx) const
{
    uint32_t bitVal = 0;
    
    switch (opt.GetOption()) {
    case CTipVideoMediaOption::REFRESH_FLAG:
        bitVal = CRtcpAppMediaoptsPacket::REFRESH_FLAG;
        break;
        
    case CTipVideoMediaOption::INBAND_PARAM_SETS:
        bitVal = CRtcpAppMediaoptsPacket::INBAND_PARAM_SETS;
        break;
        
    case CTipVideoMediaOption::CABAC:
        bitVal = CRtcpAppMediaoptsPacket::CABAC;
        break;
        
    case CTipVideoMediaOption::LTRP:
        bitVal = CRtcpAppMediaoptsPacket::LTRP;
        break;
        
    case CTipVideoMediaOption::GDR:
        bitVal = CRtcpAppMediaoptsPacket::GDR;
        break;
        
    case CTipVideoMediaOption::HP_8X8_TRANSFORMS:
        bitVal = CRtcpAppMediaoptsPacket::HP_8X8_TRANSFORMS;
        break;
        
    case CTipVideoMediaOption::UNRESTRICTED_XGA_1_5:
        bitVal = CRtcpAppMediaoptsPacket::UNRESTRICTED_XGA_1_5;
        break;
        
    case CTipVideoMediaOption::UNRESTRICTED_720P:
        bitVal = CRtcpAppMediaoptsPacket::UNRESTRICTED_720P;
        break;
        
    case CTipVideoMediaOption::UNRESTRICTED_1080P:
        bitVal = CRtcpAppMediaoptsPacket::UNRESTRICTED_1080P;
        break;
        
    case CTipVideoMediaOption::UNRESTRICED_XGA_30:
        bitVal = CRtcpAppMediaoptsPacket::UNRESTRICED_XGA_30;
        break;
        
    case CTipVideoMediaOption::EKT:
        bitVal = CRtcpAppMediaoptsPacket::EKT;
        break;

    default:
        break;
    }

    if (bitVal != 0) {
        if (opt.GetState() & CTipMediaOption::OPTION_SUPPORTED_TX) {
            tx |= bitVal;
        } else {
            tx &= ~bitVal;
        }

        if (opt.GetState() & CTipMediaOption::OPTION_SUPPORTED_RX) {
            rx |= bitVal;
        } else {
            rx &= ~bitVal;
        }
    }
}

void CMapTipSystem::MapAudioOption(const CTipMediaOption& opt, uint32_t& tx, uint32_t& rx) const
{
    uint32_t bitVal = 0;
    
    switch (opt.GetOption()) {
    case CTipAudioMediaOption::ACTIVITY_METRIC:
        bitVal = CRtcpAppMediaoptsPacket::ACTIVITY_METRIC;
        break;
        
    case CTipAudioMediaOption::DYNAMIC_OUTPUT:
        bitVal = CRtcpAppMediaoptsPacket::DYNAMIC_OUTPUT;
        break;
        
    case CTipAudioMediaOption::CAPABLE_G722_LEGACY:
        bitVal = CRtcpAppMediaoptsPacket::CAPABLE_G722_LEGACY;
        break;
        
    case CTipAudioMediaOption::USING_G722_LEGACY:
        bitVal = CRtcpAppMediaoptsPacket::USING_G722_LEGACY;
        break;
        
    case CTipAudioMediaOption::EKT:
        bitVal = CRtcpAppMediaoptsPacket::EKT;
        break;

    default:
        break;
    }

    if (bitVal != 0) {
        if (opt.GetState() & CTipMediaOption::OPTION_SUPPORTED_TX) {
            tx |= bitVal;
        } else {
            tx &= ~bitVal;
        }

        if (opt.GetState() & CTipMediaOption::OPTION_SUPPORTED_RX) {
            rx |= bitVal;
        } else {
            rx &= ~bitVal;
        }
    }
}

void CMapTipSystem::AddVideoOption(CRtcpAppMediaoptsPacket::VideoOption opt, uint32_t tx, uint32_t rx)
{
    CTipVideoMediaOption::VideoOption vmoOpt;
    
    switch (opt) {
    case CRtcpAppMediaoptsPacket::REFRESH_FLAG:
        vmoOpt = CTipVideoMediaOption::REFRESH_FLAG;
        break;
        
    case CRtcpAppMediaoptsPacket::INBAND_PARAM_SETS:
        vmoOpt = CTipVideoMediaOption::INBAND_PARAM_SETS;
        break;
        
    case CRtcpAppMediaoptsPacket::CABAC:
        vmoOpt = CTipVideoMediaOption::CABAC;
        break;
        
    case CRtcpAppMediaoptsPacket::LTRP:
        vmoOpt = CTipVideoMediaOption::LTRP;
        break;
        
    case CRtcpAppMediaoptsPacket::GDR:
        vmoOpt = CTipVideoMediaOption::GDR;
        break;

    case CRtcpAppMediaoptsPacket::HP_8X8_TRANSFORMS:
        vmoOpt = CTipVideoMediaOption::HP_8X8_TRANSFORMS;
        break;
        
    case CRtcpAppMediaoptsPacket::UNRESTRICTED_XGA_1_5:
        vmoOpt = CTipVideoMediaOption::UNRESTRICTED_XGA_1_5;
        break;
        
    case CRtcpAppMediaoptsPacket::UNRESTRICTED_720P:
        vmoOpt = CTipVideoMediaOption::UNRESTRICTED_720P;
        break;
        
    case CRtcpAppMediaoptsPacket::UNRESTRICTED_1080P:
        vmoOpt = CTipVideoMediaOption::UNRESTRICTED_1080P;
        break;
        
    case CRtcpAppMediaoptsPacket::UNRESTRICED_XGA_30:
        vmoOpt = CTipVideoMediaOption::UNRESTRICED_XGA_30;
        break;
        
    default:
        AMDEBUG(TIPNEG, ("invalid option specified %d", opt));
        return;
    }

    AddOption(VIDEO, vmoOpt, (tx & opt), (rx & opt));
}

void CMapTipSystem::AddVideoOption(CRtcpAppMediaoptsPacket::SharedOption opt, uint32_t tx, uint32_t rx)
{
    AddSharedOption(VIDEO, opt, tx, rx);
}

void CMapTipSystem::AddAudioOption(CRtcpAppMediaoptsPacket::AudioOption opt, uint32_t tx, uint32_t rx)
{
    CTipAudioMediaOption::AudioOption amoOpt;

    switch (opt) {
    case CRtcpAppMediaoptsPacket::ACTIVITY_METRIC:
        amoOpt = CTipAudioMediaOption::ACTIVITY_METRIC;
        break;
        
    case CRtcpAppMediaoptsPacket::DYNAMIC_OUTPUT:
        amoOpt = CTipAudioMediaOption::DYNAMIC_OUTPUT;
        break;
        
    case CRtcpAppMediaoptsPacket::CAPABLE_G722_LEGACY:
        amoOpt = CTipAudioMediaOption::CAPABLE_G722_LEGACY;
        break;
        
    case CRtcpAppMediaoptsPacket::USING_G722_LEGACY:
        amoOpt = CTipAudioMediaOption::USING_G722_LEGACY;
        break;

    default:
        AMDEBUG(TIPNEG, ("invalid option specified %d", opt));
        return;
    }

    AddOption(AUDIO, amoOpt, (tx & opt), (rx & opt));
}

void CMapTipSystem::AddAudioOption(CRtcpAppMediaoptsPacket::SharedOption opt, uint32_t tx, uint32_t rx)
{
    AddSharedOption(AUDIO, opt, tx, rx);
}

void CMapTipSystem::AddSharedOption(MediaType type,
                                    CRtcpAppMediaoptsPacket::SharedOption opt,
                                    uint32_t tx, uint32_t rx)
{
    uint16_t optVal;
    
    switch (opt) {
    case CRtcpAppMediaoptsPacket::EKT:
        if (type == VIDEO) {
            optVal = CTipVideoMediaOption::EKT;
        } else {
            optVal = CTipAudioMediaOption::EKT;
        }
        break;

    default:
        AMDEBUG(TIPNEG, ("invalid option specified %d", opt));
        return;
    }

    AddOption(type, optVal, (tx & opt), (rx & opt));
}

void CMapTipSystem::AddOption(MediaType type, uint16_t opt, bool tx, bool rx)
{
    CTipMediaOption::OptionState state;

    if (tx && rx) {
        state = CTipMediaOption::OPTION_SUPPORTED_BOTH;
    } else if (tx) {
        state = CTipMediaOption::OPTION_SUPPORTED_TX;
    } else if (rx) {
        state = CTipMediaOption::OPTION_SUPPORTED_RX;
    } else {
        state = CTipMediaOption::OPTION_NOT_SUPPORTED;
    }

    CTipMediaOption mo(type, opt, state);
    MediaOptionMap::value_type v(opt, mo);

    mOptionMap[type].erase(opt);
    mOptionMap[type].insert(v);
}

void CMapTipSystem::NegotiateOption(CTipVideoMediaOption::VideoOption opt,
                                    const CTipSystem& local,
                                    const CTipSystem& remote)
{
    return NegotiateOption(VIDEO, opt, local.GetMediaOptionState(opt),
                           remote.GetMediaOptionState(opt));
}

void CMapTipSystem::NegotiateOption(CTipAudioMediaOption::AudioOption opt,
                                    const CTipSystem& local,
                                    const CTipSystem& remote)
{
    return NegotiateOption(AUDIO, opt, local.GetMediaOptionState(opt),
                           remote.GetMediaOptionState(opt));
}

void CMapTipSystem::NegotiateOption(MediaType type, uint16_t opt,
                                    CTipMediaOption::OptionState lState,
                                    CTipMediaOption::OptionState rState)
{
    CTipMediaOption::OptionState nState = CTipMediaOption::OPTION_NOT_SUPPORTED;

    if ((lState & CTipMediaOption::OPTION_SUPPORTED_TX) &&
        (rState & CTipMediaOption::OPTION_SUPPORTED_RX)) {
        
        nState = CTipMediaOption::OPTION_SUPPORTED_TX;
    }

    if ((lState & CTipMediaOption::OPTION_SUPPORTED_RX) &&
        (rState & CTipMediaOption::OPTION_SUPPORTED_TX)) {

        if (nState == CTipMediaOption::OPTION_SUPPORTED_TX) {
            nState = CTipMediaOption::OPTION_SUPPORTED_BOTH;
        } else {
            nState = CTipMediaOption::OPTION_SUPPORTED_RX;
        }
    }

    CTipMediaOption mo(type, opt, nState);
    MediaOptionMap::value_type v(opt, mo);

    mOptionMap[type].erase(opt);
    mOptionMap[type].insert(v);
}

