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

#ifndef RTCP_TIP_PACKET_MANAGER_H
#define RTCP_TIP_PACKET_MANAGER_H

#include <list>

#include "rtcp_tip_types.h"
#include "rtcp_packet.h"
#include "rtcp_tip_ack_packet.h"
#include "packet_buffer.h"

namespace LibTip {

    class CTipPacketManager {
    public:
        CTipPacketManager(uint32_t intervalMS = 250, uint32_t maxTx = 40);
        ~CTipPacketManager();

        // set and get retranmission parameters
        void SetRetransmissionInterval(uint32_t intervalMS);
        void SetRetransmissionLimit(uint32_t limit);

        uint32_t GetRetransmissionInterval() const { return mTxInterval; }
        uint32_t GetRetransmissionLimit() const { return mTxMax; }

        uint64_t GetPacketTimeoutMsec() const { return mTxInterval * mTxMax; }

        // enable packing each TipPacket with an empty RR and SDES
        // prior to transmission
        void EnableWrapper(uint32_t ssrc);

        // disable wrappers
        void DisableWrapper();
        
        // add a new packet to be tracked, returns 0 if the packet was
        // successfully added.  packet memory is now owned by the
        // manager, will be returned to the user when the packet is
        // ack'ed or times out.
        int Add(CRtcpTipPacket* packet);

        // ack a packet in the queue removing it, returns the ACK'ed
        // packet or NULL if no packet was found
        CRtcpTipPacket* Ack(const CRtcpTipPacket& ack);

        // remove a packet from the queue, returns the removed packet.
        // NOTE only removes the first packet with matching pType
        CRtcpTipPacket* Remove(TipPacketType pType);

        // find a packet in the queue.  NOTE only finds the first packet
        // with the matching pType.
        CRtcpTipPacket* Find(TipPacketType pType);
    
        // get the next packet to be transmitted.  the packet's tx count
        // will be inremented each time it is retrieved.  if NULL is
        // returned, all packets have been transmitted for this interval.
        // the next call to GetPacket will start over at the first packet.
        // if expired returns as true then this packet has expired and
        // should be free'd.  the buffer pointer (cannot be NULL) will be
        // set to a network ready buffer containing the packet data.
        CRtcpTipPacket* GetPacket(bool& expired, CPacketBuffer** buffer);

        // get the amount of time (in milliseconds) between now and when
        // the next packets are scheduled to be transmitted.
        uint64_t GetNextTransmitTime() const;

        // place the given packet into the given buffer, adding
        // wrappers if configured to do so
        void Pack(const CRtcpPacket& packet, CPacketBuffer& buffer);
        
    protected:
        // interval between transmits (in milliseconds)
        uint32_t mTxInterval;
    
        // max number of transmits
        uint32_t mTxMax;

        // time of next transmit (in milliseconds)
        uint64_t mNextTxTime;

        // should we add the empty RR and SDES wrapper
        bool mWrapper;

        // SSRC used for empty RR and SDES wrappers
        uint32_t mWrapperSSRC;
        
        // structure used to manage an outgoing packet
        struct PacketEntry {
            CRtcpTipPacket*    mpPacket;
            CPacketBuffer*     mpBuffer;
            uint32_t           mTxCount;
        };
        typedef std::list<PacketEntry> CRtcpPacketList;
        CRtcpPacketList mPacketList;
        CRtcpPacketList::iterator mPacketListTxIterator;

        // helper functions
        CRtcpPacketList::iterator FindPacket(TipPacketType pType);
    
        void Remove(CRtcpPacketList::iterator iter);

    private:
        // no copy of assignment
        CTipPacketManager(const CTipPacketManager&);
        CTipPacketManager& operator=(const CTipPacketManager&);
    };

};

#endif
