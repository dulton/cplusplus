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

#include "rtcp_tip_mediaopts_packet.h"
using namespace LibTip;

CRtcpAppMediaoptsPacket::CRtcpAppMediaoptsPacket(Version version) :
    CRtcpTipPacket(MEDIAOPTS)
{
    mBase.version = version;
    mBase.reserved = 0;

    // version 2 requires the reserved bits be 0xFFFF and always has
    // tx and rx options
    if (mBase.version == 2) {
        mBase.reserved = 0xFFFF;

        AddSSRC(0, 0, 0);
    }
    
    IncrSize(sizeof(mBase));
}

CRtcpAppMediaoptsPacket::~CRtcpAppMediaoptsPacket() {

}

void CRtcpAppMediaoptsPacket::AddSSRC(uint32_t ssrc, uint32_t xmitOpt, uint32_t rcvOpt)
{
    // V2 only supports a single SSRC so enforce that
    if (mBase.version == 2) {
        ssrc = 0;
    }
    
    SsrcMap::iterator i = mSsrcMap.find(ssrc);
    if (i != mSsrcMap.end()) {
        // just update here so we don't lose options which may be in
        // the RtcpAppMOSsrc.optMap
        (*i).second.xmitOptions = xmitOpt;
        (*i).second.rcvOptions  = rcvOpt;

        return;
    }

    // new SSRC, incr size and add new data
    
    // for V2 new SSRC adds 8 bytes
    // 4 bytes tx opt, 4 bytes rx opt
    if (mBase.version == 2) {
        IncrSize((sizeof(uint32_t) * 2));
    } else {
        // new SSRC adds 16 bytes
        // 4 bytes SSRC, 4 bytes tx opt, 4 bytes rx opt, 4 bytes end-tag
        IncrSize((sizeof(uint32_t) * 4));
    }
    
    RtcpAppMOSsrc moSsrc;
    moSsrc.xmitOptions = xmitOpt;
    moSsrc.rcvOptions = rcvOpt;
    
    mSsrcMap[ssrc] = moSsrc;
}

std::list<uint32_t> CRtcpAppMediaoptsPacket::GetAllSSRC() const
{
    std::list<uint32_t> ssrc;
    SsrcMap::const_iterator i;

    for (i = mSsrcMap.begin(); i != mSsrcMap.end(); ++i) {
        ssrc.push_back((*i).first);
    }

    return ssrc;
}

int CRtcpAppMediaoptsPacket::GetSSRC(uint32_t ssrc, uint32_t& xmitOpt, uint32_t& rcvOpt) const
{
    // version 2 only supports a single ssrc so enforce that
    if (mBase.version == 2) {
        ssrc = 0;
    }
    
    SsrcMap::const_iterator i = mSsrcMap.find(ssrc);

    if (i == mSsrcMap.end()) {
        return -1;
    }

    xmitOpt = (*i).second.xmitOptions;
    rcvOpt = (*i).second.rcvOptions;
    return 0;
}

int CRtcpAppMediaoptsPacket::AddOption(uint32_t ssrc, OptionTag optTag, uint32_t optValue)
{
    // version 2 only supports a single ssrc so enforce that
    if (mBase.version == 2) {
        ssrc = 0;
    }
    
    if (optTag == LASTTAG) {
        // caller not allowed to add this tag
        return -1;
    }
    
    SsrcMap::iterator si = mSsrcMap.find(ssrc);
    if (si == mSsrcMap.end()) {
        return -1;
    }

    OptionMap& om = (*si).second.optMap;
    OptionMap::iterator oi = om.find(optTag);

    if (oi == om.end()) {
        // new tag, add 4 bytes
        IncrSize(sizeof(uint32_t));
    }
    
    om[optTag] = optValue;
    return 0;
}

std::list<CRtcpAppMediaoptsPacket::OptionTag>
CRtcpAppMediaoptsPacket::GetOptions(uint32_t ssrc) const
{
    // version 2 only supports a single ssrc so enforce that
    if (mBase.version == 2) {
        ssrc = 0;
    }
    
    std::list<OptionTag> options;
    SsrcMap::const_iterator si = mSsrcMap.find(ssrc);

    if (si == mSsrcMap.end()) {
        return options;
    }

    const OptionMap& om = (*si).second.optMap;
    OptionMap::const_iterator i;
    for (i = om.begin(); i != om.end(); ++i) {
        options.push_back((*i).first);
    }

    return options;
}

