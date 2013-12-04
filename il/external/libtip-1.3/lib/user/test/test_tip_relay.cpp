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

#include "tip_csrc.h"
#include "rtcp_packet.h"
#include "rtcp_tip_muxctrl_packet.h"
#include "rtcp_tip_mediaopts_packet.h"
#include "rtcp_tip_ack_packet.h"
#include "rtcp_tip_flowctrl_packet.h"
#include "rtcp_tip_refresh_packet.h"
#include "rtcp_tip_reqtosend_packet.h"
#include "rtcp_tip_feedback_packet.h"
#include "rtcp_tip_echo_packet.h"
#include "rtcp_tip_spimap_packet.h"
#include "rtcp_tip_notify_packet.h"
#include "tip_relay.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CTipRelayTest : public CppUnit::TestFixture {
private:
    CTipRelay* amr;
    uint16_t pos;

public:
    void setUp() {
        amr = new CTipRelay();
        CPPUNIT_ASSERT( amr != NULL );

        pos = 0;
    }

    void tearDown() {
        delete amr;
    }
    
    void testClassifyInvalid() {
        uint8_t buffer[1024] = { 0 };

        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer, sizeof(buffer), pos),
                              CTipRelay::NOT_TIP );
    }

    void testClassifyInvalid2() {
        CRtcpPacket packet;
        CPacketBufferData buffer;

        packet.SetType(CRtcpPacket::RR);
        packet.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::NOT_TIP );
    }

    void testClassifyInvalid3() {
        CRtcpPacket packet;
        CPacketBufferData buffer;

        packet.SetType(CRtcpPacket::APP);
        packet.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::NOT_TIP );
    }

    void testClassifyInvalid4() {
        TipPacketType invalid_types[] = {
            RESERVED0,
            RESERVED2,
            RESERVED9,
            RESERVED10,
            RESERVED11,
            RESERVED13,
            RESERVED14,
            ACK_RESERVED0,
            ACK_RESERVED2,
            ACK_RESERVED9,
            ACK_RESERVED10,
            ACK_RESERVED11,
            ACK_RESERVED13,
            ACK_RESERVED14
        };

        for (uint32_t i = 0; i < (sizeof(invalid_types) / sizeof(TipPacketType)); i++) {
            CRtcpTipPacket packet(invalid_types[i]);
            CPacketBufferData buffer;

            packet.Pack(buffer);
            CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                                  CTipRelay::NOT_TIP );
        }
    }
    
    void testClassifyMuxCtrl() {
        CRtcpAppMuxCtrlPacket packet;
        CPacketBufferData buffer;

        packet.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::SYSTEM );

        CRtcpTipAckPacket ack(ACK_MUXCTRL);
        buffer.Reset();
        ack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::SYSTEM );
    }

    void testClassifyMediaopts() {
        CRtcpAppMediaoptsPacket packet;
        CPacketBufferData buffer;

        packet.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::SYSTEM );

        CRtcpTipAckPacket ack(ACK_MEDIAOPTS);
        buffer.Reset();
        ack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::SYSTEM );
    }

    void testClassifyReqToSend() {
        CRtcpAppReqToSendPacket packet;
        CPacketBufferData buffer;

        packet.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::SYSTEM );

        CRtcpAppReqToSendAckPacket ack;
        buffer.Reset();
        ack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::SYSTEM );
    }

    void testClassifySystemInvalid() {
        TipPacketType system_types[] = {
            ACK_TIPECHO, // not used
        };

        for (uint32_t i = 0; i < (sizeof(system_types) / sizeof(TipPacketType)); i++) {
            CRtcpTipPacket packet(system_types[i]);
            CPacketBufferData buffer;

            packet.Pack(buffer);
            CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                                  CTipRelay::NOT_TIP );
        }
    }

    void testClassifyTxFlowCtrl() {
        CRtcpAppTXFlowCtrlPacket packet;
        CPacketBufferData buffer;
        CTipCSRC csrc;

        csrc.SetSourcePos(1);
        packet.SetTarget(csrc.GetCSRC());
        packet.Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::MEDIA_SOURCE );
        CPPUNIT_ASSERT_EQUAL( pos, (uint16_t) (1 << 1) );

        CRtcpTipAckPacket ack(ACK_TXFLOWCTRL);
        buffer.Reset();
        ack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::MEDIA_SINK );
    }
    
    void testClassifyRxFlowCtrl() {
        CRtcpAppRXFlowCtrlPacket packet;
        CPacketBufferData buffer;
        CTipCSRC csrc;

        csrc.SetSourcePos(1);
        packet.SetTarget(csrc.GetCSRC());
        packet.Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::MEDIA_SINK );
        CPPUNIT_ASSERT_EQUAL( pos, (uint16_t) (1 << 1) );

        CRtcpTipAckPacket ack(ACK_RXFLOWCTRL);
        buffer.Reset();
        ack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::MEDIA_SOURCE );
    }

    void testClassifyRefresh() {
        CRtcpAppRefreshPacket packet;
        CPacketBufferData buffer;
        CTipCSRC csrc;

        csrc.SetSourcePos(1);
        packet.SetTarget(csrc.GetCSRC());
        packet.Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::MEDIA_SOURCE );
        CPPUNIT_ASSERT_EQUAL( pos, (uint16_t) (1 << 1) );

        CRtcpTipAckPacket ack(ACK_REFRESH);
        buffer.Reset();
        ack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::MEDIA_SINK );
    }

    void testClassifyFB() {
        CRtcpAppFeedbackPacket packet;
        CPacketBufferData buffer;
        CTipCSRC csrc;

        csrc.SetSourcePos(1);
        packet.SetTarget(csrc.GetCSRC());
        packet.Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::MEDIA_SOURCE );
        CPPUNIT_ASSERT_EQUAL( pos, (uint16_t) (1 << 1) );
    }

    void testClassifyEcho() {
        CRtcpAppEchoPacket packet;
        CPacketBufferData buffer;

        packet.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::SYSTEM );
    }
    
    void testClassifySpiMap() {
        CRtcpAppSpiMapPacket packet;
        CPacketBufferData buffer;

        packet.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::SYSTEM );

        CRtcpTipAckPacket ack(ACK_SPIMAP);
        buffer.Reset();
        ack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::SYSTEM );
    }

    void testClassifyNotify() {
        CRtcpAppNotifyPacket packet;
        CPacketBufferData buffer;

        packet.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::SYSTEM );

        CRtcpTipAckPacket ack(ACK_NOTIFY);
        buffer.Reset();
        ack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( amr->Classify(buffer.GetBuffer(), buffer.GetBufferSize(), pos),
                              CTipRelay::SYSTEM );
    }

    CPPUNIT_TEST_SUITE( CTipRelayTest );
    CPPUNIT_TEST( testClassifyInvalid );
    CPPUNIT_TEST( testClassifyInvalid2 );
    CPPUNIT_TEST( testClassifyInvalid3 );
    CPPUNIT_TEST( testClassifyInvalid4 );
    CPPUNIT_TEST( testClassifyMuxCtrl );
    CPPUNIT_TEST( testClassifyMediaopts );
    CPPUNIT_TEST( testClassifyReqToSend );
    CPPUNIT_TEST( testClassifySystemInvalid );
    CPPUNIT_TEST( testClassifyTxFlowCtrl );
    CPPUNIT_TEST( testClassifyRxFlowCtrl );
    CPPUNIT_TEST( testClassifyRefresh );
    CPPUNIT_TEST( testClassifyFB );
    CPPUNIT_TEST( testClassifyEcho );
    CPPUNIT_TEST( testClassifySpiMap );
    CPPUNIT_TEST( testClassifyNotify );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CTipRelayTest );
