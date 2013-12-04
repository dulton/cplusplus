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

#include "rtcp_tip_reqtosend_packet.h"
#include "test_packet_data.h"
#include "tip_debug_tools.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CRtcpAppReqToSendPacketTest : public CppUnit::TestFixture {
private:
    CRtcpAppReqToSendPacket* packet;

public:
    void setUp() {
        packet = new CRtcpAppReqToSendPacket();
    }

    void tearDown() {
        delete packet;
    }

    void testDefaults() {
        CPPUNIT_ASSERT_EQUAL( packet->GetFlags(), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetVideoPos(), (uint16_t) 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetAudioPos(), (uint16_t) 0 );
    }

    void testFlags() {
        packet->SetFlags(0xA5A5A5A5);
        CPPUNIT_ASSERT_EQUAL( packet->GetFlags(), (uint32_t) 0xA5A5A5A5 );
    }

    void testVideoPos() {
        packet->SetVideoPos(0xA5A5);
        CPPUNIT_ASSERT_EQUAL( packet->GetVideoPos(), (uint16_t) 0xA5A5 );
    }

    void testAudioPos() {
        packet->SetAudioPos(0xA5A5);
        CPPUNIT_ASSERT_EQUAL( packet->GetAudioPos(), (uint16_t) 0xA5A5 );
    }

#define RTCP_APP_REQTOSEND_BYTES(flags, vpos, apos) \
    ((flags) >> 24), (((flags) >> 16) & 0xFF),      \
        (((flags) >> 8) & 0xFF), ((flags) & 0xFF),  \
        (((vpos) >> 8) & 0xFF), ((vpos) & 0xFF),    \
        (((apos) >> 8) & 0xFF), ((apos) & 0xFF)

    void testPack1() {
        CPacketBufferData buffer;

        uint8_t def[] = { RTCP_PACKET_BYTES(REQTOSEND,
                                            CRtcpPacket::APP,
                                            packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1,
                          RTCP_TIP_PACKET_BYTES,
                          RTCP_APP_REQTOSEND_BYTES(0, 0, 0)
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

        uint8_t def[] = { RTCP_PACKET_BYTES(REQTOSEND,
                                            CRtcpPacket::APP,
                                            packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1,
                          RTCP_TIP_PACKET_BYTES,
                          RTCP_APP_REQTOSEND_BYTES(0x12345678, 0x90AB, 0xCDEF)
        };

        packet->SetFlags(0x12345678);
        packet->SetVideoPos(0x90AB);
        packet->SetAudioPos(0xCDEF);
        
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
        CRtcpAppReqToSendPacket packet2;
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
        CRtcpAppReqToSendPacket packet2;
        CPacketBufferData buffer;

        packet->SetFlags(0x12345678);
        packet->SetVideoPos(0x90AB);
        packet->SetAudioPos(0xCDEF);
        
        packet->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );

        CPPUNIT_ASSERT_EQUAL( packet->GetFlags(), packet2.GetFlags() );
        CPPUNIT_ASSERT_EQUAL( packet->GetVideoPos(), packet2.GetVideoPos() );
        CPPUNIT_ASSERT_EQUAL( packet->GetAudioPos(), packet2.GetAudioPos() );
    }

    void testUnpackFail() {
        CRtcpAppReqToSendPacket packet2;
        CPacketBufferData buffer;

        packet2.SetType(CRtcpPacket::RR);
        packet2.Pack(buffer);
        
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testAck() {
        CRtcpAppReqToSendAckPacket ack;

        CPPUNIT_ASSERT_EQUAL( ack.GetTipPacketType(), ACK_REQTOSEND );
    }

    void testAck2() {
        packet->SetNtpTime(0x1234567890ABCDEFLL);
        packet->SetFlags(0x12345678);
        packet->SetVideoPos(0x90AB);
        packet->SetAudioPos(0xCDEF);

        CRtcpAppReqToSendAckPacket ack(*packet);
        CPPUNIT_ASSERT_EQUAL( ack.GetNtpTime(), (uint64_t) 0x1234567890ABCDEFLL );
        CPPUNIT_ASSERT_EQUAL( ack.GetFlags(), (uint32_t) 0x12345678 );
        CPPUNIT_ASSERT_EQUAL( ack.GetVideoPos(), (uint16_t) 0x90AB );
        CPPUNIT_ASSERT_EQUAL( ack.GetAudioPos(), (uint16_t) 0xCDEF );
    }
    
    void testToStream() {
        ostringstream oss;
        packet->ToStream(oss);
    }

    CPPUNIT_TEST_SUITE( CRtcpAppReqToSendPacketTest );
    CPPUNIT_TEST( testDefaults );
    CPPUNIT_TEST( testFlags );
    CPPUNIT_TEST( testVideoPos );
    CPPUNIT_TEST( testAudioPos );
    CPPUNIT_TEST( testPack1 );
    CPPUNIT_TEST( testPack2 );
    CPPUNIT_TEST( testUnpack1 );
    CPPUNIT_TEST( testUnpack2 );
    CPPUNIT_TEST( testUnpackFail );
    CPPUNIT_TEST( testAck );
    CPPUNIT_TEST( testAck2 );
    CPPUNIT_TEST( testToStream );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CRtcpAppReqToSendPacketTest );
