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

#ifndef RTCP_PACKET_H
#define RTCP_PACKET_H

#include <string.h>
#include <stdint.h>
#include <sstream>

#include "tip_constants.h"
#include "rtcp_tip_types.h"
#include "packet_buffer.h"

namespace LibTip {
    
    class CRtcpPacket {
    public:
        CRtcpPacket();
        virtual ~CRtcpPacket();
    
        /* get version */
        enum { RTCP_VERSION = 2 };
        uint8_t GetVersion() const { return ((mRtcpHeader.vps & VPS_VERSION_MASK) >> VPS_VERSION_SHIFT); }

        /* get padding bit */
        uint8_t GetPadding() const { return ((mRtcpHeader.vps & VPS_PADDING_MASK) >> VPS_PADDING_SHIFT); }

        /* set and get subtype */
        void SetSubType(uint8_t type) {
            mRtcpHeader.vps &= ~VPS_SUBTYPE_MASK;
            mRtcpHeader.vps |= ((type << VPS_SUBTYPE_SHIFT) & VPS_SUBTYPE_MASK);
        }
        uint8_t GetSubType() const { return ((mRtcpHeader.vps & VPS_SUBTYPE_MASK) >> VPS_SUBTYPE_SHIFT); }

        /* set and get type field */
        enum {
            SR    = 200,
            RR    = 201,
            SDES  = 202,
            BYE   = 203,
            APP   = 204,
            RTPFB = 205,
            PSFB  = 206,
            XR    = 207
        };
        void SetType(uint8_t type) { mRtcpHeader.type = type; }
        uint8_t GetType() const { return mRtcpHeader.type; }

        /* functions to convert between # of bytes and RTCP length */
        static uint32_t BytesToRtcpLength(uint32_t numBytes);
        static uint32_t RtcpLengthToBytes(uint32_t rtcpLength);

        /* return rtcp length field, lengths are in RTCP length units */
        uint16_t GetLength() const { return mRtcpHeader.length; }

        /* non-virtual functions for writing and reading data from a
           packet (network) buffer.  derived classes should not
           inherit these but inherit the PackData and UnpackData
           version instead (which are called by these functions). */
        uint32_t Pack(CPacketBuffer& buffer) const;

        uint32_t GetPackSize() const;
    
        int Unpack(CPacketBuffer& buffer);

        // stream the contents of this packet (human readable) to the
        // given stream.  mType if present will cause the printed data
        // to be media specific.
        virtual void ToStream(std::ostream& o, MediaType mType = MT_MAX) const;
        
    protected:
        // virtual pack and unpack functions.  derived classes should
        // inherit these and implement their own data packing and
        // unpacking.  prior to that they should call their base
        // class' version of these functions (i.e.
        // BaseClass::PackData()).
        virtual uint32_t PackData(CPacketBuffer& buffer) const;
        virtual int UnpackData(CPacketBuffer& buffer);

        // virtual clear function.  this function is called
        // immediately before UnpackData() allowing a derived class to
        // clear any dynamically allocated data as this data should be
        // replaced by the data being unpacked.
        virtual void ClearData();
        
        // pad a packed buffer to 4 byte alignment
        void Pad(CPacketBuffer& buffer) const;
        
        // set the size of the packet, also sets the rtcp length
        void SetSize(uint32_t size);

        // increment/decrement the size of the packet, also sets the rtcp length
        void IncrSize(uint32_t size) { SetSize(mSize + size); }
        void DecrSize(uint32_t size) { SetSize(mSize - size); }
    
        struct RtcpHeader {
            uint8_t  vps;
            uint8_t  type;
            uint16_t length;
        };

        enum {
            VPS_VERSION_SHIFT = 6,
            VPS_VERSION_MASK  = 0xC0,

            VPS_PADDING_SHIFT = 5,
            VPS_PADDING_MASK  = 0x20,

            VPS_SUBTYPE_SHIFT = 0,
            VPS_SUBTYPE_MASK  = 0x1F,
        };

        RtcpHeader mRtcpHeader;

    private:
        uint32_t     mSize;
    };

    // a base RTCP packet does not have an SSRC in it, but most
    // derived types do so this class provides that functionality in one
    // place.
    class CRtcpPacketSSRC : public CRtcpPacket {
    public:
        CRtcpPacketSSRC();
        virtual ~CRtcpPacketSSRC();

        /* set and get SSRC field */
        void SetSSRC(uint32_t ssrc) { mSSRC = ssrc; }
        uint32_t GetSSRC() const { return mSSRC; }

    protected:
        virtual uint32_t PackData(CPacketBuffer& buffer) const;
        virtual int UnpackData(CPacketBuffer& buffer);
        
        uint32_t    mSSRC;
    };

    class CRtcpAppPacket : public CRtcpPacketSSRC {
    public:
        CRtcpAppPacket();
        virtual ~CRtcpAppPacket();

        /* set and get RTCP app name */
        enum {
            RTCP_APPNAME_LENGTH = 4
        };
        struct RtcpAppName {
            char mName[RTCP_APPNAME_LENGTH];
        };
        void SetAppName(const char* const name) {
            memcpy(mAppName.mName, name, RTCP_APPNAME_LENGTH);
        }
        void SetAppName(RtcpAppName& name) {
            SetAppName(name.mName);
        }
        RtcpAppName GetAppName() const { return mAppName; }

    protected:
        virtual uint32_t PackData(CPacketBuffer& buffer) const;

        virtual int UnpackData(CPacketBuffer& buffer);
    
        RtcpAppName mAppName;
    };

    class CRtcpTipPacket : public CRtcpAppPacket {
    public:
        CRtcpTipPacket(TipPacketType type);
        virtual ~CRtcpTipPacket();

        // get/set ntp time
        void SetNtpTime(uint64_t ntpTime) {
            mTipHeader.mNtpTime = ntpTime;
        }
        uint64_t GetNtpTime() const {
            return mTipHeader.mNtpTime;
        }

        // function to get information about the subtype
        virtual TipPacketType GetTipPacketType() const;
        virtual const char* const GetTipPacketTypeString() const;
    
        // is the passed in packet my ACK?
        virtual bool IsMyAck(const CRtcpTipPacket& ack) const;

        // stream the contents of this packet (human readable) to the
        // given stream.  mType if present will cause the printed data
        // to be media specific.
        virtual void ToStream(std::ostream& o, MediaType mType = MT_MAX) const;

        // convert the given positions to a string and insert into the
        // given stream.
        static void PositionsToStream(std::ostream& o, uint16_t pos, MediaType mType);
        
    protected:
        virtual uint32_t PackData(CPacketBuffer& buffer) const;

        virtual int UnpackData(CPacketBuffer& buffer);

        struct RtcpTipHeader {
            uint64_t mNtpTime;
        };
        RtcpTipHeader mTipHeader;
    };

};

#endif /* RTCP_PACKET_H_ */
