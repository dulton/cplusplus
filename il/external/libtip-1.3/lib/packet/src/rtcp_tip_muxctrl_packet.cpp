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

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "rtcp_tip_muxctrl_packet.h"
using namespace LibTip;

CRtcpAppMuxCtrlPacketBase::CRtcpAppMuxCtrlPacketBase(uint8_t version) :
    CRtcpTipPacket(MUXCTRL)
{
	// only default is the version, everything else goes to 0
	memset(&mCtrl, 0, sizeof(mCtrl));
    SetVersion(version);

    IncrSize(sizeof(mCtrl));
}

CRtcpAppMuxCtrlPacketBase::~CRtcpAppMuxCtrlPacketBase()
{
}

uint32_t CRtcpAppMuxCtrlPacketBase::PackData(CPacketBuffer& buffer) const
{
    // NOTE: we do NOT call CRtcpTipPacket::PackData() here b/c the
    // format of MUXCTRL does not have the xmit ntp time in the
    // standard place.  we instead bypass and go straight to
    // CRtcpAppPacket::PackData().  ntp time is pulled from
    // CRtcpTipPacket below.
	CRtcpAppPacket::PackData(buffer);

    buffer.Add(mCtrl.mvp);
    buffer.Add(mCtrl.options);
    buffer.Add(mCtrl.numXmit);
    buffer.Add(mCtrl.numRcv);
    buffer.Add(mTipHeader.mNtpTime);
    buffer.Add(mCtrl.confID);
    buffer.Add(mCtrl.xmitPositions);
    buffer.Add(mCtrl.rcvPositions);

    return buffer.GetBufferSize();
}

int CRtcpAppMuxCtrlPacketBase::UnpackData(CPacketBuffer& buffer)
{
    // NOTE: we do NOT call CRtcpTipPacket::UnpackData() here b/c the
    // format of MUXCTRL does not have the xmit ntp time in the
    // standard place.  we instead bypass and go straight to
    // CRtcpAppPacket::UnpackData().  ntp time is set in
    // CRtcpTipPacket below.  
    int ret = CRtcpAppPacket::UnpackData(buffer);
	if (ret != 0) {
        return ret;
    }

    // also b/c we do not call CRtcpTipPacket::UnpackData() we must
    // duplicate the error handling in that method, ie validate the
    // appname.  this is a more strict check than is in
    // CRtcpTipPacket::UnpackData() b/c we know what our appname
    // should be.
    if (memcmp(mAppName.mName, GetExtensionForTipPacketType(MUXCTRL), RTCP_APPNAME_LENGTH) != 0) {
        return -1;
    }

    buffer.Rem(mCtrl.mvp);
    buffer.Rem(mCtrl.options);
    buffer.Rem(mCtrl.numXmit);
    buffer.Rem(mCtrl.numRcv);
    buffer.Rem(mTipHeader.mNtpTime);
    buffer.Rem(mCtrl.confID);
    buffer.Rem(mCtrl.xmitPositions);
    buffer.Rem(mCtrl.rcvPositions);

    return ret;
}


void CRtcpAppMuxCtrlPacketBase::ToStream(std::ostream& o, MediaType mType) const
{
    CRtcpTipPacket::ToStream(o, mType);

    uint8_t profile = GetProfile();

    o << "\n\tVERSION:  " << std::dec << static_cast<int>(GetVersion())
      << "\n\tPROFILE:  " << static_cast<int>(profile) << " ("
      << ((profile & SECURE_PROFILE) ? "S" : "") << "AVP"
      << ((profile & FEEDBACK_PROFILE) ? "F" : "") << ')'
      << "\n\tOPTIONS:  " << static_cast<int>(mCtrl.options)
      << "\n\tXMIT:     " << static_cast<int>(mCtrl.numXmit)
      << " (0x" << std::hex << mCtrl.xmitPositions << std::dec << ')';

    PositionsToStream(o, mCtrl.xmitPositions, mType);
    
    o << "\n\tRECV:     " << static_cast<int>(mCtrl.numRcv)
      << " (0x" << std::hex << mCtrl.rcvPositions << std::dec << ')';

    PositionsToStream(o, mCtrl.rcvPositions, mType);

    o << "\n\tCONFID:   " << mCtrl.confID;
}

CRtcpAppMuxCtrlPacket::CRtcpAppMuxCtrlPacket() :
    CRtcpAppMuxCtrlPacketBase(DEFAULT_VERSION)
{

}

CRtcpAppMuxCtrlPacket::~CRtcpAppMuxCtrlPacket()
{

}

int CRtcpAppMuxCtrlPacket::UnpackData(CPacketBuffer& buffer)
{
    int ret = CRtcpAppMuxCtrlPacketBase::UnpackData(buffer);
    if (ret != 0) {
        return ret;
    }

    if (GetVersion() != DEFAULT_VERSION) {
        // unsupported version
        return -1;
    }

    return 0;
}
    
    
CRtcpAppMuxCtrlV7Packet::CRtcpAppMuxCtrlV7Packet() :
    CRtcpAppMuxCtrlPacketBase(DEFAULT_VERSION)
{
	// only default is the version, everything else goes to 0
	memset(&mCtrlV7, 0, sizeof(mCtrlV7));
    SetVersion(DEFAULT_VERSION);

    IncrSize(sizeof(mCtrlV7));
}

