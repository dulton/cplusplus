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

#include "tip_time.h"
#include "rtcp_rr_packet.h"
#include "rtcp_sdes_packet.h"
#include "rtcp_tip_packet_manager.h"
using namespace LibTip;

CTipPacketManager::CTipPacketManager(uint32_t intervalMS, uint32_t maxTx)
{
    mTxInterval = intervalMS;
    mTxMax = maxTx;
    mNextTxTime = 0;
    mWrapper = true;
    mWrapperSSRC = 0;
    mPacketListTxIterator = mPacketList.end();
}

CTipPacketManager::~CTipPacketManager()
{
    // free any pending packet pointers here
    while (! mPacketList.empty()) {
        PacketEntry pe = mPacketList.front();
        mPacketList.pop_front();

        delete pe.mpPacket;
        delete pe.mpBuffer;
    }
}

void CTipPacketManager::SetRetransmissionInterval(uint32_t intervalMS)
{
    mTxInterval = intervalMS;
}

void CTipPacketManager::SetRetransmissionLimit(uint32_t limit)
{
    mTxMax = limit;
}

void CTipPacketManager::EnableWrapper(uint32_t ssrc)
{
    mWrapper     = true;
    mWrapperSSRC = ssrc;
}

void CTipPacketManager::DisableWrapper()
{
    mWrapper = false;
}

int CTipPacketManager::Add(CRtcpTipPacket* packet)
{
    if (packet == NULL) {
        return -1;
    }
    
    PacketEntry pe;
    pe.mpPacket = packet;
    pe.mTxCount = 0;

    // if NTP timestamp is 0 then set timestamp of packet to now,
    // otherwise just leave it as is
    if (packet->GetNtpTime() == 0) {
        packet->SetNtpTime(GetNtpTimestamp());
    }
    
    // pack this packet once during the Add() so we don't have to do
    // it each time we retransmit
    pe.mpBuffer = new CPacketBufferData();
    if (pe.mpBuffer == NULL) {
        return -1;
    }

    // pack packet plus any needed wrappers into the buffer
    Pack(*packet, *pe.mpBuffer);

    mPacketList.push_back(pe);
    
    if (mPacketList.size() == 1) {
        mPacketListTxIterator = mPacketList.begin();

        // schedule ourselves to run now
        mNextTxTime = GetMsecTimestamp();
    }

    return 0;
}

CRtcpTipPacket* CTipPacketManager::Ack(const CRtcpTipPacket& ack)
{
    CRtcpPacketList::iterator i;
    for (i = mPacketList.begin(); i != mPacketList.end(); ++i) {
        if ((*i).mpPacket->IsMyAck(ack)) {
            CRtcpTipPacket* ret = (*i).mpPacket;
            Remove(i);
            return ret;
        }
    }

    return NULL;
}

CRtcpTipPacket* CTipPacketManager::Remove(TipPacketType pType)
{
    CRtcpPacketList::iterator i = FindPacket(pType);

    if (i != mPacketList.end()) {
        CRtcpTipPacket* ret = (*i).mpPacket;
        Remove(i);
        return ret;
    }

    return NULL;
}

CRtcpTipPacket* CTipPacketManager::Find(TipPacketType pType)
{
    CRtcpPacketList::iterator i = FindPacket(pType);

    if (i != mPacketList.end()) {
        return (*i).mpPacket;
    }

    return NULL;
}

CTipPacketManager::CRtcpPacketList::iterator
CTipPacketManager::FindPacket(TipPacketType pType)
{
    CRtcpPacketList::iterator i;

    for (i = mPacketList.begin(); i != mPacketList.end(); ++i) {
        if ((*i).mpPacket->GetTipPacketType() == pType) {
            break;
        }
    }

    return i;
}

void CTipPacketManager::Remove(CRtcpPacketList::iterator iter) {
    if (mPacketListTxIterator == iter) {
        ++mPacketListTxIterator;
    }

    // free memory associated with packet buffer
    delete (*iter).mpBuffer;

    mPacketList.erase(iter);

    // if this was the last packet in the list then we have nothing to do
    if (mPacketList.size() == 0) {
        mNextTxTime = 0;
    }
}

CRtcpTipPacket* CTipPacketManager::GetPacket(bool& expired, CPacketBuffer** buffer)
{
    // arg check
    if (buffer == NULL) {
        return NULL;
    }
    
    // just in case we get called with an empty list...
    if (mPacketList.size() == 0) {
        *buffer = NULL;
        return NULL;
    }

    if (mPacketListTxIterator == mPacketList.end()) {
        // end of the road, reset to the beginning and return NULL
        mPacketListTxIterator = mPacketList.begin();
        mNextTxTime = GetMsecTimestamp() + (uint64_t) mTxInterval;
        *buffer = NULL;
        return NULL;
    }

    CRtcpTipPacket* ret = (*mPacketListTxIterator).mpPacket;

    if ((*mPacketListTxIterator).mTxCount >= mTxMax) {
        // too many xmits on this guy, remove from list and return as expired
        expired = true;
        *buffer = NULL;
        
        Remove(mPacketListTxIterator);
    } else {
        expired = false;
        *buffer = (*mPacketListTxIterator).mpBuffer;
        
        (*mPacketListTxIterator).mTxCount++;
        ++mPacketListTxIterator;
    }

    return ret;
}

uint64_t CTipPacketManager::GetNextTransmitTime() const
{
    if (mNextTxTime == 0) {
        // nothing to do
        return (uint64_t) -1;
    }
    
    uint64_t now = GetMsecTimestamp();
    if (now > mNextTxTime) {
        return 0;
    }

    return (mNextTxTime - now);
}

void CTipPacketManager::Pack(const CRtcpPacket& packet, CPacketBuffer& buffer)
{
    if (mWrapper) {
        CRtcpRRPacket rr;
        rr.SetSSRC(mWrapperSSRC);
        rr.Pack(buffer);

        CRtcpSDESPacket sdes;
        sdes.AddChunk(mWrapperSSRC);
        sdes.Pack(buffer);
    }
    
    packet.Pack(buffer);
}
