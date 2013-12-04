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

#include <arpa/inet.h> /* for ntohl and friends */

#include "tip_debug_print.h"
#include "rtcp_packet.h"
using namespace LibTip;

CRtcpPacket::CRtcpPacket()
{
	// init default values
	mRtcpHeader.vps    = ((RTCP_VERSION << VPS_VERSION_SHIFT) | (0 << VPS_PADDING_SHIFT) | (0 << VPS_SUBTYPE_SHIFT));
	mRtcpHeader.type   = 0;
	mRtcpHeader.length = 0;

    SetSize(sizeof(mRtcpHeader));
}

CRtcpPacket::~CRtcpPacket()
{

}

uint32_t CRtcpPacket::Pack(CPacketBuffer& buffer) const
{
    // call virtual pack function to get derived data packed
    PackData(buffer);
    
    // pad data to 4 byte alignment
    Pad(buffer);

    return buffer.GetBufferSize();
}

int CRtcpPacket::Unpack(CPacketBuffer& buffer)
{
    // track consumed bytes
    uint32_t start = buffer.GetBufferOffset();
        
    // clear out old data
    ClearData();
    
    // verify buffer is at least minimum size
    if (buffer.GetBufferSize() < mSize) {
        AMDEBUG(PKTERR,
                ("buffer (%u bytes) is too small for packet type %hhu subtype %hhu size %u bytes",
                 buffer.GetBufferSize(), mRtcpHeader.type, GetSubType(), mSize));
        return -1;
    }

    if (UnpackData(buffer) != 0) {
        return -1;
    }

    // consume any extra data, should be just padding bytes
    uint32_t consumed = (buffer.GetBufferOffset() - start);
    if (consumed < RtcpLengthToBytes(mRtcpHeader.length)) {
        buffer.ResetHead((start +  RtcpLengthToBytes(mRtcpHeader.length)));
    }

    return 0;
}

uint32_t CRtcpPacket::GetPackSize() const
{
    uint32_t mod = (mSize % sizeof(uint32_t));
    if (mod == 0) {
        return mSize;
    }

    return (mSize + (4 - mod));
}

uint32_t CRtcpPacket::PackData(CPacketBuffer& buffer) const
{
    buffer.Add(mRtcpHeader.vps);
    buffer.Add(mRtcpHeader.type);
    buffer.Add(GetLength());

	return buffer.GetBufferSize();
}

int CRtcpPacket::UnpackData(CPacketBuffer& buffer)
{
    uint32_t bufsize = buffer.GetBufferSize();
    
    buffer.Rem(mRtcpHeader.vps);
    buffer.Rem(mRtcpHeader.type);
    buffer.Rem(mRtcpHeader.length);

    // version must be 2
    if (GetVersion() != RTCP_VERSION) {
        AMDEBUG(PKTERR, ("invalid RTCP version %hhu", GetVersion()));
        return -1;
    }

    // make sure our internal size (which includes derived class data)
    // and rtcp length are compatible
    if (mSize > RtcpLengthToBytes(mRtcpHeader.length)) {
        AMDEBUG(PKTERR,
                ("RTCP length (%u bytes) is too small for packet type %hhu subtype %hhu size %u bytes",
                 RtcpLengthToBytes(mRtcpHeader.length), mRtcpHeader.type, GetSubType(), mSize));
        return -1;
    }

    // make sure buffer size and rtcp length are compatible
    if (RtcpLengthToBytes(mRtcpHeader.length) > bufsize) {
        AMDEBUG(PKTERR,
                ("RTCP length (%u bytes) is too large for packet buffer (%u bytes)",
                 RtcpLengthToBytes(mRtcpHeader.length), bufsize));
        return -1;
    }

    return 0;
}

void CRtcpPacket::ClearData()
{

}

void CRtcpPacket::Pad(CPacketBuffer& buffer) const
{
    uint8_t pad = 0;

    while ((buffer.GetBufferSize() % 4) != 0) {
        buffer.Add(pad);
    }
}

void CRtcpPacket::SetSize(uint32_t size)
{
    mSize = size;
    
    // anticipate padding to 4 byte length by rounding up
    mRtcpHeader.length = BytesToRtcpLength(mSize);
    if ((mSize % sizeof(uint32_t)) != 0) {
        mRtcpHeader.length++;
    }
}

uint32_t CRtcpPacket::BytesToRtcpLength(uint32_t numBytes)
{
	return ((numBytes / sizeof(uint32_t)) - 1);
}

