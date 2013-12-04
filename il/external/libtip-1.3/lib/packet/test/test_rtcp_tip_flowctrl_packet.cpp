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

#include "rtcp_tip_flowctrl_packet.h"
#include "test_packet_data.h"
#include "tip_debug_tools.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CRtcpAppFlowCtrlPacketTest : public CppUnit::TestFixture {
private:
    CRtcpAppFlowCtrlPacket* packet;

public:
    void setUp() {
        packet = new CRtcpAppFlowCtrlPacket(TXFLOWCTRL);
    }

    void tearDown() {
        delete packet;
    }

    void testDefaults() {
        CPPUNIT_ASSERT_EQUAL( packet->GetOpcode(), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetTarget(), (uint32_t) 0 );
    }

    void testOpcode() {
        packet->SetOpcode(0xA5A5A5A5);
        CPPUNIT_ASSERT_EQUAL( packet->GetOpcode(), (uint32_t) 0xA5A5A5A5 );
    }

    void testTarget() {
        packet->SetTarget(0xA5A5A5A5);
        CPPUNIT_ASSERT_EQUAL( packet->GetTarget(), (uint32_t) 0xA5A5A5A5 );
    }

#define RTCP_APP_FLOWCTRL_BYTES(opcode, target)      \
        ((opcode) >> 24), (((opcode) >> 16) & 0xFF), \
        (((opcode) >> 8) & 0xFF), ((opcode) & 0xFF), \
        ((target) >> 24), (((target) >> 16) & 0xFF), \
        (((target) >> 8) & 0xFF), ((target) & 0xFF)

    void testPack1() {
        CPacketBufferData buffer;

        uint8_t def[] = { RTCP_PACKET_BYTES(TXFLOWCTRL,
                                            CRtcpPacket::APP,
                                            packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1,
                          RTCP_TIP_PACKET_BYTES,
                          RTCP_APP_FLOWCTRL_BYTES(0, 0)
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

        uint8_t def[] = { RTCP_PACKET_BYTES(TXFLOWCTRL,
                                            CRtcpPacket::APP,
                                            packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1,
                          RTCP_TIP_PACKET_BYTES,
                          RTCP_APP_FLOWCTRL_BYTES(0xA5A5A5A5, 0x12345678)
        };

        packet->SetOpcode(0xA5A5A5A5);
        packet->SetTarget(0x12345678);
        
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
        CRtcpAppFlowCtrlPacket packet2(packet->GetTipPacketType());
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
        CRtcpAppFlowCtrlPacket packet2(TXFLOWCTRL);
        CPacketBufferData buffer;

        packet->SetOpcode(0xA5A5A5A5);
        packet->SetTarget(0x12345678);
        
        packet->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );

        CPPUNIT_ASSERT_EQUAL( packet->GetOpcode(), packet2.GetOpcode() );
        CPPUNIT_ASSERT_EQUAL( packet->GetTarget(), packet2.GetTarget() );
    }

    void testUnpackFail() {
        CRtcpAppFlowCtrlPacket packet2(TXFLOWCTRL);
        CPacketBufferData buffer;

        packet2.SetType(CRtcpPacket::RR);
        packet2.Pack(buffer);
        
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }
        
    void testTXFlowCtrl() {
        CRtcpAppTXFlowCtrlPacket packet2;
        CPPUNIT_ASSERT_EQUAL( packet2.GetTipPacketType(), TXFLOWCTRL );
    }
    
    void testRXFlowCtrl() {
        CRtcpAppRXFlowCtrlPacket packet2;
        CPPUNIT_ASSERT_EQUAL( packet2.GetTipPacketType(), RXFLOWCTRL );
    }
    
    void testToStream() {
        ostringstream oss;
        packet->ToStream(oss);

        CRtcpAppRXFlowCtrlPacket packet2;
        packet2.ToStream(oss);

        CRtcpAppTXFlowCtrlPacket packet3;
        packet3.ToStream(oss);
    }
    
    CPPUNIT_TEST_SUITE( CRtcpAppFlowCtrlPacketTest );
    CPPUNIT_TEST( testDefaults );
    CPPUNIT_TEST( testOpcode );
    CPPUNIT_TEST( testTarget );
    CPPUNIT_TEST( testPack1 );
    CPPUNIT_TEST( testPack2 );
    CPPUNIT_TEST( testUnpack1 );
    CPPUNIT_TEST( testUnpack2 );
    CPPUNIT_TEST( testUnpackFail );
    CPPUNIT_TEST( testTXFlowCtrl );
    CPPUNIT_TEST( testRXFlowCtrl );
    CPPUNIT_TEST( testToStream );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CRtcpAppFlowCtrlPacketTest );