CRtcpAppMuxCtrlV7Packet::~CRtcpAppMuxCtrlV7Packet()
{
    while (! mTlv.empty()) {
        CTipTlv* ptlv = mTlv.front();
        mTlv.pop_front();
        delete ptlv;
    }
}

int CRtcpAppMuxCtrlV7Packet::SetParticipantID(const uint8_t* id, uint32_t len)
{
    if (id == NULL) {
        // no good
        return -1;
    }

    if (len > CTipTlv::MAX_TLV_DATA_LEN) {
        // too big
        return -1;
    }

    // find participant TLV if there is one
    CTipTlv* partTLV = const_cast<CTipTlv*>(FindTLV(PARTICIPANT_ID_TAG));
    if (partTLV == NULL) {
        partTLV = new CTipTlv();

        if (partTLV == NULL) {
            return -1;
        }

        partTLV->SetTag(PARTICIPANT_ID_TAG);

        // add to the linked list
        mTlv.push_back(partTLV);
        
    } else {
        DecrSize(partTLV->GetPackSize());
    }

    partTLV->SetData(id, len);
    IncrSize(partTLV->GetPackSize());
    return 0;
}

uint32_t CRtcpAppMuxCtrlV7Packet::GetParticipantID(uint8_t* id) const
{
    // find participant TLV if there is one
    const CTipTlv* partTLV = FindTLV(PARTICIPANT_ID_TAG);
    if (partTLV == NULL) {
        return 0;
    }

    uint32_t len = partTLV->GetLength();
    if (id != NULL) {
        memcpy(id, partTLV->GetData(), len);
    }

    return len;
}

const CTipTlv* CRtcpAppMuxCtrlV7Packet::FindTLV(uint8_t tag) const
{
    CTlvpList::const_iterator i;
    for (i = mTlv.begin(); i != mTlv.end(); ++i) {
        if ((*i)->GetTag() == tag) {
            return *i;
        }
    }

    return NULL;
}

uint32_t CRtcpAppMuxCtrlV7Packet::PackData(CPacketBuffer& buffer) const
{
    CRtcpAppMuxCtrlPacketBase::PackData(buffer);

    buffer.Add(mCtrlV7.numSharedPositions);
    buffer.Add(mCtrlV7.reserved);
    buffer.Add(mCtrlV7.sharedPositions);
    
    CTlvpList::const_iterator i;
    for (i = mTlv.begin(); i != mTlv.end(); ++i) {
        (*i)->Pack(buffer);
    }

    return buffer.GetBufferSize();
}

int CRtcpAppMuxCtrlV7Packet::UnpackData(CPacketBuffer& buffer)
{
    int ret = CRtcpAppMuxCtrlPacketBase::UnpackData(buffer);
    if (ret != 0) {
        return ret;
    }

    if (GetVersion() < DEFAULT_VERSION) {
        // unsupported version
        return -1;
    }
    
    buffer.Rem(mCtrlV7.numSharedPositions);
    buffer.Rem(mCtrlV7.reserved);
    buffer.Rem(mCtrlV7.sharedPositions);

    // if there is data left then try to unpack that into TLVs
    while (buffer.GetBufferSize() >= CTipTlv::MIN_TLV_PACK_LEN) {
        CTipTlv* tlv = new CTipTlv();
        if (tlv == NULL) {
            // oh well, we tried
            ret = -1;
            break;
        }

        // if the tlv cannot unpacket properly then the packet is malformed
        if (tlv->Unpack(buffer) != 0) {
            ret = -1;
            delete tlv;
            break;
        }

        // if the tlv tag is reserved then this is padding so while it
        // may unpack we do not track it
        if (tlv->GetTag() == RESERVED_TAG) {
            delete tlv;
            break;
        }
        
        mTlv.push_back(tlv);
        IncrSize(tlv->GetPackSize());
    }

    return ret;
}

void CRtcpAppMuxCtrlV7Packet::ClearData()
{
    // we are about to unpack a newly received buffer.  clear out old TLVs.
    while (! mTlv.empty()) {
        CTipTlv* ptlv = mTlv.front();
        mTlv.pop_front();

        DecrSize(ptlv->GetPackSize());
        delete ptlv;
    }
    
}

void CRtcpAppMuxCtrlV7Packet::ToStream(std::ostream& o, MediaType mType) const
{
    CRtcpAppMuxCtrlPacketBase::ToStream(o, mType);

    o << "\n\tSHARED:   " << std::dec << static_cast<int>(mCtrlV7.numSharedPositions)
      << " (0x" << std::hex << mCtrlV7.sharedPositions << std::dec << ')';
}