int CRtcpAppMediaoptsPacket::GetOption(uint32_t ssrc, OptionTag optTag, uint32_t& optValue) const
{
    // version 2 only supports a single ssrc so enforce that
    if (mBase.version == 2) {
        ssrc = 0;
    }
    
    SsrcMap::const_iterator si = mSsrcMap.find(ssrc);
    if (si == mSsrcMap.end()) {
        return -1;
    }

    const OptionMap& om = si->second.optMap;
    OptionMap::const_iterator oi = om.find(optTag);
    if (oi == om.end()) {
        return -1;
    }

    optValue = (*oi).second;
    return 0;
}

uint32_t CRtcpAppMediaoptsPacket::PackData(CPacketBuffer& buffer) const
{
    CRtcpTipPacket::PackData(buffer);

    buffer.Add(mBase.version);
    buffer.Add(mBase.reserved);

    SsrcMap::const_iterator si;
    for (si = mSsrcMap.begin(); si != mSsrcMap.end(); ++si) {

        // only add SSRC for V3
        if (mBase.version >= 3) {
            buffer.Add(si->first);
        }
        
        buffer.Add(si->second.xmitOptions);
        buffer.Add(si->second.rcvOptions);

        const OptionMap& om = si->second.optMap;
        OptionMap::const_iterator oi;
        
        for (oi = om.begin(); oi != om.end(); ++oi) {
            uint32_t opt = (((oi->first << OPT_TAG_SHIFT) & OPT_TAG_MASK) | 
                          ((oi->second << OPT_VAL_SHIFT) & OPT_VAL_MASK));
            
            buffer.Add(opt);
        }

        // force add last tag for V3
        if (mBase.version >= 3) {
            uint32_t opt = (((LASTTAG << OPT_TAG_SHIFT) & OPT_TAG_MASK) | 
                            ((0 << OPT_VAL_SHIFT) & OPT_VAL_MASK));
            buffer.Add(opt);
        }
    }
    
    return buffer.GetBufferSize();
}

int CRtcpAppMediaoptsPacket::UnpackData(CPacketBuffer& buffer)
{
    int ret = CRtcpTipPacket::UnpackData(buffer);
    if (ret != 0) {
        return ret;
    }

    buffer.Rem(mBase.version);
    buffer.Rem(mBase.reserved);

    // validate version
    if (mBase.version < MINIMUM_VERSION || mBase.version > MAXIMUM_VERSION) {
        return -1;
    }
    
    // anything less than 4 bytes is likely padding so ignore it.
    // anything bigger try to process.
    while (buffer.GetBufferSize() >= sizeof(uint32_t)) {
        if (mBase.version == 2) {
            ret = UnpackSSRCV2(buffer);
        } else {
            ret = UnpackSSRC(buffer);
        }
        if (ret != 0) {
            return ret;
        }
    }

    return ret;
}

int CRtcpAppMediaoptsPacket::UnpackSSRC(CPacketBuffer& buffer)
{
    // minimum 12 bytes required per SSRC, anything else is bogus
    const uint32_t minSSRCSize = (sizeof(uint32_t) * 3);
    if (buffer.GetBufferSize() < minSSRCSize) {
        return -1;
    }

    uint32_t ssrc;
    RtcpAppMOSsrc moSsrc;

    buffer.Rem(ssrc);
    buffer.Rem(moSsrc.xmitOptions);
    buffer.Rem(moSsrc.rcvOptions);

    // adjust size of packet to match removed SSRC (if new).  note
    // only adding 16 bytes here as the rest will be added when we
    // unpack the options
    if (mSsrcMap.find(ssrc) == mSsrcMap.end()) {
        IncrSize((sizeof(uint32_t) * 4));
    }
    
    mSsrcMap[ssrc] = moSsrc;

    // unpack options, there aren't really errors unpacking options so
    // just stop when we get -1
    while (UnpackOption(ssrc, buffer) == 0) { ; }

    return 0;
}

