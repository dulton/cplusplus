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

#include "rtcp_tip_muxctrl_packet.h"
#include "test_packet_data.h"
#include "tip_debug_tools.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CRtcpAppMuxCtrlPacketTest : public CppUnit::TestFixture {
private:
    CRtcpAppMuxCtrlV7Packet* packet;

public:
    void setUp() {
        packet = new CRtcpAppMuxCtrlV7Packet();
    }

    void tearDown() {
        delete packet;
    }

    void testV6() {
        CRtcpAppMuxCtrlPacket packet2;
        CPPUNIT_ASSERT_EQUAL( packet2.GetVersion(), (uint8_t) CRtcpAppMuxCtrlPacket::DEFAULT_VERSION );
    }
    
    void testVersion() {
        CPPUNIT_ASSERT_EQUAL( packet->GetVersion(), (uint8_t) CRtcpAppMuxCtrlV7Packet::DEFAULT_VERSION );

        packet->SetVersion(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetVersion(), (uint8_t) 4 );
    }

    void testProfile() {
        CPPUNIT_ASSERT_EQUAL( packet->GetProfile(), (uint8_t) 0 );

        packet->SetProfile(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetProfile(), (uint8_t) 4 );
    }

    void testOptions() {
        CPPUNIT_ASSERT_EQUAL( packet->GetOptions(), (uint8_t) 0 );

        packet->SetOptions(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetOptions(), (uint8_t) 4 );
    }        
    
    void testNumXmit() {
        CPPUNIT_ASSERT_EQUAL( packet->GetNumXmit(), (uint8_t) 0 );

        packet->SetNumXmit(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetNumXmit(), (uint8_t) 4 );
    }

    void testNumRcv() {
        CPPUNIT_ASSERT_EQUAL( packet->GetNumRcv(), (uint8_t) 0 );

        packet->SetNumRcv(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetNumRcv(), (uint8_t) 4 );
    }

    void testXmitTime() {
        CPPUNIT_ASSERT_EQUAL( packet->GetNtpTime(), (uint64_t) 0 );

        packet->SetNtpTime(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetNtpTime(), (uint64_t) 4 );
    }

    void testConfID() {
        CPPUNIT_ASSERT_EQUAL( packet->GetConfID(), (uint64_t) 0 );

        packet->SetConfID(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetConfID(), (uint64_t) 4 );
    }

    void testXmitPositions() {
        CPPUNIT_ASSERT_EQUAL( packet->GetXmitPositions(), (uint16_t) 0 );

        packet->SetXmitPositions(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetXmitPositions(), (uint16_t) 4 );
    }

    void testRcvPositions() {
        CPPUNIT_ASSERT_EQUAL( packet->GetRcvPositions(), (uint16_t) 0 );

        packet->SetRcvPositions(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetRcvPositions(), (uint16_t) 4 );
    }

    void testNumShared() {
        CPPUNIT_ASSERT_EQUAL( packet->GetNumShared(), (uint8_t) 0 );

        packet->SetNumShared(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetNumShared(), (uint8_t) 4 );
    }

    void testSharedPositions() {
        CPPUNIT_ASSERT_EQUAL( packet->GetSharedPositions(), (uint16_t) 0 );

        packet->SetSharedPositions(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetSharedPositions(), (uint16_t) 4 );
    }

    void testParticipantID() {
        CPPUNIT_ASSERT_EQUAL( packet->GetParticipantID(NULL), (uint32_t) 0 );

        uint8_t* testID = (uint8_t*) "abcdABCD1234";
        uint32_t testIDLen = strlen((char*) testID);
        uint8_t getBuffer[CTipTlv::MAX_TLV_DATA_LEN];

        CPPUNIT_ASSERT( packet->SetParticipantID(testID, testIDLen) == 0 );

        CPPUNIT_ASSERT( packet->GetParticipantID(getBuffer) == testIDLen );

        CPPUNIT_ASSERT( memcmp(getBuffer, testID, testIDLen) == 0 );

        // add a second one and verify it overwrites
        uint8_t* testID2 = (uint8_t*) "ABCD1234";
        uint32_t testID2Len = strlen((char*) testID2);
        
        CPPUNIT_ASSERT( packet->SetParticipantID(testID2, testID2Len) == 0 );
        
        CPPUNIT_ASSERT( packet->GetParticipantID(getBuffer) == testID2Len );

        CPPUNIT_ASSERT( memcmp(getBuffer, testID2, testID2Len) == 0 );

        // try and set some invalid things and make sure it fails
        CPPUNIT_ASSERT( packet->SetParticipantID(NULL, testIDLen) == -1 );

        CPPUNIT_ASSERT( packet->SetParticipantID(testID, CTipTlv::MAX_TLV_DATA_LEN + 1) == -1 );
    }

