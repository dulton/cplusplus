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

#include "rtcp_tip_notify_packet.h"
#include "test_packet_data.h"
#include "tip_debug_tools.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CRtcpAppNotifyPacketTest : public CppUnit::TestFixture {
private:
    CRtcpAppNotifyPacket* packet;

public:
    void setUp() {
        packet = new CRtcpAppNotifyPacket();
        CPPUNIT_ASSERT( packet != NULL );
    }

    void tearDown() {
        delete packet;
    }

    void testDefaults() {
        CPPUNIT_ASSERT_EQUAL( packet->GetTLVCount(), (uint32_t) 0 );
    }

    void testAdd() {
        CPPUNIT_ASSERT_EQUAL( packet->AddTLV(CRtcpAppNotifyPacket::RESERVED, NULL, 0), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetTLVCount(), (uint32_t) 1 );
    }

    void testAdd2() {
        CPPUNIT_ASSERT_EQUAL( packet->AddTLV(CRtcpAppNotifyPacket::RESERVED, NULL, 0), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->AddTLV(CRtcpAppNotifyPacket::RESERVED, NULL, 0), 0 );

        CPPUNIT_ASSERT_EQUAL( packet->GetTLVCount(), (uint32_t) 2 );
    }

    void testRem() {
        CPPUNIT_ASSERT_EQUAL( packet->AddTLV(CRtcpAppNotifyPacket::RESERVED, NULL, 0), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->RemTLV(CRtcpAppNotifyPacket::RESERVED), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetTLVCount(), (uint32_t) 0 );
    }
    
    void testRem2() {
        CPPUNIT_ASSERT_EQUAL( packet->AddTLV(CRtcpAppNotifyPacket::RESERVED, NULL, 0), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->AddTLV(CRtcpAppNotifyPacket::RESERVED, NULL, 0), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->RemTLV(CRtcpAppNotifyPacket::RESERVED), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetTLVCount(), (uint32_t) 1 );
    }

    void testRem3() {
        CPPUNIT_ASSERT_EQUAL( packet->RemTLV(CRtcpAppNotifyPacket::RESERVED), -1 );
    }
    
    void testGetTag() {
        CPPUNIT_ASSERT_EQUAL( packet->AddTLV(CRtcpAppNotifyPacket::RESERVED, NULL, 0), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetTLVByTag(CRtcpAppNotifyPacket::RESERVED, NULL), 0 );
    }
    
    void testGetTag2() {
        uint32_t data = 0x12345678;
        uint32_t data2;
        
        CPPUNIT_ASSERT_EQUAL( packet->AddTLV(CRtcpAppNotifyPacket::RESERVED, (uint8_t*) &data, sizeof(data)), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetTLVByTag(CRtcpAppNotifyPacket::RESERVED, (uint8_t*) &data2), (int) sizeof(data2) );
        CPPUNIT_ASSERT_EQUAL( data, data2 );
    }

    void testGetTag3() {
        CPPUNIT_ASSERT_EQUAL( packet->GetTLVByTag(CRtcpAppNotifyPacket::RESERVED, NULL), -1 );
    }
    
    void testGetIndex() {
        CPPUNIT_ASSERT_EQUAL( packet->AddTLV(CRtcpAppNotifyPacket::RESERVED, NULL, 0), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetTLVByIndex(0, NULL), 0 );
    }
    
    void testGetIndex2() {
        uint32_t data = 0x12345678;
        uint32_t data2;
        
        CPPUNIT_ASSERT_EQUAL( packet->AddTLV(CRtcpAppNotifyPacket::RESERVED, (uint8_t*) &data, sizeof(data)), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetTLVByIndex(0, (uint8_t*) &data2), (int) sizeof(data2) );
        CPPUNIT_ASSERT_EQUAL( data, data2 );
    }
    
    void testGetIndex3() {
        CPPUNIT_ASSERT_EQUAL( packet->GetTLVByIndex(0, NULL), -1 );
    }

    void testGetIndex4() {
        CPPUNIT_ASSERT_EQUAL( packet->AddTLV(CRtcpAppNotifyPacket::RESERVED, NULL, 0), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetTLVByIndex(1, NULL), -1 );
    }        