int CRtcpAppMediaoptsPacket::UnpackSSRCV2(CPacketBuffer& buffer)
{
    // minimum 8 bytes required anything else is bogus
    const uint32_t minSSRCSize = (sizeof(uint32_t) * 2);
    if (buffer.GetBufferSize() < minSSRCSize) {
        return -1;
    }

    // for V2 only one SSRC block so use 0
    uint32_t ssrc = 0;
    RtcpAppMOSsrc moSsrc;

    buffer.Rem(moSsrc.xmitOptions);
    buffer.Rem(moSsrc.rcvOptions);

    if (mSsrcMap.find(ssrc) == mSsrcMap.end()) {
        IncrSize((sizeof(uint32_t) * 2));
    }
    
    mSsrcMap[ssrc] = moSsrc;

    // unpack options, there aren't really errors unpacking options so
    // just stop when we get -1.  for V2 there isn't a LASTTAG so
    // we'll just loop until we consume the whole packet.
    while (UnpackOption(ssrc, buffer) == 0) { ; }

    return 0;
}

int CRtcpAppMediaoptsPacket::UnpackOption(uint32_t ssrc, CPacketBuffer& buffer)
{
    // can be 0 or more options, each is 4 bytes
    if (buffer.GetBufferSize() < sizeof(uint32_t)) {
        return -1;
    }

    uint32_t option;
    buffer.Rem(option);

    OptionTag optTag = (OptionTag) ((option & OPT_TAG_MASK) >> OPT_TAG_SHIFT);
    uint32_t  optVal = ((option & OPT_VAL_MASK) >> OPT_VAL_SHIFT);

    // if this is the lag tag we are done processing options for this ssrc
    if (optTag == LASTTAG) {
        return -1;
    }

    // something valid, increase size (if new)
    if (mSsrcMap[ssrc].optMap.find(optTag) == mSsrcMap[ssrc].optMap.end()) {
        IncrSize(sizeof(uint32_t));
    }
    
    // some other option, add it to the list
    mSsrcMap[ssrc].optMap[optTag] = optVal;
    return 0;
}

void CRtcpAppMediaoptsPacket::ClearData()
{
    SsrcMap::iterator i;
    for (i = mSsrcMap.begin(); i != mSsrcMap.end(); ++i) {

        OptionMap& om = (*i).second.optMap;
        OptionMap::iterator oi;

        for (oi = om.begin(); oi != om.end(); ++oi) {
            // subtract 4 bytes for each option tag
            DecrSize(sizeof(uint32_t));
        }

        // subtract size of SSRC block
        if (mBase.version == 2) {
            DecrSize((sizeof(uint32_t) * 2));
        } else {
            DecrSize((sizeof(uint32_t) * 4));
        }
    }
    
    // remove all entries
    mSsrcMap.clear();

    // for V2 add back in the one required SSRC
    if (mBase.version == 2) {
        AddSSRC(0, 0, 0);
    }
}

uint32_t CRtcpAppMediaoptsPacket::GetBitValueForAuxFrameRate(AuxFrameRate fr)
{
    if (fr == AUX_1FPS) {
        return AUX_BIT1;
    }

    if (fr == AUX_30FPS) {
        return AUX_BIT0;
    }
    
    return 0;
}

CRtcpAppMediaoptsPacket::AuxFrameRate
CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(uint32_t bv)
{
    if ((bv & AUX_BIT1) && (bv & AUX_BIT0) == 0) {
        return AUX_1FPS;
    }

    if ((bv & AUX_BIT0) && (bv & AUX_BIT1) == 0) {
        return AUX_30FPS;
    }

    return AUX_5FPS;
}

