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

#include "rtcp_tip_refresh_packet.h"
#include "test_packet_data.h"
#include "tip_debug_tools.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CRtcpAppRefreshPacketTest : public CppUnit::TestFixture {
private:
    CRtcpAppRefreshPacket* packet;

public:
    void setUp() {
        packet = new CRtcpAppRefreshPacket();
    }

    void tearDown() {
        delete packet;
    }

    void testDefaults() {
        CPPUNIT_ASSERT_EQUAL( packet->GetTarget(), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetFlags(), (uint32_t) 0 );
    }

    void testTarget() {
        packet->SetTarget(0xA5A5A5A5);
        CPPUNIT_ASSERT_EQUAL( packet->GetTarget(), (uint32_t) 0xA5A5A5A5 );
    }

    void testFlags() {
        packet->SetFlags(0xA5A5A5A5);
        CPPUNIT_ASSERT_EQUAL( packet->GetFlags(), (uint32_t) 0xA5A5A5A5 );
    }

#define RTCP_APP_REFRESH_BYTES(target, flags)        \
        ((target) >> 24), (((target) >> 16) & 0xFF), \
        (((target) >> 8) & 0xFF), ((target) & 0xFF), \
        ((flags) >> 24), (((flags) >> 16) & 0xFF),   \
        (((flags) >> 8) & 0xFF), ((flags) & 0xFF)

    void testPack1() {
        CPacketBufferData buffer;

        uint8_t def[] = { RTCP_PACKET_BYTES(REFRESH,
                                            CRtcpPacket::APP,
                                            packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1,
                          RTCP_TIP_PACKET_BYTES,
                          RTCP_APP_REFRESH_BYTES(0, 0)
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

        uint8_t def[] = { RTCP_PACKET_BYTES(REFRESH,
                                            CRtcpPacket::APP,
                                            packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1,
                          RTCP_TIP_PACKET_BYTES,
                          RTCP_APP_REFRESH_BYTES(0x12345678, 0x90ABCDEF)
        };

        packet->SetTarget(0x12345678);
        packet->SetFlags(0x90ABCDEF);
        
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
        CRtcpAppRefreshPacket packet2;
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
        CRtcpAppRefreshPacket packet2;
        CPacketBufferData buffer;

        packet->SetTarget(0x12345678);
        
        packet->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );

        CPPUNIT_ASSERT_EQUAL( packet->GetTarget(), packet2.GetTarget() );
    }

    void testUnpack3() {
        // special case, buffer is missing flags field.  should unpack
        // anyways with a default flags value.
        uint8_t def[] = { RTCP_PACKET_BYTES(REFRESH,
                                            CRtcpPacket::APP,
                                            // note -1 b/c this is
                                            // RTCP length not # of
                                            // bytes
                                            (packet->GetLength() - 1)),
                          RTCP_APP_PACKET_BYTES_TP1,
                          RTCP_TIP_PACKET_BYTES,
                          RTCP_APP_REFRESH_BYTES(0x12345678, 0x90ABCDEF)
        };

        CPacketBuffer buffer2(def, (sizeof(def) - 4));
        std::cout << buffer2.GetBufferSize() << std::endl;
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer2), 0 );

        CPPUNIT_ASSERT_EQUAL( packet->GetTarget(), (uint32_t) 0x12345678 );
        CPPUNIT_ASSERT_EQUAL( packet->GetFlags(),
                              (uint32_t) CRtcpAppRefreshPacket::REFRESH_PREFER_GDR );
    }
    
    void testUnpackFail() {
        CRtcpAppRefreshPacket packet2;
        CPacketBufferData buffer;

        packet2.SetType(CRtcpPacket::RR);
        packet2.Pack(buffer);
        
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    CPPUNIT_TEST_SUITE( CRtcpAppRefreshPacketTest );
    CPPUNIT_TEST( testDefaults );
    CPPUNIT_TEST( testTarget );
    CPPUNIT_TEST( testFlags );
    CPPUNIT_TEST( testPack1 );
    CPPUNIT_TEST( testPack2 );
    CPPUNIT_TEST( testUnpack1 );
    CPPUNIT_TEST( testUnpack2 );
    CPPUNIT_TEST( testUnpack3 );
    CPPUNIT_TEST( testUnpackFail );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CRtcpAppRefreshPacketTest );
