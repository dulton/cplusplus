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

#ifndef RTCP_TIP_MEDIAOPTS_PACKET_H
#define RTCP_TIP_MEDIAOPTS_PACKET_H

#include <list>
#include <map>

#include "rtcp_packet.h"

namespace LibTip {

    class CRtcpAppMediaoptsPacket : public CRtcpTipPacket {
    public:
        enum Version {
            MINIMUM_VERSION = 2,
            MAXIMUM_VERSION = 3 // default
        };

        CRtcpAppMediaoptsPacket(Version version = MAXIMUM_VERSION);
        virtual ~CRtcpAppMediaoptsPacket();

        // get the version
        uint16_t GetVersion() const { return mBase.version; }

        enum AudioOption {
            ACTIVITY_METRIC       = 0x00000001,
            DYNAMIC_OUTPUT        = 0x00000002,
            CAPABLE_G722_LEGACY   = 0x00000004,
            USING_G722_LEGACY     = 0x00000008,
        };
    
        enum VideoOption {
            REFRESH_FLAG          = 0x00000001,
            INBAND_PARAM_SETS     = 0x00000002,
            CABAC                 = 0x00000004,
            LTRP                  = 0x00000008,
            AUX_BIT0              = 0x00000020,
            GDR                   = 0x00000100,
            AUX_BIT1              = 0x00000200,
            HP_8X8_TRANSFORMS     = 0x00000400,
            UNRESTRICTED_XGA_1_5  = 0x00000800,
            UNRESTRICTED_720P     = 0x00001000,
            UNRESTRICTED_1080P    = 0x00002000,
            UNRESTRICED_XGA_30    = 0x00004000,
        };

        // flag byte values enabled by REFRESH_FLAGS option above and
        // appended to the end of video RTP packets.
        enum RefreshFlags {
            NOT_A_REFRESH = 0,
            FIRST_PACKET_IDR,
            FIRST_PACKET_GDR,
            FIRST_PACKET_LTRP0_CANDIDATE,
            FIRST_PACKET_LTRP1_CANDIDATE,
            FIRST_PACKET_LTRP0_REPAIR,
            FIRST_PACKET_LTRP1_REPAIR,
            FIRST_PACKET_NON_LTRP_REPAIR,
        };
        
        enum SharedOption {
            EKT                   = 0x02000000,
        };

        enum AuxFrameRate {
            AUX_1FPS  = 0,
            AUX_5FPS,
            AUX_30FPS
        };

        static uint32_t GetBitValueForAuxFrameRate(AuxFrameRate fr);
        static AuxFrameRate GetAuxFrameRateForBitValue(uint32_t bv);
    
        // add/update an SSRC with options to the packet
        void AddSSRC(uint32_t ssrc, uint32_t xmitOpt, uint32_t rcvOpt);

        // returns list of SSRCs plus xmit and rcv options contained in
        // this packet
        std::list<uint32_t> GetAllSSRC() const;
    
        // get the options associated with the given SSRC
        int GetSSRC(uint32_t ssrc, uint32_t& xmitOpt, uint32_t& rcvOpt) const;
    
        enum OptionTag {
            RESERVED     = 0,
            PROFILE      = 1,
            LEGACYMIX    = 5,
            RXRESOLUTION = 6,
            LASTTAG      = 255
        };

        // values used with the profile option tag above
        enum ProfileValue {
            PROFILE_RESERVED = 0,
            PROFILE_SATELLITE,
            PROFILE_PUBLIC_INTERNET
        };
        
        // add an option tag/value pair for the given SSRC to the packet.
        int AddOption(uint32_t ssrc, OptionTag optTag, uint32_t optValue);

        // returns list of option tags associated with an SSRC
        std::list<OptionTag> GetOptions(uint32_t ssrc) const;
    
        // get the option associated with the given SSRC and tag
        int GetOption(uint32_t ssrc, OptionTag optTag, uint32_t& optValue) const;

        void ToStream(std::ostream& o, MediaType mType = MT_MAX) const;
        
    protected:

        // pack the component data into the packet buffer.
        virtual uint32_t PackData(CPacketBuffer& buffer) const;

        // unpack from a network packet buffer
        virtual int UnpackData(CPacketBuffer& buffer);
    
        // clear out dynamic data prior to unpack
        virtual void ClearData();
        
        // helper functions to unpack a network packet
        int UnpackSSRC(CPacketBuffer& buffer);
        int UnpackSSRCV2(CPacketBuffer& buffer);
        int UnpackOption(uint32_t ssrc, CPacketBuffer& buffer);

        // helper stream function
        void OptionsToStream(std::ostream& o, uint32_t opt, MediaType mType) const;
        void OptionTagValueToStream(std::ostream& o, OptionTag tag,
                                    uint32_t value) const;
        
        struct RtcpAppMOBase {
            uint16_t version;
            uint16_t reserved;
        };
        RtcpAppMOBase mBase;
        
        enum {
            OPT_TAG_SHIFT = 24,
            OPT_TAG_MASK  = 0xFF000000,
            OPT_VAL_SHIFT = 0,
            OPT_VAL_MASK  = 0x00FFFFFF,
        };

        typedef std::map<OptionTag, uint32_t> OptionMap;
        struct RtcpAppMOSsrc {
            uint32_t  xmitOptions;
            uint32_t  rcvOptions;
            OptionMap optMap;
        };
        typedef std::map<uint32_t, RtcpAppMOSsrc> SsrcMap;
        SsrcMap mSsrcMap;
    };

};

#endif