#define RTCP_APP_NOTIFY_BYTES(tag, len, data)   \
    (tag), (len),                               \
    ((data) >> 24), (((data) >> 16) & 0xFF),    \
    (((data) >> 8) & 0xFF), ((data) & 0xFF)

    void testPack1() {
        CPacketBufferData buffer;

        uint8_t def[] = { RTCP_PACKET_BYTES(NOTIFY,
                                            CRtcpPacket::APP,
                                            packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1,
                          RTCP_TIP_PACKET_BYTES
        };

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

        // note: we have to convert here b/c TLV data is treated as a
        // uint8_t* buffer not a uint32_t.  so the burden is on the
        // user to convert before adding.
        uint32_t data = htonl(0x12345678);
        CPPUNIT_ASSERT_EQUAL( packet->AddTLV(CRtcpAppNotifyPacket::RESERVED, (uint8_t*) &data, sizeof(data)), 0 );

        data = htonl(0x90ABCDEF);
        CPPUNIT_ASSERT_EQUAL( packet->AddTLV(CRtcpAppNotifyPacket::SECURITYICON, (uint8_t*) &data, sizeof(data)), 0 );
        
        uint8_t def[] = { RTCP_PACKET_BYTES(NOTIFY,
                                            CRtcpPacket::APP,
                                            packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1,
                          RTCP_TIP_PACKET_BYTES,
                          RTCP_APP_NOTIFY_BYTES(0, 4, 0x12345678),
                          RTCP_APP_NOTIFY_BYTES(3, 4, 0x90ABCDEF),
        };

        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet->Pack(buffer) );
        if (memcmp(buffer.GetBuffer(), def, packet->GetPackSize()) != 0) {
            ostringstream oss;
            
            oss << "\nExpected:  " << HexDump(def, packet->GetPackSize())
                << "\nPacked:    " << HexDump(buffer.GetBuffer(), packet->GetPackSize());
            CPPUNIT_FAIL(oss.str());
        }
    }
    
    void testUnpack1() {
        CRtcpAppNotifyPacket packet2;
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
        CRtcpAppNotifyPacket packet2;
        CPacketBufferData buffer;
        CPacketBufferData buffer2;

        uint32_t data = 0x12345678;
        CPPUNIT_ASSERT_EQUAL( packet->AddTLV(CRtcpAppNotifyPacket::RESERVED, (uint8_t*) &data, sizeof(data)), 0 );
        
        packet->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );
        packet2.Pack(buffer2);

        uint32_t data2;
        CPPUNIT_ASSERT_EQUAL( packet2.GetTLVByTag(CRtcpAppNotifyPacket::RESERVED, (uint8_t*) &data2), (int) sizeof(data2) );
        CPPUNIT_ASSERT_EQUAL( data, data2 );
        
        if (memcmp(buffer.GetBuffer(), buffer2.GetBuffer(), buffer.GetBufferSize()) != 0) {
            ostringstream oss;
            
            oss << "\nExpected:  " << HexDump(buffer.GetBuffer(), buffer.GetBufferSize())
                << "\nPacked:    " << HexDump(buffer2.GetBuffer(), buffer.GetBufferSize());

            CPPUNIT_FAIL(oss.str());
        }
    }

    void testUnpackFail() {
        CRtcpAppNotifyPacket packet2;
        CPacketBufferData buffer;

        packet2.SetType(CRtcpPacket::RR);
        packet2.Pack(buffer);
        
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }
    
    void testUnpackClear() {
        uint32_t data = 0x12345678;
        CPPUNIT_ASSERT_EQUAL( packet->AddTLV(CRtcpAppNotifyPacket::RESERVED, (uint8_t*) &data, sizeof(data)), 0 );
        
        CRtcpAppNotifyPacket packet2;
        CPacketBufferData buffer;

        packet2.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet2.GetPackSize() );
    }
    
    void testToStream() {
        ostringstream oss;
        packet->ToStream(oss);
    }

    CPPUNIT_TEST_SUITE( CRtcpAppNotifyPacketTest );
    CPPUNIT_TEST( testDefaults );
    CPPUNIT_TEST( testAdd );
    CPPUNIT_TEST( testAdd2 );
    CPPUNIT_TEST( testRem );
    CPPUNIT_TEST( testRem2 );
    CPPUNIT_TEST( testRem3 );
    CPPUNIT_TEST( testGetTag );
    CPPUNIT_TEST( testGetTag2 );
    CPPUNIT_TEST( testGetTag3 );
    CPPUNIT_TEST( testGetIndex );
    CPPUNIT_TEST( testGetIndex2 );
    CPPUNIT_TEST( testGetIndex3 );
    CPPUNIT_TEST( testGetIndex4 );
    CPPUNIT_TEST( testPack1 );
    CPPUNIT_TEST( testPack2 );
    CPPUNIT_TEST( testUnpack1 );
    CPPUNIT_TEST( testUnpack2 );
    CPPUNIT_TEST( testUnpackFail );
    CPPUNIT_TEST( testUnpackClear );
    CPPUNIT_TEST( testToStream );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CRtcpAppNotifyPacketTest );
