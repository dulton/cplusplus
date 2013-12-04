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

#include <iostream>
using namespace std;

#include "rtcp_tip_echo_packet.h"
#include "test_packet_data.h"
#include "tip_debug_tools.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CRtcpAppEchoPacketTest : public CppUnit::TestFixture {
private:
    CRtcpAppEchoPacket* packet;

public:
    void setUp() {
        packet = new CRtcpAppEchoPacket();
    }

    void tearDown() {
        delete packet;
    }

    void testDefaults() {
        CPPUNIT_ASSERT_EQUAL( packet->GetTipPacketType(), TIPECHO );
        CPPUNIT_ASSERT_EQUAL( packet->GetRcvNtpTime(), (uint64_t) 0 );
    }

    void testRcvNtpTime() {
        packet->SetRcvNtpTime(0x1234567890ABCDEFLL);
        CPPUNIT_ASSERT_EQUAL( packet->GetRcvNtpTime(),
                              (uint64_t) 0x1234567890ABCDEFLL );
    }

    void testAck() {
        CRtcpTipPacket ack(MUXCTRL);
        CPPUNIT_ASSERT_EQUAL( packet->IsMyAck(ack), false );
    }
    
    void testAck2() {
        CRtcpAppEchoPacket ack;
        ack.SetNtpTime(1);
        
        CPPUNIT_ASSERT_EQUAL( packet->IsMyAck(ack), false );
    }
    
    void testAck3() {
        CRtcpAppEchoPacket ack;

        ack.SetNtpTime(1);
        packet->SetNtpTime(1);

        packet->SetRcvNtpTime(2);
        
        CPPUNIT_ASSERT_EQUAL( packet->IsMyAck(ack), false );
    }
    
    void testAck4() {
        CRtcpTipPacket ack(TIPECHO);

        ack.SetNtpTime(1);
        packet->SetNtpTime(1);

        CPPUNIT_ASSERT_EQUAL( packet->IsMyAck(ack), false );
    }
    
    void testAck5() {
        CRtcpAppEchoPacket ack;

        ack.SetNtpTime(1);
        packet->SetNtpTime(1);

        CPPUNIT_ASSERT_EQUAL( packet->IsMyAck(ack), false );
    }
    
    void testAck6() {
        CRtcpAppEchoPacket ack;

        ack.SetNtpTime(1);
        packet->SetNtpTime(1);

        ack.SetRcvNtpTime(2);
        
        CPPUNIT_ASSERT_EQUAL( packet->IsMyAck(ack), true );
    }

    void testToStream() {
        ostringstream oss;
        packet->ToStream(oss);
    }
    
#define RTCP_APP_ECHO_BYTES(r1, r2)             \
    ((r1) >> 24), (((r1) >> 16) & 0xFF),        \
        (((r1) >> 8) & 0xFF), ((r1) & 0xFF),    \
        ((r2) >> 24), (((r2) >> 16) & 0xFF),    \
        (((r2) >> 8) & 0xFF), ((r2) & 0xFF)

    void testPack1() {
        CPacketBufferData buffer;

        uint8_t def[] = { RTCP_PACKET_BYTES(TIPECHO,
                                            CRtcpPacket::APP,
                                            packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1,
                          RTCP_TIP_PACKET_BYTES,
                          RTCP_APP_ECHO_BYTES(0, 0)
        };

        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), (uint32_t) sizeof(def) );
        CPPUNIT_ASSERT_EQUAL( (packet->GetPackSize() % 4), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet->Pack(buffer) );
        if (memcmp(buffer.GetBuffer(), def, packet->GetPackSize()) != 0) {
            ostringstream oss;
            
            oss << "\nExpected:  " << HexDump(def, packet->GetPackSize())
                << "\nPacked:    " << HexDump(buffer.GetBuffer(), packet->GetPackSize());
            CPPUNIT_FAIL(oss.str());
        }
    }
    
    void testPack2() {
        CPacketBufferData buffer;

        uint8_t def[] = { RTCP_PACKET_BYTES(TIPECHO,
                                            CRtcpPacket::APP,
                                            packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1,
                          RTCP_TIP_PACKET_BYTES,
                          RTCP_APP_ECHO_BYTES(0x12345678, 0x90ABCDEF)
        };

        packet->SetRcvNtpTime(0x1234567890ABCDEFLL);
        
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), (uint32_t) sizeof(def) );
        CPPUNIT_ASSERT_EQUAL( (packet->GetPackSize() % 4), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet->Pack(buffer) );
        if (memcmp(buffer.GetBuffer(), def, packet->GetPackSize()) != 0) {
            ostringstream oss;
            
            oss << "\nExpected:  " << HexDump(def, packet->GetPackSize())
                << "\nPacked:    " << HexDump(buffer.GetBuffer(), packet->GetPackSize());
            CPPUNIT_FAIL(oss.str());
        }
    }

    void testUnpack1() {
        CRtcpAppEchoPacket packet2;
        CPacketBufferData buffer;
        CPacketBufferData buffer2;

        packet->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );
        packet2.Pack(buffer2);
        if (memcmp(buffer.GetBuffer(), buffer2.GetBuffer(), buffer.GetBufferSize()) != 0) {
            ostringstream oss;
            
            oss << "\nExpected:  " << HexDump(buffer.GetBuffer(), buffer.GetBufferSize())
                << "\nPacked:    " << HexDump(buffer2.GetBuffer(), buffer.GetBufferSize());

            CPPUNIT_FAIL(oss.str());
        }
    }
    
    void testUnpack2() {
        CRtcpAppEchoPacket packet2;
        CPacketBufferData buffer;

        packet->SetRcvNtpTime(0x1234567890ABCDEFLL);
        packet->Pack(buffer);
        
        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetRcvNtpTime(), packet2.GetRcvNtpTime() );
    }

    void testUnpackFail() {
        CRtcpAppEchoPacket packet2;
        CPacketBufferData buffer;

        packet2.SetType(CRtcpPacket::RR);
        packet2.Pack(buffer);
        
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    CPPUNIT_TEST_SUITE( CRtcpAppEchoPacketTest );
    CPPUNIT_TEST( testDefaults );
    CPPUNIT_TEST( testRcvNtpTime );
    CPPUNIT_TEST( testAck );
    CPPUNIT_TEST( testAck2 );
    CPPUNIT_TEST( testAck3 );
    CPPUNIT_TEST( testAck4 );
    CPPUNIT_TEST( testAck5 );
    CPPUNIT_TEST( testAck6 );
    CPPUNIT_TEST( testToStream );
    CPPUNIT_TEST( testPack1 );
    CPPUNIT_TEST( testPack2 );
    CPPUNIT_TEST( testUnpack1 );
    CPPUNIT_TEST( testUnpack2 );
    CPPUNIT_TEST( testUnpackFail );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CRtcpAppEchoPacketTest );
