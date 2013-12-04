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

#ifndef RTCP_TIP_MUXCTRL_PACKET_H
#define RTCP_TIP_MUXCTRL_PACKET_H

#include <list>

#include "rtcp_tip_tlv.h"
#include "rtcp_packet.h"

namespace LibTip {

    class CRtcpAppMuxCtrlPacketBase : public CRtcpTipPacket {
    public:
        CRtcpAppMuxCtrlPacketBase(uint8_t version);
        virtual ~CRtcpAppMuxCtrlPacketBase() = 0;

        // get the version
        uint8_t GetVersion() const {
            return ((mCtrl.mvp & CTRL_VERSION_MASK) >> CTRL_VERSION_SHIFT);
        }
        void SetVersion(uint8_t version) {
            mCtrl.mvp &= ~CTRL_VERSION_MASK;
            mCtrl.mvp |= ((version << CTRL_VERSION_SHIFT) & CTRL_VERSION_MASK);
        }

        // set/get the profile
        enum {
            SECURE_PROFILE   = (1 << 0),
            FEEDBACK_PROFILE = (1 << 1)
        };
        enum {
            AVP   = 0,
            SAVP  = (SECURE_PROFILE),
            AVPF  = (FEEDBACK_PROFILE),
            SAVPF = (SECURE_PROFILE | FEEDBACK_PROFILE),
        };
        void SetProfile(uint8_t profile) {
            mCtrl.mvp &= ~CTRL_PROFILE_MASK;
            mCtrl.mvp |= ((profile << CTRL_PROFILE_SHIFT) & CTRL_PROFILE_MASK);
        }
        uint8_t GetProfile() const {
            return ((mCtrl.mvp & CTRL_PROFILE_MASK) >> CTRL_PROFILE_SHIFT);
        }		
        
        // set/get the options
        enum {
            IS_FOCUS = 0x01,
            IS_TRANSCODER = 0x02,
        };
        void SetOptions(uint8_t options) {
            mCtrl.options = options;
        }
        uint8_t GetOptions() const {
            return mCtrl.options;
        }
        
        // set/get the number of xmit streams
        void SetNumXmit(uint8_t numXmit) {
            mCtrl.numXmit = numXmit;
        }
        uint8_t GetNumXmit() const {
            return mCtrl.numXmit;
        }
	
        // set/get the number of rcv streams
        void SetNumRcv(uint8_t numRcv) {
            mCtrl.numRcv = numRcv;
        }
        uint8_t GetNumRcv() const {
            return mCtrl.numRcv;
        }

        // set/get the conference id
        void SetConfID(uint64_t confID) {
            mCtrl.confID = confID;
        }
        uint64_t GetConfID() const {
            return mCtrl.confID;
        }

        // set/get the xmit positions
        void SetXmitPositions(uint16_t xmitPos) {
            mCtrl.xmitPositions = xmitPos;
        }
        uint16_t GetXmitPositions() const {
            return mCtrl.xmitPositions;
        }
	
        // set/get the rcv positions
        void SetRcvPositions(uint16_t rcvPos) {
            mCtrl.rcvPositions = rcvPos;
        }
        uint16_t GetRcvPositions() const {
            return mCtrl.rcvPositions;
        }

        virtual void ToStream(std::ostream& o, MediaType mType = MT_MAX) const;

    protected:

        // pack the component data into the packet buffer.
        virtual uint32_t PackData(CPacketBuffer& buffer) const;

        // unpack from a network packet buffer
        virtual int UnpackData(CPacketBuffer& buffer);
    
        enum {
            CTRL_VERSION_MASK  = 0xF0,
            CTRL_VERSION_SHIFT = 4,
            CTRL_PROFILE_MASK  = 0x0F,
            CTRL_PROFILE_SHIFT = 0,
        };
        struct RtcpTipCtrl {
            uint8_t   mvp;
            uint8_t   options;
            uint8_t   numXmit;
            uint8_t   numRcv;
            // note we do not add xmitNtpTime here b/c it is tracked by CRtcpTipPacket
            uint16_t  xmitPositions;
            uint16_t  rcvPositions;
            // note confID out of order to preserve 64 bit alignment
            uint64_t  confID;
        };
        RtcpTipCtrl mCtrl;
    };

    class CRtcpAppMuxCtrlPacket : public CRtcpAppMuxCtrlPacketBase {
    public:
        CRtcpAppMuxCtrlPacket();
        virtual ~CRtcpAppMuxCtrlPacket();

        // default MuxCtrl packet version used by this class
        static const uint8_t DEFAULT_VERSION = 6;

    protected:
        virtual int UnpackData(CPacketBuffer& buffer);
    };
    
    class CRtcpAppMuxCtrlV7Packet : public CRtcpAppMuxCtrlPacketBase {
    public:
        CRtcpAppMuxCtrlV7Packet();
        virtual ~CRtcpAppMuxCtrlV7Packet();

        // default version used by this packet.
        static const uint8_t DEFAULT_VERSION = 7;

        // set/get the number of shared positions
        void SetNumShared(uint8_t numShared) {
            mCtrlV7.numSharedPositions = numShared;
        }
        uint8_t GetNumShared() const {
            return mCtrlV7.numSharedPositions;
        }

        // set/get the shared positions
        void SetSharedPositions(uint16_t sharedPos) {
            mCtrlV7.sharedPositions = sharedPos;
        }
        uint16_t GetSharedPositions() const {
            return mCtrlV7.sharedPositions;
        }

        // get/set a participant ID (TLV).  if id is NULL, clear
        // participant id.  get returns the length of the id, and value of
        // id in parameter if non-NULL
        int SetParticipantID(const uint8_t* id, uint32_t len);
        uint32_t GetParticipantID(uint8_t* id) const;
	
        void ToStream(std::ostream& o, MediaType mType = MT_MAX) const;
        
    protected:

        // pack the component data into the packet buffer.
        virtual uint32_t PackData(CPacketBuffer& buffer) const;

        // unpack from a network packet buffer
        virtual int UnpackData(CPacketBuffer& buffer);

        // clear out dynamic data prior to unpack
        virtual void ClearData();
        
        /* find a specific TLV by tag */
        const CTipTlv* FindTLV(uint8_t tag) const;
    
        struct RtcpTipCtrlV7 {
            uint8_t   numSharedPositions;
            uint8_t   reserved;
            uint16_t  sharedPositions;
        };
        RtcpTipCtrlV7 mCtrlV7;

        enum {
            RESERVED_TAG = 0,
            PARTICIPANT_ID_TAG = 1,
        };
        typedef std::list<CTipTlv*> CTlvpList;
        CTlvpList mTlv;

    private:
        // no copy of assignment
        CRtcpAppMuxCtrlV7Packet(const CRtcpAppMuxCtrlV7Packet&);
        CRtcpAppMuxCtrlV7Packet& operator=(const CRtcpAppMuxCtrlV7Packet&);
    };
};

#endif
