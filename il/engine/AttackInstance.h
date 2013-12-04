/// @file
/// @brief Attack instance header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ATTACK_INSTANCE_H_
#define _ATTACK_INSTANCE_H_

#include "FlowConfig.h"
#include "PktLoopHandler.h"

class TestFlowInstance;

#ifdef UNIT_TEST
#define UNIT_TEST_VIRTUAL virtual
#define UNIT_TEST_PRIVATE protected
#else
#define UNIT_TEST_VIRTUAL
#define UNIT_TEST_PRIVATE private
#endif

DECL_L4L7_ENGINE_NS

const uint8_t ATTACK_DEFAULT_RESPONSE_DATA[2] = {0x2D, 0x0A};
const std::vector<uint8_t> ATTACK_DEFAULT_RESPONSE_DATA_VECTOR(ATTACK_DEFAULT_RESPONSE_DATA, ATTACK_DEFAULT_RESPONSE_DATA + 2);
const FlowConfig::Packet ATTACK_DEFAULT_RESPONSE = {ATTACK_DEFAULT_RESPONSE_DATA_VECTOR, 0, false, 0};

///////////////////////////////////////////////////////////////////////////////

class AttackInstance: public FlowInstance
{
  public:

    AttackInstance(FlowEngine& engine, handle_t flowHandle, PlaylistInstance *pi, size_t flowIdx, bool client, flowLpHdlrSharedPtr_t loopHandler);

    AttackInstance(FlowInstance & flow);

    void Start();

  UNIT_TEST_PRIVATE:
    void DoNextPacket();

private:
    void Reset();
    void Initialize();
    void TCPStart();
    void UDPStart();
    void WaitForSend(uint32_t delay);
    void SendPacket(const FlowConfig::Packet* packet, uint32_t delay);
    void WaitForIncomingPacket(const FlowConfig::Packet* packet);
    void PacketReceived();
    const FlowConfig::Packet * GetNextPktToSend();
    const FlowConfig::Packet * GetNextPktToReceive();

    // Check if it's stateless(UDP) or stateful(TCP)
    FlowConfig::eLayer GetAttackType(void) { return mConfig->layer; };

    uint32_t                      mWaitTimeout;
    uint32_t                      mInactivityTimeout; // used only for udp attack

    const std::vector<uint16_t>& mClientPayload;
    const std::vector<uint16_t>& mServerPayload;
    uint16_t             mCurrentClientPayloadIndex;
    uint16_t             mCurrentServerPayloadIndex;
    uint16_t             mNumOfPktsToSend; // Not pkts already sent, but number of pkts that needs to send
    uint16_t             mNumOfPktsReceived; // Only client need this
    uint16_t             mNumOfResponsesExpected;
    bool                 mGracePeriodStarted;

    // For attacks, tx and Rx are independent, so they should have seperate pkt loop handlers
    PktLoopHandler mTxPktLoopHandler;
    PktLoopHandler mRxPktLoopHandler;

#ifdef UNIT_TEST
    friend class TestAttackInstance;
    friend class TestPktLoopHandler;
#endif
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