uint32_t CRtcpPacket::RtcpLengthToBytes(uint32_t rtcpLength)
{
	return ((rtcpLength + 1) * sizeof(uint32_t));
}

void CRtcpPacket::ToStream(std::ostream& o, MediaType mType) const
{
}

CRtcpPacketSSRC::CRtcpPacketSSRC() : mSSRC(0)
{
    IncrSize(sizeof(mSSRC));
}

CRtcpPacketSSRC::~CRtcpPacketSSRC() 
{

}

uint32_t CRtcpPacketSSRC::PackData(CPacketBuffer& buffer) const
{
    CRtcpPacket::PackData(buffer);

    buffer.Add(mSSRC);
	return buffer.GetBufferSize();
}

int CRtcpPacketSSRC::UnpackData(CPacketBuffer& buffer)
{
    int ret = CRtcpPacket::UnpackData(buffer);
    if (ret == 0) {
        buffer.Rem(mSSRC);
    }
    
    return ret;
}

CRtcpAppPacket::CRtcpAppPacket()
{
    SetType(APP);
	memset(mAppName.mName, 0, RTCP_APPNAME_LENGTH);
    IncrSize(RTCP_APPNAME_LENGTH);
}

CRtcpAppPacket::~CRtcpAppPacket()
{

}

uint32_t CRtcpAppPacket::PackData(CPacketBuffer& buffer) const
{
    CRtcpPacketSSRC::PackData(buffer);

    buffer.Add(mAppName);
	return buffer.GetBufferSize();
}

int CRtcpAppPacket::UnpackData(CPacketBuffer& buffer)
{
    int ret = CRtcpPacketSSRC::UnpackData(buffer);
    if (ret == 0) {
        if (GetType() != APP) {
            AMDEBUG(PKTERR, ("RTCP type (%hhu) is not APP (%hhu)", GetType(), APP));
            return -1;
        }
        
        buffer.Rem(mAppName);
    }
    
    return ret;
}

CRtcpTipPacket::CRtcpTipPacket(TipPacketType type)
{
	mTipHeader.mNtpTime = 0;

    SetSubType(ConvertTipToRtcp(type));
    SetAppName(GetExtensionForTipPacketType(type));
    
    IncrSize(sizeof(mTipHeader));
}

CRtcpTipPacket::~CRtcpTipPacket()
{

}

TipPacketType CRtcpTipPacket::GetTipPacketType() const
{
    return ConvertRtcpToTip(GetSubType(), GetAppName().mName);
}

const char* const CRtcpTipPacket::GetTipPacketTypeString() const
{
    return LibTip::GetTipPacketTypeString(GetTipPacketType());
}

bool CRtcpTipPacket::IsMyAck(const CRtcpTipPacket& ack) const
{
    TipPacketType myAckType = ConvertTipNonAckToAck(GetTipPacketType());
    TipPacketType theirType = ack.GetTipPacketType();

    if (myAckType != theirType) {
        return false;
    }

    if (GetNtpTime() != ack.GetNtpTime()) {
        return false;
    }

    return true;
}

uint32_t CRtcpTipPacket::PackData(CPacketBuffer& buffer) const
{
    CRtcpAppPacket::PackData(buffer);

    buffer.Add(mTipHeader.mNtpTime);
    
	return buffer.GetBufferSize();
}

int CRtcpTipPacket::UnpackData(CPacketBuffer& buffer)
{
    int ret = CRtcpAppPacket::UnpackData(buffer);
    if (ret == 0) {
        // verify that the appname field (extracted by CRtcpAppPacket)
        // is a valid TIP appname
        if (ConvertRtcpToTip(GetSubType(), mAppName.mName) == MAX_PACKET_TYPE) {
            AMDEBUG(PKTERR, ("RTCP APP NAME (%c%c%c%c) is not a TIP APP NAME (%s or %s)",
                             mAppName.mName[0], mAppName.mName[1], mAppName.mName[2],
                             mAppName.mName[3], kRtcpAppExtension[0], kRtcpAppExtension[1]));
            return -1;
        }
        
        buffer.Rem(mTipHeader.mNtpTime);
    }

    return ret;
}

void CRtcpTipPacket::ToStream(std::ostream& o, MediaType mType) const
{
    o << "\n\tTYPE:     " << GetTipPacketTypeString()
      << "\n\tNTP:      " << std::dec << mTipHeader.mNtpTime;
}

