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

#include "rtcp_rr_packet.h"
#include "rtcp_sdes_packet.h"
#include "rtcp_tip_muxctrl_packet.h"
#include "rtcp_tip_packet_manager.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CTipPacketManagerTest : public CppUnit::TestFixture {
private:
    CTipPacketManager* mgr;

public:
    void setUp() {
        mgr = new CTipPacketManager(250, 10);
        CPPUNIT_ASSERT( mgr != NULL );
    }

    void tearDown() {
        delete mgr;
    }
    
    void testCreate() {
        bool expired;
        CPacketBuffer* buffer;

        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == NULL );
        CPPUNIT_ASSERT( buffer == NULL );
    }

    void testInterval() {
        CPPUNIT_ASSERT_EQUAL( mgr->GetRetransmissionInterval(), (uint32_t) 250 );

        mgr->SetRetransmissionInterval(1000);
        CPPUNIT_ASSERT_EQUAL( mgr->GetRetransmissionInterval(), (uint32_t) 1000 );
    }
    
    void testLimit() {
        CPPUNIT_ASSERT_EQUAL( mgr->GetRetransmissionLimit(), (uint32_t) 10 );

        mgr->SetRetransmissionLimit(100);
        CPPUNIT_ASSERT_EQUAL( mgr->GetRetransmissionLimit(), (uint32_t) 100 );
    }
    
    void testAdd() {
        CRtcpAppMuxCtrlPacket* muxctrl = new CRtcpAppMuxCtrlPacket();
        bool expired;
        CPacketBuffer* buffer;

        CPPUNIT_ASSERT_EQUAL( mgr->Add(muxctrl), 0 );
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == muxctrl );
        CPPUNIT_ASSERT_EQUAL( expired, false );
        CPPUNIT_ASSERT( buffer != NULL );
    
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == NULL );

        // do it twice to verify looping
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == muxctrl );
        CPPUNIT_ASSERT_EQUAL( expired, false );
        CPPUNIT_ASSERT( buffer != NULL );
    
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == NULL );
    }

    void testAddMulti() {
        CRtcpAppMuxCtrlPacket* muxctrl1 = new CRtcpAppMuxCtrlPacket();
        CRtcpAppMuxCtrlPacket* muxctrl2 = new CRtcpAppMuxCtrlPacket();
        bool expired;
        CPacketBuffer* buffer;

        CPPUNIT_ASSERT_EQUAL( mgr->Add(muxctrl1), 0 );
        CPPUNIT_ASSERT_EQUAL( mgr->Add(muxctrl2), 0 );
    
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == muxctrl1 );
        CPPUNIT_ASSERT_EQUAL( expired, false );
        CPPUNIT_ASSERT( buffer != NULL );

        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == muxctrl2 );
        CPPUNIT_ASSERT_EQUAL( expired, false );
        CPPUNIT_ASSERT( buffer != NULL );

        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == NULL );

        // do it twice to verify looping
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == muxctrl1 );
        CPPUNIT_ASSERT_EQUAL( expired, false );
        CPPUNIT_ASSERT( buffer != NULL );

        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == muxctrl2 );
        CPPUNIT_ASSERT_EQUAL( expired, false );
        CPPUNIT_ASSERT( buffer != NULL );

        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == NULL );
    }

    void testAddMultiMid() {
        CRtcpAppMuxCtrlPacket* muxctrl1 = new CRtcpAppMuxCtrlPacket();
        CRtcpAppMuxCtrlPacket* muxctrl2 = new CRtcpAppMuxCtrlPacket();
        CRtcpAppMuxCtrlPacket* muxctrl3 = new CRtcpAppMuxCtrlPacket();
        bool expired;
        CPacketBuffer* buffer;

        CPPUNIT_ASSERT_EQUAL( mgr->Add(muxctrl1), 0 );
        CPPUNIT_ASSERT_EQUAL( mgr->Add(muxctrl2), 0 );
    
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == muxctrl1 );
        CPPUNIT_ASSERT_EQUAL( expired, false );
        CPPUNIT_ASSERT( buffer != NULL );

        // add 3rd after 1st has been retrieved
        CPPUNIT_ASSERT_EQUAL( mgr->Add(muxctrl3), 0 );
    
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == muxctrl2 );
        CPPUNIT_ASSERT_EQUAL( expired, false );
        CPPUNIT_ASSERT( buffer != NULL );

        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == muxctrl3 );
        CPPUNIT_ASSERT_EQUAL( expired, false );
        CPPUNIT_ASSERT( buffer != NULL );

        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == NULL );

        // do it twice to verify looping
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == muxctrl1 );
        CPPUNIT_ASSERT_EQUAL( expired, false );
        CPPUNIT_ASSERT( buffer != NULL );

        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == muxctrl2 );
        CPPUNIT_ASSERT_EQUAL( expired, false );
        CPPUNIT_ASSERT( buffer != NULL );

        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == muxctrl3 );
        CPPUNIT_ASSERT_EQUAL( expired, false );
        CPPUNIT_ASSERT( buffer != NULL );

        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == NULL );
    }

    void testAddInvalid() {
        CPPUNIT_ASSERT_EQUAL( mgr->Add(NULL), -1 );
    }

    void testGetInvalid() {
        CRtcpAppMuxCtrlPacket* muxctrl = new CRtcpAppMuxCtrlPacket();
        bool expired;
        
        CPPUNIT_ASSERT_EQUAL( mgr->Add(muxctrl), 0 );
        CPPUNIT_ASSERT( mgr->GetPacket(expired, NULL) == NULL );
    }
    
    void testRetransLimit() {
        CRtcpAppMuxCtrlPacket* muxctrl = new CRtcpAppMuxCtrlPacket();
        bool expired;
        CPacketBuffer* buffer;

        mgr->SetRetransmissionLimit(2);
        
        CPPUNIT_ASSERT_EQUAL( mgr->Add(muxctrl), 0 );
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == muxctrl );
        CPPUNIT_ASSERT_EQUAL( expired, false );
        CPPUNIT_ASSERT( buffer != NULL );
    
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == NULL );

        // do it twice to verify looping
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == muxctrl );
        CPPUNIT_ASSERT_EQUAL( expired, false );
        CPPUNIT_ASSERT( buffer != NULL );
    
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == NULL );

        // third time the packet should expire
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == muxctrl );
        CPPUNIT_ASSERT_EQUAL( expired, true );
        CPPUNIT_ASSERT( buffer == NULL );
    
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == NULL );

        // fourth time the list should be empty
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == NULL );

        // we own the memory as it timed out
        delete muxctrl;
    }

    void testAck() {
        CRtcpAppMuxCtrlPacket* muxctrl = new CRtcpAppMuxCtrlPacket();
        CRtcpTipAckPacket ack(ACK_MUXCTRL);
        CRtcpTipAckPacket badack(ACK_MEDIAOPTS);
        bool expired;
        CPacketBuffer* buffer;

        muxctrl->SetNtpTime(1);
        CPPUNIT_ASSERT_EQUAL( mgr->Add(muxctrl), 0 );

        CPPUNIT_ASSERT( mgr->Ack(badack) == NULL );
        CPPUNIT_ASSERT( mgr->Ack(ack) == NULL );

        ack.SetNtpTime(1);
        CPPUNIT_ASSERT_EQUAL( mgr->Ack(ack), (CRtcpTipPacket*) muxctrl );

        // verify packet has been removed
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == NULL );

        // we own the memory as it was acked
        delete muxctrl;
    }

    void testRemove() {
        CRtcpAppMuxCtrlPacket* muxctrl = new CRtcpAppMuxCtrlPacket();
        bool expired;
        CPacketBuffer* buffer;

        mgr->SetRetransmissionLimit(2);
        
        CPPUNIT_ASSERT_EQUAL( mgr->Add(muxctrl), 0 );
        CPPUNIT_ASSERT( mgr->Remove(MEDIAOPTS) == NULL );
        CPPUNIT_ASSERT( mgr->Remove(MUXCTRL) == muxctrl );

        // verify packet has been removed
        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == NULL );

        // we own the memory as we removed the packet
        delete muxctrl;
    }

    void testAddWithoutWrapper() {
        CRtcpAppMuxCtrlPacket* muxctrl = new CRtcpAppMuxCtrlPacket();
        CPacketBuffer* buffer;
        bool expired;

        mgr->EnableWrapper(0x12345678);
        mgr->DisableWrapper();
        CPPUNIT_ASSERT_EQUAL( mgr->Add(muxctrl), 0 );

        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == muxctrl );

        // unpack buffer into MUXCTRL
        CPPUNIT_ASSERT_EQUAL( muxctrl->Unpack(*buffer), 0 );
    }
    
    void testAddWithWrapper() {
        CRtcpRRPacket rr;
        CRtcpSDESPacket sdes;
        CRtcpAppMuxCtrlPacket* muxctrl = new CRtcpAppMuxCtrlPacket();
        CPacketBuffer* buffer;
        bool expired;

        mgr->EnableWrapper(0x12345678);
        CPPUNIT_ASSERT_EQUAL( mgr->Add(muxctrl), 0 );

        CPPUNIT_ASSERT( mgr->GetPacket(expired, &buffer) == muxctrl );

        // unpack buffer into an RR, SDES, MUXCTRL
        CPPUNIT_ASSERT_EQUAL( rr.Unpack(*buffer), 0 );
        CPPUNIT_ASSERT_EQUAL( sdes.Unpack(*buffer), 0 );
        CPPUNIT_ASSERT_EQUAL( muxctrl->Unpack(*buffer), 0 );

        CPPUNIT_ASSERT_EQUAL( rr.GetSSRC(), (uint32_t) 0x12345678 );
        CPPUNIT_ASSERT_EQUAL( sdes.GetChunkSSRC(0), (uint32_t) 0x12345678 );
    }
    
    void testFind() {
        CRtcpAppMuxCtrlPacket* muxctrl = new CRtcpAppMuxCtrlPacket();

        mgr->SetRetransmissionLimit(2);
        
        CPPUNIT_ASSERT_EQUAL( mgr->Add(muxctrl), 0 );
        CPPUNIT_ASSERT( mgr->Find(MEDIAOPTS) == NULL );
        CPPUNIT_ASSERT( mgr->Find(MUXCTRL) == muxctrl );
    }
    
    void testTimerDefault() {
        CPPUNIT_ASSERT_EQUAL( mgr->GetNextTransmitTime(), (uint64_t) -1 );
    }

    void testTimerAdd() {
        CRtcpAppMuxCtrlPacket* muxctrl = new CRtcpAppMuxCtrlPacket();

        CPPUNIT_ASSERT_EQUAL( mgr->Add(muxctrl), 0 );
        CPPUNIT_ASSERT_EQUAL( mgr->GetNextTransmitTime(), (uint64_t) 0 );
    }

    void testTimerSend() {
        CRtcpAppMuxCtrlPacket* muxctrl = new CRtcpAppMuxCtrlPacket();
        bool expired;
        CPacketBuffer* buffer;

        mgr->SetRetransmissionInterval(1000);
        
        CPPUNIT_ASSERT_EQUAL( mgr->Add(muxctrl), 0 );
        sleep(1);

        mgr->GetPacket(expired, &buffer);
        mgr->GetPacket(expired, &buffer);
        CPPUNIT_ASSERT( mgr->GetNextTransmitTime() > 900 );
    }

    void testTimerSend2() {
        CRtcpAppMuxCtrlPacket* muxctrl = new CRtcpAppMuxCtrlPacket();
        CRtcpAppMuxCtrlPacket* muxctrl2 = new CRtcpAppMuxCtrlPacket();
        bool expired;
        CPacketBuffer* buffer;
    
        mgr->SetRetransmissionInterval(1000);
        
        CPPUNIT_ASSERT_EQUAL( mgr->Add(muxctrl), 0 );
        CPPUNIT_ASSERT_EQUAL( mgr->Add(muxctrl2), 0 );

        mgr->GetPacket(expired, &buffer);
        mgr->GetPacket(expired, &buffer);
        mgr->GetPacket(expired, &buffer);
        CPPUNIT_ASSERT( mgr->GetNextTransmitTime() > 900 );

        sleep(1);
    
        mgr->GetPacket(expired, &buffer);
        CPPUNIT_ASSERT_EQUAL( mgr->GetNextTransmitTime(), (uint64_t) 0 );
    
        mgr->GetPacket(expired, &buffer);
        mgr->GetPacket(expired, &buffer);
        CPPUNIT_ASSERT( mgr->GetNextTransmitTime() > 900 );
    }

    CPPUNIT_TEST_SUITE( CTipPacketManagerTest );
    CPPUNIT_TEST( testCreate );
    CPPUNIT_TEST( testInterval );
    CPPUNIT_TEST( testLimit );
    CPPUNIT_TEST( testAdd );
    CPPUNIT_TEST( testAddMulti );
    CPPUNIT_TEST( testAddMultiMid );
    CPPUNIT_TEST( testAddInvalid );
    CPPUNIT_TEST( testGetInvalid );
    CPPUNIT_TEST( testRetransLimit );
    CPPUNIT_TEST( testAck );
    CPPUNIT_TEST( testRemove );
    CPPUNIT_TEST( testAddWithoutWrapper );
    CPPUNIT_TEST( testAddWithWrapper );
    CPPUNIT_TEST( testFind );
    CPPUNIT_TEST( testTimerDefault );
    CPPUNIT_TEST( testTimerAdd );
    CPPUNIT_TEST( testTimerSend );
    CPPUNIT_TEST( testTimerSend2 );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CTipPacketManagerTest );
