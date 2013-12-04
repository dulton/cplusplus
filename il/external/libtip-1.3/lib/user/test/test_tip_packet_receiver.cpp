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
#include "rtcp_tip_ack_packet.h"
#include "private/tip_packet_receiver.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CTipPacketReceiverTest : public CppUnit::TestFixture {
private:
    CTipPacketReceiver* pr;
    CRtcpTipPacket* packet;

public:
    void setUp() {
        // turn off debug prints
        gDebugFlags = 0;

        pr = new CTipPacketReceiver();
        CPPUNIT_ASSERT( pr != NULL );

        packet = new CRtcpTipPacket(MUXCTRL);
        CPPUNIT_ASSERT( packet != NULL );
    }

    void tearDown() {
        delete pr;

        // don't delete packet here, the receiver will own the packet
        // after ProcessPacket() is called.
    }

    void testNew() {
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_NEW );
    }

    void testNew2() {
        packet->SetNtpTime(1);
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_NEW );

        // add of a second new packet will cause the first to be deleted
        packet = new CRtcpTipPacket(MUXCTRL);
        packet->SetNtpTime(2);
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_NEW );
    }

    void testOld() {
        packet->SetNtpTime(1);
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_NEW );

        packet->SetNtpTime(0);
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_DROP );
    }

    void testDup() {
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_NEW );

        CRtcpTipAckPacket* ack = new CRtcpTipAckPacket(*packet);
        pr->RegisterAck(*packet, ack);

        // RegisterAck will delete the packet, so create a new one.
        packet = new CRtcpTipPacket(MUXCTRL);
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_DUP );
        CPPUNIT_ASSERT_EQUAL( pr->FindDupAck(*packet), (CRtcpTipPacket*) ack );

        // receiver does not own this packet as its a dup
        delete packet;
    }

    void testFindDup() {
        packet->SetNtpTime(1);
        CPPUNIT_ASSERT_EQUAL( pr->FindDupAck(*packet), (CRtcpTipPacket*) NULL );
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_NEW );
        CPPUNIT_ASSERT_EQUAL( pr->FindDupAck(*packet), (CRtcpTipPacket*) NULL );

        CRtcpTipAckPacket* ack = new CRtcpTipAckPacket(*packet);
        pr->RegisterAck(*packet, ack);

        // RegisterAck will delete the packet, so create a new one.
        packet = new CRtcpTipPacket(MUXCTRL);
        packet->SetNtpTime(1);
        CPPUNIT_ASSERT_EQUAL( pr->FindDupAck(*packet), (CRtcpTipPacket*) ack );
        
        packet->SetNtpTime(2);
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_NEW );
        CPPUNIT_ASSERT_EQUAL( pr->FindDupAck(*packet), (CRtcpTipPacket*) NULL );
    }
    
    void testNoAck() {
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_NEW );
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_DROP );
    }        

    void testAckTwice() {
        packet->SetNtpTime(1);
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_NEW );

        CRtcpTipAckPacket* ack = new CRtcpTipAckPacket(*packet);
        pr->RegisterAck(*packet, ack);

        // RegisterAck will delete the packet, so create a new one.
        packet = new CRtcpTipPacket(MUXCTRL);
        packet->SetNtpTime(1);
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_DUP );

        // register the same ACK
        pr->RegisterAck(*packet, ack);
        CPPUNIT_ASSERT_EQUAL( pr->FindDupAck(*packet), (CRtcpTipPacket*) ack );

        // receiver does not own this packet as its a dup
        delete packet;
    }

    void testFind() {
        packet->SetNtpTime(1);
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_NEW );

        CPPUNIT_ASSERT_EQUAL( pr->Find(packet->GetTipPacketType()), packet );
    }
    
    void testForget() {
        packet->SetNtpTime(1);
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_NEW );

        pr->Forget(packet->GetTipPacketType());

        // Forget() will delete the old packet so create a new one
        // with the same ntp time
        packet = new CRtcpTipPacket(MUXCTRL);
        CPPUNIT_ASSERT( packet != NULL );

        packet->SetNtpTime(1);
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_NEW );
    }
    
    void testFindID() {
        packet->SetNtpTime(1);
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_NEW );
        CPPUNIT_ASSERT_EQUAL( pr->FindID(packet), packet );
    }
    
    void testFindIDInv() {
        CPPUNIT_ASSERT( pr->FindID(NULL) == NULL );

        // we didn't use packet so free it
        delete packet;
    }

    void testFindIDInv2() {
        uint32_t dummy;
        
        packet->SetNtpTime(1);
        CPPUNIT_ASSERT_EQUAL( pr->ProcessPacket(packet), CTipPacketReceiver::AMPR_NEW );
        CPPUNIT_ASSERT( pr->FindID(&dummy) == NULL );
    }        
    
    CPPUNIT_TEST_SUITE( CTipPacketReceiverTest );
    CPPUNIT_TEST( testNew );
    CPPUNIT_TEST( testNew2 );
    CPPUNIT_TEST( testOld );
    CPPUNIT_TEST( testDup );
    CPPUNIT_TEST( testFindDup );
    CPPUNIT_TEST( testNoAck );
    CPPUNIT_TEST( testAckTwice );
    CPPUNIT_TEST( testFind );
    CPPUNIT_TEST( testForget );
    CPPUNIT_TEST( testFindID );
    CPPUNIT_TEST( testFindIDInv );
    CPPUNIT_TEST( testFindIDInv2 );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CTipPacketReceiverTest );