void CRtcpTipPacket::PositionsToStream(std::ostream& o, uint16_t pos, MediaType mType)
{
    if (mType == MT_MAX) {
        return;
    }

    o << "( ";
    
    if (mType == AUDIO) {
		if (pos & PositionToMask(POS_AUDIO_INVALID)) {
            o << "INVALID ";
        }
        
		if (pos & PositionToMask(POS_AUDIO_CENTER)) {
            o << "CENTER ";
        }
        
		if (pos & PositionToMask(POS_AUDIO_LEFT)) {
            o << "LEFT ";
        }
        
		if (pos & PositionToMask(POS_AUDIO_RIGHT)) {
            o << "RIGHT ";
        }
        
		if (pos & PositionToMask(POS_AUDIO_AUX)) {
            o << "AUX ";
        }
        
		if (pos & PositionToMask(POS_AUDIO_UNUSED1)) {
            o << "UNUSED1 ";
        }
        
		if (pos & PositionToMask(POS_AUDIO_UNUSED2)) {
            o << "UNUSED2 ";
        }
        
		if (pos & PositionToMask(POS_AUDIO_UNUSED3)) {
            o << "UNUSED3 ";
        }
        
		if (pos & PositionToMask(POS_AUDIO_UNUSED4)) {
            o << "UNUSED4 ";
        }
        
		if (pos & PositionToMask(POS_AUDIO_UNUSED5)) {
            o << "UNUSED5 ";
        }
        
		if (pos & PositionToMask(POS_AUDIO_UNUSED6)) {
            o << "UNUSED6 ";
        }
        
		if (pos & PositionToMask(POS_AUDIO_UNUSED7)) {
            o << "UNUSED7 ";
        }
        
		if (pos & PositionToMask(POS_AUDIO_LEGACY)) {
            o << "LEGACY ";
        }
        
		if (pos & PositionToMask(POS_AUDIO_UNUSED8)) {
            o << "UNUSED8 ";
        }
        
		if (pos & PositionToMask(POS_AUDIO_UNUSED9)) {
            o << "UNUSED9 ";
        }
        
		if (pos & PositionToMask(POS_AUDIO_MAX)) {
            o << "MAX ";
        }

    } else {

		if (pos & PositionToMask(POS_VIDEO_INVALID)) {
            o << "INVALID ";
        }
        
		if (pos & PositionToMask(POS_VIDEO_CENTER)) {
            o << "CENTER ";
        }
        
		if (pos & PositionToMask(POS_VIDEO_LEFT)) {
            o << "LEFT ";
        }
        
		if (pos & PositionToMask(POS_VIDEO_RIGHT)) {
            o << "RIGHT ";
        }
        
		if (pos & PositionToMask(POS_VIDEO_AUX_1_5FPS)) {
            o << "AUX_1_5FPS ";
        }
        
		if (pos & PositionToMask(POS_VIDEO_AUX_30FPS)) {
            o << "AUX_30FPS ";
        }
        
		if (pos & PositionToMask(POS_VIDEO_UNUSED1)) {
            o << "UNUSED1 ";
        }
        
		if (pos & PositionToMask(POS_VIDEO_UNUSED2)) {
            o << "UNUSED2 ";
        }
        
		if (pos & PositionToMask(POS_VIDEO_UNUSED3)) {
            o << "UNUSED3 ";
        }
        
		if (pos & PositionToMask(POS_VIDEO_LEGACY_CENTER)) {
            o << "LEGACY_CENTER ";
        }
        
		if (pos & PositionToMask(POS_VIDEO_LEGACY_LEFT)) {
            o << "LEGACY_LEFT ";
        }
        
		if (pos & PositionToMask(POS_VIDEO_LEGACY_RIGHT)) {
            o << "LEGACY_RIGHT ";
        }
        
		if (pos & PositionToMask(POS_VIDEO_UNUSED4)) {
            o << "UNUSED4 ";
        }
        
		if (pos & PositionToMask(POS_VIDEO_UNUSED5)) {
            o << "UNUSED5 ";
        }
        
		if (pos & PositionToMask(POS_VIDEO_UNUSED6)) {
            o << "UNUSED6 ";
        }
        
		if (pos & PositionToMask(POS_VIDEO_MAX)) {
            o << "MAX ";
        }
    }

    o << ')';
}