void CRtcpAppMediaoptsPacket::ToStream(std::ostream& o, MediaType mType) const
{
    CRtcpTipPacket::ToStream(o, mType);

    o << "\n\tVERSION:  " << std::dec << static_cast<int>(mBase.version);

    SsrcMap::const_iterator i;

    for (i = mSsrcMap.begin(); i != mSsrcMap.end(); ++i) {
        o << "\n\tSSRC:     0x" << std::hex << (*i).first
          << "\n\t  TX:      0x" << (*i).second.xmitOptions;

        OptionsToStream(o, (*i).second.xmitOptions, mType);
        
        o << "\n\t  RX:      0x" << (*i).second.rcvOptions;
        OptionsToStream(o, (*i).second.rcvOptions, mType);

        const OptionMap& om = (*i).second.optMap;
        OptionMap::const_iterator j;
        for (j = om.begin(); j != om.end(); ++j) {
            OptionTagValueToStream(o, (*j).first, (*j).second);
        }
    }
}

void CRtcpAppMediaoptsPacket::OptionsToStream(std::ostream& o, uint32_t opt,
                                              MediaType mType) const
{
    if (mType == MT_MAX) {
        return;
    }
    
    o << " ( ";
    
    if (mType == AUDIO) {
        if (opt & CRtcpAppMediaoptsPacket::ACTIVITY_METRIC) {
            o << "ACTIVITY_METRIC ";
        }
        
        if (opt & CRtcpAppMediaoptsPacket::DYNAMIC_OUTPUT) {
            o << "DYNAMIC_OUTPUT ";
        }
        
        if (opt & CRtcpAppMediaoptsPacket::CAPABLE_G722_LEGACY) {
            o << "CAPABLE_G722_LEGACY ";
        }
        
        if (opt & CRtcpAppMediaoptsPacket::USING_G722_LEGACY) {
            o << "USING_G722_LEGACY ";
        }
        
        if (opt & CRtcpAppMediaoptsPacket::EKT) {
            o << "EKT ";
        }
    } else {
        if (opt & CRtcpAppMediaoptsPacket::REFRESH_FLAG) {
            o << "REFRESH_FLAG ";
        }
        
        if (opt & CRtcpAppMediaoptsPacket::INBAND_PARAM_SETS) {
            o << "INBAND_PARAM_SETS ";
        }
        
        if (opt & CRtcpAppMediaoptsPacket::CABAC) {
            o << "CABAC ";
        }
        
        if (opt & CRtcpAppMediaoptsPacket::LTRP) {
            o << "LTRP ";
        }
        
        if (opt & CRtcpAppMediaoptsPacket::AUX_BIT0) {
            o << "AUX_BIT0 ";
        }
        
        if (opt & CRtcpAppMediaoptsPacket::GDR) {
            o << "GDR ";
        }
        
        if (opt & CRtcpAppMediaoptsPacket::AUX_BIT1) {
            o << "AUX_BIT1 ";
        }
        
        if (opt & CRtcpAppMediaoptsPacket::HP_8X8_TRANSFORMS) {
            o << "HP_8X8_TRANSFORMS";
        }
        
        if (opt & CRtcpAppMediaoptsPacket::UNRESTRICTED_XGA_1_5) {
            o << "UNRESTRICTED_XGA_1_5";
        }
        
        if (opt & CRtcpAppMediaoptsPacket::UNRESTRICTED_720P) {
            o << "UNRESTRICTED_720P";
        }
        
        if (opt & CRtcpAppMediaoptsPacket::UNRESTRICTED_1080P) {
            o << "UNRESTRICTED_1080P";
        }
        
        if (opt & CRtcpAppMediaoptsPacket::UNRESTRICED_XGA_30) {
            o << "UNRESTRICED_XGA_30";
        }
        
        if (opt & CRtcpAppMediaoptsPacket::EKT) {
            o << "EKT ";
        }
    }

    o << ')';
}

void CRtcpAppMediaoptsPacket::OptionTagValueToStream(std::ostream& o, OptionTag tag,
                                                     uint32_t value) const
{
    o << "\n\t  OPTION:  ";

    switch (tag) {
    case PROFILE:
        o << "PROFILE      = ";
        break;

    case LEGACYMIX:
        o << "LEGACYMIX    = ";
        break;

    case RXRESOLUTION:
        o << "RXRESOLUTION = ";
        
    default:
        o << "UNKNOWN      = ";
        break;
    }

    o << std::dec << value;
}