#define RTCP_MUXCTRL_PACKET_BYTES(ver)                              \
    (ver), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                    \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                 \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                 \
        0x00, 0x00, 0x00, 0x00

    void testPack() {
        CPacketBufferData buffer;

        // NOTE:  do not include RTCP_TIP_PACKET_BYTES here
        uint8_t def[] = { RTCP_PACKET_BYTES(MUXCTRL, CRtcpPacket::APP, packet->GetLength()),
                        RTCP_APP_PACKET_BYTES_TP1,
                        RTCP_MUXCTRL_PACKET_BYTES(CRtcpAppMuxCtrlV7Packet::DEFAULT_VERSION << 4) };
    
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

#define RTCP_MUXCTRL_PACKET_BYTES_PID 0x01, 0x04, 'a', 'b', 'c', 'd'

    void testPack2() {
        CPacketBufferData buffer;

        packet->SetParticipantID((uint8_t*) "abcd", 4);
    
        // NOTE:  do not include RTCP_TIP_PACKET_BYTES here
        uint8_t def[] = { RTCP_PACKET_BYTES(MUXCTRL, CRtcpPacket::APP, packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1,
                          RTCP_MUXCTRL_PACKET_BYTES(CRtcpAppMuxCtrlV7Packet::DEFAULT_VERSION << 4),
                          RTCP_MUXCTRL_PACKET_BYTES_PID,
                          0, 0 // padding
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

    void testUnpack() {
        CRtcpAppMuxCtrlV7Packet packet2;
        CPacketBufferData buffer;

        packet->SetProfile(4);
        packet->SetOptions(4);
        packet->SetNumXmit(4);
        packet->SetNumRcv(4);
        packet->SetNtpTime(4);
        packet->SetConfID(4);
        packet->SetXmitPositions(4);
        packet->SetRcvPositions(4);
        packet->SetNumShared(4);
        packet->SetSharedPositions(4);

        uint8_t* testID = (uint8_t*) "abcdABCD1234";
        uint32_t testIDLen = strlen((char*) testID);
        packet->SetParticipantID(testID, testIDLen);
    
        packet->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );

        CPPUNIT_ASSERT_EQUAL( packet->GetVersion(), packet2.GetVersion() );
        CPPUNIT_ASSERT_EQUAL( packet->GetProfile(), packet2.GetProfile() );
        CPPUNIT_ASSERT_EQUAL( packet->GetOptions(), packet2.GetOptions() );
        CPPUNIT_ASSERT_EQUAL( packet->GetNumXmit(), packet2.GetNumXmit() );
        CPPUNIT_ASSERT_EQUAL( packet->GetNumRcv(), packet2.GetNumRcv() );
        CPPUNIT_ASSERT_EQUAL( packet->GetConfID(), packet2.GetConfID() );
        CPPUNIT_ASSERT_EQUAL( packet->GetXmitPositions(), packet2.GetXmitPositions() );
        CPPUNIT_ASSERT_EQUAL( packet->GetRcvPositions(), packet2.GetRcvPositions() );
        CPPUNIT_ASSERT_EQUAL( packet->GetNumShared(), packet2.GetNumShared() );
        CPPUNIT_ASSERT_EQUAL( packet->GetSharedPositions(), packet2.GetSharedPositions() );

        uint8_t getBuffer[CTipTlv::MAX_TLV_DATA_LEN];
        CPPUNIT_ASSERT( packet2.GetParticipantID(getBuffer) == testIDLen );
        CPPUNIT_ASSERT( memcmp(getBuffer, testID, testIDLen) == 0 );

        CPPUNIT_ASSERT_EQUAL( packet->GetLength(), packet2.GetLength() );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet2.GetPackSize() );
    }

    void testUnpackFail1() {
        CRtcpAppMuxCtrlV7Packet packet2;
        CPacketBufferData buffer;

        packet2.SetType(CRtcpPacket::RR);
        packet2.Pack(buffer);
        
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }
    
    void testUnpackFail2() {
        CRtcpAppMuxCtrlV7Packet packet2;
        CPacketBufferData buffer;

        packet2.Pack(buffer);

        // add a malformed TLV
        uint8_t data = 2;
        buffer.Add(data);
        buffer.Add(data);
        
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testUnpackFail3() {
        CPacketBufferData buffer;
        
        packet->SetVersion(5);
        packet->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }
    
    void testUnpackFail4() {
        CPacketBufferData buffer;
        
        packet->SetVersion(6);
        packet->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testUnpackFail5() {
        CPacketBufferData buffer;

        // set to an invalid appname
        packet->SetAppName("xtp1");
        packet->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testUnpackFail6() {
        CRtcpAppMuxCtrlPacket packet1;
        CRtcpAppMuxCtrlPacket packet2;
        CPacketBufferData buffer;

        packet2.SetType(CRtcpPacket::RR);
        packet2.Pack(buffer);
        
        CPPUNIT_ASSERT_EQUAL( packet1.Unpack(buffer), -1 );
    }
    
    void testUnpackFail7() {
        CRtcpAppMuxCtrlPacket packet1;
        CRtcpAppMuxCtrlPacket packet2;
        CPacketBufferData buffer;

        packet2.SetVersion(5);
        packet2.Pack(buffer);
        
        CPPUNIT_ASSERT_EQUAL( packet1.Unpack(buffer), -1 );
    }
    
    void testUnpackFail8() {
        CRtcpAppMuxCtrlPacket packet1;
        CRtcpAppMuxCtrlPacket packet2;
        CPacketBufferData buffer;

        packet2.SetVersion(7);
        packet2.Pack(buffer);
        
        CPPUNIT_ASSERT_EQUAL( packet1.Unpack(buffer), -1 );
    }
    
    void testUnpackClear() {
        uint8_t* testID = (uint8_t*) "abcdABCD1234";
        uint32_t testIDLen = strlen((char*) testID);
        packet->SetParticipantID(testID, testIDLen);

        CRtcpAppMuxCtrlV7Packet packet2;
        CPacketBufferData buffer;

        packet2.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet2.GetPackSize() );
    }
                          
    void testToStream() {
        ostringstream oss;
        packet->ToStream(oss);
    }

    CPPUNIT_TEST_SUITE( CRtcpAppMuxCtrlPacketTest );
    CPPUNIT_TEST( testV6 );
    CPPUNIT_TEST( testVersion );
    CPPUNIT_TEST( testProfile );
    CPPUNIT_TEST( testOptions );
    CPPUNIT_TEST( testNumXmit );
    CPPUNIT_TEST( testNumRcv );
    CPPUNIT_TEST( testXmitTime );
    CPPUNIT_TEST( testConfID );
    CPPUNIT_TEST( testXmitPositions );
    CPPUNIT_TEST( testRcvPositions );
    CPPUNIT_TEST( testNumShared );
    CPPUNIT_TEST( testSharedPositions );
    CPPUNIT_TEST( testParticipantID );
    CPPUNIT_TEST( testPack );
    CPPUNIT_TEST( testPack2 );
    CPPUNIT_TEST( testUnpack );
    CPPUNIT_TEST( testUnpackFail1 );
    CPPUNIT_TEST( testUnpackFail2 );
    CPPUNIT_TEST( testUnpackFail3 );
    CPPUNIT_TEST( testUnpackFail4 );
    CPPUNIT_TEST( testUnpackFail5 );
    CPPUNIT_TEST( testUnpackFail6 );
    CPPUNIT_TEST( testUnpackFail7 );
    CPPUNIT_TEST( testUnpackFail8 );
    CPPUNIT_TEST( testUnpackClear );
    CPPUNIT_TEST( testToStream );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CRtcpAppMuxCtrlPacketTest );
