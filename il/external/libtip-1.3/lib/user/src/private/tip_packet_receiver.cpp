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
#include "private/tip_packet_receiver.h"
using namespace LibTip;

CTipPacketReceiver::CTipPacketReceiver()
{
    memset(mReceiveData, 0, sizeof(mReceiveData));
}

CTipPacketReceiver::~CTipPacketReceiver()
{
    for (uint32_t i = 0; i < MAX_PACKET_TYPE; i++) {
        delete mReceiveData[i].mLastPacket;
        delete mReceiveData[i].mLastAckPacket;
    }
}

CTipPacketReceiver::Action
CTipPacketReceiver::ProcessPacket(CRtcpTipPacket* packet)
{
    // a few possible things that can happen...
    //
    // 1. the timestamp in the packet is older than the last received
    // packet of this type.  so we drop the packet.
    //
    // 2. the timestamp in the packet is newer than the last received
    // packet of this type or this is the first packet of this type.
    // process it as a new packet.
    //
    // 3. the timestamp in the packet is the same as the last received
    // packet of this type.
    //
    // 3a.  the timestamp in the packet is not the same as the last
    // acked packet of this type or we have never acked this type.
    // drop this packet.
    // 
    // 3b.  the timestamp in the packet is the same as the last acked
    // packet of this type.  process it as a duplicate packet.

    uint64_t timestamp = packet->GetNtpTime();
    uint32_t pType = packet->GetTipPacketType();

    if (timestamp < mReceiveData[pType].mLastReceived) {
        // case #1
        return AMPR_DROP;
        
    } else if (timestamp > mReceiveData[pType].mLastReceived ||
               mReceiveData[pType].mNumUniqueReceived == 0) {
        
        // case #2
        mReceiveData[pType].mLastReceived = timestamp;

        if (mReceiveData[pType].mLastAckPacket == NULL) {
            AMDEBUG(RECV, ("deleting un-acked packet type %s",
                           packet->GetTipPacketTypeString()));
        }

        delete mReceiveData[pType].mLastPacket;
        mReceiveData[pType].mLastPacket = packet;

        delete mReceiveData[pType].mLastAckPacket;
        mReceiveData[pType].mLastAckPacket = NULL;

        mReceiveData[pType].mNumUniqueReceived++;
        
        return AMPR_NEW;

    } else {
        // case #3

        if (timestamp != mReceiveData[pType].mLastAcked ||
            mReceiveData[pType].mNumUniqueAcked == 0) {
        
            // case #3a
            return AMPR_DROP;

        } else {

            // case #3b
            return AMPR_DUP;

        }
    }
}

void CTipPacketReceiver::RegisterAck(const CRtcpTipPacket& packet,
                                     CRtcpTipPacket* ack)
{
    // only record the timestamp if it is newer
    uint64_t timestamp = packet.GetNtpTime();
    uint32_t pType = packet.GetTipPacketType();

    // if this is a new ACK or if this is our first ACK
    if (timestamp > mReceiveData[pType].mLastAcked ||
        mReceiveData[pType].mNumUniqueAcked == 0) {
        
        mReceiveData[pType].mLastAcked = timestamp;

        delete mReceiveData[pType].mLastAckPacket;
        mReceiveData[pType].mLastAckPacket = ack;

        mReceiveData[pType].mNumUniqueAcked++;

    } else if (timestamp < mReceiveData[pType].mLastAcked) {
        // an old ack, free up the ack packet as we are responsible
        // for it
        delete ack;

    } else {
        // a duplicate ack, if this is the same ack as we had before
        // then do nothing.  if its a new ack then free it.
        if (ack != mReceiveData[pType].mLastAckPacket) {
            delete ack;
        }
    }
}

CRtcpTipPacket* CTipPacketReceiver::FindDupAck(const CRtcpTipPacket& packet)
{
    uint32_t pType = packet.GetTipPacketType();
    return mReceiveData[pType].mLastAckPacket;
}

CRtcpTipPacket* CTipPacketReceiver::Find(TipPacketType pType)
{
    if (pType >= MAX_PACKET_TYPE) {
        return NULL;
    }

    return mReceiveData[pType].mLastPacket;
}

void CTipPacketReceiver::Forget(TipPacketType pType)
{
    if (pType >= MAX_PACKET_TYPE) {
        return;
    }
    
    mReceiveData[pType].mLastReceived = 0;
    mReceiveData[pType].mLastAcked    = 0;
    
    delete mReceiveData[pType].mLastPacket;
    mReceiveData[pType].mLastPacket = NULL;

    delete mReceiveData[pType].mLastAckPacket;
    mReceiveData[pType].mLastAckPacket = NULL;
}

CRtcpTipPacket* CTipPacketReceiver::FindID(void* id)
{
    for (uint32_t i = 0; i < MAX_PACKET_TYPE; i++) {
        if (mReceiveData[i].mLastPacket == id) {
            return mReceiveData[i].mLastPacket;
        }
    }

    return NULL;
}
